/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include <base.h>
#include "thread.h"

#if defined(GLib_WIN)

#include "win/thread_win.cpp"

#elif defined(GLib_UNIX)

#include "posix/thread_posix.cpp"

#else

// no support for threads in non-Win and non-Unix systems

#endif

TInterruptibleThread& TInterruptibleThread::operator=(const TInterruptibleThread& Other) {
    TThread::operator =(Other);
	SleeperBlocker = Other.SleeperBlocker;
	return *this;
}
	
void TInterruptibleThread::WaitForInterrupt(const int Msecs) {
	SleeperBlocker.Block(Msecs);
}

void TInterruptibleThread::Interrupt() {
	SleeperBlocker.Release();
}


////////////////////////////////////////////
// Thread executor
TThreadExecutor::TExecutorThread::TExecutorThread():
		TThread(),
		Executor(NULL),
		Runnable(NULL),
		Notify(NULL) {}

TThreadExecutor::TExecutorThread::TExecutorThread(TThreadExecutor* _Executor, const PNotify& _Notify):
		TThread(),
		Executor(_Executor),
		Runnable(NULL),
		Notify(_Notify) {}


void TThreadExecutor::TExecutorThread::Run() {
	while (true) {
		try {
			// get a task
			PRunnable Runnable = Executor->WaitForTask();
			// check if the executor was terminated
			if (Runnable.Empty()) { break; }
			// run the task
			Runnable->Run();
		} catch (const PExcept& Except) {
			Notify->OnNotifyFmt(TNotifyType::ntErr, "Exception in worker thread: %s", Except->GetMsgStr().CStr());
		}
	}

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Thread %ld exiting finished ...", GetThreadId());
}

TThreadExecutor::TThreadExecutor(const TInt& PoolSize, const PNotify& _Notify):
		ThreadV(),
		TaskQ(),
		Lock(),
		Notify(_Notify),
		IsFinished(false) {

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Creating executor with %d threads ...", PoolSize.Val);

	for (int i = 0; i < PoolSize; i++) {
		ThreadV.Add(TExecutorThread(this, Notify));
	}

	ThreadV.StartAll();
}

TThreadExecutor::~TThreadExecutor() {
	// cancel the threads
	Lock.Lock();

	for (int i = 0; i < ThreadV.Len(); i++) {
		if (ThreadV[i].IsAlive()) {
			ThreadV[i].Cancel();
		}
	}

	Lock.Release();
	// join
	ThreadV.Join();
}

void TThreadExecutor::Execute(const PRunnable& Runnable) {
	if (Runnable.Empty()) { return; }

	Lock.Lock();

	Notify->OnNotify(TNotifyType::ntInfo, "Adding new runnable to queue ...");
	TaskQ.Push(Runnable);
	Lock.Signal();

	int QSize = TaskQ.Len();

	Lock.Release();

	if (QSize > 0 && QSize % 100 == 0) {
		Notify->OnNotifyFmt(TNotifyType::ntWarn, "Task queue has %ld pending tasks!", QSize);
	}
}

TThreadExecutor::PRunnable TThreadExecutor::WaitForTask() {
	Lock.Lock();

	while (!IsFinished && TaskQ.Empty()) {
		Lock.WaitForSignal();
	}

	if (IsFinished) { return nullptr; }

	PRunnable Runnable = TaskQ.Pop();

	Lock.Release();

	return Runnable;
}

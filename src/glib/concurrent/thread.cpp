/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
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

	uint64 QSize = TaskQ.Len();

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

/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
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

	if (IsFinished) { return NULL; }

	PRunnable Runnable = TaskQ.Pop();

	Lock.Release();

	return Runnable;
}

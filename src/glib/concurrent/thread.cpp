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
	while (Runnable != NULL) {
		try {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Executing on thread %ld ...", GetThreadId());

			Runnable->Run();
			Runnable = NULL;

			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Thread %ld finished!", GetThreadId());

			Executor->OnThreadFinished(this);
		} catch (const PExcept& Except) {
			Notify->OnNotifyFmt(TNotifyType::ntErr, "Failed to execute thread: %s", Except->GetMsgStr().CStr());
			Notify->OnNotifyFmt(TNotifyType::ntErr, "%s", Except->GetLocStr().CStr());
		} catch (...) {
			Notify->OnNotify(TNotifyType::ntErr, "Unknown exception while executing thread!");
		}
	}

	// TODO danger!!! A task can remain the the queue and all threads are paused!!! Fix this before using
	// in a real scenario

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Thread %ld exiting Run method ...", GetThreadId());
}

TThreadExecutor::TThreadExecutor(const TInt& PoolSize, const PNotify& _Notify):
		ThreadV(),
		TaskQ(),
		QSection(TCriticalSectionType::cstRecursive),
		Notify(_Notify) {

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Creating executor with %d threads ...", PoolSize.Val);

	for (int i = 0; i < PoolSize; i++) {
		ThreadV.Add(TExecutorThread(this, Notify));
	}
}

TThreadExecutor::~TThreadExecutor() {
	TLock Lck(QSection);

	for (int i = 0; i < ThreadV.Len(); i++) {
		if (ThreadV[i].IsAlive()) {
			ThreadV[i].Cancel();
		}
	}
}

void TThreadExecutor::Execute(const PRunnable& Runnable) {
	TLock Lck(QSection);
	Notify->OnNotify(TNotifyType::ntInfo, "Adding new runnable to queue ...");
	TaskQ.Push(Runnable);
	ExecuteTasks();
}

void TThreadExecutor::OnThreadFinished(TExecutorThread* Thread) {
	TLock Lock(QSection);
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Thread %ld finished working, fetching new task ...", Thread->GetThreadId());
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "%ld tasks in queue ...", TaskQ.Len().Val);

	if (!TaskQ.Empty()) {
		Thread->SetRunnable(TaskQ.Pop());
	}
}

void TThreadExecutor::ExecuteTasks() {
	TLock Lck(QSection);
	Notify->OnNotify(TNotifyType::ntInfo, "Checking if any tasks can be executed ...");
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "%ld tasks in queue ...", TaskQ.Len().Val);

	// check if a thread is free
	int i = 0;
	while (!TaskQ.Empty() && i < ThreadV.Len()) {
		TExecutorThread& Worker = ThreadV[i];

		if (!Worker.IsAlive()) {
			Notify->OnNotify(TNotifyType::ntInfo, "Executing task ...");
			Worker.SetRunnable(TaskQ.Pop());
			Worker.Start();
		}
		i++;
	}
}

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
void TThreadExecutor::TExecutorThread::Run() {
	Notify->OnNotify("Executing task ...");

	try {
		Runnable->Run();
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "Failed to execute thread: %s", Except->GetMsgStr());
		Notify->OnNotifyFmt(TNotifyType::ntErr, "%s", Except->GetLocStr());
	}

	TLock Lck(QSection);

}

TThreadExecutor::TThreadExecutor(const TInt& PoolSize, const PNotify& _Notify):
		ThreadV(PoolSize, PoolSize),
		TaskQ(),
		QSection(TCriticalSectionType::cstRecursive),
		Notify(_Notify) {}

void TThreadExecutor::Execute(const TRunnable& Runnable) {
	Notify->OnNotify(TNotifyType::ntInfo, "Adding new runnable to queue ...");

	{
		TLock Lck(QSection);
		TaskQ.Add(Runnable);
		ExecuteTasks();
	}
}

void TThreadExecutor::ExecuteTasks() {
	Notify->OnNotify("Checking if any tasks can be executed ...");

	TLock Lck(QSection);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d tasks in queue ...", TaskQ.Len());

	// check if a thread is free
	int i = 0;
	while (!TaskQ.Empty() && i < ThreadV.Len()) {
		if (!ThreadV[i].IsRunning()) {
			ThreadV[i].ExecuteRunnable(TaskQ[0]);
		}
		i++;
	}
}

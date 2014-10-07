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

#ifndef THREAD_H
#define THREAD_H

#include <base.h>

enum TCriticalSectionType {
	cstFast,
	cstRecursive
};

#if defined(GLib_WIN)

#include "win/thread_win.h"

#elif defined(GLib_UNIX)

#include "posix/thread_posix.h"

#else

// no support for threads in non-Win and non-Unix systems

#endif

ClassTPE(TInterruptibleThread, PInterruptipleThread, TThread)// {
protected:
	// Use for interrupting and waiting
	TBlocker SleeperBlocker;
public:
	TInterruptibleThread(): TThread() { }
	TInterruptibleThread(const TInterruptibleThread& Other) { operator=(Other); }
	TInterruptibleThread& operator=(const TInterruptibleThread& Other);
	
	void WaitForInterrupt(const int Msecs = INFINITE);
	void Interrupt();
};

////////////////////////////////////////////
// Lock 
//   Wrapper around criticla section, which automatically enters 
//   on construct, and leaves on scope unwinding (destruct)
class TLock {
private:
	TCriticalSection& CriticalSection;
public:
	TLock(TCriticalSection& _CriticalSection):
		CriticalSection(_CriticalSection) { CriticalSection.Enter(); }
	~TLock() { CriticalSection.Leave(); }
};

////////////////////////////////////////////
// Thread executor
//   contains a pool of threads which can execute a TRunnable object
class TThreadExecutor {
public:
	ClassTP(TRunnable, PRunnable)// {
	public:
		void Run() = 0;
	};
private:
	class TExecutorThread: public TThread {
	private:
		PRunnable Runnable;
		PNotify Notify;
	public:
		TExecutorThread(const PNotify& _Notify): Runnable(NULL), Notify(_Notify) {}

		void Run();
		void ExecuteRunnable(PRunnable& Runnable);
	};

private:
	TVec<TExecutorThread> ThreadV;
	TVec<PRunnable> TaskQ;

	TCriticalSection QSection;

	PNotify Notify;

public:
	TThreadExecutor(const TInt& PoolSize=1, const PNotify& Notify=TNullNotify::New());

	virtual ~TThreadExecutor() {}

	void Execute(const TRunnable& Runnable);

private:
	void ExecuteTasks();
};

#endif

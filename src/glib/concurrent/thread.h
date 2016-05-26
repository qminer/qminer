/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef THREAD_H
#define THREAD_H

#include <base.h>

enum TMutexType {
	mtFast,
	mtRecursive
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
	TInterruptibleThread(const TInterruptibleThread& Other): TThread() { operator=(Other); }
	TInterruptibleThread& operator=(const TInterruptibleThread& Other);
	
	void WaitForInterrupt(const int Msecs = INFINITE);
	void Interrupt();
};

////////////////////////////////////////////
// Lock 
//   Wrapper around critical section, which automatically enters
//   on construct, and leaves on scope unwinding (destruct)
class TLock {
	friend class TCondVarLock;
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
	class TRunnable;
		typedef TPt<TRunnable> PRunnable;
	class TRunnable {
	private:
		TCRef CRef;
	public:
		friend class TPt<TRunnable>;
	public:
		TRunnable() {}
		virtual ~TRunnable() {}

		virtual void Run() = 0;
		bool operator ==(const TRunnable& Other) const { return this == &Other; }
	};
private:
	class TExecutorThread: public TThread {
	private:
		TThreadExecutor* Executor;
		PRunnable Runnable;
		PNotify Notify;
	public:
		TExecutorThread();
		TExecutorThread(TThreadExecutor* Executor, const PNotify& Notify);

		void Run();
		void SetRunnable(const PRunnable& _Runnable) { Runnable = _Runnable; };
	};

private:
	typedef TThreadV<TExecutorThread> TExecThreadV;
	typedef TLinkedQueue<PRunnable> TTaskQueue;

	TExecThreadV ThreadV;
	TTaskQueue TaskQ;

	TCondVarLock Lock;

	PNotify Notify;

	volatile bool IsFinished;

public:
	TThreadExecutor(const TInt& PoolSize=1, const PNotify& Notify=TNullNotify::New());

	~TThreadExecutor();

	void Execute(const PRunnable& Runnable);

private:
	PRunnable WaitForTask();
};

#endif

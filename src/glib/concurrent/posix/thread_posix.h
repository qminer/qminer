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

/*
 * thread.h
 *
 *  Created on: May 24, 2010
 *      Author: tadej
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <base.h>
#include <pthread.h>

#define INFINITE TInt::Mx

class TBlocker {
protected:
	pthread_cond_t Event;
	pthread_mutex_t Mutex;

	//HANDLE Event;

public:
	TBlocker();
	~TBlocker();

	void Block(int Msecs = TInt::Mx);
	void Release();
};

/** Critical section */
class TCriticalSection {
protected:
	//CRITICAL_SECTION Cs;
	TCriticalSectionType Type;
	pthread_mutex_t Cs;
	pthread_mutexattr_t CsAttr;

public:
	TCriticalSection(const TCriticalSectionType& _Type = TCriticalSectionType::cstFast);
	~TCriticalSection();

	void Enter();
	bool TryEnter();
	void Leave();
};

////////////////////////////////////////////
// Thread
ClassTP(TThread, PThread)// {
private:
	const static int STATUS_CREATED;
	const static int STATUS_STARTED;
	const static int STATUS_CANCELLED;
	const static int STATUS_FINISHED;

	pthread_t ThreadHandle;
	int ThreadId;

	// Use for interrupting and waiting
	TBlocker* SleeperBlocker;
	TCriticalSection CriticalSection;

	volatile sig_atomic_t Status;

private:
    static void * EntryPoint(void * pArg);
    static void SetFinished(void *pArg);
public:
    TThread();

	TThread(const TThread& Other) {
		operator=(Other);
	}
	TThread& operator =(const TThread& Other);

    virtual ~TThread();

    // starts the thread
    void Start();
    // when started the thread calls this function
    virtual void Run() { printf("empty run\n"); };

    // terminates the thread
    void Cancel();

    // windows thread id
    int GetThreadId() const { return ThreadId; }
    // windows thread handle
    pthread_t GetThreadHandle() const { return ThreadHandle; }

	void Interrupt();
	void WaitForInterrupt(const int Msecs = INFINITE);

	int Join();

	bool IsAlive() const { return Status == STATUS_STARTED; }
	bool IsCancelled() const { return Status == STATUS_CANCELLED; }

	static int GetCoreCount();
};


// waits until the given thread finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWait(TThreadType& Thread, const int& MxMSecs = -1) {
    //DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
	int dwMilliSeconds = (MxMSecs == -1) ? TInt::Mx : MxMSecs;
    const int Res = pthread_join(Thread.GetThreadHandle(), dwMilliSeconds);

    		//WaitForSingleObject(Thread.GetThreadHandle(), dwMilliSeconds);
    return Res;//(Res == WAIT_ABANDONED) || (Res == WAIT_OBJECT_0);
}

// waits until at least one of threads from the given vector of threads
// finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWaitOne(TVec<TThreadType>& ThreadV, const int& MxMSecs = -1) {
    // prepare parameters
    /*const int Threads = ThreadV.Len();
    const int nCount = (int)Threads;
    const int dwMilliSeconds = (MxMSecs == -1) ? TInt::Mx : ((int)MxMSecs);
    // create array
    pthread_t* lpHandles = new pthread_t[Threads];
    for (int ThreadN = 0; ThreadN < Threads; ThreadN++) {
        lpHandles[ThreadN] = ThreadV[ThreadN].GetThreadHandle();
    }
    // wait
    //const int Res = pthread_
    		//WaitForMultipleObjects(
        //nCount, lpHandles, waitAll, dwMilliSeconds);
    // delete array
    for (int ThreadN = 0; ThreadN)
    delete lpHandles;
    // check the output and finish
    if (Res == WAIT_TIMEOUT) { return -1; } // timed-out
    return (int)(Res - WAIT_OBJECT_0); // return the index of finished thread*/
	int ret = 0;
	for (int i = 0;i < ThreadV.Len(); i++) {
		ret = ThreadV[i].Join(MxMSecs);
	}
	return ret;
}


template<class TThreadType>
class TThreadV : public TVec<TThreadType> {
public:
	void StartAll() {
		for (int i = 0; i < this->Len(); i++) {
			(*this)[i].Start();
		}
	}
	int Join() {
		int Status = 0;
		for (int i = 0; i < this->Len(); i++) {
			Status = (*this)[i].Join();//ThreadWait<TThreadType>((*this)[i]);
		}
		return Status;
	}
};

////////////////////////////////////////////
// Mutex
class TMutex {
private:
	pthread_mutex_t MutexHandle;
	pthread_mutexattr_t Attributes;


public:
    TMutex(const bool& LockOnStartP = false);
    ~TMutex();

    // waits so the mutex is released and locks it
    bool Wait(const int& MxMSecs = -1);

	void GetLock();
    // releases the mutex
    bool Release();

    // mutex handle
    pthread_mutex_t GetThreadHandle() const { return MutexHandle; }
};

#endif /* THREAD_H_ */

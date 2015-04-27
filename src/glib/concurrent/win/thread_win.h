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

// win32 impl here
#ifndef THREAD_H_
#define THREAD_H_

////////////////////////////////////////////
// Mutex
class TMutex {
private:
    HANDLE MutexHandle;

public:
    TMutex(const bool& LockOnStartP = false);
    ~TMutex();

    // waits so the mutex is released and locks it
    bool Wait(const int& MxMSecs = -1);
	// lock the mutex
	void GetLock();
    // releases the mutex
    bool Release();

    // windows mutex handle
    HANDLE GetThreadHandle() const { return MutexHandle; }
};

////////////////////////////////////////////
// Critical Section
class TCriticalSection {
private:
	CRITICAL_SECTION Cs;

public:
	TCriticalSection(const TCriticalSectionType& _Type = TCriticalSectionType::cstFast);
	~TCriticalSection();

	// start of critical section
	void Enter();
	// try entering critical section, return false when fail
	bool TryEnter();
	// end of critical section
	void Leave();
};

////////////////////////////////////////////
// Blocker 
class TBlocker {
protected:
	HANDLE Event;

public:
	TBlocker();
	~TBlocker();
	
	void Block(int MxMSecs = -1);
	void Release();
};

////////////////////////////////////////////
// Thread
ClassTP(TThread, PThread)// {
protected:
    HANDLE ThreadHandle;
    DWORD ThreadId;

private:
    static DWORD WINAPI EntryPoint(LPVOID pArg);

public:
    TThread();
	TThread(const TThread& Other) { operator=(Other); }

    virtual ~TThread();

	TThread& operator=(const TThread& Other);

    // starts the thread
    void Start();
    // when started the thread calls this function
    virtual void Run() { printf("empty run\n"); };

    // windows thread id
    int GetThreadId() const { return (int)ThreadId; }
    // windows thread handle
    HANDLE GetThreadHandle() const { return ThreadHandle; }

	// join with this thread (wait for it to finish)
	int Join();
	// sleep for given amount of miliseconds
	void Sleep(const int& MSecs);

	// get number of cores in the system
	static int GetCoreCount();
};

// waits until the given thread finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWait(TThreadType& Thread, const int& MxMSecs = -1) {
    DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
    const DWORD Res = WaitForSingleObject(Thread.GetThreadHandle(), dwMilliSeconds);
    return (Res == WAIT_ABANDONED) || (Res == WAIT_OBJECT_0);
}

// waits until at least one of threads from the given vector of threads
// finishes the execution
// - optionaly can be given a maximal number of miliseconds to wait
template <class TThreadType>
int ThreadWaitOne(TVec<TThreadType>& ThreadV, const int& MxMSecs = -1) {
    // prepare parameters
    const int Threads = ThreadV.Len();
    const DWORD nCount = (DWORD)Threads;
    const DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
    // create array
    HANDLE* lpHandles = new HANDLE[Threads];
    for (int ThreadN = 0; ThreadN < Threads; ThreadN++) {
        lpHandles[ThreadN] = ThreadV[ThreadN].GetThreadHandle();
    }
    // wait
    const DWORD Res = WaitForMultipleObjects(
        nCount, lpHandles, FALSE, dwMilliSeconds);
    // delete array
    delete lpHandles;
    // check the output and finish
    if (Res == WAIT_TIMEOUT) { return -1; } // timed-out
    return (int)(Res - WAIT_OBJECT_0); // return the index of finished thread
}

////////////////////////////////////////////
// Thread-Vector
template<class TThreadType>
class TThreadV : public TVec<TThreadType> {
public:
	void StartAll() {
		for (int i = 0; i < Len(); i++) {
			(*this)[i].Start();
		}
	}
	int Join() {
		int Status = 0;
		for (int i = 0; i < Len(); i++) {
			Status = ThreadWait<TThreadType>((*this)[i]);
		}
		return Status;
	}
};

////////////////////////////////////////////
// Conditional variable lock
// blocks threads on a until a condition is fulfilled
class TCondVarLock {
private:
	//pthread_cond_t CondVar;
	TMutex Mutex;

public:
	TCondVarLock();
	~TCondVarLock();

	// locks the mutex
	void Lock();
	// releases the mutex
	bool Release();
	// must be locked before calling this method
	// unlocks the mutex and waits for a signal once it gets the signal the
	// mutex is automatically locked
	void WaitForSignal();
	// must be locked before signaling
	// unblocks at least one thread waiting on the
	// conditional variable
	void Signal();
	// must be locked before broadcasting
	// unblocks all threads waiting on the
	// conditional variable
	void Broadcast();
};

#endif /* THREAD_H_ */

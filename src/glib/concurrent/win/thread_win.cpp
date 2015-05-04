/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/*
 * thread.cpp
 *
 *  Created on: May 24, 2010
 *      Author: tadej
 */
#include "thread_win.h"

////////////////////////////////////////////
// Mutex
TMutex::TMutex(const bool& LockOnStartP) {
    MutexHandle = CreateMutex(NULL, LockOnStartP ? TRUE : FALSE, NULL);
}

TMutex::~TMutex() {
    CloseHandle(MutexHandle);
}

bool TMutex::Wait(const int& MxMSecs) {
    DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
    const DWORD Res = WaitForSingleObject(MutexHandle, dwMilliSeconds);
    return (Res == WAIT_ABANDONED) || (Res == WAIT_OBJECT_0);
}

void TMutex::GetLock() {
	while (!Wait(1000)) { }
}

bool TMutex::Release() {
    return ReleaseMutex(MutexHandle) != 0;
}

////////////////////////////////////////////
// Critical Section
TCriticalSection::TCriticalSection(const TCriticalSectionType& _Type) {
	//TODO: add support for other types
	Assert(_Type == TCriticalSectionType::cstFast);
	InitializeCriticalSection(&Cs);
}
TCriticalSection::~TCriticalSection() {
	DeleteCriticalSection(&Cs);
}
void TCriticalSection::Enter() {
	EnterCriticalSection(&Cs);
}
bool TCriticalSection::TryEnter() {
	return TryEnterCriticalSection(&Cs) != 0;
}
void TCriticalSection::Leave() {
	LeaveCriticalSection(&Cs);
}

////////////////////////////////////////////
// Blocker 
TBlocker::TBlocker() {
	Event = CreateEvent(
        NULL,	// default security attributes
        false,  // auto-reset event
        false,  // initial state is nonsignaled
        NULL);	// object name
	IAssert(Event != NULL);
}

TBlocker::~TBlocker() {
	if (Event != NULL) {
		CloseHandle(Event);
	}
}

void TBlocker::Block(int MxMSecs) {
	if (Event != NULL) {
	    DWORD dwMilliSeconds = (MxMSecs == -1) ? INFINITE : ((DWORD)MxMSecs);
		switch (WaitForSingleObject(Event, dwMilliSeconds))
		{
			// Event object was signaled
			case WAIT_OBJECT_0:
				ResetEvent(Event);
				break;
			// An error occurred
			default:
				EFailR(TStr::Fmt("Thread wait error: %d\n", GetLastError()));
		}
	}
}

void TBlocker::Release() {
	SetEvent(Event);
}

////////////////////////////////////////////
// Thread
DWORD WINAPI TThread::EntryPoint(LPVOID pArg) {
    TThread* pThis = (TThread*)pArg;
    pThis->Run();
    return 0;
}

TThread::TThread(): ThreadHandle(NULL), ThreadId(0) { }

TThread::~TThread() {
    if (ThreadHandle != NULL) {
        CloseHandle(ThreadHandle);
    }
}

TThread &TThread::operator =(const TThread& Other) {
	ThreadHandle = Other.ThreadHandle;
	ThreadId = Other.ThreadId;
	return *this;
}

void TThread::Start() {
    // create new thread
    ThreadHandle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size
        EntryPoint,             // thread function
        this,                   // argument to thread function
        0,                      // use default creation flags
        &ThreadId);             // returns the thread identifier

    // make sure all ok so far...
    EAssert(ThreadHandle != NULL);
}

int TThread::Join() {
	return ThreadWait<TThread>(*this);
}

void TThread::Sleep(const int& MSecs) {
	TSysProc::Sleep(MSecs);
}

int TThread::GetCoreCount() {
	int NumCpu;
	//Blaz: we know it's widnows, since it's in thread_win.cpp :-)
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	NumCpu = sysinfo.dwNumberOfProcessors;
	return NumCpu;
}

////////////////////////////////////////////
// Conditional variable lock
TCondVarLock::TCondVarLock():
	Mutex(TMutexType::mtRecursive) {}

TCondVarLock::~TCondVarLock() {
	// pthread_cond_destroy should be called to free a condition variable that is no longer needed
//	pthread_cond_destroy(&CondVar);
}

void TCondVarLock::Lock() {
	Mutex.GetLock();
}

bool TCondVarLock::Release() {
	return Mutex.Release();
}

void TCondVarLock::WaitForSignal() {
//	pthread_cond_wait(&CondVar, &Mutex.MutexHandle);
}

void TCondVarLock::Signal() {
//	pthread_cond_signal(&CondVar);
}

void TCondVarLock::Broadcast() {
//	pthread_cond_broadcast(&CondVar);
}

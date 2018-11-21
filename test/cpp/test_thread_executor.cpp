#include <base.h>
#include <mine.h>
#include <thread.h>
#include "microtest.h"

#ifdef GLib_UNIX

class TestRunnable : public TThreadPool::TRunnable {
public:
    int Input;
    int* Output;
    TestRunnable(const int& Input, int* Output): Input(Input), Output(Output) { }
    void Run() { *Output = Input + 1; }
    ~TestRunnable() {}
};

class TestSlowRunnable : public TThreadPool::TRunnable {
public:
    int Input;
    uint MSecBefore;
    uint MSecAfter;
    TIntV* Output;
    TCriticalSection& CriticalSection;
    TestSlowRunnable(const int& Input, const uint& MSecBefore, const uint& MSecAfter, TIntV* Output, TCriticalSection& CriticalSection):
      Input(Input), MSecBefore(MSecBefore), MSecAfter(MSecAfter), Output(Output), CriticalSection(CriticalSection) { }
    void Run() {
        TSysProc::Sleep(MSecBefore);
        // todo lock
        CriticalSection.Enter();
        Output->Add(Input);
        CriticalSection.Leave();
        TSysProc::Sleep(MSecAfter);
    }
    ~TestSlowRunnable() {}
};

TEST(constructor_destructor) {
    TThreadPool ThreadPool(1);
}

TEST(increment1) {
    TThreadPool ThreadPool(1);
    int Result;
    TestRunnable* Runnable = new TestRunnable(1, &Result);
    ThreadPool.Execute(Runnable);
    TSysProc::Sleep(50);
    ASSERT_EQ(Result, 2);
}

TEST(two_slow) {
    TThreadPool ThreadPool(2);
    TIntV Result;
    TCriticalSection CriticalSection;
    TestSlowRunnable* Runnable1 = new TestSlowRunnable(1, 0, 0, &Result, CriticalSection);
    TestSlowRunnable* Runnable2 = new TestSlowRunnable(2, 50, 0, &Result, CriticalSection);
    ThreadPool.Execute(Runnable2);
    ThreadPool.Execute(Runnable1);
    TSysProc::Sleep(100);
    ASSERT_EQ(Result.Len(), 2);
    ASSERT_EQ(Result[0], 1);
    ASSERT_EQ(Result[1], 2);
}


TEST(two_race) {
    PNotify Notify = TStdNotify::New();
    //PNotify Notify = TNullNotify::New();
    TThreadPool ThreadPool(2, Notify);
    TIntV Result;
    TCriticalSection CriticalSection;
    TestSlowRunnable* Runnable1 = new TestSlowRunnable(1, 0, 0, &Result, CriticalSection);
    TestSlowRunnable* Runnable2 = new TestSlowRunnable(2, 0, 0, &Result, CriticalSection);
    ThreadPool.Execute(Runnable2);
    ThreadPool.Execute(Runnable1);
    TSysProc::Sleep(100);
    ASSERT_EQ(Result.Len(), 2);
}

#endif

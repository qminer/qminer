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

#ifndef TASK_H
#define TASK_H

#include <base.h>
#include "thread.h"


/** Functor class. */
template<class InItem, class OutItem>
class TTask {
public:
	virtual void Each(const InItem& It, OutItem& Out, int Index) = 0;
};

/** Functor class with partial per-thread buckets.
Enables having partial results for each thread and combining them in the end. */
template<class InItem, class OutItem>
class TAggregationTask : public TTask<InItem, OutItem> {
protected:
	int ItemsPerThread;
	TVec<OutItem> Partials;
	OutItem Aggregate;

	/** Access partial result for current thread */
	inline OutItem& GetPartial(int Index) {
		//printf("%d %d %d %d\n", Index, ItemsPerThread, Index % ItemsPerThread, Index / ItemsPerThread);
		return Partials[Index / ItemsPerThread];
	}

public:
	TAggregationTask(int TotalItems) {
		int ThreadNo = TThread::GetCoreCount();
		Partials.Gen(ThreadNo);
		/*for (int i = 0; i < Partials.Len(); i++) {
			Partials[i] = OutItem();
		}*/
		ItemsPerThread = (TotalItems / ThreadNo) + 1;
	}

	virtual OutItem& GetAggregateResult() = 0;
};



/** A Job class, describing the processing of a specific subsegment of items. */
template<class InItem, class OutItem>
class TJob {
private: 
	TCRef CRef; 
	friend class TPt<TJob<InItem, OutItem> >;
public:
	// What do we do?
	TTask<InItem, OutItem> *Task;
	// Ptr to input collection
	TVec<InItem> const *Items;
	// Ptr to output collection
	TVec<OutItem> *Outputs;
	// index of segment start
	int IndexOffset;
	// index of segment end
	int IndexEnd;
	// End signal
	TBlocker Finished;

	static TPt<TJob<InItem, OutItem> > New() { 
		return TPt<TJob<InItem, OutItem> >(new TJob<InItem, OutItem>()); 
	}

	static TPt<TJob<InItem, OutItem> > New(TTask<InItem, OutItem> *T, TVec<InItem> const *I, TVec<OutItem> *O) { 
		return TPt<TJob<InItem, OutItem> >(new TJob<InItem, OutItem>(T,I,O)); 
	}

	TJob() : IndexOffset(0) {}
	TJob(TTask<InItem, OutItem> *T, TVec<InItem> const  *I, TVec<OutItem> *O) :
		 Task(T), Items(I), Outputs(O),IndexOffset(0), IndexEnd(I->Len()) {
	}

};

/** Job processing thread. Should be running continuously. 
	Input is fed via AddJob, IsFinished signals the end of queue processing. */
template<class InItem, class OutItem>
class TJobThread: public TInterruptibleThread { 
private: 
	TCRef CRef; 
public: 
	friend class TPt<TJobThread<InItem, OutItem> >;
protected:
	TLst<TPt<TJob<InItem, OutItem> > > Q;
	TPt<TJob<InItem, OutItem> > CurrentJob;
	TCriticalSection Critical;
	volatile bool StopFlag;
	volatile bool InProgress;

public:

	TJobThread() : TInterruptibleThread(), StopFlag(false), InProgress(false) {}
	~TJobThread() {
		Interrupt();
	}

	static TPt<TJobThread<InItem, OutItem> > New() { return TPt<TJobThread<InItem, OutItem> >(new TJobThread<InItem, OutItem>()); }

	virtual void Run() {
		while (!StopFlag) {
			// Who disturbs my slumber?
			WaitForInterrupt();

			Critical.Enter();
			if (Q.Empty()) {
				// False alarm
				Critical.Leave();
				continue;
			}

			CurrentJob = Q.Last()->GetVal();
			InProgress = true;
			Critical.Leave();
			
			// Make shortcuts
			const TVec<InItem>& InItems = *(CurrentJob->Items);
			TVec<OutItem>& OutItems = *(CurrentJob->Outputs);
			IAssertR(InItems.Len() == OutItems.Len(), "InItems must be same size as OutItems!");
			int Offset = CurrentJob->IndexOffset;
			TTask<InItem, OutItem>& Task = *(CurrentJob->Task);
			// Inner loop!
			/*TVec<InItem>::TIter*/auto i = InItems.GetI(Offset);
			/*TVec<OutItem>::TIter*/auto o = OutItems.GetI(Offset);
			int n = Offset;
			/*const TVec<InItem>::TIter*/auto End = InItems.GetI(CurrentJob->IndexEnd);
			for ( ; i != End; i++) {
				Task.Each(*i, *o, n++);
				o++;
			}

			Critical.Enter();
			InProgress = false;
			// Release block on master thread who is waiting for completion.
			CurrentJob->Finished.Release();
			Q.Del(Q.Last());
			CurrentJob.Clr();
			Critical.Leave();
		}
	}

	/** Feed queue */
	void AddJob(const TPt<TJob<InItem, OutItem> >& Job) {
		Critical.Enter();
		Q.AddFront(Job);
		Critical.Leave();
		// You've got mail.
		Interrupt();
	}

	/** Kill after you finish processing the current item. */
	void Stop() {
		Critical.Enter();
		StopFlag = true;
		Critical.Leave();
		Interrupt();
	}


	bool Empty() {
		bool Empty = false;
		Critical.Enter();
		Empty = Q.Empty();
		Critical.Leave();
		return Empty;
	}

	bool IsFinished() {
		bool Finished = false;
		Critical.Enter();
		Finished = Q.Empty() && !InProgress;
		Critical.Leave();
		return Finished;
	}


	void WaitForTask(int TaskId = 0) {
		// Block for InProgress and CurrentJob pointer creation (should be fast)
		Critical.Enter();
		if (InProgress) {
			TPt<TJob<InItem, OutItem> > CurrentJob = Q.Last()->GetVal();

			// Do not block on critical section while waiting for item to process!
			Critical.Leave();

			// Block for job completion (this is job-dependent)
			CurrentJob->Finished.Block();

			// Block again to prevent refcount massacre (should be fast)
			Critical.Enter();
			CurrentJob.Clr();
			
		}
		Critical.Leave();	
	}
		
};

/** Thread pool manager. Takes care of splitting the job across threads. */
template<class InItem, class OutItem>
class TJobThreadPool : public TVec<TPt<TJobThread<InItem, OutItem> > > {
public:

	TJobThreadPool() { }
	TJobThreadPool(int ThreadNo) : TVec<TPt<TJobThread<InItem, OutItem> > >(ThreadNo)  {
		for (int i = 0; i < ThreadNo; i++) {
			(*this)[i] = TJobThread<InItem, OutItem>::New();
		}
	}

	void AddJob(TPt<TJob<InItem, OutItem> > Job){
		int AllItems = Job->Items->Len();
		int ItemsPerThread = AllItems / Len() + 1;
		int Start = 0;
		int End = ItemsPerThread;
		TVec<TPt<TJob<InItem, OutItem> > > SubJobs(Len());
		
		for (int i = 0; i < MIN(Len(), AllItems); i++) {
			TPt<TJob<InItem, OutItem> > SubJob = TJob<InItem, OutItem>::New(Job->Task, Job->Items, Job->Outputs);
			SubJob->IndexOffset = Start;
			SubJob->IndexEnd = MIN(AllItems, End);
			//printf("setting up job from %d to %d of %d\n", Start, SubJob->IndexEnd, AllItems);
			SubJobs[i] = SubJob;
			Start = End;
			End += ItemsPerThread;
			(*this)[i]->AddJob(SubJob);
		}
	}

	bool IsFinished() {
		for (int i = 0; i < Len(); i++) {
			if (!(*this)[i]->IsFinished()) {
				return false;
			}
		}
		return true;
	}

	void WaitForTasks(int TaskId = 0) {
		for (int i = 0; i < Len(); i++) {
			(*this)[i]->WaitForTask(TaskId);
		}
	}
};

/** Simple for-each loop, no threads involved. */
template<class InItem, class OutItem>
class TLoops {
public:
	virtual void ForEach(const TVec<InItem>& Items, TVec<OutItem>& Output, TTask<InItem, OutItem>& DoThis) {
		ExecuteAndForget(Items, Output, DoThis);
		WaitForCompletion();
	}

	virtual void ExecuteAndForget(const TVec<InItem>& Items, TVec<OutItem>& Outputs, TTask<InItem, OutItem>& DoThis) {
		for (int i = 0; i < Items.Len(); i++) {
			DoThis.Each(Items[i], Outputs[i], i);
		}
	}

	virtual void WaitForCompletion() {
	}

};

/** Parallel for-each loob with its own thread manager. Reusing is encouraged. */
template<class InItem, class OutItem>
class TParallel : public TLoops<InItem, OutItem> {
protected:
	TJobThreadPool<InItem, OutItem> Threads;

public:

	TParallel(int NumCpu, bool DoStart = true)  {
		Threads = TJobThreadPool<InItem, OutItem>(NumCpu);
		if (DoStart) {
			Start();
		}
	}

	TParallel(bool DoStart = true)   {
		int NumCpu = TThread::GetCoreCount();
		Threads = TJobThreadPool<InItem, OutItem>(NumCpu);
		if (DoStart) {
			Start();
		}
	}

	~TParallel() {
		for (int i = 0; i < Threads.Len(); i++) {
			Threads[i]->Stop();
		}
		for (int i = 0; i < Threads.Len(); i++) {
			Threads[i]->Interrupt();
			Threads[i]->Join();
		}
	}

	void Start() {
		for (int i = 0; i < Threads.Len(); i++) {
			Threads[i]->Start();
		}
	}

	int GetThreadNo() const {
		return Threads.Len();
	}

	virtual void ForEach(const TVec<InItem>& Items, TVec<OutItem>& Outputs, TTask<InItem, OutItem>& DoThis) {
		ExecuteAndForget(Items, Outputs, DoThis);
		WaitForCompletion();

	}

	virtual void ExecuteAndForget(const TVec<InItem>& Items, TVec<OutItem>& Outputs, TTask<InItem, OutItem>& DoThis) {
		TPt<TJob<InItem, OutItem> > Job = TJob<InItem, OutItem>::New(&DoThis, &Items, &Outputs);
		Threads.AddJob(Job);
	}

	virtual void WaitForCompletion() {
		Threads.WaitForTasks();
		while(!Threads.IsFinished()) { TSysProc::Sleep(1); }
	}

};


#endif

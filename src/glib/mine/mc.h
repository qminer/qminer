/*
 * denseclust.h
 *
 *  Created on: Sep 17, 2014
 *      Author: lstopar
 */

#ifndef CTMC_H_
#define CTMC_H_

namespace TMc {

using namespace TMl;

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TEuclDist {
public:
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist(const TFullMatrix& X, const TFullMatrix& Y) { return GetDist2(X, Y).Sqrt(); }
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist2(const TFullMatrix& X, const TFullMatrix& Y);
};

class TEuclMds {
public:
	// projects the points stored in the column of X onto d
	// dimensions
	static TFullMatrix Project(const TFullMatrix& X, const int& d=2);
};

//class TClust;
//typedef TPt<TClust> PClust;
//class TClust {
//private:
//  TCRef CRef;
//public:
//  friend class TPt<TClust>;
//protected:
//    const static int MX_ITER;
//
//    typedef TPair<TUInt64, TUInt64V> TFtrHistStat;
//    typedef TVec<TFtrHistStat> TClustHistStat;
//    typedef TVec<TClustHistStat> THistStat;
//
//	TRnd Rnd;
//	// holds centroids as column vectors
//	TFullMatrix CentroidMat;
//	// holds pairs <n,sum> where n is the number of points assigned to the
//	// centroid at index i and sum is the sum of distances of all the points
//	// assigned to the centroid to the centroid
//	TUInt64FltPrV CentroidDistStatV;
//
//	int NHistBins;			// the number of bins used in a histogram
//	TFltVV FtrBinStartVV;		// stores where each bin starts	// TODO save/load
//	THistStat HistStat;		// stores histogram for every feature in every cluster
//
//	double Sample;
//
//	bool Verbose;
//	PNotify Notify;
//
//	TClust(const int NHistBins, const double& Sample, const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
//	TClust(TSIn& SIn);
//
//	virtual ~TClust() {}
//
//public:
//	// saves the model to the output stream
//	virtual void Save(TSOut& SOut) const;
//	// loads the model from the output stream
//	static PClust Load(TSIn& SIn);
//
//	// performs the clustering
//	void Init(const TFullMatrix& X);
//	// initializes histograms for every feature
//	void InitHistogram(const TFullMatrix& X);
//
//	// assign methods
//	// assign instances to centroids
//	int Assign(const TVector& Inst) const;
//	// assign instances to centroids, instances should be in the columns of the matrix
//	TVector Assign(const TFullMatrix& InstMat) const;
//	void Assign(const TFullMatrix& InstMat, TIntV& AssignV) const;
//
//	// distance methods
//	// returns a matrix D with the distance to all the centroids
//	// D_ij is the distance between centroid i and instance j
//	// points should be represented as columns of X
//	TFullMatrix GetDistMat(const TFullMatrix& X) const;
//	// Returns a vector y containing the distance to all the
//	// centroids. The input vector x should be a column vector
//	TVector GetDistVec(const TVector& x) const;
//	// returns the distance from the cluster centroid to the point
//	double GetDist(const int& CentroidId, const TVector& Pt) const;
//
//	// returns the coordinates of a "joined" centroid
//	TVector GetJoinedCentroid(const TIntV& CentroidIdV) const;
//
//	// cluster statistics
//	// returns the means distance of all the points assigned to centroid CentroidIdx
//	// to that centroid
//	double GetMeanPtCentDist(const int& CentroidId) const;
//	// returns the number of points in the cluster
//	uint64 GetClustSize(const int& ClustId) const;
//
//	void GetHistogram(const int FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const;
//
//	int GetClusts() const { return CentroidMat.GetCols(); }
//	int GetDim() const { return CentroidMat.GetRows(); }
//	const TFullMatrix& GetCentroidMat() const { return CentroidMat; }
//
//	// sets the log to verbose or none
//	void SetVerbose(const bool& Verbose);
//
//protected:
//	// Applies the algorithm. Instances should be in the columns of X.
//	virtual void Apply(const TFullMatrix& X, const int& MaxIter=10000) = 0;
//	TVector Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;
//	// returns a matrix of squared distances
//	TFullMatrix GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;
//
//	// used during initialization
//	TFullMatrix SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV);
//	void UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV);
//	void InitStatistics(const TFullMatrix& X, const TVector& AssignV);
//
//	// returns the type of this clustering
//	virtual const TStr GetType() const = 0;
//
//private:
//	// returns the coordinates of the centroid with the specified ID
//	TVector GetCentroid(const int& CentroidId) const;
//};


class TAvgLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i, const int& j);
};

class TCompleteLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i, const int& j);
};

class TSingleLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i, const int& j);
};

template <class TLink>
class TAggClust {
public:
	static void MakeDendro(const TFullMatrix& X, TIntIntFltTrV& MergeV, const PNotify& Notify) {
		const int NInst = X.GetCols();

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "%s\n", TStrUtil::GetStr(X.GetMat(), ", ", "%.3f").CStr());

//		TFullMatrix X1 = X;	// copy

		TFullMatrix ClustDistMat = TEuclDist::GetDist2(X,X);
		TVector ItemCountV = TVector::Ones(NInst);

		for (int k = 0; k < NInst-1; k++) {
			// find active <i,j> with minimum distance
			int MnI = -1;
			int MnJ = -1;
			double MnDist = TFlt::PInf;

			// find clusters with min distance
			for (int i = 0; i < NInst; i++) {
				if (ItemCountV[i] == 0.0) { continue; }

				for (int j = i+1; j < NInst; j++) {
					if (i == j || ItemCountV[j] == 0.0) { continue; }

					if (ClustDistMat(i,j) < MnDist) {
						MnDist = ClustDistMat(i,j);
						MnI = i;
						MnJ = j;
					}
				}
			}

			double Dist = sqrt(MnDist < 0 ? 0 : MnDist);
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Merging clusters %d, %d, distance: %.3f", MnI, MnJ, Dist);
			// merge
			MergeV.Add(TIntIntFltTr(MnI, MnJ, Dist));

			TLink::JoinClusts(ClustDistMat, ItemCountV, MnI, MnJ);

			// update counts
			ItemCountV[MnI] = ItemCountV[MnI] + ItemCountV[MnJ];
			ItemCountV[MnJ] = 0;
		}
	}
};

typedef TAggClust<TAvgLink> TAlAggClust;
typedef TAggClust<TCompleteLink> TClAggClust;
typedef TAggClust<TCompleteLink> TSlAggClust;

class THierarch;
typedef TPt<THierarch> PHierarch;
class THierarch {
private:
	TCRef CRef;
public:
	friend class TPt<THierarch>;
private:
    // a vector which describes the hierarchy. each state has its own index
    // and the value at index i is the index of i-ths parent
    TIntV HierarchV;

    // state heights in the hierarchy
    TFltV StateHeightV;
    TFlt MxHeight;

    // past states
    int HistCacheSize;
    TVec<TIntV> PastStateIdV;	// TODO not optimal structure

    TFltPrV StateCoordV;
    // number of leaf states, these are stored in the first part of the hierarchy vector
    int NLeafs;

    TStrV StateNmV;

    bool Verbose;
    PNotify Notify;

public:
    THierarch(const bool& HistCacheSize, const bool& Verbose=false);
    THierarch(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PHierarch Load(TSIn& SIn);

	void Init(const TFullMatrix& CentroidMat, const int& CurrLeafId);
	void UpdateHistory(const int& CurrLeafId);

	// returns a vector of unique heights
	void GetUniqueHeightV(TFltV& HeightV) const;
	// returns the 'joined' states at the specified height, puts teh state IDs into StateIdV
	// and sets of their leafs into JoinedStateVV
	void GetStateSetsAtHeight(const double& Height, TIntV& StateIdV, TVec<TIntV>& StateSetV) const;
	// returns all the states just below the specified height
	void GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const;
	// fills the vector with IDs of the ancestors of the given state along with their heights
	void GetAncestorV(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns the ID of the ancestor of the given leaf at the specified height
	int GetAncestorAtHeight(const int& LeafId, const double& Height) const;
	// fills the vector with leaf descendants
	void GetLeafDescendantV(const int& StateId, TIntV& DescendantV) const;

	void GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	void GetHistStateIdV(const double& Height, TIntV& StateIdV) const;

	// returns the coordinates of the specified state
	const TFltPr& GetStateCoords(const int& StateId) const { return StateCoordV[StateId]; }
	// returns the total number of states in the hierarchy
	int GetStates() const { return HierarchV.Len(); }
	// returns the number of leafs in the hierarchy
	int GetLeafs() const { return NLeafs; }

	bool IsStateNm(const int& StateId) const;
	void SetStateNm(const int& StateId, const TStr& StateNm);
	const TStr& GetStateNm(const int& StateId) const;

	void SetVerbose(const bool& Verbose);
	void PrintHierarch() const;

private:
	// returns the ID of the parent state
	int GetParentId(const int& StateId) const;
	// returns the height of the state
	double GetStateHeight(const int& StateId) const { return StateHeightV[StateId]; }
	int GetNearestHeightIdx(const double& Height) const;

	bool IsRoot(const int& StateId) const;
	bool IsLeaf(const int& StateId) const;
	bool IsOnHeight(const int& StateId, const double& Height) const;
	bool IsBelowHeight(const int& StateId, const double& Height) const;
	bool IsAboveHeight(const int& StateId, const double& Height) const;

	// returns a hash table with keys being the states at the specified height
	// and the values containing their successor leafs
	void GetAncSuccH(const double& Height, TIntIntVH& StateSubStateH) const;

	// internal methods used during initialization
	// returns the index of the oldest ancestor of the state
	// this method is only used when initially building the hierarchy
	int GetOldestAncestIdx(const int& StateIdx) const;
	// for each state returns the number of leafs it's subtree has
	void GetLeafSuccesorCountV(TIntV& LeafCountV) const;
	// computes the coordinates (in 2D) of each state
	void ComputeStateCoords(const TFullMatrix& CentroidMat, const int& NStates);

	// static functions
	static TInt& GetParentId(const int& StateId, TIntV& HierarchV) { return HierarchV[StateId]; }
	static int GetParentId(const int& StateId, const TIntV& HierarchV) { return HierarchV[StateId]; }
	static int GetGrandparentId(const int& StateId, const TIntV& HierarchV) { return GetParentId(GetParentId(StateId, HierarchV), HierarchV); }
	static bool IsRoot(const int& StateId, const TIntV& HierarchV);

	// clears the state
	void ClrFlds();
};

/////////////////////////////////////////////////////////////////
// Markov Chain
class TMChain;
typedef TPt<TMChain> PMChain;
class TMChain {
private:
	TCRef CRef;
public:
	friend class TPt<TMChain>;
protected:
	int NStates;
	int CurrStateId;

	bool Verbose;

	PNotify Notify;

	// constructors
	TMChain(const bool& Verbose);
	TMChain(TSIn& SIn);

	// destructor
	virtual ~TMChain() {}

public:
	// save / load
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PMChain Load(TSIn& SIn);

	// initializes the markov chain
	void Init(const int& NStates, const TIntV& StateAssignV, const TUInt64V& TmV);
	// adds a single record to the model, the flag UpdateStates indicates if the statistics
	// should be updated
	void OnAddRec(const int& StateId, const uint64& RecTm, const bool UpdateStats=true);

	// get future state probabilities for a fixed time in the future
	void GetFutureProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, const double& Tm, TIntFltPrV& StateIdProbV) const;
	// get past state probabilities for a fixed time in the past
	void GetPastProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, const double& Tm, TIntFltPrV& StateIdProbV) const;
	// returns the most likely next states, excluding the current state,
	// along with probabilities of going into those states
	virtual void GetNextStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const = 0;
	// returns the most likely previous states, excluding the current state,
	// along with probabilities of going into those states
	virtual void GetPrevStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const = 0;

	virtual void GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm, const double EndTm, const double& DeltaTm,
			const TVec<TIntV>& StateSetV, const TIntV& StateIdV, TVec<TFltV>& FutProbVV, TVec<TFltV>& PastProbVV) const = 0;

	// static distribution
	// returns the static distribution for the joined states
	virtual TVector GetStatDist(const TVec<TIntV>& StateSetV) const = 0;
	// returns the static distribution
	virtual TVector GetStatDist() const = 0;

	// returns a vector of state sizes
	virtual TVector GetStateSizeV(const TVec<TIntV>& StateSetV) const = 0;
	virtual TFullMatrix GetTransitionMat(const TVec<TIntV>& StateSetV) const = 0;
	virtual TFullMatrix GetModel(const TVec<TIntV>& StateSetV) const = 0;

	virtual TVector GetHoldingTimeV(const TVec<TIntV>& StateSetV) const = 0;

	// returns the number of states
	int GetStates() const { return NStates; };
	// returns the ID of the current state
	int GetCurrStateId() const { return CurrStateId; };

	// returns true is the jump from OldStateId to NewStateId is considered anomalous
	virtual bool IsAnomalousJump(const int& NewStateId, const int& OldStateId) const = 0;

	void SetVerbose(const bool& Verbose);

protected:
	// initializes the statistics
	virtual void InitStats(const int& NStates) = 0;
	virtual void AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool UpdateStats) = 0;

	// get future state probabilities for all the states for a fixed time in the future
	virtual TFullMatrix GetFutureProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const = 0;
	// get [ast state probabilities for all the states for a fixed time in the past
	virtual TFullMatrix GetPastProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const = 0;

	static void GetFutureProbVOverTm(const TFullMatrix& PMat, const int& StateIdx,
			const int& Steps, TVec<TFltV>& ProbVV, const PNotify& Notify,
			const bool IncludeT0=true);

	virtual void PrintStats() const = 0;
	virtual const TStr GetType() const = 0;
};

/////////////////////////////////////////////////////////////////
// Discrete time Markov Chain
class TDtMChain: public TMChain {
private:
	TFullMatrix JumpCountMat;

public:
	TDtMChain(const bool& Verbose=false);
	TDtMChain(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// returns the most likely next states excluding the current state
	void GetNextStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const;
	// returns the most likely previous states excluding the current state
	void GetPrevStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const;

	void GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm, const double EndTm, const double& DeltaTm, const TVec<TIntV>& StateSetV, const TIntV& StateIdV, TVec<TFltV>& FutProbVV, TVec<TFltV>& PastProbVV) const { throw TExcept::New("Not implemented!!!", "here"); }

	// static distribution
	TVector GetStatDist(const TVec<TIntV>& StateSetV) const { return GetStatDist(GetTransitionMat(StateSetV)); }
	TVector GetStatDist() const { return GetStatDist(GetTransitionMat()); }

	TVector GetStateSizeV(const TVec<TIntV>& StateSetV) const { return GetStatDist(StateSetV); }
	TFullMatrix GetTransitionMat(const TVec<TIntV>& StateSetV) const;
	TFullMatrix GetModel(const TVec<TIntV>& StateSetV) const { return GetTransitionMat(StateSetV); };

	TVector GetHoldingTimeV(const TVec<TIntV>& StateSetV) const { throw TExcept::New("Not implemented!", "GetHoldingTimeV"); }

	// returns true if the jump from OldStateId to NewStateId has a low enough probability
	bool IsAnomalousJump(const int& NewStateId, const int& OldStateId) const;

protected:
	// initializes the statistics needed to model the Markov chain
	void InitStats(const int& NStates);
	void AbsOnAddRec(const int& StateIdx, const uint64& RecTm, const bool UpdateStats);

	// get future state probabilities for all the states for a fixed number of states in the future
	TFullMatrix GetFutureProbMat(const TVec<TIntV>& StateSetV, const double& TimeSteps) const;
	TFullMatrix GetPastProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const;

	void PrintStats() const { /* TODO */ }
	const TStr GetType() const { return "discrete"; }

private:
	TFullMatrix GetTransitionMat() const;

	static TVector GetStatDist(const TFullMatrix& PMat);
};

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtMChain: public TMChain {
public:
	const static uint64 TU_SECOND;
	const static uint64 TU_MINUTE;
	const static uint64 TU_HOUR;
	const static uint64 TU_DAY;

private:
	TVec<TUInt64FltPrV> QMatStats;

	double DeltaTm;

	uint64 TimeUnit;
	uint64 PrevJumpTm;

public:
	TCtMChain(const uint64& TimeUnit, const double& DeltaTm, const bool& Verbose=false);
	TCtMChain(TSIn& SIn);

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// returns the most likely next states excluding the current state
	void GetNextStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const;
	// returns the most likely previous states excluding the current state
	void GetPrevStateProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const;

	void GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm, const double EndTm, const double& DeltaTm, const TVec<TIntV>& StateSetV, const TIntV& StateIdV, TVec<TFltV>& FutProbVV, TVec<TFltV>& PastProbVV) const;

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	TVector GetStatDist() const;
	TVector GetStatDist(const TVec<TIntV>& StateSetV) const;

	// returns the size of each state used in the visualization
	TVector GetStateSizeV(const TVec<TIntV>& StateSetV) const;
	TFullMatrix GetTransitionMat(const TVec<TIntV>& StateSetV) const;
	TFullMatrix GetModel(const TVec<TIntV>& StateSetV) const { return GetQMatrix(StateSetV); }

	TVector GetHoldingTimeV(const TVec<TIntV>& StateSetV) const { return GetHoldingTimeV(GetQMatrix(StateSetV)); }

	// returns true if the jump from OldStateId to NewStateId has a low enough probability
	bool IsAnomalousJump(const int& NewStateId, const int& OldStateId) const;

protected:
	void InitStats(const int& NStates);
	void AbsOnAddRec(const int& StateIdx, const uint64& RecTm, const bool UpdateStats);

	// get future state probabilities for all the states for a fixed time in the future
	TFullMatrix GetFutureProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const;
	TFullMatrix GetPastProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const;

	// prints the statistics used to build the Q-matrix
	void PrintStats() const;
	const TStr GetType() const { return "continuous"; }

private:
	// returns the intensity matrix (Q-matrix)
	TFullMatrix GetQMatrix() const;
	// returns a Q matrix for the joined states
	TFullMatrix GetQMatrix(const TVec<TIntV>& StateSetV) const;
	// returns a Q matrix for the joined states for the time reversal Markov chain
	TFullMatrix GetRevQMatrix(const TVec<TIntV>& StateSetV) const;

	TFullMatrix GetJumpMatrix(const TVec<TIntV>& StateSetV) const { return GetJumpMatrix(GetQMatrix(StateSetV)); }
	// returns a vector of holding times
	// a holding time is the expected time that the process will stay in state i
	// it is an exponential random variable of parameter -q_ii, so its expected value
	// is -1/q_ii
	TVector GetHoldingTimeV(const TFullMatrix& QMat) const;

	static void GetNextStateProbV(const TFullMatrix& QMat, const TIntV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates, const PNotify& Notify);
	static TVector GetStatDist(const TFullMatrix& QMat);
	static TFullMatrix GetFutureProbMat(const TFullMatrix& QMat, const double& Tm, const double& DeltaTm);
	// returns a jump matrix for the given transition rate matrix
	// when the process decides to jump the jump matrix describes to
	// which state it will jump with which probability
	static TFullMatrix GetJumpMatrix(const TFullMatrix& QMat);
};

////////////////////////////////////////////////
// State assistant
class TStateAssist;
typedef TPt<TStateAssist> PStateAssist;
class TStateAssist {
private:
	TCRef CRef;
public:
	friend class TPt<TStateAssist>;
private:
	TVec<TLogReg> ClassifyV;

	TRnd Rnd;

	bool Verbose;
	PNotify Notify;

public:
	TStateAssist(const bool Verbose);
	TStateAssist(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Init(const TFullMatrix& X, const PFullClust& Clust, const PHierarch& Hierarch);
};

class TMcCallback {
public:
	virtual ~TMcCallback() {}

	virtual void OnStateChanged(const TIntFltPrV& StateIdHeightV) = 0;
	virtual void OnAnomaly(const TStr& AnomalyDesc) = 0;
	virtual void OnOutlier(const TFltV& FtrV) = 0;
};

class THierarchCtmc;
typedef TPt<THierarchCtmc> PHierarchCtmc;
class THierarchCtmc {
private:
	TCRef CRef;
public:
	friend class TPt<THierarchCtmc>;
private:
	PFullClust Clust;
    PMChain MChain;
    PHierarch Hierarch;
    PStateAssist StateAssist;

    bool Verbose;

    TMcCallback* Callback;

    PNotify Notify;

public:
    // constructors
    THierarchCtmc();
    THierarchCtmc(const PFullClust& Clust, const PMChain& MChain, const PHierarch& Hierarch,
    		const bool& Verbose=true);
    THierarchCtmc(TSIn& SIn);

    ~THierarchCtmc() {}

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// saves this models as JSON
	PJsonVal SaveJson() const;

	// update methods
	// initializes the model
	void Init(const TFullMatrix& X, const TUInt64V& RecTmV);
	void Init(TFltVV& X, const TUInt64V& RecTmV) { Init(TFullMatrix(X, true), RecTmV); }
	void InitClust(const TFullMatrix& X);
	void InitMChain(const TFullMatrix& X, const TUInt64V& RecTmV);
	void InitHierarch();
	void InitHistograms(TFltVV& InstMat);
	void InitStateAssist(const TFullMatrix& X);

	void OnAddRec(const uint64 RecTm, const TFltV& Rec);

	// future and past probabilities
	// returns the probabilities of future states at time Tm, on the specified level
	// starting from the specified state
	void GetFutStateProbV(const double& Height, const int& StateId, const double& Tm, TIntFltPrV& StateIdProbPrV) const;
	// returns the probabilities of past states at time Tm, on the specified level
	// starting from the specified state
	void GetPastStateProbV(const double& Height, const int& StateId, const double& Tm, TIntFltPrV& StateIdProbPrV) const;
	// returns a distribution of probabilities of the next states
	void GetNextStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const;
	// returns a distribution of probabilities of the previous states
	void GetPrevStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const;

	void GetProbVOverTm(const double& Height, const int& StateId, const double StartTm, const double EndTm, const double& DeltaTm, TIntV& StateIdV, TVec<TFltV>& FutProbV, TVec<TFltV>& PastProbV) const;

	void GetHistStateIdV(const double& Height, TIntV& StateIdV) const;

	void GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const;

	// stores the transition model for the current height into Mat
	void GetTransitionModel(const double& Height, TFltVV& Mat) const;
	// returns a list of ancestor states along with their heights
	void GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns a list of ancestors of the current state
	void GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const;
	// returns the current state on the specified level
	int GetCurrStateId(const double& Height) const;
	// returns the centroid of the given state
	void GetCentroid(const int& StateId, TFltV& FtrV) const;
	// returns the IDs of all the states on the specified height
	void GetStateIdVAtHeight(const double& Height, TIntV& StateIdV) const;
	// returns the number of states in the hierarchy
    int GetStates() const { return Hierarch->GetStates(); }

    // sets the name of the specified state
    void SetStateNm(const int& StateId, const TStr& StateNm);
    const TStr& GetStateNm(const int& StateId) const;

    void SetVerbose(const bool& Verbose);
    void SetCallback(TMcCallback* Callback);

private:
    void DetectAnomalies(const int& NewStateId, const int& OldStateId, const TVector& FtrVec) const;
};

}

#endif /* CTMC_H_ */

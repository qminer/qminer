/*
 * denseclust.h
 *
 *  Created on: Sep 17, 2014
 *      Author: lstopar
 */

#ifndef CTMC_H_
#define CTMC_H_

namespace TMc {

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TEuclDist {
public:
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist(const TFullMatrix& X, const TFullMatrix& Y);
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist2(const TFullMatrix& X, const TFullMatrix& Y);
};

class TClust;
typedef TPt<TClust> PClust;
class TClust {
private:
  TCRef CRef;
public:
  friend class TPt<TClust>;
protected:
	TRnd Rnd;
	// holds centroids as column vectors
	TFullMatrix CentroidMat;
	// holds pairs <n,sum> where n is the number of points assigned to the
	// centroid at index i and sum is the sum of distances of all the points
	// assigned to the centroid to the centroid
	TUInt64FltPrV CentroidDistStatV;

	PNotify Notify;

public:
	TClust(const TRnd& Rnd=TRnd(0), const PNotify& Notify=TNullNotify::New());
	virtual ~TClust() {}

protected:
	TClust(TSIn& SIn);

public:
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PClust Load(TSIn& SIn);

	// Applies the algorithm. Instances should be in the columns of X.
	virtual TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000) = 0;

	int GetClusts() const { return CentroidMat.GetCols(); }
	const TFullMatrix& GetCentroidMat() const { return CentroidMat; }

	// assign instances to centroids
	int Assign(const TVector& Inst) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	TVector Assign(const TFullMatrix& InstMat) const;

	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
	TFullMatrix GetDistMat(const TFullMatrix& X) const;
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	TVector GetDistVec(const TVector& x) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const int& CentroidIdx, const TVector& Pt) const;


	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidIdx) const;
	// returns the number of points in the cluster
	uint64 GetClustSize(const int& ClustIdx) const;

protected:
	TFullMatrix SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV);
	void UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV);

	// returns a matrix of squared distances
	TFullMatrix GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;
	TVector Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;

	void InitStatistics(const TFullMatrix& X, const TVector& AssignV);

	// returns the type of this clustering
	virtual const TStr GetType() const = 0;
};

class TFullKMeans: public TClust {
private:
	TInt K;

public:
	TFullKMeans(const int& K, const TRnd& Rnd=TRnd(0), const PNotify& Notify=TNullNotify::New());
	TFullKMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000);

protected:
	const TStr GetType() const { return "kmeans"; }
};

class TDpMeans: public TClust {
private:
	TFlt Lambda;
	TInt MinClusts;
	TInt MaxClusts;	// TODO max clusts is ignored for now

public:
	TDpMeans(const TFlt& Lambda, const TInt& MinClusts=1, const TInt& MaxClusts=TInt::Mx, const TRnd& Rnd=TRnd(0), const PNotify& Notify=TNullNotify::New());
	TDpMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000);

protected:
	const TStr GetType() const { return "dpmeans"; }
};

class TEuclMds {
public:
	// projects the points stored in the column of X onto d
	// dimensions
	static TFullMatrix Project(const TFullMatrix& X, const int& d=2);
};

class TAggClust {
public:
	static void MakeDendro(const TFullMatrix& X, TIntIntFltTrV& MergeV);
};

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

    TFltPrV StateCoordV;
    // number of leaf states, these are stored in the first part of the hierarchy vector
    int NLeafs;

    PNotify Notify;

public:
    THierarch(const PNotify& Notify=TNullNotify::New());
    THierarch(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PHierarch Load(TSIn& SIn);

	void Init(const TFullMatrix& CentroidMat);

	int GetStates() const { return HierarchV.Len(); }

	// returns the index of the state with the lowest height
	// states with flag set to true are ignored
	int GetLowestHeightIdx(const TBoolV& IgnoreV) const;
	// returns all the states on the specified height
	void GetStatesAtHeight(const double& Height, TIntSet& StateIdxV) const;
	int GetAncestorAtHeight(const int& LeafIdx, const double& Height) const;
	// returns the height of the state
	double GetStateHeight(const int& StateId) const { return StateHeightV[StateId]; }

	// returns the 'joined' states at the specified height, puts teh state IDs into StateIdV
	// and sets of their leafs into JoinedStateVV
	void GetStateSetsAtHeight(const double& Height, TIntV& StateIdV, TVec<TIntV>& JoinedStateVV) const;

	const TFltPr& GetStateCoords(const int& StateIdx) const { return StateCoordV[StateIdx]; }
	const TFlt& GetMxStateHeight() const { return MxHeight; }
	void GetUniqueHeightV(TFltV& UniqueHeightV) const;

private:
	// returns a hash table with keys being the states at the specified height
	// and the values containing their successor leafs
	void GetAncSuccH(const double& Height, TIntIntVH& StateSubStateH) const;

	// returns the index of the oldest ancestor of the state
	// this method is only used when initially building the hierarchy
	int GetOldestAncestIdx(const int& StateIdx) const;


	// for each state returns the number of leafs it's subtree has
	void GetSuccesorCountV(TIntV& SuccesorCountV) const;
	// computes the coordinates (in 2D) of each state
	void ComputeStateCoords(const TFullMatrix& CentroidMat, const int& NStates);
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
	int CurrStateIdx;

	PNotify Notify;

	TMChain(const PNotify& Notify);
	virtual ~TMChain() {}

protected:
	TMChain(TSIn& SIn);

public:
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PMChain Load(TSIn& SIn);

	void OnAddRec(const int& StateIdx, const uint64& RecTm);

	// initializes the markov chain
	void Init(const int& NStates, const TIntV& StateAssignV, const TUInt64V& TmV);

	// returns the total number of stats in the system
	int GetStates() const { return NStates; };

	int GetCurrStateIdx() const { return CurrStateIdx; };

	virtual TVector GetStateSizeV(const TVec<TIntV>& JoinedStateVV) const = 0;

	virtual TFullMatrix GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const = 0;

	// returns the most likely future states, excluding the current state,
	// along with probabilities of going into those states
	virtual void GetLikelyFutureStateV(const TVec<TIntV>& JoinedStateVV, const int& CurrState, const int& NFutStates, TIntV& FutStateIdV, TFltV& FutStateProbV) const = 0;
	// get future state probabilities for all the states for a fixed time in the future
	virtual TFullMatrix GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& Tm) const = 0;
	// get future state probabilities for a fixed time in the future
	TVector GetFutureProbV(const TVec<TIntV>& JoinedStateVV, const int& StateIdx, const double& Tm) const { return GetFutureProbMat(JoinedStateVV, Tm).GetRow(StateIdx); };

	virtual TFullMatrix GetModel(const TVec<TIntV>& JoinedStateVV) const = 0;

protected:
	// initializes the statistics
	virtual void InitStats(const int& NStates) = 0;
	virtual void AbsOnAddRec(const int& StateIdx, const uint64& RecTm) = 0;

	virtual const TStr GetType() const = 0;
};

/////////////////////////////////////////////////////////////////
// Discrete time Markov Chain
class TDtMChain: public TMChain {
private:
	TFullMatrix JumpCountMat;

public:
	TDtMChain(const PNotify& Notify=TNullNotify::New());
	TDtMChain(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	TVector GetStatDist(const TVec<TIntV>& JoinedStateVV) const { return GetStatDist(GetTransitionMat(JoinedStateVV)); }

	TVector GetStateSizeV(const TVec<TIntV>& JoinedStateVV) const { return GetStatDist(JoinedStateVV); }

	TFullMatrix GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const;

	TFullMatrix GetModel(const TVec<TIntV>& JoinedStateVV) const { return GetTransitionMat(JoinedStateVV); };

	// returns the most likely future states excluding the current state
	void GetLikelyFutureStateV(const TVec<TIntV>& JoinedStateVV, const int& CurrState, const int& NFutStates, TIntV& FutStateIdV, TFltV& FutStateProbV) const;
	// get future state probabilities for all the states for a fixed number of states in the future
	TFullMatrix GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& TimeSteps) const;
protected:
	void InitStats(const int& NStates);
	void AbsOnAddRec(const int& StateIdx, const uint64& RecTm);

private:
	TFullMatrix GetTransitionMat() const;
	TVector GetStatDist(const TFullMatrix& PMat) const;
	TVector GetStatDist() const { return GetStatDist(GetTransitionMat()); }

protected:
	const TStr GetType() const { return "discrete"; }
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
	TCtMChain(const uint64& TimeUnit, const double& DeltaTm, const PNotify& Notify=TNullNotify::New());
	TCtMChain(TSIn& SIn);

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	TVector GetStatDist() const;
	// returns a jump matrix for the given transition rate matrix
	// when the process decides to jump the jump matrix describes to
	// which state it will jump with which probability
	TFullMatrix GetJumpMatrix(const TFullMatrix& QMat) const;
	TFullMatrix GetJumpMatrix(const TVec<TIntV>& JoinedStateVV) const { return GetJumpMatrix(GetQMatrix(JoinedStateVV)); }
	// returns a vector of holding times
	// a holding time is the expected time that the process will stay in state i
	// it is an exponential random variable of parameter -q_ii, so its expected value
	// is -1/q_ii
	TVector GetHoldingTimeV(const TFullMatrix& QMat) const;

	// returns the intensity matrix (Q-matrix)
	TFullMatrix GetQMatrix() const;
	// returns a Q matrix for the joined states
	TFullMatrix GetQMatrix(const TVec<TIntV>& JoinedStateVV) const;

	TVector GetStateSizeV(const TVec<TIntV>& JoinedStateVV) const;

	TFullMatrix GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const;

	TFullMatrix GetModel(const TVec<TIntV>& JoinedStateVV) const { return GetQMatrix(JoinedStateVV); }

	void GetLikelyFutureStateV(const TVec<TIntV>& JoinedStateVV, const int& CurrState, const int& NFutStates, TIntV& FutStateIdV, TFltV& FutStateProbV) const;
	// get future state probabilities for all the states for a fixed time in the future
	TFullMatrix GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& Tm) const;
protected:
	void InitStats(const int& NStates);
	void AbsOnAddRec(const int& StateIdx, const uint64& RecTm);

	const TStr GetType() const { return "continuous"; }
};

class THierarchCtmc;
typedef TPt<THierarchCtmc> PHierarchCtmc;
class THierarchCtmc {
private:
	TCRef CRef;
public:
	friend class TPt<THierarchCtmc>;
private:
	PClust Clust;
    PMChain MChain;
    PHierarch Hierarch;

    PNotify Notify;

public:
    THierarchCtmc();
    THierarchCtmc(const PClust& Clust, const PMChain& MChain, const PHierarch& Hierarch, const PNotify& Notify=TNullNotify::New());

    // saves the model to the output stream
	void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PHierarchCtmc Load(TSIn& SIn);

	// saves this models as JSON
	PJsonVal SaveJson() const;

	// returns the probabilities of future states at time Tm, on the specified level
	// starting from the specified state
	void GetFutStateProbs(const double& Height, const int& StartState, const double& Tm, TFltV& ProbV) const;

	void GetTransitionModel(const double& Height, TFltVV& Mat) const;

	// initializes the model
	void Init(const TFullMatrix& X, const TUInt64V& RecTmV);

    int GetStates() const { return Hierarch->GetStates(); }
};

}

#endif /* CTMC_H_ */

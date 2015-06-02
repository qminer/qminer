/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
	
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

typedef TIntV TStateIdV;
typedef TIntSet TStateIdSet;
typedef TVec<TIntV> TStateSetV;
typedef TVec<TFltV> TStateFtrVV;

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TEuclDist {
public:
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist(const TFullMatrix& X, const TFullMatrix& Y)
		{ return GetDist2(X, Y).Sqrt(); }
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist2(const TFullMatrix& X, const TFullMatrix& Y);
};

class TStateIdentifier;
typedef TPt<TStateIdentifier> PStateIdentifier;
class TStateIdentifier {
private:
  TCRef CRef;
public:
  friend class TPt<TStateIdentifier>;
protected:
  	const static int MX_ITER;

    typedef TPair<TUInt64, TUInt64V> TFtrHistStat;
    typedef TVec<TFtrHistStat> TClustHistStat;
    typedef TVec<TClustHistStat> THistStat;

  	TRnd Rnd;
  	// holds centroids as column vectors
  	TFullMatrix CentroidMat;
  	TFullMatrix ControlCentroidMat;
  	// holds pairs <n,sum> where n is the number of points assigned to the
  	// centroid at index i and sum is the sum of distances of all the points
  	// assigned to the centroid to the centroid
  	TUInt64FltPrV CentroidDistStatV;

  	int NHistBins;			// the number of bins used in a histogram
  	TFltVV ObsFtrBinStartVV;		// stores where each bin starts
  	TFltVV ContrFtrBinStartVV;	// stores where each bin starts for the control matrix
  	THistStat ObsHistStat;		// stores histogram for every feature in every cluster
  	THistStat ControlHistStat;	// stores histogram for every feature in every cluster for the control matrix

  	double Sample;

  	bool Verbose;
  	PNotify Notify;

  	TStateIdentifier(const int NHistBins, const double& Sample, const TRnd& Rnd=TRnd(0),
  			const bool& Verbose=false);
  	TStateIdentifier(TSIn& SIn);

	virtual ~TStateIdentifier() {}

public:
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PStateIdentifier Load(TSIn& SIn);

	// performs the clustering
	void Init(const TFullMatrix& ObsMat, const TFltVV& FtrVV);
	// initializes histograms for every feature
	void InitHistogram(const TFltVV& ObsMat, const TFltVV& ControlFtrVV);

	// assign methods
	// assign instances to centroids
	int Assign(const TVector& Inst) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	TVector Assign(const TFltVV& InstMat) const;
	void Assign(const TFltVV& InstMat, TIntV& AssignV) const;

	// distance methods
	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
	TFullMatrix GetDistMat(const TFltVV& X) const;
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	TVector GetDistVec(const TVector& x) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const int& CentroidId, const TVector& Pt) const;

	// returns the coordinates of a "joined" centroid
	TVector GetJoinedCentroid(const TIntV& CentroidIdV) const;
	TVector GetJoinedControlCentroid(const TIntV& CentroidIdV) const;

	// cluster statistics
	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidId) const;
	// returns the number of points in the cluster
	uint64 GetClustSize(const int& ClustId) const;

	void GetHistogram(const int FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const;

	int GetClusts() const { return CentroidMat.GetCols(); }
	int GetDim() const { return CentroidMat.GetRows(); }
	int GetControlDim() const { return ControlCentroidMat.GetRows(); }
	const TFullMatrix& GetCentroidMat() const { return CentroidMat; }
	void GetCentroidVV(TVec<TFltV>& CentroidVV) const;
	void GetControlCentroidVV(TStateFtrVV& StateFtrVV) const;

	// sets the log to verbose or none
	void SetVerbose(const bool& Verbose);

protected:
	// Applies the algorithm. Instances should be in the columns of X.
	virtual void Apply(const TFullMatrix& X, const int& MaxIter=10000) = 0;
	TVector Assign(const TFltVV& X, const TVector& NormX2, const TVector& NormC2,
			const TVector& OnesN, const TVector& OnesK) const;
	// returns a matrix of squared distances
	TFullMatrix GetDistMat2(const TFltVV& X, const TVector& NormX2, const TVector& NormC2,
			const TVector& OnesN, const TVector& OnesK) const;

	// used during initialization
	TFullMatrix SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV);
	void UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV);
	void InitStatistics(const TFullMatrix& X, const TVector& AssignV);

	// returns the type of this clustering
	virtual const TStr GetType() const = 0;

private:
	void InitControlCentroids(const TFltVV& X,  const TFltVV& ControlFtrVV);
	// returns the coordinates of the centroid with the specified ID
	TVector GetCentroid(const int& CentroidId) const;
	TVector GetControlCentroid(const int& CentroidId) const;

	static void InitHist(const TFltVV& InstanceMat, const TIntV& AssignV,
			const TFltVV& FtrBinStartVV, const int& Clusts, const int& Bins,
			THistStat& HistStat);
	static void InitFtrBinStartVV(const TFltVV& InstanceMat, const int& Bins,
			TFltVV& FtrBinStartVV);
};

///////////////////////////////////////////
// K-Means
class TFullKMeans: public TStateIdentifier {
private:
	TInt K;

public:
	TFullKMeans(const int& NHistBins, const double Sample, const int& K, const TRnd& Rnd=TRnd(0),
			const bool& Verbose=false);
	TFullKMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFullMatrix& X, const int& MaxIter);

protected:
	const TStr GetType() const { return "kmeans"; }
};


///////////////////////////////////////////
// DPMeans
class TDpMeans: public TStateIdentifier {
private:
	TFlt Lambda;
	TInt MinClusts;
	TInt MaxClusts;

public:
	TDpMeans(const int& NHistBins, const double& Sample, const TFlt& Lambda,
			const TInt& MinClusts=1, const TInt& MaxClusts=TInt::Mx, const TRnd& Rnd=TRnd(0),
			const bool& Verbose=false);
	TDpMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFullMatrix& X, const int& MaxIter);

protected:
	const TStr GetType() const { return "dpmeans"; }
};

class TEuclMds {
public:
	// projects the points stored in the column of X onto d
	// dimensions
	static TFullMatrix Project(const TFullMatrix& X, const int& d=2);
};

class TAvgLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i,
			const int& j);
};

class TCompleteLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i,
			const int& j);
};

class TSingleLink {
public:
	static void JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& i,
			const int& j);
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
    TFltV StateHeightV, UniqueHeightV;
    TFlt MxHeight;

    // past states
    int HistCacheSize;
    TVec<TStateIdV> PastStateIdV;	// TODO not optimal structure

    TFltPrV StateCoordV;
    // number of leaf states, these are stored in the first part of the hierarchy vector
    int NLeafs;

    TStrV StateNmV;

    TIntFltPrSet TargetIdHeightSet;

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

	const TFltV& GetUniqueHeightV() const { return UniqueHeightV; }

	// return a list of state IDs and their heights
	void GetStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	// returns the 'joined' states at the specified height, puts teh state IDs into StateIdV
	// and sets of their leafs into JoinedStateVV
	void GetStateSetsAtHeight(const double& Height, TStateIdV& StateIdV, TStateSetV& StateSetV) const;
	// returns all the states just below the specified height
	void GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const;
	// fills the vector with IDs of the ancestors of the given state along with their heights
	void GetAncestorV(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns the ID of the ancestor of the given leaf at the specified height
	int GetAncestorAtHeight(const int& LeafId, const double& Height) const;
	// fills the vector with leaf descendants
	void GetLeafDescendantV(const int& StateId, TIntV& DescendantV) const;

	void GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	void GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const;

	// returns the coordinates of the specified state
	const TFltPr& GetStateCoords(const int& StateId) const { return StateCoordV[StateId]; }
	// returns the total number of states in the hierarchy
	int GetStates() const { return HierarchV.Len(); }
	// returns the number of leafs in the hierarchy
	int GetLeafs() const { return NLeafs; }

	bool IsStateNm(const int& StateId) const;
	void SetStateNm(const int& StateId, const TStr& StateNm);
	const TStr& GetStateNm(const int& StateId) const;

	// set/remove target states
	bool IsTarget(const int& StateId, const double& Height) const;
	void SetTarget(const int& StateId, const double& Height);
	void RemoveTarget(const int& StateId, const double& Height);

	const TIntFltPrSet& GetTargetStateIdSet() const { return TargetIdHeightSet; }

	void SetVerbose(const bool& Verbose);
	void PrintHierarch() const;

private:
	// returns the ID of the parent state
	int GetParentId(const int& StateId) const;
	// returns the height of the state
	double GetStateHeight(const int& StateId) const { return StateHeightV[StateId]; }
	int GetNearestHeightIdx(const double& Height) const;
	double GetNearestHeight(const double& InHeight) const;

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
	static int GetGrandparentId(const int& StateId, const TIntV& HierarchV)
		{ return GetParentId(GetParentId(StateId, HierarchV), HierarchV); }
	static bool IsRoot(const int& StateId, const TIntV& HierarchV);
	// returns a vector of unique heights
	static void GenUniqueHeightV(const TFltV& HeightV, TFltV& UniqueHeightV);

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

	double TmHorizon;
	double PredictionThreshold;
	int PdfBins;

	bool HasHiddenState;

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
	void Init(const TFltVV& FtrVV, const int& NStates, const TIntV& StateAssignV,
			const TUInt64V& TmV, const bool SequencedData, const TBoolV& SequenceEndV);

	// adds a single record to the model, the flag UpdateStates indicates if the statistics
	// should be updated
	void OnAddRec(const int& StateId, const uint64& RecTm, const bool IsLastInSeq);

	// get future state probabilities for a fixed time in the future
	void GetFutureProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbV) const;
	// get past state probabilities for a fixed time in the past
	void GetPastProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbV) const;
	// returns the most likely next states, excluding the current state,
	// along with probabilities of going into those states
	virtual void GetNextStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const = 0;
	// returns the most likely previous states, excluding the current state,
	// along with probabilities of going into those states
	virtual void GetPrevStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const = 0;

	virtual void GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm,
			const double EndTm, const double& DeltaTm, const TStateSetV& StateSetV,
			const TStateFtrVV& StateFtrVV, const TStateIdV& StateIdV, TVec<TFltV>& FutProbVV,
			TVec<TFltV>& PastProbVV) const = 0;

	virtual bool PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TStateSetV& StateSetV,
			const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
			double& Prob, TFltV& ProbV, TFltV& TmV) const = 0;

	// static distribution
	// returns the static distribution for the joined states
	virtual TVector GetStatDist(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const = 0;

	// returns a vector of state sizes
	virtual TVector GetStateSizeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const = 0;
	virtual TFullMatrix GetTransitionMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const = 0;
	virtual TFullMatrix GetModel(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const = 0;

	virtual TVector GetHoldingTimeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const = 0;

	// returns the number of states
	int GetStates() const { return NStates; };
	// returns the ID of the current state
	int GetCurrStateId() const { return CurrStateId; };

	// returns true is the jump from OldStateId to NewStateId is considered anomalous
	virtual bool IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const = 0;

	virtual void CreateFtrV(const TFltV& ObsFtrV, const TFltV& PrevObsFtrV,
    		const TFltV& ContrFtrV, const TFltV& PrevContrFtrV, const uint64& RecTm,
			const uint64& PrevRecTm, TFltV& FtrV) const = 0;

	// set params
	double GetTimeHorizon() const { return TmHorizon; }
	void SetTimeHorizon(const double& Horizon) { TmHorizon = Horizon; }
    double GetPredictionThreshold() const { return PredictionThreshold; }
	void SetPredictionThreshold(const double& Threshold) { PredictionThreshold = Threshold; }
	int GetPdfBins() const { return PdfBins; }
	void SetPdfBins(const int Bins) { PdfBins = Bins; }
	void SetVerbose(const bool& Verbose);

protected:
	// handling the hidden state
	int GetHiddenStateId() const;
	// inserts the hidden state into the state set vector
	void InsHiddenState(TStateSetV& StateSetV) const;
	// inserts the hidden state into the state set vector
	void InsHiddenState(TStateIdV& StateIdV) const;
	// removes the hidden state probability from the probability vector
	void RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const;

	// initializes the statistics
	virtual void AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) = 0;

	// get future state probabilities for all the states for a fixed time in the future
	virtual TFullMatrix GetFutureProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm) const = 0;
	// get [ast state probabilities for all the states for a fixed time in the past
	virtual TFullMatrix GetPastProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm) const = 0;

	static void GetFutureProbVOverTm(const TFullMatrix& PMat, const int& StateIdx,
			const int& Steps, TVec<TFltV>& ProbVV, const PNotify& Notify,
			const bool IncludeT0=true);

	virtual void InitIntensities(const TFltVV& FtrV, const TUInt64V& TmV, const TIntV& AssignV,
			const TBoolV& EndsBatchV) = 0;
	virtual const TStr GetType() const = 0;
};

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtMChain: public TMChain {
	typedef TFltV TJumpTmV;
	typedef TVec<TFltV> TJumpFtrVV;
	typedef TFltVV TJumpFtrMat;

	typedef TVec<TVec<TJumpTmV>> TJumpTmVMat;
	typedef TVVec<TJumpFtrVV> TJumpFtrVVMat;
	typedef TVVec<TJumpFtrMat> TJumpFtrMatMat;

	typedef TPropHazards TIntensModel;
	typedef TVVec<TIntensModel> TIntensModelMat;
public:
	const static uint64 TU_SECOND;
	const static uint64 TU_MINUTE;
	const static uint64 TU_HOUR;
	const static uint64 TU_DAY;
	const static uint64 TU_MONTH;

private:
	const static double MIN_STAY_TM;
	const static double HIDDEN_STATE_INTENSITY;

	TIntensModelMat IntensModelMat;
	// stores how many jump from the hidden state to the specified state occurred
	TIntV HiddenStateJumpCountV;

	double DeltaTm;

	uint64 TimeUnit;
	uint64 PrevJumpTm;

public:
	TCtMChain(const uint64& TimeUnit, const double& DeltaTm, const bool& Verbose=false);
	TCtMChain(TSIn& SIn);

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// returns the most likely next states excluding the current state
	void GetNextStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const;
	// returns the most likely previous states excluding the current state
	void GetPrevStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const;

	void GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm,
			const double EndTm, const double& DeltaTm, const TStateSetV& StateSetV,
			const TStateFtrVV& StateFtrVV, const TStateIdV& StateIdV, TVec<TFltV>& FutProbVV,
			TVec<TFltV>& PastProbVV) const;

	// approximates the probability of jumping into the target state in the prespecified
	// time horizon
	bool PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TStateSetV& StateSetV,
			const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
			double& Prob, TFltV& ProbV, TFltV& TmV) const;

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	TVector GetStatDist(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;

	// returns the size of each state used in the visualization
	TVector GetStateSizeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;
	TFullMatrix GetTransitionMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;
	TFullMatrix GetJumpMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;
	TFullMatrix GetModel(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const
		{ return GetQMatrix(StateSetV, StateFtrVV); }

	TVector GetHoldingTimeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;

	// returns true if the jump from OldStateId to NewStateId has a low enough probability
	bool IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const;

	int GetStates() const { return NStates; }

	void CreateFtrV(const TFltV& ObsFtrV, const TFltV& PrevObsFtrV,
    		const TFltV& ContrFtrV, const TFltV& PrevContrFtrV, const uint64& RecTm,
			const uint64& PrevRecTm, TFltV& FtrV) const;

protected:
//	void InitStats(const int& NStates);
	void AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch);

	// get future state probabilities for all the states for a fixed time in the future
	TFullMatrix GetFutureProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm) const;
	TFullMatrix GetPastProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm) const;

	void InitIntensities(const TFltVV& FtrVV, const TUInt64V& TmV, const TIntV& AssignV, const TBoolV& EndBatchV);
	// prints the statistics used to build the Q-matrix
//	void PrintStats() const;
	const TStr GetType() const { return "continuous"; }

private:
	// returns the intensity matrix (Q-matrix)
	TFullMatrix GetQMatrix(const TStateFtrVV& StateFtrVV) const;
	TVector GetStateIntensV(const int StateId, const TFltV& FtrV) const;

	// returns a Q matrix for the joined states
	TFullMatrix GetQMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;
	// returns a Q matrix for the joined states for the time reversal Markov chain
	TFullMatrix GetRevQMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const;

	// returns a vector of holding times
	// a holding time is the expected time that the process will stay in state i
	// it is an exponential random variable of parameter -q_ii, so its expected value
	// is -1/q_ii
	TVector GetHoldingTimeV(const TFullMatrix& QMat) const;

	bool IsHiddenStateId(const int& StateId) const { return HasHiddenState && StateId == GetHiddenStateId(); }

//	void UpdateIntensity(const int& FromStateId, const int& ToStateId, const double& Tm);

	static void GetNextStateProbV(const TFullMatrix& QMat, const TStateIdV& StateIdV,
			const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
			const PNotify& Notify);

	// returns the stationary distribution
	static TVector GetStatDist(const TFullMatrix& QMat, const PNotify& Notify);

	static TFullMatrix GetProbMat(const TFullMatrix& QMat, const double& Dt);

	static TFullMatrix GetFutureProbMat(const TFullMatrix& QMat, const double& Tm,
			const double& DeltaTm, const bool HasHiddenState=false);

	static double PredictOccurenceTime(const TFullMatrix& QMat, const int& CurrStateIdx,
			const int& TargetStateIdx, const double& DeltaTm, const double& HorizonTm,
			TFltV& TmV, TFltV& HitProbV);

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

	void Init(const TFullMatrix& X, const PStateIdentifier& Clust, const PHierarch& Hierarch);

	void GetSuggestFtrs(const int& StateId, TFltV& WgtV) const;
};

class TStreamStory;
	typedef TPt<TStreamStory> PStreamStory;
class TStreamStory{
private:
	TCRef CRef;
public:
  	friend class TPt<TStreamStory>;
	class TCallback;	// declaration in the helper classes section
private:
	PStateIdentifier StateIdentifier;
    PMChain MChain;
    PHierarch Hierarch;
    PStateAssist StateAssist;

    TFltV FtrFactorV;

    TFltV PrevObsFtrV, PrevContrFtrV;
    uint64 PrevRecTm;

    bool Verbose;

    TCallback* Callback;

    PNotify Notify;

public:
    // constructors
    TStreamStory();
    TStreamStory(const PStateIdentifier& Clust, const PMChain& MChain, const PHierarch& Hierarch,
    		const bool& Verbose=true);
    TStreamStory(TSIn& SIn);

    ~TStreamStory() {}

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// saves this models as JSON
	PJsonVal GetJson() const;

	// update methods
	// initializes the model
	void Init(const TFullMatrix& ObservMat, const TFullMatrix& ControlMat, const TUInt64V& RecTmV);
	void Init(TFltVV& ObservVV, TFltVV& ControlVV, const TUInt64V& RecTmV);
	void InitBatches(const TFullMatrix& ObservFtrs, const TFullMatrix& ControlFtrs,
			const TUInt64V& RecTmV, const TBoolV& BatchEndV);
	void InitClust(const TFullMatrix& ObsMat, const TFltVV& FtrVV,
			TIntV& AssignV);
	void InitMChain(const TFltVV& FtrVV, const TIntV& AssignV, const TUInt64V& RecTmV,
			const bool IsBatchData, const TBoolV& EndBatchV);
	void InitHierarch();
	void InitHistograms(const TFltVV& ObsMat, const TFltVV& ControlMat,
			const TUInt64V& RecTmV, const TBoolV& BatchEndV);
	void InitStateAssist(const TFullMatrix& X);

	void OnAddRec(const uint64& RecTm, const TFltV& ObsFtrV, const TFltV& ContrFtrV);

	// future and past probabilities
	// returns the probabilities of future states at time Tm, on the specified level
	// starting from the specified state
	void GetFutStateProbV(const double& Height, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbPrV) const;
	// returns the probabilities of past states at time Tm, on the specified level
	// starting from the specified state
	void GetPastStateProbV(const double& Height, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbPrV) const;
	// returns a distribution of probabilities of the next states
	void GetNextStateProbV(const double& Height, const int& StateId,
			TIntFltPrV& StateIdProbV) const;
	// returns a distribution of probabilities of the previous states
	void GetPrevStateProbV(const double& Height, const int& StateId,
			TIntFltPrV& StateIdProbV) const;

	void GetProbVOverTm(const double& Height, const int& StateId, const double StartTm,
			const double EndTm, const double& DeltaTm, TStateIdV& StateIdV,
			TVec<TFltV>& FutProbV, TVec<TFltV>& PastProbV) const;

	void GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const;

	void GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const;

	void GetStateWgtV(const int& StateId, TFltV& WgtV) const;

	// stores the transition model for the current height into Mat
	void GetTransitionModel(const double& Height, TFltVV& Mat) const;
	// returns a list of ancestor states along with their heights
	void GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns a list of ancestors of the current state
	void GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const;
	// returns the current state on the specified level
	int GetCurrStateId(const double& Height) const;
	// returns the centroid of the given state
	void GetCentroid(const int& StateId, TFltV& FtrV, const bool ObsCentroid=true) const;
	// returns the IDs of all the states on the specified height
	void GetStateIdVAtHeight(const double& Height, TStateIdV& StateIdV) const;
	// returns the number of states in the hierarchy
    int GetStates() const { return Hierarch->GetStates(); }

    // target methods
    bool IsTargetState(const int& StateId, const double& Height) const { return Hierarch->IsTarget(StateId, Height); }
    void SetTargetState(const int& StateId, const double& Height, const bool& IsTrg);

    // sets the name of the specified state
    void SetStateNm(const int& StateId, const TStr& StateNm);
    void SetControlFtrFactor(const int& ControlFtrIdx, const double& Factor);
    const TStr& GetStateNm(const int& StateId) const;

    // get/set parameters
    double GetPredictionThreshold() const { return MChain->GetPredictionThreshold(); }
    void SetPredictionThreshold(const double& Threshold) { MChain->SetPredictionThreshold(Threshold); }
    double GetTimeHorizon() const { return MChain->GetTimeHorizon(); }
    void SetTimeHorizon(const double& TmHorizon) { MChain->SetTimeHorizon(TmHorizon); }
    int GetPdfBins() const { return MChain->GetPdfBins(); }
    void SetPdfBins(const int& Bins) { MChain->SetPdfBins(Bins); }
    bool IsVerbose() const { return Verbose; }
    void SetVerbose(const bool& Verbose);
    void SetCallback(TCallback* Callback);

private:
    void CreateFtrVV(const TFltVV& ObservFtrMat, const TFltVV& ControlFtrMat,
    		const TUInt64V& RecTmV, const TBoolV& EndsBatchV, TFltVV& FtrMat) const;
    void CreateFtrV(const TFltV& ObsFtrV, const TFltV& ContrFtrV, const uint64& RecTm,
    		TFltV& FtrV) const;

    void GetStateFtrVV(TStateFtrVV& StateFtrVV) const;

    void GetStatsAtHeight(const double& Height, TStateSetV& StateSetV, TStateIdV& StateIdV,
    		TStateFtrVV& StateFtrVV) const;

    void DetectAnomalies(const int& NewStateId, const int& OldStateId, const TFltV& ObsFtrV,
    		const TFltV& FtrV) const;

    void PredictTargets(const TStateFtrVV& StateFtrVV, const int& CurrStateId) const;

    void CheckBatches(const TUInt64V& TmV, const TBoolV& BatchEndV) const;

public:
    // helper classes
    class TCallback {
    public:
    	virtual ~TCallback() {}

    	virtual void OnStateChanged(const TIntFltPrV& StateIdHeightV) = 0;
    	virtual void OnAnomaly(const TStr& AnomalyDesc) = 0;
    	virtual void OnOutlier(const TFltV& FtrV) = 0;
    	virtual void OnPrediction(const int& CurrStateId, const int& TargetStateId,
    			const double& Prob, const TFltV& ProbV, const TFltV& TmV) = 0;
    };
};

}

#endif /* CTMC_H_ */

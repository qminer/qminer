/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TMc {

using namespace TRegression;
using namespace TDist;
using namespace TClustering;

namespace {
	typedef TIntV TStateIdV;
	typedef TIntSet TStateIdSet;
	typedef TVec<TIntV> TStateSetV;
	typedef TVec<TFltV> TStateFtrVV;

	typedef TAbsKMeans TClust;
	typedef PDnsKMeans PClust;
}

//////////////////////////////////////////////////
// Histogram class
class THistogram {
private:
	TInt Bins;
	TInt64 TotalCount;
	TIntV CountV;
	TFltV BinStartV;

public:
	THistogram();
	THistogram(const int& NBins, const double& MnVal, const double& MxVal);
	THistogram(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Update(const double& FtrVal);

	const TFltV& GetBinStartV() const { return BinStartV; }
	const TIntV& GetCountV() const { return CountV; }
	const TInt64& GetTotalCount() const { return TotalCount; }

	bool Empty() const { return TotalCount == 0; }
};

//////////////////////////////////////////////////
// State Identifier
class TStateIdentifier;
typedef TPt<TStateIdentifier> PStateIdentifier;
class TStateIdentifier {
private:
  TCRef CRef;
public:
  friend class TPt<TStateIdentifier>;
private:
  	const static int MX_ITER;

    typedef TVec<THistogram> TFtrHistV;
    typedef TVec<TFtrHistV> TStateFtrHistVV;
    typedef TVVec<TFtrHistV> THistMat;

  	TRnd Rnd;

  	// clustering
  	PClust KMeans;
  	// holds centroids as column vectors
  	TFltVV ControlCentroidMat;
  	// holds pairs <n,sum> where n is the number of points assigned to the
  	// centroid at index i and sum is the sum of distances of all the points
  	// assigned to the centroid to the centroid
  	TUInt64FltPrV CentroidDistStatV;

  	int NHistBins;					// the number of bins used in a histogram
  	TStateFtrHistVV ObsHistVV;		// histograms of observation features
  	TStateFtrHistVV ControlHistVV;	// histograms of control features
  	THistMat TransHistMat;			// histograms of transitions

  	TVec<TFltV> StateContrFtrValVV;

  	double Sample;

  	bool Verbose;
  	PNotify Notify;

public:
  	TStateIdentifier(const PClust& KMeans, const int NHistBins, const double& Sample,
			const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
	TStateIdentifier(TSIn& SIn);

	virtual ~TStateIdentifier() {}
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;

	// performs the clustering
	void Init(TFltVV& ObsFtrVV, const TFltVV& ControlFtrVV);
	// initializes histograms for every feature
	void InitHistograms(const TFltVV& ObsMat, const TFltVV& ControlFtrVV);

	// assign methods
	// assign instances to centroids
	int Assign(const TFltV& Inst) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	void Assign(const TFltVV& InstMat, TIntV& AssignV) const;

	// distance methods
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	void GetCentroidDistV(const TFltV& x, TFltV& DistV) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const int& CentroidId, const TFltV& Pt) const;

	// returns the coordinates of a "joined" centroid
	void GetJoinedCentroid(const TIntV& CentroidIdV, TFltV& Centroid) const;
	void GetJoinedControlCentroid(const TIntV& CentroidIdV, TFltV& Centroid) const;

	// cluster statistics
	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidId) const;
	// returns the number of points in the cluster
	uint64 GetClustSize(const int& ClustId) const;

	void GetHistogram(const int& FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const;
	void GetTransitionHistogram(const int& FtrId, const TIntV& SourceStateSet,
			const TIntV& TargetStateSet, TFltV& BinStartV, TFltV& ProbV) const;

	int GetStates() const { return KMeans->GetClusts(); }
	int GetDim() const { return KMeans->GetDim(); }
	int GetControlDim() const { return ControlCentroidMat.GetRows(); }
	const TFltVV& GetCentroidMat() const { return KMeans->GetCentroidVV(); }
	void GetCentroidVV(TVec<TFltV>& CentroidVV) const;
	void GetControlCentroidVV(TStateFtrVV& StateFtrVV) const;

	// manual setting of control features
	double GetControlFtr(const int& StateId, const int& FtrId, const double& DefaultVal) const;
	void SetControlFtr(const int& StateId, const int& FtrId, const double& Val);
	void ClearControlFtr(const int& StateId, const int& FtrId);
	void ClearControlFtrVV();
	bool IsControlFtrSet(const int& StateId, const int& FtrId) const;
	bool IsAnyControlFtrSet() const;

	// sets the log to verbose or none
	void SetVerbose(const bool& Verbose);

protected:
	// used during initialization
	void InitStatistics(const TFltVV& X);

private:
	void InitControlCentroids(const TFltVV& X,  const TFltVV& ControlFtrVV);
	// returns the coordinates of the centroid with the specified ID
	void GetCentroid(const int& StateId, TFltV& FtrV) const;
	void GetControlCentroid(const int& StateId, TFltV& FtrV) const;

	double GetControlFtr(const int& StateId, const int& FtrId) const;
	void ClearControlFtrVV(const int& Dim);

	// histograms
	void InitHists(const TFltVV& ObsFtrVV, const TFltVV& ConstFtrVV);
	void UpdateTransitionHist(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
			const TIntV& AssignV);
	static void UpdateHist(const TFltVV& FtrVV, const TIntV& AssignV,
			const int& States, TStateFtrHistVV& StateFtrHistVV);
};

class TEuclMds {
public:
	// projects the points stored in the column of X onto d
	// dimensions
	static void Project(const TFltVV& X, TFltVV& ProjVV, const int& d=2);
};

////////////////////////////////////////////
// Hierarchy modeler
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

//    TFltPrV StateCoordV;
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

	void Init(const int& CurrLeafId, const PStateIdentifier& StateIdentifier);
	void UpdateHistory(const int& CurrLeafId);

	const TFltV& GetUniqueHeightV() const { return UniqueHeightV; }
	const TIntV& GetHierarchV() const { return HierarchV; }

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

	// for each state returns the number of leafs it's subtree has
	void GetLeafSuccesorCountV(TIntV& LeafCountV) const;

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

	bool IsLeaf(const int& StateId) const;

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
// Intensity Modeler
class TBernoulliIntens {
private:
	static const double MIN_PROB;

	typedef TVVec<TLogReg> TLogRegVV;

	int NStates;

	TLogRegVV LogRegVV;
	TBoolVV HasJumpedVV;
	TBoolVV EmptyVV;
	double DeltaTm;

public:
	TBernoulliIntens();
	TBernoulliIntens(const int& NStates, const double& DeltaTm, const double& RegFact,
			const TBoolVV& HasJumpedV, const bool Verbose=false);
	TBernoulliIntens(TSIn& SIn);

	void Save(TSOut& SOut) const;
	void Fit(const int& RowN, const int& ColN, const TFltVV& X, const TFltV& y,
			const double& Eps=1e-3);

	void GetQMat(const TStateFtrVV& StateFtrVV, TFltVV& QMat) const;
	void GetQMatRow(const int& RowN, const TFltV& FtrV, TFltV& IntensV) const;
};

/////////////////////////////////////////////////////////////////
// Discrete time Markov chain
class TDtMChain {
public:
	static void GetProbVV(const TFltVV& PMat, const int& Steps, TFltVV& ProbVV);
};

/////////////////////////////////////////////////////////////////
// Continuous time Markov chain
class TCtMChain {
public:
	/// get a model where states are aggregated
	static void GetAggrQMat(const TFltVV& QMat, const TStateSetV& AggrStateV, TFltVV& AggrQMat);

	/// get the reverse time model
	static void GetRevQMat(const TFltVV& QMat, TFltVV& RevQMat);

	/// returns the stationary distribution
	static void GetStatDistV(const TFltVV& QMat, TFltV& StatDistV);
	static void GetHoldingTmV(const TFltVV& QMat, TFltV& HoldingTmV);

	// probabilities from intensities
	/// returns the transition probability matrix for a infinitely small
	/// time step Dt
	static void GetProbVV(const TFltVV& QMat, const double& Dt, TFltVV& ProbVV);
	static void GetFutProbVV(const TFltVV& QMat, const double& DeltaTm, const double& Tm,
			TFltVV& ProbVV);
	static void GetProbVV(const TFltVV& QMat, const double& DeltaTm, const double& Tm,
			TFltVV& ProbVV);

	// jump probabilities
	static void GetJumpV(const TFltVV& QMat, const int& CurrStateId, TFltV& JumpV);
	/// returns a jump matrix for the given transition rate matrix
	/// when the process decides to jump the jump matrix describes to
	/// which state it will jump with which probability
	static void GetJumpVV(const TFltVV& QMat, TFltVV& JumpVV);

	// hitting times
	static double HitTmPdf(const TFltVV& QMat, const int& StateId, const int& TargetStateId,
			const double& DeltaTm, const double& TotalTm, const int& PdfBins, TFltV& TmV,
			TFltV& HitProbV);
};

/////////////////////////////////////////////////////////////////
// Markov Chain
class TTransitionModeler;
typedef TPt<TTransitionModeler> PTransitionModeler;
class TTransitionModeler {
private:
	TCRef CRef;
public:
	friend class TPt<TTransitionModeler>;
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
	TTransitionModeler(const bool& Verbose);
	TTransitionModeler(TSIn& SIn);

	// destructor
	virtual ~TTransitionModeler() {}

public:
	// save / load
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PTransitionModeler Load(TSIn& SIn);

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

	virtual void GetProbVAtTime(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StartStateId, const double& Tm,
			TFltV& ProbV) const = 0;

	virtual bool PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TStateSetV& StateSetV,
			const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
			double& Prob, TFltV& ProbV, TFltV& TmV) const = 0;

	// static distribution
	// returns the static distribution for the joined states
	virtual void GetStatDist(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltV& StatDist) const = 0;

	// returns a vector of state sizes
	virtual void GetTransitionVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& TransVV) const = 0;
	virtual void GetModel(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& Mat) const = 0;

	virtual void GetHoldingTimeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, TFltV& HoldingTmV) const = 0;

	// returns the number of states
	int GetStates() const { return NStates; };
	virtual const uint64& GetTimeUnit() const = 0;
	// returns the ID of the current state
	int GetCurrStateId() const { return CurrStateId; };

	// returns true is the jump from OldStateId to NewStateId is considered anomalous
	virtual bool IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const = 0;

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
	virtual void GetFutureProbVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const = 0;
	// get [ast state probabilities for all the states for a fixed time in the past
	virtual void GetPastProbVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const = 0;

	virtual void InitIntensities(const TFltVV& FtrV, const TUInt64V& TmV, const TIntV& AssignV,
			const TBoolV& EndsBatchV) = 0;
	virtual const TStr GetType() const = 0;
};

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtModeler: public TTransitionModeler {
	typedef TFltV TLabelV;
	typedef TVec<TFltV> TFtrVV;
	typedef TFltVV TJumpFtrMat;

	typedef TVec<TVec<TLabelV>> TLabelVMat;
	typedef TVVec<TFtrVV> TJumpFtrVVMat;
	typedef TVVec<TJumpFtrMat> TJumpFtrMatMat;

	typedef TBernoulliIntens TIntensModel;
public:
	static const uint64 TU_SECOND;
	static const uint64 TU_MINUTE;
	static const uint64 TU_HOUR;
	static const uint64 TU_DAY;
	static const uint64 TU_MONTH;

private:
	static const double MIN_STAY_TM;
	static const double HIDDEN_STATE_INTENSITY;

	TIntensModel IntensModel;
	// stores how many jump from the hidden state to the specified state occurred
	TIntV HiddenStateJumpCountV;

	double DeltaTm;

	uint64 TimeUnit;
	uint64 PrevJumpTm;

public:
	TCtModeler(const uint64& TimeUnit, const double& DeltaTm, const bool& Verbose=false);
	TCtModeler(TSIn& SIn);

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

	void GetProbVAtTime(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StartStateId, const double& Tm,
			TFltV& ProbV) const;

	// approximates the probability of jumping into the target state in the prespecified
	// time horizon
	bool PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TStateSetV& StateSetV,
			const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
			double& Prob, TFltV& ProbV, TFltV& TmV) const;

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	void GetStatDist(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltV& ProbV) const;

	void GetTransitionVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& TransVV) const;
	void GetJumpVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, TFltVV& JumpVV) const;
	void GetModel(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, TFltVV& QMat) const;

	void GetHoldingTimeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltV& HoldingTmV) const;

	// returns true if the jump from OldStateId to NewStateId has a low enough probability
	bool IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const;

	int GetStates() const { return NStates; }
	const uint64& GetTimeUnit() const { return TimeUnit; }

protected:
	void AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch);

	// get future state probabilities for all the states for a fixed time in the future
	void GetFutureProbVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const;
	void GetPastProbVV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const;

	void InitIntensities(const TFltVV& FtrVV, const TUInt64V& TmV, const TIntV& AssignV,
			const TBoolV& EndBatchV);
	// prints the statistics used to build the Q-matrix
	const TStr GetType() const { return "continuous"; }

private:
	void GetStateIntensV(const int StateId, const TFltV& FtrV, TFltV& IntensV) const;

	// returns the intensity matrix (Q-matrix)
	void GetQMatrix(const TStateFtrVV& StateFtrVV, TFltVV& QMat) const;
	// returns a Q matrix for the joined states
	void GetQMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& QMat) const;
	// returns a Q matrix for the joined states for the time reversal Markov chain
	void GetRevQMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& RevQMat) const;

	bool IsHiddenStateId(const int& StateId) const { return HasHiddenState && StateId == GetHiddenStateId(); }

	static void GetNextStateProbV(const TFltVV& QMat, const TStateIdV& StateIdV,
			const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
			const PNotify& Notify);

	static void GetFutureProbVV(const TFltVV& QMat, const double& Tm,
			const double& DeltaTm, TFltVV& ProbVV, const bool HasHiddenState=false);
};

/////////////////////////////////////////////////////////////////
// UI helper
class TUiHelper;
typedef TPt<TUiHelper> PUiHelper;
class TUiHelper {
private:
	TCRef CRef;
public:
	friend class TPt<TUiHelper>;
private:
	static const double STEP_FACTOR;
	static const double INIT_RADIUS_FACTOR;
	TFltPrV StateCoordV;

	TRnd Rnd;

	bool Verbose;
	PNotify Notify;
public:
	TUiHelper(const TRnd& Rnd, const bool& Verbose);
	TUiHelper(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Init(const PStateIdentifier& StateIdentifier, const PHierarch& Hierarch,
			const PTransitionModeler& MChain);

	const TFltPr& GetStateCoords(const int& StateId) const;
	void GetStateRadiusV(const TFltV& ProbV, TFltV& SizeV) const;

private:
	TFltPr& GetModStateCoords(const int& StateId);

	// computes the coordinates (in 2D) of each state
	void InitStateCoordV(const PStateIdentifier& StateIdentifier,
			const PHierarch& Hierarch);
	void RefineStateCoordV(const PStateIdentifier& StateIdentifier,
			const PHierarch& Hierarch, const PTransitionModeler& MChain);

	static double GetUIStateRaduis(const double& Prob);
	static bool NodesOverlap(const int& StartId, const int& EndId, const TFltPrV& CoordV,
			const TFltV& RaduisV);
	static int CountOverlaps(const int& StartId, const int& EndId, const TFltPrV& CoordV,
			const TFltV& RaduisV);
	static double GetOverlap(const TFltPr& Pos1, const TFltPr& Pos2,
			const double& Raduis1, const double& Raduis2);
	static void GetMoveDir(const TFltPr& Pos1, const TFltPr& Pos2, TFltPr& Dir);
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
	TFltPrV FtrBoundV;

	TRnd Rnd;

	bool Verbose;
	PNotify Notify;

public:
	TStateAssist(const bool Verbose);
	TStateAssist(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Init(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const PStateIdentifier& Clust,
			const PHierarch& Hierarch);
	void InitFtrBounds(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV);

	const TFltPr& GetFtrBounds(const int& FtrId) const;
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
	PTransitionModeler MChain;
    PHierarch Hierarch;
    PStateAssist StateAssist;
    PUiHelper UiHelper;

    TFltV PrevObsFtrV, PrevContrFtrV;
    uint64 PrevRecTm;

    bool Verbose;

    TCallback* Callback;

    PNotify Notify;

public:
    // constructors
    TStreamStory();
    TStreamStory(const PStateIdentifier& Clust, const PTransitionModeler& MChain, const PHierarch& Hierarch,
    		const TRnd& Rnd=TRnd(0), const bool& Verbose=true);
    TStreamStory(TSIn& SIn);

    ~TStreamStory() {}

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// saves this models as JSON
	PJsonVal GetJson() const;

	// update methods
	// initializes the model
	void Init(TFltVV& ObservVV, const TFltVV& ControlVV, const TUInt64V& RecTmV);
	void InitBatches(TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
			const TUInt64V& RecTmV, const TBoolV& BatchEndV);
	void InitClust(TFltVV& ObsFtrVV, const TFltVV& FtrVV,
			TIntV& AssignV);	// TODO add const
	void InitMChain(const TFltVV& FtrVV, const TIntV& AssignV, const TUInt64V& RecTmV,
			const bool IsBatchData, const TBoolV& EndBatchV);
	void InitHierarch();
	void InitHistograms(const TFltVV& ObsMat, const TFltVV& ControlMat,
			const TUInt64V& RecTmV, const TBoolV& BatchEndV);
	void InitStateAssist(TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV);	// TODO add const

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

	void GetProbVAtTime(const int& StartStateId, const double& Level, const double& Time,
			TIntV& StateIdV, TFltV& ProbV) const;

	void GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const;

	// histograms
	void GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const;
	void GetTransitionHistogram(const int& SourceId, const int& TargetId, const int& FtrId,
			TFltV& BinStartV, TFltV& ProbV) const;

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
    uint64 GetTimeUnit() const;

    // target methods
    bool IsTargetState(const int& StateId, const double& Height) const { return Hierarch->IsTarget(StateId, Height); }
    void SetTargetState(const int& StateId, const double& Height, const bool& IsTrg);

    bool IsLeaf(const int& StateId) const;

    // sets the name of the specified state
    void SetStateNm(const int& StateId, const TStr& StateNm);

    // control features
    void SetControlFtrVal(const int& StateId, const int& ContrFtrId, const double& Val);
    void SetControlFtrsVal(const int& ContrFtrId, const double& Val);
    void ResetControlFtrVal(const int& StateId, const int& FtrId);
    void ResetControlFtrVals(const int& StateId);
    void ResetControlFtrVals();
    bool IsAnyControlFtrSet() const;

    const TFltPr& GetFtrBounds(const int& FtrId) const;
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

    void PredictTargets(const uint64& RecTm, const TStateFtrVV& StateFtrVV, const int& CurrStateId) const;

    void CheckBatches(const TUInt64V& TmV, const TBoolV& BatchEndV) const;

public:
    // helper classes
    class TCallback {
    public:
    	virtual ~TCallback() {}

    	virtual void OnStateChanged(const TIntFltPrV& StateIdHeightV) = 0;
    	virtual void OnAnomaly(const TStr& AnomalyDesc) = 0;
    	virtual void OnOutlier(const TFltV& FtrV) = 0;
    	virtual void OnPrediction(const uint64& RecTm, const int& CurrStateId,
    			const int& TargetStateId, const double& Prob, const TFltV& ProbV,
				const TFltV& TmV) = 0;
    };
};

}

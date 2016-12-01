/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef STREAMSTORY_H_
#define STREAMSTORY_H_

#include "base.h"
#include "mine.h"

namespace TMc {

using namespace TClassification;
using namespace TClustering;

namespace {
	typedef TIntV TStateIdV;
	typedef TIntSet TStateIdSet;
	typedef TIntV TAggState;
	typedef TVec<TAggState> TAggStateV;
	typedef TVec<TFltV> TStateFtrVV;

	// structure for returning the "state history" or "big picture"
    typedef TVec<TTriple<TUInt64,TUInt64,TIntFltH>> TStateHist;
    typedef TVec<TPair<TFlt, TStateHist>> TScaleStateHistV;
}

// forward declarations
class TStreamStory;

// helper classes
class TStreamStoryCallback {
public:
	virtual ~TStreamStoryCallback() {}

	virtual void OnStateChanged(const uint64 Tm, const TIntFltPrV& StateIdHeightV) = 0;
	virtual void OnAnomaly(const TStr& AnomalyDesc) = 0;
	virtual void OnOutlier(const TFltV& FtrV) = 0;
	virtual void OnProgress(const int& Perc, const TStr& Msg) = 0;
	virtual void OnPrediction(const uint64& RecTm, const int& CurrStateId,
			const int& TargetStateId, const double& Prob, const TFltV& ProbV,
			const TFltV& TmV) = 0;
	virtual void OnActivityDetected(const uint64& StartTm, const uint64& EndTm, const TStr& ActNm) = 0;
};

enum TFtrType {
    ftUndefined,
    ftNumeric,
    ftCategorical,
    ftTime
};

///////////////////////////////////////////////
/// Feature information - stores the type and position of a feature
class TFtrInfo {
private:
	TFtrType Type;
	int Offset;
	int Length;

public:
	TFtrInfo();
	TFtrInfo(const TFtrType& Type, const int& Offset, const int& Length);
	TFtrInfo(TSIn& SIn);

	void Save(TSOut& SOut) const;

	TFtrInfo& operator =(const TFtrInfo& Info);

	const TFtrType& GetType() const { return Type; }
	TStr GetTypeStr() const;
	const int& GetOffset() const { return Offset; }
	const int& GetLength() const { return Length; }
	int GetRange() const { return IsNumeric() ? TInt::Mx : Length-1; }

	bool IsNumeric() const { return GetType() == ftNumeric; }
	bool IsCategorical() const { return GetType() == ftCategorical; }

	int GetCategoricalFtrVal(const TFltV& FtrV) const;
	double GetNumericFtrVal(const TFltV& FtrV) const;
	double GetFtrVal(const TFltV& FtrV) const;
};

typedef TVec<TFtrInfo> TFtrInfoV;
//================================================================

//////////////////////////////////////////////////
// Histogram class
class THistogram {
private:
	TInt Bins;
	TInt64 TotalCount;
	TIntV CountV;
	TFltV BinValV;

public:
	THistogram();
	THistogram(const int& NBins, const double& MnVal, const double& MxVal);
	THistogram(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Update(const double& FtrVal);

	const TFltV& GetBinValV() const { return BinValV; }
	const TIntV& GetCountV() const { return CountV; }
	const TInt64& GetTotalCount() const { return TotalCount; }
	double GeNmVal() const { return BinValV[0] - GetBinSize()/2; }
	double GeMxVal() const { return BinValV.Last() + GetBinSize()/2; }
	int GetBins() const { return Bins; }

	bool Empty() const { return TotalCount == 0; }

	/// generate n samples from a discrete probability distribution with histogram
	/// defined in BinCountV
	static void GenSamples(const TFltV& CountV, const int& NTrials, TFltV& SimBinV,
			TRnd& Rnd);

private:
	double GetBinSize() const { return BinValV.Len() >= 2 ? BinValV[1] - BinValV[0] : 0; }
};

//////////////////////////////////////////////////
// State Identifier
class TStateIdentifier {
private:
  	static const int MX_ITER;
  	static const int TIME_HIST_BINS;

    typedef TVec<THistogram> TFtrHistV;
    typedef TVec<TFtrHistV> TStateFtrHistVV;
    typedef TVVec<TFtrHistV> THistMat;

  	TRnd Rnd;

  	// clustering
  	PDenseKMeans KMeans;
  	// holds centroids as column vectors
  	TFltVV ControlCentroidVV;
  	TFltVV IgnoredCentroidVV;
  	// holds pairs <n,sum> where n is the number of points assigned to the
  	// centroid at index i and sum is the sum of distances of all the points
  	// assigned to the centroid to the centroid
  	TUInt64FltPrV CentroidDistStatV;

  	int NHistBins;						// the number of bins used in a histogram
  	TStateFtrHistVV ObsHistVV;			// histograms of observation features
  	TStateFtrHistVV ControlHistVV;		// histograms of control features
  	TStateFtrHistVV IgnoredHistVV;		// histograms of the ignored features
  	// time histograms
  	TVec<THistogram> StateTimeHistV;	// holds the global time histograms
  	TVec<THistogram> StateYearHistV;	// holds the yearly time histograms
  	TVec<THistogram> StateMonthHistV;	// holds the monthly time histograms
  	TVec<THistogram> StateWeekHistV;	// holds the weekly time histograms
  	TVec<THistogram> StateDayHistV;		// holds the daily time histograms


  	TVec<TFltV> StateContrFtrValVV;

  	bool IncludeTmFtrV;
  	uint64 TmUnit;
  	double Sample;

  	bool Verbose;
  	PNotify Notify;

public:
  	enum TTmHistType: uchar {
  		thtYear = 0,
		thtMonth = 1,
		thtWeek = 2,
		thtDay = 3
  	};

  	TStateIdentifier(const PDenseKMeans& KMeans, const int NHistBins, const double& Sample,
  			const bool IncludeTmFtrV, const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
	TStateIdentifier(TSIn& SIn);

	virtual ~TStateIdentifier();
	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// performs the clustering
	void Init(const TStreamStory& StreamStory, const TUInt64V& TmV, const TFltVV& ObsFtrVV,
			const TFltVV& ControlFtrVV, const TFltVV& IgnoredFtrVV);
	// initializes histograms for every feature
	void InitHistograms(const TStreamStory& StreamStory, const TFltVV& ObsMat, const TFltVV& ControlFtrVV,
			const TFltVV& IgnoredFtrVV, const TIntV& AssignV);
	void InitTimeHistogramV(const TUInt64V& TmV, const TIntV& AssignV, const int& Bins);

	// assign methods
	// assign instances to centroids
	int Assign(const uint64& RecTm, const TFltV& FtrV) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	void Assign(const TUInt64V& RecTmV, const TFltVV& FtrVV, TIntV& AssignV) const;

	// distance methods
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	void GetCentroidDistV(const uint64& RecTm, const TFltV& FtrV, TFltV& DistV) const;
	void GetCentroidDistVV(const TUInt64V& RecTmV, const TFltVV& FtrVV, TFltVV& DistVV) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const uint64& RecTm, const int& CentroidId, const TFltV& Pt) const;

	// returns the coordinates of a "joined" centroid
	void GetJoinedCentroid(const int& FtrSpaceN, const TIntV& StateIdV, TFltV& FtrV) const;
	void GetAllCentroid(const int& StateId, TFltV& FtrV) const;

	// cluster statistics
	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidId) const;
	// returns the number of points in the cluster
	uint64 GetStateSize(const int& ClustId) const;

	void GetHistogram(const TStreamStory& StreamStory, const int& FtrId,
			const TAggState& AggState, TFltV& BinValV, TFltV& BinV, const bool& NormalizeP=true) const;
	void GetGlobalTimeHistogram(const TAggState& AggState, TUInt64V& TmV, TFltV& BinV,
			const int NBins = -1, const bool& NormalizeP=true) const;
	void GetTimeHistogram(const TAggState& AggState, const TTmHistType& HistType, TIntV& BinValV,
			TFltV& BinV) const;

	int GetStates() const { return KMeans->GetClusts(); }

	int GetControlFtrVDim() const { return ControlCentroidVV.GetRows(); }

	void GetCentroidVV(TFltVV& CentroidVV) const;
	const TFltVV& GetRawCentroidVV() const { return KMeans->GetCentroidVV(); }
	void GetControlCentroidVV(const TStreamStory& StreamStory, TStateFtrVV& StateFtrVV) const;

	// manual setting of control features
	double GetControlFtr(const int& StateId, const TFtrInfo& FtrInfo, const double& DefaultVal) const;
	void SetControlFtr(const int& StateId, const TFtrInfo& FtrInfo,
			const double& Val);
	void ClearControlFtr(const int& StateId, const TFtrInfo& FtrInfo);
	void ClearControlFtrVV();
	bool IsControlFtrSet(const int& StateId, const TFtrInfo& FtrInfo) const;
	bool IsAnyControlFtrSet(const TFtrInfoV& FtrInfoV) const;

	// sets the log to verbose or none
	void SetVerbose(const bool& Verbose);

protected:
	// used during initialization
	void InitStatistics(const TUInt64V& RecTmV, const TFltVV& FtrVV, const TIntV& AssignV);

private:
	void InitCentroidVV(const TIntV& AssignV, const TFltVV& FtrVV, TFltVV& CentroidVV);
	// returns the coordinates of the centroid with the specified ID
	void GetObsCentroid(const int& StateId, TFltV& FtrV) const;
	void GetControlCentroid(const int& StateId, TFltV& FtrV) const;
	void GetIgnoredCentroid(const int& StateId, TFltV& FtrV) const;

	double GetControlFtr(const int& StateId, const TFtrInfo& FtrInfo) const;
	void GetControlFtrV(const int& StateId, const TFtrInfo& FtrInfo, TFltV& FtrV) const;

	void ClearControlFtrVV(const int& Dim);

	void GetHistogram(const TStateFtrHistVV& StateHistVV, const int& FtrN,
			const TFtrInfo& FtrInfo, const TIntV& AggState, TFltV& BinStartV, TFltV& BinV,
			const bool& NormalizeP) const;

	// histograms
	void InitHistVV(const TFtrInfoV& FtrInfoV, const int& NInst, const TFltVV& FtrVV,
			TStateFtrHistVV& HistVV);
	void InitHists(const TStreamStory& StreamStory, const TFltVV& ObsFtrVV,
			const TFltVV& ContrFtrVV, const TFltVV& IgnoredFtrVV);

	void GenClustFtrVV(const TUInt64V& TmV, const TFltVV& ObsFtrVV, TFltVV& FtrVV) const;
	void GenClustFtrV(const uint64& RecTm, const TFltV& FtrV, TFltV& ClustFtrV) const;
	void GenTmFtrV(const uint64& RecTm, TFltV& FtrV) const;

	static void UpdateHistVV(const TFtrInfoV& FtrInfoV, const TFltVV& FtrVV,
			const TIntV& AssignV, const int& States, TStateFtrHistVV& StateFtrHistVV);
	static void GetJoinedCentroid(const TIntV& StateIdV,
			const TFltVV& CentroidVV, const TUInt64V& StateSizeV, TFltV& FtrV);
	static void ResampleHist(const int& Bins, const TFltV& OrigBinValV, const TIntV& OrigBinV, TFltV& BinValV,
				TFltV& BinV);
	static int GetTmFtrDim(const uint64& TmUnit);
};

class TEuclMds {
public:
	// projects the points stored in the column of X onto d
	// dimensions
	static void Project(const TFltVV& X, TFltVV& ProjVV, const int& d=2);
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
	static void GetRevPMat(const TFltVV& PMat, TFltVV& RevPMat);

	static void GetStatDistV(const TFltVV& PMat, TFltV& DistV);
};

/////////////////////////////////////////////////////////////////
// Continuous time Markov chain
class TCtMChain {
private:
	static const double EPS;

public:
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

	// aggregation and partitioning
	/// get a model where states are aggregated
	static void GetAggrQMat(const TFltVV& QMat, const TAggStateV& AggStateV, TFltVV& AggrQMat);
	static void GetSubChain(const TFltVV& QMat, const TIntV& StateIdV, TFltVV& SubQMat);

	static void BiPartition(const TFltVV& QMat, const TFltV& ProbV, TIntV& PartV);
	/// recursive algorithm to partition a Markov chain
	static void Partition(const TFltVV& QMat, TIntV& HierarchV, TFltV& HeightV);

	// distance measures
	static double WassersteinDist1(const TFltVV& QMat, const TAggStateV& AggStateV);
	static double RelativeEntropy(const TFltVV& QMat, const TAggStateV& AggStateV);

private:
	static void AddAggSet(const TFltVV& QMat, const int& StateN, const TAggStateV& AggStateV,
			const TIntV& StateIdV, const int& ParentId, const double& MinDist,
			TIntV& HierarchV, TFltV& HeightV);
	static void SplitAggState(const int& StateN, const TIntV& PartV, TAggStateV& AggStateV,
			TIntV& StateIdV, int& CurrStateId);
};

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtmcModeller {//: public TTransitionModeler {
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

	int NStates;

	int CurrStateId;
	uint64 PrevJumpTm;

	TIntensModel IntensModel;

	bool HasHiddenState;
	// stores how many jump from the hidden state to the specified state occurred
	TIntV HiddenStateJumpCountV;

	uint64 TimeUnit;
	double DeltaTm;

	double TmHorizon;
	double PredictionThreshold;
	int PdfBins;

	bool Verbose;
	PNotify Notify;

public:
	TCtmcModeller(const uint64& TimeUnit, const double& DeltaTm, const bool& Verbose=false);
	TCtmcModeller(TSIn& SIn);

    // saves the model to the output stream
	void Save(TSOut& SOut) const;
	static TCtmcModeller* Load(TSIn& SIn);

	// initialization
	void Init(const TFltVV& FtrVV, const int& NStates, const TIntV& StateAssignV,
			const TUInt64V& TmV, const bool SequencedData, const TBoolV& SequenceEndV);

	void OnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch);

	void GetFutureProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbV) const;
	void GetPastProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, const double& Tm,
			TIntFltPrV& StateIdProbV) const;

	// returns the most likely next states excluding the current state
	void GetNextStateProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const;
	// returns the most likely previous states excluding the current state
	void GetPrevStateProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
			const int& NFutStates) const;

	void GetLikelyPathTree(const int& StartStateId, const TIntV& StateIdV, const TAggStateV& AggStateV,
			const TStateFtrVV& StateFtrVV, const int& MxDepth, TFltVV& PMat, TIntV& PMatStateIdV,
			const double& TransThreshold=0.0) const;

	void GetProbVAtTime(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const TStateIdV& StateIdV, const int& StartStateId, const double& Tm,
			TFltV& ProbV) const;
	// approximates the probability of jumping into the target state in the pre-specified
	// time horizon
	bool PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TAggStateV& StateSetV,
			const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
			double& Prob, TFltV& ProbV, TFltV& TmV) const;

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	void GetStatDist(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltV& ProbV) const;

	void GetJumpVV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV, TFltVV& JumpVV) const;

	// Q-Matrix
	void GetQMatrix(const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV,
			TFltVV& QMat) const;
	void GetSubQMatrix(const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV,
			const TAggState& TargetState, TFltVV& SubQMat);

	void GetHoldingTmV(const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV,
			TFltV& HoldingTmV) const;

	// returns true if the jump from OldStateId to NewStateId has a low enough probability
	bool IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const;

	// get model state variables
	int GetCurrStateId() const { return CurrStateId; };

	// get methods
	int GetStates() const { return NStates; }
	const uint64& GetTimeUnit() const { return TimeUnit; }

	// get/set methods
	int GetPdfBins() const { return PdfBins; }
	void SetPdfBins(const int Bins) { PdfBins = Bins; }

	double GetPredictionThreshold() const { return PredictionThreshold; }
	void SetPredictionThreshold(const double& Threshold) { PredictionThreshold = Threshold; }

	double GetTimeHorizon() const { return TmHorizon; }
	void SetTimeHorizon(const double& Horizon) { TmHorizon = Horizon; }

	void SetVerbose(const bool& Verbose);

	// static methods
	static void GetJumpVV(const TFltVV& QMat, TFltVV& JumpVV) { TCtMChain::GetJumpVV(QMat, JumpVV); }
	static void GetHoldingTmV(const TFltVV& QMat, TFltV& TmV) { TCtMChain::GetHoldingTmV(QMat, TmV); }
protected:
	void AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch);

	// get future state probabilities for all the states for a fixed time in the future
	void GetFutureProbVV(const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const;
	void GetPastProbVV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			const double& Tm, TFltVV& ProbVV) const;

	void InitIntensities(const TFltVV& FtrVV, const TUInt64V& TmV, const TIntV& AssignV,
			const TBoolV& EndBatchV);
	// prints the statistics used to build the Q-matrix
	const TStr GetType() const { return "continuous"; }

private:
	void GetStateIntensV(const int StateId, const TFltV& FtrV, TFltV& IntensV) const;

	// returns the intensity matrix (Q-matrix)
	void GetQMatrix(const TStateFtrVV& StateFtrVV, TFltVV& QMat) const;
	// returns a Q matrix for the joined states for the time reversal Markov chain
	void GetRevQMatrix(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
			TFltVV& RevQMat) const;

	// handle the hidden state
	void InsHiddenState(TAggStateV& StateSetV) const;
	void InsHiddenState(TStateIdV& StateIdV) const;
	bool IsHiddenStateId(const int& StateId) const;
	int GetHiddenStateId() const;
	void RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const;

	PJsonVal GetLikelyPathTree(const int& StartStateId, const TIntV& StateIdV, const TAggStateV& AggStateV,
			const TStateFtrVV& StateFtrVV, const int& MxDepth, int& GeneratedStates, const double& TransThreshold=0.0) const;
	int PMatFromTree(const PJsonVal& TreeJson, int& CurrStateN, TFltVV& PMat,
			TIntV& PMatStateIdV) const;

	static void GetNextStateProbV(const TFltVV& QMat, const TStateIdV& StateIdV,
			const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
			const PNotify& Notify);

	static void GetFutureProbVV(const TFltVV& QMat, const double& Tm,
			const double& DeltaTm, TFltVV& ProbVV, const bool HasHiddenState=false);
};

/////////////////////////////////////////////////////////////////
// Scale helper
class TScaleHelper {
	typedef TVec<TPair<TFlt,TFltVV>> TScaleDescV;
protected:
	PNotify Notify;
public:
	TScaleHelper(const PNotify _Notify): Notify(_Notify) {}
	virtual ~TScaleHelper() {}

	void CalcNaturalScales(const TScaleDescV& ScaleQMatPrV,
		const TRnd& Rnd, TFltV& ScaleV) const;

protected:
	virtual void GetScaleFtrV(const TFltVV& QMat, TFltV& FtrV) const = 0;
	virtual int GetFtrVDim() const = 0;
};

/////////////////////////////////////////////////////////////////
// Scale helper - based on singular values of the Q-matrix
class TEigValScaleHelper: public TScaleHelper {
private:
	static const int FTRV_DIM;
public:
	TEigValScaleHelper(const PNotify& Notify): TScaleHelper(Notify) {}
protected:
	void GetScaleFtrV(const TFltVV& QMat, TFltV& FtrV) const;
	int GetFtrVDim() const { return FTRV_DIM; }
};

////////////////////////////////////////////
// Hierarchy modeler
class THierarch {
private:
    // a vector which describes the hierarchy. each state has its own index
    // and the value at index i is the index of i-ths parent
    TIntV HierarchV;

    // state heights in the hierarchy
    TFltV StateHeightV, UniqueHeightV;
    TFltV NaturalScaleV;
    TFlt MxHeight;

    // past states
    int HistCacheSize;
    TVec<TStateIdV> PastStateIdV;	// TODO not optimal structure
    TVec<TUInt64IntPrV> HierarchHistoryVV;

    // number of leaf states, these are stored in the first part of the hierarchy vector
    int NLeafs;

    TStrV StateNmV;
    TStrV StateLabelV;

    TIntFltPrSet TargetIdHeightSet;
    bool IsTransitionBased;

    TRnd Rnd;

    bool Verbose;
    PNotify Notify;

public:
    THierarch(const bool& HistCacheSize, const bool& IsTransitionBased,
    		const TRnd& Rnd, const bool& Verbose=false);
    THierarch(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static THierarch* Load(TSIn& SIn);

	void Init(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV, const int& CurrLeafId,
	        const TStreamStory& StreamStory);
	void UpdateHistory(const int& CurrLeafId);

	const TFltV& GetUniqueHeightV() const { return UniqueHeightV; }
	const TFltV& GetUiHeightV() const { return NaturalScaleV; }
	const TIntV& GetHierarchV() const { return HierarchV; }

	double GetMinHeight() const { return UniqueHeightV[0]; }
	double GetUiMinHeight() const { return NaturalScaleV[0]; }

	double GetStateHeight(const int& StateId) const { return StateHeightV[StateId]; }
	// return a list of state IDs and their heights
	void GetStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	// returns the 'joined' states at the specified height, puts teh state IDs into StateIdV
	// and sets of their leafs into JoinedStateVV
	void GetStateSetsAtHeight(const double& Height, TStateIdV& StateIdV, TAggStateV& AggStateV) const;
	// returns all the states just below the specified height
	void GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const;
	// returns the next level of the level passed as the argument along with it's height
	double GetNextLevel(const TIntV& CurrLevelIdV, TIntV& NextLevelIdV) const;
	double GetNextUiLevel(const TIntV& CurrLevelIdV, TIntV& NextLevelIdV) const;
	// fills the vector with IDs of the ancestors of the given state along with their heights
	void GetAncestorV(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns the ID of the ancestor of the given leaf at the specified height
	int GetAncestorAtHeight(const int& LeafId, const double& Height) const;
	// fills the vector with leaf descendants
	void GetLeafDescendantV(const int& StateId, TIntV& DescendantV) const;
	void GetLeafIdV(TIntV& LeafIdV) const;
	void GetDescendantsAtHeight(const double& Height, const TIntV& StateIdV, TAggStateV& AggStateV);

	void GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	void GetUiCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const;
	/// returns the last few states that occurred in the real-time data stream
	void GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const;
	/// returns the whole history of states on the given scale
	void GetStateHistory(const double& RelOffset, const double& RelRange,
	        const int& MxStates, const double& Scale, uint64& GlobalMnDur,
	        TStateHist& TmDurStateIdPercHTrV) const;
	/// returns the whole history of states on all the scales
	void GetStateHistory(const double& RelOffset, const double& RelRange,
            const int& MxStates,
            TScaleStateHistV& ScaleTmDurIdTrV,
            uint64& MnTm, uint64& MxTm) const;

	// for each state returns the number of leafs it's subtree has
	void GetLeafSuccesorCountV(TIntV& LeafCountV) const;

	// returns the total number of states in the hierarchy
	int GetStates() const { return HierarchV.Len(); }
	// returns the number of leafs in the hierarchy
	int GetLeafs() const { return NLeafs; }

	bool IsStateNm(const int& StateId) const;
	void SetStateNm(const int& StateId, const TStr& StateNm);
	const TStr& GetStateNm(const int& StateId) const;
	const TStr& GetStateLabel(const int& StateId) const;

	// set/remove target states
	bool IsTarget(const int& StateId) const;
	bool HasTargetStates() const;
	void SetTarget(const int& StateId);
	void RemoveTarget(const int& StateId);

	bool IsLeaf(const int& StateId) const;

	const TIntFltPrSet& GetTargetStateIdSet() const { return TargetIdHeightSet; }

	void SetVerbose(const bool& Verbose);
	void PrintHierarch() const;

private:
	void InitHierarchyDist(const TStateIdentifier& StateIdentifier);
	void InitHierarchyTrans(const TStreamStory& StreamStory);

	// returns the ID of the parent state
	int GetParentId(const int& StateId) const;
	// returns the height of the state
	int GetHeightN(const double& Height) const;
	int GetUiScaleN(const double& Height) const;
	double GetNearestHeight(const double& InHeight) const;

	bool IsRoot(const int& StateId) const;
	bool IsOnHeight(const int& StateId, const double& Height) const;
	bool IsBelowHeight(const int& StateId, const double& Height) const;
	bool IsAboveHeight(const int& StateId, const double& Height) const;
	bool IsStateId(const int& StateId) const { return 0 <= StateId && StateId < HierarchV.Len(); }

	// returns a hash table with keys being the states at the specified height
	// and the values containing their successor leafs
	void GetAncSuccH(const double& Height, TIntIntVH& StateSubStateH) const;

	// internal methods used during initialization
	// returns the index of the oldest ancestor of the state
	// this method is only used when initially building the hierarchy
	int GetOldestAncestIdx(const int& StateIdx) const;

	// methods for calculating the natural scales in the model
	void CalcNaturalScales(const TStreamStory& StreamStory, const TRnd& Rnd, TFltV& ScaleV) const;
	/// initializes the history of hierarchies
	void InitHistHierarch(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV,
	        const TStateIdentifier& StateIdentifier);

	double GetNextUiHeight(const double& Height) const {
		const TFltV& UiHeightV = GetUiHeightV();
		int HeightN = 0;
		while (HeightN < UiHeightV.Len() && UiHeightV[HeightN] <= Height) {
			HeightN++;
		}
		return UiHeightV[HeightN];
	}

	// static functions
	static TInt& GetParentId(const int& StateId, TIntV& HierarchV) { return HierarchV[StateId]; }
	static int GetParentId(const int& StateId, const TIntV& HierarchV) { return HierarchV[StateId]; }
	static int GetGrandparentId(const int& StateId, const TIntV& HierarchV)
		{ return GetParentId(GetParentId(StateId, HierarchV), HierarchV); }
	static bool IsRoot(const int& StateId, const TIntV& HierarchV);
	// returns a vector of unique heights
	static void GenUniqueHeightV(const TFltV& HeightV, TFltV& UniqueHeightV);

	static int FindScaleNBin(const double& Scale, const TFltV& ScaleV);

	static bool HaveSameKeys(const TIntFltH& Hash1, const TIntFltH& Hash2) {
	    if (Hash1.Len() != Hash2.Len()) { return false; }

	    int KeyId = Hash1.FFirstKeyId();
	    while (Hash1.FNextKeyId(KeyId)) {
	        const int& Key1 = Hash1.GetKey(KeyId);
	        if (!Hash2.IsKey(Key1)) {
	            return false;
	        }
	    }

	    return true;
	}
	static double GetStateProbDiff(const TIntFltH& DistH1, const TIntFltH& DistH2) {
	    double Diff = 0;

	    int KeyId = DistH1.FFirstKeyId();
	    while (DistH1.FNextKeyId(KeyId)) {
	        const int& Key1 = DistH1.GetKey(KeyId);

	        Diff += TFlt::Abs(DistH1.GetDat(Key1) - DistH2.GetDat(Key1));
	    }

	    return Diff;
	}
	// clears the state
	void ClrFlds();
};

/////////////////////////////////////////////////////////////////
// UI helper
class TUiHelper {
public:
	typedef TTriple<TUCh, TInt, TInt> TTmDesc;
	typedef TVec<TTmDesc> TTmDescV;

	enum TNumAutoNmLevel: uchar {
		nanlLowest = 0x80,
		nanlLow = 0x81,
		nanlMeduim = 0x82,
		nanlHigh = 0x83,
		nanlHighest = 0x84
	};

	enum TDescType {
	    dtNormal,
	    dtShort
	};

	class TAutoNmDesc;
	typedef TPt<TAutoNmDesc> PAutoNmDesc;
	class TAutoNmDesc {
	protected:
		TCRef CRef;
	public:
		friend class TPt<TAutoNmDesc>;
	protected:
		int FtrN;
		double PVal;

	public:
		TAutoNmDesc(const int& FtrN, const double& PVal);
		TAutoNmDesc(TSIn& SIn);
		virtual ~TAutoNmDesc() {}

		static PAutoNmDesc Load(TSIn& SIn);
		virtual void Save(TSOut& SOut) const;

		bool operator <(const TAutoNmDesc& Othr) { return GetPVal() < Othr.GetPVal(); }

		int GetFtrId() const { return FtrN; }
		double GetPVal() const { return PVal; }

		virtual TFtrType GetFtrType() const = 0;
		virtual PJsonVal GetJson() const = 0;
		virtual PJsonVal GetNarrateJson() const = 0;
	};

	class TNumAutoNmDesc: public TAutoNmDesc {
	private:
		TNumAutoNmLevel Level;

	public:
		TNumAutoNmDesc(const int& FtrN, const double& PVal, const TNumAutoNmLevel& Level);
		TNumAutoNmDesc(TSIn& SIn);

		void Save(TSOut& SOut) const;

		TFtrType GetFtrType() const { return ftNumeric; }
		PJsonVal GetJson() const;
		PJsonVal GetNarrateJson() const;

		const TNumAutoNmLevel& GetLevel() const { return Level; }
		TStr GetAutoNmLowHighDesc() const;

		static TNumAutoNmLevel GetAutoNmLevel(const double& PVal, const double& LowPercPVal,
				const double& HighPercPVal);
	};

	class TCatAutoNmDesc: public TAutoNmDesc {
	private:
		int BinN;

	public:
		TCatAutoNmDesc(const int& FtrN, const double& PVal, const int& BinN);
		TCatAutoNmDesc(TSIn& SIn);

		void Save(TSOut& SOut) const;

		TFtrType GetFtrType() const { return ftCategorical; }
		PJsonVal GetJson() const;
		PJsonVal GetNarrateJson() const;

		const int& GetBin() const { return BinN; }
	};

	class TAutoNmTmDesc: public TAutoNmDesc {
	private:
	    TStrPr FromToStrPr;

	public:
	    TAutoNmTmDesc(const TTmDesc& TmDesc);
	    TAutoNmTmDesc(TSIn& SIn);

	    void Save(TSOut& SOut) const;
	private:
	    TFtrType GetFtrType() const { return ftTime; }
	    PJsonVal GetJson() const;
	    PJsonVal GetNarrateJson() const;
	};

	typedef TVec<PAutoNmDesc> PAutoNmDescV;
	typedef TVec<PAutoNmDescV> PAutoNmDescVV;

private:

	// state sizes and coordinates
	static const double RADIUS_FACTOR;
	static const double STEP_FACTOR;
	static const double INIT_RADIUS_FACTOR;
	static const double STATE_OCCUPANCY_PERC;

	// automatic labels
	static const double LOW_PVAL_THRESHOLD;
	static const double LOWEST_PVAL_THRESHOLD;
	static const double STATE_LOW_PVAL_THRESHOLD;

	// time descriptions
	static const TStr MONTHS[12];
	static const TStr MONTHS_SHORT[12];
	static const TStr DAYS_IN_MONTH[31];
	static const TStr DAYS_IN_WEEK[7];
	static const TStr DAYS_IN_WEEK_SHORT[7];
	static const TStr HOURS_IN_DAY[24];

	TFltPrV StateCoordV;
	PAutoNmDescV StateAutoNmV;
	PAutoNmDescVV StateIdAutoNmDescVV;
	TVec<TTmDescV> StateIdOccTmDescV;

	TRnd Rnd;

	bool Verbose;
	PNotify Notify;
public:
	TUiHelper(const TRnd& Rnd, const bool& Verbose);
	TUiHelper(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Init(const TStreamStory& StreamStory);

	const TFltPr& GetStateCoords(const int& StateId) const;
	void SetStateCoords(const int& StateId, const double& x, const double& y);
	void SetStateCoords(const TFltPrV& CoordV);
	void GetStateRadiusV(const TFltV& ProbV, TFltV& SizeV) const;

	// time descriptions
	const TUiHelper::PAutoNmDesc& GetStateAutoNm(const int& StateId) const;
	void GetAutoNmPValDesc(const int& StateId, PAutoNmDescV& Desc) const;
	void GetTmDesc(const int& StateId, TStrPrV& DescIntervalV) const;

private:
	TFltPr& GetModStateCoords(const int& StateId);

	void SetStateAutoNm(const int& StateId, const TUiHelper::PAutoNmDesc& Desc);

	// computes the coordinates (in 2D) of each state
	void InitStateCoordV(const TStreamStory& StreamStory);
	void RefineStateCoordV(const TStreamStory& StreamStory);
	void InitAutoNmV(const TStreamStory& StreamStory);
	void RefineAutoNmV();
	void InitStateExplain(const TStreamStory& StreamStory);

	bool HasMxPeaks(const int& MxPeakCount, const double& PeakMassThreshold, const TFltV& PdfHist,
			 TIntPrV& PeakBorderV, double& PeakMass, int& PeakBinCount) const;

	void GetTmDesc(const int& StateId, TTmDescV& DescV) const;

	static void GetFromToStrPr(const TTmDesc& Desc, TStrPr& StrDesc,
	        const TDescType& DescType);
	static double GetStateRaduis(const double& Prob);
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
class TStateAssist {
private:
	TVec<TLogReg> ClassifyV;
	TVec<TDecisionTree> DecisionTreeV;
	TFltPrV FtrBoundV;

	TRnd Rnd;

	bool Verbose;
	PNotify Notify;

public:
	TStateAssist(const bool Verbose);
	TStateAssist(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Init(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
			const TFltVV& IgnFtrVV, const TStateIdentifier& Clust, const THierarch& Hierarch,
			TStreamStoryCallback* Callback, const bool& MultiThread=true);
	void InitFtrBounds(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const TFltVV& IgnoredFtrVV);

	const TFltPr& GetFtrBounds(const int& FtrId) const;
	void GetFtrWgtV(const int& StateId, const int& Offset, const int& Length,
			TFltV& WgtV) const;
	PJsonVal GetStateClassifyTree(const int& StateId) const;
	PJsonVal GetStateExplain(const int& StateId) const;

private:
	void InitSingle(const TFltVV& FtrVV, const int& StateId, const double& Height,
			const THierarch& Hierarch, const TIntV& AssignV, TRnd& Rnd, TLogReg& LogReg,
			TDecisionTree& Tree);
	void FitAssistModels(const TFltVV& FtrVV, const TFltV& LabelV, TLogReg& LogReg,
			TDecisionTree& DecisionTree);
};

/////////////////////////////////////////////////////////////////
// Activity detector
class TActivityDetector {
	typedef TIntSet TActivityStep;
	typedef TVec<TIntSet> TActivityStepV;

private:
	class TActivity {
	private:
		TIntV ActivitySeq;
		TActivityStepV UniqueStepV;
//		TUInt64IntPrV StepTmStepIdV;		// represents the history
		TUInt64UInt64IntTrV StepStartEndTmIdV;	// represents the history, is not serialized with the rest of the variables

	public:
		TActivity();
		TActivity(const TActivityStepV& StepV);
		TActivity(TSIn& SIn);

		void Save(TSOut& SOut) const;

		bool Update(const uint64& Tm, const int& StateId, const PNotify& Notify);
		bool Detect(uint64& StartTm, uint64& EndTm, const PNotify& Notify);

		int GetNumSteps() const { return ActivitySeq.Len(); }

	private:
		int GetStepId(const int& StateId) const;
		bool ContainsStep(const TActivityStep& Step) const;
	};

	typedef THash<TStr, TActivity> TActivityH;

	static const int STATE_CACHE_SIZE;

	TActivityH ActivityH;
	TUInt64IntPrV StateIdHistoryV;

	TStreamStoryCallback* Callback;

	bool Verbose;
	PNotify Notify;

public:
	TActivityDetector(const bool& Verbose);
	TActivityDetector(TSIn& SIn);
	~TActivityDetector() {}

	void Save(TSOut& SOut) const;

	void OnStateChanged(const uint64& Tm, const int& NewStateId);

	void AddActivity(const TStr& ActName, const TActivityStepV& StepV);
	void RemoveActivity(const TStr& ActNm);
	void GetActivities(TStrV& ActNmV, TIntV& NumStepsV) const;

	bool IsEmpty() const;

	void SetCallback(TStreamStoryCallback* Callback);
	void SetVerbose(const bool& Verbose);
};

/////////////////////////////////////////////////////////////////
// StreamStory
class TStreamStory {
private:
	TStateIdentifier* StateIdentifier;
	TCtmcModeller* MChain;
    THierarch* Hierarch;
    TStateAssist* StateAssist;
    TActivityDetector* ActivityDetector;
    TUiHelper* UiHelper;

    TIntV FtrNToIdV;
    TFtrInfoV ObsFtrInfoV;
    TFtrInfoV ContrFtrInfoV;
    TFtrInfoV IgnFtrInfoV;

    bool FtrVecPredP;

    TFltV LastObsFtrV, LastContrFtrV;
    int LastStateId;
    uint64 LastRecTm;

    bool Verbose;

    TStreamStoryCallback* Callback;

    PNotify Notify;

public:
    // constructors
    TStreamStory();
    TStreamStory(TStateIdentifier* Clust, TCtmcModeller* MChain, THierarch* Hierarch,
    		const TRnd& Rnd=TRnd(0), const bool& Verbose=true);
    TStreamStory(TSIn& SIn);

    ~TStreamStory();

    // saves the model to the output stream
	void Save(TSOut& SOut) const;

	// saves this model as JSON
	PJsonVal GetJson() const;
	PJsonVal GetSubModelJson(const int& StateId) const;
	PJsonVal GetLikelyPathTreeJson(const int& StateId, const double& Height, const int& Depth,
			const double& TransThreshold) const;

	// update methods
	// initializes the model
	void Init(const TFtrInfoV& ObsFtrInfoV, const TFtrInfoV& ContrFtrInfoV,
			const TFtrInfoV& IgnFtrInfoV, const TFltVV& ObservFtrVV,
			const TFltVV& ControlFtrVV, const TFltVV& IgnoredFtrVV, const TUInt64V& RecTmV,
			const bool& MultiThread=true);
	void InitBatches(const TFtrInfoV& ObsFtrInfoV, const TFtrInfoV& ContrFtrInfoV,
			const TFtrInfoV& IgnFtrInfoV, const TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
			const TFltVV& IgnoredFtrVV, const TUInt64V& RecTmV, const TBoolV& BatchEndV,
			const bool& MultiThread=true);
	void InitFtrNToIdV();
	void InitClust(const TUInt64V& TmV, const TFltVV& ObsFtrVV, const TFltVV& FtrVV,
			const TFltVV& IgnoredFtrVV, TIntV& AssignV);
	void InitMChain(const TFltVV& FtrVV, const TIntV& AssignV, const TUInt64V& RecTmV,
			const bool IsBatchData, const TBoolV& EndBatchV);
	/// initializes the hierarchy
	void InitHierarch(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV);
	void InitHistograms(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const TFltVV& IgnoredFtrVV,
			const TUInt64V& RecTmV, const TBoolV& BatchEndV);
	void InitStateAssist(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV,
			const TFltVV& ContrFtrVV, const TFltVV& IgnFtrVV, const bool& MultiThread);

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

	void PredictNextState(const bool& UseFtrVP, const int& FutStateN,
			TVec<TPair<TFlt, TIntFltPrV>>& HeightStateIdProbPrVPrV) const;

	// histograms
	void GetHistogram(const int& StateId, const int& FtrId, TFltV& BinValV, TFltV& CountV,
			TFltV& AllCountV) const;
	void GetTransitionHistogram(const int& SourceId, const int& TargetId,
			const int& FtrId, TFltV& BinStartV, TFltV& SourceCountV, TFltV& TargetCountV,
			TFltV& AllCountV) const;
	void GetGlobalTimeHistogram(const int& StateId, TUInt64V& TmV, TFltV& ProbV,
			const int& NBins=-1) const;
	void GetTimeHistogram(const int& StateId, const TStateIdentifier::TTmHistType& HistType,
			TIntV& BinV, TFltV& ProbV) const;
	void GetStateHistory(const double& RelOffset, const double& RelRange,
	        const int& MxStates,
	        TVec<TPair<TFlt, TVec<TTriple<TUInt64,TUInt64,TIntFltH>>>>& ScaleTmDurIdTrPrV,
	        uint64& MnTm, uint64& MxTm) const;

	// state explanations
	PJsonVal GetStateWgtV(const int& StateId) const;
	PJsonVal GetStateClassifyTree(const int& StateId) const;
	PJsonVal GetStateExplain(const int& StateId) const;

	// stores the transition model for the current height into Mat
	void GetTransitionModel(const double& Height, TFltVV& Mat) const;
	// returns a list of ancestor states along with their heights
	void GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const;
	// returns a list of ancestors of the current state
	void GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const;
	// returns the current state on the specified level
	int GetCurrStateId(const double& Height) const;
	// returns the centroid of the given state
	void GetCentroid(const int& StateId, const int& FtrSpaceN, TFltV& FtrV) const;
	void GetCentroidVV(const int& StateId, TVec<TFltV>& FtrVV) const;
	// returns the IDs of all the states on the specified height
	void GetStateIdVAtHeight(const double& Height, TStateIdV& StateIdV) const;
	// returns the number of states in the hierarchy
    int GetStates() const { return Hierarch->GetStates(); }
    uint64 GetTimeUnit() const;
    uint64 GetLastRecTm() const { return LastRecTm; }

    // target methods
    bool IsTargetState(const int& StateId) const { return Hierarch->IsTarget(StateId); }
    void SetTargetState(const int& StateId, const bool& IsTrg);

    void AddActivity(const TStr& ActName, const TVec<TIntV>& StateIdSeqVV);
    void RemoveActivity(const TStr& ActNm);
    void GetActivities(TStrV& ActNmV, TIntV& NumStepsV) const;

    bool IsLeaf(const int& StateId) const;

    // sets the name of the specified state
    void SetStateNm(const int& StateId, const TStr& StateNm);
    void SetStatePosV(const TFltPrV& PosV);

    // control features
    void SetControlFtrVal(const int& StateId, const int& ContrFtrId, const double& Val);
    void SetControlFtrsVal(const int& ContrFtrId, const double& Val);
    void ResetControlFtrVal(const int& StateId, const int& FtrId);
    void ResetControlFtrVals(const int& StateId);
    void ResetControlFtrVals();
    bool IsAnyControlFtrSet() const;

    const TFltPr& GetFtrBounds(const int& FtrId) const;
    const TStr& GetStateLabel(const int& StateId) const;
    const TUiHelper::PAutoNmDesc& GetStateAutoNm(const int& StateId) const;
    void GetStateFtrPValDesc(const int& StateId, TUiHelper::PAutoNmDescV& DescV) const;
    void GetStateTmDesc(const int& StateId, TStrPrV& StateIntervalV) const;
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
    void SetCallback(TStreamStoryCallback* Callback);

    // feature info
    const TFtrInfoV& GetObsFtrInfoV() const { return ObsFtrInfoV; }
    const TFtrInfoV& GetContrFtrInfoV() const { return ContrFtrInfoV; }
    const TFtrInfoV& GetIgnFtrInfoV() const { return IgnFtrInfoV; }

    int GetObsDim() const { return ObsFtrInfoV.Len(); }
    int GetContrDim() const { return ContrFtrInfoV.Len(); }
    int GetIgnDim() const { return IgnFtrInfoV.Len(); }
    int GetAllDim() const { return GetObsDim() + GetContrDim() + GetIgnDim(); }

    int GetFtrId(const int& FtrN) const;
    const TFtrInfo& GetFtrInfo(const int& FtrId) const;

    bool IsObsFtrId(const int& FtrId) const { return FtrId < GetObsDim(); }
    bool IsContrFtrId(const int& FtrId) const { return GetObsDim() <= FtrId && FtrId < GetObsDim() + GetContrDim(); }
    bool IsIgnFtrId(const int& FtrId) const { return GetObsDim() + GetContrDim() <= FtrId; }

    const TStateIdentifier& GetStateIdentifier() const { EAssert(StateIdentifier != nullptr); return *StateIdentifier; }
    const THierarch& GetHierarch() const { EAssert(Hierarch != nullptr); return *Hierarch; }
    const TCtmcModeller& GetTransitionModeler() const { EAssert(MChain != nullptr); return *MChain; }

    bool IsDetectingActivities() const;
    bool IsPredictingStates() const;

private:
    PJsonVal GetLevelJson(const double& Height, const double& NextHeight, const TStateIdV& StateIdV,
    		const TFltVV& TransitionVV, const TFltV& HoldingTimeV, const TFltV& ProbV,
			const TFltV& RadiusV) const;

    void CreateFtrVV(const TFltVV& ObservFtrMat, const TFltVV& ControlFtrMat,
    		const TUInt64V& RecTmV, const TBoolV& EndsBatchV, TFltVV& FtrMat) const;
    void CreateFtrV(const TFltV& ObsFtrV, const TFltV& ContrFtrV, const uint64& RecTm,
    		TFltV& FtrV) const;

    void GetStateFtrVV(TStateFtrVV& StateFtrVV, const bool& UseFtrVP) const;

    void GetStatsAtHeight(const double& Height, TAggStateV& AggStateV, TStateIdV& StateIdV,
    		TStateFtrVV& StateFtrVV) const;

    void DetectAnomalies(const int& NewStateId, const int& OldStateId, const TFltV& ObsFtrV,
    		const TFltV& FtrV) const;

    void PredictTargets(const uint64& RecTm, const TStateFtrVV& StateFtrVV, const int& CurrStateId) const;

    void CheckBatches(const TUInt64V& TmV, const TBoolV& BatchEndV) const;

    void TreeLayout(const TFltVV& PMat, const TIntV& StateIdV, const TFltV& RadiusV, TFltPrV& PosV) const;
    void CalcTreeWidthV(const TFltVV& PMat, const TFltV& RadiusV, const double& NodePadding,
    		const int& RootN, TFltV& WidthV) const;
    void CalcTreePosV(const TFltVV& PMat, const int& RootN, const TFltV& RadiusV, const TFltV& WidthV,
    		const double& RootX, const double& RootY, TFltPrV& PosV) const;

    void TransformExplainTree(PJsonVal& TreeJson) const;
};

}

#endif

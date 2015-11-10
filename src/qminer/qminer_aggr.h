/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef QMINER_AGGR_H
#define QMINER_AGGR_H

#include "qminer_core.h"
#include "qminer_storage.h"
#include "qminer_ftr.h"

namespace TQm {

namespace TAggrs {
    
///////////////////////////////
// QMiner-Aggregator-Piechart
class TCount : public TAggr {
private:
	//meta-data
	TStr FieldNm;
	TStr JoinPathStr;
	// aggregations
	TInt Count;
	TStrH ValH;

	TCount(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt);
	TCount(const TWPt<TBase>& Base, const TStr& AggrNm,
		const PRecSet& RecSet, const int& KeyId);
public:
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt) {
		return new TCount(Base, AggrNm, RecSet, FtrExt); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const int& KeyId) { 
			return new TCount(Base, AggrNm, RecSet, KeyId); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const PRecSet& RecSet, const PJsonVal& JsonVal);

	PJsonVal SaveJson() const;
    
    // aggregator type name 
    static TStr GetType() { return "count"; }
};

///////////////////////////////
// QMiner-Aggregator-Histogram
class THistogram : public TAggr {
private:
	//meta-data
	TStr FieldNm;
	TStr JoinPathStr;
	// aggregations
	TFlt Sum;
	PMom Mom;
	THist Hist;

	THistogram(const TWPt<TBase>& Base, const TStr& AggrNm,
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& Buckets);
public:
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& Buckets) {
			return new THistogram(Base, AggrNm, RecSet, FtrExt, Buckets); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PJsonVal& JsonVal);

	PJsonVal SaveJson() const;

    // aggregator type name 
    static TStr GetType() { return "histogram"; }
};

///////////////////////////////
// QMiner-Aggregator-Bag-of-Words
class TBow: public TAggr {
protected:
	// sampled record set
	PRecSet SampleRecSet;
	// preprocessed records
	PBowDocBs BowDocBs;
	PBowDocWgtBs BowDocWgtBs;

	TBow(const TWPt<TBase>& Base, const TStr& AggrNm,
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize, 
		const TBowWordWgtType& WgtType = bwwtLogDFNrmTFIDF);

	static void ParseJson(const TWPt<TBase>& Base, const PRecSet& RecSet,
		const PJsonVal& JsonVal, PFtrExt& FtrExt, int& SampleSize);
};

///////////////////////////////
// QMiner-Aggregator-Keyword
class TKeywords : public TBow {
private:
	// aggregations
	PBowKWordSet KWordSet;

	TKeywords(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize);
public:
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize) {
			return new TKeywords(Base, AggrNm, RecSet, FtrExt, SampleSize); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PJsonVal& JsonVal);

	static PJsonVal SaveKWordSet(const PBowKWordSet& KWordSet);
	PJsonVal SaveJson() const;
    
    // aggregator type name 
    static TStr GetType() { return "keywords"; }
   
};

///////////////////////////////
// QMiner-Aggregator-DocAtlas
#ifdef QM_AGGR_DOC_ATLAS
class TDocAtlas : public TBow {
private:
	//meta-data
	TStr FieldNm;
	TStr JoinPathStr;
	// aggregations
	TStr SmallFNm;
	TStr MidFNm;
	TStr LargeFNm;

	TDocAtlas(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize);
public:
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize) {
			return new TDocAtlas(Base, AggrNm, RecSet, FtrExt, SampleSize); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PJsonVal& JsonVal);

	PJsonVal SaveJson() const;
    
    // aggregator type name 
    static TStr GetType() { return "docatlas"; }    
};
#endif

///////////////////////////////
// QMiner-Aggregator-TimeLine
class TTimeLine : public TAggr {
private:
	//meta-data
	TStr FieldNm;
	TStr JoinPathStr;
	// aggregations
	TInt Count;
	TStrH AbsDateH;
	TStrH MonthH;
	TStrH DayOfWeekH;
	TStrH HourOfDayH;

	PJsonVal GetJsonList(const TStrH& StrH) const;

	TTimeLine(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt);
public:
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
		const PRecSet& RecSet, const PFtrExt& FtrExt) {
		return new TTimeLine(Base, AggrNm, RecSet, FtrExt); }
	static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const PRecSet& RecSet, const PJsonVal& JsonVal);

	PJsonVal SaveJson() const;
    
    // aggregator type name 
    static TStr GetType() { return "timeline"; }
};

} // TAggrs namespace

namespace TStreamAggrs {

///////////////////////////////
// Record Id Buffer.
// Delays record Ids for a given amount of new records.
class TRecBuffer : public TStreamAggr {
private:
	TSignalProc::TBuffer<TRec> Buffer;
	
protected:
	void OnAddRec(const TRec& Rec) { Buffer.Update(Rec); }

    TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	// did we finish initialization
	bool IsInit() const { return Buffer.IsInit(); }
    
	// serilization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "recordBuffer"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Time series tick.
// Wrapper for exposing time series to signal processing aggregates 
class TTimeSeriesTick : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	TInt TimeFieldId;
	TInt TickValFieldId;
    // initialized after first value
    TBool InitP;
	// last value
	TFlt TickVal;
	TUInt64 TmMSecs;
	
protected:
	void OnAddRec(const TRec& Rec);

    TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return InitP; }
    
	// current values
	double GetFlt() const { return TickVal; }
	uint64 GetTmMSecs() const { return TmMSecs; }

	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "timeSeriesTick"; }   
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Time series window buffer.
// Wrapper for exposing a window in a time series to signal processing aggregates 
// TODO; use circular buffer
class TWinBuf : public TStreamAggr, public TStreamAggrOut::IFltTmIO,
                public TStreamAggrOut::IFltVec, public TStreamAggrOut::ITmVec {
private:
    TInt TimeFieldId;
    TInt TickValFieldId;
    TUInt64 WinSizeMSecs;   
    // initialized after first value
    TBool InitP;
    //the newest and oldest values
    TFlt InVal;
    TUInt64 InTmMSecs;
    TFltV OutValV;
    TUInt64V OutTmMSecsV;
    // the buffer and the time stamps
    TFltUInt64PrV AllValV; 
	
protected:
	void OnAddRec(const TRec& Rec);

    TWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return InitP; }
    
	// most recent values
	double GetInFlt() const { return InVal; }
	uint64 GetInTmMSecs() const { return InTmMSecs; }
    // old values that fall out of the buffer
	void GetOutFltV(TFltV& ValV) const { ValV = OutValV; }
	void GetOutTmMSecsV(TUInt64V& MSecsV) const { MSecsV = OutTmMSecsV; }  
	
	// the oldest value that is still in the buffer
	double GetOldestFlt() const { return AllValV[0].Val1; }
	// the oldest timestamp that is still in the buffer
	uint64 GetOldestTmMSecs() const { return AllValV[0].Val2; }

    // buffer length
	int GetN() const { return AllValV.Len(); }

	// IFltVec
	int GetFltLen() const {	return AllValV.Len(); }
	double GetFlt(const TInt& ElN) const { return AllValV[ElN].Val1; }
	void GetFltV(TFltV& ValV) const;
	// ITmVec 
	int GetTmLen() const { return AllValV.Len(); }
	uint64 GetTm(const TInt& ElN) const { return AllValV[ElN].Val2; }
	void GetTmV(TUInt64V& MSecsV) const;

	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "timeSeriesWinBuf"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////////////
// Moving Window Buffer Template
template <class TSignalType>
class TWinBuffer : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;
	TSignalType Signal;

protected:
	void OnAddRec(const TRec& Rec) {
		TFltV ValV; InAggrVal->GetOutFltV(ValV);
		TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);
		if (InAggr->IsInit()) {
			Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
				ValV, TmMSecsV);
		};
	}

	TWinBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
		// parse out input aggregate
		TStr InStoreNm = ParamVal->GetObjStr("store");
		TStr InAggrNm = ParamVal->GetObjStr("inAggr");
		PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
		InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
		QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
		InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
		QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
	}

public:

	// json constructor 
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
		return new TWinBuffer<TSignalType>(Base, ParamVal);
	}

	// Load stream aggregate state from stream
	void LoadState(TSIn& SIn) { Signal.Load(SIn); }
	// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const { Signal.Save(SOut); }

	// did we finished initialization
	bool IsInit() const { return Signal.IsInit(); }
	// current values
	double GetFlt() const { return Signal.GetValue(); }
	uint64 GetTmMSecs() const { return Signal.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }

	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const {
		PJsonVal Val = TJsonVal::NewObj();
		Val->AddToObj("Val", Signal.GetValue());
		Val->AddToObj("Time", TTm::GetTmFromMSecs(Signal.GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
		return Val;
	}

	// stream aggregator type name 
	static TStr GetType();
	TStr Type() const { return GetType(); }
};

typedef TWinBuffer<TSignalProc::TSum> TWinBufSum;
typedef TWinBuffer<TSignalProc::TMin> TWinBufMin;
typedef TWinBuffer<TSignalProc::TMax> TWinBufMax;
typedef TWinBuffer<TSignalProc::TMa> TMa;
typedef TWinBuffer<TSignalProc::TVar> TVar;

///////////////////////////////
// Exponential Moving Average.
class TEma : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTm> InAggrVal;
	// indicator
	TSignalProc::TEma Ema;

protected:
	void OnAddRec(const TRec& Rec);
    
	TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return Ema.IsInit(); }
	// current values
	double GetFlt() const { return Ema.GetValue(); }
	uint64 GetTmMSecs() const { return Ema.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "ema"; }  
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Covariance.
class TCov : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggrX, InAggrY;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrValX, InAggrValY;	    
	// indicator
	TSignalProc::TCov Cov;

protected:
	void OnAddRec(const TRec& Rec);

	TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:    
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	// did we finish initialization
	bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
	// current values
	double GetFlt() const { return Cov.GetCov(); }
	uint64 GetTmMSecs() const { return Cov.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggrX->GetAggrNm()); 
        InAggrNmV.Add(InAggrY->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "covariance"; } 
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Correlation.
class TCorr : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:    
	// input
	TWPt<TStreamAggr> InAggrCov, InAggrVarX, InAggrVarY;
	TWPt<TStreamAggrOut::IFltTm> InAggrValCov, InAggrValVarX, InAggrValVarY;	 
    //the value
    TFlt Corr;
    //the time stamp 
    TUInt64 TmMSecs;

    void InitInAggr(const TWPt<TStreamAggrBase> SABase, const TStr& InAggrNmCov,
        const TStr& InAggrNmVarX, const TStr& InAggrNmVarY);
    
protected:
	void OnAddRec(const TRec& Rec);    
    
	TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	// did we finish initialization
	bool IsInit() const { return  InAggrVarX->IsInit() && InAggrVarY->IsInit() && InAggrCov->IsInit(); }
	// current values
	double GetFlt() const { return Corr; }
	uint64 GetTmMSecs() const { return 0; }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggrCov->GetAggrNm()); 
        InAggrNmV.Add(InAggrVarX->GetAggrNm()); InAggrNmV.Add(InAggrVarY->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "correlation"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Merger field map and interpolator
class TMergerFieldMap {
public:
    /// ID of field in end store of InFldJoinSeq
	TInt InFldId;
	TJoinSeq InFldJoinSeq;
    /// ID of timefield of start store of InFldJoinSeq
	TInt TmFldId;
    /// ID of field in out store (OutStore)
	TInt OutFldId;

public:
	TMergerFieldMap() { }

	TMergerFieldMap(const TInt& _InFldId, const TJoinSeq& _InFldJoinSeq, const TInt& _TmFldId, const TInt& _OutFldId) :
		InFldId(_InFldId), InFldJoinSeq(_InFldJoinSeq), TmFldId(_TmFldId), OutFldId(_OutFldId) {} 

	TMergerFieldMap(TSIn& SIn) { Load(SIn); }
	void Save(TSOut& SOut) const {
		InFldId.Save(SOut);
		InFldJoinSeq.Save(SOut);
		TmFldId.Save(SOut);
		OutFldId.Save(SOut);
	}
	void Load(TSIn& SIn) {
		InFldId.Load(SIn);
		InFldJoinSeq.Load(SIn);
		TmFldId.Load(SIn);
		OutFldId.Load(SIn);
	}
};

//////////////////////////////////////////////
// Merger
class TMerger : public TQm::TStreamAggr {
private:
	TWPt<TStore> OutStore;

	TInt TimeFieldId;
    /// names of the output fields
	TStrV OutFldNmV;
	
	TVec<TMergerFieldMap> FieldMapV;
    /// interpolators
	TVec<TSignalProc::PInterpolator> InterpV;

    /// a hash table mapping a storeId to a list of input field maps
	THash<TUInt, TIntSet> StoreIdFldIdVH;

    /// number of input signals
	TInt NInFlds;
    /// buffer of next time points
	TSignalProc::TLinkedBuffer<TUInt64> Buff;

	TBoolV SignalsPresentV;
	TBool SignalsPresent;

    /// time of the next interpolation point
	TUInt64 NextInterpTm;
    /// this variable is used to avoid duplicates when extrapolating into the future
	TUInt64 PrevInterpTm;

	// this variable tells wether all the merger will wait for all
	// points with the same timestamp before merging
	TBool OnlyPast;
	// this variable is used together with ExactInterp and used
	// used as an output to the store
	TTriple<TUInt64, TFltV, TUInt64> PrevInterpPt;

public:
	TMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);

	void CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm);

	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	PJsonVal SaveJson(const int& Limit) const;

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	static TStr GetType() { return "stmerger"; }
	TStr Type() const { return GetType(); }

private:
	void InitFld(const TWPt<TQm::TBase> Base, const TMergerFieldMap& FieldMap,
			const TStr& InterpNm);
	// initialized internal structures
	void InitMerger(const TWPt<TQm::TBase> Base, const TStr& OutStoreNm, const TStr& OutTmFieldNm,
			const bool& CreateStoreP, const bool& ExactInterp, const TStrV& InterpV);

protected:
	void OnAddRec(const TQm::TRec& Rec);

private:
	void OnAddRec(const TQm::TRec& Rec,  const TInt& FieldMapIdx);
	// adds a new record to the specified buffer
	void AddToBuff(const int& BuffIdx, const uint64 RecTm, const TFlt& Val);
	// shifts all the buffers so that the second value is greater then the current interpolation time
	void ShiftBuff();
	// checks if all signals are present
	bool AllSignalsPresent();
	// adds the record to the output store
	void AddToStore(const TFltV& InterpValV, const uint64 InterpTm, const uint64& RecId);
	// checks if the record can be added to the output store and adds it
	void AddRec(const TFltV& InterpValV, const uint64 InterpTm, const TQm::TRec& Rec);
	// checks if the conditions for interpolation are true in this iteration
	bool CanInterpolate();
	// updates the next interpolation time
	void UpdateNextInterpTm();
	// updates the next interpolation time in the interpolators
	void UpdateInterpolators();
	// checks if the merger is initialized
	bool CheckInitialized(const int& InterpIdx, const uint64& RecTm);
	// checks edge cases and makes sure interpolation will run smoothly
	void HandleEdgeCases(const uint64& RecTm);
};

///////////////////////////////
// Resampler
class TResampler : public TStreamAggr {
private:
    // input store
    TWPt<TStore> InStore;
    // input field IDs
	TIntV InFieldIdV;
    // field interpolators
	TVec<TSignalProc::PInterpolator> InterpolatorV;
    // output store
    TWPt<TStore> OutStore;
    // input time field
    TInt TimeFieldId;

    // interval size
    TUInt64 IntervalMSecs;
    // Timestamp of last inserted record
	TUInt64 InterpPointMSecs;

	// berfore first update
	TBool UpdatedP;
	
protected:	
	void OnAddRec(const TRec& Rec);
    
private:
	// refreshes the interpolators to the specified time
	void RefreshInterpolators(const uint64& Tm);
	bool CanInterpolate();
	void CreateStore(const TStr& NewStoreNm);    
    
    // InterpolatorV contains pair (input field, interpolator)
	TResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

    PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
    static TStr GetType() { return "resampler"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Dense Feature Extractor Stream Aggregate (extracts TFltV from records)
class TFtrExtAggr : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
	TWPt<TFtrSpace> FtrSpace;
	TFltV Vec;

protected:
	void OnAddRec(const TRec& Rec);
	TFtrExtAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace);

public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace);

	// did we finish initialization
	bool IsInit() const { return true; }
	// retrieving vector of values from the aggregate
	int GetFltLen() const { return FtrSpace->GetDim(); }
	void GetFltV(TFltV& ValV) const { ValV = Vec; }
	double GetFlt(const TInt& ElN) const;

	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;
	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);

	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "ftrext"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Histogram stream aggregate.
/// Updates a histogram model, connects to a time series stream aggregate (such as TEma)
/// that implements TStreamAggrOut::IFltTm or a buffered aggregate that implements
/// TStreamAggrOut::IFltTmIO
class TOnlineHistogram : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
	TSignalProc::TOnlineHistogram Model;

	// Input aggregate: only one aggregate is expected on input, these just
	// provide access to different interfaces for convenience 
	TStreamAggr* InAggr;
	TStreamAggrOut::IFltTm* InAggrVal; // can be NULL if the input is a buffered aggregate (IFltTmIO)
	TStreamAggrOut::IFltTmIO* InAggrValBuffer; // can be NULL if the input is a time series aggregate (IFltTm)
	
	TBool BufferedP; ///< is InAggrValBuffer not NULL?

protected:
	/// Triggered when a record is added
	void OnAddRec(const TRec& Rec);
	/// JSON constructor
	TOnlineHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	/// JSON constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TOnlineHistogram(Base, ParamVal); }

	/// did we finish initialization
	bool IsInit() const { return Model.IsInit(); }

	/// serilization to JSon
	PJsonVal SaveJson(const int& Limit) const { return Model.SaveJson(); }

	/// stream aggregator type name 
	static TStr GetType() { return "onlineHistogram"; }
	/// stream aggregator type name 
	TStr Type() const { return GetType(); }

	/// returns the number of bins 
	int GetFltLen() const { return Model.GetBins(); }
	/// returns frequencies in a given bin
	double GetFlt(const TInt& ElN) const { return Model.GetCountN(ElN); }
	/// returns the vector of frequencies
	void GetFltV(TFltV& ValV) const { Model.GetCountV(ValV); }
};

///////////////////////////////
/// Chi square stream aggregate.
/// Updates a chi square model, connects to an online histogram stream aggregate
/// that implements TStreamAggrOut::IFltVec
class TChiSquare : public TStreamAggr, public TStreamAggrOut::IFlt {
private:
	// input
	TWPt<TStreamAggr> InAggrX, InAggrY;
	TWPt<TStreamAggrOut::IFltVec> InAggrValX, InAggrValY;	    
	// indicator
	TSignalProc::TChiSquare ChiSquare;

protected:
	void OnAddRec(const TRec& Rec);
	TChiSquare(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:    
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// did we finish initialization
	bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
	// get current P value
	double GetFlt() const { return ChiSquare.GetP(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggrX->GetAggrNm()); 
        InAggrNmV.Add(InAggrY->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
	// stream aggregator type name 
	static TStr GetType() { return "chiSquare"; } 
	TStr Type() const { return GetType(); }
};

/////////////////////////////
// Moving Window Buffer Sum
template <>
inline TStr TWinBuffer<TSignalProc::TSum>::GetType() { return "winBufSum"; }

/////////////////////////////
// Moving Window Buffer Min
template <>
inline TStr TWinBuffer<TSignalProc::TMin>::GetType() { return "winBufMin"; }

/////////////////////////////
// Moving Window Buffer Max
template <>
inline TStr TWinBuffer<TSignalProc::TMax>::GetType() { return "winBufMax"; }

/////////////////////////////
// Moving Average
template <>
inline void TWinBuffer<TSignalProc::TMa>::OnAddRec(const TRec& Rec) {
	TFltV ValV; InAggrVal->GetOutFltV(ValV);
	TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);
	if (InAggr->IsInit()) {
		Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
			ValV, TmMSecsV, InAggrVal->GetN());
	}
}

template <>
inline TStr TWinBuffer<TSignalProc::TMa>::GetType() { return "ma"; }

/////////////////////////////
// Moving Variance
template <>
inline void TWinBuffer<TSignalProc::TVar>::OnAddRec(const TRec& Rec) {
	TFltV ValV; InAggrVal->GetOutFltV(ValV);
	TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);
	if (InAggr->IsInit()) {
		Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
            ValV, TmMSecsV, InAggrVal->GetN());
	}
}

template <>
inline TStr TWinBuffer<TSignalProc::TVar>::GetType() { return "variance"; }



// forward declarations
class TSlottedHistogram;
typedef TPt<TSlottedHistogram> PSlottedHistogram;

///////////////////////////////
/// Helper object to maintain distribution statistics in time-slots.
class TSlottedHistogram {
private:
	// smart-pointer
	TCRef CRef;
	friend class TPt<TSlottedHistogram>;
protected:
	/// Period length in miliseconds
	TUInt64 PeriodLen;
	/// Slot granularity in miliseconds
	TUInt64 SlotGran;
	/// Number of bins
	TInt Bins;
	/// Data storage, index is truncated timestamp, data is histogram
	TVec<TSignalProc::TOnlineHistogram> Dat;
protected:
	/// Constructor, reserves appropriate internal storage
	TSlottedHistogram(const uint64 _Period, const uint64 _Slot, const int _Bins);
	/// Given timestamp calculate index
	int GetIdx(const uint64 Ts) { return (int)((Ts % PeriodLen) / SlotGran); };
public:
	/// Create new instance based on provided JSon parameters
	static PSlottedHistogram New(const uint64 Period, const uint64 Slot, const int Bins) { return new TSlottedHistogram(Period, Slot, Bins); }
	/// Virtual destructor!
	virtual ~TSlottedHistogram() { }

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	/// Add new data to statistics
	void Add(const uint64& Ts, const TInt& Val);
	/// Remove data from statistics
	void Remove(const uint64& Ts, const TInt& Val);

	/// Provide statistics
	void GetStats(const uint64 TsMin, const uint64 TsMax, TFltV& Dest);
	/// Gets number of bins
	int GetBins() { return Dat.Len(); }
};

///////////////////////////////
/// Histogram stream aggregate.
/// Updates a histogram model, connects to a time series stream aggregate (such as TEma)
/// that implements TStreamAggrOut::IFltTm or a buffered aggregate that implements
/// TStreamAggrOut::IFltTmIO
class TOnlineSlottedHistogram : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
	PSlottedHistogram Model;

	// Input aggregate: only one aggregate is expected on input, these just
	// provide access to different interfaces for convenience 
	TStreamAggr* InAggr;
	TStreamAggrOut::IFltTm* InAggrVal; // can be NULL if the input is a buffered aggregate (IFltTmIO)
	TStreamAggrOut::IFltTmIO* InAggrValBuffer; // can be NULL if the input is a time series aggregate (IFltTm)

	TBool BufferedP; ///< is InAggrValBuffer not NULL?
	/// Last entered timestamp in msec
	TUInt64 LastTm;
	/// Window length in msec
	TUInt64 WndLen;

protected:
	/// Triggered when a record is added
	void OnAddRec(const TRec& Rec);
	/// JSON constructor
	TOnlineSlottedHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	/// JSON constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TOnlineSlottedHistogram(Base, ParamVal); }

	/// did we finish initialization
	bool IsInit() const { return true; }
	void LoadState(TSIn& SIn);
	void SaveState(TSOut& SOut) const;

	/// serilization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	/// stream aggregator type name 
	static TStr GetType() { return "onlineSlottedHistogram"; }
	/// stream aggregator type name 
	TStr Type() const { return GetType(); }

	/// returns the number of bins 
	int GetFltLen() const { return Model->GetBins(); }
	/// returns frequencies in a given bin
	double GetFlt(const TInt& ElN) const;
	/// returns the vector of frequencies
	void GetFltV(TFltV& ValV) const { Model->GetStats(LastTm - WndLen, LastTm, ValV); }
};


///////////////////////////////
/// Vector-difference stream aggregate.
/// Provides difference of two vector, usually distributions, 
/// connects to an online stream aggregates
/// that implement TStreamAggrOut::IFltVec
class TVecDiff : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
	// input
	TWPt<TStreamAggr> InAggrX, InAggrY;
	TWPt<TStreamAggrOut::IFltVec> InAggrValX, InAggrValY;

protected:
	void OnAddRec(const TRec& Rec) {} // do nothing
	TVecDiff(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// did we finish initialization
	bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
	void LoadState(TSIn& SIn) { /* do nothing, there is not state */ }
	void SaveState(TSOut& SOut) const { /* do nothing, there is not state */ }
	
	/// returns the number of bins 
	int GetFltLen() const { return InAggrValX->GetFltLen(); }
	/// returns frequencies in a given bin
	double GetFlt(const TInt& ElN) const { return InAggrValX->GetFlt(ElN) - InAggrValY->GetFlt(ElN); }
	/// returns the vector of frequencies
	void GetFltV(TFltV& ValV) const;
	/// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
	/// stream aggregator type name 
	static TStr GetType() { return "onlineVecDiff"; }
	/// stream aggregator type name 
	TStr Type() const { return GetType(); }
};
} // TStreamAggrs namespace
} // TQm namespace


#endif

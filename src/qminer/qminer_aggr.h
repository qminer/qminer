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
	TSignalProc::TBuffer<TUInt64> Buffer;
	TWPt<TStore> Store;
	
protected:
	void OnAddRec(const TRec& Rec);

    TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return Buffer.IsInit(); }
	/// Resets the aggregate
	void Reset() { Buffer.Reset(); }
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
    TFieldReader ValReader;
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
	/// Resets the model state
	void Reset() { InitP = false; TickVal = 0.0; TmMSecs = 0; }

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
// Supports two parameters: window size (in milliseconds) and delay (in milliseconds)
// When a new record with timestamp t_new is added, the buffer will be updated
// and the timestamps of all contained records will satisfy: 
// t_i - t_j <= window, for all i > j, and t_new - t_i >= delay, for all i.  
// 
// ASSUMPTION!
// Internally the aggregate only tracks the IDs A <= B, C <= D, B <= D from which we can reconstruct:
//  A ... B-1 forget interval
//  B ... D-1 buffer interval
//  C ... D-1 update interval 
//  It may happen that B > C, which corresponds to records that skipped the buffer
//  In that case, Skip := B - C is the number of skipped records, and
//  A ... B-1-Skip is the forget interval
//  C+Skip ... D-1 is the update interval
// The IDs need not be in the store and any interval can be empty.
// We assume the streaming store: no holes in record IDs, increasing timestamps, consistent
// with intervals.
//
class TWinBuf : public TStreamAggr, public TStreamAggrOut::IFltTmIO,
	public TStreamAggrOut::IFltVec, public TStreamAggrOut::ITmVec, public TStreamAggrOut::ITm {
private:
	// STORAGE ACCESS
	TWPt<TStore> Store; ///< needed to access records through IDs
    TInt TimeFieldId; ///< field ID of the timestamp field in the store (used for efficiency)
    TInt ValFieldId; ///< field ID of the value field in the store (used for efficiency)
    TFieldReader ValReader;

	// ALGORITHM PARAMETERS
	TUInt64 WinSizeMSecs; ///< window size in milliseconds
	TUInt64 DelayMSecs;  ///< delay in milliseconds
    
	//
	// ALGORITHM STATE
	//
	TBool InitP; ///< Has the aggregate been updated at least once?
	TUInt64 A; ///< The ID of the first record to forget. The last record to forget is strictly smaller than B. If A == B, the forget interval is empty.
	TUInt64 B; ///< The ID of the first record in the buffer, or in case of empty buffer the ID of the first record after the buffer. Guaranteed to be in the store (due to NewRec). If B == D, the buffer is empty.
	TUInt64 C; ///< The ID of the first record to update. If C == D, the update interval is empty.
	TUInt64 D; ///< The ID of the first record after the buffer. Guaranteed to be in the store (due to NewRec). If D == B, the buffer is empty.
	/// last timestamp
	TUInt64 Timestamp;
protected:
	/// Stream aggregate update function called when a record is added
	void OnAddRec(const TRec& Rec);
	/// JSON based constructor
    TWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	/// Smart pointer JSON based constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	
	/// Load stream aggregate state from stream
	void LoadState(TSIn& SIn);
	/// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const;

	/// did we finish initialization
	bool IsInit() const { return InitP; }
	/// Resets the model state
	void Reset();
	/// most recent values. Only makes sense if delay = 0!
	double GetInFlt() const { EAssertR(IsInit(), "WinBuf not initialized yet!"); EAssertR(B < D, "WinBuf is empty - GetInFlt() isn't valid! You need to reimplement OnAdd of the aggregate that is connected to WinBuf to use GetInFltV instead of GetInFlt!"); return Value(D - 1); }
	/// most recent timestamps. Only makes sense if delay = 0!
	uint64 GetInTmMSecs() const { EAssertR(IsInit(), "WinBuf not initialized yet!"); EAssertR(B < D, "WinBuf is empty - GetInTmMSecs() isn't valid! You need to reimplement OnAdd of the aggregate that is connected to WinBuf to use GetInTmMSecsV instead of GetInTmMSecs!"); return Time(D - 1); }
    /// Is the window delayed ?
	bool DelayedP() const { return DelayMSecs > 0; }

	/// new values that just entered the buffer (needed if delay is nonzero)
	void GetInFltV(TFltV& ValV) const;
	/// new timestamps that just entered the buffer (needed if delay is nonzero)
	void GetInTmMSecsV(TUInt64V& MSecsV) const;
	
	/// old values that fall out of the buffer
	void GetOutFltV(TFltV& ValV) const;
	/// old timestamps that fall out of the buffer
	void GetOutTmMSecsV(TUInt64V& MSecsV) const;

	uint64 GetTmMSecs() const { return Timestamp; }
		
    /// buffer length
	int GetN() const { EAssertR(IsInit(), "WinBuf not initialized yet!"); return (int)(D - B); }

	/// get float vector length (IFltVec interface)
	int GetFltLen() const { return GetN(); }
	/// get float vector element (IFltVec interface)
	double GetFlt(const TInt& ElN) const { return Value(B + ElN); }
	/// get float vector of all values in the buffer (IFltVec interface)
	void GetFltV(TFltV& ValV) const;
	/// get time vector length (ITmVec interface) 
	int GetTmLen() const { return GetN(); }
	/// get time vector element (ITmVec interface
	uint64 GetTm(const TInt& ElN) const { return Time(B + ElN); }
	/// get timestamp vector of all timestamps in the buffer (ITmVec interface)
	void GetTmV(TUInt64V& MSecsV) const;
	
	/// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
	/// stream aggregator type name 
    static TStr GetType() { return "timeSeriesWinBuf"; }
	/// stream aggregator type name 
	TStr Type() const { return GetType(); }
	
	/// Test if current state is consistent with store and parameters
	bool TestValid() const;
	/// print state for debugging
	void Print(const bool& PrintState = false);
private:
	// helper functions
	uint64 Time(const uint64& RecId) const { return Store->GetFieldTmMSecs(RecId, TimeFieldId); }
	double Value(const uint64& RecId) const { return ValReader.GetFlt(TRec(Store, RecId)); }
	
	bool InStore(const uint64& RecId) const { return Store->IsRecId(RecId); }
	bool BeforeStore(const uint64& RecId) const { return RecId < Store->GetFirstRecId(); }
	bool AfterStore(const uint64& RecId) const { return RecId > Store->GetLastRecId(); }
	
	bool InBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
		return !BeforeBuffer(RecId, LastRecTmMSecs) && !AfterBuffer(RecId, LastRecTmMSecs);
	}
	bool BeforeBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
		return  BeforeStore(RecId) || (InStore(RecId) && (Time(RecId) < LastRecTmMSecs - DelayMSecs - WinSizeMSecs));
	}
	bool AfterBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
		return AfterStore(RecId) || (InStore(RecId) && (Time(RecId) > LastRecTmMSecs - DelayMSecs));
	}
	void PrintInterval(const uint64& StartId, const uint64& EndId, const TStr& Label = "", const TStr& ModCode = "39") const;
};

///////////////////////////////////////
// Windowed Stream aggregates (readers from TWinBuf) 
template <class TSignalType>
class TWinAggr : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;
	TSignalType Signal;

protected:
	void OnAddRec(const TRec& Rec) {		
		TFltV OutValV; InAggrVal->GetOutFltV(OutValV);
		TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
		if (InAggr->IsInit()) {
			if (!InAggrVal->DelayedP()) {
				Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(), OutValV, OutTmMSecsV);
			} else {
				TFltV InValV; InAggrVal->GetInFltV(InValV);
				TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
				Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV);
			}
		};
	}

	TWinAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
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
		return new TWinAggr<TSignalType>(Base, ParamVal);
	}

	// Load stream aggregate state from stream
	void LoadState(TSIn& SIn) { Signal.Load(SIn); }
	// Save state of stream aggregate to stream
	void SaveState(TSOut& SOut) const { Signal.Save(SOut); }

	// did we finished initialization
	bool IsInit() const { return Signal.IsInit(); }
	/// Resets the aggregate
	void Reset() { Signal.Reset(); }
	// current values
	double GetFlt() const { return Signal.GetValue(); }
	uint64 GetTmMSecs() const { return TStreamAggrOut::ITm::GetTmMSecsCast(InAggr); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }

	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const {
		PJsonVal Val = TJsonVal::NewObj();
		Val->AddToObj("Val", Signal.GetValue());
		Val->AddToObj("Time", TTm::GetTmFromMSecs(GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
		return Val;
	}

	// stream aggregator type name 
	static TStr GetType();
	TStr Type() const { return GetType(); }
};

typedef TWinAggr<TSignalProc::TSum> TWinBufSum;
typedef TWinAggr<TSignalProc::TMin> TWinBufMin;
typedef TWinAggr<TSignalProc::TMax> TWinBufMax;
typedef TWinAggr<TSignalProc::TMa> TMa;
typedef TWinAggr<TSignalProc::TVar> TVar;

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
	/// Resets the aggregate
	void Reset() { Ema.Reset(); }
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
	/// Resets the aggregate
	void Reset() { Cov.Reset(); }
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
	/// Resets the aggregate
	void Reset() { TmMSecs = 0;  Corr = 0; }
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

	void Reset() { throw TQmExcept::New("TMerger::Reset() not implemented!"); }

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
	void Reset() { throw TQmExcept::New("TResampler::Reset() not implemented!"); }

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
	/// Reset
	void Reset() { }
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
	// STATE
	TSignalProc::TOnlineHistogram Model;

	// PARAMETERS (reconstructed from JSON)
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
	/// Resets the aggregate
	void Reset() { Model.Reset(); }
	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;
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
/// TDigest stream aggregate.
class TTDigest : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTm> InAggrVal;
	// indicator
	TSignalProc::TTDigest Model;
	TFltV QuantilesVals;
protected:
	void OnAddRec(const TRec& Rec);
	TTDigest(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TTDigest(const TFltV& Quantiles);
public:
	/// Add new data to statistics
	void Add(const TInt& Val);
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	static PStreamAggr New(const TFltV Quantiles);
	// did we finish initialization
	bool IsInit() const { return InAggr->IsInit(); }
	/// Reset
	void Reset() {  }
	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;
	/// get current Quantile value
	double GetFlt(const TInt& ElN) const { return Model.GetQuantile(QuantilesVals[ElN]); }
		/// returns the vector of frequencies
	void GetFltV(TFltV& ValV) const {
		for (int ElN=0; ElN<QuantilesVals.Len(); ElN++) {
			ValV.Add(Model.GetQuantile(QuantilesVals[ElN]));
		}
	}
	int GetFltLen() const { return QuantilesVals.Len(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
	// stream aggregator type name
	static TStr GetType() { return "tdigest"; }
	TStr Type() const { return GetType(); }
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
	/// Reset
	void Reset() { ChiSquare.Reset(); }
	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;

	// get current Chi2 value
	double GetFlt() const { return ChiSquare.GetChi2(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggrX->GetAggrNm()); 
        InAggrNmV.Add(InAggrY->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
	// stream aggregator type name 
	static TStr GetType() { return "chiSquare"; } 
	TStr Type() const { return GetType(); }
};

///////////////////////////////
/// TMinCountSketch stream aggregate.
class TCountMinSketch : public TStreamAggr, public TStreamAggrOut::IFlt {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTm> InAggrVal;
	// indicator
	TSignalProc::TCountMinSketch Model;
protected:
	void OnAddRec(const TRec& Rec);
	TCountMinSketch(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// did we finish initialization
	bool IsInit() const { return InAggr->IsInit(); }
	/// Reset
	void Reset() {  }
	/// Load from stream
	void LoadState(TSIn& SIn);
	/// Store state into stream
	void SaveState(TSOut& SOut) const;
	/// get current Quantile value
	double GetFlt() const {	return Model.Estimate(10); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
	// stream aggregator type name
	static TStr GetType() { return "countmin"; }
	TStr Type() const { return GetType(); }
};

/////////////////////////////
// Moving Window Buffer Sum
template <>
inline TStr TWinAggr<TSignalProc::TSum>::GetType() { return "winBufSum"; }

/////////////////////////////
// Moving Window Buffer Min
template <>
inline TStr TWinAggr<TSignalProc::TMin>::GetType() { return "winBufMin"; }

/////////////////////////////
// Moving Window Buffer Max
template <>
inline TStr TWinAggr<TSignalProc::TMax>::GetType() { return "winBufMax"; }

/////////////////////////////
// Moving Average
template <>
inline void TWinAggr<TSignalProc::TMa>::OnAddRec(const TRec& Rec) {
	TFltV OutValV; InAggrVal->GetOutFltV(OutValV);
	TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
	if (InAggr->IsInit()) {
		if (!InAggrVal->DelayedP()) {
			Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
				OutValV, OutTmMSecsV, InAggrVal->GetN());
		} else {
			TFltV InValV; InAggrVal->GetInFltV(InValV);
			TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
			Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV, InAggrVal->GetN());
		}
	}
}

template <>
inline TStr TWinAggr<TSignalProc::TMa>::GetType() { return "ma"; }

/////////////////////////////
// Moving Variance
template <>
inline void TWinAggr<TSignalProc::TVar>::OnAddRec(const TRec& Rec) {
	TFltV OutValV; InAggrVal->GetOutFltV(OutValV);
	TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
	if (InAggr->IsInit()) {
		if (!InAggrVal->DelayedP()) {
			Signal.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
				OutValV, OutTmMSecsV, InAggrVal->GetN());
		} else {
			TFltV InValV; InAggrVal->GetInFltV(InValV);
			TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
			Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV, InAggrVal->GetN());
		}
	}
}

template <>
inline TStr TWinAggr<TSignalProc::TVar>::GetType() { return "variance"; }



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

	/// Resets the counts to 0
	void Reset();

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
	void Reset() { Model->Reset(); }

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
	/// resets the aggregate
	void Reset() { }
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

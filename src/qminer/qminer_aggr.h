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
#include "qminer_ftr.h"
#include "qminer_pbs.h"

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

	TRecBuffer(const TWPt<TBase>& Base, const TStr& AggrNm, const int& Len);
    TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const int& Len);
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
/// Time Stream Aggregator.
/// Stream-aggregate with builtin time window. Only acts on AddRec trigger. 
/// It introduces two new callbacks, which provide timestamps: AddRec (called for 
/// new records) and DeleteRec (called when record falls out of time window).
class TTimeStreamAggr : public TStreamAggr {
private:
	/// Store id
	TWPt<TStore> TimeStore;
	/// Field providing time information
	TInt TimeFieldId;
	/// Current minimal time, for record to be considered within time window
	TUInt64 MinTimeMSecs;
	/// Time window size in milliseconds
	TUInt64 TimeWndMSecs;
	/// Queue of records currently in the time window
	TQQueue<TRec> RecIdQ;

protected:
	/// Add new record to the aggregate. Includes timestamp of the record
	virtual void OnAddRec(const TRec& Rec, const uint64& RecTimeMSecs) = 0;
	/// Remove record that got dropped out from the time window. Includes timestamp of the record
	virtual void OnDeleteRec(const TRec& Rec, const uint64& RecTimeMSecs) { };

	/// Create new time stream aggregate, by providing which field 
	/// to use for positioning within time window
	TTimeStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
		const int& _TimeFieldId, const uint64 _TimeWndMSecs);
    /// Create new stream aggregate from JSon parameters
    TTimeStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
	/// Load time stream aggregate from stream
	TTimeStreamAggr(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	/// For handling callbacks on new records added to the store
	void OnAddRec(const TRec& Rec);
    /// For handling callbacks on deleted records from the store
    void OnDeleteRec(const TRec& Rec) { }

	/// Time window start in milliseconds (latest seen record time - time window size)
	uint64 GetTimeWndStartMSecs() const { return MinTimeMSecs; }
	/// Time window end in milliseconds (latest seen record time)
	uint64 GetTimeWndEndMSecs() const { return TimeWndMSecs + MinTimeMSecs; }
	/// Time window size in milliseconds
	uint64 GetTimeWndMSecs() const { return TimeWndMSecs; }
    
    /// Get store
    const TWPt<TStore>& GetTimeStore() const { return TimeStore; }
};

///////////////////////////////
// QMiner-Count-Stream-Aggregator
class TCount : public TTimeStreamAggr, public TStreamAggrOut::IInt, public TStreamAggrOut::IFlt {
private:
	// current count
	TInt Count;

protected:
	TCount(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store,
        const int& TimeFieldId, const uint64 TimeWndMSecs);
    /// Create new stream aggregate from JSon parameters
    TCount(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
	TCount(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);

	// add new record to the aggregate
	void OnAddRec(const TRec& Rec, const uint64& RecTimeMSecs) { Count++; }
	// remove record that got droped out of 
	void OnDeleteRec(const TRec& Rec, const uint64& RecTimeMSecs) { Count--; }
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// get aggregate value
	int GetInt() const { return Count; }
	double GetFlt() const { return (double)Count; }

	// serialization to Json
	PJsonVal SaveJson(const int& Limit) const { return TJsonVal::NewObj("count", GetInt()); }
    
    // stream aggregator type name 
    static TStr GetType() { return "count"; }
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

	TTimeSeriesTick(const TWPt<TBase>& Base,  const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& TickValFieldNm);
    TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TTimeSeriesTick(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& TickValFieldNm);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);       
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

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
class TTimeSeriesWinBuf : public TStreamAggr, public TStreamAggrOut::IFltTmIO, public TStreamAggrOut::IFltVec, public TStreamAggrOut::ITmVec {
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

	TTimeSeriesWinBuf(const TWPt<TBase>& Base,  const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& ValFieldNm, const uint64& _WinSizeMSecs);
    TTimeSeriesWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TTimeSeriesWinBuf(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& ValFieldNm, const uint64& _WinSizeMSecs);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);       
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return InitP; }
    
	// most recent values
	double GetInFlt() const { return InVal; }
	uint64 GetInTmMSecs() const { return InTmMSecs; }
    // oldest values
	void GetOutFltV(TFltV& ValV) const { ValV = OutValV; }
	void GetOutTmMSecsV(TUInt64V& MSecsV) const { MSecsV = OutTmMSecsV; }  
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

///////////////////////////////
// Moving Window Buffer Count.
class TWinBufCount : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;

protected:
	void OnAddRec(const TRec& Rec) { /* nothing */ };

	TWinBufCount(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
		const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TWinBufCount(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TWinBufCount(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);

public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	//json constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn) { } // no internal state
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const { }; // no interna state

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return InAggrVal->GetN(); }
	uint64 GetTmMSecs() const { return InAggrVal->GetInTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "winBufCount"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Window Buffer Summa.
class TWinBufSum : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;
	TSignalProc::TSum Sum;

protected:
	void OnAddRec(const TRec& Rec);

	TWinBufSum(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
		const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TWinBufSum(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TWinBufSum(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	//json constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return Sum.GetSum(); }
	uint64 GetTmMSecs() const { return Sum.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "winBufSum"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Window Buffer Min.
class TWinBufMin : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;
	TSignalProc::TMin Min;

protected:
	void OnAddRec(const TRec& Rec);

	TWinBufMin(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
		const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TWinBufMin(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TWinBufMin(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	//json constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return Min.GetMin(); }
	uint64 GetTmMSecs() const { return Min.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "winBufMin"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Window Buffer Max.
class TWinBufMax : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;
	TSignalProc::TMax Max;

protected:
	void OnAddRec(const TRec& Rec);

	TWinBufMax(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
		const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TWinBufMax(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TWinBufMax(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
		const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	//json constructor
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return Max.GetMax(); }
	uint64 GetTmMSecs() const { return Max.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "winBufMax"; }
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Average.
class TMa : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;	
	// indicator
	TSignalProc::TMa Ma;

protected:
	void OnAddRec(const TRec& Rec);
    
	TMa(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
			const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TMa(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    
	TMa(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:    
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
    //json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return Ma.GetMa(); }
	uint64 GetTmMSecs() const { return Ma.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "ma"; }  
	TStr Type() const { return GetType(); }
};

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
    
	TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& Decay, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, 
        const TWPt<TStreamAggrBase> SABase);
	TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& TmInterval,
		const TSignalProc::TEmaType& Type, const uint64& InitMinMSecs, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    
	TEma(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:
    // TmInterval == how many milliseconds it tames for a value to drop below 1/e
	// Gets stream aggr base from store name
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InStoreNm, const TStr& InAggrNm);
	// Gets stream aggr base directly
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return Ema.IsInit(); }
	// current values
	double GetFlt() const { return Ema.GetEma(); }
	uint64 GetTmMSecs() const { return Ema.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "ema"; }  
	TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Variance.
class TVar : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTmIO> InAggrVal;	
	// indicator
	TSignalProc::TVar Var;

protected:
	void OnAddRec(const TRec& Rec);
    
	TVar(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);	
	TVar(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    
	TVar(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
public:    
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm);
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
    //json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   	
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// did we finish initialization
	bool IsInit() const { return true; }
	// current values
	double GetFlt() const { return Var.GetM2(); }
	uint64 GetTmMSecs() const { return Var.GetTmMSecs(); }
	void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "variance"; }
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
    
	TCov(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNmX, const TStr& InAggrNmY, const TWPt<TStreamAggrBase> SABase);	
	TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    

public:    
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
            const TStr& InStoreNm, const TStr& InAggrNmX, const TStr& InAggrNmY);
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNmX, const TStr& InAggrNmY, const TWPt<TStreamAggrBase> SABase);
    //json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   
    
	// did we finish initialization
	bool IsInit() const { return true; }
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
    
	TCorr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStreamAggrBase> SABase,
        const TStr& InAggrNmCov, const TStr& InAggrNmVarX, const TStr& InAggrNmVarY);
	TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    
	TCorr(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);

public:    
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TStr& InStoreNm, const TStr& InAggrNmCov, const TStr& InAggrNmVarX, 
        const TStr& InAggrNmVarY);
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStreamAggrBase> SABase, const TStr& InAggrNmCov, 
        const TStr& InAggrNmVarX, const TStr& InAggrNmVarY);
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);      

    /// Load stream aggregate from stream
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) { return new TCorr(Base, SABase, SIn); }
    /// Save stream aggregate to stream
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;
    
	// did we finish initialization
	bool IsInit() const { return true; }
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
private:
    /// Input store ID
    TUInt InStoreId;
    /// Input field ID
    TInt InFieldId;
    /// Output field Name (before output store created)
    TStr OutFieldNm;
    /// Field interpolator (supports only floats)
    TSignalProc::PInterpolator Interpolator;    
    
public:
    TMergerFieldMap() { }
    TMergerFieldMap(const TWPt<TBase>& Base, const TStr& InStoreNm, const TStr& InFieldNm, 
        const TStr& OutFieldNm, const TSignalProc::PInterpolator& Interpolator);
    TMergerFieldMap(const TWPt<TBase>& Base, const TStr& InStoreNm, const TStr& InFieldNm, 
        const TSignalProc::PInterpolator& Interpolator);
    //TMergerFieldMap(const TWPt<TBase>& Base, TSIn& SIn);   
    uint GetInStoreId() const { return InStoreId; }
    int GetInFieldId() const { return InFieldId; }
    TStr GetOutFieldNm() const { return OutFieldNm; }
   	const TSignalProc::PInterpolator& GetInterpolator() const { return Interpolator; }
};

class TStMergerFieldMap {
public:
	TInt InFldId;                                        // ID of field in end store of InFldJoinSeq
	TJoinSeq InFldJoinSeq;
	TInt TmFldId;                                        // ID of timefield of start store of InFldJoinSeq
	TInt OutFldId;                                       // ID of field in out store (OutStore)
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
	TStMergerFieldMap() { }
	TStMergerFieldMap(TSIn& SIn) {
		Load(SIn);
	}
	TStMergerFieldMap(const TInt& _InFldId, const TJoinSeq& _InFldJoinSeq, const TInt& _TmFldId, const TInt& _OutFldId) : 
		InFldId(_InFldId), InFldJoinSeq(_InFldJoinSeq), TmFldId(_TmFldId), OutFldId(_OutFldId) {} 
};

//////////////////////////////////////////////
// StMerger
class TStMerger : public TQm::TStreamAggr {
private:
	TWPt<TStore> OutStore;

	TInt TimeFieldId;	
	TStrV OutFldNmV;									// names of the output fields
	
	TVec<TStMergerFieldMap> FieldMapV;
	TVec<TSignalProc::PInterpolator> InterpV;	  		// interpolators

	THash<TUInt, TIntSet> StoreIdFldIdVH;				// a hash table mapping a storeId to a list of input field maps

	TInt NInFlds;										// number of input signals
	TSignalProc::TLinkedBuffer<TUInt64> Buff;			// buffer of next time points

	TBoolV SignalsPresentV;
	TBool SignalsPresent;

	TUInt64 NextInterpTm;								// time of the next interpolation point
	TUInt64 PrevInterpTm;								// this variable is used to avoid duplicates when extrapolating into the future

	// this variable tells wether all the merger will wait for all
	// points with the same timestamp before merging
	TBool OnlyPast;
	// this variable is used together with ExactInterp and used
	// used as an output to the store
	TTriple<TUInt64, TFltV, TUInt64> PrevInterpPt;

public:
	TStMerger(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
			const TStr& OutTmFieldNm, const bool& CreateStoreP, const bool& OnlyPast,
			const TVec<TStMergerFieldMap>& FieldMapV,
			const TStrV& InterpV);

	TStMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
	TStMerger(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);
	void CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm);

	static PStreamAggr New(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
			const TStr& OutTmFieldNm, const bool& CreateStoreP, const bool& OnlyPast,
			const TVec<TStMergerFieldMap>& FieldMap, const TStrV& InterpV);
	static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	PJsonVal SaveJson(const int& Limit) const;
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) { return new TStMerger(Base, SABase, SIn); }
	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	static TStr GetType() { return "stmerger"; }
	TStr Type() const { return GetType(); }

private:
	void InitFld(const TWPt<TQm::TBase> Base, const TStMergerFieldMap& FieldMap,
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
	TResampler(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& _IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP);
	TResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TResampler(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn);

public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& IntervalMSecs, const uint64& StartMSecs = 0, 
        const bool& CreateStoreP = false);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	static PStreamAggr Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) { return new TResampler(Base, SABase, SIn); }

	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;
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
	// serialization to JSon
	PJsonVal SaveJson(const int& Limit) const;

	/// Load stream aggregate state from stream
	void _Load(TSIn& SIn);
	/// Save stream aggregate to stream
	void Save(TSOut& SOut) const;
	/// Save state of stream aggregate to stream
	void _Save(TSOut& SOut) const;

	// stream aggregator type name 
	static TStr GetType() { return "ftrext"; }
	TStr Type() const { return GetType(); }
};

//////////////////////////////////////////////
// Composed stream aggregators
class TCompositional {
private:
public:
    // checks the type is compositional
    static bool IsCompositional(const TStr& TypeNm);
	// Calls the constructor given type
	static void Register(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    
	// Creates and connects IterN Ema aggregates and returns the vector of their names. Result[0] corresponds to the aggregate that is connected to the InAggrNm
	static TStrV ItEma(const TWPt<TQm::TBase>& Base, 
        const int& Order, const double& TmInterval, const TSignalProc::TEmaType& Type,
		const uint64& InitMinMSecs, const TStr& InAggrNm, const TStr& Prefix,
		TWPt<TQm::TStreamAggrBase>& SABase);
	static TStrV ItEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

};

} // TStreamAggrs namespace

} // namespace

#endif

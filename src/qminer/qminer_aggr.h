/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Quintelligence d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Contact: 
 *   Blaz Fortuna <blaz@blazfortuna.com>
 *
 */

#ifndef QMINER_AGGR_H
#define QMINER_AGGR_H

#include "qminer_core.h"
#include "qminer_ftr.h"
#include "qminer_gs.h"

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
	/// Time window size in miliseconds
	TUInt64 TimeWndMSecs;
	/// Queue of records currently in the time window
	TQQueue<TUInt64> RecIdQ;

protected:
	/// Add new record to the aggregate. Includes timestamp of the record
	virtual void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs) = 0;
	/// Remove record that got dropped out from the time window. Includes timestamp of the record
	virtual void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs) { };

	/// Create new time stream aggregate, by providing which field 
	/// to use for positioning within time window
	TTimeStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
		const int& _TimeFieldId, const uint64 _TimeWndMSecs);
    /// Create new stream aggregate from JSon parameters
    TTimeStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
	/// Load time stream aggregate from stream
	TTimeStreamAggr(const TWPt<TBase>& Base, TSIn& SIn);
public:
	/// Save time stream aggregate to streams
	void Save(TSOut& SOut) const;

	/// For handling callbacks on new records added to the store
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId);

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
/// Time and Numeric Stream Aggregator.
/// Extension of time windowed aggregate running on single numeric fields.
/// It introduces two new callbacks, which provide timestamps and numeric
/// value of the record: AddRec (called for new records) and DeleteRec 
/// (called when record falls out of time window).
class TTimeNumStreamAggr : public TTimeStreamAggr {
private:
	/// Numeric field providing input values
	TInt FieldId;
	/// Aggregate field type (only Flt and Int supported)
	TFieldType FieldType;

	/// Read the value of field FieldId for a given record from the store
	double GetVal(const TWPt<TStore>& Store, const uint64& RecId);

protected:
	/// Add new record to the aggregate. Includes timestamp and numeric value of the record.
	virtual void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const double& RecVal) = 0;
	/// Remove record that got dropped out from the time window.
	/// Includes timestamp and numeric value of the record.
	virtual void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const double& RecVal) { };

	/// For handling callbacks on new records added to the store, coming from TTimeSTreamAggr
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs);
	/// For handling callbacks on records dropping from the time window, coming from TTimeSTreamAggr
	void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs);

	/// Create new time stream aggregate, by providing which field to use for positioning 
	/// within time window and which field to use for retrieving numeric values
	TTimeNumStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
		const int& TimeFieldId, const uint64 TimeWndMSecs, const int& _FieldId);
    /// Create new stream aggregate from JSon parameters
    TTimeNumStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
	/// Load aggregate from stream
	TTimeNumStreamAggr(const TWPt<TBase>& Base, TSIn& SIn);
public:
	/// Save aggregate to stream
	virtual void Save(TSOut& SOut) const;
};

///////////////////////////////
/// Time and Item Stream Aggregator.
/// Extension of time windowed aggregate running on discrete fields
/// It introduces two new callbacks, which provide timestamps and discrete
/// value of the record: AddRec (called for new records) and DeleteRec 
/// (called when record falls out of time window). Discrete values are
/// represented as strings.
class TTimeItemStreamAggr : public TTimeStreamAggr {
private:
	/// Join sequence, for when we need to do joins to get to the discrete values
	TJoinSeq FieldJoinSeq;
	/// Discrete field providing input values
	TInt FieldId;
	/// Aggregate field type (only Str, StrV and Bool supported).
	TFieldType FieldType;

	/// Read the value of field FieldId for a given record from the store
	void GetRecVal(const TWPt<TStore>& Store, const uint64& RecId, TStrV& RecValV);
	/// Read the value of field FieldId for a given record from the store, by first
	/// executing 
	void GetVal(const TWPt<TStore>& Store, const uint64& RecId, TStrV& RecValV);
protected:
	//TODO: continue Doxygen comments from here till the end
	// add new record to the aggregate
	virtual void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStrV& RecValV) = 0;
	// remove record that got dropped out of 
	virtual void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId,
        const uint64& RecTimeMSecs, const TStrV& RecValV) { };

	// implementation of time-stream aggregate
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs);
	// remove record that got dropped out of 
	void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs);

	TTimeItemStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
		const int& TimeFieldId, const uint64 TimeWndMSecs, const TJoinSeq& _FieldJoinSeq, 
		const int& _FieldId);
    /// Create new strem aggregate from JSon parameters
    TTimeItemStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal);
	// serialization
	TTimeItemStreamAggr(const TWPt<TBase>& Base, TSIn& SIn);
public:
	virtual void Save(TSOut& SOut) const;
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
	TCount(const TWPt<TBase>& Base, TSIn& SIn);

	// add new record to the aggregate
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs) { Count++; }
	// remove record that got droped out of 
	void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, const uint64& RecTimeMSecs) { Count--; }
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;

	// get aggregate value
	int GetInt() const { return Count; }
	double GetFlt() const { return (double)Count; }

	// serialization to Json
	PJsonVal SaveJson(const int& Limit) const { return TJsonVal::NewObj("count", GetInt()); }
    
    // stream aggregator type name 
    static TStr GetType() { return "count"; }
};

///////////////////////////////
// Numeric-Aggregator-Utility
//   uses the following approach for single-pass standard deviation calculation:
//    - [source] The Art of Computer Programming, volume 2: Seminumerical Algorithms, 3rd edn., p. 232.
//    - [code] http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance
class TNumAggrUtil {
private:
    TInt Count;
    TFlt Sum;
    TFlt Min;
    TFlt Max;
    // for computing standard deviation on the fly
    TFlt StdMean;
    TFlt StdM2;

public:
    // default
    TNumAggrUtil() { }
    // serialization
    TNumAggrUtil(TSIn& SIn);
    void Save(TSOut& SOut) const;

    void AddVal(double Val);

    int GetCount() const { return Count; }
    double GetSum() const { return Sum; }
    double GetAvg() const { return (Count > 0) ? (Sum / double(Count)) : 0.0; };
    double GetMin() const { return Min; }
    double GetMax() const { return Max; }
    double GetStDev() const;
};

///////////////////////////////
// QMiner-Numeric-Stream-Aggregator
//   aggregates numeric values in batches, restarts after time window is full
class TNumeric : public TTimeNumStreamAggr, public TStreamAggrOut::INmFlt, private TTmWnd::TCallback {
private:
	// current aggregates
	TNumAggrUtil CurrAggr;
	// last full aggregate
	TNumAggrUtil LastAggr;
	// time window
	TTmWnd TimeWnd;
	// valid names
	TStrSet NmSet;

	void InitNmSet();
protected:
	TNumeric(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
        const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId);
    /// Create new stream aggregate from JSon parameters
    TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TNumeric(const TWPt<TBase>& Base, TSIn& SIn);

	// new record
	void OnAddRec(const TWPt<TStore>& Store,
		const uint64& RecId, const uint64& RecTimeMSecs, const double& RecVal);
	// tile window callback
	void NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
        const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;

	// get aggregate values
	bool IsNmFlt(const TStr& Nm) const;
	double GetNmFlt(const TStr& Nm) const;
	void GetNmFltV(TStrFltPrV& NmFltV) const;

	// seralization to Json
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "numeric"; }
};

///////////////////////////////
// QMiner-Grouped-Numeric-Stream-Aggregator
//   aggregates numeric values in batches, restarts after time window is full
// TODO: remove
class TNumericGroup : public TTimeNumStreamAggr, public TStreamAggrOut::INmFlt, private TTmWnd::TCallback {
private:
	TJoinSeq GroupJoinSeq;
	TWPt<TStore> GroupStore;
	TInt GroupFieldId;
	// current aggregates
	THash<TStr, TNumAggrUtil> CurrAggrH;
	// last full aggregate
	THash<TStr, TNumAggrUtil> LastAggrH;
	// time window
	TTmWnd TimeWnd;
	// valid names
	TStrSet NmSet;

	void InitNmSet();
protected:
	TNumericGroup(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
        const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId, 
        const TJoinSeq& _GroupJoinSeq, const TWPt<TStore>& _GroupStore, const int& _GroupFieldId);
    /// Create new stream aggregate from JSon parameters
    TNumericGroup(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TNumericGroup(const TWPt<TBase>& Base, TSIn& SIn);

	/// For handling callbacks on new records added to the store, coming from TTimeNumStreamAggr
    void OnAddRec( const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const double& RecVal);
    /// Actually edits grouped aggregate
    void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStr& GroupStr, const double& RecVal);
	// time window callback
	void NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64& TimeWndMSecs, 
        const int& FieldId, const TJoinSeq& _FieldJoinSeq, const TWPt<TStore>& GrpStore,
        const int& _GrpFieldId);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;
               
	// get aggregate values
	bool IsNmFlt(const TStr& Nm) const;
	double GetNmFlt(const TStr& Nm) const;
	void GetNmFltV(TStrFltPrV& NmFltV) const;

	// serialization to Json
	PJsonVal SaveJson(const int& Limit) const;

    // stream aggregator type name 
    static TStr GetType() { return "numeric-grouped"; }
};

///////////////////////////////
// QMiner-ItemCount-Stream-Aggregator
//   counts items and computes count statistics
class TItem : public TTimeItemStreamAggr, public TStreamAggrOut::INmInt {
private:
	// aggreagte values
	TStrH ItemFqH;

protected:
	// add new record to the aggregate
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStrV& RecValV);
	// remove record that got droped out of 
	void OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStrV& RecValV);

	TItem(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
		const int& TimeFieldId, const uint64& TimeWndMSecs, const TJoinSeq& FieldJoinSeq,
		const int& FieldId);
    /// Create new stream aggregate from JSon parameters
    TItem(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TItem(const TWPt<TBase>& Base, TSIn& SIn);
public:
	static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store,
		const int& TimeFieldId, const uint64& TimeWndMSecs, const TJoinSeq& FieldJoinSeq,
		const int& FieldId);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	// serialization
	static PStreamAggr Load(const TWPt<TBase>& Base, TSIn& SIn);
	void Save(TSOut& SOut) const;		

	// get aggregate values
	bool IsNm(const TStr& Nm) const;
	double GetNmInt(const TStr& Nm) const;
	void GetNmIntV(TStrIntPrV& NmIntV) const;

	// seralization to Json
	PJsonVal SaveJson(const int& Limit) const;

    // stream aggregator type name 
    static TStr GetType() { return "item"; }
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
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId);

	TTimeSeriesTick(const TWPt<TBase>& Base,  const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& TickValFieldNm);
    TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& AggrNm,
		const TStr& TimeFieldNm, const TStr& TickValFieldNm);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);       

	// did we finish initialization
	bool IsInit() const { return InitP; }
    
	// current values
	double GetFlt() const { return TickVal; }
	uint64 GetTmMSecs() const { return TmMSecs; }

	// serilization to JSon
	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "timeSeriesTick"; }        
};

///////////////////////////////
// Exponential Moving Average.
class TEma : public TStreamAggr, public TStreamAggrOut::IFltTm {
private:
	// input
	TWPt<TStreamAggr> InAggr;
	TWPt<TStreamAggrOut::IFltTm> InAggrVal;
	TInt InTypeId;
	// indicator
	TSignalProc::TEma Ema;

protected:
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId);
    
	TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& Decay, 
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, 
        const TWPt<TStreamAggrBase> SABase);
	TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& TmInterval,
		const TSignalProc::TEmaType& Type, const uint64& InitMinMSecs, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
	TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal);    

public:
    // TmInterval == how many milliseconds it tames for a value to drop below 1/e
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InStoreNm, const TStr& InAggrNm);
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   
    
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
};

///////////////////////////////
// Merger field map and interpolator
class TMergerFieldMap {
private:
    /// Input store ID
    TUCh InStoreId;
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
       
    uchar GetInStoreId() const { return InStoreId; }
    int GetInFieldId() const { return InFieldId; }
    TStr GetOutFieldNm() const { return OutFieldNm; }
    
    const TSignalProc::PInterpolator& GetInterpolator() const { return Interpolator; }
};

///////////////////////////////
// Merger
class TMerger : public TStreamAggr {
private:
    /// Map from store id to time field
    TIntV InTimeFieldIdV;
	/// Map of old store id and field id to new field id
    TVec<TMergerFieldMap> FieldMapV;
    /// List of time fields for each store
    /// New store name
    TWPt<TStore> OutStore;
    /// New store time field
    TInt TimeFieldId;
    
protected:	
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId);

private:
    void CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm);
public:
    TMerger(const TWPt<TBase>& Base, const TStr& AggrNm, const TStrPrV& InStoreTimeFieldNmV,
        const TVec<TMergerFieldMap> FieldMapV_, const TStr& OutStoreNm, 
        const TStr& NewTimeFieldNm, const bool& CreateStoreP = false);

	PJsonVal SaveJson(const int& Limit) const;
    
    // stream aggregator type name 
    static TStr GetType() { return "merger"; }
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
	
protected:	
	void OnAddRec(const TWPt<TStore>& Store, const uint64& RecId);
    
private:
	void CreateStore(const TStr& NewStoreNm);    
    
    // InterpolatorV contains pair (input field, interpolator)
	TResampler(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& _IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP);
	TResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal);        
public:
    static PStreamAggr New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& IntervalMSecs, const uint64& StartMSecs = 0, 
        const bool& CreateStoreP = false);
    // json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);   
    
	PJsonVal SaveJson(const int& Limit) const;

    // stream aggregator type name 
    static TStr GetType() { return "resampler"; }    
};

} // TStreamAggrs namespace

} // namespace

#endif

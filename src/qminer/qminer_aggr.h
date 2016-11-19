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

#define __STDC_FORMAT_MACROS
#include <inttypes.h>


namespace TQm {

namespace TAggrs {

///////////////////////////////
// QMiner-Aggregator-Count
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

///////////////////////////////
// QMiner-Aggregator-TimeSpan
class TTimeSpan : public TAggr {
private:
    //meta-data
    TStr FieldNm;
    TStr JoinPathStr;
    // aggregations
    TUInt64 SlotLen;
    TUInt64H CountsH;

    PJsonVal GetJsonList(const TUInt64H& DataH) const;

    TTimeSpan(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt, const uint64 _SlotLen);
public:
    static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt, const uint64 _SlotLen) {
        return new TTimeSpan(Base, AggrNm, RecSet, FtrExt, _SlotLen);
    }
    static PAggr New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PJsonVal& JsonVal);

    PJsonVal SaveJson() const;

    // aggregator type name
    static TStr GetType() { return "timespan"; }
};

} // TAggrs namespace

namespace TStreamAggrs {

///////////////////////////////
// Record Id Buffer.
// Delays record Ids for a given amount of new records.
class TRecBuffer : public TStreamAggr {
private:
    /// Circular buffer containing record ids
    TSignalProc::TBuffer<TUInt64> Buffer;
    /// Store from where we are collecting records
    TWPt<TStore> Store;

protected:
    /// Add new record to the buffer
    void OnAddRec(const TRec& Rec);

    /// Json constructor
    TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return Buffer.IsInit(); }
    /// Resets the aggregate
    void Reset() { Buffer.Reset(); }

    /// Serilization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "recordBuffer"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Time series tick.
/// Wrapper for exposing time series to signal processing aggregates
class TTimeSeriesTick : public TStreamAggr,
                        public TStreamAggrOut::ITm,
                        public TStreamAggrOut::IFlt {
private:
    /// ID of the field from which we collect time points
    TInt TimeFieldId;
    /// Reader for extracting numeric values from records
    TFieldReader ValReader;

    /// We are not initialized until we read the first value
    TBool InitP;
    /// Time of last extracted value
    TUInt64 TmMSecs;
    /// Last extracted value
    TFlt TickVal;

protected:
    /// On new record we update value and timestamp
    void OnAddRec(const TRec& Rec);
    /// On new timestamp we update timestamp
    void OnTime(const uint64& TmMsec);
    /// No on step supported
    void OnStep();

    /// Json constructor
    TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return InitP; }
    /// Resets the model state
    void Reset() { InitP = false; TickVal = 0.0; TmMSecs = 0; }

    /// Time of last extracted value
    uint64 GetTmMSecs() const { return TmMSecs; }
    /// Last extracted value
    double GetFlt() const { return TickVal; }

    // serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "timeSeriesTick"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Time series window buffer with memory.
template <class TVal>
class TWinBufMem : public TStreamAggr,
                   public TStreamAggrOut::ITm,
                   public TStreamAggrOut::ITmIO,
                   public TStreamAggrOut::IValIO<TVal>,
                   public TStreamAggrOut::ITmVec,
                   public TStreamAggrOut::IValVec<TVal> {
private:
    typedef enum { wbmuOnAddRec, wbmuAlways } TWinBufMemUpdate;

private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input timestamp interface
    TWPt<TStreamAggrOut::ITm> InAggrTm;

    /// When should we read values from incoming aggregate
    TWinBufMemUpdate UpdateType;
    /// window size in milliseconds
    TUInt64 WinSizeMSecs;
    /// delay in milliseconds
    TUInt64 DelayMSecs;

    /// Has the aggregate been updated at least once?
    TBool InitP;
    /// Current timestamp
    TUInt64 TmMSecs;
    /// Current window buffer
    TQQueue<TPair<TUInt64, TVal> > WindowQ;
    /// Current delay buffer
    TQQueue<TPair<TUInt64, TVal> > DelayQ;

    /// New values from last trigger
    TVec<TVal> InValV;
    /// Timestamps for new values
    TUInt64V InTmMSecsV;
    /// Forgoten values from last trigger
    TVec<TVal> OutValV;
    /// Timestamps for forgoten values
    TUInt64V OutTmMSecsV;

protected:
    /// Value getter, must be overridden
    virtual TVal GetVal() const = 0;

private:
    /// Read new value from the input aggregate and adds it to the delay
    void UpdateVal();
     /// Read new timestamp from the input aggregate and move aggregates accordingly
    void UpdateTime();

protected:
    /// Get input aggregate
    const TWPt<TStreamAggr>& GetInAggr() const { return InAggr; }

    /// Stream aggregate update function called when a record is added
    void OnAddRec(const TRec& Rec);
    /// Stream aggregate that forgets records when time is updated
    void OnTime(const uint64& TmMsec);
    /// Just a expection-throwing placeholder
    void OnStep();

    /// JSON based constructor
    TWinBufMem(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// did we finish initialization
    bool IsInit() const { return InitP; }
    /// Resets the model state
    void Reset();

    // ITm
    /// time stamp of the last record
    uint64 GetTmMSecs() const { return TmMSecs; }

    // IValIO
    /// new values that just entered the buffer (needed if delay is nonzero)
    void GetInValV(TVec<TVal>& ValV) const { ValV = InValV; }
    /// old values that fall out of the buffer
    void GetOutValV(TVec<TVal>& ValV) const { ValV = OutValV; }

    // ITmIO
    /// new timestamps that just entered the buffer (needed if delay is nonzero)
    void GetInTmMSecsV(TUInt64V& MSecsV) const { MSecsV = InTmMSecsV; }
    /// old timestamps that fall out of the buffer
    void GetOutTmMSecsV(TUInt64V& MSecsV) const { MSecsV = OutTmMSecsV; }

    // IValV
    /// get buffer length
    int GetVals() const { EAssertR(IsInit(), "WinBuf not initialized yet!"); return WindowQ.Len(); }
    /// get value at
    void GetVal(const int& ElN, TVal& Val) const { Val = WindowQ[ElN].Val2; }
    /// get float vector of all values in the buffer (IFltVec interface)
    void GetValV(TVec<TVal>& ValV) const;

    // ITmV
    /// get buffer length
    int GetTmLen() const { return GetVals(); }
    /// get timestamp at
    uint64 GetTm(const int& ElN) const { return WindowQ[ElN].Val1; }
    /// get timestamp vector of all timestamps in the buffer (ITmVec interface)
    void GetTmV(TUInt64V& MSecsV) const;

    /// serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;
};

///////////////////////////////
/// Numberic circular buffer.
/// Reads from TWinBuf and stores the buffer values in memory as a circular buffer,
/// which can be resized if needed.
class TWinBufFltV : public TWinBufMem<TFlt> {
private:
    /// Input time series aggregate
    TWPt<TStreamAggrOut::IFlt> InAggrVal;

protected:
    /// Value getter, we read float from input aggregate
    TFlt GetVal() const { return InAggrVal->GetFlt(); }
    /// Json constructor
    TWinBufFltV(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "timeSeriesWinBufVector"; }
    /// Stream aggregator type name
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
template <class TVal>
class TWinBuf : public TStreamAggr,
                public TStreamAggrOut::ITm,
                public TStreamAggrOut::ITmIO,
                public TStreamAggrOut::IValIO<TVal>,
                public TStreamAggrOut::ITmVec,
                public TStreamAggrOut::IValVec<TVal> {
protected:
    // STORAGE ACCESS
    /// value getter
    virtual TVal GetRecVal(const uint64& RecId) const = 0;
    ///
    virtual void RecUpdate(const uint64& RecId) const { }
    /// needed to access records through IDs
    TWPt<TStore> Store;
private:
    /// field ID of the timestamp field in the store (used for efficiency)
    TInt TimeFieldId;

    // ALGORITHM PARAMETERS
    /// window size in milliseconds
    TUInt64 WinSizeMSecs;
    /// delay in milliseconds
    TUInt64 DelayMSecs;

    // ALGORITHM STATE
    /// Has the aggregate been updated at least once?
    TBool InitP;
    /// The ID of the first record to forget. The last record to forget is strictly smaller than B.
    /// If A == B, the forget interval is empty.
    TUInt64 A;
    /// The ID of the first record in the buffer, or in case of empty buffer the ID of the first
    /// record after the buffer. Guaranteed to be in the store (due to NewRec). If B == D, the buffer is empty.
    TUInt64 B;
    /// The ID of the first record to update. If C == D, the update interval is empty.
    TUInt64 C;
    /// The ID of the first record after the buffer. Guaranteed to be in the store (due to NewRec).
    /// If D == B, the buffer is empty.
    TUInt64 D;
    /// last timestamp
    TUInt64 Timestamp;
protected:
    /// Stream aggregate update function called when a record is added
    void OnAddRec(const TRec& Rec);
    /// Stream aggregate that forgets records when time is updated
    void OnTime(const uint64& TmMsec);
    /// Just a expection-throwing placeholder
    void OnStep() { throw TExcept::New("Should not be executed."); }

    /// JSON based constructor
    TWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// did we finish initialization
    bool IsInit() const { return InitP; }
    /// Resets the model state
    void Reset();

    // INTERFACE

    // ITm
    /// time stamp of the last record
    uint64 GetTmMSecs() const { return Timestamp; }

    // ITmIO
    /// new timestamps that just entered the buffer (needed if delay is nonzero)
    void GetInTmMSecsV(TUInt64V& MSecsV) const;
    /// old timestamps that fall out of the buffer
    void GetOutTmMSecsV(TUInt64V& MSecsV) const;

    // IValIO
    /// new values that just entered the buffer (needed if delay is nonzero)
    void GetInValV(TVec<TVal>& ValV) const;
    /// old values that fall out of the buffer
    void GetOutValV(TVec<TVal>& ValV) const;

    // IValV
    /// get buffer length
    int GetVals() const { EAssertR(IsInit(), "WinBuf not initialized yet!"); return (int)(D - B); }
    /// get value at
    void GetVal(const int& ElN, TVal& Val) const { Val = GetRecVal(B + ElN); }
    /// get float vector of all values in the buffer (IFltVec interface)
    void GetValV(TVec<TVal>& ValV) const;

    // ITmV
    /// get buffer length
    int GetTmLen() const { return GetVals(); }
    /// get timestamp at
    uint64 GetTm(const int& ElN) const { return Time(B + ElN); }
    /// get timestamp vector of all timestamps in the buffer (ITmVec interface)
    void GetTmV(TUInt64V& MSecsV) const;

    /// serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Test if current state is consistent with store and parameters
    bool TestValid() const;
    /// print state for debugging
    void Print(const bool& PrintState = false);
private:
    /// Extract timestamp from the given record
    uint64 Time(const uint64& RecId) const { return Store->GetFieldTmMSecs(RecId, TimeFieldId); }
    /// Check if record id valid
    bool InStore(const uint64& RecId) const { return Store->IsRecId(RecId); }
    /// Check if record id before the store.first
    bool BeforeStore(const uint64& RecId) const { return RecId < Store->GetFirstRecId(); }
    /// Check if record id after the store.last
    bool AfterStore(const uint64& RecId) const { return RecId > Store->GetLastRecId(); }
    /// Check if record id s in the buffer (i.e. not before or after)
    bool InBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
        return !BeforeBuffer(RecId, LastRecTmMSecs) && !AfterBuffer(RecId, LastRecTmMSecs); }
    /// Check if record id older than current buffer window
    bool BeforeBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
        return  BeforeStore(RecId) || (InStore(RecId) && (Time(RecId) < LastRecTmMSecs - DelayMSecs - WinSizeMSecs)); }
    /// Check if record id newer then the current buffer window
    bool AfterBuffer(const uint64& RecId, const uint64& LastRecTmMSecs) const {
        return AfterStore(RecId) || (InStore(RecId) && (Time(RecId) > LastRecTmMSecs - DelayMSecs)); }
    /// Debug printout
    void PrintInterval(const uint64& StartId, const uint64& EndId, const TStr& Label = "", const TStr& ModCode = "39") const;
};

///////////////////////////////
/// Time series window buffer with dense vector per record.
class TWinBufFlt : public TWinBuf<TFlt> {
private:
    /// Numberic field reader
    TFieldReader ValReader;
protected:
    /// Implementation of record value getter
    TFlt GetRecVal(const uint64& RecId) const;

    /// Json constructor
    TWinBufFlt(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TWinBufFlt(Base, ParamVal); }

    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "timeSeriesWinBuf"; }
    /// Stream aggregator type name
    TStr Type(void) const { return GetType(); }
};

///////////////////////////////
/// Time series window buffer with sparse vector per record.
class TWinBufFtrSpVec : public TWinBuf<TIntFltKdV>,
                        public TStreamAggrOut::IFtrSpace {
private:
    /// Feature space used to extract vectors from records
    PFtrSpace FtrSpace;
protected:
    /// Implementation of record value getter
    TIntFltKdV GetRecVal(const uint64& RecId) const;
    /// Fitting feature space
    void RecUpdate(const uint64& RecId) const;

    /// Json constructor
    TWinBufFtrSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TWinBufFtrSpVec(Base, ParamVal); }

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Stream aggregator type name
    static TStr GetType() { return "timeSeriesWinBufFeatureSpace"; }
    /// Stream aggregator type name
    TStr Type(void) const { return GetType(); }

    /// Get feature space used by the aggregate
    PFtrSpace GetFtrSpace() const { return FtrSpace; }
};

///////////////////////////////
// Windowed stream aggregates on numeric time steries
template <class TSignalType>
class TWinAggr : public TStreamAggr,
                 public TStreamAggrOut::ITm,
                 public TStreamAggrOut::IFlt {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input latest timestamp
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input time series
    TWPt<TStreamAggrOut::ITmIO> InAggrTmIO;
    /// Input time series
    TWPt<TStreamAggrOut::IFltIO> InAggrFltIO;

    /// signal we are maintaining on the stream
    TSignalType Signal;

protected:
    /// Update signal based on the changes from the input
    void OnStep();
    /// Json constructor
    TWinAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TWinAggr<TSignalType>(Base, ParamVal); }

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn) { Signal.Load(SIn); }
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const { Signal.Save(SOut); }

    /// Did we finished initialization
    bool IsInit() const { return Signal.IsInit(); }
    /// Resets the aggregate
    void Reset() { Signal.Reset(); }
    /// Get current signal value
    double GetFlt() const { return Signal.GetValue(); }
    /// Get latest time stamp
    uint64 GetTmMSecs() const { return InAggrTm->GetTmMSecs(); }

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
    /// Serialization to json
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType();
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};


///////////////////////////////
// Window sum on numeric time series
typedef TWinAggr<TSignalProc::TSum> TWinBufSum;
template <> inline TStr TWinAggr<TSignalProc::TSum>::GetType() { return "winBufSum"; }

///////////////////////////////
// Window mininum on numeric time series
typedef TWinAggr<TSignalProc::TMin> TWinBufMin;
template <> inline TStr TWinAggr<TSignalProc::TMin>::GetType() { return "winBufMin"; }

///////////////////////////////
// Window maximum on numeric time series
typedef TWinAggr<TSignalProc::TMax> TWinBufMax;
template <> inline TStr TWinAggr<TSignalProc::TMax>::GetType() { return "winBufMax"; }

///////////////////////////////
// Window moving average on numeric time series
typedef TWinAggr<TSignalProc::TMa> TMa;
template <> inline TStr TWinAggr<TSignalProc::TMa>::GetType() { return "ma"; }

///////////////////////////////
// Window variance on numeric time series
typedef TWinAggr<TSignalProc::TVar> TVar;
template <> inline TStr TWinAggr<TSignalProc::TVar>::GetType() { return "variance"; }

///////////////////////////////
/// Windowed stream aggregates on sparse vector time series.
/// Results are sparse vectors.
template <class TSignalType>
class TWinAggrSpVec : public TStreamAggr,
                      public TStreamAggrOut::ITm,
                      public TStreamAggrOut::ISparseVec {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input latest timestamp
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input time series
    TWPt<TStreamAggrOut::ITmIO> InAggrTmIO;
    /// Input time series
    TWPt<TStreamAggrOut::IValIO<TIntFltKdV>> InAggrSparseVecIO;

    /// signal we are maintaining on the stream
    TSignalType Signal;

protected:
    /// Update strema aggregate
    void OnStep();
    /// Json constructor
    TWinAggrSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

public:
    /// json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TWinAggrSpVec<TSignalType>(Base, ParamVal); }

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn) { Signal.Load(SIn); }
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const { Signal.Save(SOut); }

    /// Did we finished initialization
    bool IsInit() const { return Signal.IsInit(); }
    /// Resets the aggregate
    void Reset() { Signal.Reset(); }
    /// Number of values in the output vector
    int GetSparseVecLen() const { return Signal.GetValue().Len(); }
    /// Get ElN-th output value
    TIntFltKd GetSparseVecVal(const int& ElN) const { return Signal.GetValue()[0]; }
    /// Get output vector
    void GetSparseVec(TVec<TIntFltKd>& ValV) const { ValV = Signal.GetValue(); }
    /// Get latest timestamp
    uint64 GetTmMSecs() const { return InAggrTm->GetTmMSecs(); }

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType();
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Window sum on sparse vectors time series
typedef TWinAggrSpVec<TSignalProc::TSumSpVec> TWinBufSpVecSum;
template <> inline TStr TWinAggrSpVec<TSignalProc::TSumSpVec>::GetType() { return "winBufSpVecSum"; }

///////////////////////////////
// Exponential Moving Average.
class TEma : public TStreamAggr,
             public TStreamAggrOut::ITm,
             public TStreamAggrOut::IFlt {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;

    /// EMA indicator
    TSignalProc::TEma Ema;

protected:
    /// Update EMA
    void OnStep();

    /// Json constructor
    TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization?
    bool IsInit() const { return Ema.IsInit(); }
    /// Resets the aggregate
    void Reset() { Ema.Reset(); }
    /// Latest value
    double GetFlt() const { return Ema.GetValue(); }
    /// Timestamp of the latest value
    uint64 GetTmMSecs() const { return Ema.GetTmMSecs(); }

    /// List of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "ema"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
// Exponential Moving Average.
class TEmaSpVec : public TStreamAggr,
                  public TStreamAggrOut::ITm,
                  public TStreamAggrOut::ISparseVec {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::ISparseVec> InAggrSparseVec;

    /// EMA indicator
    TSignalProc::TEmaSpVec Ema;

protected:
    /// Update EMA
    void OnStep();

    /// Json constructor
    TEmaSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return Ema.IsInit(); }
    /// Resets the aggregate
    void Reset() { Ema.Reset(); }
    /// Get number of values in the output vector
    int GetSparseVecLen() const { return Ema.GetValue().Len(); }
    /// Get ElN-th value from the output vector
    TIntFltKd GetSparseVecVal(const int& ElN) const { return Ema.GetValue()[ElN]; }
    /// Get output vector
    void GetSparseVec(TVec<TIntFltKd>& ValV) const { ValV = Ema.GetValue(); }
    /// Get last update timestamp
    uint64 GetTmMSecs() const { return Ema.GetTmMSecs(); }

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "emaSpVec"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
// Threshold aggregate - outputs 1 if the value is above a threshold, 0 otherwise.
class TThresholdAggr : public TStreamAggr,
                       public TStreamAggrOut::ITm,
                       public TStreamAggrOut::IFlt {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;

    /// Threshold value
    TFlt Threshold;
    /// Was input value at last step above threshold
    TFlt IsAboveP;
    /// Timestamp of the last step
    TUInt64 TmMSecs;

protected:
    /// update aggregate value
    void OnStep();

    /// Json constructor
    TThresholdAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Is the aggregate initialized?
    bool IsInit() const { return TmMSecs != TUInt64::Mn; }
    /// Resets the aggregate
    void Reset();
    /// Current values
    double GetFlt() const { return IsAboveP; }
    /// Current timestamp
    uint64 GetTmMSecs() const { return TmMSecs; }
    /// List of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "threshold"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Covariance.
class TCov : public TStreamAggr,
             public TStreamAggrOut::ITm,
             public TStreamAggrOut::IFlt {
private:
    /// Input X aggregate
    TWPt<TStreamAggr> InAggrX;
    /// Input X timeseries
    TWPt<TStreamAggrOut::ITmIO> InAggrTmIOX;
    /// Input X timeseries
    TWPt<TStreamAggrOut::IFltIO> InAggrFltIOX;

    /// Input Y aggregate
    TWPt<TStreamAggr> InAggrY;
    /// Input Y timeseries
    TWPt<TStreamAggrOut::ITmIO> InAggrTmIOY;
    /// Input Y timeseries
    TWPt<TStreamAggrOut::IFltIO> InAggrFltIOY;

    /// Covariance
    TSignalProc::TCov Cov;

protected:
    /// Update covariance
    void OnStep();

    /// Json constructor
    TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
    /// Resets the aggregate
    void Reset() { Cov.Reset(); }
    /// Get latest covariance
    double GetFlt() const { return Cov.GetCov(); }
    /// Get time of latest covariance
    uint64 GetTmMSecs() const { return Cov.GetTmMSecs(); }

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const;
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "covariance"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
// Moving Correlation.
class TCorr : public TStreamAggr,
              public TStreamAggrOut::ITm,
              public TStreamAggrOut::IFlt {
private:
    /// Input covariance aggregate
    TWPt<TStreamAggr> InAggrCov;
    /// Input covariance timeseries
    TWPt<TStreamAggrOut::ITm> InAggrTmCov;
    /// Input covariance timeseries
    TWPt<TStreamAggrOut::IFlt> InAggrFltCov;

    /// Input X variance aggregate
    TWPt<TStreamAggr> InAggrVarX;
    /// Input X variance timeseries
    TWPt<TStreamAggrOut::IFlt> InAggrFltVarX;

    /// Input Y variance aggregate
    TWPt<TStreamAggr> InAggrVarY;
    /// Input Y variance timeseries
    TWPt<TStreamAggrOut::IFlt> InAggrFltVarY;

    /// Current correlation value
    TFlt Corr;
    /// Current timestamp
    TUInt64 TmMSecs;

protected:
    /// Update current correlation values
    void OnStep();

    /// Initialize from json
    TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Initialize from json
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const;
    /// Resets the aggregate
    void Reset() { TmMSecs = 0;  Corr = 0; }
    /// Return current correlation value
    double GetFlt() const { return Corr; }
    /// Return current timestamp
    uint64 GetTmMSecs() const { return 0; }

    /// List input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const;
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "correlation"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Merger
class TMerger : public TQm::TStreamAggr {
private:

    /// Merger field map
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
        TMergerFieldMap(const int& _InFldId, const TJoinSeq& _InFldJoinSeq,
            const int& _TmFldId, const int& _OutFldId);
        TMergerFieldMap(TSIn& SIn);

        void Save(TSOut& SOut) const;
        void Load(TSIn& SIn);
    };

private:
    /// Pointer to output store
    TWPt<TStore> OutStore;
    /// Id of timestamp field
    TInt TimeFieldId;
    /// Names of the value output fields
    TStrV OutFldNmV;
    /// Field mappings from input stores' fields to output store's field
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
    /// Json constructor
    TMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal);
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    void Reset() { throw TQmExcept::New("TMerger::Reset() not implemented!"); }
    void CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm);

    PJsonVal SaveJson(const int& Limit) const;

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Stream aggregator type name
    static TStr GetType() { return "merger"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }

    /// Utility function that gets list of stores the merger will attach to
    /// according to the given parameters json (`fields` array)
    static TStrV GetStoreNm(const PJsonVal& ParamVal);

private:
    void InitFld(const TWPt<TQm::TBase> Base, const TMergerFieldMap& FieldMap,
            const TStr& InterpNm);
    // initialized internal structures
    void InitMerger(const TWPt<TQm::TBase> Base, const TStr& OutStoreNm, const TStr& OutTmFieldNm,
            const bool& CreateStoreP, const bool& ExactInterp, const TStrV& InterpV);

protected:
    void OnAddRec(const TQm::TRec& Rec);
    void OnTime(const uint64& TmMsec);
    void OnStep();

private:
    void OnAddRec(const TQm::TRec& Rec,  const int& FieldMapIdx);
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
    void OnTime(const uint64& TmMsec);
    void OnStep();

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

    /// Stream aggregator type name
    static TStr GetType() { return "resampler"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
// Resampler of univariate time series
class TUniVarResampler : public TStreamAggr,
                   public TStreamAggrOut::ITm,
                   public TStreamAggrOut::IFlt {
private:
    // Input aggregate
    TWPt<TStreamAggr> InAggr;
    // output aggregate
    TWPt<TStreamAggr> OutAggr;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input aggregate casted to time series
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;
    // field interpolators
    TSignalProc::PInterpolator Interpolator;
    // interval size
    TUInt64 IntervalMSecs;
    // Timestamp of last generated record
    TUInt64 InterpPointMSecs;
    // value of the last generated record
    TFlt InterpPointVal;
    // berfore first update
    TBool UpdatedP;

private:
    // InterpolatorV contains pair (input field, interpolator)
    TUniVarResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

public:
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    PJsonVal GetParam() const;
    void SetParam(const PJsonVal& ParamVal);

    void Reset() { throw TQmExcept::New("TResampler::Reset() not implemented!"); }

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    PJsonVal SaveJson(const int& Limit) const;

    uint64 GetTmMSecs() const { return InterpPointMSecs; }
    double GetFlt() const { return InterpPointVal; }

    /// Stream aggregator type name
    static TStr GetType() { return "resample"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }

protected:
    void OnStep();

private:
    // refreshes the interpolators to the specified time
    void RefreshInterpolators(const uint64& Tm);
    bool CanInterpolate();
};

///////////////////////////////
/// Feature extractor stream aggregate.
/// Calls GetFullV on feature space and returns result.
class TFtrExtAggr : public TStreamAggr,
                    public TStreamAggrOut::IFltVec,
                    public TStreamAggrOut::ISparseVec,
                    public TStreamAggrOut::IFtrSpace {
private:
    /// Number of records still needed to declare we are initialized
    TInt InitCount;
    /// Shall we update the feature space as we get new records?
    TBool UpdateP;
    /// Shall we extract full vectors
    TBool FullP;
    /// Shall we extract sparse vectors
    TBool SparseP;

    /// Feature space wrapped in the aggregate.
    /// We keep a smart pointer for it to hold it up while we exist
    PFtrSpace FtrSpace;

    /// Value of last full extracted vector
    TFltV FullVec;
    /// Value of last sparse extracted vector
    TIntFltKdV SpVec;

protected:
    /// Process new record
    void OnAddRec(const TRec& Rec);

    /// JSON constructor
    TFtrExtAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// JSON constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Get current stream aggregate parameters
    PJsonVal GetParam() const;
    /// Update stream aggregate parameters. Only parameters given will be updated,
    /// rest will be left as they are at the moment.
    void SetParam(const PJsonVal& ParamVal);

    /// Did we finish initialization
    bool IsInit() const { return InitCount == 0; }
    /// Reset feature space
    void Reset();

    /// Get number of elements in the last extracted full vector
    int GetVals() const { return FullVec.Len(); }
    /// Get ElN-th element from the last extracted full vector
    void GetVal(const int& ElN, TFlt& Val) const { Val = FullVec[ElN]; }
    /// Get the last extracted full vector
    void GetValV(TFltV& ValV) const { ValV = FullVec; }

    /// Get number of elements in the last extracted sparse vector
    int GetSparseVecLen() const { return SpVec.Len(); }
    /// Get ElN-th element from the last extracte sparse vector
    TIntFltKd GetSparseVecVal(const int& ElN) const { return SpVec[ElN]; }
    /// Get the last extracted sparse vector
    void GetSparseVec(TIntFltKdV& _SpVec) const { _SpVec = SpVec; }

    /// Return internal feature space
    PFtrSpace GetFtrSpace() const { return FtrSpace; }

    // serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "featureSpace"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Nearest Neighbor for Anomaly Detection stream aggregate.
class TNNAnomalyAggr: public TStreamAggr,
                      public TStreamAggrOut::ITm,
                      public TStreamAggrOut::IInt {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggrTm;
    /// Input aggregate casted to time value
    TWPt<TStreamAggrOut::ITm> InAggrValTm;
    /// Input aggregate
    TWPt<TStreamAggr> InAggrSparseVec;
    /// Input aggregate casted to sparse vector
    TWPt<TStreamAggrOut::ISparseVec> InAggrValSparseVec;

    /// the NN anomaly detector object
    TAnomalyDetection::TNearestNeighbor Model;

    /// last processed timestamp
    TUInt64 LastTimeStamp;
    /// severity observed at the last timestamp
    TInt LastSeverity;
    /// explanation for last non-zero serverity
    PJsonVal Explanation;

    /// JSON constructor
    TNNAnomalyAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
protected:
    /// Update NN anomaly detector
   void OnStep();
public:
    /// JSON constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TNNAnomalyAggr(Base, ParamVal); }

    /// implement TStreamAggr functions
    PJsonVal GetParam() const;
    void SetParam(const PJsonVal& ParamVal);

    /// Did we finish initialization
    bool IsInit() const { return Model.IsInit(); }
    void Reset();

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    uint64 GetTmMSecs() const { return LastTimeStamp.GetMsVal(); }
    int GetInt() const { return LastSeverity; }
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "nnAnomalyDetector"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Histogram stream aggregate.
/// Updates a histogram model, connects to a time series stream aggregate (such as TEma)
/// that implements TStreamAggrOut::IFltTm or a buffered aggregate that implements
/// TStreamAggrOut::IFltTmIO
class TOnlineHistogram : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
    // PARAMETERS (reconstructed from JSON)
    // Input aggregate: only one aggregate is expected on input, these just
    // provide access to different interfaces for convenience
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input timeseries (can be NULL if the input is a buffered aggregate)
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;
    /// Input windowed time series (can be NULL if the input is a timeseries aggregate)
    TWPt<TStreamAggrOut::IFltIO> InAggrFltIO;

    /// Is buffered input aggregate provided?
    TBool BufferedP;
    // Aggregate state
    TSignalProc::TOnlineHistogram Model;
protected:
    /// Update histogram
    void OnStep();

    /// JSON constructor
    TOnlineHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// JSON constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TOnlineHistogram(Base, ParamVal); }

    /// Did we finish initialization
    bool IsInit() const { return Model.IsInit(); }
    /// Resets the aggregate
    void Reset() { Model.Reset(); }
    /// Load from stream
    void LoadState(TSIn& SIn);
    /// Store state into stream
    void SaveState(TSOut& SOut) const;
    /// serilization to JSon
    PJsonVal SaveJson(const int& Limit) const { return Model.SaveJson(); }

    /// Finds the bin index given val, returns -1 if not found
    int FindBin(const double& Val) const { return Model.FindBin(Val); }
    /// Returns the bound
    double GetBoundN(const int& BoundN) const { return Model.GetBoundN(BoundN); }

    /// returns the number of bins
    int GetVals() const { return Model.GetBins(); }
    /// returns frequencies in a given bin
    void GetVal(const int& ElN, TFlt& Val) const { Val = Model.GetCountN(ElN); }
    /// returns the vector of frequencies
    void GetValV(TFltV& ValV) const { Model.GetCountV(ValV); }

    /// stream aggregator type name
    static TStr GetType() { return "onlineHistogram"; }
    /// stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// TDigest stream aggregate.
/// Data structure useful for percentile and quantile estimation for online data streams.
/// It can be added to any anomaly detector to set the number of alarms triggered as a
/// percentage of the total samples.
/// This is the Data Lib Sketch Implementation:
///    https://github.com/vega/datalib-sketch/blob/master/src/t-digest.js
/// Paper: Ted Dunning, Otmar Ertl
///    https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
class TTDigest : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Input timeseries
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;

    /// TDigest model
    TSignalProc::TTDigest Model;
    /// Vector of quantiles we want to track
    TFltV QuantileV;

protected:
    /// Update the model
    void OnStep();
    /// Add new data to statistics
    void Add(const TFlt& Val);

    /// Json constructor
    TTDigest(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load from stream
    void LoadState(TSIn& SIn);
    /// Store state into stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return Model.IsInit(); }
    /// Resets the aggregate
    void Reset() { }
    /// returns the number of clusters
    int GetVals() const { return Model.GetClusters(); }
    /// get current Quantile value vector
    void GetVal(const int& ElN, TFlt& Val) const;
    /// get current Quantile value vector
    void GetValV(TFltV& ValV) const;

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm());}
    /// Serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// Stream aggregator type name
    static TStr GetType() { return "tdigest"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Chi square stream aggregate.
/// Updates a chi square model, connects to an online histogram stream aggregate
/// that implements TStreamAggrOut::IFltVec, where vector represents some kind
/// of distribution
class TChiSquare : public TStreamAggr, public TStreamAggrOut::IFlt {
private:
    /// Input X aggregate
    TWPt<TStreamAggr> InAggrX;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValX;

    /// Input X aggregate
    TWPt<TStreamAggr> InAggrY;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValY;

    /// ChiSquare indicator
    TSignalProc::TChiSquare ChiSquare;

protected:
    /// Update aggregate
    void OnStep();

    /// Json constructor
    TChiSquare(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load from stream
    void LoadState(TSIn& SIn);
    /// Store state into stream
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
    /// Reset
    void Reset() { ChiSquare.Reset(); }
    /// Get current Chi2 value
    double GetFlt() const { return ChiSquare.GetChi2(); }

    /// Get list of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggrX->GetAggrNm());
        InAggrNmV.Add(InAggrY->GetAggrNm());}
    // serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;
    // stream aggregator type name
    static TStr GetType() { return "chiSquare"; }
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Histogram stream aggregate.
/// Updates a histogram model, connects to a time series stream aggregate (such as TEma)
/// that implements TStreamAggrOut::IFltTm or a buffered aggregate that implements
/// TStreamAggrOut::IFltTmIO
class TOnlineSlottedHistogram : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
    // Input aggregate: only one aggregate is expected on input, these just
    // provide access to different interfaces for convenience
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;

    /// Input timeseries (can be NULL if the input is a buffered aggregate)
    TWPt<TStreamAggrOut::ITm> InAggrTm;
    /// Input timeseries (can be NULL if the input is a buffered aggregate)
    TWPt<TStreamAggrOut::IFlt> InAggrFlt;

    /// Input windowed time series (can be NULL if the input is a timeseries aggregate)
    TWPt<TStreamAggrOut::ITmIO> InAggrTmIO;
    /// Input windowed time series (can be NULL if the input is a timeseries aggregate)
    TWPt<TStreamAggrOut::IFltIO> InAggrFltIO;

    /// Is buffered input aggregate provided?
    TBool BufferedP;

    /// Slotted histogram model
    TSignalProc::TSlottedHistogram Model;
    /// Last entered timestamp in msec
    TUInt64 LastTm;
    /// Window length in msec
    TUInt64 WndLen;
    /// Last computed value
    TFltV ValV;

protected:
    /// Update stream aggregate
    void OnStep();

    /// JSON constructor
    TOnlineSlottedHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// JSON constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TOnlineSlottedHistogram(Base, ParamVal); }

    /// Load aggregate state
    void LoadState(TSIn& SIn);
    /// Save aggregate state
    void SaveState(TSOut& SOut) const;

    /// Did we finish initialization
    bool IsInit() const { return true; }
    /// Reset the histogram model
    void Reset() { Model.Reset(); }

    /// returns the number of bins
    int GetVals() const { return Model.GetBins(); }
    /// returns frequencies in a given bin
    void GetVal(const int& ElN, TFlt& Val) const { Val = ValV[ElN]; }
    /// returns the vector of frequencies
    void GetValV(TFltV& _ValV) const { _ValV = ValV; }

    /// serilization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// stream aggregator type name
    static TStr GetType() { return "onlineSlottedHistogram"; }
    /// stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Vector difference stream aggregate.
/// Provides difference of two vector, usually distributions,
/// connects to an online stream aggregates
/// that implement TStreamAggrOut::IFltVec
class TVecDiff : public TStreamAggr, public TStreamAggrOut::IFltVec {
private:
    /// Input X aggregate
    TWPt<TStreamAggr> InAggrX;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValX;

    /// Input X aggregate
    TWPt<TStreamAggr> InAggrY;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValY;

    /// Current value
    TFltV ValV;

protected:
    /// Update current value
    void OnStep();

    /// Json constructor
    TVecDiff(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Json constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    /// Load aggregate state
    void LoadState(TSIn& SIn) { ValV.Load(SIn); }
    /// Save aggregate state
    void SaveState(TSOut& SOut) const { ValV.Save(SOut); }

    // did we finish initialization
    bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
    /// resets the aggregate
    void Reset() { }

    /// returns the number of bins
    int GetVals() const { return InAggrValX->GetVals(); }
    /// returns frequencies in a given bin
    void GetVal(const int& ElN, TFlt& Val) const { Val = ValV[ElN]; }
    /// returns the vector of frequencies
    void GetValV(TFltV& _ValV) const { _ValV = ValV; }

    /// serialization to JSon
    PJsonVal SaveJson(const int& Limit) const;

    /// stream aggregator type name
    static TStr GetType() { return "onlineVecDiff"; }
    /// stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Simple linear regression stream aggregate.
/// Takes a vector X (variates) and a vector Y (covariates) and fits a linear model Y = A + B * X.
/// The results can be accessed through SaveJson. The aggregate can also compute bands if a vector
/// of quantiles is provided. For example, given a vector [0.05, 0.95] the aggregate will compute
/// A,B and A_0 and A_1, so that 5% of Y will be below A_0 + B*X and 95% of Y will be below A_1 + B*X.
///
/// Given the example above, the JSON output:
/// { "intercept" : A, "slope" : B, "quantiles" : [0.05, 0.95], "bands" : [A_0, A_1] }
///
/// The input vectors are represented by two stream aggregates who implement
/// TStreamAggrOut::IFltVec.
class TSimpleLinReg : public TStreamAggr {
private:
    /// Input X aggregate
    TWPt<TStreamAggr> InAggrX;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValX;

    /// Input X aggregate
    TWPt<TStreamAggr> InAggrY;
    /// Input X dense vector aggregate
    TWPt<TStreamAggrOut::IFltVec> InAggrValY;

    /// Decides if a record should be processed (useful on a stream to improve performance)
    PRecFilter Filter;
    /// Quantiles that specify the band interecepts
    TFltV Quantiles;

    /// Current model prepared as JSon
    /// Example: { "intercept" : 2.0, "slope" : 1.1, "quantiles" : [0.05, 0.95], "bands" : [1.0, 3.0] }
    PJsonVal Result;

protected:
    /// Calls on step
    void OnAddRec(const TRec& Rec) { if (Filter->Filter(Rec)) { OnStep(); } }
    /// Calls on step
    void OnTime(const uint64& TmMsec) { OnStep(); }
    /// Fits the linear regression and computes the bands (if configured to do so)
    void OnStep();

    /// JSON based constructor.
    /// \param ParamVal Holds the store names and aggregate names and an optional key "quantiles"
    /// with a number array of numbers between 0 and 1 : {"storeX" : "storeNameX", "inAggrX" : "inAggrNameX",
    /// "storeY" : "storeNameY", "inAggrY" : "inAggrNameY", "quantiles" : [0.05, 0.95]}
    TSimpleLinReg(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Smart pointer constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TSimpleLinReg(Base, ParamVal); }

    /// Loads state which corresponds to the Result JSON
    void LoadState(TSIn& SIn) { Result = TJsonVal::Load(SIn); }
    /// Saves state which corresponds to the Result JSON
    void SaveState(TSOut& SOut) const { Result->Save(SOut); }

    /// Is the aggregate initialized?
    bool IsInit() const { return InAggrX->IsInit() && InAggrY->IsInit(); }
    /// Resets the aggregate
    void Reset();

    /// JSON serialization
    PJsonVal SaveJson(const int& Limit) const { return Result; }

    /// Stream aggregator type name
    static TStr GetType() { return "simpleLinearRegression"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Record filter stream aggregate
class TRecFilterAggr : public TStreamAggr {
private:
    /// Aggregate that will be called if the record passes the filter
    TWPt<TStreamAggr> Aggr;
    /// Decides if a record should be processed (useful on a stream to improve performance)
    TVec<PRecFilter> FilterV;

protected:
    /// Passes the record to Aggr
    void OnAddRec(const TRec& Rec);
    /// Passes the call to Aggr
    void OnTime(const uint64& TmMsec) { Aggr->OnTime(TmMsec); }
    /// Passes the call to Aggr
    void OnStep() { Aggr->OnStep(); }

    /// JSON based constructor.
    TRecFilterAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Smart pointer constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TRecFilterAggr(Base, ParamVal); }

    /// No sate to load
    void LoadState(TSIn& SIn) { }
    /// No state to save
    void SaveState(TSOut& SOut) const { }

    /// Is the aggregate initialized?
    bool IsInit() const { return true; }
    /// Resets the aggregate
    void Reset() { }
    /// JSON serialization
    PJsonVal SaveJson(const int& Limit) const { return TJsonVal::NewObj(); }
    /// Stream aggregator type name
    static TStr GetType() { return "recordFilterAggr"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

//////////////////////////////////////////////////////////////////////////////////
/// Histogram based anomaly detector aggregate
///
///   OnStep has two phases (predict and fit):
///   1. - reads a value from an input aggregate that implements IFlt
///      - computes the bin number given an input histogram aggregate, exposes the result by implementing INmInt with "index" as input
///      - uses THistogramToPMFModel to compute the severity (anomaly score) of the bin
///      - exposes the result by implementing INmInt with "severity" as input
///   2. - updates the input histogram aggregate
///      - updates THistogramToPMFModel
///      - exposes the PMF and severities through SaveJson
class THistogramAD : public TStreamAggr, public TStreamAggrOut::INmInt {
private:
    /// Input for prediction
    TWPt<TStreamAggrOut::IFlt> InAggrVal;
    /// Bandwidth will be recomputed every time Count is divisible with AutoBandwidthSkip
    TInt AutoBandwidthSkip;
    /// Input for modelling (histogram)
    TWPt<TOnlineHistogram> HistAggr;
    /// Current severity, returned by GetFlt(), corresponds to histogram bin with index LastHistIdx
    TInt Severity;
    /// The histogram bin index of the most recent prediction, returned by GetInt().
    TInt LastHistIdx;
    /// Current PMF, computed in OnStep
    TFltV PMF;
    /// Current anomaly scores, computed in OnStep
    TIntV Severities;
    /// Explanation object holds a summary of the histogram state prior to making the last prediction (it explains why a prediction was classified with a given severity)
    PJsonVal Explanation;
    /// PMF/AD model
    THistogramToPMFModel Model;
    /// Number of calls to OnStep
    TInt Count;

protected:
    /// Predicts the current severity and updates the histogram anomaly model
    void OnStep();
    /// JSON based constructor.
    THistogramAD(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Smart pointer constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new THistogramAD(Base, ParamVal); }
    /// Loads state
    void LoadState(TSIn& SIn);
    /// Saves state
    void SaveState(TSOut& SOut) const;
    /// Is the aggregate initialized?
    bool IsInit() const { return HistAggr->IsInit() && Severities.Len() > 0; }
    /// Returns true if the string is supported
    bool IsNmInt(const TStr& Nm) const { return(Nm == "index") || (Nm == "severity"); }
    /// Returns the current histogram bin index or current severity
    int GetNmInt(const TStr& Nm) const { return Nm == "index" ? LastHistIdx : Severity; }
    /// Resets the aggregate
    void Reset();
    /// JSON serialization
    PJsonVal SaveJson(const int& Limit) const;
    /// Stream aggregator type name
    static TStr GetType() { return "histogramAD"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

///////////////////////////////
/// Template class implementation
#include "qminer_aggr.hpp"

} // TStreamAggrs namespace
} // TQm namespace

#endif

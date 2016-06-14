/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "qminer_aggr.h"

#ifdef OG_AGGR_DOC_ATLAS
#include <gkswf.h>
#include <vizmapgks.h>
#endif

namespace TQm {

namespace TAggrs {
    
///////////////////////////////
// QMiner-Aggregator-Count
TCount::TCount(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt): TAggr(Base, AggrNm) {

    // prepare join path string, if necessary
    JoinPathStr = FtrExt->GetJoinSeq(RecSet->GetStoreId()).GetJoinPathStr(Base);
    // prepare field name
    FieldNm = FtrExt->GetNm();
    // prepare 
    const int Recs = RecSet->GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        TStrV FtrValV; FtrExt->ExtractStrV(RecSet->GetRec(RecN), FtrValV);
        for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
            ValH.AddDat(FtrValV[FtrValN])++; Count++;
        }
    }
    ValH.SortByDat(false);
}

TCount::TCount(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const PRecSet& RecSet, const int& KeyId): TAggr(Base, AggrNm) {

    // prepare key name
    FieldNm = Base->GetIndexVoc()->GetKeyNm(KeyId);
    // prepare counts
    TUInt64IntKdV ResV = RecSet->GetRecIdFqV();
    if (!ResV.IsSorted()) { ResV.Sort(); }
    const uint64 Words = Base->GetIndexVoc()->GetWords(KeyId);
    for (uint64 WordId = 0; WordId < Words; WordId++) {
        // prepare filter query
        TIntUInt64PrV FilterQueryItemV;
        FilterQueryItemV.Add(TIntUInt64Pr(KeyId, WordId));
        // execute query
        TUInt64IntKdV FilterV; Base->GetIndex()->SearchAnd(FilterQueryItemV, FilterV);
        // add to count
        FilterV.Intrs(ResV); const int WordFq = FilterV.Len();
        TStr WordStr = Base->GetIndexVoc()->GetWordStr(KeyId, WordId);
        ValH.AddDat(WordStr) = WordFq; Count += WordFq;
    }
    ValH.SortByDat(false);
}

PAggr TCount::New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PJsonVal& JsonVal) {

    if (JsonVal->IsObjKey("key")) {
        // we aggregate over a key
        TStr KeyNm = JsonVal->GetObjStr("key");
        // validate the key
        QmAssert(Base->GetIndexVoc()->IsKeyNm(RecSet->GetStoreId(), KeyNm));
        // get key id
        const int KeyId = Base->GetIndexVoc()->GetKeyId(RecSet->GetStoreId(), KeyNm);
        // forward the call
        return New(Base, AggrNm, RecSet, KeyId);
    } else {
        // we aggregate over a field, first parse join
        TJoinSeq JoinSeq = JsonVal->IsObjKey("join") ?
            TJoinSeq(Base, RecSet->GetStoreId(), JsonVal->GetObjKey("join")) :
            TJoinSeq(RecSet->GetStoreId());
        // get the field
        const TStr FieldNm = JsonVal->GetObjStr("field");
        // assert if valid field
        TWPt<TStore> Store = JoinSeq.GetEndStore(Base);
        QmAssert(Store->IsFieldNm(FieldNm));
        // get the field id
        const int FieldId = Store->GetFieldId(FieldNm);
        // prepare feature extractor
        PFtrExt FtrExt = TFtrExts::TMultinomial::New(Base, JoinSeq, FieldId);
        return New(Base, AggrNm, RecSet, FtrExt);
    }
}

PJsonVal TCount::SaveJson() const { 
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "count");
    ResVal->AddToObj("field", FieldNm);
    ResVal->AddToObj("join", JoinPathStr);

    TJsonValV ValValV;
    const double FltCount = (Count > 0) ? double(Count) : 1;
    int ValKeyId = ValH.FFirstKeyId();
    while (ValH.FNextKeyId(ValKeyId)) {
        const int ValFq = ValH[ValKeyId];
        const double Percent = 100.0 * (double(ValFq) / FltCount);
        PJsonVal ValVal = TJsonVal::NewObj();
        ValVal->AddToObj("value", ValH.GetKey(ValKeyId));
        ValVal->AddToObj("frequency", ValFq);
        ValVal->AddToObj("precent", double(TFlt::Round(Percent*100.0))/100.0);
        ValValV.Add(ValVal);
    }
    ResVal->AddToObj("values", ValValV);

    return ResVal;
}

///////////////////////////////
// QMiner-Aggregator-Histogram
THistogram::THistogram(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt, const int& Buckets):
            TAggr(Base, AggrNm) {

    // prepare join path string, if necessary
    JoinPathStr = FtrExt->GetJoinSeq(RecSet->GetStoreId()).GetJoinPathStr(Base);
    // prepare field name
    FieldNm = FtrExt->GetNm();
    // if empty result set no need to do histogams
    if (RecSet->Empty()) { return; }
    // find min and max for histogram
    double MnVal = TFlt::Mx, MxVal = TFlt::Mn;
    const int Recs = RecSet->GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        TFltV FtrValV; FtrExt->ExtractFltV(RecSet->GetRec(RecN), FtrValV);
        for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
            const double Val = FtrValV[FtrValN];
            MnVal = TFlt::GetMn(MnVal, Val);
            MxVal = TFlt::GetMx(MxVal, Val);
        }
    }
    // compute histogram
    Mom = TMom::New(); Sum = 0.0;
    Hist = THist(MnVal, MxVal, Buckets);
    for (int RecN = 0; RecN < Recs; RecN++) {
        TFltV FtrValV; FtrExt->ExtractFltV(RecSet->GetRec(RecN), FtrValV);
        for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
            const double FtrVal = FtrValV[FtrValN].Val;
            Mom->Add(FtrVal); Sum += FtrVal;
            Hist.Add(FtrValV[FtrValN], true); 
        }
    }
    Mom->Def();
}

PAggr THistogram::New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PJsonVal& JsonVal) {

    // parse join
    TJoinSeq JoinSeq = JsonVal->IsObjKey("join") ?
        TJoinSeq(Base, RecSet->GetStoreId(), JsonVal->GetObjKey("join")) :
        TJoinSeq(RecSet->GetStoreId());
    // first get the field
    const TStr FieldNm = JsonVal->GetObjStr("field");
    // assert if valid field
    TWPt<TStore> Store = JoinSeq.GetEndStore(Base);
    QmAssert(Store->IsFieldNm(FieldNm));
    // get the field id
    const int FieldId = Store->GetFieldId(FieldNm);
    // get the number of buckets
    const int Buckets = TFlt::Round(JsonVal->GetObjNum("buckets", 10.0));
    // prepare feature extractor
    PFtrExt FtrExt = TFtrExts::TNumeric::New(Base, JoinSeq, FieldId);
    return New(Base, AggrNm, RecSet, FtrExt, Buckets);
}

PJsonVal THistogram::SaveJson() const { 
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "histogram");
    ResVal->AddToObj("field", FieldNm);
    ResVal->AddToObj("join", JoinPathStr);

    if (Mom.Empty()) { return ResVal; }

    ResVal->AddToObj("count", Mom->GetVals());
    ResVal->AddToObj("sum", Sum);
    ResVal->AddToObj("min", Mom->GetMn());
    ResVal->AddToObj("max", Mom->GetMx());
    ResVal->AddToObj("mean", Mom->GetMean());
    ResVal->AddToObj("stdev", Mom->GetSDev());
    ResVal->AddToObj("median", Mom->GetMedian());

    TJsonValV ValValV;
    double PercentSum = 0.0;
    for (int BucketN = 0; BucketN < Hist.GetBuckets(); BucketN++) {
        const double Percent = 100.0 * Hist.GetBucketValPerc(BucketN);
        PercentSum += Percent;
        PJsonVal ValVal = TJsonVal::NewObj();
        ValVal->AddToObj("min", Hist.GetBucketMn(BucketN));
        ValVal->AddToObj("max", Hist.GetBucketMx(BucketN));
        ValVal->AddToObj("frequency", Hist.GetBucketVal(BucketN));
        ValVal->AddToObj("precent", double(TFlt::Round(Percent*100.0))/100.0);
        ValVal->AddToObj("percentSum", PercentSum);
        ValValV.Add(ValVal);
    }
    ResVal->AddToObj("values", ValValV);

    return ResVal;
}

///////////////////////////////
// QMiner-Aggregator-Bag-of-Words
TBow::TBow(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& _RecSet, const PFtrExt& FtrExt, const int& SampleSize,
        const TBowWordWgtType& WgtType):
            TAggr(Base, AggrNm) {

    // check if need to sample
    SampleRecSet = _RecSet;
    if (SampleSize != -1 && _RecSet->GetRecs() > SampleSize) {
        SampleRecSet = _RecSet->GetSampleRecSet(SampleSize);
    }
    // prepare documents
    BowDocBs = TBowDocBs::New();
    const TWPt<TStore>& RecStore = SampleRecSet->GetStore();
    for (int RecN = 0; RecN < SampleRecSet->GetRecs(); RecN++) {        
        TStr DocNm = RecStore->GetRecNm(SampleRecSet->GetRecId(RecN));
        //TStr DocNm = TUInt64::GetStr(SampleRecSet->GetRecId(RecN));
        TStrV WdStrV; FtrExt->ExtractStrV(SampleRecSet->GetRec(RecN), WdStrV);
        BowDocBs->AddDoc(DocNm, TStrV(), WdStrV);
    }
    BowDocWgtBs = TBowDocWgtBs::New(BowDocBs, WgtType);
}

void TBow::ParseJson(const TWPt<TBase>& Base, const PRecSet& RecSet,
       const PJsonVal& JsonVal, PFtrExt& FtrExt, int& SampleSize) {

    // parse join
    TJoinSeq JoinSeq = JsonVal->IsObjKey("join") ?
        TJoinSeq(Base, RecSet->GetStoreId(), JsonVal->GetObjKey("join")) :
        TJoinSeq(RecSet->GetStoreId());
    // get the field
    const TStr FieldNm = JsonVal->GetObjStr("field");
    // assert if valid field
    TWPt<TStore> Store = JoinSeq.GetEndStore(Base);
    QmAssert(Store->IsFieldNm(FieldNm));
    // get the field id
    const int FieldId = Store->GetFieldId(FieldNm);
    // get the sample size
    SampleSize = TFlt::Round(JsonVal->GetObjNum("sample", 1000.0));
    // get the type of tokenizator
    const bool TokenizeP = JsonVal->GetObjBool("tokenize", true);
    if (TokenizeP) {    
        // read stemmer    
        PStemmer Stemmer = JsonVal->IsObjKey("stemmer") ? 
            TStemmer::ParseJson(JsonVal->GetObjKey("stemmer"), true) :
            TStemmer::New(stmtNone, true);
        // read stop words (default is English)
        PSwSet SwSet = JsonVal->IsObjKey("stopwords") ? 
            TSwSet::ParseJson(JsonVal->GetObjKey("stopwords")) :
            TSwSet::New(swstEn523); 
        // tokenizor
        PTokenizer Tokenizer = TTokenizers::THtmlUnicode::New(SwSet, Stemmer);
        // prepare feature extractor
        FtrExt = TFtrExts::TBagOfWords::New(Base, JoinSeq, 
            FieldId, TFtrExts::bowmConcat, Tokenizer);
    } else {
        FtrExt = TFtrExts::TMultinomial::New(Base, JoinSeq, FieldId);
    }
}

///////////////////////////////
// QMiner-Aggregator-Keyword
TKeywords::TKeywords(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize):
            TBow(Base, AggrNm, RecSet, FtrExt, SampleSize) {

    // get keywords
    TIntV AllDIdV; BowDocBs->GetAllDIdV(AllDIdV);
    PBowSpV BowSpV = TBowClust::GetConceptSpV(BowDocWgtBs, TBowSim::New(bstCos), AllDIdV);
    // store top keywords
    KWordSet = BowSpV->GetKWordSet(BowDocBs);
    KWordSet->SortByWgt();
    // clear preprocessed stuff (not needed anymore)
    BowDocBs.Clr(); BowDocWgtBs.Clr();
}

PAggr TKeywords::New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PJsonVal& JsonVal) {

    // parse basic parameters for creating BOW
    PFtrExt FtrExt; int SampleSize;
    TBow::ParseJson(Base, RecSet, JsonVal, FtrExt, SampleSize);
    // create aggregator
    return New(Base, AggrNm, RecSet, FtrExt, SampleSize);
}

PJsonVal TKeywords::SaveKWordSet(const PBowKWordSet& KWordSet) {
    PJsonVal KeyWdsVal = TJsonVal::NewArr();
    const int KWords = TInt::GetMn(KWordSet->GetKWords(), 100);
    for (int KWordN = 0; KWordN < KWords; KWordN++) {
        PJsonVal KeyWdVal = TJsonVal::NewObj();
        KeyWdVal->AddToObj("keyword", KWordSet->GetKWordStr(KWordN));
        KeyWdVal->AddToObj("weight", KWordSet->GetKWordWgt(KWordN));
        KeyWdsVal->AddToArr(KeyWdVal);
    }
    return KeyWdsVal;
}

PJsonVal TKeywords::SaveJson() const { 
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "keywords");
    ResVal->AddToObj("keywords", SaveKWordSet(KWordSet));
    return ResVal;
}

///////////////////////////////
// QMiner-Aggregator-DocAtlas
#ifdef QM_AGGR_DOC_ATLAS
TDocAtlas::TDocAtlas(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt, const int& SampleSize):
            TBow(Base, AggrNm, RecSet, FtrExt, SampleSize) {

    // prepare visualization
    PVizMap VizMap = TVizMapFactory::NewVizMap(BowDocBs, BowDocWgtBs, 
        BowDocBs, BowDocWgtBs, 300, 200, 0.8, TEnv::Logger);
    // generate images
    PVizMapContext VizMapContext = TVizMapContext::New(VizMap);
    // parameters
    const int FontSize = 15;
    // get temporary folder
    TStr TempFPath = Base->GetTempFPath() + "DocAtlas/";
    if (!TDir::GenDir(TempFPath)) { TEnv::Logger->OnStatus("Error creating directory " + TempFPath); }
    // save small image
    TWfBmpGks SmallGks(500, 400);
    VizMapContext->Paint(SmallGks.GetGks(), false, FontSize, 0, 
        1.0, -1, false, true, FontSize, false, -1, -1);
    SmallFNm = TFile::GetUniqueFNm(TempFPath + "small.png"); 
    SmallGks.SavePng(SmallFNm);
    // save medium image    
    TWfBmpGks MediumGks(750, 600);
    VizMapContext->Paint(MediumGks.GetGks(), false, FontSize, 0, 
        1.0, -1, false, true, FontSize, false, -1, -1);
    MidFNm = TFile::GetUniqueFNm(TempFPath + "medium.png");
    MediumGks.SavePng(MidFNm);
    // save large image
    TWfBmpGks LargeGks(1000, 800);
    VizMapContext->Paint(LargeGks.GetGks(), false, FontSize, 0, 
        1.0, -1, false, true, FontSize, false, -1, -1);
    LargeFNm = TFile::GetUniqueFNm(TempFPath + "large.png");
    LargeGks.SavePng(LargeFNm);
    // clear preprocessed stuff (not needend anymore)
    BowDocBs.Clr(); BowDocWgtBs.Clr();
}

PAggr TDocAtlas::New(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PJsonVal& JsonVal) {

    // parse basic parameters for creating BOW
    PFtrExt FtrExt; int SampleSize;
    TBow::ParseJson(Base, RecSet, JsonVal, FtrExt, SampleSize);
    // create aggregator
    return New(Base, AggrNm, RecSet, FtrExt, SampleSize);
}

PJsonVal TDocAtlas::SaveJson() const { 
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "docatlas");
    ResVal->AddToObj("field", FieldNm);
    ResVal->AddToObj("join", JoinPathStr);
    ResVal->AddToObj("small", SmallFNm.GetFBase());
    ResVal->AddToObj("small_full", SmallFNm);
    ResVal->AddToObj("medium", MidFNm.GetFBase());
    ResVal->AddToObj("medium_full", MidFNm);
    ResVal->AddToObj("large", LargeFNm.GetFBase());
    ResVal->AddToObj("large_full", LargeFNm);
    return ResVal;
}
#endif

///////////////////////////////
// QMiner-Aggregator-TimeLine
PJsonVal TTimeLine::GetJsonList(const TStrH& StrH) const {
    const double FltCount = (Count > 0) ? double(Count) : 1.0;
    PJsonVal JsonVal = TJsonVal::NewArr();
    int KeyId = StrH.FFirstKeyId();
    while (StrH.FNextKeyId(KeyId)) {
        const int ValFq = StrH[KeyId];
        const double Percent = 100.0 * (double(ValFq) / FltCount);
        PJsonVal EltVal = TJsonVal::NewObj("interval", StrH.GetKey(KeyId));
        EltVal->AddToObj("frequency", ValFq);
        EltVal->AddToObj("precent", double(TFlt::Round(Percent*100.0))/100.0);
        JsonVal->AddToArr(EltVal);
    }
    return JsonVal;
}

TTimeLine::TTimeLine(const TWPt<TBase>& Base, const TStr& AggrNm,
        const PRecSet& RecSet, const PFtrExt& FtrExt): TAggr(Base, AggrNm) {

    // prepare join path string, if necessary
    JoinPathStr = FtrExt->GetJoinSeq(RecSet->GetStoreId()).GetJoinPathStr(Base);
    // prepare field name
    FieldNm = FtrExt->GetNm();
    // initialize hash-tables
    for (int MonthN = 0; MonthN < 12; MonthN++) {
        MonthH.AddKey(TTmInfo::GetMonthNm(MonthN+1)); }
    for (int DayOfWeekN = 0; DayOfWeekN < 7; DayOfWeekN++) {
        DayOfWeekH.AddKey(TTmInfo::GetDayOfWeekNm(DayOfWeekN+1)); }
    for (int HourOfDayN = 0; HourOfDayN < 24; HourOfDayN++) {
        HourOfDayH.AddKey(TInt::GetStr(HourOfDayN)); }
    // prepare peichart
    const int Recs = RecSet->GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        TTmV FtrValV; FtrExt->ExtractTmV(RecSet->GetRec(RecN), FtrValV);
        for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
            const TTm& Tm = FtrValV[FtrValN]; 
            if (Tm.IsDef()) {
                TSecTm SecTm(Tm); Count++;
                TStr DateStr = Tm.GetWebLogDateStr();
                AbsDateH.AddDat(DateStr)++;
                MonthH.AddDat(SecTm.GetMonthNm())++;
                DayOfWeekH.AddDat(SecTm.GetDayOfWeekNm())++;
                if (0 <= Tm.GetHour() && Tm.GetHour() < 24) { 
                    HourOfDayH[Tm.GetHour()]++; }
            }
        }
    }
    AbsDateH.SortByKey(true);
}

PAggr TTimeLine::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const PRecSet& RecSet, const PJsonVal& JsonVal) {

    // parse join
    TJoinSeq JoinSeq = JsonVal->IsObjKey("join") ?
        TJoinSeq(Base, RecSet->GetStoreId(), JsonVal->GetObjKey("join")) :
        TJoinSeq(RecSet->GetStoreId());
    // get the field
    const TStr FieldNm = JsonVal->GetObjStr("field");
    // assert if valid field
    TWPt<TStore> Store = JoinSeq.GetEndStore(Base);
    QmAssert(Store->IsFieldNm(FieldNm));
    // get the field id
    const int FieldId = Store->GetFieldId(FieldNm);
    // is there a join?
    PFtrExt FtrExt = TFtrExts::TMultinomial::New(Base, JoinSeq, FieldId);
    return New(Base, AggrNm, RecSet, FtrExt);
}

PJsonVal TTimeLine::SaveJson() const { 
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "timeline");
    ResVal->AddToObj("field", FieldNm);
    ResVal->AddToObj("join", JoinPathStr);

    ResVal->AddToObj("month", GetJsonList(MonthH));
    ResVal->AddToObj("day-of-week", GetJsonList(DayOfWeekH));
    ResVal->AddToObj("hour-of-day", GetJsonList(HourOfDayH));
    ResVal->AddToObj("date", GetJsonList(AbsDateH));

    return ResVal;
}

///////////////////////////////
// QMiner-Aggregator-TimeSpan

PJsonVal TTimeSpan::GetJsonList(const TUInt64H& DataH) const {
    PJsonVal JsonVal = TJsonVal::NewArr();
    int KeyId = DataH.FFirstKeyId();
    while (DataH.FNextKeyId(KeyId)) {
        const int ValFq = DataH[KeyId];
        uint64 UnixTs = TTm::GetUnixMSecsFromWinMSecs(DataH.GetKey(KeyId) * SlotLen);
        PJsonVal EltVal = TJsonVal::NewObj("slot", UnixTs);
        EltVal->AddToObj("count", ValFq);
        JsonVal->AddToArr(EltVal);
    }
    return JsonVal;
}

TTimeSpan::TTimeSpan(const TWPt<TBase>& Base, const TStr& AggrNm,
    const PRecSet& RecSet, const PFtrExt& FtrExt, const uint64 _SlotLen) 
    : TAggr(Base, AggrNm), SlotLen(_SlotLen) {

    // prepare join path string, if necessary
    JoinPathStr = FtrExt->GetJoinSeq(RecSet->GetStoreId()).GetJoinPathStr(Base);
    // prepare field name
    FieldNm = FtrExt->GetNm();
    // prepare peichart
    const int Recs = RecSet->GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        TTmV FtrValV; FtrExt->ExtractTmV(RecSet->GetRec(RecN), FtrValV);
        for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
            const TTm& Tm = FtrValV[FtrValN];
            if (Tm.IsDef()) {
                uint64 slot = TTm::GetMSecsFromTm(Tm) / SlotLen;
                CountsH.AddDat(slot)++;
            }
        }
    }
    CountsH.SortByKey(true);
}

PAggr TTimeSpan::New(const TWPt<TBase>& Base, const TStr& AggrNm,
    const PRecSet& RecSet, const PJsonVal& JsonVal) {

    // parse join
    TJoinSeq JoinSeq = JsonVal->IsObjKey("join") ?
        TJoinSeq(Base, RecSet->GetStoreId(), JsonVal->GetObjKey("join")) :
        TJoinSeq(RecSet->GetStoreId());
    // get the field
    const TStr FieldNm = JsonVal->GetObjStr("field");
    // get the field
    const TUInt64 SlotLen = JsonVal->GetObjUInt64("slot_length");
    // assert if valid field
    TWPt<TStore> Store = JoinSeq.GetEndStore(Base);
    QmAssert(Store->IsFieldNm(FieldNm));
    // get the field id
    const int FieldId = Store->GetFieldId(FieldNm);
    // is there a join?
    PFtrExt FtrExt = TFtrExts::TMultinomial::New(Base, JoinSeq, FieldId);
    return New(Base, AggrNm, RecSet, FtrExt, SlotLen);
}

PJsonVal TTimeSpan::SaveJson() const {
    PJsonVal ResVal = TJsonVal::NewObj();
    ResVal->AddToObj("type", "timespan");
    ResVal->AddToObj("field", FieldNm);
    ResVal->AddToObj("join", JoinPathStr);
    ResVal->AddToObj("slot_length", SlotLen);
    ResVal->AddToObj("slots", GetJsonList(CountsH));
    return ResVal;
}

}

namespace TStreamAggrs {

///////////////////////////////
// Record Id Buffer.
void TRecBuffer::OnAddRec(const TRec& Rec) {
    QmAssertR(Rec.IsByRef(), "TRecBuffer::OnAddRec supports records by ref only!"); 
    QmAssertR(Rec.GetStoreId() == Store->GetStoreId(), "TRecBuffer::OnAddRec record store id mismatch"); 
    Buffer.Update(Rec.GetRecId());
}

void TRecBuffer::OnTime(const uint64& TmMsec) { }

void TRecBuffer::OnStep() { }

TRecBuffer::TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
    TStreamAggr(Base, ParamVal), Buffer(ParamVal->GetObjInt("size")),
    Store(Base->GetStoreByStoreNm(ParamVal->GetObjStr("store"))) { }

PStreamAggr TRecBuffer::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TRecBuffer(Base, ParamVal); 
}

void TRecBuffer::LoadState(TSIn& SIn) {
    Buffer.Load(SIn);
}

void TRecBuffer::SaveState(TSOut& SOut) const {
    Buffer.Save(SOut);
}

PJsonVal TRecBuffer::SaveJson(const int& Limit) const {
    PJsonVal JsonVal = TJsonVal::NewObj();
    if (!Buffer.Empty()) {
        QmAssertR(Store->IsRecId(Buffer.GetOldest()), "TRecBuffer GetOldest returned invalid record id");
        QmAssertR(Store->IsRecId(Buffer.GetNewest()), "TRecBuffer GetNewest returned invalid record id");
        const TRec& OldestRec = Store->GetRec(Buffer.GetOldest());
        const TRec& NewestRec = Store->GetRec(Buffer.GetNewest());
        JsonVal->AddToObj("oldest", OldestRec.GetJson(GetBase(), true, false, false, false, true));
        JsonVal->AddToObj("newest", NewestRec.GetJson(GetBase(), true, false, false, false, true));    
    }
    return JsonVal;
}

///////////////////////////////
// Time series tick.
void TTimeSeriesTick::OnAddRec(const TRec& Rec) {
    TickVal = ValReader.GetFlt(Rec);
    TmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
    InitP = true;
}

void TTimeSeriesTick::OnTime(const uint64& Time) {
    TmMSecs = Time;
    InitP = true;
}

void TTimeSeriesTick::OnStep() {
    QmAssertR(false, "This should have not been executed.");
}

TTimeSeriesTick::TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
        TStreamAggr(Base, ParamVal) {
        
    // get input store
    TStr StoreNm = ParamVal->GetObjStr("store");
    TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
    // get time field
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
    TimeFieldId = Store->GetFieldId(TimeFieldNm);
    // get numeric field
    TStr TickValFieldNm = ParamVal->GetObjStr("value");
    TickValFieldId = Store->GetFieldId(TickValFieldNm);
    // initialize reader for getting numeric value
    ValReader = TFieldReader(Store->GetStoreId(), TickValFieldId, Store->GetFieldDesc(TickValFieldId));
    // make sure parameters make sense
    QmAssertR(Store->GetFieldDesc(TimeFieldId).IsTm(), "[Window buffer] field " + TimeFieldNm + " not of type 'datetime'");
    QmAssertR(ValReader.IsFlt(), "[Window buffer] field " + TickValFieldNm + " cannot be casted to 'double'");
}

PStreamAggr TTimeSeriesTick::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTimeSeriesTick(Base, ParamVal);
}

void TTimeSeriesTick::LoadState(TSIn& SIn) {
    InitP.Load(SIn);
    TickVal.Load(SIn);
    TmMSecs.Load(SIn);
}

void TTimeSeriesTick::SaveState(TSOut& SOut) const {
    InitP.Save(SOut);
    TickVal.Save(SOut);
    TmMSecs.Save(SOut);
}

PJsonVal TTimeSeriesTick::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", TickVal);
    Val->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Buffer values vector

void TWinBufFltV::OnAddRec(const TRec& Rec) {
    TFltV OutValV; InAggrVal->GetOutValV(OutValV);
    if (InAggr->IsInit()) {
        for (int ElN = 0; ElN < OutValV.Len(); ElN++) {
            Queue.Pop();
        }
        if (!InAggrVal->DelayedP()) {
            Queue.Push(InAggrVal->GetInVal());
        } else {
            TFltV InValV; InAggrVal->GetInValV(InValV);
            for (int ElN = 0; ElN < InValV.Len(); ElN++) {
                Queue.PushV(InValV);
            }
        }
    }
}

TWinBufFltV::TWinBufFltV(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // get input aggregate
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TWinBufFltV] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // cast it to get timeseries data
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(InAggr());
    QmAssertR(!InAggrVal.Empty(), "[TWinBufFltV] Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

// serialization to JSon
PJsonVal TWinBufFltV::SaveJson(const int& Limit) const {
    TFltV Vec; Queue.GetSubValVec(0, Queue.Len() - 1, Vec);
    return TJsonVal::NewArr(Vec);
}

///////////////////////////////
// Exponential Moving Average.
void TEma::OnStep() {
    if (InAggr->IsInit()) {
        Ema.Update(InAggrVal->GetFlt(), InAggrVal->GetTmMSecs());
    }
}


TEma::TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), Ema(ParamVal) {
    
    // get input aggregate
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TEma] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // cast it to get timeseries data
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggr());
    QmAssertR(!InAggrVal.Empty(), "[TEma] Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TEma::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TEma(Base, ParamVal);
}

void TEma::LoadState(TSIn& SIn) {
    Ema.Load(SIn);
}

void TEma::SaveState(TSOut& SOut) const {
    Ema.Save(SOut); 
}

PJsonVal TEma::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Ema.GetValue());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(Ema.GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Threshold aggregate
TThresholdAggr::TThresholdAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // get input aggregate
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TThresholdAggr] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // cast it to get timeseries data
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggr());
    QmAssertR(!InAggrVal.Empty(), "[TThresholdAggr] Stream aggregate does not implement IFltTm interface: " + InAggrNm);
    // parse model parameters
    QmAssertR(ParamVal->IsObjKey("threshold"), "Threshold is not defined!");
    Threshold = ParamVal->GetObjNum("threshold");
    // initialize with reset
    Reset();
}

void TThresholdAggr::OnStep() {
    if (InAggr->IsInit()) {
        IsAboveP = InAggrVal->GetFlt() > Threshold ? 1.0 : 0.0;
        TmMSecs = InAggrVal->GetTmMSecs();
    }
}

PStreamAggr TThresholdAggr::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TThresholdAggr(Base, ParamVal);
}

void TThresholdAggr::LoadState(TSIn& SIn) {
    Threshold.Load(SIn);
    IsAboveP.Load(SIn);
    TmMSecs.Load(SIn);
}

void TThresholdAggr::SaveState(TSOut& SOut) const {
    Threshold.Save(SOut);
    IsAboveP.Save(SOut);
    TmMSecs.Save(SOut);
}

void TThresholdAggr::Reset() {
    IsAboveP = 0;
    TmMSecs = TUInt64::Mx;
}

PJsonVal TThresholdAggr::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", IsAboveP);
    Val->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Exponential Moving Average for sparse vectors
void TEmaSpVec::OnStep() {
    if (InAggr->IsInit()) {
        TIntFltKdV Vals;
        InAggrVal->GetValV(Vals);
        Ema.Update(Vals, InAggrVal->GetTmMSecs());
    }
}
TEmaSpVec::TEmaSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal), Ema(ParamVal) {
    // get input aggregate
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TEmaSpVec] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // cast it to get timeseries sparse vectors
    InAggrVal = dynamic_cast<TStreamAggrOut::ISparseVecTm*>(InAggr());
    QmAssertR(!InAggrVal.Empty(), "[TEmaSpVec] Stream aggregate does not implement ISparseVecTm interface: " + InAggrNm);
}

PStreamAggr TEmaSpVec::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TEmaSpVec(Base, ParamVal);
}

void TEmaSpVec::LoadState(TSIn& SIn) {
    Ema.Load(SIn);
}

void TEmaSpVec::SaveState(TSOut& SOut) const {
    Ema.Save(SOut);
}

PJsonVal TEmaSpVec::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Ema.GetJson());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(Ema.GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Moving Covariance
void TCov::OnStep() {
    TFltV ValVX; InAggrValX->GetOutValV(ValVX);
    TUInt64V TmMSecsV; InAggrValX->GetOutTmMSecsV(TmMSecsV);
    TFltV ValVY; InAggrValY->GetOutValV(ValVY);
    if (InAggrX->IsInit() && InAggrY->IsInit()) {
        Cov.Update(InAggrValX->GetInVal(), InAggrValY->GetInVal(),
            InAggrValX->GetInTmMSecs(), ValVX, ValVY, TmMSecsV, InAggrValX->GetVals());
    }
}

TCov::TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Cov(ParamVal) {
    // get input aggregate
    TStr InAggrNmX = ParamVal->GetObjStr("inAggrX");
    QmAssertR(Base->IsStreamAggr(InAggrNmX), "[TCov] Stream aggregate does not exist: " + InAggrNmX);
    InAggrX = Base->GetStreamAggr(InAggrNmX);
    // cast it to get timeseries sparse vectors
    InAggrValX = dynamic_cast<TStreamAggrOut::IFltTmIO*>(InAggrX());
    QmAssertR(!InAggrValX.Empty(), "[TCov] Stream aggregate does not implement IFltTm interface: " + InAggrNmX);
    
    // get input aggregate
    TStr InAggrNmY = ParamVal->GetObjStr("inAggrY");
    QmAssertR(Base->IsStreamAggr(InAggrNmY), "[TCov] Stream aggregate does not exist: " + InAggrNmY);
    InAggrY = Base->GetStreamAggr(InAggrNmY);
    // cast it to get timeseries sparse vectors
    InAggrValY = dynamic_cast<TStreamAggrOut::IFltTmIO*>(InAggrY());
    QmAssertR(!InAggrValY.Empty(), "[TCov] Stream aggregate does not implement IFltTm interface: " + InAggrNmY);
}

PStreamAggr TCov::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCov(Base, ParamVal);
}

PJsonVal TCov::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Cov.GetCov());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(Cov.GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Moving Correlation
void TCorr::InitInAggr(const TStr& InAggrNmCov, const TStr& InAggrNmVarX, const TStr& InAggrNmVarY) {
    // covariance cast
    QmAssertR(Base->IsStreamAggr(InAggrNmCov), "[TCorr] Stream aggregate does not exist: " + InAggrNmCov);
    InAggrCov = Base->GetStreamAggr(InAggrNmCov);
    InAggrValCov = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggrCov());
    QmAssertR(!InAggrValCov.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmCov);
    // X variance cast
    QmAssertR(Base->IsStreamAggr(InAggrNmVarX), "[TCorr] Stream aggregate does not exist: " + InAggrNmVarX);
    InAggrVarX = Base->GetStreamAggr(InAggrNmVarX);
    InAggrValVarX = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggrVarX());
    QmAssertR(!InAggrValVarX.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmVarX);
    // Y variance cast
    QmAssertR(Base->IsStreamAggr(InAggrNmVarY), "[TCorr] Stream aggregate does not exist: " + InAggrNmVarY);
    InAggrVarY = Base->GetStreamAggr(InAggrNmVarY);
    InAggrValVarY = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggrVarY());
    QmAssertR(!InAggrValVarY.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmVarY);        
}

void TCorr::OnStep() {
    //Corr = InAggrValCov->GetFlt() / ( sqrt(InAggrValVarX->GetFlt()) * sqrt(InAggrValVarY->GetFlt()));
    TFlt Cov = InAggrValCov->GetFlt();
    TFlt Var1 = InAggrValVarX->GetFlt();
    TFlt Var2 = InAggrValVarY->GetFlt();
    if ((Var1 == 0.0) || (Var2 == 0.0)) {
        Corr = 1;
    } else {
        Corr = Cov / ( sqrt(Var1) * sqrt(Var2) );
    }
    TmMSecs = InAggrValCov->GetTmMSecs();
}

TCorr::TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // parse out input aggregate
    TStr InAggrNmCov = ParamVal->GetObjStr("inAggrCov");
    TStr InAggrNmVarX = ParamVal->GetObjStr("inAggrVarX");
    TStr InAggrNmVarY = ParamVal->GetObjStr("inAggrVarY");
    // get references to input aggregates
    InitInAggr(InAggrNmCov, InAggrNmVarX, InAggrNmVarY);
}

PStreamAggr TCorr::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCorr(Base, ParamVal);
}

bool TCorr::IsInit() const {
    return InAggrVarX->IsInit() && InAggrVarY->IsInit() && InAggrCov->IsInit();
}

void TCorr::GetInAggrNmV(TStrV& InAggrNmV) const {
    InAggrNmV.Add(InAggrCov->GetAggrNm()); 
    InAggrNmV.Add(InAggrVarX->GetAggrNm());
    InAggrNmV.Add(InAggrVarY->GetAggrNm());
}

PJsonVal TCorr::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Corr);
    Val->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

//////////////////////////////////////////////
// Merger
TMerger::TMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal) {
    
    QmAssertR(ParamVal->IsObjKey("outStore"), "Field 'outStore' missing!");
    QmAssertR(ParamVal->IsObjKey("timestamp"), "Field 'timestamp' missing!");
    QmAssertR(ParamVal->IsObjKey("fields"), "Field 'fields' missing!");

    //input parameters
    TStr OutStoreNm = ParamVal->GetObjStr("outStore");
    TWPt<TStore> OutStore = Base->GetStoreByStoreNm(OutStoreNm);
    const bool CreateStoreP = ParamVal->GetObjBool("createStore", false);
    const bool Past = ParamVal->GetObjBool("onlyPast", false);
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
    PJsonVal FieldArrVal = ParamVal->GetObjKey("fields");
    TStrV InterpNmV;

    for (int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
        PJsonVal FieldVal = FieldArrVal->GetArrVal(FieldN);

        PJsonVal SourceVal = FieldVal->GetObjKey("source");
        TJoinSeq JoinSeq;
        TStr InFieldNm = FieldVal->GetObjStr("inField");
        TStr OutFieldNm = FieldVal->GetObjStr("outField");
        TStr TimeFieldNm = FieldVal->GetObjStr("timestamp");
        TStr InterpNm = FieldVal->GetObjStr("interpolation");
        InterpNmV.Add(InterpNm);

        TWPt<TStore> StartStore;

        if (SourceVal->IsStr()) {
            // we have just store name
            TStr inStoreNm = SourceVal->GetStr();
            StartStore = Base->GetStoreByStoreNm(inStoreNm);
            TInt TimeField = StartStore->GetFieldId(TimeFieldNm);
            TInt OutFieldId = OutStore->GetFieldId(OutFieldNm);

            JoinSeq = TJoinSeq(Base->GetStoreByStoreNm(inStoreNm)->GetStoreId());           
            TWPt<TStore> EndStore = JoinSeq.GetEndStore(Base);
            TInt InFieldId = EndStore->GetFieldId(InFieldNm);

            TMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
            FieldMapV.Add(MergerFieldMap);
            
        } else if (SourceVal->IsObj()) {
            // get store
            TStr inStoreNm = SourceVal->GetObjStr("store");   
            StartStore = Base->GetStoreByStoreNm(inStoreNm);
            TInt TimeField = StartStore->GetFieldId(TimeFieldNm);
            TInt OutFieldId = OutStore->GetFieldId(OutFieldNm);
            // get joins if any given
            if (SourceVal->IsObjKey("join")) {
                JoinSeq = TJoinSeq(Base, Base->GetStoreByStoreNm(inStoreNm)->GetStoreId(), SourceVal->GetObjKey("join"));               
                TWPt<TStore> EndStore = JoinSeq.GetEndStore(Base);
                TInt InFieldId = EndStore->GetFieldId(InFieldNm);

                TMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
                FieldMapV.Add(MergerFieldMap);
            } else {
                JoinSeq = TJoinSeq(Base->GetStoreByStoreNm(inStoreNm)->GetStoreId());
                TWPt<TStore> EndStore = JoinSeq.GetEndStore(Base);
                TInt InFieldId = EndStore->GetFieldId(InFieldNm);

                TMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
                FieldMapV.Add(MergerFieldMap);
            }
        }  
    
        TWPt<TStore> EndStore = JoinSeq.GetEndStore(Base);

        // check if timestamp field exsists in in store
        QmAssertR(StartStore->IsFieldNm(TimeFieldNm), "Field " + InFieldNm + " does not exist in store: " + StartStore->GetStoreNm() + "!");
        // check if the input field exists
        QmAssertR(EndStore->IsFieldNm(InFieldNm), "Field " + InFieldNm + " does not exist in store: " + EndStore->GetStoreNm() + "!");
        // check if the output field exists in the out store
        QmAssertR(OutStore->IsFieldNm(OutFieldNm), "Field " + OutFieldNm + " does not exist in the output store!");
        // check if the output field is of correct type
        QmAssertR(OutStore->GetFieldDesc(OutStore->GetFieldId(OutFieldNm)).GetFieldType() == TFieldType::oftFlt, "Field " + OutFieldNm + " is of incorrect type!");

    }

    InitMerger(Base, OutStoreNm, TimeFieldNm, CreateStoreP, Past, InterpNmV);
}

void TMerger::CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm){
    // prepare store description
    PJsonVal JsonStore = TJsonVal::NewObj();
    JsonStore->AddToObj("name", NewStoreNm);
    // prepare list of fields
    PJsonVal FieldsVal = TJsonVal::NewArr();
    //adding time field
    PJsonVal TimeFieldVal = TJsonVal::NewObj();
    TimeFieldVal->AddToObj("name", NewTimeFieldNm);
    TimeFieldVal->AddToObj("type", "datetime");
    FieldsVal->AddToArr(TimeFieldVal);  
    //adding TFlt fields from StoresAndFields vector
    for (int FieldMapN = 0; FieldMapN < NInFlds; FieldMapN++){
        //creating field 
        PJsonVal FieldVal = TJsonVal::NewObj();
        FieldVal->AddToObj("name", OutFldNmV[FieldMapN]);
        FieldVal->AddToObj("type", "float");
        FieldsVal->AddToArr(FieldVal);
    }
    // putting Store description together
    JsonStore->AddToObj("fields", FieldsVal);
    // create new store
    TStorage::CreateStoresFromSchema(TStreamAggr::GetBase(), JsonStore, 1024);
}

PStreamAggr TMerger::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TMerger(Base, ParamVal);
}

void TMerger::LoadState(TSIn& SIn) {
    TSignalProc::TLinkedBuffer<TUInt64> _Buff(SIn);
    Buff = _Buff;
    SignalsPresentV.Load(SIn);
    SignalsPresent.Load(SIn);
    NextInterpTm.Load(SIn);
    PrevInterpTm.Load(SIn);
    OnlyPast.Load(SIn);
    PrevInterpPt.Load(SIn);
}

void TMerger::SaveState(TSOut& SOut) const {
    Buff.Save(SOut);
    SignalsPresentV.Save(SOut);
    SignalsPresent.Save(SOut);
    NextInterpTm.Save(SOut);
    PrevInterpTm.Save(SOut);
    OnlyPast.Save(SOut);
    PrevInterpPt.Save(SOut);
}

PJsonVal TMerger::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", 0);
    Val->AddToObj("Time", 0);
    return Val;
}

TStrV TMerger::GetStoreNm(const PJsonVal& ParamVal) {
    // check we have list
    QmAssertR(ParamVal->IsObjKey("fields"), "Missing argument 'fields'!");
    // get the array
    TStrV StoreNmV;
    PJsonVal FieldArrVal = ParamVal->GetObjKey("fields");
    for (int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
        // get source
        PJsonVal SourceVal = FieldArrVal->GetArrVal(FieldN)->GetObjKey("source");
        // get store name
        if (SourceVal->IsStr()) {
            // we have just store name
            StoreNmV.Add(SourceVal->GetStr());
        } else if (SourceVal->IsObj()) {
            // we have object with store name as a field
            StoreNmV.Add(SourceVal->GetObjStr("store"));
        } else {
            // error
            throw TQmExcept::New("[TMerger] Unparsable source field " + SourceVal->SaveStr());
        }
    }
    return StoreNmV;
}

void TMerger::InitFld(const TWPt<TQm::TBase> Base, const TMergerFieldMap& FieldMap, 
        const TStr& InterpNm) {
    uint InStoreId = FieldMap.InFldJoinSeq.GetStartStoreId();
    if (!StoreIdFldIdVH.IsKey(InStoreId)) {
        StoreIdFldIdVH.AddDat(InStoreId, TIntSet());
    }

    // add field to internal structures
    InterpV.Add(TSignalProc::TInterpolator::New(InterpNm));
    OutFldNmV.Add(OutStore->GetFieldNm(FieldMap.OutFldId));
    StoreIdFldIdVH.GetDat(InStoreId).AddKey(InterpV.Len()-1);
}

void TMerger::InitMerger(const TWPt<TQm::TBase> Base, const TStr& OutStoreNm,
        const TStr& OutTmFieldNm, const bool& CreateStoreP, const bool& Past,
        const TStrV& InterpV) {

    // initialize output store
    // if required, create output store
    if (CreateStoreP) {
        InfoNotify("Creating store '" + OutStoreNm + "'");
        CreateStore(OutStoreNm, OutTmFieldNm);
    } else {
        OutStore = Base->GetStoreByStoreNm(OutStoreNm);
    }

    NextInterpTm = TUInt64::Mx;
    PrevInterpTm = TUInt64::Mx;
    NInFlds = InterpV.Len();

    // exact interpolation
    OnlyPast = Past;
    PrevInterpPt = TTriple<TUInt64, TFltV, TUInt64>(TUInt64::Mx, TFltV(), TUInt64::Mx);

    TimeFieldId = OutStore->GetFieldId(OutTmFieldNm);

    QmAssertR(InterpV.Len() == FieldMapV.Len(), "Invalid number of interpolators: " + TInt::GetStr(InterpV.Len()));

    // initialize in fields
    for (int i = 0; i < NInFlds; i++) {
        InitFld(Base, FieldMapV[i], InterpV[i]);
    }

    SignalsPresentV.Gen(NInFlds);
}

void TMerger::OnAddRec(const TQm::TRec& Rec) {
    // extract all the input fields that belong to this store
    const TWPt<TStore> Store = Rec.GetStore();
    const uint& StoreId = Store->GetStoreId();

    const TIntSet& FieldMapIdxSet = StoreIdFldIdVH.GetDat(StoreId);

    // iterate through the input fields and call the other OnAddRecMethod
    int KeyId = FieldMapIdxSet.FFirstKeyId();
    while (FieldMapIdxSet.FNextKeyId(KeyId)) {
        const TInt& FieldMapIdx = FieldMapIdxSet.GetKey(KeyId);
        OnAddRec(Rec, FieldMapIdx);
    }
}

void TMerger::OnTime(const uint64& TmMsec) {
    QmAssertR(false, "Merger::OnTime(const uint64& TmMsec) not supported.");
}

void TMerger::OnStep() {
    QmAssertR(false, "Merger::OnStep() should not be executed.");
}

void TMerger::OnAddRec(const TQm::TRec& Rec, const TInt& FieldMapIdx) {
    const int InterpIdx = FieldMapIdx; //StoreIdFldIdPrBuffIdxH.GetDat(StoreIdInFldIdPr);
    
    // get record time and value
    TTm Tm; Rec.GetFieldTm(FieldMapV[InterpIdx].TmFldId, Tm);
    const uint64 RecTm = TTm::GetMSecsFromTm(Tm);
    TRec JoinRec;
    // do single join and get the record
    if (FieldMapV[FieldMapIdx].InFldJoinSeq.GetJoinIdV().Len() > 0) {
        JoinRec = Rec.DoSingleJoin(GetBase(), FieldMapV[FieldMapIdx].InFldJoinSeq.GetJoinIdV());
    } else {
        JoinRec = Rec;
    }
    // extract the value
    const TFlt RecVal = JoinRec.GetFieldFlt(FieldMapV[FieldMapIdx].InFldId);

    QmAssertR(NextInterpTm == TUInt64::Mx || RecTm >= NextInterpTm, "Timestamp of the next record is lower then the current interpolation time!");

    AddToBuff(InterpIdx, RecTm, RecVal);

    // checks
    if (!CheckInitialized(InterpIdx, RecTm)) { return; }
    HandleEdgeCases(RecTm);

    // interpolate points
    while (CanInterpolate()) {
        // interpolate point
        TFltV ValV(NInFlds,0);
        for (int i = 0; i < NInFlds; i++) {
            ValV.Add(InterpV[i]->Interpolate(NextInterpTm));
        }

        // add the record to the output store
        AddRec(ValV, NextInterpTm, Rec);
        // update the next interpolation time
        UpdateNextInterpTm();
    }
}

void TMerger::AddToBuff(const int& InterpIdx, const uint64 RecTm, const TFlt& Val) {
    uint64 LastTm = Buff.Empty() ? TUInt64::Mn : Buff.GetNewest().Val;
    QmAssertR(RecTm >= LastTm, "TMerger::AddToBuff: Tried to merge past value!");

    InterpV[InterpIdx]->AddPoint(Val, RecTm);

    if (RecTm > LastTm) {
        Buff.Add(RecTm);
    }
}

void TMerger::AddToStore(const TFltV& InterpValV, const uint64 InterpTm, const uint64& RecId) {
    PJsonVal JsonVal = TJsonVal::NewObj();  //creating JSon object
    JsonVal->AddToObj(OutStore->GetFieldNm(TimeFieldId), TTm::GetTmFromMSecs(InterpTm).GetWebLogDateTimeStr(true, "T", true));

    for (int i = 0; i < NInFlds; i++) {
        JsonVal->AddToObj(OutFldNmV[i], InterpValV[i]);
    }

    uint64 NewRecId = OutStore->AddRec(JsonVal);
    if (OutStore->IsJoinNm("source")) {
        OutStore->AddJoin(OutStore->GetJoinId("source"), NewRecId, RecId, 1);
    }
}

void TMerger::AddRec(const TFltV& InterpValV, const uint64 InterpTm, const TQm::TRec& Rec) {
    if (OnlyPast) {
        // we need to wait until we get at least one future point before
        // committing the interpolation
        if (Buff.Len() > 1) {   // we already have a future point
            AddToStore(InterpValV, InterpTm, Rec.GetRecId());
            PrevInterpPt = TTriple<TUInt64, TFltV, TUInt64>(TUInt64::Mx, TFltV(), TUInt64::Mx);
        }
        else if (PrevInterpPt.Val1 != TUInt64::Mx && PrevInterpPt.Val1 != InterpTm) {
            AddToStore(PrevInterpPt.Val2, PrevInterpPt.Val1, PrevInterpPt.Val3);
            PrevInterpPt = TTriple<TUInt64, TFltV, TUInt64>(InterpTm, InterpValV, Rec.GetRecId());
        }
        else {
            // don't add to store, we need to see if the next value will have
            // the same time stamp
            PrevInterpPt = TTriple<TUInt64, TFltV, TUInt64>(InterpTm, InterpValV, Rec.GetRecId());
        }
    } else {
        AddToStore(InterpValV, InterpTm, Rec.GetRecId());
    }
}

void TMerger::ShiftBuff() {
    while (!Buff.Empty() && Buff.GetOldest() < NextInterpTm) {
        Buff.DelOldest();
    }

    UpdateInterpolators();
}

bool TMerger::AllSignalsPresent() {
    if (SignalsPresent) { return true; }

    for (int i = 0; i < NInFlds; i++) {
        if (!SignalsPresentV[i]) {
            return false;
        }
    }

    SignalsPresent = true;
    return true;
}

bool TMerger::CanInterpolate() {
    if (NextInterpTm == TUInt64::Mx) { return false; }  // this happens when all time series had the same timestamp in the previous iteration
    if (!OnlyPast && NextInterpTm == PrevInterpTm) { return false; }    // avoid duplicates when extrapolating future values

    for (int i = 0; i < NInFlds; i++) {
        if (!InterpV[i]->CanInterpolate(NextInterpTm)) {
            return false;
        }
    }

    return true;
}

void TMerger::UpdateNextInterpTm() {
    PrevInterpTm = NextInterpTm;
    NextInterpTm = Buff.Len() > 1 ? Buff.GetOldest(1).Val : TUInt64::Mx;

    QmAssertR(PrevInterpTm <= NextInterpTm, "The previous interpolation time is greater than the current interpolation time current: " + TUInt64::GetStr(PrevInterpTm) + ", next: " + TUInt64::GetHexStr(NextInterpTm) + "TMerger::UpdateNextInterpTm()");

    ShiftBuff();
}

void TMerger::UpdateInterpolators() {
    if (NextInterpTm == TUInt64::Mx) { return; }    // edge case, no points in the buffer

    for (int i = 0; i < NInFlds; i++) {
        InterpV[i]->SetNextInterpTm(NextInterpTm);
    }
}

bool TMerger::CheckInitialized(const int& InterpIdx, const uint64& RecTm) {
    if (!AllSignalsPresent()) {
        SignalsPresentV[InterpIdx] = true;

        if (!AllSignalsPresent()) { return false; } // should not continue

        NextInterpTm = RecTm;
        ShiftBuff();
    }

    return true;
}

void TMerger::HandleEdgeCases(const uint64& RecTm) {
    // the buffer was empty before this iteration,
    // the next interpolation time is not set
    if (NextInterpTm == TUInt64::Mx) {
        EAssertR(Buff.Len() == 1, "TMerger::HandleEdgeCases: The buffer is not empty even though it should be!");
        NextInterpTm = RecTm;
        UpdateInterpolators();
    }
    // duplicate value when extrapolating future
    if (!OnlyPast && NextInterpTm == PrevInterpTm) {
        NextInterpTm = TUInt64::Mx;
        Buff.DelOldest();
    }
}

///////////////////////////////
// Resampler
void TResampler::OnAddRec(const TRec& Rec) {
    QmAssertR(Rec.GetStoreId() == InStore->GetStoreId(), "Wrong store calling OnAddRec in Resampler");
    // get record time
    const uint64 RecTmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);

    // update interpolators
    for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {
        // get field value
        const double Val = Rec.GetFieldFlt(InFieldIdV[FieldN]);
        // update interpolator
        InterpolatorV[FieldN]->AddPoint(Val, RecTmMSecs);
    }

    // only do this first time when interpolation time not defined
    if (InterpPointMSecs == 0) {
        InterpPointMSecs = RecTmMSecs;
        RefreshInterpolators(RecTmMSecs);
    }

    // warning if Rec time > InterpPointMSecs, this is the first update and we cannot interpolate
    if (!UpdatedP && !CanInterpolate()) {
        if (InterpPointMSecs < RecTmMSecs) {
            InfoLog("Warning: resampler: start interpolation time is lower than the first record time, cannot interpolate. If future timestamps will keep increasing it might be possible that the resampler will be stuck and unable to interpolate.");
        }
    }
    UpdatedP = true;

    // insert new records while the interpolators allow us
    while (InterpPointMSecs <= RecTmMSecs && CanInterpolate()) {
        // we start existing record
        PJsonVal JsonVal = Rec.GetJson(GetBase(), true, false, false, false, false);
        // update timestamp
        TStr RecTmStr = TTm::GetTmFromMSecs(InterpPointMSecs).GetWebLogDateTimeStr(true, "T", true);
        JsonVal->AddToObj(InStore->GetFieldNm(TimeFieldId), RecTmStr);

        // update fields
        for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {
            const double FieldVal = InterpolatorV[FieldN]->Interpolate(InterpPointMSecs);
            EAssertR(!TFlt::IsNan(FieldVal), "TResampler: interpolated to a NaN value!");
            JsonVal->AddToObj(InStore->GetFieldNm(InFieldIdV[FieldN]), FieldVal);
        }

        //TODO use TRec instead of PJsonVal
        // add new record
        uint64 NewRecId = OutStore->AddRec(JsonVal);
        if (OutStore->IsJoinNm("source")) {
            OutStore->AddJoin(OutStore->GetJoinId("source"), NewRecId, Rec.GetRecId(), 1);
        }

        InterpPointMSecs += IntervalMSecs;
    }

    RefreshInterpolators(RecTmMSecs);
}

void TResampler::OnTime(const uint64& TmMsec) { QmAssertR(false, "TResampler::OnTime(const uint64& TmMsec): not supported."); }
void TResampler::OnStep() { QmAssertR(false, "TResampler::OnStep(): should not be executed."); }

void TResampler::RefreshInterpolators(const uint64& Tm) {
    // update time in the interpolators
    for (int i = 0; i < InterpolatorV.Len(); i++) {
        InterpolatorV[i]->SetNextInterpTm(Tm);
    }
}

bool TResampler::CanInterpolate() {
    RefreshInterpolators(InterpPointMSecs);
    for (int i = 0; i < InterpolatorV.Len(); i++) {
        if (!InterpolatorV[i]->CanInterpolate(InterpPointMSecs)) {
            return false;
        }
    }
    return true;
}

void TResampler::CreateStore(const TStr& NewStoreNm) {    
    // prepare store description
    PJsonVal StoreVal = TJsonVal::NewObj("name", NewStoreNm); 
    PJsonVal FieldsVal = TJsonVal::NewArr();
    // insert time field
    PJsonVal TmFieldVal=TJsonVal::NewObj();
    TmFieldVal->AddToObj("name", InStore->GetFieldNm(TimeFieldId));
    TmFieldVal->AddToObj("type", "datetime");
    FieldsVal->AddToArr(TmFieldVal);    
    // insert interpolated fields    
    for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {
        PJsonVal FieldVal = TJsonVal::NewObj();
        FieldVal->AddToObj("name", InStore->GetFieldNm(InFieldIdV[FieldN]));
        FieldVal->AddToObj("type", "float");
        FieldsVal->AddToArr(FieldVal);
    }
    StoreVal->AddToObj("fields", FieldsVal);
    // join that points to the original store (each record in the resampled 
    // store points to the most recent record in the orinal store)
    PJsonVal JoinsVal = TJsonVal::NewArr();
    PJsonVal JoinVal = TJsonVal::NewObj();
    JoinVal->AddToObj("name", "source");
    JoinVal->AddToObj("type", "field");
    JoinVal->AddToObj("store", InStore->GetStoreNm());
    JoinsVal->AddToArr(JoinVal);
    StoreVal->AddToObj("joins", JoinsVal);
    // create store
    InfoLog("Creating new store '" + NewStoreNm + "'");    
    TStorage::CreateStoresFromSchema(GetBase(), StoreVal, 1024);
}

TResampler::TResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal), UpdatedP(false) {
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr OutStoreNm = ParamVal->GetObjStr("outStore");
    QmAssertR(InStoreNm != OutStoreNm, "Input and output store in resampler should not be the same!");
    // get pointers to stores
    InStore = Base->GetStoreByStoreNm(InStoreNm);
    // get time field id
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
    TimeFieldId = InStore->GetFieldId(TimeFieldNm);
    // get ids of interpolated fields 
    PJsonVal FieldArrVal = ParamVal->GetObjKey("fields");
    for(int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
        PJsonVal FieldVal = FieldArrVal->GetArrVal(FieldN);        
        TStr FieldNm = FieldVal->GetObjStr("name");
        InFieldIdV.Add(InStore->GetFieldId(FieldNm));
        TStr InterpolatorType = FieldVal->GetObjStr("interpolator");
        InterpolatorV.Add(TSignalProc::TInterpolator::New(InterpolatorType));
    }    
    // get output store
    const bool CreateStoreP = ParamVal->GetObjBool("createStore", false);
    if (CreateStoreP) { CreateStore(OutStoreNm); }
    OutStore = Base->GetStoreByStoreNm(OutStoreNm);   
    // initialize time parameters
    if (ParamVal->IsObjKey("start")) {
        TStr StartTmStr = ParamVal->GetObjStr("start");
        TTm StartTm = TTm::GetTmFromWebLogDateTimeStr(StartTmStr, '-', ':', '.', 'T');
        InterpPointMSecs = TTm::GetMSecsFromTm(StartTm);
    }
    IntervalMSecs = TJsonVal::GetMSecsFromJsonVal(ParamVal->GetObjKey("interval"));
}

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TResampler(Base, ParamVal);
}

void TResampler::LoadState(TSIn& SIn) {
    InterpolatorV.Load(SIn);
    InterpPointMSecs.Load(SIn);
    UpdatedP.Load(SIn);
}

void TResampler::SaveState(TSOut& SOut) const {
    InterpolatorV.Save(SOut);
    InterpPointMSecs.Save(SOut);
    UpdatedP.Save(SOut);
}

PJsonVal TResampler::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();    
    return Val;
}


///////////////////////////////
// Dense Feature Extractor Stream Aggregate (extracts TFltV from records)
void TFtrExtAggr::OnAddRec(const TRec& Rec) {
    FtrSpace->GetFullV(Rec, Vec);
}

void TFtrExtAggr::OnTime(const uint64& TmMsec) { }

void TFtrExtAggr::OnStep() { }

TFtrExtAggr::TFtrExtAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace) :
    TStreamAggr(Base, AggrNm) {

    FtrSpace = _FtrSpace;
}

PStreamAggr TFtrExtAggr::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace) {
    return new TFtrExtAggr(Base, AggrNm, _FtrSpace);
}

void TFtrExtAggr::GetVal(const TInt& ElN, TFlt& Val) const {
    QmAssertR(Vec.Len() > ElN, "TFtrExtAggr : GetFlt : index out of bounds");
    Val = Vec[ElN];
}

void TFtrExtAggr::Save(TSOut& SOut) const {
    throw TQmExcept::New("FTrExtAggr::Save not implemented:" + GetAggrNm());
}

void TFtrExtAggr::LoadState(TSIn& SIn) {
    Vec.Load(SIn);
}

void TFtrExtAggr::SaveState(TSOut& SOut) const {
    Vec.Save(SOut);
}

PJsonVal TFtrExtAggr::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewArr(Vec);
    return Val;
}

///////////////////////////////
/// Histogram stream aggregate
void TOnlineHistogram::OnStep() {
    if (BufferedP) {
        TFltV UpdateV;
        InAggrValBuffer->GetInValV(UpdateV);
        for (int ElN = 0; ElN < UpdateV.Len(); ElN++) {
            Model.Increment(UpdateV[ElN]);
        }
        TFltV ForgetV;
        InAggrValBuffer->GetOutValV(ForgetV);
        for (int ElN = 0; ElN < ForgetV.Len(); ElN++) {
            Model.Decrement(ForgetV[ElN]);
        }
    } else {
        Model.Increment(InAggrVal->GetFlt());
    }
}

TOnlineHistogram::TOnlineHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), Model(ParamVal) {

    // Get input aggregate aggregate name
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TOnlineHistogram] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // Cast the input as a time series stream aggregate
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggr());
    // Cast the input as a time series stream aggregate as a time series buffer
    InAggrValBuffer = dynamic_cast<TStreamAggrOut::IFltTmIO*>(InAggr());
    // Check if at least one is OK
    QmAssertR(!InAggrVal.Empty() || !InAggrValBuffer.Empty(),
              "Stream aggregate does not implement IFltTm interface: " + InAggrNm);

    BufferedP = !InAggrValBuffer.Empty();
}

/// Load from stream
void TOnlineHistogram::LoadState(TSIn& SIn) {
    Model.Load(SIn);
}

/// Store state into stream
void TOnlineHistogram::SaveState(TSOut& SOut) const {
    Model.Save(SOut);
}

///////////////////////////////
/// TDigest stream aggregate
TTDigest::TTDigest(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Model(ParamVal) {
    // Get input aggregate aggregate name
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TDigest] Stream aggregate does not exist: " + InAggrNm);
    InAggr = Base->GetStreamAggr(InAggrNm);
    // Cast the input as a time series stream aggregate    
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(InAggr());
    QmAssertR(!InAggrVal.Empty(), "[TTDigest] Stream aggregate does not implement IFltTm interface: " + InAggrNm);
    // prase model parameters
    ParamVal->GetObjFltV("quantiles", QuantilesVals);
}

PStreamAggr TTDigest::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTDigest(Base, ParamVal);
}

void TTDigest::OnStep() {
    TFlt Val = InAggrVal->GetFlt();
    if (InAggr->IsInit()) {
        Model.Update(Val);
    }
}

void TTDigest::Add(const TFlt& Val) {
    if (InAggr->IsInit()) {
        Model.Update(Val);
    }
}

PJsonVal TTDigest::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    //Val->AddToObj("Centroids", Model.GetQuantile(0.9));
    return Val;
}

/// Load from stream
void TTDigest::LoadState(TSIn& SIn) {
    Model.LoadState(SIn);
}

/// Store state into stream
void TTDigest::SaveState(TSOut& SOut) const {
    Model.SaveState(SOut);
}

///////////////////////////////
/// Chi square stream aggregate
void TChiSquare::OnStep() {
    TFltV ValVX; InAggrValX->GetValV(ValVX);
    TFltV ValVY; InAggrValY->GetValV(ValVY);
    if (InAggrX->IsInit() && InAggrY->IsInit()) {
        ChiSquare.Update(ValVX, ValVY);
    }
}

TChiSquare::TChiSquare(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), ChiSquare(ParamVal) {
  
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrValX = Cast<TStreamAggrOut::IFltVec>(InAggrX);
    
    TStr InAggrNmY = ParamVal->GetObjStr("inAggrY");
    QmAssertR(Base->IsStreamAggr(InAggrNmY), "[TChiSquare] Stream aggregate does not exist: " + InAggrNmY);
    InAggrY = Base->GetStreamAggr(InAggrNmY);
    InAggrValY = dynamic_cast<TStreamAggrOut::IFltVec*>(InAggrY());
    QmAssertR(!InAggrValY.Empty(), "Stream aggregate does not implement IFltVec interface: " + InAggrNmY);
}

PStreamAggr TChiSquare::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TChiSquare(Base, ParamVal);
}

PJsonVal TChiSquare::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("P", ChiSquare.GetP());
    Val->AddToObj("Chi2", ChiSquare.GetChi2());
    return Val;
}

/// Load from stream
void TChiSquare::LoadState(TSIn& SIn) {
    ChiSquare.LoadState(SIn);
}

/// Store state into stream
void TChiSquare::SaveState(TSOut& SOut) const {
    ChiSquare.SaveState(SOut);
}

/////////////////////////////////////////////////////////////////////////////////////////

/// Constructor, reserves appropriate internal storage
TSlottedHistogram::TSlottedHistogram(const uint64 _Period, const uint64 _Slot, const int _Bins) {
    PeriodLen = _Period;
    SlotGran = _Slot;
    Bins = _Bins;
    uint64 Slots = PeriodLen / SlotGran;
    Dat.Gen((int)Slots);
    for (int i = 0; i < Dat.Len(); i++) {
        Dat[i] = TSignalProc::TOnlineHistogram(0, Bins, Bins, false, false);
    }
}

void TSlottedHistogram::Reset() {
    for (int HistN = 0; HistN < Dat.Len(); HistN++) {
        Dat[HistN].Reset();
    }   
}

/// Load stream aggregate state from stream
void TSlottedHistogram::LoadState(TSIn& SIn) {
    PeriodLen.Load(SIn);
    SlotGran.Load(SIn);
    Bins.Load(SIn);
    Dat.Load(SIn);
}

/// Save state of stream aggregate to stream
void TSlottedHistogram::SaveState(TSOut& SOut) const {
    PeriodLen.Save(SOut);
    SlotGran.Save(SOut);
    Bins.Save(SOut);
    Dat.Save(SOut);
}

/// Add new data to statistics
void TSlottedHistogram::Add(const uint64& Ts, const TInt& Val) {
    int Idx = GetIdx(Ts);
    Dat[Idx].Increment(Val);
}

/// Remove data from statistics
void TSlottedHistogram::Remove(const uint64& Ts, const TInt& Val) {
    int Idx = GetIdx(Ts);
    Dat[Idx].Decrement(Val);
}

/// Provide statistics
void TSlottedHistogram::GetStats(const uint64 TsMin, const uint64 TsMax, TFltV& Dest) {
    EAssertR(TsMax > TsMin, "Invalid period query in TSlottedHistogram. TsMax <= TsMin");
    EAssertR(TsMax - PeriodLen < TsMin, "Invalid period query in TSlottedHistogram. TsMax - period >= TsMin");
    Dest.Clr();
    uint64 TsMin2 = (TsMin / SlotGran) * SlotGran;
    uint64 TsMax2 = (TsMax / SlotGran) * SlotGran;
    for (uint64 i = TsMin2; i <= TsMax2; i += SlotGran) {
        int Idx = GetIdx(i);
        TFltV Tmp;
        Dat[Idx].GetCountV(Tmp);
        if (Dest.Len() < Tmp.Len()) {
            Dest.Gen(Tmp.Len());
        }
        for (int j = 0; j < Tmp.Len(); j++) {
            Dest[j] += Tmp[j];
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
// TOnlineSlottedHistogram
void TOnlineSlottedHistogram::OnStep() {
    if (BufferedP) {
        LastTm = InAggrValBuffer->GetInTmMSecs();
        Model->Add(InAggrValBuffer->GetInTmMSecs(), (int)InAggrValBuffer->GetInVal());
        TFltV ForgetV;
        TUInt64V ForgetTmV;
        InAggrValBuffer->GetOutValV(ForgetV);
        InAggrValBuffer->GetOutTmMSecsV(ForgetTmV);
        for (int ElN = 0; ElN < ForgetV.Len(); ElN++) {
            Model->Remove(ForgetTmV[ElN], (int)ForgetV[ElN]);
        }
    } else {
        LastTm = InAggrVal->GetTmMSecs();
        Model->Add(InAggrVal->GetTmMSecs(), (int)InAggrVal->GetFlt());
    }
}

/// JSON constructor
TOnlineSlottedHistogram::TOnlineSlottedHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : 
        TStreamAggr(Base, ParamVal) {

    // Cast the input as a stream aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    // Cast the input as a time series stream aggregate
    InAggrVal = Cast<TStreamAggrOut::IFltTm>(InAggr, false);
    // Cast the input as a time series stream aggregate as a time series buffer
    InAggrValBuffer = Cast<TStreamAggrOut::IFltTmIO>(InAggr, false);

    // Check if at least one is OK
    QmAssertR(!InAggrVal.Empty() || !InAggrValBuffer.Empty(),
        "Stream aggregate does not implement IFltTm interface: " + InAggr->GetAggrNm());
    // check if we have buffered input
    BufferedP = !InAggrValBuffer.Empty();

    // assert we have rest of parameters
    EAssert(ParamVal->IsObj());
    EAssert(ParamVal->IsObjKey("period"));
    EAssert(ParamVal->IsObjKey("window"));
    EAssert(ParamVal->IsObjKey("bins"));
    EAssert(ParamVal->IsObjKey("granularity"));

    // parse parameters
    uint64 PeriodLen = ParamVal->GetObjUInt64("period");
    WndLen = ParamVal->GetObjUInt64("window");
    TInt Bins = ParamVal->GetObjInt("bins");
    uint64 SlotGran = ParamVal->GetObjUInt64("granularity");
    LastTm = 0;
    Model = TSlottedHistogram::New(PeriodLen, SlotGran, Bins);
}

/// serilization to JSon
PJsonVal TOnlineSlottedHistogram::SaveJson(const int& Limit) const {
    TFltV ValV;
    GetValV(ValV);
    
    PJsonVal Counts = TJsonVal::NewArr();
    for (int i = 0; i < ValV.Len(); i++) {
        Counts->AddToArr(ValV[i]);
    }

    PJsonVal Res = TJsonVal::NewObj();
    Res->AddToObj("counts", Counts);
    return Res;
}

/// returns frequencies in a given bin
void TOnlineSlottedHistogram::GetVal(const TInt& ElN, TFlt& Val) const {
    TFltV Tmp;
    GetValV(Tmp);
    Val = Tmp[ElN];
}

/// Load saved state
void TOnlineSlottedHistogram::LoadState(TSIn& SIn) {
    LastTm.Load(SIn);
    WndLen.Load(SIn);
    Model->LoadState(SIn);
};

/// Persist state
void TOnlineSlottedHistogram::SaveState(TSOut& SOut) const {
    LastTm.Save(SOut);
    WndLen.Save(SOut);
    Model->SaveState(SOut);
};

///////////////////////////////
/// Vector-difference stream aggregate

/// constructor
TVecDiff::TVecDiff(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // parse out input aggregate
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrValX = Cast<TStreamAggrOut::IFltVec>(InAggrX);
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrValY = Cast<TStreamAggrOut::IFltVec>(InAggrY);
}

/// factory method
PStreamAggr TVecDiff::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TVecDiff(Base, ParamVal);
}

void TVecDiff::OnStep() {
    // Get input vectors
    TFltV ValV1, ValV2;
    InAggrValX->GetValV(ValV1);
    InAggrValY->GetValV(ValV2);
    // check sizes are ok
    QmAssertR(ValV1.Len() == ValV2.Len(), "[TVecDiff]: Dimensions of input vectors do not match");
    // reserve place for the diff
    const int Vals = ValV1.Len();
    ValV.Gen(Vals, 0);
    // get the diff
    for (int i = 0; i < ValV1.Len(); i++) {
        ValV.Add(ValV1[i] - ValV2[i]);
    }
}

/// serialization to JSon
PJsonVal TVecDiff::SaveJson(const int& Limit) const {
    PJsonVal Res = TJsonVal::NewObj();
    Res->AddToObj("name", GetAggrNm());
    Res->AddToObj("aggr1", InAggrX->SaveJson(Limit));
    Res->AddToObj("aggr2", InAggrY->SaveJson(Limit));

    PJsonVal CountsArr = TJsonVal::NewArr();
    for (int ElN = 0; ElN < ValV.Len(); ElN++) {
        CountsArr->AddToArr(ValV[ElN]);
    }
    Res->AddToObj("diff", CountsArr);
    return Res;
}

///////////////////////////////
// Simple linear regression stream aggregate.

void TSimpleLinReg::OnStep() {
    TFltV X; InAggrValX->GetValV(X);
    TFltV Y; InAggrValY->GetValV(Y);
    double A;
    double B;   

    TSpecFunc::LinearFit(X, Y, A, B);   

    Result->AddToObj("intercept", A);
    Result->AddToObj("slope", B);

    if (Quantiles.Len() > 0) {
        int N = X.Len();
        TFltV Projected(N);
        TFltV Bands(Quantiles.Len());       
        if (N > 0) { // empty vectors -> all bands are set to 0
            TLinAlg::LinComb(1.0, Y, -B, X, Projected);
            Projected.Sort(true);
            for (int QuantN = 0; QuantN < Quantiles.Len(); QuantN++) {
                EAssertR((Quantiles[QuantN] >= 0) && (Quantiles[QuantN] <= 1), "Each quantile should be a number between 0 and 1");
                int Index = (int)floor(Quantiles[QuantN] * N);
                Index = MIN(Index, N - 1);
                int Index2 = MAX(Index - 1, 0);
                Bands[QuantN] = 0.5*(Projected[Index] + Projected[Index2]);
            }
        }
        Result->AddToObj("bands", TJsonVal::NewArr(Bands));
    }
}

TSimpleLinReg::TSimpleLinReg(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // filter
    if (ParamVal->IsObjKey("filter")) {
        PJsonVal Val = ParamVal->GetObjKey("filter");
        Filter = TStreamAggrOnAddFilter::New(Val->GetObjStr("type"), Val);
    } else {
        Filter = TStreamAggrOnAddFilter::New();
    }
    // parse out input aggregate
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrValX = Cast<TStreamAggrOut::IFltVec>(InAggrX);
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrValY = Cast<TStreamAggrOut::IFltVec>(InAggrY);

    if (ParamVal->IsObjKey("quantiles")) {
        ParamVal->GetObjFltV("quantiles", Quantiles);
    }
    Reset();
}

void TSimpleLinReg::Reset() {
    Result = TJsonVal::NewObj();
    if (Quantiles.Len() > 0) {
        Result->AddToObj("quantiles", TJsonVal::NewArr(Quantiles));
    }
}

} // TStreamAggrs namespace
} // TQm namespace

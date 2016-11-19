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
}

void TTimeSeriesTick::OnStep() {
    throw TExcept::New("[TTimeSeriesTick] OnStep should have not been executed.");
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
    const int TickValFieldId = Store->GetFieldId(TickValFieldNm);
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
// Numberic circular buffer
TWinBufFltV::TWinBufFltV(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TWinBufMem<TFlt>(Base, ParamVal) {
    InAggrVal = Cast<TStreamAggrOut::IFlt>(GetInAggr());
}

PStreamAggr TWinBufFltV::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TWinBufFltV(Base, ParamVal);
}

// serialization to JSon
PJsonVal TWinBufFltV::SaveJson(const int& Limit) const {
    TFltV FltV; GetValV(FltV);
    return TJsonVal::NewArr(FltV);
}

///////////////////////////////
/// Time series window buffer with dense vector per record.
TFlt TWinBufFlt::GetRecVal(const uint64& RecId) const {
    return ValReader.GetFlt(TRec(Store, RecId));
}

TWinBufFlt::TWinBufFlt(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TWinBuf<TFlt>(Base, ParamVal) {
    // parse out input and output fields
    TStr ValFieldNm = ParamVal->GetObjStr("value");
    int ValFieldId = Store->GetFieldId(ValFieldNm);
    ValReader = TFieldReader(Store->GetStoreId(), ValFieldId, Store->GetFieldDesc(ValFieldId));
    // make sure parameters make sense
    QmAssertR(ValReader.IsFlt(), "[Window buffer] field " + ValFieldNm + " cannot be casted to 'double'");
}

PJsonVal TWinBufFlt::SaveJson(const int& Limit) const {
    PJsonVal JsonVal = TJsonVal::NewObj();
    const int Vals = GetVals();
    if (GetVals() > 0) {
        // read last value
        TFlt Flt; GetVal(Vals - 1, Flt);
        const uint64 TmMSecs = GetTm(Vals - 1);
        // save last value
        JsonVal->AddToObj("Val", Flt);
        JsonVal->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T"));
    }
    return JsonVal;
}

///////////////////////////////
/// Time series window buffer with sparse vector per record.
TIntFltKdV TWinBufFtrSpVec::GetRecVal(const uint64& RecId) const {
    TIntFltKdV Result;
    FtrSpace->GetSpV(TRec(Store, RecId), Result);
    return Result;
}

void TWinBufFtrSpVec::RecUpdate(const uint64& RecId) const {
    FtrSpace->Update(TRec(Store, RecId));
}

TWinBufFtrSpVec::TWinBufFtrSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TWinBuf<TIntFltKdV>(Base, ParamVal) {

    PJsonVal FtrSpaceParam = ParamVal->GetObjKey("featureSpace");
    FtrSpace = TFtrSpace::New(Store->GetBase(), FtrSpaceParam);
}

void TWinBufFtrSpVec::LoadState(TSIn& SIn) {
    TWinBuf<TIntFltKdV>::LoadState(SIn);
    FtrSpace->Load(Store->GetBase(), SIn);
}

void TWinBufFtrSpVec::SaveState(TSOut& SOut) const {
    TWinBuf<TIntFltKdV>::SaveState(SOut);
    FtrSpace->Save(SOut);
}

///////////////////////////////
// Exponential Moving Average.
void TEma::OnStep() {
    if (InAggr->IsInit()) {
        Ema.Update(InAggrFlt->GetFlt(), InAggrTm->GetTmMSecs());
    }
}

TEma::TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), Ema(ParamVal) {

    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr);
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
// Exponential Moving Average for sparse vectors
void TEmaSpVec::OnStep() {
    if (InAggr->IsInit()) {
        TIntFltKdV Vals; InAggrSparseVec->GetSparseVec(Vals);
        Ema.Update(Vals, InAggrTm->GetTmMSecs());
    }
}
TEmaSpVec::TEmaSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal), Ema(ParamVal) {
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrSparseVec = Cast<TStreamAggrOut::ISparseVec>(InAggr);
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
// Threshold aggregate
void TThresholdAggr::OnStep() {
    if (InAggr->IsInit()) {
        IsAboveP = InAggrFlt->GetFlt() > Threshold ? 1.0 : 0.0;
        TmMSecs = InAggrTm->GetTmMSecs();
    }
}

TThresholdAggr::TThresholdAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // parse input aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr);
    // parse model parameters
    QmAssertR(ParamVal->IsObjKey("threshold"), "Threshold is not defined!");
    Threshold = ParamVal->GetObjNum("threshold");
    // initialize with reset
    Reset();
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
// Moving Covariance
void TCov::OnStep() {
    if (InAggrX->IsInit() && InAggrY->IsInit()) {
        // new series
        TFltV InValVX; InAggrFltIOX->GetInValV(InValVX);
        TFltV InValVY; InAggrFltIOY->GetInValV(InValVY);
        TUInt64V InTmMSecsV; InAggrTmIOX->GetInTmMSecsV(InTmMSecsV);
        // delete series
        TFltV OutValVX; InAggrFltIOX->GetOutValV(OutValVX);
        TFltV OutValVY; InAggrFltIOY->GetOutValV(OutValVY);
        TUInt64V OutTmMSecsV; InAggrTmIOX->GetOutTmMSecsV(OutTmMSecsV);
        Cov.Update(InValVX, InValVY, InTmMSecsV, OutValVX, OutValVY, OutTmMSecsV);
    }
}

TCov::TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrTmIOX = Cast<TStreamAggrOut::ITmIO>(InAggrX);
    InAggrFltIOX = Cast<TStreamAggrOut::IFltIO>(InAggrX);
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrTmIOY = Cast<TStreamAggrOut::ITmIO>(InAggrY);
    InAggrFltIOY = Cast<TStreamAggrOut::IFltIO>(InAggrY);
}

void TCov::LoadState(TSIn& SIn) {
    Cov.Load(SIn);
}

void TCov::SaveState(TSOut& SOut) const {
    Cov.Save(SOut);
}

PStreamAggr TCov::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCov(Base, ParamVal);
}

void TCov::GetInAggrNmV(TStrV& InAggrNmV) const {
    InAggrNmV.Add(InAggrX->GetAggrNm());
    InAggrNmV.Add(InAggrY->GetAggrNm());
}

PJsonVal TCov::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Cov.GetCov());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(Cov.GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Moving Correlation
void TCorr::OnStep() {
    const double Cov = InAggrFltCov->GetFlt();
    const double Var1 = InAggrFltVarX->GetFlt();
    const double Var2 = InAggrFltVarY->GetFlt();
    if ((Var1 == 0.0) || (Var2 == 0.0)) {
        Corr = 1;
    } else {
        Corr = Cov / ( sqrt(Var1) * sqrt(Var2) );
    }
    TmMSecs = InAggrTmCov->GetTmMSecs();
}

TCorr::TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // covariance cast
    InAggrCov = ParseAggr(ParamVal, "inAggrCov");
    InAggrTmCov = Cast<TStreamAggrOut::ITm>(InAggrCov);
    InAggrFltCov = Cast<TStreamAggrOut::IFlt>(InAggrCov);
    // X variance cast
    InAggrVarX = ParseAggr(ParamVal, "inAggrVarX");
    InAggrFltVarX = Cast<TStreamAggrOut::IFlt>(InAggrVarX);
    // Y variance cast
    InAggrVarY = ParseAggr(ParamVal, "inAggrVarY");
    InAggrFltVarY = Cast<TStreamAggrOut::IFlt>(InAggrVarY);
}

PStreamAggr TCorr::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCorr(Base, ParamVal);
}

void TCorr::LoadState(TSIn& SIn) {
    Corr.Load(SIn);
    TmMSecs.Load(SIn);
}

void TCorr::SaveState(TSOut& SOut) const {
    Corr.Save(SOut);
    TmMSecs.Save(SOut);
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

///////////////////////////////
/// Merger

TMerger::TMergerFieldMap::TMergerFieldMap(const int& _InFldId, const TJoinSeq& _InFldJoinSeq,
    const int& _TmFldId, const int& _OutFldId): InFldId(_InFldId), InFldJoinSeq(_InFldJoinSeq),
    TmFldId(_TmFldId), OutFldId(_OutFldId) {}

TMerger::TMergerFieldMap::TMergerFieldMap(TSIn& SIn) { Load(SIn); }

void TMerger::TMergerFieldMap::Save(TSOut& SOut) const {
    InFldId.Save(SOut);
    InFldJoinSeq.Save(SOut);
    TmFldId.Save(SOut);
    OutFldId.Save(SOut);
}

void TMerger::TMergerFieldMap::Load(TSIn& SIn) {
    InFldId.Load(SIn);
    InFldJoinSeq.Load(SIn);
    TmFldId.Load(SIn);
    OutFldId.Load(SIn);
}

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
                JoinSeq = TJoinSeq(Base, Base->GetStoreByStoreNm(inStoreNm)->GetStoreId(),
                                   SourceVal->GetObjKey("join"));
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
        QmAssertR(StartStore->IsFieldNm(TimeFieldNm),
                  "Field " + InFieldNm + " does not exist in store: " + StartStore->GetStoreNm() + "!");
        // check if the input field exists
        QmAssertR(EndStore->IsFieldNm(InFieldNm),
                  "Field " + InFieldNm + " does not exist in store: " + EndStore->GetStoreNm() + "!");
        // check if the output field exists in the out store
        QmAssertR(OutStore->IsFieldNm(OutFieldNm),
                  "Field " + OutFieldNm + " does not exist in the output store!");
        // check if the output field is of correct type
        QmAssertR(OutStore->GetFieldDesc(OutStore->GetFieldId(OutFieldNm)).GetFieldType() == TFieldType::oftFlt,
                  "Field " + OutFieldNm + " is of incorrect type!");
    }
    /// we are almost done
    InitMerger(Base, OutStoreNm, TimeFieldNm, CreateStoreP, Past, InterpNmV);
}

PStreamAggr TMerger::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TMerger(Base, ParamVal);
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
        const int& FieldMapIdx = FieldMapIdxSet.GetKey(KeyId);
        OnAddRec(Rec, FieldMapIdx);
    }
}

void TMerger::OnTime(const uint64& TmMsec) {
    QmAssertR(false, "Merger::OnTime(const uint64& TmMsec) not supported.");
}

void TMerger::OnStep() {
    QmAssertR(false, "Merger::OnStep() should not be executed.");
}

void TMerger::OnAddRec(const TQm::TRec& Rec, const int& FieldMapIdx) {
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
// Resampler of univariate time series
TUniVarResampler::TUniVarResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal),
        OutAggr(),
        UpdatedP(false) {
    // parse the input aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr, false);
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr, false);

    // parse the interpolator
    Interpolator = TSignalProc::TInterpolator::New(ParamVal->GetObjStr("interpolator"));

    // initialize time parameters
    if (ParamVal->IsObjKey("start")) {
        TStr StartTmStr = ParamVal->GetObjStr("start");
        TTm StartTm = TTm::GetTmFromWebLogDateTimeStr(StartTmStr, '-', ':', '.', 'T');
        InterpPointMSecs = TTm::GetMSecsFromTm(StartTm);
    }
    IntervalMSecs = TJsonVal::GetMSecsFromJsonVal(ParamVal->GetObjKey("interval"));
}

PStreamAggr TUniVarResampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TUniVarResampler(Base, ParamVal);
}

PJsonVal TUniVarResampler::GetParam() const {
    PJsonVal ParamVal = TJsonVal::NewObj();

    if (!InAggr.Empty()) {
        ParamVal->AddToObj("inAggr", InAggr->GetAggrNm());
    } else {
        ParamVal->AddToObj("inAggr", TJsonVal::NewNull());
    }

    if (!OutAggr.Empty()) {
        ParamVal->AddToObj("outAggr", OutAggr->GetAggrNm());
    } else {
        ParamVal->AddToObj("outAggr", TJsonVal::NewNull());
    }


    return ParamVal;
}

void TUniVarResampler::SetParam(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("inAggr")) {
        const TStr AggrNm = ParamVal->GetObjStr("inAggr");
        EAssert(GetBase()->IsStreamAggr(AggrNm));
        InAggr = GetBase()->GetStreamAggr(AggrNm);
    }

    if (ParamVal->IsObjKey("outAggr")) {
        const TStr AggrNm = ParamVal->GetObjStr("outAggr");
        EAssert(GetBase()->IsStreamAggr(AggrNm));
        OutAggr = GetBase()->GetStreamAggr(AggrNm);
    }
}

void TUniVarResampler::LoadState(TSIn& SIn) {
    Interpolator = TSignalProc::TInterpolator::Load(SIn);
    InterpPointMSecs.Load(SIn);
    InterpPointVal.Load(SIn);
    UpdatedP.Load(SIn);
}

void TUniVarResampler::SaveState(TSOut& SOut) const {
    Interpolator->Save(SOut);
    InterpPointMSecs.Save(SOut);
    InterpPointVal.Save(SOut);
    UpdatedP.Save(SOut);
}

PJsonVal TUniVarResampler::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();      // TODO
    return Val;
}

void TUniVarResampler::OnStep() {
    // get record time
    const uint64 NewTmMSecs = InAggrTm->GetTmMSecs();
    const double NewVal = InAggrFlt->GetFlt();

    // update the interpolator
    Interpolator->AddPoint(NewVal, NewTmMSecs);

    // only do this first time when interpolation time not defined
    if (InterpPointMSecs == 0) {
        InterpPointMSecs = NewTmMSecs;
        RefreshInterpolators(NewTmMSecs);
    }

    // warning if Rec time > InterpPointMSecs, this is the first update and we cannot interpolate
    if (!UpdatedP && !CanInterpolate()) {
        if (InterpPointMSecs < NewTmMSecs) {
            InfoLog("Warning: resampler: start interpolation time is lower than the first record time, cannot interpolate. If future timestamps will keep increasing it might be possible that the resampler will be stuck and unable to interpolate.");
        }
    }
    UpdatedP = true;

    // insert new records while the interpolators allow us
    while (InterpPointMSecs <= NewTmMSecs && CanInterpolate()) {
        InterpPointVal = Interpolator->Interpolate(InterpPointMSecs);
        OutAggr->OnStep();

        InterpPointMSecs += IntervalMSecs;
    }

    RefreshInterpolators(NewTmMSecs);
}

void TUniVarResampler::RefreshInterpolators(const uint64& Tm) {
    // update time in the interpolators
    Interpolator->SetNextInterpTm(Tm);
}

bool TUniVarResampler::CanInterpolate() {
    RefreshInterpolators(InterpPointMSecs);
    return Interpolator->CanInterpolate(InterpPointMSecs);
}

///////////////////////////////
// Dense Feature Extractor Stream Aggregate (extracts TFltV from records)
void TFtrExtAggr::OnAddRec(const TRec& Rec) {
    // extract vectors
    if (FullP) { FtrSpace->GetFullV(Rec, FullVec); }
    if (SparseP) { FtrSpace->GetSpV(Rec, SpVec); }
    // update if needed
    if (UpdateP) {
        // show record to feature space
        FtrSpace->Update(Rec);
        // count till we are initalized
        if (InitCount > 0) { InitCount--; }
    }
}

TFtrExtAggr::TFtrExtAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // pares parameters
    InitCount = ParamVal->GetObjInt("initCount", 0);
    UpdateP = ParamVal->GetObjBool("update", true);
    FullP = ParamVal->GetObjBool("full", true);
    SparseP = ParamVal->GetObjBool("sparse", true);
    // define feature space
    ParamVal->AssertObjKey("featureSpace", __FUNCTION__);
    FtrSpace = TFtrSpace::New(Base, ParamVal->GetObjKey("featureSpace"));
}

PStreamAggr TFtrExtAggr::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TFtrExtAggr(Base, ParamVal);
}

void TFtrExtAggr::LoadState(TSIn& SIn) {
    // load feature space
    FtrSpace = TFtrSpace::Load(Base, SIn);
    // load init counter
    InitCount = TInt(SIn);
    // load vectors
    FullVec.Load(SIn);
    SpVec.Load(SIn);
}

void TFtrExtAggr::SaveState(TSOut& SOut) const {
    // save feature space (part of state)
    FtrSpace->Save(SOut);
    // save init counter
    InitCount.Save(SOut);
    // save last extracted vectors
    FullVec.Save(SOut);
    SpVec.Save(SOut);
}

PJsonVal TFtrExtAggr::GetParam() const {
    PJsonVal ParamsVal = TJsonVal::NewObj();
    ParamsVal->AddToObj("initCount", InitCount);
    ParamsVal->AddToObj("update", UpdateP);
    ParamsVal->AddToObj("full", FullP);
    ParamsVal->AddToObj("sparse", SparseP);
    return ParamsVal;
}

void TFtrExtAggr::SetParam(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("initCount")) { InitCount = ParamVal->GetObjInt("initCount"); }
    if (ParamVal->IsObjKey("update")) { UpdateP = ParamVal->GetObjBool("update"); }
    if (ParamVal->IsObjKey("full")) { FullP = ParamVal->GetObjBool("full"); }
    if (ParamVal->IsObjKey("sparse")) { SparseP = ParamVal->GetObjBool("sparse"); }
}

void TFtrExtAggr::Reset() {
    FtrSpace->Clr();
}

PJsonVal TFtrExtAggr::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    if (FullP) { Val->AddToObj("full", TJsonVal::NewArr(FullVec)); }
    if (SparseP) { Val->AddToObj("sparse", TJsonVal::NewArr(SpVec)); }
    return Val;
}

///////////////////////////////
/// Nearest Neighbor for Anomaly Detection stream aggregate.
void TNNAnomalyAggr::OnStep() {
    // make sure input aggregators are initialized
    if (InAggrTm->IsInit() && InAggrSparseVec->IsInit()) {
        // get last time stamp and last sparse vector from the input aggregators
        LastTimeStamp = InAggrValTm->GetTmMSecs();
        TIntFltKdV ValV; InAggrValSparseVec->GetSparseVec(ValV);
        // predict the severity of the alarm
        LastSeverity = Model.Predict(ValV);
        // save the explanation for the alarm
        if (LastSeverity > 0) {
            Explanation = Model.Explain(ValV);
        } else {
            Explanation = TJsonVal::NewObj();
        }
        // update the model with the current feature vector
        Model.PartialFit(ValV, LastTimeStamp);
    }
}

TNNAnomalyAggr::TNNAnomalyAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // parse parameters
    SetParam(ParamVal);
    // we start with empty explanation
    Explanation = TJsonVal::NewObj();
}

PJsonVal TNNAnomalyAggr::GetParam() const {
    PJsonVal ParamVal = TJsonVal::NewObj();

    if (!InAggrTm.Empty()) {
        ParamVal->AddToObj("inAggrTm", InAggrTm->GetAggrNm());
    } else {
        ParamVal->AddToObj("inAggrTm", TJsonVal::NewNull());
    }

    if (!InAggrSparseVec.Empty()) {
        ParamVal->AddToObj("inAggrSpV", InAggrSparseVec->GetAggrNm());
    } else {
        ParamVal->AddToObj("inAggrSpV", TJsonVal::NewNull());
    }

    ParamVal->AddToObj("rate", TJsonVal::NewArr(Model.GetRateV()));
    ParamVal->AddToObj("windowSize", Model.GetWindowSize());

    return ParamVal;
}

void TNNAnomalyAggr::SetParam(const PJsonVal& ParamVal) {
    //parse time aggregator parameters
    if (ParamVal->IsObjKey("inAggrTm")) {
        const TStr AggrNm = ParamVal->GetObjStr("inAggrTm");
        EAssert(GetBase()->IsStreamAggr(AggrNm));
        InAggrTm = GetBase()->GetStreamAggr(AggrNm);
        InAggrValTm = Cast<TStreamAggrOut::ITm>(InAggrTm);
    } else {
        throw TQmExcept::New("The definition of the NN Anomaly Aggregator requires an input time aggregator (inAggrTm)");
    }
    //parse sparse vector aggregator parameters
    if (ParamVal->IsObjKey("inAggrSpV")) {
        const TStr AggrNm = ParamVal->GetObjStr("inAggrSpV");
        EAssert(GetBase()->IsStreamAggr(AggrNm));
        InAggrSparseVec = GetBase()->GetStreamAggr(AggrNm);
        InAggrValSparseVec = Cast<TStreamAggrOut::ISparseVec>(InAggrSparseVec);
    } else {
        throw TQmExcept::New("The definition of the NN Anomaly Aggregator requires an input sparse vector aggregator (inAggrSpV)");
    }

    // parse rate parameter(s)
    TFltV RateV;
    if (ParamVal->IsObjKey("rate")) {
        // check if we get single number or array of numbers
        if (ParamVal->GetObjKey("rate")->IsNum()) {
            // we have a number
            RateV.Add(ParamVal->GetObjNum("rate"));
        } else {
            // must be an array
            ParamVal->GetObjFltV("rate", RateV);
        }
    } else {
        throw TQmExcept::New("The definition of the NN Anomaly Aggregator requires an input rate number or vector (rate)");
    }
    // if empty, use 0.05
    if (RateV.Empty()) { RateV.Add(0.05); }
    // create model
    Model = TAnomalyDetection::TNearestNeighbor(RateV, ParamVal->GetObjInt("windowSize", 100));
}

/// Reset the aggregator
void TNNAnomalyAggr::Reset() {
    TFltV RateV = Model.GetRateV();
    TInt WinSize = Model.GetWindowSize();
    Model = TAnomalyDetection::TNearestNeighbor(RateV, WinSize);
    LastSeverity = 0;
    Explanation = TJsonVal::NewObj();
}

/// Load from stream
void TNNAnomalyAggr::LoadState(TSIn& SIn) {
    Model = TAnomalyDetection::TNearestNeighbor(SIn);
    LastTimeStamp.Load(SIn);
    LastSeverity.Load(SIn);
    Explanation = new TJsonVal(SIn);
}

/// Store state into stream
void TNNAnomalyAggr::SaveState(TSOut& SOut) const {
    Model.Save(SOut);
    LastTimeStamp.Save(SOut);
    LastSeverity.Save(SOut);
    Explanation->Save(SOut);
}

PJsonVal TNNAnomalyAggr::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("time", LastTimeStamp);
    Val->AddToObj("severity", LastSeverity);
    Val->AddToObj("explanation", Explanation);
    return Val;
}

///////////////////////////////
/// Histogram stream aggregate
void TOnlineHistogram::OnStep() {
    if (BufferedP) {
        TFltV UpdateV; InAggrFltIO->GetInValV(UpdateV);
        for (int ElN = 0; ElN < UpdateV.Len(); ElN++) {
            Model.Increment(UpdateV[ElN]);
        }
        TFltV ForgetV; InAggrFltIO->GetOutValV(ForgetV);
        for (int ElN = 0; ElN < ForgetV.Len(); ElN++) {
            Model.Decrement(ForgetV[ElN]);
        }
    } else {
        Model.Increment(InAggrFlt->GetFlt());
    }
}

TOnlineHistogram::TOnlineHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), Model(ParamVal) {

    /// parse input aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr, false);
    InAggrFltIO = Cast<TStreamAggrOut::IFltIO>(InAggr, false);
    /// Check if at least one cast is OK
    if (!InAggrFlt.Empty()) {
        // all cool
    } else if (!InAggrFltIO.Empty()) {
        // also cool
    } else {
        // not cool!
        throw TQmExcept::New("Stream aggregate does not implement IFlt or TFltIO interface: " + InAggr->GetAggrNm());
    }
    /// Remember if we have buffered input (in case of both, we assumbe buffered)
    BufferedP = !InAggrFltIO.Empty();
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
void TTDigest::OnStep() {
    TFlt Val = InAggrFlt->GetFlt();
    if (InAggr->IsInit()) {
        Model.Update(Val);
    }
}

void TTDigest::Add(const TFlt& Val) {
    if (InAggr->IsInit()) {
        Model.Update(Val);
    }
}

TTDigest::TTDigest(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal), Model(ParamVal) {

    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr);
    // prase model parameters
    ParamVal->GetObjFltV("quantiles", QuantileV);
}

PStreamAggr TTDigest::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTDigest(Base, ParamVal);
}

void TTDigest::LoadState(TSIn& SIn) {
    Model.LoadState(SIn);
}

void TTDigest::SaveState(TSOut& SOut) const {
    Model.SaveState(SOut);
}

void TTDigest::GetVal(const int& ElN, TFlt& Val) const {
    Val = Model.GetQuantile(QuantileV[ElN]);
}

void TTDigest::GetValV(TFltV& ValV) const {
    for (int ElN = 0; ElN < QuantileV.Len(); ElN++) {
        ValV.Add(Model.GetQuantile(QuantileV[ElN]));
    }
}

PJsonVal TTDigest::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    PJsonVal QuantilesVal = TJsonVal::NewArr();
    for (int ElN = 0; ElN < QuantileV.Len(); ElN++) {
        PJsonVal QuantileVal = TJsonVal::NewObj();
        QuantileVal->AddToObj("quantile", QuantileV[ElN]);
        QuantileVal->AddToObj("value", Model.GetQuantile(QuantileV[ElN]));
        QuantilesVal->AddToArr(QuantileVal);
    }
    Val->AddToObj("quantiles", QuantilesVal);
    return Val;
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
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrValY = Cast<TStreamAggrOut::IFltVec>(InAggrY);
}

PStreamAggr TChiSquare::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TChiSquare(Base, ParamVal);
}

void TChiSquare::LoadState(TSIn& SIn) {
    ChiSquare.LoadState(SIn);
}

void TChiSquare::SaveState(TSOut& SOut) const {
    ChiSquare.SaveState(SOut);
}

PJsonVal TChiSquare::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("P", ChiSquare.GetP());
    Val->AddToObj("Chi2", ChiSquare.GetChi2());
    return Val;
}

///////////////////////////////
// TOnlineSlottedHistogram
void TOnlineSlottedHistogram::OnStep() {
    if (BufferedP) {
        // add new values
        TFltV InValV; InAggrFltIO->GetInValV(InValV);
        TUInt64V InTmMSecsV; InAggrTmIO->GetInTmMSecsV(InTmMSecsV);
        for (int ElN = 0; ElN < InValV.Len(); ElN++) {
            Model.Add(InTmMSecsV[ElN], (int)InValV[ElN]);
        }
        // update time stamp
        if (!InTmMSecsV.Empty()) { LastTm = InTmMSecsV.Last(); }
        // remove old values
        TFltV OutValV; InAggrFltIO->GetOutValV(OutValV);
        TUInt64V OutTmMSecsV; InAggrTmIO->GetOutTmMSecsV(OutTmMSecsV);
        for (int ElN = 0; ElN < OutValV.Len(); ElN++) {
            Model.Remove(OutTmMSecsV[ElN], (int)OutValV[ElN]);
        }
    } else {
        // just add new values to the aggregate
        LastTm = InAggrTm->GetTmMSecs();
        Model.Add(InAggrTm->GetTmMSecs(), (int)InAggrFlt->GetFlt());
    }
    // get current stats from the model
    Model.GetStats(LastTm - WndLen, LastTm, ValV);
}

TOnlineSlottedHistogram::TOnlineSlottedHistogram(const TWPt<TBase>& Base, const PJsonVal& ParamVal) :
    TStreamAggr(Base, ParamVal) {

    // Cast the input as a stream aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr, false);
    InAggrFlt = Cast<TStreamAggrOut::IFlt>(InAggr, false);
    InAggrTmIO = Cast<TStreamAggrOut::ITmIO>(InAggr, false);
    InAggrFltIO = Cast<TStreamAggrOut::IFltIO>(InAggr, false);
    /// Check if at least one cast is OK
    if (!InAggrTm.Empty() && !InAggrFlt.Empty()) {
        // all cool
    } else if (!InAggrTmIO.Empty() && !InAggrFltIO.Empty()) {
        // also cool
    } else {
        // not cool!
        throw TQmExcept::New("Stream aggregate does not implement ITm and IFlt or TTmIO and TFltIO interface: " + InAggr->GetAggrNm());
    }
    /// Remember if we have buffered input (in case of both, we assumbe buffered)
    BufferedP = !InAggrTmIO.Empty();

    // parse model parameters
    const uint64 PeriodLen = ParamVal->GetObjUInt64("period");
    const int Bins = ParamVal->GetObjInt("bins");
    const uint64 SlotGran = ParamVal->GetObjUInt64("granularity");
    Model = TSignalProc::TSlottedHistogram(PeriodLen, SlotGran, Bins);
    // get window size
    WndLen = ParamVal->GetObjUInt64("window");
}

void TOnlineSlottedHistogram::LoadState(TSIn& SIn) {
    LastTm.Load(SIn);
    WndLen.Load(SIn);
    Model.LoadState(SIn);
};

void TOnlineSlottedHistogram::SaveState(TSOut& SOut) const {
    LastTm.Save(SOut);
    WndLen.Save(SOut);
    Model.SaveState(SOut);
};

PJsonVal TOnlineSlottedHistogram::SaveJson(const int& Limit) const {
    PJsonVal Counts = TJsonVal::NewArr();
    for (int i = 0; i < ValV.Len(); i++) {
        Counts->AddToArr(ValV[i]);
    }
    return TJsonVal::NewObj("counts", Counts);
}

///////////////////////////////
/// Vector difference stream aggregate
TVecDiff::TVecDiff(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // parse out input aggregate
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrValX = Cast<TStreamAggrOut::IFltVec>(InAggrX);
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrValY = Cast<TStreamAggrOut::IFltVec>(InAggrY);
}

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
    // Get input vectors
    TFltV X; InAggrValX->GetValV(X);
    TFltV Y; InAggrValY->GetValV(Y);
    // Fit linear model
    double A, B; TSpecFunc::LinearFit(X, Y, A, B);
    // Save model for export
    Result->AddToObj("intercept", A);
    Result->AddToObj("slope", B);
    // check if we should search for quantiles
    if (Quantiles.Len() > 0) {
        const int N = X.Len();
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
        Filter = TRecFilter::New(Base, Val->GetObjStr("type"), Val);
    } else {
        Filter = TRecFilter::New(Base);
    }
    // parse out input aggregate
    InAggrX = ParseAggr(ParamVal, "inAggrX");
    InAggrValX = Cast<TStreamAggrOut::IFltVec>(InAggrX);
    InAggrY = ParseAggr(ParamVal, "inAggrY");
    InAggrValY = Cast<TStreamAggrOut::IFltVec>(InAggrY);
    // parse parameters
    if (ParamVal->IsObjKey("quantiles")) {
        ParamVal->GetObjFltV("quantiles", Quantiles);
    }
    // initialize with reset
    Reset();
}

void TSimpleLinReg::Reset() {
    Result = TJsonVal::NewObj();
    if (Quantiles.Len() > 0) {
        Result->AddToObj("quantiles", TJsonVal::NewArr(Quantiles));
    }
}

void TRecFilterAggr::OnAddRec(const TRec& Rec) {
    // the record should pass all filter tests
    for (const PRecFilter& Filter : FilterV) {
        if (!Filter->Filter(Rec)) { return; }
    }
    Aggr->OnAddRec(Rec);
}


TRecFilterAggr::TRecFilterAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // single filter
    ParamVal->AssertObjKey("filters", __FUNCTION__);
    PJsonVal Arr = ParamVal->GetObjKey("filters");
    QmAssertR(Arr->IsArr(), "TRecFilterAggr expects an array of filter parameters for key `filters`");
    FilterV.Gen(Arr->GetArrVals(), 0);
    for (int FiltN = 0; FiltN < Arr->GetArrVals(); FiltN++) {
        PJsonVal Val = Arr->GetArrVal(FiltN);
        FilterV.Add(TRecFilter::New(Base, Val->GetObjStr("type"), Val));
    }
    // parse out input aggregate
    Aggr = ParseAggr(ParamVal, "aggr");
}

//////////////////////////////////////////////////////////////////////////////////
/// Histogram based anomaly detector aggregate

void THistogramAD::OnStep() {
    if (HistAggr->IsInit()) {
        // Predict
        LastHistIdx = HistAggr->FindBin(InAggrVal->GetFlt());
        // Bin should be found and Severities should be initialized
        if ((LastHistIdx >= 0) && (LastHistIdx < Severities.Len())) {
            Severity = Severities[LastHistIdx];
        } else {
            Severity = -1;
        }
        // Save explanation
        if (Severities.Len() > 0) {
            Explanation = TJsonVal::NewObj();
            PJsonVal ExplanationCells = TJsonVal::NewArr();
            int Len = Severities.Len();
            double BoundStart = HistAggr->GetBoundN(0);
            int CurCount = 1;
            int Code = 0; //assume left extreme
            for (int SevN = LastHistIdx - 1; SevN >= 0; SevN--) {
                if (Severities[SevN] == 0) {
                    // maybe extreme right
                    Code = 1;
                    break;
                }
            }
            if (Code == 1) {
                for (int SevN = LastHistIdx + 1; SevN < Len; SevN++) {
                    if (Severities[SevN] == 0) {
                        // just unexpected
                        Code = 2;
                        break;
                    }
                }
            }
            for (int SevN = 1; SevN < Len; SevN++) {
                if (Severities[SevN] != Severities[SevN - 1]) {
                    // push
                    PJsonVal Last = TJsonVal::NewObj();
                    Last->AddToObj("severity", Severities[SevN - 1]);
                    Last->AddToObj("count", CurCount);
                    Last->AddToObj("boundStart", BoundStart);
                    Last->AddToObj("boundEnd", HistAggr->GetBoundN(SevN));
                    ExplanationCells->AddToArr(Last);
                    BoundStart = HistAggr->GetBoundN(SevN);
                    CurCount = 1;
                } else {
                    CurCount++;
                }
            }
            // push last
            PJsonVal Last = TJsonVal::NewObj();
            Last->AddToObj("severity", Severities[Len - 1]);
            Last->AddToObj("count", CurCount);
            Last->AddToObj("boundStart", BoundStart);
            Last->AddToObj("boundEnd", HistAggr->GetBoundN(Len));
            ExplanationCells->AddToArr(Last);

            Explanation->AddToObj("code", Code);
            Explanation->AddToObj("severities", ExplanationCells);
        }

        // Fit
        TFltV Hist; HistAggr->GetValV(Hist);
        Model.GetPMF(Hist, PMF, Count % AutoBandwidthSkip == 0);
        Model.ClassifyAnomalies(PMF, Severities);
        Count++;
    }
}

THistogramAD::THistogramAD(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal), Model(ParamVal) {
    Reset();
    AutoBandwidthSkip = (int)ParamVal->GetObjNum("skip", 1);
    // parse out input aggregate
    InAggrVal = Cast<TStreamAggrOut::IFlt>(ParseAggr(ParamVal, "inAggr"));
    HistAggr = Cast<TOnlineHistogram>(ParseAggr(ParamVal, "inHistogram"));
}

void THistogramAD::LoadState(TSIn& SIn) {
    Severity.Load(SIn);
    LastHistIdx.Load(SIn);
    PMF.Load(SIn);
    Severities.Load(SIn);
    Explanation = TJsonVal::Load(SIn);
    Count.Load(SIn);
}

void THistogramAD::SaveState(TSOut& SOut) const {
    Severity.Save(SOut);
    LastHistIdx.Save(SOut);
    PMF.Save(SOut);
    Severities.Save(SOut);
    Explanation->Save(SOut);
    Count.Save(SOut);
}

void THistogramAD::Reset() {
    Severity = -1;
    LastHistIdx = 0;
    PMF.Clr();
    Severities.Clr();
    Explanation = TJsonVal::NewObj();
    Count = 0;
}

PJsonVal THistogramAD::SaveJson(const int& Limit) const {
    PJsonVal Obj = TJsonVal::NewObj();
    if (Limit == -1) {
        Obj->AddToObj("pmf", TJsonVal::NewArr(PMF));
        Obj->AddToObj("severities", TJsonVal::NewArr(Severities));
    } else if (Limit >= 0 && Limit < PMF.Len()) {
        TFltV PMFClip; PMF.GetSubValV(0, Limit, PMFClip);
        TIntV SevClip;  Severities.GetSubValV(0, Limit, SevClip);
        Obj->AddToObj("pmf", TJsonVal::NewArr(PMFClip));
        Obj->AddToObj("severities", TJsonVal::NewArr(SevClip));
    }
    Obj->AddToObj("explain", Explanation);
    Obj->AddToObj("thresholds", TJsonVal::NewArr(Model.Thresholds));
    Obj->AddToObj("tol", Model.Tol);
    return Obj;
}

} // TStreamAggrs namespace
} // TQm namespace

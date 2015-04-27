/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#include "qminer_aggr.h"

#ifdef OG_AGGR_DOC_ATLAS
#include <gkswf.h>
#include <vizmapgks.h>
#endif

namespace TQm {

namespace TAggrs {
    
///////////////////////////////
// QMiner-Aggregator-Piechart
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
		SampleRecSet = _RecSet->GetSampleRecSet(SampleSize, false);
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

}

namespace TStreamAggrs {

///////////////////////////////
// QMiner-Time-Stream-Aggregator
TTimeStreamAggr::TTimeStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& _TimeFieldId, const uint64 _TimeWndMSecs): 
            TStreamAggr(Base, AggrNm), TimeStore(Store), TimeFieldId(_TimeFieldId),
			TimeWndMSecs(_TimeWndMSecs) { 

	QmAssertR(Store->GetFieldDesc(TimeFieldId).IsTm(), "Invalid field type");
}

TTimeStreamAggr::TTimeStreamAggr(const TWPt<TBase>& Base, 
        const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {

    // get store
    TStr StoreNm = ParamVal->GetObjStr("store");
    QmAssertR(Base->IsStoreNm(StoreNm), "Invalid store name " + StoreNm);
    TimeStore = Base->GetStoreByStoreNm(StoreNm);
    // get field providing time information
    TStr TimeFieldNm = ParamVal->GetObjStr("timeField");
    QmAssertR(TimeStore->IsFieldNm(TimeFieldNm), "Invalid time field name " + TimeFieldNm);
    TimeFieldId = TimeStore->GetFieldId(TimeFieldNm);
	QmAssertR(TimeStore->GetFieldDesc(TimeFieldId).IsTm(), "Invalid field type");
    // get window size
    QmAssertR(ParamVal->IsObjKey("window"), "Missing window parameter");
    TimeWndMSecs = TJsonVal::GetMSecsFromJsonVal(ParamVal->GetObjKey("window"));
}

TTimeStreamAggr::TTimeStreamAggr(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) :
	TStreamAggr(Base, SABase, SIn), TimeStore(TStore::LoadById(Base, SIn)), 
    TimeFieldId(SIn), TimeWndMSecs(SIn) {
	_Load(SIn);
}

void TTimeStreamAggr::_Load(TSIn& SIn) {
	TQQueue<TQm::TRec> _RecIdQ(SIn);
	RecIdQ = _RecIdQ;
}

void TTimeStreamAggr::Save(TSOut& SOut) const {
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	TimeStore->SaveId(SOut);
	TimeFieldId.Save(SOut); 
	TimeWndMSecs.Save(SOut);
	_Save(SOut);
}

void TTimeStreamAggr::_Save(TSOut& SOut) const {
	RecIdQ.Save(SOut);
}

void TTimeStreamAggr::OnAddRec(const TRec& Rec) {
	QmAssert(Rec.GetStoreId() == GetTimeStore()->GetStoreId());
  // process new record
	// remember record
	RecIdQ.Push(Rec);
	// get new record time
	const uint64 RecTmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
	// update threhsold
	MinTimeMSecs = RecTmMSecs - TimeWndMSecs;
	// execute callback
	OnAddRec(Rec, RecTmMSecs);
  // check if any old records to remove
	while (true) {
		const TRec& OldRec = RecIdQ.Top();
		// check if out of time window
		const uint64 OldRecTmMSecs = OldRec.GetFieldTmMSecs(TimeFieldId);
		if (OldRecTmMSecs < MinTimeMSecs) {
			//execute callback
			OnDeleteRec(OldRec, OldRecTmMSecs);
			// and from the queue
			RecIdQ.Pop();
		} else {
			// nothing old left
			break;
		}
	}
}

///////////////////////////////
// QMiner-Count-Stream-Aggregator
TCount::TCount(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store,
    const int& TimeFieldId, const uint64 TimeWndMSecs): 
        TTimeStreamAggr(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs) { }

TCount::TCount(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TTimeStreamAggr(Base, ParamVal) { }

TCount::TCount(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TTimeStreamAggr(Base, SABase, SIn), Count(SIn) { }

PStreamAggr TCount::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs) {
    
    return new TCount(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs); 
}

PStreamAggr TCount::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TCount(Base, ParamVal); 
}

PStreamAggr TCount::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
    return new TCount(Base, SABase, SIn); 
}

void TCount::_Load(TSIn& SIn) {
	Count.Load(SIn);
}

void TCount::Save(TSOut& SOut) const {
	// save name of the class
	TTypeNm<TCount>().Save(SOut);
	// super save
	TTimeStreamAggr::Save(SOut);
	// save our stuff
	_Save(SOut); 
}

void TCount::_Save(TSOut& SOut) const {
	Count.Save(SOut);
}

///////////////////////////////
// Record Id Buffer.
TRecBuffer::TRecBuffer(const TWPt<TBase>& Base, const TStr& AggrNm, const int& Len): 
    TStreamAggr(Base, AggrNm), Buffer(Len) { }
    
TRecBuffer::TRecBuffer(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
    TStreamAggr(Base, ParamVal), Buffer(ParamVal->GetObjInt("size")) { }

PStreamAggr TRecBuffer::New(const TWPt<TBase>& Base, const TStr& AggrNm, const int& Len) {
    return new TRecBuffer(Base, AggrNm, Len); 
}

PStreamAggr TRecBuffer::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TRecBuffer(Base, ParamVal); 
}

PJsonVal TRecBuffer::SaveJson(const int& Limit) const {
    PJsonVal JsonVal = TJsonVal::NewObj();
    if (!Buffer.Empty()) {
        const TRec& OldestRec = Buffer.GetOldest();
        const TRec& NewestRec = Buffer.GetNewest();
        JsonVal->AddToObj("oldest", OldestRec.GetJson(GetBase(), true, false, false, false, true));
        JsonVal->AddToObj("newest", NewestRec.GetJson(GetBase(), true, false, false, false, true));    
    }
    return JsonVal;
}

///////////////////////////////
// Time series tick.
void TTimeSeriesTick::OnAddRec(const TRec& Rec) {
	TickVal = Rec.GetFieldFlt(TickValFieldId);
	TmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
    InitP = true;
}

TTimeSeriesTick::TTimeSeriesTick(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& AggrNm, 
        const TStr& TimeFieldNm, const TStr& TickValFieldNm): TStreamAggr(Base, AggrNm) {

	// remember input fields
	TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
	TimeFieldId = Store->GetFieldId(TimeFieldNm);
	TickValFieldId = Store->GetFieldId(TickValFieldNm);
}

TTimeSeriesTick::TTimeSeriesTick(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
        TStreamAggr(Base, ParamVal) {
        
    // parse out input and output fields
    TStr StoreNm = ParamVal->GetObjStr("store");
	TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
	TimeFieldId = Store->GetFieldId(TimeFieldNm);
    TStr TickValFieldNm = ParamVal->GetObjStr("value");
	TickValFieldId = Store->GetFieldId(TickValFieldNm);
}

TTimeSeriesTick::TTimeSeriesTick(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
	TimeFieldId(SIn), TickValFieldId(SIn), InitP(SIn), TickVal(SIn), TmMSecs(SIn) { }

PStreamAggr TTimeSeriesTick::New(const TWPt<TBase>& Base, const TStr& StoreNm, 
        const TStr& AggrNm, const TStr& TimeFieldNm, const TStr& TickValFieldNm) {

    return new TTimeSeriesTick(Base, StoreNm, AggrNm, TimeFieldNm, TickValFieldNm);
}

PStreamAggr TTimeSeriesTick::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTimeSeriesTick(Base, ParamVal);
}

PStreamAggr TTimeSeriesTick::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TTimeSeriesTick(Base, SABase, SIn);
}


void TTimeSeriesTick::_Load(TSIn& SIn) {
	InitP.Load(SIn);
	TickVal.Load(SIn);
	TmMSecs.Load(SIn);
}

void TTimeSeriesTick::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	TimeFieldId.Save(SOut);
	TickValFieldId.Save(SOut);
	_Save(SOut);
}

void TTimeSeriesTick::_Save(TSOut& SOut) const {
	InitP.Save(SOut);
	TickVal.Save(SOut);
	TmMSecs.Save(SOut);
}

PJsonVal TTimeSeriesTick::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", TickVal);
	Val->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Time series winbuf.
void TTimeSeriesWinBuf::OnAddRec(const TRec& Rec) {
    // get the value and time stamp of the last record
	InVal = Rec.GetFieldFlt(TickValFieldId);
	InTmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
    InitP = true;
    // empty the former outgoing value vector
    OutValV.Clr(true); OutTmMSecsV.Clr(true);
    int ValLen = AllValV.Len();
    // update the interval    
    AllValV.Add(TFltUInt64Pr(InVal, InTmMSecs));
    
    // prepare the vector of elements that are going to be removed from the window
    for (int ValN = 0; ValN < ValLen; ValN++) {
        if ((InTmMSecs - AllValV[ValN].Val2) > WinSizeMSecs) { 
            OutValV.Add(AllValV[ValN].Val1);
            OutTmMSecsV.Add(AllValV[ValN].Val2);
        } else { break; }
    }
    
    // remove all the elements   
    if (OutValV.Len() > 0) { AllValV.Del(0, OutValV.Len() - 1); }  
}

TTimeSeriesWinBuf::TTimeSeriesWinBuf(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& AggrNm, 
        const TStr& TimeFieldNm, const TStr& TickValFieldNm, const uint64& _WinSizeMSecs): 
        TStreamAggr(Base, AggrNm), WinSizeMSecs(_WinSizeMSecs) {

	// remember input fields
	TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
	TimeFieldId = Store->GetFieldId(TimeFieldNm);
	TickValFieldId = Store->GetFieldId(TickValFieldNm);   
    OutValV = TFltV(); OutTmMSecsV = TUInt64V(); AllValV = TFltUInt64PrV();
}

TTimeSeriesWinBuf::TTimeSeriesWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
        TStreamAggr(Base, ParamVal) {
        
    // parse out input and output fields
    TStr StoreNm = ParamVal->GetObjStr("store");
	TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
	TimeFieldId = Store->GetFieldId(TimeFieldNm);
    TStr TickValFieldNm = ParamVal->GetObjStr("value");
	TickValFieldId = Store->GetFieldId(TickValFieldNm);
	// replacing GetObjInt("winsize"); which supports only int (which is not enough)
	double TmD = ParamVal->GetObjNum("winsize");
	WinSizeMSecs = (uint64)TmD;
	// temporary warning
	if (WinSizeMSecs < 60000) InfoLog("Warning: winsize of TTimeSeriesWinBuf possibly not in msecs (< 60000)");
    OutValV = TFltV(); OutTmMSecsV = TUInt64V(); AllValV = TFltUInt64PrV();
}

TTimeSeriesWinBuf::TTimeSeriesWinBuf(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
	TimeFieldId(SIn), TickValFieldId(SIn), WinSizeMSecs(SIn), InitP(SIn), InVal(SIn), InTmMSecs(SIn), OutValV(SIn), OutTmMSecsV(SIn), AllValV(SIn)  { }

PStreamAggr TTimeSeriesWinBuf::New(const TWPt<TBase>& Base, const TStr& StoreNm, 
        const TStr& AggrNm, const TStr& TimeFieldNm, const TStr& ValFieldNm, 
        const uint64& _WinSizeMSecs) {

    return new TTimeSeriesWinBuf(Base, StoreNm, AggrNm, TimeFieldNm, ValFieldNm, _WinSizeMSecs);
}

PStreamAggr TTimeSeriesWinBuf::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTimeSeriesWinBuf(Base, ParamVal);
}

PStreamAggr TTimeSeriesWinBuf::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TTimeSeriesWinBuf(Base, SABase, SIn);
}

void TTimeSeriesWinBuf::_Load(TSIn& SIn) {
	InitP.Load(SIn);
	InVal.Load(SIn);
	InTmMSecs.Load(SIn);
	OutValV.Load(SIn);
	OutTmMSecsV.Load(SIn);
	AllValV.Load(SIn);
}

void TTimeSeriesWinBuf::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	TimeFieldId.Save(SOut);
	TickValFieldId.Save(SOut);
	WinSizeMSecs.Save(SOut);
	_Save(SOut);
}

void TTimeSeriesWinBuf::_Save(TSOut& SOut) const {
	InitP.Save(SOut);
	InVal.Save(SOut);
	InTmMSecs.Save(SOut);
	OutValV.Save(SOut);
	OutTmMSecsV.Save(SOut);
	AllValV.Save(SOut);
}

void TTimeSeriesWinBuf::GetFltV(TFltV& ValV) const {
	int Len = GetN();
	ValV.Gen(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		ValV[ElN] = AllValV[ElN].Val1;
	}
}

void TTimeSeriesWinBuf::GetTmV(TUInt64V& MSecsV) const {
	int Len = GetN();
	MSecsV.Gen(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		MSecsV[ElN] = AllValV[ElN].Val2;
	}
}

PJsonVal TTimeSeriesWinBuf::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", InVal);
	Val->AddToObj("Time", TTm::GetTmFromMSecs(InTmMSecs).GetWebLogDateTimeStr(false, "T"));
    /*Val->AddToObj("FirstVal", LastTickVal);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(LastTmMSecs).GetWebLogDateTimeStr(false, "T"));*/
	return Val;
}

///////////////////////////////
// Moving Window Buffer Count.
TWinBufCount::TWinBufCount(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
	const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) :
	TStreamAggr(Base, AggrNm) {

	InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TWinBufCount::TWinBufCount(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
	// parse out input aggregate
	TStr InStoreNm = ParamVal->GetObjStr("store");
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");
	PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TWinBufCount::TWinBufCount(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : 
	TStreamAggr(Base, SABase, SIn) {

	TStr InAggrNm; InAggrNm.Load(SIn);

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

PStreamAggr TWinBufCount::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {

	const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
	return new TWinBufCount(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));
}

PStreamAggr TWinBufCount::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {

	return new TWinBufCount(Base, AggrNm, TmWinSize, InAggrNm, SABase);
}

PStreamAggr TWinBufCount::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	return new TWinBufCount(Base, ParamVal);
}


PStreamAggr TWinBufCount::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TWinBufCount(Base, SABase, SIn);
}

void TWinBufCount::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	TStr InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);
}

PJsonVal TWinBufCount::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", InAggrVal->GetN());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(InAggrVal->GetInTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Window Buffer Summa.
void TWinBufSum::OnAddRec(const TRec& Rec) {
	TFltV ValV; InAggrVal->GetOutFltV(ValV);
	TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);
	if (InAggr->IsInit()) {
		Sum.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
			ValV, TmMSecsV);
	}
}

TWinBufSum::TWinBufSum(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
	const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) :
	TStreamAggr(Base, AggrNm) {		
	InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TWinBufSum::TWinBufSum(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
	// parse out input aggregate
	TStr InStoreNm = ParamVal->GetObjStr("store");
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");
	PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

PStreamAggr TWinBufSum::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {

	const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
	return new TWinBufSum(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));
}

PStreamAggr TWinBufSum::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {

	return new TWinBufSum(Base, AggrNm, TmWinSize, InAggrNm, SABase);
}

PStreamAggr TWinBufSum::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	return new TWinBufSum(Base, ParamVal);
}

TWinBufSum::TWinBufSum(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
	Sum(SIn)  { 
	TStr InAggrNm; InAggrNm.Load(SIn);	

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TWinBufSum::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TWinBufSum(Base, SABase, SIn);
}

void TWinBufSum::_Load(TSIn& SIn) {
	Sum.Load(SIn);
}

void TWinBufSum::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff	
	_Save(SOut);
	TStr InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);	
}

void TWinBufSum::_Save(TSOut& SOut) const {
	Sum.Save(SOut);
}

PJsonVal TWinBufSum::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Sum.GetSum());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Sum.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Window Buffer Min.
void TWinBufMin::OnAddRec(const TRec& Rec) {
	TFltV ValV; InAggrVal->GetOutFltV(ValV);
	TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);	
	if (InAggr->IsInit()) {		
		Min.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
			ValV, TmMSecsV);
	}
}

TWinBufMin::TWinBufMin(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
	const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) :
	TStreamAggr(Base, AggrNm) {
	InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TWinBufMin::TWinBufMin(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
	// parse out input aggregate
	TStr InStoreNm = ParamVal->GetObjStr("store");
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");
	PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

PStreamAggr TWinBufMin::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {

	const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
	return new TWinBufMin(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));
}

PStreamAggr TWinBufMin::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {

	return new TWinBufMin(Base, AggrNm, TmWinSize, InAggrNm, SABase);
}

PStreamAggr TWinBufMin::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	return new TWinBufMin(Base, ParamVal);
}

TWinBufMin::TWinBufMin(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
Min(SIn)  {
	TStr InAggrNm; InAggrNm.Load(SIn);

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TWinBufMin::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TWinBufMin(Base, SABase, SIn);
}

void TWinBufMin::_Load(TSIn& SIn) {
	Min.Load(SIn);
}

void TWinBufMin::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff	
	_Save(SOut);
	TStr InAggrNm; InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);
}

void TWinBufMin::_Save(TSOut& SOut) const {
	Min.Save(SOut);
}

PJsonVal TWinBufMin::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Min.GetMin());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Min.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Window Buffer Min.
void TWinBufMax::OnAddRec(const TRec& Rec) {
	TFltV ValV; InAggrVal->GetOutFltV(ValV);
	TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);
	if (InAggr->IsInit()) {
		Max.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
			ValV, TmMSecsV);
	}
}

TWinBufMax::TWinBufMax(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize,
	const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) :
	TStreamAggr(Base, AggrNm) {
	InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TWinBufMax::TWinBufMax(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
	// parse out input aggregate
	TStr InStoreNm = ParamVal->GetObjStr("store");
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");
	PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

PStreamAggr TWinBufMax::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {

	const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
	return new TWinBufMax(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));
}

PStreamAggr TWinBufMax::New(const TWPt<TBase>& Base, const TStr& AggrNm,
	const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {

	return new TWinBufMax(Base, AggrNm, TmWinSize, InAggrNm, SABase);
}

PStreamAggr TWinBufMax::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	return new TWinBufMax(Base, ParamVal);
}

TWinBufMax::TWinBufMax(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
Max(SIn)  {
	TStr InAggrNm; InAggrNm.Load(SIn);

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TWinBufMax::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TWinBufMax(Base, SABase, SIn);
}

void TWinBufMax::_Load(TSIn& SIn) {
	Max.Load(SIn);
}

void TWinBufMax::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff	
	_Save(SOut);
	TStr InAggrNm; InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);
}

void TWinBufMax::_Save(TSOut& SOut) const {
	Max.Save(SOut);
}

PJsonVal TWinBufMax::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Max.GetMax());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Max.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Average.
void TMa::OnAddRec(const TRec& Rec) {
    TFltV ValV; InAggrVal->GetOutFltV(ValV);
    TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);        
	if (InAggr->IsInit()) {
		Ma.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
                ValV, TmMSecsV, InAggrVal->GetN());
	}
}

TMa::TMa(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Ma() {
						 
    InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TMa::TMa(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Ma(ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
    InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TMa::TMa(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn), Ma(SIn) {
	TStr InAggrNm; InAggrNm.Load(SIn);	

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TMa(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
         const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TMa(Base, AggrNm, TmWinSize, InAggrNm, SABase);        
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TMa(Base, ParamVal);
}

PStreamAggr TMa::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TMa(Base, SABase, SIn);
}

void TMa::_Load(TSIn& SIn) {
	Ma.Load(SIn);
}

void TMa::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	_Save(SOut);
	// save our stuff
	TStr InAggrNm; InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);	
}

void TMa::_Save(TSOut& SOut) const {
	Ma.Save(SOut);
}

PJsonVal TMa::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Ma.GetMa());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Ma.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Exponential Moving Average.
void TEma::OnAddRec(const TRec& Rec) {
	if (InAggr->IsInit()) {
		Ema.Update(InAggrVal->GetFlt(), InAggrVal->GetTmMSecs());
	}
}

TEma::TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& Decay, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Ema(Decay, Type, InitMinMSecs, TmInterval) {
						 
    InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

TEma::TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& TmInterval,
		const TSignalProc::TEmaType& Type, const uint64& InitMinMSecs, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Ema(Type, InitMinMSecs, TmInterval) {

    InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

TEma::TEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Ema(ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store", "");
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");	
	PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
    InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
    QmAssertR(!InAggr.Empty(), "TEma::TEma : Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(_InAggr());
    QmAssertR(!InAggrVal.Empty(), "TEma::TEma Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

TEma::TEma(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn), Ema(SIn) {
	TStr InAggrNm; InAggrNm.Load(SIn);
	
	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTm*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TEma::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TEma(Base, AggrNm, TmInterval, Type, InitMinMSecs, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TEma::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const double& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TEma(Base, AggrNm, (double)TmInterval, Type, InitMinMSecs, InAggrNm, SABase);        
}

PStreamAggr TEma::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TEma(Base, ParamVal);
}

PStreamAggr TEma::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TEma(Base, SABase, SIn);
}

void TEma::_Load(TSIn& SIn) {
	Ema.Load(SIn);
}

void TEma::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	_Save(SOut);
	// save our stuff
	TStr InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);
}

void TEma::_Save(TSOut& SOut) const {	
	Ema.Save(SOut);	
}

PJsonVal TEma::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Ema.GetEma());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Ema.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Variance
void TVar::OnAddRec(const TRec& Rec) {
    TFltV ValV; InAggrVal->GetOutFltV(ValV);
    TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);        
	if (InAggr->IsInit()) {		
		TInt N = InAggrVal->GetN();
		TFlt InFlt = InAggrVal->GetInFlt();
		uint64 InTmMSecs = InAggrVal->GetInTmMSecs();
		Var.Update(InFlt, InTmMSecs, ValV, TmMSecsV, N);		
	}
}

TVar::TVar(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Var() {
						 
    InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

TVar::TVar(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Var() {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
    InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);	
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());	
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);	
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TVar(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
         const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TVar(Base, AggrNm, TmWinSize, InAggrNm, SABase);        
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TVar(Base, ParamVal);
}

TVar::TVar(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : 
	TStreamAggr(Base, SABase, SIn), Var(SIn)  { 

	TStr InAggrNm; InAggrNm.Load(SIn);

	PStreamAggr _InAggr = SABase->GetStreamAggr(InAggrNm);
	InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
	QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
	QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTmIO interface: " + InAggrNm);
}

PStreamAggr TVar::Load(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) {
	return new TVar(Base, SABase, SIn);
}


void TVar::_Load(TSIn& SIn) {
	Var.Load(SIn);
}

void TVar::Save(TSOut& SOut) const {
	// save the type of the aggregate
	GetType().Save(SOut);
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	_Save(SOut);
	TStr InAggrNm; InAggrNm = InAggr->GetAggrNm(); InAggrNm.Save(SOut);
}

void TVar::_Save(TSOut& SOut) const {
	Var.Save(SOut);
}


PJsonVal TVar::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();

	Val->AddToObj("Val", Var.GetM2());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Var.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Covariance
void TCov::OnAddRec(const TRec& Rec) {
    TFltV ValVX; InAggrValX->GetOutFltV(ValVX);
    TUInt64V TmMSecsV; InAggrValX->GetOutTmMSecsV(TmMSecsV);        
    TFltV ValVY; InAggrValY->GetOutFltV(ValVY);
	if (InAggrX->IsInit() && InAggrY->IsInit()) {
		Cov.Update(InAggrValX->GetInFlt(), InAggrValY->GetInFlt(), 
            InAggrValX->GetInTmMSecs(), ValVX, ValVY, TmMSecsV, InAggrValX->GetN());
	}
}

TCov::TCov(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNmX, const TStr& InAggrNmY, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Cov() {
						 
    InAggrX = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNmX)());
    QmAssertR(!InAggrX.Empty(), "Stream aggregate does not exist: " + InAggrNmX);
    InAggrValX = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNmX)());
    QmAssertR(!InAggrValX.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmX);
    
    InAggrY = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNmY)());
    QmAssertR(!InAggrY.Empty(), "Stream aggregate does not exist: " + InAggrNmY);
    InAggrValY = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNmY)());
    QmAssertR(!InAggrValY.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmY);
}

TCov::TCov(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Cov(ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNmX = ParamVal->GetObjStr("inAggrX");
    TStr InAggrNmY = ParamVal->GetObjStr("inAggrY");
    PStreamAggr _InAggrX = Base->GetStreamAggr(InStoreNm, InAggrNmX);
    PStreamAggr _InAggrY = Base->GetStreamAggr(InStoreNm, InAggrNmY);
    
    InAggrX = dynamic_cast<TStreamAggr*>(_InAggrX());
    QmAssertR(!InAggrX.Empty(), "Stream aggregate does not exist: " + InAggrNmX);
	InAggrValX = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggrX());
    QmAssertR(!InAggrValX.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmX);
    
    InAggrY = dynamic_cast<TStreamAggr*>(_InAggrY());
    QmAssertR(!InAggrY.Empty(), "Stream aggregate does not exist: " + InAggrNmY);
	InAggrValY = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggrY());
    QmAssertR(!InAggrValY.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmY);
}

PStreamAggr TCov::New(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InStoreNm, const TStr& InAggrNmX, const TStr& InAggrNmY) {
    const uint InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TCov(Base, AggrNm, TmWinSize, InAggrNmX, InAggrNmY, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TCov::New(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNmX, const TStr& InAggrNmY, const TWPt<TStreamAggrBase> SABase) {
    return new TCov(Base, AggrNm, TmWinSize, InAggrNmX, InAggrNmY, SABase);        
}

PStreamAggr TCov::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCov(Base, ParamVal);
}

PJsonVal TCov::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Cov.GetCov());
	Val->AddToObj("Time", TTm::GetTmFromMSecs(Cov.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Correlation
void TCorr::InitInAggr(const TWPt<TStreamAggrBase> SABase, 
        const TStr& InAggrNmCov, const TStr& InAggrNmVarX, const TStr& InAggrNmVarY) {

    // load and cast three input stream aggregators with IFltTm interfaces            
    InAggrCov = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNmCov)());
    QmAssertR(!InAggrCov.Empty(), "Stream aggregate does not exist: " + InAggrNmCov);
    InAggrValCov = dynamic_cast<TStreamAggrOut::IFltTm*>(SABase->GetStreamAggr(InAggrNmCov)());
    QmAssertR(!InAggrValCov.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmCov);

    InAggrVarX = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNmVarX)());
    QmAssertR(!InAggrVarX.Empty(), "Stream aggregate does not exist: " + InAggrNmVarX);
    InAggrValVarX = dynamic_cast<TStreamAggrOut::IFltTm*>(SABase->GetStreamAggr(InAggrNmVarX)());
    QmAssertR(!InAggrValVarX.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmVarX);
    
    InAggrVarY = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNmVarY)());
    QmAssertR(!InAggrVarY.Empty(), "Stream aggregate does not exist: " + InAggrNmVarY);
    InAggrValVarY = dynamic_cast<TStreamAggrOut::IFltTm*>(SABase->GetStreamAggr(InAggrNmVarY)());
    QmAssertR(!InAggrValVarY.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNmVarY);        
}

void TCorr::OnAddRec(const TRec& Rec) {
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

TCorr::TCorr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStreamAggrBase> SABase,
        const TStr& InAggrNmCov, const TStr& InAggrNmVarX, const TStr& InAggrNmVarY): 
            TStreamAggr(Base, AggrNm) {
						 
    // get references to input aggregates
    InitInAggr(SABase, InAggrNmCov, InAggrNmVarX, InAggrNmVarY);
}

TCorr::TCorr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNmCov = ParamVal->GetObjStr("inAggrCov");
    TStr InAggrNmVarX = ParamVal->GetObjStr("inAggrVarX");
    TStr InAggrNmVarY = ParamVal->GetObjStr("inAggrVarY");
    // get store's stream aggregate base
    TWPt<TStore> Store = Base->GetStoreByStoreNm(InStoreNm);   
    TWPt<TStreamAggrBase> SABase = Base->GetStreamAggrBase(Store->GetStoreId()); 
    // get references to input aggregates
    InitInAggr(SABase, InAggrNmCov, InAggrNmVarX, InAggrNmVarY);
}

TCorr::TCorr(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn) {
    //TODO
    Fail;
}

PStreamAggr TCorr::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TStr& InStoreNm, const TStr& InAggrNmCov, const TStr& InAggrNmVarX, 
        const TStr& InAggrNmVarY) {
    
    // get store's stream aggregate base
    TWPt<TStore> Store = Base->GetStoreByStoreNm(InStoreNm);   
    TWPt<TStreamAggrBase> SABase = Base->GetStreamAggrBase(Store->GetStoreId()); 
    // call constructor
    return new TCorr(Base, AggrNm, SABase, InAggrNmCov, InAggrNmVarX, InAggrNmVarY);        
}

PStreamAggr TCorr::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStreamAggrBase> SABase, const TStr& InAggrNmCov, 
        const TStr& InAggrNmVarX, const TStr& InAggrNmVarY) {
    
    return new TCorr(Base, AggrNm, SABase, InAggrNmCov, InAggrNmVarX, InAggrNmVarY);        
}

PStreamAggr TCorr::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TCorr(Base, ParamVal);
}

void TCorr::_Load(TSIn& SIn) {
	//TODO
	Fail;
}

void TCorr::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TStreamAggr::Save(SOut);
    
    //TODO
    Fail;
}

void TCorr::_Save(TSOut& SOut) const {
	//TODO
	Fail;
}

PJsonVal TCorr::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", Corr);
	Val->AddToObj("Time", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Merger field map and interpolator
TMergerFieldMap::TMergerFieldMap(const TWPt<TBase>& Base, const TStr& InStoreNm, 
        const TStr& InFieldNm, const TStr& OutFieldNm_, 
        const TSignalProc::PInterpolator& Interpolator_) {
    
    InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    InFieldId = Base->GetStoreByStoreNm(InStoreNm)->GetFieldId(InFieldNm);
    OutFieldNm = OutFieldNm_;
    Interpolator = Interpolator_;
}

TMergerFieldMap::TMergerFieldMap(const TWPt<TBase>& Base, const TStr& InStoreNm, 
        const TStr& InFieldNm, const TSignalProc::PInterpolator& Interpolator_) {
    
    InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    InFieldId = Base->GetStoreByStoreNm(InStoreNm)->GetFieldId(InFieldNm);
    // generating name of field
	OutFieldNm = "Mer" + TGuid::GenGuid(); OutFieldNm.ChangeChAll('-', '_');
    Interpolator = Interpolator_;    
}

//////////////////////////////////////////////
// StMerger
TStMerger::TStMerger(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
			const TStr& OutTmFieldNm, const bool& CreateStoreP, const bool& Past, const TVec<TStMergerFieldMap>& _FieldMapV, 
			const TStrV& InterpV):
				TQm::TStreamAggr(Base, AggrNm), FieldMapV(_FieldMapV) {

	InitMerger(Base, OutStoreNm, OutTmFieldNm, CreateStoreP, Past, InterpV);
}

TStMerger::TStMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal):
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

			TStMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
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

				TStMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
				FieldMapV.Add(MergerFieldMap);
			} else {
				JoinSeq = TJoinSeq(Base->GetStoreByStoreNm(inStoreNm)->GetStoreId());
				TWPt<TStore> EndStore = JoinSeq.GetEndStore(Base);
				TInt InFieldId = EndStore->GetFieldId(InFieldNm);

				TStMergerFieldMap MergerFieldMap(InFieldId, JoinSeq, TimeField, OutFieldId);
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

void TStMerger::CreateStore(const TStr& NewStoreNm, const TStr& NewTimeFieldNm){
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

TStMerger::TStMerger(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) :
		TStreamAggr(Base, SABase, SIn),
		OutStore(TStore::LoadById(Base, SIn)),
		TimeFieldId(SIn),
		OutFldNmV(SIn),
		FieldMapV(SIn),
		InterpV(SIn),
		StoreIdFldIdVH(SIn),
		NInFlds(SIn),
		Buff(SIn),
		SignalsPresentV(SIn),
		SignalsPresent(SIn),
		NextInterpTm(SIn),
		PrevInterpTm(SIn),
		OnlyPast(SIn),
		PrevInterpPt(SIn) {}

PStreamAggr TStMerger::New(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
		const TStr& OutTmFieldNm, const bool& CreateStoreP, const bool& OnlyPast,
		const TVec<TStMergerFieldMap>& FieldMapV,
		const TStrV& InterpV) {
	return new TStMerger(Base, AggrNm, OutStoreNm, OutTmFieldNm, CreateStoreP, OnlyPast, FieldMapV, InterpV);
}

PStreamAggr TStMerger::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TStMerger(Base, ParamVal);
}


void TStMerger::_Load(TSIn& SIn) {
	TSignalProc::TLinkedBuffer<TUInt64> _Buff(SIn);
	Buff = _Buff;
	SignalsPresentV.Load(SIn);
	SignalsPresent.Load(SIn);
	NextInterpTm.Load(SIn);
	PrevInterpTm.Load(SIn);
	OnlyPast.Load(SIn);
	PrevInterpPt.Load(SIn);
}

void TStMerger::Save(TSOut& SOut) const {
	TStreamAggr::Save(SOut);
	OutStore->SaveId(SOut);
	TimeFieldId.Save(SOut);
	OutFldNmV.Save(SOut);
	FieldMapV.Save(SOut);
	InterpV.Save(SOut);
	StoreIdFldIdVH.Save(SOut),
	NInFlds.Save(SOut);
	_Save(SOut);
}

void TStMerger::_Save(TSOut& SOut) const {
	Buff.Save(SOut);
	SignalsPresentV.Save(SOut);
	SignalsPresent.Save(SOut);
	NextInterpTm.Save(SOut);
	PrevInterpTm.Save(SOut);
	OnlyPast.Save(SOut);
	PrevInterpPt.Save(SOut);
}

PJsonVal TStMerger::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Val", 0);
	Val->AddToObj("Time", 0);
	return Val;
}

void TStMerger::InitFld(const TWPt<TQm::TBase> Base, const TStMergerFieldMap& FieldMap, 
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

void TStMerger::InitMerger(const TWPt<TQm::TBase> Base, const TStr& OutStoreNm,
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

void TStMerger::OnAddRec(const TQm::TRec& Rec) {
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

void TStMerger::OnAddRec(const TQm::TRec& Rec, const TInt& FieldMapIdx) {
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

void TStMerger::AddToBuff(const int& InterpIdx, const uint64 RecTm, const TFlt& Val) {
	TUInt64 LastTm = Buff.Empty() ? TUInt64::Mn : Buff.GetNewest();
	QmAssertR(RecTm >= LastTm, "TStMerger::AddToBuff: Tried to merge past value!");

	InterpV[InterpIdx]->AddPoint(Val, RecTm);

	if (RecTm > LastTm) {
		Buff.Add(RecTm);
	}
}

void TStMerger::AddToStore(const TFltV& InterpValV, const uint64 InterpTm, const uint64& RecId) {
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

void TStMerger::AddRec(const TFltV& InterpValV, const uint64 InterpTm, const TQm::TRec& Rec) {
	if (OnlyPast) {
		// we need to wait until we get at least one future point before
		// committing the interpolation
		if (Buff.Len() > 1) {	// we already have a future point
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

void TStMerger::ShiftBuff() {
	while (!Buff.Empty() && Buff.GetOldest() < NextInterpTm) {
		Buff.DelOldest();
	}

	UpdateInterpolators();
}

bool TStMerger::AllSignalsPresent() {
	if (SignalsPresent) { return true; }

	for (int i = 0; i < NInFlds; i++) {
		if (!SignalsPresentV[i]) {
			return false;
		}
	}

	SignalsPresent = true;
	return true;
}

bool TStMerger::CanInterpolate() {
	if (NextInterpTm == TUInt64::Mx) { return false; }	// this happens when all time series had the same timestamp in the previous iteration
	if (!OnlyPast && NextInterpTm == PrevInterpTm) { return false; }	// avoid duplicates when extrapolating future values

	for (int i = 0; i < NInFlds; i++) {
		if (!InterpV[i]->CanInterpolate(NextInterpTm)) {
			return false;
		}
	}

	return true;
}

void TStMerger::UpdateNextInterpTm() {
	PrevInterpTm = NextInterpTm;
	NextInterpTm = Buff.Len() > 1 ? Buff.GetOldest(1) : TUInt64::Mx;
	ShiftBuff();
}

void TStMerger::UpdateInterpolators() {
	if (NextInterpTm == TUInt64::Mx) { return; }	// edge case, no points in the buffer

	for (int i = 0; i < NInFlds; i++) {
		InterpV[i]->SetNextInterpTm(NextInterpTm);
	}
}

bool TStMerger::CheckInitialized(const int& InterpIdx, const uint64& RecTm) {
	if (!AllSignalsPresent()) {
		SignalsPresentV[InterpIdx] = true;

		if (!AllSignalsPresent()) { return false; }	// should not continue

		NextInterpTm = RecTm;
		ShiftBuff();
	}

	return true;
}

void TStMerger::HandleEdgeCases(const uint64& RecTm) {
	// the buffer was empty before this iteration,
	// the next interpolation time is not set
	if (NextInterpTm == TUInt64::Mx) {
		NextInterpTm = RecTm;
		UpdateInterpolators();
	}
	// duplicate value when extrapolating future
	if (!OnlyPast && NextInterpTm == PrevInterpTm) {
		NextInterpTm = TUInt64::Mx;
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

TResampler::TResampler(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& _IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP): 
            TStreamAggr(Base, AggrNm), UpdatedP(false) {
    
    QmAssertR(InStoreNm != OutStoreNm, "Input and output store in resampler should not be the same!");
    // get pointers to stores
    InStore = Base->GetStoreByStoreNm(InStoreNm);
    // get time field id
    TimeFieldId = InStore->GetFieldId(TimeFieldNm);
    // get ids of interpolated fields 
	for(int FieldN = 0; FieldN < FieldInterpolatorPrV.Len(); FieldN++) {
        const TStr& FieldNm = FieldInterpolatorPrV[FieldN].Val1;
        InFieldIdV.Add(InStore->GetFieldId(FieldNm));
        const TStr& InterpolatorType = FieldInterpolatorPrV[FieldN].Val2;
        InterpolatorV.Add(TSignalProc::TInterpolator::New(InterpolatorType));
	}
    // get output store
    if (CreateStoreP) { CreateStore(OutStoreNm); }
    OutStore = Base->GetStoreByStoreNm(OutStoreNm);   
    // initialize time parameters
	InterpPointMSecs = StartMSecs;
	IntervalMSecs = _IntervalMSecs;
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

TResampler::TResampler(const TWPt<TBase>& Base, const TWPt<TStreamAggrBase> SABase, TSIn& SIn) : TStreamAggr(Base, SABase, SIn),
    InStore(TStore::LoadById(Base, SIn)), InFieldIdV(SIn), InterpolatorV(SIn),
    OutStore(TStore::LoadById(Base, SIn)), TimeFieldId(SIn), IntervalMSecs(SIn),
	InterpPointMSecs(SIn), UpdatedP(SIn) { }

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP) {
    
    return new TResampler(Base, AggrNm, InStoreNm, TimeFieldNm, FieldInterpolatorPrV,
        OutStoreNm, IntervalMSecs, StartMSecs, CreateStoreP);    
}

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TResampler(Base, ParamVal);
}

void TResampler::_Load(TSIn& SIn) {
	InterpPointMSecs.Load(SIn);
	UpdatedP.Load(SIn);
}

void TResampler::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	TStreamAggr::Save(SOut);
	InStore->SaveId(SOut);
	InFieldIdV.Save(SOut);
	InterpolatorV.Save(SOut);
	OutStore->SaveId(SOut);
	TimeFieldId.Save(SOut);
	IntervalMSecs.Save(SOut);
	_Save(SOut);
}

void TResampler::_Save(TSOut& SOut) const {
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

TFtrExtAggr::TFtrExtAggr(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace) :
	TStreamAggr(Base, AggrNm) {
	FtrSpace = _FtrSpace;
}

PStreamAggr TFtrExtAggr::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TFtrSpace>& _FtrSpace) {
	return new TFtrExtAggr(Base, AggrNm, _FtrSpace);
}

double TFtrExtAggr::GetFlt(const TInt& ElN) const {
	QmAssertR(Vec.Len() > ElN, "TFtrExtAggr : GetFlt : index out of bounds");
	return Vec[ElN]; 
}

PJsonVal TFtrExtAggr::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewArr(Vec);
	return Val;
}

void TFtrExtAggr::_Load(TSIn& SIn) {
	Vec.Load(SIn);
}
void TFtrExtAggr::Save(TSOut& SOut) const {
	throw TQmExcept::New("FTrExtAggr::Save not implemented:" + GetAggrNm());
}
void TFtrExtAggr::_Save(TSOut& SOut) const {
	Vec.Save(SOut);
}

///////////////////////////////
// Multi-level analysis stream aggregate
const uint64 THierchCtmc::TU_SECOND = 1000;
const uint64 THierchCtmc::TU_MINUTE = TU_SECOND*60;
const uint64 THierchCtmc::TU_HOUR = TU_MINUTE*60;
const uint64 THierchCtmc::TU_DAY = TU_HOUR*24;

THierchCtmc::TNode::TNode():
		NodeId(),
		Depth(),
		Model(NULL),
		CentroidMat(),
		StateIdV(),
		QMatrixStats(),
		Clust(Model->GetClust()),
		RecIdV(),
		ChildV(),
		PrevStateIdx(-1) {}

THierchCtmc::TNode::TNode(THierchCtmc* _Model, const PRecSet& RecSet, const int& NodeId, const int& _Depth):
		NodeId(NodeId),
		Depth(_Depth),
		Model(_Model),
		CentroidMat(),
		StateIdV(),
		QMatrixStats(),
		Clust(Model->GetClust()),
		RecIdV(),
		ChildV(),
		PrevStateIdx(-1) {

	printf("Initializing a new node ...\n");

	TIntV AssignIdxV;
	InitClusts(RecSet, AssignIdxV);
	InitIntensities(RecSet, AssignIdxV);
}

PJsonVal THierchCtmc::TNode::SaveJson() const {
	const int NStates = GetStates();
	PJsonVal RootJson = TJsonVal::NewObj();

	printf("Saving json ...\n");

	// centroids
	PJsonVal StateJsonV = TJsonVal::NewArr();
	TVector SizeV = GetStatDist();
	for (int i = 0; i < NStates; i++) {
		PJsonVal StateJson = TJsonVal::NewObj();

		TVector FeatV = Model->InvertFtrV(CentroidMat.GetCol(i).GetVec());

		PJsonVal CentroidJsonV = TJsonVal::NewArr();
		for (int FldIdx = 0; FldIdx < GetDim(); FldIdx++) {
			PJsonVal SigJson = TJsonVal::NewObj();

			SigJson->AddToObj("name", Model->GetFldNm(FldIdx));
			SigJson->AddToObj("value", FeatV[FldIdx]);

			CentroidJsonV->AddToArr(SigJson);
		}

		StateJson->AddToObj("id", StateIdV[i]);
		StateJson->AddToObj("meanCentroidDist", GetMeanPtCentroidDist(i));
		StateJson->AddToObj("size", GetStateSize(i));
		StateJson->AddToObj("time", SizeV[i]);
		StateJson->AddToObj("centroid", CentroidJsonV);

		printf("node id: %llu, size: %.2f, mean centroid dist: %.3f\n",
            NodeId.Val, SizeV[i].Val, GetMeanPtCentroidDist(i));

		StateJsonV->AddToArr(StateJson);
	}

	// intensities
	PJsonVal QMatJson = TJsonVal::NewArr();
	TFullMatrix QMatrix = GetQMatrix();
	for (int i = 0; i < NStates; i++) {
		PJsonVal QMatRowJson = TJsonVal::NewArr();
		for (int j = 0; j < NStates; j++) {
			QMatRowJson->AddToArr(QMatrix(i,j));
		}
		QMatJson->AddToArr(QMatRowJson);
	}

	// children
	PJsonVal ChildJsonV = TJsonVal::NewArr();
	for (int i = 0; i < ChildV.Len(); i++) {
		if (ChildV[i] != NULL) {
			PJsonVal ChildJson = ChildV[i]->SaveJson();
			ChildJsonV->AddToArr(ChildJson);
		}
	}

	RootJson->AddToObj("id", NodeId.GetStr());
	RootJson->AddToObj("size", RecIdV.Len());
	RootJson->AddToObj("states", StateJsonV);
	RootJson->AddToObj("intensities", QMatJson);
	RootJson->AddToObj("children", ChildJsonV);

	return RootJson;
}

void THierchCtmc::TNode::OnAddRec(const TRec& Rec, const bool IsInitialized) {
	const int NStates = GetStates();

	// add record ID to the internal structure
	RecIdV.Add(Rec.GetRecId());
	// update statistics
	if (IsInitialized) { UpdateStatistics(Rec); }
	// update intensities
	UpdateIntensities(Rec);

	// add to children
	// check to which child the record belongs
	int StateIdx = Clust->Assign(Model->GetFtrV(Rec));
	if (IsStateExpanded(StateIdx)) {
		ChildV[StateIdx]->OnAddRec(Rec, IsInitialized);
	}

	// check if any state should be expanded
	if (IsInitialized) {
		for (int i = 0; i < NStates; i++) {
			if (ShouldExpand(i)) {
				ExpandState(i);
			}
		}
	}
}


void THierchCtmc::TNode::UpdateIntensities(const TRec& Rec) {
	int CurrStateIdx = Clust->Assign(Model->GetFtrV(Rec));

	if (PrevStateIdx != -1 && CurrStateIdx != PrevStateIdx) {
		// the state has changed
		const uint64 CurrTm = Rec.GetFieldUInt64(Model->TimeFldId);
		if (PrevJumpTm != TUInt64::Mx) {
			uint64 HoldingTm = CurrTm - PrevJumpTm;
			QMatrixStats[PrevStateIdx][CurrStateIdx].Val1++;
			QMatrixStats[PrevStateIdx][CurrStateIdx].Val2 += (double) HoldingTm / Model->TimeUnit;
			printf("Updated intensity: prev state: %d, curr state: %d\n", PrevStateIdx.Val, CurrStateIdx);
		}
		PrevJumpTm = CurrTm;
	} else if (PrevStateIdx == -1) {
		PrevJumpTm = Rec.GetFieldUInt64(Model->TimeFldId);
	}

	PrevStateIdx = CurrStateIdx;
}

void THierchCtmc::TNode::UpdateStatistics(const TRec& Rec) {
	TVector Pt = Model->GetFtrV(Rec);

	int StateId = Clust->Assign(Pt);
	double CentroidDist = Clust->GetDist(StateId, Pt);

	StateStatV[StateId].Val1 += 1;
	StateStatV[StateId].Val2 += CentroidDist;
}

void THierchCtmc::TNode::InitStateStats() {
	const int NStates = GetStates();

	printf("Initailizing statistics ...\n");

	StateStatV.Gen(NStates, 0);
	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
		double MeanPtCentDist = Clust->GetMeanPtCentDist(StateIdx);
		uint64 ClustSize = Clust->GetClustSize(StateIdx);

		StateStatV.Add(TUInt64FltPr(ClustSize, ClustSize * MeanPtCentDist));

		printf("Node: %llu: state %d, points %llu, mean centroid dist %.3f\n",
            NodeId.Val, StateIdx, GetStateSize(StateIdx), GetMeanPtCentroidDist(StateIdx));
	}
}

double THierchCtmc::TNode::GetMeanPtCentroidDist(const int& StateIdx) const {
	uint64 StateSize = GetStateSize(StateIdx);
	return StateSize == 0 ? 0 : StateStatV[StateIdx].Val2 / GetStateSize(StateIdx);
}

uint64 THierchCtmc::TNode::GetStateSize(const int& StateIdx) const {
	return StateStatV[StateIdx].Val1;
}

TVector THierchCtmc::TNode::GetStatDist() const {
	// returns the stationary distribution

	// Norris: Markov Chains states:
	// Let Q be a Q-matrix with jump matrix Pi and let lambda be a measure,
	// than the following are equivalent
	// 1) lambda is invariant
	// 2) mu*Pi = mu where mu_i = lambda_i / q_i, where q_i = -q_ii

	TFullMatrix QMat = GetQMatrix();	// transition rate matrix
	TFullMatrix JumpMat = GetJumpMatrix(QMat);

	printf("Q: %s\n", TStrUtil::GetStr(QMat.GetMat(), ", ", "%.3f").CStr());
	printf("Pi: %s\n", TStrUtil::GetStr(JumpMat.GetMat(), ", ", "%.3f").CStr());

	// find the eigenvector of the jump matrix with eigen value 1
	TVector EigenVec(QMat.GetRows());
	TNumericalStuff::GetEigenVec(JumpMat.GetT().GetMat(), 1.0, EigenVec.Vec);

	printf("Mu: %s\n", TStrUtil::GetStr(EigenVec.Vec, ", ", "%.3f").CStr());
	printf("Mu*Pi: %s\n", TStrUtil::GetStr((EigenVec.GetT()*GetJumpMatrix(QMat)).Vec, ", ", "%.3f").CStr());

	// divide the elements by q_i
	for (int i = 0; i < QMat.GetRows(); i++) {
		EigenVec[i] /= -QMat(i,i);
	}

	printf("Lambda * Q: %s\n", TStrUtil::GetStr((EigenVec.GetT() * QMat).Vec, ", ", "%.3f").CStr());

	// normalize to get a distribution
	return EigenVec /= EigenVec.Sum();
}

TFullMatrix THierchCtmc::TNode::GetJumpMatrix(const TFullMatrix& QMat) {
	const int Rows = QMat.GetRows();
	const int Cols = QMat.GetCols();

	TFullMatrix JumpMat(Rows, Cols);

	for (int i = 0; i < Rows; i++) {
		if (QMat(i,i) == 0.0) {
			JumpMat(i,i) = 1;
		} else {
			for (int j = 0; j < Cols; j++) {
				if (j != i) {
					JumpMat(i,j) = QMat(i,j) / (-QMat(i,i));
				}
			}
		}
	}

	return JumpMat;
}

TVector THierchCtmc::TNode::GetHoldingTimeV(const TFullMatrix& QMat) {
	const int Rows = QMat.GetRows();

	TVector HoldTmV(Rows);
	for (int i = 0; i < Rows; i++) {
		HoldTmV[i] = -1 / QMat(i,i);
	}

	return HoldTmV;
}

bool THierchCtmc::TNode::ShouldExpand(const int& StateIdx) const {
	if (Depth >= Model->MaxDepth() || IsStateExpanded(StateIdx)) { return false; }
	if (GetStateSize(StateIdx) < 100) { return false; }
	double MeanPtCentDist = GetMeanPtCentroidDist(StateIdx);
	return MeanPtCentDist > Model->ExpandThreshold;
}

bool THierchCtmc::TNode::IsStateExpanded(const int& StateIdx) const {
	return ChildV[StateIdx] != NULL;
}

void THierchCtmc::TNode::ExpandState(const int& StateIdx) {
	if (IsStateExpanded(StateIdx)) { return; }

	printf("Expanding state: %d\n", StateIdV[StateIdx].Val);

	// fetch all the records that belong to the state
	TFullMatrix InstanceMat = Model->GetFtrVV(RecIdV);
	TVector AssignV = Clust->Assign(InstanceMat);

	TVector StateAssignIdxV = AssignV.Find([&] (const int Val) { return Val == StateIdx; });

	// if the state doesn't have enough points => ignore
	if (StateAssignIdxV.Len() < 15) {		// TODO hardcoded remove this part
		printf("Cannot expand state %d!\n", StateIdV[StateIdx].Val);
		return;
	}

	// get the record ids
	TUInt64V StateRecIdV(StateAssignIdxV.Len(), 0);
	for (int i = 0; i < StateAssignIdxV.Len(); i++) {
		StateRecIdV.Add(RecIdV[StateAssignIdxV[i]]);
	}

	// get the instance matrix
	ChildV[StateIdx] = new TNode(Model, Model->GetRecSet(StateRecIdV), StateIdV[StateIdx], Depth+1);
}

TFullMatrix THierchCtmc::TNode::GetQMatrix() const {
	// compute the intensities
	const int NStates = GetStates();

	// Q-matrix: holds jump intensities
	TFullMatrix QMatrix(NStates, NStates);
	for (int i = 0; i < NStates; i++) {
		for (int j = 0; j < NStates; j++) {
			if (j != i) {
				const uint64 N = QMatrixStats[i][j].Val1;
				const double Sum = QMatrixStats[i][j].Val2;
				QMatrix(i,j) = N > 0 ? N / Sum : 0;
			}
		}

		QMatrix(i,i) = -QMatrix.RowSum(i);
	}

	return QMatrix;
}

void THierchCtmc::TNode::InitChildV() {
	const int NStates = CentroidMat.GetCols();

	ChildV.Gen(NStates, 0);
	StateIdV.Gen(NStates, 0);
	for (int i = 0; i < NStates; i++) {
		ChildV.Add(NULL);
		StateIdV.Add(Model->GenNodeId());
	}
}

void THierchCtmc::TNode::InitClusts(const PRecSet& RecSet, TIntV& AssignIdxV) {
	TFullMatrix X = Model->GetFtrVV(RecSet);
	// run the algorithm
	Clust->Init(X);
	Clust->Assign(X, AssignIdxV);
	CentroidMat = Clust->GetCentroidMat();
//	CentroidMat = Clust->Apply(X, AssignIdxV);

//	//================================================================
//	// TODO delete
//	TVector AssignV1(AssignIdxV);
//	for (int i = 0; i < GetStates(); i++) {
//		TVector StateV = AssignV1.Find([&] (int Val) { return Val == i; });
//
//		printf("State %d has %d points\n", i, StateV.Len());
//	}
//
//	//================================================================

	// initialize children vector
	InitChildV();
	InitStateStats();
}

void THierchCtmc::TNode::InitIntensities(const PRecSet& RecSet, const TIntV& AssignIdxV) {
	// compute the intensities using the maximum likelihood estimate
	// lambda = 1 / t_avg = n / sum(t_i)

	const int NRecs = RecSet->GetRecs();
	const int NStates = GetStates();

	// initialize a matrix holding the number of measurements and the sum
	QMatrixStats = TVec<TUInt64FltPrV>(NStates, 0);
	for (int i = 0; i < NStates; i++) {
		QMatrixStats.Add(TUInt64FltPrV(NStates, NStates));
	}

	for (int i = 0; i < NRecs; i++) {
		OnAddRec(RecSet->GetRec(i), false);
	}
}

THierchCtmc::THierchCtmc(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,
			const TStr& TimeFldNm, const TInt& _MinRecs, const PJsonVal& _ClustParams,
			const TFlt& _ExpandThreshold, const TUInt64 _TimeUnit, const TInt& _MaxDepth, const int& RndSeed):
		TQm::TStreamAggr(Base, AggrNm),
		InStore(Base->GetStoreByStoreNm(InStoreNm)),
		TimeFldId(Base->GetStoreByStoreNm(InStoreNm)->GetFieldId(TimeFldNm)),
		FtrSpace(NULL),
		RootNode(NULL),
		CurrRecs(),
		MinRecs(_MinRecs),
		MaxDepth(_MaxDepth),
		ClustParams(_ClustParams),
		ExpandThreshold(_ExpandThreshold),
		TimeUnit(_TimeUnit),
		Normalize(true),
		CurrNodeId(),
		Rnd(RndSeed) {}

THierchCtmc::THierchCtmc(const THierchCtmc& Model):
		TQm::TStreamAggr(Model.Base, Model.AggrNm),
		InStore(Model.InStore),
		FldIdV(Model.FldIdV),
		TimeFldId(Model.TimeFldId),
		FtrSpace(NULL),
		RootNode(Model.RootNode),
		CurrRecs(Model.CurrRecs),
		MinRecs(Model.MinRecs),
		MaxDepth(Model.MaxDepth),
		ClustParams(Model.ClustParams),
		ExpandThreshold(Model.ExpandThreshold),
		TimeUnit(Model.TimeUnit),
		Normalize(true),
		CurrNodeId() {}


THierchCtmc::~THierchCtmc() {
//	DestroyNode(RootNode);
}

PStreamAggr THierchCtmc::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,
		const TStr& TimeFldNm, const TInt& MinRecs, const PJsonVal& ClustParams,
		const TFlt& ExpandThreshold, const TUInt64 TimeUnit, const TInt& MaxDepth, const int& RndSeed) {
	return new THierchCtmc(Base, AggrNm, InStoreNm, TimeFldNm, MinRecs, ClustParams, ExpandThreshold, TimeUnit, MaxDepth, RndSeed);
}

PStreamAggr THierchCtmc::New(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal) {
	const TStr InStoreNm = ParamVal->GetObjStr("source");
	const TStr AggrNm = ParamVal->GetObjStr("name");
	const TInt MinRecs = ParamVal->GetObjNum("minRecs");
	const PJsonVal ClustParams = ParamVal->GetObjKey("clustering");
	const TStr TimeFldNm = ParamVal->GetObjStr("timestamp");
	const TFlt ExpandThreshold = ParamVal->GetObjNum("expandThreshold");
	const TInt MaxDepth = ParamVal->IsObjKey("depth") ? ParamVal->GetObjInt("depth") : TInt::Mx;
	const TInt RndSeed = ParamVal->IsObjKey("randSeed") ? ParamVal->GetObjInt("randSeed") : 0;

	const TStr TimeUnitStr = ParamVal->GetObjStr("timeUnit");

	uint64 TimeUnit;
	if (TimeUnitStr == "day") {
		TimeUnit = TU_DAY;
	} else if (TimeUnitStr == "hour") {
		TimeUnit = TU_HOUR;
	} else if (TimeUnitStr == "minute") {
		TimeUnit = TU_MINUTE;
	} else if (TimeUnitStr == "second") {
		TimeUnit = TU_SECOND;
	} else {
		throw TExcept::New("THierchCtmc::New: invalid time unit!", "THierchCtmc::New");
	}

	return new THierchCtmc(Base, AggrNm, InStoreNm, TimeFldNm, MinRecs, ClustParams, ExpandThreshold, TimeUnit, MaxDepth, RndSeed);
}

PJsonVal THierchCtmc::SaveJson(const int& Limit) const {
	if (RootNode.Empty()) { return TJsonVal::NewNull(); }
	return RootNode->SaveJson();
}

void THierchCtmc::OnAddRec(const TRec& Rec) {
	CurrRecs++;
	if (CurrRecs == MinRecs) {
		// cluster
		InitRoot();
	} else if (CurrRecs > MinRecs) {
		RootNode->OnAddRec(Rec);
	}
}

TVector THierchCtmc::GetFtrV(const TRec& Rec) const {
	TVector Result;	FtrSpace->GetFullV(Rec, Result.Vec);
	return Result;
}

TFullMatrix THierchCtmc::GetFtrVV(const PRecSet& RecSet) const {
	// construct the input matrix
	TVec<TFltV> InstanceVV;	FtrSpace->GetFullVV(RecSet, InstanceVV);
	return TFullMatrix::ColMatrix(InstanceVV);
}

TFullMatrix THierchCtmc::GetFtrVV(const TUInt64V& RecIdV) const {
	return GetFtrVV(GetRecSet(RecIdV));
}

TVector THierchCtmc::InvertFtrV(const TVector& FeatV) const {
	TVector Result;	FtrSpace->InvertFullV(FeatV.GetVec(), Result.Vec);
	return Result;
}

int THierchCtmc::GetMaxDepth() const {
	return MaxDepth;
}

TMl::PFullClust THierchCtmc::GetClust() const {
	const TStr ClustType = ClustParams->GetObjStr("type");

	if (ClustType == "dpmeans") {
		const double Lambda = ClustParams->GetObjNum("lambda");
		const int MinClusts = ClustParams->GetObjInt("minclusts");
		const int MaxClusts = ClustParams->GetObjInt("maxclusts");
		return new TMl::TDpMeans(20, 1, Lambda, MinClusts, MaxClusts, Rnd);
	} else if (ClustType == "kmeans") {
		const int K = ClustParams->GetObjInt("k");
		return new TMl::TFullKMeans(20, 1, K, Rnd);
	} else {
		throw TExcept::New("Invalid clustering type: " + ClustType, "THierchCtmc::GetClust");
	}
}

PRecSet THierchCtmc::GetRecSet(const TUInt64V& RecIdV) const {
	return TRecSet::New(InStore, RecIdV);
}

void THierchCtmc::InitRoot() {
	const TWPt<TBase>& Base = GetBase();
	const PRecSet AllRecSet = InStore->GetAllRecs();

	AllRecSet->SortByField(true, TimeFldId);

	// construct feature space
	const TWPt<TStore>& InStore = AllRecSet->GetStore();
	FldIdV = InStore->GetFieldIdV(TFieldType::oftFlt);

	TFtrExtV FtrExtV;
	for (int i = 0; i < FldIdV.Len(); i++) {
		PFtrExt FtrExt = TFtrExts::TNumeric::New(Base, InStore, FldIdV[i], Normalize);
		FtrExtV.Add(FtrExt);
	}

	FtrSpace = TFtrSpace::New(Base, FtrExtV);
	FtrSpace->Update(AllRecSet);

	RootNode = new TNode(this, AllRecSet, GenNodeId(), 1);
}


//////////////////////////////////////////////
// Composed stream aggregators
bool TCompositional::IsCompositional(const TStr& TypeNm) {
	if (TypeNm.EqI("itEma")) {
        return true;
    }    
    return false;
}

void TCompositional::Register(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
	if (TypeNm.EqI("itEma")) {
		ItEma(Base, ParamVal);
	} else {
        throw TQmExcept::New("Unknown compositional stream aggregate " + TypeNm);
    }
};

TStrV TCompositional::ItEma(const TWPt<TQm::TBase>& Base, 
        const int& NumIter, const double& TmInterval, const TSignalProc::TEmaType& Type,
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TStr& Prefix,
        TWPt<TQm::TStreamAggrBase>& SABase){
    
	// Table of EMA names - starts with 1, because it will be pushed in RegItEmaMA
	TStrV ItEmaNames(NumIter);
	ItEmaNames[0] = Prefix + "_1";
	// first iteration takes parameter InStoreNm as input name
	SABase->AddStreamAggr(TEma::New(Base, ItEmaNames[0], 
        TmInterval, Type, InitMinMSecs, InAggrNm, SABase));
	// the rest get previous iteration as input name
	for (int Iter = 1; Iter < NumIter; Iter++){
		ItEmaNames[Iter] = Prefix + "_" + TInt::GetStr(Iter + 1);
		SABase->AddStreamAggr(TEma::New(Base, ItEmaNames[Iter], TmInterval, Type,
			InitMinMSecs, ItEmaNames[Iter - 1], SABase));
	}       
	return ItEmaNames;
}
TStrV TCompositional::ItEma(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	// Get arguments from ParamVal
	TInt NumIter = ParamVal->GetObjInt("numIter", 1);
	double TmInterval = ParamVal->GetObjNum("tmInterval", 1000.0);
	uint64 InitMinMSecs = (uint64)ParamVal->GetObjInt("initMinMSecs", 0);
	TStr InAggrNm = ParamVal->GetObjStr("inAggr");
	TStr InStoreNm = ParamVal->GetObjStr("store", "");
	TStr Prefix = ParamVal->GetObjStr("prefix", "itema");
	TWPt<TQm::TStreamAggrBase> SABase = InStoreNm.Empty() ? Base->GetStreamAggrBase() : Base->GetStreamAggrBase(
            Base->GetStoreByStoreNm(InStoreNm)->GetStoreId());
	return TCompositional::ItEma(Base, NumIter, TmInterval, 
        TSignalProc::etLinear, InitMinMSecs, InAggrNm, Prefix, SABase);
};

} // TAggrs namespace

} // TQm namespace

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
    	// prepare feature extractor
        FtrExt = TFtrExts::TBagOfWords::New(Base, JoinSeq, 
            FieldId, TFtrExts::bowmConcat, SwSet, Stemmer);
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

TTimeStreamAggr::TTimeStreamAggr(const TWPt<TBase>& Base, TSIn& SIn): 
	TStreamAggr(Base, SIn), TimeStore(TStore::LoadById(Base, SIn)), 
    TimeFieldId(SIn), TimeWndMSecs(SIn) { }

void TTimeStreamAggr::Save(TSOut& SOut) const {
	// super save
	TStreamAggr::Save(SOut);
	// save our stuff
	TimeStore->SaveId(SOut);
	TimeFieldId.Save(SOut); 
	TimeWndMSecs.Save(SOut); 
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

TCount::TCount(const TWPt<TBase>& Base, TSIn& SIn): TTimeStreamAggr(Base, SIn), Count(SIn) { }

PStreamAggr TCount::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs) {
    
    return new TCount(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs); 
}

PStreamAggr TCount::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TCount(Base, ParamVal); 
}

PStreamAggr TCount::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TCount(Base, SIn); 
}

void TCount::Save(TSOut& SOut) const {
	// save name of the class
	TTypeNm<TCount>().Save(SOut);
	// super save
	TTimeStreamAggr::Save(SOut);
	// save our stuff
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
        // deprecated, only works when records passed by reference {
        if (OldestRec.IsByRef()) { JsonVal->AddToObj("first", (int)OldestRec.GetRecId()); }
        if (NewestRec.IsByRef()) { JsonVal->AddToObj("last", (int)NewestRec.GetRecId()); }
        // }
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

PStreamAggr TTimeSeriesTick::New(const TWPt<TBase>& Base, const TStr& StoreNm, 
        const TStr& AggrNm, const TStr& TimeFieldNm, const TStr& TickValFieldNm) {

    return new TTimeSeriesTick(Base, StoreNm, AggrNm, TimeFieldNm, TickValFieldNm);
}

PStreamAggr TTimeSeriesTick::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTimeSeriesTick(Base, ParamVal);
}

PJsonVal TTimeSeriesTick::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("GenericTick", TickVal);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Time series winbuf.
void TTimeSeriesWinBuf::OnAddRec(const TRec& Rec) {
    // get the value and time stamp of the last record
	InVal = Rec.GetFieldFlt(TickValFieldId);
	InTmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
    InitP = true;
    //empty the former outgoing value vector
    OutValV.Clr(true); OutTmMSecsV.Clr(true);
    int ValLen = AllValV.Len();
    // update the interval    
    AllValV.Add(TFltUInt64Pr(InVal, InTmMSecs));
    
    //Prepare the vector of elements that are going to be removed from the window
    for (int ValN = 0; ValN < ValLen; ValN++) {
        if ((InTmMSecs - AllValV[ValN].Val2) > WinSizeMSecs) { 
            OutValV.Add(AllValV[ValN].Val1);
            OutTmMSecsV.Add(AllValV[ValN].Val2);
        } else { break; }
    }
    
    //remove all the elements   
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
    WinSizeMSecs = ParamVal->GetObjInt("winsize") * 60 * 1000;	 
    OutValV = TFltV(); OutTmMSecsV = TUInt64V(); AllValV = TFltUInt64PrV();
}

PStreamAggr TTimeSeriesWinBuf::New(const TWPt<TBase>& Base, const TStr& StoreNm, 
        const TStr& AggrNm, const TStr& TimeFieldNm, const TStr& ValFieldNm, 
        const uint64& _WinSizeMSecs) {

    return new TTimeSeriesWinBuf(Base, StoreNm, AggrNm, TimeFieldNm, ValFieldNm, _WinSizeMSecs);
}

PStreamAggr TTimeSeriesWinBuf::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TTimeSeriesWinBuf(Base, ParamVal);
}

PJsonVal TTimeSeriesWinBuf::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("LastVal", InVal);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(InTmMSecs).GetWebLogDateTimeStr(false, "T"));
    /*Val->AddToObj("FirstVal", LastTickVal);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(LastTmMSecs).GetWebLogDateTimeStr(false, "T"));*/
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
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

TMa::TMa(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Ma(ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
    InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uchar InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TMa(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
         const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TMa(Base, AggrNm, TmWinSize, InAggrNm, SABase);        
}

PStreamAggr TMa::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TMa(Base, ParamVal);
}

PJsonVal TMa::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("MA", Ma.GetMa());
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(Ma.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
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
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
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

PJsonVal TEma::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("EMA", Ema.GetEma());
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(Ema.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Moving Variance
void TVar::OnAddRec(const TRec& Rec) {
    TFltV ValV; InAggrVal->GetOutFltV(ValV);
    TUInt64V TmMSecsV; InAggrVal->GetOutTmMSecsV(TmMSecsV);        
	if (InAggr->IsInit()) {
		Var.Update(InAggrVal->GetInFlt(), InAggrVal->GetInTmMSecs(),
                ValV, TmMSecsV, InAggrVal->GetN());
	}
}

TVar::TVar(const TWPt<TBase>& Base, const TStr& AggrNm, const uint64& TmWinSize, 
        const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase): 
            TStreamAggr(Base, AggrNm), Var() {
						 
    InAggr = dynamic_cast<TStreamAggr*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
    InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(SABase->GetStreamAggr(InAggrNm)());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

TVar::TVar(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), Var(ParamVal) {
    // parse out input aggregate
    TStr InStoreNm = ParamVal->GetObjStr("store");
    TStr InAggrNm = ParamVal->GetObjStr("inAggr");
    PStreamAggr _InAggr = Base->GetStreamAggr(InStoreNm, InAggrNm);
    InAggr = dynamic_cast<TStreamAggr*>(_InAggr());
    QmAssertR(!InAggr.Empty(), "Stream aggregate does not exist: " + InAggrNm);
	InAggrVal = dynamic_cast<TStreamAggrOut::IFltTmIO*>(_InAggr());
    QmAssertR(!InAggrVal.Empty(), "Stream aggregate does not implement IFltTm interface: " + InAggrNm);
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const TStr& AggrNm,         
        const uint64& TmWinSize, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uchar InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TVar(Base, AggrNm, TmWinSize, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
         const uint64& TmWinSize, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TVar(Base, AggrNm, TmWinSize, InAggrNm, SABase);        
}

PStreamAggr TVar::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TVar(Base, ParamVal);
}

PJsonVal TVar::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();

	Val->AddToObj("VAR", Var.GetM2());
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(Var.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
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
    
    const uchar InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
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
	Val->AddToObj("COV", Cov.GetCov());
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(Cov.GetTmMSecs()).GetWebLogDateTimeStr(false, "T"));
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

TCorr::TCorr(const TWPt<TBase>& Base, TSIn& SIn): TStreamAggr(Base, SIn) {
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

void TCorr::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TStreamAggr::Save(SOut);
    
    //TODO
    Fail;
}

PJsonVal TCorr::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("CORR", Corr);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(false, "T"));
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
	OutFieldNm = "Mer" + TGuid::GenGuid(); OutFieldNm.ChangeChAll('-','_');
    Interpolator = Interpolator_;    
}
//TMergerFieldMap::TMergerFieldMap(const TWPt<TBase>& Base, TSIn& SIn): 
//    InStoreId(SIn), InFieldId(SIn), OutFieldNm(SIn), Interpolator(SIn) { }
//void TMergerFieldMap::Save(TSOut& SOut) const {
//	InStoreId.Save(SOut);
//	InFieldId.Save(SOut);
//	OutFieldNm.Save(SOut);
//	Interpolator.Save(SOut);
//	GetInterpolator().Save(SOut);
//}


///////////////////////////////
// Merger
void TMerger::OnAddRec(const TRec& Rec) {
    // make sure we support this store
    const int InTimeFieldId = InTimeFieldIdV[(int)Rec.GetStoreId()];
    QmAssertR(InTimeFieldId != -1, "Merger does not support store '" + Rec.GetStore()->GetStoreNm() + "'");
    // get record time-stamp
    uint64 TmMSecs = Rec.GetFieldTmMSecs(InTimeFieldId);
    // create new record (TODO use TRec)
	PJsonVal RecVal = TJsonVal::NewObj();
    // put the time stamp to latest record's time stamp
    TStr RecTmStr = TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T", true);
	RecVal->AddToObj(OutStore->GetFieldNm(TimeFieldId), RecTmStr);
    // get rest of the fields
	for (int FieldMapN = 0; FieldMapN < FieldMapV.Len(); FieldMapN++) {
        const TMergerFieldMap& FieldMap = FieldMapV[FieldMapN];
        // update interpolators associated with the new record's store
		if (FieldMap.GetInStoreId() == Rec.GetStoreId()) {
            const double Val = Rec.GetFieldFlt(FieldMap.GetInFieldId());            
			FieldMap.GetInterpolator()->Update(Val, TmMSecs);
		}		
		// we interpolate value in current field and add to JSon whether we are in relevant Store or not
		RecVal->AddToObj(FieldMap.GetOutFieldNm(), FieldMap.GetInterpolator()->Interpolate(TmMSecs));  
	}
    // adding JSon object as record
	OutStore->AddRec(RecVal);
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
	for (int FieldMapN = 0; FieldMapN < FieldMapV.Len(); FieldMapN++){
		//creating field 
		PJsonVal FieldVal = TJsonVal::NewObj();
		FieldVal->AddToObj("name", FieldMapV[FieldMapN].GetOutFieldNm());
		FieldVal->AddToObj("type", "float");
		FieldsVal->AddToArr(FieldVal);
	}
	// putting Store description together
	JsonStore->AddToObj("fields", FieldsVal);
    // create new store
	TStorage::CreateStoresFromSchema(TStreamAggr::GetBase(), JsonStore, 1024);
}

TMerger::TMerger(const TWPt<TBase>& Base, const TStr& AggrNm, const TStrPrV& InStoreTimeFieldNmV,
        const TVec<TMergerFieldMap> FieldMapV_, const TStr& OutStoreNm, const TStr& OutTimeFieldNm,
        const bool& CreateStoreP): TStreamAggr(Base, AggrNm), FieldMapV(FieldMapV_) {
	
    // if required, create output store
    if (CreateStoreP) {
        InfoNotify("Creating store '" + OutStoreNm + "'");
        CreateStore(OutStoreNm, OutTimeFieldNm);
    }
    // parse parameters
	OutStore = Base->GetStoreByStoreNm(OutStoreNm);
    TimeFieldId = OutStore->GetFieldId(OutTimeFieldNm);
    // initialize input store time fields map
    InTimeFieldIdV.Gen(255); InTimeFieldIdV.PutAll(-1);
    for (int InTimeFieldN = 0; InTimeFieldN < InStoreTimeFieldNmV.Len(); InTimeFieldN++) {
        // get store
        const TStr& StoreNm = InStoreTimeFieldNmV[InTimeFieldN].Val1;
        TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);
        // get field and remember it in the map
        const TStr& TimeFieldNm = InStoreTimeFieldNmV[InTimeFieldN].Val2;
        InTimeFieldIdV[(int)Store->GetStoreId()] = Store->GetFieldId(TimeFieldNm);
    }
}

PJsonVal TMerger::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	return Val;
}

//StMerger

void TStMerger::OnAddRec(const TQm::TRec& Rec) {
	
	TWPt<TStore> Store = Rec.GetStore();
	TIntV InTmFldV = Store->GetFieldIdV(TFieldType::oftTm);

	TTm Tm;
	Store->GetFieldTm(Rec.GetRecId(), InTmFldV[0], Tm);	// we have already checked that InTmFldV has exactly 1 element
	uint64 UIntTm = TTm::GetMSecsFromTm(Tm);
	for(int HashTblObjN=0; HashTblObjN<HashTableH.Len(); HashTblObjN++){  
		if(HashTableH.GetKey(HashTblObjN).Val1==Rec.GetStore()->GetStoreNm()){  //checks if we are in relevant Store
			//if we are in relevant Store, we update interpolation buffer
			InterpTableV[HashTblObjN]->Update(Rec.GetStore()->GetFieldFlt(Rec.GetRecId(),Rec.GetStore()->GetFieldId(HashTableH.GetKey(HashTblObjN).Val2)),TTm::GetMSecsFromTm(Tm));

			TVec<TUInt64>& WaitingList = WaitingListV[HashTblObjN];
			WaitingList.Add(UIntTm);

			while(!WaitingList.Empty()){
				for(int BufferIndex1 = 0; BufferIndex1<BufferMatrixV.Len();BufferIndex1++){
					if(WaitingListV[HashTblObjN].Len()>0){
						if((BufferMatrixV[BufferIndex1].Len()>0)&&(BufferMatrixV[BufferIndex1][0].Val1>WaitingListV[HashTblObjN][0])){
							WaitingListV[HashTblObjN].Del(0);
							}
						else{
							TFlt TempFlt = InterpTableV[HashTblObjN]->Interpolate(WaitingListV[HashTblObjN][0]);
							TPair<TUInt64,TFlt> TempPair(WaitingListV[HashTblObjN][0],TempFlt);
							BufferMatrixV[HashTblObjN].Add(TempPair);
							if(BufferMatrixV[HashTblObjN].Empty() && BufferMatrixV[BufferIndex1][0].Val1>TempPair.Val1) {
								throw TExcept::New("record lost");
							}
						}
						WaitingListV[HashTblObjN].Del(0);
					}
				}
			}
			bool IsFull=true;
			while (IsFull) {
				for(int BufferIndex2 = 0;BufferIndex2<BufferMatrixV.Len();BufferIndex2++){
					if(BufferMatrixV[BufferIndex2].Empty()){
						IsFull=false;
					}
				}
				if(IsFull){
					//AddRecord
					PJsonVal JsonVal = TJsonVal::NewObj();  //creating JSon object
					JsonVal->AddToObj(OutStore->GetFieldNm(TimeFieldId), TTm::GetTmFromMSecs(BufferMatrixV[0][0].Val1).GetWebLogDateTimeStr(true, "T", true));  //creating and filling time field in JSon object
					uint64 TempTm=BufferMatrixV[0][0].Val1;
					for(int BufferIndex3 = 0;BufferIndex3<BufferMatrixV.Len();BufferIndex3++){
						if (BufferMatrixV[BufferIndex3][0].Val1 != TempTm){

							//////////////////////////////////////////////////////////////
							// TODO remove me
							printf("WaitingList\n");
							for (int i = 0; i < WaitingListV.Len(); i++) {
								if (WaitingListV[i].Empty()) {
									printf("--\n");
								} else {
									for (int j = 0; j < WaitingListV[i].Len(); j++) {
										printf("%ld, ", WaitingListV[i][j].Val);
									}
									printf("\n");
								}
							}

							printf("BufferMatrix\n");
							for (int i = 0; i < BufferMatrixV.Len(); i++) {
								if (BufferMatrixV[i].Empty()) {
									printf("--\n");
								} else {
									for (int j = 0; j < BufferMatrixV[i].Len(); j++) {
										printf("%ld, ", BufferMatrixV[i][j].Val1.Val);
									}
									printf("\n");
								}
							}
							//////////////////////////////////////////////////////////////

							throw TExcept::New("Error: Timestamp mismatch.");
						}
						JsonVal->AddToObj(HashTableH[BufferIndex3].GetStr(),BufferMatrixV[BufferIndex3][0].Val2);
						BufferMatrixV[BufferIndex3].Del(0);
					}
					//addingRec
					uint64 NewRecId = OutStore->AddRec(JsonVal);
					if (OutStore->IsJoinNm("source")) {
						OutStore->AddJoin(OutStore->GetJoinId("source"), NewRecId, Rec.GetRecId(), 1);
					}
				}
			}
		}
		else{
			if(WaitingListV[HashTblObjN].Len() == 0){
				WaitingListV[HashTblObjN].Add(UIntTm);
			}
			if(WaitingListV[HashTblObjN][0]<UIntTm)
				{
				if(WaitingListV[HashTblObjN].Last() < UIntTm){WaitingListV[HashTblObjN].Add(UIntTm);}   //èe timestamp larger than last one, we add at the end
				else{																		//else we search and insert
					int TempIndex = 0;
					while((WaitingListV[HashTblObjN][TempIndex]<=UIntTm)&&(TempIndex<WaitingListV[HashTblObjN].Len())){
						TempIndex++;
					}
					WaitingListV[HashTblObjN].Add(UIntTm,TempIndex);
				}
			}
		}
	}
}

TStMerger::TStMerger(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
			const TStr& OutTmFieldNm, const bool& CreateStoreP, const TStrV& InStoreNmV,
			const TStrV& InFldNmV, const TStrV& OutFldNmV, const TStrV& InterpV):
				TQm::TStreamAggr(Base, AggrNm),
				OutFieldsV(OutFldNmV),
				OutStore(NULL){

	InitMerger(Base, OutStoreNm, OutTmFieldNm, CreateStoreP, InStoreNmV, InFldNmV, OutFldNmV, InterpV);
}

TStMerger::TStMerger(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal):
		TStreamAggr(Base, ParamVal) {
	
	QmAssertR(ParamVal->IsObjKey("outStore"), "Field 'outStore' missing!");
	QmAssertR(ParamVal->IsObjKey("timestamp"), "Field 'timestamp' missing!");
	QmAssertR(ParamVal->IsObjKey("mergingMapV"), "Field 'mergingMapV' missing!");

	//input parameters
    TStr OutStoreNm = ParamVal->GetObjStr("outStore");
	const bool CreateStoreP = ParamVal->GetObjBool("createStore", false);
	TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
	PJsonVal FieldArrVal = ParamVal->GetObjKey("mergingMapV");

	TStrV InStoreNmV;
	TStrV InFldNmV;
	TStrV OutFldNmV;
	TStrV InterpV;

	for (int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
		PJsonVal FieldVal = FieldArrVal->GetArrVal(FieldN);

		TStr InStoreNm = FieldVal->GetObjStr("inStore");
		TStr InFieldNm = FieldVal->GetObjStr("inField");
		TStr OutFieldNm = FieldVal->GetObjStr("outField");
		TStr InterpNm = FieldVal->GetObjStr("interpolation");

		InStoreNmV.Add(InStoreNm);
		InFldNmV.Add(InFieldNm);
		OutFldNmV.Add(OutFieldNm);
		InterpV.Add(InterpNm);
	}

	InitMerger(Base, OutStoreNm, TimeFieldNm, CreateStoreP, InStoreNmV, InFldNmV, OutFldNmV, InterpV);
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
	for (int FieldMapN = 0; FieldMapN < HashTableH.Len(); FieldMapN++){
		//creating field 
		PJsonVal FieldVal = TJsonVal::NewObj();
		FieldVal->AddToObj("name", OutFieldsV[FieldMapN]);
		FieldVal->AddToObj("type", "float");
		FieldsVal->AddToArr(FieldVal);
	}
	// putting Store description together
	JsonStore->AddToObj("fields", FieldsVal);
    // create new store
	TStorage::CreateStoresFromSchema(TStreamAggr::GetBase(), JsonStore, 1024);
}

TStMerger::TStMerger(const TWPt<TBase>& Base, TSIn& SIn):
		TStreamAggr(Base, SIn),
		InTimeFieldIdV(SIn),
		WaitingListV(SIn),
		BufferMatrixV(SIn),
		HashTableH(SIn),
		OutFieldsV(SIn),
		OutStoreNm(SIn),
		OutStore(TStore::LoadById(Base, SIn)),
		InterpTableV(SIn),
		TimeFieldId(SIn),
		TimeFieldNm(SIn) { }

PStreamAggr TStMerger::New(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
		const TStr& OutTmFieldNm, const bool& CreateStoreP, const TStrV& InStoreNmV,
		const TStrV& InFldNmV, const TStrV& OutFldNmV, const TStrV& InterpV) {
       return new TStMerger(Base, AggrNm, OutStoreNm, OutTmFieldNm, CreateStoreP, InStoreNmV, InFldNmV, OutFldNmV, InterpV);
}

PStreamAggr TStMerger::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TStMerger(Base, ParamVal);
}

void TStMerger::Save(TSOut& SOut) const {
	TStreamAggr::Save(SOut);
	
	InTimeFieldIdV.Save(SOut);
	WaitingListV.Save(SOut);
	BufferMatrixV.Save(SOut);
	HashTableH.Save(SOut);
	OutFieldsV.Save(SOut);
	OutStoreNm.Save(SOut);
	OutStore->SaveId(SOut);
	InterpTableV.Save(SOut);
	TimeFieldId.Save(SOut);
	TimeFieldNm.Save(SOut);
}

PJsonVal TStMerger::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("Merger", 0);
	Val->AddToObj("UTCTime", 0);
	return Val;
}

void TStMerger::InitFld(const TWPt<TQm::TBase> Base, const TStr& InStoreNm, const TStr& InFldNm, const TStr& OutFldNm,
		const TStr& InterpNm) {

	QmAssertR(Base->IsStoreNm(InStoreNm), "Input store " + InStoreNm + " does not exist!");

	// configure in time field
	const TWPt<TQm::TStore>& InStore = Base->GetStoreByStoreNm(InStoreNm);
	TIntV InTmFldV = InStore->GetFieldIdV(TFieldType::oftTm);

	// check if there is only 1 time field in the input store
	QmAssertR(InTmFldV.Len() == 1, "Input store has multiple or zero time fields: " + TInt(InTmFldV.Len()).GetStr() + "!");
	// check if the input field exists
	QmAssertR(InStore->IsFieldNm(InFldNm), "Field " + InFldNm + " does not exist in store: " + InStoreNm + "!");
	// check if the output field exists in the out store
	QmAssertR(OutStore->IsFieldNm(OutFldNm), "Field " + OutFldNm + " does not exist in the output store!");
	// check if the output field is of correct type
	QmAssertR(OutStore->GetFieldDesc(OutStore->GetFieldId(OutFldNm)).GetFieldType() == TFieldType::oftFlt, "Field " + OutFldNm + " is of incorrect type!");

	InTimeFieldIdV.Add(InTmFldV[0]);
	HashTableH.AddDat(TPair<TStr,TStr>(InStoreNm, InFldNm), OutFldNm);
	InterpTableV.Add(TSignalProc::TInterpolator::New(InterpNm));
	OutFieldsV.Add(OutFldNm);
}

void TStMerger::InitMerger(const TWPt<TQm::TBase> Base, const TStr& OutStoreNm,
		const TStr& OutTmFieldNm, const bool& CreateStoreP, const TStrV& InStoreNmV,
		const TStrV& InFldNmV, const TStrV& OutFldNmV, const TStrV& InterpV) {

	TimeFieldNm = OutTmFieldNm;

	// initialize output store
	// if required, create output store
	if (CreateStoreP) {
		InfoNotify("Creating store '" + OutStoreNm + "'");
		CreateStore(OutStoreNm, TimeFieldNm);
	} else {
		OutStore = Base->GetStoreByStoreNm(OutStoreNm);
	}

	TimeFieldId = OutStore->GetFieldId(TimeFieldNm);

	const int NFlds = InStoreNmV.Len();

	QmAssertR(InFldNmV.Len() == NFlds, "Invalid number of in fields: " + InFldNmV.Len());
	QmAssertR(OutFldNmV.Len() == NFlds, "Invalid number of out fields: " + OutFldNmV.Len());
	QmAssertR(InterpV.Len() == NFlds, "Invalid number of interpolators: " + InterpV.Len());

	for (int i = 0; i < NFlds; i++) {
		const TStr& InStoreNm = InStoreNmV[i];
		const TStr& InFieldNm = InFldNmV[i];
		const TStr& OutFieldNm = OutFldNmV[i];
		const TStr& InterpNm = InterpV[i];

		InitFld(Base, InStoreNm, InFieldNm, OutFieldNm, InterpNm);
	}

	WaitingListV.Gen(NFlds);
	BufferMatrixV.Gen(NFlds);
}


///////////////////////////////
// Resampler
void TResampler::OnAddRec(const TRec& Rec) {
    QmAssertR(Rec.GetStoreId() == InStore->GetStoreId(), "Wrong store calling OnAddRec in Resampler");
    // get record time
    const uint64 RecTmMSecs = Rec.GetFieldTmMSecs(TimeFieldId);
    // only do this first time when interpolation time not defined
    if (InterpPointMSecs == 0) { InterpPointMSecs = RecTmMSecs; }
    // update interpolators
    for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {
        // get field value
        const double Val = Rec.GetFieldFlt(InFieldIdV[FieldN]);
        // update interpolator
        InterpolatorV[FieldN]->Update(Val, RecTmMSecs);
    }  
    // insert new records until we reach new record's time
    while (InterpPointMSecs < RecTmMSecs) {
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
        // increase interpolation time
        InterpPointMSecs += IntervalMSecs;
    }
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
            TStreamAggr(Base, AggrNm) {
    
    QmAssertR(InStoreNm != OutStoreNm, "Input and output store in resampler should not be the same!");
    // get pointers to stores
    InStore = Base->GetStoreByStoreNm(InStoreNm);
    // get time field id
    TimeFieldId = InStore->GetFieldId(TimeFieldNm);
    // get ids of interpolated fields 
	for(int FieldN = 0; FieldN < FieldInterpolatorPrV.Len(); FieldN++) {
        const TStr& FieldNm = FieldInterpolatorPrV[FieldN].Val1;
        InFieldIdV.Add(InStore->GetFieldId(FieldNm()));
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

TResampler::TResampler(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
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
        InFieldIdV.Add(InStore->GetFieldId(FieldNm()));
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

TResampler::TResampler(const TWPt<TBase>& Base, TSIn& SIn): TStreamAggr(Base, SIn),
    InStore(TStore::LoadById(Base, SIn)), InFieldIdV(SIn), InterpolatorV(SIn),
    OutStore(TStore::LoadById(Base, SIn)), TimeFieldId(SIn), IntervalMSecs(SIn),
    InterpPointMSecs(SIn) { }

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP) {
    
    return new TResampler(Base, AggrNm, InStoreNm, TimeFieldNm, FieldInterpolatorPrV,
        OutStoreNm, IntervalMSecs, StartMSecs, CreateStoreP);    
}

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TResampler(Base, ParamVal);
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
	InterpPointMSecs.Save(SOut);
}

PJsonVal TResampler::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();    
	return Val;
}

//////////////////////////////////////////////
// Composed stream aggregators
bool TCompositional::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
	if (TypeNm.EqI("itEma")) {
		ItEma(Base, ParamVal);
		return true;
	} 
	return false;	
};

TStrV TCompositional::ItEma(const TWPt<TQm::TBase>& Base, const TStr& InStoreNm, TInt NumIter, const double& TmInterval, const TSignalProc::TEmaType& Type,
	const uint64& InitMinMSecs, const TStr& InAggrNm, const TStr& Prefix,
	TWPt<TQm::TStreamAggrBase>& SABase){
	// Table of EMA names - starts with 1, because it will be pushed in RegItEmaMA
	TStrV ItEmaNames(NumIter);
	ItEmaNames[0] = Prefix + "_1";
	// first iteration takes parameter InStoreNm as input name
	SABase->AddStreamAggr(TEma::New(Base, ItEmaNames[0], TmInterval, Type,
		InitMinMSecs, InAggrNm, SABase));
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
	TStr InStoreNm = ParamVal->GetObjStr("store");
	TStr Prefix = ParamVal->GetObjStr("prefix", "itema");
	TWPt<TQm::TStreamAggrBase> SABase = Base->GetStreamAggrBase(Base->GetStoreByStoreNm(InStoreNm)->GetStoreId());
	return TCompositional::ItEma(Base, InStoreNm, NumIter, TmInterval,  TSignalProc::etLinear,
		InitMinMSecs, InAggrNm, Prefix, SABase);
};

void TCompositional::TStMerger(const TWPt<TQm::TBase>& Base, const TStr& AggrNm, const TStr& OutStoreNm,
		const TStr& OutTmFieldNm, const bool& CreateStoreP, const TStrV& InStoreNmV,
		const TStrV& InFldNmV, const TStrV& OutFldNmV, const TStrV& InterpV) {
		//TStMerger StMerger(TStMerger::New(Base, StoresAndFieldsV, AggrNm,
		//	InterpolationsV, NewStoreNm, NewTmFieldNm, false));

	// create a store ID vector and remove duplicates
	TUIntV StoreIdV;
	TUIntSet StoreIdSet;
	for (int StoreN = 0; StoreN < InStoreNmV.Len(); StoreN++) {
		const TStr& InStoreNm = InStoreNmV[StoreN];
		const uint64 StoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();

		if (!StoreIdSet.IsKey(StoreId)) {
			StoreIdV.Add(StoreId);
			StoreIdSet.AddKey(StoreId);
		}
	}

	Base->AddStreamAggr(StoreIdV,
			TStMerger::New(
					Base, AggrNm, OutStoreNm, OutTmFieldNm, CreateStoreP, InStoreNmV, InFldNmV, OutFldNmV, InterpV));

}
void TCompositional::TStMerger(const TWPt<TBase>& Base, TStr& AggrNm, const PJsonVal& ParamVal) {
	//input parameters
	TStr OutStoreNm = ParamVal->GetObjStr("outStore");
	const bool CreateStoreP = ParamVal->GetObjBool("createStore", false);
	TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
	PJsonVal FieldArrVal = ParamVal->GetObjKey("mergingMapV");

	TStrV InStoreNmV;
	TStrV InFldNmV;
	TStrV OutFldNmV;
	TStrV InterpV;

	for(int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
		PJsonVal FieldVal = FieldArrVal->GetArrVal(FieldN);

		TStr InStoreNm = FieldVal->GetObjStr("inStore");
		TStr InFieldNm = FieldVal->GetObjStr("inField");
		TStr OutFieldNm = FieldVal->GetObjStr("outField");
		TStr InterpNm = FieldVal->GetObjStr("interpolation");

		InStoreNmV.Add(InStoreNm);
		InFldNmV.Add(InFieldNm);
		OutFldNmV.Add(OutFieldNm);
		InterpV.Add(InterpNm);
	}

	TCompositional::TStMerger(Base, AggrNm, OutStoreNm, TimeFieldNm, CreateStoreP, InStoreNmV, InFldNmV, OutFldNmV, InterpV);
};

}

}

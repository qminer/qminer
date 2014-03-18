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
        PStemmer Stemmer = TFtrExts::TBagOfWords::ParseStemmer(JsonVal, true);
        // read stop words (default is no stop words)
        PSwSet SwSet = TFtrExts::TBagOfWords::ParseSwSet(JsonVal);
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
	// clear preprocessed stuff (not needend anymore)
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
    PJsonVal WndVal = ParamVal->GetObjKey("window");
    if (WndVal->IsNum()) {
        TimeWndMSecs = (uint64)TFlt::Round(WndVal->GetNum());
    } else if (WndVal->IsObj()) {
        TStr Unit = WndVal->GetObjStr("unit");
        const double Value = WndVal->GetObjNum("value");
        if (Unit == "second") {
            TimeWndMSecs = (uint64)TFlt::Round(Value * 1000.0);
        } else if (Unit == "minute") {
            TimeWndMSecs = (uint64)TFlt::Round(Value * 60.0*1000.0);
        } else if (Unit == "hour") {
            TimeWndMSecs = (uint64)TFlt::Round(Value * 60.0*60.0*1000.0);
        } else if (Unit == "day") {
            TimeWndMSecs = (uint64)TFlt::Round(Value * 24.0*60.0*60.0*1000.0);
        } else {
            throw TQmExcept::New("Invalid window size unit " + Unit);
        }
    } else {
        throw TQmExcept::New("Invalid window size parameter");
    }
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

void TTimeStreamAggr::OnAddRec(const TWPt<TStore>& Store, const uint64& RecId) {
	QmAssert(Store->GetStoreId() == GetTimeStore()->GetStoreId());
  // process new record
	// remember record
	RecIdQ.Push(RecId);
	// get new record time
	const uint64 RecTmMSecs = Store->GetFieldTmMSecs(RecId, TimeFieldId);
	// update threhsold
	MinTimeMSecs = RecTmMSecs - TimeWndMSecs;
	// execute callback
	OnAddRec(Store, RecId, RecTmMSecs);
  // check if any old records to remove
	while (true) {
		const uint64 OldRecId = RecIdQ.Top();
		// check if out of time window
		const uint64 OldRecTmMSecs = Store->GetFieldTmMSecs(OldRecId, TimeFieldId);
		if (OldRecTmMSecs < MinTimeMSecs) {
			//execute callback
			OnDeleteRec(Store, OldRecId, OldRecTmMSecs);
			// and from the queue
			RecIdQ.Pop();
		} else {
			// nothing old left
			break;
		}
	}
}

///////////////////////////////
// QMiner-Time-Numeric-Stream-Aggregator
double TTimeNumStreamAggr::GetVal(const TWPt<TStore>& Store, const uint64& RecId) {
	// get the value
	if (FieldType == oftFlt) { 
		return Store->GetFieldFlt(RecId, FieldId);
	} else if (FieldType == oftInt) { 
		return double(Store->GetFieldInt(RecId, FieldId));
	}
	throw TQmExcept::New("Unsupported field type");
}

void TTimeNumStreamAggr::OnAddRec(const TWPt<TStore>& Store, 
		const uint64& RecId, const uint64& RecTimeMSecs) {

	// get record value
	const double RecVal = GetVal(Store, RecId);
	// forward the call
	OnAddRec(Store, RecId, RecTimeMSecs, RecVal);
}

void TTimeNumStreamAggr::OnDeleteRec(const TWPt<TStore>& Store, 
	   const uint64& RecId, const uint64& RecTimeMSecs) {

	// get record value
	const double RecVal = GetVal(Store, RecId);
	// forward the call
	OnDeleteRec(Store, RecId, RecTimeMSecs, RecVal);
}

TTimeNumStreamAggr::TTimeNumStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs, 
        const int& _FieldId): TTimeStreamAggr(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs), 
            FieldId(_FieldId) { 

	// assert field description suiteable
	const TFieldDesc& FieldDesc = GetTimeStore()->GetFieldDesc(FieldId);
	QmAssertR(FieldDesc.IsFlt() || FieldDesc.IsInt(), 
		"[TTimeNumStreamAggr]: Unsupported field type " + FieldDesc.GetFieldTypeStr());
	// remember the field type
	FieldType = FieldDesc.GetFieldType();
}

TTimeNumStreamAggr::TTimeNumStreamAggr(const TWPt<TBase>& Base, 
        const PJsonVal& ParamVal): TTimeStreamAggr(Base, ParamVal) {

    // get field providing numeric data    
    TStr FieldNm = ParamVal->GetObjStr("field");
    TWPt<TStore> Store = GetTimeStore();
    QmAssertR(Store->IsFieldNm(FieldNm), "Invalid field name " + FieldNm);
    FieldId = Store->GetFieldId(FieldNm);
    // check field type fits aggregator
	const TFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
	QmAssertR(FieldDesc.IsFlt() || FieldDesc.IsInt(), 
		"[TTimeNumStreamAggr]: Unsupported field type " + FieldDesc.GetFieldTypeStr());
	// remember the field type
	FieldType = FieldDesc.GetFieldType();
}

TTimeNumStreamAggr::TTimeNumStreamAggr(const TWPt<TBase>& Base, TSIn& SIn): 
    TTimeStreamAggr(Base, SIn), FieldId(SIn), FieldType(static_cast<TFieldType>(TInt(SIn).Val)) { }

void TTimeNumStreamAggr::Save(TSOut& SOut) const {
	// super save
	TTimeStreamAggr::Save(SOut);
	// save our stuff
	FieldId.Save(SOut); 
	TInt(static_cast<int>(FieldType)).Save(SOut);
}

///////////////////////////////
// QMiner-Time-Item-Stream-Aggregator
void TTimeItemStreamAggr::GetRecVal(const TWPt<TStore>& Store, const uint64& RecId, TStrV& RecValV) {
	// get the value
	if (FieldType == oftStr) {
		RecValV.Add(Store->GetFieldStr(RecId, FieldId));
	} else if (FieldType == oftStrV) {
		Store->GetFieldStrV(RecId, FieldId, RecValV);
	} else if (FieldType == oftBool) {
		RecValV.Add(Store->GetFieldBool(RecId, FieldId) ? "true" : "false");
	}
	throw TQmExcept::New("Unsupported field type");
}

void TTimeItemStreamAggr::GetVal(const TWPt<TStore>& Store, const uint64& RecId, TStrV& RecValV) {
	if (FieldJoinSeq.Empty()) {
		GetRecVal(Store, RecId, RecValV);
	} else {
		// first do the joins
		TRec Rec(Store, RecId);
		TWPt<TStore> EndStore = FieldJoinSeq.GetEndStore(GetBase());
		PRecSet RecSet = Rec.DoJoin(GetBase(), FieldJoinSeq);
		// add each join record to aggregate
		for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
			GetRecVal(EndStore(), RecId, RecValV);
		}
	}
}

void TTimeItemStreamAggr::OnAddRec(const TWPt<TStore>& Store, 
		const uint64& RecId, const uint64& RecTimeMSecs) {

	// get record value
	TStrV RecValV; GetVal(Store, RecId, RecValV);
	// forward the call
	OnAddRec(Store, RecId, RecTimeMSecs, RecValV);
}

void TTimeItemStreamAggr::OnDeleteRec(const TWPt<TStore>& Store, 
	   const uint64& RecId, const uint64& RecTimeMSecs) {

	// get record value
	TStrV RecValV; GetVal(Store, RecId, RecValV);
	// forward the call
	OnDeleteRec(Store, RecId, RecTimeMSecs, RecValV);
}

TTimeItemStreamAggr::TTimeItemStreamAggr(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64 TimeWndMSecs, 
        const TJoinSeq& _FieldJoinSeq, const int& _FieldId): 
            TTimeStreamAggr(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs), 
			FieldJoinSeq(_FieldJoinSeq), FieldId(_FieldId) { 

	// assert field description suitable
	const TFieldDesc& FieldDesc = FieldJoinSeq.GetEndStore(Base)->GetFieldDesc(FieldId);
	QmAssertR(FieldDesc.IsStr() || FieldDesc.IsStrV() || FieldDesc.IsBool(),
		"[TTimeItemStreamAggr]: Unsupported field type " + FieldDesc.GetFieldTypeStr());
	// remember the field type
	FieldType = FieldDesc.GetFieldType();
}

TTimeItemStreamAggr::TTimeItemStreamAggr(const TWPt<TBase>& Base, 
        const PJsonVal& ParamVal): TTimeStreamAggr(Base, ParamVal) {

    // get join sequence if exists
    if (ParamVal->IsObjKey("join")) {
        const uchar StartStoreId = GetTimeStore()->GetStoreId();
        PJsonVal JoinSeqVal = ParamVal->GetObjKey("join");
        FieldJoinSeq = TJoinSeq(Base, StartStoreId, JoinSeqVal);
    }    
    // get field providing item data
    TStr FieldNm = ParamVal->GetObjStr("field");
    TWPt<TStore> Store = FieldJoinSeq.GetEndStore(Base);
    QmAssertR(Store->IsFieldNm(FieldNm), "Invalid field name " + FieldNm);
    FieldId = Store->GetFieldId(FieldNm);
    // check field type fits aggregator
	const TFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
	QmAssertR(FieldDesc.IsStr() || FieldDesc.IsStrV() || FieldDesc.IsBool(),
		"[TTimeItemStreamAggr]: Unsupported field type " + FieldDesc.GetFieldTypeStr());
	// remember the field type
	FieldType = FieldDesc.GetFieldType();
}

TTimeItemStreamAggr::TTimeItemStreamAggr(const TWPt<TBase>& Base, TSIn& SIn): 
    TTimeStreamAggr(Base, SIn), FieldJoinSeq(SIn), FieldId(SIn), 
    FieldType(static_cast<TFieldType>(TInt(SIn).Val)) { }

void TTimeItemStreamAggr::Save(TSOut& SOut) const {
	// super save
	TTimeStreamAggr::Save(SOut);
	// save our stuff
	FieldJoinSeq.Save(SOut);
	FieldId.Save(SOut); 
	TInt(static_cast<int>(FieldType)).Save(SOut);
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
// Numeric-Aggregator-Utility
TNumAggrUtil::TNumAggrUtil(TSIn& SIn):
	Count(SIn), Sum(SIn), Min(SIn), Max(SIn), StdMean(SIn), StdM2(SIn) { }
	
void TNumAggrUtil::Save(TSOut& SOut) const {
	Count.Save(SOut); Sum.Save(SOut);
	Min.Save(SOut); Max.Save(SOut);
	StdMean.Save(SOut); StdM2.Save(SOut);
}

void TNumAggrUtil::AddVal(double Val) { 
	// mins and maxs
	if (Count == 0 || Val < Min) { Min = Val; }
	if (Count == 0 || Val > Max) { Max = Val; }
	// super simple stuff
	Count++; Sum += Val; 
	// variance
	const double Delta = Val - StdMean;
	StdMean += Delta / double(Count);
	StdM2 += Delta * (Val - StdMean);
}

double TNumAggrUtil::GetStDev() const {
	if (Count < 3) { return 0.0; }
	else { return StdM2 / double(Count - 2); }
}

///////////////////////////////
// QMiner-Numeric-Stream-Aggregator
void TNumeric::InitNmSet() {
	NmSet.AddKey("count"); NmSet.AddKey("sum");
	NmSet.AddKey("average"); NmSet.AddKey("min");
	NmSet.AddKey("max"); NmSet.AddKey("stdev");
}

TNumeric::TNumeric(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
    const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId): 
        TTimeNumStreamAggr(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs, FieldId), 
        TimeWnd(TimeWndMSecs) { 
    
    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet(); 
}

TNumeric::TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
       TTimeNumStreamAggr(Base, ParamVal), TimeWnd(GetTimeWndMSecs()) {

    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet();
}

TNumeric::TNumeric(const TWPt<TBase>& Base, TSIn& SIn): 
        TTimeNumStreamAggr(Base, SIn), CurrAggr(SIn), LastAggr(SIn), TimeWnd(SIn) { 
    
    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet(); 
}

void TNumeric::OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const double& RecVal) {

	// add to aggregate
	CurrAggr.AddVal(RecVal);
	// update time window
	TimeWnd.Tick(RecTimeMSecs);
}

void TNumeric::NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs) {
	// move now aggregates to last
	LastAggr = CurrAggr;
	// clear now aggregates
	CurrAggr = TNumAggrUtil();
}

PStreamAggr TNumeric::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
        const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId) { 

    return new TNumeric(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs, FieldId); 
}

PStreamAggr TNumeric::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TNumeric(Base, ParamVal); 
}

PStreamAggr TNumeric::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TNumeric(Base, SIn); 
}
    
void TNumeric::Save(TSOut& SOut) const {
	// save name of the class
	TTypeNm<TNumeric>().Save(SOut);
	// super save
	TTimeNumStreamAggr::Save(SOut);
	// save our stuff
	CurrAggr.Save(SOut); 
	LastAggr.Save(SOut); 
	TimeWnd.Save(SOut); 
}

bool TNumeric::IsNmFlt(const TStr& Nm) const {
	return NmSet.IsKey(Nm);
}

double TNumeric::GetNmFlt(const TStr& Nm) const {
	if (Nm == "count") { LastAggr.GetCount(); }
	else if (Nm == "sum") { LastAggr.GetSum(); }
	else if (Nm == "average") { LastAggr.GetAvg(); }
	else if (Nm == "min") { LastAggr.GetMin(); }
	else if (Nm == "max") { LastAggr.GetMax(); }
	else if (Nm == "stdev") { LastAggr.GetStDev(); }
	throw TQmExcept::New("Aggregate '" + Nm + "' not available [TNumeric::GetNmFlt]");
}

void TNumeric::GetNmFltV(TStrFltPrV& NmFltV) const {
	NmFltV.Add(TStrFltPr("count", LastAggr.GetCount()));
	NmFltV.Add(TStrFltPr("sum", LastAggr.GetSum()));
	NmFltV.Add(TStrFltPr("average", LastAggr.GetAvg()));
	NmFltV.Add(TStrFltPr("min", LastAggr.GetMin()));
	NmFltV.Add(TStrFltPr("max", LastAggr.GetMax()));
	NmFltV.Add(TStrFltPr("stdev", LastAggr.GetStDev()));
}

PJsonVal TNumeric::SaveJson(const int& Limit) const { 
	PJsonVal ResVal = TJsonVal::NewObj();
	ResVal->AddToObj("count", LastAggr.GetCount()); 
	ResVal->AddToObj("sum", LastAggr.GetSum()); 
	ResVal->AddToObj("average", LastAggr.GetAvg()); 
	ResVal->AddToObj("min", LastAggr.GetMin()); 
	ResVal->AddToObj("max", LastAggr.GetMax()); 
	ResVal->AddToObj("stdev", LastAggr.GetStDev()); 
	return ResVal;
}

///////////////////////////////
// QMiner-Numeric-Stream-Aggregator-Grouped
void TNumericGroup::InitNmSet() {
	NmSet.AddKey("count"); NmSet.AddKey("sum");
	NmSet.AddKey("average"); NmSet.AddKey("min");
	NmSet.AddKey("max"); NmSet.AddKey("stdev");
}

TNumericGroup::TNumericGroup(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
        const int& TimeFieldId, const uint64& TimeWndMSecs, const int& FieldId, 
        const TJoinSeq& _GroupJoinSeq, const TWPt<TStore>& _GroupStore, const int& _GroupFieldId): 
            TTimeNumStreamAggr(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs, FieldId), 
            GroupJoinSeq(_GroupJoinSeq), GroupStore(_GroupStore), GroupFieldId(_GroupFieldId),
            TimeWnd(TimeWndMSecs) { 

    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet(); 
}	

TNumericGroup::TNumericGroup(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
       TTimeNumStreamAggr(Base, ParamVal), TimeWnd(GetTimeWndMSecs()) {

    // get join sequence to grouping store if exists
    if (ParamVal->IsObjKey("groupJoin")) {
        const uchar StartStoreId = GetTimeStore()->GetStoreId();
        PJsonVal JoinSeqVal = ParamVal->GetObjKey("groupJoin");
        GroupJoinSeq = TJoinSeq(Base, StartStoreId, JoinSeqVal);
        // get end store as group store
        GroupStore = GroupJoinSeq.GetEndStore(Base);
    } else {
        // group store is same as start time store
        GroupStore = GetTimeStore();
    }
    // get grouping field if exists
    if (ParamVal->IsObjKey("groupField")) {
        TStr FieldNm = ParamVal->GetObjStr("groupField");
        QmAssertR(GroupStore->IsFieldNm(FieldNm), "[TNumericGroup]: Invalid group field name " + FieldNm);
        GroupFieldId = GroupStore->GetFieldId(FieldNm);
        // check field type fits aggregator
        QmAssertR(GroupStore->GetFieldDesc(GroupFieldId).IsStr(), 
            "[TNumericGroup]: Unsupported field group type " +
            GroupStore->GetFieldDesc(GroupFieldId).GetFieldTypeStr());
    } else {
        // just remember we don't have it
        GroupFieldId = -1;
    }    
    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet();
}

TNumericGroup::TNumericGroup(const TWPt<TBase>& Base, TSIn& SIn): 
        TTimeNumStreamAggr(Base, SIn), GroupJoinSeq(SIn), 
        GroupStore(TStore::LoadById(Base, SIn)),
        GroupFieldId(SIn), CurrAggrH(SIn), LastAggrH(SIn), TimeWnd(SIn) {

    // initialize callbacks and fields
    TimeWnd.SetCallback(this); InitNmSet(); 
}

void TNumericGroup::OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const double& RecVal) {
    
    // get grouping record
    TRec GroupRec(Store, RecId);    
    if (!GroupJoinSeq.Empty()) {
        PRecSet GroupRecSet = GroupRec.DoJoin(GetBase(), GroupJoinSeq);
        QmAssert(GroupRecSet->GetRecs() == 1);
        GroupRec = GroupRecSet->GetLastRec();
    }
    // get grouping value
    TStr GroupStr;
    if (GroupFieldId != -1) {
        GroupStr = GroupStore->GetFieldStr(GroupRec.GetRecId(), GroupFieldId);
    } else if (GroupStore->HasRecNm()) {
        GroupStr = GroupStore->GetRecNm(GroupRec.GetRecId());
    } else {
        GroupStr = TUInt64::GetStr(GroupRec.GetRecId());
    }
    OnAddRec(Store, RecId, RecTimeMSecs, GroupStr, RecVal);
}    

void TNumericGroup::OnAddRec(const TWPt<TStore>& Store, 
        const uint64& RecId, const uint64& RecTimeMSecs, const TStr& GroupStr,
        const double& RecVal) {

    if (!CurrAggrH.IsKey(GroupStr)) {CurrAggrH.AddDat(GroupStr, TNumAggrUtil());}
	// add to aggregate
	CurrAggrH.GetDat(GroupStr).AddVal(RecVal);
	// update time window
	TimeWnd.Tick(RecTimeMSecs);
}

void TNumericGroup::NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs) {
	// move now aggregates to last
	LastAggrH = CurrAggrH;
	// clear now aggregates
	CurrAggrH.Clr();
}

PStreamAggr TNumericGroup::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const TWPt<TStore>& Store, const int& TimeFieldId, const uint64& TimeWndMSecs, 
        const int& FieldId, const TJoinSeq& GroupJoinSeq, const TWPt<TStore>& GroupStore,
        const int& GroupFieldId) { 
        
    return new TNumericGroup(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs, 
        FieldId, GroupJoinSeq, GroupStore, GroupFieldId);
}
    
PStreamAggr TNumericGroup::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TNumericGroup(Base, ParamVal); 
}

PStreamAggr TNumericGroup::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TNumericGroup(Base, SIn); 
}

void TNumericGroup::Save(TSOut& SOut) const {
	// save name of the class
	TTypeNm<TNumeric>().Save(SOut);
	// super save
	TTimeNumStreamAggr::Save(SOut);    
	// save our stuff
    GroupJoinSeq.Save(SOut);
    GroupStore->SaveId(SOut);
    GroupFieldId.Save(SOut);
	CurrAggrH.Save(SOut); 
	LastAggrH.Save(SOut); 
    TimeWnd.Save(SOut); 
}

bool TNumericGroup::IsNmFlt(const TStr& Nm) const {
	return NmSet.IsKey(Nm);
}

double TNumericGroup::GetNmFlt(const TStr& Nm) const {
	if (Nm == "count") { LastAggrH[0].GetCount(); }
	else if (Nm == "sum") { LastAggrH[0].GetSum(); }
	else if (Nm == "average") { LastAggrH[0].GetAvg(); }
	else if (Nm == "min") { LastAggrH[0].GetMin(); }
	else if (Nm == "max") { LastAggrH[0].GetMax(); }
	else if (Nm == "stdev") { LastAggrH[0].GetStDev(); }
	throw TQmExcept::New("Aggregate '" + Nm + "' not available [TNumeric::GetNmFlt]");
}

void TNumericGroup::GetNmFltV(TStrFltPrV& NmFltV) const {
	NmFltV.Add(TStrFltPr("count", LastAggrH[0].GetCount()));
	NmFltV.Add(TStrFltPr("sum", LastAggrH[0].GetSum()));
	NmFltV.Add(TStrFltPr("average", LastAggrH[0].GetAvg()));
	NmFltV.Add(TStrFltPr("min", LastAggrH[0].GetMin()));
	NmFltV.Add(TStrFltPr("max", LastAggrH[0].GetMax()));
	NmFltV.Add(TStrFltPr("stdev", LastAggrH[0].GetStDev()));
}

PJsonVal TNumericGroup::SaveJson(const int& Limit) const { 
	PJsonVal ResVal = TJsonVal::NewObj();
    int aggId = LastAggrH.FFirstKeyId();
    while (LastAggrH.FNextKeyId(aggId)) {
        PJsonVal GrpVal = TJsonVal::NewObj();
        GrpVal->AddToObj("count", LastAggrH[aggId].GetCount()); 
        GrpVal->AddToObj("sum", LastAggrH[aggId].GetSum()); 
        GrpVal->AddToObj("average", LastAggrH[aggId].GetAvg()); 
        GrpVal->AddToObj("min", LastAggrH[aggId].GetMin()); 
        GrpVal->AddToObj("max", LastAggrH[aggId].GetMax()); 
        GrpVal->AddToObj("stdev", LastAggrH[aggId].GetStDev()); 
        ResVal->AddToObj(LastAggrH.GetKey(aggId), GrpVal); 
    }
	return ResVal;
}

///////////////////////////////
// QMiner-ItemCount-Stream-Aggregator
void TItem::OnAddRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStrV& RecValV) {

	for (int RecValN = 0; RecValN < RecValV.Len(); RecValN++) {
		ItemFqH.AddDat(RecValV[RecValN])++; 
	}
}

void TItem::OnDeleteRec(const TWPt<TStore>& Store, const uint64& RecId, 
        const uint64& RecTimeMSecs, const TStrV& RecValV) {

	for (int RecValN = 0; RecValN < RecValV.Len(); RecValN++) {
		const TStr& RecVal = RecValV[RecValN];
		if (ItemFqH.IsKey(RecVal)) { 
			TInt& RecFq = ItemFqH.GetDat(RecVal);
			// decrease the count
			RecFq--;
			// delete if zero or (god-forbid) less
			if (RecFq <= 0) { ItemFqH.DelKey(RecVal); }
		}
	}
}

TItem::TItem(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store, 
    const int& TimeFieldId, const uint64& TimeWndMSecs, const TJoinSeq& FieldJoinSeq, 
    const int& FieldId): TTimeItemStreamAggr(Base, AggrNm, Store, TimeFieldId, 
        TimeWndMSecs, FieldJoinSeq, FieldId) { }

TItem::TItem(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TTimeItemStreamAggr(Base, ParamVal) { }

TItem::TItem(const TWPt<TBase>& Base, TSIn& SIn): TTimeItemStreamAggr(Base, SIn), ItemFqH(SIn) { }

PStreamAggr TItem::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TWPt<TStore>& Store,
		const int& TimeFieldId, const uint64& TimeWndMSecs, const TJoinSeq& FieldJoinSeq,
		const int& FieldId) { 
    
    return new TItem(Base, AggrNm, Store, TimeFieldId, TimeWndMSecs, FieldJoinSeq, FieldId);
}

PStreamAggr TItem::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TItem(Base, ParamVal);
}

PStreamAggr TItem::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TItem(Base, SIn); 
}
    
void TItem::Save(TSOut& SOut) const {
	// save name of the class
	TTypeNm<TItem>().Save(SOut);
	// super save
	TTimeItemStreamAggr::Save(SOut);
	// save our stuff
	ItemFqH.Save(SOut);
}

bool TItem::IsNm(const TStr& Nm) const {
	return ItemFqH.IsKey(Nm);
}

double TItem::GetNmInt(const TStr& Nm) const {
	return ItemFqH.GetDat(Nm);
}

void TItem::GetNmIntV(TStrIntPrV& NmIntV) const {
	ItemFqH.GetKeyDatPrV(NmIntV);
}

PJsonVal TItem::SaveJson(const int& Limit) const {
	PJsonVal ResVal = TJsonVal::NewArr();
	int KeyId = ItemFqH.FFirstKeyId();
	while (ItemFqH.FNextKeyId(KeyId)) {
		PJsonVal ItemVal = TJsonVal::NewObj();
		ItemVal->AddToObj("name", ItemFqH.GetKey(KeyId));
		ItemVal->AddToObj("count", ItemFqH[KeyId]);
		ResVal->AddToArr(ItemVal);
	}
	return ResVal;
}

///////////////////////////////
// Time series tick.
void TTimeSeriesTick::OnAddRec(const TWPt<TStore>&Store, const uint64& RecId) {
	TickVal = Store->GetFieldFlt(RecId, TickValFieldId);
	TmMSecs = Store->GetFieldTmMSecs(RecId, TimeFieldId);
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
    InfoLog(ParamVal->SaveStr());
    return new TTimeSeriesTick(Base, ParamVal);
}

PJsonVal TTimeSeriesTick::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();
	Val->AddToObj("GenericTick", TickVal);
	Val->AddToObj("UTCTime", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(false, "T"));
	return Val;
}

///////////////////////////////
// Exponential Moving Average.
void TEma::OnAddRec(const TWPt<TStore>&Store, const uint64& RecId) {
	if (InAggr->IsInit()) {
		Ema.Update(InAggrVal->GetFlt(), InAggrVal->GetTmMSecs());
	}
}

TEma::TEma(const TWPt<TBase>& Base, const TStr& AggrNm, const double& Decay, 
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
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
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InStoreNm, const TStr& InAggrNm) {
    
    const uchar InStoreId = Base->GetStoreByStoreNm(InStoreNm)->GetStoreId();
    return new TEma(Base, AggrNm, TmInterval, Type, InitMinMSecs, InAggrNm, Base->GetStreamAggrBase(InStoreId));        
}

PStreamAggr TEma::New(const TWPt<TBase>& Base, const TStr& AggrNm, 
        const uint64& TmInterval, const TSignalProc::TEmaType& Type, 
        const uint64& InitMinMSecs, const TStr& InAggrNm, const TWPt<TStreamAggrBase> SABase) {
    
    return new TEma(Base, AggrNm, TmInterval, Type, InitMinMSecs, InAggrNm, SABase);        
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

///////////////////////////////
//Merger
void TMerger::OnAddRec(const TWPt<TQm::TStore>& Store, const uint64& RecId) {
    // make sure we support this store
    const int InTimeFieldId = InTimeFieldIdV[(int)Store->GetStoreId()];
    QmAssertR(InTimeFieldId != -1, 
        "Merger does not support store '" + Store->GetStoreNm() + "'");
    // get record time-stamp
    uint64 TmMSecs = Store->GetFieldTmMSecs(RecId, InTimeFieldId);
    // create new record (TODO use TRec)
	PJsonVal RecVal = TJsonVal::NewObj();
    // put the time stamp to latest record's time stamp
    TStr RecTmStr = TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T", true);
	RecVal->AddToObj(OutStore->GetFieldNm(TimeFieldId), RecTmStr);
    // get rest of the fields
	for (int FieldMapN = 0; FieldMapN < FieldMapV.Len(); FieldMapN++) {
        const TMergerFieldMap& FieldMap = FieldMapV[FieldMapN];
        // update interpolators associated with the new record's store
		if(FieldMap.GetInStoreId() == Store->GetStoreId()) {
            const double Val = Store->GetFieldFlt(RecId, FieldMap.GetInFieldId());            
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

///////////////////////////////
// Resampler
void TResampler::OnAddRec(const TWPt<TStore>& Store, const uint64& RecId) {
    QmAssertR(Store->GetStoreId() == InStore->GetStoreId(), "Wrong store calling OnAddRec in Resampler");
    // get record time
    const uint64 RecTmMSecs = InStore->GetFieldTmMSecs(RecId, TimeFieldId);
    // only do this first time when interpolation time not defined
    if (InterpPointMSecs == 0) { InterpPointMSecs = RecTmMSecs; }
    // update interpolators
    for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {
        // get field value
        const double Val = InStore->GetFieldFlt(RecId, InFieldIdV[FieldN]);
        // update interpolator
        InterpolatorV[FieldN]->Update(Val, RecTmMSecs);
    }  
    // insert new records until we reach new record's time
    while (InterpPointMSecs < RecTmMSecs) {
        // we start existing record
        PJsonVal JsonVal = InStore->GetRec(RecId).GetJson(GetBase(), true, false, false, false, false);
        // update timestamp
        TStr RecTmStr = TTm::GetTmFromMSecs(InterpPointMSecs).GetWebLogDateTimeStr(true, "T", true);
        JsonVal->AddToObj(InStore->GetFieldNm(TimeFieldId), RecTmStr);
        // update fields
        for (int FieldN = 0; FieldN < InFieldIdV.Len(); FieldN++) {            
            const double FieldVal = InterpolatorV[FieldN]->Interpolate(InterpPointMSecs);            
            JsonVal->AddToObj(InStore->GetFieldNm(InFieldIdV[FieldN]), FieldVal);
        }
        // add new record
        //TODO use TRec instead of PJsonVal
        OutStore->AddRec(JsonVal);
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
	IntervalMSecs = ParamVal->GetObjInt("interval");
}

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const TStr& AggrNm, const TStr& InStoreNm,  
		const TStr& TimeFieldNm, const TStrPrV& FieldInterpolatorPrV, const TStr& OutStoreNm,
		const uint64& IntervalMSecs, const uint64& StartMSecs, const bool& CreateStoreP) {
    
    return new TResampler(Base, AggrNm, InStoreNm, TimeFieldNm, FieldInterpolatorPrV,
        OutStoreNm, IntervalMSecs, StartMSecs, CreateStoreP);    
}

PStreamAggr TResampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TResampler(Base, ParamVal);
}

PJsonVal TResampler::SaveJson(const int& Limit) const {
	PJsonVal Val = TJsonVal::NewObj();    
	return Val;
}


}

}

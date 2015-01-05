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

#include "qminer_op.h"

namespace TQm {

///////////////////////////////
// QMiner-Operator-Search
void TOpSearch::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV) {

	// parse and execute the query
	OutRecSetV.Add(Base->Search(ParamVal));
}

PRecSet TOpSearch::Exec(const TWPt<TBase>& Base, const PQuery& Query) {
	// execute the query
	PRecSet RecSet = Base->Search(Query);
	// done
	return RecSet;
}

///////////////////////////////
// QMiner-Operator-Linear-Search
//TODO: make it use same query style as Base->Search
void TOpLinSearch::ParseQuery(const TWPt<TBase>& Base, const uint& StoreId, 
		const PJsonVal& QueryElt, int& FieldId, TOpLinSearchType& LinSearchType, TStr& FieldVal) {

	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);			
	// split element to field and value
	FieldId = TFlt::Round(QueryElt->GetObjNum("fieldid"));
	QmAssertR(Store->IsFieldId(FieldId), "Invalid field for store " + Store->GetStoreNm());
	// parse range
	TStr Range = QueryElt->GetObjStr("range");
	if (Range == "eq") {
		LinSearchType = oolstEqual;
	} else if (Range == "lt") {
		LinSearchType = oolstLess;
	} else if (Range == "gt") {
		LinSearchType = oolstGreater;
	} else if (Range == "ne") {
		LinSearchType = oolstNotEqual;
	} else {
		throw TQmExcept::New("Unknown range '" + Range + "'");
	}
	// value
	FieldVal = QueryElt->GetObjStr("value");
}

void TOpLinSearch::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV){

	// parse store id parameter
	const uint StoreId = uint(ParamVal->GetObjInt("storeid"));		
	QmAssertR(Base->IsStoreId(StoreId), "Invalid StoreID");
	// parse query elements
	int FieldId; TOpLinSearchType LinSearchType; TStr FieldVal; 
	ParseQuery(Base, StoreId, ParamVal, FieldId, LinSearchType, FieldVal);
	// execute the query
	PRecSet RecSet;
	QmAssertR(Base->GetStoreByStoreId(StoreId)->IsFieldId(FieldId), 
		"FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	const TFieldDesc& FieldDesc = Base->GetStoreByStoreId(StoreId)->GetFieldDesc(FieldId);
	if (FieldDesc.IsInt()){
		if (FieldVal.IsInt()){
			RecSet = Exec(Base, StoreId, FieldId, LinSearchType, FieldVal.GetInt());
			OutRecSetV.Add(RecSet);
		} else {
			throw TQmExcept::New("Field value not an int " + FieldVal);
		}
	} else {
		throw TQmExcept::New("Field type not supported " + FieldDesc.GetFieldTypeStr());
	}
}
	
PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const int& FieldVal){

	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);	
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; PStoreIter Iter = Store->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		if (OpLinSearchType == oolstEqual) { 
			if (Store->GetFieldInt(RecId, FieldId) == FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (Store->GetFieldInt(Iter->GetRecId(), FieldId) < FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (Store->GetFieldInt(RecId, FieldId) > FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (Store->GetFieldInt(RecId, FieldId) != FieldVal) { RecsV.Add(RecId); }
		} else { 
			throw TQmExcept::New("Unknown range operator.");
		}
	}	
	return TRecSet::New(Store, RecsV);
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const uint& StoreId, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const TTm& FieldVal) {
		
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	QmAssertR(FieldVal.IsDef(), "Invalid FieldVal '" + FieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; PStoreIter Iter = Store->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		TTm RecFieldVal; Store->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (OpLinSearchType == oolstEqual) { 
			if (RecFieldVal == FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (RecFieldVal < FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (RecFieldVal > FieldVal) { RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (RecFieldVal != FieldVal) { RecsV.Add(RecId); }
		} else { 
			throw TQmExcept::New("Unknown range operator.");
		}
	}	
    return TRecSet::New(Store, RecsV);
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const uint& StoreId,
		const int& FieldId, const TTm& MnFieldVal, const TTm& MxFieldVal) {

	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	QmAssertR(MnFieldVal.IsDef(), "Invalid FieldVal '" + MnFieldVal.GetStr() + "' is not valid time.");	
	QmAssertR(MxFieldVal.IsDef(), "Invalid FieldVal '" + MxFieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; PStoreIter Iter = Store->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		TTm RecFieldVal; Store->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (MnFieldVal < RecFieldVal && RecFieldVal < MxFieldVal) { RecsV.Add(RecId); }
	}
    return TRecSet::New(Store, RecsV);
}

// return items in the range between MinFieldVal and MaxFieldVal
PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const uint& StoreId,
		const int& FieldId, const uint64& MinFieldVal, const uint64& MaxFieldVal) {
			
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; PStoreIter Iter = Store->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId();
		const uint64 RecFieldVal = Store->GetFieldUInt64(RecId, FieldId);
		if (MinFieldVal <= RecFieldVal && RecFieldVal <= MaxFieldVal) { RecsV.Add(RecId); }
	}	
    return TRecSet::New(Store, RecsV);
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const uint& StoreId,
		const int& FieldId, const TOpLinSearchType& OpLinSearchType, const TIntV& FieldVals) {
	
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");

	THashSet<TInt> FieldValsH;
	for (int i=0; i < FieldVals.Len(); i++)
		FieldValsH.AddKey(FieldVals[i]);

	TUInt64V RecsV; PStoreIter Iter = Store->GetIter();
	if (OpLinSearchType == oolstIsIn) {
		while (Iter->Next()) {
			const uint64 RecId = Iter->GetRecId();
			const int RecFieldVal = Store->GetFieldInt(RecId, FieldId);
			if (FieldValsH.IsKey(RecFieldVal)) 
				RecsV.Add(RecId);
		}	
	} else if (OpLinSearchType == oolstIsNotIn) {
		while (Iter->Next()) {
			const uint64 RecId = Iter->GetRecId();
			const int RecFieldVal = Store->GetFieldInt(RecId, FieldId);
			if (!FieldValsH.IsKey(RecFieldVal)) 
				RecsV.Add(RecId);
		}	
	} else { 
		throw TQmExcept::New("Unknown range operator."); 
	}
	return TRecSet::New(Store, RecsV);	
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const PRecSet& RecSet, const int& FieldId, 
		const TOpLinSearchType& OpLinSearchType, const TTm& FieldVal) {
		
	const TWPt<TStore>& Store = RecSet->GetStore();
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	QmAssertR(FieldVal.IsDef(), "Invalid FieldVal '" + FieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		TTm RecFieldVal; Store->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (OpLinSearchType == oolstEqual) { 
			if (RecFieldVal == FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstLess) { 
			if (RecFieldVal < FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstGreater) { 
			if (RecFieldVal > FieldVal){ RecsV.Add(RecId); }
		} else if (OpLinSearchType == oolstNotEqual) { 
			if (RecFieldVal != FieldVal){ RecsV.Add(RecId); }
		} else { 
			throw TQmExcept::New("Unknown range operator.");
		}
	}
	return TRecSet::New(RecSet->GetStore(), RecsV);	
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const PRecSet& RecSet,
		const int& FieldId, const TTm& MnFieldVal, const TTm& MxFieldVal) {

	const TWPt<TStore>& Store = RecSet->GetStore();
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	QmAssertR(MnFieldVal.IsDef(), "Invalid FieldVal '" + MnFieldVal.GetStr() + "' is not valid time.");	
	QmAssertR(MxFieldVal.IsDef(), "Invalid FieldVal '" + MxFieldVal.GetStr() + "' is not valid time.");	
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		TTm RecFieldVal; Store->GetFieldTm(RecId, FieldId, RecFieldVal);
		if (MnFieldVal < RecFieldVal && RecFieldVal < MxFieldVal) { RecsV.Add(RecId); }
	}
    return TRecSet::New(RecSet->GetStore(), RecsV);
}

PRecSet TOpLinSearch::Exec(const TWPt<TBase>& Base, const PRecSet& RecSet, 
		const int& FieldId, const bool& FieldVal) {
		
	const TWPt<TStore>& Store = RecSet->GetStore();
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++){
		const uint64 RecId = RecSet->GetRecId(RecN);	
		const bool RecFieldVal = Store->GetFieldBool(RecId, FieldId);
		if (RecFieldVal == FieldVal) { RecsV.Add(RecId); }
	}
	return TRecSet::New(RecSet->GetStore(), RecsV);	
}

////////////////////////////////////
// QMiner-Operator-GroupBy
void TOpGroupBy::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV) {

	for (int InRecN = 0; InRecN < InRecSetV.Len(); InRecN++) {
		uint StoreId = InRecSetV[InRecN]->GetStoreId();
		int FieldId = TFlt::Round(ParamVal->GetObjNum("fieldid"));
		Exec(Base, InRecSetV[InRecN], StoreId, FieldId, OutRecSetV);
	}
}

void TOpGroupBy::Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, 
		const uint& StoreId, const int& FieldId, TRecSetV& OutRecSetV) {

	// check all is fine
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	// group by the field
	//TODO: fix so it does not requrie records to be sorted by FieldID
	const int Recs = InRecSet->GetRecs();
	TUInt64V RecV; int CrtVal = -1, PrevVal = -1;
	for (int RecN = 0; RecN < Recs; RecN++) {	
		const uint64 RecId = InRecSet->GetRecId(RecN);
		CrtVal = Store->GetFieldInt(RecId, FieldId);
		if (CrtVal == PrevVal) { 
			RecV.Add(RecId);
		} else { 
			OutRecSetV.Add(TRecSet::New(Store, RecV)); 
			RecV.Clr(true); RecV.Add(RecId);
			PrevVal = CrtVal;
		}			
	}
}

////////////////////////////////////
// QMiner-Operator-SplitBy
void TOpSplitBy::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV) {

    // parse out parameters
    const int FieldId = ParamVal->GetObjInt("fieldid");
    const int SplitWin = ParamVal->GetObjInt("spwin");
    // apply to each input record set
	for (int InRecN = 0; InRecN < InRecSetV.Len(); InRecN++) {
		uint StoreId = InRecSetV[InRecN]->GetStoreId();
		Exec(Base, InRecSetV[InRecN], StoreId, FieldId, SplitWin, OutRecSetV);
	}
}

void TOpSplitBy::Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, 
		const uint& StoreId, const int& FieldId, const int& SplitWinSize, 
		TRecSetV& OutRecSetV) {
	
	QmAssertR(Base->GetStoreByStoreId(StoreId)->IsFieldId(FieldId), 
        "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");
	TUInt64V RecsV; TFlt CrtVal = -1.0, StartVal = -1.0;
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	for (int RecN = 0; RecN < InRecSet->GetRecs(); RecN++) {	
		if (Store->GetFieldDesc(FieldId).GetFieldType() == oftInt) {
			CrtVal = TFlt(Store->GetFieldInt(InRecSet->GetRecId(RecN), FieldId));
		} else if (Store->GetFieldDesc(FieldId).GetFieldType() == oftFlt) {
			CrtVal = Store->GetFieldFlt(InRecSet->GetRecId(RecN), FieldId);
		} else if (Store->GetFieldDesc(FieldId).GetFieldType() == oftTm) {
			TTm Tm; Store->GetFieldTm(InRecSet->GetRecId(RecN), FieldId, Tm);
			CrtVal = double(TTm::GetMSecsFromTm(Tm)) / double(TTmInfo::GetHourMSecs());
		} else {
			throw TQmExcept::New("Sequence FieldType is not Int or Tm!");
		}
		if ((CrtVal <= (StartVal + SplitWinSize)) || (RecN == 0)){ 
			RecsV.Add(InRecSet->GetRecId(RecN));
			if (RecN == 0) {StartVal = CrtVal;}
		} else { 
			OutRecSetV.Add(TRecSet::New(Store, RecsV)); 
			RecsV.Clr(true); RecsV.Add(InRecSet->GetRecId(RecN));
			StartVal = CrtVal;
		}			
	}
	OutRecSetV.Add(TRecSet::New(Store, RecsV)); 
}

////////////////////////////////////
// QMiner-Operator-Agglomerative-Clustering

void TOpAggClust::GetRecIds(const PRecSet& InRecSet, const TVec<TIntV>& RecNV, TRecSetV& OutRecSetV) {
    for (int i = 0; i < RecNV.Len(); i++) {
        TUInt64V RecIdV; 
        for (int RecN = 0; RecN < RecNV[i].Len(); RecN++) {
            RecIdV.Add(InRecSet->GetRecId(RecNV[i][RecN]));
            printf("%d ", RecNV[i][RecN].Val);
        } printf("\n");
        PRecSet TmpSet = TRecSet::New(InRecSet->GetStore(), RecIdV);
        OutRecSetV.Add(TmpSet);	
    }		
}

void TOpAggClust::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, 
		const PJsonVal& ParamVal, TRecSetV& OutRecSetV) {

	for (int InRecN = 0; InRecN < InRecSetV.Len(); InRecN++) {
		uint StoreId = InRecSetV[InRecN]->GetStoreId();
		int FieldId = TFlt::Round(ParamVal->GetObjNum("fieldid"));				
		if (ParamVal->IsObjKey("clustern")) {
			int ClusterN = TFlt::Round(ParamVal->GetObjNum("clustern"));
			QmAssertR(ClusterN > 1, "The minimum number of clusters should be 2");
			Exec(Base, InRecSetV[InRecN], StoreId, FieldId, ClusterN, OutRecSetV);
		} else if (ParamVal->IsObjKey("sim")) {
			double Sim = ParamVal->GetObjNum("sim");
			QmAssertR(Sim > 0.0, "The value of similarity should be greater than 0.");
			Exec(Base, InRecSetV[InRecN], StoreId, FieldId, Sim, OutRecSetV);
		} else { 
			Exec(Base, InRecSetV[InRecN], StoreId, FieldId, OutRecSetV); 
		}		
	}
}

void TOpAggClust::Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, 
		const uint& StoreId, const int& FieldId, const int& ClusterN, 
		TRecSetV& OutRecSetV) {

	// check all is fine
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), 
        "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");

	// iterate over all the fields and make FtrExt vector
	PFtrExt FtrExt = TFtrExts::TNumeric::New(Base, Store, FieldId, false);	
		
	// create new feature space
	PFtrSpace FtrSpace = TFtrSpace::New(Base, FtrExt);
	// get record set of sensors
	TUInt64V InRecIdV; InRecSet->GetRecIdV(InRecIdV);
	PRecSet RecSet = TRecSet::New(Store, InRecIdV);
	// initialize the features
	FtrSpace->Update(RecSet);
	// create feature vectors
	TVec<TIntFltKdV> SpVV;
	FtrSpace->GetSpVV(RecSet, SpVV);
	
	//int RecN = RecSet->GetRecs();
	// agglomerative clustering
	TSparseColMatrix U1(SpVV, SpVV.Len(), SpVV[0].Len()); 	
			
	/*TSparseHrchAggClust SparseGrpAverage;	
	SparseGrpAverage.AggClusterAssgn(&U1);	
	TVec<TIntV> RecNV;
	SparseGrpAverage.GetClusters(ClusterN, RecNV);	
	GetRecIds(InRecSet, RecNV, OutRecSetV);*/
}

void TOpAggClust::Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, 
		const uint& StoreId, const int& FieldId, const double& Sim, 
		TRecSetV& OutRecSetV) {

	// check all is fine
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), 
        "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");

	// iterate over all the fields and make FtrExt vector
	PFtrExt FtrExt = TFtrExts::TNumeric::New(Base, Store, FieldId, false);	
		
	// create new feature space
	PFtrSpace FtrSpace = TFtrSpace::New(Base, FtrExt);
	// get record set of sensors
	TUInt64V InRecIdV; InRecSet->GetRecIdV(InRecIdV);
	PRecSet RecSet = TRecSet::New(Store, InRecIdV);
	// initialize the features
	FtrSpace->Update( RecSet);
	// create feature vectors
	TVec<TIntFltKdV> SpVV;
	FtrSpace->GetSpVV(RecSet, SpVV);
	
	//int RecN = RecSet->GetRecs();
	// agglomerative clustering
	TSparseColMatrix U1(SpVV, SpVV.Len(), SpVV[0].Len()); 	
			
	/*TSparseHrchAggClust SparseGrpAverage;	
	SparseGrpAverage.AggClusterAssgn(&U1);	
	TVec<TIntV> RecNV;
	SparseGrpAverage.GetClusters(Sim, RecNV);	
	GetRecIds(InRecSet, RecNV, OutRecSetV);*/
}

void TOpAggClust::Exec(const TWPt<TBase>& Base, const PRecSet& InRecSet, 
		const uint& StoreId, const int& FieldId, TRecSetV& OutRecSetV) {

	// check all is fine
	TWPt<TStore> Store = Base->GetStoreByStoreId(StoreId);
	QmAssertR(Store->IsFieldId(FieldId), 
        "FieldId '" + TInt::GetStr(FieldId) + "' does not exist.");

	// iterate over all the fields and make FtrExt vector
	PFtrExt FtrExt = TFtrExts::TNumeric::New(Base, Store, FieldId, false);	
		
	// create new feature space
	PFtrSpace FtrSpace = TFtrSpace::New(Base, FtrExt);
	// get record set of sensors
	TUInt64V InRecIdV; InRecSet->GetRecIdV(InRecIdV);
	PRecSet RecSet = TRecSet::New(Store, InRecIdV);
	// initialize the features
	FtrSpace->Update(RecSet);
	// create feature vectors
	TVec<TIntFltKdV> SpVV;
	FtrSpace->GetSpVV(RecSet, SpVV);
	
	//int RecN = RecSet->GetRecs();
	// agglomerative clustering
	TSparseColMatrix U1(SpVV, SpVV.Len(), SpVV[0].Len()); 	
			
	/*TSparseHrchAggClust SparseGrpAverage;	
	SparseGrpAverage.AggClusterAssgn(&U1);	
	TVec<TIntV> RecNV;
	SparseGrpAverage.GetClusters(RecNV);	
	GetRecIds(InRecSet, RecNV, OutRecSetV);*/
}

}

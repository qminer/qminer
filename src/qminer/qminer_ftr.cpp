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

#include "qminer_ftr.h"

namespace TQm {

///////////////////////////////////////////////
// QMiner-Feature-Extractor
TFunRouter<PFtrExt, TFtrExt::TNewF> TFtrExt::NewRouter;

void TFtrExt::Init() {
    Register<TFtrExts::TRandom>();
    Register<TFtrExts::TNumeric>();
    Register<TFtrExts::TCategorical>();
    Register<TFtrExts::TMultinomial>();
    Register<TFtrExts::TBagOfWords>();
    Register<TFtrExts::TJoin>();
    Register<TFtrExts::TJoin>();
}
    
TRec TFtrExt::DoSingleJoin(const TRec& Rec) const {
	QmAssertR(IsStartStore(Rec.GetStoreId()), "Start store not supported!");
	return Rec.DoSingleJoin(Base, GetJoinIdV(Rec.GetStoreId()));
}

TFtrExt::TFtrExt(const TWPt<TBase>& _Base, const TJoinSeqV& JoinSeqV): Base(_Base) {
	QmAssertR(!JoinSeqV.Empty(), "At least one join sequence must be supplied!");
	FtrStore = JoinSeqV[0].GetEndStore(Base);
    // get maping from stores to join sequence
	for (int JoinSeqN = 0; JoinSeqN < JoinSeqV.Len(); JoinSeqN++) {
		const TJoinSeq& JoinSeq = JoinSeqV[JoinSeqN];
        QmAssertR(JoinSeq.GetEndStoreId(Base) == FtrStore->GetStoreId(), "Mismatching feature stores");
		JoinSeqH.AddDat(JoinSeq.GetStartStoreId(), JoinSeq);
	}
}

TFtrExt::TFtrExt(const TWPt<TBase>& _Base, const PJsonVal& ParamVal): Base(_Base) {
    QmAssert(ParamVal->IsObjKey("source"));
    PJsonVal SourceVal = ParamVal->GetObjKey("source");
    if (SourceVal->IsStr()) {
        // we have just store name
        TStr StoreNm = SourceVal->GetStr();
        FtrStore = Base->GetStoreByStoreNm(StoreNm);
    } else if (SourceVal->IsObj()) {
        // get store
        TStr StoreNm = SourceVal->GetObjStr("store");
        FtrStore = Base->GetStoreByStoreNm(StoreNm);       
        // get joins if any give
        if (SourceVal->IsObjKey("join")) {
            TJoinSeq JoinSeq = TJoinSeq(Base, FtrStore->GetStoreId(), SourceVal->GetObjKey("join"));
            FtrStore = JoinSeq.GetEndStore(Base);
        }
    } else if (SourceVal->IsArr()) {
        // parse join sequence
        for (int SourceN = 0; SourceN < SourceVal->GetArrVals(); SourceN++) {
            PJsonVal JoinVal = SourceVal->GetArrVal(SourceN);
            // get store
            TStr StoreNm = JoinVal->GetObjStr("store");
            TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreNm);       
            // get joins if any give
            if (JoinVal->IsObjKey("join")) {                
                TJoinSeq JoinSeq = TJoinSeq(Base, Store->GetStoreId(), JoinVal->GetObjKey("join"));
                Store = JoinSeq.GetEndStore(Base);
        		JoinSeqH.AddDat(JoinSeq.GetStartStoreId(), JoinSeq);
            } else {
                // no join, just remember the store
                JoinSeqH.AddDat(Store->GetStoreId(), TJoinSeq(Store->GetStoreId()));
            }
            // update feature store or assert still consistent
            if (SourceN == 0) { FtrStore = Store; } 
            QmAssertR(FtrStore->GetStoreId() == Store->GetStoreId(), "Mismatch of feature stores");
        }
    }
}

PFtrExt TFtrExt::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
    return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

void TFtrExt::ExtractStrV(const TRec& FtrRec, TStrV& StrV) const { 
	throw TQmExcept::New("ExtractStrV not implemented!"); 
}

void TFtrExt::ExtractFltV(const TRec& FtrRec, TFltV& FltV) const {
	throw TQmExcept::New("ExtractFltV not implemented!"); 
}

void TFtrExt::ExtractTmV(const TRec& FtrRec, TTmV& TmV) const {
	throw TQmExcept::New("ExtractTmV not implemented!"); 
}

///////////////////////////////////////////////
// QMiner-Feature-Space
TStr TFtrSpace::GetNm() const {
	TChA NmChA = "Space: [";
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		if (FtrExtN > 0) { NmChA += ", "; }
		NmChA += FtrExtV[FtrExtN]->GetNm();
	}
	NmChA += "]";
	return NmChA;
}

void TFtrSpace::Clr() {
   	Mode = fsmUpdate;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->Clr();
	}
}

void TFtrSpace::Update(const TRec& Rec) {
	QmAssertR(IsUpdateMode(), "Feature space not in update mode.");
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->Update(Rec);
	}
}

void TFtrSpace::Update(const PRecSet& RecSet) {
	QmAssertR(IsUpdateMode(), "Feature space not in update mode.");
    TEnv::Logger->OnStatusFmt("Updating feature space with %d records", RecSet->GetRecs());
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		if (RecN % 1000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
		Update(RecSet->GetRec(RecN));
	}
}

void TFtrSpace::FinishUpdate() {
	DimV.Gen(FtrExtV.Len(), 0); Dim = 0;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->FinishUpdate();
		const int FtrExtDim = FtrExtV[FtrExtN]->GetDim();
		DimV.Add(FtrExtDim);
		Dim += FtrExtDim;
	}
	Mode = fsmDef;
}

void TFtrSpace::GetSpV(const TRec& Rec, TIntFltKdV& SpV) const {
	QmAssertR(IsDefMode(), "Feature space not fully defined (no call to FinishUpdate())!");
	int Offset = 0;
	for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
		FtrExtV[FtrExtN]->AddSpV(Rec, SpV, Offset);
	}
}

void TFtrSpace::GetSpVV(const PRecSet& RecSet, TVec<TIntFltKdV>& SpVV) const {
    TEnv::Logger->OnStatusFmt("Creating feature vectors from %d records", RecSet->GetRecs());
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		if (RecN % 1000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
		SpVV.Add(TIntFltKdV()); GetSpV(RecSet->GetRec(RecN), SpVV.Last());
	}
}
	
void TFtrSpace::GetCentroidSpV(const PRecSet& RecSet, 
		TIntFltKdV& CentroidSpV, const bool& NormalizeP) const {

	for (int RecStrN = 0; RecStrN < RecSet->GetRecs(); RecStrN++) { 
		// get record sparse vector
		TIntFltKdV RecSpV; GetSpV(RecSet->GetRec(RecStrN), RecSpV);
		// add it to the centroid
		TIntFltKdV SumSpV;
		TLinAlg::AddVec(RecSpV, CentroidSpV, SumSpV);
		CentroidSpV = SumSpV;
	}
	if (NormalizeP) { TLinAlg::Normalize(CentroidSpV); }
}

void TFtrSpace::GetCentroidV(const PRecSet& RecSet,
		TFltV& CentroidV, const bool& NormalizeP) const {

	CentroidV.Gen(GetDim()); CentroidV.PutAll(0.0);
	for (int RecStrN = 0; RecStrN < RecSet->GetRecs(); RecStrN++) { 
		// get record sparse vector
		TIntFltKdV RecSpV; GetSpV(RecSet->GetRec(RecStrN), RecSpV);
		// add it to the centroid
		TLinAlg::AddVec(1.0, RecSpV, CentroidV, CentroidV);
	}
	if (NormalizeP) { TLinAlg::Normalize(CentroidV); }
}

int TFtrSpace::GetDim() const {
	QmAssertR(IsDefMode(), "Feature space not fully defined (no call to FinishUpdate())!");
	return Dim;
}

TStr TFtrSpace::GetFtr(const int& FtrN) const {
	QmAssertR(IsDefMode(), "Feature space not fully defined (no call to FinishUpdate())!");
	int SumDim = 0;
	for (int DimN = 0; DimN < DimV.Len(); DimN++) {
		SumDim += DimV[DimN];
		if (SumDim > FtrN) { 
			const int LocalFtrN = FtrN - (SumDim - DimV[DimN]);
			return FtrExtV[DimN]->GetFtr(LocalFtrN);
		}
	}
	throw TQmExcept::New("Feature number out of bounds!");
	return TStr();
}

namespace TFtrExts {
    
///////////////////////////////////////////////
// Random Feature Extractor
TRandom::TRandom(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed): 
    TFtrExt(Base, JoinSeqV), Rnd(RndSeed) { }

TRandom::TRandom(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
  TFtrExt(Base, ParamVal), Rnd(ParamVal->GetObjInt("seed", 0)) { }
    
PFtrExt TRandom::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& RndSeed) {
	return new TRandom(Base, TJoinSeqV::GetV(TJoinSeq(Store->GetStoreId())), RndSeed); 
}

PFtrExt TRandom::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& RndSeed) { 
	return new TRandom(Base, TJoinSeqV::GetV(JoinSeq), RndSeed); 
}

PFtrExt TRandom::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed) { 
	return new TRandom(Base, JoinSeqV, RndSeed);
}

PFtrExt TRandom::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TRandom(Base, ParamVal); 
}    

void TRandom::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	SpV.Add(TIntFltKd(Offset, Rnd.GetUniDev())); Offset++;
}

void TRandom::ExtractFltV(const TRec& FtrRec, TFltV& FltV) const {
	FltV.Add(Rnd.GetUniDev());
}

///////////////////////////////////////////////
// Numeric Feature Extractor
double TNumeric::_GetVal(const PRecSet& FtrRecSet) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRecSet->GetStoreId() == FtrStore->GetStoreId());
	// go over all the records
	double FieldVal = 0.0;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const uint64 RecId = FtrRecSet->GetRecId(RecN);
		// extract feature value
		if (FieldDesc.IsInt()) {
			FieldVal += (double)FtrStore->GetFieldInt(RecId, FieldId);
		} else if (FieldDesc.IsFlt()) {
			FieldVal += FtrStore->GetFieldFlt(RecId, FieldId);
		} else if (FieldDesc.IsUInt64()) {
			FieldVal += (double)FtrStore->GetFieldUInt64(RecId, FieldId);
		} else if (FieldDesc.IsBool()) {
			FieldVal +=  FtrStore->GetFieldBool(RecId, FieldId) ? 1.0 : 0.0;
		} else {
			throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
				" not supported by Numeric Feature Extractor!");
		}
	}
	return FieldVal;
}

double TNumeric::_GetVal(const TRec& FtrRec) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	// extract feature value
	if (FieldDesc.IsInt()) {
		return (double)FtrRec.GetFieldInt(FieldId);
	} else if (FieldDesc.IsFlt()) {
		return FtrRec.GetFieldFlt(FieldId);
	} else if (FieldDesc.IsUInt64()) {
		return (double)FtrRec.GetFieldUInt64(FieldId);
	} else if (FieldDesc.IsBool()) {
		return FtrRec.GetFieldBool(FieldId) ? 1.0 : 0.0;
	}
	throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
		" not supported by Numeric Feature Extractor!");
}

double TNumeric::GetVal(const TRec& Rec) const {
	Assert(IsStartStore(Rec.GetStoreId()));
	if (!IsJoin(Rec.GetStoreId())) {
		if (Rec.IsByRef()) {
			return _GetVal(Rec.ToRecSet());
		} else {
			return _GetVal(Rec);
		}
	} else {
		PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
		return _GetVal(RecSet);
	}
}

TNumeric::TNumeric(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
    const int& _FieldId, const bool& NormalizeP):
        TFtrExt(Base, JoinSeqV), FtrGen(NormalizeP), FieldId(_FieldId), 
        FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)) { }

TNumeric::TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
        TFtrExt(Base, ParamVal), FtrGen(ParamVal->GetObjBool("normalize", true)) {
    
    TStr FieldNm = ParamVal->GetObjStr("field");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
}

PFtrExt TNumeric::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const bool& NormalizeP) { 

    return new TNumeric(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId, NormalizeP); 
}

PFtrExt TNumeric::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const bool& NormalizeP) { 
    
    return new TNumeric(Base, TJoinSeqV::GetV(JoinSeq), FieldId, NormalizeP); 
}

PFtrExt TNumeric::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const bool& NormalizeP) { 
    
    return new TNumeric(Base, JoinSeqV, FieldId, NormalizeP); 
}

PFtrExt TNumeric::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TNumeric(Base, ParamVal);
}

void TNumeric::Update(const TRec& Rec) {
	FtrGen.Update(GetVal(Rec));
}

void TNumeric::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
	FtrGen.AddFtr(GetVal(Rec), SpV, Offset);
}

void TNumeric::ExtractFltV(const TRec& Rec, TFltV& FltV) const {
	FltV.Add(GetVal(Rec));
}

///////////////////////////////////////////////
// Categorical Feature Extractor
TStr TCategorical::_GetVal(const TRec& FtrRec) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	// separate case when record passed by reference or value
	if (FieldDesc.IsStr()) {
		return FtrRec.IsByVal() ? FtrRec.GetFieldStr(FieldId) :
			FtrStore->GetFieldStr(FtrRec.GetRecId(), FieldId);
	} else if (FieldDesc.IsInt()) {
		const int FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldInt(FieldId) :
			FtrStore->GetFieldInt(FtrRec.GetRecId(), FieldId);
		return TInt::GetStr(FieldVal);
	} else if (FieldDesc.IsUInt64()) {
		const uint64 FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldUInt64(FieldId) :
			FtrStore->GetFieldUInt64(FtrRec.GetRecId(), FieldId);
		return TUInt64::GetStr(FieldVal);
	} else if (FieldDesc.IsBool()) {
		const bool FieldVal = FtrRec.IsByVal() ? 
			FtrRec.GetFieldBool(FieldId) :
			FtrStore->GetFieldBool(FtrRec.GetRecId(), FieldId);
		return FieldVal ? "Yes" : "No";
	} else {
    	throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
        	" not supported by Categorical Feature Extractor!");
    }
}

TStr TCategorical::GetVal(const TRec& Rec) const {
	Assert(IsStartStore(Rec.GetStoreId()));
	if (!IsJoin(Rec.GetStoreId())) {
		return _GetVal(Rec);
	} else {
		TRec JoinRec = DoSingleJoin(Rec);
		return _GetVal(JoinRec);
	}
}

TCategorical::TCategorical(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId): 
    TFtrExt(Base, JoinSeqV), FieldId(_FieldId), FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)) { }

TCategorical::TCategorical(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {
    TStr FieldNm = ParamVal->GetObjStr("field");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
}

PFtrExt TCategorical::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId) { 
    return new TCategorical(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId); 
}

PFtrExt TCategorical::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId) { 
	return new TCategorical(Base, TJoinSeqV::GetV(JoinSeq), FieldId); 
}

PFtrExt TCategorical::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId) { 
	return new TCategorical(Base, JoinSeqV, FieldId); 
}

PFtrExt TCategorical::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TCategorical(Base, ParamVal);
}

void TCategorical::Update(const TRec& Rec) {
	FtrGen.Update(GetVal(Rec));
}

void TCategorical::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
	FtrGen.AddFtr(GetVal(Rec), SpV, Offset);
}

void TCategorical::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
	StrV.Add(GetVal(Rec));
}

///////////////////////////////////////////////
// Multinomial Feature Extractor
void TMultinomial::ParseDate(const TTm& Tm, TStrV& StrV) const {
	TSecTm SecTm = Tm.GetSecTm();
	StrV.Add(SecTm.GetDtYmdStr());
	StrV.Add(SecTm.GetMonthNm());
	StrV.Add(TInt::GetStr(SecTm.GetDayN()));
	StrV.Add(SecTm.GetDayOfWeekNm());
	StrV.Add(SecTm.GetDayPart());
}

void TMultinomial::_GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRecSet->GetStoreId() == FtrStore->GetStoreId());
	// go over all the records
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const uint64 RecId = FtrRecSet->GetRecId(RecN);
		// extract feature value
		if (FtrStore->IsFieldNull(RecId, FieldId)) {
			// do nothing
		} else if (FieldDesc.IsStr()) {
			StrV.Add(FtrStore->GetFieldStr(RecId, FieldId));
		} else if (FieldDesc.IsStrV()) {
			TStrV RecStrV; FtrStore->GetFieldStrV(RecId, FieldId, RecStrV);
			StrV.AddV(RecStrV);
		} else if (FieldDesc.IsInt()) {
			StrV.Add(TInt::GetStr(FtrStore->GetFieldInt(RecId, FieldId)));
		} else if (FieldDesc.IsIntV()) {
			TIntV RecIntV; FtrStore->GetFieldIntV(RecId, FieldId, RecIntV);
			for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
				StrV.Add(RecIntV[RecIntN].GetStr());
			}
		} else if (FieldDesc.IsUInt64()) {
			StrV.Add(TUInt64::GetStr(FtrStore->GetFieldUInt64(RecId, FieldId)));
		} else if (FieldDesc.IsBool()) {
			StrV.Add(FtrStore->GetFieldBool(RecId, FieldId) ? "Yes" : "No");
		} else if (FieldDesc.IsTm()) {
			TTm FieldTm; FtrStore->GetFieldTm(RecId, FieldId, FieldTm);
			ParseDate(FieldTm, StrV);
		} else {
			throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
				" not supported by Multinomial Feature Extractor!");
		}
	}
}

void TMultinomial::_GetVal(const TRec& FtrRec, TStrV& StrV) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	// extract feature value
	if (FtrRec.IsFieldNull(FieldId)) {
		// do nothing
	} else if (FieldDesc.IsStr()) {
		StrV.Add(FtrRec.GetFieldStr(FieldId));
	} else if (FieldDesc.IsStrV()) {
		TStrV RecStrV; FtrRec.GetFieldStrV(FieldId, RecStrV);
		StrV.AddV(RecStrV);
	} else if (FieldDesc.IsInt()) {
		StrV.Add(TInt::GetStr(FtrRec.GetFieldInt(FieldId)));
	} else if (FieldDesc.IsIntV()) {
		TIntV RecIntV; FtrRec.GetFieldIntV(FieldId, RecIntV);
		for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
			StrV.Add(RecIntV[RecIntN].GetStr());
		}
	} else if (FieldDesc.IsUInt64()) {
		StrV.Add(TUInt64::GetStr(FtrRec.GetFieldUInt64(FieldId)));
	} else if (FieldDesc.IsBool()) {
		StrV.Add(FtrRec.GetFieldBool(FieldId) ? "Yes" : "No");
	} else if (FieldDesc.IsTm()) {
		TTm FieldTm; FtrRec.GetFieldTm(FieldId, FieldTm);
		ParseDate(FieldTm, StrV);
	} else {
    	throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
        	" not supported by Multinomial Feature Extractor!");
    }
}

void TMultinomial::GetVal(const TRec& Rec, TStrV& StrV) const {
	Assert(IsStartStore(Rec.GetStoreId()));
	if (!IsJoin(Rec.GetStoreId())) {
		if (Rec.IsByRef()) {
			_GetVal(Rec.ToRecSet(), StrV);
		} else {
			_GetVal(Rec, StrV);
		}
	} else {
		PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
		_GetVal(RecSet, StrV);
	}
}

TMultinomial::TMultinomial(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId): 
    TFtrExt(Base, JoinSeqV), FieldId(_FieldId), FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)) { }
    
TMultinomial::TMultinomial(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {
    TStr FieldNm = ParamVal->GetObjStr("field");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
}

PFtrExt TMultinomial::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId) {
    return new TMultinomial(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId); 
}

PFtrExt TMultinomial::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId) { 
    return new TMultinomial(Base, TJoinSeqV::GetV(JoinSeq), FieldId); 
}

PFtrExt TMultinomial::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId) { 
    return new TMultinomial(Base, JoinSeqV, FieldId); 
}

PFtrExt TMultinomial::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TMultinomial(Base, ParamVal);
}

void TMultinomial::Update(const TRec& Rec) {
	TStrV StrV; GetVal(Rec, StrV);
	FtrGen.Update(StrV);
}

void TMultinomial::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
	TStrV StrV; GetVal(Rec, StrV);
	FtrGen.AddFtr(StrV, SpV, Offset);
}

void TMultinomial::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
	GetVal(Rec, StrV);
}

void TMultinomial::ExtractTmV(const TRec& Rec, TTmV& TmV) const {
	// assert store ...
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(Rec.GetStoreId() == FtrStore->GetStoreId());
	// ... and field description
	const TFieldDesc& FieldDesc = FtrStore->GetFieldDesc(FieldId);
	// check if start store valid
	Assert(IsStartStore(Rec.GetStoreId()));
	if (FieldDesc.IsTm()) {
		if (!IsJoin(Rec.GetStoreId())) {
			TTm FieldTm;
			if (Rec.IsByRef()) {
				FtrStore->GetFieldTm(Rec.GetRecId(), FieldId, FieldTm);
			} else {
				Rec.GetFieldTm(FieldId, FieldTm);
			}
			TmV.Add(FieldTm);
		} else {
			PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
			for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
				TTm FieldTm; 
				FtrStore->GetFieldTm(Rec.GetRecId(), FieldId, FieldTm);				
				TmV.Add(FieldTm);
			}
		}
	} else {
		throw TQmExcept::New("Expected TTm type, but found " + FieldDesc.GetFieldTypeStr());
	}
}

///////////////////////////////////////////////
// Bag-of-words Feature Extractor
void TBagOfWords::_GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRecSet->GetStoreId() == FtrStore->GetStoreId());
	// go over all the recordes
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const uint64 RecId = FtrRecSet->GetRecId(RecN);
		// extract feature value
		if (FtrStore->IsFieldNull(RecId, FieldId)) {
			// do nothing
		} else if (FieldDesc.IsStr()) {
			StrV.Add(FtrStore->GetFieldStr(RecId, FieldId));
		} else if (FieldDesc.IsStrV()) {
			TStrV RecStrV; FtrStore->GetFieldStrV(RecId, FieldId, RecStrV);
			StrV.AddV(RecStrV);
		} else {
			throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
				" not supported by Bag-Of-Words Feature Extractor!");
		}
	}
}

void TBagOfWords::_GetVal(const TRec& FtrRec, TStrV& StrV) const {
	// assert store
	TWPt<TStore> FtrStore = GetFtrStore();
	Assert(FtrRec.GetStoreId() == FtrStore->GetStoreId());
	// extract feature value
	if (FtrRec.IsFieldNull(FieldId)) {
		// do nothing
	} else if (FieldDesc.IsStr()) {
		StrV.Add(FtrRec.GetFieldStr(FieldId));
	} else if (FieldDesc.IsStrV()) {
		TStrV RecStrV; FtrRec.GetFieldStrV(FieldId, RecStrV);
		StrV.AddV(RecStrV);
	} else {
        throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
            " not supported by Bag-Of-Words Feature Extractor!");
    }
}

void TBagOfWords::GetVal(const TRec& Rec, TStrV& StrV) const {
	Assert(IsStartStore(Rec.GetStoreId()));
	if (!IsJoin(Rec.GetStoreId())) {
		if (Rec.IsByRef()) {
			_GetVal(Rec.ToRecSet(), StrV);
		} else {
			_GetVal(Rec, StrV);
		}
	} else {
		PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
		_GetVal(RecSet, StrV);
	}
}

TBagOfWords::TBagOfWords(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId,
    const TBagOfWordsMode& _Mode, const PSwSet& SwSet, const PStemmer& Stemmer): 
        TFtrExt(Base, JoinSeqV), FtrGen(SwSet, Stemmer), FieldId(_FieldId), 
        FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)), Mode(_Mode) { }

TBagOfWords::TBagOfWords(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
        TFtrExt(Base, ParamVal), FtrGen(ParseSwSet(ParamVal), ParseStemmer(ParamVal, false)) {
    
    // get field
    TStr FieldNm = ParamVal->GetObjStr("field");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId, 
        const TBagOfWordsMode& Mode, const PSwSet& SwSet, const PStemmer& Stemmer) { 
    
    return new TBagOfWords(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId, Mode, SwSet, Stemmer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId, 
        const TBagOfWordsMode& Mode, const PSwSet& SwSet, const PStemmer& Stemmer) { 
		
	return new TBagOfWords(Base, TJoinSeqV::GetV(JoinSeq), FieldId, Mode, SwSet, Stemmer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId, 
        const TBagOfWordsMode& Mode, const PSwSet& SwSet, const PStemmer& Stemmer) {
    
    return new TBagOfWords(Base, JoinSeqV, FieldId, Mode, SwSet, Stemmer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TBagOfWords(Base, ParamVal);
}

void TBagOfWords::Update(const TRec& Rec) {
	// get all instances
	TStrV RecStrV; GetVal(Rec, RecStrV);
	if (Mode == bowmConcat) {
		// merge into one document
		FtrGen.Update(TStr::GetStr(RecStrV, "\n"));
	} else if (Mode == bowmCentroid) {
		// threat each as a separate document
		for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
			FtrGen.Update(RecStrV[RecStrN]); }
	} else {
		throw TQmExcept::New("Unknown tokenizer mode for handling multiple instances");
	}
}

void TBagOfWords::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
	// get all instances
	TStrV RecStrV; GetVal(Rec, RecStrV);
	if (Mode == bowmConcat) {
		// merge into one document
		FtrGen.AddFtr(TStr::GetStr(RecStrV, "\n"), SpV, Offset);
	} else if (Mode == bowmCentroid) {
		// threat each as a string as a separate document
		TIntFltKdV CentroidSpV;
		for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
			// get record sparse vector
			TIntFltKdV RecSpV; FtrGen.AddFtr(RecStrV[RecStrN], RecSpV);
			// add it to the centroid
			TIntFltKdV SumSpV;
			TLinAlg::AddVec(RecSpV, CentroidSpV, SumSpV);
			CentroidSpV = SumSpV;
		}
		// normalize centroid
		TLinAlg::Normalize(CentroidSpV);
		// add to the input sparse vector
		for (int SpN = 0; SpN < CentroidSpV.Len(); SpN++) {
			const int Id = CentroidSpV[SpN].Key;
			const double Wgt = CentroidSpV[SpN].Dat;
			SpV.Add(TIntFltKd(Offset + Id, Wgt));
		}
		// move offset for the number of tokens in feature generator
		Offset += FtrGen.GetVals();
	} else {
		throw TQmExcept::New("Unknown tokenizer mode for handling multiple instances");
	}

}

void TBagOfWords::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
	TStrV RecStrV; GetVal(Rec, RecStrV);
	for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
		FtrGen.GetTokenV(RecStrV[RecStrN], StrV);
	}
}

PSwSet TBagOfWords::GetSwSet(const TStr& SwStr) {
	if (SwStr == "en") {
		return TSwSet::New(swstEn523);
	} else if (SwStr == "si") { 
		return TSwSet::New(swstSiIsoCe);
	} else if (SwStr == "es") { 
		return TSwSet::New(swstEs);
	} else if (SwStr == "de") { 
		return TSwSet::New(swstGe);
	}
	throw TQmExcept::New("Unknown stop-word set '" + SwStr + "'");
}

void TBagOfWords::AddWords(const PSwSet& SwSet, const PJsonVal& WordsVal) {
	for (int WordN = 0; WordN < WordsVal->GetArrVals(); WordN++) {
		PJsonVal WordVal = WordsVal->GetArrVal(WordN);
		QmAssert(WordVal->IsStr());
		SwSet->AddWord(WordVal->GetStr().GetUc());
	}
}

PSwSet TBagOfWords::ParseSwSet(const PJsonVal& ParamVal) {
	// read stop words (default is no stop words)
	PSwSet SwSet = TSwSet::New(swstNone);
	if (ParamVal->IsObjKey("stopwords")) { 
		PJsonVal SwVal = ParamVal->GetObjKey("stopwords");
		if (SwVal->IsStr()) {
			SwSet = GetSwSet(SwVal->GetStr());
		} else if (SwVal->IsArr()) {
			AddWords(SwSet, SwVal);
		} else if (SwVal->IsObj()) {
			// load predefined set
			if (SwVal->IsObjKey("language")) {
				PJsonVal SwTypeVal = SwVal->GetObjKey("language");
				QmAssert(SwTypeVal->IsStr());
				SwSet = GetSwSet(SwTypeVal->GetStr());
			}
			// add any additional words
			if (SwVal->IsObjKey("words")) {
				PJsonVal SwWordsVal = SwVal->GetObjKey("words");
				QmAssert(SwWordsVal->IsArr());
				AddWords(SwSet, SwWordsVal);
			}
		} else {
			throw TQmExcept::New("Unknown stop-word definition '" + SwVal->SaveStr() + "'");
		}
	} else {
		// default is English
		SwSet = TSwSet::New(swstEn523);
	}   
    return SwSet;
}

PStemmer TBagOfWords::ParseStemmer(const PJsonVal& ParamVal, const bool& RealWordP) {
    if (ParamVal->IsObjKey("stemmer")) {
        PJsonVal StemmerVal = ParamVal->GetObjKey("stemmer");
        if (StemmerVal->IsBool()) {
            return TStemmer::New(StemmerVal->GetBool() ? stmtPorter : stmtNone, RealWordP);
        } else if (StemmerVal->IsObj()) {
            TStr StemmerType = StemmerVal->GetObjStr("type", "none");
            const bool RealWordP = StemmerVal->GetObjBool("realWord", RealWordP);
            return TStemmer::New((StemmerType == "porter") ? stmtPorter : stmtNone, RealWordP);            
        }
    }
    // default no stemmer
    return TStemmer::New(stmtNone, false);
}

///////////////////////////////////////////////
// QMiner-Feature-Extractor-Join 
void TJoin::Def() {
	QmAssertR(BucketSize > 0, "Sample parameter must be > 0!");
	// get the maximal record id
	uint64 MxRecId = 0; Dim = 0;
	TWPt<TStore> FtrStore = GetFtrStore();
	PStoreIter Iter = FtrStore->GetIter();
	while (Iter->Next()) {
		const uint64 RecId = Iter->GetRecId() / BucketSize;
		if (RecId > MxRecId) { MxRecId = RecId; }
	}
	// use that as the ceiling for dimensionality
	if (MxRecId > 0) { Dim = (int)(MxRecId + 1); }
}

TJoin::TJoin(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _BucketSize): 
    TFtrExt(Base, JoinSeqV), BucketSize(_BucketSize) { Def(); }

TJoin::TJoin(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
    TFtrExt(Base, ParamVal), BucketSize(ParamVal->GetObjInt("bucketSize", 1)) { Def(); }

PFtrExt TJoin::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& BucketSize) { 
    return new TJoin(Base, TJoinSeqV::GetV(JoinSeq), BucketSize); 
}

PFtrExt TJoin::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& BucketSize) {
    return new TJoin(Base, JoinSeqV, BucketSize); 
}

PFtrExt TJoin::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TJoin(Base, ParamVal);
}

void TJoin::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	// do the joins
	Assert(IsStartStore(FtrRec.GetStoreId()));
	PRecSet RecSet = FtrRec.DoJoin(GetBase(), GetJoinIdV(FtrRec.GetStoreId())); 
	// sort by ids, to make it a valid sparse vector
	RecSet->SortById(true);
	// generate a sparse vector out of join
	const int FirstSpN = SpV.Len(); 
	int SampleId = 0; double NormSq = 0.0, SampleFq = 0.0; 
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		const int Id = (int)RecSet->GetRecId(RecN) / BucketSize;
		const double Fq = (double)RecSet->GetRecFq(RecN);
		if (Id < Dim) { 
			if (Id == SampleId) {
				// repeat of the previous Id, just update the frequency
				SampleFq += Fq;
			} else {
				// new id, add previous one to the feature vector
				NormSq += TMath::Sqr(SampleFq);
				SpV.Add(TIntFltKd(Offset + SampleId, SampleFq)); 
				// and update the id-tracker
				SampleId = Id; SampleFq = Fq;
			}
		} else {
			break; // we are out of defined feature space
		}
	}
	// add last element if nonzero
	if (SampleFq > 0.0) {
		NormSq += TMath::Sqr(SampleFq);
		SpV.Add(TIntFltKd(Offset + SampleId, SampleFq)); 
	}
	// normalize, as returning unit norm feature vectors is polite
	if (NormSq > 0.0) { 
		const double InvNorm = 1.0 / TMath::Sqrt(NormSq);
		for (int SpN = FirstSpN; SpN < SpV.Len(); SpN++) {
			SpV[SpN].Dat *= InvNorm;
		}
	}
	// and attach to the provided vector
	Offset += Dim;
}

void TJoin::ExtractStrV(const TRec& FtrRec, TStrV& StrV) const {
	// do the joins
	Assert(IsStartStore(FtrRec.GetStoreId()));
	PRecSet RecSet = FtrRec.DoJoin(GetBase(), GetJoinIdV(FtrRec.GetStoreId())); 
	// extract record names
	TWPt<TStore> FtrStore = GetFtrStore();
	for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
		StrV.Add(FtrStore->GetRecNm(RecSet->GetRecId(RecN)));
	}	
}

///////////////////////////////////////////////
// QMiner-Feature-Extractor-Pair 
void TPair::GetFtrIdV_Update(const TRec& _FtrRec, const PFtrExt& FtrExt, TIntV& FtrIdV) {
	// do the joins
	Assert(IsStartStore(_FtrRec.GetStoreId()));
	PRecSet FtrRecSet = _FtrRec.DoJoin(GetBase(), GetJoinSeq(_FtrRec.GetStoreId())); 
	// extract string features for each record
	TIntSet FtrIdSet;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV; FtrExt->ExtractStrV(FtrRec, FtrValV);
		if (FtrValV.Empty()) { FtrValV.Add("[empty]"); }
		// load them into the vocabulary
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			// get feature id
			const int FtrId = FtrValH.AddKey(FtrValV[FtrValN]);
			// count the feature
			FtrValH[FtrId]++;
			// and remember the id
			FtrIdSet.AddKey(FtrId);
		}
	}
	// return feature id vector
	FtrIdSet.GetKeyV(FtrIdV);
}

void TPair::GetFtrIdV_RdOnly(const TRec& _FtrRec, const PFtrExt& FtrExt, TIntV& FtrIdV) const {
	// do the joins
	Assert(IsStartStore(_FtrRec.GetStoreId()));
	PRecSet FtrRecSet = _FtrRec.DoJoin(GetBase(), GetJoinSeq(_FtrRec.GetStoreId())); 
	// extract string features for each record
	TIntSet FtrIdSet;
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV; FtrExt->ExtractStrV(FtrRec, FtrValV);
		if (FtrValV.Empty()) { FtrValV.Add("[empty]"); }
		// load them into the vocabulary
		for (int FtrValN = 0; FtrValN < FtrValV.Len(); FtrValN++) {
			// get feature id
			const TStr& FtrVal = FtrValV[FtrValN];
			const int FtrId = FtrValH.GetKeyId(FtrVal);
			// if feature exists, pass on
			if (FtrId != -1) { FtrIdSet.AddKey(FtrId); }
		}
	}
	// return feature id vector
	FtrIdSet.GetKeyV(FtrIdV);
}

TPair::TPair(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
    const PFtrExt& _FtrExt1, const PFtrExt& _FtrExt2): TFtrExt(Base, JoinSeqV), 
        FtrExt1(_FtrExt1), FtrExt2(_FtrExt2) { }

TPair::TPair(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {   
    PJsonVal FtrExt1Val = ParamVal->GetObjKey("first");
    PJsonVal FtrExt2Val = ParamVal->GetObjKey("second");
    FtrExt1 = TFtrExt::New(Base, FtrExt1Val->GetObjStr("type"), FtrExt1Val);
    FtrExt2 = TFtrExt::New(Base, FtrExt2Val->GetObjStr("type"), FtrExt2Val);    
}

PFtrExt TPair::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2) { 
    
    return new TPair(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FtrExt1, FtrExt2); 
}

PFtrExt TPair::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2) { 
    
    return new TPair(Base, TJoinSeqV::GetV(JoinSeq), FtrExt1, FtrExt2); 
}

PFtrExt TPair::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2) { 
    
    return new TPair(Base, JoinSeqV, FtrExt1, FtrExt2); 
}

PFtrExt TPair::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TPair(Base, ParamVal);
}

TStr TPair::GetFtr(const int& FtrN) const {
	if (FtrIdPairH.IsKeyId(FtrN)) {
		const TIntPr& Pair = FtrIdPairH.GetKey(FtrN);
		TStr FtrVal1 = FtrValH.GetKey(Pair.Val1);
		TStr FtrVal2 = FtrValH.GetKey(Pair.Val2);
		return "[" + FtrVal1 + ", " + FtrVal2 + "]";
	}
	return "";
}

void TPair::Update(const TRec& FtrRec) {
	// extract feature values using each extractor
	TIntV FtrIdV1; GetFtrIdV_Update(FtrRec, FtrExt1, FtrIdV1);
	TIntV FtrIdV2; GetFtrIdV_Update(FtrRec, FtrExt2, FtrIdV2);
	// transform 
	for (int FtrIdN1 = 0; FtrIdN1 < FtrIdV1.Len(); FtrIdN1++) {
		const int FtrId1 = FtrIdV1[FtrIdN1];
		for (int FtrIdN2 = 0; FtrIdN2 < FtrIdV2.Len(); FtrIdN2++) {
			const int FtrId2 = FtrIdV2[FtrIdN2];
			// remember the pair
			FtrIdPairH.AddDat(TIntPr(FtrId1, FtrId2))++;
		}
	}
}

void TPair::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	// extract feature values using each extractor
	TIntV FtrIdV1; GetFtrIdV_RdOnly(FtrRec, FtrExt1, FtrIdV1);
	TIntV FtrIdV2; GetFtrIdV_RdOnly(FtrRec, FtrExt2, FtrIdV2);
	// transform
	TIntFltH PairIdWgtH;
	for (int FtrIdN1 = 0; FtrIdN1 < FtrIdV1.Len(); FtrIdN1++) {
		const int FtrId1 = FtrIdV1[FtrIdN1];
		for (int FtrIdN2 = 0; FtrIdN2 < FtrIdV2.Len(); FtrIdN2++) {
			const int FtrId2 = FtrIdV2[FtrIdN2];
			// get feature pair id
			const int PairId = FtrIdPairH.GetKeyId(TIntPr(FtrId1, FtrId2));
			// remember the pair
			if (PairId != -1) { PairIdWgtH.AddDat(PairId) += 1.0; }
		}
	}
	// transform to sparse vector
	TIntFltKdV PairSpV; PairIdWgtH.GetKeyDatKdV(PairSpV); PairSpV.Sort();
	// normalize
	TLinAlg::Normalize(PairSpV);
	// add to the existing sparse vector
	for (int PairSpN = 0; PairSpN < PairSpV.Len(); PairSpN++) {
		const TIntFltKd PairSp = PairSpV[PairSpN];
		SpV.Add(TIntFltKd(PairSp.Key + Offset, PairSp.Dat));
	}
	// increase the offset by the dimension
    Offset += GetDim();
}

void TPair::ExtractStrV(const TRec& _FtrRec, TStrV& StrV) const {
	// do the joins
	Assert(IsStartStore(_FtrRec.GetStoreId()));
	PRecSet FtrRecSet = _FtrRec.DoJoin(GetBase(), GetJoinSeq(_FtrRec.GetStoreId())); 
	// extract features for each record
	for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
		const TRec& FtrRec = FtrRecSet->GetRec(RecN);
		TStrV FtrValV1; FtrExt1->ExtractStrV(FtrRec, FtrValV1);
		TStrV FtrValV2; FtrExt2->ExtractStrV(FtrRec, FtrValV2);
		// put placeholder when empty
		if (FtrValV1.Empty()) { FtrValV1.Add("[empty]"); }
		if (FtrValV2.Empty()) { FtrValV2.Add("[empty]"); }
		// generate combined features
		for (int FtrValN1 = 0; FtrValN1 < FtrValV1.Len(); FtrValN1++) {
			const TStr& FtrVal1 = FtrValV1[FtrValN1];
			for (int FtrValN2 = 0; FtrValN2 < FtrValV2.Len(); FtrValN2++) {
				const TStr& FtrVal2 = FtrValV2[FtrValN2];
				StrV.Add("[" + FtrVal1 + ", " + FtrVal2 + "]");
			}
		}
	}
}

}

}
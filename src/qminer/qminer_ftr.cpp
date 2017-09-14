/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "qminer_ftr.h"

namespace TQm {

///////////////////////////////////////////////
// QMiner-Feature-Extractor
TFunRouter<TFtrExt::TNewF> TFtrExt::NewRouter;
TFunRouter<TFtrExt::TLoadF> TFtrExt::LoadRouter;

void TFtrExt::Init() {
    Register<TFtrExts::TRandom>();
    Register<TFtrExts::TConstant>();
    Register<TFtrExts::TNumeric>();
    Register<TFtrExts::TNumSpV>();
    Register<TFtrExts::TCategorical>();
    Register<TFtrExts::TMultinomial>();
    Register<TFtrExts::TBagOfWords>();
    Register<TFtrExts::TJoin>();
    Register<TFtrExts::TPair>();
    Register<TFtrExts::TDateWnd>();
}
    
TRec TFtrExt::DoSingleJoin(const TRec& Rec) const {
    QmAssertR(IsStartStore(Rec.GetStoreId()), "Start store not supported!");
    return Rec.DoSingleJoin(Base, GetJoinIdV(Rec.GetStoreId()));
}

TFtrExt::TFtrExt(const TWPt<TBase>& _Base, const TJoinSeqV& JoinSeqV): Base(_Base) {
    QmAssertR(!JoinSeqV.Empty(), "At least one join sequence must be supplied!");
    FtrStore = JoinSeqV[0].GetEndStore(Base);
    // get mapping from stores to join sequence
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
        JoinSeqH.AddDat(FtrStore->GetStoreId(), TJoinSeq(FtrStore->GetStoreId()));
    } else if (SourceVal->IsObj()) {
        // get store
        TStr StoreNm = SourceVal->GetObjStr("store");
        FtrStore = Base->GetStoreByStoreNm(StoreNm);       
        // get joins if any given
        if (SourceVal->IsObjKey("join")) {
            TJoinSeq JoinSeq = TJoinSeq(Base, FtrStore->GetStoreId(), SourceVal->GetObjKey("join"));
            FtrStore = JoinSeq.GetEndStore(Base);
            JoinSeqH.AddDat(JoinSeq.GetStartStoreId(), JoinSeq);
        } else {
            JoinSeqH.AddDat(FtrStore->GetStoreId(), TJoinSeq(FtrStore->GetStoreId()));            
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

TFtrExt::TFtrExt(const TWPt<TBase>& _Base, TSIn& SIn): Base(_Base) {
    JoinSeqH.Load(SIn);
    FtrStore = TStore::LoadById(Base, SIn);
}

PFtrExt TFtrExt::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
    return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

PFtrExt TFtrExt::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    TStr TypeNm(SIn);
    return LoadRouter.Fun(TypeNm)(Base, SIn);
}

void TFtrExt::Save(TSOut& SOut) const { 
    JoinSeqH.Save(SOut); 
    FtrStore->SaveId(SOut);
}

void TFtrExt::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    // get sparse vector
    TIntFltKdV SpV; AddSpV(Rec, SpV, Offset);
    // add to full vector
    for (int SpN = 0; SpN < SpV.Len(); SpN++) {
        FullV[SpV[SpN].Key] = SpV[SpN].Dat;
    }
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

PJsonVal TFtrExt::InvertFullV(const TFltV& FtrV, const int& Offset) const {
    throw TExcept::New("TFtrExt::InvertFullV: not implemented for " + GetNm());
}

PJsonVal TFtrExt::InvertFtr(const PJsonVal& FtrVal) const {
    throw TExcept::New("TFtrExt::InvertFtr: not implemented for " + GetNm());
}

PJsonVal TFtrExt::GetFtrRange() const {
    throw TExcept::New("TFtrExt::GetFtrRange: not implemented for " + GetNm());
}

///////////////////////////////////////////////
// QMiner-Feature-Space
void TFtrSpace::Init() {
    // we start with empty space
    DimV.Gen(FtrExtV.Len(), 0); Dim = 0;
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        const PFtrExt& FtrExt = FtrExtV[FtrExtN];
        // get dimensionality
        const int FtrExtDim = FtrExt->GetDim();
        // update counts
        DimV.Add(FtrExtDim); Dim += FtrExtDim;
    }   
}
    
TFtrSpace::TFtrSpace(const TWPt<TBase>& _Base, const PFtrExt& FtrExt): 
    Base(_Base), FtrExtV(TFtrExtV::GetV(FtrExt)) { Init(); }

TFtrSpace::TFtrSpace(const TWPt<TBase>& _Base, const TFtrExtV& _FtrExtV): 
    Base(_Base), FtrExtV(_FtrExtV) { Init(); }

TFtrSpace::TFtrSpace(const TWPt<TBase>& _Base, const PJsonVal& ParamVal): Base(_Base) {
    if (ParamVal->IsArr()) {
        int Len = ParamVal->GetArrVals();
        FtrExtV.Gen(Len, 0);
        for (int ParamN = 0; ParamN < Len; ParamN++) {
            PJsonVal Param = ParamVal->GetArrVal(ParamN);
            TStr Type = Param->GetObjStr("type");
            FtrExtV.Add(TFtrExt::New(_Base, Type, Param));
        }
    }
    else {
        TStr Type = ParamVal->GetObjStr("type");
        FtrExtV.Add(TFtrExt::New(_Base, Type, ParamVal));
    }
    Init();
}

TFtrSpace::TFtrSpace(const TWPt<TBase>& _Base, TSIn& SIn): 
        Base(_Base), Dim(SIn), DimV(SIn), VarDimFtrExtNV(SIn) {

    const int FtrExts = TInt(SIn).Val;
    FtrExtV.Gen(FtrExts, 0);
    for (int FtrExtN = 0; FtrExtN < FtrExts; FtrExtN++) {
        PFtrExt FtrExt = TFtrExt::Load(Base, SIn);
        FtrExtV.Add(FtrExt);
    }
}

TPt<TFtrSpace> TFtrSpace::New(const TWPt<TBase>& Base, const PFtrExt& FtrExt) { 
    return new TFtrSpace(Base, FtrExt); 
}

TPt<TFtrSpace> TFtrSpace::New(const TWPt<TBase>& Base, const TFtrExtV& FtrExtV) { 
    return new TFtrSpace(Base, FtrExtV); 
}

TPt<TFtrSpace> TFtrSpace::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TFtrSpace(Base, ParamVal);
}

TPt<TFtrSpace> TFtrSpace::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TFtrSpace(Base, SIn); 
}

void TFtrSpace::Save(TSOut& SOut) const {
    Dim.Save(SOut); DimV.Save(SOut); VarDimFtrExtNV.Save(SOut);
    TInt(FtrExtV.Len()).Save(SOut);
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        FtrExtV[FtrExtN]->Save(SOut);
    }
}

void TFtrSpace::AddFtrExt(const PFtrExt& FtrExt) {
    const int FtrExtDim = FtrExt->GetDim();
    DimV.Add(FtrExtDim);
    Dim += FtrExtDim;
    FtrExtV.Add(FtrExt);
}

TStr TFtrSpace::GetNm() const {
    TChA NmChA = "Space:";
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        NmChA += "\n - ";
        NmChA += FtrExtV[FtrExtN]->GetNm();
    }
    return NmChA;
}

void TFtrSpace::Clr() {
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        FtrExtV[FtrExtN]->Clr();
    }
    Init();
}

bool TFtrSpace::Update(const TRec& Rec) {
    bool UpdateDimP = false;
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        const PFtrExt& FtrExt = FtrExtV[FtrExtN];       
        const bool FtrExtUpdateDimP = FtrExt->Update(Rec);
        if (FtrExtUpdateDimP) {
            // get new dimensionality
            const int NewDim = FtrExt->GetDim();
            // update the dimensionality sum (existing - old + new)
            Dim = Dim - DimV[FtrExtN] + NewDim;
            // update the feature space dimensionality
            DimV[FtrExtN] = NewDim;
            // remember we did this
            UpdateDimP = true;
        }
    }
    return UpdateDimP;
}

bool TFtrSpace::Update(const PRecSet& RecSet) {
    TEnv::Logger->OnStatusFmt("Updating feature space with %d records", RecSet->GetRecs());
    bool UpdateDimP = false;
    for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
        if (RecN % 10000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
        // update according to the record
        const bool RecUpdateDimP = Update(RecSet->GetRec(RecN));
        // check if we did a dimensionality update
        UpdateDimP = UpdateDimP || RecUpdateDimP;
    }
    return UpdateDimP;
}

void TFtrSpace::GetSpV(const TRec& Rec, TIntFltKdV& SpV, const int& FtrExtN) const {
    int Offset = 0;
    SpV.Clr();
    if (FtrExtN < 0) {
        for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
            FtrExtV[FtrExtN]->AddSpV(Rec, SpV, Offset);
        }
    } else {
        EAssert(FtrExtN < FtrExtV.Len());
        FtrExtV[FtrExtN]->AddSpV(Rec, SpV, Offset);
    }
}

void TFtrSpace::GetFullV(const TRec& Rec, TFltV& FullV, const int& FtrExtN) const {
    // create empty full vector
    int Offset = 0;
    if (FtrExtN < 0) {
        FullV.Gen(GetDim()); FullV.PutAll(0.0);
        for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
            FtrExtV[FtrExtN]->AddFullV(Rec, FullV, Offset);
        }
    } else {
        EAssert(FtrExtN < FtrExtV.Len());
        FullV.Gen(FtrExtV[FtrExtN]->GetDim());
        FtrExtV[FtrExtN]->AddFullV(Rec, FullV, Offset);
    }
}

void TFtrSpace::GetSpVV(const PRecSet& RecSet, TVec<TIntFltKdV>& SpVV, const int& FtrExtN) const {
    TEnv::Logger->OnStatusFmt("Creating sparse feature vectors from %d records", RecSet->GetRecs());
    for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
        if (RecN % 10000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
        SpVV.Add(TIntFltKdV()); GetSpV(RecSet->GetRec(RecN), SpVV.Last(), FtrExtN);
    }
}

void TFtrSpace::GetFullVV(const PRecSet& RecSet, TVec<TFltV>& FullVV, const int& FtrExtN) const {
    TEnv::Logger->OnStatusFmt("Creating full feature vectors from %d records", RecSet->GetRecs());
    for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
        if (RecN % 10000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
        FullVV.Add(TFltV()); GetFullV(RecSet->GetRec(RecN), FullVV.Last(), FtrExtN);
    }
}

void TFtrSpace::GetFullVV(const PRecSet& RecSet, TFltVV& FullVV, const int& FtrExtN) const {
    TEnv::Logger->OnStatusFmt("Creating full feature vectors from %d records", RecSet->GetRecs());
    if (FtrExtN < 0) {
        FullVV.Gen(GetDim(), RecSet->GetRecs());
        TFltV Temp(GetDim());
        for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
            if (RecN % 10000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
            GetFullV(RecSet->GetRec(RecN), Temp);
            FullVV.SetCol(RecN, Temp);
        }
    } else {
        EAssert(FtrExtN < FtrExtV.Len());
        FullVV.Gen(FtrExtV[FtrExtN]->GetDim(), RecSet->GetRecs());
        TFltV Temp(FtrExtV[FtrExtN]->GetDim());
        for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
            if (RecN % 10000 == 0) { TEnv::Logger->OnStatusFmt("%d\r", RecN); }
            GetFullV(RecSet->GetRec(RecN), Temp, FtrExtN);
            FullVV.SetCol(RecN, Temp);
        }
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

double TFtrSpace::GetSingleFtr(const int& FtrExtN, const double& Val) const {
    printf("GetSingleFtr is DEPRECATED\n");
    const PFtrExt& FtrExt = FtrExtV[FtrExtN];
    return FtrExt->__GetVal(Val);
}

PJsonVal TFtrSpace::InvertFullV(const TFltV& FullV) const {
    PJsonVal Result = TJsonVal::NewArr();

    int Offset = 0;
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        const PFtrExt& FtrExt = FtrExtV[FtrExtN];

        const PJsonVal InvVal = FtrExt->InvertFullV(FullV, Offset);
        Result->AddToArr(InvVal);

        Offset += FtrExt->GetDim();
    }

    return Result;
}

PJsonVal TFtrSpace::InvertFtr(const int& FtrExtN, const PJsonVal& FtrVal) const {
    const PFtrExt& FtrExt = FtrExtV[FtrExtN];
    return FtrExt->InvertFtr(FtrVal);
}

PJsonVal TFtrSpace::GetFtrRange(const int& FtrExtN) const {
    return FtrExtV[FtrExtN]->GetFtrRange();
}

void TFtrSpace::ExtractStrV(const int& DimN, const PJsonVal& RecVal, TStrV &StrV) const {
    QmAssertR(DimN < DimV.Len(), "Dimension out of bounds!");
    PStore Store = FtrExtV[DimN]->GetFtrStore();
    TRec Rec(Store, RecVal);
    FtrExtV[DimN]->ExtractStrV(Rec, StrV);
}

int TFtrSpace::GetDim() const {
    return Dim;
}

TStr TFtrSpace::GetFtr(const int& FtrN) const {
    int SumDim = 0;
    for (int DimN = 0; DimN < DimV.Len(); DimN++) {
        SumDim += DimV[DimN];
        if (SumDim > FtrN) { 
            const int LocalFtrN = FtrN - (SumDim - DimV[DimN]);
            return FtrExtV[DimN]->GetFtr(LocalFtrN);
        }
    }
    throw TQmExcept::New("Feature number out of bounds: " + TInt::GetStr(FtrN) + "!");
    return TStr();
}

int TFtrSpace::GetFtrExts() const {
    return FtrExtV.Len();
}

PFtrExt TFtrSpace::GetFtrExt(const int& FtrExtN) const {
    QmAssertR(0 <= FtrExtN && FtrExtN < FtrExtV.Len(), "Invalid feature extractor index: " + TInt::GetStr(FtrExtN));
    return FtrExtV[FtrExtN];
}

int TFtrSpace::GetFtrExtDim(const int& FtrExtN) const {
    QmAssert(0 <= FtrExtN && FtrExtN < FtrExtV.Len());
    return DimV[FtrExtN];
}

int TFtrSpace::GetMnFtrN(const int& FtrExtN) const {
    QmAssert(0 <= FtrExtN && FtrExtN < FtrExtV.Len());
    int MnFtrN = 0, _FtrExtN = 0;
    while (_FtrExtN < FtrExtN) { 
        MnFtrN += DimV[_FtrExtN];
        _FtrExtN++;
    }
    return MnFtrN;
}

int TFtrSpace::GetMxFtrN(const int& FtrExtN) const {
    QmAssert(0 <= FtrExtN && FtrExtN < FtrExtV.Len());
    int MxFtrN = 0, _FtrExtN = 0;
    while (_FtrExtN <= FtrExtN) { 
        MxFtrN += DimV[_FtrExtN];
        _FtrExtN++;
    }
    return MxFtrN;
}

bool TFtrSpace::IsStartStore(const uint& StoreId) const {
    for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
        const PFtrExt& FtrExt = FtrExtV[FtrExtN];
        if (!FtrExt->IsStartStore(StoreId)) { return false; }
    }
    return true;
}

PBowDocBs TFtrSpace::MakeBowDocBs(const PRecSet& FtrRecSet) {
    // prepare documents
    PBowDocBs BowDocBs = TBowDocBs::New();
    for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) { 
        TStr DocNm =  FtrRecSet->GetRec(RecN).GetRecNm();
        TStrV WdStrV;
        for (int FtrExtN = 0; FtrExtN < FtrExtV.Len(); FtrExtN++) {
            FtrExtV[FtrExtN]->ExtractStrV(FtrRecSet->GetRec(RecN), WdStrV);
        }
        BowDocBs->AddDoc(DocNm, TStrV(), WdStrV);
    }
    return BowDocBs;
}

namespace TFtrExts {

///////////////////////////////////////////////
// Constant Feature Extractor
TConstant::TConstant(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
    TFtrExt(Base, ParamVal), Constant(ParamVal->GetObjNum("const", 1.0)) { }

TConstant:: TConstant(const TWPt<TBase>& Base, TSIn& SIn): TFtrExt(Base, SIn), Constant(SIn) { }

PFtrExt TConstant::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TConstant(Base, ParamVal); 
}

PFtrExt TConstant::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TConstant(Base, SIn);
}

void TConstant::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    Constant.Save(SOut);
}

void TConstant::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
    SpV.Add(TIntFltKd(Offset, Constant.Val)); Offset++;
}

void TConstant::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    FullV[Offset] = Constant.Val; Offset++;
}

void TConstant::ExtractFltV(const TRec& FtrRec, TFltV& FltV) const {
    FltV.Add(Constant.Val);
}
    
///////////////////////////////////////////////
// Random Feature Extractor
TRandom::TRandom(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed): 
    TFtrExt(Base, JoinSeqV), Rnd(RndSeed) { }

TRandom::TRandom(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
  TFtrExt(Base, ParamVal), Rnd(ParamVal->GetObjInt("seed", 0)) { }

TRandom:: TRandom(const TWPt<TBase>& Base, TSIn& SIn): TFtrExt(Base, SIn), Rnd(SIn) { }
    
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

PFtrExt TRandom::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TRandom(Base, SIn);
}

void TRandom::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    Rnd.Save(SOut);
}

void TRandom::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
    SpV.Add(TIntFltKd(Offset, Rnd.GetUniDev())); Offset++;
}

void TRandom::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    FullV[Offset] = Rnd.GetUniDev(); Offset++;
}

void TRandom::ExtractFltV(const TRec& FtrRec, TFltV& FltV) const {
    FltV.Add(Rnd.GetUniDev());
}

///////////////////////////////////////////////
// Numeric Feature Extractor
double TNumeric::GetVal(const TRec& FtrRec) const {
    Assert(IsStartStore(FtrRec.GetStoreId()));
    if (IsJoin(FtrRec.GetStoreId())) {
        // do the join
        TRec JoinRec = FtrRec.DoSingleJoin(GetBase(), GetJoinIdV(FtrRec.GetStoreId()));
        // get feature value
        return Reader.GetFlt(JoinRec);
    } else {
        // get feature value
        return Reader.GetFlt(FtrRec);
    }
}

TNumeric::TNumeric(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
    const int& _FieldId, const bool& NormalizeP): TFtrExt(Base, JoinSeqV), 
        FtrGen(NormalizeP), FieldId(_FieldId),
        FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)),
        Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { Clr(); }

TNumeric::TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {       
    // parse out parameters and initialize feature generator
    if (ParamVal->IsObjKey("min") && ParamVal->IsObjKey("max")) {
        const double MnVal = ParamVal->GetObjNum("min");
        const double MxVal = ParamVal->GetObjNum("max");
        FtrGen = TFtrGen::TNumeric(MnVal, MxVal);
    } else {
        bool NormalizeP = false;
        bool NormalizeVar = false;
        if (ParamVal->IsObjKey("normalize")) {
            PJsonVal NormalizeKey = ParamVal->GetObjKey("normalize");
            if (NormalizeKey->IsBool()) { // old boolean syntax
                NormalizeP = ParamVal->GetObjBool("normalize", false);
            } else {
                TStr Val = ParamVal->GetObjStr("normalize");
                Val = Val.ToLc();
                NormalizeP = true;
                if (Val == "var") {
                    NormalizeP = true;
                    NormalizeVar = true;
                } else if (Val == "scale") {
                    NormalizeP = true;
                    NormalizeVar = false;
                } else {
                    NormalizeP = false;
                    NormalizeVar = false;
                }
            }
        } else {
            // use defaults
        }
        FtrGen = TFtrGen::TNumeric(NormalizeP, NormalizeVar);
    }
    // parse out input parameters  
    TStr FieldNm = ParamVal->GetObjStr("field");
    QmAssertR(GetFtrStore()->IsFieldNm(FieldNm), "Unknown field '" + 
        FieldNm + "' in store '" + GetFtrStore()->GetStoreNm() + "'");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc);
}

TNumeric::TNumeric(const TWPt<TBase>& Base, TSIn& SIn): 
    TFtrExt(Base, SIn), FtrGen(SIn), FieldId(SIn), FieldDesc(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

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

PFtrExt TNumeric::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TNumeric(Base, SIn);
}

void TNumeric::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrGen.Save(SOut);
    FieldId.Save(SOut);
    FieldDesc.Save(SOut);
}

TStr TNumeric::GetNm() const {
    const TWPt<TStore>& Store = GetFtrStore();
    const TStr& FieldNm = Store->GetFieldNm(FieldId);
    return TStr::Fmt("Numeric[%s]", FieldNm.CStr());
}

bool TNumeric::Update(const TRec& Rec) {
    FtrGen.Update(GetVal(Rec));
    return false;
}

void TNumeric::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
    FtrGen.AddFtr(GetVal(Rec), SpV, Offset);
}

void TNumeric::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    FtrGen.AddFtr(GetVal(Rec), FullV, Offset);
}

void TNumeric::ExtractFltV(const TRec& Rec, TFltV& FltV) const {
    FltV.Add(FtrGen.GetFtr(GetVal(Rec)));   
}

PJsonVal TNumeric::InvertFullV(const TFltV& FtrV, const int& Offset) const {
    return TJsonVal::NewNum(FtrGen.InvFtr(FtrV[Offset]));
}

PJsonVal TNumeric::InvertFtr(const PJsonVal& FtrValJson) const {
    EAssert(FtrValJson->IsNum());
    const double& FtrVal = FtrValJson->GetNum();
    const double InvVal = FtrGen.InvFtr(FtrVal);
    return TJsonVal::NewNum(InvVal);
}

PJsonVal TNumeric::GetFtrRange() const {
    PJsonVal Result = TJsonVal::NewObj();

    Result->AddToObj("min", FtrGen.GetMn());
    Result->AddToObj("max", FtrGen.GetMx());

    return Result;
}

///////////////////////////////////////////////
// Sparse Vector Feature Extractor
void TNumSpV::GetVal(const TRec& FtrRec, TIntFltKdV& NumSpV) const {
    Assert(IsStartStore(FtrRec.GetStoreId()));
    if (IsJoin(FtrRec.GetStoreId())) {
        // do the join
        TRec JoinRec = FtrRec.DoSingleJoin(GetBase(), GetJoinIdV(FtrRec.GetStoreId()));
        // get feature value
        Reader.GetNumSpV(JoinRec, NumSpV);
    } else {
        // get feature value
        Reader.GetNumSpV(FtrRec, NumSpV);
    }
}

TNumSpV::TNumSpV(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId,
    const int& _Dim, const bool& _NormalizeP): TFtrExt(Base, JoinSeqV),
        Dim(_Dim), NormalizeP(_NormalizeP), FieldId(_FieldId),
        FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)),
        Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

TNumSpV::TNumSpV(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {       
    // parse out parameters and initialize feature generator
    Dim = ParamVal->GetObjInt("dimension", 0);
    NormalizeP = ParamVal->GetObjBool("normalize", false);
    // parse out input parameters
    TStr FieldNm = ParamVal->GetObjStr("field");
    QmAssertR(GetFtrStore()->IsFieldNm(FieldNm), "Unknown field '" + 
        FieldNm + "' in store '" + GetFtrStore()->GetStoreNm() + "'");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc);
}

TNumSpV::TNumSpV(const TWPt<TBase>& Base, TSIn& SIn):
    TFtrExt(Base, SIn), Dim(SIn), NormalizeP(SIn), FieldId(SIn), FieldDesc(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

PFtrExt TNumSpV::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const int& Dim, const bool& NormalizeP) {

    return new TNumSpV(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId, Dim, NormalizeP);
}

PFtrExt TNumSpV::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const int& Dim, const bool& NormalizeP) {
    
    return new TNumSpV(Base, TJoinSeqV::GetV(JoinSeq), FieldId, Dim, NormalizeP);
}

PFtrExt TNumSpV::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const int& Dim, const bool& NormalizeP) {
    
    return new TNumSpV(Base, JoinSeqV, FieldId, Dim, NormalizeP);
}

PFtrExt TNumSpV::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TNumSpV(Base, ParamVal);
}

PFtrExt TNumSpV::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TNumSpV(Base, SIn);
}

void TNumSpV::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    Dim.Save(SOut);
    NormalizeP.Save(SOut);
    FieldId.Save(SOut);
    FieldDesc.Save(SOut);
}

TStr TNumSpV::GetNm() const {
    return "SpVec[" + GetFtrStore()->GetFieldNm(FieldId) + "]";
};

TStr TNumSpV::GetFtr(const int& FtrN) const {
    return TStr::Fmt("SpVec[%s:%d/%d]", GetFtrStore()->GetFieldNm(FieldId).CStr(), FtrN, Dim.Val);
}
    
bool TNumSpV::Update(const TRec& Rec) {
    // we only need to update dimensionality, if new record makes it out-of-bounds
    TIntFltKdV NumSpV; GetVal(Rec, NumSpV);
    if (!NumSpV.Empty()) {
        const int NewDim = NumSpV.Last().Key + 1;
        if (NewDim > Dim) { Dim = NewDim; return true; }
    }
    return false;
}

void TNumSpV::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
    TIntFltKdV NumSpV; GetVal(Rec, NumSpV);
    if (NormalizeP) { TLinAlg::Normalize(NumSpV); }
    for (int NumSpN = 0; NumSpN < NumSpV.Len(); NumSpN++) {
        const TIntFltKd& NumSp = NumSpV[NumSpN];
        SpV.Add(TIntFltKd(Offset + NumSp.Key, NumSp.Dat));
    }
    Offset += GetDim();
}

void TNumSpV::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    TIntFltKdV NumSpV; GetVal(Rec, NumSpV);
    if (NormalizeP) { TLinAlg::Normalize(NumSpV); }
    for (int NumSpN = 0; NumSpN < NumSpV.Len(); NumSpN++) {
        const TIntFltKd& NumSp = NumSpV[NumSpN];
        FullV[Offset + NumSp.Key] = NumSp.Dat;
    }
    Offset += GetDim();
}

///////////////////////////////////////////////
// Categorical Feature Extractor
TStr TCategorical::GetVal(const TRec& Rec) const {
    Assert(IsStartStore(Rec.GetStoreId()));
    if (IsJoin(Rec.GetStoreId())) {
        TRec JoinRec = DoSingleJoin(Rec);
        return Reader.GetStr(JoinRec);
    } else {
        return Reader.GetStr(Rec);
    }
}

TCategorical::TCategorical(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId): 
    TFtrExt(Base, JoinSeqV), FieldId(_FieldId), FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)),
    Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

TCategorical::TCategorical(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {
    // warning for over-eager users
    if (ParamVal->IsObjKey("values") && ParamVal->IsObjKey("hashDimension")) {
        InfoNotify("Warning: using 'values' and 'hashDimension' at the "
            "same time to construct TCategorical feature extractor. "
            "'hashDimension' will be ignored");
    }
    // prase out feature generator parameters
    if (ParamVal->IsObjKey("values")) {
        // we have fixed values
        TStrV ValV; ParamVal->GetObjStrV("values", ValV);
        FtrGen = TFtrGen::TCategorical(ValV);
    } else if (ParamVal->IsObjKey("hashDimension")) {
        // we have hashed values into fixed dimensionality
        const int HashDim = ParamVal->GetObjInt("hashDimension");
        FtrGen = TFtrGen::TCategorical(HashDim);
    } else {
        // we have open value set
        FtrGen = TFtrGen::TCategorical();
    }
    // get input parameters (field, stuff, etc.)
    ParamVal->AssertObjKeyStr("field", __FUNCTION__);
    TStr FieldNm = ParamVal->GetObjStr("field");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc);
}

TCategorical::TCategorical(const TWPt<TBase>& Base, TSIn& SIn): 
    TFtrExt(Base, SIn), FtrGen(SIn), FieldId(SIn), FieldDesc(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

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

PFtrExt TCategorical::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TCategorical(Base, SIn);
}

void TCategorical::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrGen.Save(SOut);
    FieldId.Save(SOut);
    FieldDesc.Save(SOut);
}   

bool TCategorical::Update(const TRec& Rec) {
    return FtrGen.Update(GetVal(Rec));
}

void TCategorical::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
    FtrGen.AddFtr(GetVal(Rec), SpV, Offset);
}

void TCategorical::AddFullV(const TRec& Rec, TFltV& FtrV, int& Offset) const {
    FtrGen.AddFtr(GetVal(Rec), FtrV, Offset);
}

void TCategorical::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
    StrV.Add(GetVal(Rec));
}

PJsonVal TCategorical::InvertFullV(const TFltV& FtrV, const int& Offset) const {
    const int& Dim = GetDim();

    PJsonVal FtrJson = TJsonVal::NewArr();
    for (int ValN = 0; ValN < Dim; ValN++) {
        FtrJson->AddToArr(FtrV[Offset + ValN]);
    }

    return InvertFtr(FtrJson);
}

PJsonVal TCategorical::InvertFtr(const PJsonVal& FtrValJsonV) const {
    EAssert(FtrValJsonV->IsArr() && FtrValJsonV->GetArrVals()   == GetDim());

    PJsonVal Result = TJsonVal::NewObj();
    for (int FtrN = 0; FtrN < FtrValJsonV->GetArrVals(); FtrN++) {
        const PJsonVal FtrValJson = FtrValJsonV->GetArrVal(FtrN);

        EAssert(FtrValJson->IsNum());

        const TStr& FtrNm = FtrGen.GetVal(FtrN);
        const double& FtrVal = FtrValJson->GetNum();

        Result->AddToObj(FtrNm, FtrVal);
    }

    return Result;
}

PJsonVal TCategorical::GetFtrRange() const {
    const int& Dim = FtrGen.GetDim();

    PJsonVal Result = TJsonVal::NewArr();
    for (int ValN = 0; ValN < Dim; ValN++) {
        Result->AddToArr(FtrGen.GetVal(ValN));
    }

    return Result;
}

///////////////////////////////////////////////
// Multinomial Feature Extractor
void TMultinomial::GetVal(const TRec& Rec, TStrV& StrV, TFltV& FltV) const {
    Assert(IsStartStore(Rec.GetStoreId()));
    if (IsJoin(Rec.GetStoreId())) {
        PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
        Reader.GetStrV(RecSet, StrV);
        if (HasValFields()) { ValReader.GetFltV(RecSet, FltV); }
    } else {
        Reader.GetStrV(Rec, StrV);
        if (HasValFields()) { ValReader.GetFltV(Rec, FltV); }
    }
}

void TMultinomial::AddField(const int& FieldId) {
    FieldIdV.Add(FieldId);
    FieldDescV.Add(GetFtrStore()->GetFieldDesc(FieldId));
}

void TMultinomial::AddField(const TStr& FieldNm) {
    AddField(GetFtrStore()->GetFieldId(FieldNm));
}

void TMultinomial::AddValField(const int& ValFieldId) {
    ValFieldIdV.Add(ValFieldId);
    ValFieldDescV.Add(GetFtrStore()->GetFieldDesc(ValFieldId));
}

void TMultinomial::AddValField(const TStr& ValFieldNm) {
    AddValField(GetFtrStore()->GetFieldId(ValFieldNm));
}

TMultinomial::TMultinomial(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV,
        const int& _FieldId): TFtrExt(Base, JoinSeqV) {

    AddField(_FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV);
}
    
TMultinomial::TMultinomial(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {
    // warning for over-eager users
    if (ParamVal->IsObjKey("values") && ParamVal->IsObjKey("hashDimension")) {
        InfoNotify("Warning: using 'values' and 'hashDimension' at the "
            "same time to construct TCategorical feature extractor. "
            "'hashDimension' will be ignored");
    }
    // prase out feature generator parameters
    const bool NormalizeP = ParamVal->GetObjBool("normalize", false);
    const bool BinaryP =
                         ParamVal->GetObjBool("binary", false) ||   // XXX binary: true is deprecated, should use transform: 'binary' instead
                         ParamVal->GetObjStr("transform", "") == "binary";
    const bool LogP = ParamVal->GetObjStr("transform", "") == "log";
    if (ParamVal->IsObjKey("values")) {
        // we have fixed values
        TStrV ValV; ParamVal->GetObjStrV("values", ValV);
        FtrGen = TFtrGen::TMultinomial(NormalizeP, BinaryP, LogP, ValV);        
    } else if (ParamVal->IsObjKey("hashDimension")) {
        // we have hashed values into fixed dimensionality
        const int HashDim = ParamVal->GetObjInt("hashDimension");
        FtrGen = TFtrGen::TMultinomial(NormalizeP, BinaryP, LogP, HashDim);
    } else if (ParamVal->GetObjBool("datetime", false)) {
        // initialize feature generator with all possible date extracts
        FtrGen = TFtrGen::TMultinomial(NormalizeP, BinaryP, LogP, TFieldReader::GetDateRange());
    } else {
        // we have open value set
        FtrGen = TFtrGen::TMultinomial(NormalizeP, BinaryP, LogP);
    }
    // parse out field(s)
    PJsonVal FieldVal = ParamVal->GetObjKey("field");
    if (FieldVal->IsStr()) {
        AddField(ParamVal->GetObjStr("field"));
    } else if (FieldVal->IsArr()) {
        for (int FieldN = 0; FieldN < FieldVal->GetArrVals(); FieldN++) {
            AddField(FieldVal->GetArrVal(FieldN)->GetStr());
        }
    } else {
        throw TQmExcept::New("Missing field(s) in bag-of-words feature extractor");
    }
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV);
    // check if we have value providers as well
    if (ParamVal->IsObjKey("valueField")) {
        PJsonVal ValFieldVal = ParamVal->GetObjKey("valueField");
        if (FieldVal->IsStr()) {
            AddValField(ParamVal->GetObjStr("valueField"));
        } else if (FieldVal->IsArr()) {
            for (int FieldN = 0; FieldN < FieldVal->GetArrVals(); FieldN++) {
                AddValField(FieldVal->GetArrVal(FieldN)->GetStr());
            }
        } else {
            throw TQmExcept::New("Missing field(s) in bag-of-words feature extractor");
        }
        ValReader = TFieldReader(GetFtrStore()->GetStoreId(), ValFieldIdV, ValFieldDescV);
        // make sure lengths match
        QmAssert(ValFieldIdV.Len() == FieldIdV.Len());
    }
}

TMultinomial::TMultinomial(const TWPt<TBase>& Base, TSIn& SIn):
    TFtrExt(Base, SIn), FtrGen(SIn), FieldIdV(SIn), FieldDescV(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV) { }

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

PFtrExt TMultinomial::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TMultinomial(Base, SIn);
}
    
void TMultinomial::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrGen.Save(SOut);
    FieldIdV.Save(SOut);
    FieldDescV.Save(SOut);
}

TStr TMultinomial::GetNm() const { 
    TChA FieldNmChA = "Multinomial[";
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        if (FieldIdN > 0) { FieldNmChA += ";"; }
        FieldNmChA += GetFtrStore()->GetFieldNm(FieldIdV[FieldIdN]);
    }
    FieldNmChA += "]";
    return FieldNmChA; 
};

bool TMultinomial::Update(const TRec& Rec) {
    TStrV StrV; TFltV FltV; GetVal(Rec, StrV, FltV);
    FtrGen.Update(StrV);
    return true;
}

void TMultinomial::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
    TStrV StrV; TFltV FltV; GetVal(Rec, StrV, FltV);
    FtrGen.AddFtr(StrV, FltV, SpV, Offset);
}

void TMultinomial::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    TIntFltKdV SpV; AddSpV(Rec, SpV, Offset);
    for(int SpN = 0; SpN < SpV.Len(); SpN++ ){
        FullV[SpV[SpN].Key] = SpV[SpN].Dat;
    }
}

void TMultinomial::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
    TFltV FltV; GetVal(Rec, StrV, FltV);
}

void TMultinomial::ExtractFltV(const TRec& Rec, TFltV& FltV) const {
    TStrV StrV; GetVal(Rec, StrV, FltV);
}

void TMultinomial::ExtractTmV(const TRec& Rec, TTmV& TmV) const {
    // assert store ...
    TWPt<TStore> FtrStore = GetFtrStore();
    Assert(Rec.GetStoreId() == FtrStore->GetStoreId());
    // ... and field description
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        const int FieldId = FieldIdV[FieldIdN];
        const TFieldDesc& FieldDesc = FieldDescV[FieldIdN];    
        // check if start store valid
        Assert(IsStartStore(Rec.GetStoreId()));
        if (FieldDesc.IsTm()) {
            if (IsJoin(Rec.GetStoreId())) {
                PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
                for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
                    TTm FieldTm; FtrStore->GetFieldTm(Rec.GetRecId(), FieldId, FieldTm);                
                    TmV.Add(FieldTm);
                }
            } else {
                TTm FieldTm; Rec.GetFieldTm(FieldId, FieldTm);
                TmV.Add(FieldTm);
            }
        } else {
            throw TQmExcept::New("Expected TTm type, but found " + FieldDesc.GetFieldTypeStr());
        }
    }
}

///////////////////////////////////////////////
// Bag-of-words Feature Extractor
void TBagOfWords::GetVal(const TRec& Rec, TStrV& StrV) const {
    Assert(IsStartStore(Rec.GetStoreId()));
    if (IsJoin(Rec.GetStoreId())) {
        PRecSet RecSet = Rec.DoJoin(GetBase(), GetJoinIdV(Rec.GetStoreId()));
        Reader.GetStrV(RecSet, StrV);
    } else {
        Reader.GetStrV(Rec, StrV);
    }
}

void TBagOfWords::AddField(const int& FieldId) {
    FieldIdV.Add(FieldId);
    FieldDescV.Add(GetFtrStore()->GetFieldDesc(FieldId));
}

void TBagOfWords::AddField(const TStr& FieldNm) {
    AddField(GetFtrStore()->GetFieldId(FieldNm));
}

void TBagOfWords::NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs) {
    // forget forget forget
    InfoLog("Calling 'Forget' in " + GetNm());
    InfoLog(" - at " + TTm::GetTmFromMSecs(StartMSecs).GetWebLogDateTimeStr());
    InfoLog(" - time window size: " + TUInt64::GetStr(StartMSecs / 1000) + " seconds");
    FtrGen.Forget(ForgetFactor);
}

TBagOfWords::TBagOfWords(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
    const int& _FieldId, const TBagOfWordsMode& _Mode, const PTokenizer& Tokenizer, 
    const int& HashDim, const int& NStart, const int& NEnd): TFtrExt(Base, JoinSeqV),
        FtrGen(true, true, true, Tokenizer, HashDim, false, NStart, NEnd),
        Mode(_Mode) {

    AddField(_FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV);
}

TBagOfWords::TBagOfWords(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TFtrExt(Base, ParamVal) {
    
    // parse feature generator parameters
    const bool NormalizeP = ParamVal->GetObjBool("normalize", true);
    // get weighting type, default is IDF
    bool TfP = true, IdfP = true; 
    TStr WeightStr = ParamVal->GetObjStr("weight", "tfidf");
    if (WeightStr == "none") {
        TfP = false; IdfP = false;
    } else if (WeightStr == "tf") {
        TfP = true; IdfP = false;
    } else if (WeightStr == "idf") {
        TfP = false; IdfP = true;
    }
    // get parse out tokenizer
    PTokenizer Tokenizer;
    if (ParamVal->IsObjKey("tokenizer")) {
        PJsonVal TokenizerVal = ParamVal->GetObjKey("tokenizer");
        QmAssertR(TokenizerVal->IsObjKey("type"), 
            "Missing tokenizer type " + TokenizerVal->SaveStr());
        const TStr& TypeNm = TokenizerVal->GetObjStr("type");
        Tokenizer = TTokenizer::New(TypeNm, TokenizerVal);
    } else if (ParamVal->IsObjKey("stopwords") || ParamVal->IsObjKey("stemmer")) {
        // this will be deprecated at some point, print warning
        InfoLog("Warning: stopwords and stemmer no longer direct parameters for text feature extractor, please use tokenizer instead");
        // parse stow words
        PSwSet SwSet = ParamVal->IsObjKey("stopwords") ? 
            TSwSet::ParseJson(ParamVal->GetObjKey("stopwords")) :
            TSwSet::New(swstEn523);   
        // get stemmer
        PStemmer Stemmer = ParamVal->IsObjKey("stemmer") ? 
            TStemmer::ParseJson(ParamVal->GetObjKey("stemmer"), false) :
            TStemmer::New(stmtNone, false);
        // default is unicode html
        Tokenizer = TTokenizers::THtmlUnicode::New(SwSet, Stemmer);
    } else {
        Tokenizer = TTokenizers::THtmlUnicode::New(
            TSwSet::New(swstEn523), TStemmer::New(stmtNone, false));
    }

    // hashing dimension
    const int HashDim = ParamVal->GetObjInt("hashDimension", -1);
    // keep hash table?
    const bool StoreHashWordsP = ParamVal->GetObjBool("hashTable", false);

    // parse ngrams
    TInt NgramsStart = 1;
    TInt NgramsEnd = 1;
    if (ParamVal->IsObjKey("ngrams")) {
        PJsonVal NgramsVal = ParamVal->GetObjKey("ngrams");
        if(NgramsVal->IsNum()) {
            NgramsEnd = int(NgramsVal->GetNum());
        }
        else if (NgramsVal->IsArr()) {
            if(NgramsVal->GetArrVals() != 2) {
                throw TQmExcept::New("ngrams array parameter must be [start, end]");
            }
            NgramsStart = (int) NgramsVal->GetArrVal(0)->GetNum();
            NgramsEnd = (int) NgramsVal->GetArrVal(1)->GetNum();
        }
        else {
            throw TQmExcept::New("ngrams parameter must be number or array");
        }
        if((NgramsStart > NgramsEnd) || (NgramsStart <= 0) || (NgramsEnd <= 0)) {
            throw TQmExcept::New("ngrams parameters must be greater than zero and start >= end");
        }
    }

    // initialize
    FtrGen = TFtrGen::TBagOfWords(TfP, IdfP, NormalizeP, Tokenizer, HashDim, StoreHashWordsP, NgramsStart, NgramsEnd);
    
    // parse input field(s)
    PJsonVal FieldVal = ParamVal->GetObjKey("field");
    if (FieldVal->IsStr()) {
        AddField(ParamVal->GetObjStr("field"));
    } else if (FieldVal->IsArr()) {
        for (int FieldN = 0; FieldN < FieldVal->GetArrVals(); FieldN++) {
            AddField(FieldVal->GetArrVal(FieldN)->GetStr());
        }
    } else {
        throw TQmExcept::New("Missing field(s) in bag-of-words feature extractor");
    }
    
    // parse multi-instance mode settings
    TStr ModeStr = ParamVal->GetObjStr("mode", "concatenate");
    if (ModeStr == "concatenate") {
        Mode = bowmConcat;
    } else if (ModeStr == "centroid") {
        Mode = bowmCentroid;
    } else if (ModeStr == "tokenized") {
        Mode = bowmTokenized;
    } else {
        throw TQmExcept::New("Unknown bag-of-words multi-record merging mode: " + ModeStr);
    }
    
    // parse forgetting
    if (ParamVal->IsObjKey("stream")) {
        PJsonVal StreamVal = ParamVal->GetObjKey("stream");
        // get forgetting factor
        ForgetFactor = StreamVal->GetObjNum("factor");
        // check if we have time-field
        if (StreamVal->IsObjKey("field")) {
            // we have time stamp providing the tack
            TStr TimeFieldNm = StreamVal->GetObjStr("field");
            TimeFieldId = GetFtrStore()->GetFieldId(TimeFieldNm);
            QmAssertR(GetFtrStore()->GetFieldDesc(TimeFieldId).IsTm(), 
                TimeFieldNm + ":" + GetFtrStore()->GetFieldDesc(TimeFieldId).GetFieldTypeStr());
        } else {
            // we use system time
            TimeFieldId = -1;
        }
        // get forgetting clock rate
        QmAssertR(StreamVal->IsObjKey("interval"), "Missing forgetting interval");
        TmWnd.SetTimeWndMSecs(TJsonVal::GetMSecsFromJsonVal(StreamVal->GetObjKey("interval")));
        // set callback
        TmWnd.SetCallback(this);
    }

    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV);
}

TBagOfWords::TBagOfWords(const TWPt<TBase>& Base, TSIn& SIn):
    TFtrExt(Base, SIn), FtrGen(SIn), FieldIdV(SIn), FieldDescV(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldIdV, FieldDescV),
    Mode(LoadEnum<TBagOfWordsMode>(SIn)), TimeFieldId(SIn),
    TmWnd(SIn), ForgetFactor(SIn)  { }

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const TBagOfWordsMode& Mode, const PTokenizer& Tokenizer) { 
    
    return new TBagOfWords(Base, TJoinSeqV::GetV(TJoinSeq(Store)), FieldId, Mode, Tokenizer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const TBagOfWordsMode& Mode, const PTokenizer& Tokenizer) { 
        
    return new TBagOfWords(Base, TJoinSeqV::GetV(JoinSeq), FieldId, Mode, Tokenizer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV,
        const int& FieldId, const TBagOfWordsMode& Mode, const PTokenizer& Tokenizer) {
    
    return new TBagOfWords(Base, JoinSeqV, FieldId, Mode, Tokenizer); 
}

PFtrExt TBagOfWords::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TBagOfWords(Base, ParamVal);
}

PFtrExt TBagOfWords::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TBagOfWords(Base, SIn);
}

void TBagOfWords::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrGen.Save(SOut);
    FieldIdV.Save(SOut);
    FieldDescV.Save(SOut);
    SaveEnum<TBagOfWordsMode>(SOut, Mode);
    TimeFieldId.Save(SOut);
    TmWnd.Save(SOut);
    ForgetFactor.Save(SOut);
}
    
TStr TBagOfWords::GetNm() const { 
    TChA FieldNmChA = "BagOfWords[";
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        if (FieldIdN > 0) { FieldNmChA += ";"; }
        FieldNmChA += GetFtrStore()->GetFieldNm(FieldIdV[FieldIdN]);
    }
    FieldNmChA += "]";
    return FieldNmChA; 
};

TStr TBagOfWords::GetFtr(const int& FtrN) const {
    if (FtrGen.IsStoreHashWords()) {
        
        TStrV StrV; FtrGen.GetHashVals(FtrN).GetKeyV(StrV);
        return TStr::GetStr(StrV, ",");
    } else { 
        return FtrGen.GetVal(FtrN); 
    }
}

bool TBagOfWords::Update(const TRec& Rec) {
    // check if we should forget
    if (TmWnd.IsInit()) {
        //TODO: add support for joins and remove this check :-)
        QmAssertR(Rec.GetStoreId() == GetFtrStore()->GetStoreId(), 
            "Feature not yet supported: at the moment we only support cases "
            "when the start store and the feature store are the same (no joins).");        
        // read time value from the timestamp provider
        const uint64 TimeMSecs = (TimeFieldId != -1) ?
            Rec.GetFieldTmMSecs(TimeFieldId) : TTm::GetCurUniMSecs();
        // update the tick
        TmWnd.Tick(TimeMSecs);
    }
    // get all instances
    TStrV RecStrV; GetVal(Rec, RecStrV);
    if (Mode == bowmConcat) {
        // merge into one document
        return FtrGen.Update(TStr::GetStr(RecStrV, "\n"));
    } else if (Mode == bowmCentroid) {
        bool UpdateP = false;
        // threat each as a separate document
        for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
            const bool RecUpdateP = FtrGen.Update(RecStrV[RecStrN]); 
            UpdateP = UpdateP || RecUpdateP;
        }
        return UpdateP;
    } else if (Mode == bowmTokenized) {
        return FtrGen.Update(RecStrV);
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
        if (FtrGen.IsNormalize()) { TLinAlg::Normalize(CentroidSpV); }
        // add to the input sparse vector
        for (int SpN = 0; SpN < CentroidSpV.Len(); SpN++) {
            const int Id = CentroidSpV[SpN].Key;
            const double Wgt = CentroidSpV[SpN].Dat;
            SpV.Add(TIntFltKd(Offset + Id, Wgt));
        }
        // move offset for the number of tokens in feature generator
        Offset += FtrGen.GetDim();
    } else if (Mode == bowmTokenized) {
        // already tokenized
        FtrGen.AddFtr(RecStrV, SpV, Offset);
    } else {
        throw TQmExcept::New("Unknown tokenizer mode for handling multiple instances");
    }
}

void TBagOfWords::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    // get all instances
    TStrV RecStrV; GetVal(Rec, RecStrV);
    if (Mode == bowmConcat) {
        // merge into one document
        TStr RecStr = TStr::GetStr(RecStrV, "\n");
        FtrGen.AddFtr(RecStr, FullV, Offset);
    } else if (Mode == bowmCentroid) {
        // threat each as a string as a separate document
        TFltV CentroidV(FtrGen.GetDim());
        for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
            // get record sparse vector
            TIntFltKdV RecSpV; FtrGen.AddFtr(RecStrV[RecStrN], RecSpV);
            // add it to the centroid
            TLinAlg::AddVec(1.0, RecSpV, CentroidV);
        }
        // normalize centroid
        if (FtrGen.IsNormalize()) { TLinAlg::Normalize(CentroidV); }
        // add to the input sparse vector
        for (int CentroidN = 0; CentroidN < CentroidV.Len(); CentroidN++) {
            FullV[Offset + CentroidN] = CentroidV[CentroidN];
        }
        // move offset for the number of tokens in feature generator
        Offset += FtrGen.GetDim();
    } else if (Mode == bowmTokenized) {
        // already tokenized
        FtrGen.AddFtr(RecStrV, FullV, Offset);
    } else {
        throw TQmExcept::New("Unknown tokenizer mode for handling multiple instances");
    }
}

void TBagOfWords::ExtractStrV(const TRec& Rec, TStrV& StrV) const {
    TStrV RecStrV; GetVal(Rec, RecStrV);
    for (int RecStrN = 0; RecStrN < RecStrV.Len(); RecStrN++) { 
        FtrGen.GetFtr(RecStrV[RecStrN], StrV);
    }
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

TJoin::TJoin(const TWPt<TBase>& Base, TSIn& SIn): 
    TFtrExt(Base, SIn), BucketSize(SIn), Dim(SIn) { }

PFtrExt TJoin::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& BucketSize) { 
    return new TJoin(Base, TJoinSeqV::GetV(JoinSeq), BucketSize); 
}

PFtrExt TJoin::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& BucketSize) {
    return new TJoin(Base, JoinSeqV, BucketSize); 
}

PFtrExt TJoin::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TJoin(Base, ParamVal);
}

PFtrExt TJoin::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TJoin(Base, SIn);
}

void TJoin::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    BucketSize.Save(SOut);
    Dim.Save(SOut);
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
    PRecSet FtrRecSet = IsJoin(_FtrRec.GetStoreId()) ? 
        _FtrRec.DoJoin(GetBase(), GetJoinSeq(_FtrRec.GetStoreId())) :
        TRecSet::New(_FtrRec.GetStore(), _FtrRec.GetRecId());
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
    PRecSet FtrRecSet = IsJoin(_FtrRec.GetStoreId()) ? 
        _FtrRec.DoJoin(GetBase(), GetJoinSeq(_FtrRec.GetStoreId())) :
        TRecSet::New(_FtrRec.GetStore(), _FtrRec.GetRecId());
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

TPair::TPair(const TWPt<TBase>& Base, TSIn& SIn): TFtrExt(Base, SIn),
    FtrExt1(TFtrExt::Load(Base, SIn)), FtrExt2(TFtrExt::Load(Base, SIn)),
    FtrValH(SIn), FtrIdPairH(SIn) { }

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

PFtrExt TPair::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TPair(Base, SIn);
}

void TPair::Save(TSOut& SOut) const{
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrExt1->Save(SOut);
    FtrExt2->Save(SOut);
    FtrValH.Save(SOut);
    FtrIdPairH.Save(SOut);
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

bool TPair::Update(const TRec& FtrRec) {
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
    return true;
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

///////////////////////////////////////////////
// Date Window Feature Extractor
uint64 TDateWnd::_GetVal(const TRec& FtrRec) const {
    // assert store
    Assert(FtrRec.GetStoreId() == GetFtrStore()->GetStoreId());
    // extract feature value
    if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldId)) {
        return 0;
    } else if (FieldDesc.IsTm()) {
        return FtrRec.GetFieldTmMSecs(FieldId);
    }
    throw TQmExcept::New("Field type " + FieldDesc.GetFieldTypeStr() + 
        " not supported by Numeric Feature Extractor!");
}

uint64 TDateWnd::GetVal(const TRec& FtrRec) const {
    Assert(IsStartStore(FtrRec.GetStoreId()));
    if (IsJoin(FtrRec.GetStoreId())) {
        // do the join
        TRec JoinRec = FtrRec.DoSingleJoin(GetBase(), GetJoinIdV(FtrRec.GetStoreId()));
        // get feature value
        return _GetVal(JoinRec);
    } else {
        // get feature value
        return _GetVal(FtrRec);
    }
}

TDateWnd::TDateWnd(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
    const int& _FieldId, const int& WndSize, const TTmUnit& TmUnit,
    const bool& NormalizeP): TFtrExt(Base, JoinSeqV), 
        FtrGen(WndSize, TmUnit, NormalizeP), FieldId(_FieldId), 
        FieldDesc(GetFtrStore()->GetFieldDesc(FieldId)),
        Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

TDateWnd::TDateWnd(const TWPt<TBase>& Base, const PJsonVal& ParamVal): 
    TFtrExt(Base, ParamVal) {       
    
    // parse out parameters and initialize feature generator
    TTmUnit TmUnit = tmuUndef;
    TStr UnitStr = ParamVal->GetObjStr("unit", "day");
    if (UnitStr == "day") { TmUnit = tmuDay; }
    else if (UnitStr == "week") { TmUnit = tmuWeek; }
    else if (UnitStr == "month") { TmUnit = tmuMonth; }
    else if (UnitStr == "year") { TmUnit = tmuYear; }
    else if (UnitStr == "12hours") { TmUnit = tmu12Hour; }
    else if (UnitStr == "6hours") { TmUnit = tmu6Hour; }
    else if (UnitStr == "4hours") { TmUnit = tmu4Hour; }
    else if (UnitStr == "2hours") { TmUnit = tmu2Hour; }
    else if (UnitStr == "hour") { TmUnit = tmu1Hour; }
    else if (UnitStr == "30minutes") { TmUnit = tmu30Min; }
    else if (UnitStr == "15minutes") { TmUnit = tmu15Min; }
    else if (UnitStr == "10minutes") { TmUnit = tmu10Min; }
    else if (UnitStr == "minute") { TmUnit = tmu1Min; }
    else if (UnitStr == "second") { TmUnit = tmu1Sec; }
    QmAssert(TmUnit != tmuUndef);    
    // rest of parameters
    const int WndSize = ParamVal->GetObjInt("window", 1);
    const bool NormalizeP = ParamVal->GetObjBool("normalize", false);
    // check if we have predefined 
    if (ParamVal->IsObjKey("start") && ParamVal->IsObjKey("end")) {
        TTm StartTm = TTm::GetTmFromWebLogDateTimeStr(
            ParamVal->GetObjStr("start"), '-', ':', '.', 'T');
        TTm EndTm = TTm::GetTmFromWebLogDateTimeStr(
            ParamVal->GetObjStr("end"), '-', ':', '.', 'T');
        FtrGen = TFtrGen::TDateWnd(StartTm, EndTm, WndSize, TmUnit, NormalizeP);
    } else {
        FtrGen = TFtrGen::TDateWnd(WndSize, TmUnit, NormalizeP);
    }
    // parse out input parameters
    TStr FieldNm = ParamVal->GetObjStr("field");
    QmAssertR(GetFtrStore()->IsFieldNm(FieldNm), "Unknown field '" + 
        FieldNm + "' in store '" + GetFtrStore()->GetStoreNm() + "'");
    FieldId = GetFtrStore()->GetFieldId(FieldNm);
    FieldDesc = GetFtrStore()->GetFieldDesc(FieldId);
    Reader = TFieldReader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc);
}

TDateWnd::TDateWnd(const TWPt<TBase>& Base, TSIn& SIn): 
    TFtrExt(Base, SIn), FtrGen(SIn), FieldId(SIn), FieldDesc(SIn),
    Reader(GetFtrStore()->GetStoreId(), FieldId, FieldDesc) { }

PFtrExt TDateWnd::New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP) { 

    return new TDateWnd(Base, TJoinSeqV::GetV(TJoinSeq(Store)), 
        FieldId, WndSize, TmUnit, NormalizeP); 
}

PFtrExt TDateWnd::New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP) { 
    
    return new TDateWnd(Base, TJoinSeqV::GetV(JoinSeq),
        FieldId, WndSize, TmUnit, NormalizeP); 
}

PFtrExt TDateWnd::New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP) { 
    
    return new TDateWnd(Base, JoinSeqV, FieldId, 
        WndSize, TmUnit, NormalizeP); 
}

PFtrExt TDateWnd::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { 
    return new TDateWnd(Base, ParamVal);
}

PFtrExt TDateWnd::Load(const TWPt<TBase>& Base, TSIn& SIn) {
    return new TDateWnd(Base, SIn);
}

void TDateWnd::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    TFtrExt::Save(SOut);
    
    FtrGen.Save(SOut);
    FieldId.Save(SOut);
    FieldDesc.Save(SOut);
}

bool TDateWnd::Update(const TRec& Rec) {
    return FtrGen.Update(TTm::GetTmFromMSecs(GetVal(Rec)));
}

void TDateWnd::AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const {
    if (FtrGen.IsInit()) {
        FtrGen.AddFtr(TTm::GetTmFromMSecs(GetVal(Rec)), SpV, Offset);
    }
}

void TDateWnd::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
    if (FtrGen.IsInit()) {
        FtrGen.AddFtr(TTm::GetTmFromMSecs(GetVal(Rec)), FullV, Offset);
    }
}

}

}

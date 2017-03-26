/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "qminer_core.h"
#include "qminer_ftr.h"
#include "qminer_aggr.h"

namespace TQm {

///////////////////////////////
// QMiner Environment
bool TEnv::InitP = false;
TStr TEnv::QMinerFPath;
TStr TEnv::RootFPath;
PNotify TEnv::Error;
PNotify TEnv::Logger;
PNotify TEnv::Debug;
TInt TEnv::Verbosity = 0;
TInt TEnv::ReturnCode = 0;

void TEnv::Init() {
    // error notifications are to standard error output
    Error = TStdErrNotify::New();
    // default notifications are to standard output
    Logger = TStdNotify::New();
    // by defualt no debug notifications
    Debug = TNullNotify::New();
    // read environment variable indicating QMiner folder, uses current dir if not available
    QMinerFPath = TStr::GetNrAbsFPath(::TEnv::GetVarVal("QMINER_HOME"));
    RootFPath = TStr::GetNrFPath(TDir::GetCurDir());
    // initialize aggregators constructor router
    TAggr::Init();
    // initialize stream aggregators constructor router
    TStreamAggr::Init();
    // initialize stream aggregators on add filter constructor router
    TRecFilter::Init();
    // initialize feature extractors constructor router
    TFtrExt::Init();
    // tell we finished initialization
    InitP = true;
};

void TEnv::InitLogger(const int& _Verbosity,
        const TStr& FPath, const bool& TimestampP) {

    // direct logger to appropriate output
    if (FPath == "null") {
        TEnv::Logger = TNullNotify::New();  // no output
    } else {
        if (FPath == "std") {
            TEnv::Logger = TStdNotify::New(); // standard output
        } else {
            TEnv::Logger = TFPathNotify::New(FPath, "qm", true, fpnrDay); // output to rolling file
        }
        // wrap around timestamp if necessary
        if (TimestampP) {
            TEnv::Logger = TLogNotify::New(TEnv::Logger);
        }
    }
    // check the verbosity level
    Verbosity = _Verbosity;
    if (Verbosity == 0) {
        // no output
        TEnv::Logger = TNullNotify::New();
        TEnv::Debug = TNullNotify::New();
    } else if (Verbosity == 1) {
        // no debug output
        TEnv::Debug = TNullNotify::New();
    } else {
        // use same logger for debug messages
        TEnv::Debug = TEnv::Logger;
    }
}

void ErrorLog(const TStr& MsgStr) {
    TEnv::Error->OnStatus(MsgStr);
}
void InfoLog(const TStr& MsgStr) {
    if (TEnv::Verbosity >= 1) { TEnv::Logger->OnStatus(MsgStr); }
}
void DebugLog(const TStr& MsgStr) {
    if (TEnv::Verbosity >= 2) { TEnv::Debug->OnStatus(MsgStr); }
}

///////////////////////////////
// QMiner Exception
PExcept TQmExcept::New(const TStr& MsgStr, const TStr& LocStr) {
    TChA Stack = LocStr;
#ifdef GLib_WIN
    if (Stack.Len() > 0)
        Stack += "\n";
    Stack += "Stack trace:\n" + TBufferStackWalker::GetStackTrace();
#endif
    return PExcept(new TQmExcept(MsgStr, Stack));
}

///////////////////////////////
// QMiner-Valid-Name-Enforcer
TChA TNmValidator::ValidFirstCh = "_";
TChA TNmValidator::ValidCh = "_$";

bool TNmValidator::IsValidJsFirstCharacter(const TStr& NmStr) {
    const char FirstCh = NmStr[0];
    return (('A' <= FirstCh) && (FirstCh <= 'Z')) || (('a' <= FirstCh) && (FirstCh <= 'z')) || ValidFirstCh.IsChIn(FirstCh);
}

bool TNmValidator::IsValidJsCharacter(const char& Ch) {
    return (('A' <= Ch) && (Ch <= 'Z')) || (('a' <= Ch) && (Ch <= 'z')) || (('0' <= Ch) && (Ch <= '9')) || ValidCh.IsChIn(Ch);
}

void TNmValidator::Save(TSOut& SOut) const {
    StrictNmP.Save(SOut);
}

void TNmValidator::AssertValidNm(const TStr& NmStr) const {
    // must be non-empty
    QmAssertR(!NmStr.Empty(), "Name: cannot be empty");
    if (!StrictNmP) { return; }

    // check first character
    QmAssertR(IsValidJsFirstCharacter(NmStr), "Name: invalid first character in '" + NmStr + "'");

    // check rest
    for (int ChN = 1; ChN < NmStr.Len(); ChN++) {
        const char Ch = NmStr[ChN];
        QmAssertR(IsValidJsCharacter(Ch), TStr::Fmt("Name: invalid %d character in '%s'", ChN, NmStr.CStr()));
    }
}

void TNmValidator::SetStrictNmP(const bool& _StrictNmP) {
    StrictNmP = _StrictNmP;
}

///////////////////////////////
// Store window description
TStr TStoreWndDesc::SysInsertedAtFieldName = "_sys_inserted_at";

void TStoreWndDesc::Save(TSOut& SOut) const {
    TInt(WindowType).Save(SOut);
    WindowSize.Save(SOut);
    InsertP.Save(SOut);
    TimeFieldNm.Save(SOut);
}

void TStoreWndDesc::Load(TSIn& SIn) {
    WindowType = TStoreWndType(TInt(SIn).Val);
    WindowSize.Load(SIn);
    InsertP.Load(SIn);
    TimeFieldNm.Load(SIn);
}

///////////////////////////////
// QMiner-Join-Description
TJoinDesc::TJoinDesc(const TWPt<TBase>& Base, const TStr& _JoinNm, const uint& _JoinStoreId,
    const uint& StoreId, const TWPt<TIndexVoc>& IndexVoc, const TIndexKeyGixType& GixType) :
    JoinId(-1), InverseJoinId(-1) {

    // remember join parameters
    JoinStoreId = _JoinStoreId;
    JoinNm = _JoinNm;
    JoinType = osjtIndex;
    JoinRecFieldId = -1;
    JoinFqFieldId = -1;
    // create an internal join key in the index
    TStr JoinKeyNm = "Join" + JoinNm;
    JoinKeyId = IndexVoc->AddInternalKey(Base, StoreId, JoinKeyNm, JoinNm, GixType);
    // assert the name is valid
    Base->AssertValidNm(JoinNm);
}

TJoinDesc::TJoinDesc(const TWPt<TBase>& Base, const TStr& _JoinNm, const uint& _JoinStoreId,
        const int& _JoinRecFieldId, const int& _JoinFqFieldId):
            JoinId(-1),
            JoinNm(_JoinNm),
            JoinStoreId(_JoinStoreId),
            JoinType(osjtField),
            JoinKeyId(-1),
            JoinRecFieldId(_JoinRecFieldId),
            JoinFqFieldId(_JoinFqFieldId),
            InverseJoinId(-1) {
    Base->AssertValidNm(JoinNm);
}


TJoinDesc::TJoinDesc(TSIn& SIn) : JoinId(SIn), JoinNm(SIn), JoinStoreId(SIn),
    JoinKeyId(SIn), JoinRecFieldId(SIn), JoinFqFieldId(SIn), InverseJoinId(SIn) {

    JoinType = TStoreJoinType(TInt(SIn).Val);
}

void TJoinDesc::Save(TSOut& SOut) const {
    JoinId.Save(SOut); JoinNm.Save(SOut); JoinStoreId.Save(SOut);
    JoinKeyId.Save(SOut); JoinRecFieldId.Save(SOut); JoinFqFieldId.Save(SOut);
    InverseJoinId.Save(SOut);
    TInt(JoinType).Save(SOut);
}

TWPt<TStore> TJoinDesc::GetJoinStore(const TWPt<TBase>& Base) const {
    return Base->GetStoreByStoreId(GetJoinStoreId());
}

///////////////////////////////////////////////
// QMiner-Join-Sequence
TJoinSeq::TJoinSeq(const TWPt<TStore>& StartStore) : StartStoreId(StartStore->GetStoreId()) {}

TJoinSeq::TJoinSeq(const uint& _StartStoreId, const int& JoinId, const int& Sample) :
    StartStoreId(_StartStoreId) {
    JoinIdV.Add(TIntPr(JoinId, Sample));
}

TJoinSeq::TJoinSeq(const uint& _StartStoreId, const TIntPrV& _JoinIdV) :
    StartStoreId(_StartStoreId), JoinIdV(_JoinIdV) {}

TJoinSeq::TJoinSeq(const TWPt<TBase>& Base, const uint& _StartStoreId,
    const PJsonVal& JoinSeqVal) : StartStoreId(_StartStoreId) {

    // check if it's string (just join name), object (join name and parameters) or array
    QmAssert(JoinSeqVal->IsStr() || JoinSeqVal->IsObj() || JoinSeqVal->IsArr());
    // get initial store
    TWPt<TStore> Store = Base->GetStoreByStoreId(StartStoreId);
    // first check if we have only names given
    if (JoinSeqVal->IsStr()) {
        // only one join, no parameters
        TStr JoinNm = JoinSeqVal->GetStr();
        QmAssert(Store->IsJoinNm(JoinNm));
        JoinIdV.Add(TIntPr(Store->GetJoinId(JoinNm), -1));
    } else if (JoinSeqVal->IsObj()) {
        // one join, with parameters
        TStr JoinNm = JoinSeqVal->GetObjStr("name");
        const int Sample = TFlt::Round(JoinSeqVal->GetObjNum("sample", -1));
        QmAssert(Store->IsJoinNm(JoinNm));
        JoinIdV.Add(TIntPr(Store->GetJoinId(JoinNm), Sample));
    } else if (JoinSeqVal->IsArr()) {
        // sequence of multiple joins
        for (int JoinN = 0; JoinN < JoinSeqVal->GetArrVals(); JoinN++) {
            PJsonVal JoinVal = JoinSeqVal->GetArrVal(JoinN);
            if (JoinVal->IsStr()) {
                // only one join, no parameters
                TStr JoinNm = JoinVal->GetStr();
                QmAssert(Store->IsJoinNm(JoinNm));
                JoinIdV.Add(TIntPr(Store->GetJoinId(JoinNm), -1));
            } else if (JoinVal->IsObj()) {
                // one join, with parameters
                TStr JoinNm = JoinVal->GetObjStr("name");
                const int Sample = TFlt::Round(JoinVal->GetObjNum("sample", -1));
                QmAssert(Store->IsJoinNm(JoinNm));
                JoinIdV.Add(TIntPr(Store->GetJoinId(JoinNm), Sample));
            }
            // move store to the next step
            const int LastJoinId = JoinIdV.Last().Val1;
            Store = Store->GetJoinDesc(LastJoinId).GetJoinStore(Base);
        }
    }
}

TWPt<TStore> TJoinSeq::GetStartStore(const TWPt<TBase>& Base) const {
    return Base->GetStoreByStoreId(StartStoreId);
}

TWPt<TStore> TJoinSeq::GetEndStore(const TWPt<TBase>& Base) const {
    TWPt<TStore> Store = Base->GetStoreByStoreId(StartStoreId);
    for (int JoinIdN = 0; JoinIdN < JoinIdV.Len(); JoinIdN++) {
        const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinIdV[JoinIdN].Val1);
        Store = Base->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
    }
    return Store;
}

uint TJoinSeq::GetEndStoreId(const TWPt<TBase>& Base) const {
    return GetEndStore(Base)->GetStoreId();
}

TStr TJoinSeq::GetJoinPathStr(const TWPt<TBase>& Base, const TStr& SepStr) const {
    if (Empty()) { return TStr(); }
    TStr JoinPathStr;
    uint LastStoreId = StartStoreId;
    for (int JoinIdN = 0; JoinIdN < JoinIdV.Len(); JoinIdN++) {
        TWPt<TStore> Store = Base->GetStoreByStoreId(LastStoreId);
        const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinIdV[JoinIdN].Val1);
        if (!JoinPathStr.Empty()) { JoinPathStr += SepStr; }
        JoinPathStr += JoinDesc.GetJoinNm();
        LastStoreId = JoinDesc.GetJoinStoreId();
    }
    return JoinPathStr;
}

///////////////////////////////
// QMiner-Field-Description
TFieldDesc::TFieldDesc(const TWPt<TBase>& Base, const TStr& _FieldNm, TFieldType _FieldType,
    const bool& PrimaryP, const bool& NullP, const bool& InternalP, const bool& CodebookP):
    FieldId(-1), FieldNm(_FieldNm), FieldType(_FieldType) {

    Base->AssertValidNm(FieldNm);
    // set flags
    if (PrimaryP) { Flags.Val |= ofdfPrimary; }
    if (NullP) { Flags.Val |= ofdfNull; }
    if (InternalP) { Flags.Val |= ofdfInternal; }
    if (CodebookP) { Flags.Val |= ofdfCodebook; }
}

TFieldDesc::TFieldDesc(TSIn& SIn) {
    FieldId = TInt(SIn);
    FieldNm = TStr(SIn);
    FieldType = TFieldType(TInt(SIn).Val);
    Flags.Load(SIn);
    KeyIdV.Load(SIn);
}

void TFieldDesc::Save(TSOut& SOut) const {
    FieldId.Save(SOut);
    FieldNm.Save(SOut);
    TInt(FieldType).Save(SOut);
    Flags.Save(SOut);
    KeyIdV.Save(SOut);
}

TStr TFieldDesc::GetFieldTypeStr() const {
    switch (FieldType) {
    case oftUndef: return "undefined";
    case oftByte: return "byte";
    case oftInt: return "int";
    case oftInt16: return "int16";
    case oftInt64: return "int64";
    case oftIntV: return "int_v";
    case oftUInt: return "uint";
    case oftUInt16: return "uint16";
    case oftUInt64: return "uint64";
    case oftStr: return "string";
    case oftStrV: return "string_v";
    case oftBool: return "bool";
    case oftFlt: return "float";
    case oftSFlt: return "sfloat";
    case oftFltPr: return "float_pair";
    case oftFltV: return "float_v";
    case oftTm: return "datetime";
    case oftNumSpV: return "num_sp_v";
    case oftBowSpV: return "bow_sp_v";
    case oftTMem: return "blob";
    case oftJson: return "json";
    }
    Fail; return "";
}

///////////////////////////////
// QMiner-Store-Iterators
TStoreIterVec::TStoreIterVec() :
    FirstP(true), LastP(true), AscP(true), RecId(TUInt64::Mx), EndId(0) {}

TStoreIterVec::TStoreIterVec(const uint64& StartId, const uint64& _EndId, const bool& _AscP) :
    FirstP(true), LastP(false), AscP(_AscP), RecId(StartId), EndId(_EndId) {

    QmAssert(AscP ? (RecId <= EndId) : (RecId >= EndId));
}

PStoreIter TStoreIterVec::New(const uint64& StartId, const uint64& EndId, const bool& AscP) {
    return new TStoreIterVec(StartId, EndId, AscP);
}

bool TStoreIterVec::Next() {
    // we reached the end
    if (LastP) { return false; }
    if (FirstP) {
        // mark we did first next()
        FirstP = false;
    } else {
        // otherwise move to next record
        if (AscP) {
            RecId++;
        } else {
            RecId--;
        }
    }
    // check if this is the last record
    LastP = (RecId == EndId);
    // we reached the end!
    return true;
}

///////////////////////////////
// QMiner-Store
void TStore::LoadStore(TSIn& SIn) {
    StoreId = TUInt(SIn); StoreNm.Load(SIn);
    // load join descriptions
    JoinDescV.Load(SIn); JoinNmToIdH.Load(SIn);
    for (int JoinN = 0; JoinN < JoinDescV.Len(); JoinN++) {
        const TJoinDesc& JoinDesc = JoinDescV[JoinN];
        if (JoinDesc.IsIndexJoin()) {
            JoinNmToKeyIdH.AddDat(JoinDesc.GetJoinNm(), JoinDesc.GetJoinKeyId());
        }
    }
    // load field description
    FieldDescV.Load(SIn); FieldNmToIdH.Load(SIn);
}

TStore::TStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm) :
    Base(_Base), Index(_Base->GetIndex()), StoreId(_StoreId), StoreNm(_StoreNm) {
    Base->AssertValidNm(StoreNm);
}

TStore::TStore(const TWPt<TBase>& _Base, TSIn& SIn) :
    Base(_Base), Index(_Base->GetIndex()) {
    LoadStore(SIn);
}

TStore::TStore(const TWPt<TBase>& _Base, const TStr& FNm) :
    Base(_Base), Index(_Base->GetIndex()) {
    TFIn FIn(FNm); LoadStore(FIn);
}

void TStore::SaveStore(TSOut& SOut) const {
    StoreId.Save(SOut); StoreNm.Save(SOut);
    JoinDescV.Save(SOut); JoinNmToIdH.Save(SOut);
    FieldDescV.Save(SOut); FieldNmToIdH.Save(SOut);
}

TWPt<TStore> TStore::LoadById(const TWPt<TBase>& Base, TSIn& SIn) {
    TUInt StoreId(SIn);
    return Base->GetStoreByStoreId(StoreId.Val);
}

int TStore::AddFieldDesc(const TFieldDesc& FieldDesc) {
    QmAssertR(!IsJoinNm((FieldDesc.GetFieldNm())), "[AddFieldDesc] Name already taken: " + FieldDesc.GetFieldNm());
    QmAssertR(!IsFieldNm((FieldDesc.GetFieldNm())), "[AddFieldDesc] Name already taken: " + FieldDesc.GetFieldNm());
    // remember new field
    const int FieldId = FieldDescV.Add(FieldDesc);
    // set field ID mappings
    FieldDescV[FieldId].PutFieldId(FieldId);
    FieldNmToIdH.AddDat(FieldDesc.GetFieldNm()) = FieldId;
    // return the ID of the new field
    return FieldId;
}

PExcept TStore::FieldError(const int& FieldId, const TStr& TypeStr) const {
    return TQmExcept::New(TStr::Fmt("Wrong field-type combination requested: [%d:%s]!", FieldId, TypeStr.CStr()));
}

void TStore::OnAdd(const uint64& RecId) {
    OnAdd(GetRec(RecId));
}

void TStore::OnAdd(const TRec& Rec) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnAdd(Rec);
    }
}

void TStore::OnUpdate(const uint64& RecId) {
    OnUpdate(GetRec(RecId));
}

void TStore::OnUpdate(const TRec& Rec) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnUpdate(Rec);
    }
}

void TStore::OnDelete(const uint64& RecId) {
    OnDelete(GetRec(RecId));
}

void TStore::OnDelete(const TRec& Rec) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnDelete(Rec);
    }
}

void TStore::StrVToIntV(const TStrV& StrV, TStrHash<TInt, TBigStrPool>& StrH, TIntV& IntV) {
    const int Len = StrV.Len(); IntV.Gen(Len, 0);
    for (int StrN = 0; StrN < Len; StrN++) {
        IntV.Add(StrH.AddKey(StrV[StrN]));
    }
}

void TStore::IntVToStrV(const TIntV& IntV, const TStrHash<TInt, TBigStrPool>& StrH, TStrV& StrV) const {
    const int Len = IntV.Len(); StrV.Gen(Len, 0);
    for (int IntN = 0; IntN < Len; IntN++) {
        StrV.Add(StrH.GetKey(IntV[IntN]));
    }
}

void TStore::IntVToStrV(const TIntV& IntV, TStrV& StrV) const {
    const int Len = IntV.Len(); StrV.Gen(Len, 0);
    for (int IntN = 0; IntN < Len; IntN++) {
        StrV.Add(TInt::GetStr(IntV[IntN]));
    }
}

void TStore::AddJoinRec(const uint64& RecId, const PJsonVal& RecVal) {
    // check join records for each join
    for (int JoinN = 0; JoinN < GetJoins(); JoinN++) {
        // get join parameters
        const TJoinDesc& JoinDesc = GetJoinDesc(JoinN);
        // check if we have the join in JSon
        if (RecVal->IsObjKey(JoinDesc.GetJoinNm())) {
            // get join store
            TWPt<TStore> JoinStore = Base->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
            // different handling for field and index joins
            if (JoinDesc.IsFieldJoin()) {
                // get join record JSon object
                PJsonVal JoinRecVal = RecVal->GetObjKey(JoinDesc.GetJoinNm());
                // make sure it's an object and not array
                if (!JoinRecVal->IsObj()) {
                    ErrorLog("[TStore::AddJoinRec] Expected object for join " + JoinDesc.GetJoinNm());
                    if (JoinRecVal->IsArr()) { ErrorLog("[TStore::AddJoinRec] Got array instead"); }
                    continue;
                }
                // first make an empty join
                SetFieldNull(RecId, JoinDesc.GetJoinRecFieldId());
                if (JoinDesc.GetJoinFqFieldId() >= 0) {
                    SetFieldInt64Safe(RecId, JoinDesc.GetJoinFqFieldId(), 0);
                }
                // insert join record
                const uint64 JoinRecId = JoinStore->AddRec(JoinRecVal);
                // get join weight (useful only for inverse index joins)
                int JoinFq = JoinRecVal->GetObjInt("$fq", 1);
                // make sure weight is from valid range
                if (JoinFq < 1) {
                    ErrorLog("[TStore::AddJoinRec] Join frequency must be positive");
                    JoinFq = 1;
                }
                // mark the join
                AddJoin(JoinDesc.GetJoinId(), RecId, JoinRecId, JoinFq);
            } else if (JoinDesc.IsIndexJoin()) {
                // index joins must be in an array
                PJsonVal JoinArrVal = RecVal->GetObjKey(JoinDesc.GetJoinNm());
                if (!JoinArrVal->IsArr()) {
                    ErrorLog("[TStore::AddJoinRec] Expected array for join " + JoinDesc.GetJoinNm());
                    continue;
                }
                // add join records and remember their record ids and weights
                TUInt64IntKdV JoinRecIdFqVH(JoinArrVal->GetArrVals(), 0);
                for (int JoinRecN = 0; JoinRecN < JoinArrVal->GetArrVals(); JoinRecN++) {
                    // get join record JSon
                    PJsonVal JoinRecVal = JoinArrVal->GetArrVal(JoinRecN);
                    // insert join record
                    uint64 JoinRecId = JoinStore->AddRec(JoinRecVal);
                    // get join weight, default is 1
                    int JoinFq = JoinRecVal->GetObjInt("$fq", 1);
                    // make sure weight is from valid range
                    if (JoinFq < 1) {
                        ErrorLog("[TStore::AddJoinRec] Join frequency must be positive");
                        JoinFq = 1;
                    }
                    // index the join
                    AddJoin(JoinDesc.GetJoinId(), RecId, JoinRecId, JoinFq);
                }
            }
        } else {
            // we don't have join specified, set field joins to point to nothing
            if (JoinDesc.IsFieldJoin()) {
                SetFieldNull(RecId, JoinDesc.GetJoinRecFieldId());
                if (JoinDesc.GetJoinFqFieldId() >= 0) {
                    SetFieldInt64Safe(RecId, JoinDesc.GetJoinFqFieldId(), 0);
                }
            }
        }
    }
}

int TStore::AddJoinDesc(const TJoinDesc& JoinDesc) {
    // Join and Field names must be unique
    QmAssertR(!IsJoinNm((JoinDesc.GetJoinNm())), "[AddJoinDesc] Name already taken: " + JoinDesc.GetJoinNm());
    QmAssertR(!IsFieldNm((JoinDesc.GetJoinNm())), "[AddJoinDesc] Name already taken: " + JoinDesc.GetJoinNm());
    // remember new field
    const int JoinId = JoinDescV.Add(JoinDesc);
    // set field ID mappings
    JoinDescV[JoinId].PutJoinId(JoinId);
    JoinNmToIdH.AddDat(JoinDesc.GetJoinNm()) = JoinId;
    // remember name-> key id mapping
    if (JoinDesc.IsIndexJoin()) {
        JoinNmToKeyIdH.AddDat(JoinDesc.GetJoinNm(), JoinDesc.GetJoinKeyId());
    }
    // return the ID of the new field
    return JoinId;
}

void TStore::PutInverseJoinId(const int& JoinId, const int& InverseJoinId) {
    JoinDescV[JoinId].PutInverseJoinId(InverseJoinId);
}

TIntV TStore::GetFieldIdV(const TFieldType& Type) {
    TIntV FieldIdV;
    for (int i = 0; i < FieldDescV.Len(); i++) {
        if (FieldDescV[i].GetFieldType() == Type) {
            FieldIdV.Add(i);
        }
    }
    return FieldIdV;
}

void TStore::AddTrigger(const PStoreTrigger& Trigger) {
    TEnv::Logger->OnStatusFmt("Adding trigger to store %s", GetStoreNm().CStr());
    Trigger->Init(this);
    TriggerV.Add(Trigger);
}

void TStore::DelTrigger(const PStoreTrigger& Trigger) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        if (Trigger->GetGuid() == TriggerV[TriggerN]->GetGuid()) {
            TEnv::Logger->OnStatusFmt("Deleting trigger from store %s", GetStoreNm().CStr());
            TriggerV.Del(TriggerN, TriggerN);
            return;
        }
    }
    TEnv::Logger->OnStatusFmt("Trigger not found in store %s", GetStoreNm().CStr());
}

TRec TStore::GetRec(const uint64& RecId) {
    EAssertR(RecId != TUInt64::Mx, "Unable to create a record with invalid id");
    return TRec(this, RecId);
}

TRec TStore::GetRec(const TStr& RecNm) {
    const uint64 RecId = GetRecId(RecNm);
    return GetRec(RecId);
}

PRecSet TStore::GetAllRecs() {
    TUInt64V RecIdV((int)GetRecs(), 0);
    PStoreIter Iter = GetIter();
    while (Iter->Next()) {
        RecIdV.Add(Iter->GetRecId());
    }
    return TRecSet::New(TWPt<TStore>(this), RecIdV);
}

PRecSet TStore::GetRndRecs(const uint64& SampleSize) {
    return GetAllRecs()->GetSampleRecSet((int)SampleSize);
}

void TStore::AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // different handling for field and index joins
    if (JoinDesc.IsIndexJoin()) {
        Index->IndexJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    } else if (JoinDesc.IsFieldJoin()) {
        if (!IsFieldNull(RecId, JoinDesc.GetJoinRecFieldId())) {
            const uint64 ExistingJoinRecId = GetFieldUInt64Safe(RecId, JoinDesc.GetJoinRecFieldId());
            // if we have an existing join and the target record is different
            // then we first have to remove the existing join
            if (ExistingJoinRecId != JoinRecId) {
                int Fq = 1;
                if (JoinDesc.GetJoinFqFieldId() >= 0) {
                    Fq = (int)GetFieldInt64Safe(RecId, JoinDesc.GetJoinFqFieldId());
                }
                DelJoin(JoinDesc.GetJoinId(), RecId, ExistingJoinRecId, Fq);
            }
        }
        SetFieldUInt64Safe(RecId, JoinDesc.GetJoinRecFieldId(), JoinRecId);
        if (JoinDesc.GetJoinFqFieldId() >= 0) {
            SetFieldInt64Safe(RecId, JoinDesc.GetJoinFqFieldId(), JoinFq);
        }
    }
    // check if inverse join is defined
    if (JoinDesc.IsInverseJoinId()) {
        // get inverse join parameters
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(Base);
        const int InverseJoinId = JoinDesc.GetInverseJoinId();
        const TJoinDesc& InverseJoinDesc = JoinStore->GetJoinDesc(InverseJoinId);
        // different handling for field and index joins
        if (InverseJoinDesc.IsIndexJoin()) {
            Index->IndexJoin(JoinStore, InverseJoinId, JoinRecId, RecId, JoinFq);
        } else if (InverseJoinDesc.IsFieldJoin()) {
            if (!JoinStore->IsFieldNull(JoinRecId, InverseJoinDesc.GetJoinRecFieldId())) {
                // does the JoinRecId already have a join? If yes, we need to remove it first
                // start by finding to which item does JoinRecId currently point to
                const uint64 ExistingJoinRecId = JoinStore->GetFieldUInt64Safe(JoinRecId, InverseJoinDesc.GetJoinRecFieldId());
                // if ExistingJoinRecId is a valid record and is different than RecId
                // then we have to delete the join first, before setting new values
                if (ExistingJoinRecId != RecId) {
                    int Fq = 1;
                    if (InverseJoinDesc.GetJoinFqFieldId()>=0) {
                        Fq = (int)JoinStore->GetFieldInt64Safe(JoinRecId, InverseJoinDesc.GetJoinFqFieldId());
                    }
                    JoinStore->DelJoin(InverseJoinDesc.GetJoinId(), JoinRecId, ExistingJoinRecId, Fq);
                }
            }
            JoinStore->SetFieldUInt64Safe(JoinRecId, InverseJoinDesc.GetJoinRecFieldId(), RecId);
            if (InverseJoinDesc.GetJoinFqFieldId() >= 0) {
                JoinStore->SetFieldInt64Safe(JoinRecId, InverseJoinDesc.GetJoinFqFieldId(), JoinFq);
            }
        }
    }
}

void TStore::AddJoin(const TStr& JoinNm, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    AddJoin(GetJoinId(JoinNm), RecId, JoinRecId, JoinFq);
}

void TStore::DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // different handling for field and index joins
    if (JoinDesc.IsIndexJoin()) {
        Index->DeleteJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    } else if (JoinDesc.IsFieldJoin()) {
        SetFieldNull(RecId, JoinDesc.GetJoinRecFieldId());
        if (JoinDesc.GetJoinFqFieldId() >= 0) {
            SetFieldInt64Safe(RecId, JoinDesc.GetJoinFqFieldId(), 0);
        }
    }
    // check if inverse join is defined
    if (JoinDesc.IsInverseJoinId()) {
        // get inverse join parameters
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(Base);
        const int InverseJoinId = JoinDesc.GetInverseJoinId();
        const TJoinDesc& InverseJoinDesc = JoinStore->GetJoinDesc(InverseJoinId);
        // different handling for field and index joins
        if (InverseJoinDesc.IsIndexJoin()) {
            Index->DeleteJoin(JoinStore, InverseJoinId, JoinRecId, RecId, JoinFq);
        } else {
            JoinStore->SetFieldNull(JoinRecId, InverseJoinDesc.GetJoinRecFieldId());
            if (InverseJoinDesc.GetJoinFqFieldId() >= 0) {
                JoinStore->SetFieldInt64Safe(JoinRecId, InverseJoinDesc.GetJoinFqFieldId(), 0);
            }
        }
    }
}

void TStore::DelJoin(const TStr& JoinNm, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    DelJoin(GetJoinId(JoinNm), RecId, JoinRecId, JoinFq);
}

void TStore::DelJoins(const int& JoinId, const uint64& RecId) {
    PRecSet RecSet = GetRec(RecId).DoJoin(Base, JoinId);
    int Recs = RecSet->GetRecs();
    for (int N = 0; N < Recs; N++)
        DelJoin(JoinId, RecId, RecSet->GetRecId(N));
}

void TStore::DelJoins(const TStr& JoinNm, const uint64& RecId) {
    DelJoins(GetJoinId(JoinNm), RecId);
}

/// Get field value using field id safely
uint64 TStore::GetFieldUInt64Safe(const uint64& RecId, const int& FieldId) const {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: return (uint64)GetFieldByte(RecId, FieldId); break;
    case oftInt16: return (uint64)GetFieldInt16(RecId, FieldId); break;
    case oftInt: return (uint64)GetFieldInt(RecId, FieldId); break;
    case oftInt64: return (uint64)GetFieldInt64(RecId, FieldId); break;
    case oftUInt16: return (uint64)GetFieldUInt16(RecId, FieldId); break;
    case oftUInt: return (uint64)GetFieldUInt(RecId, FieldId); break;
    case oftUInt64: return GetFieldUInt64(RecId, FieldId); break;
    default: QmAssertR(false, TStr("GetFieldUInt64Safe: unsupported conversion for field id ") + FieldId);
    }
}

/// Get field value using field id safely
int64 TStore::GetFieldInt64Safe(const uint64& RecId, const int& FieldId) const {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: return (int64)GetFieldByte(RecId, FieldId); break;
    case oftInt16: return (int64)GetFieldInt16(RecId, FieldId); break;
    case oftInt: return (int64)GetFieldInt(RecId, FieldId); break;
    case oftInt64: return GetFieldInt64(RecId, FieldId); break;
    case oftUInt16: return (int64)GetFieldUInt16(RecId, FieldId); break;
    case oftUInt: return (int64)GetFieldUInt(RecId, FieldId); break;
    case oftUInt64: return (int64)GetFieldUInt64(RecId, FieldId); break;
    default: QmAssertR(false, TStr("GetFieldInt64Safe: unsupported conversion for field id ") + FieldId);
    }
}

/// Set field value using field id (default implementation throws exception)
void TStore::SetFieldUInt64Safe(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: SetFieldByte(RecId, FieldId, (uchar)UInt64); break;
    case oftInt16: SetFieldInt16(RecId, FieldId, (int16)UInt64); break;
    case oftInt: SetFieldInt(RecId, FieldId, (int)UInt64); break;
    case oftInt64: SetFieldInt64(RecId, FieldId, (int64)UInt64); break;
    case oftUInt16: SetFieldUInt16(RecId, FieldId, (uint16)UInt64); break;
    case oftUInt: SetFieldUInt(RecId, FieldId, (uint)UInt64); break;
    case oftUInt64: SetFieldUInt64(RecId, FieldId, (uint64)UInt64); break;
    default: QmAssertR(false, TStr("SetFieldUInt64Safe: unsupported conversion for field id ") + FieldId);
    }
}

/// Set field value using field id (default implementation throws exception)
void TStore::SetFieldInt64Safe(const uint64& RecId, const int& FieldId, const int64& Int64) {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: SetFieldByte(RecId, FieldId, (uchar)Int64); break;
    case oftInt16: SetFieldInt16(RecId, FieldId, (int16)Int64); break;
    case oftInt: SetFieldInt(RecId, FieldId, (int)Int64); break;
    case oftInt64: SetFieldInt64(RecId, FieldId, (int64)Int64); break;
    case oftUInt16: SetFieldUInt16(RecId, FieldId, (uint16)Int64); break;
    case oftUInt: SetFieldUInt(RecId, FieldId, (uint)Int64); break;
    case oftUInt64: SetFieldUInt64(RecId, FieldId, (uint64)Int64); break;
    default: QmAssertR(false, TStr("SetFieldInt64Safe: unsupported conversion for field id ") + FieldId);
    }
}

bool TStore::IsFieldNmNull(const uint64& RecId, const TStr& FieldNm) const {
    return IsFieldNull(RecId, GetFieldId(FieldNm));
}

int TStore::GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldInt(RecId, GetFieldId(FieldNm));
}

uchar TStore::GetFieldNmByte(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldByte(RecId, GetFieldId(FieldNm));
}

void TStore::GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const {
    GetFieldIntV(RecId, GetFieldId(FieldNm), IntV);
}

uint64 TStore::GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldUInt64(RecId, GetFieldId(FieldNm));
}

TStr TStore::GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldStr(RecId, GetFieldId(FieldNm));
}

void TStore::GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const {
    GetFieldStrV(RecId, GetFieldId(FieldNm), StrV);
}

bool TStore::GetFieldNmBool(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldBool(RecId, GetFieldId(FieldNm));
}

double TStore::GetFieldNmFlt(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldFlt(RecId, GetFieldId(FieldNm));
}

TFltPr TStore::GetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldFltPr(RecId, GetFieldId(FieldNm));
}

void TStore::GetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, TFltV& FltV) const {
    GetFieldFltV(RecId, GetFieldId(FieldNm), FltV);
}

void TStore::GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const {
    GetFieldTm(RecId, GetFieldId(FieldNm), Tm);
}

uint64 TStore::GetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldTmMSecs(RecId, GetFieldId(FieldNm));
}

void TStore::GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const {
    GetFieldNumSpV(RecId, GetFieldId(FieldNm), SpV);
}

void TStore::GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const {
    GetFieldBowSpV(RecId, GetFieldId(FieldNm), SpV);
}

void TStore::GetFieldNmTMem(const uint64& RecId, const TStr& FieldNm, TMem& Mem) const {
    GetFieldTMem(RecId, GetFieldId(FieldNm), Mem);
}

PJsonVal TStore::GetFieldNmJsonVal(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldJsonVal(RecId, GetFieldId(FieldNm));
}

void TStore::SetFieldNmNull(const uint64& RecId, const TStr& FieldNm) {
    SetFieldNull(RecId, GetFieldId(FieldNm));
}

void TStore::SetFieldNmByte(const uint64& RecId, const TStr& FieldNm, const uchar& Byte) {
    SetFieldByte(RecId, GetFieldId(FieldNm), Byte);
}

void TStore::SetFieldNmInt(const uint64& RecId, const TStr& FieldNm, const int& Int) {
    SetFieldInt(RecId, GetFieldId(FieldNm), Int);
}

void TStore::SetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, const TIntV& IntV) {
    SetFieldIntV(RecId, GetFieldId(FieldNm), IntV);
}

void TStore::SetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm, const uint64& UInt64) {
    SetFieldUInt64(RecId, GetFieldId(FieldNm), UInt64);
}

void TStore::SetFieldNmStr(const uint64& RecId, const TStr& FieldNm, const TStr& Str) {
    SetFieldStr(RecId, GetFieldId(FieldNm), Str);
}

void TStore::SetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, const TStrV& StrV) {
    SetFieldStrV(RecId, GetFieldId(FieldNm), StrV);
}

void TStore::SetFieldNmBool(const uint64& RecId, const TStr& FieldNm, const bool& Bool) {
    SetFieldBool(RecId, GetFieldId(FieldNm), Bool);
}

void TStore::SetFieldNmFlt(const uint64& RecId, const TStr& FieldNm, const double& Flt) {
    SetFieldFlt(RecId, GetFieldId(FieldNm), Flt);
}

void TStore::SetFieldNmFltPr(const uint64& RecId, const TStr& FieldNm, const TFltPr& FltPr) {
    SetFieldFltPr(RecId, GetFieldId(FieldNm), FltPr);
}

void TStore::SetFieldNmFltV(const uint64& RecId, const TStr& FieldNm, const TFltV& FltV) {
    SetFieldFltV(RecId, GetFieldId(FieldNm), FltV);
}

void TStore::SetFieldNmTm(const uint64& RecId, const TStr& FieldNm, const TTm& Tm) {
    SetFieldTm(RecId, GetFieldId(FieldNm), Tm);
}

void TStore::SetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm, const uint64& TmMSecs) {
    SetFieldTmMSecs(RecId, GetFieldId(FieldNm), TmMSecs);
}

void TStore::SetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, const TIntFltKdV& SpV) {
    SetFieldNumSpV(RecId, GetFieldId(FieldNm), SpV);
}

void TStore::SetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, const PBowSpV& SpV) {
    SetFieldBowSpV(RecId, GetFieldId(FieldNm), SpV);
}

void TStore::SetFieldNmTMem(const uint64& RecId, const TStr& FieldNm, const TMem& Mem) {
    SetFieldTMem(RecId, GetFieldId(FieldNm), Mem);
}

void TStore::SetFieldNmJsonVal(const uint64& RecId, const TStr& FieldNm, const PJsonVal& Json) {
    SetFieldJsonVal(RecId, GetFieldId(FieldNm), Json);
}

PJsonVal TStore::GetFieldJson(const uint64& RecId, const int& FieldId) const {
    const TFieldDesc& Desc = GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
        return TJsonVal::NewNum((double)GetFieldInt(RecId, FieldId));
    } else if (Desc.IsInt16()) {
        return TJsonVal::NewNum((double)GetFieldInt16(RecId, FieldId));
    } else if (Desc.IsInt64()) {
        return TJsonVal::NewNum((double)GetFieldInt64(RecId, FieldId));
    } else if (Desc.IsByte()) {
        return TJsonVal::NewNum((double)GetFieldByte(RecId, FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(RecId, FieldId, FieldIntV);
        return TJsonVal::NewArr(FieldIntV);
    } else if (Desc.IsUInt()) {
        return TJsonVal::NewNum((double)GetFieldUInt(RecId, FieldId));
    } else if (Desc.IsUInt16()) {
        return TJsonVal::NewNum((double)GetFieldUInt16(RecId, FieldId));
    } else if (Desc.IsUInt64()) {
        return TJsonVal::NewNum((double)GetFieldUInt64(RecId, FieldId));
    } else if (Desc.IsStr()) {
        return TJsonVal::NewStr(GetFieldStr(RecId, FieldId));
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
        return TJsonVal::NewArr(FieldStrV);
    } else if (Desc.IsBool()) {
        return TJsonVal::NewBool(GetFieldBool(RecId, FieldId));
    } else if (Desc.IsFlt()) {
        return TJsonVal::NewNum(GetFieldFlt(RecId, FieldId));
    } else if (Desc.IsSFlt()) {
        return TJsonVal::NewNum(GetFieldSFlt(RecId, FieldId));
    } else if (Desc.IsFltPr()) {
        return TJsonVal::NewArr(GetFieldFltPr(RecId, FieldId));
    } else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(RecId, FieldId, FieldFltV);
        return TJsonVal::NewArr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
        if (FieldTm.IsDef()) { return TJsonVal::NewStr(FieldTm.GetWebLogDateTimeStr(true, "T", false)); } else { return TJsonVal::NewNull(); }
    } else if (Desc.IsNumSpV()) {
        TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(RecId, FieldId, FieldIntFltKdV);
        return TJsonVal::NewStr(TStrUtil::GetStr(FieldIntFltKdV));
    } else if (Desc.IsBowSpV()) {
        return TJsonVal::NewStr("[PBowSpV]"); //TODO
    } else if (Desc.IsTMem()) {
        TMem Mem;
        GetFieldTMem(RecId, FieldId, Mem);
        return TJsonVal::NewStr(TStr::Base64Encode(Mem));
    } else if (Desc.IsJson()) {
        PJsonVal Json = GetFieldJsonVal(RecId, FieldId);
        return Json;
    }
    throw FieldError(FieldId, "GetFieldJson");
}

TStr TStore::GetFieldText(const uint64& RecId, const int& FieldId) const {
    const TFieldDesc& Desc = GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
        return TInt::GetStr(GetFieldInt(RecId, FieldId));
    } else if (Desc.IsInt16()) {
        return TInt::GetStr(GetFieldInt16(RecId, FieldId));
    } else if (Desc.IsInt64()) {
        return TInt64::GetStr(GetFieldInt64(RecId, FieldId));
    } else if (Desc.IsByte()) {
        return TInt::GetStr(GetFieldByte(RecId, FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(RecId, FieldId, FieldIntV);
        return TStrUtil::GetStr(FieldIntV);
    } else if (Desc.IsUInt()) {
        return TUInt::GetStr(GetFieldUInt(RecId, FieldId));
    } else if (Desc.IsUInt16()) {
        return TUInt::GetStr(GetFieldUInt16(RecId, FieldId));
    } else if (Desc.IsUInt64()) {
        return TUInt64::GetStr(GetFieldUInt64(RecId, FieldId));
    } else if (Desc.IsStr()) {
        return GetFieldStr(RecId, FieldId);
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
        return TStrUtil::GetStr(FieldStrV);
    } else if (Desc.IsBool()) {
        return GetFieldBool(RecId, FieldId) ? "Yes" : "No";
    } else if (Desc.IsFlt()) {
        return TFlt::GetStr(GetFieldFlt(RecId, FieldId));
    } else if (Desc.IsSFlt()) {
        return TFlt::GetStr(GetFieldSFlt(RecId, FieldId));
    } else if (Desc.IsFltPr()) {
        const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
        return TStr::Fmt("(%g, %g)", FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
    } else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(RecId, FieldId, FieldFltV);
        return TStrUtil::GetStr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
        if (FieldTm.IsDef()) { return FieldTm.GetWebLogDateTimeStr(); } else { return "--"; }
    } else if (Desc.IsNumSpV()) {
        TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(RecId, FieldId, FieldIntFltKdV);
        return TStrUtil::GetStr(FieldIntFltKdV);
    } else if (Desc.IsBowSpV()) {
        return "[PBowSpV]"; //TODO
    } else if (Desc.IsTMem()) {
        TMem Mem;
        GetFieldTMem(RecId, FieldId, Mem);
        return TStr::Base64Encode(Mem);
    } else if (Desc.IsJson()) {
        PJsonVal Json = GetFieldJsonVal(RecId, FieldId);
        return TJsonVal::GetStrFromVal(Json);
    }
    throw FieldError(FieldId, "GetDisplayText");
}

PJsonVal TStore::GetFieldNmJson(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldJson(RecId, GetFieldId(FieldNm));
}

TStr TStore::GetFieldNmText(const uint64& RecId, const TStr& FieldNm) const {
    return GetFieldText(RecId, GetFieldId(FieldNm));
}

PJsonVal TStore::GetStoreFieldsJson() const {
    TJsonValV FieldValV;
    for (int FieldN = 0; FieldN < GetFields(); FieldN++) {
        const TFieldDesc& FieldDesc = GetFieldDesc(FieldN);
        PJsonVal FieldVal = TJsonVal::NewObj();
        FieldVal->AddToObj("fieldId", FieldN);
        FieldVal->AddToObj("fieldName", FieldDesc.GetFieldNm());
        FieldVal->AddToObj("valueType", FieldDesc.GetFieldTypeStr());
        if (FieldDesc.IsKeys()) {
            TJsonValV KeyValV;
            for (int KeyN = 0; KeyN < FieldDesc.GetKeys(); KeyN++) {
                const int KeyId = FieldDesc.GetKeyId(KeyN);
                KeyValV.Add(TJsonVal::NewObj("keyId", KeyId));
            }
            FieldVal->AddToObj("keys", TJsonVal::NewArr(KeyValV));
        }
        FieldValV.Add(FieldVal);
    }
    return TJsonVal::NewArr(FieldValV);
}

PJsonVal TStore::GetStoreKeysJson(const TWPt<TBase>& Base) const {
    TJsonValV KeyValV;
    TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
    const TIntSet& KeySet = IndexVoc->GetStoreKeys(GetStoreId());
    int KeySetId = KeySet.FFirstKeyId();
    while (KeySet.FNextKeyId(KeySetId)) {
        const int KeyId = KeySet.GetKey(KeySetId);
        const TIndexKey& Key = IndexVoc->GetKey(KeyId);
        if (!Key.IsDef()) { continue; }
        if (Key.IsInternal()) { continue; }
        PJsonVal KeyVal = TJsonVal::NewObj();
        KeyVal->AddToObj("keyId", KeyId);
        KeyVal->AddToObj("keyName", Key.GetKeyNm());
        KeyVal->AddToObj("keyValue", Key.IsValue());
        KeyVal->AddToObj("keyText", Key.IsText());
        KeyVal->AddToObj("keyLocation", Key.IsLocation());
        if (Key.IsGixSort()) { KeyVal->AddToObj("sortBy", TStr(Key.IsSortById() ? "word-id" : "word-str")); }
        PJsonVal WordVocVal = TJsonVal::NewObj();
        if (Key.IsWordVoc()) {
            WordVocVal->AddToObj("wordVocId", Key.GetWordVocId());
            WordVocVal->AddToObj("values", (int)IndexVoc->GetWords(KeyId));
            KeyVal->AddToObj("wordVoc", WordVocVal);
        }
        if (Key.IsFields()) {
            TJsonValV FieldValV;
            for (int FieldN = 0; FieldN < Key.GetFields(); FieldN++) {
                const int FieldId = Key.GetFieldId(FieldN);
                FieldValV.Add(TJsonVal::NewObj("fieldId", FieldId));
            }
            KeyVal->AddToObj("fields", TJsonVal::NewArr(FieldValV));
        }
        KeyValV.Add(KeyVal);
    }
    return TJsonVal::NewArr(KeyValV);
}

PJsonVal TStore::GetStoreJoinsJson(const TWPt<TBase>& Base) const {
    // output the results
    TJsonValV JoinValV;
    for (int JoinId = 0; JoinId < GetJoins(); JoinId++) {
        // get join description
        const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
        const uint JoinStoreId = JoinDesc.GetJoinStoreId();
        TStr JoinStoreNm = Base->GetStoreByStoreId(JoinStoreId)->GetStoreNm();
        PJsonVal JoinVal = TJsonVal::NewObj();
        JoinVal->AddToObj("joinId", JoinId);
        JoinVal->AddToObj("joinName", JoinDesc.GetJoinNm());
        JoinVal->AddToObj("joinStoreId", JoinStoreId);
        JoinVal->AddToObj("joinStoreName", JoinStoreNm);
        if (JoinDesc.IsFieldJoin()) {
            JoinVal->AddToObj("joinType", TStr("field"));
        } else if (JoinDesc.IsIndexJoin()) {
            JoinVal->AddToObj("joinType", TStr("index"));
        }
        JoinValV.Add(JoinVal);
    }
    return TJsonVal::NewArr(JoinValV);
}

PJsonVal TStore::GetStoreJson(const TWPt<TBase>& Base) const {
    // get basic properties
    PJsonVal StoreVal = TJsonVal::NewObj();
    StoreVal->AddToObj("storeId", GetStoreId());
    StoreVal->AddToObj("storeName", GetStoreNm());
    StoreVal->AddToObj("storeRecords", int(GetRecs()));
    StoreVal->AddToObj("fields", GetStoreFieldsJson());
    StoreVal->AddToObj("keys", GetStoreKeysJson(Base));
    StoreVal->AddToObj("joins", GetStoreJoinsJson(Base));
    return StoreVal;
}

uint64 TStore::GetRecId(const PJsonVal& RecVal) const {
    if (RecVal->IsObjKey("$id")) {
        // parse out record id
        return RecVal->GetObjUInt64("$id");
    } else if (RecVal->IsObjKey("$name")) {
        QmAssertR(HasRecNm(), "[TStore::GetRecId] $name passed to store without primary key");
        // parse out record name
        TStr RecNm = RecVal->GetObjStr("$name");
        // check if we can even find this record
        QmAssertR(IsRecNm(RecNm), "[TStore::GetRecId] $name with unknown record name passed ");
        // get record id
        return GetRecId(RecNm);
    }
    return TUInt64::Mx;
}

/// Returns record-id of given field join
uint64 TStore::GetFieldJoinRecId(const uint64& RecId, const int& JoinId) {
    QmAssertR(IsJoinId(JoinId), "Invalid JoinId");
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    return GetFieldJoinRecId(RecId, JoinDesc);
}
/// Returns record-id of given field join
uint64 TStore::GetFieldJoinRecId(const uint64& RecId, const TJoinDesc& JoinDesc) {
    QmAssertR(JoinDesc.IsFieldJoin(), "Join is not field-join");
    // get join weight
    const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
    const TRec Rec = GetRec(RecId);
    if (Rec.IsFieldNull(JoinRecFieldId)) {
        return TUInt64::Mx;
    }
    return Rec.GetFieldUInt64Safe(JoinRecFieldId);
}
/// Returns frequency of given field join
int TStore::GetFieldJoinFq(const uint64& RecId, const int& JoinId) {
    QmAssertR(IsJoinId(JoinId), "Invalid JoinId");
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    return GetFieldJoinFq(RecId, JoinDesc);
}
/// Returns frequency of given field join
int TStore::GetFieldJoinFq(const uint64& RecId, const TJoinDesc& JoinDesc) {
    QmAssertR(JoinDesc.IsFieldJoin(), "Join is not field-join");
    // get join weight
    const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
    int JoinRecFq = 1;
    if (JoinFqFieldId > 0) {
        const TRec Rec = GetRec(RecId);
        JoinRecFq = Rec.GetFieldIntSafe(JoinFqFieldId);
    }
    return JoinRecFq;
}

void TStore::PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, TSOut& SOut) const {
    // print records
    SOut.PutStrFmtLn("Records: %d", RecSet->GetRecs());
    const int Fields = GetFields();
    for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
        const uint64 RecId = RecSet->GetRecId(RecN);
        TStr RecNm = GetRecNm(RecId);
        SOut.PutStrFmtLn("[%I64u] %s", RecId, RecNm.CStr());
        for (int FieldId = 0; FieldId < Fields; FieldId++) {
            const TFieldDesc& Desc = GetFieldDesc(FieldId);
            if (IsFieldNull(RecId, FieldId)) {
                SOut.PutStrFmtLn("  %s: NULL", Desc.GetFieldNm().CStr());
            } else if (Desc.IsStr()) {
                TStr FieldStr = GetFieldStr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            } else if (Desc.IsStrV()) {
                TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TStr::GetStr(FieldStrV, ", ").CStr());
            } else if (Desc.IsInt()) {
                const int FieldInt = GetFieldInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsInt16()) {
                const int FieldInt = GetFieldInt16(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsInt64()) {
                const int64 FieldInt = GetFieldInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsByte()) {
                const int FieldInt = GetFieldByte(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsUInt()) {
                const uint64 FieldInt = GetFieldUInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsUInt16()) {
                const uint64 FieldInt = GetFieldUInt16(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsSFlt()) {
                const float FieldFlt = GetFieldSFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %f", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsFltPr()) {
                const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: (%g, %g)", Desc.GetFieldNm().CStr(), FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
            } else if (Desc.IsTm()) {
                TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldTm.GetWebLogDateTimeStr().CStr());
            } else if (Desc.IsBool()) {
                TStr FieldStr = GetFieldBool(RecId, FieldId) ? "T" : "F";
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            }
        }
    }
}

void TStore::PrintRecSet(const TWPt<TBase>& Base, const PRecSet& RecSet, const TStr& FNm) const {
    TFOut FOut(FNm); PrintRecSet(Base, RecSet, FOut);
}

void TStore::PrintAll(const TWPt<TBase>& Base, TSOut& SOut, const bool& IncludingJoins) {
    // print headers
    PrintTypes(Base, SOut);
    // print records
    SOut.PutStrLn("Records:");
    const int Fields = GetFields();
    const int Joins = GetJoins();
    PStoreIter Iter = GetIter();
    while (Iter->Next()) {
        const uint64 RecId = Iter->GetRecId();
        TStr RecNm = GetRecNm(RecId);
        SOut.PutStrFmtLn("[%I64u] %s", RecId, RecNm.CStr());
        for (int FieldId = 0; FieldId < Fields; FieldId++) {
            const TFieldDesc& Desc = GetFieldDesc(FieldId);
            if (IsFieldNull(RecId, FieldId)) {
                SOut.PutStrFmtLn("  %s: NULL", Desc.GetFieldNm().CStr());
            } else if (Desc.IsStr()) {
                TStr FieldStr = GetFieldStr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            } else if (Desc.IsStrV()) {
                TStrV FieldStrV; GetFieldStrV(RecId, FieldId, FieldStrV);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TStr::GetStr(FieldStrV, ", ").CStr());
            } else if (Desc.IsInt()) {
                const int FieldInt = GetFieldInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsInt16()) {
                const int FieldInt = GetFieldInt16(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsInt64()) {
                const int64 FieldInt = GetFieldInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TInt64::GetStr(FieldInt).CStr());
            } else if (Desc.IsByte()) {
                const int FieldInt = GetFieldByte(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %d", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsUInt()) {
                const uint64 FieldInt = GetFieldUInt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TUInt64::GetStr(FieldInt).CStr());
            } else if (Desc.IsUInt16()) {
                const uint64 FieldInt = GetFieldUInt16(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TUInt64::GetStr(FieldInt).CStr());
            } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TUInt64::GetStr(FieldInt).CStr());
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsSFlt()) {
                const float FieldFlt = GetFieldSFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %f", Desc.GetFieldNm().CStr(), FieldFlt);
            } else if (Desc.IsFltPr()) {
                const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: (%g, %g)", Desc.GetFieldNm().CStr(), FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
            } else if (Desc.IsTm()) {
                TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldTm.GetWebLogDateTimeStr().CStr());
            } else if (Desc.IsBool()) {
                TStr FieldStr = GetFieldBool(RecId, FieldId) ? "T" : "F";
                SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), FieldStr.CStr());
            }
        }
        if (IncludingJoins) {
            for (int JoinId = 0; JoinId < Joins; JoinId++) {
                const TJoinDesc& Desc = GetJoinDesc(JoinId);
                if (Desc.IsFieldJoin())
                    continue;  // field joins are already printed with fields
                SOut.PutStrFmt("  %s: [", Desc.GetJoinNm().CStr());
                PRecSet JoinRecSet = GetRec(RecId).DoJoin(Base, Desc.GetJoinNm());
                for (int N = 0; N < JoinRecSet->GetRecs(); N++) {
                    SOut.PutStrFmt("%s%I64u:%d", N > 0 ? ", " : "", JoinRecSet->GetRecId(N), JoinRecSet->GetRecFq(N));
                }
                SOut.PutStrLn("]");
            }
        }
    }
}

void TStore::PrintAll(const TWPt<TBase>& Base, const TStr& FNm, const bool& IncludingJoins) {
    TFOut FOut(FNm); PrintAll(Base, FOut, IncludingJoins);
}

void TStore::PrintTypes(const TWPt<TBase>& Base, TSOut& SOut) const {
    SOut.PutStrFmtLn("Store Name: %s [%d]", GetStoreNm().CStr(), GetStoreId());
    SOut.PutStrFmtLn("Records: %I64u", GetRecs());
    // list fields
    SOut.PutStrLn("Fields:");
    const int Fields = GetFields();
    TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
    for (int FieldId = 0; FieldId < Fields; FieldId++) {
        const TFieldDesc& Desc = GetFieldDesc(FieldId);
        // basic parameters
        TStr Type = Desc.GetFieldTypeStr();
        // linked index keys
        TChA KeyChA;
        for (int KeyN = 0; KeyN < Desc.GetKeys(); KeyN++) {
            KeyChA += KeyChA.Empty() ? ", IK:" : ";";
            KeyChA += IndexVoc->GetKeyNm(Desc.GetKeyId(KeyN));
        }
        // save
        SOut.PutStrFmtLn("  %s [T:%s%s]", Desc.GetFieldNm().CStr(), Type.CStr(), KeyChA.CStr());
    }
    // list joins
    SOut.PutStrLn(TStr::Fmt("Joins:"));
    const int Joins = GetJoins();
    for (int JoinId = 0; JoinId < Joins; JoinId++) {
        const TJoinDesc& Desc = GetJoinDesc(JoinId);
        TStr JoinNm = Desc.GetJoinNm();
        TUInt JoinStoreId = Desc.GetJoinStoreId();
        TStr JoinType = Desc.IsFieldJoin() ? "FieldJoin" : "IndexJoin";
        SOut.PutStrFmtLn("  %s [S: %d, T: %s, ID: %d, KID: %d]", JoinNm.CStr(),
            JoinStoreId.Val, JoinType.CStr(), Desc.GetJoinId(), Desc.GetJoinKeyId());
    }
    // list keys
    SOut.PutStrLn(TStr::Fmt("Keys:"));
    const TIntSet& KeySet = IndexVoc->GetStoreKeys(GetStoreId());
    int KeySetId = KeySet.FFirstKeyId();
    while (KeySet.FNextKeyId(KeySetId)) {
        const int KeyId = KeySet.GetKey(KeySetId);
        const TIndexKey& Key = IndexVoc->GetKey(KeyId);
        if (!Key.IsDef()) { continue; }
        if (Key.IsInternal()) { continue; }
        SOut.PutStrFmt("  %s [ID: %d", Key.GetKeyNm().CStr(), KeyId);
        if (Key.IsValue()) { SOut.PutStr(" Value"); }
        if (Key.IsText()) { SOut.PutStr(" Text"); }
        if (Key.IsLocation()) { SOut.PutStr(" Location"); }
        if (Key.IsGixSort()) { SOut.PutStr(Key.IsSortById() ? "SortByWordId" : "SortByWord"); }
        if (Key.IsWordVoc()) { SOut.PutStrFmt(" WordVoc(#values=%d)", IndexVoc->GetWords(KeyId)); }
        SOut.PutStrLn("]");
    }
}

void TStore::PrintTypes(const TWPt<TBase>& Base, const  TStr& FNm) const {
    TFOut FOut(FNm); PrintTypes(Base, FOut);
}

void TStore::PrintRecSetAsJson(const TWPt<TBase>& Base, const PRecSet& RecSet, TSOut& SOut) {
    for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
        const uint64 RecId = RecSet->GetRecId(RecN);
        PJsonVal Json = GetRec(RecId).GetJson(Base, true, false);
        SOut.PutStrLn(Json->SaveStr());
    }
}

void TStore::PrintRecSetAsJson(const TWPt<TBase>& Base, const PRecSet& RecSet, const TStr& FNm) {
    TFOut FOut(FNm); PrintRecSetAsJson(Base, RecSet, FOut);
}

void TStore::PrintAllAsJson(const TWPt<TBase>& Base, TSOut& SOut) {
    PStoreIter Iter = GetIter();
    while (Iter->Next()) {
        const uint64 RecId = Iter->GetRecId();
        PJsonVal Json = GetRec(RecId).GetJson(Base, true, false);
        SOut.PutStrLn(Json->SaveStr());
    }
}

void TStore::PrintAllAsJson(const TWPt<TBase>& Base, const TStr& FNm) {
    TFOut FOut(FNm); PrintAllAsJson(Base, FOut);
}

///////////////////////////////
// QMiner-Record
PExcept TRec::FieldError(const int& FieldId, const TStr& TypeStr) const {
    return TQmExcept::New(TStr::Fmt("Wrong field-type combination requested: [%d:%s]!", FieldId, TypeStr.CStr()));
}

TRec::TRec(const TWPt<TStore>& _Store, const PJsonVal& JsonVal) :
    Store(_Store), ByRefP(false), RecId(TUInt64::Mx), RecValOut(RecVal) {

    // first parse all the fields
    for (int FieldId = 0; FieldId < Store->GetFields(); FieldId++) {
        const TFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
        // check if field exists in the JSON
        TStr FieldName = FieldDesc.GetFieldNm();
        if (!JsonVal->IsObjKey(FieldName)) { continue; }
        // parse the field from JSon
        PJsonVal FieldVal = JsonVal->GetObjKey(FieldName);
        // first check if it is set to null
        if (FieldVal->IsNull()) { SetFieldNull(FieldId); continue; }
        // otherwise get its value
        switch (FieldDesc.GetFieldType()) {
        case oftByte:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldByte(FieldId, (uchar)FieldVal->GetInt());
            break;
        case oftInt:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldInt(FieldId, FieldVal->GetInt());
            break;
        case oftInt16:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldInt16(FieldId, (int16)FieldVal->GetInt());
            break;
        case oftInt64:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldInt64(FieldId, (int64)FieldVal->GetNum());
            break;
        case oftIntV: {
            QmAssertR(FieldVal->IsArr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array.");
            TIntV IntV; FieldVal->GetArrIntV(IntV);
            SetFieldIntV(FieldId, IntV);
            break;
        }
        case oftUInt:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldUInt(FieldId, (uint)FieldVal->GetUInt64());
            break;
        case oftUInt16:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldUInt16(FieldId, (uint16)FieldVal->GetUInt64());
            break;
        case oftUInt64:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldUInt64(FieldId, FieldVal->GetUInt64());
            break;
        case oftStr:
            QmAssertR(FieldVal->IsStr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not string.");
            SetFieldStr(FieldId, FieldVal->GetStr());
            break;
        case oftStrV: {
            QmAssertR(FieldVal->IsArr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array.");
            TStrV StrV; FieldVal->GetArrStrV(StrV);
            SetFieldStrV(FieldId, StrV);
            break;
        }
        case oftBool:
            QmAssertR(FieldVal->IsBool(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not boolean.");
            SetFieldBool(FieldId, FieldVal->GetBool());
            break;
        case oftFlt:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldFlt(FieldId, FieldVal->GetNum());
            break;
        case oftSFlt:
            QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            SetFieldSFlt(FieldId, (float)FieldVal->GetNum());
            break;
        case oftFltPr: {
            // make sure it's array of length two
            QmAssertR(FieldVal->IsArr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array.");
            QmAssertR(FieldVal->GetArrVals() == 2, "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array - expected 2 fields.");
            PJsonVal JsonVal1 = FieldVal->GetArrVal(0);
            PJsonVal JsonVal2 = FieldVal->GetArrVal(1);
            // make sure both elements are numeric
            QmAssertR(JsonVal1->IsNum(), "The first element in the JSon array in data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            QmAssertR(JsonVal2->IsNum(), "The second element in the JSon array in data field " + FieldDesc.GetFieldNm() + " is not numeric.");
            // update
            SetFieldFltPr(FieldId, TFltPr(JsonVal1->GetNum(), JsonVal2->GetNum()));
            break;
        }
        case oftFltV: {
            QmAssertR(FieldVal->IsArr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array.");
            TFltV FltV; FieldVal->GetArrNumV(FltV);
            SetFieldFltV(FieldId, FltV);
            break;
        }
        case oftTm: {
            QmAssertR(FieldVal->IsStr() || FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not a number or a string that represents DateTime.");
            TTm Tm = FieldVal->GetTm();
            SetFieldTm(FieldId, Tm);
            break;
        }
        case oftTMem: {
            QmAssertR(FieldVal->IsStr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not a number or a string that represents DateTime.");
            // TODO do we support anything else? probably not on this level...
            TMem Mem; TStr::Base64Decode(FieldVal->GetStr(), Mem);
            SetFieldTMem(FieldId, Mem);
            break;
        }
        case oftJson: {
            SetFieldJsonVal(FieldId, FieldVal);
            break;
        }
        default:
            throw TQmExcept::New("Unsupported JSon data type for function - " + FieldDesc.GetFieldTypeStr());
        }
    }

    // second parse out the joins
    for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
        const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
        // check if field exists in the JSON
        TStr JoinName = JoinDesc.GetJoinNm();
        if (!JsonVal->IsObjKey(JoinName)) { continue; }
        // parse the field from JSon
        PJsonVal JoinVal = JsonVal->GetObjKey(JoinName);
        // for now we only support index joins
        QmAssertR(JoinVal->IsArr(), "Only support index joins in records by value");
        // get join store
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(Store->GetBase());
        // prepare record set with assumtion, that listed records exist
        // in case record does not exist, it will not be added to index join
        TUInt64V JoinRecIdV;
        for (int ValN = 0; ValN < JoinVal->GetArrVals(); ValN++) {
            PJsonVal JoinRecVal = JoinVal->GetArrVal(ValN);
            // get record ID from the json
            const uint64 JoinRecId = JoinStore->GetRecId(JoinRecVal);
            // rembember the ID if we found corresponding record
            if (JoinRecId != TUInt64::Mx) {
                JoinRecIdV.Add(JoinRecId);
            }
        }
        // remember join
        AddJoin(JoinDesc.GetJoinId(), TRecSet::New(JoinStore, JoinRecIdV));
    }
}

TRec::TRec(const TRec& Rec) : Store(Rec.Store), ByRefP(Rec.ByRefP), RecId(Rec.RecId),
    FieldIdPosH(Rec.FieldIdPosH), JoinIdPosH(Rec.JoinIdPosH), RecVal(Rec.RecVal),
    RecValOut(RecVal) {}

TRec& TRec::operator=(const TRec& Rec) {
    Store = Rec.Store;
    ByRefP = Rec.ByRefP;
    RecId = Rec.RecId;
    FieldIdPosH = Rec.FieldIdPosH;
    JoinIdPosH = Rec.JoinIdPosH;
    RecVal = Rec.RecVal;
    return *this;
}

TRec::TRec(const TWPt<TBase>& Base, TSIn& SIn): RecValOut(RecVal) {
    // load store
    const uint StoreId = TUInt(SIn);
    if (StoreId == TUInt::Mx) {
        // empty record
        ByRefP = false; RecId = TUInt64::Mx;
    } else {
        Store = Base->GetStoreByStoreId(StoreId);
        // check if we are by refrence or by value
        ByRefP = TBool(SIn);
        if (ByRefP) {
            // we are by reference, just load record ID and frequency
            RecId = TUInt64(SIn);
            Fq = TInt(SIn);
        } else {
            // we are by value, load serialization
            Fq = TInt(SIn);
            FieldIdPosH.Load(SIn);
            JoinIdPosH.Load(SIn);
            RecVal.Load(SIn);
        }
    }
}

void TRec::Save(TSOut& SOut) const {
    // save store id, or indicate this is an empty record with TUInt::Mx
    TUInt StoreId = Store.Empty() ? TUInt::Mx : GetStoreId();
    StoreId.Save(SOut);
    // check if there is more to save
    if (Store.Empty()) {
        // nope
    } else {
        ByRefP.Save(SOut);
        if (ByRefP) {
            RecId.Save(SOut);
            Fq.Save(SOut);
        } else {
            Fq.Save(SOut);
            FieldIdPosH.Save(SOut);
            JoinIdPosH.Save(SOut);
            RecVal.Save(SOut);
        }
    }
}

bool TRec::IsFieldNull(const int& FieldId) const {
    return IsByRef() ?
        Store->IsFieldNull(RecId, FieldId) :
        ((!FieldIdPosH.IsKey(FieldId)) || (FieldIdPosH.GetDat(FieldId) == -1));
}

int TRec::GetFieldInt(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldInt(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TInt(MIn).Val;
    }
    throw FieldError(FieldId, "Int");
}

int16 TRec::GetFieldInt16(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldInt16(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TInt16(MIn).Val;
    }
    throw FieldError(FieldId, "Int16");
}

int64 TRec::GetFieldInt64(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldInt64(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TInt64(MIn).Val;
    }
    throw FieldError(FieldId, "Int64");
}

uchar TRec::GetFieldByte(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldByte(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TUCh(MIn).Val;
    }
    throw FieldError(FieldId, "Byte");
}

void TRec::GetFieldIntV(const int& FieldId, TIntV& IntV) const {
    if (IsByRef()) {
        Store->GetFieldIntV(RecId, FieldId, IntV);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        IntV.Load(MIn);
    } else {
        throw FieldError(FieldId, "IntV");
    }
}

uint TRec::GetFieldUInt(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldUInt(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TUInt(MIn).Val;
    }
    throw FieldError(FieldId, "UInt");
}

uint16 TRec::GetFieldUInt16(const int& FieldId) const {
    if (IsByRef()) {
        return (uint16)Store->GetFieldUInt64(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TUInt16(MIn).Val;
    }
    throw FieldError(FieldId, "UInt16");
}

uint64 TRec::GetFieldUInt64(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldUInt64(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TUInt64(MIn).Val;
    }
    throw FieldError(FieldId, "UInt64");
}

uint64 TRec::GetFieldUInt64Safe(const int& FieldId) const {
    switch (Store->GetFieldDesc(FieldId).GetFieldType()) {
    case TFieldType::oftByte: return (uint64)GetFieldByte(FieldId); break;
    case TFieldType::oftUInt16: return (uint64)GetFieldUInt16(FieldId); break;
    case TFieldType::oftInt16: return (uint64)GetFieldInt16(FieldId); break;
    case TFieldType::oftInt: return (uint64)GetFieldInt(FieldId); break;
    case TFieldType::oftUInt: return (uint64)GetFieldUInt(FieldId); break;
    case TFieldType::oftUInt64: return (uint64)GetFieldUInt64(FieldId); break;
    default: QmAssertR(false, TStr("Field cannot be safely retrieved as uint64:") + FieldId);
    }
}

uint TRec::GetFieldUIntSafe(const int& FieldId) const {
    switch (Store->GetFieldDesc(FieldId).GetFieldType()) {
    case TFieldType::oftByte: return (uint)GetFieldByte(FieldId); break;
    case TFieldType::oftUInt16: return (uint)GetFieldUInt16(FieldId); break;
    case TFieldType::oftInt16: return (uint)GetFieldInt16(FieldId); break;
    case TFieldType::oftUInt: return (uint)GetFieldUInt(FieldId); break;
    default: QmAssertR(false, TStr("Field cannot be safely retrieved as uint:") + FieldId);
    }
}

uint16 TRec::GetFieldUInt16Safe(const int& FieldId) const {
    switch (Store->GetFieldDesc(FieldId).GetFieldType()) {
    case TFieldType::oftByte: return (uint16)GetFieldByte(FieldId); break;
    case TFieldType::oftUInt16: return (uint16)GetFieldUInt16(FieldId); break;
    default: QmAssertR(false, TStr("Field cannot be safely retrieved as uint16:") + FieldId);
    }
}

int TRec::GetFieldIntSafe(const int& FieldId) const {
    switch (Store->GetFieldDesc(FieldId).GetFieldType()) {
    case TFieldType::oftByte: return (int)GetFieldByte(FieldId); break;
    case TFieldType::oftUInt16: return (int)GetFieldUInt16(FieldId); break;
    case TFieldType::oftInt16: return (int)GetFieldInt16(FieldId); break;
    case TFieldType::oftInt: return (int)GetFieldInt(FieldId); break;
    default: QmAssertR(false, TStr("Field cannot be safely retrieved as int:") + FieldId);
    }
}

int16 TRec::GetFieldInt16Safe(const int& FieldId) const {
    switch (Store->GetFieldDesc(FieldId).GetFieldType()) {
    case TFieldType::oftByte: return (int16)GetFieldByte(FieldId); break;
    case TFieldType::oftInt16: return (int16)GetFieldInt16(FieldId); break;
    default: QmAssertR(false, TStr("Field cannot be safely retrieved as int16:") + FieldId);
    }
}

TStr TRec::GetFieldStr(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldStr(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TStr(MIn);
    }
    throw FieldError(FieldId, "Str");
}

void TRec::GetFieldStrV(const int& FieldId, TStrV& StrV) const {
    if (IsByRef()) {
        Store->GetFieldStrV(RecId, FieldId, StrV);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        StrV.Load(MIn);
    } else {
        throw FieldError(FieldId, "StrV");
    }
}

bool TRec::GetFieldBool(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldBool(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TBool(MIn).Val;
    }
    throw FieldError(FieldId, "Bool");
}

double TRec::GetFieldFlt(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldFlt(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TFlt(MIn).Val;
    }
    throw FieldError(FieldId, "Flt");
}
float TRec::GetFieldSFlt(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldSFlt(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TSFlt(MIn).Val;
    }
    throw FieldError(FieldId, "SFlt");
}

TFltPr TRec::GetFieldFltPr(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldFltPr(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TFltPr(MIn);
    }
    throw FieldError(FieldId, "FltPr");
}

void TRec::GetFieldFltV(const int& FieldId, TFltV& FltV) const {
    if (IsByRef()) {
        Store->GetFieldFltV(RecId, FieldId, FltV);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        FltV.Load(MIn);
    } else {
        throw FieldError(FieldId, "FltV");
    }
}

void TRec::GetFieldTm(const int& FieldId, TTm& Tm) const {
    if (IsByRef()) {
        return Store->GetFieldTm(RecId, FieldId, Tm);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        Tm = TTm::GetTmFromMSecs(TUInt64(MIn).Val);
    } else {
        throw FieldError(FieldId, "Tm");
    }
}

uint64 TRec::GetFieldTmMSecs(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldTmMSecs(RecId, FieldId);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        return TUInt64(MIn).Val;
    } else {
        throw FieldError(FieldId, "Tm");
    }
}

void TRec::GetFieldNumSpV(const int& FieldId, TIntFltKdV& NumSpV) const {
    if (IsByRef()) {
        Store->GetFieldNumSpV(RecId, FieldId, NumSpV);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        NumSpV.Load(MIn);
    } else {
        throw FieldError(FieldId, "NumSpV");
    }
}

void TRec::GetFieldBowSpV(const int& FieldId, PBowSpV& BowSpV) const {
    if (IsByRef()) {
        Store->GetFieldBowSpV(RecId, FieldId, BowSpV);
    } else if (FieldIdPosH.IsKey(FieldId)) {
        const int Pos = FieldIdPosH.GetDat(FieldId);
        TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
        BowSpV = TBowSpV::Load(MIn);
    } else {
        throw FieldError(FieldId, "NumSpV");
    }
}

void TRec::GetFieldTMem(const int& FieldId, TMem& Mem) const {
    if (IsByRef()) {
        Store->GetFieldTMem(RecId, FieldId, Mem);
    } else {
        throw FieldError(FieldId, "TMem");
    }
}

PJsonVal TRec::GetFieldJsonVal(const int& FieldId) const {
    if (IsByRef()) {
        return Store->GetFieldJsonVal(RecId, FieldId);
    } else {
        throw FieldError(FieldId, "JsonVal");
    }
}

PJsonVal TRec::GetFieldJson(const int& FieldId) const {
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
        return TJsonVal::NewNum((double)GetFieldInt(FieldId));
    } else if (Desc.IsInt16()) {
        return TJsonVal::NewNum((double)GetFieldInt16(FieldId));
    } else if (Desc.IsInt64()) {
        return TJsonVal::NewNum((double)GetFieldInt64(FieldId));
    } else if (Desc.IsByte()) {
        return TJsonVal::NewNum((double)GetFieldByte(FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(FieldId, FieldIntV);
        return TJsonVal::NewArr(FieldIntV);
    } else if (Desc.IsUInt()) {
        return TJsonVal::NewNum((double)GetFieldUInt(FieldId));
    } else if (Desc.IsUInt16()) {
        return TJsonVal::NewNum((double)GetFieldUInt16(FieldId));
    } else if (Desc.IsUInt64()) {
        return TJsonVal::NewNum((double)GetFieldUInt64(FieldId));
    } else if (Desc.IsStr()) {
        return TJsonVal::NewStr(GetFieldStr(FieldId));
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(FieldId, FieldStrV);
        return TJsonVal::NewArr(FieldStrV);
    } else if (Desc.IsBool()) {
        return TJsonVal::NewBool(GetFieldBool(FieldId));
    } else if (Desc.IsFlt()) {
        return TJsonVal::NewNum(GetFieldFlt(FieldId));
    } else if (Desc.IsSFlt()) {
        return TJsonVal::NewNum(GetFieldSFlt(FieldId));
    } else if (Desc.IsFltPr()) {
        return TJsonVal::NewArr(GetFieldFltPr(FieldId));
    } else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(FieldId, FieldFltV);
        return TJsonVal::NewArr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(FieldId, FieldTm);
        if (FieldTm.IsDef()) { return TJsonVal::NewStr(FieldTm.GetWebLogDateTimeStr(true, "T", true)); }
        else { return TJsonVal::NewNull(); }
    } else if (Desc.IsNumSpV()) {
        TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(FieldId, FieldIntFltKdV);
        return TJsonVal::NewStr(TStrUtil::GetStr(FieldIntFltKdV));
    } else if (Desc.IsBowSpV()) {
        return TJsonVal::NewStr("[PBowSpV]"); //TODO
    } else if (Desc.IsTMem()) {
        TMem Mem;
        GetFieldTMem(FieldId, Mem);
        return TJsonVal::NewStr(TStr::Base64Encode(Mem));
    } else if (Desc.IsJson()) {
        PJsonVal Json = GetFieldJsonVal(FieldId);
        return Json;
    }
    throw FieldError(FieldId, "GetFieldJson");
}

TStr TRec::GetFieldText(const int& FieldId) const {
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
        return TInt::GetStr(GetFieldInt(FieldId));
    } else if (Desc.IsInt16()) {
        return TInt::GetStr(GetFieldInt16(FieldId));
    } else if (Desc.IsInt64()) {
        return TInt64::GetStr(GetFieldInt64(FieldId));
    } else if (Desc.IsByte()) {
        return TInt::GetStr(GetFieldByte(FieldId));
    } else if (Desc.IsIntV()) {
        TIntV IntV; GetFieldIntV(FieldId, IntV);
        return TStrUtil::GetStr(IntV);
    } else if (Desc.IsBool()) {
        return GetFieldBool(FieldId) ? "Yes" : "No";
    } else if (Desc.IsUInt()) {
        return TUInt::GetStr(GetFieldUInt(FieldId));
    } else if (Desc.IsUInt16()) {
        return TUInt::GetStr(GetFieldUInt16(FieldId));
    } else if (Desc.IsUInt64()) {
        return TUInt64::GetStr(GetFieldUInt64(FieldId));
    } else if (Desc.IsStr()) {
        return GetFieldStr(FieldId);
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(FieldId, FieldStrV);
        return TStr::GetStr(FieldStrV, ", ");
    } else if (Desc.IsFlt()) {
        return TFlt::GetStr(GetFieldFlt(FieldId));
    } else if (Desc.IsSFlt()) {
        return TFlt::GetStr(GetFieldSFlt(FieldId));
    } else if (Desc.IsFltPr()) {
        const TFltPr FieldFltPr = GetFieldFltPr(FieldId);
        return TStr::Fmt("(%g, %g)", FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
    } else if (Desc.IsFltV()) {
        TFltV FltV; GetFieldFltV(FieldId, FltV);
        return TStrUtil::GetStr(FltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(FieldId, FieldTm);
        if (FieldTm.IsDef()) { return FieldTm.GetWebLogDateTimeStr(); } else { return "--"; }
    } else if (Desc.IsNumSpV()) {
        TIntFltKdV IntFltKdV; GetFieldNumSpV(FieldId, IntFltKdV);
        return TStrUtil::GetStr(IntFltKdV);
    } else if (Desc.IsBowSpV()) {
        return "[PBowSpV]"; //TODO
    } else if (Desc.IsTMem()) {
        TMem Mem;
        GetFieldTMem(FieldId, Mem);
        return TStr::Base64Encode(Mem);
    } else if (Desc.IsJson()) {
        PJsonVal Json = GetFieldJsonVal(FieldId);
        return TJsonVal::GetStrFromVal(Json);
    }
    throw FieldError(FieldId, "GetDisplayText");
}

void TRec::SetFieldNull(const int& FieldId) {
    if (IsByRef()) {
        Store->SetFieldNull(RecId, FieldId);
    } else {
        FieldIdPosH.AddDat(FieldId, -1);
    }
}

void TRec::SetFieldByte(const int& FieldId, const uchar& Byte) {
    if (IsByRef()) {
        Store->SetFieldByte(RecId, FieldId, Byte);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TUCh(Byte).Save(RecValOut);
    }
}

void TRec::SetFieldInt(const int& FieldId, const int& Int) {
    if (IsByRef()) {
        Store->SetFieldInt(RecId, FieldId, Int);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TInt(Int).Save(RecValOut);
    }
}

void TRec::SetFieldInt16(const int& FieldId, const int16& Int16) {
    if (IsByRef()) {
        Store->SetFieldInt16(RecId, FieldId, Int16);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TInt16(Int16).Save(RecValOut);
    }
}

void TRec::SetFieldInt64(const int& FieldId, const int64& Int64) {
    if (IsByRef()) {
        Store->SetFieldInt64(RecId, FieldId, Int64);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TInt64(Int64).Save(RecValOut);
    }
}

void TRec::SetFieldIntV(const int& FieldId, const TIntV& IntV) {
    if (IsByRef()) {
        Store->SetFieldIntV(RecId, FieldId, IntV);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        IntV.Save(RecValOut);
    }
}

void TRec::SetFieldUInt(const int& FieldId, const uint& UInt) {
    if (IsByRef()) {
        Store->SetFieldUInt(RecId, FieldId, UInt);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TUInt(UInt).Save(RecValOut);
    }
}

void TRec::SetFieldUInt16(const int& FieldId, const uint16& UInt16) {
    if (IsByRef()) {
        Store->SetFieldUInt16(RecId, FieldId, UInt16);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TUInt16(UInt16).Save(RecValOut);
    }
}

void TRec::SetFieldUInt64(const int& FieldId, const uint64& UInt64) {
    if (IsByRef()) {
        Store->SetFieldUInt64(RecId, FieldId, UInt64);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TUInt64(UInt64).Save(RecValOut);
    }
}

void TRec::SetFieldStr(const int& FieldId, const TStr& Str) {
    if (IsByRef()) {
        Store->SetFieldStr(RecId, FieldId, Str);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        Str.Save(RecValOut);
    }
}

void TRec::SetFieldStrV(const int& FieldId, const TStrV& StrV) {
    if (IsByRef()) {
        Store->SetFieldStrV(RecId, FieldId, StrV);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        StrV.Save(RecValOut);
    }
}

void TRec::SetFieldBool(const int& FieldId, const bool& Bool) {
    if (IsByRef()) {
        Store->SetFieldBool(RecId, FieldId, Bool);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TBool(Bool).Save(RecValOut);
    }
}

void TRec::SetFieldFlt(const int& FieldId, const double& Flt) {
    if (IsByRef()) {
        Store->SetFieldFlt(RecId, FieldId, Flt);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TFlt(Flt).Save(RecValOut);
    }
}

void TRec::SetFieldSFlt(const int& FieldId, const float& Flt) {
    if (IsByRef()) {
        Store->SetFieldSFlt(RecId, FieldId, Flt);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TSFlt(Flt).Save(RecValOut);
    }
}

void TRec::SetFieldFltV(const int& FieldId, const TFltV& FltV) {
    if (IsByRef()) {
        Store->SetFieldFltV(RecId, FieldId, FltV);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        FltV.Save(RecValOut);
    }
}

void TRec::SetFieldFltPr(const int& FieldId, const TFltPr& FltPr) {
    if (IsByRef()) {
        Store->SetFieldFltPr(RecId, FieldId, FltPr);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        FltPr.Save(RecValOut);
    }
}

void TRec::SetFieldTm(const int& FieldId, const TTm& Tm) {
    if (IsByRef()) {
        Store->SetFieldTm(RecId, FieldId, Tm);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        const uint64 TmMSecs = TTm::GetMSecsFromTm(Tm);
        TUInt64(TmMSecs).Save(RecValOut);
    }
}

void TRec::SetFieldTmMSecs(const int& FieldId, const uint64& TmMSecs) {
    if (IsByRef()) {
        Store->SetFieldTmMSecs(RecId, FieldId, TmMSecs);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TUInt64(TmMSecs).Save(RecValOut);
    }
}


void TRec::SetFieldNumSpV(const int& FieldId, const TIntFltKdV& NumSpV) {
    if (IsByRef()) {
        Store->SetFieldNumSpV(RecId, FieldId, NumSpV);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        NumSpV.Save(RecValOut);
    }
}

void TRec::SetFieldBowSpV(const int& FieldId, const PBowSpV& BowSpV) {
    if (IsByRef()) {
        Store->SetFieldBowSpV(RecId, FieldId, BowSpV);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        BowSpV->Save(RecValOut);
    }
}

void TRec::SetFieldTMem(const int& FieldId, const TMem& Mem) {
    if (IsByRef()) {
        Store->SetFieldTMem(RecId, FieldId, Mem);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        Mem.Save(RecValOut);
    }
}

void TRec::SetFieldJsonVal(const int& FieldId, const PJsonVal& Json) {
    if (IsByRef()) {
        Store->SetFieldJsonVal(RecId, FieldId, Json);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TJsonVal::GetStrFromVal(Json).Save(RecValOut);
    }
}

void TRec::AddJoin(const int& JoinId, const PRecSet& JoinRecSet) {
    JoinIdPosH.AddDat(JoinId, RecVal.Len());
    const TUInt64IntKdV& JoinRecIdFqV = JoinRecSet->GetRecIdFqV();
    JoinRecIdFqV.Save(RecValOut);
}

PRecSet TRec::ToRecSet() const {
    QmAssertR(IsByRef(), "Cannot transform record passed by value to a set!");
    return IsDef() ? TRecSet::New(Store, RecId) : TRecSet::New(Store);
}

/// Returns record-id of given field join
uint64 TRec::GetFieldJoinRecId(const int& JoinId) const {
    QmAssertR(Store->IsJoinId(JoinId), "Invalid JoinId");
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    return GetFieldJoinRecId(JoinDesc);
}

/// Returns record-id of given field join
uint64 TRec::GetFieldJoinRecId(const TJoinDesc& JoinDesc) const {
    QmAssertR(JoinDesc.IsFieldJoin(), "Join is not field-join");
    // get join weight
    const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
    if (IsFieldNull(JoinRecFieldId)) {
        return TUInt64::Mx;
    }
    return GetFieldUInt64Safe(JoinRecFieldId);
}

int TRec::GetFieldJoinFq(const int& JoinId) const {
    QmAssertR(Store->IsJoinId(JoinId), "Invalid JoinId");
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    return GetFieldJoinFq(JoinDesc);
}

int TRec::GetFieldJoinFq(const TJoinDesc& JoinDesc) const {
    QmAssertR(JoinDesc.IsFieldJoin(), "Join is not field-join");
    // get join weight
    const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
    int JoinRecFq = 1;
    if (JoinFqFieldId > 0) {
        JoinRecFq = GetFieldIntSafe(JoinFqFieldId);
    }
    return JoinRecFq;
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const int& JoinId) const {
    // get join info
    QmAssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    // check if index join
    if (JoinDesc.IsIndexJoin()) {
        TUInt64IntKdV JoinRecIdFqV;
        if (IsByRef()) {
            // by reference
            Assert(Store->IsRecId(GetRecId()));
            // do join using index
            const int JoinKeyId = JoinDesc.GetJoinKeyId();
            // execute join query
            Base->GetIndex()->SearchGixJoin(JoinKeyId, GetRecId(), JoinRecIdFqV);
        } else {
            // do join using serialized record set
            if (JoinIdPosH.IsKey(JoinId)) {
                const int Pos = JoinIdPosH.GetDat(JoinId);
                TMIn MIn(RecVal.GetBf() + Pos, RecVal.Len() - Pos, false);
                JoinRecIdFqV.Load(MIn);
            }
        }
        return TRecSet::New(JoinDesc.GetJoinStore(Base), JoinRecIdFqV);
    } else if (JoinDesc.IsFieldJoin()) {
        // do join using store field
        const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
        const uint64 JoinRecId = IsFieldNull(JoinRecFieldId) ? TUInt64::Mx : GetFieldUInt64Safe(JoinRecFieldId);
        // return record set
        if (JoinRecId != TUInt64::Mx) {
            return TRecSet::New(JoinDesc.GetJoinStore(Base), JoinRecId, GetFieldJoinFq(JoinDesc));
        } else {
            // no record, return empty set
            return TRecSet::New(JoinDesc.GetJoinStore(Base));
        }
    }
    // unknown join type
    throw TQmExcept::New("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const {
    if (Store->IsJoinNm(JoinNm)) {
        return DoJoin(Base, Store->GetJoinId(JoinNm));
    } else {
        throw TQmExcept::New("Unknown join " + JoinNm);
    }
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const {
    PRecSet RecSet = DoJoin(Base, JoinIdV[0].Val1);
    for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
        RecSet = RecSet->DoJoin(Base, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2);
    }
    return RecSet;
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const {
    return DoJoin(Base, JoinSeq.GetJoinIdV());
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const int& JoinId) const {
    PStore JoinStore = Store->GetJoinDesc(JoinId).GetJoinStore(Base);
    return TRec(JoinStore, GetFieldJoinRecId(JoinId), GetFieldJoinFq(JoinId));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const {
    const int& JoinId = Store->GetJoinId(JoinNm);
    PStore JoinStore = Store->GetJoinDesc(JoinId).GetJoinStore(Base);
    return TRec(JoinStore, GetFieldJoinRecId(JoinId), GetFieldJoinFq(JoinId));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const {
    PRecSet JoinRecSet = DoJoin(Base, JoinIdV);
    return TRec(JoinRecSet->GetStore(),
        JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0),
        JoinRecSet->Empty() ? 0 : JoinRecSet->GetRecFq(0));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const {
    PRecSet JoinRecSet = DoJoin(Base, JoinSeq);
    return TRec(JoinRecSet->GetStore(),
        JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0),
        JoinRecSet->Empty() ? 0 :JoinRecSet->GetRecFq(0));
}

PJsonVal TRec::GetJson(const TWPt<TBase>& Base, const bool& FieldsP,
    const bool& StoreInfoP, const bool& JoinRecsP, const bool& JoinRecFieldsP,
    const bool& RecInfoP) const {

    // export result set as XML
    PJsonVal RecVal = TJsonVal::NewObj();
    if (StoreInfoP) {
        PJsonVal StoreVal = TJsonVal::NewObj();
        StoreVal->AddToObj("$id", Store->GetStoreId());
        StoreVal->AddToObj("$name", Store->GetStoreNm());
        RecVal->AddToObj("$store", StoreVal);
    }
    // record name and id only if stored by reference
    if (ByRefP && RecInfoP) {
        RecVal->AddToObj("$id", (double)RecId);
        // put name only when no fields displayed and one exists in the store
        if (!FieldsP && Store->HasRecNm()) {
            RecVal->AddToObj("$name", Store->GetRecNm(RecId));
        }
    }
    // if no fields and stuff, just return what we have
    if (FieldsP) {
        // get the fields
        const int Fields = Store->GetFields();
        for (int FieldN = 0; FieldN < Fields; FieldN++) {
            const TFieldDesc& Desc = Store->GetFieldDesc(FieldN);
            // skip internal fields (e.g. record ids for joins)
            if (Desc.IsInternal()) { continue; }
            if (ByRefP) {
                if (Store->IsFieldNull(RecId, FieldN)) { continue; }
                RecVal->AddToObj(Desc.GetFieldNm(), Store->GetFieldJson(RecId, FieldN));
            } else {
                if (IsFieldNull(FieldN)) { continue; }
                RecVal->AddToObj(Desc.GetFieldNm(), GetFieldJson(FieldN));
            }
        }
    }
    // get the join fields
    if (JoinRecsP) {
        const int Joins = Store->GetJoins();
        for (int JoinId = 0; JoinId < Joins; JoinId++) {
            const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
            if (JoinDesc.IsIndexJoin()) {
                PJsonVal JoinVal = TJsonVal::NewArr();
                PRecSet JoinSet = DoJoin(Base, JoinDesc.GetJoinId());
                for (int RecN = 0; RecN < JoinSet->GetRecs(); RecN++) {
                    PJsonVal JoinRecVal = JoinSet->GetRec(RecN).GetJson(Base, JoinRecFieldsP, false, false);
                    JoinRecVal->AddToObj("$fq", JoinSet->GetRecFq(RecN));
                    JoinVal->AddToArr(JoinRecVal);
                }
                RecVal->AddToObj(JoinDesc.GetJoinNm(), JoinVal);
            } else if (JoinDesc.IsFieldJoin()) {
                PJsonVal JoinVal = TJsonVal::NewArr();
                TRec JoinRec = DoSingleJoin(Base, JoinDesc.GetJoinId());
                if (JoinRec.IsDef()) {
                    RecVal->AddToObj(JoinDesc.GetJoinNm(), JoinRec.GetJson(Base, JoinRecFieldsP, false, false));
                }
            }
        }
    }
    // finalize
    return RecVal;
}

///////////////////////////////
/// Record Comparator by Frequency
bool TRecCmpByFq::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Asc) {
        return (RecIdFq1.Dat == RecIdFq2.Dat) ?
            (RecIdFq1.Key < RecIdFq2.Key) : (RecIdFq1.Dat < RecIdFq2.Dat);
    } else {
        return (RecIdFq2.Dat == RecIdFq1.Dat) ?
            (RecIdFq2.Key < RecIdFq1.Key) : (RecIdFq2.Dat < RecIdFq1.Dat);
    }
}

///////////////////////////////
/// Record Comparator by Integer Field.
bool TRecCmpByFieldInt::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
    if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
    const int RecVal1 = Store->GetFieldInt(RecIdFq1.Key, FieldId);
    const int RecVal2 = Store->GetFieldInt(RecIdFq2.Key, FieldId);
    if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
}

///////////////////////////////
/// Record Comparator by Numeric Field.
bool TRecCmpByFieldFlt::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
    if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
    const double RecVal1 = Store->GetFieldFlt(RecIdFq1.Key, FieldId);
    const double RecVal2 = Store->GetFieldFlt(RecIdFq2.Key, FieldId);
    if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
}

///////////////////////////////
/// Record Comparator by String Field.
bool TRecCmpByFieldStr::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
    if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
    const TStr RecVal1 = Store->GetFieldStr(RecIdFq1.Key, FieldId);
    const TStr RecVal2 = Store->GetFieldStr(RecIdFq2.Key, FieldId);
    if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
}

///////////////////////////////
/// Record Comparator by Time Field.
bool TRecCmpByFieldTm::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
    if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
    const uint64 RecVal1 = Store->GetFieldTmMSecs(RecIdFq1.Key, FieldId);
    const uint64 RecVal2 = Store->GetFieldTmMSecs(RecIdFq2.Key, FieldId);
    if (Asc) { return RecVal1 < RecVal2; } else { return RecVal2 < RecVal1; }
}

///////////////////////////////
/// Record Comparator by Byte Field.
bool TRecCmpByFieldByte::operator()(const TUInt64IntKd& RecIdFq1, const TUInt64IntKd& RecIdFq2) const {
    if (Store->IsFieldNull(RecIdFq1.Key, FieldId)) { return false; }
    if (Store->IsFieldNull(RecIdFq2.Key, FieldId)) { return false; }
    const uint64 RecVal1 = Store->GetFieldByte(RecIdFq1.Key, FieldId);
    const uint64 RecVal2 = Store->GetFieldByte(RecIdFq2.Key, FieldId);
    if (Asc) { return RecVal1 < RecVal2; }
    else { return RecVal2 < RecVal1; }
}

///////////////////////////////
/// Record filter
TFunRouter<PRecFilter, TRecFilter::TNewF> TRecFilter::NewRouter;

void TRecFilter::Init() {
    Register<TRecFilter>();
    Register<TRecFilterSubsampler>();
    Register<TRecFilterByExists>();
    Register<TRecFilterByRecId>();
    Register<TRecFilterByRecFq>();
    Register<TRecFilterByField>();
    Register<TRecFilterByIndexJoin>();
}

PRecFilter TRecFilter::New(const TWPt<TBase>& Base) {
    return new TRecFilter(Base);
}

PRecFilter TRecFilter::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
    return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

///////////////////////////////
/// Record Filter by subsampling
TRecFilterSubsampler::TRecFilterSubsampler(const TWPt<TBase>& _Base, const int& _Skip):
    TRecFilter(_Base), Skip(_Skip) { }

PRecFilter TRecFilterSubsampler::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TRecFilterSubsampler(Base, ParamVal->GetObjInt("skip", 0));
}

bool TRecFilterSubsampler::Filter(const TRec& Rec) const {
    return NumUpdates++ % (Skip + 1) == 0;
}

///////////////////////////////
/// Record Filter by Record Exists.
TRecFilterByExists::TRecFilterByExists(const TWPt<TBase>& _Base, const TWPt<TStore>& _Store):
    TRecFilter(_Base), Store(_Store) { }

PRecFilter TRecFilterByExists::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    // parse parameters
    TStr StoreNm = ParamVal->GetObjStr("store", "");
    const TWPt<TStore>& Store = Base->GetStoreByStoreNm(StoreNm);
    // create filter
    return new TRecFilterByExists(Base, Store);
}

bool TRecFilterByExists::Filter(const TRec& Rec) const {
    return Store->IsRecId(Rec.GetRecId());
}

///////////////////////////////
/// Record Filter by Record Id.
TRecFilterByRecId::TRecFilterByRecId(const TWPt<TBase>& _Base, const uint64& _MinRecId, const uint64& _MaxRecId):
    TRecFilter(_Base), FilterType(rfRange), MinRecId(_MinRecId), MaxRecId(_MaxRecId) { }

TRecFilterByRecId::TRecFilterByRecId(const TWPt<TBase>& _Base, const TUInt64Set& _RecIdSet, const bool _InP):
    TRecFilter(_Base), FilterType(rfSet), RecIdSet(_RecIdSet), InP(_InP) { }

PRecFilter TRecFilterByRecId::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("minRecId") || ParamVal->IsObjKey("maxRecId")) {
        const uint64 MinRecId = ParamVal->GetObjUInt64("minRecId", 0);
        const uint64 MaxRecId = ParamVal->GetObjUInt64("maxRecId", TUInt64::Mx);
        return new TRecFilterByRecId(Base, MinRecId, MaxRecId);
    } else if (ParamVal->IsObjKey("recIdSet")) {
        TUInt64V RecIdSetV; ParamVal->GetObjUInt64V("recIdSet", RecIdSetV);
        const bool InP = ParamVal->GetObjBool("isComplement", true);
        return new TRecFilterByRecId(Base, TUInt64Set(RecIdSetV), InP);
    }
    throw TQmExcept::New("[TRecFilterByRecId] missing parameters in " + ParamVal->SaveStr());
}

bool TRecFilterByRecId::Filter(const TRec& Rec) const {
    if (FilterType == rfRange) {
        return (MinRecId <= Rec.GetRecId()) && (Rec.GetRecId() <= MaxRecId);
    } else {
        return InP ? RecIdSet.IsKey(Rec.GetRecId()) : !RecIdSet.IsKey(Rec.GetRecId());
    }
}

///////////////////////////////
/// Record Filter by Record Fq.
TRecFilterByRecFq::TRecFilterByRecFq(const TWPt<TBase>& _Base, const int& _MinFq, const int& _MaxFq):
    TRecFilter(_Base), MinFq(_MinFq), MaxFq(_MaxFq) { }

PRecFilter TRecFilterByRecFq::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    const int MinFq = ParamVal->GetObjInt("minFq", 0);
    const int MaxFq = ParamVal->GetObjInt("maxFq", TInt::Mx);
    return new TRecFilterByRecFq(Base, MinFq, MaxFq);
}

bool TRecFilterByRecFq::Filter(const TRec& Rec) const {
    return (MinFq <= Rec.GetRecFq()) && (Rec.GetRecFq() <= MaxFq);
}

///////////////////////////////
/// Record Filter by Field.
TRecFilterByField::TRecFilterByField(const TWPt<TBase>& _Base, const int& _FieldId, const bool& _FilterNullP):
    TRecFilter(_Base), FieldId(_FieldId), FilterNullP(_FilterNullP) { }

PRecFilter TRecFilterByField::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    // get store
    TStr StoreNm = ParamVal->GetObjStr("store");
    // filter out null?
    bool FilterNullP = !ParamVal->GetObjBool("letNullThrough", false);
    const TWPt<TStore>& Store = Base->GetStoreByStoreNm(StoreNm);
    // get field and its type
    QmAssertR(ParamVal->IsObjKey("field"), "[TRecFilterByField] Missing field name");
    TStr FieldNm = ParamVal->GetObjStr("field");
    QmAssertR(Store->IsFieldNm(FieldNm), "[TRecFilterByField] Unknown field '" + FieldNm + "'");
    const int FieldId = Store->GetFieldId(FieldNm);
    const TFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
    // get filter type
    TRecFilterByFieldType Type;
    if (ParamVal->IsObjKey("value")) {
        Type = rfValue;
    } else if (ParamVal->IsObjKey("minValue") || ParamVal->IsObjKey("maxValue")) {
        Type = rfRange;
    } else if (ParamVal->IsObjKey("set")) {
        Type = rfSet;
    } else {
        Type = rfUndef;
    }
    // instantiate appropriate filter
    if (FieldDesc.IsBool() && Type == rfValue) {
        const bool Val = ParamVal->GetObjBool("value", false);
        return new TRecFilterByFieldBool(Base, FieldId, Val, FilterNullP);
    } else if (FieldDesc.IsInt() && Type == rfRange) {
        const int MinVal = ParamVal->GetObjInt("minValue", TInt::Mn);
        const int MaxVal = ParamVal->GetObjInt("maxValue", TInt::Mx);
        return new TRecFilterByFieldInt(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsInt16() && Type == rfRange) {
        const int16 MinVal = (int16)ParamVal->GetObjInt("minValue", TInt16::Mn);
        const int16 MaxVal = (int16)ParamVal->GetObjInt("maxValue", TInt16::Mx);
        return new TRecFilterByFieldInt16(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsInt64() && Type == rfRange) {
        const int64 MinVal = ParamVal->GetObjInt64("minValue", TInt64::Mn);
        const int64 MaxVal = ParamVal->GetObjInt64("maxValue", TInt64::Mx);
        return new TRecFilterByFieldInt64(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsByte() && Type == rfRange) {
        const uchar MinVal = (unsigned)(char)ParamVal->GetObjInt("minValue", TUCh::Mn);
        const uchar MaxVal = (unsigned)(char)ParamVal->GetObjInt("maxValue", TUCh::Mx);
        return new TRecFilterByFieldByte(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsUInt() && Type == rfRange) {
        const uint MinVal = (unsigned)(int)ParamVal->GetObjUInt64("minValue", TUInt::Mn);
        const uint MaxVal = (unsigned)(int)ParamVal->GetObjUInt64("maxValue", TUInt::Mx);
        return new TRecFilterByFieldUInt(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsUInt16() && Type == rfRange) {
        const uint16 MinVal = (uint16)ParamVal->GetObjUInt64("minValue", TUInt16::Mn);
        const uint16 MaxVal = (uint16)ParamVal->GetObjUInt64("maxValue", TUInt16::Mx);
        return new TRecFilterByFieldUInt16(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsUInt64() && Type == rfRange) {
        const uint64 MinVal = ParamVal->GetObjUInt64("minValue", TUInt64::Mn);
        const uint64 MaxVal = ParamVal->GetObjUInt64("maxValue", TUInt64::Mx);
        return new TRecFilterByFieldUInt64(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsFlt() && Type == rfRange) {
        const double MinVal = ParamVal->GetObjNum("minValue", TFlt::Mn);
        const double MaxVal = ParamVal->GetObjNum("maxValue", TFlt::Mx);
        return new TRecFilterByFieldFlt(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsSFlt() && Type == rfRange) {
        const float MinVal = (float)ParamVal->GetObjNum("minValue", TSFlt::Mn);
        const float MaxVal = (float)ParamVal->GetObjNum("maxValue", TSFlt::Mx);
        return new TRecFilterByFieldSFlt(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsStr() && Type == rfValue) {
        const TStr Val = ParamVal->GetObjStr("value");
        return new TRecFilterByFieldStr(Base, FieldId, Val, FilterNullP);
    } else if (FieldDesc.IsStr() && Type == rfRange) {
        const TStr MinVal = ParamVal->GetObjStr("minValue");
        const TStr MaxVal = ParamVal->GetObjStr("maxValue");
        return new TRecFilterByFieldStrRange(Base, FieldId, MinVal, MaxVal, FilterNullP);
    } else if (FieldDesc.IsStr() && Type == rfSet) {
        TStrV StrV; ParamVal->GetObjStrV("set", StrV);
        return new TRecFilterByFieldStrSet(Base, FieldId, TStrSet(StrV), FilterNullP);
    } else if (FieldDesc.IsTm() && Type == rfRange) {
        const uint64 MinVal = TTm::GetWinMSecsFromUnixMSecs(ParamVal->GetObjUInt64("minValue", TUInt64::Mn));
        const uint64 MaxVal = TTm::GetWinMSecsFromUnixMSecs(ParamVal->GetObjUInt64("maxValue", uint64(TUInt64::Mx - 11644473600000LL)));
        return new TRecFilterByFieldTm(Base, FieldId, MinVal, MaxVal, FilterNullP);
    }
    // if not supported, throw exception
    throw TQmExcept::New("[TRecFilterByField] Unsupported field type: " + FieldDesc.GetFieldTypeStr());
}

///////////////////////////////
/// Keep (_FilterNullP = false) or remove (_FilterNullP = true) records that have null in the value of a field.
TRecFilterByFieldNull::TRecFilterByFieldNull(const TWPt<TBase>& _Base, const int& _FieldId, const bool& _RemoveNullValues) :
    TRecFilter(_Base), FieldId(_FieldId), RemoveNullValues(_RemoveNullValues) { }

bool TRecFilterByFieldNull::Filter(const TRec& Rec) const {
    const bool RecNull = Rec.IsFieldNull(FieldId);
    return RecNull == RemoveNullValues;
}

///////////////////////////////
/// Record Filter by Bool Field.
TRecFilterByFieldBool::TRecFilterByFieldBool(const TWPt<TBase>& _Base, const int& _FieldId, const bool& _Val, const bool& _FilterNullP):
    TRecFilterByField(_Base, _FieldId, _FilterNullP), Val(_Val) { }

bool TRecFilterByFieldBool::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const bool RecVal = Rec.GetFieldBool(FieldId);
    return RecVal == Val;
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldInt::TRecFilterByFieldInt(const TWPt<TBase>& _Base, const int& _FieldId, const int& _MinVal,
    const int& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldInt::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const int RecVal = Rec.GetFieldInt(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldInt16::TRecFilterByFieldInt16(const TWPt<TBase>& _Base, const int& _FieldId, const int16& _MinVal,
    const int16& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldInt16::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const int16 RecVal = Rec.GetFieldInt16(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldInt64::TRecFilterByFieldInt64(const TWPt<TBase>& _Base, const int& _FieldId, const int64& _MinVal,
    const int64& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

/// Filter function
bool TRecFilterByFieldInt64::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const int64 RecVal = Rec.GetFieldInt64(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldByte::TRecFilterByFieldByte(const TWPt<TBase>& _Base, const int& _FieldId, const uchar& _MinVal,
    const uchar& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldByte::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uchar RecVal = Rec.GetFieldByte(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldUInt::TRecFilterByFieldUInt(const TWPt<TBase>& _Base, const int& _FieldId, const uint& _MinVal,
    const uint& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldUInt::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uint RecVal = Rec.GetFieldUInt(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldUInt16::TRecFilterByFieldUInt16(const TWPt<TBase>& _Base, const int& _FieldId, const uint16& _MinVal,
    const uint16& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

/// Filter function
bool TRecFilterByFieldUInt16::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uint16 RecVal = Rec.GetFieldUInt16(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by UInt64 Field.
TRecFilterByFieldUInt64::TRecFilterByFieldUInt64(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal,
    const uint64& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldUInt64::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uint64 RecVal = Rec.GetFieldUInt64(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Integer Field.
TRecFilterByFieldIntSafe::TRecFilterByFieldIntSafe(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal,
    const uint64& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

bool TRecFilterByFieldIntSafe::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uint64 RecVal = Rec.GetFieldUInt64Safe(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Numeric Field.
TRecFilterByFieldFlt::TRecFilterByFieldFlt(const TWPt<TBase>& _Base, const int& _FieldId, const double& _MinVal,
        const double& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) {}

bool TRecFilterByFieldFlt::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const double RecVal = Rec.GetFieldFlt(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by Numeric Field.
TRecFilterByFieldSFlt::TRecFilterByFieldSFlt(const TWPt<TBase>& _Base, const int& _FieldId, const float& _MinVal,
    const float& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) {}

bool TRecFilterByFieldSFlt::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const float RecVal = Rec.GetFieldSFlt(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
/// Record Filter by String Field.
TRecFilterByFieldStr::TRecFilterByFieldStr(const TWPt<TBase>& _Base, const int& _FieldId, const TStr& _StrVal, const bool& _FilterNullP):
    TRecFilterByField(_Base, _FieldId, _FilterNullP), StrVal(_StrVal) {}

bool TRecFilterByFieldStr::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const TStr RecVal = Rec.GetFieldStr(FieldId);
    return StrVal == RecVal;
}

///////////////////////////////
/// Record Filter by String Field Range.
TRecFilterByFieldStrRange::TRecFilterByFieldStrRange(const TWPt<TBase>& _Base, const int& _FieldId,
    const TStr& _StrVal, const TStr& _StrValMax, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), StrValMin(_StrVal),
    StrValMax(_StrValMax) { }

/// Filter function
bool TRecFilterByFieldStrRange::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const TStr RecVal = Rec.GetFieldStr(FieldId);
    return (StrValMin <= RecVal) && (RecVal <= StrValMax);
}

///////////////////////////////
/// Record Filter by String Field Set.
TRecFilterByFieldStrSet::TRecFilterByFieldStrSet(const TWPt<TBase>& _Base, const int& _FieldId,  const TStrSet& _StrSet, const bool& _FilterNullP):
    TRecFilterByField(_Base, _FieldId, _FilterNullP), StrSet(_StrSet) { }

/// Filter function
bool TRecFilterByFieldStrSet::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const TStr RecVal = Rec.GetFieldStr(FieldId);
    return StrSet.IsKey(RecVal);
}

///////////////////////////////
/// Record Filter by String Field Set.
TRecFilterByFieldStrSetUsingCodebook::TRecFilterByFieldStrSetUsingCodebook(const TWPt<TBase>& _Base, const int& _FieldId, const PStore& _Store, const TStrSet& StrSet, const bool& _FilterNullP) :
    TRecFilterByField(_Base, _FieldId, _FilterNullP)
{
    // prepare a set of ints representing the string values specified in the StrSet
    for (int KeyId = StrSet.FFirstKeyId(); StrSet.FNextKeyId(KeyId);) {
        IntSet.AddKey(_Store->GetCodebookId(_FieldId, StrSet.GetKey(KeyId)));
    }
}

/// Filter function
bool TRecFilterByFieldStrSetUsingCodebook::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const int RecVal = Rec.GetFieldInt(FieldId);
    return IntSet.IsKey(RecVal);
}


///////////////////////////////
/// Record Filter by Time Field.
TRecFilterByFieldTm::TRecFilterByFieldTm(const TWPt<TBase>& _Base, const int& _FieldId, const uint64& _MinVal,
    const uint64& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP), MinVal(_MinVal), MaxVal(_MaxVal) { }

TRecFilterByFieldTm::TRecFilterByFieldTm(const TWPt<TBase>& _Base, const int& _FieldId, const TTm& _MinVal,
        const TTm& _MaxVal, const bool& _FilterNullP): TRecFilterByField(_Base, _FieldId, _FilterNullP),
        MinVal(_MinVal.IsDef() ? TTm::GetMSecsFromTm(_MinVal) : (uint64)TUInt64::Mn),
        MaxVal(_MaxVal.IsDef() ? TTm::GetMSecsFromTm(_MaxVal) : (uint64)TUInt64::Mx) { }

/// Filter function
bool TRecFilterByFieldTm::Filter(const TRec& Rec) const {
    bool RecNull = Rec.IsFieldNull(FieldId);
    if (RecNull) { return !FilterNullP; }
    const uint64 RecVal = Rec.GetFieldTmMSecs(FieldId);
    return (MinVal <= RecVal) && (RecVal <= MaxVal);
}

///////////////////////////////
// TRecFilterByIndexJoin
TRecFilterByIndexJoin::TRecFilterByIndexJoin(const TWPt<TStore>& Store, const int& JoinId, const uint64& _MinVal,
    const uint64& _MaxVal): TRecFilter(Store->GetBase()), Index(Store->GetBase()->GetIndex()),
    JoinKeyId(Store->GetJoinDesc(JoinId).GetJoinKeyId()), MinVal(_MinVal), MaxVal(_MaxVal) { }

PRecFilter TRecFilterByIndexJoin::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    uint64 MinVal = ParamVal->GetObjUInt64("minRecId", 0);
    uint64 MaxVal = ParamVal->GetObjUInt64("maxRecId", TUInt64::Mx);
    if (ParamVal->IsObjKey("store") && ParamVal->IsObjKey("join")) {
        TStr StoreNm = ParamVal->GetObjStr("store");
        const TWPt<TStore>& Store = Base->GetStoreByStoreNm(StoreNm);
        TStr JoinNm = ParamVal->GetObjStr("join");
        int JoinId = Store->GetJoinId(JoinNm);
        return new TRecFilterByIndexJoin(Store, JoinId, MinVal, MaxVal);
    }
    throw TQmExcept::New("[TRecFilterByIndexJoin] missing parameters in " + ParamVal->SaveStr());
}

bool TRecFilterByIndexJoin::Filter(const TRec& Rec) const {
    // perform join lookup
    TUInt64IntKdV RecIdFqV; Index->SearchGixJoin(JoinKeyId, Rec.GetRecId(), RecIdFqV);
    /// filter
    for (int RecIdFqN = 0; RecIdFqN < RecIdFqV.Len(); RecIdFqN++) {
        const uint64 RecId = RecIdFqV[RecIdFqN].Key;
        if ((MinVal <= RecId) && (RecId <= MaxVal)) {
            return true;
        }
    }
    // if nothing from the range, give up
    return false;
}

///////////////////////////////
/// Record value reader.
void TFieldReader::ParseDate(const TTm& Tm, TStrV& StrV) const {
    TSecTm SecTm = Tm.GetSecTm();
    StrV.Add(SecTm.GetDtYmdStr());
    StrV.Add(SecTm.GetMonthNm());
    StrV.Add(TInt::GetStr(SecTm.GetDayN()));
    StrV.Add(SecTm.GetDayOfWeekNm());
    StrV.Add(SecTm.GetDayPart());
}

bool TFieldReader::IsFlt(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsInt()) {
        return true;
    } else if (FieldDesc.IsInt16()) {
        return true;
    } else if (FieldDesc.IsInt64()) {
        return true;
    } else if (FieldDesc.IsByte()) {
        return true;
    } else if (FieldDesc.IsFlt()) {
        return true;
    } else if (FieldDesc.IsSFlt()) {
        return true;
    } else if (FieldDesc.IsUInt()) {
        return true;
    } else if (FieldDesc.IsUInt16()) {
        return true;
    } else if (FieldDesc.IsUInt64()) {
        return true;
    } else if (FieldDesc.IsBool()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsFltV(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsInt()) {
        return true;
    } else if (FieldDesc.IsInt16()) {
        return true;
    } else if (FieldDesc.IsInt64()) {
        return true;
    } else if (FieldDesc.IsByte()) {
        return true;
    } else if (FieldDesc.IsFlt()) {
        return true;
    } else if (FieldDesc.IsSFlt()) {
        return true;
    } else if (FieldDesc.IsFltPr()) {
        return true;
    } else if (FieldDesc.IsFltV()) {
        return true;
    } else if (FieldDesc.IsUInt()) {
        return true;
    } else if (FieldDesc.IsUInt16()) {
        return true;
    } else if (FieldDesc.IsUInt64()) {
        return true;
    } else if (FieldDesc.IsBool()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsNumSpV(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsNumSpV()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsStr(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsStr()) {
        return true;
    } else if (FieldDesc.IsInt()) {
        return true;
    } else if (FieldDesc.IsInt16()) {
        return true;
    } else if (FieldDesc.IsInt64()) {
        return true;
    } else if (FieldDesc.IsByte()) {
        return true;
    } else if (FieldDesc.IsUInt()) {
        return true;
    } else if (FieldDesc.IsUInt16()) {
        return true;
    } else if (FieldDesc.IsUInt64()) {
        return true;
    } else if (FieldDesc.IsBool()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsStrV(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsStr()) {
        return true;
    } else if (FieldDesc.IsStrV()) {
        return true;
    } else if (FieldDesc.IsInt()) {
        return true;
    } else if (FieldDesc.IsInt16()) {
        return true;
    } else if (FieldDesc.IsInt64()) {
        return true;
    } else if (FieldDesc.IsByte()) {
        return true;
    } else if (FieldDesc.IsIntV()) {
        return true;
    } else if (FieldDesc.IsUInt()) {
        return true;
    } else if (FieldDesc.IsUInt16()) {
        return true;
    } else if (FieldDesc.IsUInt64()) {
        return true;
    } else if (FieldDesc.IsBool()) {
        return true;
    } else if (FieldDesc.IsTm()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsTmMSecs(const TFieldDesc& FieldDesc) {
    if (FieldDesc.IsTm()) {
        return true;
    }
    return false;
}

bool TFieldReader::IsAll(TIsFun IsFun) const {
    for (const TFieldDesc& FieldDesc : FieldDescV) {
        if (!IsFun(FieldDesc)) { return false; }
    }
    return true;
}

double TFieldReader::GetFlt(const TRec& FtrRec) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // assert we have only one field
    Assert(FieldIdV.Len() == 1);
    // extract feature value
    if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldIdV[0])) {
        return 0.0;
    } else if (FieldDescV[0].IsInt()) {
        return (double)FtrRec.GetFieldInt(FieldIdV[0]);
    } else if (FieldDescV[0].IsInt16()) {
        return (double)FtrRec.GetFieldInt16(FieldIdV[0]);
    } else if (FieldDescV[0].IsInt64()) {
        return (double)FtrRec.GetFieldInt64(FieldIdV[0]);
    } else if (FieldDescV[0].IsByte()) {
        return (double)FtrRec.GetFieldByte(FieldIdV[0]);
    } else if (FieldDescV[0].IsFlt()) {
        return FtrRec.GetFieldFlt(FieldIdV[0]);
    } else if (FieldDescV[0].IsSFlt()) {
        return FtrRec.GetFieldSFlt(FieldIdV[0]);
    } else if (FieldDescV[0].IsUInt()) {
        return (double)FtrRec.GetFieldUInt(FieldIdV[0]);
    } else if (FieldDescV[0].IsUInt16()) {
        return (double)FtrRec.GetFieldUInt16(FieldIdV[0]);
    } else if (FieldDescV[0].IsUInt64()) {
        return (double)FtrRec.GetFieldUInt64(FieldIdV[0]);
    } else if (FieldDescV[0].IsBool()) {
        return FtrRec.GetFieldBool(FieldIdV[0]) ? 1.0 : 0.0;
    }
    throw TQmExcept::New("TFieldReader::GetFlt: Field type " + FieldDescV[0].GetFieldTypeStr() + " not supported!");
}

void TFieldReader::GetFltV(const TRec& FtrRec, TFltV& FltV) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // go over all fields and extract values
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        const int FieldId = FieldIdV[FieldIdN];
        const TFieldDesc& FieldDesc = FieldDescV[FieldIdN];
        // extract feature value
        if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldId)) {
            // do nothing
        } else if (FieldDesc.IsInt()) {
            FltV.Add((double)FtrRec.GetFieldInt(FieldId));
        } else if (FieldDesc.IsInt16()) {
            FltV.Add((double)FtrRec.GetFieldInt16(FieldId));
        } else if (FieldDesc.IsInt64()) {
            FltV.Add((double)FtrRec.GetFieldInt64(FieldId));
        } else if (FieldDesc.IsByte()) {
            FltV.Add((double)FtrRec.GetFieldByte(FieldId));
        } else if (FieldDesc.IsFlt()) {
            FltV.Add(FtrRec.GetFieldFlt(FieldId));
        } else if (FieldDesc.IsSFlt()) {
            FltV.Add(FtrRec.GetFieldSFlt(FieldId));
        } else if (FieldDesc.IsFltPr()) {
            TFltPr FltPr = FtrRec.GetFieldFltPr(FieldId);
            FltV.Add(FltPr.Val1); FltV.Add(FltPr.Val2);
        } else if (FieldDesc.IsFltV()) {
            TFltV _FltV; FtrRec.GetFieldFltV(FieldId, _FltV);
            FltV.AddV(_FltV);
        } else if (FieldDesc.IsUInt()) {
            FltV.Add((double)FtrRec.GetFieldUInt(FieldId));
        } else if (FieldDesc.IsUInt16()) {
            FltV.Add((double)FtrRec.GetFieldUInt16(FieldId));
        } else if (FieldDesc.IsUInt64()) {
            FltV.Add((double)FtrRec.GetFieldUInt64(FieldId));
        } else if (FieldDesc.IsBool()) {
            FltV.Add(FtrRec.GetFieldBool(FieldIdV[0]) ? 1.0 : 0.0);
        } else {
            throw TQmExcept::New("TFieldReader::GetFltV: Field type " + FieldDesc.GetFieldTypeStr() +
                " not supported!");
        }
    }
}

void TFieldReader::GetFltV(const PRecSet& FtrRecSet, TFltV& FltV) const {
    // assert store
    Assert(FtrRecSet->GetStoreId() == StoreId);
    // go over all fields and all records
    TWPt<TStore> FtrStore = FtrRecSet->GetStore();
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        const int FieldId = FieldIdV[FieldIdN];
        const TFieldDesc& FieldDesc = FieldDescV[FieldIdN];
        // go over all the records extract feature value
        if (FieldDesc.IsInt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldInt(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsInt16()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldInt16(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsInt64()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldInt64(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsByte()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldByte(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsFlt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add(FtrStore->GetFieldFlt(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsSFlt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldSFlt(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsFltPr()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    TFltPr FltPr = FtrStore->GetFieldFltPr(RecId, FieldId);
                    FltV.Add(FltPr.Val1); FltV.Add(FltPr.Val2);
                }
            }
        } else if (FieldDesc.IsFltV()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    TFltV _FltV; FtrStore->GetFieldFltV(RecId, FieldId, _FltV);
                    FltV.AddV(_FltV);
                }
            }
        } else if (FieldDesc.IsUInt64()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldUInt64(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsUInt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldUInt(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsUInt16()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add((double)FtrStore->GetFieldUInt16(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsBool()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    FltV.Add(FtrStore->GetFieldBool(RecId, FieldId) ? 1.0 : 0.0);
                }
            }
        } else {
            throw TQmExcept::New("TFieldReader::GetFltV: Field type " + FieldDesc.GetFieldTypeStr() +
                " not supported!");
        }
    }
}

void TFieldReader::GetNumSpV(const TRec& FtrRec, TIntFltKdV& NumSpV) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // assert we have only one field
    Assert(FieldIdV.Len() == 1);
    // extract feature value
    if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldIdV[0])) {
        NumSpV.Clr();
    } else if (FieldDescV[0].IsNumSpV()) {
        FtrRec.GetFieldNumSpV(FieldIdV[0], NumSpV);
    } else {
        throw TQmExcept::New("TFieldReader::GetNumSpV: Field type " + FieldDescV[0].GetFieldTypeStr() + " not supported!");
    }
}

TStr TFieldReader::GetStr(const TRec& FtrRec) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // assert we have only one field
    Assert(FieldIdV.Len() == 1);
    // separate case when record passed by reference or value
    if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldIdV[0])) {
        return TStr();
    } else if (FieldDescV[0].IsStr()) {
        return FtrRec.GetFieldStr(FieldIdV[0]);
    } else if (FieldDescV[0].IsInt()) {
        return TInt::GetStr(FtrRec.GetFieldInt(FieldIdV[0]));
    } else if (FieldDescV[0].IsInt16()) {
        return TInt::GetStr(FtrRec.GetFieldInt16(FieldIdV[0]));
    } else if (FieldDescV[0].IsInt64()) {
        return TInt64::GetStr(FtrRec.GetFieldInt64(FieldIdV[0]));
    } else if (FieldDescV[0].IsByte()) {
        return TInt::GetStr(FtrRec.GetFieldByte(FieldIdV[0]));
    } else if (FieldDescV[0].IsUInt()) {
        return TUInt64::GetStr(FtrRec.GetFieldUInt(FieldIdV[0]));
    } else if (FieldDescV[0].IsUInt16()) {
        return TUInt64::GetStr(FtrRec.GetFieldUInt16(FieldIdV[0]));
    } else if (FieldDescV[0].IsUInt64()) {
        return TUInt64::GetStr(FtrRec.GetFieldUInt64(FieldIdV[0]));
    } else if (FieldDescV[0].IsBool()) {
        return FtrRec.GetFieldBool(FieldIdV[0]) ? "Yes" : "No";
    } else {
        throw TQmExcept::New("TFieldReader::GetStr: Field type " + FieldDescV[0].GetFieldTypeStr() +
            " not supported!");
    }
}

void TFieldReader::GetStrV(const TRec& FtrRec, TStrV& StrV) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // go over all fields and extract values
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        const int FieldId = FieldIdV[FieldIdN];
        const TFieldDesc& FieldDesc = FieldDescV[FieldIdN];
        // extract feature value
        if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldId)) {
            // do nothing
        } else if (FieldDesc.IsStr()) {
            StrV.Add(FtrRec.GetFieldStr(FieldId));
        } else if (FieldDesc.IsStrV()) {
            TStrV RecStrV; FtrRec.GetFieldStrV(FieldId, RecStrV);
            StrV.AddV(RecStrV);
        } else if (FieldDesc.IsInt()) {
            StrV.Add(TInt::GetStr(FtrRec.GetFieldInt(FieldId)));
        } else if (FieldDesc.IsInt16()) {
            StrV.Add(TInt::GetStr(FtrRec.GetFieldInt16(FieldId)));
        } else if (FieldDesc.IsInt64()) {
            StrV.Add(TInt64::GetStr(FtrRec.GetFieldInt64(FieldId)));
        } else if (FieldDesc.IsByte()) {
            StrV.Add(TInt::GetStr(FtrRec.GetFieldByte(FieldId)));
        } else if (FieldDesc.IsIntV()) {
            TIntV RecIntV; FtrRec.GetFieldIntV(FieldId, RecIntV);
            for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
                StrV.Add(RecIntV[RecIntN].GetStr()); }
        } else if (FieldDesc.IsUInt()) {
            StrV.Add(TUInt64::GetStr(FtrRec.GetFieldUInt(FieldId)));
        } else if (FieldDesc.IsUInt16()) {
            StrV.Add(TUInt64::GetStr(FtrRec.GetFieldUInt16(FieldId)));
        } else if (FieldDesc.IsUInt64()) {
            StrV.Add(TUInt64::GetStr(FtrRec.GetFieldUInt64(FieldId)));
        } else if (FieldDesc.IsBool()) {
            StrV.Add(FtrRec.GetFieldBool(FieldId) ? "Yes" : "No");
        } else if (FieldDesc.IsTm()) {
            TTm FieldTm; FtrRec.GetFieldTm(FieldId, FieldTm);
            ParseDate(FieldTm, StrV);
        } else {
            throw TQmExcept::New("TFieldReader::GetStrV: Field type " + FieldDesc.GetFieldTypeStr() +
                " not supported!");
        }
    }
}

void TFieldReader::GetStrV(const PRecSet& FtrRecSet, TStrV& StrV) const {
    // assert store
    Assert(FtrRecSet->GetStoreId() == StoreId);
    // go over all fields and all records
    TWPt<TStore> FtrStore = FtrRecSet->GetStore();
    for (int FieldIdN = 0; FieldIdN < FieldIdV.Len(); FieldIdN++) {
        const int FieldId = FieldIdV[FieldIdN];
        const TFieldDesc& FieldDesc = FieldDescV[FieldIdN];
        // go over all the records extract feature value
        if (FieldDesc.IsStr()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(FtrStore->GetFieldStr(RecId, FieldId));
                }
            }
        } else if (FieldDesc.IsStrV()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    TStrV RecStrV; FtrStore->GetFieldStrV(RecId, FieldId, RecStrV);
                    StrV.AddV(RecStrV);
                }
            }
        } else if (FieldDesc.IsInt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TInt::GetStr(FtrStore->GetFieldInt(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsInt16()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TInt::GetStr(FtrStore->GetFieldInt16(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsInt64()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TInt64::GetStr(FtrStore->GetFieldInt64(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsByte()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TInt::GetStr(FtrStore->GetFieldByte(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsIntV()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    TIntV RecIntV; FtrStore->GetFieldIntV(RecId, FieldId, RecIntV);
                    for (int RecIntN = 0; RecIntN < RecIntV.Len(); RecIntN++) {
                        StrV.Add(RecIntV[RecIntN].GetStr()); }
                }
            }
        } else if (FieldDesc.IsUInt()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TUInt64::GetStr(FtrStore->GetFieldUInt(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsUInt16()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TUInt64::GetStr(FtrStore->GetFieldUInt16(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsUInt64()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(TUInt64::GetStr(FtrStore->GetFieldUInt64(RecId, FieldId)));
                }
            }
        } else if (FieldDesc.IsBool()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    StrV.Add(FtrStore->GetFieldBool(RecId, FieldId) ? "Yes" : "No");
                }
            }
        } else if (FieldDesc.IsTm()) {
            for (int RecN = 0; RecN < FtrRecSet->GetRecs(); RecN++) {
                const uint64 RecId = FtrRecSet->GetRecId(RecN);
                if (!FtrStore->IsFieldNull(RecId, FieldId)) {
                    TTm FieldTm; FtrStore->GetFieldTm(RecId, FieldId, FieldTm);
                    ParseDate(FieldTm, StrV);
                }
            }
        } else {
            throw TQmExcept::New("TFieldReader::GetStrV: Field type " + FieldDesc.GetFieldTypeStr() +
                " not supported!");
        }
    }
}

uint64 TFieldReader::GetTmMSecs(const TRec& FtrRec) const {
    // assert store
    Assert(FtrRec.GetStoreId() == StoreId);
    // extract feature value
    if (!FtrRec.IsDef() || FtrRec.IsFieldNull(FieldIdV[0])) {
        return 0;
    } else if (FieldDescV[0].IsTm()) {
        return FtrRec.GetFieldTmMSecs(FieldIdV[0]);
    }
    throw TQmExcept::New("Field type " + FieldDescV[0].GetFieldTypeStr() +
        " not supported by Numeric Feature Extractor!");
}

TStrV TFieldReader::GetDateRange() {
    TStrV ValV;
    // months
    ValV.AddV(TTmInfo::GetMonthNmV());
    // day of month
    for (int DayN = 1; DayN <= 31; DayN++) { ValV.Add(TInt::GetStr(DayN)); }
    // day of week
    ValV.AddV(TTmInfo::GetDayOfWeekNmV());
    // time of day
    ValV.Add("Night"); ValV.Add("Morning"); ValV.Add("Afternoon"); ValV.Add("Evening");
    // hour of day
    for (int HourN = 0; HourN < 24; HourN++) { ValV.Add(TInt::GetStr(HourN)); }
    // done
    return ValV;
}

///////////////////////////////
// QMiner-ResultSet
void TRecSet::GetSampleRecIdV(const int& SampleSize,
    const bool& FqSampleP, TUInt64IntKdV& SampleRecIdFqV) const {

    if (SampleSize == -1) {
        // we ask for all
        SampleRecIdFqV = RecIdFqV;
    } else if (SampleSize == 0) {
        // we ask for nothing
        SampleRecIdFqV.Clr();
    } else if (SampleSize > GetRecs()) {
        // we ask for more than we have, have to give it all
        SampleRecIdFqV = RecIdFqV;
    } else if (FqSampleP) {
        // Weighted random sampling with a reservoir
        // we keep current top candidates in a heap
        THeap<TFltIntKd> TopWgtRecN(SampleSize);
        // function for scoring each element according to its weight
        TRnd Rnd(1);
        auto ScoreFun = [&Rnd](const int& Wgt) {
            return pow(Rnd.GetUniDev(), 1.0 / (double)Wgt);
        };
        // Fill the reservoir with first SampleSize elements
        for (int RecN = 0; RecN < SampleSize; RecN++) {
            const double Wgt = ScoreFun(RecIdFqV[RecN].Dat);
            TopWgtRecN.Add(TFltIntKd(Wgt, RecN));
        }
        TopWgtRecN.MakeHeap();
        // randomly replace existing elements with new ones
        for (int RecN = SampleSize; RecN < GetRecs(); RecN++) {
            const double Wgt = ScoreFun(RecIdFqV[RecN].Dat);
            if (Wgt > TopWgtRecN.TopHeap().Key) {
                // remove current smallest element from the top
                TopWgtRecN.PopHeap();
                // add current one
                TopWgtRecN.PushHeap(TFltIntKd(Wgt, RecN));
            }
        }
        // use remaining top elements as result
        SampleRecIdFqV.Gen(SampleSize, 0);
        for (int RecNN = 0; RecNN < TopWgtRecN.Len(); RecNN++) {
            const int RecN = TopWgtRecN()[RecNN].Dat;
            SampleRecIdFqV.Add(RecIdFqV[RecN]);
        }
    } else {
        // Reservoir sampling
        SampleRecIdFqV.Gen(SampleSize, 0);
        // Fill the reservoir with first SampleSize elements
        for (int RecN = 0; RecN < SampleSize; RecN++) {
            SampleRecIdFqV.Add(RecIdFqV[RecN]);
        }
        // randomly replace existing elements with new ones
        TRnd Rnd(1); const double _SampleSize = (double)SampleSize;
        for (int RecN = SampleSize; RecN < GetRecs(); RecN++) {
            const double Ratio = _SampleSize / (double)RecN;
            // check if we should replace existing element
            if (Rnd.GetUniDev() < Ratio) {
                SampleRecIdFqV[Rnd.GetUniDevInt(SampleSize)] = RecIdFqV[RecN];
            }
        }
    }
}

void TRecSet::LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV) {
    RecIdFqV = SampleRecIdFqV;
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const uint64& RecId, const int& Fq) :
    Store(_Store), FqP(Fq > 1) {

    RecIdFqV.Gen(1, 0); RecIdFqV.Add(TUInt64IntKd(RecId, Fq));
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TUInt64V& RecIdV) : Store(_Store), FqP(false) {
    RecIdFqV.Gen(RecIdV.Len(), 0);
    for (int RecN = 0; RecN < RecIdV.Len(); RecN++) {
        RecIdFqV.Add(TUInt64IntKd(RecIdV[RecN], 0));
    }
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TIntV& RecIdV) : Store(_Store), FqP(false) {
    RecIdFqV.Gen(RecIdV.Len(), 0);
    int Len = RecIdV.Len();
    for (int RecN = 0; RecN < Len; RecN++) {
        RecIdFqV.Add(TUInt64IntKd((uint64)RecIdV[RecN], 0));
    }
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TUInt64IntKdV& _RecIdFqV,
    const bool& _FqP): Store(_Store), FqP(_FqP), RecIdFqV(_RecIdFqV) { }

TRecSet::TRecSet(const TWPt<TBase>& Base, TSIn& SIn) {
    Store = TStore::LoadById(Base, SIn);
    FqP.Load(SIn);
    RecIdFqV.Load(SIn);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV,
        const bool& FqP) {

    return new TRecSet(Store, RecIdFqV, FqP);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store) {
    return new TRecSet(Store, TUInt64V());
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const uint64& RecId, const int& Fq) {
    return new TRecSet(Store, RecId, Fq);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TRec& Rec) {
    Assert(Rec.IsByRef());
    return new TRecSet(Rec.GetStore(), Rec.GetRecId(), 1);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TUInt64V& RecIdV) {
    return new TRecSet(Store, RecIdV);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TIntV& RecIdV) {
    return new TRecSet(Store, RecIdV);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV) {
    return new TRecSet(Store, RecIdFqV, true);
}

void TRecSet::Save(TSOut& SOut) {
    Store->SaveId(SOut);
    FqP.Save(SOut);
    RecIdFqV.Save(SOut);
}

TRec TRecSet::GetRec(const int& RecN) const {
    return TRec(GetStore(), RecIdFqV[RecN].Key, RecIdFqV[RecN].Dat);
}

void TRecSet::GetRecIdV(TUInt64V& RecIdV) const {
    const int Recs = GetRecs();
    RecIdV.Gen(Recs, 0);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdV.Add(GetRecId(RecN));
    }
}

void TRecSet::GetRecIdSet(THashSet<TUInt64>& RecIdSet) const {
    const int Recs = GetRecs();
    RecIdSet.Gen(Recs);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdSet.AddKey(GetRecId(RecN));
    }
}

void TRecSet::GetRecIdFqH(THash<TUInt64, TInt>& RecIdFqH) const {
    const int Recs = GetRecs();
    RecIdFqH.Gen(Recs);
    for (int RecN = 0; RecN < Recs; RecN++) {
        RecIdFqH.AddDat(GetRecId(RecN), GetRecFq(RecN));
    }
}

void TRecSet::PutAllRecFq(const THash<TUInt64, TInt>& RecIdFqH) {
    const int Recs = GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        const uint64 RecId = GetRecId(RecN);
        if (RecIdFqH.IsKey(RecId)) {
            PutRecFq(RecN, RecIdFqH.GetDat(RecId));
        } else {
            PutRecFq(RecN, 0);
        }
    }
}

void TRecSet::SortById(const bool& Asc) {
    if (!RecIdFqV.IsSorted(Asc)) {
        RecIdFqV.Sort(Asc);
    }
}

void TRecSet::SortByFq(const bool& Asc) {
    RecIdFqV.SortCmp(TRecCmpByFq(Asc));
}

void TRecSet::SortByField(const bool& Asc, const int& SortFieldId) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(SortFieldId);
    // apply appropriate comparator
    if (Desc.IsInt()) {
        typedef TKeyDat<TInt, TUInt64IntKd> TItem;
        TVec<TItem> TItemV(RecIdFqV.Len());
        for (int N = 0; N < RecIdFqV.Len(); N++) {
            TItemV.SetVal(N, TItem(Store->GetFieldInt(RecIdFqV[N].Key, SortFieldId), RecIdFqV[N]));
        }
        TItemV.Sort(Asc);
        for (int N = 0; N < TItemV.Len(); N++) {
            RecIdFqV.SetVal(N, TItemV[N].Dat);
        }
    } else if (Desc.IsFlt()) {
        typedef TKeyDat<TFlt, TUInt64IntKd> TItem;
        TVec<TItem> TItemV(RecIdFqV.Len());
        for (int N = 0; N < RecIdFqV.Len(); N++) {
            TItemV.SetVal(N, TItem(Store->GetFieldFlt(RecIdFqV[N].Key, SortFieldId), RecIdFqV[N]));
        }
        TItemV.Sort(Asc);
        for (int N = 0; N < TItemV.Len(); N++) {
            RecIdFqV.SetVal(N, TItemV[N].Dat);
        }
    } else if (Desc.IsByte()) {
        typedef TKeyDat<TUCh, TUInt64IntKd> TItem;
        TVec<TItem> TItemV(RecIdFqV.Len());
        for (int N = 0; N < RecIdFqV.Len(); N++) {
            TItemV.SetVal(N, TItem(Store->GetFieldByte(RecIdFqV[N].Key, SortFieldId), RecIdFqV[N]));
        }
        TItemV.Sort(Asc);
        for (int N = 0; N < TItemV.Len(); N++) {
            RecIdFqV.SetVal(N, TItemV[N].Dat);
        }
    } else if (Desc.IsStr()) {
        typedef TKeyDat<TStr, TUInt64IntKd> TItem;
        TVec<TItem> TItemV(RecIdFqV.Len());
        for (int N = 0; N < RecIdFqV.Len(); N++) {
            TItemV.SetVal(N, TItem(Store->GetFieldStr(RecIdFqV[N].Key, SortFieldId), RecIdFqV[N]));
        }
        TItemV.Sort(Asc);
        for (int N = 0; N < TItemV.Len(); N++) {
            RecIdFqV.SetVal(N, TItemV[N].Dat);
        }
    } else if (Desc.IsTm()) {
        typedef TKeyDat<TUInt64, TUInt64IntKd> TItem;
        TVec<TItem> TItemV(RecIdFqV.Len());
        for (int N = 0; N < RecIdFqV.Len(); N++) {
            TItemV.SetVal(N, TItem(Store->GetFieldTmMSecs(RecIdFqV[N].Key, SortFieldId), RecIdFqV[N]));
        }
        TItemV.Sort(Asc);
        for (int N = 0; N < TItemV.Len(); N++) {
            RecIdFqV.SetVal(N, TItemV[N].Dat);
        }
    } else {
        throw TQmExcept::New("Unsupported sort field type!");
    }
}

void TRecSet::FilterByExists() {
    // apply filter
    FilterBy<TRecFilterByExists>(TRecFilterByExists(Store->GetBase(), Store));
}

void TRecSet::FilterByRecId(const uint64& MinRecId, const uint64& MaxRecId) {
    // apply filter
    FilterBy<TRecFilterByRecId>(TRecFilterByRecId(Store->GetBase(), MinRecId, MaxRecId));
}

void TRecSet::FilterByRecIdSet(const TUInt64Set& RecIdSet) {
    // apply filter
    FilterBy<TRecFilterByRecId>(TRecFilterByRecId(Store->GetBase(), RecIdSet, true));
}

void TRecSet::FilterByFq(const int& MinFq, const int& MaxFq) {
    // apply filter
    FilterBy<TRecFilterByRecFq>(TRecFilterByRecFq(Store->GetBase(), MinFq, MaxFq));
}

void TRecSet::FilterByFieldNull(const int& FieldId, const bool RemoveNullValues) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsNullable(), "The field is not nullable");
    // apply the filter
    FilterBy<TRecFilterByFieldNull>(TRecFilterByFieldNull(Store->GetBase(), FieldId, RemoveNullValues));
}

void TRecSet::FilterByFieldBool(const int& FieldId, const bool& Val) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsBool(), "Wrong field type, boolean expected");
    // apply the filter
    FilterBy<TRecFilterByFieldBool>(TRecFilterByFieldBool(Store->GetBase(), FieldId, Val));
}

void TRecSet::FilterByFieldInt(const int& FieldId, const int& MinVal, const int& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsInt() || (Desc.IsStr() && Desc.IsCodebook()), "Wrong field type, integer or codebook string expected");
    // apply the filter
    FilterBy<TRecFilterByFieldInt>(TRecFilterByFieldInt(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldInt16(const int& FieldId, const int16& MinVal, const int16& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsInt16(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldInt16>(TRecFilterByFieldInt16(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldInt64(const int& FieldId, const int64& MinVal, const int64& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsInt64(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldInt64>(TRecFilterByFieldInt64(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldByte(const int& FieldId, const uchar& MinVal, const uchar& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsByte(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldByte>(TRecFilterByFieldByte(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldUInt(const int& FieldId, const uint& MinVal, const uint& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsUInt(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldUInt>(TRecFilterByFieldUInt(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldUInt16(const int& FieldId, const uint16& MinVal, const uint16& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsUInt16(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldUInt16>(TRecFilterByFieldUInt16(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldFlt(const int& FieldId, const double& MinVal, const double& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsFlt(), "Wrong field type, numeric expected");
    // apply the filter
    FilterBy<TRecFilterByFieldFlt>(TRecFilterByFieldFlt(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldSFlt(const int& FieldId, const float& MinVal, const float& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsFlt(), "Wrong field type, numeric expected");
    // apply the filter
    FilterBy<TRecFilterByFieldSFlt>(TRecFilterByFieldSFlt(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldUInt64(const int& FieldId, const uint64& MinVal, const uint64& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsUInt64(), "Wrong field type, integer expected");
    // apply the filter
    FilterBy<TRecFilterByFieldUInt64>(TRecFilterByFieldUInt64(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldStr(const int& FieldId, const TStr& FldVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
    // apply the filter
    FilterBy<TRecFilterByFieldStr>(TRecFilterByFieldStr(Store->GetBase(), FieldId, FldVal));
}

void TRecSet::FilterByFieldStr(const int& FieldId, const TStr& FldVal, const TStr& FldValMax) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
    // apply the filter
    FilterBy<TRecFilterByFieldStrRange>(TRecFilterByFieldStrRange(Store->GetBase(), FieldId, FldVal, FldValMax));
}

void TRecSet::FilterByFieldStr(const int& FieldId, const TStrSet& ValSet) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
    // apply the filter
    if (Desc.IsCodebook()) {
        FilterBy<TRecFilterByFieldStrSetUsingCodebook>(TRecFilterByFieldStrSetUsingCodebook(Store->GetBase(), FieldId, Store, ValSet));
    } else {
        FilterBy<TRecFilterByFieldStrSet>(TRecFilterByFieldStrSet(Store->GetBase(), FieldId, ValSet));
    }
}

void TRecSet::FilterByFieldTm(const int& FieldId, const uint64& MinVal, const uint64& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsTm() || Desc.IsUInt64(), "Wrong field type, time expected");
    // apply the filter
    FilterBy<TRecFilterByFieldTm>(TRecFilterByFieldTm(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldTm(const int& FieldId, const TTm& MinVal, const TTm& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsTm(), "Wrong field type, time expected");
    // apply the filter
    FilterBy<TRecFilterByFieldTm>(TRecFilterByFieldTm(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldSafe(const int& FieldId, const uint64& MinVal, const uint64& MaxVal) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsTm() || Desc.IsUInt64() || Desc.IsInt64() || Desc.IsUInt() ||
              Desc.IsInt() || Desc.IsUInt16() || Desc.IsInt16() || Desc.IsByte(),
              "Wrong field type, numeric field expected");
    // apply the filter
    FilterBy<TRecFilterByFieldIntSafe>(TRecFilterByFieldIntSafe(Store->GetBase(), FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByIndexJoin(const TWPt<TBase>& Base, const int& JoinId, const uint64& MinVal, const uint64& MaxVal) {
    // get store and field type
    QmAssertR(Store->IsJoinId(JoinId), "Invalid join id");
    // apply the filter
    FilterBy<TRecFilterByIndexJoin>(TRecFilterByIndexJoin(Store, JoinId, MinVal, MaxVal));
}

TVec<PRecSet> TRecSet::SplitByFieldTm(const int& FieldId, const uint64& DiffMSecs) const {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsTm(), "Wrong field type, time expected");
    // split the record set
    return SplitBy<TRecSplitterByFieldTm>(TRecSplitterByFieldTm(Store, FieldId, DiffMSecs));
}

void TRecSet::RemoveRecId(const TUInt64& RecId) {
    const int Recs = GetRecs();
    for (int RecN = 0; RecN < Recs; RecN++) {
        if (GetRecId(RecN) == RecId) {
            RecIdFqV.Del(RecN);
            return;
        }
    }
}

void TRecSet::RemoveRecIdSet(THashSet<TUInt64>& RemoveItemIdSet) {
    // apply filter
    FilterBy<TRecFilterByRecId>(TRecFilterByRecId(Store->GetBase(), RemoveItemIdSet, false));
}

PRecSet TRecSet::Clone() const {
    return new TRecSet(Store, RecIdFqV, FqP);
}

PRecSet TRecSet::GetSampleRecSet(const int& SampleSize, const bool& IgnoreFqP) const {
    TUInt64IntKdV SampleRecIdFqV;
    const bool UseFqP = IgnoreFqP ? false : FqP.Val;
    GetSampleRecIdV(SampleSize, UseFqP, SampleRecIdFqV);
    return new TRecSet(Store, SampleRecIdFqV, FqP);
}

PRecSet TRecSet::GetLimit(const int& Limit, const int& Offset) const {
    if (Offset >= GetRecs()) {
        // offset past number of records, return empty
        return TRecSet::New(Store);
    } else {
        TUInt64IntKdV LimitRecIdFqV;
        if (Limit == -1) {
            // all items after offset
            RecIdFqV.GetSubValV(Offset, GetRecs() - 1, LimitRecIdFqV);
        } else {
            // compute the end
            const int End = TInt::GetMn(Offset + Limit, GetRecs());
            // get all items since offset till end
            RecIdFqV.GetSubValV(Offset, End - 1, LimitRecIdFqV);
        }
        return new TRecSet(Store, LimitRecIdFqV, FqP);
    }
}

PRecSet TRecSet::GetMerge(const PRecSet& RecSet) const {
    PRecSet CloneRecSet = Clone();
    CloneRecSet->Merge(RecSet);
    return CloneRecSet;
}

void TRecSet::Merge(const PRecSet& RecSet) {
    QmAssert(RecSet->GetStoreId() == GetStoreId());
    TUInt64IntKdV MergeRecIdFqV = RecSet->GetRecIdFqV();
    if (!MergeRecIdFqV.IsSorted()) { MergeRecIdFqV.Sort(); }
    if (!RecIdFqV.IsSorted()) { RecIdFqV.Sort(); }
    RecIdFqV.Union(MergeRecIdFqV);
}

void TRecSet::Merge(const TVec<PRecSet>& RecSetV) {
    for (int RsIdx = 0; RecSetV.Len(); RsIdx++) {
        Merge(RecSetV[RsIdx]);
    }
}

PRecSet TRecSet::GetIntersect(const PRecSet& RecSet) {
    QmAssert(RecSet->GetStoreId() == GetStoreId());
    TUInt64IntKdV TargetRecIdFqV = RecSet->GetRecIdFqV();
    if (!TargetRecIdFqV.IsSorted()) { TargetRecIdFqV.Sort(); }
    TUInt64IntKdV _RecIdFqV = GetRecIdFqV();
    if (!_RecIdFqV.IsSorted()) { _RecIdFqV.Sort(); }
    TUInt64IntKdV ResultRecIdFqV;
    TargetRecIdFqV.Intrs(_RecIdFqV, ResultRecIdFqV);
    return new TRecSet(GetStore(), ResultRecIdFqV, false);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const int& JoinId, const int& SampleSize, const bool& IgnoreFqP) const {
    // get join info
    AssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    // prepare joined record sample
    TUInt64IntKdV SampleRecIdKdV;
    const bool UseFqP = IgnoreFqP ? false : FqP.Val;
    GetSampleRecIdV(SampleSize, UseFqP, SampleRecIdKdV);
    const int SampleRecs = SampleRecIdKdV.Len();
    // do the join
    TUInt64IntKdV JoinRecIdFqV;
    if (JoinDesc.IsIndexJoin()) {
        // do join using index
        const int JoinKeyId = JoinDesc.GetJoinKeyId();
        // prepare join query
        TUInt64V RecIdV;
        for (int RecN = 0; RecN < SampleRecs; RecN++) {
            const uint64 RecId = SampleRecIdKdV[RecN].Key;
            RecIdV.Add(RecId);
        }
        // execute join query
        Base->GetIndex()->SearchGixJoin(JoinKeyId, RecIdV, JoinRecIdFqV);
    } else if (JoinDesc.IsFieldJoin()) {
        // do join using store field
        TUInt64H JoinRecIdFqH;
        const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
        const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
        for (int RecN = 0; RecN < SampleRecs; RecN++) {
            const uint64 RecId = SampleRecIdKdV[RecN].Key;
            if (!Store->IsFieldNull(RecId, JoinRecFieldId)) {
                const uint64 JoinRecId = Store->GetFieldUInt64Safe(RecId, JoinRecFieldId);
                int JoinRecFq = 1;
                if (JoinFqFieldId >= 0) {
                    JoinRecFq = (int) Store->GetFieldInt64Safe(RecId, JoinFqFieldId);
                }
                JoinRecIdFqH.AddDat(JoinRecId) += JoinRecFq;
            }
        }
        JoinRecIdFqH.GetKeyDatKdV(JoinRecIdFqV);
        // make sure we are sorted so we are consistent with index join
        JoinRecIdFqV.Sort();
    } else {
        // unknown join type
        throw TQmExcept::New("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
    }
    // create new RecSet
    return new TRecSet(JoinDesc.GetJoinStore(Base), JoinRecIdFqV, true);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm, const int& SampleSize, const bool& IgnoreFqP) const {

    if (Store->IsJoinNm(JoinNm)) {
        return DoJoin(Base, Store->GetJoinId(JoinNm), SampleSize, IgnoreFqP);
    }
    throw TQmExcept::New("Unknown join " + JoinNm);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const {
    PRecSet RecSet = DoJoin(Base, JoinIdV[0].Val1, JoinIdV[0].Val2);
    for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
        RecSet = RecSet->DoJoin(Base, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2);
    }
    return RecSet;
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const {
    return DoJoin(Base, JoinSeq.GetJoinIdV());
}

void TRecSet::Print(const TWPt<TBase>& Base, TSOut& SOut) {
    Store->PrintRecSet(Base, this, SOut);
}

void TRecSet::Print(const TWPt<TBase>& Base, TStr& FNm) {
    Store->PrintRecSet(Base, this, FNm);
}

PJsonVal TRecSet::GetAggrJson() const {
    // output aggregations
    PJsonVal AggrVal = TJsonVal::NewObj();
    for (int AggrN = 0; AggrN < AggrV.Len(); AggrN++) {
        const PAggr& Aggr = AggrV[AggrN];
        AggrVal->AddToObj(Aggr->GetAggrNm(), Aggr->SaveJson());
    }
    return AggrVal;
}

PJsonVal TRecSet::GetJson(const TWPt<TBase>& Base, const int& _MxHits, const int& Offset,
    const bool& FieldsP, const bool& AggrsP, const bool& StoreInfoP,
    const bool& JoinRecsP, const bool& JoinRecFieldsP) const {

    const int MxHits = (_MxHits == -1) ? GetRecs() : _MxHits;
    // export result set as XML
    PJsonVal RecSetVal = TJsonVal::NewObj();
    if (StoreInfoP) {
        PJsonVal StoreVal = TJsonVal::NewObj();
        StoreVal->AddToObj("$id", Store->GetStoreId());
        StoreVal->AddToObj("$name", Store->GetStoreNm());
        StoreVal->AddToObj("$fq", IsFq());
        RecSetVal->AddToObj("$store", StoreVal);
    }
    const int Recs = GetRecs();
    RecSetVal->AddToObj("$hits", Recs);
    // output hits
    int Hits = 0; TJsonValV RecValV;
    for (int RecN = Offset; RecN < Recs; RecN++) {
        // deal with offset
        Hits++; if (Hits > MxHits) { break; }
        // display the record in XML
        PJsonVal RecVal = GetRec(RecN).GetJson(Base, FieldsP, false, JoinRecsP, JoinRecFieldsP);
        // done
        RecValV.Add(RecVal);
    }
    // prepare records
    RecSetVal->AddToObj("records", TJsonVal::NewArr(RecValV));
    // output aggregations
    if (AggrsP) {
        RecSetVal->AddToObj("aggregates", GetAggrJson());
    }
    // finalize
    return RecSetVal;
}

///////////////////////////////
// QMiner-Index-Key
TIndexKey::TIndexKey(const TWPt<TBase>& Base, const uint& _StoreId, const TStr& _KeyNm,
    const TStr& _JoinNm, const TIndexKeyGixType& _GixType): StoreId(_StoreId), KeyNm(_KeyNm),
    WordVocId(-1), Type(oiktInternal), GixType(_GixType), SortType(oikstUndef), JoinNm(_JoinNm) {

    // make sure we have gix type
    QmAssert(GixType != oikgtUndef);
    // assert we got a valid name
    Base->AssertValidNm(KeyNm);
}

TIndexKey::TIndexKey(const TWPt<TBase>& Base, const uint& _StoreId, const TStr& _KeyNm,
    const int& _WordVocId, const TIndexKeyType& _Type, const TIndexKeyGixType& _GixType,
    const TIndexKeySortType& _SortType): StoreId(_StoreId), KeyNm(_KeyNm),
    WordVocId(_WordVocId), Type(_Type), GixType(_GixType), SortType(_SortType) {

    // no internal keys allowed here
    QmAssert(!IsInternal());
    // value or text keys require vocabulary
    if (IsValue() || IsText() || IsTextPos()) { QmAssert(WordVocId >= 0); }
    // value or text require exactly one gix storage type to be set
    if (IsValue() || IsText()) { QmAssert(GixType != oikgtUndef); }
    // location does not need vocabualry
    if (IsLocation()) { QmAssert(WordVocId == -1); }
    // name must be valid
    Base->AssertValidNm(KeyNm);
}

TIndexKey::TIndexKey(TSIn& SIn) : StoreId(SIn), KeyId(SIn),
    KeyNm(SIn), WordVocId(SIn),
    Type(LoadEnum<TIndexKeyType>(SIn)),
    GixType(LoadEnum<TIndexKeyGixType>(SIn)),
    SortType(LoadEnum<TIndexKeySortType>(SIn)),
    FieldIdV(SIn), JoinNm(SIn), Tokenizer(SIn) {}

void TIndexKey::Save(TSOut& SOut) const {
    StoreId.Save(SOut);
    KeyId.Save(SOut); KeyNm.Save(SOut); WordVocId.Save(SOut);
    SaveEnum<TIndexKeyType>(SOut, Type);
    SaveEnum<TIndexKeyGixType>(SOut, GixType);
    SaveEnum<TIndexKeySortType>(SOut, SortType);
    FieldIdV.Save(SOut); JoinNm.Save(SOut);
    Tokenizer.Save(SOut);
}

///////////////////////////////
// QMiner-Index-Word-Vocabulary
uint64 TIndexWordVoc::AddWordStr(const TStr& WordStr) {
    // get id for the (new) word
    const int WordId = WordH.AddKey(WordStr);
    // increase the count for the word, used for autocomplete
    WordH[WordId]++;
    // return the id
    return (uint64)WordId;
}

void TIndexWordVoc::GetWcWordIdV(const TStr& WcStr, TUInt64V& WcWordIdV) {
    WcWordIdV.Clr();
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        if (WordStr.IsWcMatch(WcStr, '*', '?')) {
            WcWordIdV.Add((uint64)WordId);
        }
    }
}

void TIndexWordVoc::GetAllGreaterById(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        if ((uint64)WordId > StartWordId) {
            AllGreaterV.Add((uint64)WordId);
        }
    }
}

void TIndexWordVoc::GetAllGreaterByStr(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        if (WordStr > StartWordStr) {
            AllGreaterV.Add((uint64)WordId);
        }
    }
}

void TIndexWordVoc::GetAllGreaterByFlt(const uint64& StartWordId, TUInt64V& AllGreaterV) {
    AllGreaterV.Clr();
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    TFlt StartWordFlt = StartWordStr.GetFlt();
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        TFlt WordFlt = WordStr.GetFlt();
        if (WordFlt > StartWordFlt) {
            AllGreaterV.Add((uint64)WordId);
        }
    }
}


void TIndexWordVoc::GetAllLessById(const uint64& StartWordId, TUInt64V& AllLessV) {
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        if ((uint64)WordId < StartWordId) {
            AllLessV.Add((uint64)WordId);
        }
    }
}

void TIndexWordVoc::GetAllLessByStr(const uint64& StartWordId, TUInt64V& AllLessV) {
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        if (WordStr < StartWordStr) {
            AllLessV.Add((uint64)WordId);
        }
    }
}

void TIndexWordVoc::GetAllLessByFlt(const uint64& StartWordId, TUInt64V& AllLessV) {
    TStr StartWordStr = WordH.GetKey((int)StartWordId);
    TFlt StartWordFlt = StartWordStr.GetFlt();
    int WordId = WordH.FFirstKeyId();
    while (WordH.FNextKeyId(WordId)) {
        TStr WordStr = WordH.GetKey(WordId);
        TFlt WordFlt = WordStr.GetFlt();
        if (WordFlt < StartWordFlt) {
            AllLessV.Add((uint64)WordId);
        }
    }
}

///////////////////////////////
// QMiner-Index-Vocabulary
PIndexWordVoc& TIndexVoc::GetWordVoc(const int& KeyId) {
    return WordVocV[KeyH[KeyId].GetWordVocId()];
}

const PIndexWordVoc& TIndexVoc::GetWordVoc(const int& KeyId) const {
    return WordVocV[KeyH[KeyId].GetWordVocId()];
}

TIndexVoc::TIndexVoc(TSIn& SIn) {
    KeyH.Load(SIn);
    StoreIdKeyIdSetH.Load(SIn);
    WordVocV.Load(SIn);
}

void TIndexVoc::Save(TSOut& SOut) const {
    KeyH.Save(SOut);
    StoreIdKeyIdSetH.Save(SOut);
    WordVocV.Save(SOut);
}

bool TIndexVoc::IsKeyId(const int& KeyId) const {
    return KeyH.IsKeyId(KeyId);
}

bool TIndexVoc::IsKeyNm(const uint& StoreId, const TStr& KeyNm) const {
    return KeyH.IsKey(TUIntStrPr(StoreId, KeyNm));
}

int TIndexVoc::GetKeyId(const uint& StoreId, const TStr& KeyNm) const {
    QmAssertR(IsKeyNm(StoreId, KeyNm), TStr::Fmt("Unknown key '%s' for store %d", KeyNm.CStr(), StoreId));
    return KeyH.GetKeyId(TUIntStrPr(StoreId, KeyNm));
}

uint TIndexVoc::GetKeyStoreId(const int& KeyId) const {
    return KeyH.GetKey(KeyId).Val1;
}

TStr TIndexVoc::GetKeyNm(const int& KeyId) const {
    return KeyH.GetKey(KeyId).Val2;
}

const TIndexKey& TIndexVoc::GetKey(const int& KeyId) const {
    return KeyH[KeyId];
}

const TIndexKey& TIndexVoc::GetKey(const uint& StoreId, const TStr& KeyNm) const {
    return KeyH.GetDat(TUIntStrPr(StoreId, KeyNm));
}

int TIndexVoc::GetWordVoc(const TStr& WordVocNm) const {
    for (int WordVocId = 0; WordVocId < WordVocV.Len(); WordVocId++) {
        const PIndexWordVoc& WordVoc = WordVocV[WordVocId];
        if (WordVoc->IsWordVocNm() && WordVoc->GetWordVocNm() == WordVocNm) {
            return WordVocId;
        }
    }
    return -1;
}

void TIndexVoc::SetWordVocNm(const int& WordVocId, const TStr& WordVocNm) {
    WordVocV[WordVocId]->SetWordVocNm(WordVocNm);
}

int TIndexVoc::AddKey(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm,
        const int& WordVocId, const TIndexKeyType& Type, const TIndexKeyGixType& GixType,
        const TIndexKeySortType& SortType) {

    // create key
    const int KeyId = KeyH.AddKey(TUIntStrPr(StoreId, KeyNm));
    KeyH[KeyId] = TIndexKey(Base, StoreId, KeyNm, WordVocId, Type, GixType, SortType);
    // tell to the key its ID
    KeyH[KeyId].PutKeyId(KeyId);
    // add the key to the associated store key set
    StoreIdKeyIdSetH.AddDat(StoreId).AddKey(KeyId);
    return KeyId;
}

int TIndexVoc::AddInternalKey(const TWPt<TBase>& Base, const uint& StoreId,
        const TStr& KeyNm, const TStr& JoinNm, const TIndexKeyGixType& GixType) {

    const int KeyId = KeyH.AddKey(TUIntStrPr(StoreId, KeyNm));
    KeyH[KeyId] = TIndexKey(Base, StoreId, KeyNm, JoinNm, GixType);
    KeyH[KeyId].PutKeyId(KeyId);
    return KeyId;
}

void TIndexVoc::AddKeyField(const int& KeyId, const uint& StoreId, const int& FieldId) {
    QmAssert(StoreId == KeyH[KeyId].GetStoreId());
    KeyH[KeyId].AddField(FieldId);
}

bool TIndexVoc::IsStoreKeys(const uint& StoreId) const {
    return StoreIdKeyIdSetH.IsKey(StoreId);
}

const TIntSet& TIndexVoc::GetStoreKeys(const uint& StoreId) const {
    if (StoreIdKeyIdSetH.IsKey(StoreId)) {
        return StoreIdKeyIdSetH.GetDat(StoreId);
    }
    return EmptySet;
}

bool TIndexVoc::IsWordVoc(const int& KeyId) const {
    return KeyH[KeyId].GetWordVocId() != -1;
}

bool TIndexVoc::IsWordId(const int& KeyId, const uint64& WordId) const {
    return GetWordVoc(KeyId)->IsWordId(WordId);
}

bool TIndexVoc::IsWordStr(const int& KeyId, const TStr& WordStr) const {
    return GetWordVoc(KeyId)->IsWordStr(WordStr);
}

uint64 TIndexVoc::GetWords(const int& KeyId) const {
    return GetWordVoc(KeyId)->GetWords();
}

void TIndexVoc::GetAllWordStrV(const int& KeyId, TStrV& WordStrV) const {
    GetWordVoc(KeyId)->GetAllWordV(WordStrV);
}

void TIndexVoc::GetAllWordStrFqV(const int& KeyId, TStrIntPrV& WordStrFqV) const {
    GetWordVoc(KeyId)->GetAllWordFqV(WordStrFqV);
}

TStr TIndexVoc::GetWordStr(const int& KeyId, const uint64& WordId) const {
    const PIndexWordVoc& WordVoc = GetWordVoc(KeyId);
    return WordVoc->GetWordStr(WordId);
}

uint64 TIndexVoc::GetWordFq(const int& KeyId, const uint64& WordId) const {
    return GetWordVoc(KeyId)->GetWordFq(WordId);
}

uint64 TIndexVoc::GetWordId(const int& KeyId, const TStr& WordStr) const {
    return GetWordVoc(KeyId)->GetWordId(WordStr);
}

void TIndexVoc::GetWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) const {
    QmAssert(IsWordVoc(KeyId));
    // tokenize string
    TStrV TokV; GetTokenizer(KeyId)->GetTokens(TextStr, TokV);
    // get word ids for tokens
    WordIdV.Gen(TokV.Len(), 0);
    for (int TokN = 0; TokN < TokV.Len(); TokN++) {
        const TStr& Tok = TokV[TokN];
        if (IsWordStr(KeyId, Tok)) {
            // known word
            WordIdV.Add(GetWordId(KeyId, Tok));
        } else {
            // unknown word
            WordIdV.Add(TUInt64::Mx);
        }
    }
}

uint64 TIndexVoc::AddWordStr(const int& KeyId, const TStr& WordStr) {
    return GetWordVoc(KeyId)->AddWordStr(WordStr);
}

void TIndexVoc::AddWordIdV(const int& KeyId, const TStr& TextStr, TUInt64V& WordIdV) {
    QmAssert(IsWordVoc(KeyId));
    // tokenize string
    TStrV TokV; GetTokenizer(KeyId)->GetTokens(TextStr, TokV);
    WordIdV.Gen(TokV.Len(), 0); const PIndexWordVoc& WordVoc = GetWordVoc(KeyId);
    for (int TokN = 0; TokN < TokV.Len(); TokN++) {
        WordIdV.Add(WordVoc->AddWordStr(TokV[TokN]));
    }
    WordVoc->IncRecs();
}

// void TIndexVoc::AddWordIdV(const int& KeyId, const TStrV& TextStrV, TUInt64V& WordIdV) {
//     QmAssert(IsWordVoc(KeyId));
//     // tokenize string
//     TStrV TokV;
//     const PTokenizer& Tokenizer = GetTokenizer(KeyId);
//     for (int StrN = 0; StrN < TextStrV.Len(); StrN++) {
//         Tokenizer->GetTokens(TextStrV[StrN], TokV);
//     }
//     WordIdV.Gen(TokV.Len(), 0); const PIndexWordVoc& WordVoc = GetWordVoc(KeyId);
//     for (int TokN = 0; TokN < TokV.Len(); TokN++) {
//         WordIdV.Add(WordVoc->AddWordStr(TokV[TokN]));
//     }
//     WordVoc->IncRecs();
// }

void TIndexVoc::GetWcWordIdV(const int& KeyId, const TStr& WcStr, TUInt64V& WcWordIdV) {
    QmAssert(IsWordVoc(KeyId));
    GetWordVoc(KeyId)->GetWcWordIdV(WcStr, WcWordIdV);
}

void TIndexVoc::GetAllGreaterV(const int& KeyId,
    const uint64& StartWordId, TUInt64V& WordIdV) {

    // get all the words matching criteria
    if (KeyH[KeyId].IsSortById()) {
        GetWordVoc(KeyId)->GetAllGreaterById(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByStr()) {
        GetWordVoc(KeyId)->GetAllGreaterByStr(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByFlt()) {
        GetWordVoc(KeyId)->GetAllGreaterByFlt(StartWordId, WordIdV);
    }
}

void TIndexVoc::GetAllLessV(const int& KeyId,
    const uint64& StartWordId, TUInt64V& WordIdV) {

    // get all the words matching criteria
    if (KeyH[KeyId].IsSortById()) {
        GetWordVoc(KeyId)->GetAllLessById(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByStr()) {
        GetWordVoc(KeyId)->GetAllLessByStr(StartWordId, WordIdV);
    } else if (KeyH[KeyId].IsSortByFlt()) {
        GetWordVoc(KeyId)->GetAllLessByFlt(StartWordId, WordIdV);
    }
}

const PTokenizer& TIndexVoc::GetTokenizer(const int& KeyId) const {
    return KeyH[KeyId].GetTokenizer();
}

void TIndexVoc::PutTokenizer(const int& KeyId, const PTokenizer& Tokenizer) {
    KeyH[KeyId].PutTokenizer(Tokenizer);
}

void TIndexVoc::SaveTxt(const TWPt<TBase>& Base, const TStr& FNm) const {
    TFOut FOut(FNm);
    // print store keys
    for (int StoreN = 0; StoreN < Base->GetStores(); StoreN++) {
        const TWPt<TStore>& Store = Base->GetStoreByStoreN(StoreN);
        FOut.PutStrFmt("%s[%d]: ", Store->GetStoreNm().CStr(), int(Store->GetStoreId()));
        const TIntSet& KeySet = GetStoreKeys(Store->GetStoreId());
        int KeyId = KeySet.FFirstKeyId();
        while (KeySet.FNextKeyId(KeyId)) {
            TStr KeyNm = GetKeyNm(KeySet.GetKey(KeyId));
            if (KeyId != 0) { FOut.PutStr(", "); }
            FOut.PutStr(KeyNm);
        }
        FOut.PutLn();
    }
    FOut.PutLn();
    // print keys
    int KeyId = KeyH.FFirstKeyId();
    while (KeyH.FNextKeyId(KeyId)) {
        const uint StoreId = KeyH.GetKey(KeyId).Val1;
        const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
        TStr StoreNm = Store->GetStoreNm();
        TStr KeyNm = KeyH.GetKey(KeyId).Val2;
        const TIndexKey& Key = KeyH[KeyId];
        // print key parameters
        FOut.PutStrFmt("KeyNm: '%s.%s' |Id:%d", StoreNm.CStr(), KeyNm.CStr(), KeyId);
        if (Key.IsValue()) { FOut.PutStr("|Value"); }
        if (Key.IsText()) { FOut.PutStr("|Text"); }
        if (Key.IsLocation()) { FOut.PutStr("|Location"); }
        if (Key.IsInternal()) { FOut.PutStr("|Internal"); }
        if (Key.IsWordVoc()) { FOut.PutStrFmt("|Words:%d:", GetWords(KeyId)); }
        if (Key.IsSortByStr()) { FOut.PutStr("|SortByStr"); }
        if (Key.IsSortById()) { FOut.PutStr("|SortById"); }
        if (Key.IsSortByFlt()) { FOut.PutStr("|SortByFlt"); }
        if (Key.IsFields()) {
            TChA FieldChA;
            for (int FieldN = 0; FieldN < Key.GetFields(); FieldN++) {
                const TFieldDesc& FieldDesc = Store->GetFieldDesc(Key.GetFieldId(FieldN));
                FieldChA += FieldChA.Empty() ? "|" : "; ";
                FieldChA += TStr::Fmt("%s.%s", StoreNm.CStr(), FieldDesc.GetFieldNm().CStr());
            }
            FOut.PutStr(FieldChA);
        }
        FOut.PutStrLn("|");
        // print words
        if (!Key.IsInternal()) {
            FOut.PutStr("  ");
            int ChsPerLn = 2; const int MxChsPerLn = 100;
            TStrV WordStrV; GetAllWordStrV(KeyId, WordStrV);
            for (int WordN = 0; WordN < WordStrV.Len(); WordN++) {
                if (WordN != 0) { FOut.PutStr(", "); ChsPerLn += 2; }
                // check if need to break the line
                const TStr& WordStr = WordStrV[WordN];
                if (ChsPerLn + WordStr.Len() > MxChsPerLn) {
                    FOut.PutStr("\n  "); ChsPerLn = 2;
                }
                // put out word
                FOut.PutStrFmt("'%s'", WordStr.CStr()); ChsPerLn += WordStr.Len() + 2;
            }
            FOut.PutLn();
        }
        FOut.PutLn();
    }
}

///////////////////////////////
// QMiner-Query-Item
void TQueryItem::ParseWordStr(const TStr& WordStr, const TWPt<TIndexVoc>& IndexVoc) {
    // if text key, tokenize the word string
    if (IndexVoc->GetKey(KeyId).IsText()) {
        if (!IsEqual() && !IsNotEqual()) {
            throw TQmExcept::New("Wrong sort type for text Key!");
        }
        IndexVoc->GetWordIdV(KeyId, WordStr, WordIdV);
        // we are done
        return;
    }
    // check for specail comparison ops
    if (IsWildChar()) {
        // get all matching words
        IndexVoc->GetWcWordIdV(KeyId, WordStr, WordIdV);
    } else if (IndexVoc->IsWordStr(KeyId, WordStr)) {
        // word exists, check its ID
        const uint64 WordId = IndexVoc->GetWordId(KeyId, WordStr);
        // extend if necessary
        if (IsGreater()) {
            // extend to higher words
            IndexVoc->GetAllGreaterV(KeyId, WordId, WordIdV);
        } else if (IsLess()) {
            // extend to lower words
            IndexVoc->GetAllLessV(KeyId, WordId, WordIdV);
        } else {
            // otherwise just remember the word
            WordIdV.Add(WordId);
        }
        // throw exception if no word recognized from the query string in case of range queries
        if (WordIdV.Empty() && (IsLess() || IsGreater())) {
            // behavior not defined for this case
            throw TQmExcept::New(TStr::Fmt("Unknown query string %d:'%s'!", KeyId.Val, WordStr.CStr()));
        }
    }
}

TWPt<TStore> TQueryItem::ParseJoins(const TWPt<TBase>& Base, const PJsonVal& JsonVal) {
    TWPt<TStore> JoinStore;
    // first go over all the joins
    PJsonVal JoinVal = JsonVal->GetObjKey("$join");
    if (JoinVal->IsObj()) {
        // we only have one
        JoinStore = ParseJoin(Base, JoinVal);
    } else if (JoinVal->IsArr()) {
        // wow, we have a whole array of them!
        for (int ValN = 0; ValN < JoinVal->GetArrVals(); ValN++) {
            PJsonVal Val = JoinVal->GetArrVal(ValN);
            // make sure we don't have nested arrays
            QmAssertR(Val->IsObj(), "Query: $join expects object as value");
            // handle the join
            if (JoinStore.Empty()) {
                JoinStore = ParseJoin(Base, Val);
            } else {
                TWPt<TStore> _JoinStore = ParseJoin(Base, Val);
                QmAssertR(_JoinStore->GetStoreId() == JoinStore->GetStoreId(), "Query: store mismatch");
            }
        }
    } else {
        throw TQmExcept::New("Query: bad join parameter: '" + TJsonVal::GetStrFromVal(JsonVal) + "'");
    }
    return JoinStore;
}

TWPt<TStore> TQueryItem::ParseJoin(const TWPt<TBase>& Base, const PJsonVal& JoinVal) {
    // first handle the subordinate items
    QmAssertR(JoinVal->IsObjKey("$query"), "Query: $join expects object with $query parameter");
    TQueryItem SubQuery(Base, JoinVal->GetObjKey("$query"));
    // get the store
    const TWPt<TStore>& Store = Base->GetStoreByStoreId(SubQuery.GetStoreId(Base));
    // now handle the join ID
    QmAssertR(JoinVal->IsObjKey("$name") && JoinVal->GetObjKey("$name")->IsStr(),
        "Query: $join expects $name parameter with string as value");
    const int _JoinId = Store->GetJoinId(JoinVal->GetObjKey("$name")->GetStr());
    // and sample size
    if (JoinVal->IsObjKey("$sample")) {
        QmAssertR(JoinVal->GetObjKey("$sample")->IsNum(), "Query: $sample expects number as value");
    }
    const int _SampleSize = JoinVal->IsObjKey("$sample") ? TFlt::Round(JoinVal->GetObjKey("$sample")->GetNum()) : -1;
    // add the join to the list
    ItemV.Add(TQueryItem(_JoinId, _SampleSize, SubQuery));
    // get target field
    const uint JoinStoreId = Store->GetJoinDesc(_JoinId).GetJoinStoreId();
    return Base->GetStoreByStoreId(JoinStoreId);
}

TWPt<TStore> TQueryItem::ParseFrom(const TWPt<TBase>& Base, const PJsonVal& JsonVal) {
    // get the store
    QmAssertR(JsonVal->GetObjKey("$from")->IsStr(), "Query: $from expects string as value");
    TStr StoreNm = JsonVal->GetObjKey("$from")->GetStr();
    QmAssertR(Base->IsStoreNm(StoreNm), "Query: unknown store " + StoreNm);
    return Base->GetStoreByStoreNm(StoreNm);
}

uint64 TQueryItem::ParseTm(const PJsonVal& JsonVal) {
    if (JsonVal->IsStr()) {
        // parse date as string
        TStr TmStr = JsonVal->GetStr();
        TTm Tm = TTm::GetTmFromWebLogDateTimeStr(TmStr, '-', ':', '.', 'T');
        QmAssertR(Tm.IsDef(), "Query: Unsupported time format, use ISO instead: " + TmStr);
        return TTm::GetMSecsFromTm(Tm);
    } else if (JsonVal->IsNum()) {
        // parse date as unix time stamp in miliseconds
        const double UnixMSecs = JsonVal->GetNum();
        const double WinMSecs = UnixMSecs + 11644473600000.0;
        return (uint64)WinMSecs;
    } else {
        // TODO: if you need annoter type to be supported when parsing time queries,
        // feel free to implement it here :-)
        throw TQmExcept::New("Query: Unsupported datetime value " + JsonVal->SaveStr());
    }
}

void TQueryItem::ParseKeys(const TWPt<TBase>& Base, const TWPt<TStore>& Store,
        const PJsonVal& JsonVal, const bool& IgnoreOrP) {

    // go over other keys and create their corresponding items
    for (int KeyN = 0; KeyN < JsonVal->GetObjKeys(); KeyN++) {
        // read the key
        TStr KeyNm; PJsonVal KeyVal;
        JsonVal->GetObjKeyVal(KeyN, KeyNm, KeyVal);
        // check type
        if (KeyNm.StartsWith("$")) {
            // special values
            if (KeyNm == "$or") {
                if (!IgnoreOrP) {
                    // make sure value is an array ...
                    QmAssertR(KeyVal->IsArr(), "Query: $or expects array as value");
                    // ... handle subordinate items
                    TQueryItemV OrItemV;
                    for (int ValN = 0; ValN < KeyVal->GetArrVals(); ValN++) {
                        PJsonVal Val = KeyVal->GetArrVal(ValN);
                        QmAssertR(Val->IsObj(), "Query: $or expects objects in array.");
                        OrItemV.Add(TQueryItem(Base, Store, Val));
                    }
                    ItemV.Add(TQueryItem(oqitOr, OrItemV));
                }
            } else if (KeyNm == "$not") {
                QmAssertR(KeyVal->IsObj(), "Query: $not expects object as value");
                // handle subordinate items
                ItemV.Add(TQueryItem(oqitNot, TQueryItem(Base, Store, KeyVal)));
            } else if (KeyNm == "$id") {
                QmAssertR(KeyVal->IsNum(), "Query: unsupported $id value");
                const uint64 _RecId = KeyVal->GetUInt64();
                const uint64 RecId = Store->IsRecId(_RecId) ? _RecId : TUInt64::Mx;
                ItemV.Add(TQueryItem(Store, RecId));
            } else if (KeyNm == "$name") {
                QmAssertR(KeyVal->IsStr(), "Query: unsupported $name value");
                TStr RecNm = KeyVal->GetStr();
                const uint64 RecId = Store->IsRecNm(RecNm) ? Store->GetRecId(RecNm) : TUInt64::Mx;
                ItemV.Add(TQueryItem(Store, RecId));
            } else if (KeyNm == "$join") {
                // ignore
            } else if (KeyNm == "$from") {
                // ignore, should be already handled before, we just double-check stores match
                QmAssertR(KeyVal->IsStr() && Store->GetStoreNm() == KeyVal->GetStr(), "Query: store mismatch");
            } else if (KeyNm == "$aggr") {
            } else if (KeyNm == "$sort") {
            } else if (KeyNm == "$limit") {
            } else if (KeyNm == "$offset") {
            } else {
                throw TQmExcept::New("Query: unknown parameter " + KeyNm);
            }
        } else {
            // field query
            ItemV.Add(TQueryItem(Base, Store, KeyNm, KeyVal));
        }
    }
    // if no keys, query returns all records from the store
    if (ItemV.Empty() && (Type == oqitAnd)) {
        Type = oqitStore;
        StoreId = Store->GetStoreId();
    }
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const PJsonVal& JsonVal) {
    if (JsonVal->IsObj()) {
        // default for object queries is AND
        Type = oqitAnd; TWPt<TStore> Store;
        // check if we have any join
        if (JsonVal->IsObjKey("$join")) {
            // we have a join, handle this first
            Store = ParseJoins(Base, JsonVal);
        }
        // check if result store is specified
        if (JsonVal->IsObjKey("$from")) {
            if (Store.Empty()) {
                // no join, we use from to derive our store
                Store = ParseFrom(Base, JsonVal);
            } else {
                // we already know the store, do the double-check
                TWPt<TStore> FromStore = ParseFrom(Base, JsonVal);
                QmAssertR(FromStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");
            }
        }
        // check if there is any OR and no defined store so far
        bool IgnoreOrP = false;
        if (JsonVal->IsObjKey("$or") && Store.Empty()) {
            // we have an OR query
            IgnoreOrP = true; // so we don't pares it again in ParseKeys below
            TQueryItemV OrItemV;
            PJsonVal OrVal = JsonVal->GetObjKey("$or");
            QmAssertR(OrVal->IsArr(), "Query: $or expects array as value");
            for (int ValN = 0; ValN < OrVal->GetArrVals(); ValN++) {
                PJsonVal Val = OrVal->GetArrVal(ValN);
                QmAssertR(Val->IsObj(), "Query: $or expects objects in array.");
                OrItemV.Add(TQueryItem(Base, Val));
                // do the check on return store matching with what we saw so far
                const uint OrStoreId = OrItemV.Last().GetStoreId(Base);
                if (Store.Empty()) { Store = Base->GetStoreByStoreId(OrStoreId); } else { QmAssertR(OrStoreId == Store->GetStoreId(), "Query: store mismatch"); }
            }
            // remember the or
            ItemV.Add(TQueryItem(oqitOr, OrItemV));
        }
        // handle any remaining keys, but only if we know the store
        if (!Store.Empty()) {
            // parse the rest of the keys
            ParseKeys(Base, Store, JsonVal, IgnoreOrP);
        } else {
            // by now we should know what is the result store
            throw TQmExcept::New("Query: underspecified query");
        }
    } else {
        throw TQmExcept::New("Query: expected an object: '" + TJsonVal::GetStrFromVal(JsonVal) + "'");
    }
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const PJsonVal& JsonVal) {
    QmAssert(JsonVal->IsObj());
    // default for object queries is AND
    Type = oqitAnd;
    if (JsonVal->IsObjKey("$join")) {
        // we have a join
        TWPt<TStore> JoinStore = ParseJoins(Base, JsonVal);
        QmAssertR(JoinStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");
    }
    if (JsonVal->IsObjKey("$from")) {
        // we already know the store, do the doublecheck
        TWPt<TStore> FromStore = ParseFrom(Base, JsonVal);
        QmAssertR(FromStore->GetStoreId() == Store->GetStoreId(), "Query: store mismatch");
    }
    // parse the rest of the keys
    ParseKeys(Base, Store, JsonVal, false);
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const TStr& KeyNm, const PJsonVal& KeyVal) {
    // check key exists for the specified store
    TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
    QmAssertR(IndexVoc->IsKeyNm(Store->GetStoreId(), KeyNm), "Query: unknown key " + KeyNm);
    // get key and its type
    const TIndexKey& Key = IndexVoc->GetKey(Store->GetStoreId(), KeyNm);
    // check for possible types of queries
    if (KeyVal->IsObj() && KeyVal->IsObjKey("$or")) {
        // we are an OR query of multiple subqueries on the same key
        PJsonVal ObjVals = KeyVal->GetObjKey("$or");
        QmAssertR(ObjVals->IsArr(), "Query: $or as value requires an array of potential values");
        Type = oqitOr;
        for (int ValN = 0; ValN < ObjVals->GetArrVals(); ValN++) {
            ItemV.Add(TQueryItem(Base, Store, KeyNm, ObjVals->GetArrVal(ValN)));
        }
    } else if (KeyVal->IsArr()) {
        // we are an AND query of multiple subqueries on the same key
        Type = oqitAnd;
        for (int ValN = 0; ValN < KeyVal->GetArrVals(); ValN++) {
            PJsonVal Val = KeyVal->GetArrVal(ValN);
            // make sure we don't have nested arreys
            QmAssertR(Val->IsStr() || Val->IsObj(),
                "Query: Multiple conditions for a key must be string or object");
            // handle each value
            ItemV.Add(TQueryItem(Base, Store, KeyNm, Val));
        }
    } else if (Key.IsValue() || Key.IsText()) {
        // we have a direct inverted index query
        KeyId = Key.GetKeyId();
        Type = oqitGix;
        // check how it is phrased
        if (KeyVal->IsStr()) {
            // plain string, must be equal
            CmpType = oqctEqual;
            // get target word id(s)
            ParseWordStr(KeyVal->GetStr(), IndexVoc);
        } else if (KeyVal->IsObj() && KeyVal->IsObjKey("$ne")) {
            // not-equal query
            QmAssertR(KeyVal->GetObjKey("$ne")->IsStr(), "Query: $ne value must be string");
            CmpType = oqctNotEqual;
            // get target word id(s)
            ParseWordStr(KeyVal->GetObjKey("$ne")->GetStr(), IndexVoc);
        } else if (KeyVal->IsObj() && KeyVal->IsObjKey("$gt")) {
            // greater-than query
            QmAssertR(KeyVal->GetObjKey("$gt")->IsStr(), "Query: $gt value must be string");
            CmpType = oqctGreater;
            // identify all words that are greater
            ParseWordStr(KeyVal->GetObjKey("$gt")->GetStr(), IndexVoc);
        } else if (KeyVal->IsObj() && KeyVal->IsObjKey("$lt")) {
            // less-than query
            QmAssertR(KeyVal->GetObjKey("$lt")->IsStr(), "Query: $lt value must be string");
            CmpType = oqctLess;
            // identify all words that are smaller
            ParseWordStr(KeyVal->GetObjKey("$lt")->GetStr(), IndexVoc);
        } else if (KeyVal->IsObj() && KeyVal->IsObjKey("$wc")) {
            // wildchars interparted as or with all possibilities
            QmAssertR(KeyVal->GetObjKey("$wc")->IsStr(), "Query: $wc value must be string");
            CmpType = oqctWildChar;
            // identify possibilities
            ParseWordStr(KeyVal->GetObjKey("$wc")->GetStr(), IndexVoc);
        } else {
            throw TQmExcept::New("Query: Invalid key definition: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
        }
    } else if (Key.IsLocation()) {
        // remember key id
        KeyId = Key.GetKeyId();
        if (KeyVal->IsObj() && KeyVal->IsObjKey("$location")) {
            // we are hiting location index
            Type = oqitGeo;
            // we have a location query, parse out location
            PJsonVal LocVal = KeyVal->GetObjKey("$location");
            QmAssertR(LocVal->IsArr(), "$location requires array with two coordinates");
            QmAssertR(LocVal->GetArrVals() == 2, "$location requires array with two coordinates");
            Loc.Val1 = LocVal->GetArrVal(0)->GetNum(); Loc.Val2 = LocVal->GetArrVal(1)->GetNum();
            // default values for parameters
            LocRadius = -1.0; LocLimit = 100;
            // parase out additional parameters
            if (KeyVal->IsObjKey("$radius")) {
                PJsonVal RadiusVal = KeyVal->GetObjKey("$radius");
                LocRadius = RadiusVal->GetNum();
            }
            if (KeyVal->IsObjKey("$limit")) {
                PJsonVal RadiusVal = KeyVal->GetObjKey("$limit");
                LocLimit = TFlt::Round(RadiusVal->GetNum());
                if (LocLimit <= 0) { throw TQmExcept::New("Query: $limit must be greater then zero"); }
            }
        } else {
            throw TQmExcept::New("Query: invalid value for location key: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
        }
    } else if (Key.IsLinear()) {
        // remember key id
        KeyId = Key.GetKeyId();
        // check if we have any border
        if (KeyVal->IsObj()) {
            // key is index using btree, check field type and extract type-appropriate range
            // by default range is always (-inf, inf), unless specified explicitly
            if (Key.IsSortAsInt()) {
                Type = oqitRangeInt;
                RangeIntMnMx = TIntPr(KeyVal->GetObjInt("$gt", TInt::Mn), KeyVal->GetObjInt("$lt", TInt::Mx));
            } else if (Key.IsSortAsInt16()) {
                Type = oqitRangeInt16;
                RangeInt16MnMx = TInt16Pr(KeyVal->GetObjInt("$gt", TInt16::Mn), KeyVal->GetObjInt("$lt", TInt16::Mx));
            } else if (Key.IsSortAsInt64()) {
                Type = oqitRangeInt64;
                RangeInt64MnMx = TInt64Pr(KeyVal->GetObjInt64("$gt", TInt64::Mn), KeyVal->GetObjInt64("$lt", TInt64::Mx));
            } else if (Key.IsSortAsByte()) {
                Type = oqitRangeByte;
                RangeUChMnMx = TUChPr((uchar)KeyVal->GetObjInt("$gt", TUCh::Mn), (uchar)KeyVal->GetObjInt("$lt", TUCh::Mx));
            } else if (Key.IsSortAsUInt()) {
                Type = oqitRangeUInt;
                uint64 low = (uint64)KeyVal->GetObjNum("$gt", TUInt::Mn);
                uint64 high = (uint64)KeyVal->GetObjNum("$lt", TUInt::Mx);
                RangeUIntMnMx = TUIntUIntPr((uint)low, (uint)high);
                //RangeUIntMnMx = TUIntUIntPr((uint)KeyVal->GetObjUInt64("$gt", TUInt::Mn), (uint)KeyVal->GetObjUInt64("$lt", TUInt::Mx));
            } else if (Key.IsSortAsUInt16()) {
                Type = oqitRangeUInt16;
                RangeUInt16MnMx = TUInt16Pr((uint16)KeyVal->GetObjUInt64("$gt", TUInt16::Mn), (uint16)KeyVal->GetObjUInt64("$lt", TUInt16::Mx));
            } else if (Key.IsSortAsUInt64()) {
                Type = oqitRangeUInt64;
                RangeUInt64MnMx = TUInt64Pr(KeyVal->GetObjUInt64("$gt", TUInt64::Mn), KeyVal->GetObjUInt64("$lt", TUInt64::Mx));
            } else if (Key.IsSortAsTm()) {
                Type = oqitRangeTm;
                RangeUInt64MnMx = TUInt64Pr(TUInt64::Mn, TUInt64::Mx);
                // check if we have lower bound
                if (KeyVal->IsObjKey("$gt")) { RangeUInt64MnMx.Val1 = ParseTm(KeyVal->GetObjKey("$gt")); }
                if (KeyVal->IsObjKey("$lt")) { RangeUInt64MnMx.Val2 = ParseTm(KeyVal->GetObjKey("$lt")); }
            } else if (Key.IsSortAsSFlt()) {
                Type = oqitRangeSFlt;
                RangeSFltMnMx = TSFltPr((float)KeyVal->GetObjNum("$gt", TSFlt::Mn), (float)KeyVal->GetObjNum("$lt", TSFlt::Mx));
            } else if (Key.IsSortAsFlt()) {
                Type = oqitRangeFlt;
                RangeFltMnMx = TFltPr(KeyVal->GetObjNum("$gt", TFlt::Mn), KeyVal->GetObjNum("$lt", TFlt::Mx));
            }
        } else if (Key.IsSortAsTm() && (KeyVal->IsStr() || KeyVal->IsNum())) {
            // we are given exact date time, make it a range query with both edges equal
            Type = oqitRangeUInt64;
            RangeUInt64MnMx.Val1 = RangeUInt64MnMx.Val2 = ParseTm(KeyVal);
        } else if (KeyVal->IsNum()) {
            QmAssertR(
                Key.IsSortAsInt() || Key.IsSortAsInt16() || Key.IsSortAsInt64() || Key.IsSortAsByte() || Key.IsSortAsUInt64() || Key.IsSortAsUInt() || Key.IsSortAsUInt16() || Key.IsSortAsFlt() || Key.IsSortAsSFlt(),
                "Query: wrong key value for non-integer key " + KeyNm);
            // we are given exact number, make it a range query with both edges equal
            if (Key.IsSortAsInt()) {
                Type = oqitRangeInt;
                RangeIntMnMx.Val1 = RangeIntMnMx.Val2 = KeyVal->GetInt();
            } else if (Key.IsSortAsInt16()) {
                Type = oqitRangeInt16;
                RangeInt16MnMx.Val1 = RangeInt16MnMx.Val2 = (int16)KeyVal->GetInt();
            } else if (Key.IsSortAsInt64()) {
                Type = oqitRangeInt64;
                RangeInt64MnMx.Val1 = RangeInt64MnMx.Val2 = KeyVal->GetInt64();
            } else if (Key.IsSortAsByte()) {
                Type = oqitRangeByte;
                RangeUChMnMx.Val1 = RangeUChMnMx.Val2 = (uchar)KeyVal->GetInt();
            } else if (Key.IsSortAsUInt()) {
                Type = oqitRangeUInt;
                RangeUIntMnMx.Val1 = RangeUIntMnMx.Val2 = (uint)KeyVal->GetUInt64();
            } else if (Key.IsSortAsUInt16()) {
                Type = oqitRangeUInt16;
                RangeUInt16MnMx.Val1 = RangeUInt16MnMx.Val2 = (uint16)KeyVal->GetUInt64();
            } else if (Key.IsSortAsUInt64()) {
                Type = oqitRangeUInt64;
                RangeUInt64MnMx.Val1 = RangeUInt64MnMx.Val2 = KeyVal->GetUInt64();
            } else if (Key.IsSortAsSFlt()) {
                Type = oqitRangeSFlt;
                RangeSFltMnMx.Val1 = RangeSFltMnMx.Val2 = (float)KeyVal->GetNum();
            } else if (Key.IsSortAsFlt()) {
                Type = oqitRangeFlt;
                RangeFltMnMx.Val1 = RangeFltMnMx.Val2 = KeyVal->GetNum();
            }
        }
    } else {
        throw TQmExcept::New("Query: Invalid key definition: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
    }
}

TQueryItem::TQueryItem(const TWPt<TStore>& Store, const uint64& RecId): Type(oqitRec), Rec(Store, RecId) { }

TQueryItem::TQueryItem(const TRec& _Rec): Type(oqitRec), Rec(_Rec) { }

TQueryItem::TQueryItem(const PRecSet& _RecSet): Type(oqitRecSet), RecSet(_RecSet) {
    RecSet->SortById();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, const uint64& WordId,
    const TQueryCmpType& _CmpType): Type(oqitGix), KeyId(_KeyId), WordIdV(TUInt64V::GetV(WordId)),
    CmpType(_CmpType) { }

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
    const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitGix)  {

    // read the Key
    KeyId = _KeyId;
    QmAssertR(Base->GetIndexVoc()->IsKeyId(KeyId), "Unknown Key ID: " + KeyId.GetStr());
    // parse the word string
    ParseWordStr(WordStr, Base->GetIndexVoc());
    // read the sort type
    CmpType = _CmpType;
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm,
    const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitGix)  {

    // get the key
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
    KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
    // parse the word string
    ParseWordStr(WordStr, Base->GetIndexVoc());
    // read sort type
    CmpType = _CmpType;
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& KeyNm,
    const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitGix)  {

    // get the key
    const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
    KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
    // parse the word string
    ParseWordStr(WordStr, Base->GetIndexVoc());
    // read sort type
    CmpType = _CmpType;
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
    const TFltPr& _Loc, const int& _LocLimit, const double& _LocRadius) :
    Type(oqitGeo), KeyId(_KeyId), Loc(_Loc), LocRadius(_LocRadius),
    LocLimit(_LocLimit) {

    QmAssert(LocLimit > 0);
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId,
    const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit,
    const double& _LocRadius) : Type(oqitGeo), Loc(_Loc),
    LocRadius(_LocRadius), LocLimit(_LocLimit) {

    QmAssert(LocLimit > 0);
    // get the key
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
    KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm,
    const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit,
    const double& _LocRadius) : Type(oqitGeo), Loc(_Loc),
    LocRadius(_LocRadius), LocLimit(_LocLimit) {

    QmAssert(LocLimit > 0);
    // get the key
    const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
    KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
}

TQueryItem::TQueryItem(const TQueryItemType& _Type) : Type(_Type) {
    QmAssert(Type == oqitAnd || Type == oqitOr);
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item) :
    Type(_Type), ItemV(1, 0) {
    ItemV.Add(Item);
    QmAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item1,
    const TQueryItem& Item2) : Type(_Type), ItemV(2, 0) {
    ItemV.Add(Item1); ItemV.Add(Item2);
    QmAssert(Type == oqitAnd || Type == oqitOr);
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItemV& _ItemV) :
    Type(_Type), ItemV(_ItemV) {
    QmAssert(Type == oqitAnd || Type == oqitOr);
}

TQueryItem::TQueryItem(const int& _JoinId, const int& _SampleSize, const TQueryItem& Item) :
    Type(oqitJoin), ItemV(1, 0), JoinId(_JoinId), SampleSize(_SampleSize) {
    ItemV.Add(Item);
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& JoinNm, const int& _SampleSize,
    const TQueryItem& Item) : Type(oqitJoin), ItemV(1, 0), SampleSize(_SampleSize) {

    ItemV.Add(Item);
    // get join id
    const uint StoreId = Item.GetStoreId(Base);
    const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
    JoinId = Store->GetJoinId(JoinNm);
}

uint TQueryItem::GetStoreId(const TWPt<TBase>& Base) const {
    if (IsGix() || IsGeo() || IsRange()) {
        // when in the leaf, life is easy
        return Base->GetIndexVoc()->GetKeyStoreId(KeyId);
    } else if (IsRecSet()) {
        // we have a record set in the leaf
        return RecSet->GetStoreId();
    } else if (IsRec()) {
        // we have a record in the leaf
        return Rec.GetStoreId();
    } else if (IsJoin()) {
        // there must be exactly one subordiante node
        QmAssertR(ItemV.Len() == 1, "QueryItem: Join node must have exactly one child");
        // get store id of subordinate
        const uint StoreId = ItemV[0].GetStoreId(Base);
        // and check where does the join bring us to
        const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
        return Store->GetJoinDesc(JoinId).GetJoinStoreId();
    } else if (IsStore()) {
        // we are returning complete store
        return StoreId;
    } else {
        // if there are no subordinate nodes, we have a problem
        QmAssertR(!ItemV.Empty(), "QueryItem: Non-leaf node without children");
        // otherwise must ask subordinate nodes and make sure they agree
        // first item defines the standard
        uint StoreId = ItemV[0].GetStoreId(Base);
        for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
            if (ItemV[ItemN].GetStoreId(Base) != StoreId) {
                throw TQmExcept::New("QueryItem: children nodes return records from different stores");
            }
        }
        // we survived, return varified store id
        return StoreId;
    }
}

TWPt<TStore> TQueryItem::GetStore(const TWPt<TBase>& Base) const {
    return Base->GetStoreByStoreId(GetStoreId(Base));
}

bool TQueryItem::IsFq() const {
    if (IsGix() || IsGeo()) {
        // always weighted when only one key
        return true;
    } else if (IsAnd() && ItemV.Len() == 1) {
        // we have only one sub-query, check its status
        return ItemV[0].IsFq();
    } else if (IsOr()) {
        // or is weighted when all it's elements are
        bool FqP = true;
        for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
            FqP = FqP && ItemV[ItemN].IsFq();
        }
        return FqP;
    } else if (IsJoin()) {
        // joins are also weighted
        return true;
    }
    // in other cases, nope
    return false;
}

void TQueryItem::GetKeyWordV(TKeyWordV& KeyWordPrV) const {
    KeyWordPrV.Clr();
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        KeyWordPrV.Add(TKeyWord(KeyId, WordIdV[WordIdN]));
    }
}

void TQueryItem::Optimize() {
    for (TQueryItem& Child : ItemV) {
        // optimize down the tree
        Child.Optimize();
        // and optimize localy
        if (Child.ItemV.Len() == 1 && (Child.Type == oqitAnd || Child.Type == oqitOr)) {
            // Child is AND/OR with single Child => promote Grandchild to Child
            TQueryItem Grandchild = Child.ItemV[0];
            Child = Grandchild;
        }
    }
}

///////////////////////////////
// QMiner-Query-Aggregate
TQueryAggr::TQueryAggr(const TWPt<TBase>& Base,
    const TWPt<TStore>& _Store, const PJsonVal& AggrVal) {

    // get name
    QmAssertR(AggrVal->IsObjKey("name"), "Missing aggregate 'name'.");
    AggrNm = AggrVal->GetObjStr("name");
    // parse out the type, rest the aggregate can do alone
    QmAssertR(AggrVal->IsObjKey("type"), "Missing aggregate 'type'.");
    AggrType = AggrVal->GetObjStr("type");
    // assert name is fine
    Base->AssertValidNm(AggrNm);
    // remember the json parameters
    ParamVal = AggrVal;
}

TQueryAggr::TQueryAggr(TSIn& SIn) : AggrNm(SIn), AggrType(SIn) {
    ParamVal = TJsonVal::GetValFromStr(TStr(SIn));
}

void TQueryAggr::Save(TSOut& SOut) const {
    AggrNm.Save(SOut); AggrType.Save(SOut);
    TJsonVal::GetStrFromVal(ParamVal).Save(SOut);
}

void TQueryAggr::LoadJson(const TWPt<TBase>& Base, const TWPt<TStore>& Store,
    const PJsonVal& AggrVal, TQueryAggrV& QueryAggrV) {

    if (AggrVal->IsObj()) {
        // only one aggregate
        QueryAggrV.Add(TQueryAggr(Base, Store, AggrVal));
    } else if (AggrVal->IsArr()) {
        // several aggregates in an array
        for (int AggrN = 0; AggrN < AggrVal->GetArrVals(); AggrN++) {
            QueryAggrV.Add(TQueryAggr(Base, Store, AggrVal->GetArrVal(AggrN)));
        }
    } else {
        throw TQmExcept::New("Unsupported aggregate definition: " + TJsonVal::GetStrFromVal(AggrVal));
    }
}

///////////////////////////////
// QMiner-Query
TQuery::TQuery(const TWPt<TBase>& Base, const TQueryItem& _QueryItem,
    const int& _SortFieldId, const bool& _SortAscP, const int& _Limit,
    const int& _Offset) : QueryItem(_QueryItem), SortFieldId(_SortFieldId),
    SortAscP(_SortAscP), Limit(_Limit), Offset(_Offset) {}

PQuery TQuery::New(const TWPt<TBase>& Base, const TQueryItem& QueryItem,
    const int& SortFieldId, const bool& SortAscP, const int& Limit, const int& Offset) {

    return new TQuery(Base, QueryItem, SortFieldId, SortAscP, Limit, Offset);
}

PQuery TQuery::New(const TWPt<TBase>& Base, const PJsonVal& JsonVal) {
    PQuery Query = New(Base, TQueryItem(Base, JsonVal));
    // check if there are any aggregates
    if (JsonVal->IsObjKey("$aggr")) {
        PJsonVal AggrVal = JsonVal->GetObjKey("$aggr");
        TQueryAggr::LoadJson(Base, Query->GetStore(Base), AggrVal, Query->QueryAggrV);
    }
    // check if we have any sorting
    if (JsonVal->IsObjKey("$sort")) {
        PJsonVal SortVal = JsonVal->GetObjKey("$sort");
        QmAssert(SortVal->IsObj() && SortVal->GetObjKeys() == 1);
        // parse field id
        TStr FieldNm; PJsonVal AscVal; SortVal->GetObjKeyVal(0, FieldNm, AscVal);
        TWPt<TStore> Store = Query->GetStore(Base);
        QmAssert(Store->IsFieldNm(FieldNm));
        Query->SortFieldId = Store->GetFieldId(FieldNm);
        // parse sort direction
        QmAssert(AscVal->IsNum());
        Query->SortAscP = (AscVal->GetNum() > 0.0);
    }
    // check if ther is any limit
    if (JsonVal->IsObjKey("$limit")) {
        Query->Limit = TFlt::Round(JsonVal->GetObjNum("$limit"));
    }
    // check if ther is any offset
    if (JsonVal->IsObjKey("$offset")) {
        Query->Offset = TFlt::Round(JsonVal->GetObjNum("$offset"));
    }
    Query->Optimize();
    return Query;
}

void TQuery::Optimize() {
    QueryItem.Optimize();
}

PQuery TQuery::New(const TWPt<TBase>& Base, const TStr& QueryStr) {
    PJsonVal JsonVal = TJsonVal::GetValFromStr(QueryStr);
    QmAssertR(JsonVal->IsDef(), "Invalid query JSON: '" + QueryStr + "'");
    return New(Base, JsonVal);
}

TWPt<TStore> TQuery::GetStore(const TWPt<TBase>& Base) {
    const uint StoreId = QueryItem.GetStoreId(Base);
    return Base->GetStoreByStoreId(StoreId);
}

void TQuery::Sort(const TWPt<TBase>& Base, const PRecSet& RecSet) {
    RecSet->SortByField(SortAscP, SortFieldId);
}

PRecSet TQuery::GetLimit(const PRecSet& RecSet) {
    return RecSet->GetLimit(Limit, Offset);
}

bool TQuery::IsOk(const TWPt<TBase>& Base, TStr& MsgStr) const {
    try {
        QueryItem.GetStoreId(Base);
        MsgStr.Clr(); return true;
    } catch (PExcept Except) {
        MsgStr = Except->GetMsgStr();
        return false;
    }
}

///////////////////////////////
// GeoIndex
TIntPr TGeoIndex::GetLocId(const TFltPr& Loc) const {
    // round location coordinages to a meter precision
    return TIntPr(TFlt::Round(Loc.Val1 * Precision), TFlt::Round(Loc.Val2 * Precision));
}

void TGeoIndex::LocKeyIdToRecId(const TIntV& LocKeyIdV, const int& Limit, TUInt64V& AllRecIdV) const {
    AllRecIdV.Clr();
    for (int LocKeyIdN = 0; LocKeyIdN < LocKeyIdV.Len(); LocKeyIdN++) {
        const int LocKeyId = LocKeyIdV[LocKeyIdN];
        const TUInt64V& RecIdV = LocRecIdH[LocKeyId];
        AllRecIdV.AddV(RecIdV);
        // stop when we have enough records
        if (AllRecIdV.Len() >= Limit) { break; }
    }
    AllRecIdV.Sort();
}

int TGeoIndex::AllRecs() const {
    int Recs = 0;
    int KeyId = LocRecIdH.FFirstKeyId();
    while (LocRecIdH.FNextKeyId(KeyId)) {
        Recs += LocRecIdH[KeyId].Len();
    }
    return Recs;
}

void TGeoIndex::AddKey(const TFltPr& Loc, const uint64& RecId) {
    //const int RecsStart = AllRecs();
    TIntPr LocId = GetLocId(Loc);
    // check if new location
    if (!LocRecIdH.IsKey(LocId)) {
        const int LocKeyId = LocRecIdH.AddKey(LocId);
        SphereNn.AddKey(LocKeyId, Loc.Val1, Loc.Val2);
    }
    // remember record
    LocRecIdH.GetDat(LocId).Add(RecId);
}

void TGeoIndex::DelKey(const TFltPr& Loc, const uint64& RecId) {
    //const int RecsStart = AllRecs();
    TIntPr LocId = GetLocId(Loc);
    // check if known location
    if (LocRecIdH.IsKey(LocId)) {
        const int LocKeyId = LocRecIdH.GetKeyId(LocId);
        // delete from location to record map
        TUInt64V& RecIdV = LocRecIdH[LocKeyId];
        RecIdV.DelIfIn(RecId);
        // forget about location completely, if nothing left there
        if (LocRecIdH[LocKeyId].Empty()) {
            // delete from shpere
            SphereNn.DelKey(LocKeyId);
            // delete fron sphere->rec map
            LocRecIdH.DelKeyId(LocKeyId);
        }
    }
}

void TGeoIndex::SearchRange(const TFltPr& Loc, const double& Radius,
    const int& Limit, TUInt64V& RecIdV) const {

    TIntV LocKeyIdV; SphereNn.NnQuery(Loc.Val1, Loc.Val2, Limit, Radius, LocKeyIdV);
    LocKeyIdToRecId(LocKeyIdV, Limit, RecIdV);
}

void TGeoIndex::SearchNn(const TFltPr& Loc, const int& Limit, TUInt64V& RecIdV) const {
    TIntV LocKeyIdV; SphereNn.NnQuery(Loc.Val1, Loc.Val2, Limit, LocKeyIdV);
    LocKeyIdToRecId(LocKeyIdV, Limit, RecIdV);
}

bool TGeoIndex::LocEquals(const TFltPr& Loc1, const TFltPr& Loc2) const {
    TIntPr LocId1 = GetLocId(Loc1), LocId2 = GetLocId(Loc2);
    return (LocId1 == LocId2);
}

///////////////////////////////
// QMiner-Index
TIndex::TQmGixKeyStr::TQmGixKeyStr(const TWPt<TBase>& _Base,
    const TWPt<TIndexVoc>& _IndexVoc) : Base(_Base), IndexVoc(_IndexVoc) {}

TStr TIndex::TQmGixKeyStr::GetKeyNm(const TQmGixKey& Key) const {
    // get ids
    const int KeyId = Key.Val1;
    const uint StoreId = IndexVoc->GetKeyStoreId(KeyId);
    const uint64 WordId = Key.Val2;
    // generate pretty name
    const TIndexKey& IndexKey = IndexVoc->GetKey(KeyId);
    const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
    TChA KeyChA = Store->GetStoreNm();
    if (IndexKey.IsInternal()) {
        KeyChA += "->"; KeyChA += IndexKey.GetJoinNm();
        KeyChA += "->"; KeyChA += Store->GetRecNm(WordId);
    } else {
        KeyChA += '.'; KeyChA += IndexKey.GetKeyNm();
        if (IndexKey.IsWordVoc()) {
            if (IndexVoc->IsWordId(KeyId, WordId)) {
                KeyChA += '['; KeyChA += IndexVoc->GetWordStr(KeyId, WordId); KeyChA += ']';
            } else {
                //printf("x");
            }
        }
    }
    return KeyChA;
}

int TIndex::TQmGixItemPos::MaxPos = 8;

TIndex::TQmGixItemPos::TQmGixItemPos(TSIn& SIn): RecId(SIn) {
    for (int PosN = 0; PosN < MaxPos; PosN++) {
        PosV[PosN] = TUCh(SIn);
    }
}

void TIndex::TQmGixItemPos::Save(TSOut& SOut) const {
    RecId.Save(SOut);
    // we always save all positions
    for (int PosN = 0; PosN < MaxPos; PosN++) {
        PosV[PosN].Save(SOut);
    }
}

void TIndex::TQmGixItemPos::Add(const int& Pos) {
    // make sure we still have palce to store
    Assert(IsSpace());
    // find place where to store it
    int EmptyPosN = 0;
    while (PosV[EmptyPosN] != 0) { EmptyPosN++; }
    // store position
    PosV[EmptyPosN] = (uchar)(Pos % 0xFF + 1);
}

bool TIndex::DoQueryFull(const TPt<TQmGixExpItemFull>& ExpItem, TVec<TQmGixItemFull>& RecIdFqV) const {
    // clean if there is anything on the input
    RecIdFqV.Clr();
    // execute query
    const bool Not = ExpItem->Eval(GixFull, RecIdFqV, SumMergerFull);
    // make sure we are sorted
    Assert(RecIdFqV.IsSorted());
    // pass forward return result
    return Not;
}

bool TIndex::DoQuerySmall(const TPt<TQmGixExpItemSmall>& ExpItem, TVec<TQmGixItemFull>& RecIdFqV) const {
    // execute query
    TVec<TQmGixItemSmall> SmallRecIdFqV;
    const bool Not = ExpItem->Eval(GixSmall, SmallRecIdFqV, SumMergerSmall);
    // upgrade to full
    RecIdFqV.Gen(SmallRecIdFqV.Len(), 0);
    for (const TQmGixItemSmall& SmallRecIdFq : SmallRecIdFqV) {
        RecIdFqV.Add(TQmGixItemFull((uint64)SmallRecIdFq.Key, (int)SmallRecIdFq.Dat));
    }
    // pass forward return result
    return Not;
}

bool TIndex::DoQueryTiny(const TPt<TQmGixExpItemTiny>& ExpItem, TVec<TQmGixItemFull>& RecIdFqV) const {
    // execute query
    TVec<TQmGixItemTiny> TinyRecIdV;
    const bool Not = ExpItem->Eval(GixTiny, TinyRecIdV, MergerTiny);
    // upgrade to full
    RecIdFqV.Gen(TinyRecIdV.Len(), 0);
    for (const TQmGixItemTiny& TinyRecId : TinyRecIdV) {
        RecIdFqV.Add(TQmGixItemFull((uint64)TinyRecId, 1));
    }
    // pass forward return result
    return Not;
}

TIndex::TIndex(const TStr& _IndexFPath, const TFAccess& _Access, const PIndexVoc& _IndexVoc,
    const int64& CacheSizeFull, const int64& CacheSizeSmall, const uint64& CacheSizeTiny,
    const int& SplitLen) {

    IndexFPath = _IndexFPath;
    Access = _Access;
    // initialize full invered index
    SumMergerFull = new TQmGixSumMerger<TQmGixItemFull>;
    GixFull = TGix<TQmGixKey, TQmGixItemFull>::New("Index.GixFull",
        IndexFPath, Access, SumMergerFull, CacheSizeFull, SplitLen);
    // initialize small inverted index
    SumMergerSmall = new TQmGixSumMerger<TQmGixItemSmall>;
    GixSmall = TGix<TQmGixKey, TQmGixItemSmall>::New("Index.GixSmall",
        IndexFPath, Access, SumMergerSmall, CacheSizeSmall, SplitLen);
    // initialize tiny inverted index
    MergerTiny = new TGixDefMerger<TQmGixKey, TQmGixItemTiny>;
    GixTiny = TGix<TQmGixKey, TQmGixItemTiny>::New("Index.GixTiny",
        IndexFPath, Access, MergerTiny, CacheSizeTiny, SplitLen);
    // initialize location index
    TStr SphereFNm = IndexFPath + "Index.Geo";
    if (TFile::Exists(SphereFNm) && Access != faCreate) {
        TFIn SphereFIn(SphereFNm);
        GeoIndexH.Load(SphereFIn);
    }
    // initialize btree index
    TStr BTreeFNm = IndexFPath + "Index.BTree";
    if (TFile::Exists(BTreeFNm) && Access != faCreate) {
        TFIn BTreeFIn(BTreeFNm);
        BTreeIndexByteH.Load(BTreeFIn);
        BTreeIndexIntH.Load(BTreeFIn);
        BTreeIndexInt16H.Load(BTreeFIn);
        BTreeIndexInt64H.Load(BTreeFIn);
        BTreeIndexUIntH.Load(BTreeFIn);
        BTreeIndexUInt16H.Load(BTreeFIn);
        BTreeIndexUInt64H.Load(BTreeFIn);
        BTreeIndexFltH.Load(BTreeFIn);
        BTreeIndexSFltH.Load(BTreeFIn);
    }
    // initialize vocabularies
    IndexVoc = _IndexVoc;
}

PIndex TIndex::New(const TStr& IndexFPath, const TFAccess& Access, const PIndexVoc& IndexVoc,
    const int64& CacheSizeFull, const int64& CacheSizeSmall, const uint64& CacheSizeTiny,
    const int& SplitLen) {

    return new TIndex(IndexFPath, Access, IndexVoc,
         CacheSizeFull, CacheSizeSmall, CacheSizeTiny, SplitLen);
}

TIndex::~TIndex() {
    if (!IsReadOnly()) {
        TEnv::Logger->OnStatus("Saving and closing inverted index - full");
        GixFull.Clr();
        delete SumMergerFull;
        TEnv::Logger->OnStatus("Saving and closing inverted index - small");
        GixSmall.Clr();
        delete SumMergerSmall;
        TEnv::Logger->OnStatus("Saving and closing inverted index - tiny");
        GixTiny.Clr();
        delete MergerTiny;
        {
            TEnv::Logger->OnStatus("Saving and closing location index");
            TFOut SphereFOut(IndexFPath + "Index.Geo");
            GeoIndexH.Save(SphereFOut);
        }
        {
            TEnv::Logger->OnStatus("Saving and closing btree index");
            TFOut BTreeFOut(IndexFPath + "Index.BTree");
            BTreeIndexByteH.Save(BTreeFOut);
            BTreeIndexIntH.Save(BTreeFOut);
            BTreeIndexInt16H.Save(BTreeFOut);
            BTreeIndexInt64H.Save(BTreeFOut);
            BTreeIndexUIntH.Save(BTreeFOut);
            BTreeIndexUInt16H.Save(BTreeFOut);
            BTreeIndexUInt64H.Save(BTreeFOut);
            BTreeIndexFltH.Save(BTreeFOut);
            BTreeIndexSFltH.Save(BTreeFOut);
        }
        TEnv::Logger->OnStatus("Index closed");
    } else {
        TEnv::Logger->OnStatus("Index opened in read-only mode, no saving needed");
    }
}

void TIndex::IndexValue(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
    const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
    IndexGix(KeyId, WordId, RecId, 1);
}

void TIndex::IndexValue(const int& KeyId, const TStrV& WordStrV, const uint64& RecId) {
    // load word-counts
    TUInt64H WordIdH;
    for (int WordN = 0; WordN < WordStrV.Len(); WordN++) {
        const TStr WordStr = WordStrV[WordN]; //.GetLc();
        WordIdH.AddDat(IndexVoc->AddWordStr(KeyId, WordStr))++;
    }
    // index words
    int WordKeyId = WordIdH.FFirstKeyId();
    while (WordIdH.FNextKeyId(WordKeyId)) {
        const uint64 WordId = WordIdH.GetKey(WordKeyId);
        const int WordFq = WordIdH[WordKeyId];
        IndexGix(KeyId, WordId, RecId, WordFq);
    }
}

void TIndex::IndexText(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
    // tokenize string
    TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
    // aggregate by word
    TUInt64H WordIdFqH;
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        WordIdFqH.AddDat(WordIdV[WordIdN])++;
    }
    // index words
    int WordKeyId = WordIdFqH.FFirstKeyId();
    while (WordIdFqH.FNextKeyId(WordKeyId)) {
        IndexGix(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
    }
}

void TIndex::IndexJoin(const TWPt<TStore>& Store, const int& JoinId,
    const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

    IndexGix(Store->GetJoinKeyId(JoinId), RecId, JoinRecId, JoinFq);
}

void TIndex::IndexGix(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq) {
    // -1 should never come to here
    Assert(KeyId != -1);
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // send to appropriate index
    switch (GixType) {
    case oikgtFull:
        GixFull->AddItem(TKeyWord(KeyId, WordId), TQmGixItemFull(RecId, RecFq)); break;
    case oikgtSmall:
        GixSmall->AddItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, (int16)RecFq)); break;
    case oikgtTiny:
        GixTiny->AddItem(TKeyWord(KeyId, WordId), TQmGixItemTiny((uint)RecId)); break;
    default:
        throw TQmExcept::New("[TIndex::Index] Unsupported gix type!");
    }
}

void TIndex::DeleteValue(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
    const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
    DeleteGix(KeyId, WordId, RecId, 1);
}

void TIndex::DeleteValue(const int& KeyId, const TStrV& WordStrV, const uint64& RecId) {
    // load word-counts
    TUInt64H WordIdH;
    for (int WordN = 0; WordN < WordStrV.Len(); WordN++) {
        const TStr WordStr = WordStrV[WordN]; //.GetLc();
        WordIdH.AddDat(IndexVoc->AddWordStr(KeyId, WordStr))++;
    }
    // delete words from index
    int WordKeyId = WordIdH.FFirstKeyId();
    while (WordIdH.FNextKeyId(WordKeyId)) {
        const uint64 WordId = WordIdH.GetKey(WordKeyId);
        const int WordFq = WordIdH[WordKeyId];
        DeleteGix(KeyId, WordId, RecId, WordFq);
    }
}

void TIndex::DeleteText(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
    // tokenize string
    TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
    // aggregate by word
    TUInt64H WordIdFqH;
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        WordIdFqH.AddDat(WordIdV[WordIdN])++;
    }
    // index words
    int WordKeyId = WordIdFqH.FFirstKeyId();
    while (WordIdFqH.FNextKeyId(WordKeyId)) {
        DeleteGix(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
    }
}

void TIndex::DeleteJoin(const TWPt<TStore>& Store, const int& JoinId,
    const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

    DeleteGix(Store->GetJoinKeyId(JoinId), RecId, JoinRecId, JoinFq);
}

void TIndex::DeleteGix(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq) {
    // -1 should never come to here
    Assert(KeyId != -1);
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // are we deleting all items or just few occurences?
    if (RecFq == TInt::Mx) {
        // full delete from index
        switch (GixType) {
        case oikgtFull:
            GixFull->DelItem(TKeyWord(KeyId, WordId), TQmGixItemFull(RecId, 0)); break;
        case oikgtSmall:
            GixSmall->DelItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, 0)); break;
        case oikgtTiny:
            GixTiny->DelItem(TKeyWord(KeyId, WordId), TQmGixItemTiny(RecId)); break;
        default: throw TQmExcept::New("[TIndex::Delete] Unsupported gix type!");
        }
    } else {
        switch (GixType) {
        case oikgtFull:
            GixFull->AddItem(TKeyWord(KeyId, WordId), TQmGixItemFull(RecId, -RecFq)); break;
        case oikgtSmall:
            GixSmall->AddItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, (int16)-RecFq)); break;
        case oikgtTiny:
            GixTiny->DelItem(TKeyWord(KeyId, WordId), TQmGixItemTiny(RecId)); break;
        default: throw TQmExcept::New("[TIndex::Delete] Unsupported gix type!");
        }
    }
}

void TIndex::IndexTextPos(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
    // tokenize string
    TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
    // index tokens
    IndexTextPos(KeyId, WordIdV, RecId);
}

void TIndex::IndexTextPos(const int& KeyId, const TUInt64V& WordIdV, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // aggregate by word
    THash<TUInt64, TQmGixItemPos> WordIdPosH;
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        const int WordId = WordIdV[WordIdN];
        // check if first time we see the word
        if (!WordIdPosH.IsKey(WordId)) {
            WordIdPosH.AddDat(WordId, TQmGixItemPos(RecId));
        }
        // remember the position in case there is space left
        TQmGixItemPos& ItemPos = WordIdPosH.GetDat(WordId);
        if (ItemPos.IsSpace()) { ItemPos.Add(WordIdN); }
    }
    // add to index
    for (auto& WordIdPos : WordIdPosH) {
        // get word parameters
        const TUInt64& WordId = WordIdPos.Key;
        const TQmGixItemPos ItemPos = WordIdPos.Dat;
        // add to gix
        GixPos->AddItem(TKeyWord(KeyId, WordId), ItemPos);
    }
}


void TIndex::DeleteTextPos(const int& KeyId, const TStr& TextStr, const uint64& RecId) {
    // tokenize string
    TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStr, WordIdV);
    // index tokens
    DeleteTextPos(KeyId, WordIdV, RecId);
}

void TIndex::DeleteTextPos(const int& KeyId, const TUInt64V& WordIdV, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // create list of all word ids for which we should remove given record from index
    THashSet<TUInt64> WordIdSet;
    for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
        const int WordId = WordIdV[WordIdN];
        WordIdSet.AddKey(WordId);
    }
    // remove from index
    int WordKeyId = WordIdSet.FFirstKeyId();
    while (WordIdSet.FNextKeyId(WordKeyId)) {
        const uint64 WordId = WordIdSet.GetKey(WordKeyId);
        GixPos->DelItem(TKeyWord(KeyId, WordId), TQmGixItemPos(RecId));
    }
}

void TIndex::IndexGeo(const int& KeyId, const TFltPr& Loc, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!GeoIndexH.IsKey(KeyId)) { GeoIndexH.AddDat(KeyId, TGeoIndex::New()); }
    // index new location
    GeoIndexH.GetDat(KeyId)->AddKey(Loc, RecId);
}

void TIndex::DeleteGeo(const int& KeyId, const TFltPr& Loc, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->DelKey(Loc, RecId); }
}

bool TIndex::LocEquals(const int& KeyId, const TFltPr& Loc1, const TFltPr& Loc2) const {
    return GeoIndexH.IsKey(KeyId) ? GeoIndexH.GetDat(KeyId)->LocEquals(Loc1, Loc2) : false;
}

void TIndex::IndexLinear(const int& KeyId, const uchar& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexByteH.IsKey(KeyId)) { BTreeIndexByteH.AddDat(KeyId, PBTreeIndexUCh::New()); }
    // index new location
    BTreeIndexByteH.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const int& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexIntH.IsKey(KeyId)) { BTreeIndexIntH.AddDat(KeyId, PBTreeIndexInt::New()); }
    // index new location
    BTreeIndexIntH.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const int16& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexInt16H.IsKey(KeyId)) { BTreeIndexInt16H.AddDat(KeyId, PBTreeIndexInt16::New()); }
    // index new location
    BTreeIndexInt16H.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const int64& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexInt64H.IsKey(KeyId)) { BTreeIndexInt64H.AddDat(KeyId, PBTreeIndexInt64::New()); }
    // index new location
    BTreeIndexInt64H.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const uint& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexUIntH.IsKey(KeyId)) { BTreeIndexUIntH.AddDat(KeyId, PBTreeIndexUInt::New()); }
    // index new location
    BTreeIndexUIntH.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const uint16& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexUInt16H.IsKey(KeyId)) { BTreeIndexUInt16H.AddDat(KeyId, PBTreeIndexUInt16::New()); }
    // index new location
    BTreeIndexUInt16H.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const uint64& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexUInt64H.IsKey(KeyId)) { BTreeIndexUInt64H.AddDat(KeyId, PBTreeIndexUInt64::New()); }
    // index new location
    BTreeIndexUInt64H.GetDat(KeyId)->AddKey(Val, RecId);
}


void TIndex::IndexLinear(const int& KeyId, const double& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexFltH.IsKey(KeyId)) { BTreeIndexFltH.AddDat(KeyId, PBTreeIndexFlt::New()); }
    // index new location
    BTreeIndexFltH.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::IndexLinear(const int& KeyId, const float& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // if new key, create sphere first
    if (!BTreeIndexSFltH.IsKey(KeyId)) { BTreeIndexSFltH.AddDat(KeyId, PBTreeIndexSFlt::New()); }
    // index new location
    BTreeIndexSFltH.GetDat(KeyId)->AddKey(Val, RecId);
}

void TIndex::DeleteLinear(const int& KeyId, const uchar& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexByteH.IsKey(KeyId)) { BTreeIndexByteH.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const int& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexIntH.IsKey(KeyId)) { BTreeIndexIntH.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const int16& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexInt16H.IsKey(KeyId)) { BTreeIndexInt16H.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const int64& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexInt64H.IsKey(KeyId)) { BTreeIndexInt64H.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const uint& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexUIntH.IsKey(KeyId)) { BTreeIndexUIntH.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const uint16& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexUInt16H.IsKey(KeyId)) { BTreeIndexUInt16H.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const uint64& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexUInt64H.IsKey(KeyId)) { BTreeIndexUInt64H.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const double& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexFltH.IsKey(KeyId)) { BTreeIndexFltH.GetDat(KeyId)->DelKey(Val, RecId); }
}

void TIndex::DeleteLinear(const int& KeyId, const float& Val, const uint64& RecId) {
    // we shouldn't modify read-only index
    QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
    // delete only if index exist
    if (BTreeIndexSFltH.IsKey(KeyId)) { BTreeIndexSFltH.GetDat(KeyId)->DelKey(Val, RecId); }
}

PRecSet TIndex::SearchGix(const TWPt<TBase>& Base, const int& KeyId, const uint64& WordId) const {
    // prepare Gix keys
    TKeyWord KeyWord(KeyId, WordId);
    // prepare placeholder for results
    TVec<TQmGixItemFull> RecIdFqV;
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // go to appropriate gix and always first check if we have the key at all
    switch (GixType) {
    case oikgtFull:
        DoQueryFull(TQmGixExpItemFull::NewItem(KeyWord), RecIdFqV); break;
    case oikgtSmall:
        DoQuerySmall(TQmGixExpItemSmall::NewItem(KeyWord), RecIdFqV); break;
    case oikgtTiny:
        DoQueryTiny(TQmGixExpItemTiny::NewItem(KeyWord), RecIdFqV); break;
    default:
        throw TQmExcept::New("[TIndex::SearchGixOr] Unsupported gix type!");
    }
    // create result set and return it
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdFqV);
}

PRecSet TIndex::SearchGixAnd(const TWPt<TBase>& Base, const int& KeyId, const TUInt64V& WordIdV) const {
    // prepare Gix keys
    TKeyWordV KeyWordV(WordIdV.Len(), 0);
    for (const uint64 WordId : WordIdV) {
        KeyWordV.Add(TKeyWord(KeyId, WordId));
    }
    // prepare placeholder for results
    TVec<TQmGixItemFull> RecIdFqV;
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // go to appropriate gix and always first check if we have the key at all
    switch (GixType) {
    case oikgtFull:
        DoQueryFull(TQmGixExpItemFull::NewAndV(KeyWordV), RecIdFqV); break;
    case oikgtSmall:
        DoQuerySmall(TQmGixExpItemSmall::NewAndV(KeyWordV), RecIdFqV); break;
    case oikgtTiny:
        DoQueryTiny(TQmGixExpItemTiny::NewAndV(KeyWordV), RecIdFqV); break;
    default:
        throw TQmExcept::New("[TIndex::SearchGixAnd] Unsupported gix type!");
    }
    // create result set and return it
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdFqV);
}

PRecSet TIndex::SearchGixOr(const TWPt<TBase>& Base, const int& KeyId, const TUInt64V& WordIdV) const {
    // prepare Gix keys
    TKeyWordV KeyWordV(WordIdV.Len(), 0);
    for (const uint64 WordId : WordIdV) {
        KeyWordV.Add(TKeyWord(KeyId, WordId));
    }
    // prepare placeholder for results
    TVec<TQmGixItemFull> RecIdFqV;
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // go to appropriate gix and always first check if we have the key at all
    switch (GixType) {
    case oikgtFull:
        DoQueryFull(TQmGixExpItemFull::NewOrV(KeyWordV), RecIdFqV); break;
    case oikgtSmall:
        DoQuerySmall(TQmGixExpItemSmall::NewOrV(KeyWordV), RecIdFqV); break;
    case oikgtTiny:
        DoQueryTiny(TQmGixExpItemTiny::NewOrV(KeyWordV), RecIdFqV); break;
    default:
        throw TQmExcept::New("[TIndex::SearchGixOr] Unsupported gix type!");
    }
    // create result set and return it
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdFqV);
}

void TIndex::SearchGixJoin(const int& KeyId, const uint64& RecId, TUInt64IntKdV& JoinRecIdFqV) const {
    // prepare key for gix
    TKeyWord KeyWord(KeyId, RecId);
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // go to appropriate gix and always first check if we have the key at all
    switch (GixType) {
    case oikgtFull:
        DoQueryFull(TQmGixExpItemFull::NewItem(KeyWord), JoinRecIdFqV);
        break;
    case oikgtSmall:
        DoQuerySmall(TQmGixExpItemSmall::NewItem(KeyWord), JoinRecIdFqV);
        break;
    case oikgtTiny:
        DoQueryTiny(TQmGixExpItemTiny::NewItem(KeyWord), JoinRecIdFqV);
        break;
    default:
        throw TQmExcept::New("[TIndex::SearchGixJoin] Unsupported gix type!");
    }
}

void TIndex::SearchGixJoin(const int& KeyId, const TUInt64V& RecIdV, TUInt64IntKdV& JoinRecIdFqV) const {
    // prepare keys for gix
    TKeyWordV KeyWordV(RecIdV.Len(), 0);
    for (const uint64 RecId : RecIdV) {
        KeyWordV.Add(TKeyWord(KeyId, RecId));
    }
    // check which Gix to use
    const TIndexKeyGixType GixType = GetGixType(KeyId);
    // go to appropriate gix and always first check if we have the key at all
    switch (GixType) {
    case oikgtFull:
        DoQueryFull(TQmGixExpItemFull::NewOrV(KeyWordV), JoinRecIdFqV); break;
    case oikgtSmall:
        DoQuerySmall(TQmGixExpItemSmall::NewOrV(KeyWordV), JoinRecIdFqV); break;
    case oikgtTiny:
        DoQueryTiny(TQmGixExpItemTiny::NewOrV(KeyWordV), JoinRecIdFqV); break;
    default:
        throw TQmExcept::New("[TIndex::SearchGixJoin] Unsupported gix type!");
    }
}

PRecSet TIndex::SearchGeoRange(const TWPt<TBase>& Base, const int& KeyId,
        const TFltPr& Loc, const double& Radius, const int& Limit) const {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->SearchRange(Loc, Radius, Limit, RecIdV); }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchGeoNn(const TWPt<TBase>& Base, const int& KeyId,
        const TFltPr& Loc, const int& Limit) const {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->SearchNn(Loc, Limit, RecIdV); }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TIntPr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexIntH.IsKey(KeyId)) {
        BTreeIndexIntH.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TInt16Pr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexInt16H.IsKey(KeyId)) {
        BTreeIndexInt16H.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TInt64Pr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexInt64H.IsKey(KeyId)) {
        BTreeIndexInt64H.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUChPr& RangeMinMax) {
    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexByteH.IsKey(KeyId)) {
        BTreeIndexByteH.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUIntUIntPr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexUIntH.IsKey(KeyId)) {
        BTreeIndexUIntH.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUInt16Pr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexUInt16H.IsKey(KeyId)) {
        BTreeIndexUInt16H.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TUInt64Pr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexUInt64H.IsKey(KeyId)) {
        BTreeIndexUInt64H.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TFltPr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexFltH.IsKey(KeyId)) {
        BTreeIndexFltH.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchLinear(const TWPt<TBase>& Base, const int& KeyId, const TSFltPr& RangeMinMax) {

    TUInt64V RecIdV;
    const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
    if (BTreeIndexSFltH.IsKey(KeyId)) {
        BTreeIndexSFltH.GetDat(KeyId)->SearchRange(RangeMinMax, RecIdV);
        RecIdV.Sort();
    }
    return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

void TIndex::SaveTxt(const TWPt<TBase>& Base, const TStr& FNm) {
    GixFull->SaveTxt(FNm + ".full", TQmGixKeyStr::New(Base, IndexVoc));
    GixSmall->SaveTxt(FNm + ".small", TQmGixKeyStr::New(Base, IndexVoc));
    GixTiny->SaveTxt(FNm + ".tiny", TQmGixKeyStr::New(Base, IndexVoc));
}

TBlobBsStats TIndex::GetBlobStats() const {
    TBlobBsStats Stats = GixFull->GetBlobStats();
    Stats.Add(GixSmall->GetBlobStats());
    Stats.Add(GixTiny->GetBlobStats());
    return Stats;
}

TGixStats TIndex::GetGixStats(const bool& RefreshP) const {
    TGixStats Stats = GixFull->GetGixStats(RefreshP);
    Stats.Add(GixSmall->GetGixStats(RefreshP));
    Stats.Add(GixTiny->GetGixStats(RefreshP));
    return Stats;
}

int TIndex::GetSplitLen() const {
    // make sure small and full have same settings
    EAssert(GixFull->GetSplitLen() == GixSmall->GetSplitLen());
    EAssert(GixFull->GetSplitLen() == GixTiny->GetSplitLen());
    // return
    return GixFull->GetSplitLen();
}

void TIndex::ResetStats() {
    GixFull->ResetStats();
    GixSmall->ResetStats();
    GixTiny->ResetStats();
}

int TIndex::PartialFlush(const int& WndInMsec) {
    const int WndInMsecPerGix = WndInMsec / 3;
    int Res = 0;
    Res += GixFull->PartialFlush(WndInMsecPerGix);
    Res += GixSmall->PartialFlush(WndInMsecPerGix);
    Res += GixTiny->PartialFlush(WndInMsecPerGix);
    return Res;
}

///////////////////////////////
// QMiner-Aggregator
TFunRouter<PAggr, TAggr::TNewF> TAggr::NewRouter;

void TAggr::Init() {
    Register<TAggrs::TCount>();
    Register<TAggrs::THistogram>();
    Register<TAggrs::TKeywords>();
    Register<TAggrs::TTimeLine>();
    Register<TAggrs::TTimeSpan>();
#ifdef OG_AGGR_DOC_ATLAS
    Register<TAggrs::TDocAtlas>();
#endif
}

TAggr::TAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm) : Base(_Base), AggrNm(_AggrNm) {}

PAggr TAggr::New(const TWPt<TBase>& Base, const PRecSet& RecSet, const TQueryAggr& QueryAggr) {
    return NewRouter.Fun(QueryAggr.GetType())(Base, QueryAggr.GetNm(), RecSet, QueryAggr.GetParamVal());
}

///////////////////////////////
// QMiner-Stream-Aggregator
TFunRouter<PStreamAggr, TStreamAggr::TNewF> TStreamAggr::NewRouter;

void TStreamAggr::Init() {
    Register<TStreamAggrSet>();
    Register<TStreamAggrs::TRecBuffer>();
    Register<TStreamAggrs::TTimeSeriesTick>();
    Register<TStreamAggrs::TTimeSeriesSparseVectorTick>();
    Register<TStreamAggrs::TWinBufFlt>();
    Register<TStreamAggrs::TWinBufFtrSpVec>();
    Register<TStreamAggrs::TWinBufFltV>();
    Register<TStreamAggrs::TWinBufSpV>();
    Register<TStreamAggrs::TWinBufSum>();
    Register<TStreamAggrs::TWinBufMin>();
    Register<TStreamAggrs::TWinBufMax>();
    Register<TStreamAggrs::TMa>();
    Register<TStreamAggrs::TEma>();
    Register<TStreamAggrs::TThresholdAggr>();
    Register<TStreamAggrs::TVar>();
    Register<TStreamAggrs::TCov>();
    Register<TStreamAggrs::TCorr>();
    Register<TStreamAggrs::TMerger>();
    Register<TStreamAggrs::TResampler>();
    Register<TStreamAggrs::TUniVarResampler>();
    Register<TStreamAggrs::TAggrResampler>();
    Register<TStreamAggrs::TFtrExtAggr>();
	Register<TStreamAggrs::TNNAnomalyAggr>();
    Register<TStreamAggrs::TOnlineHistogram>();
    Register<TStreamAggrs::TTDigest>();
    Register<TStreamAggrs::TChiSquare>();
    Register<TStreamAggrs::TOnlineSlottedHistogram>();
    Register<TStreamAggrs::TVecDiff>();
    Register<TStreamAggrs::TSimpleLinReg>();
    Register<TStreamAggrs::TRecFilterAggr>();
    Register<TStreamAggrs::TEmaSpVec>();
    Register<TStreamAggrs::TWinBufSpVecSum>();
    Register<TStreamAggrs::TRecSwitchAggr>();
    Register<TStreamAggrs::THistogramAD>();
}

TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm): Base(_Base), AggrNm(_AggrNm) {
    Base->AssertValidNm(AggrNm);
}

TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal):
        Base(_Base), AggrNm(ParamVal->GetObjStr("name", TGuid::GenSafeGuid())) {
    Base->AssertValidNm(AggrNm);
}

TWPt<TStreamAggr> TStreamAggr::ParseAggr(const PJsonVal& ParamVal, const TStr& AggrKeyNm) {
    // get aggregate name
    TStr InAggrNm = ParamVal->GetObjStr(AggrKeyNm);
    // make sure we know of such aggregate
    QmAssertR(Base->IsStreamAggr(InAggrNm), "[TStreamAggr] Stream aggregate does not exist: " + InAggrNm);
    // get it and return it
    return Base->GetStreamAggr(InAggrNm);
}

PStreamAggr TStreamAggr::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
    return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

void TStreamAggr::LoadState(TSIn& SIn) {
    throw TQmExcept::New("TStreamAggr::LoadState not implemented:" + GetAggrNm());
};

void TStreamAggr::SaveState(TSOut& SOut) const {
    throw TQmExcept::New("TStreamAggr::SaveState not implemented:" + GetAggrNm());
};

void TStreamAggr::LoadStateJson(const PJsonVal& State) {
    throw TQmExcept::New("TStreamAggr::LoadStateJson not implemented:" + GetAggrNm());
};

PJsonVal TStreamAggr::SaveStateJson() const {
    throw TQmExcept::New("TStreamAggr::SaveStateJson not implemented:" + GetAggrNm());
};

uint64 TStreamAggr::GetMemUsed() const {
    // sizeof(TStreamAggr) returns the size of this class including all its members and
    // alignment, but discards the size of any pointers that the members hold which
    // equals Member.GetMemUsed() - sizeof(TMemberClass) (calculated by GetExtraMemberSize)
    return sizeof(TStreamAggr) +
           TMemUtils::GetExtraMemberSize(CRef) +
           TMemUtils::GetExtraMemberSize(AggrNm) +
           TMemUtils::GetExtraMemberSize(ExeTm);
}

///////////////////////////////
// QMiner-Stream-Aggregator-Set
TStreamAggrSet::TStreamAggrSet(const TWPt<TBase>& _Base, const TStr& _AggrNm):
    TStreamAggr(_Base, _AggrNm) { }

TStreamAggrSet::TStreamAggrSet(const TWPt<TBase>& _Base, const PJsonVal& ParamVal):
        TStreamAggr(_Base, ParamVal) {

    // get list of arrays
    QmAssertR(ParamVal->IsObjKey("aggregates"), "[TStreamAggrSet] Expecting array of aggregates");
    PJsonVal AggrVals = ParamVal->GetObjKey("aggregates");
    QmAssertR(AggrVals->IsArr(), "[TStreamAggrSet] Key 'aggregates' expected to be array");
    // go over the array
    for (int AggrValN = 0; AggrValN < AggrVals->GetArrVals(); AggrValN++) {
        PJsonVal AggrVal = AggrVals->GetArrVal(AggrValN);
        // read aggregate name
        QmAssertR(AggrVal->IsStr(), "[TStreamAggrSet] 'aggregates' values expected to be strings");
        const TStr& SubAggrNm = AggrVal->GetStr();
        // make sure it exists
        QmAssertR(GetBase()->IsStreamAggr(SubAggrNm), "[TStreamAggrSet] Unknonw stream aggregate '" + SubAggrNm + "'");
        // get it and add it to the set
        AddStreamAggr(GetBase()->GetStreamAggr(SubAggrNm));
    }
}

PStreamAggr TStreamAggrSet::New(const TWPt<TBase>& Base) {
    return new TStreamAggrSet(Base, TGuid::GenSafeGuid());
}

PStreamAggr TStreamAggrSet::New(const TWPt<TBase>& Base, const TStr& AggrNm) {
    return new TStreamAggrSet(Base, AggrNm);
}

PStreamAggr TStreamAggrSet::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
    return new TStreamAggrSet(Base, ParamVal);
}

bool TStreamAggrSet::Empty() const {
    return StreamAggrV.Empty();
}

int TStreamAggrSet::Len() const {
    return StreamAggrV.Len();
}

void TStreamAggrSet::AddStreamAggr(const PStreamAggr& StreamAggr) {
    // make sure we already registered this aggregate
    QmAssertR(GetBase()->IsStreamAggr(StreamAggr->GetAggrNm()),
        "[TStreamAggrSet] Unregistered stream aggregate " + StreamAggr->GetAggrNm());
    StreamAggrV.Add(StreamAggr());
}

const TWPt<TStreamAggr>& TStreamAggrSet::GetStreamAggr(const int& StreamAggrN) const {
    return StreamAggrV[StreamAggrN];
}

TStrV TStreamAggrSet::GetStreamAggrNmV() const {
    TStrV StreamAggrNmV;
    for (const TWPt<TStreamAggr>& StreamAggr: StreamAggrV) {
        StreamAggrNmV.Add(StreamAggr->GetAggrNm());
    }
    return StreamAggrNmV;
}

void TStreamAggrSet::Reset() {
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->Reset();
    }
}

void TStreamAggrSet::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->OnStep(this);
    }
}

void TStreamAggrSet::OnTime(const uint64& TmMsec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->OnTime(TmMsec, this);
    }
}

void TStreamAggrSet::OnAddRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->OnAddRec(Rec, this);
    }
}

void TStreamAggrSet::OnUpdateRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->OnUpdateRec(Rec, this);
    }
}

void TStreamAggrSet::OnDeleteRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->OnDeleteRec(Rec, this);
    }
}

void TStreamAggrSet::PrintStat() const {
    for (TWPt<TStreamAggr>& StreamAggr : StreamAggrV) {
        StreamAggr->PrintStat();
    }
}

PJsonVal TStreamAggrSet::SaveJson(const int& Limit) const {
    TStrV StreamAggrNmV = TStreamAggrSet::GetStreamAggrNmV();
    PJsonVal ResVal = TJsonVal::NewArr(StreamAggrNmV);
    return ResVal;
}

///////////////////////////////
// QMiner-Stream-Aggregator-Trigger
TStreamAggrTrigger::TStreamAggrTrigger(const TWPt<TStreamAggr>& _StreamAggr):
    StreamAggr(_StreamAggr) {}

PStoreTrigger TStreamAggrTrigger::New(const TWPt<TStreamAggr>& StreamAggr) {
    return new TStreamAggrTrigger(StreamAggr);
}

void TStreamAggrTrigger::OnAdd(const TRec& Rec) {
    StreamAggr->OnAddRec(Rec, NULL);
}

void TStreamAggrTrigger::OnUpdate(const TRec& Rec) {
    StreamAggr->OnUpdateRec(Rec, NULL);
}

void TStreamAggrTrigger::OnDelete(const TRec& Rec) {
    StreamAggr->OnDeleteRec(Rec, NULL);
}

///////////////////////////////
// QMiner-Base
PRecSet TBase::Invert(const PRecSet& RecSet) {
    // prepare sorted list of all records from the store
    TUInt64IntKdV AllResIdV;
    const TWPt<TStore>& Store = RecSet->GetStore();
    PStoreIter Iter = Store->GetIter();
    while (Iter->Next()) {
        AllResIdV.Add(TUInt64IntKd(Iter->GetRecId(), 1));
    }
    if (!AllResIdV.IsSorted()) { AllResIdV.Sort(); }
    // remove retrieved items
    TUInt64IntKdV ResIdFqV;
    AllResIdV.Diff(RecSet->GetRecIdFqV(), ResIdFqV);
    // return new record set
    return TRecSet::New(Store, ResIdFqV, false);
}

TPair<TBool, PRecSet> TBase::_Search(const TQueryItem& QueryItem) {
    if (QueryItem.IsGix()) {
        // we have gix query, check what is the comparison operator
        if (QueryItem.IsEqual() || QueryItem.IsNotEqual()) {
            // ==, !=
            // both cases require same search on Gix
            PRecSet RecSet = Index->SearchGixAnd(this, QueryItem.GetKeyId(), QueryItem.GetWordIdV());
            // difference is that in NotEqual we negate the result
            const bool NotP = QueryItem.IsNotEqual();
            // return the pair
            return TPair<TBool, PRecSet>(NotP, RecSet);
        } else if (QueryItem.IsGreater() || QueryItem.IsLess() || QueryItem.IsWildChar()) {
            // >=, <=, ~
            // we already extended the query to all possible words, just execute
            PRecSet RecSet = Index->SearchGixOr(this, QueryItem.GetKeyId(), QueryItem.GetWordIdV());
            // return the pair without any negation
            return TPair<TBool, PRecSet>(false, RecSet);
        } else {
            // unknown operator
            throw TQmExcept::New("Index: Unknown query item operator");
        }
    } else if (QueryItem.IsGeo()) {
        if (QueryItem.IsLocRadius()) {
            // must be handled by geo index
            PRecSet RecSet = Index->SearchGeoRange(this, QueryItem.GetKeyId(),
                QueryItem.GetLoc(), QueryItem.GetLocRadius(), QueryItem.GetLocLimit());
            return TPair<TBool, PRecSet>(false, RecSet);
        } else {
            // must be handled by geo index
            PRecSet RecSet = Index->SearchGeoNn(this, QueryItem.GetKeyId(),
                QueryItem.GetLoc(), QueryItem.GetLocLimit());
            return TPair<TBool, PRecSet>(false, RecSet);
        }
    } else if (QueryItem.IsRangeInt()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeIntMinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeInt16()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeInt16MinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeInt64()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeInt64MinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeByte()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeByteMinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeUInt()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeUIntMinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeUInt16()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeUInt16MinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeUInt64()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeUInt64MinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeTm()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeUInt64MinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeFlt()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeFltMinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsRangeSFlt()) {
        // must be handled by BTree linear index
        PRecSet RecSet = Index->SearchLinear(this, QueryItem.GetKeyId(), QueryItem.GetRangeSFltMinMax());
        return TPair<TBool, PRecSet>(false, RecSet);
    } else if (QueryItem.IsJoin()) {
        // special case when it's record passed by value
        const TQueryItem& SubItem = QueryItem.GetItem(0);
        if (SubItem.IsRec() && SubItem.GetRec().IsByVal()) {
            // do the join
            PRecSet JoinRecSet = SubItem.GetRec().DoJoin(this, QueryItem.GetJoinId());
            // return joined record set
            return TPair<TBool, PRecSet>(false, JoinRecSet);
        } else {
            // do the subordinate queries
            TPair<TBool, PRecSet> NotRecSet = _Search(QueryItem.GetItem(0));
            // in case it's negated, we must invert it
            if (NotRecSet.Val1) { NotRecSet.Val2 = Invert(NotRecSet.Val2); }
            // do the join
            PRecSet JoinRecSet = NotRecSet.Val2->DoJoin(this, QueryItem.GetJoinId(), QueryItem.GetSampleSize());
            // return joined record set
            return TPair<TBool, PRecSet>(false, JoinRecSet);
        }
    } else if (QueryItem.IsRec()) {
        // make sure record past by reference
        QmAssert(QueryItem.GetRec().IsByRef());
        // return record set
        return TPair<TBool, PRecSet>(false, QueryItem.GetRec().ToRecSet());
    } else if (QueryItem.IsRecSet()) {
        // return record set
        return TPair<TBool, PRecSet>(false, QueryItem.GetRecSet());
    } else if (QueryItem.IsStore()) {
        // return whole store as record set
        const uint StoreId = QueryItem.GetStoreId();
        const TWPt<TStore> Store = GetStoreByStoreId(StoreId);
        return TPair<TBool, PRecSet>(false, Store->GetAllRecs());
    } else {
        // we have an operator, make sure it is so!
        QmAssert(QueryItem.IsAnd() || QueryItem.IsOr() || QueryItem.IsNot());
        // exeucte all interal query items
        TBoolV NotV; TRecSetV RecSetV;
        for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
            // do subsequent search
            TPair<TBool, PRecSet> NotRecSet = _Search(QueryItem.GetItem(ItemN));
            NotV.Add(NotRecSet.Val1); RecSetV.Add(NotRecSet.Val2);
        }
        // merge the results according to the operator
        if (QueryItem.IsAnd()) {
            // prepare working vectors with the first records set
            TUInt64IntKdV ResRecIdFqV = RecSetV[0]->GetRecIdFqV();
            QmAssert(ResRecIdFqV.IsSorted());
            // current negation status
            bool NotP = NotV[0];
            // than handle the rest here
            for (int ItemN = 1; ItemN < RecSetV.Len(); ItemN++) {
                // get the vector
                const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
                // decide for the operation based on not status
                if (!NotP && !NotV[ItemN]) {
                    // life is easy, just do the intersect
                    Index->GetSumMerger()->Intrs(ResRecIdFqV, RecIdFqV);
                } else if (NotP && NotV[ItemN]) {
                    // all negation, do the union
                    Index->GetSumMerger()->Union(ResRecIdFqV, RecIdFqV);
                } else if (NotP && !NotV[ItemN]) {
                    // records from RecIdFqV should not be in the main
                    TUInt64IntKdV _ResRecIdFqV;
                    Index->GetSumMerger()->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
                    ResRecIdFqV = _ResRecIdFqV;
                    NotP = false;
                } else if (!NotP && NotV[ItemN]) {
                    // records from main should not be in the RecIdFqV
                    TUInt64IntKdV _ResRecIdFqV;
                    Index->GetSumMerger()->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
                    ResRecIdFqV = _ResRecIdFqV;
                    NotP = false;
                }
            }
            // prepare resulting record set
            PRecSet RecSet = TRecSet::New(RecSetV[0]->GetStore(), ResRecIdFqV, QueryItem.IsFq());
            return TPair<TBool, PRecSet>(NotP, RecSet);
        } else if (QueryItem.IsOr()) {
            // prepare working vectors with the first records set
            TUInt64IntKdV ResRecIdFqV = RecSetV[0]->GetRecIdFqV();
            QmAssert(ResRecIdFqV.IsSorted());
            // current negation status
            bool NotP = NotV[0];
            // than handle the rest here
            for (int ItemN = 1; ItemN < RecSetV.Len(); ItemN++) {
                // get the vector
                const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
                // decide for the operation based on not status
                if (!NotP && !NotV[ItemN]) {
                    Index->GetSumMerger()->Union(ResRecIdFqV, RecIdFqV);
                } else if (NotP && NotV[ItemN]) {
                    // all negation, do the intersect
                    Index->GetSumMerger()->Intrs(ResRecIdFqV, RecIdFqV);
                } else if (NotP && !NotV[ItemN]) {
                    // records not from main or from RecIdFqV
                    TUInt64IntKdV _ResRecIdFqV;
                    Index->GetSumMerger()->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
                    ResRecIdFqV = _ResRecIdFqV;
                    NotP = true;
                } else if (!NotP && NotV[ItemN]) {
                    // records from main or not from RecIdFqV
                    TUInt64IntKdV _ResRecIdFqV;
                    Index->GetSumMerger()->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
                    ResRecIdFqV = _ResRecIdFqV;
                    NotP = true;
                }
            }
            // prepare resulting record set
            PRecSet RecSet = TRecSet::New(RecSetV[0]->GetStore(), ResRecIdFqV, QueryItem.IsFq());
            return TPair<TBool, PRecSet>(NotP, RecSet);
        } else if (QueryItem.IsNot()) {
            // just return records set but negate the current negation status
            QmAssert(RecSetV.Len() == 1);
            return TPair<TBool, PRecSet>(!NotV[0], RecSetV[0]);
        }
    }
    // we should never have come to here
    throw TQmExcept::New("Unsupported query item type");
}

void TBase::LoadBaseConf(const TStr& FPath) {
    const TStr BaseConfFNm = GetConfFNm(FPath);
    PJsonVal BaseConfJson = nullptr;

    if (!TFile::Exists(BaseConfFNm)) {
        BaseConfJson = TJsonVal::NewObj();
    } else {
        PSIn BasePropsFIn = TFIn::New(BaseConfFNm);
        BaseConfJson = TJsonVal::GetValFromSIn(BasePropsFIn);
    }

    NmValidator.SetStrictNmP(BaseConfJson->GetObjBool("strictNames", true));
}

void TBase::SaveBaseConf(const TStr& FPath) const {
    PJsonVal BaseConfJson = TJsonVal::NewObj();

    BaseConfJson->AddToObj("strictNames", NmValidator.IsStrictNmP());

    const TStr BaseConfStr = TJsonVal::GetStrFromVal(BaseConfJson);
    TFOut BasePropsFOut(GetConfFNm(FPath));
    BasePropsFOut.PutStr(BaseConfStr);
    BasePropsFOut.Flush();
}

TBase::TBase(const TStr& _FPath, const int64& IndexCacheSize, const int& SplitLen,
        const bool& StrictNmP): InitP(false), NmValidator(StrictNmP) {

    IAssertR(TEnv::IsInit(), "QMiner environment (TQm::TEnv) is not initialized");
    // open as create
    FAccess = faCreate; FPath = _FPath;
    TEnv::Logger->OnStatus("Opening in create mode");
    // prepare index
    IndexVoc = TIndexVoc::New();
    Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize, IndexCacheSize, IndexCacheSize, SplitLen);
    // initialize store blob base
    StoreBlobBs = TMBlobBs::New(FPath + "StoreBlob", FAccess);
    // initialize with empty stores
    StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
    // initialize empty stream aggregate bases for each store
    StreamAggrSetV.Gen(TEnv::GetMxStores()); StreamAggrSetV.PutAll(NULL);
}

TBase::TBase(const TStr& _FPath, const TFAccess& _FAccess, const int64& IndexCacheSize,
        const int& SplitLen): InitP(false), NmValidator(true) {

    IAssertR(TEnv::IsInit(), "QMiner environment (TQm::TEnv) is not initialized");
    // assert open type and remember location
    FAccess = _FAccess; FPath = _FPath;
    EAssert(FAccess == faRdOnly || FAccess == faUpdate || FAccess == faRestore);
    if (FAccess == faRdOnly) {
        TEnv::Logger->OnStatus("Opening in read-only mode");
    } else if (FAccess == faUpdate) {
        TEnv::Logger->OnStatus("Opening in update mode");
    } else if (FAccess == faRestore) {
        TEnv::Logger->OnStatus("Opening in restore mode");
    }

    // open file input streams
    TFIn IndexVocFIn(FPath + "IndexVoc.dat");

    // load index
    IndexVoc = TIndexVoc::Load(IndexVocFIn);
    Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize, IndexCacheSize, IndexCacheSize, SplitLen);
    // load shared store blob base
    StoreBlobBs = TMBlobBs::New(FPath + "StoreBlob", FAccess);
    // initialize with empty stores
    StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
    // initialize empty stream aggregate bases for each store
    StreamAggrSetV.Gen(TEnv::GetMxStores()); StreamAggrSetV.PutAll(NULL);

    // load the base properties
    LoadBaseConf(_FPath);
}

TBase::~TBase() {
    if (FAccess != faRdOnly) {
        TEnv::Logger->OnStatus("Saving index vocabulary ... ");

        TFOut IndexVocFOut(FPath + "IndexVoc.dat");
        IndexVoc->Save(IndexVocFOut);

        SaveBaseConf(FPath);
    } else {
        TEnv::Logger->OnStatus("No saving of qminer base neccessary!");
    }
}

bool TBase::Exists(const TStr& FPath) {
    return TIndex::Exists(FPath) &&
        TFile::Exists(FPath + "IndexVoc.dat") &&
        TFile::Exists(FPath + "StreamAggr.dat");
}

void TBase::Init() {
    InitP = true;
}

TWPt<TIndex> TBase::GetIndex() const {
    return TWPt<TIndex>(Index);
}

void TBase::AddStore(const PStore& NewStore) {
    const uint StoreId = NewStore->GetStoreId();
    QmAssertR(StoreId < TEnv::GetMxStores(), "Store ID to large: " + TUInt::GetStr(StoreId));
    // remember pointer to store
    StoreV[StoreId] = NewStore;
    // fast map from store name to store
    StoreH.AddDat(NewStore->GetStoreNm(), NewStore);
    // create stream aggregate base for the store
    PStreamAggr StreamAggrSet = TStreamAggrSet::New(this);
    // register aggregate to keep afloat
    AddStreamAggr(StreamAggrSet);
    // create trigger for the aggregate base
    NewStore->AddTrigger(TStreamAggrTrigger::New(StreamAggrSet));
    // remember the aggregate base for the store
    StreamAggrSetV[StoreId] = dynamic_cast<TStreamAggrSet*>(StreamAggrSet());
}

const TWPt<TStore> TBase::GetStoreByStoreN(const int& StoreN) const {
    Assert(IsStoreN(StoreN));
    return StoreH[StoreN];
}

const TWPt<TStore> TBase::GetStoreByStoreId(const uint& StoreId) const {
    Assert(IsStoreId(StoreId));
    return StoreV[(int)StoreId];
}

const TWPt<TStore> TBase::GetStoreByStoreNm(const TStr& StoreNm) const {
    QmAssertR(IsStoreNm(StoreNm), "Unknown store name " + StoreNm);
    return StoreH.GetDat(StoreNm);
}

PJsonVal TBase::GetStoreJson(const TWPt<TStore>& Store) {
    return Store->GetStoreJson(this);
}

bool TBase::IsStreamAggr(const TStr& StreamAggrNm) const {
    return StreamAggrH.IsKey(StreamAggrNm);
}

void TBase::AddStreamAggr(const PStreamAggr& StreamAggr) {
    QmAssertR(!IsStreamAggr(StreamAggr->GetAggrNm()),
        "Aggregate with this name already exists: " + StreamAggr->GetAggrNm());
    StreamAggrH.AddDat(StreamAggr->GetAggrNm(), StreamAggr);
}

TWPt<TStreamAggr> TBase::GetStreamAggr(const TStr& StreamAggrNm) const {
    QmAssertR(IsStreamAggr(StreamAggrNm), "Unknown stream aggregate: " + StreamAggrNm);
    return dynamic_cast<TStreamAggr*>(StreamAggrH.GetDat(StreamAggrNm)());
}

TWPt<TStreamAggrSet> TBase::GetStreamAggrSet(const uint& StoreId) const {
    return dynamic_cast<TStreamAggrSet*>(StreamAggrSetV[(int)StoreId]());
}

void TBase::Aggr(PRecSet& RecSet, const TQueryAggrV& QueryAggrV) {
    if (RecSet->Empty()) { return; }
    for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
        const TQueryAggr& Aggr = QueryAggrV[QueryAggrN];
        RecSet->AddAggr(TAggr::New(this, RecSet, Aggr));
    }
}

int TBase::NewIndexWordVoc(const TIndexKeyType& Type, const TStr& WordVocNm) {
    if ((Type & oiktValue) || (Type & oiktText)) {
        // check if we have a vocabulary with such name
        int WordVocId = WordVocNm.Empty() ? -1 : IndexVoc->GetWordVoc(WordVocNm);
        // if no, create a new one
        if (WordVocId == -1) { WordVocId = IndexVoc->NewWordVoc(); }
        // if we are given a name, assign it to the vocabulary
        if (!WordVocNm.Empty()) { IndexVoc->SetWordVocNm(WordVocId, WordVocNm); }
        // done
        return WordVocId;
    }
    // other indices do not need word vocabulary
    return -1;
}

int TBase::NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId,
        const int& WordVocId, const TIndexKeyType& Type, const TIndexKeyGixType& GixType,
        const TIndexKeySortType& SortType) {

    // make sure we do not have the key already
    QmAssertR(!IndexVoc->IsKeyNm(Store->GetStoreId(), KeyNm),
        "Key " + Store->GetStoreNm() + "." + KeyNm + " already exists!");
    // register key in the index vocabulary
    const int KeyId = IndexVoc->AddKey(Store->GetBase(),
        Store->GetStoreId(), KeyNm, WordVocId, Type, GixType, SortType);
    // connect key and field in the vocabualry and store
    IndexVoc->AddKeyField(KeyId, Store->GetStoreId(), FieldId);
    Store->AddFieldKey(FieldId, KeyId);
    // return id of created key
    return KeyId;
}

uint64 TBase::AddRec(const TWPt<TStore>& Store, const PJsonVal& RecVal) {
    QmAssertR(RecVal->IsObj(), "Invalid input JSon, not an object");
    return Store->AddRec(RecVal);
}

uint64 TBase::AddRec(const TStr& StoreNm, const PJsonVal& RecVal) {
    return AddRec(GetStoreByStoreNm(StoreNm), RecVal);
}

uint64 TBase::AddRec(const uint& StoreId, const PJsonVal& RecVal) {
    return AddRec(GetStoreByStoreId(StoreId), RecVal);
}

PRecSet TBase::Search(const PQuery& Query) {
    // do the search
    TPair<TBool, PRecSet> NotRecSet = _Search(Query->GetQueryItem());
    // take the resulting record set
    PRecSet RecSet = NotRecSet.Val2;
    Assert(!RecSet.Empty());
    // if result should be negated, do the invert
    if (NotRecSet.Val1) { RecSet = Invert(RecSet); }
    // get the aggregates
    Aggr(RecSet, Query->GetAggrItemV());
    // sort if necessary
    if (Query->IsSort()) { Query->Sort(this, RecSet); }
    // trim if necessary
    if (Query->IsLimit()) { RecSet = Query->GetLimit(RecSet); }
    // return what we have, trimed if necessary
    return RecSet;
}

PRecSet TBase::Search(const TQueryItem& QueryItem) {
    return Search(TQuery::New(this, QueryItem));
}

PRecSet TBase::Search(const TStr& QueryStr) {
    return Search(TQuery::New(this, QueryStr));
}

PRecSet TBase::Search(const PJsonVal& QueryVal) {
    return Search(TQuery::New(this, QueryVal));
}

void TBase::GarbageCollect() {
    int StoreKeyId = StoreH.FFirstKeyId();
    while (StoreH.FNextKeyId(StoreKeyId)) {
        StoreH[StoreKeyId]->GarbageCollect();
    }
}

bool TBase::SaveJSonDump(const TStr& DumpDir) {
    TStrSet SeenJoinsH;

    const int Stores = GetStores();
    TTm CurrentTime = TTm::GetCurLocTm();

    for (int S = 0; S < Stores; S++) {
        const PStore Store = GetStoreByStoreN(S);
        const TStr StoreNm = Store->GetStoreNm();
        PSOut OutRecs = TFOut::New(DumpDir + StoreNm + ".json");
        PSOut OutJoins = TFOut::New(DumpDir + StoreNm + "-joins.json");

        // joins to store - only index joins and the ones we didn't already store by reverse join
        TStrV JoinV;
        for (int J = 0; J < Store->GetJoins(); J++) {
            TJoinDesc JoinDesc = Store->GetJoinDesc(J);
            const TStr JoinNm = JoinDesc.GetJoinNm();

            if (JoinDesc.IsInverseJoinId()) {
                const int InvJoinId = JoinDesc.GetInverseJoinId();
                const PStore InvStore = JoinDesc.GetJoinStore(this);
                const TJoinDesc InvJoinDesc = InvStore->GetJoinDesc(InvJoinId);
                // if we've already added the inverse join, ignore this one
                if (SeenJoinsH.IsKey(InvStore->GetStoreNm() + "-" + InvJoinDesc.GetJoinNm())) {
                    continue;
                }
            }
            SeenJoinsH.AddKey(StoreNm + "-" + JoinNm);
            JoinV.Add(JoinNm);
            OutJoins->PutStrFmtLn("# JoinNm: %s, JoinId: %d", JoinNm.CStr(), JoinDesc.GetJoinId());
        }

        TQm::TEnv::Logger->OnStatusFmt("Backing up store %s", StoreNm.CStr());

        const uint64 Recs = Store->GetRecs();
        PStoreIter Iter = Store->GetIter();
        while (Iter->Next()) {
            const uint64 RecId = Iter->GetRecId();

            // easy part. dump records
            const PJsonVal RecJson = Store->GetRec(RecId).GetJson(this, true, false);
            const TStr JsonStr = TJsonVal::GetStrFromVal(RecJson);
            OutRecs->PutStrLn(JsonStr);

            // dump joins
            for (int J = 0; J < JoinV.Len(); J++) {
                PRecSet RecSet = Store->GetRec(RecId).DoJoin(this, JoinV[J]);
                if (RecSet->GetRecs() > 0) {
                    OutJoins->PutStrFmt("%I64u|%d|", RecId, Store->GetJoinId(JoinV[J]));
                    const int Recs = RecSet->GetRecs();
                    for (int R = 0; R < Recs; R++) {
                        OutJoins->PutStrFmt("%I64u,%d", RecSet->GetRecId(R), RecSet->GetRecFq(R));
                        if (R < Recs - 1)
                            OutJoins->PutStr(";");
                    }
                    OutJoins->PutLn();
                }
            }

            if (RecId % 1000 == 0) {
                TQm::TEnv::Logger->OnStatusFmt("Record %I64u / %I64u (%.1f%%)\r", RecId, Recs, 100 * RecId / (double)Recs);
            }
        }
    }
    uint64 DiffSecs = TTm::GetDiffSecs(TTm::GetCurLocTm(), CurrentTime);
    int Mins = (int)(DiffSecs / 60);
    TQm::TEnv::Logger->OnStatusFmt("Time needed to make the backup: %d min, %d sec", Mins, (int)(DiffSecs - (Mins * 60)));

    return true;
}

bool TBase::RestoreJSonDump(const TStr& DumpDir) {
    const int Stores = GetStores();
    TTm CurrentTime = TTm::GetCurLocTm();

    THash<TStr, THash<TUInt64, TUInt64> > StoreOldToNewIdHH;
    for (int S = 0; S < Stores; S++) {
        PStore Store = GetStoreByStoreN(S);
        const TStr StoreNm = Store->GetStoreNm();
        THash<TUInt64, TUInt64> OldToNewIdH;
        TQm::TEnv::Logger->OnStatusFmt("Adding recs for store %s", StoreNm.CStr());
        if (TFile::Exists(DumpDir + StoreNm + ".json")) {
            PSIn InRecs = TFIn::New(DumpDir + StoreNm + ".json");
            TStr Line;
            while (InRecs->GetNextLn(Line)) {
                const PJsonVal Json = TJsonVal::GetValFromStr(Line);
                const uint64 ExRecId = Json->IsObjKey("$id") ? (uint64)Json->GetObjNum("$id") : TUInt64::Mx;
                Json->DelObjKey("$id");
                const uint64 RecId = Store->AddRec(Json);
                OldToNewIdH.AddDat(ExRecId, RecId);
                // validate that the added rec id is the same as the one that was saved in json
                // if this fails then we have a problem since the joins will point different records than in the original data
                //AssertR(ExRecId == TUInt64::Mx || ExRecId == RecId, "The added record id does not match the one in the record json");
                if (RecId % 1000 == 0) {
                    TQm::TEnv::Logger->OnStatusFmt("Added record %I64u\r", RecId);
                }
            }
        } else {
            TQm::TEnv::Logger->OnStatusFmt("WARNING: File for store %s is missing. No data was imported.", StoreNm.CStr());
        }
        StoreOldToNewIdHH.AddDat(StoreNm, OldToNewIdH);
    }

    for (int S = 0; S < Stores; S++) {
        const PStore Store = GetStoreByStoreN(S);
        const TStr StoreNm = Store->GetStoreNm();

        if (TFile::Exists(DumpDir + StoreNm + "-joins.json")) {
            TQm::TEnv::Logger->OnStatusFmt("Adding joins for store %s", StoreNm.CStr());

            THash<TUInt64, TUInt64>& OldToNewIdH = StoreOldToNewIdHH.GetDat(StoreNm);
            PSIn InRecs = TFIn::New(DumpDir + StoreNm + "-joins.json");
            TStr Line;
            // if the schema was changed then join ids are likely different. we have to use the
            // name of the stored id and see into which it maps now in the new schema.
            // mapping from old join ids (from old schema) to new join ids (in new schema)
            THash<TInt, TInt> OldJoinIdToNewIdH;
            // mapping from new join ids (from new schema) to store name
            THash<TInt, TStr> NewJoinIdToStoreNmH;
            while (InRecs->GetNextLn(Line)) {
                if (Line.Len() > 0 && Line[0] == '#') {
                    // parse join name and join id when the data was stored
                    // "# JoinNm: %s, JoinId: %d
                    const int NameStart = Line.SearchCh(':') + 2;
                    const int NameEnd = Line.SearchCh(',', NameStart);
                    const int IdStart = Line.SearchChBack(':') + 2;
                    const TStr OldName = Line.GetSubStr(NameStart, NameEnd - 1);
                    const TStr OldIdStr = Line.GetSubStr(IdStart);
                    const int OldId = OldIdStr.GetInt(TInt::Mx);
                    AssertR(OldId != TInt::Mx, "Failed to parse join id from the header: " + Line);

                    if (!Store->IsJoinNm(OldName)) {
                        TQm::TEnv::Logger->OnStatusFmt("WARNING: The new schema does not contain join named %s. Ignoring it.", OldName.CStr());
                    }
                    const int NewId = Store->GetJoinId(OldName);
                    OldJoinIdToNewIdH.AddDat(OldId, NewId);
                    TStr JoinStoreNm = Store->GetJoinDesc(NewId).GetJoinStore(this)->GetStoreNm();
                    NewJoinIdToStoreNmH.AddDat(NewId, JoinStoreNm);
                    if (OldId != NewId) {
                        TQm::TEnv::Logger->OnStatusFmt("INFO: Join id for %s changed from %d to %d.", OldName.CStr(), OldId, NewId);
                    }
                    continue;
                }

                TStrV PartV; Line.SplitOnAllCh('|', PartV, false);
                AssertR(PartV.Len() == 3, TStr::Fmt("The line with json data did not contain three parts when split with |. Store Name: %s, Line val: %s", StoreNm.CStr(), Line.CStr()));
                const uint64 OldRecId = PartV[0].GetUInt64();
                // map old rec ids to new rec ids
                const uint64 NewRecId = OldToNewIdH.GetDat(OldRecId);
                if (!Store->IsRecId(NewRecId)) {
                    TQm::TEnv::Logger->OnStatusFmt("ERROR: Failed to create join for missing record %I64U in store %s.", NewRecId, StoreNm.CStr());
                    continue;
                }

                const int OldJoinId = PartV[1].GetInt();
                // if we don't have the old join anymore then ignore the data for it
                if (OldJoinIdToNewIdH.IsKey(OldJoinId) == false) {
                    continue;
                }

                // get the id for the new join and then a mapping from old to new ids;
                const int NewJoinId = OldJoinIdToNewIdH.GetDat(OldJoinId);
                const TStr JoinStoreNm = NewJoinIdToStoreNmH.GetDat(NewJoinId);
                THash<TUInt64, TUInt64>& JoinOldToNewIdH = StoreOldToNewIdHH.GetDat(JoinStoreNm);

                TStrV JoinV; PartV[2].SplitOnAllCh(';', JoinV);
                const int Joins = JoinV.Len();
                for (int N = 0; N < Joins; N++) {
                    TStr JoinRecIdStr, JoinFqStr; JoinV[N].SplitOnCh(JoinRecIdStr, ',', JoinFqStr);
                    const uint64 OldJoinRecId = JoinRecIdStr.GetUInt64();
                    // Ff some articles or other data was deleted from the index then old and
                    // new ids could be different. In most cases it should be the same.
                    const uint64 NewJoinRecId = JoinOldToNewIdH.GetDat(OldJoinRecId);
                    const int JoinFq = JoinFqStr.GetInt();
                    Store->AddJoin(NewJoinId, NewRecId, NewJoinRecId, JoinFq);
                }
                if (NewRecId % 1000 == 0) {
                    TQm::TEnv::Logger->OnStatusFmt("Added joins for rec %I64u\r", NewRecId);
                }
            }
        }
    }

    uint64 DiffSecs = TTm::GetDiffSecs(TTm::GetCurLocTm(), CurrentTime);
    int Mins = (int)(DiffSecs / 60);
    TQm::TEnv::Logger->OnStatusFmt("Time needed to make the restore: %d min, %d sec", Mins, (int)(DiffSecs - (Mins * 60)));

    return true;
}

void TBase::PrintStores(const TStr& FNm, const bool& FullP) {
    TFOut FOut(FNm);
    for (int StoreN = 0; StoreN < GetStores(); StoreN++) {
        TWPt<TStore> Store = GetStoreByStoreN(StoreN);
        FOut.PutStrLn("--------------------------------------------------------------");
        if (FullP) {
            Store->PrintAll(this, FOut);
        } else {
            Store->PrintTypes(this, FOut);
        }
    }
    FOut.PutStrLn("--------------------------------------------------------------");
}

void TBase::PrintIndexVoc(const TStr& FNm) {
    IndexVoc->SaveTxt(this, FNm);
}

void TBase::PrintIndex(const TStr& FNm, const bool& SortP) {
    Index->SaveTxt(this, FNm);
    if (SortP) {
        TIntKdV SizeIdV;
        PBigStrPool StrPool = TBigStrPool::New();
        {TFIn FIn(FNm); TStr LnStr;
        while (FIn.GetNextLn(LnStr)) {
            if (LnStr.IsWs()) { continue; }
            TStr SizeStr = LnStr.RightOfLast('\t');
            int Size;
            if (SizeStr.IsInt(Size)) {
                SizeIdV.Add(TIntKd(Size, StrPool->AddStr(LnStr)));
            }
        }}
        SizeIdV.Sort(false); TFOut FOut(FNm);
        for (int SizeIdN = 0; SizeIdN < SizeIdV.Len(); SizeIdN++) {
            const int StrId = SizeIdV[SizeIdN].Dat;
            FOut.PutStrLn(StrPool->GetStr(StrId));
        }
    }
}

// perform partial flush of data
int TBase::PartialFlush(int WndInMsec) {
    int dirty_stores = (GetStores() + 1);
    int saved = 100;
    int res = 0;
    TTmStopWatch sw(true);

    TVec<TPair<TWPt<TStore>, bool>> xstores;
    bool xindex = true;

    for (int i = 0; i < GetStores(); i++) {
        xstores.Add(TPair<TWPt<TStore>, bool>(GetStoreByStoreN(i), true));
    }

    while (saved > 0) {
        if (sw.GetMSecInt() > WndInMsec) {
            break; // time is up
        }
        int slice = WndInMsec / dirty_stores; // time-slice per store
        dirty_stores = 0;
        saved = 0; // how many saved in this loop
        int xsaved = 0; // how many saved in this loop into last store/index
        for (int i = 0; i < xstores.Len(); i++) {
            if (!xstores[i].Val2)
                continue; // this store had no dirty data in previous loop
            xsaved = xstores[i].Val1->PartialFlush(slice);
            if (xsaved == 0) {
                xstores[i].Val2 = false; // ok, this store is clean now
            } else {
                dirty_stores++;
                saved += xsaved;
            }
            TQm::TEnv::Debug->OnStatusFmt("Partial flush:     store %s = %d", xstores[i].Val1->GetStoreNm().CStr(), xsaved);
        }
        if (xindex) { // save index
            xsaved = Index->PartialFlush(slice);
            xindex = (xsaved > 0);
            if (xindex) {
                dirty_stores++;
            }
            saved += xsaved;
            TQm::TEnv::Debug->OnStatusFmt("Partial flush:     index = %d", xsaved);
        }
        res += saved;
        TQm::TEnv::Debug->OnStatusFmt("Partial flush: this loop = %d", saved);
    }
    sw.Stop();
    TQm::TEnv::Debug->OnStatusFmt("Partial flush: %d msec, res = %d", sw.GetMSecInt(), res);

    return res;
}

/// get performance statistics in JSON form
PJsonVal TBase::GetStats() {
    PJsonVal res = TJsonVal::NewObj();

    PJsonVal stores = TJsonVal::NewArr();
    for (int i = 0; i < GetStores(); i++) {
        stores->AddToArr(GetStoreByStoreN(i)->GetStats());
    }
    res->AddToObj("stores", stores);
    TGixStats gix_stats = GetGixStats();
    TBlobBsStats gix_blob_stats = GetGixBlobStats();
    res->AddToObj("gix_stats", GixStatsToJson(gix_stats));
    res->AddToObj("gix_blob", BlobBsStatsToJson(gix_blob_stats));
    res->AddToObj("access", GetFAccess());
    return res;
}

PJsonVal TBase::GetStreamAggrStats() const {
    PJsonVal ResVal = TJsonVal::NewObj();

    // go over all stream aggregates and get their execution times
    PJsonVal AggrsVal = TJsonVal::NewArr();
    // we also collect aggregate statistics per type and overall
    int AllCount = 0; double AllExeMSecs = 0.0;
    uint64 TotalMemUsed = 0;
    THash<TStr, TIntFltUInt64Tr> AggrTypeExeMSecsH;
    for (const auto& StreamAggr : StreamAggrH) {
        // get stream aggregate name
        const TStr& AggrNm = StreamAggr.Key;
        // get stream aggregate type
        const TStr AggrType = StreamAggr.Dat->Type();
        // get execution time in miliseconds
        const double ExeMSecs = StreamAggr.Dat->GetExeTm().GetMSec();
        // get the memory footprint
        const uint64 MemUsed = StreamAggr.Dat->GetMemUsed();
        // store to result output
        PJsonVal AggrVal = TJsonVal::NewObj();
        AggrVal->AddToObj("name", AggrNm);
        AggrVal->AddToObj("type", AggrType);
        AggrVal->AddToObj("msecs", ExeMSecs);
        AggrVal->AddToObj("mem", MemUsed);
        AggrsVal->AddToArr(AggrVal);
        // add to aggregate counts
        AllCount++; AllExeMSecs += ExeMSecs;
        TotalMemUsed += MemUsed;
        AggrTypeExeMSecsH.AddDat(AggrType).Val1++;
        AggrTypeExeMSecsH.AddDat(AggrType).Val2 += ExeMSecs;
        AggrTypeExeMSecsH.AddDat(AggrType).Val3 += MemUsed;
    }
    ResVal->AddToObj("aggregates", AggrsVal);

    // prepare aggregate statistics output
    PJsonVal TypesVal = TJsonVal::NewArr();
    for (const auto& AggrTypeExeMSecs : AggrTypeExeMSecsH) {
        // get type
        const TStr& AggrType = AggrTypeExeMSecs.Key;
        // get number of aggregates
        const int AggrCount = AggrTypeExeMSecs.Dat.Val1;
        // get time spent in the aggregate
        const double ExeMSecs = AggrTypeExeMSecs.Dat.Val2;
        // get the memory footprint
        const uint64 MemUsed = AggrTypeExeMSecs.Dat.Val3;
        // store to output
        PJsonVal TypeVal = TJsonVal::NewObj();
        TypeVal->AddToObj("type", AggrType);
        TypeVal->AddToObj("count", AggrCount);
        TypeVal->AddToObj("msecs", ExeMSecs);
        TypeVal->AddToObj("mem", MemUsed);
        TypesVal->AddToArr(TypeVal);
    }
    ResVal->AddToObj("types", TypesVal);

    // get overall stats
    ResVal->AddToObj("count", AllCount);
    ResVal->AddToObj("msecs", AllExeMSecs);
    ResVal->AddToObj("mem", TotalMemUsed);

    // we are good!
    return ResVal;
}

////////////////////////////////////////////////////////////////////////////////
// Export TBlobBsStats object to JSON
PJsonVal BlobBsStatsToJson(const TBlobBsStats& stats) {
    PJsonVal res = TJsonVal::NewObj();
    res->AddToObj("alloc_count", stats.AllocCount);
    res->AddToObj("alloc_size", stats.AllocSize);
    res->AddToObj("alloc_unused_size", stats.AllocUnusedSize);
    res->AddToObj("alloc_used_size", stats.AllocUsedSize);
    res->AddToObj("avg_get_len", stats.AvgGetLen);
    res->AddToObj("avg_put_len", stats.AvgPutLen);
    res->AddToObj("avg_put_new_len", stats.AvgPutNewLen);
    res->AddToObj("dels", stats.Dels);
    res->AddToObj("gets", stats.Gets);
    res->AddToObj("puts", stats.Puts);
    res->AddToObj("puts_new", stats.PutsNew);
    res->AddToObj("released_count", stats.ReleasedCount);
    res->AddToObj("released_size", stats.ReleasedSize);
    res->AddToObj("size_changes", stats.SizeChngs);
    return res;
}

/// Export TGixStats object to JSON
PJsonVal GixStatsToJson(const TGixStats& stats) {
    PJsonVal res = TJsonVal::NewObj();
    res->AddToObj("avg_len", stats.AvgLen);
    res->AddToObj("cache_all", stats.CacheAll);
    res->AddToObj("cache_all_loaded_perc", stats.CacheAllLoadedPerc);
    res->AddToObj("cache_dirty", stats.CacheDirty);
    res->AddToObj("cache_dirty_loaded_perc", stats.CacheDirtyLoadedPerc);
    res->AddToObj("mem_sed", (uint64)stats.MemUsed);
    return res;
}

}

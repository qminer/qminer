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
// QMiner-Valid-Name-Enforcer
TChA TValidNm::ValidFirstCh = "_";
TChA TValidNm::ValidCh = "_$";

void TValidNm::AssertValidNm(const TStr& NmStr) {
	// must be non-empty
	QmAssertR(!NmStr.Empty(), "Name: cannot be empty");
	// check first character
	const char FirstCh = NmStr[0];
	if ((('A' <= FirstCh) && (FirstCh <= 'Z')) || (('a' <= FirstCh) && (FirstCh <= 'z')) || ValidFirstCh.IsChIn(FirstCh)) {
		// all fine
	} else {
		throw TQmExcept::New("Name: invalid first character in '" + NmStr + "'");
	}
	// check rest
	for (int ChN = 1; ChN < NmStr.Len(); ChN++) {
		const char Ch = NmStr[ChN];
		if ((('A' <= Ch) && (Ch <= 'Z')) || (('a' <= Ch) && (Ch <= 'z')) || (('0' <= Ch) && (Ch <= '9')) || ValidCh.IsChIn(Ch)) {
			// all fine
		} else {
			throw TQmExcept::New(TStr::Fmt("Name: invalid %d character in '%s'", ChN, NmStr.CStr()));
		}
	}
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
// QMiner-Join-Description
TJoinDesc::TJoinDesc(const TStr& _JoinNm, const uint& _JoinStoreId,
	const uint& StoreId, const TWPt<TIndexVoc>& IndexVoc, const bool& IsSmall) :
	JoinId(-1), InverseJoinId(-1) {

	// remember join parameters
	JoinStoreId = _JoinStoreId;
	JoinNm = _JoinNm;
	JoinType = osjtIndex;
	JoinRecFieldId = -1;
	JoinFqFieldId = -1;
	// create an internal join key in the index
	TStr JoinKeyNm = "Join" + JoinNm;
	JoinKeyId = IndexVoc->AddInternalKey(StoreId, JoinKeyNm, JoinNm, IsSmall);
	// assert the name is valid
	TValidNm::AssertValidNm(JoinNm);
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
TFieldDesc::TFieldDesc(const TStr& _FieldNm, TFieldType _FieldType,
	const bool& PrimaryP, const bool& NullP, const bool& InternalP) :
	FieldId(-1), FieldNm(_FieldNm), FieldType(_FieldType) {

	TValidNm::AssertValidNm(FieldNm);
	// set flags
	if (PrimaryP) { Flags.Val |= ofdfPrimary; }
	if (NullP) { Flags.Val |= ofdfNull; }
	if (InternalP) { Flags.Val |= ofdfInternal; }
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
	TValidNm::AssertValidNm(StoreNm);
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
                    ErrorLog("[TStoreImpl::AddJoinRec] Expected object for join " + JoinDesc.GetJoinNm());
                    if (JoinRecVal->IsArr()) { ErrorLog("[TStoreImpl::AddJoinRec] Got array instead"); }
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
                    ErrorLog("[TStoreImpl::AddJoinRec] Join frequency must be positive");
                    JoinFq = 1;
                }
                // mark the join
                AddJoin(JoinDesc.GetJoinId(), RecId, JoinRecId, JoinFq);
            } else if (JoinDesc.IsIndexJoin()) {
                // index joins must be in an array
                PJsonVal JoinArrVal = RecVal->GetObjKey(JoinDesc.GetJoinNm());
                if (!JoinArrVal->IsArr()) {
                    ErrorLog("[TStoreImpl::AddJoinRec] Expected array for join " + JoinDesc.GetJoinNm());
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
                        ErrorLog("[TStoreImpl::AddJoinRec] Join frequency must be positive");
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
	return TRec(this, RecId);
}

TRec TStore::GetRec(const TStr& RecNm) {
	return TRec(this, GetRecId(RecNm));
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
/*
int TStore::GetFieldInt(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Int");
}
int16 TStore::GetFieldInt16(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Int16");
}
int64 TStore::GetFieldInt64(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Int64");
}
uchar TStore::GetFieldByte(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Byte");
}

void TStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
	throw FieldError(FieldId, "IntV");
}

uint TStore::GetFieldUInt(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "UInt");
}
uint16 TStore::GetFieldUInt16(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "UInt16");
}
uint64 TStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "UInt64");
}

uint64 TStore::GetFieldUInt64Safe(const uint64& RecId, const int& FieldId) const {
    throw FieldError(FieldId, "UInt64");
}
int64 TStore::GetFieldInt64Safe(const uint64& RecId, const int& FieldId) const {
    throw FieldError(FieldId, "Int64");
}

TStr TStore::GetFieldStr(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Str");
}

void TStore::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
	throw FieldError(FieldId, "StrV");
}

bool TStore::GetFieldBool(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Bool");
}

double TStore::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Flt");
}

float TStore::GetFieldSFlt(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Flt");
}

TFltPr TStore::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "FltPr");
}

void TStore::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
	throw FieldError(FieldId, "FltV");
}

void TStore::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const {
	throw FieldError(FieldId, "Tm");
}

uint64 TStore::GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const {
	TTm Tm; GetFieldTm(RecId, FieldId, Tm);
	return Tm.IsDef() ? TTm::GetMSecsFromTm(Tm) : TUInt64::Mx;
}

void TStore::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const {
	throw FieldError(FieldId, "NumSpV");
}

void TStore::GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const {
	throw FieldError(FieldId, "BowSpV");
}

void TStore::GetFieldTMem(const uint64& RecId, const int& FieldId, TMem& Mem) const {
	throw FieldError(FieldId, "TMem");
}

PJsonVal TStore::GetFieldJsonVal(const uint64& RecId, const int& FieldId) const {
	throw FieldError(FieldId, "Json");
}
*/

/// Get field value using field id safely
uint64 TStore::GetFieldUInt64Safe(const uint64& RecId, const int& FieldId) const {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: return GetFieldByte(RecId, FieldId); break;
    case oftInt16: return GetFieldInt16(RecId, FieldId); break;
    case oftInt: return GetFieldInt(RecId, FieldId); break;
    case oftInt64: return (uint64)GetFieldInt64(RecId, FieldId); break;
    case oftUInt16: return GetFieldUInt16(RecId, FieldId); break;
    case oftUInt: return GetFieldUInt(RecId, FieldId); break;
    case oftUInt64: return GetFieldUInt64(RecId, FieldId); break;
    default: QmAssertR(false, TStr("GetFieldUInt64Safe: unsupported conversion for field id ") + FieldId);
    }
}

/// Get field value using field id safely
int64 TStore::GetFieldInt64Safe(const uint64& RecId, const int& FieldId) const {
    switch (GetFieldDesc(FieldId).GetFieldType()) {
    case oftByte: return GetFieldByte(RecId, FieldId); break;
    case oftInt16: return GetFieldInt16(RecId, FieldId); break;
    case oftInt: return GetFieldInt(RecId, FieldId); break;
    case oftInt64: return GetFieldInt64(RecId, FieldId); break;
    case oftUInt16: return GetFieldUInt16(RecId, FieldId); break;
    case oftUInt: return GetFieldUInt(RecId, FieldId); break;
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
/*
void TStore::SetFieldNull(const uint64& RecId, const int& FieldId) {
	throw FieldError(FieldId, "SetNull");
}

void TStore::SetFieldByte(const uint64& RecId, const int& FieldId, const uchar& Byte) {
	throw FieldError(FieldId, "Byte");
}

void TStore::SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int) {
	throw FieldError(FieldId, "Int");
}

void TStore::SetFieldInt16(const uint64& RecId, const int& FieldId, const int16& Int16) {
	throw FieldError(FieldId, "Int16");
}

void TStore::SetFieldInt64(const uint64& RecId, const int& FieldId, const int64& Int64) {
	throw FieldError(FieldId, "Int64");
}

void TStore::SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV) {
	throw FieldError(FieldId, "IntV");
}

void TStore::SetFieldUInt(const uint64& RecId, const int& FieldId, const uint& UInt16) {
	throw FieldError(FieldId, "UInt");
}

void TStore::SetFieldUInt16(const uint64& RecId, const int& FieldId, const uint16& UInt16) {
	throw FieldError(FieldId, "UInt16");
}

void TStore::SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
	throw FieldError(FieldId, "UInt64");
}

/// Set field value using field id (default implementation throws exception)
void TStore::SetFieldUInt64Safe(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
    throw FieldError(FieldId, "UInt64");
}
/// Set field value using field id (default implementation throws exception)
void TStore::SetFieldInt64Safe(const uint64& RecId, const int& FieldId, const int64& Int64) {
    throw FieldError(FieldId, "Int64");
}


void TStore::SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str) {
	throw FieldError(FieldId, "Str");
}

void TStore::SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV) {
	throw FieldError(FieldId, "StrV");
}

void TStore::SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool) {
	throw FieldError(FieldId, "Bool");
}

void TStore::SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt) {
	throw FieldError(FieldId, "Flt");
}

void TStore::SetFieldSFlt(const uint64& RecId, const int& FieldId, const float& Flt) {
	throw FieldError(FieldId, "SFlt");
}

void TStore::SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr) {
	throw FieldError(FieldId, "FltPr");
}

void TStore::SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV) {
	throw FieldError(FieldId, "FltV");
}

void TStore::SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm) {
	throw FieldError(FieldId, "Tm");
}

void TStore::SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs) {
	throw FieldError(FieldId, "TmMSecs");
}

void TStore::SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV) {
	throw FieldError(FieldId, "NumSpV");
}

void TStore::SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV) {
	throw FieldError(FieldId, "BowSpV");
}

void TStore::SetFieldTMem(const uint64& RecId, const int& FieldId, const TMem& Mem) {
	throw FieldError(FieldId, "TMem");
}

void TStore::SetFieldJsonVal(const uint64& RecId, const int& FieldId, const PJsonVal& Json) {
	throw FieldError(FieldId, "Json");
}
*/

void TStore::SetFieldNmNull(const uint64& RecId, const TStr& FieldNm) {
	SetFieldNull(RecId, GetFieldId(FieldNm));
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
				SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TInt64::GetStr(FieldInt).CStr());
			} else if (Desc.IsUInt16()) {
				const uint64 FieldInt = GetFieldUInt16(RecId, FieldId);
				SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TInt64::GetStr(FieldInt).CStr());
			} else if (Desc.IsUInt64()) {
				const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
				SOut.PutStrFmtLn("  %s: %s", Desc.GetFieldNm().CStr(), TInt64::GetStr(FieldInt).CStr());
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
			TMem Mem;
			TStr::Base64Decode(FieldVal->GetStr(), Mem);
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
		if (FieldTm.IsDef()) { return TJsonVal::NewStr(FieldTm.GetWebLogDateTimeStr(true, "T", true)); } else { return TJsonVal::NewNull(); }
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
	JoinRecSet->GetRecIdFqV().Save(RecValOut);
}

PRecSet TRec::ToRecSet() const {
	QmAssertR(IsByRef(), "Cannot transform record passed by value to a set!");
	return IsDef() ? TRecSet::New(Store, RecId) : TRecSet::New(Store);
}

int TRec::GetFieldJoinFq(const int& JoinId) const {
    if (!Store->IsJoinId(JoinId)) {
        return -1;
    }
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    return GetFieldJoinFq(JoinDesc);
}

int TRec::GetFieldJoinFq(const TJoinDesc& JoinDesc) const {
    if (!JoinDesc.IsFieldJoin()) {
        return -1;
    }
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
            // prepare join query
            TIntUInt64PrV JoinQueryV = TIntUInt64PrV::GetV(TIntUInt64Pr(JoinKeyId, GetRecId()));
            // execute join query
            Base->GetIndex()->SearchOr(JoinQueryV, JoinRecIdFqV);
        } else {
            // do join using serialized record set
            if (JoinIdPosH.IsKey(JoinId)) {
                const int Pos = JoinIdPosH.GetKey(JoinId);
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
    PRecSet JoinRecSet = DoJoin(Base, JoinId);
    return TRec(JoinRecSet->GetStore(),
        JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0),
        GetFieldJoinFq(JoinId));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const {
    PRecSet JoinRecSet = DoJoin(Base, JoinNm);
    return TRec(JoinRecSet->GetStore(),
        JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0),
        GetFieldJoinFq(Store->GetJoinId(JoinNm)));
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

///////////////////////////////////////////////
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
	const bool& WgtSampleP, TUInt64IntKdV& SampleRecIdFqV) const {

	if (SampleSize == -1) {
        // we ask for all
		SampleRecIdFqV = RecIdFqV;
    } else if (SampleSize == 0) {
        // we ask for nothing
        SampleRecIdFqV.Clr();
    } else if (SampleSize > GetRecs()) {
        // we ask for more than we have, have to give it all
        SampleRecIdFqV = RecIdFqV;
	} else if (WgtSampleP) {
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

TRecSet::TRecSet(const TWPt<TStore>& _Store, const uint64& RecId, const int& Wgt) :
	Store(_Store), WgtP(Wgt > 1) {

	RecIdFqV.Gen(1, 0); RecIdFqV.Add(TUInt64IntKd(RecId, Wgt));
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TUInt64V& RecIdV) : Store(_Store), WgtP(false) {
	RecIdFqV.Gen(RecIdV.Len(), 0);
	for (int RecN = 0; RecN < RecIdV.Len(); RecN++) {
		RecIdFqV.Add(TUInt64IntKd(RecIdV[RecN], 0));
	}
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TIntV& RecIdV) : Store(_Store), WgtP(false) {
	RecIdFqV.Gen(RecIdV.Len(), 0);
	int Len = RecIdV.Len();
	for (int RecN = 0; RecN < Len; RecN++) {
		RecIdFqV.Add(TUInt64IntKd((uint64)RecIdV[RecN], 0));
	}
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TUInt64IntKdV& _RecIdFqV,
	const bool& _WgtP): Store(_Store), WgtP(_WgtP), RecIdFqV(_RecIdFqV) { }

TRecSet::TRecSet(const TWPt<TBase>& Base, TSIn& SIn) {
	Store = TStore::LoadById(Base, SIn);
	WgtP.Load(SIn);
	RecIdFqV.Load(SIn);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV,
        const bool& WgtP) {

	return new TRecSet(Store, RecIdFqV, WgtP);
}

PRecSet TRecSet::New(const TWPt<TStore>& Store) {
	return new TRecSet(Store, TUInt64V());
}

PRecSet TRecSet::New(const TWPt<TStore>& Store, const uint64& RecId, const int& Wgt) {
	return new TRecSet(Store, RecId, Wgt);
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
	WgtP.Save(SOut);
	RecIdFqV.Save(SOut);
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
		SortCmp(TRecCmpByFieldInt(Store, SortFieldId, Asc));
	} else if (Desc.IsFlt()) {
		SortCmp(TRecCmpByFieldFlt(Store, SortFieldId, Asc));
	} else if (Desc.IsStr()) {
		SortCmp(TRecCmpByFieldStr(Store, SortFieldId, Asc));
	} else if (Desc.IsTm()) {
		SortCmp(TRecCmpByFieldTm(Store, SortFieldId, Asc));
	} else {
		throw TQmExcept::New("Unsupported sort field type!");
	}
}

void TRecSet::FilterByExists() {
	// apply filter
	FilterBy(TRecFilterByExists(Store));
}

void TRecSet::FilterByRecId(const uint64& MinRecId, const uint64& MaxRecId) {
	// apply filter
	FilterBy(TRecFilterByRecId(MinRecId, MaxRecId));
}

void TRecSet::FilterByRecIdSet(const TUInt64Set& RecIdSet) {
	// apply filter
	FilterBy(TRecFilterByRecIdSet(RecIdSet, true));
}

void TRecSet::FilterByFq(const int& MinFq, const int& MaxFq) {
	// apply filter
	FilterBy(TRecFilterByRecFq(MinFq, MaxFq));
}

void TRecSet::FilterByFieldBool(const int& FieldId, const bool& Val) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsBool(), "Wrong field type, boolean expected");
	// apply the filter
	FilterBy(TRecFilterByFieldBool(Store, FieldId, Val));
}

void TRecSet::FilterByFieldInt(const int& FieldId, const int& MinVal, const int& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsInt(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldInt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldInt16(const int& FieldId, const int16& MinVal, const int16& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsInt16(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldInt16(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldInt64(const int& FieldId, const int64& MinVal, const int64& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsInt64(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldInt64(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldByte(const int& FieldId, const uchar& MinVal, const uchar& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsByte(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldUCh(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldUInt(const int& FieldId, const uint& MinVal, const uint& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsUInt(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldUInt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldUInt16(const int& FieldId, const uint16& MinVal, const uint16& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsUInt16(), "Wrong field type, integer expected");
	// apply the filter
	FilterBy(TRecFilterByFieldUInt16(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldFlt(const int& FieldId, const double& MinVal, const double& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsFlt(), "Wrong field type, numeric expected");
	// apply the filter
	FilterBy(TRecFilterByFieldFlt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldSFlt(const int& FieldId, const float& MinVal, const float& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsFlt(), "Wrong field type, numeric expected");
	// apply the filter
	FilterBy(TRecFilterByFieldSFlt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldStr(const int& FieldId, const TStr& FldVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
	// apply the filter
	FilterBy(TRecFilterByFieldStr(Store, FieldId, FldVal));
}

void TRecSet::FilterByFieldStrMinMax(const int& FieldId, const TStr& FldVal, const TStr& FldValMax) {
    // get store and field type
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
    // apply the filter
    FilterBy(TRecFilterByFieldStrMinMax(Store, FieldId, FldVal, FldValMax));
}

void TRecSet::FilterByFieldStrSet(const int& FieldId, const TStrSet& ValSet) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
	// apply the filter
	FilterBy(TRecFilterByFieldStrSet(Store, FieldId, ValSet));
}

void TRecSet::FilterByFieldTm(const int& FieldId, const uint64& MinVal, const uint64& MaxVal) {

	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsTm(), "Wrong field type, time expected");
	// apply the filter
	FilterBy(TRecFilterByFieldTm(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldTm(const int& FieldId, const TTm& MinVal, const TTm& MaxVal) {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsTm(), "Wrong field type, time expected");
	// apply the filter
	FilterBy(TRecFilterByFieldTm(Store, FieldId, MinVal, MaxVal));
}

TVec<PRecSet> TRecSet::SplitByFieldTm(const int& FieldId, const uint64& DiffMSecs) const {
	// get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	QmAssertR(Desc.IsTm(), "Wrong field type, time expected");
	// split the record set
	return SplitBy(TRecSplitterByFieldTm(Store, FieldId, DiffMSecs));
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
	FilterBy(TRecFilterByRecIdSet(RemoveItemIdSet, false));
}

PRecSet TRecSet::Clone() const {
	return new TRecSet(Store, RecIdFqV, WgtP);
}

PRecSet TRecSet::GetSampleRecSet(const int& SampleSize) const {
	TUInt64IntKdV SampleRecIdFqV;
	GetSampleRecIdV(SampleSize, WgtP, SampleRecIdFqV);
	return new TRecSet(Store, SampleRecIdFqV, WgtP);
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
		return new TRecSet(Store, LimitRecIdFqV, WgtP);
	}
}

PRecSet TRecSet::GetMerge(const PRecSet& RecSet) const {
	PRecSet CloneRecSet = Clone();
	CloneRecSet->Merge(RecSet);
	return CloneRecSet;
}

//PRecSet TRecSet::GetMerge(const TVec<PRecSet>& RecSetV) {
//	if (RecSetV.Len() == 0)
//		return TRecSet::New();
//	PRecSet RecSet = RecSetV[0]->Clone();
//	for (int N = 1; N < RecSetV.Len(); N++)
//		RecSet->Merge(RecSetV[N]);
//	return RecSet;
//}

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

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const int& JoinId, const int& SampleSize) const {
    // get join info
    AssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
    const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
    // prepare joined record sample
    TUInt64IntKdV SampleRecIdKdV;
    GetSampleRecIdV(SampleSize, WgtP, SampleRecIdKdV);
    const int SampleRecs = SampleRecIdKdV.Len();
    // do the join
    TUInt64IntKdV JoinRecIdFqV;
    if (JoinDesc.IsIndexJoin()) {
        // do join using index
        const int JoinKeyId = JoinDesc.GetJoinKeyId();
        // prepare join query
        TIntUInt64PrV JoinQueryV;
        for (int RecN = 0; RecN < SampleRecs; RecN++) {
            const uint64 RecId = SampleRecIdKdV[RecN].Key;
            JoinQueryV.Add(TIntUInt64Pr(JoinKeyId, RecId));
        }
        // execute join query
        Base->GetIndex()->SearchOr(JoinQueryV, JoinRecIdFqV);
    } else if (JoinDesc.IsFieldJoin()) {
        // do join using store field
        TUInt64H JoinRecIdFqH;
        const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
        for (int RecN = 0; RecN < SampleRecs; RecN++) {
            const uint64 RecId = SampleRecIdKdV[RecN].Key;
            const uint64 JoinRecId = Store->GetFieldUInt64Safe(RecId, JoinRecFieldId);
            if (JoinRecId != TUInt64::Mx) {
                const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
                int JoinRecFq = 1;
                if (JoinFqFieldId >= 0) {
                    JoinRecFq = (int)Store->GetFieldInt64Safe(RecId, JoinFqFieldId);
                }
                JoinRecIdFqH.AddDat(JoinRecId) += JoinRecFq;
            }
        }
        JoinRecIdFqH.GetKeyDatKdV(JoinRecIdFqV);
    } else {
        // unknown join type
        throw TQmExcept::New("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
    }
    // create new RecSet
    return new TRecSet(JoinDesc.GetJoinStore(Base), JoinRecIdFqV, true);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm, const int& SampleSize) const {

    if (Store->IsJoinNm(JoinNm)) {
        return DoJoin(Base, Store->GetJoinId(JoinNm), SampleSize);
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
		StoreVal->AddToObj("$wgt", IsWgt());
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
TIndexKey::TIndexKey(const uint& _StoreId, const TStr& _KeyNm, const TStr& _JoinNm,
    const bool& IsSmall): StoreId(_StoreId), KeyNm(_KeyNm), WordVocId(-1),
    TypeFlags(oiktInternal), SortType(oikstUndef), JoinNm(_JoinNm) {

    if (IsSmall) {
        TypeFlags = (TIndexKeyType)(TypeFlags | oiktSmall);
    }
    TValidNm::AssertValidNm(KeyNm);
}

TIndexKey::TIndexKey(const uint& _StoreId, const TStr& _KeyNm, const int& _WordVocId,
	const TIndexKeyType& _Type, const TIndexKeySortType& _SortType) : StoreId(_StoreId),
	KeyNm(_KeyNm), WordVocId(_WordVocId), TypeFlags(_Type), SortType(_SortType) {

	// no internal keys allowed here
	QmAssert(!IsInternal());
	// value or text keys require vocabulary
	if (IsValue() || IsText()) { QmAssert(WordVocId >= 0); }
	// location does not need vocabualry
	if (IsLocation()) { QmAssert(WordVocId == -1); }
	// name must be valid
	TValidNm::AssertValidNm(KeyNm);
}

TIndexKey::TIndexKey(TSIn& SIn) : StoreId(SIn), KeyId(SIn),
	KeyNm(SIn), WordVocId(SIn),
	TypeFlags(LoadEnum<TIndexKeyType>(SIn)),
	SortType(LoadEnum<TIndexKeySortType>(SIn)),
	FieldIdV(SIn), JoinNm(SIn), Tokenizer(SIn) {}

void TIndexKey::Save(TSOut& SOut) const {
	StoreId.Save(SOut);
	KeyId.Save(SOut); KeyNm.Save(SOut); WordVocId.Save(SOut);
	SaveEnum<TIndexKeyType>(SOut, TypeFlags);
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

int TIndexVoc::AddKey(const uint& StoreId, const TStr& KeyNm, const int& WordVocId,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	// create key
	const int KeyId = KeyH.AddKey(TUIntStrPr(StoreId, KeyNm));
	KeyH[KeyId] = TIndexKey(StoreId, KeyNm, WordVocId, Type, SortType);
	// tell to the key its ID
	KeyH[KeyId].PutKeyId(KeyId);
	// add the key to the associated store key set
	StoreIdKeyIdSetH.AddDat(StoreId).AddKey(KeyId);
	return KeyId;
}

int TIndexVoc::AddInternalKey(const uint& StoreId, const TStr& KeyNm, const TStr& JoinNm, const bool& IsSmall) {
	const int KeyId = KeyH.AddKey(TUIntStrPr(StoreId, KeyNm));
	KeyH[KeyId] = TIndexKey(StoreId, KeyNm, JoinNm, IsSmall);
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

void TIndexVoc::AddWordIdV(const int& KeyId, const TStrV& TextStrV, TUInt64V& WordIdV) {
	QmAssert(IsWordVoc(KeyId));
	// tokenize string
	TStrV TokV;
	const PTokenizer& Tokenizer = GetTokenizer(KeyId);
	for (int StrN = 0; StrN < TextStrV.Len(); StrN++) {
		Tokenizer->GetTokens(TextStrV[StrN], TokV);
	}
	WordIdV.Gen(TokV.Len(), 0); const PIndexWordVoc& WordVoc = GetWordVoc(KeyId);
	for (int TokN = 0; TokN < TokV.Len(); TokN++) {
		WordIdV.Add(WordVoc->AddWordStr(TokV[TokN]));
	}
	WordVoc->IncRecs();
}

void TIndexVoc::GetWcWordIdV(const int& KeyId, const TStr& WcStr, TUInt64V& WcWordIdV) {
	QmAssert(IsWordVoc(KeyId));
	GetWordVoc(KeyId)->GetWcWordIdV(WcStr, WcWordIdV);
}

void TIndexVoc::GetAllGreaterV(const int& KeyId,
	const uint64& StartWordId, TKeyWordV& AllGreaterV) {

	// get all the words matching criteria
	TUInt64V WordIdV;
	if (KeyH[KeyId].IsSortById()) {
		GetWordVoc(KeyId)->GetAllGreaterById(StartWordId, WordIdV);
	} else if (KeyH[KeyId].IsSortByStr()) {
		GetWordVoc(KeyId)->GetAllGreaterByStr(StartWordId, WordIdV);
	} else if (KeyH[KeyId].IsSortByFlt()) {
		GetWordVoc(KeyId)->GetAllGreaterByFlt(StartWordId, WordIdV);
	}
	// add key id
	AllGreaterV.Gen(WordIdV.Len(), 0);
	for (int WordN = 0; WordN < WordIdV.Len(); WordN++) {
		AllGreaterV.Add(TKeyWord(KeyId, (uint64)WordIdV[WordN]));
	}
}

void TIndexVoc::GetAllLessV(const int& KeyId,
	const uint64& StartWordId, TKeyWordV& AllLessV) {

	// get all the words matching criteria
	TUInt64V WordIdV;
	if (KeyH[KeyId].IsSortById()) {
		GetWordVoc(KeyId)->GetAllLessById(StartWordId, WordIdV);
	} else if (KeyH[KeyId].IsSortByStr()) {
		GetWordVoc(KeyId)->GetAllLessByStr(StartWordId, WordIdV);
	} else if (KeyH[KeyId].IsSortByFlt()) {
		GetWordVoc(KeyId)->GetAllLessByFlt(StartWordId, WordIdV);
	}
	// add key id
	AllLessV.Gen(WordIdV.Len(), 0);
	for (int WordN = 0; WordN < WordIdV.Len(); WordN++) {
		AllLessV.Add(TKeyWord(KeyId, (uint64)WordIdV[WordN]));
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
	// if wildchar, identify relevant words
	if (IsWildChar()) {
		// get all matching words
		IndexVoc->GetWcWordIdV(KeyId, WordStr, WordIdV);
		// we are done 
		return;
	}
	// otherwise just retrieve matching word id, when word exists
	if (IndexVoc->IsWordStr(KeyId, WordStr)) {
		WordIdV.Add(IndexVoc->GetWordId(KeyId, WordStr));
	}
	// throw exception if no word recognized from the query string in case of range queries
	if (WordIdV.Empty() && (IsLess() || IsGreater())) {
		// behavior not defined for this case
		throw TQmExcept::New(TStr::Fmt("Unknown query string %d:'%s'!", KeyId.Val, WordStr.CStr()));
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
		Type = oqitAnd;	TWPt<TStore> Store;
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
	SetGixFlag();
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
	SetGixFlag();
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
        // check if normal or small gix
        Type = Key.IsSmall() ? oqitLeafGixSmall : oqitLeafGix;
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
            QmAssertR(Key.IsSortAsInt() || Key.IsSortAsUInt64() || Key.IsSortAsFlt(),
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
    SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TStore>& Store, const uint64& RecId) :
	Type(oqitRec), Rec(Store, RecId) { SetGixFlag(); }

TQueryItem::TQueryItem(const TRec& _Rec) :
	Type(oqitRec), Rec(_Rec) { SetGixFlag(); }

TQueryItem::TQueryItem(const PRecSet& _RecSet) :
	Type(oqitRecSet), RecSet(_RecSet) {
	RecSet->SortById();
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
	const uint64& WordId, const TQueryCmpType& _CmpType) : 
	KeyId(_KeyId), CmpType(_CmpType) {
	Type = (Base->GetIndexVoc()->GetKey(KeyId).IsSmall() ? oqitLeafGixSmall : oqitLeafGix);
	Base->GetIndexVoc()->GetKey(KeyId).IsSmall();
	WordIdV.Add(WordId);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
	const TStr& WordStr, const TQueryCmpType& _CmpType)  {
	// read the Key
	KeyId = _KeyId;
	QmAssertR(Base->GetIndexVoc()->IsKeyId(KeyId), "Unknown Key ID: " + KeyId.GetStr());
	Type = (Base->GetIndexVoc()->GetKey(KeyId).IsSmall() ? oqitLeafGixSmall : oqitLeafGix);
	// read the sort type
	CmpType = _CmpType;
	// parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm,
	const TStr& WordStr, const TQueryCmpType& _CmpType) {

	// get the key
	QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	Type = (Base->GetIndexVoc()->GetKey(KeyId).IsSmall() ? oqitLeafGixSmall : oqitLeafGix);
	// read sort type
	CmpType = _CmpType;
	// parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& KeyNm,
	const TStr& WordStr, const TQueryCmpType& _CmpType) {

	// get the key
	const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
	QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	Type = (Base->GetIndexVoc()->GetKey(KeyId).IsSmall() ? oqitLeafGixSmall : oqitLeafGix);
	// read sort type
	CmpType = _CmpType;
	// parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
	const TFltPr& _Loc, const int& _LocLimit, const double& _LocRadius) :
	Type(oqitGeo), KeyId(_KeyId), Loc(_Loc), LocRadius(_LocRadius),
	LocLimit(_LocLimit) {
	QmAssert(LocLimit > 0);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId,
	const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit,
	const double& _LocRadius) : Type(oqitGeo), Loc(_Loc),
	LocRadius(_LocRadius), LocLimit(_LocLimit) {

	QmAssert(LocLimit > 0);
	// get the key
	QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	SetGixFlag();
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
	SetGixFlag();
}

TQueryItem::TQueryItem(const TQueryItemType& _Type) : Type(_Type) {
	QmAssert(Type == oqitAnd || Type == oqitOr);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item) :
	Type(_Type), ItemV(1, 0) {
	ItemV.Add(Item);
	QmAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item1,
	const TQueryItem& Item2) : Type(_Type), ItemV(2, 0) {
	ItemV.Add(Item1); ItemV.Add(Item2);
	QmAssert(Type == oqitAnd || Type == oqitOr);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItemV& _ItemV) :
	Type(_Type), ItemV(_ItemV) {
	QmAssert(Type == oqitAnd || Type == oqitOr);
	SetGixFlag();
}

TQueryItem::TQueryItem(const int& _JoinId, const int& _SampleSize, const TQueryItem& Item) :
	Type(oqitJoin), ItemV(1, 0), JoinId(_JoinId), SampleSize(_SampleSize) {
	ItemV.Add(Item);
	SetGixFlag();
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& JoinNm, const int& _SampleSize,
	const TQueryItem& Item) : Type(oqitJoin), ItemV(1, 0), SampleSize(_SampleSize) {

	ItemV.Add(Item);
	// get join id
	const uint StoreId = Item.GetStoreId(Base);
	const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
	JoinId = Store->GetJoinId(JoinNm);
	SetGixFlag();
}

uint TQueryItem::GetStoreId(const TWPt<TBase>& Base) const {
	if (IsLeafGix() || IsLeafGixSmall() || IsGeo() || IsRange()) {
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

bool TQueryItem::IsWgt() const {

	if (IsLeafGix() || IsLeafGixSmall() || IsGeo()) {
		// always weighted when only one key
		return true;
    } else if (IsAnd() && ItemV.Len() == 1) {
        // we have only one sub-query, check its status
        return ItemV[0].IsWgt();
	} else if (IsOr()) {
		// or is weighted when all it's elements are 
		bool WgtP = true;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			WgtP = WgtP && ItemV[ItemN].IsWgt();
		}
		return WgtP;
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
	
TQueryGixUsedType TQueryItem::GetGixFlag() const {
	//TQueryGixUsedType GixFlag = qgutUnknown;
	//if (IsLeafGix()) {
	//	GixFlag = qgutNormal;
	//} else if (IsLeafGixSmall()) {
	//	GixFlag = qgutSmall;
	//} else {
	//	GixFlag = qgutNone;
	//	int flag = 0;
	//	for (int i = 0; i < GetItems(); i++) {
	//		TQueryGixUsedType res = GetItem(i).GetGixFlag();
	//		if (res == qgutNormal) { flag |= 1; } 
	//		else if (res == qgutSmall) { flag |= 2; } 
	//		else if (res == qgutBoth) { flag |= 3; break; }
	//	}
	//	if (flag == 1) { GixFlag = qgutNormal; } 
	//	else if (flag == 2) { GixFlag = qgutSmall; } 
	//	else if (flag == 3) { GixFlag = qgutBoth; }
	//}
	return GixFlag;
}

void TQueryItem::SetGixFlag() {
	for (int i = 0; i < GetItems(); i++) {
		ItemV[i].SetGixFlag();
	}
	GixFlag = qgutUnknown;
	if (IsLeafGix()) {
		GixFlag = qgutNormal;
	} else if (IsLeafGixSmall()) {
		GixFlag = qgutSmall;
	} else {
		GixFlag = qgutNone;
		int flag = 0;
		for (int i = 0; i < GetItems(); i++) {
			TQueryGixUsedType res = GetItem(i).GetGixFlag();
			if (res == qgutNormal) { flag |= 1; } else if (res == qgutSmall) { flag |= 2; } else if (res == qgutBoth) { flag |= 3; break; }
		}
		if (flag == 1) { GixFlag = qgutNormal; } else if (flag == 2) { GixFlag = qgutSmall; } else if (flag == 3) { GixFlag = qgutBoth; }
	}
}

void TQueryItem::Optimize() {
	for (int i = 0; i < GetItems(); i++) {
		TQueryItem& child = ItemV[i];
		child.Optimize();
		if (child.ItemV.Len() == 1 && (child.Type == oqitAnd || child.Type == oqitOr)) {
			// child is AND/OR with single child => promote grandchild to child
			TQueryItem grandchild = ItemV[i].ItemV[0];
			ItemV[i] = grandchild;
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
	TValidNm::AssertValidNm(AggrNm);
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
void TIndex::TQmGixDefMerger::Union(
	TQmGixItemV& MainV, const TQmGixItemV& JoinV) const {

	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItem& Val1 = MainV.GetVal(ValN1);
		const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ResV.Add(Val1); ValN1++; } else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; } else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
	}
	for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
		ResV.Add(MainV.GetVal(RestValN1));
	}
	for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
		ResV.Add(JoinV.GetVal(RestValN2));
	}
	MainV = ResV;
}

void TIndex::TQmGixDefMerger::Intrs(
	TQmGixItemV& MainV, const TQmGixItemV& JoinV) const {

	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItem& Val1 = MainV.GetVal(ValN1);
		const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ValN1++; } else if (Val1 > Val2) { ValN2++; } else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
	}
	MainV = ResV;
}

void TIndex::TQmGixDefMerger::Minus(const TQmGixItemV& MainV,
	const TQmGixItemV& JoinV, TQmGixItemV& ResV) const {

	MainV.Diff(JoinV, ResV);
}

void TIndex::TQmGixDefMerger::Merge(TQmGixItemV& ItemV, bool Local) const {
	if (ItemV.Empty()) { return; } // nothing to do in this case
	if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted
	// merge counts
	int LastItemN = 0; bool ZeroP = false;
	for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
		if (ItemV[ItemN] != ItemV[ItemN - 1]) {
			LastItemN++;
			ItemV[LastItemN] = ItemV[ItemN];
		} else {
			ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
		}
		ZeroP = (ItemV[LastItemN].Dat <= 0) || ZeroP;
	}
	ItemV.Reserve(ItemV.Reserved(), LastItemN + 1);
	// remove items with zero count
	if (ZeroP) {
		LastItemN = 0;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			const TQmGixItem& Item = ItemV[ItemN];
			if (Item.Dat > 0 || (Local && Item.Dat < 0)) {
				ItemV[LastItemN] = Item;
				LastItemN++;
			} else if (Item.Dat < 0) {
				TEnv::Error->OnStatusFmt("Warning: negative item count %d:%d!", (int)Item.Key, (int)Item.Dat);
			}
		}
		ItemV.Reserve(ItemV.Reserved(), LastItemN);
	}
}

void TIndex::TQmGixDefMergerSmall::Union(
	TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const {

	TQmGixItemSmallV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItemSmall& Val1 = MainV.GetVal(ValN1);
		const TQmGixItemSmall& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ResV.Add(Val1); ValN1++; } else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; } else { ResV.Add(TQmGixItemSmall(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
	}
	for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
		ResV.Add(MainV.GetVal(RestValN1));
	}
	for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
		ResV.Add(JoinV.GetVal(RestValN2));
	}
	MainV = ResV;
}

void TIndex::TQmGixDefMergerSmall::Intrs(
	TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const {

	TQmGixItemSmallV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItemSmall& Val1 = MainV.GetVal(ValN1);
		const TQmGixItemSmall& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ValN1++; } else if (Val1 > Val2) { ValN2++; } else { ResV.Add(TQmGixItemSmall(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
	}
	MainV = ResV;
}

void TIndex::TQmGixDefMergerSmall::Minus(const TQmGixItemSmallV& MainV,
	const TQmGixItemSmallV& JoinV, TQmGixItemSmallV& ResV) const {
	MainV.Diff(JoinV, ResV);
}

void TIndex::TQmGixDefMergerSmall::Merge(TQmGixItemSmallV& ItemV, bool Local) const {
	if (ItemV.Empty()) { return; } // nothing to do in this case
	if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted
	// merge counts
	int LastItemN = 0; bool ZeroP = false;
	for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
		if (ItemV[ItemN] != ItemV[ItemN - 1]) {
			LastItemN++;
			ItemV[LastItemN] = ItemV[ItemN];
		} else {
			ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
		}
		ZeroP = (ItemV[LastItemN].Dat <= 0) || ZeroP;
	}
	ItemV.Reserve(ItemV.Reserved(), LastItemN + 1);
	// remove items with zero count
	if (ZeroP) {
		LastItemN = 0;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			const TQmGixItemSmall& Item = ItemV[ItemN];
			if (Item.Dat > 0 || (Local && (int16)Item.Dat < 0)) {
				ItemV[LastItemN] = Item;
				LastItemN++;
			} else if ((int16)Item.Dat < 0) {
				TEnv::Error->OnStatusFmt("Warning: negative item count %d:%d!", (int)Item.Key, (int)Item.Dat);
			}
		}
		ItemV.Reserve(ItemV.Reserved(), LastItemN);
	}
}

void TIndex::TQmGixRmDupMerger::Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const {
	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItem& Val1 = MainV.GetVal(ValN1);
		const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ResV.Add(TQmGixItem(Val1.Key, 1)); ValN1++; } else if (Val1 > Val2) { ResV.Add(TQmGixItem(Val2.Key, 1)); ValN2++; } else {
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TQmGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++;
		}
	}
	for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
		TQmGixItem Item = MainV.GetVal(RestValN1);
		Item.Dat = TInt::GetMn(1, Item.Dat);
		ResV.Add(Item);
	}
	for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
		TQmGixItem Item = JoinV.GetVal(RestValN2);
		Item.Dat = TInt::GetMn(1, Item.Dat);
		ResV.Add(Item);
	}
	MainV = ResV;
}

void TIndex::TQmGixRmDupMerger::Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const {
	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItem& Val1 = MainV.GetVal(ValN1);
		const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ValN1++; } else if (Val1 > Val2) { ValN2++; } else {
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TQmGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++;
		}
	}
	MainV = ResV;
}

void TIndex::TQmGixRmDupMergerSmall::Union(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const {
	TQmGixItemSmallV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItemSmall& Val1 = MainV.GetVal(ValN1);
		const TQmGixItemSmall& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ResV.Add(TQmGixItemSmall(Val1.Key, 1)); ValN1++; } else if (Val1 > Val2) { ResV.Add(TQmGixItemSmall(Val2.Key, 1)); ValN2++; } else {
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TQmGixItemSmall(Val1.Key, fq1 + fq2)); ValN1++; ValN2++;
		}
	}
	for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
		TQmGixItemSmall Item = MainV.GetVal(RestValN1);
		Item.Dat = TInt::GetMn(1, Item.Dat);
		ResV.Add(Item);
	}
	for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
		TQmGixItemSmall Item = JoinV.GetVal(RestValN2);
		Item.Dat = TInt::GetMn(1, Item.Dat);
		ResV.Add(Item);
	}
	MainV = ResV;
}

void TIndex::TQmGixRmDupMergerSmall::Intrs(TQmGixItemSmallV& MainV, const TQmGixItemSmallV& JoinV) const {
	TQmGixItemSmallV ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TQmGixItemSmall& Val1 = MainV.GetVal(ValN1);
		const TQmGixItemSmall& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) { ValN1++; } else if (Val1 > Val2) { ValN2++; } else {
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TQmGixItemSmall(Val1.Key, fq1 + fq2)); ValN1++; ValN2++;
		}
	}
	MainV = ResV;
}

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

//////////////////////////////////////////////////////////////////////////////////
TIndex::PQmGixExpItem TIndex::ToExpItem(const TQueryItem& QueryItem) const {
	if (QueryItem.IsLeafGix()) {
		// we have a leaf, make it into expresion item
		if (QueryItem.IsEqual()) {
			// ==
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItem::NewAndV(AllKeyV);
		} else if (QueryItem.IsGreater()) {
			// >=
			TKeyWordV AllGreaterV;
			IndexVoc->GetAllGreaterV(QueryItem.GetKeyId(),
				QueryItem.GetWordId(), AllGreaterV);
			return TQmGixExpItem::NewOrV(AllGreaterV);
		} else if (QueryItem.IsLess()) {
			// <=
			TKeyWordV AllLessV;
			IndexVoc->GetAllLessV(QueryItem.GetKeyId(),
				QueryItem.GetWordId(), AllLessV);
			return TQmGixExpItem::NewOrV(AllLessV);
		} else if (QueryItem.IsNotEqual()) {
			// !=
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItem::NewNot(TQmGixExpItem::NewAndV(AllKeyV));
		} else if (QueryItem.IsWildChar()) {
			// ~
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItem::NewOrV(AllKeyV);
		} else {
			// unknown operator
			throw TQmExcept::New("Index: Unknown query item operator");
		}
	} else if (QueryItem.IsAnd()) {
		// we have a vector of AND items
		TVec<PQmGixExpItem> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItem(QueryItem.GetItem(ItemN)));
		}
		return TQmGixExpItem::NewAndV(ExpItemV);
	} else if (QueryItem.IsOr()) {
		// we have a vector of OR items
		TVec<PQmGixExpItem> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItem(QueryItem.GetItem(ItemN)));
		}
		return TQmGixExpItem::NewOrV(ExpItemV);
	} else if (QueryItem.IsNot()) {
		// we have a negation (can have only one child item!)
		QmAssert(QueryItem.GetItems() == 1);
		return TQmGixExpItem::NewNot(ToExpItem(QueryItem.GetItem(0)));
	} else {
		// unknow handle query item type
		const int QueryItemType = (int)QueryItem.GetType();
		throw TQmExcept::New(TStr::Fmt("Index: QueryItem of type %d which must be handled outside TIndex", QueryItemType));
	}
	return TQmGixExpItem::NewEmpty();
}

TIndex::PQmGixExpItemSmall TIndex::ToExpItemSmall(const TQueryItem& QueryItem) const {
	if (QueryItem.IsLeafGixSmall()) {
		// we have a leaf, make it into expresion item
		if (QueryItem.IsEqual()) {
			// ==
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItemSmall::NewAndV(AllKeyV);
		} else if (QueryItem.IsGreater()) {
			// >=
			TKeyWordV AllGreaterV;
			IndexVoc->GetAllGreaterV(QueryItem.GetKeyId(),
				QueryItem.GetWordId(), AllGreaterV);
			return TQmGixExpItemSmall::NewOrV(AllGreaterV);
		} else if (QueryItem.IsLess()) {
			// <=
			TKeyWordV AllLessV;
			IndexVoc->GetAllLessV(QueryItem.GetKeyId(),
				QueryItem.GetWordId(), AllLessV);
			return TQmGixExpItemSmall::NewOrV(AllLessV);
		} else if (QueryItem.IsNotEqual()) {
			// !=
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItemSmall::NewNot(TQmGixExpItemSmall::NewAndV(AllKeyV));
		} else if (QueryItem.IsWildChar()) {
			// ~
			TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
			return TQmGixExpItemSmall::NewOrV(AllKeyV);
		} else {
			// unknown operator
			throw TQmExcept::New("Index: Unknown query item operator");
		}
	} else if (QueryItem.IsAnd()) {
		// we have a vector of AND items
		TVec<PQmGixExpItemSmall> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItemSmall(QueryItem.GetItem(ItemN)));
		}
		return TQmGixExpItemSmall::NewAndV(ExpItemV);
	} else if (QueryItem.IsOr()) {
		// we have a vector of OR items
		TVec<PQmGixExpItemSmall> ExpItemV(QueryItem.GetItems(), 0);
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			ExpItemV.Add(ToExpItemSmall(QueryItem.GetItem(ItemN)));
		}
		return TQmGixExpItemSmall::NewOrV(ExpItemV);
	} else if (QueryItem.IsNot()) {
		// we have a negation (can have only one child item!)
		QmAssert(QueryItem.GetItems() == 1);
		return TQmGixExpItemSmall::NewNot(ToExpItemSmall(QueryItem.GetItem(0)));
	} else {
		// unknow handle query item type
		const int QueryItemType = (int)QueryItem.GetType();
		throw TQmExcept::New(TStr::Fmt("Index: QueryItem of type %d which must be handled outside TIndex", QueryItemType));
	}
	return TQmGixExpItemSmall::NewEmpty();
}

bool TIndex::DoQuery(const TIndex::PQmGixExpItem& ExpItem,
	const PQmGixExpMerger& Merger, TQmGixItemV& ResIdFqV) const {

	// clean if there is anything on the input
	ResIdFqV.Clr();
	// execute query
	return ExpItem->Eval(Gix, ResIdFqV, Merger);
}
	
bool TIndex::DoQuerySmall(const TIndex::PQmGixExpItemSmall& ExpItem,
	const PQmGixExpMergerSmall& Merger, TQmGixItemSmallV& ResIdFqV) const {

	// clean if there is anything on the input
	ResIdFqV.Clr();
	// execute query
	return ExpItem->Eval(GixSmall, ResIdFqV, Merger);
}

void TIndex::Upgrade(const TQmGixItemSmallV& Src, TQmGixItemV& Dest) const {
    Dest.Clr(); Dest.Reserve(Src.Len());
    for (int i = 0; i < Src.Len(); i++) {
        Dest.Add(TQmGixItem((uint64)Src[i].Key, (int)Src[i].Dat));
    }
}

TIndex::TIndex(const TStr& _IndexFPath, const TFAccess& _Access,
	const PIndexVoc& _IndexVoc, const int64& CacheSize, const int64& CacheSizeSmall,
	const int& SplitLen) {

	IndexFPath = _IndexFPath;
	Access = _Access;
	// initialize invered index
	DefMerger = TQmGixDefMerger::New();
	Gix = TQmGix::New("Index", IndexFPath, Access, CacheSize, SplitLen);
	DefMergerSmall = TQmGixDefMergerSmall::New();
	GixSmall = TQmGixSmall::New("IndexSmall", IndexFPath, Access, CacheSizeSmall, SplitLen);
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

TIndex::~TIndex() {
	if (!IsReadOnly()) {
		TEnv::Logger->OnStatus("Saving and closing inverted index");
		Gix.Clr();
		TEnv::Logger->OnStatus("Saving and closing inverted index - small");
		GixSmall.Clr();
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

void TIndex::Index(const int& KeyId, const uint64& WordId, const uint64& RecId) {
	Index(KeyId, WordId, RecId, 1);
}

void TIndex::Index(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
	const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
	Index(KeyId, WordId, RecId, 1);
}

void TIndex::Index(const int& KeyId, const TStrV& WordStrV, const uint64& RecId) {
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
		Index(KeyId, WordId, RecId, WordFq);
	}
}

void TIndex::Index(const int& KeyId, const TStrIntPrV& WordStrFqV, const uint64& RecId) {
	TIntH WordIdH;
	for (int WordN = 0; WordN < WordStrFqV.Len(); WordN++) {
		const TStr WordStr = WordStrFqV[WordN].Val1; //.GetLc();
		const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
		const int WordFq = WordStrFqV[WordN].Val2;
		Index(KeyId, WordId, RecId, WordFq);
	}
}

void TIndex::Index(const uint& StoreId, const TStr& KeyNm,
	const TStr& WordStr, const uint64& RecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStr, RecId);
}

void TIndex::Index(const uint& StoreId, const TStr& KeyNm,
	const TStrV& WordStrV, const uint64& RecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStrV, RecId);
}

void TIndex::Index(const uint& StoreId, const TStr& KeyNm,
	const TStrIntPrV& WordStrFqV, const uint64& StoreRecId) {

	Index(IndexVoc->GetKeyId(StoreId, KeyNm), WordStrFqV, StoreRecId);
}

void TIndex::Index(const uint& StoreId, const TStrPrV& KeyWordV, const uint64& RecId) {
	for (int KeyWordN = 0; KeyWordN < KeyWordV.Len(); KeyWordN++) {
		const TStrPr& KeyWord = KeyWordV[KeyWordN];
		// get key and word id
		const int KeyId = IndexVoc->GetKeyId(StoreId, KeyWord.Val1);
		const uint64 WordId = IndexVoc->AddWordStr(KeyId, KeyWord.Val2);
		// index the record
		Index(KeyId, WordId, RecId, 1);
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
		Index(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
	}
}

void TIndex::IndexText(const uint& StoreId, const TStr& KeyNm,
	const TStr& TextStr, const uint64& RecId) {

	IndexText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStr, RecId);
}

void TIndex::IndexText(const int& KeyId, const TStrV& TextStrV, const uint64& RecId) {
	// tokenize string
	TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStrV, WordIdV);
	// aggregate by word
	TUInt64H WordIdFqH;
	for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
		WordIdFqH.AddDat(WordIdV[WordIdN])++;
	}
	// index words
	int WordKeyId = WordIdFqH.FFirstKeyId();
	while (WordIdFqH.FNextKeyId(WordKeyId)) {
		Index(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
	}
}

void TIndex::IndexText(const uint& StoreId, const TStr& KeyNm,
	const TStrV& TextStrV, const uint64& RecId) {

	IndexText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStrV, RecId);
}

void TIndex::IndexJoin(const TWPt<TStore>& Store, const int& JoinId,
	const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Index(Store->GetJoinKeyId(JoinId), RecId, JoinRecId, JoinFq);
}

void TIndex::IndexJoin(const TWPt<TStore>& Store, const TStr& JoinNm,
	const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Index(Store->GetJoinKeyId(JoinNm), RecId, JoinRecId, JoinFq);
}

void TIndex::Index(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq) {
	// -1 should never come to here 
	Assert(KeyId != -1);
	// we shouldn't modify read-only index
	QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
	// index
	if (UseGixSmall(KeyId)) {
		GixSmall->AddItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, (int16)RecFq));
	} else {
		Gix->AddItem(TKeyWord(KeyId, WordId), TQmGixItem(RecId, RecFq));
	}
}

void TIndex::Delete(const int& KeyId, const TStr& WordStr, const uint64& RecId) {
	const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
	Delete(KeyId, WordId, RecId, 1);
}

void TIndex::Delete(const int& KeyId, const TStrV& WordStrV, const uint64& RecId) {
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
		Delete(KeyId, WordId, RecId, WordFq);
	}
}

void TIndex::Delete(const uint& StoreId, const TStr& KeyNm, const TStr& WordStr, const uint64& RecId) {
	const int KeyId = IndexVoc->GetKeyId(StoreId, KeyNm);
	const uint64 WordId = IndexVoc->AddWordStr(KeyId, WordStr);
	Delete(KeyId, WordId, RecId, 1);
}

void TIndex::Delete(const uint& StoreId, const TStr& KeyNm, const uint64& WordId, const uint64& RecId) {
	const int KeyId = IndexVoc->GetKeyId(StoreId, KeyNm);
	Delete(KeyId, WordId, RecId, 1);
}

void TIndex::Delete(const uint& StoreId, const TStrPrV& KeyWordV, const uint64& RecId) {
	for (int KeyWordN = 0; KeyWordN < KeyWordV.Len(); KeyWordN++) {
		const TStrPr& KeyWord = KeyWordV[KeyWordN];
		// get key and word id
		const int KeyId = IndexVoc->GetKeyId(StoreId, KeyWord.Val1);
		const uint64 WordId = IndexVoc->AddWordStr(KeyId, KeyWord.Val2);
		// index the record
		Delete(KeyId, WordId, RecId, 1);
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
		Delete(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
	}
}

void TIndex::DeleteText(const uint& StoreId, const TStr& KeyNm,
	const TStr& TextStr, const uint64& RecId) {

	DeleteText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStr, RecId);
}

void TIndex::DeleteText(const int& KeyId, const TStrV& TextStrV, const uint64& RecId) {
	// tokenize string
	TUInt64V WordIdV; IndexVoc->AddWordIdV(KeyId, TextStrV, WordIdV);
	// aggregate by word
	TUInt64H WordIdFqH;
	for (int WordIdN = 0; WordIdN < WordIdV.Len(); WordIdN++) {
		WordIdFqH.AddDat(WordIdV[WordIdN])++;
	}
	// index words
	int WordKeyId = WordIdFqH.FFirstKeyId();
	while (WordIdFqH.FNextKeyId(WordKeyId)) {
		Delete(KeyId, WordIdFqH.GetKey(WordKeyId), RecId, WordIdFqH[WordKeyId]);
	}
}

void TIndex::DeleteText(const uint& StoreId, const TStr& KeyNm,
	const TStrV& TextStrV, const uint64& RecId) {

	DeleteText(IndexVoc->GetKeyId(StoreId, KeyNm), TextStrV, RecId);
}

void TIndex::DeleteJoin(const TWPt<TStore>& Store, const int& JoinId,
	const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Delete(Store->GetJoinKeyId(JoinId), RecId, JoinRecId, JoinFq);
}

void TIndex::DeleteJoin(const TWPt<TStore>& Store, const TStr& JoinNm,
	const uint64& RecId, const uint64& JoinRecId, const int& JoinFq) {

	Delete(Store->GetJoinKeyId(JoinNm), RecId, JoinRecId, JoinFq);
}

void TIndex::Delete(const int& KeyId, const uint64& WordId, const uint64& RecId, const int& RecFq) {
	// -1 should never come to here 
	Assert(KeyId != -1);
	// we shouldn't modify read-only index
	QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
	if (RecFq == TInt::Mx) {
		// full delete from index
		if (UseGixSmall(KeyId)) {
			GixSmall->DelItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, 0));
		} else {
			Gix->DelItem(TKeyWord(KeyId, WordId), TQmGixItem(RecId, 0));
		}
	} else {
		// add item with negative count, merger will delete item if necessary
		if (UseGixSmall(KeyId)) {
			GixSmall->AddItem(TKeyWord(KeyId, WordId), TQmGixItemSmall((uint)RecId, (int16)-RecFq));
		} else {
			Gix->AddItem(TKeyWord(KeyId, WordId), TQmGixItem(RecId, -RecFq));
		}
	}
}

void TIndex::Index(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc, const uint64& RecId) {
	Index(IndexVoc->GetKeyId(StoreId, KeyNm), Loc, RecId);
}

void TIndex::Index(const int& KeyId, const TFltPr& Loc, const uint64& RecId) {
	// we shouldn't modify read-only index
	QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
	// if new key, create sphere first
	if (!GeoIndexH.IsKey(KeyId)) { GeoIndexH.AddDat(KeyId, TGeoIndex::New()); }
	// index new location
	GeoIndexH.GetDat(KeyId)->AddKey(Loc, RecId);
}

void TIndex::Delete(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc, const uint64& RecId) {
	Delete(IndexVoc->GetKeyId(StoreId, KeyNm), Loc, RecId);
}

void TIndex::Delete(const int& KeyId, const TFltPr& Loc, const uint64& RecId) {
	// we shouldn't modify read-only index
	QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
	// delete only if index exist 
	if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->DelKey(Loc, RecId); }
}

bool TIndex::LocEquals(const uint& StoreId, const TStr& KeyNm, const TFltPr& Loc1, const TFltPr& Loc2) const {
	return LocEquals(IndexVoc->GetKeyId(StoreId, KeyNm), Loc1, Loc2);
}

bool TIndex::LocEquals(const int& KeyId, const TFltPr& Loc1, const TFltPr& Loc2) const {
	return GeoIndexH.IsKey(KeyId) ? GeoIndexH.GetDat(KeyId)->LocEquals(Loc1, Loc2) : false;
}

void TIndex::IndexLinear(const uint& StoreId, const TStr& KeyNm, const int& Val, const uint64& RecId) {
	IndexLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}

void TIndex::IndexLinear(const uint& StoreId, const TStr& KeyNm, const uint64& Val, const uint64& RecId) {
	IndexLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}

void TIndex::IndexLinear(const uint& StoreId, const TStr& KeyNm, const double& Val, const uint64& RecId) {
	IndexLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
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

void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uchar& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int16& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const int64& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}

void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint16& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const uint64& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}

void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const double& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
}
void TIndex::DeleteLinear(const uint& StoreId, const TStr& KeyNm, const float& Val, const uint64& RecId) {
	DeleteLinear(IndexVoc->GetKeyId(StoreId, KeyNm), Val, RecId);
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

void TIndex::SearchAnd(const TIntUInt64PrV& KeyWordV, TQmGixItemV& StoreRecIdFqV) const {
	// prepare the query
	TVec<PQmGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	TVec<PQmGixExpItemSmall> ExpItemSmallV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {
		if (UseGixSmall(KeyWordV[ItemN].Val1)) {
			ExpItemV.Add(TQmGixExpItem::NewItem(KeyWordV[ItemN]));
		} else {
			ExpItemSmallV.Add(TQmGixExpItemSmall::NewItem(KeyWordV[ItemN]));
		}
	}
	PQmGixExpItem ExpItem = TQmGixExpItem::NewAndV(ExpItemV);
	PQmGixExpItemSmall ExpItemSmall = TQmGixExpItemSmall::NewAndV(ExpItemSmallV);

	// execute the query and filter the results to desired item type
	DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
	TQmGixItemSmallV Tmp;
	DoQuerySmall(ExpItemSmall, DefMergerSmall, Tmp);
	if (Tmp.Len()>0) {
		TQmGixItemV Tmp2;
		Upgrade(Tmp, Tmp2);
		DefMerger->Intrs(StoreRecIdFqV, Tmp2);
	}
}

void TIndex::SearchOr(const TIntUInt64PrV& KeyWordV, TQmGixItemV& StoreRecIdFqV) const {
	// prepare the query
	TVec<PQmGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	TVec<PQmGixExpItemSmall> ExpItemSmallV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {			
		if (UseGixSmall(KeyWordV[ItemN].Val1)) {
			ExpItemSmallV.Add(TQmGixExpItemSmall::NewItem(KeyWordV[ItemN]));
		} else {
			ExpItemV.Add(TQmGixExpItem::NewItem(KeyWordV[ItemN]));
		}
	}
	PQmGixExpItem ExpItem = TQmGixExpItem::NewOrV(ExpItemV);
	PQmGixExpItemSmall ExpItemSmall = TQmGixExpItemSmall::NewOrV(ExpItemSmallV);

	// execute the query and filter the results to desired item type
	DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
	TQmGixItemSmallV Tmp;
	DoQuerySmall(ExpItemSmall, DefMergerSmall, Tmp);
	if (Tmp.Len()>0) {
		TQmGixItemV Tmp2;
		Upgrade(Tmp, Tmp2);
		DefMerger->Union(StoreRecIdFqV, Tmp2);
	}
}

TPair<TBool, PRecSet> TIndex::Search(const TWPt<TBase>& Base, const TQueryItem& QueryItem,
		const PQmGixExpMerger& Merger, const PQmGixExpMergerSmall& MergerSmall) const {

	// get query result store
	TWPt<TStore> Store = QueryItem.GetStore(Base);
	// when query empty, return empty set
	if (QueryItem.Empty()) {
		return TPair<TBool, PRecSet>(false, TRecSet::New(Store));
	}

	// ok, detect which gix is used - big or small one
	TQueryGixUsedType gix_flag = QueryItem.GetGixFlag();
	if (gix_flag == qgutNormal) {
		// prepare the query
		PQmGixExpItem ExpItem = ToExpItem(QueryItem);
		// do the query
		TUInt64IntKdV StoreRecIdFqV;
		const bool NotP = DoQuery(ExpItem, Merger, StoreRecIdFqV);
		// return record set
		PRecSet RecSet = TRecSet::New(Store, StoreRecIdFqV, QueryItem.IsWgt());
		return TPair<TBool, PRecSet>(NotP, RecSet);
	} else if (gix_flag == qgutSmall) {
		// prepare the query
		PQmGixExpItemSmall ExpItem = ToExpItemSmall(QueryItem);
		// do the query
		TQmGixItemSmallV StoreRecIdFqVSmall;
		const bool NotP = DoQuerySmall(ExpItem, MergerSmall, StoreRecIdFqVSmall);
		TQmGixItemV StoreRecIdFqV;
		Upgrade(StoreRecIdFqVSmall, StoreRecIdFqV);
		// return record set
		PRecSet RecSet = TRecSet::New(Store, StoreRecIdFqV, QueryItem.IsWgt());
		return TPair<TBool, PRecSet>(NotP, RecSet);

	}
	throw TQmExcept::New("Error in TIndex::Search - hybrid search is not supported.");
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

void TIndex::GetJoinRecIdFqV(const int& JoinKeyId, const uint64& RecId, TUInt64IntKdV& JoinRecIdFqV) const {
	TKeyWord KeyWord(JoinKeyId, RecId);
	if (UseGixSmall(JoinKeyId)) {
		if (!GixSmall->IsKey(KeyWord)) { return; }
		PQmGixItemSetSmall res = GixSmall->GetItemSet(KeyWord); res->Def();
		for (int i = 0; i < res->GetItems(); i++) {
			auto item = res->GetItem(i);
			JoinRecIdFqV.Add(TUInt64IntKd((uint64)item.Key, (int)item.Dat));
		}
	} else {
		if (!Gix->IsKey(KeyWord)) { return; }
		PQmGixItemSet res = Gix->GetItemSet(KeyWord); res->Def();
		for (int i = 0; i < res->GetItems(); i++) {
			JoinRecIdFqV.Add(res->GetItem(i));
		}
	}
}

void TIndex::SaveTxt(const TWPt<TBase>& Base, const TStr& FNm) {
	Gix->SaveTxt(FNm, TQmGixKeyStr::New(Base, IndexVoc));
	GixSmall->SaveTxt(FNm + ".small", TQmGixKeyStr::New(Base, IndexVoc));
}

TBlobBsStats TIndex::GetBlobStats() const {
    return TBlobBsStats::Add(Gix->GetBlobStats(), GixSmall->GetBlobStats());
}

TGixStats TIndex::GetGixStats(const bool& RefreshP) const {
    return TGixStats::Add(Gix->GetGixStats(RefreshP), GixSmall->GetGixStats(RefreshP));
}

int TIndex::PartialFlush(const int& WndInMsec) {
	int WndInMsecHalf = WndInMsec / 2;
	int Res = 0; int LastRes = 0;
	TTmStopWatch sw(true);
	while (sw.GetMSecInt() <= WndInMsec) {
		Res += Gix->PartialFlush(WndInMsecHalf);
		Res += GixSmall->PartialFlush(WndInMsecHalf);
		if (Res == LastRes) break;
		LastRes = Res;
	}
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
	Register<TStreamAggrs::TRecBuffer>();
	Register<TStreamAggrs::TTimeSeriesTick>();
	// two types of window buffers (different interfaces)
	Register<TStreamAggrs::TWinBufFlt>();
	Register<TStreamAggrs::TWinBufFtrSpVec>();
	// these attach to TWinBufFlt
	Register<TStreamAggrs::TWinBufSum>();
	Register<TStreamAggrs::TWinBufMin>();
	Register<TStreamAggrs::TWinBufMax>();
	Register<TStreamAggrs::TMa>();
	Register<TStreamAggrs::TEma>();
	Register<TStreamAggrs::TVar>();
	Register<TStreamAggrs::TCov>();
	Register<TStreamAggrs::TCorr>();
	Register<TStreamAggrs::TMerger>();
	Register<TStreamAggrs::TResampler>();
	Register<TStreamAggrs::TOnlineHistogram>();
	Register<TStreamAggrs::TChiSquare>();
	Register<TStreamAggrs::TOnlineSlottedHistogram>();
	Register<TStreamAggrs::TVecDiff>();

	// these attach to ISparseVecTm
	Register<TStreamAggrs::TEmaSpVec>();

	// these attach to TWinBufFtrSpVec
	Register<TStreamAggrs::TWinBufSpVecSum>();
}

TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal) :
Base(_Base), AggrNm(ParamVal->GetObjStr("name", TGuid::GenSafeGuid())), Guid(TGuid::GenGuid()) {
	TValidNm::AssertValidNm(AggrNm);
}

PStreamAggr TStreamAggr::New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal) {
	return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

void TStreamAggr::LoadState(TSIn& SIn) {
    throw TQmExcept::New("TStreamAggr::_Load not implemented:" + GetAggrNm());
};

void TStreamAggr::SaveState(TSOut& SOut) const {
    throw TQmExcept::New("TStreamAggr::_Save not implemented:" + GetAggrNm());
};

///////////////////////////////
// QMiner-Stream-Aggregator-Base
PStreamAggrBase TStreamAggrBase::New() {
	return new TStreamAggrBase;
}

bool TStreamAggrBase::Empty() const {
	return StreamAggrH.Empty();
}

int TStreamAggrBase::Len() const {
	return StreamAggrH.Len();
}

bool TStreamAggrBase::IsStreamAggr(const TStr& StreamAggrNm) const {
	return StreamAggrH.IsKey(StreamAggrNm);
}

const PStreamAggr& TStreamAggrBase::GetStreamAggr(const TStr& StreamAggrNm) const {
	QmAssertR(IsStreamAggr(StreamAggrNm), TStr::Fmt("Aggregate not found: %s", StreamAggrNm.CStr()));
	return StreamAggrH.GetDat(StreamAggrNm);
}

const PStreamAggr& TStreamAggrBase::GetStreamAggr(const int& StreamAggrId) const {
	return StreamAggrH[StreamAggrId];
}

void TStreamAggrBase::AddStreamAggr(const PStreamAggr& StreamAggr) {
	QmAssertR(!IsStreamAggr(StreamAggr->GetAggrNm()), TStr::Fmt("Aggregate with this name already exists: %s", StreamAggr->GetAggrNm().CStr()));
	StreamAggrH.AddDat(StreamAggr->GetAggrNm(), StreamAggr);
}

int TStreamAggrBase::GetFirstStreamAggrId() const {
	return StreamAggrH.FFirstKeyId();
}

bool TStreamAggrBase::GetNextStreamAggrId(int& AggrId) const {
	return StreamAggrH.FNextKeyId(AggrId);
}

void TStreamAggrBase::Reset() {
	int AggrId = GetFirstStreamAggrId();
	while (GetNextStreamAggrId(AggrId)) {
		GetStreamAggr(AggrId)->Reset();
	}
}

void TStreamAggrBase::OnAddRec(const TRec& Rec) {
	int KeyId = StreamAggrH.FFirstKeyId();
	while (StreamAggrH.FNextKeyId(KeyId)) {
		StreamAggrH[KeyId]->OnAddRec(Rec);
	}
}

void TStreamAggrBase::OnUpdateRec(const TRec& Rec) {
	int KeyId = StreamAggrH.FFirstKeyId();
	while (StreamAggrH.FNextKeyId(KeyId)) {
		StreamAggrH[KeyId]->OnUpdateRec(Rec);
	}
}

void TStreamAggrBase::OnDeleteRec(const TRec& Rec) {
	int KeyId = StreamAggrH.FFirstKeyId();
	while (StreamAggrH.FNextKeyId(KeyId)) {
		StreamAggrH[KeyId]->OnDeleteRec(Rec);
	}
}

PJsonVal TStreamAggrBase::SaveJson(const int& Limit) const {
	PJsonVal ResVal = TJsonVal::NewArr();
	int KeyId = StreamAggrH.FFirstKeyId();
	while (StreamAggrH.FNextKeyId(KeyId)) {
		ResVal->AddToArr(StreamAggrH[KeyId]->SaveJson(Limit));
	}
	return ResVal;
}

///////////////////////////////
// QMiner-Stream-Aggregator-Trigger
TStreamAggrTrigger::TStreamAggrTrigger(const PStreamAggrBase& _StreamAggrBase) :
	StreamAggrBase(_StreamAggrBase) {}

PStoreTrigger TStreamAggrTrigger::New(const PStreamAggrBase& StreamAggrBase) {
	return new TStreamAggrTrigger(StreamAggrBase);
}

void TStreamAggrTrigger::OnAdd(const TRec& Rec) {
	StreamAggrBase->OnAddRec(Rec);
}

void TStreamAggrTrigger::OnUpdate(const TRec& Rec) {
	StreamAggrBase->OnUpdateRec(Rec);
}

void TStreamAggrTrigger::OnDelete(const TRec& Rec) {
	StreamAggrBase->OnDeleteRec(Rec);
}

///////////////////////////////
// QMiner-Base
TBase::TBase(const TStr& _FPath, const int64& IndexCacheSize, const int& SplitLen) : InitP(false) {
	IAssertR(TEnv::IsInit(), "QMiner environment (TQm::TEnv) is not initialized");
	// open as create
	FAccess = faCreate; FPath = _FPath;
	TEnv::Logger->OnStatus("Opening in create mode");
	// prepare index
	IndexVoc = TIndexVoc::New();
	Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize, IndexCacheSize, SplitLen);
	// initialize with empty stores
	StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
	// initialize empty stream aggregate bases for each store
	StreamAggrBaseV.Gen(TEnv::GetMxStores()); StreamAggrBaseV.PutAll(NULL);
	StreamAggrDefaultBase = TStreamAggrBase::New();
	// by default no temporary folder
	TempFPathP = false;
}

TBase::TBase(const TStr& _FPath, const TFAccess& _FAccess, const int64& IndexCacheSize, const int& SplitLen) : InitP(false) {
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
	// load index
	TFIn IndexVocFIn(FPath + "IndexVoc.dat");
	IndexVoc = TIndexVoc::Load(IndexVocFIn);
	Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize, IndexCacheSize, SplitLen);
	// initialize with empty stores
	StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
	// initialize empty stream aggregate bases for each store
	StreamAggrBaseV.Gen(TEnv::GetMxStores()); StreamAggrBaseV.PutAll(NULL);
	StreamAggrDefaultBase = TStreamAggrBase::New();
	// by default no temporary folder
	TempFPathP = false;
}

TBase::~TBase() {
	if (FAccess != faRdOnly) {
		TEnv::Logger->OnStatus("Saving index vocabulary ... ");
		TFOut IndexVocFOut(FPath + "IndexVoc.dat");
		IndexVoc->Save(IndexVocFOut);
	} else {
		TEnv::Logger->OnStatus("No saving of qminer base neccessary!");
	}
}

PRecSet TBase::Invert(const PRecSet& RecSet, const TIndex::PQmGixExpMerger& Merger) {
	// prepare sorted list of all records from the store
	TIndex::TQmGixItemV AllResIdV;
	const TWPt<TStore>& Store = RecSet->GetStore();
	PStoreIter Iter = Store->GetIter();
	while (Iter->Next()) {
		AllResIdV.Add(TIndex::TQmGixItem(Iter->GetRecId(), 1));
	}
	if (!AllResIdV.IsSorted()) { AllResIdV.Sort(); }
	// remove retrieved items
	TIndex::TQmGixItemV ResIdFqV;
	Merger->Minus(AllResIdV, RecSet->GetRecIdFqV(), ResIdFqV);
	// return new record set
	return TRecSet::New(Store, ResIdFqV, false);
}

TPair<TBool, PRecSet> TBase::Search(const TQueryItem& QueryItem, const TIndex::PQmGixExpMerger& Merger, const TIndex::PQmGixExpMergerSmall& MergerSmall, const TQueryGixUsedType& ParentGixFlag) {
	if (QueryItem.IsLeafGix() || QueryItem.IsLeafGixSmall()) {
		// return empty, when can be handled by parent-index
		if (ParentGixFlag != qgutBoth)
			return TPair<TBool, PRecSet>(false, NULL);
		// we need to force index to execute the query =>
		//      create AND node with single child and execute it
		TQueryItem IndexQueryItem(oqitAnd, QueryItem);
		TPair<TBool, PRecSet> NotRecSet = Index->Search(this, IndexQueryItem, Merger, MergerSmall);
		PRecSet RecSet; bool NotP = false;
		NotP = NotRecSet.Val1; RecSet = NotRecSet.Val2;
		return TPair<TBool, PRecSet>(NotP, RecSet);
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
			TPair<TBool, PRecSet> NotRecSet = Search(QueryItem.GetItem(0), Merger, MergerSmall, QueryItem.GetGixFlag());
			// in case it's empty, we must go to index 
			if (NotRecSet.Val2.Empty()) { NotRecSet = Index->Search(this, QueryItem.GetItem(0), Merger, MergerSmall); }
			// in case it's negated, we must invert it
			if (NotRecSet.Val1) { NotRecSet.Val2 = Invert(NotRecSet.Val2, Merger); }
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
		const uint StoreId = QueryItem.GetStoreId();
		const TWPt<TStore> Store = GetStoreByStoreId(StoreId);
		return TPair<TBool, PRecSet>(false, Store->GetAllRecs());
	} else {
		TQueryItemType Type = QueryItem.GetType();
		// check it is a known type
		QmAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);
		const TQueryGixUsedType GixFlag = QueryItem.GetGixFlag();
		// do all subsequents and keep track if any needs handling
		TBoolV NotV; TRecSetV RecSetV; bool EmptyP = true;
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			// do subsequent search
			TPair<TBool, PRecSet> NotRecSet = Search(QueryItem.GetItem(ItemN), Merger, MergerSmall, GixFlag);
			NotV.Add(NotRecSet.Val1); RecSetV.Add(NotRecSet.Val2);
			// check if to do anything
			EmptyP = EmptyP && RecSetV.Last().Empty();
		}
		// check if there is anything to do
		if (EmptyP && ParentGixFlag != qgutBoth) {
			// nope, let the father handle this with inverted index
			return TPair<TBool, PRecSet>(false, NULL);
		} else {
			// yup, let's do it!
			if (QueryItem.IsAnd() || QueryItem.IsOr()) {
				// first gather all the subordinate items, that can be handled by index
				TQueryItemV IndexQueryItemV;
				for (int ItemN = 0; ItemN < RecSetV.Len(); ItemN++) {
					const PRecSet& RecSet = RecSetV[ItemN];
					if (RecSet.Empty()) {
						IndexQueryItemV.Add(QueryItem.GetItem(ItemN));
					}
				}
				// initialize the recset
				PRecSet RecSet; int ItemN = 0; bool NotP = false;
				if (IndexQueryItemV.Empty()) {
					// nothing to use index for here, just get the first in line
					RecSet = RecSetV[0]; ItemN++;
				} else {
					// call the index and use it to initialize
					TQueryItem IndexQueryItem(QueryItem.GetType(), IndexQueryItemV);
					TPair<TBool, PRecSet> NotRecSet = Index->Search(this, IndexQueryItem, Merger, MergerSmall);
					NotP = NotRecSet.Val1; RecSet = NotRecSet.Val2;
				}
				// prepare working vectors
				TUInt64IntKdV ResRecIdFqV = RecSet->GetRecIdFqV();
				QmAssert(ResRecIdFqV.IsSorted());
				// than handle the rest here
				if (QueryItem.IsAnd()) {
					for (; ItemN < RecSetV.Len(); ItemN++) {
						// only handle ones, that were not already by index above
						if (RecSetV[ItemN].Empty()) { continue; }
						// get the vector
						const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
						// decide for the operation based on not status
						if (!NotP && !NotV[ItemN]) {
							// life is easy, just do the intersect
							Merger->Intrs(ResRecIdFqV, RecIdFqV);
						} else if (NotP && NotV[ItemN]) {
							// all negation, do the union
							Merger->Union(ResRecIdFqV, RecIdFqV);
						} else if (NotP && !NotV[ItemN]) {
							// records from RecIdFqV should not be in the main
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = false;
						} else if (!NotP && NotV[ItemN]) {
							// records from main should not be in the RecIdFqV
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = false;
						}
					}
				} else if (QueryItem.IsOr()) {
					for (; ItemN < RecSetV.Len(); ItemN++) {
						// only handle ones, that were not already by index above
						if (RecSetV[ItemN].Empty()) { continue; }
						// get the vector
						const TUInt64IntKdV& RecIdFqV = RecSetV[ItemN]->GetRecIdFqV();
						// decide for the operation based on not status
						if (!NotP && !NotV[ItemN]) {
							// life is easy, just do the union
							Merger->Union(ResRecIdFqV, RecIdFqV);
						} else if (NotP && NotV[ItemN]) {
							// all negation, do the intersect
							Merger->Intrs(ResRecIdFqV, RecIdFqV);
						} else if (NotP && !NotV[ItemN]) {
							// records not from main or from RecIdFqV
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(ResRecIdFqV, RecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = true;
						} else if (!NotP && NotV[ItemN]) {
							// records from main or not from RecIdFqV
							TUInt64IntKdV _ResRecIdFqV;
							Merger->Minus(RecIdFqV, ResRecIdFqV, _ResRecIdFqV);
							ResRecIdFqV = _ResRecIdFqV;
							NotP = true;
						}
					}
				}
				// prepare resulting record set
				RecSet = TRecSet::New(RecSet->GetStore(), ResRecIdFqV, QueryItem.IsWgt());
				return TPair<TBool, PRecSet>(NotP, RecSet);
			} else if (QueryItem.IsNot()) {
				QmAssert(RecSetV.Len() == 1);
				return TPair<TBool, PRecSet>(!NotV[0], RecSetV[0]);
			}
		}
	}
	// we should never have come to here
	throw TQmExcept::New("Unsupported query item type");
	return TPair<TBool, PRecSet>(false, NULL);
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
	PStreamAggrBase StreamAggrBase = TStreamAggrBase::New();
	// create trigger for the aggregate base
	NewStore->AddTrigger(TStreamAggrTrigger::New(StreamAggrBase));
	// remember the aggregate base for the store
	StreamAggrBaseV[StoreId] = StreamAggrBase;
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
	AssertR(IsStoreNm(StoreNm), TStr("Unknown store name ") + StoreNm);
	return StoreH.GetDat(StoreNm);
}

PJsonVal TBase::GetStoreJson(const TWPt<TStore>& Store) {
	return Store->GetStoreJson(this);
}

const PStreamAggrBase& TBase::GetStreamAggrBase(const uint& StoreId) const {
	return StreamAggrBaseV[(int)StoreId];
}

const PStreamAggrBase& TBase::GetStreamAggrBase() const {
	return StreamAggrDefaultBase;
}

bool TBase::IsStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const {
	return GetStreamAggrBase(StoreId)->IsStreamAggr(StreamAggrNm);
}

bool TBase::IsStreamAggr(const TStr& StreamAggrNm) const {
	return StreamAggrDefaultBase->IsStreamAggr(StreamAggrNm);
}

const PStreamAggr& TBase::GetStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const {
	return GetStreamAggrBase(StoreId)->GetStreamAggr(StreamAggrNm);
}

const PStreamAggr& TBase::GetStreamAggr(const TStr& StoreNm, const TStr& StreamAggrNm) const {
    if (StoreNm.Empty()) {
        return GetStreamAggr(StreamAggrNm);
    } else {
        TWPt<TStore> Store = GetStoreByStoreNm(StoreNm);
        PStreamAggrBase SABase = GetStreamAggrBase(Store->GetStoreId());
        return SABase->GetStreamAggr(StreamAggrNm);
    }
}

const PStreamAggr& TBase::GetStreamAggr(const TStr& StreamAggrNm) const {
	return StreamAggrDefaultBase->GetStreamAggr(StreamAggrNm);
}

void TBase::AddStreamAggr(const uint& StoreId, const PStreamAggr& StreamAggr) {
	// add new aggregate to the stream aggregate base
	GetStreamAggrBase(StoreId)->AddStreamAggr(StreamAggr);
}

void TBase::AddStreamAggr(const TUIntV& StoreIdV, const PStreamAggr& StreamAggr) {
	for (int StoreN = 0; StoreN < StoreIdV.Len(); StoreN++) {
		AddStreamAggr(StoreIdV[StoreN], StreamAggr);
	}
}

void TBase::AddStreamAggr(const TStr& StoreNm, const PStreamAggr& StreamAggr) {
	AddStreamAggr(GetStoreByStoreNm(StoreNm)->GetStoreId(), StreamAggr);
}

void TBase::AddStreamAggr(const TStrV& StoreNmV, const PStreamAggr& StreamAggr) {
	for (int StoreN = 0; StoreN < StoreNmV.Len(); StoreN++) {
		AddStreamAggr(GetStoreByStoreNm(StoreNmV[StoreN])->GetStoreId(), StreamAggr);
	}
}

void TBase::AddStreamAggr(const PStreamAggr& StreamAggr) {
	StreamAggrDefaultBase->AddStreamAggr(StreamAggr);
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

int TBase::NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	return NewIndexKey(Store, KeyNm, NewIndexWordVoc(Type), Type, SortType);
}

int TBase::NewIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& WordVocId,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	QmAssertR(!IndexVoc->IsKeyNm(Store->GetStoreId(), KeyNm),
		"Key " + Store->GetStoreNm() + "." + KeyNm + " already exists!");
	const int KeyId = IndexVoc->AddKey(Store->GetStoreId(), KeyNm, WordVocId, Type, SortType);
	return KeyId;
}

int TBase::NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, KeyNm, FieldId, NewIndexWordVoc(Type), Type, SortType);
}

int TBase::NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, Store->GetFieldNm(FieldId),
		FieldId, NewIndexWordVoc(Type), Type, SortType);
}

int TBase::NewFieldIndexKey(const TWPt<TStore>& Store, const int& FieldId, const int& WordVocId,
	const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	return NewFieldIndexKey(Store, Store->GetFieldNm(FieldId), FieldId, WordVocId, Type, SortType);
}

int TBase::NewFieldIndexKey(const TWPt<TStore>& Store, const TStr& KeyNm, const int& FieldId,
	const int& WordVocId, const TIndexKeyType& Type, const TIndexKeySortType& SortType) {

	QmAssertR(!IndexVoc->IsKeyNm(Store->GetStoreId(), KeyNm),
		"Key " + Store->GetStoreNm() + "." + KeyNm + " already exists!");
	const int KeyId = IndexVoc->AddKey(Store->GetStoreId(), KeyNm, WordVocId, Type, SortType);
	IndexVoc->AddKeyField(KeyId, Store->GetStoreId(), FieldId);
	Store->AddFieldKey(FieldId, KeyId);
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
	TIndex::PQmGixExpMerger Merger = Index->GetDefMerger();
	TIndex::PQmGixExpMergerSmall MergerSmall = Index->GetDefMergerSmall();
	TPair<TBool, PRecSet> NotRecSet = Search(Query->GetQueryItem(), Merger, MergerSmall, Query->GetQueryItem().GetGixFlag());
	// when empty, then query can be completly covered by index
	if (NotRecSet.Val2.Empty()) {
		NotRecSet = Index->Search(this, Query->GetQueryItem(), Merger, MergerSmall);
	}
	PRecSet RecSet = NotRecSet.Val2;
	// if result should be negated, do the invert
	if (NotRecSet.Val1) { RecSet = Invert(NotRecSet.Val2, Merger); }
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
				const uint64 NewRecId = OldToNewIdH.GetDat(OldRecId);	// map old rec ids to new rec ids
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
					const uint64 NewJoinRecId = JoinOldToNewIdH.GetDat(OldJoinRecId);		// if some articles or other data was deleted from the index then old and new ids could be different. in most cases it should be the same
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
	int slice = WndInMsec / (GetStores() + 1);
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
		saved = 0; // how many saved in this loop
		int xsaved = 0; // temp variable
		for (int i = 0; i < xstores.Len(); i++) {
			if (!xstores[i].Val2)
				continue; // this store had no dirty data in previous loop
			xsaved = xstores[i].Val1->PartialFlush(slice);
			if (xsaved == 0) {
				xstores[i].Val2 = false; // ok, this store is clean now
			}
			saved += xsaved;
			TQm::TEnv::Debug->OnStatusFmt("Partial flush:     store %s = %d", xstores[i].Val1->GetStoreNm().CStr(), xsaved);
		}
		if (xindex) { // save index
			xsaved = Index->PartialFlush(slice);
			xindex = (xsaved > 0);
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

////////////////////////////////////////////////////////////////////////////////

/// Export TBlobBsStats object to JSON
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

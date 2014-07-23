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

#include "qminer_core.h"
#include "qminer_ftr.h"
#include "qminer_aggr.h"
#include "qminer_op.h"

// external dependecies
#include <sphere.h>

namespace TQm {

///////////////////////////////
// QMiner Environment
TIntTr TEnv::Version = TIntTr(0, 5, 1);

bool TEnv::InitP = false;
TStr TEnv::QMinerFPath;
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
	if ((('A'<=FirstCh)&&(FirstCh<='Z')) || (('a'<=FirstCh)&&(FirstCh<='z')) || ValidFirstCh.IsChIn(FirstCh)) {
		// all fine
	} else {
		throw TQmExcept::New("Name: invalid first character in '" + NmStr + "'");
	}
	// check rest
	for (int ChN = 1; ChN < NmStr.Len(); ChN++) {
		const char Ch = NmStr[ChN];
		if ((('A'<=Ch)&&(Ch<='Z')) || (('a'<=Ch)&&(Ch<='z')) || (('0'<=Ch)&&(Ch<='9')) || ValidCh.IsChIn(Ch)) {
			// all fine
		} else {
			throw TQmExcept::New(TStr::Fmt("Name: invalid %d character in '%s'", ChN, NmStr.CStr()));
		}
	}
}

///////////////////////////////
// QMiner-Join-Description
TJoinDesc::TJoinDesc(const TStr& _JoinNm, const uint& _JoinStoreId, 
		const uint& StoreId, const TWPt<TIndexVoc>& IndexVoc): 
            JoinId(-1), InverseJoinId(-1) { 

	// remember join parameters
	JoinStoreId = _JoinStoreId;
	JoinNm = _JoinNm;
	JoinType = osjtIndex;
	JoinRecFieldId = -1;
	JoinFqFieldId = -1;
	// create an internal join key in the index
	TStr JoinKeyNm = "Join" + JoinNm;
	JoinKeyId = IndexVoc->AddInternalKey(StoreId, JoinKeyNm, JoinNm);
	// assert the name is valid
	TValidNm::AssertValidNm(JoinNm);
}

TJoinDesc::TJoinDesc(TSIn& SIn): JoinId(SIn), JoinNm(SIn), JoinStoreId(SIn),
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
TJoinSeq::TJoinSeq(const TWPt<TStore>& StartStore): StartStoreId(StartStore->GetStoreId()) { }

TJoinSeq::TJoinSeq(const uint& _StartStoreId, const int& JoinId, const int& Sample):
	StartStoreId(_StartStoreId) { JoinIdV.Add(TIntPr(JoinId, Sample)); }

TJoinSeq::TJoinSeq(const uint& _StartStoreId, const TIntPrV& _JoinIdV):
	StartStoreId(_StartStoreId), JoinIdV(_JoinIdV) { }

TJoinSeq::TJoinSeq(const TWPt<TBase>& Base, const uint& _StartStoreId,
		const PJsonVal& JoinSeqVal): StartStoreId(_StartStoreId) { 

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
        const bool& PrimaryP, const bool& NullP, const bool& InternalP): 
            FieldId(-1), FieldNm(_FieldNm), FieldType(_FieldType) { 
			
	TValidNm::AssertValidNm(FieldNm);
	// set flags
    if (PrimaryP) {  Flags.Val |= ofdfPrimary; }
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
        case oftUndef : return "undefined";
        case oftInt : return "int"; 
        case oftIntV : return "int_v"; 
        case oftUInt64 : return "uint64";
        case oftStr : return "string"; 
        case oftStrV : return "string_v"; 
        case oftBool : return "bool"; 
        case oftFlt : return "float"; 
        case oftFltPr : return "float_pair"; 
        case oftFltV : return "float_v"; 
        case oftTm : return "datetime"; 
        case oftNumSpV : return "num_sp_v"; 
        case oftBowSpV : return "bow_sp_v"; 
    }
	Fail; return "";
}

///////////////////////////////
// QMiner-Store-Iterators
TStoreIterVec::TStoreIterVec(): FirstP(false), RecId(0), RecIds(0) { } 

TStoreIterVec::TStoreIterVec(const uint64& _RecIds): 
	FirstP(true), RecId(0), RecIds(_RecIds) { }

TStoreIterVec::TStoreIterVec(const uint64& MinId, const uint64& MaxId): 
	FirstP(true), RecId(MinId), RecIds(MaxId + 1) { }

bool TStoreIterVec::Next() {
	if (FirstP) { 
		FirstP = false; 
	} else { 
		RecId++; 
	}
	return (RecId < RecIds); 
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

TStore::TStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm): 
    Base(_Base), Index(_Base->GetIndex()), StoreId(_StoreId), StoreNm(_StoreNm) {   
        TValidNm::AssertValidNm(StoreNm); }
    
TStore::TStore(const TWPt<TBase>& _Base, TSIn& SIn): 
    Base(_Base), Index(_Base->GetIndex()) { LoadStore(SIn); }

TStore::TStore(const TWPt<TBase>& _Base, const TStr& FNm):
    Base(_Base), Index(_Base->GetIndex()) { TFIn FIn(FNm); LoadStore(FIn); }

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
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnAdd(GetRec(RecId));
    }
}

void TStore::OnUpdate(const uint64& RecId) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnUpdate(GetRec(RecId));
    }
}

void TStore::OnDelete(const uint64& RecId) {
    for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
        TriggerV[TriggerN]->OnDelete(GetRec(RecId));
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
            if (JoinDesc.IsFieldJoin())  {
                // get join record JSon object
                PJsonVal JoinRecVal = RecVal->GetObjKey(JoinDesc.GetJoinNm());
                // insert join record
                const uint64 JoinRecId = JoinStore->AddRec(JoinRecVal);
                // get join weight (useful only for inverse index joins)
                int JoinFq = JoinRecVal->GetObjInt("$fq", 1);
                // make sure weight is from valid range
                if (JoinFq < 1) {
                    ErrorLog("[TStoreImpl::AddRec] Join frequency must be positive");
                    JoinFq = 1;
                }
                // mark the join
                AddJoin(JoinDesc.GetJoinId(), RecId, JoinRecId, JoinFq);
            } else if (JoinDesc.IsIndexJoin()) {
                // index joins must be in an array
                PJsonVal JoinArrVal = RecVal->GetObjKey(JoinDesc.GetJoinNm());
                if (!JoinArrVal->IsArr()) {
                    ErrorLog("[TStoreImpl::AddRec] Expected array for join " + JoinDesc.GetJoinNm());
                    continue;
                }
                // add join records and remember their record ids and weights
                TUInt64IntKdV JoinRecIdFqVH(JoinArrVal->GetArrVals(), 0);
                for (int JoinRecN = 0 ; JoinRecN < JoinArrVal->GetArrVals(); JoinRecN++) {
                    // get join record JSon
                    PJsonVal JoinRecVal = JoinArrVal->GetArrVal(JoinRecN);
                    // insert join record
                    uint64 JoinRecId = JoinStore->AddRec(JoinRecVal);
                    // get join weight, default is 1
                    int JoinFq = JoinRecVal->GetObjInt("$fq", 1);
                    // make sure weight is from valid range
                    if (JoinFq < 1) {
                        ErrorLog("[TStoreImpl::AddRec] Join frequency must be positive");
                        JoinFq = 1;
                    }
                    // index the join
                    AddJoin(JoinDesc.GetJoinId(), RecId, JoinRecId, JoinFq);
                }
            }
        } else {
            // we don't have join specified, set field joins to point to nothing (TUInt64::Mx)
            if (JoinDesc.IsFieldJoin()) {
                SetFieldUInt64(RecId, JoinDesc.GetJoinRecFieldId(), TUInt64::Mx);
                SetFieldInt(RecId, JoinDesc.GetJoinFqFieldId(), 0);
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

TIntV TStore::GetFieldIdV(const TFieldType& Type){
	TIntV FieldIdV;
	for (int i = 0; i < FieldDescV.Len(); i++){
		if (FieldDescV[i].GetFieldType() == Type){
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
    TUInt64IntKdV RecIdFqV((int)GetRecs(), 0);
	PStoreIter Iter = GetIter();
	while (Iter->Next()) {
		RecIdFqV.Add(TUInt64IntKd(Iter->GetRecId(), 1));
	}
	return TRecSet::New(TWPt<TStore>(this), RecIdFqV, false);
}

PRecSet TStore::GetRndRecs(const uint64& SampleSize) {
	return GetAllRecs()->GetSampleRecSet((int)SampleSize, false);
}

void TStore::AddJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // different handling for field and index joins
    if (JoinDesc.IsIndexJoin()) {
        Index->IndexJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    } else if (JoinDesc.IsFieldJoin()) {
        // TODO: check if we already have an existing join here
        // and figure out if it needs deleting first (probably yes)
        SetFieldUInt64(RecId, JoinDesc.GetJoinRecFieldId(), JoinRecId);
        SetFieldInt(RecId, JoinDesc.GetJoinFqFieldId(), JoinFq);
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
            JoinStore->SetFieldUInt64(JoinRecId, InverseJoinDesc.GetJoinRecFieldId(), RecId);
            JoinStore->SetFieldInt(JoinRecId, InverseJoinDesc.GetJoinFqFieldId(), JoinFq);
        }
    }
}

void TStore::DelJoin(const int& JoinId, const uint64& RecId, const uint64 JoinRecId, const int& JoinFq) {
    const TJoinDesc& JoinDesc = GetJoinDesc(JoinId);
    // different handling for field and index joins
    if (JoinDesc.IsIndexJoin()) {
        Index->DeleteJoin(this, JoinId, RecId, JoinRecId, JoinFq);
    } else if (JoinDesc.IsFieldJoin()) {
        SetFieldUInt64(RecId, JoinDesc.GetJoinRecFieldId(), TUInt64::Mx);
        SetFieldInt(RecId, JoinDesc.GetJoinFqFieldId(), 0);
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
        }else {
            JoinStore->SetFieldUInt64(JoinRecId, InverseJoinDesc.GetJoinRecFieldId(), TUInt64::Mx);
            JoinStore->SetFieldInt(JoinRecId, InverseJoinDesc.GetJoinFqFieldId(), 0);
        }
    }
}

int TStore::GetFieldInt(const uint64& RecId, const int& FieldId) const { 
	throw FieldError(FieldId, "Int");
}

void TStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const { 
	throw FieldError(FieldId, "IntV"); 
}

uint64 TStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const { 
	throw FieldError(FieldId, "UInt64");
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
	return Tm.IsDef() ? TTm::GetMSecsFromTm(Tm) : TUInt64::Mx.Val;
}

void TStore::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const { 
	throw FieldError(FieldId, "NumSpV"); 
}

void TStore::GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const { 
	throw FieldError(FieldId, "BowSpV"); 
}

bool TStore::IsFieldNmNull(const uint64& RecId, const TStr& FieldNm) const { 
	return IsFieldNull(RecId, GetFieldId(FieldNm)); 
}

int TStore::GetFieldNmInt(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldInt(RecId, GetFieldId(FieldNm)); 
}

void TStore::GetFieldNmIntV(const uint64& RecId, const TStr& FieldNm, TIntV& IntV) const { 
	return GetFieldIntV(RecId, GetFieldId(FieldNm), IntV); 
}

uint64 TStore::GetFieldNmUInt64(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldUInt64(RecId, GetFieldId(FieldNm)); 
}

TStr TStore::GetFieldNmStr(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldStr(RecId, GetFieldId(FieldNm)); 
}

void TStore::GetFieldNmStrV(const uint64& RecId, const TStr& FieldNm, TStrV& StrV) const { 
	return GetFieldStrV(RecId, GetFieldId(FieldNm), StrV); 
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
	return GetFieldFltV(RecId, GetFieldId(FieldNm), FltV); 
}

void TStore::GetFieldNmTm(const uint64& RecId, const TStr& FieldNm, TTm& Tm) const { 
	return GetFieldTm(RecId, GetFieldId(FieldNm), Tm); 
}

uint64 TStore::GetFieldNmTmMSecs(const uint64& RecId, const TStr& FieldNm) const { 
	return GetFieldTmMSecs(RecId, GetFieldId(FieldNm)); 
}

void TStore::GetFieldNmNumSpV(const uint64& RecId, const TStr& FieldNm, TIntFltKdV& SpV) const { 
	return GetFieldNumSpV(RecId, GetFieldId(FieldNm), SpV); 
}

void TStore::GetFieldNmBowSpV(const uint64& RecId, const TStr& FieldNm, PBowSpV& SpV) const { 
	return GetFieldBowSpV(RecId, GetFieldId(FieldNm), SpV); 
}

void TStore::SetFieldNull(const uint64& RecId, const int& FieldId) {
	throw FieldError(FieldId, "SetNull");
}

void TStore::SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int) {
	throw FieldError(FieldId, "Int");
}

void TStore::SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV) {
	throw FieldError(FieldId, "IntV");
}

void TStore::SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
	throw FieldError(FieldId, "UInt64");
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

PJsonVal TStore::GetFieldJson(const uint64& RecId, const int& FieldId) const {
    const TFieldDesc& Desc = GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		return TJsonVal::NewNum((double)GetFieldInt(RecId, FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(RecId, FieldId, FieldIntV); 
		return TJsonVal::NewArr(FieldIntV);
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
    } else if (Desc.IsFltPr()) {
        return TJsonVal::NewArr(GetFieldFltPr(RecId, FieldId));
	} else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(RecId, FieldId, FieldFltV); 
		return TJsonVal::NewArr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);		
		if (FieldTm.IsDef()) { return TJsonVal::NewStr(FieldTm.GetWebLogDateTimeStr(true, "T", false)); } else { return TJsonVal::NewNull();	}
	} else if (Desc.IsNumSpV()) {
		TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(RecId, FieldId, FieldIntFltKdV); 
		return TJsonVal::NewStr(TStrUtil::GetStr(FieldIntFltKdV));
	} else if (Desc.IsBowSpV()) {
		return TJsonVal::NewStr("[PBowSpV]"); //TODO
    }
	throw FieldError(FieldId, "GetFieldJson");
}

TStr TStore::GetFieldText(const uint64& RecId, const int& FieldId) const {
    const TFieldDesc& Desc = GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		return TInt::GetStr(GetFieldInt(RecId, FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(RecId, FieldId, FieldIntV); 
		return TStrUtil::GetStr(FieldIntV);
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
    } else if (Desc.IsFltPr()) {
        const TFltPr FieldFltPr = GetFieldFltPr(RecId, FieldId);
        return TStr::Fmt("(%g, %g)", FieldFltPr.Val1.Val, FieldFltPr.Val2.Val);
	} else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(RecId, FieldId, FieldFltV); 
		return TStrUtil::GetStr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(RecId, FieldId, FieldTm);		
		if (FieldTm.IsDef()) { return FieldTm.GetWebLogDateTimeStr(); } else { return "--";	}
	} else if (Desc.IsNumSpV()) {
		TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(RecId, FieldId, FieldIntFltKdV); 
		return TStrUtil::GetStr(FieldIntFltKdV);
	} else if (Desc.IsBowSpV()) {
		return "[PBowSpV]"; //TODO
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
		if (Key.IsSort()) {	KeyVal->AddToObj("sortBy", TStr(Key.IsSortById() ? "word-id" : "word-str")); }
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
        return (uint64)RecVal->GetObjInt("$id"); 
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
		    } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
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

void TStore::PrintAll(const TWPt<TBase>& Base, TSOut& SOut) const {
	// print headers
	PrintTypes(Base, SOut);
	// print records
	SOut.PutStrLn("Records:");
    const int Fields = GetFields();
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
		    } else if (Desc.IsUInt64()) {
                const uint64 FieldInt = GetFieldUInt64(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %I64u", Desc.GetFieldNm().CStr(), FieldInt);
            } else if (Desc.IsFlt()) {
                const double FieldFlt = GetFieldFlt(RecId, FieldId);
                SOut.PutStrFmtLn("  %s: %g", Desc.GetFieldNm().CStr(), FieldFlt);
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

void TStore::PrintAll(const TWPt<TBase>& Base, const TStr& FNm) const {
	TFOut FOut(FNm); PrintAll(Base, FOut);
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
			KeyChA += KeyChA.Empty() ? ", IK:" :  ";";
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
		if (Key.IsSort()) { SOut.PutStr(Key.IsSortById() ? "SortByWordId" : "SortByWord"); }
		if (Key.IsWordVoc()) { SOut.PutStrFmt(" WordVoc(#values=%d)", IndexVoc->GetWords(KeyId)); }
		SOut.PutStrLn("]");
	}
}

void TStore::PrintTypes(const TWPt<TBase>& Base, const  TStr& FNm) const {
	TFOut FOut(FNm); PrintTypes(Base, FOut);
}

///////////////////////////////
// QMiner-Record
PExcept TRec::FieldError(const int& FieldId, const TStr& TypeStr) const { 
	return TQmExcept::New(TStr::Fmt("Wrong field-type combination requested: [%d:%s]!", FieldId, TypeStr.CStr())); 
}

TRec::TRec(const TWPt<TStore>& _Store, const PJsonVal& JsonVal): 
        Store(_Store), ByRefP(false), RecId(TUInt64::Mx), RecValOut(RecVal) {
    
    for(int FieldId = 0; FieldId < Store->GetFields(); FieldId++) {
        const TFieldDesc& FieldDesc = Store->GetFieldDesc(FieldId);
        // check if field exists in the JSON
        TStr FieldName = FieldDesc.GetFieldNm();
        if (!JsonVal->IsObjKey(FieldName)) { continue; }
        // parse the field from JSon
        PJsonVal FieldVal = JsonVal->GetObjKey(FieldName);
        switch (FieldDesc.GetFieldType()) {
            case oftInt:
                QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
                SetFieldInt(FieldId, FieldVal->GetInt());
                break;
            case oftIntV: {
                QmAssertR(FieldVal->IsArr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not array.");
                TIntV IntV; FieldVal->GetArrIntV(IntV);
                SetFieldIntV(FieldId, IntV);
                break;
            }
            case oftUInt64:
                QmAssertR(FieldVal->IsNum(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not numeric.");
                SetFieldUInt64(FieldId, (uint64)FieldVal->GetInt());
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
                QmAssertR(FieldVal->IsStr(), "Provided JSon data field " + FieldDesc.GetFieldNm() + " is not string that represents DateTime.");
                TStr TmStr = FieldVal->GetStr();
                TTm Tm = TTm::GetTmFromWebLogDateTimeStr(TmStr, '-', ':', '.', 'T');
                SetFieldTm(FieldId, Tm);
                break;
            }
            default:
                throw TQmExcept::New("Unsupported JSon data type for function - " + FieldDesc.GetFieldTypeStr());
        }
    }
}

TRec::TRec(const TRec& Rec): Store(Rec.Store), ByRefP(Rec.ByRefP), RecId(Rec.RecId),
    FieldIdPosH(Rec.FieldIdPosH), JoinIdPosH(Rec.JoinIdPosH), RecVal(Rec.RecVal), 
    RecValOut(RecVal) { }  

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

PJsonVal TRec::GetFieldJson(const int& FieldId) const {
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		return TJsonVal::NewNum((double)GetFieldInt(FieldId));
    } else if (Desc.IsIntV()) {
        TIntV FieldIntV; GetFieldIntV(FieldId, FieldIntV); 
		return TJsonVal::NewArr(FieldIntV);
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
    } else if (Desc.IsFltPr()) {
        return TJsonVal::NewArr(GetFieldFltPr(FieldId));
	} else if (Desc.IsFltV()) {
        TFltV FieldFltV; GetFieldFltV(FieldId, FieldFltV); 
		return TJsonVal::NewArr(FieldFltV);
    } else if (Desc.IsTm()) {
        TTm FieldTm; GetFieldTm(FieldId, FieldTm);		
		if (FieldTm.IsDef()) { return TJsonVal::NewStr(FieldTm.GetWebLogDateTimeStr(true, "T", true)); } else { return TJsonVal::NewNull();	}
	} else if (Desc.IsNumSpV()) {
		TIntFltKdV FieldIntFltKdV; GetFieldNumSpV(FieldId, FieldIntFltKdV); 
		return TJsonVal::NewStr(TStrUtil::GetStr(FieldIntFltKdV));
	} else if (Desc.IsBowSpV()) {
		return TJsonVal::NewStr("[PBowSpV]"); //TODO
    }
	throw FieldError(FieldId, "GetFieldJson");
}

TStr TRec::GetFieldText(const int& FieldId) const {
    const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		return TInt::GetStr(GetFieldInt(FieldId));
    } else if (Desc.IsIntV()) {
		TIntV IntV; GetFieldIntV(FieldId, IntV);
		return TStrUtil::GetStr(IntV);
    } else if (Desc.IsBool()) {
        return GetFieldBool(FieldId) ? "Yes" : "No";
    } else if (Desc.IsUInt64()) {
		return TUInt64::GetStr(GetFieldUInt64(FieldId));
	} else if (Desc.IsStr()) {
        return GetFieldStr(FieldId);
    } else if (Desc.IsStrV()) {
        TStrV FieldStrV; GetFieldStrV(FieldId, FieldStrV);
        return TStr::GetStr(FieldStrV, ", ");
    } else if (Desc.IsFlt()) {
		return TFlt::GetStr(GetFieldFlt(FieldId));
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

void TRec::SetFieldInt(const int& FieldId, const int& Int) { 
    if (IsByRef()) {
        Store->SetFieldInt(RecId, FieldId, Int);
    } else {
        FieldIdPosH.AddDat(FieldId, RecVal.Len());
        TInt(Int).Save(RecValOut);
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

void TRec::AddJoin(const int& JoinId, const PRecSet& JoinRecSet) { 
    JoinIdPosH.AddDat(JoinId, RecVal.Len());
    JoinRecSet->GetRecIdFqV().Save(RecValOut);
}

PRecSet TRec::ToRecSet() const {
    QmAssertR(IsByRef(), "Cannot transform record passed by value to a set!");
	return IsDef() ? TRecSet::New(Store, RecId) : TRecSet::New(Store);
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const int& JoinId) const {
    // get join info
    AssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
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
        return TRecSet::New(JoinDesc.GetJoinStore(Base), JoinRecIdFqV, true);        
    } else if (JoinDesc.IsFieldJoin()) {
        // do join using store field
        const int JoinRecFieldId = JoinDesc.GetJoinRecFieldId();
        const uint64 JoinRecId = IsFieldNull(JoinRecFieldId) ? TUInt64::Mx.Val :  GetFieldUInt64(JoinRecFieldId);
        // get join weight
        const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
        const int JoinRecFq = IsFieldNull(JoinRecFieldId) ? 0 : GetFieldInt(JoinFqFieldId);
        // return record set
        if (JoinRecId != TUInt64::Mx) {
            // return record
            return TRecSet::New(JoinDesc.GetJoinStore(Base), JoinRecId, JoinRecFq);
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
		return TRecSet::New();
	}
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const {
	PRecSet RecSet = DoJoin(Base, JoinIdV[0].Val1);
	for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		RecSet = RecSet->DoJoin(Base, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2, true);
	}
	return RecSet;
}

PRecSet TRec::DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const {
	return DoJoin(Base, JoinSeq.GetJoinIdV());
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const int& JoinId) const {
	PRecSet JoinRecSet = DoJoin(Base, JoinId);
	return TRec(JoinRecSet->GetStore(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TStr& JoinNm) const {
	PRecSet JoinRecSet = DoJoin(Base, JoinNm);
	return TRec(JoinRecSet->GetStore(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV) const {
	PRecSet JoinRecSet = DoJoin(Base, JoinIdV);
	return TRec(JoinRecSet->GetStore(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
}

TRec TRec::DoSingleJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq) const {
	PRecSet JoinRecSet = DoJoin(Base, JoinSeq);
	return TRec(JoinRecSet->GetStore(), 
		JoinRecSet->Empty() ? (uint64)TUInt64::Mx : JoinRecSet->GetRecId(0));
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
		RecVal->AddToObj("$id", (int)RecId);
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
// QMiner-ResultSet
void TRecSet::GetSampleRecIdV(const int& SampleSize, 
		const bool& SortedP, TUInt64IntKdV& SampleRecIdFqV) const {

	if (SampleSize == -1) {
		SampleRecIdFqV = RecIdFqV;
	} else if (SortedP) { 
		const int SampleRecs = TInt::GetMn(SampleSize, GetRecs());
		SampleRecIdFqV.Gen(SampleRecs, 0);
		for (int RecN = 0; RecN < SampleRecs; RecN++) {
			SampleRecIdFqV.Add(RecIdFqV[RecN]);
		}
	} else {
		for (int RecN = 0; RecN < GetRecs(); RecN++) {
			SampleRecIdFqV.Add(RecIdFqV[RecN]); 
		}
		if (SampleSize < GetRecs()) { 
			TRnd Rnd(1); SampleRecIdFqV.Shuffle(Rnd); 
			SampleRecIdFqV.Trunc(SampleSize); 
		}
	}
}

void TRecSet::LimitToSampleRecIdV(const TUInt64IntKdV& SampleRecIdFqV) {
	RecIdFqV = SampleRecIdFqV;
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const uint64& RecId, const int& Wgt): 
        Store(_Store), WgtP(Wgt > 1) { 
    
    RecIdFqV.Gen(1, 0); RecIdFqV.Add(TUInt64IntKd(RecId, Wgt));
}

TRecSet::TRecSet(const TWPt<TStore>& _Store, const TUInt64V& RecIdV): Store(_Store), WgtP(false) { 
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

PRecSet TRecSet::New() { 
	return new TRecSet(); 
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

PRecSet TRecSet::New(const TWPt<TStore>& Store, const TUInt64IntKdV& RecIdFqV, const bool& WgtP) {
	return new TRecSet(Store, RecIdFqV, WgtP); 
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
	TInt::SetRndSeed(1); // HACK to be consistent
	RecIdFqV.SortCmp(TCmpKeyDatByDat<TUInt64, TInt>(Asc));
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

void TRecSet::FilterByFieldInt(const int& FieldId, const int& MinVal, const int& MaxVal) {
    // get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsInt(), "Wrong field type, integer expected");
	// apply the filter
    FilterBy(TRecFilterByFieldInt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldFlt(const int& FieldId, const double& MinVal, const double& MaxVal) {
    // get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsFlt(), "Wrong field type, numeric expected");
	// apply the filter
    FilterBy(TRecFilterByFieldFlt(Store, FieldId, MinVal, MaxVal));
}

void TRecSet::FilterByFieldStr(const int& FieldId, const TStr& FldVal) {
    // get store and field type
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    QmAssertR(Desc.IsStr(), "Wrong field type, string expected");
	// apply the filter
    FilterBy(TRecFilterByFieldStr(Store, FieldId, FldVal));
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

void TRecSet::FilterByFieldTm(	const int& FieldId, const TTm& MinVal, const TTm& MaxVal) {
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
    return TRecSet::New(Store, RecIdFqV, WgtP);
}

PRecSet TRecSet::GetSampleRecSet(const int& SampleSize, const bool& SortedP) const {
	TUInt64IntKdV SampleRecIdFqV;
	GetSampleRecIdV(SampleSize, SortedP, SampleRecIdFqV);
	return TRecSet::New(Store, SampleRecIdFqV, WgtP);
}

PRecSet TRecSet::GetLimit(const int& Limit, const int& Offset) const {
	if (Offset >= GetRecs()) {
		// offset past number of recordes, return empty
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
		return TRecSet::New(Store, LimitRecIdFqV, WgtP);
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
	for (int RsIdx = 0; RecSetV.Len(); RsIdx++){
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
	return TRecSet::New(GetStore(), ResultRecIdFqV, false);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const int& JoinId, 
        const int& SampleSize, const bool& SortedP) const {

	// get join info
	AssertR(Store->IsJoinId(JoinId), "Wrong Join ID");
	const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);	
	// prepare joined record sample
	TUInt64IntKdV SampleRecIdKdV;
	GetSampleRecIdV(SampleSize, SortedP, SampleRecIdKdV);
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
        const int JoinFqFieldId = JoinDesc.GetJoinFqFieldId();
		for (int RecN = 0; RecN < SampleRecs; RecN++) {
			const uint64 RecId = SampleRecIdKdV[RecN].Key;
			const uint64 JoinRecId = Store->GetFieldUInt64(RecId, JoinRecFieldId);
            const int JoinRecFq = Store->GetFieldInt(RecId, JoinFqFieldId);
			if (JoinRecId != TUInt64::Mx) { JoinRecIdFqH.AddDat(JoinRecId) += JoinRecFq; }
		}
		JoinRecIdFqH.GetKeyDatKdV(JoinRecIdFqV);
	} else {
		// unknown join type
		throw TQmExcept::New("Unsupported join type for join " + JoinDesc.GetJoinNm() + "!");
	}
	// create new RecSet
	return TRecSet::New(JoinDesc.GetJoinStore(Base), JoinRecIdFqV, true);
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TStr& JoinNm, 
		const int& SampleSize, const bool& SortedP) const {

	if (Store->IsJoinNm(JoinNm)) {
		return DoJoin(Base, Store->GetJoinId(JoinNm), SampleSize, SortedP);
	} else {
		return TRecSet::New();
	}
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TIntPrV& JoinIdV, const bool& SortedP) const {
	PRecSet RecSet = DoJoin(Base, JoinIdV[0].Val1, JoinIdV[0].Val2, SortedP);
	for (int JoinIdN = 1; JoinIdN < JoinIdV.Len(); JoinIdN++) {
		RecSet = RecSet->DoJoin(Base, JoinIdV[JoinIdN].Val1, JoinIdV[JoinIdN].Val2, true);
	}
	return RecSet;
}

PRecSet TRecSet::DoJoin(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const bool& SortedP) const {
	return DoJoin(Base, JoinSeq.GetJoinIdV(), SortedP);
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
TIndexKey::TIndexKey(const uint& _StoreId, const TStr& _KeyNm, const int& _WordVocId, 
		const TIndexKeyType& _Type, const TIndexKeySortType& _SortType): StoreId(_StoreId), 
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
	
TIndexKey::TIndexKey(TSIn& SIn): StoreId(SIn), KeyId(SIn), 
    KeyNm(SIn), WordVocId(SIn), 
    TypeFlags(LoadEnum<TIndexKeyType>(SIn)),
    SortType(LoadEnum<TIndexKeySortType>(SIn)),
    FieldIdV(SIn), JoinNm(SIn), Tokenizer(SIn) { }

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

int TIndexVoc::AddInternalKey(const uint& StoreId, const TStr& KeyNm, const TStr& JoinNm) {
	const int KeyId = KeyH.AddKey(TUIntStrPr(StoreId, KeyNm));
	KeyH[KeyId] = TIndexKey(StoreId, KeyNm, JoinNm);
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
	for(int TokN = 0; TokN < TokV.Len(); TokN++) {
		const TStr& Tok = TokV[TokN];
		if(IsWordStr(KeyId, Tok)) {
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
	for(int TokN = 0; TokN < TokV.Len(); TokN++) {
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
	for(int TokN = 0; TokN < TokV.Len(); TokN++) {
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
				if (WordN != 0) { FOut.PutStr(", "); ChsPerLn += 2;}
				// check if need to break the line
				const TStr& WordStr = WordStrV[WordN];
				if (ChsPerLn + WordStr.Len() > MxChsPerLn) {
					FOut.PutStr("\n  "); ChsPerLn = 2; }
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
			throw TQmExcept::New("Wrong sort type for text Key!"); }
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
		QmAssertR(JoinVal->GetObjKey("$sample")->IsNum(), "Query: $sample expects number as value"); }
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

void TQueryItem::ParseKeys(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
		const PJsonVal& JsonVal, const bool& IgnoreOrP) {

	// go over other keys and create their corresponding items
	for (int KeyN = 0; KeyN < JsonVal->GetObjKeys(); KeyN++) {
		// read the key
		TStr KeyNm; PJsonVal KeyVal;
		JsonVal->GetObjKeyVal(KeyN, KeyNm, KeyVal);
		// check type
		if (KeyNm.IsPrefix("$")) {
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
			} else if (KeyNm == "$record") {
                InfoLog("Warning: $record in query language is deprecated. Used $id or $name instead to refer to record.");
				uint64 RecId = TUInt64::Mx;
				if (KeyVal->IsStr()) {
					TStr RecNm = KeyVal->GetStr();
					if (Store->IsRecNm(RecNm)) {
						RecId = Store->GetRecId(RecNm);
					}
				} else if (KeyVal->IsNum()) {
					const uint64 _RecId = (uint64)TFlt::Round(KeyVal->GetNum());
					if (Store->IsRecId(RecId)) { 
						RecId = _RecId; 
					}
				} else {
					throw TQmExcept::New("Query: unsupported $record value");
				}
				ItemV.Add(TQueryItem(Store, RecId));
			} else if (KeyNm == "$id") {
                QmAssertR(KeyVal->IsNum(), "Query: unsupported $id value");
                const uint64 _RecId = (uint64)KeyVal->GetInt();
                const uint64 RecId = Store->IsRecId(_RecId) ? _RecId : TUInt64::Mx.Val;
                ItemV.Add(TQueryItem(Store, RecId));
			} else if (KeyNm == "$name") {
                QmAssertR(KeyVal->IsStr(), "Query: unsupported $name value");
                TStr RecNm = KeyVal->GetStr();  
                const uint64 RecId = Store->IsRecNm(RecNm) ? Store->GetRecId(RecNm) : TUInt64::Mx.Val;
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
			// gix query
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
				if (Store.Empty()) { Store = Base->GetStoreByStoreId(OrStoreId); }
				else { QmAssertR(OrStoreId == Store->GetStoreId(), "Query: store mismatch"); }
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
	// first parse the value (to see if we are really leaf, or a masked AND query)
	if (KeyVal->IsStr()) {
		// parse the key
		KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
		// plain string, must be equal
		Type = oqitLeafGix;
		CmpType = oqctEqual;
		ParseWordStr(KeyVal->GetStr(), IndexVoc);
	} else if (KeyVal->IsObj()) {
		// parse the key
		// not plain string, check for operator
		if (KeyVal->IsObjKey("$location")) {
			KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
			QmAssert(IndexVoc->GetKey(KeyId).IsLocation());
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
		} else if (KeyVal->IsObjKey("$ne")) {
			QmAssertR(KeyVal->GetObjKey("$ne")->IsStr(), "Query: $ne value must be string");
			KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
			Type = oqitLeafGix;
			CmpType = oqctNotEqual;
			ParseWordStr(KeyVal->GetObjKey("$ne")->GetStr(), IndexVoc);
		} else if (KeyVal->IsObjKey("$gt")) {
			QmAssertR(KeyVal->GetObjKey("$gt")->IsStr(), "Query: $gt value must be string");
			KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
			Type = oqitLeafGix;
			CmpType = oqctGreater;
			ParseWordStr(KeyVal->GetObjKey("$gt")->GetStr(), IndexVoc);
		} else if (KeyVal->IsObjKey("$lt")) {
			QmAssertR(KeyVal->GetObjKey("$lt")->IsStr(), "Query: $lt value must be string");
			KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
			Type = oqitLeafGix;
			CmpType = oqctLess;
			ParseWordStr(KeyVal->GetObjKey("$lt")->GetStr(), IndexVoc);
		} else if (KeyVal->IsObjKey("$or")) {
			PJsonVal ObjVals = KeyVal->GetObjKey("$or");
			QmAssertR(ObjVals->IsArr(), "Query: $or as value requires an array of potential values");
			Type = oqitOr;
			for (int ValN = 0; ValN < ObjVals->GetArrVals(); ValN++) {
				ItemV.Add(TQueryItem(Base, Store, KeyNm, ObjVals->GetArrVal(ValN)));
			}
		} else if (KeyVal->IsObjKey("$wc")) {
			QmAssertR(KeyVal->GetObjKey("$wc")->IsStr(), "Query: $wc value must be string");
			KeyId = IndexVoc->GetKeyId(Store->GetStoreId(), KeyNm);
			// wildchars interparted as or with all possibilities
			Type = oqitLeafGix;
			CmpType = oqctWildChar;
			// identify possibilities
			ParseWordStr(KeyVal->GetObjKey("$wc")->GetStr(), IndexVoc);
		} else {
			throw TQmExcept::New("Query: Invalid operator: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
		}
	} else if (KeyVal->IsArr()) {
		// a vector of values => we are actually an AND query of multiple values for same key
		Type = oqitAnd;
		for (int ValN = 0; ValN < KeyVal->GetArrVals(); ValN++) {
			PJsonVal Val = KeyVal->GetArrVal(ValN);
			// make sure we don't have nested arreys
			QmAssertR(Val->IsStr() || Val->IsObj(), 
				"Query: Multiple conditions for a key must be string or object");
			// handle each value
			ItemV.Add(TQueryItem(Base, Store, KeyNm, Val));
		}
	} else {
		throw TQmExcept::New("Query: Invalid key definition: '" + TJsonVal::GetStrFromVal(KeyVal) + "'");
	}
}

TQueryItem::TQueryItem(const TWPt<TStore>& Store, const uint64& RecId):
	Type(oqitRec), Rec(Store, RecId) { }

TQueryItem::TQueryItem(const TRec& _Rec):
	Type(oqitRec), Rec(_Rec) { }

TQueryItem::TQueryItem(const PRecSet& _RecSet):
	Type(oqitRecSet), RecSet(_RecSet) { RecSet->SortById(); }

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, 
	const uint64& WordId, const TQueryCmpType& _CmpType): Type(oqitLeafGix), 
		KeyId(_KeyId), CmpType(_CmpType) { WordIdV.Add(WordId); }

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId,
		const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitLeafGix) {

    // read the Key
    KeyId = _KeyId;
    QmAssertR(Base->GetIndexVoc()->IsKeyId(KeyId), "Unknown Key ID: " + KeyId.GetStr());
    // read the sort type
    CmpType = _CmpType;
    // parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, const TStr& KeyNm,
		const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitLeafGix) {

	// get the key
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	// read sort type
	CmpType = _CmpType;
    // parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, const TStr& KeyNm,
		const TStr& WordStr, const TQueryCmpType& _CmpType): Type(oqitLeafGix) {

	// get the key
	const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
	// read sort type
	CmpType = _CmpType;
    // parse the word string
	ParseWordStr(WordStr, Base->GetIndexVoc());
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const int& _KeyId, 
	const TFltPr& _Loc, const int& _LocLimit, const double& _LocRadius): 
		Type(oqitGeo), KeyId(_KeyId), Loc(_Loc), LocRadius(_LocRadius), 
		LocLimit(_LocLimit) { QmAssert(LocLimit > 0); }

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const uint& StoreId, 
		const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit, 
		const double& _LocRadius): Type(oqitGeo), Loc(_Loc), 
			LocRadius(_LocRadius), LocLimit(_LocLimit) {

	QmAssert(LocLimit > 0);
	// get the key
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
}

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& StoreNm, 
		const TStr& KeyNm, const TFltPr& _Loc, const int& _LocLimit, 
		const double& _LocRadius): Type(oqitGeo), Loc(_Loc), 
			LocRadius(_LocRadius), LocLimit(_LocLimit) {

	QmAssert(LocLimit > 0);
	// get the key
	const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
    QmAssertR(Base->GetIndexVoc()->IsKeyNm(StoreId, KeyNm), "Unknown Key Name: " + KeyNm);
	KeyId = Base->GetIndexVoc()->GetKeyId(StoreId, KeyNm);
}

TQueryItem::TQueryItem(const TQueryItemType& _Type): Type(_Type) { 
	QmAssert(Type == oqitAnd || Type == oqitOr); }

TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item): 
	Type(_Type), ItemV(1, 0) { ItemV.Add(Item); 
		QmAssert(Type == oqitAnd || Type == oqitOr || Type == oqitNot);}
	
TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItem& Item1, 
	const TQueryItem& Item2): Type(_Type), ItemV(2, 0) { ItemV.Add(Item1); ItemV.Add(Item2); 
		QmAssert(Type == oqitAnd || Type == oqitOr);}
	
TQueryItem::TQueryItem(const TQueryItemType& _Type, const TQueryItemV& _ItemV): 
	Type(_Type), ItemV(_ItemV) { QmAssert(Type == oqitAnd || Type == oqitOr);}

TQueryItem::TQueryItem(const int& _JoinId, const int& _SampleSize, const TQueryItem& Item): 
	Type(oqitJoin), ItemV(1, 0), JoinId(_JoinId), SampleSize(_SampleSize) { ItemV.Add(Item); }

TQueryItem::TQueryItem(const TWPt<TBase>& Base, const TStr& JoinNm, const int& _SampleSize,
		const TQueryItem& Item): Type(oqitJoin), ItemV(1, 0), SampleSize(_SampleSize) { 
			
	ItemV.Add(Item); 
	// get join id
	const uint StoreId = Item.GetStoreId(Base);
	const TWPt<TStore>& Store = Base->GetStoreByStoreId(StoreId);
	JoinId = Store->GetJoinId(JoinNm);
}

uint TQueryItem::GetStoreId(const TWPt<TBase>& Base) const {
	if (IsLeafGix() || IsGeo()) { 
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
	if (IsLeafGix() || IsGeo()) {
		// always weighted when only one key
		return true;
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

TQueryAggr::TQueryAggr(TSIn& SIn): AggrNm(SIn), AggrType(SIn) {
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
	const int& _Offset): QueryItem(_QueryItem),  SortFieldId(_SortFieldId),
		SortAscP(_SortAscP), Limit(_Limit), Offset(_Offset) { }

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
		QmAssert(SortVal->IsObj() && SortVal->GetObjKeys()==1);
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
	return Query;
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
class TGeoIndex {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TGeoIndex>;

	// location precision (1,000,000 ~~ one meter)
	TFlt Precision;
	// map from location to records
	//TODO: Switch to GIX, maybe
	THash<TIntPr, TUInt64V> LocRecIdH;
	// location index
	TSphereNn<TInt, double> SphereNn;

	TIntPr GetLocId(const TFltPr& Loc) const;
	void LocKeyIdToRecId(const TIntV& LocKeyIdV, const int& Limit, TUInt64V& AllRecIdV) const;
	//DEBUG: counts all the indexed records
	int AllRecs() const;

public:
	// create new empty index
	TGeoIndex(const double& _Precision): Precision(_Precision), 
		SphereNn(TSphereNn<TInt, double>::EarthRadiusKm() * 1000.0) { }
	static PGeoIndex New(const double& Precision = 1000000.0) { return new TGeoIndex(Precision); }
	// load existing index
	TGeoIndex(TSIn& SIn): Precision(SIn), LocRecIdH(SIn), SphereNn(SIn) { }
	static PGeoIndex Load(TSIn& SIn) { return new TGeoIndex(SIn); }
	static PGeoIndex LoadBin(const TStr& FNm) { TFIn FIn(FNm); return new TGeoIndex(FIn); }
	// save index
	void Save(TSOut& SOut) { Precision.Save(SOut); LocRecIdH.Save(SOut); SphereNn.Save(SOut); }
	void SaveBin(const TStr& FNm) { TFOut FOut(FNm); Save(FOut); }
	
	// add new record
	void AddKey(const TFltPr& Loc, const uint64& RecId);
	// delete record
	void DelKey(const TFltPr& Loc, const uint64& RecId);
	// range query (in meters)
	void SearchRange(const TFltPr& Loc, const double& Radius,
		const int& Limit, TUInt64V& RecIdV) const;
	// (limit) nearest neighbour query
	void SearchNn(const TFltPr& Loc, const int& Limit, TUInt64V& RecIdV) const;

	// tells if two locations identical based on Precision
	bool LocEquals(const TFltPr& Loc1, const TFltPr& Loc2) const;
};


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
        if (Val1 < Val2) { ResV.Add(Val1); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; }
		else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++){
        ResV.Add(MainV.GetVal(RestValN1));}
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++){
        ResV.Add(JoinV.GetVal(RestValN2));}    
    MainV = ResV;
}

void TIndex::TQmGixDefMerger::Intrs(
		TQmGixItemV& MainV, const TQmGixItemV& JoinV) const {

    TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
		else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    MainV = ResV;
}

void TIndex::TQmGixDefMerger::Minus(const TQmGixItemV& MainV, 
		const TQmGixItemV& JoinV, TQmGixItemV& ResV) const {

	MainV.Diff(JoinV, ResV);
}

void TIndex::TQmGixDefMerger::Merge(TQmGixItemV& ItemV) const {
	if (ItemV.Empty()) { return; } // nothing to do in this case
	if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted
	// merge counts
	int LastItemN = 0; bool ZeroP = false;
    for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
		//const TQmGixItem& LastItem = ItemV[LastItemN];
		//const TQmGixItem& Item = ItemV[ItemN];
        if (ItemV[ItemN] != ItemV[ItemN-1])  {
            LastItemN++;
            ItemV[LastItemN] = ItemV[ItemN];
        } else {
            ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
        }
		ZeroP = (ItemV[LastItemN].Dat <= 0) || ZeroP;
    }
    ItemV.Reserve(ItemV.Reserved(), LastItemN+1);
	// remove items with zero count
	if (ZeroP) {
		LastItemN = 0;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			const TQmGixItem& Item = ItemV[ItemN];
			if (Item.Dat > 0) {
				ItemV[LastItemN] = Item;
				LastItemN++;
			} else if (Item.Dat < 0) {
				TEnv::Error->OnStatusFmt("Warning: negative item count %d:%d!", (int)Item.Key, (int)Item.Dat);
			}
		}
		ItemV.Reserve(ItemV.Reserved(), LastItemN);
	}
}

void TIndex::TQmGixRmDupMerger::Union(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const
{
	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ResV.Add(TQmGixItem(Val1.Key, 1)); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(TQmGixItem(Val2.Key, 1)); ValN2++; }
		else { 
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);			
			ResV.Add(TQmGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++; 
		}
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++)
	{
		TQmGixItem Item = MainV.GetVal(RestValN1);	
		Item.Dat = TInt::GetMn(1, Item.Dat);
        ResV.Add(Item);
	}
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++)
	{
		TQmGixItem Item = JoinV.GetVal(RestValN2);		
		Item.Dat = TInt::GetMn(1, Item.Dat);
        ResV.Add(Item);
	}    
    MainV = ResV;
}

void TIndex::TQmGixRmDupMerger::Intrs(TQmGixItemV& MainV, const TQmGixItemV& JoinV) const
{
	TQmGixItemV ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
		else 
		{ 
			int fq1 = TInt::GetMn(1, Val1.Dat);
			int fq2 = TInt::GetMn(1, Val2.Dat);
			ResV.Add(TQmGixItem(Val1.Key, fq1 + fq2)); ValN1++; ValN2++; 
		}
    }
    MainV = ResV;
}

TIndex::TQmGixKeyStr::TQmGixKeyStr(const TWPt<TBase>& _Base, 
	const TWPt<TIndexVoc>& _IndexVoc): Base(_Base), IndexVoc(_IndexVoc) { }

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
			// unknow operator
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

bool TIndex::DoQuery(const TIndex::PQmGixExpItem& ExpItem, 
        const PQmGixMerger& Merger, TQmGixItemV& ResIdFqV) const {

	// clean if there is anything on the input
	ResIdFqV.Clr(); 
    // execute query
    return ExpItem->Eval(Gix, ResIdFqV, Merger);
}

TIndex::TIndex(const TStr& _IndexFPath, const TFAccess& _Access, 
        const PIndexVoc& _IndexVoc, const int64& CacheSize) {

	IndexFPath = _IndexFPath;
    Access = _Access;
    // initialize invered index
	DefMerger = TQmGixDefMerger::New();
    Gix = TQmGix::New("Index", IndexFPath, Access, CacheSize, DefMerger);
	// initialize location index
	TStr SphereFNm = IndexFPath + "Index.Geo";
	if (TFile::Exists(SphereFNm) && Access != faCreate) {
		TFIn SphereFIn(SphereFNm); GeoIndexH.Load(SphereFIn); 
	}
    // initialize vocabularies
    IndexVoc = _IndexVoc;
}

TIndex::~TIndex() {
	if (!IsReadOnly()) {
		TEnv::Logger->OnStatus("Saving and closing inverted index");
		Gix.Clr();
		TEnv::Logger->OnStatus("Saving and closing location index");
		TFOut SphereFOut(IndexFPath + "Index.Geo"); GeoIndexH.Save(SphereFOut);
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
    Gix->AddItem(TKeyWord(KeyId, WordId), TQmGixItem(RecId, RecFq));
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

void TIndex::Delete(const int& KeyId, const uint64& WordId,  const uint64& RecId, const int& RecFq) {
	// -1 should never come to here 
	Assert(KeyId != -1);
	// we shouldn't modify read-only index
	QmAssertR(!IsReadOnly(), "Cannot edit read-only index!");
	// delete from index (add item with negative count, merger will delete item if necessary)
	Gix->AddItem(TKeyWord(KeyId, WordId), TQmGixItem(RecId, -RecFq));
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

void TIndex::MergeIndex(const TWPt<TIndex>& TmpIndex) {
    Gix->MergeIndex(TmpIndex->Gix);
}

void TIndex::SearchAnd(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const {
    // prepare the query
	TVec<PQmGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {
		ExpItemV.Add(TQmGixExpItem::NewItem(KeyWordV[ItemN]));
	}
	PQmGixExpItem ExpItem =TQmGixExpItem::NewAndV(ExpItemV);	
    // execute the query and filter the results to desired item type
    DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
}

void TIndex::SearchOr(const TIntUInt64PrV& KeyWordV, TUInt64IntKdV& StoreRecIdFqV) const {
    // prepare the query
	TVec<PQmGixExpItem> ExpItemV(KeyWordV.Len(), 0);
	for (int ItemN = 0; ItemN < KeyWordV.Len(); ItemN++) {
		ExpItemV.Add(TQmGixExpItem::NewItem(KeyWordV[ItemN]));
	}
	PQmGixExpItem ExpItem =TQmGixExpItem::NewOrV(ExpItemV);	
    // execute the query and filter the results to desired item type
    DoQuery(ExpItem, DefMerger, StoreRecIdFqV);
}

TPair<TBool, PRecSet> TIndex::Search(const TWPt<TBase>& Base,
		const TQueryItem& QueryItem, const PQmGixMerger& Merger) const {

	// get query result store
	TWPt<TStore> Store = QueryItem.GetStore(Base);
	// when query empty, return empty set
	if (QueryItem.Empty()) {
        return TPair<TBool, PRecSet>(false, TRecSet::New(Store));
    }
    // prepare the query
	PQmGixExpItem ExpItem = ToExpItem(QueryItem);
	// do the query
	TUInt64IntKdV StoreRecIdFqV;
	const bool NotP = DoQuery(ExpItem, Merger, StoreRecIdFqV);
	// return record set
	PRecSet RecSet = TRecSet::New(Store, StoreRecIdFqV, QueryItem.IsWgt());
	return TPair<TBool, PRecSet>(NotP, RecSet);
}

PRecSet TIndex::SearchRange(const TWPt<TBase>& Base, const int& KeyId, 
        const TFltPr& Loc, const double& Radius, const int& Limit) const {

	TUInt64V RecIdV;
	const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
	if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->SearchRange(Loc, Radius, Limit, RecIdV); }
	return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

PRecSet TIndex::SearchNn(const TWPt<TBase>& Base, const int& KeyId,
        const TFltPr& Loc, const int& Limit) const {
    
	TUInt64V RecIdV;
	const uint StoreId = IndexVoc->GetKey(KeyId).GetStoreId();
	if (GeoIndexH.IsKey(KeyId)) { GeoIndexH.GetDat(KeyId)->SearchNn(Loc, Limit, RecIdV); }
	return TRecSet::New(Base->GetStoreByStoreId(StoreId), RecIdV);
}

void TIndex::GetJoinRecIdFqV(const int& JoinKeyId, const uint64& RecId, TUInt64IntKdV& JoinRecIdFqV) const {
	TKeyWord KeyWord(JoinKeyId, RecId);
	if (!Gix->IsKey(KeyWord)) { return; }
	PQmGixItemSet res = Gix->GetItemSet(KeyWord); res->Def();
	for (int i=0; i<res->GetItems(); i++) {
		JoinRecIdFqV.Add(res->GetItem(i));
	}
}

void TIndex::SaveTxt(const TWPt<TBase>& Base, const TStr& FNm) {
	Gix->SaveTxt(FNm, TQmGixKeyStr::New(Base, IndexVoc));
}

///////////////////////////////
// QMiner-Temporary-Index
void TTempIndex::NewIndex(const PIndexVoc& IndexVoc) {
    // prepare a temporary index path
    TUInt64 NowTmMSec = TTm::GetMSecsFromTm(TTm::GetCurUniTm());
    TStr TempIndexFPath = TempFPath + NowTmMSec.GetStr() + "/";
    EAssertR(TDir::GenDir(TempIndexFPath), "Unable to create directory '" + TempIndexFPath + "'");
    TempIndexFPathQ.Push(TempIndexFPath);
	// prepare new temporary index
    TEnv::Logger->OnStatus(TStr::Fmt("Creating a temporary index in %s ...", TempIndexFPath.CStr()));
	TempIndex = TIndex::New(TempIndexFPath, faCreate, IndexVoc, IndexCacheSize);
}

void TTempIndex::Merge(const TWPt<TIndex>& Index) {
	// close any previous indices
	TempIndex.Clr();
    // marge new indexes with the current one
	while (!TempIndexFPathQ.Empty()) {
        TStr TempIndexFPath = TempIndexFPathQ.Top();
		TempIndexFPathQ.Pop();
        // load index
        TEnv::Logger->OnStatus(TStr::Fmt("Merging a temporary index from %s ...", TempIndexFPath.CStr()));
		PIndex NewIndex = TIndex::New(TempIndexFPath,
            faRdOnly, Index->GetIndexVoc(), int64(10*TInt::Mega));
        // merge with main index
        Index->MergeIndex(NewIndex);
        TEnv::Logger->OnStatus("Closing temporary index Start"); 
		NewIndex.Clr();
		TEnv::Logger->OnStatus("Closing temporary index Done");
        // deleting temp index
        TFFile TempFile(TempIndexFPath, ""); TStr DelFNm;
        while (TempFile.Next(DelFNm)) { TFile::Del(DelFNm, false); }
        if (!TDir::DelDir(TempIndexFPath)) {
            TEnv::Logger->OnStatus(
				TStr::Fmt("Unable to delete directory '%s'", TempIndexFPath.CStr())); 
		}
    }
}

////////////////////////////////////////////////
// QMiner-Operator
TOp::TOp(const TStr& _OpNm): OpNm(_OpNm) { TValidNm::AssertValidNm(OpNm); }

PRecSet TOp::Exec(const TWPt<TBase>& Base, const TRecSetV& InRecSetV, const PJsonVal& ParamVal){
	QmAssertR(IsFunctional(), "Non-functional operator called as functional!");
	TRecSetV OutRSetV; Exec(Base, InRecSetV, ParamVal, OutRSetV);
	QmAssertR(OutRSetV.Len() == 1, "Non-functional return for functional operator!");
	return OutRSetV[0];
}

///////////////////////////////
// QMiner-Aggregator
TFunRouter<PAggr, TAggr::TNewF> TAggr::NewRouter;

void TAggr::Init() {
    Register<TAggrs::TCount>();
    Register<TAggrs::THistogram>();
    Register<TAggrs::TKeywords>();
    Register<TAggrs::TTimeLine>();
	Register<TAggrs::TTwitterGraph>();
	#ifdef OG_AGGR_DOC_ATLAS
    Register<TAggrs::TDocAtlas>();
    #endif
}

TAggr::TAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm): Base(_Base), AggrNm(_AggrNm) { }

PAggr TAggr::New(const TWPt<TBase>& Base, const PRecSet& RecSet, const TQueryAggr& QueryAggr) {
    return NewRouter.Fun(QueryAggr.GetType())(Base, QueryAggr.GetNm(), RecSet, QueryAggr.GetParamVal());
}

///////////////////////////////
// QMiner-Stream-Aggregator
TFunRouter<PStreamAggr, TStreamAggr::TNewF> TStreamAggr::NewRouter;
TFunRouter<PStreamAggr, TStreamAggr::TLoadF> TStreamAggr::LoadRouter;

void TStreamAggr::Init() {
    Register<TStreamAggrs::TRecBuffer>();
    Register<TStreamAggrs::TCount>();
    Register<TStreamAggrs::TTimeSeriesTick>();
    Register<TStreamAggrs::TTimeSeriesWinBuf>();
    Register<TStreamAggrs::TMa>();
    Register<TStreamAggrs::TEma>();
    Register<TStreamAggrs::TVar>();
    Register<TStreamAggrs::TCov>();
    Register<TStreamAggrs::TCorr>();
    Register<TStreamAggrs::TResampler>();
}

TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, const TStr& _AggrNm): 
    Base(_Base), AggrNm(_AggrNm), Guid(TGuid::GenGuid()) { 
        TValidNm::AssertValidNm(AggrNm); }

TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, const PJsonVal& ParamVal):
    Base(_Base), AggrNm(ParamVal->GetObjStr("name")), Guid(TGuid::GenGuid()) { 
        TValidNm::AssertValidNm(AggrNm); }
        
TStreamAggr::TStreamAggr(const TWPt<TBase>& _Base, TSIn& SIn): 
    Base(_Base), AggrNm(SIn), Guid(SIn) { }
	
PStreamAggr TStreamAggr::New(const TWPt<TBase>& Base, 
        const TStr& TypeNm, const PJsonVal& ParamVal) {

    return NewRouter.Fun(TypeNm)(Base, ParamVal);
}

PStreamAggr TStreamAggr::Load(const TWPt<TBase>& Base, TSIn& SIn) {
	TStr TypeNm(SIn); return LoadRouter.Fun(TypeNm)(Base, SIn);
}

void TStreamAggr::Save(TSOut& SOut) const { 
    AggrNm.Save(SOut); Guid.Save(SOut);
}

///////////////////////////////
// QMiner-Stream-Aggregator-Base
TStreamAggrBase::TStreamAggrBase(const TWPt<TBase>& Base, TSIn& SIn) { 
	const int StreamAggrs = TInt(SIn);
	for (int StreamAggrN = 0; StreamAggrN < StreamAggrs; StreamAggrN++) {
		PStreamAggr StreamAggr = TStreamAggr::Load(Base, SIn);
		AddStreamAggr(StreamAggr);
	}
}

PStreamAggrBase TStreamAggrBase::New() { 
    return new TStreamAggrBase; 
}

PStreamAggrBase TStreamAggrBase::Load(const TWPt<TBase>& Base, TSIn& SIn) { 
    return new TStreamAggrBase(Base, SIn); 
}

void TStreamAggrBase::Save(TSOut& SOut) const { 
	TInt(StreamAggrH.Len()).Save(SOut);
	int KeyId = StreamAggrH.FFirstKeyId();
	while (StreamAggrH.FNextKeyId(KeyId)) {
		StreamAggrH[KeyId]->Save(SOut);
	}
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
	return StreamAggrH.GetDat(StreamAggrNm); 
}

const PStreamAggr& TStreamAggrBase::GetStreamAggr(const int& StreamAggrId) const {
    return StreamAggrH[StreamAggrId]; 
}

void TStreamAggrBase::AddStreamAggr(const PStreamAggr& StreamAggr) { 
	StreamAggrH.AddDat(StreamAggr->GetAggrNm(), StreamAggr); 
}

int TStreamAggrBase::GetFirstStreamAggrId() const {
    return StreamAggrH.FFirstKeyId(); 
}

bool TStreamAggrBase::GetNextStreamAggrId(int& AggrId) const { 
    return StreamAggrH.FNextKeyId(AggrId); 
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
TStreamAggrTrigger::TStreamAggrTrigger(const PStreamAggrBase& _StreamAggrBase): 
    StreamAggrBase(_StreamAggrBase) { }

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
TBase::TBase(const TStr& _FPath, const int64& IndexCacheSize): InitP(false) {
	IAssertR(TEnv::IsInit(), "QMiner environment (TQm::TEnv) is not initialized");
	// open as create
	FAccess = faCreate; FPath = _FPath;
	TEnv::Logger->OnStatus("Opening in create mode");
	// prepare index
	IndexVoc = TIndexVoc::New();
	Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize);
	// add standard operators
	AddOp(TOpLinSearch::New());
	AddOp(TOpGroupBy::New());
	AddOp(TOpSplitBy::New());
	// initialize with empty stores
	StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
    // initialize empty stream aggregate bases for each store
    StreamAggrBaseV.Gen(TEnv::GetMxStores()); StreamAggrBaseV.PutAll(NULL);
	// by default no temporary folder
	TempFPathP = false;
}

TBase::TBase(const TStr& _FPath, const TFAccess& _FAccess, const int64& IndexCacheSize): InitP(false) {
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
	Index = TIndex::New(FPath, FAccess, IndexVoc, IndexCacheSize);
	// add standard operators
	AddOp(TOpLinSearch::New());
	AddOp(TOpGroupBy::New());
	AddOp(TOpSplitBy::New());
	// initialize with empty stores
	StoreV.Gen(TEnv::GetMxStores()); StoreV.PutAll(NULL);
    // initialize empty stream aggregate bases for each store
    StreamAggrBaseV.Gen(TEnv::GetMxStores()); StreamAggrBaseV.PutAll(NULL);    
	// by default no temporary folder
	TempFPathP = false;
} 

TBase::~TBase() {
	if (FAccess != faRdOnly) {
		TEnv::Logger->OnStatus("Saving index vocabulary ... ");
		TFOut IndexVocFOut(FPath + "IndexVoc.dat");
		IndexVoc->Save(IndexVocFOut);
		TEnv::Logger->OnStatus("Saving stream aggregates ...");
		TFOut StreamAggrFOut(FPath + "StreamAggr.dat");
        SaveStreamAggrBaseV(StreamAggrFOut);
	} else {
		TEnv::Logger->OnStatus("No saving of qminer base neccessary!");
	}
}

void TBase::SaveStreamAggrBaseV(TSOut& SOut) {   
    // get number of stream aggregate bases
    int StreamAggrBases = 0;
    for (int StoreId = 0; StoreId < StreamAggrBaseV.Len(); StoreId++) {
        if (!StreamAggrBaseV[StoreId].Empty()) { StreamAggrBases++; }
    }
    // save number of aggregate bases
    TInt(StreamAggrBases).Save(SOut);        
    // save each aggregate base
    for (int StoreId = 0; StoreId < StreamAggrBaseV.Len(); StoreId++) {
        if (!StreamAggrBaseV[StoreId].Empty()) {
            TUInt(StoreId).Save(SOut);
            StreamAggrBaseV[StoreId]->Save(SOut);
        }
    }
}

void TBase::LoadStreamAggrBaseV(TSIn& SIn) {
    const int StreamAggrBases = TInt(SIn);
    for (int StreamAggrBaseN = 0; StreamAggrBaseN < StreamAggrBases; StreamAggrBaseN++) {
        const uint StoreId = TUInt(SIn);
        // load stream aggregate base
        PStreamAggrBase StreamAggrBase = TStreamAggrBase::Load(this, SIn);
        // create trigger for the aggregate base
        GetStoreByStoreId(StoreId)->AddTrigger(TStreamAggrTrigger::New(StreamAggrBase));
        // remember the aggregate base for the store
        StreamAggrBaseV[StoreId] = StreamAggrBase;
    }    
}

PRecSet TBase::Invert(const PRecSet& RecSet, const TIndex::PQmGixMerger& Merger) {
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

TPair<TBool, PRecSet> TBase::Search(const TQueryItem& QueryItem, const TIndex::PQmGixMerger& Merger) {
	if (QueryItem.IsLeafGix()) {
		// return empty, when can be handled by index
		return TPair<TBool, PRecSet>(false, NULL);
	} else if (QueryItem.IsGeo()) {
		if (QueryItem.IsLocRadius()) {
			// must be handled by geo index
			PRecSet RecSet = Index->SearchRange(this, QueryItem.GetKeyId(), 
				QueryItem.GetLoc(), QueryItem.GetLocRadius(), QueryItem.GetLocLimit());
			return TPair<TBool, PRecSet>(false, RecSet);
		} else {
			// must be handled by geo index
			PRecSet RecSet = Index->SearchNn(this, QueryItem.GetKeyId(), 
				QueryItem.GetLoc(), QueryItem.GetLocLimit());
			return TPair<TBool, PRecSet>(false, RecSet); 
		}
	} else if (QueryItem.IsJoin()) {
		// special case when it's record passed by value
		const TQueryItem& SubItem = QueryItem.GetItem(0);
		if (SubItem.IsRec() && SubItem.GetRec().IsByVal()) {
			// do the join
			PRecSet JoinRecSet = SubItem.GetRec().DoJoin(this, QueryItem.GetJoinId());
			// return joined record set
			return TPair<TBool, PRecSet>(false, JoinRecSet);
		} else {
			// do the subordiante queries
			TPair<TBool, PRecSet> NotRecSet = Search(QueryItem.GetItem(0), Merger);
			// in case it's empty, we must go to index 
			if (NotRecSet.Val2.Empty()) { NotRecSet = Index->Search(this, QueryItem.GetItem(0), Merger); } 
			// in case it's negated, we must invert it
			if (NotRecSet.Val1) { NotRecSet.Val2 = Invert(NotRecSet.Val2, Merger); }
			// do the join
			PRecSet JoinRecSet = NotRecSet.Val2->DoJoin(this, QueryItem.GetJoinId(),
				QueryItem.GetSampleSize(), NotRecSet.Val2->IsWgt());
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
		// do all subsequents and keep track if any needs handling
		TBoolV NotV; TRecSetV RecSetV; bool EmptyP = true;
		for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
			// do subsequent search
			TPair<TBool, PRecSet> NotRecSet = Search(QueryItem.GetItem(ItemN), Merger);
			NotV.Add(NotRecSet.Val1); RecSetV.Add(NotRecSet.Val2);
			// check if to do anything
			EmptyP = EmptyP && RecSetV.Last().Empty();
		}
		// check if there is anything to do
		if (EmptyP) {
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
					TPair<TBool, PRecSet> NotRecSet = Index->Search(this, IndexQueryItem, Merger);
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
	if (FAccess != faCreate) {
		// load stream aggregates
		TFIn StreamAggrBaseFIn(FPath + "StreamAggr.dat");
        LoadStreamAggrBaseV(StreamAggrBaseFIn);
	}
	// done
	InitP = true;
}

TWPt<TIndex> TBase::GetIndex() const { 
	return TempIndex.Empty() ? TWPt<TIndex>(Index) : TempIndex->GetIndex(); 
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

bool TBase::IsStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const {
	return GetStreamAggrBase(StoreId)->IsStreamAggr(StreamAggrNm);
}

const PStreamAggr& TBase::GetStreamAggr(const uint& StoreId, const TStr& StreamAggrNm) const {
	return GetStreamAggrBase(StoreId)->GetStreamAggr(StreamAggrNm);
}

const PStreamAggr& TBase::GetStreamAggr(const TStr& StoreNm, const TStr& StreamAggrNm) const {
	return GetStreamAggrBase(GetStoreByStoreNm(StoreNm)->GetStoreId())->GetStreamAggr(StreamAggrNm);
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

void TBase::Aggr(PRecSet& RecSet, const TQueryAggrV& QueryAggrV) {
	if (RecSet->Empty()) { return; }
	for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
		const TQueryAggr& Aggr = QueryAggrV[QueryAggrN];
		RecSet->AddAggr(TAggr::New(this, RecSet, Aggr));
	}
}

void TBase::AddOp(const POp& NewOp) {
	OpH.AddDat(NewOp->GetOpNm(), NewOp);
}

void TBase::Operator(const TRecSetV& InRecSetV, const PJsonVal& ParamVal, TRecSetV& OutRecSetV) {
	// check what operator was requested
	QmAssert(ParamVal->IsObjKey("operator"));
	TStr OpNm = ParamVal->GetObjStr("operator");
	// make sure we have it
	QmAssert(this->IsOp(OpNm));
	POp Op = this->GetOp(OpNm);	
	// execute the operator
    Op->Exec(this, InRecSetV, ParamVal, OutRecSetV);
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
	TIndex::PQmGixMerger Merger = Index->GetDefMerger();
	TPair<TBool, PRecSet> NotRecSet = Search(Query->GetQueryItem(), Merger);
	// when empty, then query can be completly covered by index
	if (NotRecSet.Val2.Empty()) { 
		NotRecSet = Index->Search(this, Query->GetQueryItem(), Merger); 
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

void TBase::InitTempIndex(const uint64& IndexCacheSize) { 
	TempIndex = TTempIndex::New(TempFPath, IndexCacheSize); 
	TempIndex->NewIndex(IndexVoc);
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
				TQm::TEnv::Logger->OnStatusFmt("Record %I64u / %I64u (%.1f%%)\r", RecId, Recs, 100 * RecId / (double) Recs);
            }
		}
	}
	uint64 DiffSecs = TTm::GetDiffSecs(TTm::GetCurLocTm(), CurrentTime);
	int Mins = (int) (DiffSecs / 60);
	TQm::TEnv::Logger->OnStatusFmt("Time needed to make the backup: %d min, %d sec", Mins, (int) (DiffSecs - (Mins * 60)));

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
				const uint64 ExRecId = Json->IsObjKey("$id") ? (uint64) Json->GetObjNum("$id") : TUInt64::Mx.Val;
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
	int Mins = (int) (DiffSecs / 60);
	TQm::TEnv::Logger->OnStatusFmt("Time needed to make the restore: %d min, %d sec", Mins, (int) (DiffSecs - (Mins * 60)));

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

}

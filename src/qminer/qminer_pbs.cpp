/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "qminer_pbs.h"

namespace TQm {

namespace TStorage {

/// Add new record
uint64 TStorePbBlob::AddRec(const PJsonVal& RecVal) {// check if we are given reference to existing record
    try {
        // parse out record id, if referred directly
        {
            const uint64 RecId = TStore::GetRecId(RecVal);
            if (IsRecId(RecId)) {
                // check if we have anything more than record identifier, which would require calling UpdateRec
                if (RecVal->GetObjKeys() > 1) { UpdateRec(RecId, RecVal); }
                // return named record
                return RecId;
            }
        }
        // check if we have a primary field
        if (IsPrimaryField()) {
            uint64 PrimaryRecId = TUInt64::Mx;
            // primary field cannot be nullable, so we must have it
            const TStr& PrimaryField = GetFieldNm(PrimaryFieldId);
            QmAssertR(RecVal->IsObjKey(PrimaryField), "Missing primary field in the record: " + PrimaryField);
            // parse based on the field type
            if (PrimaryFieldType == oftStr) {
                TStr FieldVal = RecVal->GetObjStr(PrimaryField);
                if (PrimaryStrIdH.IsKey(FieldVal)) {
                    PrimaryRecId = PrimaryStrIdH.GetDat(FieldVal);
                }
            } else if (PrimaryFieldType == oftInt) {
                const int FieldVal = RecVal->GetObjInt(PrimaryField);
                if (PrimaryIntIdH.IsKey(FieldVal)) {
                    PrimaryRecId = PrimaryIntIdH.GetDat(FieldVal);
                }
            } else if (PrimaryFieldType == oftUInt64) {
                const uint64 FieldVal = RecVal->GetObjInt(PrimaryField);
                if (PrimaryUInt64IdH.IsKey(FieldVal)) {
                    PrimaryRecId = PrimaryUInt64IdH.GetDat(FieldVal);
                }
            } else if (PrimaryFieldType == oftFlt) {
                const double FieldVal = RecVal->GetObjNum(PrimaryField);
                if (PrimaryFltIdH.IsKey(FieldVal)) {
                    PrimaryRecId = PrimaryFltIdH.GetDat(FieldVal);
                }
            } else if (PrimaryFieldType == oftTm) {
                TStr TmStr = RecVal->GetObjStr(PrimaryField);
                TTm Tm = TTm::GetTmFromWebLogDateTimeStr(TmStr, '-', ':', '.', 'T');
                const uint64 FieldVal = TTm::GetMSecsFromTm(Tm);
                if (PrimaryTmMSecsIdH.IsKey(FieldVal)) {
                    PrimaryRecId = PrimaryTmMSecsIdH.GetDat(FieldVal);
                }
            }
            // check if we found primary field with existing value
            if (PrimaryRecId != TUInt64::Mx) {
                // check if we have anything more than primary field, which would require redirect to UpdateRec
                if (RecVal->GetObjKeys() > 1) { UpdateRec(PrimaryRecId, RecVal); }
                // return id of named record
                return PrimaryRecId;
            }
        }
    } catch (const PExcept& Except) {
        // error parsing, report error and return nothing
        ErrorLog("[TStoreImpl::AddRec] Error parsing out reference to existing record:");
        ErrorLog(Except->GetMsgStr());
        return TUInt64::Mx;
    }

    // always add system field that means "inserted_at"
    RecVal->AddToObj(TStoreWndDesc::SysInsertedAtFieldName, TTm::GetCurUniTm().GetStr());

    // for storing record id
    TPgBlobPt CacheRecId;
    TPgBlobPt MemRecId;
    uint64 RecId = RecIdCounter++;
    // store to disk storage
    if (DataBlobP) {
        TMem CacheRecMem;
        SerializatorCache->Serialize(RecVal, CacheRecMem, this);
        TPgBlobPt Pt = DataBlob->Put(CacheRecMem.GetBf(), CacheRecMem.Len());
        CacheRecId = Pt;
        RecIdBlobPtH.AddDat(RecId) = Pt;
        // index new record
        RecIndexer.IndexRec(CacheRecMem, RecId, *SerializatorCache);
    }
    // store to in-memory storage
    if (DataMemP) {
        TMem MemRecMem;
        SerializatorMem->Serialize(RecVal, MemRecMem, this);
        TPgBlobPt Pt = DataMem->Put(MemRecMem.GetBf(), MemRecMem.Len());
        MemRecId = Pt;
        RecIdBlobPtHMem.AddDat(RecId) = Pt;
        RecIndexer.IndexRec(MemRecMem, RecId, *SerializatorMem);
    }
    // make sure we are consistent with respect to Ids!
    if (DataBlobP && DataMemP) {
        EAssert(RecId == RecIdCounter - 1);
    }

    // remember value-recordId map when primary field available
    if (IsPrimaryField()) { SetPrimaryField(RecId); }

    // insert nested join records
    AddJoinRec(RecId, RecVal);
    // call add triggers
    OnAdd(RecId);

    // return record Id of the new record
    return RecId;
}

/// Update existing record
void TStorePbBlob::UpdateRec(const uint64& RecId, const PJsonVal& RecVal) {
    // figure out which storage fields are affected
    bool CacheP = false, MemP = false, PrimaryP = false;
    bool CacheVarP = false, MemVarP = false, KeyP = false;
    for (int FieldId = 0; FieldId < GetFields(); FieldId++) {
        // check if field appears in the record JSon
        TStr FieldNm = GetFieldNm(FieldId);
        if (RecVal->IsObjKey(FieldNm)) {
            CacheP = CacheP || (FieldLocV[FieldId] == slDisk);
            MemP = MemP || (FieldLocV[FieldId] == slMemory);
            PrimaryP = PrimaryP || (FieldId == PrimaryFieldId);
            TFieldDesc fd = GetFieldDesc(FieldId);
            switch (fd.GetFieldType()) {
            case TFieldType::oftBowSpV:
            case TFieldType::oftFltV:
            case TFieldType::oftIntV:
            case TFieldType::oftNumSpV:
            case TFieldType::oftStrV:
                // variable length
                CacheVarP = CacheVarP || (FieldLocV[FieldId] == slDisk);
                MemVarP = MemVarP || (FieldLocV[FieldId] == slMemory);
                break;
            case TFieldType::oftStr:
                // variable length
                CacheVarP = CacheVarP ||
                    (FieldLocV[FieldId] == slDisk && !SerializatorCache->IsInFixedPart(FieldId));
                MemVarP = MemVarP ||
                    (FieldLocV[FieldId] == slMemory  && !SerializatorMem->IsInFixedPart(FieldId));
                break;
            default:
                break;
            }
            KeyP = KeyP || RecIndexer.IsFieldIndexKey(FieldId);
        }
    }
    // remove old primary field
    if (PrimaryP) { DelPrimaryField(RecId); }
    // update disk serialization when necessary
    if (CacheP) {
        // update serialization
        TMem Mem;
        TPgBlobPt Pt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn MIn = DataBlob->Get(Pt);

        TIntSet CacheChangedFieldIdSet;
        if (CacheVarP || KeyP) {
            // variable fields changed, so we need to serialize whole record
            TMem CacheNewRecMem;
            TIntSet CacheChangedFieldIdSet;
            TMemBase CacheOldRecMem = MIn.GetMemBase();

            SerializatorCache->SerializeUpdate(RecVal, CacheOldRecMem,
                CacheNewRecMem, this, CacheChangedFieldIdSet);

            // update the stored serializations with new values
            Pt = DataBlob->Put(Mem.GetBf(), Mem.Len(), Pt);
            RecIdBlobPtH(RecId) = Pt;
            // update indexes pointing to the record
            RecIndexer.UpdateRec(CacheOldRecMem, CacheNewRecMem, RecId,
                CacheChangedFieldIdSet, *SerializatorCache);
        } else {
            // nice, all changes can be done in-place, no index changes
            SerializatorCache->SerializeUpdateInPlace(RecVal, MIn, this,
                CacheChangedFieldIdSet);
            DataBlob->SetDirty(Pt);
        }
    }
    // update in-memory serialization when necessary
    if (MemP) {
        // update serialization
        TMem Mem;
        TPgBlobPt Pt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn MIn = DataBlob->Get(Pt);

        TIntSet ChangedFieldIdSet;
        if (MemVarP || KeyP) {
            // variable fields changed, so we need to serialize whole record
            TMem NewRecMem;
            TIntSet ChangedFieldIdSet;
            TMemBase OldRecMem = MIn.GetMemBase();

            SerializatorMem->SerializeUpdate(RecVal, OldRecMem,
                NewRecMem, this, ChangedFieldIdSet);

            // update the stored serializations with new values
            Pt = DataMem->Put(Mem.GetBf(), Mem.Len(), Pt);
            RecIdBlobPtHMem(RecId) = Pt;
            // update indexes pointing to the record
            RecIndexer.UpdateRec(OldRecMem, NewRecMem, RecId,
                ChangedFieldIdSet, *SerializatorMem);
        } else {
            // nice, all changes can be done in-place, no index changes
            SerializatorMem->SerializeUpdateInPlace(RecVal, MIn, this,
                ChangedFieldIdSet);
            DataMem->SetDirty(Pt);
        }
    }
    // check if primary key changed and update the mapping
    if (PrimaryP) { SetPrimaryField(RecId); }
    // call update triggers
    OnUpdate(RecId);
}

//////////////////////////////////////////////////////////

/// Load page with with given record and return pointer to it
TThinMIn TStorePbBlob::GetPgBf(const uint64& RecId, const bool& UseMem) const {
    if (UseMem) {
        const TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        return min;
    } else {
        const TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        return min;
    }
}

/// Get serializator for given location
TRecSerializator* TStorePbBlob::GetSerializator(const TStoreLoc& StoreLoc) const {
    return (StoreLoc == TStoreLoc::slDisk ? SerializatorCache : SerializatorMem);
}

/// Check if the value of given field for a given record is NULL
bool TStorePbBlob::IsFieldNull(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);		
    return GetSerializator(FieldLocV[FieldId])->IsFieldNull(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->IsFieldNull(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->IsFieldNull(GetPgBf(RecId, true), FieldId);
    //	//return SerializatorMem.IsFieldNull(GetPgBf(RecId), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.IsFieldNull(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
int TStorePbBlob::GetFieldInt(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldInt(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldInt(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldInt(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldInt(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldIntV(MIn, FieldId, IntV);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldIntV(GetPgBf(RecId), FieldId, IntV);
    //} else {
    //	SerializatorMem->GetFieldIntV(GetPgBf(RecId, true), FieldId, IntV);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldIntV(Rec, FieldId, IntV);
    //}
}
/// Get field value using field id (default implementation throws exception)
uint64 TStorePbBlob::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldUInt64(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldUInt64(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldUInt64(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldUInt64(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
TStr TStorePbBlob::GetFieldStr(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldStr(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldStr(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldStr(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldStr(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldStrV(MIn, FieldId, StrV);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldStrV(GetPgBf(RecId), FieldId, StrV);
    //} else {
    //	SerializatorMem->GetFieldStrV(GetPgBf(RecId, true), FieldId, StrV);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldStrV(Rec, FieldId, StrV);
    //}
}
/// Get field value using field id (default implementation throws exception)
bool TStorePbBlob::GetFieldBool(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldBool(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldBool(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldBool(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldBool(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
double TStorePbBlob::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldFlt(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldFlt(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldFlt(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldFlt(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
TFltPr TStorePbBlob::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldFltPr(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldFltPr(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldFltPr(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldFltPr(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldFltV(MIn, FieldId, FltV);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldFltV(GetPgBf(RecId), FieldId, FltV);
    //} else {
    //	SerializatorMem->GetFieldFltV(GetPgBf(RecId, true), FieldId, FltV);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldFltV(Rec, FieldId, FltV);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldTm(MIn, FieldId, Tm);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldTm(GetPgBf(RecId), FieldId, Tm);
    //} else {
    //	SerializatorMem->GetFieldTm(GetPgBf(RecId, true), FieldId, Tm);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldTm(Rec, FieldId, Tm);
    //}
}
/// Get field value using field id (default implementation throws exception)
uint64 TStorePbBlob::GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    return GetSerializator(FieldLocV[FieldId])->GetFieldTmMSecs(MIn, FieldId);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	return SerializatorCache->GetFieldTmMSecs(GetPgBf(RecId), FieldId);
    //} else {
    //	return SerializatorMem->GetFieldTmMSecs(GetPgBf(RecId, true), FieldId);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//return SerializatorMem.GetFieldTmMSecs(Rec, FieldId);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldNumSpV(MIn, FieldId, SpV);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldNumSpV(GetPgBf(RecId), FieldId, SpV);
    //} else {
    //	SerializatorMem->GetFieldNumSpV(GetPgBf(RecId, true), FieldId, SpV);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldNumSpV(Rec, FieldId, SpV);
    //}
}
/// Get field value using field id (default implementation throws exception)
void TStorePbBlob::GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const {
    TThinMIn MIn = GetPgBf(RecId, FieldLocV[FieldId] != TStoreLoc::slDisk);
    GetSerializator(FieldLocV[FieldId])->GetFieldBowSpV(MIn, FieldId, SpV);
    //if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
    //	SerializatorCache->GetFieldBowSpV(GetPgBf(RecId), FieldId, SpV);
    //} else {
    //	SerializatorMem->GetFieldBowSpV(GetPgBf(RecId, true), FieldId, SpV);
    //	//TMem Rec;
    //	//DataMem.GetVal(RecId, Rec);
    //	//SerializatorMem.GetFieldBowSpV(Rec, FieldId, SpV);
    //}
}

//////////////////////

/// Set the value of given field to NULL
void TStorePbBlob::SetFieldNull(const uint64& RecId, const int& FieldId) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldNull(min.GetBfAddrChar(), min.Len(), FieldId, true);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldNull(min.GetBfAddrChar(), min.Len(), FieldId, true);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldNull(Rec, OutRecMem, FieldId);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldInt(min.GetBfAddrChar(), min.Len(), FieldId, Int);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldInt(min.GetBfAddrChar(), min.Len(), FieldId, Int);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldInt(Rec, OutRecMem, FieldId, Int);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TMemBase mem_in = DataBlob->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorCache->SetFieldIntV(mem_in, mem_out, FieldId, IntV);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldIntV(mem_in, mem_out, FieldId, IntV);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldIntV(Rec, OutRecMem, FieldId, IntV);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldUInt64(min.GetBfAddrChar(), min.Len(), FieldId, UInt64);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldUInt64(min.GetBfAddrChar(), min.Len(), FieldId, UInt64);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldUInt64(Rec, OutRecMem, FieldId, UInt64);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TMemBase mem_in = DataBlob->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorCache->SetFieldStr(mem_in, mem_out, FieldId, Str);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldStr(mem_in, mem_out, FieldId, Str);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldStr(Rec, OutRecMem, FieldId, Str);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        TMem mem_in(min);
        TMem mem_out;
        SerializatorCache->SetFieldStrV(mem_in, mem_out, FieldId, StrV);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldStrV(mem_in, mem_out, FieldId, StrV);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldStrV(Rec, OutRecMem, FieldId, StrV);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldBool(min.GetBfAddrChar(), min.Len(), FieldId, Bool);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldBool(min.GetBfAddrChar(), min.Len(), FieldId, Bool);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldBool(Rec, OutRecMem, FieldId, Bool);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldFlt(min.GetBfAddrChar(), min.Len(), FieldId, Flt);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldFlt(min.GetBfAddrChar(), min.Len(), FieldId, Flt);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldFlt(Rec, OutRecMem, FieldId, Flt);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldFltPr(min.GetBfAddrChar(), min.Len(), FieldId, FltPr);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldFltPr(min.GetBfAddrChar(), min.Len(), FieldId, FltPr);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldFltPr(Rec, OutRecMem, FieldId, FltPr);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        TMem mem_in(min);
        TMem mem_out;
        SerializatorCache->SetFieldFltV(mem_in, mem_out, FieldId, FltV);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldFltV(mem_in, mem_out, FieldId, FltV);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldFltV(Rec, OutRecMem, FieldId, FltV);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldTm(min.GetBfAddrChar(), min.Len(), FieldId, Tm);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldTm(min.GetBfAddrChar(), min.Len(), FieldId, Tm);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldTm(Rec, OutRecMem, FieldId, Tm);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        SerializatorCache->SetFieldTmMSecs(min.GetBfAddrChar(), min.Len(), FieldId, TmMSecs);
        DataBlob->SetDirty(PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TThinMIn min = DataMem->Get(PgPt);
        SerializatorMem->SetFieldTmMSecs(min.GetBfAddrChar(), min.Len(), FieldId, TmMSecs);
        DataMem->SetDirty(PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldTmMSecs(Rec, OutRecMem, FieldId, TmMSecs);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        TMem mem_in(min);
        TMem mem_out;
        SerializatorCache->SetFieldNumSpV(mem_in, mem_out, FieldId, SpV);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldNumSpV(mem_in, mem_out, FieldId, SpV);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldNumSpV(Rec, OutRecMem, FieldId, SpV);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}
/// Set field value using field id (default implementation throws exception)
void TStorePbBlob::SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV) {
    if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
        TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
        TThinMIn min = DataBlob->Get(PgPt);
        TMem mem_in(min);
        TMem mem_out;
        SerializatorCache->SetFieldBowSpV(mem_in, mem_out, FieldId, SpV);
        RecIdBlobPtH.GetDat(RecId) = DataBlob->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
    } else {
        TPgBlobPt& PgPt = RecIdBlobPtHMem.GetDat(RecId);
        TMemBase mem_in = DataMem->GetMemBase(PgPt);
        TMem mem_out;
        SerializatorMem->SetFieldBowSpV(mem_in, mem_out, FieldId, SpV);
        RecIdBlobPtHMem.GetDat(RecId) = DataMem->Put(mem_out.GetBf(), mem_out.Len(), PgPt);
        //TMem Rec;
        //DataMem.GetVal(RecId, Rec);
        //TMem OutRecMem;
        //SerializatorMem.SetFieldBowSpV(Rec, OutRecMem, FieldId, SpV);
        //RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
        //DataMem.SetVal(RecId, Rec);
    }
}

////////////////////////////////////////////////////////////////////////

/// Gets the first record in the store (order defined by store implementation)
uint64 TStorePbBlob::FirstRecId() const {
    EAssertR(false, "Not implemented");
    return 0;
}

/// Gets the last record in the store (order defined by store implementation)
uint64 TStorePbBlob::LastRecId() const {
    EAssertR(false, "Not implemented");
    return 0;
}

/// Gets backward moving iterator (order defined by store implementation)
PStoreIter TStorePbBlob::BackwardIter() const {
    EAssertR(false, "Not implemented");
    return NULL;
}

/// Check if given ID is valid
bool TStorePbBlob::IsRecId(const uint64& RecId) const {
    return RecIdBlobPtH.IsKey(RecId);
}

/// Set primary field map
void TStorePbBlob::SetPrimaryField(const uint64& RecId) {
    if (PrimaryFieldType == oftStr) {
        PrimaryStrIdH.AddDat(GetFieldStr(RecId, PrimaryFieldId)) = RecId;
    } else if (PrimaryFieldType == oftInt) {
        PrimaryIntIdH.AddDat(GetFieldInt(RecId, PrimaryFieldId)) = RecId;
    } else if (PrimaryFieldType == oftUInt64) {
        PrimaryUInt64IdH.AddDat(GetFieldUInt64(RecId, PrimaryFieldId)) = RecId;
    } else if (PrimaryFieldType == oftFlt) {
        PrimaryFltIdH.AddDat(GetFieldFlt(RecId, PrimaryFieldId)) = RecId;
    } else if (PrimaryFieldType == oftTm) {
        PrimaryTmMSecsIdH.AddDat(GetFieldTmMSecs(RecId, PrimaryFieldId)) = RecId;
    }
}

/// Delete primary field map
void TStorePbBlob::DelPrimaryField(const uint64& RecId) {
    if (PrimaryFieldType == oftStr) {
        PrimaryStrIdH.DelIfKey(GetFieldStr(RecId, PrimaryFieldId));
    } else if (PrimaryFieldType == oftInt) {
        PrimaryIntIdH.DelIfKey(GetFieldInt(RecId, PrimaryFieldId));
    } else if (PrimaryFieldType == oftUInt64) {
        PrimaryUInt64IdH.DelIfKey(GetFieldUInt64(RecId, PrimaryFieldId));
    } else if (PrimaryFieldType == oftFlt) {
        PrimaryFltIdH.DelIfKey(GetFieldFlt(RecId, PrimaryFieldId));
    } else if (PrimaryFieldType == oftTm) {
        PrimaryTmMSecsIdH.DelIfKey(GetFieldTmMSecs(RecId, PrimaryFieldId));
    }
}

/// Check if record with given name exists
bool TStorePbBlob::IsRecNm(const TStr& RecNm) const {
    return RecNmFieldP && PrimaryStrIdH.IsKey(RecNm);
}

/// Find name of the record with given ID
TStr TStorePbBlob::GetRecNm(const uint64& RecId) const {
    // return empty string when no primary key
    if (!HasRecNm()) { return TStr(); }
    // get the name of primary key
    return GetFieldStr(RecId, PrimaryFieldId);
}

/// Return ID of record with given name
uint64 TStorePbBlob::GetRecId(const TStr& RecNm) const {
    return PrimaryStrIdH.GetDat(RecNm);
}

/// Get number of record
uint64 TStorePbBlob::GetRecs() const {
    return DataMemP ? RecIdBlobPtHMem.Len() : RecIdBlobPtH.Len();
}

/// Return iterator over store
PStoreIter TStorePbBlob::GetIter() const {
    if (Empty()) { return TStoreIterVec::New(); }
    return DataMemP ?
        //TStoreIterVec::New(DataMem.GetFirstValId(), DataMem.GetLastValId(), true) :
        TStoreIterHash<THash<TUInt64, TPgBlobPt>>::New(RecIdBlobPtHMem) :
        TStoreIterHash<THash<TUInt64, TPgBlobPt>>::New(RecIdBlobPtH);
}

/// Helper function for returning JSON definition of store
PJsonVal TStorePbBlob::GetStoreJson(const TWPt<TBase>& Base) const {
    PJsonVal Result = TStore::GetStoreJson(Base);
    Result->AddToObj("name", this->GetStoreNm());
    if (WndDesc.WindowType != TStoreWndType::swtNone) {
        PJsonVal WindowJson = TJsonVal::NewObj();
        WindowJson->AddToObj("type", WndDesc.WindowType == TStoreWndType::swtLength ? "length" : "time");
        WindowJson->AddToObj("size", (int)WndDesc.WindowSize);
        if (WndDesc.WindowType == TStoreWndType::swtTime) {
            WindowJson->AddToObj("timeField", WndDesc.TimeFieldNm);
        }
        Result->AddToObj("window", WindowJson);
    }
    return Result;
}

/// Save part of the data, given time-window
int TStorePbBlob::PartialFlush(int WndInMsec) {
    DataBlob->PartialFlush(WndInMsec);
    return 0;
}

/// Retrieve performance statistics for this store
PJsonVal TStorePbBlob::GetStats() {
    PJsonVal res = TJsonVal::NewObj();
    res->AddToObj("name", GetStoreNm());
    res->AddToObj("blob_storage", DataBlob->GetStats());
    res->AddToObj("mem_storage", DataMem->GetStats());
    return res;
}

/// Purge records that fall out of store window (when it has one)
void TStorePbBlob::GarbageCollect() {
    // if no window, nothing to do here
    if (WndDesc.WindowType == swtNone) { return; }
    // if no records, nothing to do here
    if (Empty()) { return; }
    // TODO find records to delete
}

/// Perform defragmentation
void TStorePbBlob::Defrag() {
    // TODO merge pages
    // TODO remove empty pages
}


/// Deletes all records
void TStorePbBlob::DeleteAllRecs() {
    // if no records, nothing to do here
    if (Empty()) { return; }
    TEnv::Logger->OnStatusFmt("Deleting all (%d) records in %s", GetRecs(), GetStoreNm().CStr());

    // delete records from index
    //for (uint64 DelRecId = GetFirstRecId(); DelRecId <= GetLastRecId(); DelRecId++) {
    THash<TUInt64, TPgBlobPt>* Target = (DataMemP ? &RecIdBlobPtHMem : &RecIdBlobPtH);
    for (auto it = Target->begin(); it != Target->end(); ++it) {
        uint64 DelRecId = it.GetKey();
        // executed triggers before deletion
        OnDelete(DelRecId);
        // delete record from name-id map
        if (IsPrimaryField()) { DelPrimaryField(DelRecId); }
        // delete record from indexes
        if (DataBlobP) {
            TPgBlobPt Pt = RecIdBlobPtH.GetDat(DelRecId);
            TMemBase CacheRecMem = DataBlob->GetMemBase(Pt);
            RecIndexer.DeindexRec(CacheRecMem, DelRecId, *SerializatorCache);
            //DataBlob->Del(Pt);
            RecIdBlobPtH.DelKey(DelRecId);
        }
        if (DataMemP) {
            TPgBlobPt Pt = RecIdBlobPtHMem.GetDat(DelRecId);
            TMemBase RecMem = DataMem->GetMemBase(Pt);
            RecIndexer.DeindexRec(RecMem, DelRecId, *SerializatorMem);
            //DataMem->Del(Pt);
            RecIdBlobPtHMem.DelKey(DelRecId);
        }
        // delete record from joins
        TRec Rec(this, DelRecId);
        for (int JoinN = 0; JoinN < GetJoins(); JoinN++) {
            TJoinDesc JoinDesc = GetJoinDesc(JoinN);
            // execute the join
            PRecSet JoinRecSet = Rec.DoJoin(GetBase(), JoinDesc.GetJoinId());
            for (int JoinRecN = 0; JoinRecN < JoinRecSet->GetRecs(); JoinRecN++) {
                // remove joins with all matched records, one by one
                const uint64 JoinRecId = JoinRecSet->GetRecId(JoinRecN);
                const int JoinFq = JoinRecSet->GetRecFq(JoinRecN);
                DelJoin(JoinDesc.GetJoinId(), DelRecId, JoinRecId, JoinFq);
            }
        }
    }
    // delete records from disk
    TEnv::Logger->OnStatus("Internal structures 1");
    PrimaryStrIdH.Clr();
    PrimaryIntIdH.Clr();
    PrimaryUInt64IdH.Clr();
    PrimaryFltIdH.Clr();
    PrimaryTmMSecsIdH.Clr();

    TEnv::Logger->OnStatus("Internal structures 2");
    RecIdBlobPtH.Clr();
    RecIdBlobPtHMem.Clr();
    DataBlob->Clr();
    DataMem->Clr();
    PartialFlush(TInt::Mx);
}


void TStorePbBlob::DeleteFirstNRecs(int Recs) {
    PRecSet RecSet = GetAllRecs();
    int RecCnt = RecSet->GetRecs();
    if (RecCnt <= 0) {
        return;
    }
    TUInt64V RecIds(RecCnt, 0);
    for (int i = 0; i < RecCnt; i++) {
        RecIds.Add(RecSet->GetRecId(i));
    }
    RecIds.Sort();
    if (RecIds.Len()>Recs) {
        RecIds.Del(Recs, RecIds.Len() - 1);
    }
    DeleteRecs(RecIds);
}

void TStorePbBlob::DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK) {
    if (AssertOK) {
        // assert that DelRecIdV is valid
        THash<TUInt64, TPgBlobPt>* Ht = (DataMemP ? &RecIdBlobPtHMem : &RecIdBlobPtH);
        for (int i = 0; i < DelRecIdV.Len(); i++) {
            QmAssertR(Ht->IsKey(DelRecIdV[i]), "TStorePbBlob::DeleteRecs - incorrect record id. Record with specified ID not found.");
        }
    }
    // delete records
    for (int DelRecN = 0; DelRecN < DelRecIdV.Len(); DelRecN++) {
        // report progress
        if (DelRecN % 1000 == 0) { TEnv::Logger->OnStatusFmt("    %d\r", DelRecN); }
        // what are we deleting now
        const uint64 DelRecId = DelRecIdV[DelRecN];
        // execute triggers before deletion
        OnDelete(DelRecId);
        // delete record from name-id map
        if (IsPrimaryField()) { DelPrimaryField(DelRecId); }
        // delete record from indexes
        if (DataBlobP) {
            TPgBlobPt Pt = RecIdBlobPtH.GetDat(DelRecId);
            TMemBase CacheRecMem = DataBlob->GetMemBase(Pt);
            RecIndexer.DeindexRec(CacheRecMem, DelRecId, *SerializatorCache);
            DataBlob->Del(Pt);
            RecIdBlobPtH.DelKey(DelRecId);
        }
        if (DataMemP) {
            TPgBlobPt Pt = RecIdBlobPtHMem.GetDat(DelRecId);
            TMemBase RecMem = DataMem->GetMemBase(Pt);
            RecIndexer.DeindexRec(RecMem, DelRecId, *SerializatorMem);
            DataMem->Del(Pt);
            RecIdBlobPtHMem.DelKey(DelRecId);
        }
        // delete record from joins
        TRec Rec(this, DelRecId);
        for (int JoinN = 0; JoinN < GetJoins(); JoinN++) {
            TJoinDesc JoinDesc = GetJoinDesc(JoinN);
            // execute the join
            PRecSet JoinRecSet = Rec.DoJoin(GetBase(), JoinDesc.GetJoinId());
            for (int JoinRecN = 0; JoinRecN < JoinRecSet->GetRecs(); JoinRecN++) {
                // remove joins with all matched records, one by one
                const uint64 JoinRecId = JoinRecSet->GetRecId(JoinRecN);
                const int JoinFq = JoinRecSet->GetRecFq(JoinRecN);
                DelJoin(JoinDesc.GetJoinId(), DelRecId, JoinRecId, JoinFq);
            }
        }
    }

    // report success :-)
    TEnv::Logger->OnStatusFmt("  %s records at end", TUInt64::GetStr(GetRecs()).CStr());
}

/// Initialize field location flags
void TStorePbBlob::InitDataFlags() {
    // go over all the fields and remember if we use in-memory or blob storage
    DataBlobP = false;
    DataMemP = false;
    for (int FieldId = 0; FieldId < GetFields(); FieldId++) {
        DataBlobP = DataBlobP || (FieldLocV[FieldId] == slDisk);
        DataMemP = DataMemP || (FieldLocV[FieldId] == slMemory);
    }
    // at least one must be true, otherwise we have no fields, which is not good
    EAssert(DataBlobP || DataMemP);
}

/// Initialize from given store schema
void TStorePbBlob::InitFromSchema(const TStoreSchema& StoreSchema) {
    // at start there is no primary key
    RecNmFieldP = false;
    PrimaryFieldId = -1;
    PrimaryFieldType = oftUndef;
    // create fields
    for (int i = 0; i < StoreSchema.FieldH.Len(); i++) {
        const TFieldDesc& FieldDesc = StoreSchema.FieldH[i];
        AddFieldDesc(FieldDesc);
        // check if we found a primary field
        if (FieldDesc.IsPrimary()) {
            QmAssertR(PrimaryFieldId == -1, "Store can have only one primary field");
            // only string fields can serve as record name (TODO: extend)
            RecNmFieldP = FieldDesc.IsStr();
            PrimaryFieldId = GetFieldId(FieldDesc.GetFieldNm());
            PrimaryFieldType = FieldDesc.GetFieldType();
        }
    }
    // create index keys
    TWPt<TIndexVoc> IndexVoc = GetIndex()->GetIndexVoc();
    for (int IndexKeyExN = 0; IndexKeyExN < StoreSchema.IndexKeyExV.Len(); IndexKeyExN++) {
        TIndexKeyEx IndexKeyEx = StoreSchema.IndexKeyExV[IndexKeyExN];
        // get associated field
        const int FieldId = GetFieldId(IndexKeyEx.FieldName);
        // if we are given vocabulary name, check if we have one with such name already
        const int WordVocId = GetBase()->NewIndexWordVoc(IndexKeyEx.KeyType, IndexKeyEx.WordVocName);
        // create new index key
        const int KeyId = GetBase()->NewFieldIndexKey(this, IndexKeyEx.KeyIndexName,
            FieldId, WordVocId, IndexKeyEx.KeyType, IndexKeyEx.SortType);
        // assign tokenizer to it if we have one
        if (IndexKeyEx.IsTokenizer()) { IndexVoc->PutTokenizer(KeyId, IndexKeyEx.Tokenizer); }
    }
    // prepare serializators for disk and in-memory store
    SerializatorCache = new TRecSerializator(this, StoreSchema, slDisk);
    SerializatorMem = new TRecSerializator(this, StoreSchema, slMemory);
    // initialize field to storage location map
    InitFieldLocV();
    // initialize record indexer
    RecIndexer = TRecIndexer(GetIndex(), this);
    // remember window parameters
    WndDesc = StoreSchema.WndDesc;
}

/// initialize field storage location map
void TStorePbBlob::InitFieldLocV() {
    for (int FieldId = 0; FieldId < GetFields(); FieldId++) {
        if (SerializatorCache->IsFieldId(FieldId)) {
            FieldLocV.Add(slDisk);
        } else if (SerializatorMem->IsFieldId(FieldId)) {
            FieldLocV.Add(slMemory);
        } else {
            throw TQmExcept::New("Unknown storage location for field " +
                GetFieldNm(FieldId) + " in store " + GetStoreNm());
        }
    }
}

///////////////////////////////////////////////////////////

TStorePbBlob::TStorePbBlob(const TWPt<TBase>& Base, const uint& StoreId,
    const TStr& StoreName, const TStoreSchema& StoreSchema, const TStr& _StoreFNm,
    const int64& _MxCacheSize, const int& BlockSize) :
    TStore(Base, StoreId, StoreName), StoreFNm(_StoreFNm), FAccess(faCreate) {

    SetStoreType("TStorePbBlob");
    DataBlob = new TPgBlob(_StoreFNm + "PgBlob", TFAccess::faCreate, _MxCacheSize);
    DataMem = new TPgBlob(_StoreFNm + "PgBlobMem", TFAccess::faCreate, TUInt64::Mx);
    InitFromSchema(StoreSchema);
    InitDataFlags();
}

TStorePbBlob::TStorePbBlob(const TWPt<TBase>& Base, const TStr& _StoreFNm,
    const TFAccess& _FAccess, const int64& _MxCacheSize,
    const bool& _Lazy) :
    TStore(Base, _StoreFNm + ".BaseStore"),
    StoreFNm(_StoreFNm), FAccess(_FAccess), PrimaryFieldType(oftUndef) {

    SetStoreType("TStorePbBlob");
    DataBlob = new TPgBlob(_StoreFNm + "PgBlob", _FAccess, _MxCacheSize);
    DataMem = new TPgBlob(_StoreFNm + "PgBlobMem", _FAccess, TUInt64::Mx);
    if (!_Lazy) {
        DataMem->LoadAll();
    }

    // load members
    TFIn FIn(StoreFNm + "PgBlobStore");
    RecNmFieldP.Load(FIn);
    PrimaryFieldId.Load(FIn);
    // deduce primary field type
    if (PrimaryFieldId != -1) {
        PrimaryFieldType = GetFieldDesc(PrimaryFieldId).GetFieldType();
        if (PrimaryFieldType == oftStr) {
            PrimaryStrIdH.Load(FIn);
        } else if (PrimaryFieldType == oftInt) {
            PrimaryIntIdH.Load(FIn);
        } else if (PrimaryFieldType == oftUInt64) {
            PrimaryUInt64IdH.Load(FIn);
        } else if (PrimaryFieldType == oftFlt) {
            PrimaryFltIdH.Load(FIn);
        } else if (PrimaryFieldType == oftTm) {
            PrimaryTmMSecsIdH.Load(FIn);
        } else {
            throw TQmExcept::New("Unsupported primary field type!");
        }
    } else {
        // backwards compatibility
        PrimaryStrIdH.Load(FIn);
    }
    // load time window    
    WndDesc.Load(FIn);
    // load data
    SerializatorCache = new TRecSerializator(this);
    SerializatorMem = new TRecSerializator(this);
    SerializatorCache->Load(FIn);
    SerializatorMem->Load(FIn);
    RecIdBlobPtH.Load(FIn);
    RecIdBlobPtHMem.Load(FIn);
    RecIdCounter.Load(FIn);

    // initialize field to storage location map
    InitFieldLocV();
    // initialize record indexer
    RecIndexer = TRecIndexer(GetIndex(), this);

    // initialize data storage flags
    InitDataFlags();
}

TStorePbBlob::~TStorePbBlob() {
    // save if necessary
    if (FAccess != faRdOnly) {
        TEnv::Logger->OnStatus(TStr::Fmt("Saving store '%s'...", GetStoreNm().CStr()));
        // save base store
        TFOut BaseFOut(StoreFNm + ".BaseStore");
        SaveStore(BaseFOut);
        // save store parameters
        TFOut FOut(StoreFNm + "PgBlobStore");
        // save parameters about primary field
        RecNmFieldP.Save(FOut);
        PrimaryFieldId.Save(FOut);
        if (PrimaryFieldType == oftInt) {
            PrimaryIntIdH.Save(FOut);
        } else if (PrimaryFieldType == oftUInt64) {
            PrimaryUInt64IdH.Save(FOut);
        } else if (PrimaryFieldType == oftFlt) {
            PrimaryFltIdH.Save(FOut);
        } else if (PrimaryFieldType == oftTm) {
            PrimaryTmMSecsIdH.Save(FOut);
        } else {
            PrimaryStrIdH.Save(FOut);
        }
        // save time window
        WndDesc.Save(FOut);
        // save data
        SerializatorCache->Save(FOut);
        SerializatorMem->Save(FOut);

        RecIdBlobPtH.Save(FOut);
        RecIdBlobPtHMem.Save(FOut);
        RecIdCounter.Save(FOut);

    } else {
        TEnv::Logger->OnStatus("No saving of generic store " + GetStoreNm() + " neccessary!");
    }
}

/// Store value into internal storage using TOAST method
TPgBlobPt TStorePbBlob::ToastVal(const TMemBase& Mem) { 
    TVec<TPgBlobPt> Pts;
    int BlockLen = DataBlob->GetMxBlobLen();
    int curr_index = 0;		
    while (curr_index < Mem.Len()) {
        int curr_len = MIN(BlockLen, Mem.Len() - curr_index);
        TPgBlobPt PtTmp = DataBlob->Put(Mem.GetBf() + curr_index, curr_len);
        Pts.Add(PtTmp);
        curr_index += curr_len;
    }
    TMOut SOut;
    Pts.Save(SOut);
    return DataBlob->Put(SOut.GetBfAddr(), SOut.Len());
}

/// Retrieve value that is saved using TOAST method from storage 
void TStorePbBlob::UnToastVal(const TPgBlobPt& Pt, TMem& Mem) {
    TVec<TPgBlobPt> Pts;
    TThinMIn MIn = DataBlob->Get(Pt);
    Pts.Load(MIn);
    Mem.Clr();
    for (int i = 0; i < Pts.Len(); i++) {
        TMemBase MemTmp = DataBlob->GetMemBase(Pts[i]);
        Mem.AddBf(MemTmp.GetBf(), MemTmp.Len());
    }
}

/// Delete TOAST-ed value from storage 
void TStorePbBlob::DelToastVal(const TPgBlobPt& Pt) {
    TVec<TPgBlobPt> Pts;
    TThinMIn MIn = DataBlob->Get(Pt);
    Pts.Load(MIn);
    for (int i = 0; i < Pts.Len(); i++) {
        DataBlob->Del(Pts[i]);
    }
    DataBlob->Del(Pt);
}

///////////////////////////////
/// Create new stores in an existing base from a schema definition
TVec<TWPt<TStore> > CreateStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal,
    const uint64& DefStoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH, bool UsePaged) {

    // parse and validate the schema
    InfoLog("Parsing schema");
    TStoreSchemaV SchemaV; TStoreSchema::ParseSchema(SchemaVal, SchemaV);
    TStoreSchema::ValidateSchema(Base, SchemaV);

    // create stores	
    TVec<TWPt<TStore> > NewStoreV;
    for (int SchemaN = 0; SchemaN < SchemaV.Len(); SchemaN++) {
        TStoreSchema& StoreSchema = SchemaV[SchemaN];
        TStr StoreNm = StoreSchema.StoreName;
        InfoLog("Creating " + StoreNm);
        // figure out store id
        uint StoreId = 0;
        if (StoreSchema.HasStoreIdP) {
            StoreId = StoreSchema.StoreId;
            // check if we already have store with same ID
            QmAssertR(!Base->IsStoreId(StoreId), "Store id for " + StoreNm + " already in use.");
        } else {
            // find lowest unused StoreId
            while (Base->IsStoreId(StoreId)) {
                StoreId++;
                QmAssertR(StoreId < TEnv::GetMxStores(), "Out of store Ids -- to many stores!");
            }
        }
        // get cache size for the store
        const uint64 StoreCacheSize = StoreNmCacheSizeH.IsKey(StoreNm) ?
            StoreNmCacheSizeH.GetDat(StoreNm).Val : DefStoreCacheSize;
        // create new store from the schema
        PStore Store;
        if (UsePaged && StoreSchema.StoreType == "paged") {
            Store = new TStorePbBlob(Base, StoreId, StoreNm,
                StoreSchema, Base->GetFPath() + StoreNm, StoreCacheSize, StoreSchema.BlockSizeMem);
        } else {
            Store = new TStoreImpl(Base, StoreId, StoreNm,
                StoreSchema, Base->GetFPath() + StoreNm, StoreCacheSize,
                StoreSchema.BlockSizeMem);
        }
        // add store to base
        Base->AddStore(Store);
        // remember we create the store
        NewStoreV.Add(Store);
    }

    // Create joins
    InfoLog("Creating joins");
    for (int SchemaN = 0; SchemaN < SchemaV.Len(); SchemaN++) {
        // get store
        TStoreSchema StoreSchema = SchemaV[SchemaN];
        TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreSchema.StoreName);
        // go over all outgoing joins
        for (int JoinDescExN = 0; JoinDescExN < StoreSchema.JoinDescExV.Len(); JoinDescExN++) {
            TJoinDescEx& JoinDescEx = StoreSchema.JoinDescExV[JoinDescExN];
            // get join store
            TWPt<TStore> JoinStore = Base->GetStoreByStoreNm(JoinDescEx.JoinStoreName);
            // check join type
            if (JoinDescEx.JoinType == osjtField) {
                // field join
                int JoinRecFieldId = Store->GetFieldId(JoinDescEx.JoinName + "Id");
                int JoinFqFieldId = Store->GetFieldId(JoinDescEx.JoinName + "Fq");
                Store->AddJoinDesc(TJoinDesc(JoinDescEx.JoinName,
                    JoinStore->GetStoreId(), JoinRecFieldId, JoinFqFieldId));
            } else if (JoinDescEx.JoinType == osjtIndex) {
                // index join
                Store->AddJoinDesc(TJoinDesc(JoinDescEx.JoinName,
                    JoinStore->GetStoreId(), Store->GetStoreId(),
                    Base->GetIndexVoc(), JoinDescEx.IsSmall));
            } else {
                ErrorLog("Unknown join type for join " + JoinDescEx.JoinName);
            }
        }
    }

    // Update inverse joins IDs
    InfoLog("Updating inverse join maps");
    for (int SchemaN = 0; SchemaN < SchemaV.Len(); SchemaN++) {
        // get store
        TStoreSchema StoreSchema = SchemaV[SchemaN];
        TWPt<TStore> Store = Base->GetStoreByStoreNm(StoreSchema.StoreName);
        // go over outgoing joins
        for (int JoinDescExN = 0; JoinDescExN < StoreSchema.JoinDescExV.Len(); JoinDescExN++) {
            // check if we have inverse join
            TJoinDescEx& JoinDescEx = StoreSchema.JoinDescExV[JoinDescExN];
            if (!JoinDescEx.InverseJoinName.Empty()) {
                // we do, get inverse join id
                const int JoinId = Store->GetJoinId(JoinDescEx.JoinName);
                const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
                TWPt<TStore> JoinStore = Base->GetStoreByStoreId(JoinDesc.GetJoinStoreId());
                QmAssertR(JoinStore->IsJoinNm(JoinDescEx.InverseJoinName),
                    "Invalid inverse join " + JoinDescEx.InverseJoinName);
                const int InverseJoinId = JoinStore->GetJoinId(JoinDescEx.InverseJoinName);
                // mark the map
                Store->PutInverseJoinId(JoinId, InverseJoinId);
            }
        }
    }

    // done
    return NewStoreV;
}

///////////////////////////////
/// Create new base given a schema definition
TWPt<TBase> NewBase(const TStr& FPath, const PJsonVal& SchemaVal, const uint64& IndexCacheSize,
    const uint64& DefStoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH, const bool& InitP,
    const int& SplitLen, bool UsePaged) {

    // create empty base
    InfoLog("Creating new base from schema");
    TWPt<TBase> Base = TBase::New(FPath, IndexCacheSize, SplitLen);
    // parse and apply the schema
    CreateStoresFromSchema(Base, SchemaVal, DefStoreCacheSize, StoreNmCacheSizeH, UsePaged);
    // finish base initialization if so required (default is true)
    if (InitP) { Base->Init(); }
    // done
    return Base;
}


///////////////////////////////
/// Load base created from a schema definition
TWPt<TBase> LoadBase(const TStr& FPath, const TFAccess& FAccess, const uint64& IndexCacheSize,
    const uint64& DefStoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH, const bool& InitP,
    const int& SplitLen) {

    InfoLog("Loading base created from schema definition");
    TWPt<TBase> Base = TBase::Load(FPath, FAccess, IndexCacheSize, SplitLen);
    // load stores
    InfoLog("Loading stores");
    // read store names from file
    TFIn FIn(FPath + "StoreList.json");
    TStr jsons;
    FIn.GetNextLn(jsons);
    PJsonVal json = TJsonVal::GetValFromStr(jsons);
    QmAssert(!json->IsNull() && json->IsObjKey("stores"));
    PJsonVal json_stores = json->GetObjKey("stores");
    for (int i = 0; i < json_stores->GetArrVals(); i++) {
        PJsonVal json_store = json_stores->GetArrVal(i);
        TStr StoreNm = json_store->GetObjStr("name");
        TStr StoreType = json_store->GetObjStr("type");
        InfoLog("  " + StoreNm);
        // get cache size for the store
        const uint64 StoreCacheSize = StoreNmCacheSizeH.IsKey(StoreNm) ?
            StoreNmCacheSizeH.GetDat(StoreNm).Val : DefStoreCacheSize;
        PStore Store;
        if (StoreType == "TStorePbBlob") {
            Store = new TStorePbBlob(Base, FPath + StoreNm, FAccess, StoreCacheSize);
        } else {
            Store = new TStoreImpl(Base, FPath + StoreNm, FAccess, StoreCacheSize);
        }
        Base->AddStore(Store);
    }
    InfoLog("Stores loaded");
    // finish base initialization if so required (default is true)
    if (InitP) { Base->Init(); }
    // done
    return Base;
}

///////////////////////////////
/// Save base created from a schema definition
void SaveBase(const TWPt<TBase>& Base) {
    if (Base->IsRdOnly()) {
        InfoLog("No saving of generic base necessary!");
    } else {
        // Only need to save list of stores so we know what to load next time
        // Everything else is saved automatically in destructor
        InfoLog("Saving list of stores ... ");
        PJsonVal stores = TJsonVal::NewArr();
        for (int StoreN = 0; StoreN < Base->GetStores(); StoreN++) {
            PStore store_obj = Base->GetStoreByStoreN(StoreN);
            PJsonVal store = TJsonVal::NewObj();
            store->AddToObj("name", store_obj->GetStoreNm());
            store->AddToObj("type", store_obj->GetStoreType());
            stores->AddToArr(store);
        }
        PJsonVal root = TJsonVal::NewObj();
        root->AddToObj("stores", stores);

        TFOut FOut(Base->GetFPath() + "StoreList.json");
        FOut.PutStr(TJsonVal::GetStrFromVal(root));
    }
}

}

}
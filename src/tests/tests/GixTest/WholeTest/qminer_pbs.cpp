
#include "qminer_pbs.h"

namespace TQm {

	using namespace TQm::TStorage;
	using namespace glib;

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
		uint64 MemRecId = RecIdCounter++;
		// store to disk storage
		if (DataBlobP) {
			TMem CacheRecMem; 
			SerializatorCache.Serialize(RecVal, CacheRecMem, this);
			TPgBlobPt Pt = DataBlob->Put((byte*)CacheRecMem.GetBf(), CacheRecMem.Len());
			CacheRecId = Pt;
			RecIdBlobPtH.AddDat(RecIdCounter) = Pt;
			// index new record
			RecIndexer.IndexRec(CacheRecMem, RecIdCounter, SerializatorCache);
		}
		// store to in-memory storage
		if (DataMemP) {
			TMem MemRecMem; 
			SerializatorMem.Serialize(RecVal, MemRecMem, this);
			MemRecId = DataMem.AddVal(MemRecMem);
			// index new record
			RecIndexer.IndexRec(MemRecMem, MemRecId, SerializatorMem);
		}
		// make sure we are consistent with respect to Ids!
		if (DataBlobP && DataMemP) {
			EAssert(MemRecId == RecIdCounter - 1);
		}

		// remember value-recordId map when primary field available
		if (IsPrimaryField()) { SetPrimaryField(RecIdCounter); }

		// insert nested join records
		AddJoinRec(RecIdCounter, RecVal);
		// call add triggers
		OnAdd(RecIdCounter);

		// return record Id of the new record
		return RecIdCounter;
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
						(FieldLocV[FieldId] == slDisk && !SerializatorCache.IsInFixedPart(FieldId));
					MemVarP = MemVarP || 
						(FieldLocV[FieldId] == slMemory  && !SerializatorMem.IsInFixedPart(FieldId));
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

				SerializatorCache.SerializeUpdate(RecVal, CacheOldRecMem,
					CacheNewRecMem, this, CacheChangedFieldIdSet);

				// update the stored serializations with new values
				Pt = DataBlob->Put((byte*)Mem.GetBf(), Mem.Len(), Pt);
				RecIdBlobPtH(RecId) = Pt;
				// update indexes pointing to the record
				RecIndexer.UpdateRec(CacheOldRecMem, CacheNewRecMem, RecId, CacheChangedFieldIdSet, SerializatorCache);
			} else {
				// nice, all changes can be done in-place, no index changes
				SerializatorCache.SerializeUpdateInPlace(RecVal, MIn, this, CacheChangedFieldIdSet);
				DataBlob->SetDirty(Pt);
			}
		}
		// update in-memory serialization when necessary
		if (MemP) {
			// update serialization
			TMem MemOldRecMem; 
			DataMem.GetVal(RecId, MemOldRecMem);
			TIntSet MemChangedFieldIdSet;
			if (MemVarP || KeyP) {
				// variable fields changed, so we need to serialize whole record
				TMem MemNewRecMem;
				SerializatorMem.SerializeUpdate(RecVal, MemOldRecMem,
					MemNewRecMem, this, MemChangedFieldIdSet);
				// update the stored serializations with new values
				DataMem.SetVal(RecId, MemNewRecMem);
				// update indexes pointing to the record
				RecIndexer.UpdateRec(MemOldRecMem, MemNewRecMem, RecId, MemChangedFieldIdSet, SerializatorMem);
			} else {
				// nice, all changes can be done in-place
				TThinMIn MIn(MemOldRecMem.GetBf(), MemOldRecMem.Len());
				SerializatorCache.SerializeUpdateInPlace(RecVal, MIn, this, MemChangedFieldIdSet);
			}
		}
		// check if primary key changed and update the mapping
		if (PrimaryP) { SetPrimaryField(RecId); }
		// call update triggers
		OnUpdate(RecId);
	}

	//////////////////////////////////////////////////////////

	/// Load page with with given record and return pointer to it
	TThinMIn TStorePbBlob::GetPgBf(const uint64& RecId) const {
		const TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
		TThinMIn min = DataBlob->Get(PgPt);
		return min;
	}

	/// Check if the value of given field for a given record is NULL
	bool TStorePbBlob::IsFieldNull(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.IsFieldNull(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.IsFieldNull(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	int TStorePbBlob::GetFieldInt(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldInt(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldInt(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldIntV(GetPgBf(RecId), FieldId, IntV);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldIntV(Rec, FieldId, IntV);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	uint64 TStorePbBlob::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldUInt64(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldUInt64(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	TStr TStorePbBlob::GetFieldStr(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldStr(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldStr(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldStrV(GetPgBf(RecId), FieldId, StrV);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldStrV(Rec, FieldId, StrV);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	bool TStorePbBlob::GetFieldBool(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldBool(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldBool(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	double TStorePbBlob::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldFlt(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldFlt(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	TFltPr TStorePbBlob::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldFltPr(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldFltPr(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldFltV(GetPgBf(RecId), FieldId, FltV);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldFltV(Rec, FieldId, FltV);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldTm(GetPgBf(RecId), FieldId, Tm);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldTm(Rec, FieldId, Tm);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	uint64 TStorePbBlob::GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			return SerializatorCache.GetFieldTmMSecs(GetPgBf(RecId), FieldId);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			return SerializatorMem.GetFieldTmMSecs(Rec, FieldId);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldNumSpV(GetPgBf(RecId), FieldId, SpV);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldNumSpV(Rec, FieldId, SpV);
		}
	}
	/// Get field value using field id (default implementation throws exception)
	void TStorePbBlob::GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			SerializatorCache.GetFieldBowSpV(GetPgBf(RecId), FieldId, SpV);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			SerializatorMem.GetFieldBowSpV(Rec, FieldId, SpV);
		}
	}

	//////////////////////

	/// Set the value of given field to NULL
	void TStorePbBlob::SetFieldNull(const uint64& RecId, const int& FieldId) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldNull(min.GetBfAddr(), min.Len(), FieldId, true);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldNull(Rec, OutRecMem, FieldId);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldInt(min.GetBfAddr(), min.Len(), FieldId, Int);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldInt(Rec, OutRecMem, FieldId, Int);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			TMem mem_in(min);
			TMem mem_out;
			SerializatorCache.SetFieldIntV(mem_in, mem_out, FieldId, IntV);
			RecIdBlobPtH.GetDat(RecId) = DataBlob->Put((byte*)mem_out.GetBf(), mem_out.Len(), PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldIntV(Rec, OutRecMem, FieldId, IntV);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldUInt64(min.GetBfAddr(), min.Len(), FieldId, UInt64);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldUInt64(Rec, OutRecMem, FieldId, UInt64);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldStr(min.GetBfAddr(), min.Len(), FieldId, Str);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldStr(Rec, OutRecMem, FieldId, Str);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			TMem mem_in(min);
			TMem mem_out;
			SerializatorCache.SetFieldStrV(mem_in, mem_out, FieldId, StrV);
			RecIdBlobPtH.GetDat(RecId) = DataBlob->Put((byte*)mem_out.GetBf(), mem_out.Len(), PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldStrV(Rec, OutRecMem, FieldId, StrV);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldBool(min.GetBfAddr(), min.Len(), FieldId, Bool);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldBool(Rec, OutRecMem, FieldId, Bool);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldFlt(min.GetBfAddr(), min.Len(), FieldId, Flt);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldFlt(Rec, OutRecMem, FieldId, Flt);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldFltPr(min.GetBfAddr(), min.Len(), FieldId, FltPr);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldFltPr(Rec, OutRecMem, FieldId, FltPr);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			TMem mem_in(min);
			TMem mem_out;
			SerializatorCache.SetFieldFltV(mem_in, mem_out, FieldId, FltV);
			RecIdBlobPtH.GetDat(RecId) = DataBlob->Put((byte*)mem_out.GetBf(), mem_out.Len(), PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldFltV(Rec, OutRecMem, FieldId, FltV);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldTm(min.GetBfAddr(), min.Len(), FieldId, Tm);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldTm(Rec, OutRecMem, FieldId, Tm);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			SerializatorCache.SetFieldTmMSecs(min.GetBfAddr(), min.Len(), FieldId, TmMSecs);
			DataBlob->SetDirty(PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldTmMSecs(Rec, OutRecMem, FieldId, TmMSecs);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			TMem mem_in(min);
			TMem mem_out;
			SerializatorCache.SetFieldNumSpV(mem_in, mem_out, FieldId, SpV);
			RecIdBlobPtH.GetDat(RecId) = DataBlob->Put((byte*)mem_out.GetBf(), mem_out.Len(), PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldNumSpV(Rec, OutRecMem, FieldId, SpV);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
		}
	}
	/// Set field value using field id (default implementation throws exception)
	void TStorePbBlob::SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV) {
		if (FieldLocV[FieldId] == TStoreLoc::slDisk) {
			TPgBlobPt& PgPt = RecIdBlobPtH.GetDat(RecId);
			TThinMIn min = DataBlob->Get(PgPt);
			TMem mem_in(min);
			TMem mem_out;
			SerializatorCache.SetFieldBowSpV(mem_in, mem_out, FieldId, SpV);
			RecIdBlobPtH.GetDat(RecId) = DataBlob->Put((byte*)mem_out.GetBf(), mem_out.Len(), PgPt);
		} else {
			TMem Rec;
			DataMem.GetVal(RecId, Rec);
			TMem OutRecMem;
			SerializatorMem.SetFieldBowSpV(Rec, OutRecMem, FieldId, SpV);
			RecIndexer.UpdateRec(Rec, OutRecMem, RecId, FieldId, SerializatorMem);
			DataMem.SetVal(RecId, Rec);
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
		return RecIdBlobPtH.Len();
	}

	/// Return iterator over store
	PStoreIter TStorePbBlob::GetIter() const {
		if (Empty()) { return TStoreIterVec::New(); }
		return DataMemP ?
			TStoreIterVec::New(DataMem.GetFirstValId(), DataMem.GetLastValId(), true) :
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
		res->AddToObj("storage", DataBlob->GetStats());
		return res;
	}

	/// Purge records that fall out of store window (when it has one)
	void TStorePbBlob::GarbageCollect() {
		// if no window, nothing to do here
		if (WndDesc.WindowType == swtNone) { return; }
		// if no records, nothing to do here
		if (Empty()) { return; }
		// TODO
	}

	void TStorePbBlob::DeleteFirstNRecs(int Recs) {
		// TODO
	}

	void TStorePbBlob::DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK) {
		EAssert(AssertOK);
		// TODO
	}
}
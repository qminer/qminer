/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef QMINER_PBS_H
#define QMINER_PBS_H

#include "qminer_gs.h"

namespace TQm {
namespace TStorage {

	///////////////////////////////
	/// Implementation of store which can be initialized from a schema.
	/// It also uses Paged-BLOB storage engine.
	class TStorePbBlob : public TStore {
	private:

		/// For temporarily storing inverse joins which need to be 
		/// indexed after adding records
		struct TFieldJoinDat {
			TWPt<TStore> JoinStore;
			TInt InverseJoinId;
			TUInt64 JoinRecId;
			TInt JoinFq;
		};

	private:
		/// Store filename
		TStr StoreFNm;
		/// Open mode
		TFAccess FAccess;

		/// Do we have a primary field which can act as record name
		TBool RecNmFieldP;
		/// Id of primary field (-1 if not defined)
		TInt PrimaryFieldId;
		/// Type of primary field
		TFieldType PrimaryFieldType;
		/// Hash map from TStr primary field to record ID
		THash<TStr, TUInt64> PrimaryStrIdH;
		/// Hash map from TInt primary field to record ID
		THash<TInt, TUInt64> PrimaryIntIdH;
		/// Hash map from TUInt64 primary field to record ID
		THash<TUInt64, TUInt64> PrimaryUInt64IdH;
		/// Hash map from TFlt primary field to record ID
		THash<TFlt, TUInt64> PrimaryFltIdH;
		/// Hash map from TTm primary field to record ID
		THash<TUInt64, TUInt64> PrimaryTmMSecsIdH;


		/// Flag if we are using cache store
		TBool DataBlobP;
		/// Store for records
		PPgBlob DataBlob;
		/// Hash map from record ID to BLOB pointer
		THash<TUInt64, TPgBlobPt> RecIdBlobPtH;
		/// Hash map from record ID to BLOB pointer
		THash<TUInt64, TPgBlobPt> RecIdBlobPtHMem;
		/// Flag if we are using in-memory store
		TBool DataMemP;
		/// Store for parts of records that should be in-memory
		PPgBlob DataMem;

		/// Counter for record IDs
		TUInt64 RecIdCounter;

		/// Serializator to disk
		TRecSerializator *SerializatorCache;
		/// Serializator to memory
		TRecSerializator *SerializatorMem;
		/// Map from fields to storage location
		TVec<TStoreLoc> FieldLocV;

		// record indexer
		TRecIndexer RecIndexer;
		/// Time window settings
		TStoreWndDesc WndDesc;

		/// initialize field storage location map
		void InitFieldLocV();

		/// Load page with with given record and return pointer to it
		TThinMIn GetPgBf(const uint64& RecId, const bool& UseMem = false) const;


		/// Get serializator for given location
		TRecSerializator* GetSerializator(const TStoreLoc& StoreLoc);
		/// Get serializator for given location
		TRecSerializator* GetSerializator(const TStoreLoc& StoreLoc) const;
		/// Get serializator for given field
		TRecSerializator& GetFieldSerializator(const int &FieldId);
		/// Get serializator for given field
		const TRecSerializator& GetFieldSerializator(const int &FieldId) const;
		/// Remove record from name-id map
		void DelRecNm(const uint64& RecId);
		/// Do we have a primary field
		bool IsPrimaryField() const { return PrimaryFieldId != -1; }
		/// Set primary field map
		void SetPrimaryField(const uint64& RecId);
		/// Delete primary field map
		void DelPrimaryField(const uint64& RecId);
		/// Transform Join name to it's corresponding field name
		TStr GetJoinFieldNm(const TStr& JoinNm) const { return JoinNm + "Id"; }

		/// Initialize from given store schema
		void InitFromSchema(const TStoreSchema& StoreSchema);
		/// Initialize field location flags
		void InitDataFlags();

	public:
		TStorePbBlob(const TWPt<TBase>& _Base, const uint& StoreId,
			const TStr& StoreName, const TStoreSchema& StoreSchema,
			const TStr& _StoreFNm, const int64& _MxCacheSize, const int& BlockSize);
		TStorePbBlob(const TWPt<TBase>& _Base, const TStr& _StoreFNm,
			const TFAccess& _FAccess, const int64& _MxCacheSize,
			const bool& _Lazy = false);
		// need to override destructor, to clear cache
		~TStorePbBlob();

		/// Check if given ID is valid
		bool IsRecId(const uint64& RecId) const;
		bool HasRecNm() const { return RecNmFieldP; }
		/// Check if record with given name exists
		bool IsRecNm(const TStr& RecNm) const;
		/// Find name of the record with given ID
		TStr GetRecNm(const uint64& RecId) const;
		/// Return ID of record with given name
		uint64 GetRecId(const TStr& RecNm) const;
		/// Get number of record
		uint64 GetRecs() const;
		uint64 GetFirstRecId() const { EFail; return 0; }
		uint64 GetLastRecId() const { EFail; return 0; }

		PStoreIter GetIter() const;

		/// Gets the first record in the store (order defined by store implementation)
		uint64 FirstRecId() const;
		/// Gets the last record in the store (order defined by store implementation)
		uint64 LastRecId() const;
		/// Gets forward moving iterator (order defined by store implementation)
		PStoreIter ForwardIter() const { return GetIter(); }
		/// Gets backward moving iterator (order defined by store implementation)
		PStoreIter BackwardIter() const;

		/// Add new record
		uint64 AddRec(const PJsonVal& RecVal);
		/// Update existing record
		void UpdateRec(const uint64& RecId, const PJsonVal& RecVal);

		/// Purge records that fall out of store window (when it has one)
		void GarbageCollect();
		/// Perform defragmentation
		void Defrag();
		/// Deletes all records
		void DeleteAllRecs();
		void DeleteFirstNRecs(int Recs);
		void DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK = true);

		/// Check if the value of given field for a given record is NULL
		bool IsFieldNull(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		int GetFieldInt(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
		/// Get field value using field id (default implementation throws exception)
		uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
		/// Get field value using field id (default implementation throws exception)
		bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
		/// Get field value using field id (default implementation throws exception)
		uint64 GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const;
		/// Get field value using field id (default implementation throws exception)
		void GetFieldBowSpV(const uint64& RecId, const int& FieldId, PBowSpV& SpV) const;

		/// Set the value of given field to NULL
		void SetFieldNull(const uint64& RecId, const int& FieldId);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldInt(const uint64& RecId, const int& FieldId, const int& Int);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldIntV(const uint64& RecId, const int& FieldId, const TIntV& IntV);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldUInt64(const uint64& RecId, const int& FieldId, const uint64& UInt64);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldStr(const uint64& RecId, const int& FieldId, const TStr& Str);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldStrV(const uint64& RecId, const int& FieldId, const TStrV& StrV);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldBool(const uint64& RecId, const int& FieldId, const bool& Bool);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldFlt(const uint64& RecId, const int& FieldId, const double& Flt);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldFltPr(const uint64& RecId, const int& FieldId, const TFltPr& FltPr);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldFltV(const uint64& RecId, const int& FieldId, const TFltV& FltV);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldTm(const uint64& RecId, const int& FieldId, const TTm& Tm);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldTmMSecs(const uint64& RecId, const int& FieldId, const uint64& TmMSecs);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldNumSpV(const uint64& RecId, const int& FieldId, const TIntFltKdV& SpV);
		/// Set field value using field id (default implementation throws exception)
		void SetFieldBowSpV(const uint64& RecId, const int& FieldId, const PBowSpV& SpV);

		/// Helper function for returning JSon definition of store
		PJsonVal GetStoreJson(const TWPt<TBase>& Base) const;
		
		/// Save part of the data, given time-window
		int PartialFlush(int WndInMsec = 500);
		/// Retrieve performance statistics for this store
		PJsonVal GetStats();

		/// Check if store supports TOAST
		virtual bool CanToast() { return true; }
		/// Return max size of non-TOAST-ed record
		virtual int GetMaxToastLen() { return PAGE_SIZE / 4; }
		/// Store value into internal storage using TOAST method
		virtual TPgBlobPt ToastVal(const TMemBase& Mem);
		/// Retrieve value that is saved using TOAST method from storage 
		virtual void UnToastVal(const TPgBlobPt& Pt, TMem& Mem);
		/// Delete TOAST-ed value from storage 
		virtual void DelToastVal(const TPgBlobPt& Pt);
	};

	///////////////////////////////
	/// Create new base given a schema definition
	TWPt<TBase> NewBase(
		const TStr& FPath, const PJsonVal& SchemaVal, const uint64& IndexCacheSize,
		const uint64& DefStoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH = TStrUInt64H(),
		const bool& InitP = true, const int& SplitLen = TInt::Giga, bool UsePaged = true);

	///////////////////////////////
	/// Create new stores from a schema and add them to an existing base
	TVec<TWPt<TStore> > CreateStoresFromSchema(
		const TWPt<TBase>& Base, const PJsonVal& SchemaVal,
		const uint64& DefStoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH = TStrUInt64H(), bool UsePaged = true);

	///////////////////////////////
	/// Load base created from a schema definition
	TWPt<TBase> LoadBase(const TStr& FPath, const TFAccess& FAccess, const uint64& IndexCacheSize,
		const uint64& StoreCacheSize, const TStrUInt64H& StoreNmCacheSizeH = TStrUInt64H(),
		const bool& InitP = true, const int& SplitLen = TInt::Giga);

	///////////////////////////////
	/// Save base created from a schema definition
	void SaveBase(const TWPt<TBase>& Base);
}
}
#endif
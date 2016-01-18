/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/
#ifndef QMINER_QM_NODEJS_STORE
#define QMINER_QM_NODEJS_STORE

#include <node.h>
#include <node_object_wrap.h>
#include <qminer.h>
#include "../nodeutil.h"

namespace TQm {
	namespace TStorage {

		class TNodeJsFuncStore : public TStoreNotImpl {
		private:
			v8::Persistent<v8::Function> GetRecsFun;
			v8::Persistent<v8::Function> GetFieldFun;

			v8::Persistent<v8::Function> GetRecNmFun;

			v8::Persistent<v8::Function> GetFirstRecIdFun;
			v8::Persistent<v8::Function> GetLastRecIdFun;

		public:
			v8::Persistent<v8::Function> GetAllRecsFun;

		private:
			/// Initialize from given store schema
			void InitFromSchema(const TStoreSchema& StoreSchema);
			/// Helper function
			void SetCallback(const v8::Handle<v8::Value>& CallbacksObj, v8::Persistent<v8::Function>& Callback, const TStr& Name);
			/// Sets all callbacks
			void InitCallbacks(const v8::Handle<v8::Value>& CallbacksObj);
			/// Get field helper
			v8::Handle<v8::Value> GetField(const uint64& RecId, const int& FieldId) const;
		public:
			~TNodeJsFuncStore();
			TNodeJsFuncStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm, const TStoreSchema& StoreSchema, const v8::Handle<v8::Value>& CallbacksObj);

			virtual bool HasRecNm() const { return !GetRecNmFun.IsEmpty(); }

			/// Check if record with given ID exists
			virtual bool IsRecId(const uint64& RecId) const { return true; } // throw TQmExcept::New("IsRecId: Not implemented"); }
			/// check if record with given name exists
			virtual bool IsRecNm(const TStr& RecNm) const { throw TQmExcept::New("IsRecNm not implemented"); }
			/// Get record name for a given id
			virtual TStr GetRecNm(const uint64& RecId) const;
			/// Get record id for a given name
			virtual uint64 GetRecId(const TStr& RecNm) const { throw TQmExcept::New("GetRecId not implemented"); }
			/// Get number of records in the store
			virtual uint64 GetRecs() const;
			/// Get iterator to go over all records in the store
			virtual PStoreIter GetIter() const { throw TQmExcept::New("GetIter not implemented"); }

			/// Get record set with all the records in the store
			virtual PRecSet GetAllRecs();
			/// Gets the first record in the store
			virtual uint64 GetFirstRecId() const;
			/// Gets the last record in the store
			virtual uint64 GetLastRecId() const;

			/// Does the store implement GetAllRecs?
			virtual bool HasGetAllRecs() const { return !GetAllRecsFun.IsEmpty(); }
			/// Is the forward iterator implemented?
			virtual bool HasForwardIter() const { return false; }
			/// Is the backward iterator implemented?
			virtual bool HasBackwardIter() const { return false; }
			/// Is the first record  id getter implemented?
			virtual bool HasFirstRecId() const { return !GetFirstRecIdFun.IsEmpty(); }
			/// Is the last record id getter implemented?
			virtual bool HasLastRecId() const { return !GetLastRecIdFun.IsEmpty(); }

			// MANIPULATION

			/// Add new record provided as JSon
			virtual uint64 AddRec(const PJsonVal& RecVal, const bool& TriggerEvents = true) { throw TQmExcept::New("Not implemented"); }
			/// Update existing record with updates in provided JSon
			virtual void UpdateRec(const uint64& RecId, const PJsonVal& RecVal) { throw TQmExcept::New("Not implemented"); }
			/// Deletes all records
			virtual void DeleteAllRecs() { throw TQmExcept::New("Not implemented"); }
			/// Delete the first DelRecs records (the records that were inserted first)
			virtual void DeleteFirstRecs(const int& DelRecs) { throw TQmExcept::New("Not implemented"); }
			/// Delete specific records
			virtual void DeleteRecs(const TUInt64V& DelRecIdV, const bool& AssertOK = true) { throw TQmExcept::New("Not implemented"); }

			/// Get field value using field id (default implementation throws exception)
			virtual int GetFieldInt(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual void GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const;
			/// Get field value using field id (default implementation throws exception)
			virtual uint64 GetFieldUInt64(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual TStr GetFieldStr(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual void GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const;
			/// Get field value using field id (default implementation throws exception)
			virtual bool GetFieldBool(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual double GetFieldFlt(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual TFltPr GetFieldFltPr(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual void GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const;
			/// Get field value using field id (default implementation throws exception)
			virtual void GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const;
			/// Get field value using field id (default implementation throws exception)
			virtual uint64 GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const;
			/// Get field value using field id (default implementation throws exception)
			virtual void GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const;



			virtual PJsonVal GetStats() { throw TQmExcept::New("Not implemented"); }
		};

		TVec<TWPt<TStore> > CreateJsStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal, const v8::Handle<v8::Value>& CallbacksObj);

	}

}
#endif
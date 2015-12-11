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

		class TNodeJsFuncStore : public TStore {
		private:
			v8::Persistent<v8::Function> GetRecsFun;

		private:
			/// Initialize from given store schema
			void InitFromSchema(const TStoreSchema& StoreSchema) {
				// create fields
				for (int i = 0; i<StoreSchema.FieldH.Len(); i++) {
					const TFieldDesc& FieldDesc = StoreSchema.FieldH[i];
					AddFieldDesc(FieldDesc);
				}
			}

			void SetCallback(v8::Handle<v8::Object>& CallbacksObj, v8::Persistent<v8::Function>& Callback, const TStr& Name) {
				v8::Isolate* Isolate = v8::Isolate::GetCurrent();
				v8::HandleScope HandleScope(Isolate);

				if (CallbacksObj->Has(v8::String::NewFromUtf8(Isolate, Name.CStr()))) {
					v8::Handle<v8::Value> Fun = CallbacksObj->Get(v8::String::NewFromUtf8(Isolate, Name.CStr()));
					QmAssert(Fun->IsFunction());
					Callback.Reset(Isolate, v8::Handle<v8::Function>::Cast(Fun));
				}
			}

			void InitCallbacks(v8::Handle<v8::Object>& CallbacksObj) {
				SetCallback(CallbacksObj, GetRecsFun, "GetRecords");
			}
		public:
			TNodeJsFuncStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm, const TStoreSchema& StoreSchema, v8::Handle<v8::Object>& CallbacksObj) : TStore(_Base, _StoreId, _StoreNm) {
				SetStoreType("TNodeJsFuncStore");
				InitFromSchema(StoreSchema);
				InitCallbacks(CallbacksObj);

			}
			~TNodeJsFuncStore() {
				GetRecsFun.Reset();
			}

			/// Check if record with given ID exists
			virtual bool IsRecId(const uint64& RecId) const { throw TQmExcept::New("Not implemented"); }
			/// check if record with given name exists
			virtual bool IsRecNm(const TStr& RecNm) const { throw TQmExcept::New("Not implemented"); }
			/// Get record name for a given id
			virtual TStr GetRecNm(const uint64& RecId) const { throw TQmExcept::New("Not implemented"); }
			/// Get record id for a given name
			virtual uint64 GetRecId(const TStr& RecNm) const { throw TQmExcept::New("Not implemented"); }
			/// Get number of records in the store
			virtual uint64 GetRecs() const {
				QmAssertR(!GetRecsFun.IsEmpty(), "TNodeJsFuncStore::GetRecsFun empty");
				v8::Isolate* Isolate = v8::Isolate::GetCurrent();
				v8::HandleScope HandleScope(Isolate);

				v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetRecsFun);
				v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

				v8::TryCatch TryCatch;
				v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
				if (TryCatch.HasCaught()) {
					v8::String::Utf8Value Msg(TryCatch.Message()->Get());
					throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetRecs, " + TStr(*Msg));
				}
				QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetRecs: Return type expected to be number");
				return (unsigned)(int64)RetVal->NumberValue();
			}
			/// Get iterator to go over all records in the store
			virtual PStoreIter GetIter() const { throw TQmExcept::New("Not implemented"); }

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

			virtual PJsonVal GetStats() { throw TQmExcept::New("Not implemented"); }
		};

		TVec<TWPt<TStore> > CreateJsStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal, v8::Handle<v8::Object>& CallbacksObj);

	}

}
#endif
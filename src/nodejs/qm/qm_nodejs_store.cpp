/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "qm_nodejs_store.h"

namespace TQm {
	namespace TStorage {
		void TNodeJsFuncStore::InitFromSchema(const TStoreSchema& StoreSchema) {
			// create fields
			for (int i = 0; i<StoreSchema.FieldH.Len(); i++) {
				const TFieldDesc& FieldDesc = StoreSchema.FieldH[i];
				AddFieldDesc(FieldDesc);
			}
		}

		void TNodeJsFuncStore::SetCallback(v8::Handle<v8::Object>& CallbacksObj, v8::Persistent<v8::Function>& Callback, const TStr& Name) {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);

			if (CallbacksObj->Has(v8::String::NewFromUtf8(Isolate, Name.CStr()))) {
				v8::Handle<v8::Value> Fun = CallbacksObj->Get(v8::String::NewFromUtf8(Isolate, Name.CStr()));
				QmAssert(Fun->IsFunction());
				Callback.Reset(Isolate, v8::Handle<v8::Function>::Cast(Fun));
			}
		}

		void TNodeJsFuncStore::InitCallbacks(v8::Handle<v8::Object>& CallbacksObj) {
			SetCallback(CallbacksObj, GetRecsFun, "GetRecords");
		}

		TNodeJsFuncStore::~TNodeJsFuncStore() {
			GetRecsFun.Reset();
		}

		TNodeJsFuncStore::TNodeJsFuncStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm, const TStoreSchema& StoreSchema, v8::Handle<v8::Object>& CallbacksObj) : TStore(_Base, _StoreId, _StoreNm) {
			SetStoreType("TNodeJsFuncStore");
			InitFromSchema(StoreSchema);
			InitCallbacks(CallbacksObj);
		}

		uint64 TNodeJsFuncStore::GetRecs() const {
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


		TVec<TWPt<TStore> > CreateJsStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal, v8::Handle<v8::Object>& CallbacksObj) {
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
				if (SchemaVal->IsArr()) {
					int Len = SchemaVal->GetArrVals();
					QmAssert(CallbacksObj->IsArray());
					v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(CallbacksObj);
					QmAssert(Len == Array->Length());
					for (int CallbN = 0; CallbN < Len; CallbN++) {
						QmAssert(Array->Get(CallbN)->IsObject());
						// create new store from the schema
						PStore Store = new TNodeJsFuncStore(Base, StoreId, StoreNm, StoreSchema, Array->Get(CallbN)->ToObject());
						// add store to base
						Base->AddStore(Store);
						// remember we create the store
						NewStoreV.Add(Store);
					}
				} else {
					// create new store from the schema
					PStore Store = new TNodeJsFuncStore(Base, StoreId, StoreNm, StoreSchema, CallbacksObj);
					// add store to base
					Base->AddStore(Store);
					// remember we create the store
					NewStoreV.Add(Store);
				}
			}

			// done
			return NewStoreV;
		}

	}

}

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

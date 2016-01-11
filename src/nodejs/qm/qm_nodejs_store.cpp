/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "qm_nodejs_store.h"
#include "qm_nodejs.h"

namespace TQm {
	namespace TStorage {
		void TNodeJsFuncStore::InitFromSchema(const TStoreSchema& StoreSchema) {
			// create fields
			for (int i = 0; i<StoreSchema.FieldH.Len(); i++) {
				const TFieldDesc& FieldDesc = StoreSchema.FieldH[i];
				AddFieldDesc(FieldDesc);
			}
		}

		void TNodeJsFuncStore::SetCallback(const v8::Handle<v8::Value>& CallbacksObj, v8::Persistent<v8::Function>& Callback, const TStr& Name) {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);

			if (CallbacksObj->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Name.CStr()))) {
				v8::Handle<v8::Value> Fun = CallbacksObj->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Name.CStr()));
				QmAssert(Fun->IsFunction());
				Callback.Reset(Isolate, v8::Handle<v8::Function>::Cast(Fun));
			}
		}

		void TNodeJsFuncStore::InitCallbacks(const v8::Handle<v8::Value>& CallbacksObj) {
			SetCallback(CallbacksObj, GetRecsFun, "GetRecords");
			SetCallback(CallbacksObj, GetFieldFun, "GetField");
			SetCallback(CallbacksObj, GetRecNmFun, "GetRecNm");
			SetCallback(CallbacksObj, GetFirstRecIdFun, "GetFirstRecId");
			SetCallback(CallbacksObj, GetLastRecIdFun, "GetLastRecId");
			//GetAllRecsFun has to be set after the store has been created!
		}

		TNodeJsFuncStore::~TNodeJsFuncStore() {
			GetRecsFun.Reset();
			GetFieldFun.Reset();
			GetRecNmFun.Reset();
			GetFirstRecIdFun.Reset();
			GetLastRecIdFun.Reset();
			GetAllRecsFun.Reset();
		}

		TNodeJsFuncStore::TNodeJsFuncStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm, const TStoreSchema& StoreSchema, const v8::Handle<v8::Value>& CallbacksObj) : TStoreNotImpl(_Base, _StoreId, _StoreNm) {
			SetStoreType("TNodeJsFuncStore");
			InitFromSchema(StoreSchema);
			InitCallbacks(CallbacksObj);
		}

		TStr TNodeJsFuncStore::GetRecNm(const uint64& RecId) const {
			if (HasRecNm()) {
				throw TQmExcept::New("GetRecNm: Not implemented");
			} else return TStr();
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

		PRecSet TNodeJsFuncStore::GetAllRecs() {
			QmAssertR(!GetAllRecsFun.IsEmpty(), "TNodeJsFuncStore::GetAllRecsFun empty");
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);

			v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetAllRecsFun);
			v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

			v8::TryCatch TryCatch;
			v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
			if (TryCatch.HasCaught()) {
				v8::String::Utf8Value Msg(TryCatch.Message()->Get());
				throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetAllRecs, " + TStr(*Msg));
			}			
			// unwrap a record set
			QmAssert(!RetVal.IsEmpty() && RetVal->IsObject());
			TNodeJsRecSet* JsRecSet = TNodeJsUtil::Unwrap<TNodeJsRecSet>(RetVal->ToObject());
			return JsRecSet->RecSet();
		}

		uint64 TNodeJsFuncStore::GetFirstRecId() const {
			QmAssertR(!GetFirstRecIdFun.IsEmpty(), "TNodeJsFuncStore::GetFirstRecIdFun empty");
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);

			v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFirstRecIdFun);
			v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

			v8::TryCatch TryCatch;
			v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
			if (TryCatch.HasCaught()) {
				v8::String::Utf8Value Msg(TryCatch.Message()->Get());
				throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetFirstRecId, " + TStr(*Msg));
			}
			QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetFirstRecId: Return type expected to be number");
			return (unsigned)(int64)RetVal->NumberValue();
		}

		uint64 TNodeJsFuncStore::GetLastRecId() const {
			QmAssertR(!GetLastRecIdFun.IsEmpty(), "TNodeJsFuncStore::GetLastRecIdFun empty");
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);

			v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetLastRecIdFun);
			v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

			v8::TryCatch TryCatch;
			v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
			if (TryCatch.HasCaught()) {
				v8::String::Utf8Value Msg(TryCatch.Message()->Get());
				throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetLastRecId, " + TStr(*Msg));
			}
			QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetLastRecId: Return type expected to be number");
			return (unsigned)(int64)RetVal->NumberValue();
		}

		v8::Handle<v8::Value> TNodeJsFuncStore::GetField(const uint64& RecId, const int& FieldId) const {
			QmAssertR(!GetFieldFun.IsEmpty(), "TNodeJsFuncStore::GetFieldFun empty");
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::EscapableHandleScope HandleScope(Isolate);

			v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFieldFun);
			v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

			// wrap args
			v8::Local<v8::Number> JsRecId = v8::Number::New(Isolate, (double)RecId);
			v8::Local<v8::Number> JsFieldId = v8::Number::New(Isolate, (double)FieldId);
			int Argc = 2;
			v8::Handle<v8::Value> Argv[2] = { JsRecId, JsFieldId };

			v8::TryCatch TryCatch;
			v8::Local<v8::Value> RetVal = Callback->Call(GlobalContext, Argc, Argv);
			if (TryCatch.HasCaught()) {
				v8::String::Utf8Value Msg(TryCatch.Message()->Get());
				throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetField, " + TStr(*Msg));
			}
			return HandleScope.Escape(RetVal);
		}

		int TNodeJsFuncStore::GetFieldInt(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsInt32(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
			return RetVal->Int32Value();
		}
		void TNodeJsFuncStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			if (RetVal->IsArray()) {
				// check if we have JavaScript array
				v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(RetVal);
				TIntV Res;
				for (uint32_t ElN = 0; ElN < Array->Length(); ElN++) {
					v8::Local<v8::Value> ArrayVal = Array->Get(ElN);
					QmAssertR(ArrayVal->IsInt32(), "Field " + GetFieldNm(FieldId) + " expects array of integers");
					Res.Add(ArrayVal->Int32Value());
				}
				IntV = Res;
			} else {
				// otherwise it must be GLib array (or exception)
				QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(RetVal->ToObject()) == TNodeJsIntV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected an int vector)");
				TNodeJsIntV* JsIntV = TNodeJsUtil::Unwrap<TNodeJsIntV>(RetVal->ToObject());
				IntV = JsIntV->Vec;
			}
		}
		uint64 TNodeJsFuncStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
			return (unsigned)(int64) RetVal->NumberValue();
		}
		TStr TNodeJsFuncStore::GetFieldStr(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsString(), "TNodeJsFuncStore::GetField: Return type expected to be a string");
			return TNodeJsUtil::GetStr(RetVal->ToString());
		}
		void TNodeJsFuncStore::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			if (RetVal->IsArray()) {
				// check if we have JavaScript array
				v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(RetVal);
				TStrV Res;
				for (uint32_t ElN = 0; ElN < Array->Length(); ElN++) {
					v8::Local<v8::Value> ArrayVal = Array->Get(ElN);
					QmAssertR(ArrayVal->IsString(), "Field " + GetFieldNm(FieldId) + " expects array of strings");
					Res.Add(TNodeJsUtil::GetStr(ArrayVal->ToString()));
				}
				StrV = Res;
			} else {
				// otherwise it must be GLib array (or exception)
				QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(RetVal->ToObject()) == TNodeJsStrV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a string vector)");
				TNodeJsStrV* JsStrV = TNodeJsUtil::Unwrap<TNodeJsStrV>(RetVal->ToObject());
				StrV = JsStrV->Vec;
			}
		}
		bool TNodeJsFuncStore::GetFieldBool(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStore::GetField: Return type expected to be a boolean");
			return RetVal->BooleanValue();
		}
		double TNodeJsFuncStore::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
			return RetVal->NumberValue();
		}
		TFltPr TNodeJsFuncStore::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsArray(), "Field " + GetFieldNm(FieldId) + " not array");
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(RetVal);
			QmAssert(Array->Length() >= 2);
			QmAssert(Array->Get(0)->IsNumber());
			QmAssert(Array->Get(1)->IsNumber());
			return TFltPr(Array->Get(0)->NumberValue(), Array->Get(1)->NumberValue());
		}
		void TNodeJsFuncStore::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			if (RetVal->IsArray()) {
				// check if we have JavaScript array
				v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(RetVal);
				TFltV Res;
				for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
					v8::Local<v8::Value> ArrayVal = Array->Get(FltN);
					QmAssertR(ArrayVal->IsNumber(), "Field " + GetFieldNm(FieldId) + " expects array of numbers");
					const double Val = ArrayVal->NumberValue();
					Res.Add(Val);
				}
				FltV = Res;
			} else {
				// otherwise it must be GLib array (or exception)
				QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(RetVal->ToObject()) == TNodeJsFltV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a vector)");
				TNodeJsFltV* JsFltV = TNodeJsUtil::Unwrap<TNodeJsFltV>(RetVal->ToObject());
				FltV = JsFltV->Vec;
			}
		}
		void TNodeJsFuncStore::GetFieldTm(const uint64& RecId, const int& FieldId, TTm& Tm) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsObject() || RetVal->IsString() || RetVal->IsNumber(), "Field " + GetFieldNm(FieldId) + " not object or string");
			Tm = TTm::GetTmFromMSecs(TNodeJsUtil::GetTmMSecs(RetVal));
		}
		uint64 TNodeJsFuncStore::GetFieldTmMSecs(const uint64& RecId, const int& FieldId) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsObject() || RetVal->IsString() || RetVal->IsNumber(), "Field " + GetFieldNm(FieldId) + " not object or string");
			return TNodeJsUtil::GetTmMSecs(RetVal);
		}
		void TNodeJsFuncStore::GetFieldNumSpV(const uint64& RecId, const int& FieldId, TIntFltKdV& SpV) const {
			v8::Isolate* Isolate = v8::Isolate::GetCurrent();
			v8::HandleScope HandleScope(Isolate);
			v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
			QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(RetVal->ToObject()) == TNodeJsSpVec::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a sparse vector)");
			TNodeJsSpVec* JsSpVec = TNodeJsUtil::Unwrap<TNodeJsSpVec>(RetVal->ToObject());
			SpV = JsSpVec->Vec;
		}

		TVec<TWPt<TStore> > CreateJsStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal, const v8::Handle<v8::Value>& CallbacksObj) {
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
					QmAssert(Len == (int)Array->Length());
					for (int CallbN = 0; CallbN < Len; CallbN++) {
						QmAssert(Array->Get(CallbN)->IsObject());
						// create new store from the schema
						PStore Store = new TNodeJsFuncStore(Base, StoreId, StoreNm, StoreSchema, Array->Get(CallbN));
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

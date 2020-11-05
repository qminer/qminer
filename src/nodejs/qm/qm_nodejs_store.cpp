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

        void TNodeJsFuncStore::SetCallback(const v8::Local<v8::Value>& CallbacksObj, v8::Persistent<v8::Function>& Callback, const TStr& Name) {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);

            v8::Local<v8::String> FldNm = TNodeJsUtil::ToLocal(Nan::New(Name.CStr()));
            v8::Local<v8::Object> CallbackObj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(CallbacksObj));

            if (Nan::Has(CallbackObj, FldNm).ToChecked()) {
                v8::Local<v8::Value> Fun = TNodeJsUtil::ToLocal(Nan::Get(CallbackObj, FldNm));
                QmAssert(Fun->IsFunction());
                Callback.Reset(Isolate, v8::Local<v8::Function>::Cast(Fun));
            }
        }

        void TNodeJsFuncStore::InitCallbacks(const v8::Local<v8::Value>& CallbacksObj) {
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

        TNodeJsFuncStore::TNodeJsFuncStore(const TWPt<TBase>& _Base, uint _StoreId, const TStr& _StoreNm, const TStoreSchema& StoreSchema, const v8::Local<v8::Value>& CallbacksObj) : TStoreNotImpl(_Base, _StoreId, _StoreNm) {
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

            v8::TryCatch TryCatch(Isolate);
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, 0, NULL);
            if (TryCatch.HasCaught()) {
                Nan::Utf8String Msg(TryCatch.Message()->Get());
                throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetRecs, " + TStr(*Msg));
            }

            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
            QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetRecs: Return type expected to be number");
            return (unsigned long long)Nan::To<int64>(RetVal).FromJust();
        }

        PRecSet TNodeJsFuncStore::GetAllRecs() {
            QmAssertR(!GetAllRecsFun.IsEmpty(), "TNodeJsFuncStore::GetAllRecsFun empty");
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);

            v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetAllRecsFun);
            v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

            v8::TryCatch TryCatch(Isolate);
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, 0, NULL);
            if (TryCatch.HasCaught()) {
                Nan::Utf8String Msg(TryCatch.Message()->Get());
                throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetAllRecs, " + TStr(*Msg));
            }

            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
            // unwrap a record set
            QmAssert(!RetVal.IsEmpty() && RetVal->IsObject());
            TNodeJsRecSet* JsRecSet = TNodeJsUtil::Unwrap<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal)));
            return JsRecSet->RecSet();
        }

        uint64 TNodeJsFuncStore::GetFirstRecId() const {
            QmAssertR(!GetFirstRecIdFun.IsEmpty(), "TNodeJsFuncStore::GetFirstRecIdFun empty");
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);

            v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFirstRecIdFun);
            v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

            v8::TryCatch TryCatch(Isolate);
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, 0, NULL);
            if (TryCatch.HasCaught()) {
                Nan::Utf8String Msg(TryCatch.Message()->Get());
                throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetFirstRecId, " + TStr(*Msg));
            }

            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
            QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetFirstRecId: Return type expected to be number");
            return (unsigned long long)Nan::To<int64>(RetVal).FromJust();
        }

        uint64 TNodeJsFuncStore::GetLastRecId() const {
            QmAssertR(!GetLastRecIdFun.IsEmpty(), "TNodeJsFuncStore::GetLastRecIdFun empty");
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);

            v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetLastRecIdFun);
            v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

            v8::TryCatch TryCatch(Isolate);
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, 0, NULL);
            if (TryCatch.HasCaught()) {
                Nan::Utf8String Msg(TryCatch.Message()->Get());
                throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetLastRecId, " + TStr(*Msg));
            }

            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
            QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetLastRecId: Return type expected to be number");
            return (unsigned long long)Nan::To<int64>(RetVal).FromJust();
        }

        v8::Local<v8::Value> TNodeJsFuncStore::GetField(const uint64& RecId, const int& FieldId) const {
            QmAssertR(!GetFieldFun.IsEmpty(), "TNodeJsFuncStore::GetFieldFun empty");
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::EscapableHandleScope HandleScope(Isolate);

            v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFieldFun);
            v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

            // wrap args
            v8::Local<v8::Number> JsRecId = Nan::New((double)RecId);
            v8::Local<v8::Number> JsFieldId = Nan::New((double)FieldId);
            int Argc = 2;
            v8::Local<v8::Value> Argv[2] = { JsRecId, JsFieldId };

            v8::TryCatch TryCatch(Isolate);
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, Argc, Argv);
            if (TryCatch.HasCaught()) {
                Nan::Utf8String Msg(TryCatch.Message()->Get());
                throw TQm::TQmExcept::New("Javascript exception triggered from TNodeJsFuncStore::GetField, " + TStr(*Msg));
            }

            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
            return HandleScope.Escape(RetVal);
        }

        int TNodeJsFuncStore::GetFieldInt(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsInt32(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
            return Nan::To<int>(RetVal).FromJust();
        }
        void TNodeJsFuncStore::GetFieldIntV(const uint64& RecId, const int& FieldId, TIntV& IntV) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            if (RetVal->IsArray()) {
                // check if we have JavaScript array
                v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(RetVal);
                TIntV Res;
                for (uint32_t ElN = 0; ElN < Array->Length(); ElN++) {
                    v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, ElN));
                    QmAssertR(ArrayVal->IsInt32(), "Field " + GetFieldNm(FieldId) + " expects array of integers");
                    Res.Add(Nan::To<int>(ArrayVal).FromJust());
                }
                IntV = Res;
            } else {
                // otherwise it must be GLib array (or exception)
                QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal))) == TNodeJsIntV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected an int vector)");
                TNodeJsIntV* JsIntV = TNodeJsUtil::Unwrap<TNodeJsIntV>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal)));
                IntV = JsIntV->Vec;
            }
        }
        uint64 TNodeJsFuncStore::GetFieldUInt64(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
            return (unsigned long long)Nan::To<int64>(RetVal).FromJust();
        }
        TStr TNodeJsFuncStore::GetFieldStr(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsString(), "TNodeJsFuncStore::GetField: Return type expected to be a string");
            return TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(RetVal)));
        }
        void TNodeJsFuncStore::GetFieldStrV(const uint64& RecId, const int& FieldId, TStrV& StrV) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            if (RetVal->IsArray()) {
                // check if we have JavaScript array
                v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(RetVal);
                TStrV Res;
                for (uint32_t ElN = 0; ElN < Array->Length(); ElN++) {
                    v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, ElN));
                    QmAssertR(ArrayVal->IsString(), "Field " + GetFieldNm(FieldId) + " expects array of strings");
                    Res.Add(TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(ArrayVal))));
                }
                StrV = Res;
            } else {
                // otherwise it must be GLib array (or exception)
                QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal))) == TNodeJsStrV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a string vector)");
                TNodeJsStrV* JsStrV = TNodeJsUtil::Unwrap<TNodeJsStrV>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal)));
                StrV = JsStrV->Vec;
            }
        }
        bool TNodeJsFuncStore::GetFieldBool(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStore::GetField: Return type expected to be a boolean");
            return  Nan::To<bool>(RetVal).FromJust();
        }
        double TNodeJsFuncStore::GetFieldFlt(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStore::GetField: Return type expected to be a number");
            return Nan::To<double>(RetVal).FromJust();
        }
        TFltPr TNodeJsFuncStore::GetFieldFltPr(const uint64& RecId, const int& FieldId) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            QmAssertR(RetVal->IsArray(), "Field " + GetFieldNm(FieldId) + " not array");
            v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(RetVal);
            QmAssert(Array->Length() >= 2);
            QmAssert(TNodeJsUtil::ToLocal(Nan::Get(Array, 0))->IsNumber());
            QmAssert(TNodeJsUtil::ToLocal(Nan::Get(Array, 1))->IsNumber());
            return TFltPr(Nan::To<double>(TNodeJsUtil::ToLocal(Nan::Get(Array, 0))).FromJust(), Nan::To<double>(TNodeJsUtil::ToLocal(Nan::Get(Array, 1))).FromJust());
        }
        void TNodeJsFuncStore::GetFieldFltV(const uint64& RecId, const int& FieldId, TFltV& FltV) const {
            v8::Isolate* Isolate = v8::Isolate::GetCurrent();
            v8::HandleScope HandleScope(Isolate);
            v8::Local<v8::Value> RetVal = GetField(RecId, FieldId);
            if (RetVal->IsArray()) {
                // check if we have JavaScript array
                v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(RetVal);
                TFltV Res;
                for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
                    v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, FltN));
                    QmAssertR(ArrayVal->IsNumber(), "Field " + GetFieldNm(FieldId) + " expects array of numbers");
                    const double Val = Nan::To<double>(ArrayVal).FromJust();
                    Res.Add(Val);
                }
                FltV = Res;
            } else {
                // otherwise it must be GLib array (or exception)
                QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal))) == TNodeJsFltV::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a vector)");
                TNodeJsFltV* JsFltV = TNodeJsUtil::Unwrap<TNodeJsFltV>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal)));
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
            QmAssertR(RetVal->IsObject() && (TNodeJsUtil::GetClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal))) == TNodeJsSpVec::GetClassId()), "TNodeJsFuncStore::GetField: Return type not an object (expected a sparse vector)");
            TNodeJsSpVec* JsSpVec = TNodeJsUtil::Unwrap<TNodeJsSpVec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(RetVal)));
            SpV = JsSpVec->Vec;
        }

        TVec<TWPt<TStore> > CreateJsStoresFromSchema(const TWPt<TBase>& Base, const PJsonVal& SchemaVal, const v8::Local<v8::Value>& CallbacksObj) {
            // parse and validate the schema
            InfoLog("Parsing schema");
            TStoreSchemaV SchemaV; TStoreSchema::ParseSchema(Base, SchemaVal, SchemaV);
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
                    v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(CallbacksObj);
                    QmAssert(Len == (int)Array->Length());
                    for (int CallbN = 0; CallbN < Len; CallbN++) {
                        QmAssert(TNodeJsUtil::ToLocal(Nan::Get(Array, CallbN))->IsObject());
                        // create new store from the schema
                        PStore Store = new TNodeJsFuncStore(Base, StoreId, StoreNm, StoreSchema, TNodeJsUtil::ToLocal(Nan::Get(Array, CallbN)));
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

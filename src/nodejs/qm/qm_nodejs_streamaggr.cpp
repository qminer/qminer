/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "qm_nodejs.h"
#include "qm_param.h"
#include "../la/la_nodejs.h"

#include <node_buffer.h>

///////////////////////////////
// NodeJs Stream Aggregator

v8::Persistent<v8::Function> TNodeJsStreamAggr::Constructor;

void TNodeJsStreamAggr::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    // template for creating function from javascript using "new", uses _NewJs callback
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsStreamAggr>);
    // child will have the same properties and methods, but a different callback: _NewCpp
    v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStreamAggr>);
    child->Inherit(tpl);

    child->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    child->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "reset", _reset);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onStep", _onStep);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onTime", _onTime);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onAdd", _onAdd);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onUpdate", _onUpdate);
    NODE_SET_PROTOTYPE_METHOD(tpl, "onDelete", _onDelete);
    NODE_SET_PROTOTYPE_METHOD(tpl, "saveJson", _saveJson);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
    NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
    NODE_SET_PROTOTYPE_METHOD(tpl, "saveStateJson", _saveStateJson);
    NODE_SET_PROTOTYPE_METHOD(tpl, "loadStateJson", _loadStateJson);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getInteger", _getInteger);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFloat", _getFloat);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestamp", _getTimestamp);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatLength", _getFloatLength);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatAt", _getFloatAt);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatVector", _getFloatVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampLength", _getTimestampLength);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampAt", _getTimestampAt);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampVector", _getTimestampVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getInFloatVector", _getInFloatVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getInTimestampVector", _getInTimestampVector); // multiple interfaces
    NODE_SET_PROTOTYPE_METHOD(tpl, "getOutFloatVector", _getOutFloatVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getOutTimestampVector", _getOutTimestampVector); // multiple interfaces
    NODE_SET_PROTOTYPE_METHOD(tpl, "getNumberOfRecords", _getNumberOfRecords); // multiple interfaces

    NODE_SET_PROTOTYPE_METHOD(tpl, "getInValueVector", _getInValueVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getOutValueVector", _getOutValueVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getValueVector", _getValueVector);

    NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureSpace", _getFeatureSpace);

    // Properties
    #if NODE_MODULE_VERSION >= 134 // Node.js >= 24
    tpl->InstanceTemplate()->SetNativeDataProperty(TNodeJsUtil::ToLocal(Nan::New("name")), _name);
    #else
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("name")), _name);
    #endif
    #if NODE_MODULE_VERSION >= 134 // Node.js >= 24
    tpl->InstanceTemplate()->SetNativeDataProperty(TNodeJsUtil::ToLocal(Nan::New("val")), _val);
    #else
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("val")), _val);
    #endif
    #if NODE_MODULE_VERSION >= 134 // Node.js >= 24
    tpl->InstanceTemplate()->SetNativeDataProperty(TNodeJsUtil::ToLocal(Nan::New("init")), _init);
    #else
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("init")), _init);
    #endif

    // This has to be last, otherwise the properties won't show up on the object in JavaScript
    // Constructor is used when creating the object from C++
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(child->GetFunction(context)));
    // we need to export the class for calling using "new FIn(...)"
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));

    TNodeJsUtil::RegisterClassNmAccessor(GetClassId(), "name");
}

TNodeJsStreamAggr* TNodeJsStreamAggr::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // make sure we have enough parameters and not to many
    QmAssertR(Args.Length() == 2 || Args.Length() == 3,
              "stream aggregator constructor expects two or three parameters");
    // make sure we get what we expect
    QmAssertR(Args[0]->IsObject() && Args[1]->IsObject(),
              "stream aggregator constructor expects first two arguments as objects");
    // unwrap the base
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));

    // placeholder for the new stream aggregate
    TQm::PStreamAggr StreamAggr;
    // to which stores should we attach the new aggregate
    TStrV StoreNmV;
    // get aggregate type
    TStr TypeNm = TNodeJsUtil::GetArgStr(Args, 1, "type", "javaScript");
    // call constructor appropriate to the type
    if (TypeNm == "javaScript") {
        // we have a javascript stream aggregate, first get its name if we have one
        TStr AggrName = TNodeJsUtil::GetArgStr(Args, 1, "name", TGuid::GenSafeGuid());
        // create aggregate
        StreamAggr = TNodeJsFuncStreamAggr::New(JsBase->Base, AggrName, TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
    } else if (TypeNm == "merger") {
        // we have merger, get its parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgToNmJson(Args, 1);
        // create new merger aggregate
        StreamAggr = TQm::TStreamAggrs::TMerger::New(JsBase->Base, ParamVal);
        // automatically attach merger to all listed stores
        TStrV _StoreNmV = TQm::TStreamAggrs::TMerger::GetStoreNm(ParamVal);
        StoreNmV.AddV(_StoreNmV);
    } else {
        // we have a GLib stream aggregate, translate parameters to PJsonVal
        PJsonVal ParamVal = TNodeJsUtil::GetArgToNmJson(Args, 1);
        if (Args.Length() >= 3 && Args[2]->IsString()) {
            ParamVal->AddToObj("store", TNodeJsUtil::GetArgStr(Args, 2));
        }
        // create new aggregate
        StreamAggr = TQm::TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
    }
    // check if we have any more stores names passed to attached to
    if (Args.Length() > 2) {
        if (Args[2]->IsString()) {
            StoreNmV.Add(TNodeJsUtil::GetArgStr(Args, 2));
        } else if (Args[2]->IsArray()) {
            PJsonVal StoresJson = TNodeJsUtil::GetArgJson(Args, 2);
            QmAssertR(StoresJson->IsDef(), "[StreamAggr] Args[2] should be a string (store name) or a string array (store names)");
            TStrV _StoreNmV; StoresJson->GetArrStrV(_StoreNmV);
            StoreNmV.AddV(_StoreNmV);
        } else if (TNodeJsUtil::IsArgWrapObj<TNodeJsStore>(Args, 2)) {
            const TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[2])));
            const TStr& StoreNm = JsStore->Store->GetStoreNm();
            StoreNmV.Add(StoreNm);
        }
    }

    // register stream aggregate
    JsBase->Base->AddStreamAggr(StreamAggr);
    // attach to listed stores
    if (!StoreNmV.Empty()) {
        for (const TStr& StoreNm : StoreNmV) {
            // make sure it exists
            QmAssertR(JsBase->Base->IsStoreNm(StoreNm), "[StreamAggr] Unknown store " + StoreNm);
            const uint StoreId = JsBase->Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
            // attech the stream aggregate to the store
            JsBase->Base->GetStreamAggrSet(StoreId)->AddStreamAggr(StreamAggr);
        }
    }

    // we are good
    return new TNodeJsStreamAggr(StreamAggr);
}

void TNodeJsStreamAggr::reset(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    JsSA->SA->Reset();

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::onStep(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // if arg 1 exists, get the caller stream aggregate
    if (Args.Length() >= 1) {
        EAssertR(TNodeJsUtil::IsClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])), TNodeJsStreamAggr::GetClassId()), "Argument expected to be a stream aggregate!");
        TNodeJsStreamAggr* JsSACaller = ObjectWrap::Unwrap<TNodeJsStreamAggr>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
        JsSA->SA->OnStep(JsSACaller->SA);
    } else {
        JsSA->SA->OnStep(NULL);
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::onTime(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    QmAssertR(Args.Length() >= 1, "sa.onTime should take one argument of type TUInt64");
    const uint64 Time = TNodeJsUtil::GetArgTmMSecs(Args, 0);
    // if arg 1 exists, get the caller stream aggregate
    if (Args.Length() >= 2) {
        EAssertR(TNodeJsUtil::IsClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])), TNodeJsStreamAggr::GetClassId()), "Argument expected to be a stream aggregate!");
        TNodeJsStreamAggr* JsSACaller = ObjectWrap::Unwrap<TNodeJsStreamAggr>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
        JsSA->SA->OnTime(Time, JsSACaller->SA);
    } else {
        JsSA->SA->OnTime(Time, NULL);
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::onAdd(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    QmAssertR(Args.Length() >= 1 && Args[0]->IsObject(), "sa.onAdd should take one argument of type TNodeJsRec");
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    // if arg 1 exists, get the caller stream aggregate
    if (Args.Length() >= 2) {
        EAssertR(TNodeJsUtil::IsClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])), TNodeJsStreamAggr::GetClassId()), "Argument expected to be a stream aggregate!");
        TNodeJsStreamAggr* JsSACaller = ObjectWrap::Unwrap<TNodeJsStreamAggr>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
        JsSA->SA->OnAddRec(JsRec->Rec, JsSACaller->SA);
    } else {
        JsSA->SA->OnAddRec(JsRec->Rec, NULL);
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::onUpdate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    QmAssertR(Args.Length() >= 1 && Args[0]->IsObject(), "sa.onUpdate should take one argument of type TNodeJsRec");
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    // if arg 1 exists, get the caller stream aggregate
    if (Args.Length() >= 2) {
        EAssertR(TNodeJsUtil::IsClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])), TNodeJsStreamAggr::GetClassId()), "Argument expected to be a stream aggregate!");
        TNodeJsStreamAggr* JsSACaller = ObjectWrap::Unwrap<TNodeJsStreamAggr>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
        JsSA->SA->OnUpdateRec(JsRec->Rec, JsSACaller->SA);
    } else {
        JsSA->SA->OnUpdateRec(JsRec->Rec, NULL);
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::onDelete(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    QmAssertR(Args.Length() >= 1 && Args[0]->IsObject(), "sa.onDelete should take one argument of type TNodeJsRec");
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    // if arg 1 exists, get the caller stream aggregate
    if (Args.Length() >= 2) {
        EAssertR(TNodeJsUtil::IsClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])), TNodeJsStreamAggr::GetClassId()), "Argument expected to be a stream aggregate!");
        TNodeJsStreamAggr* JsSACaller = ObjectWrap::Unwrap<TNodeJsStreamAggr>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
        JsSA->SA->OnDeleteRec(JsRec->Rec, JsSACaller->SA);
    } else {
        JsSA->SA->OnDeleteRec(JsRec->Rec, NULL);
    }

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::saveJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    const TInt Limit = TNodeJsUtil::GetArgInt32(Args, 0, -1);
    PJsonVal Json = JsSA->SA->SaveJson(Limit);
    v8::Local<v8::Value> V8Json = TNodeJsUtil::ParseJson(Isolate, Json);

    Args.GetReturnValue().Set(V8Json);
}

void TNodeJsStreamAggr::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    // save
    JsSA->SA->SaveState(*JsFOut->SOut);
    JsFOut->SOut->Flush();
    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsStreamAggr::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);

    // load
    JsSA->SA->LoadState(*JsFIn->SIn);

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::saveStateJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // save
    PJsonVal StateJson = JsSA->SA->SaveStateJson();
    v8::Local<v8::Value> Result = TNodeJsUtil::ParseJson(Isolate, StateJson);
    Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamAggr::loadStateJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    PJsonVal StateJson = TNodeJsUtil::GetArgJson(Args, 0);

    // load
    JsSA->SA->LoadStateJson(StateJson);

    Args.GetReturnValue().Set(JS_GET_HOLDER(Args));
}

void TNodeJsStreamAggr::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsSA->SA->GetParams()));
}

void TNodeJsStreamAggr::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > 0, "TNodeJsStreamAggr::setParams: takes one argument!");

    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    const PJsonVal ParamVal = TNodeJsUtil::GetObjToNmJson(Args[0]);
    JsSA->SA->SetParams(ParamVal);

    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsStreamAggr::getInteger(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    if (Args.Length() == 0) {
        // try to cast as IInt
        TWPt<TQm::TStreamAggrOut::IInt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IInt*>(JsSA->SA());
        if (Aggr.Empty()) {
            throw TQm::TQmExcept::New("TNodeJsStreamAggr getInteger: stream aggregate does not implement IInt: " + JsSA->SA->GetAggrNm());
        }
        Args.GetReturnValue().Set(Nan::New(Aggr->GetInt()));
    } else if (Args.Length() == 1) {
        // try to cast as INmInt
        TStr Nm = TNodeJsUtil::GetArgStr(Args, 0);
        TWPt<TQm::TStreamAggrOut::INmInt> Aggr = dynamic_cast<TQm::TStreamAggrOut::INmInt*>(JsSA->SA());
        if (Aggr.Empty()) {
            throw TQm::TQmExcept::New("TNodeJsStreamAggr getInteger: stream aggregate does not implement INmInt: " + JsSA->SA->GetAggrNm());
        }
        if (!Aggr->IsNmInt(Nm)) {
            Args.GetReturnValue().Set(Nan::Null());
        } else {
            int Res = Aggr->GetNmInt(Nm);
            Args.GetReturnValue().Set(Nan::New(Res));
        }
    } else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr getInteger: too many input arguments: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::getFloat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    if (Args.Length() == 0) {
        // try to cast as IFlt
        TWPt<TQm::TStreamAggrOut::IFlt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFlt*>(JsSA->SA());
        if (Aggr.Empty()) {
            throw TQm::TQmExcept::New("TNodeJsStreamAggr getFloat: stream aggregate does not implement IFlt: " + JsSA->SA->GetAggrNm());
        }
        Args.GetReturnValue().Set(Nan::New(Aggr->GetFlt()));
    } else if (Args.Length() == 1) {
        // try to cast as INmFlt
        TStr Nm = TNodeJsUtil::GetArgStr(Args, 0);
        TWPt<TQm::TStreamAggrOut::INmFlt> Aggr = dynamic_cast<TQm::TStreamAggrOut::INmFlt*>(JsSA->SA());
        if (Aggr.Empty()) {
            throw TQm::TQmExcept::New("TNodeJsStreamAggr  getFloat: stream aggregate does not implement INmFlt: " + JsSA->SA->GetAggrNm());
        }
        if (!Aggr->IsNmFlt(Nm)) {
            Args.GetReturnValue().Set(Nan::Null());
        } else {
            double Res = Aggr->GetNmFlt(Nm);
            Args.GetReturnValue().Set(Nan::New(Res));
        }
    } else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr getFloat: too many input arguments: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::getTimestamp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as ITm
    TWPt<TQm::TStreamAggrOut::ITm> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITm*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTm : stream aggregate does not implement ITm: " + JsSA->SA->GetAggrNm());
    }

    uint64 WinMSecs = Aggr->GetTmMSecs();
    // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
    double UnixMSecs = (double)TNodeJsUtil::GetJsTimestamp(WinMSecs);
    Args.GetReturnValue().Set(Nan::New(UnixMSecs));
}

void TNodeJsStreamAggr::getFloatLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as IFltVec
    TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltLen : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
    }

    Args.GetReturnValue().Set(Nan::New(Aggr->GetVals()));
}

void TNodeJsStreamAggr::getFloatAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as IFltVec
    int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
    TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltAt : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
    }

    TFlt Flt;
    Aggr->GetVal(ElN, Flt);
    Args.GetReturnValue().Set(Nan::New(Flt));
}

void TNodeJsStreamAggr::getFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as IFltVec
    TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltV : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
    }
    TFltV Res;
    Aggr->GetValV(Res);
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsStreamAggr::getTimestampLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as ITmVec
    TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmLen : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
    }

    Args.GetReturnValue().Set(Nan::New(Aggr->GetTmLen()));
}

void TNodeJsStreamAggr::getTimestampAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as ITmVec
    int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
    TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmAt : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
    }
    QmAssertR(JsSA->SA->IsInit(), "TNodeJsStreamAggr::getTmAt : stream aggregate '" + JsSA->SA->GetAggrNm() + "' is not initialized!");

    uint64 WinMSecs = Aggr->GetTm(ElN);
    // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
    double UnixMSecs = (double)TNodeJsUtil::GetJsTimestamp(WinMSecs);
    Args.GetReturnValue().Set(Nan::New(UnixMSecs));
}

void TNodeJsStreamAggr::getTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as ITmVec
    TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmV : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
    }
    TUInt64V Res;
    Aggr->GetTmV(Res);
    int Len = Res.Len();
    TFltV FltRes(Len);
    for (int ElN = 0; ElN < Len; ElN++) {
        // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
        FltRes[ElN] = (double)TNodeJsUtil::GetJsTimestamp(Res[ElN]);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}

void TNodeJsStreamAggr::getInFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as IFltIO
    TWPt<TQm::TStreamAggrOut::IFltIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltIO*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInFloatVector : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
    }
    TFltV Res;
    Aggr->GetInValV(Res);

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsStreamAggr::getInTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as ITmIO
    TWPt<TQm::TStreamAggrOut::ITmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmIO*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutTmV : stream aggregate does not implement ITmIO: " + JsSA->SA->GetAggrNm());
    }
    TUInt64V Res;
    Aggr->GetInTmMSecsV(Res);
    int Len = Res.Len();
    TFltV FltRes(Len);
    for (int ElN = 0; ElN < Len; ElN++) {
        // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
        FltRes[ElN] = (double)TNodeJsUtil::GetJsTimestamp(Res[ElN]);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}

void TNodeJsStreamAggr::getOutFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as IFltIO
    TWPt<TQm::TStreamAggrOut::IFltIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltIO*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutFloatVector : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
    }
    TFltV Res;
    Aggr->GetOutValV(Res);

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsStreamAggr::getOutTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as ITmIO
    TWPt<TQm::TStreamAggrOut::ITmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmIO*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutTmV : stream aggregate does not implement ITmIO: " + JsSA->SA->GetAggrNm());
    }
    TUInt64V Res;
    Aggr->GetOutTmMSecsV(Res);
    int Len = Res.Len();
    TFltV FltRes(Len);
    for (int ElN = 0; ElN < Len; ElN++) {
        // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
        FltRes[ElN] = (double)TNodeJsUtil::GetJsTimestamp(Res[ElN]);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}

void TNodeJsStreamAggr::getNumberOfRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));
    // try to cast as ITmIO
    TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
    if (Aggr.Empty()) {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getNumberOfRecords : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
    }
    Args.GetReturnValue().Set(Nan::New(Aggr->GetTmLen()));
}

void TNodeJsStreamAggr::getInValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as IValIO
    TWPt<TQm::TStreamAggrOut::IValIO<TFlt> > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IValIO<TFlt> *>(JsSA->SA());
    TWPt<TQm::TStreamAggrOut::IValIO<TIntFltKdV> > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::IValIO<TIntFltKdV> *>(JsSA->SA());

    if (!AggrFlt.Empty()) {
        TFltV Res;
        AggrFlt->GetInValV(Res);
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
    }
    else if (!AggrSpV.Empty()){
        TVec<TIntFltKdV> Res;
        AggrSpV->GetInValV(Res);
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
    }
    else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInValueVector : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::getOutValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as IValTmIO
    TWPt<TQm::TStreamAggrOut::IValIO<TFlt> > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IValIO<TFlt> *>(JsSA->SA());
    TWPt<TQm::TStreamAggrOut::IValIO<TIntFltKdV> > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::IValIO<TIntFltKdV> *>(JsSA->SA());

    if (!AggrFlt.Empty()) {
        TFltV Res;
        AggrFlt->GetOutValV(Res);
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
    }
    else if (!AggrSpV.Empty()){
        TVec<TIntFltKdV> Res;
        AggrSpV->GetOutValV(Res);
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
    } else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutValueVector : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::getFeatureSpace(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as IFtrSpace
    TWPt<TQm::TStreamAggrOut::IFtrSpace > Aggr = dynamic_cast<TQm::TStreamAggrOut::IFtrSpace *>(JsSA->SA());

    if (!Aggr.Empty()) {
        TQm::PFtrSpace FtrSpace = Aggr->GetFtrSpace();
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsFtrSpace>(new TNodeJsFtrSpace(FtrSpace)));
    } else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFeatureSpace : stream aggregate does not implement IFtrSpace: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::getValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(JS_GET_HOLDER(Args));

    // try to cast as IValTmIO
    TWPt<TQm::TStreamAggrOut::IFltVec > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IFltVec *>(JsSA->SA());
    TWPt<TQm::TStreamAggrOut::ISparseVVec > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::ISparseVVec *>(JsSA->SA());
    TWPt<TQm::TStreamAggrOut::ISparseVec > SpV = dynamic_cast<TQm::TStreamAggrOut::ISparseVec *>(JsSA->SA());

    if (!AggrFlt.Empty()) {
        TFltV Res;
        AggrFlt->GetValV(Res);
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
    } else if (!AggrSpV.Empty()){
        TVec<TIntFltKdV> Res;
        AggrSpV->GetValV(Res);
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
    } else if (!SpV.Empty()) {
        TIntFltKdV Res;
        SpV->GetSparseVec(Res);
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(Res)));
    } else {
        throw TQm::TQmExcept::New("TNodeJsStreamAggr::getValueVector : stream aggregate does not implement IValVec: " + JsSA->SA->GetAggrNm());
    }
}

void TNodeJsStreamAggr::name(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Info);
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
    Info.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(JsSA->SA->GetAggrNm().CStr())));
}


void TNodeJsStreamAggr::val(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Info);
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
    Info.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsSA->SA->SaveJson(-1)));
}

void TNodeJsStreamAggr::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = JS_GET_HOLDER(Info);
    TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
    Info.GetReturnValue().Set(JsSA->SA->IsInit());
}

///////////////////////////////
// QMiner-JavaScript-Stream-Aggr
TNodeJsFuncStreamAggr::TNodeJsFuncStreamAggr(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Local<v8::Object> TriggerVal) : TStreamAggr(_Base, _AggrNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    ThisObj.Reset(Isolate, TriggerVal);
    // Every stream aggregate should implement these two
    QmAssertR(Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onAdd"))).ToChecked() ||
        Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onTime"))).ToChecked() ||
        Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))).ToChecked(), "TNodeJsFuncStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing onAdd/onTime/onStep (any) callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("reset"))).ToChecked()) {
        v8::Local<v8::Value> _ResetFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("reset"))));
        QmAssert(_ResetFun->IsFunction());
        ResetFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_ResetFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))).ToChecked()) {
        v8::Local<v8::Value> _OnStepFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))));
        QmAssert(_OnStepFun->IsFunction());
        OnStepFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnStepFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onTime"))).ToChecked()) {
        v8::Local<v8::Value> _OnTimeFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onTime"))));
        QmAssert(_OnTimeFun->IsFunction());
        OnTimeFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnTimeFun));
    } else if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))).ToChecked()) {
        v8::Local<v8::Value> _OnStepFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))));
        QmAssert(_OnStepFun->IsFunction());
        OnTimeFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnStepFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onAdd"))).ToChecked()) {
        v8::Local<v8::Value> _OnAddFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onAdd"))));
        QmAssert(_OnAddFun->IsFunction());
        OnAddFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnAddFun));
    } else if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))).ToChecked()) {
        v8::Local<v8::Value> _OnStepFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onStep"))));
        QmAssert(_OnStepFun->IsFunction());
        OnAddFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnStepFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onUpdate"))).ToChecked()) {
        v8::Local<v8::Value> _OnUpdateFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onUpdate"))));
        QmAssert(_OnUpdateFun->IsFunction());
        OnUpdateFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnUpdateFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onDelete"))).ToChecked()) {
        v8::Local<v8::Value> _OnDeleteFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("onDelete"))));
        QmAssert(_OnDeleteFun->IsFunction());
        OnDeleteFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_OnDeleteFun));
    }

    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("saveJson"))).ToChecked()) {
        v8::Local<v8::Value> _SaveJsonFun = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("saveJson"))));
        QmAssert(_SaveJsonFun->IsFunction());
        SaveJsonFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_SaveJsonFun));
    }

    // StreamAggr::IsInit
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("init"))).ToChecked()) {
        v8::Local<v8::Value> _IsInit = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("init"))));
        QmAssert(_IsInit->IsFunction());
        IsInitFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_IsInit));
    }

    // StreamAggr::SaveState
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("save"))).ToChecked()) {
        v8::Local<v8::Value> _Save = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("save"))));
        QmAssert(_Save->IsFunction());
        SaveFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_Save));
    }

    // StreamAggr::LoadState
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("load"))).ToChecked()) {
        v8::Local<v8::Value> _Load = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("load"))));
        QmAssert(_Load->IsFunction());
        LoadFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_Load));
    }

    // StreamAggr::SaveStateJson
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("saveStateJson"))).ToChecked()) {
        v8::Local<v8::Value> _SaveStateJson = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("saveStateJson"))));
        QmAssert(_SaveStateJson->IsFunction());
        SaveStateJsonFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_SaveStateJson));
    }

    // StreamAggr::LoadStateJson
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("loadStateJson"))).ToChecked()) {
        v8::Local<v8::Value> _LoadStateJson = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("loadStateJson"))));
        QmAssert(_LoadStateJson->IsFunction());
        LoadStateJsonFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_LoadStateJson));
    }

    // StreamAggr::GetParams
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getParams"))).ToChecked()) {
        v8::Local<v8::Value> _GetParamsJson = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getParams"))));
        QmAssert(_GetParamsJson->IsFunction());
        GetParamsFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetParamsJson));
    }

     // StreamAggr::SetParams
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("setParams"))).ToChecked()) {
       v8::Local<v8::Value> _SetParamsJson = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("setParams"))));
       QmAssert(_SetParamsJson->IsFunction());
       SetParamsFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_SetParamsJson));
    }

    // IInt
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInteger"))).ToChecked()) {
        v8::Local<v8::Value> _GetInt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInteger"))));
        QmAssert(_GetInt->IsFunction());
        GetIntFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetInt));
    }
    // IFlt
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloat"))).ToChecked()) {
        v8::Local<v8::Value> _GetFlt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloat"))));
        QmAssert(_GetFlt->IsFunction());
        GetFltFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetFlt));
    }
    // ITm
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getTimestamp"))).ToChecked()) {
        v8::Local<v8::Value> _GetTm = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getTimestamp"))));
        QmAssert(_GetTm->IsFunction());
        GetTmMSecsFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetTm));
    }
    // IFltIO
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInFloatVector"))).ToChecked()) {
        v8::Local<v8::Value> _GetInFltV = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInFloatVector"))));
        QmAssert(_GetInFltV->IsFunction());
        GetInFltVFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetInFltV));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getOutFloatVector"))).ToChecked()) {
        v8::Local<v8::Value> _GetOutFltV = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getOutFloatVector"))));
        QmAssert(_GetOutFltV->IsFunction());
        GetOutFltVFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetOutFltV));
    }
    // ITmIO
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInTimestampVector"))).ToChecked()) {
        v8::Local<v8::Value> _GetInTmV = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getInTimestampVector"))));
        QmAssert(_GetInTmV->IsFunction());
        GetInTmMSecsVFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetInTmV));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getOutTimestampVector"))).ToChecked()) {
        v8::Local<v8::Value> _GetOutTmV = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getOutTimestampVector"))));
        QmAssert(_GetOutTmV->IsFunction());
        GetOutTmMSecsVFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetOutTmV));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNumberOfRecords"))).ToChecked()) {
        v8::Local<v8::Value> _GetN = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNumberOfRecords"))));
        QmAssert(_GetN->IsFunction());
        GetNFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetN));
    }

    // IFltVec
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatLength"))).ToChecked()) {
        v8::Local<v8::Value> _GetFltLen = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatLength"))));
        QmAssert(_GetFltLen->IsFunction());
        GetFltLenFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetFltLen));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatAt"))).ToChecked()) {
        v8::Local<v8::Value> _GetFltAt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatAt"))));
        QmAssert(_GetFltAt->IsFunction());
        GetFltAtFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetFltAt));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatVector"))).ToChecked()) {
        v8::Local<v8::Value> _GetFltV = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getFloatVector"))));
        QmAssert(_GetFltV->IsFunction());
        GetFltVFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetFltV));
    }

    // INmFlt
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("isNameFloat"))).ToChecked()) {
        v8::Local<v8::Value> _IsNmFlt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("isNameFloat"))));
        QmAssert(_IsNmFlt->IsFunction());
        IsNmFltFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_IsNmFlt));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNameFloat"))).ToChecked()) {
        v8::Local<v8::Value> _GetNmFlt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNameFloat"))));
        QmAssert(_GetNmFlt->IsFunction());
        GetNmFltFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetNmFlt));
    }
    // INmInt
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("isNameInteger"))).ToChecked()) {
        v8::Local<v8::Value> _IsNmInt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("isNameInteger"))));
        QmAssert(_IsNmInt->IsFunction());
        IsNmIntFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_IsNmInt));
    }
    if (Nan::Has(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNameInteger"))).ToChecked()) {
        v8::Local<v8::Value> _GetNmInt = TNodeJsUtil::ToLocal(Nan::Get(TriggerVal, TNodeJsUtil::ToLocal(Nan::New("getNameInteger"))));
        QmAssert(_GetNmInt->IsFunction());
        GetNmIntFun.Reset(Isolate, v8::Local<v8::Function>::Cast(_GetNmInt));
    }
}

TNodeJsFuncStreamAggr::~TNodeJsFuncStreamAggr() {
    ThisObj.Reset();
    // callbacks
    ResetFun.Reset();
    OnStepFun.Reset();
    OnTimeFun.Reset();
    OnAddFun.Reset();
    OnUpdateFun.Reset();
    OnDeleteFun.Reset();
    SaveJsonFun.Reset();
    IsInitFun.Reset();

    // get/set
    GetParamsFun.Reset();
    SetParamsFun.Reset();

    GetIntFun.Reset();
    // IFlt
    GetFltFun.Reset();
    // ITm
    GetTmMSecsFun.Reset();
    // IFltTmIO
    GetInFltVFun.Reset();
    GetInTmMSecsVFun.Reset();
    GetOutFltVFun.Reset();
    GetOutTmMSecsVFun.Reset();
    GetNFun.Reset();
    // IFltVec
    GetFltLenFun.Reset();
    GetFltAtFun.Reset();
    GetFltVFun.Reset();
    // ITmVec
    GetTmLenFun.Reset();
    GetTmAtFun.Reset();
    GetTmVFun.Reset();
    // INmFlt
    IsNmFltFun.Reset();
    GetNmFltFun.Reset();

    // INmInt
    IsNmIntFun.Reset();
    GetNmIntFun.Reset();

    // Serialization
    SaveFun.Reset();
    LoadFun.Reset();
    SaveStateJsonFun.Reset();
    LoadStateJsonFun.Reset();
}

void TNodeJsFuncStreamAggr::Reset() {
    if (!ResetFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, ResetFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        Nan::Call(Callback, This, 0, NULL);

        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (!OnStepFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnStepFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        if (CallerAggr.Empty()) {
            Nan::Call(Callback, This, 0, NULL);
        } else {
            const unsigned Argc = 1;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(CallerAggr)) };
            Nan::Call(Callback, This, Argc, ArgV);
        }
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::OnTime(const uint64& Time, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (!OnTimeFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnTimeFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        if (CallerAggr.Empty()) {
            const unsigned Argc = 1;
            v8::Local<v8::Value> ArgV[Argc] = { Nan::New((double)Time) };
            Nan::Call(Callback, This, Argc, ArgV);
        } else {
            const unsigned Argc = 2;
            v8::Local<v8::Value> ArgV[Argc] = { Nan::New((double)Time),  TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(CallerAggr)) };
            Nan::Call(Callback, This, Argc, ArgV);
        }
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::OnAddRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (!OnAddFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnAddFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        if (CallerAggr.Empty()) {
            const unsigned Argc = 1;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
            Nan::Call(Callback, This, Argc, ArgV);
        } else {
            const unsigned Argc = 2;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)),  TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(CallerAggr)) };
            Nan::Call(Callback, This, Argc, ArgV);
        }

        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::OnUpdateRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (!OnUpdateFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnUpdateFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        if (CallerAggr.Empty()) {
            const unsigned Argc = 1;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
            Nan::Call(Callback, This, Argc, ArgV);
        } else {
            const unsigned Argc = 2;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)),  TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(CallerAggr)) };
            Nan::Call(Callback, This, Argc, ArgV);
        }

        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::OnDeleteRec(const TQm::TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (!OnDeleteFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnDeleteFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        if (CallerAggr.Empty()) {
            const unsigned Argc = 1;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
            Nan::Call(Callback, This, Argc, ArgV);
        } else {
            const unsigned Argc = 2;
            v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)),  TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(CallerAggr)) };
            Nan::Call(Callback, This, Argc, ArgV);
        }

        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

PJsonVal TNodeJsFuncStreamAggr::SaveJson(const int& Limit) const {
    if (!SaveJsonFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SaveJsonFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);
        const unsigned Argc = 1;
        v8::Local<v8::Value> ArgV[Argc] = { Nan::New(Limit) };
        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> ReturnVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(ReturnVal->IsObject(), "Stream aggr JS callback: saveJson didn't return an object.");
        PJsonVal Res = TNodeJsUtil::GetObjJson(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(ReturnVal)));

        QmAssertR(Res->IsDef(), "Stream aggr JS callback: saveJson didn't return a valid JSON.");
        return Res;
    }
    else {
        return TJsonVal::NewObj();
    }
}

bool TNodeJsFuncStreamAggr::IsInit() const {
    if (!IsInitFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, IsInitFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", init did not return a boolean!");
        return Nan::To<bool>(RetVal).FromJust();
    } else {
        return true;
    }
}

void TNodeJsFuncStreamAggr::SaveState(TSOut& SOut) const {
    if (SaveFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::SaveState (called using sa.save) : stream aggregate does not implement a save callback: " + GetAggrNm());
    } else {
        // create TNodeJsFOut and pass it to callback
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        PSOut POut(&SOut);
        TNodeJsFOut* FOut = new TNodeJsFOut(POut);
        EAssertR(!FOut->SOut.Empty(), "Output stream closed!");
        v8::Local<v8::Object> JsFOut = TNodeJsUtil::NewInstance<TNodeJsFOut>(FOut);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SaveFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);
        const unsigned Argc = 1;
        v8::Local<v8::Value> ArgV[Argc] = { JsFOut };

        v8::TryCatch TryCatch(Isolate);
        Nan::Call(Callback, This, Argc, ArgV);
        FOut->SOut.Clr();
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

void TNodeJsFuncStreamAggr::LoadState(TSIn& SIn) {
    if (LoadFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::LoadState (called using sa.load) : stream aggregate does not implement a load callback: " + GetAggrNm());
    } else {
        // create TNodeJsFOut and pass it to callback
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        PSIn PIn(&SIn);
        v8::Local<v8::Object> JsFIn = TNodeJsUtil::NewInstance<TNodeJsFIn>(new TNodeJsFIn(PIn));

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, LoadFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);
        const unsigned Argc = 1;
        v8::Local<v8::Value> ArgV[Argc] = { JsFIn };
        v8::TryCatch TryCatch(Isolate);
        Nan::Call(Callback, This, Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

PJsonVal TNodeJsFuncStreamAggr::SaveStateJson() const {
    if (SaveStateJsonFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::SaveStateJson (called using sa.saveStateJson) : stream aggregate does not implement a save callback: " + GetAggrNm());
    } else {
        // create TNodeJsFOut and pass it to callback
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SaveStateJsonFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        return TNodeJsUtil::GetObjJson(RetVal);
    }
}

void TNodeJsFuncStreamAggr::LoadStateJson(const PJsonVal& State) {
    if (LoadStateJsonFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::LoadStateJson (called using sa.loadStateJson) : stream aggregate does not implement a load callback: " + GetAggrNm());
    } else {
        // create TNodeJsFOut and pass it to callback
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Value> StateObj = TNodeJsUtil::ParseJson(Isolate, State);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, LoadStateJsonFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);
        const unsigned Argc = 1;
        v8::Local<v8::Value> ArgV[Argc] = { StateObj };
        v8::TryCatch TryCatch(Isolate);
        Nan::Call(Callback, This, Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

PJsonVal TNodeJsFuncStreamAggr::GetParams() const {
    if (GetParamsFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::GetParams (called using sa.getParams) : stream aggregate does not implement a getParams callback: " + GetAggrNm());
    } else {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetParamsFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        return TNodeJsUtil::GetObjJson(RetVal);
    }
}

void TNodeJsFuncStreamAggr::SetParams(const PJsonVal& Params) {
    if (SetParamsFun.IsEmpty()) {
        throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::SetParams (called using sa.setParams) : stream aggregate does not implement a setParams callback: " + GetAggrNm());
    } else {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Value> ParamObj = TNodeJsUtil::ParseJson(Isolate, Params);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SetParamsFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);
        const unsigned Argc = 1;
        v8::Local<v8::Value> ArgV[Argc] = { ParamObj };
        v8::TryCatch TryCatch(Isolate);
        Nan::Call(Callback, This, Argc, ArgV);
        TNodeJsUtil::CheckJSExcept(TryCatch);
    }
}

// IInt
int TNodeJsFuncStreamAggr::GetInt() const {
    if (!GetIntFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetIntFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsInt32(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getInteger(): Return type expected to be integer");
        return Nan::To<int>(RetVal).FromJust();
    }
    else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getInteger() callback is empty!");
    }
}

// IFlt
double TNodeJsFuncStreamAggr::GetFlt() const {
    if (!GetFltFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFltFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getFloat(): Return type expected to be a number");
        return Nan::To<double>(RetVal).FromJust();
    }
    else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getFloat() callback is empty!");
    }
}

// ITm
uint64 TNodeJsFuncStreamAggr::GetTmMSecs() const {
    if (!GetTmMSecsFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetTmMSecsFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getTm(): Return type expected to be number");
        uint64 UnixMSecs = (uint64)Nan::To<double>(RetVal).FromJust();
        return TNodeJsUtil::GetCppTimestamp(UnixMSecs);
    }
    else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getTm() callback is empty!");
    }
}

// IFltIO
void TNodeJsFuncStreamAggr::GetInValV(TFltV& ValV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInValV not implemented");
}

void TNodeJsFuncStreamAggr::GetOutValV(TFltV& ValV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetOutValV not implemented");
}

// ITmIO
void TNodeJsFuncStreamAggr::GetInTmMSecsV(TUInt64V& MSecsV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInTmMSecsV not implemented");
}

void TNodeJsFuncStreamAggr::GetOutTmMSecsV(TUInt64V& MSecsV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetOutTmMSecsV not implemented");
}

int TNodeJsFuncStreamAggr::GetN() const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetN not implemented");
}

// IFltVec
int TNodeJsFuncStreamAggr::GetVals() const {
    // here be carefull when writing implementation
    // this method can be called via all derivates of IValVec
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetVals not implemented");
}

void TNodeJsFuncStreamAggr::GetVal(const int& ElN, TFlt& Val) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetVal not implemented");
}

void TNodeJsFuncStreamAggr::GetValV(TFltV& ValV) const {
    if (!GetFltVFun.IsEmpty()) {
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();
        v8::HandleScope HandleScope(Isolate);

        v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFltVFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 0, NULL);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsObject() && TNodeJsUtil::IsClass(v8::Local<v8::Object>::Cast(RetVal), TNodeJsFltV::GetClassId()), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ",GetFltV did not return a vector!");
        TNodeJsFltV* JsVec = TNodeJsUtil::Unwrap<TNodeJsFltV>(v8::Local<v8::Object>::Cast(RetVal));
        ValV = JsVec->Vec;
        // TODO: possible missing part of code
    } else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetValV() callback is empty!");
    }
}

// ITmVec
int TNodeJsFuncStreamAggr::GetTmLen() const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTmLen not implemented");
}

uint64 TNodeJsFuncStreamAggr::GetTm(const int& ElN) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTm not implemented");
}

void TNodeJsFuncStreamAggr::GetTmV(TUInt64V& TmMSecsV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTmV not implemented");
}

// INmFlt
bool TNodeJsFuncStreamAggr::IsNmFlt(const TStr& Nm) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    bool ProvidedIsNmFltFun = !IsNmFltFun.IsEmpty();
    bool ProvidedGetNmFltFun = !GetNmFltFun.IsEmpty();
    bool ProvidedGetFltFun = !GetFltFun.IsEmpty();
    if (ProvidedIsNmFltFun || ProvidedGetNmFltFun || ProvidedGetFltFun) {
        v8::Local<v8::Function> Callback = ProvidedIsNmFltFun ? v8::Local<v8::Function>::New(Isolate, IsNmFltFun) :
            (ProvidedGetNmFltFun ? v8::Local<v8::Function>::New(Isolate, GetNmFltFun) : v8::Local<v8::Function>::New(Isolate, GetFltFun));
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::Local<v8::Value> Argv[1] = { TNodeJsUtil::ToLocal(Nan::New(Nm.CStr())) };
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 1, Argv);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        if (ProvidedIsNmFltFun) {
            QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", isNameFloat(): Return type expected to be a boolean value");
            return  Nan::To<bool>(RetVal).FromJust();
        } else {
            return RetVal->IsNumber();
        }
    } else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", IsNmFlt not implemented");
    }
}

double TNodeJsFuncStreamAggr::GetNmFlt(const TStr& Nm) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    bool ProvidedGetNmFltFun = !GetNmFltFun.IsEmpty();
    bool ProvidedGetFltFun = !GetFltFun.IsEmpty();
    if (ProvidedGetNmFltFun || ProvidedGetFltFun) {
        v8::Local<v8::Function> Callback = ProvidedGetNmFltFun ? v8::Local<v8::Function>::New(Isolate, GetNmFltFun) : v8::Local<v8::Function>::New(Isolate, GetFltFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::Local<v8::Value> Argv[1] = { TNodeJsUtil::ToLocal(Nan::New(Nm.CStr())) };
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 1, Argv);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getNameFloat(): Return type expected to be a number");
        return Nan::To<double>(RetVal).FromJust();
    } else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getNameFloat() callback is empty!");
    }
}

// INmInt
bool TNodeJsFuncStreamAggr::IsNmInt(const TStr& Nm) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    bool ProvidedIsNmIntFun = !IsNmIntFun.IsEmpty();
    bool ProvidedGetNmIntFun = !GetNmIntFun.IsEmpty();
    bool ProvidedGetIntFun = !GetIntFun.IsEmpty();
    if (ProvidedIsNmIntFun || ProvidedGetNmIntFun || ProvidedGetIntFun) {
        v8::Local<v8::Function> Callback = ProvidedIsNmIntFun ? v8::Local<v8::Function>::New(Isolate, IsNmIntFun) :
            (ProvidedGetNmIntFun ? v8::Local<v8::Function>::New(Isolate, GetNmIntFun) : v8::Local<v8::Function>::New(Isolate, GetIntFun));
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::Local<v8::Value> Argv[1] = { TNodeJsUtil::ToLocal(Nan::New(Nm.CStr())) };
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 1, Argv);
        TNodeJsUtil::CheckJSExcept(TryCatch);

        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        if (ProvidedIsNmIntFun) {
            QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", isNameInteger(): Return type expected to be a boolean value");
            return  Nan::To<bool>(RetVal).FromJust();
        } else {
            return RetVal->IsNumber();
        }
    } else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", IsNmInt not implemented");
    }
}

int TNodeJsFuncStreamAggr::GetNmInt(const TStr& Nm) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    bool ProvidedGetNmIntFun = !GetNmIntFun.IsEmpty();
    bool ProvidedGetIntFun = !GetIntFun.IsEmpty();
    if (ProvidedGetNmIntFun || ProvidedGetIntFun) {
        v8::Local<v8::Function> Callback = ProvidedGetNmIntFun ? v8::Local<v8::Function>::New(Isolate, GetNmIntFun) : v8::Local<v8::Function>::New(Isolate, GetIntFun);
        v8::Local<v8::Object> This = v8::Local<v8::Object>::New(Isolate, ThisObj);

        v8::TryCatch TryCatch(Isolate);
        v8::Local<v8::Value> Argv[1] = { TNodeJsUtil::ToLocal(Nan::New(Nm.CStr())) };
        v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, This, 1, Argv);
        TNodeJsUtil::CheckJSExcept(TryCatch);
        TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
        v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
        QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getNameInteger(): Return type expected to be a number");
        return Nan::To<int>(RetVal).FromJust();
    } else {
        throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getNameInteger() callback is empty!");
    }
}

// ISparseVec
int TNodeJsFuncStreamAggr::GetSparseVecLen() const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetSparseVecLen not implemented");
}

TIntFltKd TNodeJsFuncStreamAggr::GetSparseVecVal(const int& ElN) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetSparseVecVal not implemented");
}

void TNodeJsFuncStreamAggr::GetSparseVec(TIntFltKdV& ValV) const {
    throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetSparseVec not implemented");
}

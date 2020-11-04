/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "external_analytics_model.h"

namespace TNodeJsExternalQmAddon {

/////////////////////////////////////////////
// QMiner-JavaScript-Hello-Model

void TNodeJsHelloModel::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsHelloModel>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "hello", _hello);
    NODE_SET_PROTOTYPE_METHOD(tpl, "randomVector", _randomVector);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)).ToLocal());
}

TNodeJsHelloModel* TNodeJsHelloModel::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    return new TNodeJsHelloModel();
}

void TNodeJsHelloModel::hello(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TStr Name = TNodeJsUtil::GetArgStr(Args, 0);
    printf("Hello %s!\n", Name.CStr());
}

void TNodeJsHelloModel::randomVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TRnd Random;
    TFltV Vec(2);
    Vec[0] = Random.GetUniDev();
    Vec[1] = Random.GetUniDev();
    Args.GetReturnValue().Set(TNodeJsFltV::New(Vec));
}

/////////////////////////////////////////////
// QMiner-JavaScript-Bounds-Checker

void TNodeJsBoundsChecker::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsBoundsChecker>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
    // Attach the class to the exports object
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)).ToLocal());
}

TNodeJsBoundsChecker* TNodeJsBoundsChecker::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // Parse LowerBound and UpperBound
    double LowerBound = TFlt::Mn;
    double UpperBound = TFlt::Mx;
    if (Args.Length() > 0) {
        PJsonVal ArgJson = TNodeJsUtil::GetArgJson(Args, 0);
        LowerBound = ArgJson->GetObjNum("lowerBound", TFlt::Mn);
        UpperBound = ArgJson->GetObjNum("upperBound", TFlt::Mx);
    }
    return new TNodeJsBoundsChecker(LowerBound, UpperBound);
}

void TNodeJsBoundsChecker::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // Unwrap model
    TNodeJsBoundsChecker* JsModel = ObjectWrap::Unwrap<TNodeJsBoundsChecker>(Args.Holder());
    // Unwrap input value
    double Val = TNodeJsUtil::GetArgFlt(Args, 0);
    // Run model
    double Result = Val > JsModel->UpperBound ? 1.0 : (Val < JsModel->LowerBound ? -1.0 : 0.0);
    // Wrap result
    Args.GetReturnValue().Set(Nan::New(Result));
}

/////////////////////////////////////////////
// Initialization-And-Registration

void InitExternalAnalyticsModel(v8::Local<v8::Object> ExportsQm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> ExternalModels = v8::Object::New(Isolate);
    //This will attach the HelloModel class to external object
    TNodeJsHelloModel::Init(ExternalModels);
    //This will attach the BoundsChecker class to external object
    TNodeJsBoundsChecker::Init(ExternalModels);

    // Attach all models to qm module under the property "external"
    ExportsQm->Set(TNodeJsUtil::ToLocal(Nan::New("external"), ExternalModels);
}

} // TNodeJsExternalQmAddon namespace
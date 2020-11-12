/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "modinit.h"

using namespace v8;

void InitFs(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsFs::Init(NsObj);
    TNodeJsFIn::Init(NsObj);
    TNodeJsFOut::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitLa(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsLinAlg::Init(NsObj);
    TNodeJsVec<TFlt, TAuxFltV>::Init(NsObj);
    TNodeJsVec<TInt, TAuxIntV>::Init(NsObj);
    TNodeJsVec<TStr, TAuxStrV>::Init(NsObj);
    TNodeJsVec<PJsonVal, TAuxJsonV>::Init(NsObj);

    TNodeJsBoolV::Init(NsObj);
    TNodeJsFltVV::Init(NsObj);
    TNodeJsSpVec::Init(NsObj);
    TNodeJsSpMat::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitHt(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsStrStrH::Init(NsObj);
    TNodeJsStrIntH::Init(NsObj);
    TNodeJsStrFltH::Init(NsObj);

    TNodeJsIntStrH::Init(NsObj);
    TNodeJsIntIntH::Init(NsObj);
    TNodeJsIntFltH::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitQuantiles(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsQuant::TNodeJsTDigest::Init(NsObj);
    TNodeJsQuant::TNodeJsBuffTDigest::Init(NsObj);
    TNodeJsQuant::TNodeJsGk::Init(NsObj);
    TNodeJsQuant::TNodeJsBiasedGk::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitAnalytics(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    // QMiner package
    TNodeJsAnalytics::Init(NsObj);
    TNodeJsSVC::Init(NsObj);
    TNodeJsSVR::Init(NsObj);
    TNodeJsRidgeReg::Init(NsObj);
    TNodeJsSigmoid::Init(NsObj);
    TNodeJsNNAnomalies::Init(NsObj);
    TNodeJsRecLinReg::Init(NsObj);
    TNodeJsLogReg::Init(NsObj);
    TNodeJsPropHaz::Init(NsObj);
    TNodeJsNNet::Init(NsObj);
    TNodeJsTokenizer::Init(NsObj);
    TNodeJsMDS::Init(NsObj);
    TNodeJsKMeans::Init(NsObj);
    TNodeJsDpMeans::Init(NsObj);
    TNodeJsRecommenderSys::Init(NsObj);
    TNodeJsGraphCascade::Init(NsObj);

    InitQuantiles(NsObj, "quantiles");

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitStat(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsStat::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitSnap(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    TNodeJsSnap::Init(NsObj);
    TNodeJsGraph<TUNGraph>::Init(NsObj);
    TNodeJsGraph<TNGraph>::Init(NsObj);
    TNodeJsGraph<TNEGraph>::Init(NsObj);
    TNodeJsNode<TUNGraph>::Init(NsObj);
    TNodeJsNode<TNGraph>::Init(NsObj);
    TNodeJsNode<TNEGraph>::Init(NsObj);
    TNodeJsEdge<TUNGraph>::Init(NsObj);
    TNodeJsEdge<TNGraph>::Init(NsObj);
    TNodeJsEdge<TNEGraph>::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitDeprecated(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    // init methods go here

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitStreamStory(v8::Local<v8::Object> exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> NsObj = Object::New(Isolate);

    // init methods go here
    TNodeJsStreamStory::Init(NsObj);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(NsNm.CStr())), NsObj);
}

void InitQm(v8::Local<v8::Object> exports) {
    #ifdef WIN32
    _setmaxstdio(2048);
    #endif

    // QMiner package
    TNodeJsQm::Init(exports);
    TNodeJsBase::Init(exports);
    TNodeJsStreamAggr::Init(exports);
    TNodeJsStore::Init(exports);
    // the record templates are initiated elsewhere: qm.open, qm.create, base.createStore
    TNodeJsRecByValV::Init(exports);
    TNodeJsRecSet::Init(exports);
    TNodeJsStoreIter::Init(exports);
    TNodeJsIndexKey::Init(exports);

    // feature space
    TNodeJsFtrSpace::Init(exports);
}

void InitExternalQmAddons(v8::Local<v8::Object> exports) {
    TFunRouter<TExportsVoidF>& Router = TExternalQmAddon::CreateOnce();
    TStrV TypeNmV;
    Router.GetTypeNmV(TypeNmV);
    int Len = TypeNmV.Len();
    for (int TypeN = 0; TypeN < Len; TypeN++) {
        Router.Fun(TypeNmV[TypeN])(exports);
    }
}

void Init(v8::Local<v8::Object> exports) {
    InitFs(exports, "fs");
    InitLa(exports, "la");
    InitHt(exports, "ht");
    InitAnalytics(exports, "analytics");
    InitStat(exports, "statistics");
    InitSnap(exports, "snap");
    InitDeprecated(exports, "deprecated");
    InitStreamStory(exports, "streamstory");
    InitQm(exports);
    // Initializes all external objects
    InitExternalQmAddons(exports);
}

NODE_MODULE(qm, Init);

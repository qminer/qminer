/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "modinit.h"

using namespace v8;

void InitFs(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    TNodeJsFs::Init(NsObj);
    TNodeJsFIn::Init(NsObj);
    TNodeJsFOut::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitLa(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    TNodeJsLinAlg::Init(NsObj);
    TNodeJsVec<TFlt, TAuxFltV>::Init(NsObj);
    TNodeJsVec<TInt, TAuxIntV>::Init(NsObj);
    TNodeJsVec<TStr, TAuxStrV>::Init(NsObj);

    TNodeJsBoolV::Init(NsObj);
    TNodeJsFltVV::Init(NsObj);
    TNodeJsSpVec::Init(NsObj);
    TNodeJsSpMat::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitHt(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    TNodeJsStrStrH::Init(NsObj);
    TNodeJsStrIntH::Init(NsObj);
    TNodeJsStrFltH::Init(NsObj);

    TNodeJsIntStrH::Init(NsObj);
    TNodeJsIntIntH::Init(NsObj);
    TNodeJsIntFltH::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitAnalytics(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

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
    TNodeJsRecommenderSys::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitStat(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    TNodeJsStat::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitSnap(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

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

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitDeprecated(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    // init methods go here

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitStreamStory(Handle<Object> Exports, const TStr& NsNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Handle<Object> NsObj = Object::New(Isolate);

    // init methods go here
    TNodeJsStreamStory::Init(NsObj);

    Exports->Set(String::NewFromUtf8(Isolate, NsNm.CStr()), NsObj);
}

void InitQm(Handle<Object> Exports) {
    #ifdef WIN32
    _setmaxstdio(2048); 
    #endif

    // QMiner package
    TNodeJsQm::Init(Exports);
    TNodeJsBase::Init(Exports);
    TNodeJsStreamAggr::Init(Exports);
    TNodeJsStore::Init(Exports);
    // the record templates are initiated elsewhere: qm.open, qm.create, base.createStore
    TNodeJsRecByValV::Init(Exports);
    TNodeJsRecSet::Init(Exports);
    TNodeJsStoreIter::Init(Exports);
    TNodeJsIndexKey::Init(Exports);

    // feature space
    TNodeJsFtrSpace::Init(Exports);
}

void Init(Handle<Object> Exports) {
    InitFs(Exports, "fs");
    InitLa(Exports, "la");
    InitHt(Exports, "ht");
    InitAnalytics(Exports, "analytics");
    InitStat(Exports, "statistics");
    InitSnap(Exports, "snap");
    InitDeprecated(Exports, "deprecated");
    InitStreamStory(Exports, "streamstory");
    InitQm(Exports);
}

NODE_MODULE(qm, Init);

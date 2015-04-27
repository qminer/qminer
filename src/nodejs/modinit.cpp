/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
	TNodeJsSVC::Init(NsObj);
	TNodeJsSVR::Init(NsObj);
	TNodeJsRecLinReg::Init(NsObj);
	TNodeJsLogReg::Init(NsObj);
	TNodeJsExpReg::Init(NsObj);
	TNodeJsRidgeReg::Init(NsObj);
	TNodeJsHMChain::Init(NsObj);
	TNodeJsNNet::Init(NsObj);
	TNodeJsTokenizer::Init(NsObj);

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

void InitQm(Handle<Object> Exports) {
	// QMiner package
	TNodeJsQm::Init(Exports);
	TNodeJsBase::Init(Exports);
	TNodeJsSA::Init(Exports);
	TNodeJsStore::Init(Exports);
	// the record templates are initiated elsewhere: qm.open, qm.create, base.createStore
	TNodeJsRecSet::Init(Exports);
	TNodeJsStoreIter::Init(Exports);
	TNodeJsIndexKey::Init(Exports);

	// feature space
	TNodeJsFtrSpace::Init(Exports);
}

void Init(Handle<Object> Exports, Handle<Object> module) {
	InitFs(Exports, "fs");
	InitLa(Exports, "la");
	InitHt(Exports, "ht");
	InitAnalytics(Exports, "analytics");
	InitStat(Exports, "statistics");
	InitSnap(Exports, "snap");
	InitQm(Exports);
}

NODE_MODULE(qm, Init);

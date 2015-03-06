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

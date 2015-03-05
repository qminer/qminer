#include "modinit.h"

using namespace v8;

void InitLa(Handle<Object> Exports, const TStr& NameSpaceNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	Handle<Object> NameSpaceObj = Object::New(Isolate);

	TNodeJsLinAlg::Init(NameSpaceObj);
	TNodeJsVec<TFlt, TAuxFltV>::Init(NameSpaceObj);
	TNodeJsVec<TInt, TAuxIntV>::Init(NameSpaceObj);
	TNodeJsBoolV::Init(NameSpaceObj);
	TNodeJsFltVV::Init(NameSpaceObj);
	TNodeJsSpVec::Init(NameSpaceObj);
	TNodeJsSpMat::Init(NameSpaceObj);

	Exports->Set(String::NewFromUtf8(Isolate, NameSpaceNm.CStr()), NameSpaceObj);
}

void InitAnalytics(Handle<Object> Exports, const TStr& NameSpaceNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	Handle<Object> NameSpaceObj = Object::New(Isolate);

	// QMiner package
	TNodeJsSVC::Init(NameSpaceObj);
	TNodeJsSVR::Init(NameSpaceObj);
	TNodeJsRecLinReg::Init(NameSpaceObj);
	TNodeJsLogReg::Init(NameSpaceObj);
	TNodeJsExpReg::Init(NameSpaceObj);
	TNodeJsHMChain::Init(NameSpaceObj);
	TNodeJsNNet::Init(NameSpaceObj);
	TNodeJsTokenizer::Init(NameSpaceObj);

	// FIXME remove
	//===============================================
	// File stream
	TNodeJsFIn::Init(Exports);
	TNodeJsFOut::Init(Exports);
	//===============================================

	Exports->Set(String::NewFromUtf8(Isolate, NameSpaceNm.CStr()), NameSpaceObj);
}

void Init(Handle<Object> exports, Handle<Object> module) {
	InitLa(exports, "la");
	InitAnalytics(exports, "analytics");
}

NODE_MODULE(qm1, Init);

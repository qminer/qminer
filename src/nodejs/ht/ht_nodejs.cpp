#include "ht_nodejs.h"

const TStr TAuxStrIntH::ClassId = "StrIntH";
const TStr TAuxStrFltH::ClassId = "StrFltH";
const TStr TAuxStrStrH::ClassId = "StrStrH";
const TStr TAuxIntIntH::ClassId = "IntIntH";
const TStr TAuxIntFltH::ClassId = "IntFltH";
const TStr TAuxIntStrH::ClassId = "IntStrH";

///////////////////////////////
// NodeJs-Hash-Map
template<class TKey, class TDat, class TAux>
v8::Persistent<v8::Function> TNodeJsHash<TKey, TDat, TAux>::constructor;

//void TNodeJsHashUtil::Init(v8::Handle<v8::Object> exports) {
//    NODE_SET_METHOD(exports, "newStrIntH", _newStrIntH);
//    NODE_SET_METHOD(exports, "newStrFltH", _newStrFltH);
//    NODE_SET_METHOD(exports, "newStrStrH", _newStrStrH);
//    NODE_SET_METHOD(exports, "newIntIntH", _newIntIntH);
//    NODE_SET_METHOD(exports, "newIntFltH", _newIntFltH);
//    NODE_SET_METHOD(exports, "newIntStrH", _newIntStrH);
//}

//void TNodeJsHashUtil::newStrIntH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsStrIntH::New());
//}
//
//void TNodeJsHashUtil::newStrFltH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsStrFltH::New());
//}
//
//void TNodeJsHashUtil::newStrStrH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsStrStrH::New());
//}
//
//void TNodeJsHashUtil::newIntIntH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsIntIntH::New());
//}
//
//void TNodeJsHashUtil::newIntFltH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsIntFltH::New());
//}
//
//void TNodeJsHashUtil::newIntStrH(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//    v8::HandleScope HandleScope(Isolate);
//    Args.GetReturnValue().Set(TNodeJsIntStrH::New());
//}

void init(v8::Handle<v8::Object> exports) {
//    TNodeJsHashUtil::Init(exports);

	TNodeJsStrStrH::Init(exports);
    TNodeJsStrIntH::Init(exports);
    TNodeJsStrFltH::Init(exports);

    TNodeJsIntStrH::Init(exports);
    TNodeJsIntIntH::Init(exports);
    TNodeJsIntFltH::Init(exports);
}

NODE_MODULE(ht, init);


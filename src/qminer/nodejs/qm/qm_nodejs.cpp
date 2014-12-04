#include "qm_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-Base
TWPt<TBase> TNodeJsBase::Base = new TBase();

void TNodeJsBase::Init(v8::Handle<v8::Object> exports) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   // Add all methods, getters and setters here.
   NODE_SET_METHOD(exports, "store", store);
   NODE_SET_METHOD(exports, "getStoreList", getStoreList);
   NODE_SET_METHOD(exports, "createStore", createStore);
   NODE_SET_METHOD(exports, "search", search);
   NODE_SET_METHOD(exports, "gc", gc);
   NODE_SET_METHOD(exports, "newStreamAggr", newStreamAggr);
   NODE_SET_METHOD(exports, "getStreamAggr", getStreamAggr);
   NODE_SET_METHOD(exports, "getStreamAggrNames", getStreamAggrNames);

}

void TNodeJsBase::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStoreList(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::search(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::gc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   Base->GarbageCollect();   
   Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsBase::newStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
      
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
   TNodeJsBase::Init(exports);   
}

NODE_MODULE(qm, init)


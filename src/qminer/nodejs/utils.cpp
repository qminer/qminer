#include "utils.h"

/////////////////////////////////////////
// TNodeJsUtil
PJsonVal TNodeJsUtil::GetObjJson(const v8::Local<v8::Object>& Obj) {
//	if (Obj->IsArray()) {
//		// TODO
//	} else {
//		v8::Local<v8::Array> FldVals = Obj->GetOwnPropertyNames();
//
//		for ()
//	}
}

PJsonVal TNodeJsUtil::GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	EAssertR(Args.Length() >= ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
	EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object!");

	return GetObjJson(Args[ArgN]->ToObject());
}

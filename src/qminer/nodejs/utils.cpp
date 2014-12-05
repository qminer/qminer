#include "utils.h"

/////////////////////////////////////////
// TNodeJsUtil
PJsonVal TNodeJsUtil::GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	EAssertR(Args.Length() >= ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
	EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object!");

	return GetObjJson(Args[ArgN]->ToObject());
}

PJsonVal TNodeJsUtil::GetObjJson(const v8::Local<v8::Object>& Obj) {
	if (Obj->IsArray()) {
		PJsonVal JsonArr = TJsonVal::NewArr();
		v8::Array* Arr = v8::Array::Cast(*Obj);

		for (uint i = 0; i < Arr->Length(); i++) {
			v8::Local<v8::Value> Val = Arr->Get(i);

			if (Val->IsArray()) {
				JsonArr->AddToArr(GetObjJson(Val->ToObject()));
			}
			else if (Val->IsBoolean()) {
				JsonArr->AddToArr(Val->BooleanValue());
			}
			else if (Val->IsNumber()) {
				JsonArr->AddToArr(Val->NumberValue());
			}
			else if (Val->IsString() || Val->IsRegExp() || Val->IsDate()) {
				JsonArr->AddToArr(TStr(*v8::String::Utf8Value(Val->ToString())));
			}
			else if (Val->IsObject()) {
				JsonArr->AddToArr(GetObjJson(Val->ToObject()));
			}
		}

		return JsonArr;
	} else {
		PJsonVal JsonVal = TJsonVal::NewObj();

		v8::Local<v8::Array> FldNmV = Obj->GetOwnPropertyNames();
		for (uint i = 0; i < FldNmV->Length(); i++) {
			const v8::Local<v8::Value> FldNmVal = FldNmV->Get(i);

			EAssertR(FldNmVal->IsString(), "TNodeJsUtil::GetObjJson: Property name not string!");

			const TStr FldNm(*v8::String::Utf8Value(FldNmVal->ToString()));

			v8::Local<v8::Value> FldVal = Obj->Get(FldNmVal);

			if (FldVal->IsArray()) {
				JsonVal->AddToObj(FldNm, GetObjJson(FldVal->ToObject()));
			}
			else if (FldVal->IsBoolean()) {
				JsonVal->AddToObj(FldNm, FldVal->BooleanValue());
			}
			else if (FldVal->IsNumber()) {
				JsonVal->AddToObj(FldNm, FldVal->NumberValue());
			}
			else if (FldVal->IsString() || FldVal->IsRegExp() || FldVal->IsDate()) {
				JsonVal->AddToObj(FldNm, TStr(*v8::String::Utf8Value(FldVal->ToString())));
			}
			else if (FldVal->IsObject()) {
				JsonVal->AddToObj(FldNm, GetObjJson(FldVal->ToObject()));
			}
		}

		return JsonVal;
	}
}

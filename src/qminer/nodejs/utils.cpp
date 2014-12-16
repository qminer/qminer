#include "utils.h"

/////////////////////////////////////////
// TNodeJsUtil
PJsonVal TNodeJsUtil::GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	EAssertR(Args.Length() >= ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
	EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object!");

	return GetObjJson(Args[ArgN]->ToObject());
}

v8::Local<v8::Value> TNodeJsUtil::ParseJson(v8::Isolate* Isolate, const PJsonVal& JsonVal) {
	if (!JsonVal->IsDef()) {
		return v8::Undefined(Isolate);
	}
	else if (JsonVal->IsBool()) {
		return v8::Boolean::New(Isolate, JsonVal->GetBool());
	}
	else if (JsonVal->IsNull()) {
		return v8::Null(Isolate);
	}
	else if (JsonVal->IsNum()) {
		return v8::Number::New(Isolate, JsonVal->GetNum());
	}
	else if (JsonVal->IsStr()) {
		return v8::String::NewFromUtf8(Isolate, JsonVal->GetStr().CStr());
	}
	else if (JsonVal->IsArr()) {
		const uint Len = JsonVal->GetArrVals();

		v8::Local<v8::Array> ResArr = v8::Array::New(Isolate, Len);

		for (uint i = 0; i < Len; i++) {
			ResArr->Set(i, ParseJson(Isolate, JsonVal->GetArrVal(i)));
		}

		return ResArr;
	} else if (JsonVal->IsObj()) {
		v8::Local<v8::Object> ResObj = v8::Object::New(Isolate);

		const int NKeys = JsonVal->GetObjKeys();

		for (int i = 0; i < NKeys; i++) {
			TStr Key;	PJsonVal Val;
			JsonVal->GetObjKeyVal(i, Key, Val);

			ResObj->Set(v8::String::NewFromUtf8(Isolate, Key.CStr()), ParseJson(Isolate, Val));
		}

		return ResObj;
	} else {
		throw TExcept::New("Invalid JSON!", "TNodeJsUtil::ParseJson");
	}
}

PJsonVal TNodeJsUtil::GetObjJson(const v8::Local<v8::Object>& Obj) {
	EAssertR(Obj->IsObject(), "TNodeJsUtil::GetObjJson: Cannot parse non-object types!");
	EAssertR(!Obj->IsFunction(), "TNodeJsUtil::GetObjJson: Cannot parse functions!");

	if (Obj->IsUndefined()) {
		return TJsonVal::New();
	}
	if (Obj->IsNull()) {
		return TJsonVal::NewNull();
	}
	else if (Obj->IsBooleanObject()) {
		return TJsonVal::NewBool(Obj->BooleanValue());
	}
	else if (Obj->IsNumberObject()) {
		return TJsonVal::NewNum(Obj->NumberValue());
	}
	else if (Obj->IsStringObject() || Obj->IsRegExp() || Obj->IsDate()) {
		return TJsonVal::NewStr(TStr(*v8::String::Utf8Value(Obj->ToString())));
	}
	else if (Obj->IsArray()) {
		PJsonVal JsonArr = TJsonVal::NewArr();

		v8::Array* Arr = v8::Array::Cast(*Obj);
		for (uint i = 0; i < Arr->Length(); i++) {
			JsonArr->AddToArr(GetObjJson(Arr->Get(i)->ToObject()));
		}

		return JsonArr;
	}
	else {	// object
		PJsonVal JsonVal = TJsonVal::NewObj();

		v8::Local<v8::Array> FldNmV = Obj->GetOwnPropertyNames();
		for (uint i = 0; i < FldNmV->Length(); i++) {
			const TStr FldNm(*v8::String::Utf8Value(FldNmV->Get(i)->ToString()));

			JsonVal->AddToObj(FldNm, GetObjJson(Obj->Get(FldNmV->Get(i))->ToObject()));
		}

		return JsonVal;
	}
}
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "nodeutil.h"

/////////////////////////////////////////
// Node - Utilities

PJsonVal TNodeJsUtil::GetObjJson(const v8::Local<v8::Value>& Val, const bool IgnoreFunc) {
	AssertR(!Val->IsExternal(), "TNodeJsUtil::GetObjJson: Cannot parse v8::External!");

	if (Val->IsObject()) {
		// if we aren't ignoring functions and the object is a function
		// then throw an exception
		EAssertR(IgnoreFunc || !Val->IsFunction(), "TNodeJsUtil::GetObjJson: Cannot parse functions!");

		// parse the object
		if (Val->IsFunction()) {
			return TJsonVal::New();
		}
		else if (Val->IsBooleanObject()) {
			v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(Val);
			return TJsonVal::NewBool(BoolObj->ValueOf());
		}
		else if (Val->IsNumberObject()) {
			return TJsonVal::NewNum(Val->NumberValue());
		}
		else if (Val->IsStringObject() || Val->IsRegExp() || Val->IsDate()) {
			return TJsonVal::NewStr(TStr(*v8::String::Utf8Value(Val->ToString())));
		}
		else if (Val->IsArray()) {
			PJsonVal JsonArr = TJsonVal::NewArr();

			v8::Array* Arr = v8::Array::Cast(*Val);
			for (uint i = 0; i < Arr->Length(); i++) {
				if (!IgnoreFunc || !Arr->Get(i)->IsFunction()) {
					JsonArr->AddToArr(GetObjJson(Arr->Get(i), IgnoreFunc));
				}
			}

			return JsonArr;
		}
		else {	// general object with fields
			PJsonVal JsonVal = TJsonVal::NewObj();
			v8::Local<v8::Object> Obj = Val->ToObject();

			v8::Local<v8::Array> FldNmV = Obj->GetOwnPropertyNames();
			for (uint i = 0; i < FldNmV->Length(); i++) {
				const TStr FldNm(*v8::String::Utf8Value(FldNmV->Get(i)->ToString()));

				v8::Local<v8::Value> FldVal = Obj->Get(FldNmV->Get(i));

				if (!IgnoreFunc || !FldVal->IsFunction()) {
					JsonVal->AddToObj(FldNm, GetObjJson(FldVal, IgnoreFunc));
				}
			}

			return JsonVal;
		}
	}
	else {	// primitive
		if (Val->IsUndefined()) {
			return TJsonVal::New();
		}
		else if (Val->IsNull()) {
			return TJsonVal::NewNull();
		}
		else if (Val->IsBoolean()) {
			return TJsonVal::NewBool(Val->BooleanValue());
		}
		else if (Val->IsNumber()) {
			return TJsonVal::NewNum(Val->NumberValue());
		}
		else if (Val->IsString()) {
			return TJsonVal::NewStr(TStr(*v8::String::Utf8Value(Val->ToString())));
		}
		else {
			// TODO check for v8::Symbol
			throw TExcept::New("TNodeJsUtil::GetObjJson: Unknown v8::Primitive type!");
		}
	}
}

v8::Local<v8::Value> TNodeJsUtil::ParseJson(v8::Isolate* Isolate, const PJsonVal& JsonVal) {
    v8::EscapableHandleScope HandleScope(Isolate);
    
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
        return HandleScope.Escape(v8::Number::New(Isolate, JsonVal->GetNum()));
    }
    else if (JsonVal->IsStr()) {
        return HandleScope.Escape(v8::String::NewFromUtf8(Isolate, JsonVal->GetStr().CStr()));
    }
    else if (JsonVal->IsArr()) {
        const uint Len = JsonVal->GetArrVals();

        v8::Local<v8::Array> ResArr = v8::Array::New(Isolate, Len);

        for (uint i = 0; i < Len; i++) {
            ResArr->Set(i, ParseJson(Isolate, JsonVal->GetArrVal(i)));
        }

        return HandleScope.Escape(ResArr);
    }
    else if (JsonVal->IsObj()) {
        v8::Local<v8::Object> ResObj = v8::Object::New(Isolate);

        const int NKeys = JsonVal->GetObjKeys();

        for (int i = 0; i < NKeys; i++) {
            TStr Key;    PJsonVal Val;
            JsonVal->GetObjKeyVal(i, Key, Val);

            ResObj->Set(v8::String::NewFromUtf8(Isolate, Key.CStr()), ParseJson(Isolate, Val));
        }

        return HandleScope.Escape(ResObj);
    }
    else {
        throw TExcept::New("Invalid JSON!", "TNodeJsUtil::ParseJson");
    }
}

TStr TNodeJsUtil::GetStr(const v8::Local<v8::String>& V8Str) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::String::Utf8Value Utf8(V8Str);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetClass(const v8::Handle<v8::Object> Obj) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Value> ClassNm = Obj->GetHiddenValue(v8::String::NewFromUtf8(Isolate, "class"));
    const bool EmptyP = ClassNm.IsEmpty();
    if (EmptyP) { return ""; }
    v8::String::Utf8Value Utf8(ClassNm);
    return TStr(*Utf8);
}

bool TNodeJsUtil::IsClass(const v8::Handle<v8::Object> Obj, const TStr& ClassNm) {
    TStr ObjClassStr = GetClass(Obj);
    return ObjClassStr == ClassNm;
}

bool TNodeJsUtil::IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& ClassNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // check we have enough arguments
    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()));
    // check argument is an object
	if (!Args[ArgN]->IsObject()) return false;
    // check it's class name
    v8::Handle<v8::Value> Val = Args[ArgN];
    v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);
    TStr ClassStr = GetClass(Data);
    return ClassStr.EqI(ClassNm);
}

bool TNodeJsUtil::IsArgNull(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsNull();
}

bool TNodeJsUtil::IsArgUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() <= ArgN) { return true; }

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsUndefined();
}

bool TNodeJsUtil::IsArgNullOrUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	return IsArgUndef(Args, ArgN) || IsArgNull(Args, ArgN);
}

bool TNodeJsUtil::IsArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsFunction();
}

bool TNodeJsUtil::IsArgObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsObject();
}

bool TNodeJsUtil::IsArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsBoolean();
}

bool TNodeJsUtil::IsArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsNumber();
}

bool TNodeJsUtil::IsArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Handle<v8::Value> Val = Args[ArgN];
    return Val->IsString();
}

bool TNodeJsUtil::IsArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

	v8::Handle<v8::Value> Val = Args[ArgN];
	return Val->IsObject();
}

v8::Handle<v8::Function> TNodeJsUtil::GetArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args,
		const int& ArgN) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsArgFun(Args, ArgN), "The specified argument is not a function!");
	return v8::Handle<v8::Function>::Cast(Args[ArgN]);
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be bool", ArgN));
	v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(Val->ToObject());	
	return static_cast<bool>(BoolObj->ValueOf());
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be bool", ArgN));
	v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(Val->ToObject());
	return static_cast<bool>(BoolObj->ValueOf());
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
            v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
            EAssertR(Val->IsBoolean(),
                     TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()).CStr());
			v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(Val->ToObject());
			return static_cast<bool>(BoolObj->ValueOf());
        }
    }
    return DefVal;
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("TNodeJsUtil::GetArgInt32: Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int", ArgN));
    return Val->Int32Value();
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const int& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int", ArgN));
    return Val->Int32Value();
}


int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() > ArgN, "insufficient number of arguments!");

	EAssertR(Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr())), TStr::Fmt("Argument %d, missing property %s", ArgN, Property.CStr()).CStr());

	v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
	bool IsInt = Val->IsInt32();
	EAssertR(IsInt,
		TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
	return Val->ToNumber()->Int32Value();	
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
            v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
			bool IsInt = Val->IsInt32();
			EAssertR(IsInt,
                     TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
            return Val->ToNumber()->Int32Value();
        }
    }
    return DefVal;
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("TNodeJsUtil::GetArgFlt: Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be number", ArgN));
    return Val->NumberValue();
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const double& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be number", ArgN));
    return Val->NumberValue();
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const double& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
            v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
            EAssertR(Val->IsNumber(),
                TStr::Fmt("Argument %d, property %s expected to be number", ArgN, Property.CStr()).CStr());
            return Val->NumberValue();
        }
    }
    return DefVal;
}

/// Extract argument ArgN as TStr
TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
    v8::String::Utf8Value Utf8(Val);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Handle<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
    v8::String::Utf8Value Utf8(Val);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const TStr& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    if (Args.Length() > ArgN) {
        if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
            v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
            EAssertR(Val->IsString(), TStr::Fmt("Argument %d, property %s expected to be string", ArgN, Property.CStr()));
            v8::String::Utf8Value Utf8(Val);
            return TStr(*Utf8);
        }
    }
    return DefVal;
}

PJsonVal TNodeJsUtil::GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    EAssertR(Args.Length() >= ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object!");
	return GetObjJson(Args[ArgN]->ToObject());
}

bool TNodeJsUtil::IsObjFld(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	return Obj->Has(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
}

bool TNodeJsUtil::IsFldNull(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	if (!IsObjFld(Obj, FldNm)) { return true; }

	// the field exists, check if it is null
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
	return FldVal->IsNull() || FldVal->IsUndefined();
}

bool TNodeJsUtil::IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm, const TStr& ClassId) {
	if (!IsObjFld(Obj, FldNm)) { return false; }

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
	v8::Handle<v8::Object> FldObj = v8::Handle<v8::Object>::Cast(FldVal);

	TStr ClassStr = GetClass(FldObj);
	return ClassStr.EqI(ClassId);
}

bool TNodeJsUtil::IsFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	if (!IsObjFld(Obj, FldNm)) { return false; }

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));

	return FldVal->IsFunction();
}

bool TNodeJsUtil::IsFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	if (!IsObjFld(Obj, FldNm)) { return false; }

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));

	return FldVal->IsInt32() || FldVal->IsUint32();
}

bool TNodeJsUtil::IsFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	if (!IsObjFld(Obj, FldNm)) { return false; }

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));

	return FldVal->IsNumber() || FldVal->IsNumberObject();
}

PJsonVal TNodeJsUtil::GetFldJson(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	return GetObjJson(GetFldObj(Obj, FldNm));
}

v8::Local<v8::Object> TNodeJsUtil::GetFldObj(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsObjFld(Obj, FldNm), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is missing!");
	v8::Handle<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
	EAssertR(FldVal->IsObject(), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not an object");
	v8::Handle<v8::Object> FldObj = v8::Handle<v8::Object>::Cast(FldVal);

	return FldObj;
}

v8::Local<v8::Function> TNodeJsUtil::GetFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsFldFun(Obj, FldNm), "The field is not a function!");

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
	v8::Local<v8::Function> RetFun = v8::Handle<v8::Function>::Cast(FldVal);

	return RetFun;
}

int TNodeJsUtil::GetFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsFldInt(Obj, FldNm), "The field is not an integer!");

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));

	if (FldVal->IsInt32()) {
		return FldVal->Int32Value();
	} else {			// FldVal->IsUint32()
		return FldVal->Uint32Value();
	}
}

double TNodeJsUtil::GetFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsFldFlt(Obj, FldNm), "The field is not an integer!");

	v8::Local<v8::Value> FldVal = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
	return FldVal->NumberValue();
}

double TNodeJsUtil::ExecuteFlt(const v8::Handle<v8::Function>& Fun, const v8::Local<v8::Object>& Arg) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Handle<v8::Value> Argv[1] = { Arg };
	v8::TryCatch TryCatch;
	v8::Handle<v8::Value> RetVal = Fun->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);
	if (TryCatch.HasCaught()) {
		TryCatch.ReThrow();
		return 0;
	}
	EAssertR(RetVal->IsNumber(), "Return type expected to be number");

	return RetVal->NumberValue();
}

void TNodeJsUtil::ExecuteVoid(const v8::Handle<v8::Function>& Fun, const int& ArgC,
		v8::Handle<v8::Value> ArgV[]) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::TryCatch TryCatch;
	Fun->Call(Isolate->GetCurrentContext()->Global(), ArgC, ArgV);
	if (TryCatch.HasCaught()) {
		TryCatch.ReThrow();
		return;
	}
}

void TNodeJsUtil::ExecuteVoid(const v8::Handle<v8::Function>& Fun) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

  v8::TryCatch TryCatch;
  Fun->Call(Isolate->GetCurrentContext()->Global(), 0, nullptr);
  if (TryCatch.HasCaught()) {
    TryCatch.ReThrow();
    return;
  }
}

v8::Local<v8::Value> TNodeJsUtil::V8JsonToV8Str(const v8::Handle<v8::Value>& Json) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Handle<v8::Context> Context = v8::Context::New(Isolate);
	v8::Context::Scope ContextScope(Context);

	v8::Local<v8::Object> JSON = Context->Global()->Get(v8::String::NewFromUtf8(Isolate, "JSON"))->ToObject();
	v8::Local<v8::Value> FunObj = JSON->Get(v8::String::NewFromUtf8(Isolate, "stringify"));
	v8::Local<v8::Function> Fun = v8::Local<v8::Function>::Cast(FunObj);


	v8::TryCatch TryCatch;
	v8::Local<v8::Value> ArgV[1] = { Json };
	v8::Local<v8::Value> JsonStr = Fun->Call(Context->Global(), 1, ArgV);

	if (JsonStr.IsEmpty()) { Isolate->ThrowException(TryCatch.Exception()); }
	return HandleScope.Escape(JsonStr);
}

v8::Local<v8::Value> TNodeJsUtil::GetStrArr(const TStrV& StrV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope EscapableHandleScope(Isolate);
    v8::Local<v8::Array> JsStrV = v8::Array::New(Isolate, StrV.Len());
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        JsStrV->Set(StrN, v8::String::NewFromUtf8(Isolate, StrV[StrN].CStr()));
    }
    return EscapableHandleScope.Escape(JsStrV);
}

PMem TNodeJsUtil::GetArgMem(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	EAssertR(Args.Length() >= ArgN, "TNodeJsUtil::GetArgMem: Invalid number of arguments!");
	EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgMem: Argument is not an object!");

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::Local<v8::Object> Obj = Args[0]->ToObject();
	v8::ExternalArrayType ExternalType = Obj->GetIndexedPropertiesExternalArrayDataType();
	if (ExternalType != v8::ExternalArrayType::kExternalUint8Array) return TMem::New();
	int Len = Obj->GetIndexedPropertiesExternalArrayDataLength();
	return TMem::New(static_cast<char*>(Obj->GetIndexedPropertiesExternalArrayData()), Len);
}

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

THash<TStr, TTriple<TInt, TInt, TInt> > TNodeJsUtil::ObjNameH = THash<TStr, TTriple<TInt, TInt, TInt> >();
TTriple<TInt, TInt, TInt> TNodeJsUtil::ObjCount = TTriple<TInt,TInt,TInt>();
THash<TStr, TStr> TNodeJsUtil::ClassNmAccessorH = THash<TStr, TStr>();

void TNodeJsUtil::CheckJSExcept(const v8::TryCatch& TryCatch) {
    v8::Local<v8::Context> context = Nan::GetCurrentContext();
    if (TryCatch.HasCaught()) {
        Nan::Utf8String Msg(TryCatch.Message()->Get());
        v8::MaybeLocal<v8::Value> TmpStackTrace = TryCatch.StackTrace(context);
        if (TmpStackTrace.IsEmpty()) {
            throw TExcept::New("Javascript exception from callback triggered:" + TStr(*Msg));
        } else {
            Nan::Utf8String StackTrace(TNodeJsUtil::ToLocal(TmpStackTrace));
            throw TExcept::New("Javascript exception from callback triggered:" + TStr(*Msg) + "\n" + TStr(*StackTrace));
        }

    }
}

PJsonVal TNodeJsUtil::GetObjJson(const v8::Local<v8::Value>& Val, const bool& IgnoreFunc, const bool& IgnoreWrappedObj) {
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
            return TJsonVal::NewNum(Nan::To<double>(Val).FromJust());
        }
        else if (Val->IsStringObject() || Val->IsRegExp()) {
            return TJsonVal::NewStr(TStr(*Nan::Utf8String(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Val)))));
        }
        else if (Val->IsDate()) {
            v8::Local<v8::Date> DateObj = v8::Local<v8::Date>::Cast(Val);
            const int64 UnixMSecs = Nan::To<int64>(DateObj).FromJust();
            const uint64 WinMSecs = TNodeJsUtil::GetCppTimestamp(UnixMSecs);
            TTm Tm = TTm::GetTmFromMSecs(WinMSecs);
            return TJsonVal::NewStr(Tm.GetWebLogDateTimeStr(true, "T", true) + "Z");    // XXX is there a better way than + "Z"???
        }
        else if (Val->IsArray()) {
            PJsonVal JsonArr = TJsonVal::NewArr();

            v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Val);
            for (uint i = 0; i < Arr->Length(); i++) {
                if (!IgnoreFunc || !TNodeJsUtil::ToLocal(Nan::Get(Arr, i))->IsFunction()) {
                    JsonArr->AddToArr(GetObjJson(TNodeJsUtil::ToLocal(Nan::Get(Arr, i)), IgnoreFunc, IgnoreWrappedObj));
                }
            }

            return JsonArr;
        }
        else {  // general object with fields
            PJsonVal JsonVal = TJsonVal::NewObj();
            v8::Local<v8::Object> Obj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val));

            TStr ClassNm = GetClass(Obj);
            EAssertR(IgnoreWrappedObj || ClassNm.Empty(), "TNodeJsUtil::GetObjJson: Cannot convert '" + ClassNm + "' to json!");

            if (ClassNm.Empty()) {
                v8::Local<v8::Array> FldNmV = TNodeJsUtil::ToLocal(Nan::GetOwnPropertyNames(Obj));
                for (uint i = 0; i < FldNmV->Length(); i++) {
                    const TStr FldNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::Get(FldNmV, i))));

                    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, TNodeJsUtil::ToLocal(Nan::Get(FldNmV, i))));

                    if (!IgnoreFunc || !FldVal->IsFunction()) {
                        JsonVal->AddToObj(FldNm, GetObjJson(FldVal, IgnoreFunc, IgnoreWrappedObj));
                    }
                }
            }

            return JsonVal;
        }
    }
    else {  // primitive
        if (Val->IsUndefined()) {
            return TJsonVal::New();
        }
        else if (Val->IsNull()) {
            return TJsonVal::NewNull();
        }
        else if (Val->IsBoolean()) {
            return TJsonVal::NewBool(Nan::To<bool>(Val).FromJust());
        }
        else if (Val->IsNumber()) {
            return TJsonVal::NewNum(Nan::To<double>(Val).FromJust());
        }
        else if (Val->IsString()) {
            return TJsonVal::NewStr(TStr(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Val)))));
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
        return Nan::Undefined();
    }
    else if (JsonVal->IsBool()) {
        return Nan::New(JsonVal->GetBool());
    }
    else if (JsonVal->IsNull()) {
        return Nan::Null();
    }
    else if (JsonVal->IsNum()) {
        return HandleScope.Escape(Nan::New(JsonVal->GetNum()));
    }
    else if (JsonVal->IsStr()) {
        return HandleScope.Escape(TNodeJsUtil::ToLocal(Nan::New(JsonVal->GetStr().CStr())));
    }
    else if (JsonVal->IsArr()) {
        const uint Len = JsonVal->GetArrVals();

        v8::Local<v8::Array> ResArr = v8::Array::New(Isolate, Len);

        for (uint i = 0; i < Len; i++) {
            Nan::Set(ResArr, i, ParseJson(Isolate, JsonVal->GetArrVal(i)));
        }

        return HandleScope.Escape(ResArr);
    }
    else if (JsonVal->IsObj()) {
        v8::Local<v8::Object> ResObj = v8::Object::New(Isolate);

        const int NKeys = JsonVal->GetObjKeys();

        for (int i = 0; i < NKeys; i++) {
            TStr Key;    PJsonVal Val;
            JsonVal->GetObjKeyVal(i, Key, Val);
            Nan::Set(ResObj, TNodeJsUtil::ToLocal(Nan::New(Key.CStr())), ParseJson(Isolate, Val));
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
    Nan::Utf8String Utf8(V8Str);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetClass(const v8::Local<v8::Object> Obj) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
#if NODE_MODULE_VERSION >= 48 //NODE_6_0_MODULE_VERSION
    v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
    v8::Local<v8::Private> Private_key = v8::Private::ForApi(Isolate, TNodeJsUtil::ToLocal(Nan::New("class")));
    v8::MaybeLocal<v8::Value> ClassNm = Obj->GetPrivate(Context, Private_key);
    if (ClassNm.IsEmpty()) {
        return "";
    } else {
        Nan::Utf8String Utf8(TNodeJsUtil::ToLocal(ClassNm));
        TStr Class(*Utf8);
        if (Class == "undefined") {
            return "";
        } else {
            return Class;
        }
    }
#else
    v8::Local<v8::Value> ClassNm = Obj->GetHiddenValue(TNodeJsUtil::ToLocal(Nan::New("class"))));
    const bool EmptyP = ClassNm.IsEmpty();
    if (EmptyP) { return ""; }
    Nan::Utf8String Utf8(ClassNm);
    return TStr(*Utf8);
#endif

}

bool TNodeJsUtil::IsClass(const v8::Local<v8::Object> Obj, const TStr& ClassNm) {
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
    v8::Local<v8::Value> Val = Args[ArgN];
    v8::Local<v8::Object> Data = v8::Local<v8::Object>::Cast(Val);
    TStr ClassStr = GetClass(Data);
    return ClassStr.EqI(ClassNm);
}

bool TNodeJsUtil::IsArgNull(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsNull();
}

bool TNodeJsUtil::IsArgUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() <= ArgN) { return true; }

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsUndefined();
}

bool TNodeJsUtil::IsArgNullOrUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    return IsArgUndef(Args, ArgN) || IsArgNull(Args, ArgN);
}

bool TNodeJsUtil::IsArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsFunction();
}

bool TNodeJsUtil::IsArgObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsObject();
}

bool TNodeJsUtil::IsArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsBoolean();
}

bool TNodeJsUtil::IsArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() <= ArgN) { return false; }
    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsInt32();
}

bool TNodeJsUtil::IsArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsNumber();
}

bool TNodeJsUtil::IsArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsString();
}

bool TNodeJsUtil::IsArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN).CStr());

    v8::Local<v8::Value> Val = Args[ArgN];
    return Val->IsObject();
}

bool TNodeJsUtil::IsArgBuffer(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    EAssertR(ArgN < Args.Length() && ArgN >= 0, "Argument index out of bounds.");
    return IsBuffer(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])));
}

bool TNodeJsUtil::IsBuffer(const v8::Local<v8::Object>& Object) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
#if NODE_MODULE_VERSION >= 46 /* Node.js >= v4.x.x */
        return Object->IsUint8Array();
#else
        return Object->HasIndexedPropertiesInExternalArrayData();
#endif
}

v8::Local<v8::Function> TNodeJsUtil::GetArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args,
        const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(IsArgFun(Args, ArgN), "The specified argument is not a function!");
    return v8::Local<v8::Function>::Cast(Args[ArgN]);
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be bool", ArgN));
    v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val)));
    return static_cast<bool>(BoolObj->ValueOf());
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be bool", ArgN));
    v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val)));
    return static_cast<bool>(BoolObj->ValueOf());
}

bool TNodeJsUtil::GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        v8::Local<v8::String> TmpProperty = TNodeJsUtil::ToLocal(Nan::New(Property.CStr()));
        if (Args[ArgN]->IsObject() && Nan::Has(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty).ToChecked()) {
            v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty));
            EAssertR(Val->IsBoolean(),
                     TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()).CStr());
            v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val)));
            return static_cast<bool>(BoolObj->ValueOf());
        }
    }
    return DefVal;
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(IsArgInt32(Args, ArgN), "Argument " + TInt::GetStr(ArgN) + " is not int32!");
    v8::Local<v8::Value> Val = Args[ArgN];
    return Nan::To<int32_t>(Val).FromJust();
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const int& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int", ArgN));
    return Nan::To<int32_t>(Val).FromJust();
}


int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, "insufficient number of arguments!");
    v8::Local<v8::String> TmpProperty = TNodeJsUtil::ToLocal(Nan::New(Property.CStr()));
    EAssertR(Args[ArgN]->IsObject() && Nan::Has(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty).ToChecked(), TStr::Fmt("Argument %d, missing property %s", ArgN, Property.CStr()).CStr());

    v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty));
    bool IsInt = Val->IsInt32();
    EAssertR(IsInt,
        TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
    return Nan::To<int32_t>(Val).FromJust();
}

int TNodeJsUtil::GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        v8::Local<v8::String> TmpProperty = TNodeJsUtil::ToLocal(Nan::New(Property.CStr()));
        if (Args[ArgN]->IsObject() && Nan::Has(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty).ToChecked()) {
            v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty));
            bool IsInt = Val->IsInt32();
            EAssertR(IsInt,
                     TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
            return Nan::To<int32_t>(Val).FromJust();
        }
    }
    return DefVal;
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("TNodeJsUtil::GetArgFlt: Missing argument %d", ArgN));
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be number", ArgN));
    return Nan::To<double>(Val).FromJust();
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const double& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be number", ArgN));
    return Nan::To<double>(Val).FromJust();
}

double TNodeJsUtil::GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const double& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() > ArgN) {
        v8::Local<v8::String> TmpProperty = TNodeJsUtil::ToLocal(Nan::New(Property.CStr()));
        if (Args[ArgN]->IsObject() && Nan::Has(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty).ToChecked()) {
            v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty));
            EAssertR(Val->IsNumber(),
                TStr::Fmt("Argument %d, property %s expected to be number", ArgN, Property.CStr()).CStr());
            return Nan::To<double>(Val).FromJust();
        }
    }
    return DefVal;
}

void TNodeJsUtil::GetArgFltV(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, TFltV& FltV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssert(Args.Length() > ArgN);
    EAssertR(Args[ArgN]->IsArray(), "TNodeJsUtil::GetArgFltV: Argument is not an array!");
    v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Args[ArgN]);

    const int Len = Arr->Length();

    FltV.Gen(Len);
    for (int i = 0; i < Len; i++) {
        v8::Local<v8::Value> ArrVal = TNodeJsUtil::ToLocal(Nan::Get(Arr, i));
        EAssertR(ArrVal->IsNumber(), "TNodeJsUtil::GetArgFltV: Value is not a number!");
        FltV[i] = Nan::To<double>(ArrVal).FromJust();
    }
}

TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
    Nan::Utf8String Utf8(Val);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (ArgN >= Args.Length()) { return DefVal; }
    v8::Local<v8::Value> Val = Args[ArgN];
    EAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
    Nan::Utf8String Utf8(Val);
    return TStr(*Utf8);
}

TStr TNodeJsUtil::GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const TStr& DefVal) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    if (Args.Length() > ArgN) {
        v8::Local<v8::String> TmpProperty = TNodeJsUtil::ToLocal(Nan::New(Property.CStr()));
        if (Args[ArgN]->IsObject() && Nan::Has(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty).ToChecked()) {
            v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), TmpProperty));
            EAssertR(Val->IsString(), TStr::Fmt("Argument %d, property %s expected to be string", ArgN, Property.CStr()));
            Nan::Utf8String Utf8(Val);
            return TStr(*Utf8);
        }
    }
    return DefVal;
}

PJsonVal TNodeJsUtil::GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& IgnoreFunc, const bool& IgnoreWrappedObj) {
    EAssertR(Args.Length() > ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object, number or boolean!");
    return GetObjJson(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])), IgnoreFunc, IgnoreWrappedObj);
}

PJsonVal TNodeJsUtil::GetArgToNmJson(const v8::FunctionCallbackInfo<v8::Value>& Args,
        const int& ArgN) {
    EAssertR(Args.Length() > ArgN, "TNodeJsUtil::GetArgJson: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgJson: Argument is not an object, number or boolean!");
    return GetObjToNmJson(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[ArgN])));
}

void TNodeJsUtil::GetArgIntV(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN,
        TIntV& IntV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, "TNodeJsUtil::GetArgIntV: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsArray(), "TNodeJsUtil::GetArgIntV: argument is not an array!");

    v8::Local<v8::Array> JsIntV = v8::Local<v8::Array>::Cast(Args[ArgN]);

    const int Len = JsIntV->Length();
    if (IntV.Len() != Len) { IntV.Gen(Len); }

    for (int i = 0; i < Len; i++) {
        v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(JsIntV, i));
        EAssertR(Val->IsNumber() || Val->IsInt32(), "TNodeJsUtil::GetArgIntV: value is not an integer!");
        IntV[i] = static_cast<int>(Nan::To<double>(Val).FromJust());
    }
}

void TNodeJsUtil::GetArgIntVV(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN,
        TVec<TIntV>& IntVV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() > ArgN, "TNodeJsUtil::GetArgIntVV: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsArray(), "TNodeJsUtil::GetArgIntVV: argument is not an array!");

    v8::Local<v8::Array> JsIntVV = v8::Local<v8::Array>::Cast(Args[ArgN]);

    const int OuterLen = JsIntVV->Length();
    IntVV.Gen(OuterLen);

    for (int i = 0; i < OuterLen; i++) {
        v8::Local<v8::Value> OuterVal = TNodeJsUtil::ToLocal(Nan::Get(JsIntVV, i));
        EAssertR(OuterVal->IsArray(), "TNodeJsUtil::GetArgIntVV: value is not an array!");
        v8::Local<v8::Array> JsIntV = v8::Local<v8::Array>::Cast(OuterVal);
        const int InnerLen = JsIntV->Length();
        IntVV[i].Gen(InnerLen);

        for (int j = 0; j < InnerLen; j++) {
            v8::Local<v8::Value> InnerVal = TNodeJsUtil::ToLocal(Nan::Get(JsIntV, j));
            EAssertR(InnerVal->IsNumber() || InnerVal->IsInt32(), "TNodeJsUtil::GetArgIntVV: value is not an integer!");
            IntVV[i][j] = Nan::To<int>(InnerVal).FromJust();
        }
    }
}

bool TNodeJsUtil::IsObjFld(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    return Nan::Has(Obj, FldNmStr).ToChecked();
}

bool TNodeJsUtil::IsFldNull(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    if (!IsObjFld(Obj, FldNm)) { return true; }

    // the field exists, check if it is null
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));
    return FldVal->IsNull() || FldVal->IsUndefined();
}

bool TNodeJsUtil::IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm, const TStr& ClassId) {
    if (!IsObjFld(Obj, FldNm)) { return false; }

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));
    v8::Local<v8::Object> FldObj = v8::Local<v8::Object>::Cast(FldVal);

    TStr ClassStr = GetClass(FldObj);
    return ClassStr.EqI(ClassId);
}

bool TNodeJsUtil::IsFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    if (!IsObjFld(Obj, FldNm)) { return false; }

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));

    return FldVal->IsFunction();
}

bool TNodeJsUtil::IsFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    if (!IsObjFld(Obj, FldNm)) { return false; }

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));

    return FldVal->IsInt32() || FldVal->IsUint32();
}

bool TNodeJsUtil::IsFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    if (!IsObjFld(Obj, FldNm)) { return false; }

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));

    return FldVal->IsNumber() || FldVal->IsNumberObject();
}

PJsonVal TNodeJsUtil::GetFldJson(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    return GetObjJson(GetFldObj(Obj, FldNm));
}

v8::Local<v8::Object> TNodeJsUtil::GetFldObj(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);

    EAssertR(IsObjFld(Obj, FldNm), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is missing!");
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));
    EAssertR(FldVal->IsObject(), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not an object");
    v8::Local<v8::Object> FldObj = v8::Local<v8::Object>::Cast(FldVal);

    return HandleScope.Escape(FldObj);
}

v8::Local<v8::Function> TNodeJsUtil::GetFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);

    EAssertR(IsFldFun(Obj, FldNm), "The field is not a function!");
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));
    v8::Local<v8::Function> RetFun = v8::Local<v8::Function>::Cast(FldVal);

    return HandleScope.Escape(RetFun);
}

int TNodeJsUtil::GetFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(IsFldInt(Obj, FldNm), "The field is not an integer!");
    v8::Local<v8::String> FldNmStr = TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()));
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, FldNmStr));

    if (FldVal->IsInt32()) {
        return Nan::To<int32_t>(FldVal).FromJust();
    } else {            // FldVal->IsUint32()
        return Nan::To<uint32_t>(FldVal).FromJust();
    }
}

double TNodeJsUtil::GetFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(IsFldFlt(Obj, FldNm), "The field is not an integer!");
    v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, TNodeJsUtil::ToLocal(Nan::New(FldNm.CStr()))));
    return Nan::To<double>(FldVal).FromJust();
}

double TNodeJsUtil::ExecuteFlt(const v8::Local<v8::Function>& Fun, const v8::Local<v8::Object>& Arg) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Value> Argv[1] = { Arg };
    v8::TryCatch TryCatch(Isolate);
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);
    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
    v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
    EAssertR(RetVal->IsNumber(), "Return type expected to be number");
    return Nan::To<double>(RetVal).FromJust();
}

PJsonVal TNodeJsUtil::ExecuteJson(const v8::Local<v8::Function>& Fun,
        const v8::Local<v8::Object>& Arg1, const v8::Local<v8::Object>& Arg2) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    const int ArgC = 2;
    v8::Local<v8::Value> ArgV[ArgC] = { Arg1, Arg2 };
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 2, ArgV);
    TNodeJsUtil::CheckJSExcept(TryCatch);
    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
    v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
    return GetObjJson(RetVal);
}

void TNodeJsUtil::ExecuteVoid(const v8::Local<v8::Function>& Fun, const int& ArgC,
        v8::Local<v8::Value> ArgV[]) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);
    Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), ArgC, ArgV);
    TNodeJsUtil::CheckJSExcept(TryCatch);
}

void TNodeJsUtil::ExecuteVoid(const v8::Local<v8::Function>& Fun,
        const v8::Local<v8::Value>& Arg1, const v8::Local<v8::Value>& Arg2) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    const int ArgC = 2;
    v8::Local<v8::Value> ArgV[ArgC] = { Arg1, Arg2 };
    Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 2, ArgV);

    if (TryCatch.HasCaught()) {
        Nan::Utf8String Msg(TryCatch.Message()->Get());
        throw TExcept::New("Exception while executing VOID: " + TStr(*Msg));
    }
}

void TNodeJsUtil::ExecuteVoid(const v8::Local<v8::Function>& Fun) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::TryCatch TryCatch(Isolate);
    Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 0, nullptr);
    if (TryCatch.HasCaught()) {
        Nan::Utf8String Msg(TryCatch.Message()->Get());
        throw TExcept::New("Exception while executing VOID: " + TStr(*Msg));
    }
}

void TNodeJsUtil::ExecuteErr(const v8::Local<v8::Function>& Fun, const PExcept& Except) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    const TStr& Msg = Except->GetMsgStr();
    v8::Local<v8::String> V8Msg = TNodeJsUtil::ToLocal(Nan::New(Msg.CStr()));
    v8::Local<v8::Value> Err = v8::Exception::Error(V8Msg);

    const int ArgC = 1;
    v8::Local<v8::Value> ArgV[ArgC] = { Err };
    Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), ArgC, ArgV);
    if (TryCatch.HasCaught()) {
        Nan::Utf8String Msg(TryCatch.Message()->Get());
        throw TExcept::New("Exception while executing ERROR: " + TStr(*Msg));
    }
}

uint64 TNodeJsUtil::GetTmMSecs(v8::Local<v8::Value>& Value) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Value->IsDate()) {
        v8::Local<v8::Date> Date = v8::Local<v8::Date>::Cast(Value);
        return GetTmMSecs(Date);
    }
    else if (Value->IsString()) {
        Nan::Utf8String Utf8(Value);
        TTm Tm = TTm::GetTmFromWebLogDateTimeStr(TStr(*Utf8), '-', ':', '.', 'T');
        return TTm::GetMSecsFromTm(Tm);
    }
    else {
        EAssertR(Value->IsNumber(), "Date is not in a representation of a string, date or number!");
        return GetCppTimestamp(Nan::To<int64>(Value).FromJust());
    }
}

uint64 TNodeJsUtil::GetArgTmMSecs(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);

   EAssert(!IsArgUndef(Args, ArgN));

   v8::Local<v8::Value> Val = Args[ArgN];
   return GetTmMSecs(Val);
}


void TNodeJsUtil::ThrowJsException(v8::Isolate* Isolate, const PExcept& Except) {
    Isolate->ThrowException(v8::Exception::TypeError(
        TNodeJsUtil::ToLocal(Nan::New((TStr("[addon] Exception in constructor call: ") + Except->GetStr()).CStr()))
    ));
}

v8::Local<v8::Value> TNodeJsUtil::V8JsonToV8Str(const v8::Local<v8::Value>& Json) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);

    v8::Local<v8::Context> Context = v8::Context::New(Isolate);
    v8::Context::Scope ContextScope(Context);

    v8::Local<v8::String> FldNm = TNodeJsUtil::ToLocal(Nan::New("JSON"));
    v8::Local<v8::Object> JSON = v8::Local<v8::Object>::Cast(TNodeJsUtil::ToLocal(Nan::Get(Context->Global(), FldNm)));
    v8::Local<v8::Value> FunObj = TNodeJsUtil::ToLocal(Nan::Get(JSON, TNodeJsUtil::ToLocal(Nan::New("stringify"))));
    v8::Local<v8::Function> Fun = v8::Local<v8::Function>::Cast(FunObj);

    v8::TryCatch TryCatch(Isolate);
    v8::Local<v8::Value> ArgV[1] = { Json };
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Fun, Context->Global(), 1, ArgV);
    TNodeJsUtil::CheckJSExcept(TryCatch);
    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
    v8::Local<v8::Value> JsonStr = TNodeJsUtil::ToLocal(Tmp);
    return HandleScope.Escape(JsonStr);
}

v8::Local<v8::Value> TNodeJsUtil::GetStrArr(const TStrV& StrV) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope EscapableHandleScope(Isolate);
    v8::Local<v8::Array> JsStrV = v8::Array::New(Isolate, StrV.Len());
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        v8::Local<v8::String> TmpFldNm = TNodeJsUtil::ToLocal(Nan::New(StrV[StrN].CStr()));
        Nan::Set(JsStrV, StrN, TmpFldNm);
    }
    return EscapableHandleScope.Escape(JsStrV);
}

v8::Local<v8::Object> TNodeJsUtil::NewBuffer(const char* ChA, const size_t& Len) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
#if NODE_MODULE_VERSION >= 46 /* From Node.js v4.0.0 on */
    // The function node::Buffer::New returns a v8::MaybeLocal<v8::Object>,
    // which is just a wrapper around v8::Local<> that "enforces a check
    // whether v8::Local<> is empty before it can be used." For details, see
    // http://v8.paulfryzel.com/docs/master/singletonv8_1_1_maybe_local.html
    return HandleScope.Escape(TNodeJsUtil::ToLocal(node::Buffer::Copy(Isolate, ChA, Len)));
#else
    return HandleScope.Escape(TNodeJsUtil::ToLocalnode::Buffer::New(Isolate, ChA, Len)));
#endif
}

TMem TNodeJsUtil::GetArgMem(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    EAssertR(Args.Length() > ArgN, "TNodeJsUtil::GetArgMem: Invalid number of arguments!");
    EAssertR(Args[ArgN]->IsObject(), "TNodeJsUtil::GetArgMem: Argument is not an object!");

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Object> Obj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0]));
#if NODE_MODULE_VERSION >= 46 /* Node.js >= v4.0.0 */
    if (!Obj->IsUint8Array()) return TMem();
    return TMem(node::Buffer::Data(Obj), (int) node::Buffer::Length(Obj));
#else
    v8::ExternalArrayType ExternalType = Obj->GetIndexedPropertiesExternalArrayDataType();
    if (ExternalType != v8::ExternalArrayType::kExternalUint8Array) return TMem::New();
    int Len = Obj->GetIndexedPropertiesExternalArrayDataLength();
    return TMem(static_cast<char*>(Obj->GetIndexedPropertiesExternalArrayData()), Len);
#endif
}

void TNodeJsUtil::SetPrivate(
    v8::Local<v8::Object> Object,
    v8::Local<v8::String> Key,
    v8::Local<v8::Value> Value) {
#if NODE_MODULE_VERSION >= 48 //NODE_6_0_MODULE_VERSION
    v8::Isolate *Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> Context = Isolate->GetCurrentContext();
    v8::Local<v8::Private> Private_key = v8::Private::ForApi(Isolate, Key);
    Object->SetPrivate(Context, Private_key, Value);
#else
    Object->SetHiddenValue(Key, Value);
#endif
}

uint64 TNodeJsUtil::GetTmMSecs(v8::Local<v8::Date>& Date) {
    return GetCppTimestamp(Nan::To<int64>(Date).FromJust());
}


//////////////////////////////////////////////////////
// Async Stuff
TNodeJsAsyncUtil::TMainTaskWrapper::TMainTaskWrapper(TMainThreadTask* _Task, const bool& _DelTask):
        Task(_Task),
        DelTask(_DelTask) {}

TNodeJsAsyncUtil::TMainBlockTaskWrapper::TMainBlockTaskWrapper(TMainThreadTask* Task,
            const bool& DelTask):
        TMainTaskWrapper(Task, DelTask),
        Semaphore() {}

TNodeTask::TNodeTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync):
        Callback(),
        ArgPersist(),
        Except(),
        AsyncP(IsAsync) {

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Array> ArgsArr = v8::Array::New(Isolate, Args.Length() + 1);

    for (int ArgN = 0; ArgN < Args.Length(); ArgN++) {
        Nan::Set(ArgsArr, ArgN, Args[ArgN]);
    }
    Nan::Set(ArgsArr, Args.Length(), JS_GET_HOLDER(Args));

    ArgPersist.Reset(Isolate, ArgsArr);
}

TNodeTask::~TNodeTask() {
    Callback.Reset();
    ArgPersist.Reset();
}

v8::Local<v8::Value> TNodeTask::WrapResult() {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);

    v8::Local<v8::Value> Result = Nan::Undefined();

    return HandleScope.Escape(Result);
}

void TNodeTask::AfterRun() {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(!Callback.IsEmpty(), "The callback was not defined!");
    v8::Local<v8::Function> Fun = v8::Local<v8::Function>::New(Isolate, Callback);

    if (HasExcept()) {
        TNodeJsUtil::ExecuteErr(Fun, Except);
    } else {
        v8::Local<v8::Value> Result = WrapResult();
        TNodeJsUtil::ExecuteVoid(Fun, Nan::Undefined(), Result);
    }
}

void TNodeTask::AfterRunSync(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (HasExcept()) { throw Except; }

    Args.GetReturnValue().Set(WrapResult());
}

void TNodeTask::ExtractCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    Callback.Reset(Isolate, GetCallback(Args));
}

//////////////////////////////////////////////////////
// Node - Asynchronous Utilities
TCriticalSection TNodeJsAsyncUtil::UvSection;

TNodeJsAsyncUtil::TAsyncHandleType TNodeJsAsyncUtil::GetHandleType(const uv_async_t* UvAsync) {
    TLock Lock(UvSection);

    TAsyncHandleConfig* Config = static_cast<TAsyncHandleConfig*>(UvAsync->data);
    return Config->HandleType;
}

void TNodeJsAsyncUtil::SetAsyncData(TMainThreadHandle* UvAsync, TMainTaskWrapper* TaskWrapper) {
    TLock Lock(UvSection);

    TAsyncHandleConfig* Config = static_cast<TAsyncHandleConfig*>(UvAsync->data);

    if (Config->TaskWrapper != nullptr) {
        Config->TaskWrapper->DelTask = true;
        delete Config->TaskWrapper;
        Config->TaskWrapper = nullptr;
    }

    Config->TaskWrapper = TaskWrapper;

    AssertR(Config->TaskWrapper != nullptr, "Task wrapper is a null pointer!");
    AssertR(Config->TaskWrapper->Task != nullptr, "Task data is a null pointer!");
}

TNodeJsAsyncUtil::TMainTaskWrapper* TNodeJsAsyncUtil::ExtractAndClearData(TMainThreadHandle* UvAsync) {
    TLock Lock(UvSection);

    TAsyncHandleConfig* Config = static_cast<TAsyncHandleConfig*>(UvAsync->data);

    TMainTaskWrapper* Result = Config->TaskWrapper;
    Config->TaskWrapper = nullptr;

    return Result;
}

void TNodeJsAsyncUtil::OnMain(TMainThreadHandle* UvAsync) {
    TMainTaskWrapper* TaskWrapper = nullptr;

    try {
        TaskWrapper = ExtractAndClearData(UvAsync);

        // libuv does not always merge the tasks, so it might be
        // that this task was already processed by the previous request
        if (TaskWrapper == nullptr) { return; }

        TMainThreadTask* Task = TaskWrapper->Task;
        Task->Run();
    } catch (const PExcept& Except) {
        printf("Exception on main thread: %s!", Except->GetMsgStr().CStr());
    }

    // clean up
    if (TaskWrapper != nullptr) {
        delete TaskWrapper;
    }
}

void TNodeJsAsyncUtil::OnMainBlock(TMainThreadHandle* UvAsync) {
    TMainBlockTaskWrapper* TaskWrapper = nullptr;

    try {
        TaskWrapper = (TMainBlockTaskWrapper*) ExtractAndClearData(UvAsync);
        TMainThreadTask* Task = TaskWrapper->Task;
        Task->Run();
    } catch (const PExcept& Except) {
        printf("Exception on main thread: %s!", Except->GetMsgStr().CStr());
    }

    // release the semaphore
    uv_sem_post(&TaskWrapper->Semaphore);
    uv_sem_destroy(&TaskWrapper->Semaphore);

    // clean up
    if (TaskWrapper != nullptr) {
        delete TaskWrapper;
    }
}

void TNodeJsAsyncUtil::OnWorker(TWorkerThreadHandle* UvReq) {
    TWorkerData* Task = static_cast<TWorkerData*>(UvReq->data);

    try {
        Task->Task->Run();
    } catch (const PExcept& Except) {
        printf("Exception on worker thread: %s!", Except->GetMsgStr().CStr());
    }
}

void TNodeJsAsyncUtil::AfterOnWorker(TWorkerThreadHandle* UvReq, int Status) {
    TWorkerData* Task = static_cast<TWorkerData*>(UvReq->data);

    try {
        Task->Task->AfterRun();
    } catch (const PExcept& Except) {
        printf("Exception when calling callback: %s!", Except->GetMsgStr().CStr());
    }

    delete Task;
    delete UvReq;
}

TMainThreadHandle* TNodeJsAsyncUtil::NewBlockingHandle() {
    TMainThreadHandle* UvAsync = new TMainThreadHandle;

    UvAsync->data = new TAsyncHandleConfig(ahtBlocking);

    EAssertR(uv_async_init(uv_default_loop(), UvAsync, OnMainBlock) == 0, "Failed to initialize async handle!");
    return UvAsync;
}

TMainThreadHandle* TNodeJsAsyncUtil::NewHandle() {
    TMainThreadHandle* UvAsync = new TMainThreadHandle;

    UvAsync->data = new TAsyncHandleConfig(ahtAsync);

    EAssertR(uv_async_init(uv_default_loop(), UvAsync, OnMain) == 0, "Failed to initialize async handle!");
    return UvAsync;
}

void TNodeJsAsyncUtil::DelHandle(TMainThreadHandle* UvAsync) {
    TAsyncHandleConfig* Config = static_cast<TAsyncHandleConfig*>(UvAsync->data);
    uv_close((uv_handle_t*) UvAsync, InternalDelHandle<TMainThreadHandle>);
    delete Config;
}


void TNodeJsAsyncUtil::ExecuteOnMain(TMainThreadTask* Task, uv_async_t* UvAsync, const bool& DelTask) {
    TAsyncHandleType HandleType = GetHandleType(UvAsync);
    switch (HandleType) {
    case ahtAsync: {
        if (!DelTask) {
            delete Task;
            EAssertR(false, "Non-blocking tasks must be automatically deleted!");
        }
        SetAsyncData(UvAsync, new TMainTaskWrapper(Task, DelTask));
        // uv_async_send is thread safe
        uv_async_send(UvAsync);
        break;
    }
    case ahtBlocking: {
        TMainBlockTaskWrapper* TaskWrapper = new TMainBlockTaskWrapper(Task, DelTask);
        SetAsyncData(UvAsync, TaskWrapper);
        // initialize the semaphore
        int Err = uv_sem_init(&TaskWrapper->Semaphore, 0);

        if (Err != 0) { // check if we succeeded initializing the semaphore
            delete Task;
            throw TExcept::New("Failed to create a semaphore, code: " + TInt::GetStr(Err) + "!");
        } else {
            // uv_async_send is thread safe
            uv_async_send(UvAsync);
            uv_sem_wait(&TaskWrapper->Semaphore);
        }

        break;
    }
    default: {
        throw TExcept::New("Unknown handle type: " + TInt::GetStr(HandleType));
    }
    }
}

void TNodeJsAsyncUtil::ExecuteOnWorker(TAsyncTask* Task) {
    uv_work_t* UvReq = new uv_work_t;
    UvReq->data = new TWorkerData(Task);

    uv_queue_work(uv_default_loop(), UvReq, OnWorker, AfterOnWorker);
}

PJsonVal TNodeJsUtil::GetObjToNmJson(const v8::Local<v8::Value>& Val) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Val->IsObject()) {
        // we don't allow functions in the JSON configuration
        EAssertR(!Val->IsFunction(), "TNodeJsUtil::GetObjToNmJson: Cannot parse functions!");

        // first check if we encountered an object that needs to be replaced with
        // its name or ID. Is there a better way to do this??
        const TStr ClassId = TNodeJsUtil::GetClass(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val)));

        if (!ClassId.Empty()) {
            if (ClassNmAccessorH.IsKey(ClassId)) {

                TStr NmAccessor = TNodeJsUtil::GetClassNmAccessor(ClassId);
                TStr Name = TStr(*Nan::Utf8String(TNodeJsUtil::ToLocal(Nan::Get(
                    TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val)),
                    TNodeJsUtil::ToLocal(Nan::New(NmAccessor.CStr()))
                ))));
                return TJsonVal::NewStr(Name);
            }
            else {
                throw TExcept::New("Invalid class ID when parsing configuration: " + ClassId);
            }
        }

        else if (Val->IsBooleanObject()) {
            v8::Local<v8::BooleanObject> BoolObj = v8::Local<v8::BooleanObject>::Cast(Val);
            return TJsonVal::NewBool(BoolObj->ValueOf());
        }
        else if (Val->IsNumberObject()) {
            return TJsonVal::NewNum(Nan::To<double>(Val).FromJust());
        }
        else if (Val->IsStringObject() || Val->IsRegExp() || Val->IsDate()) {
            return TJsonVal::NewStr(TStr(*Nan::Utf8String(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Val)))));
        }
        else if (Val->IsArray()) {
            PJsonVal JsonArr = TJsonVal::NewArr();

            v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Val);
            for (uint i = 0; i < Arr->Length(); i++) {
                JsonArr->AddToArr(GetObjToNmJson(TNodeJsUtil::ToLocal(Nan::Get(Arr, i))));
            }

            return JsonArr;
        }
        else {  // general object with fields
            PJsonVal JsonVal = TJsonVal::NewObj();
            v8::Local<v8::Object> Obj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Val));

            v8::Local<v8::Array> FldNmV = TNodeJsUtil::ToLocal(Nan::GetOwnPropertyNames(Obj));
            for (uint i = 0; i < FldNmV->Length(); i++) {
                const TStr FldNm(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::Get(FldNmV, i))));
                v8::Local<v8::Value> FldVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, TNodeJsUtil::ToLocal(Nan::Get(FldNmV, i))));

                const PJsonVal FldJson = GetObjToNmJson(FldVal);
                JsonVal->AddToObj(FldNm, FldJson);
            }

            return JsonVal;
        }
    }
    else {  // primitive
        if (Val->IsUndefined()) {
            return TJsonVal::New();
        }
        else if (Val->IsNull()) {
            return TJsonVal::NewNull();
        }
        else if (Val->IsBoolean()) {
            return TJsonVal::NewBool(Nan::To<bool>(Val).FromJust());
        }
        else if (Val->IsNumber()) {
            const double& NumVal = Nan::To<double>(Val).FromJust();
            return TJsonVal::NewNum(NumVal);
        }
        else if (Val->IsString()) {
            return TJsonVal::NewStr(TStr(*Nan::Utf8String (TNodeJsUtil::ToLocal(Nan::To<v8::String>(Val)))));
        }
        else {
            // TODO check for v8::Symbol
            throw TExcept::New("TNodeJsUtil::GetObjJson: Unknown v8::Primitive type!");
        }
    }
}

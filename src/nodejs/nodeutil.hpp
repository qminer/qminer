/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_NODEJS_UTILS_HPP
#define QMINER_NODEJS_UTILS_HPP


//////////////////////////////////////////////////////
// Node - Utilities
PJsonVal TNodeJsUtil::GetObjToNmJson(const v8::Local<v8::Value>& Val) {
    if (Val->IsObject()) {
        // we don't allow functions in the JSON configuration
        EAssertR(!Val->IsFunction(), "TNodeJsUtil::GetObjToNmJson: Cannot parse functions!");

        // first check if we encountered an object that needs to be replaced with
        // its name or ID. Is there a better way to do this??
        const TStr ClassId = TNodeJsUtil::GetClass(Val->ToObject());

        if (!ClassId.Empty()) {
            if (ClassId == TNodeJsStreamAggr::GetClassId()) {
                // convert StreamAggregates to their names
                const TNodeJsStreamAggr* JsStreamAggr = TNodeJsUtil::Unwrap<TNodeJsStreamAggr>(Val->ToObject());
                return TJsonVal::NewStr(JsStreamAggr->SA->GetAggrNm());
            }
            else if (ClassId == TNodeJsStore::GetClassId()) {
                // convert stores to their names
                const TNodeJsStore* JsStore = TNodeJsUtil::Unwrap<TNodeJsStore>(Val->ToObject());
                return TJsonVal::NewStr(JsStore->Store->GetStoreNm());
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
            return TJsonVal::NewNum(Val->NumberValue());
        }
        else if (Val->IsStringObject() || Val->IsRegExp() || Val->IsDate()) {
            return TJsonVal::NewStr(TStr(*v8::String::Utf8Value(Val->ToString())));
        }
        else if (Val->IsArray()) {
            PJsonVal JsonArr = TJsonVal::NewArr();

            v8::Array* Arr = v8::Array::Cast(*Val);
            for (uint i = 0; i < Arr->Length(); i++) {
                JsonArr->AddToArr(GetObjToNmJson(Arr->Get(i)));
            }

            return JsonArr;
        }
        else {  // general object with fields
            PJsonVal JsonVal = TJsonVal::NewObj();
            v8::Local<v8::Object> Obj = Val->ToObject();

            v8::Local<v8::Array> FldNmV = Obj->GetOwnPropertyNames();
            for (uint i = 0; i < FldNmV->Length(); i++) {
                const TStr FldNm(*v8::String::Utf8Value(FldNmV->Get(i)->ToString()));
                v8::Local<v8::Value> FldVal = Obj->Get(FldNmV->Get(i));

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
            return TJsonVal::NewBool(Val->BooleanValue());
        }
        else if (Val->IsNumber()) {
            const double& NumVal = Val->NumberValue();
            return TJsonVal::NewNum(NumVal);
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

template <class TClass>
bool TNodeJsUtil::IsClass(const v8::Handle<v8::Object> Obj) {
    return IsClass(Obj, TClass::GetClassId());
}

template <class TClass>
bool TNodeJsUtil::IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    return IsArgWrapObj(Args, ArgN, TClass::GetClassId());
}

template <class TClass>
TClass* TNodeJsUtil::GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    EAssertR(ArgN < Args.Length(), "GetArgUnwrapObj: Not enough arguments!");
    EAssertR(IsArgWrapObj<TClass>(Args, ArgN), "Invalid argument class!");
    return node::ObjectWrap::Unwrap<TClass>(Args[ArgN]->ToObject());
}

template <class TClass>
TClass* TNodeJsUtil::GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent(); 
    v8::HandleScope HandleScope(Isolate);
    // check we have the argument
    EAssertR(ArgN < Args.Length(), "GetArgUnwrapObj: Not enough arguments!");
    EAssertR(Args[ArgN]->IsObject(), "GetArgUnwrapObj: Argument not an object!");
    // unwrap and return
    return TNodeJsUtil::GetUnwrapFld<TClass>(Args[ArgN]->ToObject(), Property);
}

template <class TClass>
bool TNodeJsUtil::IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent(); 
    v8::HandleScope HandleScope(Isolate);
    if (!IsObjFld(Obj, FldNm)) { return false; }
    v8::Handle<v8::Value> ValFld = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
    if (!ValFld->IsObject()) { return false; }
    v8::Handle<v8::Object> ObjFld = ValFld->ToObject();
    return IsClass(ObjFld, TClass::GetClassId());
}

template <class TClass>
TClass* TNodeJsUtil::GetUnwrapFld(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // check we have the field
    EAssertR(IsObjFld(Obj, FldNm), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is missing!");
    v8::Handle<v8::Value> ValFld = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
    // check field points to an object
    EAssertR(ValFld->IsObject(), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not an object");
    v8::Handle<v8::Object> ObjFld = ValFld->ToObject();

    return Unwrap<TClass>(ObjFld);
}

template <class TType, class TSizeTy>
void TNodeJsUtil::GetCppTmMSecsV(const TVec<TType, TSizeTy>& NodeJsTmMSecsV,
        TVec<TUInt64, TSizeTy>& CppTmMSecs) {
    TSizeTy Len = NodeJsTmMSecsV.Len();

    if (CppTmMSecs.Len() != Len) { CppTmMSecs.Gen(Len); }

    for (TSizeTy i = 0; i < Len; i++) {
        CppTmMSecs[i] = GetCppTimestamp(int64(NodeJsTmMSecsV[i]));
    }
}

template <class TClass>
void TNodeJsUtil::_NewJs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    try {
        EAssertR(Args.IsConstructCall(), "Not a constructor call (you forgot to use the new operator)");
        v8::Local<v8::Object> Instance = Args.This();
        v8::Handle<v8::String> key = v8::String::NewFromUtf8(Isolate, "class");
        // static TStr TClass:ClassId must be defined
        v8::Handle<v8::String> value = v8::String::NewFromUtf8(Isolate, TClass::GetClassId().CStr());
        Instance->SetHiddenValue(key, value);
        // This is skipped in _NewCpp
        TClass* Obj = TClass::NewFromArgs(Args);
        Obj->Wrap(Instance);
        Args.GetReturnValue().Set(Instance);
    } catch (const PExcept& Except) {
        Isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(Isolate, (TStr("[addon] Exception in constructor call, ClassId: ") + TClass::GetClassId() + ":" + Except->GetMsgStr()).CStr())));
    }
}

template <class TClass>
void TNodeJsUtil::_NewCpp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    try {
        EAssertR(Args.IsConstructCall(), "Not a constructor call");
        v8::Local<v8::Object> Instance = Args.This();
        v8::Handle<v8::String> key = v8::String::NewFromUtf8(Isolate, "class");
        // static TStr TClass:ClassId must be defined
        v8::Handle<v8::String> value = v8::String::NewFromUtf8(Isolate, TClass::GetClassId().CStr());
        Instance->SetHiddenValue(key, value);
        // wrap is done elsewhere in cpp
        Args.GetReturnValue().Set(Instance);
    }
    catch (const PExcept& Except) {
        printf("%s\n", Except->GetMsgStr().CStr());
        throw Except;
//      Isolate->ThrowException(v8::Exception::TypeError(
//          v8::String::NewFromUtf8(Isolate, (TStr("[addon] Exception in constructor call, ClassId: ") + TClass::GetClassId() + ":" + Except->GetMsgStr()).CStr())));

    }
}

template <class TClass>
v8::Local<v8::Object> TNodeJsUtil::NewInstance(TClass* Obj) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    EAssertR(!TClass::Constructor.IsEmpty(), "NewJsInstance<...>::New: constructor is empty. Did you call NewJsInstance<...>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, TClass::Constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();
    Obj->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

template <class TVal>
void TNodeJsUtil::ExecuteVoid(const v8::Handle<v8::Function>& Fun, const v8::Local<TVal>& Arg) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch;

    v8::Handle<v8::Value> Argv[1] = { Arg };
    Fun->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);
}

template <class TVal>
bool TNodeJsUtil::ExecuteBool(const v8::Handle<v8::Function>& Fun, const v8::Local<TVal>& Arg) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch;

    v8::Handle<v8::Value> Argv[1] = { Arg };
    v8::Local<v8::Value> RetVal = Fun->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);

    EAssertR(RetVal->IsBoolean(), "The return value is not a boolean!");
    return RetVal->BooleanValue();
}

template <class TClass>
TClass* TNodeJsUtil::UnwrapCheckWatcher(v8::Handle<v8::Object> Arg) {
    EAssertR(IsClass(Arg, TClass::GetClassId()), "Object is not a wrapped `" + TClass::GetClassId() + "` class");
    TClass* Obj = node::ObjectWrap::Unwrap<TClass>(Arg);
    Obj->Watcher->AssertOpen();
    return Obj;
}

//////////////////////////////////////////////////////
// Node - Asynchronous Utilities
template <typename THandle>
void TNodeJsAsyncUtil::InternalDelHandle(uv_handle_t* Handle) {
    THandle* Async = (THandle*) Handle;
    delete Async;
}

#endif

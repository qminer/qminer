/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_NODEJS_UTILS_HPP
#define QMINER_NODEJS_UTILS_HPP


 ////////////////////////////////////////////////////////
 //// Node - Utilities

template <class TClass>
bool TNodeJsUtil::IsClass(const v8::Local<v8::Object> Obj) {
    return IsClass(Obj, TClass::GetClassId());
}

template <class TClass>
bool TNodeJsUtil::IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    return IsArgWrapObj(Args, ArgN, TClass::GetClassId());
}

template <class TClass>
TClass* TNodeJsUtil::GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    EAssertR(ArgN < Args.Length(), "GetArgUnwrapObj: Not enough arguments!");
    EAssertR(IsArgWrapObj<TClass>(Args, ArgN), "Invalid argument class `" + GetClass(v8::Local<v8::Object>::Cast(Args[ArgN])) + "`, when expecting `" + TClass::GetClassId() + "`!");
    return node::ObjectWrap::Unwrap<TClass>(Nan::To<v8::Object>(Args[ArgN]).ToLocalChecked());
}

template <class TClass>
TClass* TNodeJsUtil::GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // check we have the argument
    EAssertR(ArgN < Args.Length(), "GetArgUnwrapObj: Not enough arguments!");
    EAssertR(Args[ArgN]->IsObject(), "GetArgUnwrapObj: Argument not an object!");
    // unwrap and return
    return TNodeJsUtil::GetUnwrapFld<TClass>(Nan::To<v8::Object>(Args[ArgN]).ToLocalChecked(), Property);
}

template <class TClass>
bool TNodeJsUtil::IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    if (!IsObjFld(Obj, FldNm)) { return false; }
    v8::Local<v8::Value> ValFld = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
    if (!ValFld->IsObject()) { return false; }
    v8::Local<v8::Object> ObjFld = Nan::To<v8::Object>(ValFld).ToLocalChecked();
    return IsClass(ObjFld, TClass::GetClassId());
}

template <class TClass>
TClass* TNodeJsUtil::GetUnwrapFld(v8::Local<v8::Object> Obj, const TStr& FldNm) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // check we have the field
    EAssertR(IsObjFld(Obj, FldNm), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is missing!");
    v8::Local<v8::Value> ValFld = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));
    // check field points to an object
    EAssertR(ValFld->IsObject(), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not an object");
    v8::Local<v8::Object> ObjFld = Nan::To<v8::Object>(ValFld).ToLocalChecked();

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
        v8::Local<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
        // static TStr TClass:ClassId must be defined
        v8::Local<v8::String> Value = v8::String::NewFromUtf8(Isolate, TClass::GetClassId().CStr());
        TNodeJsUtil::SetPrivate(Instance, Key, Value);
        // This is skipped in _NewCpp
        TClass* Obj = TClass::NewFromArgs(Args);
        Obj->Wrap(Instance);
        Args.GetReturnValue().Set(Instance);
        // obj statistics
        ObjNameH.AddDat(TClass::GetClassId()).Val2++;
        ObjCount.Val2++;
    }
    catch (const PExcept& Except) {
        Isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(Isolate, (TStr("[addon] Exception in constructor call, ClassId: ") + TClass::GetClassId() + ":" + Except->GetStr()).CStr())));
    }
}

template <class TClass>
void TNodeJsUtil::_NewCpp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    try {
        EAssertR(Args.IsConstructCall(), "Not a constructor call");
        v8::Local<v8::Object> Instance = Args.This();
        v8::Local<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
        // static TStr TClass:ClassId must be defined
        v8::Local<v8::String> Value = v8::String::NewFromUtf8(Isolate, TClass::GetClassId().CStr());
        TNodeJsUtil::SetPrivate(Instance, Key, Value);
        // wrap is done elsewhere in cpp
        Args.GetReturnValue().Set(Instance);
        // obj statistics
        ObjNameH.AddDat(TClass::GetClassId()).Val1++;
        ObjCount.Val1++;
    }
    catch (const PExcept& Except) {
        printf("%s\n", Except->GetStr().CStr());
        throw Except;
        //      Isolate->ThrowException(v8::Exception::TypeError(
        //          v8::String::NewFromUtf8(Isolate, (TStr("[addon] Exception in constructor call, ClassId: ") + TClass::GetClassId() + ":" + Except->GetStr()).CStr())));

    }
}

template <class TClass>
v8::Local<v8::Object> TNodeJsUtil::NewInstance(TClass* Obj) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    EAssertR(!TClass::Constructor.IsEmpty(), "NewJsInstance<" + TClass::GetClassId() + ">::New: constructor is empty. Did you call NewJsInstance<...>::Init(exports); in this module's init function?");
    v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, TClass::Constructor);
    v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext());
    v8::Local<v8::Object> Instance;
    EAssertR(MaybeInstance.ToLocal(&Instance), "NewJsInstance<" + TClass::GetClassId() + ">::New: failed to create instance (empty handle)");
    Obj->Wrap(Instance);
    return HandleScope.Escape(Instance);
}

template <class TVal>
void TNodeJsUtil::ExecuteVoid(const v8::Local<v8::Function>& Fun, const v8::Local<TVal>& Arg) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    v8::Local<v8::Value> Argv[1] = { Arg };
    Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);
}

template <class TVal>
bool TNodeJsUtil::ExecuteBool(const v8::Local<v8::Function>& Fun, const v8::Local<TVal>& Arg) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    v8::Local<v8::Value> Argv[1] = { Arg };
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Fun, Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);

    if (Tmp.IsEmpty()) {
        Isolate->ThrowException(TryCatch.Exception());
    } else {
        v8::Local<v8::Value> RetVal = Tmp.ToLocalChecked();
        EAssertR(RetVal->IsBoolean(), "The return value is not a boolean!");
        return RetVal->BooleanValue(Nan::GetCurrentContext()).FromJust();
    }
}

template <class TClass>
TClass* TNodeJsUtil::UnwrapCheckWatcher(v8::Local<v8::Object> Arg) {
    EAssertR(IsClass(Arg, TClass::GetClassId()), "Object is not a wrapped `" + TClass::GetClassId() + "` class");
    TClass* Obj = node::ObjectWrap::Unwrap<TClass>(Arg);
    Obj->Watcher->AssertOpen();
    return Obj;
}

//////////////////////////////////////////////////////
// Node - Asynchronous Utilities
template <typename THandle>
void TNodeJsAsyncUtil::InternalDelHandle(uv_handle_t* Handle) {
    THandle* Async = (THandle*)Handle;
    delete Async;
}


#endif


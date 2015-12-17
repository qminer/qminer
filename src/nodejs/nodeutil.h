/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_NODEJS_UTILS
#define QMINER_NODEJS_UTILS

#ifndef BUILDING_NODE_EXTENSION
    #define BUILDING_NODE_EXTENSION
#endif

#include <node.h>
#include <node_object_wrap.h>
#if NODE_MODULE_VERSION == 47 /* Node.js v5.0.0 */
    #include <node_buffer.h>
#endif
#include <uv.h>
#include "base.h"

#define JsDeclareProperty(Function) \
    static void Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info); \
    static void _ ## Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            Function(Name, Info); \
        } catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError( \
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    };

#define JsDeclIndexedProperty(Function) \
    static void Function(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info); \
    static void _ ## Function(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            Function(Index, Info); \
        } catch(const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError(\
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    }

#define JsDeclareSetIndexedProperty(FunctionGetter, FunctionSetter) \
    static void FunctionGetter(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info); \
    static void _ ## FunctionGetter(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            FunctionGetter(Index, Info); \
        } catch(const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError(\
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    } \
    static void FunctionSetter(uint32_t Index, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<v8::Value>& Info); \
    static void _ ## FunctionSetter(uint32_t Index, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            FunctionSetter(Index, Value, Info); \
        } catch(const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError(\
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    }

#define JsDeclareSetProperty(GetFunction, SetFunction) \
    static void GetFunction(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info); \
    static void _ ## GetFunction(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            GetFunction(Name, Info); \
        } catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError( \
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
      } \
    } \
    static void SetFunction(v8::Local<v8::String> Name, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<void>& Info); \
    static void _ ## SetFunction(v8::Local<v8::String> Name, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<void>& Info) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            SetFunction(Name, Value, Info); \
        } catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError( \
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    };

#define JsDeclareInternalFunction(Function)	\
	static void _ ## Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            Function(Args); \
        } catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError(\
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    };

#define JsDeclareFunction(Function) \
    static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args); \
    JsDeclareInternalFunction(Function);

#define JsDeclareSpecializedFunction(Function) \
    static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { throw TExcept::New("Not implemented!"); } \
    static void _ ## Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
        v8::HandleScope HandleScope(Isolate); \
        try { \
            Function(Args); \
        } catch (const PExcept& Except) { \
            Isolate->ThrowException(v8::Exception::TypeError(\
            v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr()))); \
        } \
    };

#define JsDeclareAsyncFunction(Function, TTask)	\
	static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args) {	\
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();	\
		v8::HandleScope HandleScope(Isolate);	\
		TTask* Task = new TTask(Args);	\
		Task->ExtractCallback(Args);	\
		TNodeJsAsyncUtil::ExecuteOnWorker(Task);	\
		Args.GetReturnValue().Set(v8::Undefined(Isolate));	\
	};	\
	JsDeclareInternalFunction(Function);

#define JsDeclareSyncFunction(Function, TTask)	\
	static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args) {	\
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();	\
		v8::HandleScope HandleScope(Isolate);	\
		TTask Task(Args);	\
		Task.Run();	\
		Task.AfterRunSync(Args);	\
	};	\
	JsDeclareInternalFunction(Function);

#define JsDeclareSyncAsync(SyncFun, AsyncFun, Task)	\
	JsDeclareSyncFunction(SyncFun, Task)	\
	JsDeclareAsyncFunction(AsyncFun, Task);




//////////////////////////////////////////////////////
// Node - Utilities
class TNodeJsUtil {
public:
    /// Convert v8 Json to GLib Json (PJsonVal). Is parameter IgnoreFunc is set to true the method will
	/// ignore functions otherwise an exception will be thrown when a function is encountered
    static PJsonVal GetObjJson(const v8::Local<v8::Value>& Obj, const bool IgnoreFunc=false);
    static PJsonVal GetObjProps(const v8::Local<v8::Object>& Obj) { return GetObjJson(Obj, true); }
    /// Convert GLib Json (PJsonVal) to v8 Json
    static v8::Local<v8::Value> ParseJson(v8::Isolate* Isolate, const PJsonVal& JsonVal);

    /// Transform V8 string to TStr
    static TStr GetStr(const v8::Local<v8::String>& V8Str);

    /// Gets the class name of the underlying glib object. the name is stored
    /// in an hidden variable "class"
    static TStr GetClass(const v8::Handle<v8::Object> Obj);

    /// Check if argument ArgN exists
    static bool IsArg(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) { return (Args.Length() > ArgN); }

    /// Checks if the class name of the underlying glib object matches the
    /// given string. the name is stored in an hidden variable "class"
    static bool IsClass(const v8::Handle<v8::Object> Obj, const TStr& ClassNm);
    /// Check if argument ArgN belongs to a given class
    static bool IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& ClassNm);
    /// Check if argument ArgN belongs to a given class
    template <class TClass>
    static bool IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if argument ArgN is null
    static bool IsArgNull(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// returns true if the argument is undefined or not in the arguments at all
	static bool IsArgUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
	/// returns true if the argument is null, undefined or not in the arguments at all
	static bool IsArgNullOrUndef(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::Function
    static bool IsArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::Object
    static bool IsArgObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::Bool
    static bool IsArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::Number
    static bool IsArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::String
    static bool IsArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN is a JSON object
    static bool IsArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);

    /// Extracts argument ArgN as a function
    static v8::Handle<v8::Function> GetArgFun(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN property as bool
    static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN property as bool, and use DefVal in case when not present
    static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& DefVal);
    /// Extract argument ArgN property as bool
    static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const bool& DefVal);
    /// Extract argument ArgN as int
    static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN as int, and use DefVal in case when not present
    static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const int& DefVal);
	/// Extract argument ArgN property as int
	static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property);
	/// Extract argument ArgN property as int
    static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const int& DefVal);

    /// Extract argument ArgN as double
    static double GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN as int, and use DefVal in case when not present
    static double GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const double& DefVal);
    /// Extract argument ArgN property as int
    static double GetArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const double& DefVal);

    /// Extract argument ArgN as TStr
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN as TStr, and use DefVal in case when not present
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& DefVal);
    /// Extract argument ArgN property as string
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const TStr& DefVal);
    /// Extract argument ArgN as GLib Json (PJsonVal)
    static PJsonVal GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// casts the argument into the appropriate type
    template <class TClass>
    static TClass* GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);

    /// returns true if the given object contains a field with the given name
    static bool IsObjFld(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true is the field is not defined or is null
    static bool IsFldNull(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true if the object contains a field with the specified name and
    /// that field has the provided ClassId
    static bool IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm, const TStr& ClassId);
    /// returns true if the field is a function
    static bool IsFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true if the field is an integer
    static bool IsFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true if the field is a float
    static bool IsFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm);

    /// extracts the field from the object 'Obj'
    template <class TClass>
    static TClass* GetUnwrapFld(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// extracts the field as a JSON object
    static PJsonVal GetFldJson(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// extracts the field as a handle, performs all the necessary checks
    static v8::Local<v8::Object> GetFldObj(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// extracts the field as a handle of a function
    static v8::Local<v8::Function> GetFldFun(v8::Local<v8::Object> Obj, const TStr& FldNm);

    static int GetFldInt(v8::Local<v8::Object> Obj, const TStr& FldNm);
    static double GetFldFlt(v8::Local<v8::Object> Obj, const TStr& FldNm);

    /// Executes the function with the specified argument and returns a double result.
    static double ExecuteFlt(const v8::Handle<v8::Function>& Fun, const v8::Local<v8::Object>& Arg);
    /// Executes the function with the specified argument and returns an object as a JSON object.
    static PJsonVal ExecuteJson(const v8::Handle<v8::Function>& Fun,
    		const v8::Local<v8::Object>& Arg1, const v8::Local<v8::Object>& Arg2);
    /// Executes the function with the specified argument
    template <class TVal>
	static void ExecuteVoid(const v8::Handle<v8::Function>& Fun, const v8::Local<TVal>& Arg);

    static void ExecuteVoid(const v8::Handle<v8::Function>& Fun, const int& ArgC,
    		v8::Handle<v8::Value> ArgV[]);
    static void ExecuteVoid(const v8::Handle<v8::Function>& Fun, const v8::Local<v8::Object>& Arg1,
    		const v8::Local<v8::Object>& Arg2);

    static void ExecuteVoid(const v8::Handle<v8::Function>& Fun);
    static void ExecuteErr(const v8::Handle<v8::Function>& Fun, const PExcept& Except);

    template <class TVal>
	static bool ExecuteBool(const v8::Handle<v8::Function>& Fun, const v8::Local<TVal>& Arg);

    /// converts a v8 value to a Win timestamp
    static uint64 GetTmMSecs(v8::Handle<v8::Value>& Value);
    static uint64 GetArgTmMSecs(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    static int64 GetJsTimestamp(const uint64& MSecs) { return TTm::GetUnixMSecsFromWinMSecs(MSecs); }
	static uint64 GetCppTimestamp(const int64& MSecs) { return TTm::GetWinMSecsFromUnixMSecs(MSecs); }

	/// converts a vector of UNIX timestamps used by Node.js to internal C++ timestamps
	template <class TType, class TSizeTy>
	static void GetCppTmMSecsV(const TVec<TType, TSizeTy>& NodeJsTmMSecsV,
			TVec<TUInt64, TSizeTy>& CppTmMSecs);

	/// Throws and exception which can be caught in Javascript
	static void ThrowJsException(v8::Isolate* Isolate, const PExcept& Except);

	/// Constructor callback: sets the hidden "class" property of new instance,
	/// creates a new wrapper object and wraps the new instance. This callback
	/// should be used when creating object from javascript using "new"
	template <class TClass>
	static void _NewJs(const v8::FunctionCallbackInfo<v8::Value>& Args);
	/// Constructor callback: Only sets the hidden "class" property of new instance
	/// to ClassId (wrapper pointer construction should be done elsewhere, wrapping should be done by NewJsInstance(TClass* Obj))
	/// This callback should be used when creating objects from C++ functions, by using TNodeJsUtil::NewJsInstance<Obj>
	template <class TClass>
	static void _NewCpp(const v8::FunctionCallbackInfo<v8::Value>& Args);

	/// Creates a new instance using TClass::Constructor and wraps it with Obj.
	/// The Constructor should be linked with a function template that uses TNodeJsUtil::_NewCpp<Obj> as callback
	template <class TClass>
	static v8::Local<v8::Object> NewInstance(TClass* Obj);
	
	static v8::Local<v8::Value> V8JsonToV8Str(const v8::Handle<v8::Value>& Json);
	static TStr JSONStringify(const v8::Handle<v8::Value>& Json) { return GetStr(V8JsonToV8Str(Json)->ToString()); }

    /// TStrV -> v8 string array
    static v8::Local<v8::Value> GetStrArr(const TStrV& StrV);	

	/// Convert v8 external array (binary data) to PMem
	static PMem GetArgMem(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);

	/// Used for unwrapping objects that depend on TBase being valid
	template <class TClass>
	static TClass* UnwrapCheckWatcher(v8::Handle<v8::Object> Arg);

	template <class TClass>
	static TClass* Unwrap(v8::Handle<v8::Object> Arg) { return node::ObjectWrap::Unwrap<TClass>(Arg); }

private:
	/// returns the internal C++ windows timestamp from a double representation
	/// of a UNIX timestamp
	static uint64 GetTmMSecs(const double& UnixMSecs);
	/// returns the internal C++ windows timestamp from a v8 date
	static uint64 GetTmMSecs(v8::Handle<v8::Date>& Date);
};

template <class TClass>
bool TNodeJsUtil::IsArgWrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
    return IsArgWrapObj(Args, ArgN, TClass::GetClassId());
}

template <class TClass>
TClass* TNodeJsUtil::GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	EAssertR(ArgN < Args.Length(), "GetArgObj: Not enough arguments!");
	EAssertR(IsArgWrapObj<TClass>(Args, ArgN), "Invalid argument class!");
	return node::ObjectWrap::Unwrap<TClass>(Args[ArgN]->ToObject());
}

template <class TClass>
TClass* TNodeJsUtil::GetUnwrapFld(v8::Local<v8::Object> Obj, const TStr& FldNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(IsObjFld(Obj, FldNm), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is missing!");
	v8::Handle<v8::Value> ValFld = Obj->Get(v8::String::NewFromUtf8(Isolate, FldNm.CStr()));

	EAssertR(ValFld->IsObject(), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not an object");
	v8::Handle<v8::Object> ObjFld = ValFld->ToObject();

	EAssertR(IsClass(ObjFld, TClass::GetClassId()), "TNodeJsUtil::GetUnwrapFld: Key " + FldNm + " is not of type TClass");
	return node::ObjectWrap::Unwrap<TClass>(ObjFld);
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
//		Isolate->ThrowException(v8::Exception::TypeError(
//			v8::String::NewFromUtf8(Isolate, (TStr("[addon] Exception in constructor call, ClassId: ") + TClass::GetClassId() + ":" + Except->GetMsgStr()).CStr())));

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
	if (TryCatch.HasCaught()) {
		TryCatch.ReThrow();
		return;
	}
}

template <class TVal>
bool TNodeJsUtil::ExecuteBool(const v8::Handle<v8::Function>& Fun, const v8::Local<TVal>& Arg) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::TryCatch TryCatch;

	v8::Handle<v8::Value> Argv[1] = { Arg };
	v8::Local<v8::Value> RetVal = Fun->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);

	if (TryCatch.HasCaught()) {
		TryCatch.ReThrow();
		throw TExcept::New("Exception while executing bool!", "TNodeJsUtil::ExecuteBool");
	}

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
// Async Stuff
class TAsyncTask {
public:
	virtual ~TAsyncTask() {}

	virtual void Run() = 0;
	virtual void AfterRun() = 0;
};

class TNodeTask: public TAsyncTask {
private:
	v8::Persistent<v8::Function> Callback;
	v8::Persistent<v8::Array> ArgPersist;
	PExcept Except;

public:
	TNodeTask(const v8::FunctionCallbackInfo<v8::Value>& Args);
	virtual ~TNodeTask();

	virtual v8::Handle<v8::Function> GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) = 0;
	virtual v8::Local<v8::Value> WrapResult();

	void ExtractCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);

	void AfterRun();
	void AfterRunSync(const v8::FunctionCallbackInfo<v8::Value>& Args);

protected:
	void SetExcept(const PExcept& _Except) { Except = _Except; }
	bool HasExcept() const { return !Except.Empty(); }
};


//////////////////////////////////////////////////////
// Node - Asynchronous Utilities
class TNodeJsAsyncUtil {
private:
	template <class TTask>
	struct TMainData {
		TTask* Task;
		bool DelTask;

		TMainData(TTask* Task, const bool& DelTask);
		~TMainData() { if (DelTask) { delete Task; } }
	};

	template <class TTask>
	struct TMainSemaphoreData {
		TTask* Task;
		bool DelTask;
		uv_sem_t Semaphore;

		TMainSemaphoreData(TTask* _Task, const bool& _DelTask):
			Task(_Task),
			DelTask(_DelTask),
			Semaphore() {}
		~TMainSemaphoreData() { if (DelTask) { delete Task; } }
	};

	template <class TTask>
	struct TWorkerData {
		TTask* Task;
		TWorkerData(TTask* Task);
		~TWorkerData() { delete Task; }
	};

	template <typename THandle> static void DelHandle(uv_handle_t* Handle);

	template <class TTask> static void OnMain(uv_async_t* UvAsync);
	template <class TTask> static void OnMainBlock(uv_async_t* UvAsync);

	template <class TTask> static void OnWorker(uv_work_t* UvReq);
	template <class TTask> static void AfterOnWorker(uv_work_t* UvReq, int Status);

public:
	template <class TTask> static void ExecuteOnMain(TTask* Task, const bool& DelData=true);
	template <class TTask> static void ExecuteOnMainAndWait(TTask* Task, const bool& DelData=true);
	template <class TTask> static void ExecuteOnWorker(TTask* Task);
};

template <typename THandle>
void TNodeJsAsyncUtil::DelHandle(uv_handle_t* Handle) {
	THandle* Async = (THandle*) Handle;
	delete Async;
}

template <class TTask>
TNodeJsAsyncUtil::TMainData<TTask>::TMainData(TTask* _Task, const bool& _DelTask):
		Task(_Task),
		DelTask(_DelTask) {}

template <class TTask>
TNodeJsAsyncUtil::TWorkerData<TTask>::TWorkerData(TTask* _Task):
		Task(_Task) {}

template <class TTask>
void TNodeJsAsyncUtil::OnMain(uv_async_t* UvAsync) {
	TMainData<TTask>* Task = static_cast<TMainData<TTask>*>(UvAsync->data);

	try {
		TTask::Run(*Task->Task);
	} catch (const PExcept& Except) {
		printf("Exception on main thread: %s!", Except->GetMsgStr().CStr());
	}

	// clean up
	uv_close((uv_handle_t*) UvAsync, DelHandle<uv_async_t>);
	delete Task;
}

template <class TTask>
void TNodeJsAsyncUtil::OnMainBlock(uv_async_t* UvAsync) {
	TMainSemaphoreData<TTask>* Task = static_cast<TMainSemaphoreData<TTask>*>(UvAsync->data);

	try {
		TTask::Run(*Task->Task);
	} catch (const PExcept& Except) {
		printf("Exception on main thread: %s!", Except->GetMsgStr().CStr());
	}

	// clean up
	uv_close((uv_handle_t*) UvAsync, DelHandle<uv_async_t>);

	uv_sem_post(&Task->Semaphore);
	uv_sem_destroy(&Task->Semaphore);

	delete Task;
}

template <class TTask>
void TNodeJsAsyncUtil::OnWorker(uv_work_t* UvReq) {
	TWorkerData<TTask>* Task = static_cast<TWorkerData<TTask>*>(UvReq->data);

	try {
		Task->Task->Run();
//		TTask::Run(*Task->Task);
	} catch (const PExcept& Except) {
		printf("Exception on worker thread: %s!", Except->GetMsgStr().CStr());
	}
}

template <class TTask>
void TNodeJsAsyncUtil::AfterOnWorker(uv_work_t* UvReq, int Status) {
	TWorkerData<TTask>* Task = static_cast<TWorkerData<TTask>*>(UvReq->data);

	try {
		Task->Task->AfterRun();
//		TTask::AfterRun(*Task->Task);
	} catch (const PExcept& Except) {
		printf("Exception on worker thread: %s!", Except->GetMsgStr().CStr());
	}

	delete Task;
	delete UvReq;
}


template <class TTask>
void TNodeJsAsyncUtil::ExecuteOnMain(TTask* Task, const bool& DelTask) {
	uv_async_t* UvAsync = new uv_async_t;
	UvAsync->data = new TMainData<TTask>(Task, DelTask);

	uv_async_init(uv_default_loop(), UvAsync, OnMain<TTask>);
	uv_async_send(UvAsync);
}

template <class TTask>
void TNodeJsAsyncUtil::ExecuteOnMainAndWait(TTask* Task, const bool& DelTask) {
	uv_async_t* UvAsync = new uv_async_t;
	TMainSemaphoreData<TTask>* TaskWrapper = new TMainSemaphoreData<TTask>(Task, DelTask);

	UvAsync->data = TaskWrapper;

	int Err = uv_sem_init(&TaskWrapper->Semaphore, 0);

	if (Err != 0) {
		delete UvAsync;
		delete TaskWrapper;
		throw TExcept::New("Failed to create a semaphore, code: " + TInt::GetStr(Err) + "!");
	} else {
		uv_async_init(uv_default_loop(), UvAsync, OnMainBlock<TTask>);
		uv_async_send(UvAsync);
		uv_sem_wait(&TaskWrapper->Semaphore);
	}
}

template <class TTask>
void TNodeJsAsyncUtil::ExecuteOnWorker(TTask* Task) {
	uv_work_t* UvReq = new uv_work_t;
	UvReq->data = new TWorkerData<TTask>(Task);

	uv_queue_work(uv_default_loop(), UvReq, OnWorker<TTask>, AfterOnWorker<TTask>);
}

#endif


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
#include <node_buffer.h>
#include <uv.h>
#include "base.h"
#include "thread.h"

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

#define JsDeclareInternalFunction(Function) \
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

#define JsDeclareAsyncFunction(Function, TTask) \
    static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();   \
        v8::HandleScope HandleScope(Isolate);   \
        TTask* Task = new TTask(Args);  \
        Task->ExtractCallback(Args);    \
        TNodeJsAsyncUtil::ExecuteOnWorker(Task);    \
        Args.GetReturnValue().Set(v8::Undefined(Isolate));  \
    };  \
    JsDeclareInternalFunction(Function);

#define JsDeclareSyncFunction(Function, TTask)  \
    static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
        v8::Isolate* Isolate = v8::Isolate::GetCurrent();   \
        v8::HandleScope HandleScope(Isolate);   \
        TTask Task(Args);   \
        Task.Run(); \
        Task.AfterRunSync(Args);    \
    };  \
    JsDeclareInternalFunction(Function);

#define JsDeclareSyncAsync(SyncFun, AsyncFun, Task) \
    JsDeclareSyncFunction(SyncFun, Task)    \
    JsDeclareAsyncFunction(AsyncFun, Task);


// forward declarations
class TAuxFltV;

template <class TValType, class TAuxType> class TNodeJsVec;
typedef TNodeJsVec<TFlt, TAuxFltV> TNodeJsFltV;

//////////////////////////////////////////////////////
// Node - Utilities
class TNodeJsUtil {
public:
    /// Number of objects (create by factories, created using new, deleted) per class ID
    static THash<TStr, TTriple<TInt, TInt, TInt> > ObjNameH;
    /// Total number of objects (create by factories, created using new, deleted)
    static TTriple<TInt, TInt, TInt> ObjCount;

    /// Map from ClassId to JS object name accessor (example: "TRec" -> "$id", "Store" -> "name", "StreamAggr" -> "name") 
    static THash<TStr, TStr> ClassNmAccessorH;
    /// Registers the name of JS object name accessor for the given class
    static void RegisterClassNmAccessor(const TStr& ClassId, const TStr& NmProp) { ClassNmAccessorH.AddDat(ClassId, NmProp); }
    /// Returns the name of JS object name accessor for the given class
    static TStr GetClassNmAccessor(const TStr& ClassId) { return ClassNmAccessorH.GetDatOrDef(ClassId, ""); }

    /// Checks if TryCatch caught an error, extracts the message and throws a PExcept
    static void CheckJSExcept(const v8::TryCatch& TryCatch);
    /// Convert v8 Json to GLib Json (PJsonVal). Is parameter IgnoreFunc is set to true the method will
    /// ignore functions otherwise an exception will be thrown when a function is encountered
    static PJsonVal GetObjJson(const v8::Local<v8::Value>& Val, const bool& IgnoreFunc=false, const bool& IgnoreWrappedObj=false);
    static PJsonVal GetObjProps(const v8::Local<v8::Object>& Obj) { return GetObjJson(Obj, true); }
    static PJsonVal GetObjToNmJson(const v8::Local<v8::Value>& Val);

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
    /// Checks if the class name of the underlying glib object matches the
    /// given string. the name is stored in an hidden variable "class"
    template <class TClass> static bool IsClass(const v8::Handle<v8::Object> Obj);
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
    /// Check if is argument ArgN is a 32 bit integer
    static bool IsArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::Number
    static bool IsArgFlt(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN of type v8::String
    static bool IsArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check if is argument ArgN is a JSON object
    static bool IsArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Check whether Args[ArgN] is a buffer
    static bool IsArgBuffer(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    // Check whether Object is a buffer
    static bool IsBuffer(const v8::Local<v8::Object>& Object);

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
    /// Extract argument ArgN as a double vector, the argument can be of type Array or TNodeJsFltVV
    static void GetArgFltV(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, TFltV& FltV);
    /// Extract argument ArgN as TStr
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extract argument ArgN as TStr, and use DefVal in case when not present
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& DefVal);
    /// Extract argument ArgN property as string
    static TStr GetArgStr(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const TStr& DefVal);
    /// Extract argument ArgN as GLib Json (PJsonVal)
    static PJsonVal GetArgJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const bool& IgnoreFunc=false, const bool& IgnoreWrappedObj=false);
    static PJsonVal GetArgToNmJson(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// Extracts TVec<TIntV>
    static void GetArgIntVV(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, TVec<TIntV>& IntVV);
    /// casts the argument into the appropriate type
    template <class TClass>
    static TClass* GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);
    /// casts the argument into the appropriate type
    template <class TClass>
    static TClass* GetArgUnwrapObj(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property);

    /// returns true if the given object contains a field with the given name
    static bool IsObjFld(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true is the field is not defined or is null
    static bool IsFldNull(v8::Local<v8::Object> Obj, const TStr& FldNm);
    /// returns true if the object contains a field with class of given class id 
    static bool IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm, const TStr& ClassId);
    /// returns true if the object contains a field with of given class
    template <class TClass>
    static bool IsFldClass(v8::Local<v8::Object> Obj, const TStr& FldNm);
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
    static void ExecuteVoid(const v8::Handle<v8::Function>& Fun, const v8::Local<v8::Value>& Arg1,
            const v8::Local<v8::Value>& Arg2);

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

    static v8::Local<v8::Object> NewBuffer(const char* ChA, const size_t& Len);

    /// Convert v8 external array (binary data) to PMem
    static PMem GetArgMem(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN);

    /// Used for unwrapping objects that depend on TBase being valid
    template <class TClass>
    static TClass* UnwrapCheckWatcher(v8::Handle<v8::Object> Arg);

    template <class TClass>
    static TClass* Unwrap(v8::Handle<v8::Object> Arg) {
        EAssert(IsClass<TClass>(Arg));
        return node::ObjectWrap::Unwrap<TClass>(Arg);
    }

private:
    /// returns the internal C++ windows timestamp from a double representation of a UNIX timestamp
    static uint64 GetTmMSecs(const double& UnixMSecs);
    /// returns the internal C++ windows timestamp from a v8 date
    static uint64 GetTmMSecs(v8::Handle<v8::Date>& Date);
};

//////////////////////////////////////////////////////
// Async Stuff
class TAsyncTask {
public:
    virtual ~TAsyncTask() {}

    virtual void Run() = 0;
    virtual void AfterRun() = 0;
};

class TMainThreadTask {
public:
    virtual ~TMainThreadTask() {}
    virtual void Run() = 0;
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

    void AfterRun();
    void AfterRunSync(const v8::FunctionCallbackInfo<v8::Value>& Args);

    void ExtractCallback(const v8::FunctionCallbackInfo<v8::Value>& Args);

    void SetExcept(const PExcept& _Except) { Except = _Except; }

protected:
    bool HasExcept() const { return !Except.Empty(); }
};

typedef uv_async_t TMainThreadHandle;
typedef uv_work_t TWorkerThreadHandle;

//////////////////////////////////////////////////////
// Node - Asynchronous Utilities
class TNodeJsAsyncUtil {
private:

    struct TMainTaskWrapper {
        TMainThreadTask* Task;
        bool DelTask;

        TMainTaskWrapper(TMainThreadTask* Task, const bool& DelTask);
        virtual ~TMainTaskWrapper() { if (DelTask) { delete Task; } }
    };


    struct TMainBlockTaskWrapper: public TMainTaskWrapper {
        uv_sem_t Semaphore;

        TMainBlockTaskWrapper(TMainThreadTask* Task, const bool& DelTask);
        ~TMainBlockTaskWrapper() {}
    };

    struct TWorkerData {
        TAsyncTask* Task;
        TWorkerData(TAsyncTask* _Task):
                Task(_Task) {}
        ~TWorkerData() { delete Task; }
    };

    enum TAsyncHandleType {
        ahtBlocking,
        ahtAsync
    };

    struct TAsyncHandleConfig {
        TAsyncHandleType HandleType;
        TMainTaskWrapper* TaskWrapper;

        TAsyncHandleConfig(const TAsyncHandleType& _HandleType):
            HandleType(_HandleType),
            TaskWrapper(nullptr) {}
    };

    static TCriticalSection UvSection;

    static TAsyncHandleType GetHandleType(const TMainThreadHandle* UvAsync);

    static void SetAsyncData(TMainThreadHandle* UvAsync, TMainTaskWrapper* Data);
    static TMainTaskWrapper* ExtractAndClearData(TMainThreadHandle* UvAsync);

    template <typename THandle> static void InternalDelHandle(uv_handle_t* Handle);

    static void OnMain(TMainThreadHandle* UvAsync);
    static void OnMainBlock(TMainThreadHandle* UvAsync);

    static void OnWorker(TWorkerThreadHandle* UvReq);
    static void AfterOnWorker(TWorkerThreadHandle* UvReq, int Status);

public:
    /// Creates a new UV async handle which is used to execute code from
    /// a worker thread on the main thread. The worker thread will wait
    /// for the task on the main thread to finish before continuing execution.
    /// This mehtod should only be called from the main thread!
    static TMainThreadHandle* NewBlockingHandle();
    /// Creates a new UV async handle which is used to execute code from a
    /// worker thread on the main thread.
    /// This mehtod should only be called from the main thread!
    static TMainThreadHandle* NewHandle();
    /// Closes a UV async handle and releases the memory.
    /// This mehtod should only be called from the main thread!
    static void DelHandle(TMainThreadHandle* UvAsync);

    /// executes the task on the main thread
    /// Note: when using a non-blocking handle, not every call to this
    /// method will yield an execution. For example, when ExecuteOnMain is
    /// called 5 times before a task is executed, the task will be executed only once
    /// all the other tasks will be deleted (freed)
    static void ExecuteOnMain(TMainThreadTask* Task, TMainThreadHandle* UvAsync,
            const bool& DelData);
    /// executes the task on a worker thread
    static void ExecuteOnWorker(TAsyncTask* Task);
};

// include some implementations at the end, so we don't get incomplete types
#include "nodeutil.hpp"

#endif


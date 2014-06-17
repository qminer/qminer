/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef QMINER_JS_H
#define QMINER_JS_H

#include <qminer.h>
#include <qminer_srv.h>
#include <qminer_gs.h>
#include <v8.h>
#include <typeinfo>

namespace TQm {

// All comments starting with / / # (no spaces) are copied to JavaScript API documentation
// available on QMiner wiki page https://github.com/qminer/qminer/wiki/JavaScript
// every so often.
    
//# QMiner functionality is accessible through a JavaScript API. The JavaScript environment
//# is similar to Node.js, but omits any functionality not necessary for QMiner's core tasks.
//# The API includes a simple HTTP server with RESTful web-service support for defining
//# application specific web-service APIs.
//# 
//# The JavaScript API is implemented using [V8 JavaScript Engine](https://code.google.com/p/v8/),
//# which lives in the same process as core QMiner objects. This allows for fast manipulation
//# of QMiner objects, since data is moved from C++ to JavaScript and back only when needed.
//# Currently, version 3.18 of V8 is used.
//# 
//# JavaScript API requires [initialized work environment](Quick-Start).
//#     
//# ## Libraries
//# 
//# Scripts can load external libraries or modules in the same way as Node.js.
//# 
//# Library is loaded using `require` function:
//# ```JavaScript
//# var test = require('testModule.js');
//# ```
//# 
//# Modules export functionality by defining properties or functions on the `exports` object, 
//# for example (taken from Node.js documentation):
//# ```JavaScript
//# var PI = Math.PI;
//# exports.area = function (r) {
//#   return PI * r * r;
//# };
//# exports.circumference = function (r) {
//#   return 2 * PI * r;
//# };
//# ```
//# 
//# Libraries are loaded from two places. The first is project's library folder (`src/lib/`) 
//# and the second is QMiner library folder (`QMINER_HOME/lib/`). Project's library folder 
//# has priority in case the library with the same name exists in both places. Some libraries 
//# are implemented in C++, for example `analytics` and `time`.
//# 
//# The QMiner system comes with the following libraries:
//# - `analytics.js` -- main API for analytics techniques
//# - `utilities.js` -- useful JavaScript utilities, e.g., checking variable type
//# - `time` -- wrapper around user-friendly date-time object
//# - `assert.js` -- support for writing unit tests
    
///////////////////////////////
// JavaScript-Exceptions-related macros 

// Function declaration and registration 
#define JsRegisterFunction(Template, Function) \
	Template->Set(#Function, v8::FunctionTemplate::New(_ ## Function));
#define JsLongRegisterFunction(Template, JsFunction, CppFunction) \
	Template->Set(JsFunction, v8::FunctionTemplate::New(_ ## CppFunction));
#define JsDeclareFunction(Function) static v8::Handle<v8::Value> Function(const v8::Arguments& Args); \
	static v8::Handle<v8::Value> _ ## Function(const v8::Arguments& Args) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(Function(Args)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	}

#define JsDeclareTemplatedFunction(Function) \
	static v8::Handle<v8::Value> _ ## Function(const v8::Arguments& Args) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(Function(Args)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	} \
	static v8::Handle<v8::Value> Function(const v8::Arguments& Args) { throw TQmExcept::New("Not implemented!"); }
	
// Property declaration and registration 
#define JsRegisterProperty(Template, Function) \
	Template->SetAccessor(v8::String::New(#Function), _ ## Function);
#define JsLongRegisterProperty(Template, JsFunction, CppFunction) \
	Template->SetAccessor(v8::String::New(JsFunction), _ ## CppFunction);
#define JsDeclareProperty(Function) \
	static v8::Handle<v8::Value> Function(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info); \
	static v8::Handle<v8::Value> _ ## Function(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(Function(Properties, Info)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	}

// Property declaration and registration 
#define JsRegisterSetProperty(Template, JsFunction, GetCppFunction, SetCppFunction) \
	Template->SetAccessor(v8::String::New(JsFunction), _ ## GetCppFunction, _ ## SetCppFunction);
#define JsDeclareSetProperty(GetFunction, SetFunction) \
	static v8::Handle<v8::Value> GetFunction(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info); \
	static v8::Handle<v8::Value> _ ## GetFunction(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(GetFunction(Properties, Info)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	} \
	static void SetFunction(v8::Local<v8::String> Properties, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info); \
	static void _ ## SetFunction(v8::Local<v8::String> Properties, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			SetFunction(Properties, Value, Info); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
			} else { \
				throw Except; \
			} \
		} \
	}
	
// Indexed property declaration and registration 
#define JsRegIndexedProperty(Template, Function) \
	Template->SetIndexedPropertyHandler(_ ## Function);
#define JsDeclIndexedProperty(Function) \
	static v8::Handle<v8::Value> Function(uint32_t Index, const v8::AccessorInfo& Info); \
	static v8::Handle<v8::Value> _ ## Function(uint32_t Index, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(Function(Index, Info)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	}

// Indexed property getter/setter declaration and registration 
#define JsRegGetSetIndexedProperty(Template, FunctionGetter, FunctionSetter) \
	Template->SetIndexedPropertyHandler(_ ## FunctionGetter , _## FunctionSetter);
#define JsDeclGetSetIndexedProperty(FunctionGetter, FunctionSetter) \
	static v8::Handle<v8::Value> FunctionGetter(uint32_t Index, const v8::AccessorInfo& Info); \
	static v8::Handle<v8::Value> _ ## FunctionGetter(uint32_t Index, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(FunctionGetter(Index, Info)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	} \
	static v8::Handle<v8::Value> FunctionSetter(uint32_t Index, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info); \
	static v8::Handle<v8::Value> _ ## FunctionSetter(uint32_t Index, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info) { \
		v8::HandleScope HandleScope; \
		try { \
			return HandleScope.Close(FunctionSetter(Index, Value, Info)); \
		} catch(const PExcept& Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
			} else { \
				throw Except; \
			} \
		} \
	}

///////////////////////////////
// Forward declarations 
class TScript; typedef TPt<TScript> PScript;
class TJsBase; typedef TPt<TJsBase> PJsBase;
class TJsFetch;
class TJsFetchRq;
class TJsHttpResp;

///////////////////////////////
/// JavaScript Utility Function
class TJsUtil {
private:
	/// Marks when internals already initialized
	static TBool InitP;
	/// Must be called at the beginning to initialize
	static void Init();

	// json parsing
	static v8::Persistent<v8::String> SecurityToken;
	static v8::Persistent<v8::Context> ParseContext;
	static v8::Persistent<v8::Function> JsonParser;
	static v8::Persistent<v8::Function> JsonString;

    /// Count of current number of objects of each type
    static TStrH ObjNameH;
    /// Number of changes (new/delete) so far
    static TInt ObjCount;    
    /// How often to print the statistics 
    static TInt ObjCountRate;
public:
    /// Add new object to the count table
	static void AddObj(const TStr& ObjName);
    /// Remove existing object from the count table
	static void DelObj(const TStr& ObjName);
    /// Print statistics each so often
	static void CountObj();
    /// Get current statistics
    static TStrIntPrV GetObjStat();
    /// Set rate at which statistics is printed
    static void SetObjStatRate(const int& _ObjCountRate);
        
	/// Parsing V8 exceptions, throws PJsExcept
	static void HandleTryCatch(const v8::TryCatch& try_catch);

	// ??
	static bool NamedAccessCheck(v8::Local<v8::Object> Global,
		v8::Local<v8::Value> Name, v8::AccessType Type, v8::Local<v8::Value> Data);
	static bool IndexedAccessCheck(v8::Local<v8::Object> Global, uint32_t Key,
		v8::AccessType Type, v8::Local<v8::Value> Data);
	
	// Parses PJsonVal to V8::Value
	static v8::Handle<v8::Value> ParseJson(const PJsonVal& JsonVal);
	// Parses TStr as json to V8::Value
	static v8::Handle<v8::Value> ParseJson(const TStr& JsonStr);
	// Parses v8 JSon to V8 String
	static v8::Handle<v8::Value> V8JsonToV8Str(v8::Handle<v8::Value> Json);
	// Parses V8 Json to TStr
	static TStr V8JsonToStr(v8::Handle<v8::Value> Json);

	/// Takes GLib's TTm and returns V8's Date 
	static v8::Handle<v8::Value> GetV8Date(const TTm& Tm);
	/// Takes V8's Date and returns GLib's TTm 
	static TTm GetGlibDate(v8::Handle<v8::Value> Date);	
	/// Returns V8's current date 
	static v8::Handle<v8::Value> GetCurrV8Date();

    /// Converts HttpRq from glib to JSON
    static v8::Handle<v8::Object> HttpRqToJson(PHttpRq HttpRq);
};

///////////////////////////////
/// JavaScript Directory
class TJsFPath {
private:
	/// Canonicalized file path 
	TStr CanonicalFPath; 
    
public:
	TJsFPath(const TStr& FPath = "./");
	
	/// Is directory subdirectory of this 
	bool IsSubdir(const TJsFPath& JsFPath) const; 
	/// Directory equals this 
	bool Equals(const TJsFPath& JsFPath) const; 
	/// Get directory name
	const TStr& GetFPath() const { return CanonicalFPath; }

	/// Load list of directories
	static void GetFPathV(const TStrV& FPathV, TVec<TJsFPath>& JsFPathV);
    /// Canonicalize file path 
    static TStr GetCanonicalPath(const TStr& FPath);
};

///////////////////////////////
// JavaScript Script
class TScript {
private: 
	// smart pointer
	TCRef CRef;
	friend class TPt<TScript>;
	
	// qm and fs require access to private members
	friend class TJsBase;
	friend class TJsFs;

	/// Script name
	TStr ScriptNm;
	/// Script filename
	TStr ScriptFNm;

	/// Directories with include libraries available to this script
	TStrV IncludeFPathV;
	/// Directories this script has read and/or write privileges to
	TVec<TJsFPath> AllowedFPathV;
	
	/// Security token for JavaScript context
	TStr SecurityToken;
	/// Map from server Rules to JavaScript callbacks
    TJsonValV SrvFunRuleV;
    THash<TInt, v8::Persistent<v8::Function> > JsNmFunH;;
	
public:
	/// JavaScript context
	v8::Persistent<v8::Context> Context; 
	/// QMiner Base
	TWPt<TBase> Base;
	/// List of declared triggers
	TVec<TPair<TUInt, PStoreTrigger> > TriggerV;
	/// HTTP web fetcher
	TWPt<TJsFetch> JsFetch;
	
public:
	TScript(const PBase& _Base, const TStr& _ScriptNm, const TStr& _ScriptFNm, 
		const TStrV& _IncludeFPathV, const TVec<TJsFPath>& _AllowedFPathV);
	static PScript New(const PBase& Base, const TStr& ScriptNm, const TStr& ScriptFNm, 
		const TStrV& IncludeFPathV, const TVec<TJsFPath>& AllowedFPathV) { 
			return new TScript(Base, ScriptNm, ScriptFNm, IncludeFPathV, AllowedFPathV); }
    
    // get TScript from global context
    static TWPt<TScript> GetGlobal(v8::Handle<v8::Context>& Context);
    
	~TScript();
	
	/// Get script name
	const TStr& GetScriptNm() const { return ScriptNm; }
	/// Get script filename
	const TStr& GetScriptFNm() const { return ScriptFNm; }
	
	/// Register as server function
	void RegSrvFun(TSAppSrvFunV& SrvFunV);
	/// Reloads the file with the script
	void Reload();

	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun);
	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg);
	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal, v8::Handle<v8::Object>& V8Obj);
	/// Execute JavaScript callback in this script's context
    void Execute(v8::Handle<v8::Function> Fun, v8::Handle<v8::Object>& Arg1, v8::Handle<v8::Object>& Arg2);
    /// Execute JavaScript callback in this script's context
    void Execute(v8::Handle<v8::Function> Fun, v8::Handle<v8::Value>& Arg1, v8::Handle<v8::Value>& Arg2);
    /// Execute JavaScript callback in this script's context
    v8::Handle<v8::Value> ExecuteV8(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
	/// Execute JavaScript callback in this script's context, return double
	double ExecuteFlt(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg);
	/// Execute JavaScript callback in this script's context
    bool ExecuteBool(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Object>& Arg); 
	/// Execute JavaScript callback in this script's context
    bool ExecuteBool(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Object>& Arg1, 
        const v8::Handle<v8::Object>& Arg2);
	/// Execute JavaScript callback in this script's context
	TStr ExecuteStr(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun, const TStr& Str);
	/// Execute JavaScript callback in this script's context
	TStr ExecuteStr(v8::Handle<v8::Function> Fun, const TStr& Str);

	/// Add new server function
	void AddSrvFun(const TStr& ScriptNm, const TStr& FunNm, const TStr& Verb, const v8::Persistent<v8::Function>& JsFun);
	/// Execute stored server function
	void ExecuteSrvFun(const PHttpRq& HttpRq, const TWPt<TJsHttpResp>& JsHttpResp);
    /// Get array of registered server function rules
    PJsonVal GetSrvFunRules() const { return TJsonVal::NewArr(SrvFunRuleV); }
	/// Add new fetch request
	void AddFetchRq(const TJsFetchRq& Rq);
	/// Remember new trigger
	void AddTrigger(const uint& StoreId, const PStoreTrigger& Trigger);

	/// Callback for loading modules in from javascript
	JsDeclareFunction(require);
private:
	/// Initializes main objects and runs the whole script 
	void Init();
	/// Installs main objects in the context
	void Install();
	/// Loads the script from disk and runs preprocessor (imports)
	TStr LoadSource(const TStr& FNm);
	/// Runs the whole script
	void Execute(const TStr& FNm);
	
	/// Load module from given file
	TStr LoadModuleSrc(const TStr& ModuleFNm);
	/// Get library full name (search over all include folders
	TStr GetLibFNm(const TStr& LibNm); 
};

///////////////////////////////
// QMiner-JavaScript-Object-Utility
template <class TJsObj>
class TJsObjUtil {
public:  
	/// Creates new empty JavaScript object around TJsObj
	static v8::Persistent<v8::Object> New(TJsObj* JsObj, const bool& MakeWeakP = true) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::ObjectTemplate> Template = TJsObj::GetTemplate();
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Template->NewInstance());
		Object->SetInternalField(0, v8::External::New(JsObj));
		if (MakeWeakP) { Object.MakeWeak(NULL, &Clean); }
		TJsUtil::AddObj(GetTypeNm<TJsObj>(*JsObj));
		return Object;
	}

	static void PrintObjProperties(v8::Handle<v8::Object> Obj) {
		v8::Local<v8::Array> ValueArr = Obj->GetPropertyNames();
		for (uint32 i = 0; i < ValueArr->Length(); i++) {
			v8::Local<v8::String> Str = ValueArr->Get(i)->ToString();
			v8::String::Utf8Value UtfString(Str);
			const char *CStr = *UtfString;
			printf("%s ", CStr);
		}
	}
	
	static v8::Persistent<v8::Object> New(TJsObj* JsObj, TWPt<TScript> Js, const TStr& ProtoObjPath, const bool& MakeWeakP = true) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::ObjectTemplate> Template = TJsObj::GetTemplate();
		// Get prototype object Obj
		v8::Handle<v8::Object> Global =  Js->Context->Global();
		TStrV ProtoPathV; ProtoObjPath.SplitOnAllCh('.', ProtoPathV, true);
		v8::Handle<v8::Value> Val = Global->Get(v8::String::New(ProtoPathV[0].CStr()));
		v8::Handle<v8::Object> Obj = v8::Handle<v8::Object>::Cast(Val);
		for (int ObjN = 1; ObjN < ProtoPathV.Len(); ObjN++) {
			Val = Obj->Get(v8::String::New(ProtoPathV[ObjN].CStr()));
			Obj = v8::Handle<v8::Object>::Cast(Val);
		}
		v8::Local<v8::Object> Temp = Template->NewInstance();
		Temp->SetPrototype(Obj);
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Temp);
		Object->SetInternalField(0, v8::External::New(JsObj));
		if (MakeWeakP) { Object.MakeWeak(NULL, &Clean); }
		TJsUtil::AddObj(GetTypeNm<TJsObj>(*JsObj));
		return Object;
	}
	
	/// Creates new JavaScript object around TJsObj, using supplied template
	static v8::Persistent<v8::Object> New(TJsObj* JsObj, const v8::Handle<v8::ObjectTemplate>& Template) {
		v8::HandleScope HandleScope;
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Template->NewInstance());
		Object->SetInternalField(0, v8::External::New(JsObj));
		Object.MakeWeak(NULL, &Clean);
		TJsUtil::AddObj(GetTypeNm<TJsObj>(*JsObj));
		return Object;
	}

	/// Destructor of JavaScript object, calls TJsObj destructor
	static void Clean(v8::Persistent<v8::Value> Handle, void* Id) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Object> Object = v8::Handle<v8::Object>::Cast(Handle);
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Object->GetInternalField(0));
                TJsUtil::DelObj(GetTypeNm<TJsObj>(*static_cast<TJsObj*>(WrappedObject->Value())));
		delete static_cast<TJsObj*>(WrappedObject->Value());
		Handle.Dispose();
		Handle.Clear();
	}

	/// Extracts pointer to TJsObj from v8 Arguments
	static TJsObj* GetSelf(const v8::Arguments& Args) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Object> Self = Args.Holder();
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Self->GetInternalField(0));
		TJsObj* JsObj =	static_cast<TJsObj*>(WrappedObject->Value());
		return JsObj;
	}

	/// Extracts pointer to TJsObj from v8 AccessorInfo object
	static TJsObj* GetSelf(const v8::AccessorInfo& Info) {
		v8::HandleScope HandleScope;
		v8::Local<v8::Object> Self = Info.Holder();
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Self->GetInternalField(0));
		TJsObj* JsObj =	static_cast<TJsObj*>(WrappedObject->Value());
		return JsObj;
	}

	static TJsObj* GetSelf(const v8::Handle<v8::Object> Obj) {
		v8::HandleScope HandleScope;
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Obj->GetInternalField(0));
		TJsObj* JsObj = static_cast<TJsObj*>(WrappedObject->Value());
		return JsObj;
	}

	// gets the class name of the underlying glib object. the name is stored in an hidden variable "class"
	static TStr GetClass(const v8::Handle<v8::Object> Obj) {
		v8::HandleScope HandleScope;
		v8::Local<v8::Value> ClassNm = Obj->GetHiddenValue(v8::String::New("class"));
		const bool EmptyP = ClassNm.IsEmpty();
		if (EmptyP) { return ""; }
		v8::String::Utf8Value Utf8(ClassNm);
		return TStr(*Utf8);		
	}

	/// Transform V8 string to TStr
	static TStr GetStr(const v8::Local<v8::String>& V8Str) {
		v8::HandleScope HandleScope;
		v8::String::Utf8Value Utf8(V8Str);
		return TStr(*Utf8);
	}

	/// Extract name of the function called by the script
	static TStr GetFunNm(const v8::Arguments& Args) {
		v8::Handle<v8::Value> Val = Args.Callee()->GetName();
		v8::String::Utf8Value Utf8(Val);
		return TStr(*Utf8);
	}

	/// Check if argument ArgN exists
	static bool IsArg(const v8::Arguments& Args, const int& ArgN) {
		return (Args.Length() > ArgN);
	}

	/// Checks if argument ArgN is of type string
	static bool IsArgStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsString();
	}

	/// Extract argument ArgN as TStr
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
		v8::String::Utf8Value Utf8(Val);
		return TStr(*Utf8);
	}

	/// Extract argument ArgN as TStr, and use DefVal in case when not present
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN, const TStr& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsString(), TStr::Fmt("Argument %d expected to be string", ArgN));
			v8::String::Utf8Value Utf8(Val);
			return TStr(*Utf8);
		}
		return DefVal;
	}

	/// Extract argument ArgN property as string
	static TStr GetArgStr(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const TStr& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				QmAssertR(Val->IsString(), TStr::Fmt("Argument %d, property %s expected to be string", ArgN, Property.CStr()));
				v8::String::Utf8Value Utf8(Val);
				return TStr(*Utf8);
			}
		}
		return DefVal;
	}
   
	/// Check if argument ArgN is of type boolean
	static bool IsArgBool(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsBoolean();
	}

	/// Extract argument ArgN as boolean
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be boolean", ArgN));
		return static_cast<bool>(Val->BooleanValue());
	}

	/// Extract argument ArgN as boolean, and use DefVal in case when not present
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN, const bool& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d expected to be boolean", ArgN));
			return static_cast<bool>(Val->BooleanValue());
		}
		return DefVal;
	}
	
	/// Extract argument ArgN property as bool
	static bool GetArgBool(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				 QmAssertR(Val->IsBoolean(), TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()));
				 return static_cast<bool>(Val->BooleanValue());
			}
		}
		return DefVal;
	}
        
	/// Check if argument ArgN is of type integer
	static bool IsArgInt32(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsInt32();
	}

	/// Extract argument ArgN as integer
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int32", ArgN));
		return static_cast<int>(Val->Int32Value());
	}

	/// Extract argument ArgN as integer, and use DefVal in case when not present
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN, const int& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d expected to be int32", ArgN));
			return static_cast<int>(Val->Int32Value());
		}
		return DefVal;
	}
	/// Extract argument ArgN property as int
	static int GetArgInt32(const v8::Arguments& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
				 QmAssertR(Val->IsInt32(), TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()));
				 return Val->ToNumber()->Int32Value();
			}
		}
		return DefVal;
	}
     
   	/// Check if argument ArgN is of type double
	static bool IsArgFlt(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsNumber();
	}

	/// Extract argument ArgN as double
	static double GetArgFlt(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be double", ArgN));
		return static_cast<double>(Val->NumberValue());
	}

	/// Check if argument ArgN belongs to a given class
	static bool IsArgClass(const v8::Arguments& Args, const int& ArgN, const TStr& ClassNm) {
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()));
        QmAssertR(Args[ArgN]->IsObject(), "Argument expected to be '" + ClassNm + "' but is not even an object!");
		v8::Handle<v8::Value> Val = Args[ArgN];
	 	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);			
		TStr ClassStr = GetClass(Data);
		return ClassStr.EqI(ClassNm);
	}

	/// Extract argument ArgN as a js object
	static TJsObj* GetArgObj(const v8::Arguments& Args, const int& ArgN) {
		v8::Handle<v8::Value> Val = Args[ArgN];
    	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);	
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Data->GetInternalField(0));
		return static_cast<TJsObj*>(WrappedObject->Value());		
	}

	/// Extract argument ArgN as double, and use DefVal in case when not present
	static double GetArgFlt(const v8::Arguments& Args, const int& ArgN, const double& DefVal) {
		v8::HandleScope HandleScope;
		if (Args.Length() > ArgN) {
			v8::Handle<v8::Value> Val = Args[ArgN];
			QmAssertR(Val->IsNumber(), TStr::Fmt("Argument %d expected to be double", ArgN));
			return static_cast<double>(Val->NumberValue());
		}
		return DefVal;
	}
    
   	/// Check if argument ArgN is of type json
	static bool IsArgJson(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsObject();
	}    

	/// Extract argument ArgN as JSon object, and serialize it to TStr
	static TStr GetArgJsonStr(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be json", ArgN));
		TStr JsonStr = TJsUtil::V8JsonToStr(Val);
		return JsonStr;
	}

	static TStr GetArgJsonStr(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgJsonStr : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject() &&
			Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())),
			TStr::Fmt("TJsObjUtil::GetArgJsonStr : Argument %d must be an object with property %s", ArgN, Property.CStr()));

		v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
		QmAssertR(Val->IsObject(), TStr::Fmt("TJsObjUtil::GetArgJsonStr : Args[%d].%s expected to be json", ArgN, Property.CStr()));
		TStr JsonStr = TJsUtil::V8JsonToStr(Val);
		return JsonStr;
	}

	/// Extract argument ArgN as JSon object, and transform it to PJsonVal
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN) {
		TStr JsonStr = GetArgJsonStr(Args, ArgN);
		PJsonVal Val = TJsonVal::GetValFromStr(JsonStr);
		if (!Val->IsDef()) { throw TQmExcept::New("Error parsing '" + JsonStr + "'."); }
		return Val;
	}

	/// Extract argument ArgN property as json
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgJson : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject() &&
			Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())),
			TStr::Fmt("TJsObjUtil::GetArgJson : Argument %d must be an object with property %s", ArgN, Property.CStr()));
		TStr JsonStr = GetArgJsonStr(Args, ArgN, Property);
		PJsonVal Val = TJsonVal::GetValFromStr(JsonStr);
		if (!Val->IsDef()) { throw TQmExcept::New("TJsObjUtil::GetArgJson : Error parsing '" + JsonStr + "'."); }
		return Val;
	}

	static bool IsArgFun(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		return Val->IsFunction();
	}

	/// Extract argument ArgN as JavaScript function
	static v8::Handle<v8::Function> GetArgFun(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsFunction(), TStr::Fmt("Argument %d expected to be function", ArgN));
		return HandleScope.Close(v8::Handle<v8::Function>::Cast(Val));
	}
	
	/// Extract argument ArgN as persistent JavaScript function
	static v8::Persistent<v8::Function> GetArgFunPer(const v8::Arguments& Args, const int& ArgN) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
		v8::Handle<v8::Value> Val = Args[ArgN];
		QmAssertR(Val->IsFunction(), TStr::Fmt("Argument %d expected to be function", ArgN));
		return v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Val));
	}

	/// Extract argument ArgN property as persistent Javascript function
	static v8::Persistent<v8::Function> GetArgFunPer(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgFunPer : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject() &&
			Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())),
			TStr::Fmt("TJsObjUtil::GetArgFunPer : Argument %d must be an object with property %s", ArgN, Property.CStr()));
		v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::New(Property.CStr()));
		QmAssertR(Val->IsFunction(), TStr::Fmt("TJsObjUtil::GetArgFunPer Argument[%d].%s expected to be function", ArgN, Property.CStr()));
		//return HandleScope.Close(v8::Handle<v8::Function>::Cast(Val));
		return v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Val));
	}

};

///////////////////////////////
// JavaScript Server Function
class TJsSrvFun : public TSAppSrvFun {
private:
	/// JS script context
	TWPt<TScript> Js;

	TJsSrvFun(TWPt<TScript> _Js, const TStr& _FunNm): 
        TSAppSrvFun(_FunNm, saotCustom), Js(_Js) { }
public:
	static PSAppSrvFun New(TWPt<TScript> Js, const TStr& FunNm) { 
		return new TJsSrvFun(Js, FunNm); }
	void Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////
// JavaScript Server Function
class TJsAdminSrvFun : public TSAppSrvFun {
private:
    /// List of existing JS contexts
    TVec<TWPt<TScript> > ScriptV;

	TJsAdminSrvFun(const TVec<PScript>& _ScriptV, const TStr& _FunNm): 
        TSAppSrvFun(_FunNm, saotCustom) { }
public:
	static PSAppSrvFun New(const TVec<PScript>& ScriptV, const TStr& FunNm) { 
		return new TJsAdminSrvFun(ScriptV, FunNm); }
	void Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

///////////////////////////////
// JavaScript Store Trigger
class TJsStoreTrigger : public TStoreTrigger {
private:
	/// JS script context
	TWPt<TScript> Js;
	// callbacks
	v8::Persistent<v8::Function> OnAddFun;
	v8::Persistent<v8::Function> OnUpdateFun;
	v8::Persistent<v8::Function> OnDeleteFun;

public:
	TJsStoreTrigger(TWPt<TScript> _Js, v8::Handle<v8::Object> TriggerVal);
	static PStoreTrigger New(TWPt<TScript> Js, v8::Handle<v8::Object> TriggerVal) {
		return new TJsStoreTrigger(Js, TriggerVal); }

	void OnAdd(const TRec& Rec);
	void OnUpdate(const TRec& Rec);
	void OnDelete(const TRec& Rec);
};

///////////////////////////////
// JavaScript WebPgFetch Request
class TJsFetchRq {
private:
	/// HTTP request header
	PHttpRq HttpRq; 
	/// True when there is data expected
	TBool JsonP;
	/// on success callback
	v8::Persistent<v8::Function> OnSuccessFun; 
	/// on error callback
	v8::Persistent<v8::Function> OnErrorFun; 

public:
	TJsFetchRq() { }
	/// new request with javascript callbacks
	TJsFetchRq(const PHttpRq& _HttpRq, const bool& _JsonP, 
		const v8::Persistent<v8::Function>& _OnSuccessFun,
		const v8::Persistent<v8::Function>& _OnErrorFun): 
			HttpRq(_HttpRq), JsonP(_JsonP), 
			OnSuccessFun(_OnSuccessFun), OnErrorFun(_OnErrorFun) { }

	/// Get requested URL
	TStr GetUrlStr() const { return HttpRq->GetUrl()->GetUrlStr(); }
	/// Get request HTTP request
	PHttpRq GetHttpRq() const { return HttpRq; }
	/// Requires response data
	bool IsJson() const { return JsonP; }
	/// Is there on success callback
	bool IsOnSuccess() const { return !OnSuccessFun.IsEmpty(); }
	/// Get on success callback
	v8::Persistent<v8::Function> GetOnSuccess() const { return OnSuccessFun; }
	/// IS there on success callback
	bool IsOnError() const { return !OnErrorFun.IsEmpty(); }
	/// Get on success callback
	v8::Persistent<v8::Function> GetOnError() const { return OnErrorFun; }
};

///////////////////////////////
// JavaScript WebPgFetch
class TJsFetch : private TWebPgFetch {
private:
	/// JS script context
	TWPt<TScript> Js;
	/// List of current request callbacks
	THash<TInt, TJsFetchRq> CallbackH;

protected:
	void OnFetch(const int& FId, const PWebPg& WebPg);
	void OnError(const int& FId, const TStr& MsgStr);

public:
	TJsFetch(TWPt<TScript> _Js): Js(_Js) { PutMxConns(10); }
	
	void Fetch(const TJsFetchRq& Rq);
};

///////////////////////////////
// JavaScript QMiner Base
//# 
//# ## Core QMiner objects
//# 
//# ### QMiner
//# 
//# QMiner (`qm`) is the core object in the API and is available in any script.
class TJsBase {
public:
	/// JS script context
	TWPt<TScript> Js;
	/// QMiner base
	TWPt<TBase> Base;
	
private:
	/// Object utility class
	typedef TJsObjUtil<TJsBase> TJsBaseUtil;

    TJsBase(TWPt<TScript> _Js);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
		return TJsBaseUtil::New(new TJsBase(Js)); }
    ~TJsBase() { }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

    // temporary stuff
	JsDeclareProperty(analytics); // deprecated    
	JsDeclareProperty(args); // to be moved to TJsProcess once created
    JsDeclareProperty(sysStat);  // to be moved to TJsProcess once created
	JsDeclareFunction(op); // soon to be deprecated, functionality moved to TJsRecSet

	//# 
	//# **Functions and properties:**
	//# 
    //#- `s = qm.store(storeName)` -- store with name `storeName`; `null` when no such store
	JsDeclareFunction(store);
    //#- `a = qm.getStoreList()` -- an array listing all existing stores
	JsDeclareFunction(getStoreList);
    //#- `qm.createStore(storeDef)` -- create new store(s) based on given [definition](Store Definition)
	JsDeclareFunction(createStore);
    //#- `rs = qm.search(query)` -- execute `query` specified in [QMiner Query Language](Query Language) 
    //#   and returns a record set `rs` with results
	JsDeclareFunction(search);   
    //#-- `qm.gc()` -- start garbage collection to remove records outside time windows
	JsDeclareFunction(gc);
	//#JSIMPLEMENT:src/qminer/qminer.js    
};

///////////////////////////////
// QMiner-JavaScript-Store
//# 
//# ### Store
//# 
//# Store holds records. Each record has a unique 64-bit ID. Record ID can be used to directly
//# access the record from the store using index operator:
//# ```JavaScript
//# var store = qm.store("storeName");
//# var record = store[1234];
//# ```
class TJsStore {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;

	typedef TJsObjUtil<TJsStore> TJsStoreUtil;

	TJsStore(TWPt<TScript> _Js, TWPt<TStore> _Store): Js(_Js), Store(_Store) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TWPt<TStore> Store) { 
		return TJsStoreUtil::New(new TJsStore(Js, Store)); }
	~TJsStore() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `store.name` -- name of the store
	JsDeclareProperty(name);
    //#- `store.empty` -- `true` when store is empty
	JsDeclareProperty(empty);
    //#- `store.length` -- number of records in the store
	JsDeclareProperty(length);
    //#- `rs = store.recs` -- create a record set containing all the records from the store
	JsDeclareProperty(recs);
    //#- `store.fields` -- array of all the field names
	JsDeclareProperty(fields);
    //#- `store.joins` -- array of all the join names
	JsDeclareProperty(joins);	
    //#- `store.keys` -- array of all the [index keys](#index-key)
	JsDeclareProperty(keys);	
    //#- `r = store[recordId]` -- get record with ID `recordId`; 
    //#     returns `null` when no such record exists
	JsDeclIndexedProperty(indexId);
    //#- `r = store.rec(recordName)` -- get record named `recordName`; 
    //#     returns `null` when no such record exists
	JsDeclareFunction(rec);
    //#- `store.add(record)` -- add `record` to the store 
	JsDeclareFunction(add);
    //#- `r = store.newRec(recordJson)` -- creates new record by value (not added to the store)
    JsDeclareFunction(newRec);
    //#- `r = store.newRec(recordIds)` -- creates new record set from array of record IDs;
    //#     array is expected to be of type `la.newIntVec`
	JsDeclareFunction(newRecSet);
    //#- `rs = store.sample(sampleSize)` -- create a record set containing a random 
    //#     sample of `sampleSize` records
	JsDeclareFunction(sample);
    //#- `field = store.field(fieldName)` -- get details of field named `fieldName`
	JsDeclareFunction(field);
    //#- `key = store.key(keyName)` -- get [index key](#index-key) named `keyName`
	JsDeclareFunction(key);
    //#- `store.addTrigger(trigger)` -- add `trigger` to the store triggers
	JsDeclareFunction(addTrigger);
    //#- `store.addStreamAggr(TypeName, Parameters);` -- add new [Stream Aggregate](Stream-Aggregates) 
    //#     of type `TypeName` to the store; stream aggregate is passed `Parameters` JSon
    JsDeclareFunction(addStreamAggr);
    //#- `sa = store.getStreamAggr(Name)` -- returns current value of stream aggregate `Name`
	JsDeclareFunction(getStreamAggr);
	//#- `arr = store.getStreamAggrNames()` -- returns the names of all stream aggregators as an array of strings `arr`
	JsDeclareFunction(getStreamAggrNames);
    
    //# 
    //# **Examples**:
    //# 
    //# ```JavaScript
    //# // adding new record
    //# qm.store("Movies").add({
    //#   Title: "The Hobbit: An Unexpected Journey", 
    //#   Year: 2012, 
    //#   directedBy: {
    //#     Name: "Peter Jackson",
    //#     Gender: "Male"
    //#   }
    //# }
    //# 
    //# // adding a trigger
    //# qm.store("People").addTrigger({
    //#   onAdd : function (person) { console.log("New record: " + person.Name); },
    //#   onUpdate : function (person) { console.log("Updated record: " + person.Name); },
    //#   onDelete : function (person) { console.log("Deleted record: " + person.Name); }
    //# });
    //# 
    //# // iterating over records
    //# var rs = qm.store("People");
    //# for (var i = 0; i < rs.length; i++) {
    //#   var rec = rs[i];
    //#   console.log(rec.Name + " (" + rec.Gender + ")");
    //# }
    //# ```    
};
    

///////////////////////////////
// JavaScript Record Comparator
class TJsRecCmp {
private:
	/// JS script context
	TWPt<TScript> Js;
	TWPt<TStore> Store;
	// callbacks
	v8::Persistent<v8::Function> CmpFun;

public:
	TJsRecCmp(TWPt<TScript> _Js, TWPt<TStore> _Store, 
        const v8::Persistent<v8::Function>& _CmpFun): 
            Js(_Js), Store(_Store), CmpFun(_CmpFun) { }

    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const;
};

///////////////////////////////
// JavaScript Record Filter
class TJsRecFilter {
private:
	/// JS script context
	TWPt<TScript> Js;
	TWPt<TStore> Store;
	// callbacks
	v8::Persistent<v8::Function> FilterFun;

public:
	TJsRecFilter(TWPt<TScript> _Js, TWPt<TStore> _Store, 
        const v8::Persistent<v8::Function>& _FilterFun): 
            Js(_Js), Store(_Store), FilterFun(_FilterFun) { }

    bool operator()(const TUInt64IntKd& RecIdWgt) const;
};

///////////////////////////////
// QMiner-JavaScript-Record-Set
//#
//# ### Record set
//#
//# Record set holds a subset of records from a store. Records are stored in a vector. 
//# It can also hold a vector of aggregates, which were computed over the records from 
//# the set.
class TJsRecSet {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;
	PRecSet RecSet;	

	typedef TJsObjUtil<TJsRecSet> TJsRecSetUtil;

	TJsRecSet(TWPt<TScript> _Js, const PRecSet& _RecSet);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PRecSet& RecSet);
	static PRecSet GetArgRecSet(const v8::Arguments& Args, const int& ArgN);
	~TJsRecSet() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//#   
	//#- `rs.store` -- store of the records
	JsDeclareProperty(store);
	//#- `rs.length` -- number of records in the set
	JsDeclareProperty(length);
	//#- `rs.empty` -- `true` when record set is empty
	JsDeclareProperty(empty);
	//#- `rs.weighted` -- true when records in the set are assigned weights
	JsDeclareProperty(weighted);
    //#- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
	//#- `crs = rs.clone()` -- creates new instance of record set
	JsDeclareFunction(clone);
	//#- `jrs = rs.join(JoinName)` -- executes a join `JoinName` on the records in the set, result is another record set
	//#- `jrs = rs.join(JoinName, SampleSize)` -- executes a join `JoinName` on a sample of `SampleSize` records in the set, result is another record set
	JsDeclareFunction(join);
	//#- `aggr = rs.aggr()` -- returns an array of all the aggregates contained in the records set
	//#- `aggr = rs.aggr(Query)` -- creates a new aggregates based on the `Query` parameters
	JsDeclareFunction(aggr);
	//#- `rs.trunc(Recs)` -- truncate to first `Recs` record
	JsDeclareFunction(trunc);
	//#- `srs = rs.sample(Recs)` -- create new record set by randomly sampling `Recs` records
	JsDeclareFunction(sample);
	//#- `rs.shuffle(Seed)` -- shuffle order using random seed `Seed`
	JsDeclareFunction(shuffle);
	//#- `rs.reverse()` -- reverse record order
	JsDeclareFunction(reverse);
	//#- `rs.sortById(Asc)` -- sort records according to record id; if `Asc > 0` sorted in ascending order
	JsDeclareFunction(sortById);
	//#- `rs.sortByFq(Asc)` -- sort records according to weight; if `Asc > 0` sorted in ascending order
	JsDeclareFunction(sortByFq);
	//#- `rs.sortByField(FieldName, Asc)` -- sort records according to value of field `FieldName`; if `Asc > 0` sorted in ascending order
	JsDeclareFunction(sortByField);
	//#- `rs.sort(comparator)` -- sort records according to `comparator` callback
   	JsDeclareFunction(sort);
	//#- `rs.filterById(minId, maxId)` -- keeps only records with ids between `minId` and `maxId`
	JsDeclareFunction(filterById);
	//#- `rs.filterByFq(minFq, maxFq)` -- keeps only records with weight between `minFq` and `maxFq`
	JsDeclareFunction(filterByFq);
	//#- `rs.filterByField(FieldName, minVal, maxVal)` -- keeps only records with numeric value of field `FieldName` between `minVal` and `maxVal`
	//#- `rs.filterByField(FieldName, Val)` -- keeps only records with string value of field `FieldName` equal to `Val`
	JsDeclareFunction(filterByField);
	//#- `rs.filter(filter)` -- keeps only records that pass `filter` callback
	JsDeclareFunction(filter);
    //#- `rs.deleteRecs(rs2)` -- delete from `rs` records that are also in `rs2`
	JsDeclareFunction(deleteRecs);
    //#- `rs.toJSON()` -- provide json version of record set, useful when calling JSON.stringify
	JsDeclareFunction(toJSON);
	//#- `rs.map(callback)` -- iterates through the record set and executes the callback function `callback` on each element:
	//#   `rs.map(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	JsDeclareFunction(map);
	//#- `rs2 = rs.setintersect(rs1)` -- returns the intersection (record set) `rs2` between two record sets `rs` and `rs1`, which should point to the same store.
	JsDeclareFunction(setintersect);
	//#- `rs2 = rs.setunion(rs1)` -- returns the union (record set) `rs2` between two record sets `rs` and `rs1`, which should point to the same store.
	JsDeclareFunction(setunion);
	//#- `rs2 = rs.setdiff(rs1)` -- returns the set difference (record set) `rs2`=`rs`\`rs1`  between two record sets `rs` and `rs1`, which should point to the same store.
	JsDeclareFunction(setdiff);


    //# 
    //# **Examples**:
    //# 
    //# ```JavaScript
    //# TODO
    //# ```        
};

///////////////////////////////
// QMiner-JavaScript-Record
//#
//# ### Record
//#
class TJsRec {
public:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;
	TRec Rec;
	TInt Fq;

private:
	typedef TJsObjUtil<TJsRec> TJsRecUtil;
	// we have a separate template for each store
	static TVec<v8::Persistent<v8::ObjectTemplate> > TemplateV;

	TJsRec(TWPt<TScript> _Js, const TRec& _Rec, const int& _Fq): 
		Js(_Js), Store(_Rec.GetStore()), Rec(_Rec), Fq(_Fq) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TRec& Rec, const int& Fq = 0) { 
		return TJsRecUtil::New(new TJsRec(Js, Rec, Fq), GetTemplate(Js->Base, Rec.GetStore())); }
    static TRec GetArgRec(const v8::Arguments& Args, const int& ArgN);

	~TJsRec() { }

	static v8::Handle<v8::ObjectTemplate> GetTemplate(const TWPt<TBase>& Base, const TWPt<TStore>& Store);

	//# 
	//# **Functions and properties:**
	//#   
    //#- `rec.$id`
    JsDeclareProperty(id);
    //#- `rec.$name`
    JsDeclareProperty(name);
    //#- `rec.$fq`
	JsDeclareProperty(fq);
    //#- `rec.fieldName`
	JsDeclareSetProperty(getField, setField);
    //#- `rec.joinName`
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
    //#- `rec.addJoin(joinName, joinRecord)`
    //#- `rec.addJoin(joinName, joinRecord, joinFrequency)`
    JsDeclareFunction(addJoin);
    //#- `rec.delJoin(joinName, joinRecord)`
    //#- `rec.delJoin(joinName, joinRecord, joinFrequency)`
    JsDeclareFunction(delJoin);
    //#- `rec.toJSON()` -- provide json version of record, useful when calling JSON.stringify
    JsDeclareFunction(toJSON);
};

///////////////////////////////
// QMiner-JavaScript-IndexKey
//# 
//# ### Index key
//# 
class TJsIndexKey {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TIndexKey IndexKey;

	typedef TJsObjUtil<TJsIndexKey> TJsIndexKeyUtil;

	TJsIndexKey(TWPt<TScript> _Js, const TIndexKey& _IndexKey): 
		Js(_Js), IndexKey(_IndexKey) { }	
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TIndexKey& IndexKey) { 
		return TJsIndexKeyUtil::New(new TJsIndexKey(Js, IndexKey)); }
	~TJsIndexKey() { }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#   
    //#- `key.store`    
	JsDeclareProperty(store);				
    //#- `key.name`    
	JsDeclareProperty(name);
    //#- `key.voc`    
	JsDeclareProperty(voc);
    //#- `key.fq`    
	JsDeclareProperty(fq);
};

///////////////////////////////
// QMiner-LinAlg
//# 
//# ## Linear Algebra
//# 
//# A global object `la` is used to construct vectors (sparse, dense) and matrices and 
//# it is available in any script. The object includes
//# several functions from linear algebra.
class TJsLinAlg {
public:
	/// JS script context
	TWPt<TScript> Js;    
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsLinAlg> TJsLinAlgUtil;
    
    explicit TJsLinAlg(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
		return TJsLinAlgUtil::New(new TJsLinAlg(Js)); }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `vec = la.newVec()` -- generate an empty float vector
	//#- `vec = la.newVec({"vals":k, "mxvals":l})` -- generate a vector with `k` zeros and reserve additional `l-k` elements 
	//#- `vec = la.newVec(a)` -- copy a javascript number array `a` 
	//#- `vec = la.newVec(vec2)` -- clone a float vector `vec2`
	JsDeclareFunction(newVec);
	//#- `vec = la.newIntVec()` -- generate an empty float vector
	//#- `vec = la.newIntVec({"vals":k, "mxvals":l})` -- generate a vector with `k` zeros and reserve additional `l-k` elements 
	//#- `vec = la.newIntVec(a)` -- copy a javascript int array `a` 
	//#- `vec = la.newIntVec(vec2)` -- clone an int vector `vec2`
	JsDeclareFunction(newIntVec);
	//#- `mat = la.newMat()` -- generates a 0x0 matrix
	//#- `mat = la.newMat(a)` -- generates a matrix from a javascript array `a`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
	//#- `mat = la.newMat({"rows":r, "cols":c, "random":b})` -- creates a matrix with `r` rows and `c` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
	//#- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`
	JsDeclareFunction(newMat);
	//#- `vec = la.newSpVec(dim)` -- creates an empty sparse vector `vec`, where `dim` is an optional (-1 by default) integer parameter that sets the dimension
	//#- `vec = la.newSpVec(a, dim)` -- creats a sparse vector `vec` from a javascript array `a`, whose elements are javascript arrays with two elements (integer row index and double value). `dim` is optional and sets the dimension
	JsDeclareFunction(newSpVec);
	//#- `mat = la.newSpMat()` -- creates an empty sparse matrix `mat`
	//#- `mat = la.newSpMat(rowIdxV, colIdxV, valV)` -- creates an sparse matrix based on two int vectors `rowIdxV` (row indices) and `colIdxV` (column indices) and float vector of values `valV`
	//#- `mat = la.newSpMat(a, r)` -- creates an sparse matrix with `r` rows (optional parameter), where `a` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `mat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
	//#- `mat = la.newSpMat({"rows":r, "cols":c}) --- creates a sparse matrix with `c` columns and `r` rows, which should be integers
	JsDeclareFunction(newSpMat);
	//#- `res = la.svd(mat, k, {"iter":iter, "tol":tol})` -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter object contains integer number of iterations `iter` (default 2) and the tolerance number `tol` (default 1e-6). The outpus are stored as two dense matrices: `res.U`, `res.V` and a dense float vector `res.s`.
	JsDeclareFunction(svd);	
	//#JSIMPLEMENT:src/qminer/linalg.js
};

///////////////////////////////
// QMiner-Vector
//# 
//# ### Vector
//# 
//# Vector is an array of objects implemented in glib/base/ds.h. 
//# Some functions are implemented for float vectors only. Using the global `la` object, flaot and int vectors can be generated in the following ways:
//# 
//# ```JavaScript
//# var fltv = la.newVec(); //empty vector
//# var intv = la.newIntVec(); //empty vector
//# // refer to la.newVec, la.newIntVec functions for alternative ways to generate vectors
//# ```
//# 
template <class TVal, class TAux> class TJsVec;

class TAuxFltV {
public:	
	static const TStr ClassId; //ClassId is set to "TFltV"
	static v8::Handle<v8::Value> GetObjVal(const double& Val, v8::HandleScope& Handlescope) {
		return Handlescope.Close(v8::Number::New(Val));
	}
	static double GetArgVal(const v8::Arguments& Args, const int& ArgN) {
		return TJsObjUtil<TJsVec<TFlt, TAuxFltV> >::GetArgFlt(Args, ArgN);
	}
	static double CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToNumber()->Value();
	}
};

class TAuxIntV {
public:	
	static const TStr ClassId; //ClassId is set to "TIntV"
	static v8::Handle<v8::Value> GetObjVal(const int& Val, v8::HandleScope& Handlescope) {
		return Handlescope.Close(v8::Integer::New(Val));
	}
	static int GetArgVal(const v8::Arguments& Args, const int& ArgN) {
		return TJsObjUtil<TJsVec<TInt, TAuxIntV> >::GetArgInt32(Args, ArgN);
	}
	static int CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToInt32()->Value();
	}
};

template <class TVal = TFlt, class TAux = TAuxFltV>
class TJsVec {
public:
	/// JS script context
	TWPt<TScript> Js;    
	typedef TVec<TVal> TValV;
	TValV Vec;
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsVec<TVal, TAux> > TJsVecUtil;
    explicit TJsVec(TWPt<TScript> _Js): Js(_Js) { }	
public:	
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		v8::Persistent<v8::Object> obj = TJsVecUtil::New(new TJsVec(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New(TAux::ClassId.CStr());
		obj->SetHiddenValue(key, value);		
		return  obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TValV& _Vec) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsVec::SetVec(obj, _Vec);		
		return  obj;
	}
	static TValV& GetVec(const v8::Handle<v8::Object> Obj) {
		return TJsVecUtil::GetSelf(Obj)->Vec;
	}
	static void SetVec(const v8::Handle<v8::Object> Obj, const TValV& _Vec) {
		TJsVecUtil::GetSelf(Obj)->Vec = _Vec;
	}
	
	/// template	
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `x = vec.at(elN)` -- gets the value `x` at index `elN` of vector `vec` (0-based indexing)
	JsDeclareFunction(at);
	//#- `x = vec[elN]; vec[elN] = y` -- get value `x` at index `elN`, set value at index `elN` to `y` of vector `vec`(0-based indexing)
	JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `vec.put(elN, y)` -- set value at index `elN` to `y` of vector `vec`(0-based indexing)
	JsDeclareFunction(put);	
	//#- `vec.push(y)` -- append value `y` to vector `vec`
	JsDeclareFunction(push);
	//#- `vec.unshift(y)` -- insert value `y` to the begining of vector `vec`. Returns the length of the modified array.
	JsDeclareFunction(unshift);
	//#- `vec.pushV(vec2)` -- append vector `vec2` to vector `vec`. Implemented for dense integer and dense float vectors.
	JsDeclareTemplatedFunction(pushV);
	//#- `x = vec.sum()` -- sums the elements of `vec`
	JsDeclareFunction(sum);
	//#- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in `vec`
	JsDeclareFunction(getMaxIdx);
	//#- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	JsDeclareFunction(sort);
	//#- `res = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `res.vec` and the permutation `res.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(sortPerm);	
	//#- `mat = vec1.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec1 * vec2^T`. Implemented for dense float vectors. 
	JsDeclareTemplatedFunction(outer);
	//#- `x = vec1.inner(vec2)` -- `x` is the standard dot product between vectors `vec1` and `vec2`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(inner);
	//#- `vec3 = vec1.plus(vec2)` --`vec3` is the sum of vectors `vec1` and `vec2`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(plus);
	//#- `vec3 = vec1.minus(vec2)` --`vec3` is the difference of vectors `vec1` and `vec2`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(minus);
	//#- `vec2 = vec1.multiply(a)` --`vec2` is a vector obtained by multiplying vector `vec1` with a scalar (number) `a`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(multiply);
	//#- `vec.normalize();` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors.
	JsDeclareTemplatedFunction(normalize);
	//#- `len = vec.length` -- integer `len` is the length of vector `vec`
	JsDeclareProperty(length);
	//#- `vec.print()` -- print vector in console
	JsDeclareFunction(print);
	//#- `D = vec.diag()` -- `D` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(diag);
	//#- `D = vec.spDiag()` -- `D` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(spDiag);
	//#- `x = vec.norm()` -- `x` is the Euclidean norm of `vec`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(norm);
	//#- `vec2 = vec.sparse()` -- `vec2` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors.
	JsDeclareTemplatedFunction(sparse);
};
typedef TJsVec<TFlt, TAuxFltV> TJsFltV;
typedef TJsVec<TInt, TAuxIntV> TJsIntV;

template <class TVal, class TAux>
v8::Handle<v8::ObjectTemplate> TJsVec<TVal, TAux>::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, at);	
		JsRegGetSetIndexedProperty(TmpTemp, indexGet, indexSet);
		JsRegisterFunction(TmpTemp, put);
		JsRegisterFunction(TmpTemp, push);
		JsRegisterFunction(TmpTemp, unshift);
		JsRegisterFunction(TmpTemp, pushV);
		JsRegisterFunction(TmpTemp, sum);
		JsRegisterFunction(TmpTemp, getMaxIdx);
		JsRegisterFunction(TmpTemp, sort);
		JsRegisterFunction(TmpTemp, sortPerm);
		JsRegisterFunction(TmpTemp, outer);
		JsRegisterFunction(TmpTemp, inner);
		JsRegisterFunction(TmpTemp, plus);
		JsRegisterFunction(TmpTemp, minus);
		JsRegisterFunction(TmpTemp, multiply);
		JsRegisterFunction(TmpTemp, normalize);
		JsRegisterProperty(TmpTemp, length);
		JsRegisterFunction(TmpTemp, print);
		JsRegisterFunction(TmpTemp, diag);
		JsRegisterFunction(TmpTemp, spDiag);	
		JsRegisterFunction(TmpTemp, norm);
		JsRegisterFunction(TmpTemp, sparse);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);		
	}
	return Template;
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::at(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Args);
	TInt Index = TJsVecUtil::GetArgInt32(Args, 0);	
	QmAssertR(Index >= 0 && Index < JsVec->Vec.Len(), "vector at: index out of bounds");
	TVal result = JsVec->Vec[Index];
	return TAux::GetObjVal(result, HandleScope);
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::indexGet(uint32_t Index, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Info);	
	QmAssertR(Index < (uint32_t)JsVec->Vec.Len(), "vector at: index out of bounds");
	TVal result = JsVec->Vec[Index];
	return TAux::GetObjVal(result, HandleScope);
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::put(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Args);
	if (Args.Length() == 2) {
		QmAssertR(Args[0]->IsInt32(), "the first argument should be an integer");
		QmAssertR(Args[1]->IsNumber(), "the second argument should be a number");				
		TInt Index = TJsVecUtil::GetArgInt32(Args, 0);
		TVal Val = TAux::GetArgVal(Args, 1);	
		QmAssertR(Index >= 0 && Index < JsVec->Vec.Len(), "vector put: index out of bounds");		
		JsVec->Vec[Index] = Val;
	}
	return HandleScope.Close(v8::Undefined());	
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::indexSet(uint32_t Index, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Info);
	QmAssertR(Index < (uint32_t)JsVec->Vec.Len(), "vector at: index out of bounds");
	TVal Val = TAux::CastVal(Value);	
	JsVec->Vec[Index] = Val;	
	return HandleScope.Close(v8::Undefined());	
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::push(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Args);	
	// assume number
	TVal Val = TAux::GetArgVal(Args, 0);	
	JsVec->Vec.Add(Val);	
	return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));	
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::unshift(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Args);
	// assume number
	TVal Val = TAux::GetArgVal(Args, 0);

	JsVec->Vec.Ins(0, Val);
	return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::sum(const v8::Arguments& Args) {
	// currently only float vectors are supported
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	double result = 0.0;
	int Els = JsVec->Vec.Len();
	if (Els > 0) {
		for (int ElN = 0; ElN < Els; ElN++) {
			result += JsVec->Vec[ElN];
		}
	}
	return HandleScope.Close(v8::Number::New(result));
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::getMaxIdx(const v8::Arguments& Args) {
	// currently only float vectors are supported
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	double Val = TFlt::Mn;
	int Idx = -1;	
	int Els = JsVec->Vec.Len();
	for (int ElN = 0; ElN < Els; ElN++) {
		if (JsVec->Vec[ElN] > Val) {
			Val = JsVec->Vec[ElN];
			Idx = ElN;
		}		
	}
	return HandleScope.Close(v8::Int32::New(Idx));
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::sort(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	bool Asc = true;
	if (Args.Length() > 0) {
		if (Args[0]->IsBoolean()) {
			Asc = Args[0]->BooleanValue();
		}
	}
	v8::Persistent<v8::Object> JsResult = TJsVec<TVal,TAux>::New(JsVec->Js);
	TVec<TVal>& Result = JsVec->Vec;
	Result.Sort(Asc);
	return HandleScope.Close(JsResult);
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::print(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec<TVal, TAux>* JsVec = TJsVecUtil::GetSelf(Args);
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ElN++) {
		printf("%s ", JsVec->Vec[ElN].GetStr().CStr());		
	}
	printf("\n");	
	return HandleScope.Close(v8::Undefined());	
}

///////////////////////////////
// QMiner-FltVV
//# 
//# ### Matrix (dense matrix)
//# 
//# Matrix is a double 2D array implemented in glib/base/ds.h. 
//# Using the global `la` object, dense matrices are generated in several ways:
//# 
//# ```JavaScript
//# var fltv = la.newVec(); //empty matrix
//# // refer to la.newMat function for alternative ways to generate dense matrices
//# ```
//# 
class TJsFltVV {
public:
	/// JS script context
	TWPt<TScript> Js;    
	TFltVV Mat;
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsFltVV> TJsFltVVUtil;    
    explicit TJsFltVV(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 		
		v8::Persistent<v8::Object> obj = TJsFltVVUtil::New(new TJsFltVV(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TFltVV");
		obj->SetHiddenValue(key, value);
		return  obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TFltVV& _Mat) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsFltVV::SetFltVV(obj, _Mat);		
		return  obj;
	}
	static TFltVV& GetFltVV(const v8::Handle<v8::Object> Obj) {
		return TJsFltVVUtil::GetSelf(Obj)->Mat;
	}
	static void SetFltVV(const v8::Handle<v8::Object> Obj, const TFltVV& _Mat) {
		TJsFltVVUtil::GetSelf(Obj)->Mat = _Mat;
	}
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `val = mat.at(i,j)` -- Gets the element of `mat` (matrix). Input: row index `i` (integer), column index `j` (integer). Output: `val` (number). Uses zero-based indexing.
	JsDeclareFunction(at);	
	//#- `mat.put(i, j, val)` -- Sets the element of `mat` (matrix). Input: row index `i` (integer), column index `j` (integer), value `val` (number). Uses zero-based indexing.
	JsDeclareFunction(put);
	//#- `y = mat.multiply(x)` -- Matrix multiplication: if `x` is a number, then `y` is a matrix. If `x` is a vector (dense or sparse), then `y` is a dense vector. If `x` is a matrix (sparse or dense), then `y` is a dense matrix.
	JsDeclareFunction(multiply);
	//#- `y = mat.multiplyT(x)` -- the result is equivalent to mat.transpose().multiply(), supported inputs include a number (scalar), dense or sparse vector and dense or sparse matrix. The result is always dense.
	JsDeclareFunction(multiplyT);
	//#- `mat3 = mat1.plus(mat2)` -- `mat3` is the sum of matrices `mat1` and `mat2`
	JsDeclareFunction(plus);
	//#- `mat3 = mat1.minus(mat2)` -- `mat3` is the difference of matrices `mat1` and `mat2`
	JsDeclareFunction(minus);
	//#- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat1` transposed
	JsDeclareFunction(transpose);
	//#- `x = mat.solve(y)` -- vector `x` is the solution to the linear system `mat * x = y`
	JsDeclareFunction(solve);
	//#- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	JsDeclareFunction(rowNorms);
	//#- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	JsDeclareFunction(colNorms);
	//#- `mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation)
	JsDeclareFunction(normalizeCols);
	//#- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	JsDeclareFunction(sparse);
	//#- `x = mat.frob()` -- number `x` is the Frobenious norm of matrix `mat`
	JsDeclareFunction(frob);
	//#- `r = mat.rows` -- integer `r` corresponds to the number of rows of `mat`
	JsDeclareProperty(rows);
	//#- `c = mat.cols` -- integer `c` corresponds to the number of columns of `mat`
	JsDeclareProperty(cols);
	//#- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	JsDeclareFunction(printStr);
	//#- `mat.print()` -- print matrix `mat` to console
	JsDeclareFunction(print);
	//#- `i = mat.rowMaxIdx(j)`: get the index `i` of the maximum element in row `j` of dense matrix `mat`
	JsDeclareFunction(rowMaxIdx);
	//#- `i = mat.colMaxIdx(j)`: get the index `i` of the maximum element in column `j` of dense matrix `mat`
	JsDeclareFunction(colMaxIdx);
	//#- `vec = mat.getCol(i)` -- `vec` corresponds to the `i`-th column of dense matrix `mat`. `i` must be an integer.
	JsDeclareFunction(getCol);
	//#- `mat.setCol(i, vec)` -- Sets the column of a dense matrix `mat`. `i` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setCol);
	//#- `vec = mat.getRow(i)` -- `vec` corresponds to the `i`-th row of dense matrix `mat`. `i` must be an integer.
	JsDeclareFunction(getRow);
	//#- `mat.setRow(i, vec)` -- Sets the row of a dense matrix `mat`. `i` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setRow);
};

///////////////////////////////
// QMiner-Sparse-Vector
//# 
//# ### SpVector (sparse vector)
//# 
//# Sparse vector is an array of (int,double) pairs that represent column indices and values (TIntFltKdV is implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse vectors can be generated in the following ways:
//# 
//# ```JavaScript
//# var spVec = la.newSpVec(); //empty vector
//# // refer to la.newSpVec for alternative ways to generate sparse vectors
//# ```
//# 
class TJsSpV {
public:
	/// JS script context
	TWPt<TScript> Js;    
	TIntFltKdV Vec;
	int Dim;
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsSpV> TJsSpVUtil;    
	explicit TJsSpV(TWPt<TScript> _Js) : Js(_Js), Dim(-1) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		v8::Persistent<v8::Object> obj = TJsSpVUtil::New(new TJsSpV(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TIntFltKdV");
		obj->SetHiddenValue(key, value);		
		return  obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TIntFltKdV& _Vec) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsSpV::SetSpV(obj, _Vec);
		return  obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TIntFltKdV& _Vec, const int _Dim) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsSpV::SetSpV(obj, _Vec);
		TJsSpV::SetDim(obj, _Dim);
		return  obj;
	}
	static TIntFltKdV& GetSpV(const v8::Handle<v8::Object> Obj) {
		return TJsSpVUtil::GetSelf(Obj)->Vec;
	}
	static void SetSpV(const v8::Handle<v8::Object> Obj, const TIntFltKdV& _Vec) {
		TJsSpVUtil::GetSelf(Obj)->Vec = _Vec;
	}
	static void SetDim(const v8::Handle<v8::Object> Obj, const int& _Dim) {
		TJsSpVUtil::GetSelf(Obj)->Dim = _Dim;
	}
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `val = spVec.at(i)` -- Gets the element of a sparse vector `spVec`. Input: index `i` (integer). Output: value `val` (number). Uses 0-based indexing
	JsDeclareFunction(at);	
	//#- `spVec.put(i, val)` -- Set the element of a sparse vector `spVec`. Inputs: index `i` (integer), value `val` (number). Uses 0-based indexing
	JsDeclareFunction(put);		
	//#- `x = spVec.sum()` -- `x` is the sum of elements of `spVec`
	JsDeclareFunction(sum);	
	//#- `x = spVec.inner(y)` -- `x` is the inner product between `spVec` and vector (sparse or dense) `y`.
	JsDeclareFunction(inner);	
	//#- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `a` (number) and vector `spVec`
	JsDeclareFunction(multiply);
	//#- `spVec.normalize()` -- normalizes the vector spVec (inplace operation)
	JsDeclareFunction(normalize);
	//#- `n = spVec.nnz` -- gets the number of nonzero elements `n` of vector `spVec`
	JsDeclareProperty(nnz);	
	//#- `d = spVec.dim` -- gets the dimension `d` (-1 means that it is unknown)
	JsDeclareProperty(dim);	
	//#- `spVec.print()` -- prints the vector to console
	JsDeclareFunction(print);
	//#- `x = spVec.norm()` -- returns `x` - the norm of `spVec`
	JsDeclareFunction(norm);
	//#- `vec = spVec.full()` --  returns `y` - a dense vector representation of sparse vector `spVec`.
	JsDeclareFunction(full);
	//#- `valVec = spVec.valVec()` --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
	JsDeclareFunction(valVec);
	//#- `idxVec = spVec.idxVec()` --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
	JsDeclareFunction(idxVec);
};


///////////////////////////////
// QMiner-Sparse-Col-Matrix
//# 
//# ### SpMatrix (sparse column matrix)
//# 
//# SpMatrix is a sparse matrix represented as a dense vector of sparse vectors which correspond to matrix columns (TVec<TIntFltKdV>, implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse matrices are generated in several ways:
//# 
//# ```JavaScript
//# var spMat = la.newSpMat(); //empty matrix
//# // refer to la.newSpMat function for alternative ways to generate sparse matrices
//# ```
//# 
class TJsSpMat {
public:
	/// JS script context
	TWPt<TScript> Js;    
	// 
	TVec<TIntFltKdV> Mat;	
	TInt Rows;
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsSpMat> TJsSpMatUtil;    
	explicit TJsSpMat(TWPt<TScript> _Js) : Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 		
		v8::Persistent<v8::Object> obj = TJsSpMatUtil::New(new TJsSpMat(Js), Js, "la.spMat");
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TVec<TIntFltKdV>");
		obj->SetHiddenValue(key, value);
		SetRows(obj, -1);
		return  obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TVec<TIntFltKdV>& _Mat) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsSpMat::SetSpMat(obj, _Mat);		
		return  obj;
	}
	static TVec<TIntFltKdV>& GetSpMat(const v8::Handle<v8::Object> Obj) {
		return TJsSpMatUtil::GetSelf(Obj)->Mat;
	}
	static void SetSpMat(const v8::Handle<v8::Object> Obj, const TVec<TIntFltKdV>& _Mat) {
		TJsSpMatUtil::GetSelf(Obj)->Mat = _Mat;
	}
	static void SetRows(const v8::Handle<v8::Object> Obj, const int& _Rows) {
		TJsSpMatUtil::GetSelf(Obj)->Rows = _Rows;
	}
	static int GetRows(const v8::Handle<v8::Object> Obj) {
		return TJsSpMatUtil::GetSelf(Obj)->Rows;
	}
	static int GetCols(const v8::Handle<v8::Object> Obj) {
		return TJsSpMatUtil::GetSelf(Obj)->Mat.Len();
	}
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `val = spMat.at(i,j)` -- Gets the element of `spMat` (sparse matrix). Input: row index `i` (integer), column index `j` (integer). Output: `val` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//#- `spMat.put(i, j, val)` -- Sets the element of `spMat` (sparse matrix). Input: row index `i` (integer), column index `j` (integer), value `val` (number). Uses zero-based indexing.
	JsDeclareFunction(put);
	//#- `x = spMat[i]; spMat[i] = x` -- setting and getting sparse vectors `x` from sparse column matrix, given column index `i` (integer)
	JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `spMat.push(x)` -- attaches a column `x` (sparse vector) to `spMat` (sparse matrix)
	JsDeclareFunction(push);
	//#- `y = spMat.multiply(x)` -- Matrix multiplication: if `x` is a number, then `y` is a matrix. If `x` is a vector (dense or sparse), then `y` is a dense vector. If `x` is a matrix (sparse or dense), then `y` is a dense matrix.
	JsDeclareFunction(multiply);
	//#- `y = spMat.multiplyT(x)` -- the result is equivalent to mat.transpose().multiply(), supported inputs include a number (scalar), dense or sparse vector and dense or sparse matrix. The result is always dense.
	JsDeclareFunction(multiplyT);
	//#- `spMat3 = spMat1.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat1` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(plus);
	//#- `spMat3 = spMat1.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat1` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(minus);
	//#- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `mat1` (sparse matrix) transposed 
	JsDeclareFunction(transpose);	
	//#- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	JsDeclareFunction(colNorms);
	//#- `spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation)
	JsDeclareFunction(normalizeCols);
	//#- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	JsDeclareFunction(full);
	//#- `x = spMat.frob()` -- number `x` is the Frobenious norm of `spMat` (sparse matrix)
	JsDeclareFunction(frob);
	//#- `r = spMat.rows` -- integer `r` corresponds to the number of rows of `spMat` (sparse matrix)
	JsDeclareProperty(rows);
	//#- `c = spMat.cols` -- integer `c` corresponds to the number of columns of `spMat` (sparse matrix)
	JsDeclareProperty(cols);
	//#- `spMat.print()` -- print `spMat` (sparse matrix) to console
	JsDeclareFunction(print);
	//#- `spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`
	JsDeclareFunction(save);
	//#- `spMat.load(fin)` -- load `spMat` (sparse matrix) from input steam `fin`
	JsDeclareFunction(load);
	//#JSIMPLEMENT:src/qminer/spMat.js
};


///////////////////////////////
// QMiner-JavaScript-Machine-Learning
//#
//# ## Analytics.js (use require)
//#
//# Analytics algorithms for working with records stored in
//# QMiner and with linear algebra objects created by `la`.
//# 
//# To start using it must be loaded using `var analytics = require('analytics.js');`.
class TJsAnalytics {
public:
	/// JS script context
	TWPt<TScript> Js;	
    
private:
	typedef TJsObjUtil<TJsAnalytics> TJsAnalyticsUtil;
    
	TJsAnalytics(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
        return TJsAnalyticsUtil::New(new TJsAnalytics(Js)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `ftrSpace = analytics.newFeatureSpace(featureExtractors)` -- create new
    //#     feature space consisting of [Feature Extractor](Feature-Extractors),
    //#     declared in JSon `featureExtractors`
    JsDeclareFunction(newFeatureSpace);
	JsDeclareFunction(newFeatureSpace2);
    //#- `ftrSpace = analytics.loadFeatureSpace(fin)` -- load serialized feature 
    //#     space from `fin` stream
    JsDeclareFunction(loadFeatureSpace);
    
    //#- `model = analytics.trainSvmClassify(matrix, vector, parameters)` -- trains binary
    //#     classification model using columns from `matrix` as training data and
    //#     `vector` as target variable (must be of values either 1 or -1); optional
    //#     training `parameters` are a JSon with parameter `c` (SVM cost parameter,
    //#     default = 1.0) and `j` (factor to multiply SVM cost parameter for positive 
    //#     examples with (default is 1.0)); result is a linear model
	JsDeclareFunction(trainSvmClassify);
    //#- `model = analytics.trainSvmRegression(matrix, vector, parameters)` -- trains 
    //#     regression model using columns from `matrix` as training data and `vector` as 
    //#     target variable; optional training `parameters` are a JSon with parameter `c` 
    //#     (SVM cost parameter, default = 1.0) and `eps` (ignore threshold defining
    //#     epsilon size tunnel around the model, default is 1.0)); result is a linear model
    JsDeclareFunction(trainSvmRegression);
    //#- `model = analytics.loadSvmModel(fin)` -- load serialized linear model 
    //#     from `fin` stream
	JsDeclareFunction(loadSvmModel);
    
    //#- `model = analytics.newRecLinReg(parameters)` -- create new recursive linear regression
    //#     model; training `parameters` are `dim` (dimensionality of feature space, e.g.
    //#     `ftrSpace.dim`), `forgetFact` (forgetting factor, default is 1.0) and `regFact` 
    //#     (regularization parameter to avoid over-fitting, default is 1.0).)
    JsDeclareFunction(newRecLinReg);
    //#- `model = analytics.loadRecLinRegModel(fin)` -- load serialized linear model
	//#     from `fin` stream
	JsDeclareFunction(loadRecLinRegModel);

    //#- `model = analytics.newHoeffdingTree(jsonStream, jsonParams)` -- create new
    //#     incremental decision tree learner; parameters are passed as JSON
    JsDeclareFunction(newHoeffdingTree);    
    
    // clustering (TODO: still depends directly on feature space)
    // trainKMeans(featureSpace, positives, negatives, parameters)
	JsDeclareFunction(trainKMeans);
    
    //#- `options = analytics.getLanguageOptions()` -- get options for text parsing 
    //#     (stemmers, stop word lists) as a json object, with two arrays:
    //#     `options.stemmer` and `options.stopwords`
	JsDeclareFunction(getLanguageOptions);     
    //#JSIMPLEMENT:src/qminer/js/analytics.js
};

///////////////////////////////
// QMiner-JavaScript-Feature-Space
//#
//# ### Feature Space
//#
//# Holds the definition of the feature space and feature extractors, which
//# can create feature vectors from QMiner records. Feature space is created
//# by calling `analytics.newFeatureSpace` and providing [Feature Extractor](Feature-Extractors) 
//# declarations as parameters.
class TJsFtrSpace {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// Feature space 
    PFtrSpace FtrSpace;    
    
private:
	typedef TJsObjUtil<TJsFtrSpace> TJsFtrSpaceUtil;
    
	TJsFtrSpace(TWPt<TScript> _Js, const PFtrSpace& _FtrSpace): Js(_Js), FtrSpace(_FtrSpace) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PFtrSpace& FtrSpace) { 
		return TJsFtrSpaceUtil::New(new TJsFtrSpace(Js, FtrSpace)); }
    static PFtrSpace GetArgFtrSpace(const v8::Arguments& Args, const int& ArgN);

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `ftrSpace.dim` -- dimensionality of feature space
    JsDeclareProperty(dim);    
    //#- `ftrSpace.save(fout)` -- serialize feature space to `fout` output stream
    JsDeclareFunction(save);
    //#- `ftrSpace.updateRecord(record)` -- update feature space definitions and extractors
    //#     by exposing them to `record`. For example, this can update the vocabulary
    //#     used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecord);
    //#- `ftrSpace.updateRecord(recordSet)` -- update feature space definitions and extractors
    //#     by exposing them to records from `recordSet`. For example, this can update 
    //#     the vocabulary used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecords);
    JsDeclareFunction(finishUpdate); // deprecated
    //#- `strVec = ftrSpace.extractStrings(record)` -- use feature extractors to extract string 
    //#     features from `record` (e.g. words from string fields); results are returned
    //#     as a string array
    JsDeclareFunction(extractStrings);
	//#- `ftrName = ftrSpace.getFtr(ftrN)` -- returns the name `ftrName` (string) of `ftrN`-th feature in feature space `ftrSpace`
	JsDeclareFunction(getFtr);
	//#- `spVec = ftrSpace.ftrSpVec(record)` -- extracts sparse feature vector from `record`
    JsDeclareFunction(ftrSpVec);
    //#- `vec = ftrSpace.ftrVec(record)` -- extracts feature vector from `record`
    JsDeclareFunction(ftrVec);
    //#- `spMatrix = ftrSpace.ftrSpColMat(recordSet)` -- extracts sparse feature vectors from 
    //#     records in `recordSet` and returns them as columns in a sparse matrix.
	JsDeclareFunction(ftrSpColMat);
    //#- `matrix = ftrSpace.ftrColMat(recordSet)` -- extracts feature vectors from 
    //#     records in `recordSet` and returns them as columns in a matrix.
    JsDeclareFunction(ftrColMat);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
//#
//# ### Support Vector Machine model
//#
//# Holds SVM classification or regression model. This object is result of
//# `analytics.trainSvmClassify` or `analytics.trainSvmRegression`.
class TJsSvmModel {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// SVM Model
    PSVMModel Model;
    
private:
	typedef TJsObjUtil<TJsSvmModel> TJsSvmModelUtil;
    
	TJsSvmModel(TWPt<TScript> _Js, const PSVMModel& _Model): Js(_Js), Model(_Model) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PSVMModel& Model) { 
        return TJsSvmModelUtil::New(new TJsSvmModel(Js, Model)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `res = model.predict(vector)` -- sends `vector` (full or sparse) through the 
    //#     model and returns the prediction as a real number
	JsDeclareFunction(predict);
    //#- `vec = model.weights` -- weights of the SVM linear model as a full vector
	JsDeclareProperty(weights);   
    //#- `model.save(fout)` -- saves model to output stream `fout`
	JsDeclareFunction(save);
};

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression
//#
//# ### Recursive Linear Regression model
//#
//# Holds online regression model. This object is result of `analytics.newRecLinReg`.
class TJsRecLinRegModel {
public:
	/// JS script context
	TWPt<TScript> Js;	
	/// RecLinReg Model
	TSignalProc::PRecLinReg Model;    
private:
	typedef TJsObjUtil<TJsRecLinRegModel> TJsRecLinRegModelUtil;
	TJsRecLinRegModel(TWPt<TScript> _Js, const TSignalProc::PRecLinReg& _Model): Js(_Js), Model(_Model) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TSignalProc::PRecLinReg& Model) {
		return TJsRecLinRegModelUtil::New(new TJsRecLinRegModel(Js, Model)); }
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
    
	//# 
	//# **Functions and properties:**
	//#     
    //#- `model.learn(vector, target)` -- updates the model using full `vector` as
    //#     `target` number as training data
	JsDeclareFunction(learn);
    //#- `res = model.predict(vector)` -- sends `vector` (full or sparse) through the 
    //#     model and returns the prediction as a real number
	JsDeclareFunction(predict);
    //#- `vec = model.weights` -- weights of the linear model as a full vector    
	JsDeclareProperty(weights);
    //#- `model.dim` -- dimensionality of the feature space on which this model works
	JsDeclareProperty(dim);
	//#- `model.save(fout)` -- saves model to output stream `fout`
	JsDeclareFunction(save);
};

///////////////////////////////
// QMiner-JavaScript-HoeffdingTree
//#
//# ### Hoeffding Tree model
//#
//# First, we have to initialize the learner. 
//# We specify the order of attributes in a stream example, and describe each attribute.
//# For each attribute, we specifty its type and --- in case of discrete attributes --- enumerate
//# all possible values of the attribute. See titanicConfig below. 
//#
//# The HoeffdingTree algorithm comes with many parameters:
//#
//# - gracePeriod. Denotes ``recomputation period''; if gracePeriod=200, the algorithm
//#	    will recompute information gains (or Gini indices) every 200 examples. Recomputation
//#	    is the most expensive operation in the algorithm; we have to recompute gains at each
//#	    leaf of the tree. (If ConceptDriftP=true, in each node of the tree.)
//# - splitConfidence. The probability of making a mistake when splitting a leaf. Let A1 and A2
//#	    be attributes with the highest information gains G(A1) and G(A2). The algorithm
//#	    uses [Hoeffding inequality](http://en.wikipedia.org/wiki/Hoeffding's_inequality#General_case)
//#	    to ensure that the attribute with the highest estimate (estimate is computed form the sample
//#	    of the stream examples that are currently in the leaf) is truly the best (assuming the process
//#	    generating the data is stationary). So A1 is truly best with probability at least 1-splitConfidence.
//# - tieBreaking. If two attributes are equally good --- or almost equally good --- the algorithm will
//#	    will never split the leaf. We address this with tieBreaking parameter and consider two attributes
//#	    equally good whenever G(A1)-G(A2) <= tieBreaking, i.e., when they have similar gains. (Intuition: If
//#	    the attributes are equally good, we don't care on which one we split.)
//# - conceptDriftP. Denotes whether the algorithm adapts to potential changes in the data. If set to true,
//#	    we use a variant of [CVFDT learner](http://homes.cs.washington.edu/~pedrod/papers/kdd01b.pdf );
//#     if set to false, we use a variant of [VFDT learner](http://homes.cs.washington.edu/~pedrod/papers/kdd00.pdf).
//# - driftCheck. If DriftCheckP=true, the algorithm sets nodes into self-evaluation mode every driftCheck
//#	    examples and swaps the tree 
//# - windowSize. The algorithm keeps a sliding window of the last windowSize stream examples. It makes sure
//#	    the model reflects the concept represented by the examples from the sliding window. It needs to keep
//#	    the window in order to ``forget'' the example when it becomes too old. 
class TJsHoeffdingTree {
public:
	/// JS script context
	TWPt<TScript> Js;
	// HoeffdingTree, the learner 
	THoeffding::PHoeffdingTree HoeffdingTree;
private:
	typedef TJsObjUtil<TJsHoeffdingTree> TJsHoeffdingTreeUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

	TJsHoeffdingTree(TWPt<TScript> Js_, PJsonVal StreamConfig, PJsonVal JsonConfig)
		: Js(Js_), HoeffdingTree(THoeffding::THoeffdingTree::New(StreamConfig, JsonConfig)) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js_, PJsonVal StreamConfig, PJsonVal JsonConfig) { 
		return TJsHoeffdingTreeUtil::New(new TJsHoeffdingTree(Js_, StreamConfig, JsonConfig)); }
	
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
	//#- `ht.process(discreteV, numericV, label)` -- processes the stream example; `discreteV` is vector of discrete attribute values;
	//#   `numericV` is vector of numeric attribute values; `label` is class label of the example; returns nothing;
	//#- `ht.process(line)` -- processes the stream example; `line` is comma-separated string of attribute values (for example "a1,a2,c", where c is the class label); returns nothing;
	JsDeclareFunction(process);
	//#- `ht.classify(discreteV, numericV)` -- classifies the stream example; `discreteV` is vector of discrete attribute values; `numericV` is vector of numeric attribute values; returns the class label 
	//#- `ht.classify(line)` -- classifies the stream example; `line` is comma-separated string of attribute values; returns the class label 
	JsDeclareFunction(classify);
	//#- `ht.exportModel(outParams)` -- writes the current model into file `outParams.file` in format `outParams.type`;
	//#   here, `outParams = { file: filePath, type: exportType }` where `file` is the file path and `type` is the export type (currently only `DOT` or `XML` supported) 
	JsDeclareFunction(exportModel);
};

///////////////////////////////
// QMiner-JavaScript-GeoIP
class TJsGeoIp {
private:
	// this is singleton
	static bool InitP;
	static PGeoIpBs GeoIpBs;	
	static PGeoIpBs GetGeoIpBs();

private:
	typedef TJsObjUtil<TJsGeoIp> TJsGeoIpUtil;
	explicit TJsGeoIp() { }	
public:
	static v8::Persistent<v8::Object> New() { return TJsGeoIpUtil::New(new TJsGeoIp); }
	~TJsGeoIp() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// function
	JsDeclareFunction(country);				
	JsDeclareFunction(location);
};

///////////////////////////////
// QMiner-JavaScript-DMoz
class TJsDMoz {
private:
	// this is singleton
	static bool InitP;
	static PDMozCfy DMozCfy;	
	static const PDMozCfy& GetDMozCfy();

private:
	typedef TJsObjUtil<TJsDMoz> TJsDMozUtil;
	explicit TJsDMoz() { }	
public:
	static v8::Persistent<v8::Object> New() { return TJsDMozUtil::New(new TJsDMoz); }
	~TJsDMoz() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// function
	JsDeclareFunction(classify);				
};

//#
//# ## System and I/O
//#
//# ### Process
//# 
class TJsProcess {
public:
	/// JS script context
	TWPt<TScript> Js;

private:
	/// Object utility class
	typedef TJsObjUtil<TJsProcess> TJsProcessUtil;

	explicit TJsProcess(TWPt<TScript> _Js): Js(_Js) { }

public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		return TJsProcessUtil::New(new TJsProcess(Js)); }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //#
	//# **Functions and properties:**
	//#
	//#- `process.sleep(millis)` -- Halts execution for the given amount of milliseconds `millis`.
    JsDeclareFunction(sleep);

    //#JSIMPLEMENT:src/qminer/process.js
};


//#
//# ### Utilities.js (use require)
//# 
//#JSIMPLEMENT:src/qminer/js/utilities.js    

//#
//# ### Assert.js (use require)
//# 
//#JSIMPLEMENT:src/qminer/js/assert.js    

///////////////////////////////
// JavaScript Console
//#
//# ### Console
//# 
//# Writing and reading from console. Also very useful to create
//# "interactive breakpoints" using `console.start()`. All outputs
//# are automatically prefixed by current date and time.
class TJsConsole {
public:
	/// JS script context
	TWPt<TScript> Js;
    
private:
	/// Object utility class
	typedef TJsObjUtil<TJsConsole> TJsConsoleUtil;
    
    explicit TJsConsole(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
		return TJsConsoleUtil::New(new TJsConsole(Js)); }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `console.log(message)` -- writes `message` to standard output, using
    //#     prefix `[console]` to indicate the text came from console object;
    //#     `message` must be of type string
    //#- `console.log(prefix, message)` -- writes `message` to standard output, 
    //#     using provided prefix `[prefix]`; both `message` and `prefix` must
    //#     be of type string
	JsDeclareFunction(log);
    //#- `line = console.getln()` -- reads a line from command line and returns
    //#     it as string
	JsDeclareFunction(getln);
	//#- `console.print(str)` -- prints a string to standard output
	JsDeclareFunction(print);
    //#JSIMPLEMENT:src/qminer/console.js    
};

///////////////////////////////
// QMiner-JavaScript-Filesystem
//#
//# ### File system
//# 
class TJsFs {
public:
	// directories we're allowed to access 
	TVec<TJsFPath> AllowedFPathV;
	
private:
	typedef TJsObjUtil<TJsFs> TJsFsUtil;

	TJsFs(const TVec<TJsFPath>& AllowedDirV_): AllowedFPathV(AllowedDirV_) { }
public:
	static v8::Persistent<v8::Object> New(TScript* Js) { 
		return TJsFsUtil::New(new TJsFs(Js->AllowedFPathV)); }
	~TJsFs() { }
	
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

    /// Are we allowed to access given path
	bool CanAccess(const TStr& FPath);
    /// Are we allowed to access given path
	static bool CanAccess(const v8::Arguments& Args);

    //# 
	//# **Functions and properties:**
	//#     
    //#- `fin = fs.openRead(fileName)`
	JsDeclareFunction(openRead);
    //#- `fout = fs.openWrite(fileName)`
	JsDeclareFunction(openWrite);
    //#- `fout = fs.openAppend(fileName)`
	JsDeclareFunction(openAppend);
    //#- `fs.exists(fileName)`
	JsDeclareFunction(exists);
    //#- `fs.copy(fromFileName, toFileName)`
	JsDeclareFunction(copy);
    //#- `fs.move(fromFileName, toFileName)`
	JsDeclareFunction(move);
    //#- `fs.del(fileName)`
	JsDeclareFunction(del);
    //#- `fs.rename(fromFileName, toFileName)`
	JsDeclareFunction(rename);
    //#- `info = fs.fileInfo(fileName,)`
	JsDeclareFunction(fileInfo);
    //#- `fs.mkdir(dirName)`
	JsDeclareFunction(mkdir);
    //#- `fs.rmdir(dirName)`
	JsDeclareFunction(rmdir);
    //#- `list = fs.listFile(dirName, fileExtension)`
    //#- `list = fs.listFile(dirName, fileExtension, recursive)`
	JsDeclareFunction(listFile);
};

///////////////////////////////
// QMiner-JavaScript-FIn
//#
//# ### Input File Stream
//# 
class TJsFIn {
public:
	PSIn SIn;
private:
	typedef TJsObjUtil<TJsFIn> TJsFInUtil;
	TJsFIn(const TStr& FNm): SIn(TZipIn::NewIfZip(FNm)) { }
public:
	static v8::Persistent<v8::Object> New(const TStr& FNm) {
		return TJsFInUtil::New(new TJsFIn(FNm)); }
    static PSIn GetArgFIn(const v8::Arguments& Args, const int& ArgN);

   	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `char = fin.peekCh()`
	JsDeclareFunction(peekCh);
    //#- `char = fin.getCh()`
	JsDeclareFunction(getCh);
    //#- `line = fin.readLine()`
	JsDeclareFunction(readLine);
    //#- `fin.eof`
	JsDeclareProperty(eof);
    //#- `fin.length`
	JsDeclareProperty(length);
};

///////////////////////////////
// QMiner-JavaScript-FOut
//#
//# ### Output File Stream
//# 
class TJsFOut {
public:
	PSOut SOut;
private:
	typedef TJsObjUtil<TJsFOut> TJsFOutUtil;
	TJsFOut(const TStr& FilePath, const bool& AppendP): SOut(TFOut::New(FilePath, AppendP)) { }
	TJsFOut(const TStr& FilePath): SOut(TZipOut::NewIfZip(FilePath)) { }

public:
	static v8::Persistent<v8::Object> New(const TStr& FilePath, const bool& AppendP = false) { 
		return TJsFOutUtil::New(new TJsFOut(FilePath, AppendP)); }
    static PSOut GetArgFOut(const v8::Arguments& Args, const int& ArgN);
    
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `fout.write(data)`
	JsDeclareFunction(write);
    //#- `fout.writeLine(data)`
	JsDeclareFunction(writeLine);
    //#- `fout.flush()`
	JsDeclareFunction(flush);
    //#- `fout.close()`
  	JsDeclareFunction(close);
};

///////////////////////////////
// JavaScript Http
//#
//# ### HTTP
//# 
class TJsHttp {
public:
	/// JS script context
	TWPt<TScript> Js;
    
private:
	/// Object utility class
	typedef TJsObjUtil<TJsHttp> TJsHttpUtil;
    
    explicit TJsHttp(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
		return TJsHttpUtil::New(new TJsHttp(Js)); }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `http.get(url)`
    //#- `http.get(url, success_callback)`
    //#- `http.get(url, success_callback, error_callback)`
    //#- `http.getStr(url)`
    //#- `http.getStr(url, success_callback)`
    //#- `http.getStr(url, success_callback, error_callback)`
	JsDeclareFunction(get);
    //#- `http.post(url, mimeType, data)`
    //#- `http.post(url, mimeType, data, success_callback)`
    //#- `http.post(url, mimeType, data, success_callback, error_callback)`
    //#- `http.postStr(url)`
    //#- `http.postStr(url, mimeType, data, success_callback)`
    //#- `http.postStr(url, mimeType, data, success_callback, error_callback)`
	JsDeclareFunction(post);
    //#- `http.onRequest(path, verb, function (request, response) { ... })`
	JsDeclareFunction(onRequest);
    //#JSIMPLEMENT:src/qminer/http.js    
};

///////////////////////////////
// QMiner-JavaScript-Http-Response
//#
//# ### HTTP Response
//# 
class TJsHttpResp {
public:
    /// Callback to the webserver for sending response    
	TWebSrv* WebSrv;
    /// Request ID for reference when responding
    TUInt64 SockId;
    /// Response status code
    TInt StatusCode;
    /// Content Type
    TStr ContTypeStr;
    /// Content Body
    TMem BodyMem;
    /// True when finished and response already sent
    TBool DoneP;
    
private:
	typedef TJsObjUtil<TJsHttpResp> TJsHttpRespUtil;
    
public:
	TJsHttpResp(TWebSrv* _WebSrv, const uint64& _SockId): WebSrv(_WebSrv),
        SockId(_SockId), StatusCode(THttp::OkStatusCd), 
        ContTypeStr(THttp::AppJSonFldVal), DoneP(false) { }
	static v8::Persistent<v8::Object> _New(TWebSrv* WebSrv, const uint64& SockId) { 
		return TJsHttpRespUtil::New(new TJsHttpResp(WebSrv, SockId), false); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `resp.setStatusCode(statusCode)`
	JsDeclareFunction(setStatusCode);
    //#- `resp.setStatusCode(mimeType)`
    JsDeclareFunction(setContentType);
    //#- `resp.add(data)`
	JsDeclareFunction(add);	
    //#- `resp.close()`
	JsDeclareFunction(close);
    //#- `resp.send(data)`
    JsDeclareFunction(send);
};

///////////////////////////////
// QMiner-JavaScript-Time
//#
//# ### Date-Time
//#
//# Wrapper around GLib's TTm. Used as return for `DateTime` field type. 
//# New one can be created using `tm = require('time')`.
class TJsTm {
public:
    /// Date-time
    TTm Tm;
private:
	typedef TJsObjUtil<TJsTm> TJsTmUtil;
    
	TJsTm(const TTm& _Tm): Tm(_Tm) { }
public:
	static v8::Persistent<v8::Object> New() { 
		return TJsTmUtil::New(new TJsTm(TTm::GetCurUniTm())); }
	static v8::Persistent<v8::Object> New(const TTm& Tm) { 
		return TJsTmUtil::New(new TJsTm(Tm)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#
    //#- `tm.string` -- string representation of time (e.g. 2014-05-29T10:09:12)
    JsDeclareProperty(string);
    //#- `tm.dateString` -- string representation of date (e.g. 2014-05-29)
    JsDeclareProperty(dateString);
    //#- `tm.timestamp` -- unix timestamp representation of time (seconds since 1970)
    JsDeclareProperty(timestamp);
    //#- `tm.year`
    JsDeclareProperty(year);
    //#- `tm.month`
    JsDeclareProperty(month);
    //#- `tm.day`
    JsDeclareProperty(day);
    //#- `tm.dayOfWeek`
    JsDeclareProperty(dayOfWeek);
    //#- `tm.hour`
    JsDeclareProperty(hour);
    //#- `tm.minute`
    JsDeclareProperty(minute);
    //#- `tm.second`
    JsDeclareProperty(second);
    //#- `tm.milisecond`
    JsDeclareProperty(milisecond);
    //#- `tm.now` -- returns new time object representing current local time
    JsDeclareProperty(now);
    //#- `tm.nowUTC` -- returns new time object represented current UTC time
    JsDeclareProperty(nowUTC);    
    //#- `tm.add(val, unit)` -- adds `val` to the time; `unit` defines the unit 
    //#     of `val`, options are `second` (default), `minute`, `hour`, and `day`.
    JsDeclareFunction(add);
    //#- `tm.sub(val, unit)` -- subtracts `val` from the time; `unit` defines the 
    //#     unit, same as in `add`
    JsDeclareFunction(sub); 
    //#- `tm.toJSON()` -- returns json representation of time    
    JsDeclareFunction(toJSON);
    //#- `date = tm.parse(`2014-05-29T10:09:12`) -- parses string and returns it
    //#     as Date-Time object
	JsDeclareFunction(parse);
};
//#
//# ## Other libraries
//#
//#JSIMPLEMENT:src/qminer/js/twitter.js 


///////////////////////////////////////////////
/// Javscript Function Feature Extractor.
//-
//- ## Numeric Feature Extractor
//-
class TJsFuncFtrExt : public TFtrExt {
// Js wrapper API
public:
	/// JS script context
	TWPt<TScript> Js;
private:
	//typedef TJsObjUtil<TJsFuncFtrExt> TJsFuncFtrExtUtil;
	// private constructor
	TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal, const v8::Persistent<v8::Function>& _Fun) : Js(_Js), Fun(_Fun), TFtrExt(Js->Base, ParamVal) { }
public:
	/*static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PJsonVal& ParamVal, const v8::Persistent<v8::Function>& _Fun) {
		return TJsFuncFtrExtUtil::New(new TJsFuncFtrExt(Js, ParamVal, _Fun));
	}*/
	// public smart pointer
	static PFtrExt NewFtrExt(TWPt<TScript> Js, const PJsonVal& ParamVal, const v8::Persistent<v8::Function>& _Fun) {
		return new TJsFuncFtrExt(Js, ParamVal, _Fun);
	}
	/*static v8::Handle<v8::ObjectTemplate> GetTemplate() {
		v8::HandleScope HandleScope;
		static v8::Persistent<v8::ObjectTemplate> Template;
		if (Template.IsEmpty()) {
			v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
			TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
			TmpTemp->SetInternalFieldCount(1);
			Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
		}
		return Template;
	}*/
// Core functionality
private:
	// Core part
	v8::Persistent<v8::Function> Fun;
	double ExecuteFunc(const TRec& FtrRec) const {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Value> RecArg = TJsRec::New(Js, FtrRec);
		return Js->ExecuteFlt(Fun, RecArg);
	}
// Feature extractor API
private:
	TJsFuncFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal); // will fail
	TJsFuncFtrExt(const TWPt<TBase>& Base, TSIn& SIn); // will fail
public:
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal); // will fail
	static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn); // will fail
	void Save(TSOut& SOut) const;

	TStr GetNm() const { return "jsfunc"; }
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return "jsfunc"; }

	void Clr() { };
	bool Update(const TRec& Rec) { return false; }
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	// flat feature extraction
	void ExtractFltV(const TRec& FtrRec, TFltV& FltV) const;

	// feature extractor type name 
	static TStr GetType() { return "jsfunc"; }

	////- `val = ftrExt.extract(rec) -- extracts `val` (number) from `rec` (record) using internal JS function (input for constructing feature exractor)
	//JsDeclareFunction(extract);
};


}

#endif

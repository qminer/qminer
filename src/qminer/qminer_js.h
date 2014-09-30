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

#ifdef V8_DEBUG
    // include v8 debug headers
	#include <v8-debug.h>
#endif

namespace TQm {

// All comments starting with / / # (no spaces) are copied to JavaScript API documentation
// available on QMiner wiki page https://github.com/qminer/qminer/wiki/JavaScript
// every so often.
    
// code to generate table of contents:
//    cat jsdocfinal.txt | grep '##' | sed 's/###/\/\/#  -/' | sed 's/##/\/\/# -/' | sed 's/- \(.*\)/- [\1](\1)/'    
// required post-edit example: (#System and I/O) =>(system-and-io)
    
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
//# **Intellisense**: When using Visual Studio 2013 one can enable intellisense for the JavaScript API globally by navigating: 
//# Tools / Options / Text Editor / JavaScript / IntelliSense / References, and selecting "Implicit(Web)" reference group and adding qminer.intellisense.js and qminer.js (located in QMINER_HOME/src/qminer/)
//# 
//# JavaScript API requires [initialized work environment](Quick-Start).
//#
//# ## Table of contents
//#
//# - [Libraries](#libraries)
//# - [Core QMiner objects](#core-qminer-objects)
//#  - [QMiner](#qminer)
//#  - [Stream Aggregate](#stream-aggregate)
//#  - [Store](#store)
//#  - [Store iterator](#store-iterator)
//#  - [Record set](#record-set)
//#  - [Record](#record)
//#  - [Index key](#index-key)
//# - [Linear Algebra](#linear-algebra)
//#  - [Vector](#vector)
//#  - [Matrix (dense matrix)](#matrix-dense-matrix)
//#  - [SpVector (sparse vector)](#spvector-sparse-vector)
//#  - [SpMatrix (sparse column matrix)](#spmatrix-sparse-column-matrix)
//# - [analytics.js (use require)](#analyticsjs-use-require)
//#  - [Feature Space](#feature-space)
//#  - [Support Vector Machine model](#support-vector-machine-model)
//#  - [Neural network model](#neural-network-model)
//#  - [Recursive Linear Regression model](#recursive-linear-regression-model)
//#  - [Hoeffding Tree model](#hoeffding-tree-model)
//# - [System and I/O](#system-and-io)
//#  - [Process](#process)
//#  - [utilities.js (use require)](#utilitiesjs-use-require)
//#  - [assert.js (use require)](#assertjs-use-require)
//#  - [Console](#console)
//#  - [File system](#file-system)
//#  - [Input File Stream](#input-file-stream)
//#  - [Output File Stream](#output-file-stream)
//#  - [HTTP](#http)
//#  - [HTTP Response](#http-response)
//#  - [Date-Time](#date-time)
//# - [Other libraries](#other-libraries)
//#  - [twitter.js (use require)](#twitterjs-use-require)
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
//#- **analytics.js** -- main API for analytics techniques
//#- **utilities.js** -- useful JavaScript utilities, e.g., checking variable type
//#- **time** -- wrapper around user-friendly date-time object
//#- **assert.js** -- support for writing unit tests
//#- **twitter.js** -- support for processing tweets


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
		} catch (const PExcept& Except) { \
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
    
    /// TStrV -> v8 string array
    static v8::Handle<v8::Value> GetStrArr(const TStrV& StrV);
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
    v8::Handle<v8::Value> ExecuteV8(v8::Handle<v8::Function> Fun, v8::Handle<v8::Value>& Arg1, v8::Handle<v8::Value>& Arg2);
    /// Execute JavaScript callback in this script's context
    v8::Handle<v8::Value> ExecuteV8(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
	/// Execute JavaScript callback in this script's context, return double
	double ExecuteFlt(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg);
	/// Execute JavaScript callback in this script's context, return double
	int ExecuteInt(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg);
	/// Execute JavaScript callback in this script's context, write result to vector
	void ExecuteFltVec(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg, TFltV& Vec);
	/// Execute JavaScript callback in this script's context
    bool ExecuteBool(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Object>& Arg); 
	/// Execute JavaScript callback in this script's context
    bool ExecuteBool(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Object>& Arg1, 
        const v8::Handle<v8::Object>& Arg2);
	/// Execute JavaScript callback in this script's context
	TStr ExecuteStr(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
	/// Execute JavaScript callback in this script's context
	void Execute(v8::Handle<v8::Function> Fun, const TStr& Str);
	/// Execute JavaScript callback in this script's context given string argument
	TStr ExecuteStr(v8::Handle<v8::Function> Fun, const TStr& Str);
	/// Execute JavaScript callback in this script's context given int argument
	PJsonVal ExecuteJson(v8::Handle<v8::Function> Fun, const TInt& ArgInt);
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
	static v8::Persistent<v8::Object> New(TJsObj* JsObj, const v8::Handle<v8::ObjectTemplate>& Template, const bool& MakeWeakP = true) {
		v8::HandleScope HandleScope;
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Template->NewInstance());
		Object->SetInternalField(0, v8::External::New(JsObj));
		if (MakeWeakP) { Object.MakeWeak(NULL, &Clean); }
		TJsUtil::AddObj(GetTypeNm<TJsObj>(*JsObj));
		return Object;
	}
    
    /// Mark object destructible by GC
    static void MakeWeak(v8::Persistent<v8::Object>& Obj) {
        Obj.MakeWeak(NULL, &Clean);
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

	// checks if the class name of the underlying glib object matches the given string. the name is stored in an hidden variable "class"
	static bool IsClass(const v8::Handle<v8::Object> Obj, const TStr& ClassNm) {
		TStr ObjClassStr = GetClass(Obj);		
		return ObjClassStr == ClassNm;
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

	/// Extract Val as JSon object, and serialize it to TStr
	static TStr GetValJsonStr(const v8::Handle<v8::Value> Val) {
		v8::HandleScope HandleScope;
		QmAssertR(Val->IsObject(), "Val expected to be object");
		TStr JsonStr = TJsUtil::V8JsonToStr(Val);
		return JsonStr;
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

	/// Extract Val as JSon object, and transform it to PJsonVal
	static PJsonVal GetValJson(const v8::Handle<v8::Value> Val) {
		TStr JsonStr = GetValJsonStr(Val);
		PJsonVal JsonVal = TJsonVal::GetValFromStr(JsonStr);
		if (!JsonVal->IsDef()) { throw TQmExcept::New("Error parsing '" + JsonStr + "'."); }
		return JsonVal;
	}

	/// Extract argument ArgN property as json
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN, const TStr& Property) {
		v8::HandleScope HandleScope;
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("TJsObjUtil::GetArgJson : Missing argument %d", ArgN));
		QmAssertR(Args[ArgN]->IsObject(), TStr::Fmt("TJsObjUtil::GetArgJson : Argument %d must be an object", ArgN));
		QmAssertR(Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr())), TStr::Fmt("TJsObjUtil::GetArgJson : Argument %d must have property %s", ArgN, Property.CStr()));
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
	JsDeclareFunction(op); // soon to be deprecated, functionality moved to TJsRecSet

	//# 
	//# **Functions and properties:**
	//# 
    //#- `store = qm.store(storeName)` -- store with name `storeName`; `store = null` when no such store
	JsDeclareFunction(store);
    //#- `strArr = qm.getStoreList()` -- an array of strings listing all existing stores
	JsDeclareFunction(getStoreList);
    //#- `qm.createStore(storeDef)` -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
    //#- `qm.createStore(storeDef, storeSizeInMB)` -- create new store(s) based on given `storeDef` (Json) [definition](Store Definition)
	JsDeclareFunction(createStore);
    //#- `rs = qm.search(query)` -- execute `query` (Json) specified in [QMiner Query Language](Query Language) 
    //#   and returns a record set `rs` with results
	JsDeclareFunction(search);   
    //#- `qm.gc()` -- start garbage collection to remove records outside time windows
	JsDeclareFunction(gc);
	//#- `sa = qm.newStreamAggr(paramJSON)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate.
	//#- `sa = qm.newStreamAggr(paramJSON, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = qm.newStreamAggr(paramJSON, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates) object `sa`. The constructor parameters are stored in `paramJSON` object. `paramJSON` must contain field `type` which defines the type of the aggregate. Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//#- `sa = qm.newStreamAggr(funObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.
	//#- `sa = qm.newStreamAggr(funObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = qm.newStreamAggr(funObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The function object `funObj` defines the aggregate name and four callbacks: onAdd (takes record as input), onUpdate (takes record as input), onDelete (takes record as input) and saveJson (takes one numeric parameter - limit) callbacks. An example: `funObj = new function () {this.name = 'aggr1'; this.onAdd = function (rec) { }; this.onUpdate = function (rec) { }; this.onDelete = function (rec) { };  this.saveJson = function (limit) { return {}; } }`.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	//#- `sa = qm.newStreamAggr(ftrExtObj)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.
	//#- `sa = qm.newStreamAggr(ftrExtObj, storeName)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeName` is used to register the stream aggregate for events on the appropriate store.
	//#- `sa = qm.newStreamAggr(ftrExtObj, storeNameArr)` -- create a new [Stream Aggregate](Stream-Aggregates). The `ftrExtObj = {type : 'ftrext', name : 'aggr1', featureSpace: fsp }` object has three parameters: `type='ftrext'`,`name` (string) and feature space `featureSpace` whose value is a feature space object.  Second parameter `storeNameArr` is an array of store names, where the stream aggregate will be registered.
	JsDeclareFunction(newStreamAggr);
	//#- `sa = qm.getStreamAggr(saName)` -- gets the stream aggregate `sa` given name (string).
	JsDeclareFunction(getStreamAggr);
	//#- `strArr = qm.getStreamAggrNames()` -- gets the stream aggregate names of stream aggregates in the default stream aggregate base.
	JsDeclareFunction(getStreamAggrNames);
	//#JSIMPLEMENT:src/qminer/qminer.js    
};

//# 
//# ### Stream Aggregate
//# 
//# Stream aggregates are objects used for processing data streams - their main functionality includes four functions: onAdd, onUpdate, onDelte process a record, and saveJson which returns a JSON object that describes the aggregate's state.
class TJsSA {
public:
	/// JS script context
	TWPt<TScript> Js;
	/// QMiner base
	TWPt<TStreamAggr> SA;

private:
	/// Object utility class
	typedef TJsObjUtil<TJsSA> TJsSAUtil;

	TJsSA(TWPt<TScript> _Js, TWPt<TStreamAggr> _SA) : Js(_Js), SA(_SA) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TWPt<TStreamAggr> SA) {
		return TJsSAUtil::New(new TJsSA(Js, SA));
	}

	~TJsSA() { }

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//# 
	//#- `str = sa.name` -- returns the name (unique) of the stream aggregate
	JsDeclareProperty(name);
	//#- `sa = sa.onAdd(rec)` -- executes onAdd function given an input record `rec` and returns self
	JsDeclareFunction(onAdd);
	//#- `sa = sa.onUpdate(rec)` -- executes onUpdate function given an input record `rec` and returns self
	JsDeclareFunction(onUpdate);
	//#- `sa = sa.onDelete(rec)` -- executes onDelete function given an input record `rec` and returns self
	JsDeclareFunction(onDelete);
	//#- `objJSON = sa.saveJson(limit)` -- executes saveJson given an optional number parameter `limit`, whose meaning is specific to each type of stream aggregate
	JsDeclareFunction(saveJson);
	//#- `sa = sa.save(fout)` -- executes save function given output stream `fout` as input. returns self.
	JsDeclareFunction(save);
	//#- `sa = sa.load(fin)` -- executes load function given input stream `fin` as input. returns self.
	JsDeclareFunction(load);
	//#- `objJSON = sa.val` -- same as sa.saveJson(-1)
	JsDeclareProperty(val);
	// IInt
	//#- `num = sa.getInt()` -- returns a number if sa implements the interface IInt
	JsDeclareFunction(getInt);
	// IFlt
	//#- `num = sa.getFlt()` -- returns a number if sa implements the interface IFlt
	JsDeclareFunction(getFlt);
	// ITm
	//#- `num = sa.getTm()` -- returns a number if sa implements the interface ITm. The result is a windows timestamp (number of milliseconds since 1601)
	JsDeclareFunction(getTm);
	// IFltVec
	//#- `num = sa.getFltLen()` -- returns a number (internal vector length) if sa implements the interface IFltVec.
	JsDeclareFunction(getFltLen);
	//#- `num = sa.getFltAt(idx)` -- returns a number (element at index) if sa implements the interface IFltVec.
	JsDeclareFunction(getFltAt);
	//#- `vec = sa.getFltV()` -- returns a dense vector if sa implements the interface IFltVec.
	JsDeclareFunction(getFltV);
	// ITmVec
	//#- `num = sa.getTmLen()` -- returns a number (timestamp vector length) if sa implements the interface ITmVec.
	JsDeclareFunction(getTmLen);
	//#- `num = sa.getTmAt(idx)` -- returns a number (windows timestamp at index) if sa implements the interface ITmVec.
	JsDeclareFunction(getTmAt);
	//#- `vec = sa.getTmV()` -- returns a dense vector of windows timestamps if sa implements the interface ITmVec.
	JsDeclareFunction(getTmV);
	// IFltTmIO
	//#- `num = sa.getInFlt()` -- returns a number (input value arriving in the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getInFlt);
	//#- `num = sa.getInTm()` -- returns a number (windows timestamp arriving in the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getInTm);
	//#- `vec = sa.getOutFltV()` -- returns a dense vector (values leaving the buffer) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getOutFltV);
	//#- `vec = sa.getOutTmV()` -- returns a dense vector (windows timestamps leaving the bugger) if sa implements the interface IFltTmIO.
	JsDeclareFunction(getOutTmV);
	//#- `num = sa.getN()` -- returns a number of records in the input buffer if sa implements the interface IFltTmIO.
	JsDeclareFunction(getN);

};

///////////////////////////////
// JavaScript Stream Aggregator
class TJsStreamAggr :
	public TStreamAggr,
	public TStreamAggrOut::IInt,
	//public TStreamAggrOut::IFlt,	
	//public TStreamAggrOut::ITm,
	public TStreamAggrOut::IFltTmIO,
	public TStreamAggrOut::IFltVec,
	public TStreamAggrOut::ITmVec,
	public TStreamAggrOut::INmFlt,
	public TStreamAggrOut::INmInt,
	// combinations
	public TStreamAggrOut::IFltTm
	//public TStreamAggrOut::IFltVecTm
{
private:
	/// JS script context
	TWPt<TScript> Js;
	// callbacks
	v8::Persistent<v8::Function> OnAddFun;
	v8::Persistent<v8::Function> OnUpdateFun;
	v8::Persistent<v8::Function> OnDeleteFun;
	v8::Persistent<v8::Function> SaveJsonFun;

	v8::Persistent<v8::Function> GetIntFun;
	// IFlt 
	v8::Persistent<v8::Function> GetFltFun;
	// ITm 
	v8::Persistent<v8::Function> GetTmMSecsFun;
	// IFltTmIO 
	v8::Persistent<v8::Function> GetInFltFun;
	v8::Persistent<v8::Function> GetInTmMSecsFun;
	v8::Persistent<v8::Function> GetOutFltVFun;
	v8::Persistent<v8::Function> GetOutTmMSecsVFun;
	v8::Persistent<v8::Function> GetNFun;
	// IFltVec
	v8::Persistent<v8::Function> GetFltLenFun;
	v8::Persistent<v8::Function> GetFltAtFun;
	v8::Persistent<v8::Function> GetFltVFun;
	// ITmVec
	v8::Persistent<v8::Function> GetTmLenFun;
	v8::Persistent<v8::Function> GetTmAtFun;
	v8::Persistent<v8::Function> GetTmVFun;
	// INmFlt 
	v8::Persistent<v8::Function> IsNmFltFun;
	v8::Persistent<v8::Function> GetNmFltFun;
	v8::Persistent<v8::Function> GetNmFltVFun;
	// INmInt
	v8::Persistent<v8::Function> IsNmFun;
	v8::Persistent<v8::Function> GetNmIntFun;
	v8::Persistent<v8::Function> GetNmIntVFun;

public:
	TJsStreamAggr(TWPt<TScript> _Js, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal);
	static PStreamAggr New(TWPt<TScript> Js, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal) {
		return new TJsStreamAggr(Js, _AggrNm, TriggerVal);
	}

	void OnAddRec(const TRec& Rec);
	void OnUpdateRec(const TRec& Rec);
	void OnDeleteRec(const TRec& Rec);
	PJsonVal SaveJson(const int& Limit) const;

	// stream aggregator type name 
	static TStr GetType() { return "javaScript"; }
	TStr Type() const { return GetType(); }
	void _Save(TSOut& SOut) const;
	v8::Persistent<v8::Function> SaveFun;
	void _Load(TSIn& SIn);
	v8::Persistent<v8::Function> LoadFun;

	// IInt
	int GetInt() const;
	// IFlt 
	double GetFlt() const;
	// ITm 
	uint64 GetTmMSecs() const;
	// IFltTmIO 
	double GetInFlt() const;
	uint64 GetInTmMSecs() const;
	void GetOutFltV(TFltV& ValV) const;
	void GetOutTmMSecsV(TUInt64V& MSecsV) const;
	int GetN() const;
	// IFltVec
	int GetFltLen() const;
	double GetFlt(const TInt& ElN) const; // GetFltAtFun
	void GetFltV(TFltV& ValV) const;
	// ITmVec
	int GetTmLen() const;
	uint64 GetTm(const TInt& ElN) const; // GetTmAtFun
	void GetTmV(TUInt64V& TmMSecsV) const;
	// INmFlt 
	bool IsNmFlt(const TStr& Nm) const;
	double GetNmFlt(const TStr& Nm) const;
	void GetNmFltV(TStrFltPrV& NmFltV) const;
	// INmInt
	bool IsNm(const TStr& Nm) const;
	double GetNmInt(const TStr& Nm) const;
	void GetNmIntV(TStrIntPrV& NmIntV) const;
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
		return TJsStoreUtil::New(new TJsStore(Js, Store), Js, "qm.storeProto");
	}
	/*static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TWPt<TStore> Store) { 
		return TJsStoreUtil::New(new TJsStore(Js, Store)); }*/
	~TJsStore() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `str = store.name` -- name of the store
	JsDeclareProperty(name);
    //#- `bool = store.empty` -- `bool = true` when store is empty
	JsDeclareProperty(empty);
    //#- `len = store.length` -- number of records in the store
	JsDeclareProperty(length);
    //#- `rs = store.recs` -- create a record set containing all the records from the store
	JsDeclareProperty(recs);
    //#- `objArr = store.fields` -- array of all the field descriptor JSON objects
	JsDeclareProperty(fields);
    //#- `objArr = store.joins` -- array of all the join names
	JsDeclareProperty(joins);	
    //#- `objArr = store.keys` -- array of all the [index keys](#index-key) objects    
	JsDeclareProperty(keys);
    //#- `rec = store.first` -- first record from the store
    JsDeclareProperty(first);
    //#- `rec = store.last` -- last record from the store
    JsDeclareProperty(last);
    //#- `iter = store.forwardIter` -- returns iterator for iterating over the store from start to end
    JsDeclareProperty(forwardIter);
    //#- `iter = store.backwardIter` -- returns iterator for iterating over the store from end to start
    JsDeclareProperty(backwardIter);
    //#- `rec = store[recId]` -- get record with ID `recId`; 
    //#     returns `null` when no such record exists
	JsDeclIndexedProperty(indexId);
    //#- `rec = store.rec(recName)` -- get record named `recName`; 
    //#     returns `null` when no such record exists
	JsDeclareFunction(rec);
    //#- `recId = store.add(rec)` -- add record `rec` to the store and return its ID `recId`
	JsDeclareFunction(add);
    //#- `rec = store.newRec(recordJson)` -- creates new record `rec` by (JSON) value `recordJson` (not added to the store)
    JsDeclareFunction(newRec);
    //#- `rs = store.newRecSet(idVec)` -- creates new record set from an integer vector record IDs `idVec` (type la.newIntVec);
	JsDeclareFunction(newRecSet);
    //#- `rs = store.sample(sampleSize)` -- create a record set containing a random 
    //#     sample of `sampleSize` records
	JsDeclareFunction(sample);
    //#- `field = store.field(fieldName)` -- get details of field named `fieldName`
	JsDeclareFunction(field);
    //#- `key = store.key(keyName)` -- get [index key](#index-key) named `keyName`
	JsDeclareFunction(key);
    //#- `store.addTrigger(trigger)` -- add `trigger` to the store triggers. Trigger is a JS object with three properties `onAdd`, `onUpdate`, `onDelete` whose values are callbacks
	JsDeclareFunction(addTrigger);	
    //#- `sa = store.getStreamAggr(saName)` -- returns a stream aggregate `sa` whose name is `saName`
	JsDeclareFunction(getStreamAggr);
	//#- `strArr = store.getStreamAggrNames()` -- returns the names of all stream aggregators listening on the store as an array of strings `strArr`
	JsDeclareFunction(getStreamAggrNames);
	//#- `objJSON = store.toJSON()` -- returns the store as a JSON
	JsDeclareFunction(toJSON);
	//#JSIMPLEMENT:src/qminer/store.js

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
// JavaScript Store Iterator
//# 
//# ### Store iterator
//# 
class TJsStoreIter {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;
    PStoreIter Iter;

	typedef TJsObjUtil<TJsStoreIter> TJsStoreIterUtil;

	TJsStoreIter(TWPt<TScript> _Js, const TWPt<TStore>& _Store, 
        const PStoreIter& _Iter): Js(_Js), Store(_Store), Iter(_Iter) { }	
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, 
        const TWPt<TStore>& Store, const PStoreIter& Iter) { 
		return TJsStoreIterUtil::New(new TJsStoreIter(Js, Store, Iter)); }
	~TJsStoreIter() { }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#   
    //#- `store = iter.store` -- get the store
	JsDeclareProperty(store);
    //#- `rec = iter.rec` -- get current record
	JsDeclareProperty(rec);
    //#- `bool = iter.next()` -- moves to the next record or returns false if no record left; must be called at least once before `iter.rec` is available
    JsDeclareFunction(next);
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
// JavaScript Record Splitter
class TJsRecSplitter {
private:
	/// JS script context
	TWPt<TScript> Js;
	TWPt<TStore> Store;
	// callbacks
	v8::Persistent<v8::Function> SplitterFun;

public:
	TJsRecSplitter(TWPt<TScript> _Js, TWPt<TStore> _Store, 
        const v8::Persistent<v8::Function>& _SplitterFun): 
            Js(_Js), Store(_Store), SplitterFun(_SplitterFun) { }

    bool operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const;
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
	//#- `storeName = rs.store` -- store of the records
	JsDeclareProperty(store);
	//#- `len = rs.length` -- number of records in the set
	JsDeclareProperty(length);
	//#- `bool = rs.empty` -- `bool = true` when record set is empty
	JsDeclareProperty(empty);
	//#- `bool =  rs.weighted` -- `bool = true` when records in the set are assigned weights
	JsDeclareProperty(weighted);
    //#- `rec = rs[n]` -- return n-th record from the record set
	JsDeclIndexedProperty(indexId);
	//#- `rs2 = rs.clone()` -- creates new instance of record set
	JsDeclareFunction(clone);
	//#- `rs2 = rs.join(joinName)` -- executes a join `joinName` on the records in the set, result is another record set `rs2`.
	//#- `rs2 = rs.join(joinName, sampleSize)` -- executes a join `joinName` on a sample of `sampleSize` records in the set, result is another record set `rs2`.
	JsDeclareFunction(join);
	//#- `aggrsJSON = rs.aggr()` -- returns an object where keys are aggregate names and values are JSON serialized aggregate values of all the aggregates contained in the records set
	//#- `aggr = rs.aggr(aggrQueryJSON)` -- computes the aggregates based on the `aggrQueryJSON` parameter JSON object. If only one aggregate is involved and an array of JSON objects when more than one are returned.
	JsDeclareFunction(aggr);
	//#- `rs = rs.trunc(limit_num)` -- truncate to first `limit_num` record and return self.
	//#- `rs = rs.trunc(limit_num, offset_num)` -- truncate to `limit_num` record starting with `offset_num` and return self.
	JsDeclareFunction(trunc);
	//#- `rs2 = rs.sample(num)` -- create new record set by randomly sampling `num` records.
	JsDeclareFunction(sample);
	//#- `rs = rs.shuffle(seed)` -- shuffle order using random integer seed `seed`. Returns self.
	JsDeclareFunction(shuffle);
	//#- `rs = rs.reverse()` -- reverse record order. Returns self.
	JsDeclareFunction(reverse);
	//#- `rs = rs.sortById(asc)` -- sort records according to record id; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortById);
	//#- `rs = rs.sortByFq(asc)` -- sort records according to weight; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByFq);
	//#- `rs = rs.sortByField(fieldName, asc)` -- sort records according to value of field `fieldName`; if `asc > 0` sorted in ascending order. Returns self.
	JsDeclareFunction(sortByField);
	//#- `rs = rs.sort(comparatorCallback)` -- sort records according to `comparator` callback. Example: rs.sort(function(rec,rec2) {return rec.Val < rec2.Val;} ) sorts rs in ascending order (field Val is assumed to be a num). Returns self.
   	JsDeclareFunction(sort);
	//#- `rs = rs.filterById(minId, maxId)` -- keeps only records with ids between `minId` and `maxId`. Returns self.
	JsDeclareFunction(filterById);
	//#- `rs = rs.filterByFq(minFq, maxFq)` -- keeps only records with weight between `minFq` and `maxFq`. Returns self.
	JsDeclareFunction(filterByFq);
	//#- `rs = rs.filterByField(fieldName, minVal, maxVal)` -- keeps only records with numeric value of field `fieldName` between `minVal` and `maxVal`. Returns self.
	//#- `rs = rs.filterByField(fieldName, minTm, maxTm)` -- keeps only records with value of time field `fieldName` between `minVal` and `maxVal`. Returns self.
	//#- `rs = rs.filterByField(fieldName, str)` -- keeps only records with string value of field `fieldName` equal to `str`. Returns self.
	JsDeclareFunction(filterByField);
	//#- `rs = rs.filter(filterCallback)` -- keeps only records that pass `filterCallback` function. Returns self.
	JsDeclareFunction(filter);
	//#- `rsArr = rs.split(splitterCallback)` -- split records according to `splitter` callback. Example: rs.split(function(rec,rec2) {return (rec2.Val - rec2.Val) > 10;} ) splits rs in whenever the value of field Val increases for more then 10. Result is an array of record sets. 
   	JsDeclareFunction(split);
    //#- `rs = rs.deleteRecs(rs2)` -- delete from `rs` records that are also in `rs2`. Returns self.
	JsDeclareFunction(deleteRecs);
    //#- `objsJSON = rs.toJSON()` -- provide json version of record set, useful when calling JSON.stringify
	JsDeclareFunction(toJSON);
	//#- `rs = rs.each(callback)` -- iterates through the record set and executes the callback function `callback` on each element. Returns self. Examples:
	//#  - `rs.each(function (rec) { console.log(JSON.stringify(rec)); })`
	//#  - `rs.each(function (rec, idx) { console.log(JSON.stringify(rec) + ', ' + idx); })`
	JsDeclareFunction(each);
	//#- `arr = rs.map(callback)` -- iterates through the record set, applies callback function `callback` to each element and returns new array with the callback outputs. Examples:
	//#  - `arr = rs.map(function (rec) { return JSON.stringify(rec); })`
	//#  - `arr = rs.map(function (rec, idx) {  return JSON.stringify(rec) + ', ' + idx; })`
	JsDeclareFunction(map);
	//#- `rs3 = rs.setintersect(rs2)` -- returns the intersection (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setintersect);
	//#- `rs3 = rs.setunion(rs2)` -- returns the union (record set) `rs3` between two record sets `rs` and `rs2`, which should point to the same store.
	JsDeclareFunction(setunion);
	//#- `rs3 = rs.setdiff(rs2)` -- returns the set difference (record set) `rs3`=`rs`\`rs2`  between two record sets `rs` and `rs1`, which should point to the same store.
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
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TRec& Rec, 
        const int& Fq = 0, const bool& MakeWeakP = true) { 
    		return TJsRecUtil::New(new TJsRec(Js, Rec, Fq), 
                GetTemplate(Js->Base, Rec.GetStore()), MakeWeakP); }
    static TRec GetArgRec(const v8::Arguments& Args, const int& ArgN);

	~TJsRec() { }

	static v8::Handle<v8::ObjectTemplate> GetTemplate(const TWPt<TBase>& Base, const TWPt<TStore>& Store);

	//# 
	//# **Functions and properties:**
	//#   
    //#- `recId = rec.$id` -- returns record ID
    JsDeclareProperty(id);
    //#- `recName = rec.$name` -- returns record name
    JsDeclareProperty(name);
    //#- `recFq = rec.$fq` -- returns record frequency (used for randomized joins)
	JsDeclareProperty(fq);
	//#- `recStore = rec.$store` -- returns record store
	JsDeclareProperty(store);
    //#- `rec['fieldName'] = val` -- sets the record's field `fieldName` to `val`. Equivalent: `rec.fieldName = val`.
	//#- `val = rec['fieldName']` -- gets the value `val` at field `fieldName`. Equivalent: `val = rec.fieldName`.
	JsDeclareSetProperty(getField, setField);
    //#- `rs = rec['joinName']` -- gets the record set if `joinName` is an index join. Equivalent: `rs = rec.joinName`. No setter currently.
	//#- `rec2 = rec['joinName']` -- gets the record `rec2` is the join `joinName` is a field join. Equivalent: `rec2 = rec.joinName`. No setter currently.
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
    //#- `rec = rec.addJoin(joinName, joinRecord)` -- adds a join record `joinRecord` to join `jonName` (string). Returns self.
    //#- `rec = rec.addJoin(joinName, joinRecord, joinFrequency)` -- adds a join record `joinRecord` to join `jonName` (string) with join frequency `joinFrequency`. Returns self.
    JsDeclareFunction(addJoin);
    //#- `rec = rec.delJoin(joinName, joinRecord)` -- deletes join record `joinRecord` from join `joinName` (string). Returns self.
    //#- `rec = rec.delJoin(joinName, joinRecord, joinFrequency)` -- deletes join record `joinRecord` from join `joinName` (string) with join frequency `joinFrequency`. Return self.
    JsDeclareFunction(delJoin);
    //#- `objJSON = rec.toJSON()` -- provide json version of record, useful when calling JSON.stringify
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
    //#- `storeName = key.store` -- gets the store name `storeName`
	JsDeclareProperty(store);				
    //#- `keyName = key.name` -- gets the key name
	JsDeclareProperty(name);
    //#- `strArr = key.voc` -- gets the array of words (as strings) in the vocabulary
	JsDeclareProperty(voc);
    //#- `strArr = key.fq` -- gets the array of weights (as strings) in the vocabulary
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
	//#- `vec = la.newVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
	//#- `vec = la.newVec(arr)` -- copy a javascript number array `arr` 
	//#- `vec = la.newVec(vec2)` -- clone a float vector `vec2`
	JsDeclareFunction(newVec);
	//#- `intVec = la.newIntVec()` -- generate an empty integer vector
	//#- `intVec = la.newIntVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
	//#- `intVec = la.newIntVec(arr)` -- copy a javascript int array `arr` 
	//#- `intVec = la.newIntVec(vec2)` -- clone an int vector `vec2`
	JsDeclareFunction(newIntVec);
	//#- `mat = la.newMat()` -- generates a 0x0 matrix
	//#- `mat = la.newMat({"rows":num, "cols":num2, "random":bool})` -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
	//#- `mat = la.newMat(nestedArr)` -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
	//#- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`
	JsDeclareFunction(newMat);
	//#- `spVec = la.newSpVec(len)` -- creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
	//#- `spVec = la.newSpVec(nestedArr, len)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
	JsDeclareFunction(newSpVec);
	//#- `spMat = la.newSpMat()` -- creates an empty sparse matrix `spMat`
	//#- `spMat = la.newSpMat(rowIdxVec, colIdxVec, valVec)` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
	//#- `spMat = la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
	//#- `spMat = la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
	JsDeclareFunction(newSpMat);
	//#- `svdRes = la.svd(mat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
	//#- `svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
	JsDeclareFunction(svd);	
    //TODO: #- `intVec = la.loadIntVeC(fin)` -- load integer vector from input stream `fin`.
    //JsDeclareFunction(loadIntVec);
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
//# var vec = la.newVec(); //empty vector
//# var intVec = la.newIntVec(); //empty vector
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
	//#- `num = vec.at(idx)` -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
	//#- `num = intVec.at(idx)` -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
	JsDeclareFunction(at);
	//#- `vec2 = vec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	//#- `intVec2 = intVec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	JsDeclareFunction(subVec);
	//#- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
	JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `vec = vec.put(idx, num)` -- set value of vector `vec` at index `idx` to `num` (0-based indexing). Returns self.
	//#- `intVec = intVec.put(idx, num)` -- set value of integer vector `intVec` at index `idx` to `num` (0-based indexing). Returns self.
	JsDeclareFunction(put);	
	//#- `len = vec.push(num)` -- append value `num` to vector `vec`. Returns `len` - the length  of the modified array
	//#- `len = intVec.push(num)` -- append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array
	JsDeclareFunction(push);
	//#- `len = vec.unshift(num)` -- insert value `num` to the begining of vector `vec`. Returns the length of the modified array.
	//#- `len = intVec.unshift(num)` -- insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.
	JsDeclareFunction(unshift);
	//#- `len = vec.pushV(vec2)` -- append vector `vec2` to vector `vec`.
	//#- `len = intVec.pushV(intVec2)` -- append integer vector `intVec2` to integer vector `intVec`.
	JsDeclareTemplatedFunction(pushV);
	//#- `num = vec.sum()` -- return `num`: the sum of elements of vector `vec`
	//#- `num = intVec.sum()` -- return `num`: the sum of elements of integer vector `intVec`
	JsDeclareFunction(sum);
	//#- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in vector `vec`
	//#- `idx = intVec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in integer vector `vec`
	JsDeclareFunction(getMaxIdx);
	//#- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	//#- `intVec2 = intVec.sort(asc)` -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	JsDeclareFunction(sort);
	//#- `sortRes = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
	JsDeclareTemplatedFunction(sortPerm);	
	//#- `vec = vec.shuffle()` -- shuffels the vector `vec` (inplace operation). Returns self.
	JsDeclareFunction(shuffle);
	//#- `vec = vec.trunc(num)` -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
	JsDeclareFunction(trunc);
	//#- `mat = vec.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only. 
	JsDeclareTemplatedFunction(outer);
	//#- `num = vec.inner(vec2)` -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(inner);
	//#- `vec3 = vec.plus(vec2)` --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(plus);
	//#- `vec3 = vec.minus(vec2)` --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(minus);
	//#- `vec2 = vec.multiply(num)` --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(multiply);
	//#- `vec = vec.normalize()` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
	JsDeclareTemplatedFunction(normalize);
	//#- `len = vec.length` -- integer `len` is the length of vector `vec`
	//#- `len = intVec.length` -- integer `len` is the length of integer vector `vec`
	JsDeclareProperty(length);
	//#- `vec = vec.print()` -- print vector in console. Returns self.
	//#- `intVec = intVec.print()` -- print integer vector in console. Returns self.
	JsDeclareFunction(print);
	//#- `mat = vec.diag()` -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(diag);
	//#- `spMat = vec.spDiag()` -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(spDiag);
	//#- `num = vec.norm()` -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(norm);
	//#- `spVec = vec.sparse()` -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
	JsDeclareTemplatedFunction(sparse);
	//#- `mat = vec.toMat()` -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
	//#- `mat = intVec.toMat()` -- `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.
	JsDeclareTemplatedFunction(toMat);
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
		JsRegisterFunction(TmpTemp, subVec);
		JsRegGetSetIndexedProperty(TmpTemp, indexGet, indexSet);
		JsRegisterFunction(TmpTemp, put);
		JsRegisterFunction(TmpTemp, push);
		JsRegisterFunction(TmpTemp, unshift);
		JsRegisterFunction(TmpTemp, pushV);
		JsRegisterFunction(TmpTemp, sum);
		JsRegisterFunction(TmpTemp, getMaxIdx);
		JsRegisterFunction(TmpTemp, sort);
		JsRegisterFunction(TmpTemp, sortPerm);
		JsRegisterFunction(TmpTemp, shuffle);
		JsRegisterFunction(TmpTemp, trunc);
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
		JsRegisterFunction(TmpTemp, toMat);
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
v8::Handle<v8::Value> TJsVec<TVal, TAux>::subVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsVecUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsVecUtil::IsArgClass(Args, 0, "TIntV")) {
			TJsIntV* IdxV = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 0);
			int Len = IdxV->Vec.Len();
			TVec<TVal> Res(Len);
			for (int ElN = 0; ElN < Len; ElN++) {
				Res[ElN] = JsVec->Vec[IdxV->Vec[ElN]];
			}
			return TJsVec<TVal, TAux>::New(JsVec->Js, Res);
		}
		else if (Args[0]->IsArray()) {
			v8::Handle<v8::Value> V8IdxV = TJsLinAlg::newIntVec(Args);
			v8::Handle<v8::Object> V8IdxVObj = v8::Handle<v8::Object>::Cast(V8IdxV);
			v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(V8IdxVObj->GetInternalField(0));
			TJsIntV* IdxV = static_cast<TJsIntV*>(WrappedObject->Value());
			int Len = IdxV->Vec.Len();
			TVec<TVal> Res(Len);
			for (int ElN = 0; ElN < Len; ElN++) {
				Res[ElN] = JsVec->Vec[IdxV->Vec[ElN]];
			}
			return TJsVec<TVal, TAux>::New(JsVec->Js, Res);
		}		
	}
	return HandleScope.Close(v8::Undefined());
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
	return Args.Holder();
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
	bool Asc = TJsObjUtil<TJsVec>::GetArgBool(Args, 0 , true);
	TVec<TVal> Result = JsVec->Vec;
	Result.Sort(Asc);
	return TJsVec<TVal, TAux>::New(JsVec->Js, Result);
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::shuffle(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
    static TRnd Rnd; JsVec->Vec.Shuffle(Rnd);
	return Args.Holder();
}

template <class TVal, class TAux>
v8::Handle<v8::Value> TJsVec<TVal, TAux>::trunc(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
    const int NewLen = TJsObjUtil<TJsVec>::GetArgInt32(Args, 0);
    JsVec->Vec.Trunc(NewLen);
	return Args.Holder();
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
	return Args.Holder();
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
	//#- `num = mat.at(rowIdx,colIdx)` -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);	
	//#- `mat = mat.put(rowIdx, colIdx, num)` -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `mat2 = mat.multiply(num)` -- Matrix multiplication: `num` is a number, `mat2` is a matrix
	//#- `vec2 = mat.multiply(vec)` -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
	//#- `vec = mat.multiply(spVec)` -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
	//#- `mat3 = mat.multiply(mat2)` -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
	//#- `mat2 = mat.multiply(spMat)` -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
	JsDeclareFunction(multiply);
	//#- `mat2 = mat.multiplyT(num)` -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec2 = mat.multiplyT(vec)` -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec = mat.multiplyT(spVec)` -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat3 = mat.multiplyT(mat2)` -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat2 = mat.multiplyT(spMat)` -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	JsDeclareFunction(multiplyT);
	//#- `mat3 = mat.plus(mat2)` -- `mat3` is the sum of matrices `mat` and `mat2`
	JsDeclareFunction(plus);
	//#- `mat3 = mat.minus(mat2)` -- `mat3` is the difference of matrices `mat` and `mat2`
	JsDeclareFunction(minus);
	//#- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat` transposed
	JsDeclareFunction(transpose);
	//#- `vec2 = mat.solve(vec)` -- vector `vec2` is the solution to the linear system `mat * vec2 = vec`
	JsDeclareFunction(solve);
	//#- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	JsDeclareFunction(rowNorms);
	//#- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	JsDeclareFunction(colNorms);
	//#- `mat = mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	JsDeclareFunction(sparse);
	//#- `num = mat.frob()` -- number `num` is the Frobenious norm of matrix `mat`
	JsDeclareFunction(frob);
	//#- `num = mat.rows` -- integer `num` corresponds to the number of rows of `mat`
	JsDeclareProperty(rows);
	//#- `num = mat.cols` -- integer `num` corresponds to the number of columns of `mat`
	JsDeclareProperty(cols);
	//#- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	JsDeclareFunction(printStr);
	//#- `mat = mat.print()` -- print matrix `mat` to console. Returns self.
	JsDeclareFunction(print);
	//#- `colIdx = mat.rowMaxIdx(rowIdx)`: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
	JsDeclareFunction(rowMaxIdx);
	//#- `rowIdx = mat.colMaxIdx(colIdx)`: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
	JsDeclareFunction(colMaxIdx);
	//#- `vec = mat.getCol(colIdx)` -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
	JsDeclareFunction(getCol);
	//#- `mat = mat.setCol(colIdx, vec)` -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
	JsDeclareFunction(setCol);
	//#- `vec = mat.getRow(rowIdx)` -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
	JsDeclareFunction(getRow);
	//#- `mat.setRow(rowIdx, vec)` -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setRow);
	//#- `vec = mat.diag()` -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
	JsDeclareFunction(diag);
	//#- `mat = mat.save(fout)` -- print `mat` (full matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//#- `mat = mat.load(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(load);
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
	//#- `num = spVec.at(idx)` -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
	JsDeclareFunction(at);	
	//#- `spVec = spVec.put(idx, num)` -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
	JsDeclareFunction(put);		
	//#- `num = spVec.sum()` -- `num` is the sum of elements of `spVec`
	JsDeclareFunction(sum);	
	//#- `num = spVec.inner(vec)` -- `num` is the inner product between `spVec` and dense vector `vec`.
	//#- `num = spVec.inner(spVec)` -- `num` is the inner product between `spVec` and sparse vector `spVec`.
	JsDeclareFunction(inner);	
	//#- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
	JsDeclareFunction(multiply);
	//#- `spVec = spVec.normalize()` -- normalizes the vector spVec (inplace operation). Returns self.
	JsDeclareFunction(normalize);
	//#- `num = spVec.nnz` -- gets the number of nonzero elements `num` of vector `spVec`
	JsDeclareProperty(nnz);	
	//#- `num = spVec.dim` -- gets the dimension `num` (-1 means that it is unknown)
	JsDeclareProperty(dim);	
	//#- `spVec = spVec.print()` -- prints the vector to console. Return self.
	JsDeclareFunction(print);
	//#- `num = spVec.norm()` -- returns `num` - the norm of `spVec`
	JsDeclareFunction(norm);
	//#- `vec = spVec.full()` --  returns `vec` - a dense vector representation of sparse vector `spVec`.
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
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TVec<TIntFltKdV>& _Mat, const TInt& _Rows) {
		v8::Persistent<v8::Object> obj = New(Js);
		TJsSpMat::SetSpMat(obj, _Mat);
		TJsSpMat::SetRows(obj, _Rows);
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
	//#- `num = spMat.at(rowIdx,colIdx)` -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//#- `spMat = spMat.put(rowIdx, colIdx, num)` -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `spMat = spMat.push(spVec)` -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
	JsDeclareFunction(push);
	//#- `spMat2 = spMat.multiply(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
	//#- `vec2 = spMat.multiply(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
	//#- `vec = spMat.multiply(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
	//#- `mat2 = spMat.multiply(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
	//#- `mat = spMat.multiply(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
	JsDeclareFunction(multiply);
	//#- `spMat2 = spMat.multiplyT(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec2 = spMat.multiplyT(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec = spMat.multiplyT(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat2 = spMat.multiplyT(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat = spMat.multiplyT(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
	JsDeclareFunction(multiplyT);
	//#- `spMat3 = spMat.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(plus);
	//#- `spMat3 = spMat.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(minus);
	//#- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed 
	JsDeclareFunction(transpose);	
	//#- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	JsDeclareFunction(colNorms);
	//#- `spMat = spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	JsDeclareFunction(full);
	//#- `num = spMat.frob()` -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
	JsDeclareFunction(frob);
	//#- `num = spMat.rows` -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
	JsDeclareProperty(rows);
	//#- `num = spMat.cols` -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
	JsDeclareProperty(cols);
	//#- `spMat = spMat.print()` -- print `spMat` (sparse matrix) to console. Returns self.
	JsDeclareFunction(print);
	//#- `spMat = spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//#- `spMat = spMat.load(fin)` -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
	JsDeclareFunction(load);
	//#JSIMPLEMENT:src/qminer/spMat.js
};


///////////////////////////////
// QMiner-JavaScript-Machine-Learning
//#
//# ## analytics.js (use require)
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
    //#- `fsp = analytics.newFeatureSpace(featureExtractors)` -- create new
    //#     feature space consisting of [Feature Extractor](Feature-Extractors),
    //#     declared in JSon `featureExtractors`
    JsDeclareFunction(newFeatureSpace);
    //#- `fsp = analytics.loadFeatureSpace(fin)` -- load serialized feature 
    //#     space from `fin` stream
    JsDeclareFunction(loadFeatureSpace);    
    //#- `svmModel = analytics.trainSvmClassify(mat, vec, svmParameters)` -- trains binary
    //#     classification model using stochastic subgradient descent, where the columns from `mat` represent training feature vectors and vector
    //#     `vec` represents the training targets (must be of values either 1 or -1); optional
    //#     training parameters with their default values are a JSON object: `svmParameters = {c: 1.0, j: 1.0, batchSize: 10000, maxIterations: 10000, maxTime: 600, minDiff: 1e-6, verbose: false}`. 
    //#     The parameter `c` is the SVM cost parameter, `j` (factor to multiply SVM cost parameter for positive examples with (default is 1.0)), `batchSize` controls the sample size for stochastic subgradient calculations, `maxIterations` limits the number of subgradient steps, `maxTime` limits the runtime in seconds, `minDiff` is a tolerance that is used as a stopping condition, `verbose` controls verbosity of the algorithm; result is a linear model
	JsDeclareFunction(trainSvmClassify);
    //#- `svmModel = analytics.trainSvmRegression(mat, vec, svmRegParameters)` -- trains 
    //#     regression model using stochastic subgradient descent, where the columns from `mat` represent training feature vectors and vector `vec` represents the training targets;
    //#     optional training parameters with their default values are a JSON object: `svmRegParameters = {c : 1.0, eps : 1.0, batchSize : 10000, maxIterations : 10000, maxTime : 600, minDiff : 1e-6, varbose : false}`.
	//#     The parameter `c` is the SVM cost parameter, `eps` controls the epsilon-insensitive L1 loss, `batchSize` controls the sample size for stochastic subgradient calculations, `maxIterations` limits the number of subgradient steps, `maxTime` limits the runtime in seconds, `minDiff` is a tolerance that is used as a stopping condition, `verbose` controls verbosity of the algorithm; result is a linear model.
    JsDeclareFunction(trainSvmRegression);
    //#- `svmModel = analytics.loadSvmModel(fin)` -- load serialized linear model 
    //#     from `fin` stream
	JsDeclareFunction(loadSvmModel);    
    //#- `nnModel = analytics.newNN(nnParameters)` -- create new neural network
    //#     model; constructing `nnParameters` are a JSON object with properties: `nnParameters.layout` (javascript array of integers, where every integer represents number of neurons in a layer
    //#     ), `nnParameters.learnRate` (number learn rate, default is 0.1), `nnParameters.momentum` (number momentum, default is 0.1),
    //#     `nnParameters.tFuncHidden` (a string representing transfer function in hidden layers) and `nnParameters.tFuncOut` (a string representing transfer function in the output layer). 
	//#     The following strings correspond to transfer functions: `"tanHyper"`,`"sigmoid"`,`"fastTanh"`,`"fastSigmoid"`,`"linear"`.
    JsDeclareFunction(newNN);
    //#- `recLinRegModel = analytics.newRecLinReg(recLinRegParameters)` -- create new recursive linear regression
    //#     model; training `recLinRegParameters` are `recLinRegParameters.dim` (dimensionality of feature space, e.g.
    //#     `ftrSpace.dim`), `recLinRegParameters.forgetFact` (forgetting factor, default is 1.0) and `recLinRegParameters.regFact` 
    //#     (regularization parameter to avoid over-fitting, default is 1.0).)
    JsDeclareFunction(newRecLinReg);
    //#- `recLinRegModel = analytics.loadRecLinRegModel(fin)` -- load serialized linear model
	//#     from `fin` stream
	JsDeclareFunction(loadRecLinRegModel);

    //#- `htModel = analytics.newHoeffdingTree(jsonStream, htJsonParams)` -- create new
    //#     incremental decision tree learner; parameters `htJsonParams` are passed as JSON
    JsDeclareFunction(newHoeffdingTree);    
    
    // clustering (TODO: still depends directly on feature space)
    // trainKMeans(featureSpace, positives, negatives, parameters)
	JsDeclareFunction(trainKMeans);
    
    //#- `tokenizer = analytics.newTokenizer({ type: <type>, ...})` -- create new tokenizer
    //#     of type `<type>`. Syntax same as when defining index keys in stores or `text` feature 
    //#     extractors.
    JsDeclareFunction(newTokenizer);
    //#- `langOptionsJson = analytics.getLanguageOptions()` -- get options for text parsing 
    //#     (stemmers, stop word lists) as a json object, with two arrays:
    //#     `langOptionsJson.stemmer` and `langOptionsJson.stopwords`
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
	static PFtrSpace GetArgFtrSpace(v8::Handle<v8::Value> Val);
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `num = fsp.dim` -- dimensionality of feature space
    JsDeclareProperty(dim);    
    //#- `fsp = fsp.save(fout)` -- serialize feature space to `fout` output stream. Returns self.
    JsDeclareFunction(save);
    //#- `fsp = fsp.updateRecord(rec)` -- update feature space definitions and extractors
    //#     by exposing them to record `rec`. Returns self. For example, this can update the vocabulary
    //#     used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecord);
    //#- `fsp = fsp.updateRecords(rs)` -- update feature space definitions and extractors
    //#     by exposing them to records from record set `rs`. Returns self. For example, this can update 
    //#     the vocabulary used by bag-of-words extractor by taking into account new text.
	JsDeclareFunction(updateRecords);
    JsDeclareFunction(finishUpdate); // deprecated
    //#- `strArr = fsp.extractStrings(rec)` -- use feature extractors to extract string 
    //#     features from record `rec` (e.g. words from string fields); results are returned
    //#     as a string array
    JsDeclareFunction(extractStrings);
	//#- `ftrName = fsp.getFtr(idx)` -- returns the name `ftrName` (string) of `idx`-th feature in feature space `fsp`
	JsDeclareFunction(getFtr);
	//#- `spVec = fsp.ftrSpVec(rec)` -- extracts sparse feature vector `spVec` from record `rec`
    JsDeclareFunction(ftrSpVec);
    //#- `vec = fsp.ftrVec(rec)` -- extracts feature vector `vec` from record  `rec`
    JsDeclareFunction(ftrVec);
    //#- `spMat = fsp.ftrSpColMat(rs)` -- extracts sparse feature vectors from 
    //#     record set `rs` and returns them as columns in a sparse matrix `spMat`.
	JsDeclareFunction(ftrSpColMat);
    //#- `mat = fsp.ftrColMat(rs)` -- extracts feature vectors from 
    //#     record set `rs` and returns them as columns in a matrix `mat`.
    JsDeclareFunction(ftrColMat);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
//#
//# ### Support Vector Machine model
//#
//# Holds SVM classification or regression model. This object is result of
//# `analytics.trainSvmClassify` or `analytics.trainSvmRegression`.
// TODO rewrite to JavaScript
class TJsSvmModel {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// SVM Model
    TSvm::TLinModel Model;
    
private:
	typedef TJsObjUtil<TJsSvmModel> TJsSvmModelUtil;
    
	TJsSvmModel(TWPt<TScript> _Js, const TSvm::TLinModel& _Model): 
        Js(_Js), Model(_Model) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TSvm::TLinModel& Model) { 
        return TJsSvmModelUtil::New(new TJsSvmModel(Js, Model)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
    //#- `num = svmModel.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	//#- `num = svmModel.predict(spVec)` -- sends sparse vector `spVec` through the model and returns the prediction as a real number `num` (-1 or 1 for classification)
	JsDeclareFunction(predict);
    //#- `vec = svmModel.weights` -- weights of the SVM linear model as a full vector `vec`
	JsDeclareProperty(weights);   
    //#- `svmModel = svmModel.save(fout)` -- saves model to output stream `fout`. Returns self.
	JsDeclareFunction(save);
};

///////////////////////////////
// QMiner-JavaScript-Neural-Networks
//#
//# ### Neural network model
//#
//# Holds the neural network model. This object is result of `analytics.newNN`.
class TJsNN {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// NN Model
    TSignalProc::PNNet NN;

private:
    typedef TJsObjUtil<TJsNN> TJsNNUtil;
	TJsNN(TWPt<TScript> _Js, const TSignalProc::PNNet& _NN): Js(_Js), NN(_NN) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TSignalProc::PNNet& NN) {
		return TJsNNUtil::New(new TJsNN(Js, NN)); }
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
    
	//# 
	//# **Functions and properties:**
	//#     
	//#- `nnModel = nnModel.learn(inVec, outVec)` -- uses a pair of input `inVec` and output `outVec` to perform one step of learning with backpropagation. Returns self.
	JsDeclareFunction(learn);
    //#- `vec2 = nnModel.predict(vec)` -- sends vector `vec` through the model and returns the prediction as a vector `vec2`
	JsDeclareFunction(predict);
    //#- `nnModel.setLearnRate(float)` -- sets learning rate of the network
	JsDeclareFunction(setLearnRate);
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
    //#- `recLinRegModel = recLinRegModel.learn(vec, num)` -- updates the model using full vector `vec` and target number `num`as training data. Returns self.
	JsDeclareFunction(learn);
    //#- `num = recLinRegModel.predict(vec)` -- sends vector `vec` through the 
    //#     model and returns the prediction as a real number `num`
	JsDeclareFunction(predict);
    //#- `vec = recLinRegModel.weights` -- weights of the linear model as a full vector `vec`   
	JsDeclareProperty(weights);
    //#- `num = recLinRegModel.dim` -- dimensionality of the feature space on which this model works
	JsDeclareProperty(dim);
	//#- `recLinRegModel = recLinRegModel.save(fout)` -- saves model to output stream `fout`. Returns self.
	JsDeclareFunction(save);
};

///////////////////////////////
// QMiner-JavaScript-HoeffdingTree
//#
//# ### Hoeffding Tree model
//#
//# First, we have to initialize the learner by passing description of the data stream
//# and algorithm parameters. When describing the data stream, we have to specify the order of
//# attributes in a stream example and describe each attribute. For each attribute, we specifty
//# its type and --- in case of discrete attributes --- enumerate all possible values of the attribute.
//#
//# The HoeffdingTree algorithm comes with many parameters:
//#
//#- `gracePeriod` -- Denotes ``recomputation period''; if `gracePeriod=200`, the algorithm
//#	    will recompute information gains (or Gini indices) every `200` examples. Recomputation
//#	    is the most expensive operation in the algorithm, because we have to recompute gains at each
//#	    leaf of the tree for each attribute. (If `ConceptDriftP=true`, we have to recompute gains in each
//#       node of the tree.)
//#- `splitConfidence` -- The probability of making a mistake when splitting a leaf. Let `A1` and `A2`
//#	    be attributes with the highest information gains `G(A1)` and `G(A2)`. The algorithm
//#	    uses [Hoeffding inequality](http://en.wikipedia.org/wiki/Hoeffding's_inequality#General_case)
//#	    to ensure that the attribute with the highest estimate (estimate is computed form the sample
//#	    of the stream examples that are currently in the leaf) is truly the best (assuming the process
//#	    generating the data is stationary). So `A1` is truly best with probability at least `1-splitConfidence`.
//#- `tieBreaking` -- If two attributes are equally good --- or almost equally good --- the algorithm will
//#	    will never split the leaf. We address this with the `tieBreaking` parameter and consider two attributes
//#	    equally good whenever `G(A1)-G(A2) <= tieBreaking`, i.e., when they have similar gains. (Intuition: If
//#	    the attributes are equally good, we don't care on which one we split.)
//#- `conceptDriftP` -- Denotes whether the algorithm adapts to potential changes in the data. If set to `true`,
//#	    we use a variant of [CVFDT learner](http://homes.cs.washington.edu/~pedrod/papers/kdd01b.pdf );
//#      if set to `false`, we use a variant of [VFDT learner](http://homes.cs.washington.edu/~pedrod/papers/kdd00.pdf).
//#- `driftCheck` -- If `DriftCheckP=N` (this is one of the algorithm parameters), the algorithm sets nodes into
//#       self-evaluation mode every `N` examples. If one of the alternate trees performs better than the ``main''
//#       tree, the algorithm swaps the best-performing alternate tree in place of the main one. 
//#- `windowSize` -- The algorithm keeps a sliding window of the last `windowSize` stream examples. It makes sure
//#	    the model reflects the concept represented by the examples from the sliding window. It needs to keep
//#	    the window in order to ``forget'' the example when it becomes too old. 
//#- `maxNodes` -- The algorithm stops growing the tree if the tree contains more than (or equal to) `maxNodes` nodes.
//#       Alternate tree contributions are also counted. This parameter must be nonnegative integer. NOTE: If `maxNodes=0`,
//#       then the algorithm let's the tree grow arbitrarily.
//#- `attrDiscretization` -- Attribute discretization technique. Possible values are `histogram` and `bst`. See
//#       [this paper](http://dl.acm.org/citation.cfm?id=1786604) and [this paper](http://kt.ijs.si/elena_ikonomovska/DAMI10.pdf)
//#       for description. Note that `histogram` is cheap and uses up constant amount of memory, independent of the stream, but
//#       its inicialization depends on the order of stream elements. The `bst` is essentially batch technique and uses up
//#       lots of memory. 
//#- `clsAttrHeuristic` -- This tells the algorithm what attribute heurstic measure to use. Possible values are `infoGain`, for
//#       [information gain](http://en.wikipedia.org/wiki/Information_gain_in_decision_trees), and `giniGain`, for
//#       [gini index](http://en.wikipedia.org/wiki/Decision_tree_learning#Gini_impurity). For classification only; regression
//#       trees only support [standard deviation reduction](http://kt.ijs.si/elena_ikonomovska/DAMI10.pdf).
//#- `clsLeafModel` -- This tells the algorithm what model to fit in the leaves of the classification tree. Options are `majority`,
//#       which means majority classifier (predict the most frequent label in the leaf) and `naiveBayes`, which means
//#       [Naive Bayes classifier](http://en.wikipedia.org/wiki/Naive_Bayes_classifier).
//#- `regLeafModel` -- This tells the algorithm what model to fit in the leaves of the regression tree. Options are `mean`, which
//#       means the algorithm predicts the average value of the examples in the leaf, and `linear`, which means the algorithm fits
//#       [perceptron](http://en.wikipedia.org/wiki/Perceptron). (See [this paper](http://kt.ijs.si/elena_ikonomovska/DAMI10.pdf)
//#       for details.)
//#- `sdrTreshold` -- Stopping criterion for regression tree learner. The algorithm will not split the leaf unless the standard
//#       deviation reduction `sdr(A)` for the best attribute `A` is at least `sdtrTreshold`. Make sure that `sdrTreshold >= 0`.
//#- `phAlpha` -- Correction parameter for the [Page-Hinkley test](http://kt.ijs.si/elena_ikonomovska/00-disertation.pdf):
//#       It is the minimal absolute amplitude of change that we wish to detect. Should be adjusted according to the expected
//#       standard deviation of the signal. (See Elena Ikonomovska's [thesis](http://kt.ijs.si/elena_ikonomovska/00-disertation.pdf)
//#       for details.) Default is `phAlpha=0.005`.
//#- `phLambda` -- This is the threshold that corresponds to the admissible false alarm rate. Default is `phLambda=50.0`.
//#- `phInit` -- This threshold tells the algorithm when to start using Page-Hinkley test. The idea is to wait for `phInit` examples
//#       to accumulate in the nodes, so we have "more stable" estimates of mean. Default is `phInit=100`.
//#

class TJsHoeffdingTree {
public:
	/// JS script context
	TWPt<TScript> Js;
	// HoeffdingTree, the learner 
	THoeffding::PHoeffdingTree HoeffdingTree;
private:
	typedef TJsObjUtil<TJsHoeffdingTree> TJsHoeffdingTreeUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
	
	THoeffding::TTaskType JsTaskType;
	
	TJsHoeffdingTree(TWPt<TScript> Js_, PJsonVal StreamConfig, PJsonVal JsonConfig);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js_, PJsonVal StreamConfig, PJsonVal JsonConfig) { 
		return TJsHoeffdingTreeUtil::New(new TJsHoeffdingTree(Js_, StreamConfig, JsonConfig)); }
	
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//#     
	//#- `htModel = htModel.process(strArr, numArr, labelStr)` -- processes the stream example; `strArr` is an array of discrete attribute values (strings);
	//#   `numArr` is an array of numeric attribute values (numbers); `labelStr` is the class label of the example; the function returns self.
	//#- `htModel.process(line)` -- processes the stream example; `line` is comma-separated string of attribute values (for example `"a1,a2,c"`, where `c` is the class label); the function returns nothing.
	JsDeclareFunction(process);
	//#- `labelStr = htModel.classify(strArr, numArr)` -- classifies the stream example; `strArr` is an array of discrete attribute values (strings); `numArr` is an array of numeric attribute values (numbers); returns the class label `labelStr`.
	//#- `labelStr = htModel.classify(line)` -- classifies the stream example; `line` is comma-separated string of attribute values; returns the class label `labelStr`.
	JsDeclareFunction(classify);
	//#-  `htModel.predict(strArr, numArr` -- predicts numerical value that belongs to the example; `strArr` is an array of discrete values (strings); `numArr` is an array of numeric attribute values (numbers); returns a number.
	//#   
	JsDeclareFunction(predict);
	//#- `htModel = htModel.exportModel(htOutParams)` -- writes the current model into file `htOutParams.file` in format `htOutParams.type`. Returns self.
	//#   here, `htOutParams = { file: filePath, type: exportType }` where `file` is the file path and `type` is the export type (currently only `DOT` and `XML` are supported).
	JsDeclareFunction(exportModel);
};

///////////////////////////////
// QMiner-JavaScript-Tokenizer
//#
//# ### Tokenizer
//#
//# Breaks text into tokens (i.e. words).
class TJsTokenizer {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// Tokenizer Model
    PTokenizer Tokenizer;

private:
    typedef TJsObjUtil<TJsTokenizer> TJsTokenizerUtil;
	TJsTokenizer(TWPt<TScript> _Js, const PTokenizer& _Tokenizer): 
        Js(_Js), Tokenizer(_Tokenizer) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PTokenizer& Tokenizer) {
		return TJsTokenizerUtil::New(new TJsTokenizer(Js, Tokenizer)); }
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
    
	//# 
	//# **Functions and properties:**
	//#     
	//#- `arr = tokenizer.getTokens(string)` -- tokenizes given strings and returns it as an array of strings.
	JsDeclareFunction(getTokens);
	//#- `arr = tokenizer.getSentences(string)` -- breaks text into sentence and returns them as an array of strings.
    JsDeclareFunction(getSentences);
	//#- `arr = tokenizer.getParagraphs(string)` -- breaks text into paragraphs and returns them as an array of strings.
	JsDeclareFunction(getParagraphs);
};


///////////////////////////////
// QMiner-Snap
//# 
//# ## Snap graph library
//# 
//# A global object `snap` is used to construct graphs
//# it is available in any script. The object includes
//# several functions from snap library.
class TJsSnap {
public:
	/// JS script context
	TWPt<TScript> Js;

private:
	/// Object utility class
	typedef TJsObjUtil<TJsSnap> TJsSnapUtil;

	explicit TJsSnap(TWPt<TScript> _Js) : Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		return TJsSnapUtil::New(new TJsSnap(Js));
	}

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `graph = snap.newUGraph()` -- generate an empty undirected graph
	JsDeclareFunction(newUGraph);
	//#- `graph = snap.newDGraph()` -- generate an empty directed graph
	JsDeclareFunction(newDGraph);
	//#- `number = snap.DegreeCentrality(node)` -- returns degree centrality of a node
	JsDeclareFunction(DegreeCentrality);
	//#- `spvector = snap.CommunityDetection(UGraph, alg)` -- returns communities of graph (alg = `gn`, `imap` or `cnm`)
	JsDeclareFunction(CommunityDetection);
	//#- `jsonstring = snap.CommunityEvolution(path)` -- return communities alg = `gn`, `imap` or `cnm`
	JsDeclareFunction(CommunityEvolution);
	//#- `jsonstring = snap.CorePeriphery(UGraph, alg)` -- return communities alg = `lip`
	JsDeclareFunction(CorePeriphery);
};

///////////////////////////////
// QMiner-Undirected-Graph
//# 
//# ### Undirected Graph
//# 
//# Undirected graph
class TJsUGraph {
public:
	/// JS script context
	TWPt<TScript> Js;
	PUNGraph Graph;
	TStr InFNm;
private:
	/// Object utility class
	typedef TJsObjUtil<TJsUGraph> TJsUGraphUtil;

	TJsUGraph(TWPt<TScript> _Js) : Js(_Js) { 
		Graph = TUNGraph::New(); 
	};

	TJsUGraph(TWPt<TScript> _Js, TStr path) : Js(_Js), InFNm(path) { 
		Graph = TSnap::LoadEdgeList<PUNGraph>(InFNm); 
	};

public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		v8::Persistent<v8::Object> obj = TJsUGraphUtil::New(new TJsUGraph(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TUNGraph");
		obj->SetHiddenValue(key, value);
		return obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TStr path) {
		v8::Persistent<v8::Object> obj = TJsUGraphUtil::New(new TJsUGraph(Js, path));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TUNGraph");
		obj->SetHiddenValue(key, value);
		return obj;
	}

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `idx = graph.addNode()` -- add a node to graph and return its ID `idx`
	//#- `idx = graph.addNode(idx)` -- add a node with ID `idx`, returns node ID
	JsDeclareFunction(addNode);
	//#- `edgeIdx = graph.addEdge(nodeIdx1, nodeIdx2)` -- add an edge 
	JsDeclareFunction(addEdge);
	//#- `idx = graph.delNode(idx)` -- delete a node with ID `idx`
	JsDeclareFunction(delNode);
	//#- `idx = graph.delEdge(idx1, idx2)` -- delete an edge
	JsDeclareFunction(delEdge);
	//#- `isNode = graph.isNode(idx)` -- check if a node with ID `idx` exists in the graph
	JsDeclareFunction(isNode);
	//#- `isEdge = graph.isEdge(idx1, idx2)` -- check if an edge connecting nodes with IDs `idx1` and `idx2` exists in the graph
	JsDeclareFunction(isEdge);
	//#- `nodesCount = graph.nodeCount()` -- gets number of nodes in the graph
	JsDeclareFunction(nodeCount);
	//#- `edgesCount = graph.edgeCount()` -- gets number of edges in the graph
	JsDeclareFunction(edgeCount);
	//#- `node = graph.getNode(idx)` -- gets node with ID `idx`
	JsDeclareFunction(getNode);
	//#- `node = graph.getFirstNode()` -- gets first node
	JsDeclareFunction(getFirstNode);
	//#- `node = graph.getLastNode()` -- gets last node
	JsDeclareFunction(getLastNode);
	//#- `edge = graph.getFirstEdge()` -- gets first edge
	JsDeclareFunction(getFirstEdge);
	//#- `edge = graph.getLastEdge()` -- gets last edge
	JsDeclareFunction(getLastEdge)
	//#- `graph = graph.dump(fNm)` -- dumps a graph to file named `fNm`
	JsDeclareFunction(dump);
};

///////////////////////////////
// QMiner-Directed-Graph
//# 
//# ### Directed Graph
//# 
//# Directed graph
class TJsDGraph {
public:
	/// JS script context
	TWPt<TScript> Js;
	PNGraph Graph;
	TStr InFNm;
private:
	/// Object utility class
	typedef TJsObjUtil<TJsDGraph> TJsDGraphUtil;

	TJsDGraph(TWPt<TScript> _Js) : Js(_Js) {
		Graph = TNGraph::New();
	};

	TJsDGraph(TWPt<TScript> _Js, TStr path) : Js(_Js), InFNm(path) {
		Graph = TSnap::LoadEdgeList<PNGraph>(InFNm);
	};

public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		v8::Persistent<v8::Object> obj = TJsDGraphUtil::New(new TJsDGraph(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TNGraph");
		obj->SetHiddenValue(key, value);
		return obj;
	}
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TStr path) {

		v8::Persistent<v8::Object> obj = TJsDGraphUtil::New(new TJsDGraph(Js, path));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TNGraph");
		obj->SetHiddenValue(key, value);
		return obj;

	}

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	//# 
	//# **Functions and properties:**
	//# 
	//#- `idx = graph.addNode()` -- add a node to graph and return its ID `idx`
	//#- `idx = graph.addNode(idx)` -- add a node with ID `idx`, returns node ID
	//# 
	//# **Functions and properties:**
	//# 
	//#- `idx = graph.addNode()` -- add a node to graph and return its ID `idx`
	//#- `idx = graph.addNode(idx)` -- add a node with ID `idx`, returns node ID
	JsDeclareFunction(addNode);
	//#- `edgeIdx = graph.addEdge(nodeIdx1, nodeIdx2)` -- add an edge 
	JsDeclareFunction(addEdge);
	//#- `idx = graph.delNode(idx)` -- delete a node with ID `idx`
	JsDeclareFunction(delNode);
	//#- `idx = graph.delEdge(idx1, idx2)` -- delete an edge
	JsDeclareFunction(delEdge);
	//#- `isNode = graph.isNode(idx)` -- check if a node with ID `idx` exists in the graph
	JsDeclareFunction(isNode);
	//#- `isEdge = graph.isEdge(idx1, idx2)` -- check if an edge connecting nodes with IDs `idx1` and `idx2` exists in the graph
	JsDeclareFunction(isEdge);
	//#- `nodesCount = graph.nodeCount()` -- gets number of nodes in the graph
	JsDeclareFunction(nodeCount);
	//#- `edgesCount = graph.edgeCount()` -- gets number of edges in the graph
	JsDeclareFunction(edgeCount);
	//#- `node = graph.getNode(idx)` -- gets node with ID `idx`
	JsDeclareFunction(getNode);
	//#- `node = graph.getFirstNode()` -- gets first node
	JsDeclareFunction(getFirstNode);
	//#- `node = graph.getLastNode()` -- gets last node
	JsDeclareFunction(getLastNode);
	//#- `edge = graph.getFirstEdge()` -- gets first edge
	JsDeclareFunction(getFirstEdge);
	//#- `edge = graph.getLastEdge()` -- gets last edge
	JsDeclareFunction(getLastEdge)
	//#- `graph = graph.dump(fNm)` -- dumps a graph to file named `fNm`
	JsDeclareFunction(dump);
};

///////////////////////////////
// QMiner-Node
//# 
//# ### Node
//# 
//# Node

template <class T>
class TJsNode {
public:
	/// JS script context
	TWPt<TScript> Js;
	T Node;
	TIntV NIdV;

private:
	/// Object utility class
	typedef TJsObjUtil<TJsNode> TJsNodeUtil;
	TInt Id;
	TJsNode(TWPt<TScript> Js_, T a);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, T a) {
		v8::Persistent<v8::Object> obj = TJsNodeUtil::New(new TJsNode(Js, a));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("Graph");
		obj->SetHiddenValue(key, value);
		return obj;
	}

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//# 
	//#- `id = node.getId()` -- return id of the node
	JsDeclareFunction(getId);
	//#- `deg = node.getDeg()` -- return degree of the node
	JsDeclareFunction(getDeg);
	//#- `indeg = node.getDeg()` -- return in-degree of the node
	JsDeclareFunction(getInDeg);
	//#- `outdeg = node.getDeg()` -- return out-degree of the node
	JsDeclareFunction(getOutDeg);
	//#- `nid = node.getNbrNId(N)` -- return id of Nth neighbour
	JsDeclareFunction(getNbrNId);
	//#- `node = node.getNext()` -- return next node
	JsDeclareFunction(getNext);
	//#- `node = graph.getPrev()` -- return previous node
	JsDeclareFunction(getPrev);
};

///////////////////////////////
// QMiner-Edge
//# 
//# ### Edge
//# 
//# Edge
template <class T>
class TJsEdge {
public:
	/// JS script context
	TWPt<TScript> Js;
	T Edge;
	TIntV NIdV;

private:
	/// Object utility class
	typedef TJsObjUtil<TJsEdge> TJsEdgeUtil;
	TInt Id;
	explicit TJsEdge(TWPt<TScript> Js_, T edge);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, T edge) {
		return TJsEdgeUtil::New(new TJsEdge(Js, edge));
	}

	/// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	//# 
	//# **Functions and properties:**
	//# 
	//#- `id = edge.getId()` -- return id of the edge
	JsDeclareFunction(getId);
	//#- `id = edge.getSrcNodeId()` -- return id of source node
	JsDeclareFunction(getSrcNodeId);
	//#- `id = edge.getDstNodeId()` -- return id of destination node
	JsDeclareFunction(getDstNodeId);
	//#- `edge = edge.getNext()` -- return next edge
	JsDeclareFunction(getNext);

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
	//#- `process.stop()` -- Stops the current process.
	//#- `process.stop(returnCode)` -- Stops the current process and returns `returnCode
    JsDeclareFunction(stop);
	//#- `process.sleep(millis)` -- Halts execution for the given amount of milliseconds `millis`.
    JsDeclareFunction(sleep);
    //#- `a = process.args` -- array of command-line arguments 
    //#     used to start current QMiner instance
    JsDeclareProperty(args);
    //#- `objJSON = process.sysStat` -- statistics about system and qminer process (E.g. memory consumption).
    JsDeclareProperty(sysStat);
	//#- `str = process.scriptNm` -- Returns the name of the script.
	JsDeclareProperty(scriptNm);
	//#- `str = process.scriptFNm` -- Returns absolute script file path.
	JsDeclareProperty(scriptFNm);
	//#- `globalVarNames = process.getGlobals()` -- Returns an array of all global variable names
	JsDeclareFunction(getGlobals);
	//#- `process.exitScript()` -- Exits the current script
	JsDeclareFunction(exitScript);
    //#- `process.returnCode` -- current code to be returned by QMiner process
  	JsDeclareSetProperty(getReturnCode, setReturnCode);
	//#- `str = process.qminer_home` -- returns the path to QMINER_HOME
	JsDeclareProperty(qminer_home);
	//#- `str = process.project_home` -- returns the path to project folder
	JsDeclareProperty(project_home);
    //#JSIMPLEMENT:src/qminer/process.js
};


//#
//# ### utilities.js (use require)
//# 
//#JSIMPLEMENT:src/qminer/js/utilities.js    

//#
//# ### assert.js (use require)
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
    //#- `console.log(prefixStr, message)` -- writes `message` to standard output, 
    //#     using provided prefix `[prefixStr]`; both `message` and `prefixStr` must
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
    //#- `fin = fs.openRead(fileName)` -- open file in read mode and return file input stream `fin`
	JsDeclareFunction(openRead);
    //#- `fout = fs.openWrite(fileName)` -- open file in write mode and return file output stream `fout`
	JsDeclareFunction(openWrite);
    //#- `fout = fs.openAppend(fileName)` -- open file in append mode and return file output stream `fout`
	JsDeclareFunction(openAppend);
    //#- `bool = fs.exists(fileName)` -- does file exist?
	JsDeclareFunction(exists);
    //#- `fs.copy(fromFileName, toFileName)` -- copy file
	JsDeclareFunction(copy);
    //#- `fs.move(fromFileName, toFileName)` -- move file
	JsDeclareFunction(move);
    //#- `fs.del(fileName)` -- delete file
	JsDeclareFunction(del);
    //#- `fs.rename(fromFileName, toFileName)` -- rename file
	JsDeclareFunction(rename);
    //#- `fileInfoJson = fs.fileInfo(fileName)` -- returns file info as a json object {createTime:str, lastAccessTime:str, lastWriteTime:str, size:num}.
	JsDeclareFunction(fileInfo);
    //#- `fs.mkdir(dirName)` -- make folder
	JsDeclareFunction(mkdir);
    //#- `fs.rmdir(dirName)` -- delete folder
	JsDeclareFunction(rmdir);
    //#- `strArr = fs.listFile(dirName, fileExtension)` -- returns list of files in directory given file extension
    //#- `strArr = fs.listFile(dirName, fileExtension, recursive)` -- returns list of files in directory given extension. `recursive` is a boolean
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
    //#- `char = fin.peekCh()` -- peeks a character
	JsDeclareFunction(peekCh);
    //#- `char = fin.getCh()` -- reads a character
	JsDeclareFunction(getCh);
    //#- `line = fin.readLine()` -- reads a line
	JsDeclareFunction(readLine);
    //#- `bool = fin.eof` -- end of stream?
	JsDeclareProperty(eof);
    //#- `len = fin.length` -- returns the length of input stream
	JsDeclareProperty(length);
	//#- `str = fin.readAll()` -- reads the whole file
	JsDeclareFunction(readAll);
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
	TJsFOut(PSOut& SOut_) : SOut(SOut_) { }
public:
	static v8::Persistent<v8::Object> New(const TStr& FilePath, const bool& AppendP = false) { 
		return TJsFOutUtil::New(new TJsFOut(FilePath, AppendP)); }
	static v8::Persistent<v8::Object> New(PSOut& SOut_) {
		return TJsFOutUtil::New(new TJsFOut(SOut_)); }

    static PSOut GetArgFOut(const v8::Arguments& Args, const int& ArgN);
    
	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    //# 
	//# **Functions and properties:**
	//#     
    //#- `fout = fout.write(data)` -- writes to output stream. `data` can be a number, a json object or a string.
	JsDeclareFunction(write);
    //#- `fout = fout.writeLine(data)` -- writes data to output stream and adds newline
	JsDeclareFunction(writeLine);
    //#- `fout = fout.flush()` -- flushes output stream
	JsDeclareFunction(flush);
    //#- `fout = fout.close()` -- closes output stream
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
	//#- `http.get(url)` -- gets url, but does nothing with response
    //#- `http.get(url, httpJsonSuccessCallback)` -- gets url and executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.
    //#- `http.get(url, httpJsonSuccessCallback, httpErrorCallback)` -- gets url and executes httpJsonSuccessCallback (signature: function (objJson) {}) on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.
	//#- `http.getStr(url)` -- gets url, but does nothing with response
	//#- `http.getStr(url, httpStrSuccessCallback)` -- gets url and executes httpStrSuccessCallback, a function with signature: function (str) {} on success. 
    //#- `http.getStr(url, httpStrSuccessCallback, httpErrorCallback)` -- gets url and executes httpJsonSuccessCallback (signature: function (str) {}) on success or httpErrorCallback (signature: function (message) {}) on error.
	JsDeclareFunction(get);
    //#- `http.post(url, mimeType, data)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string)
    //#- `http.post(url, mimeType, data, httpJsonSuccessCallback)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success. Error will occour if objJson is not a JSON object.
    //#- `http.post(url, mimeType, data, httpJsonSuccessCallback, httpErrorCallback)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpJsonSuccessCallback, a function with signature: function (objJson) {} on success or httpErrorCallback (signature: function (message) {}) on error. Error will occour if objJson is not a JSON object.
    //#- `http.postStr(url)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string)
    //#- `http.postStr(url, mimeType, data, httpStrSuccessCallback)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success.
    //#- `http.postStr(url, mimeType, data, httpStrSuccessCallback, httpErrorCallback)` -- post to `url` (string) using `mimeType` (string), where the request body is `data` (string). executes httpStrSuccessCallback, a function with signature: function (str) {} on success or httpErrorCallback (signature: function (message) {}) on error.
	JsDeclareFunction(post);
    //#- `http.onRequest(path, verb, httpRequestCallback)` -- path: function path without server name and script name. Example: `http.onRequest("test", "GET", function (req, resp) { })` executed from `script.js` on localhost will execute a get request from `http://localhost/script/test`. `verb` can be one of the following {"GET","POST","PUT","DELETE","PATCH"}. `httpRequestCallback` is a function with signature: function (request, response) { /*...*/ }
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
    //#- `httpResponse.setStatusCode(statusCode)` -- sets status code (integer)
	JsDeclareFunction(setStatusCode);
    //#- `httpResponse.setContentType(mimeType)` -- sets content type (string)
    JsDeclareFunction(setContentType);
    //#- `httpResponse.add(dataStr)` -- adds `dataStr` (string) to request body
	//#- `httpResponse.add(dataJSON)` -- adds `dataJSON` (JSON object) to request body
	JsDeclareFunction(add);	
    //#- `httpResponse.close()` -- closes and executes the response
	JsDeclareFunction(close);
    //#- `httpResponse.send(dataStr)` -- adds `dataStr` (string) and closes the response
	//#- `httpResponse.send(dataJSON)` -- adds `dataJSON` (JSON object) and closes the response
    JsDeclareFunction(send);
};

///////////////////////////////
// QMiner-JavaScript-Time
//#
//# ### Date-Time
//#
//# Wrapper around GLib's TTm. Used as return for `DateTime` field type. 
//# New one can be created using `tm = require('time.js')`.
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
    //#- `str = tm.string` -- string representation of time (e.g. 2014-05-29T10:09:12)
    JsDeclareProperty(string);
    //#- `str = tm.dateString` -- string representation of date (e.g. 2014-05-29)
    JsDeclareProperty(dateString);
    //#- `num = tm.timestamp` -- unix timestamp representation of time (seconds since 1970)
    JsDeclareProperty(timestamp);
    //#- `num = tm.year` -- year (number)
    JsDeclareProperty(year);
    //#- `num = tm.month` -- month (number)
    JsDeclareProperty(month);
    //#- `num = tm.day` -- day (number)
    JsDeclareProperty(day);
	//#- `str = tm.dayOfWeek` -- day of week (string)	
    JsDeclareProperty(dayOfWeek);
    //#- `num = tm.dayOfWeekNum` -- day of week (number)
    JsDeclareProperty(dayOfWeekNum);
    //#- `num = tm.hour` -- hour (number)
    JsDeclareProperty(hour);
    //#- `num = tm.minute` -- minute (number)
    JsDeclareProperty(minute);
    //#- `num = tm.second` -- second (number)
    JsDeclareProperty(second);
    //#- `num = tm.millisecond` -- millisecond (number)
    JsDeclareProperty(millisecond);
	JsDeclareProperty(milisecond); // deprecated
    //#- `tm2 = tm.now` -- returns new time object representing current local time
    JsDeclareProperty(now);
    //#- `tm2 = tm.nowUTC` -- returns new time object represented current UTC time
    JsDeclareProperty(nowUTC);    
    //#- `tm = tm.add(val, unit)` -- adds `val` to the time and returns self; `unit` defines the unit 
    //#     of `val`, options are `second` (default), `minute`, `hour`, and `day`.
    JsDeclareFunction(add);
    //#- `tm = tm.sub(val, unit)` -- subtracts `val` from the time and returns self; `unit` defintes the unit of `val`. options are `second` (default), `minute`, `hour`, and `day`.
    JsDeclareFunction(sub); 
    //#- `tmJSON = tm.toJSON()` -- returns json representation of time    
    JsDeclareFunction(toJSON);
    //#- `tm2 = tm.parse(str)` -- parses string `str` in weblog format (example: `2014-05-29T10:09:12`)  and returns a date time object. Weblog format uses `T` to separate date and time, uses `-` for date units separation and `:` for time units separation (`YYYY-MM-DDThh-mm-ss`).
    //#     as Date-Time object
	JsDeclareFunction(parse);
	//#- `tm2 = tm.fromWindowsTimestamp(num)` -- constructs date time from a windows timestamp (milliseconds since 1601).
	JsDeclareFunction(fromWindowsTimestamp);
	//#- `tm2 = tm.fromUnixTimestamp(num)` -- constructs date time from a UNIX timestamp (seconds since 1970).
	JsDeclareFunction(fromUnixTimestamp);
	//#- `tm2 = tm.clone()` -- clones `tm` to `tm2`
	JsDeclareFunction(clone);
	//#- `num = tm.windowsTimestamp` -- returns windows system time in milliseconds from 1/1/1601
	JsDeclareProperty(windowsTimestamp);
};
//#
//# ## Other libraries
//#
//#JSIMPLEMENT:src/qminer/js/twitter.js 
//#JSIMPLEMENT:src/qminer/js/xml.js 
//#JSIMPLEMENT:src/qminer/js/visualization.js 

///////////////////////////////////////////////
/// Javscript Function Feature Extractor.
//-
//- ## Javascript Feature Extractor
//-
class TJsFuncFtrExt : public TFtrExt {
// Js wrapper API
public:
	/// JS script context
	TWPt<TScript> Js;
private:
	typedef TJsObjUtil<TJsFuncFtrExt> TJsFuncFtrExtUtil;
	// private constructor
	TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal, const v8::Persistent<v8::Function>& _Fun): 
        TFtrExt(_Js->Base, ParamVal), Js(_Js), Fun(_Fun) { 
            Name = ParamVal->GetObjStr("name", "jsfunc"); 
            Dim = ParamVal->GetObjInt("dim", 1); }
public:
	// public smart pointer
	static PFtrExt NewFtrExt(TWPt<TScript> Js, const PJsonVal& ParamVal, 
        const v8::Persistent<v8::Function>& _Fun) {
            return new TJsFuncFtrExt(Js, ParamVal, _Fun); }
// Core functionality
private:
	// Core part
	TInt Dim;
	TStr Name;
	v8::Persistent<v8::Function> Fun;
    
	double ExecuteFunc(const TRec& FtrRec) const {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Value> RecArg = TJsRec::New(Js, FtrRec);		
		return Js->ExecuteFlt(Fun, RecArg);
	}
	
	void ExecuteFuncVec(const TRec& FtrRec, TFltV& Vec) const {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Value> RecArg = TJsRec::New(Js, FtrRec);		
		Js->ExecuteFltVec(Fun, RecArg, Vec);
	}
public:
	// Assumption: object without key "fun" is a JSON object (the key "fun" is reserved for a javascript function, which is not a JSON object)
	static PJsonVal CopySettings(v8::Local<v8::Object> Obj) {
		// clone all properties except fun!
		v8::Local<v8::Array> Properties = Obj->GetOwnPropertyNames();
		PJsonVal ParamVal = TJsonVal::NewObj();
		for (uint32 PropN = 0; PropN < Properties->Length(); PropN++) {
			// get each property as string, extract arg json and attach it to ParamVal
			TStr PropStr = TJsUtil::V8JsonToStr(Properties->Get(PropN));
			PropStr = PropStr.GetSubStr(1, PropStr.Len() - 2); // remove " char at the beginning and end
			if (PropStr == "fun") continue;
			v8::Handle<v8::Value> Val = Obj->Get(Properties->Get(PropN));
			if (Val->IsNumber()) {
				ParamVal->AddToObj(PropStr, Val->NumberValue());
			}
			if (Val->IsString()) {
				v8::String::Utf8Value Utf8(Val);
				TStr ValueStr(*Utf8);
				ParamVal->AddToObj(PropStr, ValueStr);
			}
			if (Val->IsBoolean()) {
				ParamVal->AddToObj(PropStr, Val->BooleanValue());
			}
			if (Val->IsObject() || Val->IsArray()) {
				ParamVal->AddToObj(PropStr, TJsFuncFtrExtUtil::GetValJson(Val));
			}
		}
		//printf("JSON: %s\n", TJsonVal::GetStrFromVal(ParamVal).CStr());
		return ParamVal;
	}
// Feature extractor API
private:
	TJsFuncFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal); // will throw exception (saving, loading not supported)
	TJsFuncFtrExt(const TWPt<TBase>& Base, TSIn& SIn); // will throw exception (saving, loading not supported)
public:
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal); // will throw exception (saving, loading not supported)
	static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn); // will throw exception (saving, loading not supported)
	void Save(TSOut& SOut) const;

	TStr GetNm() const { return Name; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return TStr::Fmt("%s[%d]", GetNm().CStr(), FtrN) ; }

	void Clr() { };
	bool Update(const TRec& Rec) { return false; }
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	// flat feature extraction
	void ExtractFltV(const TRec& FtrRec, TFltV& FltV) const;

	// feature extractor type name 
	static TStr GetType() { return "jsfunc"; }
};


}

#endif

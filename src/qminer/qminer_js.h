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
		} catch(PExcept Except) { \
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
		} catch(PExcept Except) { \
			if(typeid(Except) == typeid(TQmExcept::New(""))) { \
				v8::Handle<v8::Value> Why = v8::String::New(Except->GetMsgStr().CStr()); \
				v8::ThrowException(Why); \
				return v8::Undefined(); \
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
		} catch(PExcept Except) { \
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

public:
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
    static PJsonVal HttpRqToJson(PHttpRq HttpRq);
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
// QMiner-JavaScript-Object-Utility
template <class TJsObj>
class TJsObjUtil {
public:  
	/// Creates new empty JavaScript object around TJsObj
	static v8::Persistent<v8::Object> New(TJsObj* JsObj) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::ObjectTemplate> Template = TJsObj::GetTemplate();
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Template->NewInstance());
		Object->SetInternalField(0, v8::External::New(JsObj));
		Object.MakeWeak(NULL, &Clean);
		return Object;
	}

	/// Creates new JavaScript object around TJsObj, using supplied template
	static v8::Persistent<v8::Object> New(TJsObj* JsObj, const v8::Handle<v8::ObjectTemplate>& Template) {
		v8::HandleScope HandleScope;
		v8::Persistent<v8::Object> Object = v8::Persistent<v8::Object>::New(Template->NewInstance());
		Object->SetInternalField(0, v8::External::New(JsObj));
		Object.MakeWeak(NULL, &Clean);
		return Object;
	}

	/// Destructor of JavaScript object, calls TJsObj destructor
	static void Clean(v8::Persistent<v8::Value> Handle, void* Id) {
		v8::HandleScope HandleScope;
		v8::Handle<v8::Object> Object = v8::Handle<v8::Object>::Cast(Handle);
		v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Object->GetInternalField(0));
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
		v8::Handle<v8::Value> ClassNm = Obj->GetHiddenValue(v8::String::New("class"));
		if (!ClassNm->IsString()) return "";		
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
			//TODO: check IsObject()
			if (Args[ArgN]->ToObject()->Has(v8::String::New(Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[0]->ToObject()->Get(v8::String::New(Property.CStr()));
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

	/// Extract argument ArgN as JSon object, and transform it to PJsonVal
	static PJsonVal GetArgJson(const v8::Arguments& Args, const int& ArgN) {
		TStr JsonStr = GetArgJsonStr(Args, ArgN);
		PJsonVal Val = TJsonVal::GetValFromStr(JsonStr);
		if (!Val->IsDef()) { throw TQmExcept::New("Error parsing '" + JsonStr + "'."); }
		return Val;
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
	TVec<TPair<TUCh, PStoreTrigger> > TriggerV;
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
    v8::Handle<v8::Value> ExecuteV8(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal);
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
	void ExecuteSrvFun(const PHttpRq& HttpRq, v8::Handle<v8::Object>& RespObj);
    /// Get array of registered server function rules
    PJsonVal GetSrvFunRules() const { return TJsonVal::NewArr(SrvFunRuleV); }
	/// Add new fetch request
	void AddFetchRq(const TJsFetchRq& Rq);
	/// Remember new trigger
	void AddTrigger(const uchar& StoreId, const PStoreTrigger& Trigger);

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

	void OnAdd(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId);
	void OnUpdate(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId);
	void OnDelete(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId);
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
// JavaScript Console
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

	// functions
	JsDeclareFunction(say);
};

///////////////////////////////
// JavaScript QMiner Base
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
	
	// property
	//JsDeclareProperty(correlation);
	JsDeclareProperty(args);
	JsDeclareProperty(analytics);
	// basic functions
	JsDeclareFunction(store);
	JsDeclareFunction(getStoreList);
	JsDeclareFunction(createStore);
	JsDeclareFunction(search);   
	JsDeclareFunction(op);
	JsDeclareFunction(gc);
};

///////////////////////////////
// QMiner-JavaScript-Store
class TJsStore {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;

	typedef TJsObjUtil<TJsStore> TJsStoreUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

	TJsStore(TWPt<TScript> _Js, TWPt<TStore> _Store): Js(_Js), Store(_Store) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, TWPt<TStore> Store) { 
		return TJsStoreUtil::New(new TJsStore(Js, Store)); }
	~TJsStore() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// properties
	JsDeclareProperty(name);
	JsDeclareProperty(empty);
	JsDeclareProperty(length);
	JsDeclareProperty(recs);
	JsDeclareProperty(fields);
	JsDeclareProperty(joins);	
	JsDeclareProperty(keys);	
	// index
	JsDeclIndexedProperty(indexId);
	// functions
	JsDeclareFunction(rec);
	JsDeclareFunction(add);
	JsDeclareFunction(sample);
	JsDeclareFunction(key);
	JsDeclareFunction(addTrigger);
    JsDeclareFunction(addStreamAggr);
	JsDeclareFunction(getStreamAggr);
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
class TJsRecSet {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TWPt<TStore> Store;
	PRecSet RecSet;	

	typedef TJsObjUtil<TJsRecSet> TJsRecSetUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

	TJsRecSet(TWPt<TScript> _Js, const PRecSet& _RecSet);
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PRecSet& RecSet);
	static PRecSet GetArgRecSet(const v8::Arguments& Args, const int& ArgN);
	~TJsRecSet() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// properties
	JsDeclareProperty(store);
	JsDeclareProperty(length);
	JsDeclareProperty(empty);
	JsDeclareProperty(weighted);
	// index
	JsDeclIndexedProperty(indexId);
	// functions
	JsDeclareFunction(clone);
	JsDeclareFunction(join);
	JsDeclareFunction(aggr);
	JsDeclareFunction(trunc);
	JsDeclareFunction(sample);
	JsDeclareFunction(shuffle);
	JsDeclareFunction(reverse);
	JsDeclareFunction(sortById);
	JsDeclareFunction(sortByFq);
	JsDeclareFunction(sortByField);
   	JsDeclareFunction(sort);
	JsDeclareFunction(filterById);
	JsDeclareFunction(filterByFq);
	JsDeclareFunction(filterByField);
	JsDeclareFunction(filter);
	JsDeclareFunction(toJSON);
};

///////////////////////////////
// QMiner-JavaScript-Record
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

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate(const TWPt<TBase>& Base, const TWPt<TStore>& Store);
	// properties
    JsDeclareProperty(id);
    JsDeclareProperty(name);
	JsDeclareProperty(fq);
	JsDeclareProperty(field);
	JsDeclareProperty(join);
	JsDeclareProperty(sjoin);
    // function
    JsDeclareFunction(addJoin);
    JsDeclareFunction(delJoin);
    JsDeclareFunction(toJSON);
};

///////////////////////////////
// QMiner-JavaScript-IndexKey
class TJsIndexKey {
private:
	/// JS script context
	TWPt<TScript> Js;	
	TIndexKey IndexKey;

	typedef TJsObjUtil<TJsIndexKey> TJsIndexKeyUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

	TJsIndexKey(TWPt<TScript> _Js, const TIndexKey& _IndexKey): 
		Js(_Js), IndexKey(_IndexKey) { }	
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TIndexKey& IndexKey) { 
		return TJsIndexKeyUtil::New(new TJsIndexKey(Js, IndexKey)); }
	~TJsIndexKey() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// properties
	JsDeclareProperty(store);				
	JsDeclareProperty(name);
	JsDeclareProperty(voc);
	JsDeclareProperty(fq);
};

///////////////////////////////
// QMiner-JavaScript-Machine-Learning
class TJsAnalytics {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// maps an AL id string to an AL object
    static THash<TStr, PBowAL> ActiveLearnerH; 
    
private:
	typedef TJsObjUtil<TJsAnalytics> TJsAnalyticsUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsAnalytics(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
        return TJsAnalyticsUtil::New(new TJsAnalytics(Js)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	JsDeclareFunction(getLanguageOptions);
    JsDeclareFunction(newFeatureSpace);
	JsDeclareFunction(trainSvmClassify);
    JsDeclareFunction(trainSvmRegression);
	JsDeclareFunction(trainKMeans);
	JsDeclareFunction(newActiveLearner);
	JsDeclareFunction(delActiveLearner);
	JsDeclareFunction(newRecLinReg);
};

///////////////////////////////
// QMiner-JavaScript-Feature-Space
class TJsFtrSpace {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// Feature space 
    PFtrSpace FtrSpace;    
    
private:
	typedef TJsObjUtil<TJsFtrSpace> TJsFtrSpaceUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsFtrSpace(TWPt<TScript> _Js, const PFtrSpace& _FtrSpace): Js(_Js), FtrSpace(_FtrSpace) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PFtrSpace& FtrSpace) { 
		return TJsFtrSpaceUtil::New(new TJsFtrSpace(Js, FtrSpace)); }
    static PFtrSpace GetArgFtrSpace(const v8::Arguments& Args, const int& ArgN);

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    // for batch learning of feature space
	JsDeclareFunction(updateRecord);
	JsDeclareFunction(updateRecords);
    JsDeclareFunction(finishUpdate);
    // use feature extractors to extract string features (E.g. words)
    JsDeclareFunction(extractStrings);
    // mapping of records to feature vectors
    JsDeclareFunction(ftrSpVec);
    JsDeclareFunction(ftrVec);
};

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
class TJsSvmModel {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// Feature Space
    PFtrSpace FtrSpace;    
    /// Should vectors be normalized prior to usage
    TBool NormalizeP;
    /// Does model have probabilistic sigmoid mapping
    TBool ProbP;
    /// SVM Model
    PSVMModel Model;
    
private:
	typedef TJsObjUtil<TJsSvmModel> TJsSvmModelUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsSvmModel(TWPt<TScript> _Js, const PFtrSpace& _FtrSpace, 
        const bool& _NormalizeP, const PSVMModel& _Model): 
            Js(_Js), FtrSpace(_FtrSpace), NormalizeP(_NormalizeP), 
            ProbP(false), Model(_Model) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const PFtrSpace& FtrSpace, 
        const bool& NormalizeP, const PSVMModel& Model) { return TJsSvmModelUtil::New(
            new TJsSvmModel(Js, FtrSpace, NormalizeP, Model)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    JsDeclareProperty(featureSpace);
	JsDeclareFunction(classify);
};

///////////////////////////////
// QMiner-JavaScript-Active-Learner
class TJsAL {
public:
	/// JS script context
	TWPt<TScript> Js;	
    /// AL Model
    PBowAL AL;
    
private:
	typedef TJsObjUtil<TJsAL> TJsALUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsAL(TWPt<TScript> _Js, const PBowAL& _AL): 
            Js(_Js), AL(_AL) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js,
            const PBowAL& AL) { return TJsALUtil::New(
            new TJsAL(Js, AL)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    JsDeclareFunction(getQuestion);
	JsDeclareFunction(answerQuestion);
	JsDeclareFunction(getPositives);
};

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression
class TJsRecLinRegModel {
public:
	/// JS script context
	TWPt<TScript> Js;	
	/// RecLinReg Model
	TSignalProc::PRecLinReg Model;    
private:
	typedef TJsObjUtil<TJsRecLinRegModel> TJsRecLinRegModelUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;    
	TJsRecLinRegModel(TWPt<TScript> _Js, const TSignalProc::PRecLinReg& _Model): Js(_Js), Model(_Model) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js, const TSignalProc::PRecLinReg& Model) {
		return TJsRecLinRegModelUtil::New(new TJsRecLinRegModel(Js, Model)); }
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	JsDeclareFunction(learn);
	JsDeclareFunction(predict);
};

///////////////////////////////
// QMiner-JavaScript-Correlation
class TJsCorrelation {

private:
	typedef TJsObjUtil<TJsCorrelation> TJsCorrelationUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

public:
	explicit TJsCorrelation() { }	
	static v8::Persistent<v8::Object> New() { return TJsCorrelationUtil::New(new TJsCorrelation); }
	~TJsCorrelation() { }

	// template
	static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// function
	JsDeclareFunction(pearson);					
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
	static v8::Persistent<v8::ObjectTemplate> Template;

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
// QMiner-JavaScript-Filesystem
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

	// functions
	JsDeclareFunction(openRead);
	JsDeclareFunction(openWrite);
	JsDeclareFunction(openAppend);
	JsDeclareFunction(exists);
	JsDeclareFunction(copy);
	JsDeclareFunction(move);
	JsDeclareFunction(del);
	JsDeclareFunction(rename);
	JsDeclareFunction(fileInfo);
	JsDeclareFunction(mkdir);
	JsDeclareFunction(rmdir);
	JsDeclareFunction(listFile);
};

///////////////////////////////
// QMiner-JavaScript-FIn
class TJsFIn {
public:
	PSIn SIn;
private:
	typedef TJsObjUtil<TJsFIn> TJsFInUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;

	TJsFIn(const TStr& FNm): SIn(TZipIn::NewIfZip(FNm)) { }
public:
	static v8::Persistent<v8::Object> New(const TStr& FNm) {
		return TJsFInUtil::New(new TJsFIn(FNm)); }

   	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	JsDeclareFunction(peekCh);
	JsDeclareFunction(getCh);
	JsDeclareFunction(getNextLn);
	JsDeclareProperty(eof);
	JsDeclareProperty(length);
};

///////////////////////////////
// QMiner-JavaScript-FOut
class TJsFOut {
public:
	PSOut SOut;
private:
	typedef TJsObjUtil<TJsFOut> TJsFOutUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
	TJsFOut(const TStr& FilePath, const bool& AppendP): SOut(TFOut::New(FilePath, AppendP)) { }
	TJsFOut(const TStr& FilePath): SOut(TZipOut::NewIfZip(FilePath)) { }

public:
	static v8::Persistent<v8::Object> New(const TStr& FilePath, const bool& AppendP = false) { 
		return TJsFOutUtil::New(new TJsFOut(FilePath, AppendP)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	JsDeclareFunction(write);
	JsDeclareFunction(writeLine);
	JsDeclareFunction(flush);
};

///////////////////////////////
// JavaScript Http
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

	// server
	JsDeclareFunction(onRequest);
    // client
	JsDeclareFunction(get);
	JsDeclareFunction(post);
};

///////////////////////////////
// QMiner-JavaScript-Http-Response
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
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsHttpResp(TWebSrv* _WebSrv, const uint64& _SockId): WebSrv(_WebSrv),
        SockId(_SockId), StatusCode(THttp::OkStatusCd), 
        ContTypeStr(THttp::AppJSonFldVal), DoneP(false) { }
public:
	static v8::Persistent<v8::Object> New(TWebSrv* WebSrv, const uint64& SockId) { 
		return TJsHttpRespUtil::New(new TJsHttpResp(WebSrv, SockId)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

	JsDeclareFunction(setStatusCode);
    JsDeclareFunction(setContentType);
	JsDeclareFunction(add);	
	JsDeclareFunction(close);
    JsDeclareFunction(send);
};

///////////////////////////////
// QMiner-JavaScript-Time
class TJsTm {
public:
    /// Date-time
    TTm Tm;
private:
	typedef TJsObjUtil<TJsTm> TJsTmUtil;
	static v8::Persistent<v8::ObjectTemplate> Template;
    
	TJsTm(const TTm& _Tm): Tm(_Tm) { }
public:
	static v8::Persistent<v8::Object> New() { 
		return TJsTmUtil::New(new TJsTm(TTm::GetCurUniTm())); }
	static v8::Persistent<v8::Object> New(const TTm& Tm) { 
		return TJsTmUtil::New(new TJsTm(Tm)); }

	static v8::Handle<v8::ObjectTemplate> GetTemplate();

    // object properties
    JsDeclareProperty(string);
    JsDeclareProperty(timestamp);
    // for creating new objects
    JsDeclareProperty(now);
    JsDeclareProperty(nowUTC);
    
    // object functions
    JsDeclareFunction(add);
    JsDeclareFunction(sub); 
    JsDeclareFunction(toJSON);
    // for creating new objects
    JsDeclareFunction(parse)
};

///////////////////////////////
// QMiner-LinAlg
class TJsLinAlg {
public:
	/// JS script context
	TWPt<TScript> Js;    
private:
	TFltV Vec;
	/// Object utility class
	typedef TJsObjUtil<TJsLinAlg> TJsLinAlgUtil;
    
    explicit TJsLinAlg(TWPt<TScript> _Js): Js(_Js) { }
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) { 
		return TJsLinAlgUtil::New(new TJsLinAlg(Js)); }

	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

	JsDeclareFunction(newVec);
	JsDeclareFunction(newMat);
};

///////////////////////////////
// QMiner-FltV
class TJsFltV {
public:
	/// JS script context
	TWPt<TScript> Js;    
	TFltV Vec;
private:	
	/// Object utility class
	typedef TJsObjUtil<TJsFltV> TJsFltVUtil;    
    explicit TJsFltV(TWPt<TScript> _Js): Js(_Js) { }	
public:
	static v8::Persistent<v8::Object> New(TWPt<TScript> Js) {
		v8::Persistent<v8::Object> obj = TJsFltVUtil::New(new TJsFltV(Js));
		v8::Handle<v8::String> key = v8::String::New("class");
		v8::Handle<v8::String> value = v8::String::New("TFltV");
		obj->SetHiddenValue(key, value);
		return  obj;
	}
	static TFltV& GetFltV(const v8::Handle<v8::Object> Obj) {
		return TJsFltVUtil::GetSelf(Obj)->Vec;
	}
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();

	// get element
	JsDeclareFunction(at);	
	// set element, returns undefined
	JsDeclareFunction(put);	
	// INPLACE : append an element, returns undefined
	JsDeclareFunction(push);	
	// sum elements
	JsDeclareFunction(sum);
	// outer product
	JsDeclareFunction(outer);
	// inner product
	JsDeclareFunction(inner);
	// add vectors
	JsDeclareFunction(plus);
	// subtract vectors
	JsDeclareFunction(minus);
	// scalar multiply
	JsDeclareFunction(multiply);
	// INPLACE : normalizes the vector, returns undefined
	JsDeclareFunction(normalize);
	// gets vector length
	JsDeclareProperty(length);
};

///////////////////////////////
// QMiner-FltVV
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
	static TFltVV& GetFltVV(const v8::Handle<v8::Object> Obj) {
		return TJsFltVVUtil::GetSelf(Obj)->Mat;
	}
	/// template
    static v8::Handle<v8::ObjectTemplate> GetTemplate();
	// get element
	JsDeclareFunction(at);	
	// set element, returns undefined
	JsDeclareFunction(put);
	// matrix x scalar, matrix x vector, matrix x matrix
	JsDeclareFunction(multiply);
	// matrix + matrix
	JsDeclareFunction(plus);
	// matrix - matrix
	JsDeclareFunction(minus);
	// returns the transpose of a matrix
	JsDeclareFunction(transpose);
	// solves a linear system A x = y. Input: y, Output: x
	JsDeclareFunction(solve);
	// INPLACE : changes the matrix by normalizing columns, return undefined
	JsDeclareFunction(normalizeCols);
	// get number of rows
	JsDeclareProperty(rows);
	// get number of columns
	JsDeclareProperty(cols);
};

}

#endif

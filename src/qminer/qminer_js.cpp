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

#include "qminer_js.h"

#ifdef GLib_WIN
	#pragma comment(lib, "winmm.lib")
#endif

namespace TQm {

/////////////////////////////////
// QMiner-JavaScript-Utility
TBool TJsUtil::InitP = false;

void TJsUtil::Init() {
	if (!InitP) {
		v8::HandleScope HandleScope;
		// security token
		if (SecurityToken.IsEmpty()) {
			TStr GuidStr = TGuid::GenGuid();
			v8::Handle<v8::String> GuidV8Str = v8::String::New(GuidStr.CStr());
			SecurityToken = v8::Persistent<v8::String>::New(GuidV8Str);
		}
		// context for parsing
		if (ParseContext.IsEmpty()) {
			ParseContext = v8::Context::New();
			ParseContext->SetSecurityToken(SecurityToken);
		}
		// json parsers
		JsonParser.Dispose();
		JsonString.Dispose();
		v8::Context::Scope ContextScope(ParseContext);
		v8::Handle<v8::Object> Json = ParseContext->Global()->Get(v8::String::New("JSON"))->ToObject();
		JsonParser = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Json->Get(v8::String::New("parse"))));
		JsonString = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Json->Get(v8::String::New("stringify"))));
		InitP = true;
	}
}

v8::Persistent<v8::String> TJsUtil::SecurityToken;
v8::Persistent<v8::Context> TJsUtil::ParseContext;
v8::Persistent<v8::Function> TJsUtil::JsonParser;
v8::Persistent<v8::Function> TJsUtil::JsonString;

void TJsUtil::HandleTryCatch(const v8::TryCatch& TryCatch) {
	v8::HandleScope HandleScope;
	if (!TryCatch.HasCaught()) { return; }
	// get trace
	v8::String::Utf8Value ErrTrace(TryCatch.StackTrace());
	if (ErrTrace.length() > 0 && !TryCatch.StackTrace()->IsUndefined()) {
        TStr ErrorMsg = TStr(*ErrTrace);
		throw TQmExcept::New(ErrorMsg);
	} else {
		v8::Local<v8::Value> Except = TryCatch.Exception();
		if (Except->IsString()) {
			v8::String::Utf8Value ErrStr(Except->ToString());
			throw TQmExcept::New(TStr::Fmt("%s", *ErrStr));
		} else if (Except->IsObject() &&
					!(Except->ToObject()->Get(v8::String::New("message"))->IsUndefined()) &&
					!(Except->ToObject()->Get(v8::String::New("name"))->IsUndefined())) {
			v8::String::Utf8Value Name(Except->ToObject()->Get(v8::String::New("name")));
			v8::String::Utf8Value ErrMsg(Except->ToObject()->Get(v8::String::New("message")));
			throw TQmExcept::New(TStr::Fmt("%s %s", *Name, *ErrMsg));
		} else {
			v8::String::Utf8Value ErrMsg(Except);
			throw TQmExcept::New(TStr::Fmt("%s", *ErrMsg));
		}
	}
}

bool TJsUtil::NamedAccessCheck(v8::Local<v8::Object> Global,
		v8::Local<v8::Value> Name, v8::AccessType Type, v8::Local<v8::Value> Data) {

	return true; 
}

bool TJsUtil::IndexedAccessCheck(v8::Local<v8::Object> Global,
		uint32_t Key, v8::AccessType Type, v8::Local<v8::Value> Data) {

	return true; 
}

v8::Handle<v8::Value> TJsUtil::ParseJson(const PJsonVal& JsonVal) {
	return ParseJson(TJsonVal::GetStrFromVal(JsonVal));
}

v8::Handle<v8::Value> TJsUtil::ParseJson(const TStr& JsonStr) {
	Init(); // initialize if not yet
	v8::HandleScope HandleScope;
	v8::Context::Scope ContextScope(ParseContext);
	v8::Handle<v8::Value> JsonV8Str = v8::String::New(JsonStr.CStr());
	v8::TryCatch TryCatch;
	v8::Handle<v8::Value> Value = JsonParser->Call(ParseContext->Global(), 1, &JsonV8Str);
	if (Value.IsEmpty()) { return HandleScope.Close(TryCatch.Exception()); }
	return HandleScope.Close(Value); 
}

v8::Handle<v8::Value> TJsUtil::V8JsonToV8Str(v8::Handle<v8::Value> Json) {
	Init(); // initialize if not yet
	v8::HandleScope HandleScope;
	v8::Context::Scope ContextScope(ParseContext);
	v8::TryCatch TryCatch;
	v8::Handle<v8::Value> JsonStr = JsonString->Call(ParseContext->Global(), 1, &Json);
	if (JsonStr.IsEmpty()) { return HandleScope.Close(TryCatch.Exception()); }
	return HandleScope.Close(JsonStr); 
}

TStr TJsUtil::V8JsonToStr(v8::Handle<v8::Value> Json) {
	v8::Handle<v8::Value> JsonStr = V8JsonToV8Str(Json);
	v8::String::Utf8Value Utf8(JsonStr);
	return TStr(*Utf8); 
}

v8::Handle<v8::Value> TJsUtil::GetV8Date(const TTm& Tm) {
	v8::HandleScope HandleScope;
	// get milliseconds since 01 January, 1970 UTC
	const double MSecs = (double)TTm::GetDateTimeIntFromTm(Tm) * 1000.0;
	v8::Handle<v8::Value> Date = v8::Date::New(MSecs);
	return HandleScope.Close(Date); 
}

TTm TJsUtil::GetGlibDate(v8::Handle<v8::Value> Date) {
	v8::HandleScope HandleScope;
	// get seconds since 01 January, 1970 UTC
	const int Secs = TFlt::Round(Date->NumberValue() / 1000.0);
	return TTm::GetTmFromDateTimeInt(Secs);
}

v8::Handle<v8::Value> TJsUtil::GetCurrV8Date() {
	v8::HandleScope HandleScope;
	// get milliseconds since 01 January, 1970 UTC
	const double MSecs = (double)TTm::GetDateTimeIntFromTm(TTm::GetCurLocTm()) * 1000.0;
	v8::Handle<v8::Value> Date = v8::Date::New(MSecs);
	return HandleScope.Close(Date);
}

PJsonVal TJsUtil::HttpRqToJson(PHttpRq HttpRq) {
    PJsonVal Request = TJsonVal::NewObj();
    // Headers
    for (int FieldN = 0; FieldN < HttpRq->GetFldValH().Len(); FieldN++) {
        TStr KeyVal = HttpRq->GetFldValH().GetKey(FieldN);
        Request->AddToObj(KeyVal, HttpRq->GetFldValH().GetDat(KeyVal));
    }
    // Method
    Request->AddToObj("method", HttpRq->GetMethodNm());
    // URL
    // - scheme
    Request->AddToObj("scheme", HttpRq->GetUrl()->GetSchemeNm());
    // - path
    Request->AddToObj("path", HttpRq->GetUrl()->GetPathStr());
    // GET arguments
    PJsonVal Args = TJsonVal::NewObj();
    for (int KeyN = 0; KeyN < HttpRq->GetUrlEnv()->GetKeys(); KeyN++) {
        PJsonVal ArgVals = TJsonVal::NewArr();
        TStr ArgKey = HttpRq->GetUrlEnv()->GetKeyNm(KeyN);
        for (int ValN = 0; ValN < HttpRq->GetUrlEnv()->GetVals(ArgKey); ValN++) {
            ArgVals->AddToArr(TJsonVal::NewStr(HttpRq->GetUrlEnv()->GetVal(ArgKey, ValN)));
        }
        Args->AddToObj(ArgKey, ArgVals);
    }
    Request->AddToObj("args", Args);
    // Request Body
    // - raw data
    TStr Data;
    if (HttpRq->IsBody()) {
        Data = HttpRq->GetBodyAsStr();
        TEnv::Debug->OnStatusFmt("Data Received %s ", Data.CStr());
        // parse JSON
        if (HttpRq->IsContType(THttp::AppJSonFldVal)) {
            PJsonVal BodyVal = TJsonVal::GetValFromSIn(HttpRq->GetBodyAsSIn());
            // check we got a valid JSon
            if (BodyVal->IsObj() || BodyVal->IsArr()) {
                // yes
                Request->AddToObj("jsonData", BodyVal);
            }
        }
    }
    Request->AddToObj("data", Data);

    return Request;
}

///////////////////////////////
// QMiner-JavaScript-Directory
TJsFPath::TJsFPath(const TStr& FPath): CanonicalFPath(GetCanonicalPath(FPath)) { }

bool TJsFPath::Equals(const TJsFPath& JsFPath) const {
	return CanonicalFPath == JsFPath.GetFPath();
}

bool TJsFPath::IsSubdir(const TJsFPath& JsFPath) const {
	return CanonicalFPath.IsPrefix(JsFPath.GetFPath());
}

void TJsFPath::GetFPathV(const TStrV& FPathV, TVec<TJsFPath>& JsFPathV) {
	for(TStrV::TIter It = FPathV.BegI(); It != FPathV.EndI(); ++It) {
		JsFPathV.Add(TJsFPath(*It));
	}
}

TStr TJsFPath::GetCanonicalPath(const TStr& FPath) {
	// get absolute path
	TStr AbsFPath = TStr::GetNrAbsFPath(FPath);
	// remove any redundancies
	TStrV CanonV; AbsFPath.SplitOnAllCh('/', CanonV);
	TSStack<TStr> CanonS; TStr CurrStr;
	for (int ElN = 0; ElN < CanonV.Len(); ++ElN) {
		CurrStr = CanonV.GetVal(ElN);
		if (CurrStr == "..") {
			QmAssert(!CanonS.Empty());
			CanonS.Pop();
		} else if (CurrStr != ".") {
			CanonS.Push(CurrStr+"/");
		}
	}
	// assemble the canonical path (from left to right
	QmAssert(!CanonS.Empty());
	// we start with drive letter (Windows) or slash (Unix)
	TChA CanonFPath = AbsFPath.LeftOf('/'); CanonFPath += '/';
	// get the rest of the path
	for (int CanonN = CanonS.Len() - 1; CanonN >= 0; CanonN--) {
		CanonFPath += CanonS[CanonN];
	}
	// done
	return CanonFPath;
}

///////////////////////////////
// QMiner-JavaScript-Userscript
TScript::TScript(const PBase& _Base, const TStr& _ScriptNm, const TStr& _ScriptFNm, 
		const TStrV& _IncludeFPathV, const TVec<TJsFPath>& _AllowedFPathV): ScriptNm(_ScriptNm), 
			ScriptFNm(_ScriptFNm), IncludeFPathV(_IncludeFPathV), AllowedFPathV(_AllowedFPathV),
			Base(_Base){ 
	
	// initialize callbacks for http client
	JsFetch = new TJsFetch(this);
	// initialize script
	Init(); 
}

TWPt<TScript> TScript::GetGlobal(v8::Handle<v8::Context>& Context) {
	v8::HandleScope HandleScope;
    // get global object from context
    v8::Local<v8::Object> Global = v8::Local<v8::Object>::Cast(Context->Global()->GetPrototype());
    // get wrapped object within global object
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Global->GetInternalField(0));
    // cast wrapped object to script
    return TWPt<TScript>(static_cast<TScript*>(WrappedObject->Value()));    
}

TScript::~TScript() {
	v8::HandleScope HandleScope;
	// delete fetcher
	JsFetch.Del();
	// remove context
	Context.Dispose();
	// NOTE. There is no special reason for picking 11 iterations.
	for(int i = 0; i < 11; ++i) { v8::V8::LowMemoryNotification(); }
}

void TScript::RegSrvFun(TSAppSrvFunV& SrvFunV) { 
	TEnv::Logger->OnStatusFmt("Registering %s at /%s/", ScriptFNm.GetFMid().CStr(), ScriptNm.CStr());
	SrvFunV.Add(TQm::TJsSrvFun::New(this, ScriptNm));
}

void TScript::Reload() {
	// clear any existing functions
	JsNmFunH.Clr();
	// clear any existing triggers
	for (int TriggerN = 0; TriggerN < TriggerV.Len(); TriggerN++) {
		const uchar StoreId = TriggerV[TriggerN].Val1;
		PStoreTrigger Trigger = TriggerV[TriggerN].Val2;
		Base->GetStoreByStoreId(StoreId)->DelTrigger(Trigger);
	}
	TriggerV.Clr();
	// reload and reinit
	Init();
}

void TScript::Execute(v8::Handle<v8::Function> Fun) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	Fun->Call(Context->Global(), 0, NULL);
	TJsUtil::HandleTryCatch(TryCatch);
}

void TScript::Execute(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1; v8::Handle<v8::Value> Argv[Argc] = { Arg };
	Fun->Call(Context->Global(), Argc, Argv);
	TJsUtil::HandleTryCatch(TryCatch);
}

void TScript::Execute(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { TJsUtil::ParseJson(JsonVal) };
	Fun->Call(Context->Global(), Argc, Argv);
	TJsUtil::HandleTryCatch(TryCatch);
}

void TScript::Execute(v8::Handle<v8::Function> Fun, const PJsonVal& Arg1, v8::Handle<v8::Object>& Arg2) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 2;
	v8::Handle<v8::Value> Argv[Argc] = { TJsUtil::ParseJson(Arg1), Arg2 };
	Fun->Call(Context->Global(), Argc, Argv);
	TJsUtil::HandleTryCatch(TryCatch);
}

v8::Handle<v8::Value> TScript::ExecuteV8(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { TJsUtil::ParseJson(JsonVal) };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
    return HandleScope.Close(RetVal);
}

bool TScript::ExecuteBool(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Object>& Arg) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { Arg };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
    QmAssertR(RetVal->IsBoolean(), "Return type expected to be boolean");
    return RetVal->BooleanValue();
}

bool TScript::ExecuteBool(v8::Handle<v8::Function> Fun, 
        const v8::Handle<v8::Object>& Arg1, const v8::Handle<v8::Object>& Arg2) {
    
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 2;
	v8::Handle<v8::Value> Argv[Argc] = { Arg1, Arg2 };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
    QmAssertR(RetVal->IsBoolean(), "Return type expected to be boolean");
    return RetVal->BooleanValue();
}

TStr TScript::ExecuteStr(v8::Handle<v8::Function> Fun, const PJsonVal& JsonVal) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { TJsUtil::ParseJson(JsonVal) };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
	// check we got what we expected
	if (RetVal->IsString()) { return TStr(*v8::String::Utf8Value(RetVal)); }
	// else complain
	throw TQmExcept::New("Wrong return type!");
}


void TScript::Execute(v8::Handle<v8::Function> Fun, const TStr& Str) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { v8::String::New(Str.CStr()) };
	Fun->Call(Context->Global(), Argc, Argv);
	TJsUtil::HandleTryCatch(TryCatch);
}

TStr TScript::ExecuteStr(v8::Handle<v8::Function> Fun, const TStr& Str) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { v8::String::New(Str.CStr()) };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
	// check we got what we expected
	if (RetVal->IsString()) { return TStr(*v8::String::Utf8Value(RetVal)); }
	// else complain
	throw TQmExcept::New("Wrong return type!");
}

void TScript::AddSrvFun(const TStr& ScriptNm, const TStr& FunNm, 
		const TStr& Verb, const v8::Persistent<v8::Function>& JsFun) {
		
    //@TODO Make Verb an Array of Verbs
    // Parse FunNm
    PJsonVal Rule = TJsonVal::NewArr();

    PJsonVal BaseRule = TJsonVal::NewObj();
    BaseRule->AddToObj("verb", Verb);
    BaseRule->AddToObj("path", ScriptNm);
    Rule->AddToArr(BaseRule);
    
    if(!FunNm.IsChIn('/')) {
        PJsonVal Path = TJsonVal::NewObj();
        Path->AddToObj("path", FunNm);
        Rule->AddToArr(Path);
    }
    else {
        TStrV KeyValStrV;
        FunNm.SplitOnAllCh('/', KeyValStrV, true);
        for (int KeyValStrN = 0; KeyValStrN < KeyValStrV.Len(); KeyValStrN++){
            TStr KeyValStr = KeyValStrV[KeyValStrN];
            if(KeyValStr.IsChIn('<')) {
                // argument
                KeyValStr.DelSubStr(0,0);
                KeyValStr.DelSubStr(KeyValStr.Len()-1, KeyValStr.Len());
                Rule->GetArrVal(Rule->GetArrVals()-1)->AddToObj("param", KeyValStr);
            }
            else {
                // path
                PJsonVal Path = TJsonVal::NewObj();
                Path->AddToObj("path", KeyValStr);
                Rule->AddToArr(Path);
            }
        }
    }
    InfoLog("Adding Rule: " + Rule->SaveStr());
    SrvFunRuleV.Add(Rule);
    JsNmFunH.AddDat(TInt(SrvFunRuleV.Len()-1), JsFun);
}

void TScript::ExecuteSrvFun(const PHttpRq& HttpRq, v8::Handle<v8::Object>& RespObj) {
	v8::HandleScope HandleScope;
    // Get the Verb
    TStr Verb = HttpRq->GetMethodNm();
    // Get the Path
	PUrl Url = HttpRq->GetUrl();
    const int PathSegs = Url->GetPathSegs();
    //TEnv::Logger->OnStatusFmt("Execute for %s request: %s", Verb.CStr(), Url->GetPathStr().CStr());

    // Search the rules
    int SelectedRule = -1;
    PJsonVal Params;
    bool RuleMatched = false;
    for(int RuleN = 0; RuleN < SrvFunRuleV.Len(); RuleN++) {
        PJsonVal Rule = SrvFunRuleV.GetVal(RuleN);
        // Verb Match?
        if(!(Rule->GetArrVal(0)->GetObjStr("verb") == Verb)) {
            continue; // No
        }
        // Number of Rules possibly match?
        if(Rule->GetArrVals() > PathSegs) {
            continue; // No
        }
        int Seg = 0;
        bool IsMatch = true;
        Params = TJsonVal::NewObj();
        for(int PathN = 0; PathN < Rule->GetArrVals(); PathN++) {
            if(Rule->GetArrVal(PathN)->GetObjStr("path") == Url->GetPathSeg(Seg)) {
                if(Rule->GetArrVal(PathN)->IsObjKey(("param"))) {
                    Seg++;
                    Params->AddToObj(Rule->GetArrVal(PathN)->GetObjStr("param"), Url->GetPathSeg(Seg));
                }
                Seg++;
            }
            else {
                // break and continue outer loop
                IsMatch = false;
                break;
            }
        }
        if(!IsMatch) {
            continue;
        }
        // Is there extra stuff that shouldn't be there?
        if(Seg < PathSegs-1) {
            continue; // try next one
        }
       // If we got here, this should be a perfect match made in heaven
       // that's the hypothesis at least
       RuleMatched = true;
       SelectedRule = RuleN;
       break;
    }
    // Get the function
    // Request
    PJsonVal Request = TJsUtil::HttpRqToJson(HttpRq);
    // @TODO add request args
    Request->AddToObj("params", Params);
    // Exec
    if(RuleMatched && SelectedRule >= 0) {
        // get function
        v8::Persistent<v8::Function> JsFun = JsNmFunH.GetDat(SelectedRule);
        // execute
        try {
            Execute(JsFun, Request, RespObj);
        } catch (const PExcept& Except) {
            InfoLog("Error handling request: " + Except->GetMsgStr());
            throw TQmExcept::New("Error handling request: " + Except->GetMsgStr());
        } catch (...) {
            throw TQmExcept::New("Error handling request: unknown internal Error");
        }
    }
    else {
        // TODO: Not Found
		throw TQmExcept::New("Error handling request: not found");
    }
}

void TScript::AddTrigger(const uchar& StoreId, const PStoreTrigger& Trigger) { 
	TriggerV.Add(TPair<TUCh, PStoreTrigger>(StoreId, Trigger)); 
}

v8::Handle<v8::Value> TScript::require(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // get pointer to the context and script
    v8::Handle<v8::Object> Self = Args.Holder();
    v8::Handle<v8::Context> Context = Self->CreationContext();
	v8::Context::Scope ContextScope(Context);    
    TWPt<TScript> Script = TScript::GetGlobal(Context);
    // get module filename
    TStr ModuleFNm = TJsObjUtil<TScript>::GetArgStr(Args, 0);
    // check if one of built-in modules
    if (ModuleFNm == "geoip") { 
        return TJsGeoIp::New();
    } else if (ModuleFNm == "time") {
        return TJsTm::New();
    }
    // load the source of the module
    InfoLog("Loading " + ModuleFNm);
    TStr ModuleSource = Script->LoadModuleSrc(ModuleFNm);
    // handle JS exceptions
    v8::TryCatch TryCatch;
    // compile the module
    v8::Handle<v8::Script> Module = v8::Script::Compile(v8::String::New(ModuleSource.CStr()));
    // execute the module
    Module->Run();
    // collect the result
    v8::Handle<v8::Function> ModuleFun = v8::Handle<v8::Function>::Cast(
        Context->Global()->Get(v8::String::New("module")));
    v8::Handle<v8::Value> RetVal = ModuleFun->Call(Context->Global(), 0, NULL);
    // handle errors
    TJsUtil::HandleTryCatch(TryCatch);
    // return result
    return HandleScope.Close(RetVal);
}

void TScript::Init() {
    // report on include folders
	InfoLog("Include directories:");
	for (int IncludeFPathN = 0; IncludeFPathN < IncludeFPathV.Len(); IncludeFPathN++) {
		InfoLog("  " + IncludeFPathV[IncludeFPathN]);
	}    
	// report on accessible folders
	InfoLog("Accessible directories:");
	for (int AllowedDirsN = 0; AllowedDirsN < AllowedFPathV.Len(); AllowedDirsN++) {
		InfoLog("  " + AllowedFPathV[AllowedDirsN].GetFPath());
	}
	// do global initialization if not yet done
	v8::HandleScope HandleScope;
	// clear current context
	Context.Dispose(); Context.Clear();
    // create new context
    v8::Handle<v8::ObjectTemplate> GlobalTemp = v8::ObjectTemplate::New();
    GlobalTemp->SetInternalFieldCount(1);
	Context = v8::Context::New(NULL, GlobalTemp);
    // prepare security token
	SecurityToken = TGuid::GenGuid();
	Context->SetSecurityToken(v8::Persistent<v8::String>::New(v8::String::New(SecurityToken.CStr())));
    // get into the context scope
	v8::Context::Scope ContextScope(Context);
    // add pointer to TScript to the global object
    v8::Handle<v8::Object>::Cast(Context->Global()->GetPrototype())->SetInternalField(0, v8::External::New(this));
    // initialize module-loading system
    v8::Local<v8::Function> RequireFun = v8::FunctionTemplate::New(require)->GetFunction();
    Context->Global()->Set(v8::String::New("require"), RequireFun);
	// install core objects
	Install();
	// execute the script    
    Execute(TEnv::QMinerFPath + "qminer.js");
	Execute(ScriptFNm);
}

void TScript::Install() {
	v8::HandleScope HandleScope;
	// delete existing objects, if they exist
	Context->Global()->Delete(v8::String::New("console"));
	Context->Global()->Delete(v8::String::New("qm"));
	Context->Global()->Delete(v8::String::New("fs"));
	Context->Global()->Delete(v8::String::New("http"));
	Context->Global()->Delete(v8::String::New("linalg"));
	// create fresh ones
    DebugLog("Installing 'console' object");
	Context->Global()->Set(v8::String::New("console"), TJsConsole::New(this));
    DebugLog("Installing 'qm' object");
	Context->Global()->Set(v8::String::New("qm"), TJsBase::New(this));
    DebugLog("Installing 'fs' object");
	Context->Global()->Set(v8::String::New("fs"), TJsFs::New(this));
    DebugLog("Installing 'http' object");
	Context->Global()->Set(v8::String::New("http"), TJsHttp::New(this));
	DebugLog("Installing 'linalg' object");
	Context->Global()->Set(v8::String::New("linalg"), TJsLinAlg::New(this));
}

TStr TScript::LoadSource(const TStr& FNm) {
	THash<TStr, bool> ImportH; // make sure each file is imported only once 
	TSStack<TStr> Headers; // depth-first importing 
	TStr LineStr; int IndexN;
	// import requested files 
	ImportH.AddDat(FNm, true);
	Headers.Push(FNm);
	TChA ScriptSource = "";
	while (!Headers.Empty()) {
		TStr TmpFile(Headers.Top());
        TChA TmpSource;
		Headers.Pop();
		TFIn FIn(TmpFile);
		TStr FPath = TmpFile.GetFPath();
		while (FIn.GetNextLn(LineStr)) {
			IndexN = 0; 
			// skip whitespace 
			while (IndexN < LineStr.Len() && TCh::IsWs(LineStr.GetCh(IndexN))) { ++IndexN; }
			// 10 = length of (import + quotes + space + at least one character)
			if (LineStr.Len() >= IndexN+10 && LineStr.GetSubStr(IndexN, IndexN+6) == "import ") {
                InfoLog("Warning: 'import' will be deprecated, use require to load libraries");
				// extract library path, assuming no syntax errors in the userscript 
				TStr LibNm = LineStr.RightOf('"').LeftOf('"');
                if (LibNm == "util.js") { 
                    InfoLog("Warning: util.js is no longer needed, now included by default");
                    continue; 
                }
				TStr LibFNm = GetLibFNm(LibNm);
				if (!ImportH.IsKey(LibFNm)) {
					ImportH.AddDat(LibFNm, true); Headers.Push(LibFNm); 
				} else {
					//TEnv::Logger->OnStatusFmt("[%s] Already imported: '%s'", TmpFile.CStr(), LibFNm.CStr());
				}
			} else { 
				TmpSource += LineStr + "\n";
			}
		}
        TmpSource += ScriptSource;
		ScriptSource = TmpSource; TmpSource = "";
	}
    return ScriptSource;
}

void TScript::Execute(const TStr& FNm) {
	v8::HandleScope HandleScope;
	v8::Context::Scope ContextScope(Context);
	v8::TryCatch TryCatch;
    // load script source
    TStr Source = LoadSource(FNm);
	// compile script
    v8::Handle<v8::Script> Script = v8::Script::Compile(
        v8::String::New(Source.CStr()), 
        v8::String::New(FNm.GetFBase().CStr()));
	// check if compilation failed
	TJsUtil::HandleTryCatch(TryCatch);
	// run script
    Script->Run();
	// check if ran successfully
	TJsUtil::HandleTryCatch(TryCatch);
}

TStr TScript::LoadModuleSrc(const TStr& ModuleFNm) {
    TChA ModulChA;
    ModulChA += "function module(){var exports={};";
    ModulChA += TStr::LoadTxt(GetLibFNm(ModuleFNm));
    ModulChA += ";return exports;}";
	return ModulChA;
}

TStr TScript::GetLibFNm(const TStr& LibNm) {
	for (int FPathN = 0; FPathN < IncludeFPathV.Len(); FPathN++) {
		TStr FNm = IncludeFPathV[FPathN] + LibNm;
		if (TFile::Exists(FNm)) { return FNm; }
	}
    ErrorLog("Library " + LibNm + " not found in any include directories");
	throw TQmExcept::New("Library " + LibNm + " not found in any include directories");
}

///////////////////////////////
// QMiner-JavaScript-Server-Function
void TJsSrvFun::Exec(const TStrKdV& FldNmValKdV, const PSAppSrvRqEnv& RqEnv) {
	PHttpRq HttpRq = RqEnv->GetHttpRq();
	PUrl Url = HttpRq->GetUrl();

    // leaving this admin stuff untouched from previous release: @TODO fix
    // extract function name from URL
    // Get the URL used which may include variables
    const int PathSegs = Url->GetPathSegs();
	TStr FunNm = (PathSegs >= 2) ? Url->GetPathSeg(1) : "";
	// check if in admin mode or normal callback
	if (FunNm == "admin") {
		QmAssertR(PathSegs >= 3, "No admin parameters in URL " + Url->GetUrlStr());
		TStr AdminMode = Url->GetPathSeg(2);
        int StatusCode = THttp::ErrNotFoundStatusCd;
		if (AdminMode == "reload") {
			Js->Reload();
            StatusCode = THttp::OkStatusCd;
		} else {
			// what else can be done?
		}
		// prepare response
		PHttpResp HttpResp = THttpResp::New(StatusCode, 
			THttp::TextPlainFldVal, false, TMIn::New(AdminMode));
        // send response
       	RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp); 
	} else if (FunNm.Empty()) {
        // no parameters, just list the rules
        PJsonVal SrvFunRules = Js->GetSrvFunRules();
		// prepare response
		PHttpResp HttpResp = THttpResp::New(THttp::OkStatusCd, THttp::AppJSonFldVal, 
            false, TMIn::New(TJsonVal::GetStrFromVal(SrvFunRules)));
        // send response
       	RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp);         
    } else {
        // prepare response object
    	v8::HandleScope HandleScope;
    	v8::Context::Scope ContextScope(Js->Context);        
        v8::Handle<v8::Object> HttpResp = TJsHttpResp::New(RqEnv->GetWebSrv(), RqEnv->GetSockId());
		// call appropriate JavaScript function
		Js->ExecuteSrvFun(HttpRq, HttpResp);
	}		
}	

///////////////////////////////
// QMiner-JavaScript-Store-Trigger
TJsStoreTrigger::TJsStoreTrigger(TWPt<TScript> _Js, v8::Handle<v8::Object> TriggerVal): Js(_Js) {
	v8::HandleScope HandleScope;
	if (TriggerVal->Has(v8::String::New("onAdd"))) {
		v8::Handle<v8::Value> _OnAddFun = TriggerVal->Get(v8::String::New("onAdd"));
		QmAssert(_OnAddFun->IsFunction());
		OnAddFun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(_OnAddFun));
	}
	if (TriggerVal->Has(v8::String::New("onUpdate"))) {
		v8::Handle<v8::Value> _OnUpdateFun = TriggerVal->Get(v8::String::New("onUpdate"));
		QmAssert(_OnUpdateFun->IsFunction());
		OnUpdateFun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(_OnUpdateFun));
	}
	if (TriggerVal->Has(v8::String::New("onDelete"))) {
		v8::Handle<v8::Value> _OnDeleteFun = TriggerVal->Get(v8::String::New("onDelete"));
		QmAssert(_OnDeleteFun->IsFunction());
		OnDeleteFun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(_OnDeleteFun));
	}
}

void TJsStoreTrigger::OnAdd(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId) {
	v8::HandleScope HandleScope;
	if (!OnAddFun.IsEmpty()) { 
		Js->Execute(OnAddFun, TJsRec::New(Js, Store->GetRec(RecId)));
	}
}

void TJsStoreTrigger::OnUpdate(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId) {
	v8::HandleScope HandleScope;
	if (!OnUpdateFun.IsEmpty()) { 
		Js->Execute(OnUpdateFun, TJsRec::New(Js, Store->GetRec(RecId)));
	}
}

void TJsStoreTrigger::OnDelete(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const uint64& RecId) {
	v8::HandleScope HandleScope;
	if (!OnDeleteFun.IsEmpty()) { 
		Js->Execute(OnDeleteFun, TJsRec::New(Js, Store->GetRec(RecId)));
	}
}

///////////////////////////////
// QMiner-JavaScript-WebPgFetch
void TJsFetch::OnFetch(const int& FId, const PWebPg& WebPg) {
	// execute callback
	{
		const TJsFetchRq& Rq = CallbackH.GetDat(FId);
		//TEnv::Logger->OnStatusFmt("OnFetch[%s]", Rq.GetUrlStr().CStr());
		// is there a callback to call
		if (Rq.IsOnSuccess()) {
			// there is, do we have any data
			if (Rq.IsJson()) {
				// parse
				try {
					// parse json
					PJsonVal JsonVal = TJsonVal::GetValFromSIn(WebPg->GetHttpResp()->GetBodyAsSIn());
					// execute callback
					Js->Execute(Rq.GetOnSuccess(), JsonVal);
				} catch (const PExcept& Except) {
					// call error handler on error
					if (Rq.IsOnError()) {
						Js->Execute(Rq.GetOnError(), "Error parsing return data: " + Except->GetMsgStr());
					}
				}
			} else {
				// no data, just notify on success
				Js->Execute(Rq.GetOnSuccess(), WebPg->GetHttpBodyAsStr());
			}
		}
	}
	CallbackH.DelIfKey(FId);
}

void TJsFetch::OnError(const int& FId, const TStr& MsgStr) {
	// execute callback
	{
		const TJsFetchRq& Rq = CallbackH.GetDat(FId);
		//TEnv::Logger->OnStatusFmt("OnError[%s]: %s", Rq.GetUrlStr().CStr(), MsgStr.CStr());
		// is there a callback to call
		if (Rq.IsOnError()) {
			Js->Execute(Rq.GetOnError(), "Error fetching: " + MsgStr);
		}
	}
	CallbackH.DelIfKey(FId);
}

void TJsFetch::Fetch(const TJsFetchRq& Rq) {
	const int FId = FetchHttpRq(Rq.GetHttpRq());
	CallbackH.AddDat(FId, Rq);
}

///////////////////////////////
// QMiner-JavaScript-Console
v8::Handle<v8::ObjectTemplate> TJsConsole::GetTemplate() {
	v8::HandleScope HandleScope;
    v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
    JsRegisterFunction(TmpTemp, say);
    JsLongRegisterFunction(TmpTemp, "log", say);
    TmpTemp->SetInternalFieldCount(1);
    return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

v8::Handle<v8::Value> TJsConsole::say(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	if (Args.Length() == 1) {      
		const TStr MsgStr = TJsConsoleUtil::GetArgStr(Args, 0);
		InfoLog("[console] " + MsgStr);
	} else if (Args.Length() > 1) {
		const TStr TitleStr = TJsConsoleUtil::GetArgStr(Args, 0);
		const TStr MsgStr = TJsConsoleUtil::GetArgStr(Args, 1);
		TEnv::Logger->OnStatusFmt("[%s] %s", TitleStr.CStr(), MsgStr.CStr());
	} else {
		InfoLog("[console]");
	}
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Base

TJsBase::TJsBase(TWPt<TScript> _Js): Js(_Js), Base(_Js->Base) { }

v8::Handle<v8::ObjectTemplate> TJsBase::GetTemplate() {
	v8::HandleScope HandleScope;
	v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
	JsRegisterProperty(TmpTemp, args);
	JsRegisterProperty(TmpTemp, analytics);
	JsRegisterFunction(TmpTemp, store);
	JsRegisterFunction(TmpTemp, getStoreList);
	JsRegisterFunction(TmpTemp, createStore);
	JsRegisterFunction(TmpTemp, search);
	JsLongRegisterFunction(TmpTemp, "operator", op);
	JsRegisterFunction(TmpTemp, gc);
	TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
	TmpTemp->SetInternalFieldCount(1);
	return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

/*v8::Handle<v8::Value> TJsBase::correlation(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return TJsCorrelation::New();
}*/

v8::Handle<v8::Value> TJsBase::args(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	PJsonVal ArgsVal = TJsonVal::NewArr(Env.GetArgV());
	return HandleScope.Close(TJsUtil::ParseJson(ArgsVal));
}

v8::Handle<v8::Value> TJsBase::analytics(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Info);
	return TJsAnalytics::New(JsBase->Js);
}

v8::Handle<v8::Value> TJsBase::store(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	const TStr StoreNm = TJsBaseUtil::GetArgStr(Args, 0);
	if (JsBase->Base->IsStoreNm(StoreNm)) {
		return TJsStore::New(JsBase->Js, JsBase->Base->GetStoreByStoreNm(StoreNm));
	} else {
		return HandleScope.Close(v8::Null());
	}
}

/// Create stores from the JS interface
v8::Handle<v8::Value> TJsBase::getStoreList(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
    TJsonValV StoreValV;
    const int Stores = JsBase->Base->GetStores();
    for (int StoreN = 0; StoreN < Stores; StoreN++) {
		TWPt<TStore> Store = JsBase->Base->GetStoreByStoreN(StoreN);
        StoreValV.Add(JsBase->Base->GetStoreJson(Store));
    }
    PJsonVal JsonVal = TJsonVal::NewArr(StoreValV);
    return HandleScope.Close(TJsUtil::ParseJson(JsonVal));
}


/// Create stores from the JS interface
v8::Handle<v8::Value> TJsBase::createStore(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
        PJsonVal SchemaVal = TJsBaseUtil::GetArgJson(Args, 0);
    // check we can write
    QmAssertR(!JsBase->Base->IsRdOnly(), "Base opened as read-only");
    //TODO: ability to pass store sizes via function parameter
    // for now 1GB is assumed for each store
    TStorage::CreateStoresFromSchema(JsBase->Base, SchemaVal, (uint64)TInt::Giga);   
    return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsBase::search(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	try {
		TStr QueryStr = TJsBaseUtil::GetArgJsonStr(Args, 0);
		// execute the query
		PRecSet RecSet = JsBase->Base->Search(QueryStr);
		// return results
		return TJsRecSet::New(JsBase->Js, RecSet);
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return HandleScope.Close(v8::Null());
}
	
v8::Handle<v8::Value> TJsBase::op(const v8::Arguments& Args) {	
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);	
	try {		
		PRecSet InRecSet = TJsRecSet::GetArgRecSet(Args, 0);
		PJsonVal ParamVal = TJsBaseUtil::GetArgJson(Args, 1);
		// execute the query
		TRecSetV RecSetV; JsBase->Base->Operator(TRecSetV::GetV(InRecSet), ParamVal, RecSetV);		
		// return results
		v8::Local<v8::Array> JsRecSetV = v8::Array::New(RecSetV.Len());
		for (int RsN = 0; RsN < RecSetV.Len(); RsN++) {
			JsRecSetV->Set(v8::Number::New(RsN), TJsRecSet::New(JsBase->Js, RecSetV[RsN]));
		} 
		return HandleScope.Close(JsRecSetV);		
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsBase::gc(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	try {
		JsBase->Base->GarbageCollect();
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Store
v8::Persistent<v8::ObjectTemplate> TJsStore::Template;

v8::Handle<v8::ObjectTemplate> TJsStore::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, name);
		JsRegisterProperty(TmpTemp, empty);
		JsRegisterProperty(TmpTemp, length);
		JsRegisterProperty(TmpTemp, recs);
		JsRegisterProperty(TmpTemp, fields);
		JsRegisterProperty(TmpTemp, joins);
		JsRegisterProperty(TmpTemp, keys);
		JsRegIndexedProperty(TmpTemp, indexId);
		JsRegisterFunction(TmpTemp, rec);
		JsRegisterFunction(TmpTemp, add);
		JsRegisterFunction(TmpTemp, sample);
		JsRegisterFunction(TmpTemp, key);
		JsRegisterFunction(TmpTemp, addTrigger);
        JsRegisterFunction(TmpTemp, addStreamAggr);
        JsRegisterFunction(TmpTemp, getStreamAggr);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsStore::name(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Local<v8::String> StoreName = v8::String::New(JsStore->Store->GetStoreNm().CStr());
	return HandleScope.Close(StoreName);
}

v8::Handle<v8::Value> TJsStore::empty(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Handle<v8::Boolean> IsEmpty = v8::Boolean::New(JsStore->Store->Empty());
	return HandleScope.Close(IsEmpty);
}

v8::Handle<v8::Value> TJsStore::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Local<v8::Integer> Recs = v8::Integer::New((int)JsStore->Store->GetRecs());
	return HandleScope.Close(Recs);
}

v8::Handle<v8::Value> TJsStore::recs(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	PRecSet ResultSet = JsStore->Store->GetAllRecs();
	return TJsRecSet::New(JsStore->Js, ResultSet); 
}

v8::Handle<v8::Value> TJsStore::fields(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Local<v8::Array> FieldNmV = v8::Array::New(JsStore->Store->GetFields());
	for (int FieldN = 0; FieldN < JsStore->Store->GetFields(); FieldN++) {
		FieldNmV->Set(v8::Number::New(FieldN), v8::String::New(JsStore->Store->GetFieldNm(FieldN).CStr()));
	}	
	return HandleScope.Close(FieldNmV);
}

v8::Handle<v8::Value> TJsStore::joins(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Local<v8::Array> JoinNmV = v8::Array::New(JsStore->Store->GetJoins());
	for (int JoinN = 0; JoinN < JsStore->Store->GetJoins(); JoinN++) {
		JoinNmV->Set(v8::Number::New(JoinN), v8::String::New(JsStore->Store->GetJoinNm(JoinN).CStr()));
	}
	return HandleScope.Close(JoinNmV);
}

v8::Handle<v8::Value> TJsStore::keys(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	TWPt<TIndexVoc> IndexVoc = JsStore->Js->Base->GetIndexVoc();
	TIntSet KeySet = IndexVoc->GetStoreKeys(JsStore->Store->GetStoreId());	
	v8::Local<v8::Array> KeyNmV = v8::Array::New(KeySet.Len());
	int KeySetId = KeySet.FFirstKeyId(), KeyN = 0;
	while (KeySet.FNextKeyId(KeySetId)){	
		KeyNmV->Set(v8::Number::New(KeyN), TJsIndexKey::New(JsStore->Js, IndexVoc->GetKey(KeyN)));		
		KeyN++;
	}	
	return HandleScope.Close(KeyNmV);
}

v8::Handle<v8::Value> TJsStore::indexId(uint32_t Index, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	const uint64 RecId = (uint64)Index;
	if (JsStore->Store->IsRecId(RecId)) {
		return TJsRec::New(JsStore->Js, JsStore->Store->GetRec(RecId));
	} else {
		return HandleScope.Close(v8::Null());
	}
}

v8::Handle<v8::Value> TJsStore::rec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const TStr RecNm = TJsStoreUtil::GetArgStr(Args, 0);
	if (JsStore->Store->IsRecNm(RecNm)) {
		return TJsRec::New(JsStore->Js, JsStore->Store->GetRec(RecNm));
	} else {
		return HandleScope.Close(v8::Null());
	}
}

v8::Handle<v8::Value> TJsStore::add(const v8::Arguments& Args) {
	//TODO check if !Base->IsRdOnly() and GenericBase != NULL
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	try {
		// check we can write
		QmAssertR(!JsStore->Js->Base->IsRdOnly(), "Base opened as read-only");
		// get parameters
		PJsonVal RecVal = TJsStoreUtil::GetArgJson(Args, 0);
		// add record
        TWPt<TStorage::TStoreImpl> Store((TStorage::TStoreImpl*)JsStore->Store());
        const uint64 RecId = Store->AddRec(RecVal);
		v8::Local<v8::Integer> _RecId = v8::Integer::New((int)RecId);
		return HandleScope.Close(_RecId);
	} catch (const PExcept& Except) {
		throw TQmExcept::New("[except] " + Except->GetMsgStr());
	}
	return v8::Null();
}

v8::Handle<v8::Value> TJsStore::sample(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const int SampleSize = TJsStoreUtil::GetArgInt32(Args, 0);
	PRecSet ResultSet = JsStore->Store->GetRndRecs(SampleSize);
	return TJsRecSet::New(JsStore->Js, ResultSet); 
}

v8::Handle<v8::Value> TJsStore::key(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const TStr KeyNm = TJsStoreUtil::GetArgStr(Args, 0);
	TWPt<TIndexVoc> IndexVoc = JsStore->Js->Base->GetIndexVoc();
	if (IndexVoc->IsKeyNm(JsStore->Store->GetStoreId(), KeyNm)) {	 
		TIndexKey Key = IndexVoc->GetKey(IndexVoc->GetKeyId(JsStore->Store->GetStoreId(), KeyNm));
		return HandleScope.Close(TJsIndexKey::New(JsStore->Js, Key));				
	}	
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsStore::addTrigger(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	// parse parameters
	QmAssert(Args.Length() == 1);
	v8::Handle<v8::Value> TriggerVal = Args[0];
	QmAssert(TriggerVal->IsObject());
	// add trigger
	PStoreTrigger Trigger = TJsStoreTrigger::New(JsStore->Js, TriggerVal->ToObject());
	// add to store
	JsStore->Store->AddTrigger(Trigger);
	// add to JS context, in case we reload
	JsStore->Js->TriggerV.Add(TPair<TUCh, PStoreTrigger>(JsStore->Store->GetStoreId(), Trigger));
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsStore::addStreamAggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
    // get parameter
    TStr TypeNm = TJsStoreUtil::GetArgStr(Args, 0);
    PJsonVal ParamVal = TJsStoreUtil::GetArgJson(Args, 1);
    // add store parameter
    ParamVal->NewObj("store", JsStore->Store->GetStoreNm());
    // create new aggregate
    PStreamAggr StreamAggr = TStreamAggr::New(JsStore->Js->Base, TypeNm, ParamVal);
    JsStore->Js->Base->AddStreamAggr(JsStore->Store->GetStoreId(), StreamAggr);
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsStore::getStreamAggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const TStr AggrNm = TJsStoreUtil::GetArgStr(Args, 0);
	const int Limit = TJsStoreUtil::GetArgInt32(Args, 1, 100);
	// TODO
    TWPt<TBase> Base = JsStore->Js->Base;
    const uchar StoreId = JsStore->Store->GetStoreId();
	if (Base->IsStreamAggr(StoreId, AggrNm)) {
        PStreamAggr StreamAggr = Base->GetStreamAggr(StoreId, AggrNm);
		PJsonVal StreamAggrVal = StreamAggr->SaveJson(Limit);
		return HandleScope.Close(TJsUtil::ParseJson(StreamAggrVal));
	}
	return HandleScope.Close(v8::Null());
}

///////////////////////////////
// JavaScript Record Comparator
bool TJsRecCmp::operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
	v8::HandleScope HandleScope;
    // prepare record objects
    v8::Persistent<v8::Object> JsRec1 = TJsRec::New(Js, TRec(Store, RecIdWgt1.Key), RecIdWgt1.Dat);
    v8::Persistent<v8::Object> JsRec2 = TJsRec::New(Js, TRec(Store, RecIdWgt2.Key), RecIdWgt2.Dat);
    // call JavaScript Comparator
    return Js->ExecuteBool(CmpFun, JsRec1, JsRec2);
}

///////////////////////////////
// JavaScript Record Filter
bool TJsRecFilter::operator()(const TUInt64IntKd& RecIdWgt) const {
	v8::HandleScope HandleScope;
    // prepare record objects
    v8::Persistent<v8::Object> JsRec = TJsRec::New(Js, TRec(Store, RecIdWgt.Key), RecIdWgt.Dat);
    // call JavaScript Comparator
    return Js->ExecuteBool(FilterFun, JsRec);
}

///////////////////////////////
// QMiner-JavaScript-Record-Set
v8::Persistent<v8::ObjectTemplate> TJsRecSet::Template;

TJsRecSet::TJsRecSet(TWPt<TScript> _Js, const PRecSet& _RecSet):
    Js(_Js), Store(_RecSet->GetStore()), RecSet(_RecSet) { RecSet->FilterByExists(); }

v8::Persistent<v8::Object> TJsRecSet::New(TWPt<TScript> Js, const PRecSet& RecSet) { 
    return TJsRecSetUtil::New(new TJsRecSet(Js, RecSet));
}

PRecSet TJsRecSet::GetArgRecSet(const v8::Arguments& Args, const int& ArgN) {
    v8::HandleScope HandleScope;
    // check we have the argument at all
    AssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    // check it's of the right type
    AssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be Object", ArgN));
    // get the wrapped 
    v8::Handle<v8::Object> RecordSet = v8::Handle<v8::Object>::Cast(Val);
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RecordSet->GetInternalField(0));
    // cast it to record set
    TJsRecSet* JsRecSet = static_cast<TJsRecSet*>(WrappedObject->Value());
    return JsRecSet->RecSet;
}
    
v8::Handle<v8::ObjectTemplate> TJsRecSet::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, store);
		JsRegisterProperty(TmpTemp, length);
		JsRegisterProperty(TmpTemp, empty);
		JsRegisterProperty(TmpTemp, weighted);
		JsRegIndexedProperty(TmpTemp, indexId);
		JsRegisterFunction(TmpTemp, clone);
		JsRegisterFunction(TmpTemp, join);
		JsRegisterFunction(TmpTemp, aggr);
		JsRegisterFunction(TmpTemp, trunc);
		JsRegisterFunction(TmpTemp, sample);
		JsRegisterFunction(TmpTemp, shuffle);
		JsRegisterFunction(TmpTemp, reverse);
		JsRegisterFunction(TmpTemp, sortById);
		JsRegisterFunction(TmpTemp, sortByFq);
		JsRegisterFunction(TmpTemp, sortByField);
		JsRegisterFunction(TmpTemp, sort);
		JsRegisterFunction(TmpTemp, filterById);
		JsRegisterFunction(TmpTemp, filterByFq);
		JsRegisterFunction(TmpTemp, filterByField);
		JsRegisterFunction(TmpTemp, filter);
		JsRegisterFunction(TmpTemp, toJSON);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsRecSet::store(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	const TWPt<TStore>& Store = JsRecSet->RecSet->GetStore();
	return HandleScope.Close(TJsStore::New(JsRecSet->Js, Store));
}

v8::Handle<v8::Value> TJsRecSet::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	v8::Local<v8::Integer> Recs = v8::Integer::New(JsRecSet->RecSet->GetRecs());
	return HandleScope.Close(Recs);
}

v8::Handle<v8::Value> TJsRecSet::empty(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	v8::Handle<v8::Boolean> IsEmpty = v8::Boolean::New(JsRecSet->RecSet->Empty());
	return HandleScope.Close(IsEmpty);
}

v8::Handle<v8::Value> TJsRecSet::weighted(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	v8::Handle<v8::Boolean> IsWgt = v8::Boolean::New(JsRecSet->RecSet->IsWgt());
	return HandleScope.Close(IsWgt);
}

v8::Handle<v8::Value> TJsRecSet::indexId(uint32_t Index, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	const int RecN = Index;
	if (0 <= RecN && RecN < JsRecSet->RecSet->GetRecs()) {
		const uint64 RecId = JsRecSet->RecSet->GetRecId(RecN);
		if (JsRecSet->Store->IsRecId(RecId)) { 
			return TJsRec::New(JsRecSet->Js, JsRecSet->RecSet->GetRec(RecN), JsRecSet->RecSet->GetRecFq(RecN));
		} else {
			return HandleScope.Close(v8::Null());
		}
	} else {
		return HandleScope.Close(v8::Null());
	}
}

v8::Handle<v8::Value> TJsRecSet::clone(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	PRecSet RecSet = JsRecSet->RecSet->Clone();
	return TJsRecSet::New(JsRecSet->Js, RecSet);
}

v8::Handle<v8::Value> TJsRecSet::join(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	TStr JoinNm = TJsRecSetUtil::GetArgStr(Args, 0);
	const int SampleSize = TJsRecSetUtil::GetArgInt32(Args, 1, -1);
	PRecSet RecSet = JsRecSet->RecSet->DoJoin(JsRecSet->Js->Base, JoinNm, SampleSize);
	return TJsRecSet::New(JsRecSet->Js, RecSet);
}

v8::Handle<v8::Value> TJsRecSet::aggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	if (Args.Length() == 0) { 
		// asking for existing aggregates
		PJsonVal AggrVal = JsRecSet->RecSet->GetAggrJson();
		return HandleScope.Close(TJsUtil::ParseJson(AggrVal));
	} else {
		// parameters for computing new aggregate
		PRecSet RecSet = JsRecSet->RecSet;
		PJsonVal AggrVal = TJsRecSetUtil::GetArgJson(Args, 0);
		const TWPt<TBase>& Base = JsRecSet->Js->Base;
		const TWPt<TStore>& Store = JsRecSet->RecSet->GetStore();
		TQueryAggrV QueryAggrV; TQueryAggr::LoadJson(Base, Store, AggrVal, QueryAggrV);
		// if recset empty, not much to do
		if (RecSet->Empty()) { HandleScope.Close(v8::Null()); }
		// compute new aggregates
		v8::Local<v8::Array> AggrValV = v8::Array::New(QueryAggrV.Len());
		for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
			const TQueryAggr& QueryAggr = QueryAggrV[QueryAggrN];
			// compute aggregate
			PAggr Aggr = TAggr::New(Base, RecSet, QueryAggr);
			// serialize to json
			AggrValV->Set(QueryAggrN, TJsUtil::ParseJson(Aggr->SaveJson()));
		}
		// return aggregates
		if (AggrValV->Length() == 1) { 
			// if only one, return as object
			if (AggrValV->Get(0)->IsObject()) {
				return AggrValV->Get(0);
			} else {
				return HandleScope.Close(v8::Null());
			}
		} else {
			// otherwise return as array
			if (AggrValV->IsArray()) {
				return HandleScope.Close(AggrValV);
			} else {
				return HandleScope.Close(v8::Null());
			}
		}
	}
}

v8::Handle<v8::Value> TJsRecSet::trunc(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int Recs = TJsRecSetUtil::GetArgInt32(Args, 0);
	JsRecSet->RecSet->Trunc(Recs);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::sample(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int SampleSize = TJsRecSetUtil::GetArgInt32(Args, 0);
	PRecSet ResultSet = JsRecSet->RecSet->GetSampleRecSet(SampleSize, JsRecSet->RecSet->IsWgt());
	return New(JsRecSet->Js, ResultSet); 
}

v8::Handle<v8::Value> TJsRecSet::shuffle(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int RndSeed = TJsRecSetUtil::GetArgInt32(Args, 0, 0);
	TRnd Rnd(RndSeed);
	JsRecSet->RecSet->Shuffle(Rnd);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::reverse(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	JsRecSet->RecSet->Reverse();
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::sortById(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	JsRecSet->RecSet->SortById(Asc);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::sortByFq(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	JsRecSet->RecSet->SortByFq(Asc);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::sortByField(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const TStr SortFieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	const int SortFieldId = JsRecSet->Store->GetFieldId(SortFieldNm);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 1, 0) > 0);
	JsRecSet->RecSet->SortByField(Asc, SortFieldId);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::sort(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    QmAssertR(Args.Length() == 1, "sort(..) expects one argument.");
    v8::Persistent<v8::Function> CmpFun = TJsRecSetUtil::GetArgFunPer(Args, 0);   
	JsRecSet->RecSet->SortCmp(TJsRecCmp(JsRecSet->Js, JsRecSet->Store, CmpFun));
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::filterById(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    if (Args.Length() == 1) {
        // we expect an array of IDs which we filter out
        PJsonVal ParamVal = TJsRecSetUtil::GetArgJson(Args, 0);
        if(!ParamVal->IsArr()) {
            throw TQmExcept::New("Expected Array.");
        }
        TUInt64Set RecIdSet;
        for (int ArrValN = 0; ArrValN < ParamVal->GetArrVals(); ArrValN++) {
            PJsonVal Val = ParamVal->GetArrVal(ArrValN);
            uint64 RecId = (uint64) Val->GetNum();
            RecIdSet.AddKey(RecId);
        }
        JsRecSet->RecSet->FilterByRecIdSet(RecIdSet);
    } else if (Args.Length() == 2) {
        // we expect min and max ID
        const int MnRecId = TJsRecSetUtil::GetArgInt32(Args, 0);
        const int MxRecId = TJsRecSetUtil::GetArgInt32(Args, 1);
        JsRecSet->RecSet->FilterByRecId((uint64)MnRecId, (uint64)MxRecId);
    }
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::filterByFq(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int MnFq = TJsRecSetUtil::GetArgInt32(Args, 0);
	const int MxFq = TJsRecSetUtil::GetArgInt32(Args, 1);
	JsRecSet->RecSet->FilterByFq(MnFq, MxFq);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::filterByField(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    // get field
	const TStr FieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	const int FieldId = JsRecSet->Store->GetFieldId(FieldNm);
    const TFieldDesc& Desc = JsRecSet->Store->GetFieldDesc(FieldId);
    // parse filter according to field type
    if (Desc.IsInt()) {
        const int MnVal = TJsRecSetUtil::GetArgInt32(Args, 1);
        const int MxVal = TJsRecSetUtil::GetArgInt32(Args, 2);
        JsRecSet->RecSet->FilterByFieldInt(FieldId, MnVal, MxVal);
    } else if (Desc.IsStr() && TJsRecSetUtil::IsArgStr(Args, 1)) {
        TStr StrVal = TJsRecSetUtil::GetArgStr(Args, 1);
        JsRecSet->RecSet->FilterByFieldStr(FieldId, StrVal);
    } else if (Desc.IsFlt()) {
        const double MnVal = TJsRecSetUtil::GetArgFlt(Args, 1);
        const double MxVal = TJsRecSetUtil::GetArgFlt(Args, 2);
        JsRecSet->RecSet->FilterByFieldFlt(FieldId, MnVal, MxVal);
    } else if (Desc.IsTm()) {
        const TStr MnTmStr = TJsRecSetUtil::GetArgStr(Args, 1);
        const uint64 MnTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MnTmStr, '-', ':', '.', 'T'));
        if (TJsRecSetUtil::IsArg(Args, 2)) {
            // we have upper limit
            const TStr MxTmStr = TJsRecSetUtil::GetArgStr(Args, 2);
            const uint64 MxTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MxTmStr, '-', ':', '.', 'T'));
            JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, MxTmMSecs);
        } else {
            // we do not have upper limit
            JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, TUInt64::Mx);
        }
    } else {
        throw TQmExcept::New("Unsupported filed type for record set filtering: " + Desc.GetFieldTypeStr());
    }
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::filter(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    QmAssertR(Args.Length() == 1, "filter(..) expects one argument.");
    v8::Persistent<v8::Function> FilterFun = TJsRecSetUtil::GetArgFunPer(Args, 0);   
	JsRecSet->RecSet->FilterBy(TJsRecFilter(JsRecSet->Js, JsRecSet->Store, FilterFun));
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsRecSet::toJSON(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int MxHits = -1;
	const int Offset = 0;
	const bool JoinRecsP = TJsRecSetUtil::IsArg(Args, 0) ? 
		(TJsRecSetUtil::IsArgBool(Args, 0) ? TJsRecSetUtil::GetArgBool(Args, 0, false) : false) : false;
	const bool JoinRecFieldsP = TJsRecSetUtil::IsArg(Args, 1) ? 
		(TJsRecSetUtil::IsArgBool(Args, 1) ? TJsRecSetUtil::GetArgBool(Args, 1, false) : false) : false;
	const bool FieldsP = true;
	const bool AggrsP = false;
    const bool StoreInfoP = false;
    PJsonVal JsObj = JsRecSet->RecSet->GetJson(JsRecSet->Js->Base, 
		MxHits, Offset, FieldsP, AggrsP, StoreInfoP, JoinRecsP, JoinRecFieldsP);	
	return HandleScope.Close(TJsUtil::ParseJson(JsObj));
}

///////////////////////////////
// QMiner-JavaScript-Record
TVec<v8::Persistent<v8::ObjectTemplate> > TJsRec::TemplateV;

v8::Handle<v8::ObjectTemplate> TJsRec::GetTemplate(const TWPt<TBase>& Base, const TWPt<TStore>& Store) {
	v8::HandleScope HandleScope;
	// initialize template vector on the first call
	if (TemplateV.Empty()) {
		// reserve space for maximal number of stores
		TemplateV.Gen(TUCh::Mx);
	}
	// make sure template id is a valid
    const uchar StoreId = Store->GetStoreId();
	QmAssert(TUCh::Mn <= StoreId && StoreId < TUCh::Mx);
	// initialize template if not already prepared
	if (TemplateV[(int)StoreId].IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsLongRegisterProperty(TmpTemp, "$id", id);
		JsLongRegisterProperty(TmpTemp, "$name", name);
		JsLongRegisterProperty(TmpTemp, "$fq", fq);
		JsRegisterFunction(TmpTemp, toJSON);
		JsRegisterFunction(TmpTemp, addJoin);
		JsRegisterFunction(TmpTemp, delJoin);
		// register all the fields
		for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
            TStr FieldNm = Store->GetFieldDesc(FieldN).GetFieldNm();
            JsLongRegisterProperty(TmpTemp, FieldNm.CStr(), field);
		}
        for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
            const TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
            if (JoinDesc.IsFieldJoin()) {
                JsLongRegisterProperty(TmpTemp, JoinDesc.GetJoinNm().CStr(), sjoin);               
            } else if (JoinDesc.IsIndexJoin()) {
                JsLongRegisterProperty(TmpTemp, JoinDesc.GetJoinNm().CStr(), join);
            } else {
                ErrorLog("Unknown join type " + Store->GetStoreNm() + "." + JoinDesc.GetJoinNm());
            }
        }
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);		
		TemplateV[(int)StoreId] = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return TemplateV[(int)StoreId];
}

TRec TJsRec::GetArgRec(const v8::Arguments& Args, const int& ArgN) { 
    v8::HandleScope HandleScope;
    // check we have the argument at all
    AssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    // check it's of the right type
    AssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be Object", ArgN));
    // get the wrapped 
    v8::Handle<v8::Object> Rec = v8::Handle<v8::Object>::Cast(Val);
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(Rec->GetInternalField(0));
    // cast it to record set
    TJsRec* JsRec = static_cast<TJsRec*>(WrappedObject->Value());
    return JsRec->Rec;
}

v8::Handle<v8::Value> TJsRec::id(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	v8::Local<v8::Integer> RecId = v8::Integer::New((int)(JsRec->Rec.GetRecId()));
	return HandleScope.Close(RecId);
}

v8::Handle<v8::Value> TJsRec::name(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
    TStr RecNm = JsRec->Rec.GetRecNm();
	return HandleScope.Close(v8::String::New(RecNm.CStr()));
}

v8::Handle<v8::Value> TJsRec::fq(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	return HandleScope.Close(v8::Integer::New(JsRec->Fq));
}

v8::Handle<v8::Value> TJsRec::field(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	const TRec& Rec = JsRec->Rec; 
    const TWPt<TStore>& Store = JsRec->Store;
    TStr FieldNm = TJsRecUtil::GetStr(Properties);
	const int FieldId = Store->GetFieldId(FieldNm);
	// check if null
	if (Rec.IsByRef()) {
		if (Store->IsFieldNull(Rec.GetRecId(), FieldId)) { return HandleScope.Close(v8::Null()); }
	} else {
		if (Rec.IsFieldNull(FieldId)) { return HandleScope.Close(v8::Null()); }
	}
	// not null, get value
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		const int Val = Rec.IsByRef() ? Store->GetFieldInt(Rec.GetRecId(), FieldId) : Rec.GetFieldInt(FieldId);
		return HandleScope.Close(v8::Integer::New(Val));
    } else if (Desc.IsIntV()) {
        TIntV IntV; Rec.IsByRef() ? Store->GetFieldIntV(Rec.GetRecId(), FieldId, IntV) : Rec.GetFieldIntV(FieldId, IntV); 
		v8::Handle<v8::Array> JsIntV = v8::Array::New(IntV.Len());
		for (int IntN = 0; IntN < IntV.Len(); IntN++) {
			JsIntV->Set(IntN, v8::Integer::New(IntV[IntN]));
		}
		return HandleScope.Close(JsIntV);
    } else if (Desc.IsUInt64()) {
		const uint64 Val = Rec.IsByRef() ? Store->GetFieldUInt64(Rec.GetRecId(), FieldId) : Rec.GetFieldUInt64(FieldId);
		return HandleScope.Close(v8::Integer::New((int)Val));
	} else if (Desc.IsStr()) {
		const TStr Val = Rec.IsByRef() ? Store->GetFieldStr(Rec.GetRecId(), FieldId) : Rec.GetFieldStr(FieldId);
		return HandleScope.Close(v8::String::New(Val.CStr()));
    } else if (Desc.IsStrV()) {
        TStrV StrV; Rec.IsByRef() ? Store->GetFieldStrV(Rec.GetRecId(), FieldId, StrV) : Rec.GetFieldStrV(FieldId, StrV); 
		v8::Handle<v8::Array> JsStrV = v8::Array::New(StrV.Len());
		for (int StrN = 0; StrN < StrV.Len(); StrN++) {
			JsStrV->Set(StrN, v8::String::New(StrV[StrN].CStr()));
		}
		return HandleScope.Close(JsStrV);
	} else if (Desc.IsBool()) {
		const bool Val = Rec.IsByRef() ? Store->GetFieldBool(Rec.GetRecId(), FieldId) : Rec.GetFieldBool(FieldId);
		return HandleScope.Close(v8::Boolean::New(Val));
	} else if (Desc.IsFlt()) {
		const double Val = Rec.IsByRef() ? Store->GetFieldFlt(Rec.GetRecId(), FieldId) : Rec.GetFieldFlt(FieldId);
		return HandleScope.Close(v8::Number::New(Val));
	} else if (Desc.IsFltPr()) {
		const TFltPr FltPr = Rec.IsByRef() ? Store->GetFieldFltPr(Rec.GetRecId(), FieldId) : Rec.GetFieldFltPr(FieldId);
		v8::Handle<v8::Array> JsFltPr = v8::Array::New(2);
		JsFltPr->Set(0, v8::Number::New(FltPr.Val1));
		JsFltPr->Set(1, v8::Number::New(FltPr.Val2));
		return HandleScope.Close(JsFltPr);    
	} else if (Desc.IsFltV()) {
        TFltV FltV; Rec.IsByRef() ? Store->GetFieldFltV(Rec.GetRecId(), FieldId, FltV) : Rec.GetFieldFltV(FieldId, FltV); 
		v8::Handle<v8::Array> JsFltV = v8::Array::New(FltV.Len());
		for (int FltN = 0; FltN < FltV.Len(); FltN++) {
			JsFltV->Set(FltN, v8::Number::New(FltV[FltN]));
		}
		return HandleScope.Close(JsFltV);
	} else if (Desc.IsTm()) {
        TTm FieldTm; Rec.IsByRef() ? Store->GetFieldTm(Rec.GetRecId(), FieldId, FieldTm) : Rec.GetFieldTm(FieldId, FieldTm); 
		if (FieldTm.IsDef()) { 
			//return HandleScope.Close(v8::String::New(FieldTm.GetWebLogDateTimeStr(false, "T", false).CStr()));
			return TJsTm::New(FieldTm);
		} else { 
			return HandleScope.Close(v8::Null());
		}
	} else if (Desc.IsNumSpV()) {
        TIntFltKdV IntFltKdV; Rec.IsByRef() ? Store->GetFieldNumSpV(Rec.GetRecId(), FieldId, IntFltKdV) : Rec.GetFieldNumSpV(FieldId, IntFltKdV); 
		v8::Handle<v8::Array> JsIntFltKdV = v8::Array::New(IntFltKdV.Len());
		for (int IntFltKdN = 0; IntFltKdN < IntFltKdV.Len(); IntFltKdN++) {
			const TIntFltKd& IntFltKd = IntFltKdV[IntFltKdN];
			v8::Handle<v8::Object> JsIntFltKd = v8::Object::New();
			JsIntFltKd->Set(v8::String::New("id"), v8::Integer::New(IntFltKd.Key));
			JsIntFltKd->Set(v8::String::New("value"), v8::Number::New(IntFltKd.Dat));
			JsIntFltKdV->Set(IntFltKdN, JsIntFltKd);
		}
		return HandleScope.Close(JsIntFltKdV);
	} else if (Desc.IsBowSpV()) {
		return HandleScope.Close(v8::Null()); //TODO
    }
	throw TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

v8::Handle<v8::Value> TJsRec::join(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	TStr JoinNm = TJsRecUtil::GetStr(Properties);
	PRecSet RecSet = JsRec->Rec.DoJoin(JsRec->Js->Base, JoinNm);
	return TJsRecSet::New(JsRec->Js, RecSet);
}

v8::Handle<v8::Value> TJsRec::sjoin(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	TStr JoinNm = TJsRecUtil::GetStr(Properties);
	TRec JoinRec = JsRec->Rec.DoSingleJoin(JsRec->Js->Base, JoinNm);
	TWPt<TStore> JoinStore = JoinRec.GetStore();
	if (JoinRec.IsDef() && JoinStore->IsRecId(JoinRec.GetRecId())) { 
		return TJsRec::New(JsRec->Js, JoinRec);
	} else {
		return HandleScope.Close(v8::Null());
	}
}

v8::Handle<v8::Value> TJsRec::addJoin(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    TJsRec* JsRec = TJsRecUtil::GetSelf(Args);
    // read argument as record
    TStr JoinNm = TJsRecUtil::GetArgStr(Args, 0);
    TRec JoinRec = TJsRec::GetArgRec(Args, 1);
    const int JoinFq = TJsRecUtil::GetArgInt32(Args, 2, 1);
    // check parameters fine
    QmAssertR(JsRec->Store->IsJoinNm(JoinNm), "[addJoin] Unknown join " + JsRec->Store->GetStoreNm() + "." + JoinNm);
    QmAssertR(JoinFq > 0, "[addJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
    // get generic store
    TWPt<TStore> Store = JsRec->Store();
    const int JoinId = Store->GetJoinId(JoinNm);
    // add join
    Store->AddJoin(JoinId, JsRec->Rec.GetRecId(), JoinRec.GetRecId(), JoinFq);
	// return
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsRec::delJoin(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    TJsRec* JsRec = TJsRecUtil::GetSelf(Args);
    // read argument as record
    TStr JoinNm = TJsRecUtil::GetArgStr(Args, 0);
    TRec JoinRec = TJsRec::GetArgRec(Args, 1);
    const int JoinFq = TJsRecUtil::GetArgInt32(Args, 2, 1);
    // check parameters fine
    QmAssertR(JsRec->Store->IsJoinNm(JoinNm), "[delJoin] Unknown join " + JsRec->Store->GetStoreNm() + "." + JoinNm);
    QmAssertR(JoinFq > 0, "[delJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
    // get generic store
    TWPt<TStore> Store = JsRec->Store();
    const int JoinId = Store->GetJoinId(JoinNm);
    // delete join
    Store->DelJoin(JoinId, JsRec->Rec.GetRecId(), JoinRec.GetRecId(), JoinFq);
	// return
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsRec::toJSON(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    TJsRec* JsRec = TJsRecUtil::GetSelf(Args);
	const bool JoinRecsP = TJsRecUtil::IsArg(Args, 0) ? 
		(TJsRecUtil::IsArgBool(Args, 0) ? TJsRecUtil::GetArgBool(Args, 0, false) : false) : false;
	const bool JoinRecFieldsP = TJsRecUtil::IsArg(Args, 1) ? 
		(TJsRecUtil::IsArgBool(Args, 1) ? TJsRecUtil::GetArgBool(Args, 1, false) : false) : false;
    const bool FieldsP = true;
    const bool StoreInfoP = false;
    PJsonVal JsObj = JsRec->Rec.GetJson(JsRec->Js->Base, FieldsP, StoreInfoP, JoinRecsP, JoinRecFieldsP);
    // return
    return HandleScope.Close(TJsUtil::ParseJson(JsObj));
}

///////////////////////////////
// QMiner-JavaScript-IndexKey
v8::Persistent<v8::ObjectTemplate> TJsIndexKey::Template;

v8::Handle<v8::ObjectTemplate> TJsIndexKey::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, store);				
		JsRegisterProperty(TmpTemp, name);	
		JsRegisterProperty(TmpTemp, voc);
		JsRegisterProperty(TmpTemp, fq);
		
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsIndexKey::store(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsIndexKey* JsIndexKey = TJsIndexKeyUtil::GetSelf(Info);
	const uchar StoreId = JsIndexKey->IndexKey.GetStoreId();
	TStr StoreNm = JsIndexKey->Js->Base->GetStoreByStoreId(StoreId)->GetStoreNm();
	return HandleScope.Close(v8::String::New(StoreNm.CStr()));
}

v8::Handle<v8::Value> TJsIndexKey::name(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsIndexKey* JsIndexKey = TJsIndexKeyUtil::GetSelf(Info);
	TStr IndexKeyNm = JsIndexKey->IndexKey.GetKeyNm();
	return HandleScope.Close(v8::String::New(IndexKeyNm.CStr()));
}

v8::Handle<v8::Value> TJsIndexKey::voc(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsIndexKey* JsIndexKey = TJsIndexKeyUtil::GetSelf(Info);	
	TStrV KeyValV;
	JsIndexKey->Js->Base->GetIndexVoc()->GetAllWordStrV(JsIndexKey->IndexKey.GetKeyId(), KeyValV);
	v8::Local<v8::Array> KeyValV8 = v8::Array::New(KeyValV.Len());
	for (int WordN = 0; WordN < KeyValV.Len(); WordN++) {		
		KeyValV8->Set(v8::Number::New(WordN), v8::String::New(KeyValV[WordN].CStr()));		
	}
	return HandleScope.Close(KeyValV8);
}

v8::Handle<v8::Value> TJsIndexKey::fq(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsIndexKey* JsIndexKey = TJsIndexKeyUtil::GetSelf(Info);	
	TStrIntPrV KeyValV;
	JsIndexKey->Js->Base->GetIndexVoc()->GetAllWordStrFqV(JsIndexKey->IndexKey.GetKeyId(), KeyValV);
	v8::Local<v8::Array> KeyValV8 = v8::Array::New(KeyValV.Len());
	for (int WordN = 0; WordN < KeyValV.Len(); WordN++) {		
		KeyValV8->Set(v8::Number::New(WordN), v8::String::New(KeyValV[WordN].Val2.GetStr().CStr()));		
	}
	return HandleScope.Close(KeyValV8);
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine
THash<TStr, PBowAL> TJsAnalytics::ActiveLearnerH; 

v8::Persistent<v8::ObjectTemplate> TJsAnalytics::Template;

v8::Handle<v8::ObjectTemplate> TJsAnalytics::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
        JsRegisterFunction(TmpTemp, getLanguageOptions);
        JsRegisterFunction(TmpTemp, newFeatureSpace);
        JsRegisterFunction(TmpTemp, trainSvmClassify);		
        JsRegisterFunction(TmpTemp, trainSvmRegression);
        JsRegisterFunction(TmpTemp, trainKMeans);						
        JsRegisterFunction(TmpTemp, newActiveLearner);						
        JsRegisterFunction(TmpTemp, delActiveLearner);						
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsAnalytics::getLanguageOptions(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    
    // 0) create response object
	PJsonVal LangOpts = TJsonVal::NewObj();

    // 1) get available stemmers
    TStrV StemmerTypeNmV, StemmerTypeDNmV;
    TStemmer::GetStemmerTypeNmV(StemmerTypeNmV, StemmerTypeDNmV);
    PJsonVal ArrStemmer = TJsonVal::NewArr(StemmerTypeNmV);
    LangOpts->AddToObj("stemmer", ArrStemmer);

    // 2) get available stopword lists
    TStrV SwSetTypeNmV, SwSetTypeDNmV;
    TSwSet::GetSwSetTypeNmV(SwSetTypeNmV, SwSetTypeDNmV);
    PJsonVal ArrStopword = TJsonVal::NewArr(SwSetTypeNmV);
	LangOpts->AddToObj("stopwords", ArrStopword);

	return HandleScope.Close(TJsUtil::ParseJson(LangOpts));
}

v8::Handle<v8::Value> TJsAnalytics::newFeatureSpace(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
	try {
        // get first argument as json
        PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 0);
        // parse definitions of feature extractors
        TFtrExtV FtrExtV;
        if (ParamVal->IsObj()) {
            FtrExtV.Add(TFtrExt::New(JsAnalytics->Js->Base, ParamVal->GetObjStr("type"), ParamVal));
        } else if (ParamVal->IsArr()) {
            for (int ArrValN = 0; ArrValN < ParamVal->GetArrVals(); ArrValN++) {
                PJsonVal ArrVal = ParamVal->GetArrVal(ArrValN);
                FtrExtV.Add(TFtrExt::New(JsAnalytics->Js->Base, ArrVal->GetObjStr("type"), ArrVal));            
            }
        }
        // create feature space
        PFtrSpace FtrSpace = TFtrSpace::New(JsAnalytics->Js->Base, FtrExtV);
		// done
        return TJsFtrSpace::New(JsAnalytics->Js, FtrSpace);
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

// svm.trainSvmClassify(featureSpace, positives, negatives, parameters)
v8::Handle<v8::Value> TJsAnalytics::trainSvmClassify(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args, 0);
    PRecSet PosRecSet = TJsRecSet::GetArgRecSet(Args, 1);
    PRecSet NegRecSet = TJsRecSet::GetArgRecSet(Args, 2);
    PJsonVal SvmParamVal = TJsAnalyticsUtil::IsArgJson(Args, 3) ?
        TJsAnalyticsUtil::GetArgJson(Args, 3) : TJsonVal::NewObj();
    // parse SVM parameters
    const double C = SvmParamVal->GetObjNum("c", 1.0);
    const double j = SvmParamVal->GetObjNum("j", 1.0);
    const bool NormalizeP = SvmParamVal->GetObjBool("normalize", true);
    // do SVM
    try {
        // add records to SVM training set
        PSVMTrainSet TrainSet = TSparseTrainSet::New(PosRecSet->GetRecs() + NegRecSet->GetRecs());
        for (int RecN = 0; RecN < PosRecSet->GetRecs(); RecN++) {
            TRec Rec = PosRecSet->GetRec(RecN);
            TIntFltKdV SpV; FtrSpace->GetSpV(Rec, SpV);
            TrainSet->AddAttrV(SpV, 1.0, NormalizeP);
        }
        for (int RecN = 0; RecN < NegRecSet->GetRecs(); RecN++) {
            TRec Rec = NegRecSet->GetRec(RecN);
            TIntFltKdV SpV; FtrSpace->GetSpV(Rec, SpV);
            TrainSet->AddAttrV(SpV, -1.0, NormalizeP);
        }
        // train SVM
        PSVMModel Model = TSVMModel::NewClsLinear(TrainSet, C, j);
    	// return
        return TJsSvmModel::New(JsAnalytics->Js, FtrSpace, NormalizeP, Model);
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

// svm.trainSvmRegression(featureSpace, records, values, parameters)
v8::Handle<v8::Value> TJsAnalytics::trainSvmRegression(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args, 0);
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 1);
    PJsonVal ValArr = TJsAnalyticsUtil::GetArgJson(Args, 2);
    QmAssertR(ValArr->IsArr(), "Regression target values must be passed as array");
    TFltV ValV; ValArr->GetArrNumV(ValV);
    QmAssertR(ValV.Len() == RecSet->GetRecs(), "Number of records does not match number of provided target variables");
    PJsonVal SvmParamVal = TJsAnalyticsUtil::IsArgJson(Args, 3) ?
        TJsAnalyticsUtil::GetArgJson(Args, 3) : TJsonVal::NewObj();
    // parse SVM parameters
    const double C = SvmParamVal->GetObjNum("c", 1.0);
    const double E = SvmParamVal->GetObjNum("eps", 1.0);
    const bool NormalizeP = SvmParamVal->GetObjBool("normalize", true);
    // do SVM
    try {
        // add records to SVM training set
        PSVMTrainSet TrainSet = TSparseTrainSet::New(RecSet->GetRecs());
        for (int RecN = 0; RecN < RecSet->GetRecs(); RecN++) {
            // get record
            TRec Rec = RecSet->GetRec(RecN);
            TIntFltKdV SpV; FtrSpace->GetSpV(Rec, SpV);
            // get value
            const double Val = ValV[RecN];
            // add to trainset
            TrainSet->AddAttrV(SpV, Val, NormalizeP);
        }
        // train SVM
        PSVMModel Model = TSVMModel::NewRegLinear(TrainSet, E, C);
    	// return
        return TJsSvmModel::New(JsAnalytics->Js, FtrSpace, NormalizeP, Model);
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

// trainKMeans(featureSpace, positives, negatives, parameters)
v8::Handle<v8::Value> TJsAnalytics::trainKMeans(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args, 0);
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 1);
    PJsonVal KMeansParamVal = TJsAnalyticsUtil::IsArgJson(Args, 2) ?
        TJsAnalyticsUtil::GetArgJson(Args, 2) : TJsonVal::NewObj();
    // parse SVM parameters
    const int Clusts = KMeansParamVal->GetObjInt("k");
    const int MaxIter = KMeansParamVal->GetObjInt("maxIterations", 50);
    const int RndSeed = KMeansParamVal->GetObjInt("randomSeed", 1);
    // do kmeans
    try {
        // prepare feature vectors
        TVec<TIntFltKdV> SpVV; SpVV.Clr();
        FtrSpace->GetSpVV(RecSet, SpVV);
        // convert to sparse matrix with records as columns
        TSparseColMatrix FtrMatrix(SpVV, FtrSpace->GetDim(), SpVV.Len());
        // do the clustering
        TRnd Rnd(RndSeed);
        TSparseKMeans KMeans(&FtrMatrix, Clusts, MaxIter, TEnv::Logger, Rnd); 
        KMeans.Init(); KMeans.Apply();
        // get assignments
        TVec<TUInt64V> ClusterRecIdV(Clusts);
        const TIntV& Assignment = KMeans.GetAssignment();
        for(int RecN = 0; RecN < KMeans.GetDocs(); RecN++) {
            // cluster id
            const int ClustN = Assignment[RecN];
            // record id
            const uint64 RecId = RecSet->GetRecId(RecN);
            // remember assignment
            ClusterRecIdV[ClustN].Add(RecId);
        }
    	// return
		v8::Local<v8::Array> JsRecSetV = v8::Array::New(Clusts);
		for (int ClustN = 0; ClustN < Clusts; ClustN++) {
            PRecSet ClusterRecSet = TRecSet::New(RecSet->GetStore(), ClusterRecIdV[ClustN]);
			JsRecSetV->Set(v8::Number::New(ClustN), TJsRecSet::New(JsAnalytics->Js, ClusterRecSet));
		} 
		return HandleScope.Close(JsRecSetV);		
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

// newActiveLearner(featureSpace, records, query, parameters)
v8::Handle<v8::Value> TJsAnalytics::newActiveLearner(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);   
    // arg 0 is either a string or a feature space
    if(TJsAnalyticsUtil::IsArgStr(Args, 0)) {
        TStr name = TJsAnalyticsUtil::GetArgStr(Args, 0);
        // Check if AL exists
        if(!TJsAnalytics::ActiveLearnerH.IsKey(name)) {
            return HandleScope.Close(v8::Null());
        }
        // Load existing AL and return
        PBowAL AL = TJsAnalytics::ActiveLearnerH.GetDat(name);
        return TJsAL::New(JsAnalytics->Js, AL);
    }
    // if it's something else (i.e. FeatureSpace)
    PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args, 0);
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 1);
    PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 2);
    // parse  parameters
    TStr name = ParamVal->GetObjStr("name", TStr("default"));
    TStr Query = ParamVal->GetObjStr("query", TStr(""));
    const double C = ParamVal->GetObjNum("c", 1.0);
    const double j = ParamVal->GetObjNum("j", 3.0);
    const uint32 MnDocs = (uint32) ParamVal->GetObjNum("MnDocs", 2);
    try {
        // Run the feature extraction, state is stored in object. BOW representation
        PBowDocBs BowDocBs = FtrSpace->MakeBowDocBs(RecSet);
        // create AL
        PBowAL AL = TBowAL::NewFromQuery(BowDocBs, Query, MnDocs, MnDocs, C, j);
        // Generate questions
        AL->GenQueryDIdV(true);
        // Store AL
        TJsAnalytics::ActiveLearnerH.AddDat(name, AL);
    	// create and return TJsAL
        return TJsAL::New(JsAnalytics->Js, AL);
	} catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	return v8::Undefined();
}

// delActiveLearner(name)
v8::Handle<v8::Value> TJsAnalytics::delActiveLearner(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TStr name = TJsAnalyticsUtil::GetArgStr(Args, 0);
    // Check if AL exists
    if(!TJsAnalytics::ActiveLearnerH.IsKey(name)) {
        return HandleScope.Close(v8::Null());
    }
    // Delete AL
    TJsAnalytics::ActiveLearnerH.DelKey(name);

	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Feature-Space
v8::Persistent<v8::ObjectTemplate> TJsFtrSpace::Template;

v8::Handle<v8::ObjectTemplate> TJsFtrSpace::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, updateRecord);
		JsRegisterFunction(TmpTemp, updateRecords);
		JsRegisterFunction(TmpTemp, finishUpdate);					
		JsRegisterFunction(TmpTemp, extractStrings);						
		//JsRegisterFunction(TmpTemp, extractNumbers);						
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

PFtrSpace TJsFtrSpace::GetArgFtrSpace(const v8::Arguments& Args, const int& ArgN) {
    v8::HandleScope HandleScope;
    // check we have the argument at all
    AssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    // check it's of the right type
    AssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be Object", ArgN));
    // get the wrapped 
    v8::Handle<v8::Object> FtrSpace = v8::Handle<v8::Object>::Cast(Val);
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(FtrSpace->GetInternalField(0));
    // cast it to record set
    TJsFtrSpace* JsFtrSpace = static_cast<TJsFtrSpace*>(WrappedObject->Value());
    return JsFtrSpace->FtrSpace;    
}

v8::Handle<v8::Value> TJsFtrSpace::updateRecord(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    TRec Rec = TJsRec::GetArgRec(Args, 0);
    // update with new records
    JsFtrSpace->FtrSpace->Update(Rec);
	// return
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsFtrSpace::updateRecords(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
    // update with new records
    JsFtrSpace->FtrSpace->Update(RecSet);
	// return
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsFtrSpace::finishUpdate(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    // finish update
    JsFtrSpace->FtrSpace->FinishUpdate();
	// return
	return HandleScope.Close(v8::Null());
}

// extractStrings(String, Dimension=0)
v8::Handle<v8::Value> TJsFtrSpace::extractStrings(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PJsonVal RecVal = TJsFtrSpaceUtil::GetArgJson(Args, 0);
    uint32 DimN = TJsFtrSpaceUtil::GetArgInt32(Args, 1, 0);
    TStrV StrV;
    
    JsFtrSpace->FtrSpace->ExtractStrV(DimN, RecVal, StrV);

	// return
	v8::Handle<v8::Array> StrArr = v8::Array::New(StrV.Len());
    for(int StrN = 0; StrN < StrV.Len(); StrN++) {
		StrArr->Set(v8::Uint32::New(StrN), v8::String::New(StrV.GetVal(StrN).CStr()));
	}
	return HandleScope.Close(StrArr);
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model
v8::Persistent<v8::ObjectTemplate> TJsSvmModel::Template;

v8::Handle<v8::ObjectTemplate> TJsSvmModel::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
        JsRegisterProperty(TmpTemp, featureSpace);
		JsRegisterFunction(TmpTemp, classify);						
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsSvmModel::featureSpace(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSvmModel* JsSvmModel = TJsSvmModelUtil::GetSelf(Info);	
	return TJsFtrSpace::New(JsSvmModel->Js, JsSvmModel->FtrSpace);
}

// svm.trainClassify(featureSpace, positives, negatives, parameters)
v8::Handle<v8::Value> TJsSvmModel::classify(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsSvmModel* JsSvmModel = TJsSvmModelUtil::GetSelf(Args);	
    TRec Rec = TJsRec::GetArgRec(Args, 0);
    // get feature vector
    TIntFltKdV SpV; JsSvmModel->FtrSpace->GetSpV(Rec, SpV);
    if (JsSvmModel->NormalizeP) { TLinAlg::Normalize(SpV); }
    // classify
    const double Res = JsSvmModel->Model->GetRes(SpV);
    v8::Local<v8::Number> JsRes = v8::Number::New(Res);
	// return
	return HandleScope.Close(JsRes);
}

///////////////////////////////
// QMiner-JavaScript-Active-Learner
v8::Persistent<v8::ObjectTemplate> TJsAL::Template;

v8::Handle<v8::ObjectTemplate> TJsAL::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
        JsRegisterFunction(TmpTemp, getQuestion);
		JsRegisterFunction(TmpTemp, answerQuestion);						
        JsRegisterFunction(TmpTemp, getPositives);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsAL::getQuestion(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAL* JsAL = TJsALUtil::GetSelf(Args);	
    int32 QId = TJsALUtil::GetArgInt32(Args, 0, 0);

    // Check if there are questions
    if(JsAL->AL->GetQueryDIds() == 0) {
        return HandleScope.Close(v8::Number::New(-1));
    }

    // Get Question
    int DId;
    double Dist;
    JsAL->AL->GetQueryDistDId(QId, Dist, DId);
	
    // Get the number of docs currently assigned
    int DocN = JsAL->AL->GetPosDocN();
    // Get the InfoRetMode - Negating makes more sense here.
    bool InfoRetMode = !JsAL->AL->GetInfoRetMode();
    // Get Keywords for positive docs
    TStr Keywords = JsAL->AL->GetKeyWdStr();
    
    // Form Question Object
    PJsonVal QuestionObj = TJsonVal::NewObj();
    QuestionObj->AddToObj("questionId", DId); // this is actually the Doc Id for the AL
    QuestionObj->AddToObj("keywords", Keywords);
    QuestionObj->AddToObj("count",  DocN);
    QuestionObj->AddToObj("mode",  InfoRetMode);

	return HandleScope.Close(TJsUtil::ParseJson(QuestionObj));
}

v8::Handle<v8::Value> TJsAL::answerQuestion(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAL* JsAL = TJsALUtil::GetSelf(Args);	
    int32 DId = TJsALUtil::GetArgInt32(Args, 0);
    bool Positive = TJsALUtil::GetArgBool(Args, 1, true);

    // check if there are questions
    if(JsAL->AL->GetQueryDIds() == 0) {
        return HandleScope.Close(v8::Number::New(-1));
    }
    
    // answer
    JsAL->AL->MarkQueryDoc(DId, Positive);
    // regenerate questions
    JsAL->AL->GenQueryDIdV(true);
   
    return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsAL::getPositives(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAL* JsAL = TJsALUtil::GetSelf(Args);	

    // Check if it is ready
    if(!JsAL->AL->GetInfoRetMode()) {
        return HandleScope.Close(v8::Number::New(-1));
    }

    TIntV PosDIdV;
    JsAL->AL->GetAllPosDocs(PosDIdV);
    PJsonVal Positives = TJsonVal::NewArr(PosDIdV);
    return HandleScope.Close(TJsUtil::ParseJson(Positives));
}

///////////////////////////////
// QMiner-JavaScript-Correlation
v8::Persistent<v8::ObjectTemplate> TJsCorrelation::Template;

v8::Handle<v8::ObjectTemplate> TJsCorrelation::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, pearson);						
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsCorrelation::pearson(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// return
	return HandleScope.Close(v8::Null());
}

///////////////////////////////
// QMiner-JavaScript-GeoIP
bool TJsGeoIp::InitP = false;
PGeoIpBs TJsGeoIp::GeoIpBs = NULL;

PGeoIpBs TJsGeoIp::GetGeoIpBs() {
	if (!InitP) {
		InitP = true;
		//TODO: check for folder
		GeoIpBs = TGeoIpBs::LoadBin("./dbs/GeoIP-Full.GeoIP");
	}
	return GeoIpBs;
}

v8::Persistent<v8::ObjectTemplate> TJsGeoIp::Template;

v8::Handle<v8::ObjectTemplate> TJsGeoIp::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, country);				
		JsRegisterFunction(TmpTemp, location);	
		
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsGeoIp::country(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// read IP address
	TStr IpNumStr = TJsGeoIpUtil::GetArgStr(Args, 0);
	QmAssertR(TUInt::IsIpStr(IpNumStr), "Invalid IP address " + IpNumStr);
	// get country
	PGeoIpBs GeoIpBs = GetGeoIpBs();
	const int OrgId = GeoIpBs->GetOrgId(IpNumStr);
	if (OrgId == -1) { return HandleScope.Close(v8::Null()); }
	const TStr CountryNm = GeoIpBs->GetCountryNm(OrgId);
	// return
	return HandleScope.Close(v8::String::New(CountryNm.CStr()));
}

v8::Handle<v8::Value> TJsGeoIp::location(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// read IP address
	TStr IpNumStr = TJsGeoIpUtil::GetArgStr(Args, 0);
	QmAssertR(TUInt::IsIpStr(IpNumStr), "Invalid IP address " + IpNumStr);
	// get location
	PGeoIpBs GeoIpBs = GetGeoIpBs();
	const int OrgId = GeoIpBs->GetOrgId(IpNumStr);
	if (OrgId == -1) { return HandleScope.Close(v8::Null()); }
	// prepare response
	PJsonVal LocVal = TJsonVal::NewObj();
	LocVal->AddToObj("country", GeoIpBs->GetCountryNm(OrgId));
	LocVal->AddToObj("region", GeoIpBs->GetRegionNm(OrgId));
	LocVal->AddToObj("city", GeoIpBs->GetCityNm(OrgId));
	LocVal->AddToObj("ISP", GeoIpBs->GetIspNm(OrgId));
	LocVal->AddToObj("organization", GeoIpBs->GetOrgNm(OrgId));
	// return
	return HandleScope.Close(TJsUtil::ParseJson(LocVal));
}

///////////////////////////////
// QMiner-JavaScript-Fs
v8::Handle<v8::ObjectTemplate> TJsFs::GetTemplate() {
	v8::HandleScope HandleScope;
	v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
	JsRegisterFunction(TmpTemp, openRead);
	JsRegisterFunction(TmpTemp, openWrite);
	JsRegisterFunction(TmpTemp, openAppend);
	JsRegisterFunction(TmpTemp, exists);
	JsRegisterFunction(TmpTemp, copy);
	JsRegisterFunction(TmpTemp, move);
	JsRegisterFunction(TmpTemp, del);
	JsRegisterFunction(TmpTemp, rename);
	JsRegisterFunction(TmpTemp, fileInfo);
	JsRegisterFunction(TmpTemp, mkdir);
	JsRegisterFunction(TmpTemp, rmdir);
	JsRegisterFunction(TmpTemp, listFile);
	TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
	TmpTemp->SetInternalFieldCount(1);
	return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

bool TJsFs::CanAccess(const TStr& FPath) {
	TJsFPath JsFPath(FPath);
    for (int AllowedFPathN = 0; AllowedFPathN < AllowedFPathV.Len(); AllowedFPathN++) {
        if (JsFPath.IsSubdir(AllowedFPathV[AllowedFPathN])) { return true; }
    }
    return false;
}

bool TJsFs::CanAccess(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FPath = TJsFsUtil::GetArgStr(Args, 0);
	return JsFs->CanAccess(FPath);
}

// TODO: Perform standard check for file existence, etc.
v8::Handle<v8::Value> TJsFs::openRead(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
    QmAssertR(TFile::Exists(FNm), "File '" + FNm + "' does not exist");
	return TJsFIn::New(FNm);
}

v8::Handle<v8::Value> TJsFs::openWrite(const v8::Arguments& Args) { // call withb AppendP = false
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
    QmAssertR(TFile::Exists(FNm), "File '" + FNm + "' does not exist");
	return TJsFOut::New(FNm);
}

v8::Handle<v8::Value> TJsFs::openAppend(const v8::Arguments& Args) { // call with AppendP = true 
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
    //QmAssertR(TFile::Exists(FNm), "File '" + FNm + "' does not exist");
	return TJsFOut::New(FNm, true);
}

v8::Handle<v8::Value> TJsFs::exists(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
	return v8::Boolean::New(TFile::Exists(FNm));
}

v8::Handle<v8::Value> TJsFs::copy(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr SrcFNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(SrcFNm), "You don't have permission to access file '" + SrcFNm + "'");
    QmAssertR(TFile::Exists(SrcFNm), "File '" + SrcFNm + "' does not exist");
    TStr DstFNm = TJsFsUtil::GetArgStr(Args, 1);
    QmAssertR(JsFs->CanAccess(DstFNm), "You don't have permission to access file '" + DstFNm + "'");
	TFile::Copy(SrcFNm, DstFNm);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsFs::move(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr SrcFNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(SrcFNm), "You don't have permission to access file '" + SrcFNm + "'");
    QmAssertR(TFile::Exists(SrcFNm), "File '" + SrcFNm + "' does not exist");
    TStr DstFNm = TJsFsUtil::GetArgStr(Args, 1);
    QmAssertR(JsFs->CanAccess(DstFNm), "You don't have permission to access file '" + DstFNm + "'");
	TFile::Copy(SrcFNm, DstFNm);
	TFile::Del(SrcFNm, false); // ThrowExceptP = false 
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsFs::del(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
    QmAssertR(TFile::Exists(FNm), "File '" + FNm + "' does not exist");
	TFile::Del(FNm, false); // ThrowExceptP = false 
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsFs::rename(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr SrcFNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(SrcFNm), "You don't have permission to access file '" + SrcFNm + "'");
    QmAssertR(TFile::Exists(SrcFNm), "File '" + SrcFNm + "' does not exist");
    TStr DstFNm = TJsFsUtil::GetArgStr(Args, 1);
    QmAssertR(JsFs->CanAccess(DstFNm), "You don't have permission to access file '" + DstFNm + "'");
	TFile::Rename(SrcFNm, DstFNm);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsFs::fileInfo(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
    QmAssertR(TFile::Exists(FNm), "File '" + FNm + "' does not exist");
	const uint64 CreateTm = TFile::GetCreateTm(FNm);
	const uint64 LastAccessTm = TFile::GetLastAccessTm(FNm);
	const uint64 LastWriteTm = TFile::GetLastWriteTm(FNm);
	const uint64 Size = TFile::GetSize(FNm);
	v8::Handle<v8::Object> Obj = v8::Object::New();
	Obj->Set(v8::String::New("createTime"), v8::String::New(TTm::GetTmFromMSecs(CreateTm).GetWebLogDateTimeStr().CStr()));
	Obj->Set(v8::String::New("lastAccessTime"), v8::String::New(TTm::GetTmFromMSecs(LastAccessTm).GetWebLogDateTimeStr().CStr()));
	Obj->Set(v8::String::New("lastWriteTime"), v8::String::New(TTm::GetTmFromMSecs(LastWriteTm).GetWebLogDateTimeStr().CStr()));
	Obj->Set(v8::String::New("size"), v8::Number::New(static_cast<double>(Size)));
	return HandleScope.Close(Obj);
}

v8::Handle<v8::Value> TJsFs::mkdir(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FPath = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FPath), "You don't have permission to access directory '" + FPath + "'");
	const bool GenDirP = TDir::GenDir(FPath);
	return v8::Boolean::New(GenDirP);
}

v8::Handle<v8::Value> TJsFs::rmdir(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FPath = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FPath), "You don't have permission to access directory '" + FPath + "'");
	const bool DelDirP = TDir::DelDir(FPath);
	return HandleScope.Close(v8::Boolean::New(DelDirP));
}

v8::Handle<v8::Value> TJsFs::listFile(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    // read parameters
    TStr FPath = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FPath), "You don't have permission to access directory '" + FPath + "'");
    TStr FExt = TJsFsUtil::GetArgStr(Args, 1);
    const bool RecurseP = TJsFsUtil::GetArgBool(Args, 2, false);
    // get file list
	TStrV FNmV;
	TFFile::GetFNmV(FPath, TStrV::GetV(FExt), RecurseP, FNmV);
	v8::Handle<v8::Array> FNmArr = v8::Array::New(FNmV.Len());
	for(int FldN = 0; FldN < FNmV.Len(); ++FldN) {
		FNmArr->Set(v8::Uint32::New(FldN), v8::String::New(FNmV.GetVal(FldN).CStr()));
	}
	return HandleScope.Close(FNmArr);
}

///////////////////////////////
// QMiner-JavaScript-FIn
v8::Persistent<v8::ObjectTemplate> TJsFIn::Template;

v8::Handle<v8::ObjectTemplate> TJsFIn::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, peekCh);
		JsRegisterFunction(TmpTemp, getCh);
		JsRegisterFunction(TmpTemp, getNextLn);
		JsRegisterProperty(TmpTemp, eof);
		JsRegisterProperty(TmpTemp, length);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

// takes no arguments 
v8::Handle<v8::Value> TJsFIn::getCh(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFIn* JsFIn = TJsFInUtil::GetSelf(Args);
	return v8::String::New(TStr(JsFIn->SIn->GetCh()).CStr());
}

v8::Handle<v8::Value> TJsFIn::peekCh(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFIn* JsFIn = TJsFInUtil::GetSelf(Args);
	return v8::String::New(TStr(JsFIn->SIn->PeekCh()).CStr());
}

v8::Handle<v8::Value> TJsFIn::getNextLn(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFIn* JsFIn = TJsFInUtil::GetSelf(Args);
	TChA LnChA; JsFIn->SIn->GetNextLn(LnChA);
	return v8::String::New(LnChA.CStr());
}

v8::Handle<v8::Value> TJsFIn::eof(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFIn* JsFIn = TJsFInUtil::GetSelf(Info);
	return v8::Boolean::New(JsFIn->SIn->Eof());
}

v8::Handle<v8::Value> TJsFIn::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFIn* JsFIn = TJsFInUtil::GetSelf(Info);
	return v8::Uint32::New(JsFIn->SIn->Len());
}

///////////////////////////////
// QMiner-JavaScript-FOut
v8::Persistent<v8::ObjectTemplate> TJsFOut::Template;

v8::Handle<v8::ObjectTemplate> TJsFOut::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, write);
		JsRegisterFunction(TmpTemp, writeLine);
        JsRegisterFunction(TmpTemp, flush);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsFOut::write(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    QmAssertR(Args.Length() == 1, "Invalid number of arguments to fout.write()");
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);
    int OutN = 0;
	if(TJsFOutUtil::IsArgFlt(Args, 0)) {
        OutN = JsFOut->SOut->PutFlt(TJsFOutUtil::GetArgFlt(Args, 0));
	} else if(TJsFOutUtil::IsArgJson(Args, 0)) {
        OutN = JsFOut->SOut->PutStr(TJsFOutUtil::GetArgJsonStr(Args, 0));
	} else if (TJsFOutUtil::IsArgStr(Args, 0)) {
        OutN = JsFOut->SOut->PutStr(TJsFOutUtil::GetArgStr(Args, 0));
	} else {
        throw TQmExcept::New("Invalid parameter type to fout.write()");
	}
	//JsFOut->SOut->Flush();
	return v8::Integer::New(OutN);
}

v8::Handle<v8::Value> TJsFOut::writeLine(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // first we write
    v8::Handle<v8::Value> Res = write(Args);
    int OutN = Res->Int32Value();
    // then we make a new line
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);  
    OutN += JsFOut->SOut->PutLn();
	return v8::Integer::New(OutN);
}

v8::Handle<v8::Value> TJsFOut::flush(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);
	JsFOut->SOut->Flush();
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Console
v8::Handle<v8::ObjectTemplate> TJsHttp::GetTemplate() {
	v8::HandleScope HandleScope;
    v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
 	JsRegisterFunction(TmpTemp, onRequest);
	JsLongRegisterFunction(TmpTemp, "get", get);
	JsLongRegisterFunction(TmpTemp, "getStr", get);
	JsLongRegisterFunction(TmpTemp, "post", post);
	JsLongRegisterFunction(TmpTemp, "postStr", post);
    TmpTemp->SetInternalFieldCount(1);
    return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

v8::Handle<v8::Value> TJsHttp::onRequest(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsHttp* JsHttp = TJsHttpUtil::GetSelf(Args);
	// get function name and callback
	TStr SrvFunNm = TJsHttpUtil::GetArgStr(Args, 0);
	TStr Verb = TJsHttpUtil::GetArgStr(Args, 1);
	v8::Persistent<v8::Function> Callback = TJsHttpUtil::GetArgFunPer(Args, 2);
	// register
	TEnv::Logger->OnStatusFmt("Mapping URL '/%s/%s' %s to JS function.", 
        JsHttp->Js->GetScriptNm().CStr(), SrvFunNm.CStr(), Verb.CStr());

	JsHttp->Js->AddSrvFun(JsHttp->Js->GetScriptNm(), SrvFunNm, Verb, Callback);
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsHttp::get(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsHttp* JsHttp = TJsHttpUtil::GetSelf(Args);
	// get function name and callback
	TStr UrlStr = TJsHttpUtil::GetArgStr(Args, 0);
	v8::Persistent<v8::Function> OnSuccess = (Args.Length() > 1) ? 
		TJsHttpUtil::GetArgFunPer(Args, 1) : v8::Persistent<v8::Function>();
	v8::Persistent<v8::Function> OnError = (Args.Length() > 2) ? 
		TJsHttpUtil::GetArgFunPer(Args, 2) : v8::Persistent<v8::Function>();
	// what should we return
	const bool JsonP = (TJsHttpUtil::GetFunNm(Args) == "get");
	// fetch
	TEnv::Logger->OnStatusFmt("Fetching URL '%s'", UrlStr.CStr());
	JsHttp->Js->JsFetch->Fetch(TJsFetchRq(THttpRq::New(TUrl::New(UrlStr)), JsonP, OnSuccess, OnError));
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsHttp::post(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsHttp* JsHttp = TJsHttpUtil::GetSelf(Args);
	// get function name and callback
	TStr UrlStr = TJsHttpUtil::GetArgStr(Args, 0);
	TStr MimeTypeStr = TJsHttpUtil::GetArgStr(Args, 1);
	TStr BodyStr = TJsHttpUtil::GetArgStr(Args, 2);
	v8::Persistent<v8::Function> OnSuccess = (Args.Length() > 3) ? 
		TJsHttpUtil::GetArgFunPer(Args, 3) : v8::Persistent<v8::Function>();
	v8::Persistent<v8::Function> OnError = (Args.Length() > 4) ? 
		TJsHttpUtil::GetArgFunPer(Args, 4) : v8::Persistent<v8::Function>();
	// what should we return
	const bool JsonP = (TJsHttpUtil::GetFunNm(Args) == "post");
	// prepare request
	TMem BodyMem; BodyMem.AddBf(BodyStr.CStr(), BodyStr.Len());
	PHttpRq HttpRq = THttpRq::New(hrmPost, TUrl::New(UrlStr), MimeTypeStr, BodyMem);
	// fetch
	TEnv::Logger->OnStatusFmt("Fetching URL '%s'", UrlStr.CStr());
	JsHttp->Js->JsFetch->Fetch(TJsFetchRq(HttpRq, JsonP, OnSuccess, OnError));
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Http-Response
v8::Persistent<v8::ObjectTemplate> TJsHttpResp::Template;

v8::Handle<v8::ObjectTemplate> TJsHttpResp::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, setStatusCode);
		JsRegisterFunction(TmpTemp, setContentType);
		JsRegisterFunction(TmpTemp, add);
		JsRegisterFunction(TmpTemp, close);
		JsRegisterFunction(TmpTemp, send);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

//TODO change to property
v8::Handle<v8::Value> TJsHttpResp::setStatusCode(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // check we are still allowed to do anything    
    TJsHttpResp* JsHttpResp = TJsHttpRespUtil::GetSelf(Args);
    QmAssertR(!JsHttpResp->DoneP, "HTTP response already completed.");
    // update response code
    QmAssertR(Args.Length() == 1, "Invalid number of arguments to httpResponse.setStatusCode()");
    JsHttpResp->StatusCode = TJsHttpRespUtil::GetArgInt32(Args, 0);
    return v8::Undefined();
}

//TODO change to property
v8::Handle<v8::Value> TJsHttpResp::setContentType(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // check we are still allowed to do anything    
    TJsHttpResp* JsHttpResp = TJsHttpRespUtil::GetSelf(Args);
    QmAssertR(!JsHttpResp->DoneP, "HTTP response already completed.");
    // update content type
    QmAssertR(Args.Length() == 1, "Invalid number of arguments to httpResponse.setContentType()");
    JsHttpResp->ContTypeStr = TJsHttpRespUtil::GetArgStr(Args, 0);
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsHttpResp::add(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // check we are still allowed to do anything    
    TJsHttpResp* JsHttpResp = TJsHttpRespUtil::GetSelf(Args);
    QmAssertR(!JsHttpResp->DoneP, "HTTP response already completed.");
    // in case we have arguments, add them to the buffer
    if (Args.Length() > 0) {
        QmAssertR(Args.Length() == 1, "Invalid number of arguments to httpResponse.add()");
        if(TJsHttpRespUtil::IsArgJson(Args, 0)) {
            JsHttpResp->BodyMem += TJsHttpRespUtil::GetArgJsonStr(Args, 0);
        } else if (TJsHttpRespUtil::IsArgStr(Args, 0)) {
            JsHttpResp->BodyMem += TJsHttpRespUtil::GetArgStr(Args, 0);
        } else {
            throw TQmExcept::New("Invalid parameter type to httpResponse.add()");
        }
    }
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsHttpResp::close(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // check we are still allowed to do anything    
    TJsHttpResp* JsHttpResp = TJsHttpRespUtil::GetSelf(Args);
    QmAssertR(!JsHttpResp->DoneP, "HTTP response already completed.");
    // First mark we are done (we can only send response once)
    JsHttpResp->DoneP = true;
    // Prepare response
    PSIn BodySIn = JsHttpResp->BodyMem.GetSIn();
    PHttpResp HttpResp = THttpResp::New(JsHttpResp->StatusCode, 
        JsHttpResp->ContTypeStr, false, BodySIn);
    // Execute response
    JsHttpResp->WebSrv->SendHttpResp(JsHttpResp->SockId, HttpResp); 
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsHttpResp::send(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // first add the data to the buffer
    TJsHttpResp::add(Args);
    // and execute the response
    TJsHttpResp::close(Args);
    return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Time
v8::Persistent<v8::ObjectTemplate> TJsTm::Template;

v8::Handle<v8::ObjectTemplate> TJsTm::GetTemplate() {
	v8::HandleScope HandleScope;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, string);
		JsRegisterProperty(TmpTemp, timestamp);
        JsRegisterProperty(TmpTemp, now);
        JsRegisterProperty(TmpTemp, nowUTC);
        JsRegisterFunction(TmpTemp, add);
		JsRegisterFunction(TmpTemp, sub);
		JsRegisterFunction(TmpTemp, toJSON);
		JsRegisterFunction(TmpTemp, parse);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsTm::string(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Info);
    TStr TmStr = JsTm->Tm.GetWebLogDateTimeStr(true, "T", true);   
	return HandleScope.Close(v8::String::New(TmStr.CStr()));
}

v8::Handle<v8::Value> TJsTm::timestamp(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Info);
    const int Timestamp = TTm::GetDateTimeIntFromTm(JsTm->Tm);
	return HandleScope.Close(v8::Int32::New(Timestamp));
}

v8::Handle<v8::Value> TJsTm::now(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(TJsTm::New(TTm::GetCurLocTm()));
}

v8::Handle<v8::Value> TJsTm::nowUTC(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(TJsTm::New(TTm::GetCurUniTm()));
}

v8::Handle<v8::Value> TJsTm::add(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Args);   
    // parse arguments
    const int Val = TJsTmUtil::GetArgInt32(Args, 0);
    const TStr Unit = TJsTmUtil::GetArgStr(Args, 1, "second");
    // add according to the unit
    if (Unit == "second") {
        JsTm->Tm.AddTime(0, 0, Val);
    } else if (Unit == "minute") {
        JsTm->Tm.AddTime(0, Val);        
    } else if (Unit == "hour") {
        JsTm->Tm.AddTime(Val);        
    } else if (Unit == "day") {
        JsTm->Tm.AddDays(Val);        
    }
    // nothing to return
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsTm::sub(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Args);   
    // parse arguments
    const int Val = TJsTmUtil::GetArgInt32(Args, 0);
    const TStr Unit = TJsTmUtil::GetArgStr(Args, 1, "second");
    // add according to the unit
    if (Unit == "second") {
        JsTm->Tm.SubTime(0, 0, Val);
    } else if (Unit == "minute") {
        JsTm->Tm.SubTime(0, Val);        
    } else if (Unit == "hour") {
        JsTm->Tm.SubTime(Val);        
    } else if (Unit == "day") {
        JsTm->Tm.SubDays(Val);        
    }
    // nothing to return
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsTm::toJSON(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Args);   
    // prepare response object
    v8::Local<v8::Object> TmJson = v8::Object::New();
    TmJson->Set(v8::String::New("year"), v8::Int32::New(JsTm->Tm.GetYear()));
    TmJson->Set(v8::String::New("month"), v8::Int32::New(JsTm->Tm.GetMonth()));
    TmJson->Set(v8::String::New("day"), v8::Int32::New(JsTm->Tm.GetDay()));
    TmJson->Set(v8::String::New("hour"), v8::Int32::New(JsTm->Tm.GetHour()));
    TmJson->Set(v8::String::New("minute"), v8::Int32::New(JsTm->Tm.GetMin()));
    TmJson->Set(v8::String::New("second"), v8::Int32::New(JsTm->Tm.GetSec()));
    TmJson->Set(v8::String::New("milisecond"), v8::Int32::New(JsTm->Tm.GetMSec()));
    // return constructed json
    return HandleScope.Close(TmJson);
}

v8::Handle<v8::Value> TJsTm::parse(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // read string argument
    TStr TmStr = TJsTmUtil::GetArgStr(Args, 0);
    // prepare response object
    TTm Tm = TTm::GetTmFromWebLogDateTimeStr(TmStr, '-', ':', '.', 'T');
    // return constructed json
    return HandleScope.Close(TJsTm::New(Tm));
}

///////////////////////////////
// QMiner-LinAlg
v8::Handle<v8::ObjectTemplate> TJsLinAlg::GetTemplate() {
	v8::HandleScope HandleScope;
    v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
 	JsRegisterFunction(TmpTemp, newVec);
	JsRegisterFunction(TmpTemp, newMat);
	
    TmpTemp->SetInternalFieldCount(1);
    return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

v8::Handle<v8::Value> TJsLinAlg::newVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	v8::Persistent<v8::Object> JsVec = TJsFltV::New(JsLinAlg->Js);
	TFltV& Vec = TJsFltV::GetFltV(JsVec);
	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		int Length = Array->Length();		
		Vec.Gen(Length, 0);
		for (int ElN = 0; ElN < Length; ElN++) {			
			Vec.Add(Array->Get(ElN)->NumberValue());
		}
		return HandleScope.Close(JsVec);
	}
	if (Args[0]->IsObject()) {		
		int MxVals = TJsLinAlgUtil::GetArgInt32(Args, 0, "mxVals", -1);
		int Vals = TJsLinAlgUtil::GetArgInt32(Args, 0, "vals", -1);		
		if (MxVals > 0 && Vals >= 0) {
			Vec.Gen(MxVals, Vals);
		}
		if (MxVals == -1 && Vals >= 0) {
			Vec.Gen(Vals);
		}
	}
	return HandleScope.Close(JsVec);
}

v8::Handle<v8::Value> TJsLinAlg::newMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	v8::Persistent<v8::Object> JsMat = TJsFltVV::New(JsLinAlg->Js);
	TFltVV& Mat = TJsFltVV::GetFltVV(JsMat);

	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		int Rows = Array->Length();		
		if (Rows > 0) {
			// are the objects arrays
			int Cols;
			for (int RowN = 0; RowN < Rows; RowN++) {	
				if (!Array->Get(RowN)->IsArray()) {
					throw TQmExcept::New("object is not an array of arrays in TJsLinAlg::newMat()");							
					return HandleScope.Close(JsMat);
				} else {
					v8::Handle<v8::Array> Row = v8::Handle<v8::Array>::Cast(Array->Get(RowN));
					if (RowN == 0) {
						Cols = Row->Length();
						Mat.Gen(Rows, Cols);
					} else {
						if ((int)Row->Length() != Cols) {							
							throw TQmExcept::New("inconsistent number of columns in TJsLinAlg::newMat()");							
							return HandleScope.Close(JsMat);
						}
					}
					for (int ColN = 0; ColN < Cols; ColN++) {
						Mat.PutXY(RowN, ColN, Row->Get(ColN)->NumberValue());
					}
				}
			}				
			return HandleScope.Close(JsMat);
		}
	}
	if (Args[0]->IsObject()) {		
		int Cols = TJsLinAlgUtil::GetArgInt32(Args, 0, "cols", 0);
		int Rows = TJsLinAlgUtil::GetArgInt32(Args, 0, "rows", 0);		
		if (Cols > 0 && Rows > 0) {
			Mat.Gen(Rows, Cols);
		}		
	}
	return HandleScope.Close(JsMat);
}

///////////////////////////////
// QMiner-FltV
v8::Handle<v8::ObjectTemplate> TJsFltV::GetTemplate() {
	v8::HandleScope HandleScope;
    v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
 	JsRegisterFunction(TmpTemp, at);
	JsRegisterFunction(TmpTemp, put);
	JsRegisterFunction(TmpTemp, push);
	JsRegisterFunction(TmpTemp, sum);
	JsRegisterFunction(TmpTemp, outer);
	JsRegisterFunction(TmpTemp, inner);
	JsRegisterFunction(TmpTemp, plus);
	JsRegisterFunction(TmpTemp, minus);
	JsRegisterFunction(TmpTemp, multiply);
	JsRegisterFunction(TmpTemp, normalize);
	JsRegisterProperty(TmpTemp, length);
    TmpTemp->SetInternalFieldCount(1);
    return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);	
}

v8::Handle<v8::Value> TJsFltV::at(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	TInt Index = TJsFltVUtil::GetArgInt32(Args, 0);	
	QmAssertR(Index >= 0 && Index < JsFltV->Vec.Len(), "vector at: index out of bounds");
	double result = JsFltV->Vec[Index];
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsFltV::put(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	if (Args.Length() == 2) {
		QmAssertR(Args[0]->IsInt32(), "the first argument should be an integer");
		QmAssertR(Args[1]->IsNumber(), "the second argument should be a number");				
		TInt Index = TJsFltVUtil::GetArgInt32(Args, 0);	
		TFlt Val = TJsFltVUtil::GetArgFlt(Args, 1);	
		QmAssertR(Index >= 0 && Index < JsFltV->Vec.Len(), "vector put: index out of bounds");		
		JsFltV->Vec[Index] = Val;
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltV::push(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	TFlt Val = TJsFltVUtil::GetArgFlt(Args, 0);
	int result = JsFltV->Vec.Add(Val);	
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsFltV::sum(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	double result = 0.0;
	if (JsFltV->Vec.Len() > 0) {
		result = TLinAlg::SumVec(JsFltV->Vec);
	}
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsFltV::outer(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsFltVUtil::IsArgClass(Args, 0, "TFltV")) {
				// get argument vector
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() > 0 && JsVec->Vec.Len(), "vectors must have nonzero length");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsFltV->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len(), JsVec->Vec.Len());
				TLinAlg::OuterProduct(JsFltV->Vec, JsVec->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltV::inner(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	double result = 0.0;
	if (Args[0]->IsObject()) {
		if (TJsFltVUtil::IsArgClass(Args, 0, "TFltV")) {
			TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
			QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector' * vector: dimensions mismatch");
			result = TLinAlg::DotProduct(JsFltV->Vec, JsVec->Vec);
		}
	}
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsFltV::plus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsFltVUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector + vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
				TFltV& Result = TJsFltV::GetFltV(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len());
				TLinAlg::LinComb(1.0, JsFltV->Vec, 1.0, JsVec->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltV::minus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsFltVUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector - vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
				TFltV& Result = TJsFltV::GetFltV(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len());
				TLinAlg::LinComb(1.0, JsFltV->Vec, -1.0, JsVec->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltV::multiply(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);
	if (TJsFltVUtil::IsArgFlt(Args, 0)) {		
		double Scalar = TJsFltVUtil::GetArgFlt(Args, 0);
		// create output object
		v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
		// get the internal glib vector
		TFltV& Result = TJsFltV::GetFltV(JsResult);
		// computation
		Result.Gen(JsFltV->Vec.Len());
		TLinAlg::MultiplyScalar(Scalar, JsFltV->Vec, Result);
		
		return HandleScope.Close(JsResult);
	}
	return HandleScope.Close(v8::Undefined());
}


v8::Handle<v8::Value> TJsFltV::normalize(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Args);		
	if (JsFltV->Vec.Len() > 0) {
		TLinAlg::Normalize(JsFltV->Vec);
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltV::length(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsFltVUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsFltV->Vec.Len()));
}

///////////////////////////////
// QMiner-FltVV
v8::Handle<v8::ObjectTemplate> TJsFltVV::GetTemplate() {
	v8::HandleScope HandleScope;
    v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
	JsRegisterFunction(TmpTemp, at);
	JsRegisterFunction(TmpTemp, put);
	JsRegisterFunction(TmpTemp, multiply);
	JsRegisterFunction(TmpTemp, plus);
	JsRegisterFunction(TmpTemp, minus);
	JsRegisterFunction(TmpTemp, transpose);
	JsRegisterFunction(TmpTemp, solve);
	JsRegisterFunction(TmpTemp, normalizeCols);
 	JsRegisterProperty(TmpTemp, rows);
	JsRegisterProperty(TmpTemp, cols);
    TmpTemp->SetInternalFieldCount(1);
    return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);	
}

v8::Handle<v8::Value> TJsFltVV::at(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);	
	TInt Row = TJsFltVVUtil::GetArgInt32(Args, 0);	
	TInt Col = TJsFltVVUtil::GetArgInt32(Args, 1);
	TInt Rows = JsFltVV->Mat.GetRows();
	TInt Cols = JsFltVV->Mat.GetCols();
	QmAssertR(Row >= 0 && Col >= 0 && Row < Rows && Col < Cols, "matrix at: index out of bounds");
	double result = JsFltVV->Mat.At(Row,Col);	
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsFltVV::put(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);	
	if (Args.Length() == 3) {
		QmAssertR(Args[0]->IsInt32(), "the first argument should be an integer");
		QmAssertR(Args[1]->IsInt32(), "the second argument should be an integer");
		QmAssertR(Args[2]->IsNumber(), "the third argument should be a number");
		TInt Row = TJsFltVVUtil::GetArgInt32(Args, 0);	
		TInt Col = TJsFltVVUtil::GetArgInt32(Args, 1);	
		TInt Rows = JsFltVV->Mat.GetRows();
		TInt Cols = JsFltVV->Mat.GetCols();
		QmAssertR(Row >= 0 && Col >= 0 && Row < Rows && Col < Cols, "matrix put: index out of bounds");
		TFlt Val = TJsFltVVUtil::GetArgFlt(Args, 2);
		JsFltVV->Mat.At(Row, Col) = Val;	
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::multiply(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgFlt(Args, 0)) {
			double Scalar = TJsFltVVUtil::GetArgFlt(Args, 0);
			// create JS result and get the internal data				
			v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
			TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
			// computation
			Result.Gen(JsMat->Mat.GetRows(), JsMat->Mat.GetCols());
			TLinAlg::MultiplyScalar(Scalar, JsMat->Mat, Result);
			return HandleScope.Close(JsResult);			
		}
		if (Args[0]->IsObject()) {
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "matrix * vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js);
				TFltV& Result = TJsFltV::GetFltV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetRows());
				TLinAlg::Multiply(JsMat->Mat, JsVec->Vec, Result);

				return HandleScope.Close(JsResult);			
			}
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() == JsMat2->Mat.GetRows(), "matrix * matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetRows(), JsMat2->Mat.GetCols());
				TLinAlg::Multiply(JsMat->Mat, JsMat2->Mat, Result);

				return HandleScope.Close(JsResult);
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::plus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (Args[0]->IsObject()) {			
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() == JsMat2->Mat.GetCols() && JsMat->Mat.GetRows() == JsMat2->Mat.GetRows(), "matrix - matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetRows(), JsMat2->Mat.GetCols());
				TLinAlg::LinComb(1.0, JsMat->Mat, 1.0, JsMat2->Mat, Result);
				return HandleScope.Close(JsResult);
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::minus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (Args[0]->IsObject()) {			
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() == JsMat2->Mat.GetCols() && JsMat->Mat.GetRows() == JsMat2->Mat.GetRows(), "matrix - matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetRows(), JsMat2->Mat.GetCols());
				TLinAlg::LinComb(1.0, JsMat->Mat, -1.0, JsMat2->Mat, Result);
				return HandleScope.Close(JsResult);
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::transpose(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	
	v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsFltVV->Js);
	TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
	Result.Gen(JsFltVV->Mat.GetCols(), JsFltVV->Mat.GetRows());
	TLinAlg::Transpose(JsFltVV->Mat, Result);

	return HandleScope.Close(JsResult);	
}

v8::Handle<v8::Value> TJsFltVV::solve(const v8::Arguments& Args) {
v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "matrix \\ vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js);
				TFltV& Result = TJsFltV::GetFltV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetCols());
				TNumericalStuff::SolveLinearSystem(JsMat->Mat, JsVec->Vec, Result);			

				return HandleScope.Close(JsResult);			
			}			
		}
	}	
	return HandleScope.Close(v8::Undefined());		
}

v8::Handle<v8::Value> TJsFltVV::normalizeCols(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	TLinAlg::NormalizeColumns(JsFltVV->Mat);
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::rows(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsFltVV->Mat.GetRows()));
}

v8::Handle<v8::Value> TJsFltVV::cols(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsFltVV->Mat.GetCols()));
}

}
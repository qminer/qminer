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

TStrH TJsUtil::ObjNameH = TStrH();
TInt TJsUtil::ObjCount = 0;
TInt TJsUtil::ObjCountRate = 0;

void TJsUtil::AddObj(const TStr& ObjName) {
    ObjNameH.AddDat(ObjName)++;
    CountObj();
}

void TJsUtil::DelObj(const TStr& ObjName) {
    QmAssert(ObjNameH.IsKey(ObjName));
    ObjNameH.GetDat(ObjName)--;
    CountObj();
}

void TJsUtil::CountObj() {
    ObjCount++;
    if ((ObjCountRate > 0) && (ObjCount % ObjCountRate == 0)) {
        int KeyId = ObjNameH.FFirstKeyId();
        DebugLog("COUNT start ----------------\n");
        while (ObjNameH.FNextKeyId(KeyId)) {
            TEnv::Logger->OnStatusFmt("COUNT: %s %d \n",
                ObjNameH.GetKey(KeyId).CStr(), ObjNameH[KeyId].Val);
        }
        InfoLog("COUNT end ------------------\n");
    }
}

TStrIntPrV TJsUtil::GetObjStat() {
    TStrIntPrV ObjNameCountV;
    ObjNameH.GetKeyDatPrV(ObjNameCountV);
    return ObjNameCountV;
}

void TJsUtil::SetObjStatRate(const int& _ObjCountRate) {
    ObjCountRate = _ObjCountRate;
}

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

v8::Handle<v8::Object> TJsUtil::HttpRqToJson(PHttpRq HttpRq) {
	v8::HandleScope HandleScope;
    v8::Handle<v8::Object> Request = v8::Object::New();
    // Headers
    for (int FieldN = 0; FieldN < HttpRq->GetFldValH().Len(); FieldN++) {
        TStr KeyVal = HttpRq->GetFldValH().GetKey(FieldN);
        Request->Set(v8::String::New(KeyVal.CStr()), v8::String::New(HttpRq->GetFldValH().GetDat(KeyVal).CStr()));
    }
    // Method
    Request->Set(v8::String::New("method"), v8::String::New(HttpRq->GetMethodNm().CStr()));
    // URL
    // - scheme
    Request->Set(v8::String::New("scheme"), v8::String::New(HttpRq->GetUrl()->GetSchemeNm().CStr()));
    // - path
    Request->Set(v8::String::New("path"), v8::String::New(HttpRq->GetUrl()->GetPathStr().CStr()));
    // GET arguments
    v8::Handle<v8::Object> Args = v8::Object::New();
    for (int KeyN = 0; KeyN < HttpRq->GetUrlEnv()->GetKeys(); KeyN++) {
        TStr ArgKey = HttpRq->GetUrlEnv()->GetKeyNm(KeyN);
        v8::Handle<v8::Array> ArgVals = v8::Array::New(HttpRq->GetUrlEnv()->GetVals(ArgKey));
        for (int ValN = 0; ValN < HttpRq->GetUrlEnv()->GetVals(ArgKey); ValN++) {
            ArgVals->Set(ValN, v8::String::New(HttpRq->GetUrlEnv()->GetVal(ArgKey, ValN).CStr()));
        }
        Args->Set(v8::String::New(ArgKey.CStr()), ArgVals);
    }
    Request->Set(v8::String::New("args"), Args);
    // Request Body
    // - raw data
    TStr Data;
    if (HttpRq->IsBody()) {
        Data = HttpRq->GetBodyAsStr();
        //TEnv::Debug->OnStatusFmt("Data Received %s ", Data.CStr());
        // parse JSON
        if (HttpRq->IsContType(THttp::AppJSonFldVal)) {
            v8::Handle<v8::Value> BodyVal = TJsUtil::ParseJson(HttpRq->GetBodyAsStr());
            Request->Set(v8::String::New("jsonData"), BodyVal);
        }
    }
    Request->Set(v8::String::New("data"), v8::String::New(Data.CStr()));
	return HandleScope.Close(Request); 
}

v8::Handle<v8::Value> TJsUtil::GetStrArr(const TStrV& StrV) {
   	v8::HandleScope HandleScope;
    v8::Handle<v8::Array> JsStrV = v8::Array::New(StrV.Len());
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        JsStrV->Set(StrN, v8::String::New(StrV[StrN].CStr()));
    }
    return HandleScope.Close(JsStrV);
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
#ifdef V8_DEBUG
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::Locker Locker(Isolate);
#endif
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
		const uint StoreId = TriggerV[TriggerN].Val1;
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

void TScript::Execute(v8::Handle<v8::Function> Fun, v8::Handle<v8::Object>& Arg1, v8::Handle<v8::Object>& Arg2) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 2;
	v8::Handle<v8::Value> Argv[Argc] = {Arg1, Arg2 };
	Fun->Call(Context->Global(), Argc, Argv);
	TJsUtil::HandleTryCatch(TryCatch);
}

void TScript::Execute(v8::Handle<v8::Function> Fun, v8::Handle<v8::Value>& Arg1, v8::Handle<v8::Value>& Arg2) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 2;
	v8::Handle<v8::Value> Argv[Argc] = {Arg1, Arg2 };
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

double TScript::ExecuteFlt(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { Arg };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
	QmAssertR(RetVal->IsNumber(), "Return type expected to be number");
	return RetVal->NumberValue();
}

void TScript::ExecuteFltVec(v8::Handle<v8::Function> Fun, const v8::Handle<v8::Value>& Arg, TFltV& Vec) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { Arg };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
	// Cast as FltV and copy result
	v8::Handle<v8::Object> RetValObj = v8::Handle<v8::Object>::Cast(RetVal);
	v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RetValObj->GetInternalField(0));
	// cast it to js vector and copy internal vector
	TJsFltV* JsVec = static_cast<TJsFltV*>(WrappedObject->Value());
	Vec = JsVec->Vec;
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

PJsonVal TScript::ExecuteJson(v8::Handle<v8::Function> Fun, const TInt& ArgInt) {
	v8::HandleScope HandleScope;
	v8::TryCatch TryCatch;
	const int Argc = 1;
	v8::Handle<v8::Value> Argv[Argc] = { v8::Number::New((double)ArgInt) };
	v8::Handle<v8::Value> RetVal = Fun->Call(Context->Global(), Argc, Argv);
	// handle errors
	TJsUtil::HandleTryCatch(TryCatch);
	// check we got what we expected
	if (RetVal->IsObject()) { 
		return TJsonVal::GetValFromStr(TJsUtil::V8JsonToStr(RetVal));
	}
	// else complain
	throw TQmExcept::New("Wrong return type, expected JSON!");
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

void TScript::ExecuteSrvFun(const PHttpRq& HttpRq, const TWPt<TJsHttpResp>& JsHttpResp) {
	v8::HandleScope HandleScope;
    // Get the Verb
    TStr Verb = HttpRq->GetMethodNm();
    // Get the Path
	PUrl Url = HttpRq->GetUrl();
    const int PathSegs = Url->GetPathSegs();
    //TEnv::Logger->OnStatusFmt("Execute for %s request: %s", Verb.CStr(), Url->GetPathStr().CStr());

    // Search the rules
    int SelectedRule = -1;
    v8::Handle<v8::Object> Params = v8::Object::New();
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
        for(int PathN = 0; PathN < Rule->GetArrVals(); PathN++) {
            if(Rule->GetArrVal(PathN)->GetObjStr("path") == Url->GetPathSeg(Seg)) {
                if(Rule->GetArrVal(PathN)->IsObjKey(("param"))) {
                    Seg++;
                    Params->Set(v8::String::New(Rule->GetArrVal(PathN)->GetObjStr("param").CStr()),
                                v8::String::New(Url->GetPathSeg(Seg).CStr()));
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
    v8::Handle<v8::Object> Request = TJsUtil::HttpRqToJson(HttpRq);
    // @TODO add request args
    Request->Set(v8::String::New("params"), Params);
    // Exec
    if(RuleMatched && SelectedRule >= 0) {
        // get function
        v8::Persistent<v8::Function> JsFun = JsNmFunH.GetDat(SelectedRule);
        // execute
        try {
            v8::Handle<v8::Object> HttpResp = TJsObjUtil<TJsHttpResp>::New(JsHttpResp());
            Execute(JsFun, Request, HttpResp);
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

void TScript::AddTrigger(const uint& StoreId, const PStoreTrigger& Trigger) { 
	TriggerV.Add(TPair<TUInt, PStoreTrigger>(StoreId, Trigger)); 
}

v8::Handle<v8::Value> TScript::require(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    TStr ModuleFNm;
    try {
        // get pointer to the context and script
        v8::Handle<v8::Object> Self = Args.Holder();
        v8::Handle<v8::Context> Context = Self->CreationContext();
        v8::Context::Scope ContextScope(Context);    
        TWPt<TScript> Script = TScript::GetGlobal(Context);
        // get module filename
        ModuleFNm = TJsObjUtil<TScript>::GetArgStr(Args, 0);
        // check if one of built-in modules
        if (ModuleFNm == "__analytics__") { 
            return TJsAnalytics::New(Script);
        } else if (ModuleFNm == "geoip") { 
            return TJsGeoIp::New();
        } else if (ModuleFNm == "dmoz") { 
            return TJsDMoz::New();
        } else if (ModuleFNm == "time") {
            return TJsTm::New();
        } else if (ModuleFNm == "analytics") { 
            InfoLog("Warning: require('analytics') is deprecated, use require('analytics.js') instead");
            return TJsAnalytics::New(Script);
        }
        // load the source of the module
        InfoLog("Loading " + ModuleFNm);
        TStr ModuleSource = Script->LoadModuleSrc(ModuleFNm);
        // handle JS exceptions
        v8::TryCatch TryCatch;
        // compile the module
        v8::Handle<v8::Script> Module = v8::Script::Compile(
            v8::String::New(ModuleSource.CStr()),
             v8::String::New(ModuleFNm.CStr()));
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
    } catch (const PExcept& Except) {
        ErrorLog("Error loading module " + ModuleFNm + ": " + Except->GetMsgStr());
    }
    // return null
    return HandleScope.Close(v8::Null());    
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

#ifdef V8_DEBUG
	// for debugging JavaScript
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::Locker Lock(Isolate);
#endif

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
	Execute(TEnv::QMinerFPath + "process.js");
	Execute(TEnv::QMinerFPath + "console.js");
    Execute(TEnv::QMinerFPath + "qminer.js");
	Execute(TEnv::QMinerFPath + "fs.js");
	Execute(TEnv::QMinerFPath + "http.js");
	Execute(TEnv::QMinerFPath + "linalg.js");
	Execute(TEnv::QMinerFPath + "spMat.js");
	Execute(TEnv::QMinerFPath + "store.js");
	Execute(ScriptFNm);
}

void TScript::Install() {
	v8::HandleScope HandleScope;
	// delete existing objects, if they exist
	Context->Global()->Delete(v8::String::New("process"));
	Context->Global()->Delete(v8::String::New("console"));
	Context->Global()->Delete(v8::String::New("qm"));
	Context->Global()->Delete(v8::String::New("fs"));
	Context->Global()->Delete(v8::String::New("http"));
	Context->Global()->Delete(v8::String::New("la"));
	// create fresh ones
	DebugLog("Installing 'process' object");
	Context->Global()->Set(v8::String::New("process"), TJsProcess::New(this));
    DebugLog("Installing 'console' object");
	Context->Global()->Set(v8::String::New("console"), TJsConsole::New(this));
    DebugLog("Installing 'qm' object");
	Context->Global()->Set(v8::String::New("qm"), TJsBase::New(this));
    DebugLog("Installing 'fs' object");
	Context->Global()->Set(v8::String::New("fs"), TJsFs::New(this));
    DebugLog("Installing 'http' object");
	Context->Global()->Set(v8::String::New("http"), TJsHttp::New(this));
	DebugLog("Installing 'la' object");
	Context->Global()->Set(v8::String::New("la"), TJsLinAlg::New(this));
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
        TWPt<TJsHttpResp> JsHttpResp = new TJsHttpResp(RqEnv->GetWebSrv(), RqEnv->GetSockId());
		// call appropriate JavaScript function
		Js->ExecuteSrvFun(HttpRq, JsHttpResp);
	}
}

///////////////////////////////
// QMiner-JavaScript-Server-Function
void TJsAdminSrvFun::Exec(const TStrKdV& FldNmValKdV, const PSAppSrvRqEnv& RqEnv) {
    PJsonVal JsonVal = TJsonVal::NewObj();
    // get memory statistics
    TStrIntPrV ObjNameCountV = TJsUtil::GetObjStat();
    PJsonVal MemObjArr = TJsonVal::NewArr();
    for (int ObjNameCountN = 0; ObjNameCountN < ObjNameCountV.Len(); ObjNameCountN++) {
        MemObjArr->AddToArr(TJsonVal::NewObj(
            ObjNameCountV[ObjNameCountN].Val1, 
            ObjNameCountV[ObjNameCountN].Val2));
    }    
    PJsonVal MemVal = TJsonVal::NewObj();
    MemVal->AddToObj("objects", MemObjArr);
#ifdef GLib_LINUX   
    TSysMemStat MemStat;
    MemVal->AddToObj("size", TUInt64::GetStr(MemStat.Size));
    MemVal->AddToObj("sizeKb", TUInt64::GetKiloStr(MemStat.Size));
    MemVal->AddToObj("sizeMb", TUInt64::GetMegaStr(MemStat.Size));
#endif  
    JsonVal->AddToObj("memory", MemVal);
    // return statistics
    PHttpResp HttpResp = THttpResp::New(THttp::OkStatusCd,
        THttp::AppJSonFldVal, false, TMIn::New(JsonVal->SaveStr()));
    RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp);         
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

void TJsStoreTrigger::OnAdd(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnAddFun.IsEmpty()) { 
		Js->Execute(OnAddFun, TJsRec::New(Js, Rec));
	}
}

void TJsStoreTrigger::OnUpdate(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnUpdateFun.IsEmpty()) { 
		Js->Execute(OnUpdateFun, TJsRec::New(Js, Rec));
	}
}

void TJsStoreTrigger::OnDelete(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnDeleteFun.IsEmpty()) { 
		Js->Execute(OnDeleteFun, TJsRec::New(Js, Rec));
	}
}

///////////////////////////////
// QMiner-JavaScript-Stream-Aggr
TJsStreamAggr::TJsStreamAggr(TWPt<TScript> _Js, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal) : TStreamAggr(_Js->Base, _AggrNm), Js(_Js) {
	v8::HandleScope HandleScope;
	// Every stream aggregate should implement these two
	QmAssertR(TriggerVal->Has(v8::String::New("onAdd")), "TJsStreamAggr constructor, name: " + _AggrNm  + ", type: javaScript. Missing onAdd callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");
	QmAssertR(TriggerVal->Has(v8::String::New("saveJson")), "TJsStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing saveJson callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");
		
	v8::Handle<v8::Value> _OnAddFun = TriggerVal->Get(v8::String::New("onAdd"));
	QmAssert(_OnAddFun->IsFunction());
	OnAddFun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(_OnAddFun));
	
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
	
	v8::Handle<v8::Value> _SaveJsonFun = TriggerVal->Get(v8::String::New("saveJson"));
	QmAssert(_SaveJsonFun->IsFunction());
	SaveJsonFun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(_SaveJsonFun));
}

void TJsStreamAggr::OnAddRec(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnAddFun.IsEmpty()) {
		Js->Execute(OnAddFun, TJsRec::New(Js, Rec));
	}
}

void TJsStreamAggr::OnUpdateRec(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnUpdateFun.IsEmpty()) {
		Js->Execute(OnUpdateFun, TJsRec::New(Js, Rec));
	}
}

void TJsStreamAggr::OnDeleteRec(const TRec& Rec) {
	v8::HandleScope HandleScope;
	if (!OnDeleteFun.IsEmpty()) {
		Js->Execute(OnDeleteFun, TJsRec::New(Js, Rec));
	}
}

///////////////////////////////
// QMiner-JavaScript-WebPgFetch
void TJsFetch::OnFetch(const int& FId, const PWebPg& WebPg) {
	// execute callback
	{
        QmAssert(CallbackH.IsKey(FId));
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
        QmAssert(CallbackH.IsKey(FId));
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
// QMiner-JavaScript-Base
TJsBase::TJsBase(TWPt<TScript> _Js): Js(_Js), Base(_Js->Base) { }

v8::Handle<v8::ObjectTemplate> TJsBase::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, analytics);
		JsRegisterFunction(TmpTemp, store);
		JsRegisterFunction(TmpTemp, getStoreList);
		JsRegisterFunction(TmpTemp, createStore);
		JsRegisterFunction(TmpTemp, search);
		JsLongRegisterFunction(TmpTemp, "operator", op);
		JsRegisterFunction(TmpTemp, gc);
		JsRegisterFunction(TmpTemp, newStreamAggr);
		JsRegisterFunction(TmpTemp, getStreamAggr);
		JsRegisterFunction(TmpTemp, getStreamAggrNames);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template =  v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsBase::analytics(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Info);
    InfoLog("Warning: qm.analytics will be deprecated, use require('analytics') instead.");
	return TJsAnalytics::New(JsBase->Js);
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
    // check we can write
    QmAssertR(!JsBase->Base->IsRdOnly(), "Base opened as read-only");
    // parse arguments
    PJsonVal SchemaVal = TJsBaseUtil::GetArgJson(Args, 0);
	uint64 DefStoreSize = (uint64) TJsBaseUtil::GetArgInt32(Args, 1, 1024);
    DefStoreSize = DefStoreSize * TInt::Mega;
    // create new stores
    TVec<TWPt<TStore> > NewStoreV = TStorage::CreateStoresFromSchema(
        JsBase->Base, SchemaVal, DefStoreSize);   
    // return store (if only one) or array of stores (if more)
    if (NewStoreV.Len() == 1) { 
        return TJsStore::New(JsBase->Js, NewStoreV[0]);
    } else if (NewStoreV.Len() > 1) {
		v8::Local<v8::Array> JsNewStoreV = v8::Array::New(NewStoreV.Len());
		for (int NewStoreN = 0; NewStoreN < NewStoreV.Len(); NewStoreN++) {
			JsNewStoreV->Set(v8::Number::New(NewStoreN),
                TJsStore::New(JsBase->Js, NewStoreV[NewStoreN]));
		}         
    }
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

v8::Handle<v8::Value> TJsBase::newStreamAggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	// we have only one parameter which is supposed to be object
	QmAssertR(Args.Length() <= 2 && Args.Length() > 0, "qm.newStreamAggr expects at least one parameter");
	QmAssertR(Args[0]->IsObject(), "qm.newStreamAggr expects object as first parameter");

	PStreamAggr StreamAggr;

	// parse out parameters
	PJsonVal ParamVal = TJsBaseUtil::GetArgJson(Args, 0);
	// get aggregate type
	TStr TypeNm = TJsBaseUtil::GetArgStr(Args, 0, "type", "javaScript");
	// check if the aggregate is a pure javascript aggregate (js callbacks)
	if (TypeNm == "javaScript") {
		// we have a javascript stream aggregate
		TStr AggrName = TJsBaseUtil::GetArgStr(Args, 0, "name", "");
		// we need a name, if not give just generate one
		if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
		// create aggregate
		StreamAggr = TJsStreamAggr::New(
			JsBase->Js, AggrName, Args[0]->ToObject());
	} else if (TypeNm == "ftrext") {
		TStr AggrName = TJsBaseUtil::GetArgStr(Args, 0, "name", "");
		QmAssertR(Args[0]->ToObject()->Has(v8::String::New("featureSpace")), "addStreamAggr: featureSpace property missing!");
		// we need a name, if not give just generate one
		if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
		PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args[0]->ToObject()->Get(v8::String::New("featureSpace")));
		StreamAggr = TStreamAggrs::TFtrExtAggr::New(JsBase->Base, AggrName, FtrSpace);
	}
	else if (TypeNm == "stmerger") {
		// create new aggregate
		StreamAggr = TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
		PJsonVal FieldArrVal = ParamVal->GetObjKey("fields");
		TStrV InterpNmV;
		QmAssertR(ParamVal->IsObjKey("fields"), "Missing argument 'fields'!");
		// automatically register the aggregate for addRec callbacks
		for (int FieldN = 0; FieldN < FieldArrVal->GetArrVals(); FieldN++) {
			PJsonVal FieldVal = FieldArrVal->GetArrVal(FieldN);
			PJsonVal SourceVal = FieldVal->GetObjKey("source");
			TStr StoreNm = "";
			if (SourceVal->IsStr()) {
				// we have just store name
				StoreNm = SourceVal->GetStr();
			}
			else if (SourceVal->IsObj()) {
				// get store
				StoreNm = SourceVal->GetObjStr("store");
			}
			JsBase->Base->AddStreamAggr(JsBase->Base->GetStoreByStoreNm(StoreNm)->GetStoreId(), StreamAggr);
		}
	}
	else {
		// we have a GLib stream aggregate, translate parameters to PJsonVal
		PJsonVal ParamVal = TJsBaseUtil::GetArgJson(Args, 0);
		if (Args.Length() > 1 && Args[1]->IsString()) {
			ParamVal->AddToObj("store", TJsBaseUtil::GetArgStr(Args, 1));
		}
		
		// check if it's one stream aggregate or composition
		if (TStreamAggrs::TCompositional::IsCompositional(TypeNm)) {
			// we have a composition of aggregates, call code to assemble it
			TStreamAggrs::TCompositional::Register(JsBase->Base, TypeNm, ParamVal);
		}
		else {
			// create new aggregate
			StreamAggr = TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
		}
	}
	if (!TStreamAggrs::TCompositional::IsCompositional(TypeNm)) {
		if (Args.Length() > 1) {
			TStrV Stores(0);
			if (Args[1]->IsString()) {
				Stores.Add(TJsBaseUtil::GetArgStr(Args, 1));
			}
			if (Args[1]->IsArray()) {
				PJsonVal StoresJson = TJsBaseUtil::GetArgJson(Args, 1);
				QmAssertR(StoresJson->IsDef(), "qm.newStreamAggr : Args[1] should be a string (store name) or a string array (store names)");
				StoresJson->GetArrStrV(Stores);
			}
			for (int StoreN = 0; StoreN < Stores.Len(); StoreN++) {
				QmAssertR(JsBase->Base->IsStoreNm(Stores[StoreN]), "qm.newStreamAggr, Args[1] : store does not exist!");
				JsBase->Base->AddStreamAggr(Stores[StoreN], StreamAggr);
			}
		}
		else {
			JsBase->Base->AddStreamAggr(StreamAggr);
		}
		// non-compositional aggregates are returned
		return HandleScope.Close(TJsSA::New(JsBase->Js, StreamAggr));
	}
	else {
		return HandleScope.Close(v8::Null());
	}
}

v8::Handle<v8::Value> TJsBase::getStreamAggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	const TStr AggrNm = TJsBaseUtil::GetArgStr(Args, 0);
	if (JsBase->Base->IsStreamAggr(AggrNm)) {
		PStreamAggr StreamAggr = JsBase->Base->GetStreamAggr(AggrNm);
		return HandleScope.Close(TJsSA::New(JsBase->Js, StreamAggr));
	}
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsBase::getStreamAggrNames(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsBase* JsBase = TJsBaseUtil::GetSelf(Args);
	PStreamAggrBase SABase = JsBase->Base->GetStreamAggrBase();
	int AggrId = SABase->GetFirstStreamAggrId();
	v8::Local<v8::Array> Arr = v8::Array::New();
	uint32 Counter = 0;
	while (SABase->GetNextStreamAggrId(AggrId)) {
		v8::Local<v8::String> AggrNm = v8::String::New(SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
		Arr->Set(Counter, AggrNm);
		Counter++;
	}
	return HandleScope.Close(Arr);	
}

///////////////////////////////
// QMiner-JavaScript-Stream-Aggregate
v8::Handle<v8::ObjectTemplate> TJsSA::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, name);
		JsRegisterFunction(TmpTemp, onAdd);
		JsRegisterFunction(TmpTemp, onUpdate);
		JsRegisterFunction(TmpTemp, onDelete);
		JsRegisterFunction(TmpTemp, saveJson);
		JsRegisterProperty(TmpTemp, val);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsSA::name(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Info);
	v8::Local<v8::String> SAName = v8::String::New(JsSA->SA->GetAggrNm().CStr());
	return HandleScope.Close(SAName);
}

v8::Handle<v8::Value> TJsSA::onAdd(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Args);
	const TRec Rec = TJsRec::GetArgRec(Args, 0);
	JsSA->SA->OnAddRec(Rec);
	return HandleScope.Close(Args.Holder());
}

v8::Handle<v8::Value> TJsSA::onUpdate(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Args);
	const TRec Rec = TJsRec::GetArgRec(Args, 0);
	JsSA->SA->OnUpdateRec(Rec);
	return HandleScope.Close(Args.Holder());
}

v8::Handle<v8::Value> TJsSA::onDelete(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Args);
	const TRec Rec = TJsRec::GetArgRec(Args, 0);
	JsSA->SA->OnDeleteRec(Rec);
	return HandleScope.Close(Args.Holder());
}

v8::Handle<v8::Value> TJsSA::saveJson(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Args);
	const TInt Limit = TJsSAUtil::GetArgInt32(Args, 0, -1);
	PJsonVal Json = JsSA->SA->SaveJson(Limit);
	v8::Handle<v8::Value> V8Json = TJsUtil::ParseJson(Json);
	return HandleScope.Close(V8Json);
}

v8::Handle<v8::Value> TJsSA::val(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSA* JsSA = TJsSAUtil::GetSelf(Info);
	PJsonVal Json = JsSA->SA->SaveJson(-1);
	v8::Handle<v8::Value> V8Json = TJsUtil::ParseJson(Json);
	return HandleScope.Close(V8Json);
}

///////////////////////////////
// QMiner-JavaScript-Store
v8::Handle<v8::ObjectTemplate> TJsStore::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, name);
		JsRegisterProperty(TmpTemp, empty);
		JsRegisterProperty(TmpTemp, length);
		JsRegisterProperty(TmpTemp, recs);
		JsRegisterProperty(TmpTemp, fields);
		JsRegisterProperty(TmpTemp, joins);
		JsRegisterProperty(TmpTemp, keys);
        JsRegisterProperty(TmpTemp, first);
        JsRegisterProperty(TmpTemp, last);
        JsRegisterProperty(TmpTemp, forwardIter);
        JsRegisterProperty(TmpTemp, backwardIter);
		JsRegIndexedProperty(TmpTemp, indexId);
		JsRegisterFunction(TmpTemp, rec);
		JsRegisterFunction(TmpTemp, add);
		JsRegisterFunction(TmpTemp, newRec);
		JsRegisterFunction(TmpTemp, newRecSet);
		JsRegisterFunction(TmpTemp, sample);
		JsRegisterFunction(TmpTemp, field);        
		JsRegisterFunction(TmpTemp, key);
		JsRegisterFunction(TmpTemp, addTrigger);
        //JsRegisterFunction(TmpTemp, addStreamAggr);
        JsRegisterFunction(TmpTemp, getStreamAggr);
		JsRegisterFunction(TmpTemp, getStreamAggrNames);
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
	v8::Local<v8::Array> FieldV = v8::Array::New(JsStore->Store->GetFields());
	for (int FieldId = 0; FieldId < JsStore->Store->GetFields(); FieldId++) {
        const TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);
        v8::Local<v8::Object> Field = v8::Object::New();
        Field->Set(v8::String::New("id"), v8::Int32::New(FieldDesc.GetFieldId()));
        Field->Set(v8::String::New("name"), v8::String::New(FieldDesc.GetFieldNm().CStr()));
        Field->Set(v8::String::New("type"), v8::String::New(FieldDesc.GetFieldTypeStr().CStr()));
        Field->Set(v8::String::New("nullable"), v8::Boolean::New(FieldDesc.IsNullable()));
        Field->Set(v8::String::New("internal"), v8::Boolean::New(FieldDesc.IsInternal()));
        Field->Set(v8::String::New("primary"), v8::Boolean::New(FieldDesc.IsPrimary()));
		FieldV->Set(v8::Number::New(FieldId), Field);
	}	
	return HandleScope.Close(FieldV);
}

v8::Handle<v8::Value> TJsStore::joins(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	v8::Local<v8::Array> JoinV = v8::Array::New(JsStore->Store->GetJoins());
	for (int JoinId = 0; JoinId < JsStore->Store->GetJoins(); JoinId++) {
        const TJoinDesc& JoinDesc = JsStore->Store->GetJoinDesc(JoinId);
        TWPt<TStore> JoinStore = JoinDesc.GetJoinStore(JsStore->Js->Base);
        v8::Local<v8::Object> Join = v8::Object::New();
        Join->Set(v8::String::New("id"), v8::Int32::New(JoinDesc.GetJoinId()));
        Join->Set(v8::String::New("name"), v8::String::New(JoinDesc.GetJoinNm().CStr()));
        Join->Set(v8::String::New("store"), v8::String::New(JoinStore->GetStoreNm().CStr()));
        if (JoinDesc.IsInverseJoinId()) {
            Join->Set(v8::String::New("inverse"), v8::String::New(JoinStore->GetJoinNm(JoinDesc.GetInverseJoinId()).CStr()));
        }        
        if (JoinDesc.IsFieldJoin()) {
            Join->Set(v8::String::New("type"), v8::String::New("field"));
            Join->Set(v8::String::New("recordField"), v8::String::New(JsStore->Store->GetFieldNm(JoinDesc.GetJoinRecFieldId()).CStr()));
            Join->Set(v8::String::New("weightField"), v8::String::New(JsStore->Store->GetFieldNm(JoinDesc.GetJoinFqFieldId()).CStr()));           
        } else if (JoinDesc.IsIndexJoin()) {
            Join->Set(v8::String::New("type"), v8::String::New("index"));            
        	TWPt<TIndexVoc> IndexVoc = JsStore->Js->Base->GetIndexVoc();
            Join->Set(v8::String::New("key"), TJsIndexKey::New(JsStore->Js, IndexVoc->GetKey(JoinDesc.GetJoinKeyId())));;
        }
		JoinV->Set(v8::Number::New(JoinId), Join);
	}
	return HandleScope.Close(JoinV);
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

v8::Handle<v8::Value> TJsStore::first(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
    v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
    const uint64 FirstRecId = JsStore->Store->FirstRecId();
    if (FirstRecId == TUInt64::Mx) { return v8::Null(); }
    return HandleScope.Close(TJsRec::New(JsStore->Js, JsStore->Store->GetRec(FirstRecId)));
}

v8::Handle<v8::Value> TJsStore::last(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
    v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
    const uint64 LastRecId = JsStore->Store->LastRecId();
    if (LastRecId == TUInt64::Mx) { return v8::Null(); }
    return HandleScope.Close(TJsRec::New(JsStore->Js, JsStore->Store->GetRec(LastRecId)));
}

v8::Handle<v8::Value> TJsStore::forwardIter(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
    v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	return HandleScope.Close(TJsStoreIter::New(JsStore->Js,
        JsStore->Store, JsStore->Store->ForwardIter()));
}

v8::Handle<v8::Value> TJsStore::backwardIter(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
    v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Info);
	return HandleScope.Close(TJsStoreIter::New(JsStore->Js,
        JsStore->Store, JsStore->Store->BackwardIter()));
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

v8::Handle<v8::Value> TJsStore::newRec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
    PJsonVal RecVal = TJsStoreUtil::GetArgJson(Args, 0);
    TRec Rec(JsStore->Store(), RecVal);
    return TJsRec::New(JsStore->Js, Rec);
}

v8::Handle<v8::Value> TJsStore::newRecSet(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	if (Args.Length() > 0) {
        // argument 0 = TJsIntV of record ids
		QmAssertR(TJsStoreUtil::IsArgClass(Args, 0, "TIntV"), 
			"Store.getRecSetByIdV: The first argument must be a TIntV (js linalg full int vector)");
		TJsIntV* JsVec = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 0);
		PRecSet ResultSet = TRecSet::New(JsStore->Store, JsVec->Vec);
		return TJsRecSet::New(JsStore->Js, ResultSet);
	}
	return TJsRecSet::New(JsStore->Js, TRecSet::New());
}

v8::Handle<v8::Value> TJsStore::sample(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const int SampleSize = TJsStoreUtil::GetArgInt32(Args, 0);
	PRecSet ResultSet = JsStore->Store->GetRndRecs(SampleSize);
	return TJsRecSet::New(JsStore->Js, ResultSet); 
}

v8::Handle<v8::Value> TJsStore::field(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const TStr FieldNm = TJsStoreUtil::GetArgStr(Args, 0);
    if (JsStore->Store->IsFieldNm(FieldNm)) {
        const int FieldId = JsStore->Store->GetFieldId(FieldNm);
        const TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);
        v8::Local<v8::Object> Field = v8::Object::New();
        Field->Set(v8::String::New("id"), v8::Int32::New(FieldDesc.GetFieldId()));
        Field->Set(v8::String::New("name"), v8::String::New(FieldDesc.GetFieldNm().CStr()));
        Field->Set(v8::String::New("type"), v8::String::New(FieldDesc.GetFieldTypeStr().CStr()));
        Field->Set(v8::String::New("nullable"), v8::Boolean::New(FieldDesc.IsNullable()));
        Field->Set(v8::String::New("internal"), v8::Boolean::New(FieldDesc.IsInternal()));
        Field->Set(v8::String::New("primary"), v8::Boolean::New(FieldDesc.IsPrimary()));
        return HandleScope.Close(Field);
    }
	return HandleScope.Close(v8::Null());
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
	JsStore->Js->TriggerV.Add(TPair<TUInt, PStoreTrigger>(JsStore->Store->GetStoreId(), Trigger));
	return HandleScope.Close(v8::Null());
}

//v8::Handle<v8::Value> TJsStore::addStreamAggr(const v8::Arguments& Args) {
//	v8::HandleScope HandleScope;
//    TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
//	Args.Length
//	v8::Handle<v8::Value> Result = TJsBase::newStreamAggr(Args);
//	return HandleScope.Close(Result);
//	
//}

v8::Handle<v8::Value> TJsStore::getStreamAggr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	const TStr AggrNm = TJsStoreUtil::GetArgStr(Args, 0);	
    TWPt<TBase> Base = JsStore->Js->Base;
    const uint StoreId = JsStore->Store->GetStoreId();
	if (Base->IsStreamAggr(StoreId, AggrNm)) {
        PStreamAggr StreamAggr = Base->GetStreamAggr(StoreId, AggrNm);
		return HandleScope.Close(TJsSA::New(JsStore->Js, StreamAggr));
	}
	return HandleScope.Close(v8::Null());
}

v8::Handle<v8::Value> TJsStore::getStreamAggrNames(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStore* JsStore = TJsStoreUtil::GetSelf(Args);
	TWPt<TBase> Base = JsStore->Js->Base;
	const uint StoreId = JsStore->Store->GetStoreId();
	PStreamAggrBase SABase = Base->GetStreamAggrBase(StoreId);
	int AggrId = SABase->GetFirstStreamAggrId();
	v8::Local<v8::Array> Arr = v8::Array::New();
	uint32 Counter = 0;
	while (SABase->GetNextStreamAggrId(AggrId)) {
		v8::Local<v8::String> AggrNm = v8::String::New(SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
		Arr->Set(Counter, AggrNm);
		Counter++;
	}
	return HandleScope.Close(Arr);
}

///////////////////////////////
// JavaScript Store Iterator
v8::Handle<v8::ObjectTemplate> TJsStoreIter::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, store);
        JsRegisterProperty(TmpTemp, rec);
        JsRegisterFunction(TmpTemp, next);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsStoreIter::store(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStoreIter* JsStoreIter = TJsStoreIterUtil::GetSelf(Info);
	return HandleScope.Close(TJsStore::New(JsStoreIter->Js, JsStoreIter->Store));
}

v8::Handle<v8::Value> TJsStoreIter::rec(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsStoreIter* JsStoreIter = TJsStoreIterUtil::GetSelf(Info);
    const uint64 RecId = JsStoreIter->Iter->GetRecId();
	return HandleScope.Close(TJsRec::New(JsStoreIter->Js, JsStoreIter->Store->GetRec(RecId)));
}

v8::Handle<v8::Value> TJsStoreIter::next(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsStoreIter* JsStoreIter = TJsStoreIterUtil::GetSelf(Args);    
	return HandleScope.Close(v8::Boolean::New(JsStoreIter->Iter->Next()));
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
// JavaScript Record Comparator
bool TJsRecSplitter::operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
	v8::HandleScope HandleScope;
    // prepare record objects
    v8::Persistent<v8::Object> JsRec1 = TJsRec::New(Js, TRec(Store, RecIdWgt1.Key), RecIdWgt1.Dat);
    v8::Persistent<v8::Object> JsRec2 = TJsRec::New(Js, TRec(Store, RecIdWgt2.Key), RecIdWgt2.Dat);
    // call JavaScript Comparator
    return Js->ExecuteBool(SplitterFun, JsRec1, JsRec2);
}

///////////////////////////////
// QMiner-JavaScript-Record-Set
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
	static v8::Persistent<v8::ObjectTemplate> Template;
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
		JsRegisterFunction(TmpTemp, deleteRecs);
		JsRegisterFunction(TmpTemp, toJSON);
		JsRegisterFunction(TmpTemp, map);
		JsRegisterFunction(TmpTemp, setunion);
		JsRegisterFunction(TmpTemp, setintersect);
		JsRegisterFunction(TmpTemp, setdiff);
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
	return Args.Holder();
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
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::reverse(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	JsRecSet->RecSet->Reverse();
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::sortById(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	JsRecSet->RecSet->SortById(Asc);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::sortByFq(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	JsRecSet->RecSet->SortByFq(Asc);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::sortByField(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const TStr SortFieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	const int SortFieldId = JsRecSet->Store->GetFieldId(SortFieldNm);
	const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 1, 0) > 0);
	JsRecSet->RecSet->SortByField(Asc, SortFieldId);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::sort(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    QmAssertR(Args.Length() == 1, "sort(..) expects one argument.");
    v8::Persistent<v8::Function> CmpFun = TJsRecSetUtil::GetArgFunPer(Args, 0);   
	JsRecSet->RecSet->SortCmp(TJsRecCmp(JsRecSet->Js, JsRecSet->Store, CmpFun));
	return Args.Holder();
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
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::filterByFq(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	const int MnFq = TJsRecSetUtil::GetArgInt32(Args, 0);
	const int MxFq = TJsRecSetUtil::GetArgInt32(Args, 1);
	JsRecSet->RecSet->FilterByFq(MnFq, MxFq);
	return Args.Holder();
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
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::filter(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    QmAssertR(Args.Length() == 1, "filter(..) expects one argument.");
    v8::Persistent<v8::Function> FilterFun = TJsRecSetUtil::GetArgFunPer(Args, 0);   
	JsRecSet->RecSet->FilterBy(TJsRecFilter(JsRecSet->Js, JsRecSet->Store, FilterFun));
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::split(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    QmAssertR(Args.Length() == 1, "split(..) expects one argument.");
    v8::Persistent<v8::Function> SplitterFun = TJsRecSetUtil::GetArgFunPer(Args, 0);   
	TRecSetV RecSetV = JsRecSet->RecSet->SplitBy(TJsRecSplitter(JsRecSet->Js, JsRecSet->Store, SplitterFun));
    // prepare result array
    v8::Local<v8::Array> JsRecSetV = v8::Array::New(RecSetV.Len());
    for (int RecSetN = 0; RecSetN < RecSetV.Len(); RecSetN++) {
        JsRecSetV->Set(RecSetN, TJsRecSet::New(JsRecSet->Js, RecSetV[RecSetN]));
    }
	return HandleScope.Close(JsRecSetV);
}

v8::Handle<v8::Value> TJsRecSet::deleteRecs(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
    TUInt64Set RecIdSet; RecSet->GetRecIdSet(RecIdSet);
    JsRecSet->RecSet->RemoveRecIdSet(RecIdSet);
	return Args.Holder();
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

v8::Handle<v8::Value> TJsRecSet::map(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	PRecSet RecSet = JsRecSet->RecSet;
	QmAssertR(TJsRecSetUtil::IsArgFun(Args, 0), "map: Argument 0 is not a function!");
	v8::Handle<v8::Function> CallbackFun = TJsRecSetUtil::GetArgFun(Args, 0);
	// iterate through the recset
	const uint64 Recs = RecSet->GetRecs();
	for (uint64 RecIdx = 0; RecIdx < Recs; RecIdx++) {
		TRec Rec = RecSet->GetRec((int)RecIdx);
		v8::Handle<v8::Value> RecArg = TJsRec::New(JsRecSet->Js, Rec, RecSet->GetRecFq((int)RecIdx));
		v8::Handle<v8::Value> IdxArg = v8::Integer::New((int)RecIdx);
		// execute callback
		JsRecSet->Js->Execute(CallbackFun, RecArg, IdxArg);
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecSet::setintersect(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setintersect: the record sets do not point to the same store!");
	// Coputation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records in RecIdSet
	TRecFilterByRecIdSet Filter(RecIdSet, true);
	RecSet2->FilterBy(Filter);
	return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

v8::Handle<v8::Value> TJsRecSet::setunion(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setunion: the record sets do not point to the same store!");
	// GetMerge sorts the argument!
	PRecSet RecSet1Clone = RecSet1->Clone();
	PRecSet RecSet2 = JsRecSet->RecSet->GetMerge(RecSet1Clone);
	return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

v8::Handle<v8::Value> TJsRecSet::setdiff(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setdiff: the record sets do not point to the same store!");
	// Computation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records NOT in RecIdSet
	RecSet2->FilterBy(TRecFilterByRecIdSet(RecIdSet, false));
	return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

///////////////////////////////
// QMiner-JavaScript-Record
TVec<v8::Persistent<v8::ObjectTemplate> > TJsRec::TemplateV;

v8::Handle<v8::ObjectTemplate> TJsRec::GetTemplate(const TWPt<TBase>& Base, const TWPt<TStore>& Store) {
	v8::HandleScope HandleScope;
	// initialize template vector on the first call
	if (TemplateV.Empty()) {
		// reserve space for maximal number of stores
		TemplateV.Gen(TEnv::GetMxStores());
	}
	// make sure template id is a valid
    const uint StoreId = Store->GetStoreId();
	QmAssert(StoreId < TEnv::GetMxStores());
	// initialize template if not already prepared
	if (TemplateV[(int)StoreId].IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsLongRegisterProperty(TmpTemp, "$id", id);
		JsLongRegisterProperty(TmpTemp, "$name", name);
		JsLongRegisterProperty(TmpTemp, "$fq", fq);
		JsLongRegisterProperty(TmpTemp, "$store", store);
		JsRegisterFunction(TmpTemp, toJSON);
		JsRegisterFunction(TmpTemp, addJoin);
		JsRegisterFunction(TmpTemp, delJoin);
		// register all the fields
		for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
            TStr FieldNm = Store->GetFieldDesc(FieldN).GetFieldNm();
            JsRegisterSetProperty(TmpTemp, FieldNm.CStr(), getField, setField);
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

v8::Handle<v8::Value> TJsRec::store(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	return HandleScope.Close(TJsStore::New(JsRec->Js, JsRec->Store()));
}

v8::Handle<v8::Value> TJsRec::getField(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	const TRec& Rec = JsRec->Rec; 
    const TWPt<TStore>& Store = JsRec->Store;
    TStr FieldNm = TJsRecUtil::GetStr(Properties);
	const int FieldId = Store->GetFieldId(FieldNm);
	// check if null
    if (Rec.IsFieldNull(FieldId)) { return HandleScope.Close(v8::Null()); }
	// not null, get value
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
		const int Val = Rec.GetFieldInt(FieldId);
		return HandleScope.Close(v8::Integer::New(Val));
    } else if (Desc.IsIntV()) {
        v8::Persistent<v8::Object> JsIntV = TJsIntV::New(JsRec->Js);
        Rec.GetFieldIntV(FieldId, TJsIntV::GetVec(JsIntV));
        return JsIntV;
    } else if (Desc.IsUInt64()) {
		const uint64 Val = Rec.GetFieldUInt64(FieldId);
		return HandleScope.Close(v8::Integer::New((int)Val));
	} else if (Desc.IsStr()) {
		const TStr Val = Rec.GetFieldStr(FieldId);
		return HandleScope.Close(v8::String::New(Val.CStr()));
    } else if (Desc.IsStrV()) {
        TStrV StrV; Rec.GetFieldStrV(FieldId, StrV); 
		v8::Handle<v8::Array> JsStrV = v8::Array::New(StrV.Len());
		for (int StrN = 0; StrN < StrV.Len(); StrN++) {
			JsStrV->Set(StrN, v8::String::New(StrV[StrN].CStr()));
		}
		return HandleScope.Close(JsStrV);
	} else if (Desc.IsBool()) {
		const bool Val = Rec.GetFieldBool(FieldId);
		return HandleScope.Close(v8::Boolean::New(Val));
	} else if (Desc.IsFlt()) {
		const double Val = Rec.GetFieldFlt(FieldId);
		return HandleScope.Close(v8::Number::New(Val));
	} else if (Desc.IsFltPr()) {
		const TFltPr FltPr = Rec.GetFieldFltPr(FieldId);
		v8::Handle<v8::Array> JsFltPr = v8::Array::New(2);
		JsFltPr->Set(0, v8::Number::New(FltPr.Val1));
		JsFltPr->Set(1, v8::Number::New(FltPr.Val2));
		return HandleScope.Close(JsFltPr);    
	} else if (Desc.IsFltV()) {
        v8::Persistent<v8::Object> JsFltV = TJsFltV::New(JsRec->Js);
        Rec.GetFieldFltV(FieldId, TJsFltV::GetVec(JsFltV)); 
        return JsFltV;        
	} else if (Desc.IsTm()) {
        TTm FieldTm; Rec.GetFieldTm(FieldId, FieldTm); 
		if (FieldTm.IsDef()) { 
			return TJsTm::New(FieldTm);
		} else { 
			return HandleScope.Close(v8::Null());
		}
	} else if (Desc.IsNumSpV()) {
        v8::Persistent<v8::Object> JsSpV = TJsSpV::New(JsRec->Js);
        Rec.GetFieldNumSpV(FieldId, TJsSpV::GetSpV(JsSpV)); 
        return JsSpV;
	} else if (Desc.IsBowSpV()) {
		return HandleScope.Close(v8::Null()); //TODO
    }
	throw TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TJsRec::setField(v8::Local<v8::String> Properties,
        v8::Local<v8::Value> Value, const v8::AccessorInfo& Info) {
    
	v8::HandleScope HandleScope;
	TJsRec* JsRec = TJsRecUtil::GetSelf(Info);
	TRec& Rec = JsRec->Rec;
    const TWPt<TStore>& Store = JsRec->Store;
    TStr FieldNm = TJsRecUtil::GetStr(Properties);
	const int FieldId = Store->GetFieldId(FieldNm);
	//TODO: for now we don't support by-value records, fix this
    QmAssertR(Rec.IsByRef(), "Only records by reference (from stores) supported for setters.");
	// not null, get value
	const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Value->IsNull()) {
        QmAssertR(Desc.IsNullable(), "Field " + FieldNm + " not nullable");
        Rec.SetFieldNull(FieldId);
    } else if (Desc.IsInt()) {
        QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not int");
        const int Int = Value->Int32Value();
        Rec.SetFieldInt(FieldId, Int);
    } else if (Desc.IsIntV()) {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
    } else if (Desc.IsUInt64()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not uint64");
        const uint64 UInt64 = (uint64)Value->IntegerValue();
        Rec.SetFieldUInt64(FieldId, UInt64);
	} else if (Desc.IsStr()) {
        QmAssertR(Value->IsString(), "Field " + FieldNm + " not string");
        v8::String::Utf8Value Utf8(Value);
        Rec.SetFieldStr(FieldId, TStr(*Utf8));
    } else if (Desc.IsStrV()) {
        QmAssertR(Value->IsArray(), "Field " + FieldNm + " not array");
        v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Value);
        TStrV StrV;
        for (uint32_t StrN = 0; StrN < Array->Length(); StrN++) {
            v8::String::Utf8Value Utf8(Array->Get(StrN));
            StrV.Add(TStr(*Utf8));
        }
        Rec.SetFieldStrV(FieldId, StrV);
	} else if (Desc.IsBool()) {
        QmAssertR(Value->IsBoolean(), "Field " + FieldNm + " not boolean");
        Rec.SetFieldBool(FieldId, Value->BooleanValue());
	} else if (Desc.IsFlt()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not numeric");
        Rec.SetFieldFlt(FieldId, Value->NumberValue());
	} else if (Desc.IsFltPr()) {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
	} else if (Desc.IsFltV()) {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
	} else if (Desc.IsTm()) {
        QmAssertR(Value->IsObject() || Value->IsString(), "Field " + FieldNm + " not object or string");            
        if (Value->IsObject()){
            TJsTm* JsTm = TJsObjUtil<TJsTm>:: GetSelf(Value->ToObject());                
            Rec.SetFieldTm(FieldId, JsTm->Tm);
        } else if (Value->IsString()){
            v8::String::Utf8Value Utf8(Value);                
            Rec.SetFieldTm(FieldId, TTm::GetTmFromWebLogDateTimeStr(TStr(*Utf8), '-', ':', '.', 'T'));            
        }        
	} else if (Desc.IsNumSpV()) {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
	} else if (Desc.IsBowSpV()) {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
    } else {
        throw TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
    }
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
	return Args.Holder();
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
	return Args.Holder();
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
v8::Handle<v8::ObjectTemplate> TJsIndexKey::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
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
	const uint StoreId = JsIndexKey->IndexKey.GetStoreId();
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
    if (!JsIndexKey->IndexKey.IsWordVoc()) { return v8::Null(); }
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
    if (!JsIndexKey->IndexKey.IsWordVoc()) { return v8::Null(); }
	TStrIntPrV KeyValV;
	JsIndexKey->Js->Base->GetIndexVoc()->GetAllWordStrFqV(JsIndexKey->IndexKey.GetKeyId(), KeyValV);
	v8::Local<v8::Array> KeyValV8 = v8::Array::New(KeyValV.Len());
	for (int WordN = 0; WordN < KeyValV.Len(); WordN++) {		
		KeyValV8->Set(v8::Number::New(WordN), v8::String::New(KeyValV[WordN].Val2.GetStr().CStr()));		
	}
	return HandleScope.Close(KeyValV8);
}

///////////////////////////////
// QMiner-LinAlg
v8::Handle<v8::ObjectTemplate> TJsLinAlg::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, newVec);
		JsRegisterFunction(TmpTemp, newIntVec);
		JsRegisterFunction(TmpTemp, newMat);
		JsRegisterFunction(TmpTemp, newSpVec);
		JsRegisterFunction(TmpTemp, newSpMat);
		JsRegisterFunction(TmpTemp, svd);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsLinAlg::newVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	
	TFltV Vec;
	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		int Length = Array->Length();		
		Vec.Gen(Length, 0);
		for (int ElN = 0; ElN < Length; ElN++) {			
			Vec.Add(Array->Get(ElN)->NumberValue());
		}
	}
	else {
		if (Args[0]->IsObject()) {
			// we got another vector as paremeter, make a copy of it
			if (TJsLinAlgUtil::IsArgClass(Args, 0, "TFltV")) {
				// get argument vector
				TJsFltV* JsVec2 = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				Vec = JsVec2->Vec;
			}
			else {
				// we have object with parameters, parse them out
				int MxVals = TJsLinAlgUtil::GetArgInt32(Args, 0, "mxVals", -1);
				int Vals = TJsLinAlgUtil::GetArgInt32(Args, 0, "vals", 0);
				if (MxVals >= 0) {
					Vec.Gen(MxVals, Vals);
				}
				else {
					Vec.Gen(Vals);
				}
			}
		}
	}
	v8::Persistent<v8::Object> JsVec = TJsFltV::New(JsLinAlg->Js, Vec);
	return HandleScope.Close(JsVec);	
}

v8::Handle<v8::Value> TJsLinAlg::newIntVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	TIntV Vec;
	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		int Length = Array->Length();		
		Vec.Gen(Length, 0);
		for (int ElN = 0; ElN < Length; ElN++) {			
			Vec.Add(Array->Get(ElN)->Int32Value());
		}
	}
	else {
		if (Args[0]->IsObject()) {
			// we got another vector as paremeter, make a copy of it
			if (TJsLinAlgUtil::IsArgClass(Args, 0, "TIntV")) {
				// get argument vector
				TJsIntV* JsVec2 = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 0);
				Vec = JsVec2->Vec;
			}
			else {
				// we have object with parameters, parse them out
				int MxVals = TJsLinAlgUtil::GetArgInt32(Args, 0, "mxVals", -1);
				int Vals = TJsLinAlgUtil::GetArgInt32(Args, 0, "vals", -1);
				if (MxVals > 0 && Vals >= 0) {
					Vec.Gen(MxVals, Vals);
				}
				if (MxVals == -1 && Vals >= 0) {
					Vec.Gen(Vals);
				}
			}
		}
	}
	v8::Persistent<v8::Object> JsVec = TJsIntV::New(JsLinAlg->Js, Vec);
	return HandleScope.Close(JsVec);
}

v8::Handle<v8::Value> TJsLinAlg::newMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	TFltVV Mat;

	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
		int Rows = Array->Length();		
		if (Rows > 0) {
			// are the objects arrays
			int Cols;
			for (int RowN = 0; RowN < Rows; RowN++) {	
				if (!Array->Get(RowN)->IsArray()) {
					throw TQmExcept::New("object is not an array of arrays in TJsLinAlg::newMat()");							
				} else {
					v8::Handle<v8::Array> Row = v8::Handle<v8::Array>::Cast(Array->Get(RowN));
					if (RowN == 0) {
						Cols = Row->Length();
						Mat.Gen(Rows, Cols);
					} else {
						if ((int)Row->Length() != Cols) {							
							throw TQmExcept::New("inconsistent number of columns in TJsLinAlg::newMat()");							
						}
					}
					for (int ColN = 0; ColN < Cols; ColN++) {
						Mat.PutXY(RowN, ColN, Row->Get(ColN)->NumberValue());
					}
				}
			}				
		}
	}
	else {
		if (Args[0]->IsObject()) {
			if (TJsLinAlgUtil::IsArgClass(Args, 0, "TFltVV")) {
				// get argument matrix
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				Mat = JsMat2->Mat;
			}
			else {
				bool GenRandom = TJsLinAlgUtil::GetArgBool(Args, 0, "random", false);
				int Cols = TJsLinAlgUtil::GetArgInt32(Args, 0, "cols", 0);
				int Rows = TJsLinAlgUtil::GetArgInt32(Args, 0, "rows", 0);
				if (Cols > 0 && Rows > 0) {
					Mat.Gen(Rows, Cols);
					if (GenRandom) {
						TLAMisc::FillRnd(Mat);
					}
				}
			}
		}
	}

	v8::Persistent<v8::Object> JsMat = TJsFltVV::New(JsLinAlg->Js, Mat);
	return HandleScope.Close(JsMat);
}

v8::Handle<v8::Value> TJsLinAlg::newSpVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	int Dim = -1;
	TIntFltKdV Vec;
	if (Args.Length() > 0) {
		if (Args[0]->IsArray()) {
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
			int Length = Array->Length();		
			Vec.Gen(Length, 0);
			for (int ElN = 0; ElN < Length; ElN++) {			
				if (Array->Get(ElN)->IsArray()) {
					v8::Handle<v8::Array> KdPair = v8::Handle<v8::Array>::Cast(Array->Get(ElN));
					if (KdPair->Length() >= 2) {
						if (KdPair->Get(0)->IsInt32() && KdPair->Get(1)->IsNumber()) {
							Vec.Add(TIntFltKd(KdPair->Get(0)->Int32Value(), KdPair->Get(1)->NumberValue()));
						}
					}
				}				
			}			
			if (Args.Length() > 1 && Args[1]->IsObject()) {
				Dim = TJsLinAlgUtil::GetArgInt32(Args, 1, "dim", -1);
			}
			Vec.Sort();
		} else if (Args[0]->IsObject()) {		
			Dim = TJsLinAlgUtil::GetArgInt32(Args, 0, "dim", -1);
		}
	}
	v8::Persistent<v8::Object> JsSpVec = TJsSpV::New(JsLinAlg->Js, Vec, Dim);
	return HandleScope.Close(JsSpVec);
}

v8::Handle<v8::Value> TJsLinAlg::newSpMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);	
	int Rows = -1;	
	TVec<TIntFltKdV> Mat;	
	if (Args.Length() > 0) {
		// corrdinate
		if (Args.Length() >= 3 && TJsObjUtil<TJsSpMat>::IsArgClass(Args, 0, "TIntV") && TJsObjUtil<TJsSpMat>::IsArgClass(Args, 1, "TIntV") && TJsObjUtil<TJsSpMat>::IsArgClass(Args, 2, "TFltV")) {
			TJsIntV* RowIdxV = TJsObjUtil<TQm::TJsVec<TInt, TAuxIntV> >::GetArgObj(Args, 0);
			TJsIntV* ColIdxV = TJsObjUtil<TQm::TJsVec<TInt, TAuxIntV> >::GetArgObj(Args, 1);
			TJsFltV* ValV = TJsObjUtil<TQm::TJsVec<TFlt, TAuxFltV> >::GetArgObj(Args, 2);
			int Cols = -1;
			if (Args.Length() == 4) {
				Cols = TJsObjUtil<TQm::TJsVec<TFlt, TAuxFltV> >::GetArgInt32(Args, 3, -1);
			}
			if (Cols == -1) {
				Cols = ColIdxV->Vec.GetMxVal() + 1;
			}
			TSparseOps<TInt, TFlt>::CoordinateCreateSparseColMatrix(RowIdxV->Vec, ColIdxV->Vec, ValV->Vec, Mat, Cols);
		}
		else {
			if (Args[0]->IsArray()) {
				// javascript arrays
				v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
				int Cols = Array->Length();
				Mat.Gen(Cols);
				for (int ColN = 0; ColN < Cols; ColN++) {
					if (Array->Get(ColN)->IsArray()) {
						v8::Handle<v8::Array> SpVecArray = v8::Handle<v8::Array>::Cast(Array->Get(ColN));
						int Els = SpVecArray->Length();
						for (int ElN = 0; ElN < Els; ElN++) {
							if (SpVecArray->Get(ElN)->IsArray()) {
								v8::Handle<v8::Array> KdPair = v8::Handle<v8::Array>::Cast(SpVecArray->Get(ElN));
								if (KdPair->Length() >= 2) {
									if (KdPair->Get(0)->IsInt32() && KdPair->Get(1)->IsNumber()) {
										Mat[ColN].Add(TIntFltKd(KdPair->Get(0)->Int32Value(), KdPair->Get(1)->NumberValue()));
									}
								}
							}
						}
					}
					Mat[ColN].Sort();
				}
				if (Args.Length() > 1 && Args[1]->IsObject()) {
					Rows = TJsLinAlgUtil::GetArgInt32(Args, 1, "rows", -1);
				}
			}
			else {
				if (Args[0]->IsObject()) {
					Rows = TJsLinAlgUtil::GetArgInt32(Args, 0, "rows", -1);
					int Cols = TJsLinAlgUtil::GetArgInt32(Args, 0, "cols", 0);
					Mat.Gen(Cols);
				}
			}
		}
	}

	v8::Persistent<v8::Object> JsSpMat = TJsSpMat::New(JsLinAlg->Js, Mat, Rows);
	return HandleScope.Close(JsSpMat);	
}

v8::Handle<v8::Value> TJsLinAlg::svd(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsLinAlg* JsLinAlg = TJsLinAlgUtil::GetSelf(Args);
	
	v8::Persistent<v8::Object> JsU = TJsFltVV::New(JsLinAlg->Js);
	TFltVV& U = TJsFltVV::GetFltVV(JsU);
	v8::Persistent<v8::Object> JsV = TJsFltVV::New(JsLinAlg->Js);
	TFltVV& V = TJsFltVV::GetFltVV(JsV);
	v8::Persistent<v8::Object> Jss = TJsFltV::New(JsLinAlg->Js);
	TFltV& s = TJsFltV::GetVec(Jss);

	v8::Handle<v8::Object> JsObj = v8::Object::New();
	v8::Persistent<v8::Object> JsResult = v8::Persistent<v8::Object>::New(v8::Isolate::GetCurrent(), JsObj);
	
	if (Args.Length() > 1) {
		int Iters = 2;
		double Tol = 1e-6;
		if (Args.Length() > 2) {			
			PJsonVal ParamVal = TJsLinAlgUtil::GetArgJson(Args, 2);   
			Iters = ParamVal->GetObjInt("iter", 2);
			Tol = ParamVal->GetObjNum("tol", 1e-6);
		}
		if (Args[0]->IsObject() && Args[1]->IsInt32()) {
			int k = Args[1]->Int32Value();
			if (TJsLinAlgUtil::IsArgClass(Args, 0, "TFltVV")) {
				// get argument matrix
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				TFullMatrix Mat2(JsMat2->Mat);
				TLinAlg::ComputeThinSVD(Mat2, k, U, s, V, Iters, Tol);		
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("U")), JsU);
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("V")), JsV);
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("s")), Jss);
				return HandleScope.Close(JsResult);
			}
			if (TJsLinAlgUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
				// get argument matrix
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				if (JsMat2->Rows != -1) {					
					TSparseColMatrix Mat2(JsMat2->Mat, JsMat2->Rows, JsMat2->Mat.Len());
					TLinAlg::ComputeThinSVD(Mat2, k, U, s, V, Iters, Tol);					
				} else {
					TSparseColMatrix Mat2(JsMat2->Mat);
					TLinAlg::ComputeThinSVD(Mat2, k, U, s, V, Iters, Tol);					
				}
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("U")), JsU);
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("V")), JsV);
				JsResult->Set(v8::Handle<v8::String>(v8::String::New("s")), Jss);
				return HandleScope.Close(JsResult);
			}
		}
	}
	return HandleScope.Close(v8::Undefined());
}

///////////////////////////////
// QMiner-Vector
const TStr TAuxFltV::ClassId = "TFltV";
const TStr TAuxIntV::ClassId = "TIntV";

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::pushV(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	if (Args[0]->IsObject()) {
		if (TJsVecUtil::IsArgClass(Args, 0, "TFltV")) {
			TJsFltV* JsVec2 = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
			JsVec->Vec.AddV(JsVec2->Vec);
			return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));	
		}
	}
	return HandleScope.Close(v8::Undefined());
}


template <>
v8::Handle<v8::Value> TJsVec<TInt, TAuxIntV>::pushV(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	if (Args[0]->IsObject()) {
		if (TJsVecUtil::IsArgClass(Args, 0, "TIntV")) {
			TJsIntV* JsVec2 = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 0);
			JsVec->Vec.AddV(JsVec2->Vec);
			return HandleScope.Close(v8::Integer::New(JsVec->Vec.Len()));	
		}
	}
	return HandleScope.Close(v8::Undefined());
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::sortPerm(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	bool Asc = true;
	if (Args.Length() > 0) {
		if (Args[0]->IsBoolean()) {
			Asc = Args[0]->BooleanValue();
		}
	}

	v8::Persistent<v8::Object> JsSortedV = TJsFltV::New(JsVec->Js);
	TFltV& SortedV = TJsFltV::GetVec(JsSortedV);
	v8::Persistent<v8::Object> JsPermV = TJsIntV::New(JsVec->Js);
	TVec<int>& PermV = TJsVec<int, TAuxIntV>::GetVec(JsPermV);

	TVec<TFlt>::SortGetPerm(JsVec->Vec, SortedV, PermV, Asc);

	v8::Handle<v8::Object> JsObj = v8::Object::New();
	v8::Persistent<v8::Object> JsResult = v8::Persistent<v8::Object>::New(v8::Isolate::GetCurrent(), JsObj);
	JsResult->Set(v8::Handle<v8::String>(v8::String::New("vec")), JsSortedV);
	JsResult->Set(v8::Handle<v8::String>(v8::String::New("perm")), JsPermV);
	return HandleScope.Close(JsResult);
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::outer(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsVecUtil::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsObjUtil<TJsVec>::IsArgClass(Args, 0, "TFltV")) {
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

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::inner(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV =  TJsObjUtil<TJsVec>::GetSelf(Args);
	double result = 0.0;
	if (Args[0]->IsObject()) {
		if ( TJsObjUtil<TJsVec>::IsArgClass(Args, 0, "TFltV")) {
			TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
			QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector' * vector: dimensions mismatch");
			result = TLinAlg::DotProduct(JsFltV->Vec, JsVec->Vec);
		}
	}
	return HandleScope.Close(v8::Number::New(result));
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::plus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV =  TJsObjUtil<TJsVec>::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if ( TJsObjUtil<TJsVec>::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector + vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
				TFltV& Result = TJsVec::GetVec(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len());
				TLinAlg::LinComb(1.0, JsFltV->Vec, 1.0, JsVec->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
			if ( TJsObjUtil<TJsVec>::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() >= JsVec->Dim, "vector + sp_vector: dimensions mismatch");
				if (JsVec->Dim == -1) {
					QmAssertR(JsFltV->Vec.Len() >= TLAMisc::GetMaxDimIdx(JsVec->Vec) + 1, "vector + sp_vector: index overflow");
				}
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
				TFltV& Result = TJsVec::GetVec(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len());
				TLinAlg::AddVec(1.0, JsVec->Vec, JsFltV->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::minus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsObjUtil<TJsVec>::GetSelf(Args);
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {
			if (TJsObjUtil<TJsVec>::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsFltV->Vec.Len() == JsVec->Vec.Len(), "vector - vector: dimensions mismatch");
				// create JS result and get the internal data				
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
				TFltV& Result = TJsVec::GetVec(JsResult);
				// computation
				Result.Gen(JsFltV->Vec.Len());
				TLinAlg::LinComb(1.0, JsFltV->Vec, -1.0, JsVec->Vec, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::multiply(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsObjUtil<TJsVec>::GetSelf(Args);
	if (TJsObjUtil<TJsVec>::IsArgFlt(Args, 0)) {		
		double Scalar = TJsObjUtil<TJsVec>::GetArgFlt(Args, 0);
		// create output object
		v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltV->Js);
		// get the internal glib vector
		TFltV& Result = TJsVec::GetVec(JsResult);
		// computation
		Result.Gen(JsFltV->Vec.Len());
		TLinAlg::MultiplyScalar(Scalar, JsFltV->Vec, Result);
		
		return HandleScope.Close(JsResult);
	}
	return HandleScope.Close(v8::Undefined());
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::normalize(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsObjUtil<TJsVec>::GetSelf(Args);		
	if (JsFltV->Vec.Len() > 0) {
		TLinAlg::Normalize(JsFltV->Vec);
	}
	return Args.Holder();
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::diag(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsObjUtil<TJsVec>::GetSelf(Args);	
	v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsFltV->Js);
	TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
	// computation
	TLAMisc::Diag(JsFltV->Vec, Result);
	return HandleScope.Close(JsResult);	
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::spDiag(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltV* JsFltV = TJsObjUtil<TJsVec>::GetSelf(Args);	
	v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsFltV->Js);
	TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);
	// computation
	TLAMisc::Diag(JsFltV->Vec, Result);
	return HandleScope.Close(JsResult);	
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::norm(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	double result = TLinAlg::Norm(JsVec->Vec);
	return HandleScope.Close(v8::Number::New(result));
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::sparse(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	TIntFltKdV Res;
	TLAMisc::ToSpVec(JsVec->Vec, Res);		
	v8::Persistent<v8::Object> JsResult = TJsSpV::New(JsVec->Js, Res);
	TJsSpV::SetDim(JsResult, JsVec->Vec.Len());
	return HandleScope.Close(JsResult);	
}

template <>
v8::Handle<v8::Value> TJsVec<TFlt, TAuxFltV>::toMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	TFltVV Res(JsVec->Vec, JsVec->Vec.Len(), 1);
	v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsVec->Js, Res);
	return HandleScope.Close(JsResult);
}


template <>
v8::Handle<v8::Value> TJsVec<TInt, TAuxIntV>::toMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsVec* JsVec = TJsObjUtil<TJsVec>::GetSelf(Args);
	int Len = JsVec->Vec.Len();
	TFltV Temp(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		Temp[ElN] = JsVec->Vec[ElN];
	}
	TFltVV Res(Temp, Len, 1);
	v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsVec->Js, Res);
	return HandleScope.Close(JsResult);
}

///////////////////////////////
// QMiner-FltVV

v8::Handle<v8::ObjectTemplate> TJsFltVV::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, at);
		JsRegisterFunction(TmpTemp, put);
		JsRegisterFunction(TmpTemp, multiply);
		JsRegisterFunction(TmpTemp, multiplyT);
		JsRegisterFunction(TmpTemp, plus);
		JsRegisterFunction(TmpTemp, minus);
		JsRegisterFunction(TmpTemp, transpose);
		JsRegisterFunction(TmpTemp, solve);
		JsRegisterFunction(TmpTemp, rowNorms);
		JsRegisterFunction(TmpTemp, colNorms);
		JsRegisterFunction(TmpTemp, normalizeCols);
		JsRegisterFunction(TmpTemp, sparse);
		JsRegisterFunction(TmpTemp, frob);
		JsRegisterProperty(TmpTemp, rows);
		JsRegisterProperty(TmpTemp, cols);
		JsRegisterFunction(TmpTemp, printStr);
		JsRegisterFunction(TmpTemp, print);
		JsRegisterFunction(TmpTemp, rowMaxIdx);
		JsRegisterFunction(TmpTemp, colMaxIdx);
		JsRegisterFunction(TmpTemp, getCol);
		JsRegisterFunction(TmpTemp, setCol);
		JsRegisterFunction(TmpTemp, getRow);
		JsRegisterFunction(TmpTemp, setRow);
		JsRegisterFunction(TmpTemp, diag);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
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
	return Args.Holder();
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
				// computation				
				TFltV Result(JsMat->Mat.GetRows());				
				TLinAlg::Multiply(JsMat->Mat, JsVec->Vec, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result);
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

			if (TJsFltVVUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() > TLAMisc::GetMaxDimIdx(JsVec->Vec), "matrix * sparse_vector: dimensions mismatch");
				int Rows = JsMat->Mat.GetRows();
				TFltVV Result(Rows, 1);
				// Copy could be omitted if we implemented SparseColMat * SparseVec
				TVec<TIntFltKdV> TempSpMat(1);				
				TempSpMat[0] = JsVec->Vec;				
				TLinAlg::Multiply(JsMat->Mat, TempSpMat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);			
			}

			if (TJsFltVVUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetCols() >= JsMat2->Rows, "matrix * sparse_col_matrix: dimensions mismatch");
				// computation				
				int Rows = JsMat->Mat.GetRows();
				int Cols = JsMat->Mat.GetCols();
				if (JsMat2->Rows == -1) {
					QmAssertR(Cols > TLAMisc::GetMaxDimIdx(JsMat2->Mat), "matrix * sparse_col_matrix: dimensions mismatch");
				}
				TFltVV Result(Rows, JsMat2->Mat.Len());					
				TLinAlg::Multiply(JsMat->Mat, JsMat2->Mat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::multiplyT(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgFlt(Args, 0)) {
			double Scalar = TJsFltVVUtil::GetArgFlt(Args, 0);
			// create JS result and get the internal data				
			v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
			TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
			// computation
			Result = JsMat->Mat;
			Result.Transpose();
			TLinAlg::MultiplyScalar(Scalar, Result, Result);
			return HandleScope.Close(JsResult);	
		}
		if (Args[0]->IsObject()) {
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetRows() == JsVec->Vec.Len(), "matrix' * vector: dimensions mismatch");
				// computation				
				TFltV Result(JsMat->Mat.GetCols());				
				TLinAlg::MultiplyT(JsMat->Mat, JsVec->Vec, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result);
				return HandleScope.Close(JsResult);			
			}
			if (TJsFltVVUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetRows() == JsMat2->Mat.GetRows(), "matrix' * matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetCols(), JsMat2->Mat.GetCols());
				TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
				return HandleScope.Close(JsResult);
			}

			if (TJsFltVVUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetRows() > TLAMisc::GetMaxDimIdx(JsVec->Vec), "matrix' * sparse_vector: dimensions mismatch");
				TFltVV Result(JsMat->Mat.GetCols(), 1);
				// Copy could be omitted if we implemented SparseColMat * SparseVec
				TVec<TIntFltKdV> TempSpMat(1);				
				TempSpMat[0] = JsVec->Vec;				
				TLinAlg::MultiplyT(JsMat->Mat, TempSpMat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);			
			}

			if (TJsFltVVUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.GetRows() >= JsMat2->Rows, "matrix' * sparse_col_matrix: dimensions mismatch");
				// computation				
				int Rows = JsMat->Mat.GetRows();
				int Cols = JsMat->Mat.GetCols();
				if (JsMat2->Rows == -1) {
					QmAssertR(Rows > TLAMisc::GetMaxDimIdx(JsMat2->Mat), "matrix' * sparse_col_matrix: dimensions mismatch");
				}
				TFltVV Result(Cols, JsMat2->Mat.Len());				
				TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js, Result);
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
				TFltV& Result = TJsFltV::GetVec(JsResult);
				// computation
				Result.Gen(JsMat->Mat.GetCols());
				TFltVV Mat2 = JsMat->Mat;
				TFltV Vec2 = JsVec->Vec;
				TNumericalStuff::SolveLinearSystem(Mat2, Vec2, Result);
				return HandleScope.Close(JsResult);			
			}			
		}
	}	
	return HandleScope.Close(v8::Undefined());		
}

v8::Handle<v8::Value> TJsFltVV::rowNorms(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltVV->Js);
	TFltV& Result = TJsFltV::GetVec(JsResult);	
	int Rows = JsFltVV->Mat.GetRows();
	int Cols = JsFltVV->Mat.GetCols();
	Result.Gen(Rows);	
	Result.PutAll(0.0);
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			Result[RowN] += TMath::Sqr(JsFltVV->Mat.At(RowN, ColN));
		}
		Result[RowN] = TMath::Sqrt(Result[RowN]);
	}
	return HandleScope.Close(JsResult);		
}

v8::Handle<v8::Value> TJsFltVV::colNorms(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsFltVV->Js);
	TFltV& Result = TJsFltV::GetVec(JsResult);	
	int Cols = JsFltVV->Mat.GetCols();
	Result.Gen(Cols);	
	Result.PutAll(0.0);
	for (int ColN = 0; ColN < Cols; ColN++) {
		Result[ColN] = TLinAlg::Norm(JsFltVV->Mat, ColN);
	}
	return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsFltVV::normalizeCols(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	TLinAlg::NormalizeColumns(JsFltVV->Mat);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFltVV::sparse(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	TVec<TIntFltKdV> SpMat = TVec<TIntFltKdV>();
	TLinAlg::Sparse(JsFltVV->Mat, SpMat);	
	v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsFltVV->Js, SpMat);
	TJsSpMat::SetRows(JsResult, JsFltVV->Mat.GetRows());
	return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsFltVV::frob(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	double FrobNorm = 0.0;
	int Cols = JsFltVV->Mat.GetCols();
	for (int ColN = 0; ColN < Cols; ColN++) {
		FrobNorm += TLinAlg::Norm2(JsFltVV->Mat, ColN);
	}
	return HandleScope.Close(v8::Number::New(TMath::Sqrt(FrobNorm)));
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

v8::Handle<v8::Value> TJsFltVV::printStr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	TStr Out = "";
	TLAMisc::PrintTFltVVToStr(JsFltVV->Mat, Out);
	return HandleScope.Close(v8::String::New(Out.CStr()));	
}

v8::Handle<v8::Value> TJsFltVV::print(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsFltVV = TJsFltVVUtil::GetSelf(Args);
	TLAMisc::PrintTFltVV(JsFltVV->Mat, "");	
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFltVV::rowMaxIdx(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgInt32(Args, 0)) {
			int RowN = TJsFltVVUtil::GetArgInt32(Args, 0);
			int Idx = TLinAlg::GetRowMaxIdx(JsMat->Mat, RowN);
			return HandleScope.Close(v8::Integer::New(Idx));
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::colMaxIdx(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgInt32(Args, 0)) {
			int ColN = TJsFltVVUtil::GetArgInt32(Args, 0);
			int Idx = TLinAlg::GetColMaxIdx(JsMat->Mat, ColN);
			return HandleScope.Close(v8::Integer::New(Idx));
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::getCol(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgInt32(Args, 0)) {
			int ColN = TJsFltVVUtil::GetArgInt32(Args, 0);
			QmAssertR(ColN < JsMat->Mat.GetCols() , "matrix: get col vector: index out of bounds");
			TFltV Result;
			JsMat->Mat.GetCol(ColN, Result);
			v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result);
			return HandleScope.Close(JsResult);	
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::setCol(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 1) {
		QmAssertR(Args[1]->IsObject() && TJsFltVVUtil::IsArgClass(Args, 1, "TFltV") && TJsFltVVUtil::IsArgInt32(Args, 0), "arg[0] should be an integer, arg[1] should be a TFltV");
		TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 1);
		int ColN = TJsFltVVUtil::GetArgInt32(Args, 0);	
		QmAssertR(JsMat->Mat.GetRows() == JsVec->Vec.Len() , "matrix: set col vector: dimensions mismatch");
		QmAssertR(ColN < JsMat->Mat.GetCols() , "matrix: set col vector: index out of bounds");
		int Rows = JsMat->Mat.GetRows();
		for (int RowN = 0; RowN < Rows; RowN++) {
			JsMat->Mat.At(RowN, ColN) = JsVec->Vec[RowN];
		}
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFltVV::getRow(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsFltVVUtil::IsArgInt32(Args, 0)) {
			int RowN = TJsFltVVUtil::GetArgInt32(Args, 0);
			QmAssertR(RowN < JsMat->Mat.GetRows() , "matrix: get row vector: index out of bounds");
			TFltV Result;
			JsMat->Mat.GetRow(RowN, Result);
			v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result);
			return HandleScope.Close(JsResult);	
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::setRow(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	if (Args.Length() > 1) {
		QmAssertR(Args[1]->IsObject() && TJsFltVVUtil::IsArgClass(Args, 1, "TFltV") && TJsFltVVUtil::IsArgInt32(Args, 0), "arg[0] should be an integer, arg[1] should be a TFltV");
		TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 1);
		int RowN = TJsFltVVUtil::GetArgInt32(Args, 0);	
		QmAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len() , "matrix: set row vector: dimensions mismatch");
		QmAssertR(RowN < JsMat->Mat.GetRows() , "matrix: set row vector: index out of bounds");
		int Cols = JsMat->Mat.GetCols();
		for (int ColN = 0; ColN < Cols; ColN++) {
			JsMat->Mat.At(RowN, ColN) = JsVec->Vec[ColN];
		}
	}
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsFltVV::diag(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFltVV* JsMat = TJsFltVVUtil::GetSelf(Args);
	QmAssertR(JsMat->Mat.GetRows() == JsMat->Mat.GetCols(), "matrix: diag: square matrix expected! ");
	TFltV Result(JsMat->Mat.GetRows());
	for (int ElN = 0; ElN < JsMat->Mat.GetRows(); ElN++) {
		Result[ElN] = JsMat->Mat.At(ElN, ElN);
	}
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result);
	return HandleScope.Close(JsResult);
	return HandleScope.Close(v8::Undefined());
}



///////////////////////////////
// QMiner-SparseVec

v8::Handle<v8::ObjectTemplate> TJsSpV::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, at);
		JsRegisterFunction(TmpTemp, put);
		JsRegisterFunction(TmpTemp, sum);
		JsRegisterFunction(TmpTemp, inner);
		JsRegisterFunction(TmpTemp, multiply);
		JsRegisterFunction(TmpTemp, normalize);
		JsRegisterProperty(TmpTemp, nnz);
		JsRegisterProperty(TmpTemp, dim);
		JsRegisterFunction(TmpTemp, print);
		JsRegisterFunction(TmpTemp, norm);
		JsRegisterFunction(TmpTemp, full);
		JsRegisterFunction(TmpTemp, valVec);
		JsRegisterFunction(TmpTemp, idxVec);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsSpV::at(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	TInt Index = TJsSpVUtil::GetArgInt32(Args, 0);		
	QmAssertR(JsSpV->Dim == -1 || Index < JsSpV->Dim, "sparse vector at: index out of bounds");
	double result = 0.0;
	for (int ElN = 0; ElN < JsSpV->Vec.Len(); ElN++) {
		if (JsSpV->Vec[ElN].Key == Index) {
			result = JsSpV->Vec[ElN].Dat;
			break;
		}
	}
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsSpV::put(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	if (Args.Length() == 2) {
		QmAssertR(Args[0]->IsInt32(), "the first argument should be an integer");
		QmAssertR(Args[1]->IsNumber(), "the second argument should be a number");				
		TInt Index = TJsSpVUtil::GetArgInt32(Args, 0);	
		TFlt Val = TJsSpVUtil::GetArgFlt(Args, 1);	
		bool Found = false;
		for (int ElN = 0; ElN < JsSpV->Vec.Len(); ElN++) {
			if (JsSpV->Vec[ElN].Key == Index) {
				JsSpV->Vec[ElN].Dat = Val;
				Found = true;
				break;
			}
		}
		if (!Found) {
			JsSpV->Vec.Add(TIntFltKd(Index, Val));
			JsSpV->Vec.Sort();
		}		
		// update dimension
		if (JsSpV->Dim != -1) {
			if (Index >= JsSpV->Dim) {
				JsSpV->Dim = Index + 1;
			}
		}
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpV::sum(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	double result = 0.0;
	if (JsSpV->Vec.Len() > 0) {
		result = TLinAlg::SumVec(JsSpV->Vec);
	}
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsSpV::inner(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	double result = 0.0;
	if (Args[0]->IsObject()) {
		if (TJsSpVUtil::IsArgClass(Args, 0, "TFltV")) {
			TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
			QmAssertR(JsSpV->Dim == -1 || JsSpV->Dim == JsVec->Vec.Len(), "sparse_vector' * vector: dimensions mismatch");
			result = TLinAlg::DotProduct(JsVec->Vec, JsSpV->Vec);
		}
		if (TJsSpVUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
			TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
			QmAssertR(JsSpV->Dim == -1 || JsVec->Dim == -1 || JsSpV->Dim == JsVec->Dim, "sparse_vector' * sparse_vector: dimensions mismatch");
			result = TLinAlg::DotProduct(JsVec->Vec, JsSpV->Vec);
		}
	}
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsSpV::multiply(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	if (TJsSpVUtil::IsArgFlt(Args, 0)) {		
		double Scalar = TJsSpVUtil::GetArgFlt(Args, 0);
		// create output object
		v8::Persistent<v8::Object> JsResult = TJsSpV::New(JsSpV->Js);
		// get the internal glib vector
		TIntFltKdV& Result = TJsSpV::GetSpV(JsResult);
		// computation
		Result.Gen(JsSpV->Vec.Len());
		TLinAlg::MultiplyScalar(Scalar, JsSpV->Vec, Result);		
		return HandleScope.Close(JsResult);
	}
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsSpV::normalize(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);		
	if (JsSpV->Vec.Len() > 0) {
		TLinAlg::Normalize(JsSpV->Vec);
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpV::nnz(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsSpV->Vec.Len()));
}

v8::Handle<v8::Value> TJsSpV::dim(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsSpV->Dim));
}

v8::Handle<v8::Value> TJsSpV::print(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);		
	printf("Sparse vector dimension: %d\n", JsSpV->Dim);
	for (int ElN = 0; ElN < JsSpV->Vec.Len(); ElN++) {
		printf("%d %f\n", JsSpV->Vec[ElN].Key.Val, JsSpV->Vec[ElN].Dat.Val);
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpV::norm(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	double result = TLinAlg::Norm(JsSpV->Vec);
	return HandleScope.Close(v8::Number::New(result));
}

v8::Handle<v8::Value> TJsSpV::full(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	
	int Len = JsSpV->Dim;
	if (Args.Length() > 0) {
		if (TJsSpVUtil::IsArgInt32(Args, 0)) {		
			Len = TJsSpVUtil::GetArgInt32(Args, 0);
		}
	}
	if (Len == -1) {
		Len = TLAMisc::GetMaxDimIdx(JsSpV->Vec) + 1;
	}
	TFltV Res;
	TLAMisc::ToVec(JsSpV->Vec, Res, Len);		
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsSpV->Js, Res);
	return HandleScope.Close(JsResult);	
}


v8::Handle<v8::Value> TJsSpV::valVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	int NNZ = JsSpV->Vec.Len();
	TFltV Res(NNZ, 0);
	// copy values to Res
	for (int ElN = 0; ElN < NNZ; ElN++) {
		Res.Add(JsSpV->Vec[ElN].Dat);
	}
	// wrap result in JS
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsSpV->Js, Res);
	return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsSpV::idxVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpV* JsSpV = TJsSpVUtil::GetSelf(Args);
	int NNZ = JsSpV->Vec.Len();
	TIntV Res(NNZ, 0);
	// copy values to Res
	for (int ElN = 0; ElN < NNZ; ElN++) {
		Res.Add(JsSpV->Vec[ElN].Key);
	}
	// wrap result in JS
	v8::Persistent<v8::Object> JsResult = TJsIntV::New(JsSpV->Js, Res);
	return HandleScope.Close(JsResult);
}


///////////////////////////////
// QMiner-Sparse-Col-Matrix

v8::Handle<v8::ObjectTemplate> TJsSpMat::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, at);
		JsRegisterFunction(TmpTemp, put);
		JsRegGetSetIndexedProperty(TmpTemp, indexGet, indexSet);
		JsRegisterFunction(TmpTemp, push);
		JsRegisterFunction(TmpTemp, multiply);
		JsRegisterFunction(TmpTemp, multiplyT);
		JsRegisterFunction(TmpTemp, plus);
		JsRegisterFunction(TmpTemp, minus);
		JsRegisterFunction(TmpTemp, transpose);
		JsRegisterFunction(TmpTemp, colNorms);
		JsRegisterFunction(TmpTemp, normalizeCols);
		JsRegisterFunction(TmpTemp, full);
		JsRegisterFunction(TmpTemp, frob);
		JsRegisterProperty(TmpTemp, rows);
		JsRegisterProperty(TmpTemp, cols);
		JsRegisterFunction(TmpTemp, print);
		JsRegisterFunction(TmpTemp, save);
		JsRegisterFunction(TmpTemp, load);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsSpMat::at(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);	
	TInt Row = TJsSpMatUtil::GetArgInt32(Args, 0);	
	TInt Col = TJsSpMatUtil::GetArgInt32(Args, 1);
	TInt Rows = JsSpMat->Rows;
	TInt Cols = JsSpMat->Mat.Len();
	QmAssertR(Row >= 0 &&  (Row < Rows || Rows == -1) && Col >= 0 && Col < Cols, "sparse col matrix at: index out of bounds");		
	double result = 0.0;
	int Els = JsSpMat->Mat[Col].Len();
	for (int ElN = 0; ElN < Els; ElN++) {
		if (JsSpMat->Mat[Col][ElN].Key == Row) {
			result = JsSpMat->Mat[Col][ElN].Dat;
			break;
		}
	}
	return HandleScope.Close(v8::Number::New(result));

}

v8::Handle<v8::Value> TJsSpMat::put(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);	
	if (Args.Length() == 3) {
		QmAssertR(Args[0]->IsInt32(), "the first argument should be an integer");
		QmAssertR(Args[1]->IsInt32(), "the second argument should be an integer");
		QmAssertR(Args[2]->IsNumber(), "the third argument should be a number");
		TInt Row = TJsSpMatUtil::GetArgInt32(Args, 0);	
		TInt Col = TJsSpMatUtil::GetArgInt32(Args, 1);	
		TInt Rows = JsSpMat->Rows;
		TInt Cols = JsSpMat->Mat.Len();
		QmAssertR(Row >= 0 &&  (Row < Rows || Rows == -1) && Col >= 0 && Col < Cols, "sparse col matrix put: index out of bounds");		
		TFlt Val = TJsSpMatUtil::GetArgFlt(Args, 2);
		bool Found = false;
		int Els = JsSpMat->Mat[Col].Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			if (JsSpMat->Mat[Col][ElN].Key == Row) {
				JsSpMat->Mat[Col][ElN].Dat = Val;
				Found = true;
				break;
			}
		}
		if (!Found) {
			JsSpMat->Mat[Col].Add(TIntFltKd(Row, Val));
			JsSpMat->Mat[Col].Sort();
		}		
		// update dimension
		if (JsSpMat->Rows != -1) {
			if (Row >= JsSpMat->Rows) {
				JsSpMat->Rows = Row + 1;
			}
		}
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpMat::indexGet(uint32_t Index, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Info);	
	QmAssertR(Index < (uint32_t)JsSpMat->Mat.Len(), "sp matrix index at: index out of bounds");
	v8::Persistent<v8::Object> JsResult = TJsSpV::New(JsSpMat->Js, JsSpMat->Mat[Index], JsSpMat->Rows);	
	return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsSpMat::indexSet(uint32_t Index, v8::Local<v8::Value> Value, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Info);	
	QmAssertR(Index < (uint32_t)JsSpMat->Mat.Len(), "sp matrix index set: index out of bounds");
	v8::Handle<v8::Object> ValObj = v8::Handle<v8::Object>::Cast(Value);
	JsSpMat->Mat[Index] = TJsSpV::GetSpV(ValObj);	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsSpMat::push(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsMat = TJsSpMatUtil::GetSelf(Args);	
	if (Args.Length() > 0) {		
		if (Args[0]->IsObject()) {			
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				JsMat->Mat.Add(JsVec->Vec);
				if (JsMat->Rows.Val != -1) {
					JsMat->Rows = MAX(JsMat->Rows.Val, TLAMisc::GetMaxDimIdx(JsVec->Vec) + 1);
				}		
			}
		}
	}
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpMat::multiply(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsMat = TJsSpMatUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsSpMatUtil::IsArgFlt(Args, 0)) {
			double Scalar = TJsSpMatUtil::GetArgFlt(Args, 0);
			// create JS result and get the internal data				
			v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsMat->Js);
			TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);
			// computation			
			TLinAlg::MultiplyScalar(Scalar, JsMat->Mat, Result);
			return HandleScope.Close(JsResult);			
		}
		if (Args[0]->IsObject()) {
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.Len() == JsVec->Vec.Len(), "sparse_col_matrix * vector: dimensions mismatch");
				// computation				
				int Rows = JsMat->Rows;				
				if (JsMat->Rows == -1) {
					Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;
				}
				TFltVV Result(Rows, 1);
				// Copy could be omitted if we implemented SparseColMat * TFltV
				TLinAlg::Multiply(JsMat->Mat, TFltVV(JsVec->Vec, JsVec->Vec.Len(), 1), Result, Rows);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);			
			}
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);				
				QmAssertR(JsMat->Mat.Len() == JsMat2->Mat.GetRows(), "sparse_col_matrix * matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				int Rows = JsMat->Rows;
				if (Rows == -1) {
					Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;
				}
				Result.Gen(Rows, JsMat2->Mat.GetCols());
				TLinAlg::Multiply(JsMat->Mat, JsMat2->Mat, Result, Rows);
				return HandleScope.Close(JsResult);
			}
			
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.Len() > TLAMisc::GetMaxDimIdx(JsVec->Vec), "sparse_col_matrix * sparse_vector: dimensions mismatch");
				// computation				
				int Rows = JsMat->Rows;				
				if (JsMat->Rows == -1) {
					Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;					
				}
				TFltVV Result(Rows, 1);
				// Copy could be omitted if we implemented SparseColMat * SparseVec
				TVec<TIntFltKdV> TempSpMat(1);
				TempSpMat[0] = JsVec->Vec;				
				TLinAlg::Multiply(JsMat->Mat, TempSpMat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);			
			}

			if (TJsSpMatUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Mat.Len() >= JsMat2->Rows, "sparse_col_matrix * sparse_col_matrix: dimensions mismatch");
				// computation				
				int Rows = JsMat->Rows;				
				if (JsMat->Rows == -1) {
					Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;					
				}
				if (JsMat2->Rows == -1) {
					QmAssertR(JsMat->Mat.Len() >= TLAMisc::GetMaxDimIdx(JsMat2->Mat) + 1, "sparse_col_matrix * sparse_col_matrix: dimensions mismatch");
				}
				TFltVV Result(Rows, JsMat2->Mat.Len());					
				TLinAlg::Multiply(JsMat->Mat, JsMat2->Mat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsSpMat::multiplyT(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsMat = TJsSpMatUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (TJsSpMatUtil::IsArgFlt(Args, 0)) {
			double Scalar = TJsSpMatUtil::GetArgFlt(Args, 0);
			// create JS result and get the internal data				
			v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsMat->Js);
			TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);
			// computation
			int Rows = JsMat->Rows;
			if (Rows == -1) {
				Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;
			}
			TLinAlg::Transpose(JsMat->Mat, Result, Rows);
			TLinAlg::MultiplyScalar(Scalar, Result, Result);
			return HandleScope.Close(JsResult);			
		}
		if (Args[0]->IsObject()) {
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TFltV")) {
				TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Rows == -1 || JsMat->Rows == JsVec->Vec.Len(), "sparse_col_matrix' * vector: dimensions mismatch");
				if (JsMat->Rows == -1) {
					QmAssertR(TLAMisc::GetMaxDimIdx(JsMat->Mat) < JsVec->Vec.Len(), "sparse_col_matrix' * vector: dimensions mismatch");
				}
				// computation				
				int Cols = JsMat->Mat.Len();				
				TFltVV Result(Cols, 1);
				// Copy could be omitted if we implemented SparseColMat * TFltV
				TLinAlg::MultiplyT(JsMat->Mat, TFltVV(JsVec->Vec, JsVec->Vec.Len(), 1), Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);			
			}
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TFltVV")) {			
				TJsFltVV* JsMat2 = TJsObjUtil<TQm::TJsFltVV>::GetArgObj(Args, 0);				
				QmAssertR(JsMat->Rows == -1 || JsMat->Rows == JsMat2->Mat.GetRows(), "sparse_col_matrix' * matrix: dimensions mismatch");
				if (JsMat->Rows == -1) {
					QmAssertR(TLAMisc::GetMaxDimIdx(JsMat->Mat) < JsMat2->Mat.GetRows(), "sparse_col_matrix' * matrix: dimensions mismatch");
				}
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js);
				TFltVV& Result = TJsFltVV::GetFltVV(JsResult);
				// computation
				int Cols = JsMat->Mat.Len();
				Result.Gen(Cols, JsMat2->Mat.GetCols());
				TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
				return HandleScope.Close(JsResult);
			}
			
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
				TJsSpV* JsVec = TJsObjUtil<TQm::TJsSpV>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Rows == -1 || JsVec->Dim == -1 || JsMat->Rows == JsVec->Dim, "sparse_col_matrix' * sparse_vector: dimensions mismatch");
				// computation				
				int Cols = JsMat->Mat.Len();
				TFltVV Result(Cols, 1);
				// Copy could be omitted if we implemented SparseColMat * SparseVec
				TVec<TIntFltKdV> TempSpMat(1);
				TempSpMat[0] = JsVec->Vec;				
				TLinAlg::MultiplyT(JsMat->Mat, TempSpMat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsMat->Js, Result.Get1DVec());
				return HandleScope.Close(JsResult);
			}

			if (TJsSpMatUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Rows == -1 || JsMat2->Rows == -1 || JsMat->Rows == JsMat2->Rows, "sparse_col_matrix' * sparse_matrix: dimensions mismatch");
				// computation				
				int Cols = JsMat->Mat.Len();
				TFltVV Result(Cols, JsMat2->Mat.Len());					
				TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
				// create JS result with the Result vector	
				v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsMat->Js, Result);
				return HandleScope.Close(JsResult);			
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsSpMat::plus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsMat = TJsSpMatUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (Args[0]->IsObject()) {			
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {			
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Rows == -1 || JsMat2->Rows == -1 || JsMat->Rows == JsMat2->Rows, "matrix - matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsMat->Js);				
				TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);
				// computation				
				Result.Gen(MAX(JsMat->Mat.Len(), JsMat2->Mat.Len()));
				int Len1 = JsMat->Mat.Len();
				int Len2 = JsMat2->Mat.Len();
				int Len = Result.Len();
				for (int ColN = 0; ColN < Len; ColN++) {
					if (ColN < Len1 && ColN < Len2) {
						TLinAlg::LinComb(1.0, JsMat->Mat[ColN], 1.0, JsMat2->Mat[ColN], Result[ColN]);
					}
					if (ColN >= Len1 && ColN < Len2) {
						Result[ColN] = JsMat2->Mat[ColN];
					}
					if (ColN < Len1 && ColN >= Len2) {
						Result[ColN] = JsMat->Mat[ColN];
					}
				}
				if (JsMat->Rows == -1 && JsMat2->Rows == -1) {
					TJsSpMat::SetRows(JsResult, TLAMisc::GetMaxDimIdx(Result) + 1);
				} else {
					TJsSpMat::SetRows(JsResult, MAX(JsMat->Rows, JsMat2->Rows));
				}
				return HandleScope.Close(JsResult);
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());		
}

v8::Handle<v8::Value> TJsSpMat::minus(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsMat = TJsSpMatUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		if (Args[0]->IsObject()) {			
			if (TJsSpMatUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {			
				TJsSpMat* JsMat2 = TJsObjUtil<TQm::TJsSpMat>::GetArgObj(Args, 0);
				QmAssertR(JsMat->Rows == -1 || JsMat2->Rows == -1 || JsMat->Rows == JsMat2->Rows, "matrix - matrix: dimensions mismatch");
				// create JS result and get the internal data
				v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsMat->Js);
				TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);
				// computation				
				Result.Gen(MAX(JsMat->Mat.Len(), JsMat2->Mat.Len()));
				int Len1 = JsMat->Mat.Len();
				int Len2 = JsMat2->Mat.Len();
				int Len = Result.Len();
				for (int ColN = 0; ColN < Len; ColN++) {
					if (ColN < Len1 && ColN < Len2) {
						TLinAlg::LinComb(1.0, JsMat->Mat[ColN], -1.0, JsMat2->Mat[ColN], Result[ColN]);
					}
					if (ColN >= Len1 && ColN < Len2) {
						Result[ColN].Gen(JsMat2->Mat[ColN].Len());
						TLinAlg::MultiplyScalar(-1, JsMat2->Mat[ColN], Result[ColN]);
					}
					if (ColN < Len1 && ColN >= Len2) {
						Result[ColN] = JsMat->Mat[ColN];
					}
				}
				if (JsMat->Rows == -1 && JsMat2->Rows == -1) {
					TJsSpMat::SetRows(JsResult, TLAMisc::GetMaxDimIdx(Result) + 1);
				} else {
					TJsSpMat::SetRows(JsResult, MAX(JsMat->Rows, JsMat2->Rows));
				}
				return HandleScope.Close(JsResult);
			}
		}
	}	
	return HandleScope.Close(v8::Undefined());	
}

v8::Handle<v8::Value> TJsSpMat::transpose(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);	
	v8::Persistent<v8::Object> JsResult = TJsSpMat::New(JsSpMat->Js);
	TVec<TIntFltKdV>& Result = TJsSpMat::GetSpMat(JsResult);	
	TLinAlg::Transpose(JsSpMat->Mat, Result);
	return HandleScope.Close(JsResult);	
}

v8::Handle<v8::Value> TJsSpMat::colNorms(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);	
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsSpMat->Js);
	TFltV& Result = TJsFltV::GetVec(JsResult);	
	int Cols = JsSpMat->Mat.Len();
	Result.Gen(Cols);
	for (int ColN = 0; ColN < Cols; ColN++) {
		Result[ColN] = TLinAlg::Norm(JsSpMat->Mat[ColN]);
	}
	return HandleScope.Close(JsResult);	
}

v8::Handle<v8::Value> TJsSpMat::normalizeCols(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	TLinAlg::NormalizeColumns(JsSpMat->Mat);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpMat::full(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	TFltVV FullMat;
	int Rows = JsSpMat->Rows;
	if (Rows == -1) {
		Rows = TLAMisc::GetMaxDimIdx(JsSpMat->Mat) + 1;
	}
	TLinAlg::Full(JsSpMat->Mat, FullMat, Rows);	
	v8::Persistent<v8::Object> JsResult = TJsFltVV::New(JsSpMat->Js, FullMat);
	return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsSpMat::frob(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	double FrobNorm = 0.0;
	int Cols = JsSpMat->Mat.Len();
	for (int ColN = 0; ColN < Cols; ColN++) {
		FrobNorm += TLinAlg::Norm2(JsSpMat->Mat[ColN]);
	}
	return HandleScope.Close(v8::Number::New(TMath::Sqrt(FrobNorm)));
}

v8::Handle<v8::Value> TJsSpMat::rows(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsSpMat->Rows));
}

v8::Handle<v8::Value> TJsSpMat::cols(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Info);	
	return HandleScope.Close(v8::Number::New(JsSpMat->Mat.Len()));
}

v8::Handle<v8::Value> TJsSpMat::print(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	TLAMisc::PrintSpMat(JsSpMat->Mat, "");
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpMat::save(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	PSOut SOut = TJsFOut::GetArgFOut(Args, 0);
	// save to stream
	JsSpMat->Rows.Save(*SOut);
	JsSpMat->Mat.Save(*SOut);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsSpMat::load(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsSpMat* JsSpMat = TJsSpMatUtil::GetSelf(Args);
	PSIn SIn = TJsFIn::GetArgFIn(Args, 0);
	// load from stream
	JsSpMat->Rows.Load(*SIn);
	JsSpMat->Mat.Load(*SIn);
	return Args.Holder();
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine
v8::Handle<v8::ObjectTemplate> TJsAnalytics::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
        JsRegisterFunction(TmpTemp, newFeatureSpace);
        JsRegisterFunction(TmpTemp, loadFeatureSpace);
        JsRegisterFunction(TmpTemp, trainSvmClassify);		
        JsRegisterFunction(TmpTemp, trainSvmRegression);
		JsRegisterFunction(TmpTemp, loadSvmModel);
        JsRegisterFunction(TmpTemp, newNN);
        JsRegisterFunction(TmpTemp, newRecLinReg);
        JsRegisterFunction(TmpTemp, newHoeffdingTree);
		JsRegisterFunction(TmpTemp, loadRecLinRegModel);
		JsRegisterFunction(TmpTemp, trainKMeans);						
        JsRegisterFunction(TmpTemp, newTokenizer);
        JsRegisterFunction(TmpTemp, getLanguageOptions);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsAnalytics::newFeatureSpace(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
	QmAssertR(Args.Length() > 0, "analytics.newFeatureSpace : input not specified!");
	try {
		TFtrExtV FtrExtV;		
		if (Args[0]->IsArray()) {
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
			for (uint32 ObjectN = 0; ObjectN < Array->Length(); ObjectN++) {
				if (Array->Get(ObjectN)->IsObject()) {
					v8::Local<v8::Object> Obj = Array->Get(ObjectN)->ToObject();
					// get property "type"
					if (Obj->Has(v8::String::New("type"))) {
						v8::Handle<v8::Value> TypeVal = Obj->Get(v8::String::New("type"));
						if (TypeVal->IsString()) {
							v8::String::Utf8Value Utf8(TypeVal);
							TStr Type(*Utf8);
							if (Type == "jsfunc") {
								QmAssertR(Obj->Has(v8::String::New("fun")), "analytics.newFeatureSpace object of type 'jsfunc' should have a property 'fun'");
								QmAssertR(Obj->Get(v8::String::New("fun"))->IsFunction(), "analytics.newFeatureSpace object.fun should be a function");
								v8::Persistent<v8::Function> Fun = v8::Persistent<v8::Function>::New(v8::Handle<v8::Function>::Cast(Obj->Get(v8::String::New("fun"))));
								PJsonVal ParamVal = TJsFuncFtrExt::CopySettings(Obj);
								PFtrExt FtrExt = TJsFuncFtrExt::NewFtrExt(JsAnalytics->Js, ParamVal, Fun);
								FtrExtV.Add(FtrExt);
							}
							else {
								// Json val to glib JSON
								PJsonVal ParamVal = TJsonVal::GetValFromStr(TJsUtil::V8JsonToStr(Array->Get(ObjectN)));
								if (ParamVal->IsObj()) {
									FtrExtV.Add(TFtrExt::New(JsAnalytics->Js->Base, ParamVal->GetObjStr("type"), ParamVal));
								}
							}
						}
					}
				}
			}
		} else if (Args[0]->IsObject()) {
			// get type
			TStr Type = TJsAnalyticsUtil::GetArgStr(Args, 0, "type", "");
			if (Type == "jsfunc") {
				// All properties should be JSON objects, except for "fun", which is a function
				// example (Twitter text length feature extractor):
				// { type : 'jsfunc', source: { store: 'Tweets' }, fun : function(rec) {return rec.Text.length;}}
				// extract function!
				v8::Persistent<v8::Function> Fun = TJsAnalyticsUtil::GetArgFunPer(Args, 0, "fun");
				PJsonVal ParamVal = TJsFuncFtrExt::CopySettings(Args[0]->ToObject());
				PFtrExt FtrExt = TJsFuncFtrExt::NewFtrExt(JsAnalytics->Js, ParamVal, Fun);
				FtrExtV.Add(FtrExt);
			}
			else {				
				// JSON object expected
				// example (bag of words extractor)
				// { type: 'numeric', source: { store: 'Movies' }, field: 'Rating', normalize: true }
				PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 0);
				if (ParamVal->IsObj()) {
					FtrExtV.Add(TFtrExt::New(JsAnalytics->Js->Base, ParamVal->GetObjStr("type"), ParamVal));
				}
			}
		}
		
		// create feature space
		PFtrSpace FtrSpace = TFtrSpace::New(JsAnalytics->Js->Base, FtrExtV);
		// done
		return TJsFtrSpace::New(JsAnalytics->Js, FtrSpace);		
	}
	catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
	}
	
	return v8::Undefined();
}


v8::Handle<v8::Value> TJsAnalytics::loadFeatureSpace(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    // parse parameters
    PSIn SIn = TJsFIn::GetArgFIn(Args, 0);
    // load feature space from stream
    PFtrSpace FtrSpace = TFtrSpace::Load(JsAnalytics->Js->Base, *SIn);
    // done
    return TJsFtrSpace::New(JsAnalytics->Js, FtrSpace);
}

v8::Handle<v8::Value> TJsAnalytics::trainSvmClassify(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    QmAssertR(Args.Length() >= 2, "trainSvmClassify: missing arguments!");
    // get class information
    QmAssertR(Args[1]->IsObject(), "trainSvmClassify: second argument expected to be object");
    TFltV& ClsV = TJsFltV::GetVec(Args[1]->ToObject());
    // parse SVM parameters
    PJsonVal SvmParamVal = TJsonVal::NewObj();
    if (Args.Length() > 2 && TJsAnalyticsUtil::IsArgJson(Args, 2)) {
        SvmParamVal = TJsAnalyticsUtil::GetArgJson(Args, 2); }
    const double SvmCost = SvmParamVal->GetObjNum("c", 1.0);
    const double SvmUnbalance = SvmParamVal->GetObjNum("j", 1.0);
    const double SampleSize = SvmParamVal->GetObjNum("batchSize", 10000);
    const int MxIter = SvmParamVal->GetObjInt("maxIterations", 10000);
	const int MxTime = (int)(1000 * SvmParamVal->GetObjNum("maxTime", 600));
    const double MnDiff = SvmParamVal->GetObjNum("minDiff", 1e-6);
	PNotify Notify = TEnv::Logger;
	const bool Verbose = SvmParamVal->GetObjBool("verbose", false);
	if (!Verbose) {
		Notify = TNotify::NullNotify;
	}
    // check what kind of input data we got and train and return the model
    try {
        if (TJsAnalyticsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
            // we have sparse matrix on the input
            QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
            TVec<TIntFltKdV>& VecV = TJsSpMat::GetSpMat(Args[0]->ToObject());
            return TJsSvmModel::New(JsAnalytics->Js, 
                TSvm::SolveClassify<TVec<TIntFltKdV>>(VecV, 
                    TLAMisc::GetMaxDimIdx(VecV) + 1, VecV.Len(), ClsV, SvmCost, 
                    SvmUnbalance, MxTime, MxIter, MnDiff, SampleSize, 
                    Notify));
        } else if (TJsAnalyticsUtil::IsArgClass(Args, 0, "TFltVV")) {
            // we have dense matrix on the input
            QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
            TFltVV& VecV = TJsFltVV::GetFltVV(Args[0]->ToObject());
            return TJsSvmModel::New(JsAnalytics->Js, 
                TSvm::SolveClassify<TFltVV>(VecV, VecV.GetRows(),
                    VecV.GetCols(), ClsV, SvmCost, SvmUnbalance, MxTime, 
					MxIter, MnDiff, SampleSize, Notify));
        } else {
            // TODO support JavaScript array of TJsFltV or TJsSpV
            throw TQmExcept::New("unsupported type of the first argument");
        }
    } catch (const PExcept& Except) {
        InfoLog("[except] trainSvmClassify: " + Except->GetMsgStr());
    }    
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsAnalytics::trainSvmRegression(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    QmAssertR(Args.Length() >= 2, "trainSvmRegression: missing arguments!");
    // get target value information
    QmAssertR(Args[1]->IsObject(), "trainSvmRegression: second argument expected to be object");
    TFltV& ValV = TJsFltV::GetVec(Args[1]->ToObject());
    // parse SVM parameters
    PJsonVal SvmParamVal = TJsonVal::NewObj();
    if (Args.Length() > 2 && TJsAnalyticsUtil::IsArgJson(Args, 2)) {
        SvmParamVal = TJsAnalyticsUtil::GetArgJson(Args, 2); }
    const double SvmCost = SvmParamVal->GetObjNum("c", 1.0);
    const double SvmEps = SvmParamVal->GetObjNum("eps", 1.0);
    const double SampleSize = SvmParamVal->GetObjNum("batchSize", 10000);
    const int MxIter = SvmParamVal->GetObjInt("maxIterations", 10000);
	const int MxTime = (int)(1000 * SvmParamVal->GetObjNum("maxTime", 600));
    const double MnDiff = SvmParamVal->GetObjNum("minDiff", 1e-6);
	PNotify Notify = TEnv::Logger;
	const bool Verbose = SvmParamVal->GetObjBool("verbose", false);
	if (!Verbose) {
		Notify = TNotify::NullNotify;
	}
    // check what kind of input data we got
    try {
        if (TJsAnalyticsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
            // we have sparse matrix on the input
            QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
            TVec<TIntFltKdV>& VecV = TJsSpMat::GetSpMat(Args[0]->ToObject());
            return TJsSvmModel::New(JsAnalytics->Js, 
                TSvm::SolveRegression<TVec<TIntFltKdV>>(VecV, 
                    TLAMisc::GetMaxDimIdx(VecV) + 1, VecV.Len(), ValV, SvmCost, 
                    SvmEps, MxTime, MxIter, MnDiff, SampleSize, 
					Notify));
        } else if (TJsAnalyticsUtil::IsArgClass(Args, 0, "TFltVV")) {
            // we have dense matrix on the input
            QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
            TFltVV& VecV = TJsFltVV::GetFltVV(Args[0]->ToObject());
            return TJsSvmModel::New(JsAnalytics->Js, 
                TSvm::SolveRegression<TFltVV>(VecV, VecV.GetRows(),
                    VecV.GetCols(), ValV, SvmCost, SvmEps, MxTime, 
					MxIter, MnDiff, SampleSize, Notify));
        } else {
            // TODO support JavaScript array of TJsFltV or TJsSpV
            throw TQmExcept::New("unsupported type of the first argument");
        }
    } catch (const PExcept& Except) {
        InfoLog("[except] trainSvmRegression: " + Except->GetMsgStr());
    }   
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsAnalytics::loadSvmModel(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);	
	if (Args.Length() > 0) {
        PSIn SIn = TJsFIn::GetArgFIn(Args, 0);
		return HandleScope.Close(TJsSvmModel::New(JsAnalytics->Js, TSvm::TLinModel(*SIn)));
	}
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsAnalytics::newNN(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 0);
   
    TIntV LayoutV; ParamVal->GetObjIntV("layout", LayoutV);
    double LearnRate = ParamVal->GetObjNum("learnRate", 0.1);
    double Momentum = ParamVal->GetObjNum("momentum", 0.5);
    TStr TFuncHiddenLStr = ParamVal->GetObjStr("tFuncHidden", "tanHyper");
    TStr TFuncOutLStr = ParamVal->GetObjStr("tFuncOut", "tanHyper");
    TSignalProc::TTFunc TFuncHiddenL;
    TSignalProc::TTFunc TFuncOutL;
    
    if (TFuncHiddenLStr == "tanHyper") { 
        TFuncHiddenL = TSignalProc::TTFunc::tanHyper;
    } else if (TFuncHiddenLStr == "sigmoid") {
        TFuncHiddenL = TSignalProc::TTFunc::sigmoid;
    } else if (TFuncHiddenLStr == "fastTanh") {
        TFuncHiddenL = TSignalProc::TTFunc::fastTanh;
    } else if (TFuncHiddenLStr == "fastSigmoid") {
        TFuncHiddenL = TSignalProc::TTFunc::fastSigmoid;
    } else if (TFuncHiddenLStr == "linear") {
        TFuncHiddenL = TSignalProc::TTFunc::linear;
    } else {
        throw TExcept::New("Unknown transfer function type " + TFuncHiddenLStr);
    }

    if (TFuncOutLStr == "tanHyper") { 
        TFuncOutL = TSignalProc::TTFunc::tanHyper;
    } else if (TFuncOutLStr == "sigmoid") {
        TFuncOutL = TSignalProc::TTFunc::sigmoid;
    } else if (TFuncOutLStr == "fastTanh") {
        TFuncOutL = TSignalProc::TTFunc::fastTanh;
    } else if (TFuncOutLStr == "fastSigmoid") {
        TFuncOutL = TSignalProc::TTFunc::fastSigmoid;
    } else if (TFuncOutLStr == "linear") {
        TFuncOutL = TSignalProc::TTFunc::linear;
    } else {
        throw TExcept::New("Unknown transfer function type " + TFuncOutLStr);
    }

    try {
        TSignalProc::PNNet NN = TSignalProc::TNNet::New(LayoutV, LearnRate, Momentum, TFuncHiddenL, TFuncOutL);
        return TJsNN::New(JsAnalytics->Js, NN);
    }
    catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
    }
    return v8::Undefined();
}


v8::Handle<v8::Value> TJsAnalytics::newRecLinReg(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 0);
   
    int dim = ParamVal->GetObjInt("dim");
	double ForgetFact = ParamVal->GetObjNum("forgetFact", 1.0);
	double RegFact = ParamVal->GetObjNum("regFact", 1.0);

    try {
        TSignalProc::PRecLinReg linReg = TSignalProc::TRecLinReg::New(dim, RegFact, ForgetFact);
        return TJsRecLinRegModel::New(JsAnalytics->Js, linReg);
    }
    catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
    }
    return v8::Undefined();
}

v8::Handle<v8::Value> TJsAnalytics::loadRecLinRegModel(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
	if (Args.Length() > 0) {
        PSIn SIn = TJsFIn::GetArgFIn(Args, 0);
        TSignalProc::PRecLinReg Model = TSignalProc::TRecLinReg::Load(*SIn);
		return HandleScope.Close(TJsRecLinRegModel::New(JsAnalytics->Js, Model));
	}
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsAnalytics::newHoeffdingTree(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);
    PJsonVal StreamConfig = TJsAnalyticsUtil::GetArgJson(Args, 0);
	PJsonVal JsonConfig = TJsAnalyticsUtil::GetArgJson(Args, 1);
	try {
        return TJsHoeffdingTree::New(JsAnalytics->Js, StreamConfig, JsonConfig);
    }
    catch (const PExcept& Except) {
		InfoLog("[except] " + Except->GetMsgStr());
    }
    return v8::Undefined();
}

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
        // normalize as KMeans assumes normalized inputs
        for (int SpVN = 0; SpVN < SpVV.Len(); SpVN++) {
            TLinAlg::Normalize(SpVV[SpVN]); }
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

v8::Handle<v8::Value> TJsAnalytics::newTokenizer(const v8::Arguments& Args) {
    v8::HandleScope HandleScope;
    // parameters
	TJsAnalytics* JsAnalytics = TJsAnalyticsUtil::GetSelf(Args);    
    PJsonVal ParamVal = TJsAnalyticsUtil::GetArgJson(Args, 0);
    QmAssertR(ParamVal->IsObjKey("type"), 
        "Missing tokenizer type " + ParamVal->SaveStr());
    const TStr& TypeNm = ParamVal->GetObjStr("type");
    // create
    PTokenizer Tokenizer = TTokenizer::New(TypeNm, ParamVal);
    // return
    return TJsTokenizer::New(JsAnalytics->Js, Tokenizer);
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

///////////////////////////////
// QMiner-JavaScript-Feature-Space

v8::Handle<v8::ObjectTemplate> TJsFtrSpace::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
        JsRegisterProperty(TmpTemp, dim);
		JsRegisterFunction(TmpTemp, save);
		JsRegisterFunction(TmpTemp, updateRecord);
		JsRegisterFunction(TmpTemp, updateRecords);
		JsRegisterFunction(TmpTemp, finishUpdate);					
		JsRegisterFunction(TmpTemp, extractStrings);	
		JsRegisterFunction(TmpTemp, getFtr);
		JsRegisterFunction(TmpTemp, ftrSpVec);						
		JsRegisterFunction(TmpTemp, ftrVec);	
		JsRegisterFunction(TmpTemp, ftrSpColMat);						
		JsRegisterFunction(TmpTemp, ftrColMat);	
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

PFtrSpace TJsFtrSpace::GetArgFtrSpace(v8::Handle<v8::Value> Val) {
	v8::HandleScope HandleScope;	
	// check it's of the right type
	AssertR(Val->IsObject(), "GetArgFtrSpace: Argument expected to be Object");
	// get the wrapped 
	v8::Handle<v8::Object> FtrSpace = v8::Handle<v8::Object>::Cast(Val);
	v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(FtrSpace->GetInternalField(0));
	// cast it to record set
	TJsFtrSpace* JsFtrSpace = static_cast<TJsFtrSpace*>(WrappedObject->Value());
	return JsFtrSpace->FtrSpace;
}

v8::Handle<v8::Value> TJsFtrSpace::dim(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Info);
	return HandleScope.Close(v8::Integer::New(JsFtrSpace->FtrSpace->GetDim()));
}

v8::Handle<v8::Value> TJsFtrSpace::save(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PSOut SOut = TJsFOut::GetArgFOut(Args, 0);
    // save to stream
    JsFtrSpace->FtrSpace->Save(*SOut);
	// return
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFtrSpace::updateRecord(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    TJsRec* JsRec = TJsObjUtil<TJsRec>::GetArgObj(Args, 0);
    // update with new records
    JsFtrSpace->FtrSpace->Update(JsRec->Rec);
	// return
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFtrSpace::updateRecords(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
    // update with new records
    JsFtrSpace->FtrSpace->Update(RecSet);
	// return
	return Args.Holder();
}

v8::Handle<v8::Value> TJsFtrSpace::finishUpdate(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    InfoLog("Warning: featureSpace.finishUpdate() is no longer necessary and hence deprecated.");
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
    // get strings
    TStrV StrV; JsFtrSpace->FtrSpace->ExtractStrV(DimN, RecVal, StrV);
	// return as JS array
	v8::Handle<v8::Array> StrArr = v8::Array::New(StrV.Len());
    for(int StrN = 0; StrN < StrV.Len(); StrN++) {
		StrArr->Set(v8::Uint32::New(StrN), v8::String::New(StrV.GetVal(StrN).CStr()));
	}
	return HandleScope.Close(StrArr);
}

// ftrStr = getFtr(FtrN)
v8::Handle<v8::Value> TJsFtrSpace::getFtr(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);
	uint32 FtrN = TJsFtrSpaceUtil::GetArgInt32(Args, 0, 0);
	TStr FtrNm = JsFtrSpace->FtrSpace->GetFtr(FtrN);
	// return as JS array
	return HandleScope.Close(v8::String::New(FtrNm.CStr()));
}


v8::Handle<v8::Value> TJsFtrSpace::ftrSpVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    TRec Rec = TJsRec::GetArgRec(Args, 0);
    // create feature vector
	TIntFltKdV SpV;
	JsFtrSpace->FtrSpace->GetSpV(Rec, SpV);
	v8::Persistent<v8::Object> JsSpV = TJsSpV::New(JsFtrSpace->Js, SpV);
	// return
	return HandleScope.Close(JsSpV);
}

v8::Handle<v8::Value> TJsFtrSpace::ftrVec(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);
	TRec Rec = TJsRec::GetArgRec(Args, 0);
	// create feature vector, compute
	TFltV FltV;
	JsFtrSpace->FtrSpace->GetFullV(Rec, FltV);
	// Create result and append vector
	v8::Persistent<v8::Object> JsFltV = TJsFltV::New(JsFtrSpace->Js, FltV);
	// return
	return HandleScope.Close(JsFltV);
}

v8::Handle<v8::Value> TJsFtrSpace::ftrSpColMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
    // create feature matrix
	TVec<TIntFltKdV> SpMat;
	JsFtrSpace->FtrSpace->GetSpVV(RecSet, SpMat);
	// create result
	v8::Persistent<v8::Object> JsSpMat = TJsSpMat::New(JsFtrSpace->Js, SpMat);
	// return
	return HandleScope.Close(JsSpMat);
}

v8::Handle<v8::Value> TJsFtrSpace::ftrColMat(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsFtrSpace* JsFtrSpace = TJsFtrSpaceUtil::GetSelf(Args);	
    PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
    // create feature matrix
	TFltVV Mat;
	JsFtrSpace->FtrSpace->GetFullVV(RecSet, Mat);
	// create result
	v8::Persistent<v8::Object> JsMat = TJsFltVV::New(JsFtrSpace->Js, Mat);
	// return
	return HandleScope.Close(JsMat);
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Machine-Model

v8::Handle<v8::ObjectTemplate> TJsSvmModel::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, predict);
		JsLongRegisterFunction(TmpTemp, "classify", predict);
		JsRegisterProperty(TmpTemp, weights);
		JsRegisterFunction(TmpTemp, save);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsSvmModel::predict(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsSvmModel* JsSvmModel = TJsSvmModelUtil::GetSelf(Args);	
    // get vector vector
    QmAssertR(Args.Length() > 0, "svm.predict: missing argument");
    if (TJsSvmModelUtil::IsArgClass(Args, 0, "TFltV")) {
        const double Res = JsSvmModel->Model.Predict(TJsFltV::GetVec(Args[0]->ToObject()));
    	return HandleScope.Close(v8::Number::New(Res));
    } else if (TJsSvmModelUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
        const double Res = JsSvmModel->Model.Predict(TJsSpV::GetSpV(Args[0]->ToObject()));
    	return HandleScope.Close(v8::Number::New(Res));
    }
    throw TQmExcept::New("svm.predict: unsupported type of the first argument");
}

v8::Handle<v8::Value> TJsSvmModel::weights(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
    // parse arguments
	TJsSvmModel* JsSvmModel = TJsSvmModelUtil::GetSelf(Info);	
    // get feature vector
	TFltV WgtV; JsSvmModel->Model.GetWgtV(WgtV);
	v8::Persistent<v8::Object> JsWgtV = TJsFltV::New(JsSvmModel->Js, WgtV);
    return HandleScope.Close(JsWgtV);
}

v8::Handle<v8::Value> TJsSvmModel::save(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// parse arguments
	TJsSvmModel* JsSvmModel = TJsSvmModelUtil::GetSelf(Args);
	if (Args.Length() > 0) {
		PSOut SOut = TJsFOut::GetArgFOut(Args, 0);
		JsSvmModel->Model.Save(*SOut);
	}
	return Args.Holder();
}

///////////////////////////////
// QMiner-JavaScript-Neural-Networks
v8::Handle<v8::ObjectTemplate> TJsNN::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;    
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, learn);
		JsRegisterFunction(TmpTemp, predict);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}
// TODO: learn without feedforward, if we know the values in out layer are correct
v8::Handle<v8::Value> TJsNN::learn(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsNN* JsNN = TJsNNUtil::GetSelf(Args);
    // get feature vector
    QmAssertR(TJsNNUtil::IsArgClass(Args, 0, "TFltV"), 
        "NeuralNetwork.learn: The first argument must be a JsTFltV (js linalg full vector)"); 
    QmAssertR(TJsNNUtil::IsArgClass(Args, 1, "TFltV"), 
        "NeuralNetwork.learn: The second argument must be a JsTFltV (js linalg full vector)"); 
    TJsFltV* JsVecIn = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
    TJsFltV* JsVecTarget = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 1);
    // TODO: do some checking of dimensions etc..
    // first get output values
    JsNN->NN->FeedFwd(JsVecIn->Vec);
    // then check how we performed and learn
    JsNN->NN->BackProp(JsVecTarget->Vec);
    
	return Args.Holder();
}

v8::Handle<v8::Value> TJsNN::predict(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsNN* JsNN = TJsNNUtil::GetSelf(Args);
    // get feature vector
    QmAssertR(TJsNNUtil::IsArgClass(Args, 0, "TFltV"), 
        "RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)"); 
    TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
    JsNN->NN->FeedFwd(JsVec->Vec);
    v8::Persistent<v8::Object> JsFltV = TJsFltV::New(JsNN->Js);
    TFltV& FltV = TJsFltV::GetVec(JsFltV);
    JsNN->NN->GetResults(FltV);

    return HandleScope.Close(JsFltV);
}

///////////////////////////////
// QMiner-JavaScript-Recursive-Linear-Regression

v8::Handle<v8::ObjectTemplate> TJsRecLinRegModel::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;    
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, learn);
		JsRegisterFunction(TmpTemp, predict);
		JsRegisterFunction(TmpTemp, save);
		JsRegisterProperty(TmpTemp, weights);
		JsRegisterProperty(TmpTemp, dim);
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsRecLinRegModel::learn(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsRecLinRegModel* JsRecLinRegModel = TJsRecLinRegModelUtil::GetSelf(Args);
    // get feature vector
    QmAssertR(TJsRecLinRegModelUtil::IsArgClass(Args, 0, "TFltV"), 
        "RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)"); 
    TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
    // make sure dimensions match
    QmAssertR(JsRecLinRegModel->Model->GetDim() == JsVec->Vec.Len(), 
        "RecLinRegModel.learn: model dimension != passed argument dimension");				
    const double Target = TJsRecLinRegModelUtil::GetArgFlt(Args, 1);
    // learn
    JsRecLinRegModel->Model->Learn(JsVec->Vec, Target);
	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecLinRegModel::predict(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsRecLinRegModel* JsRecLinRegModel = TJsRecLinRegModelUtil::GetSelf(Args);
    // get feature vector
    QmAssertR(TJsRecLinRegModelUtil::IsArgClass(Args, 0, "TFltV"), 
        "RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)"); 
    TJsFltV* JsVec = TJsObjUtil<TQm::TJsFltV>::GetArgObj(Args, 0);
    QmAssertR(JsRecLinRegModel->Model->GetDim() == JsVec->Vec.Len(), 
        "RecLinRegModel.learn: model dimension != sample dimension");				
    const double Predict = JsRecLinRegModel->Model->Predict(JsVec->Vec);
    return HandleScope.Close(v8::Number::New(Predict));
}

v8::Handle<v8::Value> TJsRecLinRegModel::weights(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
    // parse arguments
    TJsRecLinRegModel* JsRecLinRegModel = TJsRecLinRegModelUtil::GetSelf(Info);
    // get feature vector
	TFltV Coef;
	JsRecLinRegModel->Model->GetCoeffs(Coef);	    
	v8::Persistent<v8::Object> JsResult = TJsFltV::New(JsRecLinRegModel->Js, Coef);
    return HandleScope.Close(JsResult);
}

v8::Handle<v8::Value> TJsRecLinRegModel::save(const v8::Arguments& Args) {
	QmAssertR(Args.Length() > 0, "TJsRecLinRegModel::save: SOut not specified!");

	v8::HandleScope HandleScope;
	// parse arguments
	TJsRecLinRegModel* JsModel = TJsRecLinRegModelUtil::GetSelf(Args);
	PSOut SOut = TJsFOut::GetArgFOut(Args, 0);
	JsModel->Model->Save(*SOut);

	return Args.Holder();
}

v8::Handle<v8::Value> TJsRecLinRegModel::dim(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	// parse arguments
    TJsRecLinRegModel* JsRecLinRegModel = TJsRecLinRegModelUtil::GetSelf(Info);
	return HandleScope.Close(v8::Integer::New(JsRecLinRegModel->Model->GetDim()));
}

///////////////////////////////
// QMiner-JavaScript-HoeffdingTree
v8::Handle<v8::ObjectTemplate> TJsHoeffdingTree::GetTemplate() {
	v8::HandleScope HandleScope;
	v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
	JsRegisterFunction(TmpTemp, process);
	JsRegisterFunction(TmpTemp, classify);
	JsRegisterFunction(TmpTemp, exportModel);
	TmpTemp->SetInternalFieldCount(1);
	return v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
}

// NOTE: It is your responsibility to make sure the input line is a valid stream example 
v8::Handle<v8::Value> TJsHoeffdingTree::process(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsHoeffdingTree* JsHoeffdingTree = TJsHoeffdingTreeUtil::GetSelf(Args);
	if(Args.Length() == 1 && Args[0]->IsString()) {
		TStr Line = TJsHoeffdingTreeUtil::GetArgStr(Args, 0);
		// printf("Line '%s'\n", Line.CStr());
		JsHoeffdingTree->HoeffdingTree->Process(Line);
		// printf("End\n");
	} else if(Args.Length() >= 3 && Args[0]->IsObject() && Args[1]->IsObject() && Args[2]->IsString()) {
		PJsonVal DiscreteVal = TJsHoeffdingTreeUtil::GetArgJson(Args, 0);
		PJsonVal NumericVal = TJsHoeffdingTreeUtil::GetArgJson(Args, 1);
		TStr Label = TJsHoeffdingTreeUtil::GetArgStr(Args, 2);
		TStrV DisV; TFltV NumV;
		if(DiscreteVal->IsArr() && NumericVal->IsArr()) {
			DiscreteVal->GetArrStrV(DisV);
			NumericVal->GetArrNumV(NumV);
			JsHoeffdingTree->HoeffdingTree->Process(DisV, NumV, Label);
		}
	}
	return HandleScope.Close(v8::Undefined());
}

// NOTE: It is your responsibility to make sure the input line is a valid stream example 
v8::Handle<v8::Value> TJsHoeffdingTree::classify(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsHoeffdingTree* JsHoeffdingTree = TJsHoeffdingTreeUtil::GetSelf(Args);
	if(Args.Length() == 1 && Args[0]->IsString()) {
		TStr Line = TJsHoeffdingTreeUtil::GetArgStr(Args, 0);
		TStr Label = JsHoeffdingTree->HoeffdingTree->Classify(Line);
		return HandleScope.Close(v8::String::New(Label.CStr()));
	} else if(Args.Length() >= 2 && Args[0]->IsObject() && Args[1]->IsObject()) {
		PJsonVal DiscreteVal = TJsHoeffdingTreeUtil::GetArgJson(Args, 0);
		PJsonVal NumericVal = TJsHoeffdingTreeUtil::GetArgJson(Args, 1);
		TStrV DisV; TFltV NumV;
		if(DiscreteVal->IsArr() && NumericVal->IsArr()) {
			DiscreteVal->GetArrStrV(DisV);
			NumericVal->GetArrNumV(NumV);
			// THoeffding::TLabel Label = JsHoeffdingTree->HoeffdingTree->Classify(DisV, NumV);
			TStr Label = JsHoeffdingTree->HoeffdingTree->Classify(DisV, NumV);
			// return HandleScope.Close(v8::Number::New(Label));
			return HandleScope.Close(v8::String::New(Label.CStr()));
		} // else { EFailR("No such function"); }
	} // else { EFailR("Unsupported."); }
	return HandleScope.Close(v8::Undefined());
}

v8::Handle<v8::Value> TJsHoeffdingTree::exportModel(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// Save model to fileName in the choosen format 
	TJsHoeffdingTree* JsHoeffdingTree = TJsHoeffdingTreeUtil::GetSelf(Args);
	if(Args.Length() == 1 && Args[0]->IsObject()) {
		PJsonVal Val = TJsHoeffdingTreeUtil::GetArgJson(Args, 0);
		if(Val->IsObjKey("file") && Val->IsObjKey("type")) {
			TStr FNm = Val->GetObjStr("file");
			TStr Type = Val->GetObjStr("type");
			THoeffding::TExportType ExportType;
			// When supported, accept JSON 
			if(Type == "DOT") { ExportType = THoeffding::etDOT; }
			else if(Type == "JSON") { ExportType = THoeffding::etJSON; }
			else { ExportType = THoeffding::etXML; }
			JsHoeffdingTree->HoeffdingTree->Export(FNm, ExportType);
			return HandleScope.Close(v8::Boolean::New(true));
		}
	}
	return HandleScope.Close(v8::Undefined());
}

///////////////////////////////
// QMiner-JavaScript-Tokenizer
v8::Handle<v8::ObjectTemplate> TJsTokenizer::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, getTokens);	
		JsRegisterFunction(TmpTemp, getSentences);	
		JsRegisterFunction(TmpTemp, getParagraphs);	
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsTokenizer::getTokens(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parameters
	TJsTokenizer* JsTokenizer = TJsTokenizerUtil::GetSelf(Args);
    TStr TextStr = TJsTokenizerUtil::GetArgStr(Args, 0);
    // tokenize
    TStrV TokenV; JsTokenizer->Tokenizer->GetTokens(TextStr, TokenV);
	// return
	return HandleScope.Close(TJsUtil::GetStrArr(TokenV));
}

v8::Handle<v8::Value> TJsTokenizer::getSentences(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parameters
    TStr TextStr = TJsTokenizerUtil::GetArgStr(Args, 0);
    // split into sentences
    TStrV SentenceV; TTokenizerUtil::Sentencize(TextStr, SentenceV);
	// return
	return HandleScope.Close(TJsUtil::GetStrArr(SentenceV));
}

v8::Handle<v8::Value> TJsTokenizer::getParagraphs(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    // parameters
    TStr TextStr = TJsTokenizerUtil::GetArgStr(Args, 0);
    // split into sentences
    TStrV ParagraphV; TTokenizerUtil::Sentencize(TextStr, ParagraphV);
	// return
	return HandleScope.Close(TJsUtil::GetStrArr(ParagraphV));
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


v8::Handle<v8::ObjectTemplate> TJsGeoIp::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
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
// QMiner-JavaScript-DMoz
bool TJsDMoz::InitP = false;
PDMozCfy TJsDMoz::DMozCfy = NULL;

const PDMozCfy& TJsDMoz::GetDMozCfy() {
	if (!InitP) {
		InitP = true;
		//TODO: check for folder
		DMozCfy = TDMozCfy::LoadFPath("./dbs/");
	}
	return DMozCfy;
}


v8::Handle<v8::ObjectTemplate> TJsDMoz::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, classify);				
		
		TmpTemp->SetAccessCheckCallbacks(TJsUtil::NamedAccessCheck, TJsUtil::IndexedAccessCheck);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsDMoz::classify(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	// read IP address
	TStr Text = TJsDMozUtil::GetArgStr(Args, 0);
    const int MxCats = TJsDMozUtil::GetArgInt32(Args, 1, 10);
	// classify
	PDMozCfy DMozCfy = GetDMozCfy();
    TStrFltKdV CatNmWgtV, KeyWdWgtV;
    DMozCfy->Classify(Text, CatNmWgtV, KeyWdWgtV, MxCats);
    // prepare response
    PJsonVal KeyWdV = TJsonVal::NewArr();
    for (int KeyWdN = 0; KeyWdN < KeyWdWgtV.Len(); KeyWdN++) {
        KeyWdV->AddToArr(TJsonVal::NewStr(KeyWdWgtV[KeyWdN].Key));
    }
	// return
	return HandleScope.Close(TJsUtil::ParseJson(KeyWdV));
}

///////////////////////////////
// QMiner-JavaScript-Process
v8::Handle<v8::ObjectTemplate> TJsProcess::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, stop);
		JsRegisterFunction(TmpTemp, sleep);
		JsRegisterProperty(TmpTemp, args);
		JsRegisterProperty(TmpTemp, sysStat);
		JsRegisterProperty(TmpTemp, scriptNm);
		JsRegisterProperty(TmpTemp, scriptFNm);
		JsRegisterFunction(TmpTemp, getGlobals);
		JsRegisterFunction(TmpTemp, exitScript);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsProcess::stop(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
    if (TJsProcessUtil::IsArg(Args, 0)) {
        // we have return code, get it
        TEnv::ReturnCode = TJsProcessUtil::GetArgInt32(Args, 0);
    }
    // send message to server to quit
    TLoop::Stop();
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsProcess::sleep(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	const TInt Millis = TJsProcessUtil::GetArgInt32(Args,0);
	QmAssertR(Millis >= 0, "Sleep time must be greater or equal to than 0!");
	TSysProc::Sleep(TUInt(Millis));
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsProcess::args(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	PJsonVal ArgsVal = TJsonVal::NewArr(Env.GetArgV());
	return HandleScope.Close(TJsUtil::ParseJson(ArgsVal));
}

v8::Handle<v8::Value> TJsProcess::sysStat(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
    PJsonVal StatVal = TJsonVal::NewObj();
#ifdef GLib_LINUX   
    TSysMemStat MemStat;
    StatVal->AddToObj("size", TUInt64::GetStr(MemStat.Size));
    StatVal->AddToObj("sizeKb", TUInt64::GetKiloStr(MemStat.Size));
    StatVal->AddToObj("sizeMb", TUInt64::GetMegaStr(MemStat.Size));
#endif
	return HandleScope.Close(TJsUtil::ParseJson(StatVal));
}
	
v8::Handle<v8::Value> TJsProcess::scriptNm(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsProcess* JsProc = TJsProcessUtil::GetSelf(Info);
	v8::Local<v8::String> ScriptNm = v8::String::New(JsProc->Js->GetScriptNm().CStr());
	return HandleScope.Close(ScriptNm);
}

v8::Handle<v8::Value> TJsProcess::scriptFNm(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsProcess* JsProc = TJsProcessUtil::GetSelf(Info);
	v8::Local<v8::String> ScriptFNm = v8::String::New(JsProc->Js->GetScriptFNm().CStr());
	return HandleScope.Close(ScriptFNm);
}

v8::Handle<v8::Value> TJsProcess::getGlobals(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsProcess* JsProc = TJsProcessUtil::GetSelf(Args);
	v8::Local<v8::Object> Obj = JsProc->Js->Context->Global();
	v8::Local<v8::Array> Properties = Obj->GetPropertyNames();	
	//for (uint32 PropN = 0; PropN < Properties->Length(); PropN++) {
	//	// get each property as string, extract arg json and attach it to ParamVal
	//	TStr PropStr = TJsUtil::V8JsonToStr(Properties->Get(PropN));
	//	PropStr = PropStr.GetSubStr(1, PropStr.Len() - 2); // remove " char at the beginning and end
	//	printf("%s", PropStr.CStr());
	//	if (PropN < Properties->Length() - 1) {
	//		printf(", ");
	//	}
	//	else { printf("\n"); }
	//}
	return HandleScope.Close(Properties);
}


v8::Handle<v8::Value> TJsProcess::exitScript(const v8::Arguments& Args) {
	v8::V8::TerminateExecution(v8::Isolate::GetCurrent());
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Console
v8::Handle<v8::ObjectTemplate> TJsConsole::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, log);
		JsRegisterFunction(TmpTemp, getln);
		JsRegisterFunction(TmpTemp, print);
		JsLongRegisterFunction(TmpTemp, "say", log);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

v8::Handle<v8::Value> TJsConsole::log(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	if (Args.Length() == 1) { 
		TStr MsgStr = TJsConsoleUtil::GetStr(Args[0]->ToString());
		InfoLog("[console] " + MsgStr);
	} else if (Args.Length() > 1) {
		const TStr TitleStr = TJsConsoleUtil::GetStr(Args[0]->ToString());
		const TStr MsgStr = TJsConsoleUtil::GetStr(Args[1]->ToString());
		TEnv::Logger->OnStatusFmt("[%s] %s", TitleStr.CStr(), MsgStr.CStr());
	} else {
		InfoLog("[console]");
	}
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsConsole::getln(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TStdIn StdIn;
	TStr LnStr;
	StdIn.GetNextLn(LnStr);
	return HandleScope.Close(v8::String::New(LnStr.CStr()));
}

v8::Handle<v8::Value> TJsConsole::print(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TStr Str = TJsConsoleUtil::GetStr(Args[0]->ToString());
	printf("%s", Str.CStr());
	return HandleScope.Close(v8::Undefined());
}

///////////////////////////////
// QMiner-JavaScript-Fs
v8::Handle<v8::ObjectTemplate> TJsFs::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
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
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
}

bool TJsFs::CanAccess(const TStr& FPath) {
    if (!TEnv::IsSandbox()) { return true; }
    TJsFPath JsFPath(FPath);
    for (int AllowedFPathN = 0; AllowedFPathN < AllowedFPathV.Len(); AllowedFPathN++) {
        if (JsFPath.IsSubdir(AllowedFPathV[AllowedFPathN])) { return true; }
    }
    return false;
}

bool TJsFs::CanAccess(const v8::Arguments& Args) {
    if (!TEnv::IsSandbox()) { return true; }
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
	return TJsFOut::New(FNm);
}

v8::Handle<v8::Value> TJsFs::openAppend(const v8::Arguments& Args) { // call with AppendP = true 
	v8::HandleScope HandleScope;
	TJsFs* JsFs = TJsFsUtil::GetSelf(Args);
    TStr FNm = TJsFsUtil::GetArgStr(Args, 0);
    QmAssertR(JsFs->CanAccess(FNm), "You don't have permission to access file '" + FNm + "'");
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
PSIn TJsFIn::GetArgFIn(const v8::Arguments& Args, const int& ArgN) {
    v8::HandleScope HandleScope;
    // check we have the argument at all
    AssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    // check it's of the right type
    AssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be Object", ArgN));
    // get the wrapped 
    v8::Handle<v8::Object> _JsFIn = v8::Handle<v8::Object>::Cast(Val);
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(_JsFIn->GetInternalField(0));
    // cast it to record set
    TJsFIn* JsFIn = static_cast<TJsFIn*>(WrappedObject->Value());
    return JsFIn->SIn;    
}

v8::Handle<v8::ObjectTemplate> TJsFIn::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, peekCh);
		JsRegisterFunction(TmpTemp, getCh);
		JsRegisterFunction(TmpTemp, readLine);
		JsLongRegisterFunction(TmpTemp, "getNextLn", readLine);
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

v8::Handle<v8::Value> TJsFIn::readLine(const v8::Arguments& Args) {
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
PSOut TJsFOut::GetArgFOut(const v8::Arguments& Args, const int& ArgN) {
    v8::HandleScope HandleScope;
    // check we have the argument at all
    AssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d", ArgN));
    v8::Handle<v8::Value> Val = Args[ArgN];
    // check it's of the right type
    AssertR(Val->IsObject(), TStr::Fmt("Argument %d expected to be Object", ArgN));
    // get the wrapped 
    v8::Handle<v8::Object> _JsFOut = v8::Handle<v8::Object>::Cast(Val);
    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(_JsFOut->GetInternalField(0));
    // cast it to record set
    TJsFOut* JsFOut = static_cast<TJsFOut*>(WrappedObject->Value());
    return JsFOut->SOut;    
}

v8::Handle<v8::ObjectTemplate> TJsFOut::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, write);
		JsRegisterFunction(TmpTemp, writeLine);
        JsRegisterFunction(TmpTemp, flush);
        JsRegisterFunction(TmpTemp, close);
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
    QmAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
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
    // first we write, if we have any argument
    int OutN = 0;
    if (TJsFOutUtil::IsArg(Args, 0)) {
        v8::Handle<v8::Value> Res = write(Args);
        OutN += Res->Int32Value();
    }
    // then we make a new line
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);  
    QmAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
    OutN += JsFOut->SOut->PutLn();
	return v8::Integer::New(OutN);
}

v8::Handle<v8::Value> TJsFOut::flush(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);
    QmAssertR(!JsFOut->SOut.Empty(), "Output stream already closed!");
	JsFOut->SOut->Flush();
	return v8::Undefined();
}

v8::Handle<v8::Value> TJsFOut::close(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsFOut* JsFOut = TJsFOutUtil::GetSelf(Args);
	JsFOut->SOut.Clr();
	return v8::Undefined();
}

///////////////////////////////
// QMiner-JavaScript-Console
v8::Handle<v8::ObjectTemplate> TJsHttp::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterFunction(TmpTemp, onRequest);
		JsLongRegisterFunction(TmpTemp, "get", get);
		JsLongRegisterFunction(TmpTemp, "getStr", get);
		JsLongRegisterFunction(TmpTemp, "post", post);
		JsLongRegisterFunction(TmpTemp, "postStr", post);
		TmpTemp->SetInternalFieldCount(1);
		Template = v8::Persistent<v8::ObjectTemplate>::New(TmpTemp);
	}
	return Template;
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

v8::Handle<v8::ObjectTemplate> TJsHttpResp::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
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

v8::Handle<v8::ObjectTemplate> TJsTm::GetTemplate() {
	v8::HandleScope HandleScope;
	static v8::Persistent<v8::ObjectTemplate> Template;
	if (Template.IsEmpty()) {
		v8::Handle<v8::ObjectTemplate> TmpTemp = v8::ObjectTemplate::New();
		JsRegisterProperty(TmpTemp, string);
        JsRegisterProperty(TmpTemp, dateString);
		JsRegisterProperty(TmpTemp, timestamp);
        JsRegisterProperty(TmpTemp, year);
        JsRegisterProperty(TmpTemp, month);
        JsRegisterProperty(TmpTemp, day);
        JsRegisterProperty(TmpTemp, dayOfWeek);
        JsRegisterProperty(TmpTemp, hour);
        JsRegisterProperty(TmpTemp, minute);
        JsRegisterProperty(TmpTemp, second);
        JsRegisterProperty(TmpTemp, milisecond);                
        JsRegisterProperty(TmpTemp, now);
        JsRegisterProperty(TmpTemp, nowUTC);
        JsRegisterFunction(TmpTemp, add);
		JsRegisterFunction(TmpTemp, sub);
		JsRegisterFunction(TmpTemp, toJSON);
		JsRegisterFunction(TmpTemp, parse);
		JsRegisterFunction(TmpTemp, clone);
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

v8::Handle<v8::Value> TJsTm::dateString(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Info);
    TStr TmStr = JsTm->Tm.GetWebLogDateStr();   
	return HandleScope.Close(v8::String::New(TmStr.CStr()));
}

v8::Handle<v8::Value> TJsTm::timestamp(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Info);
    const int Timestamp = TTm::GetDateTimeIntFromTm(JsTm->Tm);
	return HandleScope.Close(v8::Int32::New(Timestamp));
}

v8::Handle<v8::Value> TJsTm::year(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetYear()));
}

v8::Handle<v8::Value> TJsTm::month(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetMonth()));
}

v8::Handle<v8::Value> TJsTm::day(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetDay()));
}

v8::Handle<v8::Value> TJsTm::dayOfWeek(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
    TSecTm SecTm(TJsTmUtil::GetSelf(Info)->Tm);
	return HandleScope.Close(v8::String::New(SecTm.GetDayOfWeekNm().CStr()));
}

v8::Handle<v8::Value> TJsTm::hour(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetHour()));
}

v8::Handle<v8::Value> TJsTm::minute(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetMin()));
}

v8::Handle<v8::Value> TJsTm::second(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetSec()));
}

v8::Handle<v8::Value> TJsTm::milisecond(v8::Local<v8::String> Properties, const v8::AccessorInfo& Info) {
	v8::HandleScope HandleScope;
	return HandleScope.Close(v8::Int32::New(TJsTmUtil::GetSelf(Info)->Tm.GetMSec()));
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
    return HandleScope.Close(Args.Holder());
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
    return HandleScope.Close(Args.Holder());
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

v8::Handle<v8::Value> TJsTm::clone(const v8::Arguments& Args) {
	v8::HandleScope HandleScope;
	TJsTm* JsTm = TJsTmUtil::GetSelf(Args);
	// return constructed json
	return HandleScope.Close(TJsTm::New(JsTm->Tm));
}

///////////////////////////////////////////////
// Javascript Function Feature Extractor
TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TFtrExt(Base, ParamVal) { 
	throw TQmExcept::New("javascript function feature extractor shouldn't be constructed calling TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal), call TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal) instead (construct from JS using analytics)");
}

TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, TSIn& SIn) : TFtrExt(Base, SIn) {
	throw TQmExcept::New("javascript function feature extractor shouldn't be constructed calling TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, TSIn& SIn), call TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal) instead (construct from JS using analytics)");
}

PFtrExt TJsFuncFtrExt::New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
	return new TJsFuncFtrExt(Base, ParamVal);
}

PFtrExt TJsFuncFtrExt::Load(const TWPt<TBase>& Base, TSIn& SIn) {
	return new TJsFuncFtrExt(Base, SIn);
}
void TJsFuncFtrExt::Save(TSOut& SOut) const {
	throw TQmExcept::New("TJsFuncFtrExt::Save(TSOut& Sout) : saving is not supported");
}

void TJsFuncFtrExt::AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
	if (Dim == 1) {
		SpV.Add(TIntFltKd(Offset, ExecuteFunc(FtrRec))); Offset++;
	}
	else {
		TFltV Res;
		ExecuteFuncVec(FtrRec, Res);
		QmAssertR(Res.Len() == Dim, "JsFuncFtrExt::AddSpV Dim != result dimension!");
		for (int ElN = 0; ElN < Dim; ElN++) {
			SpV.Add(TIntFltKd(Offset + ElN, Res[ElN]));
		}
		Offset += Dim;
	}
}

void TJsFuncFtrExt::AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const {
	if (Dim == 1) {
		FullV[Offset] = ExecuteFunc(Rec); Offset++;
	}
	else {		
		TFltV Res;
		ExecuteFuncVec(Rec, Res);
		QmAssertR(Res.Len() == Dim, "JsFuncFtrExt::AddFullV Dim != result dimension!");
		for (int ElN = 0; ElN < Dim; ElN++) {
			FullV[Offset + ElN] = Res[ElN];
		}
		Offset += Dim;
	}
}

void TJsFuncFtrExt::ExtractFltV(const TRec& FtrRec, TFltV& FltV) const {
	if (Dim == 1) {
		FltV.Add(ExecuteFunc(FtrRec));
	}
	else {
		TFltV Res;
		ExecuteFuncVec(FtrRec, Res);
		QmAssertR(Res.Len() == Dim, "JsFuncFtrExt::ExtractFltV Dim != result dimension!");
		FltV.AddV(Res);
	}
}

}

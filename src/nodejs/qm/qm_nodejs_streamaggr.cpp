/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "qm_nodejs.h"
#include "qm_param.h"
#include "../la/la_nodejs.h"

#include <node_buffer.h>

///////////////////////////////
// NodeJs Stream Aggregator

v8::Persistent<v8::Function> TNodeJsSA::Constructor;

void TNodeJsSA::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// template for creating function from javascript using "new", uses _NewJs callback
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsSA>);
	// child will have the same properties and methods, but a different callback: _NewCpp
	v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsSA>);
	child->Inherit(tpl);

	child->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	child->InstanceTemplate()->SetInternalFieldCount(1);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "reset", _reset);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAdd", _onAdd);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onUpdate", _onUpdate);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onDelete", _onDelete);
	NODE_SET_PROTOTYPE_METHOD(tpl, "saveJson", _saveJson);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInteger", _getInteger);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFloat", _getFloat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestamp", _getTimestamp);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatLength", _getFloatLength);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatAt", _getFloatAt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFloatVector", _getFloatVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampLength", _getTimestampLength);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampAt", _getTimestampAt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimestampVector", _getTimestampVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInFloat", _getInFloat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInTimestamp", _getInTimestamp);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutFloatVector", _getOutFloatVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutTimestampVector", _getOutTimestampVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getNumberOfRecords", _getNumberOfRecords);

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "val"), _val);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript	
	// Constructor is used when creating the object from C++
	Constructor.Reset(Isolate, child->GetFunction());
	// we need to export the class for calling using "new FIn(...)"
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}
TNodeJsSA* TNodeJsSA::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() <= 3 && Args.Length() >= 2, "stream aggregator constructor expects at least two parameters");
	QmAssertR(Args[0]->IsObject() && Args[1]->IsObject(), "stream aggregator constructor expects first two arguments as objects");

	TQm::PStreamAggr StreamAggr;

	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args[0]->ToObject());

	// get aggregate type
	TStr TypeNm = TNodeJsUtil::GetArgStr(Args, 1, "type", "javaScript");

	if (TypeNm == "javaScript") {
		// we have a javascript stream aggregate
		TStr AggrName = TNodeJsUtil::GetArgStr(Args, 1, "name", "");
		// we need a name, if not give just generate one
		if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
		// create aggregate
		StreamAggr = TNodeJsStreamAggr::New(JsBase->Base, AggrName, Args[1]->ToObject());
	} else if (TypeNm == "ftrext") {
		TStr AggrName = TNodeJsUtil::GetArgStr(Args, 1, "name", "");
		QmAssertR(Args[1]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, "featureSpace")), "addStreamAggr: featureSpace property missing!");
		// we need a name, if not give just generate one
		if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
		throw TQm::TQmExcept::New("ftrext stream aggr not implemented yet! (needs feature space implementation)");
		// TODO
		//TQm::PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args[1]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, "featureSpace")));
		//StreamAggr = TStreamAggrs::TFtrExtAggr::New(JsBase->Base, AggrName, FtrSpace);
	} else if (TypeNm == "stmerger") {
		// create new aggregate
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 1);
		StreamAggr = TQm::TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
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
			} else if (SourceVal->IsObj()) {
				// get store
				StoreNm = SourceVal->GetObjStr("store");
			}
			JsBase->Base->AddStreamAggr(JsBase->Base->GetStoreByStoreNm(StoreNm)->GetStoreId(), StreamAggr);
		}
	} else {
		// we have a GLib stream aggregate, translate parameters to PJsonVal
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 1);
		if (Args.Length() >= 3 && Args[2]->IsString()) {
			ParamVal->AddToObj("store", TNodeJsUtil::GetArgStr(Args, 2));
		}
        // create new aggregate
        StreamAggr = TQm::TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
	}

    // handle special case when we have more then one store to attach to (e.g. merger)
    if (Args.Length() > 2) {
        TStrV Stores(0);
        if (Args[2]->IsString()) {
            Stores.Add(TNodeJsUtil::GetArgStr(Args, 2));
        } else if (Args[2]->IsArray()) {
            PJsonVal StoresJson = TNodeJsUtil::GetArgJson(Args, 2);
            QmAssertR(StoresJson->IsDef(), "stream aggr constructor : Args[2] should be a string (store name) or a string array (store names)");
            StoresJson->GetArrStrV(Stores);
        }
        for (int StoreN = 0; StoreN < Stores.Len(); StoreN++) {
            QmAssertR(JsBase->Base->IsStoreNm(Stores[StoreN]), "stream aggr constructor : Args[2] : store does not exist!");
            JsBase->Base->AddStreamAggr(Stores[StoreN], StreamAggr);
        }
    } else {
        JsBase->Base->AddStreamAggr(StreamAggr);
    }

    return new TNodeJsSA(StreamAggr);
}

void TNodeJsSA::reset(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	JsSA->SA->Reset();

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::onAdd(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onAdd should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnAddRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::onUpdate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onUpdate should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnUpdateRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::onDelete(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onDelete should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnDeleteRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::saveJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());

	const TInt Limit = TNodeJsUtil::GetArgInt32(Args, 0, -1);
	PJsonVal Json = JsSA->SA->SaveJson(Limit);
	v8::Handle<v8::Value> V8Json = TNodeJsUtil::ParseJson(Isolate, Json);

	Args.GetReturnValue().Set(V8Json);
}

void TNodeJsSA::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);

    // save
	JsSA->SA->SaveState(*JsFOut->SOut);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
    TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);

    // save
	JsSA->SA->LoadState(*JsFIn->SIn);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::getInteger(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IInt
	TWPt<TQm::TStreamAggrOut::IInt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IInt*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getInt : stream aggregate does not implement IInt: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetInt()));
}

void TNodeJsSA::getFloat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFlt
	TWPt<TQm::TStreamAggrOut::IFlt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFlt*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getFlt : stream aggregate does not implement IFlt: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetFlt()));
}

void TNodeJsSA::getTimestamp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as ITm
	TWPt<TQm::TStreamAggrOut::ITm> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITm*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getTm : stream aggregate does not implement ITm: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetTmMSecs()));
}

void TNodeJsSA::getFloatLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltVec
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getFltLen : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetFltLen()));
}
void TNodeJsSA::getFloatAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltVec
	int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getFltAt : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetFlt(ElN)));
}
void TNodeJsSA::getFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltVec
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getFltV : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}
	TFltV Res;
	Aggr->GetFltV(Res);
	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}
void TNodeJsSA::getTimestampLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());

	// try to cast as ITmVec
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getTmLen : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetTmLen()));
}
void TNodeJsSA::getTimestampAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());

	// try to cast as ITmVec
	int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getTmAt : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
	}
	QmAssertR(JsSA->SA->IsInit(), "TNodeJsSA::getTmAt : stream aggregate '" + JsSA->SA->GetAggrNm() + "' is not initialized!");
	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetTm(ElN)));
}
void TNodeJsSA::getTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as ITmVec
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getTmV : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
	}
	TUInt64V Res;
	Aggr->GetTmV(Res);
	int Len = Res.Len();
	TFltV FltRes(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		FltRes[ElN] = (double)Res[ElN];
	}

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}
void TNodeJsSA::getInFloat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getInFlt : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetInFlt()));
}
void TNodeJsSA::getInTimestamp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getInTm : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetInTmMSecs()));
}
void TNodeJsSA::getOutFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getOutFltV : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}
	TFltV Res;
	Aggr->GetOutFltV(Res);

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsSA::getOutTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getOutTmV : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}
	TUInt64V Res;
	Aggr->GetOutTmMSecsV(Res);
	int Len = Res.Len();
	TFltV FltRes(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		FltRes[ElN] = (double)Res[ElN];
	}

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}

void TNodeJsSA::getNumberOfRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsSA::getN : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetN()));
}

void TNodeJsSA::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsSA->SA->GetAggrNm().CStr()));
}


void TNodeJsSA::val(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Self);
	Info.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsSA->SA->SaveJson(-1)));
}

void TNodeJsSA::init(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Self);
	Info.GetReturnValue().Set(JsSA->SA->IsInit());
}

///////////////////////////////
// QMiner-JavaScript-Stream-Aggr
TNodeJsStreamAggr::TNodeJsStreamAggr(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal) : TStreamAggr(_Base, _AggrNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// Every stream aggregate should implement these two
	QmAssertR(TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "onAdd")), "TNodeJsStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing onAdd callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");
	QmAssertR(TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "saveJson")), "TNodeJsStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing saveJson callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");

	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "reset"))) {
		v8::Handle<v8::Value> _ResetFun = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "reset"));
		QmAssert(_ResetFun->IsFunction());
		ResetFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_ResetFun));
	}

	v8::Handle<v8::Value> _OnAddFun = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "onAdd"));
	QmAssert(_OnAddFun->IsFunction());
	OnAddFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_OnAddFun));

	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "onUpdate"))) {
		v8::Handle<v8::Value> _OnUpdateFun = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "onUpdate"));
		QmAssert(_OnUpdateFun->IsFunction());
		OnUpdateFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_OnUpdateFun));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "onDelete"))) {
		v8::Handle<v8::Value> _OnDeleteFun = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "onDelete"));
		QmAssert(_OnDeleteFun->IsFunction());
		OnDeleteFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_OnDeleteFun));
	}

	v8::Handle<v8::Value> _SaveJsonFun = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "saveJson"));
	QmAssert(_SaveJsonFun->IsFunction());
	SaveJsonFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_SaveJsonFun));

	// StreamAggr::_Save
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "save"))) {
		v8::Handle<v8::Value> _Save = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "save"));
		QmAssert(_Save->IsFunction());
		SaveFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_Save));
	}

	// StreamAggr::_Load
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "load"))) {
		v8::Handle<v8::Value> _Load = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "load"));
		QmAssert(_Load->IsFunction());
		LoadFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_Load));
	}

	// IInt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInt"))) {
		v8::Handle<v8::Value> _GetInt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInt"));
		QmAssert(_GetInt->IsFunction());
		GetIntFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInt));
	}
	// IFlt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFlt"))) {
		v8::Handle<v8::Value> _GetFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFlt"));
		QmAssert(_GetFlt->IsFunction());
		GetFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFlt));
	}
	// ITm
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getTm"))) {
		v8::Handle<v8::Value> _GetTm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getTm"));
		QmAssert(_GetTm->IsFunction());
		GetTmMSecsFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetTm));
	}
	// IFltTmIO
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInFlt"))) {
		v8::Handle<v8::Value> _GetInFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInFlt"));
		QmAssert(_GetInFlt->IsFunction());
		GetInFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInFlt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInTm"))) {
		v8::Handle<v8::Value> _GetInTm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInTm"));
		QmAssert(_GetInTm->IsFunction());
		GetInTmMSecsFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInTm));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOutFltV"))) {
		v8::Handle<v8::Value> _GetOutFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOutFltV"));
		QmAssert(_GetOutFltV->IsFunction());
		GetOutFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOutFltV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOutTmV"))) {
		v8::Handle<v8::Value> _GetOutTmV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOutTmV"));
		QmAssert(_GetOutTmV->IsFunction());
		GetOutTmMSecsVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOutTmV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getN"))) {
		v8::Handle<v8::Value> _GetN = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getN"));
		QmAssert(_GetN->IsFunction());
		GetNFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetN));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOldestFlt"))) {
		v8::Handle<v8::Value> _GetOldestFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOldestFlt"));
		QmAssert(_GetOldestFlt->IsFunction());
		GetOldestFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOldestFlt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOldestTm"))) {
		v8::Handle<v8::Value> _GetOldestTm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOldestTm"));
		QmAssert(_GetOldestTm->IsFunction());
		GetOldestTmMSecsFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOldestTm));
	}

	// IFltVec
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFltLen"))) {
		v8::Handle<v8::Value> _GetFltLen = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFltLen"));
		QmAssert(_GetFltLen->IsFunction());
		GetFltLenFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltLen));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFltAt"))) {
		v8::Handle<v8::Value> _GetFltAt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFltAt"));
		QmAssert(_GetFltAt->IsFunction());
		GetFltAtFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltAt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFltV"))) {
		v8::Handle<v8::Value> _GetFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFltV"));
		QmAssert(_GetFltV->IsFunction());
		GetFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltV));
	}
	// INmFlt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "isNmFlt"))) {
		v8::Handle<v8::Value> _IsNmFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "isNmFlt"));
		QmAssert(_IsNmFlt->IsFunction());
		IsNmFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_IsNmFlt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getNmFlt"))) {
		v8::Handle<v8::Value> _GetNmFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getNmFlt"));
		QmAssert(_GetNmFlt->IsFunction());
		GetNmFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetNmFlt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getNmFltV"))) {
		v8::Handle<v8::Value> _GetNmFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getNmFltV"));
		QmAssert(_GetNmFltV->IsFunction());
		GetNmFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetNmFltV));
	}
	// INmInt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "isNm"))) {
		v8::Handle<v8::Value> _IsNm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "isNm"));
		QmAssert(_IsNm->IsFunction());
		IsNmFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_IsNm));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getNmInt"))) {
		v8::Handle<v8::Value> _GetNmInt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getNmInt"));
		QmAssert(_GetNmInt->IsFunction());
		GetNmIntFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetNmInt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getNmIntV"))) {
		v8::Handle<v8::Value> _GetNmIntV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getNmIntV"));
		QmAssert(_GetNmIntV->IsFunction());
		GetNmIntVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetNmIntV));
	}
}

TNodeJsStreamAggr::~TNodeJsStreamAggr() {
	// callbacks
	ResetFun.Reset();
	OnAddFun.Reset();
	OnUpdateFun.Reset();
	OnDeleteFun.Reset();
	SaveJsonFun.Reset();

	GetIntFun.Reset();
	// IFlt
	GetFltFun.Reset();
	// ITm
	GetTmMSecsFun.Reset();
	// IFltTmIO
	GetInFltFun.Reset();
	GetInTmMSecsFun.Reset();
	GetOutFltVFun.Reset();
	GetOutTmMSecsVFun.Reset();
	GetNFun.Reset();
	GetOldestFltFun.Reset();
	GetOldestTmMSecsFun.Reset();
	// IFltVec
	GetFltLenFun.Reset();
	GetFltAtFun.Reset();
	GetFltVFun.Reset();
	// ITmVec
	GetTmLenFun.Reset();
	GetTmAtFun.Reset();
	GetTmVFun.Reset();
	// INmFlt
	IsNmFltFun.Reset();
	GetNmFltFun.Reset();
	GetNmFltVFun.Reset();
	// INmInt
	IsNmFun.Reset();
	GetNmIntFun.Reset();
	GetNmIntVFun.Reset();

	// Serialization
	SaveFun.Reset();
	LoadFun.Reset();
}

void TNodeJsStreamAggr::Reset() {
	if (!ResetFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, ResetFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();	

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered in " + TStr(__FUNCTION__) + TStr(": ") + TStr(*Msg));
		}
	}
}

void TNodeJsStreamAggr::OnAddRec(const TQm::TRec& Rec) {
	if (!OnAddFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnAddFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return;
		}
	}
}

void TNodeJsStreamAggr::OnUpdateRec(const TQm::TRec& Rec) {
	if (!OnUpdateFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnUpdateFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return;
		}
	}
}

void TNodeJsStreamAggr::OnDeleteRec(const TQm::TRec& Rec) {
	if (!OnDeleteFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OnDeleteFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec)) };
		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return;
		}
	}
}

PJsonVal TNodeJsStreamAggr::SaveJson(const int& Limit) const {
	if (!SaveJsonFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SaveJsonFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { v8::Number::New(Isolate, Limit) };
		v8::TryCatch TryCatch;
		v8::Local<v8::Value> ReturnVal = Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return TJsonVal::NewObj();
		}
		QmAssertR(ReturnVal->IsObject(), "Stream aggr JS callback: saveJson didn't return an object.");
		PJsonVal Res = TNodeJsUtil::GetObjJson(ReturnVal->ToObject());

		QmAssertR(Res->IsDef(), "Stream aggr JS callback: saveJson didn't return a valid JSON.");
		return Res;
	}
	else {
		return TJsonVal::NewObj();
	}
}

void TNodeJsStreamAggr::_Save(TSOut& SOut) const {
	if (SaveFun.IsEmpty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::_Save (called using sa.save) : stream aggregate does not implement a save callback: " + GetAggrNm());
	}
}

void TNodeJsStreamAggr::_Load(TSIn& SIn) {
	if (LoadFun.IsEmpty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::_Load (called using sa.load) : stream aggregate does not implement a load callback: " + GetAggrNm());
	}
}

// IInt
int TNodeJsStreamAggr::GetInt() const {
	if (!GetIntFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetIntFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return 0;
		}
		QmAssertR(RetVal->IsInt32(), "TNodeJsStreamAggr, name: " + GetAggrNm() + ", getInt(): Return type expected to be int32");
		return RetVal->Int32Value();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", getInt() callback is empty!");
	}
}
// IFlt
double TNodeJsStreamAggr::GetFlt() const {
	if (!GetFltFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFltFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return 0.0;
		}
		QmAssertR(RetVal->IsNumber(), "TNodeJsStreamAggr, name: " + GetAggrNm() + ", getFlt(): Return type expected to be int32");
		return RetVal->NumberValue();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", getFlt() callback is empty!");
	}
}
// ITm
uint64 TNodeJsStreamAggr::GetTmMSecs() const {
	if (!GetTmMSecsFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetTmMSecsFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			TryCatch.ReThrow();
			return 0;
		}
		QmAssertR(RetVal->IsNumber(), "TNodeJsStreamAggr, name: " + GetAggrNm() + ", getTm(): Return type expected to be number");
		return (uint64)RetVal->NumberValue();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", getTm() callback is empty!");
	}
}
// IFltTmIO
double TNodeJsStreamAggr::GetInFlt() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetInFlt not implemented");
}
uint64 TNodeJsStreamAggr::GetInTmMSecs() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetInTmMSecs not implemented");
}
void TNodeJsStreamAggr::GetOutFltV(TFltV& ValV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetOutFltV not implemented");
}
void TNodeJsStreamAggr::GetOutTmMSecsV(TUInt64V& MSecsV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetOutTmMSecsV not implemented");
}
int TNodeJsStreamAggr::GetN() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetN not implemented");
}
double TNodeJsStreamAggr::GetOldestFlt() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetOldestFlt not implemented");
}
uint64 TNodeJsStreamAggr::GetOldestTmMSecs() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetOldestTmMSecs not implemented");
}

// IFltVec
int TNodeJsStreamAggr::GetFltLen() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetFltLen not implemented");
}
double TNodeJsStreamAggr::GetFlt(const TInt& ElN) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetFlt not implemented");
} // GetFltAtFun
void TNodeJsStreamAggr::GetFltV(TFltV& ValV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetFltV not implemented");
}
// ITmVec
int TNodeJsStreamAggr::GetTmLen() const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetTmLen not implemented");
}
uint64 TNodeJsStreamAggr::GetTm(const TInt& ElN) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetTm not implemented");
} // GetTmAtFun
void TNodeJsStreamAggr::GetTmV(TUInt64V& TmMSecsV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetTmV not implemented");
}
// INmFlt
bool TNodeJsStreamAggr::IsNmFlt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", IsNmFlt not implemented");
}
double TNodeJsStreamAggr::GetNmFlt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetNmFlt not implemented");
}
void TNodeJsStreamAggr::GetNmFltV(TStrFltPrV& NmFltV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetNmFltV not implemented");
}
// INmInt
bool TNodeJsStreamAggr::IsNm(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", IsNm not implemented");
}
double TNodeJsStreamAggr::GetNmInt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetNmInt not implemented");
}
void TNodeJsStreamAggr::GetNmIntV(TStrIntPrV& NmIntV) const {
	throw  TQm::TQmExcept::New("TNodeJsStreamAggr, name: " + GetAggrNm() + ", GetNmIntV not implemented");
}

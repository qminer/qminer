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

v8::Persistent<v8::Function> TNodeJsStreamAggr::Constructor;

void TNodeJsStreamAggr::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// template for creating function from javascript using "new", uses _NewJs callback
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsStreamAggr>);
	// child will have the same properties and methods, but a different callback: _NewCpp
	v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStreamAggr>);
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInTimestamp", _getInTimestamp); // multiple interfaces
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInFloatVector", _getInFloatVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInTimestampVector", _getInTimestampVector); // multiple interfaces
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutFloatVector", _getOutFloatVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutTimestampVector", _getOutTimestampVector); // multiple interfaces
	NODE_SET_PROTOTYPE_METHOD(tpl, "getNumberOfRecords", _getNumberOfRecords); // multiple interfaces

	NODE_SET_PROTOTYPE_METHOD(tpl, "getInValueVector", _getInValueVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutValueVector", _getOutValueVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getValueVector", _getValueVector);
	
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureSpace", _getFeatureSpace);

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

TNodeJsStreamAggr* TNodeJsStreamAggr::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
		StreamAggr = TNodeJsFuncStreamAggr::New(JsBase->Base, AggrName, Args[1]->ToObject());
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

    return new TNodeJsStreamAggr(StreamAggr);
}

void TNodeJsStreamAggr::reset(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	JsSA->SA->Reset();

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamAggr::onAdd(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onAdd should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnAddRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamAggr::onUpdate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onUpdate should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnUpdateRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamAggr::onDelete(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onDelete should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnDeleteRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamAggr::saveJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	const TInt Limit = TNodeJsUtil::GetArgInt32(Args, 0, -1);
	PJsonVal Json = JsSA->SA->SaveJson(Limit);
	v8::Handle<v8::Value> V8Json = TNodeJsUtil::ParseJson(Isolate, Json);

	Args.GetReturnValue().Set(V8Json);
}

void TNodeJsStreamAggr::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    // save
	JsSA->SA->SaveState(*JsFOut->SOut);
	JsFOut->SOut->Flush();
	Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsStreamAggr::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
    TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);

    // save
	JsSA->SA->LoadState(*JsFIn->SIn);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamAggr::getInteger(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IInt
	TWPt<TQm::TStreamAggrOut::IInt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IInt*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInt : stream aggregate does not implement IInt: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetInt()));
}

void TNodeJsStreamAggr::getFloat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFlt
	TWPt<TQm::TStreamAggrOut::IFlt> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFlt*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFlt : stream aggregate does not implement IFlt: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetFlt()));
}

void TNodeJsStreamAggr::getTimestamp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as ITm
	TWPt<TQm::TStreamAggrOut::ITm> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITm*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTm : stream aggregate does not implement ITm: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetTmMSecs()));
}

void TNodeJsStreamAggr::getFloatLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltVec
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltLen : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetVals()));
}
void TNodeJsStreamAggr::getFloatAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltVec
	int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltAt : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}

	TFlt Flt;
	Aggr->GetVal(ElN, Flt);
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Flt));
}
void TNodeJsStreamAggr::getFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltVec
	TWPt<TQm::TStreamAggrOut::IFltVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFltV : stream aggregate does not implement IFltVec: " + JsSA->SA->GetAggrNm());
	}
	TFltV Res;
	Aggr->GetValV(Res);
	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}
void TNodeJsStreamAggr::getTimestampLength(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	// try to cast as ITmVec
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmLen : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetTmLen()));
}
void TNodeJsStreamAggr::getTimestampAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	// try to cast as ITmVec
	int ElN = TNodeJsUtil::GetArgInt32(Args, 0);
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmAt : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
	}
	QmAssertR(JsSA->SA->IsInit(), "TNodeJsStreamAggr::getTmAt : stream aggregate '" + JsSA->SA->GetAggrNm() + "' is not initialized!");
	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetTm(ElN)));
}
void TNodeJsStreamAggr::getTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as ITmVec
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getTmV : stream aggregate does not implement ITmVec: " + JsSA->SA->GetAggrNm());
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
void TNodeJsStreamAggr::getInFloat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInFlt : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetInVal()));
}
void TNodeJsStreamAggr::getInTimestamp(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::ITmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInTm : stream aggregate does not implement ITmIO: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetInTmMSecs()));
}

void TNodeJsStreamAggr::getInFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInFloatVector : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}
	TFltV Res;
	Aggr->GetInValV(Res);

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsStreamAggr::getInTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::ITmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutTmV : stream aggregate does not implement ITmIO: " + JsSA->SA->GetAggrNm());
	}
	TUInt64V Res;
	Aggr->GetInTmMSecsV(Res);
	int Len = Res.Len();
	TFltV FltRes(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		FltRes[ElN] = (double)Res[ElN];
	}

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltRes));
}

void TNodeJsStreamAggr::getOutFloatVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutFloatVector : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
	}
	TFltV Res;
	Aggr->GetOutValV(Res);

	Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsStreamAggr::getOutTimestampVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::ITmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutTmV : stream aggregate does not implement ITmIO: " + JsSA->SA->GetAggrNm());
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

void TNodeJsStreamAggr::getNumberOfRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::ITmVec> Aggr = dynamic_cast<TQm::TStreamAggrOut::ITmVec*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getNumberOfRecords : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Aggr->GetTmLen()));
}

void TNodeJsStreamAggr::getInValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	// try to cast as IValTmIO
	TWPt<TQm::TStreamAggrOut::IValTmIO<TFlt> > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IValTmIO<TFlt> *>(JsSA->SA());
	TWPt<TQm::TStreamAggrOut::IValTmIO<TIntFltKdV> > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::IValTmIO<TIntFltKdV> *>(JsSA->SA());

	if (!AggrFlt.Empty()) {
		TFltV Res;
		AggrFlt->GetInValV(Res);
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
	}
	else if (!AggrSpV.Empty()){
		TVec<TIntFltKdV> Res;
		AggrSpV->GetInValV(Res);
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
	}
	else {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getInValueVector : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
	}
}

void TNodeJsStreamAggr::getOutValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());
		
	// try to cast as IValTmIO
	TWPt<TQm::TStreamAggrOut::IValTmIO<TFlt> > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IValTmIO<TFlt> *>(JsSA->SA());
	TWPt<TQm::TStreamAggrOut::IValTmIO<TIntFltKdV> > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::IValTmIO<TIntFltKdV> *>(JsSA->SA());
		
	if (!AggrFlt.Empty()) {
		TFltV Res;
		AggrFlt->GetOutValV(Res);
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
	}
	else if (!AggrSpV.Empty()){
		TVec<TIntFltKdV> Res;
		AggrSpV->GetOutValV(Res);
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
	} else {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getOutValueVector : stream aggregate does not implement IValTmIO: " + JsSA->SA->GetAggrNm());
	}
}

void TNodeJsStreamAggr::getFeatureSpace(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	// try to cast as IFtrSpace
	TWPt<TQm::TStreamAggrOut::IFtrSpace > Aggr = dynamic_cast<TQm::TStreamAggrOut::IFtrSpace *>(JsSA->SA());

	if (!Aggr.Empty()) {
		TQm::PFtrSpace FtrSpace = Aggr->GetFtrSpace();		
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsFtrSpace>(new TNodeJsFtrSpace(FtrSpace)));
	} else {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getFeatureSpace : stream aggregate does not implement IFtrSpace: " + JsSA->SA->GetAggrNm());
	}
}

void TNodeJsStreamAggr::getValueVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Args.Holder());

	// try to cast as IValTmIO
	TWPt<TQm::TStreamAggrOut::IFltVec > AggrFlt = dynamic_cast<TQm::TStreamAggrOut::IFltVec *>(JsSA->SA());
	TWPt<TQm::TStreamAggrOut::ISparseVVec > AggrSpV = dynamic_cast<TQm::TStreamAggrOut::ISparseVVec *>(JsSA->SA());
	TWPt<TQm::TStreamAggrOut::ISparseVec > SpV = dynamic_cast<TQm::TStreamAggrOut::ISparseVec *>(JsSA->SA());

	if (!AggrFlt.Empty()) {
		TFltV Res;
		AggrFlt->GetValV(Res);
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
	} else if (!AggrSpV.Empty()){
		TVec<TIntFltKdV> Res;
		AggrSpV->GetValV(Res);
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Res)));
	} else if (!SpV.Empty()) {
		TIntFltKdV Res;
		SpV->GetValV(Res);
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(Res)));
	} else {
		throw TQm::TQmExcept::New("TNodeJsStreamAggr::getValueVector : stream aggregate does not implement IValVec: " + JsSA->SA->GetAggrNm());
	}
}

void TNodeJsStreamAggr::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsSA->SA->GetAggrNm().CStr()));
}


void TNodeJsStreamAggr::val(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
	Info.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsSA->SA->SaveJson(-1)));
}

void TNodeJsStreamAggr::init(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStreamAggr* JsSA = ObjectWrap::Unwrap<TNodeJsStreamAggr>(Self);
	Info.GetReturnValue().Set(JsSA->SA->IsInit());
}

///////////////////////////////
// QMiner-JavaScript-Stream-Aggr
TNodeJsFuncStreamAggr::TNodeJsFuncStreamAggr(TWPt<TQm::TBase> _Base, const TStr& _AggrNm, v8::Handle<v8::Object> TriggerVal) : TStreamAggr(_Base, _AggrNm) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// Every stream aggregate should implement these two
	QmAssertR(TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "onAdd")), "TNodeJsFuncStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing onAdd callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");
	QmAssertR(TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "saveJson")), "TNodeJsFuncStreamAggr constructor, name: " + _AggrNm + ", type: javaScript. Missing saveJson callback. Possible reason: type of the aggregate was not specified and it defaulted to javaScript.");

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
	
	// StreamAggr::IsInit
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "init"))) {
		v8::Handle<v8::Value> _IsInit = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "init"));
		QmAssert(_IsInit->IsFunction());
		IsInitFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_IsInit));
	}

	// StreamAggr::SaveState
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "save"))) {
		v8::Handle<v8::Value> _Save = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "save"));
		QmAssert(_Save->IsFunction());
		SaveFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_Save));
	}

	// StreamAggr::LoadState
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "load"))) {
		v8::Handle<v8::Value> _Load = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "load"));
		QmAssert(_Load->IsFunction());
		LoadFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_Load));
	}

	// IInt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInteger"))) {
		v8::Handle<v8::Value> _GetInt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInteger"));
		QmAssert(_GetInt->IsFunction());
		GetIntFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInt));
	}
	// IFlt
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFloat"))) {
		v8::Handle<v8::Value> _GetFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFloat"));
		QmAssert(_GetFlt->IsFunction());
		GetFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFlt));
	}
	// ITm
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getTimestamp"))) {
		v8::Handle<v8::Value> _GetTm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getTimestamp"));
		QmAssert(_GetTm->IsFunction());
		GetTmMSecsFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetTm));
	}
	// IFltTmIO
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInFloat"))) {
		v8::Handle<v8::Value> _GetInFlt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInFloat"));
		QmAssert(_GetInFlt->IsFunction());
		GetInFltFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInFlt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInTimestamp"))) {
		v8::Handle<v8::Value> _GetInTm = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInTimestamp"));
		QmAssert(_GetInTm->IsFunction());
		GetInTmMSecsFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInTm));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInFloatVector"))) {
		v8::Handle<v8::Value> _GetInFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInFloatVector"));
		QmAssert(_GetInFltV->IsFunction());
		GetInFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInFltV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getInTimestampVector"))) {
		v8::Handle<v8::Value> _GetInTmV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getInTimestampVector"));
		QmAssert(_GetInTmV->IsFunction());
		GetInTmMSecsVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetInTmV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOutFloatVector"))) {
		v8::Handle<v8::Value> _GetOutFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOutFloatVector"));
		QmAssert(_GetOutFltV->IsFunction());
		GetOutFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOutFltV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getOutTimestampVector"))) {
		v8::Handle<v8::Value> _GetOutTmV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getOutTimestampVector"));
		QmAssert(_GetOutTmV->IsFunction());
		GetOutTmMSecsVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetOutTmV));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getNumberOfRecords"))) {
		v8::Handle<v8::Value> _GetN = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getNumberOfRecords"));
		QmAssert(_GetN->IsFunction());
		GetNFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetN));
	}

	// IFltVec
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFloatLength"))) {
		v8::Handle<v8::Value> _GetFltLen = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFloatLength"));
		QmAssert(_GetFltLen->IsFunction());
		GetFltLenFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltLen));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFloatAt"))) {
		v8::Handle<v8::Value> _GetFltAt = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFloatAt"));
		QmAssert(_GetFltAt->IsFunction());
		GetFltAtFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltAt));
	}
	if (TriggerVal->Has(v8::String::NewFromUtf8(Isolate, "getFloatVector"))) {
		v8::Handle<v8::Value> _GetFltV = TriggerVal->Get(v8::String::NewFromUtf8(Isolate, "getFloatVector"));
		QmAssert(_GetFltV->IsFunction());
		GetFltVFun.Reset(Isolate, v8::Handle<v8::Function>::Cast(_GetFltV));
	}

	// TODO - fix glib names and expose the interfaces TNodeJsStreamAggr

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

TNodeJsFuncStreamAggr::~TNodeJsFuncStreamAggr() {
	// callbacks
	ResetFun.Reset();
	OnAddFun.Reset();
	OnUpdateFun.Reset();
	OnDeleteFun.Reset();
	SaveJsonFun.Reset();
	IsInitFun.Reset();

	GetIntFun.Reset();
	// IFlt
	GetFltFun.Reset();
	// ITm
	GetTmMSecsFun.Reset();
	// IFltTmIO
	GetInFltFun.Reset();
	GetInTmMSecsFun.Reset();
	GetInFltVFun.Reset();
	GetInTmMSecsVFun.Reset();
	GetOutFltVFun.Reset();
	GetOutTmMSecsVFun.Reset();
	GetNFun.Reset();
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

void TNodeJsFuncStreamAggr::Reset() {
	if (!ResetFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, ResetFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();	

		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered in " + TStr(__FUNCTION__) + TStr(": ") + TStr(*Msg));
		}
	}
}

void TNodeJsFuncStreamAggr::OnAddRec(const TQm::TRec& Rec) {
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
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered in TNodeJsFuncStreamAggr::OnAddRec :" + TStr(*Msg));
		}
	}
}

void TNodeJsFuncStreamAggr::OnUpdateRec(const TQm::TRec& Rec) {
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

void TNodeJsFuncStreamAggr::OnDeleteRec(const TQm::TRec& Rec) {
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

PJsonVal TNodeJsFuncStreamAggr::SaveJson(const int& Limit) const {
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
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered: " +  TStr(*Msg));
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

bool TNodeJsFuncStreamAggr::IsInit() const {
	if (!IsInitFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		
		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, IsInitFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered in TNodeJsFuncStreamAggr, name: " + GetAggrNm() + "," + TStr(*Msg));
		}
		QmAssertR(RetVal->IsBoolean(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", init did not return a boolean!");
		return RetVal->BooleanValue();
	} else {
		return true;
	}
}

void TNodeJsFuncStreamAggr::SaveState(TSOut& SOut) const {
	if (SaveFun.IsEmpty()) {
		throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::SaveState (called using sa.save) : stream aggregate does not implement a save callback: " + GetAggrNm());
	} else {
		// create TNodeJsFOut and pass it to callback
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		PSOut POut(&SOut);
		v8::Local<v8::Object> JsFOut = TNodeJsUtil::NewInstance<TNodeJsFOut>(new TNodeJsFOut(POut));
		
		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, SaveFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { JsFOut };
		
		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered: " + TStr(*Msg));
		}
	}
}

void TNodeJsFuncStreamAggr::LoadState(TSIn& SIn) {
	if (LoadFun.IsEmpty()) {
		throw TQm::TQmExcept::New("TNodeJsFuncStreamAggr::LoadState (called using sa.load) : stream aggregate does not implement a load callback: " + GetAggrNm());
	} else {
		// create TNodeJsFOut and pass it to callback
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		PSIn PIn(&SIn);
		v8::Local<v8::Object> JsFIn = TNodeJsUtil::NewInstance<TNodeJsFIn>(new TNodeJsFIn(PIn));

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, LoadFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 1;
		v8::Local<v8::Value> ArgV[Argc] = { JsFIn };
		v8::TryCatch TryCatch;
		Callback->Call(GlobalContext, Argc, ArgV);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered: " + TStr(*Msg));
		}		
	}
}

// IInt
int TNodeJsFuncStreamAggr::GetInt() const {
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
		QmAssertR(RetVal->IsInt32(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getInt(): Return type expected to be int32");
		return RetVal->Int32Value();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getInt() callback is empty!");
	}
}
// IFlt
double TNodeJsFuncStreamAggr::GetFlt() const {
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
		QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getFlt(): Return type expected to be int32");
		return RetVal->NumberValue();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getFlt() callback is empty!");
	}
}
// ITm
uint64 TNodeJsFuncStreamAggr::GetTmMSecs() const {
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
		QmAssertR(RetVal->IsNumber(), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getTm(): Return type expected to be number");
		return (uint64)RetVal->NumberValue();
	}
	else {
		throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", getTm() callback is empty!");
	}
}
// IFltTmIO
TFlt TNodeJsFuncStreamAggr::GetInVal() const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInVal not implemented");
}
uint64 TNodeJsFuncStreamAggr::GetInTmMSecs() const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInTmMSecs not implemented");
}

bool TNodeJsFuncStreamAggr::DelayedP() const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", DelayedP not implemented");
}

void TNodeJsFuncStreamAggr::GetInValV(TFltV& ValV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInValV not implemented");
}
void TNodeJsFuncStreamAggr::GetInTmMSecsV(TUInt64V& MSecsV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetInTmMSecsV not implemented");
}
void TNodeJsFuncStreamAggr::GetOutValV(TFltV& ValV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetOutValV not implemented");
}
void TNodeJsFuncStreamAggr::GetOutTmMSecsV(TUInt64V& MSecsV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetOutTmMSecsV not implemented");
}
int TNodeJsFuncStreamAggr::GetN() const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetN not implemented");
}

// IFltVec
int TNodeJsFuncStreamAggr::GetVals() const {
	// here be carefull when writing implementation
	// this method can be called via IFltVec or via ISparseVec
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetVals not implemented");
}
void TNodeJsFuncStreamAggr::GetVal(const TInt& ElN, TFlt& Val) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetVal not implemented");
} // GetFltAtFun

void TNodeJsFuncStreamAggr::GetValV(TFltV& ValV) const {
	if (!GetFltVFun.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, GetFltVFun);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		v8::TryCatch TryCatch;
		v8::Handle<v8::Value> RetVal = Callback->Call(GlobalContext, 0, NULL);
		if (TryCatch.HasCaught()) {
			v8::String::Utf8Value Msg(TryCatch.Message()->Get());
			throw TQm::TQmExcept::New("Javascript exception from callback triggered in TNodeJsFuncStreamAggr, name: " + GetAggrNm() + "," + TStr(*Msg));
		}
		QmAssertR(RetVal->IsObject() && TNodeJsUtil::IsClass(v8::Handle<v8::Object>::Cast(RetVal), TNodeJsFltV::GetClassId()), "TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ",GetFltV did not return a vector!");
		TNodeJsFltV* JsVec = TNodeJsUtil::Unwrap<TNodeJsFltV>(v8::Handle<v8::Object>::Cast(RetVal));

		ValV = JsVec->Vec;

	} else {
		throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetValV() callback is empty!");
	}
}
// ITmVec
int TNodeJsFuncStreamAggr::GetTmLen() const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTmLen not implemented");
}
uint64 TNodeJsFuncStreamAggr::GetTm(const TInt& ElN) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTm not implemented");
} // GetTmAtFun
void TNodeJsFuncStreamAggr::GetTmV(TUInt64V& TmMSecsV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetTmV not implemented");
}
// INmFlt
bool TNodeJsFuncStreamAggr::IsNmFlt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", IsNmFlt not implemented");
}
double TNodeJsFuncStreamAggr::GetNmFlt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetNmFlt not implemented");
}
void TNodeJsFuncStreamAggr::GetNmFltV(TStrFltPrV& NmFltV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetNmFltV not implemented");
}
// INmInt
bool TNodeJsFuncStreamAggr::IsNm(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", IsNm not implemented");
}
double TNodeJsFuncStreamAggr::GetNmInt(const TStr& Nm) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetNmInt not implemented");
}
void TNodeJsFuncStreamAggr::GetNmIntV(TStrIntPrV& NmIntV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetNmIntV not implemented");
}
// ISparseVec
//const TIntFltKdV& TNodeJsFuncStreamAggr::GetSparseVec() const {
//	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetSparseVec not implemented");
//}

void TNodeJsFuncStreamAggr::GetVal(const TInt& ElN, TIntFltKd& Val) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetVal not implemented");
}
void TNodeJsFuncStreamAggr::GetValV(TIntFltKdV& ValV) const {
	throw  TQm::TQmExcept::New("TNodeJsFuncStreamAggr, name: " + GetAggrNm() + ", GetValV not implemented");
}

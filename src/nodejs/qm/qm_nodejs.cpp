#include "qm_nodejs.h"
#include "qm_param.h"
#include "../la/la_nodejs.h"

///////////////////////////////
// NodeJs QMiner

THash<TStr, TUInt> TNodeJsQm::BaseFPathToId;

void TNodeJsQm::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	NODE_SET_METHOD(exports, "test", _test);

	// Add all methods, getters and setters here.
	NODE_SET_METHOD(exports, "config", _config);
	NODE_SET_METHOD(exports, "create", _create);
	NODE_SET_METHOD(exports, "open", _open);
	
	TQm::TEnv::Init();		
}

void TNodeJsQm::config(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);	
	// get schema and conf
	TStr ConfFNm = TNodeJsUtil::GetArgStr(Args, 0, "qm.conf");
	bool OverWriteP = TNodeJsUtil::GetArgBool(Args, 1, false);
	int PortN = TNodeJsUtil::GetArgInt32(Args, 2, 8080);
	int CacheSizeMB = TNodeJsUtil::GetArgInt32(Args, 3, 1024);
	// check so we don't overwrite any existing configuration file
	QmAssertR(!(TFile::Exists(ConfFNm) && !OverWriteP),
        "Configuration file already exists (" + ConfFNm + "). Use overwrite!");
	
	// create configuration file
	PJsonVal ConfigVal = TJsonVal::NewObj();
	ConfigVal->AddToObj("port", PortN);
	PJsonVal CacheVal = TJsonVal::NewObj();
	CacheVal->AddToObj("index", CacheSizeMB);
	CacheVal->AddToObj("store", CacheSizeMB);
	ConfigVal->AddToObj("cache", CacheVal);
	// save configuration file
	ConfigVal->SaveStr().SaveTxt(ConfFNm);
	// make folders if needed
	if (!TFile::Exists("db")) { TDir::GenDir("db"); }
	if (!TFile::Exists("src")) { TDir::GenDir("src"); }
	if (!TFile::Exists("src/lib")) { TDir::GenDir("src/lib"); }
	//if (!TFile::Exists("sandbox")) { TDir::GenDir("sandbox"); }
}

void TNodeJsQm::create(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get schema and conf
	TStr ConfFNm = TNodeJsUtil::GetArgStr(Args, 0, "qm.conf");
	TStr SchemaFNm = TNodeJsUtil::GetArgStr(Args, 1, "");
	

	// parse configuration file
	TQmParam Param(ConfFNm);
	// prepare lock
	TFileLock Lock(Param.LockFNm);

	Lock.Lock();
	{
		// parse schema (if no given, create an empty array)
		PJsonVal SchemaVal = SchemaFNm.Empty() ? TJsonVal::NewArr() :
			TJsonVal::GetValFromStr(TStr::LoadTxt(SchemaFNm));
		// initialize base		
		TQm::PBase Base_ = TQm::TStorage::NewBase(Param.DbFPath, SchemaVal, 16, 16);
		// save base		
		TQm::TStorage::SaveBase(Base_);
		Args.GetReturnValue().Set(TNodeJsBase::New(Base_));
		// once the base is open we need to setup the custom record templates for each store
		if (!TNodeJsQm::BaseFPathToId.IsKey(Base_->GetFPath())) {
			TUInt Keys = (uint)TNodeJsQm::BaseFPathToId.Len();
			TNodeJsQm::BaseFPathToId.AddDat(Base_->GetFPath(), Keys);
		}
		for (int StoreN = 0; StoreN < Base_->GetStores(); StoreN++) {
			TNodeJsRec::Init(Base_->GetStoreByStoreN(StoreN));
		}
	}
	// remove lock
	Lock.Unlock();
}

void TNodeJsQm::open(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get conf
	TStr ConfFNm = TNodeJsUtil::GetArgStr(Args, 0, "qm.conf");
	TBool RdOnlyP = TNodeJsUtil::GetArgBool(Args, 1, false);

	// parse configuration file
	TQmParam Param(ConfFNm);
	// prepare lock
	TFileLock Lock(Param.LockFNm);


	Lock.Lock();
	// load database and start the server
	{
		// resolve access type
		TFAccess FAccess = RdOnlyP ? faRdOnly : faUpdate;
		// load base
		TQm::PBase Base_ = TQm::TStorage::LoadBase(Param.DbFPath, FAccess,
			Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);
		Args.GetReturnValue().Set(TNodeJsBase::New(Base_));
		// once the base is open we need to setup the custom record templates for each store
		if (!TNodeJsQm::BaseFPathToId.IsKey(Base_->GetFPath())) {
			TUInt Keys = (uint)TNodeJsQm::BaseFPathToId.Len();
			TNodeJsQm::BaseFPathToId.AddDat(Base_->GetFPath(), Keys);
		}
		for (int StoreN = 0; StoreN < Base_->GetStores(); StoreN++) {			
			TNodeJsRec::Init(Base_->GetStoreByStoreN(StoreN));
		}
	}
	// remove lock
	Lock.Unlock();
}

void TNodeJsQm::test(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    
    PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
    
    printf("Json: %s\n", TJsonVal::GetStrFromVal(Json).CStr());
    v8::Handle<v8::Value> JsonObj = TNodeJsUtil::ParseJson(Isolate, Json);
    printf("Done Json\n");
    Args.GetReturnValue().Set(JsonObj->ToObject());
    
}

///////////////////////////////
// NodeJs QMiner Base

v8::Persistent<v8::Function> TNodeJsBase::constructor;

void TNodeJsBase::Init(v8::Handle<v8::Object> exports) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);

   v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
   tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "base"));
   // ObjectWrap uses the first internal field to store the wrapped pointer.
   tpl->InstanceTemplate()->SetInternalFieldCount(1);

   // Add all methods, getters and setters here.   
   NODE_SET_PROTOTYPE_METHOD(tpl, "close", _close);
   NODE_SET_PROTOTYPE_METHOD(tpl, "store", _store);
   NODE_SET_PROTOTYPE_METHOD(tpl, "getStoreList", _getStoreList);
   NODE_SET_PROTOTYPE_METHOD(tpl, "createStore", _createStore);
   NODE_SET_PROTOTYPE_METHOD(tpl, "search", _search);
   NODE_SET_PROTOTYPE_METHOD(tpl, "gc", _gc);
   NODE_SET_PROTOTYPE_METHOD(tpl, "newStreamAggr", _newStreamAggr);
   NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", _getStreamAggr);
   NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);
   
   // This has to be last, otherwise the properties won't show up on the object in JavaScript.
   constructor.Reset(Isolate, tpl->GetFunction());
   /*exports->Set(v8::String::NewFromUtf8(Isolate, "base"),
	   tpl->GetFunction());*/

}

v8::Local<v8::Object> TNodeJsBase::New(TQm::PBase _Base) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TBase");
	Instance->SetHiddenValue(Key, Value);

	TNodeJsBase* JsBase = new TNodeJsBase(_Base);
	JsBase->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsBase::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);	
	// new base(...)
	if (Args.IsConstructCall()) {
	    // new base(fPath, cacheSize);
		if (Args.Length() == 2) {
			//printf("construct call, 2 args\n");
			TStr FPath = TNodeJsUtil::GetArgStr(Args, 0);
			int CacheSize = TNodeJsUtil::GetArgInt32(Args, 1);
			TQm::PBase Base_ = TQm::TBase::New(FPath, CacheSize);
			Args.GetReturnValue().Set(TNodeJsBase::New(Base_));
			return;
		}
		else {
			//printf("construct call, 0 args expected, got %d\n", Args.Length());
			// new base(base);
			TNodeJsBase* JsBase = new TNodeJsBase();
			v8::Local<v8::Object> Instance = Args.This();
			JsBase->Wrap(Instance);
			Args.GetReturnValue().Set(Instance);
			return;
		}
	}
	// base(...) -> calls new base()
	else {	
		//printf("NOT construct call from New!\n");
		// base(fPath, cacheSize) -> calls new base(fPath, cacheSize)
		if (Args.Length() == 2) {
			const int Argc = 2;
			v8::Local<v8::Value> Argv[Argc] = { Args[0], Args[1] };
			v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
			v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
			Args.GetReturnValue().Set(Instance);
			return;
		}
		else {
			// base()->calls new base()
			v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
			v8::Local<v8::Object> Instance = cons->NewInstance();
			Args.GetReturnValue().Set(Instance);
			return;
		}
	}	
}

void TNodeJsBase::close(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
	if (!JsBase->Base.Empty()) {
		// save base
		TQm::TStorage::SaveBase(JsBase->Base);
		JsBase->Base.Clr();
	}
}

void TNodeJsBase::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   // unwrap
   TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
   TWPt<TQm::TBase> Base = JsBase->Base;

   const TStr StoreNm = TNodeJsUtil::GetArgStr(Args, 0);
   if (Base->IsStoreNm(StoreNm)) {
	   Args.GetReturnValue().Set(TNodeJsStore::New(Base->GetStoreByStoreNm(StoreNm)));
	   return;
   }
   else {
	   Args.GetReturnValue().Set(v8::Null(Isolate));
	   return;
   }   
}

void TNodeJsBase::getStoreList(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	TJsonValV StoreValV;
	const int Stores = Base->GetStores();
	for (int StoreN = 0; StoreN < Stores; StoreN++) {
		TWPt<TQm::TStore> Store = Base->GetStoreByStoreN(StoreN);
		StoreValV.Add(Base->GetStoreJson(Store));
	}
	PJsonVal JsonVal = TJsonVal::NewArr(StoreValV);
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsonVal));
}

void TNodeJsBase::createStore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   // unwrap
   TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
   TWPt<TQm::TBase> Base = JsBase->Base;
   QmAssertR(!Base->IsRdOnly(), "Base opened as read-only");
   // parse arguments
   PJsonVal SchemaVal = TNodeJsUtil::GetArgJson(Args, 0);
   uint64 DefStoreSize = (uint64)TNodeJsUtil::GetArgInt32(Args, 1, 1024);
   DefStoreSize = DefStoreSize * TInt::Mega;
   // create new stores
   TVec<TWPt<TQm::TStore> > NewStoreV = TQm::TStorage::CreateStoresFromSchema(
	   Base, SchemaVal, DefStoreSize);
   // Update record templates
   for (int StoreN = 0; StoreN < NewStoreV.Len(); StoreN++) {
	   TNodeJsRec::Init(NewStoreV[StoreN]);
   }
   // return store (if only one) or array of stores (if more)
   if (NewStoreV.Len() == 1) {
	   Args.GetReturnValue().Set(TNodeJsStore::New(NewStoreV[0]));
   }
   else if (NewStoreV.Len() > 1) {
	   v8::Local<v8::Array> JsNewStoreV = v8::Array::New(Isolate, NewStoreV.Len());
	   for (int NewStoreN = 0; NewStoreN < NewStoreV.Len(); NewStoreN++) {
		   JsNewStoreV->Set(v8::Number::New(Isolate, NewStoreN),
			   TNodeJsStore::New(NewStoreV[NewStoreN]));
	   }
   }
   Args.GetReturnValue().Set(v8::Null(Isolate));
}

void TNodeJsBase::search(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   // unwrap
   TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
   TWPt<TQm::TBase> Base = JsBase->Base;
   try {
	   TStr QueryStr = "";// TNodeJsUtil::GetArgJsonStr(Args, 0);
	   // execute the query
	   TQm::PRecSet RecSet = JsBase->Base->Search(QueryStr);
	   // return results
	   Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet));
	   return;
   }
   catch (const PExcept& Except) {
	   TQm::InfoLog("[except] " + Except->GetMsgStr());
   }
   Args.GetReturnValue().Set(v8::Null(Isolate));
}

void TNodeJsBase::gc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   // unwrap
   TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
   TWPt<TQm::TBase> Base = JsBase->Base;

   Base->GarbageCollect();   
   Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsBase::newStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   // TODO
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   // TODO
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   // unwrap
   TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());
   TWPt<TQm::TBase> Base = JsBase->Base;

   TQm::PStreamAggrBase SABase = JsBase->Base->GetStreamAggrBase();
   int AggrId = SABase->GetFirstStreamAggrId();
   v8::Local<v8::Array> Arr = v8::Array::New(Isolate);
   uint32 Counter = 0;
   while (SABase->GetNextStreamAggrId(AggrId)) {
	   v8::Local<v8::String> AggrNm = v8::String::NewFromUtf8(Isolate, SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
	   Arr->Set(Counter, AggrNm);
	   Counter++;
   }
   Args.GetReturnValue().Set(Arr);
}


///////////////////////////////
// NodeJs Stream Aggregator

v8::Persistent<v8::Function> TNodeJsSA::constructor;

void TNodeJsSA::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "sa"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.   
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAdd", _onAdd);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onUpdate", _onUpdate);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onDelete", _onDelete);
	NODE_SET_PROTOTYPE_METHOD(tpl, "saveJson", _saveJson);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInt", _getInt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFlt", _getFlt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTm", _getTm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFltLen", _getFltLen);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFltAt", _getFltAt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFltV", _getFltV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTmLen", _getTmLen);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTmAt", _getTmAt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTmV", _getTmV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInFlt", _getInFlt);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getInTm", _getInTm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutFltV", _getOutFltV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getOutTmV", _getOutTmV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getN", _getN);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "val"), _val);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "sa"),
	tpl->GetFunction());

}

v8::Local<v8::Object> TNodeJsSA::New(TWPt<TQm::TStreamAggr> _SA) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();
	
	TNodeJsSA* JsSA = new TNodeJsSA(_SA);
	JsSA->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsSA::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() <= 3 && Args.Length() >= 2, "stream aggregator constructor expects at least two parameters");
	QmAssertR(Args[0]->IsObject() && Args[1]->IsObject(), "stream aggregator constructor expects first two arguments as objects");

	// new sa(...)
	if (Args.IsConstructCall()) {
		TQm::PStreamAggr StreamAggr;

		TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args[0]->ToObject());

		// parse out parameters
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 1);
		// get aggregate type
		TStr TypeNm = TNodeJsUtil::GetArgStr(Args, 1, "type", "javaScript");

		if (TypeNm == "javaScript") {
			// we have a javascript stream aggregate
			TStr AggrName = TNodeJsUtil::GetArgStr(Args, 1, "name", "");
			// we need a name, if not give just generate one
			if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
			// create aggregate
			// TODO implement!
			throw TQm::TQmExcept::New("JS stream aggr not implemented yet!");
			//StreamAggr = TNodeJsStreamAggr::New(
			//	JsBase->Js, AggrName, Args[1]->ToObject());
		}
		else if (TypeNm == "ftrext") {
			TStr AggrName = TNodeJsUtil::GetArgStr(Args, 1, "name", "");
			QmAssertR(Args[1]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, "featureSpace")), "addStreamAggr: featureSpace property missing!");
			// we need a name, if not give just generate one
			if (AggrName.Empty()) { AggrName = TGuid::GenSafeGuid(); }
			throw TQm::TQmExcept::New("ftrext stream aggr not implemented yet! (needs feature space implementation)");
			// TODO
			//TQm::PFtrSpace FtrSpace = TJsFtrSpace::GetArgFtrSpace(Args[1]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, "featureSpace")));
			//StreamAggr = TStreamAggrs::TFtrExtAggr::New(JsBase->Base, AggrName, FtrSpace);
		}
		else if (TypeNm == "stmerger") {
			// create new aggregate
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
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 1);
			if (Args.Length() >= 3 && Args[2]->IsString()) {
				ParamVal->AddToObj("store", TNodeJsUtil::GetArgStr(Args, 2));
			}

			// check if it's one stream aggregate or composition
			if (TQm::TStreamAggrs::TCompositional::IsCompositional(TypeNm)) {
				// we have a composition of aggregates, call code to assemble it
				TQm::TStreamAggrs::TCompositional::Register(JsBase->Base, TypeNm, ParamVal);
			}
			else {
				// create new aggregate
				StreamAggr = TQm::TStreamAggr::New(JsBase->Base, TypeNm, ParamVal);
			}
		}

		if (!TQm::TStreamAggrs::TCompositional::IsCompositional(TypeNm)) {
			if (Args.Length() > 2) {
				TStrV Stores(0);
				if (Args[2]->IsString()) {
					Stores.Add(TNodeJsUtil::GetArgStr(Args, 2));
				}
				if (Args[2]->IsArray()) {
					PJsonVal StoresJson = TNodeJsUtil::GetArgJson(Args, 2);
					QmAssertR(StoresJson->IsDef(), "stream aggr constructor : Args[2] should be a string (store name) or a string array (store names)");
					StoresJson->GetArrStrV(Stores);
				}
				for (int StoreN = 0; StoreN < Stores.Len(); StoreN++) {
					QmAssertR(JsBase->Base->IsStoreNm(Stores[StoreN]), "stream aggr constructor : Args[2] : store does not exist!");
					JsBase->Base->AddStreamAggr(Stores[StoreN], StreamAggr);
				}
			}
			else {
				JsBase->Base->AddStreamAggr(StreamAggr);
			}
			// non-compositional aggregates are returned
			TNodeJsSA* JsSA = new TNodeJsSA(StreamAggr);
			v8::Local<v8::Object> Instance = Args.This();
			JsSA->Wrap(Instance);
			Args.GetReturnValue().Set(Instance);
			return;
		}
		
	}
	// sa(...) -> calls new sa(...)
	else {
		if (Args.Length() == 2) {
			const int Argc = 2;
			v8::Local<v8::Value> Argv[Argc] = { Args[0], Args[1]};
			v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
			v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
			Args.GetReturnValue().Set(Instance);
			return;
		}		
		if (Args.Length() == 3) {
			const int Argc = 3;
			v8::Local<v8::Value> Argv[Argc] = { Args[0], Args[1], Args[2] };
			v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
			v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
			Args.GetReturnValue().Set(Instance);
			return;
		}
		else {
			// sa()->calls new sa()
			v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
			v8::Local<v8::Object> Instance = cons->NewInstance();
			Args.GetReturnValue().Set(Instance);
			return;
		}
	}
}

void TNodeJsSA::onAdd(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onAdd should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnAddRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::onUpdate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onUpdate should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args[0]->ToObject());
	JsSA->SA->OnUpdateRec(JsRec->Rec);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::onDelete(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "sa.onDelete should take one argument of type TNodeJsRec");
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args[0]->ToObject());
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
	TWPt<TQm::TStreamAggr> SA = JsSA->SA;
	// TODO
	throw TQm::TQmExcept::New("sa.save not implemented yet!");
	//PSOut SOut = TJsFOut::GetArgFOut(Args, 0);
	//JsSA->SA->_Save(*SOut);
	//if (JsSA->SA->Type() == "javaScript") {
	//	auto SA = dynamic_cast<TJsStreamAggr*>(JsSA->SA());
	//	JsSA->Js->Execute(SA->SaveFun, Args[0]);

	//}
	//return HandleScope.Close(Args[0]);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	TWPt<TQm::TStreamAggr> SA = JsSA->SA;
	throw TQm::TQmExcept::New("sa.load not implemented yet!");

	//PSIn SIn = TJsFIn::GetArgFIn(Args, 0);
	//JsSA->SA->_Load(*SIn);
	//if (JsSA->SA->Type() == "javaScript") {
	//	auto SA = dynamic_cast<TJsStreamAggr*>(JsSA->SA());
	//	JsSA->Js->Execute(SA->LoadFun, Args[0]);
	//}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSA::getInt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

void TNodeJsSA::getFlt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

void TNodeJsSA::getTm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

void TNodeJsSA::getFltLen(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getFltAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getFltV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getTmLen(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getTmAt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

	Args.GetReturnValue().Set(v8::Number::New(Isolate, (double)Aggr->GetTm(ElN)));
}
void TNodeJsSA::getTmV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getInFlt(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getInTm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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
void TNodeJsSA::getOutFltV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

void TNodeJsSA::getOutTmV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
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

void TNodeJsSA::getN(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsSA* JsSA = ObjectWrap::Unwrap<TNodeJsSA>(Args.Holder());
	// try to cast as IFltTmIO
	TWPt<TQm::TStreamAggrOut::IFltTmIO> Aggr = dynamic_cast<TQm::TStreamAggrOut::IFltTmIO*>(JsSA->SA());
	if (Aggr.Empty()) {
		throw TQm::TQmExcept::New("TJsSA::getN : stream aggregate does not implement IFltTmIO: " + JsSA->SA->GetAggrNm());
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

///////////////////////////////
// NodeJs QMiner Record
v8::Persistent<v8::Function> TNodeJsStore::constructor;

void TNodeJsStore::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "store"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "rec", _rec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "each", _each);
	NODE_SET_PROTOTYPE_METHOD(tpl, "map", _map);
	NODE_SET_PROTOTYPE_METHOD(tpl, "add", _add);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRec", _newRec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRecSet", _newRecSet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sample", _sample);
	NODE_SET_PROTOTYPE_METHOD(tpl, "field", _field);
	NODE_SET_PROTOTYPE_METHOD(tpl, "key", _key);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addTrigger", _addTrigger);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", _getStreamAggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clear", _clear);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getVec", _getVec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMat", _getMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "cell", _cell);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "empty"), _empty);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "recs"), _recs);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "fields"), _fields);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "joins"), _joins);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "keys"), _keys);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "first"), _first);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "last"), _last);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "forwardIter"), _forwardIter);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "backwardIter"), _backwardIter);
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	/*exports->Set(v8::String::NewFromUtf8(Isolate, "store"),
		tpl->GetFunction());*/
}

v8::Local<v8::Object> TNodeJsStore::New(TWPt<TQm::TStore> _Store) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsStore* JsStore = new TNodeJsStore(_Store, nullptr);	// TODO
	JsStore->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsStore::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		if (Args.Length() == 2) {
			QmAssertR(Args[0]->IsString() && Args[1]->IsObject() && TNodeJsUtil::IsClass(Args[1]->ToObject(), "TBase"), "TNodeJsStore constructor expecting store name and base object as arguments");
			TStr StoreNm = TNodeJsUtil::GetArgStr(Args, 0);
			TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args[1]->ToObject());

			if (JsBase->Base->IsStoreNm(StoreNm)) {
				TWPt<TQm::TStore> Store = JsBase->Base->GetStoreByStoreNm(StoreNm);
				Args.GetReturnValue().Set(TNodeJsStore::New(Store));
				return;
			}
			else {
				Args.GetReturnValue().Set(v8::Undefined(Isolate));
				return;
			}
		}
		else {
			TNodeJsStore* JsStore = new TNodeJsStore();
			v8::Local<v8::Object> Instance = Args.This();
			JsStore->Wrap(Instance);
			Args.GetReturnValue().Set(Instance);
			return;
		}
	}
	else {
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

v8::Local<v8::Value> TNodeJsStore::Field(const TQm::TRec& Rec, const int FieldId) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	
	// not null, get value
	const TQm::TFieldDesc& Desc = Rec.GetStore()->GetFieldDesc(FieldId);
	if (Desc.IsInt()) {
		const int Val = Rec.GetFieldInt(FieldId);
		return HandleScope.Escape(v8::Integer::New(Isolate, Val));
	}
	else if (Desc.IsIntV()) {
		TIntV IntV; Rec.GetFieldIntV(FieldId, IntV);
		return HandleScope.Escape(TNodeJsVec<TInt, TAuxIntV>::New(IntV));
	}
	else if (Desc.IsUInt64()) {
		const uint64 Val = Rec.GetFieldUInt64(FieldId);
		return HandleScope.Escape(v8::Integer::New(Isolate, (int)Val));
	}
	else if (Desc.IsStr()) {
		const TStr Val = Rec.GetFieldStr(FieldId);
		return HandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
	}
	else if (Desc.IsStrV()) {
		TStrV StrV; Rec.GetFieldStrV(FieldId, StrV);
		return HandleScope.Escape(TNodeJsVec<TStr, TAuxStrV>::New(StrV));
	}
	else if (Desc.IsBool()) {
		const bool Val = Rec.GetFieldBool(FieldId);
		return v8::Boolean::New(Isolate, Val);
	}
	else if (Desc.IsFlt()) {
		const double Val = Rec.GetFieldFlt(FieldId);
		return HandleScope.Escape(v8::Number::New(Isolate, Val));
	}
	else if (Desc.IsFltPr()) {
		const TFltPr FltPr = Rec.GetFieldFltPr(FieldId);
		v8::Local<v8::Array> JsFltPr = v8::Array::New(Isolate, 2);
		JsFltPr->Set(0, v8::Number::New(Isolate, FltPr.Val1));
		JsFltPr->Set(1, v8::Number::New(Isolate, FltPr.Val2));
		return HandleScope.Escape(JsFltPr);
	}
	else if (Desc.IsFltV()) {
		TFltV FltV; Rec.GetFieldFltV(FieldId, FltV);
		return HandleScope.Escape(TNodeJsVec<TFlt, TAuxFltV>::New(FltV));
	}
	else if (Desc.IsTm()) {
		TTm FieldTm; Rec.GetFieldTm(FieldId, FieldTm);
		if (FieldTm.IsDef()) {
			throw TQm::TQmExcept::New("TODO: implement TJsTm and the method Store::Field");
			//return TJsTm::New(FieldTm);
		}
		else {
			return v8::Null(Isolate);
		}
	}
	else if (Desc.IsNumSpV()) {
		TIntFltKdV SpV; Rec.GetFieldNumSpV(FieldId, SpV);
		return HandleScope.Escape(TNodeJsSpVec::New(SpV));
	}
	else if (Desc.IsBowSpV()) {
		throw TQm::TQmExcept::New("Store::Field BowSpV not implemented");
	}
	throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

v8::Local<v8::Value> TNodeJsStore::Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	// check if null
	if (!Store->IsRecId(RecId)) {
		return v8::Null(Isolate);
	}
	// check if field is null
	if (Store->IsFieldNull(RecId, FieldId)) {
		return v8::Null(Isolate);
	}
	TQm::TRec& Rec = Store->GetRec(RecId);
	return HandleScope.Escape(Field(Rec, FieldId));
}


void TNodeJsStore::rec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
		if (JsStore->Store->IsRecNm(RecNm)) {
			Args.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(RecNm)));
		}
		else {
			Args.GetReturnValue().Set(v8::Undefined(Isolate));
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::each(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");
		Args.GetReturnValue().Set(Args.Holder());

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		const TWPt<TQm::TStore> Store = JsStore->Store;

		if (!Store->Empty()) {
			TQm::PStoreIter Iter = Store->ForwardIter();

			QmAssert(Iter->Next());
			uint64 Count = 0;
			uint64 RecId = Iter->GetRecId();
			const unsigned Argc = 2;

			v8::Local<v8::Object> RecObj = TNodeJsRec::New(Store->GetRec(RecId));
			TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(RecObj);

			do {
				JsRec->Rec = Store->GetRec(Iter->GetRecId());
				v8::Local<v8::Value> ArgV[Argc] = {
					RecObj,
					v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
				};
				Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
			} while (Iter->Next());
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		const TWPt<TQm::TStore> Store = JsStore->Store;
		const uint64 Recs = Store->GetRecs();

		v8::Handle<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

		if (!Store->Empty()) {
			v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

			TQm::PStoreIter Iter = Store->ForwardIter();

			QmAssert(Iter->Next());
			uint64 Count = 0;
			uint64 RecId = Iter->GetRecId();
			const unsigned Argc = 2;

			v8::Local<v8::Object> RecObj = TNodeJsRec::New(Store->GetRec(RecId));
			TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(RecObj);

			do {
				JsRec->Rec = Store->GetRec(Iter->GetRecId());
				v8::Local<v8::Value> ArgV[Argc] = {
					RecObj,
					v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count))
				};
				v8::Local<v8::Value> ReturnVal = Callback->Call(GlobalContext, Argc, ArgV);
				ResultV->Set(Count, ReturnVal);
				Count++;
			} while (Iter->Next());
		}

		Args.GetReturnValue().Set(ResultV);
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::add(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;
		TWPt<TQm::TBase> Base = JsStore->Store->GetBase();

		// check we can write
		QmAssertR(!Base->IsRdOnly(), "Base opened as read-only");

		PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
		const uint64 RecId = Store->AddRec(RecVal);

		Args.GetReturnValue().Set(v8::Integer::NewFromUnsigned(Isolate, RecId));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::newRec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
		printf("%s\n", TJsonVal::GetStrFromVal(RecVal).CStr());
		TQm::TRec Rec(Store, RecVal);
		printf("%s\n", TJsonVal::GetStrFromVal(Rec.GetJson(Store->GetBase())).CStr());
		
		Args.GetReturnValue().Set(TNodeJsRec::New(Rec));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::newRecSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
	TWPt<TQm::TStore> Store = JsStore->Store;

	if (Args.Length() > 0) {
		// argument 0 = TJsIntV of record ids
		QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, "TIntV"),
			"Store.getRecSetByIdV: The first argument must be a TIntV (js linalg full int vector)");
		TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject()); 
		TQm::PRecSet ResultSet = TQm::TRecSet::New(Store, JsVecArg->Vec);
		Args.GetReturnValue().Set(TNodeJsRecSet::New(ResultSet));
		return;
	}
	Args.GetReturnValue().Set(TNodeJsRecSet::New());	
}

void TNodeJsStore::sample(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		Args.GetReturnValue().Set(TNodeJsRecSet::New(JsStore->Store->GetRndRecs(SampleSize)));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::field(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		if (JsStore->Store->IsFieldNm(FieldNm)) {
			const int FieldId = JsStore->Store->GetFieldId(FieldNm);
			const TQm::TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);

			v8::Local<v8::Object> Field = v8::Object::New(Isolate);
			Field->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "name"), v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldNm().CStr()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "type"), v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldTypeStr().CStr()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "nullable"), v8::Boolean::New(Isolate, FieldDesc.IsNullable()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "internal"), v8::Boolean::New(Isolate, FieldDesc.IsInternal()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "primary"), v8::Boolean::New(Isolate, FieldDesc.IsPrimary()));

			Args.GetReturnValue().Set(Field);
		}
		else {
			Args.GetReturnValue().Set(v8::Null(Isolate));
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::key(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr KeyNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();

		if (IndexVoc->IsKeyNm(JsStore->Store->GetStoreId(), KeyNm)) {
			TQm::TIndexKey Key = IndexVoc->GetKey(IndexVoc->GetKeyId(JsStore->Store->GetStoreId(), KeyNm));
			Args.GetReturnValue().Set(TNodeJsIndexKey::New(JsStore->Store, Key));
		}
		else {
			Args.GetReturnValue().Set(v8::Null(Isolate));
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::addTrigger(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssert(Args.Length() == 1);
	v8::Handle<v8::Value> TriggerVal = Args[0];
	QmAssert(TriggerVal->IsObject());

	try {
		///TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		//TWPt<TQm::TStore>& Store = JsStore->Store;
		// TODO
		//TQm::PStoreTrigger Trigger = TJsStoreTrigger::New(TriggerVal->ToObject());

		//Store->AddTrigger(Trigger);
		//JsStore->Js->TriggerV.Add(TPair<TUInt, PStoreTrigger>(JsStore->Store->GetStoreId(), Trigger));

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr AggrNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		TWPt<TQm::TBase> Base = JsStore->Store->GetBase();
		const uint StoreId = JsStore->Store->GetStoreId();

		if (Base->IsStreamAggr(StoreId, AggrNm)) {
			TQm::PStreamAggr StreamAggr = Base->GetStreamAggr(StoreId, AggrNm);
			//	TODO
			// Args.GetReturnValue().Set(TJsSA::New(StreamAggr));
		}
		else {
			Args.GetReturnValue().Set(v8::Null(Isolate));
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		TWPt<TQm::TStore>& Store = JsStore->Store;
		const TWPt<TQm::TBase>& Base = JsStore->Store->GetBase();

		TQm::PStreamAggrBase SABase = Base->GetStreamAggrBase(Store->GetStoreId());
		int AggrId = SABase->GetFirstStreamAggrId();

		v8::Local<v8::Array> Arr = v8::Array::New(Isolate);
		uint32 Counter = 0;
		while (SABase->GetNextStreamAggrId(AggrId)) {
			v8::Local<v8::String> AggrNm = v8::String::NewFromUtf8(Isolate, SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
			Arr->Set(Counter, AggrNm);
			Counter++;
		}
		Args.GetReturnValue().Set(Arr);
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		PJsonVal StoreJson = JsStore->Store->GetStoreJson(JsStore->Store->GetBase());

		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, StoreJson));
		return;
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::clear(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		const int DelRecs = TNodeJsUtil::GetArgInt32(Args, 0, (int)JsStore->Store->GetRecs());

		JsStore->Store->DeleteFirstNRecs(DelRecs);
		Args.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
		return;
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		if (!Store->IsFieldNm(FieldNm)) {
			throw TQm::TQmExcept::New("store.getVec: fieldName not found: " + FieldNm);
		}

		const int FieldId = JsStore->Store->GetFieldId(FieldNm);

		int Recs = (int)JsStore->Store->GetRecs();
		const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

		if (Desc.IsInt()) {
			TIntV ColV(Recs);

			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV[RecN] = JsStore->Store->GetFieldInt(Iter->GetRecId(), FieldId);
				Iter->Next();
			}

			Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
			return;
		}
		else if (Desc.IsUInt64()) {
			TFltV ColV(Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV[RecN] = (double)JsStore->Store->GetFieldUInt64(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
			return;
		}

		else if (Desc.IsStr()) {
			TStrV ColV(Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = JsStore->Store->GetFieldStr(Iter->GetRecId(), FieldId);
                Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TStr, TAuxStrV>::New(ColV));
			return;
		}

		else if (Desc.IsBool()) {
			TIntV ColV(Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV[RecN] = (int)JsStore->Store->GetFieldBool(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
			return;
		}
		else if (Desc.IsFlt()) {
			TFltV ColV(Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV[RecN] = JsStore->Store->GetFieldFlt(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
			return;
		}
		else if (Desc.IsTm()) {
			TFltV ColV(Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			TTm Tm;
			for (int RecN = 0; RecN < Recs; RecN++) {
				Store->GetFieldTm(Iter->GetRecId(), FieldId, Tm);
				ColV[RecN] = (double)TTm::GetMSecsFromTm(Tm);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
			return;
		}
		else if (Desc.IsFltV()) {
			throw TQm::TQmExcept::New("store.getVec does not support type float_v - use store.getMat instead");
		}
		else if (Desc.IsNumSpV()) {
			throw TQm::TQmExcept::New("store.getVec does not support type num_sp_v - use store.getMat instead");
		}
		throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		if (!Store->IsFieldNm(FieldNm)) {
			throw TQm::TQmExcept::New("store.getMat: fieldName not found: " + FieldNm);
		}
		const int FieldId = JsStore->Store->GetFieldId(FieldNm);
		int Recs = (int)JsStore->Store->GetRecs();
		const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

		if (Desc.IsInt()) {
			TFltVV ColV(1, Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV.At(0, RecN) = (double)JsStore->Store->GetFieldInt(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsUInt64()) {
			TFltVV ColV(1, Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV.At(0, RecN) = (double)JsStore->Store->GetFieldUInt64(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsBool()) {
			TFltVV ColV(1, Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV.At(0, RecN) = (double)JsStore->Store->GetFieldBool(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsFlt()) {
			TFltVV ColV(1, Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				ColV.At(0, RecN) = JsStore->Store->GetFieldFlt(Iter->GetRecId(), FieldId);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsTm()) {
			TFltVV ColV(1, Recs);
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			TTm Tm;
			for (int RecN = 0; RecN < Recs; RecN++) {
				Store->GetFieldTm(Iter->GetRecId(), FieldId, Tm);
				ColV.At(0, RecN) = (double)TTm::GetMSecsFromTm(Tm);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsFltV()) {
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			TFltV Vec;
			JsStore->Store->GetFieldFltV(Iter->GetRecId(), FieldId, Vec);
			TFltVV ColV(Vec.Len(), Recs);
			for (int RecN = 0; RecN < Recs; RecN++) {
				JsStore->Store->GetFieldFltV(Iter->GetRecId(), FieldId, Vec);
				QmAssertR(Vec.Len() == ColV.GetRows(), TStr::Fmt("store.getCol for field type fltvec: dimensions are not consistent! %d expected, %d found in row %d", ColV.GetRows(), Vec.Len(), RecN));
				// copy row
				ColV.SetCol(RecN, Vec);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
			return;
		}
		else if (Desc.IsNumSpV()) {
			TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			TVec<TIntFltKdV> ColV(Recs);
			for (int RecN = 0; RecN < Recs; RecN++) {
				JsStore->Store->GetFieldNumSpV(Iter->GetRecId(), FieldId, ColV[RecN]);
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsSpMat::New(ColV));
			return;
		}
		else if (Desc.IsStr()) {
			throw TQm::TQmExcept::New("store.getMat does not support type string - use store.getVec instead");
		}
		throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::cell(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
	TWPt<TQm::TStore> Store = JsStore->Store;

	TInt RecId = TNodeJsUtil::GetArgInt32(Args, 0);
	TInt FieldId = -1;
	if (Args.Length() == 2 && Args[1]->IsInt32()) {
		FieldId = TNodeJsUtil::GetArgInt32(Args, 1);
	}
	else if (Args.Length() == 2 && Args[1]->IsString()) {
		TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 1);
		if (JsStore->Store->IsFieldNm(FieldNm)) {
			FieldId = JsStore->Store->GetFieldId(FieldNm);
		}
	}
	else {
		throw TQm::TQmExcept::New("store.cell: Args[1] expected to be an int (fieldId) or a string (fieldName)");
	}
	
	Args.GetReturnValue().Set(TNodeJsStore::Field(JsStore->Store, RecId, FieldId));
}

void TNodeJsStore::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsStore->Store->GetStoreNm().CStr()));
}

void TNodeJsStore::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStore->Store->Empty()));
}

void TNodeJsStore::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
}

void TNodeJsStore::recs(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	TQm::PRecSet ResultSet = JsStore->Store->GetAllRecs();
	Info.GetReturnValue().Set(TNodeJsRecSet::New(ResultSet));
}

void TNodeJsStore::fields(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	v8::Local<v8::Array> FieldV = v8::Array::New(Isolate, JsStore->Store->GetFields());
	for (int FieldId = 0; FieldId < JsStore->Store->GetFields(); FieldId++) {
		const TQm::TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);
		v8::Local<v8::Object> Field = v8::Object::New(Isolate);
		Field->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "name"), v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldNm().CStr()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "type"), v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldTypeStr().CStr()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "nullable"), v8::Boolean::New(Isolate, FieldDesc.IsNullable()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "internal"), v8::Boolean::New(Isolate, FieldDesc.IsInternal()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "primary"), v8::Boolean::New(Isolate, FieldDesc.IsPrimary()));
		FieldV->Set(v8::Number::New(Isolate, FieldId), Field);
	}
	Info.GetReturnValue().Set(FieldV);
}

void TNodeJsStore::joins(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	v8::Local<v8::Array> JoinV = v8::Array::New(Isolate,JsStore->Store->GetJoins());
	for (int JoinId = 0; JoinId < JsStore->Store->GetJoins(); JoinId++) {
		const TQm::TJoinDesc& JoinDesc = JsStore->Store->GetJoinDesc(JoinId);
		TWPt<TQm::TStore> JoinStore = JoinDesc.GetJoinStore(JsStore->Store->GetBase());
		v8::Local<v8::Object> Join = v8::Object::New(Isolate);
		Join->Set(v8::String::NewFromUtf8(Isolate,"id"),
            v8::Int32::New(Isolate, JoinDesc.GetJoinId()));
		Join->Set(v8::String::NewFromUtf8(Isolate,"name"),
            v8::String::NewFromUtf8(Isolate,JoinDesc.GetJoinNm().CStr()));
		Join->Set(v8::String::NewFromUtf8(Isolate,"store"),
            v8::String::NewFromUtf8(Isolate,JoinStore->GetStoreNm().CStr()));
		if (JoinDesc.IsInverseJoinId()) {
			Join->Set(v8::String::NewFromUtf8(Isolate,"inverse"),
                v8::String::NewFromUtf8(Isolate,JoinStore->GetJoinNm(JoinDesc.GetInverseJoinId()).CStr()));
		}
		if (JoinDesc.IsFieldJoin()) {
			Join->Set(v8::String::NewFromUtf8(Isolate,"type"),
                v8::String::NewFromUtf8(Isolate,"field"));
			Join->Set(v8::String::NewFromUtf8(Isolate,"recordField"),
                v8::String::NewFromUtf8(Isolate,JsStore->Store->GetFieldNm(JoinDesc.GetJoinRecFieldId()).CStr()));
			Join->Set(v8::String::NewFromUtf8(Isolate,"weightField"),
                v8::String::NewFromUtf8(Isolate,JsStore->Store->GetFieldNm(JoinDesc.GetJoinFqFieldId()).CStr()));
		}
		else if (JoinDesc.IsIndexJoin()) {
			Join->Set(v8::String::NewFromUtf8(Isolate,"type"),
                v8::String::NewFromUtf8(Isolate,"index"));
			TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
			Join->Set(v8::String::NewFromUtf8(Isolate,"key"),
                TNodeJsIndexKey::New(JsStore->Store, IndexVoc->GetKey(JoinDesc.GetJoinKeyId())));
		}
		JoinV->Set(v8::Number::New(Isolate, JoinId), Join);
	}
	Info.GetReturnValue().Set(JoinV);
}

void TNodeJsStore::keys(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
	TIntSet KeySet = IndexVoc->GetStoreKeys(JsStore->Store->GetStoreId());
	v8::Local<v8::Array> KeyNmV = v8::Array::New(Isolate, KeySet.Len());
	int KeySetId = KeySet.FFirstKeyId(), KeyN = 0;
	while (KeySet.FNextKeyId(KeySetId)){
		KeyNmV->Set(KeyN, TNodeJsIndexKey::New(JsStore->Store, IndexVoc->GetKey(KeyN)));
		KeyN++;
	}
	Info.GetReturnValue().Set(KeyNmV);
}

void TNodeJsStore::first(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	const uint64 FirstRecId = JsStore->Store->FirstRecId();
	if (FirstRecId == TUInt64::Mx) { 
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
	Info.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(FirstRecId)));
}

void TNodeJsStore::last(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	const uint64 LastRecId = JsStore->Store->LastRecId();
	if (LastRecId == TUInt64::Mx) {
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
	Info.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(LastRecId)));
}

void TNodeJsStore::forwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);
	v8::Local<v8::Object> F = TNodeJsStoreIter::New(
		JsStore->Store, JsStore->Store->ForwardIter());
	Info.GetReturnValue().Set(F);
}

void TNodeJsStore::backwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(TNodeJsStoreIter::New(
		JsStore->Store, JsStore->Store->BackwardIter()));
}


void TNodeJsStore::indexId(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Info.Holder());

	if (JsStore->Store->IsRecId(Index)) {
		Info.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(Index)));
		return;
	}
	
	Info.GetReturnValue().Set(v8::Null(Isolate));
}

///////////////////////////////
// NodeJs QMiner Record
TVec<TVec<v8::Persistent<v8::Function> > > TNodeJsRec::BaseStoreIdConstructor;

void TNodeJsRec::Init(const TWPt<TQm::TStore>& Store) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	QmAssertR(TNodeJsQm::BaseFPathToId.IsKey(Store->GetBase()->GetFPath()), "Base Id not found in TNodeJsRec::Init.");
	uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(Store->GetBase()->GetFPath());
	if (BaseStoreIdConstructor.Empty()) {
		// support 16 bases opened!
		BaseStoreIdConstructor.Gen(16);
	}
	if (BaseStoreIdConstructor[BaseId].Empty()) {
		// reserve space for maximal number of stores
		BaseStoreIdConstructor[BaseId].Gen(TQm::TEnv::GetMxStores());
	}

	// make sure template id is a valid
	const uint StoreId = Store->GetStoreId();
	QmAssertR(StoreId < TQm::TEnv::GetMxStores(), "Store id should not exceed max number of stores");
	// initialize template if not already prepared
	if (BaseStoreIdConstructor[BaseId][(int)StoreId].IsEmpty()) {

		v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
		tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "rec"));
		// ObjectWrap uses the first internal field to store the wrapped pointer.
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Add all prototype methods, getters and setters here.
		NODE_SET_PROTOTYPE_METHOD(tpl, "$clone", _clone);
		NODE_SET_PROTOTYPE_METHOD(tpl, "addJoin", _addJoin);
		NODE_SET_PROTOTYPE_METHOD(tpl, "delJoin", _delJoin);
		NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);

		// Properties 
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$id"), _id);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$name"), _name);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$fq"), _fq);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
		// register all the fields
		for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
			TStr FieldNm = Store->GetFieldDesc(FieldN).GetFieldNm();
			tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate,  FieldNm.CStr()), _getField, _setField);
		}		

		for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
			const TQm::TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
			if (JoinDesc.IsFieldJoin()) {
				tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, JoinDesc.GetJoinNm().CStr()), _sjoin);
			}
			else if (JoinDesc.IsIndexJoin()) {
				tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, JoinDesc.GetJoinNm().CStr()), _join);
			}
			else {
				TQm::ErrorLog("Unknown join type " + Store->GetStoreNm() + "." + JoinDesc.GetJoinNm());
			}
		}
		
		// This has to be last, otherwise the properties won't show up on the object in JavaScript.
		BaseStoreIdConstructor[BaseId][(int)StoreId].Reset(Isolate, tpl->GetFunction());
		//exports->Set(v8::String::NewFromUtf8(Isolate, "rec"), tpl->GetFunction());
	}
}

v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	QmAssertR(TNodeJsQm::BaseFPathToId.IsKey(Rec.GetStore()->GetBase()->GetFPath()), "rec constructor: Base Id not found!");
	uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(Rec.GetStore()->GetBase()->GetFPath());
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, BaseStoreIdConstructor[BaseId][Rec.GetStoreId()]);

	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsRec* JsRec = new TNodeJsRec(Rec, _Fq);
	printf("%s\n", TJsonVal::GetStrFromVal(Rec.GetJson(Rec.GetStore()->GetBase())).CStr());
	JsRec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsRec::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		if (Args.Length() == 2) {
			QmAssertR(Args[0]->IsObject() && Args[1]->IsObject(), "TNodeJsRec constructor expecting record JSON object and store object");
			// input = recJSON, store
			PJsonVal RecJSON = TNodeJsUtil::GetArgJson(Args, 0);
			TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args[1]->ToObject());
			// build rec
			TQm::TRec Rec(JsStore->Store, RecJSON);
			Args.GetReturnValue().Set(TNodeJsRec::New(Rec));
			return;		
		}
		else {
			TNodeJsRec* JsRec = new TNodeJsRec();
			v8::Local<v8::Object> Instance = Args.This();
			JsRec->Wrap(Instance);
			Args.GetReturnValue().Set(Instance);
			return;
		}
	}
	else {
		QmAssertR(Args[0]->IsObject() && Args[1]->IsObject(), "TNodeJsRec constructor expecting record JSON object and store object");
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args[1]->ToObject());

		QmAssertR(TNodeJsQm::BaseFPathToId.IsKey(JsStore->Store->GetBase()->GetFPath()), "rec constructor: Base Id not found!");
		uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(JsStore->Store->GetBase()->GetFPath());
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, BaseStoreIdConstructor[BaseId][JsStore->Store->GetStoreId()]);
		
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

void TNodeJsRec::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsRec::New(JsRec->Rec, JsRec->Fq));
}

void TNodeJsRec::addJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args.Holder());

	// read argument as record
	QmAssertR(Args.Length() >= 2 && Args[1]->IsObject(), "rec.addJoin needs at least 2 args: JoinNm, rec and fq (optional)");
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	// Check ?
	TNodeJsRec* JsJoinRec = ObjectWrap::Unwrap<TNodeJsRec>(Args[1]->ToObject());
	const int JoinFq = TNodeJsUtil::GetArgInt32(Args, 2, 1);
	// check parameters fine
	QmAssertR(JsRec->Rec.GetStore()->IsJoinNm(JoinNm), "[addJoin] Unknown join " + JsRec->Rec.GetStore()->GetStoreNm() + "." + JoinNm);
	QmAssertR(JoinFq > 0, "[addJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
	// get generic store
	TWPt<TQm::TStore> Store = JsRec->Rec.GetStore();
	const int JoinId = Store->GetJoinId(JoinNm);
	// add join
	Store->AddJoin(JoinId, JsRec->Rec.GetRecId(), JsJoinRec->Rec.GetRecId(), JoinFq);
	// return
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRec::delJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args.Holder());


	// read argument as record
	QmAssertR(Args.Length() >= 2 && Args[1]->IsObject(), "rec.addJoin needs at least 2 args: JoinNm, rec and fq (optional)");
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	// Check ?
	TNodeJsRec* JsJoinRec = ObjectWrap::Unwrap<TNodeJsRec>(Args[1]->ToObject());
	const int JoinFq = TNodeJsUtil::GetArgInt32(Args, 2, 1);
	// check parameters fine
	QmAssertR(JsRec->Rec.GetStore()->IsJoinNm(JoinNm), "[delJoin] Unknown join " + JsRec->Rec.GetStore()->GetStoreNm() + "." + JoinNm);
	QmAssertR(JoinFq > 0, "[delJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
	// get generic store
	TWPt<TQm::TStore> Store = JsRec->Rec.GetStore();
	const int JoinId = Store->GetJoinId(JoinNm);
	// add join
	Store->DelJoin(JoinId, JsRec->Rec.GetRecId(), JsJoinRec->Rec.GetRecId(), JoinFq);
	// return
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRec::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args.Holder());

	const bool JoinRecsP = TNodeJsUtil::IsArg(Args, 0) ?
		(TNodeJsUtil::IsArgBool(Args, 0) ? TNodeJsUtil::GetArgBool(Args, 0, false) : false) : false;
	const bool JoinRecFieldsP = TNodeJsUtil::IsArg(Args, 1) ?
		(TNodeJsUtil::IsArgBool(Args, 1) ? TNodeJsUtil::GetArgBool(Args, 1, false) : false) : false;

	const bool FieldsP = true;
	const bool StoreInfoP = false;
	
	PJsonVal RecJson = JsRec->Rec.GetJson(JsRec->Rec.GetStore()->GetBase(), FieldsP, StoreInfoP, JoinRecsP, JoinRecFieldsP);
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, RecJson));
}


void TNodeJsRec::id(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRec->Rec.GetRecId()));
}

void TNodeJsRec::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsRec->Rec.GetRecNm().CStr()));
}

void TNodeJsRec::fq(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRec->Fq));
}

void TNodeJsRec::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(TNodeJsStore::New(JsRec->Rec.GetStore()));
}

void TNodeJsRec::getField(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	const TQm::TRec& Rec = JsRec->Rec;
	const TWPt<TQm::TStore>& Store = Rec.GetStore();
	TStr FieldNm = TNodeJsUtil::GetStr(Name);
	const int FieldId = Store->GetFieldId(FieldNm);
	
	Info.GetReturnValue().Set(TNodeJsStore::Field(Rec, FieldId));
}

void TNodeJsRec::setField(v8::Local<v8::String> Name, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<void>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	TQm::TRec& Rec = JsRec->Rec;
	const TWPt<TQm::TStore>& Store = Rec.GetStore();
	TStr FieldNm = TNodeJsUtil::GetStr(Name);
	const int FieldId = Store->GetFieldId(FieldNm);
	//TODO: for now we don't support by-value records, fix this
	QmAssertR(Rec.IsByRef(), "Only records by reference (from stores) supported for setters.");
	// not null, get value
	const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
	if (Value->IsNull()) {
		QmAssertR(Desc.IsNullable(), "Field " + FieldNm + " not nullable");
		Rec.SetFieldNull(FieldId);
	}
	else if (Desc.IsInt()) {
		QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not integer");
		const int Int = Value->Int32Value();
		Rec.SetFieldInt(FieldId, Int);
	}
	else if (Desc.IsIntV()) {
		// check if we have JavaScript array
		QmAssertR(Value->IsArray(), TStr::Fmt("rec.%s should be set to an array!", FieldNm.CStr()));
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Value);
		TIntV IntV;
		for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
			v8::Local<v8::Value> ArrayVal = Array->Get(FltN);
			QmAssertR(ArrayVal->IsInt32(), "Field " + FieldNm + " expects array of integers");
			const int Val = ArrayVal->Int32Value();
			IntV.Add(Val);
		}
		Rec.SetFieldIntV(FieldId, IntV);
	}
	else if (Desc.IsUInt64()) {
		QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not uint64");
		const uint64 UInt64 = (uint64)Value->IntegerValue();
		Rec.SetFieldUInt64(FieldId, UInt64);
	}
	else if (Desc.IsStr()) {
		QmAssertR(Value->IsString(), "Field " + FieldNm + " not string");
		v8::String::Utf8Value Utf8(Value);
		Rec.SetFieldStr(FieldId, TStr(*Utf8));
	}
	else if (Desc.IsStrV()) {
		QmAssertR(Value->IsArray(), "Field " + FieldNm + " not array");
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Value);
		TStrV StrV;
		for (uint32_t StrN = 0; StrN < Array->Length(); StrN++) {
			v8::Local<v8::Value> ArrayVal = Array->Get(StrN);
			QmAssertR(ArrayVal->IsString(), "Field " + FieldNm + " expects array of strings");
			v8::String::Utf8Value Utf8(ArrayVal);
			StrV.Add(TStr(*Utf8));
		}
		Rec.SetFieldStrV(FieldId, StrV);
	}
	else if (Desc.IsBool()) {
		QmAssertR(Value->IsBoolean(), "Field " + FieldNm + " not boolean");
		Rec.SetFieldBool(FieldId, Value->BooleanValue());
	}
	else if (Desc.IsFlt()) {
		QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not numeric");
		TFlt Val(Value->NumberValue());
		bool NaNFound = Val.IsNan();
		if (NaNFound) {
			throw TQm::TQmExcept::New("Cannot set record field (type float) to NaN, for field name: " + FieldNm);
		}
		Rec.SetFieldFlt(FieldId, Val);
	}
	else if (Desc.IsFltPr()) {
		QmAssertR(Value->IsArray(), "Field " + FieldNm + " not array");
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Value);
		QmAssert(Array->Length() >= 2);
		QmAssert(Array->Get(0)->IsNumber());
		QmAssert(Array->Get(1)->IsNumber());
		TFltPr FltPr(Array->Get(0)->NumberValue(), Array->Get(1)->NumberValue());
	}
	else if (Desc.IsFltV()) {
		if (Value->IsArray()) {
			// check if we have JavaScript array
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Value);
			TFltV FltV;
			for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
				v8::Local<v8::Value> ArrayVal = Array->Get(FltN);
				QmAssertR(ArrayVal->IsNumber(), "Field " + FieldNm + " expects array of numbers");
				const double Val = ArrayVal->NumberValue();
				FltV.Add(Val);
			}
			Rec.SetFieldFltV(FieldId, FltV);
		}
		else {
			// otherwise it must be GLib array (or exception)
			TNodeJsVec<TFlt, TAuxFltV>* JsFltV = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Value->ToObject());
			Rec.SetFieldFltV(FieldId, JsFltV->Vec);
		}
	}
	else if (Desc.IsTm()) {
		QmAssertR(Value->IsObject() || Value->IsString(), "Field " + FieldNm + " not object or string");
		if (Value->IsObject()){
			// TODO
			throw TQm::TQmExcept::New("TODO: TNodeJsTm not implemented yet. Cannot set record field (type tm): " + FieldNm);
			//TJsTm* JsTm = TJsObjUtil<TJsTm>::GetSelf(Value->ToObject());
			//Rec.SetFieldTm(FieldId, JsTm->Tm);
		}
		else if (Value->IsString()){
			v8::String::Utf8Value Utf8(Value);
			Rec.SetFieldTm(FieldId, TTm::GetTmFromWebLogDateTimeStr(TStr(*Utf8), '-', ':', '.', 'T'));
		}
	}
	else if (Desc.IsNumSpV()) {
		// it can only be GLib sparse vector
		TNodeJsSpVec* JsSpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Value->ToObject());
		Rec.SetFieldNumSpV(FieldId, JsSpVec->Vec);
	}
	else if (Desc.IsBowSpV()) {
		throw TQm::TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
	}
	else {
		throw TQm::TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
	}
}


void TNodeJsRec::join(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);

	TStr JoinNm = TNodeJsUtil::GetStr(Name);
	TQm::PRecSet RecSet = JsRec->Rec.DoJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
	Info.GetReturnValue().Set(TNodeJsRecSet::New(RecSet));
}

void TNodeJsRec::sjoin(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);

	TStr JoinNm = TNodeJsUtil::GetStr(Name);
	TQm::TRec JoinRec = JsRec->Rec.DoSingleJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
	TWPt<TQm::TStore> JoinStore = JoinRec.GetStore();
	if (JoinRec.IsDef() && JoinStore->IsRecId(JoinRec.GetRecId())) {
		Info.GetReturnValue().Set(TNodeJsRec::New(JoinRec));
		return;
	}
	else {
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
}

///////////////////////////////
// NodeJs QMiner Record Set
v8::Persistent<v8::Function> TNodeJsRecSet::constructor;

void TNodeJsRecSet::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "rs"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "clone", _clone);
	NODE_SET_PROTOTYPE_METHOD(tpl, "join", _join);
	NODE_SET_PROTOTYPE_METHOD(tpl, "aggr", _aggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "trunc", _trunc);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sample", _sample);
	NODE_SET_PROTOTYPE_METHOD(tpl, "shuffle", _shuffle);
	NODE_SET_PROTOTYPE_METHOD(tpl, "revers", _reverse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortById", _sortById);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortByFq", _sortByFq);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortByField", _sortByField);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sort", sort);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterById", _filterById);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterByFq", _filterByFq);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterByField", _filterByField);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filter", _filter);
	NODE_SET_PROTOTYPE_METHOD(tpl, "split", _split);
	NODE_SET_PROTOTYPE_METHOD(tpl, "deleteRecs", _deleteRecs);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "each", _each);
	NODE_SET_PROTOTYPE_METHOD(tpl, "map", _map);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setintersect", _setintersect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setunion", _setunion);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setdiff", _setdiff);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getVec", _getVec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMat", _getMat);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "empty"), _empty);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weighted"), _weighted);
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	/*exports->Set(v8::String::NewFromUtf8(Isolate, "rs"),
		tpl->GetFunction());*/
}

v8::Local<v8::Object> TNodeJsRecSet::New() {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsRecSet* JsRecSet = new TNodeJsRecSet();
	JsRecSet->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

v8::Local<v8::Object> TNodeJsRecSet::New(const TQm::PRecSet& RecSet) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsRecSet* JsRecSet = new TNodeJsRecSet(RecSet);
	JsRecSet->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsRecSet::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		TNodeJsRecSet* JsRecSet = new TNodeJsRecSet();
		v8::Local<v8::Object> Instance = Args.This();
		JsRecSet->Wrap(Instance);
		Args.GetReturnValue().Set(Instance);
		return;
	}
	else {
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

void TNodeJsRecSet::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	TQm::PRecSet RecSet = JsRecSet->RecSet->Clone();
	Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet));
}

void TNodeJsRecSet::join(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 1, -1);
	TQm::PRecSet RecSet = JsRecSet->RecSet->DoJoin(JsRecSet->RecSet->GetStore()->GetBase(), JoinNm, SampleSize);
	Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet));
}

void TNodeJsRecSet::aggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	if (Args.Length() == 0) {
		// asking for existing aggregates
		PJsonVal AggrVal = JsRecSet->RecSet->GetAggrJson();
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, AggrVal));
		return;
	}
	else {
		// parameters for computing new aggregate
		TQm::PRecSet RecSet = JsRecSet->RecSet;
		PJsonVal AggrVal = TNodeJsUtil::GetArgJson(Args, 0);
		const TWPt<TQm::TBase>& Base = JsRecSet->RecSet->GetStore()->GetBase();
		const TWPt<TQm::TStore>& Store = JsRecSet->RecSet->GetStore();
		TQm::TQueryAggrV QueryAggrV; TQm::TQueryAggr::LoadJson(Base, Store, AggrVal, QueryAggrV);
		// if recset empty, not much to do
		if (RecSet->Empty()) {
			Args.GetReturnValue().Set(v8::Null(Isolate));
			return;
		}
		// compute new aggregates
		v8::Local<v8::Array> AggrValV = v8::Array::New(Isolate, QueryAggrV.Len());
		for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
			const TQm::TQueryAggr& QueryAggr = QueryAggrV[QueryAggrN];
			// compute aggregate
			TQm::PAggr Aggr = TQm::TAggr::New(Base, RecSet, QueryAggr);
			// serialize to json
			AggrValV->Set(QueryAggrN, TNodeJsUtil::ParseJson(Isolate, Aggr->SaveJson()));
		}
		// return aggregates
		if (AggrValV->Length() == 1) {
			// if only one, return as object
			if (AggrValV->Get(0)->IsObject()) {
				Args.GetReturnValue().Set(AggrValV->Get(0));
				return;
			}
			else {
				Args.GetReturnValue().Set(v8::Null(Isolate));
				return;
			}
		}
		else {
			// otherwise return as array
			if (AggrValV->IsArray()) {
				Args.GetReturnValue().Set(AggrValV);
				return;
			}
			else {
				Args.GetReturnValue().Set(v8::Null(Isolate));
				return;
			}
		}
	}
}

void TNodeJsRecSet::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	if (Args.Length() == 1) {
		const int Recs = TNodeJsUtil::GetArgInt32(Args, 0);
		JsRecSet->RecSet->Trunc(Recs);
	}
	else if (Args.Length() == 2) {
		const int Limit = TNodeJsUtil::GetArgInt32(Args, 0);
		const int Offset = TNodeJsUtil::GetArgInt32(Args, 1);
		JsRecSet->RecSet = JsRecSet->RecSet->GetLimit(Limit, Offset);
	}
	else {
		throw TQm::TQmExcept::New("Unsupported number of arguments to RecSet.trunc()");
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sample(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 0);
	TQm::PRecSet ResultSet = JsRecSet->RecSet->GetSampleRecSet(SampleSize, JsRecSet->RecSet->IsWgt());

	Args.GetReturnValue().Set(TNodeJsRecSet::New(ResultSet));
}

void TNodeJsRecSet::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	const int RndSeed = TNodeJsUtil::GetArgInt32(Args, 0, 0);
	TRnd Rnd(RndSeed);
	JsRecSet->RecSet->Shuffle(Rnd);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::reverse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	
	JsRecSet->RecSet->Reverse();
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortById(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	const bool Asc = TNodeJsUtil::GetArgInt32(Args, 0, 0) > 0;
	JsRecSet->RecSet->SortById(Asc);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortByFq(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	
	const bool Asc = TNodeJsUtil::GetArgInt32(Args, 0, 0) > 0;
	JsRecSet->RecSet->SortByFq(Asc);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	const TStr SortFieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int SortFieldId = JsRecSet->RecSet->GetStore()->GetFieldId(SortFieldNm);
	const bool Asc = TNodeJsUtil::GetArgInt32(Args, 1, 0) > 0;
	JsRecSet->RecSet->SortByField(Asc, SortFieldId);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsFunction(),
        "sort(..) expects one argument, which is a function.");
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);	
	for (int i = 0; i < JsRecSet->RecSet->GetRecs(); i++) {
		JsRecSet->RecSet->PutRecFq(i, i);
	}
	JsRecSet->RecSet->SortCmp(TJsRecPairFilter(JsRecSet->RecSet->GetStore(), Callback));
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filterById(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	if (Args.Length() == 1) {
		// we expect an array of IDs which we filter out
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		if (!ParamVal->IsArr()) {
			throw TQm::TQmExcept::New("Expected Array.");
		}
		TUInt64Set RecIdSet;
		for (int ArrValN = 0; ArrValN < ParamVal->GetArrVals(); ArrValN++) {
			PJsonVal Val = ParamVal->GetArrVal(ArrValN);
			uint64 RecId = (uint64)Val->GetNum();
			RecIdSet.AddKey(RecId);
		}
		JsRecSet->RecSet->FilterByRecIdSet(RecIdSet);
	}
	else if (Args.Length() == 2) {
		// we expect min and max ID
		const int MnRecId = TNodeJsUtil::GetArgInt32(Args, 0);
		const int MxRecId = TNodeJsUtil::GetArgInt32(Args, 1);
		JsRecSet->RecSet->FilterByRecId((uint64)MnRecId, (uint64)MxRecId);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filterByFq(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	
	const int MnFq = TNodeJsUtil::GetArgInt32(Args, 0);
	const int MxFq = TNodeJsUtil::GetArgInt32(Args, 1);
	JsRecSet->RecSet->FilterByFq(MnFq, MxFq);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filterByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	// get field
	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int FieldId = JsRecSet->RecSet->GetStore()->GetFieldId(FieldNm);
	const TQm::TFieldDesc& Desc = JsRecSet->RecSet->GetStore()->GetFieldDesc(FieldId);
	// parse filter according to field type
	if (Desc.IsInt()) {
		const int MnVal = TNodeJsUtil::GetArgInt32(Args, 1);
		const int MxVal = TNodeJsUtil::GetArgInt32(Args, 2);
		JsRecSet->RecSet->FilterByFieldInt(FieldId, MnVal, MxVal);
	}
	else if (Desc.IsStr() && TNodeJsUtil::IsArgStr(Args, 1)) {
		TStr StrVal = TNodeJsUtil::GetArgStr(Args, 1);
		JsRecSet->RecSet->FilterByFieldStr(FieldId, StrVal);
	}
	else if (Desc.IsFlt()) {
		const double MnVal = TNodeJsUtil::GetArgFlt(Args, 1);
		const double MxVal = TNodeJsUtil::GetArgFlt(Args, 2);
		JsRecSet->RecSet->FilterByFieldFlt(FieldId, MnVal, MxVal);
	}
	else if (Desc.IsTm()) {
		const TStr MnTmStr = TNodeJsUtil::GetArgStr(Args, 1);
		const uint64 MnTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MnTmStr, '-', ':', '.', 'T'));
		if (Args.Length() >= 3) {
			// we have upper limit
			const TStr MxTmStr = TNodeJsUtil::GetArgStr(Args, 2);
			const uint64 MxTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MxTmStr, '-', ':', '.', 'T'));
			JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, MxTmMSecs);
		}
		else {
			// we do not have upper limit
			JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, TUInt64::Mx);
		}
	}
	else {
		throw TQm::TQmExcept::New("Unsupported filed type for record set filtering: " + Desc.GetFieldTypeStr());
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1, "filter(..) expects one argument.");
	QmAssertR(Args.Length() == 1 && Args[0]->IsFunction(),
        "filter(..) expects one argument, which is a function.");
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	
	JsRecSet->RecSet->FilterBy(TJsRecFilter(JsRecSet->RecSet->GetStore(), Callback));

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::split(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1, "split(..) expects one argument.");
	QmAssertR(Args.Length() == 1 && Args[0]->IsFunction(),
        "split(..) expects one argument, which is a function.");
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	TQm::TRecSetV RecSetV = JsRecSet->RecSet->SplitBy(TJsRecPairFilter(JsRecSet->RecSet->GetStore(), Callback));
	// prepare result array
	v8::Local<v8::Array> JsRecSetV = v8::Array::New(Isolate, RecSetV.Len());
	for (int RecSetN = 0; RecSetN < RecSetV.Len(); RecSetN++) {
		JsRecSetV->Set(RecSetN, TNodeJsRecSet::New(RecSetV[RecSetN]));
	}
	Args.GetReturnValue().Set(JsRecSetV);
}

void TNodeJsRecSet::deleteRecs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "deleteRecs(..) expects a record set as input");
	TNodeJsRecSet* ArgJsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args[0]->ToObject());

	TUInt64Set RecIdSet; ArgJsRecSet->RecSet->GetRecIdSet(RecIdSet);
	JsRecSet->RecSet->RemoveRecIdSet(RecIdSet);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());
	
	const int MxHits = -1;
	const int Offset = 0;
	
    // make sure we do not try to interpet parameters when toJSON is called by JSON.stringify
	const bool JoinRecsP = (Args.Length() > 0) ?
        TNodeJsUtil::IsArgBool(Args, 0) ? TNodeJsUtil::GetArgBool(Args, 0, false) : false : false;
	const bool JoinRecFieldsP = (Args.Length() > 0) ?
        TNodeJsUtil::IsArgBool(Args, 1) ? TNodeJsUtil::GetArgBool(Args, 1, false) : false : false;
    // rest are always
	const bool FieldsP = true;
	const bool StoreInfoP = false;
	const bool AggrsP = false;
	
	PJsonVal JsObj = JsRecSet->RecSet->GetJson(JsRecSet->RecSet->GetStore()->GetBase(),
		MxHits, Offset, FieldsP, AggrsP, StoreInfoP, JoinRecsP, JoinRecFieldsP);
	
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsObj));
}

void TNodeJsRecSet::each(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 is not a function!");
	
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	// iterate through the recset
	const int Recs = RecSet->GetRecs();
	if (Recs > 0) {

		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::New(RecSet->GetRec(0));
		TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(RecObj);

		for (int RecIdN = 0; RecIdN < Recs; RecIdN++) {
			JsRec->Rec = RecSet->GetRec(RecIdN);
			JsRec->Fq = RecSet->GetRecFq(RecIdN);

			v8::Local<v8::Value> ArgV[Argc] = {
				RecObj,
				v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, RecIdN))
			};
			Callback->Call(GlobalContext, Argc, ArgV);
		}
	}
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "map: Argument 0 is not a function!");

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	// iterate through the recset
	const int Recs = RecSet->GetRecs();
	
    v8::Handle<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

	if (Recs > 0) {
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::New(RecSet->GetRec(0));
		TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(RecObj);

		for (int RecIdN = 0; RecIdN < Recs; RecIdN++) {
			JsRec->Rec = RecSet->GetRec(RecIdN);
			JsRec->Fq = RecSet->GetRecFq(RecIdN);

			v8::Local<v8::Value> ArgV[Argc] = {
				RecObj,
				v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, RecIdN))
			};
			v8::Handle<v8::Value> Result = Callback->Call(GlobalContext, Argc, ArgV);			
			ResultV->Set(RecIdN, Result);
		}
	}
	Args.GetReturnValue().Set(ResultV);
}

void TNodeJsRecSet::setintersect(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "rs.setintersect: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(), "recset.setintersect: the record sets do not point to the same store!");
	// Coputation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records in RecIdSet
	TQm::TRecFilterByRecIdSet Filter(RecIdSet, true);
	RecSet2->FilterBy(Filter);
	Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet2));
}

void TNodeJsRecSet::setunion(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "rs.setunion: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(), "recset.setunion: the record sets do not point to the same store!");
	//// GetMerge sorts the argument!
	TQm::PRecSet RecSet1Clone = RecSet1->Clone();
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->GetMerge(RecSet1Clone);
	
	Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet2));
}

void TNodeJsRecSet::setdiff(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "rs.setdiff: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(), "recset.setdiff: the record sets do not point to the same store!");
	// Computation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records NOT in RecIdSet
	RecSet2->FilterBy(TQm::TRecFilterByRecIdSet(RecIdSet, false));
	Args.GetReturnValue().Set(TNodeJsRecSet::New(RecSet2));
}

void TNodeJsRecSet::getVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	TWPt<TQm::TStore> Store = RecSet->GetStore();
	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int FieldId = JsRecSet->RecSet->GetStore()->GetFieldId(FieldNm);
	int Recs = (int)JsRecSet->RecSet->GetRecs();
	const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

	if (Desc.IsInt()) {
		TIntV ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = Store->GetFieldInt(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
		return;
	}
	else if (Desc.IsUInt64()) {
		TFltV ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = (double)Store->GetFieldUInt64(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
		return;
	}

	else if (Desc.IsStr()) {
		TStrV ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = Store->GetFieldStr(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TStr, TAuxStrV>::New(ColV));
		return;
	}

	else if (Desc.IsBool()) {
		TIntV ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = (int)Store->GetFieldBool(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
		return;
	}
	else if (Desc.IsFlt()) {
		TFltV ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = Store->GetFieldFlt(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
		return;
	}
	else if (Desc.IsTm()) {
		TFltV ColV(Recs);
		TTm Tm;
		for (int RecN = 0; RecN < Recs; RecN++) {
			Store->GetFieldTm(RecSet()->GetRecId(RecN), FieldId, Tm);
			ColV[RecN] = (double)TTm::GetMSecsFromTm(Tm);
		}
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
		return;
	}
	else if (Desc.IsFltV()) {
		throw TQm::TQmExcept::New("rs.getVec does not support type float_v - use store.getMat instead");
	}
	else if (Desc.IsNumSpV()) {
		throw TQm::TQmExcept::New("rs.getVec does not support type num_sp_v - use store.getMat instead");
	}
	throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::getMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	TWPt<TQm::TStore> Store = RecSet->GetStore();
	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int FieldId = JsRecSet->RecSet->GetStore()->GetFieldId(FieldNm);
	int Recs = (int)JsRecSet->RecSet->GetRecs();
	const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

	if (Desc.IsInt()) {
		TFltVV ColV(1, Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV(0, RecN) = (double)Store->GetFieldInt(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;		
	}
	else if (Desc.IsUInt64()) {
		TFltVV ColV(1, Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV(0, RecN) = (double)Store->GetFieldUInt64(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;
	}
	else if (Desc.IsBool()) {
		TFltVV ColV(1, Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV(0, RecN) = (double)Store->GetFieldBool(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;
	}
	else if (Desc.IsFlt()) {
		TFltVV ColV(1, Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			ColV(0, RecN) = Store->GetFieldFlt(RecSet()->GetRecId(RecN), FieldId);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;
	}
	else if (Desc.IsTm()) {
		TFltVV ColV(1, Recs);
		TTm Tm;
		for (int RecN = 0; RecN < Recs; RecN++) {
			Store->GetFieldTm(RecSet()->GetRecId(RecN), FieldId, Tm);
			ColV(0, RecN) = (double)TTm::GetMSecsFromTm(Tm);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;
	}
	else if (Desc.IsFltV()) {
		TFltV Vec;
		Store->GetFieldFltV(RecSet()->GetRecId(0), FieldId, Vec);
		TFltVV ColV(Vec.Len(), Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			Store->GetFieldFltV(RecSet()->GetRecId(RecN), FieldId, Vec);
			QmAssertR(Vec.Len() == ColV.GetRows(),
                TStr::Fmt("store.getCol for field type fltvec: dimensions are "
                    "not consistent! %d expected, %d found in row %d",
                    ColV.GetRows(), Vec.Len(), RecN));
			ColV.SetCol(RecN, Vec);
		}
		Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
		return;
	}
	else if (Desc.IsNumSpV()) {
		TVec<TIntFltKdV> ColV(Recs);
		for (int RecN = 0; RecN < Recs; RecN++) {
			Store->GetFieldNumSpV(RecSet()->GetRecId(RecN), FieldId, ColV[RecN]);
		}
		Args.GetReturnValue().Set(TNodeJsSpMat::New(ColV));
		return;
	}
	else if (Desc.IsStr()) {
		throw TQm::TQmExcept::New("store.getMat does not support type string - use store.getVec instead");
	}
	throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Info.Holder());
	
	Info.GetReturnValue().Set(TNodeJsStore::New(JsRecSet->RecSet->GetStore()));
}

void TNodeJsRecSet::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRecSet->RecSet->GetRecs()));
}

void TNodeJsRecSet::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Info.Holder());

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsRecSet->RecSet->Empty()));
}

void TNodeJsRecSet::weighted(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Info.Holder());

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsRecSet->RecSet->IsWgt()));
}

void TNodeJsRecSet::indexId(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Info.Holder());

	const int RecN = Index;
	if (0 <= RecN && RecN < JsRecSet->RecSet->GetRecs()) {
		const uint64 RecId = JsRecSet->RecSet->GetRecId(RecN);
		if (JsRecSet->RecSet->GetStore()->IsRecId(RecId)) {
			Info.GetReturnValue().Set(TNodeJsRec::New(JsRecSet->RecSet->GetRec(RecN), JsRecSet->RecSet->GetRecFq(RecN)));
			return;
		}
	}
	
	Info.GetReturnValue().Set(v8::Null(Isolate));
}

///////////////////////////////
// NodeJs QMiner Store Iterator
v8::Persistent<v8::Function> TNodeJsStoreIter::constructor;

void TNodeJsStoreIter::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "storeIter"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "next", _next);
	
	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "rec"), _rec);
	
	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	//exports->Set(v8::String::NewFromUtf8(Isolate, "storeIter"),
	//	tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsStoreIter::New(const TWPt<TQm::TStore>& _Store, const TWPt<TQm::TStoreIter>& _Iter) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();
	TNodeJsStoreIter* JsStoreIter = new TNodeJsStoreIter(_Store, _Iter);
	JsStoreIter->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsStoreIter::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		TNodeJsStoreIter* JsStoreIter = new TNodeJsStoreIter();
		v8::Local<v8::Object> Instance = Args.This();
		JsStoreIter->Wrap(Instance);
		Args.GetReturnValue().Set(Instance);
		return;
	}
	else {
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

void TNodeJsStoreIter::next(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Args.Holder());
	
	const bool NextP = JsStoreIter->Iter->Next();
	if (JsStoreIter->JsRec == NULL && NextP) {
		// first time, create placeholder
		const uint64 RecId = JsStoreIter->Iter->GetRecId();
		v8::Local<v8::Object> _RecObj = TNodeJsRec::New(JsStoreIter->Store->GetRec(RecId), 1);
		JsStoreIter->RecObj.Reset(Isolate, _RecObj);
		JsStoreIter->JsRec = ObjectWrap::Unwrap<TNodeJsRec>(_RecObj);
	}
	else if (NextP) {
		// not first time, just update the placeholder
		const uint64 RecId = JsStoreIter->Iter->GetRecId();
		JsStoreIter->JsRec->Rec = JsStoreIter->Store->GetRec(RecId);
	}	
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, NextP));
}

void TNodeJsStoreIter::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Info.Holder());

	Info.GetReturnValue().Set(TNodeJsStore::New(JsStoreIter->Store));
}

void TNodeJsStoreIter::rec(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Info.Holder());

	Info.GetReturnValue().Set(JsStoreIter->RecObj);	
}

///////////////////////////////
// NodeJs QMiner Record Filter
bool TJsRecFilter::operator()(const TUInt64IntKd& RecIdWgt) const {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// prepare record objects - since they are local, they are safe from GC
	v8::Local<v8::Object> JsRec = TNodeJsRec::New(TQm::TRec(Store, RecIdWgt.Key), RecIdWgt.Dat);

	v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
	v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
	const unsigned Argc = 1;
	v8::Local<v8::Value> ArgV[Argc] = { JsRec};
	v8::Local<v8::Value> ReturnVal = Callbck->Call(GlobalContext, Argc, ArgV);

	QmAssertR(ReturnVal->IsBoolean(), "Filter callback must return a boolean!");
	return ReturnVal->BooleanValue();
}

///////////////////////////////
// NodeJs QMiner Record Filter
bool TJsRecPairFilter::operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// prepare record objects - since they are local, they are safe from GC
	v8::Local<v8::Object> JsRec1 = TNodeJsRec::New(TQm::TRec(Store, RecIdWgt1.Key), RecIdWgt1.Dat);
	v8::Local<v8::Object> JsRec2 = TNodeJsRec::New(TQm::TRec(Store, RecIdWgt2.Key), RecIdWgt2.Dat);

	v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
	v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
	const unsigned Argc = 2;
	v8::Local<v8::Value> ArgV[Argc] = { JsRec1, JsRec2 };
	v8::Local<v8::Value> ReturnVal = Callbck->Call(GlobalContext, Argc, ArgV);

	QmAssertR(ReturnVal->IsBoolean(), "Comparator callback must return a boolean!");
	return ReturnVal->BooleanValue();
}

///////////////////////////////
// NodeJs QMiner Index Key
v8::Persistent<v8::Function> TNodeJsIndexKey::constructor;

void TNodeJsIndexKey::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "indexKey"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "voc"), _voc);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "fq"), _fq);
	
	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	//exports->Set(v8::String::NewFromUtf8(Isolate, "indexKey"), tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsIndexKey::New(const TWPt<TQm::TStore>& _Store, const TQm::TIndexKey& _IndexKey) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();
	TNodeJsIndexKey* JsIndexKey = new TNodeJsIndexKey(_Store, _IndexKey);
	JsIndexKey->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsIndexKey::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	if (Args.IsConstructCall()) {
		TNodeJsIndexKey* JsIndexKey = new TNodeJsIndexKey;
		v8::Local<v8::Object> Instance = Args.This();
		JsIndexKey->Wrap(Instance);
		Args.GetReturnValue().Set(Instance);
		return;
	} else {
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

void TNodeJsIndexKey::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = ObjectWrap::Unwrap<TNodeJsIndexKey>(Info.Holder());
	// return
	Info.GetReturnValue().Set(TNodeJsStore::New(JsIndexKey->Store));
}

void TNodeJsIndexKey::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = ObjectWrap::Unwrap<TNodeJsIndexKey>(Info.Holder());
	// return
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsIndexKey->IndexKey.GetKeyNm().CStr()));
}

void TNodeJsIndexKey::voc(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = ObjectWrap::Unwrap<TNodeJsIndexKey>(Info.Holder());
    // preapre vocabulary array
    if (!JsIndexKey->IndexKey.IsWordVoc()) {
        // no vocabulary
        Info.GetReturnValue().Set(v8::Null(Isolate));
    } else {
        // get vocabulary
        TStrV KeyValV;
        JsIndexKey->Store->GetBase()->GetIndexVoc()->GetAllWordStrV(
            JsIndexKey->IndexKey.GetKeyId(), KeyValV);
        Info.GetReturnValue().Set(TNodeJsVec<TStr, TAuxStrV>::New(KeyValV));
    }
}

void TNodeJsIndexKey::fq(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = ObjectWrap::Unwrap<TNodeJsIndexKey>(Info.Holder());
    // preapre vocabulary array
    if (!JsIndexKey->IndexKey.IsWordVoc()) {
        // no vocabulary
        Info.GetReturnValue().Set(v8::Null(Isolate));
    } else {
        // get vocabulary
        TStrIntPrV KeyValV;
        JsIndexKey->Store->GetBase()->GetIndexVoc()->GetAllWordStrFqV(
            JsIndexKey->IndexKey.GetKeyId(), KeyValV);
        TIntV ValV(KeyValV.Len(), 0);
        for (int WordN = 0; WordN < KeyValV.Len(); WordN++) {
            ValV.Add(KeyValV[WordN].Val2);
        }
        Info.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ValV));
    }
}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
    // QMiner package
    TNodeJsQm::Init(exports);
    TNodeJsBase::Init(exports);   
	TNodeJsSA::Init(exports);
    TNodeJsStore::Init(exports);
    // the record templates are initiated elsewhere: qm.open, qm.create, base.createStore
    TNodeJsRecSet::Init(exports);
    TNodeJsStoreIter::Init(exports);
    TNodeJsIndexKey::Init(exports);
    
    // Linear algebra package
    TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
    TNodeJsVec<TInt, TAuxIntV>::Init(exports);
    TNodeJsVec<TStr, TAuxStrV>::Init(exports);
    TNodeJsFltVV::Init(exports);
    TNodeJsSpVec::Init(exports);
    TNodeJsSpMat::Init(exports);
}

NODE_MODULE(qm, init)


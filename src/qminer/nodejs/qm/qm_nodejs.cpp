#include "qm_nodejs.h"
#include "qm_param.h"
#include "../la/la_nodejs.h"

///////////////////////////////
// NodeJs-Qminer

void TNodeJsQm::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
	QmAssertR(!(TFile::Exists(ConfFNm) && !OverWriteP), "Configuration file already exists (" + ConfFNm + "). Use overwrite!");
	
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
	}
	// remove lock
	Lock.Unlock();
}

///////////////////////////////
// NodeJs-Qminer-Base

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
   
   // This has to be last, otherwise the properties won't show up on the
   // object in JavaScript.
   constructor.Reset(Isolate, tpl->GetFunction());
   exports->Set(v8::String::NewFromUtf8(Isolate, "base"),
	   tpl->GetFunction());

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
      
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   //Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}


///////////////////////////////
// NodeJs-Qminer-Store
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

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "store"),
		tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsStore::New(TWPt<TQm::TStore> _Store) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsStore* JsStore = new TNodeJsStore(_Store);
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
			TNodeJsRec* JsRec = TNodeJsRec::GetJsRec(RecObj);

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
			TNodeJsRec* JsRec = TNodeJsRec::GetJsRec(RecObj);

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

		Args.GetReturnValue().Set(TNodeJsRec::New(TQm::TRec(Store, RecVal)));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::newRecSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() > 0) {
		// argument 0 = TJsIntV of record ids
		QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, "TIntV"),
			"Store.getRecSetByIdV: The first argument must be a TIntV (js linalg full int vector)");
		//TNodeJsVec<TInt, TAuxIntV>* JsVecArg
		//TNodeJsIntV* JsVec = TJsObjUtil<TQm::TJsIntV>::GetArgObj(Args, 0);
		//PRecSet ResultSet = TRecSet::New(JsStore->Store, JsVec->Vec);
		//return TJsRecSet::New(JsStore->Js, ResultSet);
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
			//	TODO		Args.GetReturnValue().Set(TJsIndexKey::New(Key));
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
		TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

		TWPt<TQm::TStore>& Store = JsStore->Store;
		//TODO
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
			//	TODO		Args.GetReturnValue().Set(TJsSA::New(StreamAggr));
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

			//TODO			PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
				// TODO				ColV[RecN] = JsStore->Store->GetFieldInt(Iter->GetRecId(), FieldId);
				// TODO				Iter->Next();
			}

			// TODO			Args.GetReturnValue().Set(TJsIntV::New(JsStore->Js, ColV));
			return;
		}
		else if (Desc.IsUInt64()) {
			TFltV ColV(Recs);
			//TODO			//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			//for (int RecN = 0; RecN < Recs; RecN++) {
			//	ColV[RecN] = (double)JsStore->Store->GetFieldUInt64(Iter->GetRecId(), FieldId);
			//	Iter->Next();
			//}
			//Args.GetReturnValue().Set(TJsFltV::New(JsStore->Js, ColV));
			return;
		}

		else if (Desc.IsStr()) {
			TStrV ColV(Recs);
			// TODO			PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			/*for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = JsStore->Store->GetFieldStr(Iter->GetRecId(), FieldId);
			Iter->Next();
			}
			Args.GetReturnValue().Set(TJsStrV::New(JsStore->Js, ColV));
			return;*/
		}

		else if (Desc.IsBool()) {
			TIntV ColV(Recs);
			/*	PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			for (int RecN = 0; RecN < Recs; RecN++) {
			ColV[RecN] = (int)JsStore->Store->GetFieldBool(Iter->GetRecId(), FieldId);
			Iter->Next();
			}
			Args.GetReturnValue().Set(TJsIntV::New(JsStore->Js, ColV));
			return;*/
		}
		else if (Desc.IsFlt()) {
			TFltV ColV(Recs);
			//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			//for (int RecN = 0; RecN < Recs; RecN++) {
			//	ColV[RecN] = JsStore->Store->GetFieldFlt(Iter->GetRecId(), FieldId);
			//	Iter->Next();
			//}
			//Args.GetReturnValue().Set(TJsFltV::New(JsStore->Js, ColV));
			//return;
		}
		else if (Desc.IsTm()) {
			TFltV ColV(Recs);
			//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
			//TTm Tm;
			//for (int RecN = 0; RecN < Recs; RecN++) {
			//	Store->GetFieldTm(Iter->GetRecId(), FieldId, Tm);
			//	ColV[RecN] = (double)TTm::GetMSecsFromTm(Tm);
			//	Iter->Next();
			//}
			//Args.GetReturnValue().Set(TJsFltV::New(JsStore->Js, ColV));
			//return;
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

	//try {
	//	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

	//	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());
	//	TWPt<TQm::TStore> Store = JsStore->Store;

	//	if (!Store->IsFieldNm(FieldNm)) {
	//		throw TQm::TQmExcept::New("store.getMat: fieldName not found: " + FieldNm);
	//	}
	//	const int FieldId = JsStore->Store->GetFieldId(FieldNm);
	//	int Recs = (int)JsStore->Store->GetRecs();
	//	const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

	//	if (Desc.IsInt()) {
	//		TFltVV ColV(1, Recs);
	//		//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		//for (int RecN = 0; RecN < Recs; RecN++) {
	//		//	ColV.At(0, RecN) = (double)JsStore->Store->GetFieldInt(Iter->GetRecId(), FieldId);
	//		//	Iter->Next();
	//		//}
	//		//Args.GetReturnValue().Set(TJsFltVV::New(JsStore->Js, ColV));
	//		//return;
	//	}
	//	else if (Desc.IsUInt64()) {
	//		TFltVV ColV(1, Recs);
	//		/*PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		for (int RecN = 0; RecN < Recs; RecN++) {
	//			ColV.At(0, RecN) = (double)JsStore->Store->GetFieldUInt64(Iter->GetRecId(), FieldId);
	//			Iter->Next();
	//		}
	//		Args.GetReturnValue().Set(TJsFltVV::New(JsStore->Js, ColV));
	//		return;*/
	//	}
	//	else if (Desc.IsBool()) {
	//		TFltVV ColV(1, Recs);
	//		//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		//for (int RecN = 0; RecN < Recs; RecN++) {
	//		//	ColV.At(0, RecN) = (double)JsStore->Store->GetFieldBool(Iter->GetRecId(), FieldId);
	//		//	Iter->Next();
	//		//}
	//		//return HandleScope.Close(TJsFltVV::New(JsStore->Js, ColV));
	//	}
	//	else if (Desc.IsFlt()) {
	//		TFltVV ColV(1, Recs);
	//		//PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		//for (int RecN = 0; RecN < Recs; RecN++) {
	//		//	ColV.At(0, RecN) = JsStore->Store->GetFieldFlt(Iter->GetRecId(), FieldId);
	//		//	Iter->Next();
	//		//}
	//		//Args.GetReturnValue().Set(TJsFltVV::New(JsStore->Js, ColV));
	//		//return;
	//	}
	//	else if (Desc.IsTm()) {
	//		TFltVV ColV(1, Recs);
	//		PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		TTm Tm;
	//		for (int RecN = 0; RecN < Recs; RecN++) {
	//			Store->GetFieldTm(Iter->GetRecId(), FieldId, Tm);
	//			ColV.At(0, RecN) = (double)TTm::GetMSecsFromTm(Tm);
	//			Iter->Next();
	//		}
	//		Args.GetReturnValue().Set(TJsFltVV::New(JsStore->Js, ColV));
	//		return;
	//	}
	//	else if (Desc.IsFltV()) {
	//		PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		TFltV Vec;
	//		JsStore->Store->GetFieldFltV(Iter->GetRecId(), FieldId, Vec);
	//		TFltVV ColV(Vec.Len(), Recs);
	//		for (int RecN = 0; RecN < Recs; RecN++) {
	//			JsStore->Store->GetFieldFltV(Iter->GetRecId(), FieldId, Vec);
	//			QmAssertR(Vec.Len() == ColV.GetRows(), TStr::Fmt("store.getCol for field type fltvec: dimensions are not consistent! %d expected, %d found in row %d", ColV.GetRows(), Vec.Len(), RecN));
	//			// copy row
	//			ColV.SetCol(RecN, Vec);
	//			Iter->Next();
	//		}
	//		Args.GetReturnValue().Set(TJsFltVV::New(JsStore->Js, ColV));
	//		return;
	//	}
	//	else if (Desc.IsNumSpV()) {
	//		PStoreIter Iter = Store->ForwardIter(); Iter->Next();
	//		TVec<TIntFltKdV> ColV(Recs);
	//		for (int RecN = 0; RecN < Recs; RecN++) {
	//			JsStore->Store->GetFieldNumSpV(Iter->GetRecId(), FieldId, ColV[RecN]);
	//			Iter->Next();
	//		}
	//		Args.GetReturnValue().Set(TJsSpMat::New(JsStore->Js, ColV));
	//		return;
	//	}
	//	else if (Desc.IsStr()) {
	//		throw TQmExcept::New("store.getMat does not support type string - use store.getVec instead");
	//	}
	//	throw TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
	//} catch (const PExcept& Except) {
	//	throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	//}
}

void TNodeJsStore::cell(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
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

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::recs(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::fields(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::joins(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::keys(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::first(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::last(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::forwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsStore::backwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);

	//Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}



///////////////////////////////
// NodeJs-Qminer-Record
v8::Persistent<v8::Function> TNodeJsRec::constructor;

void TNodeJsRec::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

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
	//tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, getField, setField
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "join"), _join);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "sjoin"), _sjoin);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "rec"),
		tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsRec* JsRec = new TNodeJsRec(Rec);
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
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
		v8::Local<v8::Object> Instance = cons->NewInstance();
		Args.GetReturnValue().Set(Instance);
		return;
	}
}

TNodeJsRec* TNodeJsRec::GetJsRec(v8::Local<v8::Object> RecObj) {
	QmAssertR(RecObj->IsObject(), "Argument is not an object!");
	v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RecObj->GetInternalField(0));
	return static_cast<TNodeJsRec*>(WrappedObject->Value());
}

void TNodeJsRec::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	//TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

	//	const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
	//	if (JsStore->Store->IsRecNm(RecNm)) {
	//		Args.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(RecNm)));
	//	}
	//	else {
	//		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	//	}
}

void TNodeJsRec::addJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	//TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

	//	const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
	//	if (JsStore->Store->IsRecNm(RecNm)) {
	//		Args.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(RecNm)));
	//	}
	//	else {
	//		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	//	}
}

void TNodeJsRec::delJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	//TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Args.Holder());

	//	const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
	//	if (JsStore->Store->IsRecNm(RecNm)) {
	//		Args.GetReturnValue().Set(TNodeJsRec::New(JsStore->Store->GetRec(RecNm)));
	//	}
	//	else {
	//		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	//	}
}

void TNodeJsRec::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Args.Holder());

	const bool JoinRecsP = TNodeJsUtil::GetArgBool(Args, 0, false);	
	const bool JoinRecFieldsP = TNodeJsUtil::GetArgBool(Args, 1, false);
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

	/*v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsStore->Store->GetStoreNm().CStr()));*/
}

void TNodeJsRec::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = ObjectWrap::Unwrap<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(TNodeJsStore::New(JsRec->Rec.GetStore()));
}

void TNodeJsRec::join(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	/*v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsStore->Store->GetStoreNm().CStr()));*/
}

void TNodeJsRec::sjoin(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	/*v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = ObjectWrap::Unwrap<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsStore->Store->GetStoreNm().CStr()));*/
}

///////////////////////////////
// NodeJs-Qminer-Record-Set
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
	//tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "indexId"), _indexId);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "rs"),
		tpl->GetFunction());
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

TNodeJsRecSet* TNodeJsRecSet::GetJsRecSet(v8::Local<v8::Object> RecSetObj) {
	QmAssertR(RecSetObj->IsObject(), "Argument is not an object!");
	v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RecSetObj->GetInternalField(0));
	return static_cast<TNodeJsRecSet*>(WrappedObject->Value());
}

void TNodeJsRecSet::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = JsRecSet->RecSet->Clone();
	//return TJsRecSet::New(JsRecSet->Js, RecSet);
}

void TNodeJsRecSet::join(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//TStr JoinNm = TJsRecSetUtil::GetArgStr(Args, 0);
	//const int SampleSize = TJsRecSetUtil::GetArgInt32(Args, 1, -1);
	//PRecSet RecSet = JsRecSet->RecSet->DoJoin(JsRecSet->Js->Base, JoinNm, SampleSize);
	//return TJsRecSet::New(JsRecSet->Js, RecSet);
}
void TNodeJsRecSet::aggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//if (Args.Length() == 0) {
	//	// asking for existing aggregates
	//	PJsonVal AggrVal = JsRecSet->RecSet->GetAggrJson();
	//	return HandleScope.Close(TJsUtil::ParseJson(AggrVal));
	//}
	//else {
	//	// parameters for computing new aggregate
	//	PRecSet RecSet = JsRecSet->RecSet;
	//	PJsonVal AggrVal = TJsRecSetUtil::GetArgJson(Args, 0);
	//	const TWPt<TBase>& Base = JsRecSet->Js->Base;
	//	const TWPt<TStore>& Store = JsRecSet->RecSet->GetStore();
	//	TQueryAggrV QueryAggrV; TQueryAggr::LoadJson(Base, Store, AggrVal, QueryAggrV);
	//	// if recset empty, not much to do
	//	if (RecSet->Empty()) { HandleScope.Close(v8::Null()); }
	//	// compute new aggregates
	//	v8::Local<v8::Array> AggrValV = v8::Array::New(QueryAggrV.Len());
	//	for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
	//		const TQueryAggr& QueryAggr = QueryAggrV[QueryAggrN];
	//		// compute aggregate
	//		PAggr Aggr = TAggr::New(Base, RecSet, QueryAggr);
	//		// serialize to json
	//		AggrValV->Set(QueryAggrN, TJsUtil::ParseJson(Aggr->SaveJson()));
	//	}
	//	// return aggregates
	//	if (AggrValV->Length() == 1) {
	//		// if only one, return as object
	//		if (AggrValV->Get(0)->IsObject()) {
	//			return AggrValV->Get(0);
	//		}
	//		else {
	//			return HandleScope.Close(v8::Null());
	//		}
	//	}
	//	else {
	//		// otherwise return as array
	//		if (AggrValV->IsArray()) {
	//			return HandleScope.Close(AggrValV);
	//		}
	//		else {
	//			return HandleScope.Close(v8::Null());
	//		}
	//	}
	//}
}

void TNodeJsRecSet::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//if (Args.Length() == 1) {
	//	const int Recs = TJsRecSetUtil::GetArgInt32(Args, 0);
	//	JsRecSet->RecSet->Trunc(Recs);
	//}
	//else if (Args.Length() == 2) {
	//	const int Limit = TJsRecSetUtil::GetArgInt32(Args, 0);
	//	const int Offset = TJsRecSetUtil::GetArgInt32(Args, 1);
	//	JsRecSet->RecSet = JsRecSet->RecSet->GetLimit(Limit, Offset);
	//}
	//else {
	//	throw TQmExcept::New("Unsupported number of arguments to RecSet.trunc()");
	//}
	//return Args.Holder();
}

void TNodeJsRecSet::sample(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const int SampleSize = TJsRecSetUtil::GetArgInt32(Args, 0);
	//PRecSet ResultSet = JsRecSet->RecSet->GetSampleRecSet(SampleSize, JsRecSet->RecSet->IsWgt());
	//return New(JsRecSet->Js, ResultSet);
}

void TNodeJsRecSet::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const int RndSeed = TJsRecSetUtil::GetArgInt32(Args, 0, 0);
	//TRnd Rnd(RndSeed);
	//JsRecSet->RecSet->Shuffle(Rnd);
	//return Args.Holder();
}

void TNodeJsRecSet::reverse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//JsRecSet->RecSet->Reverse();
	//return Args.Holder();
}

void TNodeJsRecSet::sortById(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	//JsRecSet->RecSet->SortById(Asc);
	//return Args.Holder();
}

void TNodeJsRecSet::sortByFq(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 0, 0) > 0);
	//JsRecSet->RecSet->SortByFq(Asc);
	//return Args.Holder();
}
void TNodeJsRecSet::sortByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const TStr SortFieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	//const int SortFieldId = JsRecSet->Store->GetFieldId(SortFieldNm);
	//const bool Asc = (TJsRecSetUtil::GetArgInt32(Args, 1, 0) > 0);
	//JsRecSet->RecSet->SortByField(Asc, SortFieldId);
	//return Args.Holder();
}

void TNodeJsRecSet::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//QmAssertR(Args.Length() == 1, "sort(..) expects one argument.");
	//v8::Persistent<v8::Function> CmpFun = TJsRecSetUtil::GetArgFunPer(Args, 0);
	//for (int i = 0; i < JsRecSet->RecSet->GetRecs(); i++) {
	//	JsRecSet->RecSet->PutRecFq(i, i);
	//}
	//JsRecSet->RecSet->SortCmp(TJsRecCmp(JsRecSet->Js, JsRecSet->Store, CmpFun));
	//return Args.Holder();
}

void TNodeJsRecSet::filterById(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//if (Args.Length() == 1) {
	//	// we expect an array of IDs which we filter out
	//	PJsonVal ParamVal = TJsRecSetUtil::GetArgJson(Args, 0);
	//	if (!ParamVal->IsArr()) {
	//		throw TQmExcept::New("Expected Array.");
	//	}
	//	TUInt64Set RecIdSet;
	//	for (int ArrValN = 0; ArrValN < ParamVal->GetArrVals(); ArrValN++) {
	//		PJsonVal Val = ParamVal->GetArrVal(ArrValN);
	//		uint64 RecId = (uint64)Val->GetNum();
	//		RecIdSet.AddKey(RecId);
	//	}
	//	JsRecSet->RecSet->FilterByRecIdSet(RecIdSet);
	//}
	//else if (Args.Length() == 2) {
	//	// we expect min and max ID
	//	const int MnRecId = TJsRecSetUtil::GetArgInt32(Args, 0);
	//	const int MxRecId = TJsRecSetUtil::GetArgInt32(Args, 1);
	//	JsRecSet->RecSet->FilterByRecId((uint64)MnRecId, (uint64)MxRecId);
	//}
	//return Args.Holder();
}

void TNodeJsRecSet::filterByFq(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const int MnFq = TJsRecSetUtil::GetArgInt32(Args, 0);
	//const int MxFq = TJsRecSetUtil::GetArgInt32(Args, 1);
	//JsRecSet->RecSet->FilterByFq(MnFq, MxFq);
	//return Args.Holder();
}

void TNodeJsRecSet::filterByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//// get field
	//const TStr FieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	//const int FieldId = JsRecSet->Store->GetFieldId(FieldNm);
	//const TFieldDesc& Desc = JsRecSet->Store->GetFieldDesc(FieldId);
	//// parse filter according to field type
	//if (Desc.IsInt()) {
	//	const int MnVal = TJsRecSetUtil::GetArgInt32(Args, 1);
	//	const int MxVal = TJsRecSetUtil::GetArgInt32(Args, 2);
	//	JsRecSet->RecSet->FilterByFieldInt(FieldId, MnVal, MxVal);
	//}
	//else if (Desc.IsStr() && TJsRecSetUtil::IsArgStr(Args, 1)) {
	//	TStr StrVal = TJsRecSetUtil::GetArgStr(Args, 1);
	//	JsRecSet->RecSet->FilterByFieldStr(FieldId, StrVal);
	//}
	//else if (Desc.IsFlt()) {
	//	const double MnVal = TJsRecSetUtil::GetArgFlt(Args, 1);
	//	const double MxVal = TJsRecSetUtil::GetArgFlt(Args, 2);
	//	JsRecSet->RecSet->FilterByFieldFlt(FieldId, MnVal, MxVal);
	//}
	//else if (Desc.IsTm()) {
	//	const TStr MnTmStr = TJsRecSetUtil::GetArgStr(Args, 1);
	//	const uint64 MnTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MnTmStr, '-', ':', '.', 'T'));
	//	if (TJsRecSetUtil::IsArg(Args, 2)) {
	//		// we have upper limit
	//		const TStr MxTmStr = TJsRecSetUtil::GetArgStr(Args, 2);
	//		const uint64 MxTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MxTmStr, '-', ':', '.', 'T'));
	//		JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, MxTmMSecs);
	//	}
	//	else {
	//		// we do not have upper limit
	//		JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, TUInt64::Mx);
	//	}
	//}
	//else {
	//	throw TQmExcept::New("Unsupported filed type for record set filtering: " + Desc.GetFieldTypeStr());
	//}
	//return Args.Holder();
}

void TNodeJsRecSet::filter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//QmAssertR(Args.Length() == 1, "filter(..) expects one argument.");
	//v8::Persistent<v8::Function> FilterFun = TJsRecSetUtil::GetArgFunPer(Args, 0);
	//JsRecSet->RecSet->FilterBy(TJsRecFilter(JsRecSet->Js, JsRecSet->Store, FilterFun));
	//return Args.Holder();
}

void TNodeJsRecSet::split(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//QmAssertR(Args.Length() == 1, "split(..) expects one argument.");
	//v8::Persistent<v8::Function> SplitterFun = TJsRecSetUtil::GetArgFunPer(Args, 0);
	//TRecSetV RecSetV = JsRecSet->RecSet->SplitBy(TJsRecSplitter(JsRecSet->Js, JsRecSet->Store, SplitterFun));
	//// prepare result array
	//v8::Local<v8::Array> JsRecSetV = v8::Array::New(RecSetV.Len());
	//for (int RecSetN = 0; RecSetN < RecSetV.Len(); RecSetN++) {
	//	JsRecSetV->Set(RecSetN, TJsRecSet::New(JsRecSet->Js, RecSetV[RecSetN]));
	//}
	//return HandleScope.Close(JsRecSetV);
}

void TNodeJsRecSet::deleteRecs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = TJsRecSet::GetArgRecSet(Args, 0);
	//TUInt64Set RecIdSet; RecSet->GetRecIdSet(RecIdSet);
	//JsRecSet->RecSet->RemoveRecIdSet(RecIdSet);
	//return Args.Holder();
}

void TNodeJsRecSet::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//const int MxHits = -1;
	//const int Offset = 0;
	//const bool JoinRecsP = TJsRecSetUtil::IsArg(Args, 0) ?
	//	(TJsRecSetUtil::IsArgBool(Args, 0) ? TJsRecSetUtil::GetArgBool(Args, 0, false) : false) : false;
	//const bool JoinRecFieldsP = TJsRecSetUtil::IsArg(Args, 1) ?
	//	(TJsRecSetUtil::IsArgBool(Args, 1) ? TJsRecSetUtil::GetArgBool(Args, 1, false) : false) : false;
	//const bool FieldsP = true;
	//const bool AggrsP = false;
	//const bool StoreInfoP = false;
	//PJsonVal JsObj = JsRecSet->RecSet->GetJson(JsRecSet->Js->Base,
	//	MxHits, Offset, FieldsP, AggrsP, StoreInfoP, JoinRecsP, JoinRecFieldsP);
	//return HandleScope.Close(TJsUtil::ParseJson(JsObj));
}
void TNodeJsRecSet::each(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = JsRecSet->RecSet;
	//QmAssertR(TJsRecSetUtil::IsArgFun(Args, 0), "each: Argument 0 is not a function!");
	//v8::Handle<v8::Function> CallbackFun = TJsRecSetUtil::GetArgFun(Args, 0);
	//// iterate through the recset
	//const int Recs = RecSet->GetRecs();
	//if (Recs > 0) {
	//	// prepare record placeholder
	//	v8::Persistent<v8::Object> RecArg = TJsRec::New(JsRecSet->Js,
	//		RecSet->GetRec(0), RecSet->GetRecFq(0), false);
	//	TJsRec* JsRec = TJsRec::GetJsRec(RecArg);
	//	// go over the record set
	//	for (int RecIdN = 0; RecIdN < Recs; RecIdN++) {
	//		JsRec->Rec = RecSet->GetRec(RecIdN);
	//		JsRec->Fq = RecSet->GetRecFq(RecIdN);
	//		v8::Handle<v8::Value> IdxArg = v8::Integer::New(RecIdN);
	//		// execute callback
	//		JsRecSet->Js->Execute(CallbackFun, RecArg, IdxArg);
	//	}
	//	TJsObjUtil<TJsRec>::MakeWeak(RecArg);
	//}
	//return Args.Holder();
}

void TNodeJsRecSet::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = JsRecSet->RecSet;
	//QmAssertR(TJsRecSetUtil::IsArgFun(Args, 0), "map: Argument 0 is not a function!");
	//v8::Handle<v8::Function> CallbackFun = TJsRecSetUtil::GetArgFun(Args, 0);
	//// create a new array for storing the results
	//const int Recs = RecSet->GetRecs();
	//v8::Handle<v8::Array> ResultV = v8::Array::New(Recs);
	//if (Recs > 0) {
	//	// prepare record placeholder
	//	v8::Persistent<v8::Object> RecArg = TJsRec::New(JsRecSet->Js,
	//		RecSet->GetRec(0), RecSet->GetRecFq(0), false);
	//	TJsRec* JsRec = TJsRec::GetJsRec(RecArg);
	//	// iterate through the recset
	//	for (int RecIdN = 0; RecIdN < Recs; RecIdN++) {
	//		JsRec->Rec = RecSet->GetRec(RecIdN);
	//		JsRec->Fq = RecSet->GetRecFq(RecIdN);
	//		v8::Handle<v8::Value> IdxArg = v8::Integer::New(RecIdN);
	//		// execute callback
	//		v8::Handle<v8::Value> Result = JsRecSet->Js->ExecuteV8(CallbackFun, RecArg, IdxArg);
	//		// store in the new array
	//		ResultV->Set(RecIdN, Result);
	//	}
	//	TJsObjUtil<TJsRec>::MakeWeak(RecArg);
	//}
	//return ResultV;
}

void TNodeJsRecSet::setintersect(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	//QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setintersect: the record sets do not point to the same store!");
	//// Coputation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	//PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	//TUInt64Set RecIdSet;
	//RecSet1->GetRecIdSet(RecIdSet);
	////second parameter in filter is false -> keep only records in RecIdSet
	//TRecFilterByRecIdSet Filter(RecIdSet, true);
	//RecSet2->FilterBy(Filter);
	//return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

void TNodeJsRecSet::setunion(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	//QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setunion: the record sets do not point to the same store!");
	//// GetMerge sorts the argument!
	//PRecSet RecSet1Clone = RecSet1->Clone();
	//PRecSet RecSet2 = JsRecSet->RecSet->GetMerge(RecSet1Clone);
	//return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

void TNodeJsRecSet::setdiff(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet1 = TJsRecSet::GetArgRecSet(Args, 0);
	//QmAssertR(JsRecSet->Store->GetStoreId() == RecSet1->GetStoreId(), "recset.setdiff: the record sets do not point to the same store!");
	//// Computation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	//PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	//TUInt64Set RecIdSet;
	//RecSet1->GetRecIdSet(RecIdSet);
	////second parameter in filter is false -> keep only records NOT in RecIdSet
	//RecSet2->FilterBy(TRecFilterByRecIdSet(RecIdSet, false));
	//return HandleScope.Close(TJsRecSet::New(JsRecSet->Js, RecSet2));
}

void TNodeJsRecSet::getVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = JsRecSet->RecSet;
	//TWPt<TStore> Store = RecSet->GetStore();
	//const TStr FieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	//const int FieldId = JsRecSet->RecSet->GetStore()->GetFieldId(FieldNm);
	//int Recs = (int)JsRecSet->RecSet->GetRecs();
	//const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

	//if (Desc.IsInt()) {
	//	TIntV ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV[RecN] = Store->GetFieldInt(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsIntV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsUInt64()) {
	//	TFltV ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV[RecN] = (double)Store->GetFieldUInt64(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltV::New(JsRecSet->Js, ColV));
	//}

	//else if (Desc.IsStr()) {
	//	TStrV ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV[RecN] = Store->GetFieldStr(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsStrV::New(JsRecSet->Js, ColV));
	//}

	//else if (Desc.IsBool()) {
	//	TIntV ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV[RecN] = (int)Store->GetFieldBool(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsIntV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsFlt()) {
	//	TFltV ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV[RecN] = Store->GetFieldFlt(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsTm()) {
	//	TFltV ColV(Recs);
	//	TTm Tm;
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		Store->GetFieldTm(RecSet()->GetRecId(RecN), FieldId, Tm);
	//		ColV[RecN] = (double)TTm::GetMSecsFromTm(Tm);
	//	}
	//	return HandleScope.Close(TJsFltV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsFltV()) {
	//	throw TQmExcept::New("rs.getVec does not support type float_v - use store.getMat instead");
	//}
	//else if (Desc.IsNumSpV()) {
	//	throw TQmExcept::New("rs.getVec does not support type num_sp_v - use store.getMat instead");
	//}
	//throw TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::getMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Args);
	//PRecSet RecSet = JsRecSet->RecSet;
	//TWPt<TStore> Store = RecSet->GetStore();
	//const TStr FieldNm = TJsRecSetUtil::GetArgStr(Args, 0);
	//const int FieldId = JsRecSet->RecSet->GetStore()->GetFieldId(FieldNm);
	//int Recs = (int)JsRecSet->RecSet->GetRecs();
	//const TFieldDesc& Desc = Store->GetFieldDesc(FieldId);

	//if (Desc.IsInt()) {
	//	TFltVV ColV(1, Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV(0, RecN) = (double)Store->GetFieldInt(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsUInt64()) {
	//	TFltVV ColV(1, Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV(0, RecN) = (double)Store->GetFieldUInt64(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsBool()) {
	//	TFltVV ColV(1, Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV(0, RecN) = (double)Store->GetFieldBool(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsFlt()) {
	//	TFltVV ColV(1, Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		ColV(0, RecN) = Store->GetFieldFlt(RecSet()->GetRecId(RecN), FieldId);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsTm()) {
	//	TFltVV ColV(1, Recs);
	//	TTm Tm;
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		Store->GetFieldTm(RecSet()->GetRecId(RecN), FieldId, Tm);
	//		ColV(0, RecN) = (double)TTm::GetMSecsFromTm(Tm);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsFltV()) {
	//	TFltV Vec;
	//	Store->GetFieldFltV(RecSet()->GetRecId(0), FieldId, Vec);
	//	TFltVV ColV(Vec.Len(), Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		Store->GetFieldFltV(RecSet()->GetRecId(RecN), FieldId, Vec);
	//		QmAssertR(Vec.Len() == ColV.GetRows(), TStr::Fmt("store.getCol for field type fltvec: dimensions are not consistent! %d expected, %d found in row %d", ColV.GetRows(), Vec.Len(), RecN));
	//		ColV.SetCol(RecN, Vec);
	//	}
	//	return HandleScope.Close(TJsFltVV::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsNumSpV()) {
	//	TVec<TIntFltKdV> ColV(Recs);
	//	for (int RecN = 0; RecN < Recs; RecN++) {
	//		Store->GetFieldNumSpV(RecSet()->GetRecId(RecN), FieldId, ColV[RecN]);
	//	}
	//	return HandleScope.Close(TJsSpMat::New(JsRecSet->Js, ColV));
	//}
	//else if (Desc.IsStr()) {
	//	throw TQmExcept::New("store.getMat does not support type string - use store.getVec instead");
	//}
	//throw TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	//const TWPt<TStore>& Store = JsRecSet->RecSet->GetStore();
	//return HandleScope.Close(TJsStore::New(JsRecSet->Js, Store));
}

void TNodeJsRecSet::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	//v8::Local<v8::Integer> Recs = v8::Integer::New(JsRecSet->RecSet->GetRecs());
	//return HandleScope.Close(Recs);
}

void TNodeJsRecSet::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	//v8::Handle<v8::Boolean> IsEmpty = v8::Boolean::New(JsRecSet->RecSet->Empty());
	//return HandleScope.Close(IsEmpty);
}

void TNodeJsRecSet::weighted(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	//v8::HandleScope HandleScope;
	//TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
	//v8::Handle<v8::Boolean> IsWgt = v8::Boolean::New(JsRecSet->RecSet->IsWgt());
	//return HandleScope.Close(IsWgt);
}

// TODO figure out the indexed properties
//void TNodeJsRecSet::indexId(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
//	v8::HandleScope HandleScope;
//	TJsRecSet* JsRecSet = TJsRecSetUtil::GetSelf(Info);
//	const int RecN = Index;
//	if (0 <= RecN && RecN < JsRecSet->RecSet->GetRecs()) {
//		const uint64 RecId = JsRecSet->RecSet->GetRecId(RecN);
//		if (JsRecSet->Store->IsRecId(RecId)) {
//			return TJsRec::New(JsRecSet->Js, JsRecSet->RecSet->GetRec(RecN), JsRecSet->RecSet->GetRecFq(RecN));
//		}
//		else {
//			return HandleScope.Close(v8::Null());
//		}
//	}
//	else {
//		return HandleScope.Close(v8::Null());
//	}
//}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
   TNodeJsQm::Init(exports);
   TNodeJsBase::Init(exports);   
   TNodeJsStore::Init(exports);
   TNodeJsRec::Init(exports);
   TNodeJsRecSet::Init(exports);
   // LA
   TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
   TNodeJsVec<TInt, TAuxIntV>::Init(exports);
   TNodeJsFltVV::Init(exports);
   TNodeJsSpVec::Init(exports);
   TNodeJsSpMat::Init(exports);
}

NODE_MODULE(qm, init)


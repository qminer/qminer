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
// NodeJs QMiner

THash<TStr, TUInt> TNodeJsQm::BaseFPathToId;

void TNodeJsQm::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// Add all methods, getters and setters here.
	NODE_SET_METHOD(exports, "config", _config);
	NODE_SET_METHOD(exports, "create", _create);
	NODE_SET_METHOD(exports, "open", _open);
	NODE_SET_METHOD(exports, "verbosity", _verbosity);

    // Add properties
    exports->SetAccessor(v8::String::NewFromUtf8(Isolate, "debug"), _debug);
    
    // initialize QMiner environment
	TQm::TEnv::Init();
    // default to no output
    TQm::TEnv::InitLogger(0, "std");
}

void TNodeJsQm::config(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    printf("DEPRECATED: consider using `new qm.Base(...)`!\n");
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
}

void TNodeJsQm::create(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    printf("DEPRECATED: consider using `new qm.Base(...)`!\n");
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get schema and conf
	TStr ConfFNm = TNodeJsUtil::GetArgStr(Args, 0, "qm.conf");
	TStr SchemaFNm = TNodeJsUtil::GetArgStr(Args, 1, "");
	TBool Clear = TNodeJsUtil::GetArgBool(Args, 2, false);

	// parse configuration file
	TQmParam Param(ConfFNm);
	TStr FPath = Param.DbFPath;
	if (TDir::Exists(FPath)) {
		TStrV FNmV;
		TStrV FExtV;
		TFFile::GetFNmV(FPath, FExtV, true, FNmV);
		bool DirEmpty = FNmV.Len() == 0;
		if (!DirEmpty && !Clear) {
			// if not empty and (clear == false) throw exception
			throw TQm::TQmExcept::New("qm.create: database folder not empty and clear is set to false!");
		}
		else if (!DirEmpty && Clear) {
			// else delete all files
			for (int FileN = 0; FileN < FNmV.Len(); FileN++) {
				TFile::Del(FNmV[FileN], true);
			}
		}

	}

	// prepare lock
	TFileLock Lock(Param.LockFNm);

	Lock.Lock();
	{
		// parse schema (if no given, create an empty array)
		PJsonVal SchemaVal = SchemaFNm.Empty() ? TJsonVal::NewArr() :
			TJsonVal::GetValFromStr(TStr::LoadTxt(SchemaFNm));
		// initialize base		
		TWPt<TQm::TBase> Base_ = TQm::TStorage::NewBase(Param.DbFPath, SchemaVal, Param.IndexCacheSize, Param.DefStoreCacheSize);
		// save base		
		TQm::TStorage::SaveBase(Base_);

		Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsBase>(new TNodeJsBase(Base_)));
		// once the base is open we need to setup the custom record templates for each store
		if (!TNodeJsQm::BaseFPathToId.IsKey(Base_->GetFPath())) {
			TUInt Keys = (uint)TNodeJsQm::BaseFPathToId.Len();
			TNodeJsQm::BaseFPathToId.AddDat(Base_->GetFPath(), Keys);
		}
		if (Clear) {
			// TODO simplify TNodeJsRec template selection:
            // see comment in v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq)
			// Since contents of db folder were not empty we must reset all record templates (one per store)
			uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(FPath);
			TNodeJsRec::Clear(BaseId);
		}
		for (int StoreN = 0; StoreN < Base_->GetStores(); StoreN++) {
			TNodeJsRec::Init(Base_->GetStoreByStoreN(StoreN));
		}
	}
	// remove lock
	Lock.Unlock();
}

void TNodeJsQm::open(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    printf("DEPRECATED: consider using `new qm.Base(...)`!\n");
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
		TWPt<TQm::TBase> Base_ = TQm::TStorage::LoadBase(Param.DbFPath, FAccess,
			Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);
		Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsBase>(new TNodeJsBase(Base_)));
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

void TNodeJsQm::verbosity(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// get verbosity level
	const int Verbosity = TNodeJsUtil::GetArgInt32(Args, 0, 0);
    // set verbosity level
    TQm::TEnv::InitLogger(Verbosity, "std");
}

void TNodeJsQm::debug(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
    #ifdef NDEBUG
        Info.GetReturnValue().Set(v8::Boolean::New(Isolate, false));
    #else
        Info.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
    #endif
}

///////////////////////////////
// NodeJs QMiner Base
v8::Persistent<v8::Function> TNodeJsBase::Constructor;
const TStr TNodeJsBase::ClassId = "Base";

void TNodeJsBase::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// template for creating function from javascript using "new", uses _NewJs callback
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsBase>);
	// child will have the same properties and methods, but a different callback: _NewCpp
	v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsBase>);
	child->Inherit(tpl);

	child->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	child->InstanceTemplate()->SetInternalFieldCount(1);

	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.   
	NODE_SET_PROTOTYPE_METHOD(tpl, "close", _close);
	NODE_SET_PROTOTYPE_METHOD(tpl, "store", _store);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStoreList", _getStoreList);
	NODE_SET_PROTOTYPE_METHOD(tpl, "createStore", _createStore);
	NODE_SET_PROTOTYPE_METHOD(tpl, "search", _search);
	NODE_SET_PROTOTYPE_METHOD(tpl, "garbageCollect", _garbageCollect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "partialFlush", _partialFlush);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStats", _getStats);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", _getStreamAggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript	
	// Constructor is used when creating the object from C++
	Constructor.Reset(Isolate, child->GetFunction());
	// we need to export the class for calling using "new FIn(...)"
	exports->Set(v8::String::NewFromUtf8(Isolate, ClassId.CStr()),
		tpl->GetFunction());

}

TNodeJsBase::TNodeJsBase(const TStr& DbFPath_, const TStr& SchemaFNm, const PJsonVal& Schema,
        const bool& Create, const bool& ForceCreate, const bool& RdOnlyP,
        const uint64& IndexCacheSize, const uint64& StoreCacheSize) {
    
    Watcher = TNodeJsBaseWatcher::New();

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TStr DbFPath = DbFPath_;
	if (!(DbFPath.LastCh() == '/' || DbFPath.LastCh() == '\\')) { DbFPath += "/"; }
	
	TStr LockFNm = DbFPath + "lock";
	
	// clean folder and lock
	if (ForceCreate) {
		if (TFile::Exists(LockFNm)) {
			TFile::Del(LockFNm, false);
		}
		if (TDir::Exists(DbFPath)) {
			TStrV FNmV;
			TStrV FExtV;
			TFFile::GetFNmV(DbFPath, FExtV, true, FNmV);
			bool DirEmpty = FNmV.Len() == 0;
			if (!DirEmpty) {
				// delete all files
				for (int FileN = 0; FileN < FNmV.Len(); FileN++) {
					TFile::Del(FNmV[FileN], true);
				}
			}
		}
	}
	if (Create) {
		if (TDir::Exists(DbFPath)) {
			TStrV FNmV;
			TStrV FExtV;
			TFFile::GetFNmV(DbFPath, FExtV, true, FNmV);
			bool DirEmpty = FNmV.Len() == 0;
			if (!DirEmpty) {
				// if not empty and create was called
				throw TQm::TQmExcept::New("new base(...): database folder not empty "
                    "and mode=create. Clear db folder or use mode=createClean!");
			}
		} else {
			TDir::GenDir(DbFPath);
		}

		// prepare lock
		TFileLock Lock(LockFNm);

		Lock.Lock();

		// use file if specified, otherwise use schema (default is empty)
		PJsonVal SchemaVal = SchemaFNm.Empty() ? Schema :
			TJsonVal::GetValFromStr(TStr::LoadTxt(SchemaFNm));
		// initialize base		

		Base = TQm::TStorage::NewBase(DbFPath, SchemaVal, IndexCacheSize, StoreCacheSize);
		// save base		
		TQm::TStorage::SaveBase(Base);

		// once the base is open we need to setup the custom record templates for each store
		if (!TNodeJsQm::BaseFPathToId.IsKey(Base->GetFPath())) {
			TUInt Keys = (uint)TNodeJsQm::BaseFPathToId.Len();
			TNodeJsQm::BaseFPathToId.AddDat(Base->GetFPath(), Keys);
		}
		if (ForceCreate) {
			// TODO simplify TNodeJsRec template selection:
            // see comment in v8::Local<v8::Object> TNodeJsRec::New(const TQm::TRec& Rec, const TInt& _Fq)
			// Since contents of db folder were not empty we must reset all record templates (one per store)
			uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(DbFPath);
			TNodeJsRec::Clear(BaseId);
		}
		for (int StoreN = 0; StoreN < Base->GetStores(); StoreN++) {
			TNodeJsRec::Init(Base->GetStoreByStoreN(StoreN));
		}

		// remove lock
		Lock.Unlock();
	}
	if (!Create) { // open mode
		// prepare lock
		TFileLock Lock(LockFNm);

		Lock.Lock();
		// load database and start the server
		{
			// resolve access type
			TFAccess FAccess = RdOnlyP ? faRdOnly : faUpdate;
			// load base
			TStrUInt64H StoreNmCacheSizeH;
			Base = TQm::TStorage::LoadBase(DbFPath, FAccess,
				IndexCacheSize, StoreCacheSize, StoreNmCacheSizeH);
			// once the base is open we need to setup the custom record templates for each store
			if (!TNodeJsQm::BaseFPathToId.IsKey(Base->GetFPath())) {
				TUInt Keys = (uint)TNodeJsQm::BaseFPathToId.Len();
				TNodeJsQm::BaseFPathToId.AddDat(Base->GetFPath(), Keys);
			}
			for (int StoreN = 0; StoreN < Base->GetStores(); StoreN++) {
				TNodeJsRec::Init(Base->GetStoreByStoreN(StoreN));
			}
		}
		// remove lock
		Lock.Unlock();
	}
}

TNodeJsBase* TNodeJsBase::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	// parse arguments
	EAssertR(Args.Length() == 1 && Args[0]->IsObject(), "base constructor expects a JSON object with parameters");

	PJsonVal Val = TNodeJsUtil::GetArgJson(Args, 0);

	TStr DbPath = Val->GetObjStr("dbPath", "./db/");
	// mode: create, createClean, open, openReadOnly
	TStr Mode = Val->GetObjStr("mode", "openReadOnly");

	TStr SchemaFNm = Val->GetObjStr("schemaPath", "");
	PJsonVal Schema = TJsonVal::NewArr();
	if (Val->IsObjKey("schema")) {
		Schema = Val->GetObjKey("schema");
	}

	bool Create = ((Mode == "create") || (Mode == "createClean"));
	bool ForceCreate = (Mode == "createClean");
	bool ReadOnly = (Mode == "openReadOnly");
	uint64 IndexCache = (uint64)Val->GetObjInt("indexCache", 1024) * (uint64)TInt::Mega;
	uint64 StoreCache = (uint64)Val->GetObjInt("storeCache", 1024) * (uint64)TInt::Mega;

	TStr UnicodeFNm = Val->GetObjStr("unicode", TQm::TEnv::QMinerFPath + "./UnicodeDef.Bin");
	if (!TUnicodeDef::IsDef()) { TUnicodeDef::Load(UnicodeFNm); }
	// Load Stopword Files
	TStr StopWordsPath = Val->GetObjStr("stopwords", TQm::TEnv::QMinerFPath + "resources/stopwords/");
	TSwSet::LoadSwDir(StopWordsPath);

	return new TNodeJsBase(DbPath, SchemaFNm, Schema, Create, ForceCreate, ReadOnly, IndexCache, StoreCache);
}

void TNodeJsBase::close(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());

	JsBase->Watcher->Close();

	if (!JsBase->Base.Empty()) {
		// save base
		TQm::TStorage::SaveBase(JsBase->Base);
		JsBase->Base.Del();
		JsBase->Base.Clr();
	}
}

void TNodeJsBase::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	const TStr StoreNm = TNodeJsUtil::GetArgStr(Args, 0);
	if (Base->IsStoreNm(StoreNm)) {
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsStore>(
			new TNodeJsStore(Base->GetStoreByStoreNm(StoreNm), JsBase->Watcher)));
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
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
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
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
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
		Args.GetReturnValue().Set(
			TNodeJsUtil::NewInstance<TNodeJsStore>(
			new TNodeJsStore(NewStoreV[0], JsBase->Watcher)));
	}
	else if (NewStoreV.Len() > 1) {
		v8::Local<v8::Array> JsNewStoreV = v8::Array::New(Isolate, NewStoreV.Len());
		for (int NewStoreN = 0; NewStoreN < NewStoreV.Len(); NewStoreN++) {
			JsNewStoreV->Set(v8::Number::New(Isolate, NewStoreN),
				TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(NewStoreV[NewStoreN], JsBase->Watcher))
				);
		}
	}
	else {
		Args.GetReturnValue().Set(v8::Null(Isolate));
	}
}

void TNodeJsBase::search(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	PJsonVal QueryVal = TNodeJsUtil::GetArgJson(Args, 0);
	// execute the query
	TQm::PRecSet RecSet = JsBase->Base->Search(QueryVal);
	// return results
	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet, JsBase->Watcher)));
}

void TNodeJsBase::garbageCollect(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	Base->GarbageCollect();
	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsBase::partialFlush(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;
	
	const TInt WndInMesc = TNodeJsUtil::GetArgInt32(Args, 0, 500);

	int res = Base->PartialFlush(WndInMesc);
	Args.GetReturnValue().Set(v8::Integer::New(Isolate, res));
}

void TNodeJsBase::getStats(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	PJsonVal res = Base->GetStats();
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, res));
}

void TNodeJsBase::getStreamAggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());

	const TStr AggrNm = TNodeJsUtil::GetArgStr(Args, 0);
	if (JsBase->Base->IsStreamAggr(AggrNm)) {
		TQm::PStreamAggr StreamAggr = JsBase->Base->GetStreamAggr(AggrNm);
		Args.GetReturnValue().Set(TNodeJsSA::New(StreamAggr));
	}
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
	TWPt<TQm::TBase> Base = JsBase->Base;

	TQm::PStreamAggrBase SABase = JsBase->Base->GetStreamAggrBase();
	int AggrId = SABase->GetFirstStreamAggrId();
	v8::Local<v8::Array> Arr = v8::Array::New(Isolate);
	uint32 Counter = 0;
	while (SABase->GetNextStreamAggrId(AggrId)) {
		v8::Local<v8::String> AggrNm = v8::String::NewFromUtf8(
            Isolate, SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
		Arr->Set(Counter, AggrNm);
		Counter++;
	}
	Args.GetReturnValue().Set(Arr);
}


///////////////////////////////
// NodeJs QMiner Record
v8::Persistent<v8::Function> TNodeJsStore::Constructor;
const TStr TNodeJsStore::ClassId = "Store";

void TNodeJsStore::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStore>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "recordByName", _recordByName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "each", _each);
	NODE_SET_PROTOTYPE_METHOD(tpl, "map", _map);
	NODE_SET_PROTOTYPE_METHOD(tpl, "push", _push);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRecord", _newRecord);
	NODE_SET_PROTOTYPE_METHOD(tpl, "newRecordSet", _newRecordSet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sample", _sample);
	NODE_SET_PROTOTYPE_METHOD(tpl, "field", _field);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isNumeric", _isNumeric);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isString", _isString);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isDate", _isDate);
	NODE_SET_PROTOTYPE_METHOD(tpl, "key", _key);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", _getStreamAggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clear", _clear);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getVector", _getVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMatrix", _getMatrix);
	NODE_SET_PROTOTYPE_METHOD(tpl, "cell", _cell);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "empty"), _empty);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "allRecords"), _allRecords);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "fields"), _fields);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "joins"), _joins);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "keys"), _keys);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "first"), _first);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "last"), _last);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "forwardIter"), _forwardIter);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "backwardIter"), _backwardIter);
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "base"), _base);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	Constructor.Reset(Isolate, tpl->GetFunction());
	// So we can add stuff to the prototype in JS
	exports->Set(v8::String::NewFromUtf8(Isolate, ClassId.CStr()), tpl->GetFunction());
}

v8::Local<v8::Value> TNodeJsStore::Field(const TQm::TRec& Rec, const int FieldId) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	// check if field is null
	if (Rec.IsFieldNull(FieldId)) {
		return v8::Null(Isolate);
	}
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
			// milliseconds from 1601-01-01T00:00:00Z
			double WinMSecs = (double)TTm::GetMSecsFromTm(FieldTm);
			// milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
			double UnixMSecs = (double)TNodeJsUtil::GetJsTimestamp((uint64)WinMSecs);
			return HandleScope.Escape(v8::Date::New(Isolate, UnixMSecs));
		}
		else {
			return v8::Null(Isolate);
		}
	}
	else if (Desc.IsNumSpV()) {
		TIntFltKdV SpV; Rec.GetFieldNumSpV(FieldId, SpV);
		return HandleScope.Escape(TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(SpV)));
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
	TQm::TRec Rec = Store->GetRec(RecId);
	return HandleScope.Escape(Field(Rec, FieldId));
}

void TNodeJsStore::recordByName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

		const TStr RecNm = TNodeJsUtil::GetArgStr(Args, 0);
		if (JsStore->Store->IsRecNm(RecNm)) {
			Args.GetReturnValue().Set(TNodeJsRec::NewInstance(
                new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(RecNm))));
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


	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");
	Args.GetReturnValue().Set(Args.Holder());

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
	const TWPt<TQm::TStore> Store = JsStore->Store;
	
	if (!Store->Empty()) {
		TQm::PStoreIter Iter = Store->ForwardIter();

		QmAssert(Iter->Next());
		uint32_t Count = 0;
		uint64 RecId = Iter->GetRecId();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::NewInstance(
			new TNodeJsRec(JsStore->Watcher, Store->GetRec(RecId)));
		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(RecObj);

		do {
			JsRec->Rec = Store->GetRec(Iter->GetRecId());
			v8::Local<v8::Value> ArgV[Argc] = {
				RecObj,
				v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count++))
			};
			v8::TryCatch try_catch;
			Callback->Call(Isolate->GetCurrentContext()->Global(), Argc, ArgV);
			if (try_catch.HasCaught()) {
				try_catch.ReThrow();				
				return;				
			}
		} while (Iter->Next());
	}
}

void TNodeJsStore::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

	const TWPt<TQm::TStore> Store = JsStore->Store;
	const int Recs = (int)Store->GetRecs();

	v8::Handle<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

	if (!Store->Empty()) {
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();

		TQm::PStoreIter Iter = Store->ForwardIter();

		QmAssert(Iter->Next());
		uint32_t Count = 0;
		uint64 RecId = Iter->GetRecId();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::NewInstance(
			new TNodeJsRec(JsStore->Watcher, Store->GetRec(RecId)));
		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(RecObj);

		do {
			JsRec->Rec = Store->GetRec(Iter->GetRecId());
			v8::Local<v8::Value> ArgV[Argc] = {
				RecObj,
				v8::Local<v8::Number>::New(Isolate, v8::Integer::NewFromUnsigned(Isolate, Count))
			};
			v8::TryCatch try_catch;
			v8::Local<v8::Value> ReturnVal = Callback->Call(GlobalContext, Argc, ArgV);
			if (try_catch.HasCaught()) {
				try_catch.ReThrow();
				return;
			}
			ResultV->Set(Count, ReturnVal);
			Count++;
		} while (Iter->Next());
	}

	Args.GetReturnValue().Set(ResultV);
}

void TNodeJsStore::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;
		TWPt<TQm::TBase> Base = JsStore->Store->GetBase();

		// check we can write
		QmAssertR(!Base->IsRdOnly(), "Base opened as read-only");
    
		PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
		const uint64 RecId = Store->AddRec(RecVal);

		Args.GetReturnValue().Set(v8::Integer::NewFromUnsigned(Isolate, (uint32_t)RecId));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::newRecord(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
		TQm::TRec Rec(Store, RecVal);

		Args.GetReturnValue().Set(
			TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, Rec))
			);

	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::newRecordSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
	TWPt<TQm::TStore> Store = JsStore->Store;

	if (Args.Length() > 0) {
		// argument 0 = TJsIntV of record ids
		QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, "TIntV"),
			"Store.getRecSetByIdV: The first argument must be a TIntV (js linalg full int vector)");
		TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
		TQm::PRecSet ResultSet = TQm::TRecSet::New(Store, JsVecArg->Vec);
		Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(ResultSet, JsStore->Watcher)));
		return;
	}
	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(
        new TNodeJsRecSet(TQm::TRecSet::New(Store), JsStore->Watcher)));
}

void TNodeJsStore::sample(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 0);
		QmAssertR(SampleSize >= 0, "Store.sample: sample size should be nonnegative!");
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(
            new TNodeJsRecSet(JsStore->Store->GetRndRecs(SampleSize), JsStore->Watcher)));
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

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

		if (JsStore->Store->IsFieldNm(FieldNm)) {
			const int FieldId = JsStore->Store->GetFieldId(FieldNm);
			const TQm::TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);

			v8::Local<v8::Object> Field = v8::Object::New(Isolate);
			Field->Set(v8::String::NewFromUtf8(Isolate, "id"),
                v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "name"),
                v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldNm().CStr()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "type"),
                v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldTypeStr().CStr()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "nullable"),
                v8::Boolean::New(Isolate, FieldDesc.IsNullable()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "internal"),
                v8::Boolean::New(Isolate, FieldDesc.IsInternal()));
			Field->Set(v8::String::NewFromUtf8(Isolate, "primary"),
                v8::Boolean::New(Isolate, FieldDesc.IsPrimary()));

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

void TNodeJsStore::isNumeric(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore>& Store = JsStore->Store;

		EAssertR(Store->IsFieldNm(FldNm), "store.isNumeric: Invalid field name: " + FldNm);

		const int FldId = JsStore->Store->GetFieldId(FldNm);
		const TQm::TFieldDesc& FldDesc = Store->GetFieldDesc(FldId);

		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, FldDesc.IsFlt() || FldDesc.IsInt() || FldDesc.IsUInt64()));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::isString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore>& Store = JsStore->Store;

		EAssertR(Store->IsFieldNm(FldNm), "store.isString: Invalid field name: " + FldNm);

		const int FldId = JsStore->Store->GetFieldId(FldNm);
		const TQm::TFieldDesc& FldDesc = Store->GetFieldDesc(FldId);

		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, FldDesc.IsStr()));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::isDate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore>& Store = JsStore->Store;

		EAssertR(Store->IsFieldNm(FldNm), "store.isString: Invalid field name: " + FldNm);

		const int FldId = JsStore->Store->GetFieldId(FldNm);
		const TQm::TFieldDesc& FldDesc = Store->GetFieldDesc(FldId);

		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, FldDesc.IsTm()));
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

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();

		if (IndexVoc->IsKeyNm(JsStore->Store->GetStoreId(), KeyNm)) {
			TQm::TIndexKey Key = IndexVoc->GetKey(IndexVoc->GetKeyId(JsStore->Store->GetStoreId(), KeyNm));
			Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsIndexKey>(
                new TNodeJsIndexKey(JsStore->Store, Key, JsStore->Watcher)));
		}
		else {
			Args.GetReturnValue().Set(v8::Null(Isolate));
		}
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

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

		TWPt<TQm::TBase> Base = JsStore->Store->GetBase();
		const uint StoreId = JsStore->Store->GetStoreId();

		if (Base->IsStreamAggr(StoreId, AggrNm)) {
			TQm::PStreamAggr StreamAggr = Base->GetStreamAggr(StoreId, AggrNm);
			Args.GetReturnValue().Set(TNodeJsSA::New(StreamAggr));
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
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

		TWPt<TQm::TStore>& Store = JsStore->Store;
		const TWPt<TQm::TBase>& Base = JsStore->Store->GetBase();

		TQm::PStreamAggrBase SABase = Base->GetStreamAggrBase(Store->GetStoreId());
		int AggrId = SABase->GetFirstStreamAggrId();

		v8::Local<v8::Array> Arr = v8::Array::New(Isolate);
		uint32 Counter = 0;
		while (SABase->GetNextStreamAggrId(AggrId)) {
			v8::Local<v8::String> AggrNm = v8::String::NewFromUtf8(Isolate,
                SABase->GetStreamAggr(AggrId)->GetAggrNm().CStr());
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
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
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
		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		const int DelRecs = TNodeJsUtil::GetArgInt32(Args, 0, (int)JsStore->Store->GetRecs());

		JsStore->Store->DeleteFirstNRecs(DelRecs);
		Args.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
		return;
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		if (!Store->IsFieldNm(FieldNm)) {
			throw TQm::TQmExcept::New("store.getVector: fieldName not found: " + FieldNm);
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
				ColV[RecN] = (double)TNodeJsUtil::GetJsTimestamp(TTm::GetMSecsFromTm(Tm));
				Iter->Next();
			}
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
			return;
		}
		else if (Desc.IsFltV()) {
			throw TQm::TQmExcept::New("store.getVector does not support type float_v - use store.getMatrix instead");
		}
		else if (Desc.IsNumSpV()) {
			throw TQm::TQmExcept::New("store.getVector does not support type num_sp_v - use store.getMatrix instead");
		}
		throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::getMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);

		TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
		TWPt<TQm::TStore> Store = JsStore->Store;

		if (!Store->IsFieldNm(FieldNm)) {
			throw TQm::TQmExcept::New("store.getMatrix: fieldName not found: " + FieldNm);
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
				QmAssertR(Vec.Len() == ColV.GetRows(), TStr::Fmt("store.getCol for field type fltvec: "
                    "dimensions are not consistent! %d expected, %d found in row %d", ColV.GetRows(), Vec.Len(), RecN));
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
			throw TQm::TQmExcept::New("store.getMatrix does not support type string - use store.getVector instead");
		}
		throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
	}
}

void TNodeJsStore::cell(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
	TWPt<TQm::TStore> Store = JsStore->Store;
	QmAssertR(Args.Length() == 2, "Store.cell: two arguments expected");
	TInt RecId = TNodeJsUtil::GetArgInt32(Args, 0);
	TInt FieldId = -1;
	if (Args.Length() == 2 && Args[1]->IsInt32()) {
		FieldId = TNodeJsUtil::GetArgInt32(Args, 1);
		QmAssertR(Store->IsFieldId(FieldId), "Store.cell: field with fieldId not found");
	}
	else if (Args.Length() == 2 && Args[1]->IsString()) {
		TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 1);
		QmAssertR(Store->IsFieldNm(FieldNm), "Store.cell: field with fieldName not found");
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
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsStore->Store->GetStoreNm().CStr()));
}

void TNodeJsStore::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStore->Store->Empty()));
}

void TNodeJsStore::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
}

void TNodeJsStore::allRecords(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	TQm::PRecSet ResultSet = JsStore->Store->GetAllRecs();
	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(ResultSet, JsStore->Watcher)));
}

void TNodeJsStore::fields(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	v8::Local<v8::Array> FieldV = v8::Array::New(Isolate, JsStore->Store->GetFields());
	for (int FieldId = 0; FieldId < JsStore->Store->GetFields(); FieldId++) {
		const TQm::TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);
		v8::Local<v8::Object> Field = v8::Object::New(Isolate);
		Field->Set(v8::String::NewFromUtf8(Isolate, "id"),
            v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "name"),
            v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldNm().CStr()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "type"),
            v8::String::NewFromUtf8(Isolate, FieldDesc.GetFieldTypeStr().CStr()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "nullable"),
            v8::Boolean::New(Isolate, FieldDesc.IsNullable()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "internal"),
            v8::Boolean::New(Isolate, FieldDesc.IsInternal()));
		Field->Set(v8::String::NewFromUtf8(Isolate, "primary"),
            v8::Boolean::New(Isolate, FieldDesc.IsPrimary()));
		FieldV->Set(v8::Number::New(Isolate, FieldId), Field);
	}
	Info.GetReturnValue().Set(FieldV);
}

void TNodeJsStore::joins(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	v8::Local<v8::Array> JoinV = v8::Array::New(Isolate, JsStore->Store->GetJoins());
	for (int JoinId = 0; JoinId < JsStore->Store->GetJoins(); JoinId++) {
		const TQm::TJoinDesc& JoinDesc = JsStore->Store->GetJoinDesc(JoinId);
		TWPt<TQm::TStore> JoinStore = JoinDesc.GetJoinStore(JsStore->Store->GetBase());
		v8::Local<v8::Object> Join = v8::Object::New(Isolate);
		Join->Set(v8::String::NewFromUtf8(Isolate, "id"),
			v8::Int32::New(Isolate, JoinDesc.GetJoinId()));
		Join->Set(v8::String::NewFromUtf8(Isolate, "name"),
			v8::String::NewFromUtf8(Isolate, JoinDesc.GetJoinNm().CStr()));
		Join->Set(v8::String::NewFromUtf8(Isolate, "store"),
			v8::String::NewFromUtf8(Isolate, JoinStore->GetStoreNm().CStr()));
		if (JoinDesc.IsInverseJoinId()) {
			Join->Set(v8::String::NewFromUtf8(Isolate, "inverse"),
				v8::String::NewFromUtf8(Isolate, JoinStore->GetJoinNm(JoinDesc.GetInverseJoinId()).CStr()));
		}
		if (JoinDesc.IsFieldJoin()) {
			Join->Set(v8::String::NewFromUtf8(Isolate, "type"),
				v8::String::NewFromUtf8(Isolate, "field"));
			Join->Set(v8::String::NewFromUtf8(Isolate, "recordField"),
				v8::String::NewFromUtf8(Isolate, JsStore->Store->GetFieldNm(JoinDesc.GetJoinRecFieldId()).CStr()));
			Join->Set(v8::String::NewFromUtf8(Isolate, "weightField"),
				v8::String::NewFromUtf8(Isolate, JsStore->Store->GetFieldNm(JoinDesc.GetJoinFqFieldId()).CStr()));
		}
		else if (JoinDesc.IsIndexJoin()) {
			Join->Set(v8::String::NewFromUtf8(Isolate, "type"),
				v8::String::NewFromUtf8(Isolate, "index"));
			TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
			Join->Set(v8::String::NewFromUtf8(Isolate, "key"),
				TNodeJsUtil::NewInstance<TNodeJsIndexKey>(new TNodeJsIndexKey(
                JsStore->Store, IndexVoc->GetKey(JoinDesc.GetJoinKeyId()), JsStore->Watcher)));
		}
		JoinV->Set(v8::Number::New(Isolate, JoinId), Join);
	}
	Info.GetReturnValue().Set(JoinV);
}

void TNodeJsStore::keys(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
	TIntSet KeySet = IndexVoc->GetStoreKeys(JsStore->Store->GetStoreId());
	TIntV Vec;  KeySet.GetKeyV(Vec);
	v8::Local<v8::Array> KeyNmV = v8::Array::New(Isolate, KeySet.Len());
	for (int KeyN = 0; KeyN < Vec.Len(); KeyN++) {
		KeyNmV->Set(KeyN,
			TNodeJsUtil::NewInstance<TNodeJsIndexKey>(
                new TNodeJsIndexKey(JsStore->Store, IndexVoc->GetKey(Vec[KeyN]), JsStore->Watcher)));
	}
	Info.GetReturnValue().Set(KeyNmV);
}

void TNodeJsStore::first(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	const uint64 FirstRecId = JsStore->Store->FirstRecId();
	if (FirstRecId == TUInt64::Mx) {
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
	Info.GetReturnValue().Set(
		TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(FirstRecId)))
		);
}

void TNodeJsStore::last(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	const uint64 LastRecId = JsStore->Store->LastRecId();
	if (LastRecId == TUInt64::Mx) {
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
	Info.GetReturnValue().Set(
		TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(LastRecId)))
		);
}

void TNodeJsStore::forwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsStoreIter>(
		new TNodeJsStoreIter(JsStore->Store, JsStore->Store->ForwardIter(), JsStore->Watcher)));
}

void TNodeJsStore::backwardIter(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

	Info.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsStoreIter>(
		new TNodeJsStoreIter(JsStore->Store, JsStore->Store->BackwardIter(), JsStore->Watcher)));
}


void TNodeJsStore::indexId(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Info.Holder());

	if (JsStore->Store->IsRecId(Index)) {
		Info.GetReturnValue().Set(
			TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(Index)))
			);
		return;
	}

	Info.GetReturnValue().Set(v8::Null(Isolate));
}

void TNodeJsStore::base(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);
	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsBase>(new TNodeJsBase(JsStore->Store->GetBase())));

}

///////////////////////////////
// NodeJs QMiner Record
TVec<TVec<v8::Persistent<v8::Function> > > TNodeJsRec::BaseStoreIdConstructor;
const TStr TNodeJsRec::ClassId = "Rec";

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

		v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsRec>);
		tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
		// ObjectWrap uses the first internal field to store the wrapped pointer.
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Add all prototype methods, getters and setters here.
		NODE_SET_PROTOTYPE_METHOD(tpl, "$clone", _clone);
		NODE_SET_PROTOTYPE_METHOD(tpl, "$addJoin", _addJoin);
		NODE_SET_PROTOTYPE_METHOD(tpl, "$delJoin", _delJoin);
		NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);

		// Properties 
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$id"), _id);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$name"), _name);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$fq"), _fq);
		tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "$store"), _store);
		// register all the fields
		for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
			TStr FieldNm = Store->GetFieldDesc(FieldN).GetFieldNm();
			tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, FieldNm.CStr()), _getField, _setField);
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
	}
}

void TNodeJsRec::Clear(const int& BaseId) {
	if (BaseStoreIdConstructor.Len() > BaseId) {
		for (int StoreN = 0; StoreN < BaseStoreIdConstructor[BaseId].Len(); StoreN++) {
			if (BaseStoreIdConstructor[BaseId][StoreN].IsEmpty()) { break; }
			BaseStoreIdConstructor[BaseId][StoreN].Reset();
		}
	}
}

v8::Local<v8::Object> TNodeJsRec::NewInstance(TNodeJsRec* JsRec) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	TQm::TRec& Rec = JsRec->Rec;
	// TODO speed-up without using file paths
	// Use a map from (uint64)Rec.GetStore()() -> v8::Persistent<v8::Function> constructor
	// We need a hash table with move constructor/assignment
	QmAssertR(TNodeJsQm::BaseFPathToId.IsKey(Rec.GetStore()->GetBase()->GetFPath()),
        "TNodeJsRec::NewInstance: Base Id not found!");
	uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(Rec.GetStore()->GetBase()->GetFPath());
	EAssertR(!BaseStoreIdConstructor[BaseId][Rec.GetStoreId()].IsEmpty(),
        "TNodeJsRec::NewInstance: constructor is empty. Did you call TNodeJsRec::Init(exports)?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(
        Isolate, BaseStoreIdConstructor[BaseId][Rec.GetStoreId()]);
	v8::Local<v8::Object> Instance = cons->NewInstance();
	JsRec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

void TNodeJsRec::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args.Holder());
	Args.GetReturnValue().Set(
		TNodeJsRec::NewInstance(new TNodeJsRec(JsRec->Watcher, JsRec->Rec, JsRec->Fq)));
}

void TNodeJsRec::addJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args.Holder());

	// read argument as record
	QmAssertR(Args.Length() >= 2 && (TNodeJsUtil::IsArgClass(Args, 1, TNodeJsRec::ClassId) || Args[1]->IsInt32()),
        "rec.addJoin needs at least 2 args: JoinNm, (rec | recId) and fq (optional)");
		
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	
	const int JoinFq = TNodeJsUtil::GetArgInt32(Args, 2, 1);
	// check parameters fine
	QmAssertR(JsRec->Rec.GetStore()->IsJoinNm(JoinNm),
        "[addJoin] Unknown join " + JsRec->Rec.GetStore()->GetStoreNm() + "." + JoinNm);
	QmAssertR(JoinFq > 0, "[addJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
	// get generic store
	TWPt<TQm::TStore> Store = JsRec->Rec.GetStore();
	const int JoinId = Store->GetJoinId(JoinNm);
		
	if (Args[1]->IsInt32()) {
		int RecId = TNodeJsUtil::GetArgInt32(Args, 1);
		Store->AddJoin(JoinId, JsRec->Rec.GetRecId(), RecId, JoinFq);
	} else {
		TNodeJsRec* JsJoinRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[1]->ToObject());
		// add join
		Store->AddJoin(JoinId, JsRec->Rec.GetRecId(), JsJoinRec->Rec.GetRecId(), JoinFq);
	}
	
	// return
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRec::delJoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args.Holder());


	// read argument as record
	QmAssertR(Args.Length() >= 2 && (TNodeJsUtil::IsArgClass(Args, 1, TNodeJsRec::ClassId) || Args[1]->IsInt32()),
        "rec.delJoin needs at least 2 args: JoinNm, (rec | recId) and fq (optional)");
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	
	const int JoinFq = TNodeJsUtil::GetArgInt32(Args, 2, 1);
	// check parameters fine
	QmAssertR(JsRec->Rec.GetStore()->IsJoinNm(JoinNm),
        "[delJoin] Unknown join " + JsRec->Rec.GetStore()->GetStoreNm() + "." + JoinNm);
	QmAssertR(JoinFq > 0, "[delJoin] Join frequency must be positive: " + TInt::GetStr(JoinFq));
	// get generic store
	TWPt<TQm::TStore> Store = JsRec->Rec.GetStore();
	const int JoinId = Store->GetJoinId(JoinNm);
	
	if (Args[1]->IsInt32()) {
		int RecId = TNodeJsUtil::GetArgInt32(Args, 1);
		Store->DelJoin(JoinId, JsRec->Rec.GetRecId(), RecId, JoinFq);
	} else {
		TNodeJsRec* JsJoinRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[1]->ToObject());
		// add join
		Store->DelJoin(JoinId, JsRec->Rec.GetRecId(), JsJoinRec->Rec.GetRecId(), JoinFq);
	}	
	// return
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRec::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args.Holder());

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
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsRec->Rec.GetRecId()));
}

void TNodeJsRec::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsRec->Rec.GetRecNm().CStr()));
}

void TNodeJsRec::fq(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRec->Fq));
}

void TNodeJsRec::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsRec->Rec.GetStore(), JsRec->Watcher))
		);

}

void TNodeJsRec::getField(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
	const TQm::TRec& Rec = JsRec->Rec;
	const TWPt<TQm::TStore>& Store = Rec.GetStore();
	const int FieldId = Store->GetFieldId(TNodeJsUtil::GetStr(Name));

	Info.GetReturnValue().Set(TNodeJsStore::Field(Rec, FieldId));
}

void TNodeJsRec::setField(v8::Local<v8::String> Name, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<void>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
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
		if (Value->IsDate()){
			v8::Handle<v8::Date> Date = v8::Handle<v8::Date>::Cast(Value);
			// milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
			double UnixMSecs = Date->NumberValue();
			// milliseconds from 1601-01-01T00:00:00Z
			double WinMSecs = UnixMSecs + 11644473600000.0;
			TTm Tm = TTm::GetTmFromMSecs((uint64)WinMSecs);
			Rec.SetFieldTm(FieldId, Tm);
		}
		else if (Value->IsString()){
			v8::String::Utf8Value Utf8(Value);
			Rec.SetFieldTm(FieldId, TTm::GetTmFromWebLogDateTimeStr(TStr(*Utf8), '-', ':', '.', 'T'));
		}
		else {
			throw TQm::TQmExcept::New("Field + " + FieldNm + " expects a javascript Date() "
                "object or a Weblog datetime formatted string (example: \"2012-12-31T00:00:05.100\")");
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
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);

	TStr JoinNm = TNodeJsUtil::GetStr(Name);
	TQm::PRecSet RecSet = JsRec->Rec.DoJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet, JsRec->Watcher)));
}

void TNodeJsRec::sjoin(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);

	TStr JoinNm = TNodeJsUtil::GetStr(Name);
	TQm::TRec JoinRec = JsRec->Rec.DoSingleJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
	TWPt<TQm::TStore> JoinStore = JoinRec.GetStore();
	if (JoinRec.IsDef() && JoinStore->IsRecId(JoinRec.GetRecId())) {
		Info.GetReturnValue().Set(
			TNodeJsRec::NewInstance(new TNodeJsRec(JsRec->Watcher, JoinRec))
			);
		return;
	}
	else {
		Info.GetReturnValue().Set(v8::Null(Isolate));
		return;
	}
}

///////////////////////////////
// NodeJs QMiner Record Set
v8::Persistent<v8::Function> TNodeJsRecSet::Constructor;
const TStr TNodeJsRecSet::ClassId = "RecSet";

void TNodeJsRecSet::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsRecSet>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "clone", _clone);
	NODE_SET_PROTOTYPE_METHOD(tpl, "join", _join);
	NODE_SET_PROTOTYPE_METHOD(tpl, "aggr", _aggr);
	NODE_SET_PROTOTYPE_METHOD(tpl, "trunc", _trunc);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sample", _sample);
	NODE_SET_PROTOTYPE_METHOD(tpl, "shuffle", _shuffle);
	NODE_SET_PROTOTYPE_METHOD(tpl, "reverse", _reverse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortById", _sortById);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortByFq", _sortByFq);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortByField", _sortByField);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sort", sort);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterById", _filterById);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterByFq", _filterByFq);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filterByField", _filterByField);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filter", _filter);
	NODE_SET_PROTOTYPE_METHOD(tpl, "split", _split);
	NODE_SET_PROTOTYPE_METHOD(tpl, "deleteRecords", _deleteRecords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "each", _each);
	NODE_SET_PROTOTYPE_METHOD(tpl, "map", _map);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setIntersect", _setIntersect);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setUnion", _setUnion);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setDiff", _setDiff);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getVector", _getVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMatrix", _getMatrix);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "empty"), _empty);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weighted"), _weighted);
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	Constructor.Reset(Isolate, tpl->GetFunction());
	// So we can add stuff to the prototype in JS
	exports->Set(v8::String::NewFromUtf8(Isolate, ClassId.CStr()), tpl->GetFunction());
}

void TNodeJsRecSet::clone(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
	TQm::PRecSet RecSet = JsRecSet->RecSet->Clone();
	Args.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet, JsRecSet->Watcher)));

}

void TNodeJsRecSet::join(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
	TStr JoinNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 1, -1);
	TQm::PRecSet RecSet = JsRecSet->RecSet->DoJoin(JsRecSet->RecSet->GetStore()->GetBase(), JoinNm, SampleSize);
	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet, JsRecSet->Watcher)));
}

void TNodeJsRecSet::aggr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	const int SampleSize = TNodeJsUtil::GetArgInt32(Args, 0);
	TQm::PRecSet ResultSet = JsRecSet->RecSet->GetSampleRecSet(SampleSize, false);

	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(ResultSet, JsRecSet->Watcher)));
}

void TNodeJsRecSet::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
	const int RndSeed = TNodeJsUtil::GetArgInt32(Args, 0, 0);
	TRnd Rnd(RndSeed);
	JsRecSet->RecSet->Shuffle(Rnd);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::reverse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	JsRecSet->RecSet->Reverse();
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortById(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
	bool Asc = false;
	if (Args.Length() > 0) {
		QmAssertR(TNodeJsUtil::IsArgBool(Args, 0) || TNodeJsUtil::IsArgFlt(Args, 0),
			"TNodeJsRecSet::sortById: Argument 0 expected to be bool or int!");
		Asc = TNodeJsUtil::IsArgBool(Args, 0) ?
			TNodeJsUtil::GetArgBool(Args, 0) :
			TNodeJsUtil::GetArgFlt(Args, 0) > 0;
	}
	JsRecSet->RecSet->SortById(Asc);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortByFq(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	bool Asc = false;
	if (Args.Length() > 0) {
		QmAssertR(TNodeJsUtil::IsArgBool(Args, 0) || TNodeJsUtil::IsArgFlt(Args, 0),
			"TNodeJsRecSet::sortByFq: Argument 0 expected to be bool or int!");
		Asc = TNodeJsUtil::IsArgBool(Args, 0) ?
			TNodeJsUtil::GetArgBool(Args, 0) :
			TNodeJsUtil::GetArgFlt(Args, 0) > 0;
	}
	JsRecSet->RecSet->SortByFq(Asc);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sortByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	const TStr SortFieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	const int SortFieldId = JsRecSet->RecSet->GetStore()->GetFieldId(SortFieldNm);

	bool Asc = false;
	if (Args.Length() > 1) {
		QmAssertR(TNodeJsUtil::IsArgBool(Args, 1) || TNodeJsUtil::IsArgFlt(Args, 1),
            "TNodeJsRecSet::sortByField: Argument 1 expected to be bool or int!");
		Asc = TNodeJsUtil::IsArgBool(Args, 1) ?
			TNodeJsUtil::GetArgBool(Args, 1) :
			TNodeJsUtil::GetArgFlt(Args, 1) > 0;
	}

	JsRecSet->RecSet->SortByField(Asc, SortFieldId);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	const int MnFq = TNodeJsUtil::GetArgInt32(Args, 0);
	const int MxFq = TNodeJsUtil::GetArgInt32(Args, 1);
	JsRecSet->RecSet->FilterByFq(MnFq, MxFq);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filterByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1, "split(..) expects one argument.");
	QmAssertR(Args.Length() == 1 && Args[0]->IsFunction(),
		"split(..) expects one argument, which is a function.");
	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	TQm::TRecSetV RecSetV = JsRecSet->RecSet->SplitBy(TJsRecPairFilter(JsRecSet->RecSet->GetStore(), Callback));
	// prepare result array
	v8::Local<v8::Array> JsRecSetV = v8::Array::New(Isolate, RecSetV.Len());
	for (int RecSetN = 0; RecSetN < RecSetV.Len(); RecSetN++) {
		JsRecSetV->Set(RecSetN,
			TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSetV[RecSetN], JsRecSet->Watcher)));

	}
	Args.GetReturnValue().Set(JsRecSetV);
}

void TNodeJsRecSet::deleteRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"deleteRecords(..) expects a record set as input");
	TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());

	TUInt64Set RecIdSet; ArgJsRecSet->RecSet->GetRecIdSet(RecIdSet);
	JsRecSet->RecSet->RemoveRecIdSet(RecIdSet);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	const int MxHits = -1;
	const int Offset = 0;

	// make sure we do not try to interpet parameters when toJSON is called by JSON.stringify
	const bool JoinRecsP = TNodeJsUtil::IsArg(Args, 0) ?
		(TNodeJsUtil::IsArgBool(Args, 0) ? TNodeJsUtil::GetArgBool(Args, 0, false) : false) : false;
	const bool JoinRecFieldsP = TNodeJsUtil::IsArg(Args, 1) ?
		(TNodeJsUtil::IsArgBool(Args, 1) ? TNodeJsUtil::GetArgBool(Args, 1, false) : false) : false;
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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 is not a function!");

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	// iterate through the recset
	const int Recs = RecSet->GetRecs();
	if (Recs > 0) {

		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::NewInstance(new TNodeJsRec(JsRecSet->Watcher, RecSet->GetRec(0)));

		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(RecObj);

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
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "map: Argument 0 is not a function!");

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
	// iterate through the recset
	const int Recs = RecSet->GetRecs();

	v8::Handle<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

	if (Recs > 0) {
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 2;

		v8::Local<v8::Object> RecObj = TNodeJsRec::NewInstance(new TNodeJsRec(JsRecSet->Watcher, RecSet->GetRec(0)));
		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(RecObj);

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

void TNodeJsRecSet::setIntersect(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "rs.setIntersect: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(),
        "recset.setIntersect: the record sets do not point to the same store!");
	// Coputation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records in RecIdSet
	TQm::TRecFilterByRecIdSet Filter(RecIdSet, true);
	RecSet2->FilterBy(Filter);
	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet2, JsRecSet->Watcher)));
}

void TNodeJsRecSet::setUnion(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "rs.setUnion: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(),
        "recset.setUnion: the record sets do not point to the same store!");
	//// GetMerge sorts the argument!
	TQm::PRecSet RecSet1Clone = RecSet1->Clone();
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->GetMerge(RecSet1Clone);

	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet2, JsRecSet->Watcher)));
}

void TNodeJsRecSet::setDiff(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "rs.setDiff: first argument expected to be an record set");
	TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());
	TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

	QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(),
        "recset.setDiff: the record sets do not point to the same store!");
	// Computation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
	TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
	TUInt64Set RecIdSet;
	RecSet1->GetRecIdSet(RecIdSet);
	//second parameter in filter is false -> keep only records NOT in RecIdSet
	RecSet2->FilterBy(TQm::TRecFilterByRecIdSet(RecIdSet, false));
	Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet2, JsRecSet->Watcher)));
}

void TNodeJsRecSet::getVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	TWPt<TQm::TStore> Store = RecSet->GetStore();
	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	if (!RecSet->GetStore()->IsFieldNm(FieldNm)) {
		throw TQm::TQmExcept::New("RecSet.getVector: fieldName not found: " + FieldNm);
	}

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
		throw TQm::TQmExcept::New("rs.getVector does not support type float_v - use store.getMatrix instead");
	}
	else if (Desc.IsNumSpV()) {
		throw TQm::TQmExcept::New("rs.getVector does not support type num_sp_v - use store.getMatrix instead");
	}
	throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::getMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

	TQm::PRecSet RecSet = JsRecSet->RecSet;
	TWPt<TQm::TStore> Store = RecSet->GetStore();
	const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
	if (!RecSet->GetStore()->IsFieldNm(FieldNm)) {
		throw TQm::TQmExcept::New("RecSet.getMatrix: fieldName not found: " + FieldNm);
	}
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
		throw TQm::TQmExcept::New("store.getMatrix does not support type string - use store.getVector instead");
	}
	throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsRecSet->RecSet->GetStore(), JsRecSet->Watcher))
		);

}

void TNodeJsRecSet::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRecSet->RecSet->GetRecs()));
}

void TNodeJsRecSet::empty(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsRecSet->RecSet->Empty()));
}

void TNodeJsRecSet::weighted(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsRecSet->RecSet->IsWgt()));
}

void TNodeJsRecSet::indexId(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

	const int RecN = Index;
	if (0 <= RecN && RecN < JsRecSet->RecSet->GetRecs()) {
		const uint64 RecId = JsRecSet->RecSet->GetRecId(RecN);
		if (JsRecSet->RecSet->GetStore()->IsRecId(RecId)) {
			Info.GetReturnValue().Set(TNodeJsRec::NewInstance(
                new TNodeJsRec(JsRecSet->Watcher, JsRecSet->RecSet->GetRec(RecN), JsRecSet->RecSet->GetRecFq(RecN))));
			return;
		}
	}

	Info.GetReturnValue().Set(v8::Null(Isolate));
}

///////////////////////////////
// NodeJs QMiner Store Iterator
v8::Persistent<v8::Function> TNodeJsStoreIter::Constructor;
const TStr TNodeJsStoreIter::ClassId = "StoreIter";

void TNodeJsStoreIter::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStoreIter>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "next", _next);

	// Properties 
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "record"), _record);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	Constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsStoreIter::next(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Args.Holder());

	const bool NextP = JsStoreIter->Iter->Next();
	if (JsStoreIter->JsRec == nullptr && NextP) {
		// first time, create placeholder
		const uint64 RecId = JsStoreIter->Iter->GetRecId();
		v8::Local<v8::Object> _RecObj = TNodeJsRec::NewInstance(
            new TNodeJsRec(JsStoreIter->Watcher, JsStoreIter->Store->GetRec(RecId), 1));

		JsStoreIter->RecObj.Reset(Isolate, _RecObj);
		JsStoreIter->JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(_RecObj);
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

	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsStoreIter->Store, JsStoreIter->Watcher))
		);
}

void TNodeJsStoreIter::record(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
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
	v8::Local<v8::Object> JsRec = TNodeJsRec::NewInstance(
        new TNodeJsRec(TNodeJsBaseWatcher::New(), TQm::TRec(Store, RecIdWgt.Key), RecIdWgt.Dat));

	v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
	v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
	const unsigned Argc = 1;
	v8::Local<v8::Value> ArgV[Argc] = { JsRec };
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
	v8::Local<v8::Object> JsRec1 = TNodeJsRec::NewInstance(
        new TNodeJsRec(TNodeJsBaseWatcher::New(), TQm::TRec(Store, RecIdWgt1.Key), RecIdWgt1.Dat));
	v8::Local<v8::Object> JsRec2 = TNodeJsRec::NewInstance(
        new TNodeJsRec(TNodeJsBaseWatcher::New(), TQm::TRec(Store, RecIdWgt2.Key), RecIdWgt2.Dat));

	v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
	v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
	const unsigned Argc = 2;
	v8::Local<v8::Value> ArgV[Argc] = { JsRec1, JsRec2 };
	v8::Local<v8::Value> ReturnVal = Callbck->Call(GlobalContext, Argc, ArgV);

	QmAssertR(ReturnVal->IsBoolean() || ReturnVal->IsNumber(), "Comparator callback must return a boolean!");
	return ReturnVal->IsBoolean() ? ReturnVal->BooleanValue() : ReturnVal->NumberValue() < 0;
}

///////////////////////////////
// NodeJs QMiner Index Key
v8::Persistent<v8::Function> TNodeJsIndexKey::Constructor;
const TStr TNodeJsIndexKey::ClassId = "IndexKey";

void TNodeJsIndexKey::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsIndexKey>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.

	// Properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "store"), _store);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "name"), _name);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "vocabulary"), _vocabulary);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "fq"), _fq);

	// This has to be last, otherwise the properties won't show up on the object in JavaScript.
	Constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsIndexKey::store(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
	// return
	Info.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsIndexKey->Store, JsIndexKey->Watcher))
		);
}

void TNodeJsIndexKey::name(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
	// return
	Info.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, JsIndexKey->IndexKey.GetKeyNm().CStr()));
}

void TNodeJsIndexKey::vocabulary(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
	// preapre vocabulary array
	if (!JsIndexKey->IndexKey.IsWordVoc()) {
		// no vocabulary
		Info.GetReturnValue().Set(v8::Null(Isolate));
	}
	else {
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
	TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
	// preapre vocabulary array
	if (!JsIndexKey->IndexKey.IsWordVoc()) {
		// no vocabulary
		Info.GetReturnValue().Set(v8::Null(Isolate));
	}
	else {
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

///////////////////////////////////////////////
// Javascript Function Feature Extractor
TNodeJsFuncFtrExt::TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base,
    const PJsonVal& ParamVal, const v8::Handle<v8::Function> _Fun, v8::Isolate* Isolate) :
        TQm::TFtrExt(Base, ParamVal), Dim(ParamVal->GetObjInt("dim", 1)),
        Name(ParamVal->GetObjStr("name", "jsfunc")), Fun() {

	Fun.Reset(Isolate, _Fun);
}

TQm::PFtrExt TNodeJsFuncFtrExt::NewFtrExt(const TWPt<TQm::TBase>& Base,
        const PJsonVal& ParamVal, const v8::Handle<v8::Function>& Fun, v8::Isolate* Isolate) {
    
	return new TNodeJsFuncFtrExt(Base, ParamVal, Fun, Isolate);
}

double TNodeJsFuncFtrExt::ExecuteFunc(const TQm::TRec& FtrRec) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, Fun);
    return TNodeJsUtil::ExecuteFlt(Callback,
        TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), FtrRec))
    );
}

void TNodeJsFuncFtrExt::ExecuteFuncVec(const TQm::TRec& FtrRec, TFltV& Vec) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, Fun);
    v8::Handle<v8::Value> Argv[1] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), FtrRec)) };
    v8::Handle<v8::Value> RetVal = Callback->Call(Isolate->GetCurrentContext()->Global(), 1, Argv);

    // Cast as FltV and copy result
    v8::Handle<v8::Object> RetValObj = v8::Handle<v8::Object>::Cast(RetVal);

    QmAssertR(TNodeJsUtil::IsClass(RetValObj, TNodeJsFltV::GetClassId()), "TJsFuncFtrExt::ExecuteFuncVec callback should return a dense vector (same type as la.newVec()).");

    v8::Local<v8::External> WrappedObject = v8::Local<v8::External>::Cast(RetValObj->GetInternalField(0));
    // cast it to js vector and copy internal vector
    TNodeJsFltV* JsVec = static_cast<TNodeJsFltV*>(WrappedObject->Value());

    Vec = JsVec->Vec;
}

TNodeJsFuncFtrExt::TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal) : TFtrExt(Base, ParamVal) {
	throw TQm::TQmExcept::New("javascript function feature extractor shouldn't be constructed "
        "calling TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal), "
        "call TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal) instead (construct from JS using analytics)");
}

TNodeJsFuncFtrExt::TNodeJsFuncFtrExt(const TWPt<TQm::TBase>& Base, TSIn& SIn) : TFtrExt(Base, SIn) {
	throw TQm::TQmExcept::New("javascript function feature extractor shouldn't be constructed "
        "calling TJsFuncFtrExt::TJsFuncFtrExt(const TWPt<TBase>& Base, TSIn& SIn), call "
        "TJsFuncFtrExt(TWPt<TScript> _Js, const PJsonVal& ParamVal) instead (construct from JS using analytics)");
}

TQm::PFtrExt TNodeJsFuncFtrExt::New(const TWPt<TQm::TBase>& Base, const PJsonVal& ParamVal) {
	return new TNodeJsFuncFtrExt(Base, ParamVal);
}

TQm::PFtrExt TNodeJsFuncFtrExt::Load(const TWPt<TQm::TBase>& Base, TSIn& SIn) {
	return new TNodeJsFuncFtrExt(Base, SIn);
}
void TNodeJsFuncFtrExt::Save(TSOut& SOut) const {
	throw TQm::TQmExcept::New("TJsFuncFtrExt::Save(TSOut& Sout) : saving is not supported");
}

void TNodeJsFuncFtrExt::AddSpV(const TQm::TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const {
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

void TNodeJsFuncFtrExt::AddFullV(const TQm::TRec& Rec, TFltV& FullV, int& Offset) const {
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

void TNodeJsFuncFtrExt::ExtractFltV(const TQm::TRec& FtrRec, TFltV& FltV) const {
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

///////////////////////////////
// NodeJs QMiner Feature Space
const TStr TNodeJsFtrSpace::ClassId = "FeatureSpace";

TNodeJsFtrSpace::TNodeJsFtrSpace(const TQm::PFtrSpace& _FtrSpace) :
FtrSpace(_FtrSpace) {}

TNodeJsFtrSpace::TNodeJsFtrSpace(const TWPt<TQm::TBase> Base, TSIn& SIn) {
	FtrSpace = TQm::TFtrSpace::Load(Base, SIn);
}

void TNodeJsFtrSpace::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsFtrSpace>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "addFeatureExtractor", _addFeatureExtractor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "updateRecord", _updateRecord);
	NODE_SET_PROTOTYPE_METHOD(tpl, "updateRecords", _updateRecords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "extractSparseVector", _extractSparseVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "extractVector", _extractVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "invertFeatureVector", _invertFeatureVector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "invertFeature", _invertFeature);
	NODE_SET_PROTOTYPE_METHOD(tpl, "extractSparseMatrix", _extractSparseMatrix);
	NODE_SET_PROTOTYPE_METHOD(tpl, "extractMatrix", _extractMatrix);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureExtractor", _getFeatureExtractor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFeature", _getFeature);
	NODE_SET_PROTOTYPE_METHOD(tpl, "filter", _filter);
	NODE_SET_PROTOTYPE_METHOD(tpl, "extractStrings", _extractStrings);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dims"), _dims);

	// So we can call new FeatureSpace
	exports->Set(v8::String::NewFromUtf8(Isolate, ClassId.CStr()), tpl->GetFunction());
}

TNodeJsFtrSpace* TNodeJsFtrSpace::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	const TWPt<TQm::TBase>& Base = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args[0]->ToObject())->Base;

	if (Args[1]->IsString() || TNodeJsUtil::IsArgClass(Args, 1, TNodeJsFIn::ClassId)) {
		bool IsArgStr = TNodeJsUtil::IsArgStr(Args, 1);//Args[1]->IsString();

		PSIn SIn = IsArgStr ?
			TFIn::New(TNodeJsUtil::GetArgStr(Args, 1)) :
			ObjectWrap::Unwrap<TNodeJsFIn>(Args[1]->ToObject())->SIn;

		return new TNodeJsFtrSpace(Base, *SIn);
	}

	TQm::TFtrExtV FtrExtV;
	if (Args[1]->IsArray()) {
		v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[1]);
		for (uint32 ObjectN = 0; ObjectN < Array->Length(); ObjectN++) {
			if (Array->Get(ObjectN)->IsObject()) {
				v8::Local<v8::Object> Obj = Array->Get(ObjectN)->ToObject();
				// get property "type"
				if (Obj->Has(v8::String::NewFromUtf8(Isolate, "type"))) {
					v8::Handle<v8::Value> TypeVal = Obj->Get(v8::String::NewFromUtf8(Isolate, "type"));
					if (TypeVal->IsString()) {
						v8::String::Utf8Value Utf8(TypeVal);
						TStr Type(*Utf8);
						if (Type == "jsfunc") {
							QmAssertR(Obj->Has(v8::String::NewFromUtf8(Isolate, "fun")),
								"analytics.newFeatureSpace object of type 'jsfunc' should have a property 'fun'");
							QmAssertR(Obj->Get(v8::String::NewFromUtf8(Isolate, "fun"))->IsFunction(),
								"analytics.newFeatureSpace object.fun should be a function");

							FtrExtV.Add(TNodeJsFtrSpace::NewFtrExtFromFunc(Base, Obj, Isolate));
						} else {
							// Json val to glib JSON
							PJsonVal ParamVal = TNodeJsUtil::GetObjJson(Array->Get(ObjectN)->ToObject());
							if (ParamVal->IsObj()) {
								FtrExtV.Add(TQm::TFtrExt::New(Base, ParamVal->GetObjStr("type"), ParamVal));
							}
						}
					}
				}
			}
		}
	} else if (Args[1]->IsObject()) {
		// get type
		TStr Type = TNodeJsUtil::GetArgStr(Args, 1, "type", "");
		if (Type == "jsfunc") {
			// All properties should be JSON objects, except for "fun", which is a function
			// example (Twitter text length feature extractor):
			// { type : 'jsfunc', source: { store: 'Tweets' }, fun : function(rec) {return rec.Text.length;}}
			v8::Local<v8::Object> Settings = Args[1]->ToObject();
			FtrExtV.Add(TNodeJsFtrSpace::NewFtrExtFromFunc(Base, Settings, Isolate));
		} else {
			// JSON object expected
			// example (bag of words extractor)
			// { type: 'numeric', source: { store: 'Movies' }, field: 'Rating', normalize: true }
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 1);
			if (ParamVal->IsObj()) {
				FtrExtV.Add(TQm::TFtrExt::New(Base, ParamVal->GetObjStr("type"), ParamVal));
			}
		}
	}

	// create feature space
	TQm::PFtrSpace FtrSpace = TQm::TFtrSpace::New(Base, FtrExtV);
	// report on what was created
	TQm::InfoLog(FtrSpace->GetNm());
	return new TNodeJsFtrSpace(FtrSpace);
}


void TNodeJsFtrSpace::dim(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Info.Holder());
		Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsFtrSpace->FtrSpace->GetDim()));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::dim");
	}
}

void TNodeJsFtrSpace::dims(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Info.Holder());

		const int FtrExts = JsFtrSpace->FtrSpace->GetFtrExts();
		v8::Handle<v8::Array> IntArr = v8::Array::New(Isolate, FtrExts);

		for (int FtrExtN = 0; FtrExtN < FtrExts; FtrExtN++) {
			IntArr->Set(v8::Uint32::New(Isolate, FtrExtN),
				v8::Integer::New(Isolate, JsFtrSpace->FtrSpace->GetFtrExtDim(FtrExtN)));
		}

		Info.GetReturnValue().Set(IntArr);
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::dims");
	}
}

void TNodeJsFtrSpace::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		PSOut SOut = TNodeJsUtil::IsArgStr(Args, 0) ?
			TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), true) :
			ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject())->SOut;

		// save to stream
		JsFtrSpace->FtrSpace->Save(*SOut);
		// return
		Args.GetReturnValue().Set(Args[0]);
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::save");
	}
}

void TNodeJsFtrSpace::addFeatureExtractor(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		TStr Type = TNodeJsUtil::GetArgStr(Args, 0, "type", "");
		if (Type == "jsfunc") {
			// All properties should be JSON objects, except for "fun", which is a function
			// example (Twitter text length feature extractor):
			// { type : 'jsfunc', source: { store: 'Tweets' }, fun : function(rec) {return rec.Text.length;}}
			// extract function!
			v8::Local<v8::Object> Settings = Args[0]->ToObject();
			JsFtrSpace->FtrSpace->AddFtrExt(TNodeJsFtrSpace::NewFtrExtFromFunc(JsFtrSpace->FtrSpace->GetBase(), Settings, Isolate));
		}
		else {
			// JSON object expected
			// example (bag of words extractor)
			// { type: 'numeric', source: { store: 'Movies' }, field: 'Rating', normalize: true }
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
			if (ParamVal->IsObj()) {
				JsFtrSpace->FtrSpace->AddFtrExt(TQm::TFtrExt::New(JsFtrSpace->FtrSpace->GetBase(), ParamVal->GetObjStr("type"), ParamVal));
			}
		}
		// return
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::addFeatureExtractor");
	}
}

void TNodeJsFtrSpace::updateRecord(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());

		// update with new records
		JsFtrSpace->FtrSpace->Update(JsRec->Rec);

		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::updateRecord");
	}
}

void TNodeJsFtrSpace::updateRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());

		// update with new records
		JsFtrSpace->FtrSpace->Update(JsRecSet->RecSet);

		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::updateRecords");
	}
}

void TNodeJsFtrSpace::extractSparseVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	
	TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
	TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());

	// create feature vector
	TIntFltKdV SpV;
	JsFtrSpace->FtrSpace->GetSpV(JsRec->Rec, SpV);

	Args.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsSpVec>(
		new TNodeJsSpVec(SpV, JsFtrSpace->FtrSpace->GetDim())));

}

void TNodeJsFtrSpace::extractVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Args[0]->ToObject());

		// create feature vector, compute
		TFltV FltV;
		JsFtrSpace->FtrSpace->GetFullV(JsRec->Rec, FltV);

		Args.GetReturnValue().Set(TNodeJsFltV::New(FltV));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::extractVector");
	}
}

void TNodeJsFtrSpace::invertFeatureVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()) || Args[0]->IsArray(), "The argument should be a float array!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		TFltV InvertV;

		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {
			TFltV& FtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject())->Vec;
			JsFtrSpace->FtrSpace->InvertFullV(FtrV, InvertV);
		}
		else {
			v8::Array* Arr = v8::Array::Cast(*Args[0]);
			TFltV FtrV(Arr->Length(), 0);

			for (uint i = 0; i < Arr->Length(); i++) {
				FtrV.Add(Arr->Get(i)->NumberValue());
			}

			JsFtrSpace->FtrSpace->InvertFullV(FtrV, InvertV);
		}

		Args.GetReturnValue().Set(TNodeJsFltV::New(InvertV));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::invertFeatureVector");
	}
}

void TNodeJsFtrSpace::invertFeature(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 2, "ftrSpace.invertFeature: Should have 2 arguments!");
	QmAssertR(TNodeJsUtil::IsArgFlt(Args, 1), "ftrSpace.invertFeature: The argument should be a float!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
		double Val = TNodeJsUtil::GetArgFlt(Args, 1);

		double InvVal = JsFtrSpace->FtrSpace->InvertFtr(FtrExtN, Val);

		Args.GetReturnValue().Set(v8::Number::New(Isolate, InvVal));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::invertFeature");
	}
}

void TNodeJsFtrSpace::extractSparseMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		TNodeJsRecSet* RecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());

		// create feature matrix
		TVec<TIntFltKdV> SpMat(RecSet->RecSet->GetRecs(), 0);
		JsFtrSpace->FtrSpace->GetSpVV(RecSet->RecSet, SpMat);

		Args.GetReturnValue().Set(TNodeJsSpMat::New(SpMat, -1));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::extractSparseMatrix");
	}
}

void TNodeJsFtrSpace::extractMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		TNodeJsRecSet* RecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args[0]->ToObject());

		// create feature matrix
		TFltVV Mat;
		JsFtrSpace->FtrSpace->GetFullVV(RecSet->RecSet, Mat);

		Args.GetReturnValue().Set(TNodeJsFltVV::New(Mat));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::extractMatrix");
	}
}

void TNodeJsFtrSpace::getFeatureExtractor(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
		const TStr FtrExtNm = JsFtrSpace->FtrSpace->GetFtrExt(FtrExtN)->GetNm();

		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, FtrExtNm.CStr()));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::getFeatureExtractor");
	}
}

void TNodeJsFtrSpace::getFeature(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		const int FtrN = TNodeJsUtil::GetArgInt32(Args, 0);
		const TStr FtrNm = JsFtrSpace->FtrSpace->GetFtr(FtrN);

		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, FtrNm.CStr()));
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::getFeature");
	}
}

void TNodeJsFtrSpace::filter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "fsp.filter: Expecting vector as parameter");
	QmAssertR(Args[0]->IsObject(), "fsp.filter: Expecting vector as parameter");

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
		QmAssertR(TNodeJsUtil::IsArgClass(Args, 0,
            TNodeJsFltV::GetClassId()) || TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpVec::ClassId),
            "FeatureSpace.filter: expecting a dense or a sparse vector as the first argument!");
		const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1);
		const bool KeepOffsetP = TNodeJsUtil::GetArgBool(Args, 2, true);
		// get dimension border
		const int MnFtrN = JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN);
		const int MxFtrN = JsFtrSpace->FtrSpace->GetMxFtrN(FtrExtN);

		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpVec::ClassId)) {
			const TIntFltKdV& SpV = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject())->Vec;

			// filter
			TIntFltKdV NewSpV;
			for (int FtrN = 0; FtrN < SpV.Len(); FtrN++) {
				const TIntFltKd& Ftr = SpV[FtrN];
				if (MnFtrN <= Ftr.Key && Ftr.Key < MxFtrN) {
					NewSpV.Add(Ftr);
				}
			}
			if (!KeepOffsetP) {
				for (int NewSpN = 0; NewSpN < NewSpV.Len(); NewSpN++) {
					NewSpV[NewSpN].Key -= MnFtrN;
				}
			}

			const int VecDim = KeepOffsetP ? JsFtrSpace->FtrSpace->GetDim() : (MxFtrN - MnFtrN);
			Args.GetReturnValue().Set(
				TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(NewSpV, VecDim)));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {
			const TFltV& Vec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject())->Vec;
			int DimN = JsFtrSpace->FtrSpace->GetFtrExtDim(FtrExtN);

			// filter				
			TFltV NewVec;
			if (KeepOffsetP) {
				NewVec.Gen(Vec.Len());
			}
			else {
				NewVec.Gen(DimN);
			}
			int VecOffset = JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN);
			int NewVecOffset = KeepOffsetP ? JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN) : 0;
			for (int FtrN = 0; FtrN < DimN; FtrN++) {
				NewVec[FtrN + NewVecOffset] = Vec[FtrN + VecOffset];
			}
			Args.GetReturnValue().Set(TNodeJsFltV::New(NewVec));
		}
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::filter");
	}
}

void TNodeJsFtrSpace::extractStrings(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

		const PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
		const int DimN = TNodeJsUtil::GetArgInt32(Args, 1, 0);

		// get strings
		TStrV StrV; JsFtrSpace->FtrSpace->ExtractStrV(DimN, RecVal, StrV);

		v8::Handle<v8::Array> StrArr = v8::Array::New(Isolate, StrV.Len());

		for (int StrN = 0; StrN < StrV.Len(); StrN++) {
			StrArr->Set(v8::Uint32::New(Isolate, StrN), v8::String::NewFromUtf8(Isolate, StrV.GetVal(StrN).CStr()));
		}

		Args.GetReturnValue().Set(StrArr);
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::extractStrings");
	}
}


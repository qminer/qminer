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

void TNodeJsQm::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // Add all methods, getters and setters here.
    NODE_SET_METHOD(exports, "config", _config);
    NODE_SET_METHOD(exports, "create", _create);
    NODE_SET_METHOD(exports, "open", _open);
    NODE_SET_METHOD(exports, "verbosity", _verbosity);
    NODE_SET_METHOD(exports, "stats", _stats);

    // Add properties
    v8::Maybe<bool> SuccessP = exports->SetAccessor(Isolate->GetCurrentContext(),
      TNodeJsUtil::ToLocal(Nan::New("flags")), _flags);
    QmAssert(!SuccessP.IsNothing() && SuccessP.FromJust());

    // initialize QMiner environment
    TQm::TEnv::Init();
    // default to no output
    TQm::TEnv::InitLogger(0, "std");

    // load unicode
    TStr UnicodeFNm = TPath::Combine(TQm::TEnv::QMinerFPath, "UnicodeDef.Bin");
    if (!TUnicodeDef::IsDef()) { TUnicodeDef::Load(UnicodeFNm); }
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
    TBool StrictNmP = TNodeJsUtil::GetArgBool(Args, 3, true);

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
        TWPt<TQm::TBase> Base_ = TQm::TStorage::NewBase(Param.DbFPath, SchemaVal, Param.IndexCacheSize, Param.DefStoreCacheSize, StrictNmP);
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

void TNodeJsQm::stats(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Result = v8::Object::New(Isolate);

    v8::Local<v8::Object> ByClass = v8::Object::New(Isolate);
    v8::Local<v8::Object> Total = v8::Object::New(Isolate);

    for (const auto& KeyDat : TNodeJsUtil::ObjNameH) {
        v8::Local<v8::Object> ClassCounts = v8::Object::New(Isolate);
        Nan::Set(ClassCounts, TNodeJsUtil::ToLocal(Nan::New("newFromCpp")),
            Nan::New(KeyDat.Dat.Val1));
        Nan::Set(ClassCounts, TNodeJsUtil::ToLocal(Nan::New("newFromJs")),
            Nan::New(KeyDat.Dat.Val2));
        Nan::Set(ClassCounts, TNodeJsUtil::ToLocal(Nan::New("destructorCalls")),
            Nan::New(KeyDat.Dat.Val3));
        Nan::Set(ByClass, TNodeJsUtil::ToLocal(Nan::New(KeyDat.Key.CStr())),
            ClassCounts);
    }

    Nan::Set(Total, TNodeJsUtil::ToLocal(Nan::New("newFromCpp")),
        Nan::New(TNodeJsUtil::ObjCount.Val1));
    Nan::Set(Total, TNodeJsUtil::ToLocal(Nan::New("newFromJs")),
        Nan::New(TNodeJsUtil::ObjCount.Val2));
    Nan::Set(Total, TNodeJsUtil::ToLocal(Nan::New("destructorCalls")),
        Nan::New(TNodeJsUtil::ObjCount.Val3));

    Nan::Set(Result, TNodeJsUtil::ToLocal(Nan::New("byClass")), ByClass);
    Nan::Set(Result, TNodeJsUtil::ToLocal(Nan::New("total")), Total);

    Args.GetReturnValue().Set(Result);
}

void TNodeJsQm::flags(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> JsObj = v8::Object::New(Isolate);

    TStr BuildTime = TStr(__DATE__) + " " + TStr(__TIME__);
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("buildTime")), TNodeJsUtil::ToLocal(Nan::New(BuildTime.CStr())));

#ifdef GLib_WIN
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("win")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("win")), Nan::New(false));
#endif

#ifdef GLib_UNIX
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("unix")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("unix")), Nan::New(false));
#endif

#ifdef GLib_LINUX
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("linux")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("linux")), Nan::New(false));
#endif

#ifdef GLib_MACOSX
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("darwin")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("darwin")), Nan::New(false));
#endif

#ifdef GLib_32Bit
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("x86")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("x86")), Nan::New(false));
#endif

#ifdef GLib_64Bit
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("x64")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("x64")), Nan::New(false));
#endif

#ifdef GLib_OPENMP
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("omp")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("omp")), Nan::New(false));
#endif

#ifdef NDEBUG
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("debug")), Nan::New(false));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("debug")), Nan::New(true));
#endif

#ifdef GLib_GCC
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("gcc")), TNodeJsUtil::ToLocal(Nan::New(__VERSION__)));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("gcc")), Nan::New(false));
#endif

#ifdef GLib_CLANG
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("clang")), TNodeJsUtil::ToLocal(Nan::New(__clang_version__)));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("clang")), Nan::New(false));
#endif

#ifdef GLib_MSC
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("msc")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("msc")), Nan::New(false));
#endif

#ifdef GLib_GLIBC
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("glibc")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("glibc")), Nan::New(false));
#endif

    // By default the blas flags are false
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas")), Nan::New(false));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_intel")), Nan::New(false));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_amd")), Nan::New(false));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_openblas")), Nan::New(false));
#ifdef BLAS
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas")), Nan::New(true));
    #ifdef INTEL
        Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_intel")), Nan::New(true));
    #elif AMD
        Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_amd")), Nan::New(true));
    #else // openblas
        Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("blas_openblas")), Nan::New(true));
    #endif
#endif

#ifdef LAPACKE
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("lapacke")), Nan::New(true));
#else
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("lapacke")), Nan::New(false));
#endif

    // basic type sizes
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(char)")), v8::Integer::New(Isolate, sizeof(char)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(int8)")), v8::Integer::New(Isolate, sizeof(int8)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(short)")), v8::Integer::New(Isolate, sizeof(short)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(int16)")), v8::Integer::New(Isolate, sizeof(int16)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(int)")), v8::Integer::New(Isolate, sizeof(int)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(int32)")), v8::Integer::New(Isolate, sizeof(int32)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(long)")), v8::Integer::New(Isolate, sizeof(long)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(long long)")), v8::Integer::New(Isolate, sizeof(long long)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(int64)")), v8::Integer::New(Isolate, sizeof(int64)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(char*)")), v8::Integer::New(Isolate, sizeof(char*)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(size_t)")), v8::Integer::New(Isolate, sizeof(size_t)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(float)")), v8::Integer::New(Isolate, sizeof(float)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(double)")), v8::Integer::New(Isolate, sizeof(double)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(char*)")), v8::Integer::New(Isolate, sizeof(char*)));

    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TStr)")), v8::Integer::New(Isolate, sizeof(TStr)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TChA)")), v8::Integer::New(Isolate, sizeof(TChA)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TMem)")), v8::Integer::New(Isolate, sizeof(TMem)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TBlobPt)")), v8::Integer::New(Isolate, sizeof(TBlobPt)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TPgBlobPt)")), v8::Integer::New(Isolate, sizeof(TPgBlobPt)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TRec)")), v8::Integer::New(Isolate, sizeof(TQm::TRec)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TRecSet)")), v8::Integer::New(Isolate, sizeof(TQm::TRecSet)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TRecFilter)")), v8::Integer::New(Isolate, sizeof(TQm::TRecFilter)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TAggr)")), v8::Integer::New(Isolate, sizeof(TQm::TAggr)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TStreamAggr)")), v8::Integer::New(Isolate, sizeof(TQm::TStreamAggr)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TFtrExt)")), v8::Integer::New(Isolate, sizeof(TQm::TFtrExt)));
    Nan::Set(JsObj, TNodeJsUtil::ToLocal(Nan::New("sizeof(TFtrSpace)")), v8::Integer::New(Isolate, sizeof(TQm::TFtrSpace)));

    Info.GetReturnValue().Set(JsObj);
}

///////////////////////////////
// NodeJs QMiner Base
v8::Persistent<v8::Function> TNodeJsBase::Constructor;
const int TNodeJsBase::MAX_BASES = 128;

void TNodeJsBase::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    // template for creating function from javascript using "new", uses _NewJs callback
    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsBase>);
    // child will have the same properties and methods, but a different callback: _NewCpp
    v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsBase>);
    child->Inherit(tpl);

    child->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    child->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "close", _close);
    NODE_SET_PROTOTYPE_METHOD(tpl, "isClosed", _isClosed);
    NODE_SET_PROTOTYPE_METHOD(tpl, "store", _store);
    NODE_SET_PROTOTYPE_METHOD(tpl, "isStore", _isStore);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStoreList", _getStoreList);
    NODE_SET_PROTOTYPE_METHOD(tpl, "createStore", _createStore);
    NODE_SET_PROTOTYPE_METHOD(tpl, "createJsStore", _createJsStore);
    NODE_SET_PROTOTYPE_METHOD(tpl, "addJsStoreCallback", _addJsStoreCallback);
    NODE_SET_PROTOTYPE_METHOD(tpl, "search", _search);
    NODE_SET_PROTOTYPE_METHOD(tpl, "garbageCollect", _garbageCollect);
    NODE_SET_PROTOTYPE_METHOD(tpl, "partialFlush", _partialFlush);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStats", _getStats);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggr", _getStreamAggr);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrStats", _getStreamAggrStats);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript
    // Constructor is used when creating the object from C++
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(child->GetFunction(context)));
    // we need to export the class for calling using "new FIn(...)"
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())),
        TNodeJsUtil::ToLocal(tpl->GetFunction(context)));

}

TNodeJsBase::TNodeJsBase(const TStr& DbFPath_, const TStr& SchemaFNm, const PJsonVal& Schema,
        const bool& Create, const bool& ForceCreate, const bool& RdOnlyP, const bool& StrictNmP,
        const uint64& IndexCacheSize, const uint64& StoreCacheSize) {

    Watcher = TNodeJsBaseWatcher::New();

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TStr DbFPath = DbFPath_;
    if (!(DbFPath.LastCh() == '/' || DbFPath.LastCh() == '\\')) { DbFPath += "/"; }

    TStr LockFNm = DbFPath + "lock";

    // clean folder and lock
    if (ForceCreate) {
        if (TDir::Exists(DbFPath)) {
            // delete only qminer stuff!
            TFile::Del(LockFNm, false);
            // json files
            TFile::Del(TPath::Combine(DbFPath, "Base.json"), false);
            TFile::Del(TPath::Combine(DbFPath, "StoreList.json"), false);
            // Index files
            TFile::DelWc(TPath::Combine(DbFPath, "Index.*"), false);
            // IndexSmall files
            TFile::DelWc(TPath::Combine(DbFPath, "IndexSmall.*"), false);
            // IndexVoc files
            TFile::Del(TPath::Combine(DbFPath, "IndexVoc.dat"), false);
            // StoreBlob files
            TFile::DelWc(TPath::Combine(DbFPath, "StoreBlob.*"), false);
            // Store files (*.BaseStore, *.Cache, *.GenericStore, *.MemCache)
            TFile::DelWc(TPath::Combine(DbFPath, "*.BaseStore"), false);
            TFile::DelWc(TPath::Combine(DbFPath, "*.Cache"), false);
            TFile::DelWc(TPath::Combine(DbFPath, "*.GenericStore"), false);
            TFile::DelWc(TPath::Combine(DbFPath, "*.MemCache"), false);
        }
    }
    if (Create) {
        if (TDir::Exists(DbFPath)) {
            if (TFile::Exists(TPath::Combine(DbFPath, "StoreList.json"))) {
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

        Base = TQm::TStorage::NewBase(DbFPath, SchemaVal, IndexCacheSize, StoreCacheSize, StrictNmP);
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
            Base = TQm::TStorage::LoadBase(DbFPath, FAccess, IndexCacheSize, StoreCacheSize);
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

    EAssertR(Mode == "create" || Mode == "createClean" || Mode == "openReadOnly" || Mode == "open", "Base.create: Unrecognized mode " + Mode);
    const bool StrictNmP = Val->GetObjBool("strictNames", true);

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

    // Load Stopword Files
    TStr StopWordsPath = Val->GetObjStr("stopwords", TQm::TEnv::QMinerFPath + "resources/stopwords/");
    TSwSet::LoadSwDir(StopWordsPath);

    return new TNodeJsBase(DbPath, SchemaFNm, Schema, Create, ForceCreate, ReadOnly, StrictNmP, IndexCache, StoreCache);
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

void TNodeJsBase::isClosed(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args.Holder());

    // check the watcher if we have closed the base
    bool IsClosed = JsBase->Watcher->IsClosed();

    if (!IsClosed) {
        // check if the base itself is closed
        IsClosed = !JsBase->Base->IsInit();
    }

    Args.GetReturnValue().Set(Nan::New(IsClosed));
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
        Args.GetReturnValue().Set(Nan::Null());
        return;
    }
}

void TNodeJsBase::isStore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());

    const TStr StoreNm = TNodeJsUtil::GetArgStr(Args, 0);

    Args.GetReturnValue().Set(Nan::New(JsBase->Base->IsStoreNm(StoreNm)));
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
            Nan::Set(JsNewStoreV, Nan::New(NewStoreN),
                TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(NewStoreV[NewStoreN], JsBase->Watcher))
            );
        }
    }
    else {
        Args.GetReturnValue().Set(Nan::Null());
    }
}

void TNodeJsBase::createJsStore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
    TWPt<TQm::TBase> Base = JsBase->Base;
    QmAssertR(!Base->IsRdOnly(), "Base opened as read-only");
    // parse arguments
    PJsonVal SchemaVal = TNodeJsUtil::GetArgJson(Args, 0);

    QmAssertR(Args.Length() == 2 && (Args[1]->IsObject() || Args[1]->IsArray()), "createJsStore expects 2 arguments: schema JSON (or array) and callback object (or array)");
    // create new stores
    TVec<TWPt<TQm::TStore> > NewStoreV = TQm::TStorage::CreateJsStoresFromSchema(
        Base, SchemaVal, Args[1]);
    // Update record templates
    for (int StoreN = 0; StoreN < NewStoreV.Len(); StoreN++) {
        TNodeJsRec::Init(NewStoreV[StoreN]);
    }
    // return store (if only one) or array of stores (if more)
    if (NewStoreV.Len() == 1) {
        Args.GetReturnValue().Set(
            TNodeJsUtil::NewInstance<TNodeJsStore>(
            new TNodeJsStore(NewStoreV[0], JsBase->Watcher)));
    } else if (NewStoreV.Len() > 1) {
        v8::Local<v8::Array> JsNewStoreV = v8::Array::New(Isolate, NewStoreV.Len());
        for (int NewStoreN = 0; NewStoreN < NewStoreV.Len(); NewStoreN++) {
            Nan::Set(JsNewStoreV, Nan::New(NewStoreN),
                TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(NewStoreV[NewStoreN], JsBase->Watcher))
            );
        }
    } else {
        Args.GetReturnValue().Set(Nan::Null());
    }
}

void TNodeJsBase::addJsStoreCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
    TWPt<TQm::TBase> Base = JsBase->Base;
    QmAssert(Args.Length() == 3);
    QmAssert(Args[0]->IsObject());
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    QmAssert(Args[1]->IsString());
    TStr PropName = TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args[1])));
    QmAssert(Args[2]->IsFunction());

    QmAssert(JsStore->Store->GetStoreType() == "TNodeJsFuncStore");
    TQm::TStorage::TNodeJsFuncStore* JsImplStore = dynamic_cast<TQm::TStorage::TNodeJsFuncStore *>(JsStore->Store());
    QmAssert(JsImplStore != NULL);

    if (PropName == "GetAllRecs") {
        JsImplStore->GetAllRecsFun.Reset(Isolate, v8::Local<v8::Function>::Cast(Args[2]));
    } else {
        throw TQm::TQmExcept::New("store.addJsStoreCallback: unrecognized callback name: " + PropName);
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

    // parse out optional max time parameter
    const int MxTimeMSecs = TNodeJsUtil::GetArgInt32(Args, 0, -1);

    Base->GarbageCollect(MxTimeMSecs);

    Args.GetReturnValue().Set(Nan::Undefined());
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
        Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsStreamAggr>(new TNodeJsStreamAggr(StreamAggr)));
    }
}

void TNodeJsBase::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
    TWPt<TQm::TBase> Base = JsBase->Base;

    // get list of names
    TStrV StreamAggrNmV = Base->GetStreamAggrNmV();
    // set list as return value
    Args.GetReturnValue().Set(TNodeJsUtil::GetStrArr(StreamAggrNmV));
}

void TNodeJsBase::getStreamAggrStats(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(Args.Holder());
    TWPt<TQm::TBase> Base = JsBase->Base;

    PJsonVal res = Base->GetStreamAggrStats();
    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, res));
}

///////////////////////////////
// NodeJs QMiner Store
v8::Persistent<v8::Function> TNodeJsStore::Constructor;

void TNodeJsStore::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStore>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
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
    NODE_SET_PROTOTYPE_METHOD(tpl, "resetStreamAggregates", _resetStreamAggregates);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getStreamAggrNames", _getStreamAggrNames);
    NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
    NODE_SET_PROTOTYPE_METHOD(tpl, "clear", _clear);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getVector", _getVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getMatrix", _getMatrix);
    NODE_SET_PROTOTYPE_METHOD(tpl, "cell", _cell);
    NODE_SET_PROTOTYPE_METHOD(tpl, "triggerOnAddCallbacks", _triggerOnAddCallbacks);

    // Properties
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("name")), _name);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("empty")), _empty);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("length")), _length);
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("allRecords")), v8::FunctionTemplate::New(Isolate, _allRecords));
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("fields")), _fields);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("joins")), _joins);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("keys")), _keys);
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("first")), v8::FunctionTemplate::New(Isolate, _first));
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("last")), v8::FunctionTemplate::New(Isolate, _last));
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("forwardIter")), v8::FunctionTemplate::New(Isolate, _forwardIter));
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("backwardIter")), v8::FunctionTemplate::New(Isolate, _backwardIter));
    tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("base")), v8::FunctionTemplate::New(Isolate, _base));

    // This has to be last, otherwise the properties won't show up on the object in JavaScript.
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
    // So we can add stuff to the prototype in JS
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
    TNodeJsUtil::RegisterClassNmAccessor(GetClassId(), "name");
}

v8::Local<v8::Value> TNodeJsStore::Field(const TQm::TRec& Rec, const int FieldId) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    Nan::EscapableHandleScope scope;

    // check if field is null
    if (Rec.IsFieldNull(FieldId)) {
        return Nan::Null();
    }
    // not null, get value
    const TQm::TFieldDesc& Desc = Rec.GetStore()->GetFieldDesc(FieldId);
    if (Desc.IsInt()) {
        const int Val = Rec.GetFieldInt(FieldId);
        return scope.Escape(v8::Integer::New(Isolate, Val));
    }
    else if (Desc.IsInt16()) {
        const int16 Val = Rec.GetFieldInt16(FieldId);
        return scope.Escape(v8::Integer::New(Isolate, Val));
    }
    else if (Desc.IsInt64()) {
        const int64 Val = Rec.GetFieldInt64(FieldId);
        return scope.Escape(Nan::New(double(Val)));
    }
    else if (Desc.IsByte()) {
        const uchar Val = Rec.GetFieldByte(FieldId);
        return scope.Escape(v8::Integer::New(Isolate, Val));
    }
    else if (Desc.IsIntV()) {
        TIntV IntV; Rec.GetFieldIntV(FieldId, IntV);
        return scope.Escape(TNodeJsVec<TInt, TAuxIntV>::New(IntV));
    }
    else if (Desc.IsUInt()) {
        const uint Val = Rec.GetFieldUInt(FieldId);
        return scope.Escape(Nan::New(Val));
    }
    else if (Desc.IsUInt16()) {
        const uint16 Val = Rec.GetFieldUInt16(FieldId);
        return scope.Escape(v8::Integer::New(Isolate, Val));
    }
    else if (Desc.IsUInt64()) {
        const uint64 Val = Rec.GetFieldUInt64(FieldId);
        return scope.Escape(v8::Integer::New(Isolate, (int)Val));
    }
    else if (Desc.IsStr()) {
        const TStr Val = Rec.GetFieldStr(FieldId);
        v8::MaybeLocal<v8::String> TmpString = TNodeJsUtil::ToLocal(Nan::New(Val.CStr()));
        return scope.Escape(TNodeJsUtil::ToLocal(TmpString));
    }
    else if (Desc.IsStrV()) {
        TStrV StrV; Rec.GetFieldStrV(FieldId, StrV);
        return scope.Escape(TNodeJsVec<TStr, TAuxStrV>::New(StrV));
    }
    else if (Desc.IsBool()) {
        const bool Val = Rec.GetFieldBool(FieldId);
        return Nan::New(Val);
    }
    else if (Desc.IsFlt()) {
        const double Val = Rec.GetFieldFlt(FieldId);
        return scope.Escape(Nan::New(Val));
    }
    else if (Desc.IsSFlt()) {
        const float Val = Rec.GetFieldSFlt(FieldId);
        return scope.Escape(Nan::New(Val));
    }
    else if (Desc.IsFltPr()) {
        const TFltPr FltPr = Rec.GetFieldFltPr(FieldId);
        v8::Local<v8::Array> JsFltPr = v8::Array::New(Isolate, 2);
        Nan::Set(JsFltPr, 0, Nan::New(FltPr.Val1));
        Nan::Set(JsFltPr, 1, Nan::New(FltPr.Val2));
        return scope.Escape(JsFltPr);
    }
    else if (Desc.IsFltV()) {
        TFltV FltV; Rec.GetFieldFltV(FieldId, FltV);
        return scope.Escape(TNodeJsVec<TFlt, TAuxFltV>::New(FltV));
    }
    else if (Desc.IsTm()) {
        TTm FieldTm; Rec.GetFieldTm(FieldId, FieldTm);
        if (FieldTm.IsDef()) {
            // milliseconds from 1601-01-01T00:00:00Z
            double WinMSecs = (double)TTm::GetMSecsFromTm(FieldTm);
            // milliseconds from 1970-01-01T00:00:00Z, which is 11644473600 seconds after Windows file time start
            double UnixMSecs = (double)TNodeJsUtil::GetJsTimestamp((uint64)WinMSecs);
            v8::Local<v8::Date> DateObj = TNodeJsUtil::ToLocal(Nan::New<v8::Date>(UnixMSecs));
            return scope.Escape(DateObj);
        }
        else {
            return Nan::Null();
        }
    }
    else if (Desc.IsNumSpV()) {
        TIntFltKdV SpV; Rec.GetFieldNumSpV(FieldId, SpV);
        return scope.Escape(TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(SpV)));
    }
    else if (Desc.IsBowSpV()) {
        throw TQm::TQmExcept::New("Store::Field BowSpV not implemented");
    } else if (Desc.IsTMem()) {
        TMem Val;
        Rec.GetFieldTMem(FieldId, Val);
        v8::Local<v8::Object> Bf = TNodeJsUtil::NewBuffer(Val.GetBf(), Val.Len());
        return scope.Escape(Bf);
    } else if (Desc.IsJson()) {
        PJsonVal Val = Rec.GetFieldJsonVal(FieldId);
        return scope.Escape(TNodeJsUtil::ParseJson(Isolate, Val));
    }
    throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

v8::Local<v8::Value> TNodeJsStore::Field(const TWPt<TQm::TStore>& Store, const uint64& RecId, const int FieldId) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
    // check if null
    if (!Store->IsRecId(RecId)) {
        return Nan::Null();
    }
    // check if field is null
    if (Store->IsFieldNull(RecId, FieldId)) {
        return Nan::Null();
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
            Args.GetReturnValue().Set(Nan::Undefined());
        }
    }
    catch (const PExcept& Except) {
        throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
    }
}

void TNodeJsStore::each(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

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

            Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), Argc, ArgV);
            TNodeJsUtil::CheckJSExcept(TryCatch);
        } while (Iter->Next());
    }
}

void TNodeJsStore::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "each: Argument 0 should be a function!");

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);

    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());

    const TWPt<TQm::TStore> Store = JsStore->Store;
    const int Recs = (int)Store->GetRecs();

    v8::Local<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

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

            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, Argc, ArgV);
            TNodeJsUtil::CheckJSExcept(TryCatch);
            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> ReturnVal = TNodeJsUtil::ToLocal(Tmp);
            Nan::Set(ResultV, Count, ReturnVal);
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

        const PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
        const bool TriggerEvents = TNodeJsUtil::GetArgBool(Args, 1, true);

        const uint64 RecId = Store->AddRec(RecVal, TriggerEvents);

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
        QmAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsIntV::GetClassId().CStr()),
            "Store.getRecSetByIdV: The first argument must be a TIntV (js linalg full int vector)");
        TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
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
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("id")),
                v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("name")),
                TNodeJsUtil::ToLocal(Nan::New(FieldDesc.GetFieldNm().CStr())));
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("type")),
                TNodeJsUtil::ToLocal(Nan::New(FieldDesc.GetFieldTypeStr().CStr())));
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("nullable")),
                Nan::New(FieldDesc.IsNullable()));
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("internal")),
                Nan::New(FieldDesc.IsInternal()));
            Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("primary")),
                Nan::New(FieldDesc.IsPrimary()));

            Args.GetReturnValue().Set(Field);
        }
        else {
            Args.GetReturnValue().Set(Nan::Null());
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

        Args.GetReturnValue().Set(Nan::New(FldDesc.IsFlt() || FldDesc.IsSFlt() || FldDesc.IsInt() || FldDesc.IsInt16() || FldDesc.IsInt64() || FldDesc.IsByte() || FldDesc.IsUInt() || FldDesc.IsUInt16() || FldDesc.IsUInt64()));
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

        Args.GetReturnValue().Set(Nan::New(FldDesc.IsStr()));
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

        Args.GetReturnValue().Set(Nan::New(FldDesc.IsTm()));
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
            Args.GetReturnValue().Set(Nan::Null());
        }
    }
    catch (const PExcept& Except) {
        throw TQm::TQmExcept::New("[except] " + Except->GetMsgStr());
    }
}

void TNodeJsStore::resetStreamAggregates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
    TWPt<TQm::TStore>& Store = JsStore->Store;
    const TWPt<TQm::TBase>& Base = JsStore->Store->GetBase();

    Base->GetStreamAggrSet(Store->GetStoreId())->Reset();
}

void TNodeJsStore::getStreamAggrNames(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
        TWPt<TQm::TStore>& Store = JsStore->Store;
        const TWPt<TQm::TBase>& Base = JsStore->Store->GetBase();

        // get list of names
        TStrV StreamAggrNmV = Base->GetStreamAggrSet(Store->GetStoreId())->GetStreamAggrNmV();
        // set list as return value
        Args.GetReturnValue().Set(TNodeJsUtil::GetStrArr(StreamAggrNmV));
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
        if (TNodeJsUtil::IsArg(Args, 0)) {
            const int DelRecs = TNodeJsUtil::GetArgInt32(Args, 0, (int)JsStore->Store->GetRecs());
            JsStore->Store->DeleteFirstRecs(DelRecs);
        } else {
            JsStore->Store->DeleteAllRecs();
        }
        Args.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
        return;
    } catch (const PExcept& Except) {
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
        else if (Desc.IsInt16()) {
            TIntV ColV(Recs);

            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = JsStore->Store->GetFieldInt16(Iter->GetRecId(), FieldId);
                Iter->Next();
            }

            Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
            return;
        }
        else if (Desc.IsInt64()) {
            TFltV ColV(Recs);

            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = (double) JsStore->Store->GetFieldInt64(Iter->GetRecId(), FieldId);
                Iter->Next();
            }

            Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
            return;
        }
        else if (Desc.IsByte()) {
            TIntV ColV(Recs);

            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = JsStore->Store->GetFieldByte(Iter->GetRecId(), FieldId);
                Iter->Next();
            }

            Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
            return;
        }
        else if (Desc.IsUInt()) {
            TFltV ColV(Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = (double)JsStore->Store->GetFieldUInt(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
            return;
        }
        else if (Desc.IsUInt16()) {
            TFltV ColV(Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = (double)JsStore->Store->GetFieldUInt16(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
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
        else if (Desc.IsSFlt()) {
            TFltV ColV(Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV[RecN] = JsStore->Store->GetFieldSFlt(Iter->GetRecId(), FieldId);
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
                ColV[RecN] = (double) TNodeJsUtil::GetJsTimestamp(TTm::GetMSecsFromTm(Tm));
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
        else if (Desc.IsInt16()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = (double)JsStore->Store->GetFieldInt16(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
            return;
        } else if (Desc.IsInt64()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = (double)JsStore->Store->GetFieldInt64(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
            return;
        } else if (Desc.IsByte()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = (double)JsStore->Store->GetFieldByte(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
            return;
        } else if (Desc.IsUInt()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = (double)JsStore->Store->GetFieldUInt(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
            return;
        } else if (Desc.IsUInt16()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = (double)JsStore->Store->GetFieldUInt16(Iter->GetRecId(), FieldId);
                Iter->Next();
            }
            Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
            return;
        } else if (Desc.IsUInt64()) {
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
        else if (Desc.IsSFlt()) {
            TFltVV ColV(1, Recs);
            TQm::PStoreIter Iter = Store->ForwardIter(); Iter->Next();
            for (int RecN = 0; RecN < Recs; RecN++) {
                ColV.At(0, RecN) = JsStore->Store->GetFieldSFlt(Iter->GetRecId(), FieldId);
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
            Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(ColV)));
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

void TNodeJsStore::triggerOnAddCallbacks(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Args.Holder());
    TWPt<TQm::TStore> Store = JsStore->Store;
    QmAssertR((Args.Length() == 1) || (Args.Length() == 0 && Store->GetRecs() > 0), "Store.triggerOnAddCallbacks: at most one argument expected!");

    if (Args.Length() == 1) {
        if (Args[0]->IsInt32()) {
            int RecId = TNodeJsUtil::GetArgInt32(Args, 0);
            Store->OnAdd(RecId);
        } else {
            TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
            Store->OnAdd(JsRec->Rec);
        }
    } else {
        Store->OnAdd(Store->GetLastRecId());
    }
}

void TNodeJsStore::name(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);
    Info.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(JsStore->Store->GetStoreNm().CStr())));
}

void TNodeJsStore::empty(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    Info.GetReturnValue().Set(Nan::New(JsStore->Store->Empty()));
}

void TNodeJsStore::length(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsStore->Store->GetRecs()));
}

void TNodeJsStore::allRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    if (!JsStore->Store->HasGetAllRecs()) { return; }

    TQm::PRecSet ResultSet = JsStore->Store->GetAllRecs();
    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(ResultSet, JsStore->Watcher)));
}

void TNodeJsStore::fields(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    v8::Local<v8::Array> FieldV = v8::Array::New(Isolate, JsStore->Store->GetFields());
    for (int FieldId = 0; FieldId < JsStore->Store->GetFields(); FieldId++) {
        const TQm::TFieldDesc& FieldDesc = JsStore->Store->GetFieldDesc(FieldId);
        v8::Local<v8::Object> Field = v8::Object::New(Isolate);
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("id")),
            v8::Int32::New(Isolate, FieldDesc.GetFieldId()));
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("name")),
            TNodeJsUtil::ToLocal(Nan::New(FieldDesc.GetFieldNm().CStr())));
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("type")),
            TNodeJsUtil::ToLocal(Nan::New(FieldDesc.GetFieldTypeStr().CStr())));
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("nullable")),
            Nan::New(FieldDesc.IsNullable()));
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("internal")),
            Nan::New(FieldDesc.IsInternal()));
        Nan::Set(Field, TNodeJsUtil::ToLocal(Nan::New("primary")),
            Nan::New(FieldDesc.IsPrimary()));
        Nan::Set(FieldV, Nan::New(FieldId), Field);
    }
    Info.GetReturnValue().Set(FieldV);
}

void TNodeJsStore::joins(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    v8::Local<v8::Array> JoinV = v8::Array::New(Isolate, JsStore->Store->GetJoins());
    for (int JoinId = 0; JoinId < JsStore->Store->GetJoins(); JoinId++) {
        const TQm::TJoinDesc& JoinDesc = JsStore->Store->GetJoinDesc(JoinId);
        TWPt<TQm::TStore> JoinStore = JoinDesc.GetJoinStore(JsStore->Store->GetBase());
        v8::Local<v8::Object> Join = v8::Object::New(Isolate);
        Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("id")),
            v8::Int32::New(Isolate, JoinDesc.GetJoinId()));
        Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("name")),
            TNodeJsUtil::ToLocal(Nan::New(JoinDesc.GetJoinNm().CStr())));
        Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("store")),
            TNodeJsUtil::ToLocal(Nan::New(JoinStore->GetStoreNm().CStr())));
        if (JoinDesc.IsInverseJoinId()) {
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("inverse")),
                TNodeJsUtil::ToLocal(Nan::New(JoinStore->GetJoinNm(JoinDesc.GetInverseJoinId()).CStr())));
        }
        if (JoinDesc.IsFieldJoin()) {
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("type")),
                TNodeJsUtil::ToLocal(Nan::New("field")));
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("recordField")),
                TNodeJsUtil::ToLocal(Nan::New(JsStore->Store->GetFieldNm(JoinDesc.GetJoinRecFieldId()).CStr())));
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("weightField")),
                TNodeJsUtil::ToLocal(Nan::New(JsStore->Store->GetFieldNm(JoinDesc.GetJoinFqFieldId()).CStr())));
        }
        else if (JoinDesc.IsIndexJoin()) {
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("type")),
                TNodeJsUtil::ToLocal(Nan::New("index")));
            TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
            Nan::Set(Join, TNodeJsUtil::ToLocal(Nan::New("key")),
                TNodeJsUtil::NewInstance<TNodeJsIndexKey>(new TNodeJsIndexKey(
                JsStore->Store, IndexVoc->GetKey(JoinDesc.GetJoinKeyId()), JsStore->Watcher)));
        }
        Nan::Set(JoinV, Nan::New(JoinId), Join);
    }
    Info.GetReturnValue().Set(JoinV);
}

void TNodeJsStore::keys(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    TWPt<TQm::TIndexVoc> IndexVoc = JsStore->Store->GetBase()->GetIndexVoc();
    TIntSet KeySet = IndexVoc->GetStoreKeys(JsStore->Store->GetStoreId());
    TIntV Vec;  KeySet.GetKeyV(Vec);
    v8::Local<v8::Array> KeyNmV = v8::Array::New(Isolate, KeySet.Len());
    for (int KeyN = 0; KeyN < Vec.Len(); KeyN++) {
        Nan::Set(KeyNmV, KeyN,
            TNodeJsUtil::NewInstance<TNodeJsIndexKey>(
                new TNodeJsIndexKey(JsStore->Store, IndexVoc->GetKey(Vec[KeyN]), JsStore->Watcher)));
    }
    Info.GetReturnValue().Set(KeyNmV);
}

void TNodeJsStore::first(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    if (!JsStore->Store->HasFirstRecId()) { return; }

    const uint64 FirstRecId = JsStore->Store->GetFirstRecId();
    if (FirstRecId == TUInt64::Mx) {
        Args.GetReturnValue().Set(Nan::Null());
        return;
    }
    Args.GetReturnValue().Set(
        TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(FirstRecId)))
    );
}

void TNodeJsStore::last(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    if (!JsStore->Store->HasLastRecId()) { return; }

    const uint64 LastRecId = JsStore->Store->GetLastRecId();
    if (LastRecId == TUInt64::Mx) {
        Args.GetReturnValue().Set(Nan::Null());
        return;
    }
    Args.GetReturnValue().Set(
        TNodeJsRec::NewInstance(new TNodeJsRec(JsStore->Watcher, JsStore->Store->GetRec(LastRecId)))
    );
}

void TNodeJsStore::forwardIter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    if (!JsStore->Store->HasForwardIter()) { return; }

    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsStoreIter>(
        new TNodeJsStoreIter(JsStore->Store, JsStore->Store->ForwardIter(), JsStore->Watcher)));
}

void TNodeJsStore::backwardIter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);

    if (!JsStore->Store->HasBackwardIter()) { return; }

    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsStoreIter>(
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

    Info.GetReturnValue().Set(Nan::Null());
}

void TNodeJsStore::base(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsStore* JsStore = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsStore>(Self);
    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsBase>(new TNodeJsBase(JsStore->Store->GetBase())));

}

///////////////////////////////
// NodeJs QMiner Record
TVec<TVec<v8::Persistent<v8::Function> > > TNodeJsRec::BaseStoreIdConstructor;

void TNodeJsRec::Init(const TWPt<TQm::TStore>& Store) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    QmAssertR(TNodeJsQm::BaseFPathToId.IsKey(Store->GetBase()->GetFPath()), "Base Id not found in TNodeJsRec::Init.");
    uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(Store->GetBase()->GetFPath());
    if (BaseStoreIdConstructor.Empty()) {
        // support 16 bases opened!
        BaseStoreIdConstructor.Gen(TNodeJsBase::MAX_BASES);
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
        tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
        // ObjectWrap uses the first internal field to store the wrapped pointer.
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        // Add all prototype methods, getters and setters here.
        NODE_SET_PROTOTYPE_METHOD(tpl, "$clone", _clone);
        NODE_SET_PROTOTYPE_METHOD(tpl, "$addJoin", _addJoin);
        NODE_SET_PROTOTYPE_METHOD(tpl, "$delJoin", _delJoin);
        NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);

        // Properties
        tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("$id")), _id);
        tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("$name")), _name);
        tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("$fq")), _fq);
        tpl->InstanceTemplate()->SetAccessorProperty(
            TNodeJsUtil::ToLocal(Nan::New("$store")),
            v8::FunctionTemplate::New(Isolate, _store)
        );

        // register all the fields
        for (int FieldN = 0; FieldN < Store->GetFields(); FieldN++) {
            TStr FieldNm = Store->GetFieldDesc(FieldN).GetFieldNm();
            tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New(FieldNm.CStr())), _getField, _setField);
        }

        for (int JoinId = 0; JoinId < Store->GetJoins(); JoinId++) {
            const TQm::TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
            if (JoinDesc.IsFieldJoin()) {
                tpl->InstanceTemplate()->SetAccessorProperty(
                    TNodeJsUtil::ToLocal(Nan::New(JoinDesc.GetJoinNm().CStr())),
                    v8::FunctionTemplate::New(Isolate, _sjoin, TNodeJsUtil::ToLocal(Nan::New(JoinDesc.GetJoinNm().CStr())))
                );
            } else if (JoinDesc.IsIndexJoin()) {
                tpl->InstanceTemplate()->SetAccessorProperty(
                    TNodeJsUtil::ToLocal(Nan::New(JoinDesc.GetJoinNm().CStr())),
                    v8::FunctionTemplate::New(Isolate, _join, TNodeJsUtil::ToLocal(Nan::New(JoinDesc.GetJoinNm().CStr())))
                );
            }
            else {
                TQm::ErrorLog("Unknown join type " + Store->GetStoreNm() + "." + JoinDesc.GetJoinNm());
            }
        }

        // This has to be last, otherwise the properties won't show up on the object in JavaScript.
        BaseStoreIdConstructor[BaseId][(int)StoreId].Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
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

    const uint BaseId = TNodeJsQm::BaseFPathToId.GetDat(Rec.GetStore()->GetBase()->GetFPath());
    const int StoreId = (int) Rec.GetStoreId();

    EAssertR(!BaseStoreIdConstructor[BaseId][StoreId].IsEmpty(),
        "TNodeJsRec::NewInstance: constructor is empty. Did you call TNodeJsRec::Init(exports)?");

    v8::Persistent<v8::Function>& PersCons = BaseStoreIdConstructor[BaseId][StoreId];
    v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, PersCons);
    v8::MaybeLocal<v8::Object> MaybeInstance = Cons->NewInstance(Isolate->GetCurrentContext());
    v8::Local<v8::Object> Instance;
    EAssertR(MaybeInstance.ToLocal(&Instance), "TNodeJsRec::NewInstance: failed to create instance (empty handle)");

    const int IntenalFldCount = Instance->InternalFieldCount();
    EAssertR(IntenalFldCount > 0, "TNodeJsRec::NewInstance: constructor has " + TInt::GetStr(IntenalFldCount) + " internal fields!");
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
    QmAssertR(Args.Length() >= 2 && (TNodeJsUtil::IsArgWrapObj(Args, 1, TNodeJsRec::GetClassId()) || Args[1]->IsInt32()),
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
        TNodeJsRec* JsJoinRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
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
    QmAssertR(Args.Length() >= 2 && (TNodeJsUtil::IsArgWrapObj(Args, 1, TNodeJsRec::GetClassId()) || Args[1]->IsInt32()),
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
        TNodeJsRec* JsJoinRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
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
    const bool RecInfoP = TNodeJsUtil::IsArg(Args, 2) && TNodeJsUtil::IsArgBool(Args, 2) ?
            TNodeJsUtil::GetArgBool(Args, 2) : true;

    const bool FieldsP = true;
    const bool StoreInfoP = false;

    PJsonVal RecJson = JsRec->Rec.GetJson(JsRec->Rec.GetStore()->GetBase(), FieldsP, StoreInfoP, JoinRecsP, JoinRecFieldsP, RecInfoP);
    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, RecJson));
}


void TNodeJsRec::id(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsRec->Rec.GetRecId()));
}

void TNodeJsRec::name(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    Info.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(JsRec->Rec.GetRecNm().CStr())));
}

void TNodeJsRec::fq(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRec->Fq));
}

void TNodeJsRec::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsRec->Rec.GetStore(), JsRec->Watcher))
        );

}

void TNodeJsRec::getField(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    const TQm::TRec& Rec = JsRec->Rec;
    const TWPt<TQm::TStore>& Store = Rec.GetStore();
    const int FieldId = Store->GetFieldId(TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Name))));

    Info.GetReturnValue().Set(TNodeJsStore::Field(Rec, FieldId));
}

void TNodeJsRec::setField(v8::Local<v8::Name> Name, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<void>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    TQm::TRec& Rec = JsRec->Rec;
    const TWPt<TQm::TStore>& Store = Rec.GetStore();
    TStr FieldNm = TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Name)));
    const int FieldId = Store->GetFieldId(FieldNm);
    //TODO: for now we don't support by-value records, fix this
    //QmAssertR(Rec.IsByRef(), "Only records by reference (from stores) supported for setters.");
    // not null, get value
    const TQm::TFieldDesc& Desc = Store->GetFieldDesc(FieldId);
    if (Value->IsNull()) {
        QmAssertR(Desc.IsNullable(), "Field " + FieldNm + " not nullable");
        Rec.SetFieldNull(FieldId);
    }
    else if (Desc.IsInt()) {
        QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not integer");
        const int Int = Nan::To<int>(Value).FromJust();
        Rec.SetFieldInt(FieldId, Int);
    }
    else if (Desc.IsInt16()) {
        QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not integer");
        const int Int = Nan::To<int>(Value).FromJust();
        Rec.SetFieldInt16(FieldId, (int16)Int);
    }
    else if (Desc.IsInt64()) {
        QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not integer");
        const int64 Int = Nan::To<int64>(Value).FromJust();
        Rec.SetFieldInt64(FieldId, Int);
    }
    else if (Desc.IsByte()) {
        QmAssertR(Value->IsInt32(), "Field " + FieldNm + " not integer");
        const int Int = Nan::To<int>(Value).FromJust();
        Rec.SetFieldByte(FieldId, (uchar)Int);
    }
    else if (Desc.IsIntV()) {
        // check if we have JavaScript array
        QmAssertR(Value->IsArray(), "rec." + FieldNm + " should be set to an array!");
        v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(Value);
        TIntV IntV;
        for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
            v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, FltN));
            QmAssertR(ArrayVal->IsInt32(), "Field " + FieldNm + " expects array of integers");
            const int Val = Nan::To<int>(ArrayVal).FromJust();
            IntV.Add(Val);
        }
        Rec.SetFieldIntV(FieldId, IntV);
    }
    else if (Desc.IsUInt()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not uint64");
        const uint UInt = Nan::To<uint>(Value).FromJust();
        Rec.SetFieldUInt64(FieldId, UInt);
    }
    else if (Desc.IsUInt16()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not uint64");
        const uint16 UInt16 = (uint16)Nan::To<int>(Value).FromJust();
        Rec.SetFieldUInt64(FieldId, UInt16);
    }
    else if (Desc.IsUInt64()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not uint64");
        const uint64 UInt64 = (uint64)Nan::To<int>(Value).FromJust();
        Rec.SetFieldUInt64(FieldId, UInt64);
    }
    else if (Desc.IsStr()) {
        QmAssertR(Value->IsString(), "Field " + FieldNm + " not string");
        Nan::Utf8String Utf8(Value);
        Rec.SetFieldStr(FieldId, TStr(*Utf8));
    }
    else if (Desc.IsStrV()) {
        QmAssertR(Value->IsArray(), "Field " + FieldNm + " not array");
        v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(Value);
        TStrV StrV;
        for (uint32_t StrN = 0; StrN < Array->Length(); StrN++) {
            v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, StrN));
            QmAssertR(ArrayVal->IsString(), "Field " + FieldNm + " expects array of strings");
            Nan::Utf8String Utf8(ArrayVal);
            StrV.Add(TStr(*Utf8));
        }
        Rec.SetFieldStrV(FieldId, StrV);
    }
    else if (Desc.IsBool()) {
        QmAssertR(Value->IsBoolean(), "Field " + FieldNm + " not boolean");
        Rec.SetFieldBool(FieldId, Nan::To<bool>(Value).FromJust());
    }
    else if (Desc.IsFlt()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not numeric");
        TFlt Val(Nan::To<double>(Value).FromJust());
        bool NaNFound = Val.IsNan();
        if (NaNFound) {
            throw TQm::TQmExcept::New("Cannot set record field (type float) to NaN, for field name: " + FieldNm);
        }
        Rec.SetFieldFlt(FieldId, Val);
    }
    else if (Desc.IsSFlt()) {
        QmAssertR(Value->IsNumber(), "Field " + FieldNm + " not numeric");
        TSFlt Val((float)Nan::To<double>(Value).FromJust());
        bool NaNFound = Val.IsNan();
        if (NaNFound) {
            throw TQm::TQmExcept::New("Cannot set record field (type float) to NaN, for field name: " + FieldNm);
        }
        Rec.SetFieldSFlt(FieldId, Val);
    }
    else if (Desc.IsFltPr()) {
        QmAssertR(Value->IsArray(), "Field " + FieldNm + " not array");
        v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(Value);
        QmAssert(Array->Length() >= 2);
        QmAssert(TNodeJsUtil::ToLocal(Nan::Get(Array, 0))->IsNumber());
        QmAssert(TNodeJsUtil::ToLocal(Nan::Get(Array, 1))->IsNumber());
        TFltPr FltPr(
            Nan::To<double>(TNodeJsUtil::ToLocal(Nan::Get(Array, 0))).FromJust(),
            Nan::To<double>(TNodeJsUtil::ToLocal(Nan::Get(Array, 1))).FromJust()
        );
        Rec.SetFieldFltPr(FieldId, FltPr);
    }
    else if (Desc.IsFltV()) {
        if (Value->IsArray()) {
            // check if we have JavaScript array
            v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(Value);
            TFltV FltV;
            for (uint32_t FltN = 0; FltN < Array->Length(); FltN++) {
                v8::Local<v8::Value> ArrayVal = TNodeJsUtil::ToLocal(Nan::Get(Array, FltN));
                QmAssertR(ArrayVal->IsNumber(), "Field " + FieldNm + " expects array of numbers");
                const double Val = Nan::To<double>(ArrayVal).FromJust();
                FltV.Add(Val);
            }
            Rec.SetFieldFltV(FieldId, FltV);
        }
        else {
            // otherwise it must be GLib array (or exception)
            TNodeJsVec<TFlt, TAuxFltV>* JsFltV = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Value)));
            Rec.SetFieldFltV(FieldId, JsFltV->Vec);
        }
    }
    else if (Desc.IsTm()) {
        QmAssertR(Value->IsObject() || Value->IsString() || Value->IsNumber(), "Field " + FieldNm + " not object or string");
        Rec.SetFieldTmMSecs(FieldId, TNodeJsUtil::GetTmMSecs(Value));
    }
    else if (Desc.IsNumSpV()) {
        // it can only be GLib sparse vector
        TNodeJsSpVec* JsSpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Value)));
        Rec.SetFieldNumSpV(FieldId, JsSpVec->Vec);
    }
    else if (Desc.IsBowSpV()) {
        throw TQm::TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
    }
    else if (Desc.IsTMem()) {
        QmAssertR(Value->IsObject(), "Field " + FieldNm + " not object");

        v8::Local<v8::Object> Object = v8::Local<v8::Object>::Cast(Value);
        QmAssertR(TNodeJsUtil::IsBuffer(Object), "TNodeJsRec::setField: argument not a buffer");

        char* Buff = node::Buffer::Data(Object);
        size_t BuffLen = node::Buffer::Length(Object);

        TMem Mem;
        Mem.AddBf(Buff, (int)BuffLen);
        Rec.SetFieldTMem(FieldId, Mem);
    }
    else if (Desc.IsJson()) {
        QmAssertR(Value->IsObject(), "Field " + FieldNm + " not an object");
        v8::Local<v8::Object> Object = v8::Local<v8::Object>::Cast(Value);
        PJsonVal Json = TNodeJsUtil::GetObjProps(Object);
        Rec.SetFieldJsonVal(FieldId, Json);
    }
    else {
        throw TQm::TQmExcept::New("Unsupported type for record setter: " + Desc.GetFieldTypeStr());
    }
}


void TNodeJsRec::join(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    QmAssert(Args.Data()->IsString());
    TStr JoinNm = TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args.Data())));
    TQm::PRecSet RecSet = JsRec->Rec.DoJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsRecSet>(new TNodeJsRecSet(RecSet, JsRec->Watcher)));
}

void TNodeJsRec::sjoin(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Args.Holder();
    TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(Self);
    QmAssert(Args.Data()->IsString());
    TStr JoinNm = TNodeJsUtil::GetStr(TNodeJsUtil::ToLocal(Nan::To<v8::String>(Args.Data())));
    TQm::TRec JoinRec = JsRec->Rec.DoSingleJoin(JsRec->Rec.GetStore()->GetBase(), JoinNm);
    TWPt<TQm::TStore> JoinStore = JoinRec.GetStore();
    if (JoinRec.IsDef() && JoinStore->IsRecId(JoinRec.GetRecId())) {
        Args.GetReturnValue().Set(
            TNodeJsRec::NewInstance(new TNodeJsRec(JsRec->Watcher, JoinRec, JoinRec.GetRecFq())));
        return;
    } else {
        Args.GetReturnValue().Set(Nan::Null());
        return;
    }
}

///////////////////////////////
// NodeJs QMiner Record Vector
v8::Persistent<v8::Function> TNodeJsRecByValV::Constructor;

void TNodeJsRecByValV::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsRecByValV>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "push", _push);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("length")), _length);
    tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);

    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

TNodeJsRecByValV* TNodeJsRecByValV::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // check we have at leat one object parameter
    QmAssertR(Args.Length() > 0 && Args[0]->IsObject(),
        "RecordVector: expected at least `base` parameter in constructor");

    // unwrap the base
    TNodeJsBase* JsBase = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    // create empty vector
    TNodeJsRecByValV* JsRecV = new TNodeJsRecByValV(JsBase->Watcher);

    if (Args.Length() == 1) {
        // we just return empty vector
        return JsRecV;
    } else if (Args.Length() == 2 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 1)) {
        // load vector from disk
        TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])));
        int Recs; JsFIn->SIn->Load(Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            JsRecV->RecV.Add(TQm::TRec(JsBase->Base, *JsFIn->SIn));
        }
        // and return it
        return JsRecV;
    } else {
        // unknown construtor, complain
        throw TExcept::New("RecordVector: Unknown constructor parameters");
    }
}

void TNodeJsRecByValV::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecByValV* JsRecV = ObjectWrap::Unwrap<TNodeJsRecByValV>(Args.Holder());
    TNodeJsRec* JsRec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsRec>(Args, 0);

    EAssertR(JsRec->Rec.IsByVal(), "RecordVector::push: the record is not by value!");
    JsRecV->RecV.Add(JsRec->Rec);

    Args.GetReturnValue().Set(Nan::Undefined());
}

void TNodeJsRecByValV::length(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecByValV* JsRecV = ObjectWrap::Unwrap<TNodeJsRecByValV>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRecV->RecV.Len()));
}


void TNodeJsRecByValV::indexId(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecByValV* JsRecV = ObjectWrap::Unwrap<TNodeJsRecByValV>(Info.Holder());

    // check if we ask for valid index
    const int RecN = Index;
    if (0 <= RecN && RecN < JsRecV->RecV.Len()) {
        // if yes, create JS wrapper around record
        Info.GetReturnValue().Set(TNodeJsRec::NewInstance(
            new TNodeJsRec(JsRecV->Watcher, JsRecV->RecV[RecN], 1)));
    } else {
        // if not, return null
        Info.GetReturnValue().Set(Nan::Null());
    }
}

void TNodeJsRecByValV::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecByValV* JsRecV = ObjectWrap::Unwrap<TNodeJsRecByValV>(Args.Holder());
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");

    // Cannot serialize directly as vector, since there is no TRec(SIn) constructor
    // that we can use to load it later. We serialize manually
    JsFOut->SOut->Save(int(JsRecV->RecV.Len()));
    for (const TQm::TRec& Rec : JsRecV->RecV) {
        Rec.Save(*JsFOut->SOut);
    }

    Args.GetReturnValue().Set(Args[0]);
}

///////////////////////////////
// NodeJs QMiner Record Set
v8::Persistent<v8::Function> TNodeJsRecSet::Constructor;

void TNodeJsRecSet::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsRecSet>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
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
    NODE_SET_PROTOTYPE_METHOD(tpl, "sort", _sort);
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
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("store")), v8::FunctionTemplate::New(Isolate, _store));
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("length")), _length);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("empty")), _empty);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("weighted")), _weighted);
    tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexId);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript.
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
    // So we can add stuff to the prototype in JS
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
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
            Args.GetReturnValue().Set(Nan::Null());
            return;
        }
        // compute new aggregates
        v8::Local<v8::Array> AggrValV = v8::Array::New(Isolate, QueryAggrV.Len());
        for (int QueryAggrN = 0; QueryAggrN < QueryAggrV.Len(); QueryAggrN++) {
            const TQm::TQueryAggr& QueryAggr = QueryAggrV[QueryAggrN];
            // compute aggregate
            TQm::PAggr Aggr = TQm::TAggr::New(Base, RecSet, QueryAggr);
            // serialize to json
            Nan::Set(AggrValV, QueryAggrN, TNodeJsUtil::ParseJson(Isolate, Aggr->SaveJson()));
        }
        // return aggregates
        if (AggrValV->Length() == 1) {
            // if only one, return as object
            if (TNodeJsUtil::ToLocal(Nan::Get(AggrValV, 0))->IsObject()) {
                Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::Get(AggrValV, 0)));
                return;
            }
            else {
                Args.GetReturnValue().Set(Nan::Null());
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
                Args.GetReturnValue().Set(Nan::Null());
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
    TQm::PRecSet ResultSet = JsRecSet->RecSet->GetSampleRecSet(SampleSize);

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
    } else if (Args.Length() == 2) {
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
    const int MxFq = TNodeJsUtil::GetArgInt32(Args, 1, TInt::Mx);
    JsRecSet->RecSet->FilterByFq(MnFq, MxFq);

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::filterByField(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

    // get field
    const TStr FieldNm = TNodeJsUtil::GetArgStr(Args, 0);
    int FieldId;
    bool IsFieldJoin = false;
    bool IsIndexJoin = false;
    const TWPt<TQm::TStore>& Store = JsRecSet->RecSet->GetStore();
    if (Store->IsFieldNm(FieldNm)) {
        // normal field
        FieldId = Store->GetFieldId(FieldNm);
    } else {
        // check if this is field join
        bool is_ok = false;
        if (Store->IsJoinNm(FieldNm)) {
            const int JoinId = Store->GetJoinId(FieldNm);
            const TQm::TJoinDesc& JoinDesc = Store->GetJoinDesc(JoinId);
            if (JoinDesc.IsFieldJoin()) {
                FieldId = JoinDesc.GetJoinRecFieldId();
                is_ok = true;
                IsFieldJoin = true;
            } else {
                FieldId = JoinId; // this is not field id, but join id
                IsIndexJoin = true;
                is_ok = true;
            }
        } else {
            //is_ok = false;
        }
        QmAssertR(is_ok, "RecordSet.filterByField: invalid field name " + FieldNm);
    }

    const TQm::TFieldDesc& Desc = JsRecSet->RecSet->GetStore()->GetFieldDesc(FieldId);
    // parse filter according to field type
    if (IsIndexJoin) {
        uint64 MnVal = TUInt64::Mn;
        uint64 MxVal = TUInt64::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = static_cast<uint64> (TNodeJsUtil::GetArgFlt(Args, 1));
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = static_cast<uint64> (TNodeJsUtil::GetArgFlt(Args, 2));
        }
        JsRecSet->RecSet->FilterByIndexJoin(Store->GetBase(), FieldId, MnVal, MxVal);
    } else if (IsFieldJoin) {
        uint64 MnVal = TUInt64::Mn;
        uint64 MxVal = TUInt64::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = static_cast<uint64> (TNodeJsUtil::GetArgFlt(Args, 1));
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = static_cast<uint64> (TNodeJsUtil::GetArgFlt(Args, 2));
        }
        JsRecSet->RecSet->FilterByFieldSafe(FieldId, MnVal, MxVal);
    } else if (Desc.IsBool()) {
        const bool Val = TNodeJsUtil::GetArgBool(Args, 1);
        JsRecSet->RecSet->FilterByFieldBool(FieldId, Val);
    } else if (Desc.IsInt()) {
        int MnVal = TInt::Mn;
        int MxVal = TInt::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = TNodeJsUtil::GetArgInt32(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = TNodeJsUtil::GetArgInt32(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldInt(FieldId, MnVal, MxVal);
    } else if (Desc.IsInt16()) {
        int16 MnVal = TInt16::Mn;
        int16 MxVal = TInt16::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = (int16) TNodeJsUtil::GetArgInt32(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = (int16) TNodeJsUtil::GetArgInt32(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldInt16(FieldId, MnVal, MxVal);
    } else if (Desc.IsInt64()) {
        int64 MnVal = TInt64::Mn;
        int64 MxVal = TInt64::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = (int64)TNodeJsUtil::GetArgFlt(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = (int64)TNodeJsUtil::GetArgFlt(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldInt64(FieldId, MnVal, MxVal);
    } else if (Desc.IsByte()) {
        uchar MnVal = TUCh::Mn;
        uchar MxVal = TUCh::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = (uchar)TNodeJsUtil::GetArgInt32(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = (uchar)TNodeJsUtil::GetArgInt32(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldByte(FieldId, MnVal, MxVal);
    } else if (Desc.IsStr()) {
        if (Args.Length() < 3 || !TNodeJsUtil::IsArgStr(Args, 2)) {
            TStr StrVal = TNodeJsUtil::GetArgStr(Args, 1);
            JsRecSet->RecSet->FilterByFieldStr(FieldId, StrVal);
        } else {
            TStr StrValMin = TNodeJsUtil::GetArgStr(Args, 1);
            TStr StrValMax = TNodeJsUtil::GetArgStr(Args, 2);
            JsRecSet->RecSet->FilterByFieldStr(FieldId, StrValMin, StrValMax);
        }
    } else if (Desc.IsFlt()) {
        double MnVal = TFlt::Mn;
        double MxVal = TFlt::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = TNodeJsUtil::GetArgFlt(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = TNodeJsUtil::GetArgFlt(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldFlt(FieldId, MnVal, MxVal);
    } else if (Desc.IsSFlt()) {
        float MnVal = TSFlt::Mn;
        float MxVal = TSFlt::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = (float)TNodeJsUtil::GetArgFlt(Args, 1);
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = (float)TNodeJsUtil::GetArgFlt(Args, 2);
        }
        JsRecSet->RecSet->FilterByFieldSFlt(FieldId, MnVal, MxVal);
    } else if (Desc.IsUInt()) {
        uint MnVal = TUInt::Mn;
        uint MxVal = TUInt::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = static_cast<uint> (TNodeJsUtil::GetArgFlt(Args, 1));
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = static_cast<uint> (TNodeJsUtil::GetArgFlt(Args, 2));
        }
        JsRecSet->RecSet->FilterByFieldUInt(FieldId, MnVal, MxVal);
    } else if (Desc.IsUInt16()) {
        uint16 MnVal = TUInt16::Mn;
        uint16 MxVal = TUInt16::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = static_cast<uint16> (TNodeJsUtil::GetArgFlt(Args, 1));
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = static_cast<uint16> (TNodeJsUtil::GetArgFlt(Args, 2));
        }
        JsRecSet->RecSet->FilterByFieldUInt16(FieldId, MnVal, MxVal);
    } else if (Desc.IsUInt64()) {
        uint64 MnVal = TUInt64::Mn;
        uint64 MxVal = TUInt64::Mx;
        if (!TNodeJsUtil::IsArgNull(Args, 1) && TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnVal = static_cast<uint64_t> (TNodeJsUtil::GetArgFlt(Args, 1));
        }
        if (Args.Length() >= 3 && !TNodeJsUtil::IsArgNull(Args, 2) && TNodeJsUtil::IsArgFlt(Args, 2)) {
            MxVal = static_cast<uint64_t> (TNodeJsUtil::GetArgFlt(Args, 2));
        }
        JsRecSet->RecSet->FilterByFieldTm(FieldId, MnVal, MxVal);
    } else if (Desc.IsTm()) {
        uint64 MnTmMSecs = TUInt64::Mn;
        uint64 MxTmMSecs = TUInt64::Mx;

        if (TNodeJsUtil::IsArgNull(Args, 1)) {
            // nothing, default value is ok
        } else if (TNodeJsUtil::IsArgStr(Args, 1)) {
            const TStr MnTmStr = TNodeJsUtil::GetArgStr(Args, 1);
            MnTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MnTmStr, '-', ':', '.', 'T'));
        } else if (TNodeJsUtil::IsArgFlt(Args, 1)) {
            MnTmMSecs = TTm::GetWinMSecsFromUnixMSecs(static_cast<uint64_t> (TNodeJsUtil::GetArgFlt(Args, 1)));
        }

        if (Args.Length() >= 3) {
            // we have upper limit
            if (TNodeJsUtil::IsArgNull(Args, 2)) {
                // nothing, default value is ok
            } else if (TNodeJsUtil::IsArgStr(Args, 2)) {
                const TStr MxTmStr = TNodeJsUtil::GetArgStr(Args, 2);
                MxTmMSecs = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(MxTmStr, '-', ':', '.', 'T'));
            } else if (TNodeJsUtil::IsArgFlt(Args, 2)) {
                MxTmMSecs = TTm::GetWinMSecsFromUnixMSecs(static_cast<uint64_t> (TNodeJsUtil::GetArgFlt(Args, 2)));
            }
        }
        JsRecSet->RecSet->FilterByFieldTm(FieldId, MnTmMSecs, MxTmMSecs);
    } else {
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
        Nan::Set(JsRecSetV, RecSetN, TNodeJsUtil::NewInstance<TNodeJsRecSet>(
            new TNodeJsRecSet(RecSetV[RecSetN], JsRecSet->Watcher)));
    }
    Args.GetReturnValue().Set(JsRecSetV);
}

void TNodeJsRecSet::deleteRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());
    QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "deleteRecords(..) expects a record set as input");
    TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));

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
    v8::TryCatch TryCatch(Isolate);

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

            Nan::Call(Callback, GlobalContext, Argc, ArgV);
            TNodeJsUtil::CheckJSExcept(TryCatch);
        }
    }
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecSet::map(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

    TQm::PRecSet RecSet = JsRecSet->RecSet;
    QmAssertR(TNodeJsUtil::IsArgFun(Args, 0), "map: Argument 0 is not a function!");

    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);
    // iterate through the recset
    const int Recs = RecSet->GetRecs();

    v8::Local<v8::Array> ResultV = v8::Array::New(Isolate, Recs);

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
            v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, GlobalContext, Argc, ArgV);
            TNodeJsUtil::CheckJSExcept(TryCatch);
            TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
            v8::Local<v8::Value> Result = TNodeJsUtil::ToLocal(Tmp);
            Nan::Set(ResultV, RecIdN, Result);
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
    TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;
    QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(),
        "recset.setIntersect: the record sets do not point to the same store!");

    // coputation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
    TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
    TUInt64Set RecIdSet; RecSet1->GetRecIdSet(RecIdSet);
    // second parameter in filter is false -> keep only records in RecIdSet
    const TWPt<TQm::TBase>& Base = RecSet1->GetStore()->GetBase();
    RecSet2->FilterBy<TQm::TRecFilterByRecId>(TQm::TRecFilterByRecId(Base, RecIdSet, true));

    // construct and return new record set from what remains
    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsRecSet>(
        new TNodeJsRecSet(RecSet2, JsRecSet->Watcher)));
}

void TNodeJsRecSet::setUnion(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

    QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "rs.setUnion: first argument expected to be an record set");
    TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
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
    TNodeJsRecSet* ArgJsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
    TQm::PRecSet RecSet1 = ArgJsRecSet->RecSet;

    QmAssertR(JsRecSet->RecSet->GetStore()->GetStoreId() == RecSet1->GetStoreId(),
        "recset.setDiff: the record sets do not point to the same store!");
    // Computation: clone RecSet, get RecIdSet of RecSet1 and filter by it's complement
    TQm::PRecSet RecSet2 = JsRecSet->RecSet->Clone();
    TUInt64Set RecIdSet; RecSet1->GetRecIdSet(RecIdSet);
    //second parameter in filter is false -> keep only records NOT in RecIdSet
    const TWPt<TQm::TBase>& Base = RecSet1->GetStore()->GetBase();
    RecSet2->FilterBy<TQm::TRecFilterByRecId>(TQm::TRecFilterByRecId(Base, RecIdSet, false));
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
        TIntV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldInt(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
        return;
    } else if (Desc.IsInt16()) {
        TIntV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldInt16(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
        return;
    } else if (Desc.IsInt64()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add((double)Store->GetFieldInt64(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    } else if (Desc.IsByte()) {
        TIntV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldByte(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
        return;
    } else if (Desc.IsUInt()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add((double)Store->GetFieldUInt(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    } else if (Desc.IsUInt16()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add((double)Store->GetFieldUInt16(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    } else if (Desc.IsUInt64()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add((double)Store->GetFieldUInt64(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    }
    else if (Desc.IsStr()) {
        TStrV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldStr(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TStr, TAuxStrV>::New(ColV));
        return;
    }
    else if (Desc.IsBool()) {
        TIntV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add((int)Store->GetFieldBool(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TInt, TAuxIntV>::New(ColV));
        return;
    }
    else if (Desc.IsFlt()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldFlt(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    }
    else if (Desc.IsSFlt()) {
        TFltV ColV(Recs, 0);
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                ColV.Add(Store->GetFieldSFlt(RecId, FieldId));
            }
        }
        Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(ColV));
        return;
    }
    else if (Desc.IsTm()) {
        TFltV ColV(Recs, 0);
        TTm Tm;
        for (int RecN = 0; RecN < Recs; RecN++) {
            const uint64 RecId = RecSet()->GetRecId(RecN);
            if (!Store->IsFieldNull(RecId, FieldId)) {
                Store->GetFieldTm(RecSet()->GetRecId(RecN), FieldId, Tm);
                ColV.Add((double)TTm::GetMSecsFromTm(Tm)); // TODO is this correct?? Shouldn't it be UNIX timestamp???
            }
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
    else if (Desc.IsInt16()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = (double)Store->GetFieldInt16(RecSet()->GetRecId(RecN), FieldId);
        }
        Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
        return;
    } else if (Desc.IsInt64()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = (double)Store->GetFieldInt64(RecSet()->GetRecId(RecN), FieldId);
        }
        Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
        return;
    } else if (Desc.IsByte()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = (double)Store->GetFieldByte(RecSet()->GetRecId(RecN), FieldId);
        }
        Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
        return;
    } else if (Desc.IsUInt()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = (double)Store->GetFieldUInt(RecSet()->GetRecId(RecN), FieldId);
        }
        Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
        return;
    } else if (Desc.IsUInt16()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = (double)Store->GetFieldUInt16(RecSet()->GetRecId(RecN), FieldId);
        }
        Args.GetReturnValue().Set(TNodeJsFltVV::New(ColV));
        return;
    } else if (Desc.IsUInt64()) {
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
    else if (Desc.IsSFlt()) {
        TFltVV ColV(1, Recs);
        for (int RecN = 0; RecN < Recs; RecN++) {
            ColV(0, RecN) = Store->GetFieldSFlt(RecSet()->GetRecId(RecN), FieldId);
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
        Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(ColV)));
        return;
    }
    else if (Desc.IsStr()) {
        throw TQm::TQmExcept::New("store.getMatrix does not support type string - use store.getVector instead");
    }
    throw TQm::TQmExcept::New("Unknown field type " + Desc.GetFieldTypeStr());
}

void TNodeJsRecSet::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Args.Holder());

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsRecSet->RecSet->GetStore(), JsRecSet->Watcher))
        );

}

void TNodeJsRecSet::length(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsRecSet->RecSet->GetRecs()));
}

void TNodeJsRecSet::empty(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

    Info.GetReturnValue().Set(Nan::New(JsRecSet->RecSet->Empty()));
}

void TNodeJsRecSet::weighted(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(Info.Holder());

    Info.GetReturnValue().Set(Nan::New(JsRecSet->RecSet->IsFq()));
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

    Info.GetReturnValue().Set(Nan::Null());
}

///////////////////////////////
// NodeJs QMiner Store Iterator
v8::Persistent<v8::Function> TNodeJsStoreIter::Constructor;

void TNodeJsStoreIter::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsStoreIter>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "next", _next);

    // Properties
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("store")), v8::FunctionTemplate::New(Isolate, _store));
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("record")), v8::FunctionTemplate::New(Isolate, _record));

    // This has to be last, otherwise the properties won't show up on the object in JavaScript.
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
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

        JsStoreIter->RecObj.Reset(_RecObj);
        JsStoreIter->JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(_RecObj);
    }
    else if (NextP) {
        // not first time, just update the placeholder
        const uint64 RecId = JsStoreIter->Iter->GetRecId();
        JsStoreIter->JsRec->Rec = JsStoreIter->Store->GetRec(RecId);
    }
    Args.GetReturnValue().Set(Nan::New(NextP));
}

void TNodeJsStoreIter::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Args.Holder());

    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsStoreIter->Store, JsStoreIter->Watcher))
    );
}

void TNodeJsStoreIter::record(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsStoreIter* JsStoreIter = ObjectWrap::Unwrap<TNodeJsStoreIter>(Args.Holder());

    Args.GetReturnValue().Set(JsStoreIter->RecObj);
}

///////////////////////////////
// NodeJs QMiner Record Filter
TJsRecFilter::TJsRecFilter(TWPt<TQm::TStore> _Store, v8::Local<v8::Function> _Callback):
        TRecFilter(_Store->GetBase()), Store(_Store) {

    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // set persistent object
    Callback.Reset(Isolate, _Callback);
}

bool TJsRecFilter::Filter(const TQm::TRec& Rec) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::TryCatch TryCatch(Isolate);

    // prepare record objects - since they are local, they are safe from GC
    v8::Local<v8::Object> JsRec = TNodeJsRec::NewInstance(
        new TNodeJsRec(TNodeJsBaseWatcher::New(), Rec, Rec.GetRecFq()));

    v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
    v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
    const unsigned Argc = 1;
    v8::Local<v8::Value> ArgV[Argc] = { JsRec };

    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callbck, GlobalContext, Argc, ArgV);
    TNodeJsUtil::CheckJSExcept(TryCatch);
    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);

    v8::Local<v8::Value> ReturnVal = TNodeJsUtil::ToLocal(Tmp);
    QmAssertR(ReturnVal->IsBoolean(), "Filter callback must return a boolean!");
    return Nan::To<bool>(ReturnVal).FromJust();
}

///////////////////////////////
// NodeJs QMiner Record Pair
TJsRecPairFilter::TJsRecPairFilter(TWPt<TQm::TStore> _Store, v8::Local<v8::Function> _Callback): Store(_Store) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // set persistent object
    Callback.Reset(Isolate, _Callback);
}

bool TJsRecPairFilter::operator()(const TUInt64IntKd& RecIdWgt1, const TUInt64IntKd& RecIdWgt2) const {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::TryCatch TryCatch(Isolate);

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
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callbck, GlobalContext, Argc, ArgV);
    TNodeJsUtil::CheckJSExcept(TryCatch);
    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
    v8::Local<v8::Value> ReturnVal = TNodeJsUtil::ToLocal(Tmp);
    QmAssertR(!ReturnVal.IsEmpty() && (ReturnVal->IsBoolean() || ReturnVal->IsNumber()), "Comparator callback must return a boolean!");
    return ReturnVal->IsBoolean()
        ? Nan::To<bool>(ReturnVal).FromJust()
        : Nan::To<double>(ReturnVal).FromJust() < 0;
}

///////////////////////////////
// NodeJs QMiner Index Key
v8::Persistent<v8::Function> TNodeJsIndexKey::Constructor;

void TNodeJsIndexKey::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsIndexKey>);
    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.

    // Properties
    tpl->InstanceTemplate()->SetAccessorProperty(TNodeJsUtil::ToLocal(Nan::New("store")), v8::FunctionTemplate::New(Isolate, _store));
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("name")), _name);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("vocabulary")), _vocabulary);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("fq")), _fq);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript.
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

void TNodeJsIndexKey::store(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Args.Holder());
    // return
    Args.GetReturnValue().Set(
        TNodeJsUtil::NewInstance<TNodeJsStore>(new TNodeJsStore(JsIndexKey->Store, JsIndexKey->Watcher))
    );
}

void TNodeJsIndexKey::name(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
    // return
    Info.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(JsIndexKey->IndexKey.GetKeyNm().CStr())));
}

void TNodeJsIndexKey::vocabulary(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
    // preapre vocabulary array
    if (!JsIndexKey->IndexKey.IsWordVoc()) {
        // no vocabulary
        Info.GetReturnValue().Set(Nan::Null());
    }
    else {
        // get vocabulary
        TStrV KeyValV;
        JsIndexKey->Store->GetBase()->GetIndexVoc()->GetAllWordStrV(
            JsIndexKey->IndexKey.GetKeyId(), KeyValV);
        Info.GetReturnValue().Set(TNodeJsVec<TStr, TAuxStrV>::New(KeyValV));
    }
}

void TNodeJsIndexKey::fq(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    // unwrap
    TNodeJsIndexKey* JsIndexKey = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsIndexKey>(Info.Holder());
    // preapre vocabulary array
    if (!JsIndexKey->IndexKey.IsWordVoc()) {
        // no vocabulary
        Info.GetReturnValue().Set(Nan::Null());
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
    const PJsonVal& ParamVal, const v8::Local<v8::Function> _Fun, v8::Isolate* Isolate) :
        TQm::TFtrExt(Base, ParamVal), Dim(ParamVal->GetObjInt("dim", 1)),
        Name(ParamVal->GetObjStr("name", "jsfunc")), Fun() {

    Fun.Reset(Isolate, _Fun);
}

TQm::PFtrExt TNodeJsFuncFtrExt::NewFtrExt(const TWPt<TQm::TBase>& Base,
        const PJsonVal& ParamVal, const v8::Local<v8::Function>& Fun, v8::Isolate* Isolate) {

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
    v8::TryCatch TryCatch(Isolate);
    v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, Fun);
    v8::Local<v8::Value> Argv[1] = { TNodeJsRec::NewInstance(new TNodeJsRec(TNodeJsBaseWatcher::New(), FtrRec)) };
    v8::MaybeLocal<v8::Value> Tmp = Nan::Call(Callback, Isolate->GetCurrentContext()->Global(), 1, Argv);
    TNodeJsUtil::CheckJSExcept(TryCatch);

    TNodeJsUtil::CheckObjEmpty(Isolate, TryCatch, Tmp);
    v8::Local<v8::Value> RetVal = TNodeJsUtil::ToLocal(Tmp);
    // Cast as FltV and copy result
    v8::Local<v8::Object> RetValObj = v8::Local<v8::Object>::Cast(RetVal);

    QmAssertR(TNodeJsUtil::IsClass(RetValObj, TNodeJsFltV::GetClassId()), "TJsFuncFtrExt::ExecuteFuncVec callback should return a dense vector (same type as la.newVec()).");
    // cast it to js vector and copy internal vector
    Vec = ObjectWrap::Unwrap<TNodeJsFltV>(RetValObj)->Vec;
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
v8::Persistent<v8::Function> TNodeJsFtrSpace::Constructor;

TNodeJsFtrSpace::TNodeJsFtrSpace(const TQm::PFtrSpace& _FtrSpace) :
        FtrSpace(_FtrSpace) {}

TNodeJsFtrSpace::TNodeJsFtrSpace(const TWPt<TQm::TBase> Base, TSIn& SIn) {
    FtrSpace = TQm::TFtrSpace::Load(Base, SIn);
}

void TNodeJsFtrSpace::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Local<v8::Context> context = Nan::GetCurrentContext();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsFtrSpace>);
    // child will have the same properties and methods, but a different callback: _NewCpp
    v8::Local<v8::FunctionTemplate> child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsFtrSpace>);
    child->Inherit(tpl);
    child->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer
    child->InstanceTemplate()->SetInternalFieldCount(1);

    tpl->SetClassName(TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
    NODE_SET_PROTOTYPE_METHOD(tpl, "clear", _clear);
    NODE_SET_PROTOTYPE_METHOD(tpl, "addFeatureExtractor", _addFeatureExtractor);
    NODE_SET_PROTOTYPE_METHOD(tpl, "updateRecord", _updateRecord);
    NODE_SET_PROTOTYPE_METHOD(tpl, "updateRecords", _updateRecords);
    NODE_SET_PROTOTYPE_METHOD(tpl, "updateRecordsAsync", _updateRecordsAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractSparseVector", _extractSparseVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractVector", _extractVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractFeature", _extractFeature);
    NODE_SET_PROTOTYPE_METHOD(tpl, "invertFeatureVector", _invertFeatureVector);
    NODE_SET_PROTOTYPE_METHOD(tpl, "invertFeature", _invertFeature);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureRange", _getFeatureRange);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractSparseMatrix", _extractSparseMatrix);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractMatrix", _extractMatrix);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractMatrixAsync", _extractMatrixAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureExtractor", _getFeatureExtractor);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFeatureExtractorType", _getFeatureExtractorType);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getFeature", _getFeature);
    NODE_SET_PROTOTYPE_METHOD(tpl, "filter", _filter);
    NODE_SET_PROTOTYPE_METHOD(tpl, "extractStrings", _extractStrings);

    // properties
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("dim")), _dim);
    tpl->InstanceTemplate()->SetAccessor(TNodeJsUtil::ToLocal(Nan::New("dims")), _dims);

    // This has to be last, otherwise the properties won't show up on the object in JavaScript
    // Constructor is used when creating the object from C++
    Constructor.Reset(Isolate, TNodeJsUtil::ToLocal(child->GetFunction(context)));

    // So we can call new FeatureSpace
    Nan::Set(exports, TNodeJsUtil::ToLocal(Nan::New(GetClassId().CStr())), TNodeJsUtil::ToLocal(tpl->GetFunction(context)));
}

TNodeJsFtrSpace* TNodeJsFtrSpace::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2, "FeatureSpace.constructor: 2 parameters must be given!");
    EAssertR(TNodeJsUtil::IsArgWrapObj<TNodeJsBase>(Args, 0), "FeatureSpace.constructor: first parameter must be Base!");
    const TWPt<TQm::TBase>& Base = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsBase>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->Base;

    if (Args[1]->IsString() || TNodeJsUtil::IsArgWrapObj(Args, 1, TNodeJsFIn::GetClassId())) {
        bool IsArgStr = TNodeJsUtil::IsArgStr(Args, 1);//Args[1]->IsString();

        PSIn SIn = IsArgStr ?
            TFIn::New(TNodeJsUtil::GetArgStr(Args, 1)) :
            ObjectWrap::Unwrap<TNodeJsFIn>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1])))->SIn;

        return new TNodeJsFtrSpace(Base, *SIn);
    }

    TQm::TFtrExtV FtrExtV;
    if (Args[1]->IsArray()) {
        v8::Local<v8::Array> Array = v8::Local<v8::Array>::Cast(Args[1]);
        for (uint32 ObjectN = 0; ObjectN < Array->Length(); ObjectN++) {
            if (TNodeJsUtil::ToLocal(Nan::Get(Array, ObjectN))->IsObject()) {
                v8::Local<v8::Object> Obj = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(TNodeJsUtil::ToLocal(Nan::Get(Array, ObjectN))));
                // get property "type"
                if (Nan::Has(Obj, TNodeJsUtil::ToLocal(Nan::New("type"))).ToChecked()) {
                    v8::Local<v8::Value> TypeVal = TNodeJsUtil::ToLocal(Nan::Get(Obj, TNodeJsUtil::ToLocal(Nan::New("type"))));
                    if (TypeVal->IsString()) {
                        Nan::Utf8String Utf8(TypeVal);
                        TStr Type(*Utf8);
                        if (Type == "jsfunc") {
                            QmAssertR(Nan::Has(Obj, TNodeJsUtil::ToLocal(Nan::New("fun"))).ToChecked(),
                                "analytics.newFeatureSpace object of type 'jsfunc' should have a property 'fun'");
                            QmAssertR(TNodeJsUtil::ToLocal(Nan::Get(Obj, TNodeJsUtil::ToLocal(Nan::New("fun"))))->IsFunction(),
                                "analytics.newFeatureSpace object.fun should be a function");

                            FtrExtV.Add(TNodeJsFtrSpace::NewFtrExtFromFunc(Base, Obj, Isolate));
                        } else {
                            // Json val to glib JSON
                            PJsonVal ParamVal = TNodeJsUtil::GetObjJson(TNodeJsUtil::ToLocal(Nan::Get(Array, ObjectN)));
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
            v8::Local<v8::Object> Settings = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[1]));
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
    } else {
        throw TQm::TQmExcept::New("FeatureSpace.constructor: invalid parameters!");
    }

    // create feature space
    TQm::PFtrSpace FtrSpace = TQm::TFtrSpace::New(Base, FtrExtV);
    // report on what was created
    TQm::InfoLog(FtrSpace->GetNm());
    return new TNodeJsFtrSpace(FtrSpace);
}

TNodeJsFtrSpace::TUpdateRecsTask::TUpdateRecsTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync):
        TNodeTask(Args, IsAsync) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 2, "Should have 1 argument!");

    JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    JsRecV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsRecByValV>(Args, 0);
}

v8::Local<v8::Function> TNodeJsFtrSpace::TUpdateRecsTask::GetCallback(
        const v8::FunctionCallbackInfo<v8::Value>& Args) {
    return TNodeJsUtil::GetArgFun(Args, 1);
}

void TNodeJsFtrSpace::TUpdateRecsTask::Run() {
    try {
        const TVec<TQm::TRec>& RecV = JsRecV->RecV;
        const int Len = RecV.Len();
        for (int RecN = 0; RecN < Len; RecN++) {
            const TQm::TRec Rec = RecV[RecN];
            JsFtrSpace->FtrSpace->Update(Rec);
        }
    } catch (const PExcept& _Except) {
        SetExcept(_Except);
    }
}

TNodeJsFtrSpace::TExtractMatrixTask::TExtractMatrixTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync):
            TNodeTask(Args, IsAsync) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    JsRecV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsRecByValV>(Args, 0);
    JsFtrVV = new TNodeJsFltVV;
}

v8::Local<v8::Function> TNodeJsFtrSpace::TExtractMatrixTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    return TNodeJsUtil::GetArgFun(Args, 1);
}

void TNodeJsFtrSpace::TExtractMatrixTask::Run() {
    try {
        const TVec<TQm::TRec> RecV = JsRecV->RecV;
        const TQm::PFtrSpace& FtrSpace = JsFtrSpace->FtrSpace;
        TFltVV& Result = JsFtrVV->Mat;

        const int Len = RecV.Len();
        const int Dim = FtrSpace->GetDim();

        Result.Gen(Dim, Len);
        TFltV FtrV(Dim);

        for (int RecN = 0; RecN < Len; RecN++) {
            const TQm::TRec& Rec = RecV[RecN];
            FtrSpace->GetFullV(Rec, FtrV);
            Result.SetCol(RecN, FtrV);
        }
    } catch (const PExcept& _Except) {
        SetExcept(_Except);
        delete JsFtrVV;
    }
}

v8::Local<v8::Value> TNodeJsFtrSpace::TExtractMatrixTask::WrapResult() {
    return TNodeJsUtil::NewInstance(JsFtrVV);
}


void TNodeJsFtrSpace::dim(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
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

void TNodeJsFtrSpace::dims(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Info.Holder());

        const int FtrExts = JsFtrSpace->FtrSpace->GetFtrExts();
        v8::Local<v8::Array> IntArr = v8::Array::New(Isolate, FtrExts);

        for (int FtrExtN = 0; FtrExtN < FtrExts; FtrExtN++) {
            Nan::Set(IntArr, v8::Uint32::New(Isolate, FtrExtN),
                v8::Integer::New(Isolate, JsFtrSpace->FtrSpace->GetFtrExtDim(FtrExtN))
            );
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
            ObjectWrap::Unwrap<TNodeJsFOut>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->SOut;
        EAssertR(!SOut.Empty(), "Output stream closed!");

        // save to stream
        JsFtrSpace->FtrSpace->Save(*SOut);
        // return
        Args.GetReturnValue().Set(Args[0]);
    }
    catch (const PExcept& Except) {
        throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsFtrSpace::save");
    }
}

void TNodeJsFtrSpace::clear(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 0, "Should get no argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    // clears the feature space
    JsFtrSpace->FtrSpace->Clr();

    Args.GetReturnValue().Set(Args.Holder());
}


void TNodeJsFtrSpace::addFeatureExtractor(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    TStr Type = TNodeJsUtil::GetArgStr(Args, 0, "type", "");
    if (Type == "jsfunc") {
        // All properties should be JSON objects, except for "fun", which is a function
        // example (Twitter text length feature extractor):
        // { type : 'jsfunc', source: { store: 'Tweets' }, fun : function(rec) {return rec.Text.length;}}
        // extract function!
        v8::Local<v8::Object> Settings = TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0]));
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

void TNodeJsFtrSpace::updateRecord(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRec>(Args, 0)) {
        TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
        QmAssertR(JsFtrSpace->FtrSpace->IsStartStore(JsRec->Rec.GetStore()->GetStoreId()),
            "FeatureSpace.updateRecord: record's and feature extractor's store/source must be the same!");
        JsFtrSpace->FtrSpace->Update(JsRec->Rec);
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        TQm::TRec Rec(Store, Json);
        JsFtrSpace->FtrSpace->Update(Rec);
    } else {
        throw TQm::TQmExcept::New("FeatureSpace.updateRecord: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFtrSpace::updateRecords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRecSet>(Args, 0)) {
        TNodeJsRecSet* JsRecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));

        EAssertR(JsFtrSpace->FtrSpace->IsStartStore(JsRecSet->RecSet->GetStore()->GetStoreId()),
            "FeatureSpace.updateRecords: record's and feature extractor's store/source must be the same!");

        JsFtrSpace->FtrSpace->Update(JsRecSet->RecSet);
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        EAssertR(Json->IsArr(), "FeatureSpace.updateRecords: expected record set or a JSON array");
        int Len = Json->GetArrVals();
        for (int RecN = 0; RecN < Len; RecN++) {
            TQm::TRec Rec(Store, Json->GetArrVal(RecN));
            JsFtrSpace->FtrSpace->Update(Rec);
        }
    } else {
        throw TQm::TQmExcept::New("FeatureSpace.updateRecords: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFtrSpace::extractSparseVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1 || Args.Length() == 2, "Should have 1  or 2 arguments!");

    TIntFltKdV SpV; // result sparse vector

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(SpV, JsFtrSpace->FtrSpace->GetDim())));
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1, -1);
    EAssertR(-1 <= FtrExtN && FtrExtN < JsFtrSpace->FtrSpace->GetFtrExts(), "FeatureSpace.extractSparseVector: invalid feature extractor ID!");

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRec>(Args, 0)) {
        TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
        QmAssertR(JsFtrSpace->FtrSpace->IsStartStore(JsRec->Rec.GetStore()->GetStoreId()),
            "FeatureSpace.extractSparseVector: record's and feature extractor's store/source must be the same!");
        JsFtrSpace->FtrSpace->GetSpV(JsRec->Rec, SpV, FtrExtN);
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        TQm::TRec Rec(Store, Json);
        JsFtrSpace->FtrSpace->GetSpV(Rec, SpV, FtrExtN);
    } else {
        throw TQm::TQmExcept::New("extractSparseVector: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(SpV, JsFtrSpace->FtrSpace->GetDim())));
}

void TNodeJsFtrSpace::extractVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1 || Args.Length() == 2, "Should have 1  or 2 arguments!");

    TFltV FltV;

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        Args.GetReturnValue().Set(TNodeJsFltV::New(FltV));
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1, -1);
    EAssertR(-1 <= FtrExtN && FtrExtN < JsFtrSpace->FtrSpace->GetFtrExts(), "FeatureSpace.extractVector: invalid feature extractor ID!");

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRec>(Args, 0)) {
        TNodeJsRec* JsRec = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));
        QmAssertR(JsFtrSpace->FtrSpace->IsStartStore(JsRec->Rec.GetStore()->GetStoreId()),
            "FeatureSpace.extractVector: record's and feature extractor's store/source must be the same!");
        JsFtrSpace->FtrSpace->GetFullV(JsRec->Rec, FltV, FtrExtN);
    }
    else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        TQm::TRec Rec(Store, Json);
        JsFtrSpace->FtrSpace->GetFullV(Rec, FltV, FtrExtN);
    }
    else {
        throw TQm::TQmExcept::New("extractVector: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(TNodeJsFltV::New(FltV));

}

// deprecated
void TNodeJsFtrSpace::extractFeature(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    printf("extractFeature is DEPRECATED\n");
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 2, "TNodeJsFtrSpace::extractFeature: Should have 2 arguments!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
    const double Val = TNodeJsUtil::GetArgFlt(Args, 1);

    // create feature vector, compute
    double RetVal = JsFtrSpace->FtrSpace->GetSingleFtr(FtrExtN, Val);

    Args.GetReturnValue().Set(Nan::New(RetVal));
}

void TNodeJsFtrSpace::extractSparseMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1 || Args.Length() == 2, "Should have 1  or 2 arguments!");

    TVec<TIntFltKdV> SpMat;

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(SpMat)));
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1, -1);
    EAssertR(-1 <= FtrExtN && FtrExtN < JsFtrSpace->FtrSpace->GetFtrExts(), "FeatureSpace.extractSparseMatrix: invalid feature extractor ID!");

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRecSet>(Args, 0)) {
        TNodeJsRecSet* RecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));

        EAssertR(JsFtrSpace->FtrSpace->IsStartStore(RecSet->RecSet->GetStore()->GetStoreId()),
            "FeatureSpace.extractSparseMatrix: record's and feature extractor's store/source must be the same!");

        JsFtrSpace->FtrSpace->GetSpVV(RecSet->RecSet, SpMat, FtrExtN);
    } else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        EAssertR(Json->IsArr(), "FeatureSpace.extractSparseMatrix: expected record set or a JSON array");
        int Len = Json->GetArrVals();
        SpMat.Gen(Len, 0);
        for (int RecN = 0; RecN < Len; RecN++) {
            TIntFltKdV SpVec;
            TQm::TRec Rec(Store, Json->GetArrVal(RecN));
            JsFtrSpace->FtrSpace->GetSpV(Rec, SpVec, FtrExtN);
            SpMat.Add(SpVec);
        }
    } else {
        throw TQm::TQmExcept::New("extractSparseMatrix: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(SpMat, -1)));
}

void TNodeJsFtrSpace::extractMatrix(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1 || Args.Length() == 2, "Should have 1  or 2 arguments!");

    TFltVV Mat;

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    if (JsFtrSpace->GetFtrSpace()->GetFtrExts() == 0) {
        Args.GetReturnValue().Set(TNodeJsFltVV::New(Mat));
        return;
    }

    TWPt<TQm::TStore> Store = JsFtrSpace->GetFtrSpace()->GetFtrExt(0)->GetFtrStore();

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1, -1);
    EAssertR(-1 <= FtrExtN && FtrExtN < JsFtrSpace->FtrSpace->GetFtrExts(), "FeatureSpace.extractMatrix: invalid feature extractor ID!");

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsRecSet>(Args, 0)) {
        TNodeJsRecSet* RecSet = TNodeJsUtil::UnwrapCheckWatcher<TNodeJsRecSet>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])));

        EAssertR(JsFtrSpace->FtrSpace->IsStartStore(RecSet->RecSet->GetStore()->GetStoreId()),
            "FeatureSpace.extractMatrix: record's and feature extractor's store/source must be the same!");

        JsFtrSpace->FtrSpace->GetFullVV(RecSet->RecSet, Mat, FtrExtN);
    }
    else if (TNodeJsUtil::IsArgJson(Args, 0)) {
        PJsonVal Json = TNodeJsUtil::GetArgJson(Args, 0);
        EAssertR(Json->IsArr(), "FeatureSpace.extractMatrix: expected record set or a JSON array");
        int Len = Json->GetArrVals();

        Mat.Gen(JsFtrSpace->FtrSpace->GetDim(), Len);
        for (int RecN = 0; RecN < Len; RecN++) {
            TFltV Vec;
            TQm::TRec Rec(Store, Json->GetArrVal(RecN));
            JsFtrSpace->FtrSpace->GetFullV(Rec, Vec, FtrExtN);
            Mat.SetCol(RecN, Vec);
        }
    }
    else {
        throw TQm::TQmExcept::New("FeatureSpace.extractMatrix: unsupported type of argument 0");
    }

    Args.GetReturnValue().Set(TNodeJsFltVV::New(Mat));

}

void TNodeJsFtrSpace::getFeatureExtractor(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
    const TStr FtrExtNm = JsFtrSpace->FtrSpace->GetFtrExt(FtrExtN)->GetNm();

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(FtrExtNm.CStr())));
}

void TNodeJsFtrSpace::getFeatureExtractorType(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
    const TStr FtrExtNm = JsFtrSpace->FtrSpace->GetFtrExt(FtrExtN)->GetFtrType();

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(FtrExtNm.CStr())));
}

void TNodeJsFtrSpace::getFeature(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const int FtrN = TNodeJsUtil::GetArgInt32(Args, 0);
    const TStr FtrNm = JsFtrSpace->FtrSpace->GetFtr(FtrN);

    Args.GetReturnValue().Set(TNodeJsUtil::ToLocal(Nan::New(FtrNm.CStr())));
}


void TNodeJsFtrSpace::invertFeatureVector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() == 1, "Should have 1 argument!");
    QmAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId()) || Args[0]->IsArray(), "The argument should be a float array!");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    TFltV InvertV;

    if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId())) {
        const TFltV& FtrV = ObjectWrap::Unwrap<TNodeJsFltV>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->Vec;
        const PJsonVal Result = JsFtrSpace->FtrSpace->InvertFullV(FtrV);
        Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Result));
    }
    else {
        EAssert(Args[0]->IsArray());
        v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Args[0]);
        TFltV FtrV(Arr->Length(), 0);

        for (uint i = 0; i < Arr->Length(); i++) {
            v8::Local<v8::Value> Val = TNodeJsUtil::ToLocal(Nan::Get(Arr, i));
            FtrV.Add(Nan::To<double>(Val).FromJust());
        }

        const PJsonVal Result = JsFtrSpace->FtrSpace->InvertFullV(FtrV);
        Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Result));
    }
}

void TNodeJsFtrSpace::invertFeature(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);

    PJsonVal FtrVal;
    if (TNodeJsUtil::IsArgJson(Args, 1)) {
        FtrVal = TNodeJsUtil::GetArgJson(Args, 1);
    } else if (TNodeJsUtil::IsArgFlt(Args, 1)) {
        FtrVal = TJsonVal::NewNum(TNodeJsUtil::GetArgFlt(Args, 1));
    } else {
        throw TExcept::New("TNodeJsFtrSpace::invertFeature: Unsupported argument type!");
    }

    const PJsonVal Result = JsFtrSpace->FtrSpace->InvertFtr(FtrExtN, FtrVal);

    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Result));
}

void TNodeJsFtrSpace::getFeatureRange(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 0);
    const PJsonVal Result = JsFtrSpace->FtrSpace->GetFtrRange(FtrExtN);

    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Result));
}

void TNodeJsFtrSpace::filter(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    QmAssertR(Args.Length() > 0, "FeatureSpace.filter: Expecting vector as parameter");
    QmAssertR(Args[0]->IsObject(), "FeatureSpace.filter: Expecting vector as parameter");

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());
    QmAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0,
        TNodeJsFltV::GetClassId()) || TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsSpVec::GetClassId()),
        "FeatureSpace.filter: expecting a dense or a sparse vector as the first argument!");
    const int FtrExtN = TNodeJsUtil::GetArgInt32(Args, 1);
    const bool KeepOffsetP = TNodeJsUtil::GetArgBool(Args, 2, true);
    // get dimension border
    const int MnFtrN = JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN);
    const int MxFtrN = JsFtrSpace->FtrSpace->GetMxFtrN(FtrExtN);

    if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsSpVec::GetClassId())) {
        const TIntFltKdV& SpV = ObjectWrap::Unwrap<TNodeJsSpVec>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->Vec;

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
        Args.GetReturnValue().Set(TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(NewSpV, VecDim)));
    }
    else if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId())) {
        const TFltV& Vec = ObjectWrap::Unwrap<TNodeJsFltV>(TNodeJsUtil::ToLocal(Nan::To<v8::Object>(Args[0])))->Vec;

        // get dimensionality of the feature extractor
        int DimN = JsFtrSpace->FtrSpace->GetFtrExtDim(FtrExtN);
        // get offset of the feature extractor
        int VecOffset = JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN);
        // make sure input vector has enough elements
        QmAssertR(Vec.Len() >= VecOffset + DimN, "FeatureSpace.filter: input vector dimensionality to small");
        // prepare place for new vector
        TFltV NewVec(KeepOffsetP ? Vec.Len() : DimN);
        // load part belonging to the give feature extractor
        int NewVecOffset = KeepOffsetP ? JsFtrSpace->FtrSpace->GetMnFtrN(FtrExtN) : 0;
        for (int FtrN = 0; FtrN < DimN; FtrN++) {
            NewVec[FtrN + NewVecOffset] = Vec[FtrN + VecOffset];
        }
        Args.GetReturnValue().Set(TNodeJsFltV::New(NewVec));
    }
}

void TNodeJsFtrSpace::extractStrings(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFtrSpace* JsFtrSpace = ObjectWrap::Unwrap<TNodeJsFtrSpace>(Args.Holder());

    const PJsonVal RecVal = TNodeJsUtil::GetArgJson(Args, 0);
    const int DimN = TNodeJsUtil::GetArgInt32(Args, 1, 0);

    // get strings
    TStrV StrV; JsFtrSpace->FtrSpace->ExtractStrV(DimN, RecVal, StrV);

    v8::Local<v8::Array> StrArr = v8::Array::New(Isolate, StrV.Len());

    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        Nan::Set(StrArr, v8::Uint32::New(Isolate, StrN), TNodeJsUtil::ToLocal(Nan::New(StrV.GetVal(StrN).CStr())));
    }

    Args.GetReturnValue().Set(StrArr);
}

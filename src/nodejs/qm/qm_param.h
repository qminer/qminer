/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <qminer.h>

class TQmParam {
public:
    // javascript parameters
    class TJsParam {
    public:
        // script namespace
        TStr Nm;
        // script filename
        TStr FNm;
        // script initialization parameters
        PJsonVal InitVal;
        // libraries include path
        TStrV IncludeFPathV;
        // folders with access permissions
        TStrV AccessFPathV;     
    private:
        // get qminer global library include folder
        void AddQMinerLibFPath() {
            IncludeFPathV.Add(TQm::TEnv::QMinerFPath + "./lib/");
        }
        
        // get local include folder if exists
        void AddLocalLibFPath() {
            TStr LibFPath = FNm.GetFPath() + "lib/";
            if (TDir::Exists(LibFPath)) {
                IncludeFPathV.Add(LibFPath);
            }
        }   
        
        // get sandbox folder
        void AddSandboxAccessFPath(const TStr& RootFNm) {
            // handle default access directories only when sandbox exists
            TStr SandboxFPath = TStr::GetNrAbsFPath("sandbox", RootFNm);
            if (TDir::Exists(SandboxFPath)) {
                // prepare script specific space
                TStr AccessFPath = TStr::GetNrAbsFPath(Nm, SandboxFPath);
                // if doesn't exist yet, create it
                if (!TDir::Exists(AccessFPath)) { TDir::GenDir(AccessFPath); }
                // done
                AccessFPathV.Add(AccessFPath);              
            }
        }
        
    public:
        // required by vectors
        TJsParam() { }
        
        // parse from json configuration file
        TJsParam(const TStr& RootFNm, const PJsonVal& JsVal) { 
            EAssertR(JsVal->IsObj(), "Unsupported type: " + TJsonVal::GetStrFromVal(JsVal));
            // we must have at least the script name
            EAssert(JsVal->IsObjKey("file"));
            // get script name              
            FNm = JsVal->GetObjStr("file");
            // get namespace (get from script name if not available)
            Nm = JsVal->IsObjKey("name") ? JsVal->GetObjStr("name") : FNm.GetFMid();
            // get initialization parameters (if available)
            InitVal = JsVal->IsObjKey("init") ? JsVal->GetObjKey("init") : TJsonVal::NewObj();
            // get library include folders
            if (JsVal->IsObjKey("include")) { 
                PJsonVal IncludesVal = JsVal->GetObjKey("include");
                EAssertR(IncludesVal->IsArr(), "Expected array of strings, not: " + TJsonVal::GetStrFromVal(IncludesVal));
                for (int IncludeN = 0; IncludeN < IncludesVal->GetArrVals(); IncludeN++) {
                    PJsonVal IncludeVal = IncludesVal->GetArrVal(IncludeN);
                    EAssertR(IncludeVal->IsStr(), "Expected string, not: " + TJsonVal::GetStrFromVal(IncludeVal));                      
                    IncludeFPathV.Add(IncludeVal->GetStr());
                }
            }
            // handle default includes
            AddLocalLibFPath();
            AddQMinerLibFPath();
            // get folders with access permissions
            if (JsVal->IsObjKey("dirs")) { 
                PJsonVal DirsVal = JsVal->GetObjKey("dirs");
                EAssertR(DirsVal->IsArr(), "Expected array of strings, not: " + TJsonVal::GetStrFromVal(DirsVal));
                for (int DirN = 0; DirN < DirsVal->GetArrVals(); DirN++) {
                    PJsonVal DirVal = DirsVal->GetArrVal(DirN);
                    EAssertR(DirVal->IsStr(), "Expected string, not: " + TJsonVal::GetStrFromVal(DirVal));
                    AccessFPathV.Add(DirVal->GetStr());
                }
            }
            // add sandbox access
            AddSandboxAccessFPath(RootFNm);
        }
        
        // parse from script filename, assuming default structure
        TJsParam(const TStr& RootFNm, const TStr& _FNm) {
            // remember script name
            FNm = _FNm;
            // derive namespace from filename
            Nm = FNm.GetFMid();
            // no initialization parameters
            InitVal = TJsonVal::NewObj();
            // handle default includes
            AddLocalLibFPath();
            AddQMinerLibFPath();
            // add sandbox access
            AddSandboxAccessFPath(RootFNm);
        }
        
    };
    
public:
    // project root
    TStr RootFPath;
    // lock file
    TStr LockFNm;
    // database path
    TStr DbFPath;
    // server port
    int PortN;
    // index cache size
    uint64 IndexCacheSize;
    // default store cache size
    uint64 DefStoreCacheSize;
    // store specific cache sizes
    TStrUInt64H StoreNmCacheSizeH;
    // javascript parameters
    TVec<TJsParam> JsParamV;
    // file serving folders
    TStrPrV WwwRootV;

private:
    void AddWwwRoot(const PJsonVal& WwwVal) {
        WwwRootV.Add(TStrPr(WwwVal->GetObjStr("name"), 
            TStr::GetNrAbsFPath(WwwVal->GetObjStr("path"), RootFPath)));
    }
    
public:
    TQmParam(const TStr& FNm) {
        EAssertR(TFile::Exists(FNm), "Missing configuration file " + FNm);
        // load configuration file
        PJsonVal ConfigVal = TJsonVal::GetValFromSIn(TFIn::New(FNm));
        EAssertR(ConfigVal->IsObj(), "Invalid setting file - not valid JSON");
        // parse out common stuff
        RootFPath = TStr::GetNrFPath(ConfigVal->GetObjStr("directory", TDir::GetCurDir()));
        LockFNm = RootFPath + "./lock";
        DbFPath = ConfigVal->GetObjStr("database", "./db/");
        PortN = TFlt::Round(ConfigVal->GetObjNum("port"));
        // parse out unicode definition file
        TStr UnicodeFNm = ConfigVal->GetObjStr("unicode", TQm::TEnv::QMinerFPath + "./UnicodeDef.Bin");
        if (!TUnicodeDef::IsDef()) { TUnicodeDef::Load(UnicodeFNm); }

        // Load Stopword Files
        TStr StopWordsPath = ConfigVal->GetObjStr("stopwords", TQm::TEnv::QMinerFPath + "resources/stopwords/");
        TSwSet::LoadSwDir(StopWordsPath);

        // parse cache
        if (ConfigVal->IsObjKey("cache")) { 
            PJsonVal CacheVal = ConfigVal->GetObjKey("cache");
            // parse out index and default store cache sizes
            IndexCacheSize = int64(CacheVal->GetObjNum("index", 1024)) * int64(TInt::Mega);
            DefStoreCacheSize = int64(CacheVal->GetObjNum("store", 1024)) * int64(TInt::Mega);
            // prase out store specific sizes, when available
            if (CacheVal->IsObjKey("stores")) {
                PJsonVal StoreCacheVals = CacheVal->GetObjKey("stores");
                for (int StoreN = 0; StoreN < StoreCacheVals->GetArrVals(); StoreN++) {
                    PJsonVal StoreCacheVal = StoreCacheVals->GetArrVal(StoreN);                 
                    TStr StoreName = StoreCacheVal->GetObjStr("name");
                    uint64 StoreCacheSize = int64(StoreCacheVal->GetObjNum("size")) * int64(TInt::Mega);
                    StoreNmCacheSizeH.AddDat(StoreName, StoreCacheSize);
                }
            }
        } else {
            // default sizes are set to 1GB for index and stores            
            IndexCacheSize = int64(1024) * int64(TInt::Mega);
            DefStoreCacheSize = int64(1024) * int64(TInt::Mega);
        }

        // load scripts
        if (ConfigVal->IsObjKey("script")) {
            // we have configuration file, read it
            PJsonVal JsVals = ConfigVal->GetObjKey("script");
            if (JsVals->IsArr()) {
                for (int JsValN = 0; JsValN < JsVals->GetArrVals(); JsValN++) {
                    JsParamV.Add(TJsParam(RootFPath, JsVals->GetArrVal(JsValN)));
                }
            } else {
                JsParamV.Add(TJsParam(RootFPath, JsVals));
            }
        } else {
            // no settings for scripts, assume default setting
            TStr SrcFPath = TStr::GetNrAbsFPath("src", RootFPath);
            TFFile File(SrcFPath, ".js", false); TStr SrcFNm;
            while (File.Next(SrcFNm)) {
                JsParamV.Add(TJsParam(RootFPath, SrcFNm));
            }
        }

        // load serving folders
        //TODO: Add to qm config ability to edit this
        if (ConfigVal->IsObjKey("wwwroot")) {
            PJsonVal WwwVals = ConfigVal->GetObjKey("wwwroot");
            if (WwwVals->IsArr()) {
                for (int WwwValN = 0; WwwValN < WwwVals->GetArrVals(); WwwValN++) {
                    AddWwwRoot(WwwVals->GetArrVal(WwwValN));
                }
            } else {
                AddWwwRoot(WwwVals);
            }           
        }
        // check for folder with admin GUI
        TStr GuiFPath = TStr::GetNrAbsFPath("gui", TQm::TEnv::QMinerFPath);
        if (TDir::Exists(GuiFPath)) {
            WwwRootV.Add(TStrPr("admin", GuiFPath));
        }
        // check for any default wwwroot
        TStr DefaultWwwRootFPath = TStr::GetNrAbsFPath("www", RootFPath);
        if (TDir::Exists(DefaultWwwRootFPath)) {
            WwwRootV.Add(TStrPr("www", DefaultWwwRootFPath));
        }
    }
};
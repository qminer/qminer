/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Quintelligence d.o.o.
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
 * Contact: 
 *   Blaz Fortuna <blaz@blazfortuna.com>
 *
 */

#include <qminer.h>
#include <qminer_srv.h>
#include <qminer_gs.h>

#ifdef USE_JS
#include <qminer_js.h>
#endif

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
			// parse out store specific sizes, when available
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

#ifdef V8_DEBUG
// for debugging JavaScript copied from linneprocessor.cc
v8::Persistent<v8::Context> DebugContext;

void DispatchDebugMessages() {
  // We are in some random thread. We should already have v8::Locker acquired
  // (we requested this when registered this callback). We was called
  // because new debug messages arrived; they may have already been processed,
  // but we shouldn't worry about this.
  //
  // All we have to do is to set context and call ProcessDebugMessages.
  //
  // We should decide which V8 context to use here. This is important for
  // "evaluate" command, because it must be executed some context.
  // In our sample we have only one context, so there is nothing really to
  // think about.
  v8::Context::Scope scope(DebugContext);

  v8::Debug::ProcessDebugMessages();
}
#endif

#ifdef USE_JS
// initialize javascript
void InitJs(const TQmParam& Param, const TQm::PBase& Base, const TStr& OnlyScriptNm, TVec<TQm::PScript>& ScriptV) {
    if (!OnlyScriptNm.Empty()) {
        TQm::InfoLog("Set limit to script " + OnlyScriptNm);
    }

#ifdef V8_DEBUG
    // for debugging JavaScript
    printf("=================================================\n");
    printf("Initializing V8 debugger...\n");
    const int DebugPort = 9222;
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::Locker Locker(Isolate);

	v8::HandleScope HandleScope;

	// Create a template for the global object.
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	v8::Handle<v8::Context> context = v8::Context::New(NULL, global);

	DebugContext = v8::Persistent<v8::Context>::New(Isolate, context);
	
	v8::Debug::SetDebugMessageDispatchHandler(DispatchDebugMessages, true);
	v8::Debug::EnableAgent("QMiner", DebugPort, false);

	printf("Debugger listening on port: %d\n", DebugPort);
	printf("=================================================\n");
#endif

	for (int JsN = 0; JsN < Param.JsParamV.Len(); JsN++) {
		const TQmParam::TJsParam& JsParam = Param.JsParamV[JsN];
        // skip if required
        if (!OnlyScriptNm.Empty() && JsParam.Nm != OnlyScriptNm) {
            TQm::InfoLog("Skipping script " + JsParam.Nm); continue;        
        }
        // otherwise continue with load
		TQm::InfoLog("Loading script " + JsParam.FNm.GetFMid() + "...");
        try {
            // initialize javascript engine		
            TVec<TQm::TJsFPath> JsFPathV;
            TQm::TJsFPath::GetFPathV(JsParam.AccessFPathV, JsFPathV);
            TQm::PScript Script = TQm::TScript::New(Base, JsParam.Nm, 
                JsParam.FNm, JsParam.IncludeFPathV, JsFPathV);
            // remember the context 
            ScriptV.Add(Script);
            // done
            TQm::InfoLog("  done");
        } catch (PExcept& Except) {
            TQm::ErrorLog("Error loading script " + JsParam.FNm.GetFMid() + ":");
            TQm::ErrorLog("  " + Except->GetMsgStr());
        }
	}
}

#endif

void ExecUrl(const TStr& UrlStr, const TStr& OkMsgStr, const TStr& ErrMsgStr) {
	// execute request
	bool Ok; TStr MsgStr; PWebPg WebPg;
	TWebFetchBlocking::GetWebPg(UrlStr, Ok, MsgStr, WebPg);
	// report on result
	if (Ok) {
		TQm::InfoLog(OkMsgStr);
	} else {
		TQm::InfoLog(ErrMsgStr + MsgStr);
	}	
}

int main(int argc, char* argv[]) {
#ifndef NDEBUG
    // report we are running with all Asserts turned on
    printf("*** Running in debug mode ***\n");
#endif    
#ifdef EXTENDEDOPENFILES
    // enable maximum number of simultaneously opened files
	_setmaxstdio(2048);
#endif

	try {
		// initialize QMiner environment
		TQm::TEnv::Init();
		// create app environment
		Env = TEnv(argc, argv, TNotify::StdNotify);
		Env.SetNoLine(); // making output prettier
		// command line parameters
		Env.PrepArgs("QMiner " + TQm::TEnv::GetVersion(), 0);
		// read the action
		const bool ConfigP = Env.IsArgStr("config");
		const bool CreateP = Env.IsArgStr("create");
		const bool StartP = Env.IsArgStr("start");
		const bool StopP = Env.IsArgStr("stop");
		//const bool ReloadP = Env.IsArgStr("reload");
		const bool ImportP = Env.IsArgStr("import");
		const bool TestP = Env.IsArgStr("test");
		const bool DebugP = Env.IsArgStr("debug");
		// stop if no action given
		const bool ActionP = (ConfigP || CreateP || StartP || StopP /*|| ReloadP*/ || DebugP || ImportP || TestP);
		// provide basic instruction when no action given
		if (!ActionP) {
			printf("\n");
			printf("Usage: qm ACTION [OPTION]...\n");
			printf("\n");
			printf("Actions: config, create, start, stop, reload, debug, import, test\n");
		} else {
			Env.SetSilent();
		}
		// configuration file
		const TStr ConfFNm = Env.GetIfArgPrefixStr("-conf=", "qm.conf", "Configration file");
		// read config-specific parameters
		if (!Env.IsSilent()) { printf("\nConfiguration parameters:\n"); }
		const int PortN = Env.GetIfArgPrefixInt("-port=", 8080, "Port number");
		const int CacheSizeMB = Env.GetIfArgPrefixInt("-cache=", 1024, "Cache size");
		const bool OverwriteP = Env.IsArgStr("-overwrite", "Overwrite existing configuration file");
		// read create-specific parameters
		if (!Env.IsSilent()) { printf("\nCreate parameters:\n"); }
		const TStr SchemaFNm = Env.GetIfArgPrefixStr("-def=", "", "Store definition file");
		// read start-specific parameters
		if (!Env.IsSilent()) { printf("\nStart parameters:\n"); }
		const bool RdOnlyP = Env.IsArgStr("-rdonly", "Open database in Read-only mode");
		const bool NoLoopP = Env.IsArgStr("-noserver", "Do not start server after script execution");
		TStr OnlyScriptNm = Env.GetIfArgPrefixStr("-script=", "", "Only run this script");
		// read stop-specific parameters
		if (!Env.IsSilent()) { printf("\nStop parameters:\n"); }
		const int ReturnCode = Env.GetIfArgPrefixInt("-return=", 0, "Return code");
		// read import-specific parameters
		if (!Env.IsSilent()) { printf("\nImport parameters:\n"); }
		const TStr ImportFNm = Env.GetIfArgPrefixStr("-file=", "", "JSON file");
		const TStr ImportStoreNm = Env.GetIfArgPrefixStr("-store=", "", "Store to receive the data");
		// read reload-specific parameters
		//if (!Env.IsSilent()) { printf("\nReload parameters:\n"); }
		//TStrV ReloadNmV = Env.GetIfArgPrefixStrV("-name=", "Script name");
		// read debug request parameters
		if (!Env.IsSilent()) { printf("\nDebug parameters:\n"); }
		TStr DebugFNm = Env.GetIfArgPrefixStr("-prefix=", "Debug-", "Prefix of debug output files");
		TStrV DebugTaskV = Env.GetIfArgPrefixStrV("-task=", "Debug tasks [indexvoc, index, stores, <store>, <store>_ALL]");
		const int JsStatRate = Env.GetIfArgPrefixInt("-jsmemstat=", 0, "Frequency of JavaScript memory statistics");        
		// read logging specific parameters
		if (!Env.IsSilent()) { printf("\nLogging parameters:\n"); }
		TStr LogFPath = Env.GetIfArgPrefixStr("-log=", "std", "Log Folder (std for standard output, null for silent)");
		const bool Verbose = Env.IsArgStr("-v", "Verbose output (used for debugging)");
		if (!Env.IsSilent()) { printf("\nPre-run file:\n"); }		
		const TStr PreRunFNm = Env.GetIfArgPrefixStr("-prerun=", "", "Pre-run file name");
		if (!Env.IsSilent()) { printf("\n"); }

        // execute pre-run command when provided
		if (!PreRunFNm.Empty()) { 
            const int ReturnCd = system(PreRunFNm.CStr());
            if (ReturnCd != 0) { 
                TQm::ErrorLog(TStr::Fmt("Error running prerun script: %d", ReturnCd)); 
            }
        }

		// stop if no action specified
		if (!ActionP) { return 0; }

		// initialize notifier
		TQm::TEnv::InitLogger(Verbose ? 2 : 1, LogFPath, true);
		printf("\n");

		// Create directory structure with basic qm.conf file
		if (ConfigP) {
			// check so we don't overwrite any existing configuration file
			if (TFile::Exists(ConfFNm) && ! OverwriteP) {
				TQm::InfoLog("Configuration file already exists (" + ConfFNm + ")");
				TQm::InfoLog("Use -overwrite to force overwrite");
				return 2;
			}
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
			if (!TFile::Exists("sandbox")) { TDir::GenDir("sandbox"); }			
		}

		
		// parse configuration file
		TQmParam Param(ConfFNm);
		// prepare lock
		TFileLock Lock(Param.LockFNm);

		// Initialize empty database
		if (CreateP) {
			// do not mess with folders with existing running qminer instance
			Lock.Lock();
            {
                // parse schema (if no given, create an empty array)
                PJsonVal SchemaVal = SchemaFNm.Empty() ? TJsonVal::NewArr() :
                    TJsonVal::GetValFromStr(TStr::LoadTxt(SchemaFNm));
                // initialize base
                TQm::PBase Base = TQm::TStorage::NewBase(Param.DbFPath, SchemaVal, 16, 16);
                // save base
                TQm::TStorage::SaveBase(Base);
            }
			// remove lock
			Lock.Unlock();
		}

		// Start QMiner engine
		if (StartP) {
			// do not mess with folders with running qminer instance
			Lock.Lock();
			// load database and start the server
			{
				// resolve access type
				TFAccess FAccess = RdOnlyP ? faRdOnly : faUpdate;
				// load base
				TQm::PBase Base = TQm::TStorage::LoadBase(Param.DbFPath, FAccess, 
                    Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);

#ifdef USE_JS
				// initialize javascript contexts
                TQm::TJsUtil::SetObjStatRate(JsStatRate);
				TVec<TQm::PScript> ScriptV; InitJs(Param, Base, OnlyScriptNm, ScriptV);
#endif
				// start server
				if (!NoLoopP) {
                    // prepare server functions 
                    TSAppSrvFunV SrvFunV;
                    // used to stop the server
                    SrvFunV.Add(TSASFunExit::New());
                    // admin webservices
                    TQm::TSrvFun::RegDefFun(Base, SrvFunV);
                    // initialize static content serving thingies
                    for (int WwwRootN = 0; WwwRootN < Param.WwwRootV.Len(); WwwRootN++) {
                        const TStrPr& WwwRoot = Param.WwwRootV[WwwRootN];
                        const TStr& UrlPath = WwwRoot.Val1, FPath = WwwRoot.Val2;
                        TQm::TEnv::Logger->OnStatusFmt("Registering '%s' at '/%s/'", FPath.CStr(), UrlPath.CStr());
                        SrvFunV.Add(TSASFunFPath::New(UrlPath, FPath));
                    }

#ifdef USE_JS
                    // register admin services
                    SrvFunV.Add(TQm::TJsAdminSrvFun::New(ScriptV, "qm_status"));
					// register javascript contexts
					for (int ScriptN = 0; ScriptN < ScriptV.Len(); ScriptN++) {
						// register server function
						ScriptV[ScriptN]->RegSrvFun(SrvFunV);
					}
#endif
					// start server
					PWebSrv WebSrv = TSAppSrv::New(Param.PortN, SrvFunV, TQm::TEnv::Logger, true, true);
					// report we started
					TQm::TEnv::Logger->OnStatusFmt("Server started on port %d", Param.PortN);
					// wait for the end
					TLoop::Run();
				}
                // save base
                TQm::TStorage::SaveBase(Base);

			}
			// remove lock
			Lock.Unlock();
		}

		// Run QMiner engine to import file
		if (ImportP) {
			Lock.Lock();
			{
				TFAccess FAccess = RdOnlyP ? faRdOnly : faUpdate;
				TQm::PBase Base = TQm::TStorage::LoadBase(Param.DbFPath, FAccess,
					Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);
				{
					TWPt<TQm::TStore> store = Base->GetStoreByStoreNm(ImportStoreNm);
					{
						PSIn fin = TFIn::New(ImportFNm);
						TStr s;
						while (fin->GetNextLn(s)) {
							PJsonVal json = TJsonVal::GetValFromStr(s);
							store->AddRec(json);
						}
					}
				}
				// save base
				TQm::TStorage::SaveBase(Base);
			}
			// remove lock
			Lock.Unlock();
		}

		// Run QMiner engine in test mode - execute custom code
		if (TestP) {
			Lock.Lock();
			{
				TFAccess FAccess = RdOnlyP ? faRdOnly : faUpdate;
				TQm::PBase Base = TQm::TStorage::LoadBase(Param.DbFPath, FAccess,
					Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);

				//{
				//	// this demo assumes movies database was initialized. it can contain existing data.
				//	TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("Movies");
				//	TRnd rnd(1212);
				//	for (int i = 0; i < 1000 * 1000; i++) {
				//		if (i % 1000 == 0) printf("== %d\n", i);
				//		// perform insert of new record
				//		auto json = TJsonVal::NewObj();
				//		json->AddToObj("Title", TStr::Fmt("Title %d", i));
				//		json->AddToObj("Plot", TStr::Fmt("Plot %d", i));
				//		json->AddToObj("Year", 1980 + rnd.GetUniDevInt(30));
				//		json->AddToObj("Rating", 1 + rnd.GetUniDevInt(9));

				//		auto json_a = TJsonVal::NewArr();
				//		json_a->AddToArr(TStr::Fmt("Genre %d", rnd.GetUniDevInt(10)));
				//		json->AddToObj("Genres", json_a);

				//		auto json_p = TJsonVal::NewObj();
				//		json_p->AddToObj("Name", TStr::Fmt("Director %d", (13 * i) % 100000));
				//		json_p->AddToObj("Gender", "Male");
				//		json->AddToObj("Director", json_p);

				//		json_a = TJsonVal::NewArr();
				//		int actors = rnd.GetUniDevInt(8) + 5;
				//		for (int k = 0; k < actors; k++) {
				//			json_p = TJsonVal::NewObj();
				//			json_p->AddToObj("Name", TStr::Fmt("Actor %d", rnd.GetUniDevInt(100000)));
				//			json_p->AddToObj("Gender", "Male");
				//			json_a->AddToArr(json_p);
				//		}
				//		json->AddToObj("Actor", json_a);

				//		store->AddRec(json);
				//	}
				//}

				{
					// this demo assumes movies database was initialized and populated with initial data
					TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("Movies");
					TRnd rnd(1212);
					TQQueue<uint64> added_ids;
					for (int i = 0; i < 200 * 1000; i++) {
						int r = rnd.GetUniDevInt(100);
						if (i % 100 == 0) printf("==================== %d\n", i);
						if (r < 10) {
							// perform insert of a new record
							int z = 2000000 + i * 7;
							auto json = TJsonVal::NewObj();
							json->AddToObj("Title", TStr::Fmt("Title word%d", z));

							TStr plot = "Plot";
							int plot_len = 2 + round(rnd.GetPoissonDev(10));
							for (int j = 0; j < plot_len; j++) {
								int word1 = round(rnd.GetPoissonDev(100));
								plot += TStr::Fmt(" word%d", word1);
							}

							json->AddToObj("Plot", plot);
							json->AddToObj("Year", 1980 + rnd.GetUniDevInt(30));
							json->AddToObj("Rating", 1 + rnd.GetUniDevInt(9));

							auto json_a = TJsonVal::NewArr();
							json_a->AddToArr(TStr::Fmt("Genre %d", rnd.GetUniDevInt(10)));
							json->AddToObj("Genres", json_a);

							auto json_p = TJsonVal::NewObj();
							json_p->AddToObj("Name", TStr::Fmt("Director %d", z % 100000));
							json_p->AddToObj("Gender", "Male");
							json->AddToObj("Director", json_p);

							json_a = TJsonVal::NewArr();
							int actors = rnd.GetUniDevInt(8) + 5;
							for (int k = 0; k < actors; k++) {
								json_p = TJsonVal::NewObj();
								json_p->AddToObj("Name", TStr::Fmt("Actor %d", rnd.GetUniDevInt(10000)));
								json_p->AddToObj("Gender", "Male");
								json_a->AddToArr(json_p);
							}
							json->AddToObj("Actor", json_a);

							auto id = store->AddRec(json);
							added_ids.Push(id);

						} else if (r < -11) {
							// perform delete of the front 5 records
							if (store->GetRecs() > 5) {
								store->DeleteFirstNRecs(5);
							}
							//printf("     deleted 5 records\n");						
						} else {
							// retrieve random movie and its actors
							if (store->GetRecs() > 0) {
								auto id = rnd.GetUniDevInt64(store->LastRecId());
								while (!store->IsRecId(id)) {
									id = rnd.GetUniDevInt64(store->LastRecId());
								}
								auto rec = store->GetRec(id);

								auto actors = rec.DoJoin(Base, "Actor");
								int actors_cnt = actors->GetRecs();
							}
						}
					}
				}
				// save base
				TQm::TStorage::SaveBase(Base);
			}
			// remove lock
			Lock.Unlock();
		}

		// Stop QMiner engine
		if (StopP) {
			ExecUrl(TStr::Fmt("http://127.0.0.1:%d/exit?return=%d", Param.PortN, ReturnCode),
                "Server stop procedure initiated", "Error stopping server: ");
		}

		// Reload QMiner script
		//if (ReloadP) {
		//	for (int ReloadNmN = 0; ReloadNmN < ReloadNmV.Len(); ReloadNmN++) {
		//		TStr ScriptNm = ReloadNmV[ReloadNmN];
		//		ExecUrl(TStr::Fmt("http://127.0.0.1:%d/%s/admin/reload", Param.PortN, ScriptNm.CStr()), 
		//			"Initializing reload of script '" + ScriptNm + "'", 
		//			"Error reloading script '" + ScriptNm + "': ");
		//	}
		//}

		// Debug dumps of database and index
		if (DebugP) {
			// do not mess with folders with existing running qminer instance
			Lock.Lock();
            {
                // load base
                TQm::PBase Base = TQm::TStorage::LoadBase(Param.DbFPath, faRdOnly, 
                    Param.IndexCacheSize, Param.DefStoreCacheSize, Param.StoreNmCacheSizeH);
                // go over task lists and prepare outputs
                for (int TaskN = 0; TaskN < DebugTaskV.Len(); TaskN++) {
                    TStr Task = DebugTaskV[TaskN];
                    if (Task == "index") {
                        Base->PrintIndex(DebugFNm + "index.txt", true);
                    } else if (Task == "indexvoc") {
                        Base->PrintIndexVoc(DebugFNm + "indexvoc.txt");
                    } else if (Task == "stores") {
                        Base->PrintStores(DebugFNm + "stores.txt");
                    } else if (Task.IsSuffix("_ALL")) {
                        TStr StoreNm = Task.LeftOfLast('_');
                        Base->GetStoreByStoreNm(StoreNm)->PrintAll(Base, DebugFNm + Task + ".txt");
                    } else if (Base->IsStoreNm(Task)) {
                        Base->GetStoreByStoreNm(Task)->PrintTypes(Base, DebugFNm + Task + ".txt");
                    } else {
                        TQm::InfoLog("Unknown debug task '" + Task + "'");
                    }
                }
            }
			// remove lock
			Lock.Unlock();
		}		
	} catch (const PExcept& Except) {
		// GLib exception
		TQm::ErrorLog("Error: " + Except->GetMsgStr());
		return 2;
	} catch (...) {
		// other exceptions
		TQm::ErrorLog("Unknown error");
		return 1;
	}
	return TQm::TEnv::ReturnCode.Val;
}
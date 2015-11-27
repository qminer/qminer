/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

ClassHdTPV(TSAppSrvFun, PSAppSrvFun, TSAppSrvFunV);

//////////////////////////////////////
// Simple-App-Server-Request-Environment
ClassTP(TSAppSrvRqEnv, PSAppSrvRqEnv)//{
private:
	TWebSrv* WebSrv;
	TUInt64 SockId;
	PHttpRq HttpRq;
	const THash<TStr, PSAppSrvFun>& FunNmToFunH;

public:
	TSAppSrvRqEnv(TWebSrv* _WebSrv, uint64 _SockId, const PHttpRq& _HttpRq, 
		const THash<TStr, PSAppSrvFun>& _FunNmToFunH): WebSrv(_WebSrv), SockId(_SockId), 
			HttpRq(_HttpRq), FunNmToFunH(_FunNmToFunH) { }
	static PSAppSrvRqEnv New(TWebSrv* WebSrv, uint64 SockId, const PHttpRq& HttpRq,
		const THash<TStr, PSAppSrvFun>& FunNmToFunH) { 
			return new TSAppSrvRqEnv(WebSrv, SockId, HttpRq, FunNmToFunH); }

	TWebSrv* GetWebSrv() const { return WebSrv; }
	uint64 GetSockId() const { return SockId; }
	const PHttpRq& GetHttpRq() const { return HttpRq; }
	bool IsFunNm(const TStr& FunNm) const { return FunNmToFunH.IsKey(FunNm); }
	void ExecFun(const TStr& FunNm, const TStrKdV& FldNmValPrV);
};

//////////////////////////////////////
// Simple-App-Server-Function
typedef enum { saotUndef, saotXml, saotJSon, saotCustom } TSAppOutType;
ClassTPV(TSAppSrvFun, PSAppSrvFun, TSAppSrvFunV)//{
public:
	static bool IsFldNm(const TStrKdV& FldNmValPrV, const TStr& FldNm);
	static TStr GetFldVal(const TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& DefFldVal = "");
	static int GetFldInt(const TStrKdV& FldNmValPrV, const TStr& FldNm); 
	static int GetFldInt(const TStrKdV& FldNmValPrV, const TStr& FldNm, const int& DefInt); 
	static double GetFldFlt(const TStrKdV& FldNmValPrV, const TStr& FldNm, const double& DefFlt);
	static bool GetFldBool(const TStrKdV& FldNmValPrV, const TStr& FldNm, const bool& DefVal);
	static uint64 GetFldUInt64(const TStrKdV& FldNmValPrV, const TStr& FldNm, const uint64& DefVal);
	static void GetFldValV(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrV& FldValV);
	static void GetFldValSet(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrSet& FldValSet);
	static bool IsFldNmVal(const TStrKdV& FldNmValPrV,	const TStr& FldNm, const TStr& FldVal);
	
	static void SetFldNmVal(TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& FldVal);
	static void SetFldNmInt(TStrKdV& FldNmValPrV, const TStr& FldNm, const int& FldVal);
	static void SetFldNmBool(TStrKdV& FldNmValPrV, const TStr& FldNm, const bool& FldVal);
	static void SetFldNmFlt(TStrKdV& FldNmValPrV, const TStr& FldNm, const double& FldVal);

	static TStr XmlHdStr;

	static PXmlDoc GetErrorXmlRes(const TStr& ErrorStr);
	static TStr GetErrorJsonRes(const TStr& ErrorStr);
private:
	TStr FunNm;
	TSAppOutType OutType;

protected:
    // executes function using parameters passed after ? and returns XML doc
	virtual PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) { 
		EAssert(OutType != saotXml); return TXmlDoc::New(); }
    // executes function using parameters passed after ? and returns JSon doc
	virtual TStr ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
		EAssert(OutType != saotJSon); return ""; };
    // executes function using parameters passed after ? and returns JavaScript doc
	virtual PSIn ExecSIn(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv,
		TStr& ContTypeStr) { EAssert(OutType != saotCustom); return NULL; };

	void LogReqRes(const TStrKdV& FldNmValPrV, const PHttpResp& HttpResp);

	bool NotifyOnRequest;
	bool ReportResponseSize;
	bool LogRqToFile;
	TStr LogRqFolder;

public:
	TSAppSrvFun(const TStr& _FunNm, const TSAppOutType& _OutType = saotXml): 
		FunNm(_FunNm), OutType(_OutType) {
		EAssert(!FunNm.Empty()); 
		NotifyOnRequest = true; 
		LogRqToFile = false; 
		ReportResponseSize = false;
	 }
	virtual ~TSAppSrvFun() { }

	void SetNotifyOnRequest(const bool& Val) { NotifyOnRequest = Val; }
	void SetLogRqToFile(const bool& Val) { LogRqToFile = Val; }
	void SetLogRqFolder(const TStr& Path) { LogRqFolder = Path; }
	void SetReportResponseSize(const bool& Val) { ReportResponseSize = Val; }

	// output type
	TSAppOutType GetFunOutType() const { return OutType; }
    // name of the function, corresponds to URL path
	TStr GetFunNm() const { return FunNm; }
	// executed by server
	virtual void Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};

//////////////////////////////////////
// Simple-App-Server
class TSAppSrv : public TWebSrv {
protected:
	TMem Favicon;
    TBool ShowParamP;
	TBool ListFunP;
    THash<TStr, PSAppSrvFun> FunNmToFunH;

	static unsigned char Favicon_bf[];
	static unsigned int Favicon_len;

public:
    TSAppSrv(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify,
		const bool& _ShowParamP = false, const bool& _ListFunP = true);
    static PWebSrv New(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify, 
		const bool& ShowParamP = false, const bool& ListFunP = true) { 
            return new TSAppSrv(PortN, SrvFunV, Notify, ShowParamP, ListFunP); }
    
    virtual void OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq);
};


//////////////////////////////////////
// Http Request Serialization Info
// info that we serialize for each http request
// this info can later be used to do the replay
class THttpReqSerInfo
{
	TStr UrlRel;
	TStr UrlBase;
	TCh ReqMethod;
	TMem Body;

public:
	THttpReqSerInfo(const TStr& UrlRel, const TStr& UrlBase, const THttpRqMethod& ReqMethod, const TMem& Body);
	THttpReqSerInfo(const PHttpRq& HttpRq);
	THttpReqSerInfo(TSIn& SIn);
	void Save(TSOut& SOut);

	PHttpRq GetHttpRq();
};

class THttpReqLogger;
typedef TPt<THttpReqLogger> PHttpReqLogger;
class THttpReqLogger {
private:
	TCRef CRef;
	THttpReqLogger();
public:
	friend class TPt<THttpReqLogger>;
	static PHttpReqLogger New(const TStr& ReqFNm, const bool& FlushEachReq = false);
	static void ApplyRequests(const TStr& ReqFNm);
};

//////////////////////////////////////
// App-Server with loging and replaying of requests
class TReplaySrv : public TSAppSrv {
private:
	void ReplayHttpRq(const PHttpRq& HttpRq);

	PSOut SOut;
	bool FlushEachRequest;
	THashSet<TStr> LoggingFunNmH;	// the set of function names for which we do the logging

public:
	TReplaySrv(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify,
		const bool& _ShowParamP = false, const bool& _ListFunP = true);
	static TReplaySrv* New(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify,
		const bool& ShowParamP = false, const bool& ListFunP = true) {
		return new TReplaySrv(PortN, SrvFunV, Notify, ShowParamP, ListFunP);
	}
	~TReplaySrv();

	// which functions should be logged - if empty, log all functions
	void AddLoggingFunNm(const TStr& FunNm) { LoggingFunNmH.AddKey(FunNm); }
	void AddLoggingFunNms(const TStrV& FunNmV) { LoggingFunNmH.AddKeyV(FunNmV); }
	void ClearLoggingFunNms() { LoggingFunNmH.Clr(); }

	bool ReplayLog(const TStr& LogFNm, const PNotify& ErrorNotify);
	bool ReplayStream(const PSIn& SIn, const PNotify& ErrorNotify);

	void StartLogging(const TStr& LogFNm, const bool& _FlushEachRequest = false, const bool& Append = true);
	void StopLogging();
	static bool RemoveLogData(const TStr& LogFNm);

	virtual void OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq);
};

//////////////////////////////////////
// Hello-World-Function
class TSASFunHelloWorld : public TSAppSrvFun {
public:
	TSASFunHelloWorld(): TSAppSrvFun("hello") { }
    static PSAppSrvFun New() { return new TSASFunHelloWorld(); }

    PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
        PXmlTok TopTok = TXmlTok::New("logs");
        TopTok->AddSubTok(TXmlTok::New("HelloWorld"));
        for (int FldN = 0; FldN < FldNmValPrV.Len(); FldN++) {
            PXmlTok ArgTok = TXmlTok::New("arg");
            ArgTok->AddArg(FldNmValPrV[FldN].Key, FldNmValPrV[FldN].Dat);
            TopTok->AddSubTok(ArgTok);
        }
        return TXmlDoc::New(TopTok);
    }
};

//////////////////////////////////////
// Exit-Function
class TSASFunExit : public TSAppSrvFun {
public:
	TSASFunExit(): TSAppSrvFun("exit") { }
    static PSAppSrvFun New() { return new TSASFunExit(); }

    PXmlDoc ExecXml(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
        // send message to server to quit
		TLoop::Stop();
        // send the reply
        PXmlTok TopTok = TXmlTok::New("exit");
        return TXmlDoc::New(TopTok);
    }
};

//////////////////////////////////////
// File-Download-Function
// Andrej added public exposure
class TSASFunFile : public TSAppSrvFun {
private:
	TStr FNm;
	TStr ContType;
	
public:
	TSASFunFile(const TStr& _Url, const TStr& _FNm, const TStr& _ContType): 
	  TSAppSrvFun(_Url, saotCustom), FNm(_FNm), ContType(_ContType) { }
	static PSAppSrvFun New(const TStr& Url, const TStr& FNm, const TStr& ContType) {
		return new TSASFunFile(Url, FNm, ContType); }

    PSIn ExecSIn(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv, TStr& ContTypeStr) {
		ContTypeStr = ContType;
        return TFIn::New(FNm);
    }
};

//////////////////////////////////////
// Folder-Download-Function
class TSASFunFPath : public TSAppSrvFun {
private:
	TStr BaseUrl;
	TStr FPath;
	TStr DefaultFNm;
	
public:
	TSASFunFPath(const TStr& _BaseUrl, const TStr& _FPath): 
	  TSAppSrvFun(_BaseUrl, saotCustom), BaseUrl(_BaseUrl), FPath(_FPath), DefaultFNm("index.html") {}

	TSASFunFPath(const TStr& _BaseUrl, const TStr& _FPath, const TStr& _DefaultFNm): 
	  TSAppSrvFun(_BaseUrl, saotCustom), BaseUrl(_BaseUrl), FPath(_FPath), DefaultFNm(_DefaultFNm) { }


	static PSAppSrvFun New(const TStr& BaseUrl, const TStr& FPath) { 
        return new TSASFunFPath(BaseUrl, FPath); }
	static PSAppSrvFun New(const TStr& BaseUrl, const TStr& FPath, const TStr& DefaultFNm) {
		return new TSASFunFPath(BaseUrl, FPath, DefaultFNm); }


    TStr GetFunNm() const { return BaseUrl; }

    PSIn ExecSIn(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv, TStr& ContTypeStr);
};

//////////////////////////////////////
// URL-Redirect-Function
class TSASFunRedirect : public TSAppSrvFun {
private:	
	class TRedirect {
	public:
		TStr FunNm;
		TStrKdV FldNmValPrV;
		TStrStrH FldNmMapH;
	};

	THash<TStr, TRedirect> MapH;
public:
	TSASFunRedirect(const TStr& FunNm, const TStr& SettingFNm);
	static PSAppSrvFun New(const TStr& FunNm, const TStr& SettingFNm) { 
		return new TSASFunRedirect(FunNm, SettingFNm); }

    void Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv);
};



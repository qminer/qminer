/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Web-Page-Fetch
ClassTP(TWebPgFetch, PWebPgFetch)//{
private:
  int TimeOutMSecs;
  int MxConns;
  int MxContLen;
  int MxRetries;
  int LastFId;
  PSockEvent LastDelEvent;
  TStr ProxyStr;
  TStr UserAgentStr;
  int GetNextFId(){LastFId++; return LastFId;}
  void PushWait(const int& FId, const PUrl& Url, const bool& QueueAtEnd = true);
  void PopWait(int& FId, PUrl& Url);
  void OpenConn(const int& FId, const PUrl& Url);
  void CloseConn(const int& FId);
  void ConnUrl(const int& FId = -1, const PUrl& Url = NULL, const bool& QueueAtEnd = true);
  UndefCopyAssign(TWebPgFetch);
protected:
  TIdUrlPrL WaitFIdUrlPrL;
  THash<TInt, PSockEvent> ConnFIdToEventH;
  void DisconnUrl(const int& FId);
public:
  TWebPgFetch():
    TimeOutMSecs(30*1000), MxConns(-1), MxContLen(-1), MxRetries(1), LastFId(0),
    LastDelEvent(), ProxyStr(), UserAgentStr(), WaitFIdUrlPrL(), ConnFIdToEventH(1000) {}
  static PWebPgFetch New(){return new TWebPgFetch();}
  virtual ~TWebPgFetch();

  // load, save
  virtual void Load(TSIn& SIn) {}
  virtual void Save(TSOut& SOut) const {};

  // connections constraints
  void PutTimeOutMSecs(const int& _TimeOutMSecs){
    EAssert(_TimeOutMSecs>0); TimeOutMSecs=_TimeOutMSecs;}
  void PutMxConns(const int& _MxConns){
    EAssert(_MxConns>0); MxConns=_MxConns;}
  int GetMxConns() const {return MxConns;}
  bool IsOkConns(const int& Conns) const {
    return (MxConns==-1)||(Conns<MxConns);}
  void PutMxContLen(const int& _MxContLen){
    EAssert((_MxContLen==-1)||(_MxContLen>=0)); MxContLen=_MxContLen;}
  int GetMxContLen() const {return MxContLen;}
  void PutMxRetries(const int& _MxRetries){
    EAssert(_MxRetries>0); MxRetries=_MxRetries;}
  int GetMxRetries() const {return MxRetries;}
  bool IsOkContLen(const int& ContLen) const {
    return (MxContLen==-1)||(ContLen<=MxContLen);}

  // active connections
  int GetWaitUrls() const {return WaitFIdUrlPrL.Len();}
  int GetConnUrls() const {return ConnFIdToEventH.Len();}
  bool Empty() const {return (GetWaitUrls()==0)&&(GetConnUrls()==0);}
  bool IsConn(const int& ConnFId) const {
    return ConnFIdToEventH.IsKey(ConnFId);}
  PSockEvent GetConn(const int& ConnFId) const {
    return ConnFIdToEventH.GetDat(ConnFId);}
  PUrl GetConnUrl(const int& ConnFId) const;
  void DelConn(const int& ConnFId){
    CloseConn(ConnFId);}
  int GetConnBytesRead(const int& ConnFId) const {
    return IsConn(ConnFId) ? GetConn(ConnFId)->GetBytesRead() : -1;}

  void PutProxyStr(const TStr& _ProxyStr){ProxyStr=_ProxyStr;}
  TStr GetProxyStr() const {return ProxyStr;}
  void PutUserAgentStr(const TStr& _UserAgentStr){UserAgentStr=_UserAgentStr;}
  void PutUserAgentStrIE8(){UserAgentStr="Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 6.0)";}
  TStr GetUserAgentStr() const {return UserAgentStr;}

  int FetchUrl(const PUrl& Url, const bool& QueueAtEnd = true);
  int FetchUrl(const TStr& RelUrlStr, const TStr& BaseUrlStr=TStr());
  int FetchHttpRq(const PHttpRq& HttpRq);
  int EnqueueUrl(const TStr& RelUrlStr, const TStr& BaseUrlStr=TStr());
  void GoFetch(){ConnUrl();}

  virtual void OnFetch(const int& FId, const PWebPg& WebPg){}
  virtual void OnNotify(const int& FId, const TStr& MsgStr){}
  virtual void OnError(const int& FId, const TStr& MsgStr){}
  virtual void OnStatus(const TStr& MsgStr){};

  friend class TWebPgFetchEvent;
};

/////////////////////////////////////////////////
// Web-Fetch-Blocking
class TWebFetchBlocking: public TWebPgFetch{
private:
  bool Ok;
  TStr MsgStr;
  PWebPg WebPg;
  UndefDefaultCopyAssign(TWebFetchBlocking);
public:
  void OnFetch(const int& FId, const PWebPg& _WebPg){
	Ok=true; MsgStr="Ok"; WebPg=_WebPg; TLoop::Unref(); }
  void OnError(const int& FId, const TStr& _MsgStr){
    Ok=false; MsgStr=_MsgStr; WebPg=NULL; TLoop::Unref(); }

  static void GetWebPg(const PUrl& Url, bool& Ok, TStr& MsgStr,
   PWebPg& WebPg, const PNotify& Notify=NULL, const TStr& ProxyStr="",
   const TStr& UserAgentStr="");
  static void GetWebPg(const TStr& UrlStr, bool& Ok, TStr& MsgStr,
   PWebPg& WebPg, const PNotify& Notify=NULL, const TStr& ProxyStr="",
   const TStr& UserAgentStr="");
  static void GetWebPg(const PHttpRq& HttpRq, bool& Ok, TStr& MsgStr,
   PWebPg& WebPg, const PNotify& Notify=NULL, const TStr& ProxyStr="",
   const TStr& UserAgentStr="");
  static bool IsGetWebPg(
   const TStr& UrlStr, TStr& MsgStr, PWebPg& WebPg, const TStr& ProxyStr=""){
    bool Ok; GetWebPg(UrlStr, Ok, MsgStr, WebPg, NULL, ProxyStr); return Ok;}
};

/////////////////////////////////////////////////
// Web-Page-To-File-Fetch

class TWebPgToFileFetch: public TWebPgFetch{
private:
  TStr OutHttpFNm;
  TStr OutHttpBodyFNm;
  TStr OutXmlFNm;
  bool OutXmlTextP;
  bool OutXmlUrlP;
  bool OutXmlToksP;
  bool OutXmlTagsP;
  bool OutXmlArgsP;
  TStr OutTxtFNm;
  bool OutTxtUrlP;
  bool OutScrP;
public:
  TWebPgToFileFetch(
   const TStr _OutHttpFNm, const TStr _OutHttpBodyFNm,
   const TStr _OutXmlFNm, const bool _OutXmlTextP, const bool _OutXmlUrlP,
   const bool _OutXmlToksP, const bool _OutXmlTagsP, const bool _OutXmlArgsP,
   const TStr _OutTxtFNm, const bool _OutTxtUrlP, const bool _OutScrP):
    TWebPgFetch(),
    OutHttpFNm(_OutHttpFNm), OutHttpBodyFNm(_OutHttpBodyFNm),
    OutXmlFNm(_OutXmlFNm), OutXmlTextP(_OutXmlTextP), OutXmlUrlP(_OutXmlUrlP),
    OutXmlToksP(_OutXmlToksP), OutXmlTagsP(_OutXmlTagsP), OutXmlArgsP(_OutXmlArgsP),
    OutTxtFNm(_OutTxtFNm), OutTxtUrlP(_OutTxtUrlP), OutScrP(_OutScrP){}
  void OnFetch(const int& FId, const PWebPg& WebPg);
  void OnError(const int& FId, const TStr& MsgStr);
};

/////////////////////////////////////////////////
// Web-Fetch-SendBatchJson
class TWebFetchSendBatchJson: public TWebPgFetch{ 
private:
  // Parameters
  int MSecInterval;
  int BatchSize;
  TStr UrlStr; 
  // State
  uint64 LastRestart;
  bool Ok;
  PJsonVal JsonArray;
  void Send();
  uint64 NumSent;
protected:
  void OnFetch(const int& FId, const PWebPg& _WebPg){Ok=true;}
  void OnError(const int& FId, const TStr& _MsgStr){Ok=false;}
public:
  TWebFetchSendBatchJson() {NumSent = 0;};
  TWebFetchSendBatchJson(const TStr& UrlStr_);
  void SetBatchSize(const int& BatchSize_);
  void SetMSecInterval(const int& MSecInterval_);
  void SendJson(const PJsonVal& Json); 
  uint64 GetNumSent() {return NumSent;}
};



// the persisten version of the web page fetch class
// if SaveFName is provided then the class will save in the provided file name
// the page requests that are still in the queue
// on creating the class instance the requests will be loaded from the file 
// and again requested

// if RepeatFailedRequests == true then the failed requests will be again requeued
// useful for calling web services that often timeout
// 
ClassTPE(TWebPgFetchPersist, PWebPgFetchPersist, TWebPgFetch)//{
protected:
	TInt SuccessCount;
	TInt ErrorCount;
	TStr SaveFName;
	bool RepeatFailedRequests;
	bool ReportState;
	TStr ReportPrefix;
	PNotify Notify;

	TWebPgFetchPersist(const TStr& _SaveFName = "", const bool& _RepeatFailedRequests = true, const bool& _ReportState = false, const TStr& _ReportPrefix = "", const PNotify& _Notify = TNotify::NullNotify);
	
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;
	
public:
	~TWebPgFetchPersist();

	static PWebPgFetchPersist New(const TStr& SaveFName = "", const bool& RepeatFailedRequests = true, const bool& ReportState = false, const TStr& ReportPrefix = "") {
		return new TWebPgFetchPersist(SaveFName, RepeatFailedRequests, ReportState, ReportPrefix);
	}

	virtual void ReportError(const TStr& MsgStr);
	virtual void OnFetch(const int& FId, const PWebPg& WebPg);
	virtual void OnError(const int& FId, const TStr& MsgStr);
	virtual void Report();

};


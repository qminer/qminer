/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

/////////////////////////////////////////////////
// Web-Page-Fetch-Event
class TWebPgFetchEvent: public TSockEvent{
private:
  TWebPgFetch* Fetch;
  int FId;
  PUrl TopUrl;
  TStr ProxyStr;
  TStr UserAgentStr;
  PUrl CurUrl;
  TStrV UrlStrV;
  TStrV IpNumV;
  TStrQuV CookieKeyValDmPathQuV;
  PSockHost SockHost;
  PSock Sock;
  int Retries;
  int TimeOutMSecs;
  TBool OppSockClosed;
  TMem SockMem;
  uint64 StartMSecs, EndMSecs;
  void AddCookieKeyValDmPathQuV(const TStrQuV& KeyValDmPathQuV);
  int GetCookies() const {return CookieKeyValDmPathQuV.Len();}
  void GetCookie(const int& CookieN, TStr& KeyNm, TStr& ValStr, TStr& DmNm, TStr& PathStr) const {
    KeyNm=CookieKeyValDmPathQuV[CookieN].Val1; ValStr=CookieKeyValDmPathQuV[CookieN].Val2; 
    DmNm=CookieKeyValDmPathQuV[CookieN].Val3; PathStr=CookieKeyValDmPathQuV[CookieN].Val4;}
  void ChangeLastUrlToLc(const PHttpResp& HttpResp);
  void CloseConn(){SockHost.Clr(); Sock.Clr();}
public:
  TWebPgFetchEvent(TWebPgFetch* _Fetch, const int& _FId,
   const PUrl& _Url, const TStr& _ProxyStr, const TStr& _UserAgentStr,
   const int& _TimeOutMSecs);
  ~TWebPgFetchEvent(){}

  TSockEvent& operator=(const TSockEvent&){Fail; return *this;}

  void OnFetchError(const TStr& MsgStr);
  void OnFetchEnd(const PHttpResp& HttpResp);
  void OnFetchEnd();

  void OnRead(const uint64& SockId, const PSIn& SIn);
  void OnWrite(const uint64& SockId) { }
  void OnAccept(const uint64& SockId, const PSock&){Fail;}
  void OnConnect(const uint64& SockId);
  void OnReadEof(const uint64& SockId);
  void OnClose(const uint64& SockId);
  void OnTimeOut(const uint64& SockId);
  void OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr);
  void OnGetHost(const PSockHost& SockHost);

  int GetFetchMSecs() const {
  if ((StartMSecs==TUInt64::Mx)||(EndMSecs==TUInt64::Mx)){return -1;} else {return int(EndMSecs-StartMSecs);}}
  int GetBytesRead() const {
    return SockMem.Len();}
  PUrl GetUrl() const { 
  return TopUrl;}
};

void TWebPgFetchEvent::AddCookieKeyValDmPathQuV(const TStrQuV& KeyValDmPathQuV){
  for (int KeyN=0; KeyN<KeyValDmPathQuV.Len(); KeyN++){
    TStr KeyNm=KeyValDmPathQuV[KeyN].Val1;
    TStr ValStr=KeyValDmPathQuV[KeyN].Val2;
    TStr DmNm=KeyValDmPathQuV[KeyN].Val3;
    TStr PathStr=KeyValDmPathQuV[KeyN].Val4;
    bool IsUpdated=false;
    for (int CookieN=0; CookieN<CookieKeyValDmPathQuV.Len(); CookieN++){
      TStr CookieKeyNm=CookieKeyValDmPathQuV[CookieN].Val1;
      TStr CookieDmNm=CookieKeyValDmPathQuV[CookieN].Val3;
      if ((KeyNm==CookieKeyNm)&&(DmNm==CookieDmNm)){
        CookieKeyValDmPathQuV[CookieN].Val2=ValStr;
        IsUpdated=true; break;
      }
    }
    if (!IsUpdated){
      CookieKeyValDmPathQuV.Add(TStrQu(KeyNm, ValStr, DmNm, PathStr));
    }
  }
}

void TWebPgFetchEvent::ChangeLastUrlToLc(const PHttpResp& HttpResp){
  static TStr MsNm="Microsoft";
  static TStr HttpsNm="HTTPS";
  TStr SrvNm=HttpResp->GetSrvNm();
  if ((SrvNm.StartsWith(MsNm))||(SrvNm.StartsWith(HttpsNm))){
    if (!UrlStrV.Last().IsLc()){
      PUrl Url=TUrl::New(UrlStrV.Last());
      Url->ToLcPath();
      UrlStrV.Last()=Url->GetUrlStr();
    }
  }
}

TWebPgFetchEvent::TWebPgFetchEvent(TWebPgFetch* _Fetch, const int& _FId, 
 const PUrl& _Url, const TStr& _ProxyStr, const TStr& _UserAgentStr,
 const int& _TimeOutMSecs):
  TSockEvent(),
  Fetch(_Fetch), FId(_FId),
  TopUrl(_Url), ProxyStr(_ProxyStr), UserAgentStr(_UserAgentStr), 
  CurUrl(_Url), UrlStrV(),
  SockHost(NULL), Sock(NULL),
  Retries(0), TimeOutMSecs(_TimeOutMSecs), OppSockClosed(true),
  SockMem(), StartMSecs(TTm::GetCurUniMSecs()), EndMSecs(TUInt64::Mx){
}

void TWebPgFetchEvent::OnFetchError(const TStr& MsgStr){
  Fetch->OnError(FId, MsgStr+" ["+CurUrl->GetUrlStr()+"]");
  CloseConn(); Fetch->DisconnUrl(FId);
}

void TWebPgFetchEvent::OnFetchEnd(const PHttpResp& HttpResp){
  EAssert(HttpResp->IsOk());
  EndMSecs=TTm::GetCurUniMSecs();
  int StatusCd=HttpResp->GetStatusCd();
  if (StatusCd/100==2){ // codes 2XX - ok
    ChangeLastUrlToLc(HttpResp);
    PWebPg WebPg=TWebPg::New(UrlStrV, IpNumV, HttpResp);
    WebPg->PutFetchMSecs(GetFetchMSecs());
    CloseConn(); Fetch->DisconnUrl(FId);
    Fetch->OnFetch(FId, WebPg);
  } else
  if (StatusCd/100==3){ // codes 3XX - redirection
    ChangeLastUrlToLc(HttpResp);
    if (UrlStrV.Len()<5){
      TStr RedirUrlStr=HttpResp->GetFldVal("Location");
      PUrl RedirUrl=TUrl::New(RedirUrlStr, CurUrl->GetUrlStr());
      if (RedirUrl->IsOk(usHttp)){
        TStrQuV CookieQuV; HttpResp->GetCookieKeyValDmPathQuV(CookieQuV);
        AddCookieKeyValDmPathQuV(CookieQuV);
        CurUrl=RedirUrl; Retries=0; CloseConn();
        TSockHost::GetAsyncSockHost(CurUrl->GetIpNumOrHostNm(), this);
      } else {
        TStr MsgStr=TStr("Invalid Redirection URL (")+RedirUrlStr+")";
        OnFetchError(MsgStr);
      }
    } else {
      TStr MsgStr=TStr("Cycling Redirection [")+TopUrl->GetUrlStr()+"]";
      OnFetchError(MsgStr);
    }
  } else { // all other codes - error
    TStr MsgStr=TStr("Http Error (")+
     TInt::GetStr(StatusCd)+"/"+HttpResp->GetReasonPhrase()+")";
    OnFetchError(MsgStr);
  }
}

void TWebPgFetchEvent::OnFetchEnd(){
  PSIn SIn=TMemIn::New(SockMem);
  PHttpResp HttpResp=THttpResp::LoadTxt(SIn);
  if (HttpResp->IsOk()){
    OnFetchEnd(HttpResp);
  } else {
    OnFetchError("Invalid Http Response");
  }
}

void TWebPgFetchEvent::OnRead(const uint64& SockId, const PSIn& SIn){
  Sock->PutTimeOut(TimeOutMSecs);
  SockMem+=SIn;
  if (!Fetch->IsOkContLen(SockMem.Len())){
    OnFetchEnd();
  }
}

void TWebPgFetchEvent::OnConnect(const uint64& SockId){
  TChA RqChA;
  if (CurUrl->IsHttpRqStr()){
    RqChA=CurUrl->GetHttpRqStr();
  } else {
    // get http components
    TStr HostNm=CurUrl->GetHostNm();
    TStr AbsPath=CurUrl->GetPathStr()+CurUrl->GetSearchStr();
    // compose http request
    RqChA+="GET "; RqChA+=AbsPath; RqChA+=" HTTP/1.0\r\n";
    RqChA+="Host: "; RqChA+=HostNm; RqChA+="\r\n";
    if (!UserAgentStr.Empty()){ 
      RqChA+="User-Agent: "; RqChA+=UserAgentStr; RqChA+="\r\n";}
    // add cookies
    if (GetCookies()>0){
      RqChA+="Cookie: ";
      for (int CookieN=0; CookieN<GetCookies(); CookieN++){
        TStr KeyNm; TStr ValStr; TStr DmNm; TStr PathStr;
        GetCookie(CookieN, KeyNm, ValStr, DmNm, PathStr);
        if (HostNm.EndsWith(DmNm)){
          if (CookieN>0){RqChA+="; ";}
          RqChA+=KeyNm; RqChA+='='; RqChA+=ValStr; 
        }
      }
      RqChA+="\r\n";
    }
    // finish request
    RqChA+="\r\n";
  }
  // send http request
  PSIn RqSIn=TMIn::New(RqChA);
  bool Ok; TStr ErrMsg; Sock->Send(RqSIn, Ok, ErrMsg);
  if (Ok){
    Sock->PutTimeOut(TimeOutMSecs);
  } else {
    OnFetchError("Unable to send the data: " + ErrMsg);
  }
}

void TWebPgFetchEvent::OnReadEof(const uint64& SockId){
  OppSockClosed=true;
  OnFetchEnd();
}

void TWebPgFetchEvent::OnClose(const uint64& SockId){
  OppSockClosed=true;
  OnFetchEnd();
}

void TWebPgFetchEvent::OnTimeOut(const uint64& SockId){
  if (OppSockClosed){
    OnFetchEnd();
  } else {
    Retries++;
    if (Retries<Fetch->GetMxRetries()){
      OnGetHost(SockHost);
    } else {
      OnFetchError("Timeout");
    }
  }
}

void TWebPgFetchEvent::OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr){
  Retries++;
  if (Retries<Fetch->GetMxRetries()){
    OnGetHost(SockHost);
  } else {
    OnFetchError(TStr("Socket Error (")+ErrStr+")");
  }
}

void TWebPgFetchEvent::OnGetHost(const PSockHost& SockHost) {
  if (SockHost->IsOk()){
    UrlStrV.Add(CurUrl->GetUrlStr());
    OppSockClosed=false;
    SockMem.Clr();
    Sock=TSock::New(this);
    int PortN;
    if (ProxyStr.Empty()){
      PortN=CurUrl->GetPortN();
    } else {
      TStr ProxyHostNm; TStr PortNStr;
      ProxyStr.SplitOnCh(ProxyHostNm, ':', PortNStr);
      PortN=PortNStr.GetInt(80);
    }
    Sock->PutTimeOut(TimeOutMSecs);
    Sock->Connect(SockHost, PortN);
  } else {
    OnFetchError("Invalid Host");
  }
}

/////////////////////////////////////////////////
// Web-Page-Fetch
void TWebPgFetch::PushWait(const int& FId, const PUrl& Url, const bool& QueueAtEnd){
  if (QueueAtEnd)
    WaitFIdUrlPrL.AddBack(TIdUrlPr(FId, Url));
  // add item to the front of the queue
  else
    WaitFIdUrlPrL.AddFront(TIdUrlPr(FId, Url));
}

void TWebPgFetch::PopWait(int& FId, PUrl& Url){
	TLstNd<TIdUrlPr>* LstNd = WaitFIdUrlPrL.First();
	FId=LstNd->Val.Val1;
	Url = LstNd->Val.Val2;
	WaitFIdUrlPrL.Del(LstNd);
}

void TWebPgFetch::OpenConn(const int& FId, const PUrl& Url){
  PSockEvent Event=PSockEvent(new TWebPgFetchEvent(this, FId, Url, ProxyStr, UserAgentStr, TimeOutMSecs));
  TSockEvent::Reg(Event);
  ConnFIdToEventH.AddDat(FId, Event);
  TStr HostNm;
  if (ProxyStr.Empty()){HostNm=Url->GetIpNumOrHostNm();}
  else {TStr PortNStr; ProxyStr.SplitOnCh(HostNm, ':', PortNStr);}
  TSockHost::GetAsyncSockHost(HostNm, Event);
}

void TWebPgFetch::CloseConn(const int& FId){
	if (ConnFIdToEventH.IsKey(FId)) {
		PSockEvent Event=ConnFIdToEventH.GetDat(FId);
		TSockEvent::UnReg(Event);
		ConnFIdToEventH.DelKey(FId);
		Event->CloseConn();
		LastDelEvent=Event;
	}
}

void TWebPgFetch::ConnUrl(const int& FId, const PUrl& Url, const bool& QueueAtEnd){
  if (FId!=-1) {
	  PushWait(FId, Url, QueueAtEnd);
  }
  while ((IsOkConns(GetConnUrls()))&&(GetWaitUrls()>0)){
    int FId; PUrl Url; PopWait(FId, Url);
    OpenConn(FId, Url);
  }
}

void TWebPgFetch::DisconnUrl(const int& FId){
  CloseConn(FId);
  ConnUrl();
}

TWebPgFetch::~TWebPgFetch(){
  TIntV ConnFIdV; ConnFIdToEventH.GetKeyV(ConnFIdV);
  for (int ConnFIdN=0; ConnFIdN<ConnFIdV.Len(); ConnFIdN++){
    CloseConn(ConnFIdV[ConnFIdN]);
  }
}

PUrl TWebPgFetch::GetConnUrl(const int& ConnFId) const {
  TWebPgFetchEvent* Event = (TWebPgFetchEvent*)GetConn(ConnFId)();
  return Event->GetUrl();
}

int TWebPgFetch::FetchUrl(const PUrl& Url, const bool& QueueAtEnd){
  int FId=-1;
  if (Url->IsOk(usHttp) && Url->IsPortOk()){
    FId=GetNextFId();
	ConnUrl(FId, Url, QueueAtEnd);
  } else {
    TStr MsgStr=TStr("Invalid URL [")+Url->GetUrlStr()+"]";
    OnError(FId, MsgStr);
  }
  return FId;
}

int TWebPgFetch::FetchUrl(const TStr& RelUrlStr, const TStr& BaseUrlStr){
  PUrl Url=TUrl::New(RelUrlStr, BaseUrlStr);
  return FetchUrl(Url);
}

int TWebPgFetch::FetchHttpRq(const PHttpRq& HttpRq){
  PUrl Url=HttpRq->GetUrl();
  Url->PutHttpRqStr(HttpRq->GetStr()); //**
  return FetchUrl(Url);
}

int TWebPgFetch::EnqueueUrl(const TStr& RelUrlStr, const TStr& BaseUrlStr){
  int FId=-1;
  PUrl Url=TUrl::New(RelUrlStr, BaseUrlStr);
  if (Url->IsOk(usHttp)){
    FId=GetNextFId();
    PushWait(FId, Url);
  } else {
    TStr MsgStr=TStr("Invalid URL [")+RelUrlStr+"] + ["+BaseUrlStr+"]";
    OnError(FId, MsgStr);
  }
  return FId;
}

/////////////////////////////////////////////////
// Web-Fetch-Blocking
void TWebFetchBlocking::GetWebPg(const PUrl& Url, bool& Ok, TStr& MsgStr,
 PWebPg& WebPg, const PNotify& Notify, const TStr& ProxyStr, const TStr& UserAgentStr){
  TNotify::OnNotify(Notify, ntInfo, TStr("Fetching: ")+Url->GetUrlStr());
  TWebFetchBlocking WebFetch;
  if (!UserAgentStr.Empty()) { WebFetch.PutUserAgentStr(UserAgentStr); }
  WebFetch.PutProxyStr(ProxyStr);
  WebFetch.FetchUrl(Url);
  TLoop::Ref();
  TLoop::Run();
  Ok=WebFetch.Ok; MsgStr=WebFetch.MsgStr; WebPg=WebFetch.WebPg;
  if (!Ok){
    TNotify::OnNotify(Notify, ntInfo, TStr("Fetching Error: ["+MsgStr+"]"));
  }
}

void TWebFetchBlocking::GetWebPg(const TStr& UrlStr, bool& Ok, TStr& MsgStr,
 PWebPg& WebPg, const PNotify& Notify, const TStr& ProxyStr, const TStr& UserAgentStr){
  TNotify::OnNotify(Notify, ntInfo, TStr("Fetching: ")+UrlStr);
  TWebFetchBlocking WebFetch;
  if (!UserAgentStr.Empty()) { WebFetch.PutUserAgentStr(UserAgentStr); }
  WebFetch.PutProxyStr(ProxyStr);
  WebFetch.FetchUrl(UrlStr);
  TLoop::Ref();
  TLoop::Run();
  Ok=WebFetch.Ok; MsgStr=WebFetch.MsgStr; WebPg=WebFetch.WebPg;
  if (!Ok){
    TNotify::OnNotify(Notify, ntInfo, TStr("Fetching Error: ["+MsgStr+"]"));
  }
}

void TWebFetchBlocking::GetWebPg(const PHttpRq& HttpRq, bool& Ok, TStr& MsgStr,
 PWebPg& WebPg, const PNotify& Notify, const TStr& ProxyStr, const TStr& UserAgentStr){
  TNotify::OnNotify(Notify, ntInfo, TStr("Fetching: ")+HttpRq->GetUrl()->GetUrlStr());
  TWebFetchBlocking WebFetch;
  if (!UserAgentStr.Empty()) { WebFetch.PutUserAgentStr(UserAgentStr); }
  WebFetch.PutProxyStr(ProxyStr);
  WebFetch.FetchHttpRq(HttpRq);
  TLoop::Ref();
  TLoop::Run();
  Ok=WebFetch.Ok; MsgStr=WebFetch.MsgStr; WebPg=WebFetch.WebPg;
  if (!Ok){
    TNotify::OnNotify(Notify, ntInfo, TStr("Fetching Error: ["+MsgStr+"]"));
  }
}

/////////////////////////////////////////////////
// Web-Page-To-File-Fetch
void TWebPgToFileFetch::OnFetch(const int&, const PWebPg& WebPg){
  // save web-page as http
  if (!OutHttpFNm.Empty()){
    WebPg->SaveAsHttp(OutHttpFNm);}
  // save http-body
  if (!OutHttpBodyFNm.Empty()){
    WebPg->SaveAsHttpBody(OutHttpBodyFNm);}
  // save web-page as xml
  if (!OutXmlFNm.Empty()){
    TStr HtmlStr=WebPg->GetHttpBodyAsStr();
    THtmlDoc::SaveHtmlToXml(HtmlStr, OutXmlFNm, WebPg->GetUrlStr(),
     OutXmlTextP, OutXmlUrlP, OutXmlToksP, OutXmlTagsP, OutXmlArgsP);
  }
  // save web-page as text
  if (!OutTxtFNm.Empty()){
    TStr HtmlStr=WebPg->GetHttpBodyAsStr();
    THtmlDoc::SaveHtmlToTxt(HtmlStr, OutTxtFNm, WebPg->GetUrlStr(),
     OutTxtUrlP, OutXmlTagsP);
  }
  // output to screen
  if (OutScrP){
    printf("%s\n", WebPg->GetUrlStr().CStr());
    printf("-----------------------\n");
    printf("%s", WebPg->GetHttpHdStr().CStr());
    printf("%s", WebPg->GetHttpBodyAsStr().CStr());
  }
}

void TWebPgToFileFetch::OnError(const int&, const TStr& MsgStr){
  // save error-message as http-body
  if (!OutHttpBodyFNm.Empty()){
    TFOut SOut(OutHttpBodyFNm); TFileId SOutFId=SOut.GetFileId();
    fprintf(SOutFId, "Error: %s\n", MsgStr.CStr());
  }
  // save error-message as http
  if (!OutHttpFNm.Empty()){
    TFOut SOut(OutHttpFNm); TFileId SOutFId=SOut.GetFileId();
    fprintf(SOutFId, "Error: %s\n", MsgStr.CStr());
  }
  // save error-message as xml
  if (!OutXmlFNm.Empty()){
    // create output file
    TFOut SOut(OutXmlFNm); TFileId SOutFId=SOut.GetFileId();
    // open top tag
    fprintf(SOutFId, "<WebPage>\n");
    // save error message
    TStr XmlMsgStr=TXmlLx::GetXmlStrFromPlainStr(MsgStr);
    fprintf(SOutFId, "  <Message>%s</Message>\n", XmlMsgStr.CStr());
    // close top tag
    fprintf(SOutFId, "</WebPage>\n");
  }
  // output to screen
  if (OutScrP){
    printf("%s\n", MsgStr.CStr());
  }
}

/////////////////////////////////////////////////
// Web-Fetch-SendBatchJson
TWebFetchSendBatchJson::TWebFetchSendBatchJson(const TStr& UrlStr_): 
 MSecInterval(-1), BatchSize(-1), UrlStr(UrlStr_), Ok(true) {
  LastRestart = TTm::GetCurUniMSecs();  
  JsonArray = TJsonVal::NewArr();
  NumSent = 0;
}

void TWebFetchSendBatchJson::SetBatchSize(const int& BatchSize_) {
  BatchSize = BatchSize_;
}

void TWebFetchSendBatchJson::SetMSecInterval(const int& MSecInterval_) {
  MSecInterval = MSecInterval_;
}

void TWebFetchSendBatchJson::SendJson(const PJsonVal& Json) {
  NumSent++;
  JsonArray->AddToArr(Json);
  uint64 CurTmMSec = TTm::GetCurUniMSecs();
  int64 DiffMSec = (int64)CurTmMSec - (int64)LastRestart;

  bool SendNow = false;

  //Time based condition:
  if (MSecInterval > 0) {
    if (DiffMSec > MSecInterval) {
      SendNow = true;
      LastRestart = CurTmMSec;
    }
  }
  if (BatchSize > 0) {
    if (JsonArray->GetArrVals() >= BatchSize) {
      SendNow = true;
    }
  }

  if (SendNow) {
    Send();
    JsonArray = TJsonVal::NewArr();
  }
}

void TWebFetchSendBatchJson::Send() {
  TStr BodyStr = TJsonVal::GetStrFromVal(JsonArray);
  TMem BodyMem; BodyMem.AddBf(BodyStr.CStr(), BodyStr.Len());
  PHttpRq HttpRq = THttpRq::New(hrmPost, TUrl::New(UrlStr), THttp::AppJSonFldVal, BodyMem);
  FetchHttpRq(HttpRq);
}


/////////////////////////////////////////////////
// TWebPgFetchPersist
TWebPgFetchPersist::TWebPgFetchPersist(const TStr& _SaveFName, const bool& _RepeatFailedRequests, const bool& _ReportState, const TStr& _ReportPrefix, const PNotify& _Notify) :
SaveFName(_SaveFName), RepeatFailedRequests(_RepeatFailedRequests), ReportState(_ReportState), ReportPrefix(_ReportPrefix), Notify(_Notify)
{
	PutMxConns(10);
	PutTimeOutMSecs(30 * 1000);
	if (SaveFName != "" && TFile::Exists(SaveFName)) {
		TFIn FIn(SaveFName);
		uint64 FileLen = TFile::GetSize(SaveFName);
		if (FileLen > 0)
			TNotify::StdNotify->OnStatusFmt("Loading %.2f MB of pending web requests...", FileLen / (double) TInt::Mega);
		Load(FIn);
		if (FileLen > 0)
			TNotify::StdNotify->OnStatus("Loading finished.");
	}
}

TWebPgFetchPersist::~TWebPgFetchPersist() {
	if (SaveFName != "") {
		TFOut FOut(SaveFName);
		Save(FOut);
	}
}

void TWebPgFetchPersist::Load(TSIn& SIn)
{
	// load PUrls and call FetchUrl on each of them
	int Count = 0;
	while (!SIn.Eof()) {
		try {
			PUrl Url = TUrl::Load(SIn);
			FetchUrl(Url);
			Count++;
		}
		catch (PExcept ex) {
			Notify->OnStatusFmt("TWebPgFetchPersist.Load. Exception while loading url: %s", ex->GetMsgStr().CStr());
		}
		catch (...) {
			Notify->OnStatus("TWebPgFetchPersist.Load. Unrecognized exception while loading a url.");
		}
	}
}

void TWebPgFetchPersist::Save(TSOut& SOut) const
{
	if (ConnFIdToEventH.Len() > 0)
		Notify->OnStatusFmt("TWebPgFetchPersist.Save. Saving %d pending web requests to disk...", ConnFIdToEventH.Len());
	// serialize requests in the queue
	for (int KeyId = ConnFIdToEventH.FFirstKeyId(); ConnFIdToEventH.FNextKeyId(KeyId);) {
		const int FId = ConnFIdToEventH.GetKey(KeyId);
		PUrl Url = GetConnUrl(FId);
		Url->Save(SOut);
	}
	// serialize requests that are currently in progress
	TLstNd<TIdUrlPr>* Item = WaitFIdUrlPrL.First();
	while (Item != NULL) {
		Item->Val.Val2->Save(SOut);;
		Item = Item->Next();
	}
	if (ConnFIdToEventH.Len() > 0)
		Notify->OnStatusFmt("TWebPgFetchPersist.Save. Saved %d requests to the disk.", ConnFIdToEventH.Len());
}

void TWebPgFetchPersist::ReportError(const TStr& MsgStr) 
{
	TNotify::StdNotify->OnStatusFmt("Error info: %s\n", MsgStr.CStr());
	/*if (!Notify.Empty())
	Notify->OnStatusFmt("TWebPgFetchPersist.ReportError: %s", MsgStr.CStr());*/
}

void TWebPgFetchPersist::OnFetch(const int& FId, const PWebPg& WebPg) 
{
	SuccessCount++; 
	Report();
}

void TWebPgFetchPersist::OnError(const int& FId, const TStr& MsgStr) 
{
	ErrorCount++;
	ReportError(MsgStr);
	Report();
	// in case of bad request don't make the request again
	/*if (!HttpResp.Empty() && HttpResp->GetStatusCd() == 400) {
	TStr Url = "";
	if (IsConn(FId))
	Url = GetConnUrl(FId)->GetUrlStr();
	if (!Notify.Empty()) {
	TStr Error = "TWebPgFetchPersist.OnError: Received http response 400 (Bad request). Skipping the request. Request: " + Url;
	Notify->OnStatus(Error.CStr());
	}
	return;
	}
	else if (!HttpResp.Empty() && !Notify.Empty()) {
	Notify->OnStatusFmt("TWebPgFetchPersist.OnError: Received http response %d.", HttpResp->GetStatusCd());
	}*/

	if (IsConn(FId) && RepeatFailedRequests) {
		PUrl Url = GetConnUrl(FId);
		FetchUrl(Url, false);	// enqueue request at the beginning of the queue		
	}
}

void TWebPgFetchPersist::Report() 
{
	if (ReportState)
		printf("%squeue size: %6d. Completed: %6d. Failed: %6d\r", ReportPrefix.CStr(), WaitFIdUrlPrL.Len(), SuccessCount.Val, ErrorCount.Val);
}

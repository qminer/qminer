/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Web-Server-Socket-Event
void TWebSrvSockEvent::OnRead(const uint64& SockId, const PSIn& SIn){
  WebSrv->OnRead(SockId, SIn);}
void TWebSrvSockEvent::OnWrite(const uint64& SockId){
  WebSrv->OnWrite(SockId);}
void TWebSrvSockEvent::OnAccept(const uint64& SockId, const PSock& Sock){
  WebSrv->OnAccept(SockId, Sock);}
void TWebSrvSockEvent::OnConnect(const uint64& SockId){
  WebSrv->OnConnect(SockId);}
void TWebSrvSockEvent::OnReadEof(const uint64& SockId){
  WebSrv->OnReadEof(SockId);}
void TWebSrvSockEvent::OnClose(const uint64& SockId){
  WebSrv->OnClose(SockId);}
void TWebSrvSockEvent::OnTimeOut(const uint64& SockId){
  WebSrv->OnTimeOut(SockId);}
void TWebSrvSockEvent::OnError(
 const uint64& SockId, const int& ErrCd, const TStr& ErrStr){
  WebSrv->OnError(SockId, ErrCd, ErrStr);}
void TWebSrvSockEvent::OnGetHost(const PSockHost& SockHost){
  WebSrv->OnGetHost(SockHost);}

/////////////////////////////////////////////////
// Web-Server
TWebSrv::TWebSrv(
 const int& _PortN, const bool& FixedPortNP, const PNotify& _Notify):
  Notify(_Notify),
  PortN(_PortN),
  HomeNrFPath(TStr::GetNrFPath(TDir::GetCurDir())),
  SockEvent(), Sock(),
  SockIdToConnH(){
  SockEvent=PSockEvent(new TWebSrvSockEvent(this));
  TSockEvent::Reg(SockEvent);
  Sock=TSock::New(SockEvent);
  try {
    if (FixedPortNP){
      Sock->Listen(PortN);
    } else {
      Fail;
      //PortN=Sock->GetPortAndListen(PortN);
    }
  } catch (...){TSockEvent::UnReg(SockEvent); throw;}
  // notify
  TChA MsgChA;
  MsgChA+="Web-Server: Started at port ";
  MsgChA+=TInt::GetStr(PortN); MsgChA+=".";
  TNotify::OnNotify(Notify, ntInfo, MsgChA);
}

TWebSrv::~TWebSrv(){
  TSockEvent::UnReg(SockEvent);
  TNotify::OnNotify(Notify, ntInfo, "Web-Server: Stopped.");
}

void TWebSrv::OnRead(const uint64& SockId, const PSIn& SIn){
  // take packet contents
  TChA PckChA; TChA::LoadTxt(SIn, PckChA);
  // return & do nothing if empty packet
  if (PckChA.Empty()){return;}
  // save packet to request string
  TChA& HttpRqChA=GetConn(SockId)->GetHttpRqChA();
  HttpRqChA+=PckChA;
  // test if the request is ok
  PSIn HttpRqSIn=TMIn::New(HttpRqChA);
  PHttpRq HttpRq=THttpRq::New(HttpRqSIn);
  // write request to file
  //{PSOut HttpRqSIn=TFOut::New("HttpRq.txt"); HttpRqSIn->PutStr(HttpRqChA);} //**
  // send request if http-request complete
  if (HttpRq->IsComplete()){
    GetConn(SockId)->PutType(wsctWaitingToRespond);
    OnHttpRq(SockId, HttpRq);
  }
}

void TWebSrv::OnWrite(const uint64& SockId){
  if (IsConn(SockId) && GetConn(SockId)->GetType()==wsctSending){
    // delete connection when everything sent
    DelConn(SockId);
  }
}

void TWebSrv::OnAccept(const uint64& SockId, const PSock& Sock){
  // create new connection
  PWebSrvConn Conn=TWebSrvConn::New(Sock, this);
  AddConn(SockId, Conn);
  Sock->PutTimeOut(25*1000);
  Conn->PutType(wsctReceiving);
  // send message
  //TStr MsgStr=TStr("New Request [")+TInt::GetStr(SockId)+"]"; //**
  //TNotify::OnNotify(Notify, ntInfo, MsgStr); //**
}

void TWebSrv::OnReadEof(const uint64& SockId) {
  // connection by peer, delete if we still have it
  DelConn(SockId);
  // send message
  //TStr MsgStr=TStr("Closed by peer [")+TInt::GetStr(SockId)+"]"; //**
  //TNotify::OnNotify(Notify, ntInfo, MsgStr); //**
}

void TWebSrv::OnClose(const uint64& SockId){
  // connection closed, delete if we still have it
  DelConn(SockId);
  // send message
  //TStr MsgStr=TStr("Closed by peer [")+TInt::GetStr(SockId)+"]"; //**
  //TNotify::OnNotify(Notify, ntInfo, MsgStr); //**
}

void TWebSrv::OnTimeOut(const uint64& SockId){
  // cancle connection
  DelConn(SockId);
  // send message
  //TStr MsgStr=TStr("Timeout [")+TInt::GetStr(SockId)+"]"; //**
  //TNotify::OnNotify(Notify, ntInfo, MsgStr); //**
}

void TWebSrv::OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr){
  // cancle connection
  DelConn(SockId);
  // notify
  TChA MsgChA; MsgChA+="Web-Server: Error: '"; MsgChA+=ErrStr; MsgChA+="'.";
  TNotify::OnNotify(Notify, ntInfo, MsgChA);
}

void TWebSrv::OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq){
  // check http-request correctness - return if error
  if (!HttpRq->IsOk()){
    TNotify::OnNotify(Notify, ntInfo, "Web-Server: Bad Http Request.");
    return;
  }
  // check url correctness - return if error
  PUrl RqUrl=HttpRq->GetUrl();
  if (!RqUrl->IsOk()){
    TNotify::OnNotify(Notify, ntInfo, "Web-Server: Bad Url Requested.");
    return;
  }

  // construct http-response
  PHttpResp HttpResp;
  if (!RqUrl->GetPathStr().Empty()){
    // get request-file-name
    TStr ExeFPath=TSysProc::GetExeFNm().GetFPath();
    TStr RqFNm=RqUrl->GetPathStr();
    if (RqFNm.LastCh()=='/'){RqFNm=RqFNm+"default.htm";}
    if ((RqFNm[0]=='/')||(RqFNm[0]=='\\')){RqFNm.DelSubStr(0, 0);}
    RqFNm=ExeFPath+RqFNm;
    // open file
    bool RqFOpened=false; PSIn RqSIn=TFIn::New(RqFNm, RqFOpened);
    if (!RqFOpened){
      // prepare default html with time
      TChA HtmlChA;
      HtmlChA+="<html><title>Error - Not Found</title><body>";
      HtmlChA+="File: "; HtmlChA+=RqUrl->GetPathStr(); HtmlChA+=" not found.";
      HtmlChA+="</body></html>";
      PSIn BodySIn=TMIn::New(HtmlChA);
      HttpResp=PHttpResp(new THttpResp(
       THttp::ErrNotFoundStatusCd, THttp::TextHtmlFldVal, false, BodySIn, ""));
    } else {
      // file successfully opened
      PSIn BodySIn=RqSIn;
      if (THttp::IsHtmlFExt(RqFNm.GetFExt())){
        // send text/html mime type if Html file
        HttpResp=PHttpResp(new THttpResp(
         THttp::OkStatusCd, THttp::TextHtmlFldVal, false, BodySIn, ""));
      } else
      if (THttp::IsGifFExt(RqFNm.GetFExt())){
        // send image/gif mime type if Gif file
        HttpResp=PHttpResp(new THttpResp(
         THttp::OkStatusCd, THttp::ImageGifFldVal, false, BodySIn, ""));
      } else {
        // send application/octet mime type
        HttpResp=PHttpResp(new THttpResp(
         THttp::OkStatusCd, THttp::AppOctetFldVal, false, BodySIn, ""));
      }
    }
  } else {
    // prepare default html with time
    TChA HtmlChA;
    HtmlChA+="<html><title>Tralala Hopsasa</title><body>";
    HtmlChA+=TSecTm::GetCurTm().GetStr();
    HtmlChA+="</body></html>";
    PSIn BodySIn=TMIn::New(HtmlChA);
    HttpResp=THttpResp::New(
     THttp::OkStatusCd, THttp::TextHtmlFldVal, false, BodySIn);
  }

  // construct & send response
  SendHttpResp(SockId, HttpResp);
  // notify
  if (RqUrl->IsOk()){
    TChA MsgChA;
    MsgChA+="Web-Server: Request for '";
    MsgChA+=RqUrl->GetUrlStr(); MsgChA+="'.";
    TNotify::OnNotify(Notify, ntInfo, MsgChA);
  }
}

void TWebSrv::SendHttpResp(const uint64& SockId, const PHttpResp& HttpResp){
  PWebSrvConn Conn;
  if (IsConn(SockId, Conn)){
    if (Conn->GetType()==wsctWaitingToRespond){
      Conn->Send(HttpResp->GetSIn());
      GetConn(SockId)->PutType(wsctSending);
    } else {
      OnError(SockId, -1, "Connection is not ready for http-response");
    }
  }
}



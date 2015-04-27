/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

/////////////////////////////////////////////////
// Forward
class TWebSrv;

/////////////////////////////////////////////////
// Web-Server-Socket-Event
class TWebSrvSockEvent: public TSockEvent{
private:
  TWebSrv* WebSrv;
public:
  TWebSrvSockEvent(TWebSrv* _WebSrv):
    TSockEvent(), WebSrv(_WebSrv){}
  ~TWebSrvSockEvent(){}

  void OnRead(const uint64& SockId, const PSIn& SIn);
  void OnWrite(const uint64& SockId);
  void OnAccept(const uint64& SockId, const PSock& Sock);
  void OnConnect(const uint64& SockId);
  void OnReadEof(const uint64& SockId);
  void OnClose(const uint64& SockId);
  void OnTimeOut(const uint64& SockId);
  void OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr);
  void OnGetHost(const PSockHost& SockHost);
};

/////////////////////////////////////////////////
// Web-Server-Connection
typedef enum {
  wsctUndef, wsctReceiving, wsctWaitingToRespond, wsctSending} TWebSrvConnType;

ClassTP(TWebSrvConn, PWebSrvConn)//{
private:
  TWebSrv* WebSrv;
  TWebSrvConnType Type;
  PSock Sock;
  TChA HttpRqChA;
  UndefDefaultCopyAssign(TWebSrvConn);
public:
  TWebSrvConn(const PSock& _Sock, TWebSrv* _WebSrv):
    WebSrv(_WebSrv), Type(wsctUndef), Sock(_Sock){}
  static PWebSrvConn New(const PSock& Sock, TWebSrv* WebSrv){
    return PWebSrvConn(new TWebSrvConn(Sock, WebSrv));}
  ~TWebSrvConn(){}

  void PutType(const TWebSrvConnType& _Type){Type=_Type;}
  TWebSrvConnType GetType() const {return Type;}

  PSock GetSock() const {return Sock;}
  void Send(const PSIn& SIn) const {Sock->SendSafe(SIn);}

  TChA& GetHttpRqChA(){return HttpRqChA;}

  friend class TWebSrv;
};

/////////////////////////////////////////////////
// Web-Server
ClassTPV(TWebSrv, PWebSrv, TWebSrvV)//{
private:
  PNotify Notify;
  int PortN;
  TStr HomeNrFPath;
  PSockEvent SockEvent;
  PSock Sock;
  THash<TUInt64, PWebSrvConn> SockIdToConnH;
  UndefDefaultCopyAssign(TWebSrv);
private:
  void OnRead(const uint64& SockId, const PSIn& SIn);
  void OnWrite(const uint64& SockId);
  void OnAccept(const uint64& SockId, const PSock& Sock);
  void OnConnect(const uint64&){Notify->OnStatus("OnConnect");}
  void OnReadEof(const uint64& SockId);
  void OnClose(const uint64& SockId);
  void OnTimeOut(const uint64& SockId);
  void OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr);
  void OnGetHost(const PSockHost&){Notify->OnStatus("OnGetHost");}
public:
  TWebSrv(
   const int& _PortN, const bool& FixedPortNP=true, const PNotify& _Notify=NULL);
  static PWebSrv New(
   const int& PortN, const bool& FixedPortNP=true, const PNotify& Notify=NULL){
    return PWebSrv(new TWebSrv(PortN, FixedPortNP, Notify));}
  virtual ~TWebSrv();

  PNotify GetNotify() const {return Notify;}
  int GetPortN() const {return PortN;}
  TStr GetHomeNrFPath() const {return HomeNrFPath;}

  int GetConns() const {return SockIdToConnH.Len();}
  bool IsConn(const uint64& SockId) const {return SockIdToConnH.IsKey(SockId);}
  bool IsConn(const uint64& SockId, PWebSrvConn& Conn) const {
    if (SockIdToConnH.IsKey(SockId)){Conn=GetConn(SockId); return true;}
    else {return false;}}
  void AddConn(const uint64& SockId, const PWebSrvConn& Conn){
    SockIdToConnH.AddDat(SockId, Conn);}
  void DelConn(const uint64& SockId){
    SockIdToConnH.DelIfKey(SockId);}
  PWebSrvConn GetConn(const uint64& SockId) const {
    return SockIdToConnH.GetDat(SockId);}
  TStr GetPeerIpNum(const uint64& SockId) const {
    return GetConn(SockId)->Sock->GetPeerIpNum();}

  virtual void OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq);
  void SendHttpResp(const uint64& SockId, const PHttpResp& HttpResp);

  friend class TWebSrvSockEvent;
};


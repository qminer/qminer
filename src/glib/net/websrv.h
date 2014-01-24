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


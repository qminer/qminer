/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Event-Loop
void TLoop::Run() {
	SockSys.RunLoop();
}

void TLoop::Stop() {
	SockSys.StopLoop();
}

void TLoop::Reset() {
	SockSys.ResetLoop();
}

void TLoop::Ref() {
	SockSys.RefLoop();
}

void TLoop::Unref() {
	SockSys.UnrefLoop();
}

/////////////////////////////////////////////////
// Socket-Event
uint64 TSockEvent::LastSockEventId = 0;

TSockEvent::~TSockEvent() { }

bool TSockEvent::IsReg(const PSockEvent& SockEvent) {
	return SockSys.IsSockEvent(SockEvent);
}

void TSockEvent::Reg(const PSockEvent& SockEvent) {
	IAssert(!SockSys.IsSockEvent(SockEvent));
	SockSys.AddSockEvent(SockEvent);
}

void TSockEvent::UnReg(const PSockEvent& SockEvent) {
	IAssert(SockSys.IsSockEvent(SockEvent));
	SockSys.DelSockEvent(SockEvent);
}

/////////////////////////////////////////////////
// Socket-Host
void TSockHost::AddIpNum(const TStr& IpNum) {
	if (TUInt::IsIpStr(IpNum)) {
		IpNumV.Add(TIpNum(IpNum, shivIpv4));
	} else if (TUInt::IsIpv6Str(IpNum)) {
		IpNumV.Add(TIpNum(IpNum, shivIpv6));
	} else {
		throw TExcept::New("Invalid IP number: " + IpNum);
	}
}

TSockHost::TSockHost(const PSockEvent& SockEvent): 
	Status(shsUndef), SockEventId(SockEvent->GetSockEventId()) { }

bool TSockHost::IsSockEvent() const {
	return SockSys.IsSockEvent(SockEventId);
}

PSockEvent TSockHost::GetSockEvent() const {
	return SockSys.GetSockEvent(SockEventId);
}

void TSockHost::GetAsyncSockHost(const TStr& HostNm, const PSockEvent& SockEvent) {
	PSockHost SockHost = TSockHost::New(SockEvent);
	// check if there is need for DNS resovling or not
	if (HostNm == TSock::LocalhostNm) {
		// no need, just return loopback IP
		SockHost->Status = shsOk;
		SockHost->IpNumV.Add(TIpNum(TSock::LoopbackIpNum, shivIpv4));
		// we do the callback from here already
		SockEvent->OnGetHost(SockHost);
	} else if (TUInt::IsIpStr(HostNm)) {
		// no need, just need to parse out IPv4
		SockHost->Status = shsOk;
		SockHost->IpNumV.Add(TIpNum(HostNm, shivIpv4));
		// we do the callback from here already
		SockEvent->OnGetHost(SockHost);
	} else if (TUInt::IsIpv6Str(HostNm)) {
		// no need, just need to parse out IPv4
		SockHost->Status = shsOk;
		SockHost->IpNumV.Add(TIpNum(HostNm, shivIpv6));
		// we do the callback from here already
		SockEvent->OnGetHost(SockHost);
	} else {
		SockHost->HostNm = HostNm;
		// we need to resolve hostname to IP
		SockSys.GetAsyncSockHost(HostNm, SockHost);
	}
}

/////////////////////////////////////////////////
// Socket
uint64 TSock::LastSockId = 0;
const TStr TSock::LocalhostNm = "localhost";
const TStr TSock::LoopbackIpNum = "127.0.0.1";

TSock::TSock(const PSockEvent& SockEvent): 
		SockId(++LastSockId), SockEventId(SockEvent->GetSockEventId()) {

	// make sure callbacks are registered
	IAssert(TSockEvent::IsReg(SockEvent));
	// initialize socket
	SockSys.NewSock(SockId, SockEventId);
}

TSock::~TSock() {
	// call socket cleanup
	SockSys.DelSock(SockId);
}

PSockEvent TSock::GetSockEvent() const {
  return SockSys.GetSockEvent(SockEventId);
}

void TSock::Listen(const int& PortN, const bool& IPv6P){
	SockSys.Listen(SockId, PortN, IPv6P);
}

//int TSock::GetPortAndListen(const int& MnPortN){
//  int PortN=MnPortN-1;
//  int ErrCd=0;
//  sockaddr_in SockAddr;
//  forever {
//    PortN++;
//    memset(&SockAddr, 0, sizeof(SockAddr));
//    SockAddr.sin_family=AF_INET;
//    SockAddr.sin_addr.s_addr=INADDR_ANY;
//    SockAddr.sin_port=htons(u_short(PortN));
//    int OkCd=bind(SockHnd, (sockaddr*)&SockAddr, sizeof(SockAddr));
//    if (OkCd==SOCKET_ERROR){
//      ErrCd=WSAGetLastError();
//      if (ErrCd!=WSAEADDRINUSE){break;}
//    } else {
//      ErrCd=0; break;
//    }
//  }
//  // get the port number (if it was automatically assigned)
//  memset(&SockAddr, 0, sizeof(SockAddr));
//  int NmLen=sizeof(sockaddr_in);
//  EAssertR(
//   getsockname(SockHnd, (sockaddr*)&SockAddr, &NmLen)==0,
//   TSockSys::GetErrStr(WSAGetLastError()));
//  PortN=ntohs(SockAddr.sin_port); 
//
//  EAssertR(
//   getsockname(SockHnd, (sockaddr*)&SockAddr, &NmLen)==0,
//   TSockSys::GetErrStr(WSAGetLastError()));
//
//  EAssertR(ErrCd==0, TSockSys::GetErrStr(ErrCd));
//  EAssertR(
//   WSAAsyncSelect(SockHnd, TSockSys::GetSockWndHnd(),
//   TSockSys::GetSockMsgHnd(), TSockSys::GetAllSockEventCdSet())==0,
//   TSockSys::GetErrStr(WSAGetLastError()));
//  EAssertR(
//   listen(SockHnd, SOMAXCONN)==0,
//   TSockSys::GetErrStr(WSAGetLastError()));
//  return PortN;
//}

void TSock::Connect(const PSockHost& SockHost, const int& PortN) {	
	SockSys.Connect(SockId, SockHost, PortN);
}

void TSock::Send(const PSIn& SIn) {
	SockSys.Send(SockId, SIn);
}

void TSock::Send(const PSIn& SIn, bool& Ok, TStr& Msg) {
	try {
		Send(SIn);
		Ok = true;
	} catch(PExcept Except) {
		Ok = false;
		Msg = Except->GetMsgStr();
	} catch(...) {
		Ok = false;
		Msg = "Sock.Send: Unknown error";
	}
}

void TSock::SendSafe(const PSIn& SIn) {
	try {
		Send(SIn);
	} catch(...) {
	}
}

TStr TSock::GetPeerIpNum() const {
	return SockSys.GetPeerIpNum(SockId);
}

TStr TSock::GetLocalIpNum() const {
	return SockSys.GetLocalIpNum(SockId);
}

void TSock::PutTimeOut(const int& MSecs){
	SockSys.AddSockTimer(SockId, MSecs);
}

void TSock::DelTimeOut(){
	SockSys.DelIfSockTimer(SockId);
}

bool TSock::IsSockId(const uint64& SockId) {
  return SockSys.IsSock(SockId);
}

/////////////////////////////////////////////////
// Timer

// we attache pointer to this class so we can execute callback on it
typedef struct {
	uv_timer_t TimerHnd;
	TTTimer* Timer;
} uv_timer_req_t;

// declaration of callback, since sock.h is not aware of libuv
void TTTimer_OnTimeOut(uv_timer_t* TimerHnd, int Status) {
	uv_timer_req_t* _TimerHnd = (uv_timer_req_t*)TimerHnd;
	_TimerHnd->Timer->OnTimeOut();
}

TTTimer::TTTimer(const int& _TimeOutMSecs, const bool& _Repeat): TimeOutMSecs(_TimeOutMSecs), 
    Repeat(_Repeat), Ticks(0), StartTm(TSecTm::GetCurTm()) 
{	
	// create new timer
	uv_timer_req_t* _TimerHnd = (uv_timer_req_t*)malloc(sizeof(uv_timer_req_t));
	// initialize
	_TimerHnd->Timer = this;
	uv_timer_init(SockSys.Loop, (uv_timer_t*)_TimerHnd );
	// remember handle
	TimerHnd = (uint64)_TimerHnd;
	// start if needed
	if (TimeOutMSecs > 0) { StartTimer(TimeOutMSecs); }
}

TTTimer::~TTTimer() {
	// stop the timer
	StopTimer();
	// close handle
	uv_close((uv_handle_t*)TimerHnd.Val, NULL);
	// cleanup after it
	uv_timer_req_t* _TimerHnd = (uv_timer_req_t*)TimerHnd.Val;
	free(_TimerHnd);
}

void TTTimer::StartTimer(const int& _TimeOutMSecs) {
	IAssert((_TimeOutMSecs == -1) || (_TimeOutMSecs >= 0));
	// if _TimeOut==-1 use previous TimeOut
	if (_TimeOutMSecs != -1) { TimeOutMSecs = _TimeOutMSecs; }
	// stop current-timer
	StopTimer();
	// start new if non-zero timeout
	if (_TimeOutMSecs > 0){
		uv_timer_t* _TimerHnd = (uv_timer_t*)TimerHnd.Val;
		uv_timer_start(_TimerHnd, TTTimer_OnTimeOut, (uint64)TimeOutMSecs, Repeat ? (uint64) TimeOutMSecs : 0);
	}
}

void TTTimer::StopTimer(){
	// get handle and stop the timer
	uv_timer_t* _TimerHnd = (uv_timer_t*)TimerHnd.Val;
	uv_timer_stop(_TimerHnd);
}

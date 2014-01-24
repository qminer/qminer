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
// Socket-System
class TSockSys {
public:
	// to keep TSockSys singleton
	static bool Active;
	// UV loop
	uv_loop_t* Loop;
	uv_timer_t* LoopRef;
	// traffic count
	TUInt64 SockBytesRead;
	TUInt64 SockBytesWritten;
	// socket-event
	THash<TUInt64, PSockEvent> IdToSockEventH;
	// socket-host
	THash<TUInt64, PSockHost> HndToSockHostH;
	// sockets
	THash<TUInt64, uv_tcp_t*> SockIdToHndH;
	THash<TUInt64, TUInt64> SockHndToIdH;
	THash<TUInt64, TUInt64> SockHndToEventIdH;
	TUInt64Set ClosedSockIdSet;
	// timers
	THash<TUInt64, uv_timer_t*> SockIdToTimerHndH;
	THash<TUInt64, TUInt64> TimerHndToSockIdH;
	
private:
	// we attache buffer information to write request, 
	// so we can after write is completed 
	typedef struct {
		uv_write_t WriteReq;
		uv_buf_t Buffer;
	} uv_write_req_t;

	UndefCopyAssign(TSockSys);

public:
	TSockSys();
	~TSockSys();

	// start event loop;
	void RunLoop() { uv_run(Loop, UV_RUN_DEFAULT); }
	// stop event loop
	void StopLoop() { uv_stop(Loop); }
	// reset loop
	void ResetLoop() { free(Loop); Loop = uv_loop_new(); }
	// increase reference count so it doesn't stop the loop when nothing to od
	void RefLoop();
	// decrease reference count, to stop the loop if nothing else to do
	void UnrefLoop();
	// empty callback used for keeping reference on the loop
	static void LoopRefCallback(uv_timer_t* TimerHnd, int Status) {  }

  // socket event
	bool IsSockEvent(const uint64& SockEventId) const;
	bool IsSockEvent(const PSockEvent& SockEvent) const;
	void AddSockEvent(const PSockEvent& SockEvent);
	void DelSockEvent(const PSockEvent& SockEvent);
	PSockEvent GetSockEvent(const uint64& SockEventId) const;

  // socket host
	// create sock host by resolving HostNm
	void GetAsyncSockHost(const TStr& HostNm, const PSockHost& SockHost);

  // socket
	bool IsSock(const uint64& SockId) const { return SockIdToHndH.IsKey(SockId); }
	bool IsSockHnd(const uint64& SockHnd) const { return SockHndToIdH.IsKey(SockHnd); }
	// initialize new tcp connection
	void NewSock(const uint64& SockId, const uint64& SockEventId);
	// close tcp connection
	void DelSock(const uint64& SockId);
	// listen on a port for connection
	void Listen(const uint64& SockId, const int& PortN, const bool& IPv6P);
	// establish connection
	void Connect(const uint64& SockId, const PSockHost& SockHost, const int& PortN);
	// write to socket
	void Send(const uint64& SockId, const PSIn& SIn);
	// check if timer exists
	bool IsTimer(const uint64& TimerHnd) { return TimerHndToSockIdH.IsKey(TimerHnd); }
	// set timeout on socket
	void AddSockTimer(const uint64& SockId, const int& MSecs);
	// delete timer if set on a socket
	void DelIfSockTimer(const uint64& SockId);
	// get peer IP address
	TStr GetPeerIpNum(const uint64& SockId);
	// get local IP address
	TStr GetLocalIpNum(const uint64& SockId);

  // callbacks for socket and host events
	// called as response to DNS resolving (uv_getaddrinfo)
	static void OnGetHost(uv_getaddrinfo_t* RequestHnd, int Status, struct addrinfo* AddrInfo);
	// called on established connection
	static void OnConnect(uv_connect_t* ConnectHnd, int Status);
	// called on write successful
	static void OnWrite(uv_write_t* WriteHnd, int Status);
	// called before on read to reserve space
	static uv_buf_t OnAlloc(uv_handle_t* SockHnd, size_t SuggestedSize);
	// called on read (if BufferLen == 0 then EOF)
	static void OnRead(uv_stream_t* SockHnd, ssize_t BufferLen, uv_buf_t Buffer);
	// called when new connected accepted by listening socket
	static void OnAccept(uv_stream_t *SockHnd, int Status);
	// called on socket closed
	static void OnClose(uv_handle_t* SockHnd);
	// called on socket timeout
	static void OnTimeOut(uv_timer_t* TimerHnd, int Status);

  // statistics
	// traffic count
	uint64 GetSockBytesRead() { return SockBytesRead; }
	uint64 GetSockBytesWritten() { return SockBytesWritten; }
	// status
	TStr GetLastErr() const;
	TStr GetStatusStr();
};

// socket system initialized
bool TSockSys::Active = false;
TSockSys SockSys; // the only instance of TSockSys

TSockSys::TSockSys() {
	// only one instance alloved
	IAssert(!Active);
	// initialize loop
	Loop = uv_loop_new();
	// done
	Active = true;
}

TSockSys::~TSockSys(){
	// TSockSys cannot be uninitialized, if it is something went wrong
	IAssert(Active);
	// kill all existing connections
	int SockKeyId = SockHndToIdH.FFirstKeyId();
	while (SockHndToIdH.FNextKeyId(SockKeyId)) {
		// check if already closed
		const uint64 SockId = SockHndToIdH[SockKeyId];
		if (ClosedSockIdSet.IsKey(SockId)) { continue; }
		// get handle
		const uint64 SockHnd = SockHndToIdH.GetKey(SockKeyId);
		// close the handle without any callback, since TSockSys is getting killed
		uv_close((uv_handle_t*)SockHnd, NULL);
	}
	// kill all existing timer
	int TimerKeyId = TimerHndToSockIdH.FFirstKeyId();
	while (TimerHndToSockIdH.FNextKeyId(TimerKeyId)) {
		// get timer handle
		const uint64 TimerHnd = TimerHndToSockIdH.GetKey(TimerKeyId);
		// close the handle without any callback, since TSockSys is getting killed
		uv_close((uv_handle_t*)TimerHnd, NULL);
	}
	// delete loop
	free(Loop);
	// mark we are off
	Active = false;
}

void TSockSys::RefLoop() { 
	IAssert(LoopRef == NULL);
	// create new timer
	LoopRef = (uv_timer_t*)malloc(sizeof(uv_timer_t));
	// initialize
	uv_timer_init(Loop, LoopRef);
	// start the timer
	uv_timer_start(LoopRef, LoopRefCallback, 0, 60000);
	// reference the timer
	uv_ref((uv_handle_t*)LoopRef); 
}

void TSockSys::UnrefLoop() { 
	IAssert(LoopRef != NULL);
	// unrefernce the timer
	uv_unref((uv_handle_t*)LoopRef); 
	// kill the timer
	uv_close((uv_handle_t*)LoopRef, NULL);
	LoopRef = NULL;
}

bool TSockSys::IsSockEvent(const uint64& SockEventId) const { 
	return Active ? IdToSockEventH.IsKey(SockEventId) : false;
}

bool TSockSys::IsSockEvent(const PSockEvent& SockEvent) const { 
	return Active ? IdToSockEventH.IsKey(SockEvent->GetSockEventId()) : false;
}

void TSockSys::AddSockEvent(const PSockEvent& SockEvent) {
	// make sure we don't add empty stuff
	EAssert(!SockEvent.Empty());
	IdToSockEventH.AddDat(SockEvent->GetSockEventId(), SockEvent);
}

void TSockSys::DelSockEvent(const PSockEvent& SockEvent) {
	if (Active) {
		IdToSockEventH.DelIfKey(SockEvent->GetSockEventId());
	}
}

PSockEvent TSockSys::GetSockEvent(const uint64& SockEventId) const {
	return IdToSockEventH.GetDat(SockEventId);
}

void TSockSys::GetAsyncSockHost(const TStr& HostNm, const PSockHost& SockHost) {
	// prepare address info request
	uv_getaddrinfo_t* Request = (uv_getaddrinfo_t*)malloc(sizeof(uv_getaddrinfo_t));
	// submit the request
	int ResCd = uv_getaddrinfo(Loop, Request, TSockSys::OnGetHost, HostNm.CStr(), NULL, NULL);
	// check submission went fine
	if (ResCd != 0) {
		// cleanup first
		free(Request);
		// and throw exception
		throw TExcept::New("TSockSys.GetAsyncSockHost: Error requestiong resolve of hostname " + HostNm);
	}
	// remember SockHost for the callback
	TUInt64 RequestHnd = (uint64)Request;
	HndToSockHostH.AddDat(RequestHnd, SockHost);
}

void TSockSys::NewSock(const uint64& SockId, const uint64& SockEventId) {
	uv_tcp_t* SockHnd = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
	int ResCd = uv_tcp_init(Loop, SockHnd);
	// check all went fine
	if (ResCd != 0) {
		// cleanup first
		free(SockHnd);
		// and throw exception
		throw TExcept::New("SockSys.NewSock: Error initializing TCP socket");
	}
	// remember handles
	SockIdToHndH.AddDat(SockId, SockHnd);
	SockHndToIdH.AddDat((uint64)SockHnd, SockId);
	SockHndToEventIdH.AddDat((uint64)SockHnd, SockEventId);
}

void TSockSys::DelSock(const uint64& SockId) {
	if (Active) {
		uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
		// close the handle
		uv_close((uv_handle_t*)SockHnd, TSockSys::OnClose);
		// mark it's under closing so we don't handle it again in the destructor
		ClosedSockIdSet.AddKey(SockId);
	}
}

void TSockSys::Listen(const uint64& SockId, const int& PortN, const bool& IPv6P) {
	// make sure it's a valid socket
	IAssert(IsSock(SockId));
	uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
	// special handling for v4 and v6 when binding
	if (!IPv6P) {
		// get address
		struct sockaddr_in Addr = uv_ip4_addr("0.0.0.0", PortN);
		// bind socket to port
		const int BindResCd = uv_tcp_bind(SockHnd, Addr);
		EAssertR(BindResCd == 0, "SockSys.Listen: Error bidning socket to port: " + SockSys.GetLastErr());
	} else {
		// get address
		struct sockaddr_in6 Addr = uv_ip6_addr("::", PortN);
		// bind socket to port
		const int BindResCd = uv_tcp_bind6(SockHnd, Addr);
		EAssertR(BindResCd == 0, "SockSys.Listen: Error bidning socket to port: " + SockSys.GetLastErr());
	}
	// make sure we have backlog of at least 128
	const int BacklogQueue = (SOMAXCONN < 128) ? 128 : SOMAXCONN;
	// enable callbacks
	const int ListenResCd = uv_listen((uv_stream_t*)SockHnd, BacklogQueue, TSockSys::OnAccept);
	EAssertR(ListenResCd == 0, "SockSys.Listen: Error setting listener on socket: " + SockSys.GetLastErr());
}

void TSockSys::Connect(const uint64& SockId, const PSockHost& SockHost, const int& PortN) {
	// make sure it's a valid socket
	IAssert(IsSock(SockId));
	uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
	// make sure we got a valid socket host
	IAssert(SockHost->IsOk());
	// get connection handle
	uv_connect_t* ConnectHnd = (uv_connect_t*)malloc(sizeof(uv_connect_t));
	// special handling for v4 and v6
	int ResCd = 0;
	if (SockHost->IsIpv4()) {
		// get address
		struct sockaddr_in Addr = uv_ip4_addr(SockHost->GetIpNum().CStr(), PortN);
		// establish connection
		ResCd = uv_tcp_connect(ConnectHnd, SockHnd, Addr, TSockSys::OnConnect);
	} else if (SockHost->IsIpv6()) {
		// get address
		struct sockaddr_in6 Addr = uv_ip6_addr(SockHost->GetIpNum().CStr(), PortN);
		// establish connection
		ResCd = uv_tcp_connect6(ConnectHnd, SockHnd, Addr, TSockSys::OnConnect);
	}
	// check for errors
	if (ResCd != 0) {
		// cleanup first
		free(SockHnd);
		// and throw exception
		throw TExcept::New("SockSys.Connect: Error establishing socket connection: " + SockSys.GetLastErr());
	}
}

void TSockSys::Send(const uint64& SockId, const PSIn& SIn) {
	// make sure it's a valid socket
	IAssert(IsSock(SockId));
	uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
	// create write request
	uv_write_req_t* WriteHnd = (uv_write_req_t*)malloc(sizeof(uv_write_req_t));
	// copy the data in the buffer
	WriteHnd->Buffer.len = SIn->Len(); //TODO: handle cases when SIn doesn't have known Len()
	WriteHnd->Buffer.base = (char*)malloc(WriteHnd->Buffer.len);
	SIn->GetBf(WriteHnd->Buffer.base, WriteHnd->Buffer.len);
	// execute the request
	int ResCd = uv_write((uv_write_t*)WriteHnd, (uv_stream_t*)SockHnd, &WriteHnd->Buffer, 1, OnWrite);
	// check for errors
	if (ResCd != 0) {
		// cleanup first
		free(WriteHnd->Buffer.base);
		free(WriteHnd);
		// and throw exception
		throw TExcept::New("SockSys.Send: Error sending data: " + SockSys.GetLastErr());
	}
}

void TSockSys::AddSockTimer(const uint64& SockId, const int& MSecs) {
	if (SockIdToTimerHndH.IsKey(SockId)) {
		// socket already has timer, stop and start with MSecs
		uv_timer_t* TimerHnd = SockIdToTimerHndH.GetDat(SockId);
		// stop existing count
		uv_timer_stop(TimerHnd);
		// start new one
		uv_timer_start(TimerHnd, OnTimeOut, MSecs, 0);
	} else {
		// create new timer
		uv_timer_t* TimerHnd = (uv_timer_t*)malloc(sizeof(uv_timer_t));
		// initialize
		uv_timer_init(SockSys.Loop, TimerHnd);
		// start the timer
		uv_timer_start(TimerHnd, OnTimeOut, MSecs, 0);
		// remember handle
		SockIdToTimerHndH.AddDat(SockId, TimerHnd);
		TimerHndToSockIdH.AddDat((uint64)TimerHnd, SockId);	
	}
}

void TSockSys::DelIfSockTimer(const uint64& SockId) {
	if (SockIdToTimerHndH.IsKey(SockId)) {
		// get timer handle
		uv_timer_t* TimerHnd = SockIdToTimerHndH.GetDat(SockId);
		// stop the timer
		uv_timer_stop(TimerHnd);
		// remove shortcuts
		SockIdToTimerHndH.DelKey(SockId);
		TimerHndToSockIdH.DelKey((uint64)TimerHnd);
		// remove shortcuts
	}
}

TStr TSockSys::GetPeerIpNum(const uint64& SockId) {
	// make sure it's a valid socket
	IAssert(IsSock(SockId));
	uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
	// get peer IP
	struct sockaddr PeerName;
	int NameLen = sizeof(PeerName);
	const int ResCd = uv_tcp_getpeername(SockHnd, &PeerName, &NameLen);
	EAssertR(ResCd == 0, "SockSys.GetLocalIpNum: " + SockSys.GetLastErr());
	// decode IP
	char PeerIpNum[17];
	if (PeerName.sa_family == AF_INET) {
		uv_ip4_name((sockaddr_in*)&PeerName, PeerIpNum, sizeof(PeerIpNum));
	} else if (PeerName.sa_family == AF_INET6) {
		uv_ip6_name((sockaddr_in6*)&PeerName, PeerIpNum, sizeof(PeerIpNum));
	} else {
		throw TExcept::New("SockSys.GetLocalIpNum: unkown address family");
	}
	// return
	return TStr(PeerIpNum);
}

TStr TSockSys::GetLocalIpNum(const uint64& SockId) {
	// make sure it's a valid socket
	IAssert(IsSock(SockId));
	uv_tcp_t* SockHnd = SockIdToHndH.GetDat(SockId);
	// get peer IP
	struct sockaddr SockName;
	int NameLen = sizeof(SockName);
	const int ResCd = uv_tcp_getsockname(SockHnd, &SockName, &NameLen);
	EAssertR(ResCd == 0, "SockSys.GetLocalIpNum: " + SockSys.GetLastErr());
	// decode IP
	char SockIpNum[64];
	if (SockName.sa_family == AF_INET) {
		uv_ip4_name((sockaddr_in*)&SockName, SockIpNum, sizeof(SockIpNum));
	} else if (SockName.sa_family == AF_INET6) {
		uv_ip6_name((sockaddr_in6*)&SockName, SockIpNum, sizeof(SockIpNum));
	} else {
		throw TExcept::New("SockSys.GetLocalIpNum: unkown address family");
	}
	// return
	return TStr(SockIpNum);
}

void TSockSys::OnGetHost(uv_getaddrinfo_t* RequestHnd, int Status, struct addrinfo* AddrInfo) {
	// get SockHost
	PSockHost SockHost;
	if (SockSys.HndToSockHostH.IsKey((uint64)RequestHnd)) {
		SockHost = SockSys.HndToSockHostH.GetDat((uint64)RequestHnd);
		SockSys.HndToSockHostH.DelKey((uint64)RequestHnd);
	} else {
		free(RequestHnd); uv_freeaddrinfo(AddrInfo);
		SaveToErrLog("SockSys.OnGetHost: unkown RequestId");
		return;
	}
	// get SockEvent
	PSockEvent SockEvent;
	if (SockHost->IsSockEvent()) {
		SockEvent = SockHost->GetSockEvent();
	} else {
		free(RequestHnd); uv_freeaddrinfo(AddrInfo);
		SaveToErrLog("SockSys.OnGetHost: SockHost without SockEvent");
		return;
	}
	// parse results
	if (Status == 0) {
		SockHost->Status = shsOk;
		// temporary buffer for storing IPs
		char _addr[64] = {'\0'}; 
		// iterate over all the resolved IPs
		struct addrinfo* AddrInfoIter = AddrInfo;
		while (AddrInfoIter != NULL) {
			//if (AddrInfoIter->ai_family 
			//AF_INET6
			// get IP as string
			if (AddrInfoIter->ai_family == AF_INET) {
				uv_ip4_name((struct sockaddr_in*)AddrInfoIter->ai_addr, _addr, sizeof(_addr));
			} else if (AddrInfoIter->ai_family == AF_INET6) {
				uv_ip6_name((struct sockaddr_in6*)AddrInfoIter->ai_addr, _addr, sizeof(_addr));
			}
			TStr IpNum(_addr);
			// add to SockHost
			SockHost->AddIpNum(IpNum);
			// go to the next IP on the list
			AddrInfoIter = AddrInfoIter->ai_next;
		}
	} else if (Status == -1) {
		// something went wrong
		SockHost->Status = shsError;
		SockHost->ErrMsg = "SockSys.OnGetHost: " + SockSys.GetLastErr();
	} else {
		// unkown status
		SockHost->Status = shsError;
		SockHost->ErrMsg = TStr::Fmt("SockSys.OnGetHost: unkown status %d", Status);
	}
	// clean up
	free(RequestHnd); uv_freeaddrinfo(AddrInfo);
	// callback
	SockEvent->OnGetHost(SockHost);
}

void TSockSys::OnConnect(uv_connect_t* ConnectHnd, int Status) {
	// get socket handle
	uv_tcp_t* SockHnd = (uv_tcp_t*)ConnectHnd->handle;
	IAssert(SockSys.IsSockHnd((uint64)SockHnd));
	// cleanup
	free(ConnectHnd);
	// get socket id
	const uint64 SockId = SockSys.SockHndToIdH.GetDat((uint64)SockHnd);
	IAssert(SockSys.IsSock(SockId));
	// get socket event
	const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
	PSockEvent SockEvent;
	if (SockSys.IsSockEvent(SockEventId)) {
		SockEvent = SockSys.GetSockEvent(SockEventId);
	} else {
		SaveToErrLog("SockSys.OnConnect: Socket without SockEvent");
		return;
	}
	// execute callback
	if (Status == 0) {
		SockEvent->OnConnect(SockId);
	} else {
		TStr ErrMsg = (Status == -1) ? 
			"SockSys.OnConnect: " + SockSys.GetLastErr() : 
			"SockSys.OnConnect: Error connecting";
		SockEvent->OnError(SockId, Status, ErrMsg);
		return;
	}
	// start listening for incoming data
	int ResCd = uv_read_start((uv_stream_t*)SockHnd, TSockSys::OnAlloc, TSockSys::OnRead);
	// check for errors
	if (ResCd != 0) {
		TStr ErrMsg = (Status == -1) ? 
			"SockSys.OnConnect: " + SockSys.GetLastErr() : 
			"SockSys.OnConnect: Error establishing read callbacks";
		SockEvent->OnError(SockId, ResCd, ErrMsg);
	}
}

void TSockSys::OnWrite(uv_write_t *WriteHnd, int Status) {
	// cast to our write request object to get access to buffer
	uv_write_req_t* _WriteHnd = (uv_write_req_t*)WriteHnd;
	// get socket handle
	uv_tcp_t* SockHnd = (uv_tcp_t*)WriteHnd->handle;
	IAssert(SockSys.IsSockHnd((uint64)SockHnd));
	// free buffer and write handle
	free(_WriteHnd->Buffer.base);
	free(_WriteHnd);
	// get socket id
	const uint64 SockId = SockSys.SockHndToIdH.GetDat((uint64)SockHnd);
	IAssert(SockSys.IsSock(SockId));
	// get socket event
	const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
	PSockEvent SockEvent;
	if (SockSys.IsSockEvent(SockEventId)) {
		SockEvent = SockSys.GetSockEvent(SockEventId);
	} else {
		SaveToErrLog("SockSys.OnWrite: Socket without SockEvent");
		return;
	}
	// execute callback
	if (Status == 0) {
		SockEvent->OnWrite(SockId);
	} else {
		TStr ErrMsg = (Status == -1) ? 
			"SockSys.OnWrite: " + SockSys.GetLastErr() : 
			"SockSys.OnWrite: Error writing to socket";
		SockEvent->OnError(SockId, Status, ErrMsg);
	}
}

uv_buf_t TSockSys::OnAlloc(uv_handle_t* SockHnd, size_t SuggestedSize) {
	// allocate buffer of max size 1MB
	uv_buf_t Buffer;
	Buffer.len = (ULONG)(SuggestedSize > (size_t)TInt::Mega ? (size_t)TInt::Mega : SuggestedSize);
	Buffer.base = new char[Buffer.len];
	return Buffer;
}

void TSockSys::OnRead(uv_stream_t* SockHnd, ssize_t BufferLen, uv_buf_t Buffer) {
	//TODO: check if we need to close _SockHnd
	//uv_tcp_t* _SockHnd = (uv_tcp_t*)SockHnd;
	// get socket handle
	IAssert(SockSys.IsSockHnd((uint64)SockHnd));
	// get socket id
	const uint64 SockId = SockSys.SockHndToIdH.GetDat((uint64)SockHnd);
	IAssert(SockSys.IsSock(SockId));
	// get socket event
	const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
	PSockEvent SockEvent;
	if (SockSys.IsSockEvent(SockEventId)) {
		SockEvent = SockSys.GetSockEvent(SockEventId);
	} else {
		// cleanup
		free(Buffer.base);
		SaveToErrLog("SockSys.OnRead: Socket without SockEvent");
		return;
	}
	// execute callback
	if (BufferLen > 0) {
		// we got data, move the ownership of buffer to TMIn
		PSIn SIn = TMIn::New(Buffer.base, (int)BufferLen, true);
		// send
		SockEvent->OnRead(SockId, SIn);		
	} else {
		uv_err_code Status = uv_last_error(SockSys.Loop).code;
		// no data, might be error or end of stream
		if (Status == UV_EOF) {
			// no more data, close the socket handle
			SockEvent->OnReadEof(SockId);
			SockSys.DelIfSockTimer(SockId);
		} else if (Status == UV_EAGAIN) {
     		// we'll wait
        } else {
			// error
			TStr ErrMsg = "SockSys.OnRead: " + SockSys.GetLastErr();
			SockEvent->OnError(SockId, Status, ErrMsg);
		}
		// cleanup buffer
		free(Buffer.base);
	}
}

void TSockSys::OnAccept(uv_stream_t* ServerSockHnd, int Status) {
	// get socket handle
	uv_tcp_t* SockHnd = (uv_tcp_t*)ServerSockHnd;
	IAssert(SockSys.IsSockHnd((uint64)SockHnd));
	// get socket id
	const uint64 SockId = SockSys.SockHndToIdH.GetDat((uint64)SockHnd);
	IAssert(SockSys.IsSock(SockId));
	// get socket event
	const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
	PSockEvent SockEvent;
	if (SockSys.IsSockEvent(SockEventId)) {
		SockEvent = SockSys.GetSockEvent(SockEventId);
	} else {
		SaveToErrLog("SockSys.OnAccept: Socket without SockEvent");
		return;
	}
	// check for success status
	if (Status == 0) {
		// create a new socket for client connection
		PSock ClientSock = TSock::New(SockEvent);
		// get client socket handle
		uv_tcp_t* ClientSockHnd = SockSys.SockIdToHndH.GetDat(ClientSock->GetSockId());
		// accept request
		const int AcceptResCd = uv_accept(ServerSockHnd, (uv_stream_t*)ClientSockHnd);
		// start listening for incoming data
		if (AcceptResCd == 0) {
			int ReadStartResCd = uv_read_start((uv_stream_t*)ClientSockHnd, TSockSys::OnAlloc, TSockSys::OnRead);
			// execute callback
			if (ReadStartResCd == 0) {
				SockEvent->OnAccept(ClientSock->GetSockId(), ClientSock);
			} else {
				TStr ErrMsg = (Status == -1) ? 
					"SockSys.OnAccept: Error starting read " + SockSys.GetLastErr() : 
					"SockSys.OnAccept: Error starting read";
				SockEvent->OnError(SockId, ReadStartResCd, ErrMsg);
			}
		} else {
			// handle errors
			TStr ErrMsg = (Status == -1) ? 
				"SockSys.OnAccept: Error accepting new connection " + SockSys.GetLastErr() : 
				"SockSys.OnAccept: Error accepting new connection";
			SockEvent->OnError(SockId, AcceptResCd, ErrMsg);
		}
	} else {
		TStr ErrMsg = (Status == -1) ? 
			"SockSys.OnAccept: Error connecting" + SockSys.GetLastErr() : 
			"SockSys.OnAccept: Error connecting";
		SockEvent->OnError(SockId, Status, ErrMsg);
	}
}

void TSockSys::OnClose(uv_handle_t* SockHnd) {
	// check if we know about the socket
	if (SockSys.IsSockHnd((uint64)SockHnd)) {
		// get socket id
		const uint64 SockId = SockSys.SockHndToIdH.GetDat((uint64)SockHnd);
		// execute callback
		const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
		if (SockSys.IsSockEvent(SockEventId)) {
			PSockEvent SockEvent = SockSys.GetSockEvent(SockEventId);
			SockEvent->OnClose(SockId);
		}
		// delete socket shortcuts
		SockSys.DelIfSockTimer(SockId);
		SockSys.SockIdToHndH.DelKey(SockId);
		SockSys.SockHndToIdH.DelKey((uint64)SockHnd);
		SockSys.SockHndToEventIdH.DelKey((uint64)SockHnd);
		// marke note that it's already closed
		SockSys.ClosedSockIdSet.DelIfKey(SockId);
	}
}

void TSockSys::OnTimeOut(uv_timer_t* TimerHnd, int Status) {
	// check we have timer
	IAssert(SockSys.IsTimer((uint64)TimerHnd));
	// get socket id
	const uint64 SockId = SockSys.TimerHndToSockIdH.GetDat((uint64)TimerHnd);
	IAssert(SockSys.IsSock(SockId));
	// get socket event
	const uint64 SockHnd = (uint64)SockSys.SockIdToHndH.GetDat(SockId);
	const uint64 SockEventId = SockSys.SockHndToEventIdH.GetDat((uint64)SockHnd);
	PSockEvent SockEvent;
	if (SockSys.IsSockEvent(SockEventId)) {
		SockEvent = SockSys.GetSockEvent(SockEventId);
		// execute callback
		if (Status == 0) {
			SockEvent->OnTimeOut(SockId);
		} else {
			TStr ErrMsg = (Status == -1) ? 
				"SockSys.OnTimeOut: " + SockSys.GetLastErr() : 
				"SockSys.OnTimeOut: Error in socket timeout";
			SockEvent->OnError(SockId, Status, ErrMsg);
		}
	} else {
		SaveToErrLog("SockSys.OnTimeOut: Socket without SockEvent");
	}
	// cleanup
	uv_close((uv_handle_t*)TimerHnd, NULL);
	// remove shortcuts
	SockSys.SockIdToTimerHndH.DelKey(SockId);
	SockSys.TimerHndToSockIdH.DelKey((uint64)TimerHnd);
}

TStr TSockSys::GetLastErr() const { 
	return TStr(uv_err_name(uv_last_error(Loop)));
}

TStr TSockSys::GetStatusStr(){
  TChA ChA;
  ChA+="Sockets: "; ChA+=TInt::GetStr(SockIdToHndH.Len()); ChA+="\r\n";
  ChA+="Host-Resolutions: "; ChA+=TInt::GetStr(HndToSockHostH.Len()); ChA+="\r\n";
  ChA+="Socket-Events: "; ChA+=TInt::GetStr(IdToSockEventH.Len()); ChA+="\r\n";
  ChA+="Timers: "; ChA+=TInt::GetStr(SockIdToTimerHndH.Len()); ChA+="\r\n";
  return ChA;
}

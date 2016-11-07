/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Forward
ClassHdTP(TSock, PSock);
ClassHdTP(TSockHost, PSockHost);

/////////////////////////////////////////////////
// Event-Loop
class TLoop {
public:
	// start the loop
	static void Run();
	// stop the loop
	static void Stop();
	// reset the loop
	static void Reset();
	// increase reference count so it doesn't stop the loop when nothing to od
	static void Ref();
	// decrease reference count, to stop the loop if nothing else to do
	static void Unref();
};

/////////////////////////////////////////////////
// Socket-Event
//   callbacks for handling socket events
ClassTP(TSockEvent, PSockEvent)//{
private:
	// generator of unique Ids
	static uint64 LastSockEventId;
private:
	// event ID used in SockSys
	TUInt64 SockEventId;

	UndefCopyAssign(TSockEvent);
public:
	TSockEvent(): SockEventId(++LastSockEventId) { }
	virtual ~TSockEvent();

	uint64 GetSockEventId() const { return SockEventId; }

	virtual void CloseConn() { }
	virtual int GetBytesRead() const { return -1; }

	virtual void OnGetHost(const PSockHost& SockHost) = 0; //{ }
	virtual void OnConnect(const uint64& SockId) = 0; //{ }
	virtual void OnRead(const uint64& SockId, const PSIn& SIn) = 0; //{ }
	virtual void OnWrite(const uint64& SockId) = 0; //{ }
	virtual void OnAccept(const uint64& SockId, const PSock& Sock) = 0; //{ }
	virtual void OnReadEof(const uint64& SockId) = 0; //{ }
	virtual void OnClose(const uint64& SockId) = 0; //{ }
	virtual void OnTimeOut(const uint64& SockId) = 0; //{ }
	virtual void OnError(const uint64& SockId, const int& ErrCd, const TStr& ErrStr) = 0; //{ }

	static bool IsReg(const PSockEvent& SockEvent);
	static void Reg(const PSockEvent& SockEvent);
	static void UnReg(const PSockEvent& SockEvent);
};

/////////////////////////////////////////////////
// Socket-Host
typedef enum { shsUndef, shsOk, shsError} TSockHostStatus;
typedef enum { shivIpv4, shivIpv6 } TSockHostIpVersion;

ClassTP(TSockHost, PSockHost)//{
private:
	typedef TKeyDat<TStr, TSockHostIpVersion> TIpNum;
private:
	// current status of 
	TSockHostStatus Status;
	// event to which we connect for DNS resolving
	TUInt64 SockEventId;
	// Host name when available
	TStr HostNm;
	// IP address
	TVec<TIpNum> IpNumV;
	// error message
	TStr ErrMsg;

	// add new IP number
	void AddIpNum(const TStr& IpNum);

	UndefDefaultCopyAssign(TSockHost);
	TSockHost(const PSockEvent& SockEvent);
public:
	static PSockHost New(const PSockEvent& SockEvent) { return new TSockHost(SockEvent); }
	
	bool IsOk() const { return Status==shsOk; }
	int GetStatus() const { return Status; }
	TStr GetErrMsg() const { return ErrMsg; }

	bool IsHostNm() const { return !HostNm.Empty(); }
	TStr GetHostNm() const { return HostNm; }
	int GetIpNums() const { return IpNumV.Len(); }
	TStr GetIpNum(const int& IpNumN = 0) const { return IpNumV[IpNumN].Key; }
	bool IsIpv4(const int& IpNumN = 0) const { return IpNumV[IpNumN].Dat == shivIpv4; }
	bool IsIpv6(const int& IpNumN = 0) const { return IpNumV[IpNumN].Dat == shivIpv6; }
	
	bool IsSockEvent() const;
	PSockEvent GetSockEvent() const;

	static void GetAsyncSockHost(const TStr& HostNm, const PSockEvent& SockEvent);

	friend class TSockSys;
};

/////////////////////////////////////////////////
// Socket
ClassTP(TSock, PSock)//{
private:
	// generator of unique Ids
	static uint64 LastSockId;
private:
	// socket ID used in TSockSys
	TUInt64 SockId;
	// link to callbacks
	TUInt64 SockEventId;

	UndefDefaultCopyAssign(TSock);
	TSock(const PSockEvent& SockEvent);
public:
	static PSock New(const PSockEvent& SockEvent) { return new TSock(SockEvent); }
	~TSock();

	// properties
	uint64 GetSockId() const { return SockId; }
	uint64 GetSockEventId() const { return SockEventId; }
	PSockEvent GetSockEvent() const;

	//int GetPortAndListen(const int& MnPortN);

	void Listen(const int& PortN, const bool& IPv6P = false);
	void Connect(const PSockHost& SockHost, const int& PortN);
	void Send(const PSIn& SIn);
	void Send(const PSIn& SIn, bool& Ok, TStr& Msg);
	void SendSafe(const PSIn& SIn);

	// connection IP numbers
	TStr GetPeerIpNum() const;
	TStr GetLocalIpNum() const;
	
	// set timer on the socket
	void PutTimeOut(const int& MSecs);
	void DelTimeOut();

	// check if socket with such id exists
	static bool IsSockId(const uint64& SockId);

	static const TStr LocalhostNm; // = "localhost"
	static const TStr LoopbackIpNum; // = "127.0.0.1"
};

/////////////////////////////////////////////////
// Timer
//   TODO: not fully tested
ClassTP(TTTimer, PTimer)//{
private:
	// timer handle
	TUInt64 TimerHnd;
	// current timeout setting
	TInt TimeOutMSecs;
	// number of ticks for repeating
	TInt Ticks;
	// timer start
	TSecTm StartTm;
    // repeat calling callback every TimeOutMSecs (if true) or a single call (if false)
    bool Repeat;

	UndefCopyAssign(TTTimer);
public:
	TTTimer(const int& _TimeOutMSecs = 0, const bool& _Repeat = false);
	virtual ~TTTimer();

	int GetTimeOut() const { return TimeOutMSecs; }

	bool IsStarted() const { return TimerHnd != 0; } 
	void StartTimer(const int& _TimeOutMSecs = -1);
	void StopTimer();
	void IncTicks() { Ticks++; }
	int GetTicks() const { return Ticks; }
	int GetSecs() const { return TSecTm::GetDSecs(StartTm, TSecTm::GetCurTm()); }

	virtual void OnTimeOut() { }
};

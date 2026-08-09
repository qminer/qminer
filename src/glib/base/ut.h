/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "bd.h"

/////////////////////////////////////////////////
// Type-Name

// required to demangle class names
#ifdef GLib_GCC
    #include <cxxabi.h>
#endif

template <class Type>
class TTypeNm: public TStr{
public:
  static TStr GetNrTypeNm(const TStr& TypeNm){
    #ifdef GLib_GCC
      // GCC requires some additional cleaning of object names
      int DemangleStatus = 0;
      char* DemangleTypeCStr = abi::__cxa_demangle(TypeNm.CStr(), 0, 0, &DemangleStatus);
      TStr DemangleTypeStr(DemangleTypeCStr);
      free(DemangleTypeCStr);
      return DemangleTypeStr;
    #else
      if (TypeNm.StartsWith("class ")){ return TypeNm.GetSubStr(6, TypeNm.Len()-1);}
      else {return TypeNm;}
    #endif
  }
public:
  TTypeNm(): TStr(GetNrTypeNm((char*)(typeid(Type).name()))){}
};

template <class Type>
TStr GetTypeNm(const Type& Var){
  TStr TypeNm = TStr(typeid(Var).name());
  return TTypeNm<Type>::GetNrTypeNm(TypeNm);
}

/////////////////////////////////////////////////
// Notifications
inline void InfoNotify(const TStr& MsgStr){InfoNotify(MsgStr.CStr());}
inline void WarnNotify(const TStr& MsgStr){WarnNotify(MsgStr.CStr());}
inline void ErrNotify(const TStr& MsgStr){ErrNotify(MsgStr.CStr());}
inline void StatNotify(const TStr& MsgStr){StatNotify(MsgStr.CStr());}

typedef enum TNotifyType_ {ntInfo, ntWarn, ntErr, ntStat} TNotifyType;

ClassTP(TNotify, PNotify)//{
private:
  TNotify(const TNotify&);
  TNotify& operator=(const TNotify&);
public:
  TNotify(){}
  virtual ~TNotify(){}

  virtual void OnNotify(const TNotifyType& /*Type*/, const TStr& /*MsgStr*/){}
  virtual void OnStatus(const TStr& /*MsgStr*/){}
  virtual void OnLn(const TStr& /*MsgStr*/){}
  virtual void OnTxt(const TStr& /*MsgStr*/){}

  // shortcuts for easier formationg
  void OnNotifyFmt(const TNotifyType& Type, const char *FmtStr, ...);
  void OnStatusFmt(const char *FmtStr, ...);
  void OnLnFmt(const char *FmtStr, ...);
  void OnTxtFmt(const char *FmtStr, ...);

  static TStr GetTypeStr(
   const TNotifyType& Type, const bool& Brief=true);
  static void OnNotify(const PNotify& Notify,
   const TNotifyType& Type, const TStr& MsgStr){
    if (!Notify.Empty()){Notify->OnNotify(Type, MsgStr);}}
  static void OnStatus(const PNotify& Notify, const TStr& MsgStr){
    if (!Notify.Empty()){Notify->OnStatus(MsgStr);}}
  static void OnLn(const PNotify& Notify, const TStr& MsgStr){
    if (!Notify.Empty()){Notify->OnLn(MsgStr);}}
  static void OnTxt(const PNotify& Notify, const TStr& MsgStr){
    if (!Notify.Empty()){Notify->OnTxt(MsgStr);}}
  static void DfOnNotify(const TNotifyType& Type, const TStr& MsgStr);

  static const PNotify NullNotify;
  static PNotify StdNotify;
  static PNotify StdErrNotify;
};

/////////////////////////////////////////////////
// Null-Notifier
class TNullNotify: public TNotify{
public:
  TNullNotify(){}
  static PNotify New(){return PNotify(new TNullNotify());}

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr){}
  void OnStatus(const TStr& MsgStr){}
};

/////////////////////////////////////////////////
// Callback-Notifier
typedef void (__stdcall *TCallbackF)(const TNotifyType& Type, const TStr& MsgStr);
class TCallbackNotify : public TNotify
{
private:
  TCallbackF CallbackF;
public:
  TCallbackNotify(const TCallbackF& _CallbackF) : CallbackF(_CallbackF) {}
  static PNotify New(const TCallbackF& CallbackF) { return PNotify(new TCallbackNotify(CallbackF)); }

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr)
  {
    Assert(CallbackF != NULL);
    CallbackF(Type, MsgStr);
  }
  void OnStatus(const TStr& MsgStr)
  {
    Assert(CallbackF != NULL);
    CallbackF(ntStat, MsgStr);
  }
};

/////////////////////////////////////////////////
// Native-Callback-Notifier
typedef void (__stdcall *TNativeCallbackF)(int Type, const char* MsgStr);
class TNativeCallbackNotify : public TNotify
{
private:
  TNativeCallbackF CallbackF;
public:
  TNativeCallbackNotify(const TNativeCallbackF& _CallbackF) : CallbackF(_CallbackF) {}
  static PNotify New(const TNativeCallbackF& CallbackF) { return PNotify(new TNativeCallbackNotify(CallbackF)); }

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr)
  {
    Assert(CallbackF != NULL);
    CallbackF((int)Type, MsgStr.CStr());
  }
  void OnStatus(const TStr& MsgStr)
  {
    Assert(CallbackF != NULL);
    CallbackF((int)ntStat, MsgStr.CStr());
  }
};

/////////////////////////////////////////////////
// Standard-Notifier
class TStdNotify: public TNotify{
public:
  bool AddTimeStamp;
  TStdNotify(const bool& _AddTimeStamp = false) {
    AddTimeStamp = _AddTimeStamp;
  }
  static PNotify New(const bool& AddTimeStamp = false){
    return PNotify(new TStdNotify(AddTimeStamp));
  }

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr);
  void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// Standard-Error-Notifier
class TStdErrNotify: public TNotify{
public:
  bool AddTimeStamp;
  TStdErrNotify(const bool& _AddTimeStamp = false) {
    AddTimeStamp = _AddTimeStamp;
  }
  static PNotify New(const bool& AddTimeStamp = false){
    return PNotify(new TStdErrNotify(AddTimeStamp));
  }

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr);
  void OnStatus(const TStr& MsgStr);
};

//////////////////////////////////////
// Log-Notify
class TLogNotify : public TNotify {
private:
  PNotify Notify;

public:
  TLogNotify(const PNotify& _Notify): Notify(_Notify) { }
  static PNotify New(const PNotify& Notify) { return new TLogNotify(Notify); }

  void OnStatus(const TStr& MsgStr);
};

//////////////////////////////////////
// Color-Notify
enum class TColorNotifyType {
  Default, DefaultBold,
  Red, BoldRed,
  Green, BoldGreen,
  Yellow, BoldYellow,
  Blue, BoldBlue,
  Magenta, BoldMagenta,
  Cyan, BoldCyan
};

class TColorNotify : public TNotify {
private:
  PNotify Notify;
  TColorNotifyType Type;

public:
  TColorNotify(const PNotify& _Notify, const TColorNotifyType& _Type): Notify(_Notify), Type(_Type) { }
  static PNotify New(const PNotify& Notify, const TColorNotifyType& Type) {
    return new TColorNotify(Notify, Type); }

  void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// String-Notifier
class TStrNotify : public TNotify {
public:
  TChA Log;
  TStrNotify(){}
  static PNotify New(){ return new TStrNotify(); }

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr);
  void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// Exception
ClassTP(TExcept, PExcept)//{
private:
  TStr MsgStr;
  TStr LocStr;
  TInt ErrorCode;
  UndefDefaultCopyAssign(TExcept);
public:
  TExcept(const TStr& _MsgStr): MsgStr(_MsgStr), LocStr(){}
  TExcept(const TStr& _MsgStr, const TStr& _LocStr): MsgStr(_MsgStr), LocStr(_LocStr){}
  TExcept(const int& _ErrorCode, const TStr& _MsgStr, const TStr& _LocStr) :
    MsgStr(_MsgStr), LocStr(_LocStr), ErrorCode(_ErrorCode) {}
  static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr());
  static PExcept New(const int& ErrorCode, const TStr& MsgStr, const TStr& LocStr = TStr());
  virtual ~TExcept(){}

  TStr GetMsgStr() const {return MsgStr;}
  TStr GetLocStr() const {return LocStr;}
  TStr GetStr() const {
    if (LocStr.Empty()){return GetMsgStr();}
    else {return GetLocStr()+": "+GetMsgStr();}}
  int GetErrorCode() { return ErrorCode; }

  // replacement exception handler
  typedef void (*TOnExceptF)(const TStr& MsgStr);
  static TOnExceptF OnExceptF;
  static bool IsOnExceptF(){return OnExceptF!=NULL;}
  static void PutOnExceptF(TOnExceptF _OnExceptF){OnExceptF=_OnExceptF;}
  static TOnExceptF GetOnExceptF(){return OnExceptF;}

  // throwing exception
  static void Throw(const TStr& MsgStr){
    if (IsOnExceptF()){(*OnExceptF)(MsgStr);}
    else {throw TExcept::New(MsgStr);}}
  static void Throw(const TStr& MsgStr, const TStr& ArgStr){
    TStr FullMsgStr=MsgStr+" ("+ArgStr+")";
    if (IsOnExceptF()){(*OnExceptF)(FullMsgStr);}
    else {throw TExcept::New(FullMsgStr);}}
  static void Throw(const TStr& MsgStr, const TStr& ArgStr1, const TStr& ArgStr2){
    TStr FullMsgStr=MsgStr+" ("+ArgStr1+", "+ArgStr2+")";
    if (IsOnExceptF()){(*OnExceptF)(FullMsgStr);}
    else {throw TExcept::New(FullMsgStr);}}
  static void ThrowFull(const TStr& MsgStr, const TStr& LocStr){
    if (IsOnExceptF()){(*OnExceptF)(MsgStr);}
    else {throw TExcept::New(MsgStr, LocStr);}}
  static void ThrowFull(const int& ErrorCode, const TStr& MsgStr, const TStr& LocStr) {
      if (IsOnExceptF()) { (*OnExceptF)(MsgStr); }
      else { throw TExcept::New(ErrorCode, MsgStr, LocStr); }
  }
};

// Needed for SNAP examples (otherwise please avoid using these)
#define Try try {
#define Catch } catch (PExcept Except){ErrNotify(Except->GetMsgStr());}
#define CatchFull } catch (PExcept Except){ErrNotify(Except->GetStr());}
#define CatchAll } catch (...){}

#ifdef GLib_WIN
#include <StackWalker.h>
/////////////////////////////////////////////////
// Stack-trace output for Windows
class TFileStackWalker : public StackWalker {
protected:
    FILE* FOut;

    void OnOutput(LPCSTR szText);
public:
    TFileStackWalker();
    void CloseOutputFile();
    ~TFileStackWalker();

    static void WriteStackTrace();
};

class TBufferStackWalker : public StackWalker {
protected:
    TChA Output;

    void OnOutput(LPCSTR szText);
public:
  TBufferStackWalker();
  TChA GetOutput();
  void ClearOutput() { Output = ""; }

  // static method that generates stack trace and returns it
  static TChA GetStackTrace();
};

extern TBufferStackWalker GlobalStackWalker;

#endif

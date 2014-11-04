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

#include "bd.h"


#ifdef GLib_WIN
#include <StackWalker.h>

class TFileStackWalker : public StackWalker
{
public:
	TFileStackWalker() : StackWalker()
	{
		int MxFNmLen = 1000;
		char* FNm = new char[MxFNmLen]; if (FNm == NULL) { return; }
		int FNmLen = GetModuleFileName(NULL, FNm, MxFNmLen); if (FNmLen == 0) { return; }
		TStr FileName = TStr(FNm);
		delete[] FNm;

		FileName += ".ErrTrace";
		FOut = fopen(FileName.CStr(), "a+b");

		time_t Time = time(NULL);
		fprintf(FOut, "\r\n--------\r\n%s --------\r\n", ctime(&Time));
	}

	void CloseOutputFile() {
		if (FOut != NULL)
			fclose(FOut);
		FOut = NULL;
	}

	~TFileStackWalker() {
		CloseOutputFile();
	}

	static void WriteStackTrace() {
		TFileStackWalker Walker;
		Walker.ShowCallstack();
		Walker.CloseOutputFile();
	}

protected:
	/*TStr FileName;*/
	FILE* FOut;

	virtual void OnOutput(LPCSTR szText)
	{
		//printf(szText); StackWalker::OnOutput(szText); 
		if (FOut == NULL) { return; }

		// LPCSTR can be a char or a wchar, depending on the compiler character settings
		// use the appropriate strcopy method to copy to a string buffer
		if (sizeof(TCHAR) == sizeof(char)) {
			fputs((char*) szText, FOut);
		}
		else {
			fputws((wchar_t*) szText, FOut);
		}
	}
};

class TBufferStackWalker : public StackWalker
{
public:
	TBufferStackWalker() : StackWalker() {  }

	TChA GetOutput() { return Output; }

	// static method that generates stack trace and returns it
	static TChA GetStackTrace() {
		TBufferStackWalker Walker;
		Walker.ShowCallstack();
		return Walker.GetOutput();
	}

protected:
	TChA Output;

	virtual void OnOutput(LPCSTR szText)
	{
		// LPCSTR can be a char or a wchar, depending on the compiler character settings
		// use the appropriate strcopy method to copy to a string buffer
		TStr Text;
		if (sizeof(TCHAR) == sizeof(char)) {
			size_t size = strlen(szText);
			char * pCopy = new char[size + 1];
			strcpy(pCopy, szText);
			Text = szText;
			delete pCopy;
			
		}
		else {
			size_t size = wcstombs(NULL, (wchar_t*) szText, 0);
			char * pCopy = new char[size + 1];
			wcstombs(pCopy, (wchar_t*) szText, size + 1);
			Text = pCopy;
			delete pCopy;
		}
		// ignore highest stack items that consist of stack walker and TExcept
		if (Text.SearchStr("StackWalker::") == -1 && Text.SearchStr("TExcept::") == -1)
			Output += Text;
	}
};
#endif

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
      if (TypeNm.IsPrefix("class ")){ return TypeNm.GetSubStr(6, TypeNm.Len()-1);}
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

typedef enum {ntInfo, ntWarn, ntErr, ntStat} TNotifyType;

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
  static const PNotify StdNotify;
  static const PNotify StdErrNotify;
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

#ifndef SWIG
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
#endif

/////////////////////////////////////////////////
// Standard-Notifier
class TStdNotify: public TNotify{
public:
  TStdNotify(){}
  static PNotify New(){return PNotify(new TStdNotify());}

  void OnNotify(const TNotifyType& Type, const TStr& MsgStr);
  void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// Standard-Error-Notifier
class TStdErrNotify: public TNotify{
public:
  TStdErrNotify(){}
  static PNotify New(){return PNotify(new TStdErrNotify());}

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

/////////////////////////////////////////////////
// File-Notifier
class TFileNotify: public TNotify{
private:
	TStr FileName;
	PSOut File;
	bool AddTimeStamp;
	
	bool SeparateFilesForEachDay;
	TStr LastLogDate;
public:
  TFileNotify(const TStr& _FileName, bool _AddTimeStamp = true, bool _SeparateFilesForEachDay = false);
  static PNotify New(const TStr& FileName, bool AddTimeStamp = true, bool SeparateFilesForEachDay = false)	{
	  return PNotify(new TFileNotify(FileName, AddTimeStamp, SeparateFilesForEachDay));
  }
  void OpenNewFileForDate();
  void OnNotify(const TNotifyType& Type, const TStr& MsgStr);
  void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// String-Notifier
class TStrNotify : public TNotify{
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
  UndefDefaultCopyAssign(TExcept);
public:
  TExcept(const TStr& _MsgStr): MsgStr(_MsgStr), LocStr(){}
  TExcept(const TStr& _MsgStr, const TStr& _LocStr): MsgStr(_MsgStr), LocStr(_LocStr){}
  static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr()) {
	  TChA Stack = LocStr;
	  
#ifdef GLib_WIN
	  if (Stack.Len() > 0)
		  Stack += "\n";
	  Stack += "Stack trace:\n";
	  Stack += TBufferStackWalker::GetStackTrace();
#endif
	  
	  return PExcept(new TExcept(MsgStr, Stack));
  }
  virtual ~TExcept(){}

  TStr GetMsgStr() const {return MsgStr;}
  TStr GetLocStr() const {return LocStr;}
  TStr GetStr() const {
    if (LocStr.Empty()){return GetMsgStr();}
    else {return GetLocStr()+": "+GetMsgStr();}}

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
};

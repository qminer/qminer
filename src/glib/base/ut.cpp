/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

////////////////////////////////////////////////
// Notifications
void TNotify::OnNotifyFmt(const TNotifyType& Type, const char *FmtStr, ...) {
  char Bf[10*1024];
  va_list valist;
  va_start(valist, FmtStr);
  const int RetVal=vsnprintf(Bf, 10*1024-2, FmtStr, valist);
  va_end(valist);
  if (RetVal!=-1) { OnNotify(Type, TStr(Bf)); }
}

void TNotify::OnStatusFmt(const char *FmtStr, ...) {
  char Bf[10*1024];
  va_list valist;
  va_start(valist, FmtStr);
  const int RetVal=vsnprintf(Bf, 10*1024-2, FmtStr, valist);
  va_end(valist);
  if (RetVal!=-1) { OnStatus(TStr(Bf)); }
}

void TNotify::OnLnFmt(const char *FmtStr, ...) {
  char Bf[10*1024];
  va_list valist;
  va_start(valist, FmtStr);
  const int RetVal=vsnprintf(Bf, 10*1024-2, FmtStr, valist);
  va_end(valist);
  if (RetVal!=-1) { OnLn(TStr(Bf)); }
}

void TNotify::OnTxtFmt(const char *FmtStr, ...) {
  char Bf[10*1024];
  va_list valist;
  va_start(valist, FmtStr);
  const int RetVal=vsnprintf(Bf, 10*1024-2, FmtStr, valist);
  va_end(valist);
  if (RetVal!=-1) { OnTxt(TStr(Bf)); }
}

TStr TNotify::GetTypeStr(
 const TNotifyType& Type, const bool& Brief){
  static TStr InfoSStr="I"; static TStr InfoLStr="Information";
  static TStr WarnSStr="W"; static TStr WarnLStr="Warning";
  static TStr ErrSStr="E"; static TStr ErrLStr="Error";
  static TStr StatSStr=""; static TStr StatLStr="Status";
  switch (Type){
    case ntInfo: if (Brief){return InfoSStr;} else {return InfoLStr;}
    case ntWarn: if (Brief){return WarnSStr;} else {return WarnLStr;}
    case ntErr: if (Brief){return ErrSStr;} else {return ErrLStr;}
    case ntStat: if (Brief){return StatSStr;} else {return StatLStr;}
    default: Fail; return TStr();
  }
}

void TNotify::DfOnNotify(const TNotifyType& Type, const TStr& MsgStr){
  switch (Type){
    case ntInfo: InfoNotify(MsgStr); break;
    case ntWarn: WarnNotify(MsgStr); break;
    case ntErr: ErrNotify(MsgStr); break;
    case ntStat: StatNotify(MsgStr); break;
    default: Fail;
  }
}

const PNotify TNotify::NullNotify=TNullNotify::New();
PNotify TNotify::StdNotify=TStdNotify::New();
PNotify TNotify::StdErrNotify=TStdErrNotify::New();

/////////////////////////////////////////////////
// Standard-Notifier
void TStdNotify::OnNotify(const TNotifyType& Type, const TStr& MsgStr){
  if (AddTimeStamp) {
    TTm NowTm = TTm::GetCurLocTm();
	printf("[%s] ", NowTm.GetHMSTColonDotStr(true, false).CStr());
  }
  if (Type==ntInfo){
    printf("%s\n", MsgStr.CStr());
  } else {
    TStr TypeStr=TNotify::GetTypeStr(Type, false);
    printf("%s: %s\n", TypeStr.CStr(), MsgStr.CStr());
  }
}

void TStdNotify::OnStatus(const TStr& MsgStr){
  if (AddTimeStamp) {
    TTm NowTm = TTm::GetCurLocTm();
	printf("[%s] ", NowTm.GetHMSTColonDotStr(true, false).CStr());
  }
  printf("%s", MsgStr.CStr());
  // print '\n' if message not overlayed
  if ((!MsgStr.Empty())&&(MsgStr.LastCh()!='\r')){
    printf("\n");}
}

/////////////////////////////////////////////////
// Standard-Error-Notifier
void TStdErrNotify::OnNotify(const TNotifyType& Type, const TStr& MsgStr){
  if (AddTimeStamp) {
    TTm NowTm = TTm::GetCurLocTm();
	fprintf(stderr, "[%s] ", NowTm.GetHMSTColonDotStr(true, false).CStr());
  }
  if (Type==ntInfo){
    fprintf(stderr, "%s\n", MsgStr.CStr());
  } else {
    TStr TypeStr=TNotify::GetTypeStr(Type, false);
    fprintf(stderr, "%s: %s\n", TypeStr.CStr(), MsgStr.CStr());
  }
}

void TStdErrNotify::OnStatus(const TStr& MsgStr){
  if (AddTimeStamp) {
    TTm NowTm = TTm::GetCurLocTm();
	fprintf(stderr, "[%s] ", NowTm.GetHMSTColonDotStr(true, false).CStr());
  }
  fprintf(stderr, "%s", MsgStr.CStr());
  // print '\n' if message not overlayed
  if ((!MsgStr.Empty())&&(MsgStr.LastCh()!='\r')){
     fprintf(stderr, "\n");}
}

//////////////////////////////////////
// Log-Notify
void TLogNotify::OnStatus(const TStr& MsgStr) {
	TTm NowTm = TTm::GetCurLocTm();
	Notify->OnStatus(TStr::Fmt("[%s %s] %s",
		NowTm.GetYMDDashStr().CStr(),
		NowTm.GetHMSTColonDotStr(true, false).CStr(),
		MsgStr.CStr()));
}

//////////////////////////////////////
// Color-Notify
// CODE	   COLOR
// [0;31m	 Red
// [1;31m	 Bold Red
// [0;32m	 Green
// [1;32m	 Bold Green
// [0;33m	 Yellow
// [01;33m	Bold Yellow
// [0;34m	 Blue
// [1;34m	 Bold Blue
// [0;35m	 Magenta
// [1;35m	 Bold Magenta
// [0;36m	 Cyan
// [1;36m	 Bold Cyan
// [0m	   Reset

void TColorNotify::OnStatus(const TStr& MsgStr) {
  switch (Type) {
    case TColorNotifyType::Default:
      Notify->OnStatus(MsgStr);
      break;
    case TColorNotifyType::DefaultBold:
      Notify->OnStatusFmt("\x1B[1m%s\033[0m", MsgStr.CStr());
      break;
    case TColorNotifyType::Red:
      Notify->OnStatusFmt("\x1B[0;31m%s\033[0m", MsgStr.CStr());
      break;
    default:
      Notify->OnStatus(MsgStr);
  }
}

//////////////////////////////////////
// Str-Notify
void TStrNotify::OnNotify(const TNotifyType& Type, const TStr& MsgStr)
{
	if (Type == ntInfo) {
		Log += MsgStr + "\n";
	}
	else {
		TStr TypeStr = TNotify::GetTypeStr(Type, false);
		Log += TypeStr + " " + MsgStr + "\n";
	}
}

void TStrNotify::OnStatus(const TStr& MsgStr)
{
	Log += MsgStr;
	// print '\n' if message not overlayed
	if ((!MsgStr.Empty()) && (MsgStr.LastCh() != '\r'))
		Log += "\n";
}

/////////////////////////////////////////////////
// Exception
TExcept::TOnExceptF TExcept::OnExceptF=NULL;

PExcept TExcept::New(const TStr& MsgStr, const TStr& LocStr) {
	TChA Stack = LocStr;

#ifdef GLib_WIN
	if (Stack.Len() > 0) { Stack += "\n"; }
	Stack += "Stack trace:\n";
	Stack += TBufferStackWalker::GetStackTrace();
#endif

	return PExcept(new TExcept(MsgStr, Stack));
}

PExcept TExcept::New(const int& ErrorCode, const TStr& MsgStr, const TStr& LocStr) {
	TChA Stack = LocStr;

#ifdef GLib_WIN
	if (Stack.Len() > 0) { Stack += "\n"; }
	Stack += "Stack trace:\n";
	Stack += TBufferStackWalker::GetStackTrace();
#endif

	return PExcept(new TExcept(ErrorCode, MsgStr, Stack));
}

#ifdef GLib_WIN
/////////////////////////////////////////////////
// Stack-trace output for Windows
void TFileStackWalker::OnOutput(LPCSTR szText) {
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

TFileStackWalker::TFileStackWalker() : StackWalker() {
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

void TFileStackWalker::CloseOutputFile() {
    if (FOut != NULL)
        fclose(FOut);
    FOut = NULL;
}

TFileStackWalker::~TFileStackWalker() {
    CloseOutputFile();
}

void TFileStackWalker::WriteStackTrace() {
    TFileStackWalker Walker;
    Walker.ShowCallstack();
    Walker.CloseOutputFile();
}

void TBufferStackWalker::OnOutput(LPCSTR szText) {
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

TBufferStackWalker::TBufferStackWalker() : StackWalker() {  }

TChA TBufferStackWalker::GetOutput() { return Output; }

// we create a global instance of the stalk walker. The first time it will be used
// it will load the modules. This is a slow process so we want to call it only once.
TBufferStackWalker GlobalStackWalker;

// static method that generates stack trace and returns it
TChA TBufferStackWalker::GetStackTrace() {
	GlobalStackWalker.ClearOutput();
	GlobalStackWalker.ShowCallstack();
    return GlobalStackWalker.GetOutput();
}
#endif

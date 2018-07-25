/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#if defined(SW_TRACE)
#include <execinfo.h>
#include <signal.h>
#endif

/////////////////////////////////////////////////
// Messages
void WrNotify(const char* CaptionCStr, const char* NotifyCStr){
#if defined(__CONSOLE__) || defined(_CONSOLE)
  printf("*** %s: %s\n", CaptionCStr, NotifyCStr);
#else
  MessageBox(NULL, NotifyCStr, CaptionCStr, MB_OK);
#endif
}

void SaveToErrLog(const char* MsgCStr){
  int MxFNmLen=1000;
  char* FNm=new char[MxFNmLen]; if (FNm==NULL){return;}
  int FNmLen=GetModuleFileName(NULL, FNm, MxFNmLen); if (FNmLen==0){return;}
  FNm[FNmLen++]='.'; FNm[FNmLen++]='E'; FNm[FNmLen++]='r'; FNm[FNmLen++]='r';
  FNm[FNmLen++]=char(0);
  time_t Time=time(NULL);
  FILE* fOut=fopen(FNm, "a+b"); if (fOut==NULL){return;}
  fprintf(fOut, "--------\r\n%s\r\n%s%s\r\n--------\r\n",
   FNm, ctime(&Time), MsgCStr);
  fclose(fOut);
  delete[] FNm;
}

#if defined(SW_TRACE)
void PrintBacktrace() {
  // stack dump, works for g++
  void *array[20];
  size_t size;

  // flush stdout
  fflush(0);

	// get the trace and print it to stdout
  size = backtrace(array, sizeof(array)/sizeof(array[0]));
  backtrace_symbols_fd(array, size, STDERR_FILENO);
}

void signalHandler(int sig) {
  fprintf(stderr, "Error: signal %d\n", sig);
  PrintBacktrace();
  abort();
}

void terminateHandler()
{
  std::exception_ptr exptr = std::current_exception();
  if (exptr != 0) {
    // the only useful feature of exception_ptr is that it can be rethrown...
    try {
      std::rethrow_exception(exptr);
    }
    catch (std::exception &ex) {
      fprintf(stderr, "Terminated due to exception: %s\n", ex.what());
    }
    catch (...) {
      fprintf(stderr, "Terminated due to unknown exception\n");
    }
  }
  else {
    fprintf(stderr, "Terminated due to unknown reason\n");
  }
  PrintBacktrace();
  abort();
}
void Crash() {
  char *p;
  p = (char *) 0;
  *p = 123;
}
#endif

/////////////////////////////////////////////////
// Assertions
TOnExeStop::TOnExeStopF TOnExeStop::OnExeStopF=NULL;

void ExeStop(
 const char* MsgCStr, const char* ReasonCStr,
 const char* CondCStr, const char* FNm, const int& LnN){
  char ReasonMsgCStr[1000];

#if defined(SW_TRACE)
  PrintBacktrace();
  //Crash();
#endif
#ifdef GLib_WIN
  TFileStackWalker::WriteStackTrace();
#endif

  // construct reason message
  if (ReasonCStr==NULL){ReasonMsgCStr[0]=0;}
  else {sprintf(ReasonMsgCStr, " [Reason:'%s']", ReasonCStr);}
  // construct full message
  char FullMsgCStr[1000];
  if (MsgCStr==NULL){
    if (CondCStr==NULL){
      sprintf(FullMsgCStr, "Execution stopped%s!", ReasonMsgCStr);
    } else {
      sprintf(FullMsgCStr, "Execution stopped: %s%s, file %s, line %d",
       CondCStr, ReasonMsgCStr, FNm, LnN);
    }
  } else {
    if (CondCStr==NULL){
      sprintf(FullMsgCStr, "%s\nExecution stopped!", MsgCStr);
    } else {
      sprintf(FullMsgCStr, "Message: %s%s\nExecution stopped: %s, file %s, line %d",
       MsgCStr, ReasonMsgCStr, CondCStr, FNm, LnN);
    }
  }
  // report full message to log file
  SaveToErrLog(FullMsgCStr);

#if defined(SW_NOABORT)
  throw TExcept::New(FullMsgCStr);
#endif

  // report to screen & stop execution
  bool Continue=false;
  // call handler
  if (TOnExeStop::IsOnExeStopF()){
    Continue=!((*TOnExeStop::GetOnExeStopF())(FullMsgCStr));}
  if (!Continue){
    ErrNotify(FullMsgCStr);
#ifdef GLib_WIN
    abort();
#else
    abort();
#endif
  }
}

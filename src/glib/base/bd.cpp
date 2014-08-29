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

#ifndef NDEBUG
#define SW_TRACE
#endif

#if defined(SW_TRACE) && defined(GLib_UNIX)
#include <execinfo.h>
#endif


/////////////////////////////////////////////////
// Mathmatical-Errors
#if defined(__BCPLUSPLUS__) && (__BCPLUSPLUS__==0x0530)
int std::_matherr(struct math_exception* e){
  e->retval=0;
  return 1;
}
#elif defined(GLib_GLIBC) || defined(GLib_BSD)
int _matherr(struct __exception* e){
  e->retval=0;
  return 1;
}
#elif defined(GLib_SOLARIS)
int _matherr(struct __math_exception* e){
  e->retval=0;
  return 1;
}
#elif defined(GLib_CYGWIN)
int matherr(struct __exception *e){
  e->retval=0;
  return 1;
}
#elif defined(GLib_MACOSX)
//int matherr(struct exception *e) {
//  e->retval=0;
//  return 1;
//}
#else
int _matherr(struct _exception* e){
  e->retval=0;
  return 1;
}
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

#if defined(SW_TRACE) && defined(GLib_UNIX)
void PrintBacktrace() {
	/*
  // stack dump, works for g++
  void *array[20];
  size_t size;

  // flush stdout
  fflush(0);

  // get the trace and print it to stdout
  size = backtrace(array, 20);
  backtrace_symbols_fd(array, size, 1);
	*/
	void *trace[16];
	char **messages = (char **) NULL;
	int i, trace_size = 0;
#endif

/////////////////////////////////////////////////
// Assertions
TOnExeStop::TOnExeStopF TOnExeStop::OnExeStopF=NULL;

void ExeStop(
 const char* MsgCStr, const char* ReasonCStr,
 const char* CondCStr, const char* FNm, const int& LnN){
  char ReasonMsgCStr[1000];

#if defined(SW_TRACE) && defined(GLib_UNIX)
  PrintBacktrace();
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

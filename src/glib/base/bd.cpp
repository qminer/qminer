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

#if defined(SW_TRACE)
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

#if defined(SW_TRACE)
void PrintBacktrace() {
  // stack dump, works for g++
  void *array[20];
  size_t size;

  // flush stdout
  fflush(0);

  // get the trace and print it to stdout
  size = backtrace(array, 20);
  backtrace_symbols_fd(array, size, 1);
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

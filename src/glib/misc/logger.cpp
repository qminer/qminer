/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "logger.h"

void TLogger::NotifyVerbose(const int& VerbosityLevel, const char *Str) 
{
    if (this->VerbosityLevel >= VerbosityLevel)
        Notify(ntInfo, Str);
}

void TLogger::NotifyVerbose(const int& VerbosityLevel, const TNotifyType& Type, const char *Str)
{
    if (this->VerbosityLevel >= VerbosityLevel)
        Notify(Type, Str);
}

void TLogger::NotifyVerboseFmt(const int& VerbosityLevel, const char *FmtStr, ...) 
{
    if (this->VerbosityLevel >= VerbosityLevel) {
        va_list valist; va_start(valist, FmtStr);   
        NotifyFmt(ntInfo, FmtStr, valist); va_end(valist); 
    }
}

void TLogger::NotifyVerboseFmt(const int& VerbosityLevel, const TNotifyType& Type, const char *FmtStr, ...)
{
    if (this->VerbosityLevel >= VerbosityLevel) {
        va_list valist; va_start(valist, FmtStr);
        NotifyFmt(Type, FmtStr, valist); va_end(valist);
    }
}

void TLogger::NotifyInfo(const char *Str)
{
    Notify(ntInfo, Str);
}

void TLogger::NotifyWarn(const char *Str)
{
    Notify(ntWarn, Str);
}

void TLogger::NotifyErr(const char *Str)
{
    Notify(ntErr, Str);
}

void TLogger::NotifyInfoFmt(const char *FmtStr, ...) 
{
    va_list valist; va_start(valist, FmtStr);   
    NotifyFmt(ntInfo, FmtStr, valist); va_end(valist); 
}

void TLogger::NotifyWarnFmt(const char *FmtStr, ...)
{
    va_list valist; va_start(valist, FmtStr);
    NotifyFmt(ntWarn, FmtStr, valist); va_end(valist);
}

void TLogger::NotifyErrFmt(const char *FmtStr, ...) 
{
    va_list valist; va_start(valist, FmtStr);
    NotifyFmt(ntErr, FmtStr, valist); va_end(valist);
}

void TLogger::NotifyErr(const char *Str, const PExcept& Except)
{
    TStr Error = Str + Except->GetMsgStr() + "\n" + Except->GetLocStr();
    Notify(ntErr, Error.CStr());
}


void TLogger::NotifyFmt(const TNotifyType& Type, const char *FmtStr, va_list argptr) 
{
    const int RetVal=vsnprintf(NotifyBuff, NOTIFY_BUFF_SIZE-2, FmtStr, argptr);
    if (RetVal < 0) return;
    Notify(Type, NotifyBuff);
}

void TLogger::Notify(const TNotifyType& Type, const char *Str) 
{
    try {
        TChA FullStr;
        // prepend spaces if necessary
        FullStr.AddChTo(' ', StartingSpaces);
        FullStr += Str;
        for (int N = 0; N < NotifyInstV.Len(); N++) {
            if (!NotifyInstV[N].Empty())
                NotifyInstV[N]->OnNotify(Type, FullStr);
        }
    }
    catch (PExcept ex) {
        TNotify::StdNotify->OnNotifyFmt(ntErr, "Notify error: %s", ex->GetMsgStr().CStr());
    }
    catch (...) {
        TNotify::StdNotify->OnNotify(ntErr, "Notify error.");
    }
}
void TLogger::PrintInfo(const TStr& Str)
{
    PrintInfo(Str.CStr());
}

void TLogger::PrintWarning(const TStr& Str)
{
    PrintWarning(Str.CStr());
}

void TLogger::PrintError(const TStr& Str)
{
    PrintError(Str.CStr());
}

void TLogger::PrintInfo(const char *FmtStr, ...)
{
    va_list valist; va_start(valist, FmtStr);
    printf("*** Info: ");
    printf(FmtStr, valist);
    printf("\n");
}

void TLogger::PrintWarning(const char *FmtStr, ...)
{
    va_list valist; va_start(valist, FmtStr);
    printf("*** Warning: ");
    printf(FmtStr, valist);
    printf("\n");
}

void TLogger::PrintError(const char *FmtStr, ...)
{
    va_list valist; va_start(valist, FmtStr);
    printf("*** Error: ");
    printf(FmtStr, valist);
    printf("\n");
}

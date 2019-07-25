/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <base.h>

// the logger class offers two main functionalities above the TNotify classes
// 1. Allows one to add a group of notifiers that will all be notified
//      Add notifiers using AddLogger() calls
//      Calls to any notification method (NotifyInfo, NotifyErr, Notify, NotifyVerbose, ...) will store the data in all notifiers
// 2. Allows user to set verbosity level that will determine if the NotifyVerbose and NotifyVerboseFmt will actually store the
//    notifications or not.
//    When calling NotifyVerbose* specify the verbosity level. Notification will be stored only if the TLogger::VerbosityLevel value
//    is same or lower. Handy for debug messages that you'd only want to see diplayed in testing mode
#define NOTIFY_BUFF_SIZE 100*1024
ClassTP(TLogger, PLogger)//{
private:
    int VerbosityLevel;
    char NotifyBuff[NOTIFY_BUFF_SIZE];
    int StartingSpaces;

    //TLogger(const TLogger&);
    TLogger& operator=(const TLogger&);

public:
    static PLogger New(int VerbosityLevel = 3) {
        return PLogger(new TLogger(VerbosityLevel));
    }
    TLogger(const int& _VerbosityLevel = 3) { VerbosityLevel = _VerbosityLevel; StartingSpaces = 0; }
    ~TLogger() { }

    // i don't know why, but I had to define this in order to compile
    bool operator==(const TLogger& BSet) const { return &BSet == this;}

    TVec<PNotify> NotifyInstV;

    // (In/De)creaseSpaces methods allow one to have indented content in the logs. can be used to get tree-like view of function calls
    void IncreaseSpaces(const int& ForVal = 2) { StartingSpaces = MIN(StartingSpaces + ForVal, 100); }
    void DecreaseSpaces(const int& ForVal = 2) { StartingSpaces = MAX(StartingSpaces - ForVal, 0); }
    void ResetSpaces() { StartingSpaces = 0; }

    // when calling NotifyVerbose, only VerbosityLevel >= _VerbosityLevel will be used
    void SetVerbosityLevel(const int _VerbosityLevel) { VerbosityLevel = _VerbosityLevel; }
    int GetVerbosityLevel() { return VerbosityLevel; }
    void AddLogger(const PNotify& Notify) { NotifyInstV.Add(Notify); }
    void RemoveLoggers() { NotifyInstV.Clr(); }
    void RemoveLogger(const PNotify& Notify) {
        // SearchForW on the vector doesn't compile so we have to check item by item
        for (int N = 0; N < NotifyInstV.Len(); N++) {
            if (NotifyInstV[N]() == Notify())
                NotifyInstV.Del(N);
        }
        //NotifyInstV.DelIfIn(Notify);
    }

    void NotifyVerbose(const int& VerbosityLevel, const char *Str);
    void NotifyVerbose(const int& VerbosityLevel, const TNotifyType& Type, const char *Str);
    void NotifyVerboseFmt(const int& VerbosityLevel, const char *FmtStr, ...);
    void NotifyVerboseFmt(const int& VerbosityLevel, const TNotifyType& Type, const char *FmtStr, ...);

    void NotifyInfo(const char *Str);
    void NotifyWarn(const char *Str);
    void NotifyErr(const char *Str);

    void NotifyInfoFmt(const char *FmtStr, ...);
    void NotifyWarnFmt(const char *FmtStr, ...);
    void NotifyErrFmt(const char *FmtStr, ...);
    void NotifyErr(const char *Str, const PExcept& Except);
    void NotifyFmt(const TNotifyType& Type, const char *FmtStr, va_list Argptr);
    void Notify(const TNotifyType& Type, const char *Str);

    void StatusFmt(const char *FmtStr, ...);
    void Status(const char *Str);

    static void PrintInfo(const TStr& Str);
    static void PrintWarning(const TStr& Str);
    static void PrintError(const TStr& Str);

    static void PrintInfo(const char *FmtStr, ...);
    static void PrintWarning(const char *FmtStr, ...);
    static void PrintError(const char *FmtStr, ...);

    static void Print(const TStr& Type, const char *FmtStr, va_list Argptr);
};

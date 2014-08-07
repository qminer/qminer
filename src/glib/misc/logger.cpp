#include "logger.h"

void TLogger::NotifyVerbose(const int& VerbosityLevel, const char *Str) 
{
	if (this->VerbosityLevel >= VerbosityLevel)
		Notify(ntInfo, Str);
}

void TLogger::NotifyVerboseFmt(const int& VerbosityLevel, const char *FmtStr, ...) 
{
	if (this->VerbosityLevel >= VerbosityLevel) {
		va_list valist; va_start(valist, FmtStr);	
		Notify(ntInfo, FmtStr, valist); va_end(valist); 
	}
}

void TLogger::NotifyInfo(const char *FmtStr, ...) 
{
	va_list valist; va_start(valist, FmtStr);	
	Notify(ntInfo, FmtStr, valist); va_end(valist); 
}

void TLogger::NotifyErr(const char *FmtStr, ...) 
{
	va_list valist; va_start(valist, FmtStr);
	Notify(ntErr, FmtStr, valist); va_end(valist); 
}

void TLogger::NotifyErr(const char *Str, const PExcept& Except)
{
	TStr Error = Str + Except->GetMsgStr() + "\n" + Except->GetLocStr();
	Notify(ntErr, Error.CStr());
}


void TLogger::Notify(const TNotifyType& Type, const char *FmtStr, va_list argptr) 
{
	const int RetVal=vsnprintf(NotifyBuff, NOTIFY_BUFF_SIZE-2, FmtStr, argptr);
	if (RetVal < 0) return;
	Notify(Type, NotifyBuff);
}

void TLogger::Notify(const TNotifyType& Type, const char *Str) 
{
	TChA FullStr;
	// prepend spaces if necessary
	FullStr.AddChTo(' ', StartingSpaces);
	FullStr += Str;	
	for (int N=0; N < NotifyInstV.Len(); N++) {
		if (! NotifyInstV[N].Empty())
			NotifyInstV[N]->OnNotify(Type, FullStr); 
	}
}
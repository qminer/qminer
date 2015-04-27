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

#include "bd.h"

/////////////////////////////////////////////////
// Find-File-Descriptor
ClassTP(TFFileDesc, PFFileDesc)//{
public:
#ifdef GLib_WIN
  WIN32_FIND_DATA FDesc;
  HANDLE FFileH;
#else
  DIR* FDesc;
  dirent* DirEnt;
#endif
public:
  TFFileDesc();
  static PFFileDesc New(){return PFFileDesc(new TFFileDesc());}
  ~TFFileDesc();
  TFFileDesc(TSIn&){Fail;}
  static PFFileDesc Load(TSIn& SIn){return new TFFileDesc(SIn);}
  void Save(TSOut&){Fail;}

  TFFileDesc& operator=(const TFFileDesc&){Fail; return *this;}

  TStr GetFBase() const;
  bool IsDir() const;
};

/////////////////////////////////////////////////
// Find-File
ClassTP(TFFile, PFFile)//{
private:
  TStrV FPathV, FExtV;
  TStr FBaseWc;
  bool CsImpP;
  bool RecurseP;
  int FPathN;
  PFFileDesc FFileDesc;
  PFFile SubFFile;
  TStr CurFNm;
  int CurFNmN;
  UndefDefaultCopyAssign(TFFile);
public:
  TFFile(const TStr& _FNmWc, const bool& _RecurseP=false);
  TFFile(const TStr& _FPath, const TStr& _FExt, const bool& _RecurseP=false);
  TFFile(const TStrV& _FPathV, const TStrV& _FExtV,
   const TStr& _FBaseWc, const bool& _RecurseP);
  static PFFile New(const TStr& FNmWc, const bool& RecurseP){
    return PFFile(new TFFile(FNmWc, RecurseP));}
  static PFFile New(const TStrV& FPathV, const TStrV& FExtV,
   const TStr FBaseWc, const bool& RecurseP){
    return PFFile(new TFFile(FPathV, FExtV, FBaseWc, RecurseP));}
  static PFFile New(const TStr& FPath, const TStr& FExt, const bool& RecurseP){
    return PFFile(new TFFile(FPath, FExt, RecurseP));}
  ~TFFile();
  TFFile(TSIn&){Fail;}
  static PFFile Load(TSIn& SIn){return new TFFile(SIn);}
  void Save(TSOut&){Fail;}

  bool Next(TStr& FNm);
  bool Next(){TStr FNm; return Next(FNm);}
  TStr GetFNm() const {return CurFNm;}
  int GetFNmN() const {return CurFNmN;}

  static void GetFNmV(
   const TStr& FPath, const TStrV& FExtV, const bool& RecurseP, TStrV& FNmV);
  static void GetFNmV(
   const TStr& FPath, const TStr& FExt, const bool& RecurseP, TStrV& FNmV);

  bool IsDir() const {
   return FFileDesc->IsDir();}
};

/////////////////////////////////////////////////
// Directories
class TDir{
public:
  static TStr GetCurDir();
  static TStr GetExeDir();
  static bool GenDir(const TStr& FPathFNm);
  static bool DelDir(const TStr& FPathFNm);
  static bool Exists(const TStr& FPathFNm);
};

/////////////////////////////////////////////////
// File-Path-Log
typedef enum { fpnrNone, fpnrHour, fpnrDay } TFPathNotifyRollover;
class TFPathNotify : public TNotify {
private:
	// file name parameters
	TStr LogFPath;
	TStr PrefixFNm;
	// rollover
	TFPathNotifyRollover Rollover;
	TTm LastTm;
	// output
	PSOut LogSOut;
	TBool FlushP;

private:
	void UpdateSOut(const TTm& NowTm);
	UndefDefaultCopyAssign(TFPathNotify);

public:
	TFPathNotify(const TStr& _LogFPath, const TStr& _PrefixFNm, 
		const bool& _FlushP, const TFPathNotifyRollover& _Rollover);
	static PNotify New(const TStr& LogFPath, const TStr& PrefixFNm, 
		const bool& FlushP, const TFPathNotifyRollover& Rollover = fpnrNone) {
			return PNotify(new TFPathNotify(LogFPath, PrefixFNm, FlushP, Rollover));}

	void OnStatus(const TStr& MsgStr);
};

/////////////////////////////////////////////////
// File-Lock
class TFileLock {
private:
  TStr LockFNm;
  TStr LockId;
public:
  TFileLock(const TStr& _LockFNm);
  // create lock
  void Lock();
  // remove lock
  void Unlock();
};

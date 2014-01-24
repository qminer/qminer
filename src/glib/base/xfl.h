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

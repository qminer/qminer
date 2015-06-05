/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Find-File-Descriptor
#ifdef GLib_WIN
TFFileDesc::TFFileDesc(): FFileH(INVALID_HANDLE_VALUE) {}

bool TFFileDesc::IsDir() const {
  return (FDesc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)!=0;
}

TStr TFFileDesc::GetFBase() const {
  return TStr(FDesc.cFileName);
}

TFFileDesc::~TFFileDesc() {}

#elif defined(GLib_UNIX)

TFFileDesc::TFFileDesc(): FDesc(NULL), DirEnt(NULL) {}

bool TFFileDesc::IsDir() const {
  Fail;   // !bn: function not needed; besides we do not have full path to the entry
  return false;
}

TStr TFFileDesc::GetFBase() const {
  Assert(DirEnt != NULL);
  return TStr(DirEnt->d_name);
}

TFFileDesc::~TFFileDesc() {
  if (FDesc) closedir(FDesc);
}
#endif

/////////////////////////////////////////////////
// Find-File
TFFile::TFFile(const TStr& FNmWc, const bool& _RecurseP):
  FPathV(), FExtV(), FBaseWc(),
  CsImpP(false), RecurseP(_RecurseP), FPathN(0-1),
  FFileDesc(TFFileDesc::New()), SubFFile(), CurFNm(), CurFNmN(0-1){
  // prepare file-base-name wild-card
	FBaseWc = FNmWc.GetFBase(); if (!CsImpP){ FBaseWc = FBaseWc.GetUc(); }
  // get & assign file-name
  TStr FPath=FNmWc.GetFPath();
  FPathV.Add(TStr::GetNrFPath(FPath));
}

TFFile::TFFile(const TStr& _FPath, const TStr& _FExt, const bool& _RecurseP):
  FPathV(), FExtV(), FBaseWc(),
  CsImpP(false), RecurseP(_RecurseP), FPathN(0-1),
  FFileDesc(TFFileDesc::New()), SubFFile(), CurFNm(), CurFNmN(0-1){
  FPathV.Add(TStr::GetNrFPath(_FPath));
  if (!_FExt.Empty()){
    FExtV.Add(TStr::GetNrFExt(_FExt));
	if (!CsImpP){ FExtV.Last() = FExtV.Last().GetUc(); }
  }
}

TFFile::TFFile(const TStrV& _FPathV, const TStrV& _FExtV, const TStr& _FBaseWc,
 const bool& _RecurseP):
  FPathV(_FPathV), FExtV(_FExtV), FBaseWc(_FBaseWc),
  CsImpP(false), RecurseP(_RecurseP), FPathN(0-1),
  FFileDesc(TFFileDesc::New()), SubFFile(), CurFNm(), CurFNmN(0-1){
  // prepare file-paths
  for (int FPathN=0; FPathN<FPathV.Len(); FPathN++){
    FPathV[FPathN]=TStr::GetNrFPath(FPathV[FPathN]);}
  // prepare file-extensions
  for (int FExtN=0; FExtN<FExtV.Len(); FExtN++){
    FExtV[FExtN]=TStr::GetNrFExt(FExtV[FExtN]);
	if (!CsImpP){ FExtV[FExtN] = FExtV[FExtN].GetUc(); }
  }
  // prepare file-base wild-card
  if (!CsImpP){ FBaseWc = FBaseWc.GetUc(); }
}

#ifdef GLib_WIN
TFFile::~TFFile(){
  if (FFileDesc->FFileH!=INVALID_HANDLE_VALUE){
    IAssert(FindClose(FFileDesc->FFileH));}
}

bool TFFile::Next(TStr& FNm){
  // if need to recurse
  if (!SubFFile.Empty()){
    if (SubFFile->Next(FNm)){CurFNm=FNm; CurFNmN++; return true;}
    else {SubFFile=NULL;}
  }
  // for all required file-paths
  while (FPathN<FPathV.Len()){
    if ((FPathN!=-1)&&(FindNextFile(FFileDesc->FFileH, &FFileDesc->FDesc))){
      // next file-name available on the current file-path
      TStr FBase=FFileDesc->GetFBase();
      if ((RecurseP)&&(FFileDesc->IsDir())){
        // file-name is directory and recursion is required
        if ((FBase!=".")&&(FBase!="..")){
          // directory is non-trivial - prepare sub-file-find for recursion
          TStr SubFPath=FPathV[FPathN]+FBase;
          TStrV SubFPathV; SubFPathV.Add(SubFPath);
          SubFFile=New(SubFPathV, FExtV, FBaseWc, RecurseP);
          if (SubFFile->Next(FNm)){CurFNm=FNm; CurFNmN++; return true;}
          else {SubFFile=NULL;}
        }
      } else {
        // return file-name if fits
        if ((FBase!=".")&&(FBase!="..")){
          FNm=FPathV[FPathN]+FBase;
          TStr FExt=FNm.GetFExt(); if (!CsImpP){FExt = FExt.GetUc(); FBase = FBase.GetUc();}
          if (((FExtV.Empty())||(FExtV.SearchForw(FExt)!=-1))&&
           ((FBaseWc.Empty())||(FBase.IsWcMatch(FBaseWc)))){
            CurFNm=FNm; CurFNmN++; return true;}
        }
      }
    } else {
      // close file-find descriptor if needed
      if (FPathN!=-1){
        IAssert(FindClose(FFileDesc->FFileH));
        FFileDesc->FFileH=INVALID_HANDLE_VALUE;
      }
      // find next file existing path from the input list
      while ((++FPathN<FPathV.Len())&&
       ((FFileDesc->FFileH=FindFirstFile((FPathV[FPathN]+"*.*").CStr(),
       &FFileDesc->FDesc))==INVALID_HANDLE_VALUE)){}
      if ((FPathN<FPathV.Len())&&(RecurseP)&&(FFileDesc->IsDir())){
        // file-path found, file-name is directory and recursion is required
        TStr FBase=FFileDesc->GetFBase();
        if ((FBase!=".")&&(FBase!="..")){
          TStr SubFPath=FPathV[FPathN]+FBase;
          TStrV SubFPathV; SubFPathV.Add(SubFPath);
          SubFFile=New(SubFPathV, FExtV, FBaseWc, RecurseP);
          if (SubFFile->Next(FNm)){CurFNm=FNm; CurFNmN++; return true;}
          else {SubFFile=NULL;}
        }
      } else {
        // return file-name if fits
        if (FPathN<FPathV.Len()){
          TStr FBase=FFileDesc->GetFBase();
          if ((FBase!=".")&&(FBase!="..")){
            FNm=FPathV[FPathN]+FBase;
            TStr FExt=FNm.GetFExt(); if (!CsImpP){FExt = FExt.GetUc(); FBase = FBase.GetUc();}
            if (((FExtV.Empty())||(FExtV.SearchForw(FExt)!=-1))&&
             ((FBaseWc.Empty())||(FBase.IsWcMatch(FBaseWc)))){
              CurFNm=FNm; CurFNmN++; return true;
            }
          }
        }
      }
    }
  }
  // not found
  CurFNm=""; CurFNmN=-1; return false;
}
#elif defined(GLib_UNIX)
TFFile::~TFFile(){}

bool TFFile::Next(TStr& FNm){
  // if need to recurse
  if (!SubFFile.Empty()){
    if (SubFFile->Next(FNm)){CurFNm=FNm; CurFNmN++; return true;}
    else {SubFFile=NULL;}
  }
  // for all required file-paths
  while (FPathN<FPathV.Len()){
    // try to find anything within FPathV[FPathN] directory
    while (true) {
      // if directory not open -> open next first
      if (!FFileDesc->FDesc) {
        if ((++FPathN)<FPathV.Len()) {
          FFileDesc->FDesc = opendir(FPathV[FPathN].CStr());
        } else break;
        if (!FFileDesc->FDesc) break;   // failed to open this one; pass control to outer loop
      }

      FFileDesc->DirEnt = readdir(FFileDesc->FDesc);

      if (FFileDesc->DirEnt) {
        // found something
        TStr FBase = FFileDesc->GetFBase();
        FNm = FPathV[FPathN]+FBase;

        struct stat Stat;
        int ErrCd = stat(FNm.CStr(), &Stat);
        if (ErrCd != 0) { EAssert(ErrCd==0); }

        if (S_ISREG(Stat.st_mode)) {
          if ((FBase!=".")&&(FBase!="..")){
            TStr FExt=FNm.GetFExt(); if (!CsImpP){FExt = FExt.GetUc(); FBase = FBase.GetUc();}
            if (((FExtV.Empty())||(FExtV.SearchForw(FExt)!=-1))&&
             ((FBaseWc.Empty())||(FBase.IsWcMatch(FBaseWc)))){
              CurFNm=FNm; CurFNmN++; return true;}
          }
        } else if (S_ISDIR(Stat.st_mode) && RecurseP) {
          if ((FBase!=".")&&(FBase!="..")){
            TStr SubFPath=FPathV[FPathN]+FBase;
            TStrV SubFPathV; SubFPathV.Add(SubFPath);
            SubFFile=New(SubFPathV, FExtV, FBaseWc, RecurseP);
            if (SubFFile->Next(FNm)){CurFNm=FNm; CurFNmN++; return true;}
            else {SubFFile=NULL;}
          }
        }
      } else {
        // end of directory; clean up (ignore DirEnt, it's allocated within FDesc), pass control to outer loop
        FFileDesc->DirEnt = NULL;
        int ErrCd = closedir(FFileDesc->FDesc);
        FFileDesc->FDesc = NULL;
        if (ErrCd != 0) { EAssert(ErrCd==0); }
        break;
      }
    }
  }
  // not found
  CurFNm=""; CurFNmN=-1; return false;
}
#endif

void TFFile::GetFNmV(
 const TStr& FPath, const TStrV& FExtV, const bool& RecurseP, TStrV& FNmV){
  // prepare file-directory traversal
  TStrV FPathV; FPathV.Add(FPath);
  TFFile FFile(FPathV, FExtV, "", RecurseP); TStr FNm;
  // traverse directory
  FNmV.Clr();
  while (FFile.Next(FNm)){
    FNmV.Add(FNm);
  }
}

void TFFile::GetFNmV(
 const TStr& FPath, const TStr& FExt, const bool& RecurseP, TStrV& FNmV){
  GetFNmV(FPath, TStrV::GetV(FExt), RecurseP, FNmV);    
}

/////////////////////////////////////////////////
// Directories
TStr TDir::GetCurDir(){
  const int MxBfL=1000;
  char Bf[MxBfL];
  int BfL=GetCurrentDirectory(MxBfL, Bf);
  IAssert((BfL!=0)&&(BfL<MxBfL));
  return TStr::GetNrFPath(TStr(Bf));
}

TStr TDir::GetExeDir(){
  const int MxBfL=1000;
  char Bf[MxBfL];
  int BfL=GetModuleFileName(NULL, Bf, MxBfL);
  IAssert((BfL!=0)&&(BfL<MxBfL));
  return TStr::GetNrFPath(TStr(Bf).GetFPath());
}

bool TDir::GenDir(const TStr& FPathFNm){
  return CreateDirectory(FPathFNm.CStr(), NULL)!=0;
}

bool TDir::DelDir(const TStr& FPathFNm){
  return RemoveDirectory(FPathFNm.CStr())!=0;
}

#ifdef GLib_WIN

bool TDir::Exists(const TStr& FPathFNm){
  DWORD dwAttrib = GetFileAttributes(FPathFNm.CStr());
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

#elif defined(GLib_UNIX)

bool TDir::Exists(const TStr& FPathFNm){
  return TFile::Exists(FPathFNm); //HACK
}

#endif

void TDir::ListFiles(const TStr& DirNm, TStrV& FNmV) {
#ifndef GLib_WIN
	DIR *dp;
	struct dirent *dirp;

	if ((dp = opendir(DirNm.CStr())) == nullptr) {
		throw TExcept::New("Failed to open directory " + DirNm, "TDir::ListDir");
	}

	while ((dirp = readdir(dp)) != nullptr) {
		FNmV.Add(TStr(dirp->d_name));
	}

	closedir(dp);
#else
	throw TExcept::New("TDir::ListFiles: Not implemented on Windows, please implement!");
#endif
}

//////////////////////////////////////
// File-Log
void TFPathNotify::UpdateSOut(const TTm& Tm) {
  if (!LogSOut.Empty()) { LogSOut->Flush(); LogSOut.Clr(); }
  TChA FNm = LogFPath; FNm += PrefixFNm;
  if (Rollover == fpnrDay) {
    FNm += TStr::Fmt("-Y%04d-M%02d-D%02d", Tm.GetYear(), Tm.GetMonth(), Tm.GetDay());
  } else if (Rollover == fpnrHour) {
    FNm += TStr::Fmt("-Y%04d-M%02d-D%02d-H%02d", Tm.GetYear(), Tm.GetMonth(), Tm.GetDay(), Tm.GetHour());
  }
  FNm += ".log";
  LogSOut = TFOut::New(FNm, true);
}

TFPathNotify::TFPathNotify(const TStr& _LogFPath, const TStr& _PrefixFNm,
    const bool& _FlushP, const TFPathNotifyRollover& _Rollover): LogFPath(_LogFPath), 
      PrefixFNm(_PrefixFNm), Rollover(_Rollover), FlushP(_FlushP) { 

  LastTm = TTm::GetCurUniTm();
  UpdateSOut(LastTm);
}

void TFPathNotify::OnStatus(const TStr& MsgStr) {
  // check if new hour so we switch to new log file
  if (Rollover != fpnrNone) {
    TTm NowTm = TTm::GetCurUniTm();
    if ((Rollover == fpnrDay) && (NowTm.GetDay() != LastTm.GetDay())) { 
      LastTm = NowTm; UpdateSOut(LastTm);  
    } else if ((Rollover == fpnrHour) && (NowTm.GetHour() != LastTm.GetHour())) { 
      LastTm = NowTm; UpdateSOut(LastTm);
    }
  }
  // write log line
  LogSOut->PutStrLn(MsgStr); 
  // we flush for each line when in debug mode
  if (FlushP) { LogSOut->Flush(); }
}

/////////////////////////////////////////////////
// File-Lock
TFileLock::TFileLock(const TStr& _LockFNm): 
  LockFNm(_LockFNm), LockId(TGuid::GenGuid()) { }

// create lock
void TFileLock::Lock() { 
  // make sure nobody else has the lock at the moment
  EAssertR(!TFile::Exists(LockFNm), "Lock already exists (" + LockFNm + ")!");
  // create lock file
  TFOut(LockFNm).PutStr(LockId); 
}

// remove lock
void TFileLock::Unlock() { 
  // check we still have lock file
  EAssertR(TFile::Exists(LockFNm), "Missing lock file");
  // make sure we are deleting our lock file
  TStr bla = TStr::LoadTxt(LockFNm);
  EAssertR(LockId == TStr::LoadTxt(LockFNm), "Mismatch between lock files");
  // delete the lock file (relasing the lock)
  EAssertR(TFile::Del(LockFNm, false), "Error deleting lock file");
}

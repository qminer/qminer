/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "bd.h"

/////////////////////////////////////////////////
// Blob-Pointer
ClassTVQ(TBlobPt, TBlobPtV, TBlobPtQ)//{
public:
  static const int MnBlobBfL;
  uint16 Seg;
  uint Addr;
public:
  TBlobPt(): Seg(0), Addr(TUInt::Mx){}
  TBlobPt(const TBlobPt& Pt): Seg(Pt.Seg), Addr(Pt.Addr) {}
  TBlobPt(const uint16& _Seg, const uint& _Addr): Seg(_Seg), Addr(_Addr) {}
  TBlobPt(const uint& _Addr): Seg(0), Addr(_Addr) {}
  TBlobPt(const int& _Addr): Seg(0), Addr(uint(_Addr)) {IAssert(_Addr>=0);}
  ~TBlobPt(){}
  TBlobPt(TSIn& SIn){SIn.Load(Seg); SIn.Load(Addr);}
  void Save(TSOut& SOut) const {SOut.Save(Seg); SOut.Save(Addr);}

  TBlobPt& operator=(const TBlobPt& Pt){
    if (this!=&Pt){Seg=Pt.Seg; Addr=Pt.Addr;}
    return *this;}
  bool operator==(const TBlobPt& Pt) const {
    return (Seg==Pt.Seg)&&(Addr==Pt.Addr);}
  bool operator<(const TBlobPt& Pt) const {
    return (Seg<Pt.Seg)||((Seg==Pt.Seg)&&(Addr<Pt.Addr));}
  uint64 GetMemUsed() const {return sizeof(TBlobPt);}

  int GetPrimHashCd() const {return abs(int(Addr));}
  int GetSecHashCd() const {return (abs(int(Addr))+int(Seg)*0x10);}

  bool Empty() const {return Addr==TUInt::Mx;}
  void Clr(){Seg=0; Addr=TUInt::Mx;}
  void PutSeg(const uint16& _Seg){Seg=_Seg;}
  uint16 GetSeg() const {return Seg;}
  void PutAddr(const uint& _Addr){Addr=_Addr;}
  uint GetAddr() const {return Addr;}

  static TBlobPt Load(const PFRnd& FRnd){
  uint16 Seg=FRnd->GetUInt16(); uint Addr=FRnd->GetUInt();
    return TBlobPt(Seg, Addr);}
  void Save(const PFRnd& FRnd) const {FRnd->PutUInt16(Seg); FRnd->PutUInt(Addr);}
  static TBlobPt LoadAddr(const PFRnd& FRnd, const uint16& Seg=0){
    return TBlobPt(Seg, FRnd->GetUInt());}
  void SaveAddr(const PFRnd& FRnd) const {FRnd->PutUInt(Addr);}

  TStr GetAddrStr() const {
    TChA AddrChA; AddrChA+=TInt::GetStr(Seg); AddrChA+=':';
    AddrChA+=TUInt::GetStr(Addr); return AddrChA;}

  TStr GetStr() const;
};

/////////////////////////////////////////////////
// Statistics for TBlobBs
class TBlobBsStats {
public:
  uint64 Puts;
  uint64 PutsNew;
  uint64 Gets;
  uint64 Dels;
  uint64 SizeChngs;
  double AvgGetLen;
  double AvgPutLen;
  double AvgPutNewLen;
  uint64 AllocUsedSize;
  uint64 AllocUnusedSize;
  uint64 AllocSize;
  uint64 AllocCount;
  uint64 ReleasedCount;
  uint64 ReleasedSize;

public:
  TBlobBsStats() { Reset(); }

  /// Resets data in this object
  void Reset();
  /// Creates a clone - copies all data
  TBlobBsStats Clone() const;
  /// Correctly add data from another object into this one
  void Add(const TBlobBsStats& Other);
};

/////////////////////////////////////////////////
// Blob-Base
typedef enum {bbsUndef, bbsOpened, bbsClosed} TBlobBsState;
typedef enum {btUndef, btBegin, btEnd} TBlobTag;
typedef enum {bsUndef, bsActive, bsFree} TBlobState;

ClassTPV(TBlobBs, PBlobBs, TBlobBsV)//{
public:
  static const int MnBlobBfL;
  static const int MxBlobFLen;
  UndefCopyAssign(TBlobBs);
public:
  TBlobBs(){}
  virtual ~TBlobBs(){}
  TBlobBs(TSIn&){Fail;}
  static PBlobBs Load(TSIn&){Fail; return NULL;}
  void Save(TSOut&) const {Fail;}

  virtual TStr GetVersionStr() const=0;
  void PutVersionStr(const PFRnd& FBlobBs);
  void AssertVersionStr(const PFRnd& FBlobBs);

  TStr GetBlobBsStateStr(const TBlobBsState& BlobBsState);
  int GetStateStrLen(){return 6;}
  void PutBlobBsStateStr(const PFRnd& FBlobBs, const TBlobBsState& State);
  void AssertBlobBsStateStr(const PFRnd& FBlobBs, const TBlobBsState& State);

  static const TStr MxSegLenVNm;
  void PutMxSegLen(const PFRnd& FBlobBs, const int& MxSegLen);
  int GetMxSegLen(const PFRnd& FBlobBs);

  static const TStr BlockLenVNm;
  static void GenBlockLenV(TIntV& BlockLenV);
  void PutBlockLenV(const PFRnd& FBlobBs, const TIntV& BlockLenV);
  void GetBlockLenV(const PFRnd& FBlobBs, TIntV& BlockLenV);

  static const TStr FFreeBlobPtVNm;
  void GenFFreeBlobPtV(const TIntV& BlockLenV, TBlobPtV& FFreeBlobPtV);
  void PutFFreeBlobPtV(const PFRnd& FBlobBs, const TBlobPtV& FFreeBlobPtV);
  void GetFFreeBlobPtV(const PFRnd& FBlobBs, TBlobPtV& FFreeBlobPtV);

  void GetAllocInfo(
   const int& BfL, const TIntV& BlockLenV, int& MxBfL, int& FFreeBlobPtN);

  uint GetBeginBlobTag(){return 0xABCDEFFF;}
  uint GetEndBlobTag(){return 0xFFFEDCBA;}
  void PutBlobTag(const PFRnd& FBlobBs, const TBlobTag& BlobTag);
  void AssertBlobTag(const PFRnd& FBlobBs, const TBlobTag& BlobTag);

  void PutBlobState(const PFRnd& FBlobBs, const TBlobState& State);
  TBlobState GetBlobState(const PFRnd& FBlobBs);
  void AssertBlobState(const PFRnd& FBlobBs, const TBlobState& State);

  void AssertBfCsEqFlCs(const TCs& BfCs, const TCs& FCs);

  /// save data stored in SIn for the first time
  virtual TBlobPt PutBlob(const PSIn& SIn)=0;
  TBlobPt PutBlob(const TStr& Str){
    PSIn SIn=TStrIn::New(Str); return PutBlob(SIn);}
  /// update the SIn data currently stored in BlobPt. If data is reallocated, return the size of released chunk (ReleasedSize)
  virtual TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn, int& ReleasedSize)=0;
  /// return blob stored in BlobPt
  virtual PSIn GetBlob(const TBlobPt& BlobPt)=0;
  /// delete blob stored in BlobPt. Return the size of the released data block
  virtual int DelBlob(const TBlobPt& BlobPt)=0;

  virtual TBlobPt GetFirstBlobPt()=0;
  virtual TBlobPt FFirstBlobPt()=0;
  virtual bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn)=0;
  bool FNextBlobPt(TBlobPt& TrvBlobPt, PSIn& BlobSIn){
    TBlobPt BlobPt; return FNextBlobPt(TrvBlobPt, BlobPt, BlobSIn);}

  virtual const TBlobBsStats& GetStats()=0;
  virtual void ResetStats() = 0;
};

/////////////////////////////////////////////////
// General-Blob-Base
class TGBlobBs: public TBlobBs{
private:
  /// Random-access file - BLOB storage
  PFRnd FBlobBs;
  /// access mode for file
  TFAccess Access;
  /// maximal length of segment - of BLOB file
  int MxSegLen;
  /// list of precomputed block lengths - each BLOB falls into one of them,
  /// so that allocations happen in just several possible chuncks
  TIntV BlockLenV;
  /// list of free blob pointers (their content was deleted, so blobs are free)
  TBlobPtV FFreeBlobPtV;
  TBlobPt FirstBlobPt;
  static TStr GetNrBlobBsFNm(const TStr& BlobBsFNm);
  TBlobBsStats Stats;
public:
  TGBlobBs(const TStr& BlobBsFNm, const TFAccess& _Access=faRdOnly,
   const int& _MxSegLen=-1);
  static PBlobBs New(const TStr& BlobBsFNm, const TFAccess& Access=faRdOnly,
   const int& MxSegLen=-1){
    return PBlobBs(new TGBlobBs(BlobBsFNm, Access, MxSegLen));}
  ~TGBlobBs();

  TGBlobBs& operator=(const TGBlobBs&){Fail; return *this;}

  TStr GetVersionStr() const {return TStr("General Blob Base Format 1.0");}
  // save data stored in SIn for the first time
  TBlobPt PutBlob(const PSIn& SIn);
  // update the SIn data currently stored in BlobPt. If data is reallocated, return the size of released chunk (ReleasedSize)
  TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn, int& ReleasedSize);
  // return blob stored in BlobPt
  PSIn GetBlob(const TBlobPt& BlobPt);
  // delete blob stored in BlobPt. Return the size of the released data block
  int DelBlob(const TBlobPt& BlobPt);

  TBlobPt GetFirstBlobPt(){return FirstBlobPt;}
  TBlobPt FFirstBlobPt();
  bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn);

  static bool Exists(const TStr& BlobBsFNm);

  const TBlobBsStats& GetStats() { return Stats; }
  void ResetStats() { Stats.Reset(); }
};

/////////////////////////////////////////////////
// Multiple-File-Blob-Base
class TMBlobBs: public TBlobBs{
private:
  TFAccess Access;
  int MxSegLen;
  TStr NrFPath, NrFMid;
  TBlobBsV SegV;
  TIntV BlockLenV;
  /// for each block size store the segment index that last had space to store the buffer of that size
  THash<TInt, TUInt16> BlockSizeToSegH;
  static void GetNrFPathFMid(const TStr& BlobBsFNm, TStr& NrFPath, TStr& NrFMid);
  static TStr GetMainFNm(const TStr& NrFPath, const TStr& NrFMid);
  static TStr GetSegFNm(const TStr& NrFPath, const TStr& NrFMid, const int& SegN);
  void LoadMain(int& Segs);
  void SaveMain() const;
  TBlobBsStats Stats;
public:
  TMBlobBs(const TStr& BlobBsFNm, const TFAccess& _Access=faRdOnly,
   const int& _MxSegLen=-1);
  static PBlobBs New(const TStr& BlobBsFNm, const TFAccess& Access=faRdOnly,
   const int& MxSegLen=-1){
    return PBlobBs(new TMBlobBs(BlobBsFNm, Access, MxSegLen));}
  ~TMBlobBs();

  TMBlobBs& operator=(const TMBlobBs&){Fail; return *this;}

  TStr GetVersionStr() const {
    return TStr("Multiple-File Blob Base Format 1.0");}
  /// save data stored in SIn for the first time
  TBlobPt PutBlob(const PSIn& SIn);
  /// update the SIn data currently stored in BlobPt. If data is reallocated, return the size of released chunk (ReleasedSize)
  TBlobPt PutBlob(const TBlobPt& BlobPt, const PSIn& SIn, int& ReleasedSize);
  /// return blob stored in BlobPt
  PSIn GetBlob(const TBlobPt& BlobPt);
  /// delete blob stored in BlobPt. Return the size of the released data block
  int DelBlob(const TBlobPt& BlobPt);

  TBlobPt GetFirstBlobPt();
  TBlobPt FFirstBlobPt();
  bool FNextBlobPt(TBlobPt& TrvBlobPt, TBlobPt& BlobPt, PSIn& BlobSIn);

  static bool Exists(const TStr& BlobBsFNm);

  const TBlobBsStats& GetStats();
  void ResetStats();
};

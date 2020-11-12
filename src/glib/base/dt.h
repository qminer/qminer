/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "bd.h"


/////////////////////////////////////////////////
// Forward
class TILx;
class TOLx;
ClassHdTP(TXmlTok, PXmlTok);

/////////////////////////////////////////////////
// Random
class TRnd{
public:
  static const int RndSeed;
private:
  static const int a, m, q, r;
  int Seed;
  int GetNextSeed(){
    if ((Seed=a*(Seed%q)-r*(Seed/q))>0){return Seed;} else {return Seed+=m;}}
public:
  TRnd(const int& _Seed=1, const int& Steps=0){
    PutSeed(_Seed); Move(Steps);}
  explicit TRnd(TSIn& SIn){SIn.Load(Seed);}
  void Save(TSOut& SOut) const {SOut.Save(Seed);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TRnd& operator=(const TRnd& Rnd){Seed=Rnd.Seed; return *this;}
  bool operator==(const TRnd&) const {Fail; return false;}

  double GetUniDev(){return GetNextSeed()/double(m);}
  int GetUniDevInt(const int& Range=0);
  /// returns a random integer in range [MnVal, MxVal] (both inclusive)
  int GetUniDevInt(const int& MnVal, const int& MxVal){
    IAssert(MnVal<=MxVal); return MnVal+GetUniDevInt(MxVal-MnVal+1);}
  uint GetUniDevUInt(const uint& Range=0);
  int64 GetUniDevInt64(const int64& Range=0);
  uint64 GetUniDevUInt64(const uint64& Range=0);
  double GetNrmDev();
  double GetNrmDev(
   const double& Mean, const double& SDev, const double& Mn, const double& Mx);
  double GetExpDev();
  double GetExpDev(const double& Lambda); // mean=1/lambda
  double GetGammaDev(const int& Order);
  double GetPoissonDev(const double& Mean);
  double GetBinomialDev(const double& Prb, const int& Trials);
  int GetGeoDev(const double& Prb){
    return 1+(int)floor(log(1.0-GetUniDev())/log(1.0-Prb));}
  double GetPowerDev(const double& AlphaSlope){ // power-law degree distribution (AlphaSlope>0)
    IAssert(AlphaSlope>1.0);
    return pow(1.0-GetUniDev(), -1.0/(AlphaSlope-1.0));}
  double GetRayleigh(const double& Sigma) { // 1/sqrt(alpha) = sigma
    IAssert(Sigma>0.0);
    return Sigma*sqrt(-2*log(1-GetUniDev()));}
  double GetWeibull(const double& K, const double& Lambda) { // 1/alpha = lambda
    IAssert(Lambda>0.0 && K>0.0);
    return Lambda*pow(-log(1-GetUniDev()), 1.0/K);}
  //void GetSphereDev(const int& Dim, TFltV& ValV);

  void PutSeed(const int& _Seed);
  int GetSeed() const {return Seed;}
  void Randomize(){PutSeed(RndSeed);}
  void Move(const int& Steps);
  bool Check();

  static double GetUniDevStep(const int& Seed, const int& Steps){
    TRnd Rnd(Seed); Rnd.Move(Steps); return Rnd.GetUniDev();}
  static double GetNrmDevStep(const int& Seed, const int& Steps){
    TRnd Rnd(Seed); Rnd.Move(Steps); return Rnd.GetNrmDev();}
  static double GetExpDevStep(const int& Seed, const int& Steps){
    TRnd Rnd(Seed); Rnd.Move(Steps); return Rnd.GetExpDev();}

  static TRnd LoadTxt(TILx& Lx);
  void SaveTxt(TOLx& Lx) const;

  uint64 GetMemUsed() const;
};

/////////////////////////////////////////////////
// Memory chunk - simple buffer, non-resizable
class TMemBase {
protected:
  int MxBfL, BfL;
  char* Bf;
  bool Owner;
public:
  TMemBase() : MxBfL(0), BfL(0), Bf(NULL), Owner(false) {}
  TMemBase(const int& _BfL) : MxBfL(_BfL), BfL(_BfL), Bf(NULL), Owner(true) {
    IAssert(BfL >= 0);
    Bf = new char[BfL];
  }
  TMemBase(const void* _Bf, const int& _BfL, const bool& _Owner = true) :
    MxBfL(_BfL), BfL(_BfL), Bf(NULL), Owner(_Owner) {
    IAssert(BfL >= 0);
    if (BfL > 0) {
      if (Owner) {
        Bf = new char[BfL]; IAssert(Bf != NULL); memcpy(Bf, _Bf, BfL);
      } else {
        Bf = (char*)_Bf;
      }
    }
  }
  TMemBase(TMemBase&& Src) {
    MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = Src.Bf; Owner = Src.Owner;
    Src.MxBfL = Src.BfL = 0; Src.Bf = NULL;  Src.Owner = false;
  }
  virtual ~TMemBase() {
    if (Owner && Bf != NULL) {
      delete[] Bf; } }
  int Len() const { return BfL; }
  bool Empty() const { return BfL == 0; }
  char* GetBf() const { return Bf; }
  void Copy(const TMemBase& Mem) {
    if (this != &Mem) {
      if (Owner && Bf != NULL) { delete[] Bf; }
      MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL; Owner = (MxBfL > 0);
      if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL); }
    }
  }
  TMemBase& operator=(TMemBase&& Src) {
    if (this != &Src) {
      if (Owner && Bf != NULL) { delete[] Bf; }
      MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = Src.Bf; Owner = Src.Owner;
      Src.MxBfL = Src.BfL = 0; Src.Bf = NULL;  Src.Owner = false;
    }
    return *this;
  }
  TMemBase& operator=(const TMemBase& Mem) {
    Copy(Mem); return *this;
  }
  friend class TMem;
};

/////////////////////////////////////////////////
/// Thin Input-Memory. Used to present existing TMem as TSIn.
/// It doesn't allocate or release any memory.
class TThinMIn : public TSIn {
protected:
  uchar* Bf;
  int BfC, BfL;
public:
  TThinMIn();
  TThinMIn(const TMemBase& Mem);
  TThinMIn(const void* _Bf, const int& _BfL);
  TThinMIn(const TThinMIn& min);

  bool Eof() { return BfC == BfL; }
  int Len() const { return BfL - BfC; }
  char GetCh();
  char PeekCh();
  int GetBf(const void* LBf, const TSize& LBfL);
  void Reset() { Cs = TCs(); BfC = 0; }
  uchar* GetBfAddr() { return Bf; }
  char* GetBfAddrChar() { return (char*)Bf; }
  void MoveTo(int Offset);
  bool GetNextLnBf(TChA& LnChA);
  TMemBase GetMemBase() { return TMemBase(GetBfAddr(), Len(), false); }
  TThinMIn& operator=(const TThinMIn& Mem) {
    Bf = Mem.Bf; BfC = Mem.BfC; BfL = Mem.BfL; return *this;
  }
  TStr GetSNm() const;
};

/////////////////////////////////////////////////////////////////////
/// Memory chunk. Advanced memory buffer, supports resizing etc.
class TMem {
protected:
  int MxBfL, BfL;
  char* Bf;
  void Resize(const int& _MxBfL);
  bool DoFitLen(const int& LBfL) const {return BfL+LBfL<=MxBfL;}

public:
  TMem(const int& _MxBfL=0) : MxBfL(_MxBfL), BfL(0), Bf(NULL) {
    IAssert(BfL >= 0); BfL = 0; Bf = NULL;
    if (MxBfL>0){Bf=new char[MxBfL]; IAssert(Bf!=NULL);}}
  TMem(const void* _Bf, const int& _BfL) : MxBfL(_BfL), BfL(_BfL), Bf(NULL) {
    IAssert(BfL >= 0); MxBfL = _BfL; BfL = _BfL; Bf = NULL;
    if (BfL > 0) { Bf = new char[BfL]; IAssert(Bf != NULL); memcpy(Bf, _Bf, BfL); } }
  TMem(const TMem& Mem) : MxBfL(0), BfL(0), Bf(NULL) {
    MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL;
    if (MxBfL>0){Bf=new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL);}}
  TMem(const TStr& Str);
  TMem(TMem&& Src) : MxBfL(0), BfL(0), Bf(NULL) {
    MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = Src.Bf;
    Src.MxBfL = Src.BfL = 0; Src.Bf = NULL;
  }
  ~TMem() { if (Bf != NULL) { delete[] Bf; }; Bf = NULL; }
  explicit TMem(TSIn& SIn) {
    SIn.Load(MxBfL); SIn.Load(BfL);
    Bf = new char[MxBfL = BfL]; SIn.LoadBf(Bf, BfL); }

  int Len() const { return BfL; }
  bool Empty() const { return BfL == 0; }
  char* GetBf() const { return Bf; }
  void Copy(const TMem& Mem);
  void Copy(const TMemBase& Mem);
  operator TMemBase() const { return TMemBase(Bf, BfL, false); }
  void Load(PSIn& SIn) {
    Clr(); SIn->Load(MxBfL); SIn->Load(BfL);
    Bf = new char[MxBfL = BfL]; SIn->LoadBf(Bf, BfL); }
  void Load(TSIn& SIn) {
    Clr(); SIn.Load(MxBfL); SIn.Load(BfL);
    Bf = new char[MxBfL = BfL]; SIn.LoadBf(Bf, BfL); }

  void Save(TSOut& SOut) const {
    SOut.Save(MxBfL); SOut.Save(BfL); SOut.SaveBf(Bf, BfL);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TMem& operator=(const TMem& Mem);
  TMem& operator=(TMem&& Src);
  TMem& operator=(const TMemBase& Mem);
  TMem& operator=(TMemBase&& Src);
  char* operator()() const {return Bf;}
  TMem& operator+=(const char& Ch);
  TMem& operator+=(const TMem& Mem);
  TMem& operator+=(const TStr& Str);
  TMem& operator+=(const PSIn& SIn);
  char& operator[](const int& ChN) const {
    Assert((0<=ChN)&&(ChN<BfL)); return Bf[ChN];}
  int GetMemUsed() const {return int(sizeof(TMem) + MxBfL);}

  void Gen(const int& _BfL){
    Clr(); Resize(_BfL); BfL=_BfL;}
  void GenZeros(const int& _BfL){
    Clr(false); Resize(_BfL); BfL=_BfL;
    if (BfL > 0) memset(Bf, 0, BfL);}
  void Reserve(const int& _MxBfL, const bool& DoClr = true){
    if (DoClr){ Clr(); } Resize(_MxBfL);}
  void Del(const int& BChN, const int& EChN);
  void Clr(const bool& DoDel=true){
    if (DoDel){if (Bf!=NULL){delete[] Bf;} MxBfL=0; BfL=0; Bf=NULL;}
    else {BfL=0;}}
  void Trunc(const int& _BfL){
    if ((0<=_BfL)&&(_BfL<=BfL)){BfL=_BfL;}}
  void Push(const char& Ch){operator+=(Ch);}
  char Pop(){IAssert(BfL>0); BfL--; return Bf[BfL];}

  bool DoFitStr(const TStr& Str) const;
  //int AddStr(const TStr& Str);
  void AddBf(const void* Bf, const int& BfL);
  TStr GetAsStr(const char& NewNullCh='\0') const;
  // returns a hexadecimal representation of the byte array
  TStr GetHexStr() const;
  static TMem GetFromHex(const TStr& Str);
  PSIn GetSIn() const {
    TMOut MOut(BfL); MOut.SaveBf(Bf, BfL); return MOut.GetSIn();}

  static void LoadMem(const PSIn& SIn, TMem& Mem){
    Mem.Clr(); Mem.Gen(SIn->Len()); SIn->GetBf(Mem.Bf, SIn->Len());}
  void SaveMem(const PSOut& SOut) const {SOut->SaveBf(Bf, Len());}
};

/////////////////////////////////////////////////
// Input-Memory
class TMemIn: public TSIn{
private:
  const char* Bf;
  int BfC, BfL;
public:
  TMemIn(const TMem& _Mem, const int& _BfC=0);
  static PSIn New(const TMem& Mem){
    return PSIn(new TMemIn(Mem));}
  ~TMemIn(){}

  bool Eof(){return BfC==BfL;}
  int Len() const {return BfL-BfC;}
  char GetCh(){Assert(BfC<BfL); return Bf[BfC++];}
  char PeekCh(){Assert(BfC<BfL); return Bf[BfC];}
  int GetBf(const void* LBf, const TSize& LBfL);
  void Reset(){Cs=TCs(); BfC=0;}
  bool GetNextLnBf(TChA& LnChA);

  TStr GetSNm() const;
};

/////////////////////////////////////////////////
// Output-Memory (TMem stored by reference)
class TRefMemOut: public TSOut{
private:
  TMem& Mem;
public:
  TRefMemOut(TMem& _Mem);
  static PSOut New(TMem& Mem){return new TRefMemOut(Mem);}
  ~TRefMemOut(){}

  int PutCh(const char& Ch){Mem += Ch; return Ch;}
  int PutBf(const void* LBf, const TSize& LBfL);
  void Flush(){}

  TStr GetSNm() const;
};

/////////////////////////////////////////////////
// Char-Array
class TChA{
private:
  int MxBfL, BfL;
  char* Bf;
  void Resize(const int& _MxBfL);
public:
  explicit TChA(const int& _MxBfL=256){
    Bf=new char[(MxBfL=_MxBfL)+1]; Bf[BfL=0]=0;}
  TChA(const char* CStr){
    Bf=new char[(MxBfL=BfL=int(strlen(CStr)))+1]; strcpy(Bf, CStr);}
  TChA(const char* CStr, const int& StrLen) : MxBfL(StrLen), BfL(StrLen) {
    Bf=new char[StrLen+1]; strncpy(Bf, CStr, StrLen); Bf[StrLen]=0;}
  TChA(const TChA& ChA){
    Bf=new char[(MxBfL=ChA.MxBfL)+1]; BfL=ChA.BfL; strcpy(Bf, ChA.CStr());}
  TChA(const TStr& Str);
  TChA(const TMem& Mem){
    Bf=new char[(MxBfL=BfL=Mem.Len())+1]; Bf[MxBfL]=0;
    memcpy(CStr(), Mem(), Mem.Len());}
  ~TChA(){delete[] Bf;}
  explicit TChA(TSIn& SIn){
    SIn.Load(MxBfL); SIn.Load(BfL); SIn.Load(Bf, MxBfL, BfL);}
  void Load(TSIn& SIn){ delete[] Bf;
    SIn.Load(MxBfL); SIn.Load(BfL); SIn.Load(Bf, MxBfL, BfL);}
  void Save(TSOut& SOut, const bool& SaveCompact=true) const { //J:
    SOut.Save(SaveCompact?BfL:MxBfL); SOut.Save(BfL); SOut.Save(Bf, BfL);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TChA& operator=(const TChA& ChA);
  TChA& operator=(const TStr& Str);
  TChA& operator=(const char* CStr);
  bool operator==(const TChA& ChA) const {return strcmp(CStr(), ChA.CStr())==0;}
  bool operator==(const char* _CStr) const {return strcmp(CStr(), _CStr)==0;}
  bool operator==(const char& Ch) const {return (BfL==1)&&(Bf[0]==Ch);}
  bool operator!=(const TChA& ChA) const {return strcmp(CStr(), ChA.CStr())!=0;}
  bool operator!=(const char* _CStr) const {return strcmp(CStr(), _CStr)!=0;}
  bool operator!=(const char& Ch) const {return !((BfL==1)&&(Bf[0]==Ch));}
  bool operator<(const TChA& ChA) const {return strcmp(CStr(), ChA.CStr())<0;}

  TChA& operator+=(const TMem& Mem);
  TChA& operator+=(const TChA& ChA);
  TChA& operator+=(const TStr& Str);
  TChA& operator+=(const char* CStr);
  TChA& operator+=(const char& Ch){
  Assert(Ch != 0);
    if (BfL==MxBfL){Resize(BfL+1);}
    Bf[BfL]=Ch; BfL++; Bf[BfL]=0; return *this;}
  char operator[](const int& ChN) const {
    Assert((0<=ChN)&&(ChN<BfL)); return Bf[ChN];}
  char& operator[](const int& ChN){
    Assert((0<=ChN)&&(ChN<BfL)); return Bf[ChN];}
  int GetMemUsed() const {return int(2*sizeof(int)+sizeof(char*)+sizeof(char)*MxBfL);}

  char* operator ()(){return Bf;}
  const char* operator ()() const {return Bf;}
  char* CStr() {return Bf;}
  const char* CStr() const {return Bf;}

  void Clr(){Bf[BfL=0]=0;}
  int Len() const {return BfL;}
  bool Empty() const {return BfL==0;}
  void Ins(const int& BChN, const char* CStr);
  void Del(const int& ChN);
  void DelLastCh(){Pop();}
  void Push(const char& Ch){operator+=(Ch);}
  char Pop(){IAssert(BfL>0); BfL--; char Ch=Bf[BfL]; Bf[BfL]=0; return Ch;}
  void Trunc();
  void Trunc(const int& _BfL){
    if ((0<=_BfL)&&(_BfL<=BfL)){Bf[BfL=_BfL]=0;}}
  void Reverse();

  void AddCh(const char& Ch, const int& MxLen=-1){
    if ((MxLen==-1)||(BfL<MxLen)){operator+=(Ch);}}
  void AddChTo(const char& Ch, const int& ToChN){
    while (Len()<ToChN){AddCh(Ch);}}
  void AddBf(char *NewBf, const int& BfS){
    if ((BfL+BfS+1)>MxBfL){Resize(BfL+BfS+1);}
    strncpy(Bf+BfL,NewBf,BfS); BfL+=BfS; Bf[BfL]=0;}
  void PutCh(const int& ChN, const char& Ch){
    Assert((0<=ChN)&&(ChN<BfL)); Bf[ChN]=Ch;}
  char GetCh(const int& ChN) const {return operator[](ChN);}
  char LastCh() const { Assert(1<=BfL); return Bf[BfL-1]; }
  char LastLastCh() const { Assert(2<=BfL); return Bf[BfL-2]; }

  TChA GetSubStr(const int& BChN, const int& EChN) const;

  int CountCh(const char& Ch, const int& BChN=0) const;
  int SearchCh(const char& Ch, const int& BChN=0) const;
  int SearchChBack(const char& Ch, int BChN=-1) const;
  int SearchStr(const TChA& Str, const int& BChN=0) const;
  int SearchStr(const TStr& Str, const int& BChN=0) const;
  int SearchStr(const char* CStr, const int& BChN=0) const;
  bool IsStrIn(const TStr& Str) const {return SearchStr(Str)!=-1;}
  bool IsPrefix(const char* CStr, const int& BChN=0) const;
  bool IsPrefix(const TStr& Str) const;
  bool IsPrefix(const TChA& Str) const;
  bool IsPrefixLc(const char* CStr, const int& BChN=0) const;
  bool IsPrefixLc(const TStr& Str) const;
  bool IsPrefixLc(const TChA& Str) const;
  bool IsSuffix(const char* CStr) const;
  bool IsSuffix(const TStr& Str) const;
  bool IsSuffix(const TChA& Str) const;

  bool IsChIn(const char& Ch) const {return SearchCh(Ch)!=-1;}
  void ChangeCh(const char& SrcCh, const char& DstCh);
  TChA& ToUc();
  TChA& ToLc();
  TChA& ToTrunc();
  void CompressWs();
  void Swap(const int& ChN1, const int& ChN2);
  void Swap(TChA& ChA);

  int GetPrimHashCd() const;
  int GetSecHashCd() const;

  static void LoadTxt(const PSIn& SIn, TChA& ChA);
  void SaveTxt(const PSOut& SOut) const;
};

/////////////////////////////////////////////////
// Input-Char-Array
class TChAIn: public TSIn{
private:
  const char* Bf;
  int BfC, BfL;
private:
  TChAIn();
  TChAIn(const TChAIn&);
  TChAIn& operator=(const TChAIn&);
public:
  TChAIn(const TChA& ChA, const int& _BfC=0);
  static PSIn New(const TChA& ChA){return PSIn(new TChAIn(ChA));}
  ~TChAIn(){}

  bool Eof(){return BfC==BfL;}
  int Len() const {return BfL-BfC;}
  char GetCh(){Assert(BfC<BfL); return Bf[BfC++];}
  char PeekCh(){Assert(BfC<BfL); return Bf[BfC];}
  int GetBf(const void* LBf, const TSize& LBfL);
  void Reset(){Cs=TCs(); BfC=0;}
  bool GetNextLnBf(TChA& LnChA);
  TStr GetSNm() const;
};

/////////////////////////////////////////////////
/// String.
///
/// Multithreaded operation.
///
/// Operations that modify string content are not thread safe. If multiple
/// threads might write to the same string, access needs to be serialized
/// via locks. A single writer and multiple readers also require locking.
/// There is no need to lock multiple read operations in multithreaded
/// environments.
///
/// Small example:
///     int main() {
///         TStr Str0("abc"); // char* constructor
///         Str Str1("def"); // char* constructor
///         TStr Str2 = Str1; // copy constructor
///         Str2 = Str0;      // copy assignment
///         TStr& Str3 = Str2; // no copying
///         Str0, Str and Str2 call destructors, Str3 doesnt call destructor (that's as it should be)
///         return 0;
///     }

class TStr;
template <class TVal, class TSizeTy> class TVec;
typedef TVec<TStr, int> TStrV;

class TStr{
public:
  typedef const char* TIter;  //!< Random access iterator.
private:
  /// Used to construct empty strings ("") to be returned by CStr()
  const static char EmptyStr;
  /// String
  char* Inner;

  /// Wraps the char pointer with a new string. The char pointer is NOT
  /// copied and the new string becomes responsible for deleting it.
  static TStr WrapCStr(char* CStr);

public:
  /// Empty String Constructor
  TStr(): Inner(nullptr) {}
  /// C-String constructor
  TStr(const char* CStr);
  /// 1 char constructor
  explicit TStr(const char& Ch);
  /// copy constructor
  TStr(const TStr& Str);
  /// move constructor
  TStr(TStr&& Str);
  /// TChA constructor (char-array class)
  TStr(const TChA& ChA);
  /// TMem constructor
  TStr(const TMem& Mem);
  /// TSStr constructor
  TStr(const TSStr& SStr); // KILL
  /// Stream (file) reading constructor
  explicit TStr(const PSIn& SIn);

  /// We only delete when not empty
  ~TStr() { Clr(); }

  /// Returns an iterator pointing to the first element in the string.
  TIter BegI() const { return Empty() ? &EmptyStr : Inner; }
  /// Returns an iterator pointing to the first element in the string (used by C++11)
  TIter begin() const { return Empty() ? &EmptyStr : Inner; }
  /// Returns an iterator referring to the past-the-end element in the string.
  TIter EndI() const { return Empty() ? &EmptyStr : Inner + Len(); }
  /// Returns an iterator referring to the past-the-end element in the string (used by C++11))
  TIter end() const { return Empty() ? &EmptyStr : Inner + Len(); }
  /// Returns an iterator an element at position \c ValN.
  TIter GetI(const int ValN) const { return Empty() ? &EmptyStr : Inner + ValN; }

  /// Deserialize TStr from stream, when IsSmall, the string is saved as CStr,
  /// otherwise the format is first the length and then the data including last \0
  explicit TStr(TSIn& SIn, const bool& IsSmall = false);
  // Left for compatibility reasons, best if we can remove it at some point
  void Load(TSIn& SIn, const bool& IsSmall = false);
  /// Serialize TStr to stream, when IsSmall, the string is saved as CStr,
  /// otherwise the format is first the length and then the data including last \0
  void Save(TSOut& SOut, const bool& IsSmall = false) const;
   /// Deserialize from XML File
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  /// Serialize to XML File
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  /// Assigment operator TStr = TStr
  TStr& operator=(const TStr& Str);
  /// Move assigment operator TStr = TStr
  TStr& operator=(TStr&& Str);
  /// Assigment operator TStr = TChA
  TStr& operator=(const TChA& ChA);
  /// Assigment operator TStr = char* (C-String)
  TStr& operator=(const char* CStr);

  /// Concatenates and assigns (not thread safe)
  TStr& operator+=(const TStr& Str) { *this = (*this + Str); return *this; } ;
  /// Concatenates and assigns (not thread safe)
  TStr& operator+=(const char* _CStr) { *this = (*this + _CStr); return *this; } ;
  /// Concatenates and assigns (not thread safe)
  TStr& operator+=(const char Ch) { *this = (*this + Ch); return *this; };

  /// Boolean comparison TStr == char*
  bool operator==(const char* _CStr) const;
  /// Boolean comparison TStr == TStr
  bool operator==(const TStr& Str) const { return operator==(Str.CStr()); }
  // TStr != TStr
  bool operator!=(const TStr& Str) const { return !operator==(Str); }
  // TStr != C-String
  bool operator!=(const char* CStr) const { return !operator==(CStr); }
  /// < (is less than comparison) TStr < TStr
  bool operator<(const TStr& Str) const;

  /// Indexing operator, returns character at position ChN
  char operator[](const int& ChN) const { return GetCh(ChN); }
  /// Indexing operator, returns character at position ChN by reference
  char& operator[](const int& ChN);

  /// Get the inner C-String
  const char* CStr() const { return Empty() ? &EmptyStr : Inner; }
  /// Return a COPY of the string as a C String (char array)
  char* CloneCStr() const;
  /// Set character to given value (not thread safe)
  void PutCh(const int& ChN, const char& Ch);
  /// Get character at position ChN
  char GetCh(const int& ChN) const;
  /// Get last character in string (before null terminator)
  char LastCh() const {return GetCh(Len()-1);}
  /// Get String Length (null terminator not included)
  int Len() const { return Empty() ? 0 : (int)strlen(Inner); }
  /// Check if this is an empty string
  bool Empty() const;
  /// Deletes the char pointer if it is not nullptr. (not thread safe)
  void Clr();
  /// returns a reference to this string
  const TStr& GetStr() const { return *this; }
  /// Memory used by this String object
  int GetMemUsed() const;

  /// Case insensitive comparison
  static int CmpI(const char* p, const char* r);
  /// Case insensitive comparison
  int CmpI(const TStr& Str) const {return CmpI(CStr(), Str.CStr()); }
  /// Case insensitive equality
  bool EqI(const TStr& Str) const {return CmpI(CStr(), Str.CStr()) == 0; }
  /// Is upper-case?
  bool IsUc() const;
  /// Converts this string to all uppercase (not thread safe)
  TStr& ToUc();
  /// Returns a new string converted to uppercase
  TStr GetUc() const;
  /// Is lower-case?
  bool IsLc() const;
  /// Converts this string to all lowercase (not thread safe)
  TStr& ToLc();
  /// Returns new string converted to lowercase
  TStr GetLc() const;
  /// Converts this string to capitalized (first char is uppercase, rest lowercase) (not thread safe)
  TStr& ToCap();
  /// Returns string as capitalized (first char is uppercase, rest lowercase)
  TStr GetCap() const;

  /// Replaces string with truncated (remove whitespace at start and end) (not thread safe)
  TStr& ToTrunc();
  /// Returns truncated string (remove whitespace at start and end)
  TStr GetTrunc() const;
  /// Replaces each char with two chars with its hex-code (not thread safe)
  TStr& ToHex();
  /// Get string with each char replaced with two chars with its hex-code
  TStr GetHex() const;
  /// Does inverse of ToHex (not thread safe)
  TStr& FromHex();
  /// Does inverse of GetHex
  TStr GetFromHex() const;

  /// Get substring from in interval [BchN, EchN]
  TStr GetSubStr(const int& BChN, const int& EChN) const;
  /// Get substring from BchN (includive) to the end of the string
  TStr GetSubStr(const int& BChN) const { return GetSubStr(BChN, Len()-1); }
  /// safe version for getting substring from BchN to EchN
  TStr GetSubStrSafe(const int& BChN, const int& EChN) const;
  /// safe version for getting substring from BchN to EchN
  TStr GetSubStrSafe(const int& BChN) const { return GetSubStrSafe(BChN, Len() - 1); }
  /// Insert a string Str into this string starting position BchN (not thread safe)
  void InsStr(const int& BChN, const TStr& Str);
  /// Delete all the occurrences of char Ch (not thread safe)
  void DelChAll(const char& Ch);
  /// Delete substring from BChN to EChN (not thread safe)
  void DelSubStr(const int& BChN, const int& EChN);
  /// Delete first occurrences of substring Str (not thread safe)
  bool DelStr(const TStr& Str);
  /// Delete all occurrences of substring Str (single pass) (not thread safe)
  int DelStrAll(const TStr& Str);

  /// Get substring from beginning till the character before first occurrence of SplitCh
  TStr LeftOf(const char& SplitCh) const;
  /// Get substring from beginning till the character before last occurrence of SplitCh
  TStr LeftOfLast(const char& SplitCh) const;
  /// Get substring from the character after first occurrence of SplitCh till the end
  TStr RightOf(const char& SplitCh) const;
  /// Get substring from the character after last occurrence of SplitCh till the end
  TStr RightOfLast(const char& SplitCh) const;
  /// Remove the StartStr if it occurs at the beginning of the string
  TStr TrimLeft(const TStr& StartStr) const { return StartsWith(StartStr) ? GetSubStrSafe(StartStr.Len()) : TStr(*this); }
  /// Remove the EndStr if it occurs at the end of the string
  TStr TrimRight(const TStr& EndStr) const { return EndsWith(EndStr) ? GetSubStrSafe(0, Len() - EndStr.Len() - 1) : TStr(*this); }

  /// Puts the contents to the left of LeftOfChN (exclusive) into LStr and the
  /// contents on the right of RightOfChN into RStr (exclusive)
  void SplitLeftOfRightOf(TStr& LStr, const int& LeftOfChN, const int& RightOfChN, TStr& RStr) const;
  /// Split on the index, return Pair of Left/Right strings, omits the target index
  void SplitOnChN(TStr& LStr, const int& ChN, TStr& RStr) const;
  /// Split on first occurrence of SplitCh, return Pair of Left/Right strings, omits the target character
  /// if the character is not found the whole string is returned as the left side
  void SplitOnCh(TStr& LStr, const char& SplitCh, TStr& RStr) const;
  /// Splits on the first occurrence of the target string
  /// if the target string is not found the whole string is returned as the left side
  void SplitOnStr(TStr& LStr, const TStr& SplitStr, TStr& RStr) const;
  /// Split on last occurrence of SplitCh, return Pair of Left/Right strings
  /// if the character is not found the whole string is returned as the right side
  void SplitOnLastCh(TStr& LStr, const char& SplitCh, TStr& RStr) const;
  /// Split on all occurrences of SplitCh, write to StrV, optionally don't create empy strings (default true)
  void SplitOnAllCh(const char& SplitCh, TStrV& StrV, const bool& SkipEmpty=true) const;
  /// Split on all occurrences of any char in SplitChStr, optionally don't create empy strings (default true)
  void SplitOnAllAnyCh(const TStr& SplitChStr, TStrV& StrV, const bool& SkipEmpty=true) const;
  /// Split on the occurrences of any string in StrV
  void SplitOnWs(TStrV& StrV) const;
  /// Split on the occurrences of any non alphanumeric character
  void SplitOnNonAlNum(TStrV& StrV) const;
  /// Split on all the occurrences of SplitStr, doesn't remove the split string (splitting aabbaacc on aa results in aa, bbaa, cc
  void SplitOnStr(const TStr& SplitStr, TStrV& StrV) const;

  /// Get substring from beginning till character positioned at EChN (exclusive).
  /// In case EChN is negative, it counts from the back of the string.
  TStr Left(const int& EChN) const;
  /// Get substring from character positioned at BChN (inclusive) till the end.
  /// In case BChN is negative, it counts from the back of the string.
  TStr Right(const int& BChN) const {return BChN>=0 ? GetSubStr(BChN, Len()-1) : GetSubStr(Len()+BChN, Len()-1);}

  /// Counts occurrences of a character between [BChN, end]
  int CountCh(const char& Ch, const int& BChN=0) const;
  /// Returns the position of the first occurrence of a character between [BChN, end]
  int SearchCh(const char& Ch, const int& BChN=0) const;
  /// Returns the position of the last occurrence of a character between [BChN, end]
  int SearchChBack(const char& Ch, int BChN=-1) const;
  /// Returns the position of the first occurrence of a (sub)string between [BChN, end]
  int SearchStr(const TStr& Str, const int& BChN=0) const;
  /// Returns true if character occurs in string
  bool IsChIn(const char& Ch) const {return SearchCh(Ch)!=-1;}
  /// Returns true if (sub)string occurs in string
  bool IsStrIn(const TStr& Str) const {return SearchStr(Str)!=-1;}
  /// Returns true if this string starts with the prefix c-string
  bool StartsWith(const char *Str) const;
  /// Returns true if this string starts with the prefix string
  bool StartsWith(const TStr& Str) const { return StartsWith(Str.CStr()); }
  /// Returns true if this string ends with the sufix c-string
  bool EndsWith(const char *Str) const;
  /// Returns true if this string ends with the sufix string
  bool EndsWith(const TStr& Str) const { return EndsWith(Str.CStr()); }

  /// Replaces first occurrence of SrcCh character with DstCh. Start search at BChN. (not thread safe)
  int ChangeCh(const char& SrcCh, const char& DstCh, const int& BChN=0);
  /// Replaces all occurrences of SrcCh character with DstCh (not thread safe)
  int ChangeChAll(const char& SrcCh, const char& DstCh);
  /// Replace first occurrence of ScrStr string with DstStr string. Start search at BChN. (not thread safe)
  int ChangeStr(const TStr& SrcStr, const TStr& DstStr, const int& BChN=0);
  /// Replace all occurrences of ScrStr string with DstStr string (not thread safe)
  int ChangeStrAll(const TStr& SrcStr, const TStr& DstStr);
  /// Returns a String with the order of the characters in this String Reversed
  TStr Reverse() const;

  int GetPrimHashCd() const;
  int GetSecHashCd() const;
  int GetHashTrick() const; // @TODO change this to uint32_t

  /// Return true if string is 'T' or 'F'. Return true or false accordingly in Val
  bool IsBool(bool& Val) const;

  // integer
  bool IsInt(const bool& Check, const int& MnVal, const int& MxVal, int& Val) const;
  bool IsInt(int& Val) const {return IsInt(false, 0, 0, Val);}
  bool IsInt() const {int Val; return IsInt(false, 0, 0, Val);}
  int GetInt() const {int Val; IAssertR(IsInt(false, 0, 0, Val), *this); return Val;}
  int GetInt(const int& DfVal) const {
    int Val; if (IsInt(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  // unsigned integer
  bool IsUInt(const bool& Check, const uint& MnVal, const uint& MxVal, uint& Val) const;
  bool IsUInt(uint& Val) const {return IsUInt(false, 0, 0, Val);}
  bool IsUInt() const {uint Val; return IsUInt(false, 0, 0, Val);}
  uint GetUInt() const {uint Val; IAssert(IsUInt(false, 0, 0, Val)); return Val;}
  uint GetUInt(const uint& DfVal) const {
    uint Val; if (IsUInt(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  // 64-bit integer
  bool IsInt64(const bool& Check, const int64& MnVal, const int64& MxVal, int64& Val) const;
  bool IsInt64(int64& Val) const {return IsInt64(false, 0, 0, Val);}
  bool IsInt64() const {int64 Val; return IsInt64(false, 0, 0, Val);}
  int64 GetInt64() const {
    int64 Val; IAssert(IsInt64(false, 0, 0, Val)); return Val;}
  int64 GetInt64(const int64& DfVal) const {
    int64 Val; if (IsInt64(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  // unsigned 64-bit integer
  bool IsUInt64(const bool& Check, const uint64& MnVal, const uint64& MxVal, uint64& Val) const;
  bool IsUInt64(uint64& Val) const {return IsUInt64(false, 0, 0, Val);}
  bool IsUInt64() const {uint64 Val; return IsUInt64(false, 0, 0, Val);}
  uint64 GetUInt64() const {
    uint64 Val; IAssert(IsUInt64(false, 0, 0, Val)); return Val;}
  uint64 GetUInt64(const uint64& DfVal) const {
    uint64 Val; if (IsUInt64(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  bool IsHexInt(const bool& Check, const int& MnVal, const int& MxVal, int& Val) const;
  bool IsHexInt(int& Val) const {return IsHexInt(false, 0, 0, Val);}
  bool IsHexInt() const {int Val; return IsHexInt(false, 0, 0, Val);}
  int GetHexInt() const {
    int Val; IAssert(IsHexInt(false, 0, 0, Val)); return Val;}
  int GetHexInt(const int& DfVal) const {
    int Val; if (IsHexInt(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  bool IsHexInt64(const bool& Check, const int64& MnVal, const int64& MxVal, int64& Val) const;
  bool IsHexInt64(int64& Val) const {return IsHexInt64(false, 0, 0, Val);}
  bool IsHexInt64() const {int64 Val; return IsHexInt64(false, 0, 0, Val);}
  int64 GetHexInt64() const {
    int64 Val; IAssert(IsHexInt64(false, 0, 0, Val)); return Val;}
  int64 GetHexInt64(const int64& DfVal) const {
    int64 Val; if (IsHexInt64(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  bool IsFlt(const bool& Check, const double& MnVal, const double& MxVal,
   double& Val, const char& DecDelimCh='.') const;
  bool IsFlt(double& Val) const {return IsFlt(false, 0, 0, Val);}
  bool IsFlt() const {double Val; return IsFlt(false, 0, 0, Val);}
  double GetFlt() const {
    double Val; IAssert(IsFlt(false, 0, 0, Val)); return Val;}
  double GetFlt(const double& DfVal) const {
    double Val; if (IsFlt(false, 0, 0, Val)){return Val;} else {return DfVal;}}

  /*
   * Word matching methods
   */
  bool IsWord(const bool& WsPrefixP=true, const bool& FirstUcAllowedP=true) const;
  bool IsWs() const;

  bool IsWcMatch(
   const int& StrBChN, const TStr& WcStr, const int& WcStrBChN, TStrV& StarStrV,
   const char& StarCh='*', const char& QuestCh='?') const;
  bool IsWcMatch(
   const TStr& WcStr, TStrV& StarStrV,
   const char& StarCh='*', const char& QuestCh='?') const;
  bool IsWcMatch(const TStr& WcStr, const char& StarCh, const char& QuestCh) const;
  bool IsWcMatch(const TStr& WcStr, const int& StarStrN, TStr& StarStr) const;
  bool IsWcMatch(const TStr& WcStr) const;
  TStr GetWcMatch(const TStr& WcStr, const int& StarStrN=0) const;

  /*
   * Path Handling Functions
   */
  TStr GetFPath() const;
  TStr GetFBase() const;
  TStr GetFMid() const;
  TStr GetFExt() const;

  /*
   * Static Path Handling Functions
   */
  static TStr GetNrFPath(const TStr& FPath);
  static TStr GetNrFMid(const TStr& FMid);
  static TStr GetNrFExt(const TStr& FExt);
  static TStr GetNrNumFExt(const int& FExtN, const int& MinLen = 3);
  static TStr GetNrFNm(const TStr& FNm);
  static TStr GetNrAbsFPath(const TStr& FPath, const TStr& BaseFPath=TStr());
  static bool IsAbsFPath(const TStr& FPath);
  static TStr PutFExt(const TStr& FNm, const TStr& FExt);
  static TStr PutFExtIfEmpty(const TStr& FNm, const TStr& FExt);
  static TStr PutFBase(const TStr& FNm, const TStr& FBase);
  static TStr PutFBaseIfEmpty(const TStr& FNm, const TStr& FBase);
  static TStr AddToFMid(const TStr& FNm, const TStr& ExtFMid);
  static TStr GetNumFNm(const TStr& FNm, const int& Num);
  static TStr GetFNmStr(const TStr& Str, const bool& AlNumOnlyP=true);

  /*
   * Static Save/Load Text File Functions
   */
  static TStr LoadTxt(const PSIn& SIn){
    return TStr(SIn);}
  static TStr LoadTxt(const TStr& FNm){
    PSIn SIn=TFIn::New(FNm); return LoadTxt(SIn);}
  void SaveTxt(const PSOut& SOut) const {
    SOut->SaveBf(CStr(), Len());}
  void SaveTxt(const TStr& FNm) const {
    PSOut SOut=TFOut::New(FNm); SaveTxt(SOut);}

  /*
   * Static methods for FmtStr
   */
  static TStr GetStr(const TStr& Str, const char* FmtStr);
  static TStr GetStr(const TStr& Str, const TStr& FmtStr) { return GetStr(Str, FmtStr.CStr()); }
  static TStr GetStr(const TStrV& StrV, const TStr& DelimiterStr);
  static TStr Fmt(const char *FmtStr, ...);
  static TStr GetSpaceStr(const int& Spaces);

  /// Concatenates the first string parameter with the char array
  friend TStr operator+(const TStr& LStr, const char* RCStr);
  /// Concatenates the two strings
  friend TStr operator+(const TStr& LStr, const TStr& RStr);
  /// Concatenates the first string parameter with single char
  friend TStr operator+(const TStr& LStr, const char Ch);



  /// Base64-encode given buffer and return resulting string
  static TStr Base64Encode(const void* Bf, const int BfL);
  /// Base64-encode given buffer and return resulting string
  static TStr Base64Encode(const TMemBase& Mem) { return Base64Encode(Mem.GetBf(), Mem.Len()); }
  /// Base64-encode given buffer and return resulting string
  static TStr Base64Encode(const TMem& Mem) { return Base64Encode(Mem.GetBf(), Mem.Len()); }
  /// Base64-decode given string and fill this TMem object
  static void Base64Decode(const TStr& In, TMem& Mem);

private:

  /// Static mapping utility for base64 characters
  static const TStr base64_chars;

  /// This method checks if given character belongs to base64 range
  static inline bool is_base64(unsigned char c) { return (isalnum(c) || (c == '+') || (c == '/')); };


  /// internal method used to check if the string stored in TChRet is an unsigned integer
  /// IMPORTANT: TChRet must be initialized (GetCh() must be called at least once)
  static bool IsUInt(TChRet& Ch, const bool& Check, const uint& MnVal, const uint& MxVal, uint& Val);
  /// internal method used to check if the string stored in TChRet is a 64-bit unsigned integer
  /// IMPORTANT: TChRet must be initialized (GetCh() must be called at least once)
  static bool IsUInt64(TChRet& Ch, const bool& Check, const uint64& MnVal, const uint64& MxVal, uint64& Val);
};

/////////////////////////////////////////////////
// Input-String
class TStrIn: public TSIn{
private:
  const bool OwnP;
  const char* Bf;
  int BfC, BfL;
private:
  TStrIn();
  TStrIn(const TStrIn&);
  TStrIn& operator=(const TStrIn&);
public:
  TStrIn(const TStr& Str, const bool& MakeCopyP = true);
  static PSIn New(const TStr& Str, const bool& MakeCopyP = true);
  ~TStrIn(){ if (OwnP) { delete[] Bf; }}

  bool Eof(){return BfC==BfL;}
  int Len() const {return BfL-BfC;}
  char GetCh(){Assert(BfC<BfL); return Bf[BfC++];}
  char PeekCh(){Assert(BfC<BfL); return Bf[BfC];}
  int GetBf(const void* LBf, const TSize& LBfL);
  void Reset(){Cs=TCs(); BfC=0;}
  bool GetNextLnBf(TChA& LnChA);
  TStr GetSNm() const;
};

/////////////////////////////////////////////////
// Double-String
class TDbStr{
public:
  TStr Str1;
  TStr Str2;
public:
  TDbStr(): Str1(), Str2(){}
  TDbStr(const TDbStr& DbStr): Str1(DbStr.Str1), Str2(DbStr.Str2){}
  TDbStr(const TStr& _Str1): Str1(_Str1), Str2(){}
  TDbStr(const TStr& _Str1, const TStr& _Str2): Str1(_Str1), Str2(_Str2){}
  explicit TDbStr(TSIn& SIn): Str1(SIn), Str2(SIn){}
  void Save(TSOut& SOut) const {Str1.Save(SOut); Str2.Save(SOut);}

  TDbStr& operator=(const TDbStr& DbStr){
    if (this!=&DbStr){Str1=DbStr.Str1; Str2=DbStr.Str2;} return *this;}
  bool operator==(const TDbStr& DbStr) const {
    return (Str1==DbStr.Str1)&&(Str2==DbStr.Str2);}
  bool operator<(const TDbStr& DbStr) const {
    return (Str1<DbStr.Str1)||((Str1==DbStr.Str1)&&(Str2<DbStr.Str2));}

  TStr GetStr(const TStr& MidStr=TStr()) const {
    if (Filled()){return Str1+MidStr+Str2;} else {return Str1+Str2;}}
  int GetPrimHashCd() const {
    return Str1.GetPrimHashCd()+Str2.GetPrimHashCd();}
  int GetSecHashCd() const {
    return Str1.GetSecHashCd()+Str2.GetSecHashCd();}

  bool Empty() const {return (Str1.Empty())&&(Str2.Empty());}
  bool Filled() const {return (!Str2.Empty())&&(!Str1.Empty());}
};

/////////////////////////////////////////////////
// String-Pool
ClassTP(TStrPool, PStrPool)//{
private:
  uint MxBfL, BfL, GrowBy;
  char *Bf;
private:
  void Resize(const uint& _MxBfL);
public:
  TStrPool(const uint& MxBfLen = 0, const uint& _GrowBy = 16*1024*1024);
  TStrPool(TSIn& SIn, bool LoadCompact = true);
  TStrPool(const TStrPool& Pool) : MxBfL(Pool.MxBfL), BfL(Pool.BfL), GrowBy(Pool.GrowBy) {
    Bf = (char *) malloc(Pool.MxBfL); IAssertR(Bf, TStr::Fmt("Can not resize buffer to %u bytes. [Program failed to allocate more memory. Solution: Get a bigger machine.]", MxBfL).CStr()); memcpy(Bf, Pool.Bf, Pool.BfL); }
  ~TStrPool() { if (Bf) free(Bf); else IAssertR(MxBfL == 0, TStr::Fmt("size: %u, expected size: 0", MxBfL).CStr());  Bf = 0; MxBfL = 0; BfL = 0; }

  static PStrPool New(const uint& _MxBfLen = 0, const uint& _GrowBy = 16*1024*1024) { return PStrPool(new TStrPool(_MxBfLen, _GrowBy)); }
  static PStrPool New(TSIn& SIn) { return new TStrPool(SIn); }
  static PStrPool New(const TStr& fileName) { PSIn SIn = TFIn::New(fileName); return new TStrPool(*SIn); }
  static PStrPool Load(TSIn& SIn, bool LoadCompacted = true) { return PStrPool(new TStrPool(SIn, LoadCompacted)); }
  void Save(TSOut& SOut) const;
  void Save(const TStr& FNm){PSOut SOut=TFOut::New(FNm); Save(*SOut);}

  uint Len() const { return BfL; }
  uint Size() const { return MxBfL; }
  bool Empty() const { return ! Len(); }
  char* operator () () const { return Bf; }
  TStrPool& operator = (const TStrPool& Pool);

  uint AddStr(const char *Str, const uint& Len);
  uint AddStr(const char *Str) { return AddStr(Str, uint(strlen(Str)) + 1); }
  uint AddStr(const TStr& Str) { return AddStr(Str.CStr(), Str.Len() + 1); }

  TStr GetStr(const uint& Offset) const { Assert(Offset < BfL);
    if (Offset == 0) return TStr(); else return TStr(Bf + Offset); }
  const char *GetCStr(const uint& Offset) const { Assert(Offset < BfL);
    if (Offset == 0) return TStr().CStr(); else return Bf + Offset; }

  // Clr() removes the empty string at the start.
  // Call AddStr("") after Clr(), if you want to use the pool again.
  void Clr(bool DoDel = false) { BfL = 0; if (DoDel && Bf) { free(Bf); Bf = 0; MxBfL = 0; } }
  int Cmp(const uint& Offset, const char *Str) const { Assert(Offset < BfL);
    if (Offset != 0) return strcmp(Bf + Offset, Str); else return strcmp("", Str); }

  static int GetPrimHashCd(const char *CStr);
  static int GetSecHashCd(const char *CStr);
  int GetPrimHashCd(const uint& Offset) { Assert(Offset < BfL);
    if (Offset != 0) return GetPrimHashCd(Bf + Offset); else return GetPrimHashCd(""); }
  int GetSecHashCd(const uint& Offset) { Assert(Offset < BfL);
    if (Offset != 0) return GetSecHashCd(Bf + Offset); else return GetSecHashCd(""); }
  int GetMemUsed() const { return (int) (MxBfL + 3*sizeof(uint)); }
};

/////////////////////////////////////////////////
// String-Pool-64bit
ClassTP(TStrPool64, PStrPool64)//{
private:
  ::TSize MxBfL, BfL, GrowBy;
  char *Bf;
private:
  void Resize(const ::TSize& _MxBfL);
public:
  TStrPool64(::TSize _MxBfL = 0, ::TSize _GrowBy = 16*1024*1024);
  TStrPool64(const TStrPool64& StrPool);
  TStrPool64(TSIn& SIn, bool LoadCompact = true);
  ~TStrPool64() { Clr(true); }
  void Save(TSOut& SOut) const;

  static PStrPool64 New(::TSize MxBfL = 0, ::TSize GrowBy = 16*1024*1024) {
      return PStrPool64(new TStrPool64(MxBfL, GrowBy)); }
  static PStrPool64 Load(TSIn& SIn, bool LoadCompact = true) {
      return PStrPool64(new TStrPool64(SIn, LoadCompact)); }

  TStrPool64& operator=(const TStrPool64& StrPool);

  uint64 GetMemUsed() const { return 3*sizeof(::TSize) + uint64(MxBfL); }

  bool Empty() const { return (BfL == 0); }
  uint64 Len() const {return BfL;}
  uint64 Reserved() const { return MxBfL; }
  void Clr(bool DoDel = false);
  int Cmp(uint64 Offset, const char *Str) const { Assert(Offset < BfL);
    if (Offset != 0) return strcmp(Bf + Offset, Str); else return strcmp("", Str); }

  uint64 AddStr(const TStr& Str);
  TStr GetStr(const uint64& StrId) const;
};

/////////////////////////////////////////////////
// Number Base Template
template <class Base> class TNum{
public:
  Base Val;
  TNum() : Val(0){}
  TNum(const Base& _Val) : Val(_Val){}
  operator Base() const { return Val; }
  explicit TNum(TSIn& SIn){ SIn.Load(Val); }
  void Load(TSIn& SIn){ SIn.Load(Val); }
  void Save(TSOut& SOut) const { SOut.Save(Val); }

  TNum& operator=(const TNum& Other){ Val = Other.Val; return *this; }
  TNum& operator=(const Base& _Val){ Val = _Val; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix
  Base& operator()() { return Val; }

  int GetMemUsed() const { return sizeof(TNum); }
};

/////////////////////////////////////////////////
// Complex double
template<>
class TNum<std::complex<double>>{
public:
  std::complex<double> Val;
  TNum() : Val(0){}
  TNum(std::complex<double>& _Val) : Val(_Val){}
  operator std::complex<double>() const { return Val; }
  explicit TNum(TSIn& SIn){ double real = 0.0, imag = 0.0; SIn.Load(real); SIn.Load(imag); Val = std::complex<double>(real, imag); }
  void Load(TSIn& SIn){ double real = 0.0, imag = 0.0; SIn.Load(real); SIn.Load(imag); Val = std::complex<double>(real, imag); }
  void Save(TSOut& SOut) const { SOut.Save(Val.real()); SOut.Save(Val.imag()); }

  TNum& operator=(const TNum& Other){ Val = Other.Val; return *this; }
  TNum& operator=(const std::complex<double>& _Val){ Val = _Val; return *this; }
  TNum& operator++(){ Val = Val + std::complex<double>(1.0); return *this; } // prefix
  TNum& operator--(){ Val = Val - std::complex<double>(1.0); return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val = Val + std::complex<double>(1.0); return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val = Val - std::complex<double>(1.0); return oldVal; } // postfix
  std::complex<double>& operator()() { return Val; }
  int GetMemUsed() const { return sizeof(TNum); }
};

/////////////////////////////////////////////////
// Complex float
template<>
class TNum<std::complex<float>>{
public:
  std::complex<float> Val;
  TNum() : Val(0){}
  TNum(std::complex<float>& _Val) : Val(_Val){}
  operator std::complex<float>() const { return Val; }
  explicit TNum(TSIn& SIn){ float real = 0.0f; float imag = 0.0f; SIn.Load(real); SIn.Load(imag); Val = std::complex<float>(real, imag); }
  void Load(TSIn& SIn){ float real = 0.0f; float imag = 0.0f; SIn.Load(real); SIn.Load(imag); Val = std::complex<float>(real, imag); }
  void Save(TSOut& SOut) const { SOut.Save(Val.real()); SOut.Save(Val.imag()); }

  TNum& operator=(const TNum& Other){ Val = Other.Val; return *this; }
  TNum& operator=(const std::complex<float>& _Val){ Val = _Val; return *this; }
  TNum& operator++(){ Val = Val + std::complex<float>(1.0f); return *this; } // prefix
  TNum& operator--(){ Val = Val - std::complex<float>(1.0f); return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val = Val + std::complex<float>(1.0f); return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val = Val - std::complex<float>(1.0f); return oldVal; } // postfix
  std::complex<float>& operator()() { return Val; }
  int GetMemUsed() const { return sizeof(TNum); }
};

/////////////////////////////////////////////////
// Void
class TVoid{
public:
  TVoid(){}
  TVoid(TSIn&){}
  void Save(TSOut&) const {}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TVoid& operator=(const TVoid&){return *this;}
  bool operator==(const TVoid&) const {return true;}
  bool operator<(const TVoid&) const {Fail; return false;}
  int GetMemUsed() const {return sizeof(TVoid);}
};

/////////////////////////////////////////////////
// Boolean
class TBool{
public:
  bool Val;
public:
  static const bool Mn;
  static const bool Mx;
  static const int Vals;
  static TRnd Rnd;

  static const TStr FalseStr;
  static const TStr TrueStr;
  static const TStr NStr;
  static const TStr YStr;
  static const TStr NoStr;
  static const TStr YesStr;

  TBool(): Val(false){}
  TBool(const bool& _Val): Val(_Val){}
  operator bool() const {return Val;}
  explicit TBool(TSIn& SIn){SIn.Load(Val);}
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TBool& operator=(const TBool& Bool){Val=Bool.Val; return *this;}
  TBool& operator+=(const TBool& Bool) {Val |= Bool.Val; return *this;}
  bool operator==(const TBool& Bool) const {return Val==Bool.Val;}
  bool operator<(const TBool& Bool) const {//return Val<Bool.Val;
    return (Val==false)&&(Bool.Val==true);}
  bool operator()() const {return Val;}
  int GetMemUsed() const {return sizeof(TBool);}

  int GetPrimHashCd() const {return Val;}
  int GetSecHashCd() const {return Val;}

  TStr GetStr() { return GetStr(Val); }

  static bool GetRnd(){return Rnd.GetUniDevInt(2)==1;}

  static TStr GetStr(const bool& Val){
    if (Val){return TrueStr;} else {return FalseStr;}}
  static TStr GetStr(const TBool& Bool){
    return GetStr(Bool.Val);}
  static TStr GetYNStr(const bool& Val){
    if (Val){return YStr;} else {return NStr;}}
  static TStr GetYesNoStr(const bool& Val){
    if (Val){return YesStr;} else {return NoStr;}}
  static TStr Get01Str(const bool& Val){
    if (Val){return "1";} else {return "0";}}
  static bool IsValStr(const TStr& Str);
  static bool GetValFromStr(const TStr& Str);
  static bool GetValFromStr(const TStr& Str, const bool& DfVal);
};

/////////////////////////////////////////////////
// Char
class TCh{
public:
  char Val;
public:
  static const char Mn;
  static const char Mx;
  static const int Vals;

  static const char NullCh;
  static const char TabCh;
  static const char LfCh;
  static const char CrCh;
  static const char EofCh;
  static const char HashCh;

  TCh(): Val(TCh::NullCh){}
  TCh(const char& _Val): Val(_Val){}
  operator char() const {return Val;}
  explicit TCh(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TCh& operator=(const TCh& Ch){Val=Ch.Val; return *this;}
  bool operator==(const TCh& Ch) const {return Val==Ch.Val;}
  bool operator<(const TCh& Ch) const {return Val<Ch.Val;}
  char operator()() const {return Val;}
  int GetMemUsed() const {return sizeof(TCh);}

  int GetPrimHashCd() const {return Val;}
  int GetSecHashCd() const {return Val;}

  static bool IsWs(const char& Ch){
    return (Ch==' ')||(Ch==TabCh)||(Ch==CrCh)||(Ch==LfCh);}
  static bool IsAlpha(const char& Ch){
    return (('A'<=Ch)&&(Ch<='Z'))||(('a'<=Ch)&&(Ch<='z'));}
  static bool IsNum(const char& Ch){return ('0'<=Ch)&&(Ch<='9');}
  static bool IsAlNum(const char& Ch){return IsAlpha(Ch)||IsNum(Ch);}
  static int GetNum(const char& Ch){Assert(IsNum(Ch)); return Ch-'0';}
  static bool IsHex(const char& Ch){return
    (('0'<=Ch)&&(Ch<='9'))||(('A'<=Ch)&&(Ch<='F'))||(('a'<=Ch)&&(Ch<='f'));}
  static int GetHex(const char& Ch){
    if (('0'<=Ch)&&(Ch<='9')){return Ch-'0';}
    else if (('A'<=Ch)&&(Ch<='F')){return Ch-'A'+10;}
    else if (('a'<=Ch)&&(Ch<='f')){return Ch-'a'+10;}
    else Fail; return 0;}
  static char GetHexCh(const int& Val){
    if ((0<=Val)&&(Val<=9)){return char('0'+char(Val));}
    else if ((10<=Val)&&(Val<=15)){return char('A'+char(Val-10));}
    else Fail; return 0;}
  static char IsUc(const char& Ch){
    return ('A'<=Ch)&&(Ch<='Z');}
  static char GetUc(const char& Ch){
    if (('a'<=Ch)&&(Ch<='z')){return Ch-'a'+'A';} else {return Ch;}}

  static TStr GetStr(const TCh& Ch){
    return TStr(Ch.Val);}
};

/////////////////////////////////////////////////
// Unsigned-Char
class TUCh{
public:
  uchar Val;
public:
  static const uchar Mn;
  static const uchar Mx;
  static const int Vals;

  TUCh(): Val(TCh::NullCh){}
  TUCh(const uchar& _Val): Val(_Val){}
  operator uchar() const {return Val;}
  explicit TUCh(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TUCh& operator=(const TUCh& UCh){Val=UCh.Val; return *this;}
  bool operator==(const TUCh& UCh) const {return Val==UCh.Val;}
  bool operator<(const TUCh& UCh) const {return Val<UCh.Val;}
  TUCh& operator+=(const TUCh& UCh) { Val += UCh.Val; return *this; }
  TUCh& operator-=(const TUCh& UCh) { Val -= UCh.Val; return *this; }
  uchar operator()() const {return Val;}
  int GetMemUsed() const {return sizeof(TUCh);}

  int GetPrimHashCd() const {return Val;}
  int GetSecHashCd() const {return Val;}
};

/////////////////////////////////////////////////
// Short-Integer
class TSInt{
public:
  static const int16 Mn;
  static const int16 Mx;
  int16 Val;
public:
  TSInt(): Val(0){}
  TSInt(const int16& _Val): Val(_Val){}
  operator int16() const {return Val;}
  explicit TSInt(TSIn& SIn){SIn.Load(Val);}
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  int GetPrimHashCd() const {return Val;}
  int GetSecHashCd() const {return Val/0x10;}
  TSInt& operator=(const TSInt& Int) { Val = Int.Val; return *this; }
  TSInt& operator=(const int16& Int) { Val = Int; return *this; }
  bool operator==(const TSInt& Int) const { return Val == Int.Val; }
  bool operator==(const int& Int) const { return Val == Int; }
  bool operator!=(const int& Int) const { return Val != Int; }
  bool operator<(const TSInt& Int) const { return Val<Int.Val; }
  bool operator<(const int16& Int) const { return Val<Int; }
  int operator()() const { return Val; }

  TSInt& operator+=(const int16& Int) { Val += Int; return *this; }
  TSInt& operator-=(const int16& Int) { Val -= Int; return *this; }

  TSInt& operator++() { ++Val; return *this; } // prefix
  TSInt& operator--() { --Val; return *this; } // prefix
  /// Returns the memory footprint
  uint64 GetMemUsed() const { return sizeof(TSInt); }
};
typedef TSInt TInt16;

/////////////////////////////////////////////////
// Unsigned Short-Integer
class TUSInt {
public:
  static const uint16 Mn;
  static const uint16 Mx;
  uint16 Val;
public:
  TUSInt() : Val(0) {}
  TUSInt(const uint16& _Val) : Val(_Val) {}
  operator uint16() const { return Val; }
  explicit TUSInt(TSIn& SIn) { SIn.Load(Val); }
  void Load(TSIn& SIn) { SIn.Load(Val); }
  void Save(TSOut& SOut) const { SOut.Save(Val); }
  int GetPrimHashCd() const { return Val; }
  int GetSecHashCd() const { return Val / 0x10; }
  TUSInt& operator=(const TUSInt& Int) { Val = Int.Val; return *this; }
  TUSInt& operator=(const uint16& Int) { Val = Int; return *this; }
  bool operator==(const TUSInt& Int) const { return Val == Int.Val; }
  bool operator==(const int& Int) const { return Val == Int; }
  bool operator!=(const int& Int) const { return Val != Int; }
  bool operator<(const TUSInt& Int) const { return Val<Int.Val; }
  bool operator<(const uint16& Int) const { return Val<Int; }
  int operator()() const { return Val; }
  TUSInt& operator+=(const uint16& Int) { Val += Int; return *this; }
  TUSInt& operator-=(const uint16& Int) { Val -= Int; return *this; }
  TUSInt& operator++() { ++Val; return *this; } // prefix
  TUSInt& operator--() { --Val; return *this; } // prefix
};
typedef TUSInt TUInt16;

/////////////////////////////////////////////////
// Integer
typedef TNum<int> TInt;
template<>
class TNum<int>{
public:
  int Val;
public:
  static const int Mn;
  static const int Mx;
  static const int Kilo;
  static const int Mega;
  static const int Giga;
  static TRnd Rnd;

  TNum(): Val(0){}
  TNum(const int& _Val): Val(_Val){}
  operator int() const {return Val;}
  explicit TNum(TSIn& SIn){ SIn.Load(Val); }
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TNum& operator=(const TNum& Int){ Val = Int.Val; return *this; }
  TNum& operator=(const int& Int){ Val = Int; return *this; }
  bool operator==(const TNum& Int) const { return Val == Int.Val; }
  bool operator==(const int& Int) const {return Val==Int;}
  bool operator!=(const int& Int) const {return Val!=Int;}
  bool operator<(const TNum& Int) const { return Val<Int.Val; }
  bool operator<(const int& Int) const {return Val<Int;}
  int operator()() const {return Val;}
  TNum& operator+=(const int& Int){ Val += Int; return *this; }
  TNum& operator-=(const int& Int){ Val -= Int; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix

  int GetMemUsed() const { return sizeof(TInt); }

  int GetPrimHashCd() const {return Val;}
  int GetSecHashCd() const {return Val/0x10;}

  static int Abs(const int& Int){return Int<0?-Int:Int;}
  static int Sign(const int& Int){return Int<0?-1:(Int>0?1:0);}
  static void Swap(int& Int1, int& Int2){
    int SwapInt1=Int1; Int1=Int2; Int2=SwapInt1;}
  static int GetRnd(const int& Range=0){return Rnd.GetUniDevInt(Range);}
  static void SetRndSeed(const int& Seed){Rnd.PutSeed(Seed);}

  static bool IsOdd(const int& Int){return ((Int%2)==1);}
  static bool IsEven(const int& Int){return ((Int%2)==0);}

  static int GetMn(const int& Int1, const int& Int2){
    return Int1<Int2?Int1:Int2;}
  static int GetMx(const int& Int1, const int& Int2){
    return Int1>Int2?Int1:Int2;}
  static int GetMn(const int& Int1, const int& Int2, const int& Int3){
    return GetMn(Int1, GetMn(Int2, Int3));}
  static int GetMn(const int& Int1, const int& Int2,
   const int& Int3, const int& Int4){
    return GetMn(GetMn(Int1, Int2), GetMn(Int3, Int4));}
  static int GetMx(const int& Int1, const int& Int2, const int& Int3){
    return GetMx(Int1, GetMx(Int2, Int3));}
  static int GetMx(const int& Int1, const int& Int2,
   const int& Int3, const int& Int4){
    return GetMx(GetMx(Int1, Int2), GetMx(Int3, Int4));}
  static int GetInRng(const int& Val, const int& Mn, const int& Mx){
    IAssert(Mn<=Mx); return Val<Mn?Mn:(Val>Mx?Mx:Val);}

  TStr GetStr() const { return TNum::GetStr(Val); }

  static TStr GetStr(const int& Val){ return TStr::Fmt("%d", Val); }
  static TStr GetStr(const TNum& Int){ return GetStr(Int.Val); }
  static TStr GetStr(const int& Val, const char* FmtStr);
  static TStr GetStr(const int& Val, const TStr& FmtStr){ return GetStr(Val, FmtStr.CStr());}

  //J: So that TInt can convert any kind of integer to a string
  static TStr GetStr(const uint& Val){ return TStr::Fmt("%u", Val); }
  #ifdef GLib_WIN
  static TStr GetStr(const int64& Val) {return TStr::Fmt("%I64d", Val);}
  static TStr GetStr(const uint64& Val) {return TStr::Fmt("%I64u", Val);}
  #else
  static TStr GetStr(const int64& Val) {return TStr::Fmt("%lld", Val);}
  static TStr GetStr(const uint64& Val) {return TStr::Fmt("%llu", Val);}
  #endif

  static TStr GetHexStr(const int& Val){
    char Bf[255]; sprintf(Bf, "%X", Val); return TStr(Bf);}
  static TStr GetHexStr(const TNum& Int){
    return GetHexStr(Int.Val);}

  static TStr GetKiloStr(const int& Val){
    if (Val>=100*1000){return GetStr(Val/1000)+"K";}
    else if (Val>=1000){return GetStr(Val/1000)+"."+GetStr((Val%1000)/100)+"K";}
    else {return GetStr(Val);}}
  static TStr GetMegaStr(const int& Val){
    if (Val>=100*1000000){return GetStr(Val/1000000)+"M";}
    else if (Val>=1000000){
      return GetStr(Val/1000000)+"."+GetStr((Val%1000000)/100000)+"M";}
    else {return GetKiloStr(Val);}}
  // get the number as string using the thousands separator (1234 -> 1,234)
  TStr GetSepStr(const char& Sep = ',') const;

  // frugal
  static char* SaveFrugalInt(char *pDest, int i);
  static char* LoadFrugalInt(char *pSrc, int& i);
  static void TestFrugalInt();
  static void SaveFrugalIntV(TSOut& SOut, const TVec<TNum, int>& IntV);
  static void LoadFrugalIntV(TSIn& SIn, TVec<TNum, int>& IntV, bool ClrP = true);
};

/////////////////////////////////////////////////
// Unsigned-Integer
typedef TNum<uint> TUInt;
template<>
class TNum<uint>{
public:
  uint Val;
public:
  static const uint Mn;
  static const uint Mx;
  static TRnd Rnd;

  TNum() : Val(0){}
  TNum(const uint& _Val) : Val(_Val){}
  operator uint() const {return Val;}
  explicit TNum(TSIn& SIn){SIn.Load(Val);}
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TNum& operator=(const TNum& UInt){ Val = UInt.Val; return *this; }
  TNum& operator=(const uint& _Val){ Val = _Val; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix
  TNum& operator+=(const uint& Int){ Val += Int; return *this; }
  TNum& operator-=(const uint& Int){ Val -= Int; return *this; }
  uint operator()() const {return Val;}
  uint& operator()() {return Val;}
  TNum& operator~(){ Val = ~Val; return *this; }
  TNum& operator&=(const TNum& UInt){ Val &= UInt.Val; return *this; }
  TNum& operator|=(const TNum& UInt){ Val |= UInt.Val; return *this; }
  TNum& operator^=(const TNum& UInt){ Val ^= UInt.Val; return *this; }
  TNum& operator>>=(const int& ShiftBits){ Val >>= ShiftBits; return *this; }
  TNum& operator<<=(const int& ShiftBits){ Val <<= ShiftBits; return *this; }
  int GetMemUsed() const {return sizeof(TNum);}

  int GetPrimHashCd() const {return int(Val);}
  int GetSecHashCd() const {return Val/0x10;}

  static uint GetRnd(const uint& Range=0){return Rnd.GetUniDevUInt(Range);}

  TStr GetStr() const {return TNum::GetStr(Val);}
  static TStr GetStr(const uint& Val){
    char Bf[255]; sprintf(Bf, "%u", Val); return TStr(Bf);}
  static TStr GetStr(const TNum& UInt){
    return GetStr(UInt.Val);}
  static TStr GetStr(const uint& Val, const char* FmtStr);
  static TStr GetStr(const uint& Val, const TStr& FmtStr){
    return GetStr(Val, FmtStr.CStr());}

  static TStr GetKiloStr(const uint& Val){
    if (Val>100*1000){return GetStr(Val/1000)+"K";}
    else if (Val>1000){return GetStr(Val/1000)+"."+GetStr((Val%1000)/100)+"K";}
    else {return GetStr(Val);}}
  static TStr GetMegaStr(const uint& Val){
    if (Val>100*1000000){return GetStr(Val/1000000)+"M";}
    else if (Val>1000000){
      return GetStr(Val/1000000)+"."+GetStr((Val%1000000)/100000)+"M";}
    else {return GetKiloStr(Val);}}

  static uint JavaUIntToCppUInt(const uint& JavaUInt){
    uint B1=(JavaUInt & 0xFF000000) >> 24;
    uint B2=(JavaUInt & 0x00FF0000) >> 16;
    uint B3=(JavaUInt & 0x0000FF00) >> 8;
    uint B4=(JavaUInt & 0x000000FF) >> 0;
    uint CppUInt=(B4<<24)+(B3<<16)+(B2<<8)+(B1<<0);
    return CppUInt;}

  static bool IsIpStr(const TStr& IpStr, uint& Ip, const char& SplitCh = '.');
  static bool IsIpStr(const TStr& IpStr, const char& SplitCh = '.') { uint Ip; return IsIpStr(IpStr, Ip, SplitCh); }
  static uint GetUIntFromIpStr(const TStr& IpStr, const char& SplitCh = '.');
  static TStr GetStrFromIpUInt(const uint& Ip);
  static bool IsIpv6Str(const TStr& IpStr, const char& SplitCh = ':');

  static uint GetFromBufSafe(const char * Bf) {
#ifdef ARM
    uint Val;
    memcpy(&Val, Bf, sizeof(uint)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    return Val;
#else
    return *((uint*)Bf);
#endif
  }
};

/////////////////////////////////////////////////
// Signed-Integer-64Bit
typedef TNum<int64> TInt64;
template<>
class TNum<int64>{
public:
  int64 Val;
public:
  static const int64 Mn;
  static const int64 Mx;

  TNum() : Val(0){}
  TNum(const TNum& Int) : Val(Int.Val){}
  TNum(const int64& Int) : Val(Int){}
  operator int64() const { return Val; }
  explicit TNum(TSIn& SIn){ SIn.Load(Val); }
  void Load(TSIn& SIn){ SIn.Load(Val); }
  void Save(TSOut& SOut) const { SOut.Save(Val); }
  TNum& operator=(const TNum& Int){ Val = Int.Val; return *this; }
  TNum& operator+=(const TNum& Int){ Val += Int.Val; return *this; }
  TNum& operator-=(const TNum& Int){ Val -= Int.Val; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix
    int GetMemUsed() const { return sizeof(TNum); }

#ifdef GLib_WIN
  TStr GetStr() const { return TStr::Fmt("%I64", Val); }
  static TStr GetStr(const TNum& Int){ return TStr::Fmt("%I64", Int.Val); }
  static TStr GetHexStr(const TNum& Int){ return TStr::Fmt("%I64X", Int.Val); }
#else
  TStr GetStr() const { return TStr::Fmt("%ll", Val); }
  static TStr GetStr(const TNum& Int){ return TStr::Fmt("%ll", Int.Val); }
  static TStr GetHexStr(const TNum& Int){ return TStr::Fmt("%ll", Int.Val); }
#endif

  static TStr GetKiloStr(const int64& Val){
    if (Val>100 * 1000){ return GetStr(Val / 1000) + "K"; }
    else if (Val>1000){ return GetStr(Val / 1000) + "." + GetStr((Val % 1000) / 100) + "K"; }
    else { return GetStr(Val); }
  }
  static TStr GetMegaStr(const int64& Val){
    if (Val>100 * 1000000){ return GetStr(Val / 1000000) + "M"; }
    else if (Val>1000000){
      return GetStr(Val / 1000000) + "." + GetStr((Val % 1000000) / 100000) + "M";
    }
    else { return GetKiloStr(Val); }
  }
  /*static TStr GetGigaStr(const int64& Val){
  if (Val>100*1000000000){return GetStr(Val/1000000000)+"G";}
  else if (Val>1000000000){
  return GetStr(Val/1000000000)+"."+GetStr((Val%1000000000)/100000000)+"G";}
  else {return GetMegaStr(Val);}}*/

  static int64 GetFromBufSafe(const char * Bf) {
#ifdef ARM
    int64 Val;
    memcpy(&Val, Bf, sizeof(int64)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    return Val;
#else
    return *((int64*)Bf);
#endif
  }
};

/////////////////////////////////////////////////
// Unsigned-Integer-64Bit
typedef TNum<uint64> TUInt64;
template<>
class TNum<uint64>{
public:
  uint64 Val;
public:
  static const uint64 Mn;
  static const uint64 Mx;

  TNum() : Val(0){}
  TNum(const TNum& Int) : Val(Int.Val){}
  TNum(const uint64& Int) : Val(Int){}
  TNum(const uint& MsVal, const uint& LsVal) : Val(0){
    Val=(((uint64)MsVal) << 32) | ((uint64)LsVal);}
  explicit TNum(void* Pt) : Val(0){
     TConv_Pt64Ints32 Conv(Pt); Val=Conv.GetUInt64();}
  operator uint64() const {return Val;}
  explicit TNum(TSIn& SIn){ SIn.Load(Val); }
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TNum& operator=(const TNum& Int){ Val = Int.Val; return *this; }
  TNum& operator+=(const TNum& Int){ Val += Int.Val; return *this; }
  TNum& operator-=(const TNum& Int){ Val -= Int.Val; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix
  int GetMemUsed() const {return sizeof(TNum);}

  int GetPrimHashCd() const { return (int)GetMsVal() + (int)GetLsVal(); } //TODO: to check
  int GetSecHashCd() const { return ((int)GetMsVal() + (int)GetLsVal()) / 0x10; } //TODO: to check

  uint GetMsVal() const {
    return (uint)(Val >> 32);}
  uint GetLsVal() const {
    return (uint)(Val & 0xffffffff);}

  static uint64 GetMn(const uint64& Int1, const uint64& Int2){
    return Int1<Int2?Int1:Int2;}
  static uint64 GetMx(const uint64& Int1, const uint64& Int2){
    return Int1>Int2?Int1:Int2;}

  #ifdef GLib_WIN
  TStr GetStr() const {return TStr::Fmt("%I64u", Val);}
static TStr GetStr(const TNum& Int){return TStr::Fmt("%I64u", Int.Val);}
static TStr GetHexStr(const TNum& Int){return TStr::Fmt("%I64X", Int.Val);}
  #else
  TStr GetStr() const {return TStr::Fmt("%llu", Val);}
static TStr GetStr(const TNum& Int){return TStr::Fmt("%llu", Int.Val);}
static TStr GetHexStr(const TNum& Int){return TStr::Fmt("%llX", Int.Val);}
  #endif

  static TStr GetKiloStr(const uint64& Val){
    if (Val>100*1000){return GetStr(Val/1000)+"K";}
    else if (Val>1000){return GetStr(Val/1000)+"."+GetStr((Val%1000)/100)+"K";}
    else {return GetStr(Val);}}
  static TStr GetMegaStr(const uint64& Val){
    if (Val>100*1000000){return GetStr(Val/1000000)+"M";}
    else if (Val>1000000){
      return GetStr(Val/1000000)+"."+GetStr((Val%1000000)/100000)+"M";}
    else {return GetKiloStr(Val);}}
  /*static TStr GetGigaStr(const uint64& Val){
    if (Val>100*1000000000){return GetStr(Val/1000000000)+"G";}
    else if (Val>1000000000){
      return GetStr(Val/1000000000)+"."+GetStr((Val%1000000000)/100000000)+"G";}
    else {return GetMegaStr(Val);}}*/

  static uint64 GetFromBufSafe(const char * Bf) {
    #ifdef ARM
    uint64 Val;
    memcpy(&Val, Bf, sizeof(uint64)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    return Val;
    #else
    return *((uint64*)Bf);
    #endif
  }
};

/////////////////////////////////////////////////
// Float
typedef TNum<double> TFlt;
template<>
class TNum<double>{
public:
  double Val;
public:
  static const double Mn;
  static const double Mx;
  static const double NInf;
  static const double PInf;
  static const double Eps;
  static const double EpsHalf;
  static TRnd Rnd;

  TNum() : Val(0.0){}
  TNum(const double& _Val) : Val(_Val){}
  operator double() const {return Val;}
  explicit TNum(TSIn& SIn){ SIn.Load(Val); }
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  explicit TNum(TSIn& SIn, const bool& IsTxt){
    if (IsTxt){ TStr Str(SIn, true); Val = Str.GetFlt(0); }
    else { SIn.Load(Val); }
  }
  void Load(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut, const bool& IsTxt) const {
    if (IsTxt){GetStr(Val).Save(SOut, true);} else {SOut.Save(Val);}}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TNum& operator=(const TNum& Flt){ Val = Flt.Val; return *this; }
  TNum& operator=(const double& Flt){ Val = Flt; return *this; }
  bool operator==(const TNum& Flt) const _CMPWARN{ return Val == Flt.Val; }
  bool operator==(const double& Flt) const _CMPWARN {return Val==Flt;}
  bool operator!=(const double& Flt) const _CMPWARN {return Val!=Flt;}
  double operator()() const {return Val;}
  TNum& operator+=(const double& Flt){ Val += Flt; return *this; }
  TNum& operator-=(const double& Flt){ Val -= Flt; return *this; }
  TNum& operator*=(const double& Flt){ Val *= Flt; return *this; }
  TNum& operator/=(const double& Flt){ Val /= Flt; return *this; }
  TNum& operator++(){ ++Val; return *this; } // prefix
  TNum& operator--(){ --Val; return *this; } // prefix
  TNum operator++(int){ TNum oldVal = Val; Val++; return oldVal; } // postfix
  TNum operator--(int){ TNum oldVal = Val; Val--; return oldVal; } // postfix
  int GetMemUsed() const { return sizeof(TNum); }

  int GetPrimHashCd() const {
    int Expn; return int((frexp(Val, &Expn)-0.5)*double(TInt::Mx));}
  int GetSecHashCd() const {
    int Expn; frexp(Val, &Expn); return Expn;}

  static double Abs(const double& Flt){return Flt<0?-Flt:Flt;}
  static int Sign(const double& Flt){return Flt<0?-1:(Flt>0?1:0);}
  static int Round(const double& Flt){return int(floor(Flt+0.5));}
  static double GetRnd(){return Rnd.GetUniDev();}
  static bool Eq6(const double& LFlt, const double& RFlt){
    return fabs(LFlt-RFlt)<0.000001;}

  static double GetMn(const double& Flt1, const double& Flt2){
    return Flt1<Flt2?Flt1:Flt2;}
  static double GetMn(const double& Flt1, const double& Flt2, const double& Flt3){
    return GetMn(GetMn(Flt1, Flt2), Flt3); }
  static double GetMn(const double& Flt1, const double& Flt2, const double& Flt3, const double& Flt4){
    return GetMn(GetMn(Flt1, Flt2), GetMn(Flt3, Flt4)); }

  static double GetMx(const double& Flt1, const double& Flt2){
    return Flt1>Flt2?Flt1:Flt2;}
  static double GetMx(const double& Flt1, const double& Flt2, const double Flt3){
    return GetMx(GetMx(Flt1, Flt2), Flt3); }
  static double GetMx(const double& Flt1, const double& Flt2, const double Flt3, const double& Flt4){
    return GetMx(GetMx(Flt1, Flt2), GetMx(Flt3, Flt4)); }

  static double GetInRng(const double& Val, const double& Mn, const double& Mx){
    IAssert(Mn<=Mx); return Val<Mn?Mn:(Val>Mx?Mx:Val);}

  static bool IsNum(const double& Val){
    return (Mn<=Val)&&(Val<=Mx);}
  static bool IsNan(const double& Val){
    return (_isnan(Val) != 0);}

  bool IsNum() const { return IsNum(Val); }
  bool IsNan() const { return IsNan(Val); }

  TStr GetStr() const {return TNum::GetStr(Val);}
  static TStr GetStr(const double& Val, const int& Width=-1, const int& Prec=-1);
  static TStr GetStr(const TNum& Flt, const int& Width=-1, const int& Prec=-1){
    return GetStr(Flt.Val, Width, Prec);}
  static TStr GetStr(const double& Val, const char* FmtStr);
  static TStr GetStr(const double& Val, const TStr& FmtStr){
    return GetStr(Val, FmtStr.CStr());}
  static TStr GetPrcStr(const double& RelVal, const double& FullVal){
    return GetStr(100*RelVal/FullVal, "%3.0f%%");}

  static TStr GetKiloStr(const double& Val){
    if (fabs(Val)>100*1000){return TStr::Fmt("%.0fK", Val/1000);}
    else if (fabs(Val)>1000){return TStr::Fmt("%.1fK", Val/1000);}
    else {return TStr::Fmt("%.0f", Val);}}
  static TStr GetMegaStr(const double& Val){
    if (fabs(Val)>100*1000000){return TStr::Fmt("%.0fM", Val/1000000);}
    else if (fabs(Val)>1000000){return TStr::Fmt("%.1fM", Val/1000000);}
    else {return GetKiloStr(Val);}}
  static TStr GetGigaStr(const double& Val){
    if (fabs(Val)>100*1000000000.0){return TStr::Fmt("%.0fG", Val/1000000000.0);}
    else if (fabs(Val)>1000000000.0){return TStr::Fmt("%.1fG", Val/1000000000.0);}
    else {return GetMegaStr(Val);}}
  static double GetFromBufSafe(const char * Bf) {
    #ifdef ARM
    double Val;
    memcpy(&Val, Bf, sizeof(double)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    return Val;
    #else
    return *((double*)Bf);
    #endif
  }

  // TODO test!
  static void SetBufSafe(const double& Val, char * Bf) {
    #ifdef ARM
    memcpy(Bf, &Val, sizeof(double)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    #else
    *((double*)Bf) = Val;
    #endif
  }
};

/////////////////////////////////////////////////
// Ascii-Float
class TAscFlt: public TFlt{
public:
  TAscFlt(): TFlt(){}
  TAscFlt(const double& Val): TFlt(Val){}
  explicit TAscFlt(TSIn& SIn): TFlt(SIn, true){}
  void Save(TSOut& SOut) const {TFlt::Save(SOut, true);}
};

/////////////////////////////////////////////////
// Short-Float
class TSFlt{
public:
  sdouble Val;
public:
  static const sdouble Mn;
  static const sdouble Mx;

  TSFlt(): Val(0){}
  TSFlt(const sdouble& _Val): Val(sdouble(_Val)){}
  //TSFlt(const double& _Val): Val(sdouble(_Val)){}
  operator sdouble() const {return Val;}
  //operator double() const {return Val;}
  explicit TSFlt(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TSFlt& operator=(const TSFlt& SFlt){Val=SFlt.Val; return *this;}
  bool operator==(const TSFlt& SFlt) const _CMPWARN {return Val==SFlt.Val;}
  bool operator==(const double& Flt) const _CMPWARN {return Val==Flt;}
  bool operator!=(const double& Flt) const _CMPWARN {return Val!=Flt;}
  bool operator<(const TSFlt& SFlt) const {return Val<SFlt.Val;}
  sdouble operator()() const {return Val;}
  TSFlt& operator+=(const double& SFlt){Val+=sdouble(SFlt); return *this;}
  TSFlt& operator-=(const double& SFlt){Val-=sdouble(SFlt); return *this;}
  TSFlt& operator*=(const double& SFlt){Val*=sdouble(SFlt); return *this;}
  TSFlt& operator/=(const double& SFlt){Val/=sdouble(SFlt); return *this;}
  TSFlt& operator++(){++Val; return *this;} // prefix
  TSFlt& operator--(){--Val; return *this;} // prefix
  TSFlt operator++(int){TSFlt oldVal = Val; Val++; return oldVal;} // postfix
  TSFlt operator--(int){TSFlt oldVal = Val; Val--; return oldVal;} // postfix
  int GetMemUsed() const {return sizeof(TSFlt);}

  static bool IsNum(const float& Val) {
    return (Mn <= Val) && (Val <= Mx);
  }
  static bool IsNan(const float& Val) {
    return (_isnan(Val) != 0);
  }

  bool IsNum() const { return IsNum(Val); }
  bool IsNan() const { return IsNan(Val); }

  int GetPrimHashCd() const {
    int Expn; return int((frexp(Val, &Expn)-0.5)*double(TInt::Mx));}
  int GetSecHashCd() const {
    int Expn; frexp(Val, &Expn); return Expn;}
  static float GetFromBufSafe(const char * Bf) {
#ifdef ARM
    float Val;
    memcpy(&Val, Bf, sizeof(float)); //we cannot use a cast on ARM (needs 8byte memory aligned doubles)
    return Val;
#else
    return *((float*)Bf);
#endif
  }
};

/////////////////////////////////////////////////
// Long-Float
class TLFlt{
public:
  ldouble Val;
public:
  static const ldouble Mn;
  static const ldouble Mx;

  TLFlt(): Val(0){}
  TLFlt(const ldouble& _Val): Val(_Val){}
  operator ldouble() const {return Val;}
  explicit TLFlt(TSIn& SIn){SIn.Load(Val);}
  void Save(TSOut& SOut) const {SOut.Save(Val);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TLFlt& operator=(const TLFlt& LFlt){Val=LFlt.Val; return *this;}
  bool operator==(const TLFlt& LFlt) const _CMPWARN {return Val==LFlt.Val;}
  bool operator==(const ldouble& LFlt) const _CMPWARN {return Val==LFlt;}
  bool operator!=(const ldouble& LFlt) const _CMPWARN {return Val!=LFlt;}
  bool operator<(const TLFlt& LFlt) const {return Val<LFlt.Val;}
  ldouble operator()() const {return Val;}
  TLFlt& operator+=(const ldouble& LFlt){Val+=LFlt; return *this;}
  TLFlt& operator-=(const ldouble& LFlt){Val-=LFlt; return *this;}
  TLFlt& operator++(){++Val; return *this;} // prefix
  TLFlt& operator--(){--Val; return *this;} // prefix
  TLFlt operator++(int){TLFlt oldVal = Val; Val++; return oldVal;} // postfix
  TLFlt operator--(int){TLFlt oldVal = Val; Val--; return oldVal;} // postfix
  int GetMemUsed() const {return sizeof(TLFlt);}

  int GetPrimHashCd() const {Fail; return 0;}
  int GetSecHashCd() const {Fail; return 0;}

  static TStr GetStr(const ldouble& Val, const int& Width=-1, const int& Prec=-1);
  static TStr GetStr(const TLFlt& LFlt, const int& Width=-1, const int& Prec=-1){
    return GetStr(LFlt.Val, Width, Prec);}
  static TStr GetStr(const ldouble& Val, const char* FmtStr);
  static TStr GetStr(const ldouble& Val, const TStr& FmtStr){
    return GetStr(Val, FmtStr.CStr());}
};

/////////////////////////////////////////////////
// Float-Rectangle
class TFltRect{
public:
  TFlt MnX, MnY, MxX, MxY;
public:
  TFltRect():
    MnX(), MnY(), MxX(), MxY(){}
  TFltRect(const TFltRect& FltRect):
    MnX(FltRect.MnX), MnY(FltRect.MnY), MxX(FltRect.MxX), MxY(FltRect.MxY){}
  TFltRect(
   const double& _MnX, const double& _MnY,
   const double& _MxX, const double& _MxY):
    MnX(_MnX), MnY(_MnY), MxX(_MxX), MxY(_MxY){}
  TFltRect(TSIn& SIn):
    MnX(SIn), MnY(SIn), MxX(SIn), MxY(SIn){}
  void Save(TSOut& SOut) const {
    MnX.Save(SOut); MnY.Save(SOut); MxX.Save(SOut); MxY.Save(SOut);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TFltRect& operator=(const TFltRect& FltRect){
    MnX=FltRect.MnX; MnY=FltRect.MnY; MxX=FltRect.MxX; MxY=FltRect.MxY;
    return *this;}

  // get coordinates
  double GetMnX() const {return MnX;}
  double GetMnY() const {return MnY;}
  double GetMxX() const {return MxX;}
  double GetMxY() const {return MxY;}

  // get lengths
  double GetXLen() const {return MxX-MnX;}
  double GetYLen() const {return MxY-MnY;}

  // get centers
  double GetXCenter() const {return MnX+(MxX-MnX)/2;}
  double GetYCenter() const {return MnY+(MxY-MnY)/2;}

  // tests
  bool IsXYIn(const double& X, const double& Y) const {
    return (MnX<=X)&&(X<=MxX)&&(MnY<=Y)&&(Y<=MxY);}
  static bool Intersection(const TFltRect& Rect1, const TFltRect& Rect2);

  // string
  TStr GetStr() const;
};

/////////////////////////////////////////////////
// Saving and loading enums
template <class TVal>
void SaveEnum(TSOut& SOut, const TVal& Val) {
    TInt((int)Val).Save(SOut);
}

template <class TVal>
TVal LoadEnum(TSIn& SIn) {
    return (TVal)(TInt(SIn).Val);
}

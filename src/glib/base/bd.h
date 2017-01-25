/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef bd_h
#define bd_h

#include <cstdint>

/////////////////////////////////////////////////
// Basic-Macro-Definitions
#define forever for(;;)

/////////////////////////////////////////////////
// Basic-Type-Definitions
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

typedef float sdouble;
typedef long double ldouble;
typedef FILE* TFileId;

typedef char int8;
typedef short int16;
typedef int int32;
#ifdef GLib_WIN
  typedef __int64 int64;
#elif defined(GLib_GLIBC)
  typedef int64_t int64;
#else
  typedef long long int64;
#endif

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
#ifdef GLib_WIN
  typedef unsigned __int64 uint64;
#elif defined(GLib_GLIBC)
  typedef u_int64_t uint64;
#else
typedef unsigned long long uint64;
#endif

#if (!defined(__ssize_t_defined) && !defined(GLib_MACOSX))
  typedef ptrdiff_t ssize_t;
#endif

#if defined(GLib_MACOSX)
  #define _isnan(x) std::isnan(x)
  #define _finite(x) isfinite(x)
#elif defined(GLib_UNIX)
  #define _isnan(x) isnan(x)
  #define _finite(x) finite(x)
#endif

#if defined(GLib_WIN)
  #define _vsnprintf vsnprintf
#endif

typedef size_t TSize;
# ifdef GLib_64Bit
  #define TSizeMx (18446744073709551615UL)
# else
  #define TSizeMx (4294967295U)
# endif

/////////////////////////////////////////////////
// Localization
typedef enum {lUndef, lUs, lSi} TLoc;

/////////////////////////////////////////////////
// Messages
void WrNotify(const char* CaptionCStr, const char* NotifyCStr);
void SaveToErrLog(const char* MsgCStr);

inline void InfoNotify(const char* NotifyCStr){
  WrNotify("Information", NotifyCStr);}
inline void WarnNotify(const char* NotifyCStr){
  WrNotify("Warning", NotifyCStr);}
inline void ErrNotify(const char* NotifyCStr){
  WrNotify("Error", NotifyCStr);}
inline void StatNotify(const char* NotifyCStr){
  WrNotify("Status", NotifyCStr);}

/////////////////////////////////////////////////
// Compiler-Options
#if defined (GLib_WIN)
  #define snprintf _snprintf
  #define vsnprintf  _vsnprintf
  // disable warning 'debug symbol greater than 255 chars'
  #pragma warning(disable: 4786)
  // disable warning 'return type for 'identifier::operator->'
  // is not a UDT or reference to a UDT.
  //#pragma warning(disable: 4284)
  // 'declaration' : function differs from 'declaration'
  // only by calling convention
  //#pragma warning(disable: 4666)
  #pragma warning(disable: 4996)
#endif

#if defined (GLib_GCC)
  #define _CONSOLE
#endif

// use console output if console already exists, otherwise
// define GLib_COUT to use console (class COut)
#if defined(_CONSOLE)||defined(__CONSOLE__)
  #define GLib_Console
#endif

// create console if want to output but there is no console allocated
#if defined(GLib_Console) && (! (defined(__CONSOLE__)||defined(_CONSOLE)))
  #define GLib_CreateConsole
#endif

/////////////////////////////////////////////////
// Class-Definition
#define ClassT(TNm) \
class TNm{

#define ClassTV(TNm, TNmV) \
class TNm; \
typedef TVec<TNm> TNmV; \
class TNm{

#define ClassTVQ(TNm, TNmV, TNmQ) \
class TNm; \
typedef TVec<TNm> TNmV; \
typedef TQQueue<TNm> TNmQ; \
class TNm{

#define ClassTP(TNm, PNm) \
class TNm; \
typedef TPt<TNm> PNm; \
class TNm{ \
protected: \
  TCRef CRef; \
public: \
  friend class TPt<TNm>;

#define ClassHdTP(TNm, PNm) \
class TNm; \
typedef TPt<TNm> PNm;

#define ClassTPE(TNm, PNm, ENm) \
class TNm; \
typedef TPt<TNm> PNm; \
class TNm: public ENm{ \
public: \
  friend class TPt<TNm>;

#define ClassTPEE(TNm, PNm, ENm1, ENm2) \
class TNm; \
typedef TPt<TNm> PNm; \
class TNm: public ENm1, public ENm2{ \
public: \
  friend class TPt<TNm>;

#define ClassTE(TNm, ENm) \
class TNm; \
class TNm: public ENm{ \
public: \

#define ClassTPV(TNm, PNm, TNmV) \
class TNm; \
typedef TPt<TNm> PNm; \
typedef TVec<PNm> TNmV; \
class TNm{ \
protected: \
  TCRef CRef; \
public: \
  friend class TPt<TNm>;

#define ClassHdTPV(TNm, PNm, TNmV) \
class TNm; \
typedef TPt<TNm> PNm; \
typedef TVec<PNm> TNmV;

#define ClassTPVL(TNm, PNm, TNmV, TNmL, TNmLN) \
class TNm; \
typedef TPt<TNm> PNm; \
typedef TVec<PNm> TNmV; \
typedef TLst<PNm> TNmL; \
typedef TLstNd<PNm>* TNmLN; \
class TNm{ \
protected: \
  TCRef CRef; \
public: \
  friend class TPt<TNm>;

/////////////////////////////////////////////////
// Forward-Definitions
class TSIn;
class TSOut;
class TStr;
class TChA;
class TXmlObjSer;
class TXmlObjSerTagNm;
template <class TRec> class TPt;
ClassHdTP(TXmlTok, PXmlTok)
ClassHdTP(TExcept, PExcept)
ClassHdTP(TXmlDoc, PXmlDoc)

/////////////////////////////////////////////////
// DefaultConstructor/CopyConstructor/AssignmentOperator-UnDefinition
#define UndefDefaultCopyAssign(TNm) \
private: \
  TNm(){} \
  TNm(const TNm&); \
  TNm& operator=(const TNm&)

#define UndefCopyAssign(TNm) \
private: \
  TNm(const TNm&); \
  TNm& operator=(const TNm&)

#define UndefCopy(TNm) \
private: \
  TNm(const TNm&)

#define UndefAssign(TNm) \
private: \
  TNm& operator=(const TNm&)

/////////////////////////////////////////////////
// Assertions
class TOnExeStop{
public:
  typedef bool (*TOnExeStopF)(char* MsgCStr);
private:
  static TOnExeStopF OnExeStopF;
public:
  static bool IsOnExeStopF(){return OnExeStopF!=NULL;}
  static void PutOnExeStopF(TOnExeStopF _OnExeStopF){OnExeStopF=_OnExeStopF;}
  static TOnExeStopF GetOnExeStopF(){return OnExeStopF;}
};

void ExeStop(
 const char* MsgStr, const char* ReasonStr,
 const char* CondStr, const char* FNm, const int& LnN);

#define Fail ExeStop(NULL, NULL, "Fail", __FILE__, __LINE__)

#define FailR(Reason) ExeStop((Reason), NULL, "Fail", __FILE__, __LINE__)
#define FailRA(Reason, ArgStr) \
  ExeStop((TStr(Reason)+" ("+ArgStr+")").CStr(), NULL, "Fail", __FILE__, __LINE__)

#define EFail TExcept::ThrowFull("", TStr("[")+ TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) + "]")

#define EFailR(Reason) TExcept::ThrowFull(Reason, TStr("[")+TStr(__FILE__)+" line "+TInt::GetStr(__LINE__)+"]")

#ifdef NDEBUG
#define DebugCode(Code)
#else
#define DebugCode(Code) \
  Code
#endif

#ifdef NDEBUG
#define Assert(Cond)
#else
#define Assert(Cond) \
  ((Cond) ? static_cast<void>(0) : ExeStop(NULL, NULL, #Cond, __FILE__, __LINE__))
#endif

#ifdef NDEBUG
#define AssertR(Cond, Reason)
#else
#define AssertR(Cond, Reason) ((Cond) ? static_cast<void>(0) : \
  ExeStop(NULL, TStr(Reason).CStr(), #Cond, __FILE__, __LINE__))
#endif

#define IAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : ExeStop(NULL, NULL, #Cond, __FILE__, __LINE__))

#define IAssertR(Cond, Reason) \
  ((Cond) ? static_cast<void>(0) : ExeStop(NULL, TStr(Reason).CStr(), #Cond, __FILE__, __LINE__))

#define WAssert(Cond, MsgCStr) \
  ((Cond) ? static_cast<void>(0) : WarnNotify(MsgCStr))

#define SAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : ExeStop(TSysStr::GetLastMsgCStr(), NULL, #Cond, __FILE__, __LINE__))

#define FAssert(Cond, MsgCStr) \
  ((Cond) ? static_cast<void>(0) : ExeStop(TStr(MsgCStr).CStr(), NULL, NULL, __FILE__, __LINE__))

#define FSAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : ExeStop(TSysStr::GetLastMsgCStr(), NULL, NULL, __FILE__, __LINE__))

#define EAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : TExcept::ThrowFull(#Cond, TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) +": "+ TStr(#Cond)))

#define EAssertR(Cond, MsgStr) \
  ((Cond) ? static_cast<void>(0) : TExcept::ThrowFull(MsgStr, TStr(__FILE__)+" line "+TInt::GetStr(__LINE__)+": "+TStr(#Cond)))

#define EAssertRA(Cond, MsgStr, ArgStr) \
  ((Cond) ? static_cast<void>(0) : TExcept::Throw(MsgStr, ArgStr))

#define EAssertRAA(Cond, MsgStr, ArgStr1, ArgStr2) \
  ((Cond) ? static_cast<void>(0) : TExcept::Throw(MsgStr, ArgStr1, ArgStr2))

#define ESAssert(Cond) \
  ((Cond) ? static_cast<void>(0) : TExcept::Throw(TSysStr::GetLastMsgCStr(), \
    TStr(__FILE__) + TStr(" line ") + TInt::GetStr(__LINE__) + TStr(": ")+ TStr(#Cond) ))

// compile time assert
// #define STATIC_ASSERT(x) { const char temp[ (((x) == 0) ? 0 : 1) ] = {'\0'}; }
template <bool BoolVal> struct TStaticAssert;
template <> struct TStaticAssert<true> { enum { value = 1 }; };
template<int IntVal> struct TStaticAssertTest{};

#define CAssert(Cond) \
  /* { typedef TStaticAssertTest<sizeof(TStaticAssert<(Cond)==0?false:true>)> TestStaticAssert; } */

/////////////////////////////////////////////////
// Xml-Object-Serialization
template <class TRec>
bool IsXLoadFromFileOk(const TStr& FNm, const TStr& Nm, TRec& Rec, TStr& MsgStr);
template <class TRec>
void XLoadFromFile(const TStr& FNm, const TStr& Nm, TRec& Rec);

#define XLoadHd(Nm) \
  {TStr TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSer::AssertXmlHd(XmlTok, Nm, TypeNm);}
#define XLoad(Nm) \
  Nm.LoadXml(XmlTok->GetTagTok(#Nm), #Nm);

#define XSaveHd(Nm) \
  TStr _TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSerTagNm XmlObjSerTagNm(SOut, false, Nm, _TypeNm);
#define XSaveHdArg(Nm, ArgNm, ArgVal) \
  TStr _TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSerTagNm XmlObjSerTagNm(SOut, false, Nm, _TypeNm, ArgNm, ArgVal);
#define XSaveBETag(Nm) \
  TStr _TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSerTagNm XmlObjSerTagNm(SOut, true, Nm, _TypeNm);
#define XSaveBETagArg(Nm, ArgNm, ArgVal) \
  TStr _TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSerTagNm XmlObjSerTagNm(SOut, true, Nm, _TypeNm, ArgNm, ArgVal);
#define XSaveBETagArg4(Nm, ArgNm1, ArgVal1, ArgNm2, ArgVal2, ArgNm3, ArgVal3, ArgNm4, ArgVal4) \
  TStr _TypeNm=TXmlObjSer::GetTagNm(TStr(typeid(*this).name())); \
  TXmlObjSerTagNm XmlObjSerTagNm(SOut, true, Nm, _TypeNm, ArgNm1, ArgVal1, ArgNm2, ArgVal2, ArgNm3, ArgVal3, ArgNm4, ArgVal4);
#define XSave(Nm) \
  Nm.SaveXml(SOut, #Nm)
#define XSaveToFile(Nm, FNm) \
  {TFOut SOut(FNm); Nm.SaveXml(SOut, #Nm);}

/////////////////////////////////////////////////
// Basic-Operators
typedef enum {loUndef, loNot, loAnd, loOr} TLogOp;
typedef enum {roUndef, roLs, roLEq, roEq, roNEq, roGEq, roGt} TRelOp;

/////////////////////////////////////////////////
// Comparation-Macros
#ifndef MIN
  #define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
  #define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

/////////////////////////////////////////////////
// Comparator-Definitions
template<class T>
class TEq{
public:
  bool operator()(const T& x, const T& y) const {return x==y;}
};

template<class T>
class TNEq{
public:
  bool operator()(const T& x, const T& y) const {return x!=y;}
};

template<class T>
class TLss{
public:
  bool operator()(const T& x, const T& y) const {return x<y;}
  int Cmp(const T& d1, const T& d2) const {
    if (d1<d2){return -1;}
    else if (d2<d1){return 1;}
    else {return 0;}
  }
};

template<class T>
class TLEq{
public:
  bool operator()(const T& x, const T& y) const {return x<=y;}
};

template<class T>
class TGtr{
public:
  bool operator() (const T& x, const T& y) const { return x>y; }
  int Cmp(const T& d1, const T& d2) const { //** vprasaj Janeza za uporabo
    if (d1<d2){return 1;}
    else if (d2<d1){return -1;}
    else {return 0;}
  }
};

template<class T>
class TGEq{
public:
  bool operator()(const T& x, const T& y) const {return x>=y;}
};

template<class T>
class TCmp{
public:
  int operator()(const T& x, const T& y) const {
    if (x < y){return -1;}
    else if (x > y){return 1;}
    else {return 0;}
  }
};

/////////////////////////////////////////////////
// Operator-Definitions
template <class TRec>
bool operator!=(const TRec& Rec1, const TRec& Rec2){return !(Rec1==Rec2);}

template <class TRec>
bool operator>(const TRec& Rec1, const TRec& Rec2){return Rec2<Rec1;}

template <class TRec>
bool operator<=(const TRec& Rec1, const TRec& Rec2){return !(Rec2<Rec1);}

template <class TRec>
bool operator>=(const TRec& Rec1, const TRec& Rec2){return !(Rec1<Rec2);}

template <class TRec>
bool Cmp(const int& RelOp, const TRec& Rec1, const TRec& Rec2){
  switch (RelOp){
    case roLs: return Rec1<Rec2;
    case roLEq: return Rec1<=Rec2;
    case roEq: return Rec1==Rec2;
    case roNEq: return Rec1!=Rec2;
    case roGEq: return Rec1>=Rec2;
    case roGt: return Rec1>Rec2;
    default: Fail; return false;
  }
}

/////////////////////////////////////////////////
// Reference-Count
class TCRef{
private:
  int Refs;
private:
  TCRef& operator=(const TCRef&);
  TCRef(const TCRef&);
public:
  TCRef(): Refs(0){}
  ~TCRef(){Assert(Refs==0);}

  void MkRef(){Refs++;}
  void UnRef(){Assert(Refs>0); Refs--;}
  bool NoRef() const {return Refs==0;}
  int GetRefs() const {return Refs;}
  uint64 GetMemUsed() const { return sizeof(TCRef); }
};

/////////////////////////////////////////////////
// Weak-Pointer forward declaration
template <class TRec> class TWPt;

/////////////////////////////////////////////////
// Smart-Pointer-With-Reference-Count
template <class TRec>
class TPt{
public:
  typedef TRec TObj;
private:
  TRec* Addr;
  void MkRef() const {
    if (Addr!=NULL){
      Addr->CRef.MkRef();
    }
  }
  void UnRef() const {
    if (Addr!=NULL){
      Addr->CRef.UnRef();
      if (Addr->CRef.NoRef()){delete Addr;}
    }
  }
public:
  TPt(): Addr(NULL){}
  TPt(const TPt& Pt): Addr(Pt.Addr){MkRef();}
  TPt(TRec* _Addr): Addr(_Addr){MkRef();}
  TPt(TWPt<TRec> WPt): Addr(WPt()){MkRef();}
  static TPt New(){return TObj::New();}
  ~TPt(){UnRef();}
  explicit TPt(TSIn& SIn);
  explicit TPt(TSIn& SIn, void* ThisPt);
  void Save(TSOut& SOut) const;
  void LoadXml(const TPt<TXmlTok>& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TPt& operator=(const TPt& Pt){
    if (this!=&Pt){Pt.MkRef(); UnRef(); Addr=Pt.Addr;} return *this;}
  bool operator==(const TPt& Pt) const {return *Addr==*Pt.Addr;}
  bool operator!=(const TPt& Pt) const {return *Addr!=*Pt.Addr;}
  bool operator<(const TPt& Pt) const {return *Addr<*Pt.Addr;}

  TRec* operator->() const {Assert(Addr!=NULL); return Addr;}
  TRec& operator*() const {Assert(Addr!=NULL); return *Addr;}
  TRec& operator[](const int& RecN) const {
    Assert(Addr!=NULL); return Addr[RecN];}
  TRec* operator()() const {return Addr;}

  bool Empty() const {return Addr==NULL;}
  void Clr(){UnRef(); Addr=NULL;}
  bool Unique() const {
    if (Addr==NULL){return false;} else {return (Addr->CRef.GetRefs()==1);}}
  int GetRefs() const {
    if (Addr==NULL){return -1;} else {return Addr->CRef.GetRefs();}}

  int GetPrimHashCd() const {return Addr->GetPrimHashCd();}
  int GetSecHashCd() const {return Addr->GetSecHashCd();}
  uint64 GetMemUsed() const { return (Empty() ? 0 : Addr->GetMemUsed()) + sizeof(TPt); }

  TPt<TRec> Clone(){return MkClone(*this);}
};

/////////////////////////////////////////////////
// Weak-Pointer (no reference count)
template <class TRec>
class TWPt{
public:
  typedef TRec TObj;
private:
  TRec* Addr;
public:
  TWPt(): Addr(NULL){}
  TWPt(TRec* _Addr): Addr(_Addr){}
  TWPt(const TWPt& WPt): Addr(WPt.Addr){}
  TWPt(const TPt<TRec>& Pt): Addr(Pt()){}
  ~TWPt(){}

  TWPt& operator=(TRec* _Addr){Addr=_Addr; return *this;}
  TWPt& operator=(const TWPt& WPt){Addr=WPt.Addr; return *this;}
  TWPt& operator=(const TPt<TRec>& Pt){Addr=Pt(); return *this;}
  bool operator==(const TWPt& WPt) const {return *Addr==*WPt.Addr;}
  bool operator!=(const TWPt& WPt) const {return *Addr!=*WPt.Addr;}
  bool operator<(const TWPt& WPt) const {return *Addr<*WPt.Addr;}

  TRec* operator->() const {Assert(Addr!=NULL); return Addr;}
  TRec& operator*() const {Assert(Addr!=NULL); return *Addr;}
  TRec& operator[](const int& RecN) const {
    Assert(Addr!=NULL); return Addr[RecN];}
  TRec* operator()() const {return Addr;}

  bool Empty() const {return Addr==NULL;}
  void Clr(){Addr=NULL;}
  void Del(){delete Addr; Addr=NULL;}

  int GetPrimHashCd() const {return Addr->GetPrimHashCd();}
  int GetSecHashCd() const { return Addr->GetSecHashCd(); }
  //uint64 GetMemUsed() const { return (Empty() ? 0 : Addr->GetMemUsed()) + sizeof(TPt); }
};

/////////////////////////////////////////////////
// Simple-String
class TSStr{
private:
  char* Bf;
public:
  TSStr(): Bf(NULL){
    Bf=new char[0+1]; Bf[0]=0;}
  TSStr(const TSStr& SStr): Bf(NULL){
    Bf=new char[strlen(SStr.Bf)+1]; strcpy(Bf, SStr.Bf);}
  TSStr(const char* _Bf): Bf(NULL){
    Bf=new char[strlen(_Bf)+1]; strcpy(Bf, _Bf);}
  ~TSStr(){delete[] Bf;}

  TSStr& operator=(const TSStr& SStr){
    if (this!=&SStr){
      delete[] Bf; Bf=new char[strlen(SStr.Bf)+1]; strcpy(Bf, SStr.Bf);}
    return *this;}

  char* CStr() {return Bf;}
  const char* CStr() const {return Bf;}
  bool Empty() const {return Bf[0]==0;}
  int Len() const {return int(strlen(Bf));}
};

/////////////////////////////////////////////////
// Conversion-Pointer64-To-Integers32
class TConv_Pt64Ints32{
private:
  union{
    void* Pt;
    uint64 UInt64;
    struct {uint Ls; uint Ms;} UInt32;
  } Val;
  UndefCopyAssign(TConv_Pt64Ints32);
public:
  TConv_Pt64Ints32(){
    Val.Pt=0; Val.UInt32.Ms=0; Val.UInt32.Ls=0;}
  TConv_Pt64Ints32(void* Pt){
    Val.UInt32.Ms=0; Val.UInt32.Ls=0; Val.Pt=Pt;}
  TConv_Pt64Ints32(const uint& Ms, const uint& Ls){
    Val.Pt=0; Val.UInt32.Ms=Ms; Val.UInt32.Ls=Ls;}

  void PutPt(void* Pt){Val.Pt=Pt;}
  void* GetPt() const {return Val.Pt;}
  void PutUInt64(const uint64& _UInt64){Val.UInt64=_UInt64;}
  uint64 GetUInt64() const {return Val.UInt64;}
  void PutMsUInt32(const uint& Ms){Val.UInt32.Ms=Ms;}
  uint GetMsUInt32() const {return Val.UInt32.Ms;}
  void PutLsUInt32(const uint& Ls){Val.UInt32.Ls=Ls;}
  uint GetLsUInt32() const {return Val.UInt32.Ls;}
};

/////////////////////////////////////////////////
// Swap
template <class TRec>
void Swap(TRec& Rec1, TRec& Rec2){
    std::swap(Rec1, Rec2); // C++11
  //TRec Rec=Rec1; Rec1=Rec2; Rec2=Rec; // C++98
}

/////////////////////////////////////////////////

/// Computes a hash code from a pair of hash codes. ##TPairHashImpl1
class TPairHashImpl1 {
public:
  static inline int GetHashCd(const int hc1, const int hc2) {
    unsigned long long sum = ((unsigned long long) hc1) + ((unsigned long long) hc2);
    unsigned long long c = ((sum * (sum + 1)) >> 1) + hc1;
    return (int) (c % 0x7fffffffULL); }
};

/// Computes a hash code from a pair of hash codes. ##TPairHashImpl2
class TPairHashImpl2 {
public:
  static inline int GetHashCd(const int hc1, const int hc2) {
    unsigned long long sum = ((unsigned long long) hc1) + ((unsigned long long) hc2);
    unsigned long long c = ((sum * (sum + 1)) >> 1) + hc1;
    unsigned int R = (unsigned int) (c >> 31), Q = (unsigned int) (c & 0x7fffffffULL);
    if ((R & 0x80000000U) != 0) R -= 0x7fffffffU;
    unsigned int RQ = R + Q;
    if (RQ < 0x7fffffffU) return (int) RQ;
    RQ -= 0x7fffffffU;
    return (RQ == 0x7fffffffU) ? 0 : (int) RQ; }
};

//////////////////////////////////////////
// Type traits
#ifdef GLib_CPP11

// forward declarations
class TBool;
class TCh;
class TUCh;
class TUSInt;

template <class Base>                                class TNum;
template <class TVal, class TSizeTy>                 class TVec;
template <class TKey, class TDat, class THashFunc>   class THash;
template <class TVal1, class TVal2>                  class TPair;

namespace gtraits {
  /// cpp type traits, helper to check if type is a container
  template <typename T> struct is_container : std::false_type{};
  // TODO: use a built-in trait to detect shallow classes when compilers will implement most type traits
  /// helper to check if the type is shallow (does not have any pointers or references and can be copied using memcpy)
  template <typename T> struct is_shallow : std::false_type{};

  // helper types and classes
  namespace utils {
      template<typename T> struct bool_type : std::true_type{};
      template<> struct bool_type<std::false_type> : std::false_type{};

      template <class TVal1, class TVal2>
      struct TPairHelper {
        private:
          // is_shallow
          template <class T1 = TVal1, class T2 = TVal2, typename std::enable_if<is_shallow<T1>::value && is_shallow<T2>::value,bool>::type = true>
          static std::true_type IsShallow();
          template <class T1 = TVal1, class T2 = TVal2, typename std::enable_if<!(is_shallow<T1>::value && is_shallow<T2>::value),bool>::type = true>
          static std::false_type IsShallow();
        public:
          using shallow_type = decltype(IsShallow());
      };
  }

  // Specializations: is_shallow
  // basic types
  template <> struct is_shallow<TBool> : std::true_type{};
  template <> struct is_shallow<TCh> : std::true_type{};
  template <> struct is_shallow<TUCh> : std::true_type{};
  template <> struct is_shallow<TUSInt> : std::true_type{};
  // TNum
  template <class Base> struct is_shallow<TNum<Base>> : std::true_type{};
  // TPair
  template <class TVal1, class TVal2>
  struct is_shallow<TPair<TVal1,TVal2>> : utils::bool_type<typename utils::TPairHelper<TVal1,TVal2>::shallow_type>{};

  // Specializations: is_container
  template <class TVal, class TSizeTy>
  struct is_container<TVec<TVal,TSizeTy>> : std::true_type{};
  template<class TKey, class TDat, class THashFunc>
  struct is_container<THash<TKey,TDat,THashFunc>> : std::true_type{};
}

#endif

namespace TMemUtils {
#ifdef GLib_CPP11

  /// fundamental types (int, float, ...)
  template <typename T, typename std::enable_if<std::is_fundamental<T>::value,bool>::type = true>
  uint64 GetMemUsed(const T& Val) {
    return sizeof(T);
  }

  /// get memory usage for regular glib classes
  template <typename T, typename std::enable_if<std::is_class<T>::value && !gtraits::is_container<T>::value,bool>::type = true>
  uint64 GetMemUsed(const T& Val) {
    return Val.GetMemUsed();
  }

  /// glib containers
  template <typename T, typename std::enable_if<gtraits::is_container<T>::value,bool>::type = true>
  uint64 GetMemUsed(const T& Val) {
    return Val.GetMemUsed(true);   // deep
  }

  /// references
  template <typename T, typename std::enable_if<std::is_reference<T>::value,bool>::type = true>
  uint64 GetMemUsed(T Val) {
    return GetMemUsed<typename std::remove_reference<T>::type>(Val);
  }

  /// pointers
  template <typename T, typename std::enable_if<std::is_pointer<T>::value,bool>::type = true>
  uint64 GetMemUsed(T Val) {
    if (Val == nullptr) { return sizeof(T); }
    return sizeof(T) + GetMemUsed(*Val);
  }

#else
  template <class T>
  uint64 GetMemUsed(const T& Val) {
    return Val.GetMemUsed();
  }
  template <class T>
  uint64 GetMemUsed(T* Val) {
    if (Val == NULL) { return sizeof(T*); }
    return sizeof(T*) + GetMemUsed(*Val);
  }
#endif

  /////////////////////////////////////////////////
  // Functions which returns the part of the memory footprint of
  // an object which ignored by the sizeof operator

  /// returns the extra space that the instances members occupy
  /// which is not taken into account by the sizeof operator
  template <class TClass>
  uint64 GetExtraMemberSize(const TClass& Inst) {
    return TMemUtils::GetMemUsed(Inst) - sizeof(TClass);
  }
  /// returns the extra space that the container occupies
  /// which is not taken into account by the sizeof operator
  template <class TClass>
  uint64 GetExtraContainerSizeShallow(const TClass& Inst) {
    return Inst.GetMemUsed(false) - sizeof(TClass);
  }
}

// Depending on the platform and compiler settings choose the faster implementation (of the same hash function)
#if (defined(GLib_64Bit)) && ! (defined(DEBUG) || defined(_DEBUG))
  typedef TPairHashImpl1 TPairHashImpl;
#else
  typedef TPairHashImpl2 TPairHashImpl;
#endif

#endif

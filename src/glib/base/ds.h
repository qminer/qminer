/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Address-Pointer
template <class TRec>
class TAPt{
private:
  TRec* Addr;
public:
  TAPt(): Addr(NULL){}
  TAPt(const TAPt& Pt): Addr(Pt.Addr){}
  TAPt(TRec* _Addr): Addr(_Addr){}
  TAPt(TSIn&){Fail;}
  void Save(TSOut&) const {Fail;}

  TAPt& operator=(const TAPt& Pt){Addr=Pt.Addr; return *this;}
  TAPt& operator=(TRec* _Addr){Addr=_Addr; return *this;}
  bool operator==(const TAPt& Pt) const {return *Addr==*Pt.Addr;}
  bool operator!=(const TAPt& Pt) const {return *Addr!=*Pt.Addr;}
  bool operator<(const TAPt& Pt) const {return *Addr<*Pt.Addr;}

  TRec* operator->() const {Assert(Addr!=NULL); return Addr;}
  TRec& operator*() const {Assert(Addr!=NULL); return *Addr;}
  TRec& operator[](const int& RecN) const {
    Assert(Addr!=NULL); return Addr[RecN];}
  TRec* operator()() const {return Addr;}

  bool Empty() const {return Addr==NULL;}
};

/////////////////////////////////////////////////
// Pair
template <class TVal1, class TVal2>
class TPair{
public:
  TVal1 Val1;
  TVal2 Val2;
public:
  TPair(): Val1(), Val2(){}
  TPair(const TPair& Pair): Val1(Pair.Val1), Val2(Pair.Val2){}
  TPair(const TPair&& Pair): Val1(std::move(Pair.Val1)), Val2(std::move(Pair.Val2)) {}
  TPair(const TVal1& _Val1, const TVal2& _Val2): Val1(_Val1), Val2(_Val2){}
  explicit TPair(TSIn& SIn): Val1(SIn), Val2(SIn){}
  void Save(TSOut& SOut) const {
    Val1.Save(SOut); Val2.Save(SOut);}
  void Load(TSIn& SIn) {Val1.Load(SIn); Val2.Load(SIn);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TPair& operator=(const TPair& Pair){
    if (this!=&Pair){Val1=Pair.Val1; Val2=Pair.Val2;} return *this;}
  TPair& operator=(TPair&& Pair) {
	  if (this != &Pair) {std::swap(Val1, Pair.Val1); std::swap(Val2, Pair.Val2);} return *this;}
  bool operator==(const TPair& Pair) const {
    return (Val1==Pair.Val1)&&(Val2==Pair.Val2);}
  bool operator<(const TPair& Pair) const {
    return (Val1<Pair.Val1)||((Val1==Pair.Val1)&&(Val2<Pair.Val2));}

  uint64 GetMemUsed() const {return Val1.GetMemUsed()+Val2.GetMemUsed();}

  int GetPrimHashCd() const {return TPairHashImpl::GetHashCd(Val1.GetPrimHashCd(), Val2.GetPrimHashCd()); }
  int GetSecHashCd() const {return TPairHashImpl::GetHashCd(Val2.GetSecHashCd(), Val1.GetSecHashCd()); }

  void GetVal(TVal1& _Val1, TVal2& _Val2) const {_Val1=Val1; _Val2=Val2;}
  const TVal1& GetVal1() const { return Val1;}
  const TVal2& GetVal2() const { return Val2;}
  TStr GetStr() const {return TStr("Pair(")+Val1.GetStr()+", "+Val2.GetStr()+")";}
};

template <class TVal1, class TVal2, class TSizeTy>
void GetSwitchedPrV(const TVec<TPair<TVal1, TVal2>, TSizeTy>& SrcPrV, TVec<TPair<TVal2, TVal1>, TSizeTy>& DstPrV){
  const TSizeTy Prs = SrcPrV.Len();
  DstPrV.Gen(Prs, 0);
  for (TSizeTy PrN=0; PrN<Prs; PrN++){
    const TPair<TVal1, TVal2>& SrcPr=SrcPrV[PrN];
    DstPrV.Add(TPair<TVal2, TVal1>(SrcPr.Val2, SrcPr.Val1));
  }
}

typedef TPair<TBool, TBool> TBoolPr;
typedef TPair<TBool, TCh> TBoolChPr;
typedef TPair<TBool, TFlt> TBoolFltPr;
typedef TPair<TUCh, TUCh> TUChPr;
typedef TPair<TUCh, TInt> TUChIntPr;
typedef TPair<TUCh, TUInt64> TUChUInt64Pr;
typedef TPair<TUCh, TStr> TUChStrPr;
typedef TPair<TInt, TBool> TIntBoolPr;
typedef TPair<TInt, TCh> TIntChPr;
typedef TPair<TInt, TUCh> TIntUChPr;
typedef TPair<TInt, TInt> TIntPr;
typedef TPair<TInt, TUInt64> TIntUInt64Pr;
typedef TPair<TInt, TIntPr> TIntIntPrPr;
typedef TPair<TInt, TVec<TInt, int> > TIntIntVPr;
typedef TPair<TInt, TFlt> TIntFltPr;
typedef TPair<TInt, TStr> TIntStrPr;
typedef TPair<TInt, TStrV> TIntStrVPr;
typedef TPair<TInt16, TInt16> TInt16Pr;
typedef TPair<TInt64, TInt64> TInt64Pr;
typedef TPair<TIntPr, TInt> TIntPrIntPr;
typedef TPair<TUInt, TUInt> TUIntUIntPr;
typedef TPair<TUInt, TInt> TUIntIntPr;
typedef TPair<TUInt, TStr> TUIntStrPr;
typedef TPair<TUInt16, TUInt16> TUInt16Pr;
typedef TPair<TUInt64, TInt> TUInt64IntPr;
typedef TPair<TUInt64, TUInt64> TUInt64Pr;
typedef TPair<TUInt64, TFlt> TUInt64FltPr;
typedef TPair<TUInt64, TStr> TUInt64StrPr;
typedef TPair<TFlt, TInt> TFltIntPr;
typedef TPair<TFlt, TUInt64> TFltUInt64Pr;
typedef TPair<TFlt, TFlt> TFltPr;
typedef TPair<TFlt, TStr> TFltStrPr;
typedef TPair<TSFlt, TSFlt> TSFltPr;
typedef TPair<TAscFlt, TInt> TAscFltIntPr;
typedef TPair<TAscFlt, TAscFlt> TAscFltPr;
typedef TPair<TFlt, TStr> TFltStrPr;
typedef TPair<TAscFlt, TStr> TAscFltStrPr;
typedef TPair<TStr, TInt> TStrIntPr;
typedef TPair<TStr, TFlt> TStrFltPr;
typedef TPair<TStr, TStr> TStrPr;
typedef TPair<TStr, TStrV> TStrStrVPr;
typedef TPair<TStrV, TInt> TStrVIntPr;
typedef TPair<TInt, TIntPr> TIntIntPrPr;
typedef TPair<TInt, TStrPr> TIntStrPrPr;
typedef TPair<TFlt, TStrPr> TFltStrPrPr;

/// Compares the pair by the second value.
template <class TVal1, class TVal2>
class TCmpPairByVal2 {
private:
  bool IsAsc;
public:
  TCmpPairByVal2(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TPair<TVal1, TVal2>& P1, const TPair<TVal1, TVal2>& P2) const {
    if (IsAsc) { return P1.Val2 < P2.Val2; } else { return P2.Val2 < P1.Val2; }
  }
};

/////////////////////////////////////////////////
// Triple
template <class TVal1, class TVal2, class TVal3>
class TTriple{
public:
  TVal1 Val1;
  TVal2 Val2;
  TVal3 Val3;
public:
  TTriple(): Val1(), Val2(), Val3(){}
  TTriple(const TTriple& Triple):
    Val1(Triple.Val1), Val2(Triple.Val2), Val3(Triple.Val3){}
  TTriple(TTriple&& Triple):
  	Val1(std::move(Triple.Val1)), Val2(std::move(Triple.Val2)), Val3(std::move(Triple.Val3)) {}
  TTriple(const TVal1& _Val1, const TVal2& _Val2, const TVal3& _Val3):
    Val1(_Val1), Val2(_Val2), Val3(_Val3){}
  explicit TTriple(TSIn& SIn): Val1(SIn), Val2(SIn), Val3(SIn){}
  void Load(TSIn& SIn) {Val1.Load(SIn); Val2.Load(SIn); Val3.Load(SIn);}
  void Save(TSOut& SOut) const {
    Val1.Save(SOut); Val2.Save(SOut); Val3.Save(SOut);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TTriple& operator=(const TTriple& Triple){
    if (this!=&Triple){Val1=Triple.Val1; Val2=Triple.Val2; Val3=Triple.Val3;}
    return *this;}
  TTriple& operator=(TTriple&& Triple) {
	  if (this != &Triple) {
		std::swap(Val1, Triple.Val1);
		std::swap(Val2, Triple.Val2);
		std::swap(Val3, Triple.Val3);
	  }
	  return *this;
  }
  bool operator==(const TTriple& Triple) const {
    return (Val1==Triple.Val1)&&(Val2==Triple.Val2)&&(Val3==Triple.Val3);}
  bool operator<(const TTriple& Triple) const {
    return (Val1<Triple.Val1)||((Val1==Triple.Val1)&&(Val2<Triple.Val2))||
     ((Val1==Triple.Val1)&&(Val2==Triple.Val2)&&(Val3<Triple.Val3));}

  int GetPrimHashCd() const {return  TPairHashImpl::GetHashCd(TPairHashImpl::GetHashCd(Val1.GetPrimHashCd(), Val2.GetPrimHashCd()), Val3.GetPrimHashCd()); }
  int GetSecHashCd() const {return TPairHashImpl::GetHashCd(TPairHashImpl::GetHashCd(Val2.GetSecHashCd(), Val3.GetSecHashCd()), Val1.GetSecHashCd()); }
  int GetMemUsed() const {return Val1.GetMemUsed()+Val2.GetMemUsed()+Val3.GetMemUsed();}

  void GetVal(TVal1& _Val1, TVal2& _Val2, TVal3& _Val3) const {
    _Val1=Val1; _Val2=Val2; _Val3=Val3;}
  TStr GetStr() const {
    return TStr("Triple(")+Val1.GetStr()+", "+Val2.GetStr()+", "+Val3.GetStr()+")";}
};

typedef TTriple<TCh, TCh, TCh> TChTr;
typedef TTriple<TCh, TInt, TInt> TChIntIntTr;
typedef TTriple<TUCh, TInt, TInt> TUChIntIntTr;
typedef TTriple<TInt, TInt, TInt> TIntTr;
typedef TTriple<TUInt64, TUInt64, TUInt64> TUInt64Tr;
typedef TTriple<TUInt64, TUInt64, TInt> TUInt64UInt64IntTr;
typedef TTriple<TInt, TStr, TInt> TIntStrIntTr;
typedef TTriple<TInt, TInt, TStr> TIntIntStrTr;
typedef TTriple<TInt, TInt, TFlt> TIntIntFltTr;
typedef TTriple<TInt, TFlt, TInt> TIntFltIntTr;
typedef TTriple<TInt, TFlt, TFlt> TIntFltFltTr;
typedef TTriple<TFlt, TInt, TFlt> TFltIntFltTr;
typedef TTriple<TInt, TVec<TInt, int>, TInt> TIntIntVIntTr;
typedef TTriple<TInt, TInt, TVec<TInt, int> > TIntIntIntVTr;
typedef TTriple<TFlt, TFlt, TFlt> TFltTr;
typedef TTriple<TFlt, TInt, TInt> TFltIntIntTr;
typedef TTriple<TFlt, TFlt, TInt> TFltFltIntTr;
typedef TTriple<TFlt, TFlt, TStr> TFltFltStrTr;
typedef TTriple<TChA, TChA, TChA> TChATr;
typedef TTriple<TStr, TStr, TStr> TStrTr;
typedef TTriple<TStr, TInt, TInt> TStrIntIntTr;
typedef TTriple<TStr, TFlt, TFlt> TStrFltFltTr;
typedef TTriple<TStr, TStr, TInt> TStrStrIntTr;
typedef TTriple<TStr, TInt, TStrV> TStrIntStrVTr;
    
/// Compares the triple by the second value.
template <class TVal1, class TVal2, class TVal3>
class TCmpTripleByVal2 {
private:
  bool IsAsc;
public:
  TCmpTripleByVal2(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TTriple<TVal1, TVal2, TVal3>& T1, const TTriple<TVal1, TVal2, TVal3>& T2) const {
    if (IsAsc) { return T1.Val2 < T2.Val2; } else { return T2.Val2 < T1.Val2; }
  }
};
    
/// Compares the triple by the third value.
template <class TVal1, class TVal2, class TVal3>
class TCmpTripleByVal3 {
private:
  bool IsAsc;
public:
  TCmpTripleByVal3(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TTriple<TVal1, TVal2, TVal3>& T1, const TTriple<TVal1, TVal2, TVal3>& T2) const {
    if (IsAsc) { return T1.Val3 < T2.Val3; } else { return T2.Val3 < T1.Val3; }
  }
};

/////////////////////////////////////////////////
// Quad
template <class TVal1, class TVal2, class TVal3, class TVal4>
class TQuad{
public:
  TVal1 Val1;
  TVal2 Val2;
  TVal3 Val3;
  TVal4 Val4;
public:
  TQuad():
    Val1(), Val2(), Val3(), Val4(){}
  TQuad(const TQuad& Quad):
    Val1(Quad.Val1), Val2(Quad.Val2), Val3(Quad.Val3), Val4(Quad.Val4){}
  TQuad(const TVal1& _Val1, const TVal2& _Val2, const TVal3& _Val3, const TVal4& _Val4):
    Val1(_Val1), Val2(_Val2), Val3(_Val3), Val4(_Val4){}
  explicit TQuad(TSIn& SIn):
    Val1(SIn), Val2(SIn), Val3(SIn), Val4(SIn){}
  void Save(TSOut& SOut) const {
    Val1.Save(SOut); Val2.Save(SOut); Val3.Save(SOut); Val4.Save(SOut);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TQuad& operator=(const TQuad& Quad){
    if (this!=&Quad){
      Val1=Quad.Val1; Val2=Quad.Val2; Val3=Quad.Val3; Val4=Quad.Val4;}
    return *this;}
  bool operator==(const TQuad& Quad) const {
    return (Val1==Quad.Val1)&&(Val2==Quad.Val2)&&(Val3==Quad.Val3)&&(Val4==Quad.Val4);}
  bool operator<(const TQuad& Quad) const {
    return (Val1<Quad.Val1)||((Val1==Quad.Val1)&&(Val2<Quad.Val2))||
     ((Val1==Quad.Val1)&&(Val2==Quad.Val2)&&(Val3<Quad.Val3))||
     ((Val1==Quad.Val1)&&(Val2==Quad.Val2)&&(Val3==Quad.Val3)&&(Val4<Quad.Val4));}

  int GetPrimHashCd() const {return  TPairHashImpl::GetHashCd(TPairHashImpl::GetHashCd(Val1.GetPrimHashCd(), Val2.GetPrimHashCd()), TPairHashImpl::GetHashCd(Val3.GetPrimHashCd(), Val4.GetPrimHashCd())); }
  int GetSecHashCd() const {return TPairHashImpl::GetHashCd(TPairHashImpl::GetHashCd(Val2.GetSecHashCd(), Val3.GetSecHashCd()), TPairHashImpl::GetHashCd(Val4.GetSecHashCd(), Val1.GetSecHashCd())); }

  void GetVal(TVal1& _Val1, TVal2& _Val2, TVal3& _Val3, TVal4& _Val4) const {
    _Val1=Val1; _Val2=Val2; _Val3=Val3; _Val4=Val4;}
  TStr GetStr() const {
    return TStr("Quad(")+Val1.GetStr()+", "+Val2.GetStr()+", "+Val3.GetStr()+", "+Val4.GetStr()+")";}
};

typedef TQuad<TStr, TStr, TInt, TInt> TStrStrIntIntQu;
typedef TQuad<TStr, TStr, TStr, TStr> TStrQu;
typedef TQuad<TInt, TInt, TInt, TInt> TIntQu;
typedef TQuad<TFlt, TFlt, TFlt, TFlt> TFltQu;
typedef TQuad<TFlt, TInt, TInt, TInt> TFltIntIntIntQu;
typedef TQuad<TInt, TStr, TInt, TInt> TIntStrIntIntQu;
typedef TQuad<TInt, TInt, TFlt, TFlt> TIntIntFltFltQu;

/////////////////////////////////////////////////
// Tuple
template<class TVal, int NVals>
class TTuple {
private:
  TVal ValV [NVals];
public:
  TTuple(){}
  TTuple(const TVal& InitVal) { for (int i=0; i<Len(); i++) ValV[i]=InitVal; }
  TTuple(const TTuple& Tup) { for (int i=0; i<Len(); i++) ValV[i]=Tup[i]; }
  TTuple(TSIn& SIn) { for (int i=0; i<Len(); i++) ValV[i].Load(SIn); }
  void Save(TSOut& SOut) const { for (int i=0; i<Len(); i++) ValV[i].Save(SOut); }
  void Load(TSIn& SIn) { for (int i=0; i<Len(); i++) ValV[i].Load(SIn); }

  int Len() const { return NVals; }
  TVal& operator[] (const int& ValN) { return ValV[ValN]; }
  const TVal& operator[] (const int& ValN) const { return ValV[ValN]; }
  TTuple& operator = (const TTuple& Tup) { if (this != & Tup) {
    for (int i=0; i<Len(); i++) ValV[i]=Tup[i]; } return *this; }
  bool operator == (const TTuple& Tup) const {
    if (Len()!=Tup.Len()) { return false; }  if (&Tup==this) { return true; }
    for (int i=0; i<Len(); i++) if(ValV[i]!=Tup[i]){return false;} return true; }
  bool operator < (const TTuple& Tup) const {
    if (Len() == Tup.Len()) { for (int i=0; i<Len(); i++) {
      if(ValV[i]<Tup[i]){return true;} else if(ValV[i]>Tup[i]){return false;} } return false; }
    else { return Len() < Tup.Len(); } }
  void Sort(const bool& Asc=true);
  int FindMx() const;
  int FindMn() const;
  int GetPrimHashCd() const { int hc = 0;
    for (int i = 0; i < NVals; i++) { hc = TPairHashImpl::GetHashCd(hc, ValV[i].GetPrimHashCd()); }
    return hc; }
  int GetSecHashCd() const { int hc = 0;
    for (int i = 1; i < NVals; i++) { hc = TPairHashImpl::GetHashCd(hc, ValV[i].GetSecHashCd()); }
    if (NVals > 0) { hc = TPairHashImpl::GetHashCd(hc, ValV[0].GetSecHashCd()); }
    return hc; }
  
  TStr GetStr() const { TChA ValsStr;
    for (int i=0; i<Len(); i++) { ValsStr+=" "+ValV[i].GetStr(); }
    return TStr::Fmt("Tuple(%d):", Len())+ValsStr; }
};

/////////////////////////////////////////////////
// Key-Data
template <class TKey, class TDat>
class TKeyDat{
public:
  TKey Key;
  TDat Dat;
public:
  TKeyDat(): Key(), Dat(){}
  TKeyDat(const TKeyDat& KeyDat): Key(KeyDat.Key), Dat(KeyDat.Dat){}
  TKeyDat(TKeyDat&& KeyDat): Key(std::move(KeyDat.Key)), Dat(std::move(KeyDat.Dat)){}
  explicit TKeyDat(const TKey& _Key): Key(_Key), Dat(){}
  TKeyDat(const TKey& _Key, const TDat& _Dat): Key(_Key), Dat(_Dat){}
  explicit TKeyDat(TSIn& SIn): Key(SIn), Dat(SIn){}
  void Save(TSOut& SOut) const {Key.Save(SOut); Dat.Save(SOut);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TKeyDat& operator=(const TKeyDat& KeyDat){
    if (this!=&KeyDat){Key=KeyDat.Key; Dat=KeyDat.Dat;} return *this;}
  TKeyDat& operator=(TKeyDat&& KeyDat){
    if (this!=&KeyDat){Key=std::move(KeyDat.Key); Dat=std::move(KeyDat.Dat);} return *this;}
  bool operator==(const TKeyDat& KeyDat) const {return Key==KeyDat.Key;}
  bool operator<(const TKeyDat& KeyDat) const {return Key<KeyDat.Key;}

  int GetPrimHashCd() const {return Key.GetPrimHashCd();}
  int GetSecHashCd() const {return Key.GetSecHashCd();}
  uint64 GetMemUsed() const { return Key.GetMemUsed() + Dat.GetMemUsed(); }
};

template <class TKey, class TDat>
void GetSwitchedKdV(const TVec<TKeyDat<TKey, TDat>, int>& SrcKdV, TVec<TKeyDat<TDat, TKey>, int>& DstKdV){
  const int Kds=SrcKdV.Len();
  DstKdV.Gen(Kds, 0);
  for (int KdN=0; KdN<Kds; KdN++){
    const TKeyDat<TKey, TDat>& SrcKd=SrcKdV[KdN];
    DstKdV.Add(TKeyDat<TDat, TKey>(SrcKd.Dat, SrcKd.Key));
  }
}

template <class TKey, class TDat>
void GetKeyV(const TVec<TKeyDat<TKey, TDat>, int>& SrcKdV, TVec<TKey, int>& DstV){
  const int Kds=SrcKdV.Len();
  DstV.Gen(Kds, 0);
  for (int KdN=0; KdN<Kds; KdN++){
    DstV.Add(SrcKdV[KdN].Key);
  }
}

template <class TKey, class TDat>
void GetDatV(const TVec<TKeyDat<TKey, TDat>, int>& SrcKdV, TVec<TDat, int>& DstV){
  const int Kds=SrcKdV.Len();
  DstV.Gen(Kds, 0);
  for (int KdN=0; KdN<Kds; KdN++){
    DstV.Add(SrcKdV[KdN].Dat);
  }
}

typedef TKeyDat<TInt, TInt> TIntKd;
typedef TKeyDat<TInt, TUInt64> TIntUInt64Kd;
typedef TKeyDat<TInt, TFlt> TIntFltKd;
typedef TKeyDat<TIntPr, TFlt> TIntPrFltKd;
typedef TKeyDat<TInt, TFltPr> TIntFltPrKd;
typedef TKeyDat<TInt, TSFlt> TIntSFltKd;
typedef TKeyDat<TInt, TStr> TIntStrKd;
typedef TKeyDat<TUInt, TInt> TUIntIntKd;
typedef TKeyDat<TUInt, TUInt> TUIntKd;
typedef TKeyDat<TUInt64, TInt> TUInt64IntKd;
typedef TKeyDat<TUInt64, TFlt> TUInt64FltKd;
typedef TKeyDat<TUInt64, TStr> TUInt64StrKd;
typedef TKeyDat<TFlt, TBool> TFltBoolKd;
typedef TKeyDat<TFlt, TInt> TFltIntKd;
typedef TKeyDat<TFlt, TUInt64> TFltUInt64Kd;
typedef TKeyDat<TFlt, TIntPr> TFltIntPrKd;
typedef TKeyDat<TFlt, TUInt> TFltUIntKd;
typedef TKeyDat<TFlt, TFlt> TFltKd;
typedef TKeyDat<TFlt, TStr> TFltStrKd;
typedef TKeyDat<TFlt, TBool> TFltBoolKd;
typedef TKeyDat<TFlt, TIntBoolPr> TFltIntBoolPrKd;
typedef TKeyDat<TAscFlt, TInt> TAscFltIntKd;
typedef TKeyDat<TStr, TBool> TStrBoolKd;
typedef TKeyDat<TStr, TInt> TStrIntKd;
typedef TKeyDat<TStr, TFlt> TStrFltKd;
typedef TKeyDat<TStr, TAscFlt> TStrAscFltKd;
typedef TKeyDat<TStr, TStr> TStrKd;

// Key-Data comparator

template <class TVal1, class TVal2>
class TCmpKeyDatByDat {
private:
  bool IsAsc;
public:
  TCmpKeyDatByDat(const bool& AscSort=true) : IsAsc(AscSort) { }
  bool operator () (const TKeyDat<TVal1, TVal2>& P1, const TKeyDat<TVal1, TVal2>& P2) const {
    if (IsAsc) { return P1.Dat < P2.Dat; } else { return P2.Dat < P1.Dat; }
  }
};

//#//////////////////////////////////////////////
/// Vector is a sequence \c TVal objects representing an array that can change in size. ##TVec
template <class TVal, class TSizeTy = int>
class TVec{
public:
  typedef TVal* TIter;  //!< Random access iterator to \c TVal.
protected:
  TSizeTy MxVals; //!< Vector capacity. Capacity is the size of allocated storage. If <tt>MxVals==-1</tt>, then \c ValT is not owned by the vector, and it won't free it at destruction.
  TSizeTy Vals;   //!< Vector length. Length is the number of elements stored in the vector.
  TVal* ValT;     //!< Pointer to the memory where the elements of the vector are stored.
  /// Resizes the vector so that it can store at least \c _MxVals.
  void Resize(const TSizeTy& _MxVals=-1);
  /// Constructs the out of bounds error message.
  TStr GetXOutOfBoundsErrMsg(const TSizeTy& ValN) const;
public:
  TVec(): MxVals(0), Vals(0), ValT(NULL){}
  TVec(const TVec<TVal, TSizeTy>& Vec);
#ifdef GLib_CPP11
  // Move constructor
  TVec(TVec<TVal, TSizeTy>&& Vec);
#endif

  /// Constructs a vector (an array) of length \c _Vals.
  explicit TVec(const TSizeTy& _Vals){
    IAssert(0<=_Vals); MxVals=Vals=_Vals;
    if (_Vals==0){ValT=NULL;} else {ValT=new TVal[_Vals];}}
  /// Constructs a vector (an array) of length \c _Vals, while reserving enough memory to store \c _MxVals elements.
  TVec(const TSizeTy& _MxVals, const TSizeTy& _Vals){
    IAssert((0 <= _Vals) && (_Vals <= _MxVals)); MxVals = _MxVals; Vals = _Vals;
    if (_MxVals == 0){ ValT = NULL; } else { ValT = new TVal[_MxVals]; }}
  /// Constructs a vector of \c _Vals elements of memory array \c _ValT. ##TVec::TVec
  explicit TVec(TVal *_ValT, const TSizeTy& _Vals):
    MxVals(-1), Vals(_Vals), ValT(_ValT){}
  ~TVec(){if ((ValT!=NULL) && (MxVals!=-1)){delete[] ValT;}}
  explicit TVec(TSIn& SIn): MxVals(0), Vals(0), ValT(NULL){Load(SIn);}
  // For fast deserialization
  explicit TVec(TMIn& MemIn) : MxVals(0), Vals(0), ValT(NULL) { LoadMemCpy(MemIn); }

  void Load(TSIn& SIn);
  // optimized deserialization from stream, uses memcpy
  void LoadMemCpy(TMIn& SIn);
  void Save(TSOut& SOut) const;  
  // optimized serialization to stream, uses memcpy
  void SaveMemCpy(TMOut& SOut) const;

  /// Sends the vector contents via a socket \c fd.
  int Send(int sd);
  /// Writes \c nbytes bytes starting at \c ptr to a file/socket descriptor \c fd.
  int WriteN(int fd, char *ptr, int nbytes);
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  /// Assigns new contents to the vector, replacing its current content.
  TVec<TVal, TSizeTy>& operator=(const TVec<TVal, TSizeTy>& Vec);
  // Move assignment
  TVec<TVal, TSizeTy>& operator=(TVec<TVal, TSizeTy>&& Vec);
  /// Appends value \c Val to the vector.
  TVec<TVal, TSizeTy>& operator+(const TVal& Val){Add(Val); return *this;}
  /// Checks that the two vectors have the same contents.
  bool operator==(const TVec<TVal, TSizeTy>& Vec) const;
  /// Lexicographically compares two vectors. ##TVec::Less
  bool operator<(const TVec<TVal, TSizeTy>& Vec) const;
  /// Returns a reference to the element at position \c ValN in the vector.
  const TVal& operator[](const TSizeTy& ValN) const {
    AssertR((0<=ValN)&&(ValN<Vals), GetXOutOfBoundsErrMsg(ValN));
    return ValT[ValN];}
  /// Returns a reference to the element at position \c ValN in the vector.
  TVal& operator[](const TSizeTy& ValN){
    AssertR((0<=ValN)&&(ValN<Vals), GetXOutOfBoundsErrMsg(ValN));
    return ValT[ValN];}
  /// Returns the memory footprint (the number of bytes) of the vector.
  uint64 GetMemUsed() const {
	  return TSizeTy(2 * sizeof(TSizeTy) + sizeof(TVal*) + sizeof(TVal)*(MxVals != -1 ? MxVals : 0));
  }
  /// Returns the memory footprint (the number of bytes) of the vector.
  uint64 GetMemUsedDeep() const {
	  uint64 MemSize = 2 * sizeof(TSizeTy) + sizeof(TVal*);
	  if (ValT != NULL && MxVals != -1){
		  for (TSizeTy i = 0; i < MxVals; i++){
			  MemSize += ValT[i].GetMemUsed();
		  }
	  }
	  return MemSize;
  }
  /// Returns the memory size (the number of bytes) of a binary representation.
  TSizeTy GetMemSize(bool flat = true) const {
	  return TSizeTy(2 * sizeof(TVal) + sizeof(TVal)*Vals);
  }
  
  /// Returns primary hash code of the vector. Used by \c THash.
  int GetPrimHashCd() const;
  /// Returns secondary hash code of the vector. Used by \c THash.
  int GetSecHashCd() const;
  
  /// Constructs a vector (an array) of \c _Vals elements.
  void Gen(const TSizeTy& _Vals){ IAssert(0<=_Vals);
    if (ValT!=NULL && MxVals!=-1){delete[] ValT;} MxVals=Vals=_Vals;
    if (MxVals==0){ValT=NULL;} else {ValT=new TVal[MxVals];}}
  /// Constructs a vector (an array) of \c _Vals elements, while reserving enough memory for \c _MxVals elements.
  void Gen(const TSizeTy& _MxVals, const TSizeTy& _Vals){ IAssert((0<=_Vals)&&(_Vals<=_MxVals));
    if (ValT!=NULL  && MxVals!=-1){delete[] ValT;} MxVals=_MxVals; Vals=_Vals;
    if (_MxVals==0){ValT=NULL;} else {ValT=new TVal[_MxVals];}}
  /// Constructs a vector of \c _Vals elements of memory array \c _ValT. ##TVec::GenExt
  void GenExt(TVal *_ValT, const TSizeTy& _Vals){
    if (ValT!=NULL && MxVals!=-1){delete[] ValT;}
    MxVals=-1; Vals=_Vals; ValT=_ValT;}
  /// Returns true if the vector was created using the \c GenExt(). ##TVec::IsExt
  bool IsExt() const {return MxVals==-1;}
  /// Reserves enough memory for the vector to store \c _MxVals elements.
  void Reserve(const TSizeTy& _MxVals){Resize(_MxVals);}
  /// Reserves enough memory for the vector to store \c _MxVals elements and sets its length to \c _Vals.
  void Reserve(const TSizeTy& _MxVals, const TSizeTy& _Vals){ IAssert((0<=_Vals)&&(_Vals<=_MxVals)); Resize(_MxVals); Vals=_Vals;}
  /// Clears the contents of the vector. ##TVec::Clr
  void Clr(const bool& DoDel=true, const TSizeTy& NoDelLim=-1);
  /// Truncates the vector's length and capacity to \c _Vals elements. ##TVec::Trunc
  void Trunc(const TSizeTy& _Vals=-1);
  /// The vector reduces its capacity (frees memory) to match its size.
  void Pack();
  /// Takes over the data and the capacity from \c Vec. ##TVec::MoveFrom
  void MoveFrom(TVec<TVal, TSizeTy>& Vec);
  /// Swaps the contents of the vector with \c Vec.
  void Swap(TVec<TVal, TSizeTy>& Vec);
  
  /// Tests whether the vector is empty. ##TVec::Empty
  bool Empty() const {return Vals==0;}
  /// Returns the number of elements in the vector. ##TVec::Len
  TSizeTy Len() const {return Vals;}
  /// Returns the size of allocated storage capacity.
  TSizeTy Reserved() const {return MxVals;}
  /// Returns a reference to the last element of the vector.
  const TVal& Last() const {return GetVal(Len()-1);}
  /// Returns a reference to the last element of the vector.
  TVal& Last(){return GetVal(Len()-1);}
  /// Returns the position of the last element.
  TSizeTy LastValN() const {return Len()-1;}
  /// Returns a reference to the one before last element of the vector.
  const TVal& LastLast() const { AssertR(1<Vals, GetXOutOfBoundsErrMsg(Vals-2)); return ValT[Vals-2];}
  /// Returns a reference to the one before last element of the vector.
  TVal& LastLast(){ AssertR(1<Vals, GetXOutOfBoundsErrMsg(Vals-2)); return ValT[Vals-2];}
  
  /// Returns an iterator pointing to the first element in the vector.
  TIter BegI() const {return ValT;}
  /// Returns an iterator pointing to the first element in the vector (used by C++11)
  TIter begin() const {return ValT;}
  /// Returns an iterator referring to the past-the-end element in the vector.
  TIter EndI() const {return ValT+Vals;}
  /// Returns an iterator referring to the past-the-end element in the vector (used by C++11))
  TIter end() const {return ValT+Vals;}
  /// Returns an iterator an element at position \c ValN.
  TIter GetI(const TSizeTy& ValN) const {return ValT+ValN;}
  
  /// Adds a new element at the end of the vector, after its current last element. ##TVec::Add
  TSizeTy Add(){ AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    if (Vals==MxVals){Resize();} return Vals++;}
  /// Adds a new element at the end of the vector, after its current last element. ##TVec::Add1
  TSizeTy Add(const TVal& Val){ AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    if (Vals==MxVals){Resize();} ValT[Vals]=Val; return Vals++;}
  TSizeTy Add(TVal& Val){ AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    if (Vals==MxVals){Resize();} ValT[Vals]=Val; return Vals++;}
  /// Adds element \c Val at the end of the vector. #TVec::Add2
  TSizeTy Add(const TVal& Val, const TSizeTy& ResizeLen){ AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    if (Vals==MxVals){Resize(MxVals+ResizeLen);} ValT[Vals]=Val; return Vals++;}
  /// Adds the elements of the vector \c ValV to the to end of the vector.
  TSizeTy AddV(const TVec<TVal, TSizeTy>& ValV);
  /// Adds the elements of the vector \c ValV to the to end of the vector using memcpy.
  TSizeTy AddVMemCpy(const TVec<TVal, TSizeTy>& ValV);
  /// Adds element \c Val to a sorted vector. ##TVec::AddSorted
  TSizeTy AddSorted(const TVal& Val, const bool& Asc=true, const TSizeTy& _MxVals=-1);
  /// Adds element \c Val to a sorted vector. ##TVec::AddBackSorted
  TSizeTy AddBackSorted(const TVal& Val, const bool& Asc);
  /// Adds element \c Val to a sorted vector only if the element \c Val is not already in the vector. ##TVec::AddMerged
  TSizeTy AddMerged(const TVal& Val);
  /// Adds elements of \c ValV to a sorted vector only if a particular element is not already in the vector. ##TVec::AddMerged1
  TSizeTy AddVMerged(const TVec<TVal, TSizeTy>& ValV);
  /// Adds element \c Val to a vector only if the element \c Val is not already in the vector. ##TVec::AddUnique
  TSizeTy AddUnique(const TVal& Val);
  /// Returns a reference to the element at position \c ValN in the vector.
  const TVal& GetVal(const TSizeTy& ValN) const {return operator[](ValN);}
  /// Returns a reference to the element at position \c ValN in the vector.
  TVal& GetVal(const TSizeTy& ValN){return operator[](ValN);}
  /// Sets the value of element at position \c ValN to \c Val.
  void SetVal(const TSizeTy& ValN, const TVal& Val){AssertR((0<=ValN)&&(ValN<Vals), GetXOutOfBoundsErrMsg(ValN)); ValT[ValN] = Val;}
  /// Returns a vector on elements at positions <tt>BValN...EValN</tt> (inclusive).
  void GetSubValV(const TSizeTy& BValN, const TSizeTy& EValN, TVec<TVal, TSizeTy>& ValV) const;
  /// Returns a vector on elements at positions <tt>BValN...EValN</tt> using memcpy.
  void GetSubValVMemCpy(const TSizeTy& _BValN, const TSizeTy& _EValN, TVec<TVal, TSizeTy>& SubValV) const;

  /// Inserts new element \c Val before the element at position \c ValN.
  void Ins(const TSizeTy& ValN, const TVal& Val);
  /// Removes the element at position \c ValN.
  void Del(const TSizeTy& ValN);
  /// Removes the element at position \c ValN using memcpy
  void DelMemCpy(const TSizeTy& ValN);
  /// Removes the elements at positions <tt>MnValN...MxValN</tt>.
  void Del(const TSizeTy& MnValN, const TSizeTy& MxValN);
  /// Removes the elements at positions <tt>MnValN...MxValN</tt> using memcpy
  void DelMemCpy(const TSizeTy& MnValN, const TSizeTy& MxValN);
  /// Removes the last element of the vector.
  void DelLast(){Del(Len()-1);}
  /// Removes the first occurrence of element \c Val.
  bool DelIfIn(const TVal& Val);
  /// Removes all occurrences of element \c Val.
  void DelAll(const TVal& Val);
  /// Sets all elements of the vector to value \c Val.
  void PutAll(const TVal& Val);
  /// removes elements with indices in ValNV
  void Del(const TVec<TNum<TSizeTy>, TSizeTy>& ValNV);

  /// Move element from position \c ValN1 to \c ValN2
  void Move(const TSizeTy& FromValN, const TSizeTy& ToValN){
    ValT[ToValN] = std::move(ValT[FromValN]); } // C++11
    //ValT[ValN2] = ValT[ValN1]; } // C++98
  /// Swaps elements at positions \c ValN1 and \c ValN2.
  void Swap(const TSizeTy& ValN1, const TSizeTy& ValN2){ 
    Assert(0 <= ValN1 && ValN1 < Len() && 0 <= ValN2 && ValN2 < Len());	std::swap(ValT[ValN1], ValT[ValN2]); } // C++11
    // const TVal Val=ValT[ValN1]; ValT[ValN1]=ValT[ValN2]; ValT[ValN2]=Val;} // C++98
  /// Swaps the elements that iterators \c LVal and \c RVal point to.
  static void SwapI(TIter LVal, TIter RVal){ const TVal Val=*LVal; *LVal=*RVal; *RVal=Val;}
  
  /// Generates next permutation of the elements in the vector. ##TVec::NextPerm
  bool NextPerm();
  /// Generates previous permutation of the elements in the vector. ##TVec::PrevPerm
  bool PrevPerm();
  
  // Sorting functions
  /// Picks three random elements at positions <tt>LValN...RValN</tt> and returns the middle one.
  TSizeTy GetPivotValN(const TSizeTy& LValN, const TSizeTy& RValN, TRnd& Rnd) const;
  /// Bubble sorts the values between positions <tt>MnLValN...MxLValN</tt>. ##TVec::BSort
  void BSort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc);
  /// Insertion sorts the values between positions <tt>MnLValN...MxLValN</tt>. ##TVec::ISort
  void ISort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc);
  /// Partitions the values between positions <tt>MnLValN...MxLValN</tt>. ##TVec::Partition
  TSizeTy Partition(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc, TRnd& Rnd);
  /// Quick sorts the values between positions <tt>MnLValN...MxLValN</tt>. ##TVec::QSort
  void QSort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc, TRnd& Rnd);
  /// Sorts the elements of the vector. ##TVec::Sort
  void Sort(TRnd& Rnd, const bool& Asc=true);
  void Sort(const bool& Asc=true) { TRnd Rnd; Sort(Rnd, Asc); }
  /// Sorts the elements of the vector in a new copy and returns the permutation. ##TVec::SortGetPerm
  static void SortGetPerm(const TVec<TVal, TSizeTy>& Vec, TVec<TVal, TSizeTy>& SortedVec, TVec<TInt, TSizeTy>& PermV, bool Asc = true);
  /// Checks whether the vector is sorted in ascending (if \c Asc=true) or descending (if \c Asc=false) order.
  bool IsSorted(const bool& Asc=true) const;
  /// Randomly shuffles the elements of the vector.
  void Shuffle(TRnd& Rnd);
  /// Reverses the order of the elements in the vector.
  void Reverse();
  /// Reverses the order of elements between <tt>LValN...RValN</tt>.
  void Reverse(TSizeTy LValN, TSizeTy RValN){ Assert(LValN>=0 && RValN<Len()); while (LValN < RValN){Swap(LValN++, RValN--);} }
  /// Sorts the vector and only keeps a single element of each value.
  void Merge();

  TStr GetStr() const { return TStr("<Vector>");}
  
  /// Picks three random elements at positions <tt>BI...EI</tt> and returns the middle one under the comparator \c Cmp.
  template <class TCmp>
  static TIter GetPivotValNCmp(const TIter& BI, const TIter& EI, const TCmp& Cmp) {
    TSizeTy SubVals=TSizeTy(EI-BI); if (SubVals > TInt::Mx-1) { SubVals = TInt::Mx-1; }
    const TSizeTy ValN1=TInt::GetRnd(SubVals), ValN2=TInt::GetRnd(SubVals), ValN3=TInt::GetRnd(SubVals);
    const TVal& Val1 = *(BI+ValN1);  const TVal& Val2 = *(BI+ValN2); const TVal& Val3 = *(BI+ValN3);
    if (Cmp(Val1, Val2)) {
      if (Cmp(Val2, Val3)) return BI+ValN2;
      else if (Cmp(Val3, Val1)) return BI+ValN1;
      else return BI+ValN3;
    } else {
      if (Cmp(Val1, Val3)) return BI+ValN1;
      else if (Cmp(Val3, Val2)) return BI+ValN2;
      else return BI+ValN3; } }
  /// Partitions the values between positions <tt>BI...EI</tt> under the comparator \c Cmp.
  template <class TCmp>
  static TIter PartitionCmp(TIter BI, TIter EI, const TVal Pivot, const TCmp& Cmp) {
    forever {
      while (Cmp(*BI, Pivot)){++BI;}  --EI;
      while (Cmp(Pivot, *EI)){--EI;}
      if (!(BI<EI)){return BI;}  SwapI(BI, EI);  ++BI; } }
  /// Bubble sorts the values between positions <tt>BI...EI</tt> under the comparator \c Cmp.
  template <class TCmp>
  static void BSortCmp(TIter BI, TIter EI, const TCmp& Cmp) {
    for (TIter i = BI; i != EI; ++i) {
      for (TIter j = EI-1; j != i; --j) {
        if (Cmp(*j, *(j-1))) { SwapI(j, j-1); } } } }
  /// Insertion sorts the values between positions <tt>BI...EI</tt> under the comparator \c Cmp.
  template <class TCmp>
  static void ISortCmp(TIter BI, TIter EI, const TCmp& Cmp) {
    if (BI + 1 < EI) {
      for (TIter i = BI, j; i != EI; ++i) { TVal Tmp=*i;  j=i;
        while (j > BI && Cmp(Tmp, *(j-1))) { *j = *(j-1); --j; } *j=Tmp; } } }
  /// Quick sorts the values between positions <tt>BI...EI</tt> under the comparator \c Cmp.
  template <class TCmp>
  static void QSortCmp(TIter BI, TIter EI, const TCmp& Cmp) {
    if (BI + 1 < EI) {
      if (EI - BI < 20) { ISortCmp(BI, EI, Cmp); }
      else { const TVal Val = *GetPivotValNCmp(BI, EI, Cmp);
        TIter Split = PartitionCmp(BI, EI, Val, Cmp);
        QSortCmp(BI, Split, Cmp);  QSortCmp(Split, EI, Cmp); } } }
  /// Sorts the elements of the vector using the comparator \c Cmp.
  template <class TCmp>
  void SortCmp(const TCmp& Cmp){ QSortCmp(BegI(), EndI(), Cmp);}
  /// Checks whether the vector is sorted according to the comparator \c Cmp.
  template <class TCmp>
  bool IsSortedCmp(const TCmp& Cmp) const {
    if (EndI() == BegI()) return true;
    for (TIter i = BegI(); i != EndI()-1; ++i) {
      if (Cmp(*(i+1), *i)){return false;} } return true; }
  
  /// Result is the intersection of \c this vector with \c ValV. ##TVec::Intrs
  void Intrs(const TVec<TVal, TSizeTy>& ValV);
  /// Result is the union of \c this vector with \c ValV. ##TVec::Union
  void Union(const TVec<TVal, TSizeTy>& ValV);
  /// Subtracts \c ValV from \c this vector. ##TVec::Diff
  void Diff(const TVec<TVal, TSizeTy>& ValV);
  /// \c DstValV is the intersection of vectors \c this and \c ValV. ##TVec::Intrs1
  void Intrs(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const;
  /// \c DstValV is the union of vectors \c this and \c ValV. ##TVec::Union1
  void Union(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const;
  /// \c DstValV is the difference of vectors \c this and \c ValV. ##TVec::Diff1
  void Diff(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const;
  /// Returns the size of the intersection of vectors \c this and \c ValV. ##TVec::IntrsLen
  TSizeTy IntrsLen(const TVec<TVal, TSizeTy>& ValV) const;
  /// Returns the size of the union of vectors \c this and \c ValV. ##TVec::UnionLen
  TSizeTy UnionLen(const TVec<TVal, TSizeTy>& ValV) const;

  /// Counts the number of occurrences of \c Val in the vector.
  TSizeTy Count(const TVal& Val) const;
  /// Returns the position of an element with value \c Val. ##TVec::SearchBin
  TSizeTy SearchBin(const TVal& Val) const;
  /// Returns the position of an element with value \c Val. ##TVec::SearchBin1
  TSizeTy SearchBin(const TVal& Val, TSizeTy& InsValN) const;
  /// Returns the position of an element with value \c Val. ##TVec::SearchForw
  TSizeTy SearchForw(const TVal& Val, const TSizeTy& BValN=0) const;
  /// Returns the position of an element with value \c Val. ##TVec::SearchBack
  TSizeTy SearchBack(const TVal& Val) const;
  /// Returns the starting position of vector \c ValV. ##TVec::SearchVForw
  TSizeTy SearchVForw(const TVec<TVal, TSizeTy>& ValV, const TSizeTy& BValN=0) const;
  /// Returns the indexes of all the occurences of the element
  void FindAll(const TVal& Val, TVec<TInt, TSizeTy>& IdxV) const;
  /// Returns the indexes of all the elements that match the criteria
  template <typename TFun>
  void FindAllSatisfy(const TFun& Fun, TVec<TInt, TSizeTy>& IdxV) const;

  /// Checks whether element \c Val is a member of the vector.
  bool IsIn(const TVal& Val) const {return SearchForw(Val)!=-1;}
  /// Checks whether element \c Val is a member of the vector. ##TVec::IsIn
  bool IsIn(const TVal& Val, TSizeTy& ValN) const { ValN=SearchForw(Val); return ValN!=-1;}
  /// Checks whether element \c Val is a member of the vector. ##TVec::IsInBin
  bool IsInBin(const TVal& Val) const {return SearchBin(Val)!=-1;}
  /// Returns reference to the first occurrence of element \c Val.
  const TVal& GetDat(const TVal& Val) const { return GetVal(SearchForw(Val));}
  /// Returns reference to the first occurrence of element \c Val. ##TVec::GetAddDat
  TVal& GetAddDat(const TVal& Val){ AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    const TSizeTy ValN=SearchForw(Val); if (ValN==-1){Add(Val); return Last();} else {return GetVal(ValN);}}
  /// Returns the position of the largest element in the vector.
  TSizeTy GetMxValN() const;
  /// Returns the position of the smallest element in the vector.
  TSizeTy GetMnValN() const;
  /// Returns the largest element in the vector \c Val. ###TVec::GetMxVal
  const TVal& GetMxVal() const {return GetVal(GetMxValN());}
  /// Returns the smallest element in the vector \c Val. ###TVec::GetMnVal
  const TVal& GetMnVal() const { return GetVal(GetMnValN()); }
  
  /// Returns a vector on element \c Val1.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1){
    TVec<TVal, TSizeTy> V(1, 0); V.Add(Val1); return V;}
  /// Returns a vector on elements \c Val1, \c Val2.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2){
    TVec<TVal, TSizeTy> V(2, 0); V.Add(Val1); V.Add(Val2); return V;}
  /// Returns a vector on elements <tt>Val1...Val3</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3){
    TVec<TVal, TSizeTy> V(3, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); return V;}
  /// Returns a vector on elements <tt>Val1...Val4</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4){
    TVec<TVal, TSizeTy> V(4, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); return V;}
  /// Returns a vector on elements <tt>Val1...Val5</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4, const TVal& Val5){
    TVec<TVal, TSizeTy> V(5, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); V.Add(Val5); return V;}
  /// Returns a vector on elements <tt>Val1...Val6</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4, const TVal& Val5, const TVal& Val6){
    TVec<TVal, TSizeTy> V(6, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); V.Add(Val5); V.Add(Val6); return V;}
  /// Returns a vector on elements <tt>Val1...Val7</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4, const TVal& Val5, const TVal& Val6, const TVal& Val7){
    TVec<TVal, TSizeTy> V(7, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); V.Add(Val5); V.Add(Val6); V.Add(Val7); return V;}
  /// Returns a vector on elements <tt>Val1...Val8</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4, const TVal& Val5, const TVal& Val6, const TVal& Val7, const TVal& Val8){
    TVec<TVal, TSizeTy> V(8, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); V.Add(Val5); V.Add(Val6); V.Add(Val7); V.Add(Val8); return V;}
  /// Returns a vector on elements <tt>Val1...Val9</tt>.
  static TVec<TVal, TSizeTy> GetV(const TVal& Val1, const TVal& Val2, const TVal& Val3, const TVal& Val4, const TVal& Val5, const TVal& Val6, const TVal& Val7, const TVal& Val8, const TVal& Val9){
    TVec<TVal, TSizeTy> V(9, 0); V.Add(Val1); V.Add(Val2); V.Add(Val3); V.Add(Val4); V.Add(Val5); V.Add(Val6); V.Add(Val7); V.Add(Val8); V.Add(Val9); return V;}
};

//#//////////////////////////////////////////////
/// TSIter is a stride iterator. ##TVec
template <class TVal, class TSizeTy = int>
class TSIter{
protected:
	TVec<TVal, TSizeTy>* Vector;
	TSizeTy start;
	TSizeTy stride;
public:
	TSIter<TVal, TSizeTy>(){
		Vector = NULL;
		start = 0;
		stride = 1;
	}
	TSIter<TVal, TSizeTy>(TVec<TVal, TSizeTy>* Vec, TSizeTy _start = 0, TSizeTy _stride = 1){
		Vector = Vec;
		start = _start;
		stride = _stride;
	}
	const TVal& operator[](const TSizeTy& El) const {
		TSizeTy index = start + El*stride;
		AssertR((0 <= index) && (index < Vector->Vals), GetXOutOfBoundsErrMsg(index));
		return (*Vector)[index];
	}
	TVal& operator[](const TSizeTy& El){
		TSizeTy index = start + El*stride;
		AssertR((0 <= index) && (index < Vector->Vals), GetXOutOfBoundsErrMsg(index));
		return (*Vector)[index];
	}
};



/////////////////////////////////////////////////
// Common-Vector-Types
typedef TVec<TBool> TBoolV;
typedef TVec<TCh> TChV;
typedef TVec<TUCh> TUChV;
typedef TVec<TUInt> TUIntV;
typedef TVec<TInt> TIntV;
typedef TVec<TUInt64> TUInt64V;
typedef TVec<TFlt> TFltV;
typedef TVec<TSFlt> TSFltV;
typedef TVec<TAscFlt> TAscFltV;
//typedef TVec<TStr> TStrV; // defined in dt.h
typedef TVec<TChA> TChAV;
typedef TVec<TIntPr> TIntPrV;
typedef TVec<TIntTr> TIntTrV;
typedef TVec<TIntQu> TIntQuV;
typedef TVec<TFltPr> TFltPrV;
typedef TVec<TFltTr> TFltTrV;
typedef TVec<TIntKd> TIntKdV;
typedef TVec<TUChIntPr> TUChIntPrV;
typedef TVec<TUChUInt64Pr> TUChUInt64PrV;
typedef TVec<TIntUInt64Pr> TIntUInt64PrV;
typedef TVec<TIntUInt64Kd> TIntUInt64KdV;
typedef TVec<TIntFltPr> TIntFltPrV;
typedef TVec<TIntFltPrKd> TIntFltPrKdV;
typedef TVec<TFltIntPr> TFltIntPrV;
typedef TVec<TFltUInt64Pr> TFltUInt64PrV;
typedef TVec<TFltStrPr> TFltStrPrV;
typedef TVec<TAscFltStrPr> TAscFltStrPrV;
typedef TVec<TIntUChPr> TIntUChPrV;
typedef TVec<TIntStrPr> TIntStrPrV;
typedef TVec<TIntIntStrTr> TIntIntStrTrV;
typedef TVec<TIntIntFltTr> TIntIntFltTrV;
typedef TVec<TIntFltIntTr> TIntFltIntTrV;
typedef TVec<TIntFltFltTr> TIntFltFltTrV;
typedef TVec<TFltIntFltTr> TFltIntFltTrV;
typedef TVec<TIntStrIntTr> TIntStrIntTrV;
typedef TVec<TIntKd> TIntKdV;
typedef TVec<TUIntIntKd> TUIntIntKdV;
typedef TVec<TIntFltKd> TIntFltKdV;
typedef TVec<TIntPrFltKd> TIntPrFltKdV;
typedef TVec<TIntStrKd> TIntStrKdV;
typedef TVec<TIntStrPrPr> TIntStrPrPrV;
typedef TVec<TIntStrVPr> TIntStrVPrV;
typedef TVec<TIntIntVIntTr> TIntIntVIntTrV;
typedef TVec<TIntIntIntVTr> TIntIntIntVTrV;
typedef TVec<TUInt64UInt64IntTr> TUInt64UInt64IntTrV;
typedef TVec<TUInt64IntPr> TUInt64IntPrV;
typedef TVec<TUInt64FltPr> TUInt64FltPrV;
typedef TVec<TUInt64StrPr> TUInt64StrPrV;
typedef TVec<TUInt64IntKd> TUInt64IntKdV;
typedef TVec<TUInt64FltKd> TUInt64FltKdV;
typedef TVec<TUInt64StrKd> TUInt64StrKdV;
typedef TVec<TFltBoolKd> TFltBoolKdV;
typedef TVec<TFltIntKd> TFltIntKdV;
typedef TVec<TFltUInt64Kd> TFltUInt64KdV;
typedef TVec<TFltIntPrKd> TFltIntPrKdV;
typedef TVec<TFltKd> TFltKdV;
typedef TVec<TFltStrKd> TFltStrKdV;
typedef TVec<TFltStrPrPr> TFltStrPrPrV;
typedef TVec<TFltIntIntTr> TFltIntIntTrV;
typedef TVec<TFltFltStrTr> TFltFltStrTrV;
typedef TVec<TAscFltIntPr> TAscFltIntPrV;
typedef TVec<TAscFltIntKd> TAscFltIntKdV;
typedef TVec<TStrPr> TStrPrV;
typedef TVec<TStrIntPr> TStrIntPrV;
typedef TVec<TStrFltPr> TStrFltPrV;
typedef TVec<TStrIntKd> TStrIntKdV;
typedef TVec<TStrFltKd> TStrFltKdV;
typedef TVec<TStrAscFltKd> TStrAscFltKdV;
typedef TVec<TStrTr> TStrTrV;
typedef TVec<TStrQu> TStrQuV;
typedef TVec<TStrFltFltTr> TStrFltFltTrV;
typedef TVec<TStrStrIntTr> TStrStrIntTrV;
typedef TVec<TStrKd> TStrKdV;
typedef TVec<TStrStrVPr> TStrStrVPrV;
typedef TVec<TStrVIntPr> TStrVIntPrV;
typedef TVec<TFltIntIntIntQu> TFltIntIntIntQuV;
typedef TVec<TIntStrIntIntQu> TIntStrIntIntQuV;
typedef TVec<TIntIntPrPr> TIntIntPrPrV;

//#//////////////////////////////////////////////
/// Vector Pool. ##TVecPool
template <class TVal, class TSizeTy=int>
class TVecPool {
public:
  typedef TPt<TVecPool<TVal, TSizeTy> > PVecPool;
  typedef TVec<TVal, TSizeTy> TValV;
private:
  TCRef CRef;
  TBool FastCopy;
  TSize GrowBy, MxVals, Vals;
  TVal EmptyVal;                // Empty value/vector
  TVal *ValBf;                  // Buffer for storing all the values
  TVec<uint64, int> IdToOffV;   // Id to one past last (Vector starts at [id-1]). Vector length is IdToOff[id]-IdToOff[id-1]
private:
  void Resize(const TSize& _MxVals);
public:
  /// Vector pool constructor. ##TVecPool::TVecPool
  TVecPool(const TSize& ExpectVals=0, const TSize& _GrowBy=1000000, const bool& _FastCopy=false, const TVal& _EmptyVal=TVal());
  TVecPool(const TVecPool<TVal, TSizeTy>& Pool);
  TVecPool(TSIn& SIn);
  ~TVecPool() { if (ValBf != NULL) { delete [] ValBf; } ValBf=NULL; }
  static PVecPool New(const TSize& ExpectVals=0, const TSize& GrowBy=1000000, const bool& FastCopy=false) {
    return new TVecPool(ExpectVals, GrowBy, FastCopy); }
  static PVecPool Load(TSIn& SIn) { return new TVecPool(SIn); }
  static PVecPool Load(const TStr& FNm) { TFIn FIn(FNm); return Load(FIn); }
  void Save(TSOut& SOut) const;
  TVecPool& operator = (const TVecPool& Pool);
  
  /// Returns the total number of vectors stored in the vector pool.
  int GetVecs() const { return IdToOffV.Len(); }
  /// Returns the total number of values stored in the vector pool.
  TSize GetVals() const { return Vals; }
  /// Tests whether vector of id \c VId is in the pool.
  bool IsVId(const int& VId) const { return (0 <= VId) && (VId < IdToOffV.Len()); }
  /// Returns the total capacity of the pool.
  uint64 Reserved() const { return MxVals; }
  /// Reserves enough capacity for the pool to store \c MxVals elements.
  void Reserve(const TSize& MxVals) { Resize(MxVals); }
  /// Returns the reference to an empty value.
  const TVal& GetEmptyVal() const { return EmptyVal; }
  /// Sets the empty value.
  void SetEmptyVal(const TVal& _EmptyVal) { EmptyVal = _EmptyVal; }
  /// Returns the total memory footprint (in bytes) of the pool.
  uint64 GetMemUsed() const {
    return sizeof(TCRef)+sizeof(TBool)+3*sizeof(TSize)+sizeof(TVal*)+MxVals*sizeof(TVal);}
  
  /// Adds vector \c ValV to the pool and returns its id.
  int AddV(const TValV& ValV);
  /// Adds a vector of length \c ValVLen to the pool and returns its id. ##TVecPool::AddEmptyV
  int AddEmptyV(const int& ValVLen);
  /// Returns the number of elements in the vector with id \c VId.
  int GetVLen(const int& VId) const { if (VId==0){return 0;} else {return int(IdToOffV[VId]-IdToOffV[VId-1]);}}
  /// Returns pointer to the first element of the vector with id \c VId.
  TVal* GetValVPt(const int& VId) const {
    if (GetVLen(VId)==0){return (TVal*)&EmptyVal;}
    else {return ValBf+IdToOffV[VId-1];}}
  /// Returns \c ValV which is a reference (not a copy) to vector with id \c VId. ##TVecPool::GetV
  void GetV(const int& VId, TValV& ValV) const {
    if (GetVLen(VId)==0){ValV.Clr();}
    else { ValV.GenExt(GetValVPt(VId), GetVLen(VId)); } }
  /// Sets the values of vector \c VId with those in \c ValV.
  void PutV(const int& VId, const TValV& ValV) {
    IAssert(IsVId(VId) && GetVLen(VId) == ValV.Len());
    if (FastCopy) {
      memcpy(GetValVPt(VId), ValV.BegI(), sizeof(TVal)*ValV.Len()); }
    else { TVal* ValPt = GetValVPt(VId);
      for (::TSize ValN=0; ValN < ::TSize(ValV.Len()); ValN++, ValPt++) { *ValPt=ValV[ValN]; }
    } }
  /// Deletes all elements of value \c DelVal from all vectors. ##TVecPool::CompactPool
  void CompactPool(const TVal& DelVal);
  /// Shuffles the order of all elements in the pool. ##TVecPool::ShuffleAll
  void ShuffleAll(TRnd& Rnd);
  void ShuffleAll() { TRnd Rnd; ShuffleAll(Rnd); }
  
  /// Clears the contents of the pool. ##TVecPool::Clr
  void Clr(bool DoDel = true) {
    IdToOffV.Clr(DoDel);  MxVals=0;  Vals=0;
    if (DoDel && ValBf!=NULL) { delete [] ValBf; ValBf=NULL;}
    if (! DoDel) { PutAll(EmptyVal); } }
  /// Sets the values of all elements in the pool to \c Val.
  void PutAll(const TVal& Val) {
    for (TSize ValN = 0; ValN < MxVals; ValN++) { ValBf[ValN]=Val; } }
  friend class TPt<TVecPool<TVal> >;
};


///////////////////////////////
// Linked queue
template <class TVal, class TSizeTy = TUInt64>
class TLinkedQueue {
private:
	class TNode {
	public:
		TNode* Next;
		TVal Val;

		TNode(TNode* Next, const TVal& Val);
	};

	TNode* First;
	TNode* Last;
	TSizeTy Size;

public:
	TLinkedQueue();
	TLinkedQueue(TSIn& SIn);
	~TLinkedQueue();

	void Save(TSOut& SOut) const;

	void Push(const TVal& Val);
	TVal Pop();
	TVal& Peek();
	void DelFirst();

	bool Empty() const { return Len() == 0; };
	TSizeTy Len() const { return Size; };
};

/////////////////////////////////////////////////
// Below are old 32-bit implementations of TVec and other classes.
// Old TVec takes at most 2G elements.
// The new vector class supports 64-bits for the number of elements,
// but also allows 32-bits for backward compatibility.
// by Jure (Jan 2013)
namespace TGLib_OLD {
/////////////////////////////////////////////////
// Vector Pool
template<class TVal>
class TVecPool {
public:
  typedef TPt<TVecPool<TVal> > PVecPool;
  typedef TVec<TVal> TValV;
private:
  TCRef CRef;
  TBool FastCopy;
  ::TSize GrowBy, MxVals, Vals;
  TVal EmptyVal;           // empty vector
  TVal *ValBf;             // buffer storing all the values
  TVec< ::TSize> IdToOffV; // id to one past last (vector starts at [id-1])
private:
  void Resize(const ::TSize& _MxVals);
public:
  TVecPool(const ::TSize& ExpectVals=0, const ::TSize& _GrowBy=1000000, const bool& _FastCopy=false, const TVal& _EmptyVal=TVal());
  TVecPool(const TVecPool& Pool);
  TVecPool(TSIn& SIn);
  ~TVecPool() { if (ValBf != NULL) { delete [] ValBf; } ValBf=NULL; }
  static PVecPool New(const ::TSize& ExpectVals=0, const ::TSize& GrowBy=1000000, const bool& FastCopy=false) {
    return new TVecPool(ExpectVals, GrowBy, FastCopy); }
  static PVecPool Load(TSIn& SIn) { return new TVecPool(SIn); }
  static PVecPool Load(const TStr& FNm) { TFIn FIn(FNm); return Load(FIn); }
  void Save(TSOut& SOut) const;

  TVecPool& operator = (const TVecPool& Pool);

  ::TSize GetVals() const { return Vals; }
  ::TSize GetVecs() const { return IdToOffV.Len(); }
  bool IsVId(const int& VId) const { return (0 <= VId) && (VId < IdToOffV.Len()); }
  ::TSize Reserved() const { return MxVals; }
  void Reserve(const ::TSize& MxVals) { Resize(MxVals); }
  const TVal& GetEmptyVal() const { return EmptyVal; }
  void SetEmptyVal(const TVal& _EmptyVal) { EmptyVal = _EmptyVal; }
  ::TSize GetMemUsed() const {
    return sizeof(TCRef)+sizeof(TBool)+3*sizeof(TSize)+sizeof(TVal*)+MxVals*sizeof(TVal);}

  int AddV(const TValV& ValV);
  int AddEmptyV(const int& ValVLen);
  uint GetVLen(const int& VId) const {
    if (VId==0){return 0;}
    else {return uint(IdToOffV[VId]-IdToOffV[VId-1]);}}
  TVal* GetValVPt(const int& VId) const {
    if (GetVLen(VId)==0){return (TVal*)&EmptyVal;}
    else {return ValBf+IdToOffV[VId-1];}}
  void GetV(const int& VId, TValV& ValV) const {
    if (GetVLen(VId)==0){ValV.Clr();}
    else { ValV.GenExt(GetValVPt(VId), GetVLen(VId)); } }
  void PutV(const int& VId, const TValV& ValV) {
    IAssert(IsVId(VId) && GetVLen(VId) == ValV.Len());
    if (FastCopy) {
      memcpy(GetValVPt(VId), ValV.BegI(), sizeof(TVal)*ValV.Len()); }
    else { TVal* ValPt = GetValVPt(VId);
      for (uint ValN=0; ValN < uint(ValV.Len()); ValN++, ValPt++) { *ValPt=ValV[ValN]; }
    }
  }
  void CompactPool(const TVal& DelVal); // delete all elements of value DelVal from all vectors
  void ShuffleAll(TRnd& Rnd); // shuffles all the order of elements in the Pool (does not respect vector boundaries)
  void ShuffleAll() { TRnd Rnd; ShuffleAll(Rnd); }

  //bool HasIdMap() const { return ! IdToOffV.Empty(); }
  //void ClrIdMap() { IdToOffV.Clr(true); }
  void Clr(bool DoDel = true) {
    IdToOffV.Clr(DoDel);  MxVals=0;  Vals=0;
    if (DoDel && ValBf!=NULL) { delete [] ValBf; ValBf=NULL;}
    if (! DoDel) { PutAll(EmptyVal); }
  }
  void PutAll(const TVal& Val) {
    for (TSize ValN = 0; ValN < MxVals; ValN++) { ValBf[ValN]=Val; } }

  friend class TPt<TVecPool<TVal> >;
};

template <class TVal>
void TVecPool<TVal>::Resize(const ::TSize& _MxVals){
  if (_MxVals <= MxVals){ return; } else { MxVals = _MxVals; }
  if (ValBf == NULL) {
    try { ValBf = new TVal [MxVals]; }
    catch (std::exception Ex) {
      FailR(TStr::Fmt("TVecPool::Resize 1: %s, MxVals: %d. [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), _MxVals).CStr()); }
    IAssert(ValBf != NULL);
    if (EmptyVal != TVal()) { PutAll(EmptyVal); }
  } else {
    // printf("*** Resize vector pool: %llu -> %llu\n", uint64(Vals), uint64(MxVals));
    TVal* NewValBf = NULL;
    try { NewValBf = new TVal [MxVals]; }
    catch (std::exception Ex) { FailR(TStr::Fmt("TVecPool::Resize 2: %s, MxVals: %d. [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), _MxVals).CStr()); }
    IAssert(NewValBf != NULL);
    if (FastCopy) {
      memcpy(NewValBf, ValBf, Vals*sizeof(TVal)); }
    else {
      for (TSize ValN = 0; ValN < Vals; ValN++){ NewValBf[ValN] = ValBf[ValN]; } }
    if (EmptyVal != TVal()) { // init empty values
      for (TSize ValN = Vals; ValN < MxVals; ValN++) { NewValBf[ValN] = EmptyVal; }
    }
    delete [] ValBf;
    ValBf = NewValBf;
  }
}

template <class TVal>
TVecPool<TVal>::TVecPool(const ::TSize& ExpectVals, const ::TSize& _GrowBy, const bool& _FastCopy, const TVal& _EmptyVal) :
  GrowBy(_GrowBy), MxVals(0), Vals(0), EmptyVal(_EmptyVal), ValBf(NULL) {
  IdToOffV.Add(0);
  Resize(ExpectVals);
}

template <class TVal>
TVecPool<TVal>::TVecPool(const TVecPool& Pool):
  FastCopy(Pool.FastCopy), GrowBy(Pool.GrowBy),
  MxVals(Pool.MxVals), Vals(Pool.Vals), EmptyVal(Pool.EmptyVal), IdToOffV(Pool.IdToOffV) {
  try { ValBf = new TVal [MxVals]; }
  catch (std::exception Ex) { FailR(TStr::Fmt("TVecPool::TVecPool: %s, MxVals: %d", Ex.what(), MxVals).CStr()); }
  IAssert(ValBf != NULL);
  if (FastCopy) {
    memcpy(ValBf, Pool.ValBf, MxVals*sizeof(TVal)); }
  else {
    for (TSize ValN = 0; ValN < MxVals; ValN++){ ValBf[ValN] = Pool.ValBf[ValN]; } }
}

template <class TVal>
TVecPool<TVal>::TVecPool(TSIn& SIn):
  FastCopy(SIn) {
  uint64 _GrowBy, _MxVals, _Vals;
  SIn.Load(_GrowBy); SIn.Load(_MxVals);  SIn.Load(_Vals);
  IAssert(_GrowBy<TSizeMx && _MxVals<TSizeMx && _Vals<TSizeMx);
  GrowBy=TSize(_GrowBy);  MxVals=TSize(_Vals);  Vals=TSize(_Vals); //note MxVals==Vals
  EmptyVal = TVal(SIn);
  if (MxVals==0) { ValBf = NULL; } else { ValBf = new TVal [MxVals]; }
  for (TSize ValN = 0; ValN < Vals; ValN++) { ValBf[ValN] = TVal(SIn); }
  { TInt MxVals(SIn), Vals(SIn);
  IdToOffV.Gen(Vals);
  for (int ValN = 0; ValN < Vals; ValN++) {
    uint64 Offset;  SIn.Load(Offset);  IAssert(Offset < TSizeMx);
    IdToOffV[ValN]=TSize(Offset);
  } }
}

template <class TVal>
void TVecPool<TVal>::Save(TSOut& SOut) const {
  SOut.Save(FastCopy);
  uint64 _GrowBy=GrowBy, _MxVals=MxVals, _Vals=Vals;
  SOut.Save(_GrowBy); SOut.Save(_MxVals);  SOut.Save(_Vals);
  SOut.Save(EmptyVal);
  for (TSize ValN = 0; ValN < Vals; ValN++) { ValBf[ValN].Save(SOut); }
  { SOut.Save(IdToOffV.Len());  SOut.Save(IdToOffV.Len());
  for (int ValN = 0; ValN < IdToOffV.Len(); ValN++) {
    const uint64 Offset=IdToOffV[ValN];  SOut.Save(Offset);
  } }
}

template <class TVal>
TVecPool<TVal>& TVecPool<TVal>::operator = (const TVecPool& Pool) {
  if (this!=&Pool) {
    FastCopy = Pool.FastCopy;
    GrowBy = Pool.GrowBy;
    MxVals = Pool.MxVals;
    Vals = Pool.Vals;
    EmptyVal = Pool.EmptyVal;
    IdToOffV=Pool.IdToOffV;
    try { ValBf = new TVal [MxVals]; }
    catch (std::exception Ex) { FailR(TStr::Fmt("TVec::operator= : %s, MxVals: %d", Ex.what(), MxVals).CStr()); }
    IAssert(ValBf != NULL);
    if (FastCopy) {
      memcpy(ValBf, Pool.ValBf, Vals*sizeof(TVal)); }
    else {
      for (uint64 ValN = 0; ValN < Vals; ValN++){ ValBf[ValN] = Pool.ValBf[ValN]; } }
  }
  return *this;
}

template<class TVal>
int TVecPool<TVal>::AddV(const TValV& ValV) {
  const ::TSize ValVLen = ValV.Len();
  if (ValVLen == 0) { return 0; }
  if (MxVals < Vals+ValVLen) { Resize(Vals+MAX(ValVLen, GrowBy)); }
  if (FastCopy) { memcpy(ValBf+Vals, ValV.BegI(), sizeof(TVal)*ValV.Len()); }
  else { for (uint ValN=0; ValN < ValVLen; ValN++) { ValBf[Vals+ValN]=ValV[ValN]; } }
  Vals+=ValVLen;  IdToOffV.Add(Vals);
  return IdToOffV.Len()-1;
}

template<class TVal>
int TVecPool<TVal>::AddEmptyV(const int& ValVLen) {
  if (ValVLen==0){return 0;}
  if (MxVals < Vals+ValVLen){Resize(Vals+MAX(TSize(ValVLen), GrowBy)); }
  Vals+=ValVLen; IdToOffV.Add(Vals);
  return IdToOffV.Len()-1;
}

// delete all elements of value DelVal from all vectors
// empty space is left at the end of the pool
template<class TVal>
void TVecPool<TVal>::CompactPool(const TVal& DelVal) {
  ::TSize TotalDel=0, NDel=0;
  // printf("Compacting %d vectors\n", IdToOffV.Len());
  for (int vid = 1; vid < IdToOffV.Len(); vid++) {
    // if (vid % 10000000 == 0) { printf(" %dm", vid/1000000);  fflush(stdout); }
    const uint Len = GetVLen(vid);
    TVal* ValV = GetValVPt(vid);
    if (TotalDel > 0) { IdToOffV[vid-1] -= TotalDel; } // update end of vector
    if (Len == 0) { continue; }
    NDel = 0;
    for (TVal* v = ValV; v < ValV+Len-NDel; v++) {
      if (*v == DelVal) {
        TVal* Beg = v;
        while (*v == DelVal && v < ValV+Len) { v++; NDel++; }
        memcpy(Beg, v, sizeof(TVal)*int(Len - ::TSize(v - ValV)));
        v -= NDel;
      }
    }
    memcpy(ValV-TotalDel, ValV, sizeof(TVal)*Len);  // move data
    TotalDel += NDel;
  }
  IdToOffV.Last() -= TotalDel;
  for (::TSize i = Vals-TotalDel; i < Vals; i++) { ValBf[i] = EmptyVal; }
  Vals -= TotalDel;
  // printf("  deleted %llu elements from the pool\n", TotalDel);
}

// shuffles all the order of elements in the pool (does not respect vector boundaries)
template<class TVal>
void TVecPool<TVal>::ShuffleAll(TRnd& Rnd) {
  for (::TSize n = Vals-1; n > 0; n--) {
    const ::TSize k = ::TSize(((uint64(Rnd.GetUniDevInt())<<32) | uint64(Rnd.GetUniDevInt())) % (n+1));
    const TVal Tmp = ValBf[n];
    ValBf[n] = ValBf[k];
    ValBf[k] = Tmp;
  }
}

}; // namespace TGLib_OLD

typedef TVecPool<TInt> TIntVecPool;
typedef TPt<TIntVecPool> PIntVecPool;

/////////////////////////////////////////////////
// Vector-Pointer
template <class TVal>
class PVec{
private:
  TCRef CRef;
public:
  TVec<TVal> V;
public:
  PVec<TVal>(): V(){}
  PVec<TVal>(const PVec<TVal>& Vec): V(Vec.V){}
  static TPt<PVec<TVal> > New(){
    return new PVec<TVal>();}
  PVec<TVal>(const int& MxVals, const int& Vals): V(MxVals, Vals){}
  static TPt<PVec<TVal> > New(const int& MxVals, const int& Vals){
    return new PVec<TVal>(MxVals, Vals);}
  PVec<TVal>(const TVec<TVal>& _V): V(_V){}
  static TPt<PVec<TVal> > New(const TVec<TVal>& V){
    return new PVec<TVal>(V);}
  explicit PVec<TVal>(TSIn& SIn): V(SIn){}
  static TPt<PVec<TVal> > Load(TSIn& SIn){return new PVec<TVal>(SIn);}
  void Save(TSOut& SOut) const {V.Save(SOut);}

  PVec<TVal>& operator=(const PVec<TVal>& Vec){
    if (this!=&Vec){V=Vec.V;} return *this;}
  bool operator==(const PVec<TVal>& Vec) const {return V==Vec.V;}
  bool operator<(const PVec<TVal>& Vec) const {return V<Vec.V;}
  TVal& operator[](const int& ValN) const {return V[ValN];}

  bool Empty() const {return V.Empty();}
  int Len() const {return V.Len();}
  TVal GetVal(const int& ValN) const {return V[ValN];}

  int Add(const TVal& Val){return V.Add(Val);}

  friend class TPt<PVec<TVal> >;
};

/////////////////////////////////////////////////
// Common-Vector-Pointer-Types
typedef PVec<TFlt> TFltVP;
typedef TPt<TFltVP> PFltV;
typedef PVec<TAscFlt> TAscFltVP;
typedef TPt<TAscFltVP> PAscFltV;
typedef PVec<TStr> TStrVP;
typedef TPt<TStrVP> PStrV;

/////////////////////////////////////////////////
// 2D-Vector
template <class TVal, class TSizeTy = int, bool colmajor = false>
class TVVec{
protected:
	TSizeTy XDim, YDim;
	TVec<TVal, TSizeTy> ValV;
	TBool ColMajor;
	//TSizeTy XStride, YStride;
public:
	TVVec() : XDim(), YDim(), ValV(), ColMajor(){}
	TVVec(const TVVec& Vec) :
		XDim(Vec.XDim), YDim(Vec.YDim), ValV(Vec.ValV), ColMajor(Vec.ColMajor){}
	TVVec(const TVec<TVal, TSizeTy>& Vec) :
		XDim(colmajor ? Vec.Len() : 1), YDim(colmajor ? 1 : Vec.Len()), ValV(Vec), ColMajor(colmajor){}
	TVVec(const TVec<TVal, TSizeTy> && Vec) :
		XDim(colmajor ? Vec.Len() : 1), YDim(colmajor ? 1 : Vec.Len()), ValV(std::move(Vec)), ColMajor(colmajor){}
	//-------------------------------------------------------------------------------------------
#ifdef GLib_CPP11
	TVVec(const TVVec&& Vec) :
		XDim(std::move(Vec.XDim)), YDim(std::move(Vec.YDim)), ValV(std::move(Vec.ValV)), ColMajor(std::move(Vec.ColMajor)){ }
#endif
	TVVec(const TSizeTy& _XDim, const TSizeTy& _YDim, const TBool& _ColMajor = false) :
		XDim(), YDim(), ValV(), ColMajor(_ColMajor){
		Gen(_XDim, _YDim);
	}
	explicit TVVec(const TVec<TVal, TSizeTy>& _ValV, const TSizeTy& _XDim, const TSizeTy& _YDim, const TBool& _ColMajor = false) :
		XDim(_XDim), YDim(_YDim), ValV(_ValV), ColMajor(_ColMajor){
		IAssert(ValV.Len() == XDim*YDim);
	}
	//Andrej beta specialize this only for row major, works only for row major
	void GetExtRows(TVVec& VVec, const TSizeTy& RowStart, const TSizeTy& RowEnd){
		//VVec.ValV.MxVals = -1;//I will not be resonsible for this memory!
		VVec.ValV.GenExt(&ValV[RowStart*YDim], (RowEnd - RowStart + 1)*YDim);
		VVec.XDim = (RowEnd - RowStart + 1);
		VVec.YDim = YDim;
	}
	explicit TVVec(TSIn& SIn) { Load(SIn); }
	void Load(TSIn& SIn){
		SIn.Load(XDim);
		SIn.Load(YDim);
		ValV.Load(SIn);
		if (colmajor){
			ColMajor.Load(SIn);
		}
		else{
			ColMajor = false;
		}
	}
	void Save(TSOut& SOut) const {
		SOut.Save(XDim);
		SOut.Save(YDim);
		ValV.Save(SOut);
		/*Added by Andrej*/
		if (colmajor){
			ColMajor.Save(SOut);
		}
	}
  TVVec<TVal, TSizeTy>& operator=(const TVVec<TVal, TSizeTy>& Vec){
	  if (this != &Vec){ XDim = Vec.XDim; YDim = Vec.YDim;  ValV = Vec.ValV; ColMajor = Vec.ColMajor; } return *this;
  }
  bool operator==(const TVVec& Vec) const {
	  return (XDim == Vec.XDim) && (YDim == Vec.YDim) && (ValV == Vec.ValV) && (ColMajor == Vec.ColMajor);
  }

  bool Empty() const {return ValV.Len()==0;}
  void Clr(){XDim=0; YDim=0; ValV.Clr();}
  void Gen(const TSizeTy& _XDim, const TSizeTy& _YDim){
	  EAssert((_XDim >= 0) && (_YDim >= 0));
	  XDim = _XDim; YDim = _YDim;  ValV.Gen(XDim*YDim); ColMajor = colmajor;
  }
  TSizeTy GetXDim() const {return XDim;}
  TSizeTy GetYDim() const {return YDim;}
  TSizeTy GetRows() const {return XDim;}
  TSizeTy GetCols() const {return YDim;}
  TVec<TVal, TSizeTy>& Get1DVec(){return ValV;}

  const TVal& At(const TSizeTy& X, const TSizeTy& Y) const {
	  Assert((0 <= X) && (X<TSizeTy(XDim)) && (0 <= Y) && (Y<TSizeTy(YDim)));
	  return colmajor ? ValV[Y*XDim + X] : ValV[X*YDim + Y];
  }
  TVal& At(const TSizeTy& X, const TSizeTy& Y){
	  Assert((0 <= X) && (X<TSizeTy(XDim)) && (0 <= Y) && (Y<TSizeTy(YDim)));
	  return colmajor ? ValV[Y*XDim + X] : ValV[X*YDim + Y];
  }
  TVal& operator()(const TSizeTy& X, const TSizeTy& Y){
    return At(X, Y);}
  const TVal& operator()(const TSizeTy& X, const TSizeTy& Y) const {
    return At(X, Y);}

  void PutXY(const TSizeTy& X, const TSizeTy& Y, const TVal& Val){At(X, Y)=Val;}
  void PutAll(const TVal& Val){ValV.PutAll(Val);}
  void PutX(const TSizeTy& X, const TVal& Val){
    for (TSizeTy Y=0; Y<TSizeTy(YDim); Y++){At(X, Y)=Val;}}
  void PutY(const TSizeTy& Y, const TVal& Val){
    for (TSizeTy X=0; X<TSizeTy(XDim); X++){At(X, Y)=Val;}}
  TVal GetXY(const TSizeTy& X, const TSizeTy& Y) const {
	  Assert((0 <= X) && (X<TSizeTy(XDim)) && (0 <= Y) && (Y<TSizeTy(YDim)));
	  return colmajor ? ValV[Y*XDim + X] : ValV[X*YDim + Y];
  }
  void GetRow(const TSizeTy& RowN, TVec<TVal, TSizeTy>& Vec) const;
  // Get a pointer to the row (no copying in case of row major format)
  void GetRowPtr(const TSizeTy& RowN, TVec<TVal, TSizeTy>& Vec);
  void GetRowsPtr(const TSizeTy& RowStart, const TSizeTy& RowEnd, TVVec<TVal, TSizeTy>& Vec);
  void GetRowSIter(const TSizeTy& RowN, TSIter<TVal, TSizeTy>& SIter);
  void GetCol(const TSizeTy& ColN, TVec<TVal, TSizeTy>& Vec) const;
  void GetColPtr(const TSizeTy& ColN, TVec<TVal, TSizeTy>& Vec);
  void GetColSIter(const TSizeTy& ColN, TSIter<TVal, TSizeTy>& Vec);

  void SetRow(const TSizeTy& RowN, const TVec<TVal, TSizeTy>& Vec);
  void SetCol(const TSizeTy& ColN, const TVec<TVal, TSizeTy>& Vec);

  void AddCol(const TVec<TVal, TSizeTy>& Col) { AddYDim();	SetCol(GetCols()-1, Col); }

  void SwapX(const TSizeTy& X1, const TSizeTy& X2);
  void SwapY(const TSizeTy& Y1, const TSizeTy& Y2);
  void Swap(TVVec<TVal, TSizeTy, colmajor>& VVec);

  void ShuffleX(TRnd& Rnd);
  void ShuffleY(TRnd& Rnd);
  void GetMxValXY(TSizeTy& X, TSizeTy& Y) const;

  void CopyFrom(const  TVVec<TVal, TSizeTy, colmajor>& VVec);
  void AddXDim();
  void AddYDim(const TSizeTy& NDims=1);
  void DelX(const TSizeTy& X);
  void DelY(const TSizeTy& Y);
  void DelCols(const TVec<TNum<TSizeTy>, TSizeTy>& ColIdV);
#ifndef INTEL_TRANSPOSE
  void Transpose() {
	  TVec<TVal, TSizeTy> ValV2 = ValV;
	  int Counter = 0;
	  for (int Y = 0; Y < YDim; Y++) {
		  for (int X = 0; X < XDim; X++) {
			  ValV2[Counter] = colmajor ? ValV[Y*XDim + Y] : ValV[X*YDim + Y];
			  Counter++;
		  }
	  }
	  std::swap(ValV, ValV2);
	  TSizeTy temp2 = XDim;
	  XDim = YDim;
	  YDim = temp2;
  }
  void SwitchDim() {
	  TSizeTy temp2 = XDim;
	  XDim = YDim;
	  YDim = temp2;
  }
#else
  //INTEL MKL in place transpose needs to be fixed to support both row and column major format
  void Transpose() {
	  //void mkl_dimatcopy(const char ordering, const char trans, size_t rows, size_t cols, const double alpha, double * AB, size_t lda, size_t ldb);
	  char Format = colmajor ? 'C' : 'T';
	  mkl_dimatcopy(Format, 'T', XDim, YDim, 1.0, &ValV[0].Val, YDim, XDim);
	  TSizeTy temp2 = XDim;
	  XDim = YDim;
	  YDim = temp2;
  }
  void SwitchDim() {
	  TSizeTy temp2 = XDim;
	  XDim = YDim;
	  YDim = temp2;
  }
#endif
};


/////////////////////////////////////////////////
// Common-2D-Vector-Types
typedef TVVec<TBool> TBoolVV;
typedef TVVec<TCh> TChVV;
typedef TVVec<TInt> TIntVV;
typedef TVVec<TSFlt> TSFltVV;
typedef TVVec<TFlt> TFltVV;
typedef TVVec<TStr> TStrVV;
typedef TVVec<TIntPr> TIntPrVV;
typedef TVVec<TUInt64> TUInt64VV;

/////////////////////////////////////////////////
// 3D-Vector
template <class TVal>
class TVVVec{
private:
  TInt XDim, YDim, ZDim;
  TVec<TVal> ValV;
public:
  TVVVec(): XDim(), YDim(), ZDim(), ValV(){}
  TVVVec(const TVVVec& Vec):
    XDim(Vec.XDim), YDim(Vec.YDim), ZDim(Vec.ZDim), ValV(Vec.ValV){}
  TVVVec(const int& _XDim, const int& _YDim, const int& _ZDim):
    XDim(), YDim(), ZDim(), ValV(){Gen(_XDim, _YDim, _ZDim);}
  explicit TVVVec(TSIn& SIn):
    XDim(SIn), YDim(SIn), ZDim(SIn), ValV(SIn){}
  void Save(TSOut& SOut) const {
    XDim.Save(SOut); YDim.Save(SOut); ZDim.Save(SOut); ValV.Save(SOut);}

  TVVVec<TVal>& operator=(const TVVVec<TVal>& Vec){
    XDim=Vec.XDim; YDim=Vec.YDim; ZDim=Vec.ZDim; ValV=Vec.ValV;
    return *this;
  }
  bool operator==(const TVVVec& Vec) const {
    return (XDim==Vec.XDim)&&(YDim==Vec.YDim)&&(ZDim==Vec.ZDim)&&
     (ValV==Vec.ValV);}

  bool Empty() const {return ValV.Len()==0;}
  void Clr(){XDim=0; YDim=0; ZDim=0; ValV.Clr();}
  void Gen(const int& _XDim, const int& _YDim, const int& _ZDim){
    Assert((_XDim>=0)&&(_YDim>=0)&&(_ZDim>=0));
    XDim=_XDim; YDim=_YDim; ZDim=_ZDim; ValV.Gen(XDim*YDim*ZDim);}
  TVal& At(const int& X, const int& Y, const int& Z){
    Assert((0<=X)&&(X<int(XDim))&&(0<=Y)&&(Y<int(YDim))&&(0<=Z)&&(Z<int(ZDim)));
    return ValV[X*YDim*ZDim+Y*ZDim+Z];}
  const TVal& At(const int& X, const int& Y, const int& Z) const {
    Assert((0<=X)&&(X<int(XDim))&&(0<=Y)&&(Y<int(YDim))&&(0<=Z)&&(Z<int(ZDim)));
    return ValV[X*YDim*ZDim+Y*ZDim+Z];}
  TVal& operator()(const int& X, const int& Y, const int& Z){
    return At(X, Y, Z);}
  const TVal& operator()(const int& X, const int& Y, const int& Z) const {
    return At(X, Y, Z);}
  int GetXDim() const {return XDim;}
  int GetYDim() const {return YDim;}
  int GetZDim() const {return ZDim;}
};

/////////////////////////////////////////////////
// Common-3D-Vector-Types
typedef TVVVec<TInt> TIntVVV;
typedef TVVVec<TFlt> TFltVVV;

/////////////////////////////////////////////////
// Tree
template <class TVal>
class TTree{
private:
  TVec<TTriple<TInt, TIntV, TVal> > NodeV; // (ParentNodeId, ChildNodeIdV, NodeVal)
public:
  TTree(): NodeV(){}
  TTree(const TTree& Tree): NodeV(Tree.NodeV){}
  explicit TTree(TSIn& SIn): NodeV(SIn){}
  void Save(TSOut& SOut) const {NodeV.Save(SOut);}
  void Load(TSIn& SIn) {NodeV.Load(SIn);}
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm="");
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TTree& operator=(const TTree& Tree){if (this!=&Tree){NodeV=Tree.NodeV;} return *this;}
  bool operator==(const TTree& Tree) const {return NodeV==Tree.NodeV;}
  bool operator<(const TTree& Tree) const {return false;}

  int GetPrimHashCd() const {return NodeV.GetPrimHashCd();}
  int GetSecHashCd() const {return NodeV.GetSecHashCd();}

  int GetMemUsed() const {return NodeV.GetMemUsed();}

  void Clr(){NodeV.Clr();}

  int AddNode(const int& ParentNodeId, const TVal& NodeVal=TVal()){
    IAssert(((ParentNodeId==-1)&&(NodeV.Len()==0))||(NodeV.Len()>0));
    if (ParentNodeId!=-1){NodeV[ParentNodeId].Val2.Add(NodeV.Len());}
    return NodeV.Add(TTriple<TInt, TIntV, TVal>(ParentNodeId, TIntV(), NodeVal));}
  int AddRoot(const TVal& NodeVal=TVal()){
    return AddNode(-1, NodeVal);}
 
  int GetNodes() const {return NodeV.Len();}
  void GetNodeIdV(TIntV& NodeIdV, const int& NodeId=0);
  int GetParentNodeId(const int& NodeId) const {return NodeV[NodeId].Val1;} 
  int GetChildren(const int& NodeId) const {return NodeV[NodeId].Val2.Len();}
  int GetChildNodeId(const int& NodeId, const int& ChildN) const {return NodeV[NodeId].Val2[ChildN];}
  TVal& GetNodeVal(const int& NodeId){return NodeV[NodeId].Val3;}

  void GenRandomTree(const int& Nodes, TRnd& Rnd);

  void DelNode(const int& NodeId);
  void CopyTree(const int& SrcNodeId, TTree& DstTree, const int& DstParentNodeId=-1);

  void WrTree(const int& NodeId=0, const int& Lev=0);
};

/////////////////////////////////////////////////
// Common-Tree-Types
typedef TTree<TInt> TIntTree;
typedef TTree<TFlt> TFltTree;
typedef TTree<TStr> TStrTree;
typedef TTree<TStrIntPr> TStrIntPrTree;
typedef TTree<TStrIntStrVTr> TStrIntStrVTrTree;

/////////////////////////////////////////////////
// Stack
template <class TVal>
class TSStack{
private:
  TVec<TVal> ValV;
public:
  TSStack(): ValV(){}
  TSStack(const int& MxVals): ValV(MxVals, 0){}
  TSStack(const TSStack& Stack): ValV(Stack.ValV){}
  explicit TSStack(TSIn& SIn): ValV(SIn){}
  void Save(TSOut& SOut) const {ValV.Save(SOut);}

  TSStack& operator=(const TSStack& Stack){
    if (this!=&Stack){ValV=Stack.ValV;} return *this;}
  bool operator==(const TSStack& Stack) const {return this==&Stack;}
  const TVal& operator[](const int& ValN) const {return ValV[ValV.Len()-ValN-1];}
  TVal& operator[](const int& ValN) {return ValV[ValV.Len()-ValN-1];}

  bool Empty(){return ValV.Len()==0;}
  void Clr(const bool& DoDel=false) {ValV.Clr(DoDel);}
  bool IsIn(const TVal& Val) const {return ValV.IsIn(Val);}
  int Len(){return ValV.Len();}
  TVal& Top(){Assert(0<ValV.Len()); return ValV.Last();}
  const TVal& Top() const {Assert(0<ValV.Len()); return ValV.Last();}
  void Push(){ValV.Add();}
  void Push(const TVal& Val){ValV.Add(Val);}
  void Pop(){Assert(0<ValV.Len()); ValV.DelLast();}
};

/////////////////////////////////////////////////
// Common-Stack-Types
typedef TSStack<TInt> TIntS;
typedef TSStack<TBoolChPr> TBoolChS;

/////////////////////////////////////////////////
/// Queue
template <class TVal>
class TQQueue{
  //   Circular buffer implementation.
  //   If Last == Next, then the queue is Empty, which means
  //   that the queue can store ValV.Len() - 1 elements
private:
  TInt MxLast; ///< deprecated, used for binary backward compatibility
  TInt MxLen; ///< the queue will resize if set to -1, otherwise each push will trigger a pop once the queue is full
  TInt Last; ///< Last = 1 + index of the newest element (Back())
  TInt Next; ///< Next = the index of the oldest element (Front())
  TVec<TVal> ValV; ///< Queue values and reserved space
public:
  ///Constructor. _MxLast (ignored), MxLen sets the fixed size or autoresize if set to -1
  TQQueue(const int& _MxLast=64, const int& _MxLen=-1):
    MxLast(_MxLast), MxLen(_MxLen), Last(0), Next(0), ValV(){
    Assert(int(MxLast)>0); Assert((MxLen==-1)||(int(MxLen)>0));
    if (MxLen > 0) {
      ValV.Gen(MxLen);
    }
  }
  /// Copy constructor
  TQQueue(const TQQueue& Queue):
    MxLast(Queue.MxLast), MxLen(Queue.MxLen),
    Last(Queue.Last), Next(Queue.Next), ValV(Queue.ValV){}
  /// Deserialize constructor
  explicit TQQueue(TSIn& SIn):
    MxLast(SIn), MxLen(SIn), Last(SIn), Next(SIn), ValV(SIn){}
  /// Serialize
  void Save(TSOut& SOut) const {
    MxLast.Save(SOut); MxLen.Save(SOut);
    Last.Save(SOut); Next.Save(SOut); ValV.Save(SOut);}
  /// Deserialize
  void Load(TSIn& SIn){
    MxLast.Load(SIn); MxLen.Load(SIn);
    Last.Load(SIn); Next.Load(SIn); ValV.Load(SIn);}

  /// Copy
  TQQueue& operator=(const TQQueue& Queue){
    if (this!=&Queue){MxLast=Queue.MxLast; MxLen=Queue.MxLen;
      Last=Queue.Last; Next=Queue.Next; ValV=Queue.ValV;}
      return *this;}
  
  /// Index operator. Queue[0] corresponds to Back() and Queue[Queue.Len()-1] corresponts to Front()
  const TVal& operator[](const int& ValN) const {
	Assert((0<=ValN)&&(ValN<Len()));
    int Index = (Next + ValN) % ValV.Len();  
    return ValV[Index];}

  /// Deletes the queue
  void Clr(const bool& DoDel=true){ValV.Clr(DoDel); Last=Next=0;}
  /// Initialize the queue. _MxLast is ignored, _MxLen determines the size with autoresize if set to -1.
  void Gen(const int& _MxLast=64, const int& _MxLen=-1){
    MxLast=_MxLast; MxLen=_MxLen; Last=0; Next=0; ValV.Clr();}

  /// Copy a subvector given start and end index
  void GetSubValVec(const int& BValN, const int& EValN, TVec<TVal>& SubValV) const {
    // compensate for incorrect index?
    EAssert((BValN >= 0) && (EValN <= Len() - 1));
    SubValV.Gen(EValN-BValN+1);
    int Vals = ValV.Len();
    for (int ValN=BValN; ValN<=EValN; ValN++){
      SubValV[ValN-BValN]=ValV[(Next+ValN) % Vals];}
  }

  /// Is the queue empty?
  bool Empty() const {return Last==Next;}

  /// Number of elements in the queue
  int Len() const {
    return Last >= Next ? Last - Next : ValV.Len() - (Next - Last);    
  }

  /// Most recently added element, last one to go out
  const TVal& Back() const {
    Assert(!Empty()); return ValV[(Last - 1) % ValV.Len()];
  }

  /// The oldest element, first one to go out
  const TVal& Front() const {
    Assert(!Empty()); return ValV[Next];
  }

  /// The oldest element, first one to go out
  const TVal& Top() const {
    return Front();
  }

  /// Remove the element at the front (dequeue)
  void Pop(){
    Assert(!Empty());
    Next = (Next + 1) % ValV.Len();
  }
  /// Insert the element at the back (enqueue)
  void Push(const TVal& Val){
    int Vals = ValV.Len();
    if ((MxLen != -1) && (MxLen == Len())){ Pop(); }
    // check if we have enough space to add an element
    if (Vals > Len() + 1) {
      ValV[Last] = Val;
      Last++;
      Last = Last % Vals;
    } else {
      int OldLen = Len();
      // resize!
      int NewVals = Vals < 16 ? 16 : 2 * Vals;
      TVec<TVal> Vec; Vec.Gen(NewVals);
      for (int N = Next; N < Next + OldLen; N++) {
        Vec[N - Next] = ValV[N % Vals];
      }
      Vec[OldLen] = Val;
      // copy back
      ValV = Vec;
      Next = 0;
      Last = OldLen + 1;
    }
  }
  /// add all items from the vector to the queue
  void PushV(const TVec<TVal>& ValV) {
    for (int ValN = 0; ValN < ValV.Len(); ValN++) {
      Push(ValV[ValN]); }
  }
  /// Permute the elements of the queue
  void Shuffle(TRnd& Rnd){
    TVec<TVal> ValV(Len(), 0); while (!Empty()){ValV.Add(Top()); Pop();}
    ValV.Shuffle(Rnd); Clr();
    for (int ValN=0; ValN<ValV.Len(); ValN++){Push(ValV[ValN]);}}
};

/////////////////////////////////////////////////
// Common-Queue-Types
typedef TQQueue<TInt> TIntQ;
typedef TQQueue<TFlt> TFltQ;
typedef TQQueue<TStr> TStrQ;
typedef TQQueue<TIntPr> TIntPrQ;
typedef TQQueue<TIntStrPr> TIntStrPrQ;
typedef TQQueue<TFltV> TFltVQ;
typedef TQQueue<TAscFltV> TAscFltVQ;
typedef TVec<TQQueue<TInt> > TIntQV;

/////////////////////////////////////////////////
// List-Node
template <class TVal>
class TLstNd{
public:
  TLstNd* PrevNd;
  TLstNd* NextNd;
  TVal Val;
public:
  TLstNd(): PrevNd(NULL), NextNd(NULL), Val(){}
  TLstNd(const TLstNd&);
  TLstNd(TLstNd* _PrevNd, TLstNd* _NextNd, const TVal& _Val):
    PrevNd(_PrevNd), NextNd(_NextNd), Val(_Val){}

  TLstNd& operator=(const TLstNd&);

  bool IsPrev() const {return (PrevNd != NULL); }
  bool IsNext() const {return (NextNd != NULL); }
  TLstNd* Prev() const {Assert(this!=NULL); return PrevNd;}
  TLstNd* Next() const {Assert(this!=NULL); return NextNd;}
  TVal& GetVal(){Assert(this!=NULL); return Val;}
  const TVal& GetVal() const {Assert(this!=NULL); return Val;}
};

/////////////////////////////////////////////////
// List
template <class TVal>
class TLst{
public:
  typedef TLstNd<TVal>* PLstNd;
private:
  int Nds;
  PLstNd FirstNd;
  PLstNd LastNd;
public:
  TLst(): Nds(0), FirstNd(NULL), LastNd(NULL){}
  TLst(const TLst&);
  TLst<TVal>& operator=(const TLst<TVal>& Val);
  ~TLst(){Clr();}
  explicit TLst(TSIn& SIn);
  void Save(TSOut& SOut) const;

  void Clr(){
    PLstNd Nd=FirstNd;
    while (Nd!=NULL){PLstNd NextNd=Nd->NextNd; delete Nd; Nd=NextNd;}
    Nds=0; FirstNd=NULL; LastNd=NULL;}

  bool Empty() const {return Nds==0;}
  int Len() const {return Nds;}
  PLstNd First() const {return FirstNd;}
  PLstNd Last() const {return LastNd;}
  TVal& FirstVal() const {return FirstNd->GetVal();}
  TVal& LastVal() const {return LastNd->GetVal();}

  PLstNd AddFront(const TVal& Val);
  PLstNd AddBack(const TVal& Val);
  PLstNd AddFrontSorted(const TVal& Val, const bool& Asc=true);
  PLstNd AddBackSorted(const TVal& Val, const bool& Asc=true);
  void PutFront(const PLstNd& Nd);
  void PutBack(const PLstNd& Nd);
  PLstNd Ins(const PLstNd& Nd, const TVal& Val);
  void Del(const TVal& Val);
  void Del(const PLstNd& Nd);
  void DelFirst() { PLstNd DelNd = FirstNd; Del(DelNd); }
  void DelLast() { PLstNd DelNd = LastNd; Del(DelNd); }

  PLstNd SearchForw(const TVal& Val);
  PLstNd SearchBack(const TVal& Val);
  uint64 GetMemUsed() const {
	  return uint64(sizeof(int) + 2 * sizeof(PLstNd) + Nds * sizeof(TLstNd<TVal>));
  }
  friend class TLstNd<TVal>;
};

/////////////////////////////////////////////////
// Common-List-Types
typedef TLst<TInt> TIntL;
typedef TLstNd<TInt>* PIntLN;
typedef TLst<TIntKd> TIntKdL;
typedef TLstNd<TIntKd>* PIntKdLN;
typedef TLst<TFlt> TFltL;
typedef TLstNd<TFlt>* PFltLN;
typedef TLst<TFltIntKd> TFltIntKdL;
typedef TLstNd<TFltIntKd>* PFltIntKdLN;
typedef TLst<TAscFltIntKd> TAscFltIntKdL;
typedef TLstNd<TAscFltIntKd>* PAscFltIntKdLN;
typedef TLst<TStr> TStrL;
typedef TLstNd<TStr>* PStrLN;

/////////////////////////////////////////////////
// Record-File
template <class THd, class TRec>
class TFRec{
private:
  PFRnd FRnd;
public:
  TFRec(const TStr& FNm, const TFAccess& FAccess, const bool& CreateIfNo):
    FRnd(PFRnd(new TFRnd(FNm, FAccess, CreateIfNo, sizeof(THd), sizeof(TRec)))){}
  TFRec(const TFRec&);

  TFRec& operator=(const TFRec&);

  void SetRecN(const int& RecN){FRnd->SetRecN(RecN);}
  int GetRecN(){return FRnd->GetRecN();}
  int GetRecs(){return FRnd->GetRecs();}

  void GetHd(THd& Hd){FRnd->GetHd(&Hd);}
  void PutHd(const THd& Hd){FRnd->PutHd(&Hd);}
  void GetRec(TRec& Rec, const int& RecN=-1){FRnd->GetRec(&Rec, RecN);}
  void PutRec(const TRec& Rec, const int& RecN=-1){FRnd->PutRec(&Rec, RecN);}
};

/////////////////////////////////////////////////
// Function
template <class TFuncPt>
class TFunc{
private:
  TFuncPt FuncPt;
public:
  TFunc(): FuncPt(NULL){}
  TFunc(const TFunc& Func): FuncPt(Func.FuncPt){}
  TFunc(const TFuncPt& _FuncPt): FuncPt(_FuncPt){}
  TFunc(TSIn&){Fail;}
  void Save(TSOut&) const {Fail;}

  TFunc& operator=(const TFunc& Func){
    if (this!=&Func){FuncPt=Func.FuncPt;} return *this;}
  bool operator==(const TFunc& Func) const {
    return FuncPt==Func.FuncPt;}
  bool operator<(const TFunc&) const {
    Fail; return false;}
  TFuncPt operator()() const {return FuncPt;}
};

//#//////////////////////////////////////////////
/// Simple heap data structure. ##THeap
template <class TVal, class TCmp = TLss<TVal> >
class THeap {
private:
  TCmp Cmp;
  TVec<TVal> HeapV;
private:
  void PushHeap(const int& First, int HoleIdx, const int& Top, TVal Val);
  void AdjustHeap(const int& First, int HoleIdx, const int& Len, TVal Val);
  void MakeHeap(const int& First, const int& Len);
public:
  THeap() : HeapV() { }
  THeap(const int& MxVals) : Cmp(), HeapV(MxVals, 0) { }
  THeap(const TCmp& _Cmp) : Cmp(_Cmp), HeapV() { }
  THeap(const TVec<TVal>& Vec) : Cmp(), HeapV(Vec) { MakeHeap(); }
  THeap(const TVec<TVal>& Vec, const TCmp& _Cmp) : Cmp(_Cmp), HeapV(Vec) { MakeHeap(); }
  THeap(const THeap& Heap) : Cmp(Heap.Cmp), HeapV(Heap.HeapV) { }
  THeap& operator = (const THeap& H) { Cmp=H.Cmp; HeapV=H.HeapV; return *this; }

  /// Returns a reference to the element at the top of the heap (the largest element of the heap).
  const TVal& TopHeap() const { return HeapV[0]; }
  /// Pushes an element \c Val to the heap.
  void PushHeap(const TVal& Val);
  /// Removes the top element from the heap.
  TVal PopHeap();
  /// Returns the number of elements in the heap.
  int Len() const { return HeapV.Len(); }
  /// Tests whether the heap is empty.
  bool Empty() const { return HeapV.Empty(); }
  /// Returns a reference to the vector containing the elements of the heap.
  const TVec<TVal>& operator()() const { return HeapV; }
  /// Returns a reference to the vector containing the elements of the heap.
  TVec<TVal>& operator()() { return HeapV; }
  /// Adds an element to the data structure. Heap property is not maintained by \c Add() and thus after all the elements are added \c MakeHeap() needs to be called.
  void Add(const TVal& Val) { HeapV.Add(Val); }
  /// Builds a heap from a set of elements.
  void MakeHeap() { MakeHeap(0, Len()); }
};


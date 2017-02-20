/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */


#ifndef SRC_GLIB_BASE_DS_HPP_
#define SRC_GLIB_BASE_DS_HPP_

/////////////////////////////////////////////////
// Tuple
template<class TVal, int NVals>
void TTuple<TVal, NVals>::Sort(const bool& Asc) {
  TVec<TVal, int> V(NVals);
  for (int i=0; i<NVals; i++) { V.Add(ValV[i]); }
  V.Sort(Asc);
  for (int i=0; i<NVals; i++) { ValV[i] = V[i]; }
}

template<class TVal, int NVals>
int TTuple<TVal, NVals>::FindMx() const {
  TVal MxVal = ValV[0];
  int ValN = 0;
  for (int i = 1; i < NVals; i++) {
    if (MxVal<ValV[i]) {
      MxVal=ValV[i]; ValN=i;
    }
  }
  return ValN;
}

template<class TVal, int NVals>
int TTuple<TVal, NVals>::FindMn() const {
  TVal MnVal = ValV[0];
  int ValN = 0;
  for (int i = 1; i < NVals; i++) {
    if (MnVal>ValV[i]) {
      MnVal=ValV[i]; ValN=i;
    }
  }
  return ValN;
}

//#//////////////////////////////////////////////
/// Vector is a sequence \c TVal objects representing an array that can change in size. ##TVec

// add new vector of data to current vector, use memcpy for performance
template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddVMemCpy(const TVec<TVal, TSizeTy>& ValV) {
	if (ValV.Len() == 0)
		return 0;
	Resize(Vals + ValV.Len());
	memcpy(ValT + Vals, ValV.ValT, ValV.Len() * sizeof(TVal));
	Vals += ValV.Len();
	return ValV.Len();
}

// optimized deserialization from stream, uses memcpy
template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::LoadMemCpy(TMIn& SIn) {
	if ((ValT != NULL) && (MxVals != -1)) { delete[] ValT; }
	SIn.Load(MxVals);
	SIn.Load(Vals);
	MxVals = Vals;
	if (MxVals == 0) {
		ValT = NULL;
	} else {
		ValT = new TVal[MxVals];
		SIn.GetBfMemCpy(ValT, Vals*sizeof(TVal));
	}
}
// optimized serialization from stream, uses memcpy
template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::SaveMemCpy(TMOut& SOut) const {
	SOut.Save(MxVals);
	SOut.Save(Vals);
	if (MxVals > 0)
		SOut.AppendBf(ValT, Vals*sizeof(TVal));
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Resize(const TSizeTy& _MxVals){
  IAssertR(MxVals!=-1, TStr::Fmt("Can not increase the capacity of the vector. %s. [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]", GetTypeNm(*this).CStr()).CStr());
  IAssertR(MxVals!=(TInt::Mx-1024), TStr::Fmt("Buffer size at maximum. %s. [Program refuses to allocate more memory. Solution-1: Send your test case to developers.]", GetTypeNm(*this).CStr()).CStr());
  if (_MxVals==-1){
    if (Vals==0){MxVals=16;} else {MxVals*=2;}
  } else {
    if (_MxVals<=MxVals){return;} else {MxVals=_MxVals;}
  }
  if (MxVals < 0) {
    MxVals = TInt::Mx-1024;
  }
  if (ValT==NULL){
    try {ValT=new TVal[MxVals];}
    catch (std::exception& Ex){
      FailR(TStr::Fmt("TVec::Resize: %s, Length:%s, Capacity:%s, New capacity:%s, Type:%s [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]",
        Ex.what(), TInt::GetStr(Vals).CStr(), TInt::GetStr(MxVals).CStr(), TInt::GetStr(_MxVals).CStr(), GetTypeNm(*this).CStr()).CStr());}
  } else {
    TVal* NewValT = NULL;
    try {
      NewValT=new TVal[MxVals];}
    catch (std::exception& Ex){
      FailR(TStr::Fmt("TVec::Resize: %s, Length:%s, Capacity:%s, New capacity:%s, Type:%s [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]",
        Ex.what(), TInt::GetStr(Vals).CStr(), TInt::GetStr(MxVals).CStr(), TInt::GetStr(_MxVals).CStr(), GetTypeNm(*this).CStr()).CStr());}
    IAssert(NewValT!=NULL);
    for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=std::move(ValT[ValN]);} //C++11
    //for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=ValT[ValN];} // C++98
    delete[] ValT; ValT=NewValT;
  }
}

template <class TVal, class TSizeTy>
TStr TVec<TVal, TSizeTy>::GetXOutOfBoundsErrMsg(const TSizeTy& ValN) const {
  return TStr()+
  "Index:"+TInt::GetStr(ValN)+
  " Vals:"+TInt::GetStr(Vals)+
  " MxVals:"+TInt::GetStr(MxVals)+
  " Type:"+GetTypeNm(*this);
}

template <class TVal, class TSizeTy>
TVec<TVal, TSizeTy>::TVec(const TVec<TVal, TSizeTy>& Vec){
  MxVals=Vec.MxVals; Vals=Vec.Vals;
  if (MxVals==0){ValT=NULL;} else {ValT=new TVal[MxVals];}
  for (TSizeTy ValN=0; ValN<Vec.Vals; ValN++){ValT[ValN]=Vec.ValT[ValN];}
}

#ifdef GLib_CPP11
template <class TVal, class TSizeTy>
TVec<TVal, TSizeTy>::TVec(TVec<TVal, TSizeTy>&& Vec) : MxVals(0), Vals(0), ValT() {
	//TSizeTy MxVals; //!< Vector capacity. Capacity is the size of allocated storage. If <tt>MxVals==-1</tt>, then \c ValT is not owned by the vector, and it won't free it at destruction.
	//TSizeTy Vals;   //!< Vector length. Length is the number of elements stored in the vector.
    //TVal* ValT;     //!< Pointer to the memory where the elements of the vector are stored.
	MxVals = std::move(Vec.MxVals);
	Vals = std::move(Vec.Vals);
	ValT = Vec.ValT;
	Vec.MxVals = 0;
	Vec.Vals = 0;
	Vec.ValT = NULL;
}
#endif

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Load(TSIn& SIn){
  if ((ValT!=NULL)&&(MxVals!=-1)){delete[] ValT;}
  SIn.Load(MxVals); SIn.Load(Vals); MxVals=Vals;
  if (MxVals==0){ValT=NULL;} else {ValT=new TVal[MxVals];}
  for (TSizeTy ValN=0; ValN<Vals; ValN++){ValT[ValN]=TVal(SIn);}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Save(TSOut& SOut) const {
  if (MxVals!=-1){SOut.Save(MxVals);} else {SOut.Save(Vals);}
  SOut.Save(Vals);
  for (TSizeTy ValN=0; ValN<Vals; ValN++){ValT[ValN].Save(SOut);}
}

template <class TVal, class TSizeTy>
TVec<TVal, TSizeTy>& TVec<TVal, TSizeTy>::operator=(const TVec<TVal, TSizeTy>& Vec){
	if (this != &Vec) {
		// check if we have enough space and we are not in a vector pool
		if ((MxVals > 0) && (Vec.Vals <= MxVals)) {
			// only need to copy the new number of elements
			Vals = Vec.Vals;
		} else {
			// delete old buffer if we have it and own it
			if ((ValT != NULL) && (MxVals != -1)) { delete[] ValT; }
			MxVals = Vals = Vec.Vals;
			// create the buffer if we have any values
			if (MxVals == 0) { ValT = NULL; } else { ValT = new TVal[MxVals]; }
		}
		// copy values
		for (TSizeTy ValN = 0; ValN < Vec.Vals; ValN++) { ValT[ValN] = Vec.ValT[ValN]; }
	}
	return *this;
}

template <class TVal, class TSizeTy>
TVec<TVal, TSizeTy>& TVec<TVal, TSizeTy>::operator=(TVec<TVal, TSizeTy>&& Vec) {
	if (this != &Vec) {
		if(MxVals != -1) delete[] ValT;
		MxVals = std::move(Vec.MxVals);
		Vals = std::move(Vec.Vals);
		ValT = Vec.ValT;
		Vec.MxVals = 0;
		Vec.Vals = 0;
		Vec.ValT = NULL;
	}
	return *this;
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::operator==(const TVec<TVal, TSizeTy>& Vec) const {
  if (this==&Vec){return true;}
  if (Len()!=Vec.Len()){return false;}
  for (TSizeTy ValN=0; ValN<Vals; ValN++){
    if (ValT[ValN]!=Vec.ValT[ValN]){return false;}}
  return true;
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::operator<(const TVec<TVal, TSizeTy>& Vec) const {
  if (this==&Vec){return false;}
  if (Len()==Vec.Len()){
    for (TSizeTy ValN=0; ValN<Vals; ValN++){
      if (ValT[ValN]<Vec.ValT[ValN]){return true;}
      else if (ValT[ValN]>Vec.ValT[ValN]){return false;}
      else {}
    }
    return false;
  } else {
    return Len()<Vec.Len();
  }
}

// Improved hashing of vectors (Jure Apr 20 2013)
// This change makes binary representation of vectors incompatible with previous code.
// Previous hash functions are available for compatibility in class TVecHashF_OldGLib
template <class TVal, class TSizeTy>
int TVec<TVal, TSizeTy>::GetPrimHashCd() const {
  int hc = 0;
  for (TSizeTy i=0; i<Vals; i++){
    hc = TPairHashImpl::GetHashCd(hc, ValT[i].GetPrimHashCd());
  }
  return hc;
}

// Improved hashing of vectors (Jure Apr 20 2013)
// This change makes binary representation of vectors incompatible with previous code.
// Previous hash functions are available for compatibility in class TVecHashF_OldGLib
template <class TVal, class TSizeTy>
int TVec<TVal, TSizeTy>::GetSecHashCd() const {
  int hc = 0;
  for (TSizeTy i=0; i<Vals; i++){
    hc = TPairHashImpl::GetHashCd(hc, ValT[i].GetSecHashCd());
  }
  if (Vals > 0) {
    hc = TPairHashImpl::GetHashCd(hc, ValT[0].GetSecHashCd()); }
  return hc;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Clr(const bool& DoDel, const TSizeTy& NoDelLim){
  if ((DoDel)||((!DoDel)&&(NoDelLim!=-1)&&(MxVals>NoDelLim))){
    if ((ValT!=NULL)&&(MxVals!=-1)){delete[] ValT;}
    MxVals=Vals=0; ValT=NULL;
  } else {
    IAssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
    Vals=0;
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Trunc(const TSizeTy& _Vals){
  IAssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  IAssert((_Vals==-1)||(_Vals>=0));
  if ((_Vals!=-1)&&(_Vals>=Vals)){
    return;
  } else
    if (((_Vals==-1)&&(Vals==0))||(_Vals==0)){
      if (ValT!=NULL){delete[] ValT;}
      MxVals=Vals=0; ValT=NULL;
    } else {
      if (_Vals==-1){
        if (MxVals==Vals){return;} else {MxVals=Vals;}
      } else {
        MxVals=Vals=_Vals;
      }
      TVal* NewValT=new TVal[MxVals];
      IAssert(NewValT!=NULL);
      for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=std::move(ValT[ValN]);} //C++11
      // for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=ValT[ValN];} // C++98
      delete[] ValT; ValT=NewValT;
    }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Pack(){
  IAssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  if (Vals==0){
    if (ValT!=NULL){delete[] ValT;} ValT=NULL;
  } else
    if (Vals<MxVals){
      MxVals=Vals;
      TVal* NewValT=new TVal[MxVals];
      IAssert(NewValT!=NULL);
      for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=std::move(ValT[ValN]);} //C++11
      // for (TSizeTy ValN=0; ValN<Vals; ValN++){NewValT[ValN]=ValT[ValN];} // C++98
      delete[] ValT; ValT=NewValT;
    }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::MoveFrom(TVec<TVal, TSizeTy>& Vec){
  if (this!=&Vec){
    if (ValT!=NULL && MxVals!=-1){delete[] ValT;}
    MxVals=Vec.MxVals; Vals=Vec.Vals; ValT=Vec.ValT;
    Vec.MxVals=0; Vec.Vals=0; Vec.ValT=NULL;
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Swap(TVec<TVal, TSizeTy>& Vec){
  if (this!=&Vec){
    ::Swap(MxVals, Vec.MxVals);
    ::Swap(Vals, Vec.Vals);
    ::Swap(ValT, Vec.ValT);
  }
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddV(const TVec<TVal, TSizeTy>& ValV){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  for (TSizeTy ValN=0; ValN<ValV.Vals; ValN++){Add(ValV[ValN]);}
  return Len();
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddSorted(const TVal& Val, const bool& Asc, const TSizeTy& _MxVals){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  // if we have the max items to store and the new val is not relevant (the current worse is still better
  // then the new val) then we can return without adding and deleting the item
  if (_MxVals != -1 && Len() == _MxVals) {
    if (Asc && ValT[Len()-1] < Val) return Len()-1;
    else if (!Asc && ValT[Len()-1] > Val) return Len()-1;
  }
  TSizeTy ValN=Add(Val);
  if (Asc){
    while ((ValN>0)&&(ValT[ValN]<ValT[ValN-1])){
      Swap(ValN, ValN-1); ValN--;}
  } else {
    while ((ValN>0)&&(ValT[ValN]>ValT[ValN-1])){
      Swap(ValN, ValN-1); ValN--;}
  }
  if ((_MxVals!=-1)&&(Len()>_MxVals)){Del(_MxVals, Len()-1);}
  return ValN;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddBackSorted(const TVal& Val, const bool& Asc){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  Add();
  TSizeTy ValN=Vals-2;
  while ((ValN>=0)&&((Asc&&(Val<ValT[ValN]))||(!Asc&&(Val>ValT[ValN])))){
    Move(ValN, ValN+1); }
    //ValT[ValN+1]=ValT[ValN]; ValN--;}
  ValT[ValN+1]=Val;
  return ValN+1;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddMerged(const TVal& Val){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  TSizeTy ValN=SearchBin(Val);
  if (ValN==-1){return AddSorted(Val);}
  else {GetVal(ValN)=Val; return -1;}
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddVMerged(const TVec<TVal, TSizeTy>& ValV){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  for (TSizeTy ValN=0; ValN<ValV.Vals; ValN++){AddMerged(ValV[ValN]);}
  return Len();
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::AddUnique(const TVal& Val){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  TSizeTy ValN=SearchForw(Val);
  if (ValN==-1){return Add(Val);}
  else {GetVal(ValN)=Val; return -1;}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::GetSubValV(const TSizeTy& _BValN, const TSizeTy& _EValN, TVec<TVal, TSizeTy>& SubValV) const {
  SubValV.Clr();
  if (Len() == 0 || _BValN >= Len() || _BValN > _EValN) { return; }
  const TSizeTy BValN=TInt::GetInRng(_BValN, 0, Len()-1);
  const TSizeTy EValN=TInt::GetInRng(_EValN, 0, Len()-1);
  const TSizeTy SubVals=TInt::GetMx(0, EValN-BValN+1);
  SubValV.Gen(SubVals, 0);
  for (TSizeTy ValN = BValN; ValN <= EValN; ValN++) {
    SubValV.Add(GetVal(ValN));}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::GetSubValVMemCpy(const TSizeTy& _BValN, const TSizeTy& _EValN, TVec<TVal, TSizeTy>& SubValV) const {
    SubValV.Clr();
	if (Len() == 0 || _BValN >= Len() || _BValN > _EValN) { return; }
	const TSizeTy BValN = TInt::GetInRng(_BValN, 0, Len() - 1);
	const TSizeTy EValN = TInt::GetInRng(_EValN, 0, Len() - 1);
	const TSizeTy SubVals = TInt::GetMx(0, EValN - BValN + 1);
	SubValV.Gen(SubVals, 0);
	memcpy(SubValV.ValT, ValT + BValN, SubVals * sizeof(TVal));
	SubValV.Vals += SubVals;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Ins(const TSizeTy& ValN, const TVal& Val){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  Add();  Assert((0<=ValN)&&(ValN<Vals));
  for (TSizeTy MValN=Vals-2; MValN>=ValN; MValN--){Move(MValN, MValN+1);}
  //for (TSizeTy MValN=Vals-2; MValN>=ValN; MValN--){ValT[MValN+1]=ValT[MValN];}
  ValT[ValN]=Val;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Del(const TSizeTy& ValN){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  Assert((0<=ValN)&&(ValN<Vals));
  for (TSizeTy MValN=ValN+1; MValN<Vals; MValN++){Move(MValN, MValN-1);}
  //for (TSizeTy MValN=ValN+1; MValN<Vals; MValN++){ValT[MValN-1]=ValT[MValN];}
  ValT[--Vals]=TVal();
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::DelMemCpy(const TSizeTy& ValN) {
	AssertR(MxVals != -1, "This vector was obtained from TVecPool. Such vectors cannot change its size!");
	Assert((0 <= ValN) && (ValN<Vals));
	if (ValN < Vals - 1) {
		memmove(ValT + ValN, ValT + ValN + 1, sizeof(TVal) * (Vals - ValN - 1)); // overlapping buffers, use memmove instead of memcpy
	}
	ValT[--Vals] = TVal();
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Del(const TSizeTy& MnValN, const TSizeTy& MxValN){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  Assert((0<=MnValN)&&(MnValN<Vals)&&(0<=MxValN)&&(MxValN<Vals));
  Assert(MnValN<=MxValN);
  for (TSizeTy ValN=MxValN+1; ValN<Vals; ValN++){
    Move(ValN, MnValN+ValN-MxValN-1);}
    //ValT[MnValN+ValN-MxValN-1]=ValT[ValN];}
  for (TSizeTy ValN=Vals-MxValN+MnValN-1; ValN<Vals; ValN++){
    ValT[ValN]=TVal();}
  Vals-=MxValN-MnValN+1;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::DelMemCpy(const TSizeTy& MnValN, const TSizeTy& MxValN) {
	AssertR(MxVals != -1, "This vector was obtained from TVecPool. Such vectors cannot change its size!");
	Assert((0 <= MnValN) && (MnValN<Vals) && (0 <= MxValN) && (MxValN<Vals));
	Assert(MnValN <= MxValN);
	if (MxValN < Vals - 1) {
		memmove(ValT + MnValN, ValT + MxValN + 1, sizeof(TVal) * (Vals - MxValN - 1)); // overlapping buffers, use memmove instead of memcpy
	}
	Vals -= MxValN - MnValN + 1;
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::DelIfIn(const TVal& Val){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  TSizeTy ValN=SearchForw(Val);
  if (ValN!=-1){Del(ValN); return true;}
  else {return false;}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::DelAll(const TVal& Val){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  TSizeTy ValN;
  while ((ValN=SearchForw(Val))!=-1){Del(ValN);}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::PutAll(const TVal& Val){
  for (TSizeTy ValN=0; ValN<Vals; ValN++){ValT[ValN]=Val;}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Del(const TVec<TNum<TSizeTy>, TSizeTy>& ValNV) {
	THashSet<TNum<TSizeTy>> ColIdSet;
	for (int ColIdN = 0; ColIdN < ValNV.Len(); ColIdN++) {
		ColIdSet.AddKey(ValNV[ColIdN]);
	}

	TVec<TVal,TSizeTy> NewVec(Len() - ColIdSet.Len(), Len() - ColIdSet.Len());

	int NewValN = 0;
	TVec<TVal, TSizeTy> Col;
	for (int ValN = 0; ValN < Len(); ValN++) {
		if (!ColIdSet.IsKey(ValN)) {
			#ifdef GLib_CPP11
			NewVec[NewValN++] = std::move(this->operator [](ValN));
			#else
			NewVec[NewValN++] = this->operator [](ValN);
			#endif
		}
	}

#ifdef GLib_CPP11
	*this = std::move(NewVec);
#else
	*this = NewVVec;
#endif
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::BSort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc){
  for (TSizeTy ValN1=MnLValN; ValN1<=MxRValN; ValN1++){
    for (TSizeTy ValN2=MxRValN; ValN2>ValN1; ValN2--){
      if (Asc){
        if (ValT[ValN2]<ValT[ValN2-1]){Swap(ValN2, ValN2-1);}
      } else {
        if (ValT[ValN2]>ValT[ValN2-1]){Swap(ValN2, ValN2-1);}
      }
    }
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::ISort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc){
  if (MnLValN<MxRValN){
    for (TSizeTy ValN1=MnLValN+1; ValN1<=MxRValN; ValN1++){
      TVal Val=ValT[ValN1]; TSizeTy ValN2=ValN1;
      if (Asc){
        while ((ValN2>MnLValN)&&(ValT[ValN2-1]>Val)){
          Move(ValN2-1, ValN2); ValN2--;}
          //ValT[ValN2]=ValT[ValN2-1]; ValN2--;}
      } else {
        while ((ValN2>MnLValN)&&(ValT[ValN2-1]<Val)){
          Move(ValN2-1, ValN2); ValN2--;}
          //ValT[ValN2]=ValT[ValN2-1]; ValN2--;}
      }
      ValT[ValN2]=Val;
    }
  }
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::GetPivotValN(const TSizeTy& LValN, const TSizeTy& RValN, TRnd& Rnd) const {
  TSizeTy SubVals=RValN-LValN+1;
  if (SubVals > TInt::Mx-1) { SubVals = TInt::Mx-1; }
  const TSizeTy ValN1=LValN+Rnd.GetUniDevInt(int(SubVals));
  const TSizeTy ValN2=LValN+Rnd.GetUniDevInt(int(SubVals));
  const TSizeTy ValN3=LValN+Rnd.GetUniDevInt(int(SubVals));
  const TVal& Val1=ValT[ValN1];
  const TVal& Val2=ValT[ValN2];
  const TVal& Val3=ValT[ValN3];
  if (Val1<Val2){
    if (Val2<Val3){return ValN2;}
    else if (Val3<Val1){return ValN1;}
    else {return ValN3;}
  } else {
    if (Val1<Val3){return ValN1;}
    else if (Val3<Val2){return ValN2;}
    else {return ValN3;}
  }
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::Partition(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc, TRnd& Rnd){
  TSizeTy PivotValN=GetPivotValN(MnLValN, MxRValN, Rnd);
  Assert(MnLValN <= PivotValN && PivotValN <= MxRValN);
  Swap(PivotValN, MnLValN);
  TVal PivotVal=ValT[MnLValN];
  TSizeTy LValN=MnLValN-1;  TSizeTy RValN=MxRValN+1;
  forever {
    if (Asc){
      do {RValN--;} while (ValT[RValN]>PivotVal);
      do {LValN++;} while (ValT[LValN]<PivotVal);
    } else {
      do {RValN--;} while (ValT[RValN]<PivotVal);
      do {LValN++;} while (ValT[LValN]>PivotVal);
    }
    if (LValN<RValN){Swap(LValN, RValN);}
    else {return RValN;}
  };
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::QSort(const TSizeTy& MnLValN, const TSizeTy& MxRValN, const bool& Asc, TRnd& Rnd){
  if (MnLValN<MxRValN){
    if (MxRValN-MnLValN<20){
      ISort(MnLValN, MxRValN, Asc);
    } else {
      TSizeTy SplitValN=Partition(MnLValN, MxRValN, Asc, Rnd);
      QSort(MnLValN, SplitValN, Asc, Rnd);
      QSort(SplitValN+1, MxRValN, Asc, Rnd);
    }
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Sort(TRnd& Rnd, const bool& Asc){
  QSort(0, Len()-1, Asc, Rnd);
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::SortGetPerm(const TVec<TVal, TSizeTy>& Vec, TVec<TVal, TSizeTy>& SortedVec, TVec<TInt, TSizeTy>& PermV, bool Asc) {
	TSizeTy Len = Vec.Len();
	TVec<TPair<TVal, TInt> > PairV; PairV.Gen(Len, 0);
	for (TSizeTy ElN = 0; ElN < Len; ElN++) {
		PairV.Add(TPair<TVal, TInt>(Vec[ElN], ElN));
	}
	PairV.Sort(Asc);
	SortedVec.Gen(Len, 0);
	PermV.Gen(Len, 0);
	for (TSizeTy ElN = 0; ElN < Len; ElN++) {
		SortedVec.Add(PairV[ElN].Val1);
		PermV.Add(PairV[ElN].Val2);
	}
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::IsSorted(const bool& Asc) const {
  if (Asc){
    for (TSizeTy ValN=0; ValN<Vals-1; ValN++){
      if (ValT[ValN]>ValT[ValN+1]){return false;}}
  } else {
    for (TSizeTy ValN=0; ValN<Vals-1; ValN++){
      if (ValT[ValN]<ValT[ValN+1]){return false;}}
  }
  return true;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Shuffle(TRnd& Rnd){
  if (Len() < TInt::Mx) {
    for (TSizeTy ValN=0; ValN<Vals-1; ValN++){
      const int Range = int(Vals-ValN);
      Swap(ValN, ValN+Rnd.GetUniDevInt(Range));
    }
  } else {
    for (TSizeTy ValN=0; ValN<Vals-1; ValN++){
      const TSizeTy Range = Vals-ValN;
      Swap(ValN, TSizeTy(ValN+Rnd.GetUniDevInt64(Range)));
    }
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Reverse(){
  for (TSizeTy ValN=0; ValN<Vals/2; ValN++){
    Swap(ValN, Vals-ValN-1);}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Merge(){
  AssertR(MxVals!=-1, "This vector was obtained from TVecPool. Such a vector cannot change its size!");
  TVec<TVal, TSizeTy> SortedVec(*this); SortedVec.Sort();
  Clr();
  for (TSizeTy ValN=0; ValN<SortedVec.Len(); ValN++){
    if ((ValN==0)||(SortedVec[ValN-1]!=SortedVec[ValN])){
      Add(SortedVec[ValN]);}
  }
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::NextPerm() {
  // start with a sorted sequence to obtain all permutations
  TSizeTy First = 0, Last = Len(), Next = Len()-1;
  if (Last < 2) return false;
  for(; ; ) {
    // find rightmost element smaller than successor
    TSizeTy Next1 = Next;
    if (GetVal(--Next) < GetVal(Next1)) { // swap with rightmost element that's smaller, flip suffix
      TSizeTy Mid = Last;
      for (; GetVal(Next) >= GetVal(--Mid); ) { }
      Swap(Next, Mid);
      Reverse(Next1, Last-1);
      return true;
    }
    if (Next == First) { // pure descending, flip all
      Reverse();
      return false;
    }
  }
}

template <class TVal, class TSizeTy>
bool TVec<TVal, TSizeTy>::PrevPerm() {
  TSizeTy First = 0, Last = Len(), Next = Len()-1;
  if (Last < 2) return false;
  for(; ; ) {
    // find rightmost element not smaller than successor
    TSizeTy Next1 = Next;
    if (GetVal(--Next) >= GetVal(Next1)) { // swap with rightmost element that's not smaller, flip suffix
      TSizeTy Mid = Last;
      for (; GetVal(Next) < GetVal(--Mid); ) { }
      Swap(Next, Mid);
      Reverse(Next1, Last);
      return true;
    }
    if (Next == First) { // pure descending, flip all
      Reverse();
      return false;
    }
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Intrs(const TVec<TVal, TSizeTy>& ValV){
  TVec<TVal, TSizeTy> IntrsVec;
  Intrs(ValV, IntrsVec);
  MoveFrom(IntrsVec);
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Union(const TVec<TVal, TSizeTy>& ValV){
  TVec<TVal, TSizeTy> UnionVec;
  Union(ValV, UnionVec);
  MoveFrom(UnionVec);
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Diff(const TVec<TVal, TSizeTy>& ValV){
  TVec<TVal, TSizeTy> DiffVec;
  Diff(ValV, DiffVec);
  MoveFrom(DiffVec);
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Intrs(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const {
  DstValV.Clr();
  TSizeTy ValN1=0, ValN2=0;
  while ((ValN1<Len())&&(ValN2<ValV.Len())){
    const TVal& Val1=GetVal(ValN1);
    while ((ValN2<ValV.Len())&&(Val1>ValV.GetVal(ValN2))){
      ValN2++;}
    if ((ValN2<ValV.Len())&&(Val1==ValV.GetVal(ValN2))){
      DstValV.Add(Val1); ValN2++;}
    ValN1++;
  }
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Union(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const {
  DstValV.Gen(TInt::GetMx(Len(), ValV.Len()), 0);
  TSizeTy ValN1=0, ValN2=0;
  while ((ValN1<Len())&&(ValN2<ValV.Len())){
    const TVal& Val1=GetVal(ValN1);
    const TVal& Val2=ValV.GetVal(ValN2);
    if (Val1<Val2){DstValV.Add(Val1); ValN1++;}
    else if (Val1>Val2){DstValV.Add(Val2); ValN2++;}
    else {DstValV.Add(Val1); ValN1++; ValN2++;}
  }
  for (TSizeTy RestValN1=ValN1; RestValN1<Len(); RestValN1++){
    DstValV.Add(GetVal(RestValN1));}
  for (TSizeTy RestValN2=ValN2; RestValN2<ValV.Len(); RestValN2++){
    DstValV.Add(ValV.GetVal(RestValN2));}
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::Diff(const TVec<TVal, TSizeTy>& ValV, TVec<TVal, TSizeTy>& DstValV) const {
  DstValV.Clr();
  TSizeTy ValN1=0, ValN2=0;
  while (ValN1<Len() && ValN2<ValV.Len()) {
    const TVal& Val1 = GetVal(ValN1);
    while (ValN2<ValV.Len() && Val1>ValV.GetVal(ValN2)) ValN2++;
    if (ValN2<ValV.Len()) {
      if (Val1!=ValV.GetVal(ValN2)) { DstValV.Add(Val1); }
      ValN1++;
    }
  }
  for (TSizeTy RestValN1=ValN1; RestValN1<Len(); RestValN1++){
    DstValV.Add(GetVal(RestValN1));}
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::IntrsLen(const TVec<TVal, TSizeTy>& ValV) const {
  TSizeTy Cnt=0, ValN1=0, ValN2=0;
  while ((ValN1<Len())&&(ValN2<ValV.Len())){
    const TVal& Val1=GetVal(ValN1);
    while ((ValN2<ValV.Len())&&(Val1>ValV.GetVal(ValN2))){
      ValN2++;}
    if ((ValN2<ValV.Len())&&(Val1==ValV.GetVal(ValN2))){
      ValN2++; Cnt++;}
    ValN1++;
  }
  return Cnt;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::UnionLen(const TVec<TVal, TSizeTy>& ValV) const {
  TSizeTy Cnt = 0, ValN1 = 0, ValN2 = 0;
  while ((ValN1 < Len()) && (ValN2 < ValV.Len())) {
    const TVal& Val1 = GetVal(ValN1);
    const TVal& Val2 = ValV.GetVal(ValN2);
    if (Val1 < Val2) {
      Cnt++; ValN1++;
    } else if (Val1 > Val2) {
      Cnt++; ValN2++;
    } else {
      Cnt++; ValN1++; ValN2++;
    }
  }
  Cnt += (Len() - ValN1) + (ValV.Len() - ValN2);
  return Cnt;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::Count(const TVal& Val) const {
  TSizeTy Count = 0;
  for (TSizeTy i = 0; i < Len(); i++){
    if (Val == ValT[i]){Count++;}}
  return Count;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::SearchBin(const TVal& Val) const {
  TSizeTy LValN=0, RValN=Len()-1;
  while (RValN>=LValN){
    TSizeTy ValN=(LValN+RValN)/2;
    if (Val==ValT[ValN]){return ValN;}
    if (Val<ValT[ValN]){RValN=ValN-1;} else {LValN=ValN+1;}
  }
  return -1;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::SearchBin(const TVal& Val, TSizeTy& InsValN) const {
  TSizeTy LValN=0, RValN=Len()-1;
  while (RValN>=LValN){
    TSizeTy ValN=(LValN+RValN)/2;
    if (Val==ValT[ValN]){InsValN=ValN; return ValN;}
    if (Val<ValT[ValN]){RValN=ValN-1;} else {LValN=ValN+1;}
  }
  InsValN=LValN; return -1;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::SearchForw(const TVal& Val, const TSizeTy& BValN) const {
  for (TSizeTy ValN=BValN; ValN<Vals; ValN++){
    if (Val==ValT[ValN]){return ValN;}}
  return -1;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::SearchBack(const TVal& Val) const {
  for (TSizeTy ValN=Vals-1; ValN>=0; ValN--){
    if (Val==ValT[ValN]){return ValN;}}
  return -1;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::SearchVForw(const TVec<TVal, TSizeTy>& ValV, const TSizeTy& BValN) const {
  TSizeTy ValVLen=ValV.Len();
  for (TSizeTy ValN=BValN; ValN<Vals-ValVLen+1; ValN++){
    bool Found=true;
    for (TSizeTy SubValN=0; SubValN<ValVLen; SubValN++){
      if (ValV[SubValN]!=GetVal(ValN+SubValN)){Found=false; break;}
    }
    if (Found){return ValN;}
  }
  return -1;
}

template <class TVal, class TSizeTy>
void TVec<TVal, TSizeTy>::FindAll(const TVal& Val, TVec<TInt, TSizeTy>& IdxV) const {
  const int Dim = Len();
  TSizeTy Count = 0;
  for (TSizeTy ValN = 0; ValN < Dim; ValN++) {
    if (ValT[ValN] == Val) { Count++; }
  }
  IdxV.Gen(Count, 0);
  for (TSizeTy ValN = 0; ValN < Dim; ValN++) {
    if (ValT[ValN] == Val) { IdxV.Add(ValT[ValN]); }
  }
}

template <class TVal, class TSizeTy>
template <typename TFun>
void TVec<TVal, TSizeTy>::FindAllSatisfy(const TFun& Fun, TVec<TInt, TSizeTy>& IdxV) const {
  const int Dim = Len();
  TSizeTy Count = 0;
  for (TSizeTy ValN = 0; ValN < Dim; ValN++) {
	if (Fun(ValT[ValN])) { Count++; }
  }
  IdxV.Gen(Count, 0);
  for (TSizeTy ValN = 0; ValN < Dim; ValN++) {
	if (Fun(ValT[ValN])) { IdxV.Add(ValT[ValN]); }
  }
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::GetMxValN() const {
  if (Vals==0){return -1;}
  TSizeTy MxValN=0;
  for (TSizeTy ValN=1; ValN<Vals; ValN++){
    if (ValT[ValN]>ValT[MxValN]){MxValN=ValN;}
  }
  return MxValN;
}

template <class TVal, class TSizeTy>
TSizeTy TVec<TVal, TSizeTy>::GetMnValN() const {
  if (Vals == 0) { return -1; }
  TSizeTy MnValN = 0;
  for (TSizeTy ValN = 1; ValN<Vals; ValN++) {
      if (ValT[ValN]<ValT[MnValN]) { MnValN = ValN; }
  }
  return MnValN;
}

template <class TVal, class TSizeTy>
uint64 TVec<TVal, TSizeTy>::GetMemUsedDeep() const {
  uint64 MemUsed = sizeof(TVec<TVal,TSizeTy>) +
         TMemUtils::GetExtraMemberSize(MxVals) +
         TMemUtils::GetExtraMemberSize(Vals);

  if (ValT != NULL && MxVals != -1){
    for (TSizeTy i = 0; i < MxVals; i++){
      MemUsed += TMemUtils::GetMemUsed(ValT[i]);
    }
  }

  return MemUsed;
}

template <class TVal, class TSizeTy>
uint64 TVec<TVal, TSizeTy>::GetMemUsedShallow() const {
  return sizeof(TVec<TVal,TSizeTy>) +
         TMemUtils::GetExtraMemberSize(MxVals) +
         TMemUtils::GetExtraMemberSize(Vals) +
         sizeof(TVal)*(MxVals != -1 ? MxVals : 0);
}

//#//////////////////////////////////////////////
/// Vector Pool. ##TVecPool
template <class TVal, class TSizeTy>
void TVecPool<TVal, TSizeTy>::Resize(const TSize& _MxVals){
  if (_MxVals <= MxVals){ return; } else { MxVals = _MxVals; }
  if (ValBf == NULL) {
    try { ValBf = new TVal [MxVals]; }
    catch (std::exception& Ex) {
      FailR(TStr::Fmt("TVecPool::Resize 1: %s, MxVals: %s. [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), TInt::GetStr(uint64(_MxVals)).CStr()).CStr()); }
    IAssert(ValBf != NULL);
    if (EmptyVal != TVal()) { PutAll(EmptyVal); }
  } else {
    // printf("*** Resize vector pool: %llu -> %llu\n", uint64(Vals), uint64(MxVals));
    TVal* NewValBf = NULL;
    try { NewValBf = new TVal [MxVals]; }
    catch (std::exception& Ex) {
      FailR(TStr::Fmt("TVecPool::Resize 1: %s, MxVals: %s. [Program failed to allocate more memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), TInt::GetStr(uint64(_MxVals)).CStr()).CStr()); }
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

template <class TVal, class TSizeTy>
TVecPool<TVal, TSizeTy>::TVecPool(const TSize& ExpectVals, const TSize& _GrowBy, const bool& _FastCopy, const TVal& _EmptyVal) : GrowBy(_GrowBy), MxVals(0), Vals(0), EmptyVal(_EmptyVal), ValBf(NULL) {
  IdToOffV.Add(0);
  Resize(ExpectVals);
}

template <class TVal, class TSizeTy>
TVecPool<TVal, TSizeTy>::TVecPool(const TVecPool& Pool) : FastCopy(Pool.FastCopy), GrowBy(Pool.GrowBy), MxVals(Pool.MxVals), Vals(Pool.Vals), EmptyVal(Pool.EmptyVal), IdToOffV(Pool.IdToOffV) {
  try {
    ValBf = new TVal [MxVals]; }
  catch (std::exception Ex) {
    FailR(TStr::Fmt("TVecPool::TVecPool: %s, MxVals: %s. [Program failed to allocate memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), TInt::GetStr(uint64(MxVals)).CStr()).CStr()); }
  IAssert(ValBf != NULL);
  if (FastCopy) {
    memcpy(ValBf, Pool.ValBf, MxVals*sizeof(TVal)); }
  else {
    for (TSize ValN = 0; ValN < MxVals; ValN++){ ValBf[ValN] = Pool.ValBf[ValN]; } }
}

template <class TVal, class TSizeTy>
TVecPool<TVal, TSizeTy>::TVecPool(TSIn& SIn) : FastCopy(SIn) {
  uint64 _GrowBy, _MxVals, _Vals;
  SIn.Load(_GrowBy); SIn.Load(_MxVals);  SIn.Load(_Vals);
  IAssertR(_GrowBy<TSizeMx && _MxVals<TSizeMx && _Vals<TSizeMx, "This is a 64-bit vector pool. Use a 64-bit compiler.");
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

template <class TVal, class TSizeTy>
void TVecPool<TVal, TSizeTy>::Save(TSOut& SOut) const {
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

template <class TVal, class TSizeTy>
TVecPool<TVal, TSizeTy>& TVecPool<TVal, TSizeTy>::operator = (const TVecPool& Pool) {
  if (this!=&Pool) {
    FastCopy = Pool.FastCopy;
    GrowBy = Pool.GrowBy;
    MxVals = Pool.MxVals;
    Vals = Pool.Vals;
    EmptyVal = Pool.EmptyVal;
    IdToOffV=Pool.IdToOffV;
    try {
      ValBf = new TVal [MxVals]; }
    catch (std::exception Ex) {
      FailR(TStr::Fmt("TVecPool::operator=: %s, MxVals: %s. [Program failed to allocate memory. Solution: Get a bigger machine and a 64-bit compiler.]", Ex.what(), TInt::GetStr(uint64(MxVals)).CStr()).CStr()); }
    IAssert(ValBf != NULL);
    if (FastCopy) {
      memcpy(ValBf, Pool.ValBf, Vals*sizeof(TVal)); }
    else {
      for (TSize ValN = 0; ValN < Vals; ValN++){ ValBf[ValN] = Pool.ValBf[ValN]; } }
  }
  return *this;
}

template <class TVal, class TSizeTy>
int TVecPool<TVal, TSizeTy>::AddV(const TValV& ValV) {
  const TSizeTy ValVLen = ValV.Len();
  if (ValVLen == 0) { return 0; }
  if ((TSizeTy)MxVals < (TSizeTy)(Vals+ValVLen)) { Resize(Vals+MAX(ValVLen, GrowBy)); }
  if (FastCopy) { memcpy(ValBf+Vals, ValV.BegI(), sizeof(TVal)*ValV.Len()); }
  else { for (int ValN=0; ValN < ValVLen; ValN++) { ValBf[Vals+ValN]=ValV[ValN]; } }
  Vals+=ValVLen;  IdToOffV.Add(Vals);
  return IdToOffV.Len()-1;
}

template <class TVal, class TSizeTy>
int TVecPool<TVal, TSizeTy>::AddEmptyV(const int& ValVLen) {
  if (ValVLen==0){return 0;}
  if (MxVals < Vals+ValVLen){Resize(Vals+MAX(TSize(ValVLen), GrowBy)); }
  Vals+=ValVLen; IdToOffV.Add(Vals);
  return IdToOffV.Len()-1;
}

// Delete all elements of value DelVal from all vectors. Empty space is left at the end of the pool.
template <class TVal, class TSizeTy>
void TVecPool<TVal, TSizeTy>::CompactPool(const TVal& DelVal) {
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
template <class TVal, class TSizeTy>
void TVecPool<TVal, TSizeTy>::ShuffleAll(TRnd& Rnd) {
  for (::TSize n = Vals-1; n > 0; n--) {
    const ::TSize k = ::TSize(((uint64(Rnd.GetUniDevInt())<<32) | uint64(Rnd.GetUniDevInt())) % (n+1));
    const TVal Tmp = ValBf[n];
    ValBf[n] = ValBf[k];
    ValBf[k] = Tmp;
  }
}

///////////////////////////////
// Linked queue

template <class TVal, class TSizeTy>
TLinkedQueue<TVal, TSizeTy>::TNode::TNode(TNode* _Next, const TVal& _Val):
		Next(_Next),
		Val(_Val) {}

template <class TVal, class TSizeTy>
TLinkedQueue<TVal, TSizeTy>::TLinkedQueue():
		First(nullptr),
		Last(nullptr),
		Size() {
	Size = 0;	// explicitly set the size in case the default constructor does nothing
}

template <class TVal, class TSizeTy>
TLinkedQueue<TVal, TSizeTy>::TLinkedQueue(TSIn& SIn):
		First(nullptr),
		Last(nullptr),
		Size() {
	Size = 0;	// explicitly set the size in case the default constructor does nothing

	const TSizeTy FinalSize(SIn);
	for (TSizeTy i = 0; i < FinalSize; i++) {
		Push(TVal(SIn));
	}
}

template <class TVal, class TSizeTy>
TLinkedQueue<TVal, TSizeTy>::~TLinkedQueue() {
	while (!Empty()) { DelFirst(); }
}

template <class TVal, class TSizeTy>
void TLinkedQueue<TVal, TSizeTy>::Save(TSOut& SOut) const {
	Size.Save(SOut);

	TNode* Curr = First;
	while (Curr != nullptr) {
		Curr->Val.Save(SOut);
		Curr = Curr->Next;
	}
}

template <class TVal, class TSizeTy>
void TLinkedQueue<TVal, TSizeTy>::Push(const TVal& Val) {
	TNode* Node = new TNode(nullptr, Val);

	if (Size++ == 0) {
		First = Node;
	} else {
		Last->Next = Node;
	}

	Last = Node;
}

template <class TVal, class TSizeTy>
TVal TLinkedQueue<TVal, TSizeTy>::Pop() {
	TVal Result = Peek();
	DelFirst();
	return Result;
}

template <class TVal, class TSizeTy>
TVal& TLinkedQueue<TVal, TSizeTy>::Peek() {
	IAssertR(!Empty(), "Cannot peek when the queue is empty!");
	return First->Val;
}

template <class TVal, class TSizeTy>
void TLinkedQueue<TVal, TSizeTy>::DelFirst() {
	IAssertR(!Empty(), "Cannot delete elements from empty buffer!");

	TNode* Temp = First;

	if (--Size == 0) {
		First = nullptr;
		Last = nullptr;
	} else {
		First = First->Next;
	}

	delete Temp;
}

/////////////////////////////////////////////////
// 2D-Vector

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::SwapX(const TSizeTy& X1, const TSizeTy& X2){
  for (TSizeTy Y=0; Y<TSizeTy(YDim); Y++){
    TVal Val=At(X1, Y); At(X1, Y)=At(X2, Y); At(X2, Y)=Val;}
}


template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::SwapY(const TSizeTy& Y1, const TSizeTy& Y2){
  for (TSizeTy X=0; X<TSizeTy(XDim); X++){
    TVal Val=At(X, Y1); At(X, Y1)=At(X, Y2); At(X, Y2)=Val;}
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::Swap(TVVec<TVal, TSizeTy, colmajor>& Vec){  //J:
	if (this != &Vec){
		::Swap(XDim, Vec.XDim);
		::Swap(YDim, Vec.YDim);
		ValV.Swap(Vec.ValV);
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::ShuffleX(TRnd& Rnd){
  if (GetXDim() < TInt::Mx) {
    for (TSizeTy X=0; X<XDim-1; X++){SwapX(X, X+Rnd.GetUniDevInt(XDim-X));}
  } else {
	for (TSizeTy X=0; X<XDim-1; X++){SwapX(X, X+Rnd.GetUniDevInt64(XDim-X));}
  }
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::ShuffleY(TRnd& Rnd){
  if (GetYDim() < TInt::Mx) {
    for (TSizeTy Y=0; Y<YDim-1; Y++){SwapY(Y, Y+Rnd.GetUniDevInt(YDim-Y));}
  } else {
	for (TSizeTy Y=0; Y<YDim-1; Y++){SwapY(Y, Y+Rnd.GetUniDevInt64(YDim-Y));}
  }
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::GetMxValXY(TSizeTy& X, TSizeTy& Y) const {
  TSizeTy MxValN=ValV.GetMxValN();
  if (colmajor){
	  Y = MxValN%YDim;
	  X = MxValN / YDim;
  }
  else{
	  Y = MxValN%XDim;
	  X = MxValN / XDim;
  }
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::CopyFrom(const  TVVec<TVal, TSizeTy, colmajor>& VVec){
	TSizeTy CopyXDim = MIN(GetXDim(), VVec.GetXDim());
	TSizeTy CopyYDim = MIN(GetYDim(), VVec.GetYDim());
	for (TSizeTy X = 0; X<CopyXDim; X++){
		for (TSizeTy Y = 0; Y<CopyYDim; Y++){
			At(X, Y) = VVec.At(X, Y);
		}
	}
}


template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::AddXDim(){
	TVVec<TVal, TSizeTy, colmajor> NewVVec(XDim + 1, YDim);
	NewVVec.CopyFrom(*this);
	*this = NewVVec;
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::AddYDim(const TSizeTy& NDims){
	TVVec<TVal, TSizeTy, colmajor> NewVVec(XDim, YDim + NDims);
	NewVVec.CopyFrom(*this);
	*this = NewVVec;
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::DelX(const TSizeTy& X){
	TVVec<TVal, TSizeTy, colmajor> NewVVec(XDim - 1, YDim);
	for (TSizeTy Y = 0; Y<YDim; Y++){
		for (TSizeTy LX = 0; LX<X; LX++){
			NewVVec.At(LX, Y) = At(LX, Y);
		}
		for (TSizeTy RX = X + 1; RX<XDim; RX++){
			NewVVec.At(RX - 1, Y) = At(RX, Y);
		}
	}
	*this = NewVVec;
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::DelY(const TSizeTy& Y){
	TVVec<TVal, TSizeTy, colmajor> NewVVec(XDim, YDim - 1);
	for (TSizeTy X = 0; X<XDim; X++){
		for (TSizeTy LY = 0; LY<Y; LY++){
			NewVVec.At(X, LY) = At(X, LY);
		}
		for (TSizeTy RY = Y + 1; RY<YDim; RY++){
			NewVVec.At(X, RY - 1) = At(X, RY);
		}
	}
	*this = NewVVec;
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::DelCols(const TVec<TNum<TSizeTy>, TSizeTy>& ColIdV) {
	THashSet<TNum<TSizeTy>> ColIdSet;
	for (int ColIdN = 0; ColIdN < ColIdV.Len(); ColIdN++) {
		ColIdSet.AddKey(ColIdV[ColIdN]);
	}

	TVVec<TVal,TSizeTy,colmajor> NewVVec(XDim, YDim - ColIdSet.Len());

	int NewColN = 0;
	TVec<TVal, TSizeTy> Col;
	for (int ColN = 0; ColN < YDim; ColN++) {
		if (!ColIdSet.IsKey(ColN)) {
			GetCol(ColN, Col);
			NewVVec.SetCol(NewColN++, Col);
		}
	}

#ifdef GLib_CPP11
	*this = std::move(NewVVec);
#else
	*this = NewVVec;
#endif
}

template <class TVal, class TSizeTy, bool colmajor>
void TVVec<TVal, TSizeTy, colmajor>::GetRow(const TSizeTy& RowN, TVec<TVal, TSizeTy>& Vec) const {
	EAssert((0 <= RowN) && (RowN<TSizeTy(XDim)));
	if (Vec.Len() != GetCols()) { Vec.Gen(GetCols()); }
	for (TSizeTy ColN = 0; ColN < YDim; ColN++) {
		Vec[ColN] = At(RowN, ColN);
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetRowPtr(const TSizeTy& RowN, TVec<TVal, TSizeTy>& Vec){
	EAssert((0<=RowN)&&(RowN<TSizeTy(XDim)));
	if (!colmajor){
		Vec.GenExt(&ValV[RowN*YDim], YDim);
	}
	else{
		GetRow(RowN, Vec);
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetRowsPtr(const TSizeTy& RowStart, const TSizeTy& RowEnd, TVVec<TVal, TSizeTy>& VVec){
	EAssert((0 <= RowStart) && (RowStart<TSizeTy(XDim)));
	EAssert((0 <= RowStart) && (RowEnd<TSizeTy(XDim)));
	if (!colmajor){
		this->GetExtRows(VVec, RowStart, RowEnd);
	}
	else{
		//Allocate
		VVec.Gen((RowEnd - RowStart + 1), YDim);
		for (TSizeTy c = 0; c < YDim; c++){
			for (TSizeTy r = RowStart; r <= RowEnd; r++){
				VVec.ValV.Add(At(c, r));
			}
		}
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetRowSIter(const TSizeTy& RowN, TSIter<TVal, TSizeTy>& SIter) {
	Assert((0 <= RowN) && (RowN<TSizeTy(XDim)));
	if (!colmajor){
		SIter = TSIter<TVal, TSizeTy>(&this->Get1DVec(), RowN*YDim, 1);
	}
	else{
		SIter = TSIter<TVal, TSizeTy>(&this->Get1DVec(), RowN, YDim);
	}
}


template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetCol(const TSizeTy& ColN, TVec<TVal, TSizeTy>& Vec) const {
	EAssert((0 <= ColN) && (ColN<TSizeTy(YDim)));
	if (Vec.Len() != GetRows()) { Vec.Gen(GetRows()); }
	for (TSizeTy RowN = 0; RowN < XDim; RowN++) {
		Vec[RowN] = At(RowN, ColN);
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetColPtr(const TSizeTy& ColN, TVec<TVal, TSizeTy>& Vec){
	Assert((0 <= ColN) && (ColN<TSizeTy(YDim)));
	if (colmajor){
		Vec.GenExt(&ValV[ColN*XDim], XDim);
	}
	else
	{
		GetCol(ColN, Vec);
	}
}



template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::GetColSIter(const TSizeTy& ColN, TSIter<TVal, TSizeTy>& SIter){
	Assert((0 <= ColN) && (ColN<TSizeTy(YDim)));
	if (!colmajor){
		SIter = TSIter<TVal, TSizeTy>(&this->Get1DVec(), ColN, XDim);
	}
	else{
		SIter = TSIter<TVal, TSizeTy>(&this->Get1DVec(), ColN*XDim, 1);
	}
}

template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::SetRow(const TSizeTy& RowN, const TVec<TVal, TSizeTy>& Vec) {
	EAssert((0<=RowN)&&(RowN<TSizeTy(XDim))&&(Vec.Len()==TSizeTy(YDim)));
	for (TSizeTy ColN = 0; ColN < YDim; ColN++) {
		At(RowN, ColN) = Vec[ColN];
	}
}


template <class TVal, class TSizeTy, bool colmajor>
void  TVVec<TVal, TSizeTy, colmajor>::SetCol(const TSizeTy& ColN, const TVec<TVal, TSizeTy>& Vec) {
	EAssert((0 <= ColN) && (ColN<TSizeTy(YDim)) && (Vec.Len() == TSizeTy(XDim)));
	for (TSizeTy RowN = 0; RowN < XDim; RowN++) {
		At(RowN, ColN) = Vec[RowN];
	}
}

/////////////////////////////////////////////////
// Tree

template <class TVal>
void TTree<TVal>::GetNodeIdV(TIntV& NodeIdV, const int& NodeId){
  if (NodeId==0){NodeIdV.Clr(); if (GetNodes()==0){return;}}
  else if (GetParentNodeId(NodeId)==-1){return;}
  NodeIdV.Add(NodeId);
  for (int ChildN=0; ChildN<GetChildren(NodeId); ChildN++){
    int ChildNodeId=GetChildNodeId(NodeId, ChildN);
    if (ChildNodeId!=-1){
      GetNodeIdV(NodeIdV, ChildNodeId);
    }
  }
}

template <class TVal>
void TTree<TVal>::GenRandomTree(const int& Nodes, TRnd& Rnd){
  Clr();
  if (Nodes>0){
    AddRoot(TVal());
    for (int NodeN=1; NodeN<Nodes; NodeN++){
      int ParentNodeId=Rnd.GetUniDevInt(0, GetNodes()-1);
      AddNode(ParentNodeId, TVal());
    }
  }
}

template <class TVal>
void TTree<TVal>::DelNode(const int& NodeId){
  if (NodeId==0){
    Clr();
  } else {
    TIntV& ChildNodeIdV=NodeV[GetParentNodeId(NodeId)].Val2;
    int ChildNodeIdN=ChildNodeIdV.SearchForw(NodeId);
    ChildNodeIdV[ChildNodeIdN]=-1;
  }
}

template <class TVal>
void TTree<TVal>::CopyTree(const int& SrcNodeId, TTree& DstTree, const int& DstParentNodeId){
  int DstNodeId=DstTree.AddNode(DstParentNodeId, GetNodeVal(SrcNodeId));
  for (int ChildN=0; ChildN<GetChildren(SrcNodeId); ChildN++){
    int ChildNodeId=GetChildNodeId(SrcNodeId, ChildN);
    if (ChildNodeId!=-1){
      CopyTree(ChildNodeId, DstTree, DstNodeId);
    }
  }
}

template <class TVal>
void TTree<TVal>::WrTree(const int& NodeId, const int& Lev){
  for (int LevN=0; LevN<Lev; LevN++){printf("| ");}
  printf("%d (%d)\n", NodeId, GetChildren(NodeId));
  for (int ChildN=0; ChildN<GetChildren(NodeId); ChildN++){
    int ChildNodeId=GetChildNodeId(NodeId, ChildN);
    if (ChildNodeId!=-1){
      WrTree(ChildNodeId, Lev+1);
    }
  }
}

/////////////////////////////////////////////////
// List
template <class TVal>
TLst<TVal>::TLst(TSIn& SIn):
  Nds(0), FirstNd(NULL), LastNd(NULL){
  int CheckNds=0; SIn.Load(CheckNds);
  for (int NdN=0; NdN<CheckNds; NdN++){AddBack(TVal(SIn));}
  Assert(Nds==CheckNds);
}

template <class TVal>
TLst<TVal>& TLst<TVal>::operator=(const TLst<TVal>& Val){
	if (this != &Val) {
		Clr();
		PLstNd Nd = Val.First();
		while (Nd != NULL) {
			AddBack(TVal(Nd->Val));
			Nd = Nd->NextNd;
		}
	}
	return *this;
}

template <class TVal>
void TLst<TVal>::Save(TSOut& SOut) const {
  SOut.Save(Nds);
  PLstNd Nd=FirstNd; int CheckNds=0;
  while (Nd!=NULL){
    Nd->Val.Save(SOut); Nd=Nd->NextNd; CheckNds++;}
  IAssert(Nds==CheckNds);
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::AddFront(const TVal& Val){
  PLstNd Nd=new TLstNd<TVal>(NULL, FirstNd, Val);
  if (FirstNd!=NULL){FirstNd->PrevNd=Nd; FirstNd=Nd;}
  else {FirstNd=Nd; LastNd=Nd;}
  Nds++; return Nd;
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::AddBack(const TVal& Val){
  PLstNd Nd=new TLstNd<TVal>(LastNd, NULL, Val);
  if (LastNd!=NULL){LastNd->NextNd=Nd; LastNd=Nd;}
  else {FirstNd=Nd; LastNd=Nd;}
  Nds++; return Nd;
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::AddFrontSorted(const TVal& Val, const bool& Asc){
  PLstNd Nd=First();
  if (Nd==NULL){
    return Ins(Nd, Val);
  } else {
    while ((Nd!=NULL)&&((Asc&&(Val>Nd->Val))||(!Asc&&(Val<Nd->Val)))){
      Nd=Nd->Next();}
    if (Nd==NULL){return Ins(Last(), Val);}
    else {return Ins(Nd->Prev(), Val);}
  }
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::AddBackSorted(const TVal& Val, const bool& Asc){
  PLstNd Nd=Last();
  while ((Nd!=NULL)&&((Asc&&(Val<Nd->Val))||(!Asc&&(Val>Nd->Val)))){
    Nd=Nd->Prev();}
  return Ins(Nd, Val);
}

template <class TVal>
void TLst<TVal>::PutFront(const PLstNd& Nd){
  Assert(Nd!=NULL);
  // unchain
  if (Nd->PrevNd==NULL){FirstNd=Nd->NextNd;}
  else {Nd->PrevNd->NextNd=Nd->NextNd;}
  if (Nd->NextNd==NULL){LastNd=Nd->PrevNd;}
  else {Nd->NextNd->PrevNd=Nd->PrevNd;}
  // add to front
  Nd->PrevNd=NULL; Nd->NextNd=FirstNd;
  if (FirstNd!=NULL){FirstNd->PrevNd=Nd; FirstNd=Nd;}
  else {FirstNd=Nd; LastNd=Nd;}
}

template <class TVal>
void TLst<TVal>::PutBack(const PLstNd& Nd){
  Assert(Nd!=NULL);
  // unchain
  if (Nd->PrevNd==NULL){FirstNd=Nd->NextNd;}
  else {Nd->PrevNd->NextNd=Nd->NextNd;}
  if (Nd->NextNd==NULL){LastNd=Nd->PrevNd;}
  else {Nd->NextNd->PrevNd=Nd->PrevNd;}
  // add to back
  Nd->PrevNd=LastNd; Nd->NextNd=NULL;
  if (LastNd!=NULL){LastNd->NextNd=Nd; LastNd=Nd;}
  else {FirstNd=Nd; LastNd=Nd;}
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::Ins(const PLstNd& Nd, const TVal& Val){
  if (Nd==NULL){return AddFront(Val);}
  else if (Nd->NextNd==NULL){return AddBack(Val);}
  else {
    PLstNd NewNd=new TLstNd<TVal>(Nd, Nd->NextNd, Val);
    Nd->NextNd=NewNd; NewNd->NextNd->PrevNd=NewNd;
    Nds++; return NewNd;
  }
}

template <class TVal>
void TLst<TVal>::Del(const TVal& Val){
  PLstNd Nd=SearchForw(Val);
  if (Nd!=NULL){Del(Nd);}
}

template <class TVal>
void TLst<TVal>::Del(const PLstNd& Nd){
  Assert(Nd!=NULL);
  if (Nd->PrevNd==NULL){FirstNd=Nd->NextNd;}
  else {Nd->PrevNd->NextNd=Nd->NextNd;}
  if (Nd->NextNd==NULL){LastNd=Nd->PrevNd;}
  else {Nd->NextNd->PrevNd=Nd->PrevNd;}
  Nds--; delete Nd;
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::SearchForw(const TVal& Val){
  PLstNd Nd=First();
  while (Nd!=NULL){
    if (Nd->GetVal()==Val){return Nd;}
    Nd=Nd->Next();
  }
  return NULL;
}

template <class TVal>
TLstNd<TVal>* TLst<TVal>::SearchBack(const TVal& Val){
  PLstNd Nd=Last();
  while (Nd!=NULL){
    if (Nd->GetVal()==Val){return Nd;}
    Nd=Nd->Prev();
  }
  return NULL;
}

template <class TVal>
uint64 TLst<TVal>::GetMemUsed(const bool& DeepP) const {
  if (DeepP) {
    uint64 MemUsed = sizeof(TLst<TVal>);
    PLstNd Nd=First();
    while (Nd!=NULL){
      MemUsed += TMemUtils::GetMemUsed(Nd);
      Nd=Nd->Next();
    }
    return MemUsed;
  } else {
    return sizeof(TLst<TVal>) + sizeof(TVal) * Nds;
  }
}

//#//////////////////////////////////////////////
/// Simple heap data structure. ##THeap
template <class TVal, class TCmp>
void THeap<TVal, TCmp>::PushHeap(const TVal& Val) {
  HeapV.Add(Val);
  PushHeap(0, HeapV.Len()-1, 0, Val);
}

template <class TVal, class TCmp>
TVal THeap<TVal, TCmp>::PopHeap() {
  IAssert(! HeapV.Empty());
  const TVal Top = HeapV[0];
  HeapV[0] = HeapV.Last();
  HeapV.DelLast();
  if (! HeapV.Empty()) {
    AdjustHeap(0, 0, HeapV.Len(), HeapV[0]);
  }
  return Top;
}

template <class TVal, class TCmp>
void THeap<TVal, TCmp>::PushHeap(const int& First, int HoleIdx, const int& Top, TVal Val) {
  int Parent = (HoleIdx-1)/2;
  while (HoleIdx > Top && Cmp(HeapV[First+Parent], Val)) {
    HeapV[First+HoleIdx] = HeapV[First+Parent];
    HoleIdx = Parent;  Parent = (HoleIdx-1)/2;
  }
  HeapV[First+HoleIdx] = Val;
}

template <class TVal, class TCmp>
void THeap<TVal, TCmp>::AdjustHeap(const int& First, int HoleIdx, const int& Len, TVal Val) {
  const int Top = HoleIdx;
  int Right = 2*HoleIdx+2;
  while (Right < Len) {
    if (Cmp(HeapV[First+Right], HeapV[First+Right-1])) { Right--; }
    HeapV[First+HoleIdx] = HeapV[First+Right];
    HoleIdx = Right;  Right = 2*(Right+1); }
  if (Right == Len) {
    HeapV[First+HoleIdx] = HeapV[First+Right-1];
    HoleIdx = Right-1; }
  PushHeap(First, HoleIdx, Top, Val);
}

template <class TVal, class TCmp>
void THeap<TVal, TCmp>::MakeHeap(const int& First, const int& Len) {
  if (Len < 2) { return; }
  int Parent = (Len-2)/2;
  while (true) {
    AdjustHeap(First, Parent, Len, HeapV[First+Parent]);
    if (Parent == 0) { return; }
    Parent--;
  }
}

#endif /* SRC_GLIB_BASE_DS_HPP_ */

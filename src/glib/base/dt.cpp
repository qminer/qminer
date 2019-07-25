/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Random
const int TRnd::RndSeed=0;
const int TRnd::a=16807;
const int TRnd::m=2147483647;
const int TRnd::q=127773; // m DIV a
const int TRnd::r=2836; // m MOD a

void TRnd::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Seed=TXmlObjSer::GetIntArg(XmlTok, "Seed");
}

void TRnd::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Seed", TInt::GetStr(Seed));
}

void TRnd::PutSeed(const int& _Seed){
  Assert(_Seed>=0);
  if (_Seed==0){
    //Seed=int(time(NULL));
    Seed=abs(int(TSysTm::GetPerfTimerTicks()));
  } else {
    Seed=_Seed;
    //Seed=abs(_Seed*100000)+1;
  }
}

void TRnd::Move(const int& Steps){
  for (int StepN=0; StepN<Steps; StepN++){GetNextSeed();}
}

bool TRnd::Check(){
  int PSeed=Seed; Seed=1;
  for (int SeedN=0; SeedN<10000; SeedN++){GetNextSeed();}
  bool Ok=Seed==1043618065; Seed=PSeed; return Ok;
}

int TRnd::GetUniDevInt(const int& Range){
  int Seed=GetNextSeed();
  if (Range==0){return Seed;}
  else {return Seed%Range;}
}

uint TRnd::GetUniDevUInt(const uint& Range){
  uint Seed=uint(GetNextSeed()%0x10000)*0x10000+uint(GetNextSeed()%0x10000);
  if (Range==0){return Seed;}
  else {return Seed%Range;}
}

int64 TRnd::GetUniDevInt64(const int64& Range){
  const int64 RndVal = int64((uint64(GetUniDevInt())<<32) | uint64(GetUniDevInt()));
  if (Range==0){return RndVal;}
  else {return RndVal%Range;}
}

uint64 TRnd::GetUniDevUInt64(const uint64& Range){
 const uint64 RndVal = uint64((uint64(GetUniDevInt())<<32) | uint64(GetUniDevInt()));
 if (Range==0){return RndVal;}
 else {return RndVal%Range;}
}

double TRnd::GetNrmDev(){
  double v1, v2, rsq;
  do {
    v1=2.0*GetUniDev()-1.0; // pick two uniform numbers in the square
    v2=2.0*GetUniDev()-1.0; // extending from -1 to +1 in each direction
    rsq=v1*v1+v2*v2; // see if they are in the unit cicrcle
  } while ((rsq>=1.0)||(rsq==0.0)); // and if they are not, try again
  double fac=sqrt(-2.0*log(rsq)/rsq); // Box-Muller transformation
  return v1*fac;
//  return v2*fac; // second deviate
}

double TRnd::GetNrmDev(
 const double& Mean, const double& SDev, const double& Mn, const double& Mx){
  double Val=Mean+GetNrmDev()*SDev;
  if (Val<Mn){Val=Mn;}
  if (Val>Mx){Val=Mx;}
  return Val;
}

double TRnd::GetExpDev(){
  double UniDev;
  do {
    UniDev=GetUniDev();
  } while (UniDev==0.0);
  return -log(UniDev);
}

double TRnd::GetExpDev(const double& Lambda) {
  return GetExpDev()/Lambda;
}

double TRnd::GetGammaDev(const int& Order){
  int j;
  double am,e,s,v1,v2,x,y;
  if (Order<1){Fail;}
  if (Order<6) {
    x=1.0;
    for (j=1;j<=Order;j++) x *=GetUniDev();
    x = -log(x);
  } else {
    do {
      do {
        do {
          v1=2.0*GetUniDev()-1.0;
          v2=2.0*GetUniDev()-1.0;
        } while (v1*v1+v2*v2 > 1.0);
        y=v2/v1;
        am=Order-1;
        s=sqrt(2.0*am+1.0);
        x=s*y+am;
      } while (x <= 0.0);
      e=(1.0+y*y)*exp(am*log(x/am)-s*y);
    } while (GetUniDev()>e);
  }
  return x;
}

double TRnd::GetPoissonDev(const double& Mean){
  static double sq,alxm,g,oldm=(-1.0);
  double em,t,y;
  if (Mean < 12.0) {
    if (Mean != oldm) {
      oldm=Mean;
      g=exp(-Mean);
    }
    em = -1;
    t=1.0;
    do {
      ++em;
      t *= GetUniDev();
    } while (t>g);
  } else {
    if (Mean != oldm) {
      oldm=Mean;
      sq=sqrt(2.0*Mean);
      alxm=log(Mean);
      g=Mean*alxm-TSpecFunc::LnGamma(Mean+1.0);
    }
    do {
      do {
        y=tan(TMath::Pi*GetUniDev());
        em=sq*y+Mean;
      } while (em < 0.0);
      em=floor(em);
      t=0.9*(1.0+y*y)*exp(em*alxm-TSpecFunc::LnGamma(em+1.0)-g);
    } while (GetUniDev()>t);
  }
  return em;
}

double TRnd::GetBinomialDev(const double& Prb, const int& Trials){
  int j;
  static int nold=(-1);
  double am,em,g,angle,p,bnl,sq,t,y;
  static double pold=(-1.0),pc,plog,pclog,en,oldg;

  p=(Prb <= 0.5 ? Prb : 1.0-Prb);
  am=Trials*p;
  if (Trials < 25) {
    bnl=0.0;
    for (j=1;j<=Trials;j++)
      if (GetUniDev() < p) ++bnl;
  } else if (am < 1.0) {
    g=exp(-am);
    t=1.0;
    for (j=0;j<=Trials;j++) {
      t *= GetUniDev();
      if (t < g) break;
    }
    bnl=(j <= Trials ? j : Trials);
  } else {
    if (Trials != nold) {
      en=Trials;
      oldg=TSpecFunc::LnGamma(en+1.0);
      nold=Trials;
    } if (p != pold) {
      pc=1.0-p;
      plog=log(p);
      pclog=log(pc);
      pold=p;
    }
    sq=sqrt(2.0*am*pc);
    do {
      do {
        angle=TMath::Pi*GetUniDev();
        y=tan(angle);
        em=sq*y+am;
      } while (em < 0.0 || em >= (en+1.0));
      em=floor(em);
      t=1.2*sq*(1.0+y*y)*exp(oldg-(em+1.0)
        -TSpecFunc::LnGamma(en-em+1.0)+em*plog+(en-em)*pclog);
    } while (GetUniDev() > t);
    bnl=em;
  }
  if (p != Prb) bnl=Trials-bnl;
  return bnl;
}

// sample points from d-dimensional unit sphere
/*void TRnd::GetSphereDev(const int& Dim, TFltV& ValV) {
  if (ValV.Len() != Dim) { ValV.Gen(Dim); }
  double Length = 0.0;
  for (int i = 0; i < Dim; i++) {
    ValV[i] = GetNrmDev();
    Length += TMath::Sqr(ValV[i]); }
  Length = 1.0 / sqrt(Length);
  for (int i = 0; i < Dim; i++) {
    ValV[i] *= Length;
  }
}*/

TRnd TRnd::LoadTxt(TILx& Lx){
  return TRnd(Lx.GetInt());
}

void TRnd::SaveTxt(TOLx& Lx) const {
  Lx.PutInt(Seed);
}

uint64 TRnd::GetMemUsed() const {
    return sizeof(TRnd) + TMemUtils::GetExtraMemberSize(Seed);
}

/////////////////////////////////////////////////
/// Thin Input-Memory
TThinMIn::TThinMIn() :
    TSBase(), TSIn(), Bf(NULL), BfC(0), BfL(0) { }

TThinMIn::TThinMIn(const TMemBase& Mem) :
TSBase(), TSIn(), Bf(NULL), BfC(0), BfL(0) {

	Bf = (uchar*)Mem.GetBf();
	BfL = Mem.Len();
}

TThinMIn::TThinMIn(const void* _Bf, const int& _BfL) :
TSBase(), TSIn(),
Bf(NULL), BfC(0), BfL(_BfL) {

	Bf = (uchar*)_Bf;
}

TThinMIn::TThinMIn(const TThinMIn& min) :
TSBase(), TSIn() {
	Bf = min.Bf;
	BfL = min.BfL;
	BfC = min.BfC;
}

char TThinMIn::GetCh() {
	IAssertR(BfC<BfL, "Reading beyond the end of stream.");
	return Bf[BfC++];
}

char TThinMIn::PeekCh() {
	IAssertR(BfC<BfL, "Reading beyond the end of stream.");
	return Bf[BfC];
}

int TThinMIn::GetBf(const void* LBf, const TSize& LBfL) {
	IAssert(TSize(BfC + LBfL) <= TSize(BfL));
	int LBfS = 0;
	for (TSize LBfC = 0; LBfC<LBfL; LBfC++) {
		LBfS += (((char*)LBf)[LBfC] = Bf[BfC++]);
	}
	return LBfS;
}

void TThinMIn::MoveTo(int Offset) {
	IAssertR(Offset<BfL, "Reading beyond the end of stream.");
	BfC = Offset;
}

bool TThinMIn::GetNextLnBf(TChA& LnChA) {
	return GetNextLn(LnChA);
}

TStr TThinMIn::GetSNm() const {
    return "Thin input memory";
}
/////////////////////////////////////////////////
// Memory

void TMem::Copy(const TMem& Mem) {
  if (this != &Mem) {
    if (Bf != NULL) { delete[] Bf; }
    MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL;
    if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL); }
  }
}
void TMem::Copy(const TMemBase& Mem) {
  if (Bf != NULL) { delete[] Bf; }
  MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL;
  if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL); }
}

void TMem::Resize(const int& _MxBfL){
  if (_MxBfL<=MxBfL){return;}
  else {if (MxBfL*2<_MxBfL){MxBfL=_MxBfL;} else {MxBfL*=2;}}
  char* NewBf=new char[MxBfL]; IAssert(NewBf!=NULL);
  if (BfL>0){memcpy(NewBf, Bf, BfL);}
  if (Bf!=NULL){delete[] Bf;}
  Bf=NewBf;
}

TMem::TMem(const TStr& Str) {
  MxBfL = Str.Len(); BfL = MxBfL; Bf = NULL;
  if (MxBfL>0){
    Bf=new char[MxBfL];
    if (BfL>0){memcpy(Bf, Str.CStr(), BfL);}
  }
}

void TMem::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveHdArg(Nm, "BfL", TInt::GetStr(BfL));
  SOut.PutStr(TXmlLx::GetXmlStrFromPlainMem(*this));
}

bool TMem::DoFitStr(const TStr& Str) const {
  return DoFitLen(Str.Len()+1);
}

TMem& TMem::operator+=(const char& Ch){
  if (BfL==MxBfL){Resize(BfL+1);}
  Bf[BfL]=Ch; BfL++; return *this;
}

TMem& TMem::operator+=(const TMem& Mem){
  int LBfL=Mem.Len(); if (BfL+LBfL>MxBfL){Resize(BfL+LBfL);}
  if (LBfL>0){memcpy(&Bf[BfL], Mem(), LBfL);}
  BfL+=LBfL; return *this;
}

TMem& TMem::operator+=(const TStr& Str){
  int LBfL=Str.Len(); if (BfL+LBfL>MxBfL){Resize(BfL+LBfL);}
  if (LBfL>0){memcpy(Bf+BfL, Str.CStr(), LBfL);}
  BfL+=LBfL; return *this;
}

TMem& TMem::operator+=(const PSIn& SIn){
  int LBfL=SIn->Len(); if (BfL+LBfL>MxBfL){Resize(BfL+LBfL);}
  char* LBf=new char[LBfL];
  SIn->GetBf(LBf, LBfL);
  if (LBfL>0){memcpy(Bf+BfL, LBf, LBfL);}
  delete[] LBf;
  BfL+=LBfL; return *this;
}

TMem& TMem::operator=(const TMem& Mem) {
  if (this != &Mem) {
    if (Bf != NULL) { delete[] Bf; }
    MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL;
    if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL); }
  }
  return *this;
}
TMem& TMem::operator=(TMem&& Src) {
  if (this != &Src) {
    if (Bf != NULL) { delete[] Bf; }
    MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = Src.Bf;
    Src.MxBfL = Src.BfL = 0; Src.Bf = NULL;
  }
  return *this;
}
TMem& TMem::operator=(const TMemBase& Mem) {
  if (Bf != NULL) { delete[] Bf; }
  MxBfL = Mem.MxBfL; BfL = Mem.BfL; Bf = NULL;
  if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Mem.Bf, BfL); }
  return *this;
}
TMem& TMem::operator=(TMemBase&& Src) {
  if (Bf != NULL) { delete[] Bf; }
  if (Src.Owner) { // take over the buffer
    MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = Src.Bf;
    Src.MxBfL = Src.BfL = 0; Src.Bf = NULL; Src.Owner = false;
  }
  else { // copy the contents
    MxBfL = Src.MxBfL; BfL = Src.BfL; Bf = NULL;
    if (MxBfL>0) { Bf = new char[MxBfL]; memcpy(Bf, Src.Bf, BfL); }
  }
  return *this;
}

void TMem::Del(const int& BChN, const int& EChN){
  if (BChN>EChN){return;}
  if ((BChN==0)&&(EChN==BfL-1)){Clr(); return;}
  IAssert((0<=BChN)&&(BChN<=EChN)&&(EChN<BfL));
  memmove(Bf+BChN, Bf+EChN+1, BfL-EChN-1);
  BfL-=(EChN-BChN+1);
}

//int TMem::AddStr(const TStr& Str){
//  int LBfL=Str.Len()+1; Resize(BfL+LBfL);
//  if (LBfL>0){memcpy(&Bf[BfL], Str.CStr(), LBfL);}
//  int FChN=BfL; BfL+=LBfL; return FChN;
//}

void TMem::AddBf(const void* _Bf, const int& _BfL){
  IAssert((_BfL>=0) && (_Bf != NULL));
  Reserve(Len() + _BfL, false);
  memcpy(Bf + BfL, _Bf, _BfL);
  BfL+=_BfL;
  //char* ChBf=(char*)Bf;
  //for (int BfC=0; BfC<BfL; BfC++){
  //  char Ch=ChBf[BfC];
  //  operator+=(Ch);
  //}
}

TStr TMem::GetAsStr(const char& NewNullCh) const {
  if (NewNullCh!='\0'){
    TChA ChA(*this);
    ChA.ChangeCh('\0', NewNullCh);
    return ChA;
  } else {
    return TStr(*this);
  }
}

TStr TMem::GetHexStr() const {
	TChA ChA;
	for (int ChN = 0; ChN< BfL ; ChN++){
		uchar Ch = uchar(Bf[ChN]);
		char MshCh = TCh::GetHexCh((Ch / 16) % 16);
		char LshCh = TCh::GetHexCh(Ch % 16);
		ChA += MshCh; ChA += LshCh;
	}
	return TStr(ChA);
}

TMem TMem::GetFromHex(const TStr& Str) {
	int StrLen = Str.Len(); IAssert(StrLen % 2 == 0);
	TChA ChA; int ChN = 0;
	while (ChN<StrLen){
		char MshCh = Str.GetCh(ChN); ChN++;
		char LshCh = Str.GetCh(ChN); ChN++;
		uchar Ch = uchar(TCh::GetHex(MshCh) * 16 + TCh::GetHex(LshCh));
		ChA += Ch;
	}
	return TMem(ChA.CStr(), ChA.Len());
}

///////////////////////////////////////////////////////////////
// Base64 encoding
// Code skeleton taken from http://www.adp-gmbh.ch/cpp/common/base64.html
// on 5.12.2015 and re-arranged to fit into glib.

const TStr TStr::base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

TStr TStr::Base64Encode(const void* Bf, const int BfL) {
	TStr ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	int BfLTmp = BfL;
	const uchar* BfTmp = (uchar*)Bf;
	while (BfLTmp--) {
		char_array_3[i++] = *(BfTmp++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++) {
			char_array_3[j] = '\0';
		}
		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) {
			ret += base64_chars[char_array_4[j]];
		}
		while ((i++ < 3)) {
			ret += '=';
		}
	}
	return ret;
}

void TStr::Base64Decode(const TStr& In, TMem& Mem) {
	int in_len = In.Len();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	Mem.Reserve(In.Len());
	while (in_len-- && (In[in_] != '=') && is_base64(In[in_])) {
		char_array_4[i++] = In[in_];
		in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++) {
				char_array_4[i] = (uchar)base64_chars.SearchCh(char_array_4[i]);
			}
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			Mem.AddBf(&char_array_3, 3);
			//for (i = 0; (i < 3); i++) {
			//	Mem.AddBf(&char_array_3[i], 1);
			//}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++) {
			char_array_4[j] = 0;
		}
		for (j = 0; j < 4; j++) {
			char_array_4[j] = (uchar)base64_chars.SearchCh(char_array_4[j]);
		}
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) {
			Mem.AddBf(&char_array_3[j], 1);
		}
	}
}

/////////////////////////////////////////////////
// Input-Memory
TMemIn::TMemIn(const TMem& _Mem, const int& _BfC):
  TSBase(), TSIn(), Bf(_Mem()), BfC(_BfC), BfL(_Mem.Len()){}

int TMemIn::GetBf(const void* LBf, const TSize& LBfL){
  Assert(TSize(BfC+LBfL)<=TSize(BfL));
  int LBfS=0;
  for (TSize LBfC=0; LBfC<LBfL; LBfC++){
    LBfS+=(((char*)LBf)[LBfC]=Bf[BfC++]);}
  return LBfS;
}

bool TMemIn::GetNextLnBf(TChA& LnChA){
  // not implemented
  FailR(TStr::Fmt("TMemIn::GetNextLnBf: not implemented").CStr());
  return false;
}

TStr TMemIn::GetSNm() const {
  return "Input-Memory";
}

/////////////////////////////////////////////////
// Output-Memory
TRefMemOut::TRefMemOut(TMem& _Mem): TSBase(),
    TSOut(), Mem(_Mem){}

int TRefMemOut::PutBf(const void* LBf, const TSize& LBfL){
  int LBfS=0;
  for (TSize LBfC=0; LBfC<LBfL; LBfC++){
    char Ch=((char*)LBf)[LBfC];
    LBfS+=Ch; Mem+=Ch;
  }
  return LBfS;
}

TStr TRefMemOut::GetSNm() const {
  return "Output-Reference-Memory";
}

/////////////////////////////////////////////////
// Char-Array
void TChA::Resize(const int& _MxBfL){
  // Get max vale for MxBfL
  const auto Max = TInt::Mx - 1;
  // Make sure we got valid _MxBfL
  IAssert(0 <= _MxBfL && _MxBfL < Max);
  // Check if there is something to do
  if (_MxBfL <= MxBfL) return;
  // Check if _MxBfL at least twice larger then MxBf
  // We devide to avoid overflow when multiplying with 2
  if (_MxBfL / 2 >= MxBfL) { MxBfL = _MxBfL; }
  // if not we try to dobule the size. In case we go under 0,
  // which indicates overflow, we resort to max size
  else { MxBfL *= 2; if (MxBfL < 0 || MxBfL > Max) MxBfL = Max; }
  // Reserve new buffer
  char* NewBf=new char[MxBfL+1]; IAssert(NewBf!=NULL);
  // Copy old contents ...
  strcpy(NewBf, Bf);
  // and delete old buffer
  delete[] Bf; Bf=NewBf;
}

TChA::TChA(const TStr& Str){
  Bf=new char[(MxBfL=BfL=Str.Len())+1];
  strcpy(Bf, Str.CStr());
}

void TChA::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveHdArg(Nm, "BfL", TInt::GetStr(BfL));
  SOut.PutStr(TXmlLx::GetXmlStrFromPlainStr(*this));
}

TChA& TChA::operator=(const TChA& ChA){
  if (this!=&ChA){
    if (ChA.BfL>MxBfL){delete[] Bf; Bf=new char[(MxBfL=ChA.BfL)+1];}
    BfL=ChA.BfL; strcpy(Bf, ChA.CStr());
  }
  return *this;
}

TChA& TChA::operator=(const TStr& Str){
  if (Str.Len()>MxBfL){delete[] Bf; Bf=new char[(MxBfL=Str.Len())+1];}
  BfL=Str.Len(); strcpy(Bf, Str.CStr());
  return *this;
}

TChA& TChA::operator=(const char* CStr){
  int CStrLen=int(strlen(CStr));
  if (CStrLen>MxBfL){delete[] Bf; Bf=new char[(MxBfL=CStrLen)+1];}
  BfL=CStrLen; strcpy(Bf, CStr);
  return *this;
}

TChA& TChA::operator+=(const TMem& Mem) {
  Resize(BfL+Mem.Len());
  strcpy(Bf+BfL, Mem.GetBf()); BfL+=Mem.Len(); return *this;
}

TChA& TChA::operator+=(const TChA& ChA){
  Resize(BfL+ChA.Len());
  strcpy(Bf+BfL, ChA.CStr()); BfL+=ChA.Len(); return *this;
}

TChA& TChA::operator+=(const TStr& Str){
  Resize(BfL+Str.Len());
  strcpy(Bf+BfL, Str.CStr()); BfL+=Str.Len(); return *this;
}

TChA& TChA::operator+=(const char* CStr){
  int CStrLen=(int)strlen(CStr); Resize(BfL+CStrLen);
  strcpy(Bf+BfL, CStr); BfL+=CStrLen; return *this;
}

void TChA::Ins(const int& BChN, const char* CStr){
  Assert((0<=BChN)&&(BChN<=BfL)); //** ali je <= v (BChN<=BfL) upravicen?
  int CStrLen=int(strlen(CStr)); Resize(BfL+CStrLen);
  memmove(Bf+BChN+CStrLen, Bf+BChN, BfL-BChN+1);
  memmove(Bf+BChN, CStr, CStrLen); BfL+=CStrLen;
}

void TChA::Del(const int& ChN){
  Assert((0<=ChN)&&(ChN<BfL));
  memmove(Bf+ChN, Bf+ChN+1, BfL-ChN);
  BfL--;
}

void TChA::Trunc(){
  int BChN=0; while ((BChN<BfL)&&(GetCh(BChN)<=' ')){BChN++;}
  int EChN=BfL-1; while ((0<=EChN)&&(GetCh(EChN)<=' ')){EChN--;}
  if (BChN<=EChN){
    for (int ChN=BChN; ChN<=EChN; ChN++){
      PutCh(ChN-BChN, GetCh(ChN));}
    Trunc(EChN-BChN+1);
  } else {
    Clr();
  }
/*  int BChN=0; while ((BChN<BfL)&&(Bf[BChN]<=' ')){BChN++;}
  int EChN=BfL-1; while ((0<=EChN)&&(Bf[EChN]<=' ')){EChN--;}
  if (BChN<=EChN){
    for (int ChN=BChN; ChN<=EChN; ChN++){Bf[ChN-BChN]=Bf[ChN];}
    Bf[BfL=EChN+1]=0;
  } else {
    Clr();
  }*/
}

void TChA::Reverse(){
  for (int ChN=0; ChN<BfL/2; ChN++){
    char Ch=Bf[ChN];
    Bf[ChN]=Bf[BfL-ChN-1];
    Bf[BfL-ChN-1]=Ch;
  }
}

TChA TChA::GetSubStr(const int& _BChN, const int& _EChN) const {
  int BChN=TInt::GetMx(_BChN, 0);
  int EChN=TInt::GetMn(_EChN, Len()-1);
  int Chs=EChN-BChN+1;
  if (Chs<=0){return TStr();}
  else if (Chs==Len()){return *this;}
  else {
    //char* Bf=new char[Chs+1]; strncpy(Bf, CStr()+BChN, Chs); Bf[Chs]=0;
    //TStr Str(Bf); delete[] Bf;
    //return Str;
    return TChA(CStr()+BChN, Chs);
  }
}

int TChA::CountCh(const char& Ch, const int& BChN) const {
  int ChN=TInt::GetMx(BChN, 0);
  const int ThisLen=Len();
  int Cnt = 0;
  while (ChN<ThisLen){if (Bf[ChN]==Ch){ Cnt++;} ChN++;}
  return Cnt;
}

int TChA::SearchCh(const char& Ch, const int& BChN) const {
  int ChN=TInt::GetMx(BChN, 0);
  const int ThisLen=Len();
  while (ChN<ThisLen){if (Bf[ChN]==Ch){return ChN;} ChN++;}
  return -1;
}

int TChA::SearchChBack(const char& Ch, int BChN) const {
  if (BChN >= Len() || BChN < 0) { BChN = Len()-1; }
  for (int i = BChN; i >= 0; i--) {
    if (GetCh(i) == Ch) { return i; }
  }
  return -1;
}

int TChA::SearchStr(const TChA& Str, const int& BChN) const {
  return SearchStr(Str.CStr(), BChN);
}

int TChA::SearchStr(const TStr& Str, const int& BChN) const {
  return SearchStr(Str.CStr(), BChN);
}

int TChA::SearchStr(const char* CStr, const int& BChN) const {
  const char* BegPos=strstr(Bf+BChN, CStr);
  if (BegPos==NULL){return -1;}
  else {return int(BegPos-Bf);}
}

bool TChA::IsPrefix(const char* CStr, const int& BChN) const {
  if (BChN+(int)strlen(CStr)>Len()){return false;}
  const char* B = Bf+BChN;
  const char* C = CStr;
  while (*C!=0 && *B==*C) {
    B++; C++;
  }
  if (*C==0){return true;}
  else {return false;}
}

bool TChA::IsPrefix(const TStr& Str) const {
  return IsPrefix(Str.CStr());
}

bool TChA::IsPrefix(const TChA& Str) const {
  return IsPrefix(Str.CStr());
}

bool TChA::IsPrefixLc(const char* CStr, const int& BChN) const {
  if (BChN+(int)strlen(CStr)>Len()){return false;}
  const char* B = Bf+BChN;
  const char* C = CStr;
  while (*C!=0 && tolower(*B)==tolower(*C) ) {
    B++; C++;
  }
  if (*C==0){return true;}
  else {return false;}
}

bool TChA::IsPrefixLc(const TStr& Str) const {
  return IsPrefixLc(Str.CStr());
}

bool TChA::IsPrefixLc(const TChA& Str) const {
  return IsPrefixLc(Str.CStr());
}

bool TChA::IsSuffix(const char* CStr) const {
  if ((int)strlen(CStr) > Len()) { return false; }
  const char* E = Bf+Len()-1;
  const char* C = CStr+strlen(CStr)-1;
  while (C >= CStr && *E==*C) {
    E--;  C--;
  }
  if (C+1 == CStr) { return true; }
  else { return false; }
}

bool TChA::IsSuffix(const TStr& Str) const {
  return IsSuffix(Str.CStr());
}

bool TChA::IsSuffix(const TChA& Str) const {
  return IsSuffix(Str.CStr());
}

void TChA::ChangeCh(const char& SrcCh, const char& DstCh){
  int StrLen=Len();
  for (int ChN=0; ChN<StrLen; ChN++){if (Bf[ChN]==SrcCh){Bf[ChN]=DstCh;}}
}

/*void TChA::ToUc(){
  int StrLen=Len();
  for (int ChN=0; ChN<StrLen; ChN++){Bf[ChN]=(char)toupper(Bf[ChN]);}
}

void TChA::ToLc(){
  int StrLen=Len();
  for (int ChN=0; ChN<StrLen; ChN++){Bf[ChN]=(char)tolower(Bf[ChN]);}
}*/

TChA& TChA::ToLc() {
  char *c = Bf;
  while (*c) {
    *c = (char) tolower(*c);  c++;
  }
  return *this;
}

TChA& TChA::ToUc() {
  char *c = Bf;
  while (*c) {
    *c = (char) toupper(*c); c++;
  }
  return *this;
}

TChA& TChA::ToTrunc(){
  int StrLen=Len(); int BChN=0; int EChN=StrLen-1;
  while ((BChN<StrLen)&&TCh::IsWs(GetCh(BChN))){BChN++;}
  while ((EChN>=0)&&TCh::IsWs(GetCh(EChN))){EChN--;}
  if ((BChN!=0)||(EChN!=StrLen-1)){
    int DstChN=0;
    for (int SrcChN=BChN; SrcChN<=EChN; SrcChN++){
      PutCh(DstChN, GetCh(SrcChN)); DstChN++;}
    Trunc(DstChN);
  }
  return *this;
}

void TChA::CompressWs(){
  int StrLen=Len(); int SrcChN=0; int DstChN=0;
  while ((SrcChN<StrLen)&&TCh::IsWs(GetCh(SrcChN))){SrcChN++;}
  while (SrcChN<StrLen){
    if ((TCh::IsWs(GetCh(SrcChN)))&&(DstChN>0)&&(TCh::IsWs(GetCh(DstChN-1)))){
      SrcChN++;
    } else {
      PutCh(DstChN, GetCh(SrcChN)); SrcChN++; DstChN++;
    }
  }
  if ((DstChN>0)&&(TCh::IsWs(GetCh(DstChN-1)))){DstChN--;}
  Trunc(DstChN);
}

void TChA::Swap(const int& ChN1, const int& ChN2){
  char Ch=GetCh(ChN1);
  PutCh(ChN1, GetCh(ChN2));
  PutCh(ChN2, Ch);
}

void TChA::Swap(TChA& ChA) {
  ::Swap(MxBfL, ChA.MxBfL);
  ::Swap(BfL, ChA.BfL);
  ::Swap(Bf, ChA.Bf);
}

int TChA::GetPrimHashCd() const {
  return TStrHashF_DJB::GetPrimHashCd(CStr());
}

int TChA::GetSecHashCd() const {
  return TStrHashF_DJB::GetSecHashCd(CStr());
}


void TChA::LoadTxt(const PSIn& SIn, TChA& ChA){
  delete[] ChA.Bf;
  ChA.Bf=new char[(ChA.MxBfL=ChA.BfL=SIn->Len())+1];
  SIn->GetBf(ChA.CStr(), SIn->Len()); ChA.Bf[ChA.BfL]=0;
}

void TChA::SaveTxt(const PSOut& SOut) const {
  SOut->SaveBf(CStr(), Len());
}

/*TChA operator+(const TChA& LStr, const TChA& RStr){
  return LStr+=RStr; }
}

TChA operator+(const TChA& LStr, const TStr& RStr){
  return LStr+=RStr.CStr();
}

TChA operator+(const TStr& LStr, const char* RCStr){
  return LStr+=RCStr;
}*/


/////////////////////////////////////////////////
// Input-Char-Array
TChAIn::TChAIn(const TChA& ChA, const int& _BfC):
  TSBase(), TSIn(), Bf(ChA.CStr()), BfC(_BfC), BfL(ChA.Len()){}

int TChAIn::GetBf(const void* LBf, const TSize& LBfL){
  Assert(TSize(BfC+LBfL)<=TSize(BfL));
  int LBfS=0;
  for (TSize LBfC=0; LBfC<LBfL; LBfC++){
    LBfS+=(((char*)LBf)[LBfC]=Bf[BfC++]);}
  return LBfS;
}

bool TChAIn::GetNextLnBf(TChA& LnChA){
  // not implemented
  FailR(TStr::Fmt("TChAIn::GetNextLnBf: not implemented").CStr());
  return false;
}

TStr TChAIn::GetSNm() const {
  return "Input-Char-Array";
}

/////////////////////////////////////////////////
// String
const char TStr::EmptyStr = 0;

TStr TStr::WrapCStr(char* _CStr) {
	TStr NewStr;

	if (_CStr != nullptr && _CStr[0] == 0) {
		delete[] _CStr;
	} else if (_CStr != nullptr) {
		NewStr.Inner = _CStr;
	}

	return NewStr;
}

TStr::TStr(const char* _CStr): Inner(nullptr) {
	if (_CStr == nullptr) { return; }

	const int Len = (int)strlen(_CStr);
	if (Len > 0) {
		Inner = new char[Len+1];
		strcpy(Inner, _CStr);
	}
}

TStr::TStr(const char& Ch): Inner(nullptr) {
    if (Ch != 0) {
        Inner = new char[2];
        Inner[0] = Ch; Inner[1] = 0;
    }
}

TStr::TStr(const TStr& Str): Inner(nullptr) {
	if (!Str.Empty()) {
		Inner = Str.CloneCStr();
	}
}

TStr::TStr(TStr&& Str): Inner(nullptr) {
    Inner = Str.Inner;
    // reset other
    Str.Inner=nullptr;
}

TStr::TStr(const TChA& ChA): Inner(nullptr) {
    if (!ChA.Empty()) {
        Inner = new char[ChA.Len()+1];
        strcpy(Inner, ChA.CStr());
    }
}

TStr::TStr(const TMem& Mem): Inner(nullptr) {
    if (!Mem.Empty()) {
		const int Len = Mem.Len();

        Inner = new char[Len + 1];
        memcpy(Inner, Mem(), Len);
		Inner[Len] = 0;
    }
}

TStr::TStr(const TSStr& SStr): Inner(nullptr) {
	if (!SStr.Empty()) {
		Inner = new char[SStr.Len()+1];
		strcpy(Inner, SStr.CStr());
	}
}

TStr::TStr(const PSIn& SIn): Inner(nullptr) {
	const int SInLen = SIn->Len();
	if (SInLen > 0) {
		Inner = new char[SInLen + 1];
		SIn->GetBf(Inner, SInLen);
		Inner[SInLen] = 0;
	}
}

TStr::TStr(TSIn& SIn, const bool& IsSmall): Inner(nullptr) {
	if (IsSmall) {
		if (SIn.PeekCh() == 0) {
            EAssert(SIn.GetCh() == 0);
        } else {
			SIn.Load(Inner);
		}
	} else {
		int BfL; SIn.Load(BfL);
		if (BfL == 0) {
            EAssert(SIn.GetCh() == 0);
        } else {
			SIn.Load(Inner, BfL, BfL);
		}
	}
    // delete buffer and replace with null in case it's ""
    if (Inner != nullptr && Inner[0] == 0) {
		delete[] Inner;
        Inner = nullptr;
    }
}

void TStr::Load(TSIn& SIn, const bool& IsSmall) {
    *this = TStr(SIn, IsSmall);
}

void TStr::Save(TSOut& SOut, const bool& IsSmall) const {
    if (IsSmall){ SOut.Save(CStr()); }
    else { const int BfL = Len(); SOut.Save(BfL); SOut.Save(CStr(), BfL); }
}

void TStr::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  TStr TokStr=XmlTok->GetTokStr(false);
  operator=(TokStr);
}

void TStr::SaveXml(TSOut& SOut, const TStr& Nm) const {
  TStr XmlStr=TXmlLx::GetXmlStrFromPlainStr(*this);
  if (XmlStr.Empty()){XSaveBETag(Nm);}
  else {XSaveHd(Nm); SOut.PutStr(XmlStr);}
}

TStr& TStr::operator=(const TStr& Str) {
	if (this != &Str) {
		Clr();
		if (!Str.Empty()) {
			Inner = Str.CloneCStr();
		}
	}
    return *this;
}

TStr& TStr::operator=(TStr&& Str) {
	if (this != &Str) {
		std::swap(Inner, Str.Inner);
		Str.Clr();
	}
    return *this;
}

TStr& TStr::operator=(const TChA& ChA) {
	Clr();
	if (!ChA.Empty()) {
		Inner = new char[ChA.Len() + 1];
		strcpy(Inner, ChA.CStr());
	}
    return *this;
}

TStr& TStr::operator=(const char* CStr) {
	Clr();
	const int StrLen = (int)strlen(CStr);
	if (StrLen > 0) {
		Inner = new char[StrLen+1];
		strcpy(Inner, CStr);
	}
	return *this;
}

bool TStr::operator==(const char* _CStr) const {
	if (_CStr == nullptr) { return false; }
	return (CStr() == _CStr) || (strcmp(CStr(), _CStr) == 0);
}

bool TStr::operator<(const TStr& Str) const {
    return strcmp(CStr(), Str.CStr()) < 0;
}

char& TStr::operator[](const int& ChN) {
	Assert( (0 <= ChN) && (ChN < Len()) );
	return Inner[ChN];
}

void TStr::PutCh(const int& ChN, const char& Ch) {
    Assert((0<=ChN)&&(ChN<Len()));
    Inner[ChN] = Ch;
}

char TStr::GetCh(const int& ChN) const {
    // Assert index not negative, index not >= Length
    Assert( (0 <= ChN) && (ChN < Len()) );
    return Inner[ChN];
}

char* TStr::CloneCStr() const {
	const int Length = Len();
	char* Bf = new char[Length+1];
	if (Length > 0) {
		strcpy(Bf, Inner);
	} else {
		Bf[0] = 0;
	}
	return Bf;
}

bool TStr::Empty() const {
	 AssertR(Inner == nullptr || Inner[0] != 0, "TStr::Empty string is not nullptr. Fix immediately!");
	 return  Inner == nullptr;
}

void TStr::Clr() {
	if (Inner != nullptr) {
		delete[] Inner;
		Inner = nullptr;
	}
}

int TStr::GetMemUsed() const {
    return int(sizeof(TStr) + (Empty() ? 0 : (Len() + 1)));
}

int TStr::CmpI(const char* p, const char* r) {
	if (!p){ return r ? (*r ? -1 : 0) : 0; }
	if (!r){ return (*p ? 1 : 0); }
	while (*p && *r){
		int i = int(toupper(*p++)) - int(toupper(*r++));
		if (i != 0){ return i; }
	}
	return int(toupper(*p++)) - int(toupper(*r++));
}

bool TStr::IsUc() const {
	const int StrLen = Len();
	for (int ChN = 0; ChN<StrLen; ChN++){
		if (('a' <= Inner[ChN]) && (Inner[ChN] <= 'z')){ return false; }
	}
	return true;
}

TStr& TStr::ToUc() {
	const int StrLen = Len();
	for (int ChN = 0; ChN<StrLen; ChN++){
        Inner[ChN] = toupper(Inner[ChN]);
	}
	return *this;
}

TStr TStr::GetUc() const {
    return TStr(*this).ToUc();
}

bool TStr::IsLc() const {
	const int StrLen = Len();
	for (int ChN = 0; ChN<StrLen; ChN++){
		if (('A' <= Inner[ChN]) && (Inner[ChN] <= 'Z')){ return false; }
	}
	return true;
}

TStr& TStr::ToLc() {
	const int StrLen = Len();
	for (int ChN = 0; ChN<StrLen; ChN++){
        Inner[ChN] = tolower(Inner[ChN]);
	}
	return *this;
}

TStr TStr::GetLc() const {
    return TStr(*this).ToLc();
}

TStr& TStr::ToCap() {
	if (Empty()) { return *this; }
	const int StrLen = Len();
	// copy first char in uppercase
	Inner[0] = (char)toupper(Inner[0]);
	// copy all other chars in lowercase
	for (int ChN = 1; ChN < StrLen; ChN++){
		Inner[ChN] = (char)tolower(Inner[ChN]);
	}
	return *this;
}

TStr TStr::GetCap() const{
	return TStr(*this).ToCap();
}

TStr& TStr::ToTrunc() {
    *this = GetTrunc();
    return *this;
}

TStr TStr::GetTrunc() const {
  int ThisLen = Len();

  int BChN = 0;
  int EChN = ThisLen - 1;

  while ((BChN < ThisLen) && TCh::IsWs(GetCh(BChN))) { BChN++; }
  while ((EChN >= 0) && TCh::IsWs(GetCh(EChN))){ EChN--; }
  if (BChN > EChN) { return TStr(); }
  return GetSubStr(BChN, EChN);
}

TStr& TStr::ToHex() {
    *this = GetHex();
    return *this;
}

TStr TStr::GetHex() const {
  TChA ChA;
  int StrLen=Len();
  for (int ChN=0; ChN<StrLen; ChN++){
    uchar Ch=uchar(GetCh(ChN));
    char MshCh=TCh::GetHexCh((Ch/16)%16);
    char LshCh=TCh::GetHexCh(Ch%16);
    ChA+=MshCh; ChA+=LshCh;
  }
  return TStr(ChA);
}


TStr& TStr::FromHex() {
    *this = GetFromHex();
    return *this;
}

TStr TStr::GetFromHex() const {
  int StrLen=Len(); IAssert(StrLen%2==0);
  TChA ChA; int ChN=0;
  while (ChN<StrLen){
	char MshCh = GetCh(ChN); ChN++;
	char LshCh = GetCh(ChN); ChN++;
    uchar Ch=uchar(TCh::GetHex(MshCh)*16+TCh::GetHex(LshCh));
	EAssertR(Ch != 0, "TStr::GetFromHex null terminator found! ");
    ChA+=Ch;
  }
  return TStr(ChA);
}

TStr TStr::GetSubStr(const int& BChN, const int& EChN) const {
	int StrLen = Len();
	EAssertR(0 <= BChN && BChN <= EChN && EChN < StrLen, "TStr::GetSubStr index out of bounds");
	int Chs = EChN - BChN + 1;
	// initialize accordingly
	char* Bf = nullptr;
	if (Chs <= 0) {
		// create empty string
		return TStr();
	}
	else if (Chs == StrLen) {
		// keep copy of everything
		Bf = CloneCStr();//
	}
	else {
		// get copy of a substring
		Bf = new char[Chs + 1]; strncpy(Bf, CStr() + BChN, Chs); Bf[Chs] = 0;
	}
	return WrapCStr(Bf);
}

// safe version of GetSubStr().
// Fixes BChN and EChN values that are outside of string's range
// supports also negative indices (python like):
// GetSubStrSafe(0,-1) will return all but last char
TStr TStr::GetSubStrSafe(const int& BChN, const int& EChN) const {
	int StrLen = Len();
	int StartN;
	if (BChN <= -StrLen)
		StartN = 0;
	else if (BChN < 0)
		StartN = StrLen + BChN;
	else
		StartN = BChN;
	int EndN;
	if (EChN < 0)
		EndN = StrLen + EChN - 1;
	else
		EndN = EChN >= StrLen ? StrLen - 1 : EChN;

	if (!(0 <= StartN && StartN <= EndN && EndN < StrLen))
		return TStr();
	return GetSubStr(StartN, EndN);
}

void TStr::InsStr(const int& BChN, const TStr& Str) {
    const int ThisLen = Len();
    EAssert((0<=BChN)&&(BChN<=ThisLen));
    TChA ChA = TChA(ThisLen + Str.Len());
    if (BChN==0){
        ChA += Str;
        ChA += CStr();
    } else if (BChN==ThisLen){
        ChA += CStr();
        ChA += Str;
    } else {
        ChA += GetSubStr(0, BChN-1);
        ChA += Str;
        ChA += GetSubStr(BChN, ThisLen-1);
    }
    *this = ChA;
}

void TStr::DelChAll(const char& DelCh) {
    const int ThisLen = Len();
    TChA ChA = TChA(ThisLen);
    for (int ChN = 0; ChN < ThisLen; ChN++) {
        const char Ch = GetCh(ChN);
        if (Ch != DelCh) { ChA += Ch; }
    }
    *this = ChA;
}

void TStr::DelSubStr(const int& BChN, const int& EChN) {
	int StrLen = Len();
	EAssertR(0 <= BChN && BChN <= EChN && EChN < StrLen, "TStr::DelSubStr index out of bounds");

    int Chs = Len() - (EChN - BChN + 1);
    if (Chs == 0) {
        // nothing left after delete, clear it all
        Clr();
    } else if (Chs < Len()) {
        // actual substring
        char* Bf=new char[Chs + 1];
        // copy before
        strncpy(Bf, CStr(), BChN);
        // copy after
        strncpy(Bf+BChN, CStr()+EChN+1, Len()-EChN-1);
        // we are done, just replace current
        Bf[Chs]=0;
        *this = WrapCStr(Bf);
    }
}

bool TStr::DelStr(const TStr& Str) {
	int ChN = SearchStr(Str);
	if (ChN != -1){
		DelSubStr(ChN, ChN + Str.Len() - 1);
        return true;
	}
    return false;
}

int TStr::DelStrAll(const TStr& Str) {
	return ChangeStrAll(Str, "");
}

TStr TStr::LeftOf(const char& SplitCh) const {
  int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=0;
  while ((ChN<ThisLen)&&(ThisBf[ChN]!=SplitCh)){ChN++;}
  return (ChN == 0 || ChN==ThisLen) ? "" : GetSubStr(0, ChN-1);
}

TStr TStr::LeftOfLast(const char& SplitCh) const {
  const char* ThisBf=CStr();
  int ChN=Len()-1;
  while ((ChN>=0)&&(ThisBf[ChN]!=SplitCh)){ChN--;}
  return (ChN == 0 || ChN == -1) ? "" : GetSubStr(0, ChN - 1);
}

TStr TStr::RightOf(const char& SplitCh) const {
  int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=0;
  while ((ChN<ThisLen)&&(ThisBf[ChN]!=SplitCh)){ChN++;}
  return (ChN>=ThisLen - 1) ? "" : GetSubStr(ChN+1, ThisLen-1);
}

TStr TStr::RightOfLast(const char& SplitCh) const {
  int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=Len()-1;
  while ((ChN>=0)&&(ThisBf[ChN]!=SplitCh)){ChN--;}
  return (ChN == ThisLen -1 || ChN==-1) ? "" : GetSubStr(ChN+1, ThisLen-1);
}

void TStr::SplitLeftOfRightOf(TStr& LStr, const int& LeftOfChN, const int& RightOfChN, TStr& RStr) const {
	const int StrLen = Len();

	EAssertR(LeftOfChN >= 0 && LeftOfChN <= RightOfChN && RightOfChN < StrLen, "Splitting index should be greater than 0 and less than length!");

	// clear the left and right strings
	LStr.Clr();
	RStr.Clr();

	if (Empty()) { return; }

	const char* InnerPt = CStr();

	const int LeftLen = LeftOfChN;
	const int RightLen = StrLen - RightOfChN - 1;

	// copy into left and right
	// if the length of any of the strings is 0 than leave it empty
	if (LeftLen > 0) {
		LStr.Inner = new char[LeftLen + 1];
		memcpy(LStr.Inner, InnerPt, LeftLen);
		LStr.Inner[LeftLen] = 0;
	}
	if (RightLen > 0) {
		RStr.Inner = new char[RightLen + 1];
		memcpy(RStr.Inner, InnerPt + RightOfChN + 1, RightLen);
		RStr.Inner[RightLen] = 0;
	}
}

void TStr::SplitOnChN(TStr& LStr, const int& ChN, TStr& RStr) const {
	SplitLeftOfRightOf(LStr, ChN, ChN, RStr);
}

void TStr::SplitOnCh(TStr& LStr, const char& SplitCh, TStr& RStr) const {
	LStr.Clr();
	RStr.Clr();

	// check if the string is empty
	if (Empty()) { return; }

	const char* InnerPt = CStr();

	// find the pointer to the delimiter
	const char* ChPtr = strchr(InnerPt, SplitCh);

	// if the character was not found than return this in the left string
	if (ChPtr == nullptr) {
		LStr = *this;
		return;
	}

	// split
	SplitOnChN(LStr, (int)(ChPtr - InnerPt), RStr);
}

void TStr::SplitOnStr(TStr& LStr, const TStr& SplitStr, TStr& RStr) const {
	LStr.Clr();
	RStr.Clr();

	// check if the string is empty
	if (Empty()) { return; }

	const char* InnerPt = CStr();

	const char* MatchPt = strstr(InnerPt, SplitStr.CStr());

	if (MatchPt == nullptr || SplitStr.Empty()) {
		LStr = *this;
		return;
	}

	const int MatchIdx = (int)(MatchPt - InnerPt);

	SplitLeftOfRightOf(LStr, MatchIdx, MatchIdx + SplitStr.Len() - 1, RStr);
}

void TStr::SplitOnLastCh(TStr& LStr, const char& SplitCh, TStr& RStr) const {
	LStr.Clr();
	RStr.Clr();
	// check if the string is empty
	if (Empty()) { return; }

	const char* InnerPt = CStr();

	// find the pointer to the delimiter
	const char* ChPtr = strrchr(InnerPt, SplitCh);

	// if the character was not found than return this in the right string
	if (ChPtr == nullptr) {
		RStr = *this;
		return;
	}

	// split
	SplitOnChN(LStr, (int)(ChPtr - InnerPt), RStr);
}

void TStr::SplitOnAllCh(const char& SplitCh, TStrV& StrV, const bool& SkipEmpty) const {
  StrV.Clr();
  char* Bf=new char[Len()+1];
  strcpy(Bf, CStr());
  char* CurStrBf=Bf;
  forever{
    char* BfC=CurStrBf;
    while ((*BfC!=0)&&(*BfC!=SplitCh)){BfC++;}
    bool IsEnd=(*BfC=='\0');
    *BfC=0;
    if ((BfC>CurStrBf)||(!SkipEmpty)){StrV.Add(TStr(CurStrBf));}
    if (IsEnd){break;}
    CurStrBf=BfC+1;
  }
  delete[] Bf;
}

void TStr::SplitOnAllAnyCh(
 const TStr& SplitChStr, TStrV& StrV, const bool& SkipEmpty) const {
  // reset string output-vector
  StrV.Clr();
  // prepare working-copy of string
  char* Bf=new char[Len()+1];
  strcpy(Bf, CStr());
  char* CurStrBf=Bf; // pointer to current string
  // prepare pointer to split-char-string
  const char* SplitChBf=SplitChStr.CStr();
  forever{
    char* BfC=CurStrBf; // set the counter for working-string
    while (*BfC!=0){
      const char* SplitChBfC=SplitChBf; // set counter for split-char-string
      while ((*SplitChBfC!=0)&&(*SplitChBfC!=*BfC)){SplitChBfC++;}
      if (*SplitChBfC!=0){break;} // if split-char found
      BfC++;
    }
    bool IsEnd=(*BfC==0);
    *BfC=0;
    if ((BfC>CurStrBf)||(!SkipEmpty)){StrV.Add(TStr(CurStrBf));}
    if (IsEnd){break;}
    CurStrBf=BfC+1;
  }
  // delete working-copy
  delete[] Bf;
}

void TStr::SplitOnWs(TStrV& StrV) const {
  StrV.Clr();
  char* Bf=new char[Len()+1];
  strcpy(Bf, CStr());
  char* StrBf=Bf;
  forever{
    while ((*StrBf!=0)&&(TCh::IsWs(*StrBf))){StrBf++;}
    char* BfC=StrBf;
    while ((*BfC!=0)&&(!TCh::IsWs(*BfC))){BfC++;}
    bool IsEnd=(*BfC=='\0');
    *BfC=0;
    if (BfC>StrBf){StrV.Add(TStr(StrBf));}
    if (IsEnd){break;}
    StrBf=BfC+1;
  }
  delete[] Bf;
}

void TStr::SplitOnNonAlNum(TStrV& StrV) const {
  StrV.Clr();
  char* Bf=new char[Len()+1];
  strcpy(Bf, CStr());
  char* StrBf=Bf;
  forever{
    while ((*StrBf!=0)&&(!TCh::IsAlNum(*StrBf))){StrBf++;}
    char* BfC=StrBf;
    while ((*BfC!=0)&&(TCh::IsAlNum(*BfC))){BfC++;}
    bool IsEnd=(*BfC=='\0');
    *BfC=0;
    if (BfC>StrBf){StrV.Add(TStr(StrBf));}
    if (IsEnd){break;}
    StrBf=BfC+1;
  }
  delete[] Bf;
}

void TStr::SplitOnStr(const TStr& SplitStr, TStrV& StrV) const {
  StrV.Clr();
  int SplitStrLen=SplitStr.Len();
  int PrevChN=0; int ChN=0;
  int StrLen = Len();
  // on empty string we need to end here, otherwise the GetSubStr crashes
  if (StrLen == 0) { return; }
  while (ChN < StrLen && (ChN = SearchStr(SplitStr, ChN)) != -1){
    // extract & add string
	  if (ChN - 1 >= 0 && ChN - 1 < StrLen) {
		  TStr SubStr = GetSubStr(PrevChN, ChN - 1);
		  StrV.Add(SubStr);
	  }
	PrevChN = ChN;
	ChN += SplitStrLen;
  }
  // add last string
  TStr LastSubStr=GetSubStr(PrevChN, StrLen-1);
  StrV.Add(LastSubStr);
}

TStr TStr::Left(const int& EChN) const {
	if (EChN == 0) { return ""; }
	return EChN>=0 ? GetSubStr(0, EChN-1) : GetSubStr(0, Len()+EChN-1);
}

int TStr::CountCh(const char& Ch, const int& BChN) const {
	const int ThisLen = Len();
	if(ThisLen == 0) { return 0; }
	EAssertR(BChN >= 0 && BChN < ThisLen, "TStr::CountCh index BChN out of bounds!");

	const char* ThisBf = CStr();
	int Chs = 0;

	for (int ChN = BChN; ChN<ThisLen; ChN++) {
    if (ThisBf[ChN]==Ch) { Chs++; }
  }
  return Chs;
}

int TStr::SearchCh(const char& Ch, const int& BChN) const {
  int ThisLen = Len();
  if(ThisLen == 0) { return -1; }
  EAssertR(BChN >= 0 && BChN < ThisLen, "TStr::SearchCh index BChN out of bounds!");

  const char* ThisBf = CStr();
  int ChN = BChN;

  while (ChN < ThisLen){
    if (ThisBf[ChN] == Ch){ return ChN; }
    ChN++;
  }
  return -1;
}

int TStr::SearchChBack(const char& Ch, int BChN) const {
  const int StrLen = Len();
  if (StrLen == 0) { return 0; }
  const char* ThisBf = CStr();

  if (BChN < 0 || BChN >= StrLen) { BChN = StrLen - 1; }

  const char* Pt = ThisBf + BChN;

  while (Pt >= ThisBf) {
    if (*Pt == Ch){ return (int) (Pt - ThisBf); }
    Pt--;
  }
  return -1;
}

int TStr::SearchStr(const TStr& Str, const int& BChN) const {
  const int ThisLen = Len();
  EAssertR(BChN >= 0 && BChN <= ThisLen, "TStr::SearchStr: index BChN out of bounds!");

  // special case for handling empty strings
  if (ThisLen == 0) { return Str.Empty() ? 0 : -1; }

  const int NrBChN = BChN;
  const char* StrPt = strstr((const char*) CStr() + NrBChN, Str.CStr());
  if (StrPt == nullptr) { return -1; }
  return int(StrPt - CStr());
}

bool TStr::StartsWith(const char *Str) const {
	const int OtherLen = (int)strlen(Str);
	const int ThisLen = Len();
	if (OtherLen > ThisLen || ThisLen == 0) { return false; }

	int Cmp = strncmp(Str, CStr(), OtherLen);
	return Cmp == 0;
}

bool TStr::EndsWith(const char *Str) const {
	const int OtherLen = (int)strlen(Str);
	const int ThisLen = Len();
	if (OtherLen > ThisLen) {
		// too long to be a suffix anyway
		return false;
	} else {
		// move to the point in the buffer where we would expect the suffix to be
		const char *ending = CStr() + ThisLen - OtherLen;
		int cmp = strncmp(Str, ending, OtherLen);
		return cmp == 0;
	}
}

int TStr::ChangeCh(const char& SrcCh, const char& DstCh, const int& BChN) {
	AssertR(SrcCh != '\0' && DstCh != '\0', "TStr::ChangeCh: source and destination character should not be the terminator char!");
	int ChN = SearchCh(SrcCh, BChN);
	if (ChN != -1) { PutCh(ChN, DstCh); }
	return ChN;
}

int TStr::ChangeChAll(const char& SrcCh, const char& DstCh) {
	if (Empty()) { return 0; }
	int FirstChN = SearchCh(SrcCh);
	if (FirstChN == -1){
		return 0;
	} else {
		const int StrLen = Len(); int Changes = 0;
		for (int ChN = FirstChN; ChN < StrLen; ChN++){
            if (GetCh(ChN) == SrcCh) { PutCh(ChN, DstCh); Changes++; }
		}
		return Changes;
	}
}

int TStr::ChangeStr(const TStr& SrcStr, const TStr& DstStr, const int& BChN) {
	if (Empty() && BChN == 0) { return -1; }
	const int ChN = SearchStr(SrcStr, BChN);
	if (ChN != -1){
        DelSubStr(ChN, ChN + SrcStr.Len() - 1);
        InsStr(ChN, DstStr);
	}
    return ChN;
}

int TStr::ChangeStrAll(const TStr& SrcStr, const TStr& DstStr) {
	if (Empty() || SrcStr.Empty()) { return 0; }

	const int Length = Len();
	const int SrcLen = SrcStr.Len();
	const int DstLen = DstStr.Len();

	const char* InnerPt = CStr();
	const char* SrcCStr = SrcStr.CStr();
	const char* DstCStr = DstStr.CStr();

	// find how many times SrcStr appears in this string
	const char* CurrPos = Inner;
	const char* NextHit;

	int NMatches = 0;
	while ((NextHit = strstr(CurrPos, SrcCStr)) != nullptr) {
		NMatches++;
		CurrPos = NextHit + SrcLen;
	}

	// create a new string
	char* ResStr = new char[Length + NMatches*(DstLen - SrcLen) + 1];

	// iterate through the string, instead of copying source copy destination
	int i = 0;	// index in the source string
	int j = 0;	// index in the result string

	int SeqLen;

	// find next hit, copy everything between the current position and the hit,
	// then copy the dest string
	while ((NextHit = strstr(InnerPt + i, SrcCStr)) != nullptr) {
		SeqLen = (int)(NextHit - (InnerPt + i));
		// copy the chars in between the hits
		memcpy(ResStr + j, InnerPt + i, SeqLen);

		// increase positions
		i += SeqLen;
		j += SeqLen;

		// copy the dst string
		memcpy(ResStr + j, DstCStr, DstLen);

		// increase positions
		i += SrcLen;
		j += DstLen;
	}

	// copy what is after the last match
	memcpy(ResStr + j, InnerPt + i, Length - i);

	// insert null character
	ResStr[Length + NMatches*(DstLen - SrcLen)] = 0;
	// replace with the new string
	*this = WrapCStr(ResStr);
    // return number of changes
    return NMatches;
}

TStr TStr::Reverse() const {
	const int ThisLen = Len();
    // reserve place for reverse
	char* Reversed = new char[ThisLen+1];
    // do the reversing
    for (int ChN = 0; ChN < ThisLen; ChN++) {
		Reversed[ChN] = Inner[ThisLen - ChN - 1];
	}
    // finish
	Reversed[ThisLen] = 0;
    // create new string from it
	return WrapCStr(Reversed);
}

int TStr::GetPrimHashCd() const {
    return TStrHashF_DJB::GetPrimHashCd(CStr());
}

int TStr::GetSecHashCd() const {
    return TStrHashF_DJB::GetSecHashCd(CStr());
}

int TStr::GetHashTrick() const {
    return TStrHashF_Murmur3::GetPrimHashCd(CStr());
}

bool TStr::IsBool(bool& Val) const {
  if (operator==("T")){Val=true; return true;}
  else if (operator==("F")){Val=false; return true;}
  else {return false;}
}

bool TStr::IsInt(const bool& Check, const int& MnVal, const int& MxVal, int& Val) const {
	int64 _Val;
	// assign and check for overflow
	return IsInt64(Check, MnVal, MxVal, _Val) && (int64)(Val = (int)_Val) == _Val;
}

bool TStr::IsUInt(const bool& Check, const uint& MnVal, const uint& MxVal, uint& Val) const {
	TChRet Ch(TStrIn::New(*this, false));
	Ch.GetCh();
	return IsUInt(Ch, Check, MnVal, MxVal, Val);
}

bool TStr::IsHexInt( const bool& Check, const int& MnVal, const int& MxVal, int& Val) const {
  // parsing format {ws} [+/-][0x] +{XXX}
  int _Val=0;
  bool Minus=false;
  TChRet Ch(TStrIn::New(*this, false));
  while (TCh::IsWs(Ch.GetCh())){}
  if (Ch()=='+'){Minus=false; Ch.GetCh();}
  if (Ch()=='-'){Minus=true; Ch.GetCh();}
  if (Ch()=='0'){
    Ch.GetCh();
    if (tolower(Ch())=='x' ){
      Ch.GetCh(); if (Ch.Eof()){return false;}
    }
  }
  if (!Ch.Eof() && !TCh::IsHex(Ch())){return false;}
  if (!Ch.Eof()) _Val = TCh::GetHex(Ch());
  while (TCh::IsHex(Ch.GetCh())){_Val=16*_Val+TCh::GetHex(Ch());}
  if (Minus){_Val=-_Val;}
  if (Check&&((_Val<MnVal)||(_Val>MxVal))){return false;}
  if (Ch.Eof()){Val=_Val; return true;} else {return false;}
}

bool TStr::IsInt64(const bool& Check, const int64& MnVal, const int64& MxVal, int64& Val) const {
	EAssert(!Check || (Check && MnVal <= MxVal));
	// parsing format {ws} [+/-] +{ddd}
	TChRet Ch(TStrIn::New(*this, false));

	while (TCh::IsWs(Ch.GetCh())) {}

	bool IsInt;
	uint64 UIntVal;

	if (Ch() == '-') {
		Ch.GetCh();
		IsInt = IsUInt64(Ch, true, 0, 9223372036854775808ul, UIntVal);
		Val = -1l * UIntVal;
	} else {
		if (Ch() == '+') { Ch.GetCh(); }
		IsInt = IsUInt64(Ch, true, 0, 9223372036854775807ul, UIntVal);
		Val = UIntVal;
	}

	// check
	if (!IsInt) { return false; }
	if (Check && (Val < MnVal || Val > MxVal)) { return false; }

	return true;
}

bool TStr::IsUInt64(const bool& Check, const uint64& MnVal, const uint64& MxVal, uint64& Val) const {
	TChRet Ch(TStrIn::New(*this, false));
	Ch.GetCh();
	return IsUInt64(Ch, Check, MnVal, MxVal, Val);
}

bool TStr::IsHexInt64(
 const bool& Check, const int64& MnVal, const int64& MxVal, int64& Val) const {
  // parsing format {ws} [+/-][0x] +{XXX}
  int64 _Val=0;
  bool Minus=false;
  TChRet Ch(TStrIn::New(*this, false));
  while (TCh::IsWs(Ch.GetCh())){}
  if (Ch()=='+'){Minus=false; Ch.GetCh();}
  if (Ch()=='-'){Minus=true; Ch.GetCh();}
  if (Ch()=='0'){
    Ch.GetCh();
    if (tolower(Ch())=='x' ){
      Ch.GetCh(); if (Ch.Eof()){return false;}
    }
  }
  if (!Ch.Eof()) _Val=TCh::GetHex(Ch());
  while (TCh::IsHex(Ch.GetCh())){_Val=16*_Val+TCh::GetHex(Ch());}
  if (Minus){_Val=-_Val;}
  if (Check&&((_Val<MnVal)||(_Val>MxVal))){return false;}
  if (Ch.Eof()){Val=_Val; return true;} else {return false;}
}

bool TStr::IsFlt(const bool& Check, const double& MnVal, const double& MxVal,
 double& Val, const char& DecDelimCh) const {
  // parsing format {ws} [+/-] +{d} ([.]{d}) ([E|e] [+/-] +{d})
  TChRet Ch(TStrIn::New(*this, false));
  while (TCh::IsWs(Ch.GetCh())){}
  if ((Ch()=='+')||(Ch()=='-')){Ch.GetCh();}
  if (!TCh::IsNum(Ch())&&Ch()!=DecDelimCh){return false;}
  while (TCh::IsNum(Ch.GetCh())){}
  if (Ch()==DecDelimCh){
    Ch.GetCh();
    while (TCh::IsNum(Ch.GetCh())){}
  }
  if ((Ch()=='e')||(Ch()=='E')){
    Ch.GetCh();
    if ((Ch()=='+')||(Ch()=='-')){Ch.GetCh();}
    if (!TCh::IsNum(Ch())){return false;}
    while (TCh::IsNum(Ch.GetCh())){}
  }
  if (!Ch.Eof()){return false;}
  double _Val=atof(CStr());
  if (Check&&((_Val<MnVal)||(_Val>MxVal))){
    return false;
  } else {
    Val=_Val; return true;
  }
}

bool TStr::IsWord(const bool& WsPrefixP, const bool& FirstUcAllowedP) const {
  // parsing format {ws} (A-Z,a-z) *{A-Z,a-z,0-9}
  TChRet Ch(TStrIn::New(*this, false));
  if (WsPrefixP){while (TCh::IsWs(Ch.GetCh())){}}
  else {Ch.GetCh();}
  if (!TCh::IsAlpha(Ch())){return false;}
  else if (!FirstUcAllowedP&&(TCh::IsUc(Ch()))){return false;}
  while (TCh::IsAlNum(Ch.GetCh())){}
  if (!Ch.Eof()){return false;}
  return true;
}

bool TStr::IsWs() const {
  // if string is just a bunch of whitespace chars
  TChRet Ch(TStrIn::New(*this, false));
  while (TCh::IsWs(Ch.GetCh())){}
  return Ch.Eof();
}

bool TStr::IsWcMatch(
 const int& StrBChN, const TStr& WcStr, const int& WcStrBChN, TStrV& StarStrV,
 const char& StarCh, const char& QuestCh) const {
  const int StrLen=Len(), WcStrLen=WcStr.Len();
  int StrChN=StrBChN; int WcStrChN=WcStrBChN;
  while ((StrChN<StrLen)&&(WcStrChN<WcStrLen)){
    if ((WcStr[WcStrChN]==QuestCh)||(GetCh(StrChN)==WcStr[WcStrChN])){
      StrChN++; WcStrChN++;
    } else
    if (WcStr[WcStrChN]==StarCh){
      TChA StarChA; // string substituted by star character
      for (int AfterStrChN=StrChN; AfterStrChN<=StrLen; AfterStrChN++){
        if (AfterStrChN>StrChN){
          StarChA+=GetCh(AfterStrChN-1);}
        if (IsWcMatch(AfterStrChN, WcStr, WcStrChN+1, StarStrV, StarCh, QuestCh)){
          StarStrV.Add(StarChA); return true;
        }
      }
      return false;
    } else {
      return false;
    }
  }
  if (StrChN==StrLen){
    for (int AfterWcStrChN=WcStrChN; AfterWcStrChN<WcStrLen; AfterWcStrChN++){
      if (WcStr[AfterWcStrChN]!=StarCh){return false;}}
    return true;
  } else {
    return false;
  }
}

bool TStr::IsWcMatch(
 const TStr& WcStr, TStrV& StarStrV, const char& StarCh, const char& QuestCh) const {
  bool WcMatch=IsWcMatch(0, WcStr, 0, StarStrV, StarCh, QuestCh);
  if (WcMatch){
    StarStrV.Reverse();
    return true;
  } else {
    return false;
  }
}

bool TStr::IsWcMatch(
 const TStr& WcStr, const char& StarCh, const char& QuestCh) const {
  TStrV StarStrV;
  return IsWcMatch(0, WcStr, 0, StarStrV, StarCh, QuestCh);
}

bool TStr::IsWcMatch(const TStr& WcStr, const int& StarStrN, TStr& StarStr) const {
  TStrV StarStrV;
  if (IsWcMatch(WcStr, StarStrV)){
    if (StarStrV.Len()>StarStrN){
      StarStr=StarStrV[StarStrV.Len()-StarStrN-1];
    } else {
      StarStr="";
    }
    return true;
  } else {
    return false;
  }
}

bool TStr::IsWcMatch(const TStr& WcStr) const {
  TStrV StarStrV;
  return IsWcMatch(0, WcStr, 0, StarStrV);
}

TStr TStr::GetWcMatch(const TStr& WcStr, const int& StarStrN) const {
  TStrV StarStrV;
  if (IsWcMatch(WcStr, StarStrV)&&(StarStrV.Len()>=StarStrN)){
    IAssert(StarStrN>=0);
    return StarStrV[StarStrV.Len()-StarStrN-1];
  } else {
    return "";
  }
}

TStr TStr::GetFPath() const {
  const int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=ThisLen-1;
  while ((ChN>=0)&&(ThisBf[ChN]!='/')&&(ThisBf[ChN]!='\\')){ChN--;}
  return GetSubStr(0, ChN);
}

TStr TStr::GetFBase() const {
  const int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=ThisLen-1;
  while ((ChN>=0)&&(ThisBf[ChN]!='/')&&(ThisBf[ChN]!='\\')){ChN--;}
  if (ChN + 1 > ThisLen - 1) { return TStr(); }
  return GetSubStr(ChN+1, ThisLen - 1);
}

TStr TStr::GetFMid() const {
  int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=ThisLen-1;
  while ((ChN>=0)&&(ThisBf[ChN]!='/')&&(ThisBf[ChN]!='\\')&&(ThisBf[ChN]!='.')){
    ChN--;}
  if (ChN<0){
    return *this;
  } else {
    if (ThisBf[ChN]=='.'){
      int EChN= --ChN;
      while ((ChN>=0)&&(ThisBf[ChN]!='/')&&(ThisBf[ChN]!='\\')){ChN--;}
	  if (ChN + 1 > EChN) { return TStr(); }
      return GetSubStr(ChN+1, EChN);
    } else {
		if (ChN + 1 > ThisLen - 1) { return TStr(); }
      return GetSubStr(ChN+1, ThisLen - 1);
    }
  }
}

TStr TStr::GetFExt() const {
  int ThisLen=Len(); const char* ThisBf=CStr();
  int ChN=ThisLen-1;
  while ((ChN>=0)&&(ThisBf[ChN]!='/')&&(ThisBf[ChN]!='\\')&&
   (ThisBf[ChN]!='.')){ChN--;}
  if ((ChN>=0)&&(ThisBf[ChN]=='.')){
	  if (ChN > Len() - 1) { return TStr(); }
	  return GetSubStr(ChN, Len()-1);
  } else {return TStr();}
}

TStr TStr::GetNrFPath(const TStr& FPath){
  TChA NrFPath(FPath.Len()+4); NrFPath+=FPath;
  NrFPath.ChangeCh('\\', '/');
  if (NrFPath.Empty()){NrFPath="./";}
  if ((NrFPath.Len()>=2)&&isalpha(NrFPath[0])&&(NrFPath[1]==':')){
    if (NrFPath.Len()==2){NrFPath+="./";}
    if ((NrFPath[2]!='.')&&(NrFPath[2]!='/')){NrFPath.Ins(2, "./");}
    if (NrFPath[NrFPath.Len()-1]!='/'){NrFPath+="/";}
  } else {
    if ((NrFPath[0]!='.')&&(NrFPath[0]!='/')){NrFPath.Ins(0, "./");}
    if (NrFPath[NrFPath.Len()-1]!='/'){NrFPath+="/";}
  }
  return NrFPath;
}

TStr TStr::GetNrFMid(const TStr& FMid){
  TChA NrFMid;
  int FMidLen=FMid.Len();
  for (int ChN=0; ChN<FMidLen; ChN++){
    char Ch=FMid[ChN];
    if (TCh::IsAlNum(Ch)){NrFMid+=Ch;} else {NrFMid+='_';}
  }
  return NrFMid;
}

TStr TStr::GetNrFExt(const TStr& FExt){
  if (FExt.Empty()||(FExt[0]=='.')){return FExt;}
  else {return TStr(".")+FExt;}
}

TStr TStr::GetNrNumFExt(const int& FExtN, const int& MinLen){
  TStr FExtNStr=TInt::GetStr(FExtN);
  while (FExtNStr.Len()<MinLen){
    FExtNStr=TStr("0")+FExtNStr;}
  return FExtNStr;
}

TStr TStr::GetNrFNm(const TStr& FNm){
  return GetNrFPath(FNm.GetFPath())+FNm.GetFMid()+GetNrFExt(FNm.GetFExt());
}

TStr TStr::GetNrAbsFPath(const TStr& FPath, const TStr& BaseFPath){
  TStr NrBaseFPath;
  if (BaseFPath.Empty()){
    NrBaseFPath=GetNrFPath(TDir::GetCurDir());
  } else {
    NrBaseFPath=GetNrFPath(BaseFPath);
  }
  IAssertR(IsAbsFPath(NrBaseFPath), NrBaseFPath);
  TStr NrFPath=GetNrFPath(FPath);
  TStr NrAbsFPath;
  if (IsAbsFPath(NrFPath)){
    NrAbsFPath=NrFPath;
  } else {
    NrAbsFPath=GetNrFPath(NrBaseFPath+NrFPath);
  }
  NrAbsFPath.ChangeStrAll("/./", "/");
  NrAbsFPath.ChangeStrAll("\\.\\", "\\");
  return NrAbsFPath;
}

bool TStr::IsAbsFPath(const TStr& FPath){
  if ((FPath.Len()>=3)&&isalpha(FPath[0])&&(FPath[1]==':')&&
   ((FPath[2]=='/')||(FPath[2]=='\\'))){
    return true;
  }
  if ((FPath.Len()>=2)&&(FPath[0]=='/')&&isalpha(FPath[1])){
    return true;
  }
  return false;
}

TStr TStr::PutFExt(const TStr& FNm, const TStr& FExt){
  return FNm.GetFPath()+FNm.GetFMid()+FExt;
}

TStr TStr::PutFExtIfEmpty(const TStr& FNm, const TStr& FExt){
  if (FNm.GetFExt().Empty()){
    return FNm.GetFPath()+FNm.GetFMid()+FExt;
  } else {
    return FNm;
  }
}

TStr TStr::PutFBase(const TStr& FNm, const TStr& FBase){
  return FNm.GetFPath()+FBase;
}

TStr TStr::PutFBaseIfEmpty(const TStr& FNm, const TStr& FBase){
  if (FNm.GetFBase().Empty()){
    return FNm.GetFPath()+FBase;
  } else {
    return FNm;
  }
}

TStr TStr::AddToFMid(const TStr& FNm, const TStr& ExtFMid){
  return FNm.GetFPath()+FNm.GetFMid()+ExtFMid+FNm.GetFExt();
}

TStr TStr::GetNumFNm(const TStr& FNm, const int& Num){
  return FNm.GetFPath()+FNm.GetFMid()+TInt::GetStr(Num, "%03d")+FNm.GetFExt();
}

TStr TStr::GetFNmStr(const TStr& Str, const bool& AlNumOnlyP){
  TChA FNm=Str;
  for (int ChN=0; ChN<FNm.Len(); ChN++){
    uchar Ch=FNm[ChN];
    if (AlNumOnlyP){
      if (
       (('0'<=Ch)&&(Ch<='9'))||
       (('A'<=Ch)&&(Ch<='Z'))||
       (('a'<=Ch)&&(Ch<='z'))||
       (Ch=='-')||(Ch=='_')){}
      else {Ch='_';}
    } else {
      if ((Ch<=' ')||(Ch=='/')||(Ch=='\\')||(Ch==':')||(Ch=='.')){
        Ch='_';}
    }
    FNm.PutCh(ChN, Ch);
  }
  return FNm;
}

TStr TStr::GetStr(const TStr& Str, const char* FmtStr){
  if (FmtStr==nullptr){
    return Str;
  } else {
    char Bf[1000];
    sprintf(Bf, FmtStr, Str.CStr());
    return TStr(Bf);
  }
}

TStr TStr::GetStr(const TStrV& StrV, const TStr& DelimiterStr){
  if (StrV.Empty()){return TStr();}
  TChA ResStr=StrV[0];
  for (int StrN=1; StrN<StrV.Len(); StrN++){
    ResStr+=DelimiterStr;
    ResStr+=StrV[StrN];
  }
  return ResStr;
}

TStr TStr::Fmt(const char *FmtStr, ...) {
    char Bf[10 * 1024];
    va_list valist;
    va_start(valist, FmtStr);
    const int RequiredLen = vsnprintf(Bf, 10 * 1024 - 2, FmtStr, valist);
    va_end(valist);
    if (RequiredLen < 0) {
        // error
        return TStr();
    }
    else if (RequiredLen < 10 * 1024 - 2) {
        // fits in buffer
        return TStr(Bf);
    }
    else {
        // buffer was too short
        char* NewBf = new char[RequiredLen + 1];
        va_list valist;
        va_start(valist, FmtStr);
        const int RequiredLen2 = vsnprintf(NewBf, RequiredLen + 1, FmtStr, valist);
        va_end(valist);
        return WrapCStr(NewBf);
    }
}

TStr TStr::GetSpaceStr(const int& Spaces) {
    EAssert(Spaces >= 0);
    if (Spaces == 0) { return TStr(); }
    // we have more, go for it
	char *NewBf = new char[Spaces + 1];
	for (int SpaceN = 0; SpaceN < Spaces; SpaceN++) {
        NewBf[SpaceN] = ' ';
    }
	NewBf[Spaces] = 0;
	return WrapCStr(NewBf);
}

TStr operator+(const TStr& LStr, const char* RCStr) {
	const size_t LeftLen = LStr.Len();
	const size_t RightLen = ((RCStr == nullptr) ? 0 : strlen(RCStr));

	// check if any of the strings are empty
	if (LeftLen == 0) { return TStr(RCStr); }
	else if (RightLen == 0) { return LStr; }
	else {
		const char* LCStr = LStr.CStr();

		// allocate memory
		char* ConcatStr = new char[LeftLen + RightLen + 1];

		// copy the two strings into the new memory
		memcpy(ConcatStr, LCStr, LeftLen);
		memcpy(ConcatStr + LeftLen, RCStr, RightLen);

		// finish the new string
		ConcatStr[LeftLen + RightLen] = 0;

		// return
		return TStr::WrapCStr(ConcatStr);
	}
}

TStr operator+(const TStr& LStr, const TStr& RStr) {
	return operator+(LStr, RStr.CStr());
}

TStr operator+(const TStr& LStr, const char Ch) {
	const size_t LeftLen = LStr.Len();
	const size_t RightLen = 1;

	// check if any of the strings are empty
	if (LeftLen == 0) { return TStr(Ch); }
	else if (RightLen == 0) { return LStr; }
	else {
		const char* LCStr = LStr.CStr();

		// allocate memory
		char* ConcatStr = new char[LeftLen + RightLen + 1];

		// copy the two strings into the new memory
		memcpy(ConcatStr, LCStr, LeftLen);
		memcpy(ConcatStr + LeftLen, &Ch, RightLen);

		// finish the new string
		ConcatStr[LeftLen + RightLen] = 0;

		// return
		return TStr::WrapCStr(ConcatStr);
	}
}

bool TStr::IsUInt(TChRet& Ch, const bool& Check, const uint& MnVal, const uint& MxVal, uint& Val) {
	uint64 _Val;
	// assign and check for overflow
	return IsUInt64(Ch, Check, MnVal, MxVal, _Val) && (uint64)(Val = (uint)_Val) == _Val;
}

bool TStr::IsUInt64(TChRet& Ch, const bool& Check, const uint64& MnVal, const uint64& MxVal, uint64& Val) {
	EAssert(!Check || (Check && MnVal <= MxVal));

	uint64 _Val=0;
	while (TCh::IsWs(Ch())) {
		Ch.GetCh();
	}

	if (Ch()=='+'){Ch.GetCh();}
	if (!TCh::IsNum(Ch())){return false;}
	_Val=TCh::GetNum(Ch());

	const uint64 MxBy10 = Check ? MxVal / 10 : TUInt64::Mx / 10;
	const uint64 MxBy10Rem = Check ? MxVal % 10 : TUInt64::Mx % 10;

	uint ChNum;
	while (TCh::IsNum(Ch.GetCh())) {
		ChNum = TCh::GetNum(Ch());

		// check for overflow
		if (_Val > MxBy10 || (_Val == MxBy10 && ChNum > MxBy10Rem)) {
			return false;
		}

		// increase the sum
		_Val = 10*_Val + ChNum;
	}

	if (Check && _Val < MnVal){return false;}
	if (Ch.Eof()){Val=_Val; return true;} else {return false;}
}

/////////////////////////////////////////////////
// Input-String
TStrIn::TStrIn(const TStr& _Str, const bool& MakeCopyP) :
  TSBase(), TSIn(), OwnP(MakeCopyP),
  Bf(MakeCopyP ? _Str.CloneCStr() : _Str.CStr()), BfC(0), BfL(_Str.Len()){}

PSIn TStrIn::New(const TStr& Str, const bool& MakeCopyP){
  return PSIn(new TStrIn(Str, MakeCopyP));
}

int TStrIn::GetBf(const void* LBf, const TSize& LBfL){
  Assert(TSize(BfC+LBfL)<=TSize(BfL));
  int LBfS=0;
  for (TSize LBfC=0; LBfC<LBfL; LBfC++){
    LBfS+=(((char*)LBf)[LBfC]=Bf[BfC++]);}
  return LBfS;
}

bool TStrIn::GetNextLnBf(TChA& LnChA){
  // not implemented
  FailR(TStr::Fmt("TStrIn::GetNextLnBf: not implemented").CStr());
  return false;
}

TStr TStrIn::GetSNm() const {
  return "Input-String";
}

/////////////////////////////////////////////////
// String-Pool
void TStrPool::Resize(const uint& _MxBfL) {
  uint newSize = MxBfL;
  while (newSize < _MxBfL) {
    if (newSize >= GrowBy && GrowBy > 0) newSize += GrowBy;
    else if (newSize > 0) newSize *= 2;
    else newSize = TInt::GetMn(GrowBy, 1024);
    // check for overflow at 4GB
    IAssertR(newSize >= MxBfL, TStr::Fmt("TStrPool::Resize: %u, %u [Size larger than 4Gb, which is not supported by TStrPool]", newSize, MxBfL).CStr());
  }
  if (newSize > MxBfL) {
    Bf = (char *) realloc(Bf, newSize);
    IAssertR(Bf, TStr::Fmt("old Bf size: %u, new size: %u", MxBfL, newSize).CStr());
    MxBfL = newSize;
  }
  IAssertR(MxBfL >= _MxBfL, TStr::Fmt("new size: %u, requested size: %u", MxBfL, _MxBfL).CStr());
}

TStrPool::TStrPool(const uint& MxBfLen, const uint& _GrowBy) : MxBfL(MxBfLen), BfL(0), GrowBy(_GrowBy), Bf(0) {
  //IAssert(MxBfL >= 0); IAssert(GrowBy >= 0);
  if (MxBfL > 0) { Bf = (char *) malloc(MxBfL);  IAssertR(Bf, TStr::Fmt("Can not resize buffer to %u bytes. [Program failed to allocate more memory. Solution: Get a bigger machine.]", MxBfL).CStr()); }
  AddStr(""); // add an empty string at the beginning for fast future access
}

TStrPool::TStrPool(TSIn& SIn, bool LoadCompact) : MxBfL(0), BfL(0), GrowBy(0), Bf(0) {
  SIn.Load(MxBfL);  SIn.Load(BfL);  SIn.Load(GrowBy);
  //IAssert(MxBfL >= BfL);  IAssert(BfL >= 0);  IAssert(GrowBy >= 0);
  if (LoadCompact) MxBfL = BfL;
  if (MxBfL > 0) { Bf = (char *) malloc(MxBfL); IAssertR(Bf, TStr::Fmt("Can not resize buffer to %u bytes. [Program failed to allocate more memory. Solution: Get a bigger machine.]", MxBfL).CStr()); }
  if (BfL > 0) SIn.LoadBf(Bf, BfL);
  SIn.LoadCs();
}

void TStrPool::Save(TSOut& SOut) const {
  SOut.Save(MxBfL);  SOut.Save(BfL);  SOut.Save(GrowBy);
  SOut.SaveBf(Bf, BfL);
  SOut.SaveCs();
}

TStrPool& TStrPool::operator = (const TStrPool& Pool) {
  if (this != &Pool) {
    GrowBy = Pool.GrowBy;  MxBfL = Pool.MxBfL;  BfL = Pool.BfL;
    if (Bf) free(Bf); else IAssertR(MxBfL == 0, TStr::Fmt("size: %u, expected size: 0", MxBfL).CStr());
    Bf = (char *) malloc(MxBfL);  IAssertR(Bf, TStr::Fmt("Can not resize buffer to %u bytes. [Program failed to allocate more memory. Solution: Get a bigger machine.]", MxBfL).CStr());  memcpy(Bf, Pool.Bf, BfL);
  }
  return *this;
}

// Adds Len characters to pool. To append a null
// terminated string Len must be equal to strlen(s) + 1
uint TStrPool::AddStr(const char *Str, const uint& Len) {
  IAssertR(Len > 0, "String too short (length includes the null character)");  //J: if (! Len) return -1;
  if (Len == 1 && BfL > 0) { return 0; } // empty string
  Assert(Str);  Assert(Len > 0);
  if (BfL + Len > MxBfL) Resize(BfL + Len);
  memcpy(Bf + BfL, Str, Len);
  uint Pos = BfL;  BfL += Len;  return Pos;
}

int TStrPool::GetPrimHashCd(const char *CStr) {
  return TStrHashF_DJB::GetPrimHashCd(CStr);
}

int TStrPool::GetSecHashCd(const char *CStr) {
  return TStrHashF_DJB::GetSecHashCd(CStr);
}

/////////////////////////////////////////////////
// String-Pool-64bit
void TStrPool64::Resize(const ::TSize& _MxBfL) {
  ::TSize newSize = MxBfL;
  while (newSize < _MxBfL) {
    if (newSize >= GrowBy && GrowBy > 0) newSize += GrowBy;
    else if (newSize > 0) newSize *= 2;
    else newSize = (GrowBy > ::TSize(1024)) ? ::TSize(1024) : GrowBy;
    IAssert(newSize >= MxBfL); // assert we are growing
  }
  if (newSize > MxBfL) {
    Bf = (char *) realloc(Bf, newSize);
    IAssertR(Bf, TStr::Fmt("old Bf size: %u, new size: %u", MxBfL, newSize).CStr());
    MxBfL = newSize;
  }
  IAssert(MxBfL >= _MxBfL);
}

TStrPool64::TStrPool64(::TSize _MxBfL, ::TSize _GrowBy):
    MxBfL(_MxBfL), BfL(0), GrowBy(_GrowBy), Bf(NULL) {

  if (MxBfL > 0) { Bf = (char*)malloc(MxBfL); IAssert(Bf != NULL); }
  AddStr("");
}

TStrPool64::TStrPool64(const TStrPool64& StrPool):
  MxBfL(StrPool.MxBfL), BfL(StrPool.BfL), GrowBy(StrPool.GrowBy) {
  if (Bf != NULL) { free(Bf); } else { IAssert(MxBfL == 0); }
  Bf = (char*)malloc(StrPool.MxBfL); IAssert(Bf != NULL);
  memcpy(Bf, StrPool.Bf, BfL);
}

TStrPool64::TStrPool64(TSIn& SIn, bool LoadCompact):
  MxBfL(0), BfL(0), GrowBy(0), Bf(0) {
  uint64 _GrowBy, _MxBfL, _BfL;
  SIn.Load(_GrowBy); SIn.Load(_MxBfL); SIn.Load(_BfL);
  GrowBy = (::TSize)_GrowBy; MxBfL = (::TSize)_MxBfL; BfL = (::TSize)_BfL;
  if (LoadCompact) { MxBfL = BfL; }
  if (MxBfL > 0) { Bf = (char*)malloc(MxBfL); IAssert(Bf != NULL); }
  for (::TSize BfN = 0; BfN < _BfL; BfN++) { Bf[BfN] = SIn.GetCh(); }
  SIn.LoadCs();
}

void TStrPool64::Save(TSOut& SOut) const {
  uint64 _GrowBy = GrowBy, _MxBfL = MxBfL, _BfL = BfL;
  SOut.Save(_GrowBy);  SOut.Save(_MxBfL);  SOut.Save(_BfL);
  for (::TSize BfN = 0; BfN < _BfL; BfN++) { SOut.PutCh(Bf[BfN]); }
  SOut.SaveCs();
}

TStrPool64& TStrPool64::operator=(const TStrPool64& StrPool) {
  if (this != &StrPool) {
  GrowBy = StrPool.GrowBy;  MxBfL = StrPool.MxBfL;  BfL = StrPool.BfL;
  if (Bf != NULL) { free(Bf); } else { IAssert(MxBfL == 0); }
  Bf = (char*)malloc(MxBfL); IAssert(Bf != NULL);
  memcpy(Bf, StrPool.Bf, BfL);
  }
  return *this;
}

void TStrPool64::Clr(bool DoDel) {
  BfL = 0;
  if (DoDel && (Bf!=NULL)) {
    free(Bf);
    Bf = NULL; MxBfL = 0;
  }
}

uint64 TStrPool64::AddStr(const TStr& Str) {
  const int Len = Str.Len() + 1;
  if (BfL + Len > MxBfL) { Resize(BfL + Len); }
  memcpy(Bf + BfL, Str.CStr(), Len);
  ::TSize Offset = BfL;  BfL += Len;
  return uint64(Offset);
}

TStr TStrPool64::GetStr(const uint64& StrId) const {
  ::TSize Offset = (::TSize)StrId;
  return TStr(Bf + Offset);
}

/////////////////////////////////////////////////
// Void
void TVoid::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
}

void TVoid::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETag(Nm);
}

/////////////////////////////////////////////////
// Boolean
const bool TBool::Mn=0;
const bool TBool::Mx=1;
const int TBool::Vals=TBool::Mx-TBool::Mn+1;
TRnd TBool::Rnd;

const TStr TBool::FalseStr="F";
const TStr TBool::TrueStr="T";
const TStr TBool::NStr="N";
const TStr TBool::YStr="Y";
const TStr TBool::NoStr="No";
const TStr TBool::YesStr="Yes";

void TBool::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetBoolArg(XmlTok, "Val");
}

void TBool::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TBool::GetStr(Val));
}

bool TBool::IsValStr(const TStr& Str){
  TStr UcStr=Str.GetUc();
  return
   (UcStr==FalseStr)||(UcStr==TrueStr)||
   (UcStr==YStr)||(UcStr==NStr)||
   (UcStr==YesStr)||(UcStr==NoStr);
}

bool TBool::GetValFromStr(const TStr& Str){
  return (Str==TrueStr)||(Str==YStr)||(Str==YesStr);
}

bool TBool::GetValFromStr(const TStr& Str, const bool& DfVal){
  TStr UcStr=Str.GetUc();
  if (IsValStr(UcStr)){
    return (UcStr==TrueStr)||(UcStr==YStr)||(UcStr==YesStr);
  } else {
    return DfVal;
  }
}

/////////////////////////////////////////////////
// Char
const char TCh::Mn=CHAR_MIN;
const char TCh::Mx=CHAR_MAX;
const int TCh::Vals=int(TCh::Mx)-int(TCh::Mn)+1;

const char TCh::NullCh=char(0);
const char TCh::TabCh=char(9);
const char TCh::LfCh=char(10);
const char TCh::CrCh=char(13);
const char TCh::EofCh=char(26);
const char TCh::HashCh='#';

void TCh::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=char(TXmlObjSer::GetIntArg(XmlTok, "Val"));
}

void TCh::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TInt::GetStr(Val));
}

/////////////////////////////////////////////////
// Unsigned-Char
const uchar TUCh::Mn=0;
const uchar TUCh::Mx=UCHAR_MAX;
const int TUCh::Vals=int(TUCh::Mx)-int(TUCh::Mn)+1;

void TUCh::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=uchar(TXmlObjSer::GetIntArg(XmlTok, "Val"));
}

void TUCh::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TInt::GetStr(Val));
}

/////////////////////////////////////////////////
// Integer16

const int16 TSInt::Mn = SHRT_MIN;
const int16 TSInt::Mx = SHRT_MAX;

/////////////////////////////////////////////////
// Usigned Integer16

const uint16 TUSInt::Mn = 0;
const uint16 TUSInt::Mx = USHRT_MAX;

/////////////////////////////////////////////////
// Integer
const int TInt::Mn=INT_MIN;
const int TInt::Mx=INT_MAX;
const int TInt::Kilo=1024;
const int TInt::Mega=1024*1024;
const int TInt::Giga=1024*1024*1024;
TRnd TInt::Rnd;

void TInt::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetIntArg(XmlTok, "Val");
}

void TInt::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TInt::GetStr(Val));
}

TStr TInt::GetStr(const int& Val, const char* FmtStr){
  if (FmtStr==NULL){
    return GetStr(Val);
  } else {
    char Bf[255];
    sprintf(Bf, FmtStr, Val);
    return TStr(Bf);
  }
}

TStr TInt::GetSepStr(const char& Sep) const
{
    TStr StrVal = GetStr().Reverse();
    TChA Out;
    for (int N = 0; N < StrVal.Len(); N++) {
        if (N > 0 && N % 3 == 0)
            Out += Sep;
        Out += StrVal[N];
    }
    Out.Reverse();
    return Out;
}

//-----------------------------------------------------------------------------
// Frugal integer serialization
//-----------------------------------------------------------------------------
// These routines serialize integers to sequences of 1..4 bytes, with smaller
// integers receiving shorter codes.  They do not work directly with streams
// but rather with a user-provided buffer.  It is expected that one will want
// to store several such integers at once, and it would be inefficient to
// call the virtual TSOut::PutCh method once for every byte; this is why
// SaveFrugalInt and LoadFrugalInt work with a user-provided char* buffer
// rather than with TSIn/TSOut.  To store a vector of such integers, use the
// SaveFrugalIntV/LoadFrugalIntV pair.

char* TInt::SaveFrugalInt(char *pDest, int i){
  // <0xxx xxxx> has 128 combinations and is used to store -1..126.
  // <1xxx xxxx> <00xx xxxx> has 2^13 = 8192 combinations and is used to store 127..8318.
  // <1xxx xxxx> <01xx xxxx> has 2^13 = 8192 combinations and is used to store -2..-8193.
  // <1xxx xxxx> <1xxx xxxx> <xxxx xxxx> <0xxx xxxx> has 2^29 = 536870912 combinations and is used to store 8319..536879230.
  // <1xxx xxxx> <1xxx xxxx> <xxxx xxxx> <1xxx xxxx> has 2^29 = 536870912 combinations and is used to store -8194..-536879105.
  i++;
  if (i >= 0 && i <= 127) { *pDest++ = char(i); return pDest; }
  if (i >= 128 && i < 128 + 8192) { i -= 128; *pDest++ = char(0x80 | (i & 0x7f));
    *pDest++ = char((i >> 7) & 0x3f); return pDest; }
  if (i <= -1 && i > -1 - 8192) { i = -1 - i;  *pDest++ = char(0x80 | (i & 0x7f));
    *pDest++ = char(0x40 | ((i >> 7) & 0x3f)); return pDest; }
  if (i >= 128 + 8192 && i < 128 + 8192 + 536870912) { i -= 128 + 8192;
    *pDest++ = char(0x80 | (i & 0x7f)); *pDest++ = char(0x80 | ((i >> 7) & 0x7f));
    *pDest++ = char((i >> 14) & 0xff); *pDest++ = char((i >> 22) & 0x7f); return pDest; }
  if (i <= -1 - 8192 && i > -1 - 8192 - 536870912) { i = (-1 - 8192) - i;
    *pDest++ = char(0x80 | (i & 0x7f)); *pDest++ = char(0x80 | ((i >> 7) & 0x7f));
    *pDest++ = char((i >> 14) & 0xff); *pDest++ = char(0x80 | ((i >> 22) & 0x7f)); return pDest; }
  IAssertR(false, TInt::GetStr(i)); return 0;
}

char* TInt::LoadFrugalInt(char *pSrc, int& i){
  i = 0;
  int ch = (int) ((unsigned char) (*pSrc++));
  if ((ch & 0x80) == 0) { i = ch; i--; return pSrc; }
  i = (ch & 0x7f);
  ch = (int) ((unsigned char) (*pSrc++));
  if ((ch & 0x80) == 0)
  {
    i |= (ch & 0x3f) << 7;
    if ((ch & 0x40) == 0) i += 128; else i = -1 - i;
    i--; return pSrc;
  }
  i |= (ch & 0x7f) << 7;
  ch = (int) ((unsigned char) (*pSrc++));
  i |= ch << 14;
  ch = (int) ((unsigned char) (*pSrc++));
  i |= (ch & 0x7f) << 22;
  if ((ch & 0x80) == 0) i += 128 + 8192; else i = (-1 - 8192) - i;
  i--; return pSrc;
}

// Tests the SaveFrugalInt/LoadFrugalInt combination on all the
// integers they can work with (about 10^9 integers).
void TInt::TestFrugalInt(){
  char buf[10], *p = &buf[0], *r, *s;
  int i, j;
#define __TEST(from, to, len) \
  for (i = (from); i <= (to); i++) \
    { if ((i & 0xffff) == 0) printf("%d\r", i); \
      r = SaveFrugalInt(p, i); s = LoadFrugalInt(p, j); \
      IAssert(r == s); IAssert(i == j); IAssert(r - p == len); }

  __TEST(-1, 126, 1);
  __TEST(127, 127 + 8191, 2);
  __TEST(-2 - 8191, -2, 2);
  __TEST(127 + 8192, 127 + 8191 + (1 << 29), 4);
  __TEST(-2 - 8191 - (1 << 29), -2 - 8192, 4);
#undef __TEST
}

// Suppose that the contents of 'v', encoded using SaveFrugalInt,
// occupy 'n' bytes.  SaveFrugalIntV first stores 'n' (using
// SaveFrugalInt), then the contents.
void TInt::SaveFrugalIntV(TSOut& SOut, const TIntV& v){
  // Prepare a large enough buffer.
  int count = v.Len();
  char *buf = new char[4 * (count + 1)], *pStart, *pEnd;
  // Encode the contents of 'v'.
  pStart = buf + 4; pEnd = pStart;
  for (int i = 0; i < count; i++)
    pEnd = SaveFrugalInt(pEnd, v[i].Val);
  // Encode the size of the encoded contents of 'v'.
  // This is stored at the beginning of 'buf' and is then
  // moved so that there is no gap between it and the
  // beginning of the stored contents (at pStart).
  int size = int(pEnd - pStart);
  char *pSizeStart = buf;
  char *pSizeEnd = SaveFrugalInt(pSizeStart, size);
  while (pSizeEnd > pSizeStart) *(--pStart) = *(--pSizeEnd);
  // Write the buffer and free the memory.
  SOut.PutBf(pStart, TSize(pEnd - pStart));
  delete[] buf;
}

// Loads an integer 'n' (using LoadFrugalInt), then loads
// 'n' bytes, decoding them using LoadFrugalInt and adding
// them to the vector 'v'.  If clearVec is true, 'v' is
// cleared before anything is added to it.
void TInt::LoadFrugalIntV(TSIn& SIn, TIntV& v, bool clearVec){
  if (clearVec) v.Clr();
  char sizeBuf[4], *p, *pEnd;
  // Load the first frugally-stored integer into the sizeBuf
  // buffer.  'count' bytes will be read.
  sizeBuf[0] = SIn.GetCh(); int count = 1;
  if (sizeBuf[0] & 0x80)
  {
    sizeBuf[1] = SIn.GetCh(); count++;
    if (sizeBuf[1] & 0x80) { sizeBuf[2] = SIn.GetCh();
      sizeBuf[3] = SIn.GetCh(); count += 2;}
  }
  // Decode the stored size.
  int size;
  pEnd = LoadFrugalInt(&sizeBuf[0], size);
  IAssert(pEnd - &sizeBuf[0] == count);
  if (size <= 0) return;
  // Allocate a buffer and read the compressed data.
  char *buf = new char[size];
  SIn.GetBf(buf, size);
  // Decode the compressed integers and add them into 'v'.
  p = buf; pEnd = buf + size;
  while (p < pEnd)
    { int i; p = LoadFrugalInt(p, i); v.Add(i); }
  IAssert(p == pEnd);
  delete[] buf;
}

/////////////////////////////////////////////////
// Unsigned-Integer
const uint TUInt::Mn=0;
const uint TUInt::Mx=UINT_MAX;
TRnd TUInt::Rnd;

void TUInt::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetIntArg(XmlTok, "Val");
}

void TUInt::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TInt::GetStr(Val));
}

TStr TUInt::GetStr(const uint& Val, const char* FmtStr){
  if (FmtStr==NULL){
    return GetStr(Val);
  } else {
    char Bf[255];
    sprintf(Bf, FmtStr, Val);
    return TStr(Bf);
  }
}

bool TUInt::IsIpStr(const TStr& IpStr, uint& Ip, const char& SplitCh) {
	TStrV IpStrV; IpStr.SplitOnAllCh(SplitCh, IpStrV);
    Ip = 0; int Byte = 0;
	if (IpStrV.Len() != 4) { return false; }
	if (!IpStrV[0].IsInt(true, 0, 255, Byte)) { return false; }; Ip = (uint)Byte;
	if (!IpStrV[1].IsInt(true, 0, 255, Byte)) { return false; }; Ip = (Ip << 8) | (uint)Byte;
	if (!IpStrV[2].IsInt(true, 0, 255, Byte)) { return false; }; Ip = (Ip << 8) | (uint)Byte;
	if (!IpStrV[3].IsInt(true, 0, 255, Byte)) { return false; }; Ip = (Ip << 8) | (uint)Byte;
	return true;
}

uint TUInt::GetUIntFromIpStr(const TStr& IpStr, const char& SplitCh) {
	TStrV IpStrV; IpStr.SplitOnAllCh(SplitCh, IpStrV);
    uint Ip = 0; int Byte = 0;
	EAssertR(IpStrV[0].IsInt(true, 0, 255, Byte), TStr::Fmt("Bad IP: '%s;", IpStr.CStr())); Ip = (uint)Byte;
	EAssertR(IpStrV[1].IsInt(true, 0, 255, Byte), TStr::Fmt("Bad IP: '%s;", IpStr.CStr())); Ip = (Ip << 8) | (uint)Byte;
	EAssertR(IpStrV[2].IsInt(true, 0, 255, Byte), TStr::Fmt("Bad IP: '%s;", IpStr.CStr())); Ip = (Ip << 8) | (uint)Byte;
	EAssertR(IpStrV[3].IsInt(true, 0, 255, Byte), TStr::Fmt("Bad IP: '%s;", IpStr.CStr())); Ip = (Ip << 8) | (uint)Byte;
	return Ip;
}

TStr TUInt::GetStrFromIpUInt(const uint& Ip) {
  return TStr::Fmt("%d.%d.%d.%d", ((Ip>>24) & 0xFF),
   ((Ip>>16) & 0xFF), ((Ip>>8) & 0xFF), (Ip & 0xFF));
}

bool TUInt::IsIpv6Str(const TStr& IpStr, const char& SplitCh) {
	TStrV IpStrV; IpStr.SplitOnAllCh(SplitCh, IpStrV, false);
	// check we have 8 groups
	if (IpStrV.Len() > 8) { return false; }
	// each group must be in hexa and in range from 0x0000 to 0xFFFF
	int Group = 0;
	for (int IpStrN = 0; IpStrN < IpStrV.Len(); IpStrN++) {
		if (IpStrV[IpStrN].Empty()) { continue; }
		if (IpStrV[IpStrN].IsHexInt(true, 0x0000, 0xFFFF, Group)) { continue; }
		return false;
	}
	// all fine
	return true;
}


/////////////////////////////////////////////////
// Signed-Integer-64Bit

//#if defined (GLib_WIN)
const int64 TInt64::Mn(INT64_MIN);
const int64 TInt64::Mx(INT64_MAX);
//#else

//#endif

/////////////////////////////////////////////////
// Unsigned-Integer-64Bit

#if defined (GLib_WIN)
const uint64 TUInt64::Mn(uint64(0x0000000000000000i64));
const uint64 TUInt64::Mx(uint64(0xFFFFFFFFFFFFFFFFi64));
#elif defined (GLib_BCB)
const uint64 TUInt64::Mn(0x0000000000000000i64);
const uint64 TUInt64::Mx(0xFFFFFFFFFFFFFFFFi64);
#else
const uint64 TUInt64::Mn((uint64)0x0000000000000000LL);
const uint64 TUInt64::Mx(0xFFFFFFFFFFFFFFFFLL);
#endif

void TUInt64::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetInt64Arg(XmlTok, "Val");
}

void TUInt64::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TUInt64::GetStr(Val));
}

/*/////////////////////////////////////////////////
// Unsigned-Integer-64Bit
const TUInt64 TUInt64::Mn(0, 0);
const TUInt64 TUInt64::Mx(UINT_MAX, UINT_MAX);

void TUInt64::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm); XLoad(TInt(MsVal)); XLoad(TInt(LsVal));
}

void TUInt64::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveHd(Nm); XSave(TInt(MsVal)); XSave(TInt(LsVal));
}

TUInt64& TUInt64::operator+=(const TUInt64& Int){
  uint CarryVal=(LsVal>TUInt::Mx-Int.LsVal) ? 1 : 0;
  LsVal+=Int.LsVal;
  MsVal+=Int.MsVal+CarryVal;
  return *this;
}

TUInt64& TUInt64::operator-=(const TUInt64& Int){
  IAssert(*this>=Int);
  uint CarryVal;
  if (LsVal>=Int.LsVal){
    LsVal-=Int.LsVal; CarryVal=0;
  } else {
    LsVal+=(TUInt::Mx-Int.LsVal)+1; CarryVal=1;
  }
  MsVal-=(Int.MsVal+CarryVal);
  return *this;
}

TUInt64 TUInt64::operator++(int){
  if (LsVal==TUInt::Mx){
    Assert(MsVal<TUInt::Mx); MsVal++; LsVal=0;}
  else {LsVal++;}
  return *this;
}

TUInt64 TUInt64::operator--(int){
  if (LsVal==0){
    Assert(MsVal>0); MsVal--; LsVal=TUInt::Mx;}
  else {LsVal--;}
  return *this;
}

TStr TUInt64::GetStr(const TUInt64& Int){
  char Bf[255]; sprintf(Bf, "%.0Lf", ldouble(Int));
  return TStr(Bf);
}

TStr TUInt64::GetHexStr(const TUInt64& Int){
  char Bf[255]; sprintf(Bf, "%08X%08X", Int.MsVal, Int.LsVal);
  return TStr(Bf);
}*/

/////////////////////////////////////////////////
// Float
const double TFlt::Mn=-DBL_MAX;
const double TFlt::Mx=+DBL_MAX;
const double TFlt::NInf=-DBL_MAX;
const double TFlt::PInf=+DBL_MAX;
const double TFlt::Eps=1e-16;
const double TFlt::EpsHalf  =1e-7;

TRnd TFlt::Rnd;

void TFlt::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetFltArg(XmlTok, "Val");
}

void TFlt::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TFlt::GetStr(Val));
}

TStr TFlt::GetStr(const double& Val, const int& Width, const int& Prec){
  char Bf[255];
  if ((Width==-1)&&(Prec==-1)){sprintf(Bf, "%g", Val);}
  else {sprintf(Bf, "%*.*f", Width, Prec, Val);}
  return TStr(Bf);
}

TStr TFlt::GetStr(const double& Val, const char* FmtStr){
  if (FmtStr==NULL){
    return GetStr(Val);
  } else {
    char Bf[255];
    sprintf(Bf, FmtStr, Val);
    return TStr(Bf);
  }
}

/////////////////////////////////////////////////
// Short-Float
const sdouble TSFlt::Mn=-FLT_MIN;
const sdouble TSFlt::Mx=+FLT_MAX;

void TSFlt::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=sdouble(TXmlObjSer::GetFltArg(XmlTok, "Val"));
}

void TSFlt::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TFlt::GetStr(Val));
}

/////////////////////////////////////////////////
// Long-Float
const ldouble TLFlt::Mn=-LDBL_MAX;
const ldouble TLFlt::Mx=+LDBL_MAX;
/*const ldouble TUInt64::Mn_LFlt=TUInt64::Mn;
const ldouble TUInt64::Mx_LFlt=TUInt64::Mx;
const ldouble TUInt64::MxP1_LFlt=ldouble(TUInt::Mx)+ldouble(1);*/

void TLFlt::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  Val=TXmlObjSer::GetFltArg(XmlTok, "Val");
}

void TLFlt::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg(Nm, "Val", TFlt::GetStr(double(Val)));
}

TStr TLFlt::GetStr(const ldouble& Val, const int& Width, const int& Prec){
  char Bf[255];
  if ((Width==-1)&&(Prec==-1)){sprintf(Bf, "%Lg", Val);}
  else {sprintf(Bf, "%*.*Lf", Width, Prec, Val);}
  return TStr(Bf);
}

TStr TLFlt::GetStr(const ldouble& Val, const char* FmtStr){
  if (FmtStr==NULL){
    return GetStr(Val);
  } else {
    char Bf[255];
    sprintf(Bf, FmtStr, Val);
    return TStr(Bf);
  }
}

/////////////////////////////////////////////////
// Float-Rectangle
void TFltRect::LoadXml(const PXmlTok& XmlTok, const TStr& Nm){
  XLoadHd(Nm);
  MnX=TXmlObjSer::GetFltArg(XmlTok, "MnX");
  MnY=TXmlObjSer::GetFltArg(XmlTok, "MnY");
  MxX=TXmlObjSer::GetFltArg(XmlTok, "MxX");
  MxY=TXmlObjSer::GetFltArg(XmlTok, "MxY");
}

void TFltRect::SaveXml(TSOut& SOut, const TStr& Nm) const {
  XSaveBETagArg4(Nm,
   "MnX", TFlt::GetStr(double(MnX)), "MnY", TFlt::GetStr(double(MnY)),
   "MxX", TFlt::GetStr(double(MxX)), "MxY", TFlt::GetStr(double(MxY)));
}

bool TFltRect::Intersection(const TFltRect& Rect1, const TFltRect& Rect2){
  const double MnXX = TFlt::GetMx(Rect1.GetMnX(), Rect2.GetMnX());
  const double MnYY = TFlt::GetMx(Rect1.GetMnY(), Rect2.GetMnY());
  const double MxXX = TFlt::GetMn(Rect1.GetMxX(), Rect2.GetMxX());
  const double MxYY = TFlt::GetMn(Rect1.GetMxY(), Rect2.GetMxY());
  return (MnXX < MxXX) && (MnYY < MxYY);
}

TStr TFltRect::GetStr() const {
  TChA ChA;
  ChA+='(';
  ChA+=TFlt::GetStr(MnX, "%0.2f"); ChA+=',';
  ChA+=TFlt::GetStr(MnY, "%0.2f"); ChA+=',';
  ChA+=TFlt::GetStr(MxX, "%0.2f"); ChA+=',';
  ChA+=TFlt::GetStr(MxY, "%0.2f"); ChA+=')';
  return ChA;
}

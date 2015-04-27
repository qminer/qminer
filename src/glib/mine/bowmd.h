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

/////////////////////////////////////////////////
// BagOfWords-Model
ClassTPV(TBowMd, PBowMd, TBowMdV)//{
protected:
  typedef PBowMd (*TBowMdLoad)(TSIn& SIn);
  typedef TFunc<TBowMdLoad> TBowMdLoadF;
  static THash<TStr, TBowMdLoadF> TypeToLoadFH;
  static bool Reg(const TStr& TypeNm, const TBowMdLoadF& LoadF);
private:
  TInt BowDocBsSig; // corresponding BowDocBs signature
  UndefDefaultCopyAssign(TBowMd);
public:
  TBowMd(const PBowDocBs& BowDocBs):
    BowDocBsSig(BowDocBs->GetSig()){}
  virtual ~TBowMd(){}
  TBowMd(TSIn& SIn):
    BowDocBsSig(SIn){}
  static PBowMd Load(TSIn& SIn);
  virtual void Save(TSOut& SOut){
    GetTypeNm(*this).Save(SOut); BowDocBsSig.Save(SOut);}

  // signature
  int GetBowDocBsSig() const {return BowDocBsSig;}

  // classification
  virtual void GetCfy(const PBowSpV& QueryBowSpV, TFltStrPrV& WgtCatNmPrV)=0;
  static void SaveXmlCfy(const PSOut& SOut, const TFltStrPrV& WgtCatNmPrV);
  static void SaveTxtCfy(const PSOut& SOut, const TFltStrPrV& WgtCatNmPrV);

  // model information
  virtual bool IsCat() const {return false;}
  virtual TStr GetCatNm() const {return "";}
  virtual bool IsLinComb() const {return false;}
  virtual void GetLinComb(
   const PBowDocBs&, TFltStrPrV& WgtStrPrV, double& Tsh) const {
    IAssert(IsLinComb()); WgtStrPrV.Clr(); Tsh=0;}

  // statistics
  virtual void SaveTxt(const TStr& FNm, const PBowDocBs& BowDocBs,
      const PBowDocWgtBs& BowDocWgtBs){}

  // binary files
  static const TStr BowMdFExt;
  static PBowMd LoadBin(const TStr& FNm, const PBowDocBs& BowDocBs);
  void SaveBin(const TStr& FNm){
    TFOut SOut(FNm); Save(SOut);}
};

/////////////////////////////////////////////////
// BagOfWords-Multi-Model
class TBowMultiMd: public TBowMd {
private:
  static bool IsReg;
  static bool MkReg(){return TBowMd::Reg(TTypeNm<TBowMultiMd>(), &Load);}
private:
  TBowMdV BowMdV;
public:
  TBowMultiMd(const PBowDocBs& BowDocBs, const TBowMdV& _BowMdV = TBowMdV()):
    TBowMd(BowDocBs), BowMdV(_BowMdV) { }
  static PBowMd New(const PBowDocBs& BowDocBs, const TBowMdV& _BowMdV) {
    return new TBowMultiMd(BowDocBs, _BowMdV); }
  TBowMultiMd(TSIn& SIn):
    TBowMd(SIn), BowMdV(SIn){}
  static PBowMd Load(TSIn& SIn){return PBowMd(new TBowMultiMd(SIn));}
  void Save(TSOut& SOut){
    TBowMd::Save(SOut); BowMdV.Save(SOut);}

  // model vector manipulation
  int AddBowMd(const PBowMd& BowMd){return BowMdV.Add(BowMd);}
  int GetBowMds() const {return BowMdV.Len();}
  PBowMd GetBowMd(const int& BowMdN) const {return BowMdV[BowMdN];}

  // classification
  void GetCfy(const PBowSpV& QueryBowSpV, TFltStrPrV& WgtCatNmPrV);
};

/////////////////////////////////////////////////
// BagOfWords-Winnow-Model
class TBowWinnowMd: public TBowMd {
private:
  static bool IsReg;
  static bool MkReg(){return TBowMd::Reg(TTypeNm<TBowWinnowMd>(), &Load);}
private:
  TStr CatNm;
  TFlt Beta;
  TFlt VoteTsh;
  TFltV PosExpertWgtV;
  TFltV NegExpertWgtV;
public:
  TBowWinnowMd(const PBowDocBs& BowDocBs):
    TBowMd(BowDocBs){}
  TBowWinnowMd(TSIn& SIn):
    TBowMd(SIn), CatNm(SIn), Beta(SIn), VoteTsh(SIn),
    PosExpertWgtV(SIn), NegExpertWgtV(SIn){}
  static PBowMd Load(TSIn& SIn){return PBowMd(new TBowWinnowMd(SIn));}
  void Save(TSOut& SOut){TBowMd::Save(SOut);
    CatNm.Save(SOut); Beta.Save(SOut); VoteTsh.Save(SOut);
    PosExpertWgtV.Save(SOut); NegExpertWgtV.Save(SOut);}

  // model creation
  static PBowMd New(
   const PBowDocBs& BowDocBs, const TStr& CatNm, const double& Beta=0.95);
  static PBowMd NewMulti(
   const PBowDocBs& BowDocBs, const int& TopCats=-1, const double& Beta=0.95);

  // classification
  void GetCfy(const PBowSpV& QueryBowSpV, TFltStrPrV& WgtCatNmPrV);
};

/////////////////////////////////////////////////
// BagOfWords-Centroid-Model
class TBowCentroidMd: public TBowMd {
private:
  static bool IsReg;
  static bool MkReg(){return TBowMd::Reg(TTypeNm<TBowCentroidMd>(), &Load);}

private:
  TStr CatNm;
  TFltV CentroidV;

  TBowCentroidMd(const PBowDocBs& BowDocBs):
    TBowMd(BowDocBs){}

public:
  TBowCentroidMd(TSIn& SIn): TBowMd(SIn), CatNm(SIn), CentroidV(SIn){}
  static PBowMd Load(TSIn& SIn){return PBowMd(new TBowCentroidMd(SIn));}
  void Save(TSOut& SOut){TBowMd::Save(SOut); CatNm.Save(SOut); CentroidV.Save(SOut); }

  // model creation
  static PBowMd New(const PBowDocBs& BowDocBs, 
		const PBowDocWgtBs& BowDocWgtBs, const TStr& CatNm, 
		const TIntV& TrainDIdV);

  // classification
  void GetCfy(const PBowSpV& QueryBowSpV, TFltStrPrV& WgtCatNmPrV);
  TStr GetCatNm() const {return CatNm;}
};

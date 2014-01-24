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

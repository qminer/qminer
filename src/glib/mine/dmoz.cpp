/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// DMoz-Info

// RDF files
const TStr TDMozInfo::RdfStructFBase="structure.rdf.u8";
const TStr TDMozInfo::RdfContentFBase="content.rdf.u8";
const TStr TDMozInfo::RdfStructSampleFBase="kt-structure.rdf.u8";
const TStr TDMozInfo::RdfContentSampleFBase="kt-content.rdf.u8";

// binary files
const TStr TDMozInfo::BinStructFBase="DMozStruct.DMoz";
const TStr TDMozInfo::BinFullFBase="DMozFull.DMoz";
const TStr TDMozInfo::BinStructSampleFBase="DMozSampleStruct.DMoz";
const TStr TDMozInfo::BinFullSampleFBase="DMozFullSample.DMoz";

/////////////////////////////////////////////////
// DMoz-Base
void TDMozBs::GetCatIdV(const TStrV& CatNmV, TIntV& CatIdV) const {
  CatIdV.Clr();
  for (int CatNmN=0; CatNmN<CatNmV.Len(); CatNmN++){
    int CatId=GetCatId(CatNmV[CatNmN]);
    if (CatId==-1){
      printf("\nWarning: Invalid Category Name ('%s')\n.", CatNmV[CatNmN].CStr());}
    CatIdV.Add(CatId);
  }
}

void TDMozBs::GetSubCatTypeIdPrV(const int& CatId, TUIntV& _SubCatTypeIdPrV) const {
  _SubCatTypeIdPrV.Clr();
  int SubCatTypeIdVN=CatMd5ToTopicH[CatId].SubCatTypeIdVN;
  if (SubCatTypeIdVN!=-1){
    uint UndefSubCatTypeIdPr=GetSubCatTypeIdPr(dmsctUndef); // last pair
    forever{
      // get sub-category type-id pair in uint
      uint SubCatTypeIdPr=SubCatTypeIdPrV[SubCatTypeIdVN];
      // check for terminator guard
      if (SubCatTypeIdPr==UndefSubCatTypeIdPr){break;}
      // add sub-category type-id pair
      _SubCatTypeIdPrV.Add(SubCatTypeIdPr);
      // increment vector index
      SubCatTypeIdVN++;
    }
  }
}

void TDMozBs::GetSubCatIdV(
 const int& CatId, TIntV& SubCatIdV, const bool& SubTreeOnlyP) const {
  SubCatIdV.Clr();
  TUIntV SubCatTypeIdPrV; GetSubCatTypeIdPrV(CatId, SubCatTypeIdPrV);
  for (int SubCatN=0; SubCatN<SubCatTypeIdPrV.Len(); SubCatN++){
    // get subcategory data
    int SubCatId; TDMozSubCatType SubCatType;
    TDMozBs::GetSubCatTypeCatId(SubCatTypeIdPrV[SubCatN], SubCatType, SubCatId);
    // add sub-category-id if type appropriate
    if ((SubTreeOnlyP&&IsSubTreeSubCatType(SubCatType))||(!SubTreeOnlyP)){
      SubCatIdV.Add(SubCatId);
    }
  }
}

void TDMozBs::GetExtUrlIdV(
 const int& CatId, TIntV& _ExtUrlIdV, const bool& AppendP) const {
  if (!AppendP){_ExtUrlIdV.Clr();}
  // get start position in vector pool
  int ExtUrlIdVN=CatMd5ToTopicH[CatId].ExtUrlIdVN;
  if (ExtUrlIdVN!=-1){
    forever{
      // get external-url-id
      int ExtUrlId=ExtUrlIdV[ExtUrlIdVN];
      // check for terminator guard
      if (ExtUrlId==-1){break;}
      // add external-url-id
      _ExtUrlIdV.Add(ExtUrlId);
      // increment vector index
      ExtUrlIdVN++;
    }
  }
}

void TDMozBs::GetExtUrlIdV(
 const TIntV& CatIdV, TIntV& _ExtUrlIdV, const bool& AppendP) const {
  if (!AppendP){_ExtUrlIdV.Clr();}
  for (int CatIdN=0; CatIdN<CatIdV.Len(); CatIdN++){
    int CatId=CatIdV[CatIdN];
    //printf("%s\n", DMozBs->GetCatNm(CatId).CStr());
    GetExtUrlIdV(CatId, _ExtUrlIdV, true);
  }
}

int TDMozBs::GetRndExtUrlId(const int& RootCatId, TRnd& Rnd){
  TIntV SubCatIdV; GetSubCatIdV(RootCatId, SubCatIdV);
  TIntV ExtUrlIdV; GetExtUrlIdV(RootCatId, ExtUrlIdV);
  if (!ExtUrlIdV.Empty()){SubCatIdV.Add(-1);}
  SubCatIdV.Shuffle(Rnd);
  if (SubCatIdV.Empty()){
    return -1;
  } else {
    int CatId=SubCatIdV[0];
    if (CatId==-1){
      ExtUrlIdV.Shuffle(Rnd);
      return ExtUrlIdV[0];
    } else {
      return GetRndExtUrlId(CatId, Rnd);
    }
  }
}

void TDMozBs::GetSubTreeCatIdV(
 const int& RootCatId, const TIntV& PosCatIdV, const TIntV& NegCatIdV,
 TIntV& CatIdV, const bool& SubTreeOnlyP) const {
  CatIdV.Clr();
  // create traversal queue
  TIntPrQ CatIdTypePrQ; TIntH CatIdH; TIntH WaitCatIdH;
  CatIdTypePrQ.Push(TIntPr(RootCatId, dmsctNarrow));
  WaitCatIdH.AddKey(RootCatId);
  while (!CatIdTypePrQ.Empty()){
    // get top-queue-value
    TInt CatId; TInt SubCatTypeInt;
    CatIdTypePrQ.Top().GetVal(CatId, SubCatTypeInt); CatIdTypePrQ.Pop();
    // add value to processed-categories
    IAssert(!CatIdH.IsKey(CatId));
    CatIdH.AddKey(CatId); CatIdV.Add(CatId);
    WaitCatIdH.DelKey(CatId);
    //printf("%d %s\n", CatIdH.Len(), GetCatNm(CatId).CStr());
    // process sub-categories
    TDMozSubCatType SubCatType=TDMozSubCatType(int(SubCatTypeInt));
    if (IsSubTreeSubCatType(SubCatType)){
      // get subcategories
      TUIntV SubCatTypeIdPrV; GetSubCatTypeIdPrV(CatId, SubCatTypeIdPrV);
      for (int SubCatN=0; SubCatN<SubCatTypeIdPrV.Len(); SubCatN++){
        // get subcategory data
        int SubCatId; TDMozSubCatType SubCatType;
        TDMozBs::GetSubCatTypeCatId(SubCatTypeIdPrV[SubCatN], SubCatType, SubCatId);
        // add sub-category-id if new
        if ((!CatIdH.IsKey(SubCatId))&&(!WaitCatIdH.IsKey(SubCatId))){
          // add sub-category-id if type appropriate
          if ((SubTreeOnlyP&&IsSubTreeSubCatType(SubCatType))||(!SubTreeOnlyP)){
            if (((CatId==RootCatId)&&((PosCatIdV.Empty())||(PosCatIdV.IsIn(SubCatId)))&&
             ((NegCatIdV.Empty())||(!NegCatIdV.IsIn(SubCatId))))||(CatId!=RootCatId)){
              CatIdTypePrQ.Push(TIntPr(SubCatId, SubCatType));
              WaitCatIdH.AddKey(SubCatId);
            }
          }
        }
      }
    }
  }
}

void TDMozBs::GetSubTreeCatIdV(
 const TStr& RootCatNm, const TStrV& PosCatNmV, const TStrV& NegCatNmV,
 TIntV& CatIdV, const bool& SubTreeOnlyP) const {
  TIntV PosCatIdV; GetCatIdV(PosCatNmV, PosCatIdV);
  TIntV NegCatIdV; GetCatIdV(NegCatNmV, NegCatIdV);
  int CatId=GetCatId(RootCatNm);
  EAssertRA(CatId!=-1, "Category does not exist", RootCatNm);
  GetSubTreeCatIdV(CatId, PosCatIdV, NegCatIdV, CatIdV, SubTreeOnlyP);
}

void TDMozBs::GetSubTreeDocV(
 const TStr& RootCatNm, const TStrV& PosCatNmV, const TStrV& NegCatNmV,
 TStrV& DocNmV, TStrV& DocStrV,
 const bool& SubTreeOnlyP, const int& UrlWordsCopies) const {
  // get external-url-vector
  TIntV CatIdV; GetSubTreeCatIdV(RootCatNm, PosCatNmV, NegCatNmV, CatIdV, SubTreeOnlyP);
  TIntV ExtUrlIdV; GetExtUrlIdV(CatIdV, ExtUrlIdV);
  // traverse external-url-vector
  DocNmV.Gen(ExtUrlIdV.Len(), 0); DocStrV.Gen(ExtUrlIdV.Len(), 0);
  for (int ExtUrlIdN=0; ExtUrlIdN<ExtUrlIdV.Len(); ExtUrlIdN++){
    int ExtUrlId=ExtUrlIdV[ExtUrlIdN];
    TStr UrlStr=GetExtUrlStr(ExtUrlId);
    TStr TitleStr=GetExtUrlTitleStr(ExtUrlId);
    TStr DescStr=GetExtUrlDescStr(ExtUrlId);
    // create title&description-string
    TChA DocChA;
    if (UrlWordsCopies>0){
      DocChA+='['; DocChA+=TUrl::GetDocStrFromUrlStr(UrlStr, UrlWordsCopies); DocChA+="] ";
    }
    DocChA+=TitleStr; DocChA+=" -- ";
    DocChA+=DescStr; DocChA+='\n';
    // add to vectors
    DocNmV.Add(TInt::GetStr(ExtUrlId));
    DocStrV.Add(DocChA);
  }
}

PBowDocPartClust TDMozBs::GetBowDocPartClust(
 const int& RootCatId, const TIntV& PosCatIdV, const TIntV& NegCatIdV,
 const PBowDocBs& BowDocBs, const PBowDocWgtBs& BowDocWgtBs, const PBowSim& BowSim,
 const int& MnCatDocs, const int& Cats, int& CatN){
  // increment & notify category-number
  CatN++; printf("%d/%d\r", CatN, Cats);
  // get category-name
  TStr RootCatNm=GetCatNm(RootCatId);
  // prepare doc-ids
  TIntV SubTreeCatIdV; GetSubTreeCatIdV(RootCatId, PosCatIdV, NegCatIdV, SubTreeCatIdV);
  TIntV ExtUrlIdV; GetExtUrlIdV(SubTreeCatIdV, ExtUrlIdV);
  TIntV DIdV(ExtUrlIdV.Len(), 0);
  for (int ExtUrlIdN=0; ExtUrlIdN<ExtUrlIdV.Len(); ExtUrlIdN++){
    int DId=BowDocBs->GetDId(TInt::GetStr(ExtUrlIdV[ExtUrlIdN]));
    DIdV.Add(DId);
  }
  SubTreeCatIdV.Clr(); ExtUrlIdV.Clr();
  // create centroid-vector
  PBowSpV ConceptSpV=TBowClust::GetConceptSpV(BowDocWgtBs, BowSim, DIdV);
  // create sub-partition
  PBowDocPart SubDocPart=_GetBowDocPart(
    RootCatId, PosCatIdV, NegCatIdV,
    BowDocBs, BowDocWgtBs, BowSim, MnCatDocs, Cats, CatN);
  if (SubDocPart->GetClusts()==0){SubDocPart=0;}
  // create cluster
  PBowDocPartClust DocPartClust=
   TBowDocPartClust::New(BowDocBs, RootCatNm, 0, DIdV, ConceptSpV, SubDocPart);
  // return cluster
  return DocPartClust;
}

PBowDocPart TDMozBs::_GetBowDocPart(
 const int& RootCatId, const TIntV& PosCatIdV, const TIntV& NegCatIdV,
 const PBowDocBs& BowDocBs, const PBowDocWgtBs& BowDocWgtBs, const PBowSim& BowSim,
 const int& MnCatDocs, const int& Cats, int& CatN){
  TStr RootCatNm=GetCatNm(RootCatId);
  // create document partition
  PBowDocPart DocPart=TBowDocPart::New();
  DocPart->PutNm(RootCatNm);
  // get sub-category-ids
  TIntV SubCatIdV;
  if (PosCatIdV.Empty()){GetSubCatIdV(RootCatId, SubCatIdV, true);}
  else {SubCatIdV=PosCatIdV;}
  for (int NegCatIdN=0; NegCatIdN<NegCatIdV.Len(); NegCatIdN++){
    SubCatIdV.DelIfIn(NegCatIdV[NegCatIdN]);
  }
  // recurse
  for (int SubCatIdN=0; SubCatIdN<SubCatIdV.Len(); SubCatIdN++){
    PBowDocPartClust DocPartClust=GetBowDocPartClust(
     SubCatIdV[SubCatIdN], TIntV(), TIntV(), BowDocBs, BowDocWgtBs, BowSim, MnCatDocs, Cats, CatN);
    if (DocPartClust->GetDocs()>=MAX(1, MnCatDocs)){
      DocPart->AddClust(DocPartClust);}
  }
  // return document partition
  return DocPart;
}

PBowDocPart TDMozBs::GetBowDocPart(
 const TStr& RootCatNm, const TStrV& PosCatNmV, const TStrV& NegCatNmV,
 const PBowDocBs& BowDocBs, const PBowDocWgtBs& BowDocWgtBs, const PBowSim& BowSim,
 const int& MnCatDocs){
  // prepare cat-ids from names
  int RootCatId=GetCatId(RootCatNm);
  TIntV PosCatIdV; GetCatIdV(PosCatNmV, PosCatIdV);
  TIntV NegCatIdV; GetCatIdV(NegCatNmV, NegCatIdV);
  // prepare categories
  TIntV CatIdV; GetSubTreeCatIdV(RootCatId, PosCatIdV, NegCatIdV, CatIdV);
  int Cats=CatIdV.Len(); int CatN=0; CatIdV.Clr();
  // create document partition
  PBowDocPart DocPart=TBowDocPart::New();
  DocPart->PutNm("Root");

  printf("getting the main cluster ...\n");
  // get main-cluster
  PBowDocPartClust DocPartClust=GetBowDocPartClust(
   RootCatId, PosCatIdV, NegCatIdV, BowDocBs, BowDocWgtBs, BowSim, MnCatDocs, Cats, CatN);
  if (DocPartClust->GetDocs()>=MAX(1, MnCatDocs)){
    DocPart->AddClust(DocPartClust);}
  // return document partition
  return DocPart;
}

void TDMozBs::_GetBestClustV(
 const PBowDocPart& BowDocPart, const PBowSim& BowSim,
 const PBowSpV DocBowSpV, TFltBowDocPartClustKdV& WgtClustKdV){
  for (int ClustN=0; ClustN<BowDocPart->GetClusts(); ClustN++){
    PBowDocPartClust Clust=BowDocPart->GetClust(ClustN);
    IAssert(Clust->IsConceptSpV());
    double Sim=BowSim->GetCosSim(DocBowSpV, Clust->GetConceptSpV());
    if (Sim>0){
      WgtClustKdV.Add(TFltBowDocPartClustKd(Sim, Clust));}
    if (Clust->IsSubPart()){
      _GetBestClustV(Clust->GetSubPart(), BowSim, DocBowSpV, WgtClustKdV);
    }
  }
}

void TDMozBs::_GetBestWordVV(
 TFltBowDocPartClustKdV& WgtClustKdV, const PBowSim& BowSim,
 const PBowSpV DocBowSpV, TVec<TFltIntPrV>& WgtWIdPrVV){
  WgtWIdPrVV.Clr();
  for (int ClustN=0; ClustN<WgtClustKdV.Len(); ClustN++){
    PBowDocPartClust Clust=WgtClustKdV[ClustN].Dat;
    TFltIntPrV WgtWIdPrV;
    BowSim->GetCosSim(DocBowSpV, Clust->GetConceptSpV(), WgtWIdPrV);
    WgtWIdPrV.Sort(false);
    double WgtSum=0;
    for (int WIdN=0; WIdN<WgtWIdPrV.Len(); WIdN++){WgtSum+=WgtWIdPrV[WIdN].Val1;}
    double CutWgtSum=WgtSum*0.9; double WgtSumSF=0;
    for (int WIdN=0; WIdN<WgtWIdPrV.Len(); WIdN++){
      WgtSumSF+=WgtWIdPrV[WIdN].Val1;
      WgtWIdPrV[WIdN].Val1=WgtWIdPrV[WIdN].Val1/WgtSum;
      if (WgtSumSF>CutWgtSum){WgtWIdPrV.Trunc(WIdN+1); break;}
    }
    WgtWIdPrVV.Add(WgtWIdPrV);
  }
}

void TDMozBs::GetBestClustV(
 const PBowDocPart& BowDocPart, const PBowSim& BowSim,
 const PBowSpV DocBowSpV, const int& MxCats,
 TFltBowDocPartClustKdV& WgtClustKdV, TVec<TFltIntPrV>& WgtWIdPrVV){
  WgtClustKdV.Clr();
  _GetBestClustV(BowDocPart, BowSim, DocBowSpV, WgtClustKdV);
  WgtClustKdV.Sort(false);
  WgtClustKdV.Trunc(MxCats);
  _GetBestWordVV(WgtClustKdV, BowSim, DocBowSpV, WgtWIdPrVV);
}

void TDMozBs::GetBestKWordV(
 const TFltBowDocPartClustKdV& WgtClustKdV, const double& TopWgtSumPrc,
 const bool& AddLevP, const int& MnLev, const int& MxLev,
 TStrFltPrV& KWordStrWgtPrV){
  // collect keywords
  TStrIntH KWordStrToFqH(WgtClustKdV.Len());
  TStrFltH KWordStrToWgtH(WgtClustKdV.Len());
  for (int CatN=0; CatN<WgtClustKdV.Len(); CatN++){
    // get category weight & name
    double CatWgt=WgtClustKdV[CatN].Key;
    PBowDocPartClust Clust=WgtClustKdV[CatN].Dat;
    TStr CatNm=Clust->GetNm();
    // split category-name to keywords
    TStrV CatKWordStrV;
    CatNm.SplitOnAllCh('/', CatKWordStrV);
    // put keywords to hash-table
    for (int CatKWordStrN=0; CatKWordStrN<CatKWordStrV.Len(); CatKWordStrN++){
      TStr KWordStr=CatKWordStrV[CatKWordStrN];
      if (KWordStr=="Top"){continue;}
      if (AddLevP){
        KWordStr+=TStr::Fmt("-%d", CatKWordStrN);}
      if (((MnLev==-1)||(MnLev<=CatKWordStrN))&&((MxLev==-1)||(CatKWordStrN<=MxLev))){
        KWordStrToFqH.AddDat(KWordStr)++;
        const int KWordFq=KWordStrToFqH.AddDat(KWordStr);
        KWordStrToWgtH.AddDat(KWordStr)+=CatWgt/KWordFq;
      }
    }
  }
  // prepare weight-keywords-vector and weight-sum
  TFltStrPrV WgtKWordStrV; KWordStrToWgtH.GetDatKeyPrV(WgtKWordStrV);
  double KWordWgtSum=0;
  for (int KWordN=0; KWordN<WgtKWordStrV.Len(); KWordN++){
    KWordWgtSum+=WgtKWordStrV[KWordN].Val1;}
  // select top-keywords
  double KWordWgtSumSF=0;
  WgtKWordStrV.Sort(false); KWordStrWgtPrV.Clr();
  for (int KWordN=0; KWordN<WgtKWordStrV.Len(); KWordN++){
    // prepare keyword-string & weight
    TStr KWordStr=WgtKWordStrV[KWordN].Val2;
    double KWordWgt=WgtKWordStrV[KWordN].Val1;
    double NrKWordWgt=KWordWgt;
    if (KWordWgtSum>0){NrKWordWgt=KWordWgt/KWordWgtSum;}
    // add keyword-string & weight pair
    KWordStrWgtPrV.Add(TStrFltPr(KWordStr, NrKWordWgt));
    // sum wights and finish if over sum-treshold
    KWordWgtSumSF+=KWordWgt;
    if ((KWordWgtSum>0)&&(KWordWgtSumSF/KWordWgtSum>TopWgtSumPrc)){break;}
  }
}

PDMozBs TDMozBs::LoadTxt(const TStr& FPath,
 const bool& StructOnlyP, const bool& SampleDataP,
 const uint64& ExtPgStrPoolLen){
  // create DMoz
  PDMozBs DMozBs=TDMozBs::New(ExtPgStrPoolLen);

  // prepare file-names
  TStr DMozNrFPath=TStr::GetNrFPath(FPath); // normalize path
  TStr DMozStructFNm=DMozNrFPath+TDMozInfo::RdfStructFBase;
  TStr DMozContFNm=DMozNrFPath+TDMozInfo::RdfContentFBase;
  if (SampleDataP){
    DMozStructFNm=DMozNrFPath+TDMozInfo::RdfStructSampleFBase;
    DMozContFNm=DMozNrFPath+TDMozInfo::RdfContentSampleFBase;
  }

  printf("structure file `%s`\n", DMozStructFNm.CStr());
  printf("content file `%s`\n", DMozContFNm.CStr());

  // aliases
  THash<TMd5Sig, TDMozAlias> SymMd5ToAliasH;
  TStrPool AliasStrPool;

  // create topic & alias entries
  {printf("Create topic & alias entries ...\n");
  PSIn DMozStructSIn=TFIn::New(DMozStructFNm);
  printf("skipping top tag\n");
  TXmlDoc::SkipTopTag(DMozStructSIn);
  PXmlDoc XmlDoc; int XmlDocs=0;
  forever{
    // load xml tree
    XmlDocs++;
    if (XmlDocs%1000==0){
      printf("%d Docs   %d Topics   %d Aliases   %s Cat-Chars\r",
       XmlDocs, DMozBs->CatMd5ToTopicH.Len(), SymMd5ToAliasH.Len(),
       TUInt64::GetMegaStr(uint64(DMozBs->CatNmPool.Len())).CStr());
    }
    XmlDoc=TXmlDoc::LoadTxt(DMozStructSIn);

    if (!XmlDoc->IsOk()){
        printf("%s\n", XmlDoc->GetMsgStr().CStr());
        break;
    }

    // extract fields from xml-trees
    PXmlTok TopTok=XmlDoc->GetTok();
    if (TopTok->IsTag("Topic")){
      // extract field
      TStr CatNm=TopTok->GetArgVal("r:id");
      //printf("%s\r", CatNm.CStr());
      // create entry
      TMd5Sig CatMd5(CatNm);
      //IAssert(!DMozBs->CatMd5ToTopicH.IsKey(CatMd5)); - old version
      if (!DMozBs->CatMd5ToTopicH.IsKey(CatMd5)){
        TDMozTopic& DMozTopic=DMozBs->CatMd5ToTopicH.AddDat(CatMd5);
        DMozTopic.CatSId=DMozBs->CatNmPool.AddStr(CatNm);
      }
    } else
    if (TopTok->IsTag("Alias")){
      // extract field
      TStr SymStr=TopTok->GetArgVal("r:id");
      TStr TitleStr=TopTok->GetTagTok("d:Title")->GetTokStr(false);
      TStr SubstStr=TopTok->GetTagTok("Target")->GetArgVal("r:resource");
      // create entry
      TMd5Sig SymMd5(SymStr);
      // IAssert(!SymMd5ToAliasH.IsKey(SymMd5)); - old version
      if (!SymMd5ToAliasH.IsKey(SymMd5)){
        TDMozAlias& DMozAlias=SymMd5ToAliasH.AddDat(SymMd5);
        DMozAlias.SymSId=AliasStrPool.AddStr(SymStr);
        DMozAlias.SubstSId=AliasStrPool.AddStr(SubstStr);
      }
    } else {
      Fail;
    }
  }
  printf("\n... Done.\n");}

  // create sub-categories
  {printf("Create sub-topics ...\n");
  PSIn DMozStructSIn=TFIn::New(DMozStructFNm);
  TXmlDoc::SkipTopTag(DMozStructSIn);
  PXmlDoc XmlDoc; int XmlDocs=0;
  forever{
    // load xml tree
    XmlDocs++; if (XmlDocs%1000==0){printf("%d Docs\r", XmlDocs);}
    XmlDoc=TXmlDoc::LoadTxt(DMozStructSIn);
    if (!XmlDoc->IsOk()){
        printf("%s\n", XmlDoc->GetMsgStr().CStr());
        break;
    }
    // extract fields from xml-trees
    PXmlTok TopTok=XmlDoc->GetTok();
    if (TopTok->IsTag("Topic")){
      // extract field
      TStr CatNm=TopTok->GetArgVal("r:id");
      //TInt CatId=TopTok->GetTagTok("catid")->GetTokStr(false).GetInt();
      //TStr TitleStr=TopTok->GetTagTok("d:Title")->GetTokStr(false);
      //TStr DescStr; PXmlTok DescTok;
      //if (TopTok->IsSubTag("d:Description", DescTok)){
      //  DescStr=DescTok->GetTokStr(false);}
      // get entry
      TMd5Sig CatMd5(CatNm);
      IAssert(DMozBs->CatMd5ToTopicH.IsKey(CatMd5));
      TDMozTopic& DMozTopic=DMozBs->CatMd5ToTopicH.GetDat(CatMd5);
      // prepare sub-categories vector
      DMozTopic.SubCatTypeIdVN=DMozBs->SubCatTypeIdPrV.Len();
      // extract sub-branches
      for (int SubTokN=0; SubTokN<TopTok->GetSubToks(); SubTokN++){
        // get sub-category name & type
        PXmlTok SubTok=TopTok->GetSubTok(SubTokN);
        if (!SubTok->IsTag()){continue;}
        TStr SubTagNm=SubTok->GetTagNm();
        TDMozSubCatType SubCatType=GetSubCatType(SubTagNm);
        // add sub category
        if (SubCatType!=dmsctUndef){
          // get sub-category name
          TStr SubCatNm=SubTok->GetArgVal("r:resource");
          // hash sub-category name
          TMd5Sig SubCatMd5(SubCatNm);
          // get sub-category-id in topics or aliases
          int SubCatId=DMozBs->CatMd5ToTopicH.GetKeyId(SubCatMd5);
          if (SubCatId==-1){
            int AliasId=SymMd5ToAliasH.GetKeyId(SubCatMd5);
            if (AliasId!=-1){
              // get sub-category-name as substitution for alias
              int SubstSId=SymMd5ToAliasH[AliasId].SubstSId;
              SubCatNm=AliasStrPool.GetStr(SubstSId);
              // get sub-category hash & id
              SubCatMd5=TMd5Sig(SubCatNm);
              SubCatId=DMozBs->CatMd5ToTopicH.GetKeyId(SubCatMd5);
            } else {
              //printf("Undefined category: '%s'\n", SubCatNm.CStr());
              //Fail;
            }
          }
          // get sub-category (type, id) pair
          if (SubCatId!=-1){
            uint SubCatTypeIdPr=GetSubCatTypeIdPr(SubCatType, SubCatId);
            DMozBs->SubCatTypeIdPrV.Add(SubCatTypeIdPr);
          }
        }
      }
      // conclude sub-categories vector (add undef sub-category)
      if (DMozTopic.SubCatTypeIdVN==DMozBs->SubCatTypeIdPrV.Len()){
        DMozTopic.SubCatTypeIdVN=-1; // if no sub-categories available
      } else {
        DMozBs->SubCatTypeIdPrV.Add(GetSubCatTypeIdPr(dmsctUndef));
      }
    } else
    if (TopTok->IsTag("Alias")){
    } else {
      Fail;
    }
  }
  printf("\n... Done.\n");}

  // clear aliases
  SymMd5ToAliasH.Clr();
  AliasStrPool.Clr();

  int MissingCatN = 0;

  if (!StructOnlyP){
    // create external-urls
    {printf("Create external-urls ...\n");
    PSIn DMozContSIn=TFIn::New(DMozContFNm);
    TXmlDoc::SkipTopTag(DMozContSIn);
    PXmlDoc XmlDoc; int XmlDocs=0; int Dupls=0;
    forever{
      // load xml tree
      XmlDocs++;
      if (XmlDocs%1000==0){
        printf("%d Docs   %d Duplicates   %d Ext-Urs   %s Ext-Page-Chars\r",
         XmlDocs, Dupls,
         DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH.Len(),
         TUInt64::GetMegaStr(uint64(DMozBs->ExtPgStrPool.Len())).CStr());
      }
      XmlDoc=TXmlDoc::LoadTxt(DMozContSIn);

      if (!XmlDoc->IsOk()){break;}
      // extract fields from xml-trees
      PXmlTok TopTok=XmlDoc->GetTok();
      if (TopTok->IsTag("Topic")){
        // extract category name
        TStr CatNm=TopTok->GetArgVal("r:id");
        // get category entry
        TMd5Sig CatMd5(CatNm);
        // if(!DMozBs->CatMd5ToTopicH.IsKey(CatMd5)) {printf("%s", CatNm.CStr());}
        /* IAssertR(DMozBs->CatMd5ToTopicH.IsKey(CatMd5), "Category not present `" + CatNm + "`"); */
        if (!DMozBs->CatMd5ToTopicH.IsKey(CatMd5)) {
            printf("category missing `%s`, ignoring\n", CatNm.CStr());
            ++MissingCatN;
            continue;
        }
        TDMozTopic& DMozTopic=DMozBs->CatMd5ToTopicH.AddDat(CatMd5);
        // prepare external-urls vector
        DMozTopic.ExtUrlIdVN=DMozBs->ExtUrlIdV.Len();
        // extract external links
        for (int SubTokN=0; SubTokN<TopTok->GetSubToks(); SubTokN++){
          // get sub-category name & type
          PXmlTok SubTok=TopTok->GetSubTok(SubTokN);
          if (!SubTok->IsTag()){continue;}
          TStr SubTagNm=SubTok->GetTagNm();
          if ((SubTagNm!="link")&&(SubTagNm!="link1")){continue;}
          // get external-url-string
          TStr ExtUrlStr=SubTok->GetArgVal("r:resource");
          // add external-url-string to pool
          int ExtUrlSId=DMozBs->ExtPgStrPool.AddStr(ExtUrlStr);
          // create external-url entry
          TMd5Sig ExtUrlMd5(ExtUrlStr); int ExtUrlId=-1;
          if (DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH.IsKey(ExtUrlMd5, ExtUrlId)){
            Dupls++;
            //printf("Duplicate external url: '%s'\n", ExtUrlStr.CStr());
          } else {
            ExtUrlId=DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH.AddKey(ExtUrlMd5);
            DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH[ExtUrlId]=
             TIntTr(ExtUrlSId, -1, -1);
          }
          // add external-url-id to vectors pool
          DMozBs->ExtUrlIdV.Add(ExtUrlId);
        }
        // conclude external-urls vector
        if (DMozTopic.ExtUrlIdVN==DMozBs->ExtUrlIdV.Len()){
          DMozTopic.ExtUrlIdVN=-1; // if no external urls available
        } else {
          DMozBs->ExtUrlIdV.Add(-1);
        }
      } else
      if (TopTok->IsTag("ExternalPage")){
        // extract field values
        TStr ExtUrlStr=TopTok->GetArgVal("about");
        TStr TitleStr; PXmlTok TitleTok;
        if (TopTok->IsSubTag("d:Title", TitleTok)){
          TitleStr=TitleTok->GetTokStr(false);}
        TStr DescStr; PXmlTok DescTok;
        if (TopTok->IsSubTag("d:Description", DescTok)){
          DescStr=DescTok->GetTokStr(false);}
        // get external-url entry
        TMd5Sig ExtUrlMd5(ExtUrlStr);
        int ExtUrlKId=DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH.GetKeyId(ExtUrlMd5);
        if (ExtUrlKId!=-1){
          int UrlSId=DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH[ExtUrlKId].Val1;
          int TitleSId=DMozBs->ExtPgStrPool.AddStr(TitleStr);
          int DescSId=DMozBs->ExtPgStrPool.AddStr(DescStr);
          DMozBs->ExtUrlMd5ToUrlTitleDescSIdTrH[ExtUrlKId]=
           TIntTr(UrlSId, TitleSId, DescSId);
        } else {
          printf("Undefined external url: '%s'\n", ExtUrlStr.CStr());
        }
      } else {
        Fail;
      }
    }}
  }

  if (MissingCatN > 0) {
      printf("number of categories missing in the structure: %d\n", MissingCatN);
  }

  // return DMoz
  return DMozBs;
}

PDMozBs TDMozBs::LoadBin(const TStr& FBase, const TStr& FPath){
  printf("Loading DMoz-Binary '%s' from '%s' ...", FBase.CStr(), FPath.CStr());
  TStr FNm=TStr::GetNrFPath(FPath)+FBase;
  TFIn SIn(FNm);
  PDMozBs DMozBs=new TDMozBs(SIn);
  printf(" Done.\n");
  return DMozBs;
}

void TDMozBs::SaveBin(const TStr& FBase, const TStr& FPath){
  TStr FNm=TStr::GetNrFPath(FPath)+FBase;
  TFOut SOut(FNm); Save(SOut);
}

void TDMozBs::SaveLnDocTxt(const PDMozBs& DMozBs, const TStr& RootCatNm,
 const TStr& FBase, const TStr& FPath, const bool& SaveMd5P){
  TStr FNm=TStr::GetNrFPath(FPath)+FBase;
  printf("Save DMoz-Binary to %s ...\n", FNm.CStr());
  TFOut FOut(FNm); FILE* fOut=FOut.GetFileId();
  // traverse categories
  int Cats=DMozBs->GetCats(); int SavedCats=0;
  for (int CatId=0; CatId<Cats; CatId++){
    TStr CatNm=DMozBs->GetCatNm(CatId);
    if ((!RootCatNm.Empty())&&(!CatNm.StartsWith(RootCatNm))){continue;}
    SavedCats++;
    if (SavedCats%1000==0){printf("  Saved Categories %d\r", SavedCats);}
    // external-urls
    TIntV ExtUrlIdV; DMozBs->GetExtUrlIdV(CatId, ExtUrlIdV);
    for (int ExtUrlIdN=0; ExtUrlIdN<ExtUrlIdV.Len(); ExtUrlIdN++){
      int ExtUrlId=ExtUrlIdV[ExtUrlIdN];
      TStr Md5Str=DMozBs->GetExtUrlMd5Str(ExtUrlId);
      TStr UrlStr=DMozBs->GetExtUrlStr(ExtUrlId);
      TStr TitleStr=DMozBs->GetExtUrlTitleStr(ExtUrlId);
      TStr DescStr=DMozBs->GetExtUrlDescStr(ExtUrlId);
      if (SaveMd5P){
        fprintf(fOut, "%s %s -- %s\n", Md5Str.CStr(), TitleStr.CStr(), DescStr.CStr());
      } else {
        fprintf(fOut, "%s -- %s\n", TitleStr.CStr(), DescStr.CStr());
      }
    }
  }
  printf("  Saved Categories %d\n", SavedCats);
  printf("Done.\n");
}

void TDMozBs::SaveTxt(const PDMozBs& DMozBs, const TStr& RootCatNm,
 const TStr& FBase, const TStr& FPath){
  TStr FNm=TStr::GetNrFPath(FPath)+FBase;
  printf("Dump DMoz-Binary to %s ...\n", FNm.CStr());
  TFOut FOut(FNm); FILE* fOut=FOut.GetFileId();
  // traverse categories
  int Cats=DMozBs->GetCats(); int SavedCats=0;
  fprintf(fOut, "#Categories: %d\n", Cats);
  for (int CatId=0; CatId<Cats; CatId++){
    TStr CatNm=DMozBs->GetCatNm(CatId);
    if ((!RootCatNm.Empty())&&(!CatNm.StartsWith(RootCatNm))){continue;}
    SavedCats++;
    if (SavedCats%1000==0){printf("  Saved Categories %d\r", SavedCats);}
    fprintf(fOut, "cat: '%s'\n", CatNm.CStr());
    // subcategories
    TUIntV SubCatTypeIdPrV; DMozBs->GetSubCatTypeIdPrV(CatId, SubCatTypeIdPrV);
    for (int SubCatN=0; SubCatN<SubCatTypeIdPrV.Len(); SubCatN++){
      // get subcategory data
      TDMozSubCatType SubCatType; int SubCatId;
      TDMozBs::GetSubCatTypeCatId(SubCatTypeIdPrV[SubCatN], SubCatType, SubCatId);
      // get strings
      TStr SubCatTypeStr=TDMozBs::GetSubCatTypeNm(SubCatType);
      TStr SubCatNm=DMozBs->GetCatNm(SubCatId);
      // output subcategory
      fprintf(fOut, "scat:   [%s] -- '%s'\n", SubCatTypeStr.CStr(), SubCatNm.CStr());
    }
    // external-urls
    TIntV ExtUrlIdV; DMozBs->GetExtUrlIdV(CatId, ExtUrlIdV);
    for (int ExtUrlIdN=0; ExtUrlIdN<ExtUrlIdV.Len(); ExtUrlIdN++){
      int ExtUrlId=ExtUrlIdV[ExtUrlIdN];
      TStr UrlStr=DMozBs->GetExtUrlStr(ExtUrlId);
      TStr TitleStr=DMozBs->GetExtUrlTitleStr(ExtUrlId);
      TStr DescStr=DMozBs->GetExtUrlDescStr(ExtUrlId);
      fprintf(fOut, "exut:   [%s] -- %s - %s\n", UrlStr.CStr(), TitleStr.CStr(), DescStr.CStr());
      //fprintf(fOut, "   External: '%s' - '%s'\n", TitleStr.CStr(), DescStr.CStr());
    }
  }
  printf("  Saved Categories %d\n", SavedCats);

  if (RootCatNm.Empty()){
    // external urls
    int ExtUrls=DMozBs->GetExtUrls();
    fprintf(fOut, "#External-Urls: %d\n", ExtUrls);
    for (int ExtUrlId=0; ExtUrlId<ExtUrls; ExtUrlId++){
      if (ExtUrlId%1000==0){printf("  Saved External URLs %d\r", ExtUrlId);}
      TStr UrlStr=DMozBs->GetExtUrlStr(ExtUrlId);
      TStr TitleStr=DMozBs->GetExtUrlTitleStr(ExtUrlId);
      TStr DescStr=DMozBs->GetExtUrlDescStr(ExtUrlId);
      //fprintf(fOut, "%s - '%s'\n", UrlStr.CStr(), TitleStr.CStr());
      //fprintf(fOut, "'%s' - '%s'\n", TitleStr.CStr(), DescStr.CStr());
      fprintf(fOut, "exutd: [%s] %s -- %s\n", UrlStr.CStr(), TitleStr.CStr(), DescStr.CStr());
    }
    printf("  Saved External URLs %d\n", ExtUrls);
  }
  printf("Done.\n");
}

void TDMozBs::SaveSampleTxt(const PDMozBs& DMozBs, const TStr& RootCatNm,
 const TStr& FBase, const TStr& FPath,
 const int& SampleRndSeed, const int& SampleSize,
 const bool& SampleSiteUrlP, const bool& SampleStructP){
  TStrH UrlStrH;
  if (SampleStructP){
    // structured sampling
    int RootCatId=DMozBs->GetCatId(RootCatNm);
    TRnd Rnd(SampleRndSeed);
    while (UrlStrH.Len()<SampleSize){
      int UrlId=DMozBs->GetRndExtUrlId(RootCatId, Rnd);
      if (UrlId==-1){continue;}
      TStr UrlStr=DMozBs->GetExtUrlStr(UrlId);
      if (SampleSiteUrlP){
        if (TUrl::IsSite(UrlStr)){
          UrlStrH.AddKey(UrlStr);}
      } else {
        UrlStrH.AddKey(UrlStr);
      }
    }
  } else {
    // uniform sampling
    TStrV PosCatNmV; TStrV NegCatNmV; TIntV CatIdV;
    DMozBs->GetSubTreeCatIdV(RootCatNm, PosCatNmV, NegCatNmV, CatIdV);
    TIntV ExtUrlIdV; DMozBs->GetExtUrlIdV(CatIdV, ExtUrlIdV);
    TRnd Rnd(SampleRndSeed); ExtUrlIdV.Shuffle(Rnd);
    for (int ExtUrlIdN=0; ExtUrlIdN<ExtUrlIdV.Len(); ExtUrlIdN++){
      int UrlId=ExtUrlIdV[ExtUrlIdN];
      TStr UrlStr=DMozBs->GetExtUrlStr(UrlId);
      if (SampleSiteUrlP){
        if (TUrl::IsSite(UrlStr)){
          UrlStrH.AddKey(UrlStr);}
      } else {
        UrlStrH.AddKey(UrlStr);
      }
      if (UrlStrH.Len()>=SampleSize){break;}
    }
  }
  // save sample to file
  {TStr FNm=TStr::GetNrFPath(FPath)+FBase;
  printf("Save Sample to %s ...\n", FNm.CStr());
  TFOut FOut(FNm); FILE* fOut=FOut.GetFileId();
  for (int UrlN=0; UrlN<UrlStrH.Len(); UrlN++){
    fprintf(fOut, "%s\n", UrlStrH.GetKey(UrlN).CStr());
  }}
}

TDMozSubCatType TDMozBs::GetSubCatType(const TStr& CatTypeNm){
  if (CatTypeNm=="narrow"){return dmsctNarrow;}
  if (CatTypeNm=="narrow1"){return dmsctNarrow;}
  if (CatTypeNm=="narrow2"){return dmsctNarrow;}
  if (CatTypeNm=="letterbar"){return dmsctLetterbar;}
  if (CatTypeNm=="related"){return dmsctRelated;}
  if (CatTypeNm=="symbolic"){return dmsctSymbolic;}
  else {return dmsctUndef;}
}

TStr TDMozBs::GetSubCatTypeNm(const TDMozSubCatType& SubCatType){
  switch (SubCatType){
    case dmsctUndef: return "Undef";
    case dmsctNarrow: return "Narrow";
    case dmsctLetterbar: return "Letterbar";
    case dmsctRelated: return "Related";
    case dmsctSymbolic: return "Symbolic";
    default: Fail; return TStr();
  }
}

uint TDMozBs::GetSubCatTypeIdPr(
 const TDMozSubCatType& SubCatType, const int& CatId){
  IAssert((CatId & 0xFF000000)==0);
  uint SubCatTypeIdPr=(SubCatType<<24)+CatId;
  return SubCatTypeIdPr;
}

void TDMozBs::GetSubCatTypeCatId(
 const uint& SubCatTypeIdPr, TDMozSubCatType& SubCatType, int& CatId){
  SubCatType=TDMozSubCatType((SubCatTypeIdPr & 0xFF000000)>>24);
  CatId=SubCatTypeIdPr & 0x00FFFFFF;
}

////////////////////////////////////////////
// DMoz-Classifier
TDMozCfy::TDMozCfy(PBowDocBs _BowDocBs, PBowDocPart _BowDocPart, const int& MnClustDocs) {
    // extract vocabulary
    VocBowDocBs = _BowDocBs->GetVocBs();
    // extract IDF information
    PBowDocWgtBs BowDocWgtBs = TBowDocWgtBs::New(_BowDocBs, bwwtLogDFNrmTFIDF);
    Docs = BowDocWgtBs->GetDocs();
    WordFqV = BowDocWgtBs->GetWordFqV();
    // prepare a bowpart with only prefered categories
    BowDocPart = TBowDocPart::New();
    TSStack<PBowDocPartClust> ClustS;
    for (int ClustN = 0; ClustN < _BowDocPart->GetClusts(); ClustN++) {
        ClustS.Push(_BowDocPart->GetClust(ClustN));
    }
    while (!ClustS.Empty()) {
        PBowDocPartClust Clust = ClustS.Top(); ClustS.Pop();
        // check if big enough
        if (Clust->GetDocs() < MnClustDocs) { continue; }
        // put name to cluster if empty
        if (Clust->GetNm().Empty()) {
            PBowKWordSet ClustKWordSet = Clust->GetConceptSpV()->GetKWordSet(_BowDocBs);
            Clust->PutNm(ClustKWordSet->GetTopKWords(3, 1.0)->GetKWordsStr());
        }
        // remember the cluster
        BowDocPart->AddClust(TBowDocPartClust::New(_BowDocBs, Clust->GetNm(),
            Clust->GetQual(), TIntV(), Clust->GetConceptSpV(), NULL));
        // check for any subclusters
        if (Clust->IsSubPart()) {
            PBowDocPart SubPart = Clust->GetSubPart();
            for (int ClustN = 0; ClustN < SubPart->GetClusts(); ClustN++) {
                ClustS.Push(SubPart->GetClust(ClustN));
            }
        }
    }
    // prepare similarity measure
    BowSim = TBowSim::New(bstCos);
}

TDMozCfy::TDMozCfy(PBowDocBs _BowDocBs, PBowDocPart _BowDocPart, const TStr& CatListFNm) {
    // extract vocabulary
    VocBowDocBs = _BowDocBs->GetVocBs();
    // extract IDF information
    PBowDocWgtBs BowDocWgtBs = TBowDocWgtBs::New(_BowDocBs, bwwtLogDFNrmTFIDF);
    Docs = BowDocWgtBs->GetDocs();
    WordFqV = BowDocWgtBs->GetWordFqV();
    // load a list of prefered categories
    TFIn CatListFIn(CatListFNm);
    TStrIntPrV PrefixV; TStrPrV SuffixV; TStrV IgnoreV; TStrH CatNmH; TStr LnStr;
    while (CatListFIn.GetNextLn(LnStr)) {
        if (LnStr.IsWs()) { continue;  }
        else if (LnStr.IsChIn('\t')) {
            TStr Prefix, MxLevelStr;
            LnStr.SplitOnCh(Prefix, '\t', MxLevelStr);
            PrefixV.Add(TStrIntPr(Prefix, MxLevelStr.GetInt()));
        } else if (LnStr.IsChIn('*')) {
            SuffixV.Add(TStrPr(LnStr.LeftOf('*'), LnStr.RightOf('*')));
        } else if (LnStr[0] == '-') {
            printf("Ignore: %s\n", LnStr.CStr());
            IgnoreV.Add(LnStr.GetSubStr(1));
        } else {
            CatNmH.AddDat(LnStr);
        }
    }
    // prepare a bowpart with only prefered categories
    BowDocPart = TBowDocPart::New();
    TSStack<PBowDocPartClust> ClustS;
    for (int ClustN = 0; ClustN < _BowDocPart->GetClusts(); ClustN++) {
        ClustS.Push(_BowDocPart->GetClust(ClustN));
    }
    int GoodClusts = 0;
    while (!ClustS.Empty()) {
        PBowDocPartClust Clust = ClustS.Top(); ClustS.Pop();
        TStr ClustNm = Clust->GetNm();
        // we have a black list
        bool IgnoreP = false;
        for (int IgnoreN = 0; IgnoreN < IgnoreV.Len(); IgnoreN++) {
            const TStr& Ignore = IgnoreV[IgnoreN];
            if (ClustNm.StartsWith(Ignore)) { IgnoreP = true; break; }
        }
        // if category is black-listed then skip it and all it's children
        if (IgnoreP) { continue; }

        // by default we accept
        bool AddP = true;
        // check if we have a white list
        if (!PrefixV.Empty() || !SuffixV.Empty() || !CatNmH.Empty()) {
            // we have a white list, by default all is bad
            AddP = false;
            // check if we match any prefix within the max level limit
            const int ClustLevel = ClustNm.CountCh('/');
            for (int PrefixN = 0; PrefixN < PrefixV.Len(); PrefixN++) {
                const TStr& Prefix = PrefixV[PrefixN].Val1;
                const int MxLevel = PrefixV[PrefixN].Val2;
                if (ClustNm.StartsWith(Prefix) &&  ClustLevel <= MxLevel) {
                    // we do, put the flag on
                    AddP = true;
                }
            }
            // check if we match any suffix
            for (const TStrPr& Suffix : SuffixV) {
                if (ClustNm.StartsWith(Suffix.Val1) && ClustNm.EndsWith(Suffix.Val2)) {
                    AddP = true;
                }
            }
            // check if we are in the explicit white list
            if (CatNmH.IsKey(ClustNm)) {
                AddP = true;
                CatNmH.GetDat(ClustNm)++;
            }
        }

        if (AddP) {
            BowDocPart->AddClust(TBowDocPartClust::New(_BowDocBs, ClustNm,
                Clust->GetQual(), TIntV(), Clust->GetConceptSpV(), NULL));
            GoodClusts++;
        }

        // check children
        if (Clust->IsSubPart()) {
            PBowDocPart SubPart = Clust->GetSubPart();
            for (int ClustN = 0; ClustN < SubPart->GetClusts(); ClustN++) {
                ClustS.Push(SubPart->GetClust(ClustN));
            }
        }
    }
    for (const auto& CatNmFq : CatNmH) {
        if (CatNmFq.Dat == 0) {
            printf("no instances of %s found\n", CatNmFq.Key.CStr());
        }
    }
    printf("Loaded %d DMoz clusters\n", GoodClusts);
    // prepare similarity measure
    BowSim = TBowSim::New(bstCos);
}

TDMozCfy::TDMozCfy(TSIn& SIn) {
    VocBowDocBs = TBowDocBs::Load(SIn);
    Docs.Load(SIn); WordFqV.Load(SIn);
    BowDocPart = TBowDocPart::Load(SIn);
    BowSim = TBowSim::New(bstCos);
}

void TDMozCfy::Save(TSOut& SOut) const {
    VocBowDocBs->Save(SOut);
    Docs.Save(SOut); WordFqV.Save(SOut);
    BowDocPart->Save(SOut);
}

void TDMozCfy::Classify(const TStr& HtmlStr, TStrFltKdV& CatNmWgtV,
        TStrFltKdV& KeyWdWgtV, const int& MxCats) {

    // create bow sparse vector
    PBowSpV QueryBowSpV = VocBowDocBs->GetSpVFromHtmlStr(HtmlStr, Docs, WordFqV);
    // classify
    TFltBowDocPartClustKdV WgtClustKdV; TVec<TFltIntPrV> WgtWIdPrVV; TStrFltPrV KWordStrWgtPrV;
    TDMozBs::GetBestClustV(BowDocPart, BowSim, QueryBowSpV, MxCats, WgtClustKdV, WgtWIdPrVV);
    TDMozBs::GetBestKWordV(WgtClustKdV, 0.5, false, -1, -1, KWordStrWgtPrV);
    // output categories
    CatNmWgtV.Clr();
    for (int CatClustN = 0; CatClustN < WgtClustKdV.Len(); CatClustN++){
        PBowDocPartClust Clust = WgtClustKdV[CatClustN].Dat;
        TStr CatNm = Clust->GetNm();
        double CatWgt = WgtClustKdV[CatClustN].Key;
        CatNmWgtV.Add(TStrFltKd(CatNm, CatWgt));
    }
    // output keywords
    KeyWdWgtV.Clr();
    for (int KWordN = 0; KWordN < KWordStrWgtPrV.Len(); KWordN++){
        TStr KWordStr = KWordStrWgtPrV[KWordN].Val1;
        double KWordWgt = KWordStrWgtPrV[KWordN].Val2;
        KeyWdWgtV.Add(TStrFltKd(KWordStr, KWordWgt));
    }
}

PDMozCfy TDMozCfy::LoadFPath(const TStr& DMozFPath, const int& MnClustDocs) {
    if (TFile::Exists(DMozFPath + "DMoz.dat")) {
        printf("Reading DMoz.dat. Note: DMoz.txt will be ignored. Delete the .dat file to rebuild using settings in DMoz.txt.\n");
        return TDMozCfy::LoadBin(DMozFPath + "DMoz.dat");
    } else if (TFile::Exists(DMozFPath + "Top.Bow") && TFile::Exists(DMozFPath + "Top.BowPart"))  {
        printf("Preparing DMoz.dat ...\n");
        PBowDocBs BowDocBs = TBowDocBs::LoadBin(DMozFPath + "Top.Bow");
        PBowDocPart BowDocPart = TBowDocPart::LoadBin(DMozFPath + "Top.BowPart");
        printf("Checking DMoz.txt for valid categories ...\n");
        PDMozCfy DMoz = TFile::Exists(DMozFPath + "DMoz.txt") ?
            TDMozCfy::New(BowDocBs, BowDocPart, DMozFPath + "DMoz.txt") :
            TDMozCfy::New(BowDocBs, BowDocPart);
        DMoz->SaveBin(DMozFPath + "DMoz.dat");
        return DMoz;
    } else {
        return NULL;
    }
}

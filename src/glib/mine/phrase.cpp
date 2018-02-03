/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// NGram-Base
TStr TNGramBs::GetWIdVStr(const TIntV& WIdV) const {
  TChA ChA;
  for (int WIdN=0; WIdN<WIdV.Len(); WIdN++){
    if (WIdN>0){ChA+=' '/*'_'*/;} ChA+=GetWordStr(WIdV[WIdN]);}
  return ChA;
}

TStr TNGramBs::GetNGramStr(const int& NGramId) const {
  int Words=WordStrToFqH.Len();
  if (NGramId<Words){ // if ngram-id is single word
    return WordStrToFqH.GetKey(NGramId);
  } else { // if ngram-id is multiple word
    const TIntV& WIdV=WIdVToFqH.GetKey(NGramId-Words);
    return GetWIdVStr(WIdV);
  }
}

int TNGramBs::GetNGramFq(const int& NGramId) const {
  int Words=WordStrToFqH.Len();
  if (NGramId<Words){ // if ngram-id is single word
    return WordStrToFqH[NGramId];
  } else { // if ngram-id is multiple word
    return WIdVToFqH[NGramId-Words];
  }
}

void TNGramBs::GetNGramStrFq(const int& NGramId, TStr& NGramStr, int& NGramFq) const {
  int Words=WordStrToFqH.Len();
  if (NGramId<Words){
    // if ngram-id is single word
    NGramStr=WordStrToFqH.GetKey(NGramId);
    NGramFq=WordStrToFqH[NGramId];
  } else {
    // if ngram-id is multiple word
    const TIntV& WIdV=WIdVToFqH.GetKey(NGramId-Words);
    NGramStr=GetWIdVStr(WIdV);
    NGramFq=WIdVToFqH[NGramId-Words];
  }
}

void TNGramBs::GetNGramIdV(
 const TStr& HtmlStr, TIntV& NGramIdV, TIntPrV& NGramBEChXPrV) const {
  // create MxNGramLen queues
  TVec<TIntQ> WIdQV(MxNGramLen);
  TVec<TIntPrQ> BEChXPrQV(MxNGramLen);
  for (int NGramLen=1; NGramLen<MxNGramLen; NGramLen++){
    WIdQV[NGramLen].Gen(100*NGramLen, NGramLen+1);
    BEChXPrQV[NGramLen].Gen(100*NGramLen, NGramLen+1);
  }
  bool AllWIdQClrP=true;
  // extract words from text-string
  PSIn HtmlSIn=TStrIn::New(HtmlStr, false);
  THtmlLx HtmlLx(HtmlSIn);
  while (HtmlLx.Sym!=hsyEof){
    if ((HtmlLx.Sym==hsyStr)||(HtmlLx.Sym==hsyNum)){
      // get word-string & word-id
      TStr WordStr=HtmlLx.UcChA;
      int WId; int SymBChX=HtmlLx.SymBChX; int SymEChX=HtmlLx.SymEChX;
      if ((SwSet.Empty())||(!SwSet->IsIn(WordStr))){
        if (!Stemmer.Empty()){
          WordStr=Stemmer->GetStem(WordStr);}
        if (IsWord(WordStr, WId)){
          if (!IsSkipWord(WId)){
            NGramIdV.Add(0+WId); // add single word
            NGramBEChXPrV.Add(TIntPr(SymBChX, SymEChX)); // add positions
            for (int NGramLen=1; NGramLen<MxNGramLen; NGramLen++){
              TIntQ& WIdQ=WIdQV[NGramLen];
              TIntPrQ& BEChXPrQ=BEChXPrQV[NGramLen];
              WIdQ.Push(WId); BEChXPrQ.Push(TIntPr(SymBChX, SymEChX));
              AllWIdQClrP=false;
              // if queue full
              if (WIdQ.Len()==NGramLen+1){
                // create sequence
                TIntV WIdV; WIdQ.GetSubValVec(0, WIdQ.Len()-1, WIdV);
                TIntPrV BEChXPrV; BEChXPrQ.GetSubValVec(0, BEChXPrQ.Len()-1, BEChXPrV);
                // add ngram-id or reset queues
                int WIdVP;
                if (WIdVToFqH.IsKey(WIdV, WIdVP)){ // if sequence is frequent
                  int NGramId=GetWords()+WIdVP; // get sequence ngram-id
                  NGramIdV.Add(NGramId); // add sequence ngram-id
                  NGramBEChXPrV.Add(TIntPr(BEChXPrV[0].Val1, BEChXPrV.Last().Val2)); // add positions
                }
              }
            }
          }
        } else {
          // break queue sequences if infrequent word occures
          if (!AllWIdQClrP){
            for (int NGramLen=1; NGramLen<MxNGramLen; NGramLen++){
              TIntQ& WIdQ=WIdQV[NGramLen];
              TIntPrQ& BEChXPrQ=BEChXPrQV[NGramLen];
              if (!WIdQ.Empty()){WIdQ.Clr(); BEChXPrQ.Clr();}
            }
            AllWIdQClrP=true;
          }
        }
      }
    }
    // get next symbol
    HtmlLx.GetSym();
  }
}

void TNGramBs::GetNGramStrV(
 const TStr& HtmlStr, TStrV& NGramStrV, TIntPrV& NGramBEChXPrV) const {
  TIntV NGramIdV; NGramStrV.Clr(); NGramBEChXPrV.Clr();
  TNGramBs::GetNGramIdV(HtmlStr, NGramIdV, NGramBEChXPrV);
  NGramStrV.Gen(NGramIdV.Len(), 0);
  for (int NGramIdN=0; NGramIdN<NGramIdV.Len(); NGramIdN++){
    TStr NGramStr=GetNGramStr(NGramIdV[NGramIdN]);
    NGramStrV.Add(NGramStr);
  }
}

void TNGramBs::GetNGramStrV(const TStr& HtmlStr, TStrV& NGramStrV) const {
  TIntPrV NGramBEChXPrV; GetNGramStrV(HtmlStr, NGramStrV, NGramBEChXPrV);
}

void TNGramBs::AddWordToCand(const int& WId){
  Assert(PassN>1);
  Assert(WId!=-1);
  Assert(GetWordFq(WId)>=MnNGramFq);

  // candidate queue update
  CandWIdQ.Push(WId); // add word-id to candidate queue
  int CandWIdQLen=CandWIdQ.Len(); // get shortcut to queue length
  if (CandWIdQLen==1){return;} // return if single word queue

  // create candidate if possible and increment its frequency
  if (PassN==2){ // second pass
    if (CandWIdQLen==PassN){
      CandWIdPrToFqH.AddDat(TIntPr(CandWIdQ[0], CandWIdQ[1]))++;}
  } else { // higher passes
    if ((1<CandWIdQLen)&&(CandWIdQLen<PassN)){
      // get subsequence
      TIntV SubWIdV; CandWIdQ.GetSubValVec(0, CandWIdQ.Len()-1, SubWIdV);
      // get subsequence-id & break queue if subsequence unknown
      int SubWIdVId=WIdVToFqH.GetKeyId(SubWIdV);
      if (SubWIdVId==-1){BreakNGram();}
    } else {
      Assert(CandWIdQLen==PassN);
      // get subsequence
      TIntV SubWIdV; CandWIdQ.GetSubValVec(1, CandWIdQ.Len()-1, SubWIdV);
      // get subsequence-id
      int SubWIdVId=WIdVToFqH.GetKeyId(SubWIdV);
      if (SubWIdVId==-1){
        BreakNGram(); // break queue if subsequence unknown
      } else {
        // create candidate sequence
        TIntV WIdV; CandWIdQ.GetSubValVec(0, CandWIdQ.Len()-1, WIdV);
        // add candidate sequence and increment its frequence
        CandWIdVToFqH.AddDat(WIdV)++;
      }
    }
  }
}

int TNGramBs::AddWord(const TStr& WordStr){
  int WId=-1;
  if (PassN==1){ // first pass
    WId=WordStrToFqH.AddKey(WordStr); // get word-id
    WordStrToFqH[WId]++; // increment frequency
  } else { // higher passes
    WId=GetWId(WordStr); // get word-id
    if (WId==-1){ // if word is infrequent
      BreakNGram(); // break current ngram sequence
    } else
    if (GetWordFq(WId)!=-1){ // if word is not stop-word
      AddWordToCand(WId); // add word-id to current ngram sequence
    }
  }
  return WId;
}

void TNGramBs::ConcPass(){
  IAssert(!IsFinished());
  if (PassN==1){ // first pass
    // collect stop words and words with too low frequency
    TIntV DelWIdV; // vector for word-ids for deleting
    int WIds=WordStrToFqH.Len(); // get number of words-ids
    for (int WId=0; WId<WIds; WId++){
      TStr WordStr=WordStrToFqH.GetKey(WId); // get word string
      //if ((!SwSet.Empty())&&(SwSet->IsIn(WordStr))){
      //  WordStrToFqH[WId]=-1; // reset stop-word frequency to -1
      //} else
      if (WordStrToFqH[WId]<MnNGramFq){
        DelWIdV.Add(WId); // add infrequent word-id to delete-list
      }
    }
    // delete words
    WordStrToFqH.DelKeyIdV(DelWIdV);
    WordStrToFqH.Defrag();
  } else
  if (PassN==2){ // second pass
    int Cands=CandWIdPrToFqH.Len(); // get number of candidates
    TIntV WIdV(2); // pre-decl for frequent-candidates vector
    for (int CandId=0; CandId<Cands; CandId++){
      int CandFq=CandWIdPrToFqH[CandId]; // get candidate frequency
      if (CandFq>=MnNGramFq){ // if candidate is frequent
        const TIntPr& WIdPr=CandWIdPrToFqH.GetKey(CandId); // get word-id pair
        WIdV[0]=WIdPr.Val1; WIdV[1]=WIdPr.Val2; // assign word-id to vector
        WIdVToFqH.AddDat(WIdV, CandFq); // add frequent vector
      }
    }
    // clear candidate word-id pairs
    CandWIdPrToFqH.Clr();
  } else
  if (PassN>2){ // higher passes
    int Cands=CandWIdVToFqH.Len(); // get number of candidates
    for (int CandId=0; CandId<Cands; CandId++){
      int CandFq=CandWIdVToFqH[CandId]; // get candidate frequency
      if (CandFq>=MnNGramFq){ // if candidate is frequent
        const TIntV& CandWIdV=CandWIdVToFqH.GetKey(CandId); // get word-id vector
        WIdVToFqH.AddDat(CandWIdV, CandWIdVToFqH[CandId]); // add frequent vector
      }
    }
    // clear candidate word-id vectors
    CandWIdVToFqH.Clr();
  } else {
    Fail;
  }

  // increment pass-number
  PassN++;

  // conclude or prepare for new pass
  if (IsFinished()){
    // clear queue
    CandWIdQ.Clr();
    // reset stop-words
    int WIds=WordStrToFqH.Len(); // get number of words-ids
    for (int WId=0; WId<WIds; WId++){
      TStr WordStr=WordStrToFqH.GetKey(WId); // get word string
      if ((!SwSet.Empty())&&(SwSet->IsIn(WordStr))){
        WordStrToFqH[WId]=-1; // reset stop-word frequency to -1
      }
    }
    // reset ngrams starting with with stop-words
    for (int WIdVId=0; WIdVId<WIdVToFqH.Len(); WIdVId++){
      int FirstWId=WIdVToFqH.GetKey(WIdVId)[0]; // get first word-id
      int LastWId=WIdVToFqH.GetKey(WIdVId).Last(); // get last word-id
      if ((WordStrToFqH[FirstWId]==-1)||(WordStrToFqH[LastWId]==-1)){
        TStr NGramStr=GetWIdVStr(WIdVToFqH.GetKey(WIdVId));
        WIdVToFqH[WIdVId]=-1;
      }
    }
    // print frequent ngrams
    for (int WIdVId=0; WIdVId<WIdVToFqH.Len(); WIdVId++){
      if (WIdVToFqH.GetKey(WIdVId).Len()>1){
        //TStr NGramStr=GetWIdVStr(WIdVToFqH.GetKey(WIdVId));
        //int NGramFq=WIdVToFqH[WIdVId];
        //printf("%s: %d\n", NGramStr.CStr(), NGramFq);
      }
    }
  } else {
    // prepare new queue length
    CandWIdQ.Gen(100*PassN, PassN);
  }
}

void TNGramBs::SaveTxt(const TStr& FNm, const bool& SortP) const {
  TFOut FOut(FNm); FILE* fOut=FOut.GetFileId();
  if (SortP){
    int NGrams=GetNGrams(); TStr NGramStr; int NGramFq;
    TIntPrV FqNGramIdPrV(GetNGrams(), 0);
    for (int NGramId=0; NGramId<NGrams; NGramId++){
      GetNGramStrFq(NGramId, NGramStr, NGramFq);
      if (NGramFq!=-1){
        FqNGramIdPrV.Add(TIntPr(NGramFq, NGramId));}
    }
    FqNGramIdPrV.Sort(false);
    for (int NGramN=0; NGramN<FqNGramIdPrV.Len(); NGramN++){
      int NGramFq=FqNGramIdPrV[NGramN].Val1;
      int NGramId=FqNGramIdPrV[NGramN].Val2;
      GetNGramStrFq(NGramId, NGramStr, NGramFq);
      fprintf(fOut, "'%s': %d\n", NGramStr.CStr(), NGramFq);
    }
  } else {
    int NGrams=GetNGrams(); TStr NGramStr; int NGramFq;
    for (int NGramId=0; NGramId<NGrams; NGramId++){
      GetNGramStrFq(NGramId, NGramStr,  NGramFq);
      if (NGramFq!=-1){
        fprintf(fOut, "'%s': %d\n", NGramStr.CStr(), NGramFq);}
    }
  }
}

void TNGramBs::_UpdateNGramBsFromHtmlStr(
 const PNGramBs& NGramBs, const TStr& HtmlStr,
 const PSwSet& SwSet, const PStemmer& Stemmer){
  // prepare html parsing
  PSIn HtmlSIn=TStrIn::New(HtmlStr, false);
  PHtmlDoc HtmlDoc=THtmlDoc::New(HtmlSIn, hdtAll, false);
  PHtmlTok Tok; THtmlLxSym Sym; TStr Str;
  PHtmlLxChDef ChDef=THtmlLxChDef::GetChDef();
  bool InScript=false;
  // traverse html tokens
  NGramBs->BreakNGram();
  for (int TokN=0; TokN<HtmlDoc->GetToks(); TokN++){
    PHtmlTok Tok=HtmlDoc->GetTok(TokN);
    HtmlDoc->GetTok(TokN, Sym, Str);
    switch (Sym){
      case hsyUndef: break;
      case hsySSym: break;
      case hsyStr:
      case hsyNum:{
        if (InScript){break;}
        TStr UcStr=ChDef->GetUcStr(Str);
        if (true||(SwSet.Empty())||(!SwSet->IsIn(UcStr))){
          if (!Stemmer.Empty()){
            UcStr=Stemmer->GetStem(UcStr);}
          NGramBs->AddWord(UcStr);
        }
        break;}
      case hsyBTag:
      case hsyETag:
        if (Str=="<SCRIPT>"){
          InScript=(Sym==hsyBTag);}
        if (THtmlTok::IsBreakTag(Str)){
          NGramBs->BreakNGram();}
        break;
      case hsyEof: break;
      default: break;
    }
  }
}

PNGramBs TNGramBs::GetNGramBsFromHtmlStrV(
 const TStrV& HtmlStrV,
 const int& MxNGramLen, const int& MnNGramFq,
 const PSwSet& SwSet, const PStemmer& Stemmer){
  // create n-gram-base
  /* printf("Generating frequent n-grams (MaxLen:%d MinFq:%d) ...\n", MxNGramLen, MnNGramFq); */
  PNGramBs NGramBs=TNGramBs::New(MxNGramLen, MnNGramFq, SwSet, Stemmer);
  // interations over document-set
  while (!NGramBs->IsFinished()){
    for (int HtmlStrN=0; HtmlStrN<HtmlStrV.Len(); HtmlStrN++){
      /* if ((HtmlStrN%10==0)||(HtmlStrN+1==HtmlStrV.Len())){ */
        /* printf("  Pass %2d: %6d/%6d Docs\r", NGramBs->GetPassN(), HtmlStrN+1, HtmlStrV.Len()); */
        /* if (HtmlStrN+1==HtmlStrV.Len()){ */
            /* printf("\n"); */
        /* } */
      /* } */
      // extract words & update ngram-base
      _UpdateNGramBsFromHtmlStr(NGramBs, HtmlStrV[HtmlStrN], SwSet, Stemmer);
    }
    NGramBs->ConcPass();
  }
  /* printf("Done.\n"); */
  // return
  return NGramBs;
}

PNGramBs TNGramBs::GetNGramBsFromHtmlFPathV(
 const TStr& FPath, const bool& RecurseDirP, const int& MxDocs,
 const int& MxNGramLen, const int& MnNGramFq,
 const PSwSet& SwSet, const PStemmer& Stemmer){
  // create n-gram-base
  /* printf("Generating frequent n-grams (MaxLen:%d MinFq:%d) ...\n", MxNGramLen, MnNGramFq); */
  PNGramBs NGramBs=TNGramBs::New(MxNGramLen, MnNGramFq, SwSet, Stemmer);
  // interations over document-set
  while (!NGramBs->IsFinished()){
    // prepare file-directory traversal
    TFFile FFile(FPath, "", RecurseDirP);
    // traverse files
    TStr FNm; int Docs=0;
    while (FFile.Next(FNm)){
      Docs++; if ((MxDocs!=-1)&&(Docs>MxDocs)){break;}
      /* if (Docs%100==0){ */
        /* printf("  Pass %d: %6d\r", NGramBs->GetPassN(), Docs);} */
      if (TFile::Exists(FNm)) {
        // load html
        TStr HtmlStr=TStr::LoadTxt(FNm);
        // extract words & update ngram-base
        _UpdateNGramBsFromHtmlStr(NGramBs, HtmlStr, SwSet, Stemmer);
      }
    }
    NGramBs->ConcPass();
    /* printf("  Pass %d: %6d\n", NGramBs->GetPassN()-1, Docs); */
  }
  /* printf("Done.\n"); */
  // return
  return NGramBs;
}

PNGramBs TNGramBs::GetNGramBsFromLnDoc(
 const TStr& LnDocFNm, const bool& NamedP, const int& MxDocs,
 const int& MxNGramLen, const int& MnNGramFq,
 const PSwSet& SwSet, const PStemmer& Stemmer){
  // create n-gram-base
  /* printf("Generating frequent n-grams (MaxLen:%d MinFq:%d) ...\n", MxNGramLen, MnNGramFq); */
  PNGramBs NGramBs=TNGramBs::New(MxNGramLen, MnNGramFq, SwSet, Stemmer);
  // interations over document-set
  while (!NGramBs->IsFinished()){
    // open line-doc file
    TFIn FIn(LnDocFNm); char Ch=' '; int Docs=0;
    while (!FIn.Eof()){
	  if(Ch == '\r' || Ch == '\n'){Ch = FIn.GetCh(); continue;}
      Docs++; if ((MxDocs!=-1)&&(Docs>=MxDocs)){break;}
      //printf("%d\r", Docs);
      // document name
      TChA DocNm;
      if (NamedP){
        Ch=FIn.GetCh();
        while ((!FIn.Eof())&&(Ch!='\r')&&(Ch!='\n')&&(Ch!=' ')){
          DocNm+=Ch; Ch=FIn.GetCh();}
        DocNm.Trunc();
        if (DocNm.Empty()){Docs--; continue;}
      }
      // categories
      TStrV CatNmV;
      forever {
        while ((!FIn.Eof())&&(Ch==' ')){Ch=FIn.GetCh();}
        if (Ch=='!'){
          TChA CatNm;
          while ((!FIn.Eof())&&(Ch!='\r')&&(Ch!='\n')&&(Ch!=' ')){
            CatNm+=Ch; Ch=FIn.GetCh();}
          if (!CatNm.Empty()){CatNmV.Add(CatNm);}
        } else {
          break;
        }
      }
      // document text
      TChA DocChA;
      while ((!FIn.Eof())&&(Ch!='\r')&&(Ch!='\n')){
        DocChA+=Ch; Ch=FIn.GetCh();}
      // extract words & update ngram-base
      /* printf("  Pass %2d: %6d Docs\r", NGramBs->GetPassN(), Docs); */
      _UpdateNGramBsFromHtmlStr(NGramBs, DocChA, SwSet, Stemmer);
    }
    NGramBs->ConcPass();
  }
  /* printf("\nDone.\n"); */
  // return
  return NGramBs;
}

PNGramBs TNGramBs::GetNGramBsFromReuters21578(
 const TStr& FPath, const int& MxDocs,
 const int& MxNGramLen, const int& MnNGramFq,
 const PSwSet& SwSet, const PStemmer& Stemmer){
  // create n-gram-base
  /* printf("Generating frequent n-grams (MaxLen:%d MinFq:%d) ...\n", MxNGramLen, MnNGramFq); */
  PNGramBs NGramBs=TNGramBs::New(MxNGramLen, MnNGramFq, SwSet, Stemmer);
  // interations over document-set
  while (!NGramBs->IsFinished()){
    TFFile FFile(FPath, ".SGM", false); TStr FNm; int Docs=0;
    while (FFile.Next(FNm)){
      /* printf("Processing file '%s'\n", FNm.CStr()); */
      TXmlDocV LDocV; TXmlDoc::LoadTxt(FNm, LDocV);
      for (int LDocN=0; LDocN<LDocV.Len(); LDocN++){
        Docs++;
        /* printf("  Pass %2d: %6d Docs\r", NGramBs->GetPassN(), Docs); */
        if ((MxDocs!=-1)&&(Docs>=MxDocs)){break;}
        PXmlDoc Doc=LDocV[LDocN];
        PXmlTok DocTok=Doc->GetTok();
        // get html string
        PXmlTok Tok=Doc->GetTagTok("REUTERS|TEXT");
        TStr HtmlStr=Tok->GetTokStr(false);
        // extract words & update n-gram-base
        _UpdateNGramBsFromHtmlStr(NGramBs, HtmlStr, SwSet, Stemmer);
      }
      if ((MxDocs!=-1)&&(Docs>=MxDocs)){break;}
    }
    NGramBs->ConcPass();
    /* printf("  Pass %2d: %6d Docs\r", NGramBs->GetPassN(), Docs); */
  }
  /* printf("\nDone.\n"); */
  // return
  return NGramBs;
}

/////////////////////////////////////////////////
// Stream-NGram-Hash-Table
uint TStreamNGramHash::AddKey(const TSNGHashKey& Key){
  uint PrimHashCd; Key.GetUInt(0, PrimHashCd);
  uint SecHashCd; Key.GetUInt(4, SecHashCd);
  const uint PortN=PrimHashCd%PortV.Len();
  const TUInt HashCd=SecHashCd;
  uint PrevKeyId=0;
  uint KeyId=PortV[PortN];
  while ((KeyId!=0)&&(KeyDatV[KeyId].HashCd!=HashCd)){
    PrevKeyId=KeyId; KeyId=KeyDatV[KeyId].Next;}

  //if (KeyId!=0){
  //  if ((KeyDatV[KeyId].HashCd==HashCd)&&!(KeyDatV[KeyId].Key==Key)){
  //    printf("[%d]", KeyId);
  //  }
  //}

  if (KeyId==0){
    if (KeyDatV.Len()<KeyDatV.Reserved()){
      KeyId=KeyDatV.Add(TStreamNGramHashKeyDat(0, TimeFirst, 0, HashCd));
      //KeyDatV[KeyId].Key=Key;
      TimeFirst=KeyId; if (TimeLast==0){TimeLast=KeyId;}
    } else {
      KeyId=TimeLast;
      TimeLast=KeyDatV[KeyId].TimePrev; KeyDatV[TimeLast].TimeNext=0;
      KeyDatV[KeyId]=TStreamNGramHashKeyDat(0, TimeFirst, 0, HashCd);
      //KeyDatV[KeyId].Key=Key;
      KeyDatV[TimeFirst].TimePrev=KeyId; TimeFirst=KeyId;
    }
    if (PrevKeyId==0){
      PortV[PortN]=KeyId;
    } else {
      KeyDatV[PrevKeyId].Next=KeyId;
    }
  }
  return KeyId;
}

/////////////////////////////////////////////////
// Stream-NGram-Base
void TStreamNGramBs::BreakTokStream(){
  int TokIdQVLen=TokIdQV.Len();
  for (int TokIdQN=0; TokIdQN<TokIdQVLen; TokIdQN++){
    TokIdQV[TokIdQN].Clr(false);
  }
}

void TStreamNGramBs::AddTokId(
 const int& TokId, const int& TokPos, const bool& OutputNGramsP, 
 const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV){
  if (TokId==0){BreakTokStream(); return;}
  // shortcuts
  int TokIdQVLen=TokIdQV.Len();
  TMem NGramMem(MxNGramLen*sizeof(int));
  // traverse token-queues
  for (int TokIdQN=0; TokIdQN<TokIdQVLen; TokIdQN++){
    int NGramLen=1+TokIdQN; // TokIdQV[0]==Uniram
    TIntQ& TokIdQ=TokIdQV[TokIdQN];
    if (TokIdQ.Len()==NGramLen){TokIdQ.Pop();}
    TokIdQ.Push(TokId);
    // if queue is ready 
    if (TokIdQ.Len()==NGramLen){
      // construct memory-footprint for ngram
      NGramMem.Clr(false);
      for (int TokIdN=0; TokIdN<NGramLen; TokIdN++){
        int TokId=TokIdQ[TokIdN];
        NGramMem.AddBf(&TokId, sizeof(TokId));
      }
      // create md5-signature for ngram
      TMd5Sig NGramSig(NGramMem);
      // increment ngram-frequency
      int Fq=StreamNGramHash.AddDat(NGramSig)++;
      // create output-ngram-descriptor
      if (OutputNGramsP && Fq >= MnOutputNGramFq){
        TNGramDesc NGramDesc;
        NGramDesc.Sig=NGramSig;
        NGramDesc.Fq=Fq;
        NGramDesc.Pos=TokPos;
        TChA NGramChA;
        for (int TokIdN=0; TokIdN<NGramLen; TokIdN++){
          int TokId=TokIdQ[TokIdN];
          NGramDesc.TokIdV.Add(TokId);
          if (IsTokStr()){
            TStr TokStr=GetTokStr(TokId);
            NGramDesc.TokStrV.Add(TokStr);
            if (TokIdN>0){NGramChA+=' ';}
            NGramChA+=TokStr;
          }
        }
        if (IsTokStr()){
          NGramDesc.Str=NGramChA;}
        OutputNGramDescV.Add(NGramDesc);
      }
    }
  }
}

void TStreamNGramBs::AddTokIdV(
 const TIntV& TokIdV, const int& FirstTokPos, const bool& OutputNGramsP,
 const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV){
  for (int TokIdN=0; TokIdN<TokIdV.Len(); TokIdN++){
    AddTokId(TokIdV[TokIdN], FirstTokPos+TokIdN, OutputNGramsP, MnOutputNGramFq, OutputNGramDescV);
  }
}

void TStreamNGramBs::AddTokStr(
 const TStr& TokStr, const int& TokPos, const bool& OutputNGramsP,
 const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV){
  int TokId=TokStrH.AddKey(TokStr);
  //TMd5Sig TokSig(TokStr);
  //uint TokId; TokSig.GetUInt(0, TokId);
  AddTokId(TokId, TokPos, OutputNGramsP, MnOutputNGramFq, OutputNGramDescV);
}

void TStreamNGramBs::AddDocTokIdV(
 const TIntV& DocTokIdV, const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV){
  BreakTokStream(); OutputNGramDescV.Clr();
  AddTokIdV(DocTokIdV, 0, true, MnOutputNGramFq, OutputNGramDescV);
}

void TStreamNGramBs::AddDocHtmlStr(
 const TStr& DocHtmlStr, const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV){
  BreakTokStream(); OutputNGramDescV.Clr();
  // prepare html parsing
  PSIn HtmlSIn=TStrIn::New(DocHtmlStr, false);
  PHtmlDoc HtmlDoc=THtmlDoc::New(HtmlSIn, hdtAll, true);
  PHtmlTok Tok; THtmlLxSym TokSym; TStr TokStr;
  bool InScript=false;
  // traverse html tokens
  for (int TokN=0; TokN<HtmlDoc->GetToks(); TokN++){
    PHtmlTok Tok=HtmlDoc->GetTok(TokN);
    HtmlDoc->GetTok(TokN, TokSym, TokStr);
    switch (TokSym){
      case hsyUndef: break;
      case hsySSym:
      case hsyStr:
      case hsyNum:
        if (InScript){break;}
        // printf("."); //printf("[%s] ", TokStr.CStr());
        AddTokStr(TokStr, TokN, true, MnOutputNGramFq, OutputNGramDescV);
        break;
      case hsyBTag:
      case hsyETag:
        if (TokStr=="<SCRIPT>"){
          InScript=(TokSym==hsyBTag); BreakTokStream();}
        break;
      case hsyEof: break;
      default: break;
    }
  }
}

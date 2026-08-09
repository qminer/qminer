/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// NGram-Base
ClassTP(TNGramBs, PNGramBs)//{
private:
  TInt MxNGramLen;
  TInt MnNGramFq;
  PSwSet SwSet;
  PStemmer Stemmer;
  TStrIntH WordStrToFqH;
  TIntVIntH WIdVToFqH;
private:
  // work variables
  TInt PassN;
  TIntQ CandWIdQ;
  TIntPrIntH CandWIdPrToFqH;
  TIntVIntH CandWIdVToFqH;
public:
  TNGramBs(const int& _MxNGramLen, const int& _MnNGramFq,
   const PSwSet& _SwSet, const PStemmer& _Stemmer):
    MxNGramLen(_MxNGramLen), MnNGramFq(_MnNGramFq),
    SwSet(_SwSet), Stemmer(_Stemmer),
    WordStrToFqH(), WIdVToFqH(),
    PassN(1), CandWIdQ(), CandWIdPrToFqH(), CandWIdVToFqH(){
    IAssert(MxNGramLen>=1); IAssert(MnNGramFq>=1);}
  static PNGramBs New(const int& MxNGramLen, const int& MnNGramFq,
   const PSwSet& SwSet, const PStemmer& Stemmer){
    return PNGramBs(new TNGramBs(MxNGramLen, MnNGramFq, SwSet, Stemmer));}
  TNGramBs(const TNGramBs&){Fail;}
  TNGramBs(TSIn& SIn):
    MxNGramLen(SIn), MnNGramFq(SIn),
    SwSet(SIn), Stemmer(SIn),
    WordStrToFqH(SIn), WIdVToFqH(SIn),
    PassN(-1), CandWIdQ(), CandWIdPrToFqH(), CandWIdVToFqH(){}
  static PNGramBs Load(TSIn& SIn){return new TNGramBs(SIn);}
  void Save(TSOut& SOut){
    MxNGramLen.Save(SOut); MnNGramFq.Save(SOut);
    SwSet.Save(SOut); Stemmer.Save(SOut);
    WordStrToFqH.Save(SOut); WIdVToFqH.Save(SOut);}

  TNGramBs& operator=(const TNGramBs&){Fail; return *this;}

  // comonents retrieval
  int GetMxNGramLen() const {return MxNGramLen;}
  int GetMnNGramFq() const {return MnNGramFq;}
  PSwSet GetSwSet() const {return SwSet;}
  PStemmer GetStemmer() const {return Stemmer;}
  int GetPassN() const {return PassN;}

  // words
  int GetWords() const {return WordStrToFqH.Len();}
  bool IsWord(const TStr& WordStr, int& WId) const {
    return WordStrToFqH.IsKey(WordStr, WId);}
  int GetWId(const TStr& WordStr) const {
    return WordStrToFqH.GetKeyId(WordStr);}
  TStr GetWordStr(const int& WId) const {
    return WordStrToFqH.GetKey(WId);}
  int GetWordFq(const int& WId) const {
    return WordStrToFqH[WId];}
  bool IsSkipWord(const int& WId) const {
    return WordStrToFqH[WId]==-1;}

  // word-sequences
  int GetWIdVs() const {return WIdVToFqH.Len();}
  int GetWIdVFq(const TIntV& WIdV) const {
    return WIdVToFqH.GetDat(WIdV);}
  int GetWIdVFq(const int& WIdVId) const {
    return WIdVToFqH[WIdVId];}
  TStr GetWIdVStr(const TIntV& WIdV) const;

  // ngrams
  int GetNGrams() const {return WordStrToFqH.Len()+WIdVToFqH.Len();}
  TStr GetNGramStr(const int& NGramId) const;
  int GetNGramFq(const int& NGramId) const;
  void GetNGramStrFq(const int& NGramId, TStr& NGramStr, int& NGramFq) const;
  void GetNGramIdV(const TStr& HtmlStr, TIntV& NGramIdV, TIntPrV& NGramBEChXPrV) const;
  void GetNGramStrV(const TStr& HtmlStr, TStrV& NGramStrV, TIntPrV& NGramBEChXPrV) const;
  void GetNGramStrV(const TStr& HtmlStr, TStrV& NGramStrV) const;

  // generation of ngrams
  void AddWordToCand(const int& WId);
  int AddWord(const TStr& WordStr);
  void BreakNGram(){if (!CandWIdQ.Empty()){CandWIdQ.Clr();}}
  void ConcPass();
  bool IsFinished() const {return PassN>MxNGramLen;}

  // files
  void SaveTxt(const TStr& FNm="CON", const bool& SortP=false) const;

  // n-gram-base creation
  static void _UpdateNGramBsFromHtmlStr(
   const PNGramBs& NGramBs, const TStr& HtmlStr,
   const PSwSet& SwSet, const PStemmer& Stemmer);
  static PNGramBs GetNGramBsFromHtmlStrV(
   const TStrV& HtmlStrV,
   const int& MxNGramLen, const int& MnNGramFq,
   const PSwSet& SwSet=NULL, const PStemmer& Stemmer=NULL);
  static PNGramBs GetNGramBsFromHtmlFPathV(
   const TStr& FPath, const bool& RecurseDirP, const int& MxDocs,
   const int& MxNGramLen, const int& MnNGramFq,
   const PSwSet& SwSet=NULL, const PStemmer& Stemmer=NULL);
  static PNGramBs GetNGramBsFromLnDoc(
   const TStr& LnDocFNm, const bool& NamedP, const int& MxDocs=-1,
   const int& MxNGramLen=1, const int& MnNGramFq=1,
   const PSwSet& SwSet=NULL, const PStemmer& Stemmer=NULL);
  static PNGramBs GetNGramBsFromReuters21578(
   const TStr& FPath, const int& MxDocs=-1,
   const int& MxNGramLen=1, const int& MnNGramFq=1,
   const PSwSet& SwSet=NULL, const PStemmer& Stemmer=NULL);
  static PNGramBs GetNGramBsFromCiaWFB(
   const TStr& FPath, const int& MxDocs=-1,
   const int& MxNGramLen=1, const int& MnNGramFq=1,
   const PSwSet& SwSet=NULL, const PStemmer& Stemmer=NULL);
};

/////////////////////////////////////////////////
// Stream-NGram-Hash-Table-Key-Data
typedef TMd5Sig TSNGHashKey;
typedef TUInt TSNGHashDat;

#pragma pack(push, 1) // pack class size
class TStreamNGramHashKeyDat{
public:
  TUInt TimePrev, TimeNext;
  TUInt Next;
  TUInt HashCd;
  //TMd5Sig Key;
  TSNGHashDat Dat;
public:
  TStreamNGramHashKeyDat():
    TimePrev(0), TimeNext(0), Next(0), HashCd(0), Dat(){}
  TStreamNGramHashKeyDat(
   const uint& _TimePrev, const uint& _TimeNext, const uint& _Next, const uint& _HashCd):
    TimePrev(_TimePrev), TimeNext(_TimeNext), Next(_Next), HashCd(_HashCd), Dat(){}
  explicit TStreamNGramHashKeyDat(TSIn& SIn):
    TimePrev(SIn), TimeNext(SIn), Next(SIn), HashCd(SIn), Dat(SIn){}
  void Load(TSIn& SIn){
    TimePrev.Load(SIn); TimeNext.Load(SIn); Next.Load(SIn);
    HashCd.Load(SIn); Dat.Load(SIn);}
  void Save(TSOut& SOut) const {
    TimePrev.Save(SOut); TimeNext.Save(SOut); Next.Save(SOut);
    HashCd.Save(SOut); Dat.Save(SOut);}

  bool operator==(const TStreamNGramHashKeyDat& KeyDat) const {
    return (this==&KeyDat)||(HashCd==KeyDat.HashCd);}
  TStreamNGramHashKeyDat& operator=(const TStreamNGramHashKeyDat& KeyDat){
    if (this!=&KeyDat){
      TimePrev=KeyDat.TimePrev; TimeNext=KeyDat.TimeNext; Next=KeyDat.Next;
      HashCd=KeyDat.HashCd; Dat=KeyDat.Dat;}
    return *this;}
};
#pragma pack(pop)

/////////////////////////////////////////////////
// Stream-NGram-Hash-Table
ClassTP(TStreamNGramHash, PStreamNGramHash)//{
private:
  TUIntV PortV;
  TVec<TStreamNGramHashKeyDat> KeyDatV;
  TUInt TimeFirst, TimeLast;
  UndefCopyAssign(TStreamNGramHash);
private:
  TStreamNGramHashKeyDat& GetHashKeyDat(const uint& KeyId){return KeyDatV[KeyId];}
public:
  TStreamNGramHash(const uint& MxKeys):
    PortV(), KeyDatV(), TimeFirst(), TimeLast(){Gen(MxKeys);}
  static PStreamNGramHash New(const uint& MxKeys){
    return PStreamNGramHash(new TStreamNGramHash(MxKeys));}
  TStreamNGramHash(TSIn& SIn):
    PortV(SIn), KeyDatV(SIn), TimeFirst(SIn), TimeLast(SIn){SIn.LoadCs();}
  void Load(TSIn& SIn){
    PortV.Load(SIn); KeyDatV.Load(SIn);
    TimeFirst.Load(SIn); TimeLast.Load(SIn); SIn.LoadCs();}
  void Save(TSOut& SOut) const {
    PortV.Save(SOut); KeyDatV.Save(SOut);
    TimeFirst.Save(SOut); TimeLast.Save(SOut); SOut.SaveCs();}

  TSNGHashDat& operator[](const uint& KeyId){return GetHashKeyDat(KeyId).Dat;}
  ::TSize GetMemUsed() const {Fail; return 0;} //

  void Gen(const uint& MxKeys){
    // get number of ports
    uint PortsAsPow2=0; uint _MxKeys=MxKeys;
    while (_MxKeys!=0){_MxKeys=_MxKeys/2; PortsAsPow2++;}
    uint Ports=(uint(1) << (PortsAsPow2-1));
    // create ports & keydata
    PortV.Gen(Ports); KeyDatV.Gen(MxKeys, 0);
    TimeFirst=0; TimeLast=0;
    KeyDatV.Add(TStreamNGramHashKeyDat()); // zeroth element is guard
  }

  void Clr(){
    PortV.PutAll(0); KeyDatV.Clr(false);
    TimeFirst=0; TimeLast=0;
    KeyDatV.Add(TStreamNGramHashKeyDat());}
  bool Empty() const {return Len()==0;}
  uint Len() const {return KeyDatV.Len();}
  uint GetPorts() const {return PortV.Len();}
  uint GetMxKeys() const {return KeyDatV.Reserved();}

  uint AddKey(const TSNGHashKey& Key);
  TSNGHashDat& AddDat(const TSNGHashKey& Key){
    return GetHashKeyDat(AddKey(Key)).Dat;}
  TSNGHashDat& AddDat(const TSNGHashKey& Key, const TSNGHashDat& Dat){
    return GetHashKeyDat(AddKey(Key)).Dat=Dat;}
};

/////////////////////////////////////////////////
// NGram-Descriptor
ClassTV(TNGramDesc, TNGramDescV)//{
public:
  TMd5Sig Sig;
  TInt Fq;
  TInt Pos;
  TIntV TokIdV;
  TStrV TokStrV;
  TStr Str;
public:
  TNGramDesc(){}
  TNGramDesc(TSIn& SIn){Fail;}
  void Save(TSOut& SOut) const {Fail;}

  bool operator==(const TNGramDesc& NGramDesc) const {return Sig==NGramDesc.Sig;}
  bool operator<(const TNGramDesc& NGramDesc) const {return Sig<NGramDesc.Sig;}
  int GetPrimHashCd() const {return Sig.GetPrimHashCd();}
  int GetSecHashCd() const {return Sig.GetSecHashCd();}
};

/////////////////////////////////////////////////
// Stream-NGram-Base
ClassTP(TStreamNGramBs, PStreamNGramBs)//{
private:
  TInt MxNGramLen;
  TStrH TokStrH;
  TStreamNGramHash StreamNGramHash;
  UndefCopyAssign(TStreamNGramBs);
private:

  TVec<TIntQ> TokIdQV;
public:
  TStreamNGramBs(const int& _MxNGramLen, const int& MxCachedNGrams):
    MxNGramLen(_MxNGramLen),
    TokStrH(), StreamNGramHash(MxCachedNGrams),
    TokIdQV(_MxNGramLen){
    IAssert(MxNGramLen>=1); TokStrH.AddKey("");}
  static PStreamNGramBs New(const int& MxNGramLen, const int& MxCachedNGrams){
    return PStreamNGramBs(new TStreamNGramBs(MxNGramLen, MxCachedNGrams));}
  TStreamNGramBs(TSIn& SIn):
    MxNGramLen(SIn), TokStrH(SIn), StreamNGramHash(SIn), TokIdQV(MxNGramLen){}
  static PStreamNGramBs Load(TSIn& SIn){return new TStreamNGramBs(SIn);}
  void Save(TSOut& SOut){
    MxNGramLen.Save(SOut); TokStrH.Save(SOut); StreamNGramHash.Save(SOut);}

  bool IsTokStr() const {return TokStrH.Len()>1;}
  TStr GetTokStr(const int& TokId) const {
    return TokStrH.GetKey(TokId);}

  void AddTokId(const int& TokId, const int& TokPos, const bool& OutputNGramsP,
      const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV);
  void AddTokIdV(const TIntV& TokIdV, const int& FirstTokPos, const bool& OutputNGramsP,
      const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV);
  void AddTokStr(const TStr& TokStr, const int& TokPos, const bool& OutputNGramsP,
      const int& MnOutputNGramFq, TNGramDescV& OutputNGramDescV);
  void BreakTokStream();

  void AddDocTokIdV(const TIntV& DocTokIdV, const int& MnOutputNGramFq,
      TNGramDescV& OutputNGramDescV);
  void AddDocHtmlStr(const TStr& DocHtmlStr, const int& MnOutputNGramFq,
      TNGramDescV& OutputNGramDescV);
};

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Unicode-Definition
class TUnicodeDef{
private:
  static TUnicodeDef UnicodeDef;
private:
  TUnicode* Unicode;
  UndefCopyAssign(TUnicodeDef);
public:
  TUnicodeDef(): Unicode(NULL){}
  ~TUnicodeDef(){if (Unicode!=NULL){delete Unicode;} Unicode=NULL;}

  // files
  static TStr GetDfFNm();
  static void Load(){
    Load(GetDfFNm());}
  static void Load(const TStr& FNm){
    try {
      UnicodeDef.Unicode=new TUnicode(FNm);
    } catch(...) {
      fprintf(stderr, "!!! ERROR loading %s; see glib/bin/download.sh\n\n", FNm.CStr());
    }
  }

  // status
  static bool IsDef(){
    return UnicodeDef.Unicode!=NULL;}
  static TUnicode* GetDef(){
    Assert(UnicodeDef.Unicode!=NULL); return UnicodeDef.Unicode;}
};

/////////////////////////////////////////////////
// Unicode-String
class TUStr;
typedef TVec<TUStr> TUStrV;

class TUStr{
private:
  TIntV UniChV;
  TInt PrimHash, SecondHash; 
  static void AssertUnicodeDefOk(){
    EAssertR(TUnicodeDef::IsDef(), "Unicode-Definition-File not loaded!");}
public:
  TUStr(): UniChV(),PrimHash(),SecondHash(){AssertUnicodeDefOk();}
  TUStr(const TUStr& UStr): UniChV(UStr.UniChV){AssertUnicodeDefOk();}
  TUStr(const TIntV& _UniChV): UniChV(_UniChV){AssertUnicodeDefOk();}
  TUStr(const int& MxLen, const int& Len): UniChV(MxLen, Len){}
  TUStr(const TStr& Str);
  ~TUStr(){}
  TUStr(TSIn& SIn): UniChV(SIn){AssertUnicodeDefOk();}
  int GetPrimHashCd() const;
  int UpdatePrimHashCd();
  int GetSecHashCd() const;
  int UpdateSecHashCd();
  void UpdateHashCd();
  void UpdateHashCd(const TInt& UnicodeChar);
  void Reserve(const int& MxLen){this->UniChV = TIntV(MxLen, 0);};
  void Add(const TInt& UnicodeChar){this->UniChV.Add(UnicodeChar);}
  void AddV(const TUStr& UnicodeCharV){int Chs = UnicodeCharV.Len(); for(int ChN = 0; ChN < Chs; ChN++){this->UniChV.Add(UnicodeCharV[ChN]);}}
  void Save(TSOut& SOut) const {UniChV.Save(SOut);}
  void Load(TSIn & SIn){UniChV.Load(SIn);};
  void LoadXml(const PXmlTok& XmlTok, const TStr& Nm);
  void SaveXml(TSOut& SOut, const TStr& Nm) const;

  TUStr& operator=(const TUStr& UStr){
    if (this!=&UStr){UniChV=UStr.UniChV;} return *this;}
  bool operator==(const TUStr& UStr) const {return UniChV==UStr.UniChV;}

  TUStr& operator+=(const TUStr& UStr){UniChV.AddV(UStr.UniChV); return *this;}
  TUStr& operator+=(const TIntV& UStr){UniChV.AddV(UStr); return *this;}
  TUStr& operator+=(const int& UStrChar){UniChV.Add(UStrChar); return *this;}
  TInt& operator[](const int& UniChN) {return UniChV[UniChN];}
  const TInt& operator[](const int& UniChN) const {return UniChV[UniChN];}  

  // basic operations
  void Clr(){UniChV.Clr();}
  int Len() const {return UniChV.Len();}
  bool Empty() const {return UniChV.Empty();}

  // transformations
  void ToLowerCase();
  void ToUpperCase();
  void ToStarterCase();
  void ToUc();//ToCap->ToUc

  // remove redundant spaces, normalize; can remove punctuation
  void ToCompact(bool RemovePunctP = true);
  //Currently no removing of punctuations is supported
  TUStr NormalizeSpaces(bool RemovePunctP = false);

  void GetSubValV(const int& BChN, const int& EChN, TUStr& UniChV);
  TUStr GetSubValV(const int& BChN, const int& EChN);

  // word boundaries
  void GetWordBoundPV(TIntV& WordBoundPosV);
  void GetWordBoundPV(TBoolV& WordBoundPV);
  void GetWordUStrV(TUStrV& UStrV);
  void GetWordUStrV(TUStrV& WordUStrV, TIntV& TerminalV);
  void GetWordUStrV(TUStrV& WordUStrV, TBoolV& TerminalV);
  void GetWordUStrLst(TLst<TUStr>& WordUStrV, TLst<TBool>& TerminalV);
  void GetWordStrV(TStrV& WordStrV);
  bool HasTerminal();

  // conversions to string
  TStr GetStr() const;
  TStr GetStarterStr() const;
  TStr GetStarterLowerCaseStr() const;

  // scripts
  static int GetScriptId(const TStr& ScriptNm);
  static TStr GetScriptNm(const int& ScriptId);
  static int GetChScriptId(const int& UniCh);
  static TStr GetChScriptNm(const int& UniCh);

  // characters
  static TStr GetChNm(const int& UniCh);
  static TStr GetChTypeStr(const int& UniCh);
  static bool IsCase(const int& UniCh);
  static bool IsUpperCase(const int& UniCh);
  static bool IsLowerCase(const int& UniCh);
  static bool IsAlphabetic(const int& UniCh);
  static bool IsMath(const int& UniCh);
  static bool IsNumeric(const int& UniCh);
  static bool IsSpace(const int& UniCh);
  static bool IsTerminal(const int& UniCh);

  bool IsWord();

  // converstions to/from UTF8
  static TStr EncodeUtf8(const int& UniCh);
};

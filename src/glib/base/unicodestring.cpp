/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

/////////////////////////////////////////////////
// Unicode-Definition

TUnicodeDef TUnicodeDef::UnicodeDef;

TStr TUnicodeDef::GetDfFNm(){
  return TSysProc::GetExeFNm().GetFPath()+"UnicodeDef.Bin";
}

/////////////////////////////////////////////////
// Unicode-String
TUStr::TUStr(const TStr& Str){
  AssertUnicodeDefOk();
  TUnicodeDef::GetDef()->DecodeUtf8(Str, UniChV);
  TIntV NfcUniChV; TUnicodeDef::GetDef()->Decompose(UniChV, NfcUniChV, true);
  UniChV=NfcUniChV;
}

void TUStr::ToCompact(bool remove_punct){
	int n  = this->Len();
	int i = 0;
	//Replace All Terminals with spaces
	if(remove_punct){
		while(i < n){
			if(TUStr::IsTerminal(this->UniChV[i])){
				this->UniChV[i] = ' ';
			}
			i++;
		}
	}
	//Compact --- replace consecutive spaces with only one
	bool first_space  = true;
	
	i = 0;
	while(i < n){
		if(TUStr::IsSpace(this->UniChV[i])){//Space encountered
			if(!first_space){
				first_space = true;
			}
			else{//Multiple spaces one after another, compact them
				this->UniChV.Del(i);
				n = n - 1;
				i = i - 1;
			}
		}
		else{//No space reset first_space to false
			if(first_space){
				first_space = false;
			}
		}
		++i;
		if(i == n && first_space){
			--i;
		}
	}
}

TUStr TUStr::NormalizeSpaces(bool remove_punct){
	TUStr SpaceNormalized;
	int n = this->Len();
	SpaceNormalized.Reserve(n);
	bool first_space = true;
	for (int i = 0; i < n; i++){
		TInt UnicodeChar = this->UniChV[i];
		if (TUStr::IsSpace(UnicodeChar)){//Space encountered
			if (!first_space){
				first_space = true;
				SpaceNormalized.Add(' ');
			}
			//Multiple spaces one after another, compact them
		}
		else{//No space reset first_space to false
			if (first_space){
				first_space = false;
			}
			SpaceNormalized.Add(UnicodeChar);
		}	
	}
	return SpaceNormalized;
}

void TUStr::ToLowerCase(){
  TUnicodeDef::GetDef()->ToSimpleLowerCase(UniChV);
}

void TUStr::ToUpperCase(){
  TUnicodeDef::GetDef()->ToSimpleUpperCase(UniChV);
}

void TUStr::ToUc(){
  TUnicodeDef::GetDef()->ToSimpleTitleCase(UniChV);
}

void TUStr::ToStarterCase(){
  TIntV StarterUniChV;
  TUnicodeDef::GetDef()->ExtractStarters(UniChV, StarterUniChV);
  TUnicodeDef::GetDef()->Decompose(StarterUniChV, UniChV, true);
}

void TUStr::GetWordBoundPV(TBoolV& WordBoundPV){
  TUnicodeDef::GetDef()->FindWordBoundaries(UniChV, WordBoundPV);
}

void TUStr::GetWordBoundPV(TIntV& WordBoundPosV){
  TBoolV WordBoundPV;
  TUnicodeDef::GetDef()->FindWordBoundaries(UniChV, WordBoundPV);
  int n = WordBoundPV.Len();
  WordBoundPosV.Reserve(n);
  for(int i = 0; i <= n; i++){
	  if(WordBoundPV[i]){
		WordBoundPosV.Add(i);
	  }
  }
}

void TUStr::GetSubValV(const int& BChN, const int& EChN, TUStr& UniChV){
	for(int ChN = BChN; ChN <= EChN; ChN++){
		UniChV.Add(this->UniChV[ChN]);
	}
}

TUStr TUStr::GetSubValV(const int& BChN, const int& EChN){
	//size_t Len = EChN - BChN + 1;
	TIntV UniChV; 
	int capacity = BChN - EChN + 1;
	if(capacity < this->UniChV.Reserved()){
		UniChV.Reserve(BChN - EChN + 1, 0);
	}
	this->UniChV.GetSubValV(BChN, EChN, UniChV);
	return TUStr(UniChV);
}

void TUStr::GetWordUStrV(TUStrV& WordUStrV, TIntV& TerminalV){
  // clear word vector
  WordUStrV.Clr();
  // create boundaries
  TBoolV WordBoundPV; GetWordBoundPV(WordBoundPV);
  IAssert(Len()==WordBoundPV.Len()-1);
  IAssert((WordBoundPV.Len()>0)&&(WordBoundPV.Last()));
  // traverse characters and bounds
  int UniChs=Len(); TIntV WordUniChV;
  bool terminal = false;

  for (int UniChN=0; UniChN<=UniChs; UniChN++){
    if ((UniChN==UniChs)||(WordBoundPV[UniChN+1])){ // finish or word-boundary
      if (UniChN<UniChs){ // if not finish
        // if last-word-char or single-alphabetic-char
        if ((!WordUniChV.Empty())||(IsAlphabetic(UniChV[UniChN]))){
          WordUniChV.Add(UniChV[UniChN]); // add char
        }
		else{
			if(WordUStrV.Len() > 0){
				if(IsTerminal(UniChV[UniChN])) terminal = true;
			}
		}
      }
      if (!WordUniChV.Empty()){ // add current word to vector
        TUStr WordUStr(WordUniChV); // construct word from char-vector
        WordUStrV.Add(WordUStr); // add word to word-vector
        WordUniChV.Clr(false); // clear char-vector
		if(terminal){ TerminalV.Add(WordUStrV.Len() - 1);}
		terminal = false;
      }
    } else {
      // add character to char-vector
      WordUniChV.Add(UniChV[UniChN]);
    }
  }
}

void TUStr::GetWordUStrV(TUStrV& WordUStrV, TBoolV& TerminalV){
  // clear word vector
  WordUStrV.Clr();
  // create boundaries
  TBoolV WordBoundPV; GetWordBoundPV(WordBoundPV);
  TerminalV.Reserve(WordBoundPV.Len());
  IAssert(Len()==WordBoundPV.Len()-1);
  IAssert((WordBoundPV.Len()>0)&&(WordBoundPV.Last()));
  // traverse characters and bounds
  int UniChs=Len(); TIntV WordUniChV;
  bool terminal = false;

  for (int UniChN=0; UniChN<=UniChs; UniChN++){
    if ((UniChN==UniChs)||(WordBoundPV[UniChN+1])){ // finish or word-boundary
      if (UniChN<UniChs){ // if not finish
        // if last-word-char or single-alphabetic-char
        if ((!WordUniChV.Empty())||(IsAlphabetic(UniChV[UniChN]))){
          WordUniChV.Add(UniChV[UniChN]); // add char
        }
		else{
			if(WordUStrV.Len() > 0){
				if(IsTerminal(UniChV[UniChN])) terminal = true;
			}
		}
      }
      if (!WordUniChV.Empty()){ // add current word to vector
        TUStr WordUStr(WordUniChV); // construct word from char-vector
        WordUStrV.Add(WordUStr); // add word to word-vector
        WordUniChV.Clr(false); // clear char-vector
		if(terminal){ TerminalV.Add(true);}
		else{ TerminalV.Add(false);}
		terminal = false;
      }
    } else {
      // add character to char-vector
      WordUniChV.Add(UniChV[UniChN]);
    }
  }
}

void TUStr::GetWordUStrLst(TLst<TUStr>& WordUStrV, TLst<TBool> &TerminalV){ //TBoolV& TerminalV){
  
// clear word vector
  WordUStrV.Clr();
  // create boundaries
  TBoolV WordBoundPV; GetWordBoundPV(WordBoundPV);
  //TerminalV.Reserve(WordBoundPV.Len());
  IAssert(Len()==WordBoundPV.Len()-1);
  IAssert((WordBoundPV.Len()>0)&&(WordBoundPV.Last()));
  // traverse characters and bounds
  int UniChs=Len(); TIntV WordUniChV;
  bool terminal = false;

  for (int UniChN=0; UniChN<=UniChs; UniChN++){
    if ((UniChN==UniChs)||(WordBoundPV[UniChN+1])){ // finish or word-boundary
      if (UniChN<UniChs){ // if not finish
        // if last-word-char or single-alphabetic-char
        if ((!WordUniChV.Empty())||(IsAlphabetic(UniChV[UniChN]))){
          WordUniChV.Add(UniChV[UniChN]); // add char
        }
		else{
			if(WordUStrV.Len() > 0){
				if(IsTerminal(UniChV[UniChN])) terminal = true;
			}
		}
      }
      if (!WordUniChV.Empty()){ // add current word to vector
        TUStr WordUStr(WordUniChV); // construct word from char-vector
		WordUStrV.AddBack(WordUStr); // add word to word-vector
        WordUniChV.Clr(false); // clear char-vector
		if(terminal){ TerminalV.AddBack(true);}
		else{ TerminalV.AddBack(false);}
		terminal = false;
      }
    } else {
      // add character to char-vector
      WordUniChV.Add(UniChV[UniChN]);
    }
  }
}


void TUStr::GetWordUStrV(TUStrV& WordUStrV){
  // clear word vector
  WordUStrV.Clr();
  // create boundaries
  TBoolV WordBoundPV; GetWordBoundPV(WordBoundPV);
  IAssert(Len()==WordBoundPV.Len()-1);
  IAssert((WordBoundPV.Len()>0)&&(WordBoundPV.Last()));
  // traverse characters and bounds
  int UniChs=Len(); TIntV WordUniChV;
  for (int UniChN=0; UniChN<=UniChs; UniChN++){
    if ((UniChN==UniChs)||(WordBoundPV[UniChN+1])){ // finish or word-boundary
      if (UniChN<UniChs){ // if not finish
        // if last-word-char or single-alphabetic-char
        if ((!WordUniChV.Empty())||(IsAlphabetic(UniChV[UniChN]))){
          WordUniChV.Add(UniChV[UniChN]); // add char
        }
      }
      if (!WordUniChV.Empty()){ // add current word to vector
        TUStr WordUStr(WordUniChV); // construct word from char-vector
        WordUStrV.Add(WordUStr); // add word to word-vector
        WordUniChV.Clr(false); // clear char-vector
      }
    } else {
      // add character to char-vector
      WordUniChV.Add(UniChV[UniChN]);
    }
  }
}

void TUStr::GetWordStrV(TStrV& WordStrV){
  // clear word vector
  TUStrV WordUStrV;
  //WordUStrV.Clr();
  // create boundaries
  TBoolV WordBoundPV; GetWordBoundPV(WordBoundPV);
  IAssert(Len()==WordBoundPV.Len()-1);
  IAssert((WordBoundPV.Len()>0)&&(WordBoundPV.Last()));
  // traverse characters and bounds
  int UniChs=Len(); TIntV WordUniChV;
  for (int UniChN=0; UniChN<=UniChs; UniChN++){
    if ((UniChN==UniChs)||(WordBoundPV[UniChN+1])){ // finish or word-boundary
      if (UniChN<UniChs){ // if not finish
        // if last-word-char or single-alphabetic-char
        if ((!WordUniChV.Empty())||(IsAlphabetic(UniChV[UniChN]))){
          WordUniChV.Add(UniChV[UniChN]); // add char
        }
      }
      if (!WordUniChV.Empty()){ // add current word to vector
        TUStr WordUStr(WordUniChV); // construct word from char-vector
        WordStrV.Add(WordUStr.GetStr()); // add word to word-vector
        WordUniChV.Clr(false); // clear char-vector
      }
    } else {
      // add character to char-vector
      WordUniChV.Add(UniChV[UniChN]);
    }
  }
}

TStr TUStr::GetStr() const {
  TStr Str=TUnicodeDef::GetDef()->EncodeUtf8Str(UniChV);
  return Str;
}

TStr TUStr::GetStarterStr() const {
  TIntV UniChV1; TIntV UniChV2;
  TUnicodeDef::GetDef()->ExtractStarters(UniChV, UniChV1);
  TUnicodeDef::GetDef()->Decompose(UniChV1, UniChV2, true);
  TStr Str=TUnicodeDef::GetDef()->EncodeUtf8Str(UniChV2);
  return Str;
}

TStr TUStr::GetStarterLowerCaseStr() const {
  TIntV UniChV1; TIntV UniChV2; TIntV UniChV3;
  TUnicodeDef::GetDef()->GetSimpleLowerCase(UniChV, UniChV1);
  TUnicodeDef::GetDef()->ExtractStarters(UniChV1, UniChV2);
  TUnicodeDef::GetDef()->Decompose(UniChV2, UniChV3, true);
  TStr Str=TUnicodeDef::GetDef()->EncodeUtf8Str(UniChV3);
  return Str;
}

int TUStr::GetScriptId(const TStr& ScriptNm){
  return TUnicodeDef::GetDef()->ucd.GetScriptByName(ScriptNm);
}

TStr TUStr::GetScriptNm(const int& ScriptId){
  return TUnicodeDef::GetDef()->ucd.GetScriptName(ScriptId);
}

int TUStr::GetChScriptId(const int& UniCh){
  return TUnicodeDef::GetDef()->ucd.GetScript(UniCh);
}

TStr TUStr::GetChScriptNm(const int& UniCh){
  return GetScriptNm(GetChScriptId(UniCh));
}

TStr TUStr::GetChNm(const int& UniCh){
  TStr UniChNm(TUnicodeDef::GetDef()->ucd.GetCharNameS(UniCh));
  return UniChNm;
}

TStr TUStr::GetChTypeStr(const int& UniCh){
  TChA ChTypeChA;
  ChTypeChA+='[';
  if (IsCase(UniCh)){ChTypeChA+="Case,";}
  if (IsUpperCase(UniCh)){ChTypeChA+="UpperCase,";}
  if (IsLowerCase(UniCh)){ChTypeChA+="LowerCase,";}
  if (IsAlphabetic(UniCh)){ChTypeChA+="Alphabetic,";}
  if (IsMath(UniCh)){ChTypeChA+="Math,";}
  if (ChTypeChA.LastCh()=='['){ChTypeChA+=']';}
  else {ChTypeChA[ChTypeChA.Len()-1]=']';}
  return ChTypeChA;
}

bool TUStr::IsTerminal(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
	  return ChInfo.IsTerminalPunctuation();}//What about nonterminal punctuation
  else {return false;}
}

bool TUStr::HasTerminal(){
	for(int i = 0; i < Len(); i++){
		if(TUStr::IsTerminal(this->UniChV[i]) || this->UniChV[i] == 58){
			return true;
		}
	}
	return false;
}

bool TUStr::IsSpace(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
	  return ChInfo.IsWhiteSpace();}
  else {return false;}
}

bool TUStr::IsCase(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
    return ChInfo.IsCased();}
  else {return false;}
}

bool TUStr::IsUpperCase(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
    return ChInfo.IsUppercase();}
  else {return false;}
}

bool TUStr::IsLowerCase(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
    return ChInfo.IsLowercase();}
  else {return false;}
}

bool TUStr::IsAlphabetic(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
    return ChInfo.IsAlphabetic();}
  else {return false;}
}

bool TUStr::IsNumeric(const int& UniCh){
	return (UniCh > '0') && (UniCh < '9');
}

bool TUStr::IsMath(const int& UniCh){
  TUniChInfo ChInfo;
  if (TUnicodeDef::GetDef()->ucd.IsGetChInfo(UniCh, ChInfo)){
    return ChInfo.IsMath();}
  else {return false;}
}

bool TUStr::IsWord(){
  for(int i = 0; i < this->Len(); i++){
		if(!this->IsAlphabetic(UniChV[i])){
			return false;	
		}
  }
  return true;
}

TStr TUStr::EncodeUtf8(const int& UniCh) {
  AssertUnicodeDefOk();
  return TUnicodeDef::GetDef()->EncodeUtf8Str(TIntV::GetV(UniCh));
}

int TUStr::GetPrimHashCd() const{
	if(this->PrimHash != 0){
		return this->PrimHash;
	}
    int hash = 0;
	int len = this->Len();
	int inc =  1;
	for(int i = 0; i < len; i = i + inc){
		hash = (hash * 37) + this->UniChV[i].GetPrimHashCd();
	}
	return hash;
}

int TUStr::UpdatePrimHashCd(){
	if(this->PrimHash != 0){
		return this->PrimHash;
	}
    int hash = 0;
	int len = this->Len();
	int inc =  1;
	for(int i = 0; i < len; i = i + inc){
		hash = (hash * 37) + this->UniChV[i].GetPrimHashCd();
	}
	this->PrimHash = hash;
	return hash;
}

int TUStr::GetSecHashCd() const{
	if(this->SecondHash != 0){
		return this->SecondHash;
	}
    int hash = 0;
	int len = this->Len();
	int inc = ((len - 32) / 32) + 1;
	for(int i = 0; i < len; i = i + inc){
		hash = (hash * 37) + this->UniChV[i].GetSecHashCd();
	}
	return hash;
}

int TUStr::UpdateSecHashCd(){
	if(this->SecondHash != 0){
		return this->SecondHash;
	}
    int hash = 0;
	int len = this->Len();
	int inc = ((len - 32) / 32) + 1;
	for(int i = 0; i < len; i = i + inc){
		hash = (hash * 37) + this->UniChV[i].GetSecHashCd();
	}
	this->SecondHash = hash;
	return hash;
}

void TUStr::UpdateHashCd(){
	this->UpdatePrimHashCd();
	this->UpdateSecHashCd();
}

void TUStr::UpdateHashCd(const TInt& UnicodeChar){
	this->PrimHash  = (this->PrimHash * 37) + UnicodeChar.GetPrimHashCd();
	this->SecondHash  = (this->SecondHash * 37) + UnicodeChar.GetSecHashCd();
}
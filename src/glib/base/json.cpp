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
// Json-Value
TJsonVal::TJsonVal(TSIn& SIn):
  JsonValType((TJsonValType)(TInt(SIn).Val)), Bool(SIn), 
  Num(SIn), Str(SIn), ValV(SIn), KeyValH(SIn) { }

void TJsonVal::Save(TSOut& SOut) const {
  TInt((int)JsonValType).Save(SOut);
  Bool.Save(SOut); Num.Save(SOut);
  Str.Save(SOut); ValV.Save(SOut);
  KeyValH.Save(SOut);
}

TStr TJsonVal::SaveStr() { 
  return GetStrFromVal(this); 
}

bool TJsonVal::operator==(const TJsonVal& JsonVal) const {
  return JsonValType == JsonVal.JsonValType &&
    Bool == JsonVal.Bool &&
    Num == JsonVal.Num &&
    Str == JsonVal.Str &&
    ValV == JsonVal.ValV && 
    KeyValH == JsonVal.KeyValH;    
}

bool TJsonVal::operator!=(const TJsonVal& JsonVal) const {
  return !(*this == JsonVal);
}

void TJsonVal::AddToObj(const PJsonVal& Val) {
	EAssert(Val->IsObj());
	int KeyId = Val->KeyValH.FFirstKeyId();
	while (Val->KeyValH.FNextKeyId(KeyId)) {
		AddToObj(Val->KeyValH.GetKey(KeyId), Val->KeyValH[KeyId]);
	}
}

PJsonVal TJsonVal::NewArr(const TJsonValV& ValV) {
	PJsonVal Val = TJsonVal::NewArr();
	for (int ValN = 0; ValN < ValV.Len(); ValN++) {
		Val->AddToArr(ValV[ValN]);
	}
	return Val;
}

PJsonVal TJsonVal::NewArr(const TIntV& IntV) {
	PJsonVal Val = TJsonVal::NewArr();
	for (int IntN = 0; IntN < IntV.Len(); IntN++) {
		Val->AddToArr(TJsonVal::NewNum((double)IntV[IntN]));
	}
	return Val;
}

PJsonVal TJsonVal::NewArr(const TFltV& FltV) {
	PJsonVal Val = TJsonVal::NewArr();
	for (int FltN = 0; FltN < FltV.Len(); FltN++) {
		Val->AddToArr(TJsonVal::NewNum(FltV[FltN]));
	}
	return Val;
}

PJsonVal TJsonVal::NewArr(const double& Val1, const double& Val2) {
    PJsonVal Val = TJsonVal::NewArr();
    Val->AddToArr(TJsonVal::NewNum(Val1));
    Val->AddToArr(TJsonVal::NewNum(Val2));
	return Val;
}

PJsonVal TJsonVal::NewArr(const TStrV& StrV) {
	PJsonVal Val = TJsonVal::NewArr();
	for (int StrN = 0; StrN < StrV.Len(); StrN++) {
		Val->AddToArr(TJsonVal::NewStr(StrV[StrN]));
	}
	return Val;
}

PJsonVal TJsonVal::NewArr(const TFltPr& FltPr) {
  PJsonVal Val = TJsonVal::NewArr();
  Val->AddToArr(TJsonVal::NewNum(FltPr.Val1));
  Val->AddToArr(TJsonVal::NewNum(FltPr.Val2));
  return Val;
}

void TJsonVal::GetArrStrV(TStrV& StrV) const {
    EAssert(IsArr());
    for (int StrN = 0; StrN < GetArrVals(); StrN++) {
        PJsonVal ArrVal = GetArrVal(StrN);
        EAssert(ArrVal->IsStr());
        StrV.Add(ArrVal->GetStr());
    }
}

PJsonVal TJsonVal::GetObjKey(const TStr& Key) const {
  EAssert(IsObj());
  EAssertR(IsObjKey(Key), "Unknown key " + Key); 
  return KeyValH.GetDat(Key);
}

PJsonVal TJsonVal::GetObjKey(const char *Key) const {
  EAssert(IsObj());
  EAssertR(IsObjKey(Key), TStr::Fmt("Unknown key %s", Key)); 
  return KeyValH.GetDat(Key);
}

bool TJsonVal::GetObjBool(const TStr& Key, const bool& DefBool) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetBool() : DefBool;
}

bool TJsonVal::GetObjBool(const char *Key, const bool& DefBool) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetBool() : DefBool;
}

double TJsonVal::GetObjNum(const TStr& Key, const double& DefNum) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetNum() : DefNum;
} 

double TJsonVal::GetObjNum(const char *Key, const double& DefNum) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetNum() : DefNum;
}

int TJsonVal::GetObjInt(const TStr& Key, const int& DefInt) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetInt() : DefInt;
} 

int TJsonVal::GetObjInt(const char *Key, const int& DefInt) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetInt() : DefInt;
}

TStr TJsonVal::GetObjStr(const TStr& Key, const TStr& DefStr) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetStr() : DefStr;
}

TStr TJsonVal::GetObjStr(const char *Key, const TStr& DefStr) const { 
  EAssert(IsObj());
  return (IsObjKey(Key)) ? KeyValH.GetDat(Key)->GetStr() : DefStr;
}

void TJsonVal::GetObjStrV(const TStr& Key, TStrV& StrV) const {
    EAssert(IsObj());
    EAssert(IsObjKey(Key));
    GetObjKey(Key)->GetArrStrV(StrV);
}

void TJsonVal::GetObjStrV(const char *Key, TStrV& StrV) const {
    EAssert(IsObj());
    EAssert(IsObjKey(Key));
    GetObjKey(Key)->GetArrStrV(StrV);
}
  
PJsonVal TJsonVal::GetValFromLx(TILx& Lx){
  static TFSet ValExpect=TFSet()|syIdStr|syFlt|syQStr|syLBracket|syLBrace|syRBracket;
  PJsonVal Val=TJsonVal::New();
  if ((Lx.Sym==syIdStr)&&(Lx.Str=="null")){
    Val->PutNull(); Lx.GetSym();
  } else if ((Lx.Sym==syIdStr)&&(Lx.Str=="true")){
    Val->PutBool(true); Lx.GetSym();
  } else if ((Lx.Sym==syIdStr)&&(Lx.Str=="false")){
    Val->PutBool(false); Lx.GetSym();
  } else if (Lx.Sym==syFlt){
    Val->PutNum(Lx.Flt); Lx.GetSym();
  } else if (Lx.Sym==syQStr){
    Val->PutStr(Lx.Str); Lx.GetSym();
  } else if (Lx.Sym==syLBracket){
    Val->PutArr(); Lx.GetSym(ValExpect); // added ValExpect to correctyl parse arrays of floats
    if (Lx.Sym!=syRBracket){
      forever{
        PJsonVal SubVal=TJsonVal::GetValFromLx(Lx);
        Val->AddToArr(SubVal);
        if (Lx.Sym==syComma){Lx.GetSym(ValExpect);} 
        else if (Lx.Sym==syRBracket){break;} 
        else {TExcept::Throw("JSON Array not properly formed.");}
      }
    }
    Lx.GetSym();
  } else if (Lx.Sym==syLBrace){
    Val->PutObj(); Lx.GetSym(TFSet()|syRBrace|syQStr);
    if (Lx.Sym!=syRBrace){
      forever{
        TStr SubKey=Lx.Str; 
        Lx.GetSym(syColon); 
        Lx.GetSym(ValExpect);
        PJsonVal SubVal=TJsonVal::GetValFromLx(Lx);
        Val->AddToObj(SubKey, SubVal);
        if (Lx.Sym==syComma){Lx.GetSym(TFSet()|syQStr);} 
        else if (Lx.Sym==syRBrace){break;} 
        else {TExcept::Throw("JSON Object not properly formed.");}
      }
    }
    Lx.GetSym();
  } else {
    TExcept::Throw("Unexpected JSON symbol.");
  }
  return Val;
}

PJsonVal TJsonVal::GetValFromSIn(const PSIn& SIn, bool& Ok, TStr& MsgStr){
  TILx Lx(SIn, TFSet()|iloCmtAlw|iloCsSens|iloExcept|iloSigNum);
  PJsonVal Val;
  try {
    Lx.GetSym(TFSet()|syLBracket|syLBrace);
    Val=GetValFromLx(Lx);
	Ok=true; TStr MsgStr="Ok";
  }
  catch (PExcept Except){
    Val=TJsonVal::New();
    Ok=false; MsgStr=Except->GetMsgStr();
  }
  return Val;
}

PJsonVal TJsonVal::GetValFromSIn(const PSIn& SIn){
  bool Ok = true; TStr MsgStr = "";
  return GetValFromSIn(SIn, Ok, MsgStr);
}

PJsonVal TJsonVal::GetValFromStr(const TStr& JsonStr, bool& Ok, TStr& MsgStr){
  PSIn SIn=TStrIn::New(JsonStr);
  return GetValFromSIn(SIn, Ok, MsgStr);
}

PJsonVal TJsonVal::GetValFromStr(const TStr& JsonStr){
  PSIn SIn=TStrIn::New(JsonStr);
  return GetValFromSIn(SIn);
}

void TJsonVal::AddEscapeChAFromStr(const TStr& Str, TChA& ChA){
	if (TUnicodeDef::IsDef()) {
		// parse the UTF8 string
		TIntV UStr; TUnicodeDef::GetDef()->DecodeUtf8(Str, UStr);
		// escape the string
		for (int ChN = 0; ChN < UStr.Len(); ChN++) {
			const int UCh = UStr[ChN];
			if (UCh < 0x80) {
				// 7-bit ascii
				const char Ch = (char)UCh;
				switch (Ch) {
					case '"' : ChA.AddCh('\\'); ChA.AddCh('"'); break;
					case '\\' : ChA.AddCh('\\'); ChA.AddCh('\\'); break;
					case '/' : ChA.AddCh('\\'); ChA.AddCh('/'); break;
					case '\b' : ChA.AddCh('\\'); ChA.AddCh('b'); break;
					case '\f' : ChA.AddCh('\\'); ChA.AddCh('f'); break;
					case '\n' : ChA.AddCh('\\'); ChA.AddCh('n'); break;
					case '\r' : ChA.AddCh('\\'); ChA.AddCh('r'); break;
					case '\t' : ChA.AddCh('\\'); ChA.AddCh('t'); break;
					default :
						ChA.AddCh(Ch);
				}
			} else {
				// escape
				ChA += "\\u";
				ChA += TStr::Fmt("%04x", UCh);
			}
		}
	} else {
		// escape the string
		for (int ChN = 0; ChN < Str.Len(); ChN++) {
			const char Ch = Str[ChN];
			if ((Ch & 0x80) == 0) {
				// 7-bit ascii
				switch (Ch) {
					case '"' : ChA.AddCh('\\'); ChA.AddCh('"'); break;
					case '\\' : ChA.AddCh('\\'); ChA.AddCh('\\'); break;
					case '/' : ChA.AddCh('\\'); ChA.AddCh('/'); break;
					case '\b' : ChA.AddCh('\\'); ChA.AddCh('b'); break;
					case '\f' : ChA.AddCh('\\'); ChA.AddCh('f'); break;
					case '\n' : ChA.AddCh('\\'); ChA.AddCh('n'); break;
					case '\r' : ChA.AddCh('\\'); ChA.AddCh('r'); break;
					case '\t' : ChA.AddCh('\\'); ChA.AddCh('t'); break;
					default : ChA.AddCh(Ch);
				}
			} else {
				// escape
				ChA += "\\u";
				ChA += TStr::Fmt("%02x", (int)Ch);
			}
		}
	}
}

void TJsonVal::AddQChAFromStr(const TStr& Str, TChA& ChA){
  ChA+="\"";
  AddEscapeChAFromStr(Str, ChA);
  ChA+="\"";
}

void TJsonVal::GetChAFromVal(const PJsonVal& Val, TChA& ChA){
  switch (Val->GetJsonValType()){
    case jvtNull: 
      ChA+="null"; break;
    case jvtBool:
      if (Val->GetBool()){ChA+="true";} else {ChA+="false";} break;
    case jvtNum: 
      ChA+=TStr::Fmt("%f", Val->GetNum()); break;
    case jvtStr:
      AddQChAFromStr(Val->GetStr(), ChA); break;
    case jvtArr:
      ChA+="[";
      for (int ArrValN=0; ArrValN<Val->GetArrVals(); ArrValN++){
        if (ArrValN>0){ChA+=", ";}
        GetChAFromVal(Val->GetArrVal(ArrValN), ChA);
      }
      ChA+="]"; 
      break;
    case jvtObj:
      ChA+="{";
      for (int ObjKeyN=0; ObjKeyN<Val->GetObjKeys(); ObjKeyN++){
        if (ObjKeyN>0){ChA+=", ";}
        TStr ObjKey; PJsonVal ObjVal; Val->GetObjKeyVal(ObjKeyN, ObjKey, ObjVal);
        AddQChAFromStr(ObjKey, ChA);
        ChA+=":";
        GetChAFromVal(ObjVal, ChA);
      }
      ChA+="}"; 
      break;
	default: TExcept::Throw("Error serializing json to string");
  }
}

TStr TJsonVal::GetStrFromVal(const PJsonVal& Val){
  TChA ChA;
  GetChAFromVal(Val, ChA);
  return ChA;
}

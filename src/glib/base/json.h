/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "bd.h"

/////////////////////////////////////////////////
// Json-Value

typedef enum {
  jvtUndef, jvtNull, jvtBool, jvtNum, jvtStr, jvtArr, jvtObj} TJsonValType;

// forward declarations
class TJsonVal;
typedef TPt<TJsonVal> PJsonVal;
typedef TVec<PJsonVal> TJsonValV;

class TJsonVal {
private:
  TCRef CRef;
public:
  friend class TPt<TJsonVal>;
private:
  TJsonValType JsonValType;
  TBool Bool; 
  TFlt Num; 
  TStr Str;
  TJsonValV ValV;
  THash<TStr, PJsonVal> KeyValH;
  UndefCopyAssign(TJsonVal);
public:
  TJsonVal(): JsonValType(jvtUndef){}
  static PJsonVal New(){
    return new TJsonVal();}
  TJsonVal(TSIn& SIn);
  static PJsonVal Load(TSIn& SIn){return new TJsonVal(SIn);}
  void Save(TSOut& SOut) const;
  TStr SaveStr();

  bool operator==(const TJsonVal& JsonVal) const;
  bool operator!=(const TJsonVal& JsonVal) const;

  // putting value
  void PutNull(){JsonValType=jvtNull;}
  void PutBool(const bool& BoolArg){ JsonValType = jvtBool; Bool = BoolArg; }
  void PutNum(const double& _Num){JsonValType=jvtNum; Num=_Num;}
  void PutStr(const TStr& _Str){JsonValType=jvtStr; Str=_Str;}
  void PutArr(){JsonValType=jvtArr;}
  void AddToArr(const PJsonVal& Val){
    EAssert(JsonValType==jvtArr); ValV.Add(Val);}
  void SetArrVal(const int& ValN, const PJsonVal& Val){
	  EAssert(JsonValType==jvtArr&&ValN < GetArrVals()); ValV[ValN] = Val;}
  void AddToArr(const int& Val){ AddToArr(NewNum((double)Val)); }
  void AddToArr(const uint& Val){ AddToArr(NewNum((double)Val)); }
  void AddToArr(const double& Val){ AddToArr(NewNum(Val)); }
  void AddToArr(const double& Val1, const double& Val2){ AddToArr(NewArr(Val1, Val2)); }
  void AddToArr(const TStr& Val){ AddToArr(NewStr(Val)); }
  void AddToArr(const char* Val){ AddToArr(NewStr(Val)); }
  void AddToArr(const bool& Val){ AddToArr(NewBool(Val)); }
  void AddToArr(const TJsonValV& ValV){ AddToArr(NewArr(ValV)); }
  void PutObj(){JsonValType=jvtObj;}
  void AddToObj(const TStr& KeyNm, const PJsonVal& Val);
  void AddToObj(const TStr& KeyNm, const int& Val){ AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const uint& Val){ AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const uint64& Val){ AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const int64& Val) { AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const double& Val){ AddToObj(KeyNm, NewNum(Val)); }
  void AddToObj(const TStr& KeyNm, const double& Val1, const double& Val2){ AddToObj(KeyNm, NewArr(Val1, Val2)); }
  void AddToObj(const TStr& KeyNm, const TStr& Val){ AddToObj(KeyNm, NewStr(Val)); }
  void AddToObj(const TStr& KeyNm, const char* Val){ AddToObj(KeyNm, NewStr(Val)); }
  void AddToObj(const TStr& KeyNm, const bool& Val){ AddToObj(KeyNm, NewBool(Val)); }
  void AddToObj(const TStr& KeyNm, const TJsonValV& ValV){ AddToObj(KeyNm, NewArr(ValV)); }
  void AddToObj(const PJsonVal& Val);
  void MergeObj(const PJsonVal& Val);
  
  // simplified creation of basic elements
  static PJsonVal NewNull() { PJsonVal Val = TJsonVal::New(); Val->PutNull(); return Val; }
  static PJsonVal NewBool(const bool& Bool) { PJsonVal Val = TJsonVal::New(); Val->PutBool(Bool); return Val; }
  static PJsonVal NewNum(const double& Num) { PJsonVal Val = TJsonVal::New(); Val->PutNum(Num); return Val; }
  static PJsonVal NewStr(const TStr& Str) { PJsonVal Val = TJsonVal::New(); Val->PutStr(Str); return Val; }
  static PJsonVal NewArr() { PJsonVal Val = TJsonVal::New(); Val->PutArr(); return Val; }
  static PJsonVal NewArr(const TJsonValV& ValV);
  static PJsonVal NewArr(const TIntV& IntV);
  static PJsonVal NewArr(const TUInt64V& IntV);
  static PJsonVal NewArr(const TFltV& FltV);
  static PJsonVal NewArr(const TIntFltKdV& IntFltKdV);
  static PJsonVal NewArr(const double& Val1, const double& Val2);
  static PJsonVal NewArr(const TStrV& StrV);
  static PJsonVal NewArr(const TFltPr& FltPr);
  static PJsonVal NewObj() { PJsonVal Val = TJsonVal::New(); Val->PutObj(); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const PJsonVal& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const int& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const uint64& ObjVal) {
      PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const double& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const TStr& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const bool& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }

  // testing value-type
  TJsonValType GetJsonValType() const {return JsonValType;}
  bool IsDef() const {return JsonValType!=jvtUndef;}
  bool IsNull() const {return JsonValType==jvtNull;}
  bool IsBool() const {return JsonValType==jvtBool;}
  bool IsNum() const {return JsonValType==jvtNum;}
  bool IsStr() const {return JsonValType==jvtStr;}
  bool IsArr() const {return JsonValType==jvtArr;}
  bool IsObj() const {return JsonValType==jvtObj;}
  bool IsTm() const;

  // getting value
  bool GetBool() const {EAssert(IsBool()); return Bool;}
  double GetNum() const {EAssert(IsNum()); return Num;}
  int GetInt() const {EAssert(IsNum()); return TFlt::Round(Num);}
  uint64 GetUInt64() const {EAssert(IsNum()); return (unsigned long long)(int64)(Num);}
  uint GetUInt() const { EAssert(IsNum()); return uint(Num); }
  int64 GetInt64() const { EAssert(IsNum()); return int64(Num); }
  const TStr& GetStr() const {EAssert(IsStr()); return Str;}
  uint64 GetTmMSecs() const { return TTm::GetMSecsFromTm(GetTm()); }
  TTm GetTm() const;

  int GetArrVals() const {EAssert(IsArr()); return ValV.Len();}
  PJsonVal GetArrVal(const int& ValN) const {return ValV[ValN];}
  void GetArrNumV(TFltV& FltV) const;
  void GetArrNumSpV(TIntFltKdV& NumSpV) const;
  void GetArrIntV(TIntV& IntV) const;
  void GetArrUInt64V(TUInt64V& IntV) const;
  void GetArrStrV(TStrV& StrV) const;
  int GetObjKeys() const {EAssert(IsObj()); return KeyValH.Len();}
  void GetObjKeyVal(const int& KeyValN, TStr& Key, PJsonVal& Val) const {
    EAssert(IsObj()); Key=KeyValH.GetKey(KeyValN); Val=KeyValH[KeyValN];}
  const TStr& GetObjKey(const int& KeyValN) const {
	  EAssert(IsObj()); return KeyValH.GetKey(KeyValN);}
  bool IsObjKey(const TStr& Key) const {EAssert(IsObj()); return KeyValH.IsKey(Key);}
  bool IsObjKey(const char *Key) const {EAssert(IsObj()); return KeyValH.IsKey(Key);}
  PJsonVal GetObjKey(const TStr& Key) const;
  PJsonVal GetObjKey(const char *Key) const;
  bool GetObjBool(const TStr& Key) const { return GetObjKey(Key)->GetBool(); }
  bool GetObjBool(const char *Key) const { return GetObjKey(Key)->GetBool(); }
  double GetObjNum(const TStr& Key) const { return GetObjKey(Key)->GetNum(); }
  double GetObjNum(const char *Key) const { return GetObjKey(Key)->GetNum(); }
  int GetObjInt(const TStr& Key) const { return GetObjKey(Key)->GetInt(); }
  int GetObjInt(const char *Key) const { return GetObjKey(Key)->GetInt(); }
  uint64 GetObjUInt64(const TStr& Key) const { return GetObjKey(Key)->GetUInt64(); }
  uint64 GetObjUInt64(const char *Key) const { return GetObjKey(Key)->GetUInt64(); }
  int64 GetObjInt64(const char* Key) const { return GetObjKey(Key)->GetInt64(); }
  int64 GetObjInt64(const TStr& Key) const { return GetObjInt64(Key.CStr()); }
  const TStr& GetObjStr(const TStr& Key) const { return GetObjKey(Key)->GetStr(); }
  const TStr& GetObjStr(const char *Key) const { return GetObjKey(Key)->GetStr(); }
  TTm GetObjTm(const char* Key) const { return GetObjKey(Key)->GetTm(); }
  TTm GetObjTm(const TStr& Key) const { return GetObjTm(Key.CStr()); }
  uint64 GetObjTmMSecs(const char* Key) const { return GetObjKey(Key)->GetTmMSecs(); }
  uint64 GetObjTmMSecs(const TStr& Key) const { return GetObjTmMSecs(Key.CStr()); }
  bool GetObjBool(const TStr& Key, const bool& DefBool) const;
  bool GetObjBool(const char *Key, const bool& DefBool) const;
  double GetObjNum(const TStr& Key, const double& DefNum) const;
  double GetObjNum(const char *Key, const double& DefNum) const;
  int GetObjInt(const TStr& Key, const int& DefNum) const;
  int GetObjInt(const char *Key, const int& DefNum) const;
  int64 GetObjInt64(const TStr& Key, const int64& DefNum) const;
  int64 GetObjInt64(const char *Key, const int64& DefNum) const;
  uint64 GetObjUInt64(const TStr& Key, const uint64& DefNum) const;
  uint64 GetObjUInt64(const char *Key, const uint64& DefNum) const;
  void GetObjIntV(const TStr& Key, TIntV& IntV) const;
  void GetObjUInt64V(const TStr& Key, TUInt64V& UInt64V) const;
  void GetObjFltV(const TStr& Key, TFltV& FltV) const;
  const TStr& GetObjStr(const TStr& Key, const TStr& DefStr) const;
  const TStr& GetObjStr(const char *Key, const TStr& DefStr) const;
  void GetObjStrV(const TStr& Key, TStrV& StrV) const;
  void GetObjStrV(const char *Key, TStrV& StrV) const;

  // removing value
  void DelObjKey(const TStr& Key) { EAssert(IsObj()); KeyValH.DelIfKey(Key); /*KeyValH.Defrag();*/ }
  void DelObjKey(const char *Key) { EAssert(IsObj()); KeyValH.DelIfKey(Key); /*KeyValH.Defrag();*/ }
  void DelArrVal(const int& ValN) { EAssert(IsArr()); ValV.Del(ValN); }

  // validation
  void AssertObjKey(const TStr& Key, const TStr& Fun);
  void AssertObjKeyStr(const TStr& Key, const TStr& Fun);
  void AssertObjKeyNum(const TStr& Key, const TStr& Fun);
  void AssertObjKeyBool(const TStr& Key, const TStr& Fun);

  // (de)serialization
  static PJsonVal GetValFromLx(TILx& Lx);
  static PJsonVal GetValFromSIn(const PSIn& SIn, bool& Ok, TStr& MsgStr);
  static PJsonVal GetValFromSIn(const PSIn& SIn);
  static PJsonVal GetValFromStr(const TStr& JsonStr, bool& Ok, TStr& MsgStr);
  static PJsonVal GetValFromStr(const TStr& JsonStr);
  static void AddEscapeChAFromStr(const TStr& Str, TChA& ChA);
  static TStr AddEscapeStrFromStr(const TStr& Str) { 
	  TChA ChA; AddEscapeChAFromStr(Str, ChA); return ChA; }
  static void AddQChAFromStr(const TStr& Str, TChA& ChA);
  static void GetChAFromVal(const PJsonVal& Val, TChA& ChA);
  static TStr GetStrFromVal(const PJsonVal& Val);
  
  // parsing json object to milliseconds from following supported formats:
  //  - 123  => milliseconds to milliseconds
  //  - {"value":12,"unit":"hour"} => parse out value and unit (second, minute, hour, day)
  //  - {"value":60} => assumes default unit second
  static uint64 GetMSecsFromJsonVal(const PJsonVal& Val);  
};

//////////////////////////////////////////////////////////////////////////////
// Binary serialization of Json Value
class TBsonObj {
public:
	static void Serialize(const TJsonVal& JsonVal, TSOut& SOut) { 
        CreateBsonRecursive(JsonVal, NULL, SOut); };
	static void SerializeVoc(const TJsonVal& JsonVal, TStrHash<TInt, TBigStrPool>& Voc, TSOut& SOut) { 
        CreateBsonRecursive(JsonVal, &Voc, SOut); };
	static int64 GetMemUsed(const TJsonVal& JsonVal) { 
        return GetMemUsedRecursive(JsonVal, false); };
	static int64 GetMemUsedVoc(const TJsonVal& JsonVal) { 
        return GetMemUsedRecursive(JsonVal, true); };

	static PJsonVal GetJson(TSIn& SIn) { 
        return GetJsonRecursive(SIn, NULL); };
	static PJsonVal GetJsonVoc(TSIn& SIn, TStrHash<TInt, TBigStrPool>& Voc) { 
        return GetJsonRecursive(SIn, &Voc); };

	static void UnitTest() { }

private:
	static void CreateBsonRecursive(const TJsonVal& JsonVal, 
        TStrHash<TInt, TBigStrPool> *Voc, TSOut& SOut);
	static int64 GetMemUsedRecursive(const TJsonVal& JsonVal, bool UseVoc);
	static PJsonVal GetJsonRecursive(TSIn& SIn, TStrHash<TInt, TBigStrPool>* Voc);
};

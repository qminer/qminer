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
  void PutBool(const bool& _Bool){JsonValType=jvtBool; Bool=_Bool;}
  void PutNum(const double& _Num){JsonValType=jvtNum; Num=_Num;}
  void PutStr(const TStr& _Str){JsonValType=jvtStr; Str=_Str;}
  void PutArr(){JsonValType=jvtArr;}
  void AddToArr(const PJsonVal& Val){
    EAssert(JsonValType==jvtArr); ValV.Add(Val);}
  void PutObj(){JsonValType=jvtObj;}
  void AddToObj(const TStr& KeyNm, const PJsonVal& Val){
    EAssert(JsonValType==jvtObj); 
	EAssert(KeyNm != "");
	KeyValH.AddDat(KeyNm, Val);}
  void AddToObj(const TStr& KeyNm, const int& Val){ AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const uint& Val){ AddToObj(KeyNm, NewNum((double)Val)); }
  void AddToObj(const TStr& KeyNm, const double& Val){ AddToObj(KeyNm, NewNum(Val)); }
  void AddToObj(const TStr& KeyNm, const double& Val1, const double& Val2){ AddToObj(KeyNm, NewArr(Val1, Val2)); }
  void AddToObj(const TStr& KeyNm, const TStr& Val){ AddToObj(KeyNm, NewStr(Val)); }
  void AddToObj(const TStr& KeyNm, const char* Val){ AddToObj(KeyNm, NewStr(Val)); }
  void AddToObj(const TStr& KeyNm, const bool& Val){ AddToObj(KeyNm, NewBool(Val)); }
  void AddToObj(const TStr& KeyNm, const TJsonValV& ValV){ AddToObj(KeyNm, NewArr(ValV)); }
  void AddToObj(const PJsonVal& Val);

  // simplified creation of basic elements
  static PJsonVal NewNull() { PJsonVal Val = TJsonVal::New(); Val->PutNull(); return Val; }
  static PJsonVal NewBool(const bool& Bool) { PJsonVal Val = TJsonVal::New(); Val->PutBool(Bool); return Val; }
  static PJsonVal NewNum(const double& Num) { PJsonVal Val = TJsonVal::New(); Val->PutNum(Num); return Val; }
  static PJsonVal NewStr(const TStr& Str) { PJsonVal Val = TJsonVal::New(); Val->PutStr(Str); return Val; }
  static PJsonVal NewArr() { PJsonVal Val = TJsonVal::New(); Val->PutArr(); return Val; }
  static PJsonVal NewArr(const TJsonValV& ValV);
  static PJsonVal NewArr(const TIntV& IntV);
  static PJsonVal NewArr(const TFltV& FltV);
  static PJsonVal NewArr(const double& Val1, const double& Val2);
  static PJsonVal NewArr(const TStrV& StrV);
  static PJsonVal NewArr(const TFltPr& FltPr);
  static PJsonVal NewObj() { PJsonVal Val = TJsonVal::New(); Val->PutObj(); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const PJsonVal& ObjVal) {
	  PJsonVal Val = TJsonVal::New(); Val->PutObj(); Val->AddToObj(KeyNm, ObjVal); return Val; }
  static PJsonVal NewObj(const TStr& KeyNm, const int& ObjVal) {
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

  // getting value
  bool GetBool() const {EAssert(IsBool()); return Bool;}
  double GetNum() const {EAssert(IsNum()); return Num;}
  int GetInt() const {EAssert(IsNum()); return TFlt::Round(Num);}
  TStr GetStr() const {EAssert(IsStr()); return Str;}
  int GetArrVals() const {EAssert(IsArr()); return ValV.Len();}
  PJsonVal GetArrVal(const int& ValN) const {return ValV[ValN];}
  void GetArrNumV(TFltV& FltV) const;
  void GetArrIntV(TIntV& IntV) const;
  void GetArrStrV(TStrV& StrV) const;
  int GetObjKeys() const {EAssert(IsObj()); return KeyValH.Len();}
  void GetObjKeyVal(const int& KeyValN, TStr& Key, PJsonVal& Val) const {
    EAssert(IsObj()); Key=KeyValH.GetKey(KeyValN); Val=KeyValH[KeyValN];}
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
  TStr GetObjStr(const TStr& Key) const { return GetObjKey(Key)->GetStr(); }
  TStr GetObjStr(const char *Key) const { return GetObjKey(Key)->GetStr(); }
  bool GetObjBool(const TStr& Key, const bool& DefBool) const;
  bool GetObjBool(const char *Key, const bool& DefBool) const;
  double GetObjNum(const TStr& Key, const double& DefNum) const;
  double GetObjNum(const char *Key, const double& DefNum) const;
  int GetObjInt(const TStr& Key, const int& DefNum) const;
  int GetObjInt(const char *Key, const int& DefNum) const;
  TStr GetObjStr(const TStr& Key, const TStr& DefStr) const;
  TStr GetObjStr(const char *Key, const TStr& DefStr) const;
  void GetObjStrV(const TStr& Key, TStrV& StrV) const;
  void GetObjStrV(const char *Key, TStrV& StrV) const;

  // removing value
  void DelObjKey(const TStr& Key) {EAssert(IsObj()); KeyValH.DelIfKey(Key); }
  void DelObjKey(const char *Key) {EAssert(IsObj()); KeyValH.DelIfKey(Key); }
  void DelArrVal(const int& ValN) { EAssert(IsArr()); ValV.Del(ValN); }

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
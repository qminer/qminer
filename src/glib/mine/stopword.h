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
// Stop-Word-Set

// reference
// The stoplist is the SMART system's list of 524 common words, like "the" and "of".)

typedef enum {swstNone, swstEn, swstEn8, swstEn425, swstEn523, swstGe, swstEs, swstSi, swstUndef } TSwSetType;

ClassTP(TSwSet, PSwSet)//{
private:
  TInt SwSetType;
  TStrH SwStrH;
  TInt MnWordLen;
  void Add(const TStr& WordStr);
  void MultiAdd(const TStr& Str);
  void AddEn425();
  void AddEn523();
  void AddEs();
  void AddGe();
  void AddSi();
  UndefCopyAssign(TSwSet);
public:
  TSwSet(const TSwSetType& _SwSetType=swstNone, const int& _MnWordLen=0);
  static PSwSet New(const TSwSetType& SwSetType=swstNone){
    return PSwSet(new TSwSet(SwSetType));}
  TSwSet(TSIn& SIn): SwSetType(SIn), SwStrH(SIn){}
  static PSwSet Load(TSIn& SIn){return new TSwSet(SIn);}
  void Save(TSOut& SOut) const {SwSetType.Save(SOut); SwStrH.Save(SOut);}

  // component retrieval
  TSwSetType GetSwSetType() const {
    return TSwSetType(int(SwSetType));}
  bool IsIn(const TStr& WordStr, const bool& UcWordStrP=true) const;
  bool IsEmpty(){ return (SwSetType == swstNone) || (SwStrH.Empty()); }
  void AddWord(const TStr& WordStr);
  void LoadFromFile(const TStr& FNm);

  // traverse words
  int FFirstSwId() const {return SwStrH.FFirstKeyId();}
  bool FNextSwId(int& SwId, TStr& SwStr) const {
    if (SwStrH.FNextKeyId(SwId)){
      SwStr=SwStrH.GetKey(SwId); return true;
    } else {return false;}}

  // stop-word-set creators
  static void GetSwSetTypeNmV(TStrV& SwSetTypeNmV, TStrV& SwSetTypeDNmV);
  static TStr GetSwSetTypeNmVStr();
  static TSwSetType GetSwSetType(const TStr& SwSetTypeNm);
  static PSwSet GetSwSet(const TSwSetType& SwSetType);
  static PSwSet GetSwSet(const bool& StopWordP, const TSwSetType& SwSetType){
    if (StopWordP){return GetSwSet(SwSetType);} else {return NULL;}}
  static PSwSet GetSwSet(const TStr& SwSetTypeNm){
    return GetSwSet(true, GetSwSetType(SwSetTypeNm));}
  
    /// Extract stop-words from JSon parameters
  static PSwSet ParseJson(const PJsonVal& JsonVal);
};


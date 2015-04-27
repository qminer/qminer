/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
  static THash<TStr, TStrV> StopWordFiles;
  void Add(const TStr& WordStr);
  void MultiAdd(const TStr& Str);
  void AddEn425();
  void AddEn523();
  void AddEn8();
  void AddEs();
  void AddGe();
  void AddSi();
  UndefCopyAssign(TSwSet);


public:
  // New New() API
  TSwSet(const TStr& SwListNm, const int& _MnWordLen=0); // NEW API
  TSwSet(const TStrV& SwListNmV, const int& _MnWordLen=0);
  static PSwSet New(const TStr& SwListNm){return PSwSet(new TSwSet(SwListNm));}
  void AddByName(const TStr& _SwListNm);
  static bool IsValidList(const TStr SwListNm);
  // Old New() API
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
  // New Load File API
  /// loads a snowball stopword file into the TSwSet Hasmap (StopWordFiles)
  static void LoadSwFile(const TStr& FNm);
  /// loads all snowball stopword files in a directory using LoadSwFile()
  static void LoadSwDir(const TStr& DNm);

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


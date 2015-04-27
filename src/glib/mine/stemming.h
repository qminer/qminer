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
// Stemmer
typedef enum {stmtNone, stmtPorter} TStemmerType;

ClassTP(TStemmer, PStemmer)//{
private:
  TInt StemmerType; // TStemmerType
  TBool RealWordP;
  TStrStrH StemStrToRealWordStrH;
  TStrStrH SynonymStrToWordStr;
public:
  TStemmer(
   const TStemmerType& _StemmerType=stmtNone, const bool& _RealWordP=false):
    StemmerType(_StemmerType), RealWordP(_RealWordP),
    StemStrToRealWordStrH(), SynonymStrToWordStr(){}
  static PStemmer New(
   const TStemmerType& StemmerType=stmtNone, const bool& RealWordP=false){
    return PStemmer(new TStemmer(StemmerType, RealWordP));}
  TStemmer(TSIn& SIn):
    StemmerType(SIn), RealWordP(SIn),
    StemStrToRealWordStrH(SIn), SynonymStrToWordStr(SIn){}
  static PStemmer Load(TSIn& SIn){return new TStemmer(SIn);}
  void Save(TSOut& SOut) const {
    StemmerType.Save(SOut); RealWordP.Save(SOut);
    StemStrToRealWordStrH.Save(SOut); SynonymStrToWordStr.Save(SOut);}

  TStemmer& operator=(const TStemmer& Stemmer){
    StemmerType=Stemmer.StemmerType;
    RealWordP=Stemmer.RealWordP;
    StemStrToRealWordStrH=Stemmer.StemStrToRealWordStrH;
    SynonymStrToWordStr=Stemmer.SynonymStrToWordStr;
    return *this;}

  void AddSynonym(const TStr& SynonymStr, const TStr& WordStr){
    SynonymStrToWordStr.AddDat(SynonymStr.GetUc(), WordStr.GetUc());}

  TStr GetStem(const TStr& WordStr);

  TStemmerType GetStemmerType(){
    return (TStemmerType)(int)StemmerType;}

  // stemmer creators
  static void GetStemmerTypeNmV(TStrV& StemmerTypeNmV, TStrV& StemmerTypeDNmV);
  static TStr GetStemmerTypeNmVStr();
  static TStemmerType GetStemmerType(const TStr& StemmerTypeNm);
  static PStemmer GetStemmer(const TStemmerType& StemmerType){
    return TStemmer::New(StemmerType, true);}
  static PStemmer GetStemmer(const bool& StemmerP, const TStemmerType& StemmerType){
    if (StemmerP){return GetStemmer(StemmerType);} else {return NULL;}}
  static PStemmer GetStemmer(const TStr& StemmerTypeNm){
    return GetStemmer(true, GetStemmerType(StemmerTypeNm));}
  
  static PStemmer ParseJson(const PJsonVal& StemmerVal, const bool& RealWordP);
};

/////////////////////////////////////////////////
// Porter-Stemmer
// http://www.tartarus.org/~martin/PorterStemmer/

class TPorterStemmer{
protected:
  static inline bool IsVowel(bool prevVowel, char ch) { return ch == 'E' ||
    ch == 'O' || ch == 'A' || ch == 'I' || ch == 'U' || (ch == 'Y' && ! prevVowel); }

  static inline int Measure(const char *pStart, const char *pEnd){
    if (pStart == pEnd) return 0;
    // At the beginning of a word, we pretend that the previous letter
    // was a vowel when we call IsVowel.  This makes sure that a Y at the
    // beginning of a word is recognized as a consonant.
    int m = 0; bool vowel = IsVowel(true, *pStart++);
    // Basically, our measure means we must count transitions from V to C.
    while (pStart < pEnd)
    {
      bool newVowel = IsVowel(vowel, *pStart++);
      if (vowel && ! newVowel) m++;
      vowel = newVowel;
    }
    return m;
  }

  // Tests if measure > 0.  Assumes that pStart and pEnd are not equal
  // (i.e., that we may access the first letter of the string).
  static inline bool MeasureG0(const char *pStart, const char *pEnd){
    if (pEnd <= pStart) return false; // the empty word has measure 0
    bool vowel = IsVowel(true, *pStart++);
    while (pStart < pEnd)
    {
      bool newVowel = IsVowel(vowel, *pStart++);
      if (vowel && ! newVowel) return true;
      vowel = newVowel;
    }
    return false;
  }

  static inline bool HasVowels(const char *pStart, const char *pEnd){
    bool vowel = true;
    while (pStart < pEnd)
      { vowel = IsVowel(vowel, *pStart++); if (vowel) return true; }
    return false;
  }

  static inline bool EndsWithDoubleConsonant(const char *pStart, const char *pEnd){
    if (pEnd - pStart < 2) return false;
    char ch = pEnd[-1];
    if (pEnd[-2] != ch) return false;
    // Note that 'YY' cannot possibly be a double consonant.  The kind
    // (consonant or vowel) of a Y is always the opposite of that of the
    // letter preceding it.  Thus, one of these two Ys must be a vowel and
    // the other a consonant.  Not that YY would ever appear in practice,
    // of course.  Even in Omar Khayyam it only appears in the middle of the word. :)
    return !(ch == 'E' || ch == 'A' || ch == 'O' || ch == 'I' || ch == 'U' || ch == 'Y');
  }

  static inline bool IsCvc(const char *pStart, const char *pEnd){
    if (pEnd - pStart < 3) return false;
    bool vowel = IsVowel(true, *pStart); if (vowel) return false;
    // At this point, pStart points at the first character.
    while (pStart < pEnd && ! vowel) vowel = IsVowel(vowel, *(++pStart));
    if (pStart != pEnd - 2) return false;
    // At this point, pStart points at the first vowel.
    vowel = IsVowel(vowel, *(++pStart));
    if (vowel) return false;
    // At this point, pStart points at the last character of the word.
    // This final consonant must not be W, X, or Y.
    char last = *pStart++;
    return !(last == 'W' || last == 'X' || last == 'Y');
  }

public:
  static char *StemInPlace(char *pWord); // assumes word is in uppercase
  static TStr Stem(const TStr& s); // will convert s into uppercase
  static TStr StemX(const TStr& s); // strips ' or 's, then calls Stem
};


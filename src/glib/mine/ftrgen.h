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

namespace TFtrGen {

///////////////////////////////////////
/// Numeric feature generator
class TNumeric {
private:
    typedef enum { ntNone, ntNormalize, ntMnMxVal } TNumericType;
    
private:
    /// Feature generator type
    TNumericType Type;
    /// Minimal value for normalization
    TFlt MnVal;
    /// Maximal value for normalization
    TFlt MxVal;   

public:
    TNumeric(const bool& NormalizeP = true):
		Type(NormalizeP ? ntNormalize : ntNone), MnVal(TFlt::Mx), MxVal(TFlt::Mn) { }
    TNumeric(const double& _MnVal, const double& _MxVal):
        Type(ntMnMxVal), MnVal(_MnVal), MxVal(_MxVal) { }
    TNumeric(TSIn& SIn): Type(LoadEnum<TNumericType>(SIn)), MnVal(SIn), MxVal(SIn) {  }
    void Save(TSOut& SOut) const;

    void Clr();
    bool Update(const double& Val);
    double GetFtr(const double& Val) const;
    void AddFtr(const double& Val, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const double& Val, TFltV& FullV, int& Offset) const;
    
    int GetDim() const { return 1; }
};

///////////////////////////////////////
/// Categorical feature generator
class TCategorical {
private:
    typedef enum { ctOpen, ctFixed, ctHash } TCategoricalType;
    
private:
    /// Feature generator type
    TCategoricalType Type;
    /// Feature value range    
    TStrSet ValSet;
    /// Hash dimensionality
    TInt HashDim;

public:
    TCategorical(): Type(ctOpen) { }
    TCategorical(const TStrV& ValV): Type(ctFixed), ValSet(ValV) { }
    TCategorical(const int& _HashDim): Type(ctHash), HashDim(_HashDim) { }
    TCategorical(TSIn& SIn): Type(LoadEnum<TCategoricalType>(SIn)), ValSet(SIn), HashDim(SIn) { }
    void Save(TSOut& SOut) const;

    void Clr();
    bool Update(const TStr& Val);
    int GetFtr(const TStr& Val) const;
    void AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStr& Val, TFltV& FullV, int& Offset) const;

    int GetDim() const { return (Type == ctHash) ? HashDim.Val : ValSet.Len(); }
    TStr GetVal(const int& ValN) const { return (Type == ctHash) ? "hash" : ValSet.GetKey(ValN); }
};

///////////////////////////////////////
// Multinomial feature generator
class TMultinomial {
private:
    typedef enum { mtNone, mtNormalize } TMultinomialType;
    
private:
    /// Feature generator type
    TMultinomialType Type;
    /// Feature generation handled by categorical feature generator
    TCategorical FtrGen;

    
public:
    TMultinomial(const bool& NormalizeP = true): FtrGen() { }
    TMultinomial(const bool& NormalizeP, const TStrV& ValV): FtrGen(ValV) { }
    TMultinomial(const bool& NormalizeP, const int& HashDim): FtrGen(HashDim) { }
    TMultinomial(TSIn& SIn): Type(LoadEnum<TMultinomialType>(SIn)), FtrGen(SIn) { }
    void Save(TSOut& SOut) const;

    void Clr() { FtrGen.Clr(); }
    bool Update(const TStr& Str);
    bool Update(const TStrV& StrV);
    void AddFtr(const TStr& Str, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStr& Str, TFltV& FullV, int& Offset) const;
    void AddFtr(const TStrV& StrV, TIntFltKdV& SpV) const;
    void AddFtr(const TStrV& StrV, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStrV& StrV, TFltV& FullV, int& Offset) const;

    int GetDim() const { return FtrGen.GetDim(); }
    TStr GetVal(const int& ValN) const { return FtrGen.GetVal(ValN); }
};

///////////////////////////////////////
/// Bag of words feature generator.
/// Can produce just word counts, or normalized with TFIDF weights.
class TBagOfWords {
private:
    /// Settings:
    ///  - how to weight each word: TF or TFIDF
    ///  - normalize resulting vector to L2-norm 1.0
    typedef enum {
        btTf = (1 << 0),
        btIdf = (1 << 1),
        btNormalize = (1 << 2),
        btHashing = (1 << 3)
    } TBagOfWordsType;
    
private:
    /// Feature generation settings
    TInt Type;
    /// Tokenizer
    PTokenizer Tokenizer;
    /// Stop word set
    PSwSet SwSet;
    /// Stemmer
    PStemmer Stemmer;
    /// Vocabulary (not use in case of hashing)
    TStrSet TokenSet;
    /// Hashing dimension
    TInt HashDim;
    /// Ngrams Range Start
    TInt NStart;
    /// Ngrams Range End
    TInt NEnd;
    
    /// Number of documents processed so far
    TInt Docs;
    /// Document frequency for each token
    TIntV DocFqV;
    
    /// True after first Forget call
    TBool ForgetP;
    /// Number of documents before last forget
    TFlt OldDocs;
    /// Document frequency counts before last forget
    TFltV OldDocFqV;

public:
    TBagOfWords() { }
    TBagOfWords(const bool& TfP, const bool& IdfP, const bool& NormalizeP,
        PTokenizer _Tokenizer = NULL, const int& _HashDim = -1, const int& NStart=1, const int& NEnd=1);
    TBagOfWords(TSIn& SIn);
    void Save(TSOut& SOut) const;

    // Settings getters
    bool IsTf() const { return ((Type & btTf) != 0); }
    bool IsIdf() const { return ((Type & btIdf) != 0); }
    bool IsNormalize() const { return ((Type & btNormalize) != 0); }
    bool IsHashing() const { return ((Type & btHashing) != 0); }    
    
    void Clr();
    void GetFtr(const TStr& Str, TStrV& TokenStrV) const;
    bool Update(const TStrV& TokenStrV);
    bool Update(const TStr& Val);
    void AddFtr(const TStrV& TokenStrV, TIntFltKdV& SpV) const;
    void AddFtr(const TStr& Val, TIntFltKdV& SpV) const;
    void AddFtr(const TStrV& TokenStrV, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStrV& TokenStrV, TFltV& FullV, int& Offset) const;
    void AddFtr(const TStr& Val, TFltV& FullV, int& Offset) const;
    
    /// Forgetting, assumes calling on equally spaced time interval.
    void Forget(const double& Factor);

    int GetDim() const { return IsHashing() ? HashDim.Val : TokenSet.Len(); }
    TStr GetVal(const int& ValN) const { if(IsHashing()) {return TInt::GetStr(ValN);
    	} else {
    		return TokenSet.GetKey(ValN);
    	}
    }
    
    PSwSet GetSwSet() const { return SwSet; }
    PStemmer GetStemmer() const { return Stemmer; }

    /// Generate Ngrams
    void GenerateNgrams(const TStrV& TokenStrV, TStrV& NgramStrV) const;
}; 

///////////////////////////////////////
// Sparse-Feature-Generator
class TSparseNumeric {
private:
    TInt MxId;
    TNumeric FtrGen;

public:
    TSparseNumeric() { }
    TSparseNumeric(TSIn& SIn): MxId(SIn), FtrGen(SIn) { }
    void Save(TSOut& SOut) const { MxId.Save(SOut); FtrGen.Save(SOut); }

    void Update(const TIntFltKdV& SpV);
    void AddFtr(const TIntFltKdV& InSpV, TIntFltKdV& SpV, int& Offset) const;

    int GetVals() const { return MxId + 1; }
};

}

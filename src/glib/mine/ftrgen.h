/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
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
    
    double InvFtr(const TFltV& FullV, int& Offset) const;

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
	TMultinomial(const bool& NormalizeP = true) : Type(NormalizeP ? mtNormalize : mtNone), FtrGen() { }
	TMultinomial(const bool& NormalizeP, const TStrV& ValV) : Type(NormalizeP ? mtNormalize : mtNone), FtrGen(ValV) { }
	TMultinomial(const bool& NormalizeP, const int& HashDim) : Type(NormalizeP ? mtNormalize : mtNone), FtrGen(HashDim) { }
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
        btHashing = (1 << 3),
        btStoreHashWords = (1 << 4)
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
    /// Vocabulary (not used when hashing)
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

    /// Set of tokens that hash into specific dimension
    TVec<TStrSet> HashWordV;
    /// default return in case sets are empty
    TStrSet EmptySet;

public:
    TBagOfWords() { }
    TBagOfWords(const bool& TfP, const bool& IdfP, const bool& NormalizeP,
        PTokenizer _Tokenizer = NULL, const int& _HashDim = -1,
        const bool& StoreHashWordsP = false, const int& NStart = 1, const int& NEnd = 1);
    TBagOfWords(TSIn& SIn);
    void Save(TSOut& SOut) const;

    // Settings getters
    bool IsTf() const { return ((Type & btTf) != 0); }
    bool IsIdf() const { return ((Type & btIdf) != 0); }
    bool IsNormalize() const { return ((Type & btNormalize) != 0); }
    bool IsHashing() const { return ((Type & btHashing) != 0); }
    bool IsStoreHashWords() const { return ((Type & btStoreHashWords) != 0); }
    
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

    /// Hashing Related Functions
    int GetDim() const { return IsHashing() ? HashDim.Val : TokenSet.Len(); }
    TStr GetVal(const int& ValN) const { return IsHashing() ? TInt::GetStr(ValN) : TokenSet.GetKey(ValN); }
    const TVec<TStrSet>& GetHashWordH() const { return HashWordV; }
    TStrSet GetHashVals(const int& Hash) const { return IsStoreHashWords() ? HashWordV.GetVal(Hash) : EmptySet; }
    
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

///////////////////////////////////////
/// Date window feature generator
class TDateWnd {
private:
    TBool InitP;
    /// Start date
    TUInt StartUnit;
    /// End date
    TUInt EndUnit;
    /// Window size
    TInt WndSize;
    /// Time unit
    TTmUnit TmUnit;
    /// Normalize output to 1
    TBool NormalizeP;
    /// Weight used in the feature vectors
    TFlt Wgt;
    
    // initialize the weight based on parameters
    void InitWgt();

public:
    TDateWnd(): InitP(false), TmUnit(tmuUndef) { }
    TDateWnd(const int& _WndSize, const TTmUnit& _TmUnit, const bool& _NormalizeP = true);
    TDateWnd(const TTm& StartTm, const TTm& EndTm, const int& _WndSize, 
        const TTmUnit& _TmUnit, const bool& _NormalizeP = true);
    TDateWnd(TSIn& SIn): InitP(SIn), StartUnit(SIn), EndUnit(SIn), WndSize(SIn),
        TmUnit(LoadEnum<TTmUnit>(SIn)), NormalizeP(SIn) { InitWgt(); }
    void Save(TSOut& SOut) const;

    void Clr() { InitP = false; }
    bool Update(const TTm& Val);
    int GetFtr(const TTm& Val) const;
    void AddFtr(const TTm& Val, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TTm& Val, TFltV& FullV, int& Offset) const;
    
    int GetDim() const { return EndUnit - StartUnit + WndSize; }
};

}

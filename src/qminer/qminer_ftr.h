/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef QMINER_FTR_H
#define QMINER_FTR_H

#include "qminer_core.h"

namespace TQm {

///////////////////////////////
/// Feature extractor.
class TFtrExt;
typedef TPt<TFtrExt> PFtrExt;
typedef TVec<PFtrExt> TFtrExtV;

class TFtrExt {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TFtrExt>;
    
	/// New constructor delegate
	typedef PFtrExt (*TNewF)(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    /// Feature extractor New constructor router
	static TFunRouter<PFtrExt, TNewF> NewRouter;
    /// Load constructor delegate
	typedef PFtrExt (*TLoadF)(const TWPt<TBase>& Base, TSIn& SIn);   
    /// Feature extractor Load constructor router
	static TFunRouter<PFtrExt, TLoadF> LoadRouter;
public:
    /// Register default feature extractors
    static void Init();
    /// Register new feature extractor
    template <class TObj> static void Register() {
        NewRouter.Register(TObj::GetType(), TObj::New);
        LoadRouter.Register(TObj::GetType(), TObj::Load);
    }
private:
    /// QMiner Base pointer
    TWPt<TBase> Base;
    
    /// Map from record store to join which brings it to feature extraction store
	THash<TUInt, TJoinSeq> JoinSeqH;
	/// The end store from which features are actually extracted 
	TWPt<TStore> FtrStore;

protected:
    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }

	/// Checks the record store and uses appropriate join sequence 
    /// to derive a single join record from feature store
	TRec DoSingleJoin(const TRec& Rec) const;

    /// Feature extractor with multiple start stores
	TFtrExt(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV);
    /// Feature extractor parsed from JSon parameters
	TFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	/// Load basic class of feature extractor   
	TFtrExt(const TWPt<TBase>& _Base, TSIn& SIn);
    
public:
    /// Create new feature extractor from JSon parameters
	static PFtrExt New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    /// Virtual destructor!
	virtual ~TFtrExt() { }

	/// Load feature extractor from stream
	static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
	/// Save basic class of feature extractor to stream
	virtual void Save(TSOut& SOut) const;   
   
	/// Name of the feature (e.g. field name, store name)
	virtual TStr GetNm() const { return "[undefined]"; };
	/// Dimensionality of the feature space
	virtual int GetDim() const = 0;
	// string representation of the FtrN-th feature
	virtual TStr GetFtr(const int& FtrN) const = 0;
   	/// Reset feature extractor to forget all previously seen records
	virtual void Clr() = 0;
    
	/// Update the feature extractor using the info from the given record.
    /// Returns true if the update changes the dimensionality.
	virtual bool Update(const TRec& Rec) = 0;
	/// Attaches features to a given sparse feature vectors with a given offset
	virtual void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const = 0;
	/// Attaches features to a given full feature vectors with a given offset
	virtual void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	/// Inverts features from the given feature vector. The features must start
	/// at the given offset. Increases the offset by its dimension
	virtual void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const = 0;

	// for more strait-forward feature extraction (i.e. used by basic aggregators)
	// attaches values to the given vector, keeps what is in there already
	virtual void ExtractStrV(const TRec& Rec, TStrV& StrV) const;
	virtual void ExtractFltV(const TRec& Rec, TFltV& FltV) const;
	virtual void ExtractTmV(const TRec& Rec, TTmV& TmV) const;

	/// Check if the given store is one of the allowed start stores
	bool IsStartStore(const uint& StoreId) const { return JoinSeqH.IsKey(StoreId); }
	/// Is there a join to be done when starting from the given store
	bool IsJoin(const uint& StoreId) const { return JoinSeqH.GetDat(StoreId).IsJoin(); }
    /// Get the join sequence required to be executed for the given store
	const TJoinSeq& GetJoinSeq(const uint& StoreId) const { return JoinSeqH.GetDat(StoreId); }
    /// Get the join IDs required to be executed for the given store
	const TIntPrV& GetJoinIdV(const uint& StoreId) const { return JoinSeqH.GetDat(StoreId).GetJoinIdV(); }
	/// Store from which the features come
	TWPt<TStore> GetFtrStore() const { return FtrStore; }
};

///////////////////////////////////////////////
/// Feature space.
class TFtrSpace {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TFtrSpace>;
    /// QMiner Base pointer
    TWPt<TBase> Base;
    
    /// Current dimensionality of feature space
	TInt Dim;
    /// Partial dimensions. N-th element contains dimensionality 
    /// of the first N feature extractors summed together
	TIntV DimV;
    /// List of feature extractors with variable dimensionality
    TIntV VarDimFtrExtNV;
    /// Feature extractors composing the feature space
	TFtrExtV FtrExtV;
    
    void Init();

	TFtrSpace(const TWPt<TBase>& _Base, const PFtrExt& FtrExt);
	TFtrSpace(const TWPt<TBase>& _Base, const TFtrExtV& _FtrExtV);
    TFtrSpace(const TWPt<TBase>& _Base, TSIn& SIn);
public:
    /// Create feature space with one feature extractor
	static TPt<TFtrSpace> New(const TWPt<TBase>& Base, const PFtrExt& FtrExt); 
    /// Create feature space with multiple feature extractors
    static TPt<TFtrSpace> New(const TWPt<TBase>& Base, const TFtrExtV& FtrExtV);
    /// Load existing feature space from stream
    static TPt<TFtrSpace> Load(const TWPt<TBase>& Base, TSIn& SIn);
    /// Save feature space to stream
	void Save(TSOut& SOut) const;       
	/// Add a feature extractore
	void AddFtrExt(const PFtrExt& FtrExt);

	/// Generate a name of feature space. Composed by concatenating feature extractor names.
	TStr GetNm() const;

	/// Clear existing future space. Calls all internal feature extractors
	void Clr();
	/// Update feature extractors given a record
	bool Update(const TRec& Rec);
	/// Update feature extractors given a set of records
	bool Update(const PRecSet& RecSet);
    /// Extract sparse feature vector from a record
	void GetSpV(const TRec& Rec, TIntFltKdV& SpV) const;
    /// Extract full feature vector from a record
    void GetFullV(const TRec& Rec, TFltV& FullV) const;
	/// Extracting sparse feature vectors from a record set
	void GetSpVV(const PRecSet& RecSet, TVec<TIntFltKdV>& SpVV) const;
	/// Extracting full feature vectors from a record set
	void GetFullVV(const PRecSet& RecSet, TVec<TFltV>& FullVV) const;
	/// Extracting full feature vectors (columns) from a record set
	void GetFullVV(const PRecSet& RecSet, TFltVV& FullVV) const;
	/// Compute sparse centroid of a given record set
	void GetCentroidSpV(const PRecSet& RecSet, TIntFltKdV& CentroidSpV, const bool& NormalizeP = true) const;
	/// Compute full centroid of a given record set
	void GetCentroidV(const PRecSet& RecSet, TFltV& CentroidV, const bool& NormalizeP = true) const;
	/// Returns the inverse operation on the feature vector
	void InvertFullV(const TFltV& FullV, TFltV& InvertV) const;
	/// returns the inverse operation on a single feature
	double InvertFtr(const int& FtrExtN, const TFlt& FtrVal) const;
    
    /// String vector for a record transformed by a feature extractor
    void ExtractStrV(const int& DimN, const PJsonVal& RecVal, TStrV &StrV) const;

	/// Dimensionality of the feature space
	int GetDim() const;
	/// String representation of the FtrN-th feature
	TStr GetFtr(const int& FtrN) const;
    /// Number of feature extractors
    int GetFtrExts() const;
    /// Get feature extractor
    PFtrExt GetFtrExt(const int& FtrExtN) const;
    /// Dimensionality of space formed by FtrExtN-th feature extractor
    int GetFtrExtDim(const int& FtrExtN) const;
    /// Start dimension for the FtrExtN-th feature extractor
    int GetMnFtrN(const int& FtrExtN) const;
    /// End dimension +1 for the FtrExtN-th feature extractor
    int GetMxFtrN(const int& FtrExtN) const;

	/// Prepares an empty bow and registers all the features
	PBowDocBs MakeBowDocBs(const PRecSet& FtrRecSet);

	const TWPt<TBase> GetBase() const { return Base; }
};
typedef TPt<TFtrSpace> PFtrSpace;

///////////////////////////////////////////////
/// Implemented feature extractors.
namespace TFtrExts {

///////////////////////////////////////////////
/// Constant feature extractor.
class TConstant : public TFtrExt {
private:
	// constant feature generator
	mutable TFlt Constant;

	TConstant(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
	TConstant(const TWPt<TBase>& Base, TSIn& SIn);

public:
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;

	TStr GetNm() const { return "Constant"; };
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return "Constant"; }

	void Clr() { }; 
    bool Update(const TRec& Rec) { return false; }
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractFltV(const TRec& FtrRec, TFltV& FltV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "constant"; } 
};

///////////////////////////////////////////////
/// Random feature extractor.
class TRandom : public TFtrExt {
private:
	// random feature generator
	mutable TRnd Rnd;

	TRandom(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed);
	TRandom(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TRandom(const TWPt<TBase>& Base, TSIn& SIn);

public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;

	TStr GetNm() const { return "Random"; };
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return "Random"; }

	void Clr() { };
    bool Update(const TRec& Rec) { return false; }
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractFltV(const TRec& FtrRec, TFltV& FltV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "random"; }   
};

///////////////////////////////////////////////
/// Numeric Feature Extractor
class TNumeric : public TFtrExt {
private:
    /// Feature generator
    TFtrGen::TNumeric FtrGen;
	/// Field Id
	TInt FieldId;
    /// Field description
    TFieldDesc FieldDesc;

    /// Get value from a given record
	double _GetVal(const TRec& Rec) const; 
    /// Check if there is join, and forward to _GetVal
	double GetVal(const TRec& Rec) const; 

	TNumeric(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& _FieldId, const bool& _NormalizeP);
    TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TNumeric(const TWPt<TBase>& Base, TSIn& SIn);    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;   
    
	TStr GetNm() const { return "Numeric[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return GetNm(); }

	void Clr() { FtrGen.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractFltV(const TRec& Rec, TFltV& FltV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "numeric"; }   
};

///////////////////////////////////////////////
/// Categorical Feature Extractor.
/// Categorical distribution (also called a "generalized Bernoulli distribution")
/// is a probability distribution that describes the result of a random event that 
/// can take on one of K possible outcomes.
/// [http://en.wikipedia.org/wiki/Categorical_distribution]
class TCategorical : public TFtrExt {
private:
	// nominal feature generator
	TFtrGen::TCategorical FtrGen;
	/// Field Id
	TInt FieldId;
    /// Field description
    TFieldDesc FieldDesc;

	TStr _GetVal(const TRec& FtrRec) const; 
	TStr GetVal(const TRec& Rec) const; 

	TCategorical(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId);
    TCategorical(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TCategorical(const TWPt<TBase>& Base, TSIn& SIn);    
    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;   
    
	TStr GetNm() const { return "Categorical[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return FtrGen.GetDim(); }
	TStr GetFtr(const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void Clr() { FtrGen.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "categorical"; }      
};

///////////////////////////////////////////////
/// Multinomial Feature Extractor.
/// For n independent trials each of which leads to a success for exactly one of 
/// k categories, with each category having a given fixed success probability, 
/// the multinomial distribution gives the probability of any particular combination 
/// of numbers of successes for the various categories.
/// [http://en.wikipedia.org/wiki/Multinomial_distribution]
class TMultinomial : public TFtrExt {
private:
	// multinomial feature generator
	TFtrGen::TMultinomial FtrGen;
	// field Id
	TIntV FieldIdV;
    // field description
    TFieldDescV FieldDescV;
    
	void ParseDate(const TTm& Tm, TStrV& StrV) const;
	void _GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const; 
	void _GetVal(const TRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const TRec& Rec, TStrV& StrV) const; 
        
    void AddField(const int& FieldId);
    void AddField(const TStr& FieldNm);
    
	TMultinomial(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId);
    TMultinomial(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TMultinomial(const TWPt<TBase>& Base, TSIn& SIn);
    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;   
    
	TStr GetNm() const;
	int GetDim() const { return FtrGen.GetDim(); }
	TStr GetFtr(const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void Clr() { FtrGen.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;
	void ExtractTmV(const TRec& Rec, TTmV& TmV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "multinomial"; }   
};

///////////////////////////////////////////////
// Bag-of-words Feature Extractor.
typedef enum { bowmConcat, bowmCentroid, bowmTokenized } TBagOfWordsMode;

class TBagOfWords : public TFtrExt, protected TTmWnd::TCallback {
private:
	/// Bag of words feature generator
	TFtrGen::TBagOfWords FtrGen;
    
	/// Field Id
	TIntV FieldIdV;
    /// Field description
    TFieldDescV FieldDescV;

 	/// How to deal with multiple instances
	TBagOfWordsMode Mode;
    
    /// Time field id, when provided
    TInt TimeFieldId;
    /// Time window calling Forget on feature generator
    TTmWnd TmWnd;
    /// Forgetting factor
    TFlt ForgetFactor;            

    /// n-grams Start
    int NStart;
    /// n-grams End
    int NEnd;

	void _GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const; 
	void _GetVal(const TRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const TRec& Rec, TStrV& StrV) const; 
    
    void AddField(const int& FieldId);
    void AddField(const TStr& FieldNm);

protected:
    // time window callback
    void NewTimeWnd(const uint64& TimeWndMSecs, const uint64& StartMSecs);

private:
    TBagOfWords(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& _FieldId, const TBagOfWordsMode& _Mode, 
        const PTokenizer& Tokenizer, const int& HashDim = -1, 
        const int& NStart=1, const int& NEnd=1);
    TBagOfWords(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TBagOfWords(const TWPt<TBase>& Base, TSIn& SIn);
    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const TBagOfWordsMode& Mode = bowmConcat, 
        const PTokenizer& Tokenizer = TTokenizers::THtmlUnicode::New(
        TSwSet::New(swstEn523), TStemmer::New(stmtPorter, false)));
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const TBagOfWordsMode& Mode = bowmConcat, 
        const PTokenizer& Tokenizer = TTokenizers::THtmlUnicode::New(
        TSwSet::New(swstEn523), TStemmer::New(stmtPorter, false)));
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const TBagOfWordsMode& Mode = bowmConcat, 
        const PTokenizer& Tokenizer = TTokenizers::THtmlUnicode::New(
        TSwSet::New(swstEn523), TStemmer::New(stmtPorter, false)));
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    
    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;
    
	TStr GetNm() const;
	int GetDim() const { return FtrGen.GetDim(); }
	TStr GetFtr(const int& FtrN) const;

	void Clr() { FtrGen.Clr(); }

	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;

    // feature extractor type name 
    static TStr GetType() { return "text"; }   
    
    static PStemmer ParseStemmer(const PJsonVal& ParamVal, const bool& RealWordP = false);
};

///////////////////////////////////////////////
/// Join Feature Extractor.
/// The feature extractor only works for stores with less then 2B records and
/// assumes dimensionality is the max RecId + 1.
class TJoin : public TFtrExt {
private:
    /// Number of records consecutive by ID that are bucketed together.
    /// The default is 1, which equals to no bucketing. Bucketing increases speed.
	TInt BucketSize;
    /// Dimensionality of feature extractor (equals to max RecId + 1)
	TInt Dim;

	// initializes dimensionality of the feature extractor
	void Def();

	TJoin(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _BucketSize);
    TJoin(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TJoin(const TWPt<TBase>& Base, TSIn& SIn);
    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& BucketSize = 1);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& BucketSize = 1);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;
    
	TStr GetNm() const { return "Join[" + GetFtrStore()->GetStoreNm() + "]"; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return GetFtrStore()->GetStoreNm(); }
		
	void Clr() { Def(); }
	bool Update(const TRec& Rec) { return false; }
	// sparse vector extraction
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	//void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;

    // feature extractor type name 
    static TStr GetType() { return "join"; }   
};

///////////////////////////////////////////////
/// Pair Feature Extractor. 
/// Combines two feature extractors, which implement ExtractStrV.
class TPair : public TFtrExt {
private:
	// feature extractor pair
	PFtrExt FtrExt1;
	PFtrExt FtrExt2;
	// feature extractor's vocabulary
	TStrHash<TInt> FtrValH;
	// pair features
	TIntPrIntH FtrIdPairH;

	// get vector of feature ids
	void GetFtrIdV_Update(const TRec& FtrRec, const PFtrExt& FtrExt, TIntV& FtrIdV);
	void GetFtrIdV_RdOnly(const TRec& FtrRec, const PFtrExt& FtrExt, TIntV& FtrIdV) const;

	TPair(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const PFtrExt& _FtrExt1, const PFtrExt& _FtrExt2);
    TPair(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TPair(const TWPt<TBase>& Base, TSIn& SIn);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV,
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;    
    
	TStr GetNm() const { return "Pair[" + FtrExt1->GetNm() + "," + FtrExt2->GetNm() + "]"; };
	int GetDim() const { return FtrIdPairH.Len(); }
	TStr GetFtr(const int& FtrN) const;

	void Clr() { FtrValH = TStrHash<TInt>(); FtrIdPairH.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& FtrRec);
	void AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;
	//void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

	// flat feature extraction
	void ExtractStrV(const TRec& FtrRec, TStrV& StrV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "pair"; }       
};

///////////////////////////////////////////////
/// Date Window Feature Extractor
class TDateWnd : public TFtrExt {
private:
    /// Feature generator
    TFtrGen::TDateWnd FtrGen;
	/// Field Id
	TInt FieldId;
    /// Field description
    TFieldDesc FieldDesc;

    /// Get value from a given record
	uint64 _GetVal(const TRec& Rec) const; 
    /// Check if there is join, and forward to _GetVal
	uint64 GetVal(const TRec& Rec) const; 

	TDateWnd(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& _FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP);
    TDateWnd(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
    TDateWnd(const TWPt<TBase>& Base, TSIn& SIn);    
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const int& WndSize, const TTmUnit& TmUnit,
        const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

    static PFtrExt Load(const TWPt<TBase>& Base, TSIn& SIn);
    void Save(TSOut& SOut) const;   
    
	TStr GetNm() const { return "DateWnd[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return FtrGen.GetDim(); }
	TStr GetFtr(const int& FtrN) const { return GetNm(); } //TODO return actual range

	void Clr() { FtrGen.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	void InvFullV(const TFltV& FullV, int& Offset, TFltV& InvV) const;

    // feature extractor type name 
    static TStr GetType() { return "dateWindow"; }
};

} // TFtrExts namespace

} // namespace

#endif

/**
 * QMiner - Open Source Analytics Platform
 * 
 * Copyright (C) 2014 Quintelligence d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Contact: 
 *   Blaz Fortuna <blaz@blazfortuna.com>
 *
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
    /// Stream aggregate descriptions
	static TFunRouter<PFtrExt, TNewF> NewRouter;   
public:
    /// Register default aggregates
    static void Init();
    /// Register new aggregate
    template <class TObj> static void Register() { 
        NewRouter.Register(TObj::GetType(), TObj::New);
    }    
private:    
    /// QMiner Base pointer
    TWPt<TBase> Base;
    
    /// Map from record store to join which brings it to feature extraction store
	THash<TUCh, TJoinSeq> JoinSeqH;
	/// The end store from which features are actually extracted 
	TWPt<TStore> FtrStore;

protected:
    /// Get pointer to QMiner base
    const TWPt<TBase>& GetBase() const { return Base; }

	/// Checkes the record store and uses appropriate join sequence 
    /// to derive a single join record from feature store
	TRec DoSingleJoin(const TRec& Rec) const;

    /// Feature extractor with multiple start stores
	TFtrExt(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV);
    /// Feature extractor parsed from JSon parameters
	TFtrExt(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Create new feature extractor from JSon parameters
	static PFtrExt New(const TWPt<TBase>& Base, const TStr& TypeNm, const PJsonVal& ParamVal);
    
	virtual ~TFtrExt() { }

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

	// for more strait-forward feature extraction (i.e. used by basic aggregators)
	// attaches values to the given vector, keeps what is in there already
	virtual void ExtractStrV(const TRec& Rec, TStrV& StrV) const;
	virtual void ExtractFltV(const TRec& Rec, TFltV& FltV) const;
	virtual void ExtractTmV(const TRec& Rec, TTmV& TmV) const;

	/// Check if the given store is one of the allowed start stores
	bool IsStartStore(const uchar& StoreId) const { return JoinSeqH.IsKey(StoreId); }
	/// Is there a join to be done when starting from the given store
	bool IsJoin(const uchar& StoreId) const { return JoinSeqH.GetDat(StoreId).IsJoin(); }
    /// Get the join sequence required to be executed for the given store
	const TJoinSeq& GetJoinSeq(const uchar& StoreId) const { return JoinSeqH.GetDat(StoreId); }
    /// Get the join IDs required to be executed for the given store
	const TIntPrV& GetJoinIdV(const uchar& StoreId) const { return JoinSeqH.GetDat(StoreId).GetJoinIdV(); }
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

	TFtrSpace(const TWPt<TBase>& _Base, const PFtrExt& FtrExt): 
        Base(_Base), FtrExtV(TFtrExtV::GetV(FtrExt)) { Init(); }
	TFtrSpace(const TWPt<TBase>& _Base, const TFtrExtV& _FtrExtV): 
        Base(_Base), FtrExtV(_FtrExtV) { Init(); }
public:
	static TPt<TFtrSpace> New(const TWPt<TBase>& Base, const PFtrExt& FtrExt) { 
        return new TFtrSpace(Base, FtrExt); }
	static TPt<TFtrSpace> New(const TWPt<TBase>& Base, const TFtrExtV& FtrExtV) { 
        return new TFtrSpace(Base, FtrExtV); }

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
	/// Compute sparse centroid of a given record set
	void GetCentroidSpV(const PRecSet& RecSet, TIntFltKdV& CentroidSpV, const bool& NormalizeP = true) const;
	/// Compute full centroid of a given record set
	void GetCentroidV(const PRecSet& RecSet, TFltV& CentroidV, const bool& NormalizeP = true) const;
    
    /// String vector for a record transformed by a feature extractor
    void ExtractStrV(const int& DimN, const PJsonVal& RecVal, TStrV &StrV) const;

	/// Dimensionality of the feature space
	int GetDim() const;
	/// String representation of the FtrN-th feature
	TStr GetFtr(const int& FtrN) const;

	/// Prepares an empty bow and registers all the features
	PBowDocBs MakeBowDocBs(const PRecSet& FtrRecSet);    
};
typedef TPt<TFtrSpace> PFtrSpace;

///////////////////////////////////////////////
/// Implemented feature extractors.
namespace TFtrExts {

///////////////////////////////////////////////
/// Random feature extractor.
class TRandom : public TFtrExt {
private:
	// random feature generator
	mutable TRnd Rnd;

	TRandom(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed);
	TRandom(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& RndSeed);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Random"; };
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return "Random"; }

	void Clr() { };
    bool Update(const TRec& Rec) { return false; }
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

	// flat feature extraction
	void ExtractFltV(const TRec& FtrRec, TFltV& FltV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "random"; }   
};

///////////////////////////////////////////////
/// Numeric Feature Extractor
class TNumeric : public TFtrExt {
private:
    /// numeric extractor type
    typedef enum { tNone, tNormalize, tMnMxVal } TType;
private:
    /// Feature generator type
    TType Type;
    /// Minimal value for normalization
    TFlt MnVal;
    /// Maximal value for normalization
    TFlt MxVal;
    
	/// Field Id
	TInt FieldId;
    /// Field description
    TFieldDesc FieldDesc;

    /// Get value from a given record
	double GetRecVal(const TRec& Rec) const; 
	double GetFtrVal(const TRec& Rec) const; 

	TNumeric(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& _FieldId, const bool& _NormalizeP);
    TNumeric(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, 
        const int& FieldId, const bool& NormalizeP = true);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Numeric[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return 1; }
	TStr GetFtr(const int& FtrN) const { return GetNm(); }

	void Clr();
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;
	void AddFullV(const TRec& Rec, TFltV& FullV, int& Offset) const;

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
// TODO do not transform integers to strings
class TCategorical : public TFtrExt {
private:
	// nominal feature generator
	TFtrGen::TNominal FtrGen;
	// field Id
	TInt FieldId;
    // field description
    TFieldDesc FieldDesc;

	TStr _GetVal(const TRec& FtrRec) const; 
	TStr GetVal(const TRec& Rec) const; 

	TCategorical(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId);
    TCategorical(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Categorical[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return FtrGen.GetVals(); }
	TStr GetFtr(const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void Clr() { FtrGen = TFtrGen::TNominal(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;

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
// TODO do not transform integers to strings
class TMultinomial : public TFtrExt {
private:
	// multinomial feature generator
	TFtrGen::TMultiNom FtrGen;
	// field Id
	TInt FieldId;
    // field description
    TFieldDesc FieldDesc;
    
	void ParseDate(const TTm& Tm, TStrV& StrV) const;
	void _GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const; 
	void _GetVal(const TRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const TRec& Rec, TStrV& StrV) const; 

	TMultinomial(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId);
    TMultinomial(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Multinomial[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return FtrGen.GetVals(); }
	TStr GetFtr(const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void Clr() { FtrGen = TFtrGen::TMultiNom(); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;
	void ExtractTmV(const TRec& Rec, TTmV& TmV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "multinomial"; }   
};

///////////////////////////////////////////////
// Bag-of-words Feature Extractor.
typedef enum { bowmConcat, bowmCentroid } TBagOfWordsMode;

class TBagOfWords : public TFtrExt {
private:
	// numeric feature generator
	TFtrGen::TToken FtrGen;
	// field Id
	TInt FieldId;
    // field description
    TFieldDesc FieldDesc;
 	// how to deal with multiple instances
	TBagOfWordsMode Mode;

	void _GetVal(const PRecSet& FtrRecSet, TStrV& StrV) const; 
	void _GetVal(const TRec& FtrRec, TStrV& StrV) const; 
	void GetVal(const TRec& Rec, TStrV& StrV) const; 

	TBagOfWords(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& _FieldId,
	  const TBagOfWordsMode& _Mode, const PSwSet& SwSet, const PStemmer& Stemmer);
    TBagOfWords(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, const int& FieldId, 
        const TBagOfWordsMode& Mode = bowmConcat, const PSwSet& SwSet = TSwSet::New(swstEn523), 
        const PStemmer& Stemmer = TStemmer::New(stmtPorter, false));
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& FieldId, 
        const TBagOfWordsMode& Mode = bowmConcat, const PSwSet& SwSet = TSwSet::New(swstEn523), 
        const PStemmer& Stemmer = TStemmer::New(stmtPorter, false));
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& FieldId, 
        const TBagOfWordsMode& Mode = bowmConcat, const PSwSet& SwSet = TSwSet::New(swstEn523), 
        const PStemmer& Stemmer = TStemmer::New(stmtPorter, false));
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "BagOfWords[" + GetFtrStore()->GetFieldNm(FieldId) + "]"; };
	int GetDim() const { return FtrGen.GetVals(); }
	TStr GetFtr(const int& FtrN) const { return FtrGen.GetVal(FtrN); }

	void Clr() { FtrGen = TFtrGen::TToken(FtrGen.GetSwSet(), FtrGen.GetStemmer()); }
	// sparse vector extraction
	bool Update(const TRec& Rec);
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;

	// flat feature extraction
	void ExtractStrV(const TRec& Rec, TStrV& StrV) const;

    // feature extractor type name 
    static TStr GetType() { return "text"; }   
    
private:
	static PSwSet GetSwSet(const TStr& SwStr);
	static void AddWords(const PSwSet& SwSet, const PJsonVal& WordsVal);    
public:
    /// Extract stop-words from JSon parameters
    static PSwSet ParseSwSet(const PJsonVal& ParamVal);
    /// Extract stemmer from JSon parameters.
    /// In case real word parameter is not give, value of RealWordP is used
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
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, const int& BucketSize = 1);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV, const int& BucketSize = 1);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Join[" + GetFtrStore()->GetStoreNm() + "]"; }
	int GetDim() const { return Dim; }
	TStr GetFtr(const int& FtrN) const { return GetFtrStore()->GetStoreNm(); }
		
	void Clr() { Def(); }
	bool Update(const TRec& Rec) { return false; }
	// sparse vector extraction
	void AddSpV(const TRec& Rec, TIntFltKdV& SpV, int& Offset) const;

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
public:
	static PFtrExt New(const TWPt<TBase>& Base, const TWPt<TStore>& Store, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeq& JoinSeq, 
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const TJoinSeqV& JoinSeqV,
        const PFtrExt& FtrExt1, const PFtrExt& FtrExt2);
	static PFtrExt New(const TWPt<TBase>& Base, const PJsonVal& ParamVal);

	TStr GetNm() const { return "Pair[" + FtrExt1->GetNm() + "," + FtrExt2->GetNm() + "]"; };
	int GetDim() const { return FtrIdPairH.Len(); }
	TStr GetFtr(const int& FtrN) const;

	void Clr() { FtrValH = TStrHash<TInt>(); FtrIdPairH.Clr(); }
	// sparse vector extraction
	bool Update(const TRec& FtrRec);
	void AddSpV(const TRec& FtrRec, TIntFltKdV& SpV, int& Offset) const;

	// flat feature extraction
	void ExtractStrV(const TRec& FtrRec, TStrV& StrV) const;
    
    // feature extractor type name 
    static TStr GetType() { return "pair"; }       
};

} // TFtrExts namespace

} // namespace

#endif
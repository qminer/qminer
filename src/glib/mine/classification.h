/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TClassification {

///////////////////////////////////////////
// Logistic Regression using the Newton-Raphson method
class TLogReg {
private:
	double Lambda;
	TFltV WgtV;

	bool IncludeIntercept;

	bool Verbose;
	PNotify Notify;

public:
	// default constructor, sets the regularization parameter
	TLogReg(const double& RegFact=1, const bool IncludeIntercept=false, const bool Verbose=true);
	TLogReg(TSIn& SIn);

	void Save(TSOut& SOut) const;

	// Fits the regression model. The method assumes that the instances are stored in the
	// columns of the matrix X and the responses are stored in vector y.
	void Fit(const TFltVV& X, const TFltV& y, const double& Eps=1e-3, const int& MxIter=10000);
	// returns the expected response for the given feature vector
	double Predict(const TFltV& x) const;

	void GetWgtV(TFltV& WgtV) const;

	// get functions
	const double& GetLambda() { return Lambda; }
	const bool& GetIntercept() { return IncludeIntercept; }
	// set functions
	void SetLambda(const double& _Lambda) { Lambda = _Lambda; }
	void SetIntercept(const bool& _IncludeIntercept) { IncludeIntercept = _IncludeIntercept; }

	bool Initialized() const { return !WgtV.Empty(); }
private:
	double PredictWithoutIntercept(const TFltV& x) const;
};

///////////////////////////////////////////
// Decision Tree - stopping criteria
class TDtSplitCriteria;
	typedef TPt<TDtSplitCriteria> PDtSplitCriteria;
class TDtSplitCriteria {
private:
	TCRef CRef;
public:
  	friend class TPt<TDtSplitCriteria>;
public:
  	virtual ~TDtSplitCriteria() {}

  	static PDtSplitCriteria Load(TSIn& SIn);
  	virtual void Save(TSOut& SOut) const;

	virtual double GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) const = 0;
protected:
	virtual const TStr GetType() const = 0;
};

///////////////////////////////////////////
// Information Gain Criterion
class TInfoGain: public TDtSplitCriteria {
public:
	TInfoGain(): TDtSplitCriteria() {}
	static PDtSplitCriteria New() { return new TInfoGain; }

	virtual double GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) const;

protected:
	virtual const TStr GetType() const { return "InfoGain"; }
};

/////////////////////////////////////////////
//// Gain Ratio Criterion
class TGainRatio: public TInfoGain {
public:
	double GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) const;
protected:
	const TStr GetType() const { return "GainRatio"; }
};

///////////////////////////////////////////
// Decision Tree - prunning
class TDtPruneCriteria;
	typedef TPt<TDtPruneCriteria> PDtPruneCriteria;
class TDtPruneCriteria {
private:
	TCRef CRef;
public:
  	friend class TPt<TDtPruneCriteria>;
public:
  	TDtPruneCriteria() {}
  	virtual ~TDtPruneCriteria() {}

  	static PDtPruneCriteria Load(TSIn& SIn);
  	virtual void Save(TSOut& SOut) const;

  	virtual bool ShouldPrune(const bool& IsLeaf, const int& NExamples, const double& Class1Prob) const = 0;

protected:
  	virtual const TStr GetType() const = 0;
};

class TDtMinExamplesPrune: public TDtPruneCriteria {
private:
	TInt MinExamples;
public:
	TDtMinExamplesPrune(const int& _MinExamples):
		TDtPruneCriteria(),
		MinExamples(_MinExamples) {}
	static PDtPruneCriteria New(const int& MinExamples) { return new TDtMinExamplesPrune(MinExamples); }
	TDtMinExamplesPrune(TSIn& SIn): MinExamples(SIn) {}

	void Save(TSOut& SOut) const;

	bool ShouldPrune(const bool& IsLeaf, const int& NExamples, const double& Class1Prob) const;

protected:
	const TStr GetType() const { return "byExamples"; }
};

///////////////////////////////////////////
// Decision Tree - prunning
class TDtGrowCriteria;
	typedef TPt<TDtGrowCriteria> PDtGrowCriteria;
class TDtGrowCriteria {
private:
	TCRef CRef;
public:
  	friend class TPt<TDtGrowCriteria>;
private:
  	TFlt MinPosClassProb;
  	TFlt MinNegClassProb;
  	TInt MinExamples;
public:
  	TDtGrowCriteria(const double& MinClassProb=0, const int& MinExamples=0);
  	TDtGrowCriteria(const double& MinPosClassProb, const double& MinNegClassProb,
  			const int& MinExamples=0);
  	TDtGrowCriteria(TSIn& SIn);

  	static PDtGrowCriteria New(const double& MinClassProb=0, const int& MinExamples=0) { return new TDtGrowCriteria(MinClassProb, MinExamples); }
  	static PDtGrowCriteria New(const double& MinPosClassProb, const double& MinNegClassProb,
  			const int& MinExamples=0) {
  		return new TDtGrowCriteria(MinPosClassProb, MinNegClassProb, MinExamples); }
  	virtual ~TDtGrowCriteria() {}

  	static PDtGrowCriteria Load(TSIn& SIn);
  	virtual void Save(TSOut& SOut) const;

  	virtual bool ShouldGrow(const int& NExamples, const double& Class1Prob) const;

protected:
  	virtual const TStr GetType() const { return "default"; }
};

///////////////////////////////////////////
// Decision Tree
class TDecisionTree {
private:
	class TNode {
		friend class TDecisionTree;
	private:


		TNode* Left;
		TNode* Right;

		TDecisionTree* Tree;

		int CutFtrN;
		double CutFtrVal;

		int NExamples;

		TFltV ClassHist;
		TFltV FtrHist;
		TFltIntFltTrV CutFtrCorrFtrNPValTrV;

		TNode(TDecisionTree* Tree);
		TNode(TDecisionTree* Tree, TSIn& SIn);
		~TNode() { CleanUp(); }

		void Save(TSOut& SOut) const;

		double Predict(const TFltV& FtrV) const;

		PJsonVal GetJson() const;
		PJsonVal ExplainLabel(const int& Label) const;

		void Fit(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV);
		bool Prune();
		void CopyNode(const TNode& Node);
		void SetTree(TDecisionTree* Tree);

		bool CanSplitNumFtr(const TFltIntPrV& ValClassPrV, const int& TotalPos,
				double& CutVal, double& Score) const;
		void CalcCorrFtrV(const TFltVV& FtrVV, const TIntV& InstNV);
		void Split(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& InstNV);

		bool HasLeft() const { return Left != nullptr; }
		bool HasRight() const { return Right != nullptr; }
		bool IsLeaf() const { return !HasLeft() && !HasRight(); }

		bool ShouldGrow() const;
		bool ShouldPrune() const;

		void CleanUp();
	};

	TNode* Root;

	PDtSplitCriteria SplitCriteria;
	PDtPruneCriteria PruneCriteria;
	PDtGrowCriteria GrowCriteria;

	TBool CalcCorr;

public:
	TDecisionTree(const PDtSplitCriteria& SplitCriteria=TInfoGain::New(),
			const PDtPruneCriteria& PruneCriteria=TDtMinExamplesPrune::New(1),
			const PDtGrowCriteria& GrowCriteria=TDtGrowCriteria::New(),
			const bool& CalcCorr=false);
	TDecisionTree(TSIn& SIn);
	TDecisionTree(const TDecisionTree& Other);
#ifdef GLib_CPP11
	TDecisionTree(TDecisionTree&& Other);
#endif
	~TDecisionTree() { CleanUp(); }

	TDecisionTree& operator =(const TDecisionTree& Tree);
#ifdef GLib_CPP11
	TDecisionTree& operator =(TDecisionTree&& Tree);
#endif

	void Save(TSOut& SOut) const;

	void Fit(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify=TNotify::NullNotify);
	double Predict(const TFltV& FtrV) const;

	PJsonVal GetJson() const;
	PJsonVal ExplainPositive() const;

private:
	void Grow(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify);
	void Prune(const PNotify& Notify);

	double GetSplitScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) const;
	bool ShouldPrune(const bool& IsLeaf, const int& NExamples, const double& Class1Prob) const;
	bool ShouldGrow(const int& NExamples, const double& Class1Prob) const;
	bool IsCalcCorr() const { return CalcCorr; }

	bool HasRoot() const { return Root != nullptr; }
	void CleanUp();
};

}

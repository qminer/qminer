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
	void Fit(const TFltVV& X, const TFltV& y, const double& Eps=1e-3);
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
// Information Gain Criterion
class TInfoGain {
public:
	static double GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN);
};

///////////////////////////////////////////
// Gain Ratio Criterion
class TGainRatio {
public:
	static double GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) {

		const double InfoGain = TInfoGain::GetScore(LeftLen, RightLen, LeftPosN, RightPosN);
		return InfoGain / TSpecFunc::Entropy(double(LeftLen) / (LeftLen + RightLen));
	}
};

///////////////////////////////////////////
// Decision Tree
class TDecisionTree {
private:
	typedef TFltIntIntTr TFtrClassInstNTr;
	typedef TVec<TFltIntIntTr> TFtrClassInstNTrV;
	typedef TVec<TFtrClassInstNTrV> TFtrClassInstNTrVV;

	class THistogram {
	private:
		TFltV BinV;
	public:
		THistogram();
		THistogram(const int& Bins);
		THistogram(TSIn& SIn);

		void Save(TSOut& SOut) const;

		PJsonVal GetJson() const;

		void Set(const int& BinN, const double& Val);
		double Get(const int& BinN) const;
	};

	class TNode {
		TNode* Left;
		TNode* Right;

		int CutFtrN;
		double CutFtrVal;

		int NExamples;

		THistogram ClassHist;
		THistogram FtrHist;

	public:
		TNode();
		TNode(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
				const PNotify& Notify);
		TNode(TSIn& SIn);
		TNode(const TNode& Node);
		~TNode();

		TNode& operator=(const TNode& Node);

		void Save(TSOut& SOut) const;

		double Predict(const TFltV& FtrV) const;

		PJsonVal GetJson() const;

		bool Prune();

	private:
		void Grow(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
				const PNotify& Notify);

		bool CanSplitNumFtr(TFltIntPrV& ValClassPrV, double& CutVal, double& Score) const;
		void CalcStats(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
				const PNotify& Notify);
		bool IsLeaf() const { return Left == nullptr && Right == nullptr; }
		bool ShouldPrune() const;

		static void CopyNode(const TNode& Node, TNode& Copy);
		void CleanUp();
	};

	TNode* Root;

public:
	TDecisionTree();
	TDecisionTree(TSIn& SIn);
	TDecisionTree(const TDecisionTree& Other);
#ifdef GLib_CPP11
	TDecisionTree(TDecisionTree&& Other);
#endif
	~TDecisionTree();

	TDecisionTree& operator=(const TDecisionTree& Tree);

	void Save(TSOut& SOut) const;

	void Fit(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify=TNotify::NullNotify);
	double Predict(const TFltV& FtrV) const;

	PJsonVal GetJson() const;

private:
	void Grow(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify);
	void Prune(const PNotify& Notify);

	void CleanUp();
};

}

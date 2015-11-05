/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TClassification;

///////////////////////////////////////////
// Logistic Regression
TLogReg::TLogReg(const double& _Lambda, const bool _IncludeIntercept, const bool _Verbose):
		Lambda(_Lambda),
		WgtV(),
		IncludeIntercept(_IncludeIntercept),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TLogReg::TLogReg(TSIn& SIn):
		Lambda(TFlt(SIn)),
		WgtV(SIn),
		IncludeIntercept(TBool(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TLogReg::Save(TSOut& SOut) const {
	TFlt(Lambda).Save(SOut);
	WgtV.Save(SOut);
	TBool(IncludeIntercept).Save(SOut);
	TBool(Verbose).Save(SOut);
}


void TLogReg::Fit(const TFltVV& _X, const TFltV& y, const double& Eps) {
	TFltVV X(_X);

	if (IncludeIntercept) {
		// add 1s into the last row
		X.AddXDim();
		for (int i = 0; i < X.GetCols(); i++) {
			X(X.GetRows()-1, i) = 1;
		}
	}

	const int NInst = X.GetCols();
	const int Dim = X.GetRows();
	const int OrigDim = IncludeIntercept ? Dim-1 : Dim;
	EAssertR(NInst == y.Len(), "TLogReg::Fit the number of instances in X.GetCols() and y.Len() do not match");
	// minimize the following objective function:
	// L(w) = (sum(log(1 + exp(w*x_i)) - y_i*w*x_i) + lambda*beta*beta'/2) / m
	// using Newton-Raphson algorithm:
	// w <- w - H^(-1)(w)*g(w)
	// g(w) = (X*(s(beta*x) - y)' + lambda*beta')
	// H(w) = X*W*X^(-1) + lambda*I
	// where H is the Hessian at point w, g is the gradient of the objective function at point w
	// W is a diagonal matrix defined as W_ii = p_i(1 - p_i)

	// temporary variables
	TFltV ProbV(NInst, NInst);					// vector of probabilities
	TFltV PrevProbV(NInst, NInst);				// vector of probs in the previous step, used to terminate the procedure
	TFltV DeltaWgtV(Dim, Dim);					// the step used to update the weights
	TFltV YMinP(NInst, NInst);
	TFltV GradV(Dim, Dim);						// gradient
	TFltVV XTimesW(Dim, NInst);					// temporary variable to compute (X*W)*X'
	TFltVV H(Dim, Dim);							// Hessian
	TFltVV X_t(X.GetCols(), X.GetRows());	TLinAlg::Transpose(X, X_t);		// the transposed instance matrix
	TVec<TIntFltKdV> WgtColSpVV(NInst, NInst);	// weight matrix

	// generate weight matrix with only ones on the diagonal
	// so you don't recreate all the object every iteration
	for (int i = 0; i < NInst; i++) {
		WgtColSpVV[i].Add(TIntFltKd(i, 1));
	}

	WgtV.Gen(Dim);

	// perform the algorithm
	double Diff = TFlt::NInf;
	double AbsDiff;
	int k = 1;
	do {
		if (k % 10 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Step: %d, diff: %.3f", k, Diff);
		}

		// compute the probabilities p_i = 1 / (1 + exp(-w*x_i)) and
		// compute the weight matrix diagonal W_ii = p_i(1 - p_i)
		TLinAlg::Multiply(X_t, WgtV, ProbV);
		for (int i = 0; i < NInst; i++) {
			ProbV[i] = 1 / (1 + TMath::Power(TMath::E, -ProbV[i]));
			WgtColSpVV[i][0].Dat = ProbV[i]*(1 - ProbV[i]);
		}

		// compute the Hessian H = X*W*X' + lambda*I
		// 1) compute X*W
		TLinAlg::Multiply(X, WgtColSpVV, XTimesW);
		// 2) compute H = (X*W)*X'
		TLinAlg::Multiply(XTimesW, X_t, H);
		// 3) add lambda to the diagonal of H, exclude the punishment for the intercept
		for (int i = 0; i < OrigDim; i++) {
			H(i,i) += Lambda;
		}

		// compute the gradient g(w) = X*(y - p)' + lambda * w
		// 1) compute (y - p)
		TLinAlg::LinComb(1, y, -1, ProbV, YMinP);
		// 2) compute X*(y - p)
		TLinAlg::Multiply(X, YMinP, GradV);
		// 3) add lambda * w, exclude the punishment for the intercept
		for (int i = 0; i < OrigDim; i++) {
			GradV[i] += Lambda*WgtV[i];
		}

		// compute delta_w = H(w) \ (g(w))
#ifdef LAPACKE
		const TFlt SingEps = 1e-10;

		if (H.GetRows() == 1) {	// fix for a bug in SVD factorization
			DeltaWgtV[0] = GradV[0] / H(0,0);
		} else {
			TLinAlg::SVDSolve(H, DeltaWgtV, GradV, SingEps);
		}
#else
		throw TExcept::New("Should include LAPACKE!!");
#endif

		if (TFlt::IsNan(TLinAlg::Norm(DeltaWgtV))) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Got NaNs while fitting logistic regression! The weights could still be OK.");
			break;
		}

		// update the current weight vector
		for (int i = 0; i < Dim; i++) {
			WgtV[i] += DeltaWgtV[i];
		}

		// recompute the termination criteria and store the probabilities for
		// the next iteration
		Diff = TFlt::NInf;
		for (int i = 0; i < NInst; i++) {
			AbsDiff = TFlt::Abs(PrevProbV[i] - ProbV[i]);
			if (AbsDiff > Diff) { Diff = AbsDiff; }

			PrevProbV[i] = ProbV[i];
		}

		k++;
	} while (Diff > Eps);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged. Diff: %.5f", Diff);
}

double TLogReg::Predict(const TFltV& x) const {
	if (IncludeIntercept) {
		TFltV x1(x);	x1.Add(1);
		return PredictWithoutIntercept(x1);
	} else {
		return PredictWithoutIntercept(x);
	}
}

void TLogReg::GetWgtV(TFltV& _WgtV) const {
	_WgtV = WgtV;
	if (IncludeIntercept) {
		_WgtV.DelLast();
	}
}

double TLogReg::PredictWithoutIntercept(const TFltV& x) const {
	if (!Initialized()) { return 0; }
	EAssertR(x.Len() == WgtV.Len(), "Dimension mismatch while predicting!");
	return 1 / (1 + TMath::Power(TMath::E, -TLinAlg::DotProduct(WgtV, x)));
}


///////////////////////////////////////////
// Information Gain
double TInfoGain::GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
		const int& RightPosN) {

	const double EntS = TSpecFunc::Entropy(double(LeftPosN + RightPosN) / (LeftLen + RightLen));
	const double EntLeft = TSpecFunc::Entropy(double(LeftPosN) / LeftLen);
	const double EntRight = TSpecFunc::Entropy(double(RightPosN) / RightLen);

	const double ProbLeft = double(LeftLen) / (LeftLen + RightLen);
	const double ProbRight = 1 - ProbLeft;

	return EntS - ProbLeft*EntLeft - ProbRight*EntRight;
}

///////////////////////////////////////////
// Decision Tree - Histogram
TDecisionTree::THistogram::THistogram():
		BinV() {}

TDecisionTree::THistogram::THistogram(const int& Bins):
		BinV(Bins) {}

TDecisionTree::THistogram::THistogram(TSIn& SIn):
		BinV(SIn) {}

void TDecisionTree::THistogram::Save(TSOut& SOut) const {
	BinV.Save(SOut);
}

PJsonVal TDecisionTree::THistogram::GetJson() const {
	return TJsonVal::NewArr(BinV);
}

void TDecisionTree::THistogram::Set(const int& BinN, const double& Val) {
	EAssert(BinN < BinV.Len());
	BinV[BinN] = Val;
}

double TDecisionTree::THistogram::Get(const int& BinN) const {
	EAssert(BinN < BinV.Len());
	return BinV[BinN];
}

///////////////////////////////////////////
// Decision Tree - Node
TDecisionTree::TNode::TNode(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
			const PNotify& Notify):
		Left(nullptr),
		Right(nullptr),
		CutFtrN(-1),
		CutFtrVal(TFlt::NInf),
		NExamples(0),
		ClassHist(),
		FtrHist() {
	Grow(FtrVV, ClassV, NodeInstNV, Notify);
}

TDecisionTree::TNode::TNode():
		Left(nullptr),
		Right(nullptr),
		CutFtrN(-1),
		CutFtrVal(TFlt::NInf),
		NExamples(0),
		ClassHist(),
		FtrHist() {}

TDecisionTree::TNode::TNode(TSIn& SIn):
		Left(nullptr),
		Right(nullptr),
		CutFtrN(TInt(SIn)),
		CutFtrVal(TFlt(SIn)),
		NExamples(TInt(SIn)),
		ClassHist(SIn),
		FtrHist(SIn) {

	const TBool HasLeft(SIn);
	const TBool HasRight(SIn);

	if (HasLeft) { Left = new TNode(SIn); }
	if (HasRight) { Right = new TNode(SIn); }
}

TDecisionTree::TNode::TNode(const TNode& Node):
		Left(nullptr),
		Right(nullptr) {
	CopyNode(Node, *this);
}

TDecisionTree::TNode::~TNode() {
	CleanUp();
}

TDecisionTree::TNode& TDecisionTree::TNode::operator =(const TNode& Node) {
	if (this != &Node) {
		CopyNode(Node, *this);
	}

	return *this;
}

void TDecisionTree::TNode::Save(TSOut& SOut) const {
	TInt(CutFtrN).Save(SOut);
	TFlt(CutFtrVal).Save(SOut);
	TInt(NExamples).Save(SOut);
	ClassHist.Save(SOut);
	FtrHist.Save(SOut);

	// save children
	const TBool HasLeft = Left != nullptr;
	const TBool HasRight = Right != nullptr;

	HasLeft.Save(SOut);
	HasRight.Save(SOut);

	if (HasLeft) { Left->Save(SOut); }
	if (HasRight) { Right->Save(SOut); }
}

double TDecisionTree::TNode::Predict(const TFltV& FtrV) const {
	if (IsLeaf()) {
		return ClassHist.Get(1);
	} else if (FtrV[CutFtrN] <= CutFtrVal) {
		return Left->Predict(FtrV);
	} else {
		return Right->Predict(FtrV);
	}
}

PJsonVal TDecisionTree::TNode::GetJson() const {
	PJsonVal RootJson = TJsonVal::NewObj();

	const bool IsLeaf = Left == nullptr && Right == nullptr;

	RootJson->AddToObj("examples", NExamples);
	RootJson->AddToObj("classes", ClassHist.GetJson());
	RootJson->AddToObj("features", FtrHist.GetJson());

	if (!IsLeaf) {
		PJsonVal CutJson = TJsonVal::NewObj();
		CutJson->AddToObj("id", CutFtrN);
		CutJson->AddToObj("value", CutFtrVal);
		RootJson->AddToObj("cut", CutJson);
	}

	PJsonVal ChildrenJson = TJsonVal::NewArr();

	if (Left != nullptr) {
		ChildrenJson->AddToArr(Left->GetJson());
	}
	if (Right != nullptr) {
		ChildrenJson->AddToArr(Right->GetJson());
	}

	RootJson->AddToObj("children", ChildrenJson);

	return RootJson;
}

bool TDecisionTree::TNode::Prune() {
	bool PruneLeft = false;
	bool PruneRight = false;

	if (Left != nullptr) {
		PruneLeft = Left->Prune();
	}
	if (Right != nullptr) {
		PruneRight = Right->Prune();
	}

	if (PruneLeft && PruneRight) {
		delete Left;	Left = nullptr;
		delete Right;	Right = nullptr;
	}

	bool PruneMe = ShouldPrune();
	return PruneMe;
}

void TDecisionTree::TNode::Grow(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
		const PNotify& Notify) {
	const int NInst = NodeInstNV.Len();

	EAssert(!NodeInstNV.Empty());

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Creating a node with %d instances ...", NodeInstNV.Len());

	const int Dim = FtrVV.GetRows();

	CutFtrN = -1;

	TFltIntPrV ValClassPrV(NInst);

	// get the best score and cut value
	double BestScore = TFlt::NInf, CutVal, Score;
	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		for (int i = 0; i < NInst; i++) {
			const int InstN = NodeInstNV[i];
			ValClassPrV[i].Val1 = FtrVV(FtrN, InstN);
			ValClassPrV[i].Val2 = ClassV[InstN];
		}


		if (CanSplitNumFtr(ValClassPrV, CutVal, Score) && Score > BestScore) {
			BestScore = Score;
			CutFtrN = FtrN;
			CutFtrVal = CutVal;
		}
	}

	// cut the dataset into left and right and build the tree recursively
	if (CutFtrN >= 0) {
		int NInstLeft = 0;
		for (int i = 0; i < NInst; i++) {
			if (FtrVV(CutFtrN, NodeInstNV[i]) <= CutFtrVal) {
				NInstLeft++;
			}
		}

		TIntV LeftInstNV(NInstLeft, 0), RightInstNV(NInst - NInstLeft, 0);

		int CurrInstN;
		for (int i = 0; i < NInst; i++) {
			CurrInstN = NodeInstNV[i];
			if (FtrVV(CutFtrN, CurrInstN) <= CutFtrVal) {
				LeftInstNV.Add(CurrInstN);
			} else {
				RightInstNV.Add(CurrInstN);
			}
		}

		// get the dimension of the new matrices
		Left = new TNode(FtrVV, ClassV, LeftInstNV, Notify);
		Right = new TNode(FtrVV, ClassV, RightInstNV, Notify);
	}

	CalcStats(FtrVV, ClassV, NodeInstNV, Notify);
}

bool TDecisionTree::TNode::CanSplitNumFtr(TFltIntPrV& ValClassPrV, double& CutVal, double& Score) const {
	const int NInst = ValClassPrV.Len();

	ValClassPrV.Sort(true);

	// count the total number of positive instances
	int TotalPos = 0;
	for (int i = 0; i < NInst; i++) {
		TotalPos += ValClassPrV[i].Val2;
	}

	Score = TFlt::NInf;

	int PosS0 = 0;	// the number of positive instances in the left set

	int PosS1, S0Len, S1Len;
	double CurrScore;

	for (int CutN = 0; CutN < NInst-1; CutN++) {
		const TFltIntPr& FtrValClassPr = ValClassPrV[CutN];
		const TFltIntPr& NextFtrValClassPr = ValClassPrV[CutN+1];

		const double CurrVal = FtrValClassPr.Val1;
		const double NextVal = NextFtrValClassPr.Val1;

		const int CurrClass = FtrValClassPr.Val2;
		const int NextClass = NextFtrValClassPr.Val2;

		PosS0 += CurrClass;

		// the cut point always occurs on the boundary between two classes
		// so if the class doesn't change there is not need to check
		if (CurrVal != NextVal && CurrClass != NextClass) {
			S0Len = CutN + 1;
			S1Len = NInst - S0Len;
			PosS1 = TotalPos - PosS0;

			CurrScore = TInfoGain::GetScore(S0Len, S1Len, PosS0, PosS1);

			if (CurrScore > Score) {
				Score = CurrScore;
				CutVal = (CurrVal + NextVal) / 2;
			}
		}
	}

	return Score != TFlt::NInf;
}

void TDecisionTree::TNode::CalcStats(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& NodeInstNV,
		const PNotify& Notify) {
	Notify->OnNotify(TNotifyType::ntInfo, "Calculating statistics ...");

	const int NInst = NodeInstNV.Len();
	const int Dim = FtrVV.GetRows();

	ClassHist = THistogram(2);
	FtrHist = THistogram(Dim);

	EAssert(Dim > 0);

	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		double FtrSum = 0;

		for (int i = 0; i < NInst; i++) {
			FtrSum += FtrVV(FtrN, NodeInstNV[i]);
		}

		FtrHist.Set(FtrN, FtrSum / NInst);
	}

	double Prob1 = 0;
	for (int i = 0; i < NInst; i++) {
		Prob1 += ClassV[NodeInstNV[i]];
	}

	Prob1 /= NInst;
	ClassHist.Set(0, 1 - Prob1);
	ClassHist.Set(1, Prob1);

	NExamples = NInst;
}

bool TDecisionTree::TNode::ShouldPrune() const {
	return IsLeaf() && NExamples < 1;
}

void TDecisionTree::TNode::CopyNode(const TNode& Node, TNode& Copy) {
	Copy.CleanUp();

	Copy.CutFtrN = Node.CutFtrN;
	Copy.CutFtrVal = Node.CutFtrVal;
	Copy.NExamples = Node.NExamples;
	Copy.ClassHist = Node.ClassHist;
	Copy.FtrHist = Node.FtrHist;

	if (Node.Left != nullptr) {
		Copy.Left = new TNode();
		CopyNode(*Node.Left, *Copy.Left);
	}
	if (Node.Right != nullptr) {
		Copy.Right = new TNode();
		CopyNode(*Node.Right, *Copy.Right);
	}
}

void TDecisionTree::TNode::CleanUp() {
	if (Left != nullptr) { delete Left; }
	if (Right != nullptr) { delete Right; }

	Left = nullptr;
	Right = nullptr;
}


///////////////////////////////////////////
// Decision Tree
TDecisionTree::TDecisionTree(): Root(nullptr) {}

TDecisionTree::TDecisionTree(TSIn& SIn): Root(nullptr) {
	const TBool HasRoot(SIn);

	if (HasRoot) {
		Root = new TNode(SIn);
	}
}

TDecisionTree::TDecisionTree(const TDecisionTree& Other):
		Root(nullptr) {

	if (Other.Root != nullptr) {
		Root = new TNode(*Other.Root);
	}
}

#ifdef GLib_CPP11
TDecisionTree::TDecisionTree(TDecisionTree&& Tree) {
	CleanUp();
	Root = Tree.Root;
	Tree.Root = nullptr;
}
#endif

TDecisionTree::~TDecisionTree() {
	CleanUp();
}

TDecisionTree& TDecisionTree::operator =(const TDecisionTree& Tree) {
	if (this != &Tree) {
		CleanUp();

		if (Tree.Root != nullptr) {
			Root = new TNode(*Tree.Root);
		}
	}

	return *this;
}

void TDecisionTree::Save(TSOut& SOut) const {
	const TBool HasRoot = Root != nullptr;

	HasRoot.Save(SOut);
	if (HasRoot) {
		Root->Save(SOut);
	}
}

void TDecisionTree::Fit(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify) {
	const int NInst = FtrVV.GetCols();

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Building a decision tree on %d instances ...", NInst);

	Grow(FtrVV, ClassV, Notify);
	Prune(Notify);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Done!");
}

double TDecisionTree::Predict(const TFltV& FtrV) const {
	EAssert(Root != nullptr);
	return Root->Predict(FtrV);
}

PJsonVal TDecisionTree::GetJson() const {
	if (Root == nullptr) { return TJsonVal::NewObj(); }
	return Root->GetJson();
}

void TDecisionTree::Grow(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify) {
	CleanUp();
	TIntV InstNV(FtrVV.GetCols());	TLAUtil::Range(FtrVV.GetCols(), InstNV);
	Root = new TNode(FtrVV, ClassV, InstNV, Notify);
}

void TDecisionTree::Prune(const PNotify& Notify) {
	if (Root != nullptr) {
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Prunning ...");
		Root->Prune();
	}
}

void TDecisionTree::CleanUp() {
	if (Root != nullptr) {
		delete Root;
		Root = nullptr;
	}
}

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


void TLogReg::Fit(const TFltVV& _X, const TFltV& y, const double& Eps, const int& MxIter) {
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
		if (k >= MxIter) {
		    printf("Reached the maximum number of iterations, will break ...\n");
		    break;
		}
		if (k % 10 == 0) {
			printf("Step: %d, diff: %.3f\n", k, Diff);
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
			printf("Got NaNs while fitting logistic regression! The weights could still be OK.\n");
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
// Decision Tree - Splitting criteria
PDtSplitCriteria TDtSplitCriteria::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "InfoGain") {
		return new TInfoGain();
	} else if (Type == "GainRatio") {
		return new TGainRatio();
	} else {
		throw TExcept::New("Invalid type: " + Type);
	}
}

void TDtSplitCriteria::Save(TSOut& SOut) const {
	GetType().Save(SOut);
}

///////////////////////////////////////////
// Information Gain
double TInfoGain::GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
		const int& RightPosN) const {

	const double EntS = TSpecFunc::Entropy(double(LeftPosN + RightPosN) / (LeftLen + RightLen));
	const double EntLeft = TSpecFunc::Entropy(double(LeftPosN) / LeftLen);
	const double EntRight = TSpecFunc::Entropy(double(RightPosN) / RightLen);

	const double ProbLeft = double(LeftLen) / (LeftLen + RightLen);
	const double ProbRight = 1 - ProbLeft;

	return EntS - ProbLeft*EntLeft - ProbRight*EntRight;
}

/////////////////////////////////////////////
//// Gain Ratio
double TGainRatio::GetScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
		const int& RightPosN) const {

	const double InfoGain = TInfoGain::GetScore(LeftLen, RightLen, LeftPosN, RightPosN);
	return InfoGain / TSpecFunc::Entropy(double(LeftLen) / (LeftLen + RightLen));
}

///////////////////////////////////////////
// Decision Tree - prunning
PDtPruneCriteria TDtPruneCriteria::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "byExamples") {
		return new TDtMinExamplesPrune(SIn);
	} else {
		throw TExcept::New("Invalid type: " + Type);
	}
}

void TDtPruneCriteria::Save(TSOut& SOut) const {
	GetType().Save(SOut);
}

///////////////////////////////////////////
// Decision Tree - prunning by the numbre of examples
void TDtMinExamplesPrune::Save(TSOut& SOut) const {
	TDtPruneCriteria::Save(SOut);
	MinExamples.Save(SOut);
}

bool TDtMinExamplesPrune::ShouldPrune(const bool& IsLeaf, const int& NExamples, const double& Class1Prob) const {
	return IsLeaf && NExamples < 1;
}

///////////////////////////////////////////
// Decision Tree - growing
TDtGrowCriteria::TDtGrowCriteria(const double& _MinClassProb, const int& _MinExamples):
		MinPosClassProb(_MinClassProb),
		MinNegClassProb(MinPosClassProb),
		MinExamples(_MinExamples) {}

TDtGrowCriteria::TDtGrowCriteria(const double& _MinPosClassProb, const double& _MinNegClassProb, const int& _MinExamples):
		MinPosClassProb(_MinPosClassProb),
		MinNegClassProb(_MinNegClassProb),
		MinExamples(_MinExamples) {}

TDtGrowCriteria::TDtGrowCriteria(TSIn& SIn):
		MinPosClassProb(SIn),
		MinNegClassProb(SIn),
		MinExamples(SIn) {}

PDtGrowCriteria TDtGrowCriteria::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "default") {
		return new TDtGrowCriteria(SIn);
	} else {
		throw TExcept::New("Invalid type: " + Type);
	}
}

void TDtGrowCriteria::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	MinPosClassProb.Save(SOut);
	MinNegClassProb.Save(SOut);
	MinExamples.Save(SOut);
}

bool TDtGrowCriteria::ShouldGrow(const int& NExamples, const double& Class1Prob) const {
	return NExamples > MinExamples && Class1Prob > MinPosClassProb && (1 - Class1Prob) > MinNegClassProb;
}

///////////////////////////////////////////
// Decision Tree - Node
TDecisionTree::TNode::TNode(TDecisionTree* _Tree):
		Left(nullptr),
		Right(nullptr),
		Tree(_Tree),
		CutFtrN(-1),
		CutFtrVal(TFlt::NInf),
		NExamples(0),
		ClassHist(),
		FtrHist(),
		CutFtrCorrFtrNPValTrV() {}

TDecisionTree::TNode::TNode(TDecisionTree* _Tree, TSIn& SIn):
		Left(nullptr),
		Right(nullptr),
		Tree(_Tree),
		CutFtrN(TInt(SIn)),
		CutFtrVal(TFlt(SIn)),
		NExamples(TInt(SIn)),
		ClassHist(SIn),
		FtrHist(SIn),
		CutFtrCorrFtrNPValTrV(SIn) {

	const TBool HasLeft(SIn);
	const TBool HasRight(SIn);

	if (HasLeft) { Left = new TNode(Tree, SIn); }
	if (HasRight) { Right = new TNode(Tree, SIn); }
}

void TDecisionTree::TNode::Save(TSOut& SOut) const {
	TInt(CutFtrN).Save(SOut);
	TFlt(CutFtrVal).Save(SOut);
	TInt(NExamples).Save(SOut);
	ClassHist.Save(SOut);
	FtrHist.Save(SOut);
	CutFtrCorrFtrNPValTrV.Save(SOut);

	// save children
	const TBool SaveLeft = HasLeft();
	const TBool SaveRight = HasRight();

	SaveLeft.Save(SOut);
	SaveRight.Save(SOut);

	if (SaveLeft) { Left->Save(SOut); }
	if (SaveRight) { Right->Save(SOut); }
}

double TDecisionTree::TNode::Predict(const TFltV& FtrV) const {
	if (IsLeaf()) {
		return ClassHist[1];
	} else if (FtrV[CutFtrN] <= CutFtrVal) {
		return Left->Predict(FtrV);
	} else {
		return Right->Predict(FtrV);
	}
}

PJsonVal TDecisionTree::TNode::GetJson() const {
	PJsonVal RootJson = TJsonVal::NewObj();

	RootJson->AddToObj("examples", NExamples);
	RootJson->AddToObj("classes", TJsonVal::NewArr(ClassHist));
	RootJson->AddToObj("features", TJsonVal::NewArr(FtrHist));

	if (!IsLeaf()) {
		PJsonVal CutJson = TJsonVal::NewObj();

		CutJson->AddToObj("id", CutFtrN);
		CutJson->AddToObj("value", CutFtrVal);

		if (Tree->IsCalcCorr()) {
			PJsonVal FtrCorrJson = TJsonVal::NewArr();
			for (int i = 0; i < CutFtrCorrFtrNPValTrV.Len(); i++) {
				const TFltIntFltTr& FtrCorr = CutFtrCorrFtrNPValTrV[i];

				const TFlt& Corr = FtrCorr.Val1;
				const TInt& FtrN = FtrCorr.Val2;
				const TFlt& PVal = FtrCorr.Val3;

				PJsonVal CorrJson = TJsonVal::NewObj();
				CorrJson->AddToObj("id", FtrN);
				CorrJson->AddToObj("corr", Corr);
				CorrJson->AddToObj("p", PVal);

				FtrCorrJson->AddToArr(CorrJson);
			}
			CutJson->AddToObj("alternatives", FtrCorrJson);
		}

		RootJson->AddToObj("cut", CutJson);
	}

	PJsonVal ChildrenJson = TJsonVal::NewArr();

	if (HasLeft()) {
		ChildrenJson->AddToArr(Left->GetJson());
	}
	if (HasRight()) {
		ChildrenJson->AddToArr(Right->GetJson());
	}

	RootJson->AddToObj("children", ChildrenJson);

	return RootJson;
}

PJsonVal TDecisionTree::TNode::ExplainLabel(const int& Label) const {
	if (IsLeaf()) {
		if (ClassHist[Label] <= ClassHist[1 - Label]) {
			return PJsonVal();
		} else {
			const double Prob = ClassHist[Label];

			PJsonVal Result = TJsonVal::NewArr();

			PJsonVal IntersectJson = TJsonVal::NewObj();
			IntersectJson->AddToObj("covered", int(NExamples*Prob));
			IntersectJson->AddToObj("purity", Prob);
			IntersectJson->AddToObj("terms", TJsonVal::NewArr());

			Result->AddToArr(IntersectJson);

			return Result;
		}
	}

	PJsonVal Result = TJsonVal::NewArr();

	if (HasLeft()) {
		PJsonVal LeftUnion = Left->ExplainLabel(Label);

		if (!LeftUnion.Empty()) {
			if (LeftUnion->GetArrVals() == 0) {
				LeftUnion->AddToArr(TJsonVal::NewArr());
			}

			for (int i = 0; i < LeftUnion->GetArrVals(); i++) {
				PJsonVal IntersectJson = LeftUnion->GetArrVal(i);
				PJsonVal TermsJson = IntersectJson->GetObjKey("terms");
				bool HadFtr = false;
				for (int TermN = 0; TermN < TermsJson->GetArrVals(); TermN++) {
					PJsonVal TermJson = TermsJson->GetArrVal(TermN);

					const int TermFtrN = TermJson->GetObjInt("ftrId");
					if (TermFtrN == CutFtrN) {
						HadFtr = true;
						if (TermJson->GetObjNum("le") == TFlt::PInf) {
							TermJson->AddToObj("le", CutFtrVal);
						}
					}
				}
				if (!HadFtr) {
					PJsonVal TermJson = TJsonVal::NewObj();
					TermJson->AddToObj("ftrId", CutFtrN);
					TermJson->AddToObj("le", CutFtrVal);
					TermJson->AddToObj("gt", TFlt::NInf);
					TermsJson->AddToArr(TermJson);
				}

				Result->AddToArr(IntersectJson);
			}
		}
	}
	if (HasRight()) {
		PJsonVal RightUnion = Right->ExplainLabel(Label);

		if (!RightUnion.Empty()) {
			if (RightUnion->GetArrVals() == 0) {
				RightUnion->AddToArr(TJsonVal::NewArr());
			}

			for (int i = 0; i < RightUnion->GetArrVals(); i++) {
				PJsonVal IntersectJson = RightUnion->GetArrVal(i);
				PJsonVal TermsJson = IntersectJson->GetObjKey("terms");

				bool HadFtr = false;
				for (int TermN = 0; TermN < TermsJson->GetArrVals(); TermN++) {
					PJsonVal TermJson = TermsJson->GetArrVal(TermN);

					const int TermFtrN = TermJson->GetObjInt("ftrId");
					if (TermFtrN == CutFtrN) {
						HadFtr = true;
						if (TermJson->GetObjNum("gt") == TFlt::NInf) {
							TermJson->AddToObj("gt", CutFtrVal);
						}
					}
				}
				if (!HadFtr) {
					PJsonVal TermJson = TJsonVal::NewObj();
					TermJson->AddToObj("ftrId", CutFtrN);
					TermJson->AddToObj("le", TFlt::PInf);
					TermJson->AddToObj("gt", CutFtrVal);
					TermsJson->AddToArr(TermJson);
				}

				Result->AddToArr(IntersectJson);
			}
		}
	}

	return Result->GetArrVals() > 0 ? Result : PJsonVal();
}

void TDecisionTree::TNode::Fit(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& InstNV) {
	EAssert(!InstNV.Empty());

	const int Dim = FtrVV.GetRows();

	NExamples = InstNV.Len();

	ClassHist.Gen(2);
	FtrHist.Gen(Dim);

	{
		int TotalPos = 0;
		double BestScore = TFlt::NInf, CutVal = TFlt::NInf, Score = TFlt::NInf;

		for (int i = 0; i < NExamples; i++) {
			AssertR(0 <= InstNV[i] && InstNV[i] < FtrVV.GetCols(), "Invalid instance index: " + TInt::GetStr(InstNV[i]) + "!");
			TotalPos += (int) ClassV[InstNV[i]];
		}

		ClassHist[0] = 1 - double(TotalPos) / NExamples;
		ClassHist[1] = 1 - ClassHist[0];

		TFltIntPrV ValClassPrV(NExamples);

		// get the best score and cut value
		int InstN;
		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			double FtrSum = 0;

			for (int i = 0; i < NExamples; i++) {
				InstN = InstNV[i];

				AssertR(0 <= InstN && InstN < FtrVV.GetCols(), "Invalid instance index: " + TInt::GetStr(InstN) + "!");

				ValClassPrV[i].Val1 = FtrVV(FtrN, InstN);
				ValClassPrV[i].Val2 = (int) ClassV[InstN];
				FtrSum += FtrVV(FtrN, InstN);
			}

			ValClassPrV.Sort(true);	// have to sort to speed up the calculation

			if (CanSplitNumFtr(ValClassPrV, TotalPos, CutVal, Score) && Score > BestScore) {
				BestScore = Score;
				CutFtrN = FtrN;
				CutFtrVal = CutVal;
			}

			FtrHist[FtrN] = FtrSum / NExamples;
		}
	}

	// cut the dataset into left and right and build the tree recursively
	if (ShouldGrow() && CutFtrN >= 0) {
		EAssert(CutFtrN < Dim);
		// the best attribute is now selected, calculate the correlation between the
		// selected attribute and other attributes, then split the node
		CalcCorrFtrV(FtrVV, InstNV);
		Split(FtrVV, ClassV, InstNV);
	}
}

bool TDecisionTree::TNode::Prune() {
	bool PruneLeft = false;
	bool PruneRight = false;

	if (HasLeft()) {
		PruneLeft = Left->Prune();
	}
	if (HasRight()) {
		PruneRight = Right->Prune();
	}

	if (PruneLeft && PruneRight) {
		CleanUp();
	}

	bool PruneMe = ShouldPrune();
	return PruneMe;
}

void TDecisionTree::TNode::CopyNode(const TNode& Node) {
	CleanUp();

	CutFtrN = Node.CutFtrN;
	CutFtrVal = Node.CutFtrVal;
	NExamples = Node.NExamples;
	ClassHist = Node.ClassHist;
	FtrHist = Node.FtrHist;

	if (Node.HasLeft()) {
		Left = new TNode(Tree);
		Left->CopyNode(*Node.Left);
	}
	if (Node.HasRight()) {
		Right = new TNode(Tree);
		Right->CopyNode(*Node.Right);
	}
}

void TDecisionTree::TNode::SetTree(TDecisionTree* _Tree) {
	Tree = _Tree;
	if (HasLeft()) { Left->SetTree(Tree); }
	if (HasRight()) { Right->SetTree(Tree); }
}

bool TDecisionTree::TNode::CanSplitNumFtr(const TFltIntPrV& ValClassPrV, const int& TotalPos,
		double& CutVal, double& Score) const {
	const int NInst = ValClassPrV.Len();
	const int MxCutN = NInst-1;

	Score = TFlt::NInf;
	int PosS0 = 0;	// the number of positive instances in the left set

	int CutN = 0;
	while (CutN < MxCutN) {
		const TFltIntPr& FtrValClassPr = ValClassPrV[CutN];

		const double& CurrVal = FtrValClassPr.Val1;
		const int& CurrClass = FtrValClassPr.Val2;

		PosS0 += CurrClass;

		// the cut point always occurs on the boundary between two classes
		// so if the class doesn't change there is not need to check
		if (CurrClass != ValClassPrV[CutN+1].Val2) {
			// if the values of the attribute are the same then move
			// to where they first change since that is where the cut will
			// actually be performed
			while (CutN < MxCutN && ValClassPrV[CutN+1].Val1 == CurrVal) {
				CutN++;
				PosS0 += ValClassPrV[CutN].Val2;
			}

			const int S0Len = CutN + 1;
			const int S1Len = NInst - S0Len;
			const int PosS1 = TotalPos - PosS0;

			const double CurrScore = Tree->GetSplitScore(S0Len, S1Len, PosS0, PosS1);

			if (CurrScore > Score) {
				Score = CurrScore;
				CutVal = (CurrVal + ValClassPrV[CutN+1].Val1) / 2;
			}
		}

		CutN++;
	}

	return Score != TFlt::NInf;
}

void TDecisionTree::TNode::CalcCorrFtrV(const TFltVV& FtrVV, const TIntV& InstNV) {
	if (Tree->IsCalcCorr()) {
		const int Dim = FtrVV.GetRows();

		CutFtrCorrFtrNPValTrV.Gen(Dim-1, 0);

		TFltV CutFtrV(NExamples), OthrFtrV(NExamples);
		for (int i = 0; i < NExamples; i++) {
			CutFtrV[i] = FtrVV(CutFtrN, InstNV[i]);
		}

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			if (FtrN != CutFtrN) {
				for (int i = 0; i < NExamples; i++) {
					OthrFtrV[i] = FtrVV(FtrN, InstNV[i]);
				}

				TCorr Corr(CutFtrV, OthrFtrV);
				CutFtrCorrFtrNPValTrV.Add(TFltIntFltTr(Corr.GetCorrCf(), FtrN, Corr.GetCorrCfPrb()));
			}
		}

		CutFtrCorrFtrNPValTrV.Sort(false);
	}
}

void TDecisionTree::TNode::Split(const TFltVV& FtrVV, const TFltV& ClassV, const TIntV& InstNV) {
	// construct the children
	int NInstLeft = 0;

	for (int i = 0; i < NExamples; i++) {
		AssertR(0 <= InstNV[i] && InstNV[i] < FtrVV.GetCols(), "Invalid instance index: " + TInt::GetStr(InstNV[i]) + "!");
		if (FtrVV(CutFtrN, InstNV[i]) <= CutFtrVal) {
			NInstLeft++;
		}
	}

	TIntV LeftInstNV(NInstLeft, 0);
	TIntV RightInstNV(NExamples - NInstLeft, 0);

	int InstN;
	for (int i = 0; i < NExamples; i++) {
		InstN = InstNV[i];
		AssertR(0 <= InstN && InstN < FtrVV.GetCols(), "Invalid instance index: " + TInt::GetStr(InstN) + "!");
		if (FtrVV(CutFtrN, InstN) <= CutFtrVal) {
			LeftInstNV.Add(InstN);
		} else {
			RightInstNV.Add(InstN);
		}
	}

	Left = new TNode(Tree);
	Right = new TNode(Tree);

	Left->Fit(FtrVV, ClassV, LeftInstNV);
	Right->Fit(FtrVV, ClassV, RightInstNV);
}

bool TDecisionTree::TNode::ShouldGrow() const {
	return Tree->ShouldGrow(NExamples, ClassHist[1]);
}

bool TDecisionTree::TNode::ShouldPrune() const {
	return Tree->ShouldPrune(IsLeaf(), NExamples, ClassHist[1]);
}

void TDecisionTree::TNode::CleanUp() {
	if (Left != nullptr) { delete Left; }
	if (Right != nullptr) { delete Right; }

	Left = nullptr;
	Right = nullptr;
}

///////////////////////////////////////////
// Decision Tree
TDecisionTree::TDecisionTree(const PDtSplitCriteria& _SplitCriteria, const PDtPruneCriteria& _PruneCriteria,
			const PDtGrowCriteria& _GrowCriteria, const bool& _CalcCorr):
		Root(nullptr),
		SplitCriteria(_SplitCriteria),
		PruneCriteria(_PruneCriteria),
		GrowCriteria(_GrowCriteria),
		CalcCorr(_CalcCorr) {}

TDecisionTree::TDecisionTree(TSIn& SIn):
		Root(nullptr),
		SplitCriteria(TDtSplitCriteria::Load(SIn)),
		PruneCriteria(TDtPruneCriteria::Load(SIn)),
		GrowCriteria(TDtGrowCriteria::Load(SIn)),
		CalcCorr(SIn) {

	const TBool LoadRoot(SIn);
	if (LoadRoot) {
		Root = new TNode(this, SIn);
	}
}

TDecisionTree::TDecisionTree(const TDecisionTree& Other):
		Root(nullptr),
		SplitCriteria(Other.SplitCriteria),
		PruneCriteria(Other.PruneCriteria),
		GrowCriteria(Other.GrowCriteria),
		CalcCorr(Other.CalcCorr) {

	if (Other.HasRoot()) {
		Root = new TNode(this);
		Root->CopyNode(*Other.Root);
	}
}

#ifdef GLib_CPP11
TDecisionTree::TDecisionTree(TDecisionTree&& Other):
		Root(nullptr),
		SplitCriteria(),
		PruneCriteria(),
		GrowCriteria(),
		CalcCorr(Other.CalcCorr) {
	std::swap(Root, Other.Root);
	std::swap(SplitCriteria, Other.SplitCriteria);
	std::swap(PruneCriteria, Other.PruneCriteria);
	std::swap(GrowCriteria, Other.GrowCriteria);

	if (HasRoot()) {
		Root->SetTree(this);
	}
}
#endif

TDecisionTree& TDecisionTree::operator =(const TDecisionTree& Tree) {
	if (this != &Tree) {
		CleanUp();

		SplitCriteria = Tree.SplitCriteria;
		PruneCriteria = Tree.PruneCriteria;
		GrowCriteria = Tree.GrowCriteria;
		CalcCorr = Tree.CalcCorr;

		if (Tree.HasRoot()) {
			Root = new TNode(this);
			Root->CopyNode(*Tree.Root);
		}
	}

	return *this;
}

#ifdef GLib_CPP11
TDecisionTree& TDecisionTree::operator =(TDecisionTree&& Tree) {
//	printf("Tree: move operator called: this == %s, that == %s\n", TUInt64(this).GetStr().CStr(), TUInt64(&Tree).GetStr().CStr());
	if (this != &Tree) {
		std::swap(Root, Tree.Root);
		std::swap(SplitCriteria, Tree.SplitCriteria);
		std::swap(PruneCriteria, Tree.PruneCriteria);
		std::swap(GrowCriteria, Tree.GrowCriteria);
		std::swap(CalcCorr, Tree.CalcCorr);

		if (HasRoot()) {
			Root->SetTree(this);
		}
	}

	return *this;
}
#endif

void TDecisionTree::Save(TSOut& SOut) const {
	SplitCriteria->Save(SOut);
	PruneCriteria->Save(SOut);
	GrowCriteria->Save(SOut);
	CalcCorr.Save(SOut);

	const TBool SaveRoot = HasRoot();

	SaveRoot.Save(SOut);
	if (SaveRoot) {
		Root->Save(SOut);
	}
}

void TDecisionTree::Fit(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Building a decision tree on %d instances ...", FtrVV.GetCols());

	Grow(FtrVV, ClassV, Notify);
	Prune(Notify);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Done!");
}

double TDecisionTree::Predict(const TFltV& FtrV) const {
	EAssert(HasRoot());
	return Root->Predict(FtrV);
}

PJsonVal TDecisionTree::GetJson() const {
	if (!HasRoot()) { return TJsonVal::NewObj(); }
	return Root->GetJson();
}

PJsonVal TDecisionTree::ExplainPositive() const {
	if (!HasRoot()) { return TJsonVal::NewArr(); }
	PJsonVal Result = Root->ExplainLabel(1);
	if (Result.Empty()) { return TJsonVal::NewArr(); }
	return Result;
}

void TDecisionTree::Grow(const TFltVV& FtrVV, const TFltV& ClassV, const PNotify& Notify) {
	CleanUp();
	const int NInst = FtrVV.GetCols();

	TIntV RangeV(NInst);	TLinAlgTransform::RangeV(NInst, RangeV);

	Root = new TNode(this);
	Root->Fit(FtrVV, ClassV, RangeV);
}

void TDecisionTree::Prune(const PNotify& Notify) {
	if (HasRoot()) {
//		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Prunning ...");
		Root->Prune();
	}
}

double TDecisionTree::GetSplitScore(const int& LeftLen, const int& RightLen, const int& LeftPosN,
			const int& RightPosN) const {
	return SplitCriteria->GetScore(LeftLen, RightLen, LeftPosN, RightPosN);
}

bool TDecisionTree::ShouldPrune(const bool& IsLeaf, const int& NExamples, const double& Class1Prob) const {
	return PruneCriteria->ShouldPrune(IsLeaf, NExamples, Class1Prob);
}

bool TDecisionTree::ShouldGrow(const int& NExamples, const double& Class1Prob) const {
	return GrowCriteria->ShouldGrow(NExamples, Class1Prob);
}

void TDecisionTree::CleanUp() {
	if (HasRoot()) { delete Root; }
	Root = nullptr;
}

#include "ml.h"

using namespace TMl;

//////////////////////////////////////////////////
// Abstract clustering
const int TFullClust::MX_ITER = 10000;

TFullClust::TFullClust(const int _NHistBins, const double& _Sample, const TRnd& _Rnd, const bool& _Verbose):
		Rnd(_Rnd),
		CentroidMat(),
		CentroidDistStatV(),
		NHistBins(_NHistBins),
		Sample(_Sample),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
	EAssertR(NHistBins >= 2, "Should have at least 2 bins for the histogram!");
}

TFullClust::TFullClust(TSIn& SIn) {
	Rnd = TRnd(SIn);
	CentroidMat.Load(SIn);
	CentroidDistStatV.Load(SIn);
	NHistBins = TInt(SIn);
	FtrBinStartVV.Load(SIn);
	HistStat.Load(SIn);
	Sample = TFlt(SIn);
	Verbose = TBool(SIn);
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TFullClust::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	Rnd.Save(SOut);
	CentroidMat.Save(SOut);
	CentroidDistStatV.Save(SOut);
	TInt(NHistBins).Save(SOut);
	FtrBinStartVV.Save(SOut);
	HistStat.Save(SOut);
	TFlt(Sample).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PFullClust TFullClust::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "kmeans") {
		return new TFullKMeans(SIn);
	} else if (Type == "dpmeans") {
		return new TDpMeans(SIn);
	} else {
		throw TExcept::New("Invalid clustering type: " + Type, "TClust::Load");
	}
}

void TFullClust::Init(const TFullMatrix& X) {
	EAssertR(Sample >= 0, "Cannot sample a negative number of instances!");

	const int NInst = X.GetCols();

	Notify->OnNotify(TNotifyType::ntInfo, "Clustering ...");

	if (Sample == 1) {
		Apply(X, MX_ITER);
	} else {
		const int NSamples = Sample < 1 ? ceil(NInst*Sample) : TMath::Mn(NInst, int(Sample));

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Sampling %d instances...", NSamples);

		TIntV SampleV(NInst, 0);
		for (int i = 0; i < NInst; i++) {
			SampleV.Add(i);
		}

		SampleV.Shuffle(Rnd);
		SampleV.Trunc(NSamples);

		Apply(X(TVector::Range(X.GetRows()), SampleV), MX_ITER);
	}

	InitHistogram(X);

	Notify->OnNotify(TNotifyType::ntInfo, "Done.");
}

void TFullClust::InitHistogram(const TFullMatrix& X) {
	Notify->OnNotify(TNotifyType::ntInfo, "Computing histograms ...");

	const int Dim = GetDim();

	const TIntV AssignV = Assign(X).GetIntVec();	// TODO not optimal

	FtrBinStartVV.Gen(Dim, NHistBins+1);
	HistStat.Clr();

	// compute min and max for every feature
	double MnVal, MxVal;
	double Span, BinSize;
	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		// find min and max value
		MnVal = TFlt::Mx;
		MxVal = TFlt::Mn;

		for (int InstN = 0; InstN < X.GetCols(); InstN++) {
			if (X(FtrN, InstN) < MnVal) { MnVal = X(FtrN, InstN); }
			if (X(FtrN, InstN) > MxVal) { MxVal = X(FtrN, InstN); }
		}

		Span = MxVal - MnVal;
		BinSize = Span / NHistBins;
		for (int i = 0; i < NHistBins + 1; i++) {
			FtrBinStartVV(FtrN, i) = MnVal + i*BinSize;
		}
	}

	// init bin counts
	for (int ClustId = 0; ClustId < GetClusts(); ClustId++) {
		HistStat.Add(TClustHistStat());
		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			HistStat[ClustId].Add(TFtrHistStat(0, TUInt64V(NHistBins+2, NHistBins+2)));
		}
	}

	// compute the histogram
	for (int InstN = 0; InstN < X.GetCols(); InstN++) {
		const TVector FeatV = X.GetCol(InstN);
		const int ClustId = AssignV[InstN];

		TClustHistStat& ClustHistStat = HistStat[ClustId];

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			const double FtrVal = X(FtrN, InstN);

			TFtrHistStat& FtrHistStat = ClustHistStat[FtrN];

			FtrHistStat.Val1++;
			TUInt64V& BinCountV = FtrHistStat.Val2;

			// determine the bin
			int BinIdx = 0;

			if (FtrVal >= FtrBinStartVV(FtrN, NHistBins)) { BinIdx = BinCountV.Len()-1; }
			else {
				while (BinIdx < FtrBinStartVV.GetCols() && FtrVal >= FtrBinStartVV(FtrN, BinIdx)) {
					BinIdx++;
				}
			}

			BinCountV[BinIdx]++;
		}
	}
}

int TFullClust::Assign(const TVector& x) const {
	return GetDistVec(x).GetMinIdx();
}

TVector TFullClust::Assign(const TFullMatrix& X) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Assigning %d instances ...", X.GetCols());

	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetClusts(), true);
	const TVector NormX2 = X.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return Assign(X, NormX2, NormC2, OnesN, OnesK);
}

void TFullClust::Assign(const TFullMatrix& InstMat, TIntV& AssignV) const {
	AssignV = Assign(InstMat).GetIntVec();
}

TFullMatrix TFullClust::GetDistMat(const TFullMatrix& X) const {
	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetClusts(), true);
	const TVector NormX2 = X.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return GetDistMat2(X, NormX2, NormC2, OnesN, OnesK).Sqrt();
}

TVector TFullClust::GetDistVec(const TVector& x) const {
	TVector xC = x.IsColVec() ? x.MulT(CentroidMat) : x * CentroidMat;
	return (CentroidMat.ColNorm2V() - (xC*2) + TVector::Ones(GetClusts(), false) * x.Norm2()).Sqrt();
}

double TFullClust::GetDist(const int& CentroidIdx, const TVector& Pt) const {
	return CentroidMat.GetCol(CentroidIdx).EuclDist(Pt);
}

TVector TFullClust::GetJoinedCentroid(const TIntV& CentroidIdV) const {
	TVector Result(GetDim());

	double TotalSize = 0;
	for (int i = 0; i < CentroidIdV.Len(); i++) {
		const int CentroidIdx = CentroidIdV[i];
		const uint64 CentroidSize = GetClustSize(CentroidIdx);

		Result += GetCentroid(CentroidIdx)*CentroidSize;
		TotalSize += CentroidSize;
	}

	return Result /= TotalSize;
}

double TFullClust::GetMeanPtCentDist(const int& CentroidIdx) const {
	EAssertR(CentroidIdx < GetClusts(), TStr::Fmt("TFullKMeans::GetMeanPtCentDist: Invalid centroid index: %d", CentroidIdx));
	return CentroidDistStatV[CentroidIdx].Val2 / CentroidDistStatV[CentroidIdx].Val1;
}

uint64 TFullClust::GetClustSize(const int& ClustIdx) const {
	return CentroidDistStatV[ClustIdx].Val1;
}

void TFullClust::GetHistogram(const int FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const {
	BinV.Gen(NHistBins+2);
	BinStartV.Clr();

	for (int i = 0; i < FtrBinStartVV.GetCols(); i++) {
		BinStartV.Add(FtrBinStartVV(FtrId, i));
	}

	for (int i = 0; i < StateSet.Len(); i++) {
		const int ClustId = StateSet[i];

		const TClustHistStat& ClustHistStat = HistStat[ClustId];
		const TFtrHistStat& FtrHistStat = ClustHistStat[FtrId];
		const TUInt64V& CountV = FtrHistStat.Val2;

		for (int j = 0; j < CountV.Len(); j++) {
			BinV[j] += CountV[j];
		}
	}

	TLinAlg::NormalizeL1(BinV);
}

void TFullClust::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

TVector TFullClust::Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK).GetColMinIdxV();
}

TFullMatrix TFullClust::GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return (NormC2 * OnesN) - (CentroidMat*2).MulT(X) + (OnesK * NormX2);
}

TFullMatrix TFullClust::SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV) {
	const int NInst = X.GetCols();
	const int NAttrs = X.GetRows();

	EAssertR(NInst >= NCentroids, "TFullClust::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

	AssignIdxV = TVector(NCentroids);

	// generate k random elements
	TVector PermV = TVector::Range(NInst);
	for (int i = 0; i < NCentroids; i++) {
		const int SwapIdx = Rnd.GetUniDevInt(i, NInst-1);

		// swap
		double Temp = PermV[SwapIdx];
		PermV[SwapIdx] = PermV[i];
		PermV[i] = Temp;

		AssignIdxV[i] = PermV[i];
	}

	return X(TVector::Range(NAttrs), AssignIdxV);
}

void TFullClust::UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV) {
	const int NInst = X.GetCols();
	const int K = CentroidMat.GetCols();

	// 1. create a sparse matrix (coordinate representation) that encodes the closest centroids
	TVector OnesN = TVector::Ones(NInst, false);

	TIntV RangeV(NInst, 0);
	for (int i = 0; i < NInst; i++) { RangeV.Add(i); }

	// create a matrix
	TSparseColMatrix AssignIdxMat(NInst, K);
	TSparseOps<TInt,TFlt>::CoordinateCreateSparseColMatrix(RangeV, AssignIdxV.GetIntVec(), OnesN.GetVec(), AssignIdxMat.ColSpVV, K);

	// 2. compute the number of points that belong to each centroid, invert
	TVector ColSumInvV(K, false);	AssignIdxMat.MultiplyT(OnesN.GetVec(), ColSumInvV.Vec);
	// invert
	ColSumInvV.Map([](TFlt Val) { return 1.0 / (1.0 + Val); });

	// 3. compute the centroids
	TSparseColMatrix ColSumDiag(K, K);	TLAMisc::Diag(ColSumInvV.GetVec(), ColSumDiag.ColSpVV);
	CentroidMat = ((X * AssignIdxMat) + CentroidMat) * ColSumDiag;
}

void TFullClust::InitStatistics(const TFullMatrix& X, const TVector& AssignV) {
	const int K = GetClusts();
	TFullMatrix DistMat = GetDistMat(X);

	TVector OnesDim = TVector::Ones(X.GetRows());

	CentroidDistStatV.Gen(K,K);
	for (int i = 0; i < K; i++) {
		const TFlt CurrClustIdx = i;
		TVector VCellIdxV = AssignV.Find([=](TFlt Val) { return Val == CurrClustIdx; });
		TVector Di = DistMat(i, VCellIdxV);

		CentroidDistStatV[i].Val1 = VCellIdxV.Len();
		CentroidDistStatV[i].Val2 = Di.Sum();
	}
}

TVector TFullClust::GetCentroid(const int& CentroidId) const {
	return CentroidMat.GetCol(CentroidId);
}


//////////////////////////////////////////////////
// K-Means
TFullKMeans::TFullKMeans(const int& _NHistBins, const double _Sample, const int& _K, const TRnd& _Rnd, const bool& _Verbose):
		TFullClust(_NHistBins, _Sample, _Rnd, _Verbose),
		K(_K) {}

TFullKMeans::TFullKMeans(TSIn& SIn):
		TFullClust(SIn) {
	K.Load(SIn);
}

void TFullKMeans::Save(TSOut& SOut) const {
	TFullClust::Save(SOut);
	K.Save(SOut);
}

void TFullKMeans::Apply(const TFullMatrix& X, const int& MaxIter) {
	EAssertR(K <= X.GetRows(), "Matrix should have more rows then k!");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing KMeans ...");

	const int NInst = X.GetCols();

	// select initial centroids
	TVector AssignIdxV;
	TVector OldAssignIdxV;
	CentroidMat = SelectInitCentroids(X, K, OldAssignIdxV);

	// pointers so we don't copy
	TVector* AssignIdxVPtr = &AssignIdxV;
	TVector* OldAssignIdxVPtr = &OldAssignIdxV;

	const TVector OnesN = TVector::Ones(NInst, false);
	const TVector OnesK = TVector::Ones(K, true);

	const TVector NormX2 = X.ColNorm2V();

	for (int i = 0; i < MaxIter; i++) {
		if (i % 10000 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		*AssignIdxVPtr = Assign(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the means
		UpdateCentroids(X, *AssignIdxVPtr);

		// swap old and new assign vectors
		TVector* Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}

	InitStatistics(X, AssignIdxV);
}

//////////////////////////////////////////////////
// DPMeans
TDpMeans::TDpMeans(const int& _NHistBins, const double& _Sample, const TFlt& _Lambda, const TInt& _MinClusts, const TInt& _MaxClusts, const TRnd& _Rnd, const bool& _Verbose):
		TFullClust(_NHistBins, _Sample, _Rnd, _Verbose),
		Lambda(_Lambda),
		MinClusts(_MinClusts),
		MaxClusts(_MaxClusts) {

	EAssertR(MinClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
	EAssertR(MaxClusts >= MinClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

TDpMeans::TDpMeans(TSIn& SIn):
		TFullClust(SIn) {
	Lambda.Load(SIn);
	MinClusts.Load(SIn);
	MaxClusts.Load(SIn);
}

void TDpMeans::Save(TSOut& SOut) const {
	TFullClust::Save(SOut);
	Lambda.Save(SOut);
	MinClusts.Save(SOut);
	MaxClusts.Save(SOut);
}

void TDpMeans::Apply(const TFullMatrix& X, const int& MaxIter) {
	EAssertR(MinClusts <= X.GetCols(), "Matrix should have more rows then the min number of clusters!");
	EAssertR(MinClusts <= MaxClusts, "Minimum number of cluster should be less than the maximum.");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing DPMeans ...");

	const int NInst = X.GetCols();

	const double LambdaSq = Lambda*Lambda;

	// select initial centroids
	TVector AssignIdxV;
	TVector OldAssignIdxV;
	CentroidMat = SelectInitCentroids(X, MinClusts, OldAssignIdxV);

	// pointers so we don't copy
	TVector* AssignIdxVPtr = &AssignIdxV;
	TVector* OldAssignIdxVPtr = &OldAssignIdxV;

	const TVector OnesN = TVector::Ones(NInst, false);
	const TVector NormX2 = X.ColNorm2V();

	TVector OnesK = TVector::Ones(GetClusts(), true);

	int i = 0;
	while (i++ < MaxIter) {
		if (i % 10 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// add new centroids and compute the distance matrix
		TFullMatrix D = GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

		// assign
		*AssignIdxVPtr = D.GetColMinIdxV();

		// check if we need to increase the number of clusters
		if (GetClusts() < MaxClusts) {
			TVector CentrDistV = D.GetColMinV();
			TVector MinIdxV = D.GetColMinIdxV();

			int NewCentrIdx = CentrDistV.GetMaxIdx();
			double MaxDist = CentrDistV[NewCentrIdx];

			if (MaxDist > LambdaSq) {
				CentroidMat.AddCol(X.GetCol(NewCentrIdx));
				OnesK = TVector::Ones(GetClusts(), true);
				(*AssignIdxVPtr)[NewCentrIdx] = GetClusts()-1;

				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Max distance to centroid: %.3f, number of clusters: %d ...", sqrt(MaxDist), GetClusts());
			}
		}

		// check if converged
		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the means
		UpdateCentroids(X, *AssignIdxVPtr);

		// swap old and new assign vectors
		TVector* Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}

	InitStatistics(X, AssignIdxV);
}


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
	double Diff;
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
		TNumericalStuff::SolveSymetricSystem(H, GradV, DeltaWgtV);

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
			if (TFlt::Abs(PrevProbV[i] - ProbV[i]) > Diff) {
				Diff = TFlt::Abs(PrevProbV[i] - ProbV[i]);
			}

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
	EAssertR(x.Len() == WgtV.Len(), "Dimension mismatch while predicting!");
	return 1 / (1 + TMath::Power(TMath::E, -TLinAlg::DotProduct(WgtV, x)));
}

///////////////////////////////////////////
// Exponential Regression
TExpReg::TExpReg(const double& _Lambda, const bool _Intercept, const bool _Verbose):
		Lambda(_Lambda),
		WgtV(),
		IncludeIntercept(_Intercept),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TExpReg::TExpReg(TSIn& SIn):
		Lambda(TFlt(SIn)),
		WgtV(SIn),
		IncludeIntercept(TBool(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TExpReg::Save(TSOut& SOut) const {
	TFlt(Lambda).Save(SOut);
	WgtV.Save(SOut);
	TBool(IncludeIntercept).Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TExpReg::Fit(const TFltVV& _X, const TFltV& y, const double& _Eps) {
	PerformFitChecks(_X, y);
	TFltVV X(_X);

	if (IncludeIntercept) {
		// add 1s into the last row
		X.AddXDim();
		for (int i = 0; i < X.GetCols(); i++) {
			X(X.GetRows()-1, i) = 1;
		}
	}

	// minimize the following objective function:
	// L(w) = (sum(lambda_i*t_i - log(lambda_i)) + gamma*beta*beta'/2) / m
	// where lambda are the approximated intensities, t_i are the observed times
	// and gamma is the regularization parameter
	// using Newton-Raphson algorithm:
	// w <- w - H^(-1)(w)*g(w)
	// g(w) = (lambda .* (1 - lambda .* t)*X' + gamma*beta) / m
	// H(w) = (X*W*X' + lambda*I) / m
	// where H is the Hessian at point w, g is the gradient of the objective function at point w
	// and W = diag(lambda .* lambda .*(2*lambda .* t - 1))
	const int NInst = X.GetCols();
	const int Dim = X.GetRows();
	const int OrigDim = IncludeIntercept ? Dim - 1 : Dim;

	const double Eps = Dim*_Eps;
	const double ConstrEsp = 1e-2;

	double Intens;
	double Diff = TFlt::PInf;

	TFltV IntensV(NInst, NInst);
	TFltV DeltaWgtV(Dim, Dim);
	TFltV TempNInstV(NInst, NInst);
	TFltV GradV(Dim, Dim);

	TFltVV X_t;	TLinAlg::Transpose(X, X_t);
	TFltVV H;
	TFltVV XTimesW(Dim, NInst);

	TVec<TIntFltKdV> WgtColSpVV(NInst, NInst);	// weight matrix

	WgtV.Gen(Dim);

	// generate weight matrix with only ones on the diagonal
	// so you don't recreate all the object every iteration
	for (int i = 0; i < NInst; i++) {
		WgtColSpVV[i].Add(TIntFltKd(i, 1));
	}

	// find an initial estimate
	// the initial estimate must lie in the region where the
	// function is convex:
	// solve the system (X*X' \ X*y'*eps_constr)'
	// this solution should lie in that region, or at least it should not lie in
	// beta*x_i < 0, where the algorithm has trouble converging
	Notify->OnNotify(TNotifyType::ntInfo, "Finding a feasible solution ...");

	// store y'*eps_constr into IntensV to not waste memory
	for (int i = 0; i < NInst; i++) {
		IntensV[i] = ConstrEsp * y[i];
	}

	// compute X*X'
	TLinAlg::Multiply(X, X_t, H);
	// compute X*y'*eps_constr
	TLinAlg::Multiply(X, IntensV, DeltaWgtV);
	// solve the linear system (X*X' \ X*y'*eps_constr)
	TNumericalStuff::LUSolve(H, WgtV, DeltaWgtV);

	Notify->OnNotify(TNotifyType::ntInfo, "Found initial estimate, optimizing ...");

	int k = 0;
	do {
		if (++k % 10 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Step: %d, diff: %.3f", k, Diff);
		}

		// construct the intensities
		// lambda_i = 1 / (beta*x_i)
		// first compute beta*X and then invert
		TLinAlg::MultiplyT(X, WgtV, IntensV);
		for (int i = 0; i < NInst; i++) {
			Intens = 1 / IntensV[i];

			IntensV[i] = Intens;
			WgtColSpVV[i][0].Dat = Intens * Intens * (2*Intens*y[i] - 1);

			if (IntensV[i] < 0) {
				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Intensity lower then 0: %.3f, the algorithm may have trouble converging!", IntensV[i].Val);
			}
		}

		// construct the hessian
		// 1) compute X*W
		TLinAlg::Multiply(X, WgtColSpVV, XTimesW);
		// 2) compute H = (X*W)*X'
		TLinAlg::Multiply(XTimesW, X_t, H);
		// 3) add lambda to the diagonal of H
		// exclude the punishment for the intercept
		for (int i = 0; i < OrigDim; i++) {
			H(i,i) += Lambda;
		}

		// construct the gradient
		// g = ((intens .* (1 - intens .* y)) * X' + lambda*beta)
		// 1) compute (intens .* (1 - intens .* y)
		for (int i = 0; i < NInst; i++) {
			TempNInstV[i] = IntensV[i] * (1 - IntensV[i]*y[i]);
		}
		// 2) compute ((intens .* (1 - intens .* y)) * X'
		TLinAlg::Multiply(X, TempNInstV, GradV);
		// 3) add lambda*beta, exclude the punishment for the intercept
		for (int i = 0; i < OrigDim; i++) {
			GradV[i] += Lambda*WgtV[i];
		}

		// solve -delta_wgts = H^(-1) * g
		TNumericalStuff::LUSolve(H, DeltaWgtV, GradV);

		// subtract the negative difference from the weights
		for (int i = 0; i < Dim; i++) {
			WgtV[i] -= DeltaWgtV[i];
		}

		// done, compute the norm of the difference to see if we need to exit
		Diff = TLinAlg::Norm(DeltaWgtV);
	} while (Diff > Eps);
}

double TExpReg::Predict(const TFltV& x) const {
	if (IncludeIntercept) {
		TFltV x1(x); x1.Add(1);
		return PredictWithoutIntercept(x1);
	} else {
		return PredictWithoutIntercept(x);
	}
}

void TExpReg::GetWgtV(TFltV& _WgtV) const {
	_WgtV = WgtV;
	if (IncludeIntercept) {
		_WgtV.DelLast();
	}
}

double TExpReg::PredictWithoutIntercept(const TFltV& x) const {
	EAssertR(x.Len() == WgtV.Len(), "Invalid dimension of the feature vector!");
	return 1 / TLinAlg::DotProduct(WgtV, x);
}

void TExpReg::PerformFitChecks(const TFltVV& X, const TFltV& y) const {
	EAssertR(X.GetCols() == y.Len(), "More instances than responses!");
	// check if the input is OK
	for (int i = 0; i < y.Len(); i++) {
		EAssertR(y[i] > 0, "Times must be greater than 0!");
	}
}

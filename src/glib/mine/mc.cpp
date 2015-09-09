/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TMc;

//////////////////////////////////////////////////////
// Distance measures
TFullMatrix TEuclDist::GetDist2(const TFullMatrix& X, const TFullMatrix& Y) {
	const TVector OnesX = TVector::Ones(X.GetCols(), true);
	const TVector OnesY = TVector::Ones(Y.GetCols(), false);
	const TVector NormX2 = X.ColNorm2V().Transpose();
	const TVector NormY2 = Y.ColNorm2V();

	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
}

//////////////////////////////////////////////////
// Abstract clustering
const int TStateIdentifier::MX_ITER = 10000;

TStateIdentifier::TStateIdentifier(const int _NHistBins, const double& _Sample, const TRnd& _Rnd, const bool& _Verbose):
		Rnd(_Rnd),
		CentroidMat(),
		CentroidDistStatV(),
		NHistBins(_NHistBins),
		StateContrFtrValVV(),
		Sample(_Sample),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
	EAssertR(NHistBins >= 2, "Should have at least 2 bins for the histogram!");
}

TStateIdentifier::TStateIdentifier(TSIn& SIn) {
	Rnd = TRnd(SIn);
	CentroidMat.Load(SIn);
	ControlCentroidMat.Load(SIn);
	CentroidDistStatV.Load(SIn);
	NHistBins = TInt(SIn);
	ObsFtrBinStartVV.Load(SIn);
	ContrFtrBinStartVV.Load(SIn);
	ObsHistStat.Load(SIn);
	ControlHistStat.Load(SIn);
	StateContrFtrValVV.Load(SIn);
	Sample = TFlt(SIn);
	Verbose = TBool(SIn);
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStateIdentifier::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	Rnd.Save(SOut);
	CentroidMat.Save(SOut);
	ControlCentroidMat.Save(SOut);
	CentroidDistStatV.Save(SOut);
	TInt(NHistBins).Save(SOut);
	ObsFtrBinStartVV.Save(SOut);
	ContrFtrBinStartVV.Save(SOut);
	ObsHistStat.Save(SOut);
	ControlHistStat.Save(SOut);
	StateContrFtrValVV.Save(SOut);
	TFlt(Sample).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PStateIdentifier TStateIdentifier::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "kmeans") {
		return new TFullKMeans(SIn);
	} else if (Type == "dpmeans") {
		return new TDpMeans(SIn);
	} else {
		throw TExcept::New("Invalid clustering type: " + Type, "TClust::Load");
	}
}

void TStateIdentifier::Init(TFltVV& ObsFtrVV, const TFltVV& ControlFtrVV) {
	EAssertR(Sample >= 0, "Cannot sample a negative number of instances!");

	const TFullMatrix X(ObsFtrVV, true);	// TODO remove TFullMatrix

	const int NInst = X.GetCols();

	Notify->OnNotify(TNotifyType::ntInfo, "Clustering ...");

	if (Sample == 1) {
		Apply(X, MX_ITER);
	} else {
		const int NSamples = Sample < 1 ? (int)ceil(NInst*Sample) : TMath::Mn(NInst, int(Sample));

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Sampling %d instances...", NSamples);

		TIntV SampleV(NInst, 0);
		for (int i = 0; i < NInst; i++) {
			SampleV.Add(i);
		}

		SampleV.Shuffle(Rnd);
		SampleV.Trunc(NSamples);

		Apply(X(TVector::Range(X.GetRows()), SampleV), MX_ITER);
	}

	InitControlCentroids(X.GetMat(), ControlFtrVV);
	InitHistogram(X.GetMat(), ControlFtrVV);
	ClearControlFtrVV(ControlFtrVV.GetRows());

	Notify->OnNotify(TNotifyType::ntInfo, "Done.");
}

void TStateIdentifier::InitHistogram(const TFltVV& ObsFtrVV, const TFltVV& ControlFtrVV) {
	Notify->OnNotify(TNotifyType::ntInfo, "Computing histograms ...");

	const int NClusts = GetStates();

	const TIntV AssignV = Assign(ObsFtrVV).GetIntVec();

	InitFtrBinStartVV(ObsFtrVV, NHistBins, ObsFtrBinStartVV);
	InitFtrBinStartVV(ControlFtrVV, NHistBins, ContrFtrBinStartVV);

	InitHist(ObsFtrVV, AssignV, ObsFtrBinStartVV, NClusts, NHistBins, ObsHistStat);
	InitHist(ControlFtrVV, AssignV, ContrFtrBinStartVV, NClusts, NHistBins, ControlHistStat);
}

int TStateIdentifier::Assign(const TFltV& x) const {
	TFltV DistV;	GetCentroidDistV(x, DistV);
	return TLAMisc::GetMinIdx(DistV);
}

int TStateIdentifier::Assign(const TVector& x) const {
	return Assign(x.Vec);
}

TVector TStateIdentifier::Assign(const TFltVV& X) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Assigning %d instances ...", X.GetCols());

	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetStates(), true);
	TVector NormX2(X.GetCols(), false);	TLinAlg::GetColNorm2V(X, NormX2.Vec);//XMat.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return Assign(X, NormX2, NormC2, OnesN, OnesK);
}

void TStateIdentifier::Assign(const TFltVV& InstMat, TIntV& AssignV) const {
	AssignV = Assign(InstMat).GetIntVec();
}

TFullMatrix TStateIdentifier::GetDistMat(const TFltVV& X) const {
	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetStates(), true);
	TVector NormX2(X.GetCols(), false);	TLinAlg::GetColNorm2V(X, NormX2.Vec);//X.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return GetDistMat2(X, NormX2, NormC2, OnesN, OnesK).Sqrt();
}

void TStateIdentifier::GetCentroidDistV(const TFltV& x, TFltV& DistV) const {
	// return (CentroidMat.ColNorm2V() - (x*C*2) + TVector::Ones(GetClusts(), false) * NormX2).Sqrt();
	// 1) squared norm of X
	const double NormX2 = TLinAlg::Norm2(x);

	// 2) Result <- CentroidMat.ColNorm2V()
	TLinAlg::GetColNorm2V(CentroidMat.GetMat(), DistV);

	// 3) 2*x*C
	TFltV xC;	TLinAlg::MultiplyT(CentroidMat.GetMat(), x, xC);

	// 4) <- Result = Result - 2*x*C + ones(clusts, 1)*|x|^2
	for (int i = 0; i < DistV.Len(); i++) {
		DistV[i] += NormX2 - 2*xC[i];
		DistV[i] = sqrt(DistV[i]);
	}
}

double TStateIdentifier::GetDist(const int& CentroidIdx, const TVector& Pt) const {
	return CentroidMat.GetCol(CentroidIdx).EuclDist(Pt);
}

TVector TStateIdentifier::GetJoinedCentroid(const TIntV& CentroidIdV) const {
	TVector Result(GetDim());

	double TotalSize = 0;
	for (int i = 0; i < CentroidIdV.Len(); i++) {
		const int CentroidIdx = CentroidIdV[i];
		const uint64 CentroidSize = GetClustSize(CentroidIdx);

		Result += GetCentroid(CentroidIdx)*(double)CentroidSize;
		TotalSize += CentroidSize;
	}

	return Result /= TotalSize;
}

TVector TStateIdentifier::GetJoinedControlCentroid(const TIntV& CentroidIdV) const {
	TVector Result(GetControlDim());

	double TotalSize = 0;
	for (int i = 0; i < CentroidIdV.Len(); i++) {
		const int CentroidIdx = CentroidIdV[i];
		const uint64 CentroidSize = GetClustSize(CentroidIdx);

		Result += GetControlCentroid(CentroidIdx)*(double)CentroidSize;
		TotalSize += CentroidSize;
	}

	return Result /= TotalSize;
}

double TStateIdentifier::GetMeanPtCentDist(const int& CentroidIdx) const {
	EAssertR(CentroidIdx < GetStates(), TStr::Fmt("TFullKMeans::GetMeanPtCentDist: Invalid centroid index: %d", CentroidIdx));
	return CentroidDistStatV[CentroidIdx].Val2 / CentroidDistStatV[CentroidIdx].Val1;
}

uint64 TStateIdentifier::GetClustSize(const int& ClustIdx) const {
	return CentroidDistStatV[ClustIdx].Val1;
}

void TStateIdentifier::GetHistogram(const int FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const {
	BinV.Gen(NHistBins+2);
	BinStartV.Clr();

	const TFltVV& BinStartVV = FtrId < ObsFtrBinStartVV.GetRows() ? ObsFtrBinStartVV : ContrFtrBinStartVV;
	const THistStat& HistStat = FtrId < ObsFtrBinStartVV.GetRows() ? ObsHistStat : ControlHistStat;
	const int FtrN = FtrId < ObsFtrBinStartVV.GetRows() ? FtrId : FtrId - ObsFtrBinStartVV.GetRows();

	for (int i = 0; i < BinStartVV.GetCols(); i++) {
		BinStartV.Add(BinStartVV(FtrN, i));
	}

	for (int i = 0; i < StateSet.Len(); i++) {
		const int ClustId = StateSet[i];

		const TClustHistStat& ClustHistStat = HistStat[ClustId];
		const TFtrHistStat& FtrHistStat = ClustHistStat[FtrN];
		const TUInt64V& CountV = FtrHistStat.Val2;

		for (int j = 0; j < CountV.Len(); j++) {
			BinV[j] += (double)CountV[j];
		}
	}

	TLinAlg::NormalizeL1(BinV);
}

void TStateIdentifier::GetCentroidVV(TVec<TFltV>& CentroidVV) const {
	CentroidVV.Gen(CentroidMat.GetCols());
	for (int ColN = 0; ColN < CentroidMat.GetCols(); ColN++) {
		CentroidVV[ColN].Gen(CentroidMat.GetRows());
		for (int RowN = 0; RowN < CentroidMat.GetRows(); RowN++) {
			CentroidVV[ColN][RowN] = CentroidMat(RowN, ColN);
		}
	}
}

void TStateIdentifier::GetControlCentroidVV(TStateFtrVV& StateFtrVV) const {
	const int Cols = ControlCentroidMat.GetCols();
	const int Rows = ControlCentroidMat.GetRows();

	StateFtrVV.Gen(Cols);

	for (int StateId = 0; StateId < Cols; StateId++) {
		StateFtrVV[StateId].Gen(Rows);
		for (int FtrId = 0; FtrId < Rows; FtrId++) {
			StateFtrVV[StateId][FtrId] = GetControlFtr(StateId, FtrId, ControlCentroidMat(FtrId, StateId));
		}
	}
}

double TStateIdentifier::GetControlFtr(const int& StateId, const int& FtrId, const double& DefaultVal) const {
	return IsControlFtrSet(StateId, FtrId) ? GetControlFtr(StateId, FtrId) : DefaultVal;
}

void TStateIdentifier::SetControlFtr(const int& StateId, const int& FtrId, const double& Val) {
	StateContrFtrValVV[StateId][FtrId] = Val;
}

void TStateIdentifier::ClearControlFtr(const int& StateId, const int& FtrId) {
	SetControlFtr(StateId, FtrId, TFlt::PInf);
}

void TStateIdentifier::ClearControlFtrVV() {
	ClearControlFtrVV(StateContrFtrValVV[0].Len());
}

bool TStateIdentifier::IsControlFtrSet(const int& StateId, const int& FtrId) const {
	return GetControlFtr(StateId, FtrId) != TFlt::PInf;
}

bool TStateIdentifier::IsAnyControlFtrSet() const {
	const int States = GetStates();
	const int Dim = GetControlDim();

	for (int StateId = 0; StateId < States; StateId++) {
		for (int FtrId = 0; FtrId < Dim; FtrId++) {
			if (IsControlFtrSet(StateId, FtrId)) {
				return true;
			}
		}
	}

	return false;
}

void TStateIdentifier::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

TVector TStateIdentifier::Assign(const TFltVV& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK).GetColMinIdxV();
}

TFullMatrix TStateIdentifier::GetDistMat2(const TFltVV& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return (NormC2 * OnesN) - (CentroidMat*2).MulT(X) + (OnesK * NormX2);
}

TFullMatrix TStateIdentifier::SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV) {
	const int NInst = X.GetCols();
	const int NAttrs = X.GetRows();

	EAssertR(NInst >= NCentroids, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

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

void TStateIdentifier::UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV) {
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

void TStateIdentifier::InitStatistics(const TFullMatrix& X, const TVector& AssignV) {
	const int K = GetStates();
	TFullMatrix DistMat = GetDistMat(X.GetMat());

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

void TStateIdentifier::InitControlCentroids(const TFltVV& ObsMat,  const TFltVV& ControlFtrVV) {
	const int NCentroids = GetStates();
	const int Dim = ControlFtrVV.GetRows();
	const int NInst = ControlFtrVV.GetCols();

	ControlCentroidMat = TFullMatrix(Dim, NCentroids);
	TUInt64V CountV(NCentroids);

	TIntV AssignV;	Assign(ObsMat, AssignV);
	for (int i = 0; i < NInst; i++) {
		CountV[AssignV[i]]++;
	}

	for (int ColN = 0; ColN < NInst; ColN++) {
		const int CentId = AssignV[ColN];
		for (int RowN = 0; RowN < Dim; RowN++) {
			ControlCentroidMat(RowN, CentId) += ControlFtrVV(RowN, ColN) / CountV[CentId];
		}
	}
}

TVector TStateIdentifier::GetCentroid(const int& CentroidId) const {
	return CentroidMat.GetCol(CentroidId);
}

TVector TStateIdentifier::GetControlCentroid(const int& CentroidId) const {
	return ControlCentroidMat.GetCol(CentroidId);
}

double TStateIdentifier::GetControlFtr(const int& StateId, const int& FtrId) const {
	return StateContrFtrValVV[StateId][FtrId];
}

void TStateIdentifier::ClearControlFtrVV(const int& Dim) {
	const int NStates = GetStates();

	StateContrFtrValVV.Gen(NStates);
	for (int i = 0; i < NStates; i++) {
		StateContrFtrValVV[i].Gen(Dim);
		for (int j = 0; j < Dim; j++) {
			StateContrFtrValVV[i][j] = TFlt::PInf;
		}
	}
}

void TStateIdentifier::InitHist(const TFltVV& InstanceMat, const TIntV& AssignV,
		const TFltVV& FtrBinStartVV, const int& Clusts, const int& Bins,
		THistStat& HistStat) {
	HistStat.Clr();

	const int NInst = InstanceMat.GetCols();
	const int Dim = InstanceMat.GetRows();

	// init bin counts
	for (int ClustId = 0; ClustId < Clusts; ClustId++) {
		HistStat.Add(TClustHistStat());
		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			HistStat[ClustId].Add(TFtrHistStat(0, TUInt64V(Bins+2, Bins+2)));
		}
	}

	// compute the histogram
	for (int InstN = 0; InstN < NInst; InstN++) {
		TFltV FtrV;	InstanceMat.GetCol(InstN, FtrV);
		const int ClustId = AssignV[InstN];

		TClustHistStat& ClustHistStat = HistStat[ClustId];

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			const double FtrVal = InstanceMat(FtrN, InstN);

			TFtrHistStat& FtrHistStat = ClustHistStat[FtrN];

			FtrHistStat.Val1++;
			TUInt64V& BinCountV = FtrHistStat.Val2;

			// determine the bin
			int BinIdx = 0;

			if (FtrVal >= FtrBinStartVV(FtrN, Bins)) { BinIdx = BinCountV.Len()-1; }
			else {
				while (BinIdx < FtrBinStartVV.GetCols() && FtrVal >= FtrBinStartVV(FtrN, BinIdx)) {
					BinIdx++;
				}
			}

			BinCountV[BinIdx]++;
		}
	}
}

void TStateIdentifier::InitFtrBinStartVV(const TFltVV& InstanceMat, const int& Bins,
		TFltVV& FtrBinStartVV) {
	const int NInst = InstanceMat.GetCols();
	const int Dim = InstanceMat.GetRows();

	FtrBinStartVV.Gen(Dim, Bins+1);

	// compute min and max for every feature
	double MnVal, MxVal;
	double Span, BinSize;
	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		// find min and max value
		MnVal = TFlt::Mx;
		MxVal = TFlt::Mn;

		for (int InstN = 0; InstN < NInst; InstN++) {
			if (InstanceMat(FtrN, InstN) < MnVal) { MnVal = InstanceMat(FtrN, InstN); }
			if (InstanceMat(FtrN, InstN) > MxVal) { MxVal = InstanceMat(FtrN, InstN); }
		}

		Span = MxVal - MnVal;
		BinSize = Span / Bins;
		for (int i = 0; i < Bins + 1; i++) {
			FtrBinStartVV(FtrN, i) = MnVal + i*BinSize;
		}
	}
}


//////////////////////////////////////////////////
// K-Means
TFullKMeans::TFullKMeans(const int& _NHistBins, const double _Sample, const int& _K, const TRnd& _Rnd, const bool& _Verbose):
		TStateIdentifier(_NHistBins, _Sample, _Rnd, _Verbose),
		K(_K) {}

TFullKMeans::TFullKMeans(TSIn& SIn):
		TStateIdentifier(SIn) {
	K.Load(SIn);
}

void TFullKMeans::Save(TSOut& SOut) const {
	TStateIdentifier::Save(SOut);
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

		*AssignIdxVPtr = Assign(X.GetMat(), NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

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
		TStateIdentifier(_NHistBins, _Sample, _Rnd, _Verbose),
		Lambda(_Lambda),
		MinClusts(_MinClusts),
		MaxClusts(_MaxClusts) {

	EAssertR(MinClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
	EAssertR(MaxClusts >= MinClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

TDpMeans::TDpMeans(TSIn& SIn):
		TStateIdentifier(SIn) {
	Lambda.Load(SIn);
	MinClusts.Load(SIn);
	MaxClusts.Load(SIn);
}

void TDpMeans::Save(TSOut& SOut) const {
	TStateIdentifier::Save(SOut);
	Lambda.Save(SOut);
	MinClusts.Save(SOut);
	MaxClusts.Save(SOut);
}

void TDpMeans::Apply(const TFullMatrix& X, const int& MaxIter) {
	EAssertR(X.GetRows() > 0, "The input matrix doesn't have any features!");
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

	TVector OnesK = TVector::Ones(GetStates(), true);

	int i = 0;
	while (i++ < MaxIter) {
		if (i % 10 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// add new centroids and compute the distance matrix
		TFullMatrix D = GetDistMat2(X.GetMat(), NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

		// assign
		*AssignIdxVPtr = D.GetColMinIdxV();

		// check if we need to increase the number of clusters
		if (GetStates() < MaxClusts) {
			TVector CentrDistV = D.GetColMinV();
			TVector MinIdxV = D.GetColMinIdxV();

			int NewCentrIdx = CentrDistV.GetMaxIdx();
			double MaxDist = CentrDistV[NewCentrIdx];

			if (MaxDist > LambdaSq) {
				CentroidMat.AddCol(X.GetCol(NewCentrIdx));
				OnesK = TVector::Ones(GetStates(), true);
				(*AssignIdxVPtr)[NewCentrIdx] = GetStates()-1;

				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Max distance to centroid: %.3f, number of clusters: %d ...", sqrt(MaxDist), GetStates());
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

/////////////////////////////////////////////
// MDS
TFullMatrix TEuclMds::Project(const TFullMatrix& X, const int& d) {
	// first center the rows of matrix X
	TFullMatrix X1 = X.GetCenteredRows();

	// Let B = X'X, then we can decompose B into its spectral decomposition
	// B = V*L*V' where L is a diagonal matrix of eigenvalues, and A holds the
	// corresponding eigenvectors in its columns

	// we can now aaproximate B with just the highest 'd' eigenvalues and
	// eigenvectors: B_d = V_d * L_d * V_d'
	// the coordinates of X can then be recovered by: X_d = V_d*L_d^(.5)

	// se can use SVD do find the eigenvectors V_d and eigenvalues L_d
	// X = U*S*V', where:
	// S is a diagonal matrix where {s_i^2} are the eigenvalues of X'X=B and X*X'
	// U holds the eigenvectors of X*X'
	// V holds the eigenvectors of X'X

	// so X_d = V_d * diag({|s_i|})
	TMatVecMatTr Svd = X1.Svd(d);

	const TVector& EigValsSqrt = Svd.Val2.Map([&](const TFlt& Val) { return fabs(Val); });
	const TFullMatrix& V = Svd.Val3;

	TFullMatrix X_d = V*TFullMatrix::Diag(EigValsSqrt);

	X_d.Transpose();

	return X_d;
}

void TAvgLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = (DistMat(MnI, i)*ItemCountV[MnI] + DistMat(MnJ, i)*ItemCountV[MnJ]) / (ItemCountV[MnI] + ItemCountV[MnJ]);
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

void TCompleteLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mx(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

void TSingleLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mn(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

/////////////////////////////////////////////////////////////////
// Agglomerative clustering
THierarch::THierarch(const bool& _HistCacheSize, const bool& _Verbose):
		HierarchV(),
		StateHeightV(),
		MxHeight(TFlt::Mn),
		HistCacheSize(_HistCacheSize),
		PastStateIdV(),
		StateCoordV(),
		NLeafs(0),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(HistCacheSize >= 1, "Have to hold at least the current state!");
}

THierarch::THierarch(TSIn& SIn):
		HierarchV(SIn),
		StateHeightV(SIn),
		UniqueHeightV(SIn),
		MxHeight(SIn),
		HistCacheSize(TInt(SIn)),
		PastStateIdV(SIn),
		StateCoordV(SIn),
		NLeafs(TInt(SIn)),
		StateNmV(SIn),
		TargetIdHeightSet(SIn),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void THierarch::Save(TSOut& SOut) const {
	HierarchV.Save(SOut);
	StateHeightV.Save(SOut);
	UniqueHeightV.Save(SOut);
	MxHeight.Save(SOut);
	TInt(HistCacheSize).Save(SOut);
	PastStateIdV.Save(SOut);
	StateCoordV.Save(SOut);
	TInt(NLeafs).Save(SOut);
	StateNmV.Save(SOut);
	TargetIdHeightSet.Save(SOut);
	TBool(Verbose).Save(SOut);
}

PHierarch THierarch::Load(TSIn& SIn) {
	return new THierarch(SIn);
}

void THierarch::Init(const TFullMatrix& CentroidMat, const int& CurrLeafId) {
	ClrFlds();

	NLeafs = CentroidMat.GetCols();

	// create a hierarchy
	TIntIntFltTrV MergeV;	TAlAggClust::MakeDendro(CentroidMat, MergeV, Notify);

	Notify->OnNotify(TNotifyType::ntInfo, TStrUtil::GetStr(MergeV, ", "));

	const int NMiddleStates = MergeV.Len();
	const int NStates = NLeafs + NMiddleStates;

	HierarchV.Gen(NStates);
	StateHeightV.Gen(NStates);

	for (int i = 0; i < HierarchV.Len(); i++) {
		HierarchV[i] = -1;
	}

	for (int i = 0; i < MergeV.Len(); i++) {
		const int LeafState1Idx = MergeV[i].Val1;
		const int LeafState2Idx = MergeV[i].Val2;
		const double Height = MergeV[i].Val3;

		// find the states into which state 1 and state 2 were merged
		const int State1Idx = GetOldestAncestIdx(LeafState1Idx);
		const int State2Idx = GetOldestAncestIdx(LeafState2Idx);
		const int MergeStateIdx = NLeafs + i;

		HierarchV[State1Idx] = MergeStateIdx;
		HierarchV[State2Idx] = MergeStateIdx;
		StateHeightV[MergeStateIdx] = Height;

		EAssertR(StateHeightV[MergeStateIdx] > StateHeightV[State1Idx] && StateHeightV[MergeStateIdx] > StateHeightV[State2Idx], "Parent should have greater height that any of its children!");

		if (Height > MxHeight) { MxHeight = Height; }
	}

	HierarchV.Last() = HierarchV.Len() - 1;

	// compute state coordinates
	ComputeStateCoords(CentroidMat, NStates);

	// initialize history
	GenUniqueHeightV(StateHeightV, UniqueHeightV);
	PastStateIdV.Gen(UniqueHeightV.Len(), UniqueHeightV.Len());
	UpdateHistory(CurrLeafId);

	// initialize state names
	StateNmV.Gen(HierarchV.Len());
}

void THierarch::UpdateHistory(const int& CurrLeafId) {
	const TFltV& HeightV = GetUniqueHeightV();
	TIntFltPrV StateIdHeightPrV;	GetAncestorV(CurrLeafId, StateIdHeightPrV);

	EAssertR(HeightV.Len() == PastStateIdV.Len(), "Number of heights doesn't match the number of heights in the past state cache!");

	int CurrHeightIdx = 0;
	for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
		const int CurrStateId = StateIdHeightPrV[i].Val1;

		while (CurrHeightIdx < HeightV.Len() && IsOnHeight(CurrStateId, HeightV[CurrHeightIdx])) {
			if (PastStateIdV[CurrHeightIdx].Empty() || PastStateIdV[CurrHeightIdx][0] != CurrStateId) {
				PastStateIdV[CurrHeightIdx].Ins(0, CurrStateId);
				// cleanup
				while (PastStateIdV[CurrHeightIdx].Len() > 2 /* HistCacheSize TODO */) { PastStateIdV[CurrHeightIdx].DelLast(); }
			}
			CurrHeightIdx++;
		}
	}
}

void THierarch::GetStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	for (int i = 0; i < HierarchV.Len(); i++) {
		StateIdHeightPrV.Add(TIntFltPr(i, GetStateHeight(i)));
	}
}

void THierarch::GetStateSetsAtHeight(const double& Height, TStateIdV& StateIdV,
		TStateSetV& StateSetV) const {
	TIntIntVH StateSubStateH;	GetAncSuccH(Height, StateSubStateH);

	StateIdV.Gen(StateSubStateH.Len());
	StateSetV.Gen(StateSubStateH.Len());

	int i = 0;
	int KeyId = StateSubStateH.FFirstKeyId();
	while (StateSubStateH.FNextKeyId(KeyId)) {
		const int StateIdx = StateSubStateH.GetKey(KeyId);

		if (StateSubStateH[KeyId].Empty()) {
			StateSetV[i].Add(StateIdx);
		} else {
			StateSetV[i] = StateSubStateH[KeyId];
		}

		StateIdV[i] = StateIdx;

		i++;
	}
}

void THierarch::GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const {
	const int NStates = GetStates();

	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
		if (IsOnHeight(StateIdx, Height)) {
			StateIdV.AddKey(StateIdx);
		}
	}
}

void THierarch::GetAncestorV(const int& StateId, TIntFltPrV& StateIdHeightPrV) const {
	StateIdHeightPrV.Add(TIntFltPr(StateId, GetStateHeight(StateId)));

	int AncestorId = StateId;
	do {
		AncestorId = GetParentId(AncestorId);
		StateIdHeightPrV.Add(TIntFltPr(AncestorId, GetStateHeight(AncestorId)));
	} while (!IsRoot(AncestorId));
}

int THierarch::GetAncestorAtHeight(const int& StateId, const double& Height) const {
	EAssertR(Height <= MxHeight, "Cannot search for states at height larger than MxHeight!");
	EAssert(IsOnHeight(StateId, Height) || IsBelowHeight(StateId, Height));

	int AncestorId = StateId;

	while (!IsOnHeight(AncestorId, Height)) {
		AncestorId = GetParentId(AncestorId);
	}

	return AncestorId;
}

void THierarch::GetLeafDescendantV(const int& TargetStateId, TIntV& DescendantV) const {
	if (IsLeaf(TargetStateId)) {
		DescendantV.Add(TargetStateId);
		return;
	}

	const int NStates = HierarchV.Len();

	TIntV TempHierarchV(HierarchV);

	// for each state compute the oldest ancestor until you hit the target state or root
	bool Change;
	do {
		Change = false;

		for (int LeafId = 0; LeafId < NStates; LeafId++) {
			if (GetParentId(LeafId, TempHierarchV) != TargetStateId && !IsRoot(GetParentId(LeafId, TempHierarchV), TempHierarchV)) {
				GetParentId(LeafId, TempHierarchV) = GetGrandparentId(LeafId, TempHierarchV);
				Change = true;
			}
		}
	} while (Change);

	// take only the leafs with the target ancestor
	for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
		if (GetParentId(LeafId, TempHierarchV) == TargetStateId) {
			DescendantV.Add(LeafId);
		}
	}
}

void THierarch::GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	const TFltV& HeightV = GetUniqueHeightV();
	for (int i = 0; i < PastStateIdV.Len(); i++) {
		const TIntV& PastStateIdVOnH = PastStateIdV[i];
		EAssertR(!PastStateIdVOnH.Empty(), "Past state cache empty!");
		StateIdHeightPrV.Add(TIntFltPr(PastStateIdVOnH[0], HeightV[i]));
	}
}

void THierarch::GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const {
	const int NearestHeightIdx = GetNearestHeightIdx(Height);
	const TIntV& HistV = PastStateIdV[NearestHeightIdx];
	for (int i = 1; i < HistV.Len(); i++) {
		StateIdV.Add(HistV[i]);
	}
}

bool THierarch::IsStateNm(const int& StateId) const {
	return 0 <= StateId && StateId < HierarchV.Len() && !StateNmV[StateId].Empty();
}

void THierarch::SetStateNm(const int& StateId, const TStr& StateNm) {
	EAssertR(0 <= StateId && StateId < StateNmV.Len(), "THierarch::SetStateNm: Invalid state ID!");
	StateNmV[StateId] = StateNm;
}

const TStr& THierarch::GetStateNm(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < StateNmV.Len(), "THierarch::GetStateNm: Invalid state ID!");
	return StateNmV[StateId];
}

bool THierarch::IsTarget(const int& StateId, const double& Height) const {
	EAssertR(IsOnHeight(StateId, Height), "State " + TInt(StateId).GetStr() + " is not on height " + TFlt(Height).GetStr());
	double StateHeight = GetNearestHeight(Height);
	return TargetIdHeightSet.IsKey(TIntFltPr(StateId, StateHeight));
}

void THierarch::SetTarget(const int& StateId, const double& Height) {
	EAssertR(IsOnHeight(StateId, Height), "State " + TInt(StateId).GetStr() + " is not on height " + TFlt(Height).GetStr());
	double StateHeight = GetNearestHeight(Height);
	TargetIdHeightSet.AddKey(TIntFltPr(StateId, StateHeight));
}

void THierarch::RemoveTarget(const int& StateId, const double& Height) {
	EAssertR(IsOnHeight(StateId, Height), "State " + TInt(StateId).GetStr() + " is not on height " + TFlt(Height).GetStr());
	EAssertR(IsTarget(StateId, Height), "State " + TInt(StateId).GetStr() + " is not a target on height " + TFlt(Height).GetStr());

	double StateHeight = GetNearestHeight(Height);
	TIntFltPr StateIdHeightPr(StateId, StateHeight);

	TargetIdHeightSet.DelKey(StateIdHeightPr);
}

void THierarch::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

void THierarch::PrintHierarch() const {
	TChA ChA = "";

	for (int i = 0; i < HierarchV.Len(); i++) {
		ChA += "(" + TInt(i).GetStr() + "," + TInt::GetStr(GetParentId(i)) + "," + TFlt::GetStr(GetStateHeight(i), "%.3f") + ")";
		if (i < HierarchV.Len()-1) { ChA += ","; }
	}

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Hierarchy: %s", ChA.CStr());
}

int THierarch::GetParentId(const int& StateId) const {
	return GetParentId(StateId, HierarchV);
}

int THierarch::GetNearestHeightIdx(const double& Height) const {
	// TODO optimize:
	// 1) use binary search to find the nearest height
	const TFltV& HeightV = GetUniqueHeightV();
	for (int i = 0; i < HeightV.Len() - 1; i++) {
		if (HeightV[i] <= Height && HeightV[i+1] > Height) {
			return i;
		}
	}
	return HeightV.Len() - 1;
}

double THierarch::GetNearestHeight(const double& InHeight) const {
	return UniqueHeightV[GetNearestHeightIdx(InHeight)];
}

bool THierarch::IsRoot(const int& StateId) const {
	return IsRoot(StateId, HierarchV);
}

bool THierarch::IsLeaf(const int& StateId) const {
	return StateId < NLeafs;
}

bool THierarch::IsOnHeight(const int& StateId, const double& Height) const {
	if (IsRoot(StateId) && Height >= MxHeight) { return true; }

	const double StateHeight = GetStateHeight(StateId);
	const double ParentHeight = GetStateHeight(GetParentId(StateId));

	return StateHeight <= Height && Height < ParentHeight;
}

bool THierarch::IsBelowHeight(const int& StateId, const double& Height) const {
	if (IsOnHeight(StateId, Height)) { return false; }
	return GetStateHeight(GetParentId(StateId)) <= Height;
}

bool THierarch::IsAboveHeight(const int& StateId, const double& Height) const {
	return !IsOnHeight(StateId, Height) && !IsBelowHeight(StateId, Height);
}

void THierarch::GetAncSuccH(const double& Height, TIntIntVH& StateSubStateH) const {
	const int NStates = GetStates();

	// build a model on this height
	// get all the states on this height
	TIntSet StateSet;	GetStatesAtHeight(Height, StateSet);

	int KeyId = StateSet.FFirstKeyId();
	while (StateSet.FNextKeyId(KeyId)) {
		StateSubStateH.AddDat(StateSet.GetKey(KeyId), TIntV());
	}

	// get all the substates of the states on this height
	TIntV TempHierarchV(HierarchV);
	bool Change;
	do {
		Change = false;
		for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
			if (!StateSet.IsKey(TempHierarchV[StateIdx]) &&
					TempHierarchV[StateIdx] != TempHierarchV[TempHierarchV[StateIdx]]) {
				TempHierarchV[StateIdx] = TempHierarchV[TempHierarchV[StateIdx]];
				Change = true;
			}
		}
	} while (Change);

	for (int StateIdx = 0; StateIdx < NLeafs; StateIdx++) {
		if (StateSet.IsKey(TempHierarchV[StateIdx])) {
			StateSubStateH.GetDat(TempHierarchV[StateIdx]).Add(StateIdx);
		}
	}
}

int THierarch::GetOldestAncestIdx(const int& StateIdx) const {
	int AncestIdx = StateIdx;

	while (HierarchV[AncestIdx] != -1) {
		AncestIdx = HierarchV[AncestIdx];
	}

	return AncestIdx;
}

void THierarch::GetLeafSuccesorCountV(TIntV& LeafCountV) const {
	const int NStates = GetStates();

	LeafCountV.Gen(NStates, NStates);

	for (int i = 0; i < NLeafs; i++) {
		LeafCountV[i] = 1;
	}

	TIntV TempHierarchV(HierarchV);

	bool Change;
	do {
		Change = false;

		for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
			const int AncestorId = GetParentId(LeafId, TempHierarchV);
			const int LeafWeight = LeafCountV[LeafId];
			LeafCountV[AncestorId] += LeafWeight;
		}

		for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
			// check if the parent is root
			if (!IsRoot(GetParentId(LeafId, TempHierarchV), TempHierarchV)) {
				GetParentId(LeafId, TempHierarchV) = GetGrandparentId(LeafId, TempHierarchV);
				Change = true;
			}
		}
	} while (Change);
}

void THierarch::ComputeStateCoords(const TFullMatrix& CentroidMat, const int& NStates) {
	StateCoordV.Gen(NStates, NStates);

	TFullMatrix CoordMat = TEuclMds::Project(CentroidMat, 2);
	for (int ColIdx = 0; ColIdx < CoordMat.GetCols(); ColIdx++) {
		StateCoordV[ColIdx].Val1 = CoordMat(0, ColIdx);
		StateCoordV[ColIdx].Val2 = CoordMat(1, ColIdx);
	}

	// first find out how many ancestors each state has, so you can weight
	// the childs coordinates appropriately
	TIntV SuccesorCountV;	GetLeafSuccesorCountV(SuccesorCountV);

	TIntV TempHierarchV(HierarchV);

	bool Change;
	do {
		Change = false;

		for (int i = 0; i < NLeafs; i++) {
			const int AncestorIdx = TempHierarchV[i];
			const int AncestorSize = SuccesorCountV[AncestorIdx];
			StateCoordV[AncestorIdx].Val1 += StateCoordV[i].Val1 / AncestorSize;
			StateCoordV[AncestorIdx].Val2 += StateCoordV[i].Val2 / AncestorSize;
		}

		for (int i = 0; i < NLeafs; i++) {
			if (TempHierarchV[i] != TempHierarchV[TempHierarchV[i]]) {
				TempHierarchV[i] = TempHierarchV[TempHierarchV[i]];
				Change = true;
			}
		}
	} while (Change);
}

bool THierarch::IsRoot(const int& StateId, const TIntV& HierarchV) {
	return GetParentId(StateId, HierarchV) == StateId;
}

void THierarch::GenUniqueHeightV(const TFltV& HeightV, TFltV& UniqueHeightV) {
	TFltSet UsedHeightSet;
	for (int i = 0; i < HeightV.Len(); i++) {
		if (!UsedHeightSet.IsKey(HeightV[i])) {
			UniqueHeightV.Add(HeightV[i]);
			UsedHeightSet.AddKey(HeightV[i]);
		}
	}

	UniqueHeightV.Sort(true);	// sort ascending
}

void THierarch::ClrFlds() {
	HierarchV.Clr();
	StateHeightV.Clr();
	UniqueHeightV.Clr();
	MxHeight = TFlt::Mn;
	HistCacheSize = 1;
	PastStateIdV.Clr();
	StateCoordV.Clr();
	NLeafs = 0;
	StateNmV.Clr();
	TargetIdHeightSet.Clr();
}

/////////////////////////////////////////////////////////////////
// Abstract Markov Chain
TMChain::TMChain(const bool& _Verbose):
		NStates(-1),
		CurrStateId(-1),
		TmHorizon(1),
		PredictionThreshold(.5),
		PdfBins(100),
		HasHiddenState(false),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TMChain::TMChain(TSIn& SIn):
		NStates(TInt(SIn)),
		CurrStateId(TInt(SIn)),
		TmHorizon(TFlt(SIn)),
		PredictionThreshold(TFlt(SIn)),
		PdfBins(TInt(SIn)),
		HasHiddenState(TBool(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	PdfBins = 100;
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}


void TMChain::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	TInt(NStates).Save(SOut);
	TInt(CurrStateId).Save(SOut);
	TFlt(TmHorizon).Save(SOut);
	TFlt(PredictionThreshold).Save(SOut);
	TInt(PdfBins).Save(SOut);
	TBool(HasHiddenState).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PMChain TMChain::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "continuous") {
		return new TCtMChain(SIn);
	} else {
		throw TExcept::New("Invalid type of Markov chain: " + Type, "TMChain::Load");
	}
}

void TMChain::Init(const TFltVV& FtrVV, const int& _NStates, const TIntV& AssignV,
		const TUInt64V& TmV, const bool _HasHiddenState, const TBoolV& EndBatchV) {
	NStates = _NStates;
	HasHiddenState = _HasHiddenState;

	Notify->OnNotify(TNotifyType::ntInfo, "Initializing Markov chain ...");
	InitIntensities(FtrVV, TmV, AssignV, EndBatchV);
	Notify->OnNotify(TNotifyType::ntInfo, "Done!");

	Notify->OnNotify(TNotifyType::ntInfo, "Running the examples through the Markov chain ...");

	const int NInst = FtrVV.GetCols();
	if (EndBatchV.Empty()) {
		for (int i = 0; i < NInst; i++) {
			OnAddRec(AssignV[i], TmV[i], false);
		}
	} else {
		for (int i = 0; i < NInst; i++) {
			OnAddRec(AssignV[i], TmV[i], EndBatchV[i]);
		}
	}


	Notify->OnNotify(TNotifyType::ntInfo, "Markov chain initialization complete!");
}

void TMChain::OnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot be last in sequence if a hidden state does not exist!");

	// call child method
	AbsOnAddRec(StateId, RecTm, EndsBatch);
	if (HasHiddenState && EndsBatch) {
		CurrStateId = GetHiddenStateId();
	} else {
		CurrStateId = StateId;
	}
}

void TMChain::GetFutureProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TVector ProbV = GetFutureProbMat(StateSetV, StateFtrVV, Tm).GetRow(StateIdx);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TMChain::GetPastProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TVector ProbV = GetPastProbMat(StateSetV, StateFtrVV, Tm).GetRow(StateIdx);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TMChain::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

int TMChain::GetHiddenStateId() const {
	return HasHiddenState ? GetStates() : -2;
}

void TMChain::InsHiddenState(TStateSetV& StateSetV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");

	StateSetV.Add(TIntV(1, 1));
	StateSetV.Last()[0] = GetHiddenStateId();
}

void TMChain::InsHiddenState(TStateIdV& StateIdV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");
	StateIdV.Add(GetHiddenStateId());
}

void TMChain::RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const {
	EAssertR(HasHiddenState, "TMChain::RemoveHiddenStateProb: The model does not have a hidden state!");

	const int HiddenStateId = GetHiddenStateId();

	for (int i = 0; i < StateIdProbV.Len(); i++) {
		if (StateIdProbV[i].Val1 == HiddenStateId) {
			StateIdProbV.Del(i);
			break;
		}
	}
}

void TMChain::GetFutureProbVOverTm(const TFullMatrix& PMat, const int& StateIdx,
		const int& Steps, TVec<TFltV>& ProbVV, const PNotify& Notify, const bool IncludeT0) {

	TFullMatrix CurrentMat = TFullMatrix::Identity(PMat.GetRows());
	TVector ProbV = CurrentMat.GetRow(StateIdx);

	if (IncludeT0) {
		ProbVV.Add(ProbV.Vec);
	}

	for (int i = 0; i < Steps; i++) {
		if (i % 100 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "steps: %d", i);
		}
		// increase time
		CurrentMat = CurrentMat * PMat;
		ProbV = CurrentMat.GetRow(StateIdx);
		// normalize to minimize the error
		ProbV /= ProbV.Sum();
		// add to result
		ProbVV.Add(ProbV.Vec);
	}
}

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
TCtMChain::TCtMChain(const uint64& _TimeUnit, const double& _DeltaTm, const bool& _Verbose):
		TMChain(_Verbose),
		IntensModelMat(),
		HiddenStateJumpCountV(),
		DeltaTm(_DeltaTm),
		TimeUnit(_TimeUnit),
		PrevJumpTm(-1) {}

TCtMChain::TCtMChain(TSIn& SIn):
		TMChain(SIn),
		IntensModelMat(SIn),
		HiddenStateJumpCountV(SIn),
		DeltaTm(TFlt(SIn)),
		TimeUnit(TUInt64(SIn)),
		PrevJumpTm(TUInt64(SIn)) {
}

void TCtMChain::Save(TSOut& SOut) const {
	TMChain::Save(SOut);
	IntensModelMat.Save(SOut);
	HiddenStateJumpCountV.Save(SOut);
	TFlt(DeltaTm).Save(SOut);
	TUInt64(TimeUnit).Save(SOut);
	TUInt64(PrevJumpTm).Save(SOut);
}

void TCtMChain::GetNextStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, const TStateIdV& ExtStateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const {

	TStateIdV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	GetNextStateProbV(GetQMatrix(StateSetV, StateFtrVV), StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtMChain::GetPrevStateProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& ExtStateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
		const int& NFutStates) const {

	TStateIdV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	GetNextStateProbV(GetRevQMatrix(StateSetV, StateFtrVV), StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtMChain::GetProbVOverTm(const double& Height, const int& StateId,
		const double& StartTm, const double EndTm, const double& DeltaTm,
		const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, TVec<TFltV>& FutProbVV, TVec<TFltV>& PastProbVV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "Could not find target state!");
	EAssertR(StartTm <= 0 && EndTm >= 0, "The start and end times should include the current time!");

	const int FutureSteps = (int)ceil(EndTm / DeltaTm);
	const TFullMatrix FutProbMat = GetFutureProbMat(StateSetV, StateFtrVV, DeltaTm);

	GetFutureProbVOverTm(FutProbMat, StateIdx, FutureSteps, FutProbVV, Notify);

	if (StartTm < 0) {
		const int PastSteps = (int)ceil(-StartTm / DeltaTm);
		const TFullMatrix PastProbMat = GetPastProbMat(StateSetV, StateFtrVV, DeltaTm);
		GetFutureProbVOverTm(PastProbMat, StateIdx, PastSteps, PastProbVV, Notify, false);
	}
}

bool TCtMChain::PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TStateSetV& StateSetV,
		const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
		double& Prob, TFltV& ProbV, TFltV& TmV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Predicting occurrence time from state %d to state %d ...", CurrStateId, TargetStateId);

	const TFullMatrix QMatrix = GetQMatrix(StateSetV, StateFtrVV);

	// find the index of the current and target states on this height
	const int CurrStateIdx = StateIdV.SearchForw(CurrStateId);
	const int TargetStateIdx = StateIdV.SearchForw(TargetStateId);

	EAssertR(CurrStateIdx >= 0, "Could not find the start state!");
	EAssertR(TargetStateIdx >= 0, "Could not find the start state!");

	Prob = PredictOccurenceTime(QMatrix, CurrStateIdx, TargetStateIdx,
			DeltaTm, TmHorizon, TmV, ProbV);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Will reach state %d from state %d with prob: %.4f in time %.4f", CurrStateId, TargetStateId, Prob, TmHorizon);

	if (Prob < PredictionThreshold) { return false; }

	if (PdfBins > 0) {
		// reduce the data sent to the outside world by intepolating the PDF
		const int SamplingRate = ProbV.Len() / PdfBins;
		TFltV NewProbV(PdfBins, 0), NewTmV(PdfBins, 0);

		for (int i = 0; i < PdfBins; i++) {
			NewProbV.Add(ProbV[i*SamplingRate]);
			NewTmV.Add(TmV[i*SamplingRate]);
		}

		ProbV = NewProbV;
		TmV = NewTmV;
	}

	return true;
}

void TCtMChain::GetStatDist(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		TFltV& StatDist) const {
	TFltV AllStatDist;	GetStatDist(GetQMatrix(StateFtrVV), AllStatDist, Notify);
	if (HasHiddenState) {
		AllStatDist.DelLast();
		TLinAlg::MultiplyScalar(1 / TLinAlg::SumVec(AllStatDist), AllStatDist);
	}
	StatDist.Gen(StateSetV.Len());
	for (int StateSetN = 0; StateSetN < StateSetV.Len(); StateSetN++) {
		const TIntV& StateSet = StateSetV[StateSetN];
		double Sum = 0;
		for (int StateN = 0; StateN < StateSet.Len(); StateN++) {
			const int& StateId = StateSet[StateN];
			Sum += AllStatDist[StateId];
		}
		StatDist[StateSetN] = Sum;
	}
//	GetStatDist(GetQMatrix(StateSetV, StateFtrVV), StatDist, Notify);
//	if (HasHiddenState) {
//		StatDist.DelLast();
//		TLinAlg::MultiplyScalar(1 / TLinAlg::SumVec(StatDist), StatDist);
//	}
}

void TCtMChain::GetStateSizeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, TFltV& StateSizeV) const {
//	return GetHoldingTimeV(GetQMatrix(JoinedStateVV));

	GetStatDist(StateSetV, StateFtrVV, StateSizeV);
}

TFullMatrix TCtMChain::GetTransitionMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const {
	return GetJumpMatrix(StateSetV, StateFtrVV);
}

TFullMatrix TCtMChain::GetJumpMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const {
	TFullMatrix JumpMat = GetJumpMatrix(GetQMatrix(StateSetV, StateFtrVV));
	if (HasHiddenState) {
		// take the jump matrix and remove the last row and column
		// don't normalize the rows, so the person can see which are the end states
		JumpMat = JumpMat(TVector::Range(JumpMat.GetRows()-1), TVector::Range(JumpMat.GetCols()-1));
		return JumpMat;
	} else {
		return JumpMat;
	}
}

void TCtMChain::GetHoldingTimeV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV, TFltV& HoldingTmV) const {
	GetHoldingTimeV(GetQMatrix(StateSetV, StateFtrVV), HoldingTmV);
	if (HasHiddenState) {
		HoldingTmV.DelLast();
	}
}

bool TCtMChain::IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const {
	TVector IntensV = GetStateIntensV(OldStateId, FtrV);

	return IntensV[NewStateId] / (-IntensV[OldStateId]) < 1e-3;
}

void TCtMChain::AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot process batches with no hidden state!");

	// warn if times don't aren't ascending
	if (CurrStateId != -1 && RecTm < PrevJumpTm && (!HasHiddenState || CurrStateId != GetHiddenStateId())) {	// got past time, do not update the statistics
		TNotify::StdNotify->OnNotifyFmt(TNotifyType::ntWarn, "Current time smaller that previous time curr: %ld, prev: %ld", RecTm, PrevJumpTm);
		PrevJumpTm = RecTm;
		return;
	}
}

TFullMatrix TCtMChain::GetFutureProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const double& Tm) const {
	return GetFutureProbMat(GetQMatrix(StateSetV, StateFtrVV), Tm, DeltaTm, HasHiddenState);
}

TFullMatrix TCtMChain::GetPastProbMat(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
		const double& Tm) const {
	return GetFutureProbMat(GetRevQMatrix(StateSetV, StateFtrVV), Tm, DeltaTm, HasHiddenState);
}

void TCtMChain::InitIntensities(const TFltVV& FtrVV, const TUInt64V& TmV,
		const TIntV& AssignV, const TBoolV& EndBatchV) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Modeling intensities ...");

	// TODO handle hidden states

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();
	const int NStates = GetStates() + (HasHiddenState ? 1 : 0);

	TLabelVMat LabelVMat;			// stores the class labels
	TJumpFtrVVMat JumpFtrVVMat(NStates, NStates);	// used when constructing feature vectors
	TJumpFtrMatMat JumpFtrMatVV(NStates, NStates);	// stores the feature vectors

	IntensModelMat.Gen(NStates, NStates);			// used to store the intensity models
	TBoolVV HasJumpedV(NStates, NStates);
	// I need to keep track of a NxN matrix of Poisson processes
//	TUInt64VV PoisWaitTmVV(NStates, NStates);
//	TIntVV PoisLastJumpTmNVV(NStates, NStates);

	for (int i = 0; i < NStates; i++) {
		LabelVMat.Add(TVec<TLabelV>());
		for (int j = 0; j < NStates; j++) {
			LabelVMat[i].Add(TLabelV());
		}
	}

	double MeanSampleInterval = 0;
	for (int CurrTmN = 0; CurrTmN < NInst-1; CurrTmN++) {
		const int CurrStateId = AssignV[CurrTmN];
		const int NextStateId = AssignV[CurrTmN+1];

		const uint64 DeltaTm = TmV[CurrTmN+1] - TmV[CurrTmN];
		MeanSampleInterval += double(DeltaTm) / TimeUnit;

		double DidJump = NextStateId != CurrStateId ? 1 : 0;	// Bernoulli class
		TFltV FtrV;	FtrVV.GetCol(CurrTmN, FtrV);				// feature vector

		for (int JumpStateId = 0; JumpStateId < NStates; JumpStateId++) {
			JumpFtrVVMat(CurrStateId, JumpStateId).Add(FtrV);
			LabelVMat[CurrStateId][JumpStateId].Add(JumpStateId == NextStateId ? DidJump : 0);
		}

		if (NextStateId != CurrStateId) {
			HasJumpedV(CurrStateId, NextStateId) = true;
		}
	}

	MeanSampleInterval /= (NInst - 1);

	// construct feature matrices
	for (int State1Id = 0; State1Id < NStates; State1Id++) {
		if (HasHiddenState && State1Id == GetHiddenStateId()) { continue; }

		for (int State2Id = 0; State2Id < NStates; State2Id++) {
			const TFtrVV& JumpFtrVV = JumpFtrVVMat(State1Id, State2Id);

			const int NRows = Dim;
			const int NCols = JumpFtrVV.Len();

			JumpFtrMatVV.PutXY(State1Id, State2Id, TJumpFtrMat(FtrVV.GetRows(), JumpFtrVV.Len()));

			if (NCols == 0) { continue; }

			TJumpFtrMat& JumpFtrMat = JumpFtrMatVV(State1Id, State2Id);
			for (int RowN = 0; RowN < NRows; RowN++) {
				for (int ColN = 0; ColN < NCols; ColN++) {
					JumpFtrMat(RowN, ColN) = JumpFtrVV[ColN][RowN];
				}
			}
		}
	}

	// fit the regression models
	for (int State1Id = 0; State1Id < NStates; State1Id++) {
		if (HasHiddenState && State1Id == GetHiddenStateId()) { continue; }

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Regressing intensities for state %d", State1Id);

		for (int State2Id = 0; State2Id < NStates; State2Id++) {
			IntensModelMat.PutXY(State1Id, State2Id, TIntensModel(MeanSampleInterval, 1e-3, HasJumpedV(State1Id, State2Id), true));

			if (State1Id == State2Id) { continue; }

			TIntensModel& Model = IntensModelMat(State1Id, State2Id);

			const TJumpFtrMat& JumpFtrVV = JumpFtrMatVV(State1Id, State2Id);
			const TLabelV& LabelV = LabelVMat[State1Id][State2Id];

			if (LabelV.Empty() || TLinAlg::IsZero(LabelV)) {
				continue;
			}

			//============================================================
			// TODO delete me
			int JumpN = 0;
			for (int i = 0; i < LabelV.Len(); i++) {
				const double Label = LabelV[i];
				if (Label > 0) {
					JumpN++;
				}
			}
			EAssertR(JumpN > 0, "WTF!? How did a zero vector get here???");
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d jumps from state %d to %d ...", JumpN, State1Id, State2Id);
			//============================================================

			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Fitting a regression model from state %d to %d", State1Id, State2Id);
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Class labels:\n%s", TStrUtil::GetStr(LabelV, ", ", "%.1f").CStr());

			Model.Fit(JumpFtrVV, LabelV);
		}
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Done!");
}

TFullMatrix TCtMChain::GetQMatrix(const TStateFtrVV& StateFtrVV) const {
	// compute the intensities
	const int NStates = IntensModelMat.GetRows();

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Constructing Q matrix for %d states ...", NStates);

	// Q-matrix: holds jump intensities
	TFullMatrix QMatrix(NStates, NStates);
	for (int RowN = 0; RowN < NStates; RowN++) {
		if (IsHiddenStateId(RowN)) { continue; }

		for (int ColN = 0; ColN < NStates; ColN++) {
			if (ColN != RowN) {
				const double Intens = IntensModelMat(RowN, ColN).Predict(StateFtrVV[RowN]);
				EAssertR(Intens >= 0, "Intensity is less than 0!!!");
				QMatrix(RowN, ColN) = Intens > 10000 ? 10000 : Intens;	// TODO fix
			}
		}

		//==========================================================
		// TODO remove
		for (int ColN = 0; ColN < QMatrix.GetCols(); ColN++) {
			printf("%.15f", QMatrix(RowN, ColN).Val);
			if (ColN < QMatrix.GetCols()-1) {
				printf(", ");
			}
		}
		printf("\n");
		//==========================================================

		const double Q_ii = -QMatrix.RowSum(RowN);
		EAssertR(Q_ii != 0, "Q_ii has a zero row!");
		QMatrix(RowN,RowN) = Q_ii;
	}

	if (HasHiddenState) {
		const int HiddenStateId = GetHiddenStateId();

		for (int ColN = 0; ColN < NStates; ColN++) {
			if (IsHiddenStateId(ColN)) { continue; }

			const double Intens = HIDDEN_STATE_INTENSITY * HiddenStateJumpCountV[ColN];
			QMatrix(HiddenStateId, ColN) = Intens;
		}

		const double Q_ii = -QMatrix.RowSum(HiddenStateId);
		EAssertR(Q_ii != 0, "Q_ii has a zero row!");
		QMatrix(HiddenStateId,HiddenStateId) = Q_ii;
	}

	return QMatrix;
}

TVector TCtMChain::GetStateIntensV(const int StateId, const TFltV& FtrV) const {
	const int NStates = IntensModelMat.GetRows();

	TVector IntensV(NStates);

	for (int ColN = 0; ColN < NStates; ColN++) {
		if (ColN != StateId) {
			const double Intens = IntensModelMat(StateId, ColN).Predict(FtrV);
			EAssertR(Intens >= 0, "Intensity is less than 0!!!");
			IntensV[ColN] = Intens;
		}
	}

	IntensV[StateId] = -IntensV.Sum();
	return IntensV;
}

TFullMatrix TCtMChain::GetQMatrix(const TStateSetV& InStateSetV, const TStateFtrVV& StateFtrVV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing joined Q matrix for %d states ...", InStateSetV.Len());

	TStateSetV StateSetV(InStateSetV);

	if (HasHiddenState) {
		InsHiddenState(StateSetV);
	}

	const int NStates = StateSetV.Len();

	TFullMatrix JoinedQMat(NStates, NStates);

	const TFullMatrix QMat = GetQMatrix(StateFtrVV);
	TFltV StatDist;	GetStatDist(QMat, StatDist, Notify);

	for (int JoinState1Idx = 0; JoinState1Idx < NStates; JoinState1Idx++) {
		const TIntV& JoinState1 = StateSetV[JoinState1Idx];
		for (int JoinState2Idx = 0; JoinState2Idx < NStates; JoinState2Idx++) {
			if (JoinState1Idx == JoinState2Idx) { continue; }

			const TIntV& JoinState2 = StateSetV[JoinState2Idx];

			// the transition probability from set Ai to Aj can be
			// calculated as: q_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} q_{k,l}} {\sum_{k \in A_i} \pi_k}

			double Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];
				const double PiK = StatDist[JoinState1[k]];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					const double Q_kl = QMat(StateK,StateL);
					SumK += Q_kl;
				}

				Sum += PiK*SumK;
				SumP += PiK;
			}

			JoinedQMat(JoinState1Idx, JoinState2Idx) = Sum / SumP;
		}

		const double Q_ii = -JoinedQMat.RowSum(JoinState1Idx);
		EAssertR(Q_ii != 0, "Joined QMatrix has zero on diagonal!");
		JoinedQMat(JoinState1Idx, JoinState1Idx) = Q_ii;
	}

	return JoinedQMat;
}

TFullMatrix TCtMChain::GetRevQMatrix(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV) const {
	const int n = StateSetV.Len();
	const TFullMatrix QMat = GetQMatrix(StateSetV, StateFtrVV);
	TFltV StatDist;	GetStatDist(QMat, StatDist, Notify);

	TFullMatrix QRev(n,n);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			QRev(j,i) = QMat(i,j) * StatDist[i] / StatDist[j];
		}
	}

	return QRev;
}

void TCtMChain::GetHoldingTimeV(const TFullMatrix& QMat, TFltV& HoldingTmV) const {
	const int Rows = QMat.GetRows();

	HoldingTmV.Gen(Rows, Rows);

	double Intens;
	for (int i = 0; i < Rows; i++) {
		Intens = -QMat(i,i);
		HoldingTmV[i] = 1 / Intens;
	}
}

void TCtMChain::GetNextStateProbV(const TFullMatrix& QMat, const TStateIdV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
		const PNotify& Notify) {

	const int Dim = QMat.GetRows();

	const int NFStates = TMath::Mn(NFutStates, Dim-1);
	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TCtMChain::GetNextStateProbV: Could not find target state!");

	Notify->OnNotify(TNotifyType::ntInfo, "Fetching future states ...");

	const TFullMatrix JumpMat = GetJumpMatrix(QMat);
	const TVector ProbVec = JumpMat.GetRow(StateIdx);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbVec.Len(); j++) {
			if (j != StateIdx && !TakenIdxSet.IsKey(j) && ProbVec[j] > MxProb) {
				MxProb = ProbVec[j];
				MxIdx = j;
			}
		}

		if (ProbVec[MxIdx] <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		StateIdProbV.Add(TIntFltPr(StateIdV[MxIdx], MxProb));
	}
}

void TCtMChain::GetStatDist(const TFullMatrix& QMat, TFltV& ProbV, const PNotify& Notify) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing static distribution of %d states ...", QMat.GetRows());
	const int Dim = QMat.GetRows();
	// returns the stationary distribution
	// pi*Q = 0
//	TVector EigenVec(QMat.GetRows(), false);

	TNumericalStuff::GetEigenVec(QMat.GetT().GetMat(), 0.0, ProbV);

	const double EigSum = TLinAlg::SumVec(ProbV);
//	const double EigSum = EigenVec.Sum();

	EAssertR(EigSum != 0, "Eigenvector should not be 0, norm is " + TFlt::GetStr(TLinAlg::Norm(ProbV)) + "!");
	EAssertR(!TFlt::IsNan(EigSum), "NaNs in eigenvector!");

	//===========================================================
	// TODO remove this assertion after you know this works
	// check if the result is correct
	TFltV PiTimesQ;
	TLinAlg::MultiplyT(QMat.GetMat(), ProbV, PiTimesQ);
	const double PiQNorm = TLinAlg::Norm(PiTimesQ);
	EAssertR(PiQNorm < 1e-3, "This is not an eigenvector with eigenvalue 0");
	//===========================================================

	// normalize to get a distribution
	for (int i = 0; i < Dim; i++) {
		ProbV[i] /= EigSum;
	}
//	return EigenVec /= EigSum;
}

TFullMatrix TCtMChain::GetProbMat(const TFullMatrix& QMat, const double& Dt) {
	return TFullMatrix::Identity(QMat.GetRows()) + QMat*Dt;
}

TFullMatrix TCtMChain::GetFutureProbMat(const TFullMatrix& QMat, const double& Tm,
		const double& DeltaTm, const bool HasHiddenState) {
	EAssertR(Tm >= 0, "TCtMChain::GetFutureProbMat: does not work for negative time!");

	const int Dim = QMat.GetRows();

	if (Tm == 0) { return TFullMatrix::Identity(Dim); }

	const double QMatNorm = QMat.FromNorm();
	const double Dt = TMath::Mn(DeltaTm / QMatNorm, DeltaTm);

	const int Steps = (int) ceil(Tm / Dt);

	TFullMatrix ProbMat = GetProbMat(QMat, Dt);

	// the probabilities from state i to the hidden state should now go from i to i
	if (HasHiddenState) {
		const int Dim = ProbMat.GetRows()-1;

		TFullMatrix CorrProbMat = ProbMat(TVector::Range(Dim), TVector::Range(Dim));
		for (int RowIdx = 0; RowIdx < Dim; RowIdx++) {
			const double HiddenProb = ProbMat(RowIdx, Dim);
			CorrProbMat(RowIdx, RowIdx) += HiddenProb;
		}

		ProbMat = CorrProbMat;
	}

	return ProbMat^Steps;
}

double TCtMChain::PredictOccurenceTime(const TFullMatrix& QMat, const int& CurrStateIdx,
			const int& TargetStateIdx, const double& DeltaTm, const double& HorizonTm,
			TFltV& TmV, TFltV& HitProbV) {

	const int Dim = QMat.GetRows();
	const int OutputSize = (int)ceil(HorizonTm / DeltaTm) + 1;

	const TFullMatrix PMat = GetProbMat(QMat, DeltaTm);

//	TFullMatrix CurrProbMat = TFullMatrix::Identity(Dim);
	TFltVV CurrProbMat(Dim, Dim);
	TFltVV TempCurrProbMat(Dim, Dim);

	// initialize return structures
	double CumHitProb = 0;
	TmV.Gen(OutputSize, OutputSize);
	HitProbV.Gen(OutputSize, OutputSize);
	TFltV ReturnProbV(OutputSize, OutputSize);

	TmV[0] = 0;
	HitProbV[0] = TargetStateIdx == CurrStateIdx ? 1 : 0;
	ReturnProbV[0] = 1;

	if (TargetStateIdx == CurrStateIdx) {
		return 1;
	}

	// P(0) = I
	for (int i = 0; i < Dim; i++) {
		CurrProbMat(i, i) = 1;
	}

	double CumReturnProb, HitProb, ReturnProb, Prob;

	int n = 1;
	double CurrTm = DeltaTm;

	while (CurrTm < HorizonTm) {
		// P(nh) <- P((n-1)h)*P(h)
		TLinAlg::Multiply(CurrProbMat, PMat.GetMat(), TempCurrProbMat);
		std::swap(CurrProbMat, TempCurrProbMat);

		ReturnProb = CurrProbMat(TargetStateIdx, TargetStateIdx);
		EAssertR(!TFlt::IsNan(ReturnProb), "The return probability is nan!");

		Prob = CurrProbMat(CurrStateIdx, TargetStateIdx);
		ReturnProbV[n] = ReturnProb;

		CumReturnProb = 0;
		for (int TmN = 0; TmN < n; TmN++) {
			CumReturnProb += HitProbV[TmN]*ReturnProbV[n - TmN];
		}

		EAssertR(!TFlt::IsNan(Prob), "The probability of reachig the target state is nan!");

		HitProb = Prob - CumReturnProb*DeltaTm;
		EAssertR(!TFlt::IsNan(HitProb), "The HitProb is nan!");

		CumHitProb += HitProb;
		HitProbV[n] = HitProb;
		TmV[n] = CurrTm;

//		printf("Prob %.4f, CumReturnProb %.4f, HitProb: %.4f, CumHitProb %.4f\n", Prob, CumReturnProb*DeltaTm, HitProb, CumHitProb * DeltaTm);

		n++;
		CurrTm += DeltaTm;
	}

	return CumHitProb * DeltaTm;
}

TFullMatrix TCtMChain::GetJumpMatrix(const TFullMatrix& QMat) {
	const int Rows = QMat.GetRows();
	const int Cols = QMat.GetCols();

	TFullMatrix JumpMat(Rows, Cols);

	double Q_ij, Q_ii, J_ij;

	for (int i = 0; i < Rows; i++) {
		if (QMat(i,i) == 0.0) {
			JumpMat(i,i) = 1;
		} else {
			for (int j = 0; j < Cols; j++) {
				if (j != i) {
					Q_ij = QMat(i,j);
					Q_ii = -QMat(i,i);
					J_ij = Q_ij / Q_ii;

					EAssertR(!TFlt::IsNan(J_ij), "Jump matrix contains nan on indexes " + TInt::GetHexStr(i) +", " + TInt::GetStr(j));

					JumpMat(i,j) = J_ij;
				}
			}
		}
	}

	return JumpMat;
}

////////////////////////////////////////////////
// State assistant
TStateAssist::TStateAssist(const bool _Verbose):
		ClassifyV(),
		FtrBoundV(),
		Rnd(1),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TStateAssist::TStateAssist(TSIn& SIn):
		ClassifyV(SIn),
		FtrBoundV(SIn),
		Rnd(SIn),
		Verbose(TBool(SIn)) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStateAssist::Save(TSOut& SOut) const {
	ClassifyV.Save(SOut);
	FtrBoundV.Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TStateAssist::Init(TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const PStateIdentifier& Clust, const PHierarch& Hierarch) {
	InitFtrBounds(ObsFtrVV, ContrFtrVV);

	const TFullMatrix X(ObsFtrVV, true);	// TODO change this to TFltVV and add const in the params

	// get all the heights from the hierarchy
	TIntFltPrV StateIdHeightPrV;	Hierarch->GetStateIdHeightPrV(StateIdHeightPrV);
	TVector AssignV = Clust->Assign(X.GetMat());

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist, total states %d ...", StateIdHeightPrV.Len());

	for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
		const TIntFltPr& StateIdHeightPr = StateIdHeightPrV[i];
		const int StateId = StateIdHeightPr.Val1;
		const double Height = StateIdHeightPr.Val2;

		ClassifyV.Add(TLogReg(1, true));

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist for state %d ...", StateId);

		TStateIdV StateIdV; TStateSetV StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);

		const int StateIdx = StateIdV.SearchForw(StateId);

		EAssertR(StateIdx >= 0, "Could not find the target state!");

		const TIntSet TargetStateSet(StateSetV[StateIdx]);

		TIntV TargetIdxV;	AssignV.Find([&](const TFlt& StateId) { return TargetStateSet.IsKey(int(StateId)); }, TargetIdxV);
		TIntV NonTargetIdxV;	AssignV.Find([&](const TFlt& StateId) { return !TargetStateSet.IsKey(int(StateId)); }, NonTargetIdxV);

		if (TargetIdxV.Len() == 0 || NonTargetIdxV.Len() == 0) continue;

		// make the sets equally sized
		if (NonTargetIdxV.Len() > TargetIdxV.Len()) {
			NonTargetIdxV.Shuffle(Rnd);
			NonTargetIdxV.Trunc(TargetIdxV.Len());
		} else if (TargetIdxV.Len() > NonTargetIdxV.Len()) {
			TargetIdxV.Shuffle(Rnd);
			TargetIdxV.Trunc(NonTargetIdxV.Len());
		}

		// get the instances
		TFullMatrix PosInstMat = X(TVector::Range(X.GetRows()), TargetIdxV);
		TFullMatrix NegInstMat = X(TVector::Range(X.GetRows()), NonTargetIdxV);

		TFltVV InstanceMat(X.GetRows(), PosInstMat.GetCols() + NegInstMat.GetCols());
		TFltV y(PosInstMat.GetCols() + NegInstMat.GetCols(), PosInstMat.GetCols() + NegInstMat.GetCols());
		for (int ColIdx = 0; ColIdx < PosInstMat.GetCols(); ColIdx++) {
			for (int RowIdx = 0; RowIdx < X.GetRows(); RowIdx++) {
				InstanceMat(RowIdx, ColIdx) = PosInstMat(RowIdx, ColIdx);
			}
			y[ColIdx] = 1;
		}
		for (int ColIdx = 0; ColIdx < NegInstMat.GetCols(); ColIdx++) {
			for (int RowIdx = 0; RowIdx < X.GetRows(); RowIdx++) {
				InstanceMat(RowIdx, PosInstMat.GetCols() + ColIdx) = NegInstMat(RowIdx, ColIdx);
			}
			y[PosInstMat.GetCols() + ColIdx] = 0;
		}

		ClassifyV.Last().Fit(InstanceMat, y);
	}
}

void TStateAssist::InitFtrBounds(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV) {
	const int NObsFtrs = ObsFtrVV.GetRows();
	const int NContrFtrs = ContrFtrVV.GetRows();
	const int TotalFtrs = NObsFtrs + NContrFtrs;
	const int NInst = ObsFtrVV.GetCols();


	FtrBoundV.Gen(TotalFtrs);

	for (int FtrN = 0; FtrN < NObsFtrs; FtrN++) {
		double Min = TFlt::Mx, Max = TFlt::Mn;
		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ObsFtrVV(FtrN, InstN) < Min) { Min = ObsFtrVV(FtrN, InstN); }
			if (ObsFtrVV(FtrN, InstN) > Max) { Max = ObsFtrVV(FtrN, InstN); }
		}

		FtrBoundV[FtrN] = TFltPr(Min, Max);
	}

	for (int FtrN = 0; FtrN < NContrFtrs; FtrN++) {
		double Min = TFlt::Mx, Max = TFlt::Mn;
		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ContrFtrVV(FtrN, InstN) < Min) { Min = ContrFtrVV(FtrN, InstN); }
			if (ContrFtrVV(FtrN, InstN) > Max) { Max = ContrFtrVV(FtrN, InstN); }
		}

		FtrBoundV[NObsFtrs + FtrN] = TFltPr(Min, Max);
	}
}

void TStateAssist::GetSuggestFtrs(const int& StateId, TFltV& WgtV) const {
	EAssertR(0 <= StateId && StateId < ClassifyV.Len(), "Invalid state ID!");

	const TLogReg& Classify = ClassifyV[StateId];
	Classify.GetWgtV(WgtV);
}

/////////////////////////////////////////////////////////////////
// Hierarchical continous time Markov Chain
TStreamStory::TStreamStory():
		StateIdentifier(nullptr),
		MChain(nullptr),
		Hierarch(nullptr),
		StateAssist(nullptr),
//		FtrFactorV(),
		PrevObsFtrV(),
		PrevContrFtrV(),
		PrevRecTm(),
		Verbose(true),
		Callback(nullptr),
		Notify(nullptr) {}

TStreamStory::TStreamStory(const PStateIdentifier& _StateIdentifier, const PMChain& _MChain,
		const PHierarch& _Hierarch, const bool& _Verbose):
		StateIdentifier(_StateIdentifier),
		MChain(_MChain),
		Hierarch(_Hierarch),
		StateAssist(new TStateAssist(_Verbose)),
//		FtrFactorV(),
		PrevObsFtrV(),
		PrevContrFtrV(),
		PrevRecTm(),
		Verbose(_Verbose),
		Callback(nullptr),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
}

TStreamStory::TStreamStory(TSIn& SIn):
		StateIdentifier(TStateIdentifier::Load(SIn)),
	MChain(TMChain::Load(SIn)),
	Hierarch(THierarch::Load(SIn)),
	StateAssist(new TStateAssist(SIn)),
//	FtrFactorV(SIn),
	PrevObsFtrV(SIn),
	PrevContrFtrV(SIn),
	PrevRecTm(TUInt64(SIn)),
	Verbose(TBool(SIn)),
	Callback(nullptr),
	Notify() {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStreamStory::Save(TSOut& SOut) const {
	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::Save: saving to stream ...");

	StateIdentifier->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
	StateAssist->Save(SOut);
//	FtrFactorV.Save(SOut);
	PrevObsFtrV.Save(SOut);
	PrevContrFtrV.Save(SOut);
	TUInt64(PrevRecTm).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PJsonVal TStreamStory::GetJson() const {
	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::SaveJson: saving JSON ...");

	PJsonVal Result = TJsonVal::NewArr();

	// we need to build a hierarchy and model state transitions
	// on each level of the hierarchy

	// variables
	TStateSetV StateSetV;
	TStateIdV StateIdV;
	TIntFltPrV StateIdProbPrV;

	const TFltV& UniqueHeightV = Hierarch->GetUniqueHeightV();
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV);

	// go through all the heights except the last one, which is not interesting
	// since it is only one state
	for (int HeightN = 0; HeightN < UniqueHeightV.Len()-1; HeightN++) {
		const double CurrHeight = UniqueHeightV[HeightN];

		PJsonVal LevelJsonVal = TJsonVal::NewObj();

		StateIdV.Clr();
		StateSetV.Clr();
		StateIdProbPrV.Clr();

		// get the states on this level
		Hierarch->GetStateSetsAtHeight(CurrHeight, StateIdV, StateSetV);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// chindren
		TFullMatrix TransitionMat = MChain->GetTransitionMat(StateSetV, StateFtrVV);
		TFltV HoldingTimeV;	MChain->GetHoldingTimeV(StateSetV, StateFtrVV, HoldingTimeV);
		TFltV StateSizeV;	MChain->GetStateSizeV(StateSetV, StateFtrVV, StateSizeV);
		TLinAlg::MultiplyScalar((CurrHeight + .1), StateSizeV);

		// construct state JSON
		PJsonVal StateJsonV = TJsonVal::NewArr();
		for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
			const int StateId = StateIdV[StateN];
			const TFltPr& StateCoords = Hierarch->GetStateCoords(StateId);

			PJsonVal StateJson = TJsonVal::NewObj();
			StateJson->AddToObj("id", StateId);
			StateJson->AddToObj("x", StateCoords.Val1);
			StateJson->AddToObj("y", StateCoords.Val2);
			StateJson->AddToObj("size", StateSizeV[StateN]);
			StateJson->AddToObj("holdingTime", HoldingTimeV[StateN]);
			StateJson->AddToObj("isTarget", Hierarch->IsTarget(StateId, CurrHeight));

			if (Hierarch->IsStateNm(StateId)) {
				StateJson->AddToObj("name", Hierarch->GetStateNm(StateId));
			}

			StateJsonV->AddToArr(StateJson);
		}

		// construct tansition JSON
		PJsonVal JumpMatJson = TJsonVal::NewArr();
		for (int RowIdx = 0; RowIdx < TransitionMat.GetRows(); RowIdx++) {
			PJsonVal RowJson = TJsonVal::NewArr();

			for (int ColIdx = 0; ColIdx < TransitionMat.GetCols(); ColIdx++) {
				RowJson->AddToArr(TransitionMat(RowIdx, ColIdx));
			}

			JumpMatJson->AddToArr(RowJson);
		}

		LevelJsonVal->AddToObj("height", CurrHeight);
		LevelJsonVal->AddToObj("states", StateJsonV);
		LevelJsonVal->AddToObj("transitions", JumpMatJson);

		Result->AddToArr(LevelJsonVal);
	}

	return Result;
}

void TStreamStory::Init(TFltVV& ObservFtrVV, TFltVV& ControlFtrVV, const TUInt64V& RecTmV) {
	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, TBoolV(), FtrVV);

	TIntV AssignV;	InitClust(ObservFtrVV, FtrVV, AssignV);
	InitMChain(FtrVV, AssignV, RecTmV, false, TBoolV());
	InitHierarch();
	InitStateAssist(ObservFtrVV, ControlFtrVV);
//
//	FtrFactorV.Gen(ControlFtrVV.GetRows());
//	for (int i = 0; i < FtrFactorV.Len(); i++) {
//		FtrFactorV[i] = 1;
//	}
}

void TStreamStory::InitBatches(TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
		const TUInt64V& RecTmV, const TBoolV& BatchEndV) {
	CheckBatches(RecTmV, BatchEndV);

	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, BatchEndV, FtrVV);

	TIntV AssignV;	InitClust(ObservFtrVV, FtrVV, AssignV);
	InitMChain(FtrVV, AssignV, RecTmV, true, BatchEndV);
	InitHierarch();
	InitStateAssist(ObservFtrVV, ControlFtrVV);
//
//	FtrFactorV.Gen(ContrFtrVV.GetRows());
//	for (int i = 0; i < FtrFactorV.Len(); i++) {
//		FtrFactorV[i] = 1;
//	}
}

void TStreamStory::InitClust(TFltVV& ObsFtrVV, const TFltVV& FtrVV,
		TIntV& AssignV) {
	StateIdentifier->Init(ObsFtrVV, FtrVV);
	StateIdentifier->Assign(ObsFtrVV, AssignV);
}

void TStreamStory::InitMChain(const TFltVV& FtrVV, const TIntV& AssignV,
		const TUInt64V& RecTmV, const bool IsBatchData, const TBoolV& EndBatchV) {
	MChain->Init(FtrVV, StateIdentifier->GetStates(), AssignV, RecTmV, IsBatchData, EndBatchV);
}

void TStreamStory::InitHierarch() {
	Hierarch->Init(StateIdentifier->GetCentroidMat(), MChain->GetCurrStateId());
}

void TStreamStory::InitHistograms(const TFltVV& ObsMat, const TFltVV& ControlMat,
		const TUInt64V& RecTmV, const TBoolV& BatchEndV) {
	TFltVV FtrVV;	CreateFtrVV(ObsMat, ControlMat, RecTmV, BatchEndV, FtrVV);
	StateIdentifier->InitHistogram(ObsMat, FtrVV);
}

void TStreamStory::InitStateAssist(TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV) {
	StateAssist->Init(ObsFtrVV, ContrFtrVV, StateIdentifier, Hierarch);
}

void TStreamStory::OnAddRec(const uint64& RecTm, const TFltV& ObsFtrV, const TFltV& ContrFtrV) {
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV);
	TFltV FtrV;	CreateFtrV(ObsFtrV, ContrFtrV, RecTm, FtrV);

	const int OldStateId = MChain->GetCurrStateId();
	const int NewStateId = StateIdentifier->Assign(ObsFtrV);

	DetectAnomalies(OldStateId, NewStateId, ObsFtrV, FtrV);

	if (NewStateId != -1) {
		MChain->OnAddRec(NewStateId, RecTm, false);

		if (NewStateId != OldStateId && Callback != nullptr) {
			PredictTargets(RecTm, StateFtrVV, NewStateId);	// TODO should I use StateFtrVV???
			Hierarch->UpdateHistory(NewStateId);

			TIntFltPrV CurrStateV;	GetCurrStateAncestry(CurrStateV);
			Callback->OnStateChanged(CurrStateV);
		}
	}

	PrevObsFtrV = ObsFtrV;
	PrevContrFtrV = ContrFtrV;
	PrevRecTm = RecTm;
}

void TStreamStory::GetFutStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	EAssertR(Tm >= 0, "Time should be greater than 0!");

	try {
		TStateSetV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetFutureProbV(StateSetV, StateFtrVV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetFutStateProbs: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetPastStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	try {
		TStateSetV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetPastProbV(StateSetV, StateFtrVV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetPastStateProbV: Failed to compute past state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetNextStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TStateSetV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetNextStateProbV(StateSetV, StateFtrVV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetNextStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetPrevStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TStateSetV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetPrevStateProbV(StateSetV, StateFtrVV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetPrevStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetProbVOverTm(const double& Height, const int& StateId, const double StartTm, const double EndTm, const double& DeltaTm, TStateIdV& StateIdV, TVec<TFltV>& FutProbV, TVec<TFltV>& PastProbV) const {
	try {
		TStateSetV StateSetV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetProbVOverTm(Height, StateId, StartTm, EndTm, DeltaTm, StateSetV, StateFtrVV, StateIdV, FutProbV, PastProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetPrevStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const {
	try {
		Hierarch->GetHistStateIdV(Height, StateIdV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistStateIdV: Failed to compute fetch historical states: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const {
	try {
		TIntV LeafV;
		Hierarch->GetLeafDescendantV(StateId, LeafV);
		StateIdentifier->GetHistogram(FtrId, LeafV, BinStartV, ProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetStateWgtV(const int& StateId, TFltV& WgtV) const {
	try {
		StateAssist->GetSuggestFtrs(StateId, WgtV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateWgtV: Failed to fetch weight vector for state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetTransitionModel(const double& Height, TFltVV& Mat) const {
	TStateIdV StateIdV;
	TStateSetV StateSetV;
	TStateFtrVV StateFtrVV;

	GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
	Mat = MChain->GetModel(StateSetV, StateFtrVV).GetMat();
}

void TStreamStory::GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const {
	Hierarch->GetAncestorV(StateId, StateIdHeightPrV);
}

void TStreamStory::GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const {
	Hierarch->GetCurrStateIdHeightPrV(StateIdHeightPrV);
}

int TStreamStory::GetCurrStateId(const double& Height) const {
	return Hierarch->GetAncestorAtHeight(MChain->GetCurrStateId(), Height);
}

void TStreamStory::GetCentroid(const int& StateId, TFltV& FtrV, const bool ObsCentroid) const {
	TIntV LeafIdV;	Hierarch->GetLeafDescendantV(StateId, LeafIdV);
	TVector Centroid = ObsCentroid ?
			StateIdentifier->GetJoinedCentroid(LeafIdV) :
			StateIdentifier->GetJoinedControlCentroid(LeafIdV);
	FtrV = Centroid.Vec;
}

void TStreamStory::GetStateIdVAtHeight(const double& Height, TStateIdV& StateIdV) const {
	try {
		TStateSetV StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::TStreamStory::GetStateIdVAtHeight: Failed to fetch state IDs for height %.3f: %s", Height, Except->GetMsgStr().CStr());
		throw Except;
	}
}

uint64 TStreamStory::GetTimeUnit() const {
	return MChain->GetTimeUnit();
}

void TStreamStory::SetTargetState(const int& StateId, const double& Height, const bool& IsTrg) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Setting target state %d on height %.3f, isTarget: %s", StateId, Height, TBool::GetStr(IsTrg).CStr());

	if (IsTrg) {
		Hierarch->SetTarget(StateId, Height);
	} else {
		Hierarch->RemoveTarget(StateId, Height);
	}
}

bool TStreamStory::IsBottomState(const int& StateId) const {
	return StateId < StateIdentifier->GetStates();
}

void TStreamStory::SetStateNm(const int& StateId, const TStr& StateNm) {
	try {
		Hierarch->SetStateNm(StateId, StateNm);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetStateNm: Failed to set name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetControlFtrVal(const int& StateId, const int& FtrId, const double& Val) {
	try {
		const int ObsFtrs = StateIdentifier->GetDim();
		StateIdentifier->SetControlFtr(StateId, FtrId - ObsFtrs, Val);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetControlFtrFactor: Failed to set factor: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetControlFtrsVal(const int& FtrId, const double& Val) {
	const int States = StateIdentifier->GetStates();
	for (int StateId = 0; StateId < States; StateId++) {
		SetControlFtrVal(StateId, FtrId, Val);
	}
}

void TStreamStory::ResetControlFtrVal(const int& StateId, const int& FtrId) {
	try {
		const int ObsFtrs = StateIdentifier->GetDim();
		StateIdentifier->ClearControlFtr(StateId, FtrId - ObsFtrs);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetControlFtrFactor: Failed to set factor: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::ResetControlFtrVals(const int& StateId) {
	const int ObsDim = StateIdentifier->GetDim();
	const int Dim = StateIdentifier->GetControlDim();
	for (int FtrId = 0; FtrId < Dim; FtrId++) {
		ResetControlFtrVal(StateId, FtrId + ObsDim);
	}
}

void TStreamStory::ResetControlFtrVals() {
	const int States = StateIdentifier->GetStates();
	for (int StateId = 0; StateId < States; StateId++) {
		ResetControlFtrVals(StateId);
	}
}

const TFltPr& TStreamStory::GetFtrBounds(const int& FtrId) const {
	try {
		return StateAssist->GetFtrBounds(FtrId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetControlFtrBounds: Failed to get feature bounds: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

const TStr& TStreamStory::GetStateNm(const int& StateId) const {
	try {
		return Hierarch->GetStateNm(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateNm: Failed to get name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}

	StateIdentifier->SetVerbose(Verbose);
	MChain->SetVerbose(Verbose);
	Hierarch->SetVerbose(Verbose);
}

void TStreamStory::SetCallback(TCallback* _Callback) {
	Callback = _Callback;
}

void TStreamStory::CreateFtrVV(const TFltVV& ObsFtrMat, const TFltVV& ContrFtrMat,
		const TUInt64V& RecTmV, const TBoolV& EndsBatchV, TFltVV& FtrVV) const {
	const int NInst = ObsFtrMat.GetCols();
//	const bool HasHiddenState = !EndsBatchV.Empty();

	FtrVV.Gen(ContrFtrMat.GetRows(), NInst);
	for (int ColN = 0; ColN < NInst; ColN++) {
		for (int RowN = 0; RowN < ContrFtrMat.GetRows(); RowN++) {
			FtrVV(RowN, ColN) = ContrFtrMat(RowN, ColN);
		}
	}
//
//	TFltV FtrV, ObsFtrV, ContrFtrV, PrevObsFtrV, PrevContrFtrV;
//	uint64 RecTm, PrevRecTm;
//	for (int ColN = 0; ColN < NInst; ColN++) {
//		ObsFtrMat.GetCol(ColN, ObsFtrV);
//		ContrFtrMat.GetCol(ColN, ContrFtrV);
//		RecTm = RecTmV[ColN];
//
//		if (ColN == 0 || (HasHiddenState && EndsBatchV[ColN-1])) {
//			PrevObsFtrV.Clr();
//			PrevContrFtrV.Clr();
//			PrevRecTm = 0;
//		} else {
//			ObsFtrMat.GetCol(ColN-1, PrevObsFtrV);
//			ContrFtrMat.GetCol(ColN-1, PrevContrFtrV);
//			PrevRecTm = RecTmV[ColN-1];
//		}
//
//		MChain->CreateFtrV(ObsFtrV, PrevObsFtrV, ContrFtrV, PrevObsFtrV, RecTm, PrevRecTm, FtrV);
//
//		for (int RowN = 0; RowN < FtrV.Len(); RowN++) {
//			FtrVV(RowN, ColN) = FtrV[RowN];
//		}
//	}
}

void TStreamStory::CreateFtrV(const TFltV& ObsFtrV, const TFltV& ContrFtrV,
		const uint64& RecTm, TFltV& FtrV) const {
	FtrV = ContrFtrV;
//	MChain->CreateFtrV(ObsFtrV, PrevObsFtrV, ContrFtrV, PrevContrFtrV, RecTm, PrevRecTm, FtrV);
}

void TStreamStory::GetStateFtrVV(TStateFtrVV& StateFtrVV) const {
	StateIdentifier->GetControlCentroidVV(StateFtrVV);
}

void TStreamStory::GetStatsAtHeight(const double& Height, TStateSetV& StateSetV,
		TStateIdV& StateIdV, TStateFtrVV& StateFtrVV) const {
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	GetStateFtrVV(StateFtrVV);
}

void TStreamStory::DetectAnomalies(const int& NewStateId, const int& OldStateId,
		const TFltV& ObsFtrV, const TFltV& FtrV) const {
	if (NewStateId != OldStateId) {
		if (MChain->IsAnomalousJump(FtrV, NewStateId, OldStateId)) {
			Callback->OnAnomaly(TStr::Fmt("Anomalous jump, old state %d, new state %d", OldStateId, NewStateId));
		}
		if (NewStateId == -1) {
			Callback->OnOutlier(ObsFtrV);
		}
	}
}

void TStreamStory::PredictTargets(const uint64& RecTm, const TStateFtrVV& StateFtrVV,
		const int& CurrLeafId) const {
	const TIntFltPrSet& TargetIdHeightSet = Hierarch->GetTargetStateIdSet();

	double Prob;
	TFltV ProbV, TmV;

	TStateSetV StateSetV;
	TStateIdV StateIdV;

	int KeyId = TargetIdHeightSet.FFirstKeyId();
	while (TargetIdHeightSet.FNextKeyId(KeyId)) {
		const TIntFltPr& TrgIdHeightPr = TargetIdHeightSet.GetKey(KeyId);

		const int& TrgStateId = TrgIdHeightPr.Val1;
		const double& Height = TrgIdHeightPr.Val2;
		const int CurrStateId = Hierarch->GetAncestorAtHeight(CurrLeafId, Height);

		Prob = 0;
		ProbV.Clr();
		TmV.Clr();
		StateSetV.Clr();
		StateIdV.Clr();

		// get the state sets
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);

		bool WillOccur = MChain->PredictOccurenceTime(StateFtrVV, StateSetV, StateIdV,
				CurrStateId, TrgStateId, Prob, ProbV, TmV);

		if (WillOccur) {
			Callback->OnPrediction(RecTm, CurrStateId, TrgStateId, Prob, ProbV, TmV);
		}
	}
}

void TStreamStory::CheckBatches(const TUInt64V& TmV, const TBoolV& BatchEndV) const {
	Notify->OnNotify(TNotifyType::ntInfo, "Checking batches ...");

	// check if any batches of length 0 exist and the times are OK
	bool JustEnded = false;
	uint64 PrevTm = 0;

	for (int i = 0; i < BatchEndV.Len(); i++) {
		const bool EndsBatch = BatchEndV[i];
		const uint64 Tm = TmV[i];

		EAssertR(!EndsBatch || !JustEnded, "Found a batch of length 0. Cannot model such data! Index: " + TInt::GetStr(i));
		EAssertR(Tm >= PrevTm || JustEnded, "Time less then previous time even though the batch hasn't ended! Index: " + TInt::GetStr(i));

		JustEnded = EndsBatch;
		PrevTm = Tm;
	}

	Notify->OnNotify(TNotifyType::ntInfo, "The batches are OK!");
}

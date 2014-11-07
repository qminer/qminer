#include "ctmc.h"

using namespace TCtmc;

//////////////////////////////////////////////////////
// Distance measures
TFullMatrix TEuclDist::GetDist(const TFullMatrix& X, const TFullMatrix& Y) {
	return GetDist(X, Y).Sqrt();
}

TFullMatrix TEuclDist::GetDist2(const TFullMatrix& X, const TFullMatrix& Y) {
	const TVector OnesX = TVector::Ones(X.GetCols(), true);
	const TVector OnesY = TVector::Ones(Y.GetCols(), false);
	const TVector NormX2 = X.ColNorm2V().Transpose();
	const TVector NormY2 = Y.ColNorm2V();

	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
}

//////////////////////////////////////////////////
// Abstract clustering
TClust::TClust(const TRnd& _Rnd):
	Rnd(_Rnd),
	CentroidMat(),
	CentroidDistStatV() {}

int TClust::Assign(const TVector& x) const {
	return GetDistVec(x).GetMinIdx();
}

TVector TClust::Assign(const TFullMatrix& X) const {
	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetClusts(), true);
	const TVector NormX2 = X.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return Assign(X, NormX2, NormC2, OnesN, OnesK);
}

TFullMatrix TClust::GetDistMat(const TFullMatrix& X) const {
	const TVector OnesN = TVector::Ones(X.GetCols(), false);
	const TVector OnesK = TVector::Ones(GetClusts(), true);
	const TVector NormX2 = X.ColNorm2V();
	const TVector NormC2 = CentroidMat.ColNorm2V().Transpose();

	return GetDistMat2(X, NormX2, NormC2, OnesN, OnesK).Sqrt();
}

TVector TClust::GetDistVec(const TVector& x) const {
	TVector xC = x.IsColVec() ? x.MulT(CentroidMat) : x * CentroidMat;
	return (CentroidMat.ColNorm2V() - (xC*2) + TVector::Ones(GetClusts(), false) * x.Norm2()).Sqrt();
}

double TClust::GetDist(const int& CentroidIdx, const TVector& Pt) const {
	return CentroidMat.GetCol(CentroidIdx).EuclDist(Pt);
}

double TClust::GetMeanPtCentDist(const int& CentroidIdx) const {
	EAssertR(CentroidIdx < GetClusts(), "TFullKMeans::GetMeanPtCentDist: Invalid centroid index: " + CentroidIdx);
	return CentroidDistStatV[CentroidIdx].Val2 / CentroidDistStatV[CentroidIdx].Val1;
}

uint64 TClust::GetClustSize(const int& ClustIdx) const {
	return CentroidDistStatV[ClustIdx].Val1;
}

TFullMatrix TClust::SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV) {
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

void TClust::UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV) {
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

TFullMatrix TClust::GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return (NormC2 * OnesN) - (CentroidMat*2).MulT(X) + (OnesK * NormX2);
}

TVector TClust::Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const {
	return GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK).GetColMinIdxV();
}

void TClust::InitStatistics(const TFullMatrix& X, const TVector& AssignV) {
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

//////////////////////////////////////////////////
TFullKMeans::TFullKMeans(const int& _K, const TRnd& Rnd):
		TClust(Rnd),
		K(_K) {}

TFullMatrix TFullKMeans::Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter) {
	EAssertR(K <= X.GetRows(), "Matrix should have more rows then k!");

	printf("Executing KMeans ...\n");

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
		if (i % 10000 == 0) { printf("%d\n", i); }

		*AssignIdxVPtr = Assign(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			printf("Converged at iteration: %d\n", i);
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
	AssignV = AssignIdxV.GetIntVec();

	return CentroidMat;
}

//////////////////////////////////////////////////
// DPMeans
TDpMeans::TDpMeans(const TFlt& _Lambda, const TInt& _MinClusts, const TInt& _MaxClusts, const TRnd& Rnd):
		TClust(Rnd),
		Lambda(_Lambda),
		MinClusts(_MinClusts),
		MaxClusts(_MaxClusts) {

	EAssertR(MinClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
	EAssertR(MaxClusts >= MinClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

TFullMatrix TDpMeans::Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter) {
	EAssertR(MinClusts <= X.GetCols(), "Matrix should have more rows then the min number of clusters!");

	printf("Executing DPMeans ...\n");

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
		if (i % 10 == 0) { printf("%d\n", i); }

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

				printf("Max distance to centroid: %.3f, number of clusters: %d...\n", sqrt(MaxDist), GetClusts());
			}
		}

		// check if converged
		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			printf("Converged at iteration: %d\n", i);
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
	AssignV = AssignIdxV.GetIntVec();

	return CentroidMat;
}

/////////////////////////////////////////////////////////////////
// Agglomerative clustering
void TAggClust::MakeDendro(const TFullMatrix& X, TIntIntFltTrV& MergeV) {
	const int NInst = X.GetCols();

	printf("%s\n\n", TStrUtil::GetStr(X.GetMat(), ", ", "%.3f").CStr());

	TFullMatrix X1 = X;	// copy

	TFullMatrix ClustDistMat = TEuclDist::GetDist2(X,X);
	TVector ItemCountV = TVector::Ones(NInst);

	for (int k = 0; k < NInst-1; k++) {
		// find active <i,j> with minimum distance
		int MnI = -1;
		int MnJ = -1;
		double MnDist = TFlt::PInf;

		// find clusters with min distance
		for (int i = 0; i < NInst; i++) {
			if (ItemCountV[i] == 0.0) { continue; }

			for (int j = i+1; j < NInst; j++) {
				if (i == j || ItemCountV[j] == 0.0) { continue; }

				if (ClustDistMat(i,j) < MnDist) {
					MnDist = ClustDistMat(i,j);
					MnI = i;
					MnJ = j;
				}
			}
		}

		printf("%s\n\n", TStrUtil::GetStr(ItemCountV.Vec, ", ", "%.3f").CStr());
		printf("%s\n\n", TStrUtil::GetStr(ClustDistMat.GetMat(), ", ", "%.3f").CStr());

		// merge
		MergeV.Add(TIntIntFltTr(MnI, MnJ, sqrt(MnDist < 0 ? 0 : MnDist)));

		// average x_i and x_j and update counts
		int NewClustSize = ItemCountV[MnI] + ItemCountV[MnJ];
		X1.SetCol(MnI, (X1.GetCol(MnI)*ItemCountV[MnI] + X1.GetCol(MnJ)*ItemCountV[MnJ]) / (double) NewClustSize);
		// update counts
		ItemCountV[MnI] = NewClustSize;
		ItemCountV[MnJ] = 0;

		// update distances
		TVector NewDistV = TEuclDist::GetDist2(TFullMatrix(X1.GetCol(MnI)), X1);
		ClustDistMat.SetRow(MnI, NewDistV);
		ClustDistMat.SetCol(MnI, NewDistV.Transpose());
	}
}

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
const uint64 TCtMChain::TU_SECOND = 1000;
const uint64 TCtMChain::TU_MINUTE = TU_SECOND*60;
const uint64 TCtMChain::TU_HOUR = TU_MINUTE*60;
const uint64 TCtMChain::TU_DAY = TU_HOUR*24;

TCtMChain::TCtMChain(const uint64 _TimeUnit):
//		StateCentMat(),
		QMatStats(),
//		StateStatV(),
//		Clust(_Clust),
		TimeUnit(_TimeUnit),
		NStates(-1),
		CurrStateIdx(-1),
		PrevJumpTm(-1) {}

void TCtMChain::Init(const int& _NStates, const TIntV& StateAssignV, const TUInt64V& TmV) {
	NStates = _NStates;

	const int NRecs = StateAssignV.Len();

	// initialize a matrix holding the number of measurements and the sum
	QMatStats.Gen(NStates, 0);
	for (int i = 0; i < NStates; i++) {
		QMatStats.Add(TUInt64FltPrV(NStates, NStates));
	}

	// update intensities
	for (int i = 0; i < NRecs; i++) {
		UpdateIntensities(TmV[i], StateAssignV[i]);
	}
}

//void TCtMChain::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
////	InitStateStats(X);
//	InitIntensities(X, RecTmV, AssignV);
//}

void TCtMChain::OnAddRec(const int& StateIdx, const uint64& RecTm) {
	// update statistics
//	UpdateStatistics(Rec, StateIdx);
	// update intensities
	UpdateIntensities(RecTm, StateIdx);
	// update current state
	CurrStateIdx = StateIdx;
}

//void TCtMChain::InitStateStats(const TFullMatrix& X) {
//	const int NStates = GetStates();
//
//	printf("Initailizing statistics ...\n");
//
//	StateStatV.Gen(NStates, 0);
//	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
//		double MeanPtCentDist = Clust->GetMeanPtCentDist(StateIdx);
//		uint64 ClustSize = Clust->GetClustSize(StateIdx);
//
//		StateStatV.Add(TUInt64FltPr(ClustSize, ClustSize * MeanPtCentDist));
//
//		printf("State %d, points %ld, mean centroid dist %.3f\n", StateIdx, GetStateSize(StateIdx), GetMeanPtCentroidDist(StateIdx));
//	}
//}

//void TCtMChain::InitIntensities(const TFullMatrix& X, const TUInt64V& RecTmV, const TIntV& AssignIdxV) {
//	// compute the intensities using the maximum likelihood estimate
//	// lambda = 1 / t_avg = n / sum(t_i)
//
//	const int NRecs = X.GetCols();
//	const int NStates = GetStates();
//
//	// initialize a matrix holding the number of measurements and the sum
//	QMatStats.Gen(NStates, 0);
//	for (int i = 0; i < NStates; i++) {
//		QMatStats.Add(TUInt64FltPrV(NStates, NStates));
//	}
//
//	// update intensities
//	for (int i = 0; i < NRecs; i++) {
//		UpdateIntensities(X.GetCol(i), RecTmV[i], AssignIdxV[i]);
//	}
//}

void TCtMChain::UpdateIntensities(const uint64 RecTm, const int& RecState) {
	if (CurrStateIdx != -1 && RecState != CurrStateIdx) {
		// the state has changed
		if (PrevJumpTm != TUInt64::Mx) {
			uint64 HoldingTm = RecTm - PrevJumpTm;
			QMatStats[CurrStateIdx][RecState].Val1++;
			QMatStats[CurrStateIdx][RecState].Val2 += (double) HoldingTm / TimeUnit;
			printf("Updated intensity: prev state: %d, curr state: %d\n", CurrStateIdx, RecState);
		}
		PrevJumpTm = RecTm;
	} else if (CurrStateIdx == -1) {
		PrevJumpTm = RecTm;
	}
}

//void TCtMChain::UpdateStatistics(const TVector& Rec, const int& RecState) {
//	double CentroidDist = Clust->GetDist(RecState, Rec);
//
//	StateStatV[RecState].Val1 += 1;
//	StateStatV[RecState].Val2 += CentroidDist;
//}

//double TCtMChain::GetMeanPtCentroidDist(const int& StateIdx) const {
//	uint64 StateSize = GetStateSize(StateIdx);
//	return StateSize == 0 ? 0 : StateStatV[StateIdx].Val2 / GetStateSize(StateIdx);
//}

//uint64 TCtMChain::GetStateSize(const int& StateIdx) const {
//	return StateStatV[StateIdx].Val1;
//}

/////////////////////////////////////////////////////////////////
// Hierarchical continous time Markov Chain
THierarchCtmc::THierarchCtmc(const PClust& _Clust, const PCtMChain& _MChain, const PAggClust& _AggClust):
		Clust(_Clust),
		MChain(_MChain),
		AggClust(_AggClust),
		HierarchV(),
		StateHeightV() {}

void THierarchCtmc::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
	// partition the input space
	TIntV AssignV;	Clust->Apply(X, AssignV, 10000);
	// initialize intensities
	MChain->Init(Clust->GetClusts(), AssignV, RecTmV);

	// create a hierarchy
	TIntIntFltTrV MergeV;
	AggClust->MakeDendro(Clust->GetCentroidMat(), MergeV);

	printf("%s\n", TStrUtil::GetStr(MergeV, ", ").CStr());

	const int NLeafStates = MChain->GetStates();
	const int NMiddleStates = MergeV.Len();
	const int NStates = NLeafStates + NMiddleStates;

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
		const int MergeStateIdx = NLeafStates + i;

		HierarchV[State1Idx] = MergeStateIdx;
		HierarchV[State2Idx] = MergeStateIdx;
		StateHeightV[MergeStateIdx] = Height;
	}
}

PJsonVal THierarchCtmc::SaveJson() const {
	// TODO
	return NULL;
}

int THierarchCtmc::GetOldestAncestIdx(const int& StateIdx) const {
	int AncestIdx = StateIdx;

	while (HierarchV[AncestIdx] != -1) {
		AncestIdx = HierarchV[AncestIdx];
	}

	return AncestIdx;
}

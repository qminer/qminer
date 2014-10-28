#include "denseclust.h"

using namespace TFullClust;

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
// K-Means
TFullClust::TKMeans::TKMeans(const int& _K, const TRnd& Rnd):
		TClust(Rnd),
		K(_K) {}

TFullMatrix TFullClust::TKMeans::Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter) {
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
	EAssertR(MinClusts <= X.GetRows(), "Matrix should have more rows then the min number of clusters!");

	printf("Executing DPMeans ...\n");

	const int NInst = X.GetCols();

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
		if (i % 10000 == 0) { printf("%d\n", i); }

		// add new centroids and compute the distance matrix
		TFullMatrix D;

		double MaxCentrDist = TFlt::Mx;
		do {
			D = GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK);

			// if the max number of clusters is reached => break
			if (GetClusts() >= MaxClusts) { break; }

			TVector CentrDistV = D.GetColMinV();
			TVector MinIdxV = D.GetColMinIdxV();

			TIntFltPr MaxCentrDistPr = CentrDistV.GetMax();
			MaxCentrDist = MaxCentrDistPr.Val2;

			if (sqrt(MaxCentrDist) > Lambda) {
				CentroidMat.AddCol(X.GetCol(MaxCentrDistPr.Val1));
				OnesK = TVector::Ones(GetClusts(), true);
			}
		} while (sqrt(MaxCentrDist) > Lambda);

		// assign
		*AssignIdxVPtr = D.GetColMinIdxV();

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
// Continous time Markov Chain
void TCtmc::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
	TIntV AssignV;
	CentroidMat = Clust->Apply(X, AssignV, 10000);
	InitStateStats(X);
	InitIntensities(X, RecTmV, AssignV);
}

void TCtmc::OnAddRec(const TVector& Rec, const uint64& RecTm) {
	int RecState = Clust->Assign(Rec);

	// update statistics
	UpdateStatistics(Rec, RecState);
	// update intensities
	UpdateIntensities(Rec, RecTm, RecState);
	// update current state
	CurrStateIdx = RecState;
}

void TCtmc::InitStateStats(const TFullMatrix& X) {
	const int NStates = GetStates();

	printf("Initailizing statistics ...\n");

	StateStatV.Gen(NStates, 0);
	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
		double MeanPtCentDist = Clust->GetMeanPtCentDist(StateIdx);
		uint64 ClustSize = Clust->GetClustSize(StateIdx);

		StateStatV.Add(TUInt64FltPr(ClustSize, ClustSize * MeanPtCentDist));

		printf("State %d, points %ld, mean centroid dist %.3f\n", StateIdx, GetStateSize(StateIdx), GetMeanPtCentroidDist(StateIdx));
	}
}

void TCtmc::InitIntensities(const TFullMatrix& X, const TUInt64V& RecTmV, const TIntV& AssignIdxV) {
	// compute the intensities using the maximum likelihood estimate
	// lambda = 1 / t_avg = n / sum(t_i)

	const int NRecs = X.GetCols();
	const int NStates = GetStates();

	// initialize a matrix holding the number of measurements and the sum
	QMatStats.Gen(NStates, 0);
	for (int i = 0; i < NStates; i++) {
		QMatStats.Add(TUInt64FltPrV(NStates, NStates));
	}

	// update intensities
	for (int i = 0; i < NRecs; i++) {
		UpdateIntensities(X.GetCol(i), RecTmV[i], AssignIdxV[i]);
	}
}

void TCtmc::UpdateIntensities(const TVector& Rec, const uint64 RecTm, const int& RecState) {
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

void TCtmc::UpdateStatistics(const TVector& Rec, const int& RecState) {
	double CentroidDist = Clust->GetDist(RecState, Rec);

	StateStatV[RecState].Val1 += 1;
	StateStatV[RecState].Val2 += CentroidDist;
}

double TCtmc::GetMeanPtCentroidDist(const int& StateIdx) const {
	uint64 StateSize = GetStateSize(StateIdx);
	return StateSize == 0 ? 0 : StateStatV[StateIdx].Val2 / GetStateSize(StateIdx);
}

uint64 TCtmc::GetStateSize(const int& StateIdx) const {
	return StateStatV[StateIdx].Val1;
}

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

void TClust::Save(TSOut& SOut) const {
	Rnd.Save(SOut);
	CentroidMat.Save(SOut);
	CentroidDistStatV.Save(SOut);
}

void TClust::Load(TSIn& SIn) {
	Rnd = TRnd(SIn);
	CentroidMat.Load(SIn);
	CentroidDistStatV.Load(SIn);
}

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

void TFullKMeans::Save(TSOut& SOut) const {
	TClust::Save(SOut);
	K.Save(SOut);
}

void TFullKMeans::Load(TSIn& SIn) {
	TClust::Load(SIn);
	K.Load(SIn);
}

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

void TDpMeans::Save(TSOut& SOut) const {
	TClust::Save(SOut);
	Lambda.Save(SOut);
	MinClusts.Save(SOut);
	MaxClusts.Save(SOut);
}

void TDpMeans::Load(TSIn& SIn) {
	TClust::Load(SIn);
	Lambda.Load(SIn);
	MinClusts.Load(SIn);
	MaxClusts.Load(SIn);
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

TFullMatrix TEuclMds::Project(const TFullMatrix& X, const int& Dims) {
	// first center the rows of matrix X
	TFullMatrix X1 = X.GetCenteredRows();

	// get the eigenvectors and eigenvalues of X'X
	// if we compute the singular value decomposition X = U*S*V'
	// then the diagonal entries in S are the squared eigenvalues of X*X' and X'X
	// the vectors in U are the eigenvectors of X*X'
	// the vectors in V are the eigenvectors of X'X
	TMatVecMatTr Svd = X1.Svd(2);

	TVector& EigVals = Svd.Val2.Sqrt();
	TFullMatrix& EigVecs = Svd.Val3;

	// lets call the matrix with eigenvectors V and the matrix of eigenvalues L
	// the coordinates of the points are then X = V*L^(.5)
	return EigVecs * TFullMatrix::Diag(EigVals.Sqrt());
}

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
// Agglomerative clustering
THierarch::THierarch():
		HierarchV(),
		StateHeightV(),
		MxHeight(TFlt::Mn),
		StateCoordV() {}

void THierarch::Save(TSOut& SOut) const {
	HierarchV.Save(SOut);
	StateHeightV.Save(SOut);
	MxHeight.Save(SOut);
	StateCoordV.Save(SOut);
	TInt(NLeafs).Save(SOut);
}

void THierarch::Load(TSIn& SIn) {
	HierarchV.Load(SIn);
	StateHeightV.Load(SIn);
	MxHeight.Load(SIn);
	StateHeightV.Load(SIn);
	NLeafs = TInt(SIn);
}

void THierarch::Init(const TFullMatrix& CentroidMat) {
	NLeafs = CentroidMat.GetCols();

	// create a hierarchy
	TIntIntFltTrV MergeV;	TAggClust::MakeDendro(CentroidMat, MergeV);

	printf("%s\n", TStrUtil::GetStr(MergeV, ", ").CStr());

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

		if (Height > MxHeight) { MxHeight = Height; }
	}

	HierarchV.Last() = HierarchV.Len() - 1;

	// compute state coordinates
	ComputeStateCoords(CentroidMat, NStates);
}

int THierarch::GetLowestHeightIdx(const TBoolV& IgnoreV) const {
	const int NStates = GetStates();

	double MnHeight = TFlt::Mx;
	int MnHeightIdx = -1;

	for (int i = 0; i < NStates; i++) {
		if (!IgnoreV[i].Val && StateHeightV[i] < MnHeight) {
			MnHeight = StateHeightV[i];
			MnHeightIdx = i;
		}
	}

	return MnHeightIdx;
}

void THierarch::GetStatesAtHeight(const double& Height, TIntSet& StateIdxV) const {
	const int NStates = GetStates();

	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
		const int ParentIdx = HierarchV[StateIdx];
		if (StateHeightV[StateIdx] <= Height && StateHeightV[ParentIdx] > Height) {
			StateIdxV.AddKey(StateIdx);
		}
	}
}

int THierarch::GetAncestorAtHeight(const int& LeafIdx, const double& Height) const {
	EAssertR(Height <= MxHeight, "Cannot search for states at height larger than MxHeight!");

	TIntV TempHierarchV(HierarchV);

	int AncestorIdx = LeafIdx;

	printf("\nHierarchy:\n%s\n", TStrUtil::GetStr(TempHierarchV, ",").CStr());

	while (true) {
		const int ParentIdx = TempHierarchV[AncestorIdx];

		if (ParentIdx == AncestorIdx || StateHeightV[ParentIdx] > Height) { break; }

		AncestorIdx = ParentIdx;
	}

	return AncestorIdx;
}

void THierarch::GetStateLeafHAtHeight(const double& Height, TIntIntVH& StateSubStateH) const {
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

void THierarch::GetSuccesorCountV(TIntV& SuccesorCountV) const {
	const int NStates = GetStates();

	SuccesorCountV.Gen(NStates, NStates);

	for (int i = 0; i < NLeafs; i++) {
		SuccesorCountV[i] = 1;
	}

	TIntV TempHierarchV(HierarchV);

	bool Change;
	do {
		Change = false;

		for (int i = 0; i < NLeafs; i++) {
			const int AncestorIdx = TempHierarchV[i];
			const int LeafWeight = SuccesorCountV[i];
			SuccesorCountV[AncestorIdx] += LeafWeight;
		}

		for (int i = 0; i < NLeafs; i++) {
			if (TempHierarchV[i] != TempHierarchV[TempHierarchV[i]]) {
				TempHierarchV[i] = TempHierarchV[TempHierarchV[i]];
				Change = true;
			}
		}
	} while (Change);
}

void THierarch::ComputeStateCoords(const TFullMatrix& CentroidMat, const int& NStates) {
	StateCoordV.Gen(NStates, NStates);

	TFullMatrix CoordMat = TEuclMds::Project(CentroidMat, 2);
	for (int RowIdx = 0; RowIdx < CoordMat.GetRows(); RowIdx++) {
		StateCoordV[RowIdx].Val1 = CoordMat(RowIdx, 0);
		StateCoordV[RowIdx].Val2 = CoordMat(RowIdx, 1);
	}

	// first find out how many ancestors each state has, so you can weight
	// the childs coordinates appropriately
	TIntV SuccesorCountV;	GetSuccesorCountV(SuccesorCountV);

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

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
const uint64 TCtMChain::TU_SECOND = 1000;
const uint64 TCtMChain::TU_MINUTE = TU_SECOND*60;
const uint64 TCtMChain::TU_HOUR = TU_MINUTE*60;
const uint64 TCtMChain::TU_DAY = TU_HOUR*24;

TCtMChain::TCtMChain(const uint64 _TimeUnit):
		QMatStats(),
		TimeUnit(_TimeUnit),
		NStates(-1),
		CurrStateIdx(-1),
		PrevJumpTm(-1) {}

void TCtMChain::Save(TSOut& SOut) const {
	QMatStats.Save(SOut);
	TUInt64(TimeUnit).Save(SOut);
	TInt(NStates).Save(SOut);
	TInt(CurrStateIdx).Save(SOut);
	TUInt64(PrevJumpTm).Save(SOut);
}

void TCtMChain::Load(TSIn& SIn) {
	QMatStats.Load(SIn);
	TimeUnit = TUInt64(SIn);
	NStates = TInt(SIn);
	CurrStateIdx = TInt(SIn);
	PrevJumpTm = TUInt64(SIn);
}

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
		OnAddRec(StateAssignV[i], TmV[i]);
	}
}

void TCtMChain::OnAddRec(const int& StateIdx, const uint64& RecTm) {
	// update intensities
	UpdateIntensities(RecTm, StateIdx);
	// update current state
	CurrStateIdx = StateIdx;
}

TFullMatrix TCtMChain::GetJumpMatrix(const TFullMatrix& QMat) const {
	const int Rows = QMat.GetRows();
	const int Cols = QMat.GetCols();

	TFullMatrix JumpMat(Rows, Cols);

	for (int i = 0; i < Rows; i++) {
		if (QMat(i,i) == 0.0) {
			JumpMat(i,i) = 1;
		} else {
			for (int j = 0; j < Cols; j++) {
				if (j != i) {
					JumpMat(i,j) = QMat(i,j) / (-QMat(i,i));
				}
			}
		}
	}

	return JumpMat;
}

TVector TCtMChain::GetHoldingTimeV(const TFullMatrix& QMat) const {
	const int Rows = QMat.GetRows();

	TVector HoldTmV(Rows);
	for (int i = 0; i < Rows; i++) {
		HoldTmV[i] = -1 / QMat(i,i);
	}

	return HoldTmV;
}

TFullMatrix TCtMChain::GetQMatrix() const {
	// compute the intensities
	const int NStates = GetStates();

	// Q-matrix: holds jump intensities
	TFullMatrix QMatrix(NStates, NStates);
	for (int i = 0; i < NStates; i++) {
		for (int j = 0; j < NStates; j++) {
			if (j != i) {
				const uint64 N = QMatStats[i][j].Val1;
				const double Sum = QMatStats[i][j].Val2;
				QMatrix(i,j) = N > 0 ? N / Sum : 0;
			}
		}

		QMatrix(i,i) = -QMatrix.RowSum(i);
	}

	return QMatrix;
}

TFullMatrix TCtMChain::GetQMatrix(const TVec<TIntV>& JoinedStateVV) const {
	TFullMatrix QMat(JoinedStateVV.Len(), JoinedStateVV.Len());

	for (int i = 0; i < JoinedStateVV.Len(); i++) {
		const TIntV& CurrStateV = JoinedStateVV[i];

		for (int j = 0; j < JoinedStateVV.Len(); j++) {
			if (i == j) { continue; }

			const TIntV& JumpStateV = JoinedStateVV[j];

			uint64 SumN = 0;
			double SumTime = 0;
			for (int CurrStateIdx = 0; CurrStateIdx < CurrStateV.Len(); CurrStateIdx++) {
				const int CurrState = CurrStateV[CurrStateIdx];
				for (int JumpStateIdx = 0; JumpStateIdx < JumpStateV.Len(); JumpStateIdx++) {
					const int JumpState = JumpStateV[JumpStateIdx];

					SumN += QMatStats[CurrState][JumpState].Val1;
					SumTime += QMatStats[CurrState][JumpState].Val2;
				}
			}

			QMat(i,j) = SumN > 0 ? SumN / SumTime : 0;
		}

		QMat(i,i) = -QMat.RowSum(i);
	}

	return QMat;
}

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

/////////////////////////////////////////////////////////////////
// Hierarchical continous time Markov Chain
THierarchCtmc::THierarchCtmc(const PClust& _Clust, const PCtMChain& _MChain, const PHierarch& _Hierarch):
		Clust(_Clust),
		MChain(_MChain),
		Hierarch(_Hierarch) {}

void THierarchCtmc::Save(TSOut& SOut) const {
	Clust->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
}

void THierarchCtmc::Load(TSIn& SIn) {
	Clust->Load(SIn);
	MChain->Load(SIn);
	Hierarch->Load(SIn);
}

PJsonVal THierarchCtmc::SaveJson() const {
	const int NStates = GetStates();

	const double MxStateHeight = Hierarch->GetMxStateHeight();

	PJsonVal Result = TJsonVal::NewArr();

	// we need to build a hierarchy and model state transitions
	// on each level of the hierarchy

	TBoolV TakenHeightV(NStates, NStates);

	int KeyId;
	double CurrHeight;

	do {
		PJsonVal LevelJsonVal = TJsonVal::NewObj();

		// find the lowest unused height
		const int MinHeightIdx = Hierarch->GetLowestHeightIdx(TakenHeightV);
		TakenHeightV[MinHeightIdx] = true;

		CurrHeight = Hierarch->GetStateHeight(MinHeightIdx);

		TIntIntVH StateSubStateH;	Hierarch->GetStateLeafHAtHeight(CurrHeight, StateSubStateH);

		// extract state IDs for this level and their substates
		TVec<TIntV> JoinedStateVV(StateSubStateH.Len(),StateSubStateH.Len());
		PJsonVal StateJsonV = TJsonVal::NewArr();

		int i = 0;
		KeyId = StateSubStateH.FFirstKeyId();
		while (StateSubStateH.FNextKeyId(KeyId)) {
			const int StateIdx = StateSubStateH.GetKey(KeyId);

			PJsonVal StateJson = TJsonVal::NewObj();

			if (StateSubStateH[KeyId].Empty()) {
				JoinedStateVV[i].Add(StateIdx);
			} else {
				const TIntV& SubStateV = StateSubStateH.GetDat(StateIdx);
				for (int j = 0; j < SubStateV.Len(); j++) {
					JoinedStateVV[i].Add(SubStateV[j]);
				}
				JoinedStateVV[i] = StateSubStateH[KeyId];
			}

			const TFltPr& StateCoords = Hierarch->GetStateCoords(StateIdx);

			StateJson->AddToObj("id", StateIdx);
			StateJson->AddToObj("x", StateCoords.Val1);
			StateJson->AddToObj("y", StateCoords.Val2);

			StateJsonV->AddToArr(StateJson);

			i++;
		}

		for (i = 0; i < JoinedStateVV.Len(); i++) {
			printf("%s\n", TStrUtil::GetStr(JoinedStateVV[i], ",").CStr());
		}

		// get the index of the current state at this height
		const int CurrStateIdx = Hierarch->GetAncestorAtHeight(MChain->GetCurrStateIdx(), CurrHeight);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// chindren
		TFullMatrix LevelQMat = MChain->GetQMatrix(JoinedStateVV);
		TFullMatrix JumpMat = MChain->GetJumpMatrix(LevelQMat);
		TVector HoldingTmV = MChain->GetHoldingTimeV(LevelQMat);

		// construct JSON
		PJsonVal JumpMatJson = TJsonVal::NewArr();
		for (int RowIdx = 0; RowIdx < JumpMat.GetRows(); RowIdx++) {
			PJsonVal RowJson = TJsonVal::NewArr();

			for (int ColIdx = 0; ColIdx < JumpMat.GetCols(); ColIdx++) {
				RowJson->AddToArr(JumpMat(RowIdx, ColIdx));
			}

			JumpMatJson->AddToArr(RowJson);
		}

		PJsonVal HoldingTmJson = TJsonVal::NewArr();
		for (int i = 0; i < HoldingTmV.Len(); i++) {
			HoldingTmJson->AddToArr(HoldingTmV[i]);
		}

		LevelJsonVal->AddToObj("height", CurrHeight);
		LevelJsonVal->AddToObj("states", StateJsonV);
		LevelJsonVal->AddToObj("currentState", CurrStateIdx);
		LevelJsonVal->AddToObj("jumpMatrix", JumpMatJson);
		LevelJsonVal->AddToObj("holdingTimes", HoldingTmJson);

		Result->AddToArr(LevelJsonVal);
	} while (CurrHeight < MxStateHeight);

	return Result;
}

void THierarchCtmc::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
	// partition the input space
	TIntV AssignV;	Clust->Apply(X, AssignV, 10000);
	// initialize intensities
	MChain->Init(Clust->GetClusts(), AssignV, RecTmV);
	Hierarch->Init(Clust->GetCentroidMat());

}

#include "mc.h"

using namespace TMc;

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
TClust::TClust(const TRnd& _Rnd, const PNotify& _Notify):
	Rnd(_Rnd),
	CentroidMat(),
	CentroidDistStatV(),
	Notify(_Notify) {}

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
// K-Means
TFullKMeans::TFullKMeans(const int& _K, const TRnd& _Rnd, const PNotify& _Notify):
		TClust(_Rnd, _Notify),
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
	AssignV = AssignIdxV.GetIntVec();

	return CentroidMat;
}

//////////////////////////////////////////////////
// DPMeans
TDpMeans::TDpMeans(const TFlt& _Lambda, const TInt& _MinClusts, const TInt& _MaxClusts, const TRnd& _Rnd, const PNotify& _Notify):
		TClust(_Rnd, _Notify),
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

				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Max distance to centroid: %.3f, number of clusters: %d...", sqrt(MaxDist), GetClusts());
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
	AssignV = AssignIdxV.GetIntVec();

	return CentroidMat;
}

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

	const TVector& EigValsSqrt = Svd.Val2.Map([&](const TFlt& Val) { return abs(Val); });
	const TFullMatrix& V = Svd.Val3;

	TFullMatrix X_d = V*TFullMatrix::Diag(EigValsSqrt);

	X_d.Transpose();

	return X_d;
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
THierarch::THierarch(const PNotify& _Notify):
		HierarchV(),
		StateHeightV(),
		MxHeight(TFlt::Mn),
		StateCoordV(),
		NLeafs(0),
		Notify(_Notify) {}

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

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "\nHierarchy:\n%s", TStrUtil::GetStr(TempHierarchV, ",").CStr());

	while (true) {
		const int ParentIdx = TempHierarchV[AncestorIdx];

		if (ParentIdx == AncestorIdx || StateHeightV[ParentIdx] > Height) { break; }

		AncestorIdx = ParentIdx;
	}

	return AncestorIdx;
}

void THierarch::GetStateSetsAtHeight(const double& Height, TIntV& StateIdV, TVec<TIntV>& JoinedStateVV) const {
	TIntIntVH StateSubStateH;	GetAncSuccH(Height, StateSubStateH);

	StateIdV.Gen(StateSubStateH.Len());
	JoinedStateVV.Gen(StateSubStateH.Len());

	int i = 0;
	int KeyId = StateSubStateH.FFirstKeyId();
	while (StateSubStateH.FNextKeyId(KeyId)) {
		const int StateIdx = StateSubStateH.GetKey(KeyId);

		if (StateSubStateH[KeyId].Empty()) {
			JoinedStateVV[i].Add(StateIdx);
		} else {
//			const TIntV& SubStateV = StateSubStateH.GetDat(StateIdx);
//			for (int j = 0; j < SubStateV.Len(); j++) {
//				JoinedStateVV[i].Add(SubStateV[j]);
//			}
			JoinedStateVV[i] = StateSubStateH[KeyId];
		}

		StateIdV[i] = StateIdx;
//		HIdxToStateIdxH.AddDat(StateIdx, i);

		i++;
	}
}

void THierarch::GetUniqueHeightV(TFltV& UniqueHeightV) const {
	const int NStates = GetStates();

	TFltSet UsedHeightSet;
	for (int i = 0; i < NStates; i++) {
		if (!UsedHeightSet.IsKey(StateHeightV[i])) {
			UniqueHeightV.Add(StateHeightV[i]);
			UsedHeightSet.AddKey(StateHeightV[i]);
		}
	}
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
	for (int ColIdx = 0; ColIdx < CoordMat.GetCols(); ColIdx++) {
		StateCoordV[ColIdx].Val1 = CoordMat(0, ColIdx);
		StateCoordV[ColIdx].Val2 = CoordMat(1, ColIdx);
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
// Abstract Markov Chain
TMChain::TMChain(const PNotify& _Notify):
		NStates(-1),
		CurrStateIdx(-1),
		Notify(_Notify) {}

void TMChain::Init(const int& _NStates, const TIntV& StateAssignV, const TUInt64V& TmV) {
	NStates = _NStates;

	// initialize statistics
	InitStats(NStates);

	// update states
	const uint64 NRecs = TmV.Len();
	for (uint64 i = 0; i < NRecs; i++) {
		OnAddRec(StateAssignV[i], TmV[i]);
	}
}

void TMChain::Save(TSOut& SOut) const {
	TInt(NStates).Save(SOut);
	TInt(CurrStateIdx).Save(SOut);
}

void TMChain::Load(TSIn& SIn) {
	NStates = TInt(SIn);
	CurrStateIdx = TInt(SIn);
}

void TMChain::OnAddRec(const int& StateIdx, const uint64& RecTm) {
	// call child method
	AbsOnAddRec(StateIdx, RecTm);
	// set current state
	CurrStateIdx = StateIdx;
}

/////////////////////////////////////////////////////////////////
// Discrete time Markov Chain
TDtMChain::TDtMChain(const PNotify& _Notify):
		TMChain(_Notify),
		JumpCountMat() {}

void TDtMChain::Save(TSOut& SOut) const {
	TMChain::Save(SOut);
	JumpCountMat.Save(SOut);
}

void TDtMChain::Load(TSIn& SIn) {
	TMChain::Load(SIn);
	JumpCountMat.Load(SIn);
}

TFullMatrix TDtMChain::GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const {
	TFullMatrix Result(JoinedStateVV.Len(), JoinedStateVV.Len());

	const TFullMatrix PMat = GetTransitionMat();
	const TVector StatDist = GetStatDist();

	for (int JoinState1Idx = 0; JoinState1Idx < JoinedStateVV.Len(); JoinState1Idx++) {
		const TIntV& JoinState1 = JoinedStateVV[JoinState1Idx];
		for (int JoinState2Idx = 0; JoinState2Idx < JoinedStateVV.Len(); JoinState2Idx++) {
			const TIntV& JoinState2 = JoinedStateVV[JoinState2Idx];

			// the transition probability from set Ai to Aj can be
			// calculated as: p_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} p_{k,l}} {\sum_{k \in A_i} \pi_k}

			TFlt Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					SumK += PMat(StateK,StateL);
				}

				Sum += StatDist[JoinState1[k]]*SumK;
				SumP += StatDist[JoinState1[k]];
			}
			Result(JoinState1Idx, JoinState2Idx) = Sum / SumP;
		}
	}

	return Result;
}

void TDtMChain::GetLikelyFutureStateV(const TVec<TIntV>& JoinedStateVV, const int& CurrState, const int& NFutStates,
		TIntV& FutStateIdV, TFltV& FutStateProbV) const {
	const int NFStates = TMath::Mn(NFutStates, JoinedStateVV.Len()-1);

	TVector ProbVec = GetTransitionMat(JoinedStateVV).GetRow(CurrState);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbVec.Len(); j++) {
			if (j != CurrState && !TakenIdxSet.IsKey(j) && ProbVec[j] > MxProb) {
				MxProb = ProbVec[j];
				MxIdx = j;
			}
		}

		// if we exclude the current state than the probability that j will be the future state
		// is sum_{k=0}^{inf} p_{ii}^k*P_{ij} = p_{ij} / (1 - p_{ii})
		const double Prob = ProbVec[MxIdx] / (1 - ProbVec[CurrState]);

		if (Prob <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		FutStateIdV.Add(MxIdx);
		FutStateProbV.Add(ProbVec[MxIdx] / (1 - ProbVec[CurrState]));
	}
}

TFullMatrix TDtMChain::GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& TimeSteps) const {
	const int Steps = (int) TimeSteps;
	EAssertR(Steps >= 0, "Probs for past states not implemented!");

	return GetTransitionMat(JoinedStateVV)^Steps;
}

void TDtMChain::InitStats(const int& NStates) {
	JumpCountMat = TFullMatrix(NStates, NStates);
}

void TDtMChain::AbsOnAddRec(const int& StateIdx, const uint64& RecTm) {
	// update jump stats
	if (CurrStateIdx != -1) {
		JumpCountMat(CurrStateIdx, StateIdx)++;
	}

	if (StateIdx != CurrStateIdx) {
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Jumped to state %d", StateIdx);
	}
}

TFullMatrix TDtMChain::GetTransitionMat() const {
	TFullMatrix Result(NStates, NStates);

	#pragma omp for
	for (int RowIdx = 0; RowIdx < NStates; RowIdx++) {
		double Count = JumpCountMat.RowSum(RowIdx);


		for (int ColIdx = 0; ColIdx < NStates; ColIdx++) {
			Result(RowIdx, ColIdx) = JumpCountMat(RowIdx, ColIdx) / Count;
		}
	}

	return Result;
}

TVector TDtMChain::GetStatDist(const TFullMatrix& PMat) const {
	if (PMat.Empty()) { return TVector(0); }

	TVector EigenVec(PMat.GetRows());
	TNumericalStuff::GetEigenVec(PMat.GetT().GetMat(), 1.0, EigenVec.Vec);

	return EigenVec /= EigenVec.Sum();
}

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
const uint64 TCtMChain::TU_SECOND = 1000;
const uint64 TCtMChain::TU_MINUTE = TU_SECOND*60;
const uint64 TCtMChain::TU_HOUR = TU_MINUTE*60;
const uint64 TCtMChain::TU_DAY = TU_HOUR*24;

TCtMChain::TCtMChain(const uint64& _TimeUnit, const double& _DeltaTm, const PNotify& _Notify):
		TMChain(_Notify),
		QMatStats(),
		DeltaTm(_DeltaTm),
		TimeUnit(_TimeUnit),
		PrevJumpTm(-1) {}

void TCtMChain::Save(TSOut& SOut) const {
	TMChain::Save(SOut);
	QMatStats.Save(SOut);
	TUInt64(TimeUnit).Save(SOut);
	TUInt64(PrevJumpTm).Save(SOut);
}

void TCtMChain::Load(TSIn& SIn) {
	TMChain::Load(SIn);
	QMatStats.Load(SIn);
	TimeUnit = TUInt64(SIn);
	PrevJumpTm = TUInt64(SIn);
}

TVector TCtMChain::GetStatDist() const {
	// returns the stationary distribution

	// Norris: Markov Chains states:
	// Let Q be a Q-matrix with jump matrix Pi and let lambda be a measure,
	// than the following are equivalent
	// 1) lambda is invariant
	// 2) mu*Pi = mu where mu_i = lambda_i / q_i, where q_i = -q_ii

	TFullMatrix QMat = GetQMatrix();	// transition rate matrix
	TFullMatrix JumpMat = GetJumpMatrix(QMat);

	// find the eigenvector of the jump matrix with eigen value 1
	TVector EigenVec(QMat.GetRows());
	TNumericalStuff::GetEigenVec(JumpMat.GetT().GetMat(), 1.0, EigenVec.Vec);

	// divide the elements by q_i
	for (int i = 0; i < QMat.GetRows(); i++) {
		EigenVec[i] /= -QMat(i,i);
	}

	// normalize to get a distribution
	return EigenVec /= EigenVec.Sum();
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
	TFullMatrix Result(JoinedStateVV.Len(), JoinedStateVV.Len());

	const TFullMatrix QMat = GetQMatrix();
	const TVector StatDist = GetStatDist();

	for (int JoinState1Idx = 0; JoinState1Idx < JoinedStateVV.Len(); JoinState1Idx++) {
		const TIntV& JoinState1 = JoinedStateVV[JoinState1Idx];
		for (int JoinState2Idx = 0; JoinState2Idx < JoinedStateVV.Len(); JoinState2Idx++) {
			const TIntV& JoinState2 = JoinedStateVV[JoinState2Idx];

			// the transition probability from set Ai to Aj can be
			// calculated as: q_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} q_{k,l}} {\sum_{k \in A_i} \pi_k}

			TFlt Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					SumK += QMat(StateK,StateL);
				}

				Sum += StatDist[JoinState1[k]]*SumK;
				SumP += StatDist[JoinState1[k]];
			}
			Result(JoinState1Idx, JoinState2Idx) = Sum / SumP;
		}
	}

	return Result;
}

TVector TCtMChain::GetStateSizeV(const TVec<TIntV>& JoinedStateVV) const {
	return GetHoldingTimeV(GetQMatrix(JoinedStateVV));
}

TFullMatrix TCtMChain::GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const {
	return GetJumpMatrix(GetQMatrix(JoinedStateVV));
}

void TCtMChain::GetLikelyFutureStateV(const TVec<TIntV>& JoinedStateVV, const int& CurrState,
		const int& NFutStates, TIntV& FutStateIdV, TFltV& FutStateProbV) const {
	const int NFStates = TMath::Mn(NFutStates, JoinedStateVV.Len()-1);

	TVector ProbVec = GetJumpMatrix(JoinedStateVV).GetRow(CurrState);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbVec.Len(); j++) {
			if (j != CurrState && !TakenIdxSet.IsKey(j) && ProbVec[j] > MxProb) {
				MxProb = ProbVec[j];
				MxIdx = j;
			}
		}

		if (ProbVec[MxIdx] <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		FutStateIdV.Add(MxIdx);
		FutStateProbV.Add(ProbVec[MxIdx]);
	}
}

TFullMatrix TCtMChain::GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& Tm) const {
	const int Dim = JoinedStateVV.Len();

	const TFullMatrix QMat = GetQMatrix(JoinedStateVV);

	const double QMatNorm = QMat.FromNorm();
	const double Dt = TMath::Mn(DeltaTm / QMatNorm, DeltaTm);

	const int Steps = (int) ceil(Tm / Dt);

	EAssertR(Steps >= 0, "Probs for past states not implemented!");

	return (TFullMatrix::Identity(Dim) + QMat*Dt)^Steps;
}

void TCtMChain::InitStats(const int& NStates) {
	// initialize a matrix holding the number of measurements and the sum
	QMatStats.Gen(NStates, 0);
	for (int i = 0; i < NStates; i++) {
		QMatStats.Add(TUInt64FltPrV(NStates, NStates));
	}
}

void TCtMChain::AbsOnAddRec(const int& StateIdx, const uint64& RecTm) {
	// update intensities
	if (CurrStateIdx != -1 && StateIdx != CurrStateIdx) {
		// the state has changed
		if (PrevJumpTm != TUInt64::Mx) {
			uint64 HoldingTm = RecTm - PrevJumpTm;
			QMatStats[CurrStateIdx][StateIdx].Val1++;
			QMatStats[CurrStateIdx][StateIdx].Val2 += (double) HoldingTm / TimeUnit;
			printf("Updated intensity: prev state: %d, curr state: %d\n", CurrStateIdx, StateIdx);
		}
		PrevJumpTm = RecTm;
	} else if (CurrStateIdx == -1) {
		PrevJumpTm = RecTm;
	}
}

/////////////////////////////////////////////////////////////////
// Hierarchical continous time Markov Chain
THierarchCtmc::THierarchCtmc(const PClust& _Clust, const PMChain& _MChain,
		const PHierarch& _Hierarch, const PNotify& _Notify):
		Clust(_Clust),
		MChain(_MChain),
		Hierarch(_Hierarch),
		Notify(_Notify) {}

void THierarchCtmc::Save(TSOut& SOut) const {
	Notify->OnNotify(TNotifyType::ntInfo, "THierarchCtmc::Save: saving to stream ...");

	Clust->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
}

void THierarchCtmc::Load(TSIn& SIn) {
	Notify->OnNotify(TNotifyType::ntInfo, "THierarchCtmc::Load: loading from stream ...");

	Clust->Load(SIn);
	MChain->Load(SIn);
	Hierarch->Load(SIn);
}

PJsonVal THierarchCtmc::SaveJson() const {
	Notify->OnNotify(TNotifyType::ntInfo, "THierarchCtmc::SaveJson: saving JSON ...");

	PJsonVal Result = TJsonVal::NewArr();

	// we need to build a hierarchy and model state transitions
	// on each level of the hierarchy

	// variables
	TIntV FutureStateIdV;
	TFltV FutureStateProbV;
	TVec<TIntV> JoinedStateVV;
	TIntV StateIdV;
	TIntIntH HIdxToStateIdxH;

	TFltV UniqueHeightV;	Hierarch->GetUniqueHeightV(UniqueHeightV);
	UniqueHeightV.Sort(true);	// sort ascending

	// go through all the heights except the last one, which is not interesting
	// since it is only one state
	for (int HeightIdx = 0; HeightIdx < UniqueHeightV.Len()-1; HeightIdx++) {
		const double CurrHeight = UniqueHeightV[HeightIdx];

		PJsonVal LevelJsonVal = TJsonVal::NewObj();

		StateIdV.Clr();
		JoinedStateVV.Clr();
		HIdxToStateIdxH.Clr();

		// get the states on this level
		Hierarch->GetStateSetsAtHeight(CurrHeight, StateIdV, JoinedStateVV);

		for (int i = 0; i < StateIdV.Len(); i++) {
			HIdxToStateIdxH.AddDat(StateIdV[i], i);
		}

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "States at height %.3f:", CurrHeight);
		for (int i = 0; i < JoinedStateVV.Len(); i++) {
			Notify->OnNotify(TNotifyType::ntInfo, TStrUtil::GetStr(JoinedStateVV[i], ","));
		}

		// get the index of the current state at this height
		const int CurrStateIdx = Hierarch->GetAncestorAtHeight(MChain->GetCurrStateIdx(), CurrHeight);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// chindren
		TFullMatrix TransitionMat = MChain->GetTransitionMat(JoinedStateVV);
		TVector StateSizeV = MChain->GetStateSizeV(JoinedStateVV);

		FutureStateIdV.Clr();
		FutureStateProbV.Clr();
		MChain->GetLikelyFutureStateV(JoinedStateVV, HIdxToStateIdxH.GetDat(CurrStateIdx), 3, FutureStateIdV, FutureStateProbV);	// TODO 3 is hardcoded

		// construct state JSON
		PJsonVal StateJsonV = TJsonVal::NewArr();
		for (int i = 0; i < StateIdV.Len(); i++) {
			const int StateId = StateIdV[i];
			const TFltPr& StateCoords = Hierarch->GetStateCoords(StateId);

			PJsonVal StateJson = TJsonVal::NewObj();
			StateJson->AddToObj("id", StateId);
			StateJson->AddToObj("x", StateCoords.Val1);
			StateJson->AddToObj("y", StateCoords.Val2);
			StateJson->AddToObj("size", StateSizeV[i]);

			StateJsonV->AddToArr(StateJson);
		}

		// construct future states
		PJsonVal FutureStateJsonV = TJsonVal::NewArr();
		for (int i = 0; i < FutureStateIdV.Len(); i++) {
			PJsonVal FutureStateJson = TJsonVal::NewObj();

			FutureStateJson->AddToObj("id", StateIdV[FutureStateIdV[i]]);
			FutureStateJson->AddToObj("prob", FutureStateProbV[i]);

			FutureStateJsonV->AddToArr(FutureStateJson);
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
		LevelJsonVal->AddToObj("currentState", CurrStateIdx);
		LevelJsonVal->AddToObj("futureStates", FutureStateJsonV);
		LevelJsonVal->AddToObj("transitions", JumpMatJson);

		Result->AddToArr(LevelJsonVal);
	}

	return Result;
}

void THierarchCtmc::GetFutStateProbs(const double& Height, const int& StartState,
		const double& Tm, TFltV& ProbV) const {
	try {
		TIntV StateIdV;
		TVec<TIntV> JoinedStateVV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, JoinedStateVV);

		ProbV = MChain->GetFutureProbV(JoinedStateVV, StartState, Tm).GetVec();
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::GetFutStateProbs: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetTransitionModel(const double& Height, TFltVV& Mat) const {
	TIntV StateIdV;
	TVec<TIntV> JoinedStateVV;
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, JoinedStateVV);
	Mat = MChain->GetModel(JoinedStateVV).GetMat();
}

void THierarchCtmc::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
	// partition the input space
	TIntV AssignV;	Clust->Apply(X, AssignV, 10000);
	// initialize intensities
	MChain->Init(Clust->GetClusts(), AssignV, RecTmV);
	Hierarch->Init(Clust->GetCentroidMat());
}

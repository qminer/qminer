/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TClustering;
using namespace TDist;

//////////////////////////////////////////////////////
// Distance measures
//void TEuclDist::UpdateFtrVV(const TFltVV& FtrVV) {
//	TLinAlg::GetColNorm2V(FtrVV, FtrDimV);
//}
//
//void TEuclDist::UpdateCentroidVV(const TFltVV& CentroidVV) {
//	TLinAlg::GetColNorm2V(CentroidVV, CentroidDimV);
//}
//
//void TEuclDist::GetDistPropVV(const TFltVV& FtrVV, const TFltVV& CentroidVV, TFltVV& DistVV) const {
//	GetDist2VV(CentroidVV, FtrVV, CentroidDimV, FtrDimV, DistVV);
//}

void TEuclDist::UpdateNormX2(const TFltVV& FtrVV, TFltV& NormX2) const {
	TLinAlg::GetColNorm2V(FtrVV, NormX2);
}

void TEuclDist::UpdateNormC2(const TFltVV& CentroidVV, TFltV& NormC2) const {
	TLinAlg::GetColNorm2V(CentroidVV, NormC2);
}

void TEuclDist::GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const {
	// return (CentroidMat.ColNorm2V() - (x*C*2) + TVector::Ones(GetClusts(), false) * NormX2).Sqrt();
	// 1) squared norm of X
	const double NormX2 = TLinAlg::Norm2(FtrV);

	// 2) Result <- CentroidMat.ColNorm2V()
	TLinAlg::GetColNorm2V(CentroidVV, DistV);

	// 3) x*C
	TFltV xC;	TLinAlg::MultiplyT(CentroidVV, FtrV, xC);

	// 4) <- Result = Result - 2*x*C + ones(clusts, 1)*|x|^2
	for (int i = 0; i < DistV.Len(); i++) {
		DistV[i] += NormX2 - 2*xC[i];
		AssertR(DistV[i] > -1e-8, "Distance lower than numerical error!");
		if (DistV[i] < 0) { DistV[i] = 0; }
		DistV[i] = sqrt(DistV[i]);
	}
}

void TEuclDist::GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const {
	GetDist2VV(X, Y, D);
	double Val;
	for (int RowN = 0; RowN < D.GetRows(); RowN++) {
		for (int ColN = 0; ColN < D.GetCols(); ColN++) {
			Val = D(RowN, ColN);
			AssertR(Val > -1e-8, "Distance lower than numerical error!");
			if (Val < 0) { Val = 0; }
			D.PutXY(RowN, ColN, TMath::Sqrt(Val));
		}
	}
}

void TEuclDist::StaticGetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) {
	TFltV NormX2;	TLinAlg::GetColNorm2V(X, NormX2);
	TFltV NormY2;	TLinAlg::GetColNorm2V(Y, NormY2);

	StaticGetDist2VV(X, Y, NormX2, NormY2, D);
}



void TEuclDist::StaticGetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
		const TFltV& NormY2, TFltVV& D) {
	//	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
	// 1) X'Y
	TLinAlg::MultiplyT(X, Y, D);
	// 2) (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2)
	const int Rows = D.GetRows();
	const int Cols = D.GetCols();

	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			D.PutXY(RowN, ColN, NormX2[RowN] - 2*D(RowN, ColN) + NormY2[ColN]);
		}
	}
}

///////////////////////////////////////////
// TAbsKMeans
TAbsKMeans::TAbsKMeans(const TRnd& _Rnd, TDist::TDist* _Dist):
		CentroidVV(),
		Dist(_Dist),
		Rnd(_Rnd) {}

TAbsKMeans::TAbsKMeans(TSIn& SIn):
		CentroidVV(SIn),
		// TODO Dist
		Rnd(SIn) {}

void TAbsKMeans::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	CentroidVV.Save(SOut);
	// TODO Dist
	Rnd.Save(SOut);
}

TAbsKMeans* TAbsKMeans::Load(TSIn& SIn) {
	TStr Type(SIn);
	if (Type == "kmeans") {
		return new TDnsKMeans(SIn);
	} else if (Type == "dpmeans") {
		return new TDpMeans(SIn);
	} else {
		throw TExcept::New("Invalid clustering type: " + Type);
	}
}

void TAbsKMeans::GetCentroid(const int& ClustN, TFltV& FtrV) const {
	EAssert(0 <= ClustN && ClustN < GetClusts());
	CentroidVV.GetCol(ClustN, FtrV);
}

void TAbsKMeans::Assign(const TFltVV& FtrVV, TIntV& AssignV) const {
	TFltVV DistVV;	Dist->GetDist2VV(CentroidVV, FtrVV, DistVV);
	TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

double TAbsKMeans::GetDist(const int& ClustN, const TFltV& Pt) const {
	TFltV Centroid;	CentroidVV.GetCol(ClustN, Centroid);
	return TLinAlg::EuclDist(Centroid, Pt);
}

void TAbsKMeans::GetCentroidDistV(const TFltV& FtrV, TFltV& DistV) const {
	Dist->GetDistV(CentroidVV, FtrV, DistV);
}

void TAbsKMeans::GetDistVV(const TFltVV& FtrVV, TFltVV& DistVV) const {
	Dist->GetDistVV(CentroidVV, FtrVV, DistVV);
}

void TAbsKMeans::UpdateCentroids(const TFltVV& FtrVV, const int& NInst, TIntV& AssignV,
		const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TFltVV& TempDxKV,
		TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2) {

	const int K = CentroidVV.GetCols();

	// I. create a sparse matrix (coordinate representation) that encodes the closest centroids
	TSparseColMatrix AssignMat(NInst, K);

	bool AllClustsFull;
	do {
		AllClustsFull = true;

		TSparseOps<TInt,TFlt>::CoordinateCreateSparseColMatrix(RangeN, AssignV, OnesN, AssignMat.ColSpVV, K);

		// II. compute the number of points that belong to each centroid, invert
		AssignMat.MultiplyT(OnesN, TempK);

		// invert
		for (int ClustN = 0; ClustN < K; ClustN++) {
			if (TempK[ClustN] == 0.0) {	// don't allow empty clusters
				// select a random point and create a new centroid from it
				const int RndRecN = Rnd.GetUniDevInt(FtrVV.GetCols());
				TFltV RndRecFtrV;	GetCol(FtrVV, RndRecN, RndRecFtrV);
				CentroidVV.SetCol(ClustN, RndRecFtrV);
				Dist->UpdateNormC2(CentroidVV, NormC2);
				Assign(FtrVV, NormX2, NormC2, AssignV);
				AllClustsFull = false;
				break;
			}
			TempK[ClustN] = 1.0 / (TempK[ClustN] + 1.0);
		}
	} while (!AllClustsFull);


	// III. compute the centroids
	// compute: CentroidMat = ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag;
	TLAMisc::Diag(TempK, TempKxKSpVV);

	// 1) FtrVV * AssignIdxMat
	TLinAlg::Multiply(FtrVV, AssignMat.ColSpVV, TempDxKV);
	// 2) (FtrVV * AssignIdxMat) + CentroidMat
	TLinAlg::LinComb(1, TempDxKV, 1, CentroidVV, TempDxKV);
	// 3) ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag
	TLinAlg::Multiply(TempDxKV, TempKxKSpVV, CentroidVV);
}

void TAbsKMeans::SelectInitCentroids(const TFltVV& FtrVV, const int& K) {
	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();

	EAssertR(NInst >= K, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

	// generate k random elements
	TFltV PermV(NInst);	TLAUtil::Range(NInst, PermV);
	TIntV CentroidNV(K);

	double Temp;
	for (int i = 0; i < K; i++) {
		const int SwapIdx = Rnd.GetUniDevInt(i, NInst-1);

		// swap
		Temp = PermV[SwapIdx];
		PermV[SwapIdx] = PermV[i];
		PermV[i] = Temp;

		CentroidNV[i] = (int) PermV[i];
	}

	// construct the centroid matrix
	CentroidVV.Gen(Dim, K);
	for (int i = 0; i < K; i++) {
		const int ColN = CentroidNV[i];
		for (int RowN = 0; RowN < Dim; RowN++) {
			CentroidVV.PutXY(RowN, i, FtrVV(RowN, ColN));
		}
	}
}

void TAbsKMeans::Assign(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TIntV& AssignV) const {
	TFltVV DistVV;	Dist->GetDist2VV(CentroidVV, FtrVV, NormC2, NormX2, DistVV);
	TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

///////////////////////////////////////////
// K-Means
TDnsKMeans::TDnsKMeans(const int& _K, const TRnd& Rnd):
		TAbsKMeans(Rnd),
		K(_K) {}

TDnsKMeans::TDnsKMeans(TSIn& SIn):
		TAbsKMeans(SIn),
		K(SIn) {}

void TDnsKMeans::Save(TSOut& SOut) const {
	TAbsKMeans::Save(SOut);
	K.Save(SOut);
}

void TDnsKMeans::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
		const int& MaxIter, const PNotify& Notify) {
	EAssertR(K <= NInst, "Matrix should have more columns than K!");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing KMeans ...");

	// assignment vectors
	TIntV AssignIdxV, OldAssignIdxV;
	TIntV* AssignIdxVPtr = &AssignIdxV;
	TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
	TIntV* Temp;

	// constant reused variables
	TFltV OnesN;			TLAUtil::Ones(NInst, OnesN);
	TFltV NormX2;			Dist->UpdateNormX2(FtrVV, NormX2);
	TIntV RangeN(NInst);	TLAUtil::Range(NInst, RangeN);

	// reused variables
	TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
	TFltV NormC2(K);
	TFltV TempK(K);						// (dimension k)
	TFltVV TempDxK(Dim, K);				// (dimension d x k)
	TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

	// select initial centroids
	SelectInitCentroids(FtrVV, K);

	// do the work
	for (int i = 0; i < MaxIter; i++) {
		if (i % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// get the distance of each of the points to each of the centroids
		// and assign the instances
		Dist->UpdateNormC2(CentroidVV, NormC2);
		Dist->GetDist2VV(CentroidVV, FtrVV, NormC2, NormX2, ClustDistVV);
		TLinAlg::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

		// if the assignment hasn't changed then terminate the loop
		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the means
		UpdateCentroids(FtrVV, NInst, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV, NormX2, NormC2);

		// swap the old and new assign vectors
		Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}
}

///////////////////////////////////////////
// DPMeans
TDpMeans::TDpMeans(const TFlt& _Lambda, const TInt& _MnClusts, const TInt& _MxClusts,
			const TRnd& Rnd):
		TAbsKMeans(Rnd),
		Lambda(_Lambda),
		MnClusts(_MnClusts),
		MxClusts(_MxClusts) {

	EAssertR(MnClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
	EAssertR(MxClusts >= MnClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

TDpMeans::TDpMeans(TSIn& SIn):
		TAbsKMeans(SIn),
		Lambda(SIn),
		MnClusts(SIn),
		MxClusts(SIn) {}

void TDpMeans::Save(TSOut& SOut) const {
	TAbsKMeans::Save(SOut);
	Lambda.Save(SOut);
	MnClusts.Save(SOut);
	MxClusts.Save(SOut);
}

void TDpMeans::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
		const int& MaxIter, const PNotify& Notify) {
	EAssertR(MnClusts <= NInst, "Matrix should have more rows then the min number of clusters!");
	EAssertR(MnClusts <= MxClusts, "Minimum number of cluster should be less than the maximum.");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing DPMeans ...");

	const double LambdaSq = Lambda*Lambda;

	int K = MnClusts;

	// assignment vectors and their pointers, so we don't copy
	TIntV AssignIdxV, OldAssignIdxV;
	TIntV* AssignIdxVPtr = &AssignIdxV;
	TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
	TIntV* Temp;

	// select initial centroids
	SelectInitCentroids(FtrVV, MnClusts);

	// const variables, reused throughtout the procedure
	TFltV OnesN;			TLAUtil::Ones(NInst, OnesN);
	TFltV NormX2;			Dist->UpdateNormX2(FtrVV, NormX2);
	TIntV RangeN(NInst);	TLAUtil::Range(NInst, RangeN);


	// temporary reused variables
	TFltV FtrV;							// (dimension d)
	TFltV MinClustDistV;				// (dimension n)
	TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
	TFltV NormC2(K);					// (dimension k)
	TFltV TempK(K);						// (dimension k)
	TFltVV TempDxK(Dim, K);				// (dimension d x k)
	TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

	int i = 0;
	while (i++ < MaxIter) {
		if (i % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// compute the distance matrix to all the centroids and assignments
		Dist->UpdateNormC2(CentroidVV, NormC2);
		Dist->GetDist2VV(CentroidVV, FtrVV, NormC2, NormX2, ClustDistVV);
		TLinAlg::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

		// check if we need to increase the number of centroids
		if (K < MxClusts) {
			TLinAlg::GetColMinV(ClustDistVV, MinClustDistV);

			const int NewCentrIdx = TLAUtil::GetMaxIdx(MinClustDistV);
			const double MaxDist = MinClustDistV[NewCentrIdx];

			if (MaxDist > LambdaSq) {
				K++;
				FtrVV.GetCol(NewCentrIdx, FtrV);
				CentroidVV.AddCol(FtrV);
				ClustDistVV.AddXDim();
				NormC2.Add(0);
				TempK.Add(0);
				TempDxK.AddYDim();
				TempKxKSpVV.Gen(K);
				(*AssignIdxVPtr)[NewCentrIdx] = K-1;
				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Max distance to centroid: %.3f, number of clusters: %d ...", TMath::Sqrt(MaxDist), K);
			}
		}

		// check if converged
		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the centroids
		UpdateCentroids(FtrVV, NInst, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV, NormX2, NormC2);

		// swap old and new assign vectors
		Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}
}

///////////////////////////////////////////
// Agglomerative clustering - average link
void TAvgLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = (DistMat(MnI, i)*ItemCountV[MnI] + DistMat(MnJ, i)*ItemCountV[MnJ]) / (ItemCountV[MnI] + ItemCountV[MnJ]);
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

///////////////////////////////////////////
// Agglomerative clustering - complete link
void TCompleteLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mx(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

///////////////////////////////////////////
// Agglomerative clustering - single link
void TSingleLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mn(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

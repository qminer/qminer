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
void TEuclDist::GetDist(const TFltVV& X, const TFltVV& Y, TFltVV& D) {
	GetDist2(X, Y, D);
	for (int RowN = 0; RowN < D.GetRows(); RowN++) {
		for (int ColN = 0; ColN < D.GetCols(); ColN++) {
			D.PutXY(RowN, ColN, TMath::Sqrt(D(RowN, ColN)));
		}
	}
}

void TEuclDist::GetDist2(const TFltVV& X, const TFltVV& Y, TFltVV& D) {
	TFltV NormX2;	TLinAlg::GetColNorm2V(X, NormX2);
	TFltV NormY2;	TLinAlg::GetColNorm2V(Y, NormY2);

	GetDist2(X, Y, NormX2, NormY2, D);
}

void TEuclDist::GetDist2(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
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
TAbsKMeans::TAbsKMeans(const TRnd& _Rnd):
		Rnd(_Rnd) {}

TAbsKMeans::TAbsKMeans(TSIn& SIn):
		CentroidVV(SIn),
		Rnd(SIn) {}

void TAbsKMeans::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	CentroidVV.Save(SOut);
	Rnd.Save(SOut);
}

PDnsKMeans TAbsKMeans::Load(TSIn& SIn) {
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
	TFltV NormX2;	TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TFltV NormC2;	TLinAlg::GetColNorm2V(CentroidVV, NormC2);

	Assign(FtrVV, NormX2, NormC2, AssignV);
}

double TAbsKMeans::GetDist(const int& ClustN, const TFltV& Pt) const {
	TFltV Centroid;	CentroidVV.GetCol(ClustN, Centroid);
	return TLinAlg::EuclDist(Centroid, Pt);
}

void TAbsKMeans::GetCentroidDistV(const TFltV& FtrVV, TFltV& DistV) const {
	// return (CentroidMat.ColNorm2V() - (x*C*2) + TVector::Ones(GetClusts(), false) * NormX2).Sqrt();
	// 1) squared norm of X
	const double NormX2 = TLinAlg::Norm2(FtrVV);

	// 2) Result <- CentroidMat.ColNorm2V()
	TLinAlg::GetColNorm2V(CentroidVV, DistV);

	// 3) x*C
	TFltV xC;	TLinAlg::MultiplyT(CentroidVV, FtrVV, xC);

	// 4) <- Result = Result - 2*x*C + ones(clusts, 1)*|x|^2
	for (int i = 0; i < DistV.Len(); i++) {
		DistV[i] += NormX2 - 2*xC[i];
		DistV[i] = sqrt(DistV[i]);
	}
}

void TAbsKMeans::GetDistVV(const TFltVV& FtrVV, TFltVV& DistVV) const {
	TFltV NormX2;	TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TFltV NormC2;	TLinAlg::GetColNorm2V(CentroidVV, NormC2);

	GetDistMat2(FtrVV, NormX2, NormC2, DistVV);

	const int Rows = DistVV.GetRows();
	const int Cols = DistVV.GetCols();

	// compute the square root of each element
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			DistVV.PutXY(RowN, ColN, TMath::Sqrt(DistVV(RowN, ColN)));
		}
	}
}

void TAbsKMeans::UpdateCentroids(const TFltVV& FtrVV, const TIntV& AssignIdxV,
		const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TFltVV& TempDxKV,
		TVec<TIntFltKdV>& TempKxKSpVV) {
	const int NInst = FtrVV.GetCols();
	const int K = CentroidVV.GetCols();

	// I. create a sparse matrix (coordinate representation) that encodes the closest centroids
	TSparseColMatrix AssignIdxMat(NInst, K);
	TSparseOps<TInt,TFlt>::CoordinateCreateSparseColMatrix(RangeN, AssignIdxV, OnesN, AssignIdxMat.ColSpVV, K);

	// II. compute the number of points that belong to each centroid, invert
	AssignIdxMat.MultiplyT(OnesN, TempK);
	// invert
	for (int i = 0; i < K; i++) {
		TempK[i] = 1.0 / (TempK[i] + 1.0);
	}

	// III. compute the centroids
	// compute: CentroidMat = ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag;
	TLAMisc::Diag(TempK, TempKxKSpVV);

	// 1) FtrVV * AssignIdxMat
	TLinAlg::Multiply(FtrVV, AssignIdxMat.ColSpVV, TempDxKV);
	// 2) (FtrVV * AssignIdxMat) + CentroidMat
	TLinAlg::LinComb(1, TempDxKV, 1, CentroidVV, TempDxKV);
	// 3) ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag
	TLinAlg::Multiply(TempDxKV, TempKxKSpVV, CentroidVV);
}

void TAbsKMeans::SelectInitCentroids(const TFltVV& FtrVV, const int& K, TFltVV& CentroidFtrVV,
		TIntV& AssignV) {
	const int Dim = FtrVV.GetRows();
	const int NInst = FtrVV.GetCols();

	EAssertR(NInst >= K, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

	AssignV.Gen(K);

	// generate k random elements
	TFltV PermV(NInst);	TLAUtil::Range(NInst, PermV);
	double Temp;
	for (int i = 0; i < K; i++) {
		const int SwapIdx = Rnd.GetUniDevInt(i, NInst-1);

		// swap
		Temp = PermV[SwapIdx];
		PermV[SwapIdx] = PermV[i];
		PermV[i] = Temp;

		AssignV[i] = (int)PermV[i];
	}

	// construct the centroid matrix
	CentroidFtrVV.Gen(Dim, K);
	for (int i = 0; i < K; i++) {
		const int ColN = AssignV[i];
		for (int RowN = 0; RowN < Dim; RowN++) {
			CentroidFtrVV.PutXY(RowN, i, FtrVV(RowN, ColN));
		}
	}
}

void TAbsKMeans::Assign(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TIntV& AssignV) const {
	TFltVV DistVV;	GetDistMat2(FtrVV, NormX2, NormC2, DistVV);
	TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

void TAbsKMeans::GetDistMat2(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TFltVV& D) const {
	TEuclDist::GetDist2(CentroidVV, FtrVV, NormC2, NormX2, D);
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

void TDnsKMeans::Apply(const TFltVV& FtrVV, const int& MaxIter, const PNotify& Notify) {
	EAssertR(K <= FtrVV.GetRows(), "Matrix should have more rows then k!");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing KMeans ...");

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();

	// select initial centroids
	TIntV AssignIdxV, OldAssignIdxV;
	TIntV* AssignIdxVPtr = &AssignIdxV;
	TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
	TIntV* Temp;

	SelectInitCentroids(FtrVV, K, CentroidVV, OldAssignIdxV);

	// constant reused variables
	TFltV OnesN;			TLAUtil::Ones(NInst, OnesN);
	TFltV NormX2;			TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TIntV RangeN(NInst);	TLAUtil::Range(NInst, RangeN);

	// reused variables
	TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
	TFltV TempK(K);						// (dimension k)
	TFltVV TempDxK(Dim, K);				// (dimension d x k)
	TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

	for (int i = 0; i < MaxIter; i++) {
		if (i % 10000 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// get the distance of each of the points to each of the centroids
		TLinAlg::GetColNorm2V(CentroidVV, TempK);
		GetDistMat2(FtrVV, NormX2, TempK, ClustDistVV);

		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the means
		UpdateCentroids(FtrVV, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV);

		// swap old and new assign vectors
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

void TDpMeans::Apply(const TFltVV& FtrVV, const int& MaxIter, const PNotify& Notify) {
	EAssertR(FtrVV.GetRows() > 0, "The input matrix doesn't have any features!");
	EAssertR(MnClusts <= FtrVV.GetCols(), "Matrix should have more rows then the min number of clusters!");
	EAssertR(MnClusts <= MxClusts, "Minimum number of cluster should be less than the maximum.");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing DPMeans ...");

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();
	const double LambdaSq = Lambda*Lambda;

	int K = MnClusts;

	// assignment vectors and their pointers, so we don't copy
	TIntV AssignIdxV, OldAssignIdxV;
	TIntV* AssignIdxVPtr = &AssignIdxV;
	TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
	TIntV* Temp;

	// select initial centroids
	SelectInitCentroids(FtrVV, MnClusts, CentroidVV, OldAssignIdxV);

	// const variables, reused throughtout the procedure
	TFltV OnesN;			TLAUtil::Ones(NInst, OnesN);
	TFltV NormX2;			TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TIntV RangeN(NInst);	TLAUtil::Range(NInst, RangeN);


	// temporary reused variables
	TFltV FtrV;							// (dimension d)
	TFltV MinClustDistV;				// (dimension n)
	TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
	TFltV TempK(K);						// (dimension k)
	TFltVV TempDxK(Dim, K);				// (dimension d x k)
	TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

	int i = 0;
	while (i++ < MaxIter) {
		if (i % 10 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// add new centroids and compute the distance matrix
		TLinAlg::GetColNorm2V(CentroidVV, TempK);
		GetDistMat2(FtrVV, NormX2, TempK, ClustDistVV);

		// assign
		TLinAlg::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

		// check if we need to increase the number of clusters
		if (K < MxClusts) {
			TLinAlg::GetColMinV(ClustDistVV, MinClustDistV);

			const int NewCentrIdx = TLAUtil::GetMaxIdx(MinClustDistV);
			const double MaxDist = MinClustDistV[NewCentrIdx];

			if (MaxDist > LambdaSq) {
				K++;
				FtrVV.GetCol(NewCentrIdx, FtrV);
				CentroidVV.AddCol(FtrV);
				ClustDistVV.AddXDim();
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

		// recompute the means
		UpdateCentroids(FtrVV, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV);

		// swap old and new assign vectors
		Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}
}

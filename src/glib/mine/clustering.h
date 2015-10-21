/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TDistance {

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TEuclDist {
public:
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static void GetDist(const TFltVV& X, const TFltVV& Y, TFltVV& D);
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static void GetDist2(const TFltVV& X, const TFltVV& Y, TFltVV& D);

	static void GetDist2(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
			const TFltV& NormY2, TFltVV& D);
};

}

namespace TClustering {

///////////////////////////////////////////
// Abstract class that has methods needed be KMeans
//template <typename TDist>
template <class TDist> class TAbsKMeans;
template <class TDist> using PAbsKMeans = TPt<TAbsKMeans<TDist>>;
template <class TDist> class TAbsKMeans {
private:
	TCRef CRef;
public:
	friend class TPt<TAbsKMeans<TDist>>;
protected:
	TFltVV CentroidVV;

	TRnd Rnd;

public:
	TAbsKMeans(const TRnd& Rnd);
	TAbsKMeans(TSIn& SIn);

	virtual ~TAbsKMeans() {}

	virtual void Save(TSOut& SOut) const;
	static PAbsKMeans<TDist> Load(TSIn& SIn);

	int GetClusts() const { return CentroidVV.GetCols(); }
	int GetDim() const { return CentroidVV.GetRows(); }

	// returns the centroid (column) matrix
	const TFltVV& GetCentroidVV() const { return CentroidVV; }
	// returns the n-th centroid
	void GetCentroid(const int& ClustN, TFltV& FtrV) const;

	virtual void Apply(const TFltVV& FtrVV, const int& MaxIter=10000,
			const PNotify& Notify=TNotify::NullNotify) = 0;

	void Assign(const TFltVV& FtrVV, TIntV& AssignV) const;

	// distance methods
	// returns the distance to the specified centroid
	double GetDist(const int& ClustN, const TFltV& FtrV) const;
	// returns the distance to all the centroids
	void GetCentroidDistV(const TFltV& FtrVV, TFltV& DistV) const;

	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
	void GetDistVV(const TFltVV& FtrVV, TFltVV& DistVV) const;

protected:
	// can still optimize
	void UpdateCentroids(const TFltVV& FtrVV, const TIntV& AssignIdxV, const TFltV& OnesN,
			const TIntV& RangeN, TFltV& TempK, TFltVV& TempDxKV,
			TVec<TIntFltKdV>& TempKxKSpVV);
	void SelectInitCentroids(const TFltVV& FtrVV, const int& K);

	void Assign(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TIntV& AssignV) const;

	// returns a matrix of squared distances
	void GetDistMat2(const TFltVV& X, const TFltV& NormX2, const TFltV& NormC2,
			TFltVV& DistMat) const;

	virtual const TStr GetType() const = 0;
};

///////////////////////////////////////////
// K-Means
template <class TDist>
class TDnsKMeans : public TAbsKMeans<TDist> {
private:
	const TInt K;
public:
	TDnsKMeans(const int& K, const TRnd& Rnd=TRnd(0));
	TDnsKMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	void Apply(const TFltVV& FtrVV, const int& MaxIter=10000,
			const PNotify& Notify=TNotify::NullNotify);

protected:
	const TStr GetType() const { return "kmeans"; }
};

///////////////////////////////////////////
// DPMeans
template <class TDist>
class TDpMeans : public TAbsKMeans<TDist> {
private:
	const TFlt Lambda;
	const TInt MnClusts;
	const TInt MxClusts;
public:
	TDpMeans(const TFlt& Lambda, const TInt& MnClusts=1, const TInt& MxClusts=TInt::Mx,
			const TRnd& Rnd=TRnd(0));
	TDpMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFltVV& FtrVV, const int& MaxIter=10000,
			const PNotify& Notify=TNotify::NullNotify);

protected:
	const TStr GetType() const { return "dpmeans"; }
};


///////////////////////////////////////////
// TAbsKMeans
template <class TDist>
TAbsKMeans<TDist>::TAbsKMeans(const TRnd& _Rnd):
		Rnd(_Rnd) {}

template <class TDist>
TAbsKMeans<TDist>::TAbsKMeans(TSIn& SIn):
		CentroidVV(SIn),
		Rnd(SIn) {}

template <class TDist>
void TAbsKMeans<TDist>::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	CentroidVV.Save(SOut);
	Rnd.Save(SOut);
}

template <class TDist>
PAbsKMeans<TDist> TAbsKMeans<TDist>::Load(TSIn& SIn) {
	TStr Type(SIn);
	if (Type == "kmeans") {
		return new TDnsKMeans<TDist>(SIn);
	} else if (Type == "dpmeans") {
		return new TDpMeans<TDist>(SIn);
	} else {
		throw TExcept::New("Invalid clustering type: " + Type);
	}
}

template <class TDist>
void TAbsKMeans<TDist>::GetCentroid(const int& ClustN, TFltV& FtrV) const {
	EAssert(0 <= ClustN && ClustN < GetClusts());
	CentroidVV.GetCol(ClustN, FtrV);
}

template <class TDist>
void TAbsKMeans<TDist>::Assign(const TFltVV& FtrVV, TIntV& AssignV) const {
	TFltV NormX2;	TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TFltV NormC2;	TLinAlg::GetColNorm2V(CentroidVV, NormC2);

	Assign(FtrVV, NormX2, NormC2, AssignV);
}

template <class TDist>
double TAbsKMeans<TDist>::GetDist(const int& ClustN, const TFltV& Pt) const {
	TFltV Centroid;	CentroidVV.GetCol(ClustN, Centroid);
	return TLinAlg::EuclDist(Centroid, Pt);
}

template <class TDist>
void TAbsKMeans<TDist>::GetCentroidDistV(const TFltV& FtrVV, TFltV& DistV) const {
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

template <class TDist>
void TAbsKMeans<TDist>::GetDistVV(const TFltVV& FtrVV, TFltVV& DistVV) const {
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

template <class TDist>
void TAbsKMeans<TDist>::UpdateCentroids(const TFltVV& FtrVV, const TIntV& AssignIdxV,
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

template <class TDist>
void TAbsKMeans<TDist>::SelectInitCentroids(const TFltVV& FtrVV, const int& K) {
	const int Dim = FtrVV.GetRows();
	const int NInst = FtrVV.GetCols();

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

template <class TDist>
void TAbsKMeans<TDist>::Assign(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TIntV& AssignV) const {
	TFltVV DistVV;	GetDistMat2(FtrVV, NormX2, NormC2, DistVV);
	TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

template <class TDist>
void TAbsKMeans<TDist>::GetDistMat2(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
		TFltVV& D) const {
	TDist::GetDist2(CentroidVV, FtrVV, NormC2, NormX2, D);
}

///////////////////////////////////////////
// K-Means
template <class TDist>
TDnsKMeans<TDist>::TDnsKMeans(const int& _K, const TRnd& Rnd):
		TAbsKMeans<TDist>(Rnd),
		K(_K) {}

template <class TDist>
TDnsKMeans<TDist>::TDnsKMeans(TSIn& SIn):
		TAbsKMeans<TDist>(SIn),
		K(SIn) {}

template <class TDist>
void TDnsKMeans<TDist>::Save(TSOut& SOut) const {
	TAbsKMeans<TDist>::Save(SOut);
	K.Save(SOut);
}

template <class TDist>
void TDnsKMeans<TDist>::Apply(const TFltVV& FtrVV, const int& MaxIter, const PNotify& Notify) {
	EAssertR(K <= FtrVV.GetCols(), "Matrix should have more columns than K!");

	Notify->OnNotify(TNotifyType::ntInfo, "Executing KMeans ...");

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();

	// assignment vectors
	TIntV AssignIdxV, OldAssignIdxV;
	TIntV* AssignIdxVPtr = &AssignIdxV;
	TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
	TIntV* Temp;

	// constant reused variables
	TFltV OnesN;			TLAUtil::Ones(NInst, OnesN);
	TFltV NormX2;			TLinAlg::GetColNorm2V(FtrVV, NormX2);
	TIntV RangeN(NInst);	TLAUtil::Range(NInst, RangeN);

	// reused variables
	TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
	TFltV TempK(K);						// (dimension k)
	TFltVV TempDxK(Dim, K);				// (dimension d x k)
	TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

	// select initial centroids
	TAbsKMeans<TDist>::SelectInitCentroids(FtrVV, K);

	// do the work
	for (int i = 0; i < MaxIter; i++) {
		if (i % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// get the distance of each of the points to each of the centroids
		// and assign the instances
		TLinAlg::GetColNorm2V(TAbsKMeans<TDist>::CentroidVV, TempK);
		TAbsKMeans<TDist>::GetDistMat2(FtrVV, NormX2, TempK, ClustDistVV);
		TLinAlg::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

		// if the assignment hasn't changed then terminate the loop
		if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", i);
			break;
		}

		// recompute the means
		TAbsKMeans<TDist>::UpdateCentroids(FtrVV, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV);

		// swap the old and new assign vectors
		Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}
}

///////////////////////////////////////////
// DPMeans
template <class TDist>
TDpMeans<TDist>::TDpMeans(const TFlt& _Lambda, const TInt& _MnClusts, const TInt& _MxClusts,
			const TRnd& Rnd):
		TAbsKMeans<TDist>(Rnd),
		Lambda(_Lambda),
		MnClusts(_MnClusts),
		MxClusts(_MxClusts) {

	EAssertR(MnClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
	EAssertR(MxClusts >= MnClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

template <class TDist>
TDpMeans<TDist>::TDpMeans(TSIn& SIn):
		TAbsKMeans<TDist>(SIn),
		Lambda(SIn),
		MnClusts(SIn),
		MxClusts(SIn) {}

template <class TDist>
void TDpMeans<TDist>::Save(TSOut& SOut) const {
	TAbsKMeans<TDist>::Save(SOut);
	Lambda.Save(SOut);
	MnClusts.Save(SOut);
	MxClusts.Save(SOut);
}

template <class TDist>
void TDpMeans<TDist>::Apply(const TFltVV& FtrVV, const int& MaxIter, const PNotify& Notify) {
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
	TAbsKMeans<TDist>::SelectInitCentroids(FtrVV, MnClusts);

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
		if (i % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", i); }

		// compute the distance matrix to all the centroids and assignments
		TLinAlg::GetColNorm2V(TAbsKMeans<TDist>::CentroidVV, TempK);
		TAbsKMeans<TDist>::GetDistMat2(FtrVV, NormX2, TempK, ClustDistVV);
		TLinAlg::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

		// check if we need to increase the number of centroids
		if (K < MxClusts) {
			TLinAlg::GetColMinV(ClustDistVV, MinClustDistV);

			const int NewCentrIdx = TLAUtil::GetMaxIdx(MinClustDistV);
			const double MaxDist = MinClustDistV[NewCentrIdx];

			if (MaxDist > LambdaSq) {
				K++;
				FtrVV.GetCol(NewCentrIdx, FtrV);
				TAbsKMeans<TDist>::CentroidVV.AddCol(FtrV);
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

		// recompute the centroids
		TAbsKMeans<TDist>::UpdateCentroids(FtrVV, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV);

		// swap old and new assign vectors
		Temp = AssignIdxVPtr;
		AssignIdxVPtr = OldAssignIdxVPtr;
		OldAssignIdxVPtr = Temp;
	}
}

}

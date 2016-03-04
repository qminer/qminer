/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TDist {

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TDist {
public:
	virtual ~TDist() {}

	virtual void UpdateNormX2(const TFltVV& FtrVV, TFltV& NormX2) const {}
	virtual void UpdateNormC2(const TFltVV& CentroidVV, TFltV& NormC2) const {}

	virtual void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const = 0;
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	virtual void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	virtual void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
	virtual void GetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormXV,
		const TFltV& NormCV, TFltVV& D) const = 0;
};

class TEuclDist: public TDist {
public:
	void UpdateNormX2(const TFltVV& FtrVV, TFltV& NormX2) const;
	void UpdateNormC2(const TFltVV& CentroidVV, TFltV& NormC2) const;

	void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const;
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const;
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { StaticGetDist2VV(X, Y, D); }

	void GetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
			const TFltV& NormY2, TFltVV& D) const { StaticGetDist2VV(X, Y, NormX2, NormY2, D); }

	static void StaticGetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D);
	static void StaticGetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
			const TFltV& NormY2, TFltVV& D);
};

class TCosDist: public TDist {
public:

	void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const {} // TODO implement me
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const {} // TODO implement me
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const {}	// TODO implement me

	void GetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
			const TFltV& NormY2, TFltVV& D) const { GetDist2VV(X, Y, D); }
};

}

namespace TClustering {

///////////////////////////////////////////
// Abstract class that has methods needed be KMeans
class TAbsKMeans {
//typedef TPt<TAbsKMeans> PDnsKMeans;
//class TAbsKMeans {
//private:
//  TCRef CRef;
//public:
//  friend class TPt<TAbsKMeans>;
protected:
	TFltVV CentroidVV;
	TDist::TDist* Dist;

	TRnd Rnd;

public:
	TAbsKMeans(const TRnd& Rnd, TDist::TDist* Dist=new TDist::TEuclDist());
	TAbsKMeans(TSIn& SIn);

	virtual ~TAbsKMeans() { delete Dist; }

	virtual void Save(TSOut& SOut) const;
	static TAbsKMeans* Load(TSIn& SIn);

	int GetClusts() const { return CentroidVV.GetCols(); }
	int GetDim() const { return CentroidVV.GetRows(); }

	// returns the centroid (column) matrix
	const TFltVV& GetCentroidVV() const { return CentroidVV; }
	// returns the n-th centroid
	void GetCentroid(const int& ClustN, TFltV& FtrV) const;

	void Apply(const TFltVV& FtrVV, const int& MaxIter=10000,
			const PNotify& Notify=TNotify::NullNotify) {
		const int Dim = GetFtrVDim(FtrVV);
		EAssertR(Dim > 0, "The input matrix doesn't have any features!");
		Apply(GetFtrVV(FtrVV), GetFtrVN(FtrVV), Dim, MaxIter, Notify);
	}

	// assign methods
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
	virtual void Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
			const int& MaxIter, const PNotify& Notify) = 0;

	// can still optimize
	void UpdateCentroids(const TFltVV& FtrVV, const int& NInst, TIntV& AssignV, const TFltV& OnesN,
			const TIntV& RangeN, TFltV& TempK, TFltVV& TempDxKV,
			TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2);
	void SelectInitCentroids(const TFltVV& FtrVV, const int& K);

	void Assign(const TFltVV& FtrVV, const TFltV& NormX2, const TFltV& NormC2, TIntV& AssignV) const;

	// specialized methods
	int GetFtrVN(const TFltVV& FtrVV) const {
		return FtrVV.GetCols();
	}
	int GetFtrVDim(const TFltVV& FtrVV) const {
		// FIXME enable inputing dimension through arguments for sparse matrices
		return FtrVV.GetRows();
	}
	const TFltVV& GetFtrVV(const TFltVV& FtrVV) const {
		return FtrVV;
	}
	void GetCol(const TFltVV& FtrVV, const int& ColN, TFltV& Col) const {
		FtrVV.GetCol(ColN, Col);
	}

	// type
	virtual const TStr GetType() const = 0;
};

///////////////////////////////////////////
// K-Means
class TDnsKMeans : public TAbsKMeans {
private:
	const TInt K;
public:
	TDnsKMeans(const int& K, const TRnd& Rnd=TRnd(0));
	TDnsKMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

protected:
	void Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
			const int& MaxIter, const PNotify& Notify);

	const TStr GetType() const { return "kmeans"; }
};

///////////////////////////////////////////
// DPMeans
class TDpMeans : public TAbsKMeans {
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

protected:
	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
			const int& MaxIter, const PNotify& Notify);

	const TStr GetType() const { return "dpmeans"; }
};

///////////////////////////////////////////
// Agglomerative clustering - average link
class TAvgLink {
public:
	static void JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& i,
			const int& j);
};

///////////////////////////////////////////
// Agglomerative clustering - complete link
class TCompleteLink {
public:
	static void JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& i,
			const int& j);
};

///////////////////////////////////////////
// Agglomerative clustering - single link
class TSingleLink {
public:
	static void JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& i,
			const int& j);
};

///////////////////////////////////////////
// Agglomerative clustering
template <class TDist, class TLink>
class TAggClust {
public:
	static void MakeDendro(const TFltVV& X, TIntIntFltTrV& MergeV, const PNotify& Notify) {
		const int NInst = X.GetCols();

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "%s\n", TStrUtil::GetStr(X, ", ", "%.3f").CStr());

		TFltVV ClustDistVV;	TDist::StaticGetDist2VV(X,X, ClustDistVV);
		TIntV ItemCountV;	TLAUtil::Ones(NInst, ItemCountV);//TVector::Ones(NInst);

		for (int k = 0; k < NInst-1; k++) {
			// find active <i,j> with minimum distance
			int MnI = -1;
			int MnJ = -1;
			double MnDist = TFlt::PInf;

			// find clusters with min distance
			for (int i = 0; i < NInst; i++) {
				if (ItemCountV[i] == 0) { continue; }

				for (int j = i+1; j < NInst; j++) {
					if (i == j || ItemCountV[j] == 0) { continue; }

					if (ClustDistVV(i,j) < MnDist) {
						MnDist = ClustDistVV(i,j);
						MnI = i;
						MnJ = j;
					}
				}
			}

			double Dist = sqrt(MnDist < 0 ? 0 : MnDist);
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Merging clusters %d, %d, distance: %.3f", MnI, MnJ, Dist);
			// merge
			MergeV.Add(TIntIntFltTr(MnI, MnJ, Dist));

			TLink::JoinClusts(ClustDistVV, ItemCountV, MnI, MnJ);

			// update counts
			ItemCountV[MnI] = ItemCountV[MnI] + ItemCountV[MnJ];
			ItemCountV[MnJ] = 0;
		}
	}
};

typedef TAggClust<TDist::TEuclDist, TAvgLink> TAlAggClust;
typedef TAggClust<TDist::TEuclDist, TCompleteLink> TClAggClust;
typedef TAggClust<TDist::TEuclDist, TCompleteLink> TSlAggClust;

}

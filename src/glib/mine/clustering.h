/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TDistance {

namespace {
	typedef TVec<TIntFltKdV> TSpVV;
	typedef TIntFltKdV TSpV;
}

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TDist {
public:
	virtual ~TDist() {}

	virtual void Save(TSOut& SOut) const { GetType().Save(SOut); }
	static TDist* Load(TSIn& SIn);

	// returns the distance between y to each of the columns in X
	virtual void GetDistV(const TFltVV& X, const TFltV& y, TFltV& DistV) const = 0;
	virtual void GetDistV(const TSpVV& X, const TSpV& y, TFltV& DistV) const = 0;
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	virtual void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
	virtual void GetDistVV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const = 0;
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	virtual void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
	virtual void GetDist2VV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const = 0;

	// these methods are only used for optimization
	// if one wishes to reuse a vector of size m and a vector of size n
	// during their procedure, then they should implement these methods
	// otherwise they can be left alone and the procedure will create
	// temporary variables in each iteration
	virtual void UpdateNormX2(const TFltVV& FtrVV, TFltV& NormX2) const {}
	virtual void UpdateNormX2(const TSpVV& FtrVV, TFltV& NormX2) const {}

	virtual void UpdateNormC2(const TFltVV& CentroidVV, TFltV& NormC2) const {}
	virtual void UpdateNormC2(const TSpVV& CentroidVV, TFltV& NormC2) const {}

	virtual void GetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormXV,
			const TFltV& NormCV, TFltVV& D) const { GetDist2VV(X, Y, D); };
	virtual void GetDist2VV(const TSpVV& X, const TSpVV& Y, const TFltV& NormXV,
				const TFltV& NormCV, TFltVV& D) const { GetDist2VV(X, Y, D); };

	virtual const TStr& GetType() const = 0;
};

class TEuclDist: public TDist {
public:
	static const TStr TYPE;

	void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const { GetDistV<TFltVV, TFltV>(CentroidVV, FtrV, DistV); }
	void GetDistV(const TSpVV& CentroidVV, const TSpV& FtrV, TFltV& DistV) const { GetDistV<TSpVV, TSpV>(CentroidVV, FtrV, DistV); }

	void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TFltVV>(X, Y, D); }
	void GetDistVV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const { GetDistVV<TSpVV>(X, Y, D); }

	void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDist2VV<TFltVV>(X, Y, D); }
	void GetDist2VV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const { GetDist2VV<TSpVV>(X, Y, D); }

	void UpdateNormX2(const TFltVV& FtrVV, TFltV& NormX2) const { UpdateNormX2<TFltVV>(FtrVV, NormX2); }
	void UpdateNormX2(const TSpVV& FtrVV, TFltV& NormX2) const { UpdateNormX2<TSpVV>(FtrVV, NormX2); }

	void UpdateNormC2(const TFltVV& CentroidVV, TFltV& NormC2) const { UpdateNormC2<TFltVV>(CentroidVV, NormC2); }
	void UpdateNormC2(const TSpVV& CentroidVV, TFltV& NormC2) const { UpdateNormC2<TSpVV>(CentroidVV, NormC2); }

    void GetDist2VV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TFltVV>(X, Y, NormX2, NormY2, D); }
	void GetDist2VV(const TSpVV& X, const TSpVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TSpVV>(X, Y, NormX2, NormY2, D); }

	const TStr& GetType() const { return TYPE; }

private:
	template <class TMatType, class TVecType>
	void GetDistV(const TMatType& CentroidVV, const TVecType& FtrV, TFltV& DistV) const {
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

	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	template <class TMatType>
	void GetDistVV(const TMatType& X, const TMatType& Y, TFltVV& D) const {
		GetDist2VV(X, Y, D);
		TLAMisc::Sqrt(D);
	}
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	template <class TMatType>
	void GetDist2VV(const TMatType& X, const TMatType& Y, TFltVV& D) const {
		TFltV NormX2;	TLinAlg::GetColNorm2V(X, NormX2);
		TFltV NormY2;	TLinAlg::GetColNorm2V(Y, NormY2);

		GetDist2VV(X, Y, NormX2, NormY2, D);
	}

	template <class TMatType>
	void UpdateNormX2(const TMatType& FtrVV, TFltV& NormX2) const {
		TLinAlg::GetColNorm2V(FtrVV, NormX2);
	}

	template <class TMatType>
	void UpdateNormC2(const TMatType& CentroidVV, TFltV& NormC2) const {
		TLinAlg::GetColNorm2V(CentroidVV, NormC2);
	}

    template<class TMatType>
    void GetDist2VV(const TMatType& X, const TMatType& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const {
        //	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
        // 1) X'Y
        TLinAlg::MultiplyT(X, Y, D);
        // 2) (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2)
        const int Rows = D.GetRows();
        const int Cols = D.GetCols();

        for (int RowN = 0; RowN < Rows; RowN++) {
            for (int ColN = 0; ColN < Cols; ColN++) {
                D.PutXY(RowN, ColN, NormX2[RowN] - 2 * D(RowN, ColN) + NormY2[ColN]);
            }
        }
    }
};

class TCosDist: public TDist {
public:
	static const TStr TYPE;

	void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const {} // TODO implement me
	void GetDistV(const TSpVV& CentroidVV, const TSpV& FtrV, TFltV& DistV) const {} // TODO implement me
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const {} // TODO implement me
	void GetDistVV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const {} // TODO implement me
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	void GetDist2VV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const {}	// TODO implement me
	void GetDist2VV(const TSpVV& X, const TSpVV& Y, TFltVV& D) const {}	// TODO implement me

	const TStr& GetType() const { return TYPE; }
};

}

namespace TClustering {

using namespace TDistance;

///////////////////////////////////////////
// Abstract class that has methods needed be KMeans
template<class centroidType>
class TAbsKMeans {
//typedef TPt<TAbsKMeans> PDnsKMeans;
//class TAbsKMeans {
//private:
//  TCRef CRef;
//public:
//  friend class TPt<TAbsKMeans>;
protected:
    centroidType CentroidVV;
	TDist* Dist;

	TRnd Rnd;

public:
	TAbsKMeans(const TRnd& Rnd, TDist* Dist=new TEuclDist());
	TAbsKMeans(TSIn& SIn);

	virtual ~TAbsKMeans() { delete Dist; }

	virtual void Save(TSOut& SOut) const;
	static TAbsKMeans* Load(TSIn& SIn);

	int GetClusts() const { return GetMatVN(CentroidVV); }
	int GetDim() const { return GetMatVDim(CentroidVV); }

	// returns the centroid (column) matrix
    const centroidType& GetCentroidVV() const { return CentroidVV; }
	// returns the n-th centroid
    template<class vectorType>
    void GetCentroid(const int& ClustN, vectorType& FtrV) const;

    template<class dataType>
    void Apply(const dataType& FtrVV, const int& MaxIter = 10000,
			const PNotify& Notify=TNotify::NullNotify) {
		const int Dim = GetMatVDim(FtrVV);
		EAssertR(Dim > 0, "The input matrix doesn't have any features!");
		Apply(GetMatVV(FtrVV), GetMatVN(FtrVV), Dim, MaxIter, Notify);
	}

	// assign methods
    template<class dataType>
    void Assign(const dataType& FtrVV, TIntV& AssignV) const;

	// distance methods
	// returns the distance to the specified centroid
    template<class featureType>
    double GetDist(const int& ClustN, const featureType& FtrV) const;
	// returns the distance to all the centroids
    template<class featureType>
    void GetCentroidDistV(const featureType& FtrV, TFltV& DistV) const;

	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
    template<class dataType>
    void GetDistVV(const dataType& FtrVV, TFltVV& DistVV) const;

protected:
	virtual void Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
			const int& MaxIter, const PNotify& Notify) = 0;
    virtual void Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
        const int& MaxIter, const PNotify& Notify) = 0;

	// can still optimize
    template<class dataType>
    void UpdateCentroids(const dataType& FtrVV, const int& NInst, TIntV& AssignV, const TFltV& OnesN,
        const TIntV& RangeN, TFltV& TempK, dataType& TempDxKV,
			TSpVV& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2);

    template<class dataType>
    void SelectInitCentroids(const dataType& FtrVV, const int& K);

    template<class dataType>
    inline void Assign(const dataType& FtrVV, const TFltV& NormX2, const TFltV& NormC2, TIntV& AssignV) const;

	// specialized methods
	int GetMatVN(const TFltVV& X) const {
		return X.GetCols();
	}
	int GetMatVDim(const TFltVV& X) const {
		// FIXME enable inputing dimension through arguments for sparse matrices
		return X.GetRows();
	}
	const TFltVV& GetMatVV(const TFltVV& X) const {
		return X;
	}
	void GetCol(const TFltVV& FtrVV, const int& ColN, TFltV& Col) const {
		FtrVV.GetCol(ColN, Col);
	}

    void SetCol(TFltVV& FtrVV, const int& ColN, const TFltV& Col) const {
        FtrVV.SetCol(ColN, Col);
    }

    int GetMatVN(const TSpVV& FtrVV) const {
        return FtrVV.Len();
    }
    int GetMatVDim(const TSpVV& FtrVV) const {
        return TLAMisc::GetMaxDimIdx(FtrVV);
    }

    const TVec<TIntFltKdV>& GetMatVV(const TSpVV& X) {
        return X;
    }

    void GetCol(const TSpVV& FtrVV, const int& ColN, TSpV& Col) const {
        Col = FtrVV[ColN];
    }

    void SetCol(TSpVV& FtrVV, const int& ColN, const TSpV& Col) const {
        FtrVV[ColN] = Col;
    }

	// type
	virtual const TStr GetType() const = 0;
};




template<class centroidType>
TAbsKMeans<centroidType>::TAbsKMeans(const TRnd& _Rnd, TDist* _Dist) :
        CentroidVV(),
        Dist(_Dist),
        Rnd(_Rnd) {}

template<class centroidType>
TAbsKMeans<centroidType>::TAbsKMeans(TSIn& SIn) :
        CentroidVV(SIn),
        Dist(TDist::Load(SIn)),
        Rnd(SIn) {}

template<class centroidType>
void TAbsKMeans<centroidType>::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    CentroidVV.Save(SOut);
    Dist->Save(SOut);
    Rnd.Save(SOut);
}

template<class centroidType>
TAbsKMeans<centroidType>* TAbsKMeans<centroidType>::Load(TSIn& SIn) {
    TStr Type(SIn);
    if (Type == "kmeans") {
        return new TDnsKMeans<centroidType>(SIn);
    }
    else if (Type == "dpmeans") {
        return new TDpMeans<centroidType>(SIn);
    }
    else {
        throw TExcept::New("Invalid clustering type: " + Type);
    }
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::GetCentroid(const int& ClustN, TFltV& FtrV) const {
    EAssert(0 <= ClustN && ClustN < GetClusts());
    CentroidVV.GetCol(ClustN, FtrV);
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::GetCentroid(const int& ClustN, TSpV& FtrV) const {
    throw TExcept::New("Not implemented!");

}

template<>
template<>
inline void TAbsKMeans<TSpVV>::GetCentroid(const int& ClustN, TFltV& FtrV) const {
    throw TExcept::New("Not implemented!");
}

template<>
template<>
inline void TAbsKMeans<TSpVV>::GetCentroid(const int& ClustN, TSpV& FtrV) const {
    EAssert(0 <= ClustN && ClustN < GetClusts());
    FtrV = CentroidVV[ClustN];
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::Assign(const TFltVV& FtrVV, TIntV& AssignV) const {
    TFltVV DistVV;	Dist->GetDist2VV(CentroidVV, FtrVV, DistVV);
    TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

template<>
template<>
inline void TAbsKMeans<TSpVV>::Assign(const TSpVV& FtrVV, TIntV& AssignV) const {
    TFltVV DistVV;	Dist->GetDist2VV(CentroidVV, FtrVV, DistVV);
    TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

template<>
template<>
inline double TAbsKMeans<TFltVV>::GetDist(const int& ClustN, const TFltV& Pt) const {
    TFltV Centroid;	CentroidVV.GetCol(ClustN, Centroid);
    return TLinAlg::EuclDist(Centroid, Pt);
}

template<>
template<>
inline double TAbsKMeans<TSpVV>::GetDist(const int& ClustN, const TSpV& Pt) const {
    TSpV Centroid; Centroid = CentroidVV[ClustN];
    return TLinAlg::EuclDist(Centroid, Pt);
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::GetCentroidDistV(const TFltV& FtrV, TFltV& DistV) const {
    Dist->GetDistV(CentroidVV, FtrV, DistV);
}

template<>
template<>
inline void TAbsKMeans<TSpVV>::GetCentroidDistV(const TSpV& FtrV, TFltV& DistV) const {
    Dist->GetDistV(CentroidVV, FtrV, DistV);
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::GetDistVV(const TFltVV& FtrVV, TFltVV& DistVV) const {
    Dist->GetDistVV(CentroidVV, FtrVV, DistVV);
}

template<>
template<>
inline void TAbsKMeans<TSpVV>::GetDistVV(const TSpVV& FtrVV, TFltVV& DistVV) const {
    Dist->GetDistVV(CentroidVV, FtrVV, DistVV);
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::UpdateCentroids(const TFltVV& FtrVV, const int& NInst, TIntV& AssignV,
    const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TFltVV& TempDxKV,
    TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2) {

    const int K = GetMatVN(CentroidVV);

    // I. create a sparse matrix (coordinate representation) that encodes the closest centroids
    TSparseColMatrix AssignMat(NInst, K);

    bool AllClustsFull;
    do {
        AllClustsFull = true;

        TSparseOps<TInt, TFlt>::CoordinateCreateSparseColMatrix(RangeN, AssignV, OnesN, AssignMat.ColSpVV, K);

        // II. compute the number of points that belong to each centroid, invert
        AssignMat.MultiplyT(OnesN, TempK);

        // invert
        for (int ClustN = 0; ClustN < K; ClustN++) {
            if (TempK[ClustN] == 0.0) {	// don't allow empty clusters
                // select a random point and create a new centroid from it
                const int RndRecN = Rnd.GetUniDevInt(GetMatVN(FtrVV));
                TFltV RndRecFtrV;	GetCol(FtrVV, RndRecN, RndRecFtrV);
                SetCol(CentroidVV, ClustN, RndRecFtrV);
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


template<>
template<>
inline void TAbsKMeans<TSpVV>::UpdateCentroids(const TSpVV& FtrVV, const int& NInst, TIntV& AssignV,
    const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TSpVV& TempDxKV,
    TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2) {

    const int K = GetMatVN(CentroidVV);

    // I. create a sparse matrix (coordinate representation) that encodes the closest centroids
    TSparseColMatrix AssignMat(NInst, K);

    bool AllClustsFull;
    do {
        AllClustsFull = true;

        TSparseOps<TInt, TFlt>::CoordinateCreateSparseColMatrix(RangeN, AssignV, OnesN, AssignMat.ColSpVV, K);

        // II. compute the number of points that belong to each centroid, invert
        AssignMat.MultiplyT(OnesN, TempK);

        // invert
        for (int ClustN = 0; ClustN < K; ClustN++) {
            if (TempK[ClustN] == 0.0) {	// don't allow empty clusters
                // select a random point and create a new centroid from it
                const int RndRecN = Rnd.GetUniDevInt(GetMatVN(FtrVV));
                TSpV RndRecFtrV;	GetCol(FtrVV, RndRecN, RndRecFtrV);
                SetCol(CentroidVV, ClustN, RndRecFtrV);
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
    TLinAlg::Multiply(FtrVV, AssignMat.ColSpVV, TempDxKV); // TempDxKV has at column i the sum of all data points that are in cluster i
    // 2) (FtrVV * AssignIdxMat) + CentroidMat
    TLinAlg::LinComb(1, TempDxKV, 1, CentroidVV, TempDxKV);
    // 3) ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag
    TLinAlg::Multiply(TempDxKV, TempKxKSpVV, CentroidVV);
}

template<>
template<>
inline void TAbsKMeans<TFltVV>::SelectInitCentroids(const TFltVV& FtrVV, const int& K) {
    const int NInst = GetMatVN(FtrVV);
    const int Dim = GetMatVDim(FtrVV);

    EAssertR(NInst >= K, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

    // generate k random elements
    TFltV PermV(NInst);	TLAUtil::Range(NInst, PermV);
    TIntV CentroidNV(K);

    double Temp;
    for (int i = 0; i < K; i++) {
        const int SwapIdx = Rnd.GetUniDevInt(i, NInst - 1);

        // swap
        Temp = PermV[SwapIdx];
        PermV[SwapIdx] = PermV[i];
        PermV[i] = Temp;

        CentroidNV[i] = (int)PermV[i];
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

template<>
template<>
inline void TAbsKMeans<TSpVV>::SelectInitCentroids(const TSpVV& FtrVV, const int& K) {
    const int NInst = GetMatVN(FtrVV);
    const int Dim = GetMatVDim(FtrVV);

    EAssertR(NInst >= K, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

    // generate k random elements
    TFltV PermV(NInst);	TLAUtil::Range(NInst, PermV);
    TIntV CentroidNV(K);

    double Temp;
    for (int i = 0; i < K; i++) {
        const int SwapIdx = Rnd.GetUniDevInt(i, NInst - 1);

        // swap
        Temp = PermV[SwapIdx];
        PermV[SwapIdx] = PermV[i];
        PermV[i] = Temp;

        CentroidNV[i] = (int)PermV[i];
    }

    // construct the centroid matrix
    CentroidVV.Gen(K);
    for (int i = 0; i < K; i++) {
        const int ColN = CentroidNV[i];
        CentroidVV[i] = FtrVV[ColN];
    }
}

template<class centroidType>
template<class dataType>
inline void TAbsKMeans<centroidType>::Assign(const dataType& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
    TIntV& AssignV) const {
    TFltVV DistVV;	Dist->GetDist2VV(CentroidVV, FtrVV, NormC2, NormX2, DistVV);
    TLinAlg::GetColMinIdxV(DistVV, AssignV);
}

///////////////////////////////////////////
// K-Means
template<class centroidType>
class TDnsKMeans : public TAbsKMeans<centroidType> {
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
    void Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
        const int& MaxIter, const PNotify& Notify);

	const TStr GetType() const { return "kmeans"; }
};

template<class centroidType>
TDnsKMeans<centroidType>::TDnsKMeans(const int& _K, const TRnd& Rnd) :
        TAbsKMeans<centroidType>(Rnd),
        K(_K) {}

template<class centroidType>
TDnsKMeans<centroidType>::TDnsKMeans(TSIn& SIn) :
        TAbsKMeans<centroidType>(SIn),
        K(SIn) {}

template<class centroidType>
void TDnsKMeans<centroidType>::Save(TSOut& SOut) const {
    TAbsKMeans<centroidType>::Save(SOut);
    K.Save(SOut);
}

template<>
inline void TDnsKMeans<TFltVV>::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
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

template<>
inline void TDnsKMeans<TFltVV>::Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
    const int& MaxIter, const PNotify& Notify) {
    throw TExcept::New("Not implemented!");
}

template<>
inline void TDnsKMeans<TSpVV>::Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
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
    TSpVV TempDxK(K);				    // (dimension d x k)
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

template<>
inline void TDnsKMeans<TSpVV>::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
    const int& MaxIter, const PNotify& Notify) {
    throw TExcept::New("Not implemented!");
}

///////////////////////////////////////////
// DPMeans
template<class centroidType>
class TDpMeans : public TAbsKMeans<centroidType> {
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
    void Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
        const int& MaxIter, const PNotify& Notify);

	const TStr GetType() const { return "dpmeans"; }
};
template<class centroidType>
TDpMeans<centroidType>::TDpMeans(const TFlt& _Lambda, const TInt& _MnClusts, const TInt& _MxClusts,
    const TRnd& Rnd) :
    TAbsKMeans<centroidType>(Rnd),
    Lambda(_Lambda),
    MnClusts(_MnClusts),
    MxClusts(_MxClusts) {

    EAssertR(MnClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
    EAssertR(MxClusts >= MnClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

template<class centroidType>
TDpMeans<centroidType>::TDpMeans(TSIn& SIn) :
        TAbsKMeans<centroidType>(SIn),
        Lambda(SIn),
        MnClusts(SIn),
        MxClusts(SIn) {}

template<class centroidType>
void TDpMeans<centroidType>::Save(TSOut& SOut) const {
    TAbsKMeans<centroidType>::Save(SOut);
    Lambda.Save(SOut);
    MnClusts.Save(SOut);
    MxClusts.Save(SOut);
}

template<>
inline void TDpMeans<TFltVV>::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
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
                (*AssignIdxVPtr)[NewCentrIdx] = K - 1;
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

inline void TDpMeans<TFltVV>::Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
    const int& MaxIter, const PNotify& Notify) {
    throw TExcept::New("Not implemented!");
}

template<>
inline void TDpMeans<TSpVV>::Apply(const TSpVV& FtrVV, const int& NInst, const int& Dim,
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
    TSpV FtrV;							// (dimension d)
    TFltV MinClustDistV;				// (dimension n)
    TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
    TFltV NormC2(K);					// (dimension k)
    TFltV TempK(K);						// (dimension k)
    TSpVV TempDxK(K);   			    // (dimension d x k)
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
                GetCol(FtrVV, NewCentrIdx, FtrV);
                CentroidVV.Add(FtrV);
                ClustDistVV.AddXDim();
                NormC2.Add(0);
                TempK.Add(0);
                TempDxK.Add(TIntFltKdV());
                TempKxKSpVV.Gen(K);
                (*AssignIdxVPtr)[NewCentrIdx] = K - 1;
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

inline void TDpMeans<TSpVV>::Apply(const TFltVV& FtrVV, const int& NInst, const int& Dim,
    const int& MaxIter, const PNotify& Notify) {
    throw TExcept::New("Not implemented!");
}

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

		TFltVV ClustDistVV;	TDist().GetDist2VV(X,X, ClustDistVV);
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

typedef TAggClust<TEuclDist, TAvgLink> TAlAggClust;
typedef TAggClust<TEuclDist, TCompleteLink> TClAggClust;
typedef TAggClust<TEuclDist, TCompleteLink> TSlAggClust;

}

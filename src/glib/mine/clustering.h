/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TClustering {

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TDist;
    typedef TPt<TDist> PDist;
class TDist {
private:
    TCRef CRef;
    friend class TPt<TDist>;
public:
    virtual ~TDist() {}

    virtual void Save(TSOut& SOut) const { GetType().Save(SOut); }
    static PDist Load(TSIn& SIn);

    /// returns the distance between y to each of the columns in X
    virtual void GetDistV(const TFltVV& X, const TFltV& y, TFltV& DistV) const = 0;
    virtual void GetDistV(const TFltVV& X, const TIntFltKdV& y, TFltV& DistV) const = 0;
    virtual void GetDistV(const TVec<TIntFltKdV>& X, const TFltV& y, TFltV& DistV) const = 0;
    virtual void GetDistV(const TVec<TIntFltKdV>& X, const TIntFltKdV& y, TFltV& DistV) const = 0;
    /// returns a matrix D of distances between elements of X to elements of Y
    /// X and Y are assumed to have column vectors
    /// D_ij is the distance between x_i and y_j
    virtual void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
    virtual void GetDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const = 0;
    virtual void GetDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const = 0;
    virtual void GetDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const = 0;
    /// returns a matrix D of values which are proportional to distances between elements of X to elements of Y
    /// in some manner. For example when using Euclidean distance, D will have squared distances
    /// but when using Cosine distances, D will have regular distances
    /// X and Y are assumed to contain column vectors
    /// D_ij is (proportional to) the distance between x_i and y_j
    virtual void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const = 0;
    virtual void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const = 0;
    virtual void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const = 0;
    virtual void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const = 0;
    /// used so that the developer can optimize the computation, by precomputing
    /// two vectors and reusing them
    virtual void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, const TFltV& NormXV,
            const TFltV& NormCV, TFltVV& D) const { GetQuasiDistVV(X, Y, D); };
    virtual void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, const TFltV& NormXV,
        const TFltV& NormCV, TFltVV& D) const { GetQuasiDistVV(X, Y, D); };
    virtual void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, const TFltV& NormXV,
        const TFltV& NormCV, TFltVV& D) const { GetQuasiDistVV(X, Y, D); };
    virtual void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, const TFltV& NormXV,
                const TFltV& NormCV, TFltVV& D) const { GetQuasiDistVV(X, Y, D); };


    /// these methods are only used for optimization
    /// if one wishes to reuse a vector of size m and a vector of size n
    /// during their procedure, then they should implement these methods
    /// otherwise they can be left alone and the procedure will create
    /// temporary variables in each iteration
    virtual void UpdateXLenDistHelpV(const TFltVV& FtrVV, TFltV& NormX2) const {}
    virtual void UpdateXLenDistHelpV(const TVec<TIntFltKdV>& FtrVV, TFltV& NormX2) const {}

    virtual void UpdateCLenDistHelpV(const TFltVV& CentroidVV, TFltV& NormC2) const {}
    virtual void UpdateCLenDistHelpV(const TVec<TIntFltKdV>& CentroidVV, TFltV& NormC2) const {}

    virtual const TStr& GetType() const = 0;
};

class TEuclDist: public TDist {
public:
    static const TStr TYPE;

    static PDist New() { return new TEuclDist; }

    void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const { GetDistV<TFltVV, TFltV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TFltVV& CentroidVV, const TIntFltKdV& FtrV, TFltV& DistV) const { GetDistV<TFltVV, TIntFltKdV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TVec<TIntFltKdV>& CentroidVV, const TFltV& FtrV, TFltV& DistV) const { GetDistV<TVec<TIntFltKdV>, TFltV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TVec<TIntFltKdV>& CentroidVV, const TIntFltKdV& FtrV, TFltV& DistV) const { GetDistV<TVec<TIntFltKdV>, TIntFltKdV>(CentroidVV, FtrV, DistV); }

    void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TFltVV, TFltVV>(X, Y, D); }
    void GetDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TFltVV, TVec<TIntFltKdV>>(X, Y, D); }
    void GetDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TFltVV>(X, Y, D); }
    void GetDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, D); }

    void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDist2VV<TFltVV, TFltVV>(X, Y, D); }
    void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDist2VV<TFltVV, TVec<TIntFltKdV>>(X, Y, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const { GetDist2VV<TVec<TIntFltKdV>, TFltVV>(X, Y, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDist2VV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, D); }

    void UpdateXLenDistHelpV(const TFltVV& FtrVV, TFltV& NormX2) const { UpdateNormX2<TFltVV>(FtrVV, NormX2); }
    void UpdateXLenDistHelpV(const TVec<TIntFltKdV>& FtrVV, TFltV& NormX2) const { UpdateNormX2<TVec<TIntFltKdV>>(FtrVV, NormX2); }

    void UpdateCLenDistHelpV(const TFltVV& CentroidVV, TFltV& NormC2) const { UpdateNormC2<TFltVV>(CentroidVV, NormC2); }
    void UpdateCLenDistHelpV(const TVec<TIntFltKdV>& CentroidVV, TFltV& NormC2) const { UpdateNormC2<TVec<TIntFltKdV>>(CentroidVV, NormC2); }

    void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TFltVV, TFltVV>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TFltVV, TVec<TIntFltKdV>>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TVec<TIntFltKdV>, TFltVV>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDist2VV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, NormX2, NormY2, D); }

    const TStr& GetType() const { return TYPE; }

private:
    // returns a vector d of distances between the elements of X and vector v
    // d[i] is the distance between X_i and v
    template <class TMatType, class TVectorType>
    void GetDistV(const TMatType& X, const TVectorType& v, TFltV& d) const;

    // returns a matrix D of distances between elements of X to elements of Y
    // D_ij is the distance between x_i and y_j
    template <class TXMatType, class TYMatType>
    void GetDistVV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const;

    // returns a matrix D of squared distances between elements of X to elements of Y
    // D_ij is the distance between x_i and y_j
    template <class TXMatType, class TYMatType>
    void GetDist2VV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const;

    template<class TXMatType, class TYMatType>
    void GetDist2VV(const TXMatType& X, const TYMatType& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const;

    template <class TMatType>
    void UpdateNormX2(const TMatType& FtrVV, TFltV& NormX2) const;

    template <class TMatType>
    void UpdateNormC2(const TMatType& CentroidVV, TFltV& NormC2) const;
};



template <class TMatType, class TVectorType>
void TEuclDist::GetDistV(const TMatType& X, const TVectorType& v, TFltV& DistV) const {
    // return (CentroidMat.ColNorm2V() - (x*C*2) + TVector::Ones(GetClusts(), false) * NormX2).Sqrt();
    // 1) squared norm of X
    const double NormX2 = TLinAlg::Norm2(v);

    // 2) Result <- CentroidMat.ColNorm2V()
    TLinAlg::GetColNorm2V(X, DistV);

    // 3) x*C
    TFltV xC;	TLinAlg::MultiplyT(X, v, xC);

    // 4) <- Result = Result - 2*x*C + ones(clusts, 1)*|x|^2
    for (int i = 0; i < DistV.Len(); i++) {
        DistV[i] += NormX2 - 2 * xC[i];
        AssertR(DistV[i] > -1e-8, "Distance lower than numerical error!");
        if (DistV[i] < 0) { DistV[i] = 0; }
        DistV[i] = TMath::Sqrt(DistV[i]);
    }
}

template <class TXMatType, class TYMatType>
void TEuclDist::GetDistVV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const {
    GetDist2VV(X, Y, D);
    TLinAlgTransform::Sqrt(D);
}

template <class TXMatType, class TYMatType>
void TEuclDist::GetDist2VV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const {
    TFltV NormX2;	TLinAlg::GetColNorm2V(X, NormX2);
    TFltV NormY2;	TLinAlg::GetColNorm2V(Y, NormY2);

    GetDist2VV(X, Y, NormX2, NormY2, D);
}

template<class TXMatType, class TYMatType>
void TEuclDist::GetDist2VV(const TXMatType& X, const TYMatType& Y, const TFltV& NormX2,
    const TFltV& NormY2, TFltVV& D) const {
    //  return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
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

template <class TMatType>
void TEuclDist::UpdateNormX2(const TMatType& FtrVV, TFltV& NormX2) const {
    TLinAlg::GetColNorm2V(FtrVV, NormX2);
}

template <class TMatType>
void TEuclDist::UpdateNormC2(const TMatType& CentroidVV, TFltV& NormC2) const {
    TLinAlg::GetColNorm2V(CentroidVV, NormC2);
}

class TCosDist: public TDist {
public:
    static const TStr TYPE;

    static PDist New() { return new TCosDist; }

    void GetDistV(const TFltVV& CentroidVV, const TFltV& FtrV, TFltV& DistV) const { GetDistV<TFltVV, TFltV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TFltVV& CentroidVV, const TIntFltKdV& FtrV, TFltV& DistV) const { GetDistV<TFltVV, TIntFltKdV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TVec<TIntFltKdV>& CentroidVV, const TFltV& FtrV, TFltV& DistV) const { GetDistV<TVec<TIntFltKdV>, TFltV>(CentroidVV, FtrV, DistV); }
    void GetDistV(const TVec<TIntFltKdV>& CentroidVV, const TIntFltKdV& FtrV, TFltV& DistV) const { GetDistV<TVec<TIntFltKdV>, TIntFltKdV>(CentroidVV, FtrV, DistV); }

    void GetDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TFltVV, TFltVV>(X, Y, D); }
    void GetDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TFltVV, TVec<TIntFltKdV>>(X, Y, D); }
    void GetDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TFltVV>(X, Y, D); }
    void GetDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, D); }

    void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TFltVV, TFltVV>(X, Y, D); }
    void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TFltVV, TVec<TIntFltKdV>>(X, Y, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TFltVV>(X, Y, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, D); }

    void GetQuasiDistVV(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDistVV<TFltVV, TFltVV>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TFltVV& X, const TVec<TIntFltKdV>& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDistVV<TFltVV, TVec<TIntFltKdV>>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TFltVV& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TFltVV>(X, Y, NormX2, NormY2, D); }
    void GetQuasiDistVV(const TVec<TIntFltKdV>& X, const TVec<TIntFltKdV>& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const { GetDistVV<TVec<TIntFltKdV>, TVec<TIntFltKdV>>(X, Y, NormX2, NormY2, D); }

    void UpdateXLenDistHelpV(const TFltVV& FtrVV, TFltV& NormX2) const { UpdateNormX<TFltVV>(FtrVV, NormX2); }
    void UpdateXLenDistHelpV(const TVec<TIntFltKdV>& FtrVV, TFltV& NormX2) const { UpdateNormX<TVec<TIntFltKdV>>(FtrVV, NormX2); }

    void UpdateCLenDistHelpV(const TFltVV& CentroidVV, TFltV& NormC2) const { UpdateNormC<TFltVV>(CentroidVV, NormC2); }
    void UpdateCLenDistHelpV(const TVec<TIntFltKdV>& CentroidVV, TFltV& NormC2) const { UpdateNormC<TVec<TIntFltKdV>>(CentroidVV, NormC2); }

    const TStr& GetType() const { return TYPE; }

private:
    template <class TMatType, class TVectorType>
    void GetDistV(const TMatType& CentroidVV, const TVectorType& FtrV, TFltV& DistV) const;

    // returns a matrix D of distances between elements of X to elements of Y
    // D_ij is the distance between x_i and y_j
    template <class TXMatType, class TYMatType>
    void GetDistVV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const;

    template <class TXMatType, class TYMatType>
    void GetDistVV(const TXMatType& X, const TYMatType& Y, const TFltV& NormX2, const TFltV& NormY2, TFltVV& D) const;

    template <class TMatType>
    void UpdateNormX(const TMatType& FtrVV, TFltV& NormX2) const;

    template <class TMatType>
    void UpdateNormC(const TMatType& CentroidVV, TFltV& NormC2) const;
};



template <class TMatType, class TVectorType>
void TCosDist::GetDistV(const TMatType& CentroidVV, const TVectorType& FtrV, TFltV& DistV) const {

    // 1) norm of X
    const double NormX = TLinAlg::Norm(FtrV);
    // 2) vector of column norms of CentroidVV
    TFltV xC; TLinAlg::GetColNormV(CentroidVV, xC);

    TLinAlg::MultiplyT(CentroidVV, FtrV, DistV);

    for (int i = 0; i < DistV.Len(); i++) {
        DistV[i] = 1 - DistV[i] / xC[i] / NormX;
    }
}

template <class TXMatType, class TYMatType>
void TCosDist::GetDistVV(const TXMatType& X, const TYMatType& Y, TFltVV& D) const {
    TFltV NormX;	TLinAlg::GetColNormV(X, NormX);
    TFltV NormY;	TLinAlg::GetColNormV(Y, NormY);

    GetDistVV(X, Y, NormX, NormY, D);
}

template <class TXMatType, class TYMatType>
void TCosDist::GetDistVV(const TXMatType& X, const TYMatType& Y, const TFltV& NormX2,
        const TFltV& NormY2, TFltVV& D) const {
    // 1) X'Y
    TLinAlg::MultiplyT(X, Y, D);
    // 2) X'Y ./ sqrt(x2 x y2)
    const int Rows = D.GetRows();
    const int Cols = D.GetCols();

    for (int RowN = 0; RowN < Rows; RowN++) {
        for (int ColN = 0; ColN < Cols; ColN++) {
            D.PutXY(RowN, ColN, 1 - D(RowN, ColN) / (NormX2[RowN]*NormY2[ColN]));
        }
    }
}

template <class TMatType>
void TCosDist::UpdateNormX(const TMatType& FtrVV, TFltV& NormX2) const {
    TLinAlg::GetColNormV(FtrVV, NormX2);
}

template <class TMatType>
void TCosDist::UpdateNormC(const TMatType& CentroidVV, TFltV& NormC2) const {
    TLinAlg::GetColNormV(CentroidVV, NormC2);
}

//============================================================
// CLUSTERING METHODS - CLASS DECLARATIONS
//============================================================

///////////////////////////////////////////
// Abstract class that has methods needed be KMeans
template<class TCentroidType>
class TAbsKMeans {
    friend class TPt<TAbsKMeans<TCentroidType>>;
private:
    TCRef CRef;
protected:
    
    TCentroidType CentroidVV;
    PDist Dist;

    TRnd Rnd;
public:

    TAbsKMeans(const TRnd& Rnd, const PDist& Dist=TEuclDist::New());
    TAbsKMeans(TSIn& SIn);

    virtual ~TAbsKMeans() {}

    virtual void Save(TSOut& SOut) const;
    static TAbsKMeans<TCentroidType>* LoadPtr(TSIn& SIn);
    static TPt<TAbsKMeans<TCentroidType>> Load(TSIn& SIn);

    /// returns the number of clusters
    int GetClusts() const { return GetDataCount(CentroidVV); }
    /// returns the dimension of the data
    int GetDim() const { return GetDataDim(CentroidVV); }

    /// returns the centroid (column) matrix
    const TCentroidType& GetCentroidVV() const { return CentroidVV; }
    /// permutates the centroid matrix
    inline void PermutateCentroids(const TIntV& Mapping);
    /// returns the n-th centroid
    template<class TVectorType>
    void GetCentroid(const int& ClustN, TVectorType& FtrV) const;

    virtual void Apply(const TFltVV& FtrVV, const bool& AllowEmptyP=true,
            const int& MaxIter=10000, const PNotify& Notify=TNotify::NullNotify) = 0;
    virtual void Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP=true,
            const int& MaxIter=10000, const PNotify& Notify = TNotify::NullNotify) = 0;

    /// assign methods
    template<class TDataType>
    void Assign(const TDataType& FtrVV, TIntV& AssignV) const;

    /// distance methods
    /// returns the distance to the specified centroid
    template<class TDataType>
    double GetDist(const int& ClustN, const TDataType& FtrV) const;
    /// returns the distance to all the centroids
    template<class TDataType>
    void GetCentroidDistV(const TDataType& FtrV, TFltV& DistV) const;

    /// returns a matrix D with the distance to all the centroids
    /// D_ij is the distance between centroid i and instance j
    /// points should be represented as columns of X
    template<class TDataType>
    void GetDistVV(const TDataType& FtrVV, TFltVV& DistVV) const;

    /// returns the class used to calculate distances
    const PDist& GetDistMetric() const { return Dist; }

    inline void RemoveCentroids(const TIntV& CentroidIdV);

protected:
    /// can still optimize
    template<class TDataType>
    void UpdateCentroids(const TDataType& FtrVV, const int& NInst, TIntV& AssignV,
            const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TCentroidType& TempDxKV,
            TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2, TFltV& NormC2,
            const bool& AllowEmptyP);

    template<class TDataType>
    void SelectInitCentroids(const TDataType& FtrVV, const int& K, const int& NInst);
    template<class TDataType>
    void SelectInitCentroids(const TDataType& FtrVV);

    template<class TDataType>
    inline void Assign(const TDataType& FtrVV, const TFltV& NormX2, const TFltV& NormC2, TIntV& AssignV) const;

    /// methods that return the number of examples in the input data
    static int GetDataCount(const TFltVV& X);
    static int GetDataCount(const TVec<TIntFltKdV>& FtrVV);
    /// methods that return the dimension of the data
    static int GetDataDim(const TFltVV& X);
    static int GetDataDim(const TVec<TIntFltKdV>& FtrVV);
    /// set column of the matrix
    static void SetCol(TFltVV& FtrVV, const int& ColN, const TFltV& Col);
    static void SetCol(TFltVV& FtrVV, const int& ColN, const TIntFltKdV& Col);
    static void SetCol(TVec<TIntFltKdV>& FtrVV, const int& ColN, const TIntFltKdV& Col);
    static void SetCol(TVec<TIntFltKdV>& FtrVV, const int& ColN, const TFltV& Col);
    /// get column/cluster of the matrix
    static void GetCol(const TFltVV& FtrVV, const int& ColN, TFltV& Col);
    static void GetCol(const TVec<TIntFltKdV>& FtrVV, const int& ColN, TIntFltKdV& Col);

private:
    inline void SelectRndCentroid(const TFltVV& FtrVV, const int& CentroidN);
    inline void SelectRndCentroid(const TVec<TIntFltKdV>& FtrVV, const int& CentroidN);

    void InitCentroids(TFltVV& CentroidVV, const TFltVV& FtrVV, const TIntV& CentroidNV, const int& K);
    void InitCentroids(TFltVV& CentroidVV, const TVec<TIntFltKdV>& FtrVV, const TIntV& CentroidNV, const int& K);
    void InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TFltVV& FtrVV, const TIntV& CentroidNV, const int& K);
    void InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TVec<TIntFltKdV>& FtrVV, const TIntV& CentroidNV, const int& K);

    void InitCentroids(TFltVV& CentroidVV, const TFltVV& FtrVV);
    void InitCentroids(TFltVV& CentroidVV, const TVec<TIntFltKdV>& FtrVV);
    void InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TFltVV& FtrVV);
    void InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TVec<TIntFltKdV>& FtrVV);

protected:
    /// type
    virtual const TStr GetType() const = 0;
};


///////////////////////////////////////////
// K-Means
template<class TCentroidType>
class TDnsKMeans : public TAbsKMeans<TCentroidType> {
private:
    const TInt K;
public:
    TDnsKMeans(const int& K, const TRnd& Rnd = TRnd(0), const PDist& Dist=TEuclDist::New());
    TDnsKMeans(TSIn& SIn);

    static TPt<TAbsKMeans<TCentroidType>> New(const int& K, const TRnd& Rnd=TRnd(), TDist* Dist=new TEuclDist);
/*    static TPt<TAbsKMeans<TCentroidType>> New(TSIn& SIn)
            { return new TDnsKMeans<TCentroidType>(SIn); }
            */
    // saves the model to the output stream
    void Save(TSOut& SOut) const;
    void Apply(const TFltVV& FtrVV, const bool& AllowEmptyP = true, const int& MaxIter = 10000,
        const PNotify& Notify = TNotify::NullNotify);
    void Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP = true, const int& MaxIter = 10000,
        const PNotify& Notify = TNotify::NullNotify);

    template<class TInitCentroidMatType>
    void Apply(const TFltVV& FtrVV, const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const PNotify& Notify=TNotify::NullNotify, const TInitCentroidMatType& InitCentroidMat = TInitCentroidMatType());
    template<class TInitCentroidMatType>
    void Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP=true, const int& MaxIter = 10000,
             const PNotify& Notify=TNotify::NullNotify, const TInitCentroidMatType& InitCentroidMat = TInitCentroidMatType());

protected:

    template<class TDataType, class TInitCentroidType>
    void Apply(const TDataType& FtrVV, const int& NInst, const int& Dim, const bool& AllowEmptyP,
        const int& MaxIter, const PNotify& Notify, const TInitCentroidType& InitCentroidMat);

    const TStr GetType() const { return "kmeans"; }
};

///////////////////////////////////////////
// DPMeans
template<class TCentroidType>
class TDpMeans : public TAbsKMeans<TCentroidType> {
private:
    const TFlt Lambda;
    const TInt MnClusts;
    const TInt MxClusts;
public:
    TDpMeans(const TFlt& Lambda, const TInt& MnClusts=1, const TInt& MxClusts=TInt::Mx,
            const TRnd& Rnd=TRnd(), const PDist& Dist=TEuclDist::New());
    TDpMeans(TSIn& SIn);

    // saves the model to the output stream
    void Save(TSOut& SOut) const;

    void Apply(const TFltVV& FtrVV, const bool& AllowEmptyP=true, const int& MaxIter=10000,
        const PNotify& Notify=TNotify::NullNotify);
    void Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP=true, const int& MaxIter=10000,
        const PNotify& Notify=TNotify::NullNotify);

    void Apply(const TFltVV& FtrVV, const TFltVV& InitCentroidMat, const int& MaxIter = 10000,
        const PNotify& Notify = TNotify::NullNotify);
    void Apply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentroidMat,
        const int& MaxIter = 10000, const PNotify& Notify = TNotify::NullNotify);

protected:
    const TStr GetType() const { return "dpmeans"; }

private:
    // Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
    // the point is assigned to
    template <class TDataType>
    void Apply(const TDataType& FtrVV, const int& NInst, const int& Dim,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify);

    template <class TDataType>
    inline void AddCentroid(const TDataType& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2,
        TFltV& TempK, TCentroidType& TempDxK, const int& InstN);
};

// typedefs
typedef TDnsKMeans<TFltVV> TDenseKMeans;
typedef TDnsKMeans<TVec<TIntFltKdV>> TSparseKMeans;

typedef TPt<TAbsKMeans<TFltVV>> PDenseKMeans;
typedef TPt<TAbsKMeans<TFltVV>> PSparseKMeans;

//============================================================
// CLUSTERING METHODS - IMPLEMENTATIONS
//============================================================

// implementations
template<class TCentroidType>
TAbsKMeans<TCentroidType>::TAbsKMeans(const TRnd& _Rnd, const PDist& _Dist) :
        CentroidVV(),
        Dist(_Dist),
        Rnd(_Rnd) {}

template<class TCentroidType>
TAbsKMeans<TCentroidType>::TAbsKMeans(TSIn& SIn) :
        CentroidVV(SIn),
        Dist(TDist::Load(SIn)),
        Rnd(SIn) {}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::Save(TSOut& SOut) const {
    GetType().Save(SOut);
    CentroidVV.Save(SOut);
    Dist->Save(SOut);
    Rnd.Save(SOut);
}

template<class TCentroidType>
TAbsKMeans<TCentroidType>* TAbsKMeans<TCentroidType>::LoadPtr(TSIn& SIn) {
    TStr Type(SIn);

    if (Type == "kmeans") {
        return new TDnsKMeans<TCentroidType>(SIn);
    }
    else if (Type == "dpmeans") {
        return new TDpMeans<TCentroidType>(SIn);
    }
    else {
        throw TExcept::New("Invalid clustering type: " + Type);
    }
}

template<class TCentroidType>
TPt<TAbsKMeans<TCentroidType>> TAbsKMeans<TCentroidType>::Load(TSIn& SIn) {
    return LoadPtr(SIn);
}

template<>
inline void TAbsKMeans<TFltVV>::PermutateCentroids(const TIntV& Mapping) {
    EAssert(Mapping.Len() == CentroidVV.GetCols());
    TFltVV CentroidsTemp = CentroidVV;
    TVec<TIntFltKdV> Perm; Perm.Gen(CentroidVV.GetCols());
    for (int ColN = 0; ColN < CentroidVV.GetCols(); ColN++) {
        Perm[Mapping[ColN]].Add(TIntFltKd(ColN, 1));
    }
    TLinAlg::Multiply(CentroidsTemp, Perm, CentroidVV);
}

template<>
inline void TAbsKMeans<TVec<TIntFltKdV>>::PermutateCentroids(const TIntV& Mapping) {
    EAssert(Mapping.Len() == CentroidVV.Len());
    TVec<TIntFltKdV> CentroidsTemp = CentroidVV;
    TVec<TIntFltKdV> Perm; Perm.Gen(CentroidVV.Len());
    for (int ColN = 0; ColN < CentroidVV.Len(); ColN++) {
        Perm[Mapping[ColN]].Add(TIntFltKd(ColN, 1));
    }
    TLinAlg::Multiply(CentroidsTemp, Perm, CentroidVV);
}

template<class TCentroidType>
template<class TVectorType>
void TAbsKMeans<TCentroidType>::GetCentroid(const int& ClustN, TVectorType& FtrV) const {
    EAssert(0 <= ClustN && ClustN < GetClusts());
    GetCol(CentroidVV, ClustN, FtrV);
}

template <class TCentroidType>
template <class TMatType>
void TAbsKMeans<TCentroidType>::Assign(const TMatType& FtrVV, TIntV& AssignV) const {
    TFltVV DistVV;	Dist->GetQuasiDistVV(CentroidVV, FtrVV, DistVV);
    TLinAlgSearch::GetColMinIdxV(DistVV, AssignV);
}

template<class TCentroidType>
template<class TDataType>
double TAbsKMeans<TCentroidType>::GetDist(const int& ClustN, const TDataType& Pt) const {
    TFltV Centroid;	GetCol(CentroidVV, ClustN, Centroid);
    return TLinAlg::EuclDist(Centroid, Pt);
}

template<class TCentroidType>
template<class TDataType>
void TAbsKMeans<TCentroidType>::GetCentroidDistV(const TDataType& FtrV, TFltV& DistV) const {
    Dist->GetDistV(CentroidVV, FtrV, DistV);
}

template<class TCentroidType>
template<class TDataType>
void TAbsKMeans<TCentroidType>::GetDistVV(const TDataType& FtrVV, TFltVV& DistVV) const {
    Dist->GetDistVV(CentroidVV, FtrVV, DistVV);
}

template <class TCentroidType>
inline void TAbsKMeans<TCentroidType>::RemoveCentroids(const TIntV& CentroidIdV) {
    EAssertR(false, "TAbsKMeans<TCentroidType>::RemoveCentroids: Not implemented for this type of KMeans!");
}

template <>
inline void TAbsKMeans<TFltVV>::RemoveCentroids(const TIntV& CentroidIdV) {
    CentroidVV.DelCols(CentroidIdV);
}

template <>
inline void TAbsKMeans<TVec<TIntFltKdV>>::RemoveCentroids(const TIntV& CentroidIdV) {
    CentroidVV.Del(CentroidIdV);
}

template<class TCentroidType>
inline void TAbsKMeans<TCentroidType>::SelectRndCentroid(const TFltVV& FtrVV, const int& CentroidN) {
    const int RndRecN = Rnd.GetUniDevInt(GetDataCount(FtrVV));
    TFltV RndRecFtrV;	GetCol(FtrVV, RndRecN, RndRecFtrV);
    SetCol(CentroidVV, CentroidN, RndRecFtrV);
}

template<class TCentroidType>
inline void TAbsKMeans<TCentroidType>::SelectRndCentroid(const TVec<TIntFltKdV>& FtrVV, const int& CentroidN) {
    const int RndRecN = Rnd.GetUniDevInt(GetDataCount(FtrVV));
    TIntFltKdV RndRecFtrV;	GetCol(FtrVV, RndRecN, RndRecFtrV);
    SetCol(CentroidVV, CentroidN, RndRecFtrV);
}

template<class TCentroidType>
template<class TDataType>
inline void TAbsKMeans<TCentroidType>::UpdateCentroids(const TDataType& FtrVV, const int& NInst, TIntV& AssignV,
    const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TCentroidType& TempDxKV,
    TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& XLenDistHelpV, TFltV& CLenDistHelpV, const bool& AllowEmptyP) {

    const int K = GetDataCount(CentroidVV);

    // I. create a sparse matrix (coordinate representation) that encodes the closest centroids
    TSparseColMatrix AssignMat(NInst, K);

    bool ExistsEmpty;
    int LoopN = 0;
    do {
        ExistsEmpty = false;

        TSparseOps<TInt, TFlt>::CoordinateCreateSparseColMatrix(RangeN, AssignV, OnesN, AssignMat.ColSpVV, K);

        // II. compute the number of points that belong to each centroid, invert
        AssignMat.MultiplyT(OnesN, TempK);

        // invert
        for (int ClustN = 0; ClustN < K; ClustN++) {
            // check if the cluster is empty, if we don't allow empty clusters, select a
            // random point as the centroid
            if (TempK[ClustN] == 0.0 && !AllowEmptyP) {	// don't allow empty clusters
                // select a random point and create a new centroid from it
                SelectRndCentroid(FtrVV, ClustN);
                Dist->UpdateCLenDistHelpV(CentroidVV, CLenDistHelpV);
                Assign(FtrVV, XLenDistHelpV, CLenDistHelpV, AssignV);
                ExistsEmpty = true;
                break;
            }
            TempK[ClustN] = 1.0 / (TempK[ClustN] + 1.0);
        }
    } while (ExistsEmpty && ++LoopN < 10);


    // III. compute the centroids
    // compute: CentroidMat = ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag;
    TLinAlgTransform::Diag(TempK, TempKxKSpVV);

    // 1) FtrVV * AssignIdxMat
    TLinAlg::Multiply(FtrVV, AssignMat.ColSpVV, TempDxKV);
    // 2) (FtrVV * AssignIdxMat) + CentroidMat
    TLinAlg::LinComb(1, TempDxKV, 1, CentroidVV, TempDxKV);
    // 3) ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag
    TLinAlg::Multiply(TempDxKV, TempKxKSpVV, CentroidVV);
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TFltVV& CentroidVV, const TFltVV& FtrVV, const TIntV& CentroidNV, const int& K) {
    const int Dim = GetDataDim(FtrVV);
    // construct the centroid matrix
    CentroidVV.Gen(Dim, K);
    for (int i = 0; i < K; i++) {
        const int ColN = CentroidNV[i];
        for (int RowN = 0; RowN < Dim; RowN++) {
            CentroidVV.PutXY(RowN, i, FtrVV(RowN, ColN));
        }
    }
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TFltVV& CentroidVV, const TVec<TIntFltKdV>& FtrVV, const TIntV& CentroidNV, const int& K) {
    const int Dim = GetDataDim(FtrVV);
    // construct the centroid matrix
    CentroidVV.Gen(Dim, K);
    for (int i = 0; i < K; i++) {
        const int ColN = CentroidNV[i];
        const int Els = FtrVV[ColN].Len();
        for (int ElN = 0; ElN < Els; ElN++) {
            CentroidVV.PutXY(FtrVV[ColN][ElN].Key, i, FtrVV[ColN][ElN].Dat);
        }
    }
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TFltVV& FtrVV, const TIntV& CentroidNV, const int& K) {
    const int Dim = GetDataDim(FtrVV);
    // construct the centroid matrix
    CentroidVV.Gen(K);
    for (int i = 0; i < K; i++) {
        const int ColN = CentroidNV[i];
        for (int RowN = 0; RowN < Dim; RowN++) {
            if (FtrVV(RowN, ColN) != 0.0) { CentroidVV[i].Add(TIntFltKd(RowN, FtrVV(RowN, ColN))); }
        }
    }
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TVec<TIntFltKdV>& FtrVV, const TIntV& CentroidNV, const int& K) {
    // construct the centroid matrix
    CentroidVV.Gen(K);
    for (int ClustN = 0; ClustN < K; ClustN++) {
        CentroidVV[ClustN] = FtrVV[CentroidNV[ClustN]];
    }
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TFltVV& CentroidVV, const TFltVV& FtrVV) {
    CentroidVV = FtrVV;
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TFltVV& CentroidVV, const TVec<TIntFltKdV>& FtrVV) {
    const int Rows = GetDataDim(FtrVV);
    TLinAlgTransform::Full(FtrVV, CentroidVV, Rows);
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TFltVV& FtrVV) {
    TLinAlgTransform::Sparse(FtrVV, CentroidVV);
}

template<class TCentroidType>
void TAbsKMeans<TCentroidType>::InitCentroids(TVec<TIntFltKdV>& CentroidVV, const TVec<TIntFltKdV>& FtrVV) {
    CentroidVV = FtrVV;
}

template<class TCentroidType>
template<class TDataType>
inline void TAbsKMeans<TCentroidType>::SelectInitCentroids(const TDataType& FtrVV,
        const int& K, const int& NInst) {

    EAssertR(NInst >= K, "TStateIdentifier::SelectInitCentroids: The number of initial centroids should be less than the number of data points!");

    TIntV CentroidNV(K);

    // generate k random elements
    if (K < NInst / 2) {
        TIntSet TakenSet(K);

        int RecN;
        for (int ClustN = 0; ClustN < K; ClustN++) {
            do {	// expecting max 2 iterations before we get a hit
                RecN = Rnd.GetUniDevInt(NInst);
            } while (TakenSet.IsKey(RecN));

            TakenSet.AddKey(RecN);
            CentroidNV[ClustN] = RecN;
        }
    } else {
        TIntV PermV(NInst);	TLinAlgTransform::RangeV(NInst, PermV);

        TInt Temp;
        for (int i = 0; i < K; i++) {
            const int SwapIdx = Rnd.GetUniDevInt(i, NInst - 1);

            // swap
            Temp = PermV[SwapIdx];
            PermV[SwapIdx] = PermV[i];
            PermV[i] = Temp;

            CentroidNV[i] = PermV[i];
        }
    }

    InitCentroids(CentroidVV, FtrVV, CentroidNV, K);
}

template<class TCentroidType>
template<class TDataType>
inline void TAbsKMeans<TCentroidType>::SelectInitCentroids(const TDataType& FtrVV) {
    InitCentroids(CentroidVV, FtrVV);
}

template<class TCentroidType>
template<class TDataType>
inline void TAbsKMeans<TCentroidType>::Assign(const TDataType& FtrVV, const TFltV& NormX2, const TFltV& NormC2,
    TIntV& AssignV) const {
    TFltVV DistVV;	Dist->GetQuasiDistVV(CentroidVV, FtrVV, NormC2, NormX2, DistVV);
    TLinAlgSearch::GetColMinIdxV(DistVV, AssignV);
}

template<class TCentroidType>
int TAbsKMeans<TCentroidType>::GetDataCount(const TFltVV& FtrVV) {
    return FtrVV.GetCols();
}

template<class TCentroidType>
int TAbsKMeans<TCentroidType>::GetDataCount(const TVec<TIntFltKdV>& FtrVV) {
    return FtrVV.Len();
}

template<class TCentroidType>
int TAbsKMeans<TCentroidType>::GetDataDim(const TFltVV& X) {
    return X.GetRows();
}

template<class TCentroidType>
int TAbsKMeans<TCentroidType>::GetDataDim(const TVec<TIntFltKdV>& FtrVV) {
    // TODO enable inputing dimension through arguments for sparse matrices
    return TLinAlgSearch::GetMaxDimIdx(FtrVV) + 1;
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::SetCol(TFltVV& FtrVV, const int& ColN, const TFltV& Col) {
    FtrVV.SetCol(ColN, Col);
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::SetCol(TFltVV& FtrVV, const int& ColN, const TIntFltKdV& Col) {
    TFltV TempCol; TLinAlgTransform::ToVec(Col, TempCol, FtrVV.GetRows());
    FtrVV.SetCol(ColN, TempCol);
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::SetCol(TVec<TIntFltKdV>& FtrVV, const int& ColN, const TIntFltKdV& Col) {
    FtrVV[ColN] = Col;
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::SetCol(TVec<TIntFltKdV>& FtrVV, const int& ColN, const TFltV& Col) {
    TIntFltKdV TempCol; TLinAlgTransform::ToSpVec(Col, TempCol);
    FtrVV[ColN] = TempCol;
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::GetCol(const TFltVV& FtrVV, const int& ColN, TFltV& Col) {
    FtrVV.GetCol(ColN, Col);
}

template <class TCentroidType>
void TAbsKMeans<TCentroidType>::GetCol(const TVec<TIntFltKdV>& FtrVV, const int& ColN, TIntFltKdV& Col) {
    Col = FtrVV[ColN];
}

template<class TCentroidType>
TDnsKMeans<TCentroidType>::TDnsKMeans(const int& _K, const TRnd& Rnd, const PDist& Dist) :
        TAbsKMeans<TCentroidType>(Rnd, Dist),
        K(_K) {}

template<class TCentroidType>
TDnsKMeans<TCentroidType>::TDnsKMeans(TSIn& SIn) :
        TAbsKMeans<TCentroidType>(SIn),
        K(SIn) {}

template <class TCentroidType>
TPt<TAbsKMeans<TCentroidType>> TDnsKMeans<TCentroidType>::New(const int& K, const TRnd& Rnd, TDist* Dist) {
    return new TDnsKMeans<TCentroidType>(K, Rnd, Dist);
}

template<class TCentroidType>
void TDnsKMeans<TCentroidType>::Save(TSOut& SOut) const {
    TAbsKMeans<TCentroidType>::Save(SOut);
    K.Save(SOut);
}

template <class TCentroidType>
void TDnsKMeans<TCentroidType>::Apply(const TFltVV& FtrVV, const bool& AllowEmptyP,
    const int& MaxIter, const PNotify& Notify) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify, TFltVV());
}

template <class TCentroidType>
void TDnsKMeans<TCentroidType>::Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP,
    const int& MaxIter, const PNotify& Notify) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify, TVec<TIntFltKdV>());
}

template <class TCentroidType>
template<class TInitCentroidMatType>
void TDnsKMeans<TCentroidType>::Apply(const TFltVV& FtrVV, const bool& AllowEmptyP,
        const int& MaxIter, const PNotify& Notify, const TInitCentroidMatType& InitCentroidMat) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify, InitCentroidMat);
}

template <class TCentroidType>
template<class TInitCentroidMatType>
void TDnsKMeans<TCentroidType>::Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP,
        const int& MaxIter, const PNotify& Notify, const TInitCentroidMatType& InitCentroidMat) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify, InitCentroidMat);
}

template<class TCentroidType>
template<class TDataType, class TInitCentroidType>
void TDnsKMeans<TCentroidType>::Apply(const TDataType& FtrVV, const int& NInst, const int& Dim,
    const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify, const TInitCentroidType& InitCentroidMat) {
    EAssertR(K <= NInst, "Matrix should have more columns than K!");

    Notify->OnNotify(TNotifyType::ntInfo, "Executing KMeans ...");

    // assignment vectors
    TIntV AssignIdxV(NInst), OldAssignIdxV(NInst);
    TIntV* AssignIdxVPtr = &AssignIdxV;
    TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
    TIntV* Temp;

    // constant reused variables
    TFltV OnesN;			TLinAlgTransform::OnesV(NInst, OnesN);
    TFltV XLenDistHelpV;	TAbsKMeans<TCentroidType>::Dist->UpdateXLenDistHelpV(FtrVV, XLenDistHelpV);
    TIntV RangeN(NInst);	TLinAlgTransform::RangeV(NInst, RangeN);

    // reused variables
    TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
    TFltV CLenDistHelpV(K);
    TFltV TempK(K);						// (dimension k)
    TCentroidType TempDxK;				// (dimension d x k)
    TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

    // select initial centroids
    if (InitCentroidMat.Empty()) {
        TAbsKMeans<TCentroidType>::SelectInitCentroids(FtrVV, K, NInst);
    }
    else {
        EAssertR(TAbsKMeans<TCentroidType>::GetDataCount(InitCentroidMat) == K, "Number of columns must be equal to K!");
        TAbsKMeans<TCentroidType>::SelectInitCentroids(InitCentroidMat);
    }

    // do the work
    for (int IterN = 0; IterN < MaxIter; IterN++) {
        if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }

        // get the distance of each of the points to each of the centroids
        // and assign the instances
        TAbsKMeans<TCentroidType>::Dist->UpdateCLenDistHelpV(TAbsKMeans<TCentroidType>::CentroidVV, CLenDistHelpV);
        TAbsKMeans<TCentroidType>::Dist->GetQuasiDistVV(TAbsKMeans<TCentroidType>::CentroidVV, FtrVV, CLenDistHelpV, XLenDistHelpV, ClustDistVV);

        TLinAlgSearch::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

        
        // if the assignment hasn't changed then terminate the loop
        if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
            Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", IterN);
            break;
        }

        // recompute the means
        TAbsKMeans<TCentroidType>::UpdateCentroids(FtrVV, NInst, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV, XLenDistHelpV, CLenDistHelpV, AllowEmptyP);

        // swap the old and new assign vectors
        Temp = AssignIdxVPtr;
        AssignIdxVPtr = OldAssignIdxVPtr;
        OldAssignIdxVPtr = Temp;
    }

    EAssertR(!TLinAlgCheck::ContainsNan(TAbsKMeans<TCentroidType>::CentroidVV), "TDnsKMeans<TCentroidType>::Apply: Found NaN in the centroids!");
}

template<class TCentroidType>
TDpMeans<TCentroidType>::TDpMeans(const TFlt& _Lambda, const TInt& _MnClusts, const TInt& _MxClusts,
    const TRnd& Rnd, const PDist& Dist) :
    TAbsKMeans<TCentroidType>(Rnd, Dist),
    Lambda(_Lambda),
    MnClusts(_MnClusts),
    MxClusts(_MxClusts) {

    EAssertR(MnClusts > 0, "TDpMeans::TDpMeans: The minimal number of clusters should be greater than 0!");
    EAssertR(MxClusts >= MnClusts, "TDpMeans::TDpMeans: The max number of clusters should be greater than the min number of clusters!");
}

template<class TCentroidType>
TDpMeans<TCentroidType>::TDpMeans(TSIn& SIn) :
        TAbsKMeans<TCentroidType>(SIn),
        Lambda(SIn),
        MnClusts(SIn),
        MxClusts(SIn) {}

template<class TCentroidType>
void TDpMeans<TCentroidType>::Save(TSOut& SOut) const {
    TAbsKMeans<TCentroidType>::Save(SOut);
    Lambda.Save(SOut);
    MnClusts.Save(SOut);
    MxClusts.Save(SOut);
}

template<class TCentroidType>
void TDpMeans<TCentroidType>::Apply(const TFltVV& FtrVV, const bool& AllowEmptyP,
        const int& MaxIter, const PNotify& Notify) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
}

template<class TCentroidType>
void TDpMeans<TCentroidType>::Apply(const TVec<TIntFltKdV>& FtrVV, const bool& AllowEmptyP,
        const int& MaxIter, const PNotify& Notify) {
    const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
    EAssertR(Dim > 0, "The input matrix doesn't have any features!");
    Apply(FtrVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
}

template<class TCentroidType>
template<class TDataType>
inline void TDpMeans<TCentroidType>::Apply(const TDataType& FtrVV, const int& NInst,
        const int& Dim, const bool& AllowEmptyP, const int& MaxIter,
        const PNotify& Notify) {
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
    TAbsKMeans<TCentroidType>::SelectInitCentroids(FtrVV, MnClusts, NInst);

    // const variables, reused throughtout the procedure
    TFltV OnesN;			TLinAlgTransform::OnesV(NInst, OnesN);
    TFltV NormX2;			TAbsKMeans<TCentroidType>::Dist->UpdateXLenDistHelpV(FtrVV, NormX2);
    TIntV RangeN(NInst);	TLinAlgTransform::RangeV(NInst, RangeN);


    // temporary reused variables
    TFltV FtrV;							// (dimension d)
    TFltV MinClustDistV;				// (dimension n)
    TFltVV ClustDistVV(K, NInst);		// (dimension k x n)
    TFltV NormC2(K);					// (dimension k)
    TFltV TempK(K);						// (dimension k)
    TCentroidType TempDxK;				// (dimension d x k)
    TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

    int IterN = 0;
    while (IterN++ < MaxIter) {
        if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }

        // compute the distance matrix to all the centroids and assignments
        TAbsKMeans<TCentroidType>::Dist->UpdateCLenDistHelpV(TAbsKMeans<TCentroidType>::CentroidVV, NormC2);
        TAbsKMeans<TCentroidType>::Dist->GetQuasiDistVV(TAbsKMeans<TCentroidType>::CentroidVV, FtrVV, NormC2, NormX2, ClustDistVV);
        TLinAlgSearch::GetColMinIdxV(ClustDistVV, *AssignIdxVPtr);

        // check if we need to increase the number of centroids
        if (K < MxClusts) {
            TLinAlgSearch::GetColMinV(ClustDistVV, MinClustDistV);

            const int NewCentrIdx = TLinAlgSearch::GetMaxIdx(MinClustDistV);
            const double MaxDist = MinClustDistV[NewCentrIdx];

            if (MaxDist > LambdaSq) {
                K++;
                AddCentroid(FtrVV, ClustDistVV, NormC2, TempK, TempDxK, NewCentrIdx);
                TempKxKSpVV.Gen(K);
                (*AssignIdxVPtr)[NewCentrIdx] = K - 1;
                Notify->OnNotifyFmt(TNotifyType::ntInfo, "Max distance to centroid: %.3f, number of clusters: %d ...", TMath::Sqrt(MaxDist), K);
            }
        }

        // check if converged
        if (*AssignIdxVPtr == *OldAssignIdxVPtr) {
            Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", IterN);
            break;
        }

        // recompute the centroids
        TAbsKMeans<TCentroidType>::UpdateCentroids(FtrVV, NInst, *AssignIdxVPtr, OnesN, RangeN, TempK, TempDxK, TempKxKSpVV, NormX2, NormC2, AllowEmptyP);

        // swap old and new assign vectors
        Temp = AssignIdxVPtr;
        AssignIdxVPtr = OldAssignIdxVPtr;
        OldAssignIdxVPtr = Temp;
    }

    EAssertR(!TLinAlgCheck::ContainsNan(TAbsKMeans<TCentroidType>::CentroidVV), "TDpMeans<TCentroidType>::Apply: Found NaN in the centroids!");
}

template<>
template<>
inline void TDpMeans<TFltVV>::AddCentroid(const TFltVV& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2, 
    TFltV& TempK, TFltVV& TempDxK, const int& InstN) {
    TFltV FtrV;  FtrVV.GetCol(InstN, FtrV);
    CentroidVV.AddCol(FtrV);
    ClustDistVV.AddXDim();
    NormC2.Add(0);
    TempK.Add(0);
    TempDxK.AddYDim();
}

template<>
template<>
inline void TDpMeans<TFltVV>::AddCentroid(const TVec<TIntFltKdV>& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2,
    TFltV& TempK, TFltVV& TempDxK, const int& InstN) {
    TIntFltKdV FtrV; GetCol(FtrVV, InstN, FtrV);
    TFltV DenseFtrV; TLinAlgTransform::ToVec(FtrV, DenseFtrV, GetDataDim(FtrVV));
    CentroidVV.AddCol(DenseFtrV);
    ClustDistVV.AddXDim();
    NormC2.Add(0);
    TempK.Add(0);
    TempDxK.AddYDim();
}

template<>
template<>
inline void TDpMeans<TVec<TIntFltKdV>>::AddCentroid(const TFltVV& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2,
    TFltV& TempK, TVec<TIntFltKdV>& TempDxK, const int& InstN) {
    TFltV FtrV; FtrVV.GetCol(InstN, FtrV);
    TIntFltKdV SparseFtrV; TLinAlgTransform::ToSpVec(FtrV, SparseFtrV);
    CentroidVV.Add(SparseFtrV);
    ClustDistVV.AddXDim();
    NormC2.Add(0);
    TempK.Add(0);
    TempDxK.Add(TIntFltKdV());
}

template<>
template<>
inline void TDpMeans<TVec<TIntFltKdV>>::AddCentroid(const TVec<TIntFltKdV>& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2,
    TFltV& TempK, TVec<TIntFltKdV>& TempDxK, const int& InstN) {
    const TIntFltKdV& FtrV = FtrVV[InstN];
    CentroidVV.Add(FtrV);
    ClustDistVV.AddXDim();
    NormC2.Add(0);
    TempK.Add(0);
    TempDxK.Add(TIntFltKdV());
}

//============================================================
// HIERARCHICAL CLUSTERING
//============================================================

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

        TFltVV ClustDistVV;	TDist().GetQuasiDistVV(X,X, ClustDistVV);
        TIntV ItemCountV;	TLinAlgTransform::OnesV(NInst, ItemCountV);//TVector::Ones(NInst);

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

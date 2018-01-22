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

    TBool CalcDistQualP;
    TFlt RelMeanCentroidDist {TFlt::NInf};
public:

    TAbsKMeans(const TRnd& Rnd, const PDist& Dist=TEuclDist::New(),
            const bool& CalcDistQualP=false);
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

    // APPLY METHODS
    template <class TDataType>
    void Apply(const TDataType& FtrVV, const bool& AllowEmptyP=true,
            const int& MxIter=10000, const TWPt<TNotify>& Notify=TNotify::NullNotify());
    template <class TDataType, class TInitCentroidType>
    void Apply(const TDataType& FtrVV, const TInitCentroidType& InitCentVV,
            const bool& AllowEmptyP=true, const int& MxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify());

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

    // QUALITY MEASURES
    /// returns the mormalized weighted distance between the vectors and their centroids
    /// using the following formula:
    /// d = \frac{sum_i p_i*sum_j d(x_j,c_i) / n_i}{sum_{k=1}^n d(x_k, mu) / n}
    ///   = \frac{sum_{i,j} d(c_i,x_j)}{sum_k d(x_k, mu)}
    double GetRelMeanCentroidDist() const;

    /// returns the class used to calculate distances
    const PDist& GetDistMetric() const { return Dist; }

    inline void RemoveCentroids(const TIntV& CentroidIdV);

protected:

    // VIRTUAL APPLY FUNCTIONS, SUBCLASSES MUST OVERRIDE
    virtual void VirtApply(const TFltVV& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify()) = 0;
    virtual void VirtApply(const TFltVV& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify()) = 0;
    virtual void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify()) = 0;
    virtual void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify()) = 0;

    /// can still optimize
    template<class TDataType>
    void UpdateCentroids(const TDataType& FtrVV, const int& NInst, TIntV& AssignV,
            const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK, TCentroidType& TempDxKV,
            TCentroidType& TempDxKV2, TVec<TIntFltKdV>& TempKxKSpVV, const TFltV& NormX2,
            TFltV& NormC2, const bool& AllowEmptyP);

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
    using TBase = TAbsKMeans<TCentroidType>;
public:
    TDnsKMeans(const int& K, const TRnd& Rnd = TRnd(0), const PDist& Dist=TEuclDist::New(),
            const bool& CalcDistQualP=false);
    TDnsKMeans(TSIn& SIn);

    static TPt<TAbsKMeans<TCentroidType>> New(const int& K, const TRnd& Rnd=TRnd(),
            TDist* Dist=new TEuclDist, const bool& CalcDistQualP=false);

    // saves the model to the output stream
    void Save(TSOut& SOut) const;

protected:
    void VirtApply(const TFltVV& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify());
    void VirtApply(const TFltVV& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify());
    void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify());
    void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify());

    const TStr GetType() const { return "kmeans"; }

private:
    template<class TDataType, class TInitCentroidType>
    void VirtApply(const TDataType& FtrVV, const TInitCentroidType& InitCentVV,
            const int& NInst, const int& Dim, const bool& AllowEmptyP, const int& MaxIter,
            const TWPt<TNotify>& Notify);

    const TInt K;
};

///////////////////////////////////////////
// DPMeans
template<class TCentroidType>
class TDpMeans : public TAbsKMeans<TCentroidType> {
    using TBase = TAbsKMeans<TCentroidType>;
private:
    const TFlt Lambda;
    const TInt MnClusts;
    const TInt MxClusts;
public:
    TDpMeans(const TFlt& Lambda, const TInt& MnClusts=1, const TInt& MxClusts=TInt::Mx,
            const TRnd& Rnd=TRnd(), const PDist& Dist=TEuclDist::New(),
            const bool& CalcDistQualP=false);
    TDpMeans(TSIn& SIn);

    // saves the model to the output stream
    void Save(TSOut& SOut) const;

protected:
    void VirtApply(const TFltVV& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify());
    void VirtApply(const TFltVV& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify=TNotify::NullNotify());
    void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify());
    void VirtApply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP=true, const int& MaxIter=10000,
            const TWPt<TNotify>& Notify = TNotify::NullNotify());

    const TStr GetType() const { return "dpmeans"; }

private:
    // Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
    // the point is assigned to
    template <class TDataType, class TInitCentVV>
    void VirtApply(const TDataType& FtrVV, const TInitCentVV& InitCentVV,
            const int& NInst, const int& Dim, const bool& AllowEmptyP, const int& MaxIter,
            const TWPt<TNotify>& Notify);

    template <class TDataType>
    inline void AddCentroid(const TDataType& FtrVV, TFltVV& ClustDistVV, TFltV& NormC2,
        TFltV& TempK, TCentroidType& TempDxK, TCentroidType& TempDxK2, const int& InstN);
};

// typedefs
typedef TDnsKMeans<TFltVV> TDenseKMeans;
typedef TDnsKMeans<TVec<TIntFltKdV>> TSparseKMeans;

typedef TPt<TAbsKMeans<TFltVV>> PDenseKMeans;
typedef TPt<TAbsKMeans<TFltVV>> PSparseKMeans;


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
    static void MakeDendro(const TFltVV& X, TIntIntFltTrV& MergeV, const TWPt<TNotify>& Notify) {
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

#include "clustering.hpp"

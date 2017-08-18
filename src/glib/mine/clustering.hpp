
namespace TClustering {
    ///////////////////////////////////
    /// Euclidean Distance
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


    //////////////////////////////////////
    /// Cosine Distance
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

    ////////////////////////////////////////
    /// K-Means - base class
    template<class TCentroidType>
    TAbsKMeans<TCentroidType>::TAbsKMeans(const TRnd& _Rnd, const PDist& _Dist,
                const bool& _CalcDistQualP) :
            CentroidVV(),
            Dist(_Dist),
            Rnd(_Rnd),
            CalcDistQualP(_CalcDistQualP) {}

    template<class TCentroidType>
    TAbsKMeans<TCentroidType>::TAbsKMeans(TSIn& SIn) :
            CentroidVV(SIn),
            Dist(TDist::Load(SIn)),
            Rnd(SIn),
            CalcDistQualP(SIn),
            RelMeanCentroidDist(SIn) {}

    template<class TCentroidType>
    void TAbsKMeans<TCentroidType>::Save(TSOut& SOut) const {
        GetType().Save(SOut);
        CentroidVV.Save(SOut);
        Dist->Save(SOut);
        Rnd.Save(SOut);
        CalcDistQualP.Save(SOut);
        RelMeanCentroidDist.Save(SOut);
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
    template <class TDataType>
    void TAbsKMeans<TCentroidType>::Apply(const TDataType& FtrVV, const bool& AllowEmptyP,
            const int& MxIter, const PNotify& Notify) {
        TFltVV EmptyCentroidVV;
        Apply(FtrVV, EmptyCentroidVV, AllowEmptyP, MxIter, Notify);
    }

    template <class TCentroidType>
    template <class TDataType, class TInitCentroidType>
    void TAbsKMeans<TCentroidType>::Apply(const TDataType& FtrVV, const TInitCentroidType& InitCentVV,
            const bool& AllowEmptyP, const int& MxIter, const PNotify& Notify) {
        // compute the clusters
        VirtApply(FtrVV, InitCentVV, AllowEmptyP, MxIter, Notify);
        // calculate quality measures (if necessary)
        if (CalcDistQualP) {
            // mean of the dataset
            TFltV DataMeanFtrV; TLinAlgStat::Mean(FtrVV, DataMeanFtrV, TMatDim::mdRows);
            // distance between all clusters to all the feature vectors
            TFltVV DistVV;  GetDistVV(FtrVV, DistVV);
            // assignments of feature vectors to clusters
            TIntV AssignV;  Assign(FtrVV, AssignV);

            double ClustDistSum = 0;
            // calculate the sum of distances to centroids
            for (int RecN = 0; RecN < AssignV.Len(); ++RecN) {
                const int ClustN = AssignV[RecN];
                ClustDistSum += DistVV(ClustN, RecN);
            }

            // calculate the sum of distances to the center
            TFltV FtrVMeanDistV;    Dist->GetDistV(FtrVV, DataMeanFtrV, FtrVMeanDistV);
            const double GlobalDistSum = TLinAlg::SumVec(FtrVMeanDistV);

            RelMeanCentroidDist = ClustDistSum / GlobalDistSum;
        }
    }

    template <class TCentroidType>
    template <class TMatType>
    void TAbsKMeans<TCentroidType>::Assign(const TMatType& FtrVV, TIntV& AssignV) const {
        TFltVV DistVV;	Dist->GetQuasiDistVV(CentroidVV, FtrVV, DistVV);
        TLinAlgSearch::GetColMinIdxV(DistVV, AssignV);
    }

    template<class TCentroidType>
    template<class TDataType>
    double TAbsKMeans<TCentroidType>::GetDist(const int& ClustN, const TDataType& FtrV) const {
        TFltV ClustDistV;   GetCentroidDistV(FtrV, ClustDistV);
        return ClustDistV[ClustN];
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
    double TAbsKMeans<TCentroidType>::GetRelMeanCentroidDist() const {
        EAssert(CalcDistQualP);
        return RelMeanCentroidDist;
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
    void TAbsKMeans<TCentroidType>::UpdateCentroids(const TDataType& FtrVV, const int& NInst,
            TIntV& AssignV, const TFltV& OnesN, const TIntV& RangeN, TFltV& TempK,
            TCentroidType& TempDxKV, TCentroidType& TempDxKV2, TVec<TIntFltKdV>& TempKxKSpVV,
            const TFltV& XLenDistHelpV, TFltV& CLenDistHelpV, const bool& AllowEmptyP) {

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
        TLinAlg::LinComb(1, TempDxKV, 1, CentroidVV, TempDxKV2);
        // 3) ((FtrVV * AssignIdxMat) + CentroidMat) * ColSumDiag
        TLinAlg::Multiply(TempDxKV2, TempKxKSpVV, CentroidVV);
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


    ////////////////////////////////////////
    /// K-Means
    template<class TCentroidType>
    TDnsKMeans<TCentroidType>::TDnsKMeans(const int& _K, const TRnd& Rnd, const PDist& Dist,
                const bool& CalcDistQualP) :
            TAbsKMeans<TCentroidType>(Rnd, Dist, CalcDistQualP),
            K(_K) {}

    template<class TCentroidType>
    TDnsKMeans<TCentroidType>::TDnsKMeans(TSIn& SIn) :
            TAbsKMeans<TCentroidType>(SIn),
            K(SIn) {}

    template <class TCentroidType>
    TPt<TAbsKMeans<TCentroidType>> TDnsKMeans<TCentroidType>::New(const int& K, const TRnd& Rnd, TDist* Dist,
            const bool& CalcDistQualP) {
        return new TDnsKMeans<TCentroidType>(K, Rnd, Dist, CalcDistQualP);
    }

    template<class TCentroidType>
    void TDnsKMeans<TCentroidType>::Save(TSOut& SOut) const {
        TAbsKMeans<TCentroidType>::Save(SOut);
        K.Save(SOut);
    }

    template <class TCentroidType>
    void TDnsKMeans<TCentroidType>::VirtApply(const TFltVV& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }
    
    template <class TCentroidType>
    void TDnsKMeans<TCentroidType>::VirtApply(const TFltVV& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template <class TCentroidType>
    void TDnsKMeans<TCentroidType>::VirtApply(const TVec<TIntFltKdV>& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template <class TCentroidType>
    void TDnsKMeans<TCentroidType>::VirtApply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template<class TCentroidType>
    template<class TDataType, class TInitCentroidType>
    void TDnsKMeans<TCentroidType>::VirtApply(const TDataType& FtrVV,
            const TInitCentroidType& InitCentroidVV,
            const int& NInst, const int& Dim, const bool& AllowEmptyP, const int& MaxIter,
            const PNotify& Notify) {
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
        TCentroidType TempDxK2;				// (dimension d x k)
        TVec<TIntFltKdV> TempKxKSpVV(K);	// (dimension k x k)

        // select initial centroids
        if (InitCentroidVV.Empty()) {
            TAbsKMeans<TCentroidType>::SelectInitCentroids(FtrVV, K, NInst);
        }
        else {
            EAssertR(TAbsKMeans<TCentroidType>::GetDataCount(InitCentroidVV) == K, "Number of columns must be equal to K!");
            TAbsKMeans<TCentroidType>::SelectInitCentroids(InitCentroidVV);
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
            TAbsKMeans<TCentroidType>::UpdateCentroids(
                    FtrVV,
                    NInst,
                    *AssignIdxVPtr,
                    OnesN,
                    RangeN,
                    TempK,
                    TempDxK,
                    TempDxK2,
                    TempKxKSpVV,
                    XLenDistHelpV,
                    CLenDistHelpV,
                    AllowEmptyP
            );

            // swap the old and new assign vectors
            Temp = AssignIdxVPtr;
            AssignIdxVPtr = OldAssignIdxVPtr;
            OldAssignIdxVPtr = Temp;
        }

        EAssertR(!TLinAlgCheck::ContainsNan(TAbsKMeans<TCentroidType>::CentroidVV), "TDnsKMeans<TCentroidType>::Apply: Found NaN in the centroids!");
    }

    ////////////////////////////////////////
    /// DP-Means
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

    template <class TCentroidType>
    void TDpMeans<TCentroidType>::VirtApply(const TFltVV& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }
    
    template <class TCentroidType>
    void TDpMeans<TCentroidType>::VirtApply(const TFltVV& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template <class TCentroidType>
    void TDpMeans<TCentroidType>::VirtApply(const TVec<TIntFltKdV>& FtrVV, const TFltVV& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template <class TCentroidType>
    void TDpMeans<TCentroidType>::VirtApply(const TVec<TIntFltKdV>& FtrVV, const TVec<TIntFltKdV>& InitCentVV,
            const bool& AllowEmptyP, const int& MaxIter, const PNotify& Notify) {
        const int Dim = TAbsKMeans<TCentroidType>::GetDataDim(FtrVV);
        EAssertR(Dim > 0, "The input matrix doesn't have any features!");
        VirtApply(FtrVV, InitCentVV, TAbsKMeans<TCentroidType>::GetDataCount(FtrVV), Dim, AllowEmptyP, MaxIter, Notify);
    }

    template<class TCentroidType>
    template<class TDataType, class TInitCentVV>
    void TDpMeans<TCentroidType>::VirtApply(const TDataType& FtrVV, const TInitCentVV& InitCentVV,
            const int& NInst, const int& Dim, const bool& AllowEmptyP, const int& MaxIter,
            const PNotify& Notify) {
        EAssertR(MnClusts <= NInst, "Matrix should have more rows then the min number of clusters!");
        EAssertR(MnClusts <= MxClusts, "Minimum number of cluster should be less than the maximum.");
        EAssertR(InitCentVV.Empty() || TAbsKMeans<TCentroidType>::GetDataCount(InitCentVV) >= MnClusts, "Invalid number of initial clusters when compred to the minimum number!");

        Notify->OnNotifyFmt(TNotifyType::ntInfo, "Executing DPMeans with paramters r=%.3f, minK=%d, maxK=%d ...", Lambda, MnClusts, MxClusts);

        const double LambdaSq = Lambda*Lambda;

        int K = MnClusts;

        // assignment vectors and their pointers, so we don't copy
        TIntV AssignIdxV, OldAssignIdxV;
        TIntV* AssignIdxVPtr = &AssignIdxV;
        TIntV* OldAssignIdxVPtr = &OldAssignIdxV;
        TIntV* Temp;

        // select initial centroids
        if (InitCentVV.Empty()) {
            TAbsKMeans<TCentroidType>::SelectInitCentroids(FtrVV, K, NInst);
        } else {
            TAbsKMeans<TCentroidType>::SelectInitCentroids(InitCentVV);
        }

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
        TCentroidType TempDxK2;				// (dimension d x k)
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
            TAbsKMeans<TCentroidType>::UpdateCentroids(FtrVV,
                    NInst,
                    *AssignIdxVPtr,
                    OnesN,
                    RangeN,
                    TempK,
                    TempDxK,
                    TempDxK2,
                    TempKxKSpVV,
                    NormX2,
                    NormC2,
                    AllowEmptyP
            );

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
}

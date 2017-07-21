/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef LINALG_HPP
#define LINALG_HPP


//////////////////////////////////////////////////////////////////////
// Template-ised Sparse Operations
template <class TKey, class TDat>
void TSparseOps<TKey, TDat>::CoordinateCreateSparseColMatrix(const TVec<TKey>& RowIdxV, const TVec<TKey>& ColIdxV,
        const TVec<TDat>& ValV, TVec<TVec<TKeyDat<TKey, TDat> > >& ColMatrix, const TKey& Cols) {

    ColMatrix.Gen(Cols);
    EAssert(RowIdxV.Len() == ColIdxV.Len() && RowIdxV.Len() == ValV.Len());
    TKey Els = RowIdxV.Len();
    for (TKey ElN = 0; ElN < Els; ElN++) {
        ColMatrix[ColIdxV[ElN]].Add(TKeyDat<TKey, TDat>(RowIdxV[ElN], ValV[ElN]));
    }
    for (TKey ColN = 0; ColN < Cols; ColN++) {
        ColMatrix[ColN].Sort();
    }
}

template <class TKey, class TDat>
void TSparseOps<TKey, TDat>::SparseMerge(const TVec<TKeyDat<TKey, TDat> >& SrcV1,
        const TVec<TKeyDat<TKey, TDat> >& SrcV2, TVec<TKeyDat<TKey, TDat> >& DstV) {

    DstV.Clr();
    const int Src1Len = SrcV1.Len();
    const int Src2Len = SrcV2.Len();
    int Src1N = 0, Src2N = 0;
    while (Src1N < Src1Len && Src2N < Src2Len) {
        if (SrcV1[Src1N].Key < SrcV2[Src2N].Key) {
            DstV.Add(SrcV1[Src1N]); Src1N++;
        }
        else if (SrcV1[Src1N].Key > SrcV2[Src2N].Key) {
            DstV.Add(SrcV2[Src2N]); Src2N++;
        }
        else {
            DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, SrcV1[Src1N].Dat + SrcV2[Src2N].Dat));
            Src1N++;  Src2N++;
        }
    }
    while (Src1N < Src1Len) { DstV.Add(SrcV1[Src1N]); Src1N++; }
    while (Src2N < Src2Len) { DstV.Add(SrcV2[Src2N]); Src2N++; }
}

template <class TKey, class TDat>
void TSparseOps<TKey, TDat>::SparseLinComb(const double& p, const TVec<TKeyDat<TKey, TDat> >& SrcV1,
        const double& q, const TVec<TKeyDat<TKey, TDat> >& SrcV2, TVec<TKeyDat<TKey, TDat> >& DstV) {
    AssertR(&SrcV1 != &DstV && &SrcV2 != &DstV, "Should not use source as destination in sparse lin comb!");
    DstV.Clr();
    const int Src1Len = SrcV1.Len();
    const int Src2Len = SrcV2.Len();
    int Src1N = 0, Src2N = 0;
    while (Src1N < Src1Len && Src2N < Src2Len) {
        if (SrcV1[Src1N].Key < SrcV2[Src2N].Key) {
            DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, p * SrcV1[Src1N].Dat)); Src1N++;
        }
        else if (SrcV1[Src1N].Key > SrcV2[Src2N].Key) {
            DstV.Add(TKeyDat<TKey, TDat>(SrcV2[Src2N].Key, q * SrcV2[Src2N].Dat)); Src2N++;
        }
        else {
            DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, p * SrcV1[Src1N].Dat + q * SrcV2[Src2N].Dat));
            Src1N++;  Src2N++;
        }
    }
    while (Src1N < Src1Len) { DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, p * SrcV1[Src1N].Dat)); Src1N++; }
    while (Src2N < Src2Len) { DstV.Add(TKeyDat<TKey, TDat>(SrcV2[Src2N].Key, q * SrcV2[Src2N].Dat)); Src2N++; }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgStat::Mean(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& ColVV,
        TVec<TNum<TType>, TSizeTy>& MeanV, const TMatDim& Dim) {

    if (Dim == TMatDim::mdCols) {
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy> RowVV;
        TLinAlg::Transpose(ColVV, RowVV);
        Mean(RowVV, MeanV, TMatDim::mdRows);
    }
    else {
        // calculate the average column
        const TSizeTy NCols = ColVV.Len();

        // first find the max dimension
        TSizeTy MxDim = 0;
        for (TSizeTy ColN = 0; ColN < NCols; ++ColN) {
            const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& ColV = ColVV[ColN];
            if (ColV.Last().Key > MxDim) { MxDim = ColV.Last().Key; }
        }

        MeanV.Gen(MxDim, MxDim);

        for (TSizeTy ColN = 0; ColN < NCols; ++ColN) {
            const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& ColV = ColVV[ColN];
            for (TSizeTy ValN = 0; ValN < ColV.Len(); ++ValN) {
                const TKeyDat<TNum<TSizeTy>, TNum<TType>>& SpEntry = ColV[ValN];
                MeanV[SpEntry.Key] += SpEntry.Dat / double(NCols);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
/// Transformations of linear algebra structures
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::OnesV(const TSizeTy& Dim, TVec<TNum<TType>, TSizeTy>& OnesV) {
    if (OnesV.Len() != Dim) { OnesV.Gen(Dim); }

    for (TSizeTy i = 0; i < Dim; i++) {
        OnesV[i] = 1;
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::RangeV(const TSizeTy& Dim, TVec<TNum<TType>, TSizeTy>& RangeV) {
    const TSizeTy Mn = 0;
    const TSizeTy Mx = Dim-1;

    TLinAlgTransform::RangeV(Mn, Mx, RangeV);
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::RangeV(const TSizeTy& Min, const TSizeTy& Max,
        TVec<TNum<TType>, TSizeTy>& Res) {
    const TSizeTy Len = Max - Min + 1;

    if (Res.Len() != Len) { Res.Gen(Len); }

    for (TSizeTy i = Min; i <= Max; i++) {
        Res[i - Min] = TType(i);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Diag(const TVec<TNum<TType>, TSizeTy>& DiagV,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& D) {
    const TSizeTy Dim = DiagV.Len();

    if (D.Empty()) { D.Gen(Dim, Dim); }
    EAssert(D.GetRows() == Dim && D.GetCols() == Dim);

    for (TSizeTy i = 0; i < Dim; i++) {
        D(i,i) = DiagV[i];
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Diag(const TVec<TNum<TType>, TSizeTy>& Vec,
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& Mat) {
    TSizeTy Len = Vec.Len();
    Mat.Gen(Len);
    for (TSizeTy ColN = 0; ColN < Len; ColN++) {
        Mat[ColN].Add(TKeyDat<TNum<TSizeTy>, TNum<TType>>(ColN, Vec[ColN]));
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Identity(const TSizeTy& Dim, TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    if (X.Empty()) { X.Gen(Dim, Dim); }
    EAssert(X.GetRows() == Dim && X.GetCols() == Dim);

    for (TSizeTy i = 0; i < Dim; i++) {
        X(i,i) = 1;
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Sqrt(TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
        for (TSizeTy ColN = 0; ColN < X.GetCols(); ColN++) {
            TType Val = X(RowN, ColN);
            AssertR(Val > -1e-8, "Distance lower than numerical error!");
            if (Val < 0) { Val = 0; }
            X.PutXY(RowN, ColN, TMath::Sqrt(Val));
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Sqrt(TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X) {
    for (TSizeTy ColN = 0; ColN < X.Len(); ColN++) {
        TVec<TKeyDat<TNum<TSizeTy>, TType>, TSizeTy>& ColSpV = X[ColN];
        for (TSizeTy RowN = 0; RowN < ColSpV.Len(); RowN++) {
            const TType& Val = ColSpV[RowN].Dat;
            AssertR(Val > -1e-8, "Distance lower than numerical error!");
            if (Val < 0) { Val = 0; }
            ColSpV[RowN].Dat = TMath::Sqrt(Val);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlgTransform::Sqr(TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
        for (TSizeTy ColN = 0; ColN < X.GetCols(); ColN++) {
            X.PutXY(RowN, ColN, TMath::Sqr(X(RowN, ColN)));
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlgTransform::Sqr(TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X) {
    for (TSizeTy ColN = 0; ColN < X.Len(); ColN++) {
        TVec<TKeyDat<TNum<TSizeTy>, TType>, TSizeTy>& ColSpV = X[ColN];
        for (TSizeTy RowN = 0; RowN < ColSpV.Len(); RowN++) {
            ColSpV[RowN].Dat = TMath::Sqr(ColSpV[RowN].Dat);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::CenterRows(TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    const TSizeTy Rows = X.GetRows();
    const TSizeTy Cols = X.GetCols();

    #pragma omp parallel for
    for (TSizeTy RowIdx = 0; RowIdx < Rows; RowIdx++) {
        TType RowMean = 0;
        for (TSizeTy ColIdx = 0; ColIdx < Cols; ColIdx++) {
            RowMean += X(RowIdx, ColIdx);
        }
        RowMean /= Cols;
        for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
            X(RowIdx, ColIdx) -= RowMean;
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlgTransform::Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B){
    B.Val1.Gen(0);
    B.Val2.Gen(0);
    B.Val3.Gen(0);
    for (TSizeTy RowN = 0; RowN < A.GetRows(); RowN++) {
        for (TSizeTy ColN = 0; ColN < A.GetCols(); ColN++) {
            if (A.At(RowN, ColN) != 0.0) {
                B.Val1.Add(RowN);
                B.Val2.Add(ColN);
                B.Val3.Add(A.At(RowN, ColN));
            }
        }
    }
}


//TODO fix TVec<TIntFltKdV> indexing and type
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Sparse(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& B){
    TSizeTy Cols = A.GetCols();
    TSizeTy Rows = A.GetRows();
    B.Gen(Cols);
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        B[ColN].Gen(0);
        for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
            if (A.At(RowN, ColN) != 0.0) {
                B[ColN].Add(TKeyDat<TNum<TSizeTy>, TNum<TType>>(RowN, A.At(RowN, ColN)));
            }
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlgTransform::Full(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A, TVVec<TType, TSizeTy, ColMajor>& B, const int Rows, const int Cols) {
    B.Gen(Rows, Cols);
    B.PutAll(0.0);
    TSizeTy nnz = A.Val1.Len();
    for (TSizeTy ElN = 0; ElN < nnz; ElN++) {
        B.At(A.Val1[ElN], A.Val2[ElN]) = A.Val3[ElN];
    }
}

// Sparse to dense transform
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Full(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TSizeTy Rows){
    TSizeTy Cols = A.Len();
    B.Gen(Rows, Cols);
    B.PutAll(0.0);
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        TSizeTy Els = A[ColN].Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            B.At(A[ColN][ElN].Key, ColN) = A[ColN][ElN].Dat;
        }
    }
}

//////////////////////////////////////////////////////////////////////
/// TLinAlgCheck
template <class TType, class TSizeTy, bool ColMajor>
bool TLinAlgCheck::IsZero(const TVec<TNum<TType>, TSizeTy>& Vec) {
    const TSizeTy& Len = Vec.Len();
    for (TSizeTy i = 0; i < Len; i++) {
        if (Vec[i] != 0.0) { return false; }
    }
    return true;
}

template <class TType, class TSizeTy, bool ColMajor>
bool TLinAlgCheck::ContainsNan(const TVVec<TNum<TType>, TSizeTy, ColMajor>& FltVV) {
    const TSizeTy& Rows = FltVV.GetRows();
    const TSizeTy& Cols = FltVV.GetCols();

    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            if (TNum<TType>::IsNan(FltVV(RowN, ColN))) {
                return true;
            }
        }
    }

    return false;
}

template <class TType, class TSizeTy, bool ColMajor>
bool TLinAlgCheck::ContainsNan(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& SpVV) {
    const TSizeTy& Cols = SpVV.Len();

    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& Col = SpVV[ColN];
        const TSizeTy& Rows = Col.Len();

        for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
            if (TNum<TType>::IsNan(Col[RowN].Dat)) {
                return true;
            }
        }
    }

    return false;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgCheck::AssertOrtogonality(const TVec<TVec<TNum<TType>, TSizeTy>, TSizeTy>& Vecs,
        const TType& Threshold) {
    TSizeTy m = Vecs.Len();
    for (TSizeTy i = 0; i < m; i++) {
        for (TSizeTy j = 0; j < i; j++) {
            double res = TLinAlg::DotProduct(Vecs[i], Vecs[j]);
            if (TFlt::Abs(res) > Threshold)
                printf("<%d,%d> = %.5f", i, j, res);
        }
        double norm = TLinAlg::Norm2(Vecs[i]);
        if (TFlt::Abs(norm - 1) > Threshold)
            printf("||%d|| = %.5f", i, norm);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgCheck::AssertOrtogonality(const TVVec<TNum<TType>, TSizeTy, ColMajor>& Vecs,
        const TType& Threshold) {
    TSizeTy m = Vecs.GetCols();
    for (TSizeTy i = 0; i < m; i++) {
        for (TSizeTy j = 0; j < i; j++) {
            double res = TLinAlg::DotProduct(Vecs, i, Vecs, j);
            if (TFlt::Abs(res) > Threshold)
                printf("<%d,%d> = %.5f", i, j, res);
        }
        double norm = TLinAlg::Norm2(Vecs, i);
        if (TFlt::Abs(norm - 1) > Threshold)
            printf("||%d|| = %.5f", i, norm);
    }
    printf("\n");
}

//////////////////////////////////////////////////////////////////////
/// Search elements of matrices and vectors
template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetMaxIdx(const TVec<TNum<TType>, TSizeTy>& Vec) {
    if (Vec.Empty()) { return -1; }

    TSizeTy MxIdx = 0;
    TType MxVal = Vec[0];
    for (TSizeTy i = 1; i < Vec.Len(); i++ ) {
        if (Vec[i] > MxVal) {
            MxVal = Vec[i];
            MxIdx = i;
        }
    }

    return MxIdx;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetMinIdx(const TVec<TNum<TType>, TSizeTy>& Vec) {
    const TSizeTy Len = Vec.Len();

    TSizeTy MinIdx = 0;
    TType MinVal = Vec[MinIdx];
    for (TSizeTy i = 1; i < Len; i++) {
        if (Vec[i] < MinVal) {
            MinVal = Vec[i];
            MinIdx = i;
        }
    }

    return MinIdx;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlgSearch::GetMaxVal(const TVec<TNum<TType>, TSizeTy>& Vec) {
    const TSizeTy Len = Vec.Len();
    TType MaxVal = Vec[0];
    for (int i = 1; i < Len; i++) {
        if (Vec[i] > MaxVal) {
            MaxVal = Vec[i];
        }
    }
    return MaxVal;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetMaxDimIdx(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& SpVec) {
    return SpVec.Len() > 0 ? SpVec.Last().Key.Val : 0;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetMaxDimIdx(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& SpMat) {
    TSizeTy MaxDim = 0;
    for (TSizeTy ColN = 0; ColN < SpMat.Len(); ColN++) {
        if (!SpMat[ColN].Empty()) {
            MaxDim = MAX(MaxDim, SpMat[ColN].Last().Key.Val);
        }
    }
    return MaxDim;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetRowMaxIdx(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TSizeTy& RowN) {
    TSizeTy Idx = -1;
    TSizeTy Cols = X.GetCols();
    double MaxVal = TFlt::Mn;
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const double Val = X.At(RowN, ColN);
        if (MaxVal < Val) {
            MaxVal = Val;
            Idx = ColN;
        }
    }
    return Idx;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetRowMinIdx(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TSizeTy& RowN) {
    TSizeTy Idx = -1;
    TSizeTy Cols = X.GetCols();
    TType MnVal = TNum<TType>::Mx;
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const TType& Val = X.At(RowN, ColN);

        AssertR(!TNum<TType>::IsNan(Val), "TLinAlgSearch::GetRowMinIdx: NaN in matrix!");

        if (Val < MnVal) {
            MnVal = Val;
            Idx = ColN;
        }
    }
    return Idx;
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetColMaxIdx(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TSizeTy& ColN) {
    TSizeTy Idx = -1;
    TSizeTy Rows = X.GetRows();
    TType MaxVal = TNum<TType>::Mn;
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        const TType& Val = X.At(RowN, ColN);
        if (MaxVal < Val) {
            MaxVal = Val;
            Idx = RowN;
        }
    }
    return Idx;
}

template <class TType, class TSizeTy, bool ColMajor>
TSizeTy TLinAlgSearch::GetColMinIdx(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TSizeTy& ColN) {
    const TSizeTy Rows = X.GetRows();
    TType MinVal = TFlt::Mx;
    TSizeTy MinIdx = -1;
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        const TType& Val = X(RowN, ColN);

        AssertR(!TNum<TType>::IsNan(Val), "TLinAlgSearch::GetColMinIdx: NaN in column!");

        if (Val < MinVal) {
            MinVal = Val;
            MinIdx = RowN;
        }
    }
    EAssertR(MinIdx >= 0, "Minimum index not set!");
    return MinIdx;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlgSearch::GetColMin(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TSizeTy& ColN) {
    const TSizeTy Rows = X.GetRows();
    EAssertR(Rows > 0, "Input matrix should have at least one row!");

    TType MinVal = X(0, ColN);
    for (int RowN = 1; RowN < Rows; RowN++) {
        TType Val = X(RowN, ColN);
        if (Val < MinVal) {
            MinVal = Val;
        }
    }

    return MinVal;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgSearch::GetColMinV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TType>, TSizeTy>& ValV) {
    const TSizeTy Cols = X.GetCols();

    ValV.Gen(Cols);
    for (int ColN = 0; ColN < Cols; ColN++) {
        ValV[ColN] = GetColMin(X, ColN);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgSearch::GetRowMaxIdxV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TSizeTy>, TSizeTy>& IdxV) {
    IdxV.Gen(X.GetRows());
    TSizeTy Rows = X.GetRows();
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        IdxV[RowN] = TLinAlgSearch::GetRowMaxIdx(X, RowN);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgSearch::GetRowMinIdxV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TSizeTy>, TSizeTy>& IdxV) {
    const TSizeTy Rows = X.GetRows();

    if (IdxV.Len() != Rows) { IdxV.Gen(Rows); }

    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        IdxV[RowN] = TLinAlgSearch::GetRowMinIdx(X, RowN);
    }
}

// find the index of maximum elements for each col of X
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgSearch::GetColMaxIdxV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TSizeTy>, TSizeTy>& IdxV) {
    IdxV.Gen(X.GetCols());
    TSizeTy Cols = X.GetCols();
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        IdxV[ColN] = TLinAlgSearch::GetColMaxIdx(X, ColN);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgSearch::GetColMinIdxV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TSizeTy>, TSizeTy>& IdxV) {
    const TSizeTy& Cols = X.GetCols();

    if (IdxV.Empty()) { IdxV.Gen(Cols); }
    EAssert(IdxV.Len() == Cols);

    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        IdxV[ColN] = GetColMinIdx(X, ColN);
    }
}


///////////////////////////////////////////////////////////////////////
// Basic Linear Algebra operations
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y) {
    EAssertR(x.Len() == y.Len(), TStr::Fmt("%d != %d", x.Len(), y.Len()));
    TType result = 0.0; const  TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        result += x[i] * y[i];
    return result;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TVec<TNum<TType>, TSizeTy>>& X, TSizeTy ColId,
        const TVec<TNum<TType>, TSizeTy>& y) {
    EAssert(0 <= ColId && ColId < X.Len());
    return DotProduct(X[ColId], y);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X,
        TSizeTy ColId, const TVec<TNum<TType>, TSizeTy>& y) {
    EAssert(0 <= ColId && ColId < X.Len());
    return DotProduct(y, X[ColId]);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TSizeTy ColIdX, const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y, TSizeTy ColIdY) {
    EAssert(X.GetRows() == Y.GetRows());
    TType result = 0; const TSizeTy len = X.GetRows();
    for (TSizeTy i = 0; i < len; i++)
        result = result + X(i, ColIdX) * Y(i, ColIdY);
    return result;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
    TSizeTy ColId, const TVec<TNum<TType>, TSizeTy>& Vec) {

    EAssert(X.GetRows() == Vec.Len());
    TType result = 0.0; const TSizeTy len = X.GetRows();
    for (TSizeTy i = 0; i < len; i++)
        result += X(i, ColId) * Vec[i];
    return result;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y) {
    const TSizeTy xLen = x.Len(), yLen = y.Len();
    TType Res = 0.0;
    TSizeTy i1 = 0, i2 = 0;
    while (i1 < xLen && i2 < yLen) {
        if (x[i1].Key < y[i2].Key) i1++;
        else if (x[i1].Key > y[i2].Key) i2++;
        else { Res += x[i1].Dat * y[i2].Dat;  i1++;  i2++; }
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y) {
    TType Res = 0.0; const TSizeTy xLen = x.Len(), yLen = y.Len();
    for (TSizeTy i = 0; i < yLen; i++) {
        const TSizeTy key = y[i].Key;
        if (key < xLen) Res += y[i].Dat * x[key];
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X, TSizeTy ColId,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y) {
    TType Res = 0.0; const TSizeTy n = X.GetRows(), yLen = y.Len();
    for (TSizeTy i = 0; i < yLen; i++) {
        const TSizeTy key = y[i].Key;
        if (key < n) Res += y[i].Dat * X(key, ColId);
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y) {
    EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());
    const TSizeTy Rows = X.GetRows();
    const TSizeTy Cols = X.GetCols();
    TType Res = 0.0;
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            Res += X(RowN, ColN) * Y(RowN, ColN);
        }
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::DotProduct(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y) {
    const TSizeTy Rows = TLinAlgSearch::GetMaxDimIdx(X) + 1;
    EAssert(Rows <= Y.GetRows() && X.Len() == Y.GetCols());
    const TSizeTy Cols = X.Len();
    TType Res = 0.0;
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& ColX = X[ColN];
        const TSizeTy Els = ColX.Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            Res += ColX[ElN].Dat * Y(ColX[ElN].Key, ColN);
        }
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::OuterProduct(const TVec<TNum<TType>, TSizeTy>& x,
    const TVec<TNum<TType>, TSizeTy>& y, TVVec<TNum<TType>, TSizeTy, ColMajor>& Z) {

    EAssert(Z.GetRows() == x.Len() && Z.GetCols() == y.Len());
    const TSizeTy XLen = x.Len();
    const TSizeTy YLen = y.Len();
    for (TSizeTy i = 0; i < XLen; i++) {
        for (TSizeTy j = 0; j < YLen; j++) {
            Z(i, j) = x[i] * y[j];
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::HadamardProd(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Z) {

    EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());

    if (Z.Empty()) {
        Z.Gen(X.GetRows(), X.GetCols());
    } else {
        EAssert(X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
    }
    int Rows = X.GetRows();
    int Cols = X.GetCols();
    for (int RowN = 0; RowN < Rows; RowN++) {
        for (int ColN = 0; ColN < Cols; ColN++) {
            Z(RowN, ColN) = X(RowN, ColN) * Y(RowN, ColN);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::HadamardProd(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Z) {

    TSizeTy Rows = TLinAlgSearch::GetMaxDimIdx(X) + 1;
    EAssert(Rows <= Y.GetRows() && X.Len() == Y.GetCols());
    if (Z.Empty()) {
        Z.Gen(Y.GetRows(), Y.GetCols());
    } else {
        EAssert(Y.GetRows() == Z.GetRows() && Y.GetCols() == Z.GetCols());
    }
    TSizeTy Cols = X.Len();
    TLinAlgTransform::FillZero(Z);
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& ColX = X[ColN];
        const TSizeTy Els = ColX.Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            Z(ColX[ElN].Key, ColN) = ColX[ElN].Dat * Y(ColX[ElN].Key, ColN);
        }
    }
}

//TODO this will work only for glib type TFlt
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinCombInPlace(const TType& alpha, const TVec<TNum<TType>, TSizeTy>& x,
    const TType& beta, TVec<TNum<TType>, TSizeTy>& y) {

#ifdef BLAS
    if (TypeCheck::is_double<TType>::value == true){
        typedef double Loc;
        cblas_daxpby(x.Len(), *((Loc *)&alpha), (Loc *)&x[0].Val, 1, *((Loc *)&beta), (Loc *)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_float<TType>::value == true){
        typedef float Loc;
        cblas_saxpby(x.Len(), *((Loc *)&alpha), (Loc *)&x[0].Val, 1, *((Loc *)&beta), (Loc *)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_complex_double<TType>::value == true){
        typedef double Loc;
        //std::complex<double> alpha_(alpha); std::complex<double> beta_(beta);
        cblas_zaxpby(x.Len(), (const Loc*)&alpha, (const Loc*)&x[0].Val, 1, (const Loc*)&beta, (Loc*)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_complex_float<TType>::value == true){
        typedef float Loc;
        //std::complex<float> alpha_((float)alpha); std::complex<float> beta_((float)beta);
        cblas_caxpby(x.Len(), (const Loc*)&alpha, (const Loc*)&x[0].Val, 1, (const Loc*)&beta, (Loc*)&y[0].Val, 1);
    }
#else
    LinComb(alpha, x, beta, y, y);
#endif
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinComb(const double& p, const TVec<TNum<TType>, TSizeTy>& x,
    const double& q, const TVec<TNum<TType>, TSizeTy>& y, TVec<TNum<TType>, TSizeTy>& z) {
    if (z.Len() == 0) {
        EAssert(x.Len() == y.Len());
        z.Gen(x.Len());
    } else {
        EAssert(x.Len() == y.Len() && y.Len() == z.Len());
    }
    const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++) {
        z[i] = p * x[i] + q * y[i];
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinComb(const double& p, const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const double& q, const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Z) {
    EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());
    if (Z.Empty()) {
        Z.Gen(X.GetRows(), X.GetCols());
    } else {
        EAssert(X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
    }
    TSizeTy Rows = X.GetRows();
    TSizeTy Cols = X.GetCols();
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            Z.At(RowN, ColN) = p*X.At(RowN, ColN) + q*Y.At(RowN, ColN);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinComb(const double& p, const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TSizeTy ColId, const double& q, const TVec<TNum<TType>, TSizeTy>& y,
        TVec<TNum<TType>, TSizeTy>& z) {

    if (z.Empty()) z.Gen(X.GetRows());
    EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());

    const TSizeTy len = z.Len();
    for (TSizeTy i = 0; i < len; i++) {
        z[i] = p * X(i, ColId) + q * y[i];
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinComb(const double& p, const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TSizeTy DimId, const double& q, const TVec<TNum<TType>, TSizeTy>& y,
        TVec<TNum<TType>, TSizeTy>& z,
        int Dim) {

    EAssertR(Dim == 1 || Dim == 2, "TLinAlg::LinComb: Invalid value of argument Dim.");
    if (Dim == 1) {
        if (z.Empty()) z.Gen(X.GetRows());
        EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());

        const TSizeTy len = z.Len();
        for (TSizeTy i = 0; i < len; i++) {
            z[i] = p * X(i, DimId) + q * y[i];
        }
    }
    else if (Dim == 2) {
        if (z.Empty()) z.Gen(X.GetCols());
        EAssert(X.GetCols() == y.Len() && y.Len() == z.Len());

        const TSizeTy len = z.Len();
        for (TSizeTy i = 0; i < len; i++) {
            z[i] = p * X(DimId, i) + q * y[i];
        }
    }
}

// TEST
// z := p * x + (1 - p) * y
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::ConvexComb(const double& p, const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y, TVec<TNum<TType>, TSizeTy>& z)  {
    AssertR(0.0 <= p && p <= 1.0, TFlt::GetStr(p));
    TLinAlg::LinComb(p, x, 1.0 - p, y, z);
}

#ifdef BLAS

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TType& k, const TVec<TNum<TType>, TSizeTy>& x,
        TVec<TNum<TType>, TSizeTy>& y) {
    if (TypeCheck::is_double<TType>::value == true){
        typedef double Loc;
        cblas_daxpy(x.Len(), *((Loc *)&k), (Loc *)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_float<TType>::value == true){
        typedef float Loc;
        cblas_saxpy(x.Len(), *((Loc *)&k), (Loc *)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_complex_double<TType>::value == true){
        typedef double Loc;
        cblas_zaxpy(x.Len(), (const Loc *)&k, (const Loc*)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
    }
    else
    if (TypeCheck::is_complex_float<TType>::value == true){
        typedef float Loc;
        cblas_caxpy(x.Len(), (const Loc *)&k, (const Loc *)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
    }
    //cblas_daxpy(x.Len(), k, &x[0].Val, 1, &y[0].Val, 1);
}

#else

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TType& k, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y) {
    const int Size = x.Len();

    EAssert(y.Len() == Size);

    for (int ValN = 0; ValN < Size; ValN++) {
        y[ValN] = k*x[ValN] + y[ValN];
    }
}

#endif

// z := k * x + y
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const double& k, const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y, TVec<TNum<TType>, TSizeTy>& z)  {
    TLinAlg::LinComb(k, x, 1.0, y, z);
}

// z := x + y
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y, TVec<TNum<TType>, TSizeTy>& z) {
    TLinAlg::LinComb(1.0, x, 1.0, y, z);
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TType& k,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y, TVec<TNum<TType>, TSizeTy>& z) {
    EAssert(y.Len() == z.Len());
    z = y; // first we set z to be y
    // and than we add x to z (==y)
    const TSizeTy xLen = x.Len(), yLen = y.Len();
    for (TSizeTy i = 0; i < xLen; i++) {
        const TSizeTy ii = x[i].Key;
        if (ii < yLen) {
            z[ii] = k * x[i].Dat + y[ii];
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(double k, const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TSizeTy ColIdX, TVVec<TNum<TType>, TSizeTy, ColMajor>& Y, TSizeTy ColIdY) {
    EAssert(X.GetRows() == Y.GetRows());
    const TSizeTy len = Y.GetRows();
    for (TSizeTy i = 0; i < len; i++) {
        Y(i, ColIdY) = Y(i, ColIdY) + k * X(i, ColIdX);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const double& k, const TVec<TNum<TType>, TSizeTy>& x,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Y, const TSizeTy& ColIdY) {
    EAssert(x.Len() == Y.GetRows());
    EAssert(ColIdY >= 0 && ColIdY < x.Len());
    for (TSizeTy RowN = 0; RowN < Y.GetRows(); RowN++) {
        Y.At(RowN, ColIdY) += k*x[RowN];
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(double k, const TVVec<TNum<TType>, TSizeTy, ColMajor>& X, TSizeTy ColId,
        TVec<TNum<TType>, TSizeTy>& Result) {
    EAssert(X.GetRows() == Result.Len());
    const TSizeTy len = Result.Len();
    for (TSizeTy i = 0; i < len; i++) {
        Result[i] = Result[i] + k * X(i, ColId);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TType& k,
       const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x,
       const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y,
       TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& z) {
    LinComb(k, x, 1, y, z);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::SumVec(const TVec<TNum<TType>, TSizeTy>& x) {
    const TSizeTy len = x.Len();
    double Res = 0.0;
    for (int i = 0; i < len; i++) {
        Res += x[i];
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::SumVec(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    const TSizeTy len = x.Len();
    TType Res = 0.0;
    for (TSizeTy i = 0; i < len; i++) {
        Res += x[i].Dat;
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::SumVec(double k, const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y) {
    EAssert(x.Len() == y.Len());
    const TSizeTy len = x.Len();
    double Res = 0.0;
    for (TSizeTy i = 0; i < len; i++) {
        Res += k * x[i] + y[i];
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::EuclDist2(const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y) {
    EAssert(x.Len() == y.Len());
    const TSizeTy len = x.Len();
    TType Res = 0.0;
    for (TSizeTy i = 0; i < len; i++) {
        Res += TMath::Sqr(x[i] - y[i]);
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::EuclDist2(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y) {
    TType Res = TLinAlg::Norm2(x) - 2 * TLinAlg::DotProduct(x, y) + TLinAlg::Norm2(y);
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::EuclDist(const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y) {
    return TMath::Sqrt(TLinAlg::EuclDist2(x, y));
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob(const TVVec<TNum<TType>, TSizeTy, ColMajor> &A) {
    TType frob = 0;
    for (TSizeTy RowN = 0; RowN < A.GetRows(); RowN++) {
        for (TSizeTy ColN = 0; ColN < A.GetCols(); ColN++) {
            frob += A.At(RowN, ColN)*A.At(RowN, ColN);
        }
    }
    return sqrt(frob);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob2(const TVVec<TNum<TType>, TSizeTy, ColMajor> &A) {
    TType frob = 0;
    for (TSizeTy RowN = 0; RowN < A.GetRows(); RowN++) {
        for (TSizeTy ColN = 0; ColN < A.GetCols(); ColN++) {
            frob += A.At(RowN, ColN)*A.At(RowN, ColN);
        }
    }
    return frob;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy> &A) {
    return TMath::Sqrt(Frob2(A));
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob2(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy> &A) {
    const TSizeTy& Cols = A.Len();
    TType Res = 0.0;
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& ColX = A[ColN];
        const TSizeTy& Els = ColX.Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            Res += ColX[ElN].Dat * ColX[ElN].Dat;
        }
    }
    return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::FrobDist2(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B) {
    TType frob = 0;
    TVec<TType, TSizeTy> Apom = (const_cast<TVVec<TType, TSizeTy, ColMajor> &>(A)).Get1DVec();
    TVec<TType, TSizeTy> Bpom = (const_cast<TVVec<TType, TSizeTy, ColMajor> &>(B)).Get1DVec();
    frob = TLinAlg::EuclDist2(Apom, Bpom);
    /*for (int RowN = 0; RowN < A.GetRows(); RowN++) {
    for (int ColN = 0; ColN < A.GetCols(); ColN++) {
        frob += (A.At(RowN, ColN) - B.At(RowN, ColN))*(A.At(RowN, ColN) - B.At(RowN, ColN));
    }
    }*/
    return frob;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::FrobDist2(const TVec<TNum<TType>, TSizeTy>& A,
        const TVec<TNum<TType>, TSizeTy>& B) {
    TType frob = 0;
    frob = TLinAlg::EuclDist2(A, B);
    /*for (int RowN = 0; RowN < A.Len(); RowN++) {
        frob += (A[RowN] - B[RowN])*(A[RowN] - B[RowN]);
    }*/
    return frob;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm2(const TVec<TNum<TType>, TSizeTy>& x) {
    return TLinAlg::DotProduct(x, x);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm(const TVec<TNum<TType>, TSizeTy>& x) {
    return TMath::Sqrt(TLinAlg::Norm2(x));
}

// ||x||^2 (Euclidian), x is sparse
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm2(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    TType Result = 0;
    for (TSizeTy i = 0; i < x.Len(); i++) {
        Result += TMath::Sqr(x[i].Dat);
    }
    return Result;
}

// ||x|| (Euclidian), x is sparse
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    return TMath::Sqrt(Norm2(x));
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& x,
        const TSizeTy& ColId) {
    return Norm(x[ColId]);
}

// ||X(:,ColId)||^2 (Euclidian)
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm2(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X, const TSizeTy& ColId) {
    return TLinAlg::DotProduct(X, ColId, X, ColId);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Norm(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X, TSizeTy ColId) {
    return TMath::Sqrt(TLinAlg::Norm2(X, ColId));
}

// L1 norm of x (Sum[|xi|, i = 1..n])
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::NormL1(const TVec<TNum<TType>, TSizeTy>& x)  {
    TType norm = 0.0; const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        norm += TFlt::Abs(x[i]);
    return norm;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::NormL1(double k, const TVec<TNum<TType>, TSizeTy>& x,
        const TVec<TNum<TType>, TSizeTy>& y) {
    EAssert(x.Len() == y.Len());
    TType norm = 0.0; const TSizeTy len = x.Len();
    for (TSizeTy i = 0; i < len; i++) {
        norm += TFlt::Abs(k * x[i] + y[i]);
    }
    return norm;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::NormL1(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    TType norm = 0.0; const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        norm += TFlt::Abs(x[i].Dat);
    return norm;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Normalize(TVec<TNum<TType>, TSizeTy>& x) {
    const TType xNorm = TLinAlg::Norm(x);
    if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
    return xNorm;
}

// x := x / ||x||, x is sparse
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Normalize(TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    const TType Normx = TLinAlg::Norm(x);
    if (Normx > 0) {
        TLinAlg::MultiplyScalar(1 / Normx, x, x);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumn(TVVec<TNum<TType>, TSizeTy, ColMajor>& X, const TSizeTy& ColId) {
    double nX = TLinAlg::Norm(X, ColId);
    if (nX > 0.0) {
        for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
            X.At(RowN, ColId) /= nX;
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    for (TSizeTy ColN = 0; ColN < X.GetCols(); ColN++) {
        TLinAlg::NormalizeColumn(X, ColN);
    }
}
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeRows(TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
        TVec<TType> Row;
        X.GetRowPtr(RowN, Row);
        Normalize(Row);
    }
}

#ifdef INTEL
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVVec<TNum<TType>, TSizeTy, ColMajor>& X, TBool ColumnMajor) {
    const TSizeTy m = X.GetXDim();
    const TSizeTy n = X.GetYDim();
    TVVec<TNum<TType>, TSizeTy, ColMajor> sqrX(m, n);
    vdSqr(m*n, &X(0, 0).Val, &sqrX(0, 0).Val);
    printf("Squaring of elements done!\n");
    TVec<TNum<TType>, TSizeTy> enke(m); TVec<TType, TSizeTy> sumsqr(n);  TVec<TType, TSizeTy> norme(n); TLAMisc::Fill(enke, 1.0);
    TLinAlg::MultiplyT(sqrX, enke, sumsqr);
    printf("Summing elemnents done!\n");
    vdInvSqrt(n, &sumsqr[0].Val, &norme[0].Val);
    printf("Summing and inverting elemnents done!\n");
    // added code
    if (ColMajor) {
        TVVec<TNum<TType>, TSizeTy, ColMajor> B; B.Gen(n, m);
        TLinAlg::Transpose(X, B);

        for (TSizeTy i = 0; i < m; i++) {
            vdMul(n, &norme[0].Val, &B(0, i).Val, &B(0, i).Val);
        }
        TLinAlg::Transpose(B, X);
    }
    else {
        for (TSizeTy i = 0; i < m; i++){
            vdMul(n, &norme[0].Val, &X(i, 0).Val, &X(i, 0).Val);
        }
    }
    //TLAMisc::PrintTFltVV(X, "Normalizirana");
}
#else
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TBool ColumnMajor) {
    throw TExcept::New("TLinAlg::NormalizeColumns(TVVec<TType, TSizeTy, ColMajor>& X, TBool ColumnMajor) not implemented yet!");
}
#endif

// Normalize the columns of X
//TODO what to do when number
//MARK
template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlg::NormalizeColumns(TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>,
        TVec<TNum<TType>, TSizeTy>>& X) {
    if (X.Val2.Len() == 0) return;
    EAssert(X.Val2.IsSorted(true));
    //int?
    int Cols = X.Val2.GetMxVal() + 1;
    TVec<TNum<TType>, TSizeTy> InvColNorms(Cols); //get the last element colN and set the number of elements

    TSizeTy Els = X.Val1.Len();
    for (TSizeTy ElN = 0; ElN < Els; ElN++) {
        InvColNorms[X.Val2[ElN]] += X.Val3[ElN] * X.Val3[ElN];
    }
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        if (InvColNorms[ColN] > 0.0) {
            InvColNorms[ColN] = 1.0 / TMath::Sqrt(InvColNorms[ColN]);
        }
    }
    for (TSizeTy ElN = 0; ElN < Els; ElN++) {
        X.Val3[ElN] *= InvColNorms[X.Val2[ElN]];
    }
}

// Normalize the columns of X
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X) {
    TSizeTy Cols = X.Len();
    for (TSizeTy ElN = 0; ElN < Cols; ElN++) {
        TLinAlg::Normalize(X[ElN]);
    }
}
// Frobenius norm of matrix A
// TEST
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::FrobNorm2(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    return TLinAlg::Norm2((const_cast<TVVec<TNum<TType>, TSizeTy, ColMajor> &>(X)).Get1DVec());
}
template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::FrobNorm(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X) {
    return TMath::Sqrt(TLinAlg::FrobNorm2(X));
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeL1(TVec<TNum<TType>, TSizeTy>& x) {
    const TType xNorm = TLinAlg::NormL1(x);
    if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeL1(TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    const TType xNorm = TLinAlg::NormL1(x);
    if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::NormLinf(const TVec<TNum<TType>, TSizeTy>& x) {
    TType norm = 0.0; const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        norm = TFlt::GetMx(TFlt::Abs(x[i]), norm);
    return norm;
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::NormLinf(const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    TType norm = 0.0; const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        norm = TFlt::GetMx(TFlt::Abs(x[i].Dat), norm);
    return norm;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeLinf(TVec<TNum<TType>, TSizeTy>& x) {
    const TType xNormLinf = TLinAlg::NormLinf(x);
    if (xNormLinf > 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLinf, x, x); }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeLinf(TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x) {
    const TType xNormLInf = TLinAlg::NormLinf(x);
    if (xNormLInf > 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLInf, x, x); }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetColNormV(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TType>, TSizeTy>& ColNormV) {
    const TSizeTy Cols = X.GetCols();
    GetColNorm2V(X, ColNormV);
    for (TSizeTy i = 0; i < Cols; i++) {
        ColNormV[i] = TMath::Sqrt(ColNormV[i]);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetColNormV(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X,
        TVec<TNum<TType>, TSizeTy>& ColNormV) {
    const TSizeTy Cols = X.Len();
    GetColNorm2V(X, ColNormV);
    for (TSizeTy i = 0; i < Cols; i++) {
        ColNormV[i] = TMath::Sqrt(ColNormV[i]);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetColNorm2V(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TType>, TSizeTy>& ColNormV) {
    const TSizeTy Cols = X.GetCols();

    if (ColNormV.Len() != Cols) { ColNormV.Gen(Cols); }

    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        ColNormV[ColN] = Norm2(X, ColN);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetColNorm2V(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& SpVV,
        TVec<TNum<TType>, TSizeTy>& ColNormV) {
    const TSizeTy Cols = SpVV.Len();

    if (ColNormV.Len() != Cols) { ColNormV.Gen(Cols); }

    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        ColNormV[ColN] = Norm2(SpVV[ColN]);
    }
}

// TEST
// sum columns (Dimension = 1) or rows (Dimension = 2) and store them in vector y
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Sum(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVec<TNum<TType>, TSizeTy>& y, const int Dimension){
    TSizeTy Cols = X.GetCols();
    TSizeTy Rows = X.GetRows();
    if (Dimension == 1) {
        y.Gen(Cols);
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
                y[ColN] += X.At(RowN, ColN);
            }
        }
    }
    else if (Dimension == 2) {
        y.Gen(Rows);
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
                y[RowN] += X.At(RowN, ColN);
            }
        }
    }
    else FailR("Dimension should be 1 or 2");
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::SumRow(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X, const TSizeTy& RowN) {
    EAssertR(RowN < X.GetRows(), "Row index exceeds the number of rows!");
    const TSizeTy Cols = X.GetCols();

    TType Sum = 0;
    for (int ColN = 0; ColN < Cols; ColN++) {
        Sum += X(RowN, ColN);
    }
    return Sum;
}

// TEST
// sum columns (Dimesnion = 2) or rows (Dimension = 1) and store them in vector y
template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlg::Sum(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X, TVec<TType, TSizeTy>& y, const int Dimension) {
    TSizeTy Cols = X.Val2.GetMxVal() + 1;
    TSizeTy Rows = X.Val1.GetMxVal() + 1;
    TSizeTy Els = X.Val1.Len();
    if (Dimension == 1) {
        y.Gen(Cols);
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            //int RowN = X.Val1[ElN];
            TSizeTy ColN = X.Val2[ElN];
            y[ColN] += X.Val3[ElN];
        }
    }
    else if (Dimension == 2) {
        y.Gen(Rows);
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            TSizeTy RowN = X.Val1[ElN];
            //int ColN = X.Val2[ElN];
            y[RowN] += X.Val3[ElN];
        }
    }
    else FailR("Dimension should be 1 or 2");
}

template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlg::Transpose(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
    TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& At) {
    TSizeTy nnz = A.Val1.Len();
    At.Val1.Gen(nnz, 0);
    At.Val2.Gen(nnz, 0);
    At.Val3.Gen(nnz, 0);
    TVec<TSizeTy, TSizeTy> index;
    TIntV::SortGetPerm(A.Val2, At.Val1, index);
    for (TSizeTy ElN = 0; ElN < nnz; ElN++) {
        //At.Val1.Add(A.Val2[ElN]);
        At.Val2.Add(A.Val1[index[ElN]]);
        At.Val3.Add(A.Val3[index[ElN]]);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Sign(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& Mat,
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& Mat2) {
    Mat2 = Mat;
    TSizeTy Cols = Mat2.Len();
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        TSizeTy Els = Mat2[ColN].Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            Mat2[ColN][ElN].Dat = TMath::Sign(Mat2[ColN][ElN].Dat);
        }
    }
}


//x := k * x
// TEST
//x := k * x
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k, TVec<TNum<TType>, TSizeTy>& x) {
    TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++)
        x[i] = k * x[i];
}

//template <class TVal> TVal TLinAlg::GetColMin(const TVVec<TVal>& X, const int& ColN);
//template <class TVal> void TLinAlg::GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV);
// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k, const TVec<TNum<TType>, TSizeTy>& x,
        TVec<TNum<TType>, TSizeTy>& y) {
    TSizeTy Len = x.Len();

    if (y.Empty()) { y.Gen(Len, Len); }
    EAssert(x.Len() == y.Len());

    for (TSizeTy i = 0; i < Len; i++) {
        y[i] = k * x[i];
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& x,
        TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& y) {
    EAssert(x.Len() == y.Len());
    const TSizeTy Len = x.Len();
    for (TSizeTy i = 0; i < Len; i++) {
        y[i].Key = x[i].Key;
        y[i].Dat = k * x[i].Dat;
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Y) {
    const TSizeTy Rows = X.GetRows();
    const TSizeTy Cols = X.GetCols();

    if (Y.Empty()) { Y.Gen(Rows, Cols); }
    EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());

    for (TSizeTy i = 0; i < Rows; i++) {
        for (TSizeTy j = 0; j < Cols; j++) {
            Y(i, j) = k*X(i, j);
        }
    }
}
// Y := k * X
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k,
        const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& X,
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& Y) {
    // sparse column matrix
    Y = X;
    TSizeTy Cols = X.Len();
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        TSizeTy Els = X[ColN].Len();
        for (int ElN = 0; ElN < Els; ElN++) {
            Y[ColN][ElN].Dat = k * X[ColN][ElN].Dat;
        }
    }
}

// y := A * x
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y) {
    if (y.Empty()) { y.Gen(A.GetRows()); }
    EAssert(A.GetCols() == x.Len() && A.GetRows() == y.Len());
#ifdef BLAS
    TLinAlg::Multiply(A, x, y, TLinAlgBlasTranspose::NOTRANS, 1.0, 0.0);
#else
    TSizeTy n = A.GetRows(), m = A.GetCols();
    for (TSizeTy i = 0; i < n; i++) {
        y[i] = 0.0;
        for (TSizeTy j = 0; j < m; j++) {
            y[i] += A(i, j) * x[j];
        }
    }
#endif
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVec<TNum<TType>, TSizeTy>& x, TVVec<TNum<TType>, TSizeTy, ColMajor>& C,
        TSizeTy ColId) {
    EAssert(A.GetCols() == x.Len() && A.GetRows() == C.GetRows());
    TSizeTy n = A.GetRows(), m = A.GetCols();
    for (TSizeTy i = 0; i < n; i++) {
        C(i, ColId) = 0.0;
        for (TSizeTy j = 0; j < m; j++)
            C(i, ColId) += A(i, j) * x[j];
    }
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TSizeTy ColId,
        TVec<TNum<TType>, TSizeTy>& y)  {
    EAssert(A.GetCols() == B.GetRows() && A.GetRows() == y.Len());
    TSizeTy n = A.GetRows(), m = A.GetCols();
    for (TSizeTy i = 0; i < n; i++) {
        y[i] = 0.0;
        for (TSizeTy j = 0; j < m; j++)
            y[i] += A(i, j) * B(j, ColId);
    }
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TSizeTy ColIdB,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& C, TSizeTy ColIdC) {
    EAssert(A.GetCols() == B.GetRows() && A.GetRows() == C.GetRows());
    TSizeTy n = A.GetRows(), m = A.GetCols();
    for (TSizeTy i = 0; i < n; i++) {
        C(i, ColIdC) = 0.0;
        for (TSizeTy j = 0; j < m; j++)
            C(i, ColIdC) += A(i, j) * B(j, ColIdB);
    }
}


//LAPACKE stuff
#ifdef LAPACKE
//A is rewritten in place with orthogonal matrix Q
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(TVVec<TNum<TType>, TSizeTy, ColMajor>& A) {
    TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
    TSizeTy lda = ColMajor ? m : n;
    int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
    TVec<TNum<TType>, TSizeTy> tau; tau.Gen(MAX(1, MIN(m, n)));
    LAPACKE_dgeqrf(Matrix_Layout, m, n, &A(0, 0).Val, lda, &tau[0].Val);
    LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Q) {
    Q = A;
    TLinAlg::QRbasis(Q);
}

//A is rewritten in place with orthogonal matrix Q (column pivoting to improve stability)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(TVVec<TNum<TType>, TSizeTy, ColMajor>& A) {
    TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
    TSizeTy lda = ColMajor ? m : n;
    TSizeTy Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
    TVec<TNum<TType>, TSizeTy> tau(MAX(1, MIN(m, n)));
    TVec<TInt, TSizeTy> jvpt(MAX(1, n));
    LAPACKE_dgeqp3(Matrix_Layout, m, n, &A(0, 0).Val, lda, &jvpt[0].Val, &tau[0].Val);
    LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Q) {
    Q = A;
    TLinAlg::QRcolpbasis(Q);
}

//S S option ensures that A is not modified
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::ThinSVD(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& U, TVec<TNum<TType>, TSizeTy>& S,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& VT) {

    TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols();
    TSizeTy thin_dim = MIN(m, n);

    S.Gen(thin_dim); U.Gen(m, thin_dim); VT.Gen(thin_dim, n);

    int lda = ColMajor ? m : n;
    int ldu = ColMajor ? m : thin_dim;
    int ldvt = ColMajor ? thin_dim : n;

    TVec<TNum<TType>, TSizeTy> superb(MAX(1, MIN(m, n)));
    int opt = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

    /*int lda, ldu, ldvt;
    if (opt == LAPACK_ROW_MAJOR){
    lda = n; ldu = thin_dim; ldvt = n;
    }
    else{
    lda = m; ldu = m; ldvt = thin_dim;
    }*/
    LAPACKE_dgesvd(opt, 'S', 'S', m, n, const_cast<double *>(&A(0, 0).Val), lda, &S[0].Val, &U(0, 0).Val, ldu, &VT(0, 0).Val, ldvt, &superb[0].Val);
}
#else

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(TVVec<TNum<TType>, TSizeTy, ColMajor>& A) {
    throw TExcept::New("TLinAlg::QRbasis(TVVec<TType, TSizeTy, ColMajor>& A) not implemented yet!");
}
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Q) {
    throw TExcept::New("TLinAlg::QRbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q) not implemented yet!");
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(TVVec<TNum<TType>, TSizeTy, ColMajor>& A) {
    throw TExcept::New("TLinAlg::QRcolpbasis(TVVec<TType, TSizeTy, ColMajor>& A) not implemented yet!");
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& Q) {
    throw TExcept::New("TLinAlg::QRcolpbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q) not implemented yet!");
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::ThinSVD(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& U, TVec<TNum<TType>, TSizeTy>& S,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& VT) {
    throw TExcept::New("TLinAlg::thinSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& U, TVec<TType, TSizeTy>& S, TVVec<TType, TSizeTy, ColMajor>& VT) not implemented yet!");
}

#endif
//int TLinAlg::ComputeThinSVD(const TMatrix& X, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters = 2, const double Tol = 1e-6);

//Full matrix times sparse vector
//No need to reserve anything outside, functions currently take care of memory managment for safety
/*template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(TFltVV& ProjMat, TPair<TIntV, TFltV> &, TFltVV& result) {
};
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TFltVV& ProjMat, const TPair<TIntV, TFltV> &, TFltVV& result) {
};*/

////////////////////////////////////////////////////////////////////
// Andrej says:
// http://software.intel.com/en-us/node/468598#86F42CD2-6A3C-4E1F-B686-8690FCC03C75
//call mkl_dcoomm(transa, m, n, k, alpha, matdescra, val, rowind, colind, nnz, b, ldb, beta, c, ldc)
//if transa=='T' op(A) = A' op(m, k) = m else transa == 'N' op(m, k) = k
//A is sparse, B and C are full
//m Number of rows of the matrix A. n Number of columns of the matrix C. k Number of columns of the matrix A.
//A m x k, B op(m, k) x n, C op(m, k) x n
//C := alpha*op(A)*B + beta*C
//  matdescra[6] ={'G', 'G', 'N', 'C', 'Q', 'Q'}; //General, General, Nonunit diagonal, Zero Based indexing

typedef enum { DECOMP_SVD } TLinAlgInverseType;
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Inverse(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor >& B, const TLinAlgInverseType& DecompType) {
    switch (DecompType) {
    case DECOMP_SVD:
        TLinAlg::InverseSVD(A, B);
    }
}

// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::InverseSVD(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& B, const TType& tol) {
    // check the size of B
    if (B.Empty()) { B.Gen(A.GetCols(), A.GetRows()); }
    EAssert(B.GetCols() == A.GetRows() && B.GetRows() == A.GetCols());

    // create temp matrices
    TVec<TNum<TType>, TSizeTy> E;
    TSvd SVD;

    //U.Gen(M.GetRows(), M.GetRows());
    //V.Gen(M.GetCols(), M.GetCols());

    // do the SVD decompostion
#ifdef LAPACKE
    TVVec<TNum<TType>, TSizeTy, ColMajor> U, Vt;
    U.Gen(A.GetRows(), A.GetRows());
    Vt.Gen(A.GetCols(), A.GetCols());

    MKLfunctions::SVDFactorization(A, U, E, Vt);

    const TType Threshold = tol*E[0];
    TType Sum;
    for (TSizeTy i = 0; i < Vt.GetCols(); i++) {
        for (TSizeTy j = 0; j < U.GetRows(); j++) {
            Sum = 0;
            for (TSizeTy k = 0; k < E.Len(); k++) {
                if (E[k] <= Threshold) { break; }
                Sum += Vt(k,i)*U(j,k) / E[k];   // V is transposed
            }
            B(i,j) = Sum;
        }
    }
#else
    TVVec<TNum<TType>, TSizeTy, ColMajor> U, V;
    U.Gen(A.GetRows(), A.GetRows());
    V.Gen(A.GetCols(), A.GetCols());

    SVD.Svd(A, U, E, V);

    // calculate reciprocal values for diagonal matrix = inverse diagonal
    for (TSizeTy i = 0; i < E.Len(); i++) {
        if (E[i] > tol) {
            E[i] = 1 / E[i];
        }
        else {
            E[i] = 0.0;
        }
    }

    // calculate pseudoinverse: M^(-1) = V * E^(-1) * U'
    for (TSizeTy i = 0; i < U.GetCols(); i++) {
        for (TSizeTy j = 0; j < V.GetRows(); j++) {
            double sum = 0.0;
            for (TSizeTy k = 0; k < U.GetCols(); k++) {
                if (E[k] == 0.0) continue;
                sum += E[k] * V.At(i, k) * U.At(j, k);
            }
            B.At(i, j) = sum;
        }
    }
#endif
}

// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::InverseSVD(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& B) {
    // create temp matrices
    TVVec<TNum<TType>, TSizeTy, ColMajor> U, V;
    TVec<TNum<TType>, TSizeTy> E;
    TSvd SVD;

    //U.Gen(M.GetRows(), M.GetRows());
    //V.Gen(M.GetCols(), M.GetCols());

    U.Gen(A.GetRows(), A.GetRows());
    V.Gen(A.GetCols(), A.GetCols());


    // do the SVD decompostion
    SVD.Svd(A, U, E, V);

    // http://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse#Singular_value_decomposition_.28SVD.29
    TType tol = TFlt::Eps * MAX(A.GetRows(), A.GetCols()) * E[E.GetMxValN()];
    // calculate reciprocal values for diagonal matrix = inverse diagonal
    for (TSizeTy i = 0; i < E.Len(); i++) {
        if (E[i] > tol) {
            E[i] = 1 / E[i];
        }
        else {
            E[i] = 0.0;
        }
    }

    // calculate pseudoinverse: M^(-1) = V * E^(-1) * U'
    for (TSizeTy i = 0; i < U.GetCols(); i++) {
        for (TSizeTy j = 0; j < V.GetRows(); j++) {
            TType sum = 0;
            for (TSizeTy k = 0; k < U.GetCols(); k++) {
                if (E[k] == 0.0) continue;
                sum += E[k] * V.At(i, k) * U.At(j, k);
            }
            B.At(i, j) = sum;
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GeneralizedEigDecomp(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVec<TNum<TType>, TSizeTy>& EigValV,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& V) {
#ifdef BLAS
    const TSizeTy Dim = A.GetRows();

    // asserts
    EAssert(Dim > 0);
    EAssert(A.GetCols() == Dim && B.GetRows() == Dim && B.GetCols() == Dim);

    // output variables
    V.Gen(Dim, Dim);
    EigValV.Gen(Dim);

    TVec<TNum<TType>, TSizeTy> AlphaR(Dim);
    TVec<TNum<TType>, TSizeTy> AlphaI(Dim);
    TVec<TNum<TType>, TSizeTy> Beta(Dim);

    // construct input
    TSizeTy LeadingDimA = ColMajor ? A.GetRows() : A.GetCols();
    TSizeTy LeadingDimB = ColMajor ? B.GetRows() : B.GetCols(); // TODO is this correct???
    TSizeTy LeadingDimV = ColMajor ? V.GetRows() : V.GetCols();
    int Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

    TVVec<TNum<TType>, TSizeTy, ColMajor> A1 = A;   // A is overwritten
    TVVec<TNum<TType>, TSizeTy, ColMajor> B1 = B;   // B is overwritten

    // A will get overwritten
    int Info = LAPACKE_dggev(
        Layout,                 // matrix layout
        'N',                    // don't compute left eigenvectors
        'V',                    // compute right eigenvectors
        Dim,                    // order of matrices A,B,Vl,Vr
        (double*) &A1(0,0),     // A
        LeadingDimA,            // leading dimension of A
        (double*) &B1(0,0),     // B
        LeadingDimB,            // leading dimension of B
        (double*) &AlphaR[0],   // real part of the eigenvalues
        (double*) &AlphaI[0],   // imaginary part of the eigenvalues
        (double*) &Beta[0],     // used to compute the eigenvalues
        nullptr,                // left eigenvectors Vl
        1,                      // leading dimension of Vl (>= 1)
        (double*) &V(0,0),      // right eigenvectors Vr
        LeadingDimV             // leading dimension of Vr
    );

    EAssertR(Info == 0, "Failed to compute the generalized eigen decomposition, error: " + TInt::GetStr(Info) + "!");

    // construct the eigenvalues
    // the generalized eigenvalues will be: (AlphaR[j] + AlphaI[j]*i) / Beta[j], j = 1,...,N
    for (int i = 0; i < Dim; i++) {
        EAssertR(AlphaI[i] == 0.0, "Got complex eigenvalues!");
        EAssertR(Beta[i] != 0.0, "Infinite generalized eigenvalue!");
        EigValV[i] = AlphaR[i] / Beta[i];
    }
#else
    throw TExcept::New("TLinAlg::GeneralizedEigDecomp: Not implemented!");
#endif
}

#ifdef INTEL
//Be careful C should be of the proper size! if not populated (works only for rowmajor!)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplySF(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, false>& B,
    TVVec<TType, TSizeTy, ColMajor>& C, const TStr& transa, const int& format){
    //B is row_major
    TSizeTy m, n, k, ldb, ldc;

    //ldb = ColMajor ? B.GetRows() : B.GetCols();
    //ldc = ColMajor ? C.GetRows() : C.GetCols();

    ldb = B.GetCols();
    ldc = C.GetCols();

    n = C.GetCols();

    if (transa == "N"){
        m = C.GetRows();
        k = B.GetRows();
    }
    else{
        k = C.GetRows();
        m = B.GetRows();
    }
    double alpha = 1; double beta = 0; char matdescra[6] = { 'G', 'G', 'N', 'C', 'Q', 'Q' };
    TSizeTy nnz = A.Val3.Len();
    if (format == 0){
        MKL_DCOOMM(const_cast<char *>(transa.CStr()), &m, &n, &k, &alpha, matdescra, const_cast<double *>(&A.Val3[0].Val), const_cast<TSizeTy *>(&A.Val1[0].Val), const_cast<TSizeTy *>(&A.Val2[0].Val), &nnz, const_cast<double *>(&B(0, 0).Val), &ldb, &beta, const_cast<double *>(&C(0, 0).Val), &ldc);
    }
    else{
        //call mkl_dcsrmm(transa, m, n, k, alpha, matdescra, val, indx, pntrb, pntre, b, ldb, beta, c, ldc)
        printf("Max row %d, max column %d\n", A.Val1.Len() - 1, A.Val2.Len());
        mkl_dcsrmm(const_cast<char *>(transa.CStr()), &m, &n, &k, &alpha, matdescra, const_cast<double *>(&A.Val3[0].Val), const_cast<TSizeTy *>(&A.Val2[0].Val), const_cast<TSizeTy *>(&A.Val1[0].Val), const_cast<TSizeTy *>(&A.Val1[1].Val), const_cast<double *>(&B(0, 0).Val), &ldb, &beta, const_cast<double *>(&C(0, 0).Val), &ldc);
    }

}

//B will not be needed anymore (works only for rowmajor!)
//TODO to much hacking
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyFS(TVVec<TType, TSizeTy, ColMajor>& B, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
    TVVec<TType, TSizeTy, ColMajor>& C){
    C.SwitchDim();
    TTmStopWatch time;
    time.Start();
    B.Transpose();
    time.Stop("In place transpose of B costs: ");
    time.Start();
    MultiplySF(A, B, C, TStr("T"));//Heavy hacking
    time.Stop("Full times sparse multi costs: ");
    time.Start();
    B.Transpose();
    time.Stop("In place transpose of B costs: ");
    time.Start();
    C.Transpose();
    time.Stop("In place transpose of C costs: ");
}
#else
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplySF(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, false>& B,
    TVVec<TType, TSizeTy, ColMajor>& C, const TStr& transa, const int& format) {
    throw TExcept::New("TLinAlg::MultiplySF(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, false>& B, TVVec<TType, TSizeTy, ColMajor>& C, const TStr& transa, const int& format) not implemented yet!");
}

template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyFS(TVVec<TType, TSizeTy, ColMajor>& B, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
    TVVec<TType, TSizeTy, ColMajor>& C) {
    throw TExcept::New("TLinAlg::MultiplyFS(TVVec<TType, TSizeTy, ColMajor>& B, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A, TVVec<TType, TSizeTy, ColMajor>& C) not implemented yet!");
}

#endif

// y := A * x
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, TVec<TType, TSizeTy>& y) {
    // Assumptions on x
    EAssert(x.Val1.Len() == x.Val2.Len());
    // Dimensions must match
    EAssert(A.GetRows() >= (x.Val1.Len() == 0 ? 0 : x.Val1[x.Val1.GetMxValN()] + 1) && A.GetCols() == y.Len());
    for (TSizeTy RowN = 0; RowN < A.GetRows(); RowN++) {
        y[RowN] = 0.0;
        for (TSizeTy ElN = 0; ElN < x.Val1.Len(); ElN++) {
            y[RowN] += A.At(RowN, x.Val1[ElN]) * x.Val2[ElN];
        }
    }
}
//y  := x' * A ... row data!!
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TType, TSizeTy>& y) {
    // Assumptions on x
    EAssert(x.Val1.Len() == x.Val2.Len());
    // Dimensions must match
    EAssert(A.GetCols() >= (x.Val1.Len() == 0 ? 0 : x.Val1[x.Val1.GetMxValN()] + 1) && A.GetRows() == y.Len());
    TLinAlgTransform::FillZero(y);
    int nnz = x.Val1.Len();
    for (TSizeTy i = 0; i < nnz; i++) {
        TVec<TType, TSizeTy> row;
        (const_cast<TVVec<TType, TSizeTy, ColMajor> &>(A)).GetRowPtr(x.Val1[i], row);
        //printf("vrstic %d, stolpcev %d\n", A.GetRows(), A.GetCols());
        //printf("Row len %d\n", row.Len());
        //printf("i je %d, vrstica je %d\n", i, x.Val1[i]);
        //TLinAlg::LinCombInPlace(x.Val2[i], row, 0.0, y);
#ifdef BLAS
        //y = k * x + y
        //cblas_daxpy(row.Len(), x.Val2[i].Val, &row[0].Val, 1, &y[0].Val, 1);
        //cblas_daxpy(x.Len(), k_, (Loc *)&x[0].Val, 1, (Loc *) &y[0].Val, 1);
        AddVec(x.Val2[i].Val, row, y);
#else
        TLinAlg::LinCombInPlace(x.Val2[i].Val, row, 1.0, y);
#endif          //printf("Lincomb does not fail\n");
    }
}
// TEST Move to BLAS
// y := A' * x
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x,
        TVec<TNum<TType>, TSizeTy>& y) {
    if (y.Empty()) y.Gen(A.GetCols());
    EAssert(A.GetRows() == x.Len() && A.GetCols() == y.Len());
    TSizeTy n = A.GetCols(), m = A.GetRows();
    for (TSizeTy i = 0; i < n; i++) {
        y[i] = 0.0;
        for (TSizeTy j = 0; j < m; j++)
            y[i] += A(j, i) * x[j];
    }
}

#ifdef BLAS
typedef enum { NOTRANS = 0, TRANS = 1 } TLinAlgBlasTranspose;

template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>,
        TSizeTy, ColMajor>& C, const int& BlasTransposeFlagA, const int& BlasTransposeFlagB) {
        //C := alpha*op(A)*op(B) + beta*C,
        //where:
        //op(X) is one of op(X) = X, or op(X) = XT, or op(X) = XH,
        //alpha and beta are scalars,
        //A, B and C are matrices:
        //op(A) is an m-by-k matrix,
        //op(B) is a k-by-n matrix,
        //C is an m-by-n matrix.
        TSizeTy m, n, k, lda, ldb, ldc;
        if (BlasTransposeFlagA == TLinAlg::TLinAlgBlasTranspose::TRANS) {
            m = A.GetCols();
            k = A.GetRows();
            lda = ColMajor ? k : m;
        }
        else {
            m = A.GetRows();
            k = A.GetCols();
            lda = ColMajor ? m : k;
        }
        if (BlasTransposeFlagB == TLinAlg::TLinAlgBlasTranspose::TRANS) {
            EAssert(k == B.GetCols());
            n = B.GetRows();
            ldb = ColMajor ? n : k;
        }
        else {
            EAssert(k == B.GetRows());
            n = B.GetCols();
            ldb = ColMajor ? k : n;
        }
        EAssert(m == C.GetRows() && n == C.GetCols());
        // simplified interface

        ldc = ColMajor ? m : n;

#ifdef BLAS //Standard CBLAS interface
        CBLAS_TRANSPOSE BlasTransA = (BlasTransposeFlagA == TLinAlgBlasTranspose::TRANS) ? CblasTrans : CblasNoTrans;
        CBLAS_TRANSPOSE BlasTransB = (BlasTransposeFlagB == TLinAlgBlasTranspose::TRANS) ? CblasTrans : CblasNoTrans;
        CBLAS_ORDER Matrix_Layout = ColMajor ? CblasColMajor : CblasRowMajor;
        if (TypeCheck::is_double<TType>::value == true){
            typedef double Loc;
            double alpha = 1.0, beta = 0.0;
            cblas_dgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, alpha, (Loc *)&A(0, 0).Val, lda, (Loc *)&B(0, 0).Val, ldb, beta, (Loc *)&C(0, 0).Val, ldc);
        }
        else
        if (TypeCheck::is_float<TType>::value == true){
            typedef float Loc;
            float alpha = 1.0f, beta = 0.0f;
            cblas_sgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, alpha, (Loc *)&A(0, 0).Val, lda, (Loc *)&B(0, 0).Val, ldb, beta, (Loc *)&C(0, 0).Val, ldc);
        }
        else
        if (TypeCheck::is_complex_double<TType>::value == true){
            typedef double Loc;
            std::complex<double> alpha(1.0); std::complex<double> beta(0.0);
            cblas_zgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, (const Loc *)&alpha, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&B(0, 0).Val, ldb, (const Loc *)&beta, (Loc *)&C(0, 0).Val, ldc);
        }
        else
        if (TypeCheck::is_complex_float<TType>::value == true){
            typedef float Loc;
            std::complex<float> alpha(1.0f); std::complex<float> beta(0.0f);
            cblas_cgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, (const Loc *)&alpha, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&B(0, 0).Val, ldb, (const Loc *)&beta, (Loc *)&C(0, 0).Val, ldc);
        }


#else //Fortran 77 style interface, all values must be passed by reference!
        TStr TransposeFlagA = "N";
        TStr TransposeFlagB = "N";
        if (BlasTransposeFlagA){ TransposeFlagA = "T"; /*lda = k;*/ }
        if (BlasTransposeFlagB){ TransposeFlagB = "T"; /*ldb = n;*/ }
#ifdef AMD
        DGEMM(TransposeFlagA.CStr(), TransposeFlagB.CStr(), &m, &n, &k, &alpha, &A(0, 0).Val, &lda, &B(0, 0).Val, &ldb, &beta, &C(0, 0).Val, &ldc, TransposeFlagA.Len(), TransposeFlagB.Len());
#else
        dgemm(TransposeFlagA.CStr(), TransposeFlagB.CStr(), &m, &n, &k, &alpha, &A(0, 0).Val, &lda, &B(0, 0).Val, &ldb, &beta, &C(0, 0).Val, &ldc);
#endif
#endif
    }
#else

// C = op(A) * op(B)
template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
    const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>,
    TSizeTy, ColMajor>& C, const int& BlasTransposeFlagA, const int& BlasTransposeFlagB) {
    throw TExcept::New("TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>, TSizeTy, ColMajor>& C, const int& BlasTransposeFlagA, const int& BlasTransposeFlagB) not implemented yet!");
}

#endif

#ifdef BLAS
//Andrej ToDo In the future replace TType with TNum<type> and change double to type
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA, TType alpha, TType beta) {
    TSizeTy m = A.GetRows();
    TSizeTy n = A.GetCols();
    //Can we multiply and store in y?
    if (BlasTransposeFlagA) {//A'*x n*m x m -> n
        EAssertR(x.Len() == m, "TLinAlg::Multiply: Invalid dimension of input vector!");
        if (y.Reserved() != n) {    // TODO should I do this here?? Meybe if the length is > n it would also be OK??
            y.Gen(n, n);
        }
    }
    else{//A*x  m x n * n -> m
        EAssertR(x.Len() == n, "TLinAlg::Multiply: Invalid dimension of input vector!");
        if (y.Reserved() != m) {    // TODO should I do this here?? Meybe if the length is > m it would also be OK??
            y.Gen(m, m);
        }
    }
    TSizeTy lda = ColMajor ? m : n;
    TSizeTy incx = /*ColMajor ? x.Len() :*/ 1;
    TSizeTy incy = /*ColMajor ? y.Len() :*/ 1;
    CBLAS_ORDER Matrix_Layout = ColMajor ? CblasColMajor : CblasRowMajor;

#ifdef BLAS //Standard CBLAS interface
    CBLAS_TRANSPOSE BlasTransA = BlasTransposeFlagA ? CblasTrans : CblasNoTrans;
    /*if (BlasTransposeFlagA){ BlasTransA = CblasTrans; }*/
    if (TypeCheck::is_double<TType>::value == true){
        typedef double Loc;
        double alpha_ = alpha; double beta_ = beta;
        cblas_dgemv(Matrix_Layout, BlasTransA, m, n, alpha_, (Loc *)&A(0, 0).Val, lda, (Loc *)&x[0].Val, incx, beta_, (Loc *)&y[0].Val, incy);
    }
    else
    if (TypeCheck::is_float<TType>::value == true){
        typedef float Loc;
        float alpha_ = (float)alpha; float beta_ = (float)beta;
        cblas_sgemv(Matrix_Layout, BlasTransA, m, n, alpha_, (Loc *)&A(0, 0).Val, lda, (Loc *)&x[0].Val, incx, beta_, (Loc *)&y[0].Val, incy);
    }
    else
    if (TypeCheck::is_complex_double<TType>::value == true){
        typedef double Loc;
        std::complex<double>  alpha_(alpha); std::complex<double>  beta_(beta);
        cblas_zgemv(Matrix_Layout, BlasTransA, m, n, (const Loc *)&alpha_, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&x[0].Val, incx, (const Loc *)&beta_, (Loc *)&y[0].Val, incy);
    }
    else
    if (TypeCheck::is_complex_float<TType>::value == true){
        typedef float Loc;
        std::complex<float>  alpha_((float)alpha); std::complex<double>  beta_((float)beta);
        cblas_cgemv(Matrix_Layout, BlasTransA, m, n, (const Loc *)&alpha_, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&x[0].Val, incx, (const Loc *)&beta_, (Loc *)&y[0].Val, incy);
    }

#else //Fortran 77 style interface, all values must be passed by reference!
    TStr TransposeFlag = "N";
    if (BlasTransposeFlagA){ TransposeFlag = 'T'; }
#ifdef AMD
    DGEMV(TransposeFlag.CStr(), &m, &n, &alpha, &A(0, 0).Val, &lda, &x[0].Val, &incx, &beta, &y[0].Val, &incy, TransposeFlag.Len());
    //DGEMV(char *trans, int *m, int *n, double *alpha, double *a, int *lda, double *x, int *incx, double *beta, double *y, int *incy, int trans_len);
#else
    dgemv(TransposeFlag.CStr(), &m, &n, &alpha, &A(0, 0).Val, &lda, &x[0].Val, &incx, &beta, &y[0].Val, &incy);
#endif
#endif
}
#else

// y := alpha*op(A)*x + beta*y, where op(A) = A -- N, op(A) = A' -- T, op(A) = conj(A') -- C (only for complex)
//Andrej ToDo In the future replace TType with TNum<type> and change double to type
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA, TType alpha, TType beta) {
    throw TExcept::New("TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA, TType alpha, TType beta) not implemented yet!");
}

#endif

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>, TSizeTy, ColMajor>& C) {
    if (C.Empty()) { C.Gen(A.GetRows(), B.GetCols()); }

    EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() &&
            A.GetCols() == B.GetRows());
#ifdef BLAS
    TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::NOTRANS, TLinAlgBlasTranspose::NOTRANS);
#else
    TSizeTy RowsA = A.GetRows();
    TSizeTy ColsA = A.GetCols();
    TSizeTy ColsB = B.GetCols();
    C.PutAll(0.0);
    for (TSizeTy RowN = 0; RowN < RowsA; RowN++) {
        for (TSizeTy ColAN = 0; ColAN < ColsA; ColAN++) {
            TType Weight = A(RowN, ColAN);
            for (TSizeTy ColBN = 0; ColBN < ColsB; ColBN++) {
                C(RowN, ColBN) += Weight * B(ColAN, ColBN);
            }
        }
    }
#endif
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& C) {
    if (C.Empty()) { C.Gen(A.GetCols(), B.GetCols()); }
    EAssert(A.GetCols() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetRows() == B.GetRows());
#ifdef BLAS
    TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::TRANS, TLinAlgBlasTranspose::NOTRANS);
#else
    TSizeTy n = C.GetRows(), m = C.GetCols(), l = A.GetRows(); TType sum;
    for (TSizeTy i = 0; i < n; i++) {
        for (TSizeTy j = 0; j < m; j++) {
            sum = 0.0;
            for (TSizeTy k = 0; k < l; k++)
                sum += A(k, i)*B(k, j);
            C(i, j) = sum;
        }
    }
#endif
}

template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
    TVVec<TType, TSizeTy, ColMajor>& C){
    // B well defined
    EAssert(B.Val1.Len() == B.Val2.Len() && B.Val2.Len() == B.Val3.Len());
    // Dimensions must match
    C.PutAll(0.0);
    if (B.Val1.Len() == 0) {
        return;
    }
#ifdef INTELS
    TLinAlg::MultiplyFS(const_cast<TVVec<TType, TSizeTy, ColMajor> &>(A), B, C);
#else
    TSizeTy Nonzeros = B.Val1.Len();
    IndexType MaxRowN = B.Val1[B.Val1.GetMxValN()];
    IndexType MaxColN = B.Val2[B.Val2.GetMxValN()];
    EAssert(A.GetRows() == C.GetRows() && (MaxColN + 1) <= C.GetCols() && (MaxRowN + 1) <= A.GetCols());
    for (TSizeTy RowN = 0; RowN < A.GetRows(); RowN++) {
        for (TSizeTy ElN = 0; ElN < Nonzeros; ElN++) {
            C.At(RowN, B.Val2[ElN]) += A.At(RowN, B.Val1[ElN]) * B.Val3[ElN];
        }
    }
#endif
}

template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
    TVVec<TType, TSizeTy, ColMajor>& C) {
    // B well defined
    EAssert(B.Val1.Len() == B.Val2.Len() && B.Val2.Len() == B.Val3.Len());
    // Dimensions must match
    C.PutAll(0.0);
    if (B.Val1.Len() == 0) {
        return;
    }
    TSizeTy Nonzeros = B.Val1.Len();
    IndexType MaxRowN = B.Val1[B.Val1.GetMxValN()];
    IndexType MaxColN = B.Val2[B.Val2.GetMxValN()];
    EAssert(A.GetCols() == C.GetRows() && (MaxColN + 1) <= C.GetCols() && (MaxRowN + 1) <= A.GetRows());
    for (TSizeTy RowN = 0; RowN < A.GetCols(); RowN++) {
        for (TSizeTy ElN = 0; ElN < Nonzeros; ElN++) {
            C.At(RowN, B.Val2[ElN]) += A.At(B.Val1[ElN], RowN) * B.Val3[ElN];
        }
    }
}
// TEST
// C := A * B

//#if !defined(INTEL) || defined(INDEX_64)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
    TVVec<TType, TSizeTy, ColMajor>& C) {
    // A well defined
    EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
    // Dimensions must match
    C.PutAll(0.0);
    if (A.Val1.Len() == 0) {
        return;
    }
#if !defined(INTEL) || defined(INDEX_64)
    TSizeTy Nonzeros = A.Val1.Len();
    TSizeTy MaxRowN = A.Val1[A.Val1.GetMxValN()];
    TSizeTy MaxColN = A.Val2[A.Val2.GetMxValN()];
    EAssert(B.GetCols() == C.GetCols() && (MaxRowN + 1) <= C.GetRows() && (MaxColN + 1) <= B.GetRows());
    for (TSizeTy ColN = 0; ColN < B.GetCols(); ColN++) {
        for (TSizeTy ElN = 0; ElN < Nonzeros; ElN++) {
            C.At(A.Val1[ElN], ColN) += A.Val3[ElN] * B.At(A.Val2[ElN], ColN);
        }
    }
#else
    TLinAlg::MultiplySF(A, B, C);
#endif
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
    TVVec<TType, TSizeTy, ColMajor>& C) {
    // B well defined
    EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
    // Dimensions must match
    C.PutAll(0.0);
    if (A.Val1.Len() == 0) {
        return;
    }
#if !defined(INTEL) || defined(INDEX_64)
    TSizeTy Nonzeros = A.Val1.Len();
    TSizeTy MaxRowN = A.Val1[A.Val1.GetMxValN()];
    TSizeTy MaxColN = A.Val2[A.Val2.GetMxValN()];
    EAssert(B.GetCols() == C.GetCols() && (MaxColN + 1) <= C.GetRows() && (MaxRowN + 1) <= B.GetRows());
    for (TSizeTy ColN = 0; ColN < B.GetCols(); ColN++) {
        for (TSizeTy ElN = 0; ElN < Nonzeros; ElN++) {
            C.At(A.Val2[ElN], ColN) += A.Val3[ElN] * B.At(A.Val1[ElN], ColN);
        }
    }
#else
    TLinAlg::MultiplySF(A, B, C, "T");
#endif
}

// DENSE-SPARSECOLMAT, SPARSECOLMAT-DENSE
// C := A * B

// DENSE-SPARSECOLMAT, SPARSECOLMAT-DENSE
// C := A * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
void TLinAlg::Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
    // B = sparse column matrix
    if (C.Empty()) {
        C.Gen(A.GetRows(), B.Len());
    }
    else {
        EAssert(A.GetRows() == C.GetRows() && B.Len() == C.GetCols());
    }
    EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.GetCols());
    int Cols = B.Len();
    int Rows = A.GetRows();
    C.PutAll(0.0);
    for (int RowN = 0; RowN < Rows; RowN++) {
        for (int ColN = 0; ColN < Cols; ColN++) {
            int Els = B[ColN].Len();
            for (int ElN = 0; ElN < Els; ElN++) {
                C.At(RowN, ColN) += A.At(RowN, B[ColN][ElN].Key) * B[ColN][ElN].Dat;
            }
        }
    }
}

// C:= A' * B
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& B,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& C) {
    // C = A' B = (B' A)'
#ifdef INTELBETA
    TTriple<TVec<IndexType, TSizeTy>, TVec<TInt, TSizeTy>, TVec<TNum<TType>, TSizeTy>> BB;
    TLinAlg::Convert(B, BB); // convert the matrix to a coordinate form
    TVVec<TNum<TType>, TSizeTy, ColMajor> CC(B.Len(), A.GetCols());
    TLinAlg::MultiplyT(BB, A, CC);
    if (C.Empty()) {
        C.Gen(A.GetCols(), B.Len());
    }
    else {
        EAssert(C.GetRows() == A.GetCols() && C.GetCols() == B.Len());
    }
    TLinAlg::Transpose(CC, C);
#else
    // B = sparse column matrix
    if (C.Empty()) {
        C.Gen(A.GetCols(), B.Len());
    }
    else {
        EAssert(A.GetCols() == C.GetRows() && B.Len() == C.GetCols());
    }
    EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.GetRows());
    TSizeTy Cols = B.Len();
    TSizeTy Rows = A.GetCols();
    C.PutAll(0.0);
    for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
        for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
            TSizeTy Els = B[ColN].Len();
            for (TSizeTy ElN = 0; ElN < Els; ElN++) {
                C.At(RowN, ColN) += A.At(B[ColN][ElN].Key, RowN) * B[ColN][ElN].Dat;
            }
        }
    }
#endif
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& C) {
    // A = sparse column matrix
    EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= B.GetRows());
    TSizeTy ColsB = B.GetCols();
    //int RowsB = B.GetRows();
    TSizeTy ColsA = A.Len();
    if (C.Empty()) {
        C.Gen(ColsA, ColsB);
    }
    else {
        EAssert(C.GetRows() == ColsA && C.GetCols() == ColsB);
    }
    C.PutAll(0.0);
    for (TSizeTy RowN = 0; RowN < ColsA; RowN++) {
        for (TSizeTy ColN = 0; ColN < ColsB; ColN++) {
            TSizeTy Els = A[RowN].Len();
            for (TSizeTy ElN = 0; ElN < Els; ElN++) {
                C.At(RowN, ColN) += A[RowN][ElN].Dat * B.At(A[RowN][ElN].Key, ColN);
            }
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& B,
        TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& C) {

    EAssert(A.GetCols() >= TLinAlgSearch::GetMaxDimIdx(B) + 1);
    TSizeTy Rows = A.GetRows();
    TSizeTy Cols = B.Len();

    C.Gen(Cols);
    for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
        for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
            TType Val = 0.0;
            TSizeTy Els = B[ColN].Len();
            for (TSizeTy ElN = 0; ElN < Els; ElN++) {
                Val += A(RowN, B[ColN][ElN].Key) * B[ColN][ElN].Dat;
            }
            C[ColN].Add(TKeyDat<TNum<TSizeTy>, TNum<TType>>(RowN, Val));
        }
    }
}


template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& A,
        const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& B,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& C) {
    //// A, B = sparse column matrix
    TSizeTy ColsA = A.Len();
    TSizeTy ColsB = B.Len();
    if (C.Empty()) {
        C.Gen(ColsA, ColsB);
    }
    else {
        EAssert(ColsA == C.GetRows() && ColsB == C.GetCols());
    }
    for (TSizeTy RowN = 0; RowN < ColsA; RowN++) {
        for (TSizeTy ColN = 0; ColN < ColsB; ColN++) {
            C.At(RowN, ColN) = TLinAlg::DotProduct(A[RowN], B[ColN]);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>, TSizeTy>& A,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& b,
        TVec<TNum<TType>, TSizeTy>& c) {
    //// A = sparse column matrix, b = sparse vector
    TSizeTy ColsA = A.Len();

    if (c.Empty()) {
        c.Gen(ColsA);
    }
    else {
        EAssert(ColsA == c.Len());
    }
    for (TSizeTy RowN = 0; RowN < ColsA; RowN++) {
        c[RowN] = TLinAlg::DotProduct(A[RowN], b);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>, TSizeTy>& b,
        TVec<TNum<TType>, TSizeTy>& c) {
    //// A = dense matrix, b = sparse vector
    TSizeTy ColsA = A.GetCols();
    // check if the maximum index of the sparse vector is lesser than number of matrix rows
    EAssert(TLinAlgSearch::GetMaxDimIdx(b) <= A.GetRows());

    if (c.Empty()) {
        c.Gen(ColsA);
    }
    else {
        EAssert(ColsA == c.Len());
    }
    c.PutAll(0.0);
    for (TSizeTy ColN = 0; ColN < ColsA; ColN++) {
        int Els = b.Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            c[ColN] += b[ElN].Dat * A.At(b[ElN].Key, ColN);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVec<TVec<TKeyDat<TNum<TSizeTy>, TNum<TType>>,TSizeTy>,TSizeTy>& A,
        const TVec<TNum<TType>, TSizeTy>& b, TVec<TNum<TType>, TSizeTy>& c) {
    //// A = sparse column matrix, b = dense vector
    TSizeTy ColsA = A.Len();
    // check if the maximum index of the sparse matrix is lesser than length of vector b
    EAssert(TLinAlgSearch::GetMaxDimIdx(A) <= b.Len());

    if (c.Empty()) {
        c.Gen(ColsA);
    }
    else {
        EAssert(ColsA == c.Len());
    }
    c.PutAll(0.0);
    for (TSizeTy ColN = 0; ColN < ColsA; ColN++) {
        TSizeTy Els = A[ColN].Len();
        for (TSizeTy ElN = 0; ElN < Els; ElN++) {
            c[ColN] += A[ColN][ElN].Dat * b[A[ColN][ElN].Key];
        }
    }
}

//#ifdef INTEL
//  void TLinAlg::Multiply(const TFltVV & ProjMat, const TPair<TIntV, TFltV> & Doc, TFltV & Result);
//#endif

typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Gemm(const double& Alpha, const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, const double& Beta,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& C,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& D, const int& TransposeFlags) {

    bool tA = (TransposeFlags & GEMM_A_T) == GEMM_A_T;
    bool tB = (TransposeFlags & GEMM_B_T) == GEMM_B_T;
    bool tC = (TransposeFlags & GEMM_C_T) == GEMM_C_T;


    // setting dimensions
    TSizeTy a_i = tA ? A.GetRows() : A.GetCols();
    TSizeTy a_j = tA ? A.GetCols() : A.GetRows();

    TSizeTy b_i = tB ? B.GetRows() : B.GetCols();
    TSizeTy b_j = tB ? B.GetCols() : B.GetRows();

    TSizeTy c_i = tC ? C.GetRows() : C.GetCols();
    TSizeTy c_j = tC ? C.GetCols() : C.GetRows();

    TSizeTy d_i = D.GetCols();
    TSizeTy d_j = D.GetRows();

    // assertions for dimensions
    EAssert(a_j == c_j && b_i == c_i && a_i == b_j && c_i == d_i && c_j == d_j);

    double Aij, Bij, Cij;

    // rows of D
    for (TSizeTy j = 0; j < a_j; j++) {
        // cols of D
        for (TSizeTy i = 0; i < b_i; i++) {
            // not optimized for speed - naive algorithm
            double sum = 0.0;
            // cols of A
            for (TSizeTy k = 0; k < a_i; k++) {
                Aij = tA ? A.At(k, j) : A.At(j, k);
                Bij = tB ? B.At(i, k) : B.At(k, i);
                sum += Alpha * Aij * Bij;
            }
            Cij = tC ? C.At(i, j) : C.At(j, i);
            sum += Beta * Cij;
            D.At(j, i) = sum;
        }
    }

}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Transpose(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
        TVVec<TNum<TType>, TSizeTy, ColMajor>& B) {
    if (B.Empty()) { B.Gen(A.GetCols(), A.GetRows()); }

    EAssert(B.GetRows() == A.GetCols() && B.GetCols() == A.GetRows());
    for (TSizeTy i = 0; i < A.GetCols(); i++) {
        for (TSizeTy j = 0; j < A.GetRows(); j++) {
            B.At(i, j) = A.At(j, i);
        }
    }
}

// performes Gram-Schmidt ortogonalization on elements of Q
template <class TSizeTy>
void TLinAlg::GS(TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Q) {
    EAssert(Q.Len() > 0);
    TSizeTy m = Q.Len(); // int n = Q[0].Len();
    for (TSizeTy i = 0; i < m; i++) {
        printf("%d\r", i);
        for (TSizeTy j = 0; j < i; j++) {
            double r = TLinAlg::DotProduct(Q[i], Q[j]);
            TLinAlg::AddVec(-r, Q[j], Q[i], Q[i]);
        }
        TLinAlg::Normalize(Q[i]);
    }
    printf("\n");
}

// TEST
// Gram-Schmidt on columns of matrix Q
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GS(TVVec<TNum<TType>, TSizeTy, ColMajor>& Q) {
    TSizeTy m = Q.GetCols(), n = Q.GetRows();
    for (TSizeTy i = 0; i < m; i++) {
        printf("%d\r", i);
        for (TSizeTy j = 0; j < i; j++) {
            TType r = TLinAlg::DotProduct(Q, i, Q, j);
            TLinAlg::AddVec(-r, Q, j, Q, i);
        }
        TType nr = TLinAlg::Norm(Q, i);
        for (TSizeTy k = 0; k < n; k++)
            Q(k, i) = Q(k, i) / nr;
    }
    printf("\n");
}
// Modified Gram-Schmidt on columns of matrix Q
void TLinAlg::MGS(TFltVV& Q) {
    int Cols = Q.GetCols(), Rows = Q.GetRows();
    EAssertR(Rows >= Cols, "TLinAlg::MGS: number of rows should be greater or equal to the number of cols");
    for (int ColN = 0; ColN < Cols; ColN++) {
        TLinAlg::NormalizeColumns(Q);
        for (int ColN2 = ColN + 1; ColN2 < Cols; ColN2++) {
            double r = TLinAlg::DotProduct(Q, ColN, Q, ColN2);
            TLinAlg::AddVec(-r, Q, ColN, Q, ColN2);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlg::Pow(const TVVec<TNum<TType>, TSizeTy, ColMajor>& Mat,
        const int& k, TVVec<TNum<TType>, TSizeTy, ColMajor>& PowVV) {
    EAssertR(Mat.GetRows() == Mat.GetCols(), "TLinAlg::Pow: Can only compute powers of square matrices!");

    const TSizeTy Dim = Mat.GetRows();

    if (k == 0) {
        TLinAlgTransform::Identity(Dim, PowVV);
    } else if (k < 0) {
        TVVec<TNum<TType>, TSizeTy, ColMajor> InverseVV;
        TLinAlg::Inverse(Mat, InverseVV, TLinAlgInverseType::DECOMP_SVD);
        Pow(InverseVV, -k, PowVV);
    } else {
        PowVV.Gen(Dim, Dim);

        // we will compute the power using the binary algorithm
        // we will always hold the newest values in X, so when
        // finishing the algorithm, the result will be in X

        // X <- A
        TVVec<TNum<TType>, TSizeTy, ColMajor> TempMat(Mat);         // temporary matrix

        // pointers, so swapping is faster
        TVVec<TNum<TType>, TSizeTy, ColMajor>* X = &TempMat;
        TVVec<TNum<TType>, TSizeTy, ColMajor>* X1 = &PowVV;         // use the space already available

        // temporary variables
        TVVec<TNum<TType>, TSizeTy, ColMajor>* Temp;

        // do the work
        uint k1 = (uint) k;
        uint n = (uint) TMath::Log2(k);

        uint b;

        for (uint i = 1; i <= n; i++) {
            b = (k1 >> (n-i)) & 1;

            // X <- X*X
            TLinAlg::Multiply(*X, *X, *X1);
            // swap X and X1 so that X holds the content
            Temp = X1;
            X1 = X;
            X = Temp;
            if (b == 1) {
                // X <- X*A
                TLinAlg::Multiply(*X, Mat, *X1);
                // swap X and X1 so that X holds the content
                Temp = X1;
                X1 = X;
                X = Temp;
            }
        }

        if (&PowVV != X) {
            // the values are in X, but we are returning X1
            // copy X to PowVV
            PowVV = *X;
        }
    }
}

// returns a sub matrix of the input matrix in range [StartRow, EndRow) x [StartCol, EndCol)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::SubMat(const TVVec<TNum<TType>, TSizeTy, ColMajor>& Mat,
        const TSizeTy& StartRow, const TSizeTy& EndRow, const TSizeTy& StartCol,
        const TSizeTy& EndCol, TVVec<TNum<TType>, TSizeTy, ColMajor>& SubMat) {
    EAssert(0 <= StartRow && 0 <= StartCol);
    EAssert(EndRow <= Mat.GetRows() && EndCol <= Mat.GetCols());

    if (SubMat.GetRows() != EndRow - StartRow || SubMat.GetCols() != EndCol - StartCol) {
        SubMat.Gen(EndRow - StartRow, EndCol - StartCol);
    }

    for (TSizeTy RowN = StartRow; RowN < EndRow; RowN++) {
        for (TSizeTy ColN = StartCol; ColN < EndCol; ColN++) {
            SubMat.PutXY(RowN - StartRow, ColN - StartCol, Mat(RowN,ColN));
        }
    }
}

template <class TType, class TVecVal, class TSizeTy, bool ColMajor>
void TLinAlg::SubMat(const TVVec<TType, TSizeTy, ColMajor>& Mat, const TVec<TVecVal, TSizeTy>& ColIdxV,
        TVVec<TType, TSizeTy, ColMajor>& SubMat) {

    if (SubMat.Empty()) { SubMat.Gen(Mat.GetRows(), ColIdxV.Len()); }
    EAssert(SubMat.GetRows() == Mat.GetRows() && SubMat.GetCols() == ColIdxV.Len());

    TVec<TType, TSizeTy> ColV;
    for (TSizeTy i = 0; i < ColIdxV.Len(); i++) {
        const TSizeTy& ColN = ColIdxV[i];
        EAssert(0 <= ColN && ColN < Mat.GetCols());
        Mat.GetCol(ColN, ColV);
        SubMat.SetCol(i, ColV);
    }
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Trace(const TVVec<TNum<TType>, TSizeTy, ColMajor>& Mat) {
    EAssert(Mat.GetRows() == Mat.GetCols());
    TType sum = 0.0;
    for (TSizeTy i = 0; i < Mat.GetRows(); i++) {
        sum += Mat.At(i, i);
    }
    return sum;
}

template <class TType, class TSizeTy, bool ColMajor>
TVVec<TNum<TType>, TSizeTy, ColMajor> operator -(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y) {
    TVVec<TNum<TType>, TSizeTy, ColMajor> Z(X.GetRows(), X.GetCols());
    TLinAlg::LinComb(1.0, X, -1.0, Y, Z);
    return Z;
}

template <class TType, class TSizeTy, bool ColMajor>
TVec<TNum<TType>, TSizeTy> operator -(const TVVec<TNum<TType>, TSizeTy>& X,
        const TVVec<TNum<TType>, TSizeTy>& Y) {
    TVVec<TNum<TType>, TSizeTy> Z(X.Len(), X.Len());
    TLinAlg::AddVec(-1.0, Y, X, Z);
    return Z;
}

template <class TType, class TSizeTy, bool ColMajor>
TVVec<TNum<TType>, TSizeTy, ColMajor> operator *(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const TVVec<TNum<TType>, TSizeTy, ColMajor>& Y) {
    TVVec<TNum<TType>, TSizeTy, ColMajor> Z(X.GetRows(), Y.GetCols());
    TLinAlg::Multiply(X, Y, Z);
    return Z;
}

template <class TType, class TSizeTy, bool ColMajor>
TType operator *(const TVec<TNum<TType>, TSizeTy>& Vec1, const TVec<TNum<TType>, TSizeTy>& Vec2) {
    return TLinAlg::DotProduct(Vec1, Vec2);
}

template <class TType, class TSizeTy, bool ColMajor>
TVVec<TNum<TType>, TSizeTy, ColMajor> operator *(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const double& k) {
    TVVec<TNum<TType>, TSizeTy, ColMajor> Y(X.GetRows(), X.GetCols());
    TLinAlg::MultiplyScalar(k, X, Y);
    return Y;
}

template <class TType, class TSizeTy, bool ColMajor>
TVVec<TNum<TType>, TSizeTy, ColMajor>& operator *=(TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const double& k) {
    TLinAlg::MultiplyScalar(k, X, X);
    return X;
}

template <class TType, class TSizeTy, bool ColMajor>
TVec<TNum<TType>, TSizeTy>& operator *=(TVec<TNum<TType>, TSizeTy>& Vec, const double& k) {
    TLinAlg::MultiplyScalar(k, Vec, Vec);
    return Vec;
}

template <class TType, class TSizeTy, bool ColMajor>
TVVec<TNum<TType>, TSizeTy, ColMajor> operator /(const TVVec<TNum<TType>, TSizeTy, ColMajor>& X,
        const double& k) {
    TVVec<TNum<TType>, TSizeTy, ColMajor> Y(X.GetRows(), X.GetCols());
    TLinAlg::MultiplyScalar(1 / k, X, Y);
    return Y;
}

template <class TType, class TSizeTy, bool ColMajor>
TVec<TNum<TType>, TSizeTy> operator /(const TVec<TNum<TType>, TSizeTy>& Vec,
        const double& k) {
    TVec<TNum<TType>, TSizeTy> Res(Vec.Len(), Vec.Len());
    TLinAlg::MultiplyScalar(1 / k, Vec, Res);
    return Res;
}

/////////////////////////////////////////////////////////////////////////
//// Full-Matrix
template <class TIdxV1, class TIdxV2>
TFullMatrix TFullMatrix::operator ()(const TIdxV1& RowV, const TIdxV2& ColV) const {
    const int Rows = RowV.Len();
    const int Cols = ColV.Len();

    TFullMatrix Result(Rows, Cols);
    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            const int Idx1 = (int) RowV[i];
            const int Idx2 = (int) ColV[j];
            const TFlt Val = Mat->At(Idx1, Idx2);
            Result.Mat->PutXY(i, j, Val);
        }
    }

    return Result;
}

template <class TIdxV>
TVector TFullMatrix::operator ()(const int& RowIdx, const TIdxV& ColIdxV) const {
    EAssertR(RowIdx < GetRows(), TStr::Fmt("Invalid row index: %d", RowIdx));

    const int Cols = ColIdxV.Len();

    TVector Result(Cols, false);
    for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
        Result[ColIdx] = At(RowIdx, ColIdx);
    }

    return Result;
}

template <typename TFunc>
TFullMatrix& TFullMatrix::Map(const TFunc& Func) {
    const int& Rows = GetRows();
    const int& Cols = GetCols();

    for (int i = 0; i < Rows; i++) {
        for (int j = 0; j < Cols; j++) {
            Mat->At(i, j) = Func(Mat->At(i, j));
        }
    }

    return *this;
}

#endif /* LINALG_HPP */

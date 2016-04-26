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
/// Transformations of linear algebra structures
template <class TVal, class TSizeTy>
void TLinAlgTransform::OnesV(const int& Dim, TVec<TVal, TSizeTy>& OnesV) {
	if (OnesV.Len() != Dim) { OnesV.Gen(Dim); }

	for (int i = 0; i < Dim; i++) {
		OnesV[i] = 1;
	}
}

template <class TVal, class TSizeTy>
void TLinAlgTransform::RangeV(const TSizeTy& Dim, TVec<TVal, TSizeTy>& RangeV) {
	const TSizeTy Mn = 0;
	const TSizeTy Mx = Dim-1;

	TLinAlgTransform::RangeV(Mn, Mx, RangeV);
}

template <class TVal, class TSizeTy>
void TLinAlgTransform::RangeV(const TSizeTy& Min, const TSizeTy& Max, TVec<TVal, TSizeTy>& Res) {
	const TSizeTy Len = Max - Min + 1;

	if (Res.Len() != Len) { Res.Gen(Len); }

	Res.Gen(Len, 0);
	for (TSizeTy i = Min; i <= Max; i++) {
		Res[i - Min] = TVal(i);
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Diag(const TVec<TType, TSizeTy>& DiagV, TVVec<TType, TSizeTy, ColMajor>& D) {
	const TSizeTy Dim = DiagV.Len();

	if (D.Empty()) { D.Gen(Dim, Dim); }
	EAssert(D.GetRows() == Dim && D.GetCols() == Dim);

	for (TSizeTy i = 0; i < Dim; i++) {
		D(i,i) = DiagV[i];
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::Identity(const TSizeTy& Dim, TVVec<TType, TSizeTy, ColMajor>& X) {
	if (X.Empty()) { X.Gen(Dim, Dim); }
	EAssert(X.GetRows() == Dim && X.GetCols() == Dim);

	for (TSizeTy i = 0; i < Dim; i++) {
		X(i,i) = 1;
	}
}

template <>
inline void TLinAlgTransform::Sqrt(TFltVV& X) {
	double Val;
	for (int RowN = 0; RowN < X.GetRows(); RowN++) {
		for (int ColN = 0; ColN < X.GetCols(); ColN++) {
			Val = X(RowN, ColN);
			AssertR(Val > -1e-8, "Distance lower than numerical error!");
			if (Val < 0) { Val = 0; }
			X.PutXY(RowN, ColN, TMath::Sqrt(Val));
		}
	}
}

template <>
inline void TLinAlgTransform::Sqrt(TVec<TIntFltKdV>& X) {
	double Val;
	for (int ColN = 0; ColN < X.Len(); ColN++) {
		TIntFltKdV& ColSpV = X[ColN];
		for (int RowN = 0; RowN < ColSpV.Len(); RowN++) {
			Val = ColSpV[RowN].Dat;
			AssertR(Val > -1e-8, "Distance lower than numerical error!");
			if (Val < 0) { Val = 0; }
			ColSpV[RowN].Dat = TMath::Sqrt(Val);
		}
	}
}

template <>
inline void TLinAlgTransform::Sqr(TFltVV& X) {
    double Val;
    for (int RowN = 0; RowN < X.GetRows(); RowN++) {
        for (int ColN = 0; ColN < X.GetCols(); ColN++) {
            Val = X(RowN, ColN);
            AssertR(Val > -1e-8, "Distance lower than numerical error!");
            if (Val < 0) { Val = 0; }
            X.PutXY(RowN, ColN, TMath::Sqr(Val));
        }
    }
}

template <>
inline void TLinAlgTransform::Sqr(TVec<TIntFltKdV>& X) {
    double Val;
    for (int ColN = 0; ColN < X.Len(); ColN++) {
        TIntFltKdV& ColSpV = X[ColN];
        for (int RowN = 0; RowN < ColSpV.Len(); RowN++) {
            Val = ColSpV[RowN].Dat;
            AssertR(Val > -1e-8, "Distance lower than numerical error!");
            if (Val < 0) { Val = 0; }
            ColSpV[RowN].Dat = TMath::Sqr(Val);
        }
    }
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlgTransform::CenterRows(TVVec<TType, TSizeTy, ColMajor>& X) {
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

// Dense to sparse transform
//TODO fix TVec<TIntFltKdV> indexing and type
template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlgTransform::Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TIntFltKdV>& B){
	TSizeTy Cols = A.GetCols();
	TSizeTy Rows = A.GetRows();
	B.Gen(Cols);
	for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
		B[ColN].Gen(0);
		for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
			if (A.At(RowN, ColN) != 0.0) {
				B[ColN].Add(TIntFltKd(RowN, A.At(RowN, ColN)));
			}
		}
	}
}

// TEST
// Sparse to dense transform
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
template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlgTransform::Full(const TVec<TIntFltKdV, TSizeTy>& A, TVVec<TType, TSizeTy, ColMajor>& B, TSizeTy Rows){
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
/// Search elements of matrices and vectors
template <class TVal, class TSizeTy>
TSizeTy TLinAlgSearch::GetMaxIdx(const TVec<TVal, TSizeTy>& Vec) {
	if (Vec.Empty()) { return -1; }

	TSizeTy MxIdx = 0;
	TVal MxVal = Vec[0];
	for (TSizeTy i = 1; i < Vec.Len(); i++ ) {
		if (Vec[i] > MxVal) {
			MxVal = Vec[i];
			MxIdx = i;
		}
	}

	return MxIdx;
}


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


///////////////////////////////////////////////////////////////////////
// Basic Linear Algebra operations
template <class TType, class TSizeTy>
double TLinAlg::DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
	EAssertR(x.Len() == y.Len(), TStr::Fmt("%d != %d", x.Len(), y.Len()));
	TType result = 0.0; const  TSizeTy Len = x.Len();
	for (TSizeTy i = 0; i < Len; i++)
		result += x[i] * y[i];
	return result;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
	int ColIdX, const TVVec<TType, TSizeTy, ColMajor>& Y, int ColIdY) {
	EAssert(X.GetRows() == Y.GetRows());
	TType result = 0.0; const TSizeTy len = X.GetRows();
	for (TSizeTy i = 0; i < len; i++)
		result = result + X(i, ColIdX) * Y(i, ColIdY);
	return result;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
	int ColId, const TVec<TType, TSizeTy>& Vec) {

	EAssert(X.GetRows() == Vec.Len());
	TType result = 0.0; const TSizeTy len = X.GetRows();
	for (TSizeTy i = 0; i < len; i++)
		result += X(i, ColId) * Vec[i];
	return result;
}

template <class TType, class TSizeTy>
double TLinAlg::DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TIntFltKd>& y) {
	double Res = 0.0; const int xLen = x.Len(), yLen = y.Len();
	for (TSizeTy i = 0; i < yLen; i++) {
		const TSizeTy key = y[i].Key;
		if (key < xLen) Res += y[i].Dat * x[key];
	}
	return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, const TIntFltKdV& y) {
	TType Res = 0.0; const TSizeTy n = X.GetRows(), yLen = y.Len();
	for (TSizeTy i = 0; i < yLen; i++) {
		const TSizeTy key = y[i].Key;
		if (key < n) Res += y[i].Dat * X(key, ColId);
	}
	return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X, const TVVec<TType, TSizeTy, ColMajor>& Y) {
	EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());
	double Rows = X.GetRows();
	double Cols = X.GetCols();
	double Res = 0.0;
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			Res += X(RowN, ColN) * Y(RowN, ColN);
		}
	}
	return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::DotProduct(const TVec<TIntFltKdV>& X, const TVVec<TType, TSizeTy, ColMajor>& Y) {
	int Rows = TLinAlgSearch::GetMaxDimIdx(X) + 1;
	EAssert(Rows <= Y.GetRows() && X.Len() == Y.GetCols());
	double Cols = X.Len();
	double Res = 0.0;
	for (int ColN = 0; ColN < Cols; ColN++) {
		const TIntFltKdV& ColX = X[ColN];
		const int Els = ColX.Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			Res += ColX[ElN].Dat * Y(ColX[ElN].Key, ColN);
		}
	}
	return Res;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::OuterProduct(const TVec<TType, TSizeTy>& x,
	const TVec<TType, TSizeTy>& y, TVVec<TType, TSizeTy, ColMajor>& Z) {

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
void TLinAlg::HadamardProd(const TVVec<TType, TSizeTy, ColMajor>& X, const TVVec<TType, TSizeTy, ColMajor>& Y,
	TVVec<TType, TSizeTy, ColMajor>& Z) {
	
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
void TLinAlg::HadamardProd(const TVec<TIntFltKdV>& X, const TVVec<TType, TSizeTy, ColMajor>& Y,
	TVVec<TType, TSizeTy, ColMajor>& Z) {
	
	int Rows = TLinAlgSearch::GetMaxDimIdx(X) + 1;
	EAssert(Rows <= Y.GetRows() && X.Len() == Y.GetCols());
	if (Z.Empty()) {
		Z.Gen(Y.GetRows(), Y.GetCols());
	} else {
		EAssert(Y.GetRows() == Z.GetRows() && Y.GetCols() == Z.GetCols());
	}
	int Cols = X.Len();
	TLinAlgTransform::FillZero(Z);
	for (int ColN = 0; ColN < Cols; ColN++) {
		const TIntFltKdV& ColX = X[ColN]; 
		const int Els = ColX.Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			Z(ColX[ElN].Key, ColN) = ColX[ElN].Dat * Y(ColX[ElN].Key, ColN);
		}
	}
}

// z := p * x + q * y
//TODO should double be TType?
template <class TType, class TSizeTy>
void TLinAlg::LinComb(const double& p, const TVec<TType, TSizeTy>& x,
	const double& q, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z) {
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
// TEST
// Z := p * X + q * Y
//TODO double or type?
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::LinComb(const double& p, const TVVec<TType, TSizeTy, ColMajor>& X,
	const double& q, const TVVec<TType, TSizeTy, ColMajor>& Y, TVVec<TType, TSizeTy, ColMajor>& Z) {
	EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols() && X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
	TSizeTy Rows = X.GetRows();
	TSizeTy Cols = X.GetCols();
	for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
		for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
			Z.At(RowN, ColN) = p*X.At(RowN, ColN) + q*Y.At(RowN, ColN);
		}
	}
}

// TEST
// z := p * x + (1 - p) * y
template <class TType, class TSizeTy>
void TLinAlg::ConvexComb(const double& p, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z)  {
	AssertR(0.0 <= p && p <= 1.0, TFlt::GetStr(p));
	TLinAlg::LinComb(p, x, 1.0 - p, y, z);
}

#ifdef BLAS

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const TType& k, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y) {
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
template <class TType, class TSizeTy>
void TLinAlg::AddVec(const double& k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z)  {
	TLinAlg::LinComb(k, x, 1.0, y, z);
}

// z := x + y
template <class TType, class TSizeTy>
void TLinAlg::AddVec(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z) {
	TLinAlg::LinComb(1.0, x, 1.0, y, z);
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, TSizeTy ColIdX, TVVec<TType, TSizeTy, ColMajor>& Y, TSizeTy ColIdY) {
	EAssert(X.GetRows() == Y.GetRows());
	const TSizeTy len = Y.GetRows();
	for (TSizeTy i = 0; i < len; i++) {
		Y(i, ColIdY) = Y(i, ColIdY) + k * X(i, ColIdX);
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(const double& k, const TVec<TType, TSizeTy>& x, TVVec<TType, TSizeTy, ColMajor>& Y, const TSizeTy& ColIdY) {
	EAssert(x.Len() == Y.GetRows());
	EAssert(ColIdY >= 0 && ColIdY < x.Len());
	for (TSizeTy RowN = 0; RowN < Y.GetRows(); RowN++) {
		Y.At(RowN, ColIdY) += k*x[RowN];
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, TVec<TType, TSizeTy>& Result) {
	EAssert(X.GetRows() == Result.Len());
	const TSizeTy len = Result.Len();
	for (TSizeTy i = 0; i < len; i++) {
		Result[i] = Result[i] + k * X(i, ColId);
	}
}

template <class TType, class TSizeTy>
double TLinAlg::SumVec(const TVec<TType, TSizeTy>& x) {
	const TSizeTy len = x.Len();
	double Res = 0.0;
	for (int i = 0; i < len; i++) {
		Res += x[i];
	}
	return Res;
}

template <class TType, class TSizeTy>
double TLinAlg::SumVec(double k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
	EAssert(x.Len() == y.Len());
	const TSizeTy len = x.Len();
	double Res = 0.0;
	for (TSizeTy i = 0; i < len; i++) {
		Res += k * x[i] + y[i];
	}
	return Res;
}

template <class TType, class TSizeTy>
double TLinAlg::EuclDist2(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
	EAssert(x.Len() == y.Len());
	const TSizeTy len = x.Len();
	double Res = 0.0;
	for (TSizeTy i = 0; i < len; i++) {
		Res += TMath::Sqr(x[i] - y[i]);
	}
	return Res;
}

template <class TSizeTy, bool ColMajor>
double TLinAlg::EuclDist2(const TVec<TIntFltKd, TSizeTy>& x, const TVec<TIntFltKd, TSizeTy>& y) {
	double Res = TLinAlg::Norm2(x) - 2 * TLinAlg::DotProduct(x, y) + TLinAlg::Norm2(y);
	return Res;
}

template <class TType, class TSizeTy>
double TLinAlg::EuclDist(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
	return sqrt(TLinAlg::EuclDist2(x, y));
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob(const TVVec<TNum<TType>, TSizeTy, ColMajor> &A) {
	TType frob = 0;
	for (int RowN = 0; RowN < A.GetRows(); RowN++) {
		for (int ColN = 0; ColN < A.GetCols(); ColN++) {
			frob += A.At(RowN, ColN)*A.At(RowN, ColN);
		}
	}
	return sqrt(frob);
}

template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::Frob2(const TVVec<TType, TSizeTy, ColMajor> &A) {
	TType frob = 0;
	for (int RowN = 0; RowN < A.GetRows(); RowN++) {
		for (int ColN = 0; ColN < A.GetCols(); ColN++) {
			frob += A.At(RowN, ColN)*A.At(RowN, ColN);
		}
	}
	return frob;
}

template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::FrobDist2(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B) {
	double frob = 0;
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

template <class TType, class TSizeTy>
double TLinAlg::FrobDist2(const TVec<TType, TSizeTy>& A, const TVec<TType, TSizeTy>& B) {
	double frob = 0;
	frob = TLinAlg::EuclDist2(A, B);
	/*for (int RowN = 0; RowN < A.Len(); RowN++) {
		frob += (A[RowN] - B[RowN])*(A[RowN] - B[RowN]);
	}*/
	return frob;
}

template <class TType, class TSizeTy>
double TLinAlg::Norm2(const TVec<TType, TSizeTy>& x) {
	return TLinAlg::DotProduct(x, x);
}

template <class TType, class TSizeTy>
double TLinAlg::Norm(const TVec<TType, TSizeTy>& x) {
	return sqrt(TLinAlg::Norm2(x));
}

template <class TType, class TSizeTy>
double TLinAlg::Normalize(TVec<TType, TSizeTy>& x) {
	const double xNorm = TLinAlg::Norm(x);
	if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
	return xNorm;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumn(TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& ColId) {
	double nX = TLinAlg::Norm(X, ColId);
	if (nX > 0.0) {
		for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
			X.At(RowN, ColId) /= nX;
		}
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVVec<TType, TSizeTy, ColMajor>& X) {
	for (TSizeTy ColN = 0; ColN < X.GetCols(); ColN++) {
		TLinAlg::NormalizeColumn(X, ColN);
	}
}
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeRows(TVVec<TType, TSizeTy, ColMajor>& X) {
	for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
		TVec<TType> Row;
		X.GetRowPtr(RowN, Row);
		Normalize(Row);
	}
}

#ifdef INTEL
// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::NormalizeColumns(TVVec<TType, TSizeTy, ColMajor>& X, TBool ColumnMajor) {
	const TSizeTy m = X.GetXDim();
	const TSizeTy n = X.GetYDim();
	TVVec<TType, TSizeTy, ColMajor> sqrX(m, n);
	vdSqr(m*n, &X(0, 0).Val, &sqrX(0, 0).Val);
	printf("Squaring of elements done!\n");
	TVec<TType, TSizeTy> enke(m); TVec<TType, TSizeTy> sumsqr(n);  TVec<TType, TSizeTy> norme(n); TLAMisc::Fill(enke, 1.0);
	TLinAlg::MultiplyT(sqrX, enke, sumsqr);
	printf("Summing elemnents done!\n");
	vdInvSqrt(n, &sumsqr[0].Val, &norme[0].Val);
	printf("Summing and inverting elemnents done!\n");
	// added code
	if (ColMajor) {
		TVVec<TType, TSizeTy, ColMajor> B; B.Gen(n, m);
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
#endif

// Normalize the columns of X
//TODO what to do when number
//MARK
template <class TType, class TSizeTy, bool ColMajor, class IndexType>
void TLinAlg::NormalizeColumns(TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X) {
	if (X.Val2.Len() == 0) return;
	EAssert(X.Val2.IsSorted(true));
	//int?
	int Cols = X.Val2.GetMxVal() + 1;
	TVec<TType, TSizeTy> InvColNorms(Cols); //get the last element colN and set the number of elements

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
template<class TSizeTy>
void TLinAlg::NormalizeColumns(TVec<TIntFltKdV, TSizeTy>& X) {
	TSizeTy Cols = X.Len();
	for (TSizeTy ElN = 0; ElN < Cols; ElN++) {
		TLinAlg::Normalize(X[ElN]);
	}
}
// Frobenius norm of matrix A
// TEST
template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::FrobNorm2(const TVVec<TType, TSizeTy, ColMajor>& X) {
	return TLinAlg::Norm2((const_cast<TVVec<TType, TSizeTy, ColMajor> &>(X)).Get1DVec());
}
template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::FrobNorm(const TVVec<TType, TSizeTy, ColMajor>& X) {
	return sqrt(TLinAlg::FrobNorm2(X));
}

// ||x||^2 (Euclidian), x is sparse
template<class TSizeTy>
double TLinAlg::Norm2(const TVec<TIntFltKdV, TSizeTy>& x) {
	double Result = 0;
	for (TSizeTy i = 0; i < x.Len(); i++) {
		Result += TMath::Sqr(x[i].Dat);
	}
	return Result;
}

// ||x|| (Euclidian), x is sparse
template<class TSizeTy>
double TLinAlg::Norm(const TVec<TIntFltKdV, TSizeTy>& x) {
	return sqrt(Norm2(x));
}

template<class TSizeTy>
double TLinAlg::Norm(const TVec<TIntFltKdV, TSizeTy>& x, const int& ColId) {
	return Norm(x[ColId]);
}

// x := x / ||x||, x is sparse
template<class TSizeTy, TSizeTy>
void TLinAlg::Normalize(TVec<TIntFltKdV>& x) {
	double Normx = TLinAlg::Norm(x);
	if (Normx > 0) {
		TLinAlg::MultiplyScalar(1 / Normx, x, x);
	}
}

// ||X(:,ColId)||^2 (Euclidian)
template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::Norm2(const TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& ColId) {
	return TLinAlg::DotProduct(X, ColId, X, ColId);
}

// TEST
// ||X(:,ColId)|| (Euclidian)
template <class TType, class TSizeTy, bool ColMajor>
double TLinAlg::Norm(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId) {
	return sqrt(TLinAlg::Norm2(X, ColId));
}

// L1 norm of x (Sum[|xi|, i = 1..n])
template <class TType, class TSizeTy>
double TLinAlg::NormL1(const TVec<TType, TSizeTy>& x)  {
	double norm = 0.0; const TSizeTy Len = x.Len();
	for (TSizeTy i = 0; i < Len; i++)
		norm += TFlt::Abs(x[i]);
	return norm;
}

template <class TType, class TSizeTy>
double TLinAlg::NormL1(double k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
	EAssert(x.Len() == y.Len());
	double norm = 0.0; const TSizeTy len = x.Len();
	for (TSizeTy i = 0; i < len; i++) {
		norm += TFlt::Abs(k * x[i] + y[i]);
	}
	return norm;
}

// TEST
// x := x / ||x||_1
template <class TType, class TSizeTy>
void TLinAlg::NormalizeL1(TVec<TType, TSizeTy>& x) {
	const double xNorm = TLinAlg::NormL1(x);
	if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
}

// x := x / ||x||_1
void TLinAlg::NormalizeL1(TIntFltKdV& x) {
	const double xNorm = TLinAlg::NormL1(x);
	if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
}

// TEST
// Linf norm of x (Max{|xi|, i = 1..n})
template <class TType, class TSizeTy>
double TLinAlg::NormLinf(const TVec<TType, TSizeTy>& x) {
	double norm = 0.0; const TSizeTy Len = x.Len();
	for (TSizeTy i = 0; i < Len; i++)
		norm = TFlt::GetMx(TFlt::Abs(x[i]), norm);
	return norm;
}

// TEST
// x := x / ||x||_inf
template <class TType, class TSizeTy>
void TLinAlg::NormalizeLinf(TVec<TType, TSizeTy>& x) {
	const double xNormLinf = TLinAlg::NormLinf(x);
	if (xNormLinf > 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLinf, x, x); }
}

// x := x / ||x||_inf, , x is sparse

void TLinAlg::NormalizeLinf(TIntFltKdV& x) {
	const double xNormLInf = TLinAlg::NormLinf(x);
	if (xNormLInf > 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLInf, x, x); }
}

	// stores the squared norm of all the columns into the output vector
	void TLinAlg::GetColNormV(const TFltVV& X, TFltV& ColNormV) {
		const int Cols = X.GetCols();
		GetColNorm2V(X, ColNormV);
		for (int i = 0; i < Cols; i++) {
			ColNormV[i] = sqrt(ColNormV[i]);
		}
	}

	// stores the norm of all the columns into the output vector
void TLinAlg::GetColNormV(const TVec<TIntFltKdV>& X, TFltV& ColNormV) {
    const int Cols = X.Len();
    GetColNorm2V(X, ColNormV);
    for (int i = 0; i < Cols; i++) {
        ColNormV[i] = sqrt(ColNormV[i]);
    }
}

// stores the squared norm of all the columns into the output vector
	void TLinAlg::GetColNorm2V(const TFltVV& X, TFltV& ColNormV) {
		const int Cols = X.GetCols();

		if (ColNormV.Len() != Cols) { ColNormV.Gen(Cols); }

		for (int ColN = 0; ColN < Cols; ColN++) {
			ColNormV[ColN] = Norm2(X, ColN);
		}
	}

	// stores the norm of all the columns into the output vector
void TLinAlg::GetColNorm2V(const TVec<TIntFltKdV>& SpVV, TFltV& ColNormV) {
	const int Cols = SpVV.Len();

	if (ColNormV.Len() != Cols) { ColNormV.Gen(Cols); }

	for (int ColN = 0; ColN < Cols; ColN++) {
		ColNormV[ColN] = Norm2(SpVV[ColN]);
	}
}

// TEST
// sum columns (Dimension = 1) or rows (Dimension = 2) and store them in vector y
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Sum(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TType, TSizeTy>& y, const int Dimension){
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
double TLinAlg::SumRow(const TVVec<TType, TSizeTy, ColMajor>& X, const int& RowN) {
	EAssertR(RowN < X.GetRows(), "Row index exceeds the number of rows!");
	const int Cols = X.GetCols();

	double Sum = 0;
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
int TLinAlg::GetRowMaxIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& RowN) {
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
int TLinAlg::GetRowMinIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& RowN) {
	TSizeTy Idx = -1;
	TSizeTy Cols = X.GetCols();
	double MnVal = TFlt::Mx;
	for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
		const double Val = X.At(RowN, ColN);
		if (Val < MnVal) {
			MnVal = Val;
			Idx = ColN;
		}
	}
	return Idx;
}

// TEST
// find the index of maximum elements for a given each col of X
template <class TType, class TSizeTy, bool ColMajor>
int TLinAlg::GetColMaxIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const int& ColN) {
	TSizeTy Idx = -1;
	TSizeTy Rows = X.GetRows();
	double MaxVal = TFlt::Mn;
	for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
		double Val = X.At(RowN, ColN);
		if (MaxVal < Val) {
			MaxVal = Val;
			Idx = RowN;
		}
	}
	return Idx;
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetRowMaxIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV) {
	IdxV.Gen(X.GetRows());
	TSizeTy Rows = X.GetRows();
	for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
		IdxV[RowN] = TLinAlg::GetRowMaxIdx(X, RowN);
	}
}

template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetRowMinIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV) {
	const TSizeTy Rows = X.GetRows();

	if (IdxV.Len() != Rows) { IdxV.Gen(Rows); }

	for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
		IdxV[RowN] = TLinAlg::GetRowMinIdx(X, RowN);
	}
}

// find the index of maximum elements for each col of X
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::GetColMaxIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV) {
	IdxV.Gen(X.GetCols());
	TSizeTy Cols = X.GetCols();
	for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
		IdxV[ColN] = TLinAlg::GetColMaxIdx(X, ColN);
	}
}
//x := k * x
// TEST
//x := k * x
template <class TType, class TSizeTy>
void TLinAlg::MultiplyScalar(const double& k, TVec<TType, TSizeTy>& x) {
	TSizeTy Len = x.Len();
	for (TSizeTy i = 0; i < Len; i++)
		x[i] = k * x[i];
}

// find the index of maximum elements for each col of X
void TLinAlg::GetColMinIdxV(const TFltVV& X, TIntV& IdxV) {
	int Cols = X.GetCols();

	if (IdxV.Empty()) { IdxV.Gen(Cols); }
	EAssert(IdxV.Len() == Cols);

	for (int ColN = 0; ColN < Cols; ColN++) {
		IdxV[ColN] = GetColMinIdx(X, ColN);
	}
}

//template <class TVal> TVal TLinAlg::GetColMin(const TVVec<TVal>& X, const int& ColN);
//template <class TVal> void TLinAlg::GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV);
// TEST
// y := k * x
template <class TType, class TSizeTy>
void TLinAlg::MultiplyScalar(const double& k, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y) {
	EAssert(x.Len() == y.Len());
	TSizeTy Len = x.Len();
	for (TSizeTy i = 0; i < Len; i++)
		y[i] = k * x[i];
}
// y := k * x
void TLinAlg::MultiplyScalar(const double& k, const TIntFltKdV& x, TIntFltKdV& y) {
	EAssert(x.Len() == y.Len());
	int Len = x.Len();
	for (int i = 0; i < Len; i++) {
		y[i].Key = x[i].Key;
		y[i].Dat = k * x[i].Dat;
	}
}
// TEST
// Y := k * X
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyScalar(const double& k, const TVVec<TType, TSizeTy, ColMajor>& X, TVVec<TType, TSizeTy, ColMajor>& Y) {
	EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());
	const TSizeTy Rows = X.GetRows();
	const TSizeTy Cols = X.GetCols();
	for (TSizeTy i = 0; i < Rows; i++) {
		for (TSizeTy j = 0; j < Cols; j++) {
			Y(i, j) = k*X(i, j);
		}
	}
}
// Y := k * X
template <class TSizeTy>
void TLinAlg::MultiplyScalar(const double& k, const TVec<TIntFltKdV, TSizeTy>& X, TVec<TIntFltKdV, TSizeTy>& Y) {
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
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y) {
	if (y.Empty()) { y.Gen(A.GetRows()); }
	EAssert(A.GetCols() == x.Len() && A.GetRows() == y.Len());
#ifdef BLAS
	TLinAlg::Multiply(A, x, y, TLinAlgBlasTranspose::NOTRANS, 1.0, 0.0);
#else
	int n = A.GetRows(), m = A.GetCols();
	for (int i = 0; i < n; i++) {
		y[i] = 0.0;
		for (int j = 0; j < m; j++) {
			y[i] += A(i, j) * x[j];
        }
	}
#endif
}

// TEST
// C(:, ColId) := A * x
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVVec<TType, TSizeTy, ColMajor>& C, TSizeTy ColId) {
	EAssert(A.GetCols() == x.Len() && A.GetRows() == C.GetRows());
	TSizeTy n = A.GetRows(), m = A.GetCols();
	for (TSizeTy i = 0; i < n; i++) {
		C(i, ColId) = 0.0;
		for (TSizeTy j = 0; j < m; j++)
			C(i, ColId) += A(i, j) * x[j];
	}
}

// TEST
// y := A * B(:, ColId)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, int ColId, TVec<TType, TSizeTy>& y)  {
	EAssert(A.GetCols() == B.GetRows() && A.GetRows() == y.Len());
	TSizeTy n = A.GetRows(), m = A.GetCols();
	for (TSizeTy i = 0; i < n; i++) {
		y[i] = 0.0;
		for (TSizeTy j = 0; j < m; j++)
			y[i] += A(i, j) * B(j, ColId);
	}
}

// TEST
// C(:, ColIdC) := A * B(:, ColIdB)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, int ColIdB, TVVec<TType, TSizeTy, ColMajor>& C, int ColIdC) {
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
// Tested in other function
//A is rewritten in place with orthogonal matrix Q
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(TVVec<TType, TSizeTy, ColMajor>& A) {
	TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
	TSizeTy lda = ColMajor ? m : n;
	int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
	TVec<TType, TSizeTy> tau; tau.Gen(MAX(1, MIN(m, n)));
	LAPACKE_dgeqrf(Matrix_Layout, m, n, &A(0, 0).Val, lda, &tau[0].Val);
	LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q) {
	Q = A;
	TLinAlg::QRbasis(Q);
}

// Tested in other function
//A is rewritten in place with orthogonal matrix Q (column pivoting to improve stability)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(TVVec<TType, TSizeTy, ColMajor>& A) {
	TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
	TSizeTy lda = ColMajor ? m : n;
	TSizeTy Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
	TVec<TType, TSizeTy> tau(MAX(1, MIN(m, n)));
	TVec<TInt, TSizeTy> jvpt(MAX(1, n));
	LAPACKE_dgeqp3(Matrix_Layout, m, n, &A(0, 0).Val, lda, &jvpt[0].Val, &tau[0].Val);
	LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
}

// TEST
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::QRcolpbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q) {
	Q = A;
	TLinAlg::QRcolpbasis(Q);
}

// TEST
//S S option ensures that A is not modified
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::thinSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& U, TVec<TType, TSizeTy>& S, TVVec<TType, TSizeTy, ColMajor>& VT) {

	TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols();
	TSizeTy thin_dim = MIN(m, n);

	S.Gen(thin_dim); U.Gen(m, thin_dim); VT.Gen(thin_dim, n);

	int lda = ColMajor ? m : n;
	int ldu = ColMajor ? m : thin_dim;
	int ldvt = ColMajor ? thin_dim : n;

	TVec<TType, TSizeTy> superb(MAX(1, MIN(m, n)));
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
//	matdescra[6] ={'G', 'G', 'N', 'C', 'Q', 'Q'}; //General, General, Nonunit diagonal, Zero Based indexing

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
	TSizeTy LeadingDimB = ColMajor ? B.GetRows() : B.GetCols();	// TODO is this correct???
	TSizeTy LeadingDimV = ColMajor ? V.GetRows() : V.GetCols();
	int Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

	TVVec<TNum<TType>, TSizeTy, ColMajor> A1 = A;	// A is overwritten
	TVVec<TNum<TType>, TSizeTy, ColMajor> B1 = B;	// B is overwritten

	// A will get overwritten
	int Info = LAPACKE_dggev(
		Layout,					// matrix layout
		'N',					// don't compute left eigenvectors
		'V',					// compute right eigenvectors
		Dim,					// order of matrices A,B,Vl,Vr
		(double*) &A1(0,0),		// A
		LeadingDimA,			// leading dimension of A
		(double*) &B1(0,0),		// B
		LeadingDimB,			// leading dimension of B
		(double*) &AlphaR[0],	// real part of the eigenvalues
		(double*) &AlphaI[0],	// imaginary part of the eigenvalues
		(double*) &Beta[0],		// used to compute the eigenvalues
		nullptr,				// left eigenvectors Vl
		1,						// leading dimension of Vl (>= 1)
		(double*) &V(0,0),		// right eigenvectors Vr
		LeadingDimV				// leading dimension of Vr
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
// INTEL
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

// TEST
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
#endif			//printf("Lincomb does not fail\n");
	}
}
// TEST Move to BLAS
// y := A' * x
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y) {
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

// TEST
// C = op(A) * op(B)
template <class TType, class TSizeTy, bool ColMajor>
inline
	void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>, TSizeTy, ColMajor>& C,
	const int& BlasTransposeFlagA, const int& BlasTransposeFlagB) {
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
#endif

#ifdef BLAS
// TEST
// y := alpha*op(A)*x + beta*y, where op(A) = A -- N, op(A) = A' -- T, op(A) = conj(A') -- C (only for complex)
//Andrej ToDo In the future replace TType with TNum<type> and change double to type
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA, TType alpha, TType beta) {
	TSizeTy m = A.GetRows();
	TSizeTy n = A.GetCols();
	//Can we multiply and store in y?
	if (BlasTransposeFlagA) {//A'*x n*m x m -> n
		EAssertR(x.Len() == m, "TLinAlg::Multiply: Invalid dimension of input vector!");
		if (y.Reserved() != n) {	// TODO should I do this here?? Meybe if the length is > n it would also be OK??
			y.Gen(n, n);
		}
	}
	else{//A*x  m x n * n -> m
		EAssertR(x.Len() == n, "TLinAlg::Multiply: Invalid dimension of input vector!");
		if (y.Reserved() != m) {	// TODO should I do this here?? Meybe if the length is > m it would also be OK??
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
#endif
// TEST
// C = A * B
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, TVVec<TType, TSizeTy, ColMajor>& C) {
    if (C.Empty()) { C.Gen(A.GetRows(), B.GetCols()); }

    EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetCols() == B.GetRows());
#ifdef BLAS
	TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::NOTRANS, TLinAlgBlasTranspose::NOTRANS);
#else
	TSizeTy RowsA = A.GetRows();
	TSizeTy ColsA = A.GetCols();
	TSizeTy ColsB = B.GetCols();
	C.PutAll(0.0);
	for (TSizeTy RowN = 0; RowN < RowsA; RowN++) {
		for (TSizeTy ColAN = 0; ColAN < ColsA; ColAN++) {
			double Weight = A(RowN, ColAN);
			for (TSizeTy ColBN = 0; ColBN < ColsB; ColBN++) {
				C(RowN, ColBN) += Weight * B(ColAN, ColBN);
			}
		}
	}
#endif
}


// TEST
// C = A' * B
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, TVVec<TType, TSizeTy, ColMajor>& C) {
	if (C.Empty()) { C.Gen(A.GetCols(), B.GetCols()); }
	EAssert(A.GetCols() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetRows() == B.GetRows());
#ifdef BLAS
	TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::TRANS, TLinAlgBlasTranspose::NOTRANS);
#else
	TSizeTy n = C.GetRows(), m = C.GetCols(), l = A.GetRows(); double sum;
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


//////////////////
//  DENSE-SPARSE, SPARSE-DENSE

// TEST
// C := A * B
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

// TEST
// C:= A' * B
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
// TEST
// C:= A' * B
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
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TVec<TKeyDat<IndexType, TType>, TSizeTy>, TSizeTy>& B, TVVec<TType, TSizeTy, ColMajor>& C) {
	// C = A' B = (B' A)'
#ifdef INTELBETA
	TTriple<TVec<IndexType, TSizeTy>, TVec<TInt, TSizeTy>, TVec<TType, TSizeTy>> BB;
	TLinAlg::Convert(B, BB); // convert the matrix to a coordinate form
	TVVec<TType, TSizeTy, ColMajor> CC(B.Len(), A.GetCols());
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
	int Cols = B.Len();
	int Rows = A.GetCols();
	C.PutAll(0.0);
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			int Els = B[ColN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				C.At(RowN, ColN) += A.At(B[ColN][ElN].Key, RowN) * B[ColN][ElN].Dat;
			}
		}
	}
#endif
}


// C := A * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA) {
	// A = sparse column matrix
	EAssert(A.Len() == B.GetRows());
	int Rows = RowsA;
	int ColsB = B.GetCols();
	if (RowsA == -1) {
		Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1;
	}
	else {
		EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= RowsA);
	}
	if (C.Empty()) {
		C.Gen(Rows, ColsB);
	}
	int RowsB = B.GetRows();
	C.PutAll(0.0);
	for (int ColN = 0; ColN < ColsB; ColN++) {
		for (int RowN = 0; RowN < RowsB; RowN++) {
			int Els = A[RowN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				C.At(A[RowN][ElN].Key, ColN) += A[RowN][ElN].Dat * B.At(RowN, ColN);
			}
		}
	}
}

// C:= A' * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64 TFlt
void TLinAlg::MultiplyT(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C) {
	// A = sparse column matrix
	EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= B.GetRows());
	int ColsB = B.GetCols();
	//int RowsB = B.GetRows();
	int ColsA = A.Len();
	if (C.Empty()) {
		C.Gen(ColsA, ColsB);
	}
	else {
		EAssert(C.GetRows() == ColsA && C.GetCols() == ColsB);
	}
	C.PutAll(0.0);
	for (int RowN = 0; RowN < ColsA; RowN++) {
		for (int ColN = 0; ColN < ColsB; ColN++) {
			int Els = A[RowN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				C.At(RowN, ColN) += A[RowN][ElN].Dat * B.At(A[RowN][ElN].Key, ColN);
			}
		}
	}
}

void TLinAlg::Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TVec<TIntFltKdV>& C) {
    EAssert(A.GetCols() >= TLinAlgSearch::GetMaxDimIdx(B) + 1);
    int Rows = A.GetRows();
    int Cols = B.Len();

    C.Gen(Cols);
    for (int ColN = 0; ColN < Cols; ColN++) {
        for (int RowN = 0; RowN < Rows; RowN++) {
            TFlt val(0.0);
            int Els = B[ColN].Len();
            for (int ElN = 0; ElN < Els; ElN++) {
                val += A(RowN, B[ColN][ElN].Key) * B[ColN][ElN].Dat;
            }
            C[ColN].Add(TIntFltKd(RowN, val));
        }
    }
}

// SPARSECOLMAT-SPARSECOLMAT
// C := A * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
//TLAMisc
//GetMaxDimIdx
void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA) {
	//// A,B = sparse column matrix
	//EAssert(A.Len() == B.GetRows());
	int Rows = RowsA;
	int ColsB = B.Len();
	if (RowsA == -1) {
		Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1;
	}
	else {
		EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= RowsA);
	}
	if (C.Empty()) {
		C.Gen(Rows, ColsB);
	}
	EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.Len());
	C.PutAll(0.0);
	for (int ColN = 0; ColN < ColsB; ColN++) {
		int ElsB = B[ColN].Len();
		for (int ElBN = 0; ElBN < ElsB; ElBN++) {
			int IdxB = B[ColN][ElBN].Key;
			double ValB = B[ColN][ElBN].Dat;
			int ElsA = A[IdxB].Len();
			for (int ElAN = 0; ElAN < ElsA; ElAN++) {
				int IdxA = A[IdxB][ElAN].Key;
				double ValA = A[IdxB][ElAN].Dat;
				C.At(IdxA, ColN) += ValA * ValB;
			}
		}
	}
}

void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TVec<TIntFltKdV>& C,
		const int RowsA) {
    //// A,B = sparse column matrix
    //EAssert(A.Len() == B.GetRows());
    int Rows = RowsA;
    int ColsB = B.Len();

    if (RowsA == -1) { Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1; }
    EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= Rows);

    C.Gen(ColsB);
    EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.Len());

    for (int ColN = 0; ColN < ColsB; ColN++) {
        int ElsB = B[ColN].Len();
        for (int ElBN = 0; ElBN < ElsB; ElBN++) {
            int IdxB = B[ColN][ElBN].Key;
            double ValB = B[ColN][ElBN].Dat;
            int ElsA = A[IdxB].Len();
            for (int ElAN = 0; ElAN < ElsA; ElAN++) {
                int IdxA = A[IdxB][ElAN].Key;
                double ValA = A[IdxB][ElAN].Dat;
                C[ColN].Add(TIntFltKd(IdxA, ValA * ValB));
            }
        }
    }
}

// C:= A' * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
void TLinAlg::MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
	//// A, B = sparse column matrix
	int ColsA = A.Len();
	int ColsB = B.Len();
	if (C.Empty()) {
		C.Gen(ColsA, ColsB);
	}
	else {
		EAssert(ColsA == C.GetRows() && ColsB == C.GetCols());
	}
	for (int RowN = 0; RowN < ColsA; RowN++) {
		for (int ColN = 0; ColN < ColsB; ColN++) {
			C.At(RowN, ColN) = TLinAlg::DotProduct(A[RowN], B[ColN]);
		}
	}
}

// c := A' * b
void TLinAlg::MultiplyT(const TVec<TIntFltKdV>& A, const TIntFltKdV& b, TFltV& c) {
    //// A = sparse column matrix, b = sparse vector
    int ColsA = A.Len();

    if (c.Empty()) {
        c.Gen(ColsA);
    }
    else {
        EAssert(ColsA == c.Len());
    }
    for (int RowN = 0; RowN < ColsA; RowN++) {
        c[RowN] = TLinAlg::DotProduct(A[RowN], b);
    }
}

// c := A' * b
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TIntFltKdV& b, TFltV& c) {
    //// A = dense matrix, b = sparse vector
    int ColsA = A.GetCols();
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
        for (int ElN = 0; ElN < Els; ElN++) {
            c[ColN] += b[ElN].Dat * A.At(b[ElN].Key, ColN);
        }
    }
}

// c := A' * b
template <class IndexType, class TType, class TSizeTy, bool ColMajor>
void TLinAlg::MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TType, TSizeTy>& b, TFltV& c) {
    //// A = sparse column matrix, b = dense vector
    int ColsA = A.Len();
    // check if the maximum index of the sparse matrix is lesser than length of vector b
    EAssert(TLinAlgSearch::GetMaxDimIdx(A) <= b.Len());

    if (c.Empty()) {
        c.Gen(ColsA);
    }
    else {
        EAssert(ColsA == c.Len());
    }
    c.PutAll(0.0);
    for (int ColN = 0; ColN < ColsA; ColN++) {
        int Els = A[ColN].Len();
        for (int ElN = 0; ElN < Els; ElN++) {
            c[ColN] += A[ColN][ElN].Dat * b[A[ColN][ElN].Key];
        }
    }
}

//#ifdef INTEL
//	void TLinAlg::Multiply(const TFltVV & ProjMat, const TPair<TIntV, TFltV> & Doc, TFltV & Result);
//#endif

// TEST
// D = alpha * A(') * B(') + beta * C(')
typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Gemm(const double& Alpha, const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, const double& Beta,
	const TVVec<TType, TSizeTy, ColMajor>& C, TVVec<TType, TSizeTy, ColMajor>& D, const int& TransposeFlags) {

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

// TEST (works only for RowMajor, TSvd uses only TFltVV matrices)
// B = A^(-1)
typedef enum { DECOMP_SVD } TLinAlgInverseType;
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Inverse(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor >& B, const TLinAlgInverseType& DecompType) {
	switch (DecompType) {
	case DECOMP_SVD:
		TLinAlg::InverseSVD(A, B);
	}
}

// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd)
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::InverseSVD(const TVVec<TType, TSizeTy, ColMajor>& A,
		TVVec<TType, TSizeTy, ColMajor>& B, const double& tol) {
	// check the size of B
	if (B.Empty()) { B.Gen(A.GetCols(), A.GetRows()); }
	EAssert(B.GetCols() == A.GetRows() && B.GetRows() == A.GetCols());

	// create temp matrices
	TVec<TType, TSizeTy> E;
	TSvd SVD;

	//U.Gen(M.GetRows(), M.GetRows());
	//V.Gen(M.GetCols(), M.GetCols());

	// do the SVD decompostion
#ifdef LAPACKE
	TVVec<TType, TSizeTy, ColMajor> U, Vt;
	U.Gen(A.GetRows(), A.GetRows());
	Vt.Gen(A.GetCols(), A.GetCols());

	MKLfunctions::SVDFactorization(A, U, E, Vt);

	const double Threshold = tol*E[0];
	double Sum;
	for (int i = 0; i < Vt.GetCols(); i++) {
		for (int j = 0; j < U.GetRows(); j++) {
			Sum = 0;
			for (int k = 0; k < E.Len(); k++) {
				if (E[k] <= Threshold) { break; }
				Sum += Vt(k,i)*U(j,k) / E[k];	// V is transposed
			}
			B(i,j) = Sum;
		}
	}
#else
	TVVec<TType, TSizeTy, ColMajor> U, V;
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
void TLinAlg::InverseSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B) {
	// create temp matrices
	TVVec<TType, TSizeTy, ColMajor> U, V;
	TVec<TType, TSizeTy> E;
	TSvd SVD;

	//U.Gen(M.GetRows(), M.GetRows());
	//V.Gen(M.GetCols(), M.GetCols());

	U.Gen(A.GetRows(), A.GetRows());
	V.Gen(A.GetCols(), A.GetCols());


	// do the SVD decompostion
	SVD.Svd(A, U, E, V);

	// http://en.wikipedia.org/wiki/Moore%E2%80%93Penrose_pseudoinverse#Singular_value_decomposition_.28SVD.29
	double tol = TFlt::Eps * MAX(A.GetRows(), A.GetCols()) * E[E.GetMxValN()];
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
			double sum = 0;
			for (TSizeTy k = 0; k < U.GetCols(); k++) {
				if (E[k] == 0.0) continue;
				sum += E[k] * V.At(i, k) * U.At(j, k);
			}
			B.At(i, j) = sum;
		}
	}
}

// transpose matrix - B = A'
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::Transpose(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B) {
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
void TLinAlg::GS(TVVec<TType, TSizeTy, ColMajor>& Q) {
	TSizeTy m = Q.GetCols(), n = Q.GetRows();
	for (TSizeTy i = 0; i < m; i++) {
		printf("%d\r", i);
		for (TSizeTy j = 0; j < i; j++) {
			double r = TLinAlg::DotProduct(Q, i, Q, j);
			TLinAlg::AddVec(-r, Q, j, Q, i);
		}
		double nr = TLinAlg::Norm(Q, i);
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
// QR based on Modified Gram-Schmidt decomposition.
void TLinAlg::QR(const TFltVV& X, TFltVV& Q, TFltVV& R, const TFlt& Tol) {
	int Rows = X.GetRows();
	int Cols = X.GetCols();
	int d = MIN(Rows, Cols);

	// make a copy of X
	TFltVV A(X);
	if (Q.GetRows() != Rows || Q.GetCols() != d) { Q.Gen(Rows, d); }
	if (R.GetRows() != d || R.GetCols() != Cols) { R.Gen(d, Cols); }
	TRnd Random;
	for (int k = 0; k < d; k++) {
		R(k, k) = TLinAlg::Norm(A, k);
		// if the remainders norm is too small we construct a random vector (handles rank deficient)
		if (R(k, k) < Tol) {
			// random Q(:,k)
			for (int RowN = 0; RowN < Rows; RowN++) {
				Q(RowN, k) = Random.GetNrmDev();
			}
			// make it orthonormal on others
			for (int j = 0; j < k; j++) {
				TLinAlg::AddVec(-TLinAlg::DotProduct(Q, j, Q, k), Q, j, Q, k);
			}
			TLinAlg::NormalizeColumn(Q, k);
			R(k, k) = 0;
		}
		else {
			// normalize
			for (int RowN = 0; RowN < Rows; RowN++) {
				Q(RowN, k) = A(RowN, k) / R(k, k);
			}
		}

		// make the rest of the columns of A orthogonal to the current basis Q
		for (int j = k + 1; j < Cols; j++) {
			R(k, j) = TLinAlg::DotProduct(Q, k, A, j);
			TLinAlg::AddVec(-R(k, j), Q, k, A, j);
		}
	}
}

// rotates vector (OldX,OldY) for angle Angle (in radians!)
void TLinAlg::Rotate(const double& OldX, const double& OldY, const double& Angle, double& NewX, double& NewY) {
	NewX = OldX*cos(Angle) - OldY*sin(Angle);
	NewY = OldX*sin(Angle) + OldY*cos(Angle);
}

// checks if set of vectors is ortogonal
template <class TSizeTy>
void TLinAlg::AssertOrtogonality(const TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Vecs, const double& Threshold) {
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
//ColMajor oriented data for optimal result
template <class TType, class TSizeTy, bool ColMajor>
void TLinAlg::AssertOrtogonality(const TVVec<TType, TSizeTy, ColMajor>& Vecs, const double& Threshold) {
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
bool TLinAlg::IsOrthonormal(const TFltVV& Vecs, const double& Threshold) {
	int m = Vecs.GetCols();
	TFltVV R(m, m);
	TLinAlg::MultiplyT(Vecs, Vecs, R);
	for (int i = 0; i < m; i++) { R(i, i) -= 1; }
	return TLinAlg::Frob(R) < Threshold;
}
//};


template <class TType, class TSizeTy, bool ColMajor>
TType TLinAlg::GetColMin(const TVVec<TType, TSizeTy, ColMajor>& X, const int& ColN) {
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
void TLinAlg::GetColMinV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TType, TSizeTy>& ValV) {
	const TSizeTy Cols = X.GetCols();

	ValV.Gen(Cols);
	for (int ColN = 0; ColN < Cols; ColN++) {
		ValV[ColN] = GetColMin(X, ColN);
	}
}


template <class TType, class TSizeTy, bool ColMajor>
inline void TLinAlg::Pow(const TVVec<TType, TSizeTy, ColMajor>& Mat,
		const int& k, TVVec<TType, TSizeTy, ColMajor>& PowVV) {
	EAssertR(Mat.GetRows() == Mat.GetCols(), "TLinAlg::Pow: Can only compute powers of square matrices!");

	const TSizeTy Dim = Mat.GetRows();

	if (k == 0) {
		TLinAlgTransform::Identity(Dim, PowVV);
	} else if (k < 0) {
		TVVec<TType, TSizeTy, ColMajor> InverseVV;
		TLinAlg::Inverse(Mat, InverseVV, TLinAlgInverseType::DECOMP_SVD);
		Pow(InverseVV, -k, PowVV);
	} else {
		PowVV.Gen(Dim, Dim);

		// we will compute the power using the binary algorithm
		// we will always hold the newest values in X, so when
		// finishing the algorithm, the result will be in X

		// X <- A
		TVVec<TType, TSizeTy, ColMajor> TempMat(Mat);			// temporary matrix

		// pointers, so swapping is faster
		TVVec<TType, TSizeTy, ColMajor>* X = &TempMat;
		TVVec<TType, TSizeTy, ColMajor>* X1 = &PowVV;			// use the space already available

		// temporary variables
		TVVec<TType, TSizeTy, ColMajor>* Temp;

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

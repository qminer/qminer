/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef LINALG_H
#define LINALG_H

///////////////////////////////////////////////////////////////////////
// Blas Support
#ifdef BLAS
	#define MKL_Complex8 std::complex<float>
	#define MKL_Complex16 std::complex<double>
	#define lapack_complex_float std::complex<float>
	#define lapack_complex_double std::complex<double>
	#define LAPACK_COMPLEX_CPP
	#ifdef AMD
		#include "acml.h"
	#elif INTEL 
		#undef small
		#include "mkl.h"
	//#include "mkl_scalapack.h"
	#else
		#include "cblas.h"
		#ifdef LAPACKE
			#include "lapacke.h"
		#endif
	#endif
#endif
#include "base.h"

// define macros
#define TEMPLATE_TDnsV template <class TType, class TSizeTy = int> inline
#define TEMPLATE_TDnsVV template <class TType, class TSizeTy = int, bool ColMajor = false> inline
#define TEMPLATE_TSpV template <class TSizeTy = int> inline

#define TDnsV TVec<TType, TSizeTy>
#define TDnsVV TVVec<TType, TSizeTy, ColMajor>
#define TSpV TVec<TIntFltKd, TSizeTy>
#define TSpVV TVec<TIntFltKdV, TSizeTy>

namespace TypeCheck {
	template<typename T1>
	struct is_float { static const bool value = false; };
	template<>
	struct is_float<float> { static const bool value = true; };
	template<>
	struct is_float<TNum<float> > { static const bool value = true; };

	template<typename T1>
	struct is_double { static const bool value = false; };

	template<>
	struct is_double<double> { static const bool value = true; };
	template<>
	struct is_double<TNum<double> > { static const bool value = true; };

	template<typename T1>
	struct is_complex_float { static const bool value = false; };
	template<>
	struct is_complex_float< std::complex<float> > { static const bool value = true; };
	template<>
	struct is_complex_float< TNum<std::complex<float> > > { static const bool value = true; };

	template<typename T1>
	struct is_complex_double { static const bool value = false; };
	template<>
	struct is_complex_double< std::complex<double> > { static const bool value = true; };
	template<>
	struct is_complex_double< TNum<std::complex<double> > > { static const bool value = true; };
}

// the matrix dimension classificator for the (Dim parameter)
enum TMatDim { 
	mdCols = 1, 
	mdRows = 2
};

//////////////////////////////////////////////////////////////////////
// Miscellaneous linear algebra functions
class TLAMisc {
public:
	//Sort double array
#ifdef SCALAPACK
	template<class TSizeTy>
	static void Sort(TVec<TFlt, TSizeTy> & Vec, TVec<TSizeTy, TSizeTy>& Index, const TBool& DecreseP);
#endif

    // sums elements in vector
//	static int SumVec(const TIntV& Vec);
//	static double SumVec(const TFltV& Vec);
};

#ifdef SCALAPACK
template<class TSizeTy>
void TLAMisc::Sort(TVec<TFlt, TSizeTy> & Vec, TVec<TSizeTy, TSizeTy>& Index, const TBool& DecreseP) {
    if (Index.Empty()) {
        TLAMisc::FillRange(Vec.Len(), index);
    }
    char* id = DecreseP ? "D" : "I";
    TSizeTy n = Vec.Len();
    TSizeTy info;
    dlasrt2(id, &n, &Vec[0].Val, &Index[0], &info);
}
#endif

//////////////////////////////////////////////////////////////////////
// Template-ised Sparse Operations
template <class TKey, class TDat>
class TSparseOps {
public:
    /// Transform sparse matrix from (row,col,val) triplets to a vector of sparse columns
	static void CoordinateCreateSparseColMatrix(const TVec<TKey>& RowIdxV, const TVec<TKey>& ColIdxV,
        const TVec<TDat>& ValV, TVec<TVec<TKeyDat<TKey, TDat> > >& ColMatrix, const TKey& Cols);
    /// Merge given sparse vectors using +operator on KeyDat elements with same Key value
	static void SparseMerge(const TVec<TKeyDat<TKey, TDat> >& SrcV1,
        const TVec<TKeyDat<TKey, TDat> >& SrcV2, TVec<TKeyDat<TKey, TDat> >& DstV);
    /// Construct sparse linear combination (DstV = p*SrcV1 + q*SrcV2)
	static void SparseLinComb(const double& p, const TVec<TKeyDat<TKey, TDat> >& SrcV1,
        const double& q, const TVec<TKeyDat<TKey, TDat> >& SrcV2,
        TVec<TKeyDat<TKey, TDat> >& DstV);
};

typedef TSparseOps<TInt, TFlt> TSparseOpsIntFlt;

///////////////////////////////////////////////////////////////////////
/// Matrix. Class for matrix-vector and matrix-matrix operations
class TMatrix {
private:
	bool Transposed;
protected:
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const = 0;
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const = 0;
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const = 0;
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const = 0;
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const { FailR("TMatrix PMultiply(const TFltVV& B, TFltVV& Result) not implemented"); }
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const { FailR("TMatrix PMultiplyT(const TFltVV& B, TFltVV& Result) not implemented"); }

	virtual int PGetRows() const = 0;
	virtual int PGetCols() const = 0;
public:
	TMatrix() : Transposed(false) {}
	virtual ~TMatrix() { }

	// Result = A * B(:,ColId)
	void Multiply(const TFltVV& B, int ColId, TFltV& Result) const {
		if (Transposed) { PMultiplyT(B, ColId, Result); }
		else { PMultiply(B, ColId, Result); }
	}
	// Result = A' * B(:,ColId)
	void MultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {
		if (Transposed) { PMultiply(B, ColId, Result); }
		else { PMultiplyT(B, ColId, Result); }
	}

	// Result = A * Vec
	void Multiply(const TFltV& Vec, TFltV& Result) const {
		if (Transposed) { PMultiplyT(Vec, Result); }
		else { PMultiply(Vec, Result); }
	}
	// Result = A' * Vec
	void MultiplyT(const TFltV& Vec, TFltV& Result) const{
		if (Transposed) { PMultiply(Vec, Result); }
		else { PMultiplyT(Vec, Result); }
	}

	// Result = A * B
	void Multiply(const TFltVV& B, TFltVV& Result) const {
		if (Transposed) { PMultiplyT(B, Result); }
		else { PMultiply(B, Result); }
	}
	// Result = A' * B
	void MultiplyT(const TFltVV& B, TFltVV& Result) const {
		if (Transposed) { PMultiply(B, Result); }
		else { PMultiplyT(B, Result); }
	}

	// number of rows
	int GetRows() const { return Transposed ? PGetCols() : PGetRows(); }
	// number of columns
	int GetCols() const { return Transposed ? PGetRows() : PGetCols(); }

	virtual void Transpose() { Transposed = !Transposed; }

	void Save(TSOut& SOut) const { TBool(Transposed).Save(SOut); }
	void Load(TSIn& SIn) { Transposed = TBool(SIn); }
};

///////////////////////////////////////////////////////////////////////
// Sparse-Column-Matrix
//  matrix is given with columns as sparse vectors
class TSparseColMatrix : public TMatrix {
public:
	// number of rows and columns of matrix
	TInt RowN, ColN;
	// vector of sparse columns
	TVec<TIntFltKdV> ColSpVV;
protected:
	// Result = A * B(:,ColId)
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A * Vec
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
	// Result = A' * B(:,ColId)
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A' * Vec
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;
	// Result = A * B
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const;
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const;

    void Init();
	int PGetRows() const { return RowN; }
	int PGetCols() const { return ColN; }

public:
	TSparseColMatrix(): TMatrix() {}
	TSparseColMatrix(const int& _RowN, const int& _ColN): RowN(_RowN), ColN(_ColN), ColSpVV() {}
	TSparseColMatrix(const TVec<TIntFltKdV>& _ColSpVV): TMatrix(), ColSpVV(_ColSpVV) { Init(); }
	TSparseColMatrix(const TVec<TIntFltKdV>& _ColSpVV, const int& _RowN, const int& _ColN) :
		TMatrix(), RowN(_RowN), ColN(_ColN), ColSpVV(_ColSpVV) {}

	void Save(TSOut& SOut) {
        RowN.Save(SOut); ColN.Save(SOut); ColSpVV.Save(SOut);
	}
	void Load(TSIn& SIn) {
        RowN.Load(SIn); ColN.Load(SIn); ColSpVV = TVec<TIntFltKdV>(SIn);
	}
};

///////////////////////////////////////////////////////////////////////
// Sparse-Row-Matrix
//  matrix is given with rows as sparse vectors
class TSparseRowMatrix : public TMatrix {
public:
	// number of rows and columns of matrix
	TInt RowN, ColN;
	// vector of sparse rows
	TVec<TIntFltKdV> RowSpVV;
protected:
	// Result = A * B(:,ColId)
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A * Vec
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
	// Result = A' * B(:,ColId)
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A' * Vec
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;
	// Result = A * B
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const { FailR("Not implemented yet"); } // TODO
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const { FailR("Not implemented yet"); } // TODO

    void Init();
	int PGetRows() const { return RowN; }
	int PGetCols() const { return ColN; }

public:
	TSparseRowMatrix(): TMatrix() {}
	TSparseRowMatrix(const TVec<TIntFltKdV>& _RowSpVV): TMatrix(), RowSpVV(_RowSpVV) { Init(); }
	TSparseRowMatrix(const TVec<TIntFltKdV>& _RowSpVV, const int& _RowN, const int& _ColN):
		TMatrix(), RowN(_RowN), ColN(_ColN), RowSpVV(_RowSpVV) {}
	// loads Matlab sparse matrix format: row, column, value.
	//   Indexes start with 1.
	TSparseRowMatrix(const TStr& MatlabMatrixFNm);
	void Save(TSOut& SOut) {
		RowN.Save(SOut); ColN.Save(SOut); RowSpVV.Save(SOut);
	}
	void Load(TSIn& SIn) {
		RowN.Load(SIn); ColN.Load(SIn); RowSpVV = TVec<TIntFltKdV>(SIn);
	}
};

///////////////////////////////////////////////////////////////////////
// Full-Col-Matrix
//  matrix is given with columns of full vectors
class TFullColMatrix : public TMatrix {
public:
	// number of rows and columns of matrix
	TInt RowN, ColN;
	// vector of sparse columns
	TVec<TFltV> ColV;
protected:
	// Result = A * B(:,ColId)
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A * Vec
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
	// Result = A' * B(:,ColId)
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A' * Vec
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;
	// Result = A * B
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const { FailR("Not implemented yet"); } // TODO
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const { FailR("Not implemented yet"); } // TODO

	int PGetRows() const { return RowN; }
	int PGetCols() const { return ColN; }

public:
	TFullColMatrix(): TMatrix() {}
	// loads matrix saved in matlab with command:
	//  save -ascii Matrix.dat M
	TFullColMatrix(const TStr& MatlabMatrixFNm);
	TFullColMatrix(TVec<TFltV>& RowVV);
	void Save(TSOut& SOut) { RowN.Save(SOut); ColN.Save(SOut);  ColV.Save(SOut); }
	void Load(TSIn& SIn) { RowN.Load(SIn); ColN.Load(SIn); ColV.Load(SIn); }
};

///////////////////////////////////////////////////////////////////////
// Structured-Covariance-Matrix
//  matrix is a product of two sparse matrices X Y' (column examples, row features), 
//  which are centered implicitly by using two dense mean vectors
class TStructuredCovarianceMatrix : public TMatrix {
private:
	// number of rows and columns of matrix
	int XRows, YRows;
	int Samples;
	// mean vectors
	TFltV MeanX;
	TFltV MeanY;
	TTriple<TIntV, TIntV, TFltV> X;
	TTriple<TIntV, TIntV, TFltV> Y;
protected:
	// Result = A * B(:,ColId)
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A * B
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const;
	// Result = A * Vec
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
	// Result = A' * B(:,ColId)
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const;
	// Result = A' * Vec
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;

	int PGetRows() const { return XRows; }
	int PGetCols() const { return YRows; }

public:
	TStructuredCovarianceMatrix() : TMatrix() {}
	TStructuredCovarianceMatrix(const int XRowN_, const int YRowN_, const int SampleN_, const TFltV& MeanX_, const TFltV& MeanY_, const TTriple<TIntV, TIntV, TFltV>& X_, const TTriple<TIntV, TIntV, TFltV>& Y_) : TMatrix(), XRows(XRowN_), YRows(YRowN_), Samples(SampleN_), MeanX(MeanX_), MeanY(MeanY_), X(X_), Y(Y_) {};
	void Save(TSOut& SOut) { SOut.Save(XRows); SOut.Save(YRows); SOut.Save(Samples); MeanX.Save(SOut); MeanY.Save(SOut); X.Save(SOut); Y.Save(SOut); }
	void Load(TSIn& SIn) { SIn.Load(XRows); SIn.Load(YRows); SIn.Load(Samples); MeanX.Load(SIn); MeanY.Load(SIn); X.Load(SIn); Y.Load(SIn); }
};

//////////////////////////////////////////////////////////////////////
/// Linear algebra input/output operations
class TLinAlgIO {
public:
	// Dumps vector to file so Excel can read it
	static void SaveCsvTFltV(const TFltV& Vec, TSOut& SOut);
	// Dumps sparse vector to file so Matlab can read it
	static void SaveMatlabTFltIntKdV(const TIntFltKdV& SpV, const int& ColN, TSOut& SOut);
	/// Dumps sparse matrix to file so Matlab can read it
	static void SaveMatlabSpMat(const TVec<TIntFltKdV>& SpMat, TSOut& SOut);
	/// Dumps sparse matrix to file so Matlab can read it
	static void SaveMatlabSpMat(const TTriple<TIntV, TIntV, TFltV>& SpMat, TSOut& SOut);
	// Dumps vector to file so Matlab can read it
	static void SaveMatlabTFltV(const TFltV& m, const TStr& FName);
	// Dumps vector to file so Matlab can read it
	static void SaveMatlabTIntV(const TIntV& m, const TStr& FName);
	// Dumps column ColId from m to file so Matlab can read it
	static void SaveMatlabTFltVVCol(const TFltVV& m, int ColId, const TStr& FName);
	// Dumps matrix to file so Matlab can read it
	static void SaveMatlabTFltVV(const TFltVV& m, const TStr& FName);
	// Dumps matrix to the output stream so Matlab can read it
	static void SaveMatlabTFltVV(const TFltVV& m, TSOut& SOut);
	// Dumps main minor rowN x colN to file so Matlab can read it
	static void SaveMatlabTFltVVMjrSubMtrx(const TFltVV& m, int rowN, int colN, const TStr& FName);

	// loads matlab full matrix
	static void LoadMatlabTFltVV(const TStr& FNm, TVec<TFltV>& ColV);
	// loads matlab full matrix
	static void LoadMatlabTFltVV(const TStr& FNm, TFltVV& MatrixVV);
	// loads matlab full matrix
	static void LoadMatlabTFltVV(TVec<TFltV>& ColV, TSIn& SIn);
	// loads matlab full matrix
	static void LoadMatlabTFltVV(TFltVV& MatrixVV, TSIn& SIn);

	// prints vector to screen
	static void PrintTFltV(const TFltV& Vec, const TStr& VecNm);
	// print matrix to string
	static void PrintTFltVVToStr(const TFltVV& A, TStr& Out);
	// print matrixt to screen
	static void PrintTFltVV(const TFltVV& A, const TStr& MatrixNm);
	// print sparse matrix to screen
	static void PrintSpMat(const TTriple<TIntV, TIntV, TFltV>& A, const TStr& MatrixNm);
	// print sparse matrix to screen
	static void PrintSpMat(const TVec<TIntFltKdV>& A, const TStr& MatrixNm);
	// prints vector to screen
	static void PrintTIntV(const TIntV& Vec, const TStr& VecNm);
};

//////////////////////////////////////////////////////////////////////
/// Statistics on linear algebra structures
class TLinAlgStat {
public:
	// returns the mean value of Vec.
	static double Mean(const TFltV& Vec);
	// returns the mean value along the dimension (Dim) of Mat. See Matlab documentation - mean().
	static void Mean(const TFltVV& Mat, TFltV& Vec, const TMatDim& Dim = TMatDim::mdCols);
	// returns standard deviation. See Matlab documentation - std().
	static void Std(const TFltVV& Mat, TFltV& Vec, const int& Flag = 0, const TMatDim& Dim = TMatDim::mdCols);
	// returns the z-score for each element of X such that columns of X are centered to have mean 0 and scaled to have standard deviation 1.
	static void ZScore(const TFltVV& Mat, TFltVV& Vec, const int& Flag = 0, const TMatDim& Dim = TMatDim::mdCols);
};

//////////////////////////////////////////////////////////////////////
/// Transformations of linear algebra structures
class TLinAlgTransform {
public:

	//=======================================================
	// FILL VECTORS / MATRICES
	//=======================================================

	// set all components
	static void Fill(TFltVV& M, const double& Val);
	static void Fill(TFltV& M, const double& Val);
	// sets all compnents to zero
	static void FillZero(TFltV& Vec) { Vec.PutAll(0.0); }
	static void FillZero(TFltVV& M) { Fill(M, 0.0); }

	// fills vector with random numbers
	static void FillRnd(TFltV& Vec) { TRnd Rnd(0); FillRnd(Vec.Len(), Vec, Rnd); }
	static void FillRnd(TFltV& Vec, TRnd& Rnd) { FillRnd(Vec.Len(), Vec, Rnd); }
	static void FillRnd(TFltVV& Mat) { TRnd Rnd(0); FillRnd(Mat, Rnd); }
	static void FillRnd(TFltVV& Mat, TRnd& Rnd) { FillRnd(Mat.Get1DVec(), Rnd); }
	static void FillRnd(const int& Len, TFltV& Vec, TRnd& Rnd);

	// set matrix to identity
	static void FillIdentity(TFltVV& M);
	static void FillIdentity(TFltVV& M, const double& Elt);

	//=======================================================
	// INITIALIZE VECTORS / MATRICES
	//=======================================================

	// generates a vector of ones with dimension dim
	template <class TVal, class TSizeTy>
	static void OnesV(const int& Dim, TVec<TVal, TSizeTy>& OnesV);

	// set vector to range
	/// generates a vector with i on index i
	template <class TVal, class TSizeTy>
	static void RangeV(const TSizeTy& Dim, TVec<TVal, TSizeTy>& RangeV);
	// returns a vector with a sequence starting at Min and ending at Max, both inclusive
	template <class TVal, class TSizeTy>
	static void RangeV(const TSizeTy& Min, const TSizeTy& Max, TVec<TVal, TSizeTy>& Res);

	// creates a diagonal matrix
	template <class TType, class TSizeTy, bool ColMajor>
	static void Diag(const TVec<TType, TSizeTy>& DiagV, TVVec<TType, TSizeTy, ColMajor>& D);
	// creates a diagonal matrix
	static void Diag(const TFltV& Vec, TVec<TIntFltKdV>& Mat);

	/// create an identity matrix with the given dimension
	template <class TType, class TSizeTy, bool ColMajor>
	static void Identity(const TSizeTy& Dim, TVVec<TType, TSizeTy, ColMajor>& X);

	//=======================================================
	// ELEMENT-WISE TRANSFORMATIONS
	//=======================================================

	/// square root of each individual element
	template <class TMatType>
	inline static void Sqrt(TMatType& X);

	/// square each individual element
    template <class TMatType>
    inline static void Sqr(TMatType& X);

	//=======================================================
	// ROW / COLUMN TRANSFORMATIONS
	//=======================================================

    /// centers all the rows around the mean value
	template <class TType, class TSizeTy, bool ColMajor>
	static void CenterRows(TVVec<TType, TSizeTy, ColMajor>& X);

	//=======================================================
	// TYPE TRANSFORMATIONS
	//=======================================================

	// converts full vector to sparse
	static void ToSpVec(const TFltV& Vec, TIntFltKdV& SpVec,
		const double& CutWordWgtSumPrc = 0.0);
	// converts sparse vector to full
	static void ToVec(const TIntFltKdV& SpVec, TFltV& Vec, const int& VecLen);

	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sparse(const TVVec<TType, TSizeTy, ColMajor>& A,
			TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>,
			TVec<TType, TSizeTy>>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TIntFltKdV>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Full(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>,
			TVec<TType, TSizeTy>>& A, TVVec<TType, TSizeTy, ColMajor>& B, const int Rows,
			const int Cols);
	// Sparse to dense transform
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Full(const TVec<TIntFltKdV, TSizeTy>& A,
			TVVec<TType, TSizeTy, ColMajor>& B, TSizeTy Rows);

	// Vector of sparse vectors to sparse matrix (coordinate representation)
	static void Convert(const TVec<TPair<TIntV, TFltV>>& A, TTriple<TIntV, TIntV, TFltV>& B);
	// Vector of sparse vectors to sparse matrix (coordinate representation)
	static void Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>&B);
};

//////////////////////////////////////////////////////////////////////
/// Contains methods to check the properties of linear algebra structures
class TLinAlgCheck {
public:
	/// returns true, if the vector is a zero vector
	static bool IsZero(const TFltV& Vec, const double& Eps = 1e-6);
	static bool IsZeroOrig(const TFltV& Vec) {
		int Len = Vec.Len();
		for (int i = 0; i < Len; i++) {
			if (Vec[i] != 0.0) { return false; }
		}
		return true;
	}
	/// returns true if the matrix contains at least one nan value
	static bool ContainsNan(const TFltVV& FltVV);

	// checks if set of vectors is ortogonal
	template <class TSizeTy = int>
	inline static void AssertOrtogonality(const TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Vecs, const double& Threshold);
	//ColMajor oriented data for optimal result
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AssertOrtogonality(const TVVec<TType, TSizeTy, ColMajor>& Vecs, const double& Threshold);
	inline static bool IsOrthonormal(const TFltVV& Vecs, const double& Threshold);
};

//////////////////////////////////////////////////////////////////////
/// Search elements of matrices and vectors
class TLinAlgSearch {
public:
	// returns the index of the maximum element
	TEMPLATE_TDnsV
	static TSizeTy GetMaxIdx(const TDnsV& Vec);
	// returns the index of the minimum element
	TEMPLATE_TDnsV
	static TSizeTy GetMinIdx(const TDnsV& Vec);

	// returns the maximal value in the array
	TEMPLATE_TDnsV
	static TType GetMaxVal(const TDnsV& Vec);

	// gets the maximal index of a sparse vector
	static int GetMaxDimIdx(const TIntFltKdV& SpVec);
	// gets the maximal row index of a sparse column matrix
	static int GetMaxDimIdx(const TVec<TIntFltKdV>& SpMat);

	// TEST
	/// find the index of maximum elements for a given row of X
	TEMPLATE_TDnsVV
	static int GetRowMaxIdx(const TDnsVV& X, const TSizeTy& RowN);
	/// find the index of the smallest element in the row
	TEMPLATE_TDnsVV
	static int GetRowMinIdx(const TDnsVV& X, const TSizeTy& RowN);
	// find the index of maximum elements for a given each col of X
	TEMPLATE_TDnsVV
	static int GetColMaxIdx(const TDnsVV& X, const TSizeTy& ColN);

	// TEST
	/// find the index of maximum elements for each row of X
	TEMPLATE_TDnsVV
	static void GetRowMaxIdxV(const TDnsVV& X, TVec<TInt, TSizeTy>& IdxV);
	/// find the index of minimum elements for each row of X
	TEMPLATE_TDnsVV
	static void GetRowMinIdxV(const TDnsVV& X, TVec<TInt, TSizeTy>& IdxV);
	// find the index of maximum elements for each col of X
	TEMPLATE_TDnsVV
	static void GetColMaxIdxV(const TDnsVV& X, TVec<TInt, TSizeTy>& IdxV);
	// find the index of maximum elements for a given each col of X
	TEMPLATE_TDnsVV
	static TSizeTy GetColMinIdx(const TDnsVV& X, const TSizeTy& ColN);
	// find the index of maximum elements for each col of X
	static void GetColMinIdxV(const TFltVV& X, TIntV& IdxV);
	TEMPLATE_TDnsVV
	static TType GetColMin(const TDnsVV& X, const int& ColN);
	TEMPLATE_TDnsVV
	static void GetColMinV(const TDnsVV& X, TDnsV& ValV);
};

///////////////////////////////////////////////////////////////////////
// Basic Linear Algebra operations
class TLinAlg {
public:
	//===========================================================
	// PRODUCTS
	//===========================================================

    /// Result = <x, y>
	TEMPLATE_TDnsV
	static double DotProduct(const TDnsV& x, const TDnsV& y);
    /// Result = <X(:,ColId), y>
	static double DotProduct(const TVec<TFltV>& X, int ColId, const TFltV& y);
    /// Result = <X[ColId], y>
	static double DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TFltV& y);
    /// Result = <X(:,ColId), Y(:,ColId)>
	TEMPLATE_TDnsVV
	static double DotProduct(const TDnsVV& X, int ColIdX, const TDnsVV& Y, int ColIdY);
    /// Result = <X(:,ColId), y>
	TEMPLATE_TDnsVV
	static double DotProduct(const TDnsVV& X, int ColId, const TDnsV& y);

	// sparse dot products:
	// <x,y> where x AND y are sparse
	//TODO TIntFltKdV indexing and is TInt enough?
    /// Result = <x, y>
	static double DotProduct(const TIntFltKdV& x, const TIntFltKdV& y);
	// <x,y> where only y is sparse
	// TODO TIntFltKdV indexing and is TInt enough?
    /// Result = <x, y>
	TEMPLATE_TDnsV
	static double DotProduct(const TDnsV& x, const TVec<TIntFltKd>& y);
    
	/// Result = <X(:,ColId), y>
	TEMPLATE_TDnsVV
	static double DotProduct(const TDnsVV& X, int ColId, const TIntFltKdV& y);

	/// Result = <X, Y>
	TEMPLATE_TDnsVV
	static double DotProduct(const TDnsVV& X, const TDnsVV& Y);
	TEMPLATE_TDnsVV
	static double DotProduct(const TVec<TIntFltKdV>& X, const TDnsVV& Y);

	// z = x * y'
	TEMPLATE_TDnsVV
	static void OuterProduct(const TDnsV& x, const TDnsV& y, TDnsVV& Z);

	// Hadamard product
	TEMPLATE_TDnsVV
    static void HadamardProd(const TDnsVV& X, const TDnsVV& Y, TDnsVV& Z);
	TEMPLATE_TDnsVV
	static void HadamardProd(const TVec<TIntFltKdV>& X, const TDnsVV& Y, TDnsVV& Z);

	//===========================================================
	// SUMS AND LINEAR COMBINATIONS
	//===========================================================

	TEMPLATE_TDnsV
	static void LinComb(const double& p, const TDnsV& x, const double& q, const TDnsV& y,
			TDnsV& z);
	//TODO this will work only for glib type TFlt
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void LinCombInPlace(const TType& alpha, const TVec<TNum<TType>, TSizeTy>& x,
		const TType& beta, TVec<TNum<TType>, TSizeTy>& y);

	TEMPLATE_TDnsVV
	static void LinComb(const double& p, const TDnsVV& X, const double& q, const TDnsVV& Y,
			TDnsVV& Z);
	// z = p * x + q * y
	static void LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z);
	static void LinComb(const double& p, const TFltVV& X, int ColId,
		const double& q, const TFltV& y, TFltV& z);
	static void LinComb(const double& p, const TFltVV& X, int DimId,
		const double& q, const TFltV& y, TFltV& z, int Dim);
	static void LinComb(const double& p, const TFltVV& X, const double& q, const TFltVV& Y, TFltVV& Z);
	static void LinComb(const double& p, const TVec<TIntFltKdV>& X, const double& q, const TVec<TIntFltKdV>& Y, TVec<TIntFltKdV>& Z);
    static void LinComb(const double& p, const TFltVV& X, const double& q, const TVec<TIntFltKdV>& Y, TFltVV& Z);
    static void LinComb(const double& p, const TVec<TIntFltKdV>& X, const double& q, TFltVV const& Y, TVec<TIntFltKdV>& Z);
	TEMPLATE_TDnsV
	static void ConvexComb(const double& p, const TDnsV& x, const TDnsV& y, TDnsV& z);
	//this will fail if TType != TFlt, Specialization should be used

	//y = k * x + y
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const TType& k, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y);
	// z := k * x + y
	TEMPLATE_TDnsV
	static void AddVec(const double& k, const TDnsV& x, const TDnsV& y, TDnsV& z);
	// z := k * X[ColId] + y
	static void AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z);
	// z := k * X(:,ColId) + y
	static void AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z);
	// z := x + y
	TEMPLATE_TDnsV
	static void AddVec(const TDnsV& x, const TDnsV& y, TDnsV& z);
	// z := k * x + y
	static void AddVec(const double& k, const TIntFltKdV& x, const TFltV& y, TFltV& z);
	// z := k * X[ColId] + y
	static void AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z);
	// y := k * x + y
	static void AddVec(const double& k, const TIntFltKdV& x, TFltV& y);
	// Y(:,Col) += k * X(:,Col)
	TEMPLATE_TDnsVV
	static void AddVec(double k, const TDnsVV& X, TSizeTy ColIdX, TDnsVV& Y, TSizeTy ColIdY);
	// Y(:,ColIdY) += k * x
	TEMPLATE_TDnsVV
	static void AddVec(const double& k, const TDnsV& x, TDnsVV& Y, const TSizeTy& ColIdY);
	// Result += k * X(:,Col)
	TEMPLATE_TDnsVV
	static void AddVec(double k, const TDnsVV& X, int ColId, TDnsV& Result);
	// z = x + y
	static void AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z);

	// Result = SUM(x)
	TEMPLATE_TDnsV
	static double SumVec(const TDnsV& x);
	// Result = SUM(x)
	static double SumVec(const TIntFltKdV& x);
	// Result = SUM(k*x + y)
	TEMPLATE_TDnsV
	static double SumVec(double k, const TDnsV& x, const TDnsV& y);

	//===========================================================
	// DISTANCES AND NORMS
	//===========================================================

	// Result = ||x-y||^2 (Euclidian);
	TEMPLATE_TDnsV
	static double EuclDist2(const TDnsV& x, const TDnsV& y);

    TEMPLATE_TSpV
    static double EuclDist2(const TSpV& x, const TSpV& y);
	// Result = ||x-y||^2 (Euclidian);
    static double EuclDist2(const TFltPr& x, const TFltPr& y);
	// Result = ||x-y|| (Euclidian)
	TEMPLATE_TDnsV
	static double EuclDist(const TDnsV& x, const TDnsV& y);
	// Result = ||x-y|| (Euclidian)
	static double EuclDist(const TFltPr& x, const TFltPr& y);
	// Result = ||A||_F (Frobenious) - A dense matrix
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static TType Frob(const TVVec<TNum<TType>, TSizeTy, ColMajor> &A);

	// Result = ||A||_F^2 (Squared Frobenious) - A dense matrix
	TEMPLATE_TDnsVV
	static TType Frob2(const TDnsVV &A);
	// Result = ||A||_F (Frobenious) - A sparse matrix
	static double Frob(const TVec<TIntFltKdV> &A);
	// Result = ||A||_F^2 (Squared Frobenious) - A sparse matrix
	static double Frob2(const TVec<TIntFltKdV> &A);
	// Result = ||A - B||_F (Frobenious)
	TEMPLATE_TDnsVV
	static double FrobDist2(const TDnsVV& A, const TDnsVV& B);
	// Result = ||A - B||_F (Frobenious)
	TEMPLATE_TDnsV
	static double FrobDist2(const TDnsV& A, const TDnsV& B);

	// ||x||^2 (Euclidian)
	TEMPLATE_TDnsV
	static double Norm2(const TDnsV& x);
	// ||x|| (Euclidian);
	TEMPLATE_TDnsV
	static double Norm(const TDnsV& x);
	// x := x / ||x||
	TEMPLATE_TDnsV
	static double Normalize(TDnsV& x);
	// Normalize X(:,ColId)
	TEMPLATE_TDnsVV
	static void NormalizeColumn(TDnsVV& X, const TSizeTy& ColId);
	// Normalize the columns of X
	TEMPLATE_TDnsVV
	static void NormalizeColumns(TDnsVV& X);
	TEMPLATE_TDnsVV
	static void NormalizeRows(TDnsVV& X);
	TEMPLATE_TDnsVV
	static void NormalizeColumns(TDnsVV& X, TBool ColumnMajor);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void NormalizeColumns(TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X);
	// Normalize the columns of X
	TEMPLATE_TSpV
	static void NormalizeColumns(TSpVV& X);
	TEMPLATE_TDnsVV
	static double FrobNorm2(const TDnsVV& X);
	TEMPLATE_TDnsVV
	static double FrobNorm(const TDnsVV& X);
	static double FrobNorm2(const TVec<TIntFltKdV>& X);
	// ||x||^2 (Euclidian), x is sparse
	TEMPLATE_TSpV
	static double Norm2(const TSpVV& x);
	// ||x|| (Euclidian), x is sparse
	TEMPLATE_TSpV
	static double Norm(const TSpVV& x);
	// ||X(:, ColId)|| (Euclidian), x is sparse
	TEMPLATE_TSpV
	static double Norm(const TSpVV& x, const int& ColId);
	// x := x / ||x||, x is sparse
	template<class TSizeTy = int, TSizeTy>
	inline static void Normalize(TVec<TIntFltKdV>& x);
	// ||X(:,ColId)||^2 (Euclidian);
	TEMPLATE_TDnsVV
	static double Norm2(const TDnsVV& X, const TSizeTy& ColId);
	// ||X(:,ColId)|| (Euclidian)
	TEMPLATE_TDnsVV
	static double Norm(const TDnsVV& X, int ColId);
	// L1 norm of x (Sum[|xi|, i = 1..n]);
	TEMPLATE_TDnsV
	static double NormL1(const TDnsV& x);
	// L1 norm of k*x+y (Sum[|k*xi+yi|, i = 1..n])
	TEMPLATE_TDnsV
	static double NormL1(double k, const TDnsV& x, const TDnsV& y);
	// L1 norm of x (Sum[|xi|, i = 1..n]);
	static double NormL1(const TIntFltKdV& x);
	// x := x / ||x||_1
	TEMPLATE_TDnsV
	static void NormalizeL1(TDnsV& x);
	// x := x / ||x||_1
	inline static void NormalizeL1(TIntFltKdV& x);
	TEMPLATE_TDnsV
	static double NormLinf(const TDnsV& x);
	// Linf norm of x (Max{|xi|, i = 1..n});
	static double NormLinf(const TIntFltKdV& x);
	TEMPLATE_TDnsV
	static void NormalizeLinf(TDnsV& x);
	// x := x / ||x||_inf, , x is sparse
	static void NormalizeLinf(TIntFltKdV& x);

	// stores the squared norm of all the columns into the output vector
	static void GetColNormV(const TFltVV& X, TFltV& ColNormV);
	// stores the norm of all the columns into the output vector
    static void GetColNormV(const TVec<TIntFltKdV>& X, TFltV& ColNormV);
	// stores the norm of all the columns into the output vector
	static void GetColNorm2V(const TFltVV& X, TFltV& ColNormV);
	// stores the norm of all the columns into the output vector
	static void GetColNorm2V(const TVec<TIntFltKdV>& SpVV, TFltV& ColNormV);

	TEMPLATE_TDnsVV
	static void Sum(const TDnsVV& X, TDnsV& y, const int Dimension = 1);
	TEMPLATE_TDnsVV
	static double SumRow(const TDnsVV& X, const int& RowN);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sum(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X, TVec<TType, TSizeTy>& y, const int Dimension = 1);

	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Transpose(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
		TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& At);
	// Transpose
	static void Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows = -1);
	// Sign
	static void Sign(const TVec<TIntFltKdV>& Mat, TVec<TIntFltKdV>& Mat2);

	TEMPLATE_TDnsV
	static void MultiplyScalar(const double& k, TDnsV& x);

	// y := k * x
	TEMPLATE_TDnsV
	static void MultiplyScalar(const double& k, const TDnsV& x, TDnsV& y);
	// y := k * x
	static void MultiplyScalar(const double& k, const TIntFltKdV& x, TIntFltKdV& y);
	// Y := k * X
	TEMPLATE_TDnsVV
	static void MultiplyScalar(const double& k, const TDnsVV& X, TDnsVV& Y);
	// Y := k * X
	TEMPLATE_TSpV
	static void MultiplyScalar(const double& k, const TSpVV& X, TSpVV& Y);
	// y := A * x
	TEMPLATE_TDnsVV
	static void Multiply(const TDnsVV& A, const TDnsV& x, TDnsV& y);
	// C(:, ColId) := A * x
	TEMPLATE_TDnsVV
	static void Multiply(const TDnsVV& A, const TDnsV& x, TDnsVV& C, TSizeTy ColId);
	// y := A * B(:, ColId)
	TEMPLATE_TDnsVV
	static void Multiply(const TDnsVV& A, const TDnsVV& B, int ColId, TDnsV& y);
	// C(:, ColIdC) := A * B(:, ColIdB)
	TEMPLATE_TDnsVV
	static void Multiply(const TDnsVV& A, const TDnsVV& B, int ColIdB, TDnsVV& C, int ColIdC);

	//A is rewritten in place with orthogonal matrix Q
	TEMPLATE_TDnsVV
	static void QRbasis(TDnsVV& A);
	TEMPLATE_TDnsVV
	static void QRbasis(const TDnsVV& A, TDnsVV& Q);
	// Tested in other function
	// A is rewritten in place with orthogonal matrix Q (column pivoting to improve stability);
	TEMPLATE_TDnsVV
	static void QRcolpbasis(TDnsVV& A);
	// TEST
	TEMPLATE_TDnsVV
	static void QRcolpbasis(const TDnsVV& A, TDnsVV& Q);
	// TEST
	//S S option ensures that A is not modified
	TEMPLATE_TDnsVV
	static void thinSVD(const TDnsVV& A, TDnsVV& U, TDnsV& S, TDnsVV& VT);

	// A * x = b
	static void SVDSolve(const TFltVV& A, TFltV& x, const TFltV& b, const double& EpsSing);
	// A = U * diag(Sing) * VT
	static void ComputeSVD(const TFltVV& A, TFltVV& U, TFltV& Sing, TFltVV& VT);
	// A = U * diag(Sing) * V'
	static int ComputeThinSVD(const TMatrix& X, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters = 2, const double Tol = 1e-6);

	typedef enum { DECOMP_SVD } TLinAlgInverseType;

	// TEST (works only for RowMajor, TSvd uses only TFltVV matrices)
	// B = A^(-1)
	TEMPLATE_TDnsVV
	static void Inverse(const TDnsVV& A, TDnsVV& B, const TLinAlgInverseType& DecompType);
	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd);
	TEMPLATE_TDnsVV
	static void InverseSVD(const TDnsVV& A, TDnsVV& B, const double& tol);
	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd);
	TEMPLATE_TDnsVV
	static void InverseSVD(const TDnsVV& A, TDnsVV& B);


	// generalized eigenvalue decomposition A * V(:,j) = lambda(j) * B * V(:,j)
	template <class TType, class TSizeTy, bool ColMajor>
	static void GeneralizedEigDecomp(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
			const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVec<TNum<TType>, TSizeTy>& EigValV,
			TVVec<TNum<TType>, TSizeTy, ColMajor>& V);

	template <class TType, class TSizeTy, bool ColMajor = false>
	inline static void MultiplySF(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, false>& B,
		TVVec<TType, TSizeTy, ColMajor>& C, const TStr& transa = TStr("N"), const int& format = 0);
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyFS(TVVec<TType, TSizeTy, ColMajor>& B, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
		TVVec<TType, TSizeTy, ColMajor>& C);

	// y := A * x
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, TVec<TType, TSizeTy>& y);
	//y := x' * A ... row data!!
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TType, TSizeTy>& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A,
			const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y);

	typedef enum { NOTRANS = 0, TRANS = 1 } TLinAlgBlasTranspose;

	// C = op(A) * op(B)
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>, TSizeTy, ColMajor>& C,
		const int& BlasTransposeFlagA, const int& BlasTransposeFlagB);
	// y := alpha*op(A)*x + beta*y, where op(A) = A -- N, op(A) = A' -- T, op(A) = conj(A') -- C (only for complex)
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>,
			TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA,
			TType alpha = 1.0, TType beta = 0.0);
	// C = A * B
	TEMPLATE_TDnsVV
	static void Multiply(const TDnsVV& A, const TDnsVV& B, TDnsVV& C);
	// C = A' * B
	TEMPLATE_TDnsVV
	static void MultiplyT(const TDnsVV& A, const TDnsVV& B, TDnsVV& C);

	//////////////////
	//  DENSE-SPARSE, SPARSE-DENSE

	// C := A * B
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	// C:= A' * B
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	//#if !defined(INTEL) || defined(INDEX_64);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	// C:= A' * B
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	inline static void Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C);
	// C:= A' * B
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TVec<TKeyDat<IndexType, TType>, TSizeTy>, TSizeTy>& B, TVVec<TType, TSizeTy, ColMajor>& C);
	inline static void Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA = -1);
	inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C);
    inline static void Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TVec<TIntFltKdV>& C);
	inline static void Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA = -1);
    inline static void Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TVec<TIntFltKdV>& C, const int RowsA = -1);
	inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C);
    inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TIntFltKdV& b, TFltV& c);
    template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
    inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TIntFltKdV& b, TFltV& c);
    template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
    inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TType, TSizeTy>& b, TFltV& c);
	typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;

	// D = alpha * A(') * B(') + beta * C(')
	TEMPLATE_TDnsVV
	static void Gemm(const double& Alpha, const TDnsVV& A, const TDnsVV& B, const double& Beta,
		const TDnsVV& C, TDnsVV& D, const int& TransposeFlags);

	// transpose matrix - B = A'
	TEMPLATE_TDnsVV
	static void Transpose(const TDnsVV& A, TDnsVV& B);
	// performes Gram-Schmidt ortogonalization on elements of Q
	template <class TSizeTy = int>
	inline static void GS(TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Q);
	TEMPLATE_TDnsVV
	static void GS(TDnsVV& Q);
	// Modified Gram-Schmidt on columns of matrix Q
	inline static void MGS(TFltVV& Q);
	// QR based on Modified Gram-Schmidt decomposition.
	inline static void QR(const TFltVV& X, TFltVV& Q, TFltVV& R, const TFlt& Tol);
	// rotates vector (OldX,OldY) for angle Angle (in radians!);
	inline static void Rotate(const double& OldX, const double& OldY, const double& Angle, double& NewX, double& NewY);

	// returns the k-th power of the given matrix
	// negative values of k are allowed
	TEMPLATE_TDnsVV
	static void Pow(const TDnsVV& Mat, const int& k, TDnsVV& PowVV);

	// returns a sub matrix of the input matrix in range [StartRow, EndRow) x [StartCol, EndCol)
	TEMPLATE_TDnsVV
	static void SubMat(const TDnsVV& Mat, const TSizeTy& StartRow, const TSizeTy& EndRow,
			const TSizeTy& StartCol, const TSizeTy& EndCol, TDnsVV& SubMat);

	template <class TType, class TVecVal, class TSizeTy, bool ColMajor>
	static void SubMat(const TVVec<TType, TSizeTy, ColMajor>& Mat, const TVec<TVecVal, TSizeTy>& ColIdxV,
			TVVec<TType, TSizeTy, ColMajor>& SubMat);

	// returns the sum of diagonal elements of matrix
	TEMPLATE_TDnsVV
	static TType Trace(const TDnsVV& Mat);

	//===========================================================
	// PROJECTIONS
	//===========================================================

	static void NonNegProj(TFltV& Vec);
	static void NonNegProj(TFltVV& Mat);
};

#ifdef LAPACKE
#include "MKLfunctions.h"
#endif

//////////////////////////////////////////////////////////////////////
// Numerical-Recipes-Exception
class TNSException : public TExcept {
public:
	TStr Message;
public:
	TNSException(const TStr& Msg) : TExcept(Msg) {}
	TNSException(const TStr& MsgStr, const TStr& LocStr) : TExcept(MsgStr, LocStr) { }
	/// Create new numerical exception
	static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr()) {
		return PExcept(new TNSException(MsgStr, LocStr));
	}
};

//////////////////////////////////////////////////////////////////////
// Numerical-Linear-Algebra (copied from Numerical Recepies)
class TNumericalStuff {
private:
	static double sqr(double a);
	static double sign(double a, double b);

	// Computes (a^2 + b^2)^(1/2) without
	// destructive underflow or overflow.
	static double pythag(double a, double b);

	//displays error message to screen
	static void nrerror(const TStr& error_text);

public:
	// Householder reduction of a real, symmetric matrix a[1..n][1..n].
	// On output, a is replaced by the orthogonal matrix Q eecting the
	// transformation. d[1..n] returns the diagonal elements of the
	// tridiagonal matrix, and e[1..n] the o-diagonal elements, with
	// e[1]=0. Several statements, as noted in comments, can be omitted
	// if only eigenvalues are to be found, in which case a contains no
	// useful information on output. Otherwise they are to be included.
	static void SymetricToTridiag(TFltVV& a, int n, TFltV& d, TFltV& e);

	// QL algorithm with implicit shifts, to determine the eigenvalues
	// and eigenvectors of a real, symmetric, tridiagonal matrix, or of
	// a real, symmetric matrix previously reduced by tred2 x11.2. On
	// input, d[1..n] contains the diagonal elements of the tridiagonal
	// matrix. On output, it returns the eigenvalues. The vector e[1..n]
	// inputs the subdiagonal elements of the tridiagonal matrix, with
	// e[1] arbitrary. On output e is destroyed. When finding only the
	// eigenvalues, several lines may be omitted, as noted in the comments.
	// If the eigenvectors of a tridiagonal matrix are desired, the matrix
	// z[1..n][1..n] is input as the identity matrix. If the eigenvectors
	// of a matrix that has been reduced by tred2 are required, then z is
	// input as the matrix output by tred2. In either case, the kth column
	// of z returns the normalized eigenvector corresponding to d[k].
	static void EigSymmetricTridiag(TFltV& d, TFltV& e, int n, TFltVV& z);

	// Given a positive-dedinite symmetric matrix A(n,n), this routine
	// constructs its Cholesky decomposition, A = L * L^T . On input, only
	// the upper triangle of A need be given; it is not modified. The
	// Cholesky factor L is returned in the lower triangle of A, except for
	// its diagonal elements which are returned in p(n).
	static void CholeskyDecomposition(TFltVV& A, TFltV& p);

	// Solves the set of n linear equations A * x = b, where A is a
	// positive-definite symmetric matrix. A(n,n) and p[1..n] are input
	// as the output of the routine choldc. Only the lower triangle of A
	// is accessed. b(n) is input as the right-hand side vector. The
	// solution vector is returned in x(n). A  and p are not modified and
	// can be left in place for successive calls with diferent right-hand
	// sides b. b is not modified unless you identify b and x in the calling
	// sequence, which is allowed.
	static void CholeskySolve(const TFltVV& A, const TFltV& p, const TFltV& b, TFltV& x);

	// Solves system of linear equations A * x = b, where A is symetric
	// positive-definite matrix. A is first decomposed using
	// CholeskyDecomposition and after solved using CholeskySolve. Only
	// upper triangle of A need be given and it is not modified. However,
	// lower triangle is modified!
	static void SolveSymetricSystem(TFltVV& A, const TFltV& b, TFltV& x);

    // solve system A x_i = e_i for i = 1..n, where A and p are output
    // from CholeskyDecomposition. Result is stored to upper triangule
	// (possible since inverse of symetric matrix is also symetric! Sigh...)
	static void InverseSubstitute(TFltVV& A, const TFltV& p);

	// Calculates inverse of symetric positiv definit matrix
	// Matrix is given as upper triangule of A, result is stored
	// in upper triangule of A. Lower triangule is random (actually
	// it has part of Choleksy decompositon of A)
	static void InverseSymetric(TFltVV& A);

	// calcualtes inverse of upper triagonal matrix A
	// lower triangle is messed up...
	static void InverseTriagonal(TFltVV& A);

	// Given a matrix a[1..n][1..n], this routine replaces it by the LU
	// decomposition of a rowwise permutation of itself. a and n are input.
	// a is output, arranged as in equation (2.3.14) above; indx[1..n] is
	// an output vector that records the row permutation efected by the partial
	// pivoting; d is output as +-1 depending on whether the number of row
	// interchanges was even or odd, respectively. This routine is used in
	// combination with lubksb to solve linear equations or invert a matrix.
	static void LUDecomposition(TFltVV& A, TIntV& indx, double& d);

	// Solves the set of n linear equations A*X = B. Here a[1..n][1..n] is input,
	// not as the matrix A but rather as its LU decomposition, determined by the
	// routine ludcmp. indx[1..n] is input as the permutation vector returned by
	// ludcmp. b[1..n] is input as the right-hand side vector B, and returns with
	// the solution vector X. a, n, and indx are not modified by this routine and
	// can be left in place for successive calls with diferent right-hand sides b.
	// This routine takes into account the possibility that b will begin with many
	// zero elements, so it is efficient for use in matrix inversion.
	static void LUSolve(const TFltVV& A, const TIntV& indx, TFltV& b);


	// Finds x[1...f] that minimizes ||A' x - y||^2 + ||Gamma x||^2, where A[1...f][1...n]
	// is  a matrix with column training examples (rows = features) and y[1...n] is a
	// vector of targets. 
	// Solves the primal problem if the number of features is lower than the number of examples,
	// or the dual problem in the other case.
	//Paramter Gamma controls overfitting (large values force models to be simpler)
	// See http://en.wikipedia.org/wiki/Tikhonov_regularization, where the regularization matrix = Gamma*I
	static void LeastSquares(const TFltVV& A, const TFltV& b, const double& kappa, TFltV& x);

	// Finds x[1...f] that minimizes ||A' x - y||^2 + ||Gamma x||^2, where A[1...f][1...n]
	// is  a matrix with column training examples (rows = features) and y[1...n] is a
	// vector of targets. Paramter Gamma controls overfitting (large values force models to be simpler)
	// See http://en.wikipedia.org/wiki/Tikhonov_regularization, where the regularization matrix = Gamma*I
	static void PrimalLeastSquares(const TFltVV& A, const TFltV& b, const double& kappa, TFltV& x);


	// Finds x[1...f] that minimizes ||A' x - y||^2 + ||Gamma x||^2, where A[1...f][1...n]
	// is  a matrix with column training examples (rows = features) and y[1...n] is a
	// vector of targets. Solves the dual version of the problem and exresses it in the
	// original coordinates in the end - suitable for cases, where the number of examples
	// is larger than the number of features.
	// Paramter Gamma controls overfitting (large values force models to be simpler)
	// See http://en.wikipedia.org/wiki/Tikhonov_regularization, where the regularization matrix = Gamma*I
	static void DualLeastSquares(const TFltVV& A, const TFltV& b, const double& kappa, TFltV& x);

	// Solves system of linear equations A * x = b. A is first decomposed using
	// LUDecomposition and after solved using LUSolve. A is modified!
	static void SolveLinearSystem(TFltVV& A, const TFltV& b, TFltV& x);

	// Computes the eigenvector of A belonging to the specified eigenvalue
    // uses the inverse iteration algorithm
    // the algorithms does modify A due to its use of LU decomposition
    static void GetEigenVec(const TFltVV& A, const double& EigenVal, TFltV& EigenV,
    		const double& ConvergEps=1e-7);
};

///////////////////////////////////////////////////////////////////////
// Sparse-SVD
//   Calculates singular-value-decompositon for sparse matrixes.
//   If A is a matrix than A is decomposed to A = U S V'
//   where S is diagonal with singular values on diagonal and U
//   and V are ortogonal (U'*U = V'*V = I).
typedef enum { ssotNoOrto, ssotSelective, ssotFull } TSpSVDReOrtoType;
class TSparseSVD {
private:
	// Result = Matrix' * Matrix * Vec(:,ColId)
	static void MultiplyATA(const TMatrix& Matrix,
		const TFltVV& Vec, int ColId, TFltV& Result);
	// Result = Matrix' * Matrix * Vec
	static void MultiplyATA(const TMatrix& Matrix,
		const TFltV& Vec, TFltV& Result);
public:
	// calculates NumEig eigen values of symetric matrix
	// if SvdMatrixProductP than matrix Matrix'*Matrix is used
	static void SimpleLanczos(const TMatrix& Matrix,
		const int& NumEig, TFltV& EigValV,
		const bool& DoLocalReortoP = false,
		const bool& SvdMatrixProductP = false);
	// fast, calculates NumEig largers eigen values and vectors
	// kk should be something like 4*NumEig
	// if SvdMatrixProductP than matrix Matrix'*Matrix is used
	static void Lanczos(const TMatrix& Matrix,
		int NumEig, int Iters, const TSpSVDReOrtoType& ReOrtoType,
		TFltV& EigValV, TFltVV& EigVecVV,
		const bool& SvdMatrixProductP = false);
	static void Lanczos2(const TMatrix& Matrix,
		int MaxNumEig, int MaxSecs, const TSpSVDReOrtoType& ReOrtoType,
		TFltV& EigValV, TFltVV& EigVecVV,
		const bool& SvdMatrixProductP = false);

	// calculates only singular values (based on SimpleLanczos)
	static void SimpleLanczosSVD(const TMatrix& Matrix,
		const int& CalcSV, TFltV& SngValV,
		const bool& DoLocalReortoP = false);
	// fast, calculates NumSV largers SV (based on Lanczos)
	static void LanczosSVD(const TMatrix& Matrix,
		int NumSV, int Iters, const TSpSVDReOrtoType& ReOrtoType,
		TFltV& SgnValV, TFltVV& LeftSgnVecVV, TFltVV& RightSgnVecVV);

	// slow - ortogonal iteration
	static void OrtoIterSVD(const TMatrix& Matrix, int NumSV, int IterN, TFltV& SgnValV);
	// slow - ortogonal iteration
	static void OrtoIterSVD(const TMatrix& Matrix, const int k, TFltV& S, TFltVV& U,
		TFltVV& V, const int Iters = 100, const double Tol = 1e-6);

	// projects sparse vector to space spanned by columns of matrix U
	static void Project(const TIntFltKdV& Vec, const TFltVV& U, TFltV& ProjVec);
};

//////////////////////////////////////////////////////////////////////
// Sigmoid  --  made by Janez(TM)
//  (y = 1/[1 + exp[-Ax+B]])
class TSigmoid {
private:
	TFlt A;
	TFlt B;
private:
	// Evaluates how well the sigmoid function fits the data.
	// J(A, B) = - ln prod_i P(Y = y_i | Z = z_i).  The 'data' parameter
	// should contain (z_i, y_i) pairs.  Smaller J means a better fit.
	static double EvaluateFit(const TFltIntKdV& data, const double A, const double B);
	// Computes not only J but also its partial derivatives.
	static void EvaluateFit(const TFltIntKdV& data, const double A,
		const double B, double& J, double& JA, double& JB);
	// Let J(lambda) = J(A + lambda U, B + lambda V).
	// This function computes J and its first and second derivatives.
	// They can be used to choose a good lambda (using Newton's method)
	// when minimizing J. -- This method has not been tested yet.
	static void EvaluateFit(const TFltIntKdV& data, const double A,
		const double B, const double U, const double V, const double lambda,
		double& J, double& JJ, double& JJJ);
public:
	TSigmoid() { };
	TSigmoid(const double& A_, const double& B_) : A(A_), B(B_) { };
	// Tries to find a pair (A, B) that minimizes J(A, B).
	// Uses gradient descent.
	TSigmoid(const TFltIntKdV& data);

	TSigmoid(TSIn& SIn) { A.Load(SIn); B.Load(SIn); }
	void Load(TSIn& SIn) { A.Load(SIn); B.Load(SIn); }
	void Save(TSOut& SOut) const { A.Save(SOut); B.Save(SOut); }

	double GetVal(const double& x) const {
		return 1.0 / (1.0 + exp(-A * x + B));
	}
	double operator()(const double& x) const {
		return GetVal(x);
	}

	void GetSigmoidAB(double& A_, double& B_) { A_ = A; B_ = B; }
};

class TFullMatrix;

/////////////////////////////////////////////////////////////////////////
//// Full-Vector
class TVector {
	friend class TFullMatrix;
public:
	bool IsColVector;
	TFltV Vec;

public:
	TVector(const bool& IsColVector = true);
	TVector(const int& Dim, const bool IsColVector = true);
	TVector(const TFltV& Vect, const bool IsColVector = true);
	TVector(const TIntV& Vect, const bool IsColVector = true);
	TVector(const TFullMatrix& Mat);

    // copy constructor
    TVector(const TVector& Vector);
#ifdef GLib_CPP11
    // Move constructor
    TVector(const TVector&& Vector);
#endif
    // Move assignment
    TVector& operator=(TVector Vector);

	// returns a new zero vector
	static TVector Init(const int& Dim, const bool _IsColVect);
	// returns a vector of ones
	static TVector Ones(const int& Dim, const bool IsColVect = true);
	// returns a vector of zeros
	static TVector Zeros(const int& Dim, const bool IsColVec = true);
	// returns a vector with a sequence starting with Start (inclusive) and ending
	// with End (exclusive)
	static TVector Range(const int& Start, const int& End, const bool IsColVect = true);
	// returns a vector with a sequence starting with 0 (inclusive) and ending
	// with End (exclusive)
	static TVector Range(const int& End, const bool IsColVect = true);

	void Add(const double& Val) { Vec.Add(Val); }
	void DelLast() { Vec.DelLast(); }

	// returns true if the vectors have the same orientation and the elements are the same
	bool operator ==(const TVector& Vect) const;
	// returns the element at index Idx
	TFlt& operator [](const int& Idx) { return Vec[Idx]; }
	const TFlt& operator [](const int& Idx) const { return Vec[Idx]; }

	TVector GetT() const;
	TVector& Transpose();

	double DotProduct(const TFltV& y) const;
	double DotProduct(const TVector& y) const;

	// multiplication
	TFullMatrix operator *(const TVector& y) const;
	TVector operator *(const TFullMatrix& Mat) const;
	TVector operator *(const double& k) const;
	// multiplies all elements by Lambda
	TVector& operator *=(const double& Lambda);

	// division
	// divides all elements by Lambda
	TVector operator /(const double& Lambda) const;
	// divides all elements by Lambda
	TVector& operator /=(const double& Lambda);

	// multiply the transpose of this vector with B (e.g. x'*B)
	TVector MulT(const TFullMatrix& B) const;

	// addition
	TVector operator +(const TVector& y) const;
	TVector& operator +=(const TVector& y);

	// subtraction
	TVector operator -(const TVector& y) const;

public:
	int Len() const { return Vec.Len(); }
	bool IsColVec() const { return IsColVector; }
	bool IsRowVec() const { return !IsColVec(); }
	bool Empty() const { return Vec.Empty(); }

	template<typename TFunc> TVector& Map(const TFunc& Func);
	// applies sqrt on all elements of this matrix
	TVector& Sqrt() { return Map([](TFlt Val) { return sqrt(Val); }); }

	// returns a vector containing indexes of all the elements satisfying a condition
	template<typename TFunc> TVector Find(const TFunc& Func) const;
	template<typename TFunc, typename TRes> void Find(const TFunc& Func, TRes& Res) const;

	// returns the 'euclidian' L2 norm
	double Norm() const;
	// returns the squared 'euclidian' L2 norm
	double Norm2() const;
	// returns the sum of elements
	double Sum() const;

	// returns the euclidean distance to the other vector
	double EuclDist(const TVector& y) const;

	// returns the underlying list
	const TFltV& GetVec() const { return Vec; }
	// returns the underlying list
	TFltV& GetVec() { return Vec; }
	// returns this vector as a list of integers
	TIntV GetIntVec() const;

	double GetMaxVal() const;
	// returns the index of the maximum element
	int GetMaxIdx() const;
	// returns the index and value of the maximum element
	TIntFltPr GetMax() const;

	// returns the index of the minimum element
	int GetMinIdx() const;

	void Save(TSOut& SOut) const { TBool(IsColVector).Save(SOut); Vec.Save(SOut); }
	void Load(TSIn& SIn) { IsColVector = TBool(SIn); Vec.Load(SIn); }
};

template <typename TFunc>
TVector& TVector::Map(const TFunc& Func) {
	const int& Dim = Len();

	for (int i = 0; i < Dim; i++) {
		Vec[i] = Func(Vec[i]);
	}

	return *this;
}

template <typename TFunc>
TVector TVector::Find(const TFunc& Func) const {
	TVector Res; Find(Func, Res);
	return Res;
}

template <typename TFunc, typename TRes>
void TVector::Find(const TFunc& Func, TRes& Res) const {
	const int& Dim = Len();

	for (int i = 0; i < Dim; i++) {
		if (Func(Vec[i])) {
			Res.Add(i);
		}
	}
}

/////////////////////////////////////////////////////////////////////////
//// Full-Matrix
typedef TTriple<TFullMatrix, TFullMatrix, TFullMatrix> TFullMatrixTr;
typedef TTriple<TFullMatrix, TVector, TFullMatrix> TMatVecMatTr;

class TFullMatrix : public TMatrix {
	friend class TVector;
private:
	bool IsWrapper;
	TFltVV* Mat;

public:
	// constructors/destructors
	// empty matrix with 0 rows and 0 cols
	TFullMatrix();
	// zero matrix with the specified number of rows and cols
	TFullMatrix(const int& Rows, const int& Cols);
	// matrix from TFltVV, if IsWrapper is set to true then the
	// underlying matrix will not be deleted
	TFullMatrix(TFltVV& Mat, const bool IsWrapper);
	TFullMatrix(const TFltVV& Mat);
	// matrix from vector
	TFullMatrix(const TVector& Vec);
	// copy constructor
	TFullMatrix(const TFullMatrix& Mat);
#ifdef GLib_CPP11
	// move constructor
	TFullMatrix(TFullMatrix&& Mat);
#endif

private:
	// wraps the matrix and takes control of all the cleanup
	TFullMatrix(TFltVV* Mat);

public:
    // destructor
    virtual ~TFullMatrix();

	// copy constructor
	TFullMatrix& operator =(const TFullMatrix& Mat);
	// move constructor
	TFullMatrix& operator =(TFullMatrix&& _Mat);

	// identity matrix
	static TFullMatrix Identity(const int& Dim);
	// matrix from TVec<TFltV>, each element from the list goes into one row
	static TFullMatrix RowMatrix(const TVec<TFltV>& Mat);
	// matrix from TVec<TFltV>, each element from the list goes into one column
	static TFullMatrix ColMatrix(const TVec<TFltV>& Mat);
	// get a matrix with the values from the vector are diagonal elements
	static TFullMatrix Diag(const TVector& Diag);

private:
	void Clr();

protected:
	virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
	virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
	virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
	virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const;
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const;

	// getters
	virtual int PGetRows() const { return Mat->GetRows(); }
	virtual int PGetCols() const { return Mat->GetCols(); }
public:
	// returns the underlying TFltVV
	const TFltVV& GetMat() const { return *Mat; }
	// returns the underlying TFltVV
	TFltVV& GetMat() { return *Mat; }
	// transposed
	virtual void Transpose();
	// returns the transpose of this matrix
	TFullMatrix GetT() const;
	void GetT(TFltVV& TransposedVV) const;
	// returns the value at position (i,j)
	TFlt& At(const int& i, const int& j) { return Mat->operator ()(i, j); }
	const TFlt& At(const int& i, const int& j) const { return Mat->operator ()(i, j); }
	// sets the value at position (i,j)
	void Set(const double& Val, const int& i, const int& j) { Mat->operator ()(i, j) = Val; }
	// returns true if the matrix is empty
	bool Empty() const { return Mat->Empty(); }

	TFullMatrix& AddCol(const TFltV& Col);
	TFullMatrix& AddCol(const TVector& Col);
	TFullMatrix& AddCols(const TFullMatrix& Cols);

    // operators
    TFlt& operator ()(const int& i, const int& j) { return At(i,j); }
    const TFlt& operator ()(const int& i, const int& j) const { return At(i,j); }
    // returns a submatrix specified by RowV and ColV
    template<class TIdxV1, class TIdxV2>
    TFullMatrix operator ()(const TIdxV1& RowV, const TIdxV2& ColV) const;
    template<class TIdxV>
    TVector operator ()(const int& RowIdx, const TIdxV& ColV) const;
    
    // adds matrix B and returns itself
    TFullMatrix& operator +=(const TFullMatrix& B);
    // subtracts matrix B and returns itself
    TFullMatrix& operator -=(const TFullMatrix& B);
    
    // add/subtract
    TFullMatrix operator +(const TFullMatrix& B) const;
    TFullMatrix operator -(const TFullMatrix& B) const;
    
    // multiply
    TFullMatrix operator *(const TFullMatrix& B) const;
    TFullMatrix operator *(const TSparseColMatrix& B) const;
    // multiply the transpose of this matrix with B (e.g. A'*B)
    TFullMatrix MulT(const TFullMatrix& B) const;
	TFullMatrix MulT(const TFltVV& B) const;
    // multiplies this matrix with a vector
    TVector operator *(const TVector& x) const;
    // multiplies this matrix with a vector represented as TFltV
    // ignores the vectors orientation
    TVector operator *(const TFltV& x) const;

    // scalars
    // multiplies this matrix by a scalar and returns the result
    TFullMatrix operator *(const double& Lambda) const;
    // divides this matrix by a scalar and returns the result
    TFullMatrix operator /(const double& Lambda) const;

    // returns the power of this matrix A^n where A is this matrix and n is the argument
    TFullMatrix Pow(const int& k) const;
    TFullMatrix operator ^(const int& k) const { return Pow(k); };

    // returns the RowIdx-th row
    TVector GetRow(const int& RowIdx) const;
    // returns the ColIdx-th column
    TVector GetCol(const int& ColIdx) const;
    
    void SetRow(const int& RowIdx, const TVector& RowV);
    void SetCol(const int& ColIdx, const TVector& ColV);

    // applies an element-wise operation on this matrix and returns the matrix itself
    template<typename TFunc> TFullMatrix& Map(const TFunc& Func);
    // applies sqrt on all elements of this matrix
    TFullMatrix& Sqrt() { return Map([](TFlt Val) { return sqrt(Val); }); }

    // returns the L2 norm of the specified column
    double ColNorm(const int& ColIdx) const;
    // returns the squared L2 norm of the specified column
    double ColNorm2(const int& ColIdx) const;
    // returns the L2 norm of each column and returns them in a row vector
    TVector ColNormV() const;
    // returns the squared L2 norm of each column and returns them in a row vector
    TVector ColNorm2V() const;
    // returns the Frobenius norm of this matrix
    double FromNorm() const;

    // returns the norm of the i-th row
    double RowNormL1(const int& i) const;
    // normalizes the rows using L1 norm
    void NormalizeRowsL1();

    // returns the sum of the i-th row
    double RowSum(const int& i) const;
    // returns a vector containing the sum of rows
    TVector RowSumV() const;

    // returns a vector containing the minimum values of each column
    TVector GetColMinV() const;

    // returns the index of the maximum element in each column in a row vector
    TVector GetColMaxIdxV() const;
    // returns the index of the minimum element in each column in a row vector
	TVector GetColMinIdxV() const;

	// transforms the rows of the matrix to have mean 0
	TFullMatrix& CenterRows();
	// returns a matrix which has rows centered around zero (check CenterRows)
	TFullMatrix GetCenteredRows() const;

	// computes the singular value decomposition if this matrix X = U*S*V'
	// returns a triple where U is stored in the first value, S is stored as a vector
	// in the second value and V is stored in the third value
	// k represents the number of singular values that are computed
	TMatVecMatTr Svd(const int& k) const;
	TMatVecMatTr Svd() const { return Svd(TMath::Mn(GetRows(), GetCols())); }

	// returns the inverse of this matrix
	TFullMatrix GetInverse() const;

	bool HasNan() const;

public:
	void Save(TSOut& SOut) const;
	void Load(TSIn& SIn);
};

#include "linalg.hpp"

#undef TEMPLATE_TDnsV
#undef TEMPLATE_TDnsVV
#undef TEMPLATE_TSpV

#undef TDnsV
#undef TDnsVV
#undef TSpV
#undef TSpVV

#endif

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
#ifdef AMD
#include "acml.h"
#elif INTEL 
#define MKL_Complex8 std::complex<float>
#define MKL_Complex16 std::complex<double>
#undef small
#include "mkl.h"
//#include "mkl_scalapack.h"
#else
#define LAPACK_COMPLEX_CPP
#include "cblas.h"
#ifdef LAPACKE
#include "lapacke.h"		
#endif
#endif
#endif
namespace TypeCheck{
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
	struct is_complex_float
	{
		static const bool value = false;
	};

	template<>
	struct is_complex_float< std::complex<float> >
	{
		static const bool value = true;
	};

	template<>
	struct is_complex_float< TNum<std::complex<float>> >
	{
		static const bool value = true;
	};

	template<typename T1>
	struct is_complex_double
	{
		static const bool value = false;
	};

	template<>
	struct is_complex_double< std::complex<double> >
	{
		static const bool value = true;
	};

	template<>
	struct is_complex_double< TNum<std::complex<double>> >
	{
		static const bool value = true;
	};
}

///////////////////////////////////////////////////////////////////////
// forward declarations
class TLinAlg;
//////////////////////////////////////////////////////////////////////
// Useful stuff (hopefully)
class TLAMisc {
public:
	//Sort double array
#ifdef SCALAPACK 
	template<class TSizeTy>
	static void Sort(TVec<TFlt, TSizeTy> & Vec, TVec<TSizeTy, TSizeTy>& index, const TBool& decrease) {
		if (index.Empty()){
			TLAMisc::FillRange(Vec.Len(), index);
		}
		char* id = decrease ? "D" : "I";
		TSizeTy n = Vec.Len();
		TSizeTy info;
		dlasrt2(id, &n, &Vec[0].Val, &index[0], &info);
		//dlasrt2(id, n, d, key, info)
	}
#endif
	// Dumps vector to file so Excel can read it
	static void SaveCsvTFltV(const TFltV& Vec, TSOut& SOut);
	// Dumps sparse vector to file so Matlab can read it
	static void SaveMatlabTFltIntKdV(const TIntFltKdV& SpV, const int& ColN, TSOut& SOut);
	// Dumps sparse matrix to file so Matlab can read it
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
	// fills vector with random numbers
	static void FillRnd(TFltV& Vec) { TRnd Rnd(0); FillRnd(Vec.Len(), Vec, Rnd); }
	static void FillRnd(TFltV& Vec, TRnd& Rnd) { FillRnd(Vec.Len(), Vec, Rnd); }
	static void FillRnd(TFltVV& Mat) { TRnd Rnd(0); FillRnd(Mat, Rnd); }
	static void FillRnd(TFltVV& Mat, TRnd& Rnd) { FillRnd(Mat.Get1DVec(), Rnd); }
	static void FillRnd(const int& Len, TFltV& Vec, TRnd& Rnd);
	// set all components
	static void Fill(TFltVV& M, const double& Val);
	static void Fill(TFltV& M, const double& Val);
	// sets all compnents to zero
	static void FillZero(TFltV& Vec) { Vec.PutAll(0.0); }
	static void FillZero(TFltVV& M) { Fill(M, 0.0); }
	// set matrix to identity
	static void FillIdentity(TFltVV& M);
	static void FillIdentity(TFltVV& M, const double& Elt);
	// set vector to range
	static void FillRange(const int& Vals, TFltV& Vec);
	static void FillRange(const int& Vals, TIntV& Vec);

	template <class TVal, class TTSizeTyTy = int>
	static void FillRangeS(const TTSizeTyTy& Vals, TVec<TVal, TTSizeTyTy>& Vec) {
		//Added by Andrej
		if (Vec.Len() != Vals){
			Vec.Gen(Vals);
		}
		for (int i = 0; i < Vals; i++){
			Vec[i] = i;
		}
	};
	// sums elements in vector
	static int SumVec(const TIntV& Vec);
	static double SumVec(const TFltV& Vec);
	// converts full vector to sparse
	static void ToSpVec(const TFltV& Vec, TIntFltKdV& SpVec,
		const double& CutWordWgtSumPrc = 0.0);
	// converts sparse vector to full
	static void ToVec(const TIntFltKdV& SpVec, TFltV& Vec, const int& VecLen);
	// creates a diagonal matrix
	static void Diag(const TFltV& Vec, TFltVV& Mat);
	// creates a diagonal matrix
	static void Diag(const TFltV& Vec, TVec<TIntFltKdV>& Mat);
	// gets the maximal index of a sparse vector
	static int GetMaxDimIdx(const TIntFltKdV& SpVec);
	// gets the maximal row index of a sparse column matrix
	static int GetMaxDimIdx(const TVec<TIntFltKdV>& SpMat);
	// returns a vector with a sequence starting at Min and ending at Max
	static void RangeV(const int& Min, const int& Max, TIntV& Res);
	// returns the mean value of Vec.
	static double Mean(const TFltV& Vec);
	// returns the mean value along the dimension (Dim) of Mat. See Matlab documentation - mean().
	static void Mean(const TFltVV& Mat, TFltV& Vec, const int& Dim = 1);
	// returns standard deviation. See Matlab documentation - std().
	static void Std(const TFltVV& Mat, TFltV& Vec, const int& Flag = 0, const int& Dim = 1);
	// returns the z-score for each element of X such that columns of X are centered to have mean 0 and scaled to have standard deviation 1.
	static void ZScore(const TFltVV& Mat, TFltVV& Vec, const int& Flag = 0, const int& Dim = 1);
};
//////////////////////////////////////////////////////////////////////
// Template-ised Sparse Operations
template <class TKey, class TDat>
class TSparseOps {
private:
	typedef TVec<TKeyDat<TKey, TDat> > TKeyDatV;
public:
	static void CoordinateCreateSparseColMatrix(const TVec<TKey>& RowIdxV, const TVec<TKey>& ColIdxV, const TVec<TDat>& ValV, TVec<TKeyDatV>& ColMatrix, const TKey& Cols) {
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
	static void SparseMerge(const TKeyDatV& SrcV1, const TKeyDatV& SrcV2, TKeyDatV& DstV) {
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
	static void SparseLinComb(const double& p, const TKeyDatV& SrcV1, const double& q, const TKeyDatV& SrcV2, TKeyDatV& DstV) {
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
	int RowN, ColN;
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

	int PGetRows() const { return RowN; }
	int PGetCols() const { return ColN; }

public:
	TSparseColMatrix() : TMatrix() {}
	TSparseColMatrix(const int& _RowN, const int& _ColN) : RowN(_RowN), ColN(_ColN), ColSpVV() {}
	TSparseColMatrix(TVec<TIntFltKdV> _ColSpVV) : TMatrix(), ColSpVV(_ColSpVV) {}
	TSparseColMatrix(TVec<TIntFltKdV> _ColSpVV, const int& _RowN, const int& _ColN) :
		TMatrix(), RowN(_RowN), ColN(_ColN), ColSpVV(_ColSpVV) {}

	void Save(TSOut& SOut) {
		SOut.Save(RowN); SOut.Save(ColN); ColSpVV.Save(SOut);
	}
	void Load(TSIn& SIn) {
		SIn.Load(RowN); SIn.Load(ColN); ColSpVV = TVec<TIntFltKdV>(SIn);
	}
};

///////////////////////////////////////////////////////////////////////
// Sparse-Row-Matrix
//  matrix is given with rows as sparse vectors
class TSparseRowMatrix : public TMatrix {
public:
	// number of rows and columns of matrix
	int RowN, ColN;
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

	int PGetRows() const { return RowN; }
	int PGetCols() const { return ColN; }

public:
	TSparseRowMatrix() : TMatrix() {}
	TSparseRowMatrix(TVec<TIntFltKdV> _RowSpVV) : TMatrix(), RowSpVV(_RowSpVV) {}
	TSparseRowMatrix(TVec<TIntFltKdV> _RowSpVV, const int& _RowN, const int& _ColN) :
		TMatrix(), RowN(_RowN), ColN(_ColN), RowSpVV(_RowSpVV) {}
	// loads Matlab sparse matrix format: row, column, value.
	//   Indexes start with 1.
	TSparseRowMatrix(const TStr& MatlabMatrixFNm);
	void Save(TSOut& SOut) {
		SOut.Save(RowN); SOut.Save(ColN); RowSpVV.Save(SOut);
	}
	void Load(TSIn& SIn) {
		SIn.Load(RowN); SIn.Load(ColN); RowSpVV = TVec<TIntFltKdV>(SIn);
	}
};

///////////////////////////////////////////////////////////////////////
// Full-Col-Matrix
//  matrix is given with columns of full vectors
class TFullColMatrix : public TMatrix {
public:
	// number of rows and columns of matrix
	int RowN, ColN;
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
	TFullColMatrix() : TMatrix() {}
	// loads matrix saved in matlab with command:
	//  save -ascii Matrix.dat M
	TFullColMatrix(const TStr& MatlabMatrixFNm);
	TFullColMatrix(TVec<TFltV>& RowVV);
	void Save(TSOut& SOut) { SOut.Save(RowN); SOut.Save(ColN);  ColV.Save(SOut); }
	void Load(TSIn& SIn) { SIn.Load(RowN); SIn.Load(ColN); ColV.Load(SIn); }
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
class TLinAlg {
public:
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y);
	inline static double DotProduct(const TVec<TFltV>& X, int ColId, const TFltV& y);
	inline static double DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TFltV& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
		int ColIdX, const TVVec<TType, TSizeTy, ColMajor>& Y, int ColIdY);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
		int ColId, const TVec<TType, TSizeTy>& Vec);
	inline static double DotProduct(const TIntFltKdV& x, const TIntFltKdV& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TIntFltKd>& y);
	// <X(:,ColId),y> where only y is sparse
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, const TIntFltKdV& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void OuterProduct(const TVec<TType, TSizeTy>& x,
		const TVec<TType, TSizeTy>& y, TVVec<TType, TSizeTy, ColMajor>& Z);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void LinComb(const double& p, const TVec<TType, TSizeTy>& x,
		const double& q, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z);
	//TODO this will work only for glib type TFlt
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void LinCombInPlace(const TType& alpha, const TVec<TNum<TType>, TSizeTy>& x,
		const TType& beta, TVec<TNum<TType>, TSizeTy>& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void LinComb(const double& p, const TVVec<TType, TSizeTy, ColMajor>& X,
		const double& q, const TVVec<TType, TSizeTy, ColMajor>& Y, TVVec<TType, TSizeTy, ColMajor>& Z);
	// z = p * x + q * y
	inline static void LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z);
	inline static void LinComb(const double& p, const TFltVV& X, int ColId,
		const double& q, const TFltV& y, TFltV& z);
	inline static void LinComb(const double& p, const TFltVV& X, int DimId,
		const double& q, const TFltV& y, TFltV& z, int Dim);
	inline static void LinComb(const double& p, const TFltVV& X, const double& q, const TFltVV& Y, TFltVV& Z);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void ConvexComb(const double& p, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z);
	//this will fail if TType != TFlt, Specialization should be used
#ifdef BLAS
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const TType& k, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y);
#endif
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const double& k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z);
	inline static void AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z);
	inline static void AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z);
	// z := x + y
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z);
	inline static void AddVec(const double& k, const TIntFltKdV& x, const TFltV& y, TFltV& z);
	// z := k * X[ColId] + y
	inline static void AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z);
	inline static void AddVec(const double& k, const TIntFltKdV& x, TFltV& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, TSizeTy ColIdX, TVVec<TType, TSizeTy, ColMajor>& Y, TSizeTy ColIdY);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const double& k, const TVec<TType, TSizeTy>& x, TVVec<TType, TSizeTy, ColMajor>& Y, const TSizeTy& ColIdY);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, TVec<TType, TSizeTy>& Result);
	// z = x + y
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z);
	template <class TType, class TSizeTy = int>
	inline static double SumVec(const TVec<TType, TSizeTy>& x);
	inline static double SumVec(const TIntFltKdV& x);
	template <class TType, class TSizeTy = int>
	inline static double SumVec(double k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y);
	// Result = ||x-y||^2 (Euclidian);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double EuclDist2(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y);
	// Result = ||x-y||^2 (Euclidian);
	inline static double EuclDist2(const TFltPr& x, const TFltPr& y);
	template <class TType, class TSizeTy = int>
	inline static double EuclDist(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y);
	inline static double EuclDist(const TFltPr& x, const TFltPr& y);
	// Result = ||A||_F (Frobenious);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static TType Frob(const TVVec<TNum<TType>, TSizeTy, ColMajor> &A);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double FrobDist2(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double FrobDist2(const TVec<TType, TSizeTy>& A, const TVec<TType, TSizeTy>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TIntFltKdV>& B);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Full(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A, TVVec<TType, TSizeTy, ColMajor>& B, const int Rows, const int Cols);
	// Sparse to dense transform
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Full(const TVec<TIntFltKdV, TSizeTy>& A, TVVec<TType, TSizeTy, ColMajor>& B, TSizeTy Rows);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Transpose(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
		TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& At);
	inline static void Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows = -1);
	// Sign
	inline static void Sign(const TVec<TIntFltKdV>& Mat, TVec<TIntFltKdV>& Mat2);
	inline static void Convert(const TVec<TPair<TIntV, TFltV>>& A, TTriple<TIntV, TIntV, TFltV>& B);
	inline static void Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>&B);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Sum(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TType, TSizeTy>& y, const int Dimension = 1);
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void Sum(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X, TVec<TType, TSizeTy>& y, const int Dimension = 1);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double Norm2(const TVec<TType, TSizeTy>& x);
	// ||x|| (Euclidian);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double Norm(const TVec<TType, TSizeTy>& x);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double Normalize(TVec<TType, TSizeTy>& x);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void NormalizeColumn(TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& ColId);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void NormalizeColumns(TVVec<TType, TSizeTy, ColMajor>& X);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void NormalizeRows(TVVec<TType, TSizeTy, ColMajor>& X);
#ifdef INTEL
	// TEST
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void NormalizeColumns(TVVec<TType, TSizeTy, ColMajor>& X, TBool ColumnMajor);
#endif
	template <class TType, class TSizeTy = int, bool ColMajor = false, class IndexType = TInt>
	inline static void NormalizeColumns(TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& X);
	// Normalize the columns of X
	template<class TSizeTy = int>
	inline static void NormalizeColumns(TVec<TIntFltKdV, TSizeTy>& X);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double FrobNorm2(const TVVec<TType, TSizeTy, ColMajor>& X);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double FrobNorm(const TVVec<TType, TSizeTy, ColMajor>& X);
	// ||x||^2 (Euclidian), x is sparse
	template<class TSizeTy = int>
	inline static double Norm2(const TVec<TIntFltKdV, TSizeTy>& x);
	// ||x|| (Euclidian), x is sparse
	template<class TSizeTy = int>
	inline static double Norm(const TVec<TIntFltKdV, TSizeTy>& x);
	// x := x / ||x||, x is sparse
	template<class TSizeTy = int, TSizeTy>
	inline static void Normalize(TVec<TIntFltKdV>& x);
	// ||X(:,ColId)||^2 (Euclidian);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double Norm2(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static double Norm(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId);
	// L1 norm of x (Sum[|xi|, i = 1..n]);
	template <class TType, class TSizeTy = int>
	inline static double NormL1(const TVec<TType, TSizeTy>& x);
	template <class TType, class TSizeTy = int>
	inline static double NormL1(double k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y);
	// L1 norm of x (Sum[|xi|, i = 1..n]);
	inline static double NormL1(const TIntFltKdV& x);
	template <class TType, class TSizeTy = int>
	inline static void NormalizeL1(TVec<TType, TSizeTy>& x);
	// x := x / ||x||_1
	inline static void NormalizeL1(TIntFltKdV& x);
	template <class TType, class TSizeTy = int>
	inline static double NormLinf(const TVec<TType, TSizeTy>& x);
	// Linf norm of x (Max{|xi|, i = 1..n});
	inline static double NormLinf(const TIntFltKdV& x);
	template <class TType, class TSizeTy = int>
	inline static void NormalizeLinf(TVec<TType, TSizeTy>& x);
	// x := x / ||x||_inf, , x is sparse
	inline static void NormalizeLinf(TIntFltKdV& x);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static int GetRowMaxIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& RowN);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static int GetColMaxIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const int& ColN);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void GetRowMaxIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV);
	// find the index of maximum elements for each col of X
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void GetColMaxIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV);
	template <class TType, class TSizeTy = int>
	inline static void MultiplyScalar(const double& k, TVec<TType, TSizeTy>& x);
	// find the index of maximum elements for a given each col of X
	inline static int GetColMinIdx(const TFltVV& X, const int& ColN);
	// find the index of maximum elements for each col of X
	inline static void GetColMinIdxV(const TFltVV& X, TIntV& IdxV);
	template <class TVal> inline static TVal GetColMin(const TVVec<TVal>& X, const int& ColN);
	template <class TVal> inline static void GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV);
	template <class TType, class TSizeTy = int>
	inline static void MultiplyScalar(const double& k, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y);
	// y := k * x
	inline static void MultiplyScalar(const double& k, const TIntFltKdV& x, TIntFltKdV& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyScalar(const double& k, const TVVec<TType, TSizeTy, ColMajor>& X, TVVec<TType, TSizeTy, ColMajor>& Y);
	// Y := k * X
	template <class TSizeTy = int>
	inline static void MultiplyScalar(const double& k, const TVec<TIntFltKdV, TSizeTy>& X, TVec<TIntFltKdV, TSizeTy>& Y);
	// y := A * x
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVVec<TType, TSizeTy, ColMajor>& C, TSizeTy ColId);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, int ColId, TVec<TType, TSizeTy>& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, int ColIdB, TVVec<TType, TSizeTy, ColMajor>& C, int ColIdC);
	//LAPACKE stuff
#ifdef LAPACKE
	// Tested in other function
	//A is rewritten in place with orthogonal matrix Q
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void QRbasis(TVVec<TType, TSizeTy, ColMajor>& A);/* {
		TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
		TSizeTy lda = ColMajor ? m : n;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
		TVec<TType, TSizeTy> tau; tau.Gen(MAX(1, MIN(m, n)));
		LAPACKE_dgeqrf(Matrix_Layout, m, n, &A(0, 0).Val, lda, &tau[0].Val);
		LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
	}*/
	// TEST
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void QRbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q);/* {
		Q = A;
		TLinAlg::QRbasis(Q);
	}*/
	// Tested in other function
	//A is rewritten in place with orthogonal matrix Q (column pivoting to improve stability);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void QRcolpbasis(TVVec<TType, TSizeTy, ColMajor>& A);/* {
		TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols(); TSizeTy k = A.GetCols();
		TSizeTy lda = ColMajor ? m : n;
		TSizeTy Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
		TVec<TType, TSizeTy> tau(MAX(1, MIN(m, n)));
		TVec<TInt, TSizeTy> jvpt(MAX(1, n));
		LAPACKE_dgeqp3(Matrix_Layout, m, n, &A(0, 0).Val, lda, &jvpt[0].Val, &tau[0].Val);
		LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
	}*/
	// TEST
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void QRcolpbasis(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& Q);/* {
		Q = A;
		TLinAlg::QRcolpbasis(Q);
	}*/
	// TEST
	//S S option ensures that A is not modified
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void thinSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& U, TVec<TType, TSizeTy>& S, TVVec<TType, TSizeTy, ColMajor>& VT);
	/*{
		TSizeTy m = A.GetRows(); TSizeTy n = A.GetCols();
		TSizeTy thin_dim = MIN(m, n);
		S.Gen(thin_dim); U.Gen(m, thin_dim); VT.Gen(thin_dim, n);
		int lda = ColMajor ? m : n;
		int ldu = ColMajor ? m : thin_dim;
		int ldvt = ColMajor ? thin_dim : n;
		TVec<TType, TSizeTy> superb(MAX(1, MIN(m, n)));
		int opt = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
		LAPACKE_dgesvd(opt, 'S', 'S', m, n, const_cast<double *>(&A(0, 0).Val), lda, &S[0].Val, &U(0, 0).Val, ldu, &VT(0, 0).Val, ldvt, &superb[0].Val);
	}*/
#endif
	inline static int ComputeThinSVD(const TMatrix& X, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters = 2, const double Tol = 1e-6);
#ifdef INTEL
	template <class TType, class TSizeTy, bool ColMajor = false>
	inline static void MultiplySF(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, false>& B,
		TVVec<TType, TSizeTy, ColMajor>& C, const TStr& transa = TStr("N"), const int& format = 0);
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyFS(TVVec<TType, TSizeTy, ColMajor>& B, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A,
		TVVec<TType, TSizeTy, ColMajor>& C);
#endif
	// y := A * x
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, TVec<TType, TSizeTy>& y);
	//y := x' * A ... row data!!
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TPair<TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& x, const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TType, TSizeTy>& y);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y);
#ifdef BLAS
	typedef enum { NOTRANS = 0, TRANS = 1 } TLinAlgBlasTranspose;
	template <class TType, class TSizeTy = int, bool ColMajor = false>
		inline static void Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVVec<TNum<TType>, TSizeTy, ColMajor>& B, TVVec<TNum<TType>, TSizeTy, ColMajor>& C,
		const int& BlasTransposeFlagA, const int& BlasTransposeFlagB);
#endif
#ifdef BLAS
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TNum<TType>, TSizeTy, ColMajor>& A, const TVec<TNum<TType>, TSizeTy>& x, TVec<TNum<TType>, TSizeTy>& y, const int& BlasTransposeFlagA, TType alpha = 1.0, TType beta = 0.0);
#endif
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, TVVec<TType, TSizeTy, ColMajor>& C);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, TVVec<TType, TSizeTy, ColMajor>& C);
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	//#if !defined(INTEL) || defined(INDEX_64);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Multiply(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TTriple<TVec<TNum<TSizeTy>, TSizeTy>, TVec<TNum<TSizeTy>, TSizeTy>, TVec<TType, TSizeTy>>& A, const TVVec<TType, TSizeTy, ColMajor>& B,
		TVVec<TType, TSizeTy, ColMajor>& C);
	inline static void Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C);
	// C:= A' * B
	template <class IndexType = TInt, class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TKeyDat<IndexType, TSizeTy>>& B, TVVec<TType, TSizeTy, ColMajor>& C);
	inline static void Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA = -1);
	inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C);
	inline static void Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA = -1);
	inline static void MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C);
	typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Gemm(const double& Alpha, const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, const double& Beta,
		const TVVec<TType, TSizeTy, ColMajor>& C, TVVec<TType, TSizeTy, ColMajor>& D, const int& TransposeFlags);
	typedef enum { DECOMP_SVD } TLinAlgInverseType;
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Inverse(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor >& B, const TLinAlgInverseType& DecompType);
	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void InverseSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B, const double& tol);
	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void InverseSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B);
	// transpose matrix - B = A'
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void Transpose(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B);
	// performes Gram-Schmidt ortogonalization on elements of Q
	template <class TSizeTy = int>
	inline static void GS(TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Q);
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void GS(TVVec<TType, TSizeTy, ColMajor>& Q);
	// Modified Gram-Schmidt on columns of matrix Q
	inline static void MGS(TFltVV& Q);
	// QR based on Modified Gram-Schmidt decomposition.
	inline static void QR(const TFltVV& X, TFltVV& Q, TFltVV& R, const TFlt& Tol);
	// rotates vector (OldX,OldY) for angle Angle (in radians!);
	inline static void Rotate(const double& OldX, const double& OldY, const double& Angle, double& NewX, double& NewY);
	// checks if set of vectors is ortogonal
	template <class TSizeTy = int>
	inline static void AssertOrtogonality(const TVec<TVec<TFlt, TSizeTy>, TSizeTy>& Vecs, const double& Threshold);
	//ColMajor oriented data for optimal result
	template <class TType, class TSizeTy = int, bool ColMajor = false>
	inline static void AssertOrtogonality(const TVVec<TType, TSizeTy, ColMajor>& Vecs, const double& Threshold);
	inline static bool IsOrthonormal(const TFltVV& Vecs, const double& Threshold);
};


//////////////////////////////////////////////////////////////////////
// Basic Linear Algebra Operations
//////////////////////////////////////////////////////////////////////
// Basic Linear Algebra Operations
//class TLinAlg {
//public:
	// <x,y>
	// TEST
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
		EAssertR(x.Len() == y.Len(), TStr::Fmt("%d != %d", x.Len(), y.Len()));
		TType result = 0.0; const  TSizeTy Len = x.Len();
		for (TSizeTy i = 0; i < Len; i++)
			result += x[i] * y[i];
		return result;
	}

	double TLinAlg::DotProduct(const TVec<TFltV>& X, int ColId, const TFltV& y) {
		EAssert(0 <= ColId && ColId < X.Len());
		return DotProduct(X[ColId], y);
	}
	double TLinAlg::DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TFltV& y) {
		EAssert(0 <= ColId && ColId < X.Len());
		return DotProduct(y, X[ColId]);
	}

	// TEST
	// <X(:,ColIdX), Y(:,ColIdY)>
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
		int ColIdX, const TVVec<TType, TSizeTy, ColMajor>& Y, int ColIdY) {
		EAssert(X.GetRows() == Y.GetRows());
		TType result = 0.0; const TSizeTy len = X.GetRows();
		for (TSizeTy i = 0; i < len; i++)
			result = result + X(i, ColIdX) * Y(i, ColIdY);
		return result;
	}
	// TEST
	// <X(:,ColId), Vec>
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X,
		int ColId, const TVec<TType, TSizeTy>& Vec) {

		EAssert(X.GetRows() == Vec.Len());
		TType result = 0.0; const TSizeTy len = X.GetRows();
		for (TSizeTy i = 0; i < len; i++)
			result += X(i, ColId) * Vec[i];
		return result;
	}

	// sparse dot products:
	// <x,y> where x AND y are sparse
	//TODO TIntFltKdV indexing and is TInt enough?
	double TLinAlg::DotProduct(const TIntFltKdV& x, const TIntFltKdV& y) {
		const int xLen = x.Len(), yLen = y.Len();
		double Res = 0.0; int i1 = 0, i2 = 0;
		while (i1 < xLen && i2 < yLen) {
			if (x[i1].Key < y[i2].Key) i1++;
			else if (x[i1].Key > y[i2].Key) i2++;
			else { Res += x[i1].Dat * y[i2].Dat;  i1++;  i2++; }
		}
		return Res;
	}

	// <x,y> where only y is sparse
	//TODO TIntFltKdV indexing and is TInt enough?
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::DotProduct(const TVec<TType, TSizeTy>& x, const TVec<TIntFltKd>& y) {
		double Res = 0.0; const int xLen = x.Len(), yLen = y.Len();
		for (TSizeTy i = 0; i < yLen; i++) {
			const TSizeTy key = y[i].Key;
			if (key < xLen) Res += y[i].Dat * x[key];
		}
		return Res;
	}

	// <X(:,ColId),y> where only y is sparse
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::DotProduct(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, const TIntFltKdV& y) {
		TType Res = 0.0; const TSizeTy n = X.GetRows(), yLen = y.Len();
		for (TSizeTy i = 0; i < yLen; i++) {
			const TSizeTy key = y[i].Key;
			if (key < n) Res += y[i].Dat * X(key, ColId);
		}
		return Res;
	}

	// TEST
	// z = x * y'    
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

	// z := p * x + q * y
	//TODO should double be TType?
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::LinComb(const double& p, const TVec<TType, TSizeTy>& x,
		const double& q, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z) {

		EAssert(x.Len() == y.Len() && y.Len() == z.Len());
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
	// z = p * x + q * y
	void TLinAlg::LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z) {
		TSparseOpsIntFlt::SparseLinComb(p, x, q, y, z);
	}

	void TLinAlg::LinComb(const double& p, const TFltVV& X, int ColId,
		const double& q, const TFltV& y, TFltV& z) {

		if (z.Empty()) z.Gen(X.GetRows());
		EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());

		const int len = z.Len();
		for (int i = 0; i < len; i++) {
			z[i] = p * X(i, ColId) + q * y[i];
		}
	}
	void TLinAlg::LinComb(const double& p, const TFltVV& X, int DimId,
		const double& q, const TFltV& y, TFltV& z, int Dim) {

		EAssertR(Dim == 1 || Dim == 2, "TLinAlg::LinComb: Invalid value of argument Dim.");
		if (Dim == 1) {
			if (z.Empty()) z.Gen(X.GetRows());
			EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());

			const int len = z.Len();
			for (int i = 0; i < len; i++) {
				z[i] = p * X(i, DimId) + q * y[i];
			}
		}
		else if (Dim == 2) {
			if (z.Empty()) z.Gen(X.GetCols());
			EAssert(X.GetCols() == y.Len() && y.Len() == z.Len());

			const int len = z.Len();
			for (int i = 0; i < len; i++) {
				z[i] = p * X(DimId, i) + q * y[i];
			}
		}
	}
	void TLinAlg::LinComb(const double& p, const TFltVV& X, const double& q, const TFltVV& Y, TFltVV& Z) {
		if (Z.Empty()) Z.Gen(X.GetRows(), X.GetCols());
		EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols() && X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
		int Rows = X.GetRows();
		int Cols = X.GetCols();
		for (int RowN = 0; RowN < Rows; RowN++) {
			for (int ColN = 0; ColN < Cols; ColN++) {
				Z.At(RowN, ColN) = p*X.At(RowN, ColN) + q*Y.At(RowN, ColN);
			}
		}
	}

	// TEST
	// z := p * x + (1 - p) * y
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::ConvexComb(const double& p, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z)  {
		AssertR(0.0 <= p && p <= 1.0, TFlt::GetStr(p));
		TLinAlg::LinComb(p, x, 1.0 - p, y, z);
	}

	//this will fail if TType != TFlt, Specialization should be used
#ifdef BLAS
	// TEST
	//y = k * x + y 
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
#endif
	// TEST
	// z := k * x + y 
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z)  {
		TLinAlg::LinComb(k, x, 1.0, y, z);
	}
	// z := k * X[ColId] + y
	//Andrej template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z) {
		EAssert(0 <= ColId && ColId < X.Len());
		AddVec(k, X[ColId], y, z);
	}
	// z := k * X(:,ColId) + y
	//Andrej template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z) {
		EAssert(X.GetRows() == y.Len());
		EAssert(y.Len() == z.Len());
		const int len = z.Len();
		for (int i = 0; i < len; i++) {
			z[i] = y[i] + k * X(i, ColId);
		}
	}
	// z := x + y
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y, TVec<TType, TSizeTy>& z) {
		TLinAlg::LinComb(1.0, x, 1.0, y, z);
	}

	// z := k * x + y
	//template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TIntFltKdV& x, const TFltV& y, TFltV& z) {
		EAssert(y.Len() == z.Len());
		z = y; // first we set z to be y
		// and than we add x to z (==y)
		const int xLen = x.Len(), yLen = y.Len();
		for (int i = 0; i < xLen; i++) {
			const int ii = x[i].Key;
			if (ii < yLen) {
				z[ii] = k * x[i].Dat + y[ii];
			}
		}
	}
	// z := k * X[ColId] + y
	void TLinAlg::AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z) {
		EAssert(0 <= ColId && ColId < X.Len());
		AddVec(k, X[ColId], y, z);
	}
	// y := k * x + y
	//template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TIntFltKdV& x, TFltV& y) {
		const int xLen = x.Len(), yLen = y.Len();
		for (int i = 0; i < xLen; i++) {
			const int ii = x[i].Key;
			if (ii < yLen) {
				y[ii] += k * x[i].Dat;
			}
		}
	}

	// TEST
	// Y(:,Col) += k * X(:,Col)
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, TSizeTy ColIdX, TVVec<TType, TSizeTy, ColMajor>& Y, TSizeTy ColIdY) {
		EAssert(X.GetRows() == Y.GetRows());
		const TSizeTy len = Y.GetRows();
		for (TSizeTy i = 0; i < len; i++) {
			Y(i, ColIdY) = Y(i, ColIdY) + k * X(i, ColIdX);
		}
	}

	// TEST
	// Y(:,ColIdY) += k * x
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const double& k, const TVec<TType, TSizeTy>& x, TVVec<TType, TSizeTy, ColMajor>& Y, const TSizeTy& ColIdY) {
		EAssert(x.Len() == Y.GetRows());
		EAssert(ColIdY >= 0 && ColIdY < x.Len());
		for (TSizeTy RowN = 0; RowN < Y.GetRows(); RowN++) {
			Y.At(RowN, ColIdY) += k*x[RowN];
		}
	}

	// TEST
	// Result += k * X(:,Col)
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(double k, const TVVec<TType, TSizeTy, ColMajor>& X, int ColId, TVec<TType, TSizeTy>& Result) {
		EAssert(X.GetRows() == Result.Len());
		const TSizeTy len = Result.Len();
		for (TSizeTy i = 0; i < len; i++) {
			Result[i] = Result[i] + k * X(i, ColId);
		}
	}

	// z = x + y
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z) {
		TSparseOpsIntFlt::SparseMerge(x, y, z);
	}

	// TEST
	// Result = SUM(x)
	template <class TType, class TSizeTy>
	double TLinAlg::SumVec(const TVec<TType, TSizeTy>& x) {
		const TSizeTy len = x.Len();
		double Res = 0.0;
		for (int i = 0; i < len; i++) {
			Res += x[i];
		}
		return Res;
	}

	// Result = SUM(x)
	//template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::SumVec(const TIntFltKdV& x) {
		const int len = x.Len();
		double Res = 0.0;
		for (int i = 0; i < len; i++) {
			Res += x[i].Dat;
		}
		return Res;
	}

	// TEST
	// Result = SUM(k*x + y)
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

	// Result = ||x-y||^2 (Euclidian)
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::EuclDist2(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
		EAssert(x.Len() == y.Len());
		const TSizeTy len = x.Len();
		double Res = 0.0;
		for (TSizeTy i = 0; i < len; i++) {
			Res += TMath::Sqr(x[i] - y[i]);
		}
		return Res;
	}

	// Result = ||x-y||^2 (Euclidian)
	double TLinAlg::EuclDist2(const TFltPr& x, const TFltPr& y) {
		return TMath::Sqr(x.Val1 - y.Val1) + TMath::Sqr(x.Val2 - y.Val2);
	}

	// TEST
	// Result = ||x-y|| (Euclidian)
	template <class TType, class TSizeTy>
	double TLinAlg::EuclDist(const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
		return sqrt(TLinAlg::EuclDist2(x, y));
	}

	// Result = ||x-y|| (Euclidian)
	//template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::EuclDist(const TFltPr& x, const TFltPr& y) {
		return sqrt(TLinAlg::EuclDist2(x, y));
	}
	// Result = ||A||_F (Frobenious)
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
	// TEST
	// Result = ||A - B||_F (Frobenious)
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

	// TEST
	// Result = ||A - B||_F (Frobenious)
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::FrobDist2(const TVec<TType, TSizeTy>& A, const TVec<TType, TSizeTy>& B) {
		double frob = 0;
		frob = TLinAlg::EuclDist2(A, B);
		/*for (int RowN = 0; RowN < A.Len(); RowN++) {
		frob += (A[RowN] - B[RowN])*(A[RowN] - B[RowN]);
		}*/
		return frob;
	}

	// Dense to sparse transform
	// TEST
	// Dense to sparse transform
	template <class TType, class TSizeTy, bool ColMajor, class IndexType>
	void TLinAlg::Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& B){
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
	void TLinAlg::Sparse(const TVVec<TType, TSizeTy, ColMajor>& A, TVec<TIntFltKdV>& B){
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
	void TLinAlg::Full(const TTriple<TVec<IndexType, TSizeTy>, TVec<IndexType, TSizeTy>, TVec<TType, TSizeTy>>& A, TVVec<TType, TSizeTy, ColMajor>& B, const int Rows, const int Cols) {
		B.Gen(Rows, Cols);
		B.PutAll(0.0);
		TSizeTy nnz = A.Val1.Len();
		for (TSizeTy ElN = 0; ElN < nnz; ElN++) {
			B.At(A.Val1[ElN], A.Val2[ElN]) = A.Val3[ElN];
		}
	}

	// Sparse to dense transform
	template <class TType, class TSizeTy, bool ColMajor, class IndexType>
	void TLinAlg::Full(const TVec<TIntFltKdV, TSizeTy>& A, TVVec<TType, TSizeTy, ColMajor>& B, TSizeTy Rows){
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

	// TEST
	// Transpose
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

	// Transpose
	//TODO Index template
	void TLinAlg::Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows){
		// A is a sparse col matrix:	
		int Cols = A.Len();
		// find number of rows
		if (Rows == -1) {
			for (int ColN = 0; ColN < Cols; ColN++) {
				int Els = A[ColN].Len();
				for (int ElN = 0; ElN < Els; ElN++) {
					Rows = MAX(Rows, A[ColN][ElN].Key.Val);
				}
			}
			Rows = Rows + 1;
		}
		At.Gen(Rows);
		// transpose
		for (int ColN = 0; ColN < Cols; ColN++) {
			int Els = A[ColN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				At[A[ColN][ElN].Key].Add(TIntFltKd(ColN, A[ColN][ElN].Dat));
			}
		}
		// sort
		for (int ColN = 0; ColN < Rows; ColN++) {
			At[ColN].Sort();
		}
	}

	// Sign
	void  TLinAlg::Sign(const TVec<TIntFltKdV>& Mat, TVec<TIntFltKdV>& Mat2) {
		Mat2 = Mat;
		int Cols = Mat2.Len();
		for (int ColN = 0; ColN < Cols; ColN++) {
			int Els = Mat2[ColN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				Mat2[ColN][ElN].Dat = TMath::Sign(Mat2[ColN][ElN].Dat);
			}
		}
	}

	// Vector of sparse vectors to sparse matrix (coordinate representation)
	//TODO Index template
	void TLinAlg::Convert(const TVec<TPair<TIntV, TFltV>>& A, TTriple<TIntV, TIntV, TFltV>& B) {
		B.Val1.Clr();
		B.Val2.Clr();
		B.Val3.Clr();
		int Cols = A.Len();
		for (int ColN = 0; ColN < Cols; ColN++) {
			int Nnz = A[ColN].Val1.Len();
			for (int ElN = 0; ElN < Nnz; ElN++) {
				B.Val1.Add(A[ColN].Val1[ElN]);
				B.Val2.Add(ColN);
				B.Val3.Add(A[ColN].Val2[ElN]);
			}
		}
	}
	// Vector of sparse vectors to sparse matrix (coordinate representation)
	//TODO Index template
	void TLinAlg::Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>&B) {
		int Cols = A.Len();
		int TotalNnz = 0;
		for (int ColN = 0; ColN < Cols; ColN++) {
			TotalNnz += A[ColN].Len();
		}
		B.Val1.Gen(TotalNnz, 0);
		B.Val2.Gen(TotalNnz, 0);
		B.Val3.Gen(TotalNnz, 0);
		for (int ColN = 0; ColN < Cols; ColN++) {
			int Nnz = A[ColN].Len();
			for (int ElN = 0; ElN < Nnz; ElN++) {
				B.Val1.Add(A[ColN][ElN].Key);
				B.Val2.Add(ColN);
				B.Val3.Add(A[ColN][ElN].Dat);
			}
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

	// TEST
	// ||x||^2 (Euclidian)
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::Norm2(const TVec<TType, TSizeTy>& x) {
		return TLinAlg::DotProduct(x, x);
	}

	// ||x|| (Euclidian)
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::Norm(const TVec<TType, TSizeTy>& x) {
		return sqrt(TLinAlg::Norm2(x));
	}

	//Andrej switch this to TNum<TType>
	// TEST
	// x := x / ||x||
	template <class TType, class TSizeTy, bool ColMajor>
	double TLinAlg::Normalize(TVec<TType, TSizeTy>& x) {
		const double xNorm = TLinAlg::Norm(x);
		if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
		return xNorm;
	}

	// TEST
	// Normalize X(:,ColId)
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::NormalizeColumn(TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& ColId) {
		double nX = TLinAlg::Norm(X, ColId);
		if (nX > 0.0) {
			for (TSizeTy RowN = 0; RowN < X.GetRows(); RowN++) {
				X.At(RowN, ColId) /= nX;
			}
		}
	}

	// TEST
	// Normalize the columns of X
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
	double TLinAlg::Norm2(const TVVec<TType, TSizeTy, ColMajor>& X, int ColId) {
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

	// TEST
	// L1 norm of k*x+y (Sum[|k*xi+yi|, i = 1..n])
	template <class TType, class TSizeTy>
	double TLinAlg::NormL1(double k, const TVec<TType, TSizeTy>& x, const TVec<TType, TSizeTy>& y) {
		EAssert(x.Len() == y.Len());
		double norm = 0.0; const TSizeTy len = x.Len();
		for (TSizeTy i = 0; i < len; i++) {
			norm += TFlt::Abs(k * x[i] + y[i]);
		}
		return norm;
	}

	// L1 norm of x (Sum[|xi|, i = 1..n])


	double TLinAlg::NormL1(const TIntFltKdV& x) {
		double norm = 0.0; const int Len = x.Len();
		for (int i = 0; i < Len; i++)
			norm += TFlt::Abs(x[i].Dat);
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

	// Linf norm of x (Max{|xi|, i = 1..n})
	double TLinAlg::NormLinf(const TIntFltKdV& x) {
		double norm = 0.0; const int Len = x.Len();
		for (int i = 0; i < Len; i++)
			norm = TFlt::GetMx(TFlt::Abs(x[i].Dat), norm);
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

	// TEST
	// find the index of maximum elements for a given row of X
	template <class TType, class TSizeTy, bool ColMajor>
	int TLinAlg::GetRowMaxIdx(const TVVec<TType, TSizeTy, ColMajor>& X, const TSizeTy& RowN) {
		TSizeTy Idx = -1;
		TSizeTy Cols = X.GetCols();
		double MaxVal = TFlt::Mn;
		for (TSizeTy ColN = 0; ColN < Cols; ColN++) {
			double Val = X.At(RowN, ColN);
			if (MaxVal < Val) {
				MaxVal = Val;
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

	// TEST
	// find the index of maximum elements for each row of X
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::GetRowMaxIdxV(const TVVec<TType, TSizeTy, ColMajor>& X, TVec<TInt, TSizeTy>& IdxV) {
		IdxV.Gen(X.GetRows());
		TSizeTy Rows = X.GetRows();
		for (TSizeTy RowN = 0; RowN < Rows; RowN++) {
			IdxV[RowN] = TLinAlg::GetRowMaxIdx(X, RowN);
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
	// find the index of maximum elements for a given each col of X
	int TLinAlg::GetColMinIdx(const TFltVV& X, const int& ColN) {
		const int Rows = X.GetRows();
		double MinVal = TFlt::Mx;
		int MinIdx = -1;
		for (int RowN = 0; RowN < Rows; RowN++) {
			double Val = X(RowN, ColN);
			if (Val < MinVal) {
				MinVal = Val;
				MinIdx = RowN;
			}
		}
		return MinIdx;
	}

	// find the index of maximum elements for each col of X
	void TLinAlg::GetColMinIdxV(const TFltVV& X, TIntV& IdxV) {
		int Cols = X.GetCols();
		IdxV.Gen(X.GetCols());
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
#ifdef BLAS
		TLinAlg::Multiply(A, x, y, TLinAlgBlasTranspose::NOTRANS, 1.0, 0.0);
#else
		if (y.Empty()) y.Gen(A.GetRows());
		EAssert(A.GetCols() == x.Len() && A.GetRows() == y.Len());
		int n = A.GetRows(), m = A.GetCols();
		for (int i = 0; i < n; i++) {
			y[i] = 0.0;
			for (int j = 0; j < m; j++)
				y[i] += A(i, j) * x[j];
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
		TLAMisc::FillZero(y);
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
	void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TType, TSizeTy>& x, TVec<TType, TSizeTy>& y) {
		if (y.Empty()) y.Gen(A.GetCols());
		EAssert(A.GetRows() == x.Len() && A.GetCols() == y.Len());
		int n = A.GetCols(), m = A.GetRows();
		for (int i = 0; i < n; i++) {
			y[i] = 0.0;
			for (int j = 0; j < m; j++)
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
		if (BlasTransposeFlagA)//A'*x n*m x m -> n
			EAssert(x.Len() == m && y.Reserved() == n);
		else{//A*x  m x n * n -> m
			EAssert(x.Len() == n && y.Reserved() == m);
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
		EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetCols() == B.GetRows());
#ifdef BLAS
		TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::NOTRANS, TLinAlgBlasTranspose::NOTRANS);
#else
		EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetCols() == B.GetRows());
		TSizeTy n = C.GetRows(), m = C.GetCols(), l = A.GetCols();
		for (TSizeTy i = 0; i < n; i++) {
			for (TSizeTy j = 0; j < m; j++) {
				double sum = 0.0;
				for (TSizeTy k = 0; k < l; k++)
					sum += A(i, k)*B(k, j);
				C(i, j) = sum;
			}
		}
#endif

	}


	// TEST
	// C = A' * B
	template <class TType, class TSizeTy, bool ColMajor>
	void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVVec<TType, TSizeTy, ColMajor>& B, TVVec<TType, TSizeTy, ColMajor>& C) {
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
		EAssert(TLAMisc::GetMaxDimIdx(B) < A.GetCols());
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
	void TLinAlg::MultiplyT(const TVVec<TType, TSizeTy, ColMajor>& A, const TVec<TKeyDat<IndexType, TSizeTy>>& B, TVVec<TType, TSizeTy, ColMajor>& C) {
		// C = A' B = (B' A)'
#ifdef INTEL
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
		EAssert(TLAMisc::GetMaxDimIdx(B) < A.GetRows());
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
			Rows = TLAMisc::GetMaxDimIdx(A) + 1;
		}
		else {
			EAssert(TLAMisc::GetMaxDimIdx(A) + 1 <= RowsA);
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
		EAssert(TLAMisc::GetMaxDimIdx(A) + 1 <= B.GetRows());
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
			Rows = TLAMisc::GetMaxDimIdx(A) + 1;
		}
		else {
			EAssert(TLAMisc::GetMaxDimIdx(A) + 1 <= RowsA);
		}
		if (C.Empty()) {
			C.Gen(Rows, ColsB);
		}
		EAssert(TLAMisc::GetMaxDimIdx(B) + 1 <= A.Len());
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
	void TLinAlg::InverseSVD(const TVVec<TType, TSizeTy, ColMajor>& A, TVVec<TType, TSizeTy, ColMajor>& B, const double& tol) {
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


template <class TVal>
TVal TLinAlg::GetColMin(const TVVec<TVal>& X, const int& ColN) {
	const int Rows = X.GetRows();
	EAssertR(Rows > 0, "Input matrix should have at least one row!");

	TVal MinVal = X(0, ColN);
	for (int RowN = 1; RowN < Rows; RowN++) {
		TVal Val = X(RowN, ColN);
		if (Val < MinVal) {
			MinVal = Val;
		}
	}

	return MinVal;
}

template <class TVal>
void TLinAlg::GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV) {
	const int Cols = X.GetCols();

	ValV.Gen(Cols);
	for (int ColN = 0; ColN < Cols; ColN++) {
		ValV[ColN] = GetColMin(X, ColN);
	}
}

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
	// A is modified!!!
	static void GetEigenVec(const TFltVV& A, const double& EigenVal, TFltV& EigenV, const double& ConvergEps = 1e-7);
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
	// Move constructor
	TVector(const TVector&& Vector);
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
	TFullMatrix(TFltVV& Mat, const bool IsWrapper = false);
	// matrix from vector
	TFullMatrix(const TVector& Vec);
	// copy constructor
	TFullMatrix(const TFullMatrix& Mat);
	// move constructor
	TFullMatrix(TFullMatrix&& Mat);

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
	// returns the value at position (i,j)
	TFlt& At(const int& i, const int& j) { return Mat->operator ()(i, j); }
	const TFlt& At(const int& i, const int& j) const { return Mat->operator ()(i, j); }
	// sets the value at position (i,j)
	void Set(const double& Val, const int& i, const int& j) { Mat->operator ()(i, j) = Val; }
	// returns true if the matrix is empty
	bool Empty() const { return Mat->Empty(); }

	TFullMatrix& AddCol(const TVector& Col);
	TFullMatrix& AddCols(const TFullMatrix& Cols);

	// operators
	TFlt& operator ()(const int& i, const int& j) { return At(i, j); }
	const TFlt& operator ()(const int& i, const int& j) const { return At(i, j); }
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


public:
	void Save(TSOut& SOut) const;
	void Load(TSIn& SIn);
};

template <class TIdxV1, class TIdxV2>
TFullMatrix TFullMatrix::operator ()(const TIdxV1& RowV, const TIdxV2& ColV) const {
	const int Rows = RowV.Len();
	const int Cols = ColV.Len();

	TFullMatrix Result(Rows, Cols);
	for (int i = 0; i < Rows; i++) {
		for (int j = 0; j < Cols; j++) {
			const int Idx1 = (int)RowV[i];
			const int Idx2 = (int)ColV[j];
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

#ifdef LAPACKE
#include "MKLfunctions.h"
#endif

#endif
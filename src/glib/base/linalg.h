/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

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

//==========================================================
// TODO remove
#ifdef BLAS
#include "lapacke.h"
#endif
//==========================================================

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
{ static const bool value = false; };

template<>
struct is_complex_float< std::complex<float> >
{ static const bool value = true; };

template<>
struct is_complex_float< TNum<std::complex<float>> >
{
	static const bool value = true;
};

template<typename T1>
struct is_complex_double
{ static const bool value = false; };

template<>
struct is_complex_double< std::complex<double> >
{ static const bool value = true; };

template<>
struct is_complex_double< TNum<std::complex<double>> >
{
	static const bool value = true;
};

///////////////////////////////////////////////////////////////////////
// forward declarations
class TLinAlg;
//////////////////////////////////////////////////////////////////////
// Useful stuff (hopefully)
class TLAMisc {
public:
	//Sort double array
#ifdef SCALAPACK 
	template<class Size>
	static void Sort(TVec<TFlt, Size> & Vec, TVec<Size, Size>& index, const TBool& decrease) {
		if (index.Empty()){
			TLAMisc::FillRange(Vec.Len(), index);
		}
		char* id = decrease ? "D" : "I";
		Size n = Vec.Len();
		Size info;
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

	template <class TVal, class TSizeTy = int>
	static void FillRangeS(const TSizeTy& Vals, TVec<TVal, TSizeTy>& Vec) {
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
	// returns the index of the minimum element
	static int GetMinIdx(const TFltV& Vec);
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
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const { FailR("TMatrix PMultiply(const TFltVV& B, TFltVV& Result) not implemented");}
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const { FailR("TMatrix PMultiplyT(const TFltVV& B, TFltVV& Result) not implemented"); }

    virtual int PGetRows() const = 0;
    virtual int PGetCols() const = 0;
public:
    TMatrix(): Transposed(false) {}
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
class TSparseColMatrix: public TMatrix {
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
    TSparseColMatrix(): TMatrix() {}
    TSparseColMatrix(const int& _RowN, const int& _ColN): RowN(_RowN), ColN(_ColN), ColSpVV() {}
    TSparseColMatrix(TVec<TIntFltKdV> _ColSpVV): TMatrix(), ColSpVV(_ColSpVV) {}
    TSparseColMatrix(TVec<TIntFltKdV> _ColSpVV, const int& _RowN, const int& _ColN): 
		TMatrix(), RowN(_RowN), ColN(_ColN), ColSpVV(_ColSpVV) {}

    void Save(TSOut& SOut) {
        SOut.Save(RowN); SOut.Save(ColN); ColSpVV.Save(SOut); }
    void Load(TSIn& SIn) {
        SIn.Load(RowN); SIn.Load(ColN); ColSpVV = TVec<TIntFltKdV>(SIn); }
};

///////////////////////////////////////////////////////////////////////
// Sparse-Row-Matrix
//  matrix is given with rows as sparse vectors
class TSparseRowMatrix: public TMatrix {
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
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO

    int PGetRows() const { return RowN; }
    int PGetCols() const { return ColN; }

public:
    TSparseRowMatrix(): TMatrix() {}
    TSparseRowMatrix(TVec<TIntFltKdV> _RowSpVV): TMatrix(), RowSpVV(_RowSpVV) {}
    TSparseRowMatrix(TVec<TIntFltKdV> _RowSpVV, const int& _RowN, const int& _ColN): 
		TMatrix(), RowN(_RowN), ColN(_ColN), RowSpVV(_RowSpVV) {}
	// loads Matlab sparse matrix format: row, column, value.
    //   Indexes start with 1.
    TSparseRowMatrix(const TStr& MatlabMatrixFNm);
    void Save(TSOut& SOut) {
        SOut.Save(RowN); SOut.Save(ColN); RowSpVV.Save(SOut); }
    void Load(TSIn& SIn) {
        SIn.Load(RowN); SIn.Load(ColN); RowSpVV = TVec<TIntFltKdV>(SIn); }
};

///////////////////////////////////////////////////////////////////////
// Full-Col-Matrix
//  matrix is given with columns of full vectors
class TFullColMatrix: public TMatrix {
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
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO

    int PGetRows() const { return RowN; }
    int PGetCols() const { return ColN; }

public:
    TFullColMatrix(): TMatrix() {}
    // loads matrix saved in matlab with command:
    //  save -ascii Matrix.dat M
    TFullColMatrix(const TStr& MatlabMatrixFNm);
	TFullColMatrix(TVec<TFltV>& RowVV);
    void Save(TSOut& SOut) {SOut.Save(RowN); SOut.Save(ColN);  ColV.Save(SOut); }
    void Load(TSIn& SIn) {SIn.Load(RowN); SIn.Load(ColN); ColV.Load(SIn); }
};

///////////////////////////////////////////////////////////////////////
// Structured-Covariance-Matrix
//  matrix is a product of two sparse matrices X Y' (column examples, row features), 
//  which are centered implicitly by using two dense mean vectors
class TStructuredCovarianceMatrix: public TMatrix {	
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
    TStructuredCovarianceMatrix(): TMatrix() {}    
	TStructuredCovarianceMatrix(const int XRowN_, const int YRowN_, const int SampleN_, const TFltV& MeanX_, const TFltV& MeanY_, const TTriple<TIntV, TIntV, TFltV>& X_, const TTriple<TIntV, TIntV, TFltV>& Y_): TMatrix(), XRows(XRowN_), YRows(YRowN_), Samples(SampleN_), MeanX(MeanX_), MeanY(MeanY_), X(X_), Y(Y_) {};
    void Save(TSOut& SOut) {SOut.Save(XRows); SOut.Save(YRows); SOut.Save(Samples); MeanX.Save(SOut); MeanY.Save(SOut); X.Save(SOut); Y.Save(SOut);}
    void Load(TSIn& SIn) {SIn.Load(XRows); SIn.Load(YRows); SIn.Load(Samples); MeanX.Load(SIn); MeanY.Load(SIn); X.Load(SIn); Y.Load(SIn);}
};

//////////////////////////////////////////////////////////////////////
// Basic Linear Algebra Operations
class TLinAlg {
public:
    // <x,y>
	// TEST
    template <class Type, class Size = int, bool ColMajor = false>
	static double DotProduct(const TVec<Type, Size>& x, const TVec<Type, Size>& y) {
		EAssertR(x.Len() == y.Len(), TStr::Fmt("%d != %d", x.Len(), y.Len()));
		Type result = 0.0; const  Size Len = x.Len();
		for (Size i = 0; i < Len; i++)
			result += x[i] * y[i];
		return result;
	}
	// <X[ColId], Vec>
//  template <class Type, class Size = int, bool ColMajor = false>
//	static Type DotProduct(const TVec<TVec<TNum<Type>, Size>, Size>& X,
//          int ColId, const TVec<TNum<Type>, Size>& Vec) {
//
//		EAssert(0 <= ColId && ColId < X.Len());
//		return DotProduct(X[ColId], y);
//	}
	static double DotProduct(const TVec<TFltV>& X, int ColId, const TFltV& y) {
		EAssert(0 <= ColId && ColId < X.Len());
		return DotProduct(X[ColId], y);
	}
	static double DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TFltV& y) {
		EAssert(0 <= ColId && ColId < X.Len());
		return DotProduct(y, X[ColId]);
	}

	// TEST
    // <X(:,ColIdX), Y(:,ColIdY)>
    template <class Type, class Size = int, bool ColMajor = false>
    static double DotProduct(const TVVec<Type, Size, ColMajor>& X,
            int ColIdX, const TVVec<Type, Size, ColMajor>& Y, int ColIdY) {
		EAssert(X.GetRows() == Y.GetRows());
		Type result = 0.0; const Size len = X.GetRows();
		for (Size i = 0; i < len; i++)
			result = result + X(i, ColIdX) * Y(i, ColIdY);
		return result;
	}
	// TEST
    // <X(:,ColId), Vec>
    template <class Type, class Size = int, bool ColMajor = false>
	static double DotProduct(const TVVec<Type, Size, ColMajor>& X,
            int ColId, const TVec<Type, Size>& Vec) {
        
		EAssert(X.GetRows() == Vec.Len());
		Type result = 0.0; const Size len = X.GetRows();
		for (Size i = 0; i < len; i++)
			result += X(i, ColId) * Vec[i];
		return result;
	}

    // sparse dot products:
    // <x,y> where x AND y are sparse
	//TODO TIntFltKdV indexing and is TInt enough?
	static double DotProduct(const TIntFltKdV& x, const TIntFltKdV& y) {
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
    template <class Type, class Size = int, bool ColMajor = false>
    static double DotProduct(const TVec<Type, Size>& x, const TVec<TIntFltKd>& y) {
		double Res = 0.0; const int xLen = x.Len(), yLen = y.Len();
		for (Size i = 0; i < yLen; i++) {
			const Size key = y[i].Key;
			if (key < xLen) Res += y[i].Dat * x[key];
		}
		return Res;
	}

    // <X(:,ColId),y> where only y is sparse
    template <class Type, class Size = int, bool ColMajor = false>
    static double DotProduct(const TVVec<Type, Size, ColMajor>& X, int ColId, const TIntFltKdV& y) {
		Type Res = 0.0; const Size n = X.GetRows(), yLen = y.Len();
		for (Size i = 0; i < yLen; i++) {
			const Size key = y[i].Key;
			if (key < n) Res += y[i].Dat * X(key, ColId);
		}
		return Res;
	}

	// TEST
	// z = x * y'    
    template <class Type, class Size = int, bool ColMajor = false>
	static void OuterProduct(const TVec<Type, Size>& x,
            const TVec<Type, Size>& y, TVVec<Type, Size, ColMajor>& Z) {
        
		EAssert(Z.GetRows() == x.Len() && Z.GetCols() == y.Len());
		const Size XLen = x.Len();
		const Size YLen = y.Len();
		for (Size i = 0; i < XLen; i++) {
			for (Size j = 0; j < YLen; j++) {
				Z(i, j) = x[i] * y[j];
			}
		}
	}

    // z := p * x + q * y
	//TODO should double be Type?
    template <class Type, class Size = int, bool ColMajor = false>
    static void LinComb(const double& p, const TVec<Type, Size>& x, 
            const double& q, const TVec<Type, Size>& y, TVec<Type, Size>& z) {

		EAssert(x.Len() == y.Len() && y.Len() == z.Len());
		const Size Len = x.Len();
		for (Size i = 0; i < Len; i++) {
			z[i] = p * x[i] + q * y[i];
		}
	}

	//TODO this will work only for glib type TFlt
    template <class Type, class Size = int, bool ColMajor = false>
    static void LinCombInPlace(const Type& alpha, const TVec<TNum<Type>, Size>& x,
        const Type& beta, TVec<TNum<Type>, Size>& y) {
        
        #ifdef BLAS
            if (is_double<Type>::value == true){
                typedef double Loc;
                cblas_daxpby(x.Len(), *((Loc *)&alpha), (Loc *)&x[0].Val, 1, *((Loc *)&beta), (Loc *)&y[0].Val, 1);
            }
            else
            if (is_float<Type>::value == true){
                typedef float Loc;
                cblas_saxpby(x.Len(), *((Loc *)&alpha), (Loc *)&x[0].Val, 1, *((Loc *)&beta), (Loc *)&y[0].Val, 1);
            }
            else
            if (is_complex_double<Type>::value == true){
                typedef double Loc;
                //std::complex<double> alpha_(alpha); std::complex<double> beta_(beta);
                cblas_zaxpby(x.Len(), (const Loc*)&alpha, (const Loc*)&x[0].Val, 1, (const Loc*)&beta, (Loc*)&y[0].Val, 1);
            }
            else
            if (is_complex_float<Type>::value == true){
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
    template <class Type, class Size = int, bool ColMajor = false>
    static void LinComb(const double& p, const TVVec<Type, Size, ColMajor>& X,
		const double& q, const TVVec<Type, Size, ColMajor>& Y, TVVec<Type, Size, ColMajor>& Z) {
		EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols() && X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
		Size Rows = X.GetRows();
		Size Cols = X.GetCols();
		for (Size RowN = 0; RowN < Rows; RowN++) {
			for (Size ColN = 0; ColN < Cols; ColN++) {
				Z.At(RowN, ColN) = p*X.At(RowN, ColN) + q*Y.At(RowN, ColN);
			}
		}
	}
	// z = p * x + q * y
	static void LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z) {
		TSparseOpsIntFlt::SparseLinComb(p, x, q, y, z);
	}

	static void LinComb(const double& p, const TFltVV& X, int ColId,
	       const double& q, const TFltV& y, TFltV& z) {
	
		if (z.Empty()) z.Gen(X.GetRows());
		EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());
	
		const int len = z.Len();
	    for (int i = 0; i < len; i++) {
	        z[i] = p * X(i, ColId) + q * y[i];
	   }
	}
	static void LinComb(const double& p, const TFltVV& X, int DimId,
	        const double& q, const TFltV& y, TFltV& z, int Dim) {
		
		EAssertR(Dim == 1 || Dim == 2, "TLinAlg::LinComb: Invalid value of argument Dim.");
		if (Dim == 1) {
			if (z.Empty()) z.Gen(X.GetRows());
			EAssert(X.GetRows() == y.Len() && y.Len() == z.Len());
		
			const int len = z.Len();
			for (int i = 0; i < len; i++) {
				z[i] = p * X(i, DimId) + q * y[i];
			}
		} else if (Dim == 2) {
			if (z.Empty()) z.Gen(X.GetCols());
			EAssert(X.GetCols() == y.Len() && y.Len() == z.Len());
		
			const int len = z.Len();
			for (int i = 0; i < len; i++) {
				z[i] = p * X(DimId, i) + q * y[i];
			}
		}
	}	
	static void LinComb(const double& p, const TFltVV& X, const double& q, const TFltVV& Y, TFltVV& Z) {
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
    template <class Type, class Size = int, bool ColMajor = false>
	static void ConvexComb(const double& p, const TVec<Type, Size>& x, const TVec<Type, Size>& y, TVec<Type, Size>& z)  {
		AssertR(0.0 <= p && p <= 1.0, TFlt::GetStr(p));
		TLinAlg::LinComb(p, x, 1.0 - p, y, z);
	}

//this will fail if Type != TFlt, Specialization should be used
#ifdef BLAS
	// TEST
	//y = k * x + y 
    template <class Type, class Size = int, bool ColMajor = false>
//	static void AddVec(const double& k, const TVec<TNum<Type>, Size>& x, TVec<TNum<Type>, Size>& y) {
    static void AddVec(const Type& k, const TVec<TNum<Type>, Size>& x, TVec<TNum<Type>, Size>& y) {
		if (is_double<Type>::value == true){
			typedef double Loc;
			cblas_daxpy(x.Len(), *((Loc *)&k), (Loc *)&x[0].Val, 1, (Loc *) &y[0].Val, 1);
		}
		else
		if (is_float<Type>::value == true){
			typedef float Loc;
			cblas_saxpy(x.Len(), *((Loc *)&k), (Loc *)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
		}
		else
		if (is_complex_double<Type>::value == true){
			typedef double Loc;
			cblas_zaxpy(x.Len(), (const Loc *)&k, (const Loc*) &x[0].Val, 1, (Loc *)&y[0].Val, 1);
		}
		else
		if (is_complex_float<Type>::value == true){
			typedef float Loc;
			cblas_caxpy(x.Len(), (const Loc *)&k, (const Loc *)&x[0].Val, 1, (Loc *)&y[0].Val, 1);
		}
		//cblas_daxpy(x.Len(), k, &x[0].Val, 1, &y[0].Val, 1);
	}
#endif
	// TEST
    // z := k * x + y 
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TVec<Type, Size>& x, const TVec<Type, Size>& y, TVec<Type, Size>& z)  {
		TLinAlg::LinComb(k, x, 1.0, y, z);
	}
    // z := k * X[ColId] + y
    //Andrej template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z) {
    	EAssert(0 <= ColId && ColId < X.Len());
    	AddVec(k, X[ColId], y, z);    
	}
    // z := k * X(:,ColId) + y
    //Andrej template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z) {
    	EAssert(X.GetRows() == y.Len());
    	EAssert(y.Len() == z.Len());
    	const int len = z.Len();
    	for (int i = 0; i < len; i++) {
        	z[i] = y[i] + k * X(i, ColId);
    	}
	}
    // z := x + y
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const TVec<Type, Size>& x, const TVec<Type, Size>& y, TVec<Type, Size>& z) {
		TLinAlg::LinComb(1.0, x, 1.0, y, z);
	}

    // z := k * x + y
    //template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TIntFltKdV& x, const TFltV& y, TFltV& z) {
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
	static void AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z) {
        EAssert(0 <= ColId && ColId < X.Len());
        AddVec(k, X[ColId], y, z);
    }
    // y := k * x + y
    //template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TIntFltKdV& x, TFltV& y) {
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
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(double k, const TVVec<Type, Size, ColMajor>& X, Size ColIdX, TVVec<Type, Size, ColMajor>& Y, Size ColIdY) {
		EAssert(X.GetRows() == Y.GetRows());
		const Size len = Y.GetRows();
		for (Size i = 0; i < len; i++) {
			Y(i, ColIdY) = Y(i, ColIdY) + k * X(i, ColIdX);
		}
	}

	// TEST
	// Y(:,ColIdY) += k * x
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const double& k, const TVec<Type, Size>& x, TVVec<Type, Size, ColMajor>& Y, const Size& ColIdY) {
		EAssert(x.Len() == Y.GetRows());
		EAssert(ColIdY >= 0 && ColIdY < x.Len());
		for (Size RowN = 0; RowN < Y.GetRows(); RowN++) {
			Y.At(RowN, ColIdY) += k*x[RowN];
		}
	}

	// TEST
    // Result += k * X(:,Col)
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(double k, const TVVec<Type, Size, ColMajor>& X, int ColId, TVec<Type, Size>& Result) {
		EAssert(X.GetRows() == Result.Len());
		const Size len = Result.Len();
		for (Size i = 0; i < len; i++) {
			Result[i] = Result[i] + k * X(i, ColId);
		}
	}

	// z = x + y
    template <class Type, class Size = int, bool ColMajor = false>
	static void AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z) {
		TSparseOpsIntFlt::SparseMerge(x, y, z);
	}

    // Result = SUM(x)
    template <class Type, class Size = int>
	static double SumVec(const TVec<Type, Size>& x) {
		const Size len = x.Len();
		double Res = 0.0;
		for (int i = 0; i < len; i++) {
			Res += x[i];
		}
		return Res;
	}

	// Result = SUM(x)
    //template <class Type, class Size = int, bool ColMajor = false>
	static double SumVec(const TIntFltKdV& x) {
		const int len = x.Len();
		double Res = 0.0;
		for (int i = 0; i < len; i++) {
			Res += x[i].Dat;
		}
		return Res;
	}

	// TEST
    // Result = SUM(k*x + y)
    template <class Type, class Size = int>
	static double SumVec(double k, const TVec<Type, Size>& x, const TVec<Type, Size>& y) {
		EAssert(x.Len() == y.Len());
		const Size len = x.Len();
		double Res = 0.0;
		for (Size i = 0; i < len; i++) {
			Res += k * x[i] + y[i];
		}
		return Res;
	}

    // Result = ||x-y||^2 (Euclidian)
    template <class Type, class Size = int, bool ColMajor = false>
	static double EuclDist2(const TVec<Type, Size>& x, const TVec<Type, Size>& y) {
		EAssert(x.Len() == y.Len());
		const Size len = x.Len();
		double Res = 0.0;
		for (Size i = 0; i < len; i++) {
			Res += TMath::Sqr(x[i] - y[i]);
		}
		return Res;
	}

    // Result = ||x-y||^2 (Euclidian)
	static double EuclDist2(const TFltPr& x, const TFltPr& y) {
		return TMath::Sqr(x.Val1 - y.Val1) + TMath::Sqr(x.Val2 - y.Val2);
	}

	// TEST
    // Result = ||x-y|| (Euclidian)
    template <class Type, class Size = int>
	static double EuclDist(const TVec<Type, Size>& x, const TVec<Type, Size>& y) {
		return sqrt(TLinAlg::EuclDist2(x, y));
	}

    // Result = ||x-y|| (Euclidian)
    //template <class Type, class Size = int, bool ColMajor = false>
	static double EuclDist(const TFltPr& x, const TFltPr& y) {
		return sqrt(TLinAlg::EuclDist2(x, y));
	}
	// Result = ||A||_F (Frobenious)
    template <class Type, class Size = int, bool ColMajor = false>
    static Type Frob(const TVVec<TNum<Type>, Size, ColMajor> &A) {
        Type frob = 0;
        for (int RowN = 0; RowN < A.GetRows(); RowN++) {
            for (int ColN = 0; ColN < A.GetCols(); ColN++) {
                frob += A.At(RowN, ColN)*A.At(RowN, ColN);
            }
        }
        return sqrt(frob);
    }
	// TEST
	// Result = ||A - B||_F (Frobenious)
    template <class Type, class Size = int, bool ColMajor = false>
	static double FrobDist2(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B) {
		double frob = 0;
		TVec<Type, Size> Apom = (const_cast<TVVec<Type, Size, ColMajor> &>(A)).Get1DVec();
		TVec<Type, Size> Bpom = (const_cast<TVVec<Type, Size, ColMajor> &>(B)).Get1DVec();
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
    template <class Type, class Size = int, bool ColMajor = false>
	static double FrobDist2(const TVec<Type, Size>& A, const TVec<Type, Size>& B) {
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
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Sparse(const TVVec<Type, Size, ColMajor>& A, TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& B){
		B.Val1.Gen(0);
		B.Val2.Gen(0);
		B.Val3.Gen(0);
		for (Size RowN = 0; RowN < A.GetRows(); RowN++) {
			for (Size  ColN = 0; ColN < A.GetCols(); ColN++) {
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
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Sparse(const TVVec<Type, Size, ColMajor>& A, TVec<TIntFltKdV>& B){
		Size Cols = A.GetCols();
		Size Rows = A.GetRows();
		B.Gen(Cols);
		for (Size ColN = 0; ColN < Cols; ColN++) {
			B[ColN].Gen(0);
			for (Size RowN = 0; RowN < Rows; RowN++) {
				if (A.At(RowN, ColN) != 0.0) {
					B[ColN].Add(TIntFltKd(RowN, A.At(RowN, ColN)));
				}
			}
		}
	}

	// TEST
	// Sparse to dense transform
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Full(const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& A, TVVec<Type, Size, ColMajor>& B, const int Rows, const int Cols) {
		B.Gen(Rows, Cols);
		B.PutAll(0.0);
		Size nnz = A.Val1.Len();
		for (Size ElN = 0; ElN < nnz; ElN++) {
			B.At(A.Val1[ElN], A.Val2[ElN]) = A.Val3[ElN];
		}
	}
	
	// Sparse to dense transform
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Full(const TVec<TIntFltKdV, Size>& A, TVVec<Type, Size, ColMajor>& B, Size Rows){
		Size Cols = A.Len();
		B.Gen(Rows, Cols);
		B.PutAll(0.0);
		for (Size ColN = 0; ColN < Cols; ColN++) {
			Size Els = A[ColN].Len();
			for (Size ElN = 0; ElN < Els; ElN++) {
				B.At(A[ColN][ElN].Key, ColN) = A[ColN][ElN].Dat;
			}
		}
	}

	// TEST
	// Transpose
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Transpose(const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& A, 
		TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& At) {
		Size nnz = A.Val1.Len();
		At.Val1.Gen(nnz, 0);
		At.Val2.Gen(nnz, 0);
		At.Val3.Gen(nnz, 0);
		TVec<Size, Size> index;
		TIntV::SortGetPerm(A.Val2, At.Val1, index);
		for (Size ElN = 0; ElN < nnz; ElN++) {
			//At.Val1.Add(A.Val2[ElN]);
			At.Val2.Add(A.Val1[index[ElN]]);
			At.Val3.Add(A.Val3[index[ElN]]);
		}
	}

	// Transpose
	//TODO Index template
	static void Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows = -1){
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
	static void  Sign(const TVec<TIntFltKdV>& Mat, TVec<TIntFltKdV>& Mat2) {
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
	static void Convert(const TVec<TPair<TIntV, TFltV>>& A, TTriple<TIntV, TIntV, TFltV>& B) {
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
	static void Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>&B) {
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
	template <class Type, class Size = int, bool ColMajor = false>
	static void Sum(const TVVec<Type, Size, ColMajor>& X, TVec<Type, Size>& y, const int Dimension = 1){
		Size Cols = X.GetCols();
		Size Rows = X.GetRows();
		if (Dimension == 1) {
			y.Gen(Cols);
			for (Size ColN = 0; ColN < Cols; ColN++) {
				for (Size RowN = 0; RowN < Rows; RowN++) {
					y[ColN] += X.At(RowN, ColN);
				}
			}
		}
		else if (Dimension == 2) {
			y.Gen(Rows);
			for (Size ColN = 0; ColN < Cols; ColN++) {
				for (Size RowN = 0; RowN < Rows; RowN++) {
					y[RowN] += X.At(RowN, ColN);
				}
			}
		}
		else FailR("Dimension should be 1 or 2");
	}

	// TEST
	// sum columns (Dimesnion = 2) or rows (Dimension = 1) and store them in vector y
	template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void Sum(const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& X, TVec<Type, Size>& y, const int Dimension = 1) {
		Size Cols = X.Val2.GetMxVal() + 1;
		Size Rows = X.Val1.GetMxVal() + 1;
		Size Els = X.Val1.Len();
		if (Dimension == 1) {
			y.Gen(Cols);
			for (Size ElN = 0; ElN < Els; ElN++) {
				//int RowN = X.Val1[ElN];
				Size ColN = X.Val2[ElN];
				y[ColN] += X.Val3[ElN];
			}
		}
		else if (Dimension == 2) {
			y.Gen(Rows);
			for (Size ElN = 0; ElN < Els; ElN++) {
				Size RowN = X.Val1[ElN];
				//int ColN = X.Val2[ElN];
				y[RowN] += X.Val3[ElN];
			}
		}
		else FailR("Dimension should be 1 or 2");
	}

	// TEST
    // ||x||^2 (Euclidian)
	template <class Type, class Size = int, bool ColMajor = false>
	static double Norm2(const TVec<Type, Size>& x) {
		return TLinAlg::DotProduct(x, x);
	}
	
    // ||x|| (Euclidian)
	template <class Type, class Size = int, bool ColMajor = false>
	static double Norm(const TVec<Type, Size>& x) {
		return sqrt(TLinAlg::Norm2(x));
	}

	//Andrej switch this to TNum<Type>
	// TEST
    // x := x / ||x||
	template <class Type, class Size = int, bool ColMajor = false>
	static double Normalize(TVec<Type, Size>& x) {
		const double xNorm = TLinAlg::Norm(x);
		if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
		return xNorm;
	}

	// TEST
	// Normalize X(:,ColId)
	template <class Type, class Size = int, bool ColMajor = false>
	static void NormalizeColumn(TVVec<Type, Size, ColMajor>& X, const Size& ColId) {
		double nX = TLinAlg::Norm(X, ColId);
		if (nX > 0.0) {
			for (Size RowN = 0; RowN < X.GetRows(); RowN++) {
				X.At(RowN, ColId) /= nX;
			}
		}
	}

	// TEST
	// Normalize the columns of X
	template <class Type, class Size = int, bool ColMajor = false>
	static void NormalizeColumns(TVVec<Type, Size, ColMajor>& X) {
		for (Size ColN = 0; ColN < X.GetCols(); ColN++) {
			TLinAlg::NormalizeColumn(X, ColN);
		}
	}
	template <class Type, class Size = int, bool ColMajor = false>
	static void NormalizeRows(TVVec<Type, Size, ColMajor>& X) {
		for (Size RowN = 0; RowN < X.GetRows(); RowN++) {
			TVec<Type> Row;
			X.GetRowPtr(RowN, Row);
			Normalize(Row);
		}
	}

#ifdef INTEL
	// TEST
	template <class Type, class Size = int, bool ColMajor = false>
	static void NormalizeColumns(TVVec<Type, Size, ColMajor>& X, TBool ColumnMajor) {
		const Size m = X.GetXDim();
		const Size n = X.GetYDim();
		TVVec<Type, Size, ColMajor> sqrX(m, n);
		vdSqr(m*n, &X(0, 0).Val, &sqrX(0, 0).Val);
		printf("Squaring of elements done!\n");
		TVec<Type, Size> enke(m); TVec<Type, Size> sumsqr(n);  TVec<Type, Size> norme(n); TLAMisc::Fill(enke, 1.0);
		TLinAlg::MultiplyT(sqrX, enke, sumsqr);
		printf("Summing elemnents done!\n");
		vdInvSqrt(n, &sumsqr[0].Val, &norme[0].Val);
		printf("Summing and inverting elemnents done!\n");
		// added code
		if (ColMajor) {
			TVVec<Type, Size, ColMajor> B; B.Gen(n, m);
			TLinAlg::Transpose(X, B);

			for (Size i = 0; i < m; i++) {
				vdMul(n, &norme[0].Val, &B(0, i).Val, &B(0, i).Val);
			}
			TLinAlg::Transpose(B, X);
		}
		else {
			for (Size i = 0; i < m; i++){
				vdMul(n, &norme[0].Val, &X(i, 0).Val, &X(i, 0).Val);
			}
		}
		//TLAMisc::PrintTFltVV(X, "Normalizirana");
	}
#endif

	// Normalize the columns of X
	//TODO what to do when number
	//MARK	
    template <class Type, class Size = int, bool ColMajor = false, class IndexType = TInt>
	static void NormalizeColumns(TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& X) {
		if (X.Val2.Len() == 0) return;
		EAssert(X.Val2.IsSorted(true));
		//int?
		int Cols = X.Val2.GetMxVal() + 1;
		TVec<Type, Size> InvColNorms(Cols); //get the last element colN and set the number of elements	

		Size Els = X.Val1.Len();
		for (Size ElN = 0; ElN < Els; ElN++) {
			InvColNorms[X.Val2[ElN]] += X.Val3[ElN] * X.Val3[ElN];
		}
		for (Size ColN = 0; ColN < Cols; ColN++) {
			if (InvColNorms[ColN] > 0.0) {
				InvColNorms[ColN] = 1.0 / TMath::Sqrt(InvColNorms[ColN]);
			}
		}
		for (Size ElN = 0; ElN < Els; ElN++) {
			X.Val3[ElN] *= InvColNorms[X.Val2[ElN]];
		}
	}

	// Normalize the columns of X
	template<class Size = int>
	static void NormalizeColumns(TVec<TIntFltKdV, Size>& X) {
		Size Cols = X.Len();
		for (Size ElN = 0; ElN < Cols; ElN++) {
			TLinAlg::Normalize(X[ElN]);
		}
	}
	// Frobenius norm of matrix A
	// TEST
	template <class Type, class Size = int, bool ColMajor = false>
	static double FrobNorm2(const TVVec<Type, Size, ColMajor>& X) {
		return TLinAlg::Norm2((const_cast<TVVec<Type, Size, ColMajor> &>(X)).Get1DVec());
	}
	template <class Type, class Size = int, bool ColMajor = false>
	static double FrobNorm(const TVVec<Type, Size, ColMajor>& X) {
		return sqrt(TLinAlg::FrobNorm2(X));
	}

    // ||x||^2 (Euclidian), x is sparse
    template<class Size = int>
	static double Norm2(const TVec<TIntFltKdV, Size>& x) {
		double Result = 0;
		for (Size i = 0; i < x.Len(); i++) {
			Result += TMath::Sqr(x[i].Dat);
		}
		return Result;
	}

    // ||x|| (Euclidian), x is sparse
	template<class Size = int>
	static double Norm(const TVec<TIntFltKdV, Size>& x) {
		return sqrt(Norm2(x));
	}

    // x := x / ||x||, x is sparse
	template<class Size = int, Size>
	static void Normalize(TVec<TIntFltKdV>& x) {
		double Normx = TLinAlg::Norm(x);
		if (Normx > 0) {
			TLinAlg::MultiplyScalar(1 / Normx, x, x);
		}
	}

    // ||X(:,ColId)||^2 (Euclidian)
	template <class Type, class Size = int, bool ColMajor = false>
	static double Norm2(const TVVec<Type, Size, ColMajor>& X, int ColId) {
		return TLinAlg::DotProduct(X, ColId, X, ColId);
	}

	// TEST
    // ||X(:,ColId)|| (Euclidian)
	template <class Type, class Size = int, bool ColMajor = false>
	static double Norm(const TVVec<Type, Size, ColMajor>& X, int ColId) {
		return sqrt(TLinAlg::Norm2(X, ColId));
	}

    // L1 norm of x (Sum[|xi|, i = 1..n])
	template <class Type, class Size = int>
	static double NormL1(const TVec<Type, Size>& x)  {
		double norm = 0.0; const Size Len = x.Len();
		for (Size i = 0; i < Len; i++)
			norm += TFlt::Abs(x[i]);
		return norm;
	}

	// TEST
    // L1 norm of k*x+y (Sum[|k*xi+yi|, i = 1..n])
	template <class Type, class Size = int>
	static double NormL1(double k, const TVec<Type, Size>& x, const TVec<Type, Size>& y) {
		EAssert(x.Len() == y.Len());
		double norm = 0.0; const Size len = x.Len();
		for (Size i = 0; i < len; i++) {
			norm += TFlt::Abs(k * x[i] + y[i]);
		}
		return norm;
	}

    // L1 norm of x (Sum[|xi|, i = 1..n])


	static double NormL1(const TIntFltKdV& x) {
		double norm = 0.0; const int Len = x.Len();
		for (int i = 0; i < Len; i++)
			norm += TFlt::Abs(x[i].Dat);
		return norm;
	}

	// TEST
    // x := x / ||x||_1
	template <class Type, class Size = int>
	static void NormalizeL1(TVec<Type, Size>& x) {
		const double xNorm = TLinAlg::NormL1(x);
		if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
	}

    // x := x / ||x||_1
	static void NormalizeL1(TIntFltKdV& x) {
		const double xNorm = TLinAlg::NormL1(x);
		if (xNorm > 0.0) { TLinAlg::MultiplyScalar(1 / xNorm, x, x); }
	}

	// TEST
    // Linf norm of x (Max{|xi|, i = 1..n})
	template <class Type, class Size = int>
	static double NormLinf(const TVec<Type, Size>& x) {
		double norm = 0.0; const Size Len = x.Len();
		for (Size i = 0; i < Len; i++)
			norm = TFlt::GetMx(TFlt::Abs(x[i]), norm);
		return norm;
	}

    // Linf norm of x (Max{|xi|, i = 1..n})
	static double NormLinf(const TIntFltKdV& x) {
		double norm = 0.0; const int Len = x.Len();
		for (int i = 0; i < Len; i++)
			norm = TFlt::GetMx(TFlt::Abs(x[i].Dat), norm);
		return norm;
	}

	// TEST
    // x := x / ||x||_inf
	template <class Type, class Size = int>
	static void NormalizeLinf(TVec<Type, Size>& x) {
		const double xNormLinf = TLinAlg::NormLinf(x);
		if (xNormLinf > 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLinf, x, x); }
	}

    // x := x / ||x||_inf, , x is sparse
 	static void NormalizeLinf(TIntFltKdV& x) {
		const double xNormLInf = TLinAlg::NormLinf(x);
		if (xNormLInf> 0.0) { TLinAlg::MultiplyScalar(1.0 / xNormLInf, x, x); }
	}

 	// stores the squared norm of all the columns into the output vector
 	static void GetColNormV(const TFltVV& X, TFltV& ColNormV) {
 		const int Cols = X.GetCols();
 		GetColNorm2V(X, ColNormV);
 		for (int i = 0; i < Cols; i++) {
 			ColNormV[i] = sqrt(ColNormV[i]);
 		}
 	}

 	// stores the norm of all the columns into the output vector
 	static void GetColNorm2V(const TFltVV& X, TFltV& ColNormV) {
 		const int Cols = X.GetCols();
 		ColNormV.Gen(Cols);
 		for (int i = 0; i < Cols; i++) {
 			ColNormV[i] = Norm2(X, i);
 		}
 	}

	// TEST
	// find the index of maximum elements for a given row of X
	template <class Type, class Size = int, bool ColMajor = false>
	static int GetRowMaxIdx(const TVVec<Type, Size, ColMajor>& X, const Size& RowN) {
		Size Idx = -1;
		Size Cols = X.GetCols();
		double MaxVal = TFlt::Mn;
		for (Size ColN = 0; ColN < Cols; ColN++) {
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
	template <class Type, class Size = int, bool ColMajor = false>
	static int GetColMaxIdx(const TVVec<Type, Size, ColMajor>& X, const int& ColN) {
		Size Idx = -1;
		Size Rows = X.GetRows();
		double MaxVal = TFlt::Mn;
		for (Size RowN = 0; RowN < Rows; RowN++) {
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
	template <class Type, class Size = int, bool ColMajor = false>
	static void GetRowMaxIdxV(const TVVec<Type, Size, ColMajor>& X, TVec<TInt, Size>& IdxV) {
		IdxV.Gen(X.GetRows());
		Size Rows = X.GetRows();
		for (Size RowN = 0; RowN < Rows; RowN++) {
			IdxV[RowN] = TLinAlg::GetRowMaxIdx(X, RowN);
		}
	}
	// find the index of maximum elements for each col of X

	template <class Type, class Size = int, bool ColMajor = false>
	static void GetColMaxIdxV(const TVVec<Type, Size, ColMajor>& X, TVec<TInt, Size>& IdxV) {
		IdxV.Gen(X.GetCols());
		Size Cols = X.GetCols();
		for (Size ColN = 0; ColN < Cols; ColN++) {
			IdxV[ColN] = TLinAlg::GetColMaxIdx(X, ColN);
		}
	}
	//x := k * x
	// TEST
	//x := k * x
	template <class Type, class Size = int>
	static void MultiplyScalar(const double& k, TVec<Type, Size>& x) {
		Size Len = x.Len();
		for (Size i = 0; i < Len; i++)
			x[i] = k * x[i];
	}
	// find the index of maximum elements for a given each col of X
	static int GetColMinIdx(const TFltVV& X, const int& ColN) {
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
	static void GetColMinIdxV(const TFltVV& X, TIntV& IdxV) {
		int Cols = X.GetCols();
		IdxV.Gen(X.GetCols());
		for (int ColN = 0; ColN < Cols; ColN++) {
			IdxV[ColN] = GetColMinIdx(X, ColN);
		}
	}

	template <class TVal> static TVal GetColMin(const TVVec<TVal>& X, const int& ColN);
	template <class TVal> static void GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV);
	// TEST
    // y := k * x
	template <class Type, class Size = int>
	static void MultiplyScalar(const double& k, const TVec<Type, Size>& x, TVec<Type, Size>& y) {
		EAssert(x.Len() == y.Len());
		Size Len = x.Len();
		for (Size i = 0; i < Len; i++)
			y[i] = k * x[i];
	}
    // y := k * x
	static void MultiplyScalar(const double& k, const TIntFltKdV& x, TIntFltKdV& y) {
		EAssert(x.Len() == y.Len());
		int Len = x.Len();
		for (int i = 0; i < Len; i++) {
			y[i].Key = x[i].Key;
			y[i].Dat = k * x[i].Dat;
		}
	}
	// TEST
    // Y := k * X
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyScalar(const double& k, const TVVec<Type, Size, ColMajor>& X, TVVec<Type, Size, ColMajor>& Y) {
		EAssert(X.GetRows() == Y.GetRows() && X.GetCols() == Y.GetCols());
		const Size Rows = X.GetRows();
		const Size Cols = X.GetCols();
		for (Size i = 0; i < Rows; i++) {
			for (Size j = 0; j < Cols; j++) {
				Y(i, j) = k*X(i, j);
			}
		}
	}
	// Y := k * X
	template <class Size = int>
	static void MultiplyScalar(const double& k, const TVec<TIntFltKdV, Size>& X, TVec<TIntFltKdV, Size>& Y) {
		// sparse column matrix
		Y = X;
		Size Cols = X.Len();
		for (Size ColN = 0; ColN < Cols; ColN++) {
			Size Els = X[ColN].Len();
			for (int ElN = 0; ElN < Els; ElN++) {
				Y[ColN][ElN].Dat = k * X[ColN][ElN].Dat;
			}
		}
	}
    
    // y := A * x
#ifdef BLAS
    template <class Type, class Size = int, bool ColMajor = false>
    static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVec<Type, Size>& x, TVec<Type, Size>& y) {
        TLinAlg::Multiply(A, x, y, TLinAlgBlasTranspose::NOTRANS, 1.0, 0.0);
    }
#else
    template <class Type, class Size = int, bool ColMajor = false>
    static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVec<Type, Size>& x, TVec<Type, Size>& y) {
        if (y.Empty()) y.Gen(A.GetRows());
        EAssert(A.GetCols() == x.Len() && A.GetRows() == y.Len());
        int n = A.GetRows(), m = A.GetCols();
        for (int i = 0; i < n; i++) {
            y[i] = 0.0;
            for (int j = 0; j < m; j++)
                y[i] += A(i, j) * x[j];
        }
    }
#endif

	// TEST
    // C(:, ColId) := A * x
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A,
			const TVec<Type, Size>& x, TVVec<Type, Size, ColMajor>& C, Size ColId) {
		EAssert(A.GetCols() == x.Len() && A.GetRows() == C.GetRows());
		Size n = A.GetRows(), m = A.GetCols();
		for (Size i = 0; i < n; i++) {
			C(i, ColId) = 0.0;
			for (Size j = 0; j < m; j++)
				C(i, ColId) += A(i, j) * x[j];
		}
	}

	// TEST
    // y := A * B(:, ColId)
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, int ColId, TVec<Type, Size>& y)  {
		EAssert(A.GetCols() == B.GetRows() && A.GetRows() == y.Len());
		Size n = A.GetRows(), m = A.GetCols();
		for (Size i = 0; i < n; i++) {
			y[i] = 0.0;
			for (Size j = 0; j < m; j++)
				y[i] += A(i, j) * B(j, ColId);
		}
	}

	// TEST
    // C(:, ColIdC) := A * B(:, ColIdB)
    template <class Type, class Size = int, bool ColMajor = false>
    static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, int ColIdB, TVVec<Type, Size, ColMajor>& C, int ColIdC) {
        EAssert(A.GetCols() == B.GetRows() && A.GetRows() == C.GetRows());
        Size n = A.GetRows(), m = A.GetCols();
        for (Size i = 0; i < n; i++) {
            C(i, ColIdC) = 0.0;
            for (Size j = 0; j < m; j++)
                C(i, ColIdC) += A(i, j) * B(j, ColIdB);
        }
    }


//LAPACKE stuff
#ifdef LAPACKE
    // Tested in other function
    //A is rewritten in place with orthogonal matrix Q
    template <class Type, class Size = int, bool ColMajor = false>
    static void QRbasis(TVVec<Type, Size, ColMajor>& A) {
        Size m = A.GetRows(); Size n = A.GetCols(); Size k = A.GetCols();
        Size lda = ColMajor ? m : n;
        int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
        TVec<Type, Size> tau; tau.Gen(MAX(1, MIN(m, n)));
        LAPACKE_dgeqrf(Matrix_Layout, m, n, &A(0, 0).Val, lda, &tau[0].Val);
        LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
    }

    // TEST
    template <class Type, class Size = int, bool ColMajor = false>
    static void QRbasis(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& Q) {
        Q = A;
        TLinAlg::QRbasis(Q);
    }

    // Tested in other function
    //A is rewritten in place with orthogonal matrix Q (column pivoting to improve stability)
    template <class Type, class Size = int, bool ColMajor = false>
    static void QRcolpbasis(TVVec<Type, Size, ColMajor>& A) {
        Size m = A.GetRows(); Size n = A.GetCols(); Size k = A.GetCols();
        Size lda = ColMajor ? m : n;
        Size Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
        TVec<Type, Size> tau(MAX(1, MIN(m, n)));
        TVec<TInt, Size> jvpt(MAX(1, n));
        LAPACKE_dgeqp3(Matrix_Layout, m, n, &A(0, 0).Val, lda, &jvpt[0].Val, &tau[0].Val);
        LAPACKE_dorgqr(Matrix_Layout, m, n, k, &A(0, 0).Val, lda, &tau[0].Val);
    }

    // TEST
    template <class Type, class Size = int, bool ColMajor = false>
    static void QRcolpbasis(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& Q) {
        Q = A;
        TLinAlg::QRcolpbasis(Q);
    }

    // TEST
    //S S option ensures that A is not modified
    template <class Type, class Size = int, bool ColMajor = false>
    static void thinSVD(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& U, TVec<Type, Size>& S, TVVec<Type, Size, ColMajor>& VT) {

        Size m = A.GetRows(); Size n = A.GetCols();
        Size thin_dim = MIN(m, n);

        S.Gen(thin_dim); U.Gen(m, thin_dim); VT.Gen(thin_dim, n);

        int lda = ColMajor ? m : n;
        int ldu = ColMajor ? m : thin_dim;
        int ldvt = ColMajor ? thin_dim : n;

        TVec<Type, Size> superb(MAX(1, MIN(m, n)));
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
	static int ComputeThinSVD(const TMatrix& X, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters = 2, const double Tol = 1e-6);	

//Full matrix times sparse vector
	//No need to reserve anything outside, functions currently take care of memory managment for safety
	/*template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(TFltVV& ProjMat, TPair<TIntV, TFltV> &, TFltVV& result) {
	};
    template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TFltVV& ProjMat, const TPair<TIntV, TFltV> &, TFltVV& result) {
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
	template <class Type, class Size, bool ColMajor = false>
	static void MultiplySF(const TTriple<TVec<TNum<Size>, Size>, TVec<TNum<Size>, Size>, TVec<Type, Size>>& A, const TVVec<Type, Size, false>& B, 
		TVVec<Type, Size, ColMajor>& C, const TStr& transa = TStr("N"), const int& format = 0){
		//B is row_major 
		Size m, n, k, ldb, ldc;

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
		Size nnz = A.Val3.Len();
		if (format == 0){
			MKL_DCOOMM(const_cast<char *>(transa.CStr()), &m, &n, &k, &alpha, matdescra, const_cast<double *>(&A.Val3[0].Val), const_cast<Size *>(&A.Val1[0].Val), const_cast<Size *>(&A.Val2[0].Val), &nnz, const_cast<double *>(&B(0, 0).Val), &ldb, &beta, const_cast<double *>(&C(0, 0).Val), &ldc);
		}
		else{
			//call mkl_dcsrmm(transa, m, n, k, alpha, matdescra, val, indx, pntrb, pntre, b, ldb, beta, c, ldc)
			printf("Max row %d, max column %d\n", A.Val1.Len() - 1, A.Val2.Len());
			mkl_dcsrmm(const_cast<char *>(transa.CStr()), &m, &n, &k, &alpha, matdescra, const_cast<double *>(&A.Val3[0].Val), const_cast<Size *>(&A.Val2[0].Val), const_cast<Size *>(&A.Val1[0].Val), const_cast<Size *>(&A.Val1[1].Val), const_cast<double *>(&B(0, 0).Val), &ldb, &beta, const_cast<double *>(&C(0, 0).Val), &ldc);
		}

	}

	// TEST
	//B will not be needed anymore (works only for rowmajor!)
	//TODO to much hacking
	template <class IndexType = TInt, class Type, class Size = int, bool ColMajor = false>
	static void MultiplyFS(TVVec<Type, Size, ColMajor>& B, const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& A, 
		TVVec<Type, Size, ColMajor>& C){
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
	template <class IndexType = TInt, class Type, class Size = int,  bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TPair<TVec<IndexType, Size>, TVec<Type, Size>>& x, TVec<Type, Size>& y) {
		// Assumptions on x        
		EAssert(x.Val1.Len() == x.Val2.Len());
		// Dimensions must match
		EAssert(A.GetRows() >= (x.Val1.Len() == 0 ? 0 : x.Val1[x.Val1.GetMxValN()] + 1) && A.GetCols() == y.Len());
		for (Size RowN = 0; RowN < A.GetRows(); RowN++) {
			y[RowN] = 0.0;
			for (Size ElN = 0; ElN < x.Val1.Len(); ElN++) {
				y[RowN] += A.At(RowN, x.Val1[ElN]) * x.Val2[ElN];
			}
		}
	}
	//y  := x' * A ... row data!!
	template <class IndexType = TInt, class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TPair<TVec<IndexType, Size>, TVec<Type, Size>>& x, const TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& y) {
		// Assumptions on x        
		EAssert(x.Val1.Len() == x.Val2.Len());
		// Dimensions must match
		EAssert(A.GetCols() >= (x.Val1.Len() == 0 ? 0 : x.Val1[x.Val1.GetMxValN()] + 1) && A.GetRows() == y.Len());
		TLAMisc::FillZero(y);
		int nnz = x.Val1.Len();
		for (Size i = 0; i < nnz; i++) {
			TVec<Type, Size> row;
			(const_cast<TVVec<Type, Size, ColMajor> &>(A)).GetRowPtr(x.Val1[i], row);
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
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TVVec<Type, Size, ColMajor>& A, const TVec<Type, Size>& x, TVec<Type, Size>& y) {
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
	typedef enum { NOTRANS = 0, TRANS = 1} TLinAlgBlasTranspose;

	// TEST
	// C = op(A) * op(B)
	template <class Type, class Size = int, bool ColMajor = false>
	inline
	static void Multiply(const TVVec<TNum<Type>, Size, ColMajor>& A, const TVVec<TNum<Type>, Size, ColMajor>& B, TVVec<TNum<Type>, Size, ColMajor>& C, 
		const int& BlasTransposeFlagA, const int& BlasTransposeFlagB) {
		//C := alpha*op(A)*op(B) + beta*C,
		//where:
		//op(X) is one of op(X) = X, or op(X) = XT, or op(X) = XH,
		//alpha and beta are scalars,
		//A, B and C are matrices:
		//op(A) is an m-by-k matrix,
		//op(B) is a k-by-n matrix,
		//C is an m-by-n matrix.
//		int m, k, n;
//		//if (BlasTransposeFlagA == TLinAlgBlasTranspose::TRANS){
//		//	m = A.GetCols(), k = A.GetRows();
//		//}
//		//else{
//		//	m = A.GetRows(), k = A.GetCols();
//		//}
//		//if (BlasTransposeFlagB == TLinAlgBlasTranspose::TRANS){
//		//	EAssert(k == B.GetCols());
//		//	n = B.GetRows();
//		//}
//		//else{
//		//	EAssert(k == B.GetRows());
//		//	n = B.GetCols();
//		//}
//		
//		m = A.GetRows(), k = A.GetCols();
//			
//		EAssert(k = B.GetRows());
//		n = B.GetCols();
//
//		EAssert(m == C.GetRows() && n == C.GetCols());
//		//Simplified interface
//		double alpha = 1.0; double beta = 0.0;
//
//		int lda, ldb, ldc;
//		/*if (BlasTransposeFlagA == TLinAlgBlasTranspose::TRANS) {
//			lda = ColMajor ? k : m;
//		}
//		else {
//			lda = ColMajor ? m : k;
//		}
//		if (BlasTransposeFlagB == TLinAlgBlasTranspose::TRANS) {
//			ldb = ColMajor ? n : k;
//		}
//		else {
//			ldb = ColMajor ? k : n;
//		}*/
//		lda = ColMajor ? m : k;
//		ldb = ColMajor ? k : n;
//		ldc = ColMajor ? m : n;
////
////#ifdef CBLAS //Standard CBLAS interface
////		CBLAS_TRANSPOSE BlasTransA = CblasNoTrans, BlasTransB = CblasNoTrans;
////		CBLAS_ORDER Matrix_Layout = ColMajor ? CblasColMajor : CblasRowMajor;
////		if (ColMajor == true) {
////			if (BlasTransposeFlagA){ BlasTransA = CblasTrans; }
////			if (!BlasTransposeFlagA){ lda = k; }
////			if (BlasTransposeFlagB){ BlasTransB = CblasTrans; }
////			if (!BlasTransposeFlagB){ ldb = n; }
////		}
////		else {
////			if (BlasTransposeFlagA){ BlasTransA = CblasTrans; lda = k; }
////			if (BlasTransposeFlagB){ BlasTransB = CblasTrans; ldb = n; }
////		}
//	
//		//EAssert(k == B.GetRows() && m == C.GetRows() && n == C.GetCols());
//		////Simplified interface
//		//double alpha = 1.0; double beta = 0.0;
//
//		//int lda, ldb, ldc;
//		//if (BlasTransposeFlagA == TLinAlgBlasTranspose::TRANS) { lda = ColMajor ? k : m; }
//		//else { lda = ColMajor ? m : k; }
//
//		//if (BlasTransposeFlagB == TLinAlgBlasTranspose::TRANS) { lda = ColMajor ? n : k; }
//		//else { lda = ColMajor ? k : n; }

		Size m, n, k, lda, ldb, ldc;
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
		if (is_double<Type>::value == true){
			typedef double Loc;
			double alpha = 1.0, beta = 0.0;
			cblas_dgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, alpha, (Loc *)&A(0, 0).Val, lda, (Loc *)&B(0, 0).Val, ldb, beta, (Loc *)&C(0, 0).Val, ldc);
		}
		else
		if (is_float<Type>::value == true){
			typedef float Loc;
			float alpha = 1.0f, beta = 0.0f;
			cblas_sgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, alpha, (Loc *)&A(0, 0).Val, lda, (Loc *)&B(0, 0).Val, ldb, beta, (Loc *)&C(0, 0).Val, ldc);
		}
		else
		if (is_complex_double<Type>::value == true){
			typedef double Loc;
			std::complex<double> alpha(1.0); std::complex<double> beta(0.0);
			cblas_zgemm(Matrix_Layout, BlasTransA, BlasTransB, m, n, k, (const Loc *)&alpha, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&B(0, 0).Val, ldb, (const Loc *)&beta, (Loc *)&C(0, 0).Val, ldc);
		}
		else
		if (is_complex_float<Type>::value == true){
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
	//Andrej ToDo In the future replace Type with TNum<type> and change double to type
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<TNum<Type>, Size, ColMajor>& A, const TVec<TNum<Type>, Size>& x, TVec<TNum<Type>, Size>& y, const int& BlasTransposeFlagA, Type alpha = 1.0, Type beta = 0.0) {
		Size m = A.GetRows();
		Size n = A.GetCols();
		//Can we multiply and store in y?
		if (BlasTransposeFlagA)//A'*x n*m x m -> n
			EAssert(x.Len() == m && y.Reserved() == n);
		else{//A*x  m x n * n -> m
			EAssert(x.Len() == n && y.Reserved() == m);
		}
		Size lda = ColMajor ? m : n;
		Size incx = /*ColMajor ? x.Len() :*/ 1;
		Size incy = /*ColMajor ? y.Len() :*/ 1;
		CBLAS_ORDER Matrix_Layout = ColMajor ? CblasColMajor : CblasRowMajor;

#ifdef BLAS //Standard CBLAS interface
		CBLAS_TRANSPOSE BlasTransA = BlasTransposeFlagA ? CblasTrans : CblasNoTrans;
		/*if (BlasTransposeFlagA){ BlasTransA = CblasTrans; }*/
		if (is_double<Type>::value == true){
			typedef double Loc;
			double alpha_ = alpha; double beta_ = beta;
			cblas_dgemv(Matrix_Layout, BlasTransA, m, n, alpha_, (Loc *)&A(0, 0).Val, lda, (Loc *)&x[0].Val, incx, beta_, (Loc *)&y[0].Val, incy);
		}
		else
		if (is_float<Type>::value == true){
			typedef float Loc;
			float alpha_ = (float)alpha; float beta_ = (float) beta;
			cblas_sgemv(Matrix_Layout, BlasTransA, m, n, alpha_, (Loc *)&A(0, 0).Val, lda, (Loc *)&x[0].Val, incx, beta_, (Loc *)&y[0].Val, incy);
		}
		else
		if (is_complex_double<Type>::value == true){
			typedef double Loc;
			std::complex<double>  alpha_(alpha); std::complex<double>  beta_(beta);
			cblas_zgemv(Matrix_Layout, BlasTransA, m, n, (const Loc *)&alpha_, (const Loc *)&A(0, 0).Val, lda, (const Loc *)&x[0].Val, incx, (const Loc *)&beta_, (Loc *)&y[0].Val, incy);
		}
		else
		if (is_complex_float<Type>::value == true){
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

#ifdef BLAS
	// TEST
    // C = A * B
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, TVVec<Type, Size, ColMajor>& C) {
		EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetCols() == B.GetRows());
		TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::NOTRANS, TLinAlgBlasTranspose::NOTRANS);
	}
#else
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, TVVec<Type, Size, ColMajor>& C) {
		EAssert(A.GetRows() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetCols() == B.GetRows());
		Size n = C.GetRows(), m = C.GetCols(), l = A.GetCols();
		for (Size i = 0; i < n; i++) {
			for (Size j = 0; j < m; j++) {
				double sum = 0.0;
				for (Size k = 0; k < l; k++)
					sum += A(i, k)*B(k, j);
				C(i, j) = sum;
			}
		}
	}
#endif

#ifdef BLAS
	// TEST
	// C = A' * B
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, TVVec<Type, Size, ColMajor>& C) {
		TLinAlg::Multiply(A, B, C, TLinAlgBlasTranspose::TRANS, TLinAlgBlasTranspose::NOTRANS);
		//    EAssert(A.GetCols() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetRows() == B.GetRows());
		////C := alpha*op(A)*op(B) + beta*C,
		////where:
		////op(X) is one of op(X) = X, or op(X) = XT, or op(X) = XH,
		////alpha and beta are scalars,
		////A, B and C are matrices:
		////op(A) is an m-by-k matrix,
		////op(B) is a k-by-n matrix,
		////C is an m-by-n matrix.
		//	int m = A.GetCols(), k = A.GetRows();
		//	int n = B.GetCols(); 
		//	double alpha = 1.0; double beta = 0.0;
		//	cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, m, n, k, alpha, &A(0,0).Val, k, &B(0,0).Val, n, beta, &C(0,0).Val, n);
	}
#else
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, TVVec<Type, Size, ColMajor>& C) {
		EAssert(A.GetCols() == C.GetRows() && B.GetCols() == C.GetCols() && A.GetRows() == B.GetRows());
		Size n = C.GetRows(), m = C.GetCols(), l = A.GetRows(); double sum;
		for (Size i = 0; i < n; i++) {
			for (Size j = 0; j < m; j++) {
				sum = 0.0;
				for (Size k = 0; k < l; k++)
					sum += A(k, i)*B(k, j);
				C(i, j) = sum;
			}
		}
	}
#endif

	//////////////////
	//  DENSE-SPARSE, SPARSE-DENSE

	// TEST
	// C := A * B
#ifndef INTELS
	template <class IndexType = TInt,class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& B, 
		TVVec<Type, Size, ColMajor>& C) {
		// B well defined
		EAssert(B.Val1.Len() == B.Val2.Len() && B.Val2.Len() == B.Val3.Len());
		// Dimensions must match
		C.PutAll(0.0);
		if (B.Val1.Len() == 0) {
			return;
		}
		Size Nonzeros = B.Val1.Len();
		IndexType MaxRowN = B.Val1[B.Val1.GetMxValN()];
		IndexType MaxColN = B.Val2[B.Val2.GetMxValN()];
		EAssert(A.GetRows() == C.GetRows() && (MaxColN + 1) <= C.GetCols() && (MaxRowN + 1) <= A.GetCols());
		for (Size RowN = 0; RowN < A.GetRows(); RowN++) {
			for (Size ElN = 0; ElN < Nonzeros; ElN++) {
				C.At(RowN, B.Val2[ElN]) += A.At(RowN, B.Val1[ElN]) * B.Val3[ElN];
			}
		}
	}
#else
	template <class IndexType = TInt,class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TVVec<Type, Size, ColMajor>& A, const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& B, 
		TVVec<Type, Size, ColMajor>& C){
	// B well defined
	EAssert(B.Val1.Len() == B.Val2.Len() && B.Val2.Len() == B.Val3.Len());
	// Dimensions must match
	C.PutAll(0.0);
	if (B.Val1.Len() == 0) {
		return;
	}
	TLinAlg::MultiplyFS(const_cast<TVVec<Type, Size, ColMajor> &>(A), B, C);
}
#endif

	// TEST
	// C:= A' * B
	template <class IndexType = TInt,class Type, class Size = int, bool ColMajor = false>
static void MultiplyT(const TVVec<Type, Size, ColMajor>& A, const TTriple<TVec<IndexType, Size>, TVec<IndexType, Size>, TVec<Type, Size>>& B, 
	TVVec<Type, Size, ColMajor>& C) {
	// B well defined
	EAssert(B.Val1.Len() == B.Val2.Len() && B.Val2.Len() == B.Val3.Len());
	// Dimensions must match
	C.PutAll(0.0);
	if (B.Val1.Len() == 0) {
		return;
	}
	Size Nonzeros = B.Val1.Len();
	IndexType MaxRowN = B.Val1[B.Val1.GetMxValN()];
	IndexType MaxColN = B.Val2[B.Val2.GetMxValN()];
	EAssert(A.GetCols() == C.GetRows() && (MaxColN + 1) <= C.GetCols() && (MaxRowN + 1) <= A.GetRows());
	for (Size RowN = 0; RowN < A.GetCols(); RowN++) {
		for (Size ElN = 0; ElN < Nonzeros; ElN++) {
			C.At(RowN, B.Val2[ElN]) += A.At(B.Val1[ElN], RowN) * B.Val3[ElN];
		}
	}
}
	// TEST
	// C := A * B

#if !defined(INTEL) || defined(INDEX_64)
	template <class Type, class Size = int, bool ColMajor = false>
	static void Multiply(const TTriple<TVec<TNum<Size>, Size>, TVec<TNum<Size>, Size>, TVec<Type, Size>>& A, const TVVec<Type, Size, ColMajor>& B, 
	TVVec<Type, Size, ColMajor>& C) {
	// A well defined
	EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
	// Dimensions must match
	C.PutAll(0.0);
	if (A.Val1.Len() == 0) {
		return;
	}
	Size Nonzeros = A.Val1.Len();
	Size MaxRowN = A.Val1[A.Val1.GetMxValN()];
	Size MaxColN = A.Val2[A.Val2.GetMxValN()];
	EAssert(B.GetCols() == C.GetCols() && (MaxRowN + 1) <= C.GetRows() && (MaxColN + 1) <= B.GetRows());
	for (Size ColN = 0; ColN < B.GetCols(); ColN++) {
		for (Size ElN = 0; ElN < Nonzeros; ElN++) {
			C.At(A.Val1[ElN], ColN) += A.Val3[ElN] * B.At(A.Val2[ElN], ColN);
		}
	}
}
	// TEST
	// C:= A' * B
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TTriple<TVec<TNum<Size>, Size>, TVec<TNum<Size>, Size>, TVec<Type, Size>>& A, const TVVec<Type, Size, ColMajor>& B, 
	TVVec<Type, Size, ColMajor>& C) {
	// B well defined
	EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
	// Dimensions must match
	C.PutAll(0.0);
	if (A.Val1.Len() == 0) {
		return;
	}
	Size Nonzeros = A.Val1.Len();
	Size MaxRowN = A.Val1[A.Val1.GetMxValN()];
	Size MaxColN = A.Val2[A.Val2.GetMxValN()];
	EAssert(B.GetCols() == C.GetCols() && (MaxColN + 1) <= C.GetRows() && (MaxRowN + 1) <= B.GetRows());
	for (Size ColN = 0; ColN < B.GetCols(); ColN++) {
		for (Size ElN = 0; ElN < Nonzeros; ElN++) {
			C.At(A.Val2[ElN], ColN) += A.Val3[ElN] * B.At(A.Val1[ElN], ColN);
		}
	}
}
#else
// TEST
//If B and C are not of the proper size all will end very badly
	template <class Type, class Size = int, bool ColMajor = false>
static void Multiply(const TTriple<TVec<TNum<Size>, Size>, TVec<TNum<Size>, Size>, TVec<Type, Size>>& A, const TVVec<Type, Size, ColMajor>& B, 
	TVVec<Type, Size, ColMajor>& C) {
	// A well defined
	EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
	// Dimensions must match
	if (A.Val1.Len() == 0) {
		return;
	}
	TLinAlg::MultiplySF(A, B, C);
}

// TEST
	template <class Type, class Size = int, bool ColMajor = false>
	static void MultiplyT(const TTriple<TVec<TNum<Size>, Size>, TVec<TNum<Size>, Size>, TVec<Type, Size>>& A, const TVVec<Type, Size, ColMajor>& B,
	TVVec<Type, Size, ColMajor>& C) {
	// B well defined
	EAssert(A.Val1.Len() == A.Val2.Len() && A.Val2.Len() == A.Val3.Len());
	if (A.Val1.Len() == 0) {
		return;
	}
	TLinAlg::MultiplySF(A, B, C, "T");
}
#endif
	// DENSE-SPARSECOLMAT, SPARSECOLMAT-DENSE
	// C := A * B

	// DENSE-SPARSECOLMAT, SPARSECOLMAT-DENSE
	// C := A * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
static void Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
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
#ifndef INTEL
static void MultiplyT(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
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
}
#else
template <class IndexType = TInt, class Type, class Size = int, bool ColMajor = false>
static void MultiplyT(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
	// C = A' B = (B' A)'
	TTriple<TVec<IndexType, Size>, TVec<TInt, Size>, TVec<Type, Size>> BB;
	TLinAlg::Convert(B, BB); // convert the matrix to a coordinate form
	TVVec<Type, Size, ColMajor> CC(B.Len(), A.GetCols());
	TLinAlg::MultiplyT(BB, A, CC);
	if (C.Empty()) {
		C.Gen(A.GetCols(), B.Len());
	}
	else {
		EAssert(C.GetRows() == A.GetCols() && C.GetCols() == B.Len());
	}
	TLinAlg::Transpose(CC, C);
}
#endif


	// C := A * B
//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
static void Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA = -1) {
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
static void MultiplyT(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C) {
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
static void Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA = -1) {
	//// A,B = sparse column matrix
	//EAssert(A.Len() == B.GetRows());
	int Rows = RowsA;
	int ColsB = B.Len();
	if (RowsA == -1) {
		Rows = TLAMisc::GetMaxDimIdx(A) + 1;
	} else {
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
static void MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C) {
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
//	static void Multiply(const TFltVV & ProjMat, const TPair<TIntV, TFltV> & Doc, TFltV & Result);
//#endif

	// TEST
	// D = alpha * A(') * B(') + beta * C(')
	typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;
	template <class Type, class Size = int, bool ColMajor = false>
	static void Gemm(const double& Alpha, const TVVec<Type, Size, ColMajor>& A, const TVVec<Type, Size, ColMajor>& B, const double& Beta, 
		const TVVec<Type, Size, ColMajor>& C, TVVec<Type, Size, ColMajor>& D, const int& TransposeFlags) {

		bool tA = (TransposeFlags & GEMM_A_T) == GEMM_A_T;
		bool tB = (TransposeFlags & GEMM_B_T) == GEMM_B_T;
		bool tC = (TransposeFlags & GEMM_C_T) == GEMM_C_T;


		// setting dimensions
		Size a_i = tA ? A.GetRows() : A.GetCols();
		Size a_j = tA ? A.GetCols() : A.GetRows();

		Size b_i = tB ? B.GetRows() : B.GetCols();
		Size b_j = tB ? B.GetCols() : B.GetRows();

		Size c_i = tC ? C.GetRows() : C.GetCols();
		Size c_j = tC ? C.GetCols() : C.GetRows();

		Size d_i = D.GetCols();
		Size d_j = D.GetRows();

		// assertions for dimensions
		EAssert(a_j == c_j && b_i == c_i && a_i == b_j && c_i == d_i && c_j == d_j);

		double Aij, Bij, Cij;

		// rows of D
		for (Size j = 0; j < a_j; j++) {
			// cols of D
			for (Size i = 0; i < b_i; i++) {
				// not optimized for speed - naive algorithm
				double sum = 0.0;
				// cols of A
				for (Size k = 0; k < a_i; k++) {
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
	template <class Type, class Size = int, bool ColMajor = false>
	static void Inverse(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor >& B, const TLinAlgInverseType& DecompType) {
		switch (DecompType) {
		case DECOMP_SVD:
			TLinAlg::InverseSVD(A, B);
		}
	}

	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd)
	template <class Type, class Size = int, bool ColMajor = false>
	static void InverseSVD(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& B, const double& tol) {
		// create temp matrices
		TVVec<Type, Size, ColMajor> U, V;
		TVec<Type, Size> E;
		TSvd SVD;

		//U.Gen(M.GetRows(), M.GetRows());
		//V.Gen(M.GetCols(), M.GetCols());
		U.Gen(A.GetRows(), A.GetRows());
		V.Gen(A.GetCols(), A.GetCols());

		// do the SVD decompostion
		SVD.Svd(A, U, E, V);

		// calculate reciprocal values for diagonal matrix = inverse diagonal
		for (Size i = 0; i < E.Len(); i++) {
			if (E[i] > tol) {
				E[i] = 1 / E[i];
			}
			else {
				E[i] = 0.0;
			}
		}

		// calculate pseudoinverse: M^(-1) = V * E^(-1) * U'
		for (Size i = 0; i < U.GetCols(); i++) {
			for (Size j = 0; j < V.GetRows(); j++) {
				double sum = 0.0;
				for (Size k = 0; k < U.GetCols(); k++) {
					if (E[k] == 0.0) continue;
					sum += E[k] * V.At(i, k) * U.At(j, k);
				}
				B.At(i, j) = sum;
			}
		}
	}

	// subtypes of finding an inverse (works only for TFltVV, cuz of TSvd)
	template <class Type, class Size = int, bool ColMajor = false>
	static void InverseSVD(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& B) {
		// create temp matrices
		TVVec<Type, Size, ColMajor> U, V;
		TVec<Type, Size> E;
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
		for (Size i = 0; i < E.Len(); i++) {
			if (E[i] > tol) {
				E[i] = 1 / E[i];
			}
			else {
				E[i] = 0.0;
			}
		}

		// calculate pseudoinverse: M^(-1) = V * E^(-1) * U'
		for (Size i = 0; i < U.GetCols(); i++) {
			for (Size j = 0; j < V.GetRows(); j++) {
				double sum = 0;
				for (Size k = 0; k < U.GetCols(); k++) {
					if (E[k] == 0.0) continue;
					sum += E[k] * V.At(i, k) * U.At(j, k);
				}
				B.At(i, j) = sum;
			}
		}
	}

	// transpose matrix - B = A'
	template <class Type, class Size = int, bool ColMajor = false>
	static void Transpose(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& B) {
		EAssert(B.GetRows() == A.GetCols() && B.GetCols() == A.GetRows());
		for (Size i = 0; i < A.GetCols(); i++) {
			for (Size j = 0; j < A.GetRows(); j++) {
				B.At(i, j) = A.At(j, i);
			}
		}
	}

    // performes Gram-Schmidt ortogonalization on elements of Q
	template <class Size = int>
	static void GS(TVec<TVec<TFlt, Size>, Size>& Q) {
		EAssert(Q.Len() > 0);
		Size m = Q.Len(); // int n = Q[0].Len();
		for (Size i = 0; i < m; i++) {
			printf("%d\r", i);
			for (Size j = 0; j < i; j++) {
				double r = TLinAlg::DotProduct(Q[i], Q[j]);
				TLinAlg::AddVec(-r, Q[j], Q[i], Q[i]);
			}
			TLinAlg::Normalize(Q[i]);
		}
		printf("\n");
	}

	// TEST
    // Gram-Schmidt on columns of matrix Q
	template <class Type, class Size = int, bool ColMajor = false>
	static void GS(TVVec<Type, Size, ColMajor>& Q) {
		Size m = Q.GetCols(), n = Q.GetRows();
		for (Size i = 0; i < m; i++) {
			printf("%d\r", i);
			for (Size j = 0; j < i; j++) {
				double r = TLinAlg::DotProduct(Q, i, Q, j);
				TLinAlg::AddVec(-r, Q, j, Q, i);
			}
			double nr = TLinAlg::Norm(Q, i);
			for (Size k = 0; k < n; k++)
				Q(k, i) = Q(k, i) / nr;
		}
		printf("\n");
	}
	// Modified Gram-Schmidt on columns of matrix Q
	static void MGS(TFltVV& Q) {
		int Cols = Q.GetCols(), Rows = Q.GetRows();
		EAssertR(Rows >= Cols, "TLinAlg::MGS: number of rows should be greater or equal to the number of cols");
		for (int ColN = 0; ColN < Cols; ColN++) {
			TLinAlg::NormalizeColumns(Q);
			for (int ColN2 = ColN+1; ColN2 < Cols; ColN2++) {
				double r = TLinAlg::DotProduct(Q, ColN, Q, ColN2);
				TLinAlg::AddVec(-r, Q, ColN, Q, ColN2);
			}
		}
	}
	// QR based on Modified Gram-Schmidt decomposition.
	static void QR(const TFltVV& X, TFltVV& Q, TFltVV& R, const TFlt& Tol) {
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
 	static void Rotate(const double& OldX, const double& OldY, const double& Angle, double& NewX, double& NewY) {
		NewX = OldX*cos(Angle) - OldY*sin(Angle);
		NewY = OldX*sin(Angle) + OldY*cos(Angle);
	}

   // checks if set of vectors is ortogonal
	template <class Size = int>
	static void AssertOrtogonality(const TVec<TVec<TFlt, Size>, Size>& Vecs, const double& Threshold) {
		Size m = Vecs.Len();
		for (Size i = 0; i < m; i++) {
			for (Size j = 0; j < i; j++) {
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
	template <class Type, class Size = int, bool ColMajor = false>
	static void AssertOrtogonality(const TVVec<Type, Size, ColMajor>& Vecs, const double& Threshold) {
		Size m = Vecs.GetCols();
		for (Size i = 0; i < m; i++) {
			for (Size j = 0; j < i; j++) {
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
	static bool IsOrthonormal(const TFltVV& Vecs, const double& Threshold) {
		int m = Vecs.GetCols();
		TFltVV R(m, m);
		TLinAlg::MultiplyT(Vecs, Vecs, R);
		for (int i = 0; i < m; i++) { R(i, i) -= 1; }
		return TLinAlg::Frob(R) < Threshold;
	}
};

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
    TNSException(const TStr& Msg): TExcept(Msg) {}
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
    static void GetEigenVec(const TFltVV& A, const double& EigenVal, TFltV& EigenV, const double& ConvergEps=1e-7);

#ifdef BLAS
    // LU midstep used for LUFactorization and LUSolve
    // (Warning: the matrix is overwritten in the process)
    static void LUStep(TFltVV& A, TIntV& PermV);
    // LUFactorization create the matrices L, U and vector of permutations P such that P*A = L*U.
    // The L is unit lower triangular matrix and U is an upper triangular matrix.
    // Vector P tell's us: column i is swapped with column P[i].
    static void LUFactorization(const TFltVV& A, TFltVV& L, TFltVV& U, TIntV& P);
    // Solves the system of linear equations A * x = b, where A is a matrix, x and b are vectors.
    // Solution is saved in x.
    static void LUSolve(const TFltVV& A, TFltV& x, const TFltV& b);
    // Solves the system of linear equations A * X = B, where A, X and B are matrices.
    // Solution is saved in X.
    static void LUSolve(const TFltVV& A, TFltVV& X, const TFltVV& B);

    // solves the system A * x = b, where A is a triangular matrix, x and b are vectors.
    // The solution is saved in x.
    // UpperTriangFlag: if the matrix is upper triangular (true) or lower triangular (false).
    // DiagUnitFlag: if the matrix has ones on the diagonal (true) or not (false).
    static void TriangularSolve(TFltVV& A, TFltV& x, TFltV& b,
    		bool UpperTriangFlag = true, bool DiagonalUnitFlag = false);

	///////////////////////////////////////////////////////////////////////////
	// SVD factorization and solution

	// Makes the SVD factorization of matrix Matrix, such that A = U * Sing * VT.
	// Sing is the vector containing singular values, U is the matrix with left singular vectors,
	// VT is the matrix with right singular vectors.
	static void SVDFactorization(const TFltVV& A, TFltVV& U, TFltV& Sing, TFltVV& VT);

	// SVDSolve solves the Least Squares problem of equation A * x = b, where A is a matrix, x and b are vectors.
	// The solution is saved in x.
	static void SVDSolve(const TFltVV& A, TFltV& x, const TFltV& b, const double& EpsSing=0);
#endif
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
    TSigmoid(const double& A_, const double& B_): A(A_), B(B_) { };
	// Tries to find a pair (A, B) that minimizes J(A, B).
    // Uses gradient descent.
    TSigmoid(const TFltIntKdV& data);

    TSigmoid(TSIn& SIn) { A.Load(SIn); B.Load(SIn); }
    void Load(TSIn& SIn) { A.Load(SIn); B.Load(SIn); }
    void Save(TSOut& SOut) const {A.Save(SOut); B.Save(SOut);}

    double GetVal(const double& x) const {
        return 1.0 / (1.0 + exp(-A * x + B)); }
    double operator()(const double& x) const {
        return GetVal(x); }

    void GetSigmoidAB(double& A_, double& B_) { A_=A; B_=B; }
};

class TFullMatrix;

/////////////////////////////////////////////////////////////////////////
//// Full-Vector
class TVector { friend class TFullMatrix;
public:
    bool IsColVector;
    TFltV Vec;

public:
    TVector(const bool& IsColVector=true);
    TVector(const int& Dim, const bool IsColVector=true);
    TVector(const TFltV& Vect, const bool IsColVector=true);
    TVector(const TIntV& Vect, const bool IsColVector=true);
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
    static TVector Zeros(const int& Dim, const bool IsColVec=true);
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
//	const int& Dim = Len();
//
//	TVector Res(IsColVector);
//
//	for (int i = 0; i < Dim; i++) {
//		if (Func(Vec[i])) {
//			Res.Vec.Add(i);
//		}
//	}
//
//	return Res;
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

class TFullMatrix: public TMatrix { friend class TVector;
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
    TFullMatrix(TFltVV& Mat, const bool IsWrapper=false);
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
    TFlt& At(const int& i, const int& j) { return Mat->operator ()(i,j); }
    const TFlt& At(const int& i, const int& j) const { return Mat->operator ()(i,j); }
    // sets the value at position (i,j)
    void Set(const double& Val, const int& i, const int& j) { Mat->operator ()(i,j) = Val; }
    // returns true if the matrix is empty
    bool Empty() const { return Mat->Empty(); }
    
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
			Mat->At(i,j) = Func(Mat->At(i,j));
		}
	}

	return *this;
}

#ifdef LAPACKE
	#include "MKLfunctions.h"
#endif

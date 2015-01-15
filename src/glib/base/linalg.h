/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

///////////////////////////////////////////////////////////////////////
// Blas Support
#ifdef BLAS
	#ifdef AMD
		#include "acml.h"
	#endif
	#ifdef INTEL 
		#include "mkl.h"
	#endif
	#ifdef OPENBLAS		
		#include "cblas.h"
		#include "lapacke.h"		
	#endif
#endif

///////////////////////////////////////////////////////////////////////
// forward declarations
class TLinAlg;
class TLAMisc;

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
    // loads Matlab sparse matrix format: row, column, value.
    //   Indexes start with 1.
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
    static double DotProduct(const TFltV& x, const TFltV& y);
    // <X[i],y>
    static double DotProduct(const TVec<TFltV>& X, int ColId, const TFltV& y);
    // <X(:,ColIdX), Y(:,ColIdY)>
    static double DotProduct(const TFltVV& X, int ColIdX, const TFltVV& Y, int ColIdY);
    // <X(:,ColId), Vec>
    static double DotProduct(const TFltVV& X, int ColId, const TFltV& Vec);
    // sparse dot products:
    // <x,y> where x AND y are sparse
    static double DotProduct(const TIntFltKdV& x, const TIntFltKdV& y);
    // <X[i],y> where x AND y are sparse
    static double DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TIntFltKdV& y);
    // <x,y> where only y is sparse
    static double DotProduct(const TFltV& x, const TIntFltKdV& y);
    // <X[i],y> where only y is sparse
    static double DotProduct(const TVec<TFltV>& X, int ColId, const TIntFltKdV& y);
    // <X[i],y> where only X is sparse
    static double DotProduct(const TVec<TIntFltKdV>& X, int ColId, const TFltV& y);
    // <X(:,ColId),y> where only y is sparse
    static double DotProduct(const TFltVV& X, int ColId, const TIntFltKdV& y);
	// z = x * y'    
    static void OuterProduct(const TFltV& x, const TFltV& y, TFltVV& Z);

    // z := p * x + q * y
    static void LinComb(const double& p, const TFltV& x,
        const double& q, const TFltV& y, TFltV& z);
	// Z := p * X + q * Y
    static void LinComb(const double& p, const TFltVV& X,
        const double& q, const TFltVV& Y, TFltVV& Z);
	// z := p * X(:,ColId) + q * y
    //static void LinComb(const double& p, const TFltVV& X, int ColId,
    //    const double& q, const TFltV& y, TFltV& z);
	// if (Dim == 1) {z := p * X(:,ColId) + q * y} else if (Dim == 2) {z := p * X(:,RowId) + q * y}
    static void LinComb(const double& p, const TFltVV& X, int DimId,
        const double& q, const TFltV& y, TFltV& z, int Dim = 1);
	// z = p * x + q * y
    static void LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z);	  
    // z := p * x + (1 - p) * y
    static void ConvexComb(const double& p, const TFltV& x, const TFltV& y, TFltV& z);

    // z := k * x + y
    static void AddVec(const double& k, const TFltV& x, const TFltV& y, TFltV& z);
    // z := k * X[ColId] + y
    static void AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z);
    // z := k * X(:,ColId) + y
    static void AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z);
    // z := x + y
	static void AddVec(const TFltV& x, const TFltV& y, TFltV& z);
    // z := k * x + y
    static void AddVec(const double& k, const TIntFltKdV& x, const TFltV& y, TFltV& z);
    // z := k * X[ColId] + y
    static void AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z);
    // y := k * x + y
    static void AddVec(const double& k, const TIntFltKdV& x, TFltV& y);
    // Y(:,Col) += k * X(:,Col)
    static void AddVec(const double& k, const TFltVV& X, int ColIdX, TFltVV& Y, int ColIdY);
	// Y(:,ColIdY) += k * x
	static void AddVec(const double& k, const TFltV& x, TFltVV& Y, const int& ColIdY);
    // Result += k * X(:,Col)
    static void AddVec(const double& k, const TFltVV& X, int ColId, TFltV& Result);
	// z = x + y
    static void AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z);	    

    // Result = SUM(x)
    static double SumVec(const TFltV& x);
	// Result = SUM(x)
    static double SumVec(const TIntFltKdV& x);
    // Result = SUM(k*x + y)
    static double SumVec(double k, const TFltV& x, const TFltV& y);

    // Result = ||x-y||^2 (Euclidian)
    static double EuclDist2(const TFltV& x, const TFltV& y);
    // Result = ||x-y||^2 (Euclidian)
    static double EuclDist2(const TFltPr& x, const TFltPr& y);
    // Result = ||x-y|| (Euclidian)
    static double EuclDist(const TFltV& x, const TFltV& y);
    // Result = ||x-y|| (Euclidian)
    static double EuclDist(const TFltPr& x, const TFltPr& y);
	// Result = ||A||_F (Frobenious)
	static double Frob(const TFltVV&A);
	// Result = ||A - B||_F (Frobenious)
	static double FrobDist2(const TFltVV& A, const TFltVV& B);
	// Result = ||A - B||_F (Frobenious)
	static double FrobDist2(const TFltV& A, const TFltV& B);
	// Dense to sparse transform
	static void Sparse(const TFltVV& A, TTriple<TIntV, TIntV, TFltV>& B);
	// Dense to sparse transform
	static void Sparse(const TFltVV& A, TVec<TIntFltKdV>& B);
	// Sparse to dense transform
	static void Full(const TTriple<TIntV, TIntV, TFltV>& A, TFltVV& B, const int Rows, const int Cols);
	// Sparse to dense transform
	static void Full(const TVec<TIntFltKdV>& A, TFltVV& B, const int Rows);
	// Transpose
	static void Transpose(const TTriple<TIntV, TIntV, TFltV>& A, TTriple<TIntV, TIntV, TFltV>& At);
	// Transpose
	static void Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows = -1);
	// Sign
	static void Sign(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& B);
	// Vector of sparse vectors to sparse matrix (coordinate representation)
	static void Convert(const TVec<TPair<TIntV, TFltV> >& A, TTriple<TIntV, TIntV, TFltV>& B);
	// Vector of sparse vectors to sparse matrix (coordinate representation)
	static void Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>& B);
	// sum columns (Dimension = 2) or rows (Dimension = 1) and store them in vector y
	static void Sum(const TFltVV& X, TFltV& y, const int Dimension = 1);
	// sum columns (Dimension = 2) or rows (Dimension = 1) and store them in vector y
	static void Sum(const TTriple<TIntV, TIntV, TFltV>& X, TFltV& y, const int Dimension = 1);

    // ||x||^2 (Euclidian)
    static double Norm2(const TFltV& x);
    // ||x|| (Euclidian)
    static double Norm(const TFltV& x);
    // x := x / ||x||
    static void Normalize(TFltV& x);
	// Normalize X(:,ColId)
	static void NormalizeColumn(TFltVV& X, const int& ColId);
	// Normalize the columns of X
	static void NormalizeColumns(TFltVV& X);
	// Normalize the columns of X
	static void NormalizeColumns(TTriple<TIntV, TIntV, TFltV>& X);
	// Normalize the columns of X
	static void NormalizeColumns(TVec<TIntFltKdV>& X);
	// Frobenius norm of matrix A
	static double FrobNorm(const TFltVV& A);

    // ||x||^2 (Euclidian), x is sparse
    static double Norm2(const TIntFltKdV& x);
    // ||x|| (Euclidian), x is sparse
    static double Norm(const TIntFltKdV& x);
    // x := x / ||x||, x is sparse
    static void Normalize(TIntFltKdV& x);

    // ||X(:,ColId)||^2 (Euclidian)
    static double Norm2(const TFltVV& X, int ColId);
    // ||X(:,ColId)|| (Euclidian)
    static double Norm(const TFltVV& X, int ColId);

    // L1 norm of x (Sum[|xi|, i = 1..n])
    static double NormL1(const TFltV& x);
    // L1 norm of k*x+y (Sum[|k*xi+yi|, i = 1..n])
    static double NormL1(double k, const TFltV& x, const TFltV& y);
    // L1 norm of x (Sum[|xi|, i = 1..n])
    static double NormL1(const TIntFltKdV& x);
    // x := x / ||x||_inf
    static void NormalizeL1(TFltV& x);
    // x := x / ||x||_inf
    static void NormalizeL1(TIntFltKdV& x);

    // Linf norm of x (Max{|xi|, i = 1..n})
    static double NormLinf(const TFltV& x);
    // Linf norm of x (Max{|xi|, i = 1..n})
    static double NormLinf(const TIntFltKdV& x);
    // x := x / ||x||_inf
    static void NormalizeLinf(TFltV& x);
    // x := x / ||x||_inf, , x is sparse
    static void NormalizeLinf(TIntFltKdV& x);
	// find the index of maximum elements for a given row of X
	static int GetRowMaxIdx(const TFltVV& X, const int& RowN);
	// find the index of maximum elements for a given each col of X
	static int GetColMaxIdx(const TFltVV& X, const int& ColN);
	// find the index of maximum elements for each row of X
	static void GetRowMaxIdxV(const TFltVV& X, TIntV& IdxV);
	// find the index of maximum elements for each col of X
	static void GetColMaxIdxV(const TFltVV& X, TIntV& IdxV);

	// find the index of maximum elements for a given each col of X
	static int GetColMinIdx(const TFltVV& X, const int& ColN);
	// find the index of maximum elements for each col of X
	static void GetColMinIdxV(const TFltVV& X, TIntV& IdxV);

	template <class TVal> static TVal GetColMin(const TVVec<TVal>& X, const int& ColN);
	template <class TVal> static void GetColMinV(const TVVec<TVal>& X, TVec<TVal>& ValV);

    // y := k * x
    static void MultiplyScalar(const double& k, const TFltV& x, TFltV& y);
    // y := k * x
    static void MultiplyScalar(const double& k, const TIntFltKdV& x, TIntFltKdV& y);
    // Y := k * X
    static void MultiplyScalar(const double& k, const TFltVV& X, TFltVV& Y);
	// Y := k * X
    static void MultiplyScalar(const double& k, const TVec<TIntFltKdV>& X, TVec<TIntFltKdV>& Y);
    
    // y := A * x
    static void Multiply(const TFltVV& A, const TFltV& x, TFltV& y);
    // C(:, ColId) := A * x
    static void Multiply(const TFltVV& A, const TFltV& x, TFltVV& C, int ColId);
    // y := A * B(:, ColId)
    static void Multiply(const TFltVV& A, const TFltVV& B, int ColId, TFltV& y);
    // C(:, ColIdC) := A * B(:, ColIdB)
    static void Multiply(const TFltVV& A, const TFltVV& B, int ColIdB, TFltVV& C, int ColIdC);

//LAPACKE stuff
#ifdef LAPACKE
	static void QRbasis(TFltVV& A);
	static void QRbasis(const TFltVV& A, TFltVV& Q);
	static void QRcolpbasis(TFltVV& A);
	static void QRcolpbasis(const TFltVV& A, TFltVV& Q);
	static void thinSVD(const TFltVV& A, TFltVV& U, TFltV& S, TFltVV& VT);
#endif
	static int ComputeThinSVD(const TMatrix& X, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters = 2, const double Tol = 1e-6);	

//Full matrix times sparse vector
#ifdef INTEL	
	//No need to reserve anything outside, functions currently take care of memory managment for safety
	static void Multiply(TFltVV& ProjMat, TPair<TIntV, TFltV> &, TFltVV& result);
	static void Multiply(const TFltVV& ProjMat, const TPair<TIntV, TFltV> &, TFltVV& result);
	static void MultiplySF(const TTriple<TIntV, TIntV, TFltV>&, const TFltVV& B, TFltVV& C, const TStr& transa = TStr("N"));
#endif
	// y := A * x
	static void Multiply(const TFltVV& A, const TPair<TIntV, TFltV>& x, TFltV& y);
	// y := A' * x
	static void MultiplyT(const TFltVV& A, const TFltV& x, TFltV& y);

#ifdef BLAS
	typedef enum { NOTRANS = 0, TRANS = 1} TLinAlgBlasTranspose;
	// C = op(A) * op(B)
    static void Multiply(const TFltVV& A, const TFltVV& B, TFltVV& C, const int& BlasTransposeFlagA, const int& BlasTransposeFlagB);
	// y := alpha*op(A)*x + beta*y, where op(A) = A -- N, op(A) = A' -- T, op(A) = conj(A') -- C (only for complex)
    static void Multiply(const TFltVV& A, const TFltV& x, TFltV& y, const int& BlasTransposeFlagA, double alpha = 1.0, double beta = 0.0);
#endif
    // C = A * B
    static void Multiply(const TFltVV& A, const TFltVV& B, TFltVV& C);	

	// C = A' * B
    static void MultiplyT(const TFltVV& A, const TFltVV& B, TFltVV& C);

	//////////////////
	//  DENSE-SPARSE, SPARSE-DENSE
	// C := A * B
	static void Multiply(const TFltVV& A, const TTriple<TIntV, TIntV, TFltV>& B, TFltVV& C);
	// C:= A' * B
	static void MultiplyT(const TFltVV& A, const TTriple<TIntV, TIntV, TFltV>& B, TFltVV& C);
	// C := A * B
	static void Multiply(const TTriple<TIntV, TIntV, TFltV>& A, const TFltVV& B, TFltVV& C);
	// C:= A' * B
	static void MultiplyT(const TTriple<TIntV, TIntV, TFltV>& A, const TFltVV& B, TFltVV& C);
	// DENSE-SPARSECOLMAT, SPARSECOLMAT-DENSE
	// C := A * B
	static void Multiply(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C);
	// C:= A' * B
	static void MultiplyT(const TFltVV& A, const TVec<TIntFltKdV>& B, TFltVV& C);
	// C := A * B
	static void Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA = -1);
	// C:= A' * B
	static void MultiplyT(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C);
	// SPARSECOLMAT-SPARSECOLMAT
	// C := A * B
	static void Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA = -1);
	// C:= A' * B
	static void MultiplyT(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C);

//#ifdef INTEL
//	static void Multiply(const TFltVV & ProjMat, const TPair<TIntV, TFltV> & Doc, TFltV & Result);
//#endif

	// D = alpha * A(') * B(') + beta * C(')
	typedef enum { GEMM_NO_T = 0, GEMM_A_T = 1, GEMM_B_T = 2, GEMM_C_T = 4 } TLinAlgGemmTranspose;
	static void Gemm(const double& Alpha, const TFltVV& A, const TFltVV& B, const double& Beta, 
		const TFltVV& C, TFltVV& D, const int& TransposeFlags);
	
	// B = A^(-1)
	typedef enum { DECOMP_SVD } TLinAlgInverseType;
	static void Inverse(const TFltVV& A, TFltVV& B, const TLinAlgInverseType& DecompType);
	// subtypes of finding an inverse
	static void InverseSVD(const TFltVV& A, TFltVV& B, const double& tol);
	static void InverseSVD(const TFltVV& A, TFltVV& B);

	// transpose matrix - B = A'
	static void Transpose(const TFltVV& A, TFltVV& B);

    // performes Gram-Schmidt ortogonalization on elements of Q
    static void GS(TVec<TFltV>& Q);
    // Gram-Schmidt on columns of matrix Q
    static void GS(TFltVV& Q);
	// Modified Gram-Schmidt on columns of matrix Q
	static void MGS(TFltVV& Q);
	// QR based on Modified Gram-Schmidt decomposition.
	static void QR(const TFltVV& A, TFltVV& Q, TFltVV& R, const TFlt& Tol);

    // rotates vector (OldX,OldY) for angle Angle (in radians!)
    static void Rotate(const double& OldX, const double& OldY, const double& Angle, double& NewX, double& NewY);

    // checks if set of vectors is ortogonal
    static void AssertOrtogonality(const TVec<TFltV>& Vecs, const double& Threshold);
    static void AssertOrtogonality(const TFltVV& Vecs, const double& Threshold);
	static bool IsOrthonormal(const TFltVV& Vecs, const double& Threshold);
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

    // Solves system of linear equations A * x = b. A is first decomposed using
    // LUDecomposition and after solved using LUSolve. A is modified!
    static void SolveLinearSystem(TFltVV& A, const TFltV& b, TFltV& x);

    // Computes the eigenvector of A belonging to the specified eigenvalue
    // uses the inverse iteration algorithm
    // the algorithms does modify A due to its use of LU decomposition
    static void GetEigenVec(TFltVV& A, const double& EigenVal, TFltV& EigenV, const double& ConvergEps=1e-7);
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

//////////////////////////////////////////////////////////////////////
// Useful stuff (hopefully)
class TLAMisc {
public:
	// Dumps vector to file so Excel can read it
    static void SaveCsvTFltV(const TFltV& Vec, TSOut& SOut);
	// Dumps sparse vector to file so Matlab can read it
    static void SaveMatlabTFltIntKdV(const TIntFltKdV& SpV, const int& ColN, TSOut& SOut);
	// Dumps sparse matrix to file so Matlab can read it
    static void SaveMatlabSpMat(const TTriple<TIntV, TIntV,TFltV>& SpMat, TSOut& SOut);
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
    // sets all compnents to zero
    static void FillZero(TFltV& Vec) { Vec.PutAll(0.0); }
    static void FillZero(TFltVV& M) { Fill(M, 0.0); }
    // set matrix to identity
    static void FillIdentity(TFltVV& M);
    static void FillIdentity(TFltVV& M, const double& Elt);
    // set vector to range
    static void FillRange(const int& Vals, TFltV& Vec);
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
		Assert(RowIdxV.Len() == ColIdxV.Len() && RowIdxV.Len() == ValV.Len());
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
			} else if (SrcV1[Src1N].Key > SrcV2[Src2N].Key) { 
				DstV.Add(SrcV2[Src2N]); Src2N++;
			} else { 
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
			} else if (SrcV1[Src1N].Key > SrcV2[Src2N].Key) { 
				DstV.Add(TKeyDat<TKey, TDat>(SrcV2[Src2N].Key, q * SrcV2[Src2N].Dat)); Src2N++;
			} else { 
				DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, p * SrcV1[Src1N].Dat + q * SrcV2[Src2N].Dat));
				Src1N++;  Src2N++; 
			}
		}
		while (Src1N < Src1Len) { DstV.Add(TKeyDat<TKey, TDat>(SrcV1[Src1N].Key, p * SrcV1[Src1N].Dat)); Src1N++; }
		while (Src2N < Src2Len) { DstV.Add(TKeyDat<TKey, TDat>(SrcV2[Src2N].Key, q * SrcV2[Src2N].Dat)); Src2N++; }
	}
};

typedef TSparseOps<TInt, TFlt> TSparseOpsIntFlt;

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
	const int& Dim = Len();

	TVector Res(IsColVector);

	for (int i = 0; i < Dim; i++) {
		if (Func(Vec[i])) {
			Res.Vec.Add(i);
		}
	}

	return Res;
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
			const int Idx1 = RowV[i];
			const int Idx2 = ColV[j];
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

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef MKLFUNCTIONS_H
#define MKLFUNCTIONS_H

#ifdef INDEX_64
typedef int64 glib_index;
#define lapack_int long long
#else
typedef int glib_index;
#endif



#ifdef INTEL
	//Intel MKL fix
    #define MKL_Complex8 std::complex<float>
	#define MKL_Complex16 std::complex<double>
	#include "mkl.h"
	//#include "mkl_scalapack.h"
#else
	//OpenBlas fix
	#define lapack_complex_float std::complex<float>
	#define lapack_complex_double std::complex<double>
	#define LAPACK_COMPLEX_CPP
	#include "cblas.h"
	#include "lapacke.h"
	typedef CBLAS_ORDER CBLAS_LAYOUT;
#endif

///////////////////////////////////////////////////////////////////////////////
// template MKLfunctions class holds the mkl functions used for numeric
// calculations. It includes functions for solving with LU factorization,
// Cholesky factorization, Triangular matrices, QR factorization, LQ 
// factorization, SVD factorization and srearching the eigenvalues and 
// eigenvectors for a symmetric and unsymmetric matrices.

class MKLfunctions {

private:
	// LU midstep used for LUFactorization and LUSolve 
	// (Warning: the matrix is overwritten in the process)
	template<class Type, class Size, bool ColMajor = false>
	static void LUStep(TVVec<TNum<Type>, Size, ColMajor>& A, TVec<TNum<glib_index>, glib_index>& Perm) {//TVec<TNum<Size>, Size>& Perm)
		Assert(A.GetRows() == A.GetCols());

		// data used for factorization
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		Perm.Gen(NumOfRows_Matrix);

		// factorization
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgetrf(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgetrf(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zgetrf(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cgetrf(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val);
		}
	}

	// Cholesky midstep used for CholeskyFactorization and CholeskySolve.
	// (Warning: the matrix is overwritten in the process)
	template<class Type, class Size, bool ColMajor = false>
static void CholeskyStep(TVVec<Type, Size, ColMajor>& A) {

		// data used in function
		char uplo = 'L';
		Size OrderOfMatrix = A.GetRows();
		Size LeadingDimension_Matrix = ColMajor ? A.GetRows() : A.GetCols();
		Size Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// factorization

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dpotrf(Matrix_Layout, uplo, OrderOfMatrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_spotrf(Matrix_Layout, uplo, OrderOfMatrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zpotrf(Matrix_Layout, uplo, OrderOfMatrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cpotrf(Matrix_Layout, uplo, OrderOfMatrix, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix);
		}
	}

	// QR midstep used for LUFactorization and QRSolve. 
	// (Warning: the matrix is overwritten in the process)
	template<class Type, class Size, bool ColMajor = false>
	static void QRStep(TVVec<Type, Size, ColMajor>& A, TVec<TNum<glib_index>, glib_index>& Tau) {
		Assert(A.GetRows() >= A.GetCols());

		// data used for factorization
		Size NumOfRows = A.GetRows();
		Size NumOfCols = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows : NumOfCols;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// factorization
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgeqrf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *) &Tau[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgeqrf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zgeqrf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cgeqrf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
	}

	// LQ midstep used for LQFactorization and LQSolve.
	// (Warning: the matrix is overwritten in the process)
	template<class Type, class Size, bool ColMajor = false>
	static void LQStep(TVVec<Type, Size, ColMajor>& A, TVec<TNum<glib_index>, glib_index>& Tau) {
		Assert(A.GetRows() <= A.GetCols());

		Size NumOfRows = A.GetRows();
		Size NumOfCols = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows : NumOfCols;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgelqf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgelqf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zgelqf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cgelqf(Matrix_Layout, NumOfRows, NumOfCols, (Loc *)&A(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Tau[0].Val);
		}
	}

public:

	///////////////////////////////////////////////////////////////////////////
	// LU factorization and solution

	// LUFactorization create the matrices L, U and vector of permutations P such that P*A = L*U.
	// The L is unit lower triangular matrix and U is an upper triangular matrix. 
	// Vector P tell's us: column i is swapped with column P[i].
	template<class Type, class Size, bool ColMajor = false>
	static void LUFactorization(const TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& L,
		TVVec<Type, Size, ColMajor>& U, TVec<TNum<glib_index>, glib_index>& P) {
		Assert(A.GetRows() == A.GetCols());

		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();

		// copy of the matrix
		TVVec<Type, Size, ColMajor> M = A;

		// LUStep
		MKLfunctions::LUStep(M, P);
		
		// construction of L matrix
		// TODO zero elements not set
		L.Gen(NumOfRows_Matrix, NumOfCols_Matrix);
		for (Size i = 0; i < NumOfRows_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_Matrix; j++) {

				if (j < i) { L(i, j) = M(i, j); }

				else if (j == i) { L(i, j) = 1; }

				else { L(i, j) = 0; }
			}
		}

		// construction of U matrix
		U.Gen(NumOfRows_Matrix, NumOfCols_Matrix);
		for (Size i = 0; i < NumOfRows_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_Matrix; j++) {

				if (i <= j) { U(i, j) = M(i, j); }

				else { U(i, j) = 0; }
			}
		}
	}

	// Solves the system of linear equations A * x = b, where A is a matrix, x and b are vectors.
	// Solution is saved in x.
	template<class Type, class Size, bool ColMajor = false>
	static void LUSolve(const TVVec<TNum<Type>, Size, ColMajor>& A, TVec<TNum<Type>, Size>& x, const TVec<TNum<Type>, Size>& b) {
		Assert(A.GetRows() == b.Len());

		// for matrix
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// for vector
		Size LeadingDimension_Vector = ColMajor ? b.Len() : 1;
		Size NumOfCols_Vector = 1;

		// LU factorization
		TVVec<TNum<Type>, Size, ColMajor> M = A;
		//TODO TInt is enough?
		TVec<TNum<glib_index>, glib_index> Perm; Perm.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix));
		MKLfunctions::LUStep(M, Perm);

		// solution
		x = b;

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgetrs(Matrix_Layout, 'N', NumOfCols_Matrix, NumOfCols_Vector, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgetrs(Matrix_Layout, 'N', NumOfCols_Matrix, NumOfCols_Vector, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef  std::complex<double> Loc;
			LAPACKE_zgetrs(Matrix_Layout, 'N', NumOfCols_Matrix, NumOfCols_Vector, (const Loc *)(&M(0, 0).Val), LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)(&x[0].Val), LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cgetrs(Matrix_Layout, 'N', NumOfCols_Matrix, NumOfCols_Vector, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)(&x[0].Val), LeadingDimension_Vector);
		}
	}

	// Solves the system of linear equations A * X = B, where A, X and B are matrices.
	// Solution is saved in X.
	template<class Type, class Size, bool ColMajor = false>
	static void LUSolve(TVVec<TNum<Type>, Size, ColMajor>& A, TVVec<TNum<Type>, Size, ColMajor>& X, TVVec<TNum<Type>, Size, ColMajor>& B) {
		Assert(A.GetRows() == B.GetRows());

		// for matrix
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// for vector
		Size LeadingDimension_B = ColMajor ? B.GetRows() : B.GetCols();
		Size NumOfCols_B = B.GetCols();

		// LU factorization
		TVVec<TNum<Type>, Size, ColMajor> M = A;
		//TODO TInt is enough?
		TVec<TNum<glib_index>, glib_index> Perm; Perm.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix));
		MKLfunctions::LUStep(M, Perm);

		// solution
		X = B;

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgetrs(Matrix_Layout, 'N', NumOfRows_Matrix, NumOfCols_B, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgetrs(Matrix_Layout, 'N', NumOfRows_Matrix, NumOfCols_B, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zgetrs(Matrix_Layout, 'N', NumOfRows_Matrix, NumOfCols_B, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cgetrs(Matrix_Layout, 'N', NumOfRows_Matrix, NumOfCols_B, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				&Perm[0].Val, (Loc *)&X(0, 0).Val, LeadingDimension_B);
		}

	}

	///////////////////////////////////////////////////////////////////////////
	// Cholesky factorization and solution

	// CholeskyFactorization creates the matrix Chol, such that A = L*L^T.
	// M must be a symmetric positive-definite matrix. 
	template<class Type, class Size, bool ColMajor = false>
static void CholeskyFactorization(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& L) {
		Assert(A.GetRows() == A.GetCols());

		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();

		// copy of the matrix
		TVVec<Type, Size, ColMajor> M = A;

		// CholeskyStep
		MKLfunctions::CholeskyStep(M);

		// construction of Chol matrix
		L.Gen(NumOfRows_Matrix, NumOfCols_Matrix);
		//TODO fill zeros
		for (Size i = 0; i < NumOfRows_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_Matrix; j++) {
				if (j <= i) { L(i, j) = M(i, j); }
				else { L(i, j) = 0; }
			}
		}
	}

	// solves the equation A * x = b for a symmetric positive-definite matrix A.
	// x and b are vectors. The solution is saved in x.
	template<class Type, class Size, bool ColMajor = false>
static void CholeskySolve(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& x, TVec<Type, Size>& b) {
		Assert(A.GetRows() == b.Len());

		// data used in function
		char uplo = 'L';							// how the Cholesky factorization of Matrix is saved
		Size OrderOfMatrix = A.GetRows();
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		Size NumOfCols_Vector = 1;
		Size LeadingDimension_Vector = ColMajor ? b.Len() : 1;

		// factorization of matrix
		TVVec<Type, Size, ColMajor> M = A;
		MKLfunctions::CholeskyStep(M);

		// solution
		x = b;

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_Vector, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_spotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_Vector, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_Vector, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_Vector, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&x[0].Val, LeadingDimension_Vector);
		}

	}

	// Solves the equation A * X = B for a symmetric positive-definite matrix M.
	// X and B are matrices. The solution is saved in X.
	template<class Type, class Size, bool ColMajor = false>
static void CholeskySolve(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& X, 
		TVVec<Type, Size, ColMajor>& B) {
		Assert(A.GetRows() == B.GetRows());

		// data used in function
		char uplo = 'L';							// how the Cholesky factorization of Matrix is saved
		Size OrderOfMatrix = A.GetRows();
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size NumOfCols_B = B.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		Size LeadingDimension_B = ColMajor ? B.GetRows() : B.GetCols();
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// factorization of matrix
		TVVec<Type, Size, ColMajor> M = A;
		MKLfunctions::CholeskyStep(M);

		// solution
		X = B;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_B, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_spotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_B, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_B, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_cpotrs(Matrix_Layout, uplo, OrderOfMatrix, NumOfCols_B, (const Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&X(0, 0).Val, LeadingDimension_B);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Triangular Matrix solution

	// solves the system A * x = b, where A is a triangular matrix, x and b are vectors. 
	// The solution is saved in x.
	// UpperTriangFlag: if the matrix is upper triangular (true) or lower triangular (false).
	// DiagUnitFlag: if the matrix has ones on the diagonal (true) or not (false).
	template<class Type, class Size, bool ColMajor = false>
static void TriangularSolve(TVVec<TNum<Type>, Size, ColMajor>& A, TVec<TNum<Type>, Size>& x, TVec<TNum<Type>, Size>& b, 
		bool UpperTriangFlag = true, bool DiagonalUnitFlag = false) {
		Assert(A.GetRows() == b.Len());

		// data used for solution
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		char uplo = UpperTriangFlag ? 'U' : 'L';
		char diag = DiagonalUnitFlag ? 'U' : 'N';
		char trans = 'N';							// matrix is not transposed
		Size NumOfCols_Vector = 1;
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		//TODO fix this b.Len()?
		Size LeadingDimension_Vector = ColMajor ? b.Len() : 1;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// solution 
		x = b;
		
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dtrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_Vector, (const Loc *)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_strtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_Vector, (const Loc *)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc *)&x[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			//typedef std::complex<double> Loc;
			LAPACKE_ztrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_Vector, reinterpret_cast<const MKL_Complex16 *>(&A(0, 0).Val),
				LeadingDimension_Matrix, reinterpret_cast<MKL_Complex16 *>(&x[0].Val), LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_ctrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_Vector, (const Loc *)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc *)&x[0].Val, LeadingDimension_Vector);
		}
	}

	// solves the system A * X = B, where A is a triangular matrix, X and B are matrices. 
	// Solution is saved in X.
	// UpperTriangFlag: if the matrix is upper triangular (true) or lower triangular (false).
	// DiagUnitFlag: if the matrix has ones on the diagonal (true) or not (false).
	template<class Type, class Size, bool ColMajor = false>
static void TriangularSolve(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& X, 
		TVVec<Type, Size, ColMajor>& B, bool UpperTriangFlag = true, bool DiagonalUnitFlag = false) {
		Assert(A.GetRows() == B.GetRows());

		// data used for solution
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		char uplo = UpperTriangFlag ? 'U' : 'L';
		char diag = DiagonalUnitFlag ? 'U' : 'N';
		char trans = 'N';							// matrix is not transposed
		Size NumOfCols_B = B.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		Size LeadingDimension_B = ColMajor ? B.GetRows() : B.GetCols();
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// solution 
		X = B;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dtrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_B, (const Loc*)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc*)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_strtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_B, (const Loc*)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc*)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_ztrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_B, (const Loc*)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc*)&X(0, 0).Val, LeadingDimension_B);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_ctrtrs(Matrix_Layout, uplo, trans, diag, NumOfRows_Matrix, NumOfCols_B, (const Loc*)&A(0, 0).Val,
				LeadingDimension_Matrix, (Loc*)&X(0, 0).Val, LeadingDimension_B);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// QR factorization and solution

	// QRFactorization creates the matrices Q and R, such that A = Q*R. A is a m-by-n matrix, where m >= n.
	// Q is a m-by-n ortogonal matrix and R is a n-by-n upper triangular matrix.
	template<class Type, class Size, bool ColMajor = false>
static void QRFactorization(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& Q,
		TVVec<Type, Size, ColMajor>& R) {
		Assert(A.GetRows() >= A.GetCols());

		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// copy of Matrix
		TVVec<Type, Size, ColMajor> M = A;
		TVec<Type, Size> Tau; Tau.Gen(NumOfCols_Matrix);

		// QRStep
		MKLfunctions::QRStep(M, Tau);

		// construction of R matrix
		R.Gen(NumOfCols_Matrix, NumOfCols_Matrix);

		for (Size i = 0; i < NumOfCols_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_Matrix; j++) {
				if (i <= j) { R(i, j) = M(i, j); }
				else { R(i, j) = 0; }
			}
		}

		// construction of Q matrix
		Q = M;
		
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dorgqr(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfCols_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sorgqr(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfCols_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zorgqr(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfCols_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_corgqr(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfCols_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val);
		}
	}

	// It solves the Least Squares problem of equation A * x = b, where A is a m-by-n matrix, where m >= n,
	// x and b are vectors. The solution is saved in x.
	// (Warning: A must be a m-by-n matrix, where m >= n. Otherwise it won't work).
	template<class Type, class Size, bool ColMajor = false>
static void QRSolve(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& x, TVec<Type, Size>& b) {
		Assert(A.GetRows() == b.Len());

		// data used in solution
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// data used for factorization
		TVVec<Type, Size, ColMajor> M = A;
		TVec<Type, Size> Tau; Tau.Gen(NumOfCols_Matrix);

		MKLfunctions::QRStep(M, Tau);

		// multiplication of Q^T * b
		TVec<Type, Size> Temp = b;
		Size NumOfRows_Vector = b.Len();
		Size NumOfCols_Vector = 1;
		Size LeadingDimension_Vector = ColMajor ? b.Len() : 1;
		Size NumOfElementaryReflectors = NumOfCols_Matrix;

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dormqr(Matrix_Layout, 'L', 'T', NumOfRows_Vector, NumOfCols_Vector, NumOfElementaryReflectors,
				(const Loc *)&M(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc; 
			LAPACKE_sormqr(Matrix_Layout, 'L', 'T', NumOfRows_Vector, NumOfCols_Vector, NumOfElementaryReflectors,
				(const Loc *)&M(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&Tau[0].Val, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}


		// calulates the solution Temp with CBLAS function
		const CBLAS_LAYOUT		CBLASLayout = ColMajor ? CBLAS_LAYOUT::CblasColMajor : CBLAS_LAYOUT::CblasRowMajor;		// if matrix M is colmajor or rowmajor
		const CBLAS_SIDE		CBLASSide = CBLAS_SIDE::CblasLeft;														// the matrix is multiplied from the left
		const CBLAS_UPLO		CBLASUplo = CBLAS_UPLO::CblasUpper;														// the matrix R is saved in the upper triangular
		const CBLAS_TRANSPOSE	CBLASTrans = CBLAS_TRANSPOSE::CblasNoTrans;												// the matrix M is not transposed
		const CBLAS_DIAG		CBLASDiag = CBLAS_DIAG::CblasNonUnit;													// the diagonal elements of matrix M are not all 1


		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			cblas_dtrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			cblas_strsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef double Loc;
			cblas_ztrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef float Loc;
			cblas_ctrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}

		// solution
		x = Temp;
	}

	// It solves the Least Squares problem of equation A * X = B, where A is a m-by-n matrix, where m >= n,
	// X and B are matrices. The solution is saved in X.
	// (Warning: Matrix must be a m-by-n matrix, where m >= n. Otherwise it won't work).
	template<class Type, class Size, bool ColMajor = false>
static void QRSolve(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& X, TVVec<Type, Size, ColMajor>& B) {
		Assert(A.GetRows() == B.GetRows());

		// data used in solution
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// data used for factorization
		TVVec<Type, Size, ColMajor> M = A;
		TVec<Type, Size> Tau; Tau.Gen(NumOfCols_Matrix);

		MKLfunctions::QRStep(M, Tau);

		// multiplication of Q^T * B
		TVVec<Type, Size, ColMajor> Temp = B;
		Size NumOfRows_Vector = B.GetRows();
		Size NumOfCols_Vector = B.GetCols();
		Size LeadingDimension_Vector = ColMajor ? B.GetRows() : B.GetCols();
		Size NumOfElementaryReflectors = NumOfCols_Matrix;

		LAPACKE_dormqr(Matrix_Layout, 'L', 'T', NumOfRows_Vector, NumOfCols_Vector, NumOfElementaryReflectors, 
			&M(0, 0).Val, LeadingDimension_Matrix, &Tau[0].Val, &Temp(0, 0).Val, LeadingDimension_Vector);

		// calulates the solution x with CBLAS function
		const CBLAS_LAYOUT		CBLASLayout = ColMajor ? CBLAS_LAYOUT::CblasColMajor : CBLAS_LAYOUT::CblasRowMajor;		// if matrix M is colmajor or rowmajor
		const CBLAS_SIDE		CBLASSide = CBLAS_SIDE::CblasLeft;														// the matrix is multiplied from the left
		const CBLAS_UPLO		CBLASUplo = CBLAS_UPLO::CblasUpper;														// the matrix R is saved in the upper triangular
		const CBLAS_TRANSPOSE	CBLASTrans = CBLAS_TRANSPOSE::CblasNoTrans;												// the matrix M is not transposed
		const CBLAS_DIAG		CBLASDiag = CBLAS_DIAG::CblasNonUnit;													// the diagonal elements of matrix M are not all 1

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			cblas_dtrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			cblas_strsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef double Loc;
			cblas_ztrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef float Loc;
			cblas_ctrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfCols_Matrix, NumOfCols_Vector, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp[0].Val, LeadingDimension_Vector);
		}


		X.Gen(NumOfCols_Matrix, NumOfCols_Vector);
		for (Size i = 0; i < NumOfCols_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_Vector; j++) { X(i, j) = Temp(i, j); }
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// LQ factiorization and solution

	// LQFactorization creates the L and Q matrices, such that A = L*Q, where A is an m-by-n matrix, where m <= n.
	// L is a n-by-n lower triangular matrix and Q is a m-by-n ortogonal matrix.
	template<class Type, class Size, bool ColMajor = false>
static void LQFactorization(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& L,
		TVVec<Type, Size, ColMajor>& Q) {
		Assert(A.GetRows() <= A.GetCols());

		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// copy of Matrix
		TVVec<Type, Size, ColMajor> M = A;
		TVec<Type, Size> Tau; Tau.Gen(NumOfRows_Matrix);

		// LQStep
		MKLfunctions::LQStep(M, Tau);

		// construction of L matrix
		L.Gen(NumOfRows_Matrix, NumOfRows_Matrix);
		for (Size i = 0; i < NumOfRows_Matrix; i++) {
			for (Size j = 0; j < NumOfRows_Matrix; j++) {
				if (j <= i) { L(i, j) = M(i, j); }
				else { L(i, j) = 0; }
			}
		}

		// construction of Q matrix
		Q = M;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dorglq(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfRows_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sorglq(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfRows_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zorglq(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfRows_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_corglq(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, NumOfRows_Matrix, (Loc *)&Q(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val);
		}

	}

	// LQSolve solves the Least Square problem of equation A * x = b, where A is a m-by-n matrix, where m <= n,
	// x and b are vectors. The solution is saved in x.
	// (Warning: matrix A must be a m-by-n matrix, where m <= n. Otherwise it won't work)
	template<class Type, class Size, bool ColMajor = false>
static void LQSolve(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& x, TVec<Type, Size>& b) {
		Assert(A.GetRows() == b.Len());

		// data of matrix
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;


		// LQ factiozation
		TVec<Type, Size> Tau; Tau.Gen(NumOfRows_Matrix);
		TVVec<Type, Size, ColMajor> M = A;

		MKLfunctions::LQStep(M, Tau);

		// data of vector
		Size NumOfRows_b = b.Len();
		Size NumOfCols_b = 1;
		Size LeadingDimension_b = ColMajor ? NumOfRows_b : NumOfCols_b;

		// temporary solution
		TVec<Type, Size> Temp; Temp.Gen(NumOfCols_Matrix);
		for (Size i = 0; i < NumOfRows_b; i++) { Temp[i] = b[i]; }

		// calulates the solution L * Temp = b with CBLAS function
		const CBLAS_LAYOUT		CBLASLayout = ColMajor ? CBLAS_LAYOUT::CblasColMajor : CBLAS_LAYOUT::CblasRowMajor;		// if matrix M is colmajor or rowmajor
		const CBLAS_SIDE		CBLASSide = CBLAS_SIDE::CblasLeft;														// the matrix is multiplied from the left
		const CBLAS_UPLO		CBLASUplo = CBLAS_UPLO::CblasLower;														// the matrix R is saved in the upper triangular
		const CBLAS_TRANSPOSE	CBLASTrans = CBLAS_TRANSPOSE::CblasNoTrans;												// the matrix M is not transposed
		const CBLAS_DIAG		CBLASDiag = CBLAS_DIAG::CblasNonUnit;													// the diagonal elements of matrix M are not all 1

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			cblas_dtrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfRows_Matrix, NumOfCols_b, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc*)&Temp[0].Val, LeadingDimension_b);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			cblas_strsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfRows_Matrix, NumOfCols_b, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc*)&Temp[0].Val, LeadingDimension_b);
		}
		
		// calculates the solution x = Q^T * Temp
		char side = 'L';										// matrix is multiplied from the left
		char trans = 'T';										// matrix is transposed when multiplied
		Size NumOfReflectors = NumOfRows_Matrix;
		Size NumOfRows_Solution = Temp.Len();

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dormlq(Matrix_Layout, side, trans, NumOfRows_Solution, NumOfCols_b, NumOfReflectors, (const Loc*)&M(0, 0).Val,
				LeadingDimension_Matrix, (const Loc*)&Tau[0].Val, (Loc*)&Temp[0].Val, LeadingDimension_b);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sormlq(Matrix_Layout, side, trans, NumOfRows_Solution, NumOfCols_b, NumOfReflectors, (const Loc*)&M(0, 0).Val,
				LeadingDimension_Matrix, (const Loc*)&Tau[0].Val, (Loc*)&Temp[0].Val, LeadingDimension_b);
		}



		x.Gen(NumOfCols_Matrix);
		for (Size i = 0; i < NumOfCols_Matrix; i++) { x[i] = Temp[i]; }
	}

	// LQSolve solves the Least Square problem of equation A * X = B, where A is a m-by-n matrix, where m <= n,
	// X and B are matrices. The solution is saved in X.
	// (Warning: matrix A must be a m-by-n matrix, where m <= n. Otherwise it won't work)
	template<class Type, class Size, bool ColMajor = false>
static void LQSolve(TVVec<Type, Size, ColMajor>& A, TVVec<Type, Size, ColMajor>& X,
		TVVec<Type, Size, ColMajor>& B) {
		Assert(A.GetRows() == B.GetRows());

		// data of matrix
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;

		// LQ factiozation
		TVec<Type, Size> Tau; Tau.Gen(NumOfRows_Matrix);
		TVVec<Type, Size, ColMajor> M = A;

		MKLfunctions::LQStep(M, Tau);

		// data of vector
		Size NumOfRows_B = B.GetRows();
		Size NumOfCols_B = B.GetCols();
		Size LeadingDimension_B = ColMajor ? NumOfRows_B : NumOfCols_B;

		// temporary solution
		TVVec<Type, Size, ColMajor> Temp; Temp.Gen(NumOfCols_Matrix, NumOfCols_B);
		for (Size i = 0; i < NumOfRows_B; i++) {
			for (Size j = 0; j < NumOfCols_B; j++) { Temp(i, j) = B(i, j); }
		}
		Size NumOfRows_Solution = Temp.GetRows();
		Size NumOfCols_Solution = Temp.GetCols();
		Size LeadingDimension_Solution = ColMajor ? NumOfRows_Solution : NumOfCols_Solution;


		// calulates the solution L * Temp = B with CBLAS function
		const CBLAS_LAYOUT		CBLASLayout = ColMajor ? CBLAS_LAYOUT::CblasColMajor : CBLAS_LAYOUT::CblasRowMajor;		// if matrix M is colmajor or rowmajor
		const CBLAS_SIDE		CBLASSide = CBLAS_SIDE::CblasLeft;														// the matrix is multiplied from the left
		const CBLAS_UPLO		CBLASUplo = CBLAS_UPLO::CblasLower;														// the matrix R is saved in the upper triangular
		const CBLAS_TRANSPOSE	CBLASTrans = CBLAS_TRANSPOSE::CblasNoTrans;												// the matrix M is not transposed
		const CBLAS_DIAG		CBLASDiag = CBLAS_DIAG::CblasNonUnit;													// the diagonal elements of matrix M are not all 1


		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			cblas_dtrsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfRows_Matrix, NumOfCols_B, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp(0, 0).Val, LeadingDimension_Solution);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			cblas_strsm(CBLASLayout, CBLASSide, CBLASUplo, CBLASTrans, CBLASDiag, NumOfRows_Matrix, NumOfCols_B, 1.0,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Temp(0, 0).Val, LeadingDimension_Solution);
		}

		// calculates the solution X = Q^T * Temp
		char side = 'L';										// matrix is multiplied from the left
		char trans = 'T';										// matrix is transposed when multiplied
		Size NumOfElementaryReflectors = NumOfRows_Matrix;

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dormlq(Matrix_Layout, side, trans, NumOfRows_Solution, NumOfCols_Solution, NumOfElementaryReflectors,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val, (Loc*)&Temp(0, 0).Val, LeadingDimension_Solution);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sormlq(Matrix_Layout, side, trans, NumOfRows_Solution, NumOfCols_Solution, NumOfElementaryReflectors,
				(const Loc*)&M(0, 0).Val, LeadingDimension_Matrix, (const Loc*)&Tau[0].Val, (Loc*)&Temp(0, 0).Val, LeadingDimension_Solution);
		}

		X.Gen(NumOfCols_Matrix, NumOfCols_B);
		for (Size i = 0; i < NumOfCols_Matrix; i++) {
			for (Size j = 0; j < NumOfCols_B; j++) { X(i, j) = Temp(i, j); }
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// SVD factorization and solution -- currently works only for double and float

	// Makes the SVD factorization of matrix Matrix, such that A = U * Sing * VT.
	// Sing is the vector containing singular values, U is the matrix with left singular vectors,
	// VT is the matrix with right singular vectors.
	template<class Type, class Size, bool ColMajor = false>
static void SVDFactorization(const TVVec<Type, Size, ColMajor>& A,
		TVVec<Type, Size, ColMajor>& U, TVec<Type, Size>& Sing,
		TVVec<Type, Size, ColMajor>& VT) {

		// data used for factorization
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();

		// handle edge cases where the factorization is trivial. Double and float only!
		if (NumOfRows_Matrix == 1) {
			U.Gen(1, 1);
			U(0, 0) = 1;
			VT = A;
			Sing.Gen(1);			
			// normalize VT and set Sing[0] = oldnorm(VT)
			TVec<Type, Size>& RawV = VT.Get1DVec();
			Sing[0] = TLinAlg::Normalize(RawV);			
			return;
		} else if (NumOfCols_Matrix == 1) {
			VT.Gen(1, 1);
			VT(0, 0) = 1;
			U = A;
			Sing.Gen(1);
			// normalize U and set Sing[0] = oldnorm(U)
			TVec<Type, Size>& RawV = U.Get1DVec();
			Sing[0] = TLinAlg::Normalize(RawV);
			return;
		}

		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		// preperation for factorization
		Sing.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix));
		//Fix this part, results are only double or float due to bidiagonalization
		TVec<Type, Size> UpDiag, TauQ, TauP;
		UpDiag.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix) - 1);
		TauQ.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix));
		TauP.Gen(MIN(NumOfRows_Matrix, NumOfCols_Matrix));

		// bidiagonalization of Matrix
		TVVec<Type, Size, ColMajor> M = A;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgebrd(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&Sing[0].Val, &UpDiag[0].Val, (Loc *)&TauQ[0].Val, (Loc *)&TauP[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sgebrd(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&Sing[0].Val, (Loc *)&UpDiag[0].Val, (Loc *)&TauQ[0].Val, (Loc *)&TauP[0].Val);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			typedef double LocSing;
			LAPACKE_zgebrd(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(LocSing *)&Sing[0].Val, (LocSing *)&UpDiag[0].Val, (Loc *)&TauQ[0].Val, (Loc *)&TauP[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			typedef float LocSing;
			LAPACKE_cgebrd(Matrix_Layout, NumOfRows_Matrix, NumOfCols_Matrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(LocSing *)&Sing[0].Val, (LocSing *)&UpDiag[0].Val, (Loc *)&TauQ[0].Val, (Loc *)&TauP[0].Val);
		}

		// matrix U used in the SVD factorization
		//Andrej: Fix this for complex matrices
		U = M;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dorgbr(Matrix_Layout, 'Q', NumOfRows_Matrix, MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix,
				(Loc *)&U(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauQ[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sorgbr(Matrix_Layout, 'Q', NumOfRows_Matrix, MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix,
				(Loc *)&U(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauQ[0].Val);
		}
		/*else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zorgbr(Matrix_Layout, 'Q', NumOfRows_Matrix, MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix,
				(Loc *)&U(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauQ[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_corgbr(Matrix_Layout, 'Q', NumOfRows_Matrix, MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix,
				(Loc *)&U(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauQ[0].Val);
		}*/

		// matrix VT used in the SVD factorization
		//Andrej: Add support for complex matrices
		VT = M;
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dorgbr(Matrix_Layout, 'P', MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix, NumOfRows_Matrix,
				(Loc *)&VT(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauP[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sorgbr(Matrix_Layout, 'P', MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix, NumOfRows_Matrix,
				(Loc *)&VT(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauP[0].Val);
		}
		/*else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_zorgbr(Matrix_Layout, 'P', MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix, NumOfRows_Matrix,
				(Loc *)&VT(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauP[0].Val);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_corgbr(Matrix_Layout, 'P', MIN(NumOfRows_Matrix, NumOfCols_Matrix), NumOfCols_Matrix, NumOfRows_Matrix,
				(Loc *)&VT(0, 0).Val, LeadingDimension_Matrix, (const Loc *)&TauP[0].Val);
		}*/

		// factorization
//		TVVec<Type, Size, ColMajor> C;	// TODO do we need this matrix? Can't we just use a nullptr on the input???
		char UpperLower = NumOfRows_Matrix >= NumOfCols_Matrix ? 'U' : 'L';
		Size LeadingDimension_VT = ColMajor ? VT.GetRows() : VT.GetCols();
		Size LeadingDimension_U = ColMajor ? U.GetRows() : U.GetCols();

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dbdsqr(
				Matrix_Layout,
				UpperLower,
				Sing.Len(),
				VT.GetCols(),
				U.GetRows(),
				0,									// NCC, if NCC == 0 then C is not referenced
				(Loc *)&Sing[0].Val,
				(Loc *)&UpDiag[0].Val,
				(Loc *)&VT(0, 0).Val,
				LeadingDimension_VT,
				(Loc *)&U(0, 0).Val,				// U: U is overwritten by U * Q
				LeadingDimension_U,
				nullptr,//(Loc *)&C(0, 0).Val,		// C: C is overwritten by Q**T * C
				1									// LDC: LDC >= max(1,N) if NCC > 0; LDC >=1 if NCC = 0
			);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_sbdsqr(Matrix_Layout, UpperLower, Sing.Len(), VT.GetCols(), U.GetRows(), 0, (Loc *)&Sing[0].Val, (Loc *)&UpDiag[0].Val,
				(Loc *)&VT(0, 0).Val, LeadingDimension_VT, (Loc *)&U(0, 0).Val, LeadingDimension_U, nullptr, 1);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			typedef double LocSing;
			LAPACKE_zbdsqr(Matrix_Layout, UpperLower, Sing.Len(), VT.GetCols(), U.GetRows(), 0, (LocSing *)&Sing[0].Val, (LocSing *)&UpDiag[0].Val,
				(Loc *)&VT(0, 0).Val, LeadingDimension_VT, (Loc *)&U(0, 0).Val, LeadingDimension_U, nullptr, 1);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			typedef float LocSing;
			LAPACKE_cbdsqr(Matrix_Layout, UpperLower, Sing.Len(), VT.GetCols(), U.GetRows(), 0, (LocSing *)&Sing[0].Val, (LocSing *)&UpDiag[0].Val,
				(Loc *)&VT(0, 0).Val, LeadingDimension_VT, (Loc *)&U(0, 0).Val, LeadingDimension_U, nullptr, 1);
		}
	}

	// SVDSolve solves the Least Squares problem of equation A * x = b, where A is a matrix, x and b are vectors.
	// The solution is saved in x.
	template<class Type, class Size, bool ColMajor = false>
static void SVDSolve(const TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& x,
			const TVec<Type, Size>& b, const Type& EpsSing) {
		Assert(A.GetRows() == b.Len());

		// data used for solution
		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();

		// generating the SVD factorization
		TVVec<Type, Size, ColMajor> U, VT, M = A;
		TVec<Type, Size> Sing;
		MKLfunctions::SVDFactorization(M, U, Sing, VT);

		// generating temporary solution
		x.Gen(NumOfCols_Matrix); TLinAlgTransform::Fill(x, 0);
		TVec<Type, Size> ui; ui.Gen(U.GetRows());
		TVec<Type, Size> vi; vi.Gen(VT.GetCols());

		Size i = 0;
		while (i < MIN(NumOfRows_Matrix, NumOfCols_Matrix) && Sing[i].Val > EpsSing*Sing[0]) {
			U.GetCol(i, ui);
			VT.GetRow(i, vi);
			Type Scalar = TLinAlg::DotProduct(ui, b) / Sing[i].Val;
			//Correct Type->TNum<Type>! in the whole file
			TLinAlg::AddVec(Scalar.Val, vi, x);
			i++;
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Eigenvalues and eigenvectors Symmetric

	// Generates the vector of Eigenvalues and a matrix of Eigenvectors for a n-by-n matrix A.
	template<class Type, class Size, bool ColMajor = false>
static void SymmetricEigen(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& Eigenvalues, 
		TVVec<Type, Size, ColMajor>& Eigenvectors) {
		Assert(A.GetRows() == A.GetCols());

		Eigenvectors = A;
		char jobz = 'V';													// computes both eigenvalues and eigenvectors
		char uplo = 'U';													// stores the upper part of Matrix
		Size OrderOfMatrix = Eigenvectors.GetRows();
		Size LeadingDimension_Matrix = ColMajor ? Eigenvectors.GetRows() : Eigenvectors.GetCols();
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		Eigenvalues.Gen(OrderOfMatrix);


		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dsyev(Matrix_Layout, jobz, uplo, OrderOfMatrix, (Loc *)&Eigenvectors(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&Eigenvalues[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_ssyev(Matrix_Layout, jobz, uplo, OrderOfMatrix, (Loc *)&Eigenvectors(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&Eigenvalues[0].Val);
		}
	}

	// Generates the vector of Eigenvalues for a n-by-n matrix A.
	template<class Type, class Size, bool ColMajor = false>
static void SymmetricEigen(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& Eigenvalues) {
		Assert(A.GetRows() == A.GetCols());

		TVVec<Type, Size, ColMajor> M = A;
		char jobz = 'N';													// computes only eigenvalues
		char uplo = 'U';													// stores the upper part of Matrix
		Size OrderOfMatrix = M.GetRows();
		Size LeadingDimension_Matrix = ColMajor ? M.GetRows() : M.GetCols();
		int Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;

		Eigenvalues.Gen(OrderOfMatrix);

		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dsyev(Matrix_Layout, jobz, uplo, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Eigenvalues[0].Val);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_ssyev(Matrix_Layout, jobz, uplo, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix, (Loc *)&Eigenvalues[0].Val);
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// Eigenvalues and eigenvectors Unsymmetric

	// Generates the Eigenvalues of a n-by-n matrix A. The real parts of eigenvalues are saved in 
	// EigenvaluesReal and the imaginary parts of eigenvalues are saved in EigenvaluesImag.
	template<class Type, class Size, bool ColMajor = false>
static void UnsymmetricEigen(TVVec<Type, Size, ColMajor>& A, TVec<Type, Size>& EigenvaluesReal, 
		TVec<Type, Size>& EigenvaluesImag) {
		Assert(A.GetRows() == A.GetCols());

		Size NumOfRows_Matrix = A.GetRows();
		Size NumOfCols_Matrix = A.GetCols();
		Size OrderOfMatrix = NumOfRows_Matrix;
		Size LeadingDimension_Matrix = ColMajor ? NumOfRows_Matrix : NumOfCols_Matrix;
		Size Matrix_Layout = ColMajor ? LAPACK_COL_MAJOR : LAPACK_ROW_MAJOR;
		
		// copying Matrix (perserving the original)
		TVVec<Type, Size, ColMajor> M = A;

		// not used but needed for function: left and right eigenvectors
		char LeftEigen = 'N';												// doesn't compute left eigenvectors
		char RightEigen = 'N';												// doesn't compute right eigenvectors
		TVVec<Type, Size, ColMajor> EigenvectorsLeft, EigenvectorsRight;
		EigenvectorsLeft.Gen(NumOfRows_Matrix, NumOfCols_Matrix); 
		EigenvectorsRight.Gen(NumOfRows_Matrix, NumOfCols_Matrix);

		// allocating space for eigenvalues
		EigenvaluesReal.Gen(NumOfRows_Matrix);
		EigenvaluesImag.Gen(NumOfRows_Matrix);

		// eigenvalues calculation
		if (TypeCheck::is_double<Type>::value == true){
			typedef double Loc;
			LAPACKE_dgeev(Matrix_Layout, LeftEigen, RightEigen, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvaluesReal[0].Val, (Loc *)&EigenvaluesImag[0].Val, (Loc *)&EigenvectorsLeft(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvectorsRight(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_float<Type>::value == true){
			typedef float Loc;
			LAPACKE_dgeev(Matrix_Layout, LeftEigen, RightEigen, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvaluesReal[0].Val, (Loc *)&EigenvaluesImag[0].Val, (Loc *)&EigenvectorsLeft(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvectorsRight(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_complex_double<Type>::value == true){
			typedef std::complex<double> Loc;
			LAPACKE_dgeev(Matrix_Layout, LeftEigen, RightEigen, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvaluesReal[0].Val, (Loc *)&EigenvaluesImag[0].Val, (Loc *)&EigenvectorsLeft(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvectorsRight(0, 0).Val, LeadingDimension_Matrix);
		}
		else
		if (TypeCheck::is_complex_float<Type>::value == true){
			typedef std::complex<float> Loc;
			LAPACKE_dgeev(Matrix_Layout, LeftEigen, RightEigen, OrderOfMatrix, (Loc *)&M(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvaluesReal[0].Val, (Loc *)&EigenvaluesImag[0].Val, (Loc *)&EigenvectorsLeft(0, 0).Val, LeadingDimension_Matrix,
				(Loc *)&EigenvectorsRight(0, 0).Val, LeadingDimension_Matrix);
		}
	}
};
#endif

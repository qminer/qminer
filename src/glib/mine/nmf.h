/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef NMF_H
#define NMF_H

//============================================================
// NON-NEGATIVE MATRIX FACTORIZATION - CLASS DECLARATION
//============================================================

// Non-negative matrix factorization
// For the m x n non-negative matrix A calculates the m x r non-negative matrix U
// and r x n non-negative matrix V, such that A ~ U*V
class TNmf {

public:
	enum TGradType { gtNormal, gtRecSys };
	// calculates the NMF using Gradient Projection with first order approximation (Coordinate search)
	template <class TMatType>
	static void CFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter = 10000, 
		const double& Eps = 1e-3, const PNotify& TNotify = TNotify::NullNotify, const TGradType& GradType = TGradType::gtNormal);

	// calculates the NMF using Gradient Projection when the values A are not all known (used for Collaborate Filtering)
	template <class TMatType>
	static void EM(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter = 10000,
		const double& Eps = 1e-3, const PNotify& TNotify = TNotify::NullNotify, const TGradType& GradType = TGradType::gtNormal);
	

private:
	template <class TMatType>
	static void InitializationUV(const TMatType& A, const int& Rows, const int& Cols, const int& R, const TGradType& GradType, TFltVV& U, TFltVV& V);

	// scaling initial matrices
	template <class TMatType>
	static void InitScaling(const TMatType& A, TFltVV& U, TFltVV& V);

	// calculates the starting condition
	template <class TMatType>
	static double StoppingCondition(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const TGradType& GradType, const double& Eps);

	// calculates the condition value
	template <class TMatType>
	static double ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const TGradType& GradType);

	// update scaling for better stability of matrices
	static void UpdateScaling(TFltVV& U, TFltVV& V);

	// updates the dense matrix A in the EM algorithm
	template <class TMatType>
	static void UpdateA(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& DenseA);

	// update U
	template <class TMatType>
	static void UpdateU(const TMatType& A, TFltVV& U, const TFltVV& V, const TGradType& GradType, double& L1, const double& beta, const double& FrobA);
	// update V
	template <class TMatType>
	static void UpdateV(const TMatType& A, const TFltVV& U, TFltVV& V, const TGradType& GradType, double& L2, const double& beta, const double& FrobA);

	// get gradient by U
	template <class TMatType>
	static void GetGradientU(const TMatType& A, const TFltVV& U, const TFltVV& V, const TGradType& GradType, TFltVV& GradU);
	// get gradient by V
	template <class TMatType>
	static void GetGradientV(const TMatType& A, const TFltVV& U, const TFltVV& V, const TGradType& GradType, TFltVV& GradV);

	// gets number of rows
	static int NumOfRows(const TFltVV& Mat);
	static int NumOfRows(const TVec<TIntFltKdV>& Mat);

	// gets number of columns
	static int NumOfCols(const TFltVV& Mat);
	static int NumOfCols(const TVec<TIntFltKdV>& Mat);

	// gets the value of the matrix
	static double GetMatVal(const TFltVV& Mat, const int& RowId, const int& ColId);
	static double GetMatVal(const TVec<TIntFltKdV>& Mat, const int& RowId, const int& ColId);

	// get column of the matrix
	static void GetMatCol(const TFltVV& Mat, const int& ColId, TFltV& Vec);
	static void GetMatCol(const TVec<TIntFltKdV>& Mat, const int& ColId, TFltV& Vec);
};

//============================================================
// NON-NEGATIVE MATRIX FACTORIZATION - CLASS IMPLEMENTATION
//============================================================

template <class TMatType>
void TNmf::CFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter, 
	const double& Eps, const PNotify& Notify, const TGradType& GradType) {

	int Rows = NumOfRows(A);
	int Cols = NumOfCols(A);

	EAssert(0 < R && R <= Rows && R <= Cols);
	Notify->OnNotify(TNotifyType::ntInfo, "Executing NMF ...");

	// initialize the matrices U and V
	InitializationUV(A, Rows, Cols, R, GradType, U, V);

	// scale the matrices U and V for a better starting point:
	InitScaling(A, U, V);
	// calculate the stopping condition limit
	double StopCond = StoppingCondition(A, R, U, V, GradType, Eps);
	
	int IterN = 0; 
	double L1 = 1.0, L2 = 1.0, beta = 1.2;
	double Frob2A = TLinAlg::Frob2(A);

	do {
		if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }
		// updating U
		UpdateU(A, U, V, GradType, L1, beta, Frob2A);
		// updating V
		UpdateV(A, U, V, GradType, L2, beta, Frob2A);
		// scaling the matrices
		UpdateScaling(U, V);
		// check for stopping condition
		double Condition = ProjectedGradientNorm(A, R, U, V, GradType);
		if (Condition <= StopCond) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", IterN);
			break;
		}
		// prepare the step size for the next iteration
		L1 = L1 / beta;
		L2 = L2 / beta;
	} while (++IterN < MaxIter);
}

template <class TMatType>
void TNmf::EM(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter,
	const double& Eps, const PNotify& Notify, const TGradType& GradType) {

	int Rows = NumOfRows(A);
	int Cols = NumOfCols(A);

	EAssert(0 < R && R <= Rows && R <= Cols);
	Notify->OnNotify(TNotifyType::ntInfo, "Executing EM ...");

	TFltVV DenseA(Rows, Cols);
	for (int ColN = 0; ColN < Cols; ColN++) {
		TFltV Col; GetMatCol(A, ColN, Col);
		DenseA.SetCol(ColN, Col);
	}
	TFltV Means; TLAMisc::Mean(DenseA, Means, TMatDim::mdRows);
	for (int ColN = 0; ColN < Cols; ColN++) {
		for (int RowN = 0; RowN < Rows; RowN++) {
			if (DenseA(RowN, ColN) == 0.0) {
				DenseA(RowN, ColN) = Means[RowN];
			}
		}
	}

	// initialize the matrices U and V
	InitializationUV(DenseA, Rows, Cols, R, GradType, U, V);

	// scale the matrices U and V for a better starting point:
	InitScaling(DenseA, U, V);
	// calculate the stopping condition limit
	double StopCond = StoppingCondition(DenseA, R, U, V, GradType, Eps);

	int IterN = 0;
	double L1 = 1.0, L2 = 1.0, beta = 1.2;
	double Frob2A = TLinAlg::Frob2(A);

	do {
		if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }
		// updating U
		UpdateU(DenseA, U, V, GradType, L1, beta, Frob2A);
		// updating V
		UpdateV(DenseA, U, V, GradType, L2, beta, Frob2A);
		// scaling the matrices
		UpdateScaling(U, V);
		// check for stopping condition
		double Condition = ProjectedGradientNorm(DenseA, R, U, V, GradType);
		if (Condition <= StopCond) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", IterN);
			break;
		}
		// prepare the step size for the next iteration
		L1 = L1 / beta;
		L2 = L2 / beta;
		UpdateA(A, U, V, DenseA);
	} while (++IterN < MaxIter);
}

template <class TMatType>
void TNmf::InitializationUV(const TMatType& A, const int& Rows, const int& Cols, const int& R, const TGradType& GradType, TFltVV& U, TFltVV& V) {
	if (GradType == TGradType::gtNormal) {
		U.Gen(Rows, R); TLAMisc::FillRnd(U);
		V.Gen(R, Cols); TLAMisc::FillRnd(V);
	}
	else if (GradType == TGradType::gtRecSys) {
		EM(A, R, U, V, 10);
	}
}

template <class TMatType>
void TNmf::InitScaling(const TMatType& A, TFltVV& U, TFltVV& V) {
	// alpha = <A, U*V> / <U*V, U*V>
	TFltVV UV;  TLinAlg::Multiply(U, V, UV);
	double alpha = TLinAlg::DotProduct(A, UV) / TLinAlg::DotProduct(UV, UV);
	// U = sqrt(alpha) * U
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), U, U);
	// V = sqrt(alpha) * V
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), V, V);
}

template <class TMatType>
double TNmf::StoppingCondition(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const TGradType& GradType, const double& Eps) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// NablaU = U*(V*V') - A*V'  
	TFltVV GradU; GetGradientU(A, U, V, GradType, GradU);
	// NablaV = V'*(U'*U) - A'*U
	TFltVV GradV; GetGradientV(A, U, V, GradType, GradV);

	double NormGradU = TLinAlg::Frob(GradU);
	double NormGradV = TLinAlg::Frob(GradV);

	return Eps * (NormGradU + NormGradV);
}

template <class TMatType>
double TNmf::ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const TGradType& GradType) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// GradU = U*(V*V') - A*V'  
	TFltVV GradU; GetGradientU(A, U, V, GradType, GradU);

	// NablaV = V'*(U'*U) - A'*U
	TFltVV GradV; GetGradientV(A, U, V, GradType, GradV);

	// projected gradient for matrix U and V 
	TFltVV ProjU(Rows, R), ProjV(R, Cols);

	for (int j = 0; j < R; j++) {
		// construct the projected gradient for matrix U
		for (int i = 0; i < Rows; i++) {
			double ProjU_ij = (double)U.At(i, j) > 0.0 ? GradU.At(i, j) : TMath::Mn(0.0, (double)GradU.At(i, j));
			ProjU.PutXY(i, j, ProjU_ij);
		}
		// construct the projected gradient for matrix V
		for (int i = 0; i < Cols; i++) {
			double ProjV_ij = (double)V.At(j, i) > 0.0 ? GradV.At(j, i) : TMath::Mn(0.0, (double)GradV.At(j, i));
			ProjV.PutXY(j, i, ProjV_ij);
		}
	}
	double NormProjU = TLinAlg::Frob(ProjU);
	double NormProjV = TLinAlg::Frob(ProjV);
	return NormProjU + NormProjV;
}

template <class TMatType>
void TNmf::UpdateA(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& DenseA) {
	const int& Rows = NumOfRows(A);
	const int& Cols = NumOfCols(A);

	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			double Val = GetMatVal(A, RowN, ColN);
			if (Val == 0.0) {
				DenseA(RowN, ColN) = UV(RowN, ColN);
			}
		}
	}

}

template <class TMatType>
void TNmf::UpdateU(const TMatType& A, TFltVV& U, const TFltVV& V, const TGradType& GradType, double& L1, const double& beta, const double& FrobA) {
	// NormFX = || A - U*V ||_{F}^{2}
	TFltVV FX;  TLinAlg::Multiply(U, V, FX);
	TLinAlg::LinComb(-1, FX, 1, A, FX);
	double NormFX = TLinAlg::Frob2(FX);
	
	// GradU = U*V*V' - A*V';
	TFltVV GradU; GetGradientU(A, U, V, GradType, GradU);

	// update matrix U = [U - 1/L1 * GradU]+
	const TFltVV OldU = U; 
	TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
	TLAMisc::NonNegProj(U);

	// prepare the condition variables
	TFltVV UU; TLinAlg::MultiplyT(U, U, UU);
	TFltVV UOldU; TLinAlg::LinComb(1, U, -1, OldU, UOldU);

	TFltVV VT; TLinAlg::Transpose(V, VT);
	TFltVV VV; TLinAlg::Multiply(V, VT, VV);
	TFltVV AV; TLinAlg::Multiply(A, VT, AV);

	// 1/2 * (Frob(A)^2 - 2 * <U, A*V'> + <U'*U, V*V'> - NormFX) > (<GradU, U - OldU> + L1 / 2 * Frob(U - OldU)^2)
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(U, AV) + TLinAlg::DotProduct(UU, VV) - NormFX) > TLinAlg::DotProduct(GradU, UOldU) + L1 / 2.0 * TLinAlg::Frob2(UOldU)) {
		// update the matrix U
		L1 = L1 * beta;
		TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
		TLAMisc::NonNegProj(U);

		// update the matrices for the condition
		TLinAlg::MultiplyT(U, U, UU);
		TLinAlg::LinComb(1, U, -1, OldU, UOldU);
	}
}

template <class TMatType>
void TNmf::UpdateV(const TMatType& A, const TFltVV& U, TFltVV& V, const TGradType& GradType, double& L2, const double& beta, const double& FrobA) {
	// NormFY = || A - U*V ||_{F}^{2}
	TFltVV FY; TLinAlg::Multiply(U, V, FY);
	TLinAlg::LinComb(-1, FY, 1, A, FY);
	double NormFY = TLinAlg::Frob2(FY);

	// GradV = U'*U*V - U'*A
	TFltVV GradV; GetGradientV(A, U, V, GradType, GradV);

	// update matrix V = [V - 1/L2 * GradV]+
	const TFltVV OldV = V;
	TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
	TLAMisc::NonNegProj(V);

	// prepare the condition variables
	TFltVV VT; TLinAlg::Transpose(V, VT);
	TFltVV VV; TLinAlg::Multiply(V, VT, VV);

	TFltVV UA; TLinAlg::MultiplyT(U, A, UA);
	TFltVV UU; TLinAlg::MultiplyT(U, U, UU);

	TFltVV VoldV; TLinAlg::LinComb(1, V, -1, OldV, VoldV);
	// 1/2 * (Frob(A)^2 - 2 * <U'*A, V> + <V*V', U'*U> - NormFY) > <GradV, V - OldV> + L2 / 2 * Frob(V - OldV)^2
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(UA, V) + TLinAlg::DotProduct(VV, UU) - NormFY) > TLinAlg::DotProduct(GradV, VoldV) + L2 / 2.0 * TLinAlg::Frob2(VoldV)) {
		// update the matrix V
		L2 = L2 * beta;
		TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
		TLAMisc::NonNegProj(V);

		// update the matrices for the condition
		TLinAlg::Transpose(V, VT);
		TLinAlg::Multiply(V, VT, VV);
		TLinAlg::LinComb(1, V, -1, OldV, VoldV);
	}
}


template <class TMatType>
void TNmf::GetGradientU(const TMatType& A, const TFltVV& U, const TFltVV& V, const TGradType& GradType, TFltVV& GradU) {
	TFltVV VT;  TLinAlg::Transpose(V, VT);
	// GradU = U*V*V' - A*V';
	if (GradType == TGradType::gtNormal) {
		TFltVV VV;  TLinAlg::Multiply(V, VT, VV);
		TFltVV UVV; TLinAlg::Multiply(U, VV, UVV);
		TFltVV AV;  TLinAlg::Multiply(A, VT, AV);
		TLinAlg::LinComb(1, UVV, -1, AV, GradU);
	}
	// GradU = (W o (U*V))*V' - A*V', W(i, j) = A(i, j) > 0 ? 1 : 0
	else if (GradType == TGradType::gtRecSys) {
		TFltVV UV; TLinAlg::Multiply(U, V, UV);
		// W o (U*V)
		TLinAlg::HadamardProd(A, UV, UV, true);
		TFltVV UVV; TLinAlg::Multiply(UV, VT, UVV);
		TFltVV AV; TLinAlg::Multiply(A, VT, AV);
		TLinAlg::LinComb(1, UVV, -1, AV, GradU);
		TLinAlg::LinComb(1, GradU, 0.001, U, GradU);
	}
	else {
		throw TExcept::New("GradType not recognized!");
	}
}

template <class TMatType>
void TNmf::GetGradientV(const TMatType& A, const TFltVV& U, const TFltVV& V, const TGradType& GradType, TFltVV& GradV) {
	// GradV = U'*U*V - U'*A
	if (GradType == TGradType::gtNormal) {
		TFltVV UU;  TLinAlg::MultiplyT(U, U, UU);
		TFltVV UUV; TLinAlg::Multiply(UU, V, UUV);
		TFltVV UA;  TLinAlg::MultiplyT(U, A, UA);
		TLinAlg::LinComb(1, UUV, -1, UA, GradV);
	}
	// GradV = U'*(W o U*V) - U'*A, , W(i, j) = A(i, j) > 0 ? 1 : 0
	else if (GradType == TGradType::gtRecSys) {
		TFltVV UV; TLinAlg::Multiply(U, V, UV);
		TLinAlg::HadamardProd(A, UV, UV, true);
		TFltVV UUV; TLinAlg::MultiplyT(U, UV, UUV);
		TFltVV UA; TLinAlg::MultiplyT(U, A, UA);
		TLinAlg::LinComb(1, UUV, -1, UA, GradV);
		TLinAlg::LinComb(1, GradV, 0.001, V, GradV);
	}
	else {
		throw TExcept::New("GradType not recognized!");
	}
}

#endif

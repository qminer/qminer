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
	//============================================================
	// ALGORITHM FUNCTIONS
	//============================================================

	// calculates the NMF using Gradient Projection with first order approximation (Coordinate search)
	template <class TMatType>
	static void CFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter = 10000, 
		const double& Eps = 1e-3, const PNotify& TNotify = TNotify::NullNotify);	

	// calculates the Weighted NMF using Gradient Projection with first order approximation (Coordinate search)
	// weights are nonnegative (currently the weight matrix is W, W_ij = A_ij > 0 ? 1 : 0)
	template <class TMatType>
	static void WeightedCFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter = 10000,
		const double& Eps = 1e-3, const PNotify& TNotify = TNotify::NullNotify);

private:
	//============================================================
	// HELPER FUNCTIONS
	//============================================================

	// types of gradient calculations
	enum TGradType { gtNormal, gtWeighted };

	static void InitializeUV(const int& Rows, const int& Cols, const int& R, TFltVV& U, TFltVV& V);

	// weight initializations
	static void InitializeWeights(const TFltVV& A, TFltVV& W);
	static void InitializeWeights(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& W);

	// calculates the upper bound of the stopping condition
	template <class TMatType>
	static double StoppingCondition(const TMatType& A, const TFltVV& U, const TFltVV& V, 
		const double& Eps, const TGradType& GradType);

	// calculates the condition value
	template <class TMatType>
	static double ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V);
	template <class TMatType>
	static double WeightedProjectedGradientNorm(const TMatType& A, const int& R, 
		const TMatType& W, const TFltVV& U, const TFltVV& V);

	// update scaling for better stability of matrices
	static void UpdateScale(TFltVV& U, TFltVV& V);

	// update U
	template <class TMatType>
	static void UpdateU(const TMatType& A, TFltVV& U, const TFltVV& V, double& L1, 
		const double& beta, const double& FrobA);
	template <class TMatType>
	static void UpdateWeightedU(const TMatType& A, TFltVV& U, const TFltVV& V, const TMatType& W,
		double& L1, const double& beta, const double& FrobA);

	// update V
	template <class TMatType>
	static void UpdateV(const TMatType& A, const TFltVV& U, TFltVV& V, double& L2, 
		const double& beta, const double& FrobA);
	template <class TMatType>
	static void UpdateWeightedV(const TMatType& A, const TFltVV& U, TFltVV& V, const TMatType& W,
		double& L2, const double& beta, const double& FrobA);

	// get gradient by U
	template <class TMatType>
	static void GetGradientU(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& GradU);
	template <class TMatType>
	static void GetWeightedGradientU(const TMatType& A, const TMatType& W, 
		const TFltVV& U, const TFltVV& V, TFltVV& GradU);

	// get gradient by V
	template <class TMatType>
	static void GetGradientV(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& GradV);
	template <class TMatType>
	static void GetWeightedGradientV(const TMatType& A, const TMatType& W, 
		const TFltVV& U, const TFltVV& V, TFltVV& GradV);

	// gets number of rows
	static int NumOfRows(const TFltVV& Mat);
	static int NumOfRows(const TVec<TIntFltKdV>& Mat);

	// gets number of columns
	static int NumOfCols(const TFltVV& Mat);
	static int NumOfCols(const TVec<TIntFltKdV>& Mat);
};

//============================================================
// NON-NEGATIVE MATRIX FACTORIZATION - CLASS IMPLEMENTATION
//============================================================

template <class TMatType>
void TNmf::CFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter, 
	const double& Eps, const PNotify& Notify) {

	int Rows = NumOfRows(A);
	int Cols = NumOfCols(A);

	EAssert(0 < R && R <= Rows && R <= Cols);
	Notify->OnNotify(TNotifyType::ntInfo, "Executing NMF ...");

	// initialize the matrices U and V
	InitializeUV(Rows, Cols, R, U, V);

	// scale the matrices U and V for a better starting point
	// alpha = <A, U*V> / <U*V, U*V>
	TFltVV UV;  TLinAlg::Multiply(U, V, UV);
	double alpha = TLinAlg::DotProduct(A, UV) / TLinAlg::DotProduct(UV, UV);
	// U = sqrt(alpha) * U
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), U, U);
	// V = sqrt(alpha) * V
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), V, V);

	// calculate the stopping condition limit
	double StopCond = StoppingCondition(A, U, V, Eps, TGradType::gtNormal);
	
	int IterN = 0; 
	double L1 = 1.0, L2 = 1.0, beta = 2.0;
	double Frob2A = TLinAlg::Frob2(A);

	do {
		if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }
		// updating U
		UpdateU(A, U, V, L1, beta, Frob2A);
		// updating V
		UpdateV(A, U, V, L2, beta, Frob2A);
		// scaling the matrices
		UpdateScale(U, V);
		// check for stopping condition
		double Condition = ProjectedGradientNorm(A, R, U, V);
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
void TNmf::WeightedCFO(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter,
	const double& Eps, const PNotify& Notify) {

	int Rows = NumOfRows(A);
	int Cols = NumOfCols(A);

	EAssert(0 < R && R <= Rows && R <= Cols);
	Notify->OnNotify(TNotifyType::ntInfo, "Executing NMF ...");

	// the weight matrix
	TMatType W; InitializeWeights(A, W);
	// initalize the matrices U and V
	InitializeUV(Rows, Cols, R, U, V);

	// scale the matrices U and V for a better starting point
	// alpha = <W o A, U*V> / <W o U*V, U*V>
	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	TFltVV WUV; TLinAlg::HadamardProd(W, UV, WUV);
	double alpha = TLinAlg::DotProduct(A, UV) / TLinAlg::DotProduct(UV, WUV);
	// U = sqrt(alpha) * U
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), U, U);
	// V = sqrt(alpha) * V
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), V, V);

	// calculate the stopping condition limit
	double StopCond = StoppingCondition(A, U, V, Eps, TGradType::gtWeighted);

	int IterN = 0;
	double L1 = 1.0, L2 = 1.0, beta = 2.0;
	double Frob2A = TLinAlg::Frob2(A);

	do {
		if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }
		// updating U
		UpdateWeightedU(A, U, V, W, L1, beta, Frob2A);
		// updating V
		UpdateWeightedV(A, U, V, W, L2, beta, Frob2A);
		// scaling the matrices
		UpdateScale(U, V);
		// check for stopping condition
		double Condition = WeightedProjectedGradientNorm(A, R, W, U, V);
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
double TNmf::StoppingCondition(const TMatType& A, const TFltVV& U, const TFltVV& V, const double& Eps,
	const TGradType& GradType) {
	TFltVV GradU;
	TFltVV GradV;

	// calculate the gradients
	if (GradType == TGradType::gtNormal) {
		// NablaU = U*(V*V') - A*V'  
		GetGradientU(A, U, V, GradU);
		// NablaV = V'*(U'*U) - A'*U
		GetGradientV(A, U, V, GradV);
	}
	else if (GradType == TGradType::gtWeighted) {
		TMatType W; InitializeWeights(A, W);
		// GradU = (W o U*V)*V' - A*V'
		GetWeightedGradientU(A, W, U, V, GradU);
		// GradV = U'*(W o U*V) - U'*A
		GetWeightedGradientV(A, W, U, V, GradV);
	}
	else {
		throw TExcept::New("TNmf::StoppingCondition: Invalid GradType parameter!");
	}

	double NormGradU = TLinAlg::Frob(GradU);
	double NormGradV = TLinAlg::Frob(GradV);

	return Eps * (NormGradU + NormGradV);
}

template <class TMatType>
double TNmf::ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// GradU = U*(V*V') - A*V'  
	TFltVV GradU; GetGradientU(A, U, V, GradU);

	// NablaV = V'*(U'*U) - A'*U
	TFltVV GradV; GetGradientV(A, U, V, GradV);

	// frobenious norm of the projected gradient
	double frobNorm = 0.0;

	for (int j = 0; j < R; j++) {
		// construct the projected gradient for matrix U
		for (int i = 0; i < Rows; i++) {
			double ProjU_ij = (double)U.At(i, j) > 0.0 ? double(GradU.At(i, j)) : TMath::Mn(0.0, (double)GradU.At(i, j));
			frobNorm += ProjU_ij * ProjU_ij;
		}
		// construct the projected gradient for matrix V
		for (int i = 0; i < Cols; i++) {
			double ProjV_ij = (double)V.At(j, i) > 0.0 ? double(GradV.At(j, i)) : TMath::Mn(0.0, (double)GradV.At(j, i));
			frobNorm += ProjV_ij * ProjV_ij;
		}
	}
	return sqrt(frobNorm);
}

template <class TMatType>
double TNmf::WeightedProjectedGradientNorm(const TMatType& A, const int& R, 
	const TMatType& W, const TFltVV& U, const TFltVV& V) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// GradU = (W  o U*V)*V' - A*V'  
	TFltVV GradU; GetWeightedGradientU(A, W, U, V, GradU);

	// NablaV = U'*(W o U*V) - U'*A
	TFltVV GradV; GetWeightedGradientV(A, W, U, V, GradV);

	// frobenious norm of the projected gradient
	double frobNorm = 0.0;

	for (int j = 0; j < R; j++) {
		// construct the projected gradient for matrix U
		for (int i = 0; i < Rows; i++) {
			double ProjU_ij = (double)U.At(i, j) > 0.0 ? double(GradU.At(i, j)) : TMath::Mn(0.0, (double)GradU.At(i, j));
			frobNorm += ProjU_ij * ProjU_ij;
		}
		// construct the projected gradient for matrix V
		for (int i = 0; i < Cols; i++) {
			double ProjV_ij = (double)V.At(j, i) > 0.0 ? double(GradV.At(j, i)) : TMath::Mn(0.0, (double)GradV.At(j, i));
			frobNorm += ProjV_ij * ProjV_ij;
		}
	}
	return sqrt(frobNorm);
}

template <class TMatType>
void TNmf::UpdateU(const TMatType& A, TFltVV& U, const TFltVV& V, double& L1, const double& beta, const double& FrobA) {
	// NormFX = || A - U*V ||_{F}^{2}
	TFltVV FX;  TLinAlg::Multiply(U, V, FX);
	TLinAlg::LinComb(-1, FX, 1, A, FX);
	double NormFX = TLinAlg::Frob2(FX);
	
	// GradU = U*V*V' - A*V';
	TFltVV GradU; GetGradientU(A, U, V, GradU);

	// update matrix U = [U - 1/L1 * GradU]+
	const TFltVV OldU = U; 
	TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
	TLinAlg::NonNegProj(U);

	// prepare the condition variables
	TFltVV UU; TLinAlg::MultiplyT(U, U, UU);
	TFltVV UOldU; TLinAlg::LinComb(1, U, -1, OldU, UOldU);

	TFltVV VT; TLinAlg::Transpose(V, VT);
	TFltVV VV; TLinAlg::Multiply(V, VT, VV);
	TFltVV AV; TLinAlg::Multiply(A, VT, AV);

	// 1/2 * (Frob(A)^2 - 2 * <U, A*V'> + <U'*U, V*V'> - NormFX) > (<GradU, U - OldU> + L1 / 2 * Frob(U - OldU)^2)
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(U, AV) + TLinAlg::DotProduct(UU, VV) - NormFX) > 
		TLinAlg::DotProduct(GradU, UOldU) + L1 / 2.0 * TLinAlg::Frob2(UOldU)) {
		// update the matrix U
		L1 = L1 * beta;
		TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
		TLinAlg::NonNegProj(U);

		// update the matrices for the condition
		TLinAlg::MultiplyT(U, U, UU);
		TLinAlg::LinComb(1, U, -1, OldU, UOldU);
	}
}

template <class TMatType>
void TNmf::UpdateWeightedU(const TMatType& A,  TFltVV& U, const TFltVV& V, const TMatType& W, double& L1, 
	const double& beta, const double& FrobA) {
	// NormFX = || A - U*V ||_{W}^{2}
	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	TFltVV FX; TLinAlg::HadamardProd(W, UV, FX);
	TLinAlg::LinComb(-1, FX, 1, A, FX);
	double NormFX = TLinAlg::Frob2(FX);

	// GradU = (W o U*V)*V' - A*V';
	TFltVV GradU; GetWeightedGradientU(A, W, U, V, GradU);

	// update matrix U = [U - 1/L1 * GradU]+
	const TFltVV OldU = U;
	TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
	TLinAlg::NonNegProj(U);

	// prepare the condition variables
	TFltVV UOldU; TLinAlg::LinComb(1, U, -1, OldU, UOldU);
	TFltVV WUV; TLinAlg::HadamardProd(W, UV, WUV);

	// 1/2 * (Frob(A)^2 - 2 * <A,W o U*V> + <U*V, W o U*V> - NormFX) > (<GradU, U - OldU> + L1 / 2 * Frob(U - OldU)^2)
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(A, WUV) + TLinAlg::DotProduct(UV, WUV) - NormFX) > 
		TLinAlg::DotProduct(GradU, UOldU) + L1 / 2.0 * TLinAlg::Frob2(UOldU)) {
		// update the matrix U
		L1 = L1 * beta;
		TLinAlg::LinComb(1, OldU, -1 / L1, GradU, U);
		TLinAlg::NonNegProj(U);

		// update the matrices for the condition
		TLinAlg::Multiply(U, V, UV);
		TLinAlg::HadamardProd(W, UV, WUV);
		TLinAlg::LinComb(1, U, -1, OldU, UOldU);
	}
}

template <class TMatType>
void TNmf::UpdateV(const TMatType& A, const TFltVV& U, TFltVV& V, double& L2, const double& beta, const double& FrobA) {
	// NormFY = || A - U*V ||_{F}^{2}
	TFltVV FY; TLinAlg::Multiply(U, V, FY);
	TLinAlg::LinComb(-1, FY, 1, A, FY);
	double NormFY = TLinAlg::Frob2(FY);

	// GradV = U'*U*V - U'*A
	TFltVV GradV; GetGradientV(A, U, V, GradV);

	// update matrix V = [V - 1/L2 * GradV]+
	const TFltVV OldV = V;
	TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
	TLinAlg::NonNegProj(V);

	// prepare the condition variables
	TFltVV VT; TLinAlg::Transpose(V, VT);
	TFltVV VV; TLinAlg::Multiply(V, VT, VV);

	TFltVV UA; TLinAlg::MultiplyT(U, A, UA);
	TFltVV UU; TLinAlg::MultiplyT(U, U, UU);

	TFltVV VoldV; TLinAlg::LinComb(1, V, -1, OldV, VoldV);
	// 1/2 * (Frob(A)^2 - 2 * <U'*A, V> + <V*V', U'*U> - NormFY) > <GradV, V - OldV> + L2 / 2 * Frob(V - OldV)^2
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(UA, V) + TLinAlg::DotProduct(VV, UU) - NormFY) > 
		TLinAlg::DotProduct(GradV, VoldV) + L2 / 2.0 * TLinAlg::Frob2(VoldV)) {
		// update the matrix V
		L2 = L2 * beta;
		TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
		TLinAlg::NonNegProj(V);

		// update the matrices for the condition
		TLinAlg::Transpose(V, VT);
		TLinAlg::Multiply(V, VT, VV);
		TLinAlg::LinComb(1, V, -1, OldV, VoldV);
	}
}

template <class TMatType>
void TNmf::UpdateWeightedV(const TMatType& A, const TFltVV& U, TFltVV& V, const TMatType& W, double& L2, 
	const double& beta, const double& FrobA) {
	// NormFY = || A - U*V ||_{F}^{2}
	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	TFltVV FY; TLinAlg::HadamardProd(W, UV, FY);
	TLinAlg::LinComb(-1, FY, 1, A, FY);
	double NormFY = TLinAlg::Frob2(FY);

	// GradV = U'*(W o U*V) - U'*(W o A)
	TFltVV GradV; GetWeightedGradientV(A, W, U, V, GradV);

	// update matrix V = [V - 1/L2 * GradV]+
	const TFltVV OldV = V;
	TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
	TLinAlg::NonNegProj(V);

	// prepare the condition variables
	TFltVV WUV; TLinAlg::HadamardProd(W, UV, WUV);
	TFltVV VoldV; TLinAlg::LinComb(1, V, -1, OldV, VoldV);

	// 1/2 * (Frob(A)^2 - 2 * <A, W o U*V> + <W o U*V, W o U*V> - NormFY) > <GradV, V - OldV> + L2 / 2 * Frob(V - OldV)^2
	while (1.0 / 2 * (FrobA - 2.0 * TLinAlg::DotProduct(A, WUV) + TLinAlg::DotProduct(UV, WUV) - NormFY) > 
		TLinAlg::DotProduct(GradV, VoldV) + L2 / 2.0 * TLinAlg::Frob2(VoldV)) {
		// update the matrix V
		L2 = L2 * beta;
		TLinAlg::LinComb(1, OldV, -1 / L2, GradV, V);
		TLinAlg::NonNegProj(V);

		// update the matrices for the condition
		TLinAlg::Multiply(U, V, UV);
		TLinAlg::HadamardProd(W, UV, WUV);
		TLinAlg::LinComb(1, V, -1, OldV, VoldV);
	}
}

template <class TMatType>
void TNmf::GetGradientU(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& GradU) {
	// GradU = U*V*V' - A*V';
	TFltVV VT;  TLinAlg::Transpose(V, VT);
	TFltVV VV;  TLinAlg::Multiply(V, VT, VV);
	TFltVV UVV; TLinAlg::Multiply(U, VV, UVV);
	TFltVV AV;  TLinAlg::Multiply(A, VT, AV);
	TLinAlg::LinComb(1, UVV, -1, AV, GradU);
}

template <class TMatType>
void TNmf::GetWeightedGradientU(const TMatType& A, const TMatType& W, 
	const TFltVV& U, const TFltVV& V, TFltVV& GradU) {
	// GradU = (W o U*V)*V' - A*V';
	TFltVV VT; TLinAlg::Transpose(V, VT);
	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	TFltVV WUV; TLinAlg::HadamardProd(W, UV, WUV);
	TFltVV UVV; TLinAlg::Multiply(UV, VT, UVV);
	TFltVV AV; TLinAlg::Multiply(A, VT, AV);
	TLinAlg::LinComb(1, UVV, -1, AV, GradU);
	TLinAlg::LinComb(1, GradU, 0.01, U, GradU);
}

template <class TMatType>
void TNmf::GetGradientV(const TMatType& A, const TFltVV& U, const TFltVV& V, TFltVV& GradV) {
	// GradV = U'*U*V - U'*A
	TFltVV UU;  TLinAlg::MultiplyT(U, U, UU);
	TFltVV UUV; TLinAlg::Multiply(UU, V, UUV);
	TFltVV UA;  TLinAlg::MultiplyT(U, A, UA);
	TLinAlg::LinComb(1, UUV, -1, UA, GradV);
}

template <class TMatType>
void TNmf::GetWeightedGradientV(const TMatType& A, const TMatType& W, 
	const TFltVV& U, const TFltVV& V, TFltVV& GradV) {
	// GradV = U'*(W o U*V) - U'*A
	TFltVV UV; TLinAlg::Multiply(U, V, UV);
	TFltVV WUV; TLinAlg::HadamardProd(W, UV, WUV);
	TFltVV UUV; TLinAlg::MultiplyT(U, WUV, UUV);
	TFltVV UA; TLinAlg::MultiplyT(U, A, UA);
	TLinAlg::LinComb(1, UUV, -1, UA, GradV);
	TLinAlg::LinComb(1, GradV, 0.01, V, GradV);
}

#endif

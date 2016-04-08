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
	// calculates the NMF using Rank-one Residue Iteration
	template <class TMatType>
	static void RankOneResidueIter(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter = 10000,
		const double& Eps = 1e-3, const PNotify& Notify = TNotify::NullNotify);

private:
	// scaling initial matrices
	template <class TMatType>
	static void InitScaling(const TMatType& A, TFltVV& U, TFltVV& V);

	// calculates the starting condition
	template <class TMatType>
	static double StoppingCondition(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const double& Eps);

	// calculates the condition value
	template <class TMatType>
	static double ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V);

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
void TNmf::RankOneResidueIter(const TMatType& A, const int& R, TFltVV& U, TFltVV& V, const int& MaxIter,
	const double& Eps, const PNotify& Notify) {

	int Rows = NumOfRows(A);
	int Cols = NumOfCols(A);

	EAssert(0 < R && R <= Rows && R <= Cols);
	Notify->OnNotify(TNotifyType::ntInfo, "Executing NMF ...");

	// initialize the matrices U and V
	U.Gen(Rows, R); TLAMisc::FillRnd(U);
	V.Gen(R, Cols); TLAMisc::FillRnd(V);

	// scale the matrices U and V for a better starting point:
	InitScaling(A, U, V);
	// calculate the stopping condition
	double StopCond = StoppingCondition(A, R, U, V, Eps);
	int IterN = 0;

	do {
		if (IterN % 100 == 0) { Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d", IterN); }
		// update every column of U and every row of V
		for (int t = 0; t < R; t++) {
			// update U(:, t)
			TFltV Ut; U.GetCol(t, Ut);
			TFltV NewVt; NewVt.Gen(Cols);
			// update V(t, :)
			TFltV Vt; V.GetRow(t, Vt);
			TFltV NewUt; NewUt.Gen(Rows);
			for (int i = 0; i < R; i++) {
				if (i != t) {
					TFltV Ui; U.GetCol(i, Ui);
					TFltV Vi; V.GetRow(i, Vi);
					double UiUt = TLinAlg::DotProduct(Ui, Ut);
					double ViVt = TLinAlg::DotProduct(Vi, Vt);
					TLinAlg::LinComb(1, NewVt, UiUt, Vi, NewVt);
					TLinAlg::LinComb(1, NewUt, ViVt, Ui, NewUt);
				}
			}
			// R = A - UV
			TFltV Au; TLinAlg::MultiplyT(A, Ut, Au);
			TFltV Av; TLinAlg::Multiply(A, Vt, Av);
			TLinAlg::LinComb(1, Au, -1, NewVt, NewVt);
			TLinAlg::LinComb(1, Av, -1, NewUt, NewUt);

			bool IsRuZero = TLAMisc::IsZero(NewVt);
			// Replace V(t, :) with a zero  vector
			if (IsRuZero) {
				TFltV NewV; NewV.Gen(Cols);
				V.SetRow(t, NewV);
			}
			// V(t, :) = 1/Norm(U(:, t))^2 * NewVt
			else {
				double NormU = TLinAlg::Norm2(Ut);
				TLinAlg::MultiplyScalar(1 / NormU, NewVt);
				V.SetRow(t, NewVt);
			}

			bool IsRvZero = TLAMisc::IsZero(NewUt);
			// replace U(:, t) with a zero vector
			if (IsRvZero) {
				TFltV NewU; NewU.Gen(Rows);
				U.SetCol(t, NewU);
			}
			// replace U(:, t) = 1/Norm(V(t, :))^2 * NewUt
			else {
				double NormV = TLinAlg::Norm2(Vt);
				TLinAlg::MultiplyScalar(1 / NormV, NewUt);
				U.SetCol(t, NewUt);
			}
		}
		// check for stopping condition
		double Condition = ProjectedGradientNorm(A, R, U, V);
		if (Condition < StopCond) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Converged at iteration: %d", IterN);
			break;
		}
	} while (++IterN < MaxIter);
}

template <class TMatType>
void TNmf::InitScaling(const TMatType& A, TFltVV& U, TFltVV& V) {
	// alpha = trace(A'*U*V') / trace(V*U'*U*V')
	TFltVV UV;   TLinAlg::Multiply(U, V, UV);
	TFltVV AUV;  TLinAlg::MultiplyT(A, UV, AUV);
	TFltVV UVUV; TLinAlg::MultiplyT(UV, UV, UVUV);
	double alpha = TLAMisc::Trace(AUV) / TLAMisc::Trace(UVUV);
	// U = sqrt(alpha) * U
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), U, U);
	// V = sqrt(alpha) * V
	TLinAlg::MultiplyScalar(TMath::Sqrt(alpha), V, V);
}

template <class TMatType>
static double TNmf::StoppingCondition(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V, const double& Eps) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// NablaU = U*(V*V') - A*V' 
	TFltVV NablaU, VT, VV, UVV, AV;

	TLinAlg::Transpose(V, VT);
	TLinAlg::Multiply(V, VT, VV);
	TLinAlg::Multiply(U, VV, UVV);
	TLinAlg::Multiply(A, VT, AV);
	TLinAlg::LinComb(1, UVV, -1, AV, NablaU);

	// NablaV = V'*(U'*U) - A'*U
	TFltVV NablaV, AU, UU, VUU;
	TLinAlg::MultiplyT(U, U, UU);
	TLinAlg::MultiplyT(V, UU, VUU);
	TLinAlg::MultiplyT(A, U, AU);
	TLinAlg::LinComb(1, VUU, -1, AU, NablaV);

	double NormNablaU = TLinAlg::Frob(NablaU);
	double NormNablaV = TLinAlg::Frob(NablaV);

	return Eps * (NormNablaU + NormNablaV);
}

template <class TMatType>
static double TNmf::ProjectedGradientNorm(const TMatType& A, const int& R, const TFltVV& U, const TFltVV& V) {
	const int Rows = NumOfRows(A);
	const int Cols = NumOfCols(A);

	// NablaU = U*(V*V') - A*V'  
	TFltVV NablaU, VT, VV, UVV, AV;

	TLinAlg::Transpose(V, VT);
	TLinAlg::Multiply(V, VT, VV);
	TLinAlg::Multiply(U, VV, UVV);
	TLinAlg::Multiply(A, VT, AV);
	TLinAlg::LinComb(1, UVV, -1, AV, NablaU);

	// NablaV = (U'*U)*V - U'*A
	TFltVV NablaV, UA, UU, UUV;

	TLinAlg::MultiplyT(U, U, UU);
	TLinAlg::Multiply(UU, V, UUV);
	TLinAlg::MultiplyT(U, A, UA);
	TLinAlg::LinComb(1, UUV, -1, UA, NablaV);
	// projected gradient for matrix U and V 
	TFltVV ProjU(Rows, R), ProjV(R, Cols);

	for (int j = 0; j < R; j++) {
		// construct the projected gradient for matrix U
		for (int i = 0; i < Rows; i++) {
			double ProjU_ij = U.At(i, j) > 0 ? NablaU.At(i, j) : TMath::Mn(0.0, (double)NablaU.At(i, j));
			ProjU.PutXY(i, j, ProjU_ij);
		}
		// construct the projected gradient for matrix V
		for (int i = 0; i < Cols; i++) {
			double ProjV_ji = V.At(j, i) > 0 ? NablaV.At(j, i) : TMath::Mn(0.0, (double)NablaV.At(j, i));
			ProjV.PutXY(j, i, ProjV_ji);
		}
	}

	double NormProjU = TLinAlg::Frob(ProjU);
	double NormProjV = TLinAlg::Frob(ProjV);

	return NormProjU + NormProjV;
}

#endif

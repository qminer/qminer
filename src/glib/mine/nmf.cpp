/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

///////////////////////////////////////////
// Non-negative matrix factorization

void TNmf::InitializeUV(const int& Rows, const int& Cols, const int& R, TFltVV& U, TFltVV& V) {
	U.Gen(Rows, R); TLinAlgTransform::FillRnd(U);
	V.Gen(R, Cols); TLinAlgTransform::FillRnd(V);
}

void TNmf::InitializeWeights(const TFltVV& A, TFltVV& W) {
	int Rows = A.GetRows();
	int Cols = A.GetCols();
	W.Gen(Rows, Cols);
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			W(RowN, ColN) = A(RowN, ColN) > 0.0 ? 1.0 : 0.0;
		}
	}
}

void TNmf::InitializeWeights(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& W) {
	int Cols = A.Len(); W.Gen(Cols);
	for (int ColN = 0; ColN < Cols; ColN++) {
		const TIntFltKdV& ColA = A[ColN]; 
		const int Els = ColA.Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			if (ColA[ElN].Dat > 0.0) {
				W[ColN].Add(TIntFltKd(ColA[ElN].Key, 1));
			}
		}
	}
}

void TNmf::UpdateScale(TFltVV& U, TFltVV& V) {
	int R = U.GetCols();
	for (int i = 0; i < R; i++) {
		TFltV Vi; V.GetRow(i, Vi);
		TFltV Ui; U.GetCol(i, Ui);
		// scale = sqrt(|| V(i, :) || / || U(:, i) ||)
		double scale = TMath::Sqrt(TLinAlg::Norm(Vi) / TLinAlg::Norm(Ui));
		// U(:, i) = scale * U(:, i)
		TLinAlg::MultiplyScalar(scale,   Ui); U.SetCol(i, Ui);
		// V(i, :) = 1/scale * V(i, :)
		TLinAlg::MultiplyScalar(1/scale, Vi); V.SetRow(i, Vi);
	}
}

int TNmf::NumOfRows(const TFltVV& Mat) { return Mat.GetRows(); }
int TNmf::NumOfRows(const TVec<TIntFltKdV>& Mat) { return TLinAlgSearch::GetMaxDimIdx(Mat) + 1; }

int TNmf::NumOfCols(const TFltVV& Mat) { return Mat.GetCols(); }
int TNmf::NumOfCols(const TVec<TIntFltKdV>& Mat) { return Mat.Len(); }
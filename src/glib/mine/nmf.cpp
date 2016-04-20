/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

///////////////////////////////////////////
// Non-negative matrix factorization

void TNmf::UpdateScaling(TFltVV& U, TFltVV& V) {
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
int TNmf::NumOfRows(const TVec<TIntFltKdV>& Mat) { return TLAMisc::GetMaxDimIdx(Mat) + 1; }

int TNmf::NumOfCols(const TFltVV& Mat) { return Mat.GetCols(); }
int TNmf::NumOfCols(const TVec<TIntFltKdV>& Mat) { return Mat.Len(); }

double TNmf::GetMatVal(const TFltVV& Mat, const int& RowId, const int& ColId) {
	return Mat(RowId, ColId);
}
double TNmf::GetMatVal(const TVec<TIntFltKdV>& Mat, const int& RowId, const int& ColId) {
	const TIntFltKdV& ColV = Mat[ColId]; 
	const int& Els = ColV.Len();
	
	double Val = 0.0;
	for (int ElN = 0; ElN < Els; ElN++) {
		if (ColV[ElN].Key == RowId) {
			Val = ColV[ElN].Dat;
			break;
		}
	}
	return Val;
}

void TNmf::GetMatCol(const TFltVV& Mat, const int& ColId, TFltV& Vec) {
	Mat.GetCol(ColId, Vec);
}
void TNmf::GetMatCol(const TVec<TIntFltKdV>& Mat, const int& ColId, TFltV& Vec) {
	const TIntFltKdV& SparseVec = Mat[ColId];
	const int Rows = NumOfRows(Mat);
	TLAMisc::ToVec(SparseVec, Vec, Rows);
}
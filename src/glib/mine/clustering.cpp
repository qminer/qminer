/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TDistance;

//////////////////////////////////////////////////////
// Distance measures
void TEuclDist::GetDist(const TFltVV& X, const TFltVV& Y, TFltVV& D) {
	GetDist2(X, Y, D);
	for (int RowN = 0; RowN < D.GetRows(); RowN++) {
		for (int ColN = 0; ColN < D.GetCols(); ColN++) {
			D.PutXY(RowN, ColN, TMath::Sqrt(D(RowN, ColN)));
		}
	}
}

void TEuclDist::GetDist2(const TFltVV& X, const TFltVV& Y, TFltVV& D) {
	TFltV NormX2;	TLinAlg::GetColNorm2V(X, NormX2);
	TFltV NormY2;	TLinAlg::GetColNorm2V(Y, NormY2);

	GetDist2(X, Y, NormX2, NormY2, D);
}

void TEuclDist::GetDist2(const TFltVV& X, const TFltVV& Y, const TFltV& NormX2,
		const TFltV& NormY2, TFltVV& D) {
	//	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
	// 1) X'Y
	TLinAlg::MultiplyT(X, Y, D);
	// 2) (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2)
	const int Rows = D.GetRows();
	const int Cols = D.GetCols();

	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			D.PutXY(RowN, ColN, NormX2[RowN] - 2*D(RowN, ColN) + NormY2[ColN]);
		}
	}
}

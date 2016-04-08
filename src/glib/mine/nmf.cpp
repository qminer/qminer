/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

///////////////////////////////////////////
// Non-negative matrix factorization

int TNmf::NumOfRows(const TFltVV& Mat) { return Mat.GetRows(); }
int TNmf::NumOfRows(const TVec<TIntFltKdV>& Mat) { return TLAMisc::GetMaxDimIdx(Mat); }

int TNmf::NumOfCols(const TFltVV& Mat) { return Mat.GetCols(); }
int TNmf::NumOfCols(const TVec<TIntFltKdV>& Mat) { return Mat.Len(); }
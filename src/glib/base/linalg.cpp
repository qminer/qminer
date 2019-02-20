/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "bd.h"
//#include "base.h"
#ifdef LAPACKE
#include "MKLfunctions.h"
#endif

#ifdef EIGEN
#include <Eigen/Dense>
#include <Eigen/Sparse>
#endif

///////////////////////////////////////////////////////////////////////
// Sparse-Column-Matrix
void TSparseColMatrix::PMultiply(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= ColN && Result.Len() >= RowN);
    int i, j; TFlt *ResV = Result.BegI();
    for (i = 0; i < RowN; i++) ResV[i] = 0.0;
    for (j = 0; j < ColN; j++) {
        const TIntFltKdV& ColV = ColSpVV[j]; int len = ColV.Len();
        for (i = 0; i < len; i++) {
            ResV[ColV[i].Key] += ColV[i].Dat * B(j,ColId);
        }
    }
}

void TSparseColMatrix::PMultiply(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= ColN && Result.Len() >= RowN);
    int i, j; TFlt *ResV = Result.BegI();
    for (i = 0; i < RowN; i++) ResV[i] = 0.0;
    for (j = 0; j < ColN; j++) {
        const TIntFltKdV& ColV = ColSpVV[j]; int len = ColV.Len();
        for (i = 0; i < len; i++) {
            ResV[ColV[i].Key] += ColV[i].Dat * Vec[j];
        }
    }
}

void TSparseColMatrix::PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= RowN && Result.Len() >= ColN);
    int i, j, len; TFlt *ResV = Result.BegI();
    for (j = 0; j < ColN; j++) {
        const TIntFltKdV& ColV = ColSpVV[j];
        len = ColV.Len(); ResV[j] = 0.0;
        for (i = 0; i < len; i++) {
            ResV[j] += ColV[i].Dat * B(ColV[i].Key, ColId);
        }
    }
}

void TSparseColMatrix::PMultiplyT(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= RowN && Result.Len() >= ColN);
    int i, j, len; TFlt *VecV = Vec.BegI(), *ResV = Result.BegI();
    for (j = 0; j < ColN; j++) {
        const TIntFltKdV& ColV = ColSpVV[j];
        len = ColV.Len(); ResV[j] = 0.0;
        for (i = 0; i < len; i++) {
            ResV[j] += ColV[i].Dat * VecV[ColV[i].Key];
        }
    }
}

void TSparseColMatrix::PMultiply(const TFltVV& B, TFltVV& Result) const {
	TLinAlg::Multiply(ColSpVV, B, Result, RowN);
}

void TSparseColMatrix::PMultiplyT(const TFltVV& B, TFltVV& Result) const {
	TLinAlg::MultiplyT(ColSpVV, B, Result);
}

void TSparseColMatrix::Init() {
    ColN = ColSpVV.Len();
    for (int Col = 0; Col < ColN; Col++) {
        if (ColSpVV[Col].Empty()) { continue; }
        if (ColSpVV[Col].Last().Key >= RowN) {
            RowN = ColSpVV[Col].Last().Key + 1;
        }
    }
}

///////////////////////////////////////////////////////////////////////
// Sparse-Row-Matrix
TSparseRowMatrix::TSparseRowMatrix(const TStr& MatlabMatrixFNm) {
   FILE *F = fopen(MatlabMatrixFNm.CStr(), "rt");  EAssert(F != NULL);
   TVec<TTriple<TInt, TInt, TSFlt> > MtxV;
   RowN = 0;  ColN = 0;
   while (! feof(F)) {
     int row=-1, col=-1; float val;
     if (fscanf(F, "%d %d %f\n", &row, &col, &val) == 3) {
       EAssert(row > 0 && col > 0);
       MtxV.Add(TTriple<TInt, TInt, TSFlt>(row, col, val));
       RowN = TMath::Mx(RowN.Val, row);
       ColN = TMath::Mx(ColN.Val, col);
     }
   }
   fclose(F);
   // create matrix
   MtxV.Sort();
   RowSpVV.Gen(RowN);
   int cnt = 0;
   for (int row = 1; row <= RowN; row++) {
     while (cnt < MtxV.Len() && MtxV[cnt].Val1 == row) {
       RowSpVV[row-1].Add(TIntFltKd(MtxV[cnt].Val2-1, MtxV[cnt].Val3()));
       cnt++;
     }
   }
}

void TSparseRowMatrix::PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= RowN && Result.Len() >= ColN);
    for (int i = 0; i < ColN; i++) Result[i] = 0.0;
    for (int j = 0; j < RowN; j++) {
        const TIntFltKdV& RowV = RowSpVV[j]; int len = RowV.Len();
        for (int i = 0; i < len; i++) {
            Result[RowV[i].Key] += RowV[i].Dat * B(j,ColId);
        }
    }
}

void TSparseRowMatrix::PMultiplyT(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= RowN && Result.Len() >= ColN);
    for (int i = 0; i < ColN; i++) Result[i] = 0.0;
    for (int j = 0; j < RowN; j++) {
        const TIntFltKdV& RowV = RowSpVV[j]; int len = RowV.Len();
        for (int i = 0; i < len; i++) {
            Result[RowV[i].Key] += RowV[i].Dat * Vec[j];
        }
    }
}

void TSparseRowMatrix::PMultiply(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= ColN && Result.Len() >= RowN);
    for (int j = 0; j < RowN; j++) {
        const TIntFltKdV& RowV = RowSpVV[j];
        int len = RowV.Len(); Result[j] = 0.0;
        for (int i = 0; i < len; i++) {
            Result[j] += RowV[i].Dat * B(RowV[i].Key, ColId);
        }
    }
}

void TSparseRowMatrix::PMultiply(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= ColN && Result.Len() >= RowN);
    for (int j = 0; j < RowN; j++) {
        const TIntFltKdV& RowV = RowSpVV[j];
        int len = RowV.Len(); Result[j] = 0.0;
        for (int i = 0; i < len; i++) {
            Result[j] += RowV[i].Dat * Vec[RowV[i].Key];
        }
    }
}

void TSparseRowMatrix::Init() {
    RowN = RowSpVV.Len();
    for (int Row = 0; Row < RowN; Row++) {
        if (RowSpVV[Row].Empty()) { continue; }
        if (RowSpVV[Row].Last().Key >= ColN) {
            ColN = RowSpVV[Row].Last().Key + 1;
        }
    }
}

///////////////////////////////////////////////////////////////////////
// Full-Col-Matrix
TFullColMatrix::TFullColMatrix(const TStr& MatlabMatrixFNm): TMatrix() {
    TLinAlgIO::LoadMatlabTFltVV(MatlabMatrixFNm, ColV);
    RowN=ColV[0].Len(); ColN=ColV.Len();
    for (int i = 0; i < ColN; i++) {
        EAssertR(ColV[i].Len() == RowN, TStr::Fmt("%d != %d", ColV[i].Len(), RowN));
    }
}

TFullColMatrix::TFullColMatrix(TVec<TFltV>& FullM): TMatrix(), ColV(FullM) {
	 RowN=FullM.Len(); ColN=FullM[0].Len();
}

void TFullColMatrix::PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= RowN && Result.Len() >= ColN);
    for (int i = 0; i < ColN; i++) {
        Result[i] = TLinAlg::DotProduct(B, ColId, ColV[i]);
    }
}

void TFullColMatrix::PMultiplyT(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= RowN && Result.Len() >= ColN);
    for (int i = 0; i < ColN; i++) {
        Result[i] = TLinAlg::DotProduct(Vec, ColV[i]);
    }
}

void TFullColMatrix::PMultiply(const TFltVV& B, int ColId, TFltV& Result) const {
    EAssert(B.GetRows() >= ColN && Result.Len() >= RowN);
    for (int i = 0; i < RowN; i++) { Result[i] = 0.0; }
    for (int i = 0; i < ColN; i++) {
        TLinAlg::AddVec(B(i, ColId), ColV[i], Result, Result);
    }
}

void TFullColMatrix::PMultiply(const TFltV& Vec, TFltV& Result) const {
    EAssert(Vec.Len() >= ColN && Result.Len() >= RowN);
    for (int i = 0; i < RowN; i++) { Result[i] = 0.0; }
    for (int i = 0; i < ColN; i++) {
        TLinAlg::AddVec(Vec[i], ColV[i], Result, Result);
    }
}

///////////////////////////////////////////////////////////////////////
// Structured-Covariance-Matrix
void TStructuredCovarianceMatrix::PMultiply(const TFltVV& B, int ColId, TFltV& Result) const {FailR("Not implemented yet");} // TODO

void TStructuredCovarianceMatrix::PMultiply(const TFltVV& B, TFltVV& Result) const {
	// 1/Samples * (X - MeanX*ones(1,Samples)) (Y - MeanY*(ones(1,Samples))' B
	// 1/ Samples X (Y' B) - MeanX (MeanY' B)
	EAssert(Result.GetRows() == XRows && Result.GetCols() == B.GetCols());
	int BCols = B.GetCols();
	TFltVV YtB(Samples, BCols);;
	TLinAlg::MultiplyT(Y, B, YtB);
	TLinAlg::Multiply(X, YtB, Result); YtB.Clr();

	TFltV MeanYtB(BCols); // MeanY' B the same TFltV as  B' MeanY	
	TLinAlg::MultiplyT(B, MeanY, MeanYtB);
	// Result := 1/SampleN Result - MeanX MeanY' B	
	for (int RowN = 0; RowN < XRows; RowN++) {
		for (int ColN = 0; ColN < BCols; ColN++) {
			Result.At(RowN, ColN) = 1.0/Samples * Result.At(RowN, ColN) - MeanX[RowN]*MeanYtB[ColN];
		}
	}
}; 

void TStructuredCovarianceMatrix::PMultiply(const TFltV& Vec, TFltV& Result) const {FailR("Not implemented yet");} // TODO

void TStructuredCovarianceMatrix::PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {FailR("Not implemented yet");} // TODO

void TStructuredCovarianceMatrix::PMultiplyT(const TFltVV& B, TFltVV& Result) const {
	// 1/Samples * (Y - MeanY*ones(1,Samples)) (X - MeanX*(ones(1,Samples))' B
	// 1/ Samples Y (X' B) - MeanY (MeanX' B)
	EAssert(Result.GetRows() == YRows && Result.GetCols() == B.GetCols());
	int BCols = B.GetCols();
	TFltVV XtB(Samples, BCols);
	TLinAlg::MultiplyT(X, B, XtB);
	TLinAlg::Multiply(Y, XtB, Result); XtB.Clr();

	TFltV MeanXtB(BCols); // MeanX' B the same TFltV as  B' MeanX
	TLinAlg::MultiplyT(B, MeanX, MeanXtB);
	// Result := 1/SampleN Result - MeanY MeanX' B	
	for (int RowN = 0; RowN < YRows; RowN++) {
		for (int ColN = 0; ColN < BCols; ColN++) {
			Result.At(RowN, ColN) = 1.0/Samples * Result.At(RowN, ColN) - MeanY[RowN]*MeanXtB[ColN];
		}
	}
};

void TStructuredCovarianceMatrix::PMultiplyT(const TFltV& Vec, TFltV& Result) const {
    FailR("Not implemented yet"); // TODO
}

//////////////////////////////////////////////////////////////////////
// Linear algebra input/output operations
void TLinAlgIO::SaveCsvTFltV(const TFltV& Vec, TSOut& SOut) {
    for (int ValN = 0; ValN < Vec.Len(); ValN++) {
        SOut.PutFlt(Vec[ValN]); SOut.PutCh(',');
    }
    SOut.PutLn();
}

void TLinAlgIO::SaveMatlabTFltIntKdV(const TIntFltKdV& SpV, const int& ColN, TSOut& SOut) {
    const int Len = SpV.Len();
    for (int ValN = 0; ValN < Len; ValN++) {
        SOut.PutStrLn(TStr::Fmt("%d %d %g", SpV[ValN].Key+1, ColN+1, SpV[ValN].Dat()));
    }
}

void TLinAlgIO::SaveMatlabSpMat(const TVec<TIntFltKdV>& SpMat, TSOut& SOut) {
	int Cols = SpMat.Len();
	for (int ColN = 0; ColN < Cols; ColN++) {
		int Els = SpMat[ColN].Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			SOut.PutStrLn(SpMat[ColN][ElN].Key.GetStr() + " " + TInt::GetStr(ColN) + " " + TStr::Fmt("%.17g", SpMat[ColN][ElN].Dat.Val));
		}
	}
	SOut.Flush();
}


void TLinAlgIO::SaveMatlabSpMat(const TTriple<TIntV, TIntV,TFltV>& SpMat, TSOut& SOut) {
	int Len = SpMat.Val1.Len();
	for (int ElN = 0; ElN < Len; ElN++) {
		SOut.PutStrLn(SpMat.Val1[ElN].GetStr() + " " + SpMat.Val2[ElN].GetStr() + " " + SpMat.Val3[ElN].GetStr());
	}
	SOut.Flush();
}

void TLinAlgIO::SaveMatlabTFltV(const TFltV& m, const TStr& FName) {
    PSOut out = TFOut::New(FName);
    const int RowN = m.Len();
    for (int RowId = 0; RowId < RowN; RowId++) {
        out->PutStr(TFlt::GetStr(m[RowId], 20, 18));
        out->PutCh('\n');
    }
    out->Flush();
}

void TLinAlgIO::SaveMatlabTIntV(const TIntV& m, const TStr& FName) {
    PSOut out = TFOut::New(FName);
    const int RowN = m.Len();
    for (int RowId = 0; RowId < RowN; RowId++) {
        out->PutInt(m[RowId]);
        out->PutCh('\n');
    }
    out->Flush();
}

void TLinAlgIO::SaveMatlabTFltVVCol(const TFltVV& m, int ColId, const TStr& FName) {
    PSOut out = TFOut::New(FName);
    const int RowN = m.GetRows();
    for (int RowId = 0; RowId < RowN; RowId++) {
        out->PutStr(TFlt::GetStr(m(RowId,ColId), 20, 18));
        out->PutCh('\n');
    }
    out->Flush();
}


void TLinAlgIO::SaveMatlabTFltVV(const TFltVV& m, const TStr& FName) {
    PSOut out = TFOut::New(FName);
    TLinAlgIO::SaveMatlabTFltVV(m, *out);
}

void TLinAlgIO::SaveMatlabTFltVV(const TFltVV& m, TSOut& SOut) {
	const int RowN = m.GetRows();
	const int ColN = m.GetCols();
	for (int RowId = 0; RowId < RowN; RowId++) {
		for (int ColId = 0; ColId < ColN; ColId++) {
			SOut.PutStr(TFlt::GetStr(m(RowId, ColId), 20, 18));
			SOut.PutCh(' ');
		}
		SOut.PutCh('\n');
	}
	SOut.Flush();
}

void TLinAlgIO::SaveMatlabTFltVVMjrSubMtrx(const TFltVV& m,
        int RowN, int ColN, const TStr& FName) {

    PSOut out = TFOut::New(FName);
    for (int RowId = 0; RowId < RowN; RowId++) {
        for (int ColId = 0; ColId < ColN; ColId++) {
            out->PutStr(TFlt::GetStr(m(RowId,ColId), 20, 18)); out->PutCh(' ');
        }
        out->PutCh('\n');
    }
    out->Flush();
}

void TLinAlgIO::LoadMatlabTFltVV(const TStr& FNm, TVec<TFltV>& ColV) {
    PSIn SIn = TFIn::New(FNm);
    TLinAlgIO::LoadMatlabTFltVV(ColV, *SIn);
}

void TLinAlgIO::LoadMatlabTFltVV(const TStr& FNm, TFltVV& MatrixVV) {
	PSIn SIn = TFIn::New(FNm);
	TLinAlgIO::LoadMatlabTFltVV(MatrixVV, *SIn);
}

void TLinAlgIO::LoadMatlabTFltVV(TFltVV& MatrixVV, TSIn& SIn) {
	TVec<TFltV> ColV; LoadMatlabTFltVV(ColV, SIn);
	if (ColV.Empty()) { MatrixVV.Clr(); return; }
	const int Rows = ColV[0].Len(), Cols = ColV.Len();
	MatrixVV.Gen(Rows, Cols);
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			MatrixVV(RowN, ColN) = ColV[ColN][RowN];
		}
	}
}

void TLinAlgIO::LoadMatlabTFltVV(TVec<TFltV>& ColV, TSIn& SIn) {
	TILx Lx(&SIn, TFSet() | iloRetEoln | iloSigNum | iloExcept);
	int Row = 0, Col = 0; ColV.Clr();
	Lx.GetSym(syFlt, syEof, syEoln);
	//printf("%d x %d\r", Row, ColV.Len());
	while (Lx.Sym != syEof) {
		if (Lx.Sym == syFlt) {
			if (ColV.Len() > Col) {
				EAssert(ColV[Col].Len() == Row);
				ColV[Col].Add(Lx.Flt);
			}
			else {
				EAssert(Row == 0);
				ColV.Add(TFltV::GetV(Lx.Flt));
			}
			Col++;
		}
		else if (Lx.Sym == syEoln) {
			EAssert(Col == ColV.Len());
			Col = 0; Row++;
			if (Row % 100 == 0) {
				//printf("%d x %d\r", Row, ColV.Len());
			}
		}
		else {
			Fail;
		}
		Lx.GetSym(syFlt, syEof, syEoln);
	}
	//printf("\n");
	EAssert(Col == ColV.Len() || Col == 0);
}

void TLinAlgIO::PrintTFltV(const TFltV& Vec, const TStr& VecNm) {
    printf("%s = [", VecNm.CStr());
    for (int i = 0; i < Vec.Len(); i++) {
        printf("%.5f", Vec[i]());
		if (i < Vec.Len() - 1) { printf(", "); }
    }
    printf("]\n");
}

void TLinAlgIO::PrintTFltVVToStr(const TFltVV& A, TStr& Out) {
	Out = "";
	int Rows = A.GetRows();
	int Cols = A.GetCols();
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			Out += A.At(RowN,ColN).GetStr() + " ";
		}
		Out += "\n";
	}
    Out += "\n";

}

void TLinAlgIO::PrintTFltVV(const TFltVV& A, const TStr& MatrixNm) {
    printf("%s = [\n", MatrixNm.CStr());
	int Rows = A.GetRows();
	int Cols = A.GetCols();
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			printf("%f ", A.At(RowN, ColN).Val);
		}
		printf("\n");
	}
    printf("]\n");
}

void TLinAlgIO::PrintSpMat(const TTriple<TIntV, TIntV, TFltV>& A, const TStr& MatrixNm) {
	int Nonzeros = A.Val1.Len();
	printf("%s = [\n", MatrixNm.CStr());
	for (int ElN = 0; ElN < Nonzeros; ElN++) {
		printf("%d %d %f\n", A.Val1[ElN].Val, A.Val2[ElN].Val, A.Val3[ElN].Val);
	}
	printf("]\n");
}

void TLinAlgIO::PrintSpMat(const TVec<TIntFltKdV>& A, const TStr& MatrixNm) {
	printf("%s = [\n", MatrixNm.CStr());
	int Cols = A.Len();
	for (int ColN = 0; ColN < Cols; ColN++) {
		int Els = A[ColN].Len();
		for (int ElN = 0; ElN < Els; ElN++) {
			printf("%d %d %f\n", A[ColN][ElN].Key.Val, ColN, A[ColN][ElN].Dat.Val);
		}
	}
	printf("]\n");
}

void TLinAlgIO::PrintTIntV(const TIntV& Vec, const TStr& VecNm) {
    printf("%s = [", VecNm.CStr());
    for (int i = 0; i < Vec.Len(); i++) {
        printf("%d", Vec[i]());
        if (i < Vec.Len() - 1) printf(", ");
    }
    printf("]\n");
}

//////////////////////////////////////////////////////////////////////
// Statistics on linear algebra structures
double TLinAlgStat::Mean(const TFltV& Vec) {
    EAssertR(Vec.Len() != 0, "TLinAlgStat::Mean: Vector length should not be 0");
    return TLinAlg::SumVec(Vec) / Vec.Len();
}

void TLinAlgStat::Mean(const TFltVV& Mat, TFltV& Res, const TMatDim& Dim) {
    int Rows = Mat.GetRows();
    int Cols = Mat.GetCols();
    if (Dim == TMatDim::mdCols) {
        EAssertR(Rows != 0, TStr::Fmt("TLinAlgStat::Mean: Matrix number of rows should not be 0 "
            "when using mean with parameter 'Dim=%d'", Dim));
        if (Res.Len() != Cols) {
            Res.Gen(Cols);
        }
        TFltV Vec(Rows);
        Vec.PutAll(1.0 / Rows);
        TLinAlg::MultiplyT(Mat, Vec, Res);
    } else if (Dim == TMatDim::mdRows) {
        EAssertR(Cols != 0, TStr::Fmt("TLinAlgStat::Mean: Matrix number of columns should not be 0 "
            "when using mean with parameter 'Dim=%d'", Dim));
        if (Res.Len() != Rows) {
            Res.Gen(Rows);
        }
        TFltV Vec(Cols);
        Vec.PutAll(1.0 / Cols);
        TLinAlg::Multiply(Mat, Vec, Res);
    }
}

double TLinAlgStat::Std(const TFltV& Vec, const int& Flag) {
    EAssertR(Flag == 0 || Flag == 1, "TLinAlgStat::Std: Invalid value of 'Flag' argument. "
        "Supported 'Flag' arguments are 0 or 1. See Matlab std() documentation.");

    int Len = Vec.Len();
    double Mean = TLinAlgStat::Mean(Vec);

    double Scalar;
    if (Flag == 1) {
        Scalar = TMath::Sqrt(1.0 / (Len));
    }
    else {
        EAssertR(Len > 1, TStr::Fmt("TLinAlgStat::Std: Matrix number of rows should "
            "not be less than 2 when using mean with parameter 'Flag=%d'", Flag));
        Scalar = TMath::Sqrt(1.0 / (Len - 1));
    }

    TFltV TempRes(Len);
    TFltV Ones(Len);
    Ones.PutAll(1.0);

    TLinAlg::LinComb(-1, Vec, Mean, Ones, TempRes);
    return Scalar * TLinAlg::Norm(TempRes);
}

void TLinAlgStat::Std(const TFltVV& Mat, TFltV& Res, const int& Flag, const TMatDim& Dim) {
    EAssertR(Flag == 0 || Flag == 1, "TLinAlgStat::Std: Invalid value of 'Flag' argument. "
        "Supported 'Flag' arguments are 0 or 1. See Matlab std() documentation.");
    int Cols = Mat.GetCols();
    int Rows = Mat.GetRows();
    TFltV MeanVec;
    TLinAlgStat::Mean(Mat, MeanVec, Dim);
    EAssertR(Cols == MeanVec.Len() || Rows == MeanVec.Len(), "TLAMisc::Std");

    if (Dim == TMatDim::mdCols) {
        if(Res.Empty()) Res.Gen(Cols);
        EAssertR(Cols == Res.Len(), "TLinAlgStat::Std");

        double Scalar;
        if (Flag == 1) {
            Scalar = TMath::Sqrt(1.0 / (Rows));
        }
        else {
            EAssertR(Rows > 1, TStr::Fmt("TLinAlgStat::Std: Matrix number of rows should not be "
                "less than 2 when using mean with parameter 'Flag=%d' and 'Dim=%d'", Flag, Dim));
            Scalar = TMath::Sqrt(1.0 / (Rows - 1));
        }

        TFltV TempRes(Rows);
        TFltV Ones(Rows);
        Ones.PutAll(1.0);

        for (int ColN = 0; ColN < Cols; ColN++) {
            TLinAlg::LinComb(-1.0, Mat, ColN, MeanVec[ColN], Ones, TempRes);
            Res[ColN] = Scalar * TLinAlg::Norm(TempRes);
        }
    }
    else if (Dim == TMatDim::mdRows) {
        if(Res.Empty()) Res.Gen(Rows);
        EAssertR(Rows == Res.Len(), "TLinAlgStat::Std");

        double Scalar;
        if (Flag == 1) {
            Scalar = TMath::Sqrt(1.0 / (Cols));
        }
        else {
            EAssertR(Cols > 1, TStr::Fmt("TLinAlgStat::Std: Matrix number of columns should not be "
                "less than 2 when using mean with parameter 'Flag=%d' and 'Dim=%d'", Flag, Dim));
            Scalar = TMath::Sqrt(1.0 / (Cols - 1));
        }

        TFltV TempRes(Cols);
        TFltV Ones(Cols);
        Ones.PutAll(1.0);

        for (int RowN = 0; RowN < Rows; RowN++) {
            TLinAlg::LinComb(-1.0, Mat, RowN, MeanVec[RowN], Ones, TempRes, 2);
            Res[RowN] = Scalar * TLinAlg::Norm(TempRes);
        }
    }
}

void TLinAlgStat::ZScore(const TFltVV& Mat, TFltVV& Res, const int& Flag, const TMatDim& Dim) {
    EAssertR(Flag == 0 || Flag == 1, "TLinAlgStat::ZScore: Invalid value of 'Flag' argument. "
        "Supported 'Flag' arguments are 0 or 1. See Matlab std() documentation.");

    int Cols = Mat.GetCols();
    int Rows = Mat.GetRows();

    if (Res.Empty()) Res.Gen(Rows, Cols);

    TFltV MeanVec;
    TLinAlgStat::Mean(Mat, MeanVec, Dim);
    TFltV StdVec;
    TLinAlgStat::Std(Mat, StdVec, Flag, Dim);

    if (Dim == TMatDim::mdCols) {

        TFltV TempRes(Rows);
        TFltV Ones(Rows);
        Ones.PutAll(1.0);

        for (int ColN = 0; ColN < Cols; ColN++) {
            double Factor = StdVec[ColN] == 0.0 ? 1.0 : StdVec[ColN].Val;
            TLinAlg::LinComb(1.0/Factor, Mat, ColN, -1.0 * MeanVec[ColN]/Factor, Ones, TempRes);
            for (int RowN = 0; RowN < Rows; RowN++) {
                Res.At(RowN, ColN) = TempRes[RowN];
            }
        }
    }
    else if (Dim == TMatDim::mdRows) {

        TFltV TempRes(Cols);
        TFltV Ones(Cols);
        Ones.PutAll(1.0);

        for (int RowN = 0; RowN < Rows; RowN++) {
            double Factor = StdVec[RowN] == 0.0 ? 1.0 : StdVec[RowN].Val;
            TLinAlg::LinComb(1.0/Factor, Mat, RowN, -1.0 * MeanVec[RowN]/Factor, Ones, TempRes, 2);
            for (int ColN = 0; ColN < Cols; ColN++) {
                Res.At(RowN, ColN) = TempRes[ColN];
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////
/// Transformations of linear algebra structures
void TLinAlgTransform::Fill(TFltVV& A, const double& val) {
	const int Rows = A.GetRows();
	const int Cols = A.GetCols();
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			A.PutXY(RowN, ColN, val);
		}
	}
}

void TLinAlgTransform::Fill(TFltV& V, const double& val){
	const int n = V.Len();
	for (int i = 0; i < n; i++){
		V[i] = val;
	}
}

void TLinAlgTransform::FillRnd(const int& Len, TFltV& Vec, TRnd& Rnd) {
    Vec.Gen(Len);
    for(int i = 0; i < Len; i++) {
        Vec[i] = Rnd.GetUniDev();
    }
}

void TLinAlgTransform::FillIdentity(TFltVV& M) {
    EAssert(M.GetRows() == M.GetCols());
    int Len = M.GetRows();
    for (int i = 0; i < Len; i++) {
        for (int j = 0; j < Len; j++) M(i,j) = 0.0;
        M(i,i) = 1.0;
    }
}

void TLinAlgTransform::FillIdentity(TFltVV& M, const double& Elt) {
    EAssert(M.GetRows() == M.GetCols());
    int Len = M.GetRows();
    for (int i = 0; i < Len; i++) {
        for (int j = 0; j < Len; j++) M(i,j) = 0.0;
        M(i,i) = Elt;
    }
}

/*void TLinAlgTransform::FillRange(const int& Vals, TFltV& Vec) {
	//Added by Andrej
	if (Vec.Len() != Vals){
		Vec.Gen(Vals);
	}
	for(int i = 0; i < Vals; i++){
		Vec[i] = i;
	}
}
void TLinAlgTransform::FillRange(const int& Vals, TIntV& Vec) {
	//Added by Andrej
	if (Vec.Len() != Vals){
		Vec.Gen(Vals);
	}
	for (int i = 0; i < Vals; i++){
		Vec[i] = i;
	}
}*/

//void TLinAlgTransform::Diag(const TFltV& Vec, TFltVV& Mat) {
//	Mat.Gen(Vec.Len(), Vec.Len());
//	Mat.PutAll(0.0);
//	for (int ElN = 0; ElN < Vec.Len(); ElN++) {
//		Mat.At(ElN, ElN) = Vec[ElN];
//	}
//}

void TLinAlgTransform::ToSpVec(const TFltV& Vec, TIntFltKdV& SpVec,
        const double& CutSumPrc) {

    // determine minimal element value
    EAssert(0.0 <= CutSumPrc && CutSumPrc <= 1.0);
    const int Elts = Vec.Len();
    double EltSum = 0.0;
    for (int EltN = 0; EltN < Elts; EltN++) {
        EltSum += TFlt::Abs(Vec[EltN]); }
    const double MnEltVal = CutSumPrc * EltSum;
    // create sparse vector
    SpVec.Clr();
    for (int EltN = 0; EltN < Elts; EltN++) {
        if (TFlt::Abs(Vec[EltN]) > MnEltVal) {
            SpVec.Add(TIntFltKd(EltN, Vec[EltN]));
        }
    }
    SpVec.Pack();
}

void TLinAlgTransform::ToVec(const TIntFltKdV& SpVec, TFltV& Vec, const int& VecLen) {
    Vec.Gen(VecLen); Vec.PutAll(0.0);
    int Elts = SpVec.Len();
    for (int EltN = 0; EltN < Elts; EltN++) {
        if (SpVec[EltN].Key < VecLen) {
            Vec[SpVec[EltN].Key] = SpVec[EltN].Dat;
        }
    }
}

void TLinAlgTransform::Convert(const TVec<TPair<TIntV, TFltV>>& A, TTriple<TIntV, TIntV, TFltV>& B) {
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

void TLinAlgTransform::Convert(const TVec<TIntFltKdV>& A, TTriple<TIntV, TIntV, TFltV>&B) {
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

//////////////////////////////////////////////////////////////////////
/// Contains methods to check the properties of linear algebra structures
bool TLinAlgCheck::IsZeroTol(const TFltV& Vec, const double& Eps) {
	bool IsZero = true;
	for (int i = 0; i < Vec.Len(); i++) {
		if (!TMath::IsInEps((double)Vec[i], Eps)) {
			IsZero = false;
			break;
		}
	}
	return IsZero;
}

bool TLinAlgCheck::IsOrthonormal(const TFltVV& Vecs, const double& Threshold) {
	int m = Vecs.GetCols();
	TFltVV R(m, m);
	TLinAlg::MultiplyT(Vecs, Vecs, R);
	for (int i = 0; i < m; i++) { R(i, i) -= 1; }
	return TLinAlg::Frob(R) < Threshold;
}

////////////////////////////////////////////////////////////////////////
//// Basic Linear Algebra Operations
void TLinAlg::LinComb(const double& p, const TIntFltKdV& x, const double& q, const TIntFltKdV& y, TIntFltKdV& z) {
	TSparseOpsIntFlt::SparseLinComb(p, x, q, y, z);
}

void TLinAlg::LinComb(const double& p, const TVec<TIntFltKdV>& X, const double& q,
        const TVec<TIntFltKdV>& Y, TVec<TIntFltKdV>& Z) {
    if (Z.Empty()) { Z.Gen(X.Len()); }
    EAssert(X.Len() == Y.Len() && Y.Len() == Z.Len());
    int Cols = X.Len();
    for (int ColN = 0; ColN < Cols; ColN++) {
        TLinAlg::LinComb(p, X[ColN], q, Y[ColN], Z[ColN]);
    }
}

void TLinAlg::LinComb(const double& p, const TFltVV& X, const double& q, const TVec<TIntFltKdV>& Y, TFltVV& Z) {
    if (Z.Empty()) { Z.Gen(X.GetRows(), X.GetCols()); }
    EAssert(X.GetRows() >= TLinAlgSearch::GetMaxDimIdx(Y) && X.GetCols() == Y.Len() && X.GetRows() == Z.GetRows() && X.GetCols() == Z.GetCols());
    int Rows = X.GetRows();
    int Cols = X.GetCols();
    for (int ColN = 0; ColN < Cols; ColN++) {
        int KeyN = 0;
        for (int RowN = 0; RowN < Rows; RowN++) {
            Z.At(RowN, ColN) = p*X.At(RowN, ColN);
            if (KeyN < Y[ColN].Len() && Y[ColN][KeyN].Key == RowN) {
                Z.At(RowN, ColN) += q*Y[ColN][KeyN].Dat; KeyN++;
            }
        }
    }
}

void TLinAlg::LinComb(const double& p, const TVec<TIntFltKdV>& X, const double& q, TFltVV const& Y, TVec<TIntFltKdV>& Z) {
    if (Z.Empty()) { Z.Gen(Y.GetCols()); }
    EAssert(TLinAlgSearch::GetMaxDimIdx(X) <= Y.GetRows() && X.Len() == Y.GetCols() && Y.GetCols() == Z.Len());
    int Rows = Y.GetRows();
    int Cols = Y.GetCols();
    for (int ColN = 0; ColN < Cols; ColN++) {
        int KeyN = 0;
        for (int RowN = 0; RowN < Rows; RowN++) {
            if (X[ColN][KeyN].Key == RowN) {
                Z[ColN].Add(TIntFltKd(RowN, p*X[ColN][KeyN].Dat + q*Y.At(RowN, ColN))); KeyN++;
            }
            else {
                Z[ColN].Add(TIntFltKd(RowN, q*Y.At(RowN, ColN)));
            }
        }
    }
}

void TLinAlg::AddVec(const double& k, const TVec<TFltV>& X, int ColId, const TFltV& y, TFltV& z) {
	EAssert(0 <= ColId && ColId < X.Len());
	AddVec(k, X[ColId], y, z);
}

void TLinAlg::AddVec(const double& k, const TFltVV& X, int ColId, const TFltV& y, TFltV& z) {
	EAssert(X.GetRows() == y.Len());
	EAssert(y.Len() == z.Len());
	const int len = z.Len();
	for (int i = 0; i < len; i++) {
		z[i] = y[i] + k * X(i, ColId);
	}
}

void TLinAlg::AddVec(const double& k, const TVec<TIntFltKdV>& X, int ColId, const TFltV& y, TFltV& z) {
	EAssert(0 <= ColId && ColId < X.Len());
	AddVec(k, X[ColId], y, z);
}

void TLinAlg::AddVec(const double& k, const TIntFltKdV& x, TFltV& y) {
	const int xLen = x.Len(), yLen = y.Len();
	for (int i = 0; i < xLen; i++) {
		const int ii = x[i].Key;
		if (ii < yLen) {
			y[ii] += k * x[i].Dat;
		}
	}
}

void TLinAlg::AddVec(const TIntFltKdV& x, const TIntFltKdV& y, TIntFltKdV& z) {
	TSparseOpsIntFlt::SparseMerge(x, y, z);
}

double TLinAlg::EuclDist2(const TFltPr& x, const TFltPr& y) {
	return TMath::Sqr(x.Val1 - y.Val1) + TMath::Sqr(x.Val2 - y.Val2);
}

double TLinAlg::EuclDist(const TFltPr& x, const TFltPr& y) {
	return sqrt(TLinAlg::EuclDist2(x, y));
}

void TLinAlg::Transpose(const TVec<TIntFltKdV>& A, TVec<TIntFltKdV>& At, int Rows){
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

//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TFltVV& B, TFltVV& C, const int RowsA) {
	// A = sparse column matrix
	EAssert(A.Len() == B.GetRows());
	int Rows = RowsA;
	int ColsB = B.GetCols();
	if (RowsA == -1) {
		Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1;
	}
	else {
		EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= RowsA);
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
// SPARSECOLMAT-SPARSECOLMAT

//Andrej Urgent
//TODO template --- indextype TIntFltKdV ... TInt64
//TLAMisc
//GetMaxDimIdx
void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TFltVV& C, const int RowsA) {
	//// A,B = sparse column matrix
	//EAssert(A.Len() == B.GetRows());
	int Rows = RowsA;
	int ColsB = B.Len();
	if (RowsA == -1) {
		Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1;
	}
	else {
		EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= RowsA);
	}
	if (C.Empty()) {
		C.Gen(Rows, ColsB);
	}
	EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.Len());
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

void TLinAlg::Multiply(const TVec<TIntFltKdV>& A, const TVec<TIntFltKdV>& B, TVec<TIntFltKdV>& C,
		const int RowsA) {
    //// A,B = sparse column matrix
    //EAssert(A.Len() == B.GetRows());
    int Rows = RowsA;
    int ColsB = B.Len();

    if (RowsA == -1) { Rows = TLinAlgSearch::GetMaxDimIdx(A) + 1; }
    EAssert(TLinAlgSearch::GetMaxDimIdx(A) + 1 <= Rows);

    C.Gen(ColsB);
    EAssert(TLinAlgSearch::GetMaxDimIdx(B) + 1 <= A.Len());

    for (int ColN = 0; ColN < ColsB; ColN++) {
        int ElsB = B[ColN].Len();
        for (int ElBN = 0; ElBN < ElsB; ElBN++) {
            int IdxB = B[ColN][ElBN].Key;
            double ValB = B[ColN][ElBN].Dat;
            int ElsA = A[IdxB].Len();
            for (int ElAN = 0; ElAN < ElsA; ElAN++) {
                int IdxA = A[IdxB][ElAN].Key;
                double ValA = A[IdxB][ElAN].Dat;
                C[ColN].Add(TIntFltKd(IdxA, ValA * ValB));
            }
        }
    }
}

void TLinAlg::QR(const TFltVV& X, TFltVV& Q, TFltVV& R, const TFlt& Tol) {
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
void TLinAlg::Rotate(const double& OldX, const double& OldY, const double& Angle,
		double& NewX, double& NewY) {
	NewX = OldX*cos(Angle) - OldY*sin(Angle);
	NewY = OldX*sin(Angle) + OldY*cos(Angle);
}

void TLinAlg::NonNegProj(TFltV& Vec) {
	for (int i = 0; i < Vec.Len(); i++) {
		Vec[i] = TMath::Mx(0.0, (double)Vec[i]);
	}
}

void TLinAlg::NonNegProj(TFltVV& Mat) {
	int Rows = Mat.GetRows();
	int Cols = Mat.GetCols();
	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			Mat(RowN, ColN) = TMath::Mx(0.0, (double)Mat(RowN, ColN));
		}
	}
}

///////////////////////////////////////////////////////////////////////
// Numerical Linear Algebra
double TNumericalStuff::sqr(double a) {
  return a == 0.0 ? 0.0 : a*a;
}

double TNumericalStuff::sign(double a, double b) {
  return b >= 0.0 ? fabs(a) : -fabs(a);
}

void TNumericalStuff::nrerror(const TStr& error_text) {
    //printf("NR_ERROR: %s", error_text.CStr());
    throw TNSException::New(error_text);
}

double TNumericalStuff::pythag(double a, double b) {
    double absa = fabs(a), absb = fabs(b);
    if (absa > absb)
        return absa*sqrt(1.0+sqr(absb/absa));
    else
        return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+sqr(absa/absb)));
}

void TNumericalStuff::SymetricToTridiag(TFltVV& a, int n, TFltV& d, TFltV& e) {
    int l,k,j,i;
    double scale,hh,h,g,f;
    for (i=n;i>=2;i--) {
        l=i-1;
        h=scale=0.0;
        if (l > 1) {
            for (k=1;k<=l;k++)
                scale += fabs(a(i-1,k-1).Val);
            if (scale == 0.0) //Skip transformation.
                e[i]=a(i-1,l-1);
            else {
                for (k=1;k<=l;k++) {
                    a(i-1,k-1) /= scale; //Use scaled a's for transformation.
                    h += a(i-1,k-1)*a(i-1,k-1);
                }
                f=a(i-1,l-1);
                g=(f >= 0.0 ? -sqrt(h) : sqrt(h));
                EAssertR(_isnan(g) == 0, TFlt::GetStr(h));
                e[i]=scale*g;
                h -= f*g; //Now h is equation (11.2.4).
                a(i-1,l-1)=f-g; //Store u in the ith row of a.
                f=0.0;
                for (j=1;j<=l;j++) {
                    // Next statement can be omitted if eigenvectors not wanted
                    a(j-1,i-1)=a(i-1,j-1)/h; //Store u=H in ith column of a.
                    g=0.0; //Form an element of A  u in g.
                    for (k=1;k<=j;k++)
                        g += a(j-1,k-1)*a(i-1,k-1);
                    for (k=j+1;k<=l;k++)
                        g += a(k-1,j-1)*a(i-1,k-1);
                    e[j]=g/h; //Form element of p in temporarily unused element of e.
                    f += e[j]*a(i-1,j-1);
                }
                hh=f/(h+h); //Form K, equation (11.2.11).
                for (j=1;j<=l;j++) { //Form q and store in e overwriting p.
                    f=a(i-1,j-1);
                    e[j]=g=e[j]-hh*f;
                    for (k=1;k<=j;k++) { //Reduce a, equation (11.2.13).
                        a(j-1,k-1) -= (f*e[k]+g*a(i-1,k-1));
                        EAssert(!a(j-1,k-1).IsNan());
                    }
                }
            }
        } else
            e[i]=a(i-1,l-1);
        d[i]=h;
    }
    // Next statement can be omitted if eigenvectors not wanted
    d[1]=0.0;
    e[1]=0.0;
    // Contents of this loop can be omitted if eigenvectors not
    // wanted except for statement d[i]=a[i][i];
    for (i=1;i<=n;i++) { //Begin accumulation of transformationmatrices.
        l=i-1;
        if (d[i]) { //This block skipped when i=1.
            for (j=1;j<=l;j++) {
                g=0.0;
                for (k=1;k<=l;k++) //Use u and u=H stored in a to form PQ.
                    g += a(i-1,k-1)*a(k-1,j-1);
                for (k=1;k<=l;k++) {
                    a(k-1,j-1) -= g*a(k-1,i-1);
                    EAssert(!a(k-1,j-1).IsNan());
                }
            }
        }
        d[i]=a(i-1,i-1); //This statement remains.
        a(i-1,i-1)=1.0; //Reset row and column of a to identity  matrix for next iteration.
        for (j=1;j<=l;j++) a(j-1,i-1)=a(i-1,j-1)=0.0;
    }
}

void TNumericalStuff::EigSymmetricTridiag(TFltV& d, TFltV& e, int n, TFltVV& z) {
    int m,l,iter,i,k; // N = n+1;
    double s,r,p,g,f,dd,c,b;
    // Convenient to renumber the elements of e
    for (i=2;i<=n;i++) e[i-1]=e[i];
    e[n]=0.0;
    for (l=1;l<=n;l++) {
        iter=0;
        do {
            // Look for a single small subdiagonal element to split the matrix.
            for (m=l;m<=n-1;m++) {
        dd=TFlt::Abs(d[m])+TFlt::Abs(d[m+1]);
                if ((double)(TFlt::Abs(e[m])+dd) == dd) break;
            }
            if (m != l) {
                if (iter++ == 60) nrerror("Too many iterations in EigSymmetricTridiag");
                //Form shift.
                g=(d[l+1]-d[l])/(2.0*e[l]);
                r=pythag(g,1.0);
                //This is dm - ks.
                g=d[m]-d[l]+e[l]/(g+sign(r,g));
                s=c=1.0;
                p=0.0;
                // A plane rotation as in the original QL, followed by
                // Givens rotations to restore tridiagonal form
                for (i=m-1;i>=l;i--) {
                    f=s*e[i];
                    b=c*e[i];
                    e[i+1]=(r=pythag(f,g));
                    // Recover from underflow.
                    if (r == 0.0) {
                        d[i+1] -= p;
                        e[m]=0.0;
                        break;
                    }
                    s=f/r;
                    c=g/r;
                    g=d[i+1]-p;
                    r=(d[i]-g)*s+2.0*c*b;
                    d[i+1]=g+(p=s*r);
                    g=c*r-b;
                    // Next loop can be omitted if eigenvectors not wanted
                    for (k=0;k<n;k++) {
                        f=z(k,i);
                        z(k,i)=s*z(k,i-1)+c*f;
                        z(k,i-1)=c*z(k,i-1)-s*f;
                    }
                }
                if (r == 0.0 && i >= l) continue;
                d[l] -= p;
                e[l]=g;
                e[m]=0.0;
            }
        } while (m != l);
    }
}

void TNumericalStuff::CholeskyDecomposition(TFltVV& A, TFltV& p) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows(); p.Reserve(n,n);

  int i,j,k;
  double sum;
  for (i=1;i<=n;i++) {
    for (j=i;j<=n;j++) {
      for (sum=A(i-1,j-1),k=i-1;k>=1;k--) sum -= A(i-1,k-1)*A(j-1,k-1);
      if (i == j) {
        if (sum <= 0.0)
          nrerror("choldc failed");
        p[i-1]=sqrt(sum);
      } else A(j-1,i-1)=sum/p[i-1];
    }
  }
}

void TNumericalStuff::CholeskySolve(const TFltVV& A, const TFltV& p, const TFltV& b, TFltV& x) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows(); x.Reserve(n,n);

  int i,k;
  double sum;

  // Solve L * y = b, storing y in x
  for (i=1;i<=n;i++) {
    for (sum=b[i-1],k=i-1;k>=1;k--)
      sum -= A(i-1,k-1)*x[k-1];
    x[i-1]=sum/p[i-1];
  }

  // Solve L^T * x = y
  for (i=n;i>=1;i--) {
    for (sum=x[i-1],k=i+1;k<=n;k++)
      sum -= A(k-1,i-1)*x[k-1];
    x[i-1]=sum/p[i-1];
  }
}

void TNumericalStuff::SolveSymetricSystem(TFltVV& A, const TFltV& b, TFltV& x) {
  EAssert(A.GetRows() == A.GetCols());
  TFltV p; CholeskyDecomposition(A, p);
  CholeskySolve(A, p, b, x);
}

void TNumericalStuff::InverseSubstitute(TFltVV& A, const TFltV& p) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows(); TFltV x(n);

    int i, j, k; double sum;
    for (i = 0; i < n; i++) {
      // solve L * y = e_i, store in x
        // elements from 0 to i-1 are 0.0
        for (j = 0; j < i; j++) x[j] = 0.0;
        // solve l_ii * y_i = 1 => y_i = 1/l_ii
        x[i] = 1/p[i];
        // solve y_j for j > i
        for (j = i+1; j < n; j++) {
            for (sum = 0.0, k = i; k < j; k++)
                sum -= A(j,k) * x[k];
            x[j] = sum / p[j];
        }

      // solve L'* x = y, store in upper triangule of A
        for (j = n-1; j >= i; j--) {
            for (sum = x[j], k = j+1; k < n; k++)
                sum -= A(k,j)*x[k];
            x[j] = sum/p[j];
        }
        for (int j = i; j < n; j++) A(i,j) = x[j];
    }

}

void TNumericalStuff::InverseSymetric(TFltVV& A) {
    EAssert(A.GetRows() == A.GetCols());
    TFltV p;
    // first we calculate cholesky decomposition of A
    CholeskyDecomposition(A, p);
    // than we solve system A x_i = e_i for i = 1..n
    InverseSubstitute(A, p);
}

void TNumericalStuff::InverseTriagonal(TFltVV& A) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows(); TFltV x(n), p(n);

    int i, j, k; double sum;
    // copy upper triangle to lower one as we'll overwrite upper one
    for (i = 0; i < n; i++) {
        p[i] = A(i,i);
        for (j = i+1; j < n; j++)
            A(j,i) = A(i,j);
    }
    // solve
    for (i = 0; i < n; i++) {
        // solve R * x = e_i, store in x
        // elements from 0 to i-1 are 0.0
        for (j = n-1; j > i; j--) x[j] = 0.0;
        // solve l_ii * y_i = 1 => y_i = 1/l_ii
        x[i] = 1/p[i];
        // solve y_j for j > i
        for (j = i-1; j >= 0; j--) {
            for (sum = 0.0, k = i; k > j; k--)
                sum -= A(k,j) * x[k];
            x[j] = sum / p[j];
        }
        for (int j = 0; j <= i; j++) A(j,i) = x[j];
    }
}

void TNumericalStuff::LUDecomposition(TFltVV& A, TIntV& indx, double& d) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows(); indx.Reserve(n,n);

    int i=0,imax=0,j=0,k=0;
    double big,dum,sum,temp;
    TFltV vv(n); // vv stores the implicit scaling of each row.
    d=1.0;       // No row interchanges yet.

    // Loop over rows to get the implicit scaling information.
    for (i=1;i<=n;i++) {
        big=0.0;
        for (j=1;j<=n;j++)
            if ((temp=TFlt::Abs(A(i-1,j-1))) > big) big=temp;
        if (big == 0.0) nrerror("Singular matrix in routine LUDecomposition");
        vv[i-1]=1.0/big;
    }

    for (j=1;j<=n;j++) {
        for (i=1;i<j;i++) {
            sum=A(i-1,j-1);
            for (k=1;k<i;k++) sum -= A(i-1,k-1)*A(k-1,j-1);
            A(i-1,j-1)=sum;
        }
        big=0.0; //Initialize for the search for largest pivot element.
        for (i=j;i<=n;i++) {
            sum=A(i-1,j-1);
            for (k=1;k<j;k++)
                sum -= A(i-1,k-1)*A(k-1,j-1);
            A(i-1,j-1)=sum;

            //Is the figure of merit for the pivot better than the best so far?
            if ((dum=vv[i-1] * TFlt::Abs(sum)) >= big) {
                big=dum;
                imax=i;
            }
        }

        //Do we need to interchange rows?
        if (j != imax) {
            //Yes, do so...
            for (k=1;k<=n;k++) {
                dum=A(imax-1,k-1);
            A(imax-1,k-1)=A(j-1,k-1); // Tadej: imax-1,k looks wrong
            A(j-1,k-1)=dum;
            }
            //...and change the parity of d.
            d = -d;
            vv[imax-1]=vv[j-1]; //Also interchange the scale factor.
        }
        indx[j-1]=imax;

        //If the pivot element is zero the matrix is singular (at least to the precision of the
        //algorithm). For some applications on singular matrices, it is desirable to substitute
        //TINY for zero.
        if (A(j-1,j-1) == 0.0) A(j-1,j-1)=1e-20;

         //Now, finally, divide by the pivot element.
        if (j != n) {
            dum=1.0/(A(j-1,j-1));
            for (i=j+1;i<=n;i++) A(i-1,j-1) *= dum;
        }
    } //Go back for the next column in the reduction.
}

void TNumericalStuff::LUSolve(const TFltVV& A, const TIntV& indx, TFltV& b) {
  EAssert(A.GetRows() == A.GetCols());
  int n = A.GetRows();
    int i,ii=0,ip,j;
    double sum;
    for (i=1;i<=n;i++) {
        ip=indx[i-1];
        sum=b[ip-1];
        b[ip-1]=b[i-1];
        if (ii) {
            for (j=ii;j<=i-1;j++) { sum -= A(i-1,j-1)*b[j-1]; }
        } else if (sum) { ii=i; }
        b[i-1]=sum;
    }
    for (i=n;i>=1;i--) {
        sum=b[i-1];
        for (j=i+1;j<=n;j++) { sum -= A(i-1,j-1)*b[j-1]; }
        b[i-1]=sum/A(i-1,i-1);
    }
}

void TNumericalStuff::SolveLinearSystem(TFltVV& A, const TFltV& b, TFltV& x) {
    TIntV indx; double d;
    LUDecomposition(A, indx, d);
    x = b;
    LUSolve(A, indx, x);
}

void TNumericalStuff::LeastSquares(const TFltVV& A, const TFltV& b, const double& Gamma, TFltV& x) {
	if (A.GetRows() < A.GetCols()) {
		TNumericalStuff::PrimalLeastSquares(A, b, Gamma, x);
	} else {
		TNumericalStuff::DualLeastSquares(A, b, Gamma, x);
	}
}

void TNumericalStuff::PrimalLeastSquares(const TFltVV& A, const TFltV& b, const double& Gamma, TFltV& x) {
	EAssertR(A.GetCols() == b.Len(), "TNumericalStuff::LeastSquares: number of columns (examples) does not match the number of targets (length of b)");
	if (x.Empty()) { 
		x.Gen(A.GetRows());
	} else {
		EAssertR(x.Len() == A.GetRows(), "TNumericalStuff::LeastSquares: solution dimension does not match the number of rows of A (features)");
	}
	// x = (A * A' + Gamma^2 * I)^{-1} A * b
	int Feats = A.GetRows();
	// A'
	TFltVV At = TFltVV(A.GetCols(), A.GetRows()); 
	TLinAlg::Transpose(A, At);
	// A * A'
	TFltVV B = TFltVV(Feats, Feats);
	TLinAlg::Multiply(A, At, B);
	// I
	TFltVV I = TFltVV(Feats, Feats);
	TFltV Ones = TFltV(Feats); Ones.PutAll(1.0);
	TLinAlgTransform::Diag(Ones, I);
	// B = A * A' + Gamma^2 * I
	TLinAlg::LinComb(1.0, B, Gamma*Gamma, I, B);
	// Ab = A * b
	TFltV Ab = TFltV(Feats);
	TLinAlg::Multiply(A, b, Ab);
	TNumericalStuff::SolveLinearSystem(B, Ab, x);
}

void TNumericalStuff::DualLeastSquares(const TFltVV& A, const TFltV& b, const double& Gamma, TFltV& x) {
	EAssertR(A.GetCols() == b.Len(), "TNumericalStuff::LeastSquares: number of columns (examples) does not match the number of targets (length of b)");
	if (x.Empty()) { 
		x.Gen(A.GetRows());
	} else {
		EAssertR(x.Len() == A.GetRows(), "TNumericalStuff::DualLeastSquares: solution dimension does not match the number of rows of A (features)");
	}

	// x = A (A' * A + Gamma^2 * I)^{-1} * b
	int N = A.GetCols();
	// B = A' * A
	TFltVV B = TFltVV(N, N);
	TLinAlg::MultiplyT(A, A, B);
	// I
	TFltVV I = TFltVV(N, N);
	TFltV Ones = TFltV(N); Ones.PutAll(1.0);
	TLinAlgTransform::Diag(Ones, I);
	// B = A' * A + Gamma^2 * I
	TLinAlg::LinComb(1.0, B, Gamma*Gamma, I, B);
	// B^{-1}b
	TFltV InvBb = TFltV(N);
	TNumericalStuff::SolveLinearSystem(B, b, InvBb);
	// x = A * InvB
	TLinAlg::Multiply(A, InvBb, x);	
}

void TNumericalStuff::GetKernelVec(const TFltVV& A, TFltV& x) {
    EAssertR(A.GetRows() == A.GetCols(), "TNumericalStuff::GetKernelVec: input is not a square matrix!");

    const int Dim = A.GetRows();

#ifdef LAPACKE
    TFltVV L, U;
    TVec<TNum<index_t>, index_t> PermV;
    MKLfunctions::LUFactorization(A, L, U, PermV);
#else
    TFltVV U = A;
    TIntV PermV;
    double d;
    LUDecomposition(U, PermV, d);
#endif

    EAssertR(TFlt::Abs(U(Dim-1, Dim-1)) < 1e-6, "TNumericalStuff::GetKernelVec: Input is not a singular matrix!");

    x.Gen(Dim);

    // set the last element to an arbitrary value
    x.Last() = 1;
    // inverse iteration
    for (int RowN = Dim-2; RowN >= 0; RowN--) {
        double Sum = 0;

        for (int ColN = RowN+1; ColN < Dim; ColN++) {
            Sum += U(RowN, ColN)*x[ColN];
        }

        AssertR(double(U(RowN, RowN)) != 0, "TNumericalStuff::GetKernelVec: Dimension of kernel is more than 1!");

        x[RowN] = -Sum / U(RowN, RowN);
    }
}

void TNumericalStuff::GetEigenVec(const TFltVV& A, const double& EigenVal, TFltV& EigenV) {
    const int Dim = A.GetRows();

    // first compute (A - Lambda*I)
    TFltVV A1 = A;

    for (int i = 0; i < Dim; i++) {
        A1(i,i) -= EigenVal;
    }

    // the result is in the kernel of (A - Lambda*I)
    GetKernelVec(A1, EigenV);
}

///////////////////////////////////////////////////////////////////////
// Sparse-SVD
void TSparseSVD::MultiplyATA(const TMatrix& Matrix,
        const TFltVV& Vec, int ColId, TFltV& Result) {
    TFltV tmp(Matrix.GetRows());
    // tmp = A * Vec(:,ColId)
    Matrix.Multiply(Vec, ColId, tmp);
    // Vec = A' * tmp
    Matrix.MultiplyT(tmp, Result);
}

void TSparseSVD::MultiplyATA(const TMatrix& Matrix,
        const TFltV& Vec, TFltV& Result) {
    TFltV tmp(Matrix.GetRows());
    // tmp = A * Vec
    Matrix.Multiply(Vec, tmp);
    // Vec = A' * tmp
    Matrix.MultiplyT(tmp, Result);
}

void TSparseSVD::OrtoIterSVD(const TMatrix& Matrix,
        int NumSV, int IterN, TFltV& SgnValV) {

    int i, j, k;
    int N = Matrix.GetCols(), M = NumSV;
    TFltVV Q(N, M);

    // Q = rand(N,M)
    TRnd rnd;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++)
            Q(i,j) = rnd.GetUniDev();
    }

    TFltV tmp(N);
    for (int IterC = 0; IterC < IterN; IterC++) {
        printf("%d..", IterC);
        // Gram-Schmidt
        TLinAlg::GS(Q);
        // Q = A'*A*Q
        for (int ColId = 0; ColId < M; ColId++) {
            MultiplyATA(Matrix, Q, ColId, tmp);
            for (k = 0; k < N; k++) Q(k,ColId) = tmp[k];
        }
    }

    SgnValV.Reserve(NumSV,0);
    for (i = 0; i < NumSV; i++)
        SgnValV.Add(sqrt(TLinAlg::Norm(Q,i)));
    TLinAlg::GS(Q);
}

void TSparseSVD::OrtoIterSVD(const TMatrix& Matrix,
        const int k, TFltV& S, TFltVV& U, TFltVV& V, const int Iters_, const double Tol) {
	const int Iters = Iters_ != -1 ? Iters_ : 100;
	int Rows = Matrix.GetRows();
	int Cols = Matrix.GetCols();
	EAssert(k <= Rows && k <= Cols);
	TFltVV Q, R;
	
	if (S.Empty()) {S.Gen(k);}
	if (U.Empty()) {U.Gen(Rows, k); TLinAlgTransform::FillRnd(U);}
	if (V.Empty()) {V.Gen(Cols, k);}


	TFltV SOld = S;	
    for (int IterN = 0; IterN < Iters; IterN++) {
		Matrix.MultiplyT(U, V);
		for (int i = 0; i < k; i++) {
			S[i] = TLinAlg::Norm(V,i);
		}		
		Matrix.Multiply(V, U);
		//U = GS(AA'U)
		// orthogonalization
		TLinAlg::QR(U, U, R, Tol);
		if (!TLinAlgCheck::IsOrthonormal(U, Tol)) {
			// reorthogonalization
			TLinAlg::QR(U, U, R, Tol);
		}
		if (!TLinAlgCheck::IsOrthonormal(U, Tol)) {
			printf("Orthofail!\n");
		}
		if (IterN > 0 && sqrt(TLinAlg::FrobDist2(S, SOld)/TLinAlg::Norm2(S)) < Tol) {break;}
		SOld = S;
    }

	Matrix.MultiplyT(U, V);
	for (int i = 0; i < k; i++) {
		S[i] = TLinAlg::Norm(V, i);
	}
	TLinAlg::QR(V, V, R, Tol);
}

void TSparseSVD::SimpleLanczos(const TMatrix& Matrix,
        const int& NumEig, TFltV& EigValV,
        const bool& DoLocalReortoP, const bool& SvdMatrixProductP) {

    if (SvdMatrixProductP) {
        // if this fails, use transposed matrix
        EAssert(Matrix.GetRows() >= Matrix.GetCols());
    } else {
        EAssert(Matrix.GetRows() == Matrix.GetCols());
    }

    const int N = Matrix.GetCols(); // size of matrix
    TFltV r(N), v0(N), v1(N); // current vector and 2 previous ones
    TFltV alpha(NumEig, 0), beta(NumEig, 0); // diagonal and subdiagonal of T

    printf("Calculating %d eigen-values of %d x %d matrix\n", NumEig, N, N);

    // set starting vector
    //TRnd Rnd(0);
    for (int i = 0; i < N; i++) {
        r[i] = 1/sqrt((double)N); // Rnd.GetNrmDev();
        v0[i] = v1[i] = 0.0;
    }
    beta.Add(TLinAlg::Norm(r));

    for (int j = 0; j < NumEig; j++) {
        printf("%d\r", j+1);
        // v_j -> v_(j-1)
        v0 = v1;
        // v_j = (1/beta_(j-1)) * r
        TLinAlg::MultiplyScalar(1/beta[j], r, v1);
        // r = A*v_j
        if (SvdMatrixProductP) {
            // A = Matrix'*Matrix
            MultiplyATA(Matrix, v1, r);
        } else {
            // A = Matrix
            Matrix.Multiply(v1, r);
        }
        // r = r - beta_(j-1) * v_(j-1)
        TLinAlg::AddVec(-beta[j], v0, r, r);
        // alpha_j = vj'*r
        alpha.Add(TLinAlg::DotProduct(v1, r));
        // r = r - v_j * alpha_j
        TLinAlg::AddVec(-alpha[j], v1, r, r);
        // reortogonalization if neessary
        if (DoLocalReortoP) { } //TODO
        // beta_j = ||r||_2
        beta.Add(TLinAlg::Norm(r));
        // compoute approximatie eigenvalues T_j
        // test bounds for convergence
    }
    printf("\n");

    // prepare matrix T
    TFltV d(NumEig + 1), e(NumEig + 1);
    d[1] = alpha[0]; d[0] = e[0] = e[1] = 0.0;
    for (int i = 1; i < NumEig; i++) {
        d[i+1] = alpha[i]; e[i+1] = beta[i]; }
    // solve eigne problem for tridiagonal matrix with diag d and subdiag e
    TFltVV S(NumEig+1,NumEig+1); // eigen-vectors
    TLinAlgTransform::FillIdentity(S); // make it identity
    TNumericalStuff::EigSymmetricTridiag(d, e, NumEig, S); // solve
    //TLAMisc::PrintTFltV(d, "AllEigV");

    // check convergence
    TFltKdV AllEigValV(NumEig, 0);
    for (int i = 1; i <= NumEig; i++) {
        const double ResidualNorm = TFlt::Abs(S(i-1, NumEig-1) * beta.Last());
        if (ResidualNorm < 1e-5)
            AllEigValV.Add(TFltKd(TFlt::Abs(d[i]), d[i]));
    }

    // prepare results
    AllEigValV.Sort(false); EigValV.Gen(NumEig, 0);
    for (int i = 0; i < AllEigValV.Len(); i++) {
        if (i == 0 || (TFlt::Abs(AllEigValV[i].Dat/AllEigValV[i-1].Dat) < 0.9999))
            EigValV.Add(AllEigValV[i].Dat);
    }
}

void TSparseSVD::Lanczos(const TMatrix& Matrix, int NumEig,
        int Iters, const TSpSVDReOrtoType& ReOrtoType,
        TFltV& EigValV, TFltVV& EigVecVV, const bool& SvdMatrixProductP) {

    if (SvdMatrixProductP) {
        // if this fails, use transposed matrix
        EAssert(Matrix.GetRows() >= Matrix.GetCols());
    } else {
        EAssert(Matrix.GetRows() == Matrix.GetCols());
    }
 	EAssertR(NumEig <= Iters, TStr::Fmt("%d <= %d", NumEig, Iters));

    //if (ReOrtoType == ssotFull) printf("Full reortogonalization\n");
    int i, N = Matrix.GetCols(), K = 0; // K - current dimension of T
    double t = 0.0, eps = 1e-6; // t - 1-norm of T

    //sequence of Ritz's vectors
    TFltVV Q(N, Iters);
    double tmp = 1/sqrt((double)N);
    for (i = 0; i < N; i++) {
        Q(i,0) = tmp;
    }
    //converget Ritz's vectors
    TVec<TFltV> ConvgQV(Iters);
    TIntV CountConvgV(Iters);
    for (i = 0; i < Iters; i++) CountConvgV[i] = 0;
    // const int ConvgTreshold = 50;

    //diagonal and subdiagonal of T
    TFltV d(Iters+1), e(Iters+1);
    //eigenvectors of T
    //TFltVV V;
    TFltVV V(Iters, Iters);

    // z - current Lanczos's vector
    TFltV z(N), bb(Iters), aa(Iters), y(N);
    //printf("svd(%d,%d)...\n", NumEig, Iters);

    if (SvdMatrixProductP) {
        // A = Matrix'*Matrix
        MultiplyATA(Matrix, Q, 0, z);
    } else {
        // A = Matrix
        Matrix.Multiply(Q, 0, z);
    }

    for (int j = 0; j < (Iters-1); j++) {
        //printf("%d..\r",j+2);

        //calculates (j+1)-th Lanczos's vector
        // aa[j] = <Q(:,j), z>
        aa[j] = TLinAlg::DotProduct(Q, j, z);
        //printf(" %g -- ", aa[j].Val); //HACK

        TLinAlg::AddVec(-aa[j], Q, j, z);
        if (j > 0) {
            // z := -aa[j] * Q(:,j) + z
            TLinAlg::AddVec(-bb[j-1], Q, j-1, z);

            //reortogonalization
            if (ReOrtoType == ssotSelective || ReOrtoType == ssotFull) {
                for (i = 0; i <= j; i++) {
                    // if i-tj vector converget, than we have to ortogonalize against it
                    if ((ReOrtoType == ssotFull) ||
                        (bb[j-1] * TFlt::Abs(V(K-1, i)) < eps * t)) {

                        ConvgQV[i].Reserve(N,N); CountConvgV[i]++;
                        TFltV& vec = ConvgQV[i];
                        //vec = Q * V(:,i)
                        for (int k = 0; k < N; k++) {
                            vec[k] = 0.0;
                            for (int l = 0; l < K; l++)
                                vec[k] += Q(k,l) * V(l,i);
                        }
                        TLinAlg::AddVec(-TLinAlg::DotProduct(ConvgQV[i], z), ConvgQV[i], z ,z);
                    }
                }
            }
        }

        //adds (j+1)-th Lanczos's vector to Q
        bb[j] = TLinAlg::Norm(z);
    if (!(bb[j] > 1e-10)) {
      printf("Rank of matrix is only %d\n", j+2);
      printf("Last singular value is %g\n", bb[j].Val);
      break;
    }
        for (i = 0; i < N; i++) {
            Q(i, j+1) = z[i] / bb[j];
        }

        //next Lanzcos vector
        if (SvdMatrixProductP) {
            // A = Matrix'*Matrix
            MultiplyATA(Matrix, Q, j+1, z);
        } else {
            // A = Matrix
            Matrix.Multiply(Q, j+1, z);
        }

        //calculate T (K x K matrix)
        K = j + 2;
        // calculate diagonal
        for (i = 1; i < K; i++) d[i] = aa[i-1];
        d[K] = TLinAlg::DotProduct(Q, K-1, z);
        // calculate subdiagonal
        e[1] = 0.0;
        for (i = 2; i <= K; i++) e[i] = bb[i-2];

        //calculate 1-norm of T
        t = TFlt::GetMx(TFlt::Abs(d[1]) + TFlt::Abs(e[2]), TFlt::Abs(e[K]) + TFlt::Abs(d[K]));
        for (i = 2; i < K; i++) {
            t = TFlt::GetMx(t, TFlt::Abs(e[i]) + TFlt::Abs(d[i]) + TFlt::Abs(e[i+1]));
        }

        //set V to identity matrix
        //V.Gen(K,K);
        for (i = 0; i < K; i++) {
            for (int k = 0; k < K; k++) {
                V(i,k) = 0.0;
            }
            V(i,i) = 1.0;
        }

        //eigenvectors of T
        TNumericalStuff::EigSymmetricTridiag(d, e, K, V);
    }//for
    //printf("\n");

    // Finds NumEig largest eigen values
    TFltIntKdV sv(K);
    for (i = 0; i < K; i++) {
        sv[i].Key = TFlt::Abs(d[i+1]);
        sv[i].Dat = i;
    }
    sv.Sort(false);

    TFltV uu(Matrix.GetRows());
    const int FinalNumEig = TInt::GetMn(NumEig, K);
    EigValV.Reserve(FinalNumEig,0);
    EigVecVV.Gen(Matrix.GetCols(), FinalNumEig);
    for (i = 0; i < FinalNumEig; i++) {
        //printf("s[%d] = %20.15f\r", i, sv[i].Key.Val);
        int ii = sv[i].Dat;
        double sigma = d[ii+1].Val;
        // calculate singular value
        EigValV.Add(sigma);
        // calculate i-th right singular vector ( V := Q * W )
        TLinAlg::Multiply(Q, V, ii, EigVecVV, i);
    }
    //printf("done                           \n");
}

void TSparseSVD::Lanczos2(const TMatrix& Matrix, int MaxNumEig,
    int MaxSecs, const TSpSVDReOrtoType& ReOrtoType,
    TFltV& EigValV, TFltVV& EigVecVV, const bool& SvdMatrixProductP) {

  if (SvdMatrixProductP) {
    // if this fails, use transposed matrix
    EAssert(Matrix.GetRows() >= Matrix.GetCols());
  } else {
    EAssert(Matrix.GetRows() == Matrix.GetCols());
  }
  //EAssertR(NumEig <= Iters, TStr::Fmt("%d <= %d", NumEig, Iters));

  //if (ReOrtoType == ssotFull) printf("Full reortogonalization\n");
  int i, N = Matrix.GetCols(), K = 0; // K - current dimension of T
  double t = 0.0, eps = 1e-6; // t - 1-norm of T

  //sequence of Ritz's vectors
  TFltVV Q(N, MaxNumEig);
  double tmp = 1/sqrt((double)N);
  for (i = 0; i < N; i++) {
      Q(i,0) = tmp;
  }
  //converget Ritz's vectors
  TVec<TFltV> ConvgQV(MaxNumEig);
  TIntV CountConvgV(MaxNumEig);
  for (i = 0; i < MaxNumEig; i++) {
      CountConvgV[i] = 0;
  }
  // const int ConvgTreshold = 50;

  //diagonal and subdiagonal of T
  TFltV d(MaxNumEig+1), e(MaxNumEig+1);
  //eigenvectors of T
  //TFltVV V;
  TFltVV V(MaxNumEig, MaxNumEig);

  // z - current Lanczos's vector
  TFltV z(N), bb(MaxNumEig), aa(MaxNumEig), y(N);
  //printf("svd(%d,%d)...\n", NumEig, Iters);

  if (SvdMatrixProductP) {
      // A = Matrix'*Matrix
      MultiplyATA(Matrix, Q, 0, z);
  } else {
      // A = Matrix
      Matrix.Multiply(Q, 0, z);
  }
  TExeTm ExeTm;
  for (int j = 0; j < (MaxNumEig-1); j++) {
    printf("%d [%s]..\r",j+2, ExeTm.GetStr());
    if (ExeTm.GetSecs() > MaxSecs) { break; }

    //calculates (j+1)-th Lanczos's vector
    // aa[j] = <Q(:,j), z>
    aa[j] = TLinAlg::DotProduct(Q, j, z);
    //printf(" %g -- ", aa[j].Val); //HACK

    TLinAlg::AddVec(-aa[j], Q, j, z);
    if (j > 0) {
        // z := -aa[j] * Q(:,j) + z
        TLinAlg::AddVec(-bb[j-1], Q, j-1, z);

        //reortogonalization
        if (ReOrtoType == ssotSelective || ReOrtoType == ssotFull) {
            for (i = 0; i <= j; i++) {
                // if i-tj vector converget, than we have to ortogonalize against it
                if ((ReOrtoType == ssotFull) ||
                    (bb[j-1] * TFlt::Abs(V(K-1, i)) < eps * t)) {

                    ConvgQV[i].Reserve(N,N); CountConvgV[i]++;
                    TFltV& vec = ConvgQV[i];
                    //vec = Q * V(:,i)
                    for (int k = 0; k < N; k++) {
                        vec[k] = 0.0;
                        for (int l = 0; l < K; l++)
                            vec[k] += Q(k,l) * V(l,i);
                    }
                    TLinAlg::AddVec(-TLinAlg::DotProduct(ConvgQV[i], z), ConvgQV[i], z ,z);
                }
            }
        }
    }

    //adds (j+1)-th Lanczos's vector to Q
    bb[j] = TLinAlg::Norm(z);
    if (!(bb[j] > 1e-10)) {
      printf("Rank of matrix is only %d\n", j+2);
      printf("Last singular value is %g\n", bb[j].Val);
      break;
    }
    for (i = 0; i < N; i++)
        Q(i, j+1) = z[i] / bb[j];

    //next Lanzcos vector
    if (SvdMatrixProductP) {
        // A = Matrix'*Matrix
        MultiplyATA(Matrix, Q, j+1, z);
    } else {
        // A = Matrix
        Matrix.Multiply(Q, j+1, z);
    }

    //calculate T (K x K matrix)
    K = j + 2;
    // calculate diagonal
    for (i = 1; i < K; i++) d[i] = aa[i-1];
    d[K] = TLinAlg::DotProduct(Q, K-1, z);
    // calculate subdiagonal
    e[1] = 0.0;
    for (i = 2; i <= K; i++) e[i] = bb[i-2];

    //calculate 1-norm of T
    t = TFlt::GetMx(TFlt::Abs(d[1]) + TFlt::Abs(e[2]), TFlt::Abs(e[K]) + TFlt::Abs(d[K]));
    for (i = 2; i < K; i++)
        t = TFlt::GetMx(t, TFlt::Abs(e[i]) + TFlt::Abs(d[i]) + TFlt::Abs(e[i+1]));

    //set V to identity matrix
    //V.Gen(K,K);
    for (i = 0; i < K; i++) {
        for (int k = 0; k < K; k++)
            V(i,k) = 0.0;
        V(i,i) = 1.0;
    }

    //eigenvectors of T
    TNumericalStuff::EigSymmetricTridiag(d, e, K, V);
  }//for
  printf("... calc %d.", K);
  // Finds NumEig largest eigen values
  TFltIntKdV sv(K);
  for (i = 0; i < K; i++) {
    sv[i].Key = TFlt::Abs(d[i+1]);
    sv[i].Dat = i;
  }
  sv.Sort(false);

  TFltV uu(Matrix.GetRows());
  const int FinalNumEig = K; //TInt::GetMn(NumEig, K);
  EigValV.Reserve(FinalNumEig,0);
  EigVecVV.Gen(Matrix.GetCols(), FinalNumEig);
  for (i = 0; i < FinalNumEig; i++) {
    //printf("s[%d] = %20.15f\r", i, sv[i].Key.Val);
    int ii = sv[i].Dat;
    double sigma = d[ii+1].Val;
    // calculate singular value
    EigValV.Add(sigma);
    // calculate i-th right singular vector ( V := Q * W )
    TLinAlg::Multiply(Q, V, ii, EigVecVV, i);
  }
  printf("  done\n");
}


void TSparseSVD::SimpleLanczosSVD(const TMatrix& Matrix,
        const int& CalcSV, TFltV& SngValV, const bool& DoLocalReorto) {

    SimpleLanczos(Matrix, CalcSV, SngValV, DoLocalReorto, true);
    for (int SngValN = 0; SngValN < SngValV.Len(); SngValN++) {
      //EAssert(SngValV[SngValN] >= 0.0);
      if (SngValV[SngValN] < 0.0) {
        printf("bad sng val: %d %g\n", SngValN, SngValV[SngValN]());
        SngValV[SngValN] = 0;
      }
      SngValV[SngValN] = sqrt(SngValV[SngValN].Val);
    }
}

void TSparseSVD::LanczosSVD(const TMatrix& Matrix, int NumSV,
        int Iters, const TSpSVDReOrtoType& ReOrtoType,
        TFltV& SgnValV, TFltVV& LeftSgnVecVV, TFltVV& RightSgnVecVV) {

    // solve eigen problem for Matrix'*Matrix
    Lanczos(Matrix, NumSV, Iters, ReOrtoType, SgnValV, RightSgnVecVV, true);
    // calculate left singular vectors and sqrt singular values
    const int FinalNumSV = SgnValV.Len();
    LeftSgnVecVV.Gen(Matrix.GetRows(), FinalNumSV);
    TFltV LeftSgnVecV(Matrix.GetRows());
    for (int i = 0; i < FinalNumSV; i++) {
        if (SgnValV[i].Val < 0.0) { SgnValV[i] = 0.0; }
        const double SgnVal = sqrt(SgnValV[i]);
        SgnValV[i] = SgnVal;
        // calculate i-th left singular vector ( U := A * V * S^(-1) )
        Matrix.Multiply(RightSgnVecVV, i, LeftSgnVecV);
        for (int j = 0; j < LeftSgnVecV.Len(); j++) {
            LeftSgnVecVV(j,i) = LeftSgnVecV[j] / SgnVal; }
    }
    //printf("done                           \n");
}

void TSparseSVD::Project(const TIntFltKdV& Vec, const TFltVV& U, TFltV& ProjVec) {
    const int m = U.GetCols(); // number of columns

    ProjVec.Gen(m, 0);
    for (int j = 0; j < m; j++) {
        double x = 0.0;
        for (int i = 0; i < Vec.Len(); i++)
            x += U(Vec[i].Key, j) * Vec[i].Dat;
        ProjVec.Add(x);
    }
}

//////////////////////////////////////////////////////////////////////
// Sigmoid
double TSigmoid::EvaluateFit(const TFltIntKdV& data, const double A, const double B)
{
  double J = 0.0;
  for (int i = 0; i < data.Len(); i++)
  {
    double zi = data[i].Key; int yi = data[i].Dat;
    double e = exp(-A * zi + B);
    double denum = 1.0 + e;
    double prob = (yi > 0) ? (1.0 / denum) : (e / denum);
    J -= log(prob < 1e-20 ? 1e-20 : prob);
  }
  return J;
}

void TSigmoid::EvaluateFit(const TFltIntKdV& data, const double A, const double B, double& J, double& JA, double& JB)
{
  //               J(A, B) = \sum_{i : y_i = 1} ln [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} [ln [1 + e^{-Az_i + B}] - {-Az_i + B}]
  //                       = \sum_i ln [1 + e^{-Az_i + B}] - \sum_{i : y_i = -1} {-Az_i + B}.
  // partial J / partial A = \sum_i (-z_i) e^{-Az_i + B} / [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} Az_i.
  // partial J / partial B = \sum_i        e^{-Az_i + B} / [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} (-1).
  J = 0.0; double sum_all_PyNeg = 0.0, sum_all_ziPyNeg = 0.0, sum_yNeg_zi = 0.0, sum_yNeg_1 = 0.0;
  for (int i = 0; i < data.Len(); i++)
  {
    double zi = data[i].Key; int yi = data[i].Dat;
    double e = exp(-A * zi + B);
    double denum = 1.0 + e;
    double prob = (yi > 0) ? (1.0 / denum) : (e / denum);
    J -= log(prob < 1e-20 ? 1e-20 : prob);
    sum_all_PyNeg += e / denum;
    sum_all_ziPyNeg += zi * e / denum;
    if (yi < 0) { sum_yNeg_zi += zi; sum_yNeg_1 += 1; }
  }
  JA = -sum_all_ziPyNeg +     sum_yNeg_zi;
  JB =  sum_all_PyNeg   -     sum_yNeg_1;
}

void TSigmoid::EvaluateFit(const TFltIntKdV& data, const double A, const double B, const double U,
                           const double V, const double lambda, double& J, double& JJ, double& JJJ)
{
  // Let E_i = e^{-(A + lambda U) z_i + (B + lambda V)}.  Then we have
  // J(lambda) = \sum_i ln [1 + E_i] - \sum_{i : y_i = -1} {-(A + lambda U)z_i + (B + lambda V)}.
  // J'(lambda) = \sum_i (V - U z_i) E_i / [1 + E_i] - \sum_{i : y_i = -1} {V - U z_i).
  //            = \sum_i (V - U z_i) [1 - 1 / [1 + E_i]] - \sum_{i : y_i = -1} {V - U z_i).
  // J"(lambda) = \sum_i (V - U z_i)^2 E_i / [1 + E_i]^2.
  J = 0.0; JJ = 0.0; JJJ = 0.0;
  for (int i = 0; i < data.Len(); i++)
  {
    double zi = data[i].Key; int yi = data[i].Dat;
    double e = exp(-A * zi + B);
    double denum = 1.0 + e;
    double prob = (yi > 0) ? (1.0 / denum) : (e / denum);
    J -= log(prob < 1e-20 ? 1e-20 : prob);
    double VU = V - U * zi;
    JJ += VU * (e / denum); if (yi < 0) JJ -= VU;
    JJJ += VU * VU * e / denum / denum;
  }
}

TSigmoid::TSigmoid(const TFltIntKdV& data) {
  // Let z_i be the projection of the i'th training example, and y_i \in {-1, +1} be its class label.
  // Our sigmoid is: P(Y = y | Z = z) = 1 / [1 + e^{-Az + B}]
  // and we want to maximize \prod_i P(Y = y_i | Z = z_i)
  //                       = \prod_{i : y_i = 1} 1 / [1 + e^{-Az_i + B}]  \prod_{i : y_i = -1} e^{-Az_i + B} / [1 + e^{-Az_i + B}]
  // or minimize its negative logarithm,
  //               J(A, B) = \sum_{i : y_i = 1} ln [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} [ln [1 + e^{-Az_i + B}] - {-Az_i + B}]
  //                       = \sum_i ln [1 + e^{-Az_i + B}] - \sum_{i : y_i = -1} {-Az_i + B}.
  // partial J / partial A = \sum_i (-z_i) e^{-Az_i + B} / [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} Az_i.
  // partial J / partial B = \sum_i        e^{-Az_i + B} / [1 + e^{-Az_i + B}] + \sum_{i : y_i = -1} (-1).
  double minProj = data[0].Key, maxProj = data[0].Key;
  {for (int i = 1; i < data.Len(); i++) {
    double zi = data[i].Key; if (zi < minProj) minProj = zi; if (zi > maxProj) maxProj = zi; }}
  // const bool dump = true;
  A = 1.0; B = 0.5 * (minProj + maxProj);
  double bestJ = 0.0, bestA = 0.0, bestB = 0.0, lambda = 1.0;
  for (int nIter = 0; nIter < 50; nIter++)
  {
    double J, JA, JB; TSigmoid::EvaluateFit(data, A, B, J, JA, JB);
    if (nIter == 0 || J < bestJ) { bestJ = J; bestA = A; bestB = B; }
    // How far should we move?
    // if (dump) printf("Iter %2d: A = %.5f, B = %.5f, J = %.5f, partial = (%.5f, %.5f)\n", nIter, A.Val, B.Val, J, JA, JB);
    double norm = TMath::Sqr(JA) + TMath::Sqr(JB);
    if (norm < 1e-10) break;
    const int cl = -1; // should be -1

    double Jc = TSigmoid::EvaluateFit(data, A + cl * lambda * JA / norm, B + cl * lambda * JB / norm);
    // if (dump) printf("  At lambda = %.5f, Jc = %.5f\n", lambda, Jc);
    if (Jc > J) {
      while (lambda > 1e-5) {
        lambda = 0.5 * lambda;
        Jc = TSigmoid::EvaluateFit(data, A + cl * lambda * JA / norm, B + cl * lambda * JB / norm);
        // if (dump) printf("  At lambda = %.5f, Jc = %.5f\n", lambda, Jc);
      } }
    else if (Jc < J) {
      while (lambda < 1e5) {
        double lambda2 = 2 * lambda;
        double Jc2 = TSigmoid::EvaluateFit(data, A + cl * lambda2 * JA / norm, B + cl * lambda2 * JB / norm);
        // if (dump) printf("  At lambda = %.5f, Jc = %.5f\n", lambda2, Jc2);
        if (Jc2 > Jc) { break; }
        if (TFlt::IsNan(Jc2)) { break; }
        lambda = lambda2; Jc = Jc2; } }
    if (Jc >= J) break;
    A += cl * lambda * JA / norm; B += cl * lambda * JB / norm;
    // if (dump) printf("   Lambda = %.5f, new A = %.5f, new B = %.5f, new J = %.5f\n", lambda, A.Val, B.Val, Jc);
  }
  A = bestA; B = bestB;
}

//////////////////////////////////////////////////////////////////////
// Useful stuff (hopefuly)
#ifdef SCALAPACK
template<class Size>
void TLAMisc::Sort(TVec<TFlt, Size> & Vec, TVec<Size, Size>& index, const TBool& decrease) {
	if (index.Empty()){
		TLAMisc::FillRange(Vec.Len(), index);
	}
	char* id = decrease ? "D" : "I";
	int n = Vec.Len();
	int info;
	dlasrt2(id, &n, &Vec[0].Val, &index[0].Val, &info);
	//dlasrt2(id, n, d, key, info)
}
#endif

//int TLAMisc::SumVec(const TIntV& Vec) {
//    const int Len = Vec.Len();
//    int res = 0;
//    for (int i = 0; i < Len; i++)
//        res += Vec[i];
//    return res;
//}
//
//double TLAMisc::SumVec(const TFltV& Vec) {
//    const int Len = Vec.Len();
//    double res = 0.0;
//    for (int i = 0; i < Len; i++)
//        res += Vec[i];
//    return res;
//}

///////////////////////////////////////////////////////////////////////
// TVector
TVector::TVector(const bool& _IsColVector):
		IsColVector(_IsColVector),
		Vec() {}

TVector::TVector(const int& Dim, const bool _IsColVector):
		IsColVector(_IsColVector),
		Vec(Dim) {}

TVector::TVector(const TFltV& Vect, const bool _IsColVector):
		IsColVector(_IsColVector),
		Vec(Vect) {}

TVector::TVector(const TIntV& Vect, const bool _IsColVector):
		IsColVector(_IsColVector),
		Vec(Vect.Len()) {

	for (int i = 0; i < Vec.Len(); i++) {
		Vec[i] = Vect[i];
	}
}

TVector::TVector(const TFullMatrix& Mat):
		IsColVector(Mat.GetRows() > 1),
		Vec(TMath::Mx(Mat.GetRows(), Mat.GetCols())) {
	EAssertR(Mat.GetRows() == 1 || Mat.GetCols() == 1, "Cannot create a vector from matrix that is not a vector!");
	if (Mat.GetRows() == 1) {
		for (int ColIdx = 0; ColIdx < Mat.GetCols(); ColIdx++) {
			Vec[ColIdx] = Mat(0, ColIdx);
		}
	} else {
		for (int RowIdx = 0; RowIdx < Mat.GetRows(); RowIdx++) {
			Vec[RowIdx] = Mat(RowIdx, 0);
		}
	}
}

TVector::TVector(const TVector& Vector) {
	IsColVector = Vector.IsColVector;
	Vec = Vector.Vec;
}

#ifdef GLib_CPP11
// move constructor
TVector::TVector(const TVector&& Vector) {
	IsColVector = Vector.IsColVector;
	Vec = std::move(Vector.Vec);
}
#endif 

TVector& TVector::operator=(TVector Vector) {
	std::swap(IsColVector, Vector.IsColVector);
	std::swap(Vec, Vector.Vec);
	return *this;
}

TVector TVector::Init(const int& Dim, const bool _IsColVect = true) {
	return TVector(Dim, _IsColVect);
}

TVector TVector::Ones(const int& Dim, const bool IsColVect) {
	TVector Res(Dim, IsColVect);
	for (int i = 0; i < Dim; i++) {
		Res[i] = 1;
	}
	return TVector(Res);
}

TVector TVector::Zeros(const int& Dim, const bool IsColVec) {
	return TVector(Dim, IsColVec);
}

TVector TVector::Range(const int& Start, const int& End, const bool IsColVect) {
	EAssert(Start < End);

	const int Len = End - Start;

	TVector Res(Len, IsColVect);
	for (int i = 0; i < Len; i++) {
		Res[i] = i + Start;
	}

	return Res;
}

TVector TVector::Range(const int& End, const bool IsColVect) {
	return TVector::Range(0, End, IsColVect);
}

bool TVector::operator ==(const TVector& Vect) const {
	return IsColVector == Vect.IsColVector && Vec == Vect.Vec;
}

TVector TVector::GetT() const {
	TVector Res(*this);
	Res.Transpose();
	return Res;
}

TVector& TVector::Transpose() {
	 IsColVector = !IsColVector;
	 return *this;
}

double TVector::DotProduct(const TFltV& y) const {
	EAssert(Len() == y.Len());
	return TLinAlg::DotProduct(Vec, y);
}

double TVector::DotProduct(const TVector& y) const {
	EAssert(Len() == y.Len() && IsRowVec() && y.IsColVec());
	return DotProduct(y.Vec);
}

TFullMatrix TVector::operator *(const TVector& y) const {
	EAssertR(IsColVec() != y.IsColVec(), " TVector::operator*(TVector): invalid dimensions!");

	if (IsRowVec()) {
		// dot product
		const double& Dot = DotProduct(y);
		TFullMatrix Res(1,1);
		Res.Set(Dot,0,0);
		return Res;
	} else {
		// outer product
		TFullMatrix Res(Len(), y.Len());
		TLinAlg::OuterProduct(Vec, y.Vec, *Res.Mat);
		return Res;
	}
}

TVector TVector::operator *(const TFullMatrix& Mat) const {
	EAssertR(IsRowVec() && Len() == Mat.GetRows(), "TVector::operator*(TFullMatrix&): Invalid dimensions!");

	TVector Res(Mat.GetCols(), false);
	Mat.MultiplyT(Vec, Res.Vec);

	return Res;
}

TVector TVector::operator *(const double& Lambda) const {
	TVector Res(Vec.Len(), IsColVec());
	TLinAlg::MultiplyScalar(Lambda, Vec, Res.Vec);
	return Res;
}

TVector& TVector::operator *=(const double& Lambda) {
	TLinAlg::MultiplyScalar(Lambda, Vec, Vec);
	return *this;
}

TVector TVector::operator /(const double& Lambda) const {
	return operator *(1/Lambda);
}

TVector& TVector::operator /=(const double& Lambda) {
	return operator *=(1/Lambda);
}

TVector TVector::MulT(const TFullMatrix& B) const {
	EAssertR(Len() == B.GetRows(), "TVector::MulT: Dimension should equal the number of rows in B!");

	TVector Res(B.GetCols(), false);
	TLinAlg::MultiplyT(*B.Mat, Vec, Res.Vec);

	return Res;
}

TVector TVector::operator +(const TVector& y) const {
	EAssertR(Len() == y.Len() && IsColVec() == y.IsColVec(), "TVector::operator +(TVector& y): Invalid dimensions!");

	TVector Res(Len(), IsColVec());
	TLinAlg::LinComb(1.0, Vec, 1.0, y.Vec, Res.Vec);
	return Res;
}

TVector& TVector::operator +=(const TVector& y) {
	EAssertR(Len() == y.Len() && IsColVec() == y.IsColVec(), "TVector::operator +=(TVector&): Invalid dimensions!");
	TLinAlg::LinComb(1.0, Vec, 1.0, y.Vec, Vec);
	return *this;
}

TVector TVector::operator -(const TVector& y) const {
	EAssertR(Len() == y.Len() && IsColVec() == y.IsColVec(), "TVector::operator -(TVector& y): Invalid dimensions!");

	TVector Res(Len(), IsColVec());
	TLinAlg::LinComb(1.0, Vec, -1.0, y.Vec, Res.Vec);
	return Res;
}

double TVector::Norm() const {
	return TLinAlg::Norm(Vec);
}

double TVector::Norm2() const {
	return TLinAlg::Norm2(Vec);
}

double TVector::Sum() const {
	const int Dim = Len();

	double Sum = 0;
	for (int i = 0; i < Dim; i++) {
		Sum += Vec[i];
	}

	return Sum;
}

double TVector::EuclDist(const TVector& y) const {
	return TLinAlg::EuclDist(Vec, y.Vec);
}

TIntV TVector::GetIntVec() const {
	 const int Dim = Len();
	 TIntV Res(Dim);

	 for (int i = 0; i < Dim; i++) {
		 Res[i] = (int) Vec[i];
	 }

	 return Res;
}

double TVector::GetMaxVal() const {
	return GetMax().Val2;
}

int TVector::GetMaxIdx() const {
	return GetMax().Val1;
}

TIntFltPr TVector::GetMax() const {
	const int Dim = Len();

	double MaxVal = TFlt::Mn;
	int MaxIdx = 0;

	for (int i = 0; i < Dim; i++) {
		if (Vec[i] > MaxVal) {
			MaxVal = Vec[i];
			MaxIdx = i;
		}
	}

	return TIntFltPr(MaxIdx, MaxVal);
}

int TVector::GetMinIdx() const {
	const int Dim = Len();

	double MinVal = TFlt::Mx;
	int MinIdx = 0;

	for (int i = 0; i < Dim; i++) {
		if (Vec[i] < MinVal) {
			MinVal = Vec[i];
			MinIdx = i;
		}
	}

	return MinIdx;
}

///////////////////////////////////////////////////////////////////////
// Full-Matrix
TFullMatrix::TFullMatrix():
		TMatrix(),
		IsWrapper(false),
		Mat(new TFltVV(0,0)) {}

TFullMatrix::TFullMatrix(const int& Rows, const int& Cols):
	 	TMatrix(),
	 	IsWrapper(false),
	 	Mat(new TFltVV(Rows, Cols)) {}

TFullMatrix::TFullMatrix(TFltVV& _Mat, const bool _IsWrapper):
		TMatrix(),
		IsWrapper(_IsWrapper),
		Mat(_IsWrapper ? &_Mat : new TFltVV(_Mat)) {}

TFullMatrix::TFullMatrix(const TFltVV& _Mat):
		TMatrix(),
		IsWrapper(false),
		Mat(new TFltVV(_Mat)) {}

TFullMatrix::TFullMatrix(const TVector& Vec):
		TMatrix(),
		IsWrapper(false),
		Mat(new TFltVV(Vec.IsColVec() ? Vec.Len() : 1, Vec.IsRowVec() ? Vec.Len() : 1)) {

	if (Vec.IsColVec()) {
		for (int i = 0; i < Vec.Len(); i++) {
			Mat->PutXY(i,0,Vec[i]);
		}
	} else {
		for (int i = 0; i < Vec.Len(); i++) {
			Mat->PutXY(0,i,Vec[i]);
		}
	}
}

TFullMatrix::TFullMatrix(const TFullMatrix& Other):
		TMatrix(Other),
		IsWrapper(Other.IsWrapper),
		Mat(Other.IsWrapper ? Other.Mat : new TFltVV(*Other.Mat)) {}

#ifdef GLib_CPP11
TFullMatrix::TFullMatrix(TFullMatrix&& Other):
		TMatrix(Other),
		IsWrapper(std::move(Other.IsWrapper)),
		Mat(Other.Mat) {
	Other.Mat = nullptr;
}
#endif

TFullMatrix::TFullMatrix(TFltVV* _Mat):
		IsWrapper(false),
		Mat(_Mat) {}

TFullMatrix::~TFullMatrix() {
	Clr();
}

TFullMatrix& TFullMatrix::operator =(const TFullMatrix& Other) {
	TMatrix::operator =(Other);

	Clr();
	IsWrapper = Other.IsWrapper;
	Mat = IsWrapper ? Other.Mat : new TFltVV(*Other.Mat);

	return *this;
}

TFullMatrix& TFullMatrix::operator =(TFullMatrix&& Other) {
	TMatrix::operator =(Other);

	if (this != &Other) {
		std::swap(IsWrapper, Other.IsWrapper);
		std::swap(Mat, Other.Mat);
		Other.Clr();
	}

	return *this;
}

TFullMatrix TFullMatrix::Identity(const int& Dim) {
	TFullMatrix Mat(Dim, Dim);

    for (int i = 0; i < Dim; i++) {
    	Mat(i,i) = 1;
    }

    return Mat;
}

TFullMatrix TFullMatrix::RowMatrix(const TVec<TFltV>& Mat) {
	EAssertR(Mat.Len() > 0, "Input vector should have at least one row!");
	EAssertR(Mat[0].Len() > 0, "Input vector should have at least one column!");

	const int Rows = Mat.Len();
	const int Cols = Mat[0].Len();

	TFullMatrix Res(Rows, Cols);
	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
			Res.Mat->PutXY(RowIdx, ColIdx, Mat[RowIdx][ColIdx]);
		}
	}

	return Res;
}

TFullMatrix TFullMatrix::ColMatrix(const TVec<TFltV>& Mat) {
	EAssertR(Mat.Len() > 0, "Input vector should have at least one column!");
	EAssertR(Mat[0].Len() > 0, "Input vector should have at least one row!");

	const int Cols = Mat.Len();
	const int Rows = Mat[0].Len();

	TFullMatrix Res(Rows, Cols);
	for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
		for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
			Res.Mat->PutXY(RowIdx, ColIdx, Mat[ColIdx][RowIdx]);
		}
	}

	return Res;
}

TFullMatrix TFullMatrix::Diag(const TVector& Diag) {
	const int Dim = Diag.Len();

	TFullMatrix Result(Dim,Dim);
	for (int i = 0; i < Dim; i++) {
		Result(i,i) = Diag[i];
	}

	return Result;
}

void TFullMatrix::Clr() {
	if (!IsWrapper && Mat != nullptr) {
		delete Mat;
		Mat = nullptr;
	}
}

void TFullMatrix::PMultiply(const TFltVV& B, int ColId, TFltV& Result) const {
	TLinAlg::Multiply(*Mat, B, ColId, Result);
}

void TFullMatrix::PMultiply(const TFltV& Vec, TFltV& Result) const {
	TLinAlg::Multiply(*Mat, Vec, Result);
}

void TFullMatrix::PMultiply(const TFltVV& B, TFltVV& Result) const {
	TLinAlg::Multiply(*Mat, B, Result);
}

void TFullMatrix::PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const {
	FailR("TFullMatrix::PMultiplyT: Not implemented!!!");
}

void TFullMatrix::PMultiplyT(const TFltV& Vec, TFltV& Result) const {
	TLinAlg::MultiplyT(*Mat, Vec, Result);
}

void TFullMatrix::PMultiplyT(const TFltVV& B, TFltVV& Result) const {
	TLinAlg::MultiplyT(*Mat, B, Result);
}

void TFullMatrix::Transpose() {
	Mat->Transpose();
}

TFullMatrix TFullMatrix::GetT() const {
	TFullMatrix Res(*this);      // copy
	GetT(Res.GetMat());
	return Res;
}

void TFullMatrix::GetT(TFltVV& TransposedVV) const {
	TLinAlg::Transpose(GetMat(), TransposedVV);
}

TFullMatrix& TFullMatrix::AddCol(const TFltV& Col) {
	const int Rows = GetRows();
	const int LastColIdx = GetCols();

	EAssertR(Col.Len() == Rows, "TFullMatrix::AddCol: dimension mismatch!");

	Mat->AddYDim();
	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		Mat->PutXY(RowIdx, LastColIdx, Col[RowIdx]);
	}

	return *this;
}

TFullMatrix& TFullMatrix::AddCol(const TVector& Col) {
	return AddCol(Col.Vec);
}

TFullMatrix& TFullMatrix::AddCols(const TFullMatrix& ColMat) {
	EAssertR(GetRows() == ColMat.GetRows(), "Invalid dimensions when concatenating matrices!");

	const int Rows = GetRows();
	const int Cols = GetCols();
	const int NNewCols = ColMat.GetCols();

	Mat->AddYDim(NNewCols);
	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		for (int ColIdx = 0; ColIdx < NNewCols; ColIdx++) {
			Mat->PutXY(RowIdx, Cols + ColIdx, ColMat(RowIdx, ColIdx));
		}
	}

	return *this;
}

TFullMatrix& TFullMatrix::operator -=(const TFullMatrix& B) {
	EAssert(GetCols() == B.GetCols() && GetRows() == B.GetRows());

	TLinAlg::LinComb(1.0, *Mat, -1.0, B.GetMat(), *Mat);
	return *this;
}

TFullMatrix& TFullMatrix::operator +=(const TFullMatrix& B) {
	EAssert(GetCols() == B.GetCols() && GetRows() == B.GetRows());
	TLinAlg::LinComb(1.0, *Mat, 1.0, B.GetMat(), *Mat);
	return *this;
}

TFullMatrix TFullMatrix::operator +(const TFullMatrix& B) const {
	EAssert(GetCols() == B.GetCols() && GetRows() == B.GetRows());

	TFullMatrix Result(GetRows(), GetCols());
	TLinAlg::LinComb(1.0, *Mat, 1.0, B.GetMat(), *Result.Mat);

	return Result;
}

TFullMatrix TFullMatrix::operator -(const TFullMatrix& B) const {
	EAssert(GetCols() == B.GetCols() && GetRows() == B.GetRows());

	TFullMatrix Result(GetRows(), GetCols());
	TLinAlg::LinComb(1.0, *Mat, -1.0, B.GetMat(), *Result.Mat);

	return Result;
}

TFullMatrix TFullMatrix::operator *(const TFullMatrix& B) const {
	EAssert(GetCols() == B.GetRows());

	TFullMatrix Result(GetRows(), B.GetCols());
	Multiply(*B.Mat, *Result.Mat);

 	return Result;
}

TFullMatrix TFullMatrix::operator *(const TSparseColMatrix& B) const {
	EAssert(GetCols() == B.GetRows());

	TFullMatrix Result(GetRows(), B.GetCols());
	TLinAlg::Multiply(*Mat, B.ColSpVV, *Result.Mat);

	return Result;
}

TFullMatrix TFullMatrix::MulT(const TFullMatrix& B) const {
	return MulT(B.GetMat());
}

TFullMatrix TFullMatrix::MulT(const TFltVV& B) const {
	EAssert(GetRows() == B.GetRows());

	TFullMatrix Result(GetCols(), B.GetCols());
	MultiplyT(B, *Result.Mat);
	return Result;
}

TVector TFullMatrix::operator *(const TVector& x) const {
	EAssertR(x.IsColVec(), "x must be a column vector!");
	return operator *(x.Vec);
}

TVector TFullMatrix::operator *(const TFltV& y) const {
	TVector Res(GetRows());
	Multiply(y, Res.Vec);
	return Res;
}

TFullMatrix TFullMatrix::operator *(const double& Lambda) const {
	TFullMatrix Res(GetRows(), GetCols());
	TLinAlg::MultiplyScalar(Lambda, *Mat, *Res.Mat);

	return Res;
}

TFullMatrix TFullMatrix::operator /(const double& Lambda) const {
	return operator *(1.0/Lambda);
}

TVector TFullMatrix::GetRow(const int& RowIdx) const {
	EAssertR(RowIdx < GetRows(), "Row index should be smaller then the number of rows!");

	const int Cols = GetCols();

	TVector Res(Cols, false);
	for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
		Res[ColIdx] = Mat->At(RowIdx, ColIdx);
	}

	return Res;
}

TFullMatrix TFullMatrix::Pow(const int& k) const {
	EAssertR(k >= 0, "TFullMatrix::operator ^: Negative powers not implemented!");
	EAssertR(GetRows() == GetCols(), "TFullMatrix::operator ^: Can only compute powers of square matrices!");

	if (k == 0) { return TFullMatrix::Identity(GetRows()); }
	else if (k < 0) { return GetInverse()^(-k); }
	else {
		// we will compute the power using the binary algorithm
		// X <- A
		TFltVV* X = new TFltVV(*Mat);

		// temporary variables
		TFltVV* X1 = new TFltVV(GetRows(), GetCols());
		TFltVV* Temp;

		// do the work
		uint k1 = (uint) k;
		uint n = (uint) TMath::Log2(k);

		uint b;

		for (uint i = 1; i <= n; i++) {
			b = (k1 >> (n-i)) & 1;

			// X <- X*X
			TLinAlg::Multiply(*X, *X, *X1);

			// swap X and X1 so that X holds the content
			Temp = X1;
			X1 = X;
			X = Temp;
			if (b == 1) {
				// X <- X*A
				TLinAlg::Multiply(*X, *Mat, *X1);
				// swap X and X1 so that X holds the content
				Temp = X1;
				X1 = X;
				X = Temp;
			}
		}

		// delete the temporary variables and wrap the result
		delete X1;

		return TFullMatrix(X);
	}
}

TVector TFullMatrix::GetCol(const int& ColIdx) const {
	EAssertR(ColIdx < GetCols(), "Column index should be smaller then the number of columns!");

	const int Rows = GetRows();

	TVector Res(Rows, true);
	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		Res[RowIdx] = Mat->At(RowIdx, ColIdx);
	}

	return Res;
}

void TFullMatrix::SetRow(const int& RowIdx, const TVector& RowV) {
	EAssertR(RowV.IsRowVec(), "When setting a row the input vector should be a row vector!");
	EAssertR(RowV.Len() == GetCols(), "Dimension mismatch!");

	const int Cols = GetCols();

	for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
		Mat->At(RowIdx, ColIdx) = RowV[ColIdx];
	}
}

void TFullMatrix::SetCol(const int& ColIdx, const TVector& ColV) {
	EAssertR(ColV.IsColVec(), "When setting a column the input vector should be a column vector!");
	EAssertR(ColV.Len() == GetRows(), "Dimension mismatch!");

	const int Rows = GetRows();

	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		Mat->At(RowIdx, ColIdx) = ColV[RowIdx];
	}
}

double TFullMatrix::ColNorm(const int& ColIdx) const {
	return TLinAlg::Norm(*Mat, ColIdx);
}

double TFullMatrix::ColNorm2(const int& ColIdx) const {
	double Norm = ColNorm(ColIdx);
	return Norm * Norm;
}

TVector TFullMatrix::ColNormV() const {
	const int Cols = GetCols();

	TVector Res(Cols, false);
	for (int i = 0; i < Cols; i++) {
		Res[i] = ColNorm(i);
	}

	return Res;
}

TVector TFullMatrix::ColNorm2V() const {
	TVector Res = ColNormV();

	const int Cols = GetCols();
	for (int i = 0; i < Cols; i++) {
		Res.Vec[i] *= Res.Vec[i];
	}

	return Res;
}

double TFullMatrix::FromNorm() const {
	return TLinAlg::FrobNorm(*Mat);
}

double TFullMatrix::RowNormL1(const int& RowIdx) const {
	const int Cols = GetCols();

	double NormL1 = 0;
	for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
		NormL1 += TFlt::Abs(Mat->At(RowIdx, ColIdx));
	}

	return NormL1;
}

void TFullMatrix::NormalizeRowsL1() {
	const int Rows = GetRows();
	const int Cols = GetCols();

	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		const double Norm = RowNormL1(RowIdx);
		for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
			Mat->At(RowIdx, ColIdx) /= Norm;
		}
	}
}

double TFullMatrix::RowSum(const int& RowIdx) const {
	EAssertR(RowIdx < GetRows(), TStr::Fmt("Invalid row index: %d", RowIdx));

	const int NCols = GetCols();
	double Sum = 0;

	for (int i = 0; i < NCols; i++) {
		Sum += Mat->At(RowIdx, i);
	}

	return Sum;
}

TVector TFullMatrix::RowSumV() const {
	const int Rows = GetRows();

	TVector Res(Rows, true);
	for (int i = 0; i < Rows; i++) {
		Res.Vec[i] = RowSum(i);
	}

	return Res;
}

TVector TFullMatrix::GetColMinV() const {
	TVector Result;	TLinAlgSearch::GetColMinV(*Mat, Result.Vec);
	return Result;
}

TVector TFullMatrix::GetColMaxIdxV() const {
	TIntV IdxV;	TLinAlgSearch::GetColMaxIdxV(*Mat, IdxV);
	return TVector(IdxV, false);
}

TVector TFullMatrix::GetColMinIdxV() const {
	TIntV IdxV;	TLinAlgSearch::GetColMinIdxV(*Mat, IdxV);
	return TVector(IdxV, false);
}

TFullMatrix& TFullMatrix::CenterRows() {
	const int Rows = GetRows();
	const int Cols = GetCols();

	#pragma omp parallel for
	for (int RowIdx = 0; RowIdx < Rows; RowIdx++) {
		double RowMean = 0;
		for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
			RowMean += At(RowIdx, ColIdx);
		}
		RowMean /= Cols;
		for (int ColIdx = 0; ColIdx < Cols; ColIdx++) {
			At(RowIdx, ColIdx) -= RowMean;
		}
	}

	return *this;
}

TFullMatrix TFullMatrix::GetCenteredRows() const {
	return TFullMatrix(*this).CenterRows();
}

TTriple<TFullMatrix, TVector, TFullMatrix> TFullMatrix::Svd(const int& k) const {
	TTriple<TFullMatrix, TVector, TFullMatrix> Result;

	TLinAlg::ComputeThinSVD(*this, k, Result.Val1.GetMat(), Result.Val2.Vec, Result.Val3.GetMat());

	return Result;
}

TFullMatrix TFullMatrix::GetInverse() const {
	EAssertR(GetRows() == GetCols(), "Can only invert square matrices!");
	throw TExcept::New("TFullMatrix::GetInverse: Not implemented!");
}

bool TFullMatrix::HasNan() const {
	const int Cols = GetCols();
	const int Rows = GetRows();

	for (int RowN = 0; RowN < Rows; RowN++) {
		for (int ColN = 0; ColN < Cols; ColN++) {
			if (TFlt::IsNan(At(RowN, ColN))) {
				return true;
			}
		}
	}

	return false;
}

void TFullMatrix::Save(TSOut& SOut) const {
	EAssertR(!IsWrapper, "TFullMatrix::Save: Cannot save a wrapper!");
	TMatrix::Save(SOut);
	Mat->Save(SOut);
}

void TFullMatrix::Load(TSIn& SIn) {
	EAssertR(!IsWrapper, "TFullMatrix::Load: Cannot load a wrapper!");
	TMatrix::Load(SIn);
	Mat->Load(SIn);
	IsWrapper = false;
}
 
//#if defined(LAPACKE) && defined(EIGEN)
////no need to reserve memory for the matrices, all will be done internaly
////Set k to 500
////Tolerance ignored!
//int TLinAlg::ComputeThinSVD(const TMatrix& XYt, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters, const double Tol){
//	//TStructuredCovarianceMatrix XYt(rows, cols, SampleN, MeanX, MeanY, X, Y);
//	EAssert(k <= XYt.GetRows() && k <= XYt.GetCols());
//
//	const int its = Iters != -1 ? Iters : 2;
//	 const int m = XYt.GetRows();
//	 const int n = XYt.GetCols();
//	 int l = (int)((11 / 10.0) * k);
//	 //printf("l is %d\n", l);
//	 if ((its+1)*l >= MIN(m, n)){
//		 TFltVV XYtfull; XYtfull.Gen(m, n);
//		 TFltVV Identity; Identity.Gen(n, n);
//		 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
//		 typedef Eigen::Map<Mat> MatW;
//		 MatW IdentityWrapped(&Identity(0, 0).Val, n, n);
//		 IdentityWrapped.setIdentity(n, n);
//		 XYt.Multiply(Identity, XYtfull);
//		 TFltVV VT;
//		 TLinAlg::thinSVD(XYtfull, U, s, VT);
//	     V.Gen(VT.GetCols(), VT.GetRows());
//		 TLinAlg::Transpose(VT, V);
//	 }
//	 else{
//		 TTmStopWatch Time;
//		 if (m >= n){
//			 //H is used for intermediate result and should be of the size n times l!
//			 TFltVV H(n,l); TLAMisc::FillRnd(H);
//			 //TFltVV RSample; RSample.GenRandom(n, l);
//			 TFltVV F, F0, F1, F2; F0.Gen(m, l); F1.Gen(m, l); F2.Gen(m, l);
//			 //Time.Start();
//			 //printf("Start Multiplying with XYt'*XYt twice\n");
//			 //Size of F0 should be m x l
//			 XYt.Multiply(H, F0);
//			 //H is used for intermediate result and should be of the size n times l!
//			 XYt.MultiplyT(F0, H);
//			 //H is used for intermediate result and should be of the size n times l!
//			 XYt.Multiply(H, F1);
//			 //H is used for intermediate result and should be of the size n times l!
//			 XYt.MultiplyT(F1, H); XYt.Multiply(H, F2);
//			 //Time.Stop("Finish Multiplying with XYt'*XYt twice\n");
//			 //Free the memory
//			 H.Clr();
//			 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
//			 F.Gen(m, (its + 1) * l);//its+1
//			 typedef Eigen::Map<Mat> MatW;
//			 MatW FWrapped(&F(0, 0).Val, m, (its + 1) * l); MatW F0Wrapped(&F0(0, 0).Val, m, l); MatW F1Wrapped(&F1(0, 0).Val, m, l); MatW F2Wrapped(&F2(0, 0).Val, m, l);
//			 //printf("Started to join the memory\n");
//			 //Time.Start();
//			 FWrapped << F0Wrapped, F1Wrapped, F2Wrapped;
//			 //Time.Stop("Finished joining the memory\n");
//			 //Free the memory
//			 F0.Clr(); F1.Clr(); F2.Clr();
//			 //Do QR in place at the end F becomes Q
//			 //printf("Orthogonal basis in place computation\n");
//			 //Time.Start();
//			 TLinAlg::QRbasis(F);
//			 //Time.Stop("Orthogonal basis in place computation took: ");
//			 //Is F still valid
//			 TFltVV FF; FF.Gen(n, (its + 1) * l);
//			 //printf("Start Multiplying with XYt'\n");
//			 //Time.Start();
//			 XYt.MultiplyT(F, FF);
//			 //Time.Stop("Multiplying with XYt' took: ");
//			 TFltVV UU, VT;
//			 //printf("Size of matrix FF: %d\n", FF.GetCols());
//			 //printf("Computation of thin SVD\n");
//			 //Time.Start();
//			 TFltVV FFT; FFT.Gen((its + 1) * l, n);
//			 TLinAlg::Transpose(FF, FFT);
//			 TLinAlg::thinSVD(FFT, UU, s, VT);
//			 //Time.Stop("Computation of thin SVD took:");
//			 //printf("UU (%d, %d)\n", UU.GetRows(), UU.GetCols());
//			 //Copy and save U
//			 U.Gen(m, (its + 1)*l);
//			 TLinAlg::Multiply(F, UU, U);
//			 V.Gen(VT.GetCols(), VT.GetRows());
//			 TLinAlg::Transpose(VT, V);
//			 //U = Q*U2;
//		 }
//		 else{
//			 //H is used for intermediate result and should be of the size m times l!
//			 TFltVV H(m,l); TLAMisc::FillRnd(H);
//			 //TFltVV RSample; RSample.GenRandom(n, l);
//			 TFltVV F, F0, F1, F2; F0.Gen(n, l); F1.Gen(n, l); F2.Gen(n, l);
//			 //printf("Star Multiplying with XYt'*XYt\n");
//			 //Size of F0 should be m x l
//			 XYt.MultiplyT(H, F0);
//			 //printf("Finish Multiplying with XYt'*Xyt");
//			 //H is used for intermediate result and should be of the size m times l!
//			 XYt.Multiply(F0, H);
//			 //H is used for intermediate result and should be of the size m times l!
//			 XYt.MultiplyT(H, F1);
//			 //H is used for intermediate result and should be of the size m times l!
//			 XYt.Multiply(F1, H); XYt.MultiplyT(H, F2);
//			 printf("Finish Multiplying with XYt\n");
//			 //Free the memory
//			 H.Clr();
//			 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
//			 F.Gen(n, (its + 1) * l); TLAMisc::FillRnd(F);//its+1
//
//			 typedef Eigen::Map<Mat> MatW;
//			 MatW FWrapped(&F(0, 0).Val, n, (its + 1) * l); MatW F0Wrapped(&F0(0, 0).Val, n, l); MatW F1Wrapped(&F1(0, 0).Val, n, l); MatW F2Wrapped(&F2(0, 0).Val, n, l);
//			 //printf("Started to join the memory");
//			 FWrapped << F0Wrapped, F1Wrapped, F2Wrapped;
//			 //printf("Finished join the memory");
//			 //Free the memory
//			 F0.Clr(); F1.Clr(); F2.Clr();
//			 //Do QR in place at the end F becomes Q
//			 TLinAlg::QRbasis(F);
//			 //printf("QR finsihed\n");
//			 //Is F still valid
//			 TFltVV FF; FF.Gen(m, (its + 1) * l);
//			 XYt.Multiply(F, FF);
//			 TFltVV VV, VVT;
//			 //printf("Size of matrix FF: %d\n", FF.GetCols());
//			 TLinAlg::thinSVD(FF, U, s, VVT);
//			 VV.Gen(VVT.GetCols(), VVT.GetRows());
//			 TLinAlg::Transpose(VVT, VV);
//			 V.Gen(n, (its + 1)*l);
//			 //printf("Almost done\n");
//			 //printf("F sizes: (%d, %d), VV sizes (%d, %d), V sizes (%d, %d)", F.GetRows(), F.GetCols(), VV.GetRows(), VV.GetCols(), V.GetRows(), V.GetCols());
//			 TLinAlg::Multiply(F, VV, V);
//			 //V = Q*V2;
//		 }
//	 }
//	 //Clip to only top k components
//	 int kk = MIN(k, MIN(m, n));
//	 TFltVV UU, VV; UU.Gen(U.GetRows(), kk); VV.Gen(V.GetRows(), kk);
//	 TFltV ss; ss.Gen(kk);
//	 for (int j = 0; j < kk; j++){
//		 ss[j] = s[j];
//		 for (int i = 0; i < U.GetRows(); i++){
//			 UU(i, j) = U(i, j);
//		 }
//		 for (int i = 0; i < V.GetRows(); i++){
//			 VV(i, j) = V(i, j);
//		 }
//	 }
//	 U = UU;
//	 V = VV;
//	 s = ss;
//	 return kk;
// }
//#else

void TLinAlg::SVDSolve(const TFltVV& A, TFltV& x, const TFltV& b,
		const double& EpsSing) {
	Assert(A.GetRows() == b.Len());

	// data used for solution
	int NumOfRows_Matrix = A.GetRows();
	int NumOfCols_Matrix = A.GetCols();

	// generating the SVD factorization
	TFltVV U, VT, M = A;
	TFltV Sing;
	ComputeSVD(M, U, Sing, VT);

	// generating temporary solution
	x.Gen(NumOfCols_Matrix);
	TLinAlgTransform::FillZero(x);
	TFltV ui; ui.Gen(U.GetRows());
	TFltV vi; vi.Gen(VT.GetCols());

	double Scalar;
	int i = 0;
	while (i < MIN(NumOfRows_Matrix, NumOfCols_Matrix) && Sing[i].Val > EpsSing*Sing[0]) {
		U.GetCol(i, ui);
		VT.GetRow(i, vi);
		Scalar = TLinAlg::DotProduct(ui, b) / Sing[i].Val;
		TLinAlg::AddVec(Scalar, vi, x);
		i++;
	}
}

void TLinAlg::ComputeSVD(const TFltVV& A, TFltVV& U, TFltV& Sing,
		TFltVV& VT) {
#ifdef LAPACKE
	MKLfunctions::SVDFactorization(A, U, Sing, VT);
#else
	// TODO optimize this part
	if (VT.GetRows() != A.GetCols() || VT.GetCols() != A.GetCols()) {
		VT.Gen(A.GetCols(), A.GetCols());
	}

	TSvd SVD;
	SVD.Svd(A, U, Sing, VT);

	// transpose V
	VT.Transpose();
#endif
}


int TLinAlg::ComputeThinSVD(const TMatrix& XYt, const int& k, TFltVV& U, TFltV& s, TFltVV& V, const int Iters, const double Tol){
#if defined(LAPACKE) && defined(EIGEN)
	//no need to reserve memory for the matrices, all will be done internaly
	//Set k to 500
	//Tolerance ignored!

	EAssert(k <= XYt.GetRows() && k <= XYt.GetCols());

	const int its = Iters != -1 ? Iters : 2;
	 const int m = XYt.GetRows();
	 const int n = XYt.GetCols();
	 int l = (int)((11 / 10.0) * k);
	 //printf("l is %d\n", l);
	 if ((its+1)*l >= MIN(m, n)){
		 TFltVV XYtfull; XYtfull.Gen(m, n);
		 TFltVV Identity; Identity.Gen(n, n);
		 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
		 typedef Eigen::Map<Mat> MatW;
		 MatW IdentityWrapped(&Identity(0, 0).Val, n, n);
		 IdentityWrapped.setIdentity(n, n);
		 XYt.Multiply(Identity, XYtfull);
		 TFltVV VT;
		 TLinAlg::ThinSVD(XYtfull, U, s, VT);
		 V.Gen(VT.GetCols(), VT.GetRows());
		 TLinAlg::Transpose(VT, V);
	 }
	 else{
		 TTmStopWatch Time;
		 if (m >= n){

			 //H is used for intermediate result and should be of the size n times l!			 
			 TFltVV H(n,l); TLinAlgTransform::FillRnd(H);
			 //TFltVV RSample; RSample.GenRandom(n, l);
			 TFltVV F, F0, F1, F2; F0.Gen(m, l); F1.Gen(m, l); F2.Gen(m, l);
			 //Time.Start();
			 //printf("Start Multiplying with XYt'*XYt twice\n");
			 //Size of F0 should be m x l
			 XYt.Multiply(H, F0);
			 //H is used for intermediate result and should be of the size n times l!
			 XYt.MultiplyT(F0, H);
			 //H is used for intermediate result and should be of the size n times l!
			 XYt.Multiply(H, F1);
			 //H is used for intermediate result and should be of the size n times l!
			 XYt.MultiplyT(F1, H); XYt.Multiply(H, F2);
			 //Time.Stop("Finish Multiplying with XYt'*XYt twice\n");
			 //Free the memory
			 H.Clr();
			 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
			 F.Gen(m, (its + 1) * l);//its+1
			 typedef Eigen::Map<Mat> MatW;
			 MatW FWrapped(&F(0, 0).Val, m, (its + 1) * l); MatW F0Wrapped(&F0(0, 0).Val, m, l); MatW F1Wrapped(&F1(0, 0).Val, m, l); MatW F2Wrapped(&F2(0, 0).Val, m, l);
			 //printf("Started to join the memory\n");
			 //Time.Start();
			 FWrapped << F0Wrapped, F1Wrapped, F2Wrapped;
			 //Time.Stop("Finished joining the memory\n");
			 //Free the memory
			 F0.Clr(); F1.Clr(); F2.Clr();
			 //Do QR in place at the end F becomes Q
			 //printf("Orthogonal basis in place computation\n");
			 //Time.Start();
			 TLinAlg::QRbasis(F);
			 //Time.Stop("Orthogonal basis in place computation took: ");
			 //Is F still valid
			 TFltVV FF; FF.Gen(n, (its + 1) * l);
			 //printf("Start Multiplying with XYt'\n");
			 //Time.Start();
			 XYt.MultiplyT(F, FF);
			 //Time.Stop("Multiplying with XYt' took: ");
			 TFltVV UU, VT;
			 //printf("Size of matrix FF: %d\n", FF.GetCols());
			 //printf("Computation of thin SVD\n");
			 //Time.Start();
			 TFltVV FFT; FFT.Gen((its + 1) * l, n);
			 TLinAlg::Transpose(FF, FFT);
			 TLinAlg::ThinSVD(FFT, UU, s, VT);
			 //Time.Stop("Computation of thin SVD took:");
			 //printf("UU (%d, %d)\n", UU.GetRows(), UU.GetCols());
			 //Copy and save U
			 U.Gen(m, (its + 1)*l);
			 TLinAlg::Multiply(F, UU, U);
			 V.Gen(VT.GetCols(), VT.GetRows());
			 TLinAlg::Transpose(VT, V);
			 //U = Q*U2;
		 }
		 else{
			 //H is used for intermediate result and should be of the size m times l!
			 TFltVV H(m,l); TLinAlgTransform::FillRnd(H);
			 //TFltVV RSample; RSample.GenRandom(n, l);
			 TFltVV F, F0, F1, F2; F0.Gen(n, l); F1.Gen(n, l); F2.Gen(n, l);
			 //printf("Star Multiplying with XYt'*XYt\n");
			 //Size of F0 should be m x l
			 XYt.MultiplyT(H, F0);
			 //printf("Finish Multiplying with XYt'*Xyt");
			 //H is used for intermediate result and should be of the size m times l!
			 XYt.Multiply(F0, H);
			 //H is used for intermediate result and should be of the size m times l!
			 XYt.MultiplyT(H, F1);
			 //H is used for intermediate result and should be of the size m times l!
			 XYt.Multiply(F1, H); XYt.MultiplyT(H, F2);
			 printf("Finish Multiplying with XYt\n");
			 //Free the memory
			 H.Clr();
			 typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Mat;
			 F.Gen(n, (its + 1) * l); TLinAlgTransform::FillRnd(F);//its+1

			 typedef Eigen::Map<Mat> MatW;
			 MatW FWrapped(&F(0, 0).Val, n, (its + 1) * l); MatW F0Wrapped(&F0(0, 0).Val, n, l); MatW F1Wrapped(&F1(0, 0).Val, n, l); MatW F2Wrapped(&F2(0, 0).Val, n, l);
			 //printf("Started to join the memory");
			 FWrapped << F0Wrapped, F1Wrapped, F2Wrapped;
			 //printf("Finished join the memory");
			 //Free the memory
			 F0.Clr(); F1.Clr(); F2.Clr();
			 //Do QR in place at the end F becomes Q
			 TLinAlg::QRbasis(F);
			 //printf("QR finsihed\n");
			 //Is F still valid
			 TFltVV FF; FF.Gen(m, (its + 1) * l);
			 XYt.Multiply(F, FF);
			 TFltVV VV, VVT;
			 //printf("Size of matrix FF: %d\n", FF.GetCols());
			 TLinAlg::ThinSVD(FF, U, s, VVT);
			 VV.Gen(VVT.GetCols(), VVT.GetRows());
			 TLinAlg::Transpose(VVT, VV);
			 V.Gen(n, (its + 1)*l);
			 //printf("Almost done\n");
			 //printf("F sizes: (%d, %d), VV sizes (%d, %d), V sizes (%d, %d)", F.GetRows(), F.GetCols(), VV.GetRows(), VV.GetCols(), V.GetRows(), V.GetCols());
			 TLinAlg::Multiply(F, VV, V);
			 //V = Q*V2;
		 }
	 }
	 //Clip to only top k components
	 int kk = MIN(k, MIN(m, n));
	 TFltVV UU, VV; UU.Gen(U.GetRows(), kk); VV.Gen(V.GetRows(), kk);
	 TFltV ss; ss.Gen(kk);
	 for (int j = 0; j < kk; j++){
		 ss[j] = s[j];
		 for (int i = 0; i < U.GetRows(); i++){
			 UU(i, j) = U(i, j);
		 }
		 for (int i = 0; i < V.GetRows(); i++){
			 VV(i, j) = V(i, j);
		 }
	 }
	 U = UU;
	 V = VV;
	 s = ss;
	 return kk;
#else
	TSparseSVD::OrtoIterSVD(XYt, k, s, U, V, Iters, Tol);
	return k;
#endif
}
//#endif

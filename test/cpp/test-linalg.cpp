/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <base.h>
#include <mine.h>
#include <qminer.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

constexpr double Tol = 1e-8;

void InitFltVV(TFltVV& FltVV) {
    TRnd Rnd;
    for (int RowN = 0; RowN < FltVV.GetRows(); RowN++) {
        for (int ColN = 0; ColN < FltVV.GetCols(); ColN++) {
            FltVV(RowN, ColN) = Rnd.GetUniDev();
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
TEST(TLinAlg, OperatorsSubtract) {
    int Dim = 4;

    TFltVV X {Dim, Dim};
    TFltVV Y {Dim, Dim};
    TFltVV BadDim {Dim, Dim+1};

    const TFltVV Z = X - Y;
    for (int RowN = 0; RowN < Dim; RowN++) {
        for (int ColN = 0; ColN < Dim; ColN++) {
            ASSERT_NEAR(Z(RowN, ColN), X(RowN, ColN) - Y(RowN, ColN), Tol);
        }
    }

    ASSERT_ANY_THROW(X - BadDim);
}

TEST(TLinAlg, OperatorsMultiply) {
    TIntPr Dim1 {3, 4};
    TIntPr Dim2 {4, 2};
    TIntPr BadDim {7, 4};

    TFltVV X {Dim1.Val1, Dim1.Val2};    InitFltVV(X);
    TFltVV Y {Dim2.Val1, Dim2.Val2};    InitFltVV(Y);
    const double k = 3;

    const TFltVV Z1 = X * Y;
    TFltVV ExpectedZ1;  TLinAlg::Multiply(X, Y, ExpectedZ1);

    ASSERT_NEAR(TLinAlg::FrobNorm(Z1 - ExpectedZ1), 0, Tol);

    const TFltVV Z2 = X*k;
    TFltVV ExpectedZ2;  TLinAlg::MultiplyScalar(k, X, ExpectedZ2);

    ASSERT_NEAR(TLinAlg::FrobNorm(Z2 - ExpectedZ2), 0, Tol);
    ASSERT_ANY_THROW(X*TFltVV(BadDim.Val1, BadDim.Val2));
}

TEST(TLinAlg, OperatorsDivide) {
    const int Dim {5};

    const TFltVV FltVV {Dim, Dim};
    const TFltV FltV {Dim, Dim};
    const double k {3.0};

    const TFltVV DivVV = FltVV / k;
    const TFltV DivV = FltV / k;

    for (int RowN = 0; RowN < Dim; RowN++) {
        for (int ColN = 0; ColN < Dim; ColN++) {
            ASSERT_NEAR(DivVV(RowN, ColN), FltVV(RowN, ColN) / k, Tol);
        }
        ASSERT_NEAR(DivV[RowN], FltV[RowN] / k, Tol);
    }
}

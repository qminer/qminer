#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

constexpr double Tol = 1e-8;

void InitFltVV(TFltVV& FltVV) {
    TRnd Rnd;
    for (int RowN = 0; RowN < FltVV.GetRows(); RowN++) {
        for (int ColN = 0; ColN < FltVV.GetCols(); ColN++) {
            FltVV(RowN, ColN) = Rnd.GetUniDev();
        }
    }
}

void InitFltV(TFltV& FltV) {
    TRnd Rnd;
    for (int ValN = 0; ValN < FltV.Len(); ValN++) {
        FltV[ValN] = Rnd.GetUniDev();
    }
}

TEST(TLinAlgOperatorsSubtract) {
    int Dim{ 4 };

    TFltVV X{ Dim, Dim };
    TFltVV Y{ Dim, Dim };
    TFltVV BadDim{ Dim, Dim + 1 };

    const TFltVV Z = X - Y;
    for (int RowN = 0; RowN < Dim; RowN++) {
        for (int ColN = 0; ColN < Dim; ColN++) {
            ASSERT_NEAR(Z(RowN, ColN), X(RowN, ColN) - Y(RowN, ColN), Tol);
        }
    }

    ASSERT_ANY_THROW(X - BadDim);
}

// DISABLED: Test hangs in destructor on macOS with C++17 - investigating memory management issue
#if 0
TEST(TLinAlg, OperatorsMultiply) {
    // MATRICES
    TIntPr Dim1 {3, 4};
    TIntPr Dim2 {4, 2};
    TIntPr BadDim {7, 4};

    TFltVV X {Dim1.Val1, Dim1.Val2};    InitFltVV(X);
    TFltVV Y {Dim2.Val1, Dim2.Val2};    InitFltVV(Y);
    const double k {3};

    const TFltVV Z1 = X * Y;
    TFltVV ExpectedZ1;  TLinAlg::Multiply(X, Y, ExpectedZ1);

    ASSERT_NEAR(TLinAlg::FrobNorm(Z1 - ExpectedZ1), 0, Tol);

    const TFltVV Z2 = X*k;
    TFltVV ExpectedZ2;  TLinAlg::MultiplyScalar(k, X, ExpectedZ2);

    ASSERT_NEAR(TLinAlg::FrobNorm(Z2 - ExpectedZ2), 0, Tol);
    ASSERT_ANY_THROW(X*TFltVV(BadDim.Val1, BadDim.Val2));


    // VECTORS
    const int VecDim {5};
    TFltV Vec1 {VecDim, VecDim};    InitFltV(Vec1);
    TFltV Vec2 {VecDim, VecDim};    InitFltV(Vec2);
    TFltV BadVec {VecDim+1, VecDim+1};

    ASSERT_NEAR(Vec1 * Vec2, TLinAlg::DotProduct(Vec1, Vec2), Tol);
    ASSERT_ANY_THROW(Vec1 * BadVec);
}
#endif

TEST(TLinAlgOperatorsDivide) {
    const int Dim{ 5 };

    const TFltVV FltVV{ Dim, Dim };
    const TFltV FltV{ Dim, Dim };
    const double k{ 3.0 };

    const TFltVV DivVV = FltVV / k;
    const TFltV DivV = FltV / k;

    for (int RowN = 0; RowN < Dim; RowN++) {
        for (int ColN = 0; ColN < Dim; ColN++) {
            ASSERT_NEAR(DivVV(RowN, ColN), FltVV(RowN, ColN) / k, Tol);
        }
        ASSERT_NEAR(DivV[RowN], FltV[RowN] / k, Tol);
    }
}

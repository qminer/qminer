#include <base.h>
#include <mine.h>
#include "microtest.h"

TEST(Tuples) {

    TIntPr IntPair1{ 3, 4 };
    TIntPr IntPair2 = { 3, 4 };
    TFltPr FltPr1{ 2.0, 3.0 };
    TFltPr FltPr2 = { 2.0, 3.0 };

    ASSERT_EQ(IntPair1.Val1, 3);
    ASSERT_EQ(IntPair1.Val2, 4);
    ASSERT_EQ(IntPair2.Val1, 3);
    ASSERT_EQ(IntPair2.Val2, 4);
    ASSERT_EQ(FltPr1.Val1.Val, 2.0);
    ASSERT_EQ(FltPr1.Val2.Val, 3.0);
    ASSERT_EQ(FltPr2.Val1.Val, 2.0);
    ASSERT_EQ(FltPr2.Val2.Val, 3.0);
}
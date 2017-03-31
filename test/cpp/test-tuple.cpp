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

///////////////////////////////////////////////////////////////////////////////
TEST(Tuple, TPair) {
    TIntPr IntPair1 {3, 4};
    TIntPr IntPair2 = {3, 4};
    TFltPr FltPr1 {2.0, 3.0};
    TFltPr FltPr2 = {2.0, 3.0};

    ASSERT_EQ(IntPair1.Val1, 3);
    ASSERT_EQ(IntPair1.Val2, 4);
    ASSERT_EQ(IntPair2.Val1, 3);
    ASSERT_EQ(IntPair2.Val2, 4);
    ASSERT_EQ(FltPr1.Val1, 2.0);
    ASSERT_EQ(FltPr1.Val2, 3.0);
    ASSERT_EQ(FltPr2.Val1, 2.0);
    ASSERT_EQ(FltPr2.Val2, 3.0);
}

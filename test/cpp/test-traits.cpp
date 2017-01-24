/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <base.h>

///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

#ifdef GLib_CPP11

// test that glib numeric types have standard layouts
TEST(is_standard_layout, TNum) {
    // integers
    ASSERT_TRUE(std::is_standard_layout<TInt>::value);
    ASSERT_TRUE(std::is_standard_layout<TUInt>::value);
    ASSERT_TRUE(std::is_standard_layout<TUSInt>::value);
    ASSERT_TRUE(std::is_standard_layout<TUInt64>::value);
    // real numbers
    ASSERT_TRUE(std::is_standard_layout<TFlt>::value);
}

// characters
TEST(is_standard_layout, TCh) {
    ASSERT_TRUE(std::is_standard_layout<TCh>::value);
    ASSERT_TRUE(std::is_standard_layout<TUCh>::value);
}

// boolean
TEST(is_standard_layout, TBool) {
    ASSERT_TRUE(std::is_standard_layout<TBool>::value);
}

#endif

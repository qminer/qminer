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

TEST(is_container, TVec) {
    ASSERT_TRUE(is_container<TIntV>::value);
    ASSERT_TRUE(is_container<TStrV>::value);
    ASSERT_TRUE(is_container<TStrPrV>::value);
}

TEST(is_container, THash) {
    ASSERT_TRUE(is_container<TIntH>::value);
    ASSERT_TRUE(is_container<TIntStrH>::value);
    ASSERT_TRUE(is_container<TIntFltH>::value);
}

// test that glib numeric types have standard layouts
TEST(type_traits, TNum) {
    ASSERT_TRUE(is_shallow<TInt>::value);
    ASSERT_TRUE(is_shallow<TUInt>::value);
    ASSERT_TRUE(is_shallow<TFlt>::value);
    ASSERT_TRUE(is_shallow<TUInt64>::value);
    ASSERT_TRUE(is_shallow<TUSInt>::value);
}

// characters
TEST(type_traits, TCh) {
    ASSERT_TRUE(is_shallow<TCh>::value);
    ASSERT_TRUE(is_shallow<TUCh>::value);
}

// boolean
TEST(type_traits, TBool) {
    ASSERT_TRUE(is_shallow<TBool>::value);
}

TEST(type_traits, TStr) {
    ASSERT_FALSE(is_shallow<TStr>::value);
}

/* TEST(type_traits, TPair) { */
/*     ASSERT_TRUE(is_shallow<TIntPr>::value); */
/*     ASSERT_FALSE(is_shallow<TIntStrPr>::value); */
/* } */

#endif

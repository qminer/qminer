/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <base.h>
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

TEST(TDir, Exists) {
    EXPECT_TRUE(TDir::Exists("./files"));
    EXPECT_FALSE(TDir::Exists("./xyz"));
}

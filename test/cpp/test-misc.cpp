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

TEST(TMath, FloorLog2_uint) {
    EXPECT_EQ(TMath::FloorLog2((uint)1), 0);
    EXPECT_EQ(TMath::FloorLog2((uint)2), 1);
    EXPECT_EQ(TMath::FloorLog2((uint)3), 1);
    EXPECT_EQ(TMath::FloorLog2((uint)4), 2);
    EXPECT_EQ(TMath::FloorLog2((uint)5), 2);
    EXPECT_EQ(TMath::FloorLog2((uint)6), 2);
    EXPECT_EQ(TMath::FloorLog2((uint)7), 2);
    EXPECT_EQ(TMath::FloorLog2((uint)8), 3);
    EXPECT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18) - 1), 17);
    EXPECT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18)), 18);
    EXPECT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18) + 1), 18);
    EXPECT_EQ(TMath::FloorLog2((uint)TMath::Pow2(31)), 31);
    EXPECT_EQ(TMath::FloorLog2((uint)TMath::Pow2(32) - 1), 31);
}

TEST(TMath, FloorLog2_uint64) {
    EXPECT_EQ(TMath::FloorLog2((uint64)1), 0);
    EXPECT_EQ(TMath::FloorLog2((uint64)2), 1);
    EXPECT_EQ(TMath::FloorLog2((uint64)3), 1);
    EXPECT_EQ(TMath::FloorLog2((uint64)4), 2);
    EXPECT_EQ(TMath::FloorLog2((uint64)5), 2);
    EXPECT_EQ(TMath::FloorLog2((uint64)6), 2);
    EXPECT_EQ(TMath::FloorLog2((uint64)7), 2);
    EXPECT_EQ(TMath::FloorLog2((uint64)8), 3);
    EXPECT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32) - 1), 31);
    EXPECT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32)), 32);
    EXPECT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32) + 1), 32);
    EXPECT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(63)), 63);
    EXPECT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(64) - 1), 63);
}

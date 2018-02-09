#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

TEST(TDirExists) {
    ASSERT_TRUE(TDir::Exists("../../test/cpp/files"));
    ASSERT_FALSE(TDir::Exists("./xyz"));
}

TEST(TMathFloorLog2_uint) {
    ASSERT_EQ(TMath::FloorLog2((uint)1), 0);
    ASSERT_EQ(TMath::FloorLog2((uint)2), 1);
    ASSERT_EQ(TMath::FloorLog2((uint)3), 1);
    ASSERT_EQ(TMath::FloorLog2((uint)4), 2);
    ASSERT_EQ(TMath::FloorLog2((uint)5), 2);
    ASSERT_EQ(TMath::FloorLog2((uint)6), 2);
    ASSERT_EQ(TMath::FloorLog2((uint)7), 2);
    ASSERT_EQ(TMath::FloorLog2((uint)8), 3);
    ASSERT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18) - 1), 17);
    ASSERT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18)), 18);
    ASSERT_EQ(TMath::FloorLog2((uint)TMath::Pow2(18) + 1), 18);
    ASSERT_EQ(TMath::FloorLog2((uint)TMath::Pow2(31)), 31);
    ASSERT_EQ(TMath::FloorLog2((uint)TMath::Pow2(32) - 1), 31);
}

TEST(TMathFloorLog2_uint64) {
    ASSERT_EQ(TMath::FloorLog2((uint64)1), 0);
    ASSERT_EQ(TMath::FloorLog2((uint64)2), 1);
    ASSERT_EQ(TMath::FloorLog2((uint64)3), 1);
    ASSERT_EQ(TMath::FloorLog2((uint64)4), 2);
    ASSERT_EQ(TMath::FloorLog2((uint64)5), 2);
    ASSERT_EQ(TMath::FloorLog2((uint64)6), 2);
    ASSERT_EQ(TMath::FloorLog2((uint64)7), 2);
    ASSERT_EQ(TMath::FloorLog2((uint64)8), 3);
    ASSERT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32) - 1), 31);
    ASSERT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32)), 32);
    ASSERT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(32) + 1), 32);
    ASSERT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(63)), 63);
    ASSERT_EQ(TMath::FloorLog2((uint64)TMath::Pow2<uint64>(64) - 1), 63);
}

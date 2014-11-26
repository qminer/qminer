#include <base.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

TEST(TStr, LeftRight) {
	const TStr As = "aaabbbaaa";

	// basic tests
	EXPECT_EQ(As.Left(3), "aaa");
	EXPECT_EQ(As.Right(6), "aaa");

	// negative indexes
	EXPECT_EQ(As.Left(-6), "aaa");
	EXPECT_EQ(As.Right(-3), "aaa");

	// edge cases
	EXPECT_ANY_THROW(As.Left(1000));
	EXPECT_ANY_THROW(As.Right(1000));
	EXPECT_EQ(As.Right(0), "aaabbbaaa");
	EXPECT_EQ(As.Left(0), "");
}

TEST(TStr, SplitLeftOfRightOf) {
	const TStr Str = "abcde";

	TStr LStr, RStr;
	Str.SplitLeftOfRightOf(LStr, 2, 2, RStr);

	// basic
	ASSERT_EQ(LStr, "ab");
	ASSERT_EQ(RStr, "de");

	// edge cases
	Str.SplitLeftOfRightOf(LStr, 0, Str.Len()-1, RStr);

	ASSERT_EQ(LStr, "");
	ASSERT_EQ(RStr, "");

	// exceptions
	ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, 4,3, RStr));
	ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, -1,3, RStr));
	ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, 0,Str.Len(), RStr));
}

TEST(TStr, SplitOnChN) {
	const TStr Str = "abcde";
	const TStr EmptyStr = "";

	TStr LStr, RStr;
	Str.SplitOnChN(LStr, 2, RStr);

	// basic
	ASSERT_EQ(LStr, "ab");
	ASSERT_EQ(RStr, "de");

	// edge cases
	Str.SplitOnChN(LStr, 0, RStr);

	ASSERT_EQ(LStr, "");
	ASSERT_EQ(RStr, "bcde");

	Str.SplitOnChN(LStr, Str.Len()-1, RStr);

	ASSERT_EQ(LStr, "abcd");
	ASSERT_EQ(RStr, "");

	// exceptions
	ASSERT_ANY_THROW(Str.SplitOnChN(LStr, -1, RStr));
	ASSERT_ANY_THROW(Str.SplitOnChN(LStr, Str.Len(), RStr));
	ASSERT_ANY_THROW(EmptyStr.SplitOnChN(LStr, 0, RStr));
}

TEST(TStr, SplitOnCh) {
	const TStr Str = "abcde";
	const TStr EmptyStr = "";

	TStr LStr, RStr;
	// middle
	Str.SplitOnCh(LStr, 'c', RStr);

	ASSERT_EQ(LStr, "ab");
	ASSERT_EQ(RStr, "de");

	// non-existent
	Str.SplitOnCh(LStr, 'g', RStr);

	ASSERT_EQ(LStr, "abcde");
	ASSERT_EQ(RStr, "");

	// first
	Str.SplitOnCh(LStr, 'a', RStr);

	ASSERT_EQ(LStr, "");
	ASSERT_EQ(RStr, "bcde");

	// last
	Str.SplitOnCh(LStr, 'e', RStr);

	ASSERT_EQ(LStr, "abcd");
	ASSERT_EQ(RStr, "");

	// empty
	EmptyStr.SplitOnCh(LStr, 'a', RStr);

	ASSERT_EQ(LStr, "");
	ASSERT_EQ(RStr, "");
}

TEST(TStr, IsUInt) {
	const TStr NormalCase = "22";
	const TStr NegativeCase = "-22";
	const TStr Zero = "0";
	const TStr MxVal = "4294967295";
	const TStr Overflow = "4294967296";

	uint Val;

	ASSERT_TRUE(NormalCase.IsUInt(Val));
	ASSERT_FALSE(NegativeCase.IsUInt(Val));
	ASSERT_TRUE(Zero.IsUInt(Val));
	ASSERT_TRUE(MxVal.IsUInt(Val));
	ASSERT_FALSE(Overflow.IsUInt(Val));
}

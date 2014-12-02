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

	// normal
	ASSERT_TRUE(NormalCase.IsUInt(Val));
	ASSERT_EQ(Val, 22);

	ASSERT_FALSE(NegativeCase.IsUInt(Val));

	ASSERT_TRUE(Zero.IsUInt(Val));
	ASSERT_EQ(Val, 0);

	ASSERT_TRUE(MxVal.IsUInt(Val));
	ASSERT_EQ(Val, 4294967295);

	ASSERT_FALSE(Overflow.IsUInt(Val));

	// bounds
	ASSERT_TRUE(NormalCase.IsUInt(true, 22, 22, Val));
	ASSERT_EQ(Val, 22);

	ASSERT_FALSE(NormalCase.IsUInt(true, 20, 21, Val));
	ASSERT_ANY_THROW(NormalCase.IsUInt(true, 22, 21, Val));
}

TEST(TStr, IsUInt64) {
	const TStr NormalCase = "22";
	const TStr NegativeCase = "-22";
	const TStr Zero = "0";
	const TStr MxVal = "18446744073709551615";
	const TStr Overflow = "18446744073709551616";

	uint64 Val;

	// normal
	ASSERT_TRUE(NormalCase.IsUInt64(Val));
	ASSERT_EQ(Val, 22);

	ASSERT_FALSE(NegativeCase.IsUInt64(Val));

	ASSERT_TRUE(Zero.IsUInt64(Val));
	ASSERT_EQ(Val, 0);

	ASSERT_TRUE(MxVal.IsUInt64(Val));
	ASSERT_EQ(Val, 18446744073709551615ul);

	ASSERT_FALSE(Overflow.IsUInt64(Val));

	// bounds
	ASSERT_TRUE(NormalCase.IsUInt64(true, 22, 22, Val));
	ASSERT_EQ(Val, 22);

	ASSERT_FALSE(NormalCase.IsUInt64(true, 20, 21, Val));
	ASSERT_ANY_THROW(NormalCase.IsUInt64(true, 22, 21, Val));
}

TEST(TStr, IsInt64) {
	int64 Num = 0;
	// normal
	EXPECT_TRUE(TStr("1234").IsInt64());
	EXPECT_TRUE(TStr("9223372036854775807").IsInt64());
	EXPECT_TRUE(TStr("-9223372036854775808").IsInt64());

	EXPECT_TRUE(TStr("1234").IsInt64(Num));
	EXPECT_EQ(1234, Num);

	EXPECT_TRUE(TStr("9223372036854775807").IsInt64(Num));
	EXPECT_EQ(9223372036854775807l, Num);

	EXPECT_TRUE(TStr("-9223372036854775808").IsInt64(Num));
	EXPECT_EQ(-1l * 9223372036854775808l, Num);

	printf("%ld\n", Num);

	// overflow
	EXPECT_FALSE(TStr("9223372036854775808").IsInt64());
	EXPECT_FALSE(TStr("-9223372036854775809").IsInt64());

	// characters
	EXPECT_FALSE(TStr("salad2147483649").IsInt64());
	EXPECT_FALSE(TStr("2147483649fingers").IsInt64());
}

TEST(TStr, SaveLoadTxt) {
	const TStr FNm = "test.txt";

	const TStr Empty;
	const TStr Alphabet = "abcdefghijklmnoprstuvz";

	Empty.SaveTxt(FNm);
	EXPECT_EQ(Empty, TStr::LoadTxt(FNm));

	Alphabet.SaveTxt(FNm);
	EXPECT_EQ(Alphabet, TStr::LoadTxt(FNm));

	Empty.SaveTxt(FNm);
	EXPECT_EQ(Empty, TStr::LoadTxt(FNm));
}

TEST(TStr, SaveLoad) {
	const TStr FNm = "test1.txt";

	const TStr Empty;
	const TStr Alphabet = "abcdefghijklmnoprstuvz";
	TStr Empty1, Alphabet1;

	// is big
	Empty.Save(*TFOut::New(FNm, false)(), false);
	Empty1 = Alphabet;
	Empty1.Load(*TFIn::New(FNm)(), false);
	EXPECT_EQ(Empty, Empty1);
	EXPECT_EQ(Empty, TStr(*TFIn::New(FNm)(), false));
	EXPECT_EQ(0, Empty1.Len());

	Empty.Save(*TFOut::New(FNm, false)(), false);
	Empty1 = Empty;
	Empty1.Load(*TFIn::New(FNm)(), false);
	EXPECT_EQ(Empty, Empty1);
	EXPECT_EQ(Empty, TStr(*TFIn::New(FNm)(), false));
	EXPECT_EQ(0, Empty1.Len());

	Alphabet.Save(*TFOut::New(FNm, false)(), false);
	Alphabet1 = Alphabet;
	Alphabet1.Load(*TFIn::New(FNm)(), false);
	EXPECT_EQ(Alphabet, Alphabet1);
	EXPECT_EQ(Alphabet, TStr(*TFIn::New(FNm)(), false));

	Alphabet.Save(*TFOut::New(FNm, false)(), false);
	Alphabet1 = Empty;
	Alphabet1.Load(*TFIn::New(FNm)(), false);
	EXPECT_EQ(Alphabet, Alphabet1);
	EXPECT_EQ(Alphabet, TStr(*TFIn::New(FNm)(), false));

	// is small
	Empty.Save(*TFOut::New(FNm, false)(), true);
	Empty1 = Alphabet;
	Empty1.Load(*TFIn::New(FNm)(), true);
	EXPECT_EQ(Empty, Empty1);
	EXPECT_EQ(Empty, TStr(*TFIn::New(FNm)(), true));
	EXPECT_EQ(0, Empty1.Len());

	Empty.Save(*TFOut::New(FNm, false)(), true);
	Empty1 = Empty;
	Empty1.Load(*TFIn::New(FNm)(), true);
	EXPECT_EQ(Empty, Empty1);
	EXPECT_EQ(Empty, TStr(*TFIn::New(FNm)(), true));
	EXPECT_EQ(0, Empty1.Len());

	Alphabet.Save(*TFOut::New(FNm, false)(), true);
	Alphabet1 = Alphabet;
	Alphabet1.Load(*TFIn::New(FNm)(), true);
	EXPECT_EQ(Alphabet, Alphabet1);
	EXPECT_EQ(Alphabet, TStr(*TFIn::New(FNm)(), true));

	Alphabet.Save(*TFOut::New(FNm, false)(), true);
	Alphabet1 = Empty;
	Alphabet1.Load(*TFIn::New(FNm)(), true);
	EXPECT_EQ(Alphabet, Alphabet1);
	EXPECT_EQ(Alphabet, TStr(*TFIn::New(FNm)(), true));

	// normal constructor
	{
		TFOut FOut(FNm, false);
		FOut.PutStr(Empty);
	}

	EXPECT_EQ(Empty, TStr(TFIn::New(FNm)));
	EXPECT_EQ(0, TStr(TFIn::New(FNm)).Len());

	{
		TFOut FOut(FNm, false);
		FOut.PutStr(Alphabet);
	}

	EXPECT_EQ(Alphabet, TStr(TFIn::New(FNm)));
}

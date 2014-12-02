#include <base.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

TEST(TStr, GetUc) {
	TStr Mixedcase = "AbCd";
	TStr Uppercase = "ABCD";
	TStr Empty = "";
	EXPECT_EQ(Uppercase, Mixedcase.GetUc());
	EXPECT_EQ(Empty, Empty.GetUc());
}
TEST(TStr, ToUc) {
	TStr Mixedcase = "AbCd";	
	TStr Uppercase = "ABCD";
	TStr Empty = "";
	TStr Empty2;
    Mixedcase.ToUc();
    Empty.ToUc();
	EXPECT_EQ(Mixedcase, Uppercase);
	EXPECT_EQ(Empty, Empty2);
}
TEST(TStr, ToLc) {
	TStr Mixedcase = "AbCd";
	TStr Lowercase = "abcd";	
	TStr Empty = "";
    TStr Empty2;
    Empty.ToLc();
    Mixedcase.ToLc();
	EXPECT_EQ(Mixedcase, Lowercase);
	EXPECT_EQ(Empty, Empty2);
}
TEST(TStr, GetLc) {
	TStr Mixedcase = "AbCd";
	TStr Lowercase = "abcd";	
	TStr Empty = "";
	EXPECT_EQ(Lowercase, Mixedcase.GetLc());
	EXPECT_EQ(Empty, Empty.GetLc());
}
TEST(TStr, CmpI) {
	TStr Empty = "";
	TStr Input = "bbbb";
    TStr Big = "ZZZZZZZZZZZZZZZZZZ";
	TStr Small = "aaaa";    
	EXPECT_TRUE(Input.CmpI(Input) == 0);
	EXPECT_TRUE(Input.CmpI(Big) < 0);
	EXPECT_TRUE(Input.CmpI(Small) > 0);
	EXPECT_TRUE(Empty.CmpI(Empty) == 0);
}
TEST(TStr, EqI) {    
	TStr Mixedcase = "AbCd";
	TStr Empty = "";
	EXPECT_TRUE(Mixedcase.EqI(Mixedcase.GetUc()));    
	EXPECT_TRUE(Empty.EqI(Empty));
}
TEST(TStr, GetCap) {
	TStr Lowercase = "abcd";
	TStr Capitalized = "Abcd";
	TStr Empty = "";
	EXPECT_EQ(Capitalized, Lowercase.GetCap());
	EXPECT_EQ(Empty, Empty.GetCap());
}
TEST(TStr, ToCap) {
	TStr Lowercase = "abcd";
	TStr Capitalized = "Abcd";	
	TStr Empty = "";
	TStr Empty2;
    Lowercase.ToCap();
	EXPECT_EQ(Capitalized, Lowercase);
    Empty.ToCap();
	EXPECT_EQ(Empty2, Empty);
}

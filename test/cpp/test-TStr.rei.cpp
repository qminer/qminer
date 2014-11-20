#include <base.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

/////////////////////////////////////////////////////////////////////
// Google Test Class
class StringTest : public ::testing::Test {

protected:
	// constructor for class
	StringTest() {
		mixedcase = "AbCd";
		lowercase = "abcd";
		capitalized = "Abcd";
		uppercase = "ABCD";
        empty = "";
	}

	// you can define new objects and new typedefs here
	TStr mixedcase, lowercase, uppercase, empty, capitalized;
};

/////////////////////////////////////////////////////////////////////
// Google Test functions

TEST_F(StringTest, GetUc_Test) {
	EXPECT_EQ(uppercase, mixedcase.GetUc());
	EXPECT_EQ(empty, empty.GetUc());
}
TEST_F(StringTest, ToUc_Test) {
    TStr empty2;
    mixedcase.ToUc();
    empty.ToUc();
	EXPECT_EQ(mixedcase, uppercase);
	EXPECT_EQ(empty, empty2);
}
TEST_F(StringTest, ToLc_Test) {
    TStr empty2;
    empty.ToLc();
    mixedcase.ToLc();
	EXPECT_EQ(mixedcase, lowercase);
	EXPECT_EQ(empty, empty2);
}
TEST_F(StringTest, GetLc_Test) {
	EXPECT_EQ(lowercase, mixedcase.GetLc());
	EXPECT_EQ(empty, empty.GetLc());
}
TEST_F(StringTest, CmpI_Test) {
	TStr input = "bbbb";
    TStr big = "ZZZZZZZZZZZZZZZZZZ";
	TStr Small = "aaaa";
    bool testEq = input.CmpI(input) == 0;
    bool testBig = input.CmpI(big) < 0;
    bool testSmall = input.CmpI(Small) > 0;
	EXPECT_EQ(true, testEq);
	EXPECT_EQ(true, testBig);
	EXPECT_EQ(true, testSmall);
	EXPECT_EQ(true, empty.CmpI(empty) == 0);
}
TEST_F(StringTest, EqI_Test) {
    bool testEq;
    testEq = mixedcase.EqI(mixedcase.GetUc());
	EXPECT_EQ(true, testEq);
    testEq = empty.EqI(empty);
	EXPECT_EQ(true, testEq);
}
TEST_F(StringTest, GetCap_Test) {
	EXPECT_EQ(capitalized, lowercase.GetCap());
	EXPECT_EQ(empty, empty.GetCap());
}
TEST_F(StringTest, ToCap_Test) {
    TStr empty2;
    lowercase.ToCap();
	EXPECT_EQ(capitalized, lowercase);
    empty.ToCap();
	EXPECT_EQ(empty2, empty);
}

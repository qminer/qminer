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
		input = "AbCd";
		output = "abcd";
	}

	// you can define new objects and new typedefs here
	TStr input, output;
};

/////////////////////////////////////////////////////////////////////
// Google Test functions

// test using objects from StringTest Class
TEST_F(StringTest, GetLc_Test) {
	bool TestOk = input.GetLc() == output;
	ASSERT_EQ(true, TestOk);
}

TEST_F(StringTest, GetLc_Ok_Test) {
	ASSERT_EQ(output, input.GetLc());
}

// test using it's own objects
TEST(StringTesting, GetLc_Blabla) {
	TStr input = "AbCd";
	TStr output = "abcde";
	// test returns failed cuz the strings are not equal
	EXPECT_EQ(output, input.GetLc());

	input = "ABcDE";
	// test return ok
	EXPECT_EQ(output, input.GetLc());
}

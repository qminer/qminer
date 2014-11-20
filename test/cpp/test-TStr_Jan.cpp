#include "base.h"
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

TEST(TStr, OperatorPlusEquals) {
	TStr Str = "abc";
	TStr Empty;
	// empyt+= full	
	Empty += Str;
	EXPECT_EQ(Empty, "abc");
	// self+= self
	Str += Str;
	EXPECT_EQ(Str, "abcabc");
	Str += TStr();	
	EXPECT_EQ(Str, "abcabc");
	
	// empyt+= full
	Empty = TStr();
	Empty += "abc";
	EXPECT_EQ(Empty, "abc");
	// full+= empty
	Str = "abc";
	Str += "";
	EXPECT_EQ(Str, "abc");

	Str = "abc";
	Str += nullptr;
	EXPECT_EQ(Str, "abc");
}
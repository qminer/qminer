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

TEST(TStr, OperatorComparison) {
	TStr Str = "abc";
	TStr Str2 = "Abc";
	TStr Str3 = "abc";
	TStr Str4 = "abc ";
	TStr Empty;
	
	EXPECT_TRUE(Str == Str3);
	EXPECT_TRUE(Empty == Empty);	

	EXPECT_FALSE(Str == Str2);
	EXPECT_FALSE(Str == Str4);
	EXPECT_FALSE(Str == Empty);

	EXPECT_TRUE(Str == "abc");
	EXPECT_TRUE(Empty == "");

	EXPECT_FALSE(Str == "Abc");
	EXPECT_FALSE(Str == "abc ");
	EXPECT_FALSE(Str == "");

	EXPECT_FALSE(Empty == nullptr);
}

TEST(TStr, OperatorIndex) {
	TStr Str = "abc";
	int Len = Str.Len();
	TStr Empty;

	EXPECT_EQ(Str[0], 'a');
	EXPECT_EQ(Str[1], 'b');
	EXPECT_EQ(Str[2], 'c');
		
	// changing character
	Str[2] = 'e';
	EXPECT_EQ(Str[2], 'e');
	EXPECT_EQ(Str.Len(), 3);	
}
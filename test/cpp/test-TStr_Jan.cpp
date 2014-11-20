#include "base.h"
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

TEST(TStr, Constructors) {
	TStr Default;
	TStr CStr("abc");	
	TStr OneChar('a');
	TStr CopyCStr(CStr); 
	TStr Move(TStr("abc"));
	TStr ChA(TChA("abc"));
	TStr SStr(TSStr("abc"));

	EXPECT_EQ(Default, "");
	EXPECT_EQ(CStr, "abc");
	EXPECT_EQ(OneChar, "a");
	EXPECT_EQ(CopyCStr, "abc");
	EXPECT_EQ(Move, "abc");
	EXPECT_EQ(ChA, "abc");
	EXPECT_EQ(SStr, "abc");
}


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
	
	// == operator
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

	// != operator
	EXPECT_FALSE(Str != Str3);
	EXPECT_FALSE(Empty != Empty);

	EXPECT_TRUE(Str != Str2);
	EXPECT_TRUE(Str != Str4);
	EXPECT_TRUE(Str != Empty);

	EXPECT_FALSE(Str != "abc");
	EXPECT_FALSE(Empty != "");

	EXPECT_TRUE(Str != "Abc");
	EXPECT_TRUE(Str != "abc ");
	EXPECT_TRUE(Str != "");

	EXPECT_TRUE(Empty != nullptr);
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

TEST(TStr, OperatorPlus) {
	TStr Str = "abc";
	TStr Empty;
	
	EXPECT_EQ(Empty + Str, "abc");
	EXPECT_EQ(Str + Empty, "abc");
	EXPECT_EQ(Empty + "abc", "abc");
	EXPECT_EQ(Str + "", "abc");	
	EXPECT_EQ(Str + nullptr, "abc");
}


///// Counts occurrences of a character between [BChN, end]
//int CountCh(const char& Ch, const int& BChN = 0) const;
///// Returns the position of the first occurrence of a character between [BChN, end]
//int SearchCh(const char& Ch, const int& BChN = 0) const;
///// Returns the position of the last occurrence of a character between [BChN, end]
//int SearchChBack(const char& Ch, int BChN = -1) const;
///// Returns the position of the first occurrence of a (sub)string between [BChN, end]
//int SearchStr(const TStr& Str, const int& BChN = 0) const;
///// Returns true if character occurs in string
//bool IsChIn(const char& Ch) const { return SearchCh(Ch) != -1; }
///// Returns true if (sub)string occurs in string
//bool IsStrIn(const TStr& Str) const { return SearchStr(Str) != -1; }
///// Returns true if this string starts with the prefix c-string
//bool StartsWith(const char *Str) const;
///// Returns true if this string starts with the prefix string
//bool StartsWith(const TStr& Str) const { return StartsWith(Str.CStr()); }
///// Returns true if this string ends with the sufix c-string
//bool EndsWith(const char *Str) const;
///// Returns true if this string ends with the sufix string
//bool EndsWith(const TStr& Str) const { return EndsWith(Str.CStr()); }
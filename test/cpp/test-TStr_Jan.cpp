#include "base.h"
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

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

TEST(TStr, Search) {
	TStr Str = "abcdaaba";
	int Len = Str.Len();
	EXPECT_EQ(Str.CountCh('a'), 4);
	EXPECT_EQ(Str.CountCh('b'), 2);
	EXPECT_EQ(Str.CountCh('e'), 0);

	EXPECT_TRUE(Str.IsChIn('a'));
	EXPECT_TRUE(Str.IsChIn('b'));
	EXPECT_FALSE(Str.IsChIn('e'));	

	EXPECT_TRUE(Str.IsStrIn(Str));
	EXPECT_TRUE(Str.IsStrIn(""));
	EXPECT_TRUE(Str.IsStrIn("bcd"));
	EXPECT_TRUE(Str.IsStrIn("ab"));
	EXPECT_FALSE(Str.IsStrIn("eba"));


	EXPECT_EQ(Str.CountCh('a', 1), 3);
	EXPECT_EQ(Str.CountCh('a', 10), 0);
	EXPECT_EQ(Str.CountCh('b', 2), 1);
	EXPECT_EQ(Str.CountCh('e', 1), 0);

	EXPECT_EQ(Str.SearchCh('a'), 0);
	EXPECT_EQ(Str.SearchCh('b'), 1);
	EXPECT_EQ(Str.SearchCh('e'), -1);

	EXPECT_EQ(Str.SearchCh('a', 1), 4);
	EXPECT_EQ(Str.SearchCh('b', 2), 6);
	EXPECT_EQ(Str.SearchCh('e', 1), -1);

	EXPECT_EQ(Str.SearchChBack('a'), Len - 1);
	EXPECT_EQ(Str.SearchChBack('b'), Len - 2);
	EXPECT_EQ(Str.SearchChBack('e'), -1);

	EXPECT_EQ(Str.SearchChBack('a', Len - 2), Len - 3);
	EXPECT_EQ(Str.SearchChBack('b', Len - 3), 1);;
	EXPECT_EQ(Str.SearchChBack('e', 3), -1);	

	EXPECT_EQ(Str.SearchStr("a"), 0);
	EXPECT_EQ(Str.SearchStr("b"), 1);
	EXPECT_EQ(Str.SearchStr("e"), -1);
	EXPECT_EQ(Str.SearchStr(""), 0); // TODO: check if this is OK

	EXPECT_EQ(Str.SearchStr("a", 1), 4);
	EXPECT_EQ(Str.SearchStr("b", 2), 6);
	EXPECT_EQ(Str.SearchStr("e", 1), -1);
}

TEST(TStr, StartsWith) {
	TStr Str = "abcdef";
	EXPECT_TRUE(Str.StartsWith("abc"));
	EXPECT_TRUE(Str.StartsWith(TStr("abc")));

	EXPECT_FALSE(Str.StartsWith("bbc"));
	EXPECT_FALSE(Str.StartsWith(TStr("bbc")));

	// Empty string is a prefix of every string
	EXPECT_TRUE(Str.StartsWith("")); // starts with empty strin
	EXPECT_TRUE(Str.StartsWith(TStr()));

	EXPECT_FALSE(Str.StartsWith("abcdefg"));
	EXPECT_FALSE(Str.StartsWith("AB"));
	EXPECT_FALSE(Str.StartsWith("abcdef "));
}

TEST(TStr, EndsWith) {
	TStr Str = "abcdef";
	EXPECT_TRUE(Str.EndsWith("def"));
	EXPECT_TRUE(Str.EndsWith(TStr("def")));

	EXPECT_FALSE(Str.EndsWith("ddf"));
	EXPECT_FALSE(Str.EndsWith(TStr("ddf")));

	// Empty string is a suffix of every string
	EXPECT_TRUE(Str.EndsWith("")); // ends with empty string
	EXPECT_TRUE(Str.EndsWith(TStr())); // ends with empty string

	EXPECT_FALSE(Str.EndsWith("aabcdef"));
	EXPECT_FALSE(Str.EndsWith("EF"));
	EXPECT_FALSE(Str.EndsWith(" abcdef"));
}

TEST(TStr, ParseBool) {
	TStr StrTrue = "T";
	TStr StrFalse = "F";
	TStr Empty = "";
	TStr Str = "abc";
	bool val;

	EXPECT_TRUE(StrTrue.IsBool(val));
	EXPECT_TRUE(val);
	EXPECT_FALSE(Str.IsBool(val));
	EXPECT_TRUE(val); // val was not changed, it is still set to true	
	EXPECT_TRUE(StrFalse.IsBool(val));
	EXPECT_FALSE(val);	
	EXPECT_FALSE(Empty.IsBool(val));
	EXPECT_FALSE(val); // val was not changed, it is still set to false
}

TEST(TStr, ParseInt) {	
	int Num = 0;
	EXPECT_TRUE(TStr("1234").IsInt());
	EXPECT_TRUE(TStr("2147483647").IsInt());
	EXPECT_TRUE(TStr("-2147483648").IsInt());
	EXPECT_FALSE(TStr("-21648.0").IsInt());
	
	EXPECT_TRUE(TStr("1234").IsInt(Num));
	EXPECT_EQ(1234, Num);
	EXPECT_TRUE(TStr("2147483647").IsInt(Num));
	EXPECT_EQ(2147483647, Num);
	EXPECT_TRUE(Num > 0);
	EXPECT_TRUE(TStr("-2147483648").IsInt(Num));
	EXPECT_TRUE(Num < 0);
	EXPECT_EQ(-2147483647 - 1, Num);
		
	EXPECT_TRUE(TStr("-2147483648").IsInt(true, TInt::Mn, TInt::Mx, Num));
	EXPECT_FALSE(TStr("-2147483649").IsInt(true, TInt::Mn, TInt::Mx, Num));		

	EXPECT_FALSE(TStr("salad2147483649").IsInt());
	EXPECT_FALSE(TStr("2147483649fingers").IsInt());

	EXPECT_EQ(TStr("2147483647").GetInt(), 2147483647);
	EXPECT_EQ(TStr("2147483648").GetInt(123), 123);
	EXPECT_EQ(TStr("2147483648").GetInt(123), 123);
	EXPECT_EQ(TStr("000000000000123").GetInt(), 123);
	EXPECT_EQ(TStr("2147483647foo").GetInt(123), 123);
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



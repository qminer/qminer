/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <base.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

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
    EXPECT_EQ(-1l * 9223372036854775807l - 1l, Num);

    //printf("%ld\n", Num);

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
    
    EXPECT_EQ(TStr(nullptr), "");
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
    TStr Empty;

    EXPECT_EQ(Str[0], 'a');
    EXPECT_EQ(Str[1], 'b');
    EXPECT_EQ(Str[2], 'c');
        
    // changing character
    Str[2] = 'e';
    EXPECT_EQ(Str[2], 'e');
    EXPECT_EQ(Str.Len(), 3);    
}

TEST(TStr, CStr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    const char* StrCStr = Str.CStr();
    const char* EmptyCStr = Empty.CStr();
    EXPECT_EQ(StrCStr[0], 'a');
    EXPECT_EQ(StrCStr[6], '\0');
    EXPECT_EQ(EmptyCStr[0], '\0');
}

TEST(TStr, CloneCStr) {
    const TStr Str = "abcdef";
    const TStr Empty = "";
    char* StrClone = Str.CloneCStr();
    char* EmptyClone = Empty.CloneCStr();
    EXPECT_EQ(Str, StrClone);
    EXPECT_EQ(Empty, EmptyClone);
}

TEST(TStr, PutCh) {
    TStr Str = "abcdef";
    TStr Empty = "";    
    Str.PutCh(0, 'k');
    Str.PutCh(5, 'k');
    EXPECT_EQ(Str[0], 'k');
    EXPECT_EQ(Str[5], 'k');
#ifndef NDEBUG
    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
    EXPECT_DEATH(Str.PutCh(-1, 'a'), "");
    EXPECT_DEATH(Empty.PutCh(0, 'a'), "");
#endif    
}

TEST(TStr, GetCh) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_EQ(Str.GetCh(0), 'a');
    EXPECT_EQ(Str.GetCh(5), 'f');
#ifndef NDEBUG        
    dup2(2, 1);    // redirect stdout to stderr (Assert emits a printf to stdout)
    EXPECT_DEATH(Str.GetCh(-1), "");
    EXPECT_DEATH(Empty.GetCh(0), "");    
#endif    
}

TEST(TStr, LastCh) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_EQ(Str.LastCh(), 'f');
#ifndef NDEBUG
    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
    EXPECT_DEATH(Empty.LastCh(), "");
#endif    
}

TEST(TStr, Len) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_EQ(Str.Len(), 6);
    EXPECT_EQ(Empty.Len(), 0);
}

TEST(TStr, Empty) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_FALSE(Str.Empty());
    EXPECT_TRUE(Empty.Empty());
}

TEST(TStr, Clr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    Str.Clr();
    Empty.Clr();
    EXPECT_EQ(Str, "");
    EXPECT_EQ(Str.Len(), 0);
    EXPECT_EQ(Empty, "");
    EXPECT_EQ(Empty.Len(), 0);
}

TEST(TStr, GetStr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    const TStr& Ref = Str.GetStr();
    Str[0] = 'x';
    EXPECT_EQ(Ref[0], 'x');
    EXPECT_EQ(Ref[1], 'b');
    const TStr& RefEmpty = Empty.GetStr();
    EXPECT_EQ(RefEmpty, "");
}

TEST(TStr, GetMemUsed) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_EQ(Str.GetMemUsed(), 8 + 7);
    EXPECT_EQ(Empty.GetMemUsed(), 8);
}

TEST(TStr, Trunc) {
    TStr Str = "   abcdef    ";
    TStr Str2 = "    ";
    TStr Str3 = "abcdef    ";
    TStr Str4 = "    abcdef";
    EXPECT_EQ(Str.GetTrunc(), "abcdef");
    EXPECT_EQ(Str2.GetTrunc(), TStr());    
    EXPECT_EQ(Str.ToTrunc(), "abcdef");
    EXPECT_EQ(Str2.ToTrunc(), TStr());
    EXPECT_EQ(Str3.ToTrunc(), "abcdef");
    EXPECT_EQ(Str4.ToTrunc(), "abcdef");
}

TEST(TStr, Hex) {
    TStr Str = ".a";    
    EXPECT_EQ(Str.GetHex(), "2E61");
    EXPECT_EQ(Str.GetHex().GetFromHex(), Str);
    EXPECT_EQ(Str.ToHex(), "2E61");
    EXPECT_EQ(Str.FromHex(), ".a");    
    EXPECT_EQ(TStr().GetHex(), "");
    EXPECT_EQ(TStr().GetFromHex(), "");
}

TEST(TStr, GetSubStr) {
    TStr Str = "abcda";
    TStr Empty = "";

    EXPECT_EQ(Str.GetSubStr(3), "da");
    EXPECT_EQ(Str.GetSubStr(3, 3), "d");

    EXPECT_ANY_THROW(Str.GetSubStr(-1, -1));
    EXPECT_ANY_THROW(Str.GetSubStr(2, 1));
    EXPECT_ANY_THROW(Str.GetSubStr(-1, 100));
}
TEST(TStr, InsStr) {
    TStr Str = "abcda";
    TStr Empty = "";

    Str.InsStr(2, "xk");
    EXPECT_EQ(Str, "abxkcda");
    Str.InsStr(2, "");
    EXPECT_EQ(Str, "abxkcda");
    Str.InsStr(0, "f");
    EXPECT_EQ(Str, "fabxkcda");
    Str.InsStr(8, "f");
    EXPECT_EQ(Str, "fabxkcdaf");
    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
    EXPECT_ANY_THROW(Str.InsStr(100, "kek"));
    EXPECT_ANY_THROW(Str.InsStr(-100, "kek"));
}

TEST(TStr, Del) {
    TStr Str = "aabbaabb";
    TStr Empty = "";
    
    TStr Test = Str;
    Test.DelChAll('a');
    EXPECT_EQ(Test, "bbbb");
    Test.DelChAll('c');
    EXPECT_EQ(Test, "bbbb");
    
    Test = Str;
    Test.DelSubStr(2, 3);
    EXPECT_EQ(Test, "aaaabb");
    
    Test.DelSubStr(0, 1);
    EXPECT_EQ(Test, "aabb");
    Test.DelSubStr(2, 3);
    EXPECT_EQ(Test, "aa");
    
    EXPECT_ANY_THROW(Test.DelSubStr(-1, 5));
    
    Test.DelSubStr(0, 1);
    EXPECT_EQ(Test, "");
    Test = Str;
    Test.DelSubStr(0, 0);
    EXPECT_EQ(Test, "abbaabb");
    

    Test = Str;
    EXPECT_TRUE(Test.DelStr("ab"));
    EXPECT_EQ(Test, "abaabb");
    EXPECT_FALSE(Test.DelStr("fs"));
    EXPECT_EQ(Test, "abaabb");

    Test = Str;
    EXPECT_EQ(2, Test.DelStrAll("ab"));
    EXPECT_EQ(Test, "abab");
}

TEST(TStr, LeftOfRightOf) {
    TStr Str = "abcdef";
    TStr Empty = "";
    EXPECT_EQ(Str.LeftOf('d'), "abc");
    EXPECT_EQ(Str.RightOf('c'), "def");    
    EXPECT_EQ(Str.LeftOf('a'), "");
    EXPECT_EQ(Empty.RightOf('c'), "");
    // edge cases
    EXPECT_EQ(Str.RightOf('f'), "");
    EXPECT_EQ(Empty.LeftOf('d'), "");

    TStr Str2 = "abcdefabcdef";
    EXPECT_EQ(Str2.LeftOfLast('d'), "abcdefabc");
    EXPECT_EQ(Str2.RightOfLast('c'), "def");
    EXPECT_EQ(Empty.LeftOfLast('d'), "");
    EXPECT_EQ(Empty.RightOfLast('c'), "");
    // edge cases
    Str2 = "xabcdefabcdef";
    EXPECT_EQ(Str2.LeftOfLast('x'), "");
    EXPECT_EQ(Str2.RightOfLast('f'), "");
}


///// Splits on the first occurrence of the target string
///// if the target string is not found the whole string is returned as the left side
//void SplitOnStr(TStr& LStr, const TStr& SplitStr, TStr& RStr) const;
TEST(TStr, SplitOnStr) {
    const TStr Str = "abcd";
    const TStr EmptyStr = "";
    TStr LStr, RStr;
    
    // left empty
    Str.SplitOnStr(LStr, "ab", RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "cd");
    
    // both nonempty
    Str.SplitOnStr(LStr, "bc", RStr);
    EXPECT_EQ(LStr, "a");
    EXPECT_EQ(RStr, "d");
    
    // right empty
    Str.SplitOnStr(LStr, "cd", RStr);
    EXPECT_EQ(LStr, "ab");
    EXPECT_EQ(RStr, "");
    
    // both empty
    Str.SplitOnStr(LStr, "abcd", RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "");
    
    // no-match
    Str.SplitOnStr(LStr, "fg", RStr);
    EXPECT_EQ(LStr, Str);
    EXPECT_EQ(RStr, "");
    Str.SplitOnStr(LStr, "abcde", RStr);
    EXPECT_EQ(LStr, Str);
    EXPECT_EQ(RStr, "");
    Str.SplitOnStr(LStr, "", RStr);
    EXPECT_EQ(LStr, Str);
    EXPECT_EQ(RStr, "");
    
    // empty
    EmptyStr.SplitOnStr(LStr, "aa", RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "");
    EmptyStr.SplitOnStr(LStr, "", RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "");
}

///// Split on last occurrence of SplitCh, return Pair of Left/Right strings
///// if the character is not found the whole string is returned as the right side
//void SplitOnLastCh(TStr& LStr, const char& SplitCh, TStr& RStr) const;
TEST(TStr, SplitOnLastCh) {
    const TStr Str = "abcd";
    const TStr Str2 = "a";
    const TStr EmptyStr = "";
    TStr LStr, RStr;

    // left empty
    Str.SplitOnLastCh(LStr, 'a', RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "bcd");

    // right empty
    Str.SplitOnLastCh(LStr, 'd', RStr);
    EXPECT_EQ(LStr, "abc");
    EXPECT_EQ(RStr, "");

    // both
    Str2.SplitOnLastCh(LStr, 'a', RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "");

    // both nonempty
    Str.SplitOnLastCh(LStr, 'b', RStr);
    EXPECT_EQ(LStr, "a");
    EXPECT_EQ(RStr, "cd");

    // no-match
    Str.SplitOnLastCh(LStr, 'x', RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, Str);

    // empty
    EmptyStr.SplitOnLastCh(LStr, 'a', RStr);
    EXPECT_EQ(LStr, "");
    EXPECT_EQ(RStr, "");
}

///// Split on all occurrences of SplitCh, write to StrV, optionally don't create empy strings (default true)
//void SplitOnAllCh(const char& SplitCh, TStrV& StrV, const bool& SkipEmpty = true) const;
TEST(TStr, SplitOnAllCh) {
    TStr Str = "abcabca";
    TStrV StrV;    
    Str.SplitOnAllCh('a', StrV, true);
    EXPECT_EQ(2, StrV.Len());
    EXPECT_EQ(TStr("bc"), StrV[0]);
    EXPECT_EQ(TStr("bc"), StrV[1]);
    Str.SplitOnAllCh('a', StrV, false);
    EXPECT_EQ(4, StrV.Len());
    EXPECT_EQ(TStr(), StrV[0]);
    EXPECT_EQ(TStr("bc"), StrV[1]);
    EXPECT_EQ(TStr("bc"), StrV[2]);
    EXPECT_EQ(TStr(), StrV[3]);
    // edge cases
    Str = "a";
    Str.SplitOnAllCh('a', StrV, true);
    EXPECT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    EXPECT_EQ(2, StrV.Len());
    Str = "aa";
    Str.SplitOnAllCh('a', StrV, true);
    EXPECT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    EXPECT_EQ(3, StrV.Len());
    Str = "";
    Str.SplitOnAllCh('a', StrV, true);
    EXPECT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    EXPECT_EQ(1, StrV.Len());
    // non match
    Str = "abc";
    Str.SplitOnAllCh('x', StrV, true);
    EXPECT_EQ(1, StrV.Len());
    Str.SplitOnAllCh('x', StrV, false);
    EXPECT_EQ(1, StrV.Len());
}

///// Split on all occurrences of any char in SplitChStr, optionally don't create empy strings (default true)
//void SplitOnAllAnyCh(const TStr& SplitChStr, TStrV& StrV, const bool& SkipEmpty = true) const;
TEST(TStr, SplitOnAllAnyCh) {

}

///// Split on the occurrences of any string in StrV
//void SplitOnWs(TStrV& StrV) const;
TEST(TStr, SplitOnWs) {

}

///// Split on the occurrences of any non alphanumeric character
//void SplitOnNonAlNum(TStrV& StrV) const;
TEST(TStr, SplitOnNonAlNum) {

}

///// Split on all the occurrences of SplitStr
//void SplitOnStr(const TStr& SplitStr, TStrV& StrV) const;
TEST(TStr, SplitOnStr_VectorOutput) {
    TStr Str = "xybcxybcxy";
    // TStrV StrV; Str.SplitOnStr("xy", StrV);
    
    // EXPECT_EQ(3, StrV.Len());
    // EXPECT_TRUE(StrV[0] == "");
    // EXPECT_EQ(TStr("bc"), StrV[1]);
    // EXPECT_EQ(TStr("bc"), StrV[2]);
    
    //
    //// edge cases
    //Str = "a";
    //Str.SplitOnAllCh('a', StrV, true);
    //EXPECT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //EXPECT_EQ(2, StrV.Len());
    //Str = "aa";
    //Str.SplitOnAllCh('a', StrV, true);
    //EXPECT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //EXPECT_EQ(3, StrV.Len());
    //Str = "";
    //Str.SplitOnAllCh('a', StrV, true);
    //EXPECT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //EXPECT_EQ(1, StrV.Len());
    //// non match
    //Str = "abc";
    //Str.SplitOnAllCh('x', StrV, true);
    //EXPECT_EQ(1, StrV.Len());
    //Str.SplitOnAllCh('x', StrV, false);
    //EXPECT_EQ(1, StrV.Len());
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
    EXPECT_ANY_THROW(Str.CountCh('a', 10));
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
    EXPECT_EQ(Str.SearchStr(""), 0);

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

TEST(TStr, ChangeCh) {
    TStr Str = "aaabbaaac";
    TStr Empty;
    int ChN = 0;
    ChN = Empty.ChangeCh('a', 'c');
    EXPECT_EQ(Empty, "");
    EXPECT_EQ(ChN, -1);
    ChN = Str.ChangeCh('a', 'c');
    EXPECT_EQ(Str, "caabbaaac");
    EXPECT_EQ(ChN, 0);
    ChN = Str.ChangeCh('a', 'c', 3);
    EXPECT_EQ(Str, "caabbcaac");
    EXPECT_EQ(ChN, 5);

    EXPECT_ANY_THROW(ChN = Str.ChangeCh('a', 'c', 10));    

#ifndef NDEBUG
    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
    EXPECT_DEATH(Str.ChangeCh('a', '\0'), "");
#endif
}

TEST(TStr, ChangeChAll) {
    TStr Str = "abac";
    TStr Empty;
    int Changes = 0;
    Changes = Empty.ChangeChAll('a', 'c');
    EXPECT_EQ(Empty, "");
    EXPECT_EQ(Changes, 0);
    Changes = Str.ChangeChAll('a', 'c');
    EXPECT_EQ(Str, "cbcc");
    EXPECT_EQ(Changes, 2);
}

TEST(TStr, ChangeStr) {
    TStr Str = "abac";
    TStr Empty;
    int ChangeN = 0;
    ChangeN = Empty.ChangeStr("ab", "ac");
    EXPECT_EQ(Empty, "");
    EXPECT_EQ(ChangeN, -1);
    ChangeN = Str.ChangeStr("ab", "abab");
    EXPECT_EQ(Str, "ababac");
    EXPECT_EQ(ChangeN, 0);
}

TEST(TStr, ChangeStrAll) {
    TStr Str = "abac";
    TStr Empty;
    int Changes = 0;
    Changes = Empty.ChangeStrAll("ab", "ac");
    EXPECT_EQ(Empty, "");
    EXPECT_EQ(Changes, 0);
    Changes = Str.ChangeStrAll("", "a"); // replacing and empty string should do nothing
    EXPECT_EQ(Str, "abac");
    EXPECT_EQ(Changes, 0);
    Changes = Str.ChangeStrAll("ab", "abab");
    EXPECT_EQ(Str, "ababac");
    EXPECT_EQ(Changes, 1);
    Changes = Str.ChangeStrAll("ab", "abab");
    EXPECT_EQ(Str, "ababababac");
    EXPECT_EQ(Changes, 2);
    Changes = Str.ChangeStrAll("ababababac", "");
    EXPECT_EQ(Str, "");
    EXPECT_EQ(Changes, 1);
    Changes = Str.ChangeStrAll("", "a"); // replacing and empty string should do nothing
    EXPECT_EQ(Str, "");
    EXPECT_EQ(Changes, 0);
}

TEST(TStr, Reverse) {
    TStr Str = "abac";
    TStr Empty;
    EXPECT_EQ(Empty.Reverse(), "");    
    EXPECT_EQ(Str.Reverse(), "caba");
    EXPECT_EQ(Str.Reverse().Reverse(), Str);    
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
    
    EXPECT_EQ(TStr("2147483647").GetInt(), 2147483647);
    EXPECT_EQ(TStr("-2147483648").GetInt(), TInt::Mn);    

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

//////////////////////////////////////////////////////////////////////
// Base64 tests

TEST(TStr, Base64Encode1) {
    uchar src[] = { 'a', 'b', 'c', 'd' };
    TMem Mem(src, sizeof(src));
    TStr Str = TStr::Base64Encode(Mem);
    EXPECT_EQ(Str, "YWJjZA==");
}

TEST(TStr, Base64Encode2) {
    uchar src[] = { 'a', 'b', 'c', 'd', 'd', 'd', 'd', 'd', 'd' };
    TMem Mem(src, sizeof(src));
    TStr Str = TStr::Base64Encode(Mem);
    EXPECT_EQ(Str, "YWJjZGRkZGRk");
}

TEST(TStr, Base64Decode1) {
    TStr Str = "YWJjZA==";
    TMem Mem;
    TStr::Base64Decode(Str, Mem);
    EXPECT_EQ(Mem.Len(), 4);
    const char* Bf = Mem.GetBf();
    EXPECT_EQ(Bf[0], 'a');
    EXPECT_EQ(Bf[1], 'b');
    EXPECT_EQ(Bf[2], 'c');
    EXPECT_EQ(Bf[3], 'd');
}

TEST(TStr, Base64Decode2) {
    TStr Str = "YWJjZGRkZGRk";
    TMem Mem;
    TStr::Base64Decode(Str, Mem);
    EXPECT_EQ(Mem.Len(), 9);
    const char* Bf = Mem.GetBf();
    EXPECT_EQ(Bf[0], 'a');
    EXPECT_EQ(Bf[1], 'b');
    EXPECT_EQ(Bf[2], 'c');
    EXPECT_EQ(Bf[3], 'd');
    EXPECT_EQ(Bf[4], 'd');
    EXPECT_EQ(Bf[5], 'd');
    EXPECT_EQ(Bf[6], 'd');
    EXPECT_EQ(Bf[7], 'd');
    EXPECT_EQ(Bf[8], 'd');
}

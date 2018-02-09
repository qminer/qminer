#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

TEST(TStrGetUc) {
    TStr Mixedcase = "AbCd";
    TStr Uppercase = "ABCD";
    TStr Empty = "";
    ASSERT_EQ(Uppercase.CStr(), Mixedcase.GetUc().CStr());
    ASSERT_EQ(Empty.CStr(), Empty.GetUc().CStr());
}

TEST(TStrToUc) {
    TStr Mixedcase = "AbCd";
    TStr Uppercase = "ABCD";
    TStr Empty = "";
    TStr Empty2;
    Mixedcase.ToUc();
    Empty.ToUc();
    ASSERT_EQ(Mixedcase.CStr(), Uppercase.CStr());
    ASSERT_EQ(Empty.CStr(), Empty2.CStr());
}

TEST(TStrToLc) {
    TStr Mixedcase = "AbCd";
    TStr Lowercase = "abcd";
    TStr Empty = "";
    TStr Empty2;
    Empty.ToLc();
    Mixedcase.ToLc();
    ASSERT_EQ(Mixedcase.CStr(), Lowercase.CStr());
    ASSERT_EQ(Empty.CStr(), Empty2.CStr());
}

TEST(TStrGetLc) {
    TStr Mixedcase = "AbCd";
    TStr Lowercase = "abcd";
    TStr Empty = "";
    ASSERT_EQ(Lowercase.CStr(), Mixedcase.GetLc().CStr());
    ASSERT_EQ(Empty.CStr(), Empty.GetLc().CStr());
}

TEST(TStrCmpI) {
    TStr Empty = "";
    TStr Input = "bbbb";
    TStr Big = "ZZZZZZZZZZZZZZZZZZ";
    TStr Small = "aaaa";
    ASSERT_TRUE(Input.CmpI(Input) == 0);
    ASSERT_TRUE(Input.CmpI(Big) < 0);
    ASSERT_TRUE(Input.CmpI(Small) > 0);
    ASSERT_TRUE(Empty.CmpI(Empty) == 0);
}

TEST(TStrEqI) {
    TStr Mixedcase = "AbCd";
    TStr Empty = "";
    ASSERT_TRUE(Mixedcase.EqI(Mixedcase.GetUc()));
    ASSERT_TRUE(Empty.EqI(Empty));
}

TEST(TStrGetCap) {
    TStr Lowercase = "abcd";
    TStr Capitalized = "Abcd";
    TStr Empty = "";
    ASSERT_EQ(Capitalized.CStr(), Lowercase.GetCap().CStr());
    ASSERT_EQ(Empty.CStr(), Empty.GetCap().CStr());
}

TEST(TStrToCap) {
    TStr Lowercase = "abcd";
    TStr Capitalized = "Abcd";
    TStr Empty = "";
    TStr Empty2;
    Lowercase.ToCap();
    ASSERT_EQ(Capitalized.CStr(), Lowercase.CStr());
    Empty.ToCap();
    ASSERT_EQ(Empty2.CStr(), Empty.CStr());
}

TEST(TStrLeftRight) {
    const TStr As = "aaabbbaaa";

    // basic tests
    ASSERT_EQ(As.Left(3).CStr(), "aaa");
    ASSERT_EQ(As.Right(6).CStr(), "aaa");

    // negative indexes
    ASSERT_EQ(As.Left(-6).CStr(), "aaa");
    ASSERT_EQ(As.Right(-3).CStr(), "aaa");

    // edge cases
    ASSERT_ANY_THROW(As.Left(1000));
    ASSERT_ANY_THROW(As.Right(1000));
    ASSERT_EQ(As.Right(0).CStr(), "aaabbbaaa");
    ASSERT_EQ(As.Left(0).CStr(), "");
}

TEST(TStrSplitLeftOfRightOf) {
    const TStr Str = "abcde";

    TStr LStr, RStr;
    Str.SplitLeftOfRightOf(LStr, 2, 2, RStr);

    // basic
    ASSERT_EQ(LStr.CStr(), "ab");
    ASSERT_EQ(RStr.CStr(), "de");

    // edge cases
    Str.SplitLeftOfRightOf(LStr, 0, Str.Len() - 1, RStr);

    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");

    // exceptions
    ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, 4, 3, RStr));
    ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, -1, 3, RStr));
    ASSERT_ANY_THROW(Str.SplitLeftOfRightOf(LStr, 0, Str.Len(), RStr));
}

TEST(TStrSplitOnChN) {
    const TStr Str = "abcde";
    const TStr EmptyStr = "";

    TStr LStr, RStr;
    Str.SplitOnChN(LStr, 2, RStr);

    // basic
    ASSERT_EQ(LStr.CStr(), "ab");
    ASSERT_EQ(RStr.CStr(), "de");

    // edge cases
    Str.SplitOnChN(LStr, 0, RStr);

    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "bcde");

    Str.SplitOnChN(LStr, Str.Len() - 1, RStr);

    ASSERT_EQ(LStr.CStr(), "abcd");
    ASSERT_EQ(RStr.CStr(), "");

    // exceptions
    ASSERT_ANY_THROW(Str.SplitOnChN(LStr, -1, RStr));
    ASSERT_ANY_THROW(Str.SplitOnChN(LStr, Str.Len(), RStr));
    ASSERT_ANY_THROW(EmptyStr.SplitOnChN(LStr, 0, RStr));
}

TEST(TStrSplitOnCh) {
    const TStr Str = "abcde";
    const TStr EmptyStr = "";

    TStr LStr, RStr;
    // middle
    Str.SplitOnCh(LStr, 'c', RStr);

    ASSERT_EQ(LStr.CStr(), "ab");
    ASSERT_EQ(RStr.CStr(), "de");

    // non-existent
    Str.SplitOnCh(LStr, 'g', RStr);

    ASSERT_EQ(LStr.CStr(), "abcde");
    ASSERT_EQ(RStr.CStr(), "");

    // first
    Str.SplitOnCh(LStr, 'a', RStr);

    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "bcde");

    // last
    Str.SplitOnCh(LStr, 'e', RStr);

    ASSERT_EQ(LStr.CStr(), "abcd");
    ASSERT_EQ(RStr.CStr(), "");

    // empty
    EmptyStr.SplitOnCh(LStr, 'a', RStr);

    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");
}

TEST(TStrIsUInt) {
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

TEST(TStrIsUInt64) {
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

TEST(TStrIsInt64) {
    int64 Num = 0;
    // normal
    ASSERT_TRUE(TStr("1234").IsInt64());
    ASSERT_TRUE(TStr("9223372036854775807").IsInt64());
    ASSERT_TRUE(TStr("-9223372036854775808").IsInt64());

    ASSERT_TRUE(TStr("1234").IsInt64(Num));
    ASSERT_EQ(1234, Num);

    ASSERT_TRUE(TStr("9223372036854775807").IsInt64(Num));
    ASSERT_EQ(9223372036854775807l, Num);

    ASSERT_TRUE(TStr("-9223372036854775808").IsInt64(Num));
    ASSERT_EQ(-1l * 9223372036854775807l - 1l, Num);

    //printf("%ld\n", Num);

    // overflow
    ASSERT_FALSE(TStr("9223372036854775808").IsInt64());
    ASSERT_FALSE(TStr("-9223372036854775809").IsInt64());

    // characters
    ASSERT_FALSE(TStr("salad2147483649").IsInt64());
    ASSERT_FALSE(TStr("2147483649fingers").IsInt64());
}

TEST(TStrSaveLoadTxt) {
    const TStr FNm = "test.txt";

    const TStr Empty;
    const TStr Alphabet = "abcdefghijklmnoprstuvz";

    Empty.SaveTxt(FNm);
    ASSERT_EQ(Empty.CStr(), TStr::LoadTxt(FNm).CStr());

    Alphabet.SaveTxt(FNm);
    ASSERT_EQ(Alphabet.CStr(), TStr::LoadTxt(FNm).CStr());

    Empty.SaveTxt(FNm);
    ASSERT_EQ(Empty.CStr(), TStr::LoadTxt(FNm).CStr());
}

TEST(TStrSaveLoad) {
    const TStr FNm = "test1.txt";

    const TStr Empty;
    const TStr Alphabet = "abcdefghijklmnoprstuvz";
    TStr Empty1, Alphabet1;

    // is big
    Empty.Save(*TFOut::New(FNm, false)(), false);
    Empty1 = Alphabet;
    Empty1.Load(*TFIn::New(FNm)(), false);
    ASSERT_EQ(Empty.CStr(), Empty1.CStr());
    ASSERT_EQ(Empty.CStr(), TStr(*TFIn::New(FNm)(), false).CStr());
    ASSERT_EQ(0, Empty1.Len());

    Empty.Save(*TFOut::New(FNm, false)(), false);
    Empty1 = Empty;
    Empty1.Load(*TFIn::New(FNm)(), false);
    ASSERT_EQ(Empty.CStr(), Empty1.CStr());
    ASSERT_EQ(Empty.CStr(), TStr(*TFIn::New(FNm)(), false).CStr());
    ASSERT_EQ(0, Empty1.Len());

    Alphabet.Save(*TFOut::New(FNm, false)(), false);
    Alphabet1 = Alphabet;
    Alphabet1.Load(*TFIn::New(FNm)(), false);
    ASSERT_EQ(Alphabet.CStr(), Alphabet1.CStr());
    ASSERT_EQ(Alphabet.CStr(), TStr(*TFIn::New(FNm)(), false).CStr());

    Alphabet.Save(*TFOut::New(FNm, false)(), false);
    Alphabet1 = Empty;
    Alphabet1.Load(*TFIn::New(FNm)(), false);
    ASSERT_EQ(Alphabet.CStr(), Alphabet1.CStr());
    ASSERT_EQ(Alphabet.CStr(), TStr(*TFIn::New(FNm)(), false).CStr());

    // is small
    Empty.Save(*TFOut::New(FNm, false)(), true);
    Empty1 = Alphabet;
    Empty1.Load(*TFIn::New(FNm)(), true);
    ASSERT_EQ(Empty.CStr(), Empty1.CStr());
    ASSERT_EQ(Empty.CStr(), TStr(*TFIn::New(FNm)(), true).CStr());
    ASSERT_EQ(0, Empty1.Len());

    Empty.Save(*TFOut::New(FNm, false)(), true);
    Empty1 = Empty;
    Empty1.Load(*TFIn::New(FNm)(), true);
    ASSERT_EQ(Empty.CStr(), Empty1.CStr());
    ASSERT_EQ(Empty.CStr(), TStr(*TFIn::New(FNm)(), true).CStr());
    ASSERT_EQ(0, Empty1.Len());

    Alphabet.Save(*TFOut::New(FNm, false)(), true);
    Alphabet1 = Alphabet;
    Alphabet1.Load(*TFIn::New(FNm)(), true);
    ASSERT_EQ(Alphabet.CStr(), Alphabet1.CStr());
    ASSERT_EQ(Alphabet.CStr(), TStr(*TFIn::New(FNm)(), true).CStr());

    Alphabet.Save(*TFOut::New(FNm, false)(), true);
    Alphabet1 = Empty;
    Alphabet1.Load(*TFIn::New(FNm)(), true);
    ASSERT_EQ(Alphabet.CStr(), Alphabet1.CStr());
    ASSERT_EQ(Alphabet.CStr(), TStr(*TFIn::New(FNm)(), true).CStr());

    // normal constructor
    {
        TFOut FOut(FNm, false);
        FOut.PutStr(Empty);
    }

    ASSERT_EQ(Empty.CStr(), TStr(TFIn::New(FNm)).CStr());
    ASSERT_EQ(0, TStr(TFIn::New(FNm)).Len());

    {
        TFOut FOut(FNm, false);
        FOut.PutStr(Alphabet);
    }

    ASSERT_EQ(Alphabet.CStr(), TStr(TFIn::New(FNm)).CStr());
}

TEST(TStrConstructors) {
    TStr Default;
    TStr CStr("abc");
    TStr OneChar('a');
    TStr CopyCStr(CStr);
    TStr Move(TStr("abc"));
    TStr ChA(TChA("abc"));
    TStr SStr(TSStr("abc"));

    ASSERT_EQ(Default.CStr(), "");
    ASSERT_EQ(CStr.CStr(), "abc");
    ASSERT_EQ(OneChar.CStr(), "a");
    ASSERT_EQ(CopyCStr.CStr(), "abc");
    ASSERT_EQ(Move.CStr(), "abc");
    ASSERT_EQ(ChA.CStr(), "abc");
    ASSERT_EQ(SStr.CStr(), "abc");

    ASSERT_EQ(TStr(nullptr).CStr(), "");
}

TEST(TStrOperatorPlusEquals) {
    TStr Str = "abc";
    TStr Empty;
    // empyt+= full    
    Empty += Str;
    ASSERT_EQ(Empty.CStr(), "abc");
    // self+= self
    Str += Str;
    ASSERT_EQ(Str.CStr(), "abcabc");
    Str += TStr();
    ASSERT_EQ(Str.CStr(), "abcabc");

    // empyt+= full
    Empty = TStr();
    Empty += "abc";
    ASSERT_EQ(Empty.CStr(), "abc");
    // full+= empty
    Str = "abc";
    Str += "";
    ASSERT_EQ(Str.CStr(), "abc");

    Str = "abc";
    Str += nullptr;
    ASSERT_EQ(Str.CStr(), "abc");
}

TEST(TStrOperatorComparison) {
    TStr Str = "abc";
    TStr Str2 = "Abc";
    TStr Str3 = "abc";
    TStr Str4 = "abc ";
    TStr Empty;

    // == operator
    ASSERT_TRUE(Str == Str3);
    ASSERT_TRUE(Empty == Empty);

    ASSERT_FALSE(Str == Str2);
    ASSERT_FALSE(Str == Str4);
    ASSERT_FALSE(Str == Empty);

    ASSERT_TRUE(Str == "abc");
    ASSERT_TRUE(Empty == "");

    ASSERT_FALSE(Str == "Abc");
    ASSERT_FALSE(Str == "abc ");
    ASSERT_FALSE(Str == "");

    ASSERT_FALSE(Empty == nullptr);

    // != operator
    ASSERT_FALSE(Str != Str3);
    ASSERT_FALSE(Empty != Empty);

    ASSERT_TRUE(Str != Str2);
    ASSERT_TRUE(Str != Str4);
    ASSERT_TRUE(Str != Empty);

    ASSERT_FALSE(Str != "abc");
    ASSERT_FALSE(Empty != "");

    ASSERT_TRUE(Str != "Abc");
    ASSERT_TRUE(Str != "abc ");
    ASSERT_TRUE(Str != "");

    ASSERT_TRUE(Empty != nullptr);
}

TEST(TStrOperatorIndex) {
    TStr Str = "abc";
    TStr Empty;

    ASSERT_EQ(Str[0], 'a');
    ASSERT_EQ(Str[1], 'b');
    ASSERT_EQ(Str[2], 'c');

    // changing character
    Str[2] = 'e';
    ASSERT_EQ(Str[2], 'e');
    ASSERT_EQ(Str.Len(), 3);
}

TEST(TStrCStr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    const char* StrCStr = Str.CStr();
    const char* EmptyCStr = Empty.CStr();
    ASSERT_EQ(StrCStr[0], 'a');
    ASSERT_EQ(StrCStr[6], '\0');
    ASSERT_EQ(EmptyCStr[0], '\0');
}

TEST(TStrCloneCStr) {
    const TStr Str = "abcdef";
    const TStr Empty = "";
    char* StrClone = Str.CloneCStr();
    char* EmptyClone = Empty.CloneCStr();
    ASSERT_EQ(Str.CStr(), StrClone);
    ASSERT_EQ(Empty.CStr(), EmptyClone);
}

TEST(TStrPutCh) {
    TStr Str = "abcdef";
    TStr Empty = "";
    Str.PutCh(0, 'k');
    Str.PutCh(5, 'k');
    ASSERT_EQ(Str[0], 'k');
    ASSERT_EQ(Str[5], 'k');
//#ifndef NDEBUG
//    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
//    EXPECT_DEATH(Str.PutCh(-1, 'a'), "");
//    EXPECT_DEATH(Empty.PutCh(0, 'a'), "");
//#endif    
}

TEST(TStrGetCh) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_EQ(Str.GetCh(0), 'a');
    ASSERT_EQ(Str.GetCh(5), 'f');
//#ifndef NDEBUG        
//    dup2(2, 1);    // redirect stdout to stderr (Assert emits a printf to stdout)
//    EXPECT_DEATH(Str.GetCh(-1), "");
//    EXPECT_DEATH(Empty.GetCh(0), "");
//#endif    
}

TEST(TStrLastCh) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_EQ(Str.LastCh(), 'f');
//#ifndef NDEBUG
//    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
//    EXPECT_DEATH(Empty.LastCh(), "");
//#endif    
}

TEST(TStrLen) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_EQ(Str.Len(), 6);
    ASSERT_EQ(Empty.Len(), 0);
}

TEST(TStrEmpty) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_FALSE(Str.Empty());
    ASSERT_TRUE(Empty.Empty());
}

TEST(TStrClr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    Str.Clr();
    Empty.Clr();
    ASSERT_EQ(Str.CStr(), "");
    ASSERT_EQ(Str.Len(), 0);
    ASSERT_EQ(Empty.CStr(), "");
    ASSERT_EQ(Empty.Len(), 0);
}

TEST(TStrGetStr) {
    TStr Str = "abcdef";
    TStr Empty = "";
    const TStr& Ref = Str.GetStr();
    Str[0] = 'x';
    ASSERT_EQ(Ref[0], 'x');
    ASSERT_EQ(Ref[1], 'b');
    const TStr& RefEmpty = Empty.GetStr();
    ASSERT_EQ(RefEmpty.CStr(), "");
}

TEST(TStrGetMemUsed) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_EQ(Str.GetMemUsed(), 8 + 7);
    ASSERT_EQ(Empty.GetMemUsed(), 8);
}

TEST(TStrTrunc) {
    TStr Str = "   abcdef    ";
    TStr Str2 = "    ";
    TStr Str3 = "abcdef    ";
    TStr Str4 = "    abcdef";
    ASSERT_EQ(Str.GetTrunc().CStr(), "abcdef");
    ASSERT_EQ(Str2.GetTrunc().CStr(), TStr().CStr());
    ASSERT_EQ(Str.ToTrunc().CStr(), "abcdef");
    ASSERT_EQ(Str2.ToTrunc().CStr(), TStr().CStr());
    ASSERT_EQ(Str3.ToTrunc().CStr(), "abcdef");
    ASSERT_EQ(Str4.ToTrunc().CStr(), "abcdef");
}

TEST(TStrHex) {
    TStr Str = ".a";
    ASSERT_EQ(Str.GetHex().CStr(), "2E61");
    ASSERT_EQ(Str.GetHex().GetFromHex().CStr(), Str.CStr());
    ASSERT_EQ(Str.ToHex().CStr(), "2E61");
    ASSERT_EQ(Str.FromHex().CStr(), ".a");
    ASSERT_EQ(TStr().GetHex().CStr(), "");
    ASSERT_EQ(TStr().GetFromHex().CStr(), "");
}

TEST(TStrGetSubStr) {
    TStr Str = "abcda";
    TStr Empty = "";

    ASSERT_EQ(Str.GetSubStr(3).CStr(), "da");
    ASSERT_EQ(Str.GetSubStr(3, 3).CStr(), "d");

    ASSERT_ANY_THROW(Str.GetSubStr(-1, -1));
    ASSERT_ANY_THROW(Str.GetSubStr(2, 1));
    ASSERT_ANY_THROW(Str.GetSubStr(-1, 100));
}
TEST(TStrInsStr) {
    TStr Str = "abcda";
    TStr Empty = "";

    Str.InsStr(2, "xk");
    ASSERT_EQ(Str.CStr(), "abxkcda");
    Str.InsStr(2, "");
    ASSERT_EQ(Str.CStr(), "abxkcda");
    Str.InsStr(0, "f");
    ASSERT_EQ(Str.CStr(), "fabxkcda");
    Str.InsStr(8, "f");
    ASSERT_EQ(Str.CStr(), "fabxkcdaf");
    //dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
    ASSERT_ANY_THROW(Str.InsStr(100, "kek"));
    ASSERT_ANY_THROW(Str.InsStr(-100, "kek"));
}

TEST(TStrDel) {
    TStr Str = "aabbaabb";
    TStr Empty = "";

    TStr Test = Str;
    Test.DelChAll('a');
    ASSERT_EQ(Test.CStr(), "bbbb");
    Test.DelChAll('c');
    ASSERT_EQ(Test.CStr(), "bbbb");

    Test = Str;
    Test.DelSubStr(2, 3);
    ASSERT_EQ(Test.CStr(), "aaaabb");

    Test.DelSubStr(0, 1);
    ASSERT_EQ(Test.CStr(), "aabb");
    Test.DelSubStr(2, 3);
    ASSERT_EQ(Test.CStr(), "aa");

    ASSERT_ANY_THROW(Test.DelSubStr(-1, 5));

    Test.DelSubStr(0, 1);
    ASSERT_EQ(Test.CStr(), "");
    Test = Str;
    Test.DelSubStr(0, 0);
    ASSERT_EQ(Test.CStr(), "abbaabb");


    Test = Str;
    ASSERT_TRUE(Test.DelStr("ab"));
    ASSERT_EQ(Test.CStr(), "abaabb");
    ASSERT_FALSE(Test.DelStr("fs"));
    ASSERT_EQ(Test.CStr(), "abaabb");

    Test = Str;
    ASSERT_EQ(2, Test.DelStrAll("ab"));
    ASSERT_EQ(Test.CStr(), "abab");
}

TEST(TStrLeftOfRightOf) {
    TStr Str = "abcdef";
    TStr Empty = "";
    ASSERT_EQ(Str.LeftOf('d').CStr(), "abc");
    ASSERT_EQ(Str.RightOf('c').CStr(), "def");
    ASSERT_EQ(Str.LeftOf('a').CStr(), "");
    ASSERT_EQ(Empty.RightOf('c').CStr(), "");
    // edge cases
    ASSERT_EQ(Str.RightOf('f').CStr(), "");
    ASSERT_EQ(Empty.LeftOf('d').CStr(), "");

    TStr Str2 = "abcdefabcdef";
    ASSERT_EQ(Str2.LeftOfLast('d').CStr(), "abcdefabc");
    ASSERT_EQ(Str2.RightOfLast('c').CStr(), "def");
    ASSERT_EQ(Empty.LeftOfLast('d').CStr(), "");
    ASSERT_EQ(Empty.RightOfLast('c').CStr(), "");
    // edge cases
    Str2 = "xabcdefabcdef";
    ASSERT_EQ(Str2.LeftOfLast('x').CStr(), "");
    ASSERT_EQ(Str2.RightOfLast('f').CStr(), "");
}


///// Splits on the first occurrence of the target string
///// if the target string is not found the whole string is returned as the left side
//void SplitOnStr(TStr& LStr, const TStr& SplitStr, TStr& RStr) const;
TEST(TStrSplitOnStr) {
    const TStr Str = "abcd";
    const TStr EmptyStr = "";
    TStr LStr, RStr;

    // left empty
    Str.SplitOnStr(LStr, "ab", RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "cd");

    // both nonempty
    Str.SplitOnStr(LStr, "bc", RStr);
    ASSERT_EQ(LStr.CStr(), "a");
    ASSERT_EQ(RStr.CStr(), "d");

    // right empty
    Str.SplitOnStr(LStr, "cd", RStr);
    ASSERT_EQ(LStr.CStr(), "ab");
    ASSERT_EQ(RStr.CStr(), "");

    // both empty
    Str.SplitOnStr(LStr, "abcd", RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");

    // no-match
    Str.SplitOnStr(LStr, "fg", RStr);
    ASSERT_EQ(LStr.CStr(), Str.CStr());
    ASSERT_EQ(RStr.CStr(), "");
    Str.SplitOnStr(LStr, "abcde", RStr);
    ASSERT_EQ(LStr.CStr(), Str.CStr());
    ASSERT_EQ(RStr.CStr(), "");
    Str.SplitOnStr(LStr, "", RStr);
    ASSERT_EQ(LStr.CStr(), Str.CStr());
    ASSERT_EQ(RStr.CStr(), "");

    // empty
    EmptyStr.SplitOnStr(LStr, "aa", RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");
    EmptyStr.SplitOnStr(LStr, "", RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");
}

///// Split on last occurrence of SplitCh, return Pair of Left/Right strings
///// if the character is not found the whole string is returned as the right side
//void SplitOnLastCh(TStr& LStr, const char& SplitCh, TStr& RStr) const;
TEST(TStrSplitOnLastCh) {
    const TStr Str = "abcd";
    const TStr Str2 = "a";
    const TStr EmptyStr = "";
    TStr LStr, RStr;

    // left empty
    Str.SplitOnLastCh(LStr, 'a', RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "bcd");

    // right empty
    Str.SplitOnLastCh(LStr, 'd', RStr);
    ASSERT_EQ(LStr.CStr(), "abc");
    ASSERT_EQ(RStr.CStr(), "");

    // both
    Str2.SplitOnLastCh(LStr, 'a', RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");

    // both nonempty
    Str.SplitOnLastCh(LStr, 'b', RStr);
    ASSERT_EQ(LStr.CStr(), "a");
    ASSERT_EQ(RStr.CStr(), "cd");

    // no-match
    Str.SplitOnLastCh(LStr, 'x', RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), Str.CStr());

    // empty
    EmptyStr.SplitOnLastCh(LStr, 'a', RStr);
    ASSERT_EQ(LStr.CStr(), "");
    ASSERT_EQ(RStr.CStr(), "");
}

///// Split on all occurrences of SplitCh, write to StrV, optionally don't create empy strings (default true)
//void SplitOnAllCh(const char& SplitCh, TStrV& StrV, const bool& SkipEmpty = true) const;
TEST(TStrSplitOnAllCh) {
    TStr Str = "abcabca";
    TStrV StrV;
    Str.SplitOnAllCh('a', StrV, true);
    ASSERT_EQ(2, StrV.Len());
    ASSERT_EQ(TStr("bc").CStr(), StrV[0].CStr());
    ASSERT_EQ(TStr("bc").CStr(), StrV[1].CStr());
    Str.SplitOnAllCh('a', StrV, false);
    ASSERT_EQ(4, StrV.Len());
    ASSERT_EQ(TStr().CStr(), StrV[0].CStr());
    ASSERT_EQ(TStr("bc").CStr(), StrV[1].CStr());
    ASSERT_EQ(TStr("bc").CStr(), StrV[2].CStr());
    ASSERT_EQ(TStr().CStr(), StrV[3].CStr());
    // edge cases
    Str = "a";
    Str.SplitOnAllCh('a', StrV, true);
    ASSERT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    ASSERT_EQ(2, StrV.Len());
    Str = "aa";
    Str.SplitOnAllCh('a', StrV, true);
    ASSERT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    ASSERT_EQ(3, StrV.Len());
    Str = "";
    Str.SplitOnAllCh('a', StrV, true);
    ASSERT_EQ(0, StrV.Len());
    Str.SplitOnAllCh('a', StrV, false);
    ASSERT_EQ(1, StrV.Len());
    // non match
    Str = "abc";
    Str.SplitOnAllCh('x', StrV, true);
    ASSERT_EQ(1, StrV.Len());
    Str.SplitOnAllCh('x', StrV, false);
    ASSERT_EQ(1, StrV.Len());
}

///// Split on all occurrences of any char in SplitChStr, optionally don't create empy strings (default true)
//void SplitOnAllAnyCh(const TStr& SplitChStr, TStrV& StrV, const bool& SkipEmpty = true) const;
TEST(TStrSplitOnAllAnyCh) {

}

///// Split on the occurrences of any string in StrV
//void SplitOnWs(TStrV& StrV) const;
TEST(TStrSplitOnWs) {

}

///// Split on the occurrences of any non alphanumeric character
//void SplitOnNonAlNum(TStrV& StrV) const;
TEST(TStrSplitOnNonAlNum) {

}

///// Split on all the occurrences of SplitStr
//void SplitOnStr(const TStr& SplitStr, TStrV& StrV) const;
TEST(TStrSplitOnStr_VectorOutput) {
    TStr Str = "xybcxybcxy";
    // TStrV StrV; Str.SplitOnStr("xy", StrV);

    // ASSERT_EQ(3, StrV.Len());
    // ASSERT_TRUE(StrV[0] == "");
    // ASSERT_EQ(TStr("bc"), StrV[1]);
    // ASSERT_EQ(TStr("bc"), StrV[2]);

    //
    //// edge cases
    //Str = "a";
    //Str.SplitOnAllCh('a', StrV, true);
    //ASSERT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //ASSERT_EQ(2, StrV.Len());
    //Str = "aa";
    //Str.SplitOnAllCh('a', StrV, true);
    //ASSERT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //ASSERT_EQ(3, StrV.Len());
    //Str = "";
    //Str.SplitOnAllCh('a', StrV, true);
    //ASSERT_EQ(0, StrV.Len());
    //Str.SplitOnAllCh('a', StrV, false);
    //ASSERT_EQ(1, StrV.Len());
    //// non match
    //Str = "abc";
    //Str.SplitOnAllCh('x', StrV, true);
    //ASSERT_EQ(1, StrV.Len());
    //Str.SplitOnAllCh('x', StrV, false);
    //ASSERT_EQ(1, StrV.Len());
}

TEST(TStrSearch) {
    TStr Str = "abcdaaba";
    int Len = Str.Len();
    ASSERT_EQ(Str.CountCh('a'), 4);
    ASSERT_EQ(Str.CountCh('b'), 2);
    ASSERT_EQ(Str.CountCh('e'), 0);

    ASSERT_TRUE(Str.IsChIn('a'));
    ASSERT_TRUE(Str.IsChIn('b'));
    ASSERT_FALSE(Str.IsChIn('e'));

    ASSERT_TRUE(Str.IsStrIn(Str));
    ASSERT_TRUE(Str.IsStrIn(""));
    ASSERT_TRUE(Str.IsStrIn("bcd"));
    ASSERT_TRUE(Str.IsStrIn("ab"));
    ASSERT_FALSE(Str.IsStrIn("eba"));


    ASSERT_EQ(Str.CountCh('a', 1), 3);
    ASSERT_ANY_THROW(Str.CountCh('a', 10));
    ASSERT_EQ(Str.CountCh('b', 2), 1);
    ASSERT_EQ(Str.CountCh('e', 1), 0);

    ASSERT_EQ(Str.SearchCh('a'), 0);
    ASSERT_EQ(Str.SearchCh('b'), 1);
    ASSERT_EQ(Str.SearchCh('e'), -1);

    ASSERT_EQ(Str.SearchCh('a', 1), 4);
    ASSERT_EQ(Str.SearchCh('b', 2), 6);
    ASSERT_EQ(Str.SearchCh('e', 1), -1);

    ASSERT_EQ(Str.SearchChBack('a'), Len - 1);
    ASSERT_EQ(Str.SearchChBack('b'), Len - 2);
    ASSERT_EQ(Str.SearchChBack('e'), -1);

    ASSERT_EQ(Str.SearchChBack('a', Len - 2), Len - 3);
    ASSERT_EQ(Str.SearchChBack('b', Len - 3), 1);;
    ASSERT_EQ(Str.SearchChBack('e', 3), -1);

    ASSERT_EQ(Str.SearchStr("a"), 0);
    ASSERT_EQ(Str.SearchStr("b"), 1);
    ASSERT_EQ(Str.SearchStr("e"), -1);
    ASSERT_EQ(Str.SearchStr(""), 0);

    ASSERT_EQ(Str.SearchStr("a", 1), 4);
    ASSERT_EQ(Str.SearchStr("b", 2), 6);
    ASSERT_EQ(Str.SearchStr("e", 1), -1);
}

TEST(TStrStartsWith) {
    TStr Str = "abcdef";
    ASSERT_TRUE(Str.StartsWith("abc"));
    ASSERT_TRUE(Str.StartsWith(TStr("abc")));

    ASSERT_FALSE(Str.StartsWith("bbc"));
    ASSERT_FALSE(Str.StartsWith(TStr("bbc")));

    // Empty string is a prefix of every string
    ASSERT_TRUE(Str.StartsWith("")); // starts with empty strin
    ASSERT_TRUE(Str.StartsWith(TStr()));

    ASSERT_FALSE(Str.StartsWith("abcdefg"));
    ASSERT_FALSE(Str.StartsWith("AB"));
    ASSERT_FALSE(Str.StartsWith("abcdef "));
}

TEST(TStrEndsWith) {
    TStr Str = "abcdef";
    ASSERT_TRUE(Str.EndsWith("def"));
    ASSERT_TRUE(Str.EndsWith(TStr("def")));

    ASSERT_FALSE(Str.EndsWith("ddf"));
    ASSERT_FALSE(Str.EndsWith(TStr("ddf")));

    // Empty string is a suffix of every string
    ASSERT_TRUE(Str.EndsWith("")); // ends with empty string
    ASSERT_TRUE(Str.EndsWith(TStr())); // ends with empty string

    ASSERT_FALSE(Str.EndsWith("aabcdef"));
    ASSERT_FALSE(Str.EndsWith("EF"));
    ASSERT_FALSE(Str.EndsWith(" abcdef"));
}

TEST(TStrChangeCh) {
    TStr Str = "aaabbaaac";
    TStr Empty;
    int ChN = 0;
    ChN = Empty.ChangeCh('a', 'c');
    ASSERT_EQ(Empty.CStr(), "");
    ASSERT_EQ(ChN, -1);
    ChN = Str.ChangeCh('a', 'c');
    ASSERT_EQ(Str.CStr(), "caabbaaac");
    ASSERT_EQ(ChN, 0);
    ChN = Str.ChangeCh('a', 'c', 3);
    ASSERT_EQ(Str.CStr(), "caabbcaac");
    ASSERT_EQ(ChN, 5);

    ASSERT_ANY_THROW(ChN = Str.ChangeCh('a', 'c', 10));

//#ifndef NDEBUG
//    dup2(2, 1); // redirect stdout to stderr (Assert emits a printf to stdout)
//    EXPECT_DEATH(Str.ChangeCh('a', '\0'), "");
//#endif
}

TEST(TStrChangeChAll) {
    TStr Str = "abac";
    TStr Empty;
    int Changes = 0;
    Changes = Empty.ChangeChAll('a', 'c');
    ASSERT_EQ(Empty.CStr(), "");
    ASSERT_EQ(Changes, 0);
    Changes = Str.ChangeChAll('a', 'c');
    ASSERT_EQ(Str.CStr(), "cbcc");
    ASSERT_EQ(Changes, 2);
}

TEST(TStrChangeStr) {
    TStr Str = "abac";
    TStr Empty;
    int ChangeN = 0;
    ChangeN = Empty.ChangeStr("ab", "ac");
    ASSERT_EQ(Empty.CStr(), "");
    ASSERT_EQ(ChangeN, -1);
    ChangeN = Str.ChangeStr("ab", "abab");
    ASSERT_EQ(Str.CStr(), "ababac");
    ASSERT_EQ(ChangeN, 0);
}

TEST(TStrChangeStrAll) {
    TStr Str = "abac";
    TStr Empty;
    int Changes = 0;
    Changes = Empty.ChangeStrAll("ab", "ac");
    ASSERT_EQ(Empty.CStr(), "");
    ASSERT_EQ(Changes, 0);
    Changes = Str.ChangeStrAll("", "a"); // replacing and empty string should do nothing
    ASSERT_EQ(Str.CStr(), "abac");
    ASSERT_EQ(Changes, 0);
    Changes = Str.ChangeStrAll("ab", "abab");
    ASSERT_EQ(Str.CStr(), "ababac");
    ASSERT_EQ(Changes, 1);
    Changes = Str.ChangeStrAll("ab", "abab");
    ASSERT_EQ(Str.CStr(), "ababababac");
    ASSERT_EQ(Changes, 2);
    Changes = Str.ChangeStrAll("ababababac", "");
    ASSERT_EQ(Str.CStr(), "");
    ASSERT_EQ(Changes, 1);
    Changes = Str.ChangeStrAll("", "a"); // replacing and empty string should do nothing
    ASSERT_EQ(Str.CStr(), "");
    ASSERT_EQ(Changes, 0);
}

TEST(TStrReverse) {
    TStr Str = "abac";
    TStr Empty;
    ASSERT_EQ(Empty.Reverse().CStr(), "");
    ASSERT_EQ(Str.Reverse().CStr(), "caba");
    ASSERT_EQ(Str.Reverse().Reverse().CStr(), Str.CStr());
}

TEST(TStrParseBool) {
    TStr StrTrue = "T";
    TStr StrFalse = "F";
    TStr Empty = "";
    TStr Str = "abc";
    bool val;

    ASSERT_TRUE(StrTrue.IsBool(val));
    ASSERT_TRUE(val);
    ASSERT_FALSE(Str.IsBool(val));
    ASSERT_TRUE(val); // val was not changed, it is still set to true    
    ASSERT_TRUE(StrFalse.IsBool(val));
    ASSERT_FALSE(val);
    ASSERT_FALSE(Empty.IsBool(val));
    ASSERT_FALSE(val); // val was not changed, it is still set to false
}

TEST(TStrParseInt) {
    int Num = 0;
    ASSERT_TRUE(TStr("1234").IsInt());
    ASSERT_TRUE(TStr("2147483647").IsInt());
    ASSERT_TRUE(TStr("-2147483648").IsInt());
    ASSERT_FALSE(TStr("-21648.0").IsInt());

    ASSERT_EQ(TStr("2147483647").GetInt(), 2147483647);
    ASSERT_EQ(TStr("-2147483648").GetInt(), TInt::Mn);

    ASSERT_TRUE(TStr("1234").IsInt(Num));
    ASSERT_EQ(1234, Num);
    ASSERT_TRUE(TStr("2147483647").IsInt(Num));
    ASSERT_EQ(2147483647, Num);
    ASSERT_TRUE(Num > 0);
    ASSERT_TRUE(TStr("-2147483648").IsInt(Num));
    ASSERT_TRUE(Num < 0);
    ASSERT_EQ(-2147483647 - 1, Num);

    ASSERT_TRUE(TStr("-2147483648").IsInt(true, TInt::Mn, TInt::Mx, Num));
    ASSERT_FALSE(TStr("-2147483649").IsInt(true, TInt::Mn, TInt::Mx, Num));

    ASSERT_FALSE(TStr("salad2147483649").IsInt());
    ASSERT_FALSE(TStr("2147483649fingers").IsInt());

    ASSERT_EQ(TStr("2147483647").GetInt(), 2147483647);
    ASSERT_EQ(TStr("2147483648").GetInt(123), 123);
    ASSERT_EQ(TStr("2147483648").GetInt(123), 123);
    ASSERT_EQ(TStr("000000000000123").GetInt(), 123);
    ASSERT_EQ(TStr("2147483647foo").GetInt(123), 123);
}

TEST(TStrOperatorPlus) {
    TStr Str = "abc";
    TStr Empty;

    ASSERT_EQ((Empty + Str).CStr(), "abc");
    ASSERT_EQ((Str + Empty).CStr(), "abc");
    ASSERT_EQ((Empty + "abc").CStr(), "abc");
    ASSERT_EQ((Str + "").CStr(), "abc");
    ASSERT_EQ((Str + nullptr).CStr(), "abc");
}

//////////////////////////////////////////////////////////////////////
// Base64 tests

TEST(TStrBase64Encode1) {
    uchar src[] = { 'a', 'b', 'c', 'd' };
    TMem Mem(src, sizeof(src));
    TStr Str = TStr::Base64Encode(Mem);
    ASSERT_EQ(Str.CStr(), "YWJjZA==");
}

TEST(TStrBase64Encode2) {
    uchar src[] = { 'a', 'b', 'c', 'd', 'd', 'd', 'd', 'd', 'd' };
    TMem Mem(src, sizeof(src));
    TStr Str = TStr::Base64Encode(Mem);
    ASSERT_EQ(Str.CStr(), "YWJjZGRkZGRk");
}

TEST(TStrBase64Decode1) {
    TStr Str = "YWJjZA==";
    TMem Mem;
    TStr::Base64Decode(Str, Mem);
    ASSERT_EQ(Mem.Len(), 4);
    const char* Bf = Mem.GetBf();
    ASSERT_EQ(Bf[0], 'a');
    ASSERT_EQ(Bf[1], 'b');
    ASSERT_EQ(Bf[2], 'c');
    ASSERT_EQ(Bf[3], 'd');
}

TEST(TStrBase64Decode2) {
    TStr Str = "YWJjZGRkZGRk";
    TMem Mem;
    TStr::Base64Decode(Str, Mem);
    ASSERT_EQ(Mem.Len(), 9);
    const char* Bf = Mem.GetBf();
    ASSERT_EQ(Bf[0], 'a');
    ASSERT_EQ(Bf[1], 'b');
    ASSERT_EQ(Bf[2], 'c');
    ASSERT_EQ(Bf[3], 'd');
    ASSERT_EQ(Bf[4], 'd');
    ASSERT_EQ(Bf[5], 'd');
    ASSERT_EQ(Bf[6], 'd');
    ASSERT_EQ(Bf[7], 'd');
    ASSERT_EQ(Bf[8], 'd');
}
#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

int Prime(const int& n);
bool IsPrime(const int& d);

// Test the default constructor
TEST(TIntIntHDefaultConstructor) {
    TIntIntH TableInt;

    ASSERT_EQ(1, TableInt.Empty());
    ASSERT_EQ(0, TableInt.Len());
    ASSERT_EQ(0, TableInt.GetMxKeyIds());
}

// Table manipulations
TEST(TIntIntHManipulateTable) {
    const int64 NElems = 1000000;
    int DDist = 10;
    const char *FName = "test.hashint.dat";
    TIntIntH TableInt;
    TIntIntH TableInt1;
    TIntIntH TableInt2;
    int i;
    int d;
    int n;
    int Id;
    int Key;
    int64 KeySumVal;
    int64 DatSumVal;
    int64 KeySum;
    int64 DatSum;
    int64 KeySumDel;
    int64 DatSumDel;
    int DelCount;
    int Count;

    // add table elements
    d = Prime(NElems);
    n = d;
    KeySumVal = 0;
    DatSumVal = 0;
    for (i = 0; i < NElems; i++) {
        TableInt.AddDat(n, n + 1);
        KeySumVal += n;
        DatSumVal += (n + 1);
        //printf("add %d %d\n", n, n+1);
        n = (n + d) % NElems;
    }
    ASSERT_EQ(0, TableInt.Empty());
    ASSERT_EQ(NElems, TableInt.Len());

    ASSERT_EQ(0, (NElems - 1)*(NElems) / 2 - KeySumVal);
    ASSERT_EQ(0, (NElems)*(NElems + 1) / 2 - DatSumVal);

    // verify elements by successive keys
    KeySum = 0;
    DatSum = 0;
    for (i = 0; i < NElems; i++) {
        Id = TableInt.GetKeyId(i);
        ASSERT_TRUE(Id >= 0);
        Key = TableInt.GetKey(Id);
        ASSERT_EQ(0, TableInt.GetDat(Key) - Key - 1);
        KeySum += Key;
        DatSum += TableInt.GetDat(Key);
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by distant keys
    KeySum = 0;
    DatSum = 0;
    n = Prime(d);
    for (i = 0; i < NElems; i++) {
        Id = TableInt.GetKeyId(n);
        ASSERT_TRUE(Id >= 0);
        Key = TableInt.GetKey(Id);
        ASSERT_EQ(0, TableInt.GetDat(Key) - Key - 1);
        KeySum += Key;
        DatSum += TableInt.GetDat(Key);
        n = (n + d) % NElems;
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    for (TIntIntH::TIter It = TableInt.BegI(); It < TableInt.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        KeySum += It.GetKey();
        DatSum += It.GetDat();
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by key index
    KeySum = 0;
    DatSum = 0;
    Id = TableInt.FFirstKeyId();
    while (TableInt.FNextKeyId(Id)) {
        ASSERT_TRUE(Id >= 0);
        Key = TableInt.GetKey(Id);
        ASSERT_EQ(0, TableInt.GetDat(Key) - Key - 1);
        KeySum += Key;
        DatSum += TableInt.GetDat(Key);
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // delete elements
    DelCount = 0;
    KeySumDel = 0;
    DatSumDel = 0;
    for (n = 0; n < NElems; n += DDist) {
        Id = TableInt.GetKeyId(n);
        //printf("del %d %d %d\n", n, Id, (int) TableInt[Id]);
        KeySumDel += n;
        DatSumDel += TableInt[Id];
        TableInt.DelKeyId(Id);
        DelCount++;
    }
    ASSERT_EQ(0, TableInt.Empty());
    ASSERT_EQ(NElems - DelCount, TableInt.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TIntIntH::TIter It = TableInt.BegI(); It < TableInt.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += It.GetKey();
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // assignment
    TableInt1 = TableInt;
    ASSERT_EQ(0, TableInt1.Empty());
    ASSERT_EQ(NElems - DelCount, TableInt1.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TIntIntH::TIter It = TableInt1.BegI(); It < TableInt1.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += It.GetKey();
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // saving and loading
    {
        TFOut FOut(FName);
        TableInt.Save(FOut);
        FOut.Flush();
    }

    {
        TFIn FIn(FName);
        TableInt2.Load(FIn);
    }

    ASSERT_EQ(NElems - DelCount, TableInt2.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TIntIntH::TIter It = TableInt2.BegI(); It < TableInt2.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += It.GetKey();
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // remove all elements
    for (i = 0; i < Count; i++) {
        Id = TableInt.GetRndKeyId(TInt::Rnd, 0.5);
        TableInt.DelKeyId(Id);
    }
    ASSERT_EQ(0, TableInt.Len());
    ASSERT_EQ(1, TableInt.Empty());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TIntIntH::TIter It = TableInt.BegI(); It < TableInt.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += It.GetKey();
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(0, Count);
    ASSERT_EQ(0, KeySum);
    ASSERT_EQ(0, DatSum);

    // clear the table
    TableInt1.Clr();
    ASSERT_EQ(0, TableInt1.Len());
    ASSERT_EQ(1, TableInt1.Empty());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TIntIntH::TIter It = TableInt1.BegI(); It < TableInt1.EndI(); It++) {
        ASSERT_EQ(0, It.GetDat() - It.GetKey() - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += It.GetKey();
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(0, Count);
    ASSERT_EQ(0, KeySum);
    ASSERT_EQ(0, DatSum);
}

// Test the default constructor
TEST(TStrIntHDefaultConstructor) {
    TStrIntH TableStr;

    ASSERT_EQ(1, TableStr.Empty());
    ASSERT_EQ(0, TableStr.Len());
    ASSERT_EQ(0, TableStr.GetMxKeyIds());
}

// Table manipulations
TEST(TStrIntHManipulateTable) {
    const int64 NElems = 1000000;
    int DDist = 10;
    const char *FName = "test.hashstr.dat";
    TStrIntH TableStr;
    TStrIntH TableStr1;
    TStrIntH TableStr2;
    int i;
    int d;
    int n;
    int Id;
    int Key;
    TStr KeyStr;
    int64 KeySumVal;
    int64 DatSumVal;
    int64 KeySum;
    int64 DatSum;
    int64 KeySumDel;
    int64 DatSumDel;
    int DelCount;
    int Count;
    char s[32];

    // add table elements
    d = Prime(NElems);
    n = d;
    KeySumVal = 0;
    DatSumVal = 0;
    for (i = 0; i < NElems; i++) {
        sprintf(s, "%d", n);
        TStr Str = TStr(s);
        TableStr.AddDat(Str, n + 1);
        KeySumVal += n;
        DatSumVal += (n + 1);
        //printf("add %d %d\n", n, n+1);
        n = (n + d) % NElems;
    }
    ASSERT_EQ(0, TableStr.Empty());
    ASSERT_EQ(NElems, TableStr.Len());

    ASSERT_EQ(0, (NElems - 1)*(NElems) / 2 - KeySumVal);
    ASSERT_EQ(0, (NElems)*(NElems + 1) / 2 - DatSumVal);

    // verify elements by successive keys
    KeySum = 0;
    DatSum = 0;
    for (i = 0; i < NElems; i++) {
        sprintf(s, "%d", i);
        TStr Str = TStr(s);
        Id = TableStr.GetKeyId(s);
        ASSERT_TRUE(Id >= 0);
        KeyStr = TableStr.GetKey(Id);
        Key = atoi(KeyStr.CStr());
        //printf("vrfy %d %s %d %s %d %d\n", i, Str.CStr(), Id, KeyStr.CStr(), Key, (int) TableStr.GetDat(KeyStr));
        ASSERT_EQ(0, TableStr.GetDat(KeyStr) - Key - 1);
        KeySum += Key;
        DatSum += TableStr.GetDat(KeyStr);
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by distant keys
    KeySum = 0;
    DatSum = 0;
    n = Prime(d);
    for (i = 0; i < NElems; i++) {
        sprintf(s, "%d", i);
        TStr Str = TStr(s);
        Id = TableStr.GetKeyId(s);
        ASSERT_TRUE(Id >= 0);
        KeyStr = TableStr.GetKey(Id);
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, TableStr.GetDat(KeyStr) - Key - 1);
        KeySum += Key;
        DatSum += TableStr.GetDat(KeyStr);
        n = (n + d) % NElems;
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    for (TStrIntH::TIter It = TableStr.BegI(); It < TableStr.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        KeySum += Key;
        DatSum += It.GetDat();
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // verify elements by key index
    KeySum = 0;
    DatSum = 0;
    Id = TableStr.FFirstKeyId();
    while (TableStr.FNextKeyId(Id)) {
        ASSERT_TRUE(Id >= 0);
        KeyStr = TableStr.GetKey(Id);
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, TableStr.GetDat(KeyStr) - Key - 1);
        KeySum += Key;
        DatSum += TableStr.GetDat(KeyStr);
    }

    ASSERT_EQ(0, KeySumVal - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSum);

    // delete elements
    DelCount = 0;
    KeySumDel = 0;
    DatSumDel = 0;
    for (n = 0; n < NElems; n += DDist) {
        sprintf(s, "%d", n);
        TStr Str = TStr(s);
        Id = TableStr.GetKeyId(Str);
        //printf("del %d %d %d\n", n, Id, (int) TableStr[Id]);
        KeySumDel += n;
        DatSumDel += TableStr[Id];
        TableStr.DelKeyId(Id);
        DelCount++;
    }
    ASSERT_EQ(0, TableStr.Empty());
    ASSERT_EQ(NElems - DelCount, TableStr.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TStrIntH::TIter It = TableStr.BegI(); It < TableStr.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += Key;
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // assignment
    TableStr1 = TableStr;
    ASSERT_EQ(0, TableStr1.Empty());
    ASSERT_EQ(NElems - DelCount, TableStr1.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TStrIntH::TIter It = TableStr1.BegI(); It < TableStr1.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += Key;
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // saving and loading
    {
        TFOut FOut(FName);
        TableStr.Save(FOut);
        FOut.Flush();
    }

    {
        TFIn FIn(FName);
        TableStr2.Load(FIn);
    }

    ASSERT_EQ(NElems - DelCount, TableStr2.Len());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TStrIntH::TIter It = TableStr2.BegI(); It < TableStr2.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += Key;
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(NElems - DelCount, Count);
    ASSERT_EQ(0, KeySumVal - KeySumDel - KeySum);
    ASSERT_EQ(0, DatSumVal - DatSumDel - DatSum);

    // remove all elements
    for (i = 0; i < Count; i++) {
        Id = TableStr.GetRndKeyId(TInt::Rnd, 0.5);
        TableStr.DelKeyId(Id);
    }
    ASSERT_EQ(0, TableStr.Len());
    ASSERT_EQ(1, TableStr.Empty());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TStrIntH::TIter It = TableStr.BegI(); It < TableStr.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += Key;
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(0, Count);
    ASSERT_EQ(0, KeySum);
    ASSERT_EQ(0, DatSum);

    // clear the table
    TableStr1.Clr();
    ASSERT_EQ(0, TableStr1.Len());
    ASSERT_EQ(1, TableStr1.Empty());

    // verify elements by iterator
    KeySum = 0;
    DatSum = 0;
    Count = 0;
    for (TStrIntH::TIter It = TableStr1.BegI(); It < TableStr1.EndI(); It++) {
        KeyStr = It.GetKey();
        Key = atoi(KeyStr.CStr());
        ASSERT_EQ(0, It.GetDat() - Key - 1);
        //printf("get %d %d\n", (int) It.GetKey(), (int) It.GetDat());
        KeySum += Key;
        DatSum += It.GetDat();
        Count++;
    }

    ASSERT_EQ(0, Count);
    ASSERT_EQ(0, KeySum);
    ASSERT_EQ(0, DatSum);
}

int Prime(const int& n) {
    int d;

    d = (int)((float)n * 0.61803398875);    // golden ratio
    d |= 0x01;                                // get an odd number
    while (!IsPrime(d)) {
        d += 2;
    }

    return d;
}

bool IsPrime(const int& d) {
    int i;
    int j;

    // 1, 2, 3 are primes
    if (d < 4 || (d & 0x01) == 0) {
        return true;
    }

    // even numbers are not primes
    if (d < 4 || (d & 0x01) == 0) {
        return true;
    }

    // test divisions with < sqrt(d)
    for (i = 2; i*i <= d; i++) {
        j = d / i;
        if (j*i == d) {
            return false;
        }
    }

    return true;
}


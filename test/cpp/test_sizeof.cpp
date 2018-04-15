#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

TEST(sizeofBasicTypes) {
    ASSERT_EQ(sizeof(TUCh), 1);
    ASSERT_EQ(sizeof(TCh), 1);
    ASSERT_EQ(sizeof(TSInt), 2);
    ASSERT_EQ(sizeof(TInt), 4);
    ASSERT_EQ(sizeof(TUInt64), 8);

    ASSERT_EQ(sizeof(TSFlt), 4);
    ASSERT_EQ(sizeof(TFlt), 8);
}

 TEST(sizeofBasicStructures) {
     ASSERT_EQ(sizeof(TVec<TInt>), 16);
     ASSERT_EQ(sizeof(THash<TInt, TInt>), 48);
     ASSERT_EQ(sizeof(THashSet<TInt>), 48);
     ASSERT_EQ(sizeof(TStrHash<TInt>), 56);
     ASSERT_EQ(sizeof(TQQueue<TInt>), 32);

     ASSERT_EQ(sizeof(TStr), 8);
     ASSERT_EQ(sizeof(TChA), 16);
     ASSERT_EQ(sizeof(TMem), 24);
     ASSERT_EQ(sizeof(TPt<TMem>), 8);
     ASSERT_EQ(sizeof(TWPt<TMem>), 8);

     ASSERT_EQ(sizeof(TBlobPt), 8);
     ASSERT_EQ(sizeof(TPgBlobPt), 8);
 }

 //TEST(sizeofQMiner) {
 //    /* ASSERT_EQ(sizeof(TQm::TRec), 216); */ // TODO check on Mac and Windows
 //    ASSERT_EQ(sizeof(TQm::TRec), 200);
 //    ASSERT_EQ(sizeof(TQm::TRecSet), 56);
 //    ASSERT_EQ(sizeof(TQm::TRecFilter), 24);
 //    ASSERT_EQ(sizeof(TQm::TAggr), 32);
 //    /* ASSERT_EQ(sizeof(TQm::TStreamAggr), 32); */
 //    ASSERT_EQ(sizeof(TQm::TStreamAggr), 40);
 //    ASSERT_EQ(sizeof(TQm::TFtrExt), 80);
 //    ASSERT_EQ(sizeof(TQm::TFtrSpace), 72);
 //}

TEST(GetExtraMemberSizeTStr) {
    const TStr Str = "abc";
    ASSERT_EQ(TMemUtils::GetExtraMemberSize(Str), Str.Len() + 1);
}

TEST(GetMemUsedTVec) {
    const TStr TargetStr = "abcdefg";
    const int NVecs = 4;
    const int StrMemUsed = TargetStr.GetMemUsed();
    const int VecMem = sizeof(TVec<TInt>) + NVecs*StrMemUsed;
    const int VVecMem = sizeof(TVec<TIntV>) + NVecs*VecMem;
    const int VVVecMem = sizeof(TVec<TVec<TStrV>>) + NVecs*VVecMem;

    TStrV StrV(NVecs);
    TVec<TStrV> StrVV(NVecs);
    TVec<TVec<TStrV>> StrVVV(NVecs);

    for (int i = 0; i < NVecs; i++) {
        StrV[i] = TargetStr;
    }

    for (int i = 0; i < NVecs; i++) {
        StrVV[i] = StrV;
    }

    for (int i = 0; i < NVecs; i++) {
        StrVVV[i] = StrVV;
    }

    // shallow
    ASSERT_EQ(StrV.GetMemUsed(false), sizeof(TStrV) + NVecs * sizeof(TStr));
    ASSERT_EQ(StrVV.GetMemUsed(false), sizeof(TVec<TStrV>) + NVecs * sizeof(TStrV));
    ASSERT_EQ(StrVVV.GetMemUsed(false), sizeof(TVec<TVec<TStrV>>) + NVecs * sizeof(TVec<TStrV>));
    // deep
    ASSERT_EQ(StrV.GetMemUsed(true), VecMem);
    ASSERT_EQ(StrVV.GetMemUsed(true), VVecMem);
    ASSERT_EQ(StrVVV.GetMemUsed(true), VVVecMem);
    // default behavior
    ASSERT_EQ(StrV.GetMemUsed(false), StrV.GetMemUsed());
    ASSERT_EQ(StrVV.GetMemUsed(false), StrVV.GetMemUsed());
    ASSERT_EQ(StrVVV.GetMemUsed(false), StrVVV.GetMemUsed());
}

TEST(GetMemUsedTHash) {
    const TStr TargetStr = "abcdefghijk";

    THash<TInt, TStr> TestH(2);
    const int BaseMemUsed = TestH.GetMemUsed(true);

    ASSERT_EQ(TestH.GetMemUsed(true), TestH.GetMemUsed(false));

    // check if adding one element is visible through deep GetMemUsed
    TestH.AddDat(1, TargetStr);
    ASSERT_EQ(TestH.GetMemUsed(true), BaseMemUsed + TMemUtils::GetExtraMemberSize(TargetStr));
    ASSERT_EQ(TestH.GetMemUsed(false), BaseMemUsed);

    TestH.AddDat(2, TargetStr);

    // check for two elements
    ASSERT_EQ(TestH.GetMemUsed(true), BaseMemUsed + 2 * TMemUtils::GetExtraMemberSize(TargetStr));
    ASSERT_EQ(TestH.GetMemUsed(false), BaseMemUsed);
}

TEST(GetMemUsedpointer) {
    double* NullPtr = NULL;
    TStrV* Vec = new TStrV;
    Vec->Add("Hello");
    Vec->Add("World");

    const double* ConstNullPtr = NullPtr;
    const TStrV* ConstVec = Vec;

    const uint64 VecMem = Vec->GetMemUsed(true);

    ASSERT_EQ(TMemUtils::GetMemUsed(NullPtr), sizeof(double*));
    ASSERT_EQ(TMemUtils::GetMemUsed(ConstNullPtr), sizeof(double*));
    ASSERT_EQ(TMemUtils::GetMemUsed(Vec), sizeof(TStrV*) + VecMem);
    ASSERT_EQ(TMemUtils::GetMemUsed(ConstVec), sizeof(TStrV*) + VecMem);
}

TEST(GetMemUsedreference) {
    TStrV Vec;
    Vec.Add("Hello");
    Vec.Add("World");

    TStrV& VecRef = Vec;
    const TStrV& ConstVecRef = Vec;

    ASSERT_EQ(TMemUtils::GetMemUsed(VecRef), Vec.GetMemUsed(true));
    ASSERT_EQ(TMemUtils::GetMemUsed(ConstVecRef), Vec.GetMemUsed(true));
}

TEST(GetMemUsedfundamental) {
    int Val = 3;
    ASSERT_EQ(TMemUtils::GetMemUsed(Val), sizeof(int));
}

TEST(GetMemUsedclazz) {
    const TStr StrVal = "abc";
    const TInt IntVal = 4;
    const TFlt FltVal = 5;

    ASSERT_EQ(TMemUtils::GetMemUsed(StrVal), sizeof(TStr) + StrVal.Len() + 1);
    ASSERT_EQ(TMemUtils::GetMemUsed(IntVal), sizeof(TInt));
    ASSERT_EQ(TMemUtils::GetMemUsed(FltVal), sizeof(FltVal));
}
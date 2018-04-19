#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

TEST(sizeofBasicTypes) {
    ASSERT_EQ(sizeof(TUCh), (uint)1);
    ASSERT_EQ(sizeof(TCh), (uint)1);
    ASSERT_EQ(sizeof(TSInt), (uint)2);
    ASSERT_EQ(sizeof(TInt), (uint)4);
    ASSERT_EQ(sizeof(TUInt64), (uint)8);

    ASSERT_EQ(sizeof(TSFlt), (uint)4);
    ASSERT_EQ(sizeof(TFlt), (uint)8);
}

 TEST(sizeofBasicStructures) {
     ASSERT_EQ(sizeof(TVec<TInt>), (uint)16);
     ASSERT_EQ(sizeof(THash<TInt, TInt>), (uint)48);
     ASSERT_EQ(sizeof(THashSet<TInt>), (uint)48);
     ASSERT_EQ(sizeof(TStrHash<TInt>), (uint)56);
     ASSERT_EQ(sizeof(TQQueue<TInt>), (uint)32);

     ASSERT_EQ(sizeof(TStr), (uint)8);
     ASSERT_EQ(sizeof(TChA), (uint)16);
     ASSERT_EQ(sizeof(TMem), (uint)24);
     ASSERT_EQ(sizeof(TPt<TMem>), (uint)8);
     ASSERT_EQ(sizeof(TWPt<TMem>), (uint)8);

     ASSERT_EQ(sizeof(TBlobPt), (uint)8);
     ASSERT_EQ(sizeof(TPgBlobPt), (uint)8);
 }

 TEST(sizeofQMiner) {
     /* ASSERT_EQ(sizeof(TQm::TRec), (uint)216); */ // TODO check on Mac and Windows
     //ASSERT_EQ(sizeof(TQm::TRec), (uint)200);
     ASSERT_EQ(sizeof(TQm::TRecSet), (uint)56);
     ASSERT_EQ(sizeof(TQm::TRecFilter), (uint)24);
     ASSERT_EQ(sizeof(TQm::TAggr), (uint)32);
     /* ASSERT_EQ(sizeof(TQm::TStreamAggr), (uint)32); */
     ASSERT_EQ(sizeof(TQm::TStreamAggr), (uint)40);
     ASSERT_EQ(sizeof(TQm::TFtrExt), (uint)80);
     ASSERT_EQ(sizeof(TQm::TFtrSpace), (uint)72);
 }

TEST(GetExtraMemberSizeTStr) {
    const TStr Str = "abc";
    ASSERT_EQ((int)TMemUtils::GetExtraMemberSize(Str), Str.Len() + 1);
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
    ASSERT_EQ(StrV.GetMemUsed(true), (uint)VecMem);
    ASSERT_EQ(StrVV.GetMemUsed(true), (uint)VVecMem);
    ASSERT_EQ(StrVVV.GetMemUsed(true), (uint)VVVecMem);
    // default behavior
    ASSERT_EQ(StrV.GetMemUsed(false), StrV.GetMemUsed());
    ASSERT_EQ(StrVV.GetMemUsed(false), StrVV.GetMemUsed());
    ASSERT_EQ(StrVVV.GetMemUsed(false), StrVVV.GetMemUsed());
}

TEST(GetMemUsedTHash) {
    const TStr TargetStr = "abcdefghijk";

    THash<TInt, TStr> TestH(2);
    const uint64 BaseMemUsed = TestH.GetMemUsed(true);

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
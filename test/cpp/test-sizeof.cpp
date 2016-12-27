/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include <base.h>
#include <qminer.h>


///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

///////////////////////////////////////////////////////////////////////////////

TEST(sizeof, BasicTypes) {
    ASSERT_EQ(sizeof(TUCh), 1);
    ASSERT_EQ(sizeof(TCh), 1);
    ASSERT_EQ(sizeof(TSInt), 2);
    ASSERT_EQ(sizeof(TInt), 4);
    ASSERT_EQ(sizeof(TUInt64), 8);
    
    ASSERT_EQ(sizeof(TSFlt), 4);
    ASSERT_EQ(sizeof(TFlt), 8);
}

TEST(sizeof, BasicStructures) {
    ASSERT_EQ(sizeof(TVec<TInt>), 16);
    ASSERT_EQ(sizeof(THash<TInt, TInt>), 48);
    ASSERT_EQ(sizeof(THashSet<TInt>), 48);
    ASSERT_EQ(sizeof(TStrHash<TInt>), 56);
    ASSERT_EQ(sizeof(TQQueue<TInt>), 32);
    
    ASSERT_EQ(sizeof(TStr), 8);
    ASSERT_EQ(sizeof(TChA), 16);
    ASSERT_EQ(sizeof(TMem), 32);
    ASSERT_EQ(sizeof(TPt<TMem>), 8);
    ASSERT_EQ(sizeof(TWPt<TMem>), 8);

    ASSERT_EQ(sizeof(TBlobPt), 8);
    ASSERT_EQ(sizeof(TPgBlobPt), 8);
}

TEST(sizeof, QMiner) {
    /* ASSERT_EQ(sizeof(TQm::TRec), 216); */ // TODO check on Mac and Windows
    ASSERT_EQ(sizeof(TQm::TRec), 200);
    ASSERT_EQ(sizeof(TQm::TRecSet), 56);
    ASSERT_EQ(sizeof(TQm::TRecFilter), 24);
    ASSERT_EQ(sizeof(TQm::TAggr), 32);
    ASSERT_EQ(sizeof(TQm::TStreamAggr), 40);
    ASSERT_EQ(sizeof(TQm::TFtrExt), 80);
    ASSERT_EQ(sizeof(TQm::TFtrSpace), 72);
}

TEST(GetMemUsedDeep, TVec) {
    const TStr TargetStr = "abcdefg";
    const int NVecs = 4;
    const int StrMemUsed = TargetStr.GetMemUsed();
    const int VecMem = sizeof(TVec<TInt>) + NVecs*StrMemUsed;
    const int VVecMem = sizeof(TVec<TIntV>) + NVecs*VecMem;
    const int VVVecMem = sizeof(TVec<TVec<TIntV>>) + NVecs*VVecMem;

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

    ASSERT_EQ(StrV.GetMemUsedDeep(), VecMem);
    ASSERT_EQ(StrVV.GetMemUsedDeep(), VVecMem);
    ASSERT_EQ(StrVVV.GetMemUsedDeep(), VVVecMem);
}

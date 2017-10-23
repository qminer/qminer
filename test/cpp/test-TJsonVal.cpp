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

TEST(TJsonVal, Parsing) {
    // test objects and arrays
    EXPECT_TRUE(TJsonVal::GetValFromStr("{ }")->IsObj());
    EXPECT_TRUE(TJsonVal::GetValFromStr("{\"a\":1}")->IsObj());
    EXPECT_TRUE(TJsonVal::GetValFromStr("[ ]")->IsArr());
    EXPECT_FALSE(TJsonVal::GetValFromStr("{ ]")->IsDef());
    // test basic types
    EXPECT_TRUE(TJsonVal::GetValFromStr("123")->IsNum());
    EXPECT_TRUE(TJsonVal::GetValFromStr("true")->IsBool());
    EXPECT_TRUE(TJsonVal::GetValFromStr("\"abc\"")->IsStr());
    EXPECT_TRUE(TJsonVal::GetValFromStr("null")->IsNull());

    // ignore escaped null character
    PJsonVal StrNullVal = TJsonVal::GetValFromStr("[\"xxx\\u0000yyy\"]");
    EXPECT_TRUE(StrNullVal->IsArr());
    EXPECT_EQ(StrNullVal->GetArrVals(), 1);
    EXPECT_TRUE(StrNullVal->GetArrVal(0)->IsStr());
    EXPECT_EQ(StrNullVal->GetArrVal(0)->GetStr(), "xxx yyy");

    // handling of escapes
    EXPECT_EQ(TJsonVal::GetValFromStr("\"\\t\"")->GetStr(), "\t");
    EXPECT_EQ(TJsonVal::GetValFromStr("\"\\R\"")->GetStr(), "R");
}

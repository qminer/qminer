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






TEST(TSlottedHistogramTest, Simple) {

	try {
		TQm::TVikTest::PSlottedHistogram obj = TQm::TVikTest::TSlottedHistogram::New(20, 2);

		obj->Add(1, "a");
		obj->Add(7, "a");
		obj->Add(12, "b");
		obj->Add(18, "a");
		obj->Add(22, "b");
		obj->Add(38, "a");

		TStrIntPrV Stats;
		obj->GetStats(41, 45, Stats);

		ASSERT_EQ(Stats.Len(), 2);
		ASSERT_EQ(Stats[0].Val1, "a");
		ASSERT_EQ(Stats[0].Val2, 1);
		ASSERT_EQ(Stats[1].Val1, "b");
		ASSERT_EQ(Stats[1].Val2, 1);
	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}
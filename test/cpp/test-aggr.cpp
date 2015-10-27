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

TEST(TSlottedHistogramTest, Simple1) {
	try {
		TQm::TStreamAggrs::PSlottedHistogram obj = TQm::TStreamAggrs::TSlottedHistogram::New(20, 2, 3);

		obj->Add(1, 0); // slot 0
		obj->Add(7, 0); // slot 3
		obj->Add(12, 1); // slot 6
		obj->Add(18, 0); // slot 9
		obj->Add(22, 1); // slot 1
		obj->Add(38, 0); // slot 9

		TFltV Stats;
		obj->GetStats(41, 45, Stats); // slots from 0 to 2 inclusive
		
		ASSERT_EQ(Stats.Len(), 3);
		ASSERT_EQ(Stats[0], 1.0);
		ASSERT_EQ(Stats[1], 1.0);
		ASSERT_EQ(Stats[2], 0.0);

		obj->GetStats(45, 47, Stats);

		ASSERT_EQ(Stats.Len(), 3);
		ASSERT_EQ(Stats[0], 1.0);
		ASSERT_EQ(Stats[1], 0.0);
		ASSERT_EQ(Stats[2], 0.0);

	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}

TEST(TSlottedHistogramTest, Simple2) {
	try {
		TQm::TStreamAggrs::PSlottedHistogram obj = TQm::TStreamAggrs::TSlottedHistogram::New(20, 5, 3);

		obj->Add(1, 0); // slot 0
		obj->Add(7, 0); // slot 1
		obj->Add(12, 1); // slot 2
		obj->Add(18, 0); // slot 3
		obj->Add(22, 1); // slot 0
		obj->Add(38, 0); // slot 3

		TFltV Stats;
		obj->GetStats(41, 45, Stats); // slots from 0 to 1 inclusive

		ASSERT_EQ(Stats.Len(), 3);
		ASSERT_EQ(Stats[0], 2.0);
		ASSERT_EQ(Stats[1], 1.0);
		ASSERT_EQ(Stats[2], 0.0);

		obj->GetStats(45, 47, Stats); // slots from 1 to 1 inclusive

		ASSERT_EQ(Stats.Len(), 3);
		ASSERT_EQ(Stats[0], 1.0);
		ASSERT_EQ(Stats[1], 0.0);
		ASSERT_EQ(Stats[2], 0.0);

	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}
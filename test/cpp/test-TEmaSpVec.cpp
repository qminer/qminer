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

TEST(TEmaSpVec, Simple1) {
	try {
		TSignalProc::TEmaSpVec sum(100, TSignalProc::TEmaType::etLinear, 0, 10000, 0.001);

		TSignalProc::TEma ema2(100, TSignalProc::TEmaType::etLinear, 0, 10000);
		TSignalProc::TEma ema5(100, TSignalProc::TEmaType::etLinear, 0, 10000);
		TSignalProc::TEma ema6(100, TSignalProc::TEmaType::etLinear, 0, 10000);

		uint64 timestamp1 = 10;
		TIntFltKdV in1;
		in1.Add(TIntFltKd(2, 1.0));
		
		sum.Update(in1, timestamp1);
		ema2.Update(1.0, timestamp1);
		ema5.Update(0.0, timestamp1);
		ema6.Update(0.0, timestamp1);

		EXPECT_EQ(sum.GetTmMSecs(), timestamp1);
		const TIntFltKdV& res1 = sum.GetValue();
		EXPECT_EQ(res1.Len(), 1);
		EXPECT_EQ(res1[0].Key, 2);
		EXPECT_EQ(res1[0].Dat, 1.0);

		// add another sparse vector, don't remove anything
		uint64 timestamp2 = timestamp1 + 1000;
		TIntFltKdV in2;
		in2.Add(TIntFltKd(5, 2.0));

		sum.Update(in2, timestamp2);
		ema2.Update(0.0, timestamp2);
		ema5.Update(2.0, timestamp2);
		ema6.Update(0.0, timestamp2);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp2);
		const TIntFltKdV& res2 = sum.GetValue();
		EXPECT_EQ(res2.Len(), 2);
		EXPECT_EQ(res2[0].Key, 2);
		EXPECT_EQ(res2[0].Dat, ema2.GetValue());
		EXPECT_EQ(res2[1].Key, 5);
		EXPECT_EQ(res2[1].Dat, ema5.GetValue());

		uint64 timestamp3 = timestamp2 + 1000;
		TIntFltKdV in3;
		in3.Add(TIntFltKd(5, 3.0));
		in3.Add(TIntFltKd(6, 6.0));

		sum.Update(in3, timestamp3);
		ema2.Update(0.0, timestamp3);
		ema5.Update(3.0, timestamp3);
		ema6.Update(6.0, timestamp3);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp3);
		TIntFltKdV res3(sum.GetValue());
		EXPECT_EQ(res3.Len(), 3);
		EXPECT_EQ(res3[0].Key, 2);
		EXPECT_EQ(res3[0].Dat, ema2.GetValue());
		EXPECT_EQ(res3[1].Key, 5);
		EXPECT_EQ(res3[1].Dat, ema5.GetValue());
		EXPECT_EQ(res3[2].Key, 6);
		EXPECT_EQ(res3[2].Dat, ema6.GetValue());
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}


TEST(TEmaSpVec, Simple2) {
	try {
		TSignalProc::TEma ema(100, TSignalProc::TEmaType::etLinear, 0, 100000);

		uint64 timestamp1 = 10;
		TIntFltKdV in1;
		in1.Add(TIntFltKd(2, 1.0));

		sum.Update(in1, timestamp1);
		ema2.Update(1.0, timestamp1);
		ema5.Update(0.0, timestamp1);
		ema6.Update(0.0, timestamp1);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		uint64 timestamp2 = timestamp1 + 1000;
		TIntFltKdV in2;
		in2.Add(TIntFltKd(5, 2.0));

		sum.Update(in2, timestamp2);
		ema2.Update(0.0, timestamp2);
		ema5.Update(2.0, timestamp2);
		ema6.Update(0.0, timestamp2);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp2);
		const TIntFltKdV& res2 = sum.GetValue();
		EXPECT_EQ(res2.Len(), 2);
		EXPECT_EQ(res2[0].Key, 2);
		EXPECT_EQ(res2[0].Dat, ema2.GetValue());
		EXPECT_EQ(res2[1].Key, 5);
		EXPECT_EQ(res2[1].Dat, ema5.GetValue());

		uint64 timestamp3 = timestamp2 + 1000;
		TIntFltKdV in3;
		in3.Add(TIntFltKd(5, 3.0));
		in3.Add(TIntFltKd(6, 6.0));

		sum.Update(in3, timestamp3);
		ema2.Update(0.0, timestamp3);
		ema5.Update(3.0, timestamp3);
		ema6.Update(6.0, timestamp3);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp3);
		TIntFltKdV res3(sum.GetValue());
		EXPECT_EQ(res3.Len(), 3);
		EXPECT_EQ(res3[0].Key, 2);
		EXPECT_EQ(res3[0].Dat, ema2.GetValue());
		EXPECT_EQ(res3[1].Key, 5);
		EXPECT_EQ(res3[1].Dat, ema5.GetValue());
		EXPECT_EQ(res3[2].Key, 6);
		EXPECT_EQ(res3[2].Dat, ema6.GetValue());

	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}



TEST(TEmaSpVec, Simple2) {
	try {
		TSignalProc::TEmaSpVec sum(100, TSignalProc::TEmaType::etLinear, 0, 100000, 0.001);

		TSignalProc::TEma ema2(100, TSignalProc::TEmaType::etLinear, 0, 100000);
		TSignalProc::TEma ema5(100, TSignalProc::TEmaType::etLinear, 0, 100000);
		TSignalProc::TEma ema6(100, TSignalProc::TEmaType::etLinear, 0, 100000);

		uint64 timestamp1 = 10;
		TIntFltKdV in1;
		in1.Add(TIntFltKd(2, 1.0));

		sum.Update(in1, timestamp1);
		ema2.Update(1.0, timestamp1);
		ema5.Update(0.0, timestamp1);
		ema6.Update(0.0, timestamp1);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		uint64 timestamp2 = timestamp1 + 1000;
		TIntFltKdV in2;
		in2.Add(TIntFltKd(5, 2.0));

		sum.Update(in2, timestamp2);
		ema2.Update(0.0, timestamp2);
		ema5.Update(2.0, timestamp2);
		ema6.Update(0.0, timestamp2);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp2);
		const TIntFltKdV& res2 = sum.GetValue();
		EXPECT_EQ(res2.Len(), 2);
		EXPECT_EQ(res2[0].Key, 2);
		EXPECT_EQ(res2[0].Dat, ema2.GetValue());
		EXPECT_EQ(res2[1].Key, 5);
		EXPECT_EQ(res2[1].Dat, ema5.GetValue());

		uint64 timestamp3 = timestamp2 + 1000;
		TIntFltKdV in3;
		in3.Add(TIntFltKd(5, 3.0));
		in3.Add(TIntFltKd(6, 6.0));

		sum.Update(in3, timestamp3);
		ema2.Update(0.0, timestamp3);
		ema5.Update(3.0, timestamp3);
		ema6.Update(6.0, timestamp3);
		printf("ema2: %f\n", ema2.GetValue());
		printf("ema5: %f\n", ema5.GetValue());
		printf("ema6: %f\n", ema6.GetValue());

		EXPECT_EQ(sum.GetTmMSecs(), timestamp3);
		TIntFltKdV res3(sum.GetValue());
		EXPECT_EQ(res3.Len(), 3);
		EXPECT_EQ(res3[0].Key, 2);
		EXPECT_EQ(res3[0].Dat, ema2.GetValue());
		EXPECT_EQ(res3[1].Key, 5);
		EXPECT_EQ(res3[1].Dat, ema5.GetValue());
		EXPECT_EQ(res3[2].Key, 6);
		EXPECT_EQ(res3[2].Dat, ema6.GetValue());

	} catch (PExcept& Except) {
		printf("Error: %s", Except->GetStr());
		throw Except;
	}
}


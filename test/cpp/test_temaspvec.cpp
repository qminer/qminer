#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

// TEST(TEmaSpVecSimple1) {
    // try {
        // auto params = TJsonVal::GetValFromStr("{ \"emaType\": \"linear\", \"interval\": 10000, \"initWindow\": 100 }");

        // //TSignalProc::TEmaSpVec sum(100, TSignalProc::TEmaType::etLinear, 0, 10000, 0.001);
        // TSignalProc::TEmaSpVec sum(params);

        // TSignalProc::TEma ema2(100, TSignalProc::TEmaType::etLinear, 0, 10000);
        // TSignalProc::TEma ema5(100, TSignalProc::TEmaType::etLinear, 0, 10000);
        // TSignalProc::TEma ema6(100, TSignalProc::TEmaType::etLinear, 0, 10000);

        // uint64 timestamp1 = 10;
        // TIntFltKdV in1;
        // in1.Add(TIntFltKd(2, 1.0));

        // sum.Update(in1, timestamp1);
        // ema2.Update(1.0, timestamp1);
        // ema5.Update(0.0, timestamp1);
        // ema6.Update(0.0, timestamp1);

        // ASSERT_EQ(sum.GetTmMSecs(), timestamp1);
        // const TIntFltKdV& res1 = sum.GetValue();
        // ASSERT_EQ(res1.Len(), 1);
        // ASSERT_EQ(res1[0].Key, 2);
        // ASSERT_EQ(res1[0].Dat, 1.0);

        // // add another sparse vector, don't remove anything
        // uint64 timestamp2 = timestamp1 + 1000;
        // TIntFltKdV in2;
        // in2.Add(TIntFltKd(5, 2.0));

        // sum.Update(in2, timestamp2);
        // ema2.Update(0.0, timestamp2);
        // ema5.Update(2.0, timestamp2);
        // ema6.Update(0.0, timestamp2);
        // printf("ema2: %f\n", ema2.GetValue());
        // printf("ema5: %f\n", ema5.GetValue());
        // printf("ema6: %f\n", ema6.GetValue());

        // ASSERT_EQ(sum.GetTmMSecs(), timestamp2);
        // const TIntFltKdV& res2 = sum.GetValue();
        // ASSERT_EQ(res2.Len(), 2);
        // ASSERT_EQ(res2[0].Key, 2);
        // ASSERT_EQ(res2[0].Dat, ema2.GetValue());
        // ASSERT_EQ(res2[1].Key, 5);
        // ASSERT_EQ(res2[1].Dat, ema5.GetValue());

        // uint64 timestamp3 = timestamp2 + 1000;
        // TIntFltKdV in3;
        // in3.Add(TIntFltKd(5, 3.0));
        // in3.Add(TIntFltKd(6, 6.0));

        // sum.Update(in3, timestamp3);
        // ema2.Update(0.0, timestamp3);
        // ema5.Update(3.0, timestamp3);
        // ema6.Update(6.0, timestamp3);
        // printf("ema2: %f\n", ema2.GetValue());
        // printf("ema5: %f\n", ema5.GetValue());
        // printf("ema6: %f\n", ema6.GetValue());

        // ASSERT_EQ(sum.GetTmMSecs(), timestamp3);
        // TIntFltKdV res3(sum.GetValue());
        // ASSERT_EQ(res3.Len(), 3);
        // ASSERT_EQ(res3[0].Key, 2);
        // ASSERT_EQ(res3[0].Dat, ema2.GetValue());
        // ASSERT_EQ(res3[1].Key, 5);
        // ASSERT_EQ(res3[1].Dat, ema5.GetValue());
        // ASSERT_EQ(res3[2].Key, 6);
        // ASSERT_EQ(res3[2].Dat, ema6.GetValue());
        // printf("ema2: %f\n", ema2.GetValue());
        // printf("ema5: %f\n", ema5.GetValue());
        // printf("ema6: %f\n", ema6.GetValue());

    // } catch (PExcept& Except) {
        // printf("Error: %s", Except->GetStr().CStr());
        // throw Except;
    // }
// }



// TEST(TEmaSimpleTest1) {
    // try {
        // TSignalProc::TEma ema(TSignalProc::TEmaType::etPreviousPoint, 0, 2000);

        // ema.Update(1.0, 1000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1, ema.GetValue());

        // ema.Update(1.0, 2000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1, ema.GetValue());

        // ema.Update(1.0, 3000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1, ema.GetValue());

        // ema.Update(2.0, 4000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1, ema.GetValue());

        // ema.Update(2.0, 5000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1.393469, ema.GetValue());

        // ema.Update(3.0, 6000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(1.632121, ema.GetValue());

        // ema.Update(3.0, 7000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(2.170339, ema.GetValue());

        // ema.Update(3.0, 8000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(2.496785, ema.GetValue());

        // ema.Update(4.0, 10000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(2.496785, ema.GetValue());

        // ema.Update(5.0, 30000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(2.496785, ema.GetValue());

        // ema.Update(5.0, 31000);
        // printf("ema: %f\n", ema.GetValue());
        // ASSERT_EQ(2.496785, ema.GetValue());

    // } catch (PExcept& Except) {
        // printf("Error: %s", Except->GetStr().CStr());
        // throw Except;
    // }
// }

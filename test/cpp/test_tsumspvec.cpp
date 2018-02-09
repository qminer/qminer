#include <base.h>
#include <mine.h>
#include "microtest.h"


TEST(TSumSpVecSimple1) {
    try {
        TSignalProc::TSumSpVec sum;

        uint64 timestamp1 = 10;
        TIntFltKdV in1;
        in1.Add(TIntFltKd(2, 1.0));

        TVec<TIntFltKdV> out1;
        TUInt64V out_ts1;
        sum.Update(in1, timestamp1, out1, out_ts1);

        ASSERT_EQ(sum.GetTmMSecs(), timestamp1);
        const TIntFltKdV& res1 = sum.GetValue();
        ASSERT_EQ(res1.Len(), 1);
        ASSERT_EQ(res1[0].Key, 2);
        ASSERT_EQ(res1[0].Dat, 1.0);

        // add another sparse vector, don't remove anything
        uint64 timestamp2 = 20;
        TIntFltKdV in2;
        in2.Add(TIntFltKd(5, 2.0));

        TVec<TIntFltKdV> out2;
        TUInt64V out_ts2;
        sum.Update(in2, timestamp2, out2, out_ts2);

        ASSERT_EQ(sum.GetTmMSecs(), timestamp2);
        const TIntFltKdV& res2 = sum.GetValue();
        ASSERT_EQ(res2.Len(), 2);
        ASSERT_EQ(res2[0].Key, 2);
        ASSERT_EQ(res2[0].Dat, 1.0);
        ASSERT_EQ(res2[1].Key, 5);
        ASSERT_EQ(res2[1].Dat, 2.0);

        // now remove the first vector
        uint64 timestamp3 = 25;
        TIntFltKdV in3;
        in3.Add(TIntFltKd(5, 3.0));
        in3.Add(TIntFltKd(6, 6.0));

        TVec<TIntFltKdV> out3;
        out3.Add(in1);
        TUInt64V out_ts3;
        out_ts3.Add(timestamp1);
        sum.Update(in3, timestamp3, out3, out_ts3);

        ASSERT_EQ(sum.GetTmMSecs(), timestamp3);
        TIntFltKdV res3(sum.GetValue());
        //sum.Pack(res3);
        ASSERT_EQ(res3.Len(), 2);
        ASSERT_EQ(res3[0].Key, 5);
        ASSERT_EQ(res3[0].Dat, 5.0);
        ASSERT_EQ(res3[1].Key, 6);
        ASSERT_EQ(res3[1].Dat, 6.0);

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

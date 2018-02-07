#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

TEST(TSlottedHistogramTestSimple1) {
    try {
        TSignalProc::TSlottedHistogram obj(20, 2, 3);

        obj.Add(1, 0); // slot 0
        obj.Add(7, 0); // slot 3
        obj.Add(12, 1); // slot 6
        obj.Add(18, 0); // slot 9
        obj.Add(22, 1); // slot 1
        obj.Add(38, 0); // slot 9

        TFltV Stats;
        obj.GetStats(41, 45, Stats); // slots from 0 to 2 inclusive

        ASSERT_EQ(Stats.Len(), 3);
        ASSERT_EQ(Stats[0], 1.0);
        ASSERT_EQ(Stats[1], 1.0);
        ASSERT_EQ(Stats[2], 0.0);

        obj.GetStats(45, 47, Stats);

        ASSERT_EQ(Stats.Len(), 3);
        ASSERT_EQ(Stats[0], 1.0);
        ASSERT_EQ(Stats[1], 0.0);
        ASSERT_EQ(Stats[2], 0.0);

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

TEST(TSlottedHistogramTestSimple2) {
    try {
        TSignalProc::TSlottedHistogram obj(20, 5, 3);

        obj.Add(1, 0); // slot 0
        obj.Add(7, 0); // slot 1
        obj.Add(12, 1); // slot 2
        obj.Add(18, 0); // slot 3
        obj.Add(22, 1); // slot 0
        obj.Add(38, 0); // slot 3

        TFltV Stats;
        obj.GetStats(41, 45, Stats); // slots from 0 to 1 inclusive

        ASSERT_EQ(Stats.Len(), 3);
        ASSERT_EQ(Stats[0], 2.0);
        ASSERT_EQ(Stats[1], 1.0);
        ASSERT_EQ(Stats[2], 0.0);

        obj.GetStats(45, 47, Stats); // slots from 1 to 1 inclusive

        ASSERT_EQ(Stats.Len(), 3);
        ASSERT_EQ(Stats[0], 1.0);
        ASSERT_EQ(Stats[1], 0.0);
        ASSERT_EQ(Stats[2], 0.0);

    } catch (PExcept& Except) {
        printf("Error: %s", Except->GetStr().CStr());
        throw Except;
    }
}

/*TEST(TTDigestTest, Simple3) {
try {
TFltV Quantiles;
Quantiles.Add(0.1);
Quantiles.Add(0.2);
Quantiles.Add(0.8);
Quantiles.Add(0.9);

TQm::TStreamAggrs::TTDigest* obj = new TQm::TStreamAggrs::TTDigest::New(Quantiles);

obj->Add(1);
obj->Add(2);
obj->Add(3);
obj->Add(4);
obj->Add(5);
obj->Add(6);
obj->Add(7);
obj->Add(8);
obj->Add(9);

} catch (PExcept& Except) {
printf("Error: %s", Except->GetStr());
throw Except;
}
}*/
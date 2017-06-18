/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <base.h>
#include <mine.h>
///////////////////////////////////////////////////////////////////////////////
// Google Test
#include "gtest/gtest.h"

#ifdef WIN32
#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif
#endif

using namespace TQuant;

void GenSamplesUniform(const int& NSamples, TIntV& SampleV, const bool& ShuffleP=true) {
    if (!SampleV.Empty()) { SampleV.Clr(); }
    for (int SampleN = 1; SampleN <= NSamples; SampleN++) {
        SampleV.Add(SampleN);
    }
    if (ShuffleP) {
        TRnd Rnd(1);
        SampleV.Shuffle(Rnd);
    }
}

template <typename TGk, typename TLowerBoundFun, typename TUpperBoundFun>
void AssertQuantileRange(TGk& Gk, const TLowerBoundFun& GetLowerBound,
        const TUpperBoundFun& GetUpperBound, const double QuantileStep=1e-3) {
    double CurrQuant = 0.0;
    while (CurrQuant <= 1.0) {
        const double QuantMn = GetLowerBound(CurrQuant);
        const double QuantMx = GetUpperBound(CurrQuant);

        const double EstQuant = Gk.Query(CurrQuant);

        std::cout << "quantile: " << CurrQuant << ", allowed: [" << QuantMn << ", " << QuantMx << "], value: " << EstQuant << std::endl;

        Gk.PrintSummary();
        std::cout << std::endl;

        ASSERT_GE(EstQuant, QuantMn);
        ASSERT_LE(EstQuant, QuantMx);

        CurrQuant += QuantileStep;
    }
}

TEST(TGreenwaldKhanna, Query) {
    const double Eps = .001;
    const int NSamples = 1000;

    TGk Gk(Eps, TGk::TCompressStrategy::csManual);

    for (int i = 1; i <= NSamples; i++) {
        Gk.Insert(i);
    }

    const double Tenth = Gk.Query(.1);
    const double First = Gk.Query(1e-6);
    const double Last = Gk.Query(1 - 1e-6);

    ASSERT_NEAR(.1*NSamples, Tenth, Eps*NSamples);
    ASSERT_NEAR(1e-6*NSamples, First, Eps*NSamples);
    ASSERT_NEAR((1 - 1e-6)*NSamples, Last, Eps*NSamples);
}

TEST(TGreenwaldKhanna, Compress) {
    const double Eps = .1;
    const int NSamples = 100;

    TGk Gk(Eps, TGk::TCompressStrategy::csManual);

    TFltV MeasurementV;
    for (int i = 1; i <= NSamples; i++) {
        MeasurementV.Add(i);
    }

    for (int TestN = 0; TestN < 100; TestN++) {
        TRnd Rnd;   MeasurementV.Shuffle(Rnd);

        for (int i = 0; i < NSamples; i++) {
            Gk.Insert(MeasurementV[i]);
        }

        Gk.Compress();

        // check if we have compressed
        ASSERT_TRUE(Gk.GetSummarySize() < NSamples);

        // test a grid
        double CurrPerc = .001;
        do {
            const double Perc = Gk.Query(CurrPerc);
            ASSERT_NEAR(std::ceil(CurrPerc*NSamples), Perc, Eps*NSamples);
            CurrPerc += .001;
        } while (CurrPerc < 1);


        // check some random quantiles if they are correct
        for (int i = 0; i < 10000; i++) {
            const double Prob = Rnd.GetUniDev();
            const double Perc = Gk.Query(Prob);

            ASSERT_NEAR(std::ceil(Prob*NSamples), Perc, Eps*NSamples);
        }
    }
}

TEST(TGreenwaldKhanna, LateManualCompress) {
    const double Eps = .1;
    const int NSamples = 100;

    TRnd Rnd;

    TGk Gk(Eps, TGk::TCompressStrategy::csManual);

    TFltV MeasurementV;
    for (int i = 1; i <= NSamples; i++) {
        MeasurementV.Add(i);
    }

    for (int RunN = 0; RunN < 100; RunN++) {
        MeasurementV.Shuffle(Rnd);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            Gk.Insert(MeasurementV[SampleN]);
        }
    }

    Gk.Compress();
    ASSERT_TRUE(Gk.GetSummarySize() < NSamples);

    for (int i = 0; i < 10; i++) {
        const double Prob = Rnd.GetUniDev();
        const double Perc = Gk.Query(Prob);

        ASSERT_NEAR(std::ceil(Prob*NSamples), Perc, Eps*NSamples);
    }
}

TEST(TGreenwaldKhanna, AutoCompress) {
    const double Eps = .1;
    const int NSamples = 100;

    TRnd Rnd;

    TGk Gk(Eps);

    TFltV MeasurementV;
    for (int i = 1; i <= NSamples; i++) {
        MeasurementV.Add(i);
    }

    for (int RunN = 0; RunN < 100; RunN++) {
        MeasurementV.Shuffle(Rnd);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            Gk.Insert(MeasurementV[SampleN]);
        }
    }

    ASSERT_TRUE(Gk.GetSummarySize() < NSamples);

    // test the accuracy
    double CurrPerc = .001;
    do {
        const double Perc = Gk.Query(CurrPerc);
        ASSERT_NEAR(std::ceil(CurrPerc*NSamples), Perc, Eps*NSamples);
        CurrPerc += .001;
    } while (CurrPerc < 1);
}

TEST(TBiasedGk, Query) {
    const int NTrials = 100;
    const int NSamples = 1000;
    const double Quant0 = .01;
    const double Eps0 = .1;

    TBiasedGk BandGk(Quant0, Eps0, true);
    TBiasedGk BandlessGk(Quant0, Eps0, false);

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            BandGk.Insert(SampleV[SampleN]);
            BandlessGk.Insert(SampleV[SampleN]);
        }

        const double QuantStep = .0001;
        double CurrQuant = QuantStep;
        while (CurrQuant < 1) {
            const double ActualBand = BandGk.Query(CurrQuant);
            const double ActualBandless = BandlessGk.Query(CurrQuant);

            const double Eps = CurrQuant < Quant0 ? Eps0 * Quant0 / CurrQuant : Eps0;
            const double LowerBound = std::floor((1 - Eps)*CurrQuant*NSamples);
            const double UpperBound = std::ceil((1 + Eps)*CurrQuant*NSamples);

            ASSERT_GE(ActualBand, LowerBound);
            ASSERT_GE(ActualBandless, LowerBound);
            ASSERT_LE(ActualBand, UpperBound);
            ASSERT_LE(ActualBandless, UpperBound);

            CurrQuant += QuantStep;
        }
    }

    ASSERT_TRUE(BandGk.GetSummarySize() <= BandlessGk.GetSummarySize());
}

TEST(TBiasedGk, HighQuantiles) {
    const int NTrials = 10;
    const int NSamples = 1000;
    const double Quant0 = 1 - .01;
    const double Eps0 = .1;

    TBiasedGk Gk(Quant0, Eps0);

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }

        const double QuantStep = .0001;
        double CurrQuant = QuantStep;
        while (CurrQuant < 1) {
            const double Eps = CurrQuant > Quant0 ? Eps0 * (1 - Quant0) / (1 - CurrQuant) : Eps0;

            const double Actual = Gk.Query(CurrQuant);
            const double Expected = CurrQuant*NSamples;

            ASSERT_LE(TMath::Abs(Actual - Expected), std::ceil(NSamples*Eps));

            CurrQuant += QuantStep;
        }
    }
}

// TODO uncomment test
/* TEST(TBiasedGk, OrderedInput) { */
/*     const uint64 NSamples = 10000; */
/*     const double Quant0 = .01; */
/*     /1* const double Quant0 = 0; *1/ */
/*     const double Eps = .1; */
/*     const bool UseBands = true; */

/*     TBiasedGk IncGk(Quant0, Eps, UseBands); */
/*     TBiasedGk DecGk(Quant0, Eps, UseBands); */
/*     TBiasedGk RandGk(Quant0, Eps, UseBands); */

/*     TIntV SampleV;  GenSamplesUniform(NSamples, SampleV, true); */

/*     for (uint64 SampleN = 0; SampleN < NSamples; SampleN++) { */
/*         const uint64 SampleRand = SampleV[SampleN]; */
/*         IncGk.Insert(SampleN + 1); */
/*         DecGk.Insert(NSamples - SampleN); */
/*         RandGk.Insert(SampleRand); */
/*     } */

/*     IncGk.Compress(); */
/*     DecGk.Compress(); */
/*     RandGk.Compress(); */

/*     IncGk.PrintSummary(); */
/*     DecGk.PrintSummary(); */
/*     RandGk.PrintSummary(); */

/*     // TODO test */
/* } */

TEST(TBiasedGk, ExtremeValues) {
    const int NSamples = 10000;
    const double Quant = .01;
    const double Eps = .1;

    TBiasedGk Gk(Quant, Eps);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleV[SampleN]);
    }

    ASSERT_EQ(Gk.Query(0), 1);
    ASSERT_EQ(Gk.Query(1), NSamples);
}

TEST(TBiasedGk, ZeroQ0) {
    const int NTrials = 10;
    const int NSamples = 10000;
    const double Quant0 = 0;
    const double Eps = .1;

    TBiasedGk Gk(Quant0, Eps);

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }

        const double QuantStep = .001;
        double CurrQuant = QuantStep;
        while (CurrQuant < 1) {
            const double Actual = Gk.Query(CurrQuant);

            const double LowerBound = std::floor((1 - Eps)*CurrQuant*NSamples);
            const double UpperBound = std::ceil((1 + Eps)*CurrQuant*NSamples);

            ASSERT_GE(Actual, LowerBound);
            ASSERT_LE(Actual, UpperBound);

            CurrQuant += QuantStep;
        }
    }
}

// TODO test space limitation


/* TEST(TExpHistogram, CountEquallySpaced) { */
/*     const uint64 WindowMSec = 10000; */
/*     const uint64 SampleInterval = 1000; */
/*     const int NSamples = 1000; */
/*     const double Eps = .1; */

/*     const int ExpectedCount = WindowMSec / SampleInterval; */

/*     TExpHistWithMax ExpHistMx(WindowMSec, Eps); */
/*     TExpHistogram ExpHist(WindowMSec, Eps); */

/*     uint64 CurrTm = WindowMSec; */
/*     for (int SampleN = 0; SampleN < NSamples; SampleN++) { */
/*         ExpHistMx.Add(CurrTm, 1); */
/*         ExpHist.Add(CurrTm); */

/*         if (SampleN*SampleInterval > WindowMSec) { */
/*             const int AllowedErr = std::ceil(Eps*ExpectedCount); */
/*             const int Low = ExpectedCount - AllowedErr; */
/*             const int High = ExpectedCount + AllowedErr; */
/*             ASSERT_GE(ExpHistMx.GetCount(CurrTm), Low); */
/*             ASSERT_LE(ExpHistMx.GetCount(CurrTm), High); */

/*             ASSERT_GE(ExpHist.GetCount(CurrTm), Low); */
/*             ASSERT_LE(ExpHist.GetCount(CurrTm), High); */
/*         } */

/*         CurrTm += SampleInterval; */
/*     } */
/* } */

/* TEST(TExpHistogram, CountNonEqual) { */
/*     const uint64 WindowMSec = 1000; */
/*     const double Eps = .1; */

/*     const uint64 StartTm = WindowMSec; */
/*     const uint64 EndTm = StartTm + 100*WindowMSec; */

/*     TExpHistWithMax ExpHistMx(WindowMSec, Eps); */
/*     TExpHistogram ExpHist(WindowMSec, Eps); */

/*     TRnd Rnd(1); */

/*     uint64 CurrTm = StartTm; */
/*     TUInt64V SampleTmV; */
    /* while (CurrTm < EndTm) { */
    /*     SampleTmV.Add(CurrTm); */
    /*     while (!SampleTmV.Empty() && SampleTmV[0] <= CurrTm - WindowMSec) { */
    /*         SampleTmV.Del(0); */
    /*     } */

    /*     ExpHistMx.Add(CurrTm, 0); */
    /*     ExpHist.Add(CurrTm); */

    /*     ASSERT_TRUE(ExpHistMx.CheckInvariant1()); */
    /*     ASSERT_TRUE(ExpHistMx.CheckInvariant2()); */
    /*     ASSERT_TRUE(ExpHist.CheckInvariant1()); */
    /*     ASSERT_TRUE(ExpHist.CheckInvariant2()); */

    /*     const int Expected = SampleTmV.Len(); */

    /*     const int AllowedErr = std::ceil(Eps*Expected); */
    /*     const int Low = Expected - AllowedErr; */
    /*     const int High = Expected + AllowedErr; */

    /*     const uint ActualMx = ExpHistMx.GetCount(CurrTm); */
    /*     const uint Actual = ExpHist.GetCount(CurrTm); */

    /*     ASSERT_GE(ActualMx, Low); */
    /*     ASSERT_LE(ActualMx, High); */

    /*     ASSERT_GE(Actual, Low); */
    /*     ASSERT_LE(Actual, High); */

    /*     CurrTm += Rnd.GetUniDevInt(10, 100); */
    /* } */
/* } */

/* TEST(TExpHistogram, Compression) { */
    /* const uint64 WindowMSec = 1000; */
    /* const uint64 DeltaTm = 10; */
    /* const double Eps = .1; */

    /* TExpHistWithMax ExpHistMx(WindowMSec, Eps); */
    /* TExpHistogram ExpHist(WindowMSec, Eps); */

    /* const uint NSteps = 1000; */

    /* const uint64 StartTm = 0; */
    /* for (uint StepN = 0; StepN < NSteps; StepN++) { */
    /*     const uint64 CurrTm = StartTm + StepN*DeltaTm; */
    /*     ExpHistMx.Add(CurrTm, 0); */
    /*     ExpHist.Add(CurrTm); */
    /* } */

    /* ASSERT_LE(ExpHistMx.GetSummarySize(), 36); */
    /* ASSERT_LE(ExpHist.GetSummarySize(), 36); */
/* } */

/* TEST(TExpHistogram, MxVal) { */
    /* const uint64 WindowMSec = 10000; */
    /* const uint64 MxValInterval = 200; */
    /* const uint64 SamplesInWindow = 100; */

    /* const uint64 SampleInterval = WindowMSec / SamplesInWindow; */

    /* const double Eps = .1; */

    /* const uint NSamples = 1000; */
    /* const uint64 MxHighStayTm = WindowMSec + std::ceil(2*Eps*WindowMSec); */

    /* TExpHistWithMax ExpHistMx(WindowMSec, Eps); */

    /* uint64 LastHighTm = 0; */
    /* const uint64 StartTm = WindowMSec; */
    /* for (uint SampleN = 0; SampleN < NSamples; SampleN++) { */
    /*     const uint64 CurrTm = StartTm + SampleN*SampleInterval; */
    /*     if (SampleN % MxValInterval == 0) { */
    /*         ExpHistMx.Add(CurrTm, 2); */
    /*         LastHighTm = CurrTm; */
    /*     } */
    /*     else { */
    /*         ExpHistMx.Add(CurrTm, 0); */
    /*     } */

    /*     const uint64 TmSinceHigh = CurrTm - LastHighTm; */

    /*     if (TmSinceHigh <= WindowMSec) { */
    /*         ASSERT_EQ(2, ExpHistMx.GetMxVal(CurrTm)); */
    /*     } */
    /*     else if (TmSinceHigh > MxHighStayTm) { */
    /*         ASSERT_EQ(0, ExpHistMx.GetMxVal(CurrTm)); */
    /*     } */
    /* } */
/* } */

/* TEST(TExpHistogram, SwallowBasic) { */
/*     const uint64 WindowMSec = 1000; */
/*     const double Eps = .1; */

/*     TExpHistogram HistBig(WindowMSec, Eps); */
/*     TExpHistogram HistSmall(WindowMSec, Eps); */

/*     const int NSamples = 100; */

/*     uint64 CurrTm = 0; */
/*     for (int SampleN = 0; SampleN < NSamples; SampleN++) { */
/*         if (SampleN % 2 == 0) { */
/*             HistBig.Add(CurrTm); */
/*         } */
/*         else if (SampleN % 3 == 0) { */
/*             HistSmall.Add(CurrTm); */
/*         } */
/*         ++CurrTm; */
/*     } */

/*     const int Count1 = HistBig.GetCount(CurrTm); */
/*     const int Count2 = HistSmall.GetCount(CurrTm); */

/*     HistBig.Swallow(HistSmall); */

/*     const int NewCount = HistBig.GetCount(CurrTm); */

/*     ASSERT_TRUE(HistBig.CheckInvariant1()); */
/*     ASSERT_TRUE(HistBig.CheckInvariant2()); */

/*     ASSERT_LE(NewCount, (1+Eps)*(Count1 + Count2)); */
/*     ASSERT_GE(NewCount, (1-Eps/2)*(Count1 + Count2)); */
/* } */

/* TEST(TExpHistogram, SwallowSameTSteps) { */
/*     const uint64 WindowMSec = 1000; */
/*     const double Eps = .1; */

/*     TExpHistogram Hist1(WindowMSec, Eps); */
/*     TExpHistogram Hist2(WindowMSec, Eps); */

/*     const int NSamples = 2000; */

/*     uint64 CurrTm = 0; */
/*     for (int SampleN = 0; SampleN < NSamples; SampleN++) { */
/*         if (SampleN % 2 == 0) { */
/*             Hist1.Add(CurrTm); */
/*         } */
/*         if (SampleN % 3 == 0) { */
/*             Hist2.Add(CurrTm); */
/*         } */

/*         const uint Count1 = Hist1.GetCount(CurrTm); */
/*         const uint Count2 = Hist2.GetCount(CurrTm); */

/*         Hist1.Swallow(Hist2); */

/*         const uint NewCount = Hist1.GetCount(CurrTm); */

/*         ASSERT_TRUE(Hist1.CheckInvariant1()); */
/*         ASSERT_TRUE(Hist1.CheckInvariant2()); */

/*         ASSERT_LE(NewCount, (1+Eps)*(Count1 + Count2)); */
/*         ASSERT_GE(NewCount, std::floor((1-Eps/2)*(Count1 + Count2))); */

/*         ++CurrTm; */
/*     } */

/*     const uint Count1 = Hist1.GetCount(CurrTm); */
/*     const uint Count2 = Hist2.GetCount(CurrTm); */

/*     Hist1.Swallow(Hist2); */

/*     const uint NewCount = Hist1.GetCount(CurrTm); */

/*     ASSERT_TRUE(Hist1.CheckInvariant1()); */
/*     ASSERT_TRUE(Hist1.CheckInvariant2()); */

/*     ASSERT_LE(NewCount, (1+Eps)*(Count1 + Count2)); */
/*     ASSERT_GE(NewCount, std::floor((1-Eps/2)*(Count1 + Count2))); */
/* } */

TEST(TExpHistogram, DelNewest) {
    const double Eps = .1;
    const uint64 WindowMSec = 100;

    TExpHistogram Eh(WindowMSec, Eps);

    const int MxBlocksPerSize = std::ceil(1 / (2*Eps)) + 1;

    for (int SampleN = 0; SampleN < MxBlocksPerSize+1; SampleN++) {
        Eh.Add(SampleN);
    }

    Eh.DelNewest();

    ASSERT_TRUE(Eh.CheckInvariant1());
    ASSERT_TRUE(Eh.CheckInvariant2());

    TExpHistogram Eh1(WindowMSec, Eps);
    for (int SampleN = 0; SampleN < 23; SampleN++) {
        Eh1.Add(SampleN);
    }
    
    Eh1.DelNewest();

    ASSERT_TRUE(Eh1.CheckInvariant1());
    ASSERT_TRUE(Eh1.CheckInvariant2());

    TExpHistogram Eh2(WindowMSec, Eps);
    for (int SampleN = 0; SampleN < 51; SampleN++) {
        Eh2.Add(SampleN);
    }

    Eh2.DelNewest();
    
    ASSERT_TRUE(Eh2.CheckInvariant1());
    ASSERT_TRUE(Eh2.CheckInvariant2());
}

/* TEST(TExpHistWithMax, SwallowSameTSteps) { */
/*     const uint64 WindowMSec = 100; */
/*     const double Eps = .1; */

/*     TExpHistWithMax Hist1(WindowMSec, Eps); */
/*     TExpHistWithMax Hist2(WindowMSec, Eps); */

/*     const int NSamples = 2000; */
/*     const int HighValInterval = 300; */

/*     uint64 CurrTm = 0; */
/*     int HighHistN = 0; */
/*     uint64 LastHighTm = 0; */
/*     for (int SampleN = 0; SampleN < NSamples; SampleN++) { */
/*         if (SampleN % 2 == 0) { */
/*             if (SampleN % HighValInterval == 0 && HighHistN == 0) { */
/*                 Hist1.Add(CurrTm, 1); */
/*                 HighHistN = 1 - HighHistN; */
/*                 LastHighTm = CurrTm; */
/*             } else { */
/*                 Hist1.Add(CurrTm, 0); */
/*             } */
/*         } */
/*         if (SampleN % 3 == 0) { */
/*             if (SampleN % HighValInterval == 0 && HighHistN == 1) { */
/*                 Hist2.Add(CurrTm, 1); */
/*                 HighHistN = 1 - HighHistN; */
/*                 LastHighTm = CurrTm; */
/*             } else { */
/*                 Hist2.Add(CurrTm, 0); */
/*             } */
/*         } */

/*         const uint Count1 = Hist1.GetCount(CurrTm); */
/*         const uint Count2 = Hist2.GetCount(CurrTm); */

/*         TExpHistWithMax Merged = Hist1; */
/*         Merged.Swallow(Hist2); */

/*         const uint NewCount = Merged.GetCount(CurrTm); */

/*         ASSERT_TRUE(Hist1.CheckInvariant1()); */
/*         ASSERT_TRUE(Hist1.CheckInvariant2()); */

/*         ASSERT_LE(NewCount, (1+Eps)*(Count1 + Count2)); */
/*         ASSERT_GE(NewCount, std::floor((1-Eps)*(Count1 + Count2))); */

/*         if (Hist1.GetMxVal(CurrTm) == 0 && Hist2.GetMxVal(CurrTm) == 0) { */
/*             ASSERT_EQ(Merged.GetMxVal(CurrTm), 0); */
/*         } */
/*         else if (CurrTm - LastHighTm <= WindowMSec) { */
/*             ASSERT_EQ(Merged.GetMxVal(CurrTm), 1); */
/*         } */

/*         ++CurrTm; */
/*     } */

/*     const uint Count1 = Hist1.GetCount(CurrTm); */
/*     const uint Count2 = Hist2.GetCount(CurrTm); */

/*     TExpHistWithMax Merged = Hist1; */
/*     Merged.Swallow(Hist2); */

/*     const uint NewCount = Merged.GetCount(CurrTm); */

/*     ASSERT_TRUE(Hist1.CheckInvariant1()); */
/*     ASSERT_TRUE(Hist1.CheckInvariant2()); */

/*     ASSERT_LE(NewCount, (1+Eps)*(Count1 + Count2)); */
/*     ASSERT_GE(NewCount, std::floor((1-Eps)*(Count1 + Count2))); */

/*     if (Hist1.GetMxVal(CurrTm) == 0 && Hist2.GetMxVal(CurrTm) == 0) { */
/*         ASSERT_EQ(Merged.GetMxVal(CurrTm), 0); */
/*     } */
/*     else if (CurrTm - LastHighTm <= WindowMSec) { */
/*         ASSERT_EQ(Merged.GetMxVal(CurrTm), 1); */
/*     } */
/* } */

TEST(TSwGk, QueryAccNoWindow) {
    const int BatchSize = 1000;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    /* const double EpsEh = .04; */
    const double EpsEh = 0;
    const uint64 WindowLen = TUInt64::Mx;

    const double MxRelErr = (EpsGk + 2*EpsEh);

    TSwGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }
        const int TotalSamples = (BatchN + 1)*BatchSize;
        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();

        std::cout << "allowed error: " << MxRelErr*BatchSize << std::endl;
        std::cout << "total samples: " << TotalSamples << std::endl;

        AssertQuantileRange(Gk, LowerBoundFun, UpperBoundFun);
    }
}

// TODO check that the invariants for an EH hold after deleting the newest element
// TODO test that empty histograms get deleted
// TODO implement OnTime  functionality for the windowed GK

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
    std::cout << "generating " << NSamples << " samples\n";
    if (!SampleV.Empty()) { SampleV.Clr(); }
    for (int SampleN = 1; SampleN <= NSamples; SampleN++) {
        SampleV.Add(SampleN);
    }
    if (ShuffleP) {
        TRnd Rnd(1);
        SampleV.Shuffle(Rnd);
    }
    std::cout << "samples generated\n";
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
    /* const double TargetPerc = .01; */
    const double Quant0 = .01;
    const double Eps0 = .1;

    TBiasedGk BandGk(Quant0, Eps0, true);
    TBiasedGk BandlessGk(Quant0, Eps0, false);

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        /* std::cout << "inserting samples\n"; */
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            BandGk.Insert(SampleV[SampleN]);
            BandlessGk.Insert(SampleV[SampleN]);
        }

        /* std::cout << "printing summary\n"; */

        /* std::cout << "querying\n"; */
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

    BandGk.PrintSummary();
    BandlessGk.PrintSummary();
    std::cout << "band summary size: " << BandGk.GetSummarySize() << ", bandless summary size: " << BandlessGk.GetSummarySize() << "\n";
    ASSERT_TRUE(BandGk.GetSummarySize() <= BandlessGk.GetSummarySize());
}

/* TEST(TBiasedGk, QuerySorted) { */
/*     // TODO */
/* } */

/* TEST(TBiasedGk, ExtremeValues) { */
/*     // TODO */
/* } */

/* TEST(TBiasedGk, Bands) { */
/*     const int NTrials = 5; */
/*     const int NSamples = 1000; */
/*     const double TargetPerc = .01; */
/*     const double TargetPercEps = .01; */

/*     TBiasedGk BandGk(TargetPerc, TargetPercEps, true); */
/*     TBiasedGk NonBandGk(TargetPerc, TargetPercEps, false); */

/*     for (int TrialN = 0; TrialN < NTrials; TrialN++) { */
/*         TIntV SampleV;  GenSamplesUniform(NSamples, SampleV); */
/*         for (int SampleN = 0; SampleN < NSamples; SampleN++) { */
/*             BandGk.Insert(SampleV[SampleN]); */
/*             NonBandGk.Insert(SampleV[SampleN]); */
/*         } */

/*         std::cout << "band summary: " << BandGk.GetSummarySize() << ", non-band summary: " << NonBandGk.GetSummarySize() << "\n"; */

/*         ASSERT_LT(BandGk.GetSummarySize(), NonBandGk.GetSummarySize()); */
/*     } */
/* } */

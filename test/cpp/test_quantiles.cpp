#include <base.h>
#include <mine.h>
#include <qminer.h>

#include "microtest.h"

using namespace TQuant;
using namespace TUtils;

void GenSamplesUniform(const int& NSamples, TIntV& SampleV, const bool& ShuffleP = true) {
    if (!SampleV.Empty()) { SampleV.Clr(); }
    for (int SampleN = 1; SampleN <= NSamples; SampleN++) {
        SampleV.Add(SampleN);
    }
    if (ShuffleP) {
        TRnd Rnd(1);
        SampleV.Shuffle(Rnd);
    }
}

double GetSwGkMxRelErr(const double& EpsGk, const double& EpsEh) {
    return EpsGk + 2 * EpsEh + EpsEh*EpsEh;
}

template <typename TGk, typename TLowerBoundFun, typename TUpperBoundFun>
void AssertQuantileRange(TGk& Gk, const TLowerBoundFun& GetLowerBound,
    const TUpperBoundFun& GetUpperBound, const double PValStep = 1e-3, const bool& PrintP = false) {

    for (double PVal = 0.0; PVal <= 1.0; PVal += PValStep) {
        const double QuantMn = GetLowerBound(PVal);
        const double QuantMx = GetUpperBound(PVal);

        const double EstQuant = Gk.Query(PVal);

        if (PrintP) {
            std::cout << "p-val: " << PVal << " (estimated) quantile: " << EstQuant << ", QuantMn: " << QuantMn << ", QuantMx: " << QuantMx << std::endl;
            Gk.PrintSummary();
        }

        ASSERT_GE(EstQuant, QuantMn);
        ASSERT_LE(EstQuant, QuantMx);
    }
}

template <typename TGk, typename TLowerBoundFun, typename TUpperBoundFun>
void AssertQuantileRangeV(TGk& Gk, const TLowerBoundFun& GetLowerBound,
    const TUpperBoundFun& GetUpperBound, const double PValStep = 1e-3, const bool& PrintP = false) {

    TFltV PValV;
    for (double PVal = 0.0; PVal <= 1.0; PVal += PValStep) {
        PValV.Add(PVal);
    }

    TFltV QuantV;   Gk.Query(PValV, QuantV);

    for (int PValN = 0; PValN < PValV.Len(); ++PValN) {
        const double PVal = PValV[PValN];
        const double QuantMn = GetLowerBound(PVal);
        const double QuantMx = GetUpperBound(PVal);

        const double EstQuant = Gk.Query(PVal);

        if (PrintP) {
            std::cout << "p-val: " << PVal << " (estimated) quantile: " << EstQuant << ", (vector est.) quantile: " << QuantV[PValN].Val << ", QuantMn: " << QuantMn << ", QuantMx: " << QuantMx << std::endl;
            Gk.PrintSummary();
        }

        ASSERT_GE(EstQuant, QuantMn);
        ASSERT_LE(EstQuant, QuantMx);
        ASSERT_EQ(EstQuant, QuantV[PValN]);
    }
}

template <typename TGk, typename TLowerBoundFun, typename TUpperBoundFun>
void AssertQuantileRangeVNew(TGk& Gk, const TLowerBoundFun& GetLowerBound,
    const TUpperBoundFun& GetUpperBound, const double PValStep = 1e-3, const bool& PrintP = false) {

    TFltV PValV;
    for (double PVal = 0.0; PVal <= 1.0; PVal += PValStep) {
        PValV.Add(PVal);
    }

    TFltV QuantV;   Gk.GetQuantileV(PValV, QuantV);

    for (int PValN = 0; PValN < PValV.Len(); ++PValN) {
        const double PVal = PValV[PValN];
        const double QuantMn = GetLowerBound(PVal);
        const double QuantMx = GetUpperBound(PVal);

        const double EstQuant = Gk.GetQuantile(PVal);

        if (PrintP) {
            std::cout << "p-val: " << PVal << " (estimated) quantile: " << EstQuant << ", (vector est.) quantile: " << QuantV[PValN].Val << ", QuantMn: " << QuantMn << ", QuantMx: " << QuantMx << std::endl;
            Gk.PrintSummary();
        }

        ASSERT_GE(EstQuant, QuantMn);
        ASSERT_LE(EstQuant, QuantMx);
        ASSERT_EQ(EstQuant, QuantV[PValN]);
    }
}

template <typename TGk, typename TLowerBoundFun, typename TUpperBoundFun>
void AssertCdfRangeV(TGk& Gk, const double& MnVal, const double& MxVal,
    const TLowerBoundFun& GetLowerBound, const TUpperBoundFun& GetUpperBound,
    const double Step = 1e-3, const bool& PrintP = false) {

    TFltV ValV;
    for (double Val = MnVal; Val <= MxVal; Val += Step) {
        ValV.Add(Val);
    }

    TFltV CdfValV;   Gk.GetCdfV(ValV, CdfValV);

    for (int ValN = 0; ValN < ValV.Len(); ++ValN) {
        const double Val = ValV[ValN];
        const double CdfMn = GetLowerBound(Val);
        const double CdfMx = GetUpperBound(Val);

        const double EstCdf = Gk.GetCdf(Val);

        if (PrintP) {
            std::cout << "value: " << Val << " (estimated) CDF: " << EstCdf << ", (vector est.) CDF: " << CdfValV[ValN].Val << ", CdfMn: " << CdfMn << ", CdfMx: " << CdfMx << std::endl;
            Gk.PrintSummary();
        }

        ASSERT_GE(EstCdf, CdfMn);
        ASSERT_LE(EstCdf, CdfMx);
        ASSERT_EQ(EstCdf, CdfValV[ValN]);
    }
}

TEST(TGreenwaldKhannaGetQuantile) {
    const double Eps = .001;
    const int NSamples = 1000;

    TGk Gk(Eps, TGk::TCompressStrategy::csManual);

    for (int i = 1; i <= NSamples; i++) {
        Gk.Insert(i);
    }

    const double Tenth = Gk.GetQuantile(.1);
    const double First = Gk.GetQuantile(1e-6);
    const double Last = Gk.GetQuantile(1 - 1e-6);

    ASSERT_NEAR(.1*NSamples, Tenth, Eps*NSamples);
    ASSERT_NEAR(1e-6*NSamples, First, Eps*NSamples);
    ASSERT_NEAR((1 - 1e-6)*NSamples, Last, Eps*NSamples);
}

TEST(TGreenwaldKhannaGetQuantileExtensive) {
    const int NTrials = 100;
    const int NSamples = 1000;

    const double Eps = 0.01;

    TRnd Rnd(1);

    TGk BandGk(Eps, Rnd, true);
    TGk BandlessGk(Eps, Rnd, false);

    const int MxError = Eps*NSamples;

    const auto LowerBoundFun = [&](const double& PVal) { return std::floor(PVal*NSamples - MxError); };
    const auto UpperBoundFun = [&](const double& PVal) { return std::ceil(PVal*NSamples + MxError); };

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            BandGk.Insert(SampleV[SampleN]);
            BandlessGk.Insert(SampleV[SampleN]);
        }

        AssertQuantileRangeVNew(BandGk, LowerBoundFun, UpperBoundFun, .0001, false);
        AssertQuantileRangeVNew(BandlessGk, LowerBoundFun, UpperBoundFun, .0001, false);
    }
}

TEST(TGkGetCdf) {
    const int NTrials = 100;
    const int BatchSize = 1000;

    const double Eps = 0.1;

    TGk Gk(Eps, TRnd(1));

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; ++SampleN) {
            Gk.Insert(SampleV[SampleN]);
        }

        SampleV.Sort();

        const auto LowerBound = [&](const double& Val) { return Val / BatchSize - Eps; };
        const auto UpperBound = [&](const double& Val) { return Val / BatchSize + Eps; };

        AssertCdfRangeV(Gk, 0, 1000, LowerBound, UpperBound, 1, false);
    }
}

TEST(TGreenwaldKhannaCompress) {
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
        const auto LowerBoundFun = [&](const double& PVal) { return std::floor(NSamples*(PVal - Eps)); };
        const auto UpperBoundFun = [&](const double& PVal) { return std::ceil(NSamples*(PVal + Eps)); };
        AssertQuantileRangeVNew(Gk, LowerBoundFun, UpperBoundFun);
    }
}

TEST(TGreenwaldKhannaLateManualCompress) {
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

    // test a grid
    const auto LowerBoundFun = [&](const double& PVal) { return std::floor(NSamples*(PVal - Eps)); };
    const auto UpperBoundFun = [&](const double& PVal) { return std::ceil(NSamples*(PVal + Eps)); };
    AssertQuantileRangeVNew(Gk, LowerBoundFun, UpperBoundFun);
}

TEST(TGreenwaldKhannaAutoCompress) {
    const double Eps = .1;
    const int BatchSize = 100;
    const int NBatches = 100;

    TRnd Rnd;

    TGk Gk(Eps);

    TFltV MeasurementV;
    for (int i = 1; i <= BatchSize; i++) {
        MeasurementV.Add(i);
    }

    const auto LowerBoundFun = [&](const double& PVal) { return std::floor(BatchSize*(PVal - Eps)); };
    const auto UpperBoundFun = [&](const double& PVal) { return std::ceil(BatchSize*(PVal + Eps)); };

    for (int RunN = 0; RunN < NBatches; RunN++) {
        MeasurementV.Shuffle(Rnd);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Gk.Insert(MeasurementV[SampleN]);
        }

        // test the accuracy
        AssertQuantileRangeVNew(Gk, LowerBoundFun, UpperBoundFun);
    }

    ASSERT_TRUE(Gk.GetSummarySize() < BatchSize);
}

TEST(TGreenwaldKhannaKolmogorovSmirnovFar) {
    const int Vals = 7;
    const double Eps = 0.001;
    const double Mean1 = 5;
    const double Mean2 = 0;

    const double Sigma = 1;

    TRnd Rnd(1);

    TGk GkLow(Eps, Rnd);
    TGk GkHigh(Eps, Rnd);

    for (int ValN = 0; ValN < Vals; ++ValN) {
        const double Val1 = Rnd.GetNrmDev(Mean1, Sigma, TFlt::Mn, TFlt::Mx);
        const double Val2 = Rnd.GetNrmDev(Mean2, Sigma, TFlt::Mn, TFlt::Mx);

        /* std::cout << "inserting" << std::endl; */
        GkLow.Insert(Val1);
        GkHigh.Insert(Val2);

        /* std::cout << "testing" << std::endl; */
        const double Statistic = TStat::KolmogorovSmirnov(GkLow, GkHigh);
        const bool AreDiffP = TStat::KolmogorovSmirnovTest(GkLow, GkHigh, 0.001);

        ASSERT_FALSE(AreDiffP);
        ASSERT_GE(Statistic, 0.9);
        /* std::cout << "n: " << (ValN+1) << "K-S statistic: " << Statistic << ", are diff: " << AreDiffP << std::endl; */
    }

    const double Val1 = Rnd.GetNrmDev(Mean1, Sigma, TFlt::Mn, TFlt::Mx);
    const double Val2 = Rnd.GetNrmDev(Mean2, Sigma, TFlt::Mn, TFlt::Mx);

    GkLow.Insert(Val1);
    GkHigh.Insert(Val2);

    const bool AreDiffP = TStat::KolmogorovSmirnovTest(GkLow, GkHigh, 0.001);

    ASSERT_TRUE(AreDiffP);
}

TEST(TGreenwaldKhannaKolmogorovSmirnovNear) {
    const int Vals = 41;
    const double Eps = 0.001;
    const double Mean1 = 1;
    const double Mean2 = 0;

    const double Sigma = 1;

    TRnd Rnd(1);

    TGk GkLow(Eps, Rnd);
    TGk GkHigh(Eps, Rnd);

    for (int ValN = 0; ValN < Vals; ++ValN) {
        const double Val1 = Rnd.GetNrmDev(Mean1, Sigma, TFlt::Mn, TFlt::Mx);
        const double Val2 = Rnd.GetNrmDev(Mean2, Sigma, TFlt::Mn, TFlt::Mx);

        /* std::cout << "inserting" << std::endl; */
        GkLow.Insert(Val1);
        GkHigh.Insert(Val2);

        /* std::cout << "testing" << std::endl; */
        const double Statistic = TStat::KolmogorovSmirnov(GkLow, GkHigh);
        const bool AreDiffP = TStat::KolmogorovSmirnovTest(GkLow, GkHigh, 0.001);

        ASSERT_FALSE(AreDiffP);
        ASSERT_GE(Statistic, 0.3);
        /* std::cout << "n: " << (ValN+1) << "K-S statistic: " << Statistic << ", are diff: " << AreDiffP << std::endl; */
    }

    const double Val1 = Rnd.GetNrmDev(Mean1, Sigma, TFlt::Mn, TFlt::Mx);
    const double Val2 = Rnd.GetNrmDev(Mean2, Sigma, TFlt::Mn, TFlt::Mx);

    GkLow.Insert(Val1);
    GkHigh.Insert(Val2);

    const bool AreDiffP = TStat::KolmogorovSmirnovTest(GkLow, GkHigh, 0.001);

    ASSERT_TRUE(AreDiffP);
}

TEST(TGreenwaldKhannaKolmogorovSmirnovSame) {
    const int Vals = 1000;
    const double Eps = 0.001;
    const double Mean1 = 0;
    const double Mean2 = 0;

    const double Sigma = 1;

    TRnd Rnd(1);

    TGk GkLow(Eps, Rnd);
    TGk GkHigh(Eps, Rnd);

    for (int ValN = 0; ValN < Vals; ++ValN) {
        const double Val1 = Rnd.GetNrmDev(Mean1, Sigma, TFlt::Mn, TFlt::Mx);
        const double Val2 = Rnd.GetNrmDev(Mean2, Sigma, TFlt::Mn, TFlt::Mx);

        /* std::cout << "inserting" << std::endl; */
        GkLow.Insert(Val1);
        GkHigh.Insert(Val2);

        /* std::cout << "testing" << std::endl; */
        const double Statistic = TStat::KolmogorovSmirnov(GkLow, GkHigh);
        const bool AreDiffP = TStat::KolmogorovSmirnovTest(GkLow, GkHigh, 0.001);

        ASSERT_FALSE(AreDiffP);
        if (ValN > 30) {
            ASSERT_LE(Statistic, 0.3);
        }
        if (ValN > 40) {
            ASSERT_LE(Statistic, 0.2);
        }
        /* std::cout << "n: " << (ValN+1) << "K-S statistic: " << Statistic << ", are diff: " << AreDiffP << std::endl; */
    }
}

TEST(TBiasedGkGetQuantile) {
    const int NTrials = 100;
    const int NSamples = 1000;

    const double Quant0 = .01;
    const double Eps0 = .1;

    TBiasedGk BandGk(Quant0, Eps0, TBiasedGk::TCompressStrategy::csPeriodic, true);
    TBiasedGk BandlessGk(Quant0, Eps0, TBiasedGk::TCompressStrategy::csPeriodic, false);

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            BandGk.Insert(SampleV[SampleN]);
            BandlessGk.Insert(SampleV[SampleN]);
        }

        const auto LowerBoundFun = [&](const double& PVal) {
            return std::floor(PVal <= Quant0 ? NSamples*(PVal - Eps0*Quant0) : NSamples*PVal*(1 - Eps0)); };
        const auto UpperBoundFun = [&](const double& PVal) {
            return std::ceil(PVal <= Quant0 ? NSamples*(PVal + Eps0*Quant0) : NSamples*PVal*(1 + Eps0)); };

        AssertQuantileRangeVNew(BandGk, LowerBoundFun, UpperBoundFun, .0001);
        AssertQuantileRangeVNew(BandlessGk, LowerBoundFun, UpperBoundFun, .0001);
    }
}

TEST(TBiasedGkHighQuantiles) {
    const int NTrials = 10;
    const int NSamples = 1000;
    const double TargetPVal = 1 - .01;
    const double Eps0 = .1;

    TBiasedGk Gk(TargetPVal, Eps0);

    for (int TrialN = 0; TrialN < NTrials; TrialN++) {
        TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }

        const auto LowerBoundFun = [&](const double& PVal) {
            return std::floor(PVal >= TargetPVal ? NSamples*(PVal - Eps0*(1 - TargetPVal)) : NSamples*(PVal - Eps0*(1 - PVal))); };
        const auto UpperBoundFun = [&](const double& PVal) {
            return std::ceil(PVal >= TargetPVal ? NSamples*(PVal + Eps0*(1 - TargetPVal)) : NSamples*(PVal + Eps0*(1 - PVal))); };

        AssertQuantileRangeVNew(Gk, LowerBoundFun, UpperBoundFun, .0001);
    }
}

TEST(TBiasedGkOrderedInput) {
    const uint64 NSamples = 10000;
    const double TargetPVal = .01;
    const double Eps = .1;
    const bool UseBands = true;

    TBiasedGk IncGk(TargetPVal, Eps, TBiasedGk::TCompressStrategy::csPeriodic, UseBands);
    TBiasedGk DecGk(TargetPVal, Eps, TBiasedGk::TCompressStrategy::csPeriodic, UseBands);

    for (uint64 SampleN = 0; SampleN < NSamples; SampleN++) {
        IncGk.Insert(SampleN + 1);
        DecGk.Insert(NSamples - SampleN);
    }

    IncGk.Compress();
    DecGk.Compress();

    const auto LowerBoundFun = [&](const double& PVal) {
        return std::floor(PVal >= TargetPVal ? NSamples*(PVal - Eps*(1 - TargetPVal)) : NSamples*(PVal - Eps*(1 - PVal))); };
    const auto UpperBoundFun = [&](const double& PVal) {
        return std::floor(PVal >= TargetPVal ? NSamples*(PVal + Eps*(1 - TargetPVal)) : NSamples*(PVal + Eps*(1 - PVal))); };

    AssertQuantileRangeVNew(IncGk, LowerBoundFun, UpperBoundFun, 0.0001);
    AssertQuantileRangeVNew(DecGk, LowerBoundFun, UpperBoundFun, 0.0001);
}

TEST(TBiasedGkExtremeValues) {
    const int NSamples = 10000;
    const double Quant = .01;
    const double Eps = .1;

    TBiasedGk Gk(Quant, Eps);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleV[SampleN]);
    }

    ASSERT_EQ(Gk.GetQuantile(0), 1);
    ASSERT_EQ(Gk.GetQuantile(1), NSamples);
}

TEST(TBiasedGkZeroQ0) {
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
            const double Actual = Gk.GetQuantile(CurrQuant);

            const double LowerBound = std::floor((1 - Eps)*CurrQuant*NSamples);
            const double UpperBound = std::ceil((1 + Eps)*CurrQuant*NSamples);

            ASSERT_GE(Actual, LowerBound);
            ASSERT_LE(Actual, UpperBound);

            CurrQuant += QuantStep;
        }
    }
}

TEST(TBiasedGkGetCdf) {
    const int NTrials = 100;
    const int NSamples = 1000;

    const double Cdf0 = 0.01;
    const double Eps = 0.1;

    TBiasedGk Gk(Cdf0, Eps, TRnd(1));

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; ++SampleN) {
            Gk.Insert(SampleV[SampleN]);
        }

        SampleV.Sort();

        for (int SampleN = 0; SampleN < NSamples; ++SampleN) {
            const double Val = SampleV[SampleN];

            const double Actual = Gk.GetCdf(Val);
            const double ExpectedCdf = Val / NSamples;

            const double LowerBound = std::floor(ExpectedCdf*NSamples*(1 - Eps)) / NSamples;
            const double UpperBound = std::ceil(ExpectedCdf*NSamples*(1 + Eps)) / NSamples;

            if (Actual < LowerBound || Actual > UpperBound) {
                std::cout << std::endl;
                Gk.PrintSummary();
                std::cout << "value: " << Val << ", ";
                std::cout << "n samples: " << Gk.GetSampleN() << std::endl;
                std::cout << "expected: " << ExpectedCdf << ", bounds: [" << LowerBound << ", " << UpperBound << "], actual: " << Actual << std::endl;
                std::cout << std::endl;
            }

            ASSERT_GE(Actual, LowerBound);
            ASSERT_LE(Actual, UpperBound);
        }
    }
}

TEST(TBiasedGkGetCdfNegDir) {
    const int NTrials = 100;
    const int NSamples = 1000;

    const double Cdf0 = 0.99;
    const double Eps = 0.1;

    TBiasedGk Gk(Cdf0, Eps, TRnd(1));

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(NSamples, SampleV);
        for (int SampleN = 0; SampleN < NSamples; ++SampleN) {
            Gk.Insert(SampleV[SampleN]);
        }

        SampleV.Sort();

        for (int SampleN = 0; SampleN < NSamples; ++SampleN) {
            const double Val = SampleV[SampleN];

            /* std::cout << std::endl << "value: " << Val << std::endl; */

            const double Actual = Gk.GetCdf(Val);
            const double ExpectedCdf = Val / NSamples;

            const double MxErr = ExpectedCdf <= Cdf0 ? (1.0 - ExpectedCdf)*Eps : (1 - Cdf0)*Eps;
            const double LowerBound = std::floor((ExpectedCdf - MxErr)*NSamples) / NSamples;
            const double UpperBound = std::ceil((ExpectedCdf + MxErr)*NSamples) / NSamples;

            /* std::cout << "expected: " << ExpectedCdf << ", [" << LowerBound << ", " << UpperBound << "], value: " << Actual << std::endl; */

            if (Actual < LowerBound || Actual > UpperBound) {
                Gk.PrintSummary();
                std::cout << "total samples: " << Gk.GetSampleN().Val << std::endl;
            }

            ASSERT_GE(Actual, LowerBound);
            ASSERT_LE(Actual, UpperBound);
        }
    }
}

// TODO test space limitation

TEST(TTDigestQuery) {
    const int BatchSize = 1000;
    const int NTrials = 100;

    const int MnCentroids = 100;

    TRnd Rnd(1);
    TTDigest TDigest(MnCentroids, Rnd);

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            const int Sample = SampleV[SampleN];
            TDigest.Insert(Sample);
        }

        const auto GetWorstPVal = [&](const double& PVal) {
            // in the worst case we just hit the centroid on the right
            // in that case, the p-value we need to calculate the error
            // is given by oequation:
            // p + eps = p_t = p + 2*p_t*(1-p_t)/c = p + 2*p_t/c - 2*p_t*p_t/c
            // 0 = p + p_t*(2/c-1) - 2/c*p_t^2
            // solution: 1/4 (2 - c + sqrt(4 - 4c + c^2 + 8cp))
            // analogous for the case were p > 0.5
            if (PVal <= 0.5) {
                const double WorstPVal = (2.0 - MnCentroids + TMath::Sqrt(4 - 4 * MnCentroids + MnCentroids*MnCentroids + 8 * MnCentroids*PVal)) / 4.0;
                if (WorstPVal > 0.5) {
                    return 0.5;
                } else {
                    return WorstPVal;
                }
            } else {
                const double WorstPVal = (2.0 + MnCentroids - TMath::Sqrt(4 + 4 * MnCentroids + MnCentroids*MnCentroids - 8 * MnCentroids*PVal)) / 4.0;
                if (WorstPVal < 0.5) {
                    return 0.5;
                } else {
                    return WorstPVal;
                }
            }
        };
        const auto LowerBound = [&](const double& PVal) {
            const double WorstPVal = GetWorstPVal(PVal);
            const double Eps = 2 * WorstPVal*(1 - WorstPVal) / MnCentroids;
            /* printf("lower bound: pval: %.5f, worst pval: %.5f\n", PVal, WorstPVal); */
            return std::floor(1 + (BatchSize - 1)*(PVal - Eps));
        };
        const auto UpperBound = [&](const double& PVal) {
            const double WorstPVal = GetWorstPVal(PVal);
            const double Eps = 2 * WorstPVal*(1 - WorstPVal) / MnCentroids;
            /* printf("pval: %.5f, worst pval: %.5f\n", PVal, WorstPVal); */
            return std::ceil(1 + (BatchSize - 1)*(PVal + Eps));
        };

        AssertQuantileRangeV(TDigest, LowerBound, UpperBound, 1e-3, false);
    }
    /* PeriodicTDigest.PrintSummary(); */
    /* TDigest.Insert(0, 100); */
    /* TDigest.PrintSummary(); */
}

TEST(TTDigestSummarySize) {
    const int BatchSize = 1000;
    const int NTrials = 100;

    const int MnCentroids = 100;

    TRnd Rnd(1);
    TTDigest TDigest(MnCentroids, Rnd);

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            TDigest.Insert(SampleV[SampleN]);
            /* if (SampleN == BatchSize-1) { */
            /*     std::cout << "summary size: " << TDigest.GetSummarySize() << std::endl; */
            /* } */
            ASSERT_LE(TDigest.GetSummarySize(), 20 * MnCentroids);
        }
    }
}

TEST(TTDigestSampleN) {
    const int BatchSize = 1000;
    const int NTrials = 100;

    const int MnCentroids = 100;

    TRnd Rnd(1);
    TTDigest TDigest(MnCentroids, Rnd);

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            TDigest.Insert(SampleV[SampleN]);
            ASSERT_EQ(TrialN*BatchSize + SampleN + 1, TDigest.GetSampleN());
        }
    }
}

TEST(TMergingTDigestQuery) {
    const int BatchSize = 1000;
    const int NTrials = 1000;

    const int Delta = 100;

    TRnd Rnd(1);
    TMergingTDigest TDigest(Delta, Rnd);

    TIntV SampleV;
    for (int TrialN = 0; TrialN < NTrials; ++TrialN) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            TDigest.Insert(SampleV[SampleN]);
        }

        TDigest.Flush();

        const auto GetWorstPVal = [&](const double& PVal) {
            // in the worst case we just hit the centroid on the right
            // in that case, the p-value we need to calculate the error
            // is given by oequation:
            // p + eps = p_t = p + 2*p_t*(1-p_t)/c = p + 2*p_t/c - 2*p_t*p_t/c
            // 0 = p + p_t*(2/c-1) - 2/c*p_t^2
            // solution: 1/4 (2 - c + sqrt(4 - 4c + c^2 + 8cp))
            // analogous for the case were p > 0.5
            if (PVal <= 0.5) {
                const double WorstPVal = (2.0 - Delta + TMath::Sqrt(4 - 4 * Delta + Delta*Delta + 8 * Delta*PVal)) / 4.0;
                if (WorstPVal > 0.5) {
                    return 0.5;
                } else {
                    return WorstPVal;
                }
            } else {
                const double WorstPVal = (2.0 + Delta - TMath::Sqrt(4 + 4 * Delta + Delta*Delta - 8 * Delta*PVal)) / 4.0;
                if (WorstPVal < 0.5) {
                    return 0.5;
                } else {
                    return WorstPVal;
                }
            }
        };
        const auto LowerBound = [&](const double& PVal) {
            const double WorstPVal = GetWorstPVal(PVal);
            const double Eps = 2 * WorstPVal*(1 - WorstPVal) / Delta;
            /* printf("lower bound: pval: %.5f, worst pval: %.5f\n", PVal, WorstPVal); */
            return std::floor(1 + (BatchSize - 1)*(PVal - Eps));
        };
        const auto UpperBound = [&](const double& PVal) {
            const double WorstPVal = GetWorstPVal(PVal);
            const double Eps = 2 * WorstPVal*(1 - WorstPVal) / Delta;
            /* printf("pval: %.5f, worst pval: %.5f\n", PVal, WorstPVal); */
            return std::ceil(1 + (BatchSize - 1)*(PVal + Eps));
        };

        AssertQuantileRangeV(TDigest, LowerBound, UpperBound, 1e-3, false);
        ASSERT_LE(TDigest.GetSummarySize(), 2 * Delta);
    }

    /*     TDigest.PrintSummary(); */
}

TEST(TExpHistogramCountEquallySpaced) {
    const uint64 WindowMSec = 10000;
    const uint64 SampleInterval = 1000;
    const int NSamples = 1000;
    const double Eps = .1;

    const int ExpectedCount = WindowMSec / SampleInterval;

    TExpHistWithMax ExpHistMx(Eps);
    TExpHistogram ExpHist(Eps);

    uint64 CurrTm = WindowMSec;
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        const int64 ForgetTm = int64(CurrTm) - int64(WindowMSec);

        ExpHistMx.Add(CurrTm, 1);
        ExpHist.Add(CurrTm);

        ExpHistMx.Forget(ForgetTm);
        ExpHist.Forget(ForgetTm);

        if (SampleN*SampleInterval > WindowMSec) {
            const int AllowedErr = std::ceil(Eps*ExpectedCount);
            const int Low = ExpectedCount - AllowedErr;
            const int High = ExpectedCount + AllowedErr;
            ASSERT_GE(ExpHistMx.GetCount(), Low);
            ASSERT_LE(ExpHistMx.GetCount(), High);

            ASSERT_GE(ExpHist.GetCount(), Low);
            ASSERT_LE(ExpHist.GetCount(), High);
        }

        CurrTm += SampleInterval;
    }
}

TEST(TExpHistogramCountNonEqual) {
    const uint64 WindowMSec = 1000;
    const double Eps = .1;

    const uint64 StartTm = WindowMSec;
    const uint64 EndTm = StartTm + 100 * WindowMSec;

    TExpHistWithMax ExpHistMx(Eps);
    TExpHistogram ExpHist(Eps);

    TRnd Rnd(1);

    uint64 CurrTm = StartTm;
    TUInt64V SampleTmV;
    while (CurrTm < EndTm) {
        const int64 ForgetTm = int64(CurrTm) - int64(WindowMSec);

        SampleTmV.Add(CurrTm);
        while (!SampleTmV.Empty() && SampleTmV[0] <= CurrTm - WindowMSec) {
            SampleTmV.Del(0);
        }

        ExpHistMx.Add(CurrTm, 0);
        ExpHist.Add(CurrTm);

        ExpHistMx.Forget(ForgetTm);
        ExpHist.Forget(ForgetTm);

        ASSERT_TRUE(ExpHistMx.CheckInvariant1());
        ASSERT_TRUE(ExpHistMx.CheckInvariant2());
        ASSERT_TRUE(ExpHist.CheckInvariant1());
        ASSERT_TRUE(ExpHist.CheckInvariant2());

        const int Expected = SampleTmV.Len();

        const int AllowedErr = std::ceil(Eps*Expected);
        const int Low = Expected - AllowedErr;
        const int High = Expected + AllowedErr;

        const uint ActualMx = ExpHistMx.GetCount();
        const uint Actual = ExpHist.GetCount();

        ASSERT_GE((int)ActualMx, Low);
        ASSERT_LE((int)ActualMx, High);

        ASSERT_GE((int)Actual, Low);
        ASSERT_LE((int)Actual, High);

        CurrTm += Rnd.GetUniDevInt(10, 100);
    }
}

TEST(TExpHistogramCompression) {
    const uint64 WindowMSec = 1000;
    const uint64 DeltaTm = 10;
    const double Eps = .1;

    TExpHistWithMax ExpHistMx(Eps);
    TExpHistogram ExpHist(Eps);

    const uint NSteps = 1000;

    const uint64 StartTm = 0;
    for (uint StepN = 0; StepN < NSteps; StepN++) {
        const uint64 CurrTm = StartTm + StepN*DeltaTm;
        ExpHistMx.Add(CurrTm, 0);
        ExpHist.Add(CurrTm);
    }

    const uint64 ForgetTm = StartTm + (NSteps - 1)*DeltaTm - WindowMSec;
    ExpHistMx.Forget(ForgetTm);
    ExpHist.Forget(ForgetTm);

    ASSERT_LE(ExpHistMx.GetSummarySize(), (uint)36);
    ASSERT_LE(ExpHist.GetSummarySize(), (uint)36);
}

TEST(TExpHistogramMxVal) {
    const uint64 WindowMSec = 100;
    const uint64 MxValInterval = 200;
    /* const uint64 SamplesInWindow = 100; */

    const double Eps = .1;

    const uint NSamples = 1000;
    /* const uint64 MxHighStayTm = WindowMSec + std::ceil(2*Eps*WindowMSec); */

    TExpHistWithMax ExpHistMx(Eps);

    uint64 LastHighTm = 0;
    /* const uint64 StartTm = WindowMSec; */
    for (uint SampleN = 0; SampleN < NSamples; SampleN++) {
        const uint64 CurrTm = SampleN;
        if (SampleN % MxValInterval == 0) {
            ExpHistMx.Add(CurrTm, 2);
            LastHighTm = CurrTm;
        } else {
            ExpHistMx.Add(CurrTm, 0);
        }

        const int64 ForgetTm = int64(CurrTm) - int64(WindowMSec);
        ExpHistMx.Forget(ForgetTm);

        const double MxVal = ExpHistMx.GetMxVal();

        if (TFlt::Abs(int64(LastHighTm) - ForgetTm) > Eps*WindowMSec) {
            if (int64(LastHighTm) < ForgetTm) {
                ASSERT_EQ(0, MxVal);
            } else {
                ASSERT_EQ(2, MxVal);
            }
        }
    }
}

TEST(TExpHistogramSwallowBasic) {
    const uint64 WindowMSec = 1000;
    const double Eps = .1;

    TExpHistogram HistBig(Eps);
    TExpHistogram HistSmall(Eps);

    const int NSamples = 100;

    uint64 CurrTm = 0;
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        if (SampleN % 2 == 0) {
            HistBig.Add(CurrTm);
        } else if (SampleN % 3 == 0) {
            HistSmall.Add(CurrTm);
        }
        HistBig.Forget(CurrTm - WindowMSec);
        ++CurrTm;
    }


    const int Count1 = HistBig.GetCount();
    const int Count2 = HistSmall.GetCount();

    HistBig.Swallow(HistSmall);

    const int NewCount = HistBig.GetCount();

    ASSERT_TRUE(HistBig.CheckInvariant1());
    ASSERT_TRUE(HistBig.CheckInvariant2());

    ASSERT_LE(NewCount, (1 + Eps)*(Count1 + Count2));
    ASSERT_GE(NewCount, (1 - Eps / 2)*(Count1 + Count2));
}

TEST(TExpHistogramSwallowSameTSteps) {
    const uint64 WindowMSec = 1000;
    const double Eps = .1;

    TExpHistogram Hist1(Eps);
    TExpHistogram Hist2(Eps);

    const int NSamples = 2000;

    uint64 CurrTm = 0;
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        if (SampleN % 2 == 0) {
            Hist1.Add(CurrTm);
        }
        if (SampleN % 3 == 0) {
            Hist2.Add(CurrTm);
        }

        const int64 ForgetTm = int64(CurrTm) - int64(WindowMSec);
        Hist1.Forget(ForgetTm);
        Hist2.Forget(ForgetTm);

        const uint Count1 = Hist1.GetCount();
        const uint Count2 = Hist2.GetCount();

        Hist1.Swallow(Hist2);

        const uint NewCount = Hist1.GetCount();

        ASSERT_TRUE(Hist1.CheckInvariant1());
        ASSERT_TRUE(Hist1.CheckInvariant2());

        ASSERT_LE(NewCount, (1 + Eps)*(Count1 + Count2));
        ASSERT_GE(NewCount, std::floor((1 - Eps / 2)*(Count1 + Count2)));

        ++CurrTm;
    }

    const int64 ForgetTm = int64(CurrTm) - int64(WindowMSec);
    Hist1.Forget(ForgetTm);
    Hist2.Forget(ForgetTm);

    const uint Count1 = Hist1.GetCount();
    const uint Count2 = Hist2.GetCount();

    Hist1.Swallow(Hist2);

    const uint NewCount = Hist1.GetCount();

    ASSERT_TRUE(Hist1.CheckInvariant1());
    ASSERT_TRUE(Hist1.CheckInvariant2());

    ASSERT_LE(NewCount, (1 + Eps)*(Count1 + Count2));
    ASSERT_GE(NewCount, std::floor((1 - Eps / 2)*(Count1 + Count2)));
}

TEST(TExpHistogramDelNewest) {
    const double Eps = .1;
    const uint64 WindowMSec = 100;

    TExpHistogram Eh(Eps);

    const int MxBlocksPerSize = (int)std::ceil(1 / (2 * Eps)) + 1;

    for (int SampleN = 0; SampleN < MxBlocksPerSize + 1; SampleN++) {
        Eh.Add(SampleN);
    }

    Eh.Forget(MxBlocksPerSize - WindowMSec);

    Eh.DelNewest();

    ASSERT_TRUE(Eh.CheckInvariant1());
    ASSERT_TRUE(Eh.CheckInvariant2());

    TExpHistogram Eh1(Eps);
    for (int SampleN = 0; SampleN < 23; SampleN++) {
        Eh1.Add(SampleN);
    }

    Eh1.Forget(22 - int(WindowMSec));

    Eh1.DelNewest();

    ASSERT_TRUE(Eh1.CheckInvariant1());
    ASSERT_TRUE(Eh1.CheckInvariant2());

    TExpHistogram Eh2(Eps);
    for (int SampleN = 0; SampleN < 51; SampleN++) {
        Eh2.Add(SampleN);
    }

    Eh1.Forget(50 - int(WindowMSec));

    Eh2.DelNewest();

    ASSERT_TRUE(Eh2.CheckInvariant1());
    ASSERT_TRUE(Eh2.CheckInvariant2());
}

TEST(TExpHistWithMaxDelNewestAcc) {
    const uint WindowMSec = 100;
    const double Eps = .1;

    TExpHistWithMax Hist(Eps);

    const int BatchSize = WindowMSec;
    const int NBatches = 20;

    TRnd Rnd(1);
    for (int BatchN = 0; BatchN < NBatches; BatchN++) {
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Hist.Add(BatchN*BatchSize + SampleN, Rnd.GetUniDevUInt(1000));
        }
        Hist.Forget((BatchN + 1)*BatchSize - WindowMSec);
        Hist.DelNewest();

        ASSERT_TRUE(Hist.CheckInvariant1());
        ASSERT_TRUE(Hist.CheckInvariant2());

        const uint Count = Hist.GetCount();

        ASSERT_LE(Count, WindowMSec*(1 + Eps) - 1);
        ASSERT_GE(Count, WindowMSec*(1 - Eps) - 1);
    }
}

TEST(TExpHistWithMaxDelNewestNonMax) {
    const uint WindowMSec = 100;
    const double Eps = .1;

    TExpHistWithMax HistRising(Eps);
    TExpHistWithMax HistRisingNewest(Eps);
    TExpHistWithMax HistFalling(Eps);

    const int BatchSize = WindowMSec;
    const int NBatches = 20;

    for (int BatchN = 0; BatchN < NBatches; BatchN++) {
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            const uint64 CurrTm = BatchN*BatchSize + SampleN;
            HistRising.Add(CurrTm, SampleN);
            HistRisingNewest.Add(CurrTm, SampleN);
            HistFalling.Add(CurrTm, BatchSize - SampleN);
        }

        const int64 ForgetTm = (BatchN + 1)*BatchSize - WindowMSec;

        HistRising.Forget(ForgetTm);
        HistRisingNewest.Forget(ForgetTm);
        HistFalling.Forget(ForgetTm);

        HistRising.DelNewestNonMx();
        HistRisingNewest.DelNewest();
        HistFalling.DelNewestNonMx();

        ASSERT_TRUE(HistRising.CheckInvariant1());
        ASSERT_TRUE(HistRising.CheckInvariant2());
        ASSERT_TRUE(HistFalling.CheckInvariant1());
        ASSERT_TRUE(HistFalling.CheckInvariant2());

        const uint CountRising = HistRising.GetCount();
        const uint CountFalling = HistFalling.GetCount();

        ASSERT_LE(CountRising, WindowMSec*(1 + Eps) - 1);
        ASSERT_GE(CountRising, WindowMSec*(1 - Eps) - 1);
        ASSERT_LE(CountFalling, WindowMSec*(1 + Eps) - 1);
        ASSERT_GE(CountFalling, WindowMSec*(1 - Eps) - 1);

        const double MxRising = HistRising.GetMxVal();
        const double MxRisingNewest = HistRisingNewest.GetMxVal();
        const double MxFalling = HistFalling.GetMxVal();

        ASSERT_EQ(BatchSize - 1, MxRising);
        ASSERT_EQ(BatchSize - 2, MxRisingNewest);
        ASSERT_LE(MxFalling, BatchSize);
        ASSERT_GE(MxFalling, BatchSize*(1 - Eps));
    }
}

TEST(TExpHistWithMaxDelNewestNonMaxEdgeCaseEps) {
    const uint WindowMSec = 100;
    const double Eps = .5;

    TExpHistWithMax HistRising(Eps);
    TExpHistWithMax HistFalling(Eps);

    const int BatchSize = WindowMSec;

    for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
        HistRising.Add(SampleN, SampleN);
        HistFalling.Add(SampleN, BatchSize - SampleN);
    }

    const int64 ForgetTm = BatchSize - WindowMSec;
    HistRising.Forget(ForgetTm);
    HistFalling.Forget(ForgetTm);

    HistRising.DelNewestNonMx();
    HistFalling.DelNewestNonMx();

    ASSERT_TRUE(HistRising.CheckInvariant1());
    ASSERT_TRUE(HistRising.CheckInvariant2());
    ASSERT_TRUE(HistFalling.CheckInvariant1());
    ASSERT_TRUE(HistFalling.CheckInvariant2());

    const uint CountRising = HistRising.GetCount();
    const uint CountFalling = HistFalling.GetCount();

    ASSERT_LE(CountRising, WindowMSec*(1 + Eps) - 1);
    ASSERT_GE(CountRising, WindowMSec*(1 - Eps) - 1);
    ASSERT_LE(CountFalling, WindowMSec*(1 + Eps) - 1);
    ASSERT_GE(CountFalling, WindowMSec*(1 - Eps) - 1);

    const double MxRising = HistRising.GetMxVal();
    const double MxFalling = HistFalling.GetMxVal();

    ASSERT_EQ(BatchSize - 1, MxRising);
    ASSERT_EQ(BatchSize, MxFalling);
}

TEST(TExpHistWithMaxSwallowSameTSteps) {
    const uint64 WindowMSec = 100;
    const double Eps = .1;

    TExpHistWithMax Hist1(Eps);
    TExpHistWithMax Hist2(Eps);

    const int NSamples = 2000;
    const int HighValInterval = 300;

    uint64 CurrTm = 0;
    int HighHistN = 0;
    /* uint64 LastHighTm = 0; */

    TUInt64V Hist1CountV, Hist2CountV;
    int CountSinceHigh1 = 0;
    int CountSinceHigh2 = 0;

    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        if (SampleN % 2 == 0) {
            if (SampleN % HighValInterval == 0 && HighHistN == 0) {
                Hist1.Add(CurrTm, 1);
                HighHistN = 1 - HighHistN;
                /* LastHighTm = CurrTm; */
                CountSinceHigh1 = 0;
            } else {
                Hist1.Add(CurrTm, 0);
                ++CountSinceHigh1;
            }
            Hist1CountV.Add(CurrTm);
        }
        if (SampleN % 3 == 0) {
            if (SampleN % HighValInterval == 0 && HighHistN == 1) {
                Hist2.Add(CurrTm, 1);
                HighHistN = 1 - HighHistN;
                /* LastHighTm = CurrTm; */
                CountSinceHigh2 = 0;
            } else {
                Hist2.Add(CurrTm, 0);
                ++CountSinceHigh2;
            }
            Hist2CountV.Add(CurrTm);
        }

        const int64 ForgetTm = CurrTm - WindowMSec;
        Hist1.Forget(ForgetTm);
        Hist2.Forget(ForgetTm);

        while (!Hist1CountV.Empty() && int64(Hist1CountV[0]) <= ForgetTm) {
            Hist1CountV.Del(0);
        }
        while (!Hist2CountV.Empty() && int64(Hist2CountV[0]) <= ForgetTm) {
            Hist2CountV.Del(0);
        }

        const uint Count1 = Hist1CountV.Len();
        const uint Count2 = Hist2CountV.Len();
        const uint TotalCount = Hist1CountV.Len() + Hist2CountV.Len();

        TExpHistWithMax Merged = Hist1;
        Merged.Swallow(Hist2);

        Merged.Forget(ForgetTm);

        const uint NewCount = Merged.GetCount();

        ASSERT_TRUE(Merged.CheckInvariant1());
        ASSERT_TRUE(Merged.CheckInvariant2());

        ASSERT_LE(NewCount, std::ceil((1 + Eps)*TotalCount));
        ASSERT_GE(NewCount, std::floor((1 - Eps)*TotalCount));

        if (CountSinceHigh1 > (1 + Eps)*Count1 && CountSinceHigh2 > (1 + Eps)*Count2) {
            ASSERT_EQ(Hist1.GetMxVal(), 0);
            ASSERT_EQ(Hist2.GetMxVal(), 0);
            ASSERT_EQ(Merged.GetMxVal(), 0);
        } else if (CountSinceHigh1 < (1 - Eps)*Count1 || CountSinceHigh2 <= (1 - Eps)*Count2) {
            ASSERT_TRUE(Hist1.GetMxVal() > 0 || Hist2.GetMxVal() > 0);
            ASSERT_EQ(Merged.GetMxVal(), 1);
        }

        ++CurrTm;
    }
}

TEST(TWindowMinQuery) {
    const uint NSamples = 1000;

    const uint64 WindowLen = 100;
    const double Eps = .1;

    TWindowMin WinMin(Eps);

    uint MinInterval = 180;
    int LastMnTm = TInt::Mn;

    for (uint CurrTm = 0; CurrTm < NSamples; CurrTm++) {
        if (CurrTm % MinInterval == 0) {
            WinMin.Add(CurrTm, 0);
            LastMnTm = CurrTm;
        } else {
            WinMin.Add(CurrTm, 1);
        }

        const int ForgetTm = int(CurrTm) - int(WindowLen);
        WinMin.Forget(ForgetTm);

        const double MnVal = WinMin.GetMnVal();

        if (LastMnTm > ForgetTm) {
            ASSERT_EQ(0, MnVal);
        } else {
            ASSERT_EQ(1, MnVal);
        }
    }
}

TEST(TWindowMinForgetLarge) {
    const uint64 WindowLen = 100;
    const double Eps = .1;

    TWindowMin WinMin(Eps);

    for (uint SampleN = 0; SampleN < WindowLen; SampleN++) {
        WinMin.Add(SampleN, SampleN + 1);
        WinMin.Forget(SampleN - WindowLen);
    }

    WinMin.Add(WindowLen + 1, 0);

    Assert(WinMin.GetSummarySize() == 1);
}

TEST(TCountWindowGkQueryAccNoWindow) {
    const int BatchSize = 100;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    const double EpsEh = 0;
    const uint64 WindowLen = TInt64::Mx;

    const double MxRelErr = GetSwGkMxRelErr(EpsGk, EpsEh);

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }
        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();

        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);

        if (BatchN > 0) {
            const int TotalSamples = (BatchN + 1)*BatchSize;

            const uint64 ItemCount = Gk.GetValCount();
            const uint64 ItemRecount = Gk.GetValRecount();

            const uint64 LowerBound = (uint64)std::floor(TotalSamples*(1 - EpsEh));
            const uint64 UpperBound = (uint64)std::ceil(TotalSamples*(1 + EpsEh));

            ASSERT_GE(ItemCount, LowerBound);
            ASSERT_LE(ItemCount, UpperBound);

            ASSERT_EQ(ItemCount, ItemRecount);
        }
    }
}

TEST(TCountWindowGkQueryAccWindow) {
    const int BatchSize = 1000;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    const double EpsEh = 0;
    const uint64 WindowLen = 1000;

    const double MxRelErr = GetSwGkMxRelErr(EpsGk, EpsEh);

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }

        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();
        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);
        ASSERT_LE(Gk.GetValCount(), std::ceil((1 + EpsEh)*WindowLen));
    }
}

TEST(TCountWindowGkQuery) {
    const int BatchSize = 1000;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    const double EpsEh = .05;
    const uint64 WindowLen = 1000;

    const double MxRelErr = (EpsGk + 2 * EpsEh);

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            Gk.Insert(SampleV[SampleN]);
        }

        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();
        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);
        ASSERT_LE(Gk.GetValCount(), std::ceil((1 + EpsEh)*WindowLen));
    }
}

TEST(TCountWindowGkConceptDrift) {
    const int BatchSize = 1000;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    const double EpsEh = .05;
    const uint64 WindowLen = BatchSize;

    const double MxRelErr = GetSwGkMxRelErr(EpsGk, EpsEh);

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        const uint BatchStart = BatchSize*BatchN;

        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            const uint Val = BatchStart + SampleV[SampleN];
            Gk.Insert(Val);
        }


        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchStart + BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchStart + BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();

        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);
        ASSERT_LE(Gk.GetValCount(), std::ceil((1 + EpsEh)*WindowLen));
    }
}

TEST(TCountWindowGkItemCountExact) {
    const int NSamples = 1000;

    const int WindowLen = 10;
    const double EpsGk = .1;
    const double EpsEh = 0;

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleV[SampleN]);

        if (SampleN > 0 && SampleN % (10 * WindowLen) == 0) {
            Gk.Compress();

            const uint64 ValCount = Gk.GetValCount();
            const uint64 ValRecount = Gk.GetValRecount();

            ASSERT_EQ(WindowLen, ValCount);
            ASSERT_EQ(ValCount, ValRecount);
        }
    }

    Gk.Compress();

    const uint64 ValCount = Gk.GetValCount();
    const uint64 ValRecount = Gk.GetValRecount();

    ASSERT_EQ(WindowLen, ValCount);
    ASSERT_EQ(ValCount, ValRecount);
}

TEST(TCountWindowGkItemCountApprox) {
    const int NSamples = 100;

    const int WindowLen = 10;
    const double EpsGk = .1;
    const double EpsEh = .05;

    TCountWindowGk Gk(WindowLen, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleV[SampleN]);
    }

    Gk.Compress();

    const uint64 ValCount = Gk.GetValCount();

    ASSERT_EQ(ValCount, Gk.GetValRecount());
    ASSERT_GE(ValCount, std::floor((1 - EpsEh)*WindowLen));
    ASSERT_LE(ValCount, std::ceil((1 + EpsEh)*WindowLen));
}

TEST(TTimeWindowGkQuery) {

    const double EpsGk = .1;
    const double EpsEh = .05;
    const uint64 WindowMSec = 10000;
    const uint64 DeltaTm = 10;

    const int SamplesInWindow = WindowMSec / DeltaTm;

    const int BatchSize = SamplesInWindow;
    const int TotalBatches = 10;

    const double MxRelErr = (EpsGk + 2 * EpsEh);

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            const uint64 CurrTm = DeltaTm*(BatchN*BatchSize + SampleN);
            Gk.Insert(CurrTm, SampleV[SampleN]);
        }

        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();
        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);
        ASSERT_LE(Gk.GetValCount(), std::ceil((1 + EpsEh)*SamplesInWindow));
    }
}

TEST(TTimeWindowGkConceptDrift) {
    const uint64 WindowMSec = 10000;
    const uint64 DeltaTm = 10;
    const int BatchSize = WindowMSec / DeltaTm;
    const int TotalBatches = 10;

    const double EpsGk = .1;
    const double EpsEh = .05;

    const double MxRelErr = GetSwGkMxRelErr(EpsGk, EpsEh);

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;
    for (int BatchN = 0; BatchN < TotalBatches; BatchN++) {
        const uint BatchStart = BatchSize*BatchN;

        GenSamplesUniform(BatchSize, SampleV);
        for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
            const uint Val = BatchStart + SampleV[SampleN];
            const uint64 CurrTm = (BatchN*BatchSize + SampleN)*DeltaTm;
            Gk.Insert(CurrTm, Val);
        }

        const auto LowerBoundFun = [&](const double& Quant) { return std::floor(BatchStart + BatchSize*TMath::Mx(0.0, Quant - MxRelErr)); };
        const auto UpperBoundFun = [&](const double& Quant) { return std::ceil(BatchStart + BatchSize*TMath::Mn(1.0, Quant + MxRelErr)); };

        // test if the values are in range
        Gk.Compress();
        AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);
        ASSERT_LE(Gk.GetValCount(), std::ceil((1 + EpsEh)*BatchSize));
    }
}

TEST(TTimeWindowGkItemCountExact) {
    const int NSamples = 1000;

    const int WindowMSec = 100;
    const double EpsGk = .1;
    const double EpsEh = 0;

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    TUInt64V SampleTmV;

    TRnd Rnd(1);
    uint64 CurrTm = 0;
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        CurrTm += Rnd.GetUniDevInt(WindowMSec / 2);
        const int64 ForgetTm = CurrTm - WindowMSec;

        Gk.Insert(CurrTm, SampleV[SampleN]);

        SampleTmV.Add(CurrTm);
        while (!SampleTmV.Empty() && int64(SampleTmV[0]) <= ForgetTm) {
            SampleTmV.Del(0);
        }

        if (SampleN > 0 && SampleN % 100 == 0) {
            Gk.Compress();

            const uint64 ValCount = Gk.GetValCount();
            const uint64 ValRecount = Gk.GetValRecount();

            ASSERT_EQ(SampleTmV.Len(), ValCount);
            ASSERT_EQ(SampleTmV.Len(), ValRecount);
        }
    }

    Gk.Compress();

    const uint64 ValCount = Gk.GetValCount();
    const uint64 ValRecount = Gk.GetValRecount();

    ASSERT_EQ(SampleTmV.Len(), ValCount);
    ASSERT_EQ(SampleTmV.Len(), ValRecount);
}

TEST(TTimeWindowGkItemCountApprox) {
    const int NSamples = 1000;

    const int WindowMSec = 100;
    const double EpsGk = .1;
    const double EpsEh = .05;

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    TUInt64V SampleTmV;

    TRnd Rnd(1);
    uint64 CurrTm = 0;
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        CurrTm += Rnd.GetUniDevInt(WindowMSec / 2);
        const int64 ForgetTm = CurrTm - WindowMSec;

        Gk.Insert(CurrTm, SampleV[SampleN]);

        SampleTmV.Add(CurrTm);
        while (!SampleTmV.Empty() && int64(SampleTmV[0]) <= ForgetTm) {
            SampleTmV.Del(0);
        }

        if (SampleN > 0 && SampleN % 100 == 0) {
            Gk.Compress();

            const uint64 ValCount = Gk.GetValCount();
            const uint64 ValRecount = Gk.GetValRecount();

            ASSERT_EQ(ValCount, ValRecount);
            ASSERT_LE(ValCount, std::ceil(SampleTmV.Len())*(1 + EpsEh));
            ASSERT_GE(ValCount, std::floor(SampleTmV.Len())*(1 - EpsEh));
        }
    }

    Gk.Compress();

    const uint64 ValCount = Gk.GetValCount();
    const uint64 ValRecount = Gk.GetValRecount();

    ASSERT_EQ(ValCount, ValRecount);
    ASSERT_LE(ValCount, std::ceil(SampleTmV.Len())*(1 + EpsEh));
    ASSERT_GE(ValCount, std::floor(SampleTmV.Len())*(1 - EpsEh));
}

TEST(TTimeWindowDrainSummary) {
    const uint64 WindowMSec = 1000;
    const uint64 DeltaTm = 10;
    const int SamplesInWindow = WindowMSec / DeltaTm;

    const double EpsGk = .1;
    const double EpsEh = .05;

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(SamplesInWindow, SampleV);

    for (int SampleN = 0; SampleN < SamplesInWindow; SampleN++) {
        const uint64 Tm = SampleN*DeltaTm;
        Gk.Insert(Tm, SampleV[SampleN]);
    }

    for (int SampleN = 0; SampleN < 2 * SamplesInWindow; SampleN++) {
        const uint64 Tm = (SamplesInWindow + SampleN)*DeltaTm;
        Gk.UpdateTime(Tm);
    }

    const auto ZeroFun = [&](const double&) { return 0.0; };

    Gk.Compress();
    ASSERT_EQ(0, Gk.GetValCount());
    ASSERT_EQ(0, Gk.GetValRecount());
    AssertQuantileRangeV(Gk, ZeroFun, ZeroFun);
}

TEST(TTimeWindowAutoCompress) {
    const uint64 WindowMSec = 10000;
    const int BatchSize = WindowMSec;

    const double EpsGk = .1;
    const double EpsEh = .05;

    TTimeWindowGk Gk(WindowMSec, EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(BatchSize, SampleV);

    for (int SampleN = 0; SampleN < BatchSize; SampleN++) {
        const uint64 Tm = SampleN;
        Gk.Insert(Tm, SampleV[SampleN]);
    }

    ASSERT_LE(Gk.GetSummarySize(), 300);
}

TEST(TSwGkQuery) {
    const int NSamples = 10000;
    const int WindowLen = 1000;

    const double EpsGk = .1;
    const double EpsEh = .05;

    TSwGk Gk(EpsGk, EpsEh);
    TSwGk GkWin(EpsGk, EpsEh);

    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleN, SampleN);
        GkWin.Insert(SampleN, SampleN);
        GkWin.Forget(SampleN - WindowLen);
    }

    const double MxRelErr = GetSwGkMxRelErr(EpsGk, EpsEh);

    const auto LowerBoundFun = [&](const double& Quantile) { return std::floor(NSamples*(Quantile - MxRelErr)); };
    const auto UpperBoundFun = [&](const double& Quantile) { return std::ceil(NSamples*(Quantile + MxRelErr)); };

    AssertQuantileRangeV(Gk, LowerBoundFun, UpperBoundFun);

    const int ForgetTm = NSamples - WindowLen;

    const auto NewLowerBoundFun = [&](const double& Quantile) { return std::floor(ForgetTm + WindowLen*(Quantile - MxRelErr)); };
    const auto NewUpperBoundFun = [&](const double& Quantile) { return std::ceil(ForgetTm + WindowLen*(Quantile + MxRelErr)); };

    AssertQuantileRangeV(GkWin, NewLowerBoundFun, NewUpperBoundFun);
}

TEST(TSwGkDrainedSummary) {
    const int NSamples = 1000;

    const double EpsGk = .1;
    const double EpsEh = .05;

    TSwGk Gk(EpsGk, EpsEh);

    TIntV SampleV;  GenSamplesUniform(NSamples, SampleV);
    for (int SampleN = 0; SampleN < NSamples; SampleN++) {
        Gk.Insert(SampleN, SampleV[SampleN]);
    }

    Gk.Forget(NSamples);

    const auto ZeroFun = [&](const double&) { return 0.0; };
    AssertQuantileRangeV(Gk, ZeroFun, ZeroFun);
}
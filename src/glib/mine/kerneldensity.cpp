/**
 * Copyright (c) 2016, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

//////////////////////////////////////////////////////////////////////////////////
/// Histogram to PMF model
THistogramToPMFModel::THistogramToPMFModel(const PJsonVal& ParamVal) {
    Tol = ParamVal->GetObjNum("tol", 1e-8);
    AutoBandwidthTol = ParamVal->GetObjNum("autoBandwidthTol", 1e-8);
    MinBandwidth = ParamVal->GetObjNum("minBandwidth", 0.01);
    TStr ModelType = ParamVal->GetObjStr("pmfModel", "kdeNormal");
    if (ModelType == "kdeNormal") {
        Type = hpmfKDEGaussian;
    } else if (ModelType == "l1") {
        Type = hpmfL1;
    } else {
        throw TExcept::New("Unknown histogram PMF model type");
    }

    if (ParamVal->IsObjKey("bandwidth")) {
        Bandwidth = ParamVal->GetObjNum("bandwidth");
        AutoBandwidthP = false;
    } else {
        AutoBandwidthP = true;
    }

    if (ParamVal->IsObjKey("autoBandwidthsGrid")) {
        ParamVal->GetObjFltV("autoBandwidthsGrid", AutoBandwidthV);
    }
    if (ParamVal->IsObjKey("autoThresholdsGrid")) {
        ParamVal->GetObjFltV("autoThresholdsGrid", AutoThresholdV);
        AutoThresholdV.Sort(false); // thresholds should be sorted in descending order
    }

    if (ParamVal->IsObjKey("thresholds")) {
        try {
            ParamVal->GetObjKey("thresholds")->GetArrNumV(Thresholds);
        } catch (PExcept Except) {
            throw TExcept::New("THistogramToPMFModel thresholds parameter should be"
                " a numeric vector of values between 0 and 1, additional info: "
                + Except->GetMsgStr(), Except->GetLocStr());
        }
        // Thresholds should be in descending order
        Thresholds.Sort(false);
    } else {
        // defaults
        Thresholds.Gen(3, 0);
        Thresholds.Add(0.05);
        Thresholds.Add(0.01);
        Thresholds.Add(0.001);
    }
    // Other models that could be implemented:
    //   laplace smoothing
    //   fit 1 gaussian
}

void THistogramToPMFModel::ClassifyAnomalies(const TFltV& PMF, TFltV& Severities) {
    TFltV SortedV; TIntV PermV;
    // Sort in ascending order: most severe anomalies come first
    PMF.SortGetPerm(PMF, SortedV, PermV, true);
    int Len = PMF.Len();
    Severities = TFltV(Len); // default zero
    double CumSum = 0.0;
    int Severity = Thresholds.Len();
    int StepBack = 0;
    for (int ElN = 0; ElN < Len; ElN++) {
        CumSum += SortedV[ElN];
        // check if the next cell has the same mass up to a tolerance

        if ((ElN < Len - 1) && TMath::Abs(SortedV[ElN] - SortedV[ElN + 1]) < Tol) {
            // if it does
            StepBack++;
            continue;
        } else {
            // Decrease severity if the cumulative sum is above the threshold
            while ((Severity > 0) && (CumSum > Thresholds[Severity - 1])) {
                Severity--;
            }
            if (Severity == 0) { break; } // this group and the following will have 0 Severity
            for (int StepN = 0; StepN <= StepBack; StepN++) {
                Severities[PermV[ElN - StepN]] = Severity;
            }
            StepBack = 0;
        }
    }
}

void THistogramToPMFModel::GetPMF(const TFltV& Hist, TFltV& PMF, const bool& ComputeBandwidthP) {
    PMF = Hist;
    if (Type == hpmfL1) {
        TLinAlg::NormalizeL1(PMF);
    } else if (Type == hpmfKDEGaussian) {
        // Bandwidth in units of histogram cell width
        if (AutoBandwidthP && (ComputeBandwidthP)) {
            if (AutoBandwidthV.Len() != 0) {
                Bandwidth = TKDEModel::LeaveOneOutRateBandwidth(Hist, AutoBandwidthV, AutoThresholdV, THistogramPMFModelType::hpmfKDEGaussian, AutoBandwidthTol);
            } else {
                Bandwidth = TKDEModel::RuleOfThumbHistBandwidth(Hist);
            }
            Bandwidth = Bandwidth < MinBandwidth ? MinBandwidth : Bandwidth;
        }
        TKDEModel::ComputeHistDensity(Hist, PMF, Bandwidth);
    } else {
        throw TExcept::New("Unknown histogram PMF model type");
    }
}

//////////////////////////////////////////////////////////////////////////////////
/// Kernel-Density-Estimation
double TKDEModel::EvalKernel(const double& Data, const double& Eval, const double& Bandwidth, const THistogramPMFModelType& KType) {
    const double GCoef = 1.0 / TMath::Sqrt(2 * TMath::Pi);
    const double Scale = 1.0 / Bandwidth;
    const double U = TMath::Abs(Data - Eval) / Bandwidth;

    switch (KType) {
    case hpmfKDEGaussian: return Scale * GCoef * TMath::Power(TMath::E, -0.5 * TMath::Sqr(U));
        break;
    case hpmfKDETophat: return U < 1.0 ? Scale * 0.5 : 0.0;
        break;
    case hpmfKDEEpanechnikov: return  U < 1.0 ? Scale * 0.75 * (1 - TMath::Sqr(U)) : 0.0;
        break;
    default: throw TExcept::New("Unsupported kernel type");
        break;
    }
}

void TKDEModel::ComputeHistDensity(const TFltV& Hist, TFltV& Dens, const double& Bandwidth, const THistogramPMFModelType& KType) {
    int Len = Hist.Len();
    Dens.Gen(Len);
    /// Compute kernels for all possible distances
    TFltV Precompute(Len);
    for (int HistElN = 0; HistElN < Len; HistElN++) {
        Precompute[HistElN] = EvalKernel(HistElN, 0, Bandwidth, KType);
    }
    for (int HistElN = 0; HistElN < Len; HistElN++) {
        if (Hist[HistElN] > 0.0) {
            // only nonzero histogram elements contribute to mass
            for (int CellN = 0; CellN < Len; CellN++) {
                Dens[CellN] += Hist[HistElN] * Precompute[abs(HistElN - CellN)];
            }
        }
    }
    TLinAlg::NormalizeL1(Dens);
}

double TKDEModel::RuleOfThumbHistBandwidth(const TFltV& Hist) {
    // also known as normal distribution approximation, Gaussian approximation, or Silverman's (1986) rule of thumb
    double X = 0.0;
    double X2 = 0.0;
    double Sum = 0.0;
    int Len = Hist.Len();

    for (int HistElN = 0; HistElN < Len; HistElN++) {
        if (Hist[HistElN] > 0.0) {
            Sum += Hist[HistElN];
            X += Hist[HistElN] * HistElN;
            X2 += Hist[HistElN] * HistElN * HistElN;
        }
    }
    // sqrt(E x^2 - (E x)^2)}
    double Stdev = TMath::Sqrt(X2 / Sum - X*X / (Sum*Sum));
    return 1.06 * Stdev * TMath::Power(Sum, -0.2);
}


double TKDEModel::ComputeDensity(const TFltV& Data, const double& EvalPoint, const double& Bandwidth, const THistogramPMFModelType& KType) {
    int DLen = Data.Len();
    double Dens = 0.0;
    for (int DatN = 0; DatN < DLen; DatN++) {
        Dens += EvalKernel(Data[DatN], EvalPoint, Bandwidth, KType);
    }
    Dens /= (double)DLen;
    return Dens;
}

void TKDEModel::ComputeDensity(const TFltV& Data, const TFltV& EvalPoints, TFltV& Dens, const double& Bandwidth, const THistogramPMFModelType& KType) {
    int ELen = EvalPoints.Len();
    Dens.Gen(ELen);
    for (int PointN = 0; PointN < ELen; PointN++) {
        Dens[PointN] = ComputeDensity(Data, EvalPoints[PointN], Bandwidth, KType);
    }
}

double TKDEModel::RuleOfThumbBandwidth(const TFltV& Data) {
    // also known as normal distribution approximation, Gaussian approximation, or Silverman's (1986) rule of thumb
    double Stdev = TLinAlgStat::Std(Data);
    return 1.06 * Stdev * TMath::Power((double)Data.Len(), -0.2);
}

double TKDEModel::LeaveOneOutRateBandwidth(const TFltV& Hist_, const TFltV& Bandwidths, const TFltV& Thresholds_, const THistogramPMFModelType& KType, const double& Tol) {
    TFltV Hist = Hist_;
    TFltV Thresholds = Thresholds_;
    if (Thresholds.Len() == 0) {
        // 0.1, 0.09, ..., 0.01
        int TLen = 10;
        Thresholds.Gen(TLen);
        for (int ElN = TLen; ElN >= 1; ElN--) {
            Thresholds[ElN] = (double)ElN / 100.0;
        }
    }
    int BandLen = Bandwidths.Len();
    int HistLen = Hist.Len();
    int ThreshLen = Thresholds.Len();
    TFltVV ScoreV(BandLen, ThreshLen); // number of thresholds (severities)

    // Complexity: ~ O(bandwidths * (nonzero buckets) * ((nonzero buckets) * buckets + Thresholds)
    //             ~ O(bandwidths * (nonzero buckets)^2 * buckets
    double MinScore = TFlt::Mx;
    int MinIdx = -1;
    double NumPoints = TLinAlg::NormL1(Hist);
    for (int BandN = 0; BandN < BandLen; BandN++) {
        for (int PointN = 0; PointN < HistLen; PointN++) {
            double Count = Hist[PointN];
            if (Count == 0.0) { continue; }
            // leave one point out
            Hist[PointN]--;
            // compute PMF
            TFltV PMF; ComputeHistDensity(Hist, PMF, Bandwidths[BandN], KType);
            // sort PMF, and see where
            TFltV SortedV; TIntV PermV;
            // Sort in ascending order: most severe anomalies come first
            PMF.SortGetPerm(PMF, SortedV, PermV, true); // 
            double CumSum = 0.0;
            bool FoundPoint = false;
            for (int ElN = 0; ElN < HistLen; ElN++) {
                CumSum += SortedV[ElN];
                // we found our point
                if (PermV[ElN] == PointN) {
                    FoundPoint = true;
                }
                // chech if there are others look too similar and aggregate them
                if (ElN < HistLen - 1 && (SortedV[ElN + 1] - SortedV[ElN] < Tol)) {
                    continue;
                }
                if (FoundPoint) {
                    // thresholds are sorted in descending order
                    // find which thresholds contain the current point
                    for (int ThreshN = 0; ThreshN < ThreshLen; ThreshN++) {
                        if (CumSum <= Thresholds[ThreshN]) {
                            ScoreV(BandN, ThreshN) += Count; // the same classification would happen for each of the Count points that fell in histogram bin PointN
                        } else { break; }
                    }
                    break;
                }
            }
            // restore the point (revert leave one out)
            Hist[PointN]++;
        }
        // count proportions 
        // If Thresholds[i] == 0.05, then ScoreV(j,i)/NumPoints should be close to 0.05
        double Score = 0;
        for (int ThreshN = 0; ThreshN < ThreshLen; ThreshN++) {
            Score += TMath::Abs(Thresholds[ThreshN] - ScoreV(BandN, ThreshN) / NumPoints);
        }
        if (Score < MinScore) {
            MinScore = Score;
            MinIdx = BandN;
        }
    }
    return Bandwidths[MinIdx];
}
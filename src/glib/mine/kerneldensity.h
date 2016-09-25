/**
 * Copyright (c) 2016, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

typedef enum {
    hpmfKDEGaussian, // normal
    hpmfKDETophat, // square
    hpmfKDEEpanechnikov, //quadratic
    hpmfL1, // square
} THistogramPMFModelType;

//////////////////////////////////////////////////////////////////////////////////
/// Histogram to PMF model
///   - Smooths a histogram
///   - Classifies bins with different severities of anomalies (0 = normal, 1 = low, 2 = higher,...)
///     given thresholds (input parameter).
///   - Parametrized by a vector of thresholds: t_0 > t_2, >... > t_n (from low to high severity, example: 0.05, 0.01, 0.001)
///     where t_-1 is defined as 1 and t_n+1 is defined as 0
///   - A bin is classifiedas an anomaly with severity i, if its cumulative mass (after being sorted in ascending order)
///     lies in the interval: (n * t(i), n * t(i-1)], where there are n points and r lies in [1,...,n]
///   - Example:
///      Thresholds [ 0.1, 0.05, 0.01 ]
///      Severity intervals for cumulative mass:
///         - [  0.1 ,    1 ] -> 0
///         - [ 0.05 ,  0.1 ] -> 1
///         - [ 0.01 ,  0.5 ] -> 2
///         - [    0 , 0.01 ] -> 0
///      PMF        = [ 0.025, 0.025, 0.5, 0.3, 0.1, 0.05 ]
///      Severities = [     2,     2,   0,   0,   0,    1 ]
///         - first two bins cover 0.05 mass (severity 2)
///         - first two + last bin cover 0.1 mass, (last bin gets serverity 1)
///         - no bin has mass less or equal 0.01 (no bin gets serverity 3)
///         - the rest of the bins normal (severity 0)
///    - Handles ties
///       Thresholds [ 0.5 ]
///       PMF        = [ 0.5, 0.5 ]
///       Gets mapped to [ 0, 0 ] (the cells are considered as a group and their total mass of 1 is above 0.5)
class THistogramToPMFModel {
public:
    /// Grouping tolerance for classifying anomalies to severities
    TFlt Tol;
    /// cumulative density function anomaly thresholds (sorted in descending order). 
    TFltV Thresholds;
    /// PMF model type
    THistogramPMFModelType Type;
    /// Gaussian KDE bandwidth - if 0 (default) it will be determined automatically
    TFlt Bandwidth;
    /// If true, the bandwidth will be tuned every time GetPMF is called
    bool AutoBandwidthP;
    /// Controls automatic bandwidth selection
    TFlt MinBandwidth;

    /// Leave one-out automatic KDE bandwidth candidates
    TFltV AutoBandwidthV;
    /// Leave one-out automatic KDE threshold targets
    TFltV AutoThresholdV;
    /// Leave one-out automatic KDE tolerance
    TFlt AutoBandwidthTol;

    /// Read parameters from JSON
    THistogramToPMFModel(const PJsonVal& ParamVal);

    //  Ties: The cells are added in groups of the same mass, each
    //  group is assigned the same severity. The grouping
    //  is based on a tolerance.
    /// Maps PMF cells to severity scores
    void ClassifyAnomalies(const TFltV& PMF, TFltV& Severities);

    /// Transforms a histogram to a probability mass function (PMF). The parameter ComputeBandwidthP decides if the (costly) 
    void GetPMF(const TFltV& Hist, TFltV& PMF, const bool& ComputeBandwidthP);
};

//////////////////////////////////////////////////////////////////////////////////
/// Kernel-Density-Estimation
class TKDEModel {
public:

    /// Evaluate the kernel density around a single data point
    static double EvalKernel(const double& Data, const double& Eval, const double& Bandwidth, const THistogramPMFModelType& KType = hpmfKDEGaussian);

    /// Smooths a histogram using kernel density estimation
    /// The histogram cells should be of equal size and the bandwidth parameter is expressed in units
    /// of histogram cell size. The histogram bins are centered at 0, 1, 2,...
    static void ComputeHistDensity(const TFltV& Hist, TFltV& Dens, const double& Bandwidth, const THistogramPMFModelType& KType = hpmfKDEGaussian);

    /// Optimal bandwidth for Gaussian kernel if the data is normally distributed and represented as a histogram
    /// The histogram bins are centered at 0, 1, 2,...
    static double RuleOfThumbHistBandwidth(const TFltV& Hist);

    /// Compute density given data, kernel type (gaussian default) and bandwidth on a set of evaluation points
    static double ComputeDensity(const TFltV& Data, const double& EvalPoint, const double& Bandwidth, const THistogramPMFModelType& KType = hpmfKDEGaussian);

    /// Compute density given data, kernel type (gaussian default) and bandwidth on a set of evaluation points
    static void ComputeDensity(const TFltV& Data, const TFltV& EvalPoints, TFltV& Dens, const double& Bandwidth, const THistogramPMFModelType& KType = hpmfKDEGaussian);

    /// Optimal bandwidth for Gaussian kernel if the data is normally distributed
    static double RuleOfThumbBandwidth(const TFltV& Data);

    /// Bandwidth that aims to match detector rates.
    ///    Given a bandwidth and a resulting pdf, each threshold t results in a set of anomaly intervals - sublevel sets of the pdf
    ///    that cover t mass. Ideally, given a new sample from the same distribution, we should classify fraction t of points as anomalous.
    ///    This should ideally hold for any t: for 0.05 we want 5% anomalies, for 0.1 we want 10% anomalies. Since we only
    ///    have one sample, we use leave one out cross-validation and a range of thresholds to estimate how good a given bandwidth
    ///    matches (L1 distance between its detection rates and ideal rates). Complexity ~ O(bandwidths * (nonzero buckets)^2 * buckets).
    static double LeaveOneOutRateBandwidth(const TFltV& Hist_, const TFltV& Bandwidths, const TFltV& Thresholds_ = TFltV(), const THistogramPMFModelType& KType = hpmfKDEGaussian, const double& Tol = 1e-8);
};

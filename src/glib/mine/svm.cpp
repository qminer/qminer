/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TSvm {

///////////////////////////////////////////////////////////////////////////////
// TLinParam

TLinParam::TLinParam(TSIn& SIn) { Load(SIn); }

void TLinParam::Load(TSIn& SIn){
    Cost.Load(SIn);
    Unbalance.Load(SIn);
    Eps.Load(SIn);
    SampleSize.Load(SIn);
    MxIter.Load(SIn);
    MxTime.Load(SIn);
    MnDiff.Load(SIn);
    Verbose.Load(SIn);
}

void TLinParam::Save(TSOut& SOut) const {
    TFlt(Cost).Save(SOut);
    TFlt(Unbalance).Save(SOut);
    TFlt(Eps).Save(SOut);
    TInt(SampleSize).Save(SOut);
    TInt(MxIter).Save(SOut);
    TInt(MxTime).Save(SOut);
    TFlt(MnDiff).Save(SOut);
    TBool(Verbose).Save(SOut);
}

///////////////////////////////////////////////////////////////////////////////
// TLinModel

TLinModel::TLinModel(TSIn& SIn) { Load(SIn); }

void TLinModel::Load(TSIn& SIn){
    WgtV.Load(SIn);
    Bias.Load(SIn);
    Param.Load(SIn);
}

void TLinModel::Save(TSOut& SOut) const {
    WgtV.Save(SOut);
    Bias.Save(SOut);
    Param.Save(SOut);
}

void TLinModel::UpdateParams(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("c")) { Param.Cost = ParamVal->GetObjNum("c"); }
    if (ParamVal->IsObjKey("j")) { Param.Unbalance = ParamVal->GetObjNum("j"); }
    if (ParamVal->IsObjKey("eps")) { Param.Eps = ParamVal->GetObjNum("eps"); }
    if (ParamVal->IsObjKey("batchSize")) { Param.SampleSize = ParamVal->GetObjInt("batchSize"); }
    if (ParamVal->IsObjKey("maxIterations")) { Param.MxIter = ParamVal->GetObjInt("maxIterations"); }
    if (ParamVal->IsObjKey("maxTime")) { Param.MxTime = TFlt::Round(1000.0 * ParamVal->GetObjNum("maxTime")); }
    if (ParamVal->IsObjKey("minDiff")) { Param.MnDiff = ParamVal->GetObjNum("minDiff"); }
    if (ParamVal->IsObjKey("verbose")) { Param.Verbose = ParamVal->GetObjBool("verbose"); }
}


PJsonVal TLinModel::GetParams() const {
    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("c", Param.Cost);
    ParamVal->AddToObj("j", Param.Unbalance);
    ParamVal->AddToObj("eps", Param.Eps);
    ParamVal->AddToObj("batchSize", Param.SampleSize);
    ParamVal->AddToObj("maxIterations", Param.MxIter);
    ParamVal->AddToObj("maxTime", Param.MxTime / 1000.0); // convert from miliseconds to seconds
    ParamVal->AddToObj("minDiff", Param.MnDiff);
    ParamVal->AddToObj("verbose", Param.Verbose);
    return ParamVal;
}

double TLinModel::Predict(const TFltV& Vec) const {
    return TLinAlg::DotProduct(WgtV, Vec) + Bias;
}

double TLinModel::Predict(const TIntFltKdV& SpVec) const {
    return TLinAlg::DotProduct(WgtV, SpVec) + Bias;
}

double TLinModel::Predict(const TFltVV& Mat, const int& ColN) const {
    return TLinAlg::DotProduct(Mat, ColN, WgtV) + Bias;
}

void TLinModel::FitClassification(const TFltVV& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    SolveClassification(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

void TLinModel::FitRegression(const TFltVV& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    SolveRegression(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

void TLinModel::FitClassification(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    SolveClassification(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

void TLinModel::FitRegression(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    SolveRegression(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

template <class TVecV>
void TLinModel::SolveClassification(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const PNotify& _LogNotify, const PNotify& ErrorNotify) {

    // asserts for input parameters
    EAssertR(Dims > 0, "Dimensionality must be positive!");
    EAssertR(Vecs > 0, "Number of vectors must be positive!");
        EAssertR(Vecs == TargetV.Len(), "Number of vectors must be equal to the number of targets!");
    EAssertR(Param.Cost > 0.0, "Cost parameter must be positive!");
    EAssertR(Param.SampleSize > 0, "Sampling size must be positive!");
    EAssertR(Param.MxIter > 1, "Number of iterations to small!");

    // hide output if not verbose
    PNotify LogNotify = Param.Verbose ? _LogNotify : TNotify::NullNotify;

    LogNotify->OnStatusFmt("SVM parameters: c=%.2f, j=%.2f", Param.Cost, Param.Unbalance);

    // initialization
    TRnd Rnd(1);
    const double Lambda = 1.0 / (double(Vecs) * Param.Cost);
    // we start with random normal vector
    WgtV = TFltV(Dims); TLinAlgTransform::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
    // make it of appropriate length
    TLinAlg::MultiplyScalar(1.0 / (2.0 * TMath::Sqrt(Lambda)), WgtV, WgtV);
    // allocate space for updates
    TFltV NewWgtV(Dims);

    // split vectors into positive and negative
    TIntV PosVecIdV, NegVecIdV;
    for (int VecN = 0; VecN < Vecs; VecN++) {
        if (TargetV[VecN] > 0.0) {
            PosVecIdV.Add(VecN);
        } else {
            NegVecIdV.Add(VecN);
        }
    }
    const int PosVecs = PosVecIdV.Len(), NegVecs = NegVecIdV.Len();
    // prepare sampling ratio between positive and negative
    //  - the ration is uniform over the records when Unbalance == 1.0
    //  - if smaller then 1.0, then there is bias towards negative
    //  - if larger then 1.0, then there is bias towards positives
    double SamplingRatio = (double(PosVecs) * Param.Unbalance) /
        (double(PosVecs) * Param.Unbalance + double(NegVecs));
    LogNotify->OnStatusFmt("Sampling ration 1 positive vs %.2f negative [%.2f]",
        (1.0 / SamplingRatio - 1.0), SamplingRatio);

    TTmTimer Timer(Param.MxTime); int Iters = 0; double Diff = 1.0;
    LogNotify->OnStatusFmt("Limits: %d iterations, %.3f seconds, %.8f weight difference",
        Param.MxIter, (double)Param.MxTime /1000.0, Param.MnDiff);
    // initialize profiler
    TTmProfiler Profiler;
    const int ProfilerPre = Profiler.AddTimer("Pre");
    const int ProfilerBatch = Profiler.AddTimer("Batch");
    const int ProfilerPost = Profiler.AddTimer("Post");

    // function for writing progress reports
    int PosCount = 0, NegCount = 0;
    auto ProgressNotify = [&]() {
        LogNotify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g, ratio %.2f",
            Iters, Timer.GetStopWatch().GetMSec() / 1000.0, Diff,
            (double)PosCount / (double)(PosCount + NegCount));
        PosCount = 0; NegCount = 0;
    };

    for (int IterN = 0; IterN < Param.MxIter; IterN++) {
        if (IterN % 100 == 0) { ProgressNotify(); }

        Profiler.StartTimer(ProfilerPre);
        // tells how much we can move
        const double Nu = 1.0 / (Lambda * double(IterN + 2)); // ??
        const double VecUpdate = Nu / double(Param.SampleSize);
        // initialize updated normal vector
        TLinAlg::MultiplyScalar((1.0 - Nu * Lambda), WgtV, NewWgtV);
        Profiler.StopTimer(ProfilerPre);

        // classify examples from the sample
        Profiler.StartTimer(ProfilerBatch);
        int DiffCount = 0;
        for (int SampleN = 0; SampleN < Param.SampleSize; SampleN++) {
            int VecN = 0;
            if (Rnd.GetUniDev() > SamplingRatio) {
                // we select negative vector
                VecN = NegVecIdV[Rnd.GetUniDevInt(NegVecs)];
                NegCount++;
            } else {
                // we select positive vector
                VecN = PosVecIdV[Rnd.GetUniDevInt(PosVecs)];
                PosCount++;
            }
            const double VecCfyVal = TargetV[VecN];
            const double CfyVal = VecCfyVal * TLinAlg::DotProduct(VecV, VecN, WgtV);
            if (CfyVal < 1.0) {
                // with update from the stochastic sub-gradient
                TLinAlg::AddVec(VecUpdate * VecCfyVal, VecV, VecN, NewWgtV, NewWgtV);
                DiffCount++;
            }
        }
        Profiler.StopTimer(ProfilerBatch);

        Profiler.StartTimer(ProfilerPost);
        // project the current solution on to a ball
        const double WgtNorm = 1.0 / (TLinAlg::Norm(NewWgtV) * TMath::Sqrt(Lambda));
        if (WgtNorm < 1.0) { TLinAlg::MultiplyScalar(WgtNorm, NewWgtV, NewWgtV); }
        // compute the difference with respect to the previous iteration
        Diff = 2.0 * TLinAlg::EuclDist(WgtV, NewWgtV) / (TLinAlg::Norm(WgtV) + TLinAlg::Norm(NewWgtV));
        // remember new solution, but only when we actually did some changes
        WgtV = NewWgtV;
        Profiler.StopTimer(ProfilerPost);

        // count
        Iters++;
        // check stopping criteria with respect to time
        if (Timer.IsTimeUp()) {
            LogNotify->OnStatusFmt("Finishing due to reached time limit of %.3f seconds", (double)Param.MxTime / 1000.0);
            break;
        }
        // check stopping criteria with respect to result difference
        //if (DiffCount > 0 && (1.0 - DiffCos) < MnDiff) {
        if (DiffCount > 0 && Diff < Param.MnDiff) {
            LogNotify->OnStatusFmt("Finishing due to reached difference limit of %g", Param.MnDiff);
            break;
        }
    }
    if (Iters == Param.MxIter) {
        LogNotify->OnStatusFmt("Finished due to iteration limit of %d", Iters);
    }

    ProgressNotify();
    Profiler.PrintReport(LogNotify);
}

template <class TVecV>
void TLinModel::SolveRegression(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const PNotify& _LogNotify, const PNotify& ErrorNotify) {

    // asserts for input parameters
    EAssertR(Dims > 0, "Dimensionality must be positive!");
    EAssertR(Vecs > 0, "Number of vectors must be positive!");
    EAssertR(Vecs == TargetV.Len(), "Number of vectors must be equal to the number of targets!");
    EAssertR(Param.Cost > 0.0, "Cost parameter must be positive!");
    EAssertR(Param.SampleSize > 0, "Sampling size must be positive!");
    EAssertR(Param.MxIter > 1, "Number of iterations to small!");
    EAssertR(Param.MnDiff >= 0, "Min difference must be nonnegative!");

    // hide output if not verbose
    PNotify LogNotify = Param.Verbose ? _LogNotify : TNotify::NullNotify;

    // initialization
    TRnd Rnd(1);
    const double Lambda = 1.0 / (double(Vecs) * Param.Cost);
    // we start with random normal vector
    WgtV = TFltV(Dims); TLinAlgTransform::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
    // Scale it to appropriate norm
    TLinAlg::MultiplyScalar(1.0 / (2.0 * TMath::Sqrt(Lambda)), WgtV, WgtV);

    // True norm is a product of Norm and TLinAlg::Norm(WgtV)
    // The reason for this is that we can have very cheap updates for sparse
    // vectors - we do not need to touch all elements of WgtV in each subgradient
    // update.
    double Norm = 1.0;
    double Normw = 1.0 / (2.0 * TMath::Sqrt(Lambda));

    TTmTimer Timer(Param.MxTime); int Iters = 0; double Diff = 1.0;
    LogNotify->OnStatusFmt("Limits: %d iterations, %.3f seconds, %.8f weight difference",
        Param.MxIter, (double)Param.MxTime / 1000.0, Param.MnDiff);
    // initialize profiler
    TTmProfiler Profiler;
    const int ProfilerPre = Profiler.AddTimer("Pre");
    const int ProfilerBatch = Profiler.AddTimer("Batch");

    // function for writing progress reports
    auto ProgressNotify = [&]() {
        LogNotify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g",
                Iters, Timer.GetStopWatch().GetMSec() / 1000.0, Diff);
    };

    // Since we are using weight vector overrepresentation using Norm, we need to
    // compensate the scaling when adding examples using Coef
    double Coef = 1.0;
    for (int IterN = 0; IterN < Param.MxIter; IterN++) {
        if (IterN % 100 == 0) { ProgressNotify(); }

        Profiler.StartTimer(ProfilerPre);
        // tells how much we can move
        const double Nu = 1.0 / (Lambda * double(IterN + 2));
        // update Coef which counters Norm
        Coef /= (1 - Nu * Lambda);
        const double VecUpdate = Nu / (double(Param.SampleSize)) * Coef;

        Profiler.StopTimer(ProfilerPre);
        // Track the upper bound on the change of norm of WgtV
        Diff = 0.0;
        // process examples from the sample
        Profiler.StartTimer(ProfilerBatch);
        // store which examples will lead to gradient updates (and their factors)
        TVec<TPair<TFlt, TInt> > Updates(Param.SampleSize, 0);

        // in the first pass we find which samples will lead to updates
        for (int SampleN = 0; SampleN < Param.SampleSize; SampleN++) {
            const int VecN = Rnd.GetUniDevInt(Vecs);
            // target
            const double Target = TargetV[VecN];
            // prediction
            double Dot = TLinAlg::DotProduct(VecV, VecN, WgtV);
            // Used in bound computation
            double NorX = TLinAlg::Norm(VecV, VecN);
            // For predictions we need to use the Norm to scale correctly
            const double Pred = Norm * Dot;

            // difference
            const double Loss = Target - Pred;
            // do the update based on the difference
            if (Loss < -Param.Eps) { // y_i - z < -eps
                // update from the negative stochastic sub-gradient: -x
                Updates.Add(TPair<TFlt, TInt>(-VecUpdate, VecN));
                // update the norm of WgtV
                Normw = sqrt(Normw*Normw - 2 * VecUpdate * Dot + VecUpdate * VecUpdate * NorX * NorX);
                // update the bound on the change of norm of WgtV
                Diff += VecUpdate * NorX;
            } else if (Loss > Param.Eps) { // y_i - z > eps
                // update from the negative stochastic sub-gradient: x
                Updates.Add(TPair<TFlt, TInt>(VecUpdate, VecN));
                // update the norm of WgtV
                Normw = sqrt(Normw*Normw + 2 * VecUpdate * Dot + VecUpdate * VecUpdate * NorX * NorX);
                // update the bound on the change of norm of WgtV
                Diff += VecUpdate * NorX;
            } // else nothing to do, we are within the epsilon tube
        }
        // Diff now estimates the upper bound on |w - w_old|/|w|
        Diff /= Normw;

        // in the second pass we update
        for (int UpdateN = 0; UpdateN < Updates.Len(); UpdateN++) {
            TLinAlg::AddVec(Updates[UpdateN].Val1, VecV, Updates[UpdateN].Val2, WgtV, WgtV);
        }
        Norm *= (1 - Nu * Lambda);

        Profiler.StopTimer(ProfilerBatch);

        // renormalizing is not needed according to new results:
        // "Pegasos: Primal Estimated sub-GrAdient SOlver for SVM"
        // Shai Shalev-Shwartz, Yoram Singer, Nathan Srebro, Andrew Cotter.
        // Mathematical Programming, Series B, 127(1):3-30, 2011.

        // count
        Iters++;
        // check stopping criteria with respect to time
        if (Timer.IsTimeUp()) {
            LogNotify->OnStatusFmt("Finishing due to reached time limit of %.3f seconds", (double)Param.MxTime / 1000.0);
            break;
        }
        // check stopping criteria with respect to result difference
        if (Diff < Param.MnDiff) {
            LogNotify->OnStatusFmt("Finishing due to reached difference limit of %g", Param.MnDiff);
            break;
        }
    }
    if (Iters == Param.MxIter) {
        LogNotify->OnStatusFmt("Finished due to iteration limit of %d", Iters);
    }
    // Finally we use the Norm factor to rescale the weight vector
    TLinAlg::MultiplyScalar(Norm, WgtV, WgtV);

    ProgressNotify();

    Profiler.PrintReport(LogNotify);
}

///////////////////////////////////////////////////////////////////////////////
// TSvmLibParam

TLibSvmParam::TLibSvmParam(TSIn& SIn) { Load(SIn); }

void TLibSvmParam::Load(TSIn& SIn){
    Type.Load(SIn);
    Kernel.Load(SIn);
    Cost.Load(SIn);
    Unbalance.Load(SIn);
    Eps.Load(SIn);
    Gamma.Load(SIn);
    P.Load(SIn);
    Degree.Load(SIn);
    Nu.Load(SIn);
    Coef0.Load(SIn);
    CacheSize.Load(SIn);
    Verbose.Load(SIn);
}

void TLibSvmParam::Save(TSOut& SOut) const {
    TInt(Type).Save(SOut);
    TInt(Kernel).Save(SOut);
    TFlt(Cost).Save(SOut);
    TFlt(Unbalance).Save(SOut);
    TFlt(Eps).Save(SOut);
    TFlt(Gamma).Save(SOut);
    TFlt(P).Save(SOut);
    TInt(Degree).Save(SOut);
    TFlt(Nu).Save(SOut);
    TFlt(Coef0).Save(SOut);
    TFlt(CacheSize).Save(SOut);
    TBool(Verbose).Save(SOut);
}

svm_parameter_t TLibSvmParam::GetParamStruct() const {//returns svm_parameter_t for LIBSVM train
    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = Type;//default
    svm_parameter.kernel_type = Kernel;//default
    svm_parameter.degree = Degree;
    svm_parameter.gamma = Gamma;
    svm_parameter.coef0 = Coef0;

    // training only
    svm_parameter.C = Cost;
    svm_parameter.nu = Nu;
    svm_parameter.nr_weight = 2;
    svm_parameter.weight_label = (int *)malloc(2 * sizeof(int)); // deleted in svm_destroy_param
    svm_parameter.weight_label[0] = -1;
    svm_parameter.weight_label[1] = 1;
    svm_parameter.weight = (double *)malloc(2 * sizeof(double));  // deleted in svm_destroy_param
    svm_parameter.weight[0] = 1;
    svm_parameter.weight[1] = Unbalance;
    svm_parameter.cache_size = CacheSize;
    svm_parameter.eps = Eps;
    svm_parameter.p = P; // not needed but it has to be positive as it is checked
    svm_parameter.shrinking = 0;
    svm_parameter.probability = 0;
    return svm_parameter;
}

///////////////////////////////////////////////////////////////////////////////
// TSvmLibModelPredictParam

TLibSvmPredictParam::TLibSvmPredictParam(TSIn& SIn) { Load(SIn); }

void TLibSvmPredictParam::Load(TSIn& SIn){
    Type.Load(SIn);
    Kernel.Load(SIn);
    Gamma.Load(SIn);
    Degree.Load(SIn);
    Coef0.Load(SIn);
}

void TLibSvmPredictParam::Save(TSOut& SOut) const {
    TInt(Type).Save(SOut);
    TInt(Kernel).Save(SOut);
    TFlt(Gamma).Save(SOut);
    TInt(Degree).Save(SOut);
    TFlt(Coef0).Save(SOut);
}

svm_parameter_t TLibSvmPredictParam::GetPredictParamStruct() const {//returns svm_parameter_t for LIBSVM predict
    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = Type;//default
    svm_parameter.kernel_type = Kernel;//default
    svm_parameter.degree = Degree;
    svm_parameter.gamma = Gamma;
    svm_parameter.coef0 = Coef0;
    return svm_parameter;
}

///////////////////////////////////////////////////////////////////////////////
// TLibSvmModel

TLibSvmModel::TLibSvmModel(TSIn& SIn) { Load(SIn); }

void TLibSvmModel::Load(TSIn& SIn){
    WgtV.Load(SIn);
    Bias.Load(SIn);
    Param.Load(SIn);
    PredictParam.Load(SIn);
    SupportVectors.Load(SIn);
    Coef.Load(SIn);
    Rho.Load(SIn);
    NSupportVectors.Load(SIn);
}

void TLibSvmModel::Save(TSOut& SOut) const {
    WgtV.Save(SOut);
    Bias.Save(SOut);
    Param.Save(SOut);
    PredictParam.Save(SOut);
    SupportVectors.Save(SOut);
    Coef.Save(SOut);
    Rho.Save(SOut);
    NSupportVectors.Save(SOut);
}

void TLibSvmModel::UpdateParams(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("kernel")) {
        TStr KernelStr = ParamVal->GetObjStr("kernel");
        Param.Kernel = LIBSVM_LINEAR;
        if (KernelStr == "LINEAR") { Param.Kernel = LIBSVM_LINEAR; }
        else if (KernelStr == "POLY") { Param.Kernel = LIBSVM_POLY; }
        else if (KernelStr == "RBF") { Param.Kernel = LIBSVM_RBF; }
        else if (KernelStr == "SIGMOID") { Param.Kernel = LIBSVM_SIGMOID; }
        else if (KernelStr == "PRECOMPUTED") { Param.Kernel = LIBSVM_PRECOMPUTED; }
    }
    if (ParamVal->IsObjKey("svmType")) {
        TStr TypeStr = ParamVal->GetObjStr("svmType");
        Param.Type = DEFAULT;
        if (TypeStr == "C_SVC") { Param.Type = LIBSVM_CSVC; }
        else if (TypeStr == "NU_SVC") { Param.Type = LIBSVM_NUSVC; }
        else if (TypeStr == "ONE_CLASS") { Param.Type = LIBSVM_ONECLASS; }
        else if (TypeStr == "EPSILON_SVR") { Param.Type = LIBSVM_EPSILONSVR; }
        else if (TypeStr == "NU_SVR") { Param.Type = LIBSVM_NUSVC; }
    }
    if (ParamVal->IsObjKey("c")) { Param.Cost = ParamVal->GetObjNum("c"); }
    if (ParamVal->IsObjKey("j")) { Param.Unbalance = ParamVal->GetObjNum("j"); }
    if (ParamVal->IsObjKey("eps")) { Param.Eps = ParamVal->GetObjNum("eps"); }
    if (ParamVal->IsObjKey("gamma")) { Param.Gamma = ParamVal->GetObjNum("gamma"); }
    if (ParamVal->IsObjKey("p")) { Param.P = ParamVal->GetObjNum("p"); }
    if (ParamVal->IsObjKey("degree")) { Param.Degree = ParamVal->GetObjInt("degree"); }
    if (ParamVal->IsObjKey("nu")) { Param.Nu = ParamVal->GetObjNum("nu"); }
    if (ParamVal->IsObjKey("coef0")) { Param.Coef0 = ParamVal->GetObjNum("coef0"); }
    if (ParamVal->IsObjKey("cacheSize")) { Param.CacheSize = ParamVal->GetObjNum("cacheSize"); }
    if (ParamVal->IsObjKey("verbose")) { Param.Verbose = ParamVal->GetObjBool("verbose"); }
}

PJsonVal TLibSvmModel::GetParams() const {
    PJsonVal ParamVal = TJsonVal::NewObj();
    TStr KernelStr = "LINEAR";
    if (Param.Kernel == LIBSVM_LINEAR) { KernelStr = "LINEAR"; }
    else if (Param.Kernel == LIBSVM_POLY) { KernelStr = "POLY"; }
    else if (Param.Kernel == LIBSVM_RBF) { KernelStr = "RBF"; }
    else if (Param.Kernel == LIBSVM_SIGMOID) { KernelStr = "SIGMOID"; }
    else if (Param.Kernel == LIBSVM_PRECOMPUTED) { KernelStr = "PRECOMPUTED"; }
    ParamVal->AddToObj("kernel", KernelStr);
    TStr TypeStr = "default";
    if (Param.Type == LIBSVM_CSVC) { TypeStr =  "C_SVC"; }
    else if (Param.Type == LIBSVM_NUSVC) { TypeStr = "NU_SVC"; }
    else if (Param.Type == LIBSVM_ONECLASS) { TypeStr = "ONE_CLASS"; }
    else if (Param.Type == LIBSVM_EPSILONSVR) { TypeStr = "EPSILON_SVR"; }
    else if (Param.Type == LIBSVM_NUSVR) { TypeStr = "NU_SVR"; }
    else if (Param.Type == DEFAULT) { TypeStr = "default"; }
    ParamVal->AddToObj("svmType", TypeStr);
    ParamVal->AddToObj("c", Param.Cost);
    ParamVal->AddToObj("j", Param.Unbalance);
    ParamVal->AddToObj("eps", Param.Eps);
    ParamVal->AddToObj("gamma", Param.Gamma);
    ParamVal->AddToObj("p", Param.P);
    ParamVal->AddToObj("degree", Param.Degree);
    ParamVal->AddToObj("nu", Param.Nu);
    ParamVal->AddToObj("coef0", Param.Coef0);
    ParamVal->AddToObj("cacheSize", Param.CacheSize);
    ParamVal->AddToObj("verbose", Param.Verbose);
    return ParamVal;
}

svm_model_t* TLibSvmModel::GetModelStruct() const {
    svm_model_t* svm_model = new svm_model_t;
    svm_model->param = PredictParam.GetPredictParamStruct();
    int DimX = SupportVectors.GetXDim();
    int DimY = SupportVectors.GetYDim();
    svm_model->l = DimX;
    svm_model->SV = (svm_node_t **)malloc(DimX * sizeof(svm_node_t *));
    for (int Idx = 0; Idx < DimX; Idx++){
      svm_model->SV[Idx] = (svm_node_t *)malloc((DimY+ 1) * sizeof(svm_node_t));
      for (int cIdx = 0; cIdx < DimY; cIdx ++){
        svm_model->SV[Idx][cIdx].index = cIdx;
        svm_model->SV[Idx][cIdx].value = SupportVectors.GetXY(Idx, cIdx);
      }
      svm_model->SV[Idx][DimY].index = -1;
    }
    DimX = Coef.GetXDim();
    DimY = Coef.GetYDim();
    svm_model->nr_class = DimX + 1;
    svm_model->sv_coef = (double **)malloc(DimX * sizeof(double *));
    for (int Idx = 0; Idx < DimX; Idx++){
      svm_model->sv_coef[Idx] = (double *)malloc(DimY * sizeof(double));
      for (int cIdx = 0; cIdx < DimY; cIdx ++){
        svm_model->sv_coef[Idx][cIdx] = Coef.GetXY(Idx, cIdx);
      }
    }
    DimX = Rho.Len();
    svm_model->rho = (double *)malloc(DimX * sizeof(double));
    for (int Idx = 0; Idx < DimX; Idx++){
      svm_model->rho[Idx] = Rho[Idx];
    }
    // not needed (and therefore not saved)
    svm_model->free_sv = 0;
    svm_model->probA = NULL;
    svm_model->probB = NULL;
    svm_model->sv_indices = NULL;
    // classification specific
    svm_model->nSV = NULL;
    svm_model->label = NULL;
    if (Param.Type == C_SVC || Param.Type == NU_SVC){
      DimX = NSupportVectors.Len();
      svm_model->nSV = (int *)malloc(DimX * sizeof(int));
      for (int Idx = 0; Idx < DimX; Idx++){
        svm_model->nSV[Idx] = NSupportVectors[Idx];
      }
      DimX = svm_model->nr_class;
      svm_model->label = (int *)malloc(DimX * sizeof(int));
      for (int Idx = 0; Idx < DimX; Idx++){
        svm_model->label[Idx] = Idx;
      }
    }
    return svm_model;
}

void TLibSvmModel::DeleteModelStruct(svm_model_t* svm_model) const {
    // free svm_model->SV
    int DimX = SupportVectors.GetXDim();
    for (int Idx = 0; Idx < DimX; Idx++){
      free(svm_model->SV[Idx]);
      svm_model->SV[Idx] = NULL;
    }
    free(svm_model->SV);
    svm_model->SV = NULL;
    // free svm_model->sv_coef
    DimX = Coef.GetXDim();
    for (int Idx = 0; Idx < DimX; Idx++){
      free(svm_model->sv_coef[Idx]);
      svm_model->sv_coef[Idx] = NULL;
    }
    free(svm_model->sv_coef);
    svm_model->sv_coef = NULL;
    // free svm_model->rho
    free(svm_model->rho);
    svm_model->rho = NULL;
    // free svm_model->nSV and svm_model->label if allocated
    if (Param.Type == C_SVC || Param.Type == NU_SVC){
      free(svm_model->nSV);
      svm_model->nSV = NULL;
      free(svm_model->label);
      svm_model->label = NULL;
    }

    delete svm_model;
    svm_model = NULL;
}

void TLibSvmModel::ConvertResults(svm_model_t* svm_model, int Dim){
    PredictParam = TLibSvmPredictParam(Param.Type, Param.Kernel, Param.Gamma, Param.Degree, Param.Coef0);
    WgtV = TFltV(Dim);
    Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
    SupportVectors = TFltVV(svm_model->l, Dim);
    Coef = TFltVV(svm_model->nr_class - 1, svm_model->l);
    Rho = TFltV(svm_model->nr_class * (svm_model->nr_class - 1)/2);

    // compute normal vector from support vectors
    EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
    for (int Idx = 0; Idx < svm_model->l; ++Idx) {
        svm_node_t* SVs = svm_model->SV[Idx];
        while (SVs->index != -1) {
            SupportVectors.PutXY(Idx, SVs->index - 1, SVs->value);
            WgtV[SVs->index - 1] += svm_model->sv_coef[0][Idx] * SVs->value;
            ++SVs;
        }
        for (int cIdx = 0; cIdx < svm_model->nr_class - 1; cIdx++){
          Coef.PutXY(cIdx, Idx, svm_model->sv_coef[cIdx][Idx]);
        }
    }
    for (int Idx = 0; Idx < svm_model->nr_class * (svm_model->nr_class - 1)/2; Idx++){
        Rho.SetVal(Idx, svm_model->rho[Idx]);
    }
    if (Param.Type == C_SVC || Param.Type == NU_SVC){
        NSupportVectors = TIntV(svm_model->nr_class);
        for (int Idx = 0; Idx < svm_model->nr_class; Idx++){
            NSupportVectors.SetVal(Idx, svm_model->nSV[Idx]);
        }
    }
    // clean up
    svm_free_and_destroy_model(&svm_model);
    free(svm_model);
}

double TLibSvmModel::Predict(const TFltV& Vec) const {
    if (Param.Kernel == LINEAR){
        return TLinAlg::DotProduct(WgtV, Vec) + Bias;
    }
    svm_model_t* model = GetModelStruct();
    svm_node_t *x = (svm_node_t *)malloc((Vec.Len() + 1) * sizeof(svm_node_t));
    double* dec_val = (double *)malloc(model->nr_class*(model->nr_class-1)/2 * sizeof(double));
    for (int Idx = 0; Idx < Vec.Len(); Idx++){
        x[Idx].index = Idx;
        x[Idx].value = Vec[Idx];
    }
    x[Vec.Len()].index = -1;
    svm_predict_values(model, x, dec_val);
    double result = dec_val[0];

    free(x);
    free(dec_val);
    DeleteModelStruct(model);
    return result;
}

double TLibSvmModel::Predict(const TIntFltKdV& SpVec) const {
    if (Param.Kernel == LINEAR){
        return TLinAlg::DotProduct(WgtV, SpVec) + Bias;
    }
    int FullDim = WgtV.Len();
    TFltV Vec = TFltV(FullDim);
    for (int Idx = 0; Idx < FullDim; Idx++){
        Vec.SetVal(Idx, 0);
    }
    int Dim = SpVec.Len();
    for (int Idx = 0; Idx < Dim; Idx++){
        EAssertR(SpVec.GetVal(Idx).Key < FullDim, "Dimension mismatch.");
        Vec.SetVal(SpVec.GetVal(Idx).Key, SpVec.GetVal(Idx).Dat);
    }
    return Predict(Vec);
}

double TLibSvmModel::Predict(const TFltVV& Mat, const int& ColN) const {
    if (Param.Kernel == LINEAR){
        return TLinAlg::DotProduct(Mat, ColN, WgtV) + Bias;
    }
    int DimX = Mat.GetXDim();
    TFltV Col(DimX);
    Mat.GetCol(ColN, Col);
    return Predict(Col);
}

void TLibSvmModel::FitClassification(const TVec<TIntFltKdV>& VecV, const int& DimsA, const int& VecsA,
    const TFltV& TargetV, const PNotify& _LogNotify, const PNotify& ErrorNotify) {

    printf("inside FitClassification\n");

    if (Param.Type == DEFAULT) { Param.Type = LIBSVM_CSVC; }

    // load training parameters
    svm_parameter_t svm_parameter = Param.GetParamStruct();

    // Asserts for input arguments
    EAssertR(Param.Cost > 0.0, "Cost parameter has to be positive.");

    // load train data
    svm_problem_t svm_problem;
    svm_problem.l = VecV.Len();
    // reserve space for target variable
    svm_problem.y = (double *)malloc(VecV.Len() * sizeof(double));
    // reserve space for training vectors
    svm_problem.x = (svm_node_t **)malloc(VecV.Len() * sizeof(svm_node_t *));
    // compute number of nonzero elements and get dimensionalit
    int NonZero = 0, Dim = 0;
    for (int VecN = 0; VecN < VecV.Len(); ++VecN) {
        NonZero += (VecV[VecN].Len() + 1);
        if (!VecV[VecN].Empty()) {
            Dim = TInt::GetMx(Dim, VecV[VecN].Last().Key + 1);
        }
    }
    svm_node_t* x_space = (svm_node_t *)malloc(NonZero * sizeof(svm_node_t));
    // load training data and vectors
    int N = 0, prevN = 0;
    for (int VecN = 0; VecN < VecV.Len(); ++VecN) {
        prevN = N;
        svm_problem.y[VecN] = TargetV[VecN];
        for (int EltN = 0; EltN < VecV[VecN].Len(); ++EltN) {
            x_space[N].index = VecV[VecN][EltN].Key+1;
            x_space[N++].value = VecV[VecN][EltN].Dat;
        }
        x_space[N++].index = -1;
        svm_problem.x[VecN] = &x_space[prevN];
    }

    const char* error_msg = svm_check_parameter(&svm_problem, &svm_parameter);
    EAssertR(error_msg == NULL, error_msg);

    // hide output if not verbose
    PNotify LogNotify = Param.Verbose ? _LogNotify : TNotify::NullNotify;
    // train the model
    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, LogNotify(), ErrorNotify());

    // save model and clean up
    ConvertResults(svm_model, Dim);

    // clean up
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);
}

void TLibSvmModel::FitClassification(const TFltVV& VecV, const int& DimsA, const int& VecsA,
    const TFltV& TargetV, const PNotify& _LogNotify, const PNotify& ErrorNotify) {

    if (Param.Type == DEFAULT) { Param.Type = LIBSVM_CSVC; }

    // load training parameters
    svm_parameter_t svm_parameter = Param.GetParamStruct();

    // Asserts for input arguments
    EAssertR(Param.Cost > 0.0, "Cost parameter has to be positive.");

    const int DimN = VecV.GetXDim(); // Number of features
    const int AllN = VecV.GetYDim(); // Number of examples

    EAssertR(TargetV.Len() == AllN, "Dimension mismatch.");

    svm_problem_t svm_problem;
    svm_problem.l = AllN;
    svm_problem.y = (double *)malloc(AllN*sizeof(double));

    svm_problem.x = (svm_node_t **)malloc(AllN*sizeof(svm_node_t *));
    svm_node_t* x_space = (svm_node_t *)malloc((AllN*(DimN+1))*sizeof(svm_node_t));
    int N = 0, prevN = 0;
    for (int Idx = 0; Idx < AllN; ++Idx) { // # of examples
        prevN = N;
        svm_problem.y[Idx] = TargetV[Idx];
        for (int Jdx = 0; Jdx < DimN; ++Jdx) { // # of features
            if (VecV.At(Jdx, Idx) != 0.0) { // Store non-zero entries only
                x_space[N].index = Jdx+1;
                x_space[N].value = VecV.At(Jdx, Idx);
                ++N;
            }
        }
        x_space[N].index = -1;
        ++N;
        svm_problem.x[Idx] = &x_space[prevN];
    }

    const char* error_msg = svm_check_parameter(&svm_problem, &svm_parameter);
    EAssertR(error_msg == NULL, error_msg);

    // hide output if not verbose
    PNotify LogNotify = Param.Verbose ? _LogNotify : TNotify::NullNotify;
    // train model
    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, LogNotify(), ErrorNotify());

    // save model and clean up
    ConvertResults(svm_model, DimN);

    // clean up
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);
}

void TLibSvmModel::FitRegression(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    if (Param.Type == DEFAULT) { Param.Type = LIBSVM_EPSILONSVR; }
    FitClassification(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

void TLibSvmModel::FitRegression(const TFltVV& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV, const PNotify& LogNotify, const PNotify& ErrorNotify) {

    if (Param.Type == DEFAULT) { Param.Type = LIBSVM_EPSILONSVR; }
    FitClassification(VecV, Dims, Vecs, TargetV, LogNotify, ErrorNotify);
}

}// end namespace

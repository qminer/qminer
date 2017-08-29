/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef SVM_H
#define	SVM_H

#include "../base/base.h"
#include "libsvm.h"

namespace TSvm {

/// Linear model
class TLinModel {
private:
    TFltV WgtV;
    TFlt Bias;
public:
    TLinModel(): Bias(0.0) {  }
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) {  }
    TLinModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) {  }

    TLinModel(TSIn& SIn): WgtV(SIn), Bias(SIn) { }
    void Save(TSOut& SOut) const { WgtV.Save(SOut); Bias.Save(SOut); }
    
    /// Get weight vector
    const TFltV& GetWgtV() const { return WgtV; }
    
    /// Get bias
    double GetBias() const { return Bias; }

    /// Classify full vector
    double Predict(const TFltV& Vec) const { 
        return TLinAlg::DotProduct(WgtV, Vec) + Bias; 
    }
    
    /// Classify sparse vector
    double Predict(const TIntFltKdV& SpVec) const { 
        return TLinAlg::DotProduct(WgtV, SpVec) + Bias;
    }

    /// Classify matrix column vector
    double Predict(const TFltVV& Mat, const int& ColN) const {
        return TLinAlg::DotProduct(Mat, ColN, WgtV) + Bias;
    }
};

// LIBSVM for Eps-Support Vector Regression for sparse input
inline TLinModel LibSvmSolveRegression(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV,
        const double& Eps, const double& Cost, PNotify DebugNotify, PNotify ErrorNotify) {
     // Asserts for input arguments
    EAssertR(Cost > 0.0, "Cost parameter has to be positive.");

    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = EPSILON_SVR;
    svm_parameter.kernel_type = LINEAR;
    // If degree<0 svm_check_params reports an error, even though degree is
    // ignored by the learning when kernel_type!=polynomial
    svm_parameter.degree = 0;
    svm_parameter.C = Cost;
    svm_parameter.eps = Eps;
    // cache_size is only needed for kernel functions
    svm_parameter.cache_size = 100;
    svm_parameter.eps = 1e-3;
	svm_parameter.p = 0.1; // not needed but it has to be positive as it is checked
    svm_parameter.shrinking = 0;
    svm_parameter.probability = 0;
	//  not needed for linear SVM, but it has to be positive as it is checked
	svm_parameter.gamma = 1.0;

    svm_problem_t svm_problem;
    svm_problem.l = VecV.Len();
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

    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);

    TFltV WgtV(Dim);
    TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
    EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
    for (int Idx = 0; Idx < svm_model->l; ++Idx) {
        svm_node_t* SV = svm_model->SV[Idx];
        while (SV->index != -1) {
            WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
            ++SV;
        }
    }

    svm_free_and_destroy_model(&svm_model);
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);
    free(svm_model);

    return TLinModel(WgtV, Bias);
}

// LIBSVM for Eps-Support Vector Regression for TFltVV input
inline TLinModel LibSvmSolveRegression(const TFltVV& VecV, const TFltV& TargetV,
        const double& Eps, const double& Cost, PNotify DebugNotify, PNotify ErrorNotify) {

    // Asserts for input arguments
    EAssertR(Cost > 0.0, "Cost parameter has to be positive.");

    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = EPSILON_SVR;
    svm_parameter.kernel_type = LINEAR;
    // If degree<0 svm_check_params reports an error, even though degree is
    // ignored by the learning when kernel_type!=polynomial
    svm_parameter.degree = 0;
    svm_parameter.C = Cost;
    svm_parameter.eps = Eps;
    svm_parameter.nr_weight = 0;
    svm_parameter.weight = NULL;
    svm_parameter.weight_label = NULL;
    // cache_size is only needed for kernel functions
    svm_parameter.cache_size = 100;
    svm_parameter.eps = 1e-3;	
	svm_parameter.p = 0.1; // not needed but it has to be positive as it is checked
    svm_parameter.shrinking = 0;
    svm_parameter.probability = 0;	
	svm_parameter.gamma = 1.0; //  not needed for linear SVM, but it has to be positive as it is checked

    const int DimN = VecV.GetXDim(); // Number of features
    const int AllN = VecV.GetYDim(); // Number of examples

    // svm_parameter.gamma = 1.0/DimN;

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
            if (VecV.At(Jdx, Idx) != 0.0) {
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

    // Learn the model
    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);

    // Make sure the WgtV is non-null, i.e., in case w=0 set WgtV to a vector
    // composed of a sufficient number of zeros (e.g. [0, 0, ..., 0]).
    TFltV WgtV(DimN);
    TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
    EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
    for (int Idx = 0; Idx < svm_model->l; ++Idx) {
        svm_node_t* SV = svm_model->SV[Idx];
        while (SV->index != -1) {
            WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
            ++SV;
        }
    }

    svm_free_and_destroy_model(&svm_model);
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);

    return TLinModel(WgtV, Bias);
}

// LIBSVM for C-Support Vector Classification for sparse input
inline TLinModel LibSvmSolveClassify(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV, const double& Cost, const double& Unbalance,
	PNotify DebugNotify, PNotify ErrorNotify) {

    // Asserts for input arguments
    EAssertR(Cost > 0.0, "Cost parameter has to be positive.");
    EAssertR(Unbalance > 0.0, "Unbalance parameter has to be positive.");

    // load training parameters
    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = C_SVC;
    svm_parameter.kernel_type = LINEAR;
    // If degree<0 svm_check_params reports an error, even though degree is
    // ignored by the learning when kernel_type!=polynomial
    svm_parameter.degree = 0;
    svm_parameter.C = Cost;
    svm_parameter.nr_weight = 2;
    svm_parameter.weight_label = (int *)malloc(2 * sizeof(int));
    svm_parameter.weight_label[0] = -1;
    svm_parameter.weight_label[1] = 1;
    svm_parameter.weight = (double *)malloc(2 * sizeof(double));
    svm_parameter.weight[0] = 1;
    svm_parameter.weight[1] = Unbalance;
    
    // cache_size is only needed for kernel functions
    svm_parameter.cache_size = 100;
    svm_parameter.eps = 1e-3;
    //svm_parameter.p = 0.1; // not needed but it has to be positive as it is checked
    svm_parameter.shrinking = 0;
    svm_parameter.probability = 0;
    //  not needed for linear SVM, but it has to be positive as it is checked
    svm_parameter.gamma = 1.0;

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

    // train the model
    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);

    // compute normal vector from support vectors
    TFltV WgtV(Dim);
    TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
    EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
    for (int Idx = 0; Idx < svm_model->l; ++Idx) {
        svm_node_t* SV = svm_model->SV[Idx];
        while (SV->index != -1) {
            WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
            ++SV;
        }
    }

    svm_free_and_destroy_model(&svm_model);
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);
    free(svm_model);

    return TLinModel(WgtV, Bias);
}

// Use LIBSVM for C-Support Vector Classification
inline TLinModel LibSvmSolveClassify(const TFltVV& VecV, const TFltV& TargetV, const double& Cost, const double& Unbalance,
	PNotify DebugNotify, PNotify ErrorNotify) {

    // Asserts for input arguments
    EAssertR(Cost > 0.0, "Cost parameter has to be positive.");

    // set trainig parameters
    svm_parameter_t svm_parameter;
    svm_parameter.svm_type = C_SVC;
    svm_parameter.kernel_type = LINEAR;
    // If degree<0 svm_check_params reports an error, even though degree is
    // ignored by the learning when kernel_type!=polynomial
    svm_parameter.degree = 0;
    svm_parameter.C = Cost;
    svm_parameter.nr_weight = 2;
    svm_parameter.weight_label = (int *)malloc(2 * sizeof(int));
    svm_parameter.weight_label[0] = -1;
    svm_parameter.weight_label[1] = 1;
    svm_parameter.weight = (double *)malloc(2 * sizeof(double));
    svm_parameter.weight[0] = 1;
    svm_parameter.weight[1] = Unbalance;
    // cache_size is only needed for kernel functions
    svm_parameter.cache_size = 100;
    svm_parameter.eps = 1e-3;
    //svm_parameter.p = 0.1; // not needed but it has to be positive as it is checked
    svm_parameter.shrinking = 0;
    svm_parameter.probability = 0;
    //  not needed for linear SVM, but it has to be positive as it is checked
    svm_parameter.gamma = 1.0;

    const int DimN = VecV.GetXDim(); // Number of features
    const int AllN = VecV.GetYDim(); // Number of examples

    // svm_parameter.gamma = 1.0/DimN;

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

    svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);

    TFltV WgtV(DimN);
    TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
    EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
    for (int Idx = 0; Idx < svm_model->l; ++Idx) {
        svm_node_t* SV = svm_model->SV[Idx];
        while (SV->index != -1) {
            WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
            ++SV;
        }
    }

    svm_free_and_destroy_model(&svm_model);
    svm_destroy_param(&svm_parameter);
    free(svm_problem.y);
    free(svm_problem.x);
    free(x_space);

    return TLinModel(WgtV, Bias);
}

template <class TVecV>
TLinModel SolveClassify(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const double& Cost, const double& UnbalanceWgt,
        const int& MxMSecs, const int& MxIter, const double& MnDiff, 
        const int& SampleSize, const PNotify& Notify = TStdNotify::New()) {

    // asserts for input parameters
    EAssertR(Dims > 0, "Dimensionality must be positive!");
    EAssertR(Vecs > 0, "Number of vectors must be positive!");
	EAssertR(Vecs == TargetV.Len(), "Number of vectors must be equal to the number of targets!");
    EAssertR(Cost > 0.0, "Cost parameter must be positive!");
    EAssertR(SampleSize > 0, "Sampling size must be positive!");
    EAssertR(MxIter > 1, "Number of iterations to small!");
    
    Notify->OnStatusFmt("SVM parameters: c=%.2f, j=%.2f", Cost, UnbalanceWgt);
    
    // initialization 
    TRnd Rnd(1); 
    const double Lambda = 1.0 / (double(Vecs) * Cost);
    // we start with random normal vector
    TFltV WgtV(Dims); TLinAlgTransform::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
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
    //  - the ration is uniform over the records when UnbalanceWgt == 1.0
    //  - if smaller then 1.0, then there is bias towards negative
    //  - if larger then 1.0, then there is bias towards positives
    double SamplingRatio = (double(PosVecs) * UnbalanceWgt) /
        (double(PosVecs) * UnbalanceWgt + double(NegVecs));
    Notify->OnStatusFmt("Sampling ration 1 positive vs %.2f negative [%.2f]", 
        (1.0 / SamplingRatio - 1.0), SamplingRatio);
    
    TTmTimer Timer(MxMSecs); int Iters = 0; double Diff = 1.0;
    Notify->OnStatusFmt("Limits: %d iterations, %.3f seconds, %.8f weight difference", MxIter, (double)MxMSecs /1000.0, MnDiff);
    // initialize profiler    
    TTmProfiler Profiler;
    const int ProfilerPre = Profiler.AddTimer("Pre");
    const int ProfilerBatch = Profiler.AddTimer("Batch");
    const int ProfilerPost = Profiler.AddTimer("Post");

    // function for writing progress reports
    int PosCount = 0, NegCount = 0;
    auto ProgressNotify = [&]() {
        Notify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g, ratio %.2f", 
            Iters, Timer.GetStopWatch().GetMSec() / 1000.0, Diff, 
            (double)PosCount / (double)(PosCount + NegCount));
        PosCount = 0; NegCount = 0;
    };
       
    for (int IterN = 0; IterN < MxIter; IterN++) {
        if (IterN % 100 == 0) { ProgressNotify(); }
        
        Profiler.StartTimer(ProfilerPre);
        // tells how much we can move
        const double Nu = 1.0 / (Lambda * double(IterN + 2)); // ??
        const double VecUpdate = Nu / double(SampleSize);
        // initialize updated normal vector
        TLinAlg::MultiplyScalar((1.0 - Nu * Lambda), WgtV, NewWgtV);
        Profiler.StopTimer(ProfilerPre);
        
        // classify examples from the sample
        Profiler.StartTimer(ProfilerBatch);
        int DiffCount = 0;
        for (int SampleN = 0; SampleN < SampleSize; SampleN++) {
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
            Notify->OnStatusFmt("Finishing due to reached time limit of %.3f seconds", (double)MxMSecs / 1000.0);
            break; 
        }
        // check stopping criteria with respect to result difference
        //if (DiffCount > 0 && (1.0 - DiffCos) < MnDiff) { 
        if (DiffCount > 0 && Diff < MnDiff) { 
            Notify->OnStatusFmt("Finishing due to reached difference limit of %g", MnDiff);
            break;
        }
    }
    if (Iters == MxIter) { 
        Notify->OnStatusFmt("Finished due to iteration limit of %d", Iters);
    }
    
    ProgressNotify();
    Profiler.PrintReport(Notify);
            
    return TLinModel(WgtV);
}
        
template <class TVecV>
inline TLinModel SolveRegression(const TVecV& VecV, const int& Dims, const int& Vecs,
	const TFltV& TargetV, const double& Cost, const double& Eps,
	const int& MxMSecs, const int& MxIter, const double& MnDiff,
	const int& SampleSize, const PNotify& Notify) {

	// asserts for input parameters
	EAssertR(Dims > 0, "Dimensionality must be positive!");
	EAssertR(Vecs > 0, "Number of vectors must be positive!");
	EAssertR(Vecs == TargetV.Len(), "Number of vectors must be equal to the number of targets!");
	EAssertR(Cost > 0.0, "Cost parameter must be positive!");
	EAssertR(SampleSize > 0, "Sampling size must be positive!");
	EAssertR(MxIter > 1, "Number of iterations to small!");
	EAssertR(MnDiff >= 0, "Min difference must be nonnegative!");
	
	// initialization 
	TRnd Rnd(1);
	const double Lambda = 1.0 / (double(Vecs) * Cost);
	// we start with random normal vector
	TFltV WgtV(Dims); TLinAlgTransform::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
	// Scale it to appropriate norm
	TLinAlg::MultiplyScalar(1.0 / (2.0 * TMath::Sqrt(Lambda)), WgtV, WgtV);

	// True norm is a product of Norm and TLinAlg::Norm(WgtV)
	// The reason for this is that we can have very cheap updates for sparse
	// vectors - we do not need to touch all elements of WgtV in each subgradient
	// update.
	double Norm = 1.0;
	double Normw = 1.0 / (2.0 * TMath::Sqrt(Lambda));
	
	TTmTimer Timer(MxMSecs); int Iters = 0; double Diff = 1.0;
	Notify->OnStatusFmt("Limits: %d iterations, %.3f seconds, %.8f weight difference", MxIter, (double)MxMSecs / 1000.0, MnDiff);
	// initialize profiler    
	TTmProfiler Profiler;
	const int ProfilerPre = Profiler.AddTimer("Pre");
	const int ProfilerBatch = Profiler.AddTimer("Batch");

	// function for writing progress reports
	auto ProgressNotify = [&]() {
		Notify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g",
			Iters, Timer.GetStopWatch().GetMSec() / 1000.0, Diff);
	};

	// Since we are using weight vector overrepresentation using Norm, we need to
	// compensate the scaling when adding examples using Coef
	double Coef = 1.0;
	for (int IterN = 0; IterN < MxIter; IterN++) {
		if (IterN % 100 == 0) { ProgressNotify(); }

		Profiler.StartTimer(ProfilerPre);
		// tells how much we can move
		const double Nu = 1.0 / (Lambda * double(IterN + 2));
		// update Coef which counters Norm		
		Coef /= (1 - Nu * Lambda);
		const double VecUpdate = Nu / (double(SampleSize)) * Coef;
		
		Profiler.StopTimer(ProfilerPre);
		// Track the upper bound on the change of norm of WgtV
		Diff = 0.0;
		// process examples from the sample
		Profiler.StartTimer(ProfilerBatch);
		// store which examples will lead to gradient updates (and their factors)
		TVec<TPair<TFlt, TInt> > Updates(SampleSize, 0);
		
		// in the first pass we find which samples will lead to updates
		for (int SampleN = 0; SampleN < SampleSize; SampleN++) {
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
			if (Loss < -Eps) { // y_i - z < -eps
				// update from the negative stochastic sub-gradient: -x
				Updates.Add(TPair<TFlt, TInt>(-VecUpdate, VecN));
				// update the norm of WgtV
				Normw = sqrt(Normw*Normw - 2 * VecUpdate * Dot + VecUpdate * VecUpdate * NorX * NorX);
				// update the bound on the change of norm of WgtV
				Diff += VecUpdate * NorX;
			} else if (Loss > Eps) { // y_i - z > eps
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
		//"Pegasos: Primal Estimated sub-GrAdient SOlver for SVM" Shai Shalev-Shwartz, Yoram Singer, Nathan Srebro, Andrew Cotter." Mathematical Programming, Series B, 127(1):3-30, 2011.

		// count
		Iters++;
		// check stopping criteria with respect to time
		if (Timer.IsTimeUp()) {
			Notify->OnStatusFmt("Finishing due to reached time limit of %.3f seconds", (double)MxMSecs / 1000.0);
			break;
		}
		// check stopping criteria with respect to result difference
		if (Diff < MnDiff) {
			Notify->OnStatusFmt("Finishing due to reached difference limit of %g", MnDiff);
			break;
		}
	}
	if (Iters == MxIter) {
		Notify->OnStatusFmt("Finished due to iteration limit of %d", Iters);
	}
	// Finally we use the Norm factor to rescale the weight vector
	TLinAlg::MultiplyScalar(Norm, WgtV, WgtV);
	
	ProgressNotify();

	Profiler.PrintReport(Notify);

	return TLinModel(WgtV);
}

};

#endif

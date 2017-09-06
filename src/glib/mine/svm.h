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
  
class TLibSvmParam{
  public:
    TInt Type;
    TInt Kernel;
    TInt Degree;
    TFlt Gamma;
    TFlt Coef0;
    
    TLibSvmParam() : Type(C_SVC), Kernel(LINEAR), Degree(0), Gamma(1.0), Coef0(1.0) {  } //fill with default values
    TLibSvmParam(TInt _Type, TInt _Kernel, TInt _Degree, TFlt _Gamma, TFlt _Coef0) : 
        Type(_Type), Kernel(_Kernel), Degree(_Degree), Gamma(_Gamma), Coef0(_Coef0) {  } //fill with default values
    
    TLibSvmParam(TSIn& SIn):
      Type(TInt(SIn)),
      Kernel(TInt(SIn)),
      Degree(TInt(SIn)),
      Gamma(TFlt(SIn)),
      Coef0(TFlt(SIn)){  }
    
    void Save(TSOut& SOut) const {
      TInt(Type).Save(SOut);
      TInt(Kernel).Save(SOut);
      TInt(Degree).Save(SOut);
      TFlt(Gamma).Save(SOut);
      TFlt(Coef0).Save(SOut);
    }
    
    svm_parameter_t GetParamStruct() const {//returns svm_parameter_t for LIBSVM predict
      svm_parameter_t svm_parameter;
      svm_parameter.svm_type = Type;//default
      svm_parameter.kernel_type = Kernel;//default
      svm_parameter.degree = Degree;
      svm_parameter.gamma = Gamma;
      svm_parameter.coef0 = Coef0;
      return svm_parameter;
    }
    
    svm_parameter_t GetParamStruct(double Cost, double Unbalance, double Nu, double Eps, double CacheSize, double P) const {//returns svm_parameter_t for LIBSVM train
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
    
};//end TSvmParam

/// Linear model
class TLinModel {
private:
    TFltV WgtV;
    TFlt Bias;
    TLibSvmParam Param;
    TFltVV SupportVectors; /// support vectors
    TFltVV Coef; /// coefficients for support vectors
    TFltV Rho; /// bias terms
public:
    TLinModel(): Bias(0.0) {  }
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) {  }
    TLinModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) {  }
    TLinModel(TInt _Type, TInt _Kernel, TInt _Degree, TFlt _Gamma, TFlt _Coef0): Bias(0.0) { Param = TLibSvmParam(_Type, _Kernel, _Degree, _Gamma, _Coef0); }

    TLinModel(TSIn& SIn):
        WgtV(SIn),
        Bias(SIn),
        Param(SIn),
        SupportVectors(SIn),
        Coef(SIn),
        Rho(SIn) {  }
        
    void Save(TSOut& SOut) const { 
        WgtV.Save(SOut);
        Bias.Save(SOut);
        Param.Save(SOut);
        SupportVectors.Save(SOut);
        Coef.Save(SOut);
        Rho.Save(SOut);
    }
    
    /// Get weight vector
    const TFltV& GetWgtV() const { return WgtV; }    
    
    /// Get bias
    double GetBias() const { return Bias; }
    
    /// Get number of support vectors
    int GetNSupportVectors() const { return SupportVectors.GetXDim(); }
    
    /// Get support vectors
    const TFltVV& GetSupportVectors() const { return SupportVectors; }
    
    /// Get coeficients for support vectors
    const TFltVV& GetCoefficients() const { return Coef; }
    
    /// Get biases (rho)
    const TFltV& GetBiases() const { return Rho; }
    
    /// Get svm_model struct
    svm_model_t* GetModelStruct() const;
    
    /// Classify full vector
    double Predict(const TFltV& Vec) const { 
      if (Param.Kernel == LINEAR)
        return TLinAlg::DotProduct(WgtV, Vec) + Bias; 
      svm_model_t* model = GetModelStruct();
      svm_node_t *x = (svm_node_t *)malloc((Vec.Len() + 1) * sizeof(svm_node_t));
      double *dec_val = (double *)malloc(model->nr_class*(model->nr_class-1)/2 * sizeof(double));
      for (int Idx = 0; Idx < Vec.Len(); Idx++){
        x[Idx].index = Idx;
        x[Idx].value = Vec[Idx];
      }
      x[Vec.Len()].index = -1;
      svm_predict_values(model, x, dec_val);
      double result = dec_val[0];
      
      svm_free_and_destroy_model(&model);
      free(x);
      free(dec_val);
      free(model);
      
      return result;
    }
    
    /// Classify sparse vector
    double Predict(const TIntFltKdV& SpVec) const { 
      if (Param.Kernel == LINEAR)
        return TLinAlg::DotProduct(WgtV, SpVec) + Bias;
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

    /// Classify matrix column vector
    double Predict(const TFltVV& Mat, const int& ColN) const {
      if (Param.Kernel == LINEAR)
        return TLinAlg::DotProduct(Mat, ColN, WgtV) + Bias;
      int DimX = Mat.GetXDim();
      TFltV Col(DimX);
      Mat.GetCol(ColN, Col);
      return Predict(Col);
    }
       
    /// LIBSVM for sparse input
    void LibSvmFit(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV, double Cost, double Unbalance, double Nu, double Eps, double CacheSize, double P,
            PNotify DebugNotify, PNotify ErrorNotify);

    /// Use LIBSVM for dense input
    void LibSvmFit(const TFltVV& VecV, const TFltV& TargetV, double Cost, double Unbalance, double Nu, double Eps, double CacheSize, double P,
            PNotify DebugNotify, PNotify ErrorNotify);
}; //

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

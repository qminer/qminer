/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef SVM_H
#define	SVM_H

#include "../base/base.h"

namespace TSvm {

/// Linear model
class TLinModel {
private:
    TFltV WgtV;
    TFlt Bias;
public:
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) {  }
    TLinModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) {  }

    TLinModel(TSIn& SIn): WgtV(SIn), Bias(SIn) { }
    void Save(TSOut& SOut) const { WgtV.Save(SOut); Bias.Save(SOut); }
    
    /// Get weight vector
    void GetWgtV(TFltV& _WgtV) const { _WgtV = WgtV; }
    
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
};

template <class TVecV>
TLinModel SolveClassify(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const double& Cost, const double& UnbalanceWgt,
        const int& MxMSecs, const int& MxIter, const double& MnDiff, 
        const int& SampleSize, const PNotify& Notify = TStdNotify::New()) {

    // asserts for input parameters
    EAssertR(Dims > 0, "Dimensionality must be positive!");
    EAssertR(Vecs > 0, "Number of vectors must be positive!");
    EAssertR(Cost > 0.0, "Cost parameter must be positive!");
    EAssertR(SampleSize > 0, "Sampling size must be positive!");
    EAssertR(MxIter > 1, "Number of iterations to small!");
    
    Notify->OnStatusFmt("SVM parameters: c=%.2f, j=%.2f", Cost, UnbalanceWgt);
    
    // initialization 
    TRnd Rnd(1); 
    const double Lambda = 1.0 / (double(Vecs) * Cost);
    // we start with random normal vector
    TFltV WgtV(Dims); TLAMisc::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
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
TLinModel SolveRegression(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const double& Cost, const double& Eps,
        const int& MxMSecs, const int& MxIter, const double& MnDiff, 
        const int& SampleSize, const PNotify& Notify = TStdNotify::New()) {

    // asserts for input parameters
    EAssertR(Dims > 0, "Dimensionality must be positive!");
    EAssertR(Vecs > 0, "Number of vectors must be positive!");
    EAssertR(Cost > 0.0, "Cost parameter must be positive!");
    EAssertR(SampleSize > 0, "Sampling size must be positive!");
    EAssertR(MxIter > 1, "Number of iterations to small!");
    
    // initialization 
    TRnd Rnd(1); 
    const double Lambda = 1.0 / (double(Vecs) * Cost);
    // we start with random normal vector
    TFltV WgtV(Dims); TLAMisc::FillRnd(WgtV, Rnd); TLinAlg::Normalize(WgtV);
    // make it of appropriate length
    TLinAlg::MultiplyScalar(1.0 / (2.0 * TMath::Sqrt(Lambda)), WgtV, WgtV);
    // allocate space for updates
    TFltV NewWgtV(Dims);

    TTmTimer Timer(MxMSecs); int Iters = 0; double Diff = 1.0;
    Notify->OnStatusFmt("Limits: %d iterations, %.3f seconds, %.8f weight difference", MxIter, (double)MxMSecs / 1000.0, MnDiff);
    // initialize profiler    
    TTmProfiler Profiler;
    const int ProfilerPre = Profiler.AddTimer("Pre");
    const int ProfilerBatch = Profiler.AddTimer("Batch");
    const int ProfilerPost = Profiler.AddTimer("Post");

    // function for writing progress reports
    auto ProgressNotify = [&]() {
        Notify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g", 
            Iters, Timer.GetStopWatch().GetMSec() / 1000.0, Diff);
    };
       
    for (int IterN = 0; IterN < MxIter; IterN++) {
        if (IterN % 100 == 0) { ProgressNotify(); }
        
        Profiler.StartTimer(ProfilerPre);
        // tells how much we can move
        const double Nu = 1.0 / (Lambda * double(IterN + 2));
        const double VecUpdate = Nu / double(SampleSize);
        // initialize updated normal vector
        TLinAlg::MultiplyScalar((1.0 - Nu * Lambda), WgtV, NewWgtV);
        Profiler.StopTimer(ProfilerPre);
        
        // process examples from the sample
        Profiler.StartTimer(ProfilerBatch);
        for (int SampleN = 0; SampleN < SampleSize; SampleN++) {            
            const int VecN = Rnd.GetUniDevInt(Vecs);
            // target
            const double Target = TargetV[VecN];
            // prediction
            const double Pred = TLinAlg::DotProduct(VecV, VecN, WgtV);
            // difference
            const double Loss = Target - Pred;
            // do the update based on the difference
            if (Loss < -Eps) { // y - z < -eps
                // update from the stochastic sub-gradient: x
                TLinAlg::AddVec(-VecUpdate, VecV, VecN, NewWgtV, NewWgtV);                
            } else if (Loss > Eps) { // y - z > eps
                // update from the stochastic sub-gradient: -x
                TLinAlg::AddVec(VecUpdate, VecV, VecN, NewWgtV, NewWgtV);
            } // else nothing to do, we are within the epsilon tube
        }
        Profiler.StopTimer(ProfilerBatch);

        Profiler.StartTimer(ProfilerPost);
        // project the current solution on to a ball
        const double WgtNorm = 1.0 / (TLinAlg::Norm(NewWgtV) * TMath::Sqrt(Lambda));
        if (WgtNorm < 1.0) { TLinAlg::MultiplyScalar(WgtNorm, NewWgtV, NewWgtV); }
        // compute the difference with respect to the previous iteration
        Diff = 2.0 * TLinAlg::EuclDist(WgtV, NewWgtV) / (TLinAlg::Norm(WgtV) + TLinAlg::Norm(NewWgtV));
        // remember new solution
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
        if (Diff < MnDiff) { 
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

};

#endif

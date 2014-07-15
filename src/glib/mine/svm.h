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
public:
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV) {  }

    TLinModel(TSIn& SIn): WgtV(SIn) { }
    void Save(TSOut& SOut) const { WgtV.Save(SOut); }
    
    /// Get weight vector
    void GetWgtV(TFltV& _WgtV) const {
        _WgtV = WgtV;
    }
    
    /// Classify full vector
    double Predict(const TFltV& Vec) const { 
        return TLinAlg::DotProduct(WgtV, Vec); 
    }
    
    /// Classify sparse vector
    double Predict(const TIntFltKdV& SpVec) const { 
        return TLinAlg::DotProduct(WgtV, SpVec);
    }
};

template <class TVecV>
TLinModel SolveClassify(const TVecV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV, const double& Cost, const int& MxTime, 
        const int& MxIter, const double& MnDiff, const int& SampleSize,
        const PNotify& Notify = TStdNotify::New()) {

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

    TExeTm ExeTm; int Iters = 0; double Diff = 1.0;
    Notify->OnStatusFmt("Limits: %d iterations, %d seconds, %.8f weight difference", MxIter, MxTime, MnDiff);
    auto ProgressNotify = [&]() { 
        Notify->OnStatusFmt("  %d iterations, %.3f seconds, last weight difference %g", Iters, ExeTm.GetSecs(), Diff);
    };
    
    for (int IterN = 1; IterN < MxIter; IterN++) {
        if (IterN % 100 == 0) { ProgressNotify(); }
        // tells how much we can move
        const double Nu = 1.0 / (Lambda * double(IterN + 1));
        const double VecUpdate = Nu / double(SampleSize);
        // initialize updated normal vector
        TLinAlg::MultiplyScalar((1.0 - Nu * Lambda), WgtV, NewWgtV);
        // classify examples from the sample
        for (int SampleN = 0; SampleN < SampleSize; SampleN++) {
            const int VecN = Rnd.GetUniDevInt(Vecs);
            const double VecCfyVal = TargetV[VecN];
            const double CfyVal = VecCfyVal * TLinAlg::DotProduct(VecV, VecN, WgtV);
            if (CfyVal < 1.0) { 
                // with update from the stochastic sub-gradient
                TLinAlg::AddVec(VecUpdate * VecCfyVal, VecV, VecN, NewWgtV, NewWgtV);
            }
        }
        // project the current solution on to a ball
        const double WgtNorm = 1.0 / (TLinAlg::Norm(NewWgtV) * TMath::Sqrt(Lambda));
        if (WgtNorm < 1.0) { TLinAlg::MultiplyScalar(WgtNorm, NewWgtV, NewWgtV); }
        // compute the difference with respect to the previous iteration
        Diff = 2.0 * TLinAlg::EuclDist(WgtV, NewWgtV) / (TLinAlg::Norm(WgtV) + TLinAlg::Norm(NewWgtV));
        // remember new solution
        WgtV = NewWgtV;

        // count
        Iters++;
        // check stopping criteria with respect to time
        if (ExeTm.GetSecInt() > MxTime) {
            Notify->OnStatusFmt("Finishing due to reached time limit of %ds", MxTime);
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
            
    return TLinModel(WgtV);
}
        
};

#endif

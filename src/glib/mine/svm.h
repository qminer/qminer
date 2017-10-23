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

#include "../../qminer/qminer_core.h"

namespace TSvm {
  
enum { LIBSVM_CSVC, LIBSVM_NUSVC, LIBSVM_ONECLASS, LIBSVM_EPSILONSVR, LIBSVM_NUSVR, DEFAULT }; /// constants for Type, LIBSVM specific
enum { LIBSVM_LINEAR, LIBSVM_POLY, LIBSVM_RBF, LIBSVM_SIGMOID, LIBSVM_PRECOMPUTED }; /// constants for Kernel, LIBSVM specific
  
/// abstract class
class TSvmModel{
public:
    virtual  ~TSvmModel() {  }
    virtual void Load(TSIn& SIn) = 0;
    virtual void Save(TSOut& SOut) const = 0;
    
    virtual void UpdateParams(const PJsonVal& ParamVal) = 0;
    virtual PJsonVal GetParams() const = 0;
    
    virtual void FitRegression(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
          const TFltV& TargetV) = 0;
    virtual void FitClassification(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
          const TFltV& TargetV) = 0;
    virtual void FitRegression(const TFltVV& VecV, const int& Dims, const int& Vecs,
          const TFltV& TargetV) = 0;
    virtual void FitClassification(const TFltVV& VecV, const int& Dims, const int& Vecs,
          const TFltV& TargetV) = 0;
    
    virtual double Predict(const TFltV& Vec) const = 0;
    virtual double Predict(const TIntFltKdV& SpVec) const = 0;
    virtual double Predict(const TFltVV& Mat, const int& ColN) const = 0;
    
    virtual const TFltV& GetWgtV() const = 0;
    virtual double GetBias() const = 0;
};

/// TLinParam
class TLinParam{
  public:
    TFlt Cost;
    TFlt Unbalance;
    TFlt Eps;
    TInt SampleSize;
    TInt MxIter;
    TInt MxTime;
    TFlt MnDiff;
    TBool Verbose;
    TWPt<TNotify> Notify;
    
    TLinParam() : Cost(1.0), Unbalance(1.0), Eps(1e-3), SampleSize(1000), MxIter(10000), MxTime(1000*1), MnDiff(1e-6), Verbose(false), Notify(TNotify::NullNotify()) {  } //fill with default values
    TLinParam(double _Cost, double _Unbalance, int _SampleSize, int _MxIter, int _MxTime, double _MnDiff, bool _Verbose) : 
        Cost(_Cost), Unbalance(_Unbalance), SampleSize(_SampleSize), MxIter(_MxIter), MxTime(_MxTime), MnDiff(_MnDiff), Verbose(_Verbose), Notify(TNotify::NullNotify()) { 
            if (Verbose) { /*Notify = TQm::TEnv::Debug();*/}
        } //fill with custom values*/
    ~TLinParam() {  }
    
    TLinParam(TSIn& SIn);
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
};

/// Linear model
class TLinModel : public TSvmModel{
private:
    TFltV WgtV;
    TFlt Bias;
    TLinParam Param;
public:    
    TLinModel(): Bias(0.0) { Param = TLinParam(); }
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) { Param = TLinParam(); }
    TLinModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) { Param = TLinParam(); }
    ~TLinModel() {  }
    
    TLinModel(TSIn& SIn);
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
    
    void UpdateParams(const PJsonVal& ParamVal);
    PJsonVal GetParams() const;
    
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
    
    void FitClassification(const TFltVV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    void FitRegression(const TFltVV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    void FitClassification(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    void FitRegression(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    
    template <class TVecV>
    void SolveClassification(const TVecV& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV);
    template <class TVecV>
    void SolveRegression(const TVecV& VecV, const int& Dims, const int& Vecs,
    const TFltV& TargetV);     
};

class TLibSvmParam{
  public:
    TInt Type; /// LIBSVM specific: C_SVC, NU_SVC, EPSILON_SVR, NU_SVR, ONE_CLASS
    TInt Kernel; /// LIBSVM specific: LINEAR, POLY, RBF, SIGMOID, PRECOMPUTED
    TFlt Cost; /// parameter C of C-SVC, epsilon-SVR, and nu-SVR, SGD and PR_LOQO (default 1)
    TFlt Unbalance; /// parameter J of SGD and PR_LOQO, in LIBSVM this is used for weight calculation (default 1)
    TFlt Eps; /// LIBSVM specific: set tolerance of termination criterion (default 0.001)
    TFlt Gamma; /// LIBSVM specific: set gamma in kernel function (default 1.0)
    TFlt P; /// LIBSVM specific: set the epsilon in loss function of epsilon-SVR (default 0.1)
    TInt Degree;  /// LIBSVM specific: set degree in kernel function (default 1)
    TFlt Nu; /// LIBSVM specific: set the parameter nu of nu-SVC, one-class SVM, and nu-SVR (default 0.01)
    TFlt Coef0; /// LIBSVM specific: set coef0 in kernel function (default 1.0)
    TFlt CacheSize; /// LIBSVM specific: set cache memory size in MB (default 100)
    TBool Verbose;
    TWPt<TNotify> Notify;
    
    TLibSvmParam() : Type(DEFAULT), Kernel(LIBSVM_LINEAR), Cost(1.0), Unbalance(1.0), Eps(1e-3), Gamma(1.0), P(0.1), Degree(1), Nu(1e-2), Coef0(1.0), CacheSize(100), Verbose(false), Notify(TNotify::NullNotify()) {  } //fill with default values
    TLibSvmParam(int _Type, int _Kernel, double _Cost, double _Unbalance, double _Eps, double _Gamma, double _P, int _Degree, double _Nu, double _Coef0, double _CacheSize, bool _Verbose) : 
        Type(_Type), Kernel(_Kernel), Cost(_Cost), Unbalance(_Unbalance), Eps(_Eps), Gamma(_Gamma), P(_P), Degree(_Degree), Nu(_Nu), Coef0(_Coef0), CacheSize(_CacheSize), Verbose(_Verbose), Notify(TNotify::NullNotify()) { 
            if (Verbose) { /*Notify = TQm::TEnv::Debug();*/ }
        } //fill with custom values
    ~TLibSvmParam() {  }
    
    TLibSvmParam(TSIn& SIn);
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
    
    svm_parameter_t GetParamStruct() const;
};//end TSvmParam
  
class TLibSvmPredictParam{
  public:
    TInt Type;
    TInt Kernel;
    TFlt Gamma;
    TInt Degree;
    TFlt Coef0;
    
    TLibSvmPredictParam() : Type(DEFAULT), Kernel(LINEAR), Gamma(1.0), Degree(0), Coef0(1.0) {  } //fill with default values
    TLibSvmPredictParam(TInt _Type, TInt _Kernel, TFlt _Gamma, TInt _Degree, TFlt _Coef0) : 
        Type(_Type), Kernel(_Kernel), Gamma(_Gamma), Degree(_Degree), Coef0(_Coef0) {  } //fill with default values
    ~TLibSvmPredictParam() {  }
    
    TLibSvmPredictParam(TSIn& SIn);
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
       
    svm_parameter_t GetPredictParamStruct() const;//returns svm_parameter_t for LIBSVM predict  
};//end TSvmPredictParam

/// General model
class TLibSvmModel : public TSvmModel{
private:
    TFltV WgtV;
    TFlt Bias;
    TLibSvmParam Param; // parameters needed for training
    TLibSvmPredictParam PredictParam; // parameters needed for prediction (duplicated to be sure that the parameters don't change between calling methods fit and predict)
    TFltVV SupportVectors; // support vectors
    TFltVV Coef; // coefficients for support vectors
    TFltV Rho; // bias terms
    TIntV NSupportVectors; // number of support vectors for each class, needed for prediction
public:
    TLibSvmModel(): Bias(0.0) {  }
    TLibSvmModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) {  }
    TLibSvmModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) {  }
    TLibSvmModel(TInt _Type, TInt _Kernel, TInt _Degree, TFlt _Gamma, TFlt _Coef0): Bias(0.0) { Param = TLibSvmParam(); }
    ~TLibSvmModel() {  }
    
    TLibSvmModel(TSIn& SIn);
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
    
    void UpdateParams(const PJsonVal& ParamVal);
    PJsonVal GetParams() const;
    
    /// Get weight vector
    virtual const TFltV& GetWgtV() const { return WgtV; }    
    
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
    /// inverse of ConvertResults(svm_model_t* model)
    /// to delete model and free memory use DeleteModelStruct(svm_model_t* svm_model)
    svm_model_t* GetModelStruct() const;
    
    /// converts model from LIBSVM svm_model_t to TLibSvmModel class
    /// inverse of GetModelStruct
    void ConvertResults(svm_model_t* model, int Dim);
    
    /// free memory
    /// deletes struct created with 
    void DeleteModelStruct(svm_model_t* svm_model) const;
    
    double Predict(const TFltV& Vec) const;
    double Predict(const TIntFltKdV& SpVec) const;
    double Predict(const TFltVV& Mat, const int& ColN) const;
    
    /// LIBSVM for sparse input
    void FitClassification(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    void FitRegression(const TVec<TIntFltKdV>& VecV, const int& Dims, const int& Vecs, const TFltV& TargetV);
    /// Use LIBSVM for dense input
    void FitClassification(const TFltVV& VecV, const int& Dims, const int& Vecs,
        const TFltV& TargetV);
    void FitRegression(const TFltVV& VecV, const int& Dims, const int& Vecs, const TFltV& TargetV); 
}; //

}; //end namespace

#endif

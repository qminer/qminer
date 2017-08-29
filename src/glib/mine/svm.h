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
  
class TSvmParam{
  public:
  //private:
    TInt Type;
    TInt Kernel;
    TInt Degree;
    TFlt Cost;
    TFlt Gamma;
    TFlt Nu;
    TFlt Coef0;
    TFlt Eps;
    TFlt CacheSize;
    TFlt P;
    TInt Shrink;
    TInt Prob;
  //public:
    TSvmParam() : Type(C_SVC), Kernel(LINEAR), Degree(0), Cost(1.0), Gamma(1.0), Nu(1.0), Coef0(1.0), Eps(1e-3), CacheSize(100), P(0.1), Shrink(0), Prob(0)  {  } //fill with default values
    TSvmParam(TInt _Type, TInt _Kernel, TFlt _Cost, TFlt _Gamma, TInt _Degree, TFlt _Coef0,  TFlt _CacheSize, TFlt _Eps, TFlt _P, TFlt _Nu) : 
        Type(_Type), Kernel(_Kernel), Degree(_Degree), Cost(_Cost), Gamma(_Gamma), Nu(_Nu), Coef0(_Coef0), Eps(_Eps), CacheSize(_CacheSize), P(_P), Shrink(0), Prob(0)  {  } //fill with default values
    
    TSvmParam(TSIn& SIn):
      Type(TInt(SIn)),
      Kernel(TInt(SIn)),
      Degree(TInt(SIn)),
      Cost(TFlt(SIn)),
      Gamma(TFlt(SIn)),
      Nu(TFlt(SIn)),
      Coef0(TFlt(SIn)),
      Eps(TFlt(SIn)),
      CacheSize(TFlt(SIn)),
      P(TFlt(SIn)),
      Shrink(TInt(SIn)),
      Prob(TInt(SIn)) {  }
    
    void Save(TSOut& SOut) const {
      TInt(Type).Save(SOut);
      TInt(Kernel).Save(SOut);
      TInt(Degree).Save(SOut);
      TFlt(Cost).Save(SOut);
      TFlt(Gamma).Save(SOut);
      TFlt(Nu).Save(SOut);
      TFlt(Coef0).Save(SOut);
      TFlt(Eps).Save(SOut);
      TFlt(CacheSize).Save(SOut);
      TFlt(P).Save(SOut);
      TInt(Shrink).Save(SOut);
      TInt(Prob).Save(SOut);
    }
    
    svm_parameter_t GetParamStruct() const {//returns svm_parameter_t for LIBSVM
      svm_parameter_t svm_parameter;
      svm_parameter.svm_type = this->Type;//default
      svm_parameter.kernel_type = this->Kernel;//default
      svm_parameter.degree = this->Degree;
      svm_parameter.C = this->Cost;
      svm_parameter.gamma = this->Gamma;
      svm_parameter.nu = this->Nu;
      svm_parameter.coef0 = this->Coef0;
      svm_parameter.nr_weight = 0;
      svm_parameter.weight = NULL;
      svm_parameter.weight_label = NULL;
      svm_parameter.cache_size = this->CacheSize;
      svm_parameter.eps = this->Eps;
      svm_parameter.p = this->P; // not needed but it has to be positive as it is checked
      svm_parameter.shrinking = this->Shrink;
      svm_parameter.probability = this->Prob;
      return svm_parameter;
    }
};//end TSvmParam

/// Linear model
class TLinModel {
private:
    TFltV WgtV;
    TFlt Bias;
    TSvmParam Param;
    TInt NClass; //number of classes
    TInt NSV; //number of support vectors
    TFltVV SV; //support vectors
    TFltVV Coef; //coefficients for support vectors
    TFltV Rho; //bias terms
    TFltV ProbA;
    TFltV ProbB;
    TIntV SVIndices; //indices of support vectors in data
    TIntV Label; // for classification only
    TIntV nSV; // for classification only
    TInt Free;
public:
    TLinModel(): Bias(0.0) { this->Param = TSvmParam(); }
    TLinModel(const TFltV& _WgtV): WgtV(_WgtV), Bias(0.0) { this->Param = TSvmParam(); }
    TLinModel(const TFltV& _WgtV, const double& _Bias): WgtV(_WgtV), Bias(_Bias) { this->Param = TSvmParam(); }

    TLinModel(TSIn& SIn):
        WgtV(SIn),
        Bias(SIn),
        Param(SIn),
        NClass(SIn),
        NSV(SIn),
        SV(SIn),
        Coef(SIn),
        Rho(SIn),
        ProbA(SIn),
        ProbB(SIn),
        SVIndices(SIn),
        Label(SIn),
        nSV(SIn),
        Free(SIn) {  }
        
    void Save(TSOut& SOut) const { 
        WgtV.Save(SOut);
        Bias.Save(SOut);
        Param.Save(SOut);
        NClass.Save(SOut);
        NSV.Save(SOut);
        SV.Save(SOut);
        Coef.Save(SOut);
        Rho.Save(SOut);
        ProbA.Save(SOut);
        ProbB.Save(SOut);
        SVIndices.Save(SOut);
        Label.Save(SOut);
        nSV.Save(SOut);
        Free.Save(SOut);
    }
    
    /// Get weight vector
    const TFltV& GetWgtV() const { return WgtV; }    
    
    /// Get bias
    double GetBias() const { return Bias; }
    
    /// Get number of support vectors
    int GetNSupportVectors() const { return NSV; }
    
    /// Get support vectors
    const TFltVV& GetSupportVectors() const { return SV; }
    
    /// Get coeficients for support vectors
    const TFltVV& GetCoefficients() const { return Coef; }
    
    /// Get biases (rho)
    const TFltV& GetBiases() const { return Rho; }
    
    void SetParam(TInt Type, TInt Kernel, TFlt Cost, TFlt Gamma, TInt Degree, TFlt Coef0, TFlt CacheSize, TFlt Eps, TFlt P, TFlt Nu) {
      this->Param = TSvmParam(Type, Kernel, Cost, Gamma, Degree, Coef0, CacheSize, Eps, P, Nu);
    }
    
    /// Get svm_model struct
    svm_model_t GetModelStruct() const {
          svm_model_t svm_model;
          svm_model.param = this->Param.GetParamStruct();
          svm_model.nr_class = this->NClass;
          svm_model.l = this->NSV;
          svm_model.free_sv = this->Free;
          int DimX = this->SV.GetXDim();
          int DimY = this->SV.GetYDim();
          svm_model.SV = (svm_node_t **)malloc(DimX * sizeof(svm_node_t *));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.SV[Idx] = (svm_node_t *)malloc((DimY+ 1) * sizeof(svm_node_t));
            for (int cIdx = 0; cIdx < DimY; cIdx ++){
              svm_model.SV[Idx][cIdx].index = cIdx;
              svm_model.SV[Idx][cIdx].value = this->SV.GetXY(Idx, cIdx);
            }
            svm_model.SV[Idx][DimY].index = -1;
          }
          DimX = this->Coef.GetXDim();
          DimY = this->Coef.GetYDim();
          svm_model.sv_coef = (double **)malloc(DimX * sizeof(double *));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.sv_coef[Idx] = (double *)malloc(DimY * sizeof(double));
            for (int cIdx = 0; cIdx < DimY; cIdx ++){
              svm_model.sv_coef[Idx][cIdx] = this->Coef.GetXY(Idx, cIdx);
            }
          }
          DimX = this->Rho.Len();
          svm_model.rho = (double *)malloc(DimX * sizeof(double));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.rho[Idx] = this->Rho[Idx];
          }
          DimX = this->ProbA.Len();
          svm_model.probA = (double *)malloc(DimX * sizeof(double));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.probA[Idx] = this->ProbA[Idx];
          }
          DimX = this->ProbB.Len();
          svm_model.probB = (double *)malloc(DimX * sizeof(double));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.probB[Idx] = this->ProbB[Idx];
          }
          DimX = this->SVIndices.Len();
          svm_model.sv_indices = (int *)malloc(DimX * sizeof(int));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.sv_indices[Idx] = this->SVIndices[Idx];
          }
          DimX = this->Label.Len();
          svm_model.label = (int *)malloc(DimX * sizeof(int));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.label[Idx] = this->Label[Idx];
          }
          DimX = this->nSV.Len();
          svm_model.nSV = (int *)malloc(DimX * sizeof(int));
          for (int Idx = 0; Idx < DimX; Idx++){
            svm_model.nSV[Idx] = this->nSV[Idx];
          }    

          return svm_model;
        }

    /// Classify full vector
    double Predict(const TFltV& Vec) const { 
      if (Param.Kernel == LINEAR)
        return TLinAlg::DotProduct(WgtV, Vec) + Bias; 
      svm_model_t model = GetModelStruct();
      svm_node_t *x = (svm_node_t *)malloc((Vec.Len() + 1) * sizeof(svm_node_t));
      double *dec_val = (double *)malloc(model.nr_class*(model.nr_class-1)/2 * sizeof(double));
      for (int Idx = 0; Idx < Vec.Len(); Idx++){
        x[Idx].index = Idx;
        x[Idx].value = Vec[Idx];
      }
      x[Vec.Len()].index = -1;
      svm_predict_values(&model, x, dec_val);
      return dec_val[0];
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
    
    // LIBSVM for C-Support Vector Classification for sparse input
    void LibSvmSolveClassify(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV,
            PNotify DebugNotify, PNotify ErrorNotify) {
        
        // load training parameters
        svm_parameter_t svm_parameter = this->Param.GetParamStruct();
        
        // Asserts for input arguments
        EAssertR(this->Param.Cost > 0.0, "Cost parameter has to be positive.");

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
        
        TFltVV SVs(svm_model->l, Dim);
        TFltVV Coef(svm_model->nr_class - 1, svm_model->l);
        TFltV Rho(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbA(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbB(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TIntV SVIndices(svm_model->l);
        TIntV Label(svm_model->nr_class);
        TIntV nSV(svm_model->nr_class);

        // compute normal vector from support vectors
        TFltV WgtV(Dim);
        TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
        EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
        for (int Idx = 0; Idx < svm_model->l; ++Idx) {
            svm_node_t* SV = svm_model->SV[Idx];
            while (SV->index != -1) {
                SVs.PutXY(Idx, SV->index - 1, SV->value);
                WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
                ++SV;
            }
            SVIndices.SetVal(Idx, svm_model->sv_indices[Idx]);
            for (int cIdx = 0; cIdx < svm_model->nr_class - 1; cIdx++){
              Coef.PutXY(cIdx, Idx, svm_model->sv_coef[cIdx][Idx]);
            }
        }        
        for (int Idx = 0; Idx < svm_model->nr_class * (svm_model->nr_class - 1)/2; Idx++){
            Rho.SetVal(Idx, svm_model->rho[Idx]);
        }          
        for (int Idx = 0; Idx < svm_model->nr_class; Idx++){
          Label.SetVal(Idx, svm_model->label[Idx]);
          nSV.SetVal(Idx, svm_model->nSV[Idx]);
        }
        
        this->WgtV = WgtV;
        this->Bias = Bias;
        this->NClass = svm_model->nr_class;
        this->NSV = svm_model->l;
        this->SV = SVs;
        this->Coef = Coef;
        this->Rho = Rho;
        this->ProbA = ProbA;
        this->ProbB = ProbB;
        this->SVIndices = SVIndices;
        this->Label = Label;
        this->nSV = nSV;
        this->Free = svm_model->free_sv;

        svm_free_and_destroy_model(&svm_model);
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
        free(svm_model);
    }

    // Use LIBSVM for C-Support Vector Classification
    void LibSvmSolveClassify(const TFltVV& VecV, const TFltV& TargetV,
            PNotify DebugNotify, PNotify ErrorNotify) {
      
        // load training parameters
        svm_parameter_t svm_parameter = this->Param.GetParamStruct();

        // Asserts for input arguments
        EAssertR(this->Param.Cost > 0.0, "Cost parameter has to be positive.");

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

        svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);
        
        TFltVV SVs(svm_model->l, DimN);
        TFltVV Coef(svm_model->nr_class - 1, svm_model->l);
        TFltV Rho(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbA(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbB(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TIntV SVIndices(svm_model->l);
        TIntV Label(svm_model->nr_class);
        TIntV nSV(svm_model->nr_class);        
        TFltV WgtV(DimN);
        TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
        EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
        for (int Idx = 0; Idx < svm_model->l; ++Idx) {
            svm_node_t* SV = svm_model->SV[Idx];
            while (SV->index != -1) {
                SVs.PutXY(Idx, SV->index - 1, SV->value);
                WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
                ++SV;
            }
            SVIndices.SetVal(Idx, svm_model->sv_indices[Idx]);
            for (int cIdx = 0; cIdx < svm_model->nr_class - 1; cIdx++){
              Coef.PutXY(cIdx, Idx, svm_model->sv_coef[cIdx][Idx]);
            }
        }        
        for (int Idx = 0; Idx < svm_model->nr_class * (svm_model->nr_class - 1)/2; Idx++){
          Rho.SetVal(Idx, svm_model->rho[Idx]);
        }        
        for (int Idx = 0; Idx < svm_model->nr_class; Idx++){
          Label.SetVal(Idx, svm_model->label[Idx]);
          nSV.SetVal(Idx, svm_model->nSV[Idx]);
        }
        
        this->WgtV = WgtV;
        this->Bias = Bias;
        this->NClass = svm_model->nr_class;
        this->NSV = svm_model->l;
        this->SV = SVs;
        this->Coef = Coef;
        this->Rho = Rho;
        this->ProbA = ProbA;
        this->ProbB = ProbB;
        this->SVIndices = SVIndices;
        this->Label = Label;
        this->nSV = nSV;
        this->Free = svm_model->free_sv;

        svm_free_and_destroy_model(&svm_model);
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
    }

    // LIBSVM for Eps-Support Vector Regression for sparse input
    void LibSvmSolveRegression(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV,
            PNotify DebugNotify, PNotify ErrorNotify) {
        // load training parameters
        svm_parameter_t svm_parameter = this->Param.GetParamStruct();

        // Asserts for input arguments
        EAssertR(this->Param.Cost > 0.0, "Cost parameter has to be positive.");

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
        
        TFltVV SVs(svm_model->l, Dim);
        TFltVV Coef(svm_model->nr_class - 1, svm_model->l);
        TFltV Rho(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbA(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbB(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TIntV SVIndices(svm_model->l);
        
        // compute normal vector from support vectors
        TFltV WgtV(Dim);
        TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
        EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
        for (int Idx = 0; Idx < svm_model->l; ++Idx) {
            svm_node_t* SV = svm_model->SV[Idx];
            while (SV->index != -1) {
                SVs.PutXY(Idx, SV->index - 1, SV->value);
                WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
                ++SV;
            }
            SVIndices.SetVal(Idx, svm_model->sv_indices[Idx]);
            for (int cIdx = 0; cIdx < svm_model->nr_class - 1; cIdx++){
              Coef.PutXY(cIdx, Idx, svm_model->sv_coef[cIdx][Idx]);
            }
        }        
        for (int Idx = 0; Idx < svm_model->nr_class * (svm_model->nr_class - 1)/2; Idx++){
            Rho.SetVal(Idx, svm_model->rho[Idx]);
        }          
        
        this->WgtV = WgtV;
        this->Bias = Bias;
        this->NClass = svm_model->nr_class;
        this->NSV = svm_model->l;
        this->SV = SVs;
        this->Coef = Coef;
        this->Rho = Rho;
        this->ProbA = ProbA;
        this->ProbB = ProbB;
        this->SVIndices = SVIndices;
        this->Free = svm_model->free_sv;

        svm_free_and_destroy_model(&svm_model);
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
        free(svm_model);
    }

    // LIBSVM for Eps-Support Vector Regression for TFltVV input
    void LibSvmSolveRegression(const TFltVV& VecV, const TFltV& TargetV,
            PNotify DebugNotify, PNotify ErrorNotify) {
      
        // load training parameters
        svm_parameter_t svm_parameter = this->Param.GetParamStruct();

        // Asserts for input arguments
        EAssertR(this->Param.Cost > 0.0, "Cost parameter has to be positive.");

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
        
        TFltVV SVs(svm_model->l, DimN);
        TFltVV Coef(svm_model->nr_class - 1, svm_model->l);
        TFltV Rho(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbA(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TFltV ProbB(svm_model->nr_class * (svm_model->nr_class - 1)/2);
        TIntV SVIndices(svm_model->l);

        // Make sure the WgtV is non-null, i.e., in case w=0 set WgtV to a vector
        // composed of a sufficient number of zeros (e.g. [0, 0, ..., 0]).
        TFltV WgtV(DimN);
        TFlt Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
        EAssertR(TLinAlg::Norm(WgtV) == 0.0, "Expected a zero weight vector.");
        for (int Idx = 0; Idx < svm_model->l; ++Idx) {
            svm_node_t* SV = svm_model->SV[Idx];
            while (SV->index != -1) {
                SVs.PutXY(Idx, SV->index - 1, SV->value);
                WgtV[SV->index - 1] += svm_model->sv_coef[0][Idx] * SV->value;
                ++SV;
            }
            SVIndices.SetVal(Idx, svm_model->sv_indices[Idx]);
            for (int cIdx = 0; cIdx < svm_model->nr_class - 1; cIdx++){
              Coef.PutXY(cIdx, Idx, svm_model->sv_coef[cIdx][Idx]);
            }
        }        
        for (int Idx = 0; Idx < svm_model->nr_class * (svm_model->nr_class - 1)/2; Idx++){
          Rho.SetVal(Idx, svm_model->rho[Idx]);
        }        
        
        this->WgtV = WgtV;
        this->Bias = Bias;
        this->NClass = svm_model->nr_class;
        this->NSV = svm_model->l;
        this->SV = SVs;
        this->Coef = Coef;
        this->Rho = Rho;
        this->ProbA = ProbA;
        this->ProbB = ProbB;
        this->SVIndices = SVIndices;
        this->Free = svm_model->free_sv;

        svm_free_and_destroy_model(&svm_model);
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
    }
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

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TSvm {
  
    svm_model_t* TLibSvmModel::GetModelStruct() const {
        svm_model_t* svm_model = new svm_model_t;
        svm_model->param = Param.GetParamStruct();
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
        /// not needed (and therefore not saved)
        svm_model->free_sv = 0;
        svm_model->probA = NULL;
        svm_model->probB = NULL;
        svm_model->sv_indices = NULL;
        /// classification specific
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
        /// free svm_model->SV
        int DimX = SupportVectors.GetXDim();
        for (int Idx = 0; Idx < DimX; Idx++){
          free(svm_model->SV[Idx]);
          svm_model->SV[Idx] = NULL;
        }
        free(svm_model->SV);
        svm_model->SV = NULL;
        /// free svm_model->sv_coef
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
        /// free svm_model->nSV and svm_model->label if allocated
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
        WgtV = TFltV(Dim);
        Bias = -svm_model->rho[0]; // LIBSVM does w*x-b, while we do w*x+b; thus the sign flip
        SupportVectors = TFltVV(svm_model->l, Dim);
        Coef = TFltVV(svm_model->nr_class - 1, svm_model->l);
        Rho = TFltV(svm_model->nr_class * (svm_model->nr_class - 1)/2);

        /// compute normal vector from support vectors
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
        /// clean up
        svm_free_and_destroy_model(&svm_model);
        free(svm_model);
    }
    
    /// LIBSVM for sparse input
    void TLibSvmModel::LibSvmFit(const TVec<TIntFltKdV>& VecV, const TFltV& TargetV, double Cost, double Unbalance, double Nu, double Eps, double CacheSize, double P,
            const TWPt<TNotify>& DebugNotify, const TWPt<TNotify>& ErrorNotify) {
        
        // load training parameters
        svm_parameter_t svm_parameter = Param.GetParamStruct(Cost, Unbalance, Nu, Eps, CacheSize, P);
                
        // Asserts for input arguments
        EAssertR(Cost > 0.0, "Cost parameter has to be positive.");

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

        /// train the model
        svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);
        
        /// save model and clean up
        ConvertResults(svm_model, Dim);
        
        /// clean up
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
    }
    
    /// Use LIBSVM for dense input
    void TLibSvmModel::LibSvmFit(const TFltVV& VecV, const TFltV& TargetV, double Cost, double Unbalance, double Nu, double Eps, double CacheSize, double P,
            const TWPt<TNotify>& DebugNotify, const TWPt<TNotify>& ErrorNotify) {
      
        // load training parameters
        svm_parameter_t svm_parameter = Param.GetParamStruct(Cost, Unbalance, Nu, Eps, CacheSize, P);
        
        // Asserts for input arguments
        EAssertR(Cost > 0.0, "Cost parameter has to be positive.");

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

        /// train model
        svm_model_t* svm_model = svm_train(&svm_problem, &svm_parameter, DebugNotify, ErrorNotify);
        
        /// save model and clean up
        ConvertResults(svm_model, DimN);
        
        /// clean up
        svm_destroy_param(&svm_parameter);
        free(svm_problem.y);
        free(svm_problem.x);
        free(x_space);
    }

}

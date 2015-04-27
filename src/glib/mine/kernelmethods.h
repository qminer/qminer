/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

//////////////////////////////////////////////////////////////////////////
// Kernel utilities
class TKernelUtil {
public:
    // calculates kernel matrix for Set
    static void CalcKernelMatrix(PSVMTrainSet Set, TFltVV& K);
    // centers kernel matrix ('Kernel methods and P.A.', page 115)
    static void CenterKernelMatrix(TFltVV& K);
};

//////////////////////////////////////////////////////////////////////////
// Partial-Gram-Schmidt (alias Incomplete Cholsky of kernel matrix)
ClassTP(TPartialGS, PPartialGS) //{
public:
    TVec<TFltV> R;  // R from book Kernel Methods, page 126 (if K kernel matrix than K == R'R)
                    // R[i] == i-th row starting with i-th element!
private:
    TIntV IdV;      // IDs (from BigSet) of vectors from which we calculated columns of R
    TFltV NormV;    // norm^2 of q_i before normalized
    TFltV VecNormV; // norm^2 of elements from Set after projection to new basis
public:
    // performes Dim-steps of gram-schmidt on BigSet or until resitual norm is smaller than Eps
    TPartialGS(PSVMTrainSet BigSet, const int& Dim, const double& Eps);
    static PPartialGS New(PSVMTrainSet BigSet, const int& Dim, const double& Eps) {
        return new TPartialGS(BigSet, Dim, Eps); }
    // performes Dim-steps of gram-schmidt on BigSet
    static PPartialGS New(PSVMTrainSet BigSet, const int& Dim) {
        return new TPartialGS(BigSet, Dim, 0.0); }
    // performes  of gram-schmidt on BigSet until residual norm is smaller than Eps
    static PPartialGS New(PSVMTrainSet BigSet, const double& Eps) {
        return new TPartialGS(BigSet, BigSet->Len(), Eps); }

    TPartialGS(TSIn &SIn) {
        R.Load(SIn); IdV.Load(SIn); NormV.Load(SIn); VecNormV.Load(SIn); }
    static PPartialGS Load(TSIn &SIn) { return new TPartialGS(SIn); }
    void Save(TSOut &SOut) {
        R.Save(SOut); IdV.Save(SOut); NormV.Save(SOut); VecNormV.Save(SOut); }

    // returns kernel between i-th and j-th element
    double GetKernel(const int& VecId1, const int& VecId2);
    // number of columns
    int GetCols() const { return IdV.Len(); }
    // number of rows
    int GetRows() const { return R.Len(); }
    // returns row
    const TFltV& GetRow(const int& RowId) { IAssert(RowId < R.Len()); return R[RowId]; }
    // get documents in new basis
    void GetDocVV(TFltVV& DocVV);
    // return basis vector q_n as linear combination of documetns:
    //   q_n = c_1 * x_1 + ... + c_l * x_l, kjer je l == GetRowN()
    void GetBasisV(TVec<TFltV>& q);
    // dot prodoct between ColId-th Col of R and vector w
    double DotProdoctWithCol(const int& ColId, const TFltV& w);

    void GetIdV(TIntV& Out) { Out = IdV; }
    const TIntV& GetIdV() { return IdV; }

    void Dump(const TStr& FName);
};

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

///////////////////////////////////////////////////////////////////////
// BagOfWords-Column-Matrix
class TBowMatrix: public TMatrix {
public:
	TInt RowN;
	TVec<PBowSpV> ColSpVV;
protected:
    // Result = A * B(:,ColId)
    virtual void PMultiply(const TFltVV& B, int ColId, TFltV& Result) const;
    // Result = A * Vec
    virtual void PMultiply(const TFltV& Vec, TFltV& Result) const;
    // Result = A' * B(:,ColId)
    virtual void PMultiplyT(const TFltVV& B, int ColId, TFltV& Result) const;
    // Result = A' * Vec
    virtual void PMultiplyT(const TFltV& Vec, TFltV& Result) const;
	// Result = A * B
	virtual void PMultiply(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO
	// Result = A' * B
	virtual void PMultiplyT(const TFltVV& B, TFltVV& Result) const {FailR("Not implemented yet");} // TODO

    int PGetRows() const { return RowN; }
    int PGetCols() const { return ColSpVV.Len(); }

public:
    TBowMatrix(): RowN(0) { }
	TBowMatrix(const TVec<PBowSpV>& BowSpV);
	TBowMatrix(PBowDocWgtBs BowDocWgtBs);
	TBowMatrix(PBowDocWgtBs BowDocWgtBs, const TIntV& DIdV);
	TBowMatrix(PBowDocBs BowDocBs, PBowDocWgtBs BowDocWgtBs,
		const TStr& CatNm,	const TIntV& DIdV, TFltV& ClsV);
};

///////////////////////////////////////////////////////////////////////
// BagOfWords-Linear-Algebra
class TBowLinAlg {
public:
    // <x,y> where x AND y are sparse
    static double DotProduct(PBowSpV x, PBowSpV y);
    // <x,y> where only y is sparse
    static double DotProduct(const TFltV& x, PBowSpV y);
    // <x,y> where x AND y are sparse
    static double DotProduct(const TIntFltKdV& x, PBowSpV y);

    // y := k * x + y
    static void AddVec(const double& k, PBowSpV x, TFltV& y);
    // z := k * x + y
    static void AddVec(const double& k, PBowSpV x, const TFltV& y, TFltV& z);

    // y = X(:,Docs)' * x
    static void GetDual(const PBowDocWgtBs& X,
        const PBowSpV& x, TFltV& y, const int& Docs = -1);
    // y = X(:,Docs)' * x
    static void GetDual(const PBowDocWgtBs& X,
        const TIntFltKdV& x, TFltV& y, const int& Docs = -1);
    // y = X(:,Docs)' * x
    static void GetDual(const PBowDocWgtBs& X,
        const TFltV& x, TFltV& y, const int& Docs = -1);
};

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

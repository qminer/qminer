/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institut d.o.o.
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

///////////////////////////////////////
// Numeric-Feature-Generator
class TFtrGenNumeric {
private:        
	TBool NormalizeP;
    TFlt MnVal;
    TFlt MxVal;

public:
	TFtrGenNumeric(const bool& _NormalizeP = true): NormalizeP(_NormalizeP), MnVal(TFlt::Mx), MxVal(TFlt::Mn) { }
	TFtrGenNumeric(TSIn& SIn): NormalizeP(SIn), MnVal(SIn), MxVal(SIn) { }
	void Save(TSOut& SOut) const { NormalizeP.Save(SOut); MnVal.Save(SOut); MxVal.Save(SOut); }

    void Update(const double& Val);
	double GetFtr(const double& Val) const;
    void AddFtr(const double& Val, TIntFltKdV& SpV, int& Offset) const;
};

///////////////////////////////////////
// Nominal-Feature-Generator
class TFtrGenNominal {
private:
    TStrH ValH;   

public:
    TFtrGenNominal() { }
	TFtrGenNominal(TSIn& SIn): ValH(SIn) { }
	void Save(TSOut& SOut) const { ValH.Save(SOut);}

    void Update(const TStr& Val);
	int GetFtr(const TStr& Val) const { return ValH.IsKey(Val) ? ValH.GetKeyId(Val) : -1; }
    void AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const;

    int GetVals() const { return ValH.Len(); } 
    TStr GetVal(const int& ValN) const { return ValH.GetKey(ValN); }
};

///////////////////////////////////////
// MultiNomial-Feature-Generator
class TFtrGenMultiNom {
private:
	TFtrGenNominal FtrGen;

public:
	TFtrGenMultiNom() { }
	TFtrGenMultiNom(TSIn& SIn): FtrGen(SIn) { }
	void Save(TSOut& SOut) const { FtrGen.Save(SOut); }

    void Update(const TStr& Str);
    void Update(const TStrV& StrV);
    void AddFtr(const TStr& Str, TIntFltKdV& SpV, int& Offset) const;
    void AddFtr(const TStrV& StrV, TIntFltKdV& SpV, int& Offset) const;

    int GetVals() const { return FtrGen.GetVals(); } 
	TStr GetVal(const int& ValN) const { return FtrGen.GetVal(ValN); }
};

///////////////////////////////////////
// Tokenizable-Feature-Generator
class TFtrGenToken {
private:
    PSwSet SwSet;
    PStemmer Stemmer;
    TInt Docs;
    TStrH TokenH;

public:
    TFtrGenToken(PSwSet _SwSet, PStemmer _Stemmer): SwSet(_SwSet), Stemmer(_Stemmer) { }
	TFtrGenToken(TSIn& SIn);
	void Save(TSOut& SOut) const;

    void Update(const TStr& Val);
    void AddFtr(const TStr& Val, TIntFltKdV& SpV) const;
    void AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const;

    int GetVals() const { return TokenH.Len(); } 
    TStr GetVal(const int& ValN) const { return TokenH.GetKey(ValN); }
    void GetTokenV(const TStr& Str, TStrV& TokenStrV) const;

	PSwSet GetSwSet() const { return SwSet; }
	PStemmer GetStemmer() const { return Stemmer; }
};   

///////////////////////////////////////
// Sparse-Feature-Generator
class TFtrGenSparseNumeric {
private:
    TInt MxId;
	TFtrGenNumeric FtrGen;

public:
	TFtrGenSparseNumeric()  { }
	TFtrGenSparseNumeric(TSIn& SIn): MxId(SIn), FtrGen(SIn) { }
	void Save(TSOut& SOut) const { MxId.Save(SOut); FtrGen.Save(SOut); }

    void Update(const TIntFltKdV& SpV);
    void AddFtr(const TIntFltKdV& InSpV, TIntFltKdV& SpV, int& Offset) const;

    int GetVals() const { return MxId + 1; } 
};

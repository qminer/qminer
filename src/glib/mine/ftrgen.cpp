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
void TFtrGenNumeric::Update(const double& Val) { 
    MnVal = TFlt::GetMn(MnVal, Val); 
    MxVal = TFlt::GetMx(MxVal, Val); 
}

double TFtrGenNumeric::GetFtr(const double& Val) const { 
	if (NormalizeP) {
		return MnVal != MxVal ? (double(Val) - MnVal) / (MxVal - MnVal) : 0.0; 
	}
	return Val;
}

void TFtrGenNumeric::AddFtr(const double& Val, TIntFltKdV& SpV, int& Offset) const {
    SpV.Add(TIntFltKd(Offset, GetFtr(Val))); 
    Offset++; 
}

///////////////////////////////////////
// Nominal-Feature-Generator
void TFtrGenNominal::Update(const TStr& Val) { 
    if (!Val.Empty()) { ValH.AddKey(Val); }
}

void TFtrGenNominal::AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const {
    if (ValH.IsKey(Val)) { SpV.Add(TIntFltKd(Offset + ValH.GetKeyId(Val), 1.0)); } 
    Offset += ValH.Len(); 
}

///////////////////////////////////////
// Multi-Feature-Generator
void TFtrGenMultiNom::Update(const TStr& Str) { 
	FtrGen.Update(Str);
}

void TFtrGenMultiNom::Update(const TStrV& StrV) {
	for (int StrN = 0; StrN < StrV.Len(); StrN++) {
		FtrGen.Update(StrV[StrN]);
	}
}

void TFtrGenMultiNom::AddFtr(const TStr& Str, TIntFltKdV& SpV, int& Offset) const {
	const int FtrId = FtrGen.GetFtr(Str);
	if (FtrId != -1) {
		SpV.Add(TIntFltKd(Offset + FtrId, 1.0));
	}
    Offset += GetVals();
}

void TFtrGenMultiNom::AddFtr(const TStrV& StrV, TIntFltKdV& SpV, int& Offset) const {
	// generate feature vector just for this feature generate
	TIntFltKdV MultiNomSpV(StrV.Len(), 0);
	for (int StrN = 0; StrN < StrV.Len(); StrN++) {
		const int FtrId = FtrGen.GetFtr(StrV[StrN]);
		// only use features we've seen during updates
		if (FtrId != -1) {
			MultiNomSpV.Add(TIntFltKd(Offset + FtrId, 1.0));
		}
	}
	MultiNomSpV.Sort(); 
	// merge elements with same id
	double NormSq = 0.0; int GoodSpN = 0;
	for (int SpN = 1; SpN < MultiNomSpV.Len(); SpN++) {
		if (MultiNomSpV[GoodSpN].Key == MultiNomSpV[SpN].Key) {
			// repeatition of previous id
			MultiNomSpV[GoodSpN].Dat += MultiNomSpV[SpN].Dat; 
		} else { // new id
			// keep track of norm
			NormSq += TMath::Sqr(MultiNomSpV[GoodSpN].Dat);
			// increase the pointer to the next good position
			GoodSpN++;
			// and move the new value down to the good position
			MultiNomSpV[GoodSpN] = MultiNomSpV[SpN]; 
		}
	}
	// only bother if there is something to add
	if (MultiNomSpV.Len() > 0) {
		// update the norm with the last element
		NormSq += TMath::Sqr(MultiNomSpV[GoodSpN].Dat);
		// truncate the vector
		MultiNomSpV.Trunc(GoodSpN+1);
		// normalize
		double Norm = TMath::Sqrt(NormSq);
		TLinAlg::MultiplyScalar(1.0 / Norm, MultiNomSpV, MultiNomSpV); 
		// add the the full feature vector and increase offset count
		SpV.AddV(MultiNomSpV);
	}
	// increase the offset by the dimension
    Offset += GetVals();
}

///////////////////////////////////////
// Tokenizable-Feature-Generator
TFtrGenToken::TFtrGenToken(TSIn& SIn) { 
	SwSet = PSwSet(SIn);
	Stemmer = PStemmer(SIn);
	Docs.Load(SIn);
	TokenH.Load(SIn);
}

void TFtrGenToken::Save(TSOut& SOut) const { 
	SwSet.Save(SOut);
	Stemmer.Save(SOut);
	Docs.Save(SOut);
	TokenH.Save(SOut);
}

void TFtrGenToken::Update(const TStr& Val) {
    TStrV TokenStrV; GetTokenV(Val, TokenStrV); TStrH TokenStrH;
    for (int TokenStrN = 0; TokenStrN < TokenStrV.Len(); TokenStrN++) {
        const TStr& TokenStr = TokenStrV[TokenStrN];
        TokenStrH.AddKey(TokenStr);
    }
    int KeyId = TokenStrH.FFirstKeyId();
    while (TokenStrH.FNextKeyId(KeyId)) {
        const TStr& TokenStr = TokenStrH.GetKey(KeyId);
        TokenH.AddDat(TokenStr)++;
    }
    Docs++;
}

void TFtrGenToken::AddFtr(const TStr& Val, TIntFltKdV& SpV) const {
	int Offset = 0; AddFtr(Val, SpV, Offset);
}

void TFtrGenToken::AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const {
    // step (1): tokenize
    TStrV TokenStrV; GetTokenV(Val, TokenStrV);
    // step (2): aggregate token counts
    TIntH TokenFqH;
    for (int TokenStrN = 0; TokenStrN < TokenStrV.Len(); TokenStrN++) {
        const TStr& TokenStr = TokenStrV[TokenStrN];
        if (TokenH.IsKey(TokenStr)) { 
            const int TokenId = TokenH.GetKeyId(TokenStr);
            TokenFqH.AddDat(TokenId)++;
        }
    }
    // step (3): make a sparse vector out of it
    TIntFltKdV ValSpV(TokenFqH.Len(), 0);
    int KeyId = TokenFqH.FFirstKeyId();
    while (TokenFqH.FNextKeyId(KeyId)) {
        const int TokenId = TokenFqH.GetKey(KeyId);
        const int TokenFq = TokenFqH[KeyId];
        const int TokenDocFq = TokenH[TokenId];
        const double IDF = log(double(Docs) / double(TokenDocFq));
        ValSpV.Add(TIntFltKd(TokenId, double(TokenFq) * IDF));
    }
    ValSpV.Sort(); TLinAlg::Normalize(ValSpV);
    // step (4): add the sparse vector to the final feature vector  
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const int Key = ValSpV[ValSpN].Key + Offset;
        const double Dat = ValSpV[ValSpN].Dat;
        SpV.Add(TIntFltKd(Key, Dat));
    }
    Offset += TokenH.Len(); 
}

void TFtrGenToken::GetTokenV(const TStr& Str, TStrV& TokenStrV) const {
    THtmlLx HtmlLx(TStrIn::New(Str));
    while (HtmlLx.Sym != hsyEof){
        if (HtmlLx.Sym == hsyStr){ 
            TStr TokenStr = HtmlLx.UcChA;
            if (SwSet.Empty() || !SwSet->IsIn(TokenStr)) { 
                if (!Stemmer.Empty()) { 
                    TokenStr = Stemmer->GetStem(TokenStr); } 
                TokenStrV.Add(TokenStr);
            }
        }
        // get next symbol
        HtmlLx.GetSym();
    }
}

///////////////////////////////////////
// Sparse-Numeric-Feature-Generator
void TFtrGenSparseNumeric::Update(const TIntFltKdV& SpV) { 
    for (int SpN = 0; SpN < SpV.Len(); SpN++) {
		MxId = TInt::GetMx(SpV[SpN].Key, MxId);
		FtrGen.Update(SpV[SpN].Dat);
    }
}

void TFtrGenSparseNumeric::AddFtr(const TIntFltKdV& InSpV, TIntFltKdV& SpV, int& Offset) const {
    for (int SpN = 0; SpN < InSpV.Len(); SpN++) {
		const int Id = InSpV[SpN].Key;
		double Val = FtrGen.GetFtr(InSpV[SpN].Dat);
		SpV.Add(TIntFltKd(Offset + Id, Val));
    }
    Offset += GetVals();
}

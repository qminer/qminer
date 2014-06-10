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

namespace TFtrGen {

///////////////////////////////////////
// Numeric-Feature-Generator
void TNumeric::Save(TSOut& SOut) const { 
    SaveEnum<TNumericType>(SOut, Type); 
    MnVal.Save(SOut); MxVal.Save(SOut); 
}
    
void TNumeric::Clr() {
    if (Type == ntNone) {
        MnVal = 0.0; MxVal = 0.0;
    } else if (Type == ntNormalize) {
        MnVal = TFlt::Mx; MxVal = TFlt::Mn;
    } else if (Type == ntMnMxVal) {
        // nothing to do
    }
}

bool TNumeric::Update(const double& Val) {
	if (Type == ntNormalize) {
        MnVal = TFlt::GetMn(MnVal, Val); 
        MxVal = TFlt::GetMx(MxVal, Val);         
    }   
    return false;
}

double TNumeric::GetFtr(const double& Val) const {
	if ((Type != ntNone) && (MnVal < MxVal)) {
        return (Val - MnVal) / (MxVal - MnVal);
	}
    return Val;        
}
    
void TNumeric::AddFtr(const double& Val, TIntFltKdV& SpV, int& Offset) const {
    SpV.Add(TIntFltKd(Offset, GetFtr(Val))); Offset++;
}

void TNumeric::AddFtr(const double& Val, TFltV& FullV, int& Offset) const {
    FullV[Offset] = GetFtr(Val); Offset++;
}

///////////////////////////////////////
// Nominal-Feature-Generator
void TCategorical::Save(TSOut& SOut) const { 
    SaveEnum<TCategoricalType>(SOut, Type);
    ValSet.Save(SOut);
    HashDim.Save(SOut);
}

void TCategorical::Clr() {
    if (Type == ctOpen) { ValSet.Clr(); }
}

bool TCategorical::Update(const TStr& Val) {
    if ((Type == ctOpen) && !Val.Empty()) {
        // check if new value
        const bool NewP = !ValSet.IsKey(Val);
        // remember if new
        if (NewP) { ValSet.AddKey(Val); }
        // return if we increased dimensionality
        return NewP;
    }
    // otherwise we have fixed dimensionality
    return false;
}

int TCategorical::GetFtr(const TStr& Val) const { 
    if (Type == ctHash) {
        // get hash
        return Val.GetPrimHashCd() % HashDim;
    } else {
        // get key id
        return ValSet.IsKey(Val) ? ValSet.GetKeyId(Val) : -1; 
    }
}

void TCategorical::AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const {
    // get dimension to set to 1.0
    const int Dim = GetFtr(Val);
    // set to 1.0 if we get a dimension
    if (Dim != -1) { SpV.Add(TIntFltKd(Offset + Dim, 1.0)); }
    // update offset
    Offset += GetDim();
}

void TCategorical::AddFtr(const TStr& Val, TFltV& FullV, int& Offset) const {
    // get dimension to set to 1.0
    const int Dim = GetFtr(Val);
    // set to 1.0 if we get a dimension
    if (Dim != -1) { FullV[Offset + Dim] = 1.0; }
    // update offset
    Offset += GetDim();
}

///////////////////////////////////////
// Multi-Feature-Generator
void TMultinomial::Save(TSOut& SOut) const { 
    SaveEnum<TMultinomialType>(SOut, Type); 
    FtrGen.Save(SOut); 
}

bool TMultinomial::Update(const TStr& Str) {
    return FtrGen.Update(Str);
}

bool TMultinomial::Update(const TStrV& StrV) {
    bool UpdateP = false;
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        const bool StrUpdateP = FtrGen.Update(StrV[StrN]);
        UpdateP = UpdateP || StrUpdateP;
    }
    return UpdateP;
}

void TMultinomial::AddFtr(const TStr& Str, TIntFltKdV& SpV, int& Offset) const {
    const int FtrId = FtrGen.GetFtr(Str);
    if (FtrId != -1) {
        SpV.Add(TIntFltKd(Offset + FtrId, 1.0));
    }
    Offset += GetDim();
}

void TMultinomial::AddFtr(const TStr& Str, TFltV& FullV, int& Offset) const {
    const int FtrId = FtrGen.GetFtr(Str);
    if (FtrId != -1) {
        FullV[Offset + FtrId] = 1.0;
    }
    Offset += GetDim();    
}

void TMultinomial::AddFtr(const TStrV& StrV, TIntFltKdV& SpV) const {
    // generate internal feature vector
    SpV.Gen(StrV.Len(), 0);
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        const int FtrId = FtrGen.GetFtr(StrV[StrN]);
        // only use features we've seen during updates
        if (FtrId != -1) {
            SpV.Add(TIntFltKd(FtrId, 1.0));
        }
    }
    SpV.Sort();
    // merge elements with the same id
    int GoodSpN = 0;
    for (int SpN = 1; SpN < SpV.Len(); SpN++) {
        if (SpV[GoodSpN].Key == SpV[SpN].Key) {
            // repetition of previous id, sum counts
            SpV[GoodSpN].Dat += SpV[SpN].Dat;
        } else {
            // increase the pointer to the next good position
            GoodSpN++;
            // and move the new value down to the good position
            SpV[GoodSpN] = SpV[SpN];
        }
    }
    // truncate the vector
    SpV.Trunc(GoodSpN + 1);
    // final normalization, if needed
    if (Type == mtNormalize) { TLinAlg::Normalize(SpV); }    
}

void TMultinomial::AddFtr(const TStrV& StrV, TIntFltKdV& SpV, int& Offset) const {
    // generate feature 
    TIntFltKdV ValSpV; AddFtr(StrV, ValSpV);    
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        SpV.Add(TIntFltKd(Offset + ValSp.Key, ValSp.Dat));
    }
    // increase the offset by the dimension
    Offset += GetDim();
}

void TMultinomial::AddFtr(const TStrV& StrV, TFltV& FullV, int& Offset) const {
    // generate feature 
    TIntFltKdV ValSpV; AddFtr(StrV, ValSpV);    
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        FullV[Offset + ValSp.Key] = ValSp.Dat;
    }
    // increase the offset by the dimension
    Offset += GetDim();
}

///////////////////////////////////////
// Tokenizable-Feature-Generator
TBagOfWords::TBagOfWords(const bool& TfP, const bool& IdfP, const bool& NormalizeP, 
        PSwSet _SwSet, PStemmer _Stemmer, const int& _HashDim): SwSet(_SwSet), Stemmer(_Stemmer) { 

    // initialize tokenizer
    Tokenizer = TTokenizerHtmlUnicode::New(SwSet, Stemmer);
    // get settings flags
    Type = 0;
    if (TfP) { Type.Val |= btTf; }
    if (IdfP) { Type.Val |= btIdf; }
    if (NormalizeP) { Type.Val |= btNormalize; }
    // initialize for case of hashing
    if (_HashDim != -1) { 
        // remember the type ...
        Type.Val |= btHashing;
        // .. and the dimension
        HashDim = _HashDim;
        // initialize DF counts for hashes
        DocFqV.Gen(HashDim); DocFqV.PutAll(0);
        OldDocFqV.Gen(HashDim); OldDocFqV.PutAll(0.0);
    }
    
}

TBagOfWords::TBagOfWords(TSIn& SIn): Type(SIn),
    Tokenizer(TTokenizerHtmlUnicode::Load(SIn)), SwSet(SIn), Stemmer(SIn), 
    TokenSet(SIn), HashDim(SIn), Docs(SIn), DocFqV(SIn), ForgetP(SIn),
    OldDocs(SIn), OldDocFqV(SIn) { }

void TBagOfWords::Save(TSOut& SOut) const {
    Type.Save(SOut);
    Tokenizer->Save(SOut);
    SwSet.Save(SOut);
    Stemmer.Save(SOut);
    TokenSet.Save(SOut);
    HashDim.Save(SOut);
    Docs.Save(SOut);
    DocFqV.Save(SOut);
    ForgetP.Save(SOut);
    OldDocs.Save(SOut);
    OldDocFqV.Save(SOut);
}

void TBagOfWords::Clr() {
    Docs = 0; ForgetP = false; OldDocs = 0.0; 
    if (IsHashing()) {
        // if hashing, allocate the document counts and set to zero
        DocFqV.Gen(HashDim); DocFqV.PutAll(0);
        OldDocFqV.Gen(HashDim); OldDocFqV.PutAll(0.0);
    } else {
        // if normal vector space, just forget the existing tokens and document counts
        TokenSet.Clr(); DocFqV.Clr(); OldDocFqV.Clr();
    }
}

bool TBagOfWords::Update(const TStr& Val) {    
    // tokenize given text
    TStrV TokenStrV; GetFtr(Val, TokenStrV);
    // process tokens to update DF counts
    bool UpdateP = false;
    if (IsHashing()) {  
        // consolidate tokens and get their hashed IDs
        TIntSet TokenIdH;
        for (int TokenStrN = 0; TokenStrN < TokenStrV.Len(); TokenStrN++) {
            const TStr& TokenStr = TokenStrV[TokenStrN];
            const int TokenId = TokenStr.GetPrimHashCd() % HashDim;
            TokenIdH.AddKey(TokenId);
        }
        // update document counts
        int KeyId = TokenIdH.FFirstKeyId();
        while (TokenIdH.FNextKeyId(KeyId)) {
            const int TokenId = TokenIdH.GetKey(KeyId);
            // update DF
            DocFqV[TokenId]++;
        }
    } else {
        // consolidate tokens
        TStrH TokenStrH;
        for (int TokenStrN = 0; TokenStrN < TokenStrV.Len(); TokenStrN++) {
            const TStr& TokenStr = TokenStrV[TokenStrN];
            TokenStrH.AddKey(TokenStr);
        }
        // update document counts and update vocabulary with new tokens
        int KeyId = TokenStrH.FFirstKeyId();
        while (TokenStrH.FNextKeyId(KeyId)) {
            // get token
            const TStr& TokenStr = TokenStrH.GetKey(KeyId);
            // different processing for hashing
            int TokenId = TokenSet.GetKeyId(TokenStr);
            if (TokenId == -1) {
                // new token, remember the dimensionality change
                UpdateP = true;
                // remember the new token
                TokenId = TokenSet.AddKey(TokenStr);
                // increase document count table
                const int TokenDfId = DocFqV.Add(0);
                // increase also the old count table
                OldDocFqV.Add(0.0);
                // make sure we DF vector and TokenSet still in sync
                IAssert(TokenId == TokenDfId);
                IAssert(DocFqV.Len() == OldDocFqV.Len());
            }
            // document count update
            DocFqV[TokenId]++;
        }
    }
    // update document count
    Docs++;
    // tell if dimension changed
    return UpdateP;
}

void TBagOfWords::GetFtr(const TStr& Str, TStrV& TokenStrV) const {
    // outsource to tokenizer
    Tokenizer->GetTokens(Str, TokenStrV);
}

void TBagOfWords::AddFtr(const TStr& Val, TIntFltKdV& SpV) const {
    // tokenize
    TStrV TokenStrV; GetFtr(Val, TokenStrV);
    // aggregate token counts
    TIntH TermFqH;
    for (int TokenStrN = 0; TokenStrN < TokenStrV.Len(); TokenStrN++) {
        const TStr& TokenStr = TokenStrV[TokenStrN];
        // get token ID
        const int TokenId = IsHashing() ?
            (TokenStr.GetPrimHashCd() % HashDim) : // hashing
            TokenSet.GetKeyId(TokenStr); // vocabulary
        // add if known token
        if (TokenId != -1) {
            TermFqH.AddDat(TokenId)++;
        }
    }
    // make a sparse vector out of it
    SpV.Gen(TermFqH.Len(), 0);
    int KeyId = TermFqH.FFirstKeyId();
    while (TermFqH.FNextKeyId(KeyId)) {
        const int TermId = TermFqH.GetKey(KeyId);
        double TermVal = 1.0;
        if (IsTf()) { TermVal *= double(TermFqH[KeyId]); }
        if (IsIdf()) {
            if (ForgetP) {
                const double DocFq = double(DocFqV[TermId]) + OldDocFqV[TermId];
                if (DocFq > 0.1) { TermVal *= log((double(Docs) + OldDocs) / DocFq); }
            } else {
                TermVal *= log(double(Docs) / double(DocFqV[TermId]));
            }
        }
        SpV.Add(TIntFltKd(TermId, TermVal));
    }
    SpV.Sort();
    // step (4): normalize the vector if so required
    if (IsNormalize()) { TLinAlg::Normalize(SpV); }
}

void TBagOfWords::AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const {
    TIntFltKdV ValSpV; AddFtr(Val, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        SpV.Add(TIntFltKd(Offset + ValSp.Key, ValSp.Dat));
    }    
    // increase the offset by the dimension
    Offset += GetDim();
}

void TBagOfWords::AddFtr(const TStr& Val, TFltV& FullV, int& Offset) const {
    TIntFltKdV ValSpV; AddFtr(Val, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        FullV[Offset + ValSp.Key] = ValSp.Dat;
    }
    // increase the offset by the dimension
    Offset += GetDim();    
}

void TBagOfWords::Forget(const double& Factor) {
    // remember we started forgeting
    ForgetP = true;
    // update old document count
    OldDocs = double(Docs) + Factor * OldDocs;    
    // reset current counts
    Docs = 0;
    // do same for document frequencies
    const int Dims = GetDim();
    for (int Dim = 0; Dim < Dims; Dim++) {
        // update old document frequency
        OldDocFqV[Dim] = double(DocFqV[Dim]) + Factor * OldDocFqV[Dim];
        // reset current count
        DocFqV[Dim] = 0;
    }
}

///////////////////////////////////////
// Sparse-Numeric-Feature-Generator

void TSparseNumeric::Update(const TIntFltKdV& SpV) {
    for (int SpN = 0; SpN < SpV.Len(); SpN++) {
        MxId = TInt::GetMx(SpV[SpN].Key, MxId);
        FtrGen.Update(SpV[SpN].Dat);
    }
}

void TSparseNumeric::AddFtr(const TIntFltKdV& InSpV, TIntFltKdV& SpV, int& Offset) const {
    for (int SpN = 0; SpN < InSpV.Len(); SpN++) {
        const int Id = InSpV[SpN].Key;
        double Val = FtrGen.GetFtr(InSpV[SpN].Dat);
        SpV.Add(TIntFltKd(Offset + Id, Val));
    }
    Offset += GetVals();
}

}
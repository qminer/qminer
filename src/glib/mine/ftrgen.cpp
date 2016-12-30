/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TFtrGen {

///////////////////////////////////////
// Numeric-Feature-Generator
void TNumeric::Save(TSOut& SOut) const { 
    SaveEnum<TNumericType>(SOut, Type); 
    MnVal.Save(SOut); MxVal.Save(SOut); 
	Var.Save(SOut);
}
    
void TNumeric::Clr() {
    if (Type == ntNone) {
        MnVal = 0.0; MxVal = 0.0;
    } else if (Type == ntNormalize) {
        MnVal = TFlt::Mx; MxVal = TFlt::Mn;
	} else if (Type == ntNormalizeVar) {
		Var.Clr();
	} else if (Type == ntMnMxVal) {
        // nothing to do
    }
}

bool TNumeric::Update(const double& Val) {
	if (Type == ntNormalize) {
        MnVal = TFlt::GetMn(MnVal, Val); 
        MxVal = TFlt::GetMx(MxVal, Val);         
	}
	else if (Type == ntNormalizeVar) {
		Var.Update(Val);
	}
    return false;
}

double TNumeric::GetFtr(const double& Val) const {
	if (Type == ntNormalizeVar) {
		double Res = Val - Var.GetMean();
		double M2 = Var.GetStDev();
		if (M2 > 0) Res /= M2;
		return Res;
	} else if ((Type != ntNone) && (MnVal < MxVal)) {
		if (Val > MxVal) { 
			return 1; 
		} else if (Val < MnVal) { 
			return 0; 
		} else {
			return (Val - MnVal) / (MxVal - MnVal);
		}
	}
    return Val;        
}
    
void TNumeric::AddFtr(const double& Val, TIntFltKdV& SpV, int& Offset) const {
    SpV.Add(TIntFltKd(Offset, GetFtr(Val))); Offset++;
}

void TNumeric::AddFtr(const double& Val, TFltV& FullV, int& Offset) const {
    FullV[Offset] = GetFtr(Val); Offset++;
}

double TNumeric::InvFtr(const double& FtrVal) const {
	double InvVal = FtrVal;
	if (Type == ntNormalizeVar) {
		double M2 = Var.GetStDev();
		if (M2 > 0) InvVal *= M2;
		InvVal += Var.GetMean();
	} else if (Type != ntNone && MnVal < MxVal) {
		InvVal =  InvVal*(MxVal - MnVal) + MnVal;
	}
	return InvVal;
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

TStr TCategorical::GetVal(const int& ValN) const {
	return (Type == ctHash) ? TInt::GetStr(ValN) : ValSet.GetKey(ValN);
}

///////////////////////////////////////
// Multi-Feature-Generator
void TMultinomial::Init(const bool& NormalizeP, const bool& BinaryP, const bool& LogP) {
    if (NormalizeP) { Flags.Val |= mtNormalize; }
    if (BinaryP) {
        Flags.Val |= mtBinary;
        EAssertR(!IsLog(), "TMultinomial: flags 'binary' and 'log' are exclusive!");
    }
    if (LogP) {
        Flags.Val |= mtLog;
        EAssertR(!IsBinary(), "TMultinomial: flags 'binary' and 'log' are exclusive!");
    }
}

TMultinomial::TMultinomial(const bool& NormalizeP, const bool& BinaryP, const bool& LogP):
    Flags(0), FtrGen() { Init(NormalizeP, BinaryP, LogP); }

TMultinomial::TMultinomial(const bool& NormalizeP, const bool& BinaryP, const bool& LogP,
        const TStrV& ValV):
    Flags(0), FtrGen(ValV) {Init(NormalizeP, BinaryP, LogP); }

TMultinomial::TMultinomial(const bool& NormalizeP, const bool& BinaryP, const bool& LogP,
        const int& HashDim):
    Flags(0), FtrGen(HashDim) { Init(NormalizeP, BinaryP, LogP); }

void TMultinomial::Save(TSOut& SOut) const { 
    Flags.Save(SOut);
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

void TMultinomial::AddFtr(const TStrV& StrV, const TFltV& FltV, TIntFltKdV& SpV) const {
    // make sure we either do not have explicit values, or their dimension matches with string keys
    EAssertR(FltV.Empty() || (StrV.Len() == FltV.Len()), "TMultinomial::AddFtr:: String and double values not aligned");
    // generate internal feature vector
    SpV.Gen(StrV.Len(), 0);
    for (int StrN = 0; StrN < StrV.Len(); StrN++) {
        const int FtrId = FtrGen.GetFtr(StrV[StrN]);
        // only use features we've seen during updates
        if (FtrId != -1) {
            const double Val = FltV.Empty() ? 1.0 : FltV[StrN].Val;
            if (Val > 1e-16) { SpV.Add(TIntFltKd(FtrId, Val)); }
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
    // replace values with 1 if needed
    if (IsBinary()) { for (TIntFltKd& Sp : SpV) { Sp.Dat = 1.0; } }
    // compute the logarithm if needed
    if (IsLog()) { for (TIntFltKd& Sp : SpV) { Sp.Dat = TMath::Log(Sp.Dat + 1); } }
    // final normalization, if needed
    if (IsNormalize()) { TLinAlg::Normalize(SpV); }    
}

void TMultinomial::AddFtr(const TStrV& StrV, const TFltV& FltV, TIntFltKdV& SpV, int& Offset) const {
    // generate feature 
    TIntFltKdV ValSpV; AddFtr(StrV, FltV, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        SpV.Add(TIntFltKd(Offset + ValSp.Key, ValSp.Dat));
    }
    // increase the offset by the dimension
    Offset += GetDim();
}

void TMultinomial::AddFtr(const TStrV& StrV, const TFltV& FltV, TFltV& FullV, int& Offset) const {
    // generate feature 
    TIntFltKdV ValSpV; AddFtr(StrV, FltV, ValSpV);
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
        PTokenizer _Tokenizer, const int& _HashDim, const bool& StoreHashWordsP,
        const int& _NStart, const int& _NEnd): Tokenizer(_Tokenizer) {

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
        // keep hash table?
        if (StoreHashWordsP) { Type.Val |= btStoreHashWords; }
        // initialize it if true
        if (IsStoreHashWords()) { HashWordV.Gen(HashDim); }
        // initialize DF counts for hashes
        DocFqV.Gen(HashDim); DocFqV.PutAll(0);
        OldDocFqV.Gen(HashDim); OldDocFqV.PutAll(0.0);
    }
    NStart = _NStart;
    NEnd = _NEnd;
}

TBagOfWords::TBagOfWords(TSIn& SIn): Type(SIn),
    Tokenizer(TTokenizer::Load(SIn)), TokenSet(SIn), HashDim(SIn), NStart(SIn), NEnd(SIn),
    Docs(SIn), DocFqV(SIn), ForgetP(SIn), OldDocs(SIn), OldDocFqV(SIn), HashWordV(SIn) { }

void TBagOfWords::Save(TSOut& SOut) const {
    Type.Save(SOut);
    Tokenizer->Save(SOut);
    TokenSet.Save(SOut);
    HashDim.Save(SOut);
    NStart.Save(SOut);
    NEnd.Save(SOut);
    Docs.Save(SOut);
    DocFqV.Save(SOut);
    ForgetP.Save(SOut);
    OldDocs.Save(SOut);
    OldDocFqV.Save(SOut);
    HashWordV.Save(SOut);
}

void TBagOfWords::Clr() {
    Docs = 0; ForgetP = false; OldDocs = 0.0; 
    if (IsHashing()) {
        // if hashing, allocate the document counts and set to zero
        DocFqV.Gen(HashDim); DocFqV.PutAll(0);
        OldDocFqV.Gen(HashDim); OldDocFqV.PutAll(0.0);
        if (IsStoreHashWords()) { HashWordV.Clr(); }
    } else {
        // if normal vector space, just forget the existing tokens and document counts
        TokenSet.Clr(); DocFqV.Clr(); OldDocFqV.Clr();
    }
}

void TBagOfWords::GetFtr(const TStr& Str, TStrV& TokenStrV) const {
    // outsource to tokenizer
    EAssertR(!Tokenizer.Empty(), "Missing tokenizer in TFtrGen::TBagOfWords");
    Tokenizer->GetTokens(Str, TokenStrV);
    // counting average token length
    /*static int Count = 0, LenStr = 0, LenVec = 0;
    Count++; LenStr += Str.Len(); LenVec += TokenStrV.Len();
    if (Count % 1000 == 0) { 
        printf("Average token length[docs=%d chars=%d words=%d length=%.4f\n", 
            Count, LenStr, LenVec, (double)LenStr / (double)LenVec);
    }*/
}

void TBagOfWords::GenerateNgrams(const TStrV& TokenStrV, TStrV &NgramStrV) const {    
	if((NStart == 1) && (NEnd == 1)) { 
        NgramStrV = TokenStrV;
		return;
    }    
    const int TotalStrLen = TokenStrV.Len();
    for(int TokenStrN = 0; TokenStrN < TotalStrLen; TokenStrN++) { // for each token position, generate ngrams starting at that position
    	// Start with Token Position
    	// End with Token Position + NEnd - 1 because ngram parameters are 1-based indexes and vectors are 0-based indexes
       const int NgramEnd = MIN(TotalStrLen - 1, TokenStrN + NEnd - 1) + 1;
        for (int NgramPos = TokenStrN + (NStart - 1); NgramPos < NgramEnd; NgramPos++) {
            TChA NgramChA = TokenStrV[TokenStrN];
            for (int NgramTokenN = TokenStrN + 1; NgramTokenN <= NgramPos; NgramTokenN++) {
                NgramChA += " "; NgramChA += TokenStrV[NgramTokenN];
            }
            NgramStrV.Add(NgramChA);
        }
    }
}

bool TBagOfWords::Update(const TStrV& TokenStrV) {    
    // Generate Ngrams if necessary
	TStrV NgramStrV;
    GenerateNgrams(TokenStrV, NgramStrV);

    // process tokens to update DF counts
    bool UpdateP = false;
    if (IsHashing()) {  
        // consolidate tokens and get their hashed IDs
        TIntSet TokenIdH;
        for (int TokenStrN = 0; TokenStrN < NgramStrV.Len(); TokenStrN++) {
            const TStr& TokenStr = NgramStrV[TokenStrN];
            TInt TokenId = TokenStr.GetHashTrick() % HashDim;
            TokenIdH.AddKey(TokenId);
            if (IsStoreHashWords()) { HashWordV[TokenId].AddKey(TokenStr); }
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
        for (int TokenStrN = 0; TokenStrN < NgramStrV.Len(); TokenStrN++) {
            const TStr& TokenStr = NgramStrV[TokenStrN];
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

bool TBagOfWords::Update(const TStr& Val) {    
    // tokenize given text (reserve space assuming 5 chars per word)    
    TStrV TokenStrV(Val.Len() / 5, 0); GetFtr(Val, TokenStrV);
    // process
    return Update(TokenStrV);
}

void TBagOfWords::AddFtr(const TStrV& TokenStrV, TIntFltKdV& SpV) const {
    // aggregate token counts
    TIntH TermFqH;
	TStrV NgramStrV;
    GenerateNgrams(TokenStrV, NgramStrV);	
    for (int TokenStrN = 0; TokenStrN < NgramStrV.Len(); TokenStrN++) {
        const TStr& TokenStr = NgramStrV[TokenStrN];
        // get token ID
        const int TokenId = IsHashing() ?
            (TokenStr.GetHashTrick() % HashDim) : // hashing
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

void TBagOfWords::AddFtr(const TStr& Val, TIntFltKdV& SpV) const {
    // tokenize
    TStrV TokenStrV(Val.Len() / 5, 0); GetFtr(Val, TokenStrV);
    // create sparse vector
    AddFtr(TokenStrV, SpV);
}

void TBagOfWords::AddFtr(const TStrV& TokenStrV, TIntFltKdV& SpV, int& Offset) const {
	// create sparse vector
    TIntFltKdV ValSpV; AddFtr(TokenStrV, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        SpV.Add(TIntFltKd(Offset + ValSp.Key, ValSp.Dat));
    }    
    // increase the offset by the dimension
    Offset += GetDim();
}

void TBagOfWords::AddFtr(const TStr& Val, TIntFltKdV& SpV, int& Offset) const {
    // tokenize
    TStrV TokenStrV(Val.Len() / 5, 0); GetFtr(Val, TokenStrV);
    // create sparse vector
    TIntFltKdV ValSpV; AddFtr(TokenStrV, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        SpV.Add(TIntFltKd(Offset + ValSp.Key, ValSp.Dat));
    }    
    // increase the offset by the dimension
    Offset += GetDim();
}

void TBagOfWords::AddFtr(const TStrV& TokenStrV, TFltV& FullV, int& Offset) const {
    // create sparse vector
    TIntFltKdV ValSpV; AddFtr(TokenStrV, ValSpV);
    // add to the full feature vector and increase offset count
    for (int ValSpN = 0; ValSpN < ValSpV.Len(); ValSpN++) {
        const TIntFltKd& ValSp = ValSpV[ValSpN];
        FullV[Offset + ValSp.Key] = ValSp.Dat;
    }
    // increase the offset by the dimension
    Offset += GetDim();    
}

void TBagOfWords::AddFtr(const TStr& Val, TFltV& FullV, int& Offset) const {
    // tokenize
    TStrV TokenStrV(Val.Len() / 5, 0); GetFtr(Val, TokenStrV);
    // create sparse vector
    TIntFltKdV ValSpV; AddFtr(TokenStrV, ValSpV);
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

///////////////////////////////////////
// Date window feature generator
void TDateWnd::InitWgt() {
    if (NormalizeP) {
       Wgt = 1.0 / TMath::Sqrt((double)WndSize); 
    } else {
       Wgt = 1.0;
    }
}

TDateWnd::TDateWnd(const int& _WndSize, const TTmUnit& _TmUnit, 
    const bool& _NormalizeP): InitP(false), WndSize(_WndSize), 
        TmUnit(_TmUnit), NormalizeP(_NormalizeP) {
    
    EAssert(WndSize > 0);  
    // initialize feature vector weight
    InitWgt();
}


TDateWnd::TDateWnd(const TTm& StartTm, const TTm& EndTm, const int& _WndSize, 
        const TTmUnit& _TmUnit, const bool& _NormalizeP): InitP(true),
            WndSize(_WndSize), TmUnit(_TmUnit), NormalizeP(_NormalizeP) {

    EAssert(WndSize > 0);
    // use borders to construct time boundaries
    Update(StartTm);
    Update(EndTm);
    // initialize feature vector weight
    InitWgt();
}

void TDateWnd::Save(TSOut& SOut) const {
    InitP.Save(SOut);
    StartUnit.Save(SOut);
    EndUnit.Save(SOut);
    WndSize.Save(SOut);
    SaveEnum<TTmUnit>(SOut, TmUnit);
    NormalizeP.Save(SOut);
}
    
bool TDateWnd::Update(const TTm& Val) {
    // if first time, use it to initialize
    if (!InitP) {
        StartUnit = TSecTm(Val).GetInUnits(TmUnit);
        EndUnit = TSecTm(Val).GetInUnits(TmUnit);
        InitP = true;
        return true;
    }
    // check if we moved start or end boundary
    const uint ValUnit = TSecTm(Val).GetInUnits(TmUnit);
    if (StartUnit > ValUnit) {
        StartUnit = ValUnit; return true;
    }
    if (EndUnit < ValUnit) {
        EndUnit = ValUnit; return true;
    }
    // nope, we are fine
    return false;
}

int TDateWnd::GetFtr(const TTm& Val) const {
    EAssert(InitP);
    const uint ValUnit = TSecTm(Val).GetInUnits(TmUnit);
    if (ValUnit < StartUnit) { return 0; }
    if (ValUnit > EndUnit) { return (int)(EndUnit - StartUnit); }
    return (int)(ValUnit - StartUnit);
}

void TDateWnd::AddFtr(const TTm& Val, TIntFltKdV& SpV, int& Offset) const {
    const int Ftr = GetFtr(Val);
    for (int FtrN = 0; FtrN < WndSize; FtrN++) {
        SpV.Add(TIntFltKd(Offset + Ftr + FtrN, Wgt));
    }
    Offset += GetDim();
}

void TDateWnd::AddFtr(const TTm& Val, TFltV& FullV, int& Offset) const {
    const int Ftr = GetFtr(Val);
    for (int FtrN = 0; FtrN < WndSize; FtrN++) {
        FullV[Offset + Ftr + FtrN] = Wgt;
    }    
    Offset += GetDim();
}

}

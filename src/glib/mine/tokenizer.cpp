/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

///////////////////////////////
// Tokenizer
TFunRouter<TTokenizer::TNewF> TTokenizer::NewRouter;
TFunRouter<TTokenizer::TLoadF> TTokenizer::LoadRouter;

bool TTokenizer::Init() {
    Register<TTokenizers::TSimple>();
    Register<TTokenizers::THtml>();
    Register<TTokenizers::THtmlUnicode>();
    return true;
}

bool TTokenizer::RegP = Init();

PTokenizer TTokenizer::New(const TStr& TypeNm, const PJsonVal& JsonVal) {
    return NewRouter.Fun(TypeNm)(JsonVal);
}

PTokenizer TTokenizer::Load(TSIn& SIn) {
	TStr TypeNm(SIn);          
    return LoadRouter.Fun(TypeNm)(SIn);
}

void TTokenizer::GetTokens(const TStr& Text, TStrV& TokenV) const {
	PSIn SIn = TStrIn::New(Text, false);
	GetTokens(SIn, TokenV);
}

void TTokenizer::GetTokens(const TStrV& TextV, TVec<TStrV>& TokenVV) const {
	IAssert(TextV.Len() == TokenVV.Len()); // shall we rather say Tokens.Gen(Texts.Len(), 0); ?
	for (int TextN = 0; TextN < TextV.Len(); TextN++) {
		TStrV& TokenV = TokenVV[TextN];
		TokenVV.Gen(32,0); // assume there will be at least 32 tokens, to avoid small resizes
		GetTokens(TextV[TextN], TokenV);
	}
}

namespace TTokenizers { 
    
///////////////////////////////
// Tokenizer-Simple
PTokenizer TSimple::New(const PJsonVal& ParamVal) {
    // get stopwords
    PSwSet SwSet = ParamVal->IsObjKey("stopwords") ? 
        TSwSet::ParseJson(ParamVal->GetObjKey("stopwords")) :
        TSwSet::New(swstNone);   
    // get stemmer
    PStemmer Stemmer = ParamVal->IsObjKey("stemmer") ? 
        TStemmer::ParseJson(ParamVal->GetObjKey("stemmer"), false) :
        TStemmer::New(stmtNone, false);
    const bool ToUcP = ParamVal->GetObjBool("uppercase", true);
    return new TSimple(SwSet, Stemmer, ToUcP);
}

TSimple::TSimple(TSIn& SIn): SwSet(SIn), Stemmer(SIn), ToUcP(SIn) { }

void TSimple::Save(TSOut& SOut) const { 
    GetType().Save(SOut);
    SwSet.Save(SOut); Stemmer.Save(SOut); ToUcP.Save(SOut); 
}

void TSimple::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
	TStr LineStr; TStrV WordStrV;
	while (SIn->GetNextLn(LineStr)) {
		WordStrV.Clr(false);
		LineStr.SplitOnAllAnyCh(" .,!?\n\r()+=-{}[]%$#@\\/", WordStrV, true);
		for (int WordStrN = 0; WordStrN < WordStrV.Len(); WordStrN++) {
			const TStr& WordStr = WordStrV[WordStrN];
			const TStr UcStr = WordStr.GetUc();
			if (SwSet.Empty() || (!SwSet->IsIn(UcStr))) {
				TStr TokenStr = ToUcP ? UcStr : WordStr;
				if (!Stemmer.Empty()) {	
					TokenStr = Stemmer->GetStem(TokenStr,ToUcP); }
				TokenV.Add(TokenStr);
			}
		}
	}
}

///////////////////////////////
// Tokenizer-Html
THtml::THtml(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP): 
        SwSet(_SwSet), Stemmer(_Stemmer), ToUcP(_ToUcP) { }

PTokenizer THtml::New(const PJsonVal& ParamVal) {
    // get stopwords
    PSwSet SwSet = ParamVal->IsObjKey("stopwords") ? 
        TSwSet::ParseJson(ParamVal->GetObjKey("stopwords")) :
        TSwSet::New(swstNone);   
    // get stemmer
    PStemmer Stemmer = ParamVal->IsObjKey("stemmer") ? 
        TStemmer::ParseJson(ParamVal->GetObjKey("stemmer"), false) :
        TStemmer::New(stmtNone, false);
    const bool ToUcP = ParamVal->GetObjBool("uppercase", true);
    return new THtml(SwSet, Stemmer, ToUcP);
}

THtml::THtml(TSIn& SIn): SwSet(SIn), Stemmer(SIn), ToUcP(SIn) { }

void THtml::Save(TSOut& SOut, const bool& SaveTypeP) const {
    if (SaveTypeP) { GetType().Save(SOut); }
	SwSet.Save(SOut); Stemmer.Save(SOut); ToUcP.Save(SOut);  
}

void THtml::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
	THtmlLx HtmlLx(SIn, false);
    // traverse html string symbols
	while (HtmlLx.Sym!=hsyEof){
		if (HtmlLx.Sym==hsyStr){
			TStr UcStr = HtmlLx.UcChA;
			// check if stop word
			if ((SwSet.Empty()) || (!SwSet->IsIn(UcStr))) {
				TStr TokenStr = ToUcP ? UcStr : TStr(HtmlLx.ChA);
				if (!Stemmer.Empty()) { 
					TokenStr = Stemmer->GetStem(TokenStr); }
				TokenV.Add(TokenStr.GetLc());
			}
		}
		// get next symbol
		HtmlLx.GetSym();
	}
}

///////////////////////////////
// Tokenizer-Html-Unicode
THtmlUnicode::THtmlUnicode(const PSwSet& _SwSet, const PStemmer& _Stemmer, 
        const bool& _ToUcP): THtml(_SwSet, _Stemmer, _ToUcP) {
        
    EAssertR(TUnicodeDef::IsDef(), "Unicode not initialized!");
}

PTokenizer THtmlUnicode::New(const PJsonVal& ParamVal) {
    // get stopwords
    PSwSet SwSet = ParamVal->IsObjKey("stopwords") ? 
        TSwSet::ParseJson(ParamVal->GetObjKey("stopwords")) :
        TSwSet::New(swstNone);   
    // get stemmer
    PStemmer Stemmer = ParamVal->IsObjKey("stemmer") ? 
        TStemmer::ParseJson(ParamVal->GetObjKey("stemmer"), false) :
        TStemmer::New(stmtNone, false);
    const bool ToUcP = ParamVal->GetObjBool("uppercase", true);
    return new THtmlUnicode(SwSet, Stemmer, ToUcP);
}

void THtmlUnicode::Save(TSOut& SOut) const { 
    GetType().Save(SOut);
    THtml::Save(SOut,false);
}

void THtmlUnicode::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
	TStr LineStr; TStrV WordStrV;    
	while (SIn->GetNextLn(LineStr)) {
        TStr SimpleText = TUStr(LineStr).GetStarterLowerCaseStr();
        THtml::GetTokens(TStrIn::New(SimpleText, false), TokenV);
	}
}

}

///////////////////////////////
// Tokenizer-Utils
void TTokenizerUtil::Sentencize(const PSIn& SIn, TStrV& Sentences, const bool& SplitNewLineP) {
	TChA SentenceBuf;
	int c;
	while (!SIn->Eof()) {
		c = SIn->GetCh();
		switch (c) {
			case '\r':
			case '\n':	{
				if (!SplitNewLineP) {
					SentenceBuf += ' ';
					break;
				}
			}
			case '"' :
			case '.' :
			case '!' :
			case ':' :
			case ';' :
			case '?' :
			case '\t': {
				if (SentenceBuf.Len() > 2) {
					Sentences.Add(SentenceBuf);
					SentenceBuf.Clr();
				}
				break;
			}
			default: 
				SentenceBuf += c;
				break;
		}
	}
	if (SentenceBuf.Len() > 0) {
		Sentences.Add(SentenceBuf);
	}	
}

void TTokenizerUtil::Sentencize(const TStr& Text, TStrV& Sentences, const bool& SplitNewLineP) {
	PSIn StrIn = TStrIn::New(Text, false);
	Sentencize(StrIn, Sentences, SplitNewLineP);
}

void TTokenizerUtil::Paragraphize(const PSIn& SIn, TStrV& Paragraphs) {
	TChA ParagraphBuf;
	int c;
	bool wasSpace = false;
	while (!SIn->Eof()) {
		c = SIn->GetCh();
		// two consecutive spaces signal a new paragraph
		if (c == ' ' || c == '\t' || c == '\n') {
			if (wasSpace) {
				Paragraphs.Add(ParagraphBuf);
				ParagraphBuf.Clr();
				continue;
			}
			wasSpace = true;
		} else {
			wasSpace = false;
		}
		ParagraphBuf += c;
	}
	if (ParagraphBuf.Len() > 0) {
		Paragraphs.Add(ParagraphBuf);
	}
}

void TTokenizerUtil::Paragraphize(const TStr& Text, TStrV& Paragraphs) {
	PSIn StrIn = TStrIn::New(Text, false);
	Paragraphize(StrIn, Paragraphs);
}

void TTokenizerUtil::ToSequences(const TStrV& TokenIds, TVec<TStrV >& TupleList, int minN, int maxN) {
	for (int j = minN; j <= TMath::Mn(maxN, TokenIds.Len()); j++) {
		// Convert to n-grams
		ToSequencesForLength(TokenIds, TupleList, j);
	}
}

void TTokenizerUtil::ToSequencesForLength(const TStrV& TokenIds, TVec<TStrV >& TupleList, int n) {
	for (int i = 0; i < TokenIds.Len() - n + 1; i++) {
		TStrV& Tuple = TupleList[TupleList.Add()];
		for (int j = 0; j < n; j++) {
			Tuple.Add(TokenIds[i + j]);
		}
	}
}

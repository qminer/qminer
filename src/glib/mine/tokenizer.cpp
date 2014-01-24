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

///////////////////////////////
// Tokenizer
void TTokenizer::GetTokens(const TStr& Text, TStrV& TokenV) const {
	PSIn SIn = TStrIn::New(Text);
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

///////////////////////////////
// Tokenizer-Simple
void TTokenizerSimple::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
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
					TokenStr = Stemmer->GetStem(TokenStr); }
				TokenV.Add(TokenStr);
			}
		}
	}
}

///////////////////////////////
// Tokenizer-Html
TTokenizerHtml::TTokenizerHtml(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP,
		const int& MxNGramLen, const int& _MnNGramFq, const int& MxNGramCache): SwSet(_SwSet), 
			Stemmer(_Stemmer), ToUcP(_ToUcP), NGramBs(NULL), MnNGramFq(_MnNGramFq) {  
	
	if (MxNGramLen > 1) { NGramBs = TStreamNGramBs::New(MxNGramLen, MxNGramCache); }
}

TTokenizerHtml::TTokenizerHtml(TSIn& SIn): SwSet(SIn), Stemmer(SIn),
	ToUcP(SIn), NGramBs(SIn), MnNGramFq(SIn) { }

void TTokenizerHtml::Save(TSOut& SOut) const { 
	SwSet.Save(SOut); Stemmer.Save(SOut); ToUcP.Save(SOut);  
	NGramBs.Save(SOut); MnNGramFq.Save(SOut);
}

void TTokenizerHtml::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
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

 //   // tokenize
 //   TStrV TokenStrV; Tokenizer->GetTokens(TextStr, TokenStrV);
 //   // transform words to IDs
 //   const int Tokens = TokenStrV.Len();
 //   TIntV TokenIdV(Tokens, 0);
 //   for (int TokenN = 0; TokenN < Tokens; TokenN++) {
 //       // add token to the hashtable of all tokens
 //       const int TokenId = WordH.AddKey(TokenStrV[TokenN].GetUc());
 //       // keep track of it's count
 //       WordH[TokenId]++;
 //       // and prepare a token vector for ngram base
 //       TokenIdV.Add(TokenId);
 //   }
	//// extract the n-grams
 //   TNGramDescV NGramDescV;
	//NGramBs->AddDocTokIdV(TokenIdV, StoreThreshold, NGramDescV);
 //   // get string representations of n-grams above threshold
 //   TStrH NGramH;
 //   for (int NGramDescN = 0; NGramDescN < NGramDescV.Len(); NGramDescN++) {
 //       const TNGramDesc& NGramDesc = NGramDescV[NGramDescN];
 //       // make it into a string
 //       const TIntV& NGramTokenIdV = NGramDesc.TokIdV;
 //       TChA NGramChA = WordH.GetKey(NGramTokenIdV[0]);
 //       for (int NGramTokenIdN = 1; NGramTokenIdN < NGramTokenIdV.Len(); NGramTokenIdN++) {
 //           NGramChA += ' '; NGramChA += WordH.GetKey(NGramTokenIdV[NGramTokenIdN]);
 //       }
 //       // remember the ngram, if not stopword
 //       if (!SwSet->IsIn(NGramChA)) { NGramH.AddDat(NGramChA); }
 //   }
 //   // remember n-grams above threshold
 //   int NGramKeyId = NGramH.FFirstKeyId();
 //   while (NGramH.FNextKeyId(NGramKeyId)) {
 //       const TStr& NGramStr = NGramH.GetKey(NGramKeyId);
 //       // add to the result list
 //       ConceptV.Add(TOgNewsConcept(NGramStr, EmtpyStr));
 //   }
}

///////////////////////////////
// Tokenizer-Html-Unicode
void TTokenizerHtmlUnicode::GetTokens(const PSIn& SIn, TStrV& TokenV) const {
	TStr LineStr; TStrV WordStrV;
	while (SIn->GetNextLn(LineStr)) {
		TStr SimpleText = TUStr(LineStr).GetStarterLowerCaseStr();
		TTokenizerHtml::GetTokens(TStrIn::New(SimpleText), TokenV);
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
					printf("%s\n", SentenceBuf.CStr());
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
	PSIn StrIn = TStrIn::New(Text);
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
	PSIn StrIn = TStrIn::New(Text);
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

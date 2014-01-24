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

///////////////////////////////
// Tokenizer
ClassTP(TTokenizer, PTokenizer) // {
public:
	TTokenizer() { }
	virtual ~TTokenizer() { }

	virtual void Save(TSOut& SOut) const = 0;

	virtual void GetTokens(const PSIn& SIn, TStrV& TokenV) const = 0;
	void GetTokens(const TStr& Text, TStrV& TokenV) const;
	void GetTokens(const TStrV& TextV, TVec<TStrV>& TokenVV) const;
};

///////////////////////////////
// Tokenizer-Simple
//   Simple whitespace & punctuation tokenizer. 
//   No stopwords or stemming. Fast, threadsafe.
class TTokenizerSimple : public TTokenizer {
protected:
	PSwSet SwSet;
	PStemmer Stemmer;
	TBool ToUcP;
public:
	TTokenizerSimple(const PSwSet& _SwSet = NULL, const PStemmer& _Stemmer = NULL, 
		const bool& _ToUcP = true): SwSet(_SwSet), Stemmer(_Stemmer), ToUcP(_ToUcP) {  }
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL, bool ToUcP = true) {
		return new TTokenizerSimple(SwSet, Stemmer, ToUcP); }

	TTokenizerSimple(TSIn& SIn): SwSet(SIn), Stemmer(SIn), ToUcP(SIn) { }
	static PTokenizer Load(TSIn& SIn) { return new TTokenizerSimple(SIn); }
	void Save(TSOut& SOut) const { SwSet.Save(SOut); Stemmer.Save(SOut); ToUcP.Save(SOut); }

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
};

///////////////////////////////
// Tokenizer-Html
//   HTML-aware tough tokenizer with stopwords and stemming.
//   WARNING - NOT THREAD SAFE. WILL SEGFAULT
class TTokenizerHtml : public TTokenizer {
protected:
	PSwSet SwSet;
	PStemmer Stemmer;
	TBool ToUcP;
	// for handling n-grams
	PStreamNGramBs NGramBs;
	TInt MnNGramFq;
	
	TTokenizerHtml(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP,
		const int& MxNGramLen, const int& MnNGramFq, const int& MxNGramCache);
public:
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL, bool ToUcP = true,
		const int& MxNGramLen = 1, const int& MnNGramFq = 5, const int& MxNGramCache = 100000000) {
			return new TTokenizerHtml(SwSet, Stemmer, ToUcP, MxNGramLen, MnNGramFq, MxNGramCache); }

	// serialization
	TTokenizerHtml(TSIn& SIn);
	static PTokenizer Load(TSIn& SIn) { 
		return new TTokenizerHtml(SIn); }
	void Save(TSOut& SOut) const;

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
};

///////////////////////////////
// Tokenizer-Html-Unicode
//   Puts string to simple canoniocal form and calls HTML tokenizer, 
class TTokenizerHtmlUnicode : public TTokenizerHtml {
protected:
	TTokenizerHtmlUnicode(const PSwSet& _SwSet, const PStemmer& _Stemmer,  const bool& _ToUcP, 
		const int& MxNGramLen, const int& _MnNGramFq, const int& MxNGramCache): 
			TTokenizerHtml(_SwSet, _Stemmer, _ToUcP, MxNGramLen, _MnNGramFq, MxNGramCache) {
				EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); }
public:
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL, bool ToUcP = true,
		const int& MxNGramLen = 1, const int& MnNGramFq = 5, const int& MxNGramCache = 100000000) {
			return new TTokenizerHtmlUnicode(SwSet, Stemmer, ToUcP, MxNGramLen, MnNGramFq, MxNGramCache); }

	TTokenizerHtmlUnicode(TSIn& SIn): TTokenizerHtml(SIn) { 
		EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); }
	static PTokenizer Load(TSIn& SIn) { return new TTokenizerHtmlUnicode(SIn); }
	void Save(TSOut& SOut) const { TTokenizerHtml::Save(SOut); }

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
};

///////////////////////////////
// Tokenizer-Utils
class TTokenizerUtil {
public:
	// Sentence splitter
	static void Sentencize(const PSIn& SIn, TStrV& Sentences, const bool& SplitNewLineP = true);
	static void Sentencize(const TStr& Text, TStrV& Sentences, const bool& SplitNewLineP = true);

	// Paragraph splitter
	static void Paragraphize(const PSIn& SIn, TStrV& Paragraphs);
	static void Paragraphize(const TStr& Text, TStrV& Paragraphs);

	// Generate all possible subsequences within given length constraints
	static void ToSequences(const TStrV& TokenIds, TVec<TStrV >& TupleList, int minN, int maxN);
	// Generate all possible subsequences of length n
	static void ToSequencesForLength(const TStrV& TokenIds, TVec<TStrV >& TupleList, int n);
};

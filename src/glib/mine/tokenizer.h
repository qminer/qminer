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
    /// Getr 
///////////////////////////////
/// Tokenizer.
class TTokenizer; typedef TPt<TTokenizer> PTokenizer;
class TTokenizer {
private: 
	// smart-pointer
	TCRef CRef;
	friend class TPt<TTokenizer>;

	/// New constructor delegate
	typedef PTokenizer (*TNewF)(const PJsonVal& JsonVal);
	typedef PTokenizer (*TLoadF)(TSIn& SIn);
    /// Stream aggregate descriptions
	static TFunRouter<PTokenizer, TNewF> NewRouter;   
	static TFunRouter<PTokenizer, TLoadF> LoadRouter;   
public:
    /// Register default aggregates
    static bool Init();
    static bool RegP;
    /// Register new aggregate
    template <class TObj> static void Register() { 
        NewRouter.Register(TObj::GetType(), TObj::New);
        LoadRouter.Register(TObj::GetType(), TObj::Load);
    }

public:
	TTokenizer() { }
    static PTokenizer New(const TStr& TypeNm, const PJsonVal& JsonVal);
	virtual ~TTokenizer() { }

	virtual void Save(TSOut& SOut) const = 0;
    static PTokenizer Load(TSIn& SIn);

	virtual void GetTokens(const PSIn& SIn, TStrV& TokenV) const = 0;
	void GetTokens(const TStr& Text, TStrV& TokenV) const;
	void GetTokens(const TStrV& TextV, TVec<TStrV>& TokenVV) const;
};

namespace TTokenizers {
    
///////////////////////////////
// Tokenizer-Simple
//   Simple whitespace & punctuation tokenizer. 
class TSimple : public TTokenizer {
protected:
	PSwSet SwSet;
	PStemmer Stemmer;
	TBool ToUcP;

	TSimple(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP): 
        SwSet(_SwSet), Stemmer(_Stemmer), ToUcP(_ToUcP) {  }
public:
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL, 
        bool ToUcP = true) { return new TSimple(SwSet, Stemmer, ToUcP); }
    static PTokenizer New(const PJsonVal& ParamVal);
    
	TSimple(TSIn& SIn);
	static PTokenizer Load(TSIn& SIn) { return new TSimple(SIn); }
	void Save(TSOut& SOut) const;

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
    
    static TStr GetType() { return "simple"; }
};

///////////////////////////////
// Tokenizer-Html
//   HTML-aware tough tokenizer with stopwords and stemming.
//   WARNING - NOT THREAD SAFE. WILL SEGFAULT
class THtml : public TTokenizer {
protected:
	PSwSet SwSet;
	PStemmer Stemmer;
	TBool ToUcP;
	
	THtml(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP);
public:
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL,
        bool ToUcP = true) { return new THtml(SwSet, Stemmer, ToUcP); }
    static PTokenizer New(const PJsonVal& ParamVal);

	// serialization
	THtml(TSIn& SIn);
	static PTokenizer Load(TSIn& SIn) { return new THtml(SIn); }
	void Save(TSOut& SOut, const bool& SaveTypeP) const;
    void Save(TSOut& SOut) const { Save(SOut, true); }

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
    
    static TStr GetType() { return "html"; }
};

///////////////////////////////
// Tokenizer-Html-Unicode
//   Puts string to simple canonical form and calls HTML tokenizer, 
class THtmlUnicode : public THtml {
protected:
	THtmlUnicode(const PSwSet& _SwSet, const PStemmer& _Stemmer, const bool& _ToUcP);
public:
	static PTokenizer New(PSwSet SwSet = NULL, PStemmer Stemmer = NULL, 
        bool ToUcP = true) { return new THtmlUnicode(SwSet, Stemmer, ToUcP); }
    static PTokenizer New(const PJsonVal& ParamVal);

	THtmlUnicode(TSIn& SIn): THtml(SIn) {
        EAssertR(TUnicodeDef::IsDef(), "Unicode not initilaized!"); }
	static PTokenizer Load(TSIn& SIn) { return new THtmlUnicode(SIn); }
	void Save(TSOut& SOut) const;

	void GetTokens(const PSIn& SIn, TStrV& TokenV) const;
    
    static TStr GetType() { return "unicode"; }    
};

}

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

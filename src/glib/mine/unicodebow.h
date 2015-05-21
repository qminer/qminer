/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef UNICODETEXTTOKENIZER_H
#define UNICODETEXTTOKENIZER_H

#include "base.h"
#include "mine.h"

// Unicode vector space model
namespace TUnicodeVSM {

	//Custom Hash Function
	//Simple rolling hash implemented for TUStr
	class THASHFUNC{
	public:
		static inline int GetPrimHashCd(const TUStr&  Key) { return Key.GetPrimHashCd(); }
		static inline int GetSecHashCd(const  TUStr&  Key) { return Key.GetSecHashCd(); }
	};
	typedef THash<TUStr, TInt, THASHFUNC> TUStrIntH; //typedef THash<TUStr, TInt, THASHFUNC>
	typedef enum {
		tWord, tWordNgram, tCharNgram, tSimpleCharNgram, tSimpleWordNgram
	} TBowOptTag;
	//Currently only Glib based Tokenizer is implemented
	//ICU Tokenizer is almost done
	typedef enum {
		tGlibTok, tICUTok
	} TBowTokTag;
	typedef enum {
		tZeroBased, tOneBased
	} TIndexing;
	//Abstract class the enables skipping of certain characters
	class TFilter{
	public:
		bool IsNotValid(int unicode_char){
			return false;
		};
	};
	//Filter that skips all numeric characters
	class TFilterNum : public TFilter{
	public:
		virtual bool IsNotValid(int unicode_char){
			return TUStr::IsNumeric(unicode_char);
		};
	};
	class TUBow;
	typedef TPt<TUBow> PUBow;
	class TUBow{
	private:
		TCRef CRef;
	public:
		friend class TPt<TUBow>;
	private:
		//Lang of the text
		TStr Lang;
		//CharNgrams, WordNgrams, Words see TBowOptTag
		TInt Option;
		//Minimal and Maximal Length of the Ngrams to be generated
		TInt MinLen;
		TInt MaxLen;
	public:
        virtual ~TUBow() { }
		virtual TUStrV TokenizeWords(TStr Text, TFilter Filter = TFilterNum()){ return TUStrV(); };
		virtual TVec<TIntKd>  TextToVec(TUStr& Text){ return TVec<TIntKd>(); };
		virtual void TextToVec(TUStr& Text, TPair<TIntV, TFltV>& SparseVec){};
		virtual void TextToVec(TUStrV& Docs, TTriple<TIntV, TIntV, TFltV>& DocMatrix){};
	};
		
	class TGlibUBow;
	typedef TPt<TGlibUBow> PGlibUBow;
	class TGlibUBow : public TUBow{
	private:
		TCRef CRef;
	public:
		friend class TPt<TGlibUBow>;
	private:
		TInt Option;
		//Lang of the text
		TStr Lang;
		TBool stemmer_supported;
		//Stemmer if supported
		SStemmer Stemmer;
		//Potential stop words
		TSwSet StopSet;
		//Maximal Index of the word - number of words
		TInt MaxIndex;
		//Do you want to skip numbers?
		TBool skip_numbers;
		//Do you want to remove punctuations in preprocessing step?
		TBool remove_punct;
		//Ngrams of  min_len <= length <= max_len
		TInt min_len;
		TInt max_len;
		//Special new line char - only used when parsing documents
		TInt new_line_char;
		//Do you want to break Ngram generation on stop words?
		TBool BrOnSt;
		//Hash Map of Words as TUStr Vectors
		TUStrIntH WordIds;
		//Hash Map of Word Ngrams as Int Vectors
		TUStrIntH WordNgrams;
		//TIntVIntH WordNgrams;
		//Hash Map of Character Ngrams as Int Vectors
		TUStrIntH Ngrams;
		//Bow-Doc matrix
		TVec<TVec<TIntKd> > Matrix;
		TIntV InvDoc;
		TInt NDocs;
		TFlt TFidf;

		//CurrentDocVector
		//TVec<TIntKd> Vector;
	public:
		//Default choice is '\n', be careful some WikipediaLineDocs use '\r'
		TGlibUBow(){ new_line_char = '\n';};
		static PGlibUBow New() {return PGlibUBow(new TGlibUBow);}		
		TGlibUBow(TStr Lang, TInt Option, bool skip_numbers = true, bool remove_punct = false, bool enable_stemming = false);
		TGlibUBow(TStr Lang, TInt Option, TInt min_len, TInt max_len, bool skip_numbers = true, bool remove_punct = false, bool enable_stemming = false);
		//TGlibTokenizer(const TUStrIntH & WordIds, const TUStrIntH & WordNgrams, const TUStrIntH & Ngrams);
		//Default choice is '\n', be careful some WikipediaLineDocs use '\r'
		void processWikipediaLineDocGlib(const TStr& ime, TInt newline = '\n', TBool AddToMatrix=true);
		void processFixedWikipediaLineDocGlib(const TStr& ime, TInt new_line_char);
		TBool IsStemmerSupported(){ return stemmer_supported; }
		TInt  GetNumberOfWords()  { return WordIds.Len(); }
		TUStr  GetWord(int WordIndex)  { Assert(WordIndex < WordIds.Len() && WordIndex >= 0);  return  WordIds.GetKey(WordIndex); }
		TUStr  GetToken(int TokenIndex);
		int  GetNumberOfTokens();
		int  GetNumberOfDocs()  { return  (int)MAX(Matrix.Len(), NDocs.Val); }
		void SaveOldBin(TSOut& SOut) const;
		void SaveBin(TSOut& SOut) const;
		//Simple export to text format, outputn directory must exist!
		void Print(const TStr& Directory="./outputn") const;
		void LoadOldBin(TSIn& SIn);
		void LoadBin(TSIn& SIn);
		void DelMatrix();
		//Set Stop Words from File
		void SetStop(TStr File, TBool BrOnSt=false);
		void ComputeDocFreq(TIntV &InvDoc, TInt& NDocs);
		void ComputeDocFreq(){ ComputeDocFreq(this->InvDoc, this->NDocs); }
	public:
		virtual TVec<TIntKd>  TextToVec(TUStr& Text);
		virtual void TextToVec(TUStr& Text, TPair<TIntV, TFltV>& SparseVec);
		virtual void TextToVec(TUStrV& Docs, TTriple<TIntV, TIntV, TFltV>& DocMatrix);
		virtual void TextToVec(TUStrV& Docs, TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TFltV& invdoc);
	//This two functions do essentialy the same
	//AddTokenizeWords could and should use TokenizeWords instead of resuing the code
	//Code will be refactored on first occasion
	public:
		TVec<TIntKd> _TokenizeWords(TUStr& Text);
		TVec<TIntKd> AddTokenizeWords(TUStr& Text, TBool AddToMatrix = true, TBool UpdateVoc = true);
	public:
		TVec<TIntKd>     TokenizeWordNgrams(TUStr& Text);
		TVec<TIntKd>     AddTokenizeWordNgrams(TUStr& Text, TBool AddToMatrix = true, TBool UpdateVoc = true);
	public:
		TVec<TIntKd> AddTokenizeNgrams(TUStr& Text, TBool AddToMatrix = true, TBool UpdateVoc = true);
		//Avoid global variables
		TVec<TIntKd> AddTokenizeSimpleNgrams(TUStr& Text, TBool LowerCase = true, TBool AddToMatrix = true, TBool UpdateVoc = true);
		TVec<TIntKd> TokenizeNgrams(TUStr& Text);
		TVec<TIntKd> TokenizeSimpleNgrams(TUStr& Text);
	public:
		//Word Tokenization as preprocessing step for Word Tokens
		void TokenizeWordsPreprocess(TUStr& Text, TUStrV& Words, TBoolV& Seperators);
		void TokenizeWordsPreprocess(TUStr& Text, TLst<TUStr> & Words, TLst<TBool> & Seperators, TBool cut = false);
		/*Tokenize for TStr and TUStr*/
		//Shift = 1 One Based Indexing (Matlab indices) to Zero Based Indexing
		//Shift = 0 Zero Based Indexing 
		void CompactVocabulary(TIntV& WordIndex, TInt Shift = 1);
		void BuildVocabulary(TStr& File){ processWikipediaLineDocGlib(File, '\r', false); };
		void ExportMatrix(TTriple<TIntV, TIntV, TFltV>& Mat);
		void AddTokenize(TUStr& Text, TBool AddToMatrix = true, TBool UpdateVoc = true);
		void Tokenize(TUStr& Text);
		void GetInvDoc(TIntV& InvDoc){ InvDoc = this->InvDoc;}
		void GetInvDoc(TIntV& InvDoc, TInt& NDocs){ InvDoc = this->InvDoc; NDocs = this->NDocs; }
		const TIntV& GetInvDoc(){ return this->InvDoc; }
		void CompactMatrix(TIntV& DocIndex);
	};

			/*Tokenize for TStr and TUStr*/



}
#endif
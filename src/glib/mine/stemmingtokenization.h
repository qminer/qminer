#ifndef STEMMING_TOKENIZATION_H
#define STEMMING_TOKENIZATION_H
#include "base.h"

// To use the stemmer, add SNOWBALL_STEMMER to preprocessor and add these files to project: stem*, utilities.c, api.c, libstemmer.c
//#define SNOWBALL_STEMMER
// To use the advanced tokenizer, add CHINESE_TOKENIZATION to preprocessor and include ictclas50.lib in linker input, you also need ictclas50.dll in system path
//#define CHINESE_TOKENIZATION

#ifdef SNOWBALL_STEMMER
#include "libstemmer.h"
class TSStemmer{
	private:
			struct sb_stemmer* stemmer;
			TBool supported;
		    TStr Name;
	public:
		TSStemmer& operator=(const TSStemmer& Stemmer){
			this->stemmer = Stemmer.stemmer;
			this->Name    = Stemmer.Name;
			return *this;
		}
		bool SetStemmer(const TStr& Name);
		TStr Stem(const TStr& word);
		/*Stem in place*/
		void Stem(TStr& word); 
		void Stem(TUStr& word); 
		TStrV GetSupportedAsStrV();
		bool IsSupported(){return supported;}
};
#else//Dummy compilation

class TSStemmer{
private:
	struct sb_stemmer* stemmer;
	TBool supported;
	TStr Name;
public:
	TSStemmer& operator=(const TSStemmer& Stemmer){
		this->stemmer = Stemmer.stemmer;
		this->Name    = Stemmer.Name;
		return *this;
	}
	bool setStemmer(const TStr& Name){ return false; };
	TStr Stem(const TStr& word){ return word; };
	/*Stem in place*/
	void Stem(TStr& word){};
	void Stem(TUStr& word){};
	TStrV GetSupportedAsStrV(){ return TStrV(); }
	bool IsSupported(){ return supported; }
};

#endif

#ifdef CHINESE_TOKENIZATION
#include "ChineseStemmer.h"
class TChineseTokenizer{
	public:
		static bool Init(TStr dir);
		static bool Init();
		static int Tokenize(const TStr & Paragraph, TStr & Tokenized);//Spaces seperate the tokens
		static int CleanNumbersEnglish(const TStr & input, TUStrV & result);
		static int TokenizeClean(const TStr & input, TUStrV & result);
		static int CleanNumbersEnglish(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate);
		static int TokenizeClean(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate);
};
#else//Dummy Compilation
class TChineseTokenizer{
public:
	static bool Init(TStr dir){ return false; };
	static bool Init(){ return false; };
	static int Tokenize(const TStr & Paragraph, TStr & Tokenized){ Tokenized = Paragraph; return 0; }//Spaces seperate the tokens
	static int CleanNumbersEnglish(const TStr & input, TUStrV & result){ return 0; };
	static int TokenizeClean(const TStr & input, TUStrV & result){ TUStr UInput(input); UInput.GetWordUStrV(result); return result.Len(); };
	static int CleanNumbersEnglish(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate){ return 0; };
	static int TokenizeClean(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate){ return 0; };
};

#endif

#endif
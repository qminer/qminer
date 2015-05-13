#ifndef STEMMING_TOKENIZATION_H
#define STEMMING_TOKENIZATION_H
#include "base.h"

// To use the stemmer, add SNOWBALL_STEMMER to preprocessor and add these files to project: stem*, utilities.c, api.c, libstemmer.c
//#define SNOWBALL_STEMMER
// To use the advanced tokenizer, add CHINESE_TOKENIZATION to preprocessor and include ictclas50.lib in linker input, you also need ictclas50.dll in system path
//#define CHINESE_TOKENIZATION

#ifdef SNOWBALL_STEMMER
#include "libstemmer.h"
class SStemmer{
	private:
			struct sb_stemmer* stemmer;
			TBool supported;
		    TStr Name;
	public:
		//~SStemmer(){sb_stemmer_delete(stemmer);}
		SStemmer& operator=(const SStemmer& Stemmer){
			this->stemmer = Stemmer.stemmer;
			this->Name    = Stemmer.Name;
			return *this;
		}
		bool setStemmer(const TStr& Name);
		TStr stem(const TStr& word);
		/*Stem in place*/
		void stem(TStr& word); 
		void stem(TUStr& word); 
		TStrV getSupportedAsStrV();
		bool isSupported(){return supported;}
};
#else//Dummy compilation

class SStemmer{
private:
	struct sb_stemmer* stemmer;
	TBool supported;
	TStr Name;
public:
	//~SStemmer(){sb_stemmer_delete(stemmer);}
	SStemmer& operator=(const SStemmer& Stemmer){
		this->stemmer = Stemmer.stemmer;
		this->Name    = Stemmer.Name;
		return *this;
	}
	bool setStemmer(const TStr& Name){ return false; };
	TStr stem(const TStr& word){ return word; };
	/*Stem in place*/
	void stem(TStr& word){};
	void stem(TUStr& word){};
	TStrV getSupportedAsStrV(){ return TStrV(); }
	bool isSupported(){ return supported; }
};

#endif

#ifdef CHINESE_TOKENIZATION
#include "ChineseStemmer.h"
class ChineseTokenizer{
	public:
		static bool init(TStr dir);
		static bool init();
		static int tokenize(const TStr & Paragraph, TStr & Tokenized);//Spaces seperate the tokens
		static int cleanNumbersEnglish(const TStr & input, TUStrV & result);
		static int tokenizeClean(const TStr & input, TUStrV & result);
		static int cleanNumbersEnglish(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate);
		static int tokenizeClean(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate);
};
#else//Dummy Compilation
class ChineseTokenizer{
public:
	static bool init(TStr dir){ return false; };
	static bool init(){ return false; };
	static int tokenize(const TStr & Paragraph, TStr & Tokenized){ Tokenized = Paragraph; return 0; }//Spaces seperate the tokens
	static int cleanNumbersEnglish(const TStr & input, TUStrV & result){ return 0; };
	static int tokenizeClean(const TStr & input, TUStrV & result){ TUStr UInput(input); UInput.GetWordUStrV(result); return result.Len(); };
	static int cleanNumbersEnglish(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate){ return 0; };
	static int tokenizeClean(const TStr & input, TLst<TUStr> & result, TLst<TBool> & separate){ return 0; };
};

#endif

#endif
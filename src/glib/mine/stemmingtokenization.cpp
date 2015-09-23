/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "stemmingtokenization.h"
//#include <omp.h>

#ifdef SNOWBALL_STEMMER
bool TSStemmer::SetStemmer(const TStr& Name){
    this->Name = Name; 
    this->stemmer  = sb_stemmer_new(Name.CStr(), "UTF_8"); 
    this->supported =  (this->stemmer != NULL); 
    return supported;
}

TStr TSStemmer::Stem(const TStr& word){ 
	if(!supported){
		return word;
	}
	TStr Ret;
#pragma omp critical
	{
		const sb_symbol * stemmed_word = sb_stemmer_stem(this->stemmer, (sb_symbol*)word.CStr(), word.Len());
		Ret =  TStr((const char *)stemmed_word);
	}
	return Ret;
	
}

/*Stem in place.  
  Argument is not modified if stemmer is not supported
*/
void TSStemmer::Stem(TStr& word){ 
	if(!supported){
		return;
	}
#pragma omp critical
	{
		const sb_symbol * stemmed_word = sb_stemmer_stem(this->stemmer, (sb_symbol*)word.CStr(), word.Len());
		word = TStr((const char *)stemmed_word);
	}
}
//Conversion to Unicode and Back -- cannot be avoided
void TSStemmer::Stem(TUStr& word){ 
	if(!supported){
		return;
	}
#pragma omp critical
	{
		const sb_symbol * stemmed_word = sb_stemmer_stem(this->stemmer, (sb_symbol*)word.GetStr().CStr(), word.Len());
		word = TUStr((const char *)stemmed_word);
	}
}
TStrV TSStemmer::GetSupportedAsStrV(){
	const char **z =  sb_stemmer_list();
	TStrV List;
	while(*z){
		List.Add(TStr(*z));
		//std::cout << *z <<std::endl;
		z++;
	}
	return List;
}

#endif

#ifdef CHINESE_TOKENIZATION
#include <iostream>
bool TChineseTokenizer::Init(TStr dir){ return ICTCLAS_Init(dir.CStr());}
bool TChineseTokenizer::Init(){ return ICTCLAS_Init("./StemmingTokenization/");}
int TChineseTokenizer::Tokenize(const TStr & Paragraph, TStr & Tokenized){//Spaces seperate the tokens
	//FILE *file = fopen("report.txt", "a");
    if(Paragraph.Empty()) return 0;
	int n = Paragraph.Len(); char * result = (char *)malloc(n*10);
	int tokens = ICTCLAS_ParagraphProcess(Paragraph.CStr(), n, result, CODE_TYPE_UTF8, false);
	Tokenized = TStr(result);
	//std::cout << Tokenized.CStr() << "\n";
	//fprintf(file, "%d %d %d %s\n", 6*n, tokens, Tokenized.Len(), Tokenized.CStr());
	//fclose(file);
	//std::cout << Tokenized.CStr();
	free(result);
	//std::cout << n << " strlen " << strlen(Paragraph.CStr()) << " / " << 10*n << " Tokens " << tokens << " " << Tokenized.Len() << "\n";
	return tokens;
}
int TChineseTokenizer::CleanNumbersEnglish(const TStr & input, TUStrV & result){
	TStrV tokens;
	input.SplitOnAllAnyCh(" ", tokens);//][!\"#$%&'()*+,./:;<=>?@\\^_`{|}~-
	int num_words = 0;
	for(int i = 0; i < tokens.Len(); i++){
		TStr word = tokens.GetVal(i);
		TUStr uword = word;
		if(uword.Empty()) continue;
		//Get rid of English words, commas and parsing errors
		bool number = false;
		//number = word.IsInt();
		//word.GetCh(0);
		//if(!word.IsWord() || number){//Chinese letters are not ascii

		if(TUStr::IsAlphabetic(uword[0]) ){//The first letter is alphabetic
				if(TCh::IsAlpha(word.GetCh(0))){//regular english word skip //Do we allow that or not????
					continue;
				}
				num_words ++;
				result.Add(word);
				//fprintf(f, "%s\t%d\n", word.CStr(), TUStr(word)[0]);
			}
		//}
	}
	return num_words;
}

int TChineseTokenizer::CleanNumbersEnglish(const TStr & input, TLst<TUStr> & result, TLst<TBool> & seperate){
	TStrV tokens;
	input.SplitOnAllAnyCh(" ", tokens);//][!\"#$%&'()*+,./:;<=>?@\\^_`{|}~-
	int num_words = 0;
	for(int i = 0; i < tokens.Len(); i++){
		TStr word = tokens.GetVal(i);
		TUStr uword = word;
		if(uword.Empty()) continue;
		//Get rid of English words, commas and parsing errors
		bool number = false;
		//number = word.IsInt();
		//word.GetCh(0);
		//if(!word.IsWord() || number){//Chinese letters are not ascii

		if(TUStr::IsAlphabetic(uword[0]) ){//The first letter is alphabetic
				if(TCh::IsAlpha(word.GetCh(0))){//regular english word skip //Do we allow that or not????
					continue;
				}
				num_words ++;
				result.AddBack(word);
				seperate.AddBack(false);
				//fprintf(f, "%s\t%d\n", word.CStr(), TUStr(word)[0]);
			}
		//}
	}
	return num_words;
}

int TChineseTokenizer::TokenizeClean(const TStr & input, TUStrV & result){
	TStr temp;
	Tokenize(input, temp);
	//std::cout << "Tokenize Clean Start\n";
	int tokens = CleanNumbersEnglish(temp, result);
	//std::cout << "Tokenize Clean Success\n";
	return tokens;
}

int TChineseTokenizer::TokenizeClean(const TStr & input, TLst<TUStr> & result, TLst<TBool> & seperate){
	TStr temp;
	Tokenize(input, temp);
	//std::cout << "Tokenize Clean Start\n";
	int tokens = CleanNumbersEnglish(temp, result, seperate);
	//std::cout << "Tokenize Clean Success\n";
	return tokens;
}

#endif
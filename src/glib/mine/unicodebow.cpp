/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "unicodebow.h"
#include <iostream> 

namespace TUnicodeVSM {
	int64 loadToCBuffer(FILE** id, char *buffer, int64 dol = 134217728, char newline = '\n'){
		int64 last_new_line = 0;
		int64 i; char c;
		int64 vel = fread(buffer, sizeof(char), dol, *id);
		if (vel == 0) return -1;
		for (i = vel - 1; i >= 0; i--){
			c = buffer[i];
			if (c == newline){
				last_new_line = i;
				break;
			}
		}
		buffer[last_new_line + 1] = '\0';
		//fseeko for linux
		//refactoring with ifdef for linux
#if defined(GLib_WIN)
		_fseeki64(*id, last_new_line - vel + 1, SEEK_CUR);
#elif defined(GLib_MACOSX)
        fseeko(*id, last_new_line - vel + 1, SEEK_CUR);
#else
		fseeko64(*id, last_new_line - vel + 1, SEEK_CUR);
#endif
		return last_new_line + 1;
	}
	TGlibUBow::TGlibUBow(TStr Lang, TInt Option, bool skip_numbers, bool remove_punct, bool enable_stemming){
		this->Lang = Lang;	this->Option = Option;
		if (enable_stemming){
			if (Lang == "zh"){//Special Tokenization for Chinese
				printf("Chinese Tokenizer!\n");
				this->stemmer_supported = ChineseTokenizer::init();
				if (this->stemmer_supported){
					printf("Chinese Dll OK!\n");
				}
				else{
					printf("Chinese Dll not found in the path!\n");
				}
			}
			else{//Snowball Stemmers
				this->stemmer_supported = Stemmer.setStemmer(Lang);
				if (this->stemmer_supported){
					TStr Message = ("Creation Done SnowBall stemmer for" + Lang + " !");
					printf("%s\n", Message.CStr());
				}
				else{
					TStr Message = ("Stemmer for " + Lang + " not supported!");
					printf("%s\n", Message.CStr());
				}
			}
		}
		else{
			this->stemmer_supported = false;
		}
		this->skip_numbers = skip_numbers;
		this->remove_punct = remove_punct;
	};
	TGlibUBow::TGlibUBow(TStr Lang, TInt Option, TInt min_len, TInt max_len, bool skip_numbers, bool remove_punct, bool enable_stemming) {
		this->Lang = Lang;	this->Option = Option;
		if (enable_stemming){
			if (Lang == "zh"){//Special Tokenization for Chinese
				printf("Chinese Tokenizer!\n");
				this->stemmer_supported = ChineseTokenizer::init();
				if (this->stemmer_supported){
					printf("Chinese Dll OK!\n");
				}
				else{
					printf("Chinese Dll not found in the path!\n");
				}
			}
			else{//Snowball Stemmers
				this->stemmer_supported = Stemmer.setStemmer(Lang);
				if (this->stemmer_supported){
					TStr Message = ("Creation Done SnowBall stemmer for" + Lang + " !");
					printf("%s\n", Message.CStr());
				}
				else{
					TStr Message = ("Stemmer for " + Lang + " not supported!");
					printf("%s\n", Message.CStr());
				}
			}
		}
		else{
			this->stemmer_supported = false;
		}
		this->skip_numbers = skip_numbers;
		this->remove_punct = remove_punct;
		this->min_len = min_len; this->max_len = max_len;
	};
	//////////////////////////////////////////////////////////////
	//Deprecated section
	//Deprecate function - it will be removed soon
	void TGlibUBow::SaveOldBin(TSOut& SOut) const{
		Option.Save(SOut);
		Lang.Save(SOut);
		StopSet.Save(SOut);
		MaxIndex.Save(SOut);
		skip_numbers.Save(SOut);
		remove_punct.Save(SOut);
		min_len.Save(SOut);
		max_len.Save(SOut);
		new_line_char.Save(SOut);
		BrOnSt.Save(SOut);
		WordIds.Save(SOut);
		WordNgrams.Save(SOut);
		Ngrams.Save(SOut);
		Matrix.Save(SOut);
	}
	//Load - Deprecated version - stemmer information is not used
	void TGlibUBow::LoadOldBin(TSIn& SIn){
		Option.Load(SIn);
		Lang.Load(SIn);
		if (Lang == "zh"){//Special Tokenization for Chinese
			stemmer_supported = ChineseTokenizer::init();
		}
		else{
			stemmer_supported = Stemmer.setStemmer(Lang);
		}
		StopSet.Load(SIn);
		MaxIndex.Load(SIn);
		skip_numbers.Load(SIn);
		remove_punct.Load(SIn);
		min_len.Load(SIn);
		max_len.Load(SIn);
		new_line_char.Load(SIn);
		BrOnSt.Load(SIn);
		WordIds.Load(SIn);
		WordNgrams.Load(SIn);
		Ngrams.Load(SIn);
		Matrix.Load(SIn);
	}
	//////////////////////////////////////////////////////////////
	//Save Function, it also saves if stemmer was used or not
	void TGlibUBow::SaveBin(TSOut& SOut) const{
		Option.Save(SOut);
		Lang.Save(SOut);
		stemmer_supported.Save(SOut);
		StopSet.Save(SOut);
		MaxIndex.Save(SOut);
		skip_numbers.Save(SOut);
		remove_punct.Save(SOut);
		min_len.Save(SOut);
		max_len.Save(SOut);
		new_line_char.Save(SOut);
		BrOnSt.Save(SOut);
		WordIds.Save(SOut);
		WordNgrams.Save(SOut);
		Ngrams.Save(SOut);
		Matrix.Save(SOut);
		//Added 
		InvDoc.Save(SOut);
		NDocs.Save(SOut);
		TFidf.Save(SOut);
	}
	
	void TGlibUBow::LoadBin(TSIn& SIn){
		Option.Load(SIn);
		Lang.Load(SIn);
		stemmer_supported.Load(SIn);
		if (stemmer_supported){
			bool OK = true;
			if (Lang == "zh"){//Special Tokenization for Chinese
				OK = ChineseTokenizer::init();
			}
			else{
				OK = Stemmer.setStemmer(Lang);
			}
			if (!OK){
				printf("%s\n",("Stemmer initialization failed but originaly stemmer for " + Lang + " was used! You will obtain meaningless results!\n").CStr());
			}
		}
		StopSet.Load(SIn);
		MaxIndex.Load(SIn);
		skip_numbers.Load(SIn);
		remove_punct.Load(SIn);
		min_len.Load(SIn);
		max_len.Load(SIn);
		new_line_char.Load(SIn);
		BrOnSt.Load(SIn);
		WordIds.Load(SIn);
		WordNgrams.Load(SIn);
		Ngrams.Load(SIn);
		Matrix.Load(SIn);
		//Compatibility check
		if (!SIn.Eof()){
			InvDoc.Load(SIn);
		}
		if (!SIn.Eof()){
			NDocs.Load(SIn);
		}
		if (!SIn.Eof()){
			TFidf.Load(SIn);
		}
	}
	
	void TGlibUBow::DelMatrix(){
		this->Matrix.Clr();
	}
	//Simple export to text format, outputn directory must exist!
	void TGlibUBow::Print(const TStr& Directory) const{
		//Word Ids
		FILE *idwords = fopen((Directory + Lang + "words.txt").CStr(), "w");
		//Bow-Doc matrix
		FILE *idmatrix = fopen((Directory + Lang + "matrika.txt").CStr(), "w");
		//Export WordIds to text format
		for (int i = 0; i < this->WordIds.Len(); i++){
			TUStr pom = this->WordIds.GetKey(i);
			fprintf(idwords, "%s\n", pom.GetStr().CStr());

		}
		fclose(idwords);
		//Export Bow-Doc matrix to text format
		for (int stolpec = 0; stolpec < this->Matrix.Len(); stolpec++){
			for (int j = 0; j < this->Matrix[stolpec].Len(); j++){
				fprintf(idmatrix, "%d %d %d\n", this->Matrix[stolpec][j].Key.Val + 1, stolpec + 1, this->Matrix[stolpec][j].Dat.Val);
			}

		}
		fclose(idmatrix);
		//Export WordNgram ids to text format
		if (this->Option == tWordNgram){
			FILE *idbg = fopen(("./outputn/" + Lang + "wordngram.txt").CStr(), "w");
			for (int i = 0; i < this->WordNgrams.Len(); i++){
				TUStr pom = this->WordNgrams.GetKey(i);
				for (int j = 0; j < pom.Len(); j++){
					fprintf(idbg, "%d\t", pom[j].Val);
				}
				fprintf(idbg, "\n");

			}
			fclose(idbg);
		}
		else
			//Export CharNgram ids to text format
		if (this->Option == tCharNgram || this->Option == tSimpleCharNgram){
			FILE *idbcharg = fopen(("./outputn/" + Lang + "charngram.txt").CStr(), "w");
			for (int i = 0; i < this->Ngrams.Len(); i++){
				TUStr pom = this->Ngrams.GetKey(i);
				fprintf(idbcharg, "%s\n", pom.GetStr().CStr());
			}
			fclose(idbcharg);
		}

	}
	//SetStop Words From File
	//Usually it is not worth the hassle
	//Postprocessing is better and simpler
	void TGlibUBow::SetStop(TStr File, TBool BrOnSt){
		StopSet.LoadFromFile(File);
		this->BrOnSt = BrOnSt;
	}
	//WordNgrams need further testing
	TVec<TIntKd> TGlibUBow::TextToVec(TUStr& Text){	
		switch (Option) {
		case tWord:
			return this->_TokenizeWords(Text);
			break;
		case tWordNgram:
			return this->TokenizeWordNgrams(Text);
			break;
		case tCharNgram:
			return this->TokenizeNgrams(Text);
			break;
		case tSimpleCharNgram:
			return this->TokenizeSimpleNgrams(Text);
			break;
		default:
			return TVec<TIntKd>();
		}
	};
	//Version that returns the format suitable for external libraries
	void TGlibUBow::TextToVec(TUStr& Text, TPair<TIntV, TFltV>& SparseVec) {
		TVec<TIntKd> SpVec = TextToVec(Text);
		//Convert
		TIntV IdxV; 
		TFltV ValV;
		IdxV.Gen(SpVec.Len());
		ValV.Gen(SpVec.Len());
		for (int ElN = 0; ElN < SpVec.Len(); ElN++) {
			IdxV[ElN] = SpVec[ElN].Key;
			ValV[ElN] = SpVec[ElN].Dat;
		}
		SparseVec.Val1 = IdxV;
		SparseVec.Val2 = ValV;
	}

	void TGlibUBow::TextToVec(TUStrV& Docs, TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TFltV& invdoc) {
		TIntV* WordIdxV = &DocMatrix.Val1;
		TIntV* DocIdxV  = &DocMatrix.Val2;
		TFltV* ValV     = &DocMatrix.Val3;
		int n = Docs.Len();
		TVec<TVec<TIntKd>> DocV;
		DocV.Gen(n, n);
#pragma omp parallel for
		for (int i = 0; i < Docs.Len(); i++){
			TVec<TIntKd>& SpVec = DocV[i];
			SpVec = TextToVec(Docs[i]);
		}

		for (int i = 0; i < Docs.Len(); i++){
			TVec<TIntKd>& SpVec = DocV[i];
			int m = SpVec.Len();
			for (int ElN = 0; ElN < m; ElN++) {
				DocIdxV->Add(i);
				TInt Index = SpVec[ElN].Key;
				TInt Freq = SpVec[ElN].Dat;
				WordIdxV->Add(Index);
				TFlt Weighted = (double)(Freq.Val) * invdoc[Index.Val];
				ValV->Add(Weighted);
			}
		}
	}

	void TGlibUBow::TextToVec(TUStrV& Docs, TTriple<TIntV, TIntV, TFltV>& DocMatrix) {
		TIntV* WordIdxV = &DocMatrix.Val1;
		TIntV* DocIdxV = &DocMatrix.Val2;
		TFltV* ValV     = &DocMatrix.Val3;
		int n = Docs.Len();
		TVec<TVec<TIntKd>> DocV;
		DocV.Gen(n, n);
#pragma omp parallel for
		for (int i = 0; i < Docs.Len(); i++){
			TVec<TIntKd>& SpVec = DocV[i];
			SpVec = TextToVec(Docs[i]);
		}

		for (int i = 0; i < Docs.Len(); i++){
			TVec<TIntKd>& SpVec = DocV[i];
			int m = SpVec.Len();
			for (int ElN = 0; ElN < m; ElN++) {
				DocIdxV->Add(i);
				WordIdxV->Add(SpVec[ElN].Key);
				ValV->Add(TFlt(SpVec[ElN].Dat));
			}
		}
	}

	void TGlibUBow::AddTokenize(TUStr& Doc, TBool AddToMatrix, TBool UpdateVoc){
		switch (this->Option.Val){
		case tWordNgram:
			this->AddTokenizeWordNgrams(Doc, AddToMatrix, UpdateVoc);
			break;
		case tWord:
			this->AddTokenizeWords(Doc, AddToMatrix, UpdateVoc);
			break;
		case tCharNgram:
			this->AddTokenizeNgrams(Doc, AddToMatrix, UpdateVoc);
			break;
		case tSimpleCharNgram:
			this->AddTokenizeSimpleNgrams(Doc, AddToMatrix, UpdateVoc);
			break;
		}
	}
	TUStr  TGlibUBow::GetToken(int TokenIndex){
		switch (this->Option.Val){
		case tWordNgram:{
										Assert(TokenIndex < WordNgrams.Len() && TokenIndex >= 0);
										TUStr WordIdsCollection = WordNgrams.GetKey(TokenIndex);
										TUStr ReturnUstr; ReturnUstr.Reserve(11 * WordIdsCollection.Len());
										for (int i = 0; i < WordIdsCollection.Len(); i++){
											TUStr temp = WordIds.GetKey(WordIdsCollection[i] - 1);
											ReturnUstr.Add('\t');
											ReturnUstr.AddV(temp);
										}
										return ReturnUstr;
										break;
		}
		case tWord:
			Assert(TokenIndex < WordIds.Len() && TokenIndex >= 0);  return  WordIds.GetKey(TokenIndex);
			break;
		case tCharNgram:
			Assert(TokenIndex < Ngrams.Len() && TokenIndex >= 0);  return  Ngrams.GetKey(TokenIndex);
			break;
		case tSimpleCharNgram:
			Assert(TokenIndex < Ngrams.Len() && TokenIndex >= 0);  return  Ngrams.GetKey(TokenIndex);
			break;
		}
        throw TExcept::New("Unknown TBowOptTag");
	}
	int  TGlibUBow::GetNumberOfTokens(){
		switch (this->Option.Val){
		case tWordNgram:
			return WordNgrams.Len();
			break;
		case tWord:
			return WordIds.Len();
			break;
		case tCharNgram:
			return Ngrams.Len();
			break;
		case tSimpleCharNgram:
			return Ngrams.Len();
			break;
		}
        throw TExcept::New("Unknown TBowOptTag");
	}

	TVec<TIntKd> TGlibUBow::AddTokenizeSimpleNgrams(TUStr& Text, TBool LowerCase, TBool AddToMatrix, TBool UpdateVoc){
		TUStr Word = Text.NormalizeSpaces();
		if(LowerCase)	Word.ToLowerCase();
		TIntIntH NGramsV;

		int dolzina_besede = Word.Len();//Words[besede].Len();
			//Generate Ngrams from a word efficiently
			int pos = 0;
			int last_ngram_pos = dolzina_besede - min_len;
			//TUStr Word = ProcessText;// Words[besede];
			while (pos < last_ngram_pos){
				//No suitable Ngrams
				int dol = Word.Len();
				if (min_len > dol){
					break;
				}
				//Cannot go over Word length!
				int real_max = MIN(max_len.Val, dol - pos);
				//Reserve to avoid memory copy
				TUStr Char_Ngram; Char_Ngram.Reserve(max_len);
				//Char_Ngram.AddV(Word.GetSub(0, min_len-1));
				Word.GetSubValV(pos, pos + min_len - 1, Char_Ngram);
				//UpdateHashComputation
				Char_Ngram.UpdateHashCd();
				int id = -1;
				if (UpdateVoc){
					id = this->Ngrams.AddKey(Char_Ngram);
					this->Ngrams[id]++;
				}
				else{
					if (!this->Ngrams.IsKey(Char_Ngram, id)){
					//No more Ngrams possible, advance the position and continue
						pos++;
						continue;
					}
				}
				NGramsV.AddDat(id)++;
				//std::cout << Char_Ngram.GetStr().CStr() << "\n";
				int last_index_over = pos + real_max;
				for (int ngram_pos = pos + min_len; ngram_pos < last_index_over; ngram_pos++){
					Char_Ngram.UpdateHashCd(Word[ngram_pos]);
					Char_Ngram += Word[ngram_pos];
					//std::cout << Char_Ngram.GetStr().CStr() << "\n";
					int id = -1;
					if (UpdateVoc){
						this->Ngrams.AddKey(Char_Ngram);
						this->Ngrams[id]++;
					}
					else{
						break;
					}
					NGramsV.AddDat(id)++;
				}
				pos++;
			}
			NGramsV.SortByKey();
			TVec<TIntKd> Vector;
			NGramsV.GetKeyDatKdV(Vector);
			//Document As Vector of Pairs
			if(AddToMatrix) Matrix.Add(Vector);
			int Len = Vector.Len();
			if (Len > 0){
				this->MaxIndex = MAX(this->MaxIndex, TInt(Vector[Len - 1].Key + 1));
			}
			return Vector;
	}
	TVec<TIntKd> TGlibUBow::TokenizeSimpleNgrams(TUStr& Text){
		return TGlibUBow::AddTokenizeSimpleNgrams(Text, true, false, false);
		//return this->Vector;
	}
		
	void TGlibUBow::Tokenize(TUStr& Doc){
		switch (this->Option.Val){
		case tWordNgram:
			this->TokenizeWordNgrams(Doc);
			break;
		case tWord:
			this->_TokenizeWords(Doc);
			break;
		case tCharNgram:
			this->TokenizeNgrams(Doc);
			break;
		case tSimpleCharNgram:
			this->TokenizeSimpleNgrams(Doc);
			break;
		}
	}
	TVec<TIntKd> TGlibUBow::AddTokenizeWords(TUStr& Text, TBool AddToMatrix, TBool UpdateVoc){
		TLst<TUStr> Words; TLst<TBool> Seperators;
		TokenizeWordsPreprocess(Text, Words, Seperators);
		TLst<TUStr>::PLstNd WordNode = Words.First();
		TIntIntH IdWordsV;
		while (WordNode){
			int id = -1;
			if (UpdateVoc){
				id = this->WordIds.AddKey(WordNode->Val);
				this->WordIds[id]++;
				IdWordsV.AddDat(id)++;
			}
			else{
				if (this->WordIds.IsKey(WordNode->Val, id)){
					IdWordsV.AddDat(id)++;
				}
			}
			WordNode = WordNode->NextNd;
		}
		Words.Clr();
		Seperators.Clr();
		IdWordsV.SortByKey();
		TVec<TIntKd> Vector;
		IdWordsV.GetKeyDatKdV(Vector);
		int Len = Vector.Len();
		if (Len > 0){
			this->MaxIndex = MAX(this->MaxIndex, Vector[Len - 1].Key);
		}
		if (AddToMatrix)	this->Matrix.Add(Vector);
		return Vector;
	}
	//Word Tokenization
	TVec<TIntKd> TGlibUBow::_TokenizeWords(TUStr& Text){
		return TGlibUBow::AddTokenizeWords(Text, false, false);
		//return this->Vector;
	};
	//Word Ngrams as Tokens Simple and clean implementation
	TVec<TIntKd>  TGlibUBow::TokenizeWordNgrams(TUStr& Text){
		return TGlibUBow::AddTokenizeWordNgrams(Text, false, false);
		//return this->Vector;
	}
	//Word Ngrams as Tokens Simple and clean implementation - function can be used for building Bows
	//Word Ngrams as Tokens Simple and clean implementation - function can be used for building Bows
	TVec<TIntKd> TGlibUBow::AddTokenizeWordNgrams(TUStr& Text, TBool AddToMatrix, TBool UpdateVoc){
		//Text = TUStr("To");
		TLst<TUStr> Words; TLst<TBool> Seperators;
		//Tokenization
		TokenizeWordsPreprocess(Text, Words, Seperators);
		int beseda = 0;
		int n_besede = Words.Len();
		TUStr DocWordIds;
		TLst<TUStr>::PLstNd WordNode = Words.First();
		TLst<TBool>::PLstNd BoolNode = Seperators.First();
		TLst<TBool>::PLstNd StartBoolNode = Seperators.First();
		//StartBoolNode = StartBoolNode->NextNd;
		//printf("%d %d", Words.Len(), Seperators.Len());
		//Map Words to their ids OK
		while (beseda < n_besede){
			int id = -1;
			if (UpdateVoc){
				id = WordIds.AddKey(WordNode->Val);
				//id->id+1 Prevent troubles with Zero when computing hash code
				DocWordIds.Add(id + 1);
			}
			else{
				bool is_id = WordIds.IsKey(WordNode->Val, id);
				if (is_id){
					DocWordIds.Add(id + 1);//!!!!BUGGER id->id+1 0 Hashing is bad
					BoolNode = BoolNode->NextNd;
				}
				else{
					TLst<TBool>::PLstNd BoolNodeTemp = BoolNode->NextNd;
					Seperators.Del(BoolNode); BoolNode = BoolNodeTemp;
					//Break WordsNgram generation if word is unknown
					if (!Seperators.Empty()){
						BoolNode->Val = true;
					}
				}
			}

			WordNode = WordNode->NextNd;
			++beseda;
		}
		//Words Ngram generation
		//Iterate through all the words and generate Ngrams
		TIntIntH NGramsV;
		int dolzina = DocWordIds.Len();
		//Traverse through Word Ids Vector
		//Be Careful not to go over the end
		int start_index = 0;
		int end_index = max_len;
		int final_end_index = dolzina - 1;
		//Iterate through dolzina number of words
		for (int i = 0; i < dolzina; i++){
			//Prevent index over the length
			end_index = MIN(end_index, final_end_index);
			int dol = 1;
			//Preveri kje se lahko formirajo besedni n-grami
			//Iti cez locila najbrz nima nobenega smisla


			int zac_dolzina = MIN(max_len.Val, dolzina - i);
			int last = i + zac_dolzina - 1;
			TLst<TBool>::PLstNd BoolNode = StartBoolNode->NextNd;
			for (int k = i; k < last; k++){
				if ((BoolNode == NULL) || BoolNode->Val){
					break;
				}
				BoolNode = BoolNode->NextNd;
				dol++;
			}
			end_index = start_index + (dol - 1);
			TUStr temp(dol + 1, 0);
			int end_start_pos = i + (min_len - 1);
			if (end_start_pos >(dolzina - 1)){
				break;
			}
			DocWordIds.GetSubValV(i, end_start_pos - 1, temp);
			temp.UpdateHashCd();
			for (int j = end_start_pos; j <= end_index; j++){
				//PrintVec(temp1);
				temp.Add(DocWordIds[j]);
				temp.UpdateHashCd(DocWordIds[j]);
				int id = -1;
				if (UpdateVoc){
					id = WordNgrams.AddKey(temp);
					WordNgrams[id]++;
					NGramsV.AddDat(id)++;
				}
				else{
					bool is_id = this->WordNgrams.IsKey(temp, id);
					if (!is_id){
						break;
					}
					NGramsV.AddDat(id)++;
				}
			}
			if (StartBoolNode != NULL) StartBoolNode = StartBoolNode->NextNd;
			start_index++;
			end_index++;
		}

		Words.Clr();
		Seperators.Clr();
		NGramsV.SortByKey();
		TVec<TIntKd> Vector;
		NGramsV.GetKeyDatKdV(Vector);
		//Document as Vector of Pairs
		if (AddToMatrix) Matrix.Add(Vector);
		int Len = Vector.Len();
		if (Len > 0){//+1 unit test is needed
			this->MaxIndex = MAX(this->MaxIndex, Vector[Len - 1].Key);
		}
		return Vector;
		//WordNgrams.AddDat(temp++;
	}
	//To unify the interface this shoudl be void TGlibUBow::AddTokenizeNgrams(TUStr& Text, TVec<TKey>& KeyV, TVec<TDat> DatV, TBool AddToMatrix, TBool UpdateVoc){
	//or even better TGlibUBow::AddTokenizeNgrams(TUStr& Text, TPair<TIntV, TFltV> Vector, TBool AddToMatrix, TBool UpdateVoc)
	//Moreover it would be better to omit this->Vector variable
	TVec<TIntKd> TGlibUBow::AddTokenizeNgrams(TUStr& Text, TBool AddToMatrix, TBool UpdateVoc){//Character Ngrams as Tokens Simple and clean implementation
		TLst<TUStr> Words; TLst<TBool> Seperators;
		//TUStrV Words; TBoolV Seperators;
		TIntIntH NGramsV;
		//Text = TUStr("Poslu�am in delam, vendar delam tako, da je vsega zadosti, delam no� in dan.");
		//Preprocessing Step -> Clean Text, Mark Seperators
		TokenizeWordsPreprocess(Text, Words, Seperators);
		int besede = 0;
		int n_besede = Words.Len();
		printf("Stevilo besed %d\n", n_besede);
		TLst<TUStr>::PLstNd WordNode = Words.First();

		while (besede < n_besede){
			int dolzina_besede = WordNode->Val.Len();//Words[besede].Len();
			//Generate Ngrams from a word efficiently
			int pos = 0;
			int last_ngram_pos = dolzina_besede - min_len;
			TUStr Word = WordNode->Val;// Words[besede];
			while (pos < last_ngram_pos){
				//No suitable Ngrams
				int dol = Word.Len();
				if (min_len > dol){
					break;
					//continue;
				}
				//Cannot go over Word length!
				int real_max = MIN(max_len.Val, dol - pos);
				//Reserve to avoid memory copy
				TUStr Char_Ngram; Char_Ngram.Reserve(max_len);
				//Char_Ngram.AddV(Word.GetSub(0, min_len-1));
				Word.GetSubValV(pos, pos + min_len - 1, Char_Ngram);
				//UpdateHashComputation
				Char_Ngram.UpdateHashCd();
				int id = -1;
				if (UpdateVoc){
					printf("UpdateVoc\n");
					id = this->Ngrams.AddKey(Char_Ngram);
					this->Ngrams[id]++;
					NGramsV.AddDat(id)++;
				}
				else{
					printf("No UpdateVoc\n");
					if (this->Ngrams.IsKey(Char_Ngram, id)){
						printf("No UpdateVoc ++\n");
						NGramsV.AddDat(id)++;
					}
					else{//No more Ngrams possible, advance the position and continue
						pos++;
						continue;
					}
				}
				//std::cout << Char_Ngram.GetStr().CStr() << "\n";
				int last_index_over = pos + real_max;
				for (int ngram_pos = pos + min_len; ngram_pos < last_index_over; ngram_pos++){
					Char_Ngram.UpdateHashCd(Word[ngram_pos]);
					Char_Ngram += Word[ngram_pos];
					//std::cout << Char_Ngram.GetStr().CStr() << "\n";
					int id = -1;
					if (UpdateVoc){
						id = this->Ngrams.AddKey(Char_Ngram);
						this->Ngrams[id]++;
						NGramsV.AddDat(id)++;
					}
					else{
						if (this->Ngrams.IsKey(Char_Ngram, id)){
							NGramsV.AddDat(id)++;
						}
						else{
							break;
						}
					}
				}
				pos++;
			}
			WordNode = WordNode->NextNd;
			besede++;
		}
		printf("NGramsV Len: %d\n", NGramsV.Len());
		NGramsV.SortByKey();
		//Andrej this should be GetSpV(TVec<TKey>& KeyV, TVec<TDat> DatV)
		TVec<TIntKd> Vector;
		NGramsV.GetKeyDatKdV(Vector);
		//Document As Vector of Pairs
		if (AddToMatrix) Matrix.Add(Vector);
		int Len = Vector.Len();
		if (Len > 0){
			this->MaxIndex = MAX(this->MaxIndex, TInt(Vector[Len - 1].Key + 1));
		}
		return Vector;

	}
	TVec<TIntKd> TGlibUBow::TokenizeNgrams(TUStr& Text){//Character Ngrams as Tokens Simple and clean implementation
		return AddTokenizeNgrams(Text, false, false);
	}
	//Word Tokenization as preprocessing step for Word Tokens
	void TGlibUBow::TokenizeWordsPreprocess(TUStr& Text, TUStrV& Words, TBoolV& Seperators){
		if (Lang != "zh"){
			Text.GetWordUStrV(Words, Seperators);
		}
		else{
			ChineseTokenizer::tokenizeClean(Text.GetStr(), Words);
		}

		int i = 0;
		while (i < Words.Len()){
			if (skip_numbers){
				//Get Rid of Words Starting with the digit -- Replace this with custom filter function as parameter
				if (this->skip_numbers && TUStr::IsNumeric(Words[i][0])){
					Words.Del(i);
					TBool sep = Seperators[i];
					Seperators.Del(i);
					//Shrink the seperators list
					if (sep){
						if (!Seperators.Empty()) Seperators[i] = true;
					}
					continue;
				}
			}

			Words[i].ToLowerCase();
			TStr Word = Words[i].GetStr();
			if (!StopSet.IsEmpty()){
				if (StopSet.IsStop(Word)){
					Words.Del(i);
					TBool sep = Seperators[i];
					Seperators.Del(i);
					if (sep){
						if (i > 0) Seperators[i] = true;
					}
					continue;
				}
			}
			if (Stemmer.isSupported()){
				Stemmer.stem(Word);
				Words[i] = (TUStr)(Word);
			}
			i++;
		}
	};
	void TGlibUBow::TokenizeWordsPreprocess(TUStr& Text, TLst<TUStr> & Words, TLst<TBool> & Seperators, TBool cut){
		if (Lang != "zh"){
			Text.GetWordUStrLst(Words, Seperators);
		}
		else{
			ChineseTokenizer::tokenizeClean(Text.GetStr(), Words, Seperators);
		}

		int i = 0;
		TLst<TUStr>::PLstNd WordNode = Words.First();
		TLst<TBool>::PLstNd BoolNode = Seperators.First();
		while (WordNode){//i < Words.Len()
			if (skip_numbers){
				//Get Rid of Words Starting with the digit -- Replace this with custom filter function
				if ((this->skip_numbers && TUStr::IsNumeric(WordNode->Val[0])) || WordNode->Val.HasTerminal()){
					TLst<TUStr>::PLstNd WordNodeTemp = WordNode->NextNd;
					Words.Del(WordNode); WordNode = WordNodeTemp;
					TBool sep = BoolNode->Val;
					TLst<TBool>::PLstNd BoolNodeTemp = BoolNode->NextNd;
					Seperators.Del(BoolNode); BoolNode = BoolNodeTemp;
					//Shrink the seperators list
					//if(sep){
					//	printf("%d\n", Seperators.Len());
					if (cut || sep){
						if (!Seperators.Empty() && BoolNode != NULL) BoolNode->Val = true;
					}
					//}
					continue;
				}
			}

			WordNode->Val.ToLowerCase();
			TStr Word = WordNode->Val.GetStr();
			if (!StopSet.IsEmpty()){
				//std::cout << "STOP\n";
				if (StopSet.IsStop(Word)){
					TLst<TUStr>::PLstNd WordNodeTemp = WordNode->NextNd;
					Words.Del(WordNode);  WordNode = WordNodeTemp;
					TBool sep = BoolNode->Val;
					TLst<TBool>::PLstNd BoolNodeTemp = BoolNode->NextNd;
					Seperators.Del(BoolNode); BoolNode = BoolNodeTemp;
					//if(sep){
					if (cut || sep){
						if (!Seperators.Empty() && BoolNode != NULL) BoolNode->Val = true;
					}
					//}
					continue;
				}
			}
			if (Stemmer.isSupported()){
				Stemmer.stem(Word);
				WordNode->Val = (TUStr)(Word);
			}
			BoolNode = BoolNode->NextNd;
			WordNode = WordNode->NextNd;
			i++;
		}
	}
	void TGlibUBow::CompactVocabulary(TIntV& WordIndex, TInt Shift){
		this->Matrix.Clr();
		switch (Option) {
		case tWord:{
								   TUStrIntH CompactWordIds;
								   int n_Words = this->GetNumberOfWords();
								   int n = WordIndex.Len();	CompactWordIds.Gen(n);
								   printf("___________\nNumber of words before cut off: %d \n________________________", n_Words);
								   // cut low & high frequency words
								   int counter = 0;
								   int counter_prazen = 0;
								   int WId = 0;
								   for (int i = 0; i < n; i++){
									   WId = WordIndex[i] - Shift;
									   if (WId >= n_Words){
										   printf("%d %d\n", i, WId);
									   }
									   TUStr WordUStr; TInt WordFq;
									   //TUStr WordStr = this->WordIds.GetKey(WId);
									   this->WordIds.GetKeyDat(WId, WordUStr, WordFq);
									   if (WordFq == 0){
										   counter_prazen++;
									   }
									   counter++;
									   CompactWordIds.AddDat(WordUStr) = WordFq;
								   }
								   this->Matrix.Clr();
								   this->WordIds.Clr();
								   this->WordIds = CompactWordIds;
								   printf("___________\nNumber of words after cut off: %d \n________________________", counter);
								   printf("___________\nNumber of non appearing words before cut off: %d \n________________________", counter_prazen);
		}
			break;
		case tWordNgram:{
										TUStrIntH CompactWordNgrams;
										int n_WordNgrams = this->WordNgrams.Len();
										int n = WordIndex.Len();	CompactWordNgrams.Gen(n);
										printf("___________\nNumber of word ngrams before cut off: %d \n________________________", n_WordNgrams);
										// cut low & high frequency words
										int counter = 0;
										int counter_prazen = 0;
										int WNgramId = 0;
										for (int i = 0; i < n; i++){
											WNgramId = WordIndex[i] - Shift;
											if (WNgramId >= n_WordNgrams){
												printf("%d %d\n", i, WNgramId);
											}
											TUStr WordNgramUStr; TInt WordNgramFq;
											//TUStr WordStr = this->WordIds.GetKey(WId);
											this->WordNgrams.GetKeyDat(WNgramId, WordNgramUStr, WordNgramFq);
											if (WordNgramFq == 0){
												counter_prazen++;
											}
											counter++;
											CompactWordNgrams.AddDat(WordNgramUStr) = WordNgramFq;
										}
										this->Matrix.Clr();
										this->WordNgrams.Clr();
										this->WordNgrams = CompactWordNgrams;

										printf("___________\nNumber of word ngrams after cut off: %d \n________________________", counter);
										printf("___________\nNumber of non appearing ngrams before cut off: %d \n________________________", counter_prazen);
		}
			break;
			//tCharNgram: or BowOptTag::tCharNgramSimple is default
		default:{
										TUStrIntH CompactNgrams;
										int n_Ngrams = this->Ngrams.Len();
										int n = WordIndex.Len();	CompactNgrams.Gen(n);
										printf("___________\nNumber of ngrams before cut off: %d \n________________________", n_Ngrams);
										// cut low & high frequency words
										int counter = 0;
										int counter_prazen = 0;
										int NgramId = 0;
										for (int i = 0; i < n; i++){
											NgramId = WordIndex[i] - Shift;
											if (NgramId >= n_Ngrams){
												printf("%d %d\n", i, NgramId);
											}
											TUStr NgramUStr; TInt NgramFq;
											//TUStr WordStr = this->WordIds.GetKey(WId);
											this->Ngrams.GetKeyDat(NgramId, NgramUStr, NgramFq);
											if (NgramFq == 0){
												counter_prazen++;
											}
											counter++;
											CompactNgrams.AddDat(NgramUStr) = NgramFq;
										}
										this->Matrix.Clr();
										this->Ngrams.Clr();
										this->Ngrams = CompactNgrams;

										printf("___________\nNumber of ngrams after cut off: %d \n________________________", counter);
										printf("___________\nNumber of non appearing ngrams before cut off: %d \n________________________", counter_prazen);
		}
			break;
		}
		this->stemmer_supported = this->IsStemmerSupported();
	};

	void TGlibUBow::CompactMatrix(TIntV& DocIndex){
		TVec<TVec<TIntKd>> SubMatrix;
		int ndocs = DocIndex.Len();
		SubMatrix.Gen(ndocs, ndocs);
		for (int i = 0; i < ndocs; i++){
			int sub = DocIndex[i];
			SubMatrix[i] = this->Matrix[sub];
		}
		printf("Cut Done");
		Matrix = SubMatrix;

	}
	void TGlibUBow::processWikipediaLineDocGlib(const TStr& ime, TInt new_line_char, TBool AddToMatrix){
		int64 size = 0;
		const int dol = 134217728;
		char* buffer = new char[dol + 1];
		FILE* id0 = fopen(ime.CStr(), "rb");
		TStr Directory; TStr none;
		ime.SplitOnLastCh(Directory, '/', none);
		int doc = 0;
		TTmStopWatch ura(true);
		while ((size = loadToCBuffer(&id0, buffer, dol, new_line_char)) > 0){
			TStr Chunk = TStr(buffer);
			TStrV Documents;
			Chunk.SplitOnAllCh(new_line_char, Documents);
			int NDoc = Documents.Len();
			for (int i = 0; i < NDoc; i++){
				doc++;
				if (doc % 1000 == 0){
					printf("%d\r", doc);
				}
				TUStr Doc(Documents[i]);
				this->AddTokenize(Doc);
			}
		}
		printf("\nTime needed %f\n", ura.GetMSec());
		if (size > 0){
			delete[] buffer;
		}
	}

	void TGlibUBow::ExportMatrix(TTriple<TIntV, TIntV, TFltV>& Mat) {
		for (int ColN = 0; ColN < Matrix.Len(); ColN++) {
			for (int ElN = 0; ElN < Matrix[ColN].Len(); ElN++) {
				Mat.Val1.Add(Matrix[ColN][ElN].Key);
				Mat.Val2.Add(ColN);
				Mat.Val3.Add((double)Matrix[ColN][ElN].Dat);
			}
		}
	}
	void TGlibUBow::ComputeDocFreq(TIntV &InvDoc, TInt& NDocs){
		int nwords = this->GetNumberOfTokens();
		InvDoc.Gen(nwords, nwords); InvDoc.PutAll(TInt(0));
		int ndoc = Matrix.Len();
		for (int i = 0; i < ndoc; i++){
			TVec<TIntKd>& Doc = Matrix[i];
			int doclen = Doc.Len();
			for (int j = 0; j < doclen; j++){
				int word = Doc[j].Key;
				int freq = Doc[j].Dat;
				if (freq == 0) continue;
				InvDoc[word]++;
			}
		}
		NDocs = ndoc;
	}
    /*Tokenize for TStr and TUStr*/
	/*void Tokenize(TStr Text){
	return Tokenize(TUStr(Text), NULL, NULL);
	};*/
};

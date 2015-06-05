/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef CROSSLINGUAL_H
#define CROSSLINGUAL_H

#include "base.h"
#include "mine.h"

namespace TCrossLingual {

///////////////////////////////
// TCLProjectors
//   Interface to projector classes
class TCLProjector {
public:
	// project a sparse document	
	virtual void Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true) const = 0;
	// project a sparse matrix (columns = documents)
	virtual void Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& Tfidf = true, const TBool& DmozSpecial = true) const = 0;
	//get reference to proxy matrix
	virtual bool GetProxyMatrix(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& Proxy, bool& transpose_flag, const TStr& Friend) const = 0;
};

///////////////////////////////
// TCLHubProjector
//   All languages are aligned with a common (hub) language, typically English
class TCLHubProjector : public TCLProjector {
private:
	// Projectors: cols = num words, rows = num projectors
	TVec<TPair<TFltVV, TFltVV> > Projectors;
	// centroid length = num words
	TVec<TPair<TFltV, TFltV> > Centers;
	// inverse document frequency vectors
	TVec<TPair<TFltV, TFltV> > InvDoc;
	// Pairs of language ids
	THash<TPair<TStr, TStr>, TInt> LangIds;
	// Connection matrices for non-direct comparissons: sim(x,y) = x' X C_{X,Y} Y' y, based on two connections to the hub language: (H_X, X), (H_Y, Y)
	TVec<TPair<TFltVV, TFltVV> > ConMatrices;	
	// maps pairs of lang ids to an index in ConMatrices
	THash<TPair<TStr,TStr>, TInt> ConIdxH;
	// the index used to determine which hub basis to choose from Projectors when comparing documents in the hub language (example: when
	// comparing two English documents with Projectors{en_de, en_fr}, there are two choices
	int HubHubProjectorIdx;
	TStr Friend;
	// Hub language ID (example: "en")
	TStr HubLangId;
	// Maps language identifiers to indices in Projectors, Centers, ConMatrices
	// DocLangId -> idx1,  Lang2Id  -> idx2 in Projectors and centers and index in ConMatrices
	bool SelectMatrices(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& ProjMat, const TFltV*& Center, const TFltVV*& ConMat, const TFltV*& InvDocV, const TBool& DmozSpecial = true) const;
public:
	TCLHubProjector() {}
	// Load binary matrices
	void Load(const TStr& ModelFolder, const TStr& HubHubProjectorDir);
	// Load binary matirces use config files
	void Load(const TStr& ProjectorPathsFNm, const TStr& ConMatPathsFNm, const TStr& HubHubProjectorLangPairId);
	// project a sparse document	
	void Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true) const;
	// project a sparse matrix (columns = documents)
	void Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& Tfidf = true, const TBool& DmozSpecial = true) const;
	void DoTfidf(TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id);
	void DoTfidf(TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id);
	bool GetProxyMatrix(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& Proxy, bool& transpose_flag, const TStr& Friend) const;
	void SetFriend(TStr& _Friend){ this->Friend = _Friend; }
};


///////////////////////////////
// TCLPairwiseProjector
//   Language pairs have different common spaces. Includes logic for comparing documents in the same language
class TCLPairwiseProjector : public TCLProjector {
private:
	// Projectors: cols = num words, rows = num projectors
	TVec<TPair<TFltVV, TFltVV> > Projectors;
	// centroid length = num words
	TVec<TPair<TFltV, TFltV> > Centers;
	// Pairs of language ids
	TVec<TPair<TStr, TStr> > LangIds;
public:
	TCLPairwiseProjector() {}
	// Load binary matrices
	void Load(const TStr& ModelFolder);
	// project a sparse document	
	void Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true) const {}
	// project a sparse matrix (columns = documents)
	void Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& DmozSpecial = true) const {}
	bool GetProxyMatrix(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& Proxy, bool& transpose_flag, const TStr& Friend) const { return false; }
};


///////////////////////////////
// TCLCommonSpaceProjector
//   Projects to a common vector space
class TCLCommonSpaceProjector : public TCLProjector {
private:
	// Projectors[i]: cols = num words, rows = num projectors for the i-th language
	TVec<TFltVV> Projectors;
	// length(Centers[i]) = num words in language i
	TVec<TFltV> Centers;
	TVec<TStr> LangIds;
public:
	TCLCommonSpaceProjector() {}
	// project a sparse document	
	void Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true) const {}
	// project a sparse matrix (columns = documents)
	void Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& DmozSpecial = true) const {}
	bool GetProxyMatrix(const TStr& DocLangId, const TStr& Lang2Id, const TFltVV*& Proxy, bool& transpose_flag, const TStr& Friend) const { return false; }
};


///////////////////////////////
// TCLCore
//   Core cross-lingual class that provides: vector space representation, projections and similarity computation
class TCLCore {
private:
	TCLProjector* Projectors;
	THash<TStr, TUnicodeVSM::PGlibUBow> Tokenizers;	
	TVec<TStr> LangIdV; // i-th language <==> i-th unicode tokenizer
	TVec<TIntV> InvDocFq;
	TVec<THash<TStr, TInt> > BowMap; // each language gets a hash map that maps tokenized input words to indices
	TStr Friend;

public:
	TCLCore(TCLProjector* Projectors_, const TStrV& TokenzierLangIdV, const TStrV& TokenizerPaths);
	TCLCore(TCLProjector* Projectors_, const TStr& TokenizerPathsFNm);
	//TO-DO
	//void GetProxyMatrix(TStr Lang1, TStr Lang2){
	//	Projectors->
	//}
	// Text to sparse vector
	void TextToVector(TUStr& Text, const TStr& LangId, TPair<TIntV, TFltV>& SparseVec);
	// Vector of Unicode Text documents to DocumentMatrix
	void TextToVector(TUStrV& Docs, const TStr& LangId, TTriple<TIntV, TIntV, TFltV>& DocMatrix);
	// project a textual document	
	void Project(TUStr& Text, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true);
	// project vector of textual documents	
	void Project(TUStrV& Text, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& DmozSpecial = true);
	// project a sparse document	
	// project a sparse document	
	void Project(const TPair<TIntV, TFltV>& Doc, const TStr& DocLangId, const TStr& Lang2Id, TFltV& Projected, const TBool& DmozSpecial = true);
	// project a sparse matrix (columns = documents)
	void Project(const TTriple<TIntV, TIntV, TFltV>& DocMatrix, const TStr& DocLangId, const TStr& Lang2Id, TFltVV& Projected, const TBool& DmozSpecial = true);
	// Cosine similarity, text input
	double GetSimilarity(TUStr& Text1, TUStr& Text2, const TStr& Lang1Id, const TStr& Lang2Id);
	// Cosine similarity, sparse vector input
	double GetSimilarity(const TPair<TIntV, TFltV>& Doc1, const TPair<TIntV, TFltV>& Doc2, const TStr& Lang1Id, const TStr& Lang2Id);
	// Cosine similarity, sparse document matrix input
	void GetSimilarity(const TTriple<TIntV, TIntV, TFltV>& DocMtx1, const TTriple<TIntV, TIntV, TFltV>& DocMtx2, const TStr& Lang1Id, const TStr& Lang2Id, TFltVV& SimMtx);
	// Returns top k (weight, wordid) pairs in lang1 and lang2 that contributed most to the similarity
	double ExplainSimilarity(const TPair<TIntV, TFltV>& Doc1, const TPair<TIntV, TFltV>& Doc2, const TStr& Lang1Id, const TStr& Lang2Id, TVec<TPair<TFlt,TInt> >& Lang1Words, TVec<TPair<TFlt,TInt> >& Lang2Words);	
	// Returns top k words as TUStr in lang1 and lang2 that contributed most to the similarity
	double ExplainSimilarity(TUStr& Text1, TUStr& Text2, const TStr& Lang1Id, const TStr& Lang2Id, TUStrV& Lang1TopWords, TUStrV& Lang2TopWords, const int& k);
	// Return word string (unicode bow)
	TUStr GetWordByKeyIdLangId(const TStr& LangId, const TInt& WordIndex) {return Tokenizers.GetDat(LangId)->GetWord(WordIndex);}
	// GetSimilarity for two vectors with given language Ids, proxy matrix is not applied, be careful with connection matrices select!!!!! is not symmetric
	// GetSimilarity for two matrices with given language Ids, proxy matrix is not applied, be careful with connection matrices select!!!!! is not symmetric
	// GetSimilarity for vector and matrix with given language Ids, proxy matrix is not applied, be careful with connection matrices select!!!!! is not symmetric
	void SetFriend(TStr& _Friend){ this->Friend = _Friend; }


};

///////////////////////////////
// TCLDmozNode
//   Dmoz classifier node
class TCLDmozNode {
	public:
		TCLDmozNode() {}
		void Disp();
		void Save(TSOut& SOut) const;
		explicit TCLDmozNode(TSIn& SIn): DocIdxV(SIn), CatStr(SIn), CatPathStr(SIn), StrToChildIdH(SIn){}
		friend class TCLDmoz;
	private:
		TIntV DocIdxV; //DocIdxV = indices of documents for a given node	
		TStr CatStr; 
		TStr CatPathStr; 
		TStrIntH StrToChildIdH; // string -> child Id		
};


///////////////////////////////
// TCLCNode
//   Dmoz classifier node
class TCLCNode {
public:
	TCLCNode() {}
	void Disp();
	void Save(TSOut& SOut) const;
	explicit TCLCNode(TSIn& SIn): DocIdxV(SIn), CatStr(SIn), CatPathStr(SIn), StrToChildIdH(SIn){}
	friend class TCLClassifier;
private:
	TIntV DocIdxV; //DocIdxV = indices of documents for a given node	
	TStr CatStr; // Category id
	TStr CatPathStr; // Category path
	TStrIntH StrToChildIdH; // string -> child Id	
};


///////////////////////////////
// TCLClassifier
//   Dmoz classifier
class TCLClassifier;
typedef TPt<TCLClassifier> PCLClassifier;
class TCLClassifier {
	private:
		TCRef CRef;
	public:
		friend class TPt<TCLClassifier>;
private:
	// Core cross-lingual functionalities
	TCLCore* CLCore;	
	TStr DmozPath; 
	TStr ModelPath; //path where the model is stored
	
	TVec<TStr> Cat;
	//TVec<TVec<TIntKd> > Doc;	
	TTriple<TIntV, TIntV, TFltV> Doc;

	TStr HierarchyLangId; // dmoz -> "en"
	TStr Lang2Id;

	// Derived data
	TTree<TCLCNode> Taxonomy;	
	TFltVV PDoc;	
	TFltVV Centroids;

	int CutDepth;

public:
	TCLClassifier() {};
	static PCLClassifier New() {return PCLClassifier(new TCLClassifier());}
	TCLClassifier(const TStr& DmozPath_,const TStr& ModelPath_, const TStr& HierarchyLangId_, const TStr& Lang2Id_, TCLCore* CLCore_, int CutDepth_ = -1) {DmozPath = DmozPath_; ModelPath = ModelPath_; HierarchyLangId = HierarchyLangId_; Lang2Id = Lang2Id_; CLCore = CLCore_; CutDepth = CutDepth_;}	
	static PCLClassifier New(const TStr& DmozPath_,const TStr& ModelPath_, const TStr& HierarchyLangId_, const TStr& Lang2Id_, TCLCore* CLCore_, int CutDepth_ = -1) {return PCLClassifier(new TCLClassifier(DmozPath_, ModelPath_, HierarchyLangId_, Lang2Id_, CLCore_, CutDepth_));}
	// Loads the data
	void LoadData();
	// Load model
	void LoadModel();
	// Projects dmoz documents
	void ProjectDmoz();
	// Computes the centroids	
	void ComputeModel();
	// Get the centroid specific category string
	TStr& GetClassStr(const int& NodeId) {return Taxonomy.GetNodeVal(NodeId).CatStr;}
	// Get the centroid specific category path string
	TStr& GetClassPathStr(const int& NodeId) {return Taxonomy.GetNodeVal(NodeId).CatPathStr;}
	// Classify a single document
	void Classify(TUStr& Text, TInt& Class, const TStr& TextLangId, const TStr& Lang2Id);
	// Classify a single document, return top k classes
	void Classify(TUStr& Text, TIntV& Class, const TStr& TextLangId, const TStr& Lang2Id, const int& k = 10, const TBool& DmozSpecial = true);
	// Classify projected test document
	void ClassifyProjected(const TFltV& ProjVec, TInt& Class);
	// Classify projected test document, return top k classes
	void ClassifyProjected(const TFltV& ProjVec, TIntV& Class, const int& k);

	// Classify projected tests document, return top k classes
	void ClassifyProjected(const TFltVV& ProjVecVV, TVec<TIntV> & ClassVV, const int& k);
	// Get most frequent keywords from class path vector
	void GetBestKWordV(const TStrV& PathV, TStrV& KeywordV, TIntV& KeyFqV);
	// Get most frequent keywords from class path vector
	void GetBestKWordV(const TIntV& ClassV, TStrV& KeywordV, TIntV& KeyFqV);
	// Get most frequent keywords from class path vector
	void GetBestKWordV(const TIntV& ClassV, TStrV& KeywordV, TIntV& KeyFqV, TStrV& Categories);
	// Get most frequent keywords from class path vector
	void GetBestKWordV(const TVec<TIntV> & ClassVV, TVec<TStrV>& KeywordVV, TVec<TIntV>& KeyFqV, TVec<TStrV>& Categories);
private:	
		//// Building and using the model
		// Tree breadth first search
		template <class TVal>
		void TreeBFS(const TTree<TVal>& Tree, TIntV& IdxV) {
			IdxV.Gen(Tree.GetNodes(), 0);
			TQQueue<TInt> Queue;	
			Queue.Push(0); //root
			while (!Queue.Empty()) {
				int NodeId = Queue.Top();
				IdxV.Add(NodeId);
				Queue.Pop();
				for (int ChildN = 0; ChildN < Tree.GetChildren(NodeId); ChildN++) {
					Queue.Push(Tree.GetChildNodeId(NodeId, ChildN));
				}
			}
		}	
		////// Loading, saving, displaying, stats 
		// Save tree ID strings into a file based on a given ordering of elements
		void PrintTreeBFS(TTree<TCLCNode>& Tree, const TIntV& IdxV, const TStr& FNm);	
	
		//// Loads an int vector (ascii)
		//void LoadTIntV(const TStr& FilePath, TIntV& Vec);
		//// Saves an int vector (ascii)
		//void SaveTIntV(const TStr& FilePath, TIntV& Vec);
		//// Loads a string vector (ascii)
		//void LoadTStrV(const TStr& FilePath, TStrV& Vec);
		//// Saves document class paths (matlab sparse matrix - use load -ascii and spconvert)
		//void SaveDocumentClasses();
		//// Saves centroid class paths (matlab sparse matrix - use load -ascii and spconvert)
		//void SaveCentroidClasses();
		//// Saves path strings to a file
		//void SaveCentroidClassesPaths();		
		//// Computes and saves taxonomy statistics (node depths and number of documents per node)
		//void TreeStats();						
};




///////////////////////////////
// TCLStream
//   Comparing streams of documents
class TCLStream {
private:
	// Core cross-lingual functionalities
	TCLCore* CLCore;
	TVec<TStr> StreamLangIds; // language of each stream
	TIntV ProjStreamIdxV; // language index (points to StreamLangIds)
	TVec<TPair<TStr, TStr> > ProjStreamLangPairs; // sorted language pairs, autogenerated by using StreamLangIds and TrackPairs
	//**********************************************************************************************************************

	//// 4 languages: en, es, de, fr	
	//// 5 streams with their indices
	// 0 <-> en_bloomberg
	// 1 <-> en_news
	// 2 <-> es_news
	// 3 <-> de_news
	// 4 <-> fr_news

	//StreamLangIds[0] = en;
	//StreamLangIds[1] = en;
	//StreamLangIds[2] = es;
	//StreamLangIds[3] = de;
	//StreamLangIds[4] = fr;
	
	//// Selection of pairs of streams to track
	// TrackPairs[0] = 0, 1 ; track similarities in SimMatrices[0] // compare en_bloomberg and en_news
	// TrackPairs[1] = 1, 2 ; track similarities in SimMatrices[1] // compare en_news and es_news
	// TrackPairs[2] = 0, 3 ; track similarities in SimMatrices[2] // compare en_bloomberg and de_news
	// TrackPairs[3] = 2, 4 ; track similarities in SimMatrices[3] // compare es_news and fr_news
	// TrackPairs[4] = 0, 2 ; track similarities in SimMatrices[4] // compare en_bloomberg and es_news
		
	//**********************************************************************************************************************

	//// CCA (Pairwise) projectors (three pairs)
	// Projectors[0] = Pen, Pes; LangIds[0] = en, es
	// Projectors[1] = Pen, Pde; LangIds[1] = en, de
	// Projectors[2] = Pes, Pfr; LangIds[2] = es, fr

	// en_bloomberg <-> en_news
	// ProjStreams[0] = en_bloomberg projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = en
	// ProjStreams[1] = en_news projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = en
	// en_news <-> es_news
	// ProjStreams[2] = en_news projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = es
	// ProjStreams[3] = es_news projected with Projectors, ProjStreamLangPairs lang1 = es, lang2 = en
	// en_bloomberg <->  de_news
	// ProjStreams[4] = en_bloomberg projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = de
	// ProjStreams[5] = de_news projected with Projectors, ProjStreamLangPairs lang1 = de, lang2 = en
	// es_news <-> fr_news
	// ProjStreams[6] = es_news projected with Projectors, ProjStreamLangPairs lang1 = es, lang2 = fr
	// ProjStreams[7] = fr_news projected with Projectors, ProjStreamLangPairs lang1 = fr, lang2 = es
	// en_bloomberg <-> es_news
	// ProjStreams[8] = en_bloomberg projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = es
	// We do not need ProjStreams[9] = ProjStreams[3] = es_news projected with Projectors, ProjStreamLangPairs lang1 = es, lang2 = en
	
	// ProjStreamIdxV[0] = 0 // en_bloomberg
	// ProjStreamIdxV[1] = 1 // en_news
	// ProjStreamIdxV[2] = 1 // en_news
	// ProjStreamIdxV[3] = 2 // es_news
	// ProjStreamIdxV[4] = 0 // en_bloomberg
	// ProjStreamIdxV[5] = 3 // de_news
	// ProjStreamIdxV[6] = 2 // es_news
	// ProjStreamIdxV[7] = 4 // fr_news
	// ProjStreamIdxV[8] = 0 // en_bloomberg

	// SimMatrices[0] = ProjStreams[0]'*ProjStreams[1] // en_bloomberg <-> en_news
	// SimMatrices[1] = ProjStreams[2]'*ProjStreams[3] // en_news <-> es_news
	// SimMatrices[2] = ProjStreams[4]'*ProjStreams[5] // en_bloomberg <->  de_news
	// SimMatrices[3] = ProjStreams[6]'*ProjStreams[7] // es_news <-> fr_news
	// SimMatrices[4] = ProjStreams[8]'*ProjStreams[3] // en_bloomberg <-> es_news

	//**********************************************************************************************************************
	
    //// MCCA (Common) projectors
	// Projectors[0] = Pen; LangIds[0] = en
	// Projectors[1] = Pes; LangIds[1] = es
	// Projectors[2] = Pde; LangIds[2] = de
	// Projectors[3] = Pfr; LangIds[3] = fr
		
	// ProjStreams[0] = en_bloomberg projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = ignored
	// ProjStreams[1] = en_news projected with Projectors, ProjStreamLangPairs lang1 = en, lang2 = ignored
	// ProjStreams[2] = es_news projected with Projectors, ProjStreamLangPairs lang1 = es, lang2 = ignored
	// ProjStreams[3] = de_news projected with Projectors, ProjStreamLangPairs lang1 = de, lang2 = ignored
	// ProjStreams[4] = fr_news projected with Projectors, ProjStreamLangPairs lang1 = fr, lang2 = ignored
	
	// ProjStreamIdxV[0] = 0 // en_bloomberg
	// ProjStreamIdxV[1] = 1 // en_news
	// ProjStreamIdxV[2] = 2 // es_news
	// ProjStreamIdxV[3] = 3 // de_news
	// ProjStreamIdxV[4] = 4 // fr_news
	
	// SimMatrices[0] = ProjStreams[0]'*ProjStreams[0] // en_bloomberg <-> en_news
	// SimMatrices[1] = ProjStreams[0]'*ProjStreams[1] // en_news <-> es_news
	// SimMatrices[2] = ProjStreams[0]'*ProjStreams[2] // en_bloomberg <->  de_news
	// SimMatrices[3] = ProjStreams[1]'*ProjStreams[3] // es_news <-> fr_news
	// SimMatrices[4] = ProjStreams[0]'*ProjStreams[1] // en_bloomberg <-> es_news

	//**********************************************************************************************************************

	// Each matrix corresponds to particular projection of some stream, the columns are the newest projected documents
	TVec<TFltVV> ProjStreams;	
	// Column index of the newest document in each matrix in ProjStreams;
	TVec<TInt> LatestDocIdx;

	
	// Pairs of indices of ProjStreams that are to be compared
	TVec<TPair<TInt, TInt> > TrackPairs;
	// Similarity matrices between pairs in TrackPairs
	TVec<TFltVV> SimMatrices;
public:
	TCLStream(const TIntV& BufferSizes, const TIntV& ProjRowSizes, const TVec<TStr>& StreamLangIds_, const TVec<TPair<TInt, TInt> >& TrackPairs_) : StreamLangIds(StreamLangIds_),  TrackPairs(TrackPairs_) {
		// TODO: Set LatestDocIdx = 0
		// TODO: Gen ProjStreams and ProjStreamLangPairs (using StreamLangIds and TrackPairs)
		// TODO: Gen SimMatrices		
	}
	// Add document given text and its language id 
	void AddDocument(TUStr& Text, const TInt& StreamIdx) {
		TIntV ProjStreamIdxV;
		// TODO: Find all ProjStreams indices where ProjStreamIdxV == StreamIdx and add them to StreamIdxV		
		TStr LangId = StreamLangIds[StreamIdx];
		// Compute vector space model (fill SparseVec) for LangId outside of the loop
		TPair<TIntV, TFltV> SparseVec;
		CLCore->TextToVector(Text, LangId, SparseVec);		
		for (int ProjStreamN = 0; ProjStreamN < ProjStreamIdxV.Len(); ProjStreamN++) {						
			AddDocument(SparseVec, ProjStreamIdxV[ProjStreamN]);
		}
		// TODO: Compute all required similarities (based on TrackPairs and LatestDocIdx)
	}
	// Add document given text and stream index
	void AddDocument(const TPair<TIntV, TFltV>& SparseVec, const TInt& ProjStreamIdx) {
		// Use CLCore + ProjStreamLangPairs[ProjStreamIdx] to update the StreamIdx matrix				
		TFltV ProjText;	
		CLCore->Project(SparseVec, ProjStreamLangPairs[ProjStreamIdx].Val1 , ProjStreamLangPairs[ProjStreamIdx].Val2, ProjText);
		AddDocument(ProjText, ProjStreamIdx);
	}
	// Add projected document in a given stream
	void AddDocument(const TFltV& Doc, const TInt& ProjStreamIdx) {
		// TODO: Update ProjStreams[ProjStreamIdx] matrix
		// TODO: Increment LatestDocIdx[ProjStreamIdx]		
	}
	// TODO: Getters

};


}
#endif

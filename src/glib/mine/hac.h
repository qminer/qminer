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

namespace THrchAggClustSim {
	template<class TVector, class TLinAlgProvider>
	class TEucl {
	public:
		static double GetSim(const TVector& Item1, const TVector& Item2, const double DocN) {
			return TLinAlgProvider::EuclDist(Item1, Item2) / DocN;	// TODO is DocN used correctly???
		}
	};

	template<class TVector, class TLinAlgProvider>
	class TNegEucl {
	public:
		static double GetSim(const TVector& Item1, const TVector& Item2, const double DocN) {
			return -TLinAlgProvider::EuclDist(Item1, Item2) / DocN;	// TODO is DocN used correctly???
		}
	};

	template<class TVector, class TLinAlgProvider>
	class TCosine {
	public:
		static double GetSim(const TVector& Item1, const TVector& Item2, const double DocN) {
			return TLinAlgProvider::DotProduct(Item1, Item2) / DocN;
		}
	};

	//TODO:FIX - doesn't work as it should, negative similarities
	template<class TVector, class TLinAlgProvider>
	class TGroupAverage {
	public:
		static double GetSim(const TVector& Item1, const TVector& Item2, const double DocN) {
			TVector Sum(Item1.Len()); TLinAlgProvider::AddVec(Item1, Item2, Sum);		
			/*for (int i = 0; i < Sum.Len(); i++){
				printf("%f ", Sum[i].GetStr().CStr());
			}*/
			double dotProd = TLinAlgProvider::DotProduct(Sum, Sum);		
			return (dotProd - DocN)/(DocN * (DocN - 1));		
		}
	};
}

class TDefaultMAccess {
public:		
	static const TIntFltKdV& GetRow(const TSparseColMatrix *DocVV, int ItId) {
		return DocVV->ColSpVV[ItId];
	}
	static const TFltV& GetRow(const TFullColMatrix *DocVV, int ItId)  {
		return DocVV->ColV[ItId];
	}
};

////////////////////////////////////////////////
// Hierarchical clustering
/*
*	the HAC is based on the group average similarity measure as described 
*	at http://nlp.stanford.edu/IR-book/html/htmledition/time-complexity-of-hac-1.html
*	the implementation is the efficient HAC using priority queues
*/
template<class TVector, class TMatrix, class TLinAlgProvider, class TSimProvider>
class THrchAggClust {
public: 
	////////////////////////////////////////////////
	// The Cluster Object used by the HAC algorithm below
	class TCluster {
	private: 
		TVector VectSum;	
		TFltIntKdV PrQueue; //priority queue
		TIntV ItemVIdV;	
		int ItemN;
		bool Merged;			
		double MergeSimilarity;

	public:
		friend class TPt<TCluster>;

	public:
		TCluster() {};		
		TCluster(const TVector& _VectSum, const TIntV& _ItemVIdV, const int _ItemN, 
			const bool State) : VectSum(_VectSum), ItemVIdV(_ItemVIdV) 
		{ ItemN = _ItemN; Merged = State; };
	
		TVector& GetVectSum() { return VectSum; }
		TIntV& GetItemVIdV() { return ItemVIdV; }
		int GetItems() { return ItemN; }

		void Merge (TCluster& Cluster, double Similarity) {
			ItemVIdV.AddV(Cluster.GetItemVIdV()); 	
			TLinAlgProvider::AddVec(VectSum, Cluster.GetVectSum(), VectSum);
			TLinAlgProvider::Normalize(VectSum);		
			ItemN += Cluster.GetItems(); 
		}

		void AddToQueue(const TFlt& key, const TInt& val) { PrQueue.Add(TFltIntKd(key, val)); }
		void DelAddQueue(const TFlt& key, const TInt& val) { 							
			for (int IId = 0; IId < PrQueue.Len(); IId++) {
				if (PrQueue[IId].Dat == val) { PrQueue[IId].Key = key; }
			}		
		}
		void DelQueue (const TInt& val) {			
			for (int IId = 0; IId < PrQueue.Len(); IId++) {				
				if (PrQueue[IId].Dat == val) { PrQueue.Del(IId); break; }
			}			
		}
		void PrintQueue () {			
			for (int IId = 0; IId < PrQueue.Len(); IId++) {
				printf("%d ", PrQueue[IId].Dat.Val);				
			}
			printf("\n");
		}
		void ClearQueue() { PrQueue = TFltIntKdV(); }
		void SortQueue() { PrQueue.Sort(false); }

		int GetCandidateClusterId() { return PrQueue[0].Dat; }
		TFltIntKd GetSimilarityKD() { return PrQueue[0]; }
		void SetMerged() { Merged = true; }
		bool IsMerged() { return Merged; }
	};

	////////////////////////////////////////////////
	// The Assignment Object used by the HAC algorithm below
	class TAssign {
	private: 
		int Clust1Id;
		int Clust2Id;
		double Similarity;
		TIntV C1ItemIdV;	
		TIntV C2ItemIdV;	
		bool items;
	public: 
		TAssign() : C1ItemIdV(0), C2ItemIdV(0) {};
		TAssign(const int C1, const int C2, const double Sim, TIntV& _C1ItemIdV, 
			TIntV& _C2ItemIdV) : C1ItemIdV(_C1ItemIdV), C2ItemIdV(_C2ItemIdV) {
			Clust1Id = C1; Clust2Id = C2; Similarity = Sim; items = false;
		}

		int GetC1Id() { return Clust1Id; }
		int GetC2Id() { return Clust2Id; }
		TIntV& GetC1IdV() { return C1ItemIdV;}
		TIntV& GetC2IdV() { return C2ItemIdV;}
		bool AddedItems() { return items; }
		void setAddedItems() { items = true; }
		double GetSimilarity() { return Similarity; }
	};

public:
	friend class TPt<THrchAggClust<TVector, TMatrix, TLinAlgProvider, TSimProvider> >;

protected:	
	int ItemN;
	TVec<TFltV> DCSim; //Similarity matrix - size NxN
	TVec<TCluster> ClusterV; // all clusters = the number of items to be clustered						
	TVec<TAssign> AssignV;

private:	
	void Init(const TMatrix *ItemM) {
		DCSim = TVec<TFltV>(ItemM->GetRows()); 

		ItemN = ItemM->GetRows();
		int Items = ItemM->GetRows();
			for (int ItemN = 0; ItemN < Items; ItemN++) {
				DCSim[ItemN].Gen(Items);			
				TVector TmpV = TDefaultMAccess::GetRow(ItemM, ItemN);				
				TLinAlgProvider::Normalize(TmpV);		
				TIntV ItemVIdV; ItemVIdV.Add(ItemN);
				ClusterV.Add(TCluster(TmpV, ItemVIdV, 1, false));
			}
	}
	//compute the similarity matrix for each pair of items
	//only computed once, relevant elements updated in the CreateDendogram function
	void ComputeSimMatrix() {
		for (int ItN = 0; ItN < GetItems(); ItN++){
			for (int ColN = ItN + 1; ColN < GetItems(); ColN++){						
				DCSim[ItN][ColN] = DCSim[ColN][ItN] = 
					TFlt(TSimProvider::GetSim(ClusterV[ItN].GetVectSum(), ClusterV[ColN].GetVectSum(), 1));											  				
				ClusterV[ItN].AddToQueue(DCSim[ItN][ColN], ColN);
				ClusterV[ColN].AddToQueue(DCSim[ItN][ColN], ItN);							
			}
			ClusterV[ItN].SortQueue();
		}						
	}

	//retrieve the ID of one of the clusters with the highest value of similarity
	int GetCandidateClusterId() {
		TFltIntKdV CandidateClustV;
		for (int ItemN = 0; ItemN < GetItems(); ItemN++){	
			if (ClusterV[ItemN].IsMerged()) { continue; }
			CandidateClustV.Add(ClusterV[ItemN].GetSimilarityKD());			
		}		
		CandidateClustV.Sort(false);
		/*for (int RowN = 0; RowN < CandidateClustV.Len(); RowN++){	
			printf("%f %d ", CandidateClustV[RowN].Key, CandidateClustV[RowN].Dat);
		}
		printf("\n");*/
		return CandidateClustV[0].Dat;
	}

	//Execute the agglomerative clustering
	void CreateDendogram() {
		for (int ItN1 = 0; ItN1 < GetItems() - 1; ItN1++) {			

//			printf("similarity matrix: \n");
//			for (int RowN = 0; RowN < GetItems(); RowN++){
//				for (int ColN = 0; ColN < GetItems(); ColN++){
//					printf("%f  ", DCSim[RowN][ColN].Val);
//				}
//				printf("\n");
//			}
//			printf("\n");
//			for (int RowN = 0; RowN < GetItems(); RowN++){
//				ClusterV[RowN].PrintQueue();
//			}

			int C1 = GetCandidateClusterId();				
			int C2 = ClusterV[C1].GetCandidateClusterId();
			if (ClusterV[C2].IsMerged()) { continue; }
			double Sim = DCSim[C1][C2];
			printf("Merging cluster %d into cluster %d with similarity %f \n", C2, C1, Sim);
			AssignV.Add(TAssign(C1, C2, Sim, ClusterV[C1].GetItemVIdV(), ClusterV[C2].GetItemVIdV()));
			ClusterV[C1].Merge(ClusterV[C2], Sim);
			ClusterV[C2].SetMerged();			
			ClusterV[C1].ClearQueue();			

			for (int ItN2 = 0; ItN2 < ClusterV.Len(); ItN2++) {				
				if (ClusterV[ItN2].IsMerged() || ItN2 == C1) { continue; }

				double C1N = ClusterV[C1].GetItems();
				double C2N = ClusterV[ItN2].GetItems();				
				Sim = TSimProvider::GetSim(
					ClusterV[C1].GetVectSum(), ClusterV[ItN2].GetVectSum(), 
					C1N * C2N);
				DCSim[C1][ItN2] = DCSim[ItN2][C1] = Sim;

				ClusterV[C1].AddToQueue(Sim, ItN2);
				ClusterV[ItN2].DelAddQueue(Sim, C1);				
				ClusterV[ItN2].DelQueue(C2);					
			}	

			for (int i = 0; i < GetItems(); i++) {
				if (ClusterV[i].IsMerged()) { continue; }
				ClusterV[i].SortQueue();			
			}
		}
	}

public:	
	// the Hierarchical Agglomerative Clusrting Object constructor
	THrchAggClust() : DCSim() {};

	void AggClusterAssgn(const TMatrix *ItemM) { 
		Init(ItemM);
		ComputeSimMatrix(); 
		CreateDendogram(); 
		ClusterV.Clr(true);
		DCSim.Clr(true);
	}

	/* Cut at a pre-specified number of clusters. */
	void GetClusters(const int ClustN, TVec<TIntV>& RecNV) {
		int c1, c2, cnt = 0; 
		int AssignmentN = AssignV.Len();
		TIntV AddedV(ItemN); 	
		const int ReqAss = ItemN - ClustN;
		for (int AssN = ReqAss; AssN < AssignmentN; AssN++) {
			c1 = AssignV[AssN].GetC1Id();
			c2 = AssignV[AssN].GetC2Id();
			if (!AddedV[c1]) { 
				RecNV[cnt++].AddV(AssignV[AssN].GetC1IdV()); AddedV[c1] = 1; 
			}
			if (!AddedV[c2]) { 
				RecNV[cnt++].AddV(AssignV[AssN].GetC2IdV()); AddedV[c2] = 1; 
			}			 			
		}							
	}

	/* 
	* Cut at a prespecified level of similarity. For example, we cut the dendrogram 
	* at 0.4 if we want clusters with a minimum combination similarity of 0.4. 
	*/
	void GetClusters(const double Sim, TVec<TIntV>& RecNV) {
		int c1 = 0, c2 = 0; 
		int AssignmentN = AssignV.Len();
		TIntV AddedV(ItemN); 	
		for (int AssN = 0; AssN < AssignmentN; AssN++) {
			if (AssignV[AssN].GetSimilarity() >= Sim) { continue; }
			c1 = AssignV[AssN].GetC1Id();
			c2 = AssignV[AssN].GetC2Id();
			if (!AddedV[c1]) { 
				RecNV.Add(AssignV[AssN].GetC1IdV()); AddedV[c1] = 1; 
			}
			if (!AddedV[c2]) { 
				RecNV.Add(AssignV[AssN].GetC2IdV()); AddedV[c2] = 1; 
			}			 			
		}				
	}

	/* Cut the dendrogram where the gap between two successive combination similarities is largest. */
	void GetClusters(TVec<TIntV>& RecNV) {
		int c1 = 0, c2 = 0; 
		int AssignmentN = AssignV.Len();
		
		TFltIntKdV SimDifV;
		for (int AssN = 0; AssN < AssignmentN - 1; AssN++) {
			TFlt AbsDiff = fabs(AssignV[AssN].GetSimilarity() - AssignV[AssN + 1].GetSimilarity());
			SimDifV.Add(TFltIntKd(AbsDiff, AssN + 1));
		} SimDifV.Sort(false);
//		for (int RowN = 0; RowN < SimDifV.Len(); RowN++){
//			printf("%f %d ", SimDifV[RowN].Key.Val, SimDifV[RowN].Dat.Val);
//		}
//		printf("\n");

		int AssStN = SimDifV[0].Dat;

		TIntV AddedV(ItemN); 	
		for (int AssN = AssStN; AssN < AssignmentN; AssN++) {			
			c1 = AssignV[AssN].GetC1Id();
			c2 = AssignV[AssN].GetC2Id();
			if (!AddedV[c1]) { 
				RecNV.Add(AssignV[AssN].GetC1IdV()); AddedV[c1] = 1; 
			}
			if (!AddedV[c2]) { 
				RecNV.Add(AssignV[AssN].GetC2IdV()); AddedV[c2] = 1; 
			}			 			
		}					
	}


	int GetItems() const { return ItemN; }	

};

template class THrchAggClust<TIntFltKdV, TSparseColMatrix, TLinAlg, THrchAggClustSim::TCosine<TIntFltKdV, TLinAlg> >;
template class THrchAggClust<TFltV, TFullColMatrix, TLinAlg, THrchAggClustSim::TCosine<TFltV, TLinAlg> >;

typedef THrchAggClust<TIntFltKdV, TSparseColMatrix, TLinAlg, THrchAggClustSim::TCosine<TIntFltKdV, TLinAlg>> TSparseHrchAggClust;
typedef THrchAggClust<TFltV, TFullColMatrix, TLinAlg, THrchAggClustSim::TCosine<TFltV, TLinAlg> > TDenseHrchAggClust;

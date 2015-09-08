/////////////////////////////////////////////////
// Community detection algorithms
namespace TSnap {


namespace TSnapDetail {
// GIRVAN-NEWMAN algorithm
//	1. The betweenness of all existing edges in the network is calculated first.
//	2. The edge with the highest betweenness is removed.
//	3. The betweenness of all edges affected by the removal is recalculated.
//	4. Steps 2 and 3 are repeated until no edges remain.
//  Girvan M. and Newman M. E. J., Community structure in social and biological networks, Proc. Natl. Acad. Sci. USA 99, 7821-7826 (2002)
// Keep removing edges from Graph until one of the connected components of Graph splits into two.
void CmtyGirvanNewmanStep(PUNGraph& Graph, TIntV& Cmty1, TIntV& Cmty2) {
  TIntPrFltH BtwEH;
  TBreathFS<PUNGraph> BFS(Graph);
  Cmty1.Clr(false);  Cmty2.Clr(false);
  while (true) {
    TSnap::GetBetweennessCentr(Graph, BtwEH);
    BtwEH.SortByDat(false);
    if (BtwEH.Empty()) { return; }
    const int NId1 = BtwEH.GetKey(0).Val1;
    const int NId2 = BtwEH.GetKey(0).Val2;
    Graph->DelEdge(NId1, NId2);
    BFS.DoBfs(NId1, true, false, NId2, TInt::Mx);
    if (BFS.GetHops(NId1, NId2) == -1) { // two components
      TSnap::GetNodeWcc(Graph, NId1, Cmty1);
      TSnap::GetNodeWcc(Graph, NId2, Cmty2);
      return;
    }
  }
}

// Connected components of a graph define clusters
// OutDegH and OrigEdges stores node degrees and number of edges in the original graph
double _GirvanNewmanGetModularity(const PUNGraph& G, const TIntH& OutDegH, const int& OrigEdges, TCnComV& CnComV) {
  TSnap::GetWccs(G, CnComV); // get communities
  double Mod = 0;
  for (int c = 0; c < CnComV.Len(); c++) {
    const TIntV& NIdV = CnComV[c]();
    double EIn=0, EEIn=0;
    for (int i = 0; i < NIdV.Len(); i++) {
      TUNGraph::TNodeI NI = G->GetNI(NIdV[i]);
      EIn += NI.GetOutDeg();
      EEIn += OutDegH.GetDat(NIdV[i]);
    }
    Mod += (EIn-EEIn*EEIn/(2.0*OrigEdges));
  }
  if (Mod == 0) { return 0; }
  else { return Mod/(2.0*OrigEdges); }
}

TIntFltH MapEquationNew2Modules(PUNGraph& Graph, TIntH& Module, TIntFltH& Qi, int a, int b){
  TIntFltH Qi1;
  Qi1 = Qi;	
  float InModule=0.0, OutModule=0.0, Val;
  int Mds[2] = {a,b};
  for (int i=0; i<2; i++) {
    InModule=0.0, OutModule=0.0;
    if (Qi1.IsKey(Mds[i])){
      int CentralModule = Mds[i];
      for (TUNGraph::TEdgeI EI = Graph->BegEI(); EI < Graph->EndEI(); EI++) {
        if (Module.GetDat(EI.GetSrcNId()) == Module.GetDat(EI.GetDstNId()) && Module.GetDat(EI.GetDstNId()) == CentralModule) {
          InModule += 1.0;
        } else if ((Module.GetDat(EI.GetSrcNId()) == CentralModule && Module.GetDat(EI.GetDstNId()) != CentralModule) || (Module.GetDat(EI.GetSrcNId()) != CentralModule && Module.GetDat(EI.GetDstNId()) == CentralModule)) {
          OutModule +=1.0;
        }
	    }
      Val = 0.0;
      if (InModule+OutModule > 0) {
        Val = OutModule/(InModule+OutModule);
      }
      Qi1.DelKey(Mds[i]);
      Qi1.AddDat(Mds[i],Val);
    } else {
      Qi1.DelKey(Mds[i]);
      Qi1.AddDat(Mds[i],0.0);
    }
  }
	
  return Qi1;
}

float Equation(PUNGraph& Graph, TIntFltH& PAlpha,float& SumPAlphaLogPAlpha, TIntFltH& Qi){
  float SumPAlpha = 1.0, SumQi = 0.0, SumQiLogQi=0.0, SumQiSumPAlphaLogQiSumPAlpha = 0.0;
  for (int i=0; i<Qi.Len(); i++) {
    SumQi += Qi[i];
    SumQiLogQi += Qi[i]*log(Qi[i]);
    SumQiSumPAlphaLogQiSumPAlpha += (Qi[i]+SumPAlpha)*log(Qi[i]+SumPAlpha);
  }
  return (SumQi*log(SumQi)-2*SumQiLogQi-SumPAlphaLogPAlpha+SumQiSumPAlphaLogQiSumPAlpha);
}

} // namespace TSnapDetail

// Maximum modularity clustering by Girvan-Newman algorithm (slow)
//  Girvan M. and Newman M. E. J., Community structure in social and biological networks, Proc. Natl. Acad. Sci. USA 99, 7821-7826 (2002)
double CommunityGirvanNewman(PUNGraph& Graph, TCnComV& CmtyV) {
  TIntH OutDegH;
  const int NEdges = Graph->GetEdges();
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    OutDegH.AddDat(NI.GetId(), NI.GetOutDeg());
  }
  double BestQ = -1; // modularity
  TCnComV CurCmtyV;
  CmtyV.Clr();
  TIntV Cmty1, Cmty2;
  while (true) {
    TSnapDetail::CmtyGirvanNewmanStep(Graph, Cmty1, Cmty2);
    const double Q = TSnapDetail::_GirvanNewmanGetModularity(Graph, OutDegH, NEdges, CurCmtyV);
    //printf("current modularity: %f\n", Q);
    if (Q > BestQ) {
      BestQ = Q; 
      CmtyV.Swap(CurCmtyV);
    }
    if (Cmty1.Len()==0 || Cmty2.Len() == 0) { break; }
  }
  return BestQ;
}

// Rosvall-Bergstrom community detection algorithm based on information theoretic approach.
// See: Rosvall M., Bergstrom C. T., Maps of random walks on complex networks reveal community structure, Proc. Natl. Acad. Sci. USA 105, 1118-1123 (2008)
double Infomap(PUNGraph& Graph, TCnComV& CmtyV){	
  TIntH DegH; 
  TIntFltH PAlpha; // probability of visiting node alpha
  TIntH Module; // module of each node
  TIntFltH Qi; // probability of leaving each module
  float SumPAlphaLogPAlpha = 0.0;
  int Br = 0;
  const int e = Graph->GetEdges(); 

  // initial values
  for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
    DegH.AddDat(NI.GetId(), NI.GetDeg());
    float d = ((float)NI.GetDeg()/(float)(2*e));
    PAlpha.AddDat(NI.GetId(), d);
    SumPAlphaLogPAlpha += d*log(d);
    Module.AddDat(NI.GetId(),Br);
    Qi.AddDat(Module[Br],1.0);
    Br+=1;
  }

  float MinCodeLength = TSnapDetail::Equation(Graph,PAlpha,SumPAlphaLogPAlpha,Qi);
  float NewCodeLength, PrevIterationCodeLength = 0.0;
  int OldModule, NewModule;

  do {
    PrevIterationCodeLength = MinCodeLength;
      for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
        MinCodeLength = TSnapDetail::Equation(Graph, PAlpha, SumPAlphaLogPAlpha, Qi);
        for (int i=0; i<DegH.GetDat(NI.GetId()); i++) {
          OldModule = Module.GetDat(NI.GetId());
          NewModule = Module.GetDat(NI.GetNbrNId(i));
          if (OldModule!=NewModule){
            Module.DelKey(NI.GetId()); 
            Module.AddDat(NI.GetId(),NewModule);
            Qi = TSnapDetail::MapEquationNew2Modules(Graph,Module,Qi,OldModule, NewModule);
            NewCodeLength = TSnapDetail::Equation(Graph,PAlpha,SumPAlphaLogPAlpha, Qi);
            if (NewCodeLength<MinCodeLength) {
              MinCodeLength = NewCodeLength;
              OldModule = NewModule;
            } else {
              Module.DelKey(NI.GetId());
              Module.AddDat(NI.GetId(),OldModule);
            }
          }
       }
     }
   } while (MinCodeLength<PrevIterationCodeLength);

  Module.SortByDat(true);
  int Mod=-1;
  for (int i=0; i<Module.Len(); i++) {
    if (Module[i]>Mod){
      Mod = Module[i];
      TCnCom t;
      for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
        if (Module.GetDat(NI.GetId())==Mod)
        t.Add(NI.GetId());
      }
      CmtyV.Add(t);
    }
  }

  return MinCodeLength;
}

void CmtyEvolutionFolderBatch(TStr InFNm, TIntIntHH& sizesCont, TIntIntHH& cCont, TIntIntVH& edges, double alpha, double beta, int CmtyAlg) {

  // getting all the files with the extension .edg in the input folder
  HANDLE hFind;
  WIN32_FIND_DATA data;
  char * folder = new char[strlen(InFNm.CStr())+strlen("*.edg")];
  sprintf(folder,"%s%s",InFNm.CStr(),"*.edg");
  hFind = FindFirstFile(folder, &data);


  // reading folder with networks and calculating core/periphery
  if (hFind != INVALID_HANDLE_VALUE) {
    
	// counter is used for counting time points (e.g year)
	int br=0;

	// container of communities
	TIntIntH prev;

	// temporal container for sizes of communities in t-1 timepoint 
	TIntH prev_sizes;

	// temporal container for edges
	TIntIntVH edges_;

	// iterate all the files
    do {
	  // working with the file
      printf("%s\n", data.cFileName);

	  // WORK
	  
	  // creating path to the file - connecting path with the file name
	  char * newfile = new char[strlen(InFNm.CStr())+strlen(data.cFileName)];
	  sprintf(newfile,"%s%s",InFNm.CStr(),data.cFileName);

	  // loading graph from the file
	  PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(newfile, false);

	  // remove self edges from the graph
	  TSnap::DelSelfEdges(Graph);

	  // creating temporal (for the curent file/year/time_point) container for communities - generic for all community detection algorithms in Snap
      TCnComV CmtyV;

	  // modularity measure
      double Q = 0.0;
      
	  // for community detection algorithm name
	  TStr CmtyAlgStr;
	  
	  // depending on the input parameter, choose commnity detection algorithm, compute communities and store it into CmtyV container
      if (CmtyAlg == 1) {
        CmtyAlgStr = "Girvan-Newman";
        Q = TSnap::CommunityGirvanNewman(Graph, CmtyV); }
      else if (CmtyAlg == 2) {
        CmtyAlgStr = "Clauset-Newman-Moore";
        Q = TSnap::CommunityCNM(Graph, CmtyV); }
      else if (CmtyAlg == 3) {
        CmtyAlgStr = "Infomap";
        Q = TSnap::Infomap(Graph, CmtyV); }
      else { Fail; }

	  // temporal (current file/year/time_point) container for distribution of communities from previous time point in the current community
	  TIntIntHH distCont;

	  // if it is the first time point, store the communities and their sizes as they are
	  if (br == 0) {
	    prev.Clr();
		int size = 0;
		for (int c = 0; c < CmtyV.Len(); c++) {
          for (int i = 0; i < CmtyV[c].Len(); i++)
		    prev.AddDat(CmtyV[c][i].Val,c);	
		  int s = CmtyV[c].Len();
		  prev_sizes.AddDat(c,CmtyV[c].Len());
        }
	  }

	  // othervise check the distributions of communities from previous time points (statH1 and statH2 statistics) for labeling
	  else {
		
		// container for distributions of comunities from t-1 in the current communities
		TIntH dist;
		// container for maping of labels in relation to communities from previous time point
		TIntH map;

		// determining first free id for a new community - first_new_c_id
		int first_new_c_id=-1;
		for (THashKeyDatI<TInt, TInt> it = prev_sizes.BegI();  !it.IsEnd(); it++)
		  if (it.GetKey() > first_new_c_id)
		    first_new_c_id = it.GetKey();
		
		if (CmtyV.Len()-1>first_new_c_id)
		  first_new_c_id = CmtyV.Len()-1;
		
		first_new_c_id++;

		
		// iterate for all the communities in the current time point
	    for (int c = 0; c < CmtyV.Len(); c++) {
		 
		  TIntV stat;
		  TIntFltH statH1;
		  TIntFltH statH2;
		  
		  // initialize distributions to 0
		  for (THashKeyDatI<TInt, TInt> it = prev_sizes.BegI();  !it.IsEnd(); it++)
			  dist.AddDat(it.GetKey(),0);
		  
		  // add size 0 for new nodes which are assigned to community -1
		  dist.AddDat(-1,0);

		  // iterate for all individuals in community c (at the current time point br)
		  int id, prev_comm, pre_val;
          for (int i = 0; i < CmtyV[c].Len(); i++) {
			  // community of the individual
			  id = CmtyV[c][i].Val;
			  
			  // assume it is a new individual, not existing in t-1
			  prev_comm=-1;

			  // check if the individual existed in t-1. in case it did get the community of it in t-1 (prev_comm container - (id_of_individual,community_id_in_t-1)) 
			  if (prev.IsKey(id))
				prev_comm = prev.GetDat(CmtyV[c][i].Val);

			  // add the t-1 community of the individual to stat
			  stat.Add(prev_comm);

			  // increase count for the t-1 community in dist container (comm_from_t-1, freq_of_t-1_comm_in_t)
			  pre_val = dist.GetDat(prev_comm);
			  dist.AddDat(prev_comm,pre_val+1);
          }

		  // sum of distributions of communities from t-1 in t - should (finally) sum to 1
		  double sumstat2=0;

		  // iterate all communities from t-1 that are in the current community in t
		  int k, d;
		  for (THashKeyDatI<TInt, TInt> it = dist.BegI();  !it.IsEnd(); it++) {
			  
			  // id of t-1 community
			  k = it.GetKey();
			  
			  // size of t-1 community (at t)
			  d = it.GetDat();

			  // if the current community contains community k
			  if (d > 0){

				  // if k has size recorded in t-1
				  if (prev_sizes.IsKey(k)){
					// d - size of k in t, prev_sizes.GetDat(k) - size of k in t-1
					double stat1_ = (double)d/(double)prev_sizes.GetDat(k);
					
					// stat1_ is percentage of k that went from t-1 to the current community
					statH1.AddDat(k, stat1_);
				  }

				  // stat2_ is the ratio that k has in the current community
				  double stat2_ = (double)d/(double)CmtyV[c].Len();
				  statH2.AddDat(k, stat2_);
				  sumstat2 += stat2_;

				  // edges
				  TIntV edge;
				  // storing edges temporaly for t. after the labeling is done (mapping) the node of the edge is renamed and added to the global edge container
				  edge.Add(k); edge.Add(c); edge.Add(d); edge.Add(br-1); edge.Add(br);
				  edges_.AddDat(edges_.Len()+1,edge);
			  }

			  // if almost 100% of content in the current community is inspected the iteration can stop
			  if (sumstat2 > 0.98) break;
		  }

		  // analyse the statistics
		  int n_of_c_greater_than_half=0;
		  int id_of_c_greater_than_half=-1;
		  TIntV ids_of_c_greater_than_half;
		  
		  // count how many communities has statH1 > alpha (e.g. 0.5) and store those communities
		  for (THashKeyDatI<TInt, TFlt> it = statH1.BegI();  !it.IsEnd(); it++){
			  if (it.GetDat()>alpha){
				id_of_c_greater_than_half = it.GetKey();
				ids_of_c_greater_than_half.Add(it.GetKey());
				n_of_c_greater_than_half++;
			  }
		  }
		  
		  // if this community is build of majority of one previous community and the other parts of the community are fractions of other communities smaller than half, the new community gets the label of the community in majority 
		  if (n_of_c_greater_than_half == 1){
			  map.AddDat(c, id_of_c_greater_than_half);
		  }
		  // if there are more communities with major parts from t-1
		  else{
		    int h2part_id = -2;
			// iterate those communities
			for (int i=0; i<ids_of_c_greater_than_half.Len(); i++){
			  double H2 = statH2.GetDat(ids_of_c_greater_than_half[i]);
		      // if some of such communites has major part (>beta, e.g. 60%) of the current community, it gets its name
			  if (H2>beta){
			    h2part_id = ids_of_c_greater_than_half[i];
		      }
			}
			// naming by the choosen 'major' community
			if (h2part_id!=-2)
			  map.AddDat(c, h2part_id);
			// no condition for transfering the community label was meet, so the community get a new label
			else{
			  map.AddDat(c,first_new_c_id);
			  first_new_c_id++;
			}
		  }

		  // dist - V[(community_id_in_t-1, size)]
		  // distcont - V[(community_id_in_t, dist)]
		  distCont.AddDat(c,dist);
		 
		 }

        // save the sizes of the community with resolved labeling - used in t+1
		prev.Clr();
		prev_sizes.Clr();
		for (int c = 0; c < CmtyV.Len(); c++){
            for (int i = 0; i < CmtyV[c].Len(); i++){
				prev.AddDat(CmtyV[c][i].Val,map.GetDat(c));
			}
			int s = CmtyV[c].Len();
			prev_sizes.AddDat(map.GetDat(c),CmtyV[c].Len());
		}

		// filing the edges container - the key thing is the map(c)
		for (THashKeyDatI<TInt, TIntV> it = edges_.BegI();  !it.IsEnd(); it++){
		  TIntV edgesV;
		  edgesV.Add(map.GetDat(it.GetDat()[1]));
		  edgesV.Add(it.GetDat()[0]);
		  edgesV.Add(it.GetDat()[2]);
		  edgesV.Add(it.GetDat()[3]);
		  edgesV.Add(it.GetDat()[4]);
		  if (it.GetDat()[0] != -1)
		    edges.AddDat(edges.Len(),edgesV);
		  }
		  edges_.Clr();
	  }

	  sizesCont.AddDat(br,prev_sizes);
	  cCont.AddDat(br,prev);
	  br++;

	  // WORK - END

  } while (FindNextFile(hFind, &data));
  FindClose(hFind);
  }
}

void CmtyEvolutionFolderBatch(TStr InFNm, TIntIntVH& sizesContV, TIntIntVH& cContV, TIntIntVH& edges, double alpha, double beta, int CmtyAlg) {
	TIntIntHH sizesCont;
	TIntIntHH cCont;
	CmtyEvolutionFolderBatch(InFNm,sizesCont, cCont, edges, alpha, beta, CmtyAlg);

	TIntV uniqueId;
	for (int i=0; i < cCont.Len(); i++){
		for (THashKeyDatI<TInt, TInt> it = cCont[i].BegI();  !it.IsEnd(); it++){
			if (!uniqueId.IsIn(it.GetKey()))
				uniqueId.Add(it.GetKey());
		}
	}

	for (int j=0; j<uniqueId.Len(); j++)
	{
		TIntV cV;
		for (int i=0; i<cCont.Len(); i++)
		{
			if (cCont[i].IsKey(uniqueId[j]))
				cV.Add(cCont[i].GetDat(uniqueId[j]));
			else
				cV.Add(-1);
		}
		cContV.AddDat(uniqueId[j],cV);
	}

	TIntV uniqueC;
	for (int i=0; i < sizesCont.Len(); i++){
		for (THashKeyDatI<TInt, TInt> it = sizesCont[i].BegI();  !it.IsEnd(); it++){
			if (!uniqueC.IsIn(it.GetKey()))
				uniqueC.Add(it.GetKey());
		}
	}

	for (int j=0; j<uniqueC.Len(); j++)
	{
		TIntV cV;
		for (int i=0; i<sizesCont.Len(); i++)
		{
			if (sizesCont[i].IsKey(uniqueC[j]))
				cV.Add(sizesCont[i].GetDat(uniqueC[j]));
			else
				cV.Add(0);
		}
		sizesContV.AddDat(uniqueC[j], cV);
	}

}

void CmtyEvolutionFileBatch(TStr InFNm, TIntIntVH& sizesContV, TIntIntVH& cContV, TIntIntVH& edges, double alpha, double beta, int CmtyAlg) {
	TIntIntHH sizesCont;
	TIntIntHH cCont;
	CmtyEvolutionFileBatch(InFNm,sizesCont, cCont, edges, alpha, beta, CmtyAlg);

	TIntV uniqueId;
	for (int i=0; i < cCont.Len(); i++){
		for (THashKeyDatI<TInt, TInt> it = cCont[i].BegI();  !it.IsEnd(); it++){
			if (!uniqueId.IsIn(it.GetKey()))
				uniqueId.Add(it.GetKey());
		}
	}

	for (int j=0; j<uniqueId.Len(); j++)
	{
		TIntV cV;
		for (int i=0; i<cCont.Len(); i++)
		{
			if (cCont[i].IsKey(uniqueId[j]))
				cV.Add(cCont[i].GetDat(uniqueId[j]));
			else
				cV.Add(-1);
		}
		cContV.AddDat(uniqueId[j],cV);
	}

	TIntV uniqueC;
	for (int i=0; i < sizesCont.Len(); i++){
		for (THashKeyDatI<TInt, TInt> it = sizesCont[i].BegI();  !it.IsEnd(); it++){
			if (!uniqueC.IsIn(it.GetKey()))
				uniqueC.Add(it.GetKey());
		}
	}

	for (int j=0; j<uniqueC.Len(); j++)
	{
		TIntV cV;
		for (int i=0; i<sizesCont.Len(); i++)
		{
			if (sizesCont[i].IsKey(uniqueC[j]))
				cV.Add(sizesCont[i].GetDat(uniqueC[j]));
			else
				cV.Add(0);
		}
		sizesContV.AddDat(uniqueC[j], cV);
	}

}

void CmtyEvolutionFileBatch(TStr InFNm, TIntIntHH& sizesCont, TIntIntHH& cCont, TIntIntVH& edges, double alpha, double beta, int CmtyAlg) {


  // reading folder with networks and calculating core/periphery
    int br=0;
	TIntIntH prev;
	TIntH prev_sizes;

	TSsParser Ss(InFNm, ssfWhiteSep, true, false, true);
	Ss.Next();
	int internal_year_counter = 0;
	// variable for delimiter between networks
	TStr Marker;
	// defining variables for node ids and starting year
	int SrcNId, DstNId, t=1970;

	// temporal container for edges
	TIntIntVH edges_;
	
	while (!Ss.Eof()) {
	  
	  //printf("%i\n", t);
	  Marker = Ss.GetLnStr();
	  // get the year from the network seperator
	  t = Marker.GetSubStr(1,4).GetInt();

	  if (Marker.GetCh(0) == '#'){

	  Ss.Next();
	  PUNGraph Graph = PUNGraph::TObj::New();
	  do{
	 	if (! Ss.GetInt(0, SrcNId) || ! Ss.GetInt(1, DstNId)) {
			if (!Ss.Eof()){ 
				Ss.Next(); 
				if (!Ss.Eof()) 
					Marker = Ss.GetLnStr();
			}
			continue; 
	    }
	    if (! Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
	    if (! Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
	    Graph->AddEdge(SrcNId, DstNId);
	    Ss.Next();
	    if (!Ss.Eof())
	    Marker = Ss.GetLnStr();
      }while(Marker.GetCh(0)!='#' && !Ss.Eof());

	  
	  if (Graph->GetNodes()>0) {
	  // WORK
	  
	  TSnap::DelSelfEdges(Graph);
      TCnComV CmtyV;
      double Q = 0.0;
      TStr CmtyAlgStr;
      if (CmtyAlg == 1) {
        CmtyAlgStr = "Girvan-Newman";
        Q = TSnap::CommunityGirvanNewman(Graph, CmtyV); }
      else if (CmtyAlg == 2) {
        CmtyAlgStr = "Clauset-Newman-Moore";
        Q = TSnap::CommunityCNM(Graph, CmtyV); }
      else if (CmtyAlg == 3) {
        CmtyAlgStr = "Infomap";
        Q = TSnap::Infomap(Graph, CmtyV); }
      else { Fail; }

	  TIntIntHH distCont;

	  if (br == 0) {
		  prev.Clr();
		  int size = 0;
		  for (int c = 0; c < CmtyV.Len(); c++) {
            for (int i = 0; i < CmtyV[c].Len(); i++){
				prev.AddDat(CmtyV[c][i].Val,c);
			}
			int s = CmtyV[c].Len();
			prev_sizes.AddDat(c,CmtyV[c].Len());
          }
	  }
	  else {
		
		// containers for statistics
		
		//TIntFltHH stat1;
		//TIntIntHH stat2;
		TIntH dist;
		TIntH map;

		int first_new_c_id=-1;
		
		// getting first free id for a new community
		for (THashKeyDatI<TInt, TInt> it = prev_sizes.BegI();  !it.IsEnd(); it++)
			if (it.GetKey() > first_new_c_id)
				first_new_c_id = it.GetKey();
		if (CmtyV.Len()-1>first_new_c_id)
			first_new_c_id = CmtyV.Len()-1;
		first_new_c_id++;

	    for (int c = 0; c < CmtyV.Len(); c++) {
		 
		  TIntV stat;
		  TIntFltH statH1;
		  TIntFltH statH2;
		  
		  // initialize distributions to 0
		  for (THashKeyDatI<TInt, TInt> it = prev_sizes.BegI();  !it.IsEnd(); it++)
			  dist.AddDat(it.GetKey(),0);
		  //for new nodes
		  dist.AddDat(-1,0);

          for (int i = 0; i < CmtyV[c].Len(); i++) {
			  int id = CmtyV[c][i].Val;
			  int prev_comm=-1;
			  if (prev.IsKey(id))
				prev_comm = prev.GetDat(CmtyV[c][i].Val);
			  stat.Add(prev_comm);
			  int pre_val = dist.GetDat(prev_comm);
			  dist.AddDat(prev_comm,pre_val+1);
          }

		  double sumstat2=0;
		  for (THashKeyDatI<TInt, TInt> it = dist.BegI();  !it.IsEnd(); it++) {
				
			  int k = it.GetKey();
			  int d = it.GetDat();
			  if (d > 0){
				  if (prev_sizes.IsKey(it.GetKey())){
				
					double stat1_ = (double)d/(double)prev_sizes.GetDat(k);
					statH1.AddDat(k, stat1_);
				  }
				  double stat2_ = (double)d/(double)CmtyV[c].Len();
				  statH2.AddDat(k, stat2_);
				  sumstat2 += stat2_;

				  TIntV edge;
				  edge.Add(k);
				  edge.Add(c);
				  edge.Add(d);
				  edge.Add(br-1);
				  edge.Add(br);
				  edges_.AddDat(edges_.Len()+1,edge);
			  }

			  // adding edges between two communities in two neighbouring time points;
			  

			  if (sumstat2 > 0.98) break;
		  }

		  int n_of_c_greater_than_half=0;
		  int id_of_c_greater_than_half=-1;
		  TIntV ids_of_c_greater_than_half;
		  
		  for (THashKeyDatI<TInt, TFlt> it = statH1.BegI();  !it.IsEnd(); it++){
			  if (it.GetDat()>alpha){
				id_of_c_greater_than_half = it.GetKey();
				ids_of_c_greater_than_half.Add(it.GetKey());
				n_of_c_greater_than_half++;
			  }
		  }
		  
		  // if this community is build of majority of one previous community and the other parts of the community are fractions of other communities smaller than half, the new community gets its label 
		  if (n_of_c_greater_than_half == 1){
			  map.AddDat(c, id_of_c_greater_than_half);
		  }
		  else{
			  int h2part_id = -2;
			  for (int i=0; i<ids_of_c_greater_than_half.Len(); i++){
				  double H2 = statH2.GetDat(ids_of_c_greater_than_half[i]);
				  if (H2>beta){
					h2part_id = ids_of_c_greater_than_half[i];
				  }
			  }
			  if (h2part_id!=-2)
				  map.AddDat(c, h2part_id);
			  else{
				  map.AddDat(c,first_new_c_id);
				  first_new_c_id++;
			  }
		  }

		  distCont.AddDat(c,dist);

		  //stat1.AddDat(c,statH1);
		  //stat2.AddDat(c,statH2);
		
		}

		
		prev.Clr();
		prev_sizes.Clr();
		for (int c = 0; c < CmtyV.Len(); c++){
            for (int i = 0; i < CmtyV[c].Len(); i++){
				prev.AddDat(CmtyV[c][i].Val,map.GetDat(c));
			}
			int s = CmtyV[c].Len();
			prev_sizes.AddDat(map.GetDat(c),CmtyV[c].Len());
		}

		// filing the edges container - the key thing is the map(c)
		for (THashKeyDatI<TInt, TIntV> it = edges_.BegI();  !it.IsEnd(); it++){
			TIntV edgesV;
			int a = it.GetDat()[0];
			int b = it.GetDat()[1];
			int v = it.GetDat()[2];
			int d = it.GetDat()[3];
			int e = it.GetDat()[4];
			edgesV.Add(map.GetDat(b));
			edgesV.Add(a);
			edgesV.Add(v);
			edgesV.Add(d);
			edgesV.Add(e);
			if (a != -1)
				edges.AddDat(edges.Len(),edgesV);
		}
		edges_.Clr();

		
	  }

	  sizesCont.AddDat(br,prev_sizes);
	  cCont.AddDat(br,prev);
	  br++;
	  // WORK - END
	  }
	}
	else Ss.Next();
	}
  
}

void CmtyEvolutionJson(TStr& Json, TIntIntVH& sizesContV, TIntIntVH& cContV, TIntIntVH& edges){
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // This function creates a JSON string with communities and edges for community evolution visualization using D3.js
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
  // writing json label for edges
  Json.InsStr(Json.Len(),"{\n\"edges\":[\n");
  
  TInt br=0;
  // iterating hash of vector of edges and writing into string 
  for (THashKeyDatI<TInt, TIntV> it = edges.BegI();  !it.IsEnd(); it++)
  {
    // first node
	TInt n1 = it.GetDat()[1];
	// second node
	TInt n2 = it.GetDat()[0];
	// edge weight
	TInt w = it.GetDat()[2];
	// start time point
	TInt t0 = it.GetDat()[3];
	// end time point
	TInt t1 = it.GetDat()[4];

	if (br>0)
		Json.InsStr(Json.Len(),",");

	// writing to string
	Json.InsStr(Json.Len(),"{\"n1\":"); Json.InsStr(Json.Len(),n1.GetStr()); 
	Json.InsStr(Json.Len(),", \"n2\":"); Json.InsStr(Json.Len(),n2.GetStr());
	Json.InsStr(Json.Len(),", \"w\":"); Json.InsStr(Json.Len(),w.GetStr());
	Json.InsStr(Json.Len(),", \"t0\":"); Json.InsStr(Json.Len(),t0.GetStr());
	Json.InsStr(Json.Len(),", \"t1\":"); Json.InsStr(Json.Len(),t1.GetStr());
	Json.InsStr(Json.Len()," }\n");
	br++;
  }

  // json label for communities
  Json.InsStr(Json.Len(),"],\n\"communities\":[\n");

  br=0;
  // printing communities into json file 
  for (int i=0; i < sizesContV[0].Len(); i++)
  {
	  for (THashKeyDatI<TInt, TIntV> it = sizesContV.BegI();  !it.IsEnd(); it++)
	  {
		  // id of community
		  TInt id = it.GetKey();
		  // community size
		  TInt size = it.GetDat()[i];
		  // time
		  TInt j = i;
		  
		  // if the community has size greater than 0, output it to json string
		  if (size > 0) {
			if(br>0)
			  Json.InsStr(Json.Len(),",");

			TInt size = it.GetDat()[i];
			Json.InsStr(Json.Len(),"{\"id\":"); Json.InsStr(Json.Len(),id.GetStr()); 
			Json.InsStr(Json.Len(),", \"size\":"); Json.InsStr(Json.Len(),size.GetStr());
			Json.InsStr(Json.Len(),", \"t\":"); Json.InsStr(Json.Len(),j.GetStr());
			Json.InsStr(Json.Len()," }\n");

			br++;
		  }
	  }
  }

  // printing communities into json file - alternative ordering
  /*
  for (THashKeyDatI<TInt, TIntV> it = sizesContV.BegI();  !it.IsEnd(); it++)
  {
	  TInt id = it.GetKey();
	  int len = it.GetDat().Len();
	  for (int i=0; i < it.GetDat().Len(); i++)
	  {	
		  TInt size = it.GetDat()[i];
		  TInt j = i;
		  if (size > 0) {

			if(br>0)
			  Json.InsStr(Json.Len(),",");

			TInt size = it.GetDat()[i];

			Json.InsStr(Json.Len(),"{\"id\":"); Json.InsStr(Json.Len(),id.GetStr()); 
			Json.InsStr(Json.Len(),", \"size\":"); Json.InsStr(Json.Len(),size.GetStr());
			Json.InsStr(Json.Len(),", \"t\":"); Json.InsStr(Json.Len(),j.GetStr());
			Json.InsStr(Json.Len()," }\n");

			br++;

		  }

	  }
  }
  */

  Json.InsStr(Json.Len(),"]\n}");

}

namespace TSnapDetail {
/// Clauset-Newman-Moore community detection method.
/// At every step two communities that contribute maximum positive value to global modularity are merged.
/// See: Finding community structure in very large networks, A. Clauset, M.E.J. Newman, C. Moore, 2004
class TCNMQMatrix {
private:
  struct TCmtyDat {
    double DegFrac;
    TIntFltH NIdQH;
    int MxQId;
    TCmtyDat() : MxQId(-1) { }
    TCmtyDat(const double& NodeDegFrac, const int& OutDeg) : 
      DegFrac(NodeDegFrac), NIdQH(OutDeg), MxQId(-1) { }
    void AddQ(const int& NId, const double& Q) { NIdQH.AddDat(NId, Q);
      if (MxQId==-1 || NIdQH[MxQId]<Q) { MxQId=NIdQH.GetKeyId(NId); } }
    void UpdateMaxQ() { MxQId=-1; 
      for (int i = -1; NIdQH.FNextKeyId(i); ) { 
        if (MxQId==-1 || NIdQH[MxQId]< NIdQH[i]) { MxQId=i; } } }
    void DelLink(const int& K) { const int NId=GetMxQNId(); 
      NIdQH.DelKey(K); if (NId==K) { UpdateMaxQ(); }  }
    int GetMxQNId() const { return NIdQH.GetKey(MxQId); }
    double GetMxQ() const { return NIdQH[MxQId]; }
  };
private:
  THash<TInt, TCmtyDat> CmtyQH;
  THeap<TFltIntIntTr> MxQHeap;
  TUnionFind CmtyIdUF;
  double Q;
public:
  TCNMQMatrix(const PUNGraph& Graph) : CmtyQH(Graph->GetNodes()), 
    MxQHeap(Graph->GetNodes()), CmtyIdUF(Graph->GetNodes()) { Init(Graph); }
  void Init(const PUNGraph& Graph) {
    const double M = 0.5/Graph->GetEdges(); // 1/2m
    Q = 0.0;
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      CmtyIdUF.Add(NI.GetId());
      const int OutDeg = NI.GetOutDeg();
      if (OutDeg == 0) { continue; }
      TCmtyDat& Dat = CmtyQH.AddDat(NI.GetId(), TCmtyDat(M * OutDeg, OutDeg));
      for (int e = 0; e < NI.GetOutDeg(); e++) {
        const int DstNId = NI.GetOutNId(e);
        const double DstMod = 2 * M * (1.0 - OutDeg * Graph->GetNI(DstNId).GetOutDeg() * M);
        Dat.AddQ(DstNId, DstMod);
      }
      Q += -1.0*TMath::Sqr(OutDeg*M);
      if (NI.GetId() < Dat.GetMxQNId()) {
        MxQHeap.Add(TFltIntIntTr(Dat.GetMxQ(), NI.GetId(), Dat.GetMxQNId())); }
    }
    MxQHeap.MakeHeap();
  }
  TFltIntIntTr FindMxQEdge() {
    while (true) {
      if (MxQHeap.Empty()) { break; }
      const TFltIntIntTr TopQ = MxQHeap.PopHeap();
      if (! CmtyQH.IsKey(TopQ.Val2) || ! CmtyQH.IsKey(TopQ.Val3)) { continue; }
      if (TopQ.Val1!=CmtyQH.GetDat(TopQ.Val2).GetMxQ() && TopQ.Val1!=CmtyQH.GetDat(TopQ.Val3).GetMxQ()) { continue; }
      return TopQ;
    }
    return TFltIntIntTr(-1, -1, -1);
  }
  bool MergeBestQ() {
    const TFltIntIntTr TopQ = FindMxQEdge();
    if (TopQ.Val1 <= 0.0) { return false; }
    // joint communities
    const int I = TopQ.Val3;
    const int J = TopQ.Val2;
    CmtyIdUF.Union(I, J); // join
    Q += TopQ.Val1;
    TCmtyDat& DatJ = CmtyQH.GetDat(J);
    { TCmtyDat& DatI = CmtyQH.GetDat(I);
    DatI.DelLink(J);  DatJ.DelLink(I);
    for (int i = -1; DatJ.NIdQH.FNextKeyId(i); ) {
      const int K = DatJ.NIdQH.GetKey(i);
      TCmtyDat& DatK = CmtyQH.GetDat(K);
      double NewQ = DatJ.NIdQH[i];
      if (DatI.NIdQH.IsKey(K)) { NewQ = NewQ+DatI.NIdQH.GetDat(K);  DatK.DelLink(I); }     // K connected to I and J
      else { NewQ = NewQ-2*DatI.DegFrac*DatK.DegFrac; }  // K connected to J not I
      DatJ.AddQ(K, NewQ);
      DatK.AddQ(J, NewQ);
      MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J,K), TMath::Mx(J,K)));
    }
    for (int i = -1; DatI.NIdQH.FNextKeyId(i); ) {
      const int K = DatI.NIdQH.GetKey(i);
      if (! DatJ.NIdQH.IsKey(K)) { // K connected to I not J
        TCmtyDat& DatK = CmtyQH.GetDat(K);
        const double NewQ = DatI.NIdQH[i]-2*DatJ.DegFrac*DatK.DegFrac; 
        DatJ.AddQ(K, NewQ);
        DatK.DelLink(I);
        DatK.AddQ(J, NewQ);
        MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J,K), TMath::Mx(J,K)));
      }
    } 
    DatJ.DegFrac += DatI.DegFrac; }
    if (DatJ.NIdQH.Empty()) { CmtyQH.DelKey(J); } // isolated community (done)
    CmtyQH.DelKey(I);
    return true;
  }
  static double CmtyCMN(const PUNGraph& Graph, TCnComV& CmtyV) {
    TCNMQMatrix QMatrix(Graph);
    // maximize modularity
    while (QMatrix.MergeBestQ()) { }
    // reconstruct communities
    THash<TInt, TIntV> IdCmtyH;
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      IdCmtyH.AddDat(QMatrix.CmtyIdUF.Find(NI.GetId())).Add(NI.GetId()); 
    }
    CmtyV.Gen(IdCmtyH.Len());
    for (int j = 0; j < IdCmtyH.Len(); j++) {
      CmtyV[j].NIdV.Swap(IdCmtyH[j]);
    }
    return QMatrix.Q;
  }
};

} // namespace TSnapDetail

double CommunityCNM(const PUNGraph& Graph, TCnComV& CmtyV) {
  return TSnapDetail::TCNMQMatrix::CmtyCMN(Graph, CmtyV);
}

}; //namespace TSnap

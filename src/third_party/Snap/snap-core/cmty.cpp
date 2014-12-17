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
        double EIn = 0, EEIn = 0;
        for (int i = 0; i < NIdV.Len(); i++) {
          TUNGraph::TNodeI NI = G->GetNI(NIdV[i]);
          EIn += NI.GetOutDeg();
          EEIn += OutDegH.GetDat(NIdV[i]);
        }
        Mod += (EIn - EEIn*EEIn / (2.0*OrigEdges));
      }
      if (Mod == 0) { return 0; }
      else { return Mod / (2.0*OrigEdges); }
    }

		void MapEquationNew2Modules(PUNGraph& Graph, TIntH& Module, TIntFltH& Qi, int a, int b) {
			float InModule = 0.0, OutModule = 0.0, Val;
			int Mds[2] = { a, b };
			for (int i = 0; i<2; i++) {
				InModule = 0.0, OutModule = 0.0;
				if (Qi.IsKey(Mds[i])) {
					int CentralModule = Mds[i];

          //printf("central module: %i\n ",CentralModule);

          TIntV newM;
          for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
            if (Module.GetDat(NI.GetId()) == CentralModule)
              newM.Add(NI.GetId());
          }

					for (int j = 0; j<newM.Len(); j++) {
						//int len1 = newM.Len();

            //int c = CentralModule;
            for (int k = 0; k<Graph->GetNI(newM[j]).GetDeg(); k++) {
              //int len = Graph->GetNI(newM[j]).GetDeg();

              int ids = Graph->GetNI(newM[j]).GetId();
              int idd = Graph->GetNI(newM[j]).GetNbrNId(k);
              int ms = Module.GetDat(ids);
              int md = Module.GetDat(idd);
              //int c = CentralModule;

              if (ms == md) {
                InModule += 1.0;
                //printf("IN: \t%i - %i; moduls: %i - %i\n", ids, idd, ms, md);
              }
              else {
                OutModule += 1.0;
                //printf("OUT: \t%i - %i; moduls: %i - %i\n", ids, idd, ms, md);
              }
            }
          }
          if (InModule >1) InModule = InModule / 2;

          //printf("\n");

          Val = 0.0;
          if (InModule + OutModule > 0) {
            Val = OutModule / (InModule + OutModule);
          }
          //int control = Mds[i];
          Qi.AddDat(Mds[i], Val);
        }
        else {
          //int control = Mds[i];
          Qi.AddDat(Mds[i], 0.0);
        }
      }
    }

		double Equation(TIntFltH& PAlpha, double& SumPAlphaLogPAlpha, TIntFltH& Qi){
			double SumPAlpha = 1.0, SumQi = 0.0, SumQiLogQi = 0.0;
			double SumQiSumPAlphaLogQiSumPAlpha = 0.0, logqi = 0.0, qi = 0.0;
			for (int i = 0; i<Qi.Len(); i++) {
				SumQi += Qi[i];
				qi = Qi[i];
				if (qi != 0) {
					logqi = log(qi);
				}
				else {
					logqi = 0;
				}
				SumQiLogQi += Qi[i] * logqi;
				SumQiSumPAlphaLogQiSumPAlpha += (Qi[i] + SumPAlpha)*log(Qi[i] + SumPAlpha);
			}
			return (SumQi*log(SumQi) - 2 * SumQiLogQi - SumPAlphaLogPAlpha +
				SumQiSumPAlphaLogQiSumPAlpha);
		}

    bool edgeIntersect(PNGraph& graph, TIntV& a, TIntV& b) {
      for (int i = 0; i<a.Len(); i++) {
        for (int j = 0; j<b.Len(); j++) {
          if (graph->IsEdge(a[i], b[j]))
            return true;
        }
      }

      return false;
    }

    int vectorIntersect(TIntV& a, TIntV& b) {
      int count = 0;
      for (int i = 0; i<a.Len(); i++) {
        for (int j = 0; j<b.Len(); j++) {
          if (a[i] == b[j])
            count++;
        }
      }
      return count;
    }

    bool inComp(PNGraph& g1, PNGraph& Graph, TIntH& inCompCount, int id, int neigh) {
      bool out = true;

      int inCompN = 0;
      int inComp = 0;

      if (g1->IsNode(id) && g1->IsNode(neigh)) {
        int deg = g1->GetNI(id).GetDeg();
        int neighDeg = g1->GetNI(neigh).GetDeg();


        if (inCompCount.IsKey(id)) {
          inComp = inCompCount.GetDat(id);
        }
        if (inCompCount.IsKey(neigh)) {
          inCompN = inCompCount.GetDat(neigh);
        }

        if (inCompN < neighDeg && inComp < deg && (!g1->IsNode(neigh) || Graph->GetNI(neigh).GetDeg() - neighDeg == 0)) {
          inCompCount.AddDat(neigh, ++inCompN);
          inCompCount.AddDat(id, ++inComp);
          out = true;
        }
        else {
          out = false;
        }
      }
      return out;
    }

    void transitiveTransform(TIntV& a, TIntV& b) {
      for (int i = 0; i < a.Len(); i++) {
        bool diff = false;
        for (int j = 0; j < b.Len(); j++) {
          if (a[i] == a[j]) {
            diff = true;
            break;
          }
        }
        if (!diff) {
          b.Add(a[i]);
          break;
        }
      }
    }

		bool checkIfCrossing(TIntV& a, TIntH& t, int f, int l, int TP) {
			bool after = false;
			bool before = false;
			for (int i = 0; i < a.Len(); i++) {
				if (t.GetDat(a[i]) < TP)
					before = true;
				if (t.GetDat(a[i]) > TP)
					after = true;
			}

      if (TP == f)
        before = true;

      if (TP == l)
        after = true;

      return (after && before);
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
			if (Cmty1.Len() == 0 || Cmty2.Len() == 0) { break; }
		}
		return BestQ;
	}

  // Rosvall-Bergstrom community detection algorithm based on information theoretic approach.
  // See: Rosvall M., Bergstrom C. T., Maps of random walks on complex networks reveal community structure, Proc. Natl. Acad. Sci. USA 105, 1118-1123 (2008)
  double Infomap(PUNGraph& Graph, TCnComV& CmtyV){

    TIntFltH PAlpha; // probability of visiting node alpha
    TIntH Module; // module of each node
    TIntFltH Qi; // probability of leaving each module

    double SumPAlphaLogPAlpha = 0.0;
    int Br = 0;
    const int e = Graph->GetEdges();

    // initial values
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++) {
      int nodeId = NI.GetId();
      int nodeDeg = NI.GetDeg();
      float d = ((float)nodeDeg / (float)(2 * e));
      PAlpha.AddDat(nodeId, d);
      SumPAlphaLogPAlpha += d*log(d);
      Module.AddDat(nodeId, Br);
      Qi.AddDat(Br, 1.0);
      Br += 1;
    }

    double MinCodeLength = TSnapDetail::Equation(PAlpha, SumPAlphaLogPAlpha, Qi);
    double NewCodeLength, PrevIterationCodeLength = 0.0;
    int OldModule, NewModule;

    TIntV nodes;
    for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++)
      nodes.Add(NI.GetId());

    do {
      PrevIterationCodeLength = MinCodeLength;
      TRnd rnd;
      rnd.Randomize();
      nodes.Shuffle(rnd);
      for (int ndcounter = 0; ndcounter<nodes.Len(); ndcounter++) {
        MinCodeLength = TSnapDetail::Equation(PAlpha, SumPAlphaLogPAlpha, Qi);
        int nodeId = nodes[ndcounter];
        TUNGraph::TNodeI NI = Graph->GetNI(nodeId);
        for (int i = 0; i<NI.GetDeg(); i++) {

          OldModule = Module.GetDat(nodeId);
          NewModule = Module.GetDat(NI.GetNbrNId(i));

          if (OldModule != NewModule){

            Module.AddDat(nodeId, NewModule);

            TSnapDetail::MapEquationNew2Modules(Graph, Module, Qi, OldModule, NewModule);
            NewCodeLength = TSnapDetail::Equation(PAlpha, SumPAlphaLogPAlpha, Qi);
            if (NewCodeLength<MinCodeLength) {
              MinCodeLength = NewCodeLength;
              OldModule = NewModule;
            }
            else {
              Module.AddDat(nodeId, OldModule);
            }
          }
        }
      }
    } while (MinCodeLength<PrevIterationCodeLength);

    Module.SortByDat(true);

    int Mod = -1;
    for (int i = 0; i<Module.Len(); i++) {
      if (Module[i]>Mod){
        Mod = Module[i];
        TCnCom t;
        for (TUNGraph::TNodeI NI = Graph->BegNI(); NI < Graph->EndNI(); NI++){
          if (Module.GetDat(NI.GetId()) == Mod)
            t.Add(NI.GetId());
        }
        CmtyV.Add(t);
      }
    }

    return MinCodeLength;
  }

	// Create community evolution directed graph
	void CmtyEvolutionBatchGraph(TVec<PUNGraph, TSize>& gs, PNGraph& graph, TIntH& t, TIntH& c, TIntH& s, TIntV& e, TIntIntVH& members, double alpha, double beta, int CmtyAlg) {
		int internal_year_counter = 0;
		int newcom = 0;
		TStr Marker = "";
		int SrcNId, DstNId;
		TIntIntVH CommsAtT;

		for (int gcounter = 0; gcounter < gs.Len(); gcounter++) {

			PUNGraph Graph = gs[gcounter];

			if (Graph->GetNodes()>0) {
				// WORK

				TSnap::DelSelfEdges(Graph);
				TCnComV CmtyV;
				double Q = 0.0;
				TStr CmtyAlgStr;
				if (CmtyAlg == 1) {
					CmtyAlgStr = "Girvan-Newman";
					Q = TSnap::CommunityGirvanNewman(Graph, CmtyV);
				}
				else if (CmtyAlg == 2) {
					CmtyAlgStr = "Clauset-Newman-Moore";
					Q = TSnap::CommunityCNM(Graph, CmtyV);
				}
				else if (CmtyAlg == 3) {
					CmtyAlgStr = "Infomap";
					Q = TSnap::Infomap(Graph, CmtyV);
				}
				else { Fail; }

				TIntV coms;
				for (int c = 0; c < CmtyV.Len(); c++) {
					TIntV ids;
					for (int i = 0; i < CmtyV[c].Len(); i++){
						ids.Add(CmtyV[c][i].Val);
					}
					members.AddDat(newcom, ids);
					t.AddDat(newcom, internal_year_counter);
					s.AddDat(newcom, CmtyV[c].Len());
					coms.Add(newcom);
					newcom++;
				}

				CommsAtT.AddDat(internal_year_counter, coms);
				internal_year_counter++;
			}

		} // end for end reading communities

		int newfreeid = 0;
		int br = 0;
		THashKeyDatI<TInt, TIntV> it = CommsAtT.BegI();

		while (!it.IsEnd() && br < CommsAtT.Len() - 1) {

			// Check if there is one more Time point after this one
			if (br < CommsAtT.Len() - 1) {
				TIntV commsA = it.GetDat();
				it++;
				TIntV commsB = it.GetDat();

				for (int i = 0; i<commsA.Len(); i++) {
					for (int j = 0; j<commsB.Len(); j++) {

						TIntV a = members.GetDat(commsA[i]);
						TIntV b = members.GetDat(commsB[j]);
						int intersect = TSnapDetail::vectorIntersect(a, b);
						if (intersect > 0) {
							if (!graph->IsNode(commsA[i]))
								graph->AddNode(commsA[i]);
							if (!graph->IsNode(commsB[j]))
								graph->AddNode(commsB[j]);
							graph->AddEdge(commsA[i], commsB[j]);
							e.Add(intersect);

							int sizeA = a.Len();
							int sizeB = b.Len();

							double alpha0 = (double)intersect / (double)sizeA;
							double beta0 = (double)intersect / (double)sizeB;

							int c0 = newfreeid;
							if (c.IsKey(commsA[i])) {
								c0 = c.GetDat(commsA[i]);
							}
							else {
								c.AddDat(commsA[i], newfreeid);
								newfreeid++;
							}
							if (alpha0 > alpha && beta0 > beta) {
								c.AddDat(commsB[j], c0);
							}
							else {
								c.AddDat(commsB[j], newfreeid);
								newfreeid++;
							}
            }

          }
				}

			} // end if

			br++;
		}
	}

	// Create evolution graph JSON object
	TStr CmtyEvolutionGraphToJson(PNGraph& graph, TIntH& t, TIntH& c, TIntH& s, TIntV& e, TIntStrH txt){
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// This function creates a JSON string with communities and edges for community evolution visualization using D3.js
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		TStr Json;

		// writing json label for edges
		Json.InsStr(Json.Len(), "{\n\"edges\":[\n");

		TInt br = 0;
		// iterating hash of vector of edges and writing into string 
		for (TNGraph::TEdgeI EI = graph->BegEI(); EI<graph->EndEI(); EI++)
		{
      if (br>0)
        Json.InsStr(Json.Len(), ",");

			TInt n1 = EI.GetSrcNId();
			TInt n2 = EI.GetDstNId();
			TInt w = 1;
			if (br < e.Len())
				w = e[br];
			TInt t0 = t.GetDat(n1);
			TInt t1 = t.GetDat(n2);

			// writing to string
			Json.InsStr(Json.Len(), "{\"n1\":"); Json.InsStr(Json.Len(), n1.GetStr());
			Json.InsStr(Json.Len(), ", \"n2\":"); Json.InsStr(Json.Len(), n2.GetStr());
			Json.InsStr(Json.Len(), ", \"w\":"); Json.InsStr(Json.Len(), w.GetStr());
			Json.InsStr(Json.Len(), ", \"t0\":"); Json.InsStr(Json.Len(), t0.GetStr());
			Json.InsStr(Json.Len(), ", \"t1\":"); Json.InsStr(Json.Len(), t1.GetStr());
			Json.InsStr(Json.Len(), " }\n");
			br++;
		}

    // json label for communities
    Json.InsStr(Json.Len(), "],\n\"communities\":[\n");

		br = 0;
		// printing communities into json file

		TIntV nodes;

		// sorting by node ids

		graph->GetNIdV(nodes);
		nodes.Sort();


		// sorting by cmty ids
		/*c.SortByDat();
		c.GetKeyV(nodes);

		for (THashKeyDatI<TInt, TInt> it = c.BegI(); it < c.EndI(); it++){
		nodes.Add(it.GetKey());
		}*/

		for (int i = 0; i< nodes.Len(); i++) {
			// id of community
			TInt id = nodes[i];

			// cmty
			TInt cmty = 1;
			if (c.IsKey(id))
				cmty = c.GetDat(id);

			// community size
			TInt size = 1;
			if (s.IsKey(id))
				size = s.GetDat(id);

			// time
			TInt j = t.GetDat(id);

			//text
			TStr text = "";
			if (txt.IsKey(id))
				text = txt.GetDat(id);

			// if the community has size greater than 0, output it to json string
			if (size > 0) {
				if (br>0)
					Json.InsStr(Json.Len(), ",");

				Json.InsStr(Json.Len(), "{\"id\":"); Json.InsStr(Json.Len(), id.GetStr());
				Json.InsStr(Json.Len(), ", \"size\":"); Json.InsStr(Json.Len(), size.GetStr());
				Json.InsStr(Json.Len(), ", \"t\":"); Json.InsStr(Json.Len(), j.GetStr());
				Json.InsStr(Json.Len(), ", \"cmty\":"); Json.InsStr(Json.Len(), cmty.GetStr());
				Json.InsStr(Json.Len(), ", \"txt\":\""); Json.InsStr(Json.Len(), text.CStr()); Json.InsStr(Json.Len(), "\"");
				Json.InsStr(Json.Len(), " }\n");

				br++;
			}
		}

		Json.InsStr(Json.Len(), "]\n}");

		return Json;
	}

	// Create vector of undirected graphs
	void LoadGraphArray(TStr InFNm, TVec<PUNGraph, TSize>& gs) {

		if (InFNm.GetCh(InFNm.Len() - 1) == '/' || InFNm.GetCh(InFNm.Len() - 1) == '\\') {
			TFFile FFile(InFNm);  TStr FNm;
			for (int t = 0; FFile.Next(FNm); t++) {
				PUNGraph Graph = TSnap::LoadEdgeList<PUNGraph>(FNm.CStr(), 0, 1);
				if (Graph->GetNodes()>0) {
					gs.Add(Graph);
				}
			}
		}
		else {
			TSsParser Ss(InFNm, ssfWhiteSep, true, false, true);
			Ss.Next();
			int newcom = 0;
			TStr Marker = "";
			int SrcNId, DstNId;

			TIntIntVH CommsAtT;

			while (!Ss.Eof()) {
				Marker = Ss.GetLnStr();
				// get the year from the network seperator
				if (Marker.GetCh(0) == '#'){

					Ss.Next();
					PUNGraph Graph = PUNGraph::TObj::New();
					do{
						if (!Ss.GetInt(0, SrcNId) || !Ss.GetInt(1, DstNId)) {
							if (!Ss.Eof()){
								Ss.Next();
								if (!Ss.Eof())
									Marker = Ss.GetLnStr();
							}
							continue;
						}
						if (!Graph->IsNode(SrcNId)) { Graph->AddNode(SrcNId); }
						if (!Graph->IsNode(DstNId)) { Graph->AddNode(DstNId); }
						Graph->AddEdge(SrcNId, DstNId);
						Ss.Next();
						if (!Ss.Eof())
							Marker = Ss.GetLnStr();
					} while (Marker.GetCh(0) != '#' && !Ss.Eof());
					if (Graph->GetNodes()>0) {
						gs.Add(Graph);
					}
				}
			}
		}
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
				void AddQ(const int& NId, const double& Q) {
					NIdQH.AddDat(NId, Q);
					if (MxQId == -1 || NIdQH[MxQId]<Q) { MxQId = NIdQH.GetKeyId(NId); }
				}
				void UpdateMaxQ() {
					MxQId = -1;
					for (int i = -1; NIdQH.FNextKeyId(i);) {
						if (MxQId == -1 || NIdQH[MxQId]< NIdQH[i]) { MxQId = i; }
					}
				}
				void DelLink(const int& K) {
					const int NId = GetMxQNId();
					NIdQH.DelKey(K); if (NId == K) { UpdateMaxQ(); }
				}
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
				MxQHeap(Graph->GetNodes()), CmtyIdUF(Graph->GetNodes()) {
				Init(Graph);
			}
			void Init(const PUNGraph& Graph) {
				const double M = 0.5 / Graph->GetEdges(); // 1/2m
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
						MxQHeap.Add(TFltIntIntTr(Dat.GetMxQ(), NI.GetId(), Dat.GetMxQNId()));
					}
				}
				MxQHeap.MakeHeap();
			}
			TFltIntIntTr FindMxQEdge() {
				while (true) {
					if (MxQHeap.Empty()) { break; }
					const TFltIntIntTr TopQ = MxQHeap.PopHeap();
					if (!CmtyQH.IsKey(TopQ.Val2) || !CmtyQH.IsKey(TopQ.Val3)) { continue; }
					if (TopQ.Val1 != CmtyQH.GetDat(TopQ.Val2).GetMxQ() && TopQ.Val1 != CmtyQH.GetDat(TopQ.Val3).GetMxQ()) { continue; }
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
				for (int i = -1; DatJ.NIdQH.FNextKeyId(i);) {
					const int K = DatJ.NIdQH.GetKey(i);
					TCmtyDat& DatK = CmtyQH.GetDat(K);
					double NewQ = DatJ.NIdQH[i];
					if (DatI.NIdQH.IsKey(K)) { NewQ = NewQ + DatI.NIdQH.GetDat(K);  DatK.DelLink(I); }     // K connected to I and J
					else { NewQ = NewQ - 2 * DatI.DegFrac*DatK.DegFrac; }  // K connected to J not I
					DatJ.AddQ(K, NewQ);
					DatK.AddQ(J, NewQ);
					MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J, K), TMath::Mx(J, K)));
				}
				for (int i = -1; DatI.NIdQH.FNextKeyId(i);) {
					const int K = DatI.NIdQH.GetKey(i);
					if (!DatJ.NIdQH.IsKey(K)) { // K connected to I not J
						TCmtyDat& DatK = CmtyQH.GetDat(K);
						const double NewQ = DatI.NIdQH[i] - 2 * DatJ.DegFrac*DatK.DegFrac;
						DatJ.AddQ(K, NewQ);
						DatK.DelLink(I);
						DatK.AddQ(J, NewQ);
						MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J, K), TMath::Mx(J, K)));
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
				while (QMatrix.MergeBestQ()) {}
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
        void AddQ(const int& NId, const double& Q) {
          NIdQH.AddDat(NId, Q);
          if (MxQId == -1 || NIdQH[MxQId]<Q) { MxQId = NIdQH.GetKeyId(NId); }
        }
        void UpdateMaxQ() {
          MxQId = -1;
          for (int i = -1; NIdQH.FNextKeyId(i);) {
            if (MxQId == -1 || NIdQH[MxQId]< NIdQH[i]) { MxQId = i; }
          }
        }
        void DelLink(const int& K) {
          const int NId = GetMxQNId();
          NIdQH.DelKey(K); if (NId == K) { UpdateMaxQ(); }
        }
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
        MxQHeap(Graph->GetNodes()), CmtyIdUF(Graph->GetNodes()) {
        Init(Graph);
      }
      void Init(const PUNGraph& Graph) {
        const double M = 0.5 / Graph->GetEdges(); // 1/2m
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
            MxQHeap.Add(TFltIntIntTr(Dat.GetMxQ(), NI.GetId(), Dat.GetMxQNId()));
          }
        }
        MxQHeap.MakeHeap();
      }
      TFltIntIntTr FindMxQEdge() {
        while (true) {
          if (MxQHeap.Empty()) { break; }
          const TFltIntIntTr TopQ = MxQHeap.PopHeap();
          if (!CmtyQH.IsKey(TopQ.Val2) || !CmtyQH.IsKey(TopQ.Val3)) { continue; }
          if (TopQ.Val1 != CmtyQH.GetDat(TopQ.Val2).GetMxQ() && TopQ.Val1 != CmtyQH.GetDat(TopQ.Val3).GetMxQ()) { continue; }
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
        for (int i = -1; DatJ.NIdQH.FNextKeyId(i);) {
          const int K = DatJ.NIdQH.GetKey(i);
          TCmtyDat& DatK = CmtyQH.GetDat(K);
          double NewQ = DatJ.NIdQH[i];
          if (DatI.NIdQH.IsKey(K)) { NewQ = NewQ + DatI.NIdQH.GetDat(K);  DatK.DelLink(I); }     // K connected to I and J
          else { NewQ = NewQ - 2 * DatI.DegFrac*DatK.DegFrac; }  // K connected to J not I
          DatJ.AddQ(K, NewQ);
          DatK.AddQ(J, NewQ);
          MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J, K), TMath::Mx(J, K)));
        }
        for (int i = -1; DatI.NIdQH.FNextKeyId(i);) {
          const int K = DatI.NIdQH.GetKey(i);
          if (!DatJ.NIdQH.IsKey(K)) { // K connected to I not J
            TCmtyDat& DatK = CmtyQH.GetDat(K);
            const double NewQ = DatI.NIdQH[i] - 2 * DatJ.DegFrac*DatK.DegFrac;
            DatJ.AddQ(K, NewQ);
            DatK.DelLink(I);
            DatK.AddQ(J, NewQ);
            MxQHeap.PushHeap(TFltIntIntTr(NewQ, TMath::Mn(J, K), TMath::Mx(J, K)));
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
        while (QMatrix.MergeBestQ()) {}
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

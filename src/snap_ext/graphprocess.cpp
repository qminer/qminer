/**
 * Copyright (c) 2016, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "graphprocess.h"

namespace TGraphProcess {
    TGraphCascade::TGraphCascade(const PJsonVal& Params) {
        // build graph and node map
        PJsonVal Dag = Params->GetObjKey("dag");
        GenGraph(Dag);
        // read enabled (ignore the ones missing from the graph)
        PJsonVal EnabledNodeIdH = Params->GetObjKey("enabledNodes");
        ProcessEnabled(EnabledNodeIdH);
        PruneGraph();
        // read models
        PJsonVal NodeModels = Params->GetObjKey("nodeModels");
        ProcessModels(NodeModels);
    }

    void TGraphCascade::ProcessEnabled(const PJsonVal& EnabledList) {
        int Len = EnabledList->GetArrVals();
        for (int ElN = 0; ElN < Len; ElN++) {
            TStr NodeNm = EnabledList->GetArrVal(ElN)->GetStr();
            if (NodeNmIdH.IsKey(NodeNm)) {
                EnabledNodeIdH.AddKey(NodeNmIdH.GetDat(NodeNm));
            }
        }
    }

    void TGraphCascade::GenGraph(const PJsonVal& Dag) {
        // Dag is an object, whose props are node names, whose values are arrays of parent node names
        Graph.Clr();
        int Keys = Dag->GetObjKeys();
        for (int KeyN = 0; KeyN < Keys; KeyN++) {
            TStr Key = Dag->GetObjKey(KeyN);
            if (!NodeNmIdH.IsKey(Key)) {
                NodeNmIdH.AddDat(Key, NodeNmIdH.Len());
                NodeIdNmH.AddDat(NodeIdNmH.Len(), Key);
            }
            int DstId = NodeNmIdH.GetDat(Key);
            if (!Graph.IsNode(DstId)) {
                Graph.AddNode(DstId);
            }
            PJsonVal Val = Dag->GetObjKey(Key);
            int Parents = Val->GetArrVals();
            for (int NodeN = 0; NodeN < Parents; NodeN++) {
                TStr NodeNm = Val->GetArrVal(NodeN)->GetStr();
                if (!NodeNmIdH.IsKey(NodeNm)) {
                    NodeNmIdH.AddDat(NodeNm, NodeNmIdH.Len());
                    NodeIdNmH.AddDat(NodeIdNmH.Len(), NodeNm);
                }
                int SrcId = NodeNmIdH.GetDat(NodeNm);
                if (!Graph.IsNode(SrcId)) {
                    Graph.AddNode(SrcId);
                }
                if (!Graph.IsEdge(SrcId, DstId)) {
                    Graph.AddEdge(SrcId, DstId);
                }
            }
        }
    }

    void TGraphCascade::Print(const TIntV& SortV) {
        printf("graph start:\n");
        if (SortV.Empty()) {
            for (TNGraph::TNodeI NI = Graph.BegNI(); NI < Graph.EndNI(); NI++) {
                printf("%s %d %d\n", NodeIdNmH.GetDat(NI.GetId()).CStr(), NI.GetId(), NodeNmIdH.GetDat(NodeIdNmH.GetDat(NI.GetId())).Val);
            }
        } else {
            for (int NodeN = 0; NodeN < SortV.Len(); NodeN++) {
                printf("%s %d\n", NodeIdNmH.GetDat(SortV[NodeN]).CStr(), SortV[NodeN].Val);
            }
        }
        printf("graph end\n");
    }

    void TGraphCascade::PruneGraph() {
        // iterate over nodes
        int Nodes = NodeNmIdH.Len();
        TIntV NodeIdV;  NodeNmIdH.GetDatV(NodeIdV);
        TStrV NodeNmV;  NodeNmIdH.GetKeyV(NodeNmV);

        for (int NodeN = 0; NodeN < Nodes; NodeN++) {
            int NodeId = NodeIdV[NodeN];
            if (!EnabledNodeIdH.IsKey(NodeId)) {
                // if a node is not enabled:
                // - connect its parents to its children
                TNGraph::TNodeI NI = Graph.GetNI(NodeId);
                for (int ParentN = 0; ParentN < NI.GetInDeg(); ParentN++) {
                    for (int ChildN = 0; ChildN < NI.GetOutDeg(); ChildN++) {
                        if (!Graph.IsEdge(NI.GetInNId(ParentN), NI.GetOutNId(ChildN))) {
                            Graph.AddEdge(NI.GetInNId(ParentN), NI.GetOutNId(ChildN));
                        }
                    }
                }
                printf("deleting node %s %d\n", NodeNmV[NodeN].CStr(), NodeId);
                // - delete it (deletes edges)
                Graph.DelNode(NodeId);
            }
        }

        // generate search sequence from sinks to sources
        TopologicalSort(NIdSweep);
        Print(NIdSweep);
    }

    void TGraphCascade::ProcessModels(const PJsonVal& NodeModels) {
        int Keys = NodeModels->GetObjKeys();
        for (int KeyN = 0; KeyN < Keys; KeyN++) {
            TStr Key = NodeModels->GetObjKey(KeyN);
            EAssertR(NodeNmIdH.IsKey(Key), "TGraphCascade::ProcessModels node name unknown " + Key);
            PJsonVal Val = NodeModels->GetObjKey(Key);
            TFltV PMF;
            Val->GetArrNumV(PMF);
            int NodeId = NodeNmIdH.GetDat(Key);
            TFltV NodeCDF(PMF.Len());
            int Len = NodeCDF.Len();
            if (Len > 0) {
                NodeCDF[0] = PMF[0];
                for (int ElN = 1; ElN < Len; ElN++) {
                    NodeCDF[ElN] = NodeCDF[ElN - 1] + PMF[ElN];
                }
            }
            CDF.AddDat(NodeId, NodeCDF);
        }
    }

    void TGraphCascade::TopologicalSort(TIntV& SortedNIdV) {
        int Nodes = Graph.GetNodes();
        
        SortedNIdV.Gen(Nodes, 0); // result
        THash<TInt, TBool> Marks(Nodes); // nodeid -> mark map
        THash<TInt,TBool> TempMarks(Nodes); // nodeid -> temp mark map
        THash<TInt, TBool> Added(Nodes);
        TIntV NIdV;  Graph.GetNIdV(NIdV); // all node ids

        // set marks
        for (int NodeN = 0; NodeN < Nodes; NodeN++) {
            int NodeId = NIdV[NodeN];
            Marks.AddDat(NodeId, false);
            TempMarks.AddDat(NodeId, false);
            Added.AddDat(NodeId, false);
        }

        TSStack<TInt> Stack;
        for (int NodeN = 0; NodeN < Nodes; NodeN++) {
            int NodeId = NIdV[NodeN];
            // select an unmarked node
            if (!Marks.GetDat(NodeId)) {
                Stack.Push(NodeId);
                while (!Stack.Empty()) {
                    // visit TopNode
                    int TopNodeId = Stack.Top();
                    Marks.GetDat(TopNodeId) = true;
                    TempMarks.GetDat(TopNodeId) = true;
                    // add children, set their temp marks to true
                    TNGraph::TNodeI NI = Graph.GetNI(TopNodeId);
                    int Children = NI.GetOutDeg();
                    bool IsFinal = true;
                    for (int ChildN = 0; ChildN < Children; ChildN++) {
                        int ChildId = NI.GetOutNId(ChildN);
                        EAssertR(!TempMarks.GetDat(ChildId), "TGraphCascade::TopologicalSort: the graph is not a DAG!");
                        if (!Marks.GetDat(ChildId)) {
                            // unvisited node
                            IsFinal = false;
                            Stack.Push(ChildId);
                        }
                    }
                    if (IsFinal) {
                        // push TopNode to tail
                        if (!Added.GetDat(TopNodeId)) {
                            SortedNIdV.Add(TopNodeId);
                            Added.GetDat(TopNodeId) = true;
                        }
                        TempMarks.GetDat(TopNodeId) = false;
                        Stack.Pop();
                    }
                }
            }
        }
        SortedNIdV.Reverse();
    }

    TGraphCascade::TGraphCascade(TSIn& SIn) {
        //TODO
    }

    void TGraphCascade::Save(TSOut& SOut) const {
        //TODO
    }

    void TGraphCascade::ObserveNode(const TStr& NodeNm, const uint64& Time) {
        EAssertR(NodeNmIdH.IsKey(NodeNm), "TGraphCascade::ObserveNode unknown node name: " + NodeNm);
        int NodeId = NodeNmIdH.GetDat(NodeNm);
        Timestamps.AddDat(NodeId, Time);
    }
    
    void TGraphCascade::ComputePosterior(const uint64& Time, const int& SampleSize) {
        // handle missing observations (a child was observed, but a parent was not)
        // 
    }

    PJsonVal TGraphCascade::SaveJson() const {
        return TJsonVal::NewObj();
    }
}
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
    // build graph and node name-id maps
    PJsonVal Dag = Params->GetObjKey("dag");
    GenGraph(Dag);
    // read enabled (ignore the ones missing from the graph)
    PJsonVal EnabledNodeIdH = Params->GetObjKey("enabledNodes");
    ProcessEnabled(EnabledNodeIdH);
    PruneGraph();
    InitTimestamps();
    // read models
    PJsonVal NodeModels = Params->GetObjKey("nodeModels");
    ProcessModels(NodeModels);
    TimeUnit = Params->GetObjInt("timeUnit", 1000);
    Rnd.PutSeed(Params->GetObjInt("randSeed", 0));
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
            //printf("deleting node %s %d\n", NodeNmV[NodeN].CStr(), NodeId);
            // - delete it (deletes edges)
            Graph.DelNode(NodeId);
        }
    }

    // generate search sequence from sinks to sources
    TopologicalSort(NIdSweep);
    //Print(NIdSweep);
}

void TGraphCascade::InitTimestamps() {
    TIntV NodeIdV; Graph.GetNIdV(NodeIdV);
    int Nodes = Graph.GetNodes();
    for (int NodeN = 0; NodeN < Nodes; NodeN++) {
        Timestamps.AddDat(NodeIdV[NodeN], 0);
    }
}

void TGraphCascade::ProcessModels(const PJsonVal& NodeModels) {
    int Keys = NodeModels->GetObjKeys();
    for (int KeyN = 0; KeyN < Keys; KeyN++) {
        TStr Key = NodeModels->GetObjKey(KeyN);
        if (!NodeNmIdH.IsKey(Key)) {
            // skip, we will not need this model
            continue;
        }
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

uint64 TGraphCascade::SampleNodeTimestamp(const int& NodeId, const uint64& Time, const int& SampleN) {
    if (Timestamps.GetDat(NodeId) > 0) {
        return Timestamps.GetDat(NodeId);
    } else if (Sample.GetDat(NodeId).Len() > SampleN) {
        return Sample.GetDat(NodeId)[SampleN];
    }
    TNGraph::TNodeI NI = Graph.GetNI(NodeId);
    int Parents = NI.GetInDeg();
    if (Parents == 0) {
        throw TExcept::New("Root node has not been observed yet - cannot run simulation");
    }
    uint64 MaxParentTime = 0;
    for (int ParentN = 0; ParentN < Parents; ParentN++) {
        int ParentId = NI.GetInNId(ParentN);
        // check if parent was observed or has samples available
        if (Timestamps.GetDat(ParentId) == 0 && Sample.GetDat(ParentId).Empty()) {
            throw TExcept::New("Parent node unobserved and sample not available! Missing data or bad graph");
        }
        // get SampleN from each parent
        uint64 ParentTime = SampleNodeTimestamp(ParentId, Time, SampleN);
        // update max
        MaxParentTime = (MaxParentTime > ParentTime) ? MaxParentTime : ParentTime;
    }
    int TimeDiff = (int)(((int64)(Time)-(int64)(MaxParentTime)) / TimeUnit);
    int MinDuration = MAX(TimeDiff, 0);
    if (!CDF.IsKey(NodeId)) {
        // model is missing and the node has not been observed
        // this probably indicates that the node is always missing
        // model its duration as 0, so it should have been observed after
        // its last parent
        return MaxParentTime;
    }
    TFltV& NodeCDF = CDF.GetDat(NodeId);
    int MaxDuration = NodeCDF.Len();

    if (TimeDiff > MaxDuration) { return Time + 1; } // out of model bounds
    double CDFRemain = MinDuration == 0 ? 1.0 : (1.0 - NodeCDF[MinDuration - 1]);
    if (CDFRemain < 1e-16) { return Time + 1; } // numerically out of model bounds
    // inverse cdf sample, conditioned on elapsed time
    double Samp = (1.0 - CDFRemain) + Rnd.GetUniDev() * CDFRemain;
    // find the first CDF bin that exceeds Samp
    // SPEEDUP possible with bisection
    int BinIdx = MaxDuration;
    for (int BinN = MinDuration; BinN < MaxDuration; BinN++) {
        if (NodeCDF[BinN] >= Samp) {
            BinIdx = BinN;
            break;
        }
    }
    // compute time
    return MaxParentTime + (uint64)((BinIdx)* TimeUnit);
}

TGraphCascade::TGraphCascade(TSIn& SIn) {
    //TODO
}

void TGraphCascade::Save(TSOut& SOut) const {
    //TODO
}

void TGraphCascade::ObserveNode(const TStr& NodeNm, const uint64& Time) {
    if (!NodeNmIdH.IsKey(NodeNm)) {
        // skip, we do not use this node
        return;
    }
    int NodeId = NodeNmIdH.GetDat(NodeNm);
    if (!Graph.IsNode(NodeId)) { return; } // skip, we do not use this node
    // Assert that causality is OK, throw exception if it's violated
    EAssertR(Timestamps.GetDat(NodeId) < Time, "TGraphCascade::ObserveNode: the node `" + NodeNm + "` was observed too late given causal constraints");
    Timestamps.AddDat(NodeId, Time);
}
    
void TGraphCascade::ComputePosterior(const uint64& Time, const int& SampleSize) {
    // handle missing observations (a child was observed, but a parent was not)
    // store histograms or full samples?
    int Nodes = Graph.GetNodes();
    for (int NodeN = 0; NodeN < Nodes; NodeN++) {
        int NodeId = NIdSweep[NodeN];
        if (Timestamps.GetDat(NodeId) > 0) {
            // observed node, no need to simulate
            continue;
        }
        if (!Sample.IsKey(NodeId)) { Sample.AddKey(NodeId); }
        TUInt64V& NodeSample = Sample.GetDat(NodeId);
        NodeSample.Gen(SampleSize, 0);
        for (int SampleN = 0; SampleN < SampleSize; SampleN++) {
            // compute one sample and push it to the sample
            uint64 SampleTs = SampleNodeTimestamp(NodeId, Time, SampleN);
            NodeSample.Add(SampleTs);
        }
    }

}

PJsonVal TGraphCascade::GetPosterior(const TStrV& NodeNmV, const TFltV& QuantileV) const {
    PJsonVal Result = TJsonVal::NewObj();
    TIntV NodeIdV;
    if (NodeNmV.Empty()) {
        // go over all zero timestamps for which samples exist
        TIntV FullNodeIdV; Graph.GetNIdV(FullNodeIdV);
        int Nodes = Graph.GetNodes();
        for (int NodeN = 0; NodeN < Nodes; NodeN++) {
            int NodeId = FullNodeIdV[NodeN];
            if (Timestamps.IsKey(NodeId) && Sample.IsKey(NodeId) && !Sample.GetDat(NodeId).Empty() && Timestamps.GetDat(NodeId) == 0) {
                NodeIdV.Add(NodeId);
            }
        }
    } else {
        int Nodes = NodeNmV.Len();
        for (int NodeN = 0; NodeN < Nodes; NodeN++) {
            if (!NodeNmIdH.IsKey(NodeNmV[NodeN])) { continue; }
            int NodeId = NodeNmIdH.GetDat(NodeNmV[NodeN]);
            if (Timestamps.IsKey(NodeId) && Sample.IsKey(NodeId) && !Sample.GetDat(NodeId).Empty() && Timestamps.GetDat(NodeId) == 0) {
                NodeIdV.Add(NodeId);
            }
        }
    }
    EAssertR(QuantileV.Len() > 0, "TGraphCascade::GetPosterior quantiles should not be empty!");
    for (int QuantileN = 0; QuantileN < QuantileV.Len(); QuantileN++) {
        EAssertR((QuantileV[QuantileN] >= 0.0) && (QuantileV[QuantileN] <= 1.0), "TGraphCascade::GetPosterior quantiles should be between 0.0 and 1.0");
    }

    int Nodes = NodeIdV.Len();
    for (int NodeN = 0; NodeN < Nodes; NodeN++) {
        int NodeId = NodeIdV[NodeN];
        TStr NodeNm = NodeIdNmH.GetDat(NodeId);
        int Quantiles = QuantileV.Len();
        TUInt64V SampleV = Sample.GetDat(NodeId);
        SampleV.Sort(true);
        int SampleSize = SampleV.Len();
        PJsonVal QuantilesArr = TJsonVal::NewArr();
        for (int QuantileN = 0; QuantileN < Quantiles; QuantileN++) {
            int Idx = (int)floor(QuantileV[QuantileN] * SampleSize);
            Idx = MIN(Idx, SampleSize - 1);
            uint64 UnixTimestamp = TTm::GetUnixMSecsFromWinMSecs(SampleV[Idx]);
            QuantilesArr->AddToArr((double)UnixTimestamp);
        }
        Result->AddToObj(NodeNm, QuantilesArr);
    }
    return Result;
}

PJsonVal TGraphCascade::GetGraph() const {
    PJsonVal G = TJsonVal::NewObj();
    for (TNGraph::TNodeI NI = Graph.BegNI(); NI < Graph.EndNI(); NI++) {
        TStr NodeNm = NodeIdNmH.GetDat(NI.GetId());
        PJsonVal ParentsArr = TJsonVal::NewArr();
        int InDeg = NI.GetInDeg();
        for (int ParentN = 0; ParentN < InDeg; ParentN++) {
            TStr ParentNm = NodeIdNmH.GetDat(NI.GetInNId(ParentN));
            ParentsArr->AddToArr(ParentNm);
        }
        G->AddToObj(NodeNm, ParentsArr);
    }
    return G;
}

PJsonVal TGraphCascade::GetOrder() const {
    PJsonVal OrderArr = TJsonVal::NewArr();
    int Len = NIdSweep.Len();
    for (int NodeN = 0; NodeN < Len; NodeN++) {
        TStr NodeNm = NodeIdNmH.GetDat(NIdSweep[NodeN]);
        OrderArr->AddToArr(NodeNm);
    }
    return OrderArr;
}

/////////////////////////////////////////////////////////////////////

TEventCorrelator::TEvent::TEvent(const TUInt64PV& event_tags, const TTm event_ts):
    TagCombinations(event_tags), Ts(event_ts) {}


TEventCorrelator::TEvent::TEvent(TSIn& SIn) {
    TagCombinations.Load(SIn);
    uint64 tmp;
    SIn.Load(tmp);
    Ts = TTm::GetTmFromMSecs(tmp);
}

void TEventCorrelator::TEvent::Save(TSOut& SOut) const {
    TagCombinations.Save(SOut);
    SOut.Save(TTm::GetMSecsFromTm(Ts));
}

/////////////////////////////////////////////////////////////////////

TEventCorrelator::TEventStats::TEventStats(TSIn& SIn) {
    Cnt.Load(SIn);
    OccursBefore.Load(SIn);
}

void TEventCorrelator::TEventStats::Save(TSOut& SOut) const {
    Cnt.Save(SOut);
    OccursBefore.Save(SOut);
}

/////////////////////////////////////////////////////////////////////

void TEventCorrelator::InitFromJson(const PJsonVal& Params) {
    WinLen = Params->GetObjUInt64("win_len", 60 * 60 * 1000);

    // TODO support exclussion patterns for flags or their combinations, possibly with wildcards
}

TEventCorrelator::TEventCorrelator(TSIn& SIn) {
    PJsonVal memento = TJsonVal::Load(SIn);
    InitFromJson(memento);

    TTm tmp1(SIn);
    TTm tmp2(SIn);
    TsMin = tmp1;
    TsMax = tmp2;

    CodebookH.Load(SIn);
    CodebookInverseH.Load(SIn);

    CombCodebookH.Load(SIn);
    CombCodebookInverseH.Load(SIn);

    TagCombToStemCombH.Load(SIn);
    StemCombCountsH.Load(SIn);
    Counts.Load(SIn);

    Window.Load(SIn);
    WindowTagCounts.Load(SIn);

}

void TEventCorrelator::Save(TSOut& SOut) const {
    // create memento object for init parameters
    PJsonVal memento = TJsonVal::New();
    memento->AddToObj("win_len", WinLen);
    memento->Save(SOut);

    TsMin.Save(SOut);
    TsMax.Save(SOut);

    CodebookH.Save(SOut);
    CodebookInverseH.Save(SOut);

    CombCodebookH.Save(SOut);
    CombCodebookInverseH.Save(SOut);

    TagCombToStemCombH.Save(SOut);
    StemCombCountsH.Save(SOut);
    Counts.Save(SOut);

    Window.Save(SOut);
    WindowTagCounts.Save(SOut);
}

TStr Join(const TStrV& arr) {
    if (arr.Len() == 0) return TStr("");
    TStr res = arr[0];
    for (int i = 1; i < arr.Len(); i++) {
        res += "(";
        res += arr[i];
        res += ")";
    }
    return res;
}

void TEventCorrelator::GetCombinationsR(TUInt64PV& Res, const TUInt64PV& Tags, TUInt64PV& Curr, int Offset) {
    if (Offset >= Tags.Len()) {
        if (Curr.Len() > 0) {
            TUInt64V stems;
            TUInt64V tags;
            for (int i = 0; i < Curr.Len(); i++) {
                stems.Add(Curr[i].Val1);
                tags.Add(Curr[i].Val2);
            }
            TUInt64P Rec;
            if (CombCodebookH.IsKey(stems)) {
                Rec.Val1 = CombCodebookH.GetDat(stems);
            } else {
                Rec.Val1 = CombCodebookH.AddDat(stems, CombCodebookH.Len());
                CombCodebookInverseH.AddDat(Rec.Val1, stems);
                StemCombCountsH.AddDat(Rec.Val1, 0);
            }
            if (CombCodebookH.IsKey(tags)) {
                Rec.Val2 = CombCodebookH.GetDat(tags);
            } else {
                Rec.Val2 = CombCodebookH.AddDat(tags, CombCodebookH.Len());
                CombCodebookInverseH.AddDat(Rec.Val2, tags);
                TagCombToStemCombH.AddDat(Rec.Val2, Rec.Val1);
                StemCombCountsH.GetDat(Rec.Val1)++; // we found new tag combination for stem combination
            }
            Res.Add(Rec);
        }
    } else {
        for (int i = Offset; i <= Tags.Len(); ++i) {
            Curr.Add(Tags[i]);
            GetCombinationsR(Res, Tags, Curr, i + 1);
            Curr.DelLast();
        }
    }
}

TUInt64PV TEventCorrelator::GetCombinations(const TStrPV& Tags) {
    // pre-emptively register all tags and stems
    TUInt64PV TagsNum;
    for (int i = 0; i < Tags.Len(); i++) {
        TUInt64P Rec;
        if (CodebookH.IsKey(Tags[i].Val1)) {
            Rec.Val1 = CodebookH.GetDat(Tags[i].Val1);
        } else {
            Rec.Val1 = CodebookH.AddDat(Tags[i].Val1, CodebookH.Len());
            CodebookInverseH.AddDat(Rec.Val1, Tags[i].Val1);
        }
        TStr s = Tags[i].Val1 + "=" + Tags[i].Val2;
        if (CodebookH.IsKey(s)) {
            Rec.Val2 = CodebookH.GetDat(s);
        } else {
            Rec.Val2 = CodebookH.AddDat(s, CodebookH.Len());
            CodebookInverseH.AddDat(Rec.Val2, s);
        }
        TagsNum.Add(Rec);
    }

    TUInt64PV res;
    TUInt64PV curr;
    GetCombinationsR(res, TagsNum, curr, 0);
    return res;
}

void TEventCorrelator::IncreaseCounters(const TUInt64PV& Combinations) {
    for (int i = 0; i < Combinations.Len(); i++) {
        const TUInt64P& eid = Combinations[i];
        if (!Counts.IsKey(eid.Val2)) {
            Counts.AddDat(eid.Val2, TEventStats());
        }
        auto& item = Counts.GetDat(eid.Val2);
        item.Cnt++;

        for (auto iter = WindowTagCounts.begin(); iter != WindowTagCounts.end(); iter++) {
            if (iter->Dat == 0) continue;
            auto id2 = iter->Key;
            if (!item.OccursBefore.IsKey(id2)) {
                item.OccursBefore.AddDat(id2, 1);
            } else {
                item.OccursBefore.GetDat(id2)++;
            }
        }
    }
}

void TEventCorrelator::PurgeWindow(const TTm& EventTs) {
    TTm limit = EventTs;
    int msec = WinLen % 1000;
    int sec = (int)(WinLen / 1000);
    limit.AddTime(0, 0, -sec, -msec);

    int remove_before = 0;
    for (int remove_before = 0; remove_before < Window.Len(); remove_before++) {
        const TEvent& item = Window[remove_before];
        if (item.Ts >= limit) break;
        // subtract counts for tag combinations from window
        for (int i = 0; i < item.TagCombinations.Len(); i++) {
            WindowTagCounts.GetDat(item.TagCombinations[i].Val2)--;
        }
    }
    if (remove_before > 0) {
        Window.Del(0, remove_before - 1);
    }
}

void TEventCorrelator::AddToWindow(const TEvent& Event) {
    // insert into Window - it has to remain sorted
    Window.AddSorted(Event);

    // increase tag counts in WindowTagCounts
    for (int i = 0; i < Event.TagCombinations.Len(); i++) {
        const TUInt64& eid = Event.TagCombinations[i].Val2;
        if (WindowTagCounts.IsKey(eid)) {
            WindowTagCounts.GetDat(eid)++;
        } else {
            WindowTagCounts.AddDat(eid, 1);
        }
    }
}

void TEventCorrelator::Add(const TStrPV& EventTags, const TTm EventTs) {

    // create internal structure
    TStrPV tags(EventTags);
    tags.Sort();
    TUInt64PV e_combinations = GetCombinations(tags);
    TEvent e(e_combinations, EventTs);

    PurgeWindow(EventTs);

    // TODO calculate probability of this event to occur before updating counts

    IncreaseCounters(e_combinations);
    AddToWindow(e);
}

void TEventCorrelator::Predict(TVec<TTriple<TFlt, TStr, TStr>>& Predictions, const int MaxPredictions) {

    for (auto iter = WindowTagCounts.begin(); iter != WindowTagCounts.end(); iter++) {
        if (iter->Dat == 0) continue;
        const TUInt64& id1 = iter->Key;
        const TEventStats& item1 = Counts.GetDat(id1);

        for (auto iter2 = item1.OccursBefore.begin(); iter2 != item1.OccursBefore.end(); iter2++) {
            const TInt& cnt_cooccurr = WindowTagCounts.GetDat(iter2->Key);
            if (cnt_cooccurr > 0) continue; // event 2 already happened in this window
            TTriple<TFlt, TStr, TStr> res_item(
                -(double)cnt_cooccurr / item1.Cnt, // with which probability - negative values are used so that default sort already works
                CodebookInverseH.GetDat(iter2->Key), // which tag combination is most likely to happen
                CodebookInverseH.GetDat(id1) // which tag combination produced this prediction
            );
            Predictions.AddSorted(res_item);
        }
        // prune redundant predictions
        if (Predictions.Len() > MaxPredictions) {
            Predictions.Del(MaxPredictions, Predictions.Len() - 1);
        }
    }
    // make probabilities positive again - resulting in descending sort
    for (int i = 0; i < Predictions.Len(); i++) {
        Predictions[i].Val1 *= -1;
    }
}

}

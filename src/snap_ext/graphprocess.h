/**
 * Copyright (c) 2016, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "Snap.h"

namespace TGraphProcess {

/////////////////////////////////////////////
/// Modelling when nodes are activated (visited) in a DAG.
/// Inputs: directed acyclic graph
///       : enabled nodes (subgraph)
///       : node duration models (PMF vectors, where the indices represent units of time)
///       : units (how many milliseconds per cell)
/// one unit of time corresponds to one cell in the CDF (example: CDF vector with 
/// 5 elements might represent duration time from 0 to 4 seconds)
class TGraphCascade {
private:
    // FIXED AFTER INIT
    /// maps node id to CDF of duration (wait time model for the node)
    THash<TInt, TFltV> CDF;
    /// node ids that are enabled (others skip their internal waiting) 
    THashSet<TInt> EnabledNodeIdH;
    /// Maps node names to node ids
    TStrIntH NodeNmIdH;
    /// Maps node ids to node names
    TIntStrH NodeIdNmH;
    /// pruned directed acyclic graph (only enabled nodes)
    TNGraph Graph;
    /// Topologically ordered node IDs
    TIntV NIdSweep;
    /// number of milliseconds per unit for CDF models
    int TimeUnit;
    /// random generator
    TRnd Rnd;

    // STATE
    /// sample: node id -> vector of samples from posterior. The vector has only one element if the node has been observed (is in the past)
    THash<TInt, TUInt64V> Sample;
    /// Observed timestamps (msec from 1600), 0 if not observed yet
    THash<TInt, TUInt64> Timestamps;


private:
    /// build undirected graph and node name -> node id map
    void GenGraph(const PJsonVal& Dag);
    /// converts JSON to EnabledNodes
    void ProcessEnabled(const PJsonVal& EnabledList);
    // prints graph
    void Print(const TIntV& SortV = TIntV());
    /// remove disabled nodes
    void PruneGraph();
    /// Initializes nodeid to timestamp map
    void InitTimestamps();
    /// read PMFs, compute CDFs
    void ProcessModels(const PJsonVal& NodeModels);
    /// topological sort
    void TopologicalSort(TIntV& SortedNIdV);
    /// get timestamp (sample, or a measurement)
    uint64 SampleNodeTimestamp(const int& NodeId, const uint64& Time, const int& SampleN);

public:
    /// Construct from JSON { dag: {nodeId1: [parentId1, parentId2,...], ...}, enabledNodes: [nodeId1,...], nodeModels: { nodeId1: pmfArray, nodeId2: pmfArray}}
    TGraphCascade(const PJsonVal& Params);
    /// Load
    TGraphCascade(TSIn& SIn);
    /// Save
    void Save(TSOut& SOut) const;
    
    /// Sets the time of observing a node
    void ObserveNode(const TStr& NodeNm, const uint64& Time);
    /// Computes the posterior of node times given all available information and current time
    void ComputePosterior(const uint64& Time, const int& SampleSize);
    /// Returns quantiles for a set of nodes
    PJsonVal GetPosterior(const TStrV& NodeNmV, const TFltV& QuantileV) const;
    /// Returns the pruned graph (only enabled nodes with induced dependencies)
    PJsonVal GetGraph() const;
    /// Returns the topological order (only enabled nodes with induced dependencies)
    PJsonVal GetOrder() const;
};

/// This class correlates event co-occurances in time.
/// Events are described by tags
class TEventCorrelator {
private:
    /// Internal structure that is stored in window
    class TEvent {
    public:
        TUInt64V TagCombinations;
        TTm Ts;
        TEvent() {};
        TEvent(const TUInt64V& event_tags, const TTm event_ts);
        TEvent(TSIn& SIn);
        void Save(TSOut& SOut) const;
    };
    /// Window length in milliseconds
    TUInt64 WinLen;

    /// Mapping from string value of tag into its ID
    TStrHash<TUInt64> TagCodebookH;
    /// Window
    TVec<TEvent> Window;
    /// counts for single combination occurences inside current window
    THash<TUInt64, TInt> WindowTagCounts;
    /// counts for single combination occurences
    THash<TUInt64, TInt> SingleCounts;
    /// counts for combination-pair co-occurences
    THash<TPair<TUInt64, TUInt64>, TInt> CooccurCounts;

    /// Recursive function for creating combinations from string array
    void GetCombinationsR(TUInt64V& res, const TStrV& src, TStrV& curr, int offset);
    /// Starting function for creating combinations from string array
    TUInt64V GetCombinations(const TStrV& e);
    /// Given combination ids update single counters
    void IncreaseSingleCounters(const TUInt64V& e_combinations);
    /// Given two events update co-occurence counters
    //void IncreaseCoOccurenceCounters(const TEvent& a, const TEvent& b);

    /// Utility method for setting up this object from JSON
    void InitFromJson(const PJsonVal& Params);
public:
    /// Construct from JSON parameters
    TEventCorrelator(const PJsonVal& Params) { InitFromJson(Params); }
    /// Load from stream
    TEventCorrelator(TSIn& SIn);
    /// Save to stream
    void Save(TSOut& SOut) const;
    /// Adds given event into internal structures
    void Add(const TStrV& event_tags, const TTm event_ts);
};

}

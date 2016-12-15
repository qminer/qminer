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

// string pairs
typedef TPair<TStr, TStr> TStrP;
// vector of string pairs
typedef TVec<TStrP> TStrPV;
// string pairs
typedef TPair<TUInt64, TUInt64> TUInt64P;
// vector of string pairs
typedef TVec<TUInt64P> TUInt64PV;

/// This class correlates event co-occurrances in time.
/// Events are described by tags
class TEventCorrelator {
private:

    /// Internal structure that is stored in window
    class TEvent {
    public:
        TUInt64PV TagCombinations;
        TTm Ts;
        TEvent() {};
        TEvent(const TUInt64PV& event_tags, const TTm event_ts);
        TEvent(TSIn& SIn);
        void Save(TSOut& SOut) const;

        bool operator==(const TEvent& Other) const { return Ts == Other.Ts; }
        bool operator<(const TEvent& Other) const { return Ts < Other.Ts; }
    };

    class TEventStats {
    public:
        TInt Cnt;
        THash<TUInt64, TInt> OccursBefore;

        TEventStats() {};
        TEventStats(TSIn& SIn);
        void Save(TSOut& SOut) const;
    };

    /// Window length in milliseconds
    TUInt64 WinLen;
    /// Minimal timestamp of observed data
    TTm TsMin;
    /// Maximal timestamp of observed data
    TTm TsMax;

    /// Mapping from string value into its ID
    TStrHash<TUInt64> CodebookH;
    /// Mapping from tag ID into string value
    THash<TUInt64, TStr> CodebookInverseH;

    /// Mapping from combination into ID
    THash<TUInt64V, TUInt64> CombCodebookH;
    /// Mapping from ID into combination
    THash<TUInt64, TUInt64V> CombCodebookInverseH;

    /// Mapping from tag combination into stem combination
    THash<TUInt64, TUInt64> TagCombToStemCombH;

    /// Counts children (tag combinations) of stem combinations
    THash<TUInt64, TInt> StemCombCountsH;
    /// counts for single combination occurences and co-occurences
    THash<TUInt64, TEventStats> Counts;

    /// Window
    TVec<TEvent> Window;
    /// counts for single combination occurences inside current window
    THash<TUInt64, TInt> WindowTagCounts;

    /// Recursive function for creating combinations from string array
    void GetCombinationsR(TUInt64PV& Res, const TUInt64PV& Tags, TUInt64PV& Curr, int Offset);
    /// Starting function for creating combinations from string array
    TUInt64PV GetCombinations(const TStrPV& Tags);
    /// Given combination ids update single counters and oc-occurence counters
    void IncreaseCounters(const TUInt64PV& Combinations);
    /// Remove events from window that are older than given timestamp
    void PurgeWindow(const TTm& EventTs);
    /// Add event to window
    void AddToWindow(const TEvent& Event);

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
    void Add(const TStrPV& EventTags, const TTm EventTs);
    /// Create predictions - events that are most likely to occur
    /// Result means (probability, event_to_occurr, event_that_already_occurred)
    void Predict(TVec<TTriple<TFlt, TStr, TStr>>& Predictions, const int MaxPredictions = 10);
};

}

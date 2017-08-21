/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

/*
 // This code demonstrates how to implement, compile and test a new stream aggregate as an external
 // qminer extension.

 // To compile the external code, run this from qminer root (make sure node-gyp is up-to-date).
 // Add  --msvs_version=2015 after `node-gyp configure` on windows.
 // In case of multiple include dirs and multiple source files, each item should be enclosed within ' and the items
 // should be separated by a space (see the example below where two external source files are provided)
 node-gyp clean
 node-gyp configure -- -DADDITIONAL_QMINER_INCLUDE_DIRS="src/third_party/external_aggr_tutorial" -DADDITIONAL_QMINER_SOURCES="'src/third_party/external_aggr_tutorial/external_aggr.cpp' 'src/third_party/external_aggr_tutorial/external_aggr.h'"
 node-gyp build

 // Example usage:
    var qm = require('qminer');
    var base = new qm.Base({
            mode: 'createClean',
            schema: [{ name: 'Test',
                       fields: [
                           { name: 'Time', type: 'datetime' },
                           { name: 'Value', type: 'float' }]}]
            });
    var store = base.store('Test');
    // register tick that forwards record field values
    var tick = store.addStreamAggr({
        type: 'timeSeriesTick',
        timestamp: 'Time',
        value: 'Value'
    });
    // register boundsChecker
    var boundsChecker = store.addStreamAggr({
        type: 'boundsChecker',
        inAggr: tick.name,
        lowerBound: 2,
        upperBound: 3
    });

    // Insert a record whose Value is below lower bound: 0 < lb
    store.push({ Time: 10, Value: 0 });
    // Below lower bound, should print -1
    console.log('boundsChecker.getFloat():', boundsChecker.getFloat()); // -1

    // Insert a record with Value within bounds: lb <= 2.5 <= ub
    store.push({ Time: 20, Value: 2.5 });
    // Inside bounds, should print 0
    console.log('boundsChecker.getFloat():', boundsChecker.getFloat()); // 0

    // Works without pushing to store (just creates a recod according to store schema)
    var rec = store.newRecord({ Time: time, Value: 5 });
    // Calls all aggregates (tick and boundsChecker)
    store.triggerOnAddCallbacks(rec);
    // Upper bound exceeded, should print 1
    console.log('boundsChecker.getFloat():', boundsChecker.getFloat()); // 1
*/

// Header guard is required
#ifndef QMINER_EXTERNAL_AGGR_TUTORIAL_H
#define QMINER_EXTERNAL_AGGR_TUTORIAL_H

#include "qminer_core.h"

namespace TQm {
namespace TStreamAggrs {

///////////////////////////////
/// Bounds checker model
/// This class checs values agains lower and upper bounds. Returns:
///  -1 for values strictly lower than lower bound
///   1 for values strictly higher than upper bound
///   0 for other values
class TBoundsChecker {
    /// PARAMETERS
    TFlt LowerBound;
    TFlt UpperBound;
public:
    /// Constructor
    TBoundsChecker(const PJsonVal& ParamVal);
    /// Computation
    double Check(const double& Value) const { return Value > UpperBound ? 1.0 : (Value < LowerBound ? -1.0 : 0.0); }
    /// Model JSON serialization
    PJsonVal SaveJson() const;
};

///////////////////////////////
/// Bounds checker aggregate
///   Reads input values from input aggregate InAggr which must implement IFlt interface.
///   Applies BoundsChecker model to inputs and stores the results as state which is exposed
///   through GetFlt (the aggregate itself implements an IFlt interface).
class TBoundsCheckerAggr : public TStreamAggr,
    public TStreamAggrOut::IFlt {
private:
    /// Input aggregate
    TWPt<TStreamAggr> InAggr;
    /// Model
    TBoundsChecker BoundsChecker;
    /// State
    TFlt CurrentFlt;
protected:
    /// OnStep (TStreamAggr API)
    void OnStep(const TWPt<TStreamAggr>& CallerAggr);
    /// Json constructor (TStreamAggr API)
    TBoundsCheckerAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal), BoundsChecker(ParamVal) {
        InAggr = ParseAggr(ParamVal, "inAggr");
    }
public:
    // MINIMAL API
    /// Smart pointer Json constructor (TStreamAggr API)
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TBoundsCheckerAggr(Base, ParamVal);
    }
    /// Resets the aggregate (TStreamAggr API)
    void Reset() { CurrentFlt = 0.0; }
    /// Latest value (IFlt API)
    double GetFlt() const { return CurrentFlt; }
    /// Serialization to JSon (TStreamAggr API)
    PJsonVal SaveJson(const int& Limit) const;
    /// Stream aggregator type name (TStreamAggr API)
    static TStr GetType() { return "boundsChecker"; }
    /// Stream aggregator type name (TStreamAggr API)
    TStr Type() const { return GetType(); }

    // OPTIONAL API
    /// Load stream aggregate state from stream (TStreamAggr API)
    void LoadState(TSIn& SIn) { CurrentFlt.Load(SIn); };
    /// Save state of stream aggregate to stream (TStreamAggr API)
    void SaveState(TSOut& SOut) const { CurrentFlt.Save(SOut); };
    /// Did we finish initialization? (TStreamAggr API)
    bool IsInit() const { return true; }
    /// List of input aggregates
    void GetInAggrNmV(TStrV& InAggrNmV) const { InAggrNmV.Add(InAggr->GetAggrNm()); }

};

// Register aggregates
void InitExternalAggrTutorial_Aggr() {
    TQm::TStreamAggr::Register<TQm::TStreamAggrs::TBoundsCheckerAggr>();
}

// The argument must match the init function name
INIT_EXTERN_AGGR(InitExternalAggrTutorial_Aggr);

} // TStreamAggrs namespace
} // TQm namespace

#endif

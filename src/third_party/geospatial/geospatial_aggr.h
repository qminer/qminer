/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#ifndef QMINER_GEOSPATIAL_AGGR_H
#define QMINER_GEOSPATIAL_AGGR_H

#include "qminer_core.h"
#include "qminer_storage.h"
#include "qminer_ftr.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>


namespace TQm {
namespace TStreamAggrs {

///////////////////////////////
/// StayPoint detector aggregate
class TStayPointDetector : public TStreamAggr {
private:
    /// state returned with SaveJson
    PJsonVal State;
    /// pointer to store
    TWPt<TStore> Store;
    /// location field id for fast access
    TInt LocationFieldId;
    /// accuracy field id for fast access
    TInt AccuracyFieldId;
    /// time field id for fast access
    TInt TimeFieldId;
    // algorithm parameters
    PJsonVal Params;
protected:
    ///  Updates the stay point model, sets State JSON
    void OnAddRec(const TRec& Rec);
    /// JSON based constructor.
    TStayPointDetector(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Smart pointer constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) { return new TStayPointDetector(Base, ParamVal); }
    /// Loads state
    void LoadState(TSIn& SIn);
    /// Saves state
    void SaveState(TSOut& SOut) const;
    /// Is the aggregate initialized?
    bool IsInit() const;
    /// Resets the aggregate
    void Reset();
    /// JSON serialization
    PJsonVal SaveJson(const int& Limit) const { return State; }
    /// Stream aggregator type name
    static TStr GetType() { return "stayPointDetector"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};


} // TStreamAggrs namespace
} // TQm namespace

#endif

/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "geospatial_aggr.h"

namespace TQm {
namespace TStreamAggrs {

///////////////////////////////
// StayPoint detector aggregate
void TStayPointDetector::OnAddRec(const TRec& Rec) {
    // TODO logic
    State = TJsonVal::NewObj();
    State->AddToObj("status", 0);
    // dump parameters
    State->AddToObj("parameters", Params);
    // copy record
    TTm Timestamp; Rec.GetFieldTm(TimeFieldId, Timestamp);
    State->AddToObj("lastTimestamp",
        (double)TTm::GetUnixMSecsFromWinMSecs(Timestamp.GetMSecsFromTm(Timestamp)));
    State->AddToObj("lastLocationX", Rec.GetFieldFltPr(LocationFieldId).Val1);
    State->AddToObj("lastLocationY", Rec.GetFieldFltPr(LocationFieldId).Val2);
    State->AddToObj("lastAccuracy", Rec.GetFieldByte(AccuracyFieldId));

}

TStayPointDetector::TStayPointDetector(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // TODO parse constructor arguments
    if (ParamVal->IsObjKey("params")) {
        Params = ParamVal->GetObjKey("params");
    }
    TStr StoreNm = ParamVal->GetObjStr("store");
    Store = Base->GetStoreByStoreNm(StoreNm);
    TStr TimeFieldName = ParamVal->GetObjStr("timeField");
    TimeFieldId = Store->GetFieldId(TimeFieldName);
    TStr LocationFieldName = ParamVal->GetObjStr("locationField");
    LocationFieldId = Store->GetFieldId(LocationFieldName);
    TStr AccuracyFieldName = ParamVal->GetObjStr("accuracyField");
    AccuracyFieldId = Store->GetFieldId(AccuracyFieldName);
}

void TStayPointDetector::LoadState(TSIn& SIn) {
    // TODO
}
/// Saves state
void TStayPointDetector::SaveState(TSOut& SOut) const {
    // TODO
}
/// Is the aggregate initialized?
bool TStayPointDetector::IsInit() const {
    // TODO: initialization logic (do we have enough data - does SaveJson return a sensible object?)
    return true;
}
/// Resets the aggregate
void TStayPointDetector::Reset() {
    // TODO
}

} // TStreamAggrs namespace
} // TQm namespace


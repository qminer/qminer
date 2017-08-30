/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "external_aggr_tutorial.h"

namespace TQm {
namespace TStreamAggrs {

TBoundsChecker::TBoundsChecker(const PJsonVal& ParamVal) {
    // Parse lower and upper bound
    LowerBound = ParamVal->GetObjNum("lowerBound", TFlt::Mn); // default=min double
    UpperBound = ParamVal->GetObjNum("upperBound", TFlt::Mx); // default=max double
}

PJsonVal TBoundsChecker::SaveJson() const {
    PJsonVal Result = TJsonVal::NewObj();
    Result->AddToObj("lowerBound", LowerBound);
    Result->AddToObj("upperBound", UpperBound);
    return Result;
}

void TBoundsCheckerAggr::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    CurrentFlt = BoundsChecker.Check(
        Cast<TStreamAggrOut::IFlt>(InAggr)->GetFlt()
    );
}

PJsonVal TBoundsCheckerAggr::SaveJson(const int& Limit) const {
    PJsonVal Result = TJsonVal::NewObj();
    PJsonVal Model = BoundsChecker.SaveJson();
    PJsonVal State = TJsonVal::NewObj();
    State->AddToObj("currentValue", CurrentFlt);
    Result->AddToObj("model", Model);
    Result->AddToObj("state", State);
    return Result;
};

} // TStreamAggrs namespace
} // TQm namespace
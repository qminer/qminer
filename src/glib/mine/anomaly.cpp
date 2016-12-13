/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TAnomalyDetection {

/////////////////////////////////////////////
/// Nearest Neighbor based Annomaly Detection.
void TNearestNeighbor::UpdateDistance(const int& ColId, const int& IgnoreCol) {
    // get vector we update distances for and precompute its norm
    const TIntFltKdV& ColVec = Mat[ColId];
    const double ColNorm = TLinAlg::Norm2(ColVec);
    // search for nearest neighbor
    int NearId = -1; double NearDist = TFlt::Mx;
    for (int ColN = 0; ColN < Mat.Len(); ColN++) {
        // skip column itself and columns to ignore
        if (ColN == ColId) { continue; }
        if (ColN == IgnoreCol) { continue; }
        // get distance
        const TIntFltKdV& _ColVec = Mat[ColN];
        const double Dist = ColNorm - 2 * TLinAlg::DotProduct(ColVec, _ColVec) + TLinAlg::Norm2(_ColVec);
        // check if new nearest neighbor for existing vector ColN
        if (Dist < DistV[ColN]) { DistV[ColN] = Dist; DistColV[ColN] = ColId; }
        // check if new nearest neighbor for new vector ColId
        if (Dist < NearDist) { NearId = ColN; NearDist = Dist; }
    }
    // remember new neighbor
    DistV[ColId] = NearDist;
    DistColV[ColId] = NearId;
}

void TNearestNeighbor::UpdateThreshold() {
    ThresholdV.Gen(RateV.Len(), 0);
    // sort distances
    TFltV SortedV = DistV; SortedV.Sort(true);
    // establish thrashold for each rate
    for (const double Rate : RateV) {
        // element Id corresponding to Rate-th percentile
        const int Elt = (int)floor((1.0 - Rate) * SortedV.Len());
        // remember the distance as threshold
        ThresholdV.Add(SortedV[Elt]);
    }
}

void TNearestNeighbor::Forget(const int& ColId) {
    // identify which vectors we should update
    TIntV CheckV;
    for (int ColN = 0; ColN < Mat.Len(); ColN++) {
        // skip self
        if (ColN == ColId) { continue; }
        // we are the nearest neighbor, need to find a new one
        if (DistColV[ColN] == ColId) { CheckV.Add(ColN); }
    }
    // reasses
    for (const int ColN : CheckV) {
        // update distance for ColN ignoring vector ColId
        UpdateDistance(ColN, ColId);
    }
}

TNearestNeighbor::TNearestNeighbor(const TFltV& _RateV, const int& _WindowSize):
        RateV(_RateV), WindowSize(_WindowSize) {

    // assert rate parameter range
    for (const double Rate : RateV) {
        EAssertR(0.0 < Rate && Rate < 1.0, "TAnomalyDetection::TNearestNeighbor: Rate parameter not > 0.0 and < 1.0");
    }
    // initialize all vectors to window size
    Mat.Gen(WindowSize, 0);
    DistV.Gen(WindowSize, 0);
    DistColV.Gen(WindowSize, 0);
	IDVec.Gen(WindowSize, 0);
}

TNearestNeighbor::TNearestNeighbor(TSIn& SIn): RateV(SIn), WindowSize(SIn), Mat(SIn),
    DistV(SIn), DistColV(SIn), ThresholdV(SIn), InitVecs(SIn), NextCol(SIn), IDVec(SIn) { }

void TNearestNeighbor::Save(TSOut& SOut) {
    RateV.Save(SOut);
    WindowSize.Save(SOut);
    Mat.Save(SOut);
    DistV.Save(SOut);
    DistColV.Save(SOut);
    ThresholdV.Save(SOut);
    InitVecs.Save(SOut);
    NextCol.Save(SOut);
	IDVec.Save(SOut);
}

void TNearestNeighbor::PartialFit(const TIntFltKdV& Vec, const int& RecId) {
    if (InitVecs < WindowSize) {
        // not yet full, extend matrix and distance vectors
        Mat.Add(Vec);
		IDVec.Add(RecId);
        // make sure we are very far from everything for update distance to kick in
        DistV.Add(TFlt::Mx); DistColV.Add(InitVecs);
        // update distance for new vector
        UpdateDistance(InitVecs);
        // move onwards
        InitVecs++;
        // check if we are initialized
        if (InitVecs == WindowSize) { UpdateThreshold(); }
    } else {
        // we are full, make space first
        Forget(NextCol);
        // overwrite
        Mat[NextCol] = Vec;
		IDVec[NextCol] = RecId;
        DistV[NextCol] = TFlt::Mx;
        DistColV[NextCol] = NextCol;
        // update distance for overwriten vector
        UpdateDistance(NextCol);
        // establish new threshold
        UpdateThreshold();
        // move onwards
        NextCol++;
        if (NextCol >= WindowSize) { NextCol = 0; }
    }
}

double TNearestNeighbor::DecisionFunction(const TIntFltKdV& Vec) const {
    double NearDist = TFlt::Mx;
    for (const TIntFltKdV& Col : Mat) {
        const double Dist = TLinAlg::Norm2(Vec) - 2 * TLinAlg::DotProduct(Vec, Col) + TLinAlg::Norm2(Col);
        if (Dist < NearDist) { NearDist = Dist; }
    }
    return NearDist;
}

int TNearestNeighbor::Predict(const TIntFltKdV& Vec) const {
    // if not initialized, do nothing
    if (!IsInit()) { return 0; }
    // get distance to nearest stored element
    const double Dist = DecisionFunction(Vec);
    // find in which rate bucket if falls (zero means none)
    int Rate = 0;
    while (Rate < RateV.Len() && Dist > ThresholdV[Rate]) { Rate++; }
    // we are done
    return Rate;
}

PJsonVal TNearestNeighbor::Explain(const TIntFltKdV& Vec) const {
    // if not initialized, return null (JSON)
    if (!IsInit()) { return TJsonVal::NewNull(); }
    // find nearest neighbor
    double NearDist = TFlt::Mx; int NearColN = -1;
    for (int ColN = 0; ColN < Mat.Len(); ColN++) {
        const double Dist = TLinAlg::Norm2(Vec) - 2 * TLinAlg::DotProduct(Vec, Mat[ColN]) + TLinAlg::Norm2(Mat[ColN]);
        if (Dist < NearDist) { NearDist = Dist; NearColN = ColN; }
    }
    const TIntFltKdV& NearVec = Mat[NearColN];
    // generate JSon explanations
    PJsonVal ResVal = TJsonVal::NewObj();
    // id of the nearest element
    ResVal->AddToObj("nearestDat", DatV[NearColN]);
    ResVal->AddToObj("distance", NearDist);
    // element-wise difference
    PJsonVal DiffVal = TJsonVal::NewArr();
    int NearEltN = 0, EltN = 0;
    while (NearEltN < NearVec.Len() || EltN < Vec.Len()) {
        // get the feature ID
        const int VecFtrId = EltN < Vec.Len() ? Vec[NearEltN].Key.Val : TInt::Mx;
        const int NearFtrId = NearEltN < NearVec.Len() ? NearVec[NearEltN].Key.Val : TInt::Mx;
        const int FtrId = NearFtrId < VecFtrId ? NearFtrId : VecFtrId;
        // get values
        const double VecVal = FtrId < VecFtrId ? 0.0 : Vec[EltN].Dat.Val;
        const double NearVal = FtrId < NearFtrId ? 0.0 : NearVec[NearEltN].Dat.Val;
        // get diff
        const double Diff = TMath::Sqr(NearVal - VecVal) / NearDist;
        // add to json result
        PJsonVal FtrVal = TJsonVal::NewObj();
        //avoid unnecessary fields in the explanation
        if (Diff > 1e-8) {
            FtrVal->AddToObj("id", FtrId);
            FtrVal->AddToObj("val", VecVal);
            FtrVal->AddToObj("nearVal", NearVal);
            FtrVal->AddToObj("contribution", Diff);
            DiffVal->AddToArr(FtrVal);
        }
        // move to the next feature
        if (VecFtrId <= NearFtrId) {
            EltN++;
        }
        if (NearFtrId <= VecFtrId) {
            NearEltN++;
        }
    }
    ResVal->AddToObj("features", DiffVal);
    // first and last record in the buffer
    ResVal->AddToObj("oldestDat", DatV[NextCol]);
    int CurCol = NextCol > 0 ? NextCol - 1 : WindowSize - 1;
    ResVal->AddToObj("newestDat", DatV[CurCol]);
    return ResVal;
}

};

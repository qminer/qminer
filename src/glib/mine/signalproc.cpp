/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TSignalProc {

/////////////////////////////////////////////////
// Simple Online Moving Variance
void TVarSimple::Load(TSIn& SIn) {
    *this = TVarSimple(SIn);
}

void TVarSimple::Save(TSOut& SOut) const {
    OldM.Save(SOut);
    NewM.Save(SOut);
    OldS.Save(SOut);
    NewS.Save(SOut);
    N.Save(SOut);
}

void TVarSimple::Update(const double& InVal) {
    // See Knuth TAOCP vol 2, 3rd edition, page 232
    N++;
    if (N == 1) {
        OldM = NewM = InVal;
        OldS = 0.0;
    } else {
        NewM = OldM + (InVal - OldM) / N;
        NewS = OldS + (InVal - OldM)*(InVal - NewM);
        // set up for next iteration
        OldM = NewM;
        OldS = NewS;
    }
}

/////////////////////////////////////////////////
// Online Moving Average
void TMa::AddVal(const double& InVal) {
    // first count in the new value
    Count++;
    // update the moving average
    const double Delta = InVal - Ma;
    Ma = Ma + Delta / Count;
}

void TMa::DeleteVal(const double& OutVal) {
    // if we delete we must have something
    EAssert(Count > 0);
    // update the count
    Count--;
    // update the moving average
    if (Count == 0) {
        // in case no more values, reset
        Ma = 0;
    } else {
        // else readjust
        const double Delta = OutVal - Ma;
        Ma = Ma - Delta / Count;
    }
}

void TMa::Load(TSIn& SIn) {
    *this = TMa(SIn);
}

void TMa::Save(TSOut& SOut) const {
    Count.Save(SOut);
    Ma.Save(SOut);
    TmMSecs.Save(SOut);
}

void TMa::Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV){

    // delete old values
    for (const double OutVal : OutValV) { DeleteVal(OutVal); }
    // add the new value
    AddVal(InVal);
    // update time stamp
    TmMSecs = InTmMSecs;
}

void TMa::Update(const TFltV& InValV, const TUInt64V& InTmMSecsV,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs) {

    // delete old values
    for (const double OutVal : OutValV) { DeleteVal(OutVal); }
    // add the new value
    for (const double InVal : InValV) { AddVal(InVal); }
    // update time stamp with the largest of the new ones
    // (we cannot assume any order in the input values)
    for (const uint64 InTmMSecs : InTmMSecsV) {
        if (InTmMSecs > TmMSecs) { TmMSecs = InTmMSecs; }
    }
}

/////////////////////////////////////////////////
// Online Sum
void TSum::Load(TSIn& SIn) {
    *this = TSum(SIn);
}

void TSum::Save(TSOut& SOut) const {
    Sum.Save(SOut);
    TmMSecs.Save(SOut);
}

void TSum::Update(const double& InVal, const uint64& InTmMSecs, const TFltV& OutValV, const TUInt64V& OutTmMSecsV){
    // remove old values from the sum
    for (const double OutVal : OutValV) { Sum -= OutVal; }
    // add the new value to the resulting sum
    Sum += InVal;
    // update time stamp
    TmMSecs = InTmMSecs;
}

void TSum::Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecs) {
    // remove old values from the sum
    for (const double OutVal : OutValV) { Sum -= OutVal; }
    // add new values to the sum
    for (const double InVal : InValV) { Sum += InVal; }
    // update time stamp with the largest of the new ones
    // (we cannot assume any order in the input values)
    for (const uint64 InTmMSecs : InTmMSecsV) {
        if (InTmMSecs > TmMSecs) { TmMSecs = InTmMSecs; }
    }
}

/////////////////////////////////////////////////
// Online Sum of sparse vectors
void TSumSpVec::AddVal(const TIntFltKdV& SpV) {
    TIntFltKdV NewSum;
    TLinAlg::LinComb(1, Sum, 1, SpV, NewSum);
    Sum = NewSum;
}

void TSumSpVec::DelVal(const TIntFltKdV& SpV) {
    TIntFltKdV NewSum;
    TLinAlg::LinComb(1, Sum, -1, SpV, NewSum);
    Sum = NewSum;
}

void TSumSpVec::Load(TSIn& SIn) {
    *this = TSumSpVec(SIn);
}

void TSumSpVec::Save(TSOut& SOut) const {
    Sum.Save(SOut);
    TmMSecs.Save(SOut);
}

void TSumSpVec::Update(const TVec<TIntFltKd>& InVal, const uint64& InTmMSecs,
        const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecsV) {

    // remove old values from the sum
    for (const TIntFltKdV& OutSpV: OutValV) {
        DelVal(OutSpV);
    }
    // add new values to the sum
    AddVal(InVal);
    // update timestamp
    TmMSecs = InTmMSecs;
}

void TSumSpVec::Update(const TVec<TIntFltKdV>& InValV, const TUInt64V& InTmMSecsV,
        const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecs) {

    // remove old values from the sum
    for (const TIntFltKdV& OutSpV: OutValV) {
        DelVal(OutSpV);
    }
    // add new values to the sum
    for (const TIntFltKdV& InSpV: InValV) {
        AddVal(InSpV);
    }
    // update timestamp
    TmMSecs = InTmMSecsV.Last();
}

PJsonVal TSumSpVec::GetJson() const {
    PJsonVal arr = TJsonVal::NewArr();
    for (int i = 0; i < Sum.Len(); i++) {
        PJsonVal tmp = TJsonVal::NewObj();
        tmp->AddToObj("Idx", Sum[i].Key);
        tmp->AddToObj("Val", Sum[i].Dat);
        arr->AddToArr(tmp);
    }
    PJsonVal res = TJsonVal::NewObj();
    res->AddToObj("Sum", arr);
    res->AddToObj("Tm", TmMSecs);
    return res;
}

/////////////////////////////////////////////////
// Online Min
void TMin::AddVal(const double& InVal, const uint64& InTmMSecs) {
    // First we remove all old min candidates that are bigger then the latest value
    while (!AllValV.Empty() && AllValV.Last().Val1 >= InVal) {
        AllValV.DelLast();
    }
    // Then we remember the new minimum candidate
    AllValV.Add(TFltUInt64Pr(InVal, InTmMSecs));
}

void TMin::DelVal(const uint64& OutTmMSecs) {
    // forget all candidates older then the outgoing timestamp
    while (AllValV[0].Val2 <= OutTmMSecs) {
        AllValV.Del(0);
    }
}

void TMin::Load(TSIn& SIn) {
    *this = TMin(SIn);
}

void TMin::Save(TSOut& SOut) const {
    Min.Save(SOut);
    TmMSecs.Save(SOut);
    AllValV.Save(SOut);
}

void TMin::Update(const double& InVal, const uint64& InTmMSecs, const TFltV& OutValV, const TUInt64V& OutTmMSecsV) {
    /// Add new candidates
    AddVal(InVal, InTmMSecs);
    /// Forget old candidates
    if (!OutTmMSecsV.Empty()) { DelVal(OutTmMSecsV.Last()); }
    /// smallest candidate is the current min
    Min = AllValV[0].Val1;
    /// remember the current timestamp
    TmMSecs = InTmMSecs;
}

void TMin::Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecsV) {
    /// Add new candidates
    for (int InValN = 0; InValN < InValV.Len(); InValN++) {
        AddVal(InValV[InValN], InTmMSecsV[InValN]);
    }
    /// Forget old candidates
    if (!OutTmMSecsV.Empty()) { DelVal(OutTmMSecsV.Last()); }
    /// smallest candidate is the current min
    Min = AllValV[0].Val1;
    /// remember the current timestamp if we have any new ones
    if (!InTmMSecsV.Empty()) { TmMSecs = InTmMSecsV.Last(); }
}

/////////////////////////////////////////////////
// Online Max
void TMax::AddVal(const double& InVal, const uint64& InTmMSecs) {
    // First we remove all old max candidates that are bigger then the latest value
    while (!AllValV.Empty() && AllValV[AllValV.Len() - 1].Val1 <= InVal) {
        AllValV.DelLast();
    }
    // Then we remember the new maximum candidate
    AllValV.Add(TFltUInt64Pr(InVal, InTmMSecs));
}

void TMax::DelVal(const uint64& OutTmMSecs) {
    // forget all candidates older then the outgoing timestamp
    while (AllValV[0].Val2 <= OutTmMSecs) {
        // pop front
        AllValV.Del(0);
    }
}

void TMax::Load(TSIn& SIn) {
    *this = TMax(SIn);
}

void TMax::Save(TSOut& SOut) const {
    // parameters
    Max.Save(SOut);
    TmMSecs.Save(SOut);
    AllValV.Save(SOut);
}
void TMax::Update(const double& InVal, const uint64& InTmMSecs, const TFltV& OutValV, const TUInt64V& OutTmMSecsV){
    /// Add new candidates
    AddVal(InVal, InTmMSecs);
    /// Forget old candidates
    if (!OutTmMSecsV.Empty()) { DelVal(OutTmMSecsV.Last()); }
    /// largest candidate is the current max
    Max = AllValV[0].Val1;
    /// remember the current timestamp
    TmMSecs = InTmMSecs;
}

void TMax::Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecsV) {
    /// Add new candidates
    for (int InValN = 0; InValN < InValV.Len(); InValN++) {
        AddVal(InValV[InValN], InTmMSecsV[InValN]);
    }
    /// Forget old candidates
    if (!OutTmMSecsV.Empty()) { DelVal(OutTmMSecsV.Last()); }
    /// largest candidate is the current max
    Max = AllValV[0].Val1;
    /// remember the current timestamp if we have any new ones
    if (!InTmMSecsV.Empty()) { TmMSecs = InTmMSecsV.Last(); }
}

/////////////////////////////////////////////////
// Exponential Moving Average
double TEma::GetNi(const double& Alpha, const double& Mi) {
    switch (Type) {
    case etPreviousPoint: return 1.0;
    case etLinear: return (1 - Mi) / Alpha;
    case etNextPoint: return Mi;
    }
    throw TExcept::New("Unknown EMA interpolation type");
}

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const double& _Decay, const TEmaType& _Type, const uint64& _InitMinMSecs,
    const double& _TmInterval): Decay(_Decay), Type(_Type), LastVal(TFlt::Mn),
    TmInterval(_TmInterval), InitP(false), InitMinMSecs(_InitMinMSecs) { }

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,const double& _TmInterval):
    Type(_Type), LastVal(TFlt::Mn), TmInterval(_TmInterval), InitP(false),
    InitMinMSecs(_InitMinMSecs) { }

TEma::TEma(const PJsonVal& ParamVal) : LastVal(TFlt::Mn), InitP(false) {
    // type
    TStr TypeStr = ParamVal->GetObjStr("emaType");
    if (TypeStr == "previous") {
        Type = etPreviousPoint;
    } else if (TypeStr == "linear") {
        Type = etLinear;
    } else if (TypeStr == "next") {
        Type = etNextPoint;
    } else {
        throw TExcept::New("Unknown ema type " + TypeStr);
    }
    // rest
    TmInterval = ParamVal->GetObjNum("interval");
    InitMinMSecs = ParamVal->GetObjInt("initWindow", 0);
}

TEma::TEma(TSIn& SIn) : Decay(SIn), LastVal(SIn), Ema(SIn), TmMSecs(SIn), InitP(SIn),
InitMinMSecs(SIn), InitValV(SIn), InitMSecsV(SIn) {

    TInt TypeI; TypeI.Load(SIn);
    Type = static_cast<TEmaType>((int)TypeI);
    TFlt TmIntervalFlt; TmIntervalFlt.Load(SIn); TmInterval = TmIntervalFlt;
}

void TEma::Load(TSIn& SIn) {
    *this = TEma(SIn);
}

void TEma::Save(TSOut& SOut) const {
    // parameters
    Decay.Save(SOut);
    LastVal.Save(SOut);
    Ema.Save(SOut);
    TmMSecs.Save(SOut);
    InitP.Save(SOut);
    InitMinMSecs.Save(SOut);
    InitValV.Save(SOut);
    InitMSecsV.Save(SOut);
    // TODO: Use macro for saving enum (SaveEnum, LoadEnum)
    // TODO: change TmInterval from double to TFlt
    // PROBLEM: After changing TmInterval from double to TFlt Qminer crashes hard!
    TInt TypeI = Type; // TEmaType
    TypeI.Save(SOut);
    TFlt TmIntervalFlt = TmInterval; // double
    TmIntervalFlt.Save(SOut);;
}

void TEma::Update(const double& Val, const uint64& NewTmMSecs) {
    double TmInterval1;
    // EMA(first_point) = first_point (no smoothing is possible)
    if (InitMinMSecs == 0) {
        if (LastVal == TFlt::Mn) { LastVal = Val; Ema = Val; TmMSecs = NewTmMSecs; InitP = true;  return; }
    }
    if(NewTmMSecs == TmMSecs) {
        TmInterval1 = 1.0;
    } else{
        TmInterval1 = (double)(NewTmMSecs - TmMSecs);
    }
    if (InitP) {
        // computer parameters for EMA
        double Alpha;
        if (Decay == 0.0) {
            Alpha = TmInterval1 / TmInterval;
        } else {
            Alpha = TmInterval1 / TmInterval  * (-1.0) * TMath::Log(Decay);
        }
        const double Mi = exp(-Alpha);
        const double Ni = GetNi(Alpha, Mi);
        // compute new ema
        Ema = Mi*Ema + (Ni - Mi)*LastVal + (1.0 - Ni)*Val;
    } else {
        // update buffers
        InitValV.Add(Val);
        InitMSecsV.Add(NewTmMSecs);
        // initialize when enough data
        const uint64 StartInitMSecs = InitMSecsV[0] + InitMinMSecs;
        if (StartInitMSecs < NewTmMSecs) {
            // Initialize using "buildup time interval",
            //TODO: check how interpolation type influences this code
            const int Vals = InitMSecsV.Len();
            // compute weights for each value in buffer
            TFltV WeightV(Vals, 0);
            for (int ValN = 0; ValN < Vals; ValN++) {
                const double Alpha = (double)(TmInterval1) / Decay;
                WeightV.Add(exp(-Alpha));
            }
            // normalize weights so they sum to 1.0
            TLinAlg::NormalizeL1(WeightV);
            // compute initial value of EMA as weighted sum
            Ema = TLinAlg::DotProduct(WeightV, InitValV);
            // mark that we are done and clean up after us
            InitP = true; InitValV.Clr(); InitMSecsV.Clr();
        }
    }
    // update last value
    LastVal = Val;
    // update curret time
    TmMSecs = NewTmMSecs;
}

void TEma::Reset() {
    InitP = false;
    LastVal = TFlt::Mn;
    Ema = 0.0;
    TmMSecs = 0;
    InitValV.Gen(0);
    InitMSecsV.Gen(0);
}

/////////////////////////////////////////////////
// Exponential Moving Average - for sparse vectors

double TEmaSpVec::GetNi(const double& Alpha, const double& Mi) {
    switch (Type) {
    case etPreviousPoint: return 1.0;
    case etLinear: return (1 - Mi) / Alpha;
    case etNextPoint: return Mi;
    }
    throw TExcept::New("Unknown EMA interpolation type");
}

TEmaSpVec::TEmaSpVec(const PJsonVal& ParamVal) : LastVal(), InitP(false) {
    // type
    TStr TypeStr = ParamVal->GetObjStr("emaType");
    if (TypeStr == "previous") {
        Type = etPreviousPoint;
    } else if (TypeStr == "linear") {
        Type = etLinear;
    } else if (TypeStr == "next") {
        Type = etNextPoint;
    } else {
        throw TExcept::New("Unknown ema type " + TypeStr);
    }
    // rest
    TmInterval = ParamVal->GetObjNum("interval");
    Cutoff = ParamVal->GetObjNum("cutoff", 0.0001);
    InitMinMSecs = ParamVal->GetObjInt("initWindow", 0);
}

TEmaSpVec::TEmaSpVec(TSIn& SIn) : LastVal(SIn), Ema(SIn), TmMSecs(SIn),
    TmInterval(SIn), Cutoff(SIn), InitP(SIn),
    InitMinMSecs(SIn), InitValV(SIn), InitMSecsV(SIn) {

    TInt TypeI; TypeI.Load(SIn);
    Type = static_cast<TEmaType>((int)TypeI);
    //TFlt TmIntervalFlt; TmIntervalFlt.Load(SIn); TmInterval = TmIntervalFlt;
    //TFlt CutoffFlt; CutoffFlt.Load(SIn); Cutoff = CutoffFlt;
}

void TEmaSpVec::Load(TSIn& SIn) {
    *this = TEmaSpVec(SIn);
}

void TEmaSpVec::Save(TSOut& SOut) const {
    // parameters
    LastVal.Save(SOut);
    Ema.Save(SOut);
    TmMSecs.Save(SOut);
    TmInterval.Save(SOut);
    Cutoff.Save(SOut);
    InitP.Save(SOut);
    InitMinMSecs.Save(SOut);
    InitValV.Save(SOut);
    InitMSecsV.Save(SOut);
    // TODO: Use macro for saving enum (SaveEnum, LoadEnum)
    // TODO: change TmInterval from double to TFlt
    // PROBLEM: After changing TmInterval from double to TFlt Qminer crashes hard!
    TInt TypeI = Type; // TEmaType
    TypeI.Save(SOut);
    //TFlt TmIntervalFlt = TmInterval; // double
    //TmIntervalFlt.Save(SOut);
    //TFlt CutoffFlt = Cutoff; // double
    //CutoffFlt.Save(SOut);
}

void TEmaSpVec::Update(const TIntFltKdV& Val, const uint64& NewTmMSecs) {
    double TmInterval1;
    // EMA(first_point) = first_point (no smoothing is possible)
    if (InitMinMSecs == 0) {
        if (LastVal.Empty()) {
            LastVal = Val;
            Ema = Val;
            TmMSecs = NewTmMSecs;
            InitP = true;
            return;
        }
    }
    if (NewTmMSecs == TmMSecs) {
        TmInterval1 = 1.0;
    } else {
        TmInterval1 = (double)(NewTmMSecs - TmMSecs);
    }
    if (InitP) {
        // compute parameters for EMA
        double Alpha = TmInterval1 / TmInterval;
        const double Mi = exp(-Alpha);
        const double Ni = GetNi(Alpha, Mi);
        // compute new ema
        //Ema = Mi*Ema + (Ni - Mi)*LastVal + (1.0 - Ni)*Val;
        TIntFltKdV Tmp;
        TLinAlg::LinComb(Mi, Ema, Ni - Mi, LastVal, Tmp);
        TLinAlg::LinComb(1, Tmp, 1.0 - Ni, Val, Ema);
    } else {
        // update buffers
        InitValV.Add(Val);
        InitMSecsV.Add(NewTmMSecs);
        // initialize when enough data
        const uint64 StartInitMSecs = InitMSecsV[0] + InitMinMSecs;
        if (StartInitMSecs < NewTmMSecs) {
            // Initialize using "buildup time interval",
            //TODO: check how interpolation type influences this code
            const int Vals = InitMSecsV.Len();
            // compute weights for each value in buffer
            TFltV WeightV(Vals, 0);
            for (int ValN = 0; ValN < Vals; ValN++) {
                const double Alpha = (double)(TmInterval1);
                WeightV.Add(exp(-Alpha));
            }
            // normalize weights so they sum to 1.0
            TLinAlg::NormalizeL1(WeightV);
            // compute initial value of EMA as weighted sum
            //Ema = TLinAlg::DotProduct(WeightV, InitValV);
            TIntFltKdV Tmp;
            for (int i = 0; i < WeightV.Len(); i++) {
                TIntFltKdV Tmp2;
                TLinAlg::LinComb(1, Tmp, WeightV[i], InitValV[i], Tmp2);
                Tmp = Tmp2;
            }
            Ema = Tmp;

            // mark that we are done and clean up after us
            InitP = true;
            InitValV.Clr();
            InitMSecsV.Clr();
        }
    }
    // remove dimensions bellow cutoff
    TIntFltKdV TmpEma;
    //printf("cutoff %f\n", Cutoff.Val);
    for (int i = 0; i < Ema.Len(); i++) {
        if (TFlt::Abs(Ema[i].Dat.Val) >= Cutoff) {
            TmpEma.Add(Ema[i]);
        }
    }
    Ema = TmpEma;

    // update last value
    LastVal = Val;
    // update current time
    TmMSecs = NewTmMSecs;
}

void TEmaSpVec::Reset() {
    InitP = false;
    LastVal.Clr();
    Ema.Clr();
    TmMSecs = 0;
    InitValV.Gen(0);
    InitMSecsV.Gen(0);
}

PJsonVal TEmaSpVec::GetJson() const {
    PJsonVal arr = TJsonVal::NewArr();
    for (int i = 0; i < Ema.Len(); i++) {
        PJsonVal tmp = TJsonVal::NewObj();
        tmp->AddToObj("Idx", Ema[i].Key);
        tmp->AddToObj("Val", Ema[i].Dat);
        arr->AddToArr(tmp);
    }
    PJsonVal res = TJsonVal::NewObj();
    res->AddToObj("Sum", arr);
    res->AddToObj("Tm", TmMSecs);
    return res;
}

/////////////////////////////////////////////////
// Online Moving Standard M2
void TVar::AddVal(const double& InVal) {
    // increase count
    Count++;
    // update variance parameters
    const double Delta = InVal - Ma;
    Ma = Ma + Delta / (double)Count;
    M2 = M2 + Delta * (InVal - Ma);
}

void TVar::DelVal(const double& OutVal) {
    EAssert(Count > 0);
    // decrease count of element we are computing variance from
    Count--;
    if (Count == 0) {
        // no more elements, just reset
        Reset();
    } else {
        // decrease parameters
        const double Delta = OutVal - Ma;
        Ma = Ma - Delta / (double)Count;
        M2 = M2 - Delta * (OutVal - Ma);
    }
}

void TVar::Load(TSIn& SIn) {
    *this = TVar(SIn);
}

void TVar::Save(TSOut& SOut) const {
    Count.Save(SOut);
    Ma.Save(SOut);
    M2.Save(SOut);
    VarVal.Save(SOut);
    TmMSecs.Save(SOut);
}

void TVar::Update(const double& InVal, const uint64& InTmMSecs, const TFltV& OutValV, const TUInt64V& OutTmMSecsV) {
    // remove old values from the parameters
    for (int ValN = 0; ValN < OutValV.Len(); ValN++) { DelVal(OutValV[ValN]); }
    // add new value to the parameters
    AddVal(InVal);
    // update variance
    UpdateVar();
    // update current timestamp
    TmMSecs = InTmMSecs;
}

void TVar::Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecsV) {
    // remove old values from the parameters
    for (int ValN = 0; ValN < OutValV.Len(); ValN++) { DelVal(OutValV[ValN]); }
    // add new value to the parameters
    for (int ValN = 0; ValN < InValV.Len(); ValN++) { AddVal(InValV[ValN]); }
    // update variance
    UpdateVar();
    // update current timestamp
    if (!InTmMSecsV.Empty()) { TmMSecs = InTmMSecsV.Last(); }
}

/////////////////////////////////////////////////
// Online Moving Covariance
void TCov::AddVal(const double& InValX, const double& InValY) {
    // increase count
    Count++;
    // update parameters
    MaX = MaX + InValX;
    MaY = MaY + InValY;
    const double Delta = (InValX - MaX / (double)Count) * (InValY - MaY / (double)Count);
    M2 = M2 + Delta;
}

void TCov::DelVal(const double& OutValX, const double& OutValY) {
    EAssert(Count > 0);
    // decrease count of elements we are computing covariance from
    Count--;
    if (Count == 0) {
        // no more elements, just reset
        Reset();
    } else {
        // update parameters
        MaX = MaX - OutValX;
        MaY = MaY - OutValY;
        // here we use (count + 1) since we did not remove value yet
        const double Delta = (OutValX - MaX / ((double)Count + 1.0)) * (OutValY - MaY / ((double)Count + 1.0));
        M2 = M2 - Delta;
    }
}

void TCov::Load(TSIn& SIn) {
    *this = TCov(SIn);
}

void TCov::Save(TSOut& SOut) const {
    Count.Save(SOut);
    MaX.Save(SOut);
    MaY.Save(SOut);
    M2.Save(SOut);
    CovVal.Save(SOut);
    TmMSecs.Save(SOut);
}

void TCov::Update(const double& InValX, const double& InValY, const uint64& InTmMSecs,
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV){

    // remove old values from the parameters
    for (int ValN = 0; ValN < OutValVX.Len(); ValN++) {
        DelVal(OutValVX[ValN], OutValVY[ValN]);
    }
    // add new value to the parameters
    AddVal(InValX, InValY);
    // update variance
    UpdateCov();
    // update current timestamp
    TmMSecs = InTmMSecs;
}

void TCov::Update(const TFltV& InValVX, const TFltV& InValVY, const TUInt64V& InTmMSecsV,
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV) {

    // remove old values from the parameters
    for (int ValN = 0; ValN < OutValVX.Len(); ValN++) {
        DelVal(OutValVX[ValN], OutValVY[ValN]);
    }
    // add new value to the parameters
    for (int ValN = 0; ValN < InValVX.Len(); ValN++) {
        AddVal(InValVX[ValN], InValVY[ValN]);
    }
    // update variance
    UpdateCov();
    // update current timestamp
    if (!InTmMSecsV.Empty()) { TmMSecs = InTmMSecsV.Last(); }
}

/////////////////////////////////////////
// Time series interpolator interface
PInterpolator TInterpolator::New(const TStr& InterpolatorType) {
    if (InterpolatorType == TPreviousPoint::GetType()) {
        return TPreviousPoint::New();
    }
    else if (InterpolatorType == TLinear::GetType()) {
        return TLinear::New();
    }
    else if (InterpolatorType == TCurrentPoint::GetType()) {
        return TCurrentPoint::New();
    }
    throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

PInterpolator TInterpolator::Load(TSIn& SIn) {
    TStr InterpolatorType(SIn);
    if (InterpolatorType == TPreviousPoint::GetType()) {
        return TPreviousPoint::New(SIn);
    }
    else if (InterpolatorType == TLinear::GetType()) {
        return TLinear::New(SIn);
    }
    else if (InterpolatorType == TCurrentPoint::GetType()) {
        return TCurrentPoint::New(SIn);
    }
    throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

/////////////////////////////////////////
// Buffered interpolator
TBufferedInterpolator::TBufferedInterpolator(const TStr& _InterpolatorType):
        TInterpolator(_InterpolatorType),
        Buff() {}

TBufferedInterpolator::TBufferedInterpolator(const TStr& _InterpolatorType, TSIn& SIn) :
        TInterpolator(_InterpolatorType),
        Buff(SIn) {}

void TBufferedInterpolator::Save(TSOut& SOut) const {
    TInterpolator::Save(SOut);
    Buff.Save(SOut);
}

void TBufferedInterpolator::AddPoint(const double& Val, const uint64& Tm) {
    EAssertR(!TFlt::IsNan(Val), "TBufferedInterpolator::AddPoint: got NaN value!");

    // check if the new point can be added
    if (!Buff.Empty()) {
        const TUInt64FltPr& LastRec = Buff.GetNewest();
        EAssertR(LastRec.Val1 < Tm || (LastRec.Val1 == Tm && LastRec.Val2 == Val),
            "New point has a timestamp lower then the last point in the buffer, or same with different values " + TTm::GetTmFromDateTimeInt((uint)LastRec.Val1).GetStr() + " >= " + TTm::GetTmFromDateTimeInt((uint)Tm).GetStr() + "!");
    }

    // add the new point
    Buff.Add(TUInt64FltPr(Tm, Val));
}

/////////////////////////////////////////
// Previous point interpolator.
// Interpolate by returning last seen value
TPreviousPoint::TPreviousPoint():
        TBufferedInterpolator(TPreviousPoint::GetType()) {}

TPreviousPoint::TPreviousPoint(TSIn& SIn):
    TBufferedInterpolator(GetType(), SIn) {}

void TPreviousPoint::SetNextInterpTm(const uint64& Time) {
    // TODO optimize
    while (Buff.Len() > 1 && Buff.GetOldest(1).Val1 < Time) {
        Buff.DelOldest();
    }
}

double TPreviousPoint::Interpolate(const uint64& Tm) const {
    IAssertR(CanInterpolate(Tm), "TPreviousPoint::Interpolate: Time not in the desired interval!");
    return Buff.GetOldest().Val2;
}

bool TPreviousPoint::CanInterpolate(const uint64& Tm) const {
    return (!Buff.Empty() && Buff.GetOldest().Val1 == Tm) ||
                (Buff.Len() >= 2 && Buff.GetOldest().Val1 <= Tm && Buff.GetOldest(1).Val1 >= Tm);
}

/////////////////////////////////////////
// Current point interpolator.
TCurrentPoint::TCurrentPoint():
        TBufferedInterpolator(TCurrentPoint::GetType()) {}

TCurrentPoint::TCurrentPoint(TSIn& SIn):
    TBufferedInterpolator(GetType(), SIn) {}

void TCurrentPoint::SetNextInterpTm(const uint64& Tm) {
    // at least one past (or current time) record needs to be in the buffer
    bool Change = false;
    while (Buff.Len() >= 2 && Buff.GetOldest(1).Val1 <= Tm) {
        Buff.DelOldest();
        Change = true;
    }
    if (Change) {
        EAssertR(CanInterpolate(Tm), "WTF!? Current point interpolator cannot intrpolate after setting new time!");
    }
    // when the loop finishes we have at least 1 record in the buffer
    // with a timestamp <= Tm
}

double TCurrentPoint::Interpolate(const uint64& Tm) const {
    IAssertR(CanInterpolate(Tm), "TCurrentPoint::Interpolate: Time not in the desired interval!");
    return Buff.GetOldest().Val2;
}

bool TCurrentPoint::CanInterpolate(const uint64& Tm) const {
    return !Buff.Empty() && Buff.GetOldest().Val1 <= Tm;
}


/////////////////////////////////////////
// Time series linear interpolator
TLinear::TLinear():
        TBufferedInterpolator(TLinear::GetType()) {}

TLinear::TLinear(TSIn& SIn):
    TBufferedInterpolator(GetType(), SIn) {}

void TLinear::SetNextInterpTm(const uint64& Time) {
    while (Buff.Len() > 1 && Buff.GetOldest(1).Val1 <= Time) {
        Buff.DelOldest();
    }
}

double TLinear::Interpolate(const uint64& Tm) const {
    AssertR(CanInterpolate(Tm), "TLinear::Interpolate: Time not in the desired interval!");

    const TUInt64FltPr& PrevRec = Buff.GetOldest();
    if (Tm == PrevRec.Val1) { return PrevRec.Val2; }
    const TUInt64FltPr& NextRec = Buff.GetOldest(1);

    // don't need to check if the times of the previous rec and next rec are equal since if
    // that is true Tm will be equal to PrevRec.Tm and the correct result will be returned
    const double Result = PrevRec.Val2 + ((double) (Tm - PrevRec.Val1) / (NextRec.Val1 - PrevRec.Val1)) * (NextRec.Val2 - PrevRec.Val2);
    EAssertR(!TFlt::IsNan(Result), "TLinear: result of interpolation is NaN!");
    return Result;
}

bool TLinear::CanInterpolate(const uint64& Tm) const {
    return (!Buff.Empty() && Buff.GetOldest().Val1 == Tm) ||
            (Buff.Len() >= 2 && Buff.GetOldest().Val1 <= Tm && Tm <= Buff.GetOldest(1).Val1);
}

///////////////////////////////////////////////
// Aggregating resampler
TAggrResampler::TAggrResampler(const PJsonVal& ParamVal) {
    IntervalMSecs = ParamVal->GetObjUInt64("interval");
    TStr TypeNm = ParamVal->GetObjStr("aggType");
    Type = GetType(TypeNm);
    // start point - interval = LastResamplePointVal
    if (ParamVal->IsObjKey("start")) {
        uint64 StartTm = 0;
        PJsonVal StartVal = ParamVal->GetObjKey("start");
        if (StartVal->IsNum()) {
            StartTm = TTm::GetWinMSecsFromUnixMSecs(static_cast<uint64_t> (StartVal->GetNum()));
        } else if (StartVal->IsStr()) {
            StartTm = TTm::GetMSecsFromTm(TTm::GetTmFromWebLogDateTimeStr(StartVal->GetStr(), '-', ':', '.', 'T'));
        } else {
            throw TExcept::New("TAggrResampler constructor: start property should be a unix timestamp (numnber) or a web log time string");
        }
        EAssertR(StartTm >= IntervalMSecs, "Start point too early");
        LastResampPointMSecs = StartTm - IntervalMSecs;
        InitP = true;
    }
    if (ParamVal->IsObjKey("roundStart")) {
        RoundStart = ParamVal->GetObjStr("roundStart");
        EAssertR(RoundStart == "" || RoundStart == "h" || RoundStart == "m" || RoundStart == "s", "TAggrResampler: roundStart should be 'h', 'm' or 's'");
    }
    // XXX decide if we should use NaN for default (max,min,avg are not defined for empty buffers)?
    DefaultVal = ParamVal->GetObjNum("defaultValue", 0);
}

PJsonVal TAggrResampler::GetParams() const {
    PJsonVal Result = TJsonVal::NewObj();
    Result->AddToObj("interval", IntervalMSecs);
    Result->AddToObj("aggType", GetTypeStr(Type));
    Result->AddToObj("roundStart", RoundStart);
    Result->AddToObj("defaultValue", DefaultVal);
    return Result;
}

void TAggrResampler::Reset() {
    CurrentTmMSecs = 0;
    LastResampPointMSecs = 0;
    LastResampPointVal = 0;
    Buff.Clr();
    InitP = false;
}

void TAggrResampler::LoadState(TSIn& SIn) {
    // only state, not params
    CurrentTmMSecs.Load(SIn);
    LastResampPointMSecs.Load(SIn);
    LastResampPointVal.Load(SIn);
    Buff.Load(SIn);
    InitP.Load(SIn);
}

void TAggrResampler::SaveState(TSOut& SOut) const {
    // only state, not params
    CurrentTmMSecs.Save(SOut);
    LastResampPointMSecs.Save(SOut);
    LastResampPointVal.Save(SOut);
    Buff.Save(SOut);
    InitP.Save(SOut);
}

PJsonVal TAggrResampler::SaveJson() const {
    PJsonVal Result = TJsonVal::NewObj();
    return Result;
}

void TAggrResampler::AddPoint(const double& Val, const uint64& Tm) {
    EAssertR(!TFlt::IsNan(Val), "TAggrResampler::AddPoint: got NaN value!");
    Buff.Push(TUInt64FltPr(Tm, Val));
}

/// Sets the current time
void TAggrResampler::SetCurrentTm(const uint64& Tm) {
    CurrentTmMSecs = Tm;
    if (!InitP) {
        EAssertR(Tm >= IntervalMSecs, "TAggrResampler::SetCurrentTm: Current time point too early (currently in uninitialized state). Use 'start' property when constructing the resampler.");
        LastResampPointMSecs = Tm - IntervalMSecs;
        if (!RoundStart.Empty()) {
            TTm Time = TTm::GetTmFromMSecs(LastResampPointMSecs);
            if (RoundStart == "h") {
                Time.SubTime(0, Time.GetMin(), Time.GetSec(), Time.GetMSec());
            } else if (RoundStart == "m") {
                Time.SubTime(0, 0, Time.GetSec(), Time.GetMSec());
            } else {
                Time.SubTime(0, 0, 0, Time.GetMSec());
            }
            LastResampPointMSecs = TTm::GetMSecsFromTm(Time);
        }
        InitP = true;
    }
}

bool TAggrResampler::TryResampleOnce(double& Val, uint64& Tm, bool& FoundEmptyP) {
    FoundEmptyP = false;
    bool ResampleP = CanResample();
    if (ResampleP) {
        if (Type == TAggrResamplerType::artMax) {
            Val = TFlt::Mn;
        } else if (Type == TAggrResamplerType::artMin) {
            Val = TFlt::Mx;
        } else {
            // avg/sum
            Val = 0;
        }
        int Vals = 0;
        // keep adding to Val and removing from buffer all the values that fall in the
        // interval [LastResampPointMSecs + IntervalMSecs , LastResampPointMSecs + 2 * IntervalMSecs)
        // assert that we have not found a point older than the current window
        uint64 Start = LastResampPointMSecs + IntervalMSecs;
        uint64 End = LastResampPointMSecs + 2 * IntervalMSecs - 1;
        while (!Buff.Empty()) {
            const TUInt64FltPr& Point = Buff.Top();
            if (Point.Val1 < Start) {
                // ignore point but notify that its badly configured
                printf("TAggrResampler: stale point found. A point in the buffer should have been aggregated already but it wasn't\n");
                Buff.Pop();
                continue;
            }
            if (Point.Val1 > End) { break; }
            if (Type == TAggrResamplerType::artMax) {
                Val = MAX(Val, Point.Val2.Val);
            } else if (Type == TAggrResamplerType::artMin) {
                Val = MIN(Val, Point.Val2.Val);
            } else {
                // avg/sum
                Val += Point.Val2;
            }
            Vals++;
            Buff.Pop(); // Point == NULL
        }
        if (Type == TAggrResamplerType::artAvg) {
            Val /= (double)Vals;
        }
        if (Vals == 0 && ResampleP) {
            // notify the caller that an empty (but complete) interval was found
            FoundEmptyP = true;
            Val = DefaultVal;
            // printf("TAggrResampler:Nan generated while resampling and using average aggrgator (no data in the interval)\n");
        }
        // new left point of the last successfully aggregated interval
        Tm = LastResampPointMSecs + IntervalMSecs;
        // save state
        LastResampPointVal = Val;
        LastResampPointMSecs = Tm;
    }
    return ResampleP;
}

void TAggrResampler::PrintState(const TStr& Prefix) const {
    printf("%s: interval %s, type %s\n", Prefix.CStr(),
        TUInt64::GetStr(IntervalMSecs).CStr(), GetTypeStr(Type).CStr());
    printf("%s: last r. value: %f\n", Prefix.CStr(),
        LastResampPointVal.Val);
    printf("%s: last r. time:  %s\n", Prefix.CStr(),
        TTm::GetTmFromMSecs(LastResampPointMSecs).GetWebLogDateTimeStr().CStr());
    printf("%s: current time:  %s\n", Prefix.CStr(),
        TTm::GetTmFromMSecs(CurrentTmMSecs).GetWebLogDateTimeStr().CStr());
    printf("%s: initialized:   %s\n", Prefix.CStr(),
        InitP ? "yes" : "no");
    for (int ElN = 0; ElN < (int)Buff.Len(); ElN++) {
        printf("%s: buffer:        %f %s\n",Prefix.CStr(),  Buff[ElN].Val2.Val, TTm::GetTmFromMSecs(Buff[ElN].Val1).GetWebLogDateTimeStr().CStr());
    }
}

TAggrResamplerType TAggrResampler::GetType(const TStr& TypeStr) const {
    if (TypeStr == "avg") {
        return TAggrResamplerType::artAvg;
    } else if (TypeStr == "sum") {
        return TAggrResamplerType::artSum;
    } else if (TypeStr == "min") {
        return TAggrResamplerType::artMin;
    } else if (TypeStr == "max") {
        return TAggrResamplerType::artMax;
    } else {
        throw TExcept::New("TAggrResampler::Unknown resampler type.");
    }
}

TStr TAggrResampler::GetTypeStr(const TAggrResamplerType& Type) const {
    if (Type == TAggrResamplerType::artAvg) {
        return "avg";
    } else if (Type == TAggrResamplerType::artSum) {
        return "sum";
    } else if (Type == TAggrResamplerType::artMin) {
        return "min";
    } else if (Type == TAggrResamplerType::artMax) {
        return "max";
    } else {
        throw TExcept::New("TAggrResampler::Unknown resampler type.");
    }
}

///////////////////////////////////////////////////////////////////
// Neural Networks - Neuron
TRnd TNNet::TNeuron::Rnd = 0;

TNNet::TNeuron::TNeuron(){
}

TNNet::TNeuron::TNeuron(TInt OutputsN, TInt MyId, TTFunc TransFunc){

    TFuncNm = TransFunc;
    for(int c = 0; c < OutputsN; ++c){
        // define the edges, 0 element is weight, 1 element is weight delta
        TFlt RandWeight = RandomWeight();
        OutEdgeV.Add(TIntFltFltTr(c, RandWeight, 0.0));
        SumDeltaWeight.Add(0.0);
    }

    Id = MyId;
}
TNNet::TNeuron::TNeuron(TSIn& SIn){
    OutputVal.Load(SIn);
    Gradient.Load(SIn);
    TFuncNm = LoadEnum<TTFunc>(SIn);
    SumDeltaWeight.Load(SIn);
    OutEdgeV.Load(SIn);
    Id.Load(SIn);
}

void TNNet::TNeuron::FeedFwd(const TLayer& PrevLayer){
    TFlt SumIn = 0.0;
    // get the previous layer's outputs and sum them up
    for(int NeuronN = 0; NeuronN < PrevLayer.GetNeuronN(); ++NeuronN){
        SumIn += PrevLayer.GetOutVal(NeuronN) *
                PrevLayer.GetWeight(NeuronN, Id);
    }
    // not sure if static. maybe different fcn for output nodes
    OutputVal = TNeuron::TransferFcn(SumIn);
}

TFlt TNNet::TNeuron::TransferFcn(TFlt Sum){
    switch (TFuncNm){
        case tanHyper:
            // tanh output range [-1.0..1.0]
            // training data should be scaled to what the transfer function can handle
           return tanh(Sum);
        case softPlus:
            // the softplus function
            return log(1.0 + exp(Sum));
        case sigmoid:
           // sigmoid output range [0.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return 1.0 / (1.0 + exp(-Sum));
        case fastTanh:
           // sigmoid output range [-1.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return Sum / (1.0 + fabs(Sum));
        case fastSigmoid:
           // sigmoid output range [0.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return (Sum / 2.0) / (1.0 + fabs(Sum)) + 0.5;
        case linear:
            return Sum;
    };
    throw TExcept::New("Unknown transfer function type");
}

TFlt TNNet::TNeuron::TransferFcnDeriv(TFlt Sum){
    switch (TFuncNm){
        case tanHyper:
            // tanh derivative approximation
            return 1.0 - tanh(Sum) * tanh(Sum);
        case softPlus:
            // softplus derivative
            return 1.0 / (1.0 + exp(-Sum));
        case sigmoid:{
           double Fun = 1.0 / (1.0 + exp(-Sum));
           return Fun * (1.0 - Fun);
        }
        case fastTanh:
           return 1.0 / ((1.0 + fabs(Sum)) * (1.0 + fabs(Sum)));
        case fastSigmoid:
           return 1.0 / (2.0 * (1.0 + fabs(Sum)) * (1.0 + fabs(Sum)));
        case linear:
            return 1;
    };
    throw TExcept::New("Unknown transfer function type");
}

void TNNet::TNeuron::CalcOutGradient(TFlt TargVal){
    TFlt Delta = TargVal - OutputVal;
    // TODO: different ways of calculating gradients
    Gradient = Delta * TNeuron::TransferFcnDeriv(OutputVal);
}

void TNNet::TNeuron::CalcHiddenGradient(const TLayer& NextLayer){
    // calculate error by summing the derivatives of the weights next layer
    TFlt DerivsOfWeights = SumDOW(NextLayer);
    Gradient = DerivsOfWeights * TNeuron::TransferFcnDeriv(OutputVal);
}

TFlt TNNet::TNeuron::SumDOW(const TLayer& NextLayer) const{
    TFlt sum = 0.0;
    // sum our contributions of the errors at the nodes we feed
    for(int NeuronN = 0; NeuronN < NextLayer.GetNeuronN() - 1; ++NeuronN){
        // weight from us to next layer neuron times its gradient
        sum += GetWeight(NeuronN) * NextLayer.GetGradient(NeuronN);
    }

    return sum;
}

void TNNet::TNeuron::UpdateInputWeights(TLayer& PrevLayer, const TFlt& LearnRate, const TFlt& Momentum, const TBool& UpdateWeights){
    // the weights to be updated are in the OutEdgeV
    // in the neurons in the preceding layer
    for(int NeuronN = 0; NeuronN < PrevLayer.GetNeuronN(); ++NeuronN){
        TNeuron& Neuron = PrevLayer.GetNeuron(NeuronN);
        TFlt OldDeltaWeight = Neuron.GetDeltaWeight(Id);
        //TFlt OldWeight = Neuron.GetWeight(Id);
        TFlt OldSumDeltaWeight = Neuron.GetSumDeltaWeight(Id);

        TFlt NewDeltaWeight =
                // individual input magnified by the gradient and train rate
                LearnRate
                * Neuron.GetOutVal()
                * Gradient
                // add momentum = fraction of previous delta weight, if we are not in batch mode
                + (UpdateWeights  && OldSumDeltaWeight == 0.0 ? Momentum : TFlt())
                * OldDeltaWeight;

        if(UpdateWeights){
            if(OldSumDeltaWeight != 0.0){
                NewDeltaWeight = OldSumDeltaWeight + NewDeltaWeight;
                Neuron.SetSumDeltaWeight(Id, 0.0);
            }
            Neuron.SetDeltaWeight(Id, NewDeltaWeight);
            Neuron.UpdateWeight(Id, NewDeltaWeight);
       }
        else{
            Neuron.SumDeltaWeights(Id, NewDeltaWeight);
        }
    }
}
void TNNet::TNeuron::Save(TSOut& SOut) {
    OutputVal.Save(SOut);
    Gradient.Save(SOut);
    SaveEnum<TTFunc>(SOut, TFuncNm);
    SumDeltaWeight.Save(SOut);
    OutEdgeV.Save(SOut);
    Id.Save(SOut);
}

/////////////////////////////////////////////////////////////////////////
//// Neural Networks - Layer of Neurons
// TODO: why do we need this empty constructor?
TNNet::TLayer::TLayer(){
}

TNNet::TLayer::TLayer(const TInt& NeuronsN, const TInt& OutputsN, const TTFunc& TransFunc){
    // Add neurons to the layer, plus bias neuron
    for(int NeuronN = 0; NeuronN <= NeuronsN; ++NeuronN){
        NeuronV.Add(TNeuron(OutputsN, NeuronN, TransFunc));
        // debugging
        /*printf("Made a neuron!");
        printf(" Neuron N: %d", NeuronN);
        printf(" Neuron H: %d", NeuronV.Len());
        printf("\n");*/
    }
    // Force the bias node's output value to 1.0
    NeuronV.Last().SetOutVal(1.0);
    //printf("\n");
}
TNNet::TLayer::TLayer(TSIn& SIn){
    NeuronV.Load(SIn);
}
void TNNet::TLayer::Save(TSOut& SOut) {
    NeuronV.Save(SOut);
}

/////////////////////////////////////////////////////////////////////////
//// Neural Networks - Neural Net
TNNet::TNNet(const TIntV& LayoutV, const TFlt& _LearnRate,
            const TFlt& _Momentum, const TTFunc& TFuncHiddenL,
            const TTFunc& TFuncOutL){
    // get number of layers
    int LayersN = LayoutV.Len();
    LearnRate = _LearnRate;
    Momentum = _Momentum;
    // create each layer
    for(int LayerN = 0; LayerN < LayersN; ++LayerN){
        // Get number of neurons in the next layer
        // set number of output connections, if output layer then no output connections
        TInt OutputsN = LayerN == LayersN - 1 ? TInt(0) : LayoutV[LayerN + 1];
        // set transfer functions for hidden and output layers
        TTFunc TransFunc = LayerN == LayersN - 1 ? TFuncOutL : TFuncHiddenL;
        TInt NeuronsN = LayoutV[LayerN];
        // Add a layer to the net
        LayerV.Add(TLayer(NeuronsN, OutputsN, TransFunc));
        //for debugging
        //printf("LayerV.Len(): %d \n", LayerV.Len() );
    }
}

TNNet::TNNet(TSIn& SIn):
        LearnRate(SIn),
        Momentum(SIn) {
    LayerV.Load(SIn);
}

PNNet TNNet::Load(TSIn& SIn) {
    return new TNNet(SIn);
}

void TNNet::FeedFwd(const TFltV& InValV){
    // check if number of input values same as number of input neurons
    EAssertR(InValV.Len() == LayerV[0].GetNeuronN() - 1, "InValV must be of equal length than the first layer!");
    // assign input values to input neurons
    for(int InputN = 0; InputN < InValV.Len(); ++InputN){
        LayerV[0].SetOutVal(InputN, InValV[InputN]);
    }

    // forward propagation
    for(int LayerN = 1; LayerN < LayerV.Len(); ++LayerN){
        TLayer& PrevLayer = LayerV[LayerN - 1];
        for(int NeuronN = 0; NeuronN < LayerV[LayerN].GetNeuronN() - 1; ++NeuronN){
            LayerV[LayerN].FeedFwd(NeuronN, PrevLayer);
        }
    }
}

void TNNet::BackProp(const TFltV& TargValV, const TBool& UpdateWeights){
    // calculate overall net error (RMS of output neuron errors)
    TLayer& OutputLayer = LayerV.Last();
    Error = 0.0;

    EAssertR(TargValV.Len() == OutputLayer.GetNeuronN() - 1, "TargValV must be of equal length than the last layer!");
    for(int NeuronN = 0; NeuronN < OutputLayer.GetNeuronN() - 1; ++NeuronN){
        TFlt Delta = TargValV[NeuronN] - OutputLayer.GetOutVal(NeuronN);
        Error += Delta * Delta;
    }
    Error /= OutputLayer.GetNeuronN() - 1;
    Error = sqrt(Error); // RMS

    // recent avg error measurement
    RecentAvgError = (RecentAvgError * RecentAvgSmoothingFactor + Error)
            / (RecentAvgSmoothingFactor + 1.0);
    // Calculate output layer gradients
    for(int NeuronN = 0; NeuronN < OutputLayer.GetNeuronN() - 1; ++NeuronN){
        OutputLayer.CalcOutGradient(NeuronN, TargValV[NeuronN]);
    }
    // Calculate gradients on hidden layers
    for(int LayerN = LayerV.Len() - 2; LayerN > 0; --LayerN){
        TLayer& HiddenLayer = LayerV[LayerN];
        TLayer& NextLayer = LayerV[LayerN + 1];

        for(int NeuronN = 0; NeuronN < HiddenLayer.GetNeuronN() - 1; ++NeuronN){
            HiddenLayer.CalcHiddenGradient(NeuronN, NextLayer);
        }

    }
    // For all layers from output to first hidden layer
    // update connection weights
    for(int LayerN = LayerV.Len() - 1; LayerN > 0; --LayerN){
        TLayer& Layer = LayerV[LayerN];
        TLayer& PrevLayer = LayerV[LayerN - 1];

        for(int NeuronN = 0; NeuronN < Layer.GetNeuronN() - 1; ++NeuronN){
            Layer.UpdateInputWeights(NeuronN, PrevLayer, LearnRate, Momentum, UpdateWeights);
        }
    }
}

void TNNet::GetResults(TFltV& ResultV) const{
    ResultV.Clr(true, -1);

    for(int NeuronN = 0; NeuronN < LayerV.Last().GetNeuronN() - 1; ++NeuronN){
        ResultV.Add(LayerV.Last().GetOutVal(NeuronN));
    }
}

void TNNet::Save(TSOut& SOut) const {

    // Save model variables
    LearnRate.Save(SOut);
    Momentum.Save(SOut);
    LayerV.Save(SOut);
}

TStr TNNet::GetFunction(const TTFunc& FuncEnum) {
    TStr FuncString;
    if (FuncEnum == TSignalProc::TTFunc::tanHyper) {
        FuncString = "tanHyper";
    } else if (FuncEnum == TSignalProc::TTFunc::sigmoid) {
        FuncString = "sigmoid";
    } else if (FuncEnum == TSignalProc::TTFunc::fastTanh) {
        FuncString = "fastTanh";
    } else if (FuncEnum == TSignalProc::TTFunc::softPlus) {
        FuncString = "softPlus";
    } else if (FuncEnum == TSignalProc::TTFunc::fastSigmoid) {
        FuncString = "fastSigmoid";
    } else if (FuncEnum == TSignalProc::TTFunc::linear) {
        FuncString = "linear";
    } else {
        throw TExcept::New("Unknown transfer function type " + FuncString);
    }
    return FuncString;
}

///////////////////////////////////////////////////////////////////
// Recursive Linear Regression
TOnlineLinReg::TOnlineLinReg(const int& Dim, const double& _RegFact,
            const double& _ForgetFact):
        ForgetFact(_ForgetFact),
        RegFact(_RegFact),
        CovVV(Dim, Dim),
        Xy(Dim, Dim),
        SampleN(0),
        WgtV(Dim, Dim) {

    EAssert(0 < ForgetFact && ForgetFact <= 1);
    EAssert(RegFact >= 0);
    EAssert(Dim > 0);
}

TOnlineLinReg::TOnlineLinReg(TSIn& SIn):
        ForgetFact(SIn),
        RegFact(SIn),
        CovVV(SIn),
        Xy(SIn),
        SampleN(SIn),
        WgtV(SIn) {}

void TOnlineLinReg::Save(TSOut& SOut) const {
    ForgetFact.Save(SOut);
    RegFact.Save(SOut);
    CovVV.Save(SOut);
    Xy.Save(SOut);
    SampleN.Save(SOut);
    WgtV.Save(SOut);
}

TOnlineLinReg::TOnlineLinReg(const TOnlineLinReg& LinReg):
        ForgetFact(LinReg.ForgetFact),
        RegFact(LinReg.RegFact),
        CovVV(LinReg.CovVV),
        Xy(LinReg.Xy),
        SampleN(LinReg.SampleN),
        WgtV(LinReg.WgtV) {}

TOnlineLinReg& TOnlineLinReg::operator =(const TOnlineLinReg& LinReg) {
    // provide the strong guarantee (if copy fails, no data should be changed)
    TOnlineLinReg Temp(LinReg);
    std::swap(*this, Temp);
    return *this;
}

TOnlineLinReg::TOnlineLinReg(const TOnlineLinReg&& LinReg):
        ForgetFact(std::move(LinReg.ForgetFact)),
        RegFact(std::move(LinReg.RegFact)),
        CovVV(std::move(LinReg.CovVV)),
        Xy(std::move(LinReg.Xy)),
        SampleN(std::move(LinReg.SampleN)),
        WgtV(std::move(LinReg.WgtV)) {}

TOnlineLinReg& TOnlineLinReg::operator =(TOnlineLinReg&& LinReg) {
    if (this == &LinReg) { return *this; }

    std::swap(ForgetFact, LinReg.ForgetFact);
    std::swap(RegFact, LinReg.RegFact);
    std::swap(CovVV, LinReg.CovVV);
    std::swap(Xy, LinReg.Xy);
    std::swap(SampleN, LinReg.SampleN);
    std::swap(WgtV, LinReg.WgtV);

    return *this;
}

double TOnlineLinReg::Predict(const TFltV& Sample) {
    return WgtV * Sample;
}

void TOnlineLinReg::Learn(const TFltV& FtrV, const double& Val) {
    const int Dim = CovVV.GetRows();
    // if the forget factor is enabled, then first update the covariance matrix and X'y
    // Cn <- beta*Cn
    // Xy <- beta*Xy
    if (ForgetFact < 1) {
        CovVV *= ForgetFact;
        Xy *= ForgetFact;
    }
    // add the last sample
    // Cn <- (beta*Cn) + x*x'
    // Xy <- (beta*Xy) + x*y
    for (int RowN = 0; RowN < Dim; RowN++) {
        for (int ColN = 0; ColN < Dim; ColN++) {
            CovVV(RowN, ColN) += FtrV[RowN]*FtrV[ColN];
        }
        Xy[RowN] += FtrV[RowN]*Val;
    }
    // update n
    SampleN++;

    // update the weight vector
    // w = (Cn / n + lambda*I) \ (Xy / n)

    // XX <- Cn / n
    // Xy <= Xy / n
    TFltVV XX = CovVV / double(SampleN);
    TFltV XyByN = Xy / double(SampleN);

    // regularize
    // XX <- (Cn / n) + lambda*I
    if (RegFact > 0.0) {
        for (int ValN = 0; ValN < Dim; ValN++) {
            XX(ValN, ValN) += RegFact;
        }
    }
    // solve the system
    // w = XX \ Xy
    TNumericalStuff::SolveLinearSystem(XX, XyByN, WgtV);
}

void TOnlineLinReg::GetCoeffs(TFltV& _WgtV) const {
    _WgtV = WgtV;
}

bool TOnlineLinReg::HasNaN() const {
    int Dim = WgtV.Len();
    for (int ElN = 0; ElN < Dim; ElN++) {
        if (WgtV[ElN].IsNan()) {
            return true;
        }
    }
    return false;
}


////////////////////////////////////////////////////
// TOnlineHistogram

TOnlineHistogram::TOnlineHistogram(const PJsonVal& ParamVal) {
    EAssertR(ParamVal->IsObjKey("lowerBound"), "TOnlineHistogram: lowerBound key missing!");
    EAssertR(ParamVal->IsObjKey("upperBound"), "TOnlineHistogram: upperBound key missing!");
    // bounded lowest point
    LBound = ParamVal->GetObjNum("lowerBound");
    // bounded highest point
    UBound = ParamVal->GetObjNum("upperBound");
    EAssertR(LBound < UBound, "TOnlineHistogram: Lower bound should be smaller than upper bound");
    // number of equal bins ? (not counting possibly infinite ones)
    Bins = ParamVal->GetObjInt("bins", 5);
    EAssertR(Bins > 0, "TOnlineHistogram: Number of bins should be greater than 0");
    // include infinities in the bounds?
    AddNegInf = ParamVal->GetObjBool("addNegInf", false);
    AddPosInf = ParamVal->GetObjBool("addPosInf", false);
    AutoResize = ParamVal->GetObjBool("autoResize", false);
    MinCount = ParamVal->GetObjInt("initMinCount", 0);
    Init();
}

void TOnlineHistogram::Init() {
    Count = 0;
    CountLeftInf = 0;
    CountRightInf = 0;
    if (!AutoResize) {
        Bounds.Gen(Bins + 1, 0);
        Counts.Gen(Bins); // sets to zero
        for (int ElN = 0; ElN <= Bins; ElN++) {
            Bounds.Add(LBound + ElN * (UBound - LBound) / Bins);
        }
        CurMinIdx = 0;
        CurMaxIdx = Bins - 1;
    } else {
        Counts.Clr();
        Bounds.Clr();
        CurMinIdx = -2;
        CurMaxIdx = -2;
    }
}

TOnlineHistogram::TOnlineHistogram(TSIn& SIn) : CountLeftInf(SIn), CountRightInf(SIn), Counts(SIn),
Bounds(SIn), Count(SIn), CurMinIdx(SIn), CurMaxIdx(SIn), LBound(SIn), UBound(SIn),
Bins(SIn), AddNegInf(SIn), AddPosInf(SIn), AutoResize(SIn), MinCount(SIn) {}

void TOnlineHistogram::Save(TSOut& SOut) const {
    CountLeftInf.Save(SOut); CountRightInf.Save(SOut); Counts.Save(SOut);
    Bounds.Save(SOut); Count.Save(SOut); CurMinIdx.Save(SOut); CurMaxIdx.Save(SOut);
    LBound.Save(SOut); UBound.Save(SOut); Bins.Save(SOut); AddNegInf.Save(SOut); AddPosInf.Save(SOut);
    AutoResize.Save(SOut); MinCount.Save(SOut);
}

void TOnlineHistogram::Resize(const int& BinN) {
    // expect valid 0 <= BinN <= Bins - 1
    // check if nothing to do
    if (BinN >= CurMinIdx && BinN <= CurMaxIdx) { return; }
    // first example
    if (CurMinIdx == -2 && CurMaxIdx == -2) {
        Counts.Gen(1);
        Bounds.Add(LBound + BinN * (UBound - LBound) / Bins);
        Bounds.Add(LBound + (BinN + 1) * (UBound - LBound) / Bins);
        CurMinIdx = BinN;
        CurMaxIdx = BinN;
        return;
    }
    int OldMinIdx = CurMinIdx;
    int OldMaxIdx = CurMaxIdx;
    while (BinN < CurMinIdx && CurMinIdx > 0) {
        CurMinIdx -= (CurMaxIdx - CurMinIdx + 1);
        CurMinIdx = MAX(CurMinIdx.Val, 0);
    }
    while (BinN > CurMaxIdx && CurMinIdx < Bins - 1) {
        CurMaxIdx += (CurMaxIdx - CurMinIdx + 1);
        CurMaxIdx = MIN(CurMaxIdx.Val, Bins.Val - 1);
    }
    TFltV Counts_(CurMaxIdx - CurMinIdx + 1, 0);
    for (int ElN = CurMinIdx; ElN < OldMinIdx; ElN++) {
        Counts_.Add(0);
    }
    Counts_.AddV(Counts);
    for (int ElN = OldMaxIdx + 1; ElN <= CurMaxIdx; ElN++) {
        Counts_.Add(0);
    }
    Counts = Counts_;
    Bounds.Gen(CurMaxIdx - CurMinIdx + 2, 0);
    for (int ElN = CurMinIdx; ElN <= CurMaxIdx + 1; ElN++) {
        Bounds.Add(LBound + ElN * (UBound - LBound) / Bins);
    }
}

int TOnlineHistogram::FindBin(const double& Val) const {
    // This returns the bin index when for bins between LBound and UBound, enumerated as 0,... Bins-1 (possibly not materialized yet)
    if (Val < LBound) return -1;
    if (Val > UBound) return Bins;
    // Get bin index
    double BinWidth = (UBound - LBound) / Bins;
    int Idx = (int)floor((Val - LBound) / BinWidth);
    // Exception if Val == UBound (included in the last noninf bin)
    if (Idx == Bins) { Idx--; }
    return Idx;
}

void TOnlineHistogram::Increment(const double& Val) {
    int Idx = FindBin(Val);
    if (Idx == -1) {
        if (AddNegInf) {
            CountLeftInf++;
            Count++;
        }
        return;
    }
    if (Idx == Bins) {
        if (AddPosInf) {
            CountRightInf++;
            Count++;
        }
        return;
    }
    if (AutoResize) {
        Resize(Idx);
    }
    // IOB cannot occour (AutoResize)
    Counts[Idx - CurMinIdx]++;
    Count++;

}

void TOnlineHistogram::Decrement(const double& Val) {
    int Idx = FindBin(Val);
    if (Idx < CurMinIdx) {
        if (AddNegInf) {
            CountLeftInf--;
            Count--;
        }
        return;
    }
    if (Idx > CurMaxIdx) {
        if (AddPosInf) {
            CountRightInf--;
            Count--;
        }
        return;
    }
    // We know that IOB cannot occour
    Counts[Idx - CurMinIdx]--;
    Count--;
}

double TOnlineHistogram::GetCount(const double& Val) const {
    int Idx = FindBin(Val);
    if (Idx == -1) { return AddNegInf ? CountLeftInf.Val : 0.0; }
    if (Idx == Bins - 1) { return AddPosInf ? CountRightInf.Val : 0.0; }
    return ((Idx >= CurMinIdx) && (Idx <= CurMaxIdx)) ? Counts[Idx - CurMinIdx].Val : 0.0;
}

void TOnlineHistogram::GetCountV(TFltV& Vec) const {
    Vec.Gen(Counts.Len() + (AddNegInf ? 1 : 0) + (AddPosInf ? 1 : 0), 0);
    if (AddNegInf) { Vec.Add(CountLeftInf); }
    Vec.AddV(Counts);
    if (AddPosInf) { Vec.Add(CountRightInf); }
}

double TOnlineHistogram::GetCountN(const int& CountN) const {
    if (AddNegInf && (CountN == 0)) { return CountLeftInf; }
    if (AddPosInf && (CountN >= (Counts.Len() + (AddNegInf ? 1 : 0)))) { return CountRightInf; }
    return Counts[CountN - (AddNegInf ? 1 : 0)];
}

double TOnlineHistogram::GetBoundN(const int& BoundN) const {
    if (AddNegInf && (BoundN == 0)) { return TFlt::Mn; }
    if (AddPosInf && (BoundN >= (Bounds.Len() + (AddNegInf ? 1 : 0)))) { return TFlt::Mx; }
    return Bounds[BoundN - (AddNegInf ? 1 : 0)];
}

void TOnlineHistogram::Print() const {
    printf("Histogram:\n");
    if (AddNegInf) {
        printf("%g [%g, %g]\n", CountLeftInf.Val, TFlt::Mn, Bounds[0].Val);
    }
    for (int BinN = 0; BinN < Counts.Len(); BinN++) {
        printf("%g [%g, %g]\n", Counts[BinN].Val, Bounds[BinN].Val, Bounds[BinN + 1].Val);
    }
    if (AddPosInf) {
        printf("%g [%g, %g]\n", CountRightInf.Val, Bounds[Bins-1].Val, TFlt::Mx);
    }
}

PJsonVal TOnlineHistogram::SaveJson() const {
    PJsonVal Result = TJsonVal::NewObj();
    PJsonVal BoundsArr = TJsonVal::NewArr();
    PJsonVal CountsArr = TJsonVal::NewArr();
    if (AddNegInf) {
        BoundsArr->AddToArr(TFlt::Mn);
        CountsArr->AddToArr(CountLeftInf);
    }
    for (int ElN = 0; ElN < Counts.Len(); ElN++) {
        CountsArr->AddToArr(Counts[ElN]);
        if (Bounds.Len() > ElN) { BoundsArr->AddToArr(Bounds[ElN]); }
    }
    if (Bounds.Len() > 0) {
        BoundsArr->AddToArr(Bounds.Last());
    }
    if (AddPosInf) {
        BoundsArr->AddToArr(TFlt::Mx);
        CountsArr->AddToArr(CountRightInf);
    }
    Result->AddToObj("bounds", BoundsArr);
    Result->AddToObj("counts", CountsArr);
    return Result;
}

///////////////////////////////////////////////////////////////////
// TTDigest
void TTDigest::Init(const int& N) {
    Nc = N;
    Max = TFlt::Mn;
    Min = TFlt::Mx;
    UnmergedSum = 0;
    TempLast = 0;

    Size = (int)ceil(Nc * TMath::Pi / 2);
    TotalSum = 0;
    Last = -1;

    for (int Iter = 0; Iter < Size; Iter++) {
        Weight.Add(0);
        Mean.Add(0);
        MergeWeight.Add(0);
        MergeMean.Add(0);
    }

    int Tempsize = NumTemp(Nc);
    for (int Iter = 0; Iter < Tempsize; Iter++) {
        TempMean.Add(TFlt::Mx);
        TempWeight.Add(0);
    }
}

double TTDigest::GetQuantile(const double& Q) const {
    double Left = Min;
    double Right = Max;

    if (TotalSum == 0.0) { return -1.0; }
    if (Q <= 0) { return Min; }
    if (Q >= 1) { return Max; }
    if (Last == 0) { return Mean[0]; }

    // calculate boundaries, pick centroid via binary search
    double QSum = Q * TotalSum;

    int N1 = Last + 1;
    int N0 = 0;
    int I = Bisect(MergeMean, QSum, N0, N1);

    if (I > 0) {
        Left = Boundary(I - 1, I, Mean, Weight);
    }
    if (I < Last) {
        Right = Boundary(I, I + 1, Mean, Weight);
    }
    double Quantile = Left + (Right - Left) * (QSum -
        ((I <= 0 || I > MergeMean.Len()) ? 0.0 : MergeMean[I - 1].Val)) / Weight[I];

    return Quantile;
};

int TTDigest::GetClusters() const {
    return Mean.Len();
}

void TTDigest::Update(const double& V, const double& Count) {
    Updates++;
    if (TempLast >= TempWeight.Len()) {
        MergeValues();
    }
    TInt N_ = TempLast++;
    TempWeight[N_] = Count;
    TempMean[N_] = V;
    UnmergedSum += Count;
    MergeValues();
}

void TTDigest::SaveState(TSOut& SOut) const {
    MinPointsInit.Save(SOut);
    Nc.Save(SOut);
    Size.Save(SOut);
    Last.Save(SOut);
    TotalSum.Save(SOut);
    Weight.Save(SOut);
    Mean.Save(SOut);
    Min.Save(SOut);
    Max.Save(SOut);
    MergeWeight.Save(SOut);
    MergeMean.Save(SOut);
    Tempsize.Save(SOut);
    UnmergedSum.Save(SOut);
    TempLast.Save(SOut);
    TempWeight.Save(SOut);
    TempMean.Save(SOut);
    Updates.Save(SOut);
}

void TTDigest::LoadState(TSIn& SIn) {
    MinPointsInit.Load(SIn);
    Nc.Load(SIn);
    Size.Load(SIn);
    Last.Load(SIn);
    TotalSum.Load(SIn);
    Weight.Load(SIn);
    Mean.Load(SIn);
    Min.Load(SIn);
    Max.Load(SIn);
    MergeWeight.Load(SIn);
    MergeMean.Load(SIn);
    Tempsize.Load(SIn);
    UnmergedSum.Load(SIn);
    TempLast.Load(SIn);
    TempWeight.Load(SIn);
    TempMean.Load(SIn);
    Updates.Load(SIn);
}

PJsonVal TTDigest::GetParams() const {
    PJsonVal Result = TJsonVal::NewObj();
    Result->AddToObj("minCount", MinPointsInit);
    Result->AddToObj("clusters", Nc);
    return Result;
}

void TTDigest::SetParams(const PJsonVal& ParamVal) {
    MinPointsInit = ParamVal->GetObjInt("minCount", MinPointsInit);
    Nc = ParamVal->GetObjInt("clusters", Nc);
    Init(Nc);
}

void TTDigest::MergeValues() {
    if (UnmergedSum == 0.0) {
        return;
    }
    TFltV W = Weight;
    TFltV U = Mean;

    double Sum = 0;

    TempMean.Sort();

    TInt LastN = 0;
    if (TotalSum > 0.0) {
        LastN = Last + 1;
    }

    Last = 0;
    TotalSum += UnmergedSum;
    UnmergedSum = 0.0;

    double NewCentroid = 0;
    int IterI = 0;
    int IterJ = 0;
    // merge existing centroids with added values in temp buffers

    while (IterI < TempLast && IterJ < LastN) {
        if (TempMean[IterI] <= U[IterJ]) {
            Sum += TempWeight[IterI];
            double TW = TempWeight[IterI];
            double TM = TempMean[IterI];
            IterI++;
            NewCentroid = MergeCentroid(Sum, NewCentroid, TW, TM);

        } else {
            Sum += W[IterJ];
            double TW = W[IterJ];
            double TM = U[IterJ];
            IterJ++;
            NewCentroid = MergeCentroid(Sum, NewCentroid, TW, TM);

        }
    }

    while (IterI < TempLast) {
        Sum += TempWeight[IterI];
        double TW = TempWeight[IterI];
        double TM = TempMean[IterI];
        NewCentroid = MergeCentroid(Sum, NewCentroid, TW, TM);
        IterI++;
    }

    // only existing     centroids remain
    while (IterJ < LastN) {
        Sum += W[IterJ];
        double TW = W[IterJ];
        double TM = U[IterJ];
        NewCentroid = MergeCentroid(Sum, NewCentroid, TW, TM);
        IterJ++;
    }

    TempLast = 0;

    // swap pointers for working space and merge space

    Weight = MergeWeight;
    Mean = MergeMean;

    MergeMean = U;
    MergeWeight = W;

    MergeMean[0] = Weight[0];
    MergeWeight[0] = 0;
    for (int Iter = 1; Iter<=Last && Iter<MergeMean.Len(); ++Iter) {
        MergeWeight[Iter] = 0; // zero out merge weights
        MergeMean[Iter] = MergeMean[Iter-1] + Weight[Iter]; // stash cumulative dist
    }

    Min = TMath::Mn(Min, Mean[0]);
    if (LastN < Mean.Len()) {
        Max = TMath::Mx(Max, Mean[LastN]);
    }
}

double TTDigest::MergeCentroid(double& Sum, double& K1, double& Wt, double& Ut) {
    double K2 = Integrate((double)Nc, Sum/TotalSum);
        if (K2 - K1 <= 1.0 || MergeWeight[Last] == 0.0) {
            // merge into existing centroid if centroid index difference (k2-k1)
            // is within 1 or if current centroid is empty
            MergeWeight[Last] += Wt;
            MergeMean[Last] += (Ut - MergeMean[Last]) * Wt / MergeWeight[Last];
        } else {
            // otherwise create a new centroid
            ++Last;
            MergeMean[Last] = Ut;
            MergeWeight[Last] = Wt;
            K1 = Integrate((double)Nc, (Sum - Wt)/TotalSum);
        }

    return K1;
};

double TTDigest::Integrate(const double& Nc, const double& Q_) const {
    // First, scale and bias the quantile domain to [-1, 1]
    // Next, bias and scale the arcsin range to [0, 1]
    // This gives us a [0,1] interpolant following the arcsin shape
    // Finally, multiply by centroid count for centroid scale value
    return Nc * (asin(2 * Q_ - 1) + TMath::Pi / 2) / TMath::Pi;
}

int TTDigest::Bisect(const TFltV& A, const double& X, int& Low, int& Hi) const {
    while (Low < Hi) {
        TInt Mid = (Low + Hi) >> 1;
        if (A[Mid] < X) {
            Low = Mid + 1;
        }
        else {
            Hi = Mid;
        }
    }
    return Low;
}

double TTDigest::Boundary(const int& I, const int& J, const TFltV& U, const TFltV& W) const {
    return U[I] + (U[J] - U[I]) * W[I] / (W[I] + W[J]);
}

int TTDigest::NumTemp(const int& N) const {
    int Lo = 1, Hi = N, Mid;
    while (Lo < Hi) {
        Mid = (Lo + Hi) >> 1;
        if (N > Mid * TMath::Log(Mid) / TMath::Log(2)) {
            Lo = Mid + 1;
        }
        else {
            Hi = Mid;
        }
  }
  return Lo;
}

void TTDigest::Print() const {
    printf("\n MinPointsInit: %d\n Nc: %d\n Size: %d\n Last: %d\n TotalSum: %g\n"
        " Weight.Len(): %d\n Mean.Len(): %d\n Min: %g\n Max: %g\n MergeWeight.Len(): %d\n"
        " MergeMean.Len(): %d\n Tempsize: %d\n UnmergedSum: %g\n TempLast: %d\n"
        " TempWeight.Len(): %d\n TempMean.Len(): %d\n Updates: %d\n\n",
        MinPointsInit.Val, Nc.Val, Size.Val, Last.Val, TotalSum.Val,
        Weight.Len(), Mean.Len(), Min.Val, Max.Val, MergeWeight.Len(),
        MergeMean.Len(), Tempsize.Val, UnmergedSum.Val, TempLast.Val,
        TempWeight.Len(), TempMean.Len(), Updates.Val);
    for (int ElN = 0; ElN < Weight.Len(); ElN++) {
        printf("c:%g, w:%g\n", Mean[ElN].Val, Weight[ElN].Val);
    }
}

uint TTDigest::GetSummarySize() const {
    return Last + 1;
}

uint64 TTDigest::GetMemUsed() const {
    return sizeof(TTDigest) +
        TMemUtils::GetExtraMemberSize(MinPointsInit) +
        TMemUtils::GetExtraMemberSize(Nc) +
        TMemUtils::GetExtraMemberSize(Size) +
        TMemUtils::GetExtraMemberSize(Last) +
        TMemUtils::GetExtraMemberSize(TotalSum) +
        TMemUtils::GetExtraMemberSize(Weight) +
        TMemUtils::GetExtraMemberSize(Mean) +
        TMemUtils::GetExtraMemberSize(Min) +
        TMemUtils::GetExtraMemberSize(Max) +
        TMemUtils::GetExtraMemberSize(MergeWeight) +
        TMemUtils::GetExtraMemberSize(MergeMean) +
        TMemUtils::GetExtraMemberSize(Tempsize) +
        TMemUtils::GetExtraMemberSize(UnmergedSum) +
        TMemUtils::GetExtraMemberSize(TempLast) +
        TMemUtils::GetExtraMemberSize(TempWeight) +
        TMemUtils::GetExtraMemberSize(TempMean) +
        TMemUtils::GetExtraMemberSize(Updates);
}

/////////////////////////////////
// TChiSquare

TChiSquare::TChiSquare(const PJsonVal& ParamVal): P(TFlt::PInf) {
    // P value is set to infinity by default (null hypothesis is not rejected)
    EAssertR(ParamVal->IsObjKey("degreesOfFreedom"), "TChiSquare: degreesOfFreedom key missing!");
    // degrees of freedom
    DegreesOfFreedom = ParamVal->GetObjInt("degreesOfFreedom");
}

void TChiSquare::Print() const {
    printf("Chi2 = %g", Chi2.Val);
    printf("P = %g", P.Val);
}

void TChiSquare::Update(const TFltV& OutValVX, const TFltV& OutValVY) {
    TStatFun::ChiSquare(OutValVX, OutValVY, DegreesOfFreedom, Chi2, P);
}

void TChiSquare::LoadState(TSIn& SIn) {
    Chi2.Load(SIn);
    P.Load(SIn);
}

void TChiSquare::SaveState(TSOut& SOut) const {
    Chi2.Save(SOut);
    P.Save(SOut);
}

///////////////////////////////
/// Slotted histogram
TSlottedHistogram::TSlottedHistogram(const uint64 _Period, const uint64 _Slot, const int _Bins) {
    PeriodLen = _Period;
    SlotGran = _Slot;
    Bins = _Bins;
    uint64 Slots = PeriodLen / SlotGran;
    Dat.Gen((int)Slots);
    for (int i = 0; i < Dat.Len(); i++) {
        Dat[i] = TSignalProc::TOnlineHistogram(0, Bins, Bins, false, false);
    }
}

void TSlottedHistogram::Reset() {
    for (int HistN = 0; HistN < Dat.Len(); HistN++) {
        Dat[HistN].Reset();
    }
}

void TSlottedHistogram::LoadState(TSIn& SIn) {
    PeriodLen.Load(SIn);
    SlotGran.Load(SIn);
    Bins.Load(SIn);
    Dat.Load(SIn);
}

void TSlottedHistogram::SaveState(TSOut& SOut) const {
    PeriodLen.Save(SOut);
    SlotGran.Save(SOut);
    Bins.Save(SOut);
    Dat.Save(SOut);
}

void TSlottedHistogram::Add(const uint64& Ts, const int& Val) {
    int Idx = GetIdx(Ts);
    Dat[Idx].Increment(Val);
}

void TSlottedHistogram::Remove(const uint64& Ts, const int& Val) {
    int Idx = GetIdx(Ts);
    Dat[Idx].Decrement(Val);
}

void TSlottedHistogram::GetStats(const uint64 TsMin, const uint64 TsMax, TFltV& Dest) {
    EAssertR(TsMax > TsMin, "Invalid period query in TSlottedHistogram. TsMax <= TsMin");
    EAssertR(TsMax - PeriodLen < TsMin, "Invalid period query in TSlottedHistogram. TsMax - period >= TsMin");
    Dest.Clr();
    uint64 TsMin2 = (TsMin / SlotGran) * SlotGran;
    uint64 TsMax2 = (TsMax / SlotGran) * SlotGran;
    for (uint64 i = TsMin2; i <= TsMax2; i += SlotGran) {
        int Idx = GetIdx(i);
        TFltV Tmp;
        Dat[Idx].GetCountV(Tmp);
        if (Dest.Len() < Tmp.Len()) {
            Dest.Gen(Tmp.Len());
        }
        for (int j = 0; j < Tmp.Len(); j++) {
            Dest[j] += Tmp[j];
        }
    }
}

}

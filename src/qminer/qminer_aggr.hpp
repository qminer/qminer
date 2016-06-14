///////////////////////////////
// Time series window buffer.
template <class TVal>
void TWinBuf<TVal>::OnAddRec(const TRec& Rec) {
    InitP = true;

    uint64 Timestamp_ = Rec.GetFieldTmMSecs(TimeFieldId);
    OnTime(Timestamp_);
}

template <class TVal>
void TWinBuf<TVal>::OnTime(const uint64& TmMsec) {
    InitP = true;

    Timestamp = TmMsec;

    A = B;
    // B = first record ID in the buffer, or first record ID after the buffer (indicates an empty buffer)
    while (BeforeBuffer(B, Timestamp)) {
        B++;
    }

    C = D;
    // D = the first record ID after the buffer
    while (!AfterBuffer(D, Timestamp)) {
        D++;
    }

    // Call update on all incomming records, which includes records that skipped the buffer (both incomming and outgoing at the same time)
    // C + Skip, D - 1
    for (uint64 RecId = C; RecId < D; RecId++) {
        RecUpdate(RecId);
    }
    //Print(true);
}

template <class TVal>
TWinBuf<TVal>::TWinBuf(const TWPt<TBase>& Base, const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal) {
    // parse out input and output fields
    ParamVal->AssertObjKeyStr("store", __FUNCTION__);
    TStr StoreNm = ParamVal->GetObjStr("store");
    Store = Base->GetStoreByStoreNm(StoreNm);   
    // validate object has key, key is string. input: name
    ParamVal->AssertObjKeyStr("timestamp", __FUNCTION__);   
    TStr TimeFieldNm = ParamVal->GetObjStr("timestamp");
    TimeFieldId = Store->GetFieldId(TimeFieldNm);
    ParamVal->AssertObjKeyNum("winsize", __FUNCTION__);
    WinSizeMSecs = ParamVal->GetObjUInt64("winsize");
    DelayMSecs = ParamVal->GetObjUInt64("delay", 0);
    // make sure parameters make sense
    QmAssertR(Store->GetFieldDesc(TimeFieldId).IsTm(), "[Window buffer] field " + TimeFieldNm + " not of type 'datetime'");
}

template <class TVal>
void TWinBuf<TVal>::LoadState(TSIn& SIn) {
    InitP.Load(SIn);
    A.Load(SIn);
    B.Load(SIn);
    C.Load(SIn);
    D.Load(SIn);
    Timestamp.Load(SIn);
    TestValid(); // checks if the buffer exists in store
}

template <class TVal>
void TWinBuf<TVal>::SaveState(TSOut& SOut) const {
    InitP.Save(SOut);
    A.Save(SOut);
    B.Save(SOut);
    C.Save(SOut);
    D.Save(SOut);
    Timestamp.Save(SOut);
}

template <class TVal>
void TWinBuf<TVal>::Reset() {
    InitP = false;
    A = Store->GetRecs() == 0 ? 0 : Store->GetLastRecId() + 1;
    B = Store->GetRecs() == 0 ? 0 : Store->GetLastRecId() + 1;
    C = Store->GetRecs() == 0 ? 0 : Store->GetLastRecId() + 1;
    D = Store->GetRecs() == 0 ? 0 : Store->GetLastRecId() + 1;
    Timestamp = 0;
}

template <class TVal>
TVal TWinBuf<TVal>::GetInVal() const {
    EAssertR(IsInit(), "WinBuf not initialized yet!"); 
    EAssertR(B < D, "WinBuf is empty - GetInVal() isn't valid! You need to reimplement OnAdd "
        "of the aggregate that is connected to WinBuf to use GetInValV instead of GetInVal!"); 
    EAssertR(Store->IsRecId(D - 1), "WinBuf::GetInVal record not in store! Possible reason: store is "
        "windowed and window is too small and it does not fully contain the buffer"); 
    return GetRecVal(D - 1);
}

template <class TVal>
uint64 TWinBuf<TVal>::GetInTmMSecs() const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    EAssertR(B < D, "WinBuf is empty - GetInTmMSecs() isn't valid! "
        "You need to reimplement OnAdd of the aggregate that is connected to WinBuf to use "
        "GetInTmMSecsV instead of GetInTmMSecs!");
    EAssertR(Store->IsRecId(D - 1), "WinBuf::GetInTmMSecs record not in store! Possible reason: "
        "store is windowed and window is too small and it does not fully contain the buffer");
    return Time(D - 1);
}


template <class TVal>
void TWinBuf<TVal>::GetInValV(TVec<TVal>& ValV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Skip = B > C ? int(B - C) : 0;
    int UpdateRecords = int(D - C) - Skip;
    if (ValV.Len() != UpdateRecords) { ValV.Gen(UpdateRecords); }
    // iterate
    if (UpdateRecords > 0) {
        EAssertR(Store->IsRecId(C + Skip) && Store->IsRecId(C + Skip + UpdateRecords - 1), 
            "WinBuf::GetInValV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < UpdateRecords; RecN++) {
        ValV[RecN] = GetRecVal(C + Skip + RecN);
    }
}

template <class TVal>
void TWinBuf<TVal>::GetInTmMSecsV(TUInt64V& MSecsV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Skip = B > C ? int(B - C) : 0;
    int UpdateRecords = int(D - C) - Skip;
    if (MSecsV.Len() != UpdateRecords) { MSecsV.Gen(UpdateRecords); }
    // iterate
    if (UpdateRecords > 0) {
        EAssertR(Store->IsRecId(C + Skip) && Store->IsRecId(C + Skip + UpdateRecords - 1),
            "WinBuf::GetInTmMSecsV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < UpdateRecords; RecN++) {
        MSecsV[RecN] = Time(C + Skip + RecN);
    }
}

template <class TVal>
void TWinBuf<TVal>::GetOutValV(TVec<TVal>& ValV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Skip = B > C ? int(B - C) : 0;
    int DropRecords = int(B - A) - Skip;
    if (ValV.Len() != DropRecords) { ValV.Gen(DropRecords); }
    // iterate
    if (DropRecords > 0) {
        EAssertR(Store->IsRecId(A) && Store->IsRecId(A + DropRecords - 1),
            "WinBuf::GetOutValV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < DropRecords; RecN++) {
        ValV[RecN] = GetRecVal(A + RecN);
    }
}

template <class TVal>
void TWinBuf<TVal>::GetOutTmMSecsV(TUInt64V& MSecsV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Skip = B > C ? int(B - C) : 0;
    int DropRecords = int(B - A) - Skip;
    if (MSecsV.Len() != DropRecords) { MSecsV.Gen(DropRecords); }
    // iterate
    if (DropRecords > 0) {
        EAssertR(Store->IsRecId(A) && Store->IsRecId(A + DropRecords - 1),
            "WinBuf::GetOutTmMSecsV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < DropRecords; RecN++) {
        MSecsV[RecN] = Time(A + RecN);
    }
}

template <class TVal>
void TWinBuf<TVal>::GetValV(TVec<TVal>& ValV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Len = GetVals();
    if (ValV.Empty()) { ValV.Gen(Len); }
    // iterate
    if (Len > 0) {
        EAssertR(Store->IsRecId(B) && Store->IsRecId(B + Len - 1),
            "WinBuf::GetValV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < Len; RecN++) {
        GetVal(RecN, ValV[RecN]);
    }
}

template <class TVal>
void TWinBuf<TVal>::GetTmV(TUInt64V& MSecsV) const {
    EAssertR(IsInit(), "WinBuf not initialized yet!");
    int Len = GetVals();
    MSecsV.Gen(Len);
    // iterate
    if (Len > 0) {
        EAssertR(Store->IsRecId(B) && Store->IsRecId(B + Len - 1),
            "WinBuf::GetTmV record not in store! Possible reason: store is windowed and window is too "
            "small and it does not fully contain the buffer");
    }
    for (int RecN = 0; RecN < Len; RecN++) {
        MSecsV[RecN] = GetTm(RecN);
    }
}

template <class TVal>
PJsonVal TWinBuf<TVal>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("init", InitP);
    Val->AddToObj("A", A);
    Val->AddToObj("B", B);
    Val->AddToObj("C", C);
    Val->AddToObj("D", D);
    Val->AddToObj("timestamp", TTm::GetTmFromMSecs(Timestamp).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

template <class TVal>
bool TWinBuf<TVal>::TestValid() const {
    // non-initialized model is valid
    if (!InitP()) { return true; }
    uint64 LastRecTmMSecs = Time(Store->GetLastRecId());
    for (uint64 RecId = B; RecId < D; RecId++) {
        if (!InBuffer(RecId, LastRecTmMSecs)) { return false; }
    }
    return true;
}

template <class TVal>
void TWinBuf<TVal>::Print(const bool& PrintState) {
    int Skip = B > C ? int(B - C) : 0;
    printf("TWinBuf: initialized:%s, window:%d, delay: %d, skip:%d\n",
        IsInit() ? "true" : "false", int(WinSizeMSecs), int(DelayMSecs), Skip);
    printf("\033[34m A=%" PRIu64 ", B=%" PRIu64 ", C=%" PRIu64 ", D=%" PRIu64 "\033[0m\n", A.Val, B.Val, C.Val, D.Val);

    //if (A == 0 || B == 0 || C == 0 || D == 0) return;

    printf("Forget interval: %" PRIu64 " - %" PRIu64 "\n", A.Val, B.Val - 1 - Skip);
    printf("Buffer interval: %" PRIu64 " - %" PRIu64 "\n", B.Val, D.Val - 1);
    printf("Update interval: %" PRIu64 " - %" PRIu64 "\n", C.Val + Skip, D.Val - 1);
    if (PrintState && IsInit()) {
        uint64 LastRecId = Store->GetLastRecId();
        PrintInterval(LastRecId, LastRecId, "Last rec:");
        printf("Constraint: [%s - %s]\n",
            TTm::GetTmFromMSecs(Time(LastRecId) - DelayMSecs - WinSizeMSecs).GetWebLogTimeStr().CStr(),
            TTm::GetTmFromMSecs(Time(LastRecId) - DelayMSecs).GetWebLogTimeStr().CStr());
        if (D > 0) {
            PrintInterval(B, D - 1, "New buff:", "32");
            PrintInterval(C + Skip, D - 1, "Just in: ");
        }
        else {
            printf("New buff: [ ]\n");
            printf("Just in: [ ]\n");
        }
        if (B - Skip > 0) {
            PrintInterval(A, B - 1 - Skip, "Just out:");
        }
        else {
            printf("Just out: [ ]\n");
        }
        PrintInterval(D, LastRecId, "Pending: ");
        PrintInterval(Store->GetFirstRecId(), LastRecId, "All recs:");
    }
    printf("\n");
}

template <class TVal>
void TWinBuf<TVal>::PrintInterval(const uint64& StartId, const uint64& EndId, const TStr& Label, const TStr& ModCode) const {
    printf("%s [ \033[1;%sm", Label.CStr(), ModCode.CStr());
    for (uint64 RecId = StartId; RecId <= EndId; RecId++) {
        printf("(%s), ", TTm::GetTmFromMSecs(Time(RecId)).GetWebLogTimeStr().CStr());
    }
    printf("\033[0m]\n");
}

///////////////////////////////
// Windowed Stream aggregates
template <class TSignalType>
void TWinAggr<TSignalType>::OnStep() {
    TFltV OutValV; InAggrVal->GetOutValV(OutValV);
    TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
    if (InAggr->IsInit()) {
        if (!InAggrVal->DelayedP()) {
            Signal.Update(InAggrVal->GetInVal(), InAggrVal->GetInTmMSecs(), OutValV, OutTmMSecsV);
        } else {
            TFltV InValV; InAggrVal->GetInValV(InValV);
            TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
            Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV);
        }
    };
}

template <class TSignalType>
TWinAggr<TSignalType>::TWinAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal) {
    
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrVal = Cast<TStreamAggrOut::IFltTmIO>(InAggr);
}

template <class TSignalType>
PJsonVal TWinAggr<TSignalType>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Signal.GetValue());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
// Window moving average on numeric time series
template <>
inline void TWinAggr<TSignalProc::TMa>::OnStep() {
    TFltV OutValV; InAggrVal->GetOutValV(OutValV);
    TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
    if (InAggr->IsInit()) {
        if (!InAggrVal->DelayedP()) {
            Signal.Update(InAggrVal->GetInVal(), InAggrVal->GetInTmMSecs(),
                OutValV, OutTmMSecsV, InAggrVal->GetVals());
        } else {
            TFltV InValV; InAggrVal->GetInValV(InValV);
            TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
            Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV, InAggrVal->GetVals());
        }
    }
}

///////////////////////////////
// Window variance on numeric time series
template <>
inline void TWinAggr<TSignalProc::TVar>::OnStep() {
    TFltV OutValV; InAggrVal->GetOutValV(OutValV);
    TUInt64V OutTmMSecsV; InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
    if (InAggr->IsInit()) {
        if (!InAggrVal->DelayedP()) {
            Signal.Update(InAggrVal->GetInVal(), InAggrVal->GetInTmMSecs(),
                OutValV, OutTmMSecsV, InAggrVal->GetVals());
        } else {
            TFltV InValV; InAggrVal->GetInValV(InValV);
            TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
            Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV, InAggrVal->GetVals());
        }
    }
}

///////////////////////////////
/// Windowed stream aggregates
template <class TSignalType>
void TWinAggrSpVec<TSignalType>::OnStep() {
    TVec<TIntFltKdV> OutValV;
    InAggrVal->GetOutValV(OutValV);
    TUInt64V OutTmMSecsV;
    InAggrVal->GetOutTmMSecsV(OutTmMSecsV);
    if (InAggr->IsInit()) {
        if (!InAggrVal->DelayedP()) {
            Signal.Update(InAggrVal->GetInVal(), InAggrVal->GetInTmMSecs(), OutValV, OutTmMSecsV);
        } else {
            TVec<TIntFltKdV> InValV; InAggrVal->GetInValV(InValV);
            TUInt64V InTmMSecsV; InAggrVal->GetInTmMSecsV(InTmMSecsV);
            Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV);
        }
    };
}

template <class TSignalType>
TWinAggrSpVec<TSignalType>::TWinAggrSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal) {
    
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrVal = Cast<TStreamAggrOut::IValTmIO<TIntFltKdV>>(InAggr);
}

template <class TSignalType>
PJsonVal TWinAggrSpVec<TSignalType>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Signal.GetJson());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

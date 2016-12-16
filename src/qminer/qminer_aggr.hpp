///////////////////////////////
// Time series window buffer with memory.
template <class TVal>
void TWinBufMem<TVal>::UpdateVal() {
    // get new value
    DelayQ.Push(TPair<TUInt64, TVal>(InAggrTm->GetTmMSecs(), GetVal()));
    // once we read one input we are initialized
    InitP = true;
}

template <class TVal>
void TWinBufMem<TVal>::UpdateTime() {
    // first we clear existing in/out placeholders
    InValV.Clr(); InTmMSecsV.Clr();
    OutValV.Clr(); OutTmMSecsV.Clr();
    // update the current timestamps
    TmMSecs = InAggrTm->GetTmMSecs();
    // first we move things from delay to window
    const uint64 StartDelayMSecs = TmMSecs - DelayMSecs;
    while (!DelayQ.Empty() && DelayQ.Front().Val1 <= StartDelayMSecs) {
        // copy element from the front of the delay to the back of the window queue
        WindowQ.Push(DelayQ.Front());
        // add to the list of new elements in the window
        InValV.Add(DelayQ.Front().Val2);
        InTmMSecsV.Add(DelayQ.Front().Val1);
        // remove the front element from the delay queue
        DelayQ.Pop();
    }
    // then we remove old stuff from window
    const uint64 StartWinMSecs = TmMSecs - DelayMSecs - WinSizeMSecs;
    while (!WindowQ.Empty() && WindowQ.Front().Val1 < StartWinMSecs) {
        // add to the list of elements being removed from the window
        OutValV.Add(WindowQ.Front().Val2);
        OutTmMSecsV.Add(WindowQ.Front().Val1);
        // remove from the window
        WindowQ.Pop();
    }
}

template <class TVal>
void TWinBufMem<TVal>::OnAddRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    // Always read new value
    UpdateVal();
    // Update the time buffers
    UpdateTime();
}

template <class TVal>
void TWinBufMem<TVal>::OnTime(const uint64& TmMsec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    // Update value only when set to always update
    if (UpdateType == wbmuAlways) { UpdateVal(); }
    // Update the time buffers
    UpdateTime();
}

template <class TVal>
void TWinBufMem<TVal>::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    // Update value only when set to always update
    if (UpdateType == wbmuAlways) { UpdateVal(); }
    // Update the time buffers
    UpdateTime();
}

template <class TVal>
TWinBufMem<TVal>::TWinBufMem(const TWPt<TBase>& Base, const PJsonVal& ParamVal): TStreamAggr(Base, ParamVal) {
    // parse out input aggregate
    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(ParamVal->IsObjKey("inAggrTm") ?
            ParseAggr(ParamVal, "inAggrTm") : InAggr);
    // when should we pump in new values?
    TStr UpdateTypeStr = ParamVal->GetObjStr("update", "onNewRecord");
    UpdateType = (UpdateTypeStr == "onNewRecord") ? wbmuOnAddRec : wbmuAlways;
    // parse out window parameters
    ParamVal->AssertObjKeyNum("winsize", __FUNCTION__);
    WinSizeMSecs = ParamVal->GetObjUInt64("winsize");
    DelayMSecs = ParamVal->GetObjUInt64("delay", 0);
}

template <class TVal>
void TWinBufMem<TVal>::LoadState(TSIn& SIn) {
    UpdateType = LoadEnum<TWinBufMemUpdate>(SIn);
    WinSizeMSecs.Load(SIn); DelayMSecs.Load(SIn);
    InitP.Load(SIn); TmMSecs.Load(SIn);
    WindowQ.Load(SIn); DelayQ.Load(SIn);
    InValV.Load(SIn); InTmMSecsV.Load(SIn);
    OutValV.Load(SIn); OutTmMSecsV.Load(SIn);
}

template <class TVal>
void TWinBufMem<TVal>::SaveState(TSOut& SOut) const {
    SaveEnum<TWinBufMemUpdate>(SOut, UpdateType);
    WinSizeMSecs.Save(SOut); DelayMSecs.Save(SOut);
    InitP.Save(SOut); TmMSecs.Save(SOut);
    WindowQ.Save(SOut); DelayQ.Save(SOut);
    InValV.Save(SOut); InTmMSecsV.Save(SOut);
    OutValV.Save(SOut); OutTmMSecsV.Save(SOut);
}

template <class TVal>
void TWinBufMem<TVal>::Reset() {
    // no longer initialized
    InitP = false;
    // reset current timestamp
    TmMSecs = 0;
    // reset buffers
    WindowQ.Clr(); DelayQ.Clr();
    InValV.Clr(); InTmMSecsV.Clr();
    OutValV.Clr(); OutTmMSecsV.Clr();
}

template <class TVal>
void TWinBufMem<TVal>::GetValV(TVec<TVal>& ValV) const {
    ValV.Gen(WindowQ.Len(), 0);
    for (int ElN = 0; ElN < WindowQ.Len(); ElN++) {
        ValV.Add(WindowQ[ElN].Val2);
    }
}

template <class TVal>
void TWinBufMem<TVal>::GetTmV(TUInt64V& ValV) const {
    ValV.Gen(WindowQ.Len(), 0);
    for (int ElN = 0; ElN < WindowQ.Len(); ElN++) {
        ValV.Add(WindowQ[ElN].Val1);
    }
}

template <class TVal>
PJsonVal TWinBufMem<TVal>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("init", InitP);
    Val->AddToObj("timestamp", TTm::GetTmFromMSecs(TmMSecs).GetWebLogDateTimeStr(true, "T"));
    Val->AddToObj("delay", DelayQ.Len());
    Val->AddToObj("window", WindowQ.Len());
    Val->AddToObj("inBuffer", InValV.Len());
    Val->AddToObj("outBuffer", OutValV.Len());
    return Val;
}

///////////////////////////////
// Time series window buffer.
template <class TVal>
void TWinBuf<TVal>::OnAddRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr) {
    InitP = true;

    uint64 Timestamp_ = Rec.GetFieldTmMSecs(TimeFieldId);
    OnTime(Timestamp_, CallerAggr);
}

template <class TVal>
void TWinBuf<TVal>::OnTime(const uint64& TmMsec, const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
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

    // Call update on all incomming records, which includes records that skipped the buffer
    // (both incomming and outgoing at the same time)
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
void TWinAggr<TSignalType>::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (InAggr->IsInit()) {
        TFltV InValV; InAggrFltIO->GetInValV(InValV);
        TUInt64V InTmMSecsV; InAggrTmIO->GetInTmMSecsV(InTmMSecsV);
        TFltV OutValV; InAggrFltIO->GetOutValV(OutValV);
        TUInt64V OutTmMSecsV; InAggrTmIO->GetOutTmMSecsV(OutTmMSecsV);
        Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV);
    }
}

template <class TSignalType>
TWinAggr<TSignalType>::TWinAggr(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal) {

    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrTmIO = Cast<TStreamAggrOut::ITmIO>(InAggr);
    InAggrFltIO = Cast<TStreamAggrOut::IFltIO>(InAggr);
}

template <class TSignalType>
PJsonVal TWinAggr<TSignalType>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Signal.GetValue());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

///////////////////////////////
/// Windowed stream aggregates
template <class TSignalType>
void TWinAggrSpVec<TSignalType>::OnStep(const TWPt<TStreamAggr>& CallerAggr) {
    TScopeStopWatch StopWatch(ExeTm);
    if (InAggr->IsInit()) {
        TVec<TIntFltKdV> InValV; InAggrSparseVecIO->GetInValV(InValV);
        TUInt64V InTmMSecsV; InAggrTmIO->GetInTmMSecsV(InTmMSecsV);
        TVec<TIntFltKdV> OutValV; InAggrSparseVecIO->GetOutValV(OutValV);
        TUInt64V OutTmMSecsV; InAggrTmIO->GetOutTmMSecsV(OutTmMSecsV);
        Signal.Update(InValV, InTmMSecsV, OutValV, OutTmMSecsV);
    };
}

template <class TSignalType>
TWinAggrSpVec<TSignalType>::TWinAggrSpVec(const TWPt<TBase>& Base, const PJsonVal& ParamVal):
        TStreamAggr(Base, ParamVal) {

    InAggr = ParseAggr(ParamVal, "inAggr");
    InAggrTm = Cast<TStreamAggrOut::ITm>(InAggr);
    InAggrTmIO = Cast<TStreamAggrOut::ITmIO>(InAggr);
    InAggrSparseVecIO = Cast<TStreamAggrOut::IValIO<TIntFltKdV>>(InAggr);
}

template <class TSignalType>
PJsonVal TWinAggrSpVec<TSignalType>::SaveJson(const int& Limit) const {
    PJsonVal Val = TJsonVal::NewObj();
    Val->AddToObj("Val", Signal.GetJson());
    Val->AddToObj("Time", TTm::GetTmFromMSecs(GetTmMSecs()).GetWebLogDateTimeStr(true, "T"));
    return Val;
}

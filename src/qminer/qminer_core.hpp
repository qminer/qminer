///////////////////////////////
/// Record Set.
template <class TFilter>
void TRecSet::FilterBy(const TFilter& Filter) {
    // prepare an empty key-dat vector for storing records that pass the filter
    const int Recs = GetRecs();
    TUInt64IntKdV NewRecIdFqV(Recs, 0);
    for (int RecN = 0; RecN < Recs; RecN++) {
        const TUInt64IntKd& RecIdFq = RecIdFqV[RecN];
        const TRec Rec = GetRec(RecN);
        // check the filter
        if (Filter.Filter(Rec)) { NewRecIdFqV.Add(RecIdFq); }
    }
    // overwrite old result vector with filtered list
    RecIdFqV = NewRecIdFqV;
}

template <class TSplitter>
TVec<PRecSet> TRecSet::SplitBy(const TSplitter& Splitter) const {
    TRecSetV ResV;
    // if no records, nothing to do
    if (Empty()) { return ResV; }
    // initialize with the first record
    TUInt64IntKdV NewRecIdFqV; NewRecIdFqV.Add(RecIdFqV[0]);
    // go over the rest and see when to split
    for (int RecN = 1; RecN < GetRecs(); RecN++) {
        if (Splitter(RecIdFqV[RecN-1], RecIdFqV[RecN])) {
            // we need to split, first we create record set for all existing records
            ResV.Add(TRecSet::New(Store, NewRecIdFqV));
            // and initialize a new one
            NewRecIdFqV.Clr(false);
        }
        // add new record to the next record set
        NewRecIdFqV.Add(RecIdFqV[RecN]);
    }
    // add last record set to the result list
    ResV.Add(TRecSet::New(GetStore(), NewRecIdFqV));
    // done
    return ResV;
}

///////////////////////////////
// B-Tree Index
template <class TVal>
void TBTreeIndex<TVal>::AddKey(const TVal& Val, const uint64& RecId) {
    BTree.Add(TTreeVal(Val, RecId));
}

template <class TVal>
void TBTreeIndex<TVal>::DelKey(const TVal& Val, const uint64& RecId) {
    BTree.Del(TTreeVal(Val, RecId));
}

template <class TVal>
void TBTreeIndex<TVal>::SearchRange(const TPair<TVal, TVal>& RangeMinMax, TUInt64V& RecIdV) const {

    TVec<TTreeVal> ResValRecIdV;
    // execute query
    BTree.RangeQuery(TTreeVal(RangeMinMax.Val1, 0), TTreeVal(RangeMinMax.Val2, TUInt64::Mx), ResValRecIdV);
    // parse out record ids
    RecIdV.Gen(ResValRecIdV.Len(), 0);
    for (int ResN = 0; ResN < ResValRecIdV.Len(); ResN++) {
        RecIdV.Add(ResValRecIdV[ResN].Val2);
    }
}

///////////////////////////////
/// QMiner Index Frequency Summation Item Handler
template <class TQmGixItem>
void TIndex::TQmGixSumItemHandler<TQmGixItem>::Merge(TVec<TQmGixItem>& ItemV, const bool& IsLocal) const {
    if (ItemV.Empty()) { return; } // nothing to do in this case
    if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted
    // merge counts
    int LastItemN = 0; bool ZeroP = false;
    for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
        if (ItemV[ItemN].Key != ItemV[ItemN - 1].Key) {
            LastItemN++;
            ItemV[LastItemN] = ItemV[ItemN];
        } else {
            ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
        }
        ZeroP = ZeroP || (ItemV[LastItemN].Dat <= 0);
    }
    // remove items with zero count
    if (ZeroP) {
        int LastIndN = 0;
        for (int ItemN = 0; ItemN < LastItemN + 1; ItemN++) {
            const TQmGixItem& Item = ItemV[ItemN];
            if (Item.Dat.Val > 0 || (IsLocal && Item.Dat.Val < 0)) {
                ItemV[LastIndN] = Item;
                LastIndN++;
            } else if (Item.Dat.Val < 0) {
                TEnv::Error->OnStatusFmt("Warning: negative item count %d:%d!", (int)Item.Key, (int)Item.Dat);
            }
        }
        ItemV.Reserve(ItemV.Reserved(), LastIndN);
    } else {
        ItemV.Reserve(ItemV.Reserved(), LastItemN + 1);
    }
}

///////////////////////////////
/// QMiner Index Frequency Summation Merger
template <class TQmGixItem, class TQmGixResItem>
void TIndex::TQmGixSumMerger<TQmGixItem, TQmGixResItem>::Union(
        TVec<TQmGixResItem>& MainV, const TVec<TQmGixResItem>& JoinV) const {

    TVec<TQmGixResItem> ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixResItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixResItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ResV.Add(Val1); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; }
        else { ResV.Add(TQmGixResItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
        ResV.Add(MainV.GetVal(RestValN1));
    }
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
        ResV.Add(JoinV.GetVal(RestValN2));
    }
    MainV = ResV;
}

template <class TQmGixItem, class TQmGixResItem>
void TIndex::TQmGixSumMerger<TQmGixItem, TQmGixResItem>::Intrs(
        TVec<TQmGixResItem>& MainV, const TVec<TQmGixResItem>& JoinV) const {

    TVec<TQmGixResItem> ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixResItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixResItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
        else { ResV.Add(TQmGixResItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    MainV = ResV;
}

template <class TQmGixItem, class TQmGixResItem>
void TIndex::TQmGixSumMerger<TQmGixItem, TQmGixResItem>::Minus(const TVec<TQmGixResItem>& MainV,
        const TVec<TQmGixResItem>& JoinV, TVec<TQmGixResItem>& ResV) const {

    MainV.Diff(JoinV, ResV);
}

///////////////////////////////
/// Specialization for case when TQmGixItem == TQmGixResItem
template <class TQmGixItem>
void TIndex::TQmGixSumWithFqMerger<TQmGixItem>::Def(const TQmGixKey& Key,
        TVec<TQmGixItem>& MainV, TVec<TQmGixItem>& ResV) const {

    ResV.MoveFrom(MainV);
}

///////////////////////////////
/// Specialization for case when index has implied frequency of 1 (e.g. tiny)
template <class TQmGixItem, class TQmGixResItem>
void TIndex::TQmGixSumWithoutFqMerger<TQmGixItem, TQmGixResItem>::Def(const TQmGixKey& Key,
        TVec<TQmGixItem>& MainV, TVec<TQmGixResItem>& ResV) const {

    ResV.Gen(MainV.Len(), 0);
    for (TQmGixItem& Item : MainV) {
        ResV.Add(TQmGixResItem(Item.Val, 1));
    }
}

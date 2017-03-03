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
/// QMiner-Index-Default-Merger
template <class TQmGixItem>
void TIndex::TQmGixSumMerger<TQmGixItem>::Union(TVec<TQmGixItem>& MainV, const TVec<TQmGixItem>& JoinV) const {
    TVec<TQmGixItem> ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ResV.Add(Val1); ValN1++; }
        else if (Val1 > Val2) { ResV.Add(Val2); ValN2++; }
        else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
        ResV.Add(MainV.GetVal(RestValN1));
    }
    for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
        ResV.Add(JoinV.GetVal(RestValN2));
    }
    MainV = ResV;
}

template <class TQmGixItem>
void TIndex::TQmGixSumMerger<TQmGixItem>::Intrs(TVec<TQmGixItem>& MainV, const TVec<TQmGixItem>& JoinV) const {
    TVec<TQmGixItem> ResV; int ValN1 = 0; int ValN2 = 0;
    while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
        const TQmGixItem& Val1 = MainV.GetVal(ValN1);
        const TQmGixItem& Val2 = JoinV.GetVal(ValN2);
        if (Val1 < Val2) { ValN1++; }
        else if (Val1 > Val2) { ValN2++; }
        else { ResV.Add(TQmGixItem(Val1.Key, Val1.Dat + Val2.Dat)); ValN1++; ValN2++; }
    }
    MainV = ResV;
}

template <class TQmGixItem>
void TIndex::TQmGixSumMerger<TQmGixItem>::Minus(const TVec<TQmGixItem>& MainV,
        const TVec<TQmGixItem>& JoinV, TVec<TQmGixItem>& ResV) const {

    MainV.Diff(JoinV, ResV);
}

template <class TQmGixItem>
void TIndex::TQmGixSumMerger<TQmGixItem>::Merge(TVec<TQmGixItem>& ItemV, const bool& IsLocal) const {
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

template <class TQmGixItem>
TPt<TGixExpItem<TIndex::TQmGixKey, TQmGixItem, TIndex::TQmGixSumMerger<TQmGixItem> > >
TIndex::ToExpItem(const TQueryItem& QueryItem) const {
    // local typedefs
    typedef TGixExpItem<TQmGixKey, TQmGixItem, TQmGixSumMerger<TQmGixItem> > TQmGixExpItem;
    // process item based on its type
    if (QueryItem.IsLeafGix()) {
        // we have a leaf, make it into expresion item
        if (QueryItem.IsEqual()) {
            // ==
            TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
            return TQmGixExpItem::NewAndV(AllKeyV);
        } else if (QueryItem.IsGreater()) {
            // >=
            TKeyWordV AllGreaterV;
            IndexVoc->GetAllGreaterV(QueryItem.GetKeyId(), QueryItem.GetWordId(), AllGreaterV);
            return TQmGixExpItem::NewOrV(AllGreaterV);
        } else if (QueryItem.IsLess()) {
            // <=
            TKeyWordV AllLessV;
            IndexVoc->GetAllLessV(QueryItem.GetKeyId(), QueryItem.GetWordId(), AllLessV);
            return TQmGixExpItem::NewOrV(AllLessV);
        } else if (QueryItem.IsNotEqual()) {
            // !=
            TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
            return TQmGixExpItem::NewNot(TQmGixExpItem::NewAndV(AllKeyV));
        } else if (QueryItem.IsWildChar()) {
            // ~
            TKeyWordV AllKeyV; QueryItem.GetKeyWordV(AllKeyV);
            return TQmGixExpItem::NewOrV(AllKeyV);
        } else {
            // unknown operator
            throw TQmExcept::New("Index: Unknown query item operator");
        }
    } else if (QueryItem.IsAnd()) {
        // we have a vector of AND items
        TVec<TPt<TQmGixExpItem> > ExpItemV(QueryItem.GetItems(), 0);
        for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
            ExpItemV.Add(ToExpItem<TQmGixItem>(QueryItem.GetItem(ItemN)));
        }
        return TQmGixExpItem::NewAndV(ExpItemV);
    } else if (QueryItem.IsOr()) {
        // we have a vector of OR items
        TVec<TPt<TQmGixExpItem> > ExpItemV(QueryItem.GetItems(), 0);
        for (int ItemN = 0; ItemN < QueryItem.GetItems(); ItemN++) {
            ExpItemV.Add(ToExpItem<TQmGixItem>(QueryItem.GetItem(ItemN)));
        }
        return TQmGixExpItem::NewOrV(ExpItemV);
    } else if (QueryItem.IsNot()) {
        // we have a negation (can have only one child item!)
        QmAssert(QueryItem.GetItems() == 1);
        return TQmGixExpItem::NewNot(ToExpItem<TQmGixItem>(QueryItem.GetItem(0)));
    } else {
        // unknow handle query item type
        const int QueryItemType = (int)QueryItem.GetType();
        throw TQmExcept::New(TStr::Fmt("Index: QueryItem of type %d which must be handled outside TIndex", QueryItemType));
    }
    return TQmGixExpItem::NewEmpty();
}

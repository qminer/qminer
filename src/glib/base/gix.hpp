/////////////////////////////////////////////////
/// General Inverted Index Item Set.
template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::TChildInfo::Load(TSIn& SIn) {
    MinItem = TItem(SIn);
    MaxItem = TItem(SIn);
    Len.Load(SIn);
    Pt = TBlobPt(SIn);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::TChildInfo::Save(TSOut& SOut) const {
    MinItem.Save(SOut);
    MaxItem.Save(SOut);
    Len.Save(SOut);
    Pt.Save(SOut);
}

template <class TKey, class TItem>
uint64 TGixItemSet<TKey, TItem>::TChildInfo::GetMemUsed() const {
    return sizeof(TChildInfo) +
        TMemUtils::GetExtraMemberSize(MinItem) +
        TMemUtils::GetExtraMemberSize(MaxItem) +
        TMemUtils::GetExtraMemberSize(Len) +
        TMemUtils::GetExtraMemberSize(Pt) +
        TMemUtils::GetExtraMemberSize(LoadedP) +
        TMemUtils::GetExtraMemberSize(DirtyP);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::LoadChildVector(const int& ChildN) const {
    if (!ChildInfoV[ChildN].LoadedP) {
        // load child vector from disk
        Gix->GetChildVector(ChildInfoV[ChildN].Pt, ChildV[ChildN]);
        // mark that it is freshly loaded
        ChildInfoV[ChildN].LoadedP = true;
        ChildInfoV[ChildN].DirtyP = false;
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::LoadChildVectors() const {
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        LoadChildVector(ChildN);
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::RecalcTotalCnt() {
    TotalCnt = ItemV.Len();
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        TotalCnt += ChildInfoV[ChildN].Len;
    }
}

template <class TKey, class TItem>
int TGixItemSet<TKey, TItem>::FirstDirtyChild() {
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        if (ChildInfoV[ChildN].DirtyP && ChildInfoV[ChildN].Len < Gix->GetSplitLenMin()) {
            return ChildN;
        }
        if (ChildInfoV[ChildN].DirtyP && ChildInfoV[ChildN].Len > Gix->GetSplitLenMax()) {
            return ChildN;
        }
    }
    return -1;
}

template <class TKey, class TItem>
int TGixItemSet<TKey, TItem>::GetFirstChildToMerge() {
    // start checking either from 0 or 1 depending on whether we allow the first child to be
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        // the child at least needs to be dirty to be merged
        if (!ChildInfoV[ChildN].DirtyP) {
            continue;
        }
        // if child is not out of the size boundaries it also doesn't need to be merged
        if (ChildInfoV[ChildN].Len >= Gix->GetSplitLenMin() && ChildInfoV[ChildN].Len <= Gix->GetSplitLenMax()) {
            continue;
        }
        // for the first child we might allow it to be extra short, without need for merge
        // when removing oldest items, the first vector will be becoming shorter and shorter
        // and will be removed completely once empty
        if (ChildN == 0 && Gix->CanFirstChildBeUnfilled() && ChildInfoV[ChildN].Len <= Gix->GetSplitLenMax()) {
            continue;
        }
        // otherwise, yes, it needs to be merged
        return ChildN;
    }
    return -1;
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::PushWorkBufferToChildren() {
    // push work-buffer into children array
    int SplitLen = Gix->GetSplitLen();
    while (ItemV.Len() >= SplitLen) {
        // create a vector of SplitLen items
        TVec<TItem> SplitItemV;
        ItemV.GetSubValV(0, SplitLen - 1, SplitItemV);
        // create the child info for the vector and also push the vector to a blob
        TChildInfo ChildInfo(SplitItemV[0], SplitItemV.Last(), SplitLen, Gix->EnlistChildVector(SplitItemV));
        ChildInfo.LoadedP = false;
        ChildInfo.DirtyP = false;
        ChildInfoV.Add(ChildInfo);
        // add an empty vector to ChildV - the data for this vector will be loaded from the blob when necessary
        ChildV.Add(TVec<TItem>());
        ItemV.Del(0, SplitLen - 1);
        DirtyP = true;
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::InjectWorkBufferToChildren() {
    AssertR(ItemV.IsSorted(), "Items in working buffer ItemV should be sorted");
    if (ChildInfoV.Len() > 0 && ItemV.Len() > 0) {
        // find the first Child index into which we need to insert the first value
        // since items in ItemV will most likely have the highest values, it makes sense to go from end backwards
        int ChildN = ChildInfoV.Len()-1;
        const TItem& FirstItem = ItemV[0];
        while (ChildN > 0 && Gix->GetMerger()->IsLt(FirstItem, ChildInfoV[ChildN].MinItem)) {
            ChildN--;
        }
        // go from ChildN onward, inserting items from ItemV into ChildInfoV
        int ItemN = 0;
        TIntSet TouchedVectorH;
        while (ItemN < ItemV.Len()) {
            const TItem& Item = ItemV[ItemN];
            while (ChildN < ChildInfoV.Len() && Gix->GetMerger()->IsLt(ChildInfoV[ChildN].MaxItem, Item)) {
                ChildN++;
            }
            // if val is larger than MaxItem in last ChildInfoV vector, then all remaining values in input buffer will not be inserted into child vectors
            if (ChildN >= ChildInfoV.Len()) {
                break;
            }
            // ok, insert into j-th child
            LoadChildVector(ChildN);
            ChildV[ChildN].Add(Item);
            ChildInfoV[ChildN].Len = ChildV[ChildN].Len();
            ChildInfoV[ChildN].DirtyP = true;
            TouchedVectorH.AddKey(ChildN);
            ItemN++;
        }

        // delete items from work-buffer that have been inserted into child vectors
        if (ItemN > 0) {
            // we made at least one insertion into the children, mark itemset as dirty
            DirtyP = true;
            if (ItemN == ItemV.Len()) {
                // we inserted all items into children - clear the working buffer
                ItemV.Clr();
            } else {
                // clear only the items that were already inserted into children
                ItemV.Del(0, ItemN - 1);
            }
        }

        // go over all the vectors that we modified and merge + update stats for them
        for (int KeyId = TouchedVectorH.FFirstKeyId(); TouchedVectorH.FNextKeyId(KeyId); ) {
            int ind = TouchedVectorH.GetKey(KeyId);
            LoadChildVector(ind); // just in case - they should be in memory at this point anyway
            TVec<TItem>& cd = ChildV[ind];
            Gix->GetMerger()->Merge(cd, false);
            ChildInfoV[ind].Len = cd.Len();
            ChildInfoV[ind].DirtyP = true;
            if (cd.Len() > 0) {
                ChildInfoV[ind].MinItem = cd[0];
                ChildInfoV[ind].MaxItem = cd.Last();
            }
        }
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::PushMergedDataBackToChildren(
        const int& FirstChildToMerge, const TVec<TItem>& MergedItems) {

    int MergedItemN = 0;
    int Remaining = MergedItems.Len() - MergedItemN;
    int ChildN = FirstChildToMerge;
    while (MergedItemN < MergedItems.Len()) {
        if (ChildN < ChildInfoV.Len() && Remaining > Gix->GetSplitLen()) {
            ChildV[ChildN].Clr();
            MergedItems.GetSubValV(MergedItemN, MergedItemN + Gix->GetSplitLen() - 1, ChildV[ChildN]);
            ChildInfoV[ChildN].Len = ChildV[ChildN].Len();
            ChildInfoV[ChildN].MinItem = ChildV[ChildN][0];
            ChildInfoV[ChildN].MaxItem = ChildV[ChildN].Last();
            ChildInfoV[ChildN].DirtyP = true;
            ChildInfoV[ChildN].LoadedP = true;
            MergedItemN += ChildInfoV[ChildN].Len;
            Remaining = MergedItems.Len() - MergedItemN;
            ChildN++;
        } else {
            // put the remaining data into work-buffer
            ItemV.Clr();
            MergedItems.GetSubValV(MergedItemN, MergedItemN + Remaining - 1, ItemV);
            break;
        }
    }

    // remove children that became empty
    // remove them first from BLOB storage
    for (int Ind = ChildN; Ind < ChildInfoV.Len(); Ind++) {
        Gix->DeleteChildVector(ChildInfoV[Ind].Pt);
    }

    // finally remove them from memory
    if (ChildN < ChildInfoV.Len()) {
        ChildInfoV.Del(ChildN, ChildInfoV.Len() - 1);
        ChildV.Del(ChildN, ChildV.Len() - 1);
    }
    DirtyP = true;
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::ProcessDeletes() {
    if (ItemVDel.Len() > 0) {
        TVec<TItem> ItemVNew;
        int ItemVNewI = 0;

        // go over all indices in ItemVDel that represent which items in ItemV are keys to delete
        for (int ItemVDelN = 0; ItemVDelN < ItemVDel.Len(); ItemVDelN++) {
            // get the value to delete
            const TItem& ValToDel = ItemV[ItemVDel[ItemVDelN]];
            // find the children vector from which we need to delete the value.
            // since deletes are often called on the oldest items we immediately test
            // if val is in the first vector - if not, go from last vector backward
            int ChildN = (ChildInfoV.Len() > 0 && Gix->GetMerger()->IsLtE(ValToDel, ChildInfoV[0].MaxItem)) ? 0 : ChildInfoV.Len() - 1;
            while (ChildN >= 0 && Gix->GetMerger()->IsLtE(ValToDel, ChildInfoV[ChildN].MaxItem)) {
                if (Gix->GetMerger()->IsLtE(ChildInfoV[ChildN].MinItem, ValToDel)) {
                    LoadChildVector(ChildN);
                    Gix->GetMerger()->Delete(ValToDel, ChildV[ChildN]);
                    ChildInfoV[ChildN].Len = ChildV[ChildN].Len();
                    ChildInfoV[ChildN].DirtyP = true;
                    // since we have already found and deleted the item, we can stop iterating over children vectors
                    break;
                    // we don't update stats (min & max), because they are still usable.
                }
                ChildN--;
            }
            // copy from ItemV to ItemVNew items up to index ItemVDel[i]
            while (ItemVNewI <= ItemVDel[ItemVDelN]) {
                ItemVNew.Add(ItemV[ItemVNewI++]);
            }
            // it is possible that value val appears multiple times in ItemVNew so we have to delete all it's instances
            Gix->GetMerger()->Delete(ValToDel, ItemVNew);
        }
        // copy the remaining items from the last item to delete to the end of the vector ItemV and copy items to ItemVNew
        while (ItemVNewI < ItemV.Len()) {
            ItemVNew.Add(ItemV[ItemVNewI++]);
        }

        ItemV.Clr();
        ItemVDel.Clr();
        //ItemV.AddVMemCpy(ItemVNew);
        ItemV.AddV(ItemVNew);
        DirtyP = true;
    }
}

template <class TKey, class TItem>
TGixItemSet<TKey, TItem>::TGixItemSet(TSIn& SIn, const TGix<TKey, TItem>* _Gix):
    ItemSetKey(SIn), ItemV(SIn), ChildInfoV(SIn), MergedP(true), DirtyP(false), Gix(_Gix) {

    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        ChildV.Add(TVec<TItem>());
    };
    RecalcTotalCnt();
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Save(TMOut& SOut) {
    // make sure all is merged before saving
    Def();
    // save child vectors separately
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        if (ChildInfoV[ChildN].DirtyP && ChildInfoV[ChildN].LoadedP) {
            ChildInfoV[ChildN].Pt = Gix->StoreChildVector(ChildInfoV[ChildN].Pt, ChildV[ChildN]);
            ChildInfoV[ChildN].DirtyP = false;
        }
    }

    // save item key and set
    ItemSetKey.Save(SOut);
    //ItemV.SaveMemCpy(SOut);
    ItemV.Save(SOut);
    ChildInfoV.Save(SOut);
    DirtyP = false;
}

template <class TKey, class TItem>
uint64 TGixItemSet<TKey, TItem>::GetMemUsed() const {
    return sizeof(TGixItemSet) +
        TMemUtils::GetExtraMemberSize(CRef) +
        TMemUtils::GetExtraMemberSize(ItemSetKey) +
        TMemUtils::GetExtraMemberSize(ItemV) +
        TMemUtils::GetExtraMemberSize(ItemVDel) +
        TMemUtils::GetExtraMemberSize(TotalCnt) +
        TMemUtils::GetExtraMemberSize(ChildInfoV) +
        TMemUtils::GetExtraMemberSize(ChildV) +
        TMemUtils::GetExtraMemberSize(MergedP) +
        TMemUtils::GetExtraMemberSize(DirtyP);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::OnDelFromCache(const TBlobPt& BlobPt, void* Gix) {
    // TODO: is IsReadOnly() test necessary? Isn't the only case when DirtyP == true if we are allowed write access?
    if (!((TGix<TKey, TItem>*)Gix)->IsReadOnly() && DirtyP) {
        ((TGix<TKey, TItem>*)Gix)->StoreItemSet(BlobPt);
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::AddItem(const TItem& NewItem, const bool& NotifyCacheOnlyDelta) {
    // if NotifyCacheOnlyDelta is false we have just added a new itemset and we have to report to gix
    // the base size used by the empty itemset itself
    if (NotifyCacheOnlyDelta == false) {
        Gix->AddToNewCacheSizeInc(GetMemUsed());
    }

    if (IsFull()) {
        // if we will do a cleanup of data we need to update the cache size used
        const uint64 OldSize = GetMemUsed();
        Def();
        if (IsFull()) {
            PushWorkBufferToChildren();
        }
        // TODO: why is next RecalcTotalCnt needed? Def() already calls it if anything is changed. It might be needed only if IsFull() was true.
        RecalcTotalCnt(); // work buffer might have been merged
        Gix->AddToNewCacheSizeInc(OldSize, GetMemUsed());
    }

    if (MergedP) {
        // if itemset is merged and the newly added item is bigger than the last one
        // the itemset remains merged
        if (ItemV.Len() == 0 && ChildInfoV.Len() == 0) {
            // the first item in whole itemset
            MergedP = true;
        } else if (ItemV.Len() == 0 && ChildInfoV.Len() != 0) {
            // compare with the last item of the last child
            MergedP = Gix->GetMerger()->IsLt(ChildInfoV.Last().MaxItem, NewItem);
        } else {
            // compare to the last item in the work buffer
            MergedP = Gix->GetMerger()->IsLt(ItemV.Last(), NewItem);
        }
    }
    const uint64 OldItemVSize = ItemV.GetMemUsed();
    // if first item we are adding to the itemset, we start with size 2 to avoid default of 16
    if (ItemV.Len() == 0) { ItemV.Reserve(2); }
    // add item to the end of the list
    ItemV.Add(NewItem);
    // Update the cache size (for the newly added item). In general we could just add
    // sizeof(TItem) to cache size, however we would underestimate the used size since
    // the arrays allocate extra buffer
    Gix->AddToNewCacheSizeInc(OldItemVSize, ItemV.GetMemUsed());

    DirtyP = true;
    TotalCnt++;
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::AddItemV(const TVec<TItem>& NewItemV) {
    for (int i = 0; i < NewItemV.Len(); i++) {
        AddItem(NewItemV[i]);
    }
}

template <class TKey, class TItem>
const TItem& TGixItemSet<TKey, TItem>::GetItem(const int& ItemN) const {
    AssertR(ItemN >= 0 && ItemN < TotalCnt, TStr() + "Index: " + TInt::GetStr(ItemN) + ", TotalCnt: " + TInt::GetStr(TotalCnt));
    int Offset = ItemN;
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        if (Offset < ChildInfoV[ChildN].Len) {
            // load child vector only if needed
            LoadChildVector(ChildN);
            return ChildV[ChildN][Offset];
        }
        Offset -= ChildInfoV[ChildN].Len;
    }
    return ItemV[Offset];
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::GetItemV(TVec<TItem>& _ItemV) {
    if (ChildInfoV.Len() > 0) {
        // collect data from child itemsets
        LoadChildVectors();
        for (int i = 0; i < ChildInfoV.Len(); i++) {
            //_ItemV.AddVMemCpy(ChildV[i]);
            _ItemV.AddV(ChildV[i]);
        }
    }
    //_ItemV.AddVMemCpy(ItemV);
    _ItemV.AddV(ItemV);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::DelItem(const TItem& Item) {
    if (IsFull()) {
        const uint64 OldSize = GetMemUsed();
        Def();
        if (IsFull()) {
            PushWorkBufferToChildren();
        }
        RecalcTotalCnt(); // work buffer might have been merged
        Gix->AddToNewCacheSizeInc(OldSize, GetMemUsed());
    }

    const uint64 OldSize = ItemVDel.GetMemUsed() + ItemV.GetMemUsed();
    ItemVDel.Add(ItemV.Len());
    ItemV.Add(Item);
    const uint64 NewSize = ItemVDel.GetMemUsed() + ItemV.GetMemUsed();
    Gix->AddToNewCacheSizeInc(OldSize,  NewSize);
    MergedP = false;
    DirtyP = true;
    TotalCnt++;
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Clr() {
    const int OldSize = GetMemUsed();
    if (ChildInfoV.Len() > 0) {
        for (int i = 0; i < ChildInfoV.Len(); i++) {
            Gix->DeleteChildVector(ChildInfoV[i].Pt);
        }
        ChildV.Clr();
        ChildInfoV.Clr();
    }
    ItemV.Clr();
    ItemVDel.Clr();
    MergedP = true;
    DirtyP = true;
    TotalCnt = 0;
    Gix->AddToNewCacheSizeInc(OldSize, GetMemUsed());
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Def() {
    // call merger to pack items, if not merged yet
    if (!MergedP) {
        ProcessDeletes(); // "execute" deletes, possibly leaving some child vectors too short
        Gix->GetMerger()->Merge(ItemV, true); // first do local merge of work-buffer
        DirtyP = true;
        InjectWorkBufferToChildren(); // inject data into child vectors

        int FirstChildToMerge = GetFirstChildToMerge();
        if (FirstChildToMerge >= 0 || (ChildInfoV.Len() > 0 && ItemV.Len() > 0)) {
            if (FirstChildToMerge < 0) {
                FirstChildToMerge = ChildInfoV.Len();
            }

            // collect all data from subsequent child vectors and work-buffer
            TVec<TItem> MergedItems;
            for (int i = FirstChildToMerge; i < ChildInfoV.Len(); i++) {
                LoadChildVector(i);
                MergedItems.AddV(ChildV[i]);
            }
            MergedItems.AddV(ItemV);
            Gix->GetMerger()->Merge(MergedItems, false); // perform global merge

            PushMergedDataBackToChildren(FirstChildToMerge, MergedItems); // now save them back
            PushWorkBufferToChildren(); // it could happen that data in work buffer is still too large
        }

        // in case deletes emptied the first children completely, remove them
        while (ChildInfoV.Len() > 0 && ChildInfoV[0].Len == 0) {
            // remove them first from BLOB storage
            Gix->DeleteChildVector(ChildInfoV[0].Pt);
            // remove it from memory
            ChildInfoV.Del(0);
            ChildV.Del(0);
        }

        RecalcTotalCnt();
        MergedP = true;
    }
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::DefLocal() {
    // call merger to pack items in work buffer, if not merged yet
    if (!MergedP) {
        if (ItemVDel.Len() == 0) { // deletes are not treated as local - merger would get confused
            const int OldItemVLen = ItemV.Len();
            Gix->GetMerger()->Merge(ItemV, true); // perform local merge
            DirtyP = true;
            if (ChildInfoV.Len() > 0 && ItemV.Len() > 0) {
                if (Gix->GetMerger()->IsLt(ChildInfoV.Last().MaxItem, ItemV[0])) {
                    MergedP = true; // local merge achieved global merge
                }
            } else {
                MergedP = true;
            }
            // update the total count - since we have only been modifying the ItemV we can simply
            // update the total count by comparing previous and current number of items in ItemV
            TotalCnt = TotalCnt - OldItemVLen + ItemV.Len();
        }
    }
}

template <class TKey, class TItem>
double TGixItemSet<TKey, TItem>::GetLoadedPerc() const {
    int LoadedCount = 0;
    for (int ChildN = 0; ChildN < ChildInfoV.Len(); ChildN++) {
        if (ChildInfoV[ChildN].LoadedP) { LoadedCount++; }
    }
    return ChildInfoV.Empty() ? 1.0 : (double)LoadedCount / (double)ChildInfoV.Len();
}

#ifdef XTEST
template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Print() const {
    LoadChildVectors();
    printf("TotalCnt=%d\n", TotalCnt);
    printf("len=%d\n", ItemV.Len());
    for (int i = 0; i < ItemV.Len(); i++) {
        printf("   %d=%d\n", i, ItemV[i]);
    }
    if (ItemVDel.Len()> 0) {
        printf("deleted=%d\n", ItemVDel.Len());
        for (int i = 0; i < ItemVDel.Len(); i++) {
            printf("   %d=%d\n", i, ItemVDel[i]);
        }
    }
    for (int j = 0; j < ChildInfoV.Len(); j++) {
        printf("   *** child %d\n", j);
        printf("   *** len %d\n", ChildInfoV[j].Len);
        printf("   *** mem-len %d\n", ChildV[j].Len());
        for (int i = 0; i < ChildV[j].Len(); i++) {
            printf("      %d=%d\n", i, ChildV[j][i]);
        }
    }
}
#endif

/////////////////////////////////////////////////
// General-Inverted-Index
template <class TKey, class TItem>
void TGix<TKey, TItem>::AssertReadOnly() const {
    EAssertR(((Access == faCreate) || (Access == faUpdate)), "Index opened in Read-Only mode!");
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::AddKeyId(const TKey& Key) {
    if (IsKey(Key)) { return KeyIdH.GetDat(Key); }
    // we don't have this key, create an empty item set and return pointer to it
    AssertReadOnly(); // check if we are allowed to write
    PGixItemSet ItemSet = TGixItemSet<TKey, TItem>::New(Key, &Merger, this);
    TBlobPt KeyId = EnlistItemSet(ItemSet);
    KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
    return KeyId;
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::GetKeyId(const TKey& Key) const {
    if (IsKey(Key)) { return KeyIdH.GetDat(Key); }
    // we don't have this key, return empty pointer
    return TBlobPt();
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::GetChildVector(const TBlobPt& KeyId, TVec<TItem>& Dest) const {
    if (KeyId.Empty()) { return; }
    PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
    Dest.Load(*ItemSetSIn);
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) const {
    // check if we are allowed to write
    AssertReadOnly();
    // store the current version to the blob
    TMOut MOut;
    //Data.SaveMemCpy(MOut);
    Data.Save(MOut);
    int ReleasedSize;
    return ItemSetBlobBs->PutBlob(ExistingKeyId, MOut.GetSIn(), ReleasedSize);
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::DeleteChildVector(const TBlobPt& KeyId) const {
    AssertReadOnly(); // check if we are allowed to write
    ItemSetBlobBs->DelBlob(KeyId);    // free space in BLOB
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::EnlistChildVector(const TVec<TItem>& Data) const {
    AssertReadOnly(); // check if we are allowed to write
    TMOut MOut;
    Data.Save(MOut);
    TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
    return res;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::RefreshStats() const {
    Stats.CacheAll = 0;
    Stats.CacheDirty = 0;
    Stats.CacheAllLoadedPerc = 0;
    Stats.CacheDirtyLoadedPerc = 0;
    Stats.AvgLen = 0;

    Stats.MemUsed = this->GetMemUsed();
    TBlobPt BlobPt; PGixItemSet ItemSet;
    void* KeyDatP = ItemSetCache.FFirstKeyDat();
    while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) {
        Stats.CacheAll++;
        const double LoadedPerc = ItemSet->GetLoadedPerc();
        Stats.CacheAllLoadedPerc += LoadedPerc;
        Stats.AvgLen += ItemSet->GetItems();
        if (ItemSet->IsDirty()) {
            Stats.CacheDirty++;
            Stats.CacheDirtyLoadedPerc += LoadedPerc;
        }
    }
    if (Stats.CacheAll > 0) {
        Stats.CacheAllLoadedPerc /= Stats.CacheAll;
        Stats.AvgLen /= Stats.CacheAll;
        if (Stats.CacheDirty > 0) {
            Stats.CacheDirtyLoadedPerc /= Stats.CacheDirty;
        }
    }
}

template <class TKey, class TItem>
TGix<TKey, TItem>::TGix(const TStr& Nm, const TStr& FPath, const TFAccess& _Access,
    const TGixMerger<TKey, TItem>* _Merger, const int64& CacheSize, const int _SplitLen,
    const bool _FirstChildBeUnfilledP, const int _SplitLenMin, const int _SplitLenMax) :
        Access(_Access), Merger(_Merger), ItemSetCache(CacheSize, 1000000, GetVoidThis()),
        SplitLen(_SplitLen), SplitLenMin(_SplitLenMin), SplitLenMax(_SplitLenMax),
        FirstChildBeUnfilledP(_FirstChildBeUnfilledP) {

    // prepare filenames of the GIX datastore
    GixFNm = TStr::GetNrFPath(FPath) + Nm.GetFBase() + ".Gix";
    GixBlobFNm = TStr::GetNrFPath(FPath) + Nm.GetFBase() + ".GixDat";
    // check in what mode should we open
    if (Access == faCreate) {
        // creating a new Gix
        ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, faCreate);
    } else {
        // loading an old Gix and getting it ready for search and update
        EAssert((Access == faUpdate) || (Access == faRdOnly) || (Access == faRestore));
        // load Gix from GixFNm
        TFIn FIn(GixFNm); KeyIdH.Load(FIn);
        // load ItemSets from GixBlobFNm
        ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, Access);
    }
    // we do recounting after 10% change of the cache size
    CacheResetThreshold = int64(0.1 * double(CacheSize));
    NewCacheSizeInc = 0;
    CacheFullP = false;
}

template <class TKey, class TItem>
TGix<TKey, TItem>::~TGix() {
    if ((Access == faCreate) || (Access == faUpdate)) {
        // flush all the latest changes in cache to the disk
        ItemSetCache.Flush();
        // save the rest to GixFNm
        TFOut FOut(GixFNm); KeyIdH.Save(FOut);
    }
}

template <class TKey, class TItem>
TPt<TGixItemSet<TKey, TItem> > TGix<TKey, TItem>::GetItemSet(const TKey& Key) const {
    TBlobPt KeyId = GetKeyId(Key);
    return GetItemSet(KeyId);
}

template <class TKey, class TItem>
TPt<TGixItemSet<TKey, TItem> > TGix<TKey, TItem>::GetItemSet(const TBlobPt& KeyId) const {
    if (KeyId.Empty()) {
        // return empty itemset
        return TGixItemSet<TKey, TItem>::New(TKey(), this);
    }
    PGixItemSet ItemSet;
    if (!ItemSetCache.Get(KeyId, ItemSet)) {
        // have to load it from the hard drive...
        PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
        ItemSet = TGixItemSet<TKey, TItem>::Load(*ItemSetSIn, this);
    }
    // bring the itemset to the top of the cache
    ItemSetCache.Put(KeyId, ItemSet);
    return ItemSet;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::GetItemV(const TKey& Key, TVec<TItem>& ItemV) const {
	PGixItemSet ItemSet = GetItemSet(Key);
	// first call Def() so that we can process some pending actions (like deletes) first
	ItemSet->Def();
	// get the items for the key
	return ItemSet->GetItemV(ItemV);
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::StoreItemSet(const TBlobPt& KeyId) {
    AssertReadOnly(); // check if we are allowed to write
    // get the pointer to the item set
    PGixItemSet ItemSet;
    EAssert(ItemSetCache.Get(KeyId, ItemSet));
    ItemSet->Def();
    if (ItemSet->Empty()) {
        // itemset is empty after all deletes were processed => remove it
        ItemSetBlobBs->DelBlob(KeyId);
        KeyIdH.DelKey(ItemSet->GetKey());
        return TBlobPt(); // return NULL pointer
    } else {
        // store the current version to the blob
        TMOut MOut;
        ItemSet->Save(MOut);
        int ReleasedSize;
        TBlobPt NewKeyId = ItemSetBlobBs->PutBlob(KeyId, MOut.GetSIn(), ReleasedSize);
        // and update the KeyId in the hash table
        KeyIdH.GetDat(ItemSet->GetKey()) = NewKeyId;
        return NewKeyId;
    }
}

/// for deleting itemset from cache and blob
template <class TKey, class TItem>
void TGix<TKey, TItem>::DeleteItemSet(const TKey& Key) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        TBlobPt Pt = KeyIdH.GetDat(Key);
        ItemSetCache.Del(Pt, false);
        ItemSetBlobBs->DelBlob(Pt);
        KeyIdH.DelKey(Key);
    }
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::EnlistItemSet(const PGixItemSet& ItemSet) const {
    AssertReadOnly(); // check if we are allowed to write
    TMOut MOut;
    ItemSet->Save(MOut);
    TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
    return res;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::AddItem(const TKey& Key, const TItem& Item) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        // get the key handle
        TBlobPt KeyId = KeyIdH.GetDat(Key);
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        ItemSet->AddItem(Item);
    } else {
        // we don't have this key, create a new itemset and add new item immidiatelly
        PGixItemSet ItemSet = TGixItemSet<TKey, TItem>::New(Key, this);
        ItemSet->AddItem(Item, false);
        TBlobPt KeyId = EnlistItemSet(ItemSet); // now store this itemset to disk
        KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
        ItemSetCache.Put(KeyId, ItemSet); // add it to cache
    }
    // check if we have to drop anything from the cache
    RefreshMemUsed();
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::AddItemV(const TKey& Key, const TVec<TItem>& ItemV) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        // get the key handle
        TBlobPt KeyId = KeyIdH.GetDat(Key);
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        ItemSet->AddItemV(ItemV);
    } else {
        // we don't have this key, create a new itemset and add new item immidiatelly
        PGixItemSet ItemSet = TGixItemSet<TKey, TItem>::New(Key, this);
        ItemSet->AddItemV(ItemV);
        TBlobPt KeyId = EnlistItemSet(ItemSet); // now store this itemset to disk
        KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
    }
    // check if we have to drop anything from the cache
    RefreshMemUsed();
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::DelItem(const TKey& Key, const TItem& Item) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) { // check if this key exists
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        // clear the items from the ItemSet
        ItemSet->DelItem(Item);
        if (ItemSet->Empty()) {
            DeleteItemSet(Key);
        }
    }
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::Clr(const TKey& Key) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) { // check if this key exists
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        // clear the items from the ItemSet
        ItemSet->Clr();
    }
}

template <class TKey, class TItem>
int TGix<TKey, TItem>::PartialFlush(int WndInMsec) {
    TBlobPt BlobPt;
    PGixItemSet ItemSet;
    THashSet<TBlobPt> BlobToDelH;
    int Changes = 0;
    void* KeyDatP;

    TTmStopWatch sw(true);

    KeyDatP = ItemSetCache.FLastKeyDat();
    while (ItemSetCache.FPrevKeyDat(KeyDatP, BlobPt, ItemSet)) {
        if (sw.GetMSecInt() > WndInMsec) break;
        if (ItemSet->IsDirty()) {
            TBlobPt NewBlobPt = StoreItemSet(BlobPt);
            if (NewBlobPt.Empty()) { // if itemset is empty, we get NULL pointer
                ItemSetCache.Del(BlobPt, false);
            }
            else {
                ItemSetCache.ChangeKey(BlobPt, NewBlobPt); // blob pointer might have changed, update cache
            }
            Changes++;
        }
    }
    return Changes;
}

template <class TKey, class TItem>
int64 TGix<TKey, TItem>::GetMemUsed() const {
    int64 res = sizeof(TCRef);
    res += sizeof(TFAccess);
    res += 2 * sizeof(int64);
    res += 3 * sizeof(int);
    res += sizeof(bool);
    res += sizeof(PBlobBs);
    res += Merger->GetMemUsed();
    res += sizeof(TGixStats);
    res += GixFNm.GetMemUsed();
    res += GixBlobFNm.GetMemUsed();
    res += KeyIdH.GetMemUsed(true);
    res += ItemSetCache.GetMemUsed();
    return res;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::RefreshMemUsed() {
    // check if we have to drop anything from the cache
    if (NewCacheSizeInc > CacheResetThreshold) {
        // only report when cache size bigger then 10GB
        const bool ReportP = CacheResetThreshold > (uint64)(TInt::Giga);
        if (ReportP) { printf("Cache clean-up [%s] ... ", TUInt64::GetMegaStr(NewCacheSizeInc).CStr()); }
        // pack all the item sets
        TBlobPt BlobPt;
        PGixItemSet ItemSet;
        void* KeyDatP = ItemSetCache.FFirstKeyDat();
        while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) {
            ItemSet->DefLocal();
        }
        // clean-up cache
        CacheFullP = ItemSetCache.RefreshMemUsed();
        NewCacheSizeInc = 0;
        if (ReportP) {
            const uint64 NewSize = ItemSetCache.GetMemUsed();
            printf("Done [%s]\n", TUInt64::GetMegaStr(NewSize).CStr());
        }
    }
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::AddToNewCacheSizeInc(const uint64& OldSize, const uint64& NewSize) const {
    // no change
    if (NewSize == OldSize) {
        return;
    }
    // increased usage
    if (NewSize > OldSize) {
        NewCacheSizeInc += NewSize - OldSize;
    }
    // decreased usage
    else {
        if (NewCacheSizeInc >= OldSize - NewSize) {
            NewCacheSizeInc -= OldSize - NewSize;
        }
        // make sure we don't make an overflow
        else {
            NewCacheSizeInc = 0;
        }
    }
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const {
    TFOut FOut(FNm);
    // iterate over all the keys
    printf("Starting Gix SaveTxt\n");
    int KeyId = FFirstKeyId();
    int KeyN = 0; const int Keys = GetKeys();
    while (FNextKeyId(KeyId)) {
        if (KeyN % 1000 == 0) { printf("%d / %d\r", KeyN, Keys); } KeyN++;
        // get key and associated item set
        const TKey& Key = GetKey(KeyId);
        PGixItemSet ItemSet = GetItemSet(Key);
        // get statistics
        TStr KeyNm = KeyStr->GetKeyNm(Key);
        const int Items = ItemSet->GetItems();
        const uint64 MemUsed = ItemSet->GetMemUsed();
        // output statistics
        FOut.PutStrFmtLn("%s\t%d\t%d", KeyNm.CStr(), Items, MemUsed);
    }
    printf("Done: %d / %d\n", Keys, Keys);
}


/// print simple statistics for cache
template <class TKey, class TItem>
void TGix<TKey, TItem>::PrintStats() {
    RefreshStats();
    printf(".... gix cache stats - all=%d dirty=%d, loaded_perc=%f dirty_loaded_perc=%f, avg_len=%f, mem_used=%d \n",
        Stats.CacheAll, Stats.CacheDirty, Stats.CacheAllLoadedPerc, Stats.CacheDirtyLoadedPerc,
        Stats.AvgLen, Stats.MemUsed);
    const TBlobBsStats& blob_stats = ItemSetBlobBs->GetStats();
    printf(".... gix blob stats - puts=%u puts_new=%u gets=%u dels=%u size_chngs=%u avg_len_get=%f avg_len_put=%f avg_len_put_new=%f\n",
        blob_stats.Puts, blob_stats.PutsNew, blob_stats.Gets,
        blob_stats.Dels, blob_stats.SizeChngs, blob_stats.AvgGetLen, blob_stats.AvgPutLen, blob_stats.AvgPutNewLen);
    ItemSetBlobBs->ResetStats();
    printf(".... hash-table stats - memory=%s size=%d\n", TUInt64::GetKiloStr(KeyIdH.GetMemUsed()).CStr(), KeyIdH.Len());
    printf(".... gix - cnt=%s, memory=%s, hash=%s, cache=%s\n",
        TUInt64::GetMegaStr(KeyIdH.Len()).CStr(),
        TUInt64::GetMegaStr(GetMemUsed()).CStr(), TUInt64::GetMegaStr(KeyIdH.GetMemUsed()).CStr(), TUInt64::GetMegaStr(ItemSetCache.GetMemUsed()).CStr());
}

/////////////////////////////////////////////////
// General-Inverted-Index Expression-Item
template <class TKey, class TItem>
void TGixExpItem<TKey, TItem>::PutAnd(const TPt<TGixExpItem<TKey, TItem> >& _LeftExpItem,
    const TPt<TGixExpItem<TKey, TItem> >& _RightExpItem) {

    ExpType = getAnd;
    LeftExpItem = _LeftExpItem;
    RightExpItem = _RightExpItem;
}

template <class TKey, class TItem>
void TGixExpItem<TKey, TItem>::PutOr(const TPt<TGixExpItem<TKey, TItem> >& _LeftExpItem,
    const TPt<TGixExpItem<TKey, TItem> >& _RightExpItem) {

    ExpType = getOr;
    LeftExpItem = _LeftExpItem;
    RightExpItem = _RightExpItem;
}

template <class TKey, class TItem>
TPt<TGixExpItem<TKey, TItem> > TGixExpItem<TKey, TItem>::NewAndV(
    const TVec<TPt<TGixExpItem<TKey, TItem> > >& ExpItemV) {

    // return empty item if no key is given
    if (ExpItemV.Empty()) { return TGixExpItem<TKey, TItem>::NewEmpty(); }
    // otherwise we start with the first key
    TPt<TGixExpItem<TKey, TItem> > TopExpItem = ExpItemV[0];
    // prepare a queue, which points to the next item (left) to be expanded to tree (and left right)
    TQQueue<TPt<TGixExpItem<TKey, TItem> > > NextExpItemQ;
    // we start with the top
    NextExpItemQ.Push(TopExpItem);
    // add the rest of the items to the expresion tree
    for (int ExpItemN = 1; ExpItemN < ExpItemV.Len(); ExpItemN++) {
        const TPt<TGixExpItem<TKey, TItem> >& RightExpItem = ExpItemV[ExpItemN];
        // which item should we expand
        TPt<TGixExpItem<TKey, TItem> > ExpItem = NextExpItemQ.Top(); NextExpItemQ.Pop();
        // clone the item to be expanded
        TPt<TGixExpItem<TKey, TItem> > LeftExpItem = ExpItem->Clone();
        // and make a new subtree
        ExpItem->PutAnd(LeftExpItem, RightExpItem);
        // update the queue
        NextExpItemQ.Push(ExpItem->LeftExpItem);
        NextExpItemQ.Push(ExpItem->RightExpItem);
    }
    return TopExpItem;
}

template <class TKey, class TItem>
TPt<TGixExpItem<TKey, TItem> > TGixExpItem<TKey, TItem>::NewOrV(
    const TVec<TPt<TGixExpItem<TKey, TItem> > >& ExpItemV) {

    // return empty item if no key is given
    if (ExpItemV.Empty()) { return TGixExpItem<TKey, TItem>::NewEmpty(); }
    // otherwise we start with the first key
    TPt<TGixExpItem<TKey, TItem> > TopExpItem = ExpItemV[0];
    // prepare a queue, which points to the next item (left) to be expanded to tree (and left right)
    TQQueue<TPt<TGixExpItem<TKey, TItem> > > NextExpItemQ;
    // we start with the top
    NextExpItemQ.Push(TopExpItem);
    // add the rest of the items to the expresion tree
    for (int ExpItemN = 1; ExpItemN < ExpItemV.Len(); ExpItemN++) {
        const TPt<TGixExpItem<TKey, TItem> >& RightExpItem = ExpItemV[ExpItemN];
        // which item should we expand
        TPt<TGixExpItem<TKey, TItem> > ExpItem = NextExpItemQ.Top(); NextExpItemQ.Pop();
        // clone the item to be expanded
        TPt<TGixExpItem<TKey, TItem> > LeftExpItem = ExpItem->Clone();
        // and make a new subtree
        ExpItem->PutOr(LeftExpItem, RightExpItem);
        // update the queue
        NextExpItemQ.Push(ExpItem->LeftExpItem);
        NextExpItemQ.Push(ExpItem->RightExpItem);
    }
    return TopExpItem;
}

template <class TKey, class TItem>
TPt<TGixExpItem<TKey, TItem> > TGixExpItem<TKey, TItem>::NewAndV(const TVec<TKey>& KeyV) {
    TVec<TPt<TGixExpItem<TKey, TItem> > > ExpItemV(KeyV.Len(), 0);
    for (int KeyN = 0; KeyN < KeyV.Len(); KeyN++) {
        ExpItemV.Add(TGixExpItem<TKey, TItem>::NewItem(KeyV[KeyN]));
    }
    return NewAndV(ExpItemV);
}

template <class TKey, class TItem>
TPt<TGixExpItem<TKey, TItem> > TGixExpItem<TKey, TItem>::NewOrV(const TVec<TKey>& KeyV) {
    TVec<TPt<TGixExpItem<TKey, TItem> > > ExpItemV(KeyV.Len(), 0);
    for (int KeyN = 0; KeyN < KeyV.Len(); KeyN++) {
        ExpItemV.Add(TGixExpItem<TKey, TItem>::NewItem(KeyV[KeyN]));
    }
    return NewOrV(ExpItemV);
}

template <class TKey, class TItem>
bool TGixExpItem<TKey, TItem>::Eval(const TPt<TGix<TKey, TItem> >& Gix,
    TVec<TItem>& ResItemV, const TGixMerger<TKey, TItem>* Merger) {

    // prepare place for result
    ResItemV.Clr();
    if (ExpType == getOr) {
        EAssert(!LeftExpItem.Empty() && !RightExpItem.Empty());
        TVec<TItem> RightItemV;
        const bool NotLeft = LeftExpItem->Eval(Gix, ResItemV, Merger);
        const bool NotRight = RightExpItem->Eval(Gix, RightItemV, Merger);
        if (NotLeft && NotRight) {
            Merger->Intrs(ResItemV, RightItemV);
        } else if (!NotLeft && !NotRight) {
            Merger->Union(ResItemV, RightItemV);
        } else {
            TVec<TItem> MinusItemV;
            if (NotLeft) {
                Merger->Minus(ResItemV, RightItemV, MinusItemV);
            } else {
                Merger->Minus(RightItemV, ResItemV, MinusItemV);
            }
            ResItemV = MinusItemV;
        }
        return (NotLeft || NotRight);
    } else if (ExpType == getAnd) {
        EAssert(!LeftExpItem.Empty() && !RightExpItem.Empty());
        TVec<TItem> RightItemV;
        const bool NotLeft = LeftExpItem->Eval(Gix, ResItemV, Merger);
        const bool NotRight = RightExpItem->Eval(Gix, RightItemV, Merger);
        if (NotLeft && NotRight) {
            Merger->Union(ResItemV, RightItemV);
        } else if (!NotLeft && !NotRight) {
            Merger->Intrs(ResItemV, RightItemV);
        } else {
            TVec<TItem> MinusItemV;
            if (NotLeft) { Merger->Minus(RightItemV, ResItemV, MinusItemV); } else { Merger->Minus(ResItemV, RightItemV, MinusItemV); }
            ResItemV = MinusItemV;
        }
        return (NotLeft && NotRight);
    } else if (ExpType == getKey) {
        PGixItemSet ItemSet = Gix->GetItemSet(Key);
        if (!ItemSet.Empty()) {
            ItemSet->Def();
            ItemSet->GetItemV(ResItemV);
            Merger->Def(ItemSet->GetKey(), ResItemV);
        }
        return false;
    } else if (ExpType == getNot) {
        return !RightExpItem->Eval(Gix, ResItemV, Merger);
    } else if (ExpType == getEmpty) {
        return false; // return nothing
    }
    return true;
}

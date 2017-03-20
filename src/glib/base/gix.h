/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef GIX_H
#define GIX_H

//#include "tm.h"
//#include <inttypes.h>

// this file depends only on base.h

/////////////////////////////////////////////////
// Forward-declarations
template <class TKey, class TItem, class TGixMerger> class TGix;

/////////////////////////////////////////////////
// General-Inverted-Index-Merger - used for evaluating queries
template <class TKey, class TItem>
class TGixExpMerger {
protected:
    TCRef CRef;
    typedef TPt<TGixExpMerger<TKey, TItem> > PGixExpMerger;

public:
    virtual ~TGixExpMerger() {}

    virtual void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    virtual void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    virtual void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, TVec<TItem>& ResV) const = 0;
    virtual void Def(const TKey& Key, TVec<TItem>& MainV) const = 0;

    friend class TPt < TGixExpMerger<TKey, TItem> > ;
};

/////////////////////////////////////////////////
// General-Inverted-Index-Default-Merger
template <class TKey, class TItem>
class TGixDefMerger : public TGixExpMerger < TKey, TItem > {
private:
    typedef TPt<TGixExpMerger<TKey, TItem> > PGixMerger;

public:

    static PGixMerger New() { return new TGixDefMerger<TKey, TItem>(); }

    inline void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Union(JoinV); }
    inline void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Intrs(JoinV); }
    inline void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, TVec<TItem>& ResV) const { MainV.Diff(JoinV, ResV); }
    void Def(const TKey& Key, TVec<TItem>& MainV) const {}

    void Merge(TVec<TItem>& ItemV, bool IsLocal = false) const { ItemV.Merge(); }
    void Delete(const TItem& Item, TVec<TItem>& MainV) const { return MainV.DelAll(Item); }
    inline bool IsLt(const TItem& Item1, const TItem& Item2) const { return Item1 < Item2; }
    inline bool IsLtE(const TItem& Item1, const TItem& Item2) const { return Item1 <= Item2; }
};

/////////////////////////////////////////////////
// General-Inverted-Index Key-To-String
template <class TKey>
class TGixKeyStr {
protected:
    TCRef CRef;
    typedef TPt<TGixKeyStr<TKey> > PGixKeyStr;

public:
    virtual ~TGixKeyStr() {}
    static PGixKeyStr New() { return new TGixKeyStr < TKey > ; }

    // by default we cannot assume much about key, so just return empty string
    virtual TStr GetKeyNm(const TKey& Key) const { return TStr(); }

    friend class TPt < TGixKeyStr<TKey> > ;
};

/////////////////////////////////////////////////
// General-Inverted-Index Item-Set
//
// Supports splitting of data into child vectors.
// Assumes child vectors are individually and globaly merged

template <class TKey, class TItem, class TGixMerger>
class TGixItemSet {
private:
    TCRef CRef;
    typedef TPt<TGixItemSet<TKey, TItem, TGixMerger> > PGixItemSet;

private:
    /// This struct contans statistics about child vector
    struct TGixItemSetChildInfo {
    public:
        TItem MinVal;
        TItem MaxVal;
        TInt Len;
        TBlobPt Pt;
        bool Loaded;
        bool Dirty;

        /// default constructor
        TGixItemSetChildInfo()
            : Len(0), Loaded(false), Dirty(false) {}

        /// constructor with values
        TGixItemSetChildInfo(const TItem& _MinVal, const TItem& _MaxVal,
            const TInt& _Len, const TBlobPt& _Pt)
            : MinVal(_MinVal), MaxVal(_MaxVal), Len(_Len), Pt(_Pt),
            Loaded(false), Dirty(false) {}

        /// constructor for serialization
        TGixItemSetChildInfo(TSIn& SIn) {
            Load(SIn);
            Loaded = false;
            Dirty = false;
        }

        /// deserialize from stream
        void Load(TSIn& SIn) {
            MinVal = TItem(SIn);
            MaxVal = TItem(SIn);
            Len.Load(SIn);
            Pt = TBlobPt(SIn);
        }
        /// serialize to stream
        void Save(TSOut& SOut) const {
            MinVal.Save(SOut);
            MaxVal.Save(SOut);
            Len.Save(SOut);
            Pt.Save(SOut);
        }
    };

private:
    /// The key of this itemset
    TKey ItemSetKey;
    /// "Working buffer" of items of this itemset - could be only part of them, others can be stored in child vectors
    TVec<TItem> ItemV;
    /// List of indeces with "deleted" items
    TVec<int> ItemVDel;
    /// Combined count - from this itemset and children
    int TotalCnt;
    // optional data about child vectors - will be populated only for frequent keys
    mutable TVec<TGixItemSetChildInfo> Children;
    // optional list of child vector contents - will be populated only for frequent keys
    mutable TVec<TVec<TItem>> ChildrenData;
    // for keeping the items unique and sorted
    TBool MergedP;
    // should this itemset be stored to disk?
    TBool Dirty;
    // pointer to merger that will merge this itemset
    const TGixMerger *Merger;
    // pointer to gix - as the storage-layer (serialization of self, loading children, notifying about changes...)
    const TGix<TKey, TItem, TGixMerger> *Gix;

    /// Load single child vector into memory if not present already
    void LoadChildVector(int i) const {
        if (!Children[i].Loaded) {
            Gix->GetChildVector(Children[i].Pt, ChildrenData[i]);
            Children[i].Loaded = true;
            Children[i].Dirty = false;
        }
    }

    /// Load all child vectors into memory and get pointers to them
    void LoadChildVectors() const {
        for (int i = 0; i < Children.Len(); i++) {
            LoadChildVector(i);
        }
    }

    /// Refresh total count
    void RecalcTotalCnt() {
        TotalCnt = ItemV.Len();
        for (int i = 0; i < Children.Len(); i++) {
            TotalCnt += Children[i].Len;
        }
    }

    /// Check if there are any dirty child vectors with size outside the tolerance
    int FirstDirtyChild() {
        for (int i = 0; i < Children.Len(); i++) {
            if (Children[i].Dirty && (Children[i].Len < Gix->GetSplitLenMin() || Children[i].Len > Gix->GetSplitLenMax())) {
                return i;
            }
        }
        return -1;
    }

    /// Get the index of the first child index from which onward the content needs to be merged
    /// There can be other children with smaller indices that are dirty, but we might not want to merge them
    int FirstChildToMerge() {
        // start checking either from 0 or 1 depending on whether we allow the first child to be
        for (int i = 0; i < Children.Len(); i++) {
            // the child at least needs to be dirty to be merged
            if (!Children[i].Dirty) {
                continue;
            }
            // if child is not out of the size boundaries it also doesn't need to be merged
            if (Children[i].Len >= Gix->GetSplitLenMin() && Children[i].Len <= Gix->GetSplitLenMax()) {
                continue;
            }
            // for the first child we might allow it to be extra short, without need for merge
            // when removing oldest items, the first vector will be becoming shorter and shorter and will be removed completely once empty
            if (i == 0 && Gix->CanFirstChildBeUnfilled() && Children[i].Len <= Gix->GetSplitLenMax()) {
                continue;
            }
            // otherwise, yes, it needs to be merged
            return i;
        }
        return -1;
    }

    /// Work buffer is merged and still full, add new children collections with the data in work buffer
    void PushWorkBufferToChildren();

    /// If work buffer contains data that belongs to child vectors then push that content to them
    void InjectWorkBufferToChildren();

    /// Data has been merged in memory and needs to be pushed to child vectors (overwrite them)
    void PushMergedDataBackToChildren(int first_child_to_merge, const TVec<TItem>& MergedItems);

    /// Process any pending "delete" commands
    void ProcessDeletes();

    /// Ask child vectors about their memory usage
    uint64 GetChildMemUsed() const {
        uint64 mem = 0;
        for (int i = 0; i < ChildrenData.Len(); i++) {
            mem += ChildrenData[i].GetMemUsed();
        }
        return mem;
    }

public:
    /// Standard constructor
    TGixItemSet(const TKey& _ItemSetKey, const TGixMerger *_Merger, const TGix<TKey, TItem, TGixMerger>* _Gix) :
        ItemSetKey(_ItemSetKey), TotalCnt(0), MergedP(true), Dirty(true), Merger(_Merger), Gix(_Gix) {}
    /// Standard factory method
    static PGixItemSet New(const TKey& ItemSetKey, const TGixMerger* Merger, const TGix<TKey, TItem, TGixMerger>* Gix) {
        return new TGixItemSet(ItemSetKey, Merger, Gix);
    }

    /// Constructor for deserialization
    TGixItemSet(TSIn& SIn, const TGixMerger* _Merger, const TGix<TKey, TItem, TGixMerger>* _Gix) :
        ItemSetKey(SIn), ItemV(SIn), Children(SIn), MergedP(true), Dirty(false), Merger(_Merger), Gix(_Gix) {
        for (int i = 0; i < Children.Len(); i++) {
            ChildrenData.Add(TVec<TItem>());
        };
        RecalcTotalCnt();
    }
    /// Standard factory method for deserialization
    static PGixItemSet Load(TSIn& SIn, const TGixMerger* Merger, const TGix<TKey, TItem, TGixMerger>* Gix) {
        return new TGixItemSet(SIn, Merger, Gix);
    }
    /// Saves this itemset to output stream
    void Save(TMOut& SOut);

    // functions used by TCache
    uint64 GetMemUsed() const {
        uint64 res = 2 * sizeof(TBool);
        res += sizeof(int);
        res += sizeof(TCRef);
        res += sizeof(TGixMerger*);
        res += sizeof(TGix<TKey, TItem, TGixMerger>*);
        res += ItemSetKey.GetMemUsed();
        res += ItemV.GetMemUsed();
        res += ItemVDel.GetMemUsed();
        res += Children.GetMemUsed();
        res += ChildrenData.GetMemUsedDeep();
        return res;

        /*return ItemSetKey.GetMemUsed() + ItemV.GetMemUsed() + ItemVDel.GetMemUsed()
            + Children.GetMemUsed() + ChildrenData.GetMemUsed() + GetChildMemUsed()
            + 2 * sizeof(TBool) + sizeof(int) + sizeof(TCRef)
            + sizeof(TGixMerger*) + sizeof(TGix<TKey, TItem, TGixMerger>*);*/
    }
    void OnDelFromCache(const TBlobPt& BlobPt, void* Gix);

    // key & items
    const TKey& GetKey() const { return ItemSetKey; }
    void AddItem(const TItem& NewItem, const bool& NotifyCacheOnlyDelta = true);
    void AddItemV(const TVec<TItem>& NewItemV);
    void OverrideItems(const TVec<TItem>& NewItemV, int From, int Len);
    /// Check if this itemset is empty
    bool Empty() const { return GetItems() == 0; }
    /// Get number of items (including child itemsets)
    int GetItems() const { return TotalCnt; }
    /// Get item at given index (including child itemsets)
    const TItem& GetItem(const int& ItemN) const;
    /// Append current item set with itmes from Src
    void AppendItemSet(const TPt<TGixItemSet>& Src);
    /// Get items into vector
    void GetItemV(TVec<TItem>& _ItemV);
    /// Delete specified item from this itemset
    void DelItem(const TItem& Item);
    /// Clear all items from this itemset
    void Clr();
    /// Pack/merge this itemset
    void Def();
    /// Pack/merge this itemset - just working buffer
    void DefLocal();

    /// Flag if itemset is merged
    bool IsMerged() const { return MergedP; }
    /// Tests if current itemset is full and subsequent item should be pushed to children
    bool IsFull() const { return (ItemV.Len() >= Gix->GetSplitLen()); }

    friend class TPt < TGixItemSet > ;
    friend class TGix < TKey, TItem, TGixMerger > ;

#ifdef XTEST
    friend class XTest;
    void Print() const;
#endif
    double LoadedPerc() {
        double res = 1;
        for (int i = 0; i < Children.Len(); i++) {
            if (Children[i].Loaded)
                res += 1;
        }
        return res / (1 + Children.Len());
    }
};

#ifdef XTEST

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::Print() const {
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
    for (int j = 0; j < Children.Len(); j++) {
        printf("   *** child %d\n", j);
        printf("   *** len %d\n", Children[j].Len);
        printf("   *** mem-len %d\n", ChildrenData[j].Len());
        for (int i = 0; i < ChildrenData[j].Len(); i++) {
            printf("      %d=%d\n", i, ChildrenData[j][i]);
        }
    }
}

#endif

template <class TKey, class TItem, class TGixMerger>
const TItem& TGixItemSet<TKey, TItem, TGixMerger>::GetItem(const int& ItemN) const {
    AssertR(ItemN >= 0 && ItemN < TotalCnt, TStr() + "Index: " + TInt::GetStr(ItemN) + ", TotalCnt: " + TInt::GetStr(TotalCnt));
    int index = ItemN;
    for (int i = 0; i < Children.Len(); i++) {
        if (index < Children[i].Len) {
            // load child vector only if needed
            LoadChildVector(i);
            return ChildrenData[i][index];
        }
        index -= Children[i].Len;
    }
    return ItemV[index];
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::Save(TMOut& SOut) {
    // make sure all is merged before saving
    Def();
    // save child vectors separately
    for (int i = 0; i < Children.Len(); i++) {
        if (Children[i].Dirty && Children[i].Loaded) {
            Children[i].Pt = Gix->StoreChildVector(Children[i].Pt, ChildrenData[i]);
            Children[i].Dirty = false;
        }
    }

    // save item key and set
    ItemSetKey.Save(SOut);
    //ItemV.SaveMemCpy(SOut);
    ItemV.Save(SOut);
    Children.Save(SOut);
    Dirty = false;
}


template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::OnDelFromCache(const TBlobPt& BlobPt, void* Gix) {
    // TODO: is IsReadOnly() test necessary? Isn't the only case when Dirty == true if we are allowed write access?
    if (!((TGix<TKey, TItem, TGixMerger>*)Gix)->IsReadOnly() && Dirty) {
        ((TGix<TKey, TItem, TGixMerger>*)Gix)->StoreItemSet(BlobPt);
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::PushWorkBufferToChildren() {
    // push work-buffer into children array
    int split_len = Gix->GetSplitLen();
    while (ItemV.Len() >= split_len) {
        // create a vector of split_len items
        TVec<TItem> tmp;
        ItemV.GetSubValV(0, split_len - 1, tmp);
        // create the child info for the vector and also push the vector to a blob
        TGixItemSetChildInfo child_info(tmp[0], tmp.Last(), split_len, Gix->EnlistChildVector(tmp));
        child_info.Loaded = false;
        child_info.Dirty = false;
        Children.Add(child_info);
        // add an empty vector to ChildrenData - the data for this vector will be loaded from the blob when necessary
        ChildrenData.Add(TVec<TItem>());
        ItemV.Del(0, split_len - 1);
        Dirty = true;
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::InjectWorkBufferToChildren() {
    AssertR(ItemV.IsSorted(), "Items in working buffer ItemV should be sorted");
    if (Children.Len() > 0 && ItemV.Len() > 0) {
        // find the first Children index into which we need to insert the first value
        // since items in ItemV will most likely have the highest values, it makes sense to go from end backwards
        int j = Children.Len()-1;
        const TItem& firstVal = ItemV[0];
        while (j > 0 && Merger->IsLt(firstVal, Children[j].MinVal)) {
            j--;
        }
        // go from j onward, inserting items from ItemV into Children
        int i = 0;
        TIntSet TouchedVectorH;
        while (i < ItemV.Len()) {
            const TItem& val = ItemV[i];
            while (j < Children.Len() && Merger->IsLt(Children[j].MaxVal, val)) {
                j++;
            }
            // if val is larger than MaxVal in last Children vector, then all remaining values in input buffer will not be inserted into child vectors
            if (j >= Children.Len()) {
                break;
            }
            // ok, insert into j-th child
            LoadChildVector(j);
            ChildrenData[j].Add(val);
            Children[j].Len = ChildrenData[j].Len();
            Children[j].Dirty = true;
            TouchedVectorH.AddKey(j);
            i++;
        }

        // delete items from work-buffer that have been inserted into child vectors
        if (i > 0) {
            // we made at least one insertion into the children, mark itemset as dirty
            Dirty = true;
            if (i == ItemV.Len()) {
                // we inserted all items into children - clear the working buffer
                ItemV.Clr();
            } else {
                // clear only the items that were already inserted into children
                ItemV.Del(0, i - 1);
            }
        }

        // go over all the vectors that we modified and merge + update stats for them
        for (int KeyId = TouchedVectorH.FFirstKeyId(); TouchedVectorH.FNextKeyId(KeyId); ) {
            int ind = TouchedVectorH.GetKey(KeyId);
            LoadChildVector(ind); // just in case - they should be in memory at this point anyway
            TVec<TItem>& cd = ChildrenData[ind];
            Merger->Merge(cd, false);
            Children[ind].Len = cd.Len();
            Children[ind].Dirty = true;
            if (cd.Len() > 0) {
                Children[ind].MinVal = cd[0];
                Children[ind].MaxVal = cd.Last();
            }
        }
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::PushMergedDataBackToChildren(int first_child_to_merge, const TVec<TItem>& MergedItems) {
    int curr_index = 0;
    int remaining = MergedItems.Len() - curr_index;
    int child_index = first_child_to_merge;
    while (curr_index < MergedItems.Len()) {
        if (child_index < Children.Len() && remaining > Gix->GetSplitLen()) {
            ChildrenData[child_index].Clr();
            //MergedItems.GetSubValVMemCpy(curr_index, curr_index + Gix->GetSplitLen() - 1, ChildrenData[child_index]);
            MergedItems.GetSubValV(curr_index, curr_index + Gix->GetSplitLen() - 1, ChildrenData[child_index]);
            Children[child_index].Len = ChildrenData[child_index].Len();
            Children[child_index].MinVal = ChildrenData[child_index][0];
            Children[child_index].MaxVal = ChildrenData[child_index].Last();
            Children[child_index].Dirty = true;
            Children[child_index].Loaded = true;
            curr_index += Children[child_index].Len;
            remaining = MergedItems.Len() - curr_index;
            child_index++;
        } else {
            // put the remaining data into work-buffer
            ItemV.Clr();
            //MergedItems.GetSubValVMemCpy(curr_index, curr_index + remaining - 1, ItemV);
            MergedItems.GetSubValV(curr_index, curr_index + remaining - 1, ItemV);
            break;
        }
    }

    // remove children that became empty
    // remove them first from BLOB storage
    for (int ind = child_index; ind < Children.Len(); ind++) {
        Gix->DeleteChildVector(Children[ind].Pt);
    }

    // finally remove them from memory
    if (child_index < Children.Len()) {
        Children.Del(child_index, Children.Len() - 1);
        ChildrenData.Del(child_index, ChildrenData.Len() - 1);
    }
    Dirty = true;
}


template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::AddItem(const TItem& NewItem, const bool& NotifyCacheOnlyDelta) {
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
        if (ItemV.Len() == 0 && Children.Len() == 0) {
            MergedP = true; // the first item in whole itemset
        } else if (ItemV.Len() == 0 && Children.Len() != 0) {
            MergedP = Merger->IsLt(Children.Last().MaxVal, NewItem); // compare with the last item of the last child
        } else {
            MergedP = Merger->IsLt(ItemV.Last(), NewItem); // compare to the last item in the work buffer
        }
    }
    const uint64 OldItemVSize = ItemV.GetMemUsed();
    if (ItemV.Len() == 0) {
        ItemV.Reserve(2);
    }
    ItemV.Add(NewItem);
    // update the cache size (for the newly added item)
    // in general we could just add sizeof(TItem) to cache size - however we would underestimate the used size since the arrays allocate extra buffer
    Gix->AddToNewCacheSizeInc(OldItemVSize, ItemV.GetMemUsed());

    Dirty = true;
    TotalCnt++;
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::AddItemV(const TVec<TItem>& NewItemV) {
    for (int i = 0; i < NewItemV.Len(); i++) {
        AddItem(NewItemV[i]);
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::AppendItemSet(const TPt<TGixItemSet>& Src) {
    Src->LoadChildVectors();
    // first, process children if present
    if (Src->Children.Len() > 0) {
        // merge each of them
        for (int i = 0; i < ChildrenData.Len(); i++) {
            AddItemV(Src->ChildrenData[i]);
        }
    }
    // now get work buffer
    AddItemV(Src->ItemV);
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::GetItemV(TVec<TItem>& _ItemV) {
    if (Children.Len() > 0) {
        // collect data from child itemsets
        LoadChildVectors();
        for (int i = 0; i < Children.Len(); i++) {
            //_ItemV.AddVMemCpy(ChildrenData[i]);
            _ItemV.AddV(ChildrenData[i]);
        }
    }
    //_ItemV.AddVMemCpy(ItemV);
    _ItemV.AddV(ItemV);
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::DelItem(const TItem& Item) {
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
    Dirty = true;
    TotalCnt++;
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::Clr() {
    const int OldSize = GetMemUsed();
    if (Children.Len() > 0) {
        for (int i = 0; i < Children.Len(); i++) {
            Gix->DeleteChildVector(Children[i].Pt);
        }
        ChildrenData.Clr();
        Children.Clr();
    }
    ItemV.Clr();
    ItemVDel.Clr();
    MergedP = true;
    Dirty = true;
    TotalCnt = 0;
    Gix->AddToNewCacheSizeInc(OldSize, GetMemUsed());
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::ProcessDeletes() {
    if (ItemVDel.Len() > 0) {
        TVec<TItem> ItemVNew;
        int ItemVNewI = 0;

        // go over all indices in ItemVDel that represent which items in ItemV are keys to delete
        for (int i = 0; i < ItemVDel.Len(); i++) {
            // get the value to delete
            const TItem& val = ItemV[ItemVDel[i]];
            // find the children vector from which we need to delete the value
            // since deletes are often called on the oldest items we immediately test if val is in the first vector - if not, go from last vector backward
            int j = (Children.Len() > 0 && Merger->IsLtE(val, Children[0].MaxVal)) ? 0 : Children.Len() - 1;
            while (j >= 0 && Merger->IsLtE(val, Children[j].MaxVal)) {
                if (Merger->IsLtE(Children[j].MinVal, val)) {
                    LoadChildVector(j);
                    Merger->Delete(val, ChildrenData[j]);
                    Children[j].Len = ChildrenData[j].Len();
                    Children[j].Dirty = true;
                    // since we have already found and deleted the item, we can stop iterating over children vectors
                    break;
                    // we don't update stats (min & max), because they are still usable.
                }
                j--;
            }
            // copy from ItemV to ItemVNew items up to index ItemVDel[i]
            while (ItemVNewI <= ItemVDel[i]) {
                ItemVNew.Add(ItemV[ItemVNewI++]);
            }
            // it is possible that value val appears multiple times in ItemVNew so we have to delete all it's instances
            Merger->Delete(val, ItemVNew);
        }
        // copy the remaining items from the last item to delete to the end of the vector ItemV and copy items to ItemVNew
        while (ItemVNewI < ItemV.Len()) {
            ItemVNew.Add(ItemV[ItemVNewI++]);
        }

        ItemV.Clr();
        ItemVDel.Clr();
        //ItemV.AddVMemCpy(ItemVNew);
        ItemV.AddV(ItemVNew);
        Dirty = true;
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::Def() {
    // call merger to pack items, if not merged yet
    if (!MergedP) {
        ProcessDeletes(); // "execute" deletes, possibly leaving some child vectors too short
        Merger->Merge(ItemV, true); // first do local merge of work-buffer
        Dirty = true;
        InjectWorkBufferToChildren(); // inject data into child vectors

        int firstChildToMerge = FirstChildToMerge();
        if (firstChildToMerge >= 0 || (Children.Len() > 0 && ItemV.Len() > 0)) {
            if (firstChildToMerge < 0) {
                firstChildToMerge = Children.Len();
            }

            // collect all data from subsequent child vectors and work-buffer
            TVec<TItem> MergedItems;
            for (int i = firstChildToMerge; i < Children.Len(); i++) {
                LoadChildVector(i);
                MergedItems.AddV(ChildrenData[i]);
            }
            MergedItems.AddV(ItemV);
            Merger->Merge(MergedItems, false); // perform global merge

            PushMergedDataBackToChildren(firstChildToMerge, MergedItems); // now save them back
            PushWorkBufferToChildren(); // it could happen that data in work buffer is still too large
        }

        // in case deletes emptied the first children completely, remove them
        while (Children.Len() > 0 && Children[0].Len == 0) {
            // remove them first from BLOB storage
            Gix->DeleteChildVector(Children[0].Pt);
            // remove it from memory
            Children.Del(0);
            ChildrenData.Del(0);
        }

        RecalcTotalCnt();
        MergedP = true;
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::DefLocal() {
    // call merger to pack items in work buffer, if not merged yet
    if (!MergedP) {
        if (ItemVDel.Len() == 0) { // deletes are not treated as local - merger would get confused
            const int OldItemVLen = ItemV.Len();
            Merger->Merge(ItemV, true); // perform local merge
            Dirty = true;
            if (Children.Len() > 0 && ItemV.Len() > 0) {
                if (Merger->IsLt(Children.Last().MaxVal, ItemV[0])) {
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

//////////////////////////////////////////////////
// Basic statistics for TGix
struct TGixStats {
public:
    /// Number of itemsets in cache
    int CacheAll;
    /// Number of dirty itemsets in cache
    int CacheDirty;
    /// Percentage of loaded content for itemsets in cache
    double CacheAllLoadedPerc;
    /// Percentage of loaded content for dirty itemsets in cache
    double CacheDirtyLoadedPerc;
    /// Average length of itemsets in cache
    double AvgLen;
    /// memory usage for gix
    int64 MemUsed;

    /// This method combines statistics from to Gix objects
    static TGixStats Add(const TGixStats& Stat1, const TGixStats& Stat2) {
        TGixStats res;
        res.CacheAll = Stat1.CacheAll + Stat2.CacheAll;
        res.CacheDirty = Stat1.CacheDirty + Stat2.CacheDirty;
        res.MemUsed = Stat1.MemUsed + Stat2.MemUsed;
        res.AvgLen = res.CacheAllLoadedPerc = res.CacheDirtyLoadedPerc = 0;
        if (res.CacheAll > 0) {
            res.CacheAllLoadedPerc = (Stat1.CacheAll*Stat1.CacheAllLoadedPerc + Stat2.CacheAll * Stat2.CacheAllLoadedPerc) / res.CacheAll;
            res.AvgLen = (Stat1.CacheAll*Stat1.AvgLen + Stat2.CacheAll * Stat2.AvgLen) / res.CacheAll;
        }
        if (res.CacheDirty > 0) {
            res.CacheDirtyLoadedPerc = (Stat1.CacheDirty*Stat1.CacheDirtyLoadedPerc + Stat2.CacheDirty * Stat2.CacheDirtyLoadedPerc) / res.CacheDirty;
        }
        return res;
    }
};

/////////////////////////////////////////////////
// General-Inverted-Index
template <class TKey, class TItem, class TGixMerger /*= TGixDefMerger<TKey, TItem>*/>
class TGix {
private:
    TCRef CRef;
    typedef TPt<TGix<TKey, TItem, TGixMerger> > PGix;
    typedef TPt<TGixItemSet<TKey, TItem, TGixMerger> > PGixItemSet;
    //typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;
    typedef TGixDefMerger<TKey, TItem> _TGixDefMerger;
    typedef TPt<TGixKeyStr<TKey> > PGixKeyStr;
private:
    /// File access mode - checked during index operations
    TFAccess Access;
    /// Name of the main file
    TStr GixFNm;
    /// Name of the BLOB file
    TStr GixBlobFNm;
    /// mapping between key and BLOB pointer
    THash<TKey, TBlobPt> KeyIdH;

    /// record merger, used for packing data vectors
    //PGixMerger Merger;
    TGixMerger Merger;

    /// cache for BLOB data, mapping between pointers and data
    mutable TCache<TBlobPt, PGixItemSet> ItemSetCache;
    /// BLOB handler
    PBlobBs ItemSetBlobBs;

    uint64 CacheResetThreshold;
    mutable uint64 NewCacheSizeInc;
    /// flag if cache is full
    bool CacheFullP;

    // returns pointer to this object (used in cache call-backs)
    void* GetVoidThis() const { return (void*)this; }

    /// asserts if we are allowed to change this index
    void AssertReadOnly() const {
        EAssertR(((Access == faCreate) || (Access == faUpdate)),
            "Index opened in Read-Only mode!");
    }

    /// get keyid of a given key and create it if does not exist
    TBlobPt AddKeyId(const TKey& Key);
    /// get keyid of a given key
    TBlobPt GetKeyId(const TKey& Key) const;

    /// get child vector for given blob pointer
    void GetChildVector(const TBlobPt& Pt, TVec<TItem>& Dest) const;
    /// for storing child vectors to blob
    TBlobPt StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) const;
    /// for deleting child vectors from cache and blob
    void DeleteChildVector(const TBlobPt& KeyId) const;
    /// For enlisting new child vectors into blob
    TBlobPt EnlistChildVector(const TVec<TItem>& Data) const;
    /// Size of work-buffer
    int SplitLen;
    /// Minimal length for child vectors
    int SplitLenMin;
    /// Maximal length for child vectors
    int SplitLenMax;
    /// Can the first child vector be of any non-empty size and not be merged with following vectors
    /// This can significantly speed-up deleting items from Gix
    bool CanFirstChildBeUnfilled_;

    /// Internal member for holding statistics
    mutable TGixStats Stats;
    /// This method refreshes gix statistics
    void RefreshStats() const;

public:
    TGix(const TStr& Nm, const TStr& FPath,
        const TFAccess& _Access, const int64& CacheSize,
        const int _SplitLen, const bool _CanFirstChildBeUnfilled,
        const int _SplitLenMin, const int _SplitLenMax);
    static PGix New(const TStr& Nm, const TStr& FPath = TStr(),
        const TFAccess& Access = faRdOnly, const int64& CacheSize = 100000000,
        const int SplitLen = 1024, const bool CanFirstChildBeUnfilled = true,
        const int SplitLenMin = 512, const int SplitLenMax = 2048) {
        return new TGix(Nm, FPath, Access, CacheSize, SplitLen,
            CanFirstChildBeUnfilled, SplitLenMin, SplitLenMax);
    }

    ~TGix();

    // Gix properties
    bool IsReadOnly() const { return Access == faRdOnly; }
    bool IsCacheFullP() const { return CacheFullP; }
    TStr GetFPath() const { return GixFNm.GetFPath(); }
    int64 GetMxCacheSize() const { return GetMxMemUsed(); }
    int GetSplitLen() const { return SplitLen; }
    int GetSplitLenMax() const { return SplitLenMax; }
    int GetSplitLenMin() const { return SplitLenMin; }
    bool CanFirstChildBeUnfilled() const { return CanFirstChildBeUnfilled_; }

    /// do we have Key in the index?
    bool IsKey(const TKey& Key) const { return KeyIdH.IsKey(Key); }
    /// number of keys in the index
    int GetKeys() const { return KeyIdH.Len(); }
    /// sort keys
    void SortKeys() { KeyIdH.SortByKey(true); }

    /// get item set for given key
    PGixItemSet GetItemSet(const TKey& Key) const;
    /// get item set for given BLOB pointer
    PGixItemSet GetItemSet(const TBlobPt& Pt) const;
    /// for storing item sets from cache to blob
    TBlobPt StoreItemSet(const TBlobPt& KeyId);
    ///// for deleting item sets from cache and blob
    //void DeleteItemSet(const TBlobPt& KeyId) const;
    /// for deleting itemset from cache and blob
    void DeleteItemSet(const TKey& Key);
    /// For enlisting new itemsets into blob
    TBlobPt EnlistItemSet(const PGixItemSet& ItemSet) const;

    /// adding new item to the inverted index
    void AddItem(const TKey& Key, const TItem& Item);
    /// adding new items to the inverted index
    void AddItemV(const TKey& Key, const TVec<TItem>& ItemV);
    // delete one item
    void DelItem(const TKey& Key, const TItem& Item);
    /// clears items
    void Clr(const TKey& Key);
    /// flush all data from cache to disk
    void Flush() { ItemSetCache.FlushAndClr(); }
    /// flush a portion of data from cache to disk
    int PartialFlush(int WndInMsec = 500);


    // traversing keys
    /// get first key id
    int FFirstKeyId() const { return KeyIdH.FFirstKeyId(); }
    /// get next key id
    bool FNextKeyId(int& KeyId) const { return KeyIdH.FNextKeyId(KeyId); }
    /// get key for given key id
    const TKey& GetKey(const int& KeyId) const { return KeyIdH.GetKey(KeyId); }

    // merges another index with same key and item vocabulary
    void MergeIndex(const PGix& TmpGix);

    /// get amount of memory currently used
    int64 GetMemUsed() const {
        int64 res = sizeof(TCRef);
        res += sizeof(TFAccess);
        res += 2 * sizeof(int64);
        res += 3 * sizeof(int);
        res += sizeof(bool);
        res += sizeof(PBlobBs);
        res += sizeof(TGixMerger);
        res += sizeof(TGixStats);
        res += GixFNm.GetMemUsed();
        res += GixBlobFNm.GetMemUsed();
        res += KeyIdH.GetMemUsed();
        res += ItemSetCache.GetMemUsed();
        return res;
        /*return
            int64(sizeof(TCRef) + sizeof(TFAccess) + GixFNm.GetMemUsed() + GixBlobFNm.GetMemUsed()) +
            int64(KeyIdH.GetMemUsed()) + sizeof(TGixMerger) + int64(ItemSetCache.GetMemUsed() + sizeof(PBlobBs) +
            2*sizeof(int64) + sizeof(bool) + sizeof(TGixStats));*/
    }
    int GetNewCacheSizeInc() const { return NewCacheSizeInc; }
    uint64 GetCacheSize() const { return ItemSetCache.GetMemUsed(); }
    uint64 GetMxMemUsed() const { return ItemSetCache.GetMxMemUsed(); }
    bool IsCacheFull() const { return CacheFullP; }
    void RefreshMemUsed();
    void AddToNewCacheSizeInc(const uint64& Diff) const { NewCacheSizeInc += Diff; }
    void AddToNewCacheSizeInc(const uint64& OldSize, const uint64& NewSize) const;


    /// print statistics for index keys
    void SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const;
    /// print simple statistics for cache
    void PrintStats();
    /// get blob stats
    const TBlobBsStats& GetBlobStats() { return ItemSetBlobBs->GetStats(); }
    /// get gix stats
    const TGixStats& GetGixStats(const bool& RefreshP = true) const {
        if (RefreshP) { RefreshStats(); } return Stats; }
    /// reset blob stats
    void ResetStats() { ItemSetBlobBs->ResetStats(); }

    friend class TPt < TGix > ;
    friend class TGixItemSet < TKey, TItem, TGixMerger > ;
#ifdef XTEST
    friend class XTest;

    void KillHash() { this->KeyIdH.Clr(); }
    void KillCache() { this->ItemSetCache.FlushAndClr(); }
#endif
};



template <class TKey, class TItem, class TGixMerger>
int TGix<TKey, TItem, TGixMerger>::PartialFlush(int WndInMsec) {
    TBlobPt BlobPt;
    PGixItemSet ItemSet;
    THashSet<TBlobPt> BlobToDelH;
    int Changes = 0;
    void* KeyDatP;

    TTmStopWatch sw(true);

    KeyDatP = ItemSetCache.FLastKeyDat();
    while (ItemSetCache.FPrevKeyDat(KeyDatP, BlobPt, ItemSet)) {
        if (sw.GetMSecInt() > WndInMsec) break;
        if (ItemSet->Dirty) {
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



template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::AddKeyId(const TKey& Key) {
    if (IsKey(Key)) { return KeyIdH.GetDat(Key); }
    // we don't have this key, create an empty item set and return pointer to it
    AssertReadOnly(); // check if we are allowed to write
    PGixItemSet ItemSet = TGixItemSet<TKey, TItem, TGixMerger>::New(Key, &Merger, this);
    TBlobPt KeyId = EnlistItemSet(ItemSet);
    KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
    return KeyId;
}

template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::GetKeyId(const TKey& Key) const {
    if (IsKey(Key)) { return KeyIdH.GetDat(Key); }
    // we don't have this key, return empty pointer
    return TBlobPt();
}

template <class TKey, class TItem, class TGixMerger>
TGix<TKey, TItem, TGixMerger>::TGix(const TStr& Nm, const TStr& FPath, const TFAccess& _Access,
    const int64& CacheSize, const int _SplitLen, const bool _CanFirstChildBeUnfilled,
    const int _SplitLenMin, const int _SplitLenMax) :
        Access(_Access),
        ItemSetCache(CacheSize, 1000000, GetVoidThis()),
        SplitLen(_SplitLen),
        SplitLenMin(_SplitLenMin),
        SplitLenMax(_SplitLenMax),
        CanFirstChildBeUnfilled_(_CanFirstChildBeUnfilled) {
    // filenames of the GIX datastore
    GixFNm = TStr::GetNrFPath(FPath) + Nm.GetFBase() + ".Gix";
    GixBlobFNm = TStr::GetNrFPath(FPath) + Nm.GetFBase() + ".GixDat";

    if (Access == faCreate) {
        // creating a new Gix
        ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, faCreate);
    } else {
        // loading an old Gix and getting it ready for search and update
        EAssert((Access == faUpdate) || (Access == faRdOnly) || (Access == faRestore));
        // load Gix from GixFNm
        TFIn FIn(GixFNm);
        KeyIdH.Load(FIn);
        // load ItemSets from GixBlobFNm
        ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, Access);
    }

    CacheResetThreshold = int64(0.1 * double(CacheSize));
    NewCacheSizeInc = 0;
    CacheFullP = false;
}

template <class TKey, class TItem, class TGixMerger>
TGix<TKey, TItem, TGixMerger>::~TGix() {
    if ((Access == faCreate) || (Access == faUpdate)) {
        //this->PrintStats();
        // flush all the latest changes in cache to the disk
        ItemSetCache.Flush();
        // save the rest to GixFNm
        TFOut FOut(GixFNm);
        KeyIdH.Save(FOut);
    }
}

template <class TKey, class TItem, class TGixMerger>
TPt<TGixItemSet<TKey, TItem, TGixMerger> > TGix<TKey, TItem, TGixMerger>::GetItemSet(const TKey& Key) const {
    TBlobPt KeyId = GetKeyId(Key);
    return GetItemSet(KeyId);
}
template <class TKey, class TItem, class TGixMerger>
TPt<TGixItemSet<TKey, TItem, TGixMerger> > TGix<TKey, TItem, TGixMerger>::GetItemSet(const TBlobPt& KeyId) const {
    if (KeyId.Empty()) {
        // return empty itemset
        return TGixItemSet<TKey, TItem, TGixMerger>::New(TKey(), &Merger, this);
    }
    PGixItemSet ItemSet;
    if (!ItemSetCache.Get(KeyId, ItemSet)) {
        // have to load it from the hard drive...
        PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
        ItemSet = TGixItemSet<TKey, TItem, TGixMerger>::Load(*ItemSetSIn, &Merger, this);
    }
    // bring the itemset to the top of the cache
    ItemSetCache.Put(KeyId, ItemSet);
    return ItemSet;
}

template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::StoreItemSet(const TBlobPt& KeyId) {
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
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::DeleteItemSet(const TKey& Key) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        TBlobPt Pt = KeyIdH.GetDat(Key);
        ItemSetCache.Del(Pt, false);
        ItemSetBlobBs->DelBlob(Pt);
        KeyIdH.DelKey(Key);
    }
}

template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::EnlistItemSet(const PGixItemSet& ItemSet) const {
    AssertReadOnly(); // check if we are allowed to write
    TMOut MOut;
    ItemSet->Save(MOut);
    TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
    return res;
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::AddItem(const TKey& Key, const TItem& Item) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        // get the key handle
        TBlobPt KeyId = KeyIdH.GetDat(Key);
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        ItemSet->AddItem(Item);
    } else {
        // we don't have this key, create a new itemset and add new item immidiatelly
        PGixItemSet ItemSet = TGixItemSet<TKey, TItem, TGixMerger>::New(Key, &Merger, this);
        ItemSet->AddItem(Item, false);
        TBlobPt KeyId = EnlistItemSet(ItemSet); // now store this itemset to disk
        KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
        ItemSetCache.Put(KeyId, ItemSet); // add it to cache
    }
    // check if we have to drop anything from the cache
    RefreshMemUsed();
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::AddItemV(const TKey& Key, const TVec<TItem>& ItemV) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) {
        // get the key handle
        TBlobPt KeyId = KeyIdH.GetDat(Key);
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        ItemSet->AddItemV(ItemV);
    } else {
        // we don't have this key, create a new itemset and add new item immidiatelly
        PGixItemSet ItemSet = TGixItemSet<TKey, TItem, TGixMerger>::New(Key, &Merger, this);
        ItemSet->AddItemV(ItemV);
        TBlobPt KeyId = EnlistItemSet(ItemSet); // now store this itemset to disk
        KeyIdH.AddDat(Key, KeyId); // remember the new key and its Id
    }
    // check if we have to drop anything from the cache
    RefreshMemUsed();
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::DelItem(const TKey& Key, const TItem& Item) {
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

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::Clr(const TKey& Key) {
    AssertReadOnly(); // check if we are allowed to write
    if (IsKey(Key)) { // check if this key exists
        // load the current item set
        PGixItemSet ItemSet = GetItemSet(Key);
        // clear the items from the ItemSet
        ItemSet->Clr();
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::MergeIndex(const TPt<TGix<TKey, TItem, TGixMerger> >& TmpGix) {
    // merge itemsets
    const int TmpKeys = TmpGix->GetKeys();
    int TmpKeyId = TmpGix->FFirstKeyId();
    while (TmpGix->FNextKeyId(TmpKeyId)) {
        const TKey& TmpKey = TmpGix->GetKey(TmpKeyId);
        PGixItemSet TmpItemSet = TmpGix->GetItemSet(TmpKey);
        PGixItemSet MyItemSet = GetItemSet(TmpKey);
        MyItemSet->AppendItemSet(TmpItemSet);
        //AddItemV(ItemSet->GetKey(), ItemSet->GetItemV());
        if (TmpKeyId % 1000 == 0) {
            printf("[%d/%d]\r", TmpKeyId, TmpKeys);
        }
    }
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const {
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

/// refreshes statistics for cache
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::RefreshStats() const {
    Stats.CacheAll = 0;
    Stats.CacheDirty = 0;
    Stats.CacheAllLoadedPerc = 0;
    Stats.CacheDirtyLoadedPerc = 0;
    Stats.AvgLen = 0;

    Stats.MemUsed = this->GetMemUsed();
    TBlobPt BlobPt;
    PGixItemSet ItemSet;
    void* KeyDatP = ItemSetCache.FFirstKeyDat();
    while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) {
        Stats.CacheAll++;
        double d = ItemSet->LoadedPerc();
        Stats.CacheAllLoadedPerc += d;
        Stats.AvgLen += ItemSet->TotalCnt;
        if (ItemSet->Dirty) {
            Stats.CacheDirty++;
            Stats.CacheDirtyLoadedPerc += d;
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


/// print simple statistics for cache
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::PrintStats() {
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

//#endif

/// for storing vectors to blob
template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) const {
    AssertReadOnly(); // check if we are allowed to write
    // store the current version to the blob
    TMOut MOut;
    //Data.SaveMemCpy(MOut);
    Data.Save(MOut);
    int ReleasedSize;
    return ItemSetBlobBs->PutBlob(ExistingKeyId, MOut.GetSIn(), ReleasedSize);
}

/// for deleting child vector from blob
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::DeleteChildVector(const TBlobPt& KeyId) const {
    AssertReadOnly(); // check if we are allowed to write
    ItemSetBlobBs->DelBlob(KeyId);    // free space in BLOB
}

/// For enlisting new child vectors into blob
template <class TKey, class TItem, class TGixMerger>
TBlobPt TGix<TKey, TItem, TGixMerger>::EnlistChildVector(const TVec<TItem>& Data) const {
    AssertReadOnly(); // check if we are allowed to write
    TMOut MOut;
    Data.Save(MOut);
    TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
    return res;
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::GetChildVector(const TBlobPt& KeyId, TVec<TItem>& Dest) const {
    if (KeyId.Empty()) { return; }
    PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
    Dest.Load(*ItemSetSIn);
}

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::RefreshMemUsed() {
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

template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::AddToNewCacheSizeInc(const uint64& OldSize, const uint64& NewSize) const
{
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

/////////////////////////////////////////////////
// General-Inverted-Index Expression-Item
typedef enum { getUndef, getEmpty, getOr, getAnd, getNot, getKey } TGixExpType;

template <class TKey, class TItem, class TGixMerger>
class TGixExpItem {
private:
    TCRef CRef;
    typedef TPt<TGixExpItem<TKey, TItem, TGixMerger> > PGixExpItem;
    typedef TPt<TGixItemSet<TKey, TItem, TGixMerger> > PGixItemSet;
    typedef TPt<TGix<TKey, TItem, TGixMerger> > PGix;
    //typedef TGixDefMerger<TKey, TItem> _TGixDefMerger;
private:
    TGixExpType ExpType;
    PGixExpItem LeftExpItem;
    PGixExpItem RightExpItem;
    TKey Key;

    TGixExpItem(const TGixExpType& _ExpType, const PGixExpItem& _LeftExpItem,
        const PGixExpItem& _RightExpItem) : ExpType(_ExpType),
        LeftExpItem(_LeftExpItem), RightExpItem(_RightExpItem) {}
    TGixExpItem(const TKey& _Key) : ExpType(getKey), Key(_Key) {}
    TGixExpItem() : ExpType(getEmpty) {}
    TGixExpItem(const TGixExpItem& ExpItem) : ExpType(ExpItem.ExpType),
        LeftExpItem(ExpItem.LeftExpItem), RightExpItem(ExpItem.RightExpItem),
        Key(ExpItem.Key) {}

    void PutAnd(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);
    void PutOr(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);
public:
    // elementary operations
    static PGixExpItem NewOr(const PGixExpItem& LeftExpItem, const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getOr, LeftExpItem, RightExpItem);
    }
    static PGixExpItem NewAnd(const PGixExpItem& LeftExpItem, const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getAnd, LeftExpItem, RightExpItem);
    }
    static PGixExpItem NewNot(const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getNot, NULL, RightExpItem);
    }
    static PGixExpItem NewItem(const TKey& Key) {
        return new TGixExpItem(Key);
    }
    static PGixExpItem NewEmpty() {
        return new TGixExpItem();
    }

    // some predifined structures
    static PGixExpItem NewAndV(const TVec<PGixExpItem>& ExpItemV);
    static PGixExpItem NewOrV(const TVec<PGixExpItem>& ExpItemV);
    static PGixExpItem NewAndV(const TVec<TKey>& KeyV);
    static PGixExpItem NewOrV(const TVec<TKey>& KeyV);

    bool IsEmpty() const { return (ExpType == getEmpty); }
    TGixExpType GetExpType() const { return ExpType; }
    TKey GetKey() const { return Key; }
    PGixExpItem Clone() const { return new TGixExpItem(*this); }
    bool Eval(const PGix& Gix, TVec<TItem>& ResItemV, const TPt<TGixExpMerger<TKey, TItem>>& Merger /*= _TGixDefMerger::New() */);

    friend class TPt < TGixExpItem > ;
};

template <class TKey, class TItem, class TGixMerger>
void TGixExpItem<TKey, TItem, TGixMerger>::PutAnd(const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& _LeftExpItem,
    const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& _RightExpItem) {

    ExpType = getAnd;
    LeftExpItem = _LeftExpItem;
    RightExpItem = _RightExpItem;
}

template <class TKey, class TItem, class TGixMerger>
void TGixExpItem<TKey, TItem, TGixMerger>::PutOr(const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& _LeftExpItem,
    const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& _RightExpItem) {

    ExpType = getOr;
    LeftExpItem = _LeftExpItem;
    RightExpItem = _RightExpItem;
}

template <class TKey, class TItem, class TGixMerger>
TPt<TGixExpItem<TKey, TItem, TGixMerger> > TGixExpItem<TKey, TItem, TGixMerger>::NewAndV(
    const TVec<TPt<TGixExpItem<TKey, TItem, TGixMerger> > >& ExpItemV) {

    // return empty item if no key is given
    if (ExpItemV.Empty()) { return TGixExpItem<TKey, TItem, TGixMerger>::NewEmpty(); }
    // otherwise we start with the first key
    TPt<TGixExpItem<TKey, TItem, TGixMerger> > TopExpItem = ExpItemV[0];
    // prepare a queue, which points to the next item (left) to be expanded to tree (and left right)
    TQQueue<TPt<TGixExpItem<TKey, TItem, TGixMerger> > > NextExpItemQ;
    // we start with the top
    NextExpItemQ.Push(TopExpItem);
    // add the rest of the items to the expresion tree
    for (int ExpItemN = 1; ExpItemN < ExpItemV.Len(); ExpItemN++) {
        const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& RightExpItem = ExpItemV[ExpItemN];
        // which item should we expand
        TPt<TGixExpItem<TKey, TItem, TGixMerger> > ExpItem = NextExpItemQ.Top(); NextExpItemQ.Pop();
        // clone the item to be expanded
        TPt<TGixExpItem<TKey, TItem, TGixMerger> > LeftExpItem = ExpItem->Clone();
        // and make a new subtree
        ExpItem->PutAnd(LeftExpItem, RightExpItem);
        // update the queue
        NextExpItemQ.Push(ExpItem->LeftExpItem);
        NextExpItemQ.Push(ExpItem->RightExpItem);
    }
    return TopExpItem;
}

template <class TKey, class TItem, class TGixMerger>
TPt<TGixExpItem<TKey, TItem, TGixMerger> > TGixExpItem<TKey, TItem, TGixMerger>::NewOrV(
    const TVec<TPt<TGixExpItem<TKey, TItem, TGixMerger> > >& ExpItemV) {

    // return empty item if no key is given
    if (ExpItemV.Empty()) { return TGixExpItem<TKey, TItem, TGixMerger>::NewEmpty(); }
    // otherwise we start with the first key
    TPt<TGixExpItem<TKey, TItem, TGixMerger> > TopExpItem = ExpItemV[0];
    // prepare a queue, which points to the next item (left) to be expanded to tree (and left right)
    TQQueue<TPt<TGixExpItem<TKey, TItem, TGixMerger> > > NextExpItemQ;
    // we start with the top
    NextExpItemQ.Push(TopExpItem);
    // add the rest of the items to the expresion tree
    for (int ExpItemN = 1; ExpItemN < ExpItemV.Len(); ExpItemN++) {
        const TPt<TGixExpItem<TKey, TItem, TGixMerger> >& RightExpItem = ExpItemV[ExpItemN];
        // which item should we expand
        TPt<TGixExpItem<TKey, TItem, TGixMerger> > ExpItem = NextExpItemQ.Top(); NextExpItemQ.Pop();
        // clone the item to be expanded
        TPt<TGixExpItem<TKey, TItem, TGixMerger> > LeftExpItem = ExpItem->Clone();
        // and make a new subtree
        ExpItem->PutOr(LeftExpItem, RightExpItem);
        // update the queue
        NextExpItemQ.Push(ExpItem->LeftExpItem);
        NextExpItemQ.Push(ExpItem->RightExpItem);
    }
    return TopExpItem;
}

template <class TKey, class TItem, class TGixMerger>
TPt<TGixExpItem<TKey, TItem, TGixMerger> > TGixExpItem<TKey, TItem, TGixMerger>::NewAndV(const TVec<TKey>& KeyV) {
    TVec<TPt<TGixExpItem<TKey, TItem, TGixMerger> > > ExpItemV(KeyV.Len(), 0);
    for (int KeyN = 0; KeyN < KeyV.Len(); KeyN++) {
        ExpItemV.Add(TGixExpItem<TKey, TItem, TGixMerger>::NewItem(KeyV[KeyN]));
    }
    return NewAndV(ExpItemV);
}

template <class TKey, class TItem, class TGixMerger>
TPt<TGixExpItem<TKey, TItem, TGixMerger> > TGixExpItem<TKey, TItem, TGixMerger>::NewOrV(const TVec<TKey>& KeyV) {
    TVec<TPt<TGixExpItem<TKey, TItem, TGixMerger> > > ExpItemV(KeyV.Len(), 0);
    for (int KeyN = 0; KeyN < KeyV.Len(); KeyN++) {
        ExpItemV.Add(TGixExpItem<TKey, TItem, TGixMerger>::NewItem(KeyV[KeyN]));
    }
    return NewOrV(ExpItemV);
}

template <class TKey, class TItem, class TGixMerger>
bool TGixExpItem<TKey, TItem, TGixMerger>::Eval(const TPt<TGix<TKey, TItem, TGixMerger> >& Gix,
    TVec<TItem>& ResItemV, const TPt<TGixExpMerger<TKey, TItem>>& Merger) {

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
            if (NotLeft) { Merger->Minus(ResItemV, RightItemV, MinusItemV); } else { Merger->Minus(RightItemV, ResItemV, MinusItemV); }
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

//typedef TGixItemSet<TInt, TInt> TIntGixItemSet;
//typedef TPt<TIntGixItemSet> PIntGixItemSet;
//typedef TGix<TInt, TIntGixItemSet> TIntGix;
//typedef TPt<TIntGix> PIntGix;
//typedef TGixExpItem<TInt, TInt> TIntGixExpItem;
//typedef TPt<TIntGixExpItem> PIntGixExpItem;

#endif

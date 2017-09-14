/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef GIX_H
#define GIX_H

/////////////////////////////////////////////////
// Forward-declarations
template <class TKey, class TItem> class TGix;

/////////////////////////////////////////////////
/// Item Vector Merger.
/// Used when evaluating queries to apply logical operators to item vectors.
template <class TKey, class TItem>
class TGixMerger {
public:
    virtual ~TGixMerger() {}

    /// Add elements in JoinV that are not yet in MainV. Both vectors must be sorted.
    virtual void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    /// Removed elements from MainV that are not in JoinV. Both vectors must be sorted.
    virtual void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    /// Adds elements from MainV that are not in JoinV to ResV. MainV and JoinV must be sorted.
    virtual void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, TVec<TItem>& ResV) const = 0;

    /// Initialize vector of items for given key.
    virtual void Def(const TKey& Key, TVec<TItem>& MainV) const = 0;

    /// Merge repeated items in the ItemV vector.
    /// TODO: What is IsLocal parameter?
    virtual void Merge(TVec<TItem>& ItemV, const bool& IsLocal) const = 0;

    /// Remove all occurences of Item in MainV
    virtual void Delete(const TItem& Item, TVec<TItem>& MainV) const = 0;
    /// Is Item1 < Item2?
    virtual bool IsLt(const TItem& Item1, const TItem& Item2) const = 0;
    /// Is Item1 <= Item2?
    virtual bool IsLtE(const TItem& Item1, const TItem& Item2) const = 0;

    /// Memory footprint
    virtual uint64 GetMemUsed() const = 0;
};

/////////////////////////////////////////////////
/// Default Item Vector Merger.
/// Uses basic set operations defined on TVec.
template <class TKey, class TItem>
class TGixDefMerger : public TGixMerger < TKey, TItem > {
public:
    void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Union(JoinV); }
    void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Intrs(JoinV); }
    void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, TVec<TItem>& ResV) const { MainV.Diff(JoinV, ResV); }
    void Def(const TKey& Key, TVec<TItem>& MainV) const { }
    void Merge(TVec<TItem>& ItemV, const bool& IsLocal) const { ItemV.Merge(); }
    void Delete(const TItem& Item, TVec<TItem>& MainV) const { return MainV.DelAll(Item); }
    bool IsLt(const TItem& Item1, const TItem& Item2) const { return Item1 < Item2; }
    bool IsLtE(const TItem& Item1, const TItem& Item2) const { return Item1 <= Item2; }

    uint64 GetMemUsed() const { return sizeof(TGixDefMerger<TKey, TItem>); }
};

/////////////////////////////////////////////////
/// Key-To-String transformer
template <class TKey>
class TGixKeyStr {
protected:
    TCRef CRef;
    typedef TPt<TGixKeyStr<TKey> > PGixKeyStr;

public:
    virtual ~TGixKeyStr() {}
    static PGixKeyStr New() { return new TGixKeyStr <TKey> ; }

    // by default we cannot assume much about key, so just return empty string
    virtual TStr GetKeyNm(const TKey& Key) const { return TStr(); }

    friend class TPt<TGixKeyStr<TKey> >;
};

/////////////////////////////////////////////////
/// Item Set.
/// Holds set of items that correspond to one key. Itemset supports supports splitting of
/// data into child vectors so there is no need to store complete item set in memory.
/// Assumes child vectors are individually and globaly merged.
template <class TKey, class TItem>
class TGixItemSet {
private:
    TCRef CRef;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;

private:
    /// Meta-data about child vector
    struct TChildInfo {
    public:
        /// Value of the smallest item in the vector
        TItem MinItem;
        /// Value of the largest item in the vector
        TItem MaxItem;
        /// Number of elements in the vector
        TInt Len;
        /// Pointer to the vector in the blob base
        TBlobPt Pt;
        /// Did we load the vector from blob base to memory?
        TBool LoadedP;
        /// Is the version of vector in the memory different to the one in blob base?
        TBool DirtyP;

    public:
        /// Empty child vector info
        TChildInfo(): Len(0), LoadedP(false), DirtyP(false) {}
        /// Create non-emtpy child vector info
        TChildInfo(const TItem& _MinItem, const TItem& _MaxItem, const TInt& _Len, const TBlobPt& _Pt):
            MinItem(_MinItem), MaxItem(_MaxItem), Len(_Len), Pt(_Pt), LoadedP(false), DirtyP(false) {}

        /// Load child info from stream
        TChildInfo(TSIn& SIn): LoadedP(false), DirtyP(false) { Load(SIn); }
        /// Save child info to stream
        void Load(TSIn& SIn);
        /// serialize to stream
        void Save(TSOut& SOut) const;

        /// Memory footprint
        uint64 GetMemUsed() const;
    };

private:
    /// The key of this itemset
    TKey ItemSetKey;

    /// Working buffer of items of this itemset.
    /// Could be only part of them, others can be stored in child vectors
    TVec<TItem> ItemV;
    /// List of indices with "deleted" items
    TVec<TInt> ItemVDel;
    /// Combined count - from this itemset and children
    TInt TotalCnt;

    /// optional data about child vectors - will be populated only for frequent keys
    mutable TVec<TChildInfo> ChildInfoV;
    /// optional list of child vector contents - will be populated only for frequent keys
    mutable TVec<TVec<TItem> > ChildV;

    /// For keeping the items unique and sorted
    TBool MergedP;
    /// Should this itemset be stored to disk?
    TBool DirtyP;

    /// Pointer to gix used to access storage and merger.
    /// (serialization of self, loading children, notifying about changes...)
    const TGix<TKey, TItem>* Gix;

private:
    /// Load single child vector into memory if not present already
    void LoadChildVector(const int& ChildN) const;
    /// Load all child vectors into memory and get pointers to them
    void LoadChildVectors() const;
    /// Refresh total count
    void RecalcTotalCnt();
    /// Check if there are any dirty child vectors with size outside the tolerance
    int FirstDirtyChild();
    /// Get the index of the first child index from which onward the content needs to be merged
    /// There can be other children with smaller indices that are dirty, but we might not want to merge them
    int GetFirstChildToMerge();
    /// Work buffer is merged and still full, add new children collections with the data in work buffer
    void PushWorkBufferToChildren();
    /// If work buffer contains data that belongs to child vectors then push that content to them
    void InjectWorkBufferToChildren();
    /// Data has been merged in memory and needs to be pushed to child vectors (overwrite them)
    void PushMergedDataBackToChildren(const int& FirstChildToMerge, const TVec<TItem>& MergedItems);
    /// Process any pending "delete" commands
    void ProcessDeletes();

    /// Ask child vectors about their memory usage
    uint64 GetChildMemUsed() const { return TMemUtils::GetExtraMemberSize(ChildV); }

public:
    /// Create empty itemset
    TGixItemSet(const TKey& _ItemSetKey, const TGix<TKey, TItem>* _Gix) :
        ItemSetKey(_ItemSetKey), TotalCnt(0), MergedP(true), DirtyP(true), Gix(_Gix) {}
    /// Create empty itemset
    static PGixItemSet New(const TKey& ItemSetKey, const TGix<TKey, TItem>* Gix) {
        return new TGixItemSet(ItemSetKey, Gix); }

    /// Load itemset from stream
    TGixItemSet(TSIn& SIn, const TGix<TKey, TItem>* _Gix);
    /// Load itemset from stream
    static PGixItemSet Load(TSIn& SIn, const TGix<TKey, TItem>* Gix) {
        return new TGixItemSet(SIn, Gix); }
    /// Saves this itemset to stream
    void Save(TMOut& SOut);

    // functions called by TCache
    /// Report memory footprint
    uint64 GetMemUsed() const;
    /// Callback when items sets are kicked out from cache and need to be serialized
    void OnDelFromCache(const TBlobPt& BlobPt, void* Gix);

    /// Get key that this itme set represent
    const TKey& GetKey() const { return ItemSetKey; }

    /// Add new item to the item set. When NotifyCacheOnlyDelta is set to true,
    /// only item set memory footprint differences are sent to gix.
    void AddItem(const TItem& NewItem, const bool& NotifyCacheOnlyDelta = true);
    /// Add a set of items at once
    void AddItemV(const TVec<TItem>& NewItemV);

    /// Check if this itemset is empty
    bool Empty() const { return GetItems() == 0; }
    /// Get number of items (including child itemsets)
    int GetItems() const { return TotalCnt; }
    /// Get item at given index (including child itemsets)
    const TItem& GetItem(const int& ItemN) const;
    /// Get items into vector
    void GetItemV(TVec<TItem>& _ItemV);
    /// Delete specified item from this itemset
    void DelItem(const TItem& Item);
    /// Clear all items from this itemset
    void Clr();

    /// Pack/merge this itemset
    void Def();
    /// Pack/merge working buffer from this itemset
    void DefLocal();

    /// Flag if itemset is merged
    bool IsMerged() const { return MergedP; }
    /// Flag if itemset is dirty
    bool IsDirty() const { return DirtyP; }
    /// Tests if current itemset is full and subsequent item should be pushed to children
    bool IsFull() const { return (ItemV.Len() >= Gix->GetSplitLen()); }

    /// Compute percentage of loaded child vectors
    double GetLoadedPerc() const;

#ifdef XTEST
    // only exposed in test
    friend class XTest;
    void Print() const;
#endif

    /// Smart pointer is a friend
    friend class TPt<TGixItemSet>;
};

//////////////////////////////////////////////////
// Basic statistics for TGix
struct TGixStats {
public:
    /// Number of itemsets in cache
    TInt CacheAll;
    /// Number of dirty itemsets in cache
    TInt CacheDirty;
    /// Percentage of loaded content for itemsets in cache
    TFlt CacheAllLoadedPerc;
    /// Percentage of loaded content for dirty itemsets in cache
    TFlt CacheDirtyLoadedPerc;
    /// Average length of itemsets in cache
    TFlt AvgLen;
    /// memory usage for gix
    TUInt64 MemUsed;

public:

    /// This method combines statistics from to Gix objects
    void Add(const TGixStats& Stats) {
        // compute summed stats
        TGixStats NewStats;
        NewStats.CacheAll = CacheAll + Stats.CacheAll;
        NewStats.CacheDirty = CacheDirty + Stats.CacheDirty;
        if (NewStats.CacheAll > 0) {
            NewStats.CacheAllLoadedPerc = (CacheAll * CacheAllLoadedPerc + Stats.CacheAll * Stats.CacheAllLoadedPerc) / NewStats.CacheAll;
            NewStats.AvgLen = (CacheAll * AvgLen + Stats.CacheAll * Stats.AvgLen) / NewStats.CacheAll;
        }
        if (NewStats.CacheDirty > 0) {
            NewStats.CacheDirtyLoadedPerc = (CacheDirty * CacheDirtyLoadedPerc + Stats.CacheDirty * Stats.CacheDirtyLoadedPerc) / NewStats.CacheDirty;
        }
        NewStats.MemUsed = MemUsed + Stats.MemUsed;
        // replace this stats with summed up ones
        *this = NewStats;
    }
};

/////////////////////////////////////////////////
// General Inverted Index
template <class TKey, class TItem>
class TGix {
private:
    TCRef CRef;
    typedef TPt<TGix<TKey, TItem> > PGix;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
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

    /// Merger used for packing item vectors in item sets
    const TGixMerger<TKey, TItem>* Merger;

    /// Item set cache
    mutable TCache<TBlobPt, PGixItemSet> ItemSetCache;
    /// Disk storage (blob base)
    PBlobBs ItemSetBlobBs;

    /// Threshold for recomputing size of the cache
    uint64 CacheResetThreshold;
    /// Cache size change since last reset
    mutable uint64 NewCacheSizeInc;
    /// flag indicating cache is full
    bool CacheFullP;

    /// Size of work-buffer
    TInt SplitLen;
    /// Minimal length for child vectors
    TInt SplitLenMin;
    /// Maximal length for child vectors
    TInt SplitLenMax;
    /// Can the first child vector be of any non-empty size and not be merged with following vectors
    /// This can significantly speed-up deleting items from Gix
    TBool FirstChildBeUnfilledP;

    /// Internal member for holding statistics
    mutable TGixStats Stats;

private:
    /// Returns pointer to this object. Used in cache call-backs
    void* GetVoidThis() const { return (void*)this; }
    /// asserts if we are allowed to change this index
    void AssertReadOnly() const;

    /// get keyid of a given key and create it if does not exist
    TBlobPt AddKeyId(const TKey& Key);
    /// get keyid of a given key
    TBlobPt GetKeyId(const TKey& Key) const;

    /// Get handle to the merger
    const TGixMerger<TKey, TItem>* GetMerger() const { return Merger; }

    /// Load child vector for given blob pointer from disk
    void GetChildVector(const TBlobPt& Pt, TVec<TItem>& Dest) const;
    /// Store child vectors to disk and get back pointer to where it was stored.
    TBlobPt StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) const;
    /// Delete child vectors from cache and disk
    void DeleteChildVector(const TBlobPt& KeyId) const;
    /// For enlisting new child vectors into blob
    TBlobPt EnlistChildVector(const TVec<TItem>& Data) const;

    /// This method refreshes gix statistics
    void RefreshStats() const;

    TGix(const TStr& Nm, const TStr& FPath, const TFAccess& _Access,
        const TGixMerger<TKey, TItem>* Merger, const int64& CacheSize,
        const int _SplitLen, const bool _FirstChildBeUnfilledP,
        const int _SplitLenMin, const int _SplitLenMax);
public:
    static PGix New(const TStr& Nm, const TStr& FPath, const TFAccess& Access,
        const TGixMerger<TKey, TItem>* Merger, const int64& CacheSize = 100000000,
        const int SplitLen = 1024, const bool FirstChildBeUnfilledP = true,
        const int SplitLenMin = 512, const int SplitLenMax = 2048) {
        return new TGix(Nm, FPath, Access, Merger, CacheSize, SplitLen,
            FirstChildBeUnfilledP, SplitLenMin, SplitLenMax);
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
    bool CanFirstChildBeUnfilled() const { return FirstChildBeUnfilledP; }

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
    /// Get items for given key
    void GetItemV(const TKey& Key, TVec<TItem>& ItemV) const;
    /// for storing item sets from cache to blob
    TBlobPt StoreItemSet(const TBlobPt& KeyId);
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

    /// get first key id
    int FFirstKeyId() const { return KeyIdH.FFirstKeyId(); }
    /// get next key id
    bool FNextKeyId(int& KeyId) const { return KeyIdH.FNextKeyId(KeyId); }
    /// get key for given key id
    const TKey& GetKey(const int& KeyId) const { return KeyIdH.GetKey(KeyId); }

    /// Get amount of memory currently used
    int64 GetMemUsed() const;
    /// Get current cache increment size count
    int GetNewCacheSizeInc() const { return NewCacheSizeInc; }
    /// Get current cache size
    uint64 GetCacheSize() const { return ItemSetCache.GetMemUsed(); }
    /// Get maximal memory that can be used by the cache
    uint64 GetMxMemUsed() const { return ItemSetCache.GetMxMemUsed(); }
    /// Is cache full?
    bool IsCacheFull() const { return CacheFullP; }
    /// Refresh current memory computations
    void RefreshMemUsed();
    /// Update cache increment
    void AddToNewCacheSizeInc(const uint64& Diff) const { NewCacheSizeInc += Diff; }
    /// Update cache increment (or decrement)
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

#ifdef XTEST
    friend class XTest;
    void KillHash() { this->KeyIdH.Clr(); }
    void KillCache() { this->ItemSetCache.FlushAndClr(); }
#endif

    /// Smartpointer friend
    friend class TPt<TGix>;
    /// Itemset needs access to the disk storage and merger
    friend class TGixItemSet<TKey, TItem>;
};

/////////////////////////////////////////////////
/// Expression item types
typedef enum {
    getUndef,
    getEmpty, //< Empty item
    getOr,    //< Logical or
    getAnd,   //< Logical and
    getNot,   //< Logical not
    getKey    //< Lookup for items index under key in Gix
} TGixExpType;

/////////////////////////////////////////////////
/// Expression item
template <class TKey, class TItem>
class TGixExpItem {
private:
    TCRef CRef;
    typedef TPt<TGixExpItem<TKey, TItem> > PGixExpItem;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
    typedef TPt<TGix<TKey, TItem> > PGix;

private:
    /// Type of the opreation handled by this expression item
    TGixExpType ExpType;
    /// Left subtree (used by getOr and getAnd)
    PGixExpItem LeftExpItem;
    /// Right subtree (used by getOr, getAnd and getNot)
    PGixExpItem RightExpItem;
    /// Search key (used by getKey)
    TKey Key;

private:
    /// Convert expression item to AND
    void PutAnd(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);
    /// Convert expression item to OR
    void PutOr(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);

    TGixExpItem(const TGixExpType& _ExpType, const PGixExpItem& _LeftExpItem,
        const PGixExpItem& _RightExpItem) : ExpType(_ExpType),
        LeftExpItem(_LeftExpItem), RightExpItem(_RightExpItem) {}
    TGixExpItem(const TKey& _Key) : ExpType(getKey), Key(_Key) {}
    TGixExpItem() : ExpType(getEmpty) {}
    TGixExpItem(const TGixExpItem& ExpItem) : ExpType(ExpItem.ExpType),
        LeftExpItem(ExpItem.LeftExpItem), RightExpItem(ExpItem.RightExpItem),
        Key(ExpItem.Key) {}

public:
    // elementary operations
    static PGixExpItem NewOr(const PGixExpItem& LeftExpItem, const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getOr, LeftExpItem, RightExpItem); }
    static PGixExpItem NewAnd(const PGixExpItem& LeftExpItem, const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getAnd, LeftExpItem, RightExpItem); }
    static PGixExpItem NewNot(const PGixExpItem& RightExpItem) {
        return new TGixExpItem(getNot, NULL, RightExpItem); }
    static PGixExpItem NewItem(const TKey& Key) {
        return new TGixExpItem(Key); }
    static PGixExpItem NewEmpty() {
        return new TGixExpItem(); }

    /// Create an AND tree from given array of expression items
    static PGixExpItem NewAndV(const TVec<PGixExpItem>& ExpItemV);
    /// Create an OR tree from given array of expression items
    static PGixExpItem NewOrV(const TVec<PGixExpItem>& ExpItemV);
    /// Create an AND tree from given array of leaf items
    static PGixExpItem NewAndV(const TVec<TKey>& KeyV);
    /// Create an OR tree from given array of leaf items
    static PGixExpItem NewOrV(const TVec<TKey>& KeyV);

    /// Is current item empty?
    bool IsEmpty() const { return (ExpType == getEmpty); }
    /// Get type of expression item
    TGixExpType GetExpType() const { return ExpType; }
    /// Get expression item Key
    TKey GetKey() const { return Key; }
    /// Clone expression item
    PGixExpItem Clone() const { return new TGixExpItem(*this); }

    /// Evaluate expression item using given merger and return mathed items
    bool Eval(const PGix& Gix, TVec<TItem>& ResItemV, const TGixMerger<TKey, TItem>* Merger);

    friend class TPt<TGixExpItem>;
};

#include "gix.hpp"

#endif

/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef GIX_H
#define GIX_H

// this file depends only on base.h

/////////////////////////////////////////////////
// Forward-declarations
template <class TKey, class TItem> class TGix;
template <class TKey, class TItem> class TGixStorageLayer;

/////////////////////////////////////////////////
// General-Inverted-Index-Merger
template <class TKey, class TItem>
class TGixMerger {
protected:
    TCRef CRef;
    typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;

public:
	virtual ~TGixMerger() { }

    virtual void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    virtual void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const = 0;
    virtual void Minus(const TVec<TItem>& MainV, 
		const TVec<TItem>& JoinV, TVec<TItem>& ResV) const = 0;
	virtual void Merge(TVec<TItem>& ItemV) const = 0;
    virtual void Def(const TKey& Key, TVec<TItem>& MainV) const = 0;
	virtual bool IsLt(const TItem& Item1, const TItem& Item2) const = 0;

    friend class TPt<TGixMerger<TKey, TItem> >;
};

/////////////////////////////////////////////////
// General-Inverted-Index-Default-Merger
template <class TKey, class TItem>
class TGixDefMerger : public TGixMerger<TKey, TItem> {
private:
    typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;

public:
	static PGixMerger New() { return new TGixDefMerger<TKey, TItem>(); }

    void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Union(JoinV); }
    void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Intrs(JoinV); }
    void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, 
        TVec<TItem>& ResV) const { MainV.Diff(JoinV, ResV); }
	void Merge(TVec<TItem>& ItemV) const { ItemV.Merge(); }
    void Def(const TKey& Key, TVec<TItem>& MainV) const  { }
	bool IsLt(const TItem& Item1, const TItem& Item2) const { return Item1 < Item2; }
};

/////////////////////////////////////////////////
// General-Inverted-Index Key-To-String
template <class TKey>
class TGixKeyStr {
protected:
	TCRef CRef;
	typedef TPt<TGixKeyStr<TKey> > PGixKeyStr;

public:
	virtual ~TGixKeyStr() { }
	static PGixKeyStr New() { return new TGixKeyStr<TKey>; }

	// by default we cannot assume much about key, so just return empty string
	virtual TStr GetKeyNm(const TKey& Key) const { return TStr(); }

    friend class TPt<TGixKeyStr<TKey> >;
};

/////////////////////////////////////////////////
// General-Inverted-Index Item-Set
template <class TKey, class TItem>
class TGixItemSet {
private:
    TCRef CRef;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
    typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;
private:
	/// The key of this itemset
    TKey ItemSetKey;
	/// Items of this itemset - could be only part of them, other can be stored in child itemsets
    TVec<TItem> ItemV;
	/// Combined count - from this itemset and children
	int TotalCnt;

	// TODO combine all these into single record (?) analyse

	// TODO dirty flags?
	// optional list of blob pointers to child vectors - will be populated only for frequent keys
	TVec<TBlobPt> Children;
	// optional list of child vector lengths - will be populated only for frequent keys
	TVec<TInt> ChildrenLen;
	// optional list of child vectors - will be populated only for frequent keys
	mutable TVec<TVec<TItem>> ChildrenData;

    // for keeping the ItemV unique and sorted
    TBool MergedP;
	// pointer to merger that will merge this itemset
    PGixMerger Merger;
	// pointer to gix - the storage-layer (serialization of self, loading children, notifying about chnages...)
	const TGix<TKey, TItem> *Gix;


	/// Load single child vector into memory if not present already
	void LoadChildVector(int i) const {
		if (ChildrenLen[i] != ChildrenData[i].Len()) {
			Gix->GetChildVector(Children[i], ChildrenData[i]);
		}
	}
	/// Load all child vectors into memory and get pointers to them
	void LoadChildVectors() {
		for (int i = 0; i < Children.Len(); i++) {
			LoadChildVector(i);
		}
	}

	/// Refresh total count
	void RecalcTotalCnt() {
		TotalCnt = ItemV.Len();
		for (int i = 0; i < ChildrenLen.Len(); i++)
			TotalCnt += ChildrenLen[i];
	}

public:
	/// Standard constructor
	TGixItemSet(const TKey& _ItemSetKey, const PGixMerger& _Merger, const TGix<TKey, TItem>* _Gix) :
		ItemSetKey(_ItemSetKey), MergedP(true), Merger(_Merger), Gix(_Gix), TotalCnt(0) {}
	/// Standard factory method
	static PGixItemSet New(const TKey& ItemSetKey, const PGixMerger& Merger, const TGix<TKey, TItem>* Gix) {
		return new TGixItemSet(ItemSetKey, Merger, Gix);
	}

	/// Constructor for deserialization
	TGixItemSet(TSIn& SIn, const PGixMerger& _Merger, const TGix<TKey, TItem>* _Gix) :
		ItemSetKey(SIn), ItemV(SIn), Children(SIn), ChildrenLen(SIn), MergedP(true), Merger(_Merger), Gix(_Gix)  { 
		RecalcTotalCnt();
	}
	/// Standard factory method for deserialization
	static PGixItemSet Load(TSIn& SIn, const PGixMerger& Merger, const TGix<TKey, TItem>* Gix) {
		return new TGixItemSet(SIn, Merger, Gix);
	}
	/// Saves this itemset to output stream
    void Save(TSOut& SOut);

    // functions used by TCache
    int GetMemUsed() const {
        return ItemSetKey.GetMemUsed() + ItemV.GetMemUsed() 
			+ Children.GetMemUsed() + ChildrenLen.GetMemUsed() + ChildrenData.GetMemUsed()
			+ sizeof(TBool) + sizeof(int) + sizeof(PGixMerger);
	}
    void OnDelFromCache(const TBlobPt& BlobPt, void* Gix);

    // key & items
    const TKey& GetKey() const { return ItemSetKey; }
	void AddItem(const TItem& NewItem);
	void AddItemV(const TVec<TItem>& NewItemV);
	void OverrideItems(const TVec<TItem>& NewItemV, int From, int Len);
	/// Get number of items (including child itemsets)
	int GetItems() const { return TotalCnt;	}
	/// Get item at given index (including child itemsets)
	const TItem& GetItem(const int& ItemN) const;

    //const TVec<TItem>& GetItemV() const { return ItemV; }
	void AppendItemSet(const TPt<TGixItemSet>& Src);
	/// Get items into vector
	void GetItemV(TVec<TItem>& _ItemV);
	/// Delete specified item from this itemset
	void DelItem(const TItem& Item);
	/// Clear all items from this itemset
    void Clr();
	/// Pack/merge this itemset
    void Def();

	/// Tests if current itemset is full and subsequent item should be pushed to children
	bool IsFull() { 
		return (ItemV.Len() >= 100);
		//return (ItemV.GetMemUsed() > 10 * 1024 * 1024);  // TODO remove this after dev tests
	}

	friend class TPt<TGixItemSet>; 
#ifdef GIX_TEST
	friend class XTest;
#endif
};

template <class TKey, class TItem>
const TItem& TGixItemSet<TKey, TItem>::GetItem(const int& ItemN) const {
	AssertR(ItemN >= 0 && ItemN < TotalCnt, TStr() + "Index: " + TInt::GetStr(ItemN) + ", TotalCnt: " + TInt::GetStr(TotalCnt));
	if (ItemN < ItemV.Len()) {
		// data is in this itemset
		return ItemV[ItemN];
	}
	// data is in child itemsets
	int index = ItemN - ItemV.Len();
	for (int i = 0; i < ChildrenLen.Len(); i++) {
		if (index < ChildrenLen[i]) {
			// load child vector only if needed
			LoadChildVector(i);
			return ChildrenData[i][index];
		}
		index -= ChildrenLen[i];
	}
	return ItemV[-1]; // will trigger error, should not happen anyway
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Save(TSOut& SOut) { 
	// make sure all is merged before saving - TODO is this needed?
	Def();

	// save child vectors separately
	for (int i = 0; i < Children.Len(); i++) {
		if (ChildrenData[i].Len() > 0) {
			// TODO check dirty bit
			Children[i] = Gix->StoreChildVector(Children[i], ChildrenData[i]);
		}
	}
	// save item key and set
	ItemSetKey.Save(SOut);
	ItemV.Save(SOut);
	Children.Save(SOut);
	ChildrenLen.Save(SOut);
}


template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::OnDelFromCache(const TBlobPt& BlobPt, void* Gix) {
    if (!((TGix<TKey, TItem>*)Gix)->IsReadOnly()) {
		((TGix<TKey, TItem>*)Gix)->StoreItemSet(BlobPt);
    } 
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::AddItem(const TItem& NewItem) { 
    const int OldSize = GetMemUsed();
	if (IsFull()) {
		bool AddNewChild = false; // flag if new child Itemset should be added
		const TItem *LastItem;    // pointer to last item in the itemset, will be used for calculating MergedP
		if (Children.Len() == 0) {
			AddNewChild = true;
			LastItem = &ItemV.Last();
		} else {
			LoadChildVector(Children.Len() - 1);
			TVec<TItem>& Last = ChildrenData.Last();
			LastItem = &(Last.Last());
			
			// TODO perform Level1 merge?
			
			if (Last.Len() == ItemV.Len()) {
				AddNewChild = true;
			} else {
				AddNewChild = false;
				Last.Add(NewItem);
				ChildrenLen.Last() = Last.Len();
			}
		}
		if (AddNewChild){
			printf("adding new child, at index %d \n", Children.Len() + 1);
			TVec<TItem> new_child;
			new_child.Add(NewItem);
			ChildrenData.Add(new_child);
			ChildrenLen.Add(1);
			Children.Add(Gix->EnlistChildVector(new_child));
		}
		if (MergedP) {
			MergedP = (ItemV.Len() == 0 ? true : Merger->IsLt(*LastItem, NewItem));
		}
	} else {		
		if (MergedP) {
			// if itemset is merged and the newly added item is bigger than the last one
			// the itemset remains merged
			MergedP = (ItemV.Len() == 0 ? true : Merger->IsLt(ItemV.Last(), NewItem));
		}
		ItemV.Add(NewItem);
	}
	RecalcTotalCnt(); // child itemsets might have been merged
	// notify cache that this item grew
	Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::AddItemV(const TVec<TItem>& NewItemV) { 
	for (int i = 0; i < NewItemV.Len(); i++) {
		AddItem(NewItemV[i]);
	}
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::AppendItemSet(const TPt<TGixItemSet>& Src) {
	// access data of Src itemset directly, no child records
	AddItemV(Src->ItemV);
	Src->LoadChildVectors();
	// ok, now process children if present
	if (Src->Children.Len() > 0) {
		// merge each of them
		for (int i = 0; i < ChildrenData.Len(); i++) {
			AddItemV(Src->ChildrenData);
		}
	}
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::GetItemV(TVec<TItem>& _ItemV) {
	_ItemV = ItemV;
	if (Children.Len() > 0) {
		// collect data from child itemsets
		LoadChildVectors();
		for (int i = 0; i < Children.Len(); i++) {
			_ItemV.AddV(ChildrenData[i]);
		}
	}
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::DelItem(const TItem& Item) {
	Def();
	const int OldSize = GetMemUsed();
	ItemV.DelIfIn(Item);
	if (Children.Len()>0) {
		LoadChildVectors();
		for (int i = 0; i < Children.Len(); i++) {
			ChildrenData[i].DelIfIn(Item);
			ChildrenLen[i] = ChildrenData[i].Len();
		}
	}
	RecalcTotalCnt();
	Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Clr() { 
    const int OldSize = GetMemUsed();
	ItemV.Clr();
	if (Children.Len()>0) {
		LoadChildVectors();
		for (int i = 0; i < Children.Len(); i++) {
			Gix->DeleteChildVector(Children[i]);
		}
		ChildrenLen.Clr();
		ChildrenData.Clr();
		Children.Clr();
	}
	MergedP = true;
	TotalCnt = 0;
    Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
}

template <class TKey, class TItem>
void TGixItemSet<TKey, TItem>::Def() { 
	// call merger to pack items, if not merged yet 
	if (!MergedP) { 
		if (Children.Len() > 0) {
			// complex merge - need to also use child itemsets			
			TVec<TItem> MergedItems;  
			GetItemV(MergedItems); // collect all items - this also retrieves data from child vectors
			Merger->Merge(MergedItems); // perform merge
			int split_len = ItemV.Len(); // use first vector's size as measure for splitting

			// now save them back
			int curr_index = 0;
			int child_index = -1;
			while (curr_index < MergedItems.Len()) {
				int len = TMath::Mn<int>(split_len, MergedItems.Len() - curr_index);
				if (child_index < 0) {
					ItemV.Clr();
					MergedItems.GetSubValV(curr_index, curr_index + len - 1, ItemV);
				} else {
					ChildrenData[child_index].Clr();
					MergedItems.GetSubValV(curr_index, curr_index + len - 1, ChildrenData[child_index]);
					ChildrenLen[child_index] = ChildrenData[child_index].Len();					
				}
				curr_index += len;
				child_index++;
			}

			/*
			// clear children that became empty - 1 should remain, others must be destroyed
			bool keep_next_child = true;
			int last_good_child = -1;
			while (child_index < Children.Len()) {
				if (keep_next_child) {
					keep_next_child = false;
					PGixItemSet ItemSet = GetChildItemSet(child_index);
					ItemSet->Clr();
					last_good_child = child_index;
				} else {
					// destroy this itemset 
					Gix->DeleteItemSet(Children[child_index]);
				}
				child_index++;
			}
			if (last_good_child > 0 && last_good_child < Children.Len() - 1) {
				// remove deleted itemsets
				Children.Del(last_good_child + 1, Children.Len() - 1);
				ChildrenLen.Del(last_good_child + 1, ChildrenLen.Len() - 1);
			}
			*/

			// clear children that became empty - kill'em all
			int first_empty_child = child_index;
			while (child_index < Children.Len()) {
				Gix->DeleteChildVector(Children[child_index++]); // remove from storage
			}
			if (first_empty_child < Children.Len()) {
				// remove deleted itemsets
				Children.Del(first_empty_child, Children.Len() - 1);
				ChildrenLen.Del(first_empty_child, ChildrenLen.Len() - 1);
				ChildrenData.Del(first_empty_child, ChildrenData.Len() - 1);
			}
		} else {
			// no child records, just merge this 
			Merger->Merge(ItemV);
		}
		RecalcTotalCnt();
		MergedP = true;
	}
}
/*
/////////////////////////////////////////////////
// General-Inverted-Index Storage Layer

template <class TKey, class TItem>
class TGixStorageLayer {
private:
	TCRef CRef;
	typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
	typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;
	typedef TGixDefMerger<TKey, TItem> _TGixDefMerger;
	typedef TPt<TGixKeyStr<TKey> > PGixKeyStr;
	typedef TPt<TGixStorageLayer<TKey, TItem> > PGixStorageLayer;
private:
	/// File access mode - checked during index operations
	TFAccess Access;
	/// Name of the BLOB file
	TStr GixBlobFNm;
	/// cache for BLOB data, mapping between pointers and data
	mutable TCache<TBlobPt, PGixItemSet> ItemSetCache;
	/// BLOB handler
	PBlobBs ItemSetBlobBs;
	/// record merger, used for packing data vectors
	PGixMerger Merger;

	int64 CacheResetThreshold;
	mutable int64 NewCacheSizeInc;
	/// flag if cache is full
	bool CacheFullP;

	// returns pointer to this object (used in cache call-backs)
	void* GetVoidThis() const { return (void*)this; }
	/// asserts if we are allowed to change this index
	void AssertReadOnly() const {
		EAssertR(((Access == faCreate) || (Access == faUpdate)),
			"Index opened in Read-Only mode!");
	}

public:
	TGixStorageLayer(const TStr& _GixBlobFNm,
		const TFAccess& _Access = faRdOnly, const int64& CacheSize = 100000000,
		const PGixMerger& _Merger = _TGixDefMerger::New());
	static PGixStorageLayer New(const TStr& GixBlobFNm,
		const TFAccess& Access = faRdOnly, const int64& CacheSize = 100000000,
		const PGixMerger& Merger = _TGixDefMerger::New()) {
		return new TGixStorageLayer(GixBlobFNm, Access, CacheSize, Merger);
	}

	~TGixStorageLayer();

	// Gix properties
	bool IsReadOnly() const { return Access == faRdOnly; }
	bool IsCacheFullP() const { return CacheFullP; }
	TStr GetFPath() const { return GixFNm.GetFPath(); }
	int64 GetMxCacheSize() const { return ItemSetCache.GetMxMemUsed(); }

	/// get item set for given BLOB pointer
	PGixItemSet GetItemSet(const TBlobPt& Pt) const;

	/// get child vector for given blob pointer
	void GetChildVector(const TBlobPt& Pt, TVec<TItem>& Dest) const;

	// delete one item
	void DelItem(const TKey& Key, const TItem& Item);
	/// clears items
	void Clr(const TKey& Key);

	/// get amount of memory currently used
	int64 GetMemUsed() const {
		return int64(sizeof(TFAccess) + GixFNm.GetMemUsed() + GixBlobFNm.GetMemUsed()) + int64(ItemSetCache.GetMemUsed());
	}
	int GetNewCacheSizeInc() const { return NewCacheSizeInc; }
	int GetCacheSize() const { return ItemSetCache.GetMemUsed(); }
	int64 GetMxMemUsed() const { return ItemSetCache.GetMxMemUsed(); }
	bool IsCacheFull() const { return CacheFullP; }
	void RefreshMemUsed();
	void AddToNewCacheSizeInc(int64 diff) const { NewCacheSizeInc += diff; }

	/// for storing item sets from cache to blob
	TBlobPt StoreItemSet(const TBlobPt& KeyId);
	/// for deleting item sets from cache and blob
	void DeleteItemSet(const TBlobPt& KeyId) const;
	/// For enlisting new itemsets into blob
	TBlobPt EnlistItemSet(const PGixItemSet& ItemSet) const;

	/// for storing child vectors to blob
	TBlobPt StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data);
	/// for deleting child vectors from cache and blob
	void DeleteChildVector(const TBlobPt& KeyId) const;
	/// For enlisting new child vectors into blob
	TBlobPt EnlistChildVector(const TVec<TItem>& Data) const;

	/// print statistics for index keys
	//void SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const;

	friend class TPt<TGixStorageLayer>;

#ifdef GIX_DEBUG
	void PrintCacheKeys() {
		TBlobPt BlobPt; 
		PGixItemSet ItemSet;
		void* KeyDatP = ItemSetCache.FFirstKeyDat();
		while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) { 
			printf("%d\n\r", ItemSet->GetKey().Val1); 
		}
		printf("-------------------\n\r");
	}
#endif
};

template <class TKey, class TItem>
TGixStorageLayer<TKey, TItem>::TGixStorageLayer(const TStr& _GixBlobFNm, const TFAccess& _Access,
	const int64& CacheSize, const TPt<TGixMerger<TKey, TItem> >& _Merger) : GixBlobFNm(_GixBlobFNm), Access(_Access),
	ItemSetCache(CacheSize, 1000000, GetVoidThis()), Merger(_Merger) {

	if (Access == faCreate) {
		// creating a new Gix
		ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, faCreate);
	} else {
		// loading an old Gix and getting it ready for search and update
		EAssert((Access == faUpdate) || (Access == faRdOnly) || (Access == faRestore));
		// load ItemSets from GixBlobFNm
		ItemSetBlobBs = TMBlobBs::New(GixBlobFNm, Access);
	}

	CacheResetThreshold = int64(0.1 * double(CacheSize));
	NewCacheSizeInc = 0; 
	CacheFullP = false;
}

template <class TKey, class TItem>
TGixStorageLayer<TKey, TItem>::~TGixStorageLayer() {
	if ((Access == faCreate) || (Access == faUpdate)) {
		// flush all the latest changes in cache to the disk
		ItemSetCache.Flush();
	}
}

template <class TKey, class TItem>
TPt<TGixItemSet<TKey, TItem> > TGixStorageLayer<TKey, TItem>::GetItemSet(const TBlobPt& KeyId) const {
	if (KeyId.Empty()) { return NULL; }
	PGixItemSet ItemSet;
	if (!ItemSetCache.Get(KeyId, ItemSet)) {
		// have to load it from the hard drive...
		PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
		ItemSet = TGixItemSet<TKey, TItem>::Load(*ItemSetSIn, Merger, this);
	}
	// bring the itemset to the top of the cache
	ItemSetCache.Put(KeyId, ItemSet);
	return ItemSet;
}

template <class TKey, class TItem>
void TGixStorageLayer<TKey, TItem>::GetChildVector(const TBlobPt& KeyId, TVec<TItem>& Dest) const {
	if (KeyId.Empty()) { return; }
	PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
	Dest.Load(*ItemSetSIn);
}

template <class TKey, class TItem>
void TGixStorageLayer<TKey, TItem>::RefreshMemUsed() {
	// check if we have to drop anything from the cache
	if (NewCacheSizeInc > CacheResetThreshold) {
		printf("Cache clean-up [%s] ... ", TUInt64::GetMegaStr(NewCacheSizeInc).CStr());
		// pack all the item sets
		TBlobPt BlobPt; 
		PGixItemSet ItemSet;
		void* KeyDatP = ItemSetCache.FFirstKeyDat();
		while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) { ItemSet->Def(); }
		// clean-up cache
		CacheFullP = ItemSetCache.RefreshMemUsed();
		NewCacheSizeInc = 0;
		const uint64 NewSize = ItemSetCache.GetMemUsed();
		printf("Done [%s]\n", TUInt64::GetMegaStr(NewSize).CStr());
	}
}

template <class TKey, class TItem>
TBlobPt TGixStorageLayer<TKey, TItem>::StoreItemSet(const TBlobPt& KeyId) {
	AssertReadOnly(); // check if we are allowed to write
	// get the pointer to the item set
	PGixItemSet ItemSet; 
	EAssert(ItemSetCache.Get(KeyId, ItemSet));
	// store the current version to the blob
	TMOut MOut; 
	ItemSet->Save(MOut);
	return ItemSetBlobBs->PutBlob(KeyId, MOut.GetSIn());
}

template <class TKey, class TItem>
void TGixStorageLayer<TKey, TItem>::DeleteItemSet(const TBlobPt& KeyId) const {
	AssertReadOnly(); // check if we are allowed to write
	ItemSetCache.Del(KeyId, false); // don't trigger callback, we will handle it
	ItemSetBlobBs->DelBlob(KeyId);  // free space in BLOB
}

template <class TKey, class TItem>
TBlobPt TGixStorageLayer<TKey, TItem>::EnlistItemSet(const PGixItemSet& ItemSet) const {
	AssertReadOnly(); // check if we are allowed to write
	TMOut MOut;
	ItemSet->Save(MOut);
	TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
	//printf("enlisted new itemset to storage: %d %d \n", res.Addr, res.Seg);
	return res;
}

/// for storing vectors to blob
template <class TKey, class TItem>
TBlobPt TGixStorageLayer<TKey, TItem>::StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) {
	AssertReadOnly(); // check if we are allowed to write
	// store the current version to the blob
	TMOut MOut;
	Data.Save(MOut);
	return ItemSetBlobBs->PutBlob(ExistingKeyId, MOut.GetSIn());
}

/// for deleting child vector from blob
template <class TKey, class TItem>
void TGixStorageLayer<TKey, TItem>::DeleteChildVector(const TBlobPt& KeyId) const {
	AssertReadOnly(); // check if we are allowed to write
	ItemSetBlobBs->DelBlob(KeyId);  // free space in BLOB
}

/// For enlisting new child vectors into blob
template <class TKey, class TItem>
TBlobPt TGixStorageLayer<TKey, TItem>::EnlistChildVector(const TVec<TItem>& Data) const {
	AssertReadOnly(); // check if we are allowed to write
	TMOut MOut;
	Data.Save(MOut);
	TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
	//printf("enlisted new child vector to storage: %d %d \n", res.Addr, res.Seg);
	return res;
}
*/


/////////////////////////////////////////////////
// General-Inverted-Index
template <class TKey, class TItem>
class TGix {
private:
    TCRef CRef;
    typedef TPt<TGix<TKey, TItem> > PGix;
	//typedef TPt<TGixStorageLayer<TKey, TItem> > PGixStorageLayer;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
    typedef TPt<TGixMerger<TKey, TItem> > PGixMerger;
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
	PGixMerger Merger;

	/// cache for BLOB data, mapping between pointers and data
	mutable TCache<TBlobPt, PGixItemSet> ItemSetCache;
	/// BLOB handler
	PBlobBs ItemSetBlobBs;

	int64 CacheResetThreshold;
	mutable int64 NewCacheSizeInc;
	/// flag if cache is full
	bool CacheFullP;

	// returns pointer to this object (used in cache call-backs)
	void* GetVoidThis() const { return (void*)this; }

    /// asserts if we are allowed to change this index
    void AssertReadOnly() const {
        EAssertR(((Access==faCreate)||(Access==faUpdate)), 
            "Index opened in Read-Only mode!"); }

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

public:
    TGix(const TStr& Nm, const TStr& FPath = TStr(), 
		const TFAccess& _Access = faRdOnly, const int64& CacheSize = 100000000, 
		const PGixMerger& _Merger = _TGixDefMerger::New() );
    static PGix New(const TStr& Nm, const TStr& FPath = TStr(), 
		const TFAccess& Access = faRdOnly, const int64& CacheSize = 100000000, 
		const PGixMerger& Merger = _TGixDefMerger::New()) {
            return new TGix(Nm, FPath, Access, CacheSize, Merger); }
	
	~TGix();

    // Gix properties
    bool IsReadOnly() const { return Access == faRdOnly; }
	bool IsCacheFullP() const { return CacheFullP; }
	TStr GetFPath() const { return GixFNm.GetFPath(); }
	int64 GetMxCacheSize() const { return GixSL->GetMxMemUsed(); }

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
	/// for deleting item sets from cache and blob
	void DeleteItemSet(const TBlobPt& KeyId) const;
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
        return int64(sizeof(TFAccess) + GixFNm.GetMemUsed() + GixBlobFNm.GetMemUsed()) + 
            int64(KeyIdH.GetMemUsed()) + int64(ItemSetCache.GetMemUsed()); }
	int GetNewCacheSizeInc() const { return NewCacheSizeInc; }
	int GetCacheSize() const { return ItemSetCache.GetMemUsed(); }
	int64 GetMxMemUsed() const { return ItemSetCache.GetMxMemUsed(); }
	bool IsCacheFull() const { return CacheFullP; }
	void RefreshMemUsed();
	void AddToNewCacheSizeInc(int64 diff) const { NewCacheSizeInc += diff; }


	/// print statistics for index keys
	void SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const;

    friend class TPt<TGix>;
    friend class TGixItemSet<TKey, TItem>;
#ifdef GIX_TEST
	friend class XTest;
#endif
};

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::AddKeyId(const TKey& Key) { 
    if (IsKey(Key)) { return KeyIdH.GetDat(Key); }
    // we don't have this key, create an empty item set and return pointer to it
    AssertReadOnly(); // check if we are allowed to write
	PGixItemSet ItemSet = TGixItemSet<TKey, TItem>::New(Key, Merger, this);
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
TGix<TKey, TItem>::TGix(const TStr& Nm, const TStr& FPath, const TFAccess& _Access, 
  const int64& CacheSize, const TPt<TGixMerger<TKey, TItem> >& _Merger): Access(_Access),
  ItemSetCache(CacheSize, 1000000, GetVoidThis()), 
  Merger(_Merger) {

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
	//GixSL = TGixStorageLayer<TKey, TItem>::New(GixBlobFNm, Access, 1000000, Merger);

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
        TFOut FOut(GixFNm); 
		KeyIdH.Save(FOut);
    }
}

template <class TKey, class TItem>
TPt<TGixItemSet<TKey, TItem> > TGix<TKey, TItem>::GetItemSet(const TKey& Key) const {
    TBlobPt KeyId = GetKeyId(Key);
	return GetItemSet(KeyId);
}
template <class TKey, class TItem>
TPt<TGixItemSet<TKey, TItem> > TGix<TKey, TItem>::GetItemSet(const TBlobPt& KeyId) const {
	if (KeyId.Empty()) { return NULL; }
	PGixItemSet ItemSet;
	if (!ItemSetCache.Get(KeyId, ItemSet)) {
		// have to load it from the hard drive...
		PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
		ItemSet = TGixItemSet<TKey, TItem>::Load(*ItemSetSIn, Merger, this);
	}
	// bring the itemset to the top of the cache
	ItemSetCache.Put(KeyId, ItemSet);
	return ItemSet;
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::StoreItemSet(const TBlobPt& KeyId) {
	AssertReadOnly(); // check if we are allowed to write
	// get the pointer to the item set
	PGixItemSet ItemSet;
	EAssert(ItemSetCache.Get(KeyId, ItemSet));
	// store the current version to the blob
	TMOut MOut;
	ItemSet->Save(MOut);
	return ItemSetBlobBs->PutBlob(KeyId, MOut.GetSIn());
}

template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::EnlistItemSet(const PGixItemSet& ItemSet) const {
	AssertReadOnly(); // check if we are allowed to write
	TMOut MOut;
	ItemSet->Save(MOut);
	TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
	//printf("enlisted new itemset to storage: %d %d \n", res.Addr, res.Seg);
	return res;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::AddItem(const TKey& Key, const TItem& Item) {
    AssertReadOnly(); // check if we are allowed to write
    // get the key handle
    TBlobPt KeyId = AddKeyId(Key);
    // load the current item set
	PGixItemSet ItemSet = GetItemSet(Key);
    ItemSet->AddItem(Item);
    // check if we have to drop anything from the cache
    RefreshMemUsed();
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::AddItemV(const TKey& Key, const TVec<TItem>& ItemV) {
    AssertReadOnly(); // check if we are allowed to write
    // get the key handle
    TBlobPt KeyId = AddKeyId(Key);
    // load the current item set
    PGixItemSet ItemSet = GetItemSet(Key);	
	// add the new items to the set and update the size of new items
    ItemSet->AddItemV(ItemV);    
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
void TGix<TKey, TItem>::MergeIndex(const TPt<TGix<TKey, TItem> >& TmpGix) {
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
            printf("[%d/%d]\r", TmpKeyId, TmpKeys); }
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
		const int MemUsed = ItemSet->GetMemUsed();
		// output statistics
		FOut.PutStrFmtLn("%s\t%d\t%d", KeyNm .CStr(), Items, MemUsed);
	}
	printf("Done: %d / %d\n", Keys, Keys);
}

/// for storing vectors to blob
template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::StoreChildVector(const TBlobPt& ExistingKeyId, const TVec<TItem>& Data) const {
	printf("Storing child vector - len=%d, addr=%d\n", Data.Len(), ExistingKeyId.Addr);
	AssertReadOnly(); // check if we are allowed to write
	// store the current version to the blob
	TMOut MOut;
	Data.Save(MOut);
	return ItemSetBlobBs->PutBlob(ExistingKeyId, MOut.GetSIn());
}

/// for deleting child vector from blob
template <class TKey, class TItem>
void TGix<TKey, TItem>::DeleteChildVector(const TBlobPt& KeyId) const {
	printf("Deleting child vector - addr=%d\n", KeyId.Addr);
	AssertReadOnly(); // check if we are allowed to write
	ItemSetBlobBs->DelBlob(KeyId);  // free space in BLOB
}

/// For enlisting new child vectors into blob
template <class TKey, class TItem>
TBlobPt TGix<TKey, TItem>::EnlistChildVector(const TVec<TItem>& Data) const {
	printf("Enlisting child vector - len=%d\n", Data.Len());
	AssertReadOnly(); // check if we are allowed to write
	TMOut MOut;
	Data.Save(MOut);
	TBlobPt res = ItemSetBlobBs->PutBlob(MOut.GetSIn());
	//printf("enlisted new child vector to storage: %d %d \n", res.Addr, res.Seg);
	return res;
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::GetChildVector(const TBlobPt& KeyId, TVec<TItem>& Dest) const {
	if (KeyId.Empty()) { return; }
	PSIn ItemSetSIn = ItemSetBlobBs->GetBlob(KeyId);
	Dest.Load(*ItemSetSIn);
}

template <class TKey, class TItem>
void TGix<TKey, TItem>::RefreshMemUsed() {
	// check if we have to drop anything from the cache
	if (NewCacheSizeInc > CacheResetThreshold) {
		printf("Cache clean-up [%s] ... ", TUInt64::GetMegaStr(NewCacheSizeInc).CStr());
		// pack all the item sets
		TBlobPt BlobPt;
		PGixItemSet ItemSet;
		void* KeyDatP = ItemSetCache.FFirstKeyDat();
		while (ItemSetCache.FNextKeyDat(KeyDatP, BlobPt, ItemSet)) { ItemSet->Def(); }
		// clean-up cache
		CacheFullP = ItemSetCache.RefreshMemUsed();
		NewCacheSizeInc = 0;
		const uint64 NewSize = ItemSetCache.GetMemUsed();
		printf("Done [%s]\n", TUInt64::GetMegaStr(NewSize).CStr());
	}
}

/////////////////////////////////////////////////
// General-Inverted-Index Expression-Item
typedef enum { getUndef, getEmpty, getOr, getAnd, getNot, getKey } TGixExpType;

template <class TKey, class TItem>
class TGixExpItem {
private:
    TCRef CRef;
    typedef TPt<TGixExpItem<TKey, TItem> > PGixExpItem;
    typedef TPt<TGixItemSet<TKey, TItem> > PGixItemSet;
    typedef TPt<TGix<TKey, TItem> > PGix;
    typedef TGixDefMerger<TKey, TItem> _TGixDefMerger;
private:
    TGixExpType ExpType;
    PGixExpItem LeftExpItem;
    PGixExpItem RightExpItem;
    TKey Key;

    TGixExpItem(const TGixExpType& _ExpType, const PGixExpItem& _LeftExpItem, 
      const PGixExpItem& _RightExpItem): ExpType(_ExpType), 
        LeftExpItem(_LeftExpItem), RightExpItem(_RightExpItem) { }
    TGixExpItem(const TKey& _Key): ExpType(getKey), Key(_Key) { }
    TGixExpItem(): ExpType(getEmpty) { }
	TGixExpItem(const TGixExpItem& ExpItem): ExpType(ExpItem.ExpType),
		LeftExpItem(ExpItem.LeftExpItem), RightExpItem(ExpItem.RightExpItem),
		Key(ExpItem.Key) { }

	void PutAnd(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);
	void PutOr(const PGixExpItem& _LeftExpItem, const PGixExpItem& _RightExpItem);
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

	// some predifined structures
	static PGixExpItem NewAndV(const TVec<PGixExpItem>& ExpItemV);
	static PGixExpItem NewOrV(const TVec<PGixExpItem>& ExpItemV);
	static PGixExpItem NewAndV(const TVec<TKey>& KeyV);
	static PGixExpItem NewOrV(const TVec<TKey>& KeyV);

    bool IsEmpty() const { return (ExpType == getEmpty); }
    TGixExpType GetExpType() const { return ExpType; }
	TKey GetKey() const { return Key; }
	PGixExpItem Clone() const { return new TGixExpItem(*this); }
    bool Eval(const PGix& Gix, TVec<TItem>& ResItemV, 
		const TPt<TGixMerger<TKey, TItem> >& Merger = _TGixDefMerger::New());

    friend class TPt<TGixExpItem>;
};

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
        TVec<TItem>& ResItemV, const TPt<TGixMerger<TKey, TItem> >& Merger) {

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
            if (NotLeft) { Merger->Minus(ResItemV, RightItemV, MinusItemV); }
            else { Merger->Minus(RightItemV, ResItemV, MinusItemV); }
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
            if (NotLeft) { Merger->Minus(RightItemV, ResItemV, MinusItemV); }
            else { Merger->Minus(ResItemV, RightItemV, MinusItemV); }
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

typedef TGixItemSet<TInt, TInt> TIntGixItemSet;
typedef TPt<TIntGixItemSet> PIntGixItemSet;
typedef TGix<TInt, TIntGixItemSet> TIntGix;
typedef TPt<TIntGix> PIntGix;
typedef TGixExpItem<TInt, TInt> TIntGixExpItem;
typedef TPt<TIntGixExpItem> PIntGixExpItem;

#endif
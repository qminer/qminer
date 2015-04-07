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

//#include "tm.h"
#include <inttypes.h>

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

	void Union(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Union(JoinV); }
	void Intrs(TVec<TItem>& MainV, const TVec<TItem>& JoinV) const { MainV.Intrs(JoinV); }
	void Minus(const TVec<TItem>& MainV, const TVec<TItem>& JoinV, TVec<TItem>& ResV) const { MainV.Diff(JoinV, ResV); }
	void Def(const TKey& Key, TVec<TItem>& MainV) const {}

	void Merge(TVec<TItem>& ItemV, bool IsLocal = false) const { ItemV.Merge(); }
	void Delete(const TItem& Item, TVec<TItem>& MainV) const { return MainV.DelAll(Item); }
	bool IsLt(const TItem& Item1, const TItem& Item2) const { return Item1 < Item2; }
	bool IsLtE(const TItem& Item1, const TItem& Item2) const { return Item1 <= Item2; }
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
			const TInt& _Len, const TBlobPt& _Pt, const TBool& _MergedP)
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
		for (int i = 0; i < Children.Len(); i++)
			TotalCnt += Children[i].Len;
	}

	/// Check if there are any dirty child vectors with size outside the tolerance
	int FirstDirtyChild() {
		for (int i = 0; i < Children.Len(); i++) {
			if (Children[i].Dirty && (Children[i].Len < Gix->GetSplitLenMin() || Children[i].Len > Gix->GetSplitLenMax()))
				return i;
		}
		return -1;
	}

	/// Work buffer is merged and still full, push it to children collection
	void PushWorkBufferToChildren();

	/// Work buffer contains data that needs to be injected into child vectors
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
		ItemSetKey(_ItemSetKey), MergedP(true), Dirty(true), Merger(_Merger), Gix(_Gix), TotalCnt(0) {}
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
	/// Get number of items (including child itemsets)
	int GetItems() const { return TotalCnt; }
	/// Get item at given index (including child itemsets)
	const TItem& GetItem(const int& ItemN) const;

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

	/// Tests if current itemset is full and subsequent item should be pushed to children
	bool IsFull() const {
		return (ItemV.Len() >= Gix->GetSplitLen());
	}

	friend class TPt < TGixItemSet > ;
	friend class TGix < TKey, TItem, TGixMerger > ;

#ifdef GIX_TEST
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

#ifdef GIX_TEST

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
	if (!((TGix<TKey, TItem, TGixMerger>*)Gix)->IsReadOnly() && Dirty) {
		((TGix<TKey, TItem, TGixMerger>*)Gix)->StoreItemSet(BlobPt);
	}
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::PushWorkBufferToChildren() {
	// push work-buffer into children array
	int split_len = Gix->GetSplitLen();
	while (ItemV.Len() >= split_len) {
		TVec<TItem> tmp;
		//ItemV.GetSubValVMemCpy(0, split_len - 1, tmp);
		ItemV.GetSubValV(0, split_len - 1, tmp);
		TGixItemSetChildInfo child_info(tmp[0], tmp.Last(), split_len, Gix->EnlistChildVector(tmp), true);
		child_info.Loaded = false;
		child_info.Dirty = false;
		Children.Add(child_info);
		ChildrenData.Add(TVec<TItem>());
		//ItemV.DelMemCpy(0, split_len - 1);
		ItemV.Del(0, split_len - 1);
		Dirty = true;
	}
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::InjectWorkBufferToChildren() {
	if (Children.Len() > 0 && ItemV.Len() > 0) {
		int i = 0;
		int j = 0;
		while (i < ItemV.Len()) {
			TItem val = ItemV[i++];
			while (j < Children.Len() && Merger->IsLt(Children[j].MaxVal, val)) {
				j++;
			}
			if (j < Children.Len()) { // ok, insert into j-th child
				LoadChildVector(j);
				ChildrenData[j].Add(val);
				Children[j].Len = ChildrenData[j].Len();
				Children[j].Dirty = true;
			} else {
				i--;
				break; // all remaining values in input buffer will not be inserted into child vectors 
			}
		}
		// delete items from work-buffer that have been inserted into child vectors
		if (i < ItemV.Len()) {
			if (i > 0) {
				//ItemV.DelMemCpy(0, i - 1);
				ItemV.Del(0, i - 1);
				Dirty = true;
			}
		} else {
			ItemV.Clr();
			Dirty = true;
		}
		// merge dirty un-merged children
		for (int j = 0; j < Children.Len(); j++) {
			if (Children[j].Dirty) {
				LoadChildVector(j); // just in case - they should be in memory at this point anyway
				TVec<TItem>& cd = ChildrenData[j];
				Merger->Merge(cd, false);
				Children[j].Len = cd.Len();
				Children[j].Dirty = true;
				if (cd.Len() > 0) {
					Children[j].MinVal = cd[0];
					Children[j].MaxVal = cd.Last();
				}
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

	// clear children that became empty - kill'em all
	int first_empty_child = child_index;
	while (child_index < Children.Len()) { // remove them from BLOB storage
		Gix->DeleteChildVector(Children[child_index++].Pt); // remove from storage
	}
	if (first_empty_child < Children.Len()) { // now remove their stuff from memory
		Children.Del(first_empty_child, Children.Len() - 1);
		ChildrenData.Del(first_empty_child, ChildrenData.Len() - 1);
	}
	Dirty = true;
}


template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::AddItem(const TItem& NewItem, const bool& NotifyCacheOnlyDelta) {
    //const uint64 OldSize = GetMemUsed();
    const uint64 OldSize = (NotifyCacheOnlyDelta ? GetMemUsed() : 0); // avoid calculation of GetMemUsed if not needed
	if (IsFull()) {
		Def();
		if (IsFull()) {
			PushWorkBufferToChildren();
		}
		RecalcTotalCnt(); // work buffer might have been merged
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
    if (ItemV.Len() == 0) {
        ItemV.Reserve(2);
    }
	ItemV.Add(NewItem);
	Dirty = true;
	TotalCnt++;

	// notify cache that this item grew
    if (NotifyCacheOnlyDelta) {
        Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
    } else {
        Gix->AddToNewCacheSizeInc(GetMemUsed());
    }
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
    const uint64 OldSize = GetMemUsed();
	if (IsFull()) {
		Def();
		if (IsFull()) {
			PushWorkBufferToChildren();
		}
		RecalcTotalCnt(); // work buffer might have been merged
	}
	ItemVDel.Add(ItemV.Len());
	ItemV.Add(Item);
	MergedP = false;
	Dirty = true;
	TotalCnt++;

	Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
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
	Gix->AddToNewCacheSizeInc(GetMemUsed() - OldSize);
}

template <class TKey, class TItem, class TGixMerger>
void TGixItemSet<TKey, TItem, TGixMerger>::ProcessDeletes() {
	if (ItemVDel.Len() > 0) {
		TVec<TItem> ItemVNew;
		int ItemVNewI = 0;

		for (int i = 0; i < ItemVDel.Len(); i++) {
			TItem val = ItemV[ItemVDel[i]];
			// first delete data from children
			int j = Children.Len() - 1;
			while (j >= 0 && Merger->IsLtE(val, Children[j].MaxVal)) {
				if (Merger->IsLtE(Children[j].MinVal, val)) {
					LoadChildVector(j);
					Merger->Delete(val, ChildrenData[j]);
					Children[j].Len = ChildrenData[j].Len();
					Children[j].Dirty = true;
					// we don't update stats (min & max), because they are still usable.
				}
				j--;
			}
			// delete from the new work-buffer
			while (ItemVNewI <= ItemVDel[i]) {
				ItemVNew.Add(ItemV[ItemVNewI++]);
			}
			Merger->Delete(val, ItemVNew);
		}
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

		int first_dirty_child = FirstDirtyChild();
		if (first_dirty_child >= 0 || Children.Len() > 0 && ItemV.Len() > 0) {
			int first_child_to_merge = (first_dirty_child >= 0 ? first_dirty_child : Children.Len());

			// collect all data from subsequent child vectors and work-buffer
			TVec<TItem> MergedItems;
			for (int i = first_child_to_merge; i < Children.Len(); i++) {
				LoadChildVector(i);
				//MergedItems.AddVMemCpy(ChildrenData[i]);
				MergedItems.AddV(ChildrenData[i]);
			}
			//MergedItems.AddVMemCpy(ItemV);
			MergedItems.AddV(ItemV);
			Merger->Merge(MergedItems, false); // perform global merge

			PushMergedDataBackToChildren(first_child_to_merge, MergedItems); // now save them back
			PushWorkBufferToChildren(); // it could happen that data in work buffer is still to large

		} else if (Children.Len() > 0 && ItemV.Len() == 0) {
			// nothing, children should already be merged and work-buffer is empty
		} else {
			// nothing, there are no children and work-buffer has already been merged
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
			Merger->Merge(ItemV, true); // perform local merge
			Dirty = true;
			if (Children.Len() > 0 && ItemV.Len() > 0) {
				if (Merger->IsLt(Children.Last().MaxVal, ItemV[0])) {
					MergedP = true; // local merge achieved global merge
				}
			} else {
				MergedP = true;
			}
			RecalcTotalCnt();
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

	/// Internal member for holding statistics
	TGixStats Stats;
	/// This method refreshes gix statistics
	void RefreshStats();

public:
	TGix(const TStr& Nm, const TStr& FPath = TStr(),
		const TFAccess& _Access = faRdOnly, const int64& CacheSize = 100000000,
		int _SplitLen = 100000);
	static PGix New(const TStr& Nm, const TStr& FPath = TStr(),
		const TFAccess& Access = faRdOnly, const int64& CacheSize = 100000000,
		int _SplitLen = 100000) {
		return new TGix(Nm, FPath, Access, CacheSize, _SplitLen);
	}

	~TGix();

	// Gix properties
	bool IsReadOnly() const { return Access == faRdOnly; }
	bool IsCacheFullP() const { return CacheFullP; }
	TStr GetFPath() const { return GixFNm.GetFPath(); }
	int64 GetMxCacheSize() const { return GetMxMemUsed(); }
	int GetSplitLen() const { return SplitLen; };
	int GetSplitLenMax() const { return SplitLenMax; };
	int GetSplitLenMin() const { return SplitLenMin; };

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
    void AddToNewCacheSizeInc(uint64 diff) const { NewCacheSizeInc += diff; }


	/// print statistics for index keys
	void SaveTxt(const TStr& FNm, const PGixKeyStr& KeyStr) const;
	/// print simple statistics for cache
	void PrintStats();
	/// get blob stats
	const TBlobBsStats& GetBlobStats() { return ItemSetBlobBs->GetStats(); }
	/// get gix stats
	const TGixStats& GetGixStats(bool do_refresh = true) {
		if (do_refresh) RefreshStats();
		return Stats;
	}
	/// reset blob stats
	void ResetStats() { ItemSetBlobBs->ResetStats(); }

	friend class TPt < TGix > ;
	friend class TGixItemSet < TKey, TItem, TGixMerger > ;
#ifdef GIX_TEST
	friend class XTest;

	void KillHash() { this->KeyIdH.Clr(); }
	void KillCache() { this->ItemSetCache.FlushAndClr(); }
#endif
};



template <class TKey, class TItem, class TGixMerger>
int TGix<TKey, TItem, TGixMerger>::PartialFlush(int WndInMsec) {
	TTmStopWatch sw(true);
	TLstNd<TBlobPt>* current = ItemSetCache.Last();
	PGixItemSet curr;
	int cnt = 0;
	int cnt_all = 0;
	TBlobPt b;
	while (current != NULL) {
		if (sw.GetMSecInt() > WndInMsec) break;
		b = current->Val;
		if (ItemSetCache.Get(current->Val, curr)) {
			if (curr->Dirty) {
				TBlobPt b_new = StoreItemSet(current->Val);
				ItemSetCache.ChangeKey(b, b_new); // blob pointer might have changed, update cache
				cnt++;
			}
		}
		cnt_all++;
		current = current->PrevNd;
	}
	printf("Partial flush - %d itemsets saved to disk, scanned %d - %f.\n", cnt, cnt_all, ((double)cnt / cnt_all));
	return cnt;
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
	const int64& CacheSize, int _SplitLen) : Access(_Access),
	ItemSetCache(CacheSize, 1000000, GetVoidThis()), SplitLen(_SplitLen) {

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
	int Tolerance = SplitLen / 10;
	SplitLenMax = SplitLen + Tolerance;
	SplitLenMin = SplitLen - Tolerance;
}

template <class TKey, class TItem, class TGixMerger>
TGix<TKey, TItem, TGixMerger>::~TGix() {
	if ((Access == faCreate) || (Access == faUpdate)) {
		this->PrintStats();
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
	if (KeyId.Empty()) { return NULL; }
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
	// store the current version to the blob
	TMOut MOut;
	ItemSet->Save(MOut);
	TBlobPt NewKeyId = ItemSetBlobBs->PutBlob(KeyId, MOut.GetSIn());
	// and update the KeyId in the hash table
	KeyIdH.GetDat(ItemSet->GetKey()) = NewKeyId;
	return NewKeyId;
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
void TGix<TKey, TItem, TGixMerger>::RefreshStats() {
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
	Stats.CacheAllLoadedPerc /= Stats.CacheAll;
	Stats.CacheDirtyLoadedPerc /= Stats.CacheDirty;
	Stats.AvgLen /= Stats.CacheAll;
}


/// print simple statistics for cache
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::PrintStats() {
	RefreshStats();
	printf(".... gix cache stats - all=%d dirty=%d, loaded_perc=%f dirty_loaded_perc=%f, avg_len=%f, mem_used=%d \n",
		Stats.CacheAll, Stats.CacheDirty, Stats.CacheAllLoadedPerc, Stats.CacheDirtyLoadedPerc,
		Stats.AvgLen, Stats.MemUsed);
	const TBlobBsStats& blob_stats = ItemSetBlobBs->GetStats();
	printf(".... gix blob stats - puts=%" PRIu64 " puts_new=%" PRIu64 " gets=%" PRIu64 " dels=%" PRIu64 " size_chngs=%" PRIu64 " avg_len_get=%f avg_len_put=%f avg_len_put_new=%f\n",
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
	return ItemSetBlobBs->PutBlob(ExistingKeyId, MOut.GetSIn());
}

/// for deleting child vector from blob
template <class TKey, class TItem, class TGixMerger>
void TGix<TKey, TItem, TGixMerger>::DeleteChildVector(const TBlobPt& KeyId) const {
	AssertReadOnly(); // check if we are allowed to write
	ItemSetBlobBs->DelBlob(KeyId);  // free space in BLOB
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
		printf("Cache clean-up [%s] ... ", TUInt64::GetMegaStr(NewCacheSizeInc).CStr());
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
		const uint64 NewSize = ItemSetCache.GetMemUsed();
		printf("Done [%s]\n", TUInt64::GetMegaStr(NewSize).CStr());
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
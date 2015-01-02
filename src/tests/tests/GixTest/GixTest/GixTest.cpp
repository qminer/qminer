#define GIX_DEBUG
#define GIX_TEST

#include <base.h>
#include <mine.h>

#include <iostream>
#include <windows.h>

////////////////////////////////////////////////////////////////////////
// typedefs

//typedef  TGixDefMerger < TIntUInt64Pr, TUInt64 > TMyMerger;
//typedef TPt<TGixMerger<TIntUInt64Pr, TUInt64> > PGixMerger;

typedef TIntUInt64Pr TMyKey;
typedef TKeyDat<TUInt64, TInt> TMyItem;

// forward defs

class TMyGixDefMerger;

////////////////////////////////////////////////////////////////////////
// more typedefs
typedef  TGixItemSet < TMyKey, TMyItem, TMyGixDefMerger > TMyItemSet;
typedef  TPt < TMyItemSet > PMyItemSet;

typedef TGix<TMyKey, TMyItem, TMyGixDefMerger> TMyGix;
typedef TPt<TGix<TMyKey, TMyItem, TMyGixDefMerger> > PMyGix;


///////////////////////////////////////////////////////////////////////
// for nice outputs

HANDLE  hConsole;

void WarnNotifyI(TStr& const s) {
	SetConsoleTextAttribute(hConsole, 11);
	WarnNotify(s);
	SetConsoleTextAttribute(hConsole, 15);
}
void WarnNotifyW(TStr& const s) {
	SetConsoleTextAttribute(hConsole, 14);
	WarnNotify(s);
	SetConsoleTextAttribute(hConsole, 15);
}

#define TAssert(Cond, MsgCStr) \
  ((Cond) ? static_cast<void>(0) : WarnNotifyW( TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) +": "+ MsgCStr))


/////////////////////////////////////////////////////////////////////////

class TMyGixDefMerger : public TGixExpMerger < TMyKey, TMyItem > {
public:
	static PGixExpMerger New() { return new TMyGixDefMerger(); }

	// overridde abstract methods
	void Union(TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV) const;
	void Intrs(TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV) const;
	void Minus(const TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV, TVec<TMyItem>& ResV) const;
	void Def(const TMyKey& Key, TVec<TMyItem>& MainV) const {}

	// methods needed for usage in template
	void Merge(TVec<TMyItem>& ItemV, bool IsLocal = false) const;
	void Delete(const TMyItem& Item, TVec<TMyItem>& MainV) const { return MainV.DelAll(Item); }
	bool IsLt(const TMyItem& Item1, const TMyItem& Item2) const { return Item1 < Item2; }
	bool IsLtE(const TMyItem& Item1, const TMyItem& Item2) const { return Item1 <= Item2; }
};

void TMyGixDefMerger::Union(TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV) const {
	TVec<TMyItem> ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TMyItem& Val1 = MainV.GetVal(ValN1);
		const TMyItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) {
			ResV.Add(Val1); ValN1++;
		} else if (Val1 > Val2) {
			ResV.Add(Val2); ValN2++;
		} else {
			ResV.Add(TMyItem(Val1.Key, Val1.Dat + Val2.Dat));
			ValN1++;
			ValN2++;
		}
	}
	for (int RestValN1 = ValN1; RestValN1 < MainV.Len(); RestValN1++) {
		ResV.Add(MainV.GetVal(RestValN1));
	}
	for (int RestValN2 = ValN2; RestValN2 < JoinV.Len(); RestValN2++) {
		ResV.Add(JoinV.GetVal(RestValN2));
	}
	MainV = ResV;
}

void TMyGixDefMerger::Intrs(TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV) const {
	TVec<TMyItem> ResV; int ValN1 = 0; int ValN2 = 0;
	while ((ValN1 < MainV.Len()) && (ValN2 < JoinV.Len())) {
		const TMyItem& Val1 = MainV.GetVal(ValN1);
		const TMyItem& Val2 = JoinV.GetVal(ValN2);
		if (Val1 < Val2) {
			ValN1++;
		} else if (Val1 > Val2) {
			ValN2++;
		} else {
			ResV.Add(TMyItem(Val1.Key, Val1.Dat + Val2.Dat));
			ValN1++;
			ValN2++;
		}
	}
	MainV = ResV;
}

void TMyGixDefMerger::Minus(const TVec<TMyItem>& MainV, const TVec<TMyItem>& JoinV, TVec<TMyItem>& ResV) const {
	MainV.Diff(JoinV, ResV);
}

void TMyGixDefMerger::Merge(TVec<TMyItem>& ItemV, bool IsLocal) const {
	//printf("============================================\n");
	//for (int i = 0; i < ItemV.Len(); i++) {
	//	printf("   (%d) %d %d\n", i, ItemV[i].Key, (int)ItemV[i].Dat);
	//}
	if (ItemV.Empty()) { return; } // nothing to do in this case
	if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted

	//printf("============================================\n");
	//for (int i = 0; i < ItemV.Len(); i++) {
	//	printf("   (%d) %d %d\n", i, ItemV[i].Key, (int)ItemV[i].Dat);
	//}

	// merge counts
	int LastItemN = 0; bool ZeroP = false;
	for (int ItemN = 1; ItemN < ItemV.Len(); ItemN++) {
		if (ItemV[ItemN] != ItemV[ItemN - 1]) {
			LastItemN++;
			ItemV[LastItemN] = ItemV[ItemN];
		} else {
			ItemV[LastItemN].Dat += ItemV[ItemN].Dat;
		}
		ZeroP = (ItemV[LastItemN].Dat <= 0) || ZeroP;
	}
	ItemV.Reserve(ItemV.Reserved(), LastItemN + 1);
	// remove items with zero count
	if (ZeroP) {
		LastItemN = 0;
		for (int ItemN = 0; ItemN < ItemV.Len(); ItemN++) {
			const TMyItem& Item = ItemV[ItemN];
			if (Item.Dat > 0 || (IsLocal && Item.Dat < 0)) {
				ItemV[LastItemN] = Item;
				LastItemN++;
			} else if (Item.Dat < 0) {
				printf("Warning: negative item count %d:%d!\n", (int)Item.Key, (int)Item.Dat);
			}
		}
		ItemV.Reserve(ItemV.Reserved(), LastItemN);
	}
	//printf("============================================\n");
	//for (int i = 0; i < ItemV.Len(); i++) {
	//	printf("   (%d) %d %d\n", i, ItemV[i].Key, (int)ItemV[i].Dat);
	//}
}

////////////////////////////////////////////////////////////////////////

class XTest {
public:

	void Test_Simple_1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, TMyItem(7234, 1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 0, "Cache should contain 0 items");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->TotalCnt == 1, "Invalid itemset TotalCnt");
	}

	void Test_Simple_220() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Work-buffer should be full");
		TAssert(itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->TotalCnt == 220, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 20, "Itemset content - invalid ItemV length");
		TAssert(itemset->Children[0].Len == 100, "Itemset content - invalid first child length");
		TAssert(itemset->Children[1].Len == 100, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 0, "Itemset child content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 0, "Itemset child content - invalid second child length");

		gix.ItemSetCache.FlushAndClr();
		//TAssert(gix.GetCacheSize() == 0, "Cache should contain 0 items");
	}

	void Test_Simple_220_Unsorted() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			int item = (i * 15485867) % 16381;
			gix.AddItem(x, TMyItem(item, 1)); // pseudo-random numbers
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should not be merged");
		TAssert(itemset->TotalCnt == 220, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 20, "Itemset content - invalid ItemV length");
		TAssert(itemset->Children[0].Len == 100, "Itemset content - invalid first child length");
		TAssert(itemset->Children[1].Len == 100, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 0, "Itemset content - invalid second child length");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 20, "Itemset content - invalid ItemV length");
		TAssert(itemset->Children[0].Len == 100, "Itemset content - invalid first child length");
		TAssert(itemset->Children[1].Len == 100, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 100, "Itemset content - invalid second child length");

	}

	void Test_Merge_220_Into_50() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i % 50, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should not be full");
		TAssert(!itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 0 children");
		// 50 merged + 20 unmerged
		TAssert(itemset->ItemV.Len() == 70, TStr::Fmt("Itemset content - invalid ItemV length - %d", itemset->ItemV.Len()));

		itemset->Def();

		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 0 children");
		TAssert(itemset->ItemV.Len() == 50, "Itemset content - invalid ItemV length");

	}

	void Test_Merge_220_Into_120() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i % 120, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should not be merged");
		TAssert(itemset->Children.Len() == 1, "Itemset should have 1 children");
		// 20 merged + 20 unmerged
		TAssert(itemset->ItemV.Len() == 40, "Itemset content - invalid ItemV length");
		TAssert(itemset->Children[0].Len == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");

		itemset->Def();

		TAssert(!itemset->IsFull(), "Itemset should not be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 1, "Itemset should have 1 child");
		TAssert(itemset->ItemV.Len() == 20, "Itemset content - invalid ItemV length");
		TAssert(itemset->Children[0].Len == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");

	}

	void Test_Merge_22000_Into_50() {
		TMyGix gix("Test1", "data", faCreate, 10000000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 22000; i++) {
			gix.AddItem(x, TMyItem(i % 50, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should not be merged");
		TAssert(itemset->TotalCnt == 100, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 1 child");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");

		itemset->Def();

		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 50, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 0 children");
		TAssert(itemset->ItemV.Len() == 50, "Itemset content - invalid ItemV length");

	}

	void AddToCounter(THash<TInt, TInt>& counter, TInt i) {
		if (!counter.IsKey(i)) {
			counter.AddKey(i);
		}
		counter.GetDat(i)++;
	}

	void CheckCounts(THash<TInt, TInt>& counts, TMyGix& gix) {
		printf("Checking counts...\n");
		for (auto &key : counts) {
			auto itemset = gix.GetItemSet(TIntUInt64Pr(key.Key, (int)key.Key));
			auto cnt = key.Dat;
			TAssert(itemset->MergedP, "Itemset should be merged");
			TAssert(itemset->TotalCnt == cnt, TStr::Fmt("Invalid itemset TotalCnt: key=%d, expected=%d, actual=%d", key.Key, cnt, itemset->TotalCnt));
		}
		printf("Checking counts done.\n");
	}
	void OverwriteCounts(THash<TInt, TInt>& counts, TMyGix& gix) {
		printf("Overwritting counts...\n");
		for (auto &key : counts) {
			auto itemset = gix.GetItemSet(TIntUInt64Pr(key.Key, (int)key.Key));
			key.Dat = itemset->TotalCnt;
		}
		printf("Overwritting counts done.\n");
	}

	void Test_Feed(int cache_size = 50 * 1024 * 1024, int split_len = 100) {
		TStr Nm("Test_Feed");
		TStr FName("data");
		int total = 30000;
		int keys = 0;

		THash<TInt, TInt> counts;
		{
			// simmulate news feed
			// many articles, containing 50 random words + everyone containing words 1-5
			TMyGix gix(Nm, FName, faCreate, cache_size, split_len);
			TRnd rnd(1);
			for (int j = 0; j < total; j++) {
				// every doc containes the same 5 words
				for (int i = 1; i <= 5; i++) {
					gix.AddItem(TIntUInt64Pr(i, i), TMyItem(j, 1));
				}
				// each document contains 50 random words
				TVec<int> vals;
				for (int i = 0; i < 50; i++) {
					int w = rnd.GetUniDevInt(10, 10000);

					// prevent the same word for the same document
					while (vals.IsIn(w)) {
						w = rnd.GetUniDevInt(10, 10000);
					}
					vals.Add(w);

					auto key = TMyKey(w, w);
					gix.AddItem(key, TMyItem(j, 1));
					AddToCounter(counts, w);
				}
			}

			auto itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
			TAssert(itemset->IsFull(), "Itemset should be full");
			TAssert(itemset->MergedP, "Itemset should remain merged");
			TAssert(itemset->TotalCnt == total, "Invalid itemset TotalCnt");

			CheckCounts(counts, gix);

			gix.Flush();

			//gix.AddItem(TIntUInt64Pr(1, 1), total + 1);
			//gix.Flush();

			keys = gix.GetKeys();
			itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
			TAssert(itemset->IsFull(), "Itemset should be full");
			TAssert(itemset->MergedP, "Itemset should remain merged");
			TAssert(itemset->TotalCnt == total, TStr::Fmt("Invalid itemset TotalCnt, expected=%d, actual=%d", total, itemset->TotalCnt));

			OverwriteCounts(counts, gix); // itemsets could be merged
		}
		{
			// reload data - in read-only mode
			TMyGix gix(Nm, FName, faRdOnly, 50 * 1024 * 1024);

			TAssert(gix.GetKeys() == keys, "Invalid key count");

			CheckCounts(counts, gix);
		}
	}

	void Test_Delete_1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, TMyItem(7234, 1));
		gix.DelItem(x, TMyItem(7234, 1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 2, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 1, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 1, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 0, "Invalid itemset TotalCnt");
	}

	void Test_Delete_20() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(321, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 41, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 1, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 40, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 1, "Invalid itemset TotalCnt");
	}

	void Test_Delete_20And1() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(321, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));
		gix.AddItem(x, TMyItem(7234, 1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 42, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 1, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 40, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 2, "Invalid itemset TotalCnt");
	}

	void Test_Delete_120() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(i, 1));
		}
		gix.DelItem(x, TMyItem(7234, 120));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 143, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 1, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 42, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 120, "Invalid itemset TotalCnt");
	}

	void Test_Delete_120And1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(i, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));
		gix.AddItem(x, TMyItem(7234, 1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 144, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 1, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 42, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 121, "Invalid itemset TotalCnt");
	}

	void Test_Delete_120And110() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}
		for (int i = 0; i < 110; i++) {
			gix.DelItem(x, TMyItem(i, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == 20 + 20 + 30, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 30, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 20 + 20, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 10, "Invalid itemset TotalCnt");
	}

	void Test_Delete_22000And1000() {
		TMyGix gix("Test1", "data", faCreate, 1000000, 100);
		int xx = 122;
		int all = 22000;
		int to_delete = 980;
		TIntUInt64Pr x(xx, xx);
		// fill all
		for (int i = 0; i < all; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}
		// now delete data from the front
		for (int i = 0; i < to_delete; i++) {
			gix.DelItem(x, TMyItem(i, 1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == all - to_delete + 80 + 80, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == to_delete % 100, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 0, "Invalid list of deletes - invalid index");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == all - to_delete, "Invalid itemset TotalCnt");
		for (int i = to_delete; i < all; i++) {
			TAssert(itemset->GetItem(i - to_delete).Key == i, "Invalid item at specific index");
		}
	}

	void Test_QuasiDelete_120And1And2() {
		TMyGix gix("Test1", "data", faCreate, 1000000, 100);
		int xx = 126;
		int all = 120;
		int to_delete = 5;
		TIntUInt64Pr x(xx, xx);
		// fill all
		for (int i = 0; i < all; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}

		// item "to_delete" was added and is already inside child vector
		// add it again, then delete it twice
		// it should reach frequency 0 and be removed without warning
		gix.AddItem(x, TMyItem(to_delete, +1));
		gix.AddItem(x, TMyItem(to_delete, -1));
		gix.AddItem(x, TMyItem(to_delete, -1));

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == all + 3, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 0, "Invalid list of deletes");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == all - 1, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemV.Len() == all - 100, "Invalid work-buffer length");
		TAssert(itemset->Children[0].Len == 99, "Invalid first-child length");
		for (int i = 0; i < all; i++) {
			if (i == to_delete) {
				continue;
			} else if (i < to_delete) {
				TAssert(itemset->GetItem(i).Key == i, "Invalid item at specific index");
			} else if (i > to_delete) {
				TAssert(itemset->GetItem(i - 1).Key == i, "Invalid item at specific index");
			}
		}
	}

	void Test_QuasiDelete_120And20() {
		TMyGix gix("Test1", "data", faCreate, 1000000, 100);
		int xx = 122;
		int all = 120;
		int to_delete = 20;
		TIntUInt64Pr x(xx, xx);
		// fill all
		for (int i = 0; i < all; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}
		// now quasi-delete data from the front - give negative frequencies and merger will remove the item
		for (int i = 0; i < to_delete; i++) {
			gix.AddItem(x, TMyItem(i, -1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == all + to_delete, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 0, "Invalid list of deletes");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == all - to_delete, "Invalid itemset TotalCnt");
		for (int i = to_delete; i < all; i++) {
			TAssert(itemset->GetItem(i - to_delete).Key == i, "Invalid item at specific index");
		}
	}

	void Test_QuasiDelete_22000And1000() {
		TMyGix gix("Test1", "data", faCreate, 100000000, 100);
		int xx = 122;
		int all = 22000;
		int to_delete = 980;
		TIntUInt64Pr x(xx, xx);
		// fill all
		for (int i = 0; i < all; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}
		// now quasi-delete data from the front - give negative frequencies and merger will remove the item
		for (int i = 0; i < to_delete; i++) {
			gix.AddItem(x, TMyItem(i, -1));
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == all - to_delete + 2 * (to_delete % 100), "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == 0, "Invalid list of deletes");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == all - to_delete, "Invalid itemset TotalCnt");
		for (int i = to_delete; i < all; i++) {
			TAssert(itemset->GetItem(i - to_delete).Key == i, "Invalid item at specific index");
		}
	}

	void Test_BigInserts(int cache_size = 500 * 1024 * 1024, int split_len = 1000) {
		TStr Nm("Test_Feed_Big");
		TStr FName("data");
		int total = 200 * 1000;
		int keys = 0;
		//int voc_count = 50 * 1000; // number of possible words
		int voc_count = 50 * 1000; // number of possible words

		split_len = 1000;
		cache_size = 200 * 1024 * 1024;

		TTmStopWatch sw(true);
		THash<TInt, TInt> counts;
		{
			// simmulate news feed
			// many articles, containing X random words + everyone containing words 1-5
			auto gix = TMyGix::New(Nm, FName, faCreate, cache_size, split_len);
			//TMyGix gix(Nm, FName, faCreate, cache_size, TMyMerger::New(), split_len);
			TRnd rnd(1);
			for (int j = 0; j < total; j++) {
				// every doc contains the same 5 words
				for (int i = 1; i <= 5; i++) {
					gix->AddItem(TIntUInt64Pr(i, i), TMyItem(j, 1));
				}
				// each document contains X random words
				TVec<int> vals;
				int len = rnd.GetUniDevInt(50, 200);
				for (int i = 0; i < len; i++) {
					int w = rnd.GetUniDevInt(6, voc_count);

					// prevent the same word for the same document
					while (vals.IsIn(w)) {
						w = rnd.GetUniDevInt(10, voc_count);
					}
					vals.Add(w);

					auto key = TIntUInt64Pr(w, w);
					gix->AddItem(key, TMyItem(j, 1));
					AddToCounter(counts, w);
				}
				if (sw.GetSec() >= 5) {
					printf("-- %d - %d\n", j, gix->GetCacheSize());
					gix->PrintStats();
					gix->PartialFlush();
					gix->PrintStats();
					sw.Reset(true);
				}
			}

			auto itemset = gix->GetItemSet(TIntUInt64Pr(1, 1));
			TAssert(itemset->IsFull(), "Itemset should be full");
			TAssert(itemset->MergedP, "Itemset should remain merged");
			TAssert(itemset->TotalCnt == total, "Invalid itemset TotalCnt");

			CheckCounts(counts, *gix);

			keys = gix->GetKeys();
			gix->PrintStats();
			gix->Flush();
		}
		{
			// reload data - in read-only mode
			auto gix = TMyGix::New(Nm, FName, faRdOnly, cache_size, split_len);
			TAssert(gix->GetKeys() == keys, "Invalid key count");
			printf("== %d %d\n", gix->GetKeys(), keys);
			CheckCounts(counts, *gix);

			for (int i = 0; i < 100; i++) {
				auto key = TIntUInt64Pr(i, i);
				if (!gix->IsKey(key))
					continue;
				auto itemset = gix->GetItemSet(key);
				//printf("//// %d %d \n", i, itemset->GetItems());

				TVec<TMyItem> v;
				itemset->GetItemV(v);
				//printf("//// %d %d \n", i, v.Len());
				TAssert(itemset->GetItems() == v.Len(), "Invalid itemset len");
			}
		}
	}

	void PerformTests() {

		/*Test_Simple_1();
		Test_Simple_220();
		Test_Simple_220_Unsorted();
		Test_Merge_220_Into_50();
		Test_Merge_220_Into_120();
		Test_Merge_22000_Into_50();*/

		Test_BigInserts();

		//Test_Delete_1();
		//Test_Delete_20();
		//Test_Delete_20And1();
		//Test_Delete_120();
		//Test_Delete_120And1();
		//Test_Delete_120And110();
		//Test_Delete_22000And1000();

		//Test_QuasiDelete_120And1And2();
		//Test_QuasiDelete_120And20();
		//Test_QuasiDelete_22000And1000();

		//// this will split only big itemsets
		//WarnNotifyI(TStr("Split only big itemsets\n"));
		//Test_Feed(50 * 1024 * 1025, 1000);

		//// this will split probably all itemsets
		//WarnNotifyI(TStr("Split all itemsets\n"));
		//Test_Feed(50 * 1024 * 1025, 100);

		////// this will split probably all itemsets
		////// it will also limit cache to less than 10% of the itemsets
		////WarnNotifyI(TStr("Split all itemsets, small cache\n"));
		////Test_Feed(5 * 1024 * 1025, 1000);
	}
};

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	XTest test;
	test.PerformTests();

	return 0;
}


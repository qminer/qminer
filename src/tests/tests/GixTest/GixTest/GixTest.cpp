#define GIX_DEBUG
#define GIX_TEST

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <base.h>
#include <mine.h>

#include <iostream>
#include <windows.h>

#include "gtest/gtest.h"

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


/////////////////////////////////////////////////////////////////////////
//// for nice outputs
//
//HANDLE  hConsole;
//
//void WarnNotifyI(TStr& const s) {
//	SetConsoleTextAttribute(hConsole, 11);
//	WarnNotify(s);
//	SetConsoleTextAttribute(hConsole, 15);
//}
//void WarnNotifyW(TStr& const s) {
//	SetConsoleTextAttribute(hConsole, 14);
//	WarnNotify(s);
//	SetConsoleTextAttribute(hConsole, 15);
//}
//
//#define ASSERT_TRUE(Cond, MsgCStr) \
//  ((Cond) ? static_cast<void>(0) : WarnNotifyW( TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) +": "+ MsgCStr))


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
	if (ItemV.Empty()) { return; } // nothing to do in this case
	if (!ItemV.IsSorted()) { ItemV.Sort(); } // sort if not yet sorted

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
}

////////////////////////////////////////////////////////////////////////

class XTest {
public:

	static void Test_Simple_1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, TMyItem(7234, 1));

		EXPECT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);
		//ASSERT_TRUE(gix.GetCacheSize() == 0);

		auto itemset = gix.GetItemSet(x);
		EXPECT_TRUE(itemset->GetKey() == x);
		EXPECT_TRUE(!itemset->IsFull());
		EXPECT_TRUE(itemset->MergedP);
		EXPECT_TRUE(itemset->TotalCnt == 1);
	}

	static void Test_Simple_220() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}

		EXPECT_TRUE(!gix.IsCacheFull());
		EXPECT_TRUE(gix.KeyIdH.Len() == 1);
		//ASSERT_TRUE(gix.GetCacheSize() == 1);
		auto itemset = gix.GetItemSet(x);
		EXPECT_TRUE(itemset->GetKey() == x);
		EXPECT_TRUE(!itemset->IsFull());
		EXPECT_TRUE(itemset->MergedP);
		EXPECT_TRUE(itemset->TotalCnt == 220);
		EXPECT_TRUE(itemset->Children.Len() == 2);
		EXPECT_TRUE(itemset->ItemV.Len() == 20);
		EXPECT_TRUE(itemset->Children[0].Len == 100);
		EXPECT_TRUE(itemset->Children[1].Len == 100);
		EXPECT_TRUE(itemset->ChildrenData[0].Len() == 0);
		EXPECT_TRUE(itemset->ChildrenData[1].Len() == 0);

		gix.ItemSetCache.FlushAndClr();
		//ASSERT_TRUE(gix.GetCacheSize() == 0, "Cache should contain 0 items");
	}

	static void Test_Simple_220_Unsorted() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			int item = (i * 15485867) % 16381;
			gix.AddItem(x, TMyItem(item, 1)); // pseudo-random numbers
		}

		EXPECT_TRUE(!gix.IsCacheFull());
		EXPECT_TRUE(gix.KeyIdH.Len() == 1);
		//ASSERT_TRUE(gix.GetCacheSize() == 1);
		auto itemset = gix.GetItemSet(x);
		EXPECT_TRUE(itemset->GetKey() == x);
		EXPECT_TRUE(!itemset->IsFull());
		EXPECT_TRUE(!itemset->MergedP);
		EXPECT_TRUE(itemset->TotalCnt == 220);
		EXPECT_TRUE(itemset->Children.Len() == 2);
		EXPECT_TRUE(itemset->ItemV.Len() == 20);
		EXPECT_TRUE(itemset->Children[0].Len == 100);
		EXPECT_TRUE(itemset->Children[1].Len == 100);
		EXPECT_TRUE(itemset->ChildrenData[0].Len() == 100);
		EXPECT_TRUE(itemset->ChildrenData[1].Len() == 0);

		itemset->Def();

		EXPECT_TRUE(itemset->MergedP);
		EXPECT_TRUE(itemset->Children.Len() == 2);
		EXPECT_TRUE(itemset->ItemV.Len() == 20);
		EXPECT_TRUE(itemset->Children[0].Len == 100);
		EXPECT_TRUE(itemset->Children[1].Len == 100);
		EXPECT_TRUE(itemset->ChildrenData[0].Len() == 100);
		EXPECT_TRUE(itemset->ChildrenData[1].Len() == 100);

	}

	static void Test_Merge_220_Into_50() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i % 50, 1));
		}

		EXPECT_TRUE(!gix.IsCacheFull());
		EXPECT_TRUE(gix.KeyIdH.Len() == 1);
		//ASSERT_TRUE(gix.GetCacheSize() == 1);
		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->Children.Len() == 0);
		// 50 merged + 20 unmerged
		ASSERT_TRUE(itemset->ItemV.Len() == 70);

		itemset->Def();

		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->Children.Len() == 0);
		ASSERT_TRUE(itemset->ItemV.Len() == 50);

	}

	static void Test_Merge_220_Into_120() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, TMyItem(i % 120, 1));
		}

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);
		//ASSERT_TRUE(gix.GetCacheSize() == 1);
		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->Children.Len() == 1);
		// 20 merged + 20 unmerged
		ASSERT_TRUE(itemset->ItemV.Len() == 40);
		ASSERT_TRUE(itemset->Children[0].Len == 100);
		ASSERT_TRUE(itemset->ChildrenData[0].Len() == 100);

		itemset->Def();

		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->Children.Len() == 1);
		ASSERT_TRUE(itemset->ItemV.Len() == 20);
		ASSERT_TRUE(itemset->Children[0].Len == 100);
		ASSERT_TRUE(itemset->ChildrenData[0].Len() == 100);

	}

	static void Test_Merge_22000_Into_50() {
		TMyGix gix("Test1", "data", faCreate, 10000000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 22000; i++) {
			gix.AddItem(x, TMyItem(i % 50, 1));
		}

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);
		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 100);
		ASSERT_TRUE(itemset->Children.Len() == 0);
		ASSERT_TRUE(itemset->ItemV.Len() == 100);

		itemset->Def();

		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 50);
		ASSERT_TRUE(itemset->Children.Len() == 0);
		ASSERT_TRUE(itemset->ItemV.Len() == 50);

	}

	static void AddToCounter(THash<TInt, TInt>& counter, TInt i) {
		if (!counter.IsKey(i)) {
			counter.AddKey(i);
		}
		counter.GetDat(i)++;
	}

	static void CheckCounts(THash<TInt, TInt>& counts, TMyGix& gix) {
		printf("Checking counts...\n");
		for (auto &key : counts) {
			auto itemset = gix.GetItemSet(TIntUInt64Pr(key.Key, (int)key.Key));
			auto cnt = key.Dat;
			ASSERT_TRUE(itemset->MergedP);
			ASSERT_TRUE(itemset->TotalCnt == cnt);
		}
		printf("Checking counts done.\n");
	}
	
	static void OverwriteCounts(THash<TInt, TInt>& counts, TMyGix& gix) {
		printf("Overwritting counts...\n");
		for (auto &key : counts) {
			auto itemset = gix.GetItemSet(TIntUInt64Pr(key.Key, (int)key.Key));
			key.Dat = itemset->TotalCnt;
		}
		printf("Overwritting counts done.\n");
	}

	static void Test_Feed(int cache_size = 50 * 1024 * 1024, int split_len = 100) {
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
			ASSERT_TRUE(itemset->IsFull());
			ASSERT_TRUE(itemset->MergedP);
			ASSERT_TRUE(itemset->TotalCnt == total);

			CheckCounts(counts, gix);

			gix.Flush();

			//gix.AddItem(TIntUInt64Pr(1, 1), total + 1);
			//gix.Flush();

			keys = gix.GetKeys();
			itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
			ASSERT_TRUE(itemset->IsFull());
			ASSERT_TRUE(itemset->MergedP);
			ASSERT_TRUE(itemset->TotalCnt == total);

			OverwriteCounts(counts, gix); // itemsets could be merged
		}
		{
			// reload data - in read-only mode
			TMyGix gix(Nm, FName, faRdOnly, 50 * 1024 * 1024);

			ASSERT_TRUE(gix.GetKeys() == keys);

			CheckCounts(counts, gix);
		}
	}

	static void Test_RandomGenerateRead(int cache_size = 10 * 1024 * 1024, int split_len = 100) {
		TStr Nm("Test_Feed");
		TStr FName("data");
		int loops = 1000 * 1000;
		int total_words = 10000;
		int keys = 0;


		// simmulate news feed
		// many articles, containing 50 random words + everyone containing words 1-5
		TMyGix gix(Nm, FName, faCreate, cache_size, split_len);
		TRnd rnd(1);
		int doc_counter = 0;

		gix.PrintStats();
		gix.AddItem(TMyKey(5, 5), TMyItem(9000000, 1));
		gix.PrintStats();

		for (int i = 0; i < loops; i++) {
			int r = rnd.GetUniDevInt(100);
			if (i % 100 == 0) printf("==================== %d\n", i);
			if (r < 10) {
				// every doc containes the same 5 words
				for (int j = 1; j <= 5; j++) {
					gix.AddItem(TIntUInt64Pr(j, j), TMyItem(doc_counter, 1));
				}
				// each document contains 50 random words
				TVec<int> vals;
				for (int j = 0; j < 50; j++) {
					int w = rnd.GetUniDevInt(10, total_words);

					// prevent the same word for the same document
					while (vals.IsIn(w)) {
						w = rnd.GetUniDevInt(10, total_words);
					}
					vals.Add(w);

					auto key = TMyKey(w, w);
					gix.AddItem(key, TMyItem(doc_counter, 1));
				}
				doc_counter++;
			} else {
				// perform search in gix
				int w = rnd.GetUniDevInt(0, total_words);
				auto key = TMyKey(w, w);
				if (gix.IsKey(key))
					gix.GetItemSet(key);
			}
			if (i % 10000 == 0) {
				gix.PrintStats();
				gix.PartialFlush(100);
				gix.PrintStats();
			}
		}
	}

	static void Test_SizeTest(int cache_size = 1 * 1024 * 1024, int split_len = 1000) {
		TStr Nm("Test_Feed");
		TStr FName("data");
		int loops = 200*1000;
		int total_words = 20000;
		int article_max_len = 20;
		int keys = 0;
		cache_size *= 10;
        printf("***** size=%d\n", sizeof(TMyItem));
		{
			TMyGix gix(Nm, FName, faCreate, cache_size, split_len);
			gix.PrintStats();

			TRnd rnd(1);
			int doc_counter = 0;
			for (int i = 0; i < loops; i++) {
				//// every doc contains the same word(s)
				//for (int j = 1; j <= article_max_len; j++) {
				//	gix.AddItem(TIntUInt64Pr(j, j), TMyItem(doc_counter, 1));
				//}
				
				// pick random words
				int r = rnd.GetUniDevInt(article_max_len);
				for (int j = 1; j <= r; j++) {
					int k = rnd.GetUniDevInt(total_words);
					gix.AddItem(TIntUInt64Pr(k, k), TMyItem(doc_counter, 1));
				}

				//// each document contains single, unique word => itemset length = 1
				//gix.AddItem(TIntUInt64Pr(doc_counter, doc_counter), TMyItem(doc_counter, 1));

				doc_counter++;
				if (i % 10000 == 0) {
					gix.PrintStats();
				}
			}
			gix.PrintStats();
			gix.PartialFlush(100 * 1000);
			gix.PrintStats();

			std::cout << "------- ";
			std::cout << "Before disposing hash - press key to continue: ";
			getchar();

			gix.KillHash();
            gix.PrintStats();

			std::cout << "------- ";
			std::cout << "Before disposing cache - press key to continue: ";
			getchar();

			gix.KillCache();
			gix.PrintStats();

			std::cout << "------- ";
			std::cout << "Done - press key to continue: ";
			getchar();
			//_ASSERTE(_CrtCheckMemory());
			//_CrtDumpMemoryLeaks();
		}
	}

	static void Test_Delete_1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, TMyItem(7234, 1));
		gix.DelItem(x, TMyItem(7234, 1));

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 2);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 1);
		ASSERT_TRUE(itemset->ItemVDel[0] == 1);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 0);
	}

	static void Test_Delete_20() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(321, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 41);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 1);
		ASSERT_TRUE(itemset->ItemVDel[0] == 40);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 1);
	}

	static void Test_Delete_20And1() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(321, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));
		gix.AddItem(x, TMyItem(7234, 1));

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 42);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 1);
		ASSERT_TRUE(itemset->ItemVDel[0] == 40);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 2);
	}

	static void Test_Delete_120() {
		TMyGix gix("Test1", "data", faCreate, 100000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(i, 1));
		}
		gix.DelItem(x, TMyItem(7234, 120));

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 143);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 1);
		ASSERT_TRUE(itemset->ItemVDel[0] == 42);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 120);
	}

	static void Test_Delete_120And1() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(7234, 1));
			gix.AddItem(x, TMyItem(i, 1));
		}
		gix.DelItem(x, TMyItem(7234, 1));
		gix.AddItem(x, TMyItem(7234, 1));

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 144);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 1);
		ASSERT_TRUE(itemset->ItemVDel[0] == 42);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 121);
	}

	static void Test_Delete_120And110() {
		TMyGix gix("Test1", "data", faCreate, 10000, 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, TMyItem(i, 1));
		}
		for (int i = 0; i < 110; i++) {
			gix.DelItem(x, TMyItem(i, 1));
		}

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 20 + 20 + 30);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 30);
		ASSERT_TRUE(itemset->ItemVDel[0] == 20 + 20);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == 10);
	}

	static void Test_Delete_22000And1000() {
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

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - to_delete + 80 + 80);
		ASSERT_TRUE(itemset->ItemVDel.Len() == to_delete % 100);
		ASSERT_TRUE(itemset->ItemVDel[0] == 0);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - to_delete);
		for (int i = to_delete; i < all; i++) {
			ASSERT_TRUE(itemset->GetItem(i - to_delete).Key == i);
		}
	}

	static void Test_QuasiDelete_120And1And2() {
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

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all + 3);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 0);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - 1);
		ASSERT_TRUE(itemset->ItemV.Len() == all - 100);
		ASSERT_TRUE(itemset->Children[0].Len == 99);
		for (int i = 0; i < all; i++) {
			if (i == to_delete) {
				continue;
			} else if (i < to_delete) {
				ASSERT_TRUE(itemset->GetItem(i).Key == i);
			} else if (i > to_delete) {
				ASSERT_TRUE(itemset->GetItem(i - 1).Key == i);
			}
		}
	}

	static void Test_QuasiDelete_120And20() {
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

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all + to_delete);
		ASSERT_TRUE(itemset->ItemVDel.Len() == 0);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - to_delete);
		for (int i = to_delete; i < all; i++) {
			ASSERT_TRUE(itemset->GetItem(i - to_delete).Key == i);
		}
	}

	static void Test_QuasiDelete_22000And1000() {
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

		ASSERT_TRUE(!gix.IsCacheFull());
		ASSERT_TRUE(gix.KeyIdH.Len() == 1);

		auto itemset = gix.GetItemSet(x);
		ASSERT_TRUE(itemset->GetKey() == x);
		ASSERT_TRUE(!itemset->IsFull());
		ASSERT_TRUE(!itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - to_delete + 2 * (to_delete % 100));
		ASSERT_TRUE(itemset->ItemVDel.Len() == 0);

		itemset->Def();

		ASSERT_TRUE(itemset->MergedP);
		ASSERT_TRUE(itemset->TotalCnt == all - to_delete);
		for (int i = to_delete; i < all; i++) {
			ASSERT_TRUE(itemset->GetItem(i - to_delete).Key == i);
		}
	}

	static void Test_BigInserts(int cache_size = 500 * 1024 * 1024, int split_len = 1000) {
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
			ASSERT_TRUE(itemset->IsFull());
			ASSERT_TRUE(itemset->MergedP);
			ASSERT_TRUE(itemset->TotalCnt == total);

			CheckCounts(counts, *gix);

			keys = gix->GetKeys();
			gix->PrintStats();
			gix->Flush();
		}
		{
			// reload data - in read-only mode
			auto gix = TMyGix::New(Nm, FName, faRdOnly, cache_size, split_len);
			ASSERT_TRUE(gix->GetKeys() == keys);
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
				ASSERT_TRUE(itemset->GetItems() == v.Len());
			}
		}
	}

	void PerformBigTests() {		

		Test_BigInserts();
		Test_RandomGenerateRead();
		Test_SizeTest();

		// this will split only big itemsets
		Test_Feed(50 * 1024 * 1025, 1000);

		// this will split probably all itemsets
		Test_Feed(50 * 1024 * 1025, 100);

		//// this will split probably all itemsets
		//// it will also limit cache to less than 10% of the itemsets
		//WarnNotifyI(TStr("Split all itemsets, small cache\n"));
		//Test_Feed(5 * 1024 * 1025, 1000);
	}
};

//////////////////////////////////////////////////////////////////////////
// Unit tests to be executed
//////////////////////////////////////////////////////////////////////////

TEST(testTBlobBs, Simple10) { XTest::Test_Simple_1(); }
TEST(testTBlobBs, Simple200) { XTest::Test_Simple_220(); }
TEST(testTBlobBs, Simple220Unsorted) { XTest::Test_Simple_220_Unsorted(); }
TEST(testTBlobBs, Merge220Into50) { XTest::Test_Merge_220_Into_50(); }
TEST(testTBlobBs, Merge220Into120) { XTest::Test_Merge_220_Into_120(); }
TEST(testTBlobBs, Merge22000Into50) { XTest::Test_Merge_22000_Into_50(); }
TEST(testTBlobBs, Delete1) { XTest::Test_Delete_1(); }
TEST(testTBlobBs, Delete20) { XTest::Test_Delete_20(); }
TEST(testTBlobBs, Delete20And1) { XTest::Test_Delete_20And1(); }
TEST(testTBlobBs, Delete120) { XTest::Test_Delete_120(); }
TEST(testTBlobBs, Delete120And1) { XTest::Test_Delete_120And1(); }
TEST(testTBlobBs, Delete120And110) { XTest::Test_Delete_120And110(); }
TEST(testTBlobBs, Delete22000And1000) { XTest::Test_Delete_22000And1000(); }
TEST(testTBlobBs, QuasiDelete120And1And2) { XTest::Test_QuasiDelete_120And1And2(); }
TEST(testTBlobBs, QuasiDelete120And20) { XTest::Test_QuasiDelete_120And20(); }
TEST(testTBlobBs, QuasiDelete22000And1000) { XTest::Test_QuasiDelete_22000And1000(); }

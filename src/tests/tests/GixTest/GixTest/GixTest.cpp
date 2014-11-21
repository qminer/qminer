#define GIX_DEBUG
#define GIX_TEST

#include <base.h>
#include <mine.h>

#include <iostream>
#include <windows.h>

////////////////////////////////////////////////////////////////////////
// typedefs

typedef  TGixDefMerger < TIntUInt64Pr, TUInt64 > TMyMerger;
typedef TPt<TGixMerger<TIntUInt64Pr, TUInt64> > PGixMerger;

typedef  TGixItemSet < TIntUInt64Pr, TUInt64 > TMyItemSet;
typedef  TPt < TMyItemSet > PMyItemSet;

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

////////////////////////////////////////////////////////////////////////

class XTest {
public:

	void Test_Simple_1() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, i);
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			int item = (i * 15485867) % 16381;
			gix.AddItem(x, item); // pseudo-random numbers
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 100000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, i % 50);
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 100000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, i % 120);
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 22000; i++) {
			gix.AddItem(x, i % 50);
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

	void CheckCounts(THash<TInt, TInt>& counts, TGix<TIntUInt64Pr, TUInt64>& gix) {
		printf("Checking counts...\n");
		for (auto &key : counts) {
			auto itemset = gix.GetItemSet(TIntUInt64Pr(key.Key, (int)key.Key));
			auto cnt = key.Dat;
			TAssert(itemset->MergedP, "Itemset should be merged");
			TAssert(itemset->TotalCnt == cnt, TStr::Fmt("Invalid itemset TotalCnt: key=%d, expected=%d, actual=%d", key.Key, cnt, itemset->TotalCnt));
		}
		printf("Checking counts done.\n");
	}
	void OverwriteCounts(THash<TInt, TInt>& counts, TGix<TIntUInt64Pr, TUInt64>& gix) {
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
			TGix<TIntUInt64Pr, TUInt64> gix(Nm, FName, faCreate, cache_size, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), split_len);
			TRnd rnd(1);
			for (int j = 0; j < total; j++) {
				// every doc containes the same 5 words
				for (int i = 1; i <= 5; i++) {
					gix.AddItem(TIntUInt64Pr(i, i), j);
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

					auto key = TIntUInt64Pr(w, w);
					gix.AddItem(key, j);
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
			TGix<TIntUInt64Pr, TUInt64> gix(Nm, FName, faRdOnly, 50 * 1024 * 1024);

			TAssert(gix.GetKeys() == keys, "Invalid key count");

			CheckCounts(counts, gix);
		}
	}

	void Test_Delete_1() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, 7234);
		gix.DelItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, 7234);
			gix.AddItem(x, 321);
		}
		gix.DelItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 20; i++) {
			gix.AddItem(x, 7234);
			gix.AddItem(x, 321);
		}
		gix.DelItem(x, 7234);
		gix.AddItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, 7234);
			gix.AddItem(x, i);
		}
		gix.DelItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, 7234);
			gix.AddItem(x, i);
		}
		gix.DelItem(x, 7234);
		gix.AddItem(x, 7234);

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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 120; i++) {
			gix.AddItem(x, i);
		}
		for (int i = 0; i < 110; i++) {
			gix.DelItem(x, i);
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 1000000, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), 100);
		int xx = 122;
		int all = 22000;
		int to_delete = 980;
		TIntUInt64Pr x(xx, xx);
		// fill all
		for (int i = 0; i < all; i++) {
			gix.AddItem(x, i);
		}
		// now delete data from the front
		for (int i = 0; i < to_delete; i++) {
			gix.DelItem(x, i);
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");

		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(!itemset->MergedP, "Itemset should NOT be merged");
		TAssert(itemset->TotalCnt == all - to_delete + 80+ 80, "Invalid itemset TotalCnt");
		TAssert(itemset->ItemVDel.Len() == to_delete % 100, "Invalid list of deletes");
		TAssert(itemset->ItemVDel[0] == 0, "Invalid list of deletes - invalid index");
		
		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == all - to_delete, "Invalid itemset TotalCnt");
		for (int i = to_delete; i < all; i++) {
			TAssert(itemset->GetItem(i - to_delete) == i, "Invalid item at specific index");
		}
	}

	void Test_BigInserts(int cache_size = 500 * 1024 * 1024, int split_len = 2000) {
		TStr Nm("Test_Feed_Big");
		TStr FName("data");
		int total = 1000 * 1000;
		int keys = 0;
		int voc_count = 50 * 1000; // number of possible words

		THash<TInt, TInt> counts;
		{
			// simmulate news feed
			// many articles, containing 50 random words + everyone containing words 1-5
			TGix<TIntUInt64Pr, TUInt64> gix(Nm, FName, faCreate, cache_size, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), split_len);
			TRnd rnd(1);
			for (int j = 0; j < total; j++) {
				// every doc contains the same 5 words
				for (int i = 1; i <= 5; i++) {
					gix.AddItem(TIntUInt64Pr(i, i), j);
				}
				// each document contains 100 random words
				TVec<int> vals;
				for (int i = 0; i < 100; i++) {
					int w = rnd.GetUniDevInt(10, voc_count);

					// prevent the same word for the same document
					while (vals.IsIn(w)) {
						w = rnd.GetUniDevInt(10, voc_count);
					}
					vals.Add(w);

					auto key = TIntUInt64Pr(w, w);
					gix.AddItem(key, j);
					AddToCounter(counts, w);
				}
				if (j % 10000 == 0) {
					printf("-- %d\n", j);
				}
			}

			auto itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
			TAssert(itemset->IsFull(), "Itemset should be full");
			TAssert(itemset->MergedP, "Itemset should remain merged");
			TAssert(itemset->TotalCnt == total, "Invalid itemset TotalCnt");

			CheckCounts(counts, gix);

			keys = gix.GetKeys();

			gix.Flush();
		}
		{
			// reload data - in read-only mode
			TGix<TIntUInt64Pr, TUInt64> gix(Nm, FName, faRdOnly, cache_size, TGixDefMerger<TIntUInt64Pr, TUInt64>::New(), split_len);
			TAssert(gix.GetKeys() == keys, "Invalid key count");
			printf("== %d %d\n", gix.GetKeys(), keys);
			CheckCounts(counts, gix);
		}
	}

	void PerformTests() {



		//Test_Simple_1();
		//Test_Simple_220();
		//Test_Simple_220_Unsorted();
		//Test_Merge_220_Into_50();
		//Test_Merge_220_Into_120();
		//Test_Merge_22000_Into_50();

		Test_BigInserts();

		//Test_Delete_1();
		//Test_Delete_20();
		//Test_Delete_20And1();
		//Test_Delete_120();
		//Test_Delete_120And1();
		//Test_Delete_120And110();
		//Test_Delete_22000And1000();

		//// this will split only big itemsets
		//WarnNotifyI(TStr("Split only big itemsets\n"));
		//Test_Feed(50*1024*1025, 1000);

		//// this will split probably all itemsets
		//WarnNotifyI(TStr("Split all itemsets\n"));
		//Test_Feed(50 * 1024 * 1025, 100);

		//// this will split probably all itemsets
		//// it will also limit cache to less than 10% of the itemsets
		//WarnNotifyI(TStr("Split all itemsets, small cache\n"));
		//Test_Feed(5 * 1024 * 1025, 1000);
	}
};

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	XTest test;
	test.PerformTests();

	return 0;
}


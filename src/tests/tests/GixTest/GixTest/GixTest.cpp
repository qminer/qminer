#define GIX_DEBUG

#include <base.h>
#include <mine.h>

////////////////////////////////////////////////////////////////////////
// typedefs

typedef  TGixDefMerger < TIntUInt64Pr, TUInt64 > TMyMerger;
typedef TPt<TGixMerger<TIntUInt64Pr, TUInt64> > PGixMerger;

typedef  TGixStorageLayer < TIntUInt64Pr, TUInt64 > TMyStorageLayer;

typedef  TGixItemSet < TIntUInt64Pr, TUInt64 > TMyItemSet;
typedef  TPt < TMyItemSet > PMyItemSet;

#define TAssert(Cond, MsgCStr) \
  ((Cond) ? static_cast<void>(0) : WarnNotify( TStr(__FILE__) + " line " + TInt::GetStr(__LINE__) +": "+ MsgCStr))
/*
////////////////////////////////////////////////////////////////////////

void Print(TMyStorageLayer* gixsl, TVec<TBlobPt>* blob_pt_v) {
	printf("\n\r\n\r");
	for (int i = 0; i < blob_pt_v->Len(); i++) {
		PMyItemSet itemset = gixsl->GetItemSet(blob_pt_v->operator[](i));
		const int Items = itemset->GetItems();
		const int MemUsed = itemset->GetMemUsed();
		const TIntUInt64Pr key = itemset->GetKey();
		printf("%d %d - %d %d\n\r", key.Val1, key.Val2, Items, MemUsed);
	}
	printf("********************\n\r\n\r");
}

void TestStorageLayer1() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 390, merger);
		TVec<TBlobPt> blob_pt_v;
		for (int i = 0; i < 10; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < 5; j++) {
				itemset->AddItem(i + j);
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
			//gixsl.StoreItemSet(blob_pt);
		}
		gixsl.RefreshMemUsed();

		Print(&gixsl, &blob_pt_v);
		gixsl.PrintCacheKeys(); // should be 9, 8, 7, 6

		// randomly access some items and check the output - what is in cache
		gixsl.GetItemSet(blob_pt_v[4]);
		gixsl.GetItemSet(blob_pt_v[2]);
		gixsl.GetItemSet(blob_pt_v[3]);
		gixsl.GetItemSet(blob_pt_v[4]);

		gixsl.PrintCacheKeys(); // should be 4, 3, 2, 9

		// store data into file
		TFOut FOut(FNameGix);
		blob_pt_v.Save(FOut);
	}
	{
		// ok, now read the data out
		TVec<TBlobPt> blob_pt_v;
		TFIn FIn(FNameGix);
		blob_pt_v.Load(FIn);

		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faRdOnly, 200, merger);

		Print(&gixsl, &blob_pt_v);
		gixsl.PrintCacheKeys(); // should be 9, 8

		// randomly access some items and check the output - what is in cache
		gixsl.GetItemSet(blob_pt_v[4]);
		gixsl.GetItemSet(blob_pt_v[3]);
		gixsl.GetItemSet(blob_pt_v[2]);
		gixsl.GetItemSet(blob_pt_v[4]);

		gixsl.PrintCacheKeys(); // should be 4, 3
	}
}

////////////////////////////////////////////////

void TestSplitting() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;
		for (int i = 0; i < 3; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < 100; j++) {
				itemset->AddItem((j * 16103) % 16381);
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();

		Print(&gixsl, &blob_pt_v);
		gixsl.PrintCacheKeys(); // should be 5,4,3,2

		// randomly access some items and check the output - what is in cache
		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		printf("%d \n", itemset->GetItems());
		printf("%d \n", itemset->GetItem(3));
		itemset->GetItemV(items);


		//gixsl.GetItemSet(blob_pt_v[2]);
		//gixsl.GetItemSet(blob_pt_v[3]);
		//gixsl.GetItemSet(blob_pt_v[4]);

		gixsl.PrintCacheKeys(); // should be 4, 3, 2, 6

		// store data into file
		TFOut FOut(FNameGix);
		blob_pt_v.Save(FOut);
	}
}

////////////////////////////////////////////////////////////////////////

void Test_Splitting_NoSplit() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 8 unique items, not enough for split
		int len = 8;
		int cnt = 1;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j); // increasing, unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == len, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < len; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}

void Test_Splitting_1Split() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 12 unique items, enough for 1 split
		int len = 12;
		int cnt = 1;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j); // increasing, unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == len, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < len; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}

void Test_Splitting_1SplitNoRealMerge() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 15 items (12 unique), enough for 1 split
		// split should remain after merge
		int len = 15;
		int cnt = 1;
		int diff_vals = 12;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j % diff_vals); // non-unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset); // here the merge occurs
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == diff_vals, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < diff_vals; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}

void Test_Splitting_1SplitMerge() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 12 non-unique items, enough for 1 split
		// but when merged, falls bellow 1 vector => children deletion must take place
		int len = 12;
		int cnt = 1;
		int diff_vals = 4;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j % diff_vals); // non-unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == diff_vals, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < diff_vals; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}

void Test_Splitting_2SplitMerge() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 25 non-unique items, enough for 2 splits
		// but when merged, they fit into 2 vectors => children deletion must take place
		int len = 25;
		int cnt = 1;
		int diff_vals = 15;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j % diff_vals); // non-unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == diff_vals, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < diff_vals; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}

void Test_Splitting_10SplitsMerge() {
	TStr FNameBlob("data\\Test1");
	TStr FNameGix("data\\Test1.gix");
	{
		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faCreate, 500, merger);
		TVec<TBlobPt> blob_pt_v;

		// we have 1 itemset, containing 97 non-unique items, enough for 9 split
		// but when merged, fits inside 2 vectors => children deletion must take place

		// TODO there is a bug in the code - when child itemset is saved, storage layer 
		// returns updated blob pointer and that change is not recorded anywhere...!!!

		int len = 10*10 - 3;
		int cnt = 1;
		int diff_vals = 15;
		for (int i = 0; i < cnt; i++) {
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < len; j++) {
				itemset->AddItem(j % diff_vals); // non-unique
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
		}
		gixsl.RefreshMemUsed();
		TAssert(blob_pt_v.Len() == cnt, "Bad itemset count.");

		TVec<TUInt64> items;
		auto itemset = gixsl.GetItemSet(blob_pt_v[0]);
		itemset->GetItemV(items);
		TAssert(items.Len() == diff_vals, "Bad item count.");
		for (int j = 0; j < items.Len(); j++) {
			TAssert(items[j] == j, "Bad item value.");
		}
		for (int j = 0; j < diff_vals; j++) {
			TAssert(itemset->GetItem(j) == j, "Bad item value.");
		}
	}
}
*/
////////////////////////////////////////////////////////////////////////

void SimpleGixTest() {
	TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);
	for (int i = 0; i < 100000; i++) {
		TIntUInt64Pr x(i, i);
		gix.AddItem(x, i * 5 % 16);
		if (i % 5000 == 1) {
			printf("Evo: %d\n", i);
		}
	}
}

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
	SimpleGixTest();
	//TestStorageLayer1();
	//TestSplitting();
	
	// "unit-tests"
	//Test_Splitting_NoSplit();
	//Test_Splitting_1Split();
	//Test_Splitting_1SplitNoRealMerge();
	//Test_Splitting_1SplitMerge();
	//Test_Splitting_2SplitMerge();
	
	//Test_Splitting_10SplitsMerge();

	return 0;
}


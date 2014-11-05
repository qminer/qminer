#define GIX_DEBUG
#define GIX_TEST

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

////////////////////////////////////////////////////////////////////////

int TGixItemSet<TIntUInt64Pr, TUInt64>::len_to_split;

class XTest {
public:

	void Test_Simple_1() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);
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
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);
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
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->TotalCnt == 220, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenLen[1] == 20, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 20, "Itemset content - invalid second child length");
		
		gix.ItemSetCache.FlushAndClr();
		//TAssert(gix.GetCacheSize() == 0, "Cache should contain 0 items");
	}

	void Test_Simple_220_Unsorted() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, (i * 15485867) % 16381); // pseudo-random numbers
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should not be merged");
		TAssert(itemset->TotalCnt == 220, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenLen[1] == 20, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 20, "Itemset content - invalid second child length");

		itemset->Def();

		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenLen[1] == 20, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 20, "Itemset content - invalid second child length");

	}

	void Test_Merge_220_Into_50() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 100000);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 220; i++) {
			gix.AddItem(x, i % 50);
		}
		
		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		//itemset->Print();
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenLen[1] == 20, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 20, "Itemset content - invalid second child length");

		itemset->Def();

		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 0 children");
		TAssert(itemset->ItemV.Len() == 50, "Itemset content - invalid ItemV length");

	}

	void Test_Merge_220_Into_120() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 100000);
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
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->Children.Len() == 2, "Itemset should have 2 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenLen[1] == 20, "Itemset content - invalid second child length");
		TAssert(itemset->ChildrenData[0].Len() == 100, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[1].Len() == 20, "Itemset content - invalid second child length");

		itemset->Def();

		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->Children.Len() == 1, "Itemset should have 1 child");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		TAssert(itemset->ChildrenLen[0] == 20, "Itemset content - invalid first child length");
		TAssert(itemset->ChildrenData[0].Len() == 20, "Itemset content - invalid first child length");

	}

	void Test_Merge_22000_Into_50() {
		TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 100000);
		int i = 122;
		TIntUInt64Pr x(i, i);
		for (int i = 0; i < 22000; i++) {
			gix.AddItem(x, i % 50);
		}

		TAssert(!gix.IsCacheFull(), "Cache cannot be full");
		TAssert(gix.KeyIdH.Len() == 1, "Mapping should contain 1 item");
		//TAssert(gix.GetCacheSize() == 1, "Cache should contain 1 item");
		auto itemset = gix.GetItemSet(x);
		TAssert(itemset->GetKey() == x, "Invalid itemset key");
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(!itemset->MergedP, "Itemset should not be merged");
		TAssert(itemset->TotalCnt == 22000, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 219, "Itemset should have 219 children");
		TAssert(itemset->ItemV.Len() == 100, "Itemset content - invalid ItemV length");
		
		itemset->Def();

		TAssert(!itemset->IsFull(), "Itemset should NOT be full");
		TAssert(itemset->MergedP, "Itemset should be merged");
		TAssert(itemset->TotalCnt == 50, "Invalid itemset TotalCnt");
		TAssert(itemset->Children.Len() == 0, "Itemset should have 0 children");
		TAssert(itemset->ItemV.Len() == 50, "Itemset content - invalid ItemV length");

	}

	void Test_Feed() {
		// simmulate news feed
		// many articles, containing 50 random words + everyone containing words 1-5
		TGix<TIntUInt64Pr, TUInt64> gix("Test_Feed", "data", faCreate, 50 * 1024 * 1024);
		TRnd rnd(1);
		int total = 30000;
		for (int j = 0; j < total; j++) {
			// every doc containes the same 5 words
			for (int i = 1; i <= 5; i++) {
				gix.AddItem(TIntUInt64Pr(i, i), j);
			}
			// each document contains 50 random words
			for (int i = 0; i < 50; i++) {
				int w = rnd.GetUniDevInt(10, 10000);
				gix.AddItem(TIntUInt64Pr(w, w), j);
			}
		}

		auto itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->TotalCnt == total, "Invalid itemset TotalCnt");

		gix.Flush();

		itemset = gix.GetItemSet(TIntUInt64Pr(1, 1));
		TAssert(itemset->IsFull(), "Itemset should be full");
		TAssert(itemset->MergedP, "Itemset should remain merged");
		TAssert(itemset->TotalCnt == total, "Invalid itemset TotalCnt");
	}

	void PerformTests() {
		TGixItemSet<TIntUInt64Pr, TUInt64>::len_to_split = 100;
		Test_Simple_1();
		Test_Simple_220();
		Test_Simple_220_Unsorted();
		Test_Merge_220_Into_50();
		Test_Merge_220_Into_120();
		Test_Merge_22000_Into_50();
		
		TGixItemSet<TIntUInt64Pr, TUInt64>::len_to_split = 1000;
		Test_Feed();
	}
};

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	XTest test;
	test.PerformTests();

	return 0;
}


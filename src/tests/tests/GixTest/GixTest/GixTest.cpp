#include <base.h>
#include <mine.h>

////////////////////////////////////////////////////////////////////////
// typedefs

typedef  TGixDefMerger < TIntUInt64Pr, TUInt64 > TMyMerger;
typedef TPt<TGixMerger<TIntUInt64Pr, TUInt64> > PGixMerger;

typedef  TGixStorageLayer < TIntUInt64Pr, TUInt64 > TMyStorageLayer;

typedef  TGixItemSet < TIntUInt64Pr, TUInt64 > TMyItemSet;
typedef  TPt < TMyItemSet > PMyItemSet;

////////////////////////////////////////////////////////////////////////

void Print(TMyStorageLayer* gixsl, TVec<TBlobPt>* blob_pt_v){
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
		TMyStorageLayer gixsl(FNameBlob, faCreate, 100, merger);
		TVec<TBlobPt> blob_pt_v;
		for (int i = 0; i < 10; i++){
			TIntUInt64Pr x(i, i);
			PMyItemSet itemset = TMyItemSet::New(x, merger, &gixsl);
			for (int j = 0; j < 5; j++){
				itemset->AddItem(i + j);
			}
			TBlobPt blob_pt = gixsl.EnlistItemSet(itemset);
			blob_pt_v.Add(blob_pt);
			//gixsl.StoreItemSet(blob_pt);
		}
		gixsl.RefreshMemUsed();

		Print(&gixsl, &blob_pt_v);

		TFOut FOut(FNameGix);
		blob_pt_v.Save(FOut);
	}
	{
		// ok, now read the data out
		TVec<TBlobPt> blob_pt_v;
		TFIn FIn(FNameGix);
		blob_pt_v.Load(FIn);

		PGixMerger merger = TMyMerger::New();
		TMyStorageLayer gixsl(FNameBlob, faRdOnly, 100, merger);

		Print(&gixsl, &blob_pt_v);
	}
}

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {
	/*
	{
	TGix<TIntUInt64Pr, TUInt64> gix("Test1", "data", faCreate, 10000);

	for (int i = 0; i < 100000; i++) {
	TIntUInt64Pr x(i, i);
	gix.AddItem(x, i*5 % 16);
	if (i % 5000 == 1) {
	printf("Evo: %d\n", i);
	}
	}

	}
	*/
	TestStorageLayer1();
	return 0;
}


#define GIX_DEBUG
#define GIX_TEST

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <base.h>
#include <mine.h>
#include <qminer_core.h>
#include <qminer_gs.h>

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


////////////////////////////////////////////////////////////////////////

class XTest {
public:

	void Test_BlobBs_Simple_10() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456789"); // length 10

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 1, "Invalid AllocCount");
			TAssert(stats.AllocSize == 10, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 0, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 10, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 0, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 0, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Simple_7() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 1, "Invalid AllocCount");
			TAssert(stats.AllocSize == 8, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 1, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 7, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 0, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 0, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Medium_12() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7
			auto p2 = blobbs->PutBlob("0123456789012"); // length 13

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 2, "Invalid AllocCount");
			TAssert(stats.AllocSize == 24, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 4, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 20, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 0, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 0, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Simple_7_Del() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7
			blobbs->DelBlob(p1);

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 0, "Invalid AllocCount");
			TAssert(stats.AllocSize == 0, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 0, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 0, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 1, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 8, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Medium_12_Del() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7
			auto p2 = blobbs->PutBlob("0123456789012"); // length 13
			blobbs->DelBlob(p1);
			blobbs->DelBlob(p2);

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 0, "Invalid AllocCount");
			TAssert(stats.AllocSize == 0, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 0, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 0, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 2, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 24, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Medium_12_Del_Put() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7
			auto p2 = blobbs->PutBlob("0123456789012"); // length 13
			blobbs->DelBlob(p1);
			blobbs->DelBlob(p2);
			auto p3 = blobbs->PutBlob("0123456"); // length 7

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 1, "Invalid AllocCount");
			TAssert(stats.AllocSize == 8, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 1, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 7, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 1, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 16, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_BlobBs_Medium_12_Del_Put2() {
		WarnNotifyI(TStr("Starting BlobBs tests..."));
		{
			auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
			auto p1 = blobbs->PutBlob("0123456"); // length 7
			auto p2 = blobbs->PutBlob("0123456789012"); // length 13
			blobbs->DelBlob(p1);
			blobbs->DelBlob(p2);
			auto p3 = blobbs->PutBlob("0123456789012345678"); // length 19

			auto stats = blobbs->GetStats();
			TAssert(stats.AllocCount == 1, "Invalid AllocCount");
			TAssert(stats.AllocSize == 20, "Invalid AllocSize");
			TAssert(stats.AllocUnusedSize == 1, "Invalid AllocUnusedSize");
			TAssert(stats.AllocUsedSize == 19, "Invalid AllocUsedSize");
			TAssert(stats.ReleasedCount == 2, "Invalid ReleasedCount");
			TAssert(stats.ReleasedSize == 24, "Invalid ReleasedSize");
		}
		WarnNotifyI(TStr("Finished."));
	}

	void Test_Simple_1() {
		WarnNotifyI(TStr("Starting..."));
		{

			TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
			TStr def_dir = "..\\..\\..\\..\\..\\examples\\movies";
			
			//TStr def_file = def_dir + "\\movies.def";
			TStr def_file = def_dir + "\\movies_small.def";
			
			//TStr data_file = def_dir + "\\sandbox\\movies\\movies.json";
			TStr data_file = "..\\..\\..\\..\\..\\test\\nodejs\\sandbox\\movies\\movies_data.txt";

			// init unicode
			TUnicodeDef::Load(unicode_file);

			// create new base from definition
			PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
			TPt<TQm::TBase> Base = TQm::TStorage::NewBase("data\\", SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024);

			// load movies data
			{
				{
					TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("Movies");
					{
						PSIn fin = TFIn::New(data_file);
						TStr s;
						while (fin->GetNextLn(s)) {
							PJsonVal json = TJsonVal::GetValFromStr(s);
							store->AddRec(json);
						}
					}
				}
			}
			// do some querying
			
			/*auto res = Base->Search("{ \"$from\": \"Movies\", \"$or\": [ { \"Genres\": \"Action\" }, { \"Plot\": \"America\" } ] }");
			printf("Records: %d\n", res->GetRecs());*/

			auto res = Base->Search("{ \"$join\": { \"$name\": \"Actor\", \"$query\" : { \"$from\": \"Movies\", \"Genres\" : \"Horror\", \"$or\" : [{ \"Title\": \"lost\" }, { \"Plot\": \"lost\" }]}}}");
			printf("Records: %d\n", res->GetRecs());			
		}
		WarnNotifyI(TStr("Finished."));
	}



	void PerformTests() {
		//Test_Simple_1();

		Test_BlobBs_Simple_10();
		Test_BlobBs_Simple_7();
		Test_BlobBs_Medium_12();

		Test_BlobBs_Simple_7_Del();
		Test_BlobBs_Medium_12_Del();
		Test_BlobBs_Medium_12_Del_Put();
		Test_BlobBs_Medium_12_Del_Put2();
	}

};

////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[]) {

	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	TQm::TEnv::Init();

	XTest test;
	test.PerformTests();

	return 0;
}


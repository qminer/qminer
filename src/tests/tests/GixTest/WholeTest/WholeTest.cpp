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



TEST(testTBlobBs, Simple10) {

	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456789"); // length 10

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 1);
	EXPECT_EQ(stats.AllocSize, 10);
	EXPECT_EQ(stats.AllocUnusedSize, 0);
	EXPECT_EQ(stats.AllocUsedSize, 10);
	EXPECT_EQ(stats.ReleasedCount, 0);
	EXPECT_EQ(stats.ReleasedSize, 0);
}

TEST(testTBlobBs, Simple7) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 1);
	EXPECT_EQ(stats.AllocSize, 8);
	EXPECT_EQ(stats.AllocUnusedSize, 1);
	EXPECT_EQ(stats.AllocUsedSize, 7);
	EXPECT_EQ(stats.ReleasedCount, 0);
	EXPECT_EQ(stats.ReleasedSize, 0);

}

TEST(testTBlobBs, Medium12) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7
	auto p2 = blobbs->PutBlob("0123456789012"); // length 13

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 2);
	EXPECT_EQ(stats.AllocSize, 24);
	EXPECT_EQ(stats.AllocUnusedSize, 4);
	EXPECT_EQ(stats.AllocUsedSize, 20);
	EXPECT_EQ(stats.ReleasedCount, 0);
	EXPECT_EQ(stats.ReleasedSize, 0);

}

TEST(testTBlobBs, Simple7Del) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7
	blobbs->DelBlob(p1);

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 0);
	EXPECT_EQ(stats.AllocSize, 0);
	EXPECT_EQ(stats.AllocUnusedSize, 0);
	EXPECT_EQ(stats.AllocUsedSize, 0);
	EXPECT_EQ(stats.ReleasedCount, 1);
	EXPECT_EQ(stats.ReleasedSize, 8);

}

TEST(testTBlobBs, Medium12Del) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7
	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
	blobbs->DelBlob(p1);
	blobbs->DelBlob(p2);

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 0);
	EXPECT_EQ(stats.AllocSize, 0);
	EXPECT_EQ(stats.AllocUnusedSize, 0);
	EXPECT_EQ(stats.AllocUsedSize, 0);
	EXPECT_EQ(stats.ReleasedCount, 2);
	EXPECT_EQ(stats.ReleasedSize, 24);
}

TEST(testTBlobBs, Medium12DelPut) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7
	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
	blobbs->DelBlob(p1);
	blobbs->DelBlob(p2);
	auto p3 = blobbs->PutBlob("0123456"); // length 7

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 1);
	EXPECT_EQ(stats.AllocSize, 8);
	EXPECT_EQ(stats.AllocUnusedSize, 1);
	EXPECT_EQ(stats.AllocUsedSize, 7);
	EXPECT_EQ(stats.ReleasedCount, 1);
	EXPECT_EQ(stats.ReleasedSize, 16);
}

TEST(testTBlobBs, Medium12DelPut2) {
	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
	auto p1 = blobbs->PutBlob("0123456"); // length 7
	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
	blobbs->DelBlob(p1);
	blobbs->DelBlob(p2);
	auto p3 = blobbs->PutBlob("0123456789012345678"); // length 19

	auto stats = blobbs->GetStats();
	EXPECT_EQ(stats.AllocCount, 1);
	EXPECT_EQ(stats.AllocSize, 20);
	EXPECT_EQ(stats.AllocUnusedSize, 1);
	EXPECT_EQ(stats.AllocUsedSize, 19);
	EXPECT_EQ(stats.ReleasedCount, 2);
	EXPECT_EQ(stats.ReleasedSize, 24);
}

TEST(testTBase, myTBaseTest) {
	// init unicode
	TUnicodeDef::Load(unicode_file); 

	TQm::TEnv::Init();

	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
	TStr def_dir = "..\\..\\..\\..\\..\\examples\\movies";

	//TStr def_file = def_dir + "\\movies.def";
	TStr def_file = def_dir + "\\movies_small.def";

	//TStr data_file = def_dir + "\\sandbox\\movies\\movies.json";
	TStr data_file = "..\\..\\..\\..\\..\\test\\nodejs\\sandbox\\movies\\movies_data.txt";

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

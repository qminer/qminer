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

///////////////////////////////////////////////////////////////////////

class XTest {
public:
	static void TInMemStorage_Simple1() {
		TStr Fn = "data\\in_mem_storage";
		TStr tmp;
		{
			TQm::TStorage::TInMemStorage storage(Fn);
			TMem mem;
			mem.AddBf(&storage, 4);
			tmp = mem.GetHexStr();
			auto res1 = storage.AddVal(mem);
			EXPECT_TRUE(res1 == 0); // offset of new record
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate);
			EXPECT_EQ(storage.ValV.Len(), 1);
			EXPECT_EQ(storage.DirtyV.Len(), 1);
			EXPECT_EQ(storage.DirtyV[0], 1); // loaded and clean
			TMem mem;
			storage.GetVal(0, mem);
			EXPECT_EQ(mem.GetHexStr(), tmp);
		}
	}

	static void TInMemStorage_Lazy1() {
		TStr Fn = "data\\in_mem_storage";
		TStr tmp;
		{
			TQm::TStorage::TInMemStorage storage(Fn, 1000);
			TMem mem;
			mem.AddBf(&storage, 4);
			tmp = mem.GetHexStr();
			auto res1 = storage.AddVal(mem);
			EXPECT_TRUE(res1 == 0); // offset of new record
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, 1000, true);
			EXPECT_EQ(storage.ValV.Len(), 1);
			EXPECT_EQ(storage.DirtyV.Len(), 1);
			EXPECT_EQ(storage.DirtyV[0], 3); // not loaded yet
			TMem mem;
			storage.GetVal(0, mem);
			EXPECT_EQ(mem.GetHexStr(), tmp);
		}
	}

	static void TInMemStorage_Complex1() {
		TStr Fn = "data\\in_mem_storage";
		TStr tmp;
		int cnt = 20;
		{
			TQm::TStorage::TInMemStorage storage(Fn, 1000);
			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				tmp = mem.GetHexStr();
				auto res1 = storage.AddVal(mem);
			}
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, 1000);
			EXPECT_EQ(storage.ValV.Len(), cnt);
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet

			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				tmp = mem.GetHexStr();
				auto res1 = storage.AddVal(mem);
			}

			blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
			EXPECT_EQ(storage.ValV.Len(), 2 * cnt);
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, 1000, true);
			EXPECT_EQ(storage.ValV.Len(), 2 * cnt);
		}
	}

	static void TInMemStorage_LoadAll1() {
		TStr Fn = "data\\in_mem_storage";
		TStr tmp;
		int cnt = 20;
		{
			TQm::TStorage::TInMemStorage storage(Fn, 1000);
			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				tmp = mem.GetHexStr();
				auto res1 = storage.AddVal(mem);
			}
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, 1000, true);

			int loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != 3) { // if loaded
					loaded_cnt++;
				}
			}
			EXPECT_EQ(loaded_cnt, 0);

			storage.LoadAll();

			loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != 3) { // if loaded
					loaded_cnt++;
				}
			}
			EXPECT_EQ(loaded_cnt, storage.ValV.Len());
		}
	}

	static void TInMemStorage_LoadAll2() {
		TStr Fn = "data\\in_mem_storage";
		int cnt = 20;
		int block = 5;
		TVec<TStr> temp;
		{
			TQm::TStorage::TInMemStorage storage(Fn, block);
			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				TStr x = mem.GetHexStr();
				//printf("++++%d [%s]\n", i, x.CStr());
				temp.Add(x);
				auto res1 = storage.AddVal(mem);
			}
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, block, true);

			int loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != 3) { // if loaded
					loaded_cnt++;
				}
			}
			EXPECT_EQ(loaded_cnt, 0);

			storage.LoadAll();

			loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				ASSERT_TRUE(storage.DirtyV[i] == 1);
				loaded_cnt++;
				//printf("****[%s] * [%s]\n", temp[i].CStr(), storage.ValV[i].GetHexStr().CStr());
				EXPECT_EQ(temp[i], storage.ValV[i].GetHexStr());
			}
			EXPECT_EQ(loaded_cnt, storage.ValV.Len());
		}
	}


	static void TInMemStorage_PerfTest() {
		TStr Fn = "data\\in_mem_storage";
		TStr Fn2 = "data\\in_mem_storage2";
		int cnt = 1000 * 1000;
		int BlockSize = 1000;
		{
			// generate data
			TQm::TStorage::TInMemStorage storage(Fn, BlockSize);
			TVec<TMem, int64> ValV;
			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				auto res1 = storage.AddVal(mem);
				ValV.Add(mem);
			}
			{
				// save vector
				TFOut FOut(Fn2);
				ValV.Save(FOut);
			}
		}
		{
			TTmStopWatch sw1(true);
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, BlockSize, false);
			sw1.Stop();
			printf("in-mem storage %d\n", sw1.GetMSecInt());

			TTmStopWatch sw2(true);
			TVec<TMem, int64> ValV;
			TFIn FIn(Fn2);
			ValV.Load(FIn);
			printf("vector storage %d\n", sw2.GetMSecInt());
		}
	}
};


//////////////////////////////////////////////////
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


TEST(testTBase, MoviesTest1) {
	TQm::TEnv::Init();

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
	TPt<TQm::TBase> Base = TQm::TStorage::NewBase("data\\", SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);

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


TPt<TQm::TBase> CreatePeopleBase(bool big_file = false) {
	TQm::TEnv::Init();

	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
	TStr def_dir = "test";

	TStr def_file = def_dir + "\\people.def.json";
	TStr data_file = def_dir + (big_file ? "\\people_huge.json" : "\\people_small.json");
	TStr data_dir = "data\\";

	// init unicode
	TUnicodeDef::Load(unicode_file);

	// delete existing files
	if (TDir::Exists(data_dir)) {
		TStrV FNmV;
		TStrV FExtV;
		TFFile::GetFNmV(data_dir, FExtV, true, FNmV);
		bool DirEmpty = FNmV.Len() == 0;

		// delete all files
		for (int FileN = 0; FileN < FNmV.Len(); FileN++) {
			TFile::Del(FNmV[FileN], true);
		}
	}

	// create new base from definition
	PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
	TPt<TQm::TBase> Base = TQm::TStorage::NewBase(data_dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);

	// load movies data
	{
		{
			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("People");
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
	return Base;
}

TPt<TQm::TBase> OpenPeopleBase() {
	TQm::TEnv::Init();

	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
	TStr def_dir = "test";
	TStr data_dir = "data\\";

	// init unicode
	TUnicodeDef::Load(unicode_file);
	TPt<TQm::TBase> Base = TQm::TStorage::LoadBase(data_dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), false, 4 * TInt::Kilo);
	return Base;
}

TEST(testTBase, ClearStoreTest1) {
	auto Base = CreatePeopleBase();
	auto store = Base->GetStoreByStoreNm("People");
	store->DeleteFirstNRecs(store->GetRecs());
	EXPECT_EQ(store->GetRecs(), 0);
}
TEST(testTBase, ClearStoreTest2) {
	auto Base = CreatePeopleBase();
	auto store = Base->GetStoreByStoreNm("People");
	store->DeleteFirstNRecs(1);
	EXPECT_EQ(store->GetRecs(), 1);
}
TEST(testTBase, ClearStoreTestBig1) {
	auto Base = CreatePeopleBase(true);
	auto store = Base->GetStoreByStoreNm("People");
	store->DeleteFirstNRecs(store->GetRecs());
	EXPECT_EQ(store->GetRecs(), 0);
}
TEST(testTBase, ClearStoreTestBig2) {
	auto Base = CreatePeopleBase(true);
	auto store = Base->GetStoreByStoreNm("People");
	store->DeleteFirstNRecs(store->GetRecs() - 1);
	EXPECT_EQ(store->GetRecs(), 1);
}
TEST(testTBase, ClearStoreTestBigComplex) {
	int recs = -1;
	{
		auto Base = CreatePeopleBase(true);
		auto store = Base->GetStoreByStoreNm("People");
		recs = store->GetRecs();
		TQm::TStorage::SaveBase(Base);
	}
	{
		auto Base = OpenPeopleBase();
		auto store = Base->GetStoreByStoreNm("People");
		EXPECT_EQ(store->GetRecs(), recs);
		store->DeleteFirstNRecs(store->GetRecs() - 1);
		EXPECT_EQ(store->GetRecs(), 1);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////

TEST(testTInMemStorage, Simple1) { XTest::TInMemStorage_Simple1(); }
TEST(testTInMemStorage, Lazy1) { XTest::TInMemStorage_Lazy1(); }
TEST(testTInMemStorage, Complex1) { XTest::TInMemStorage_Complex1(); }
TEST(testTInMemStorage, LoadAll1) { XTest::TInMemStorage_LoadAll1(); }
TEST(testTInMemStorage, LoadAll2) { XTest::TInMemStorage_LoadAll2(); }
TEST(testTInMemStorage, PerfTest) { XTest::TInMemStorage_PerfTest(); }
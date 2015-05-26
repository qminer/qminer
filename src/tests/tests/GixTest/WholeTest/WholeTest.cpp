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
#include "pgblob.h"

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
			EXPECT_EQ(storage.DirtyV[0], TQm::TStorage::isdfClean); // loaded and clean
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);
			EXPECT_EQ(storage.ValV.Len(), 1);
			EXPECT_EQ(storage.DirtyV.Len(), 1);
			EXPECT_EQ(storage.DirtyV[0], TQm::TStorage::isdfNotLoaded); // not loaded yet
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate);
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);

			int loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != TQm::TStorage::isdfNotLoaded) { // if loaded
					loaded_cnt++;
				}
			}
			EXPECT_EQ(loaded_cnt, 0);

			storage.LoadAll();

			loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != TQm::TStorage::isdfNotLoaded) { // if loaded
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);

			int loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				if (storage.DirtyV[i] != TQm::TStorage::isdfNotLoaded) { // if loaded
					loaded_cnt++;
				}
			}
			EXPECT_EQ(loaded_cnt, 0);

			storage.LoadAll();

			loaded_cnt = 0;
			for (int i = 0; i < storage.ValV.Len(); i++) {
				ASSERT_TRUE(storage.DirtyV[i] == TQm::TStorage::isdfClean);
				loaded_cnt++;
				//printf("****[%s] * [%s]\n", temp[i].CStr(), storage.ValV[i].GetHexStr().CStr());
				EXPECT_EQ(temp[i], storage.ValV[i].GetHexStr());
			}
			EXPECT_EQ(loaded_cnt, storage.ValV.Len());
		}
	}


	static void TInMemStorage_PerfTest_Internal(int BlockSize) {
		printf("************ BlockSize %d\n", BlockSize);
		TStr Fn = "data\\in_mem_storage";
		TStr Fn2 = "data\\in_mem_storage2";
		int cnt = 1000 * 1000;
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
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, false);
			sw1.Stop();
			printf("in-mem storage %d\n", sw1.GetMSecInt());

			TTmStopWatch sw2(true);
			TVec<TMem, int64> ValV;
			TFIn FIn(Fn2);
			ValV.Load(FIn);
			printf("vector storage %d\n", sw2.GetMSecInt());
		}
	}

	static void TInMemStorage_PerfTest() {
		TInMemStorage_PerfTest_Internal(5000);
		TInMemStorage_PerfTest_Internal(1000);
		TInMemStorage_PerfTest_Internal(500);
		TInMemStorage_PerfTest_Internal(200);
		TInMemStorage_PerfTest_Internal(100);
		TInMemStorage_PerfTest_Internal(50);
		TInMemStorage_PerfTest_Internal(10);
		TInMemStorage_PerfTest_Internal(5);
		TInMemStorage_PerfTest_Internal(1);
	}

	static void TPgBlob_Complex1() {
		auto Base = glib::TPgBlob::Create("data\\xyz");
		auto new_page = Base->CreateNewPage();
		EXPECT_EQ(new_page.Val1.GetFileIndex(), 0);
		EXPECT_EQ(new_page.Val1.GetPage(), 0);
		//printf("%d %d\n", new_page.Val1.GetFileIndex(), new_page.Val1.GetPage());
		auto new_page2 = Base->CreateNewPage();
		EXPECT_EQ(new_page2.Val1.GetFileIndex(), 0);
		EXPECT_EQ(new_page2.Val1.GetPage(), 1);
		//printf("%d %d\n", new_page2.Val1.GetFileIndex(), new_page2.Val1.GetPage());
	}

	static void TPgBlob_Page_Init() {
		byte* bf = new byte[PAGE_SIZE];

		glib::TPgBlob::InitPageP(bf);
		auto header = (glib::TPgBlob::TPgHeader*)bf;

		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true); // new page is not saved yet
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 0);
		EXPECT_EQ(header->OffsetFreeStart, 10);
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE);

		delete[] bf;
	}

	static void TPgBlob_Page_AddInt() {
		byte* bf = new byte[PAGE_SIZE];
		int data = 8765;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res = glib::TPgBlob::AddItem(bf, (byte*)&data, sizeof(int));
		EXPECT_EQ(res, 0);

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 4);

		// retrieve value
		auto rec = glib::TPgBlob::GetItemRec(bf, res);
		int* b = (int*)(bf + rec->Offset);
		EXPECT_EQ(*b, data);

		delete[] bf;
	}

	static void TPgBlob_Page_AddDouble() {
		byte* bf = new byte[PAGE_SIZE];
		double data = 8765.4321;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res = glib::TPgBlob::AddItem(bf, (byte*)&data, sizeof(double));
		EXPECT_EQ(res, 0);

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 8);

		// retrieve value
		auto rec = glib::TPgBlob::GetItemRec(bf, res);
		double* b = (double*)(bf + rec->Offset);
		EXPECT_EQ(*b, data);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveral() {
		byte* bf = new byte[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (byte*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (byte*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (byte*)&data3, sizeof(int));
		EXPECT_EQ(res1, 0);
		EXPECT_EQ(res2, 1);
		EXPECT_EQ(res3, 2);

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 3 * 4);

		// retrieve values
		auto rec1 = glib::TPgBlob::GetItemRec(bf, res1);
		int* b1 = (int*)(bf + rec1->Offset);
		EXPECT_EQ(*b1, data1);
		auto rec2 = glib::TPgBlob::GetItemRec(bf, res2);
		int* b2 = (int*)(bf + rec2->Offset);
		EXPECT_EQ(*b2, data2);
		auto rec3 = glib::TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntMany() {
		byte* bf = new byte[PAGE_SIZE];
		int data1 = 5;

		glib::TPgBlob::InitPageP(bf);

		// add values
		for (int i = 0; i < 1000; i++) {
			glib::TPgBlob::AddItem(bf, (byte*)&data1, sizeof(int));
			data1 += 3;
		}

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1000);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 1000 * 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 1000 * 4);

		// retrieve values
		data1 = 5;
		for (int i = 0; i < 1000; i++) {
			auto rec1 = glib::TPgBlob::GetItemRec(bf, i);
			int* b1 = (int*)(bf + rec1->Offset);
			EXPECT_EQ(*b1, data1);
			data1 += 3;
		}

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveralDelete() {
		byte* bf = new byte[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (byte*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (byte*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (byte*)&data3, sizeof(int));    

		glib::TPgBlob::DeleteItem(bf, res2);

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // 3 items
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 2 * 4); // 2 actually contain data

		// retrieve values
		auto rec1 = glib::TPgBlob::GetItemRec(bf, res1);
		int* b1 = (int*)(bf + rec1->Offset);
		EXPECT_EQ(*b1, data1);
		auto rec3 = glib::TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveralDelete2() {
		byte* bf = new byte[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (byte*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (byte*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (byte*)&data3, sizeof(int));

		glib::TPgBlob::DeleteItem(bf, res1);

		// check internal state
		auto header = (glib::TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // 3 items
		EXPECT_EQ(header->OffsetFreeEnd, PAGE_SIZE - 2 * 4); // 2 actually contain data

		// retrieve values
		auto rec2 = glib::TPgBlob::GetItemRec(bf, res2);
		int* b2 = (int*)(bf + rec2->Offset);
		EXPECT_EQ(*b2, data2);
		auto rec3 = glib::TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}
};


//////////////////////////////////////////////////
//TEST(testTBlobBs, Simple10) {
//
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456789"); // length 10
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 1);
//	EXPECT_EQ(stats.AllocSize, 10);
//	EXPECT_EQ(stats.AllocUnusedSize, 0);
//	EXPECT_EQ(stats.AllocUsedSize, 10);
//	EXPECT_EQ(stats.ReleasedCount, 0);
//	EXPECT_EQ(stats.ReleasedSize, 0);
//}
//
//TEST(testTBlobBs, Simple7) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 1);
//	EXPECT_EQ(stats.AllocSize, 8);
//	EXPECT_EQ(stats.AllocUnusedSize, 1);
//	EXPECT_EQ(stats.AllocUsedSize, 7);
//	EXPECT_EQ(stats.ReleasedCount, 0);
//	EXPECT_EQ(stats.ReleasedSize, 0);
//}
//
//TEST(testTBlobBs, Medium12) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 2);
//	EXPECT_EQ(stats.AllocSize, 24);
//	EXPECT_EQ(stats.AllocUnusedSize, 4);
//	EXPECT_EQ(stats.AllocUsedSize, 20);
//	EXPECT_EQ(stats.ReleasedCount, 0);
//	EXPECT_EQ(stats.ReleasedSize, 0);
//}
//
//TEST(testTBlobBs, Simple7Del) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//	blobbs->DelBlob(p1);
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 0);
//	EXPECT_EQ(stats.AllocSize, 0);
//	EXPECT_EQ(stats.AllocUnusedSize, 0);
//	EXPECT_EQ(stats.AllocUsedSize, 0);
//	EXPECT_EQ(stats.ReleasedCount, 1);
//	EXPECT_EQ(stats.ReleasedSize, 8);
//}
//
//TEST(testTBlobBs, Medium12Del) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
//	blobbs->DelBlob(p1);
//	blobbs->DelBlob(p2);
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 0);
//	EXPECT_EQ(stats.AllocSize, 0);
//	EXPECT_EQ(stats.AllocUnusedSize, 0);
//	EXPECT_EQ(stats.AllocUsedSize, 0);
//	EXPECT_EQ(stats.ReleasedCount, 2);
//	EXPECT_EQ(stats.ReleasedSize, 24);
//}
//
//TEST(testTBlobBs, Medium12DelPut) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
//	blobbs->DelBlob(p1);
//	blobbs->DelBlob(p2);
//	auto p3 = blobbs->PutBlob("0123456"); // length 7
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 1);
//	EXPECT_EQ(stats.AllocSize, 8);
//	EXPECT_EQ(stats.AllocUnusedSize, 1);
//	EXPECT_EQ(stats.AllocUsedSize, 7);
//	EXPECT_EQ(stats.ReleasedCount, 1);
//	EXPECT_EQ(stats.ReleasedSize, 16);
//}
//
//TEST(testTBlobBs, Medium12DelPut2) {
//	auto blobbs = TMBlobBs::New("data\\blobbs_test", faCreate);
//	auto p1 = blobbs->PutBlob("0123456"); // length 7
//	auto p2 = blobbs->PutBlob("0123456789012"); // length 13
//	blobbs->DelBlob(p1);
//	blobbs->DelBlob(p2);
//	auto p3 = blobbs->PutBlob("0123456789012345678"); // length 19
//
//	auto stats = blobbs->GetStats();
//	EXPECT_EQ(stats.AllocCount, 1);
//	EXPECT_EQ(stats.AllocSize, 20);
//	EXPECT_EQ(stats.AllocUnusedSize, 1);
//	EXPECT_EQ(stats.AllocUsedSize, 19);
//	EXPECT_EQ(stats.ReleasedCount, 2);
//	EXPECT_EQ(stats.ReleasedSize, 24);
//}
//
//
//TEST(testTBase, MoviesTest1) {
//	TQm::TEnv::Init();
//
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr def_dir = "..\\..\\..\\..\\..\\examples\\movies";
//
//	//TStr def_file = def_dir + "\\movies.def";
//	TStr def_file = def_dir + "\\movies_small.def";
//
//	//TStr data_file = def_dir + "\\sandbox\\movies\\movies.json";
//	TStr data_file = "..\\..\\..\\..\\..\\test\\nodejs\\sandbox\\movies\\movies_data.txt";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//
//	// create new base from definition
//	PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//	TPt<TQm::TBase> Base = TQm::TStorage::NewBase("data\\", SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//	// load movies data
//	{
//		{
//			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("Movies");
//			{
//				PSIn fin = TFIn::New(data_file);
//				TStr s;
//				while (fin->GetNextLn(s)) {
//					PJsonVal json = TJsonVal::GetValFromStr(s);
//					store->AddRec(json);
//				}
//			}
//		}
//	}
//	// do some querying
//
//	/*auto res = Base->Search("{ \"$from\": \"Movies\", \"$or\": [ { \"Genres\": \"Action\" }, { \"Plot\": \"America\" } ] }");
//	printf("Records: %d\n", res->GetRecs());*/
//
//	auto res = Base->Search("{ \"$join\": { \"$name\": \"Actor\", \"$query\" : { \"$from\": \"Movies\", \"Genres\" : \"Horror\", \"$or\" : [{ \"Title\": \"lost\" }, { \"Plot\": \"lost\" }]}}}");
//	printf("Records: %d\n", res->GetRecs());
//}
//
//
//TPt<TQm::TBase> CreatePeopleBase(bool big_file = false) {
//	TQm::TEnv::Init();
//
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr def_dir = "test";
//
//	TStr def_file = def_dir + "\\people.def.json";
//	TStr data_file = def_dir + (big_file ? "\\people_huge.json" : "\\people_small.json");
//	TStr data_dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//
//	// delete existing files
//	if (TDir::Exists(data_dir)) {
//		TStrV FNmV;
//		TStrV FExtV;
//		TFFile::GetFNmV(data_dir, FExtV, true, FNmV);
//		bool DirEmpty = FNmV.Len() == 0;
//
//		// delete all files
//		for (int FileN = 0; FileN < FNmV.Len(); FileN++) {
//			TFile::Del(FNmV[FileN], true);
//		}
//	}
//
//	// create new base from definition
//	PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//	TPt<TQm::TBase> Base = TQm::TStorage::NewBase(data_dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//	// load movies data
//	{
//		{
//			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("People");
//			{
//				PSIn fin = TFIn::New(data_file);
//				TStr s;
//				while (fin->GetNextLn(s)) {
//					PJsonVal json = TJsonVal::GetValFromStr(s);
//					store->AddRec(json);
//				}
//			}
//		}
//	}
//	Base->ResetGixStats();
//	return Base;
//}
//
//TWPt<TQm::TBase> OpenPeopleBaseWPt(bool read_only = false) {
//	TQm::TEnv::Init();
//
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr def_dir = "test";
//	TStr data_dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//	return TQm::TStorage::LoadBase(data_dir, (read_only ? TFAccess::faRdOnly : TFAccess::faUpdate), 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), false, 4 * TInt::Kilo);
//}
//
//TPt<TQm::TBase> OpenPeopleBase(bool read_only = false) {
//	TPt<TQm::TBase> Base = OpenPeopleBaseWPt(read_only);
//	return Base;
//}
//
//TEST(testTBase, ClearStoreTest1) {
//	auto Base = CreatePeopleBase();
//	auto store = Base->GetStoreByStoreNm("People");
//	store->DeleteFirstNRecs((int)store->GetRecs());
//	EXPECT_EQ(store->GetRecs(), 0);
//}
//TEST(testTBase, ClearStoreTest2) {
//	auto Base = CreatePeopleBase();
//	auto store = Base->GetStoreByStoreNm("People");
//	store->DeleteFirstNRecs(1);
//	EXPECT_EQ(store->GetRecs(), 1);
//}
//TEST(testTBase, ClearStoreTestBig1) {
//	auto Base = CreatePeopleBase(true);
//	auto store = Base->GetStoreByStoreNm("People");
//	store->DeleteFirstNRecs((int)store->GetRecs());
//	EXPECT_EQ(store->GetRecs(), 0);
//}
//TEST(testTBase, ClearStoreTestBig2) {
//	auto Base = CreatePeopleBase(true);
//	auto store = Base->GetStoreByStoreNm("People");
//	store->DeleteFirstNRecs((int)store->GetRecs() - 1);
//	EXPECT_EQ(store->GetRecs(), 1);
//}
//TEST(testTBase, ClearStoreTestBigComplex) {
//	int recs = -1;
//	{
//		auto Base = CreatePeopleBase(true);
//		Base->PartialFlush(500);
//		auto store = Base->GetStoreByStoreNm("People");
//		recs = (int)store->GetRecs();
//		TQm::TStorage::SaveBase(Base);
//
//		printf("%s\n", Base->GetStats()->SaveStr());
//	}
//	{
//		auto Base = OpenPeopleBase();
//		auto store = Base->GetStoreByStoreNm("People");
//		EXPECT_EQ(store->GetRecs(), recs);
//		store->DeleteFirstNRecs((int)store->GetRecs() - 1);
//		EXPECT_EQ(store->GetRecs(), 1);
//	}
//}
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
//TEST(testTBase, ReadOnlyAfterCrash) {
//	int recs = -1;
//	{
//		// create new database
//		auto Base = CreatePeopleBase(true);
//		Base->PartialFlush(500);
//		auto store = Base->GetStoreByStoreNm("People");
//		recs = (int)store->GetRecs();
//		TQm::TStorage::SaveBase(Base);
//	}
//	{
//		// open it in read-only mode
//		auto Base = OpenPeopleBaseWPt(true);
//		auto Base2 = Base();
//		
//	}
//	{
//		auto Base = OpenPeopleBase();
//		auto store = Base->GetStoreByStoreNm("People");
//		EXPECT_EQ(store->GetRecs(), recs);
//		store->DeleteFirstNRecs((int)store->GetRecs() - 1);
//		EXPECT_EQ(store->GetRecs(), 1);
//	}
//}
//
//
////////////////////////////////////////////////////////////////////////////////////////////////
//
//TEST(testTInMemStorage, Simple1) { XTest::TInMemStorage_Simple1(); }
//TEST(testTInMemStorage, Lazy1) { XTest::TInMemStorage_Lazy1(); }
//TEST(testTInMemStorage, Complex1) { XTest::TInMemStorage_Complex1(); }
//TEST(testTInMemStorage, LoadAll1) { XTest::TInMemStorage_LoadAll1(); }
//TEST(testTInMemStorage, LoadAll2) { XTest::TInMemStorage_LoadAll2(); }
//TEST(testTInMemStorage, PerfTest) { XTest::TInMemStorage_PerfTest(); }

///////////////////////////////////////////////////////////////////////////


TEST(testTPgBlob, Simple) { XTest::TPgBlob_Complex1(); }
TEST(testTPgBlob, PageInit) { XTest::TPgBlob_Page_Init(); }
TEST(testTPgBlob, PageAddInt) { XTest::TPgBlob_Page_AddInt(); }
TEST(testTPgBlob, PageAddIntMany) { XTest::TPgBlob_Page_AddIntMany(); }
TEST(testTPgBlob, PageAddDouble) { XTest::TPgBlob_Page_AddDouble(); }
TEST(testTPgBlob, PageAddIntSeveral) { XTest::TPgBlob_Page_AddIntSeveral(); }
TEST(testTPgBlob, PageAddIntSeveralDelete) { XTest::TPgBlob_Page_AddIntSeveralDelete(); }
TEST(testTPgBlob, PageAddIntSeveralDelete2) { XTest::TPgBlob_Page_AddIntSeveralDelete2(); }
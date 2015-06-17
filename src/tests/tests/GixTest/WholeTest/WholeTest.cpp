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
#include "qminer_pbs.h"

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

	static void TInMemStorage_Delete1() {
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
				temp.Add(x);
				auto res1 = storage.AddVal(mem);
			}
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);
			storage.LoadAll();

			// delete 7 values in one chunk
			storage.DelVals(7);
			EXPECT_EQ(15, storage.ValV.Len()); // first block was removed
			EXPECT_EQ(5, storage.FirstValOffsetMem);
			EXPECT_EQ(2, storage.FirstValOffset);
			EXPECT_EQ(7, storage.GetFirstValId());
			EXPECT_EQ(19, storage.GetLastValId());
			EXPECT_EQ(13, storage.Len());
			EXPECT_EQ(false, storage.IsValId(4));
			EXPECT_EQ(true, storage.IsValId(7));
			EXPECT_EQ(true, storage.IsValId(19));
			EXPECT_EQ(false, storage.IsValId(20));
		}
	}

	static void TInMemStorage_Delete2() {
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
				temp.Add(x);
				auto res1 = storage.AddVal(mem);
			}
			auto blob_stats = storage.GetBlobStorage()->GetStats();
			EXPECT_TRUE(blob_stats.PutsNew == 0); // no data should be saved yet
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);
			storage.LoadAll();

			// delete 7 values, one by one
			for (int i = 0; i < 7; i++) {
				storage.DelVals(1);
			}
			EXPECT_EQ(15, storage.ValV.Len()); // first block was removed
			EXPECT_EQ(5, storage.FirstValOffsetMem);
			EXPECT_EQ(2, storage.FirstValOffset);
			EXPECT_EQ(7, storage.GetFirstValId());
			EXPECT_EQ(19, storage.GetLastValId());
			EXPECT_EQ(13, storage.Len());
			EXPECT_EQ(false, storage.IsValId(4));
			EXPECT_EQ(true, storage.IsValId(7));
			EXPECT_EQ(true, storage.IsValId(19));
			EXPECT_EQ(false, storage.IsValId(20));
		}
	}

	static void TInMemStorage_SetVal() {
		TStr Fn = "data\\in_mem_storage";
		int cnt = 20;
		int block = 5;
		int target_id = 9;
		TVec<TStr> temp;
		{
			TQm::TStorage::TInMemStorage storage(Fn, block);
			for (int i = 0; i < cnt; i++) {
				TMem mem;
				mem.AddBf(&storage, i % 4);
				TStr x = mem.GetHexStr();
				temp.Add(x);
				auto res1 = storage.AddVal(mem);
			}

			EXPECT_EQ(TQm::TStorage::isdfNew, storage.DirtyV[target_id]);

			// update 
			TMem mem;
			mem.AddBf(&storage, 6);
			storage.SetVal(target_id, mem);

			EXPECT_EQ(TQm::TStorage::isdfNew, storage.DirtyV[target_id]);
		}
		{
			TQm::TStorage::TInMemStorage storage(Fn, faUpdate, true);
			storage.LoadAll();

			EXPECT_EQ(TQm::TStorage::isdfClean, storage.DirtyV[target_id]);

			// update 
			TMem mem;
			mem.AddBf(&storage, 8);
			storage.SetVal(target_id, mem);

			EXPECT_EQ(TQm::TStorage::isdfDirty, storage.DirtyV[target_id]);
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


	/////////////////////////////////////

	static void TPgBlob_Complex1() {
		auto Base = glib::TPgBlob::Create("data\\xyz");
		glib::TPgBlobPgPt Pt;
		char* BfPt;
		Base->CreateNewPage(Pt, &BfPt);
		EXPECT_EQ(Pt.GetFIx(), 0);
		EXPECT_EQ(Pt.GetPg(), 0);
		Base->CreateNewPage(Pt, &BfPt);
		EXPECT_EQ(Pt.GetFIx(), 0);
		EXPECT_EQ(Pt.GetPg(), 1);
	}

	static void TPgBlob_Page_Init() {
		char* bf = new char[PAGE_SIZE];

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
		char* bf = new char[PAGE_SIZE];
		int data = 8765;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res = glib::TPgBlob::AddItem(bf, (char*)&data, sizeof(int));
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
		char* bf = new char[PAGE_SIZE];
		double data = 8765.4321;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res = glib::TPgBlob::AddItem(bf, (char*)&data, sizeof(double));
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
		char* bf = new char[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));
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
		char* bf = new char[PAGE_SIZE];
		int data1 = 5;

		glib::TPgBlob::InitPageP(bf);

		// add values
		for (int i = 0; i < 1000; i++) {
			glib::TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
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
		char* bf = new char[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));

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
		char* bf = new char[PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		glib::TPgBlob::InitPageP(bf);

		// add value
		auto res1 = glib::TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = glib::TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = glib::TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));

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

	//////////////////////////////////////////////////////////////////



	static void TPgBlob_AddBf1() {

		double d1 = 65.43;
		double d2 = 111234.7;
		int i1 = 89;
		TFlt tmp = 0;
		TInt tmp3 = 0;

		auto pb = glib::TPgBlob("data/pb", TFAccess::faCreate, 4194304);
		auto p1 = pb.Put((char*)&d1, sizeof(double));
		auto p2 = pb.Put((char*)&d2, sizeof(double));

		EXPECT_EQ(p1.GetFIx(), 0);
		EXPECT_EQ(p1.GetPg(), 0);
		EXPECT_EQ(p1.GetIIx(), 0);

		EXPECT_EQ(p2.GetFIx(), 0);
		EXPECT_EQ(p2.GetPg(), 0);
		EXPECT_EQ(p2.GetIIx(), 1);

		auto sin1 = pb.Get(p1);
		tmp.Load(sin1);
		EXPECT_EQ(tmp, d1);

		auto sin2 = pb.Get(p2);
		tmp.Load(sin2);
		EXPECT_EQ(tmp, d2);

		// now insert int instead of double
		p1 = pb.Put((char*)&i1, sizeof(int), p1);

		EXPECT_EQ(p1.GetFIx(), 0);
		EXPECT_EQ(p1.GetPg(), 0);
		EXPECT_EQ(p1.GetIIx(), 0);

		EXPECT_EQ(p2.GetFIx(), 0);
		EXPECT_EQ(p2.GetPg(), 0);
		EXPECT_EQ(p2.GetIIx(), 1);

		auto sin3 = pb.Get(p1);
		tmp3.Load(sin3);
		EXPECT_EQ(tmp3, i1);

		auto sin4 = pb.Get(p2);
		tmp.Load(sin4);
		EXPECT_EQ(tmp, d2);

		// peek into internal structure and check state
		auto pg_item = (glib::TPgBlob::TPgBlobPageItem*)
			(pb.GetPageBf(0) + sizeof(glib::TPgBlob::TPgHeader));
		EXPECT_EQ(pg_item->Len, 4);
		EXPECT_EQ(pg_item->Offset, 8180);
		pg_item++;
		EXPECT_EQ(pg_item->Len, 8);
		EXPECT_EQ(pg_item->Offset, 8184);
	}

	//////////////

	static void TBinTreeMaxVals_Add1() {
		glib::TBinTreeMaxVals Vals;
		glib::TPgBlobPgPt Pt(0, 0);
		Vals.Add(4, Pt);
		Vals.Add(3, Pt);
		EXPECT_EQ(Vals.GetIndexOfMax(), 0);
		Vals.Add(1, Pt);
		Vals.Add(6, Pt);
		EXPECT_EQ(Vals.GetIndexOfMax(), 3);
		Vals.Add(1, Pt);
		Vals.Add(5, Pt);
		EXPECT_EQ(Vals.GetIndexOfMax(), 3);
		Vals.Add(8, Pt);
		Vals.Add(5, Pt);
		Vals.Print();
		EXPECT_EQ(Vals.GetIndexOfMax(), 6);
		Vals.Change(6, 2);
		Vals.Print();
		EXPECT_EQ(Vals.GetIndexOfMax(), 3);
		Vals.Change(3, 3);
		Vals.Print();
		EXPECT_EQ(Vals.GetIndexOfMax(), 5);
	}
};
//
//TEST(testTInMemStorage, Simple1) { XTest::TInMemStorage_Simple1(); }
//TEST(testTInMemStorage, Lazy1) { XTest::TInMemStorage_Lazy1(); }
//TEST(testTInMemStorage, Complex1) { XTest::TInMemStorage_Complex1(); }
//TEST(testTInMemStorage, LoadAll1) { XTest::TInMemStorage_LoadAll1(); }
//TEST(testTInMemStorage, LoadAll2) { XTest::TInMemStorage_LoadAll2(); }
//TEST(testTInMemStorage, Delete1) { XTest::TInMemStorage_Delete1(); }
//TEST(testTInMemStorage, Delete2) { XTest::TInMemStorage_Delete2(); }
//TEST(testTInMemStorage, SetVal) { XTest::TInMemStorage_SetVal(); }
////TEST(testTInMemStorage, PerfTest) { XTest::TInMemStorage_PerfTest(); }
//
//
//TEST(testTPgBlob, Simple) { XTest::TPgBlob_Complex1(); }
//TEST(testTPgBlob, PageInit) { XTest::TPgBlob_Page_Init(); }
//TEST(testTPgBlob, PageAddInt) { XTest::TPgBlob_Page_AddInt(); }
//TEST(testTPgBlob, PageAddIntMany) { XTest::TPgBlob_Page_AddIntMany(); }
//TEST(testTPgBlob, PageAddDouble) { XTest::TPgBlob_Page_AddDouble(); }
//TEST(testTPgBlob, PageAddIntSeveral) { XTest::TPgBlob_Page_AddIntSeveral(); }
//TEST(testTPgBlob, PageAddIntSeveralDelete) { XTest::TPgBlob_Page_AddIntSeveralDelete(); }
//TEST(testTPgBlob, PageAddIntSeveralDelete2) { XTest::TPgBlob_Page_AddIntSeveralDelete2(); }
//
//TEST(testTPgBlob, AddBf1) { XTest::TPgBlob_AddBf1(); }
//

//TEST(TBinTreeMaxVals, Add1) { XTest::TBinTreeMaxVals_Add1(); }
//////////////////////////////////////////////////////////////////////////////////

//
//TEST(testTPgBlobFsm, Add1) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	EXPECT_EQ(fsm.Len(), 5);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 2);
//
//	bool res1;
//	glib::TPgBlobPgPt pt1;
//	res1 = fsm.FsmGetFreePage(2, pt1);
//	EXPECT_EQ(res1, true);
//	EXPECT_EQ(pt1.GetPg(), 4);
//
//	res1 = fsm.FsmGetFreePage(6, pt1);
//	EXPECT_EQ(res1, false);
//}
//
//TEST(testTPgBlobFsm, AddUpdateInc) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	glib::TPgBlobPgPt pt(0, 1);
//	fsm.FsmUpdatePage(pt, 6); // 1,1 -> 1,6
//	EXPECT_EQ(fsm.Len(), 5);
//	//EXPECT_EQ(fsm[0].GetPg(), 1);
//	//EXPECT_EQ(fsm[1].GetPg(), 4);
//	//EXPECT_EQ(fsm[2].GetPg(), 2);
//	//EXPECT_EQ(fsm[3].GetPg(), 0);
//	//EXPECT_EQ(fsm[4].GetPg(), 3);
//	//EXPECT_EQ(fsm[0].GetIIx(), 6);
//	//EXPECT_EQ(fsm[1].GetIIx(), 4);
//	//EXPECT_EQ(fsm[2].GetIIx(), 2);
//	//EXPECT_EQ(fsm[3].GetIIx(), 0);
//	//EXPECT_EQ(fsm[4].GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 6);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 3);
//}
//
//TEST(testTPgBlobFsm, AddUpdateInc2) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	glib::TPgBlobPgPt pt(0, 1);
//	fsm.FsmUpdatePage(pt, 4); // 1,1 -> 1,4
//
//	EXPECT_EQ(fsm.Len(), 5);
//	//EXPECT_EQ(fsm[0].GetPg(), 4);
//	//EXPECT_EQ(fsm[1].GetPg(), 1);
//	//EXPECT_EQ(fsm[2].GetPg(), 2);
//	//EXPECT_EQ(fsm[3].GetPg(), 0);
//	//EXPECT_EQ(fsm[4].GetPg(), 3);
//	//EXPECT_EQ(fsm[0].GetIIx(), 4);
//	//EXPECT_EQ(fsm[1].GetIIx(), 4);
//	//EXPECT_EQ(fsm[2].GetIIx(), 2);
//	//EXPECT_EQ(fsm[3].GetIIx(), 0);
//	//EXPECT_EQ(fsm[4].GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 3);
//}
//
//TEST(testTPgBlobFsm, AddUpdateDec) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	glib::TPgBlobPgPt pt(0, 1);
//	fsm.FsmUpdatePage(pt, 0); // 1,1 -> 1,0
//
//	EXPECT_EQ(fsm.Len(), 5);
//	//EXPECT_EQ(fsm[0].GetPg(), 4);
//	//EXPECT_EQ(fsm[1].GetPg(), 3);
//	//EXPECT_EQ(fsm[2].GetPg(), 2);
//	//EXPECT_EQ(fsm[3].GetPg(), 0);
//	//EXPECT_EQ(fsm[4].GetPg(), 1);
//	//EXPECT_EQ(fsm[0].GetIIx(), 4);
//	//EXPECT_EQ(fsm[1].GetIIx(), 3);
//	//EXPECT_EQ(fsm[2].GetIIx(), 2);
//	//EXPECT_EQ(fsm[3].GetIIx(), 0);
//	//EXPECT_EQ(fsm[4].GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 2);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 0);
//}
//
//TEST(testTPgBlobFsm, AddUpdateDec2) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	glib::TPgBlobPgPt pt(0, 2);
//	fsm.FsmUpdatePage(pt, 0); // 2,2 -> 2,0
//
//	EXPECT_EQ(fsm.Len(), 5);
//	//EXPECT_EQ(fsm[0].GetPg(), 4);
//	//EXPECT_EQ(fsm[1].GetPg(), 3);
//	//EXPECT_EQ(fsm[2].GetPg(), 1);
//	//EXPECT_EQ(fsm[3].GetPg(), 0);
//	//EXPECT_EQ(fsm[4].GetPg(), 2);
//	//EXPECT_EQ(fsm[0].GetIIx(), 4);
//	//EXPECT_EQ(fsm[1].GetIIx(), 3);
//	//EXPECT_EQ(fsm[2].GetIIx(), 1);
//	//EXPECT_EQ(fsm[3].GetIIx(), 0);
//	//EXPECT_EQ(fsm[4].GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 4);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 0);
//}
//
//TEST(testTPgBlobFsm, AddUpdateDec3) {
//	glib::TPgBlobFsm fsm;
//	for (int i = 0; i < 5; i++) {
//		glib::TPgBlobPgPt pt(0, i);
//		fsm.FsmAddPage(pt, i);
//	}
//
//	glib::TPgBlobPgPt pt(0, 4);
//	fsm.FsmUpdatePage(pt, 2); // 4,4 -> 4,2
//
//	EXPECT_EQ(fsm.Len(), 5);
//	//EXPECT_EQ(fsm[0].GetPg(), 3);
//	//EXPECT_EQ(fsm[1].GetPg(), 2);
//	//EXPECT_EQ(fsm[2].GetPg(), 1);
//	//EXPECT_EQ(fsm[3].GetPg(), 0);
//	//EXPECT_EQ(fsm[4].GetPg(), 4);
//	//EXPECT_EQ(fsm[0].GetIIx(), 3);
//	//EXPECT_EQ(fsm[1].GetIIx(), 2);
//	//EXPECT_EQ(fsm[2].GetIIx(), 1);
//	//EXPECT_EQ(fsm[3].GetIIx(), 0);
//	//EXPECT_EQ(fsm[4].GetIIx(), 2);
//	EXPECT_EQ(fsm.GetVal(0).GetPg(), 3);
//	EXPECT_EQ(fsm.GetVal(1).GetPg(), 2);
//	EXPECT_EQ(fsm.GetVal(2).GetPg(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetPg(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetPg(), 4);
//	EXPECT_EQ(fsm.GetVal(0).GetIIx(), 3);
//	EXPECT_EQ(fsm.GetVal(1).GetIIx(), 2);
//	EXPECT_EQ(fsm.GetVal(2).GetIIx(), 1);
//	EXPECT_EQ(fsm.GetVal(3).GetIIx(), 0);
//	EXPECT_EQ(fsm.GetVal(4).GetIIx(), 2);
//}

////////////////////////////////////////////////////
//
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
//	//TStr def_file = def_dir + "\\movies_small.def";
//	TStr def_file = ".\\movies_small.def";
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
//////////////////////////////////////////////////////////////////////////////////////////////
//
//void TestPgBlobStoreSimple(const TStr& def_file) {
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//
//	{
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		{
//			{
//				TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//				{
//					TStr s1 = "{ \"FieldInt\" : 12, \"FieldBool\" : true}";
//					PJsonVal json1 = TJsonVal::GetValFromStr(s1);
//					store->AddRec(json1);
//
//					TStr s2 = "{ \"FieldInt\" : 6345, \"FieldBool\" : false}";
//					PJsonVal json2 = TJsonVal::GetValFromStr(s2);
//					store->AddRec(json2);
//				}
//			}
//		}
//		printf("%s\n", TJsonVal::GetStrFromVal(Base->GetStats()).CStr());
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faRdOnly, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id = store->GetFieldId("FieldInt");
//		auto res = store->GetAllRecs();
//		//printf("Records: %d\n", res->GetRecs());
//		EXPECT_EQ(res->GetRecs(), 2);
//		//printf("Records: %d\n", res->GetRec(0).GetFieldInt(field_id));
//		EXPECT_EQ(res->GetRec(0).GetFieldInt(field_id), 12);
//		//printf("Records: %d\n", res->GetRec(1).GetFieldInt(field_id));
//		EXPECT_EQ(res->GetRec(1).GetFieldInt(field_id), 6345);
//
//		TQm::SaveBase2(Base);
//	}
//}
//
//TEST(testTStorePbBlob, Test1) {
//	TestPgBlobStoreSimple(".\\pgblob_test1.def");
//}
//TEST(testTStorePbBlob, Test2) {
//	TestPgBlobStoreSimple(".\\pgblob_test2.def");
//}
//TEST(testTStorePbBlob, Test2_Big) {
//	TStr def_file = ".\\pgblob_test2.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//	int rec_count = 250*1000;
//
//	{
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		{
//			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//			TStr s = "{ \"FieldInt\" : 12, \"FieldBool\" : true}";
//			PJsonVal json = TJsonVal::GetValFromStr(s);
//			PJsonVal json_int = json->GetObjKey("FieldInt");
//			PJsonVal json_bool = json->GetObjKey("FieldBool");
//			for (int i = 0; i < rec_count; i++) {
//				if (i % 1000 == 0)
//					printf("    %d\r", i);
//				json_int->PutNum(i);
//				json_bool->PutBool(i % 7 == 3);
//				store->AddRec(json);
//			}
//		}
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faRdOnly, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_int = store->GetFieldId("FieldInt");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		auto res = store->GetAllRecs();
//		EXPECT_EQ(res->GetRecs(), rec_count);
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0)
//				printf("    %d\r", i);
//			TQm::TRec& rec = res->GetRec(i);
//			EXPECT_EQ(rec.GetFieldInt(field_id_int), i);
//			EXPECT_EQ(rec.GetFieldBool(field_id_bool), i % 7 == 3);
//		}
//
//		TQm::SaveBase2(Base);
//	}
//}
//TEST(testTStorePbBlob, Test2_String) {
//	TStr def_file = ".\\pgblob_test_str1.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//	int rec_count = 1000*1000;
//
//	{
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		{
//			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//			TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//			PJsonVal json = TJsonVal::GetValFromStr(s);
//			PJsonVal json_str = json->GetObjKey("FieldString");
//			PJsonVal json_bool = json->GetObjKey("FieldBool");
//			for (int i = 0; i < rec_count; i++) {
//				if (i % 1000 == 0)
//					printf("    %d\r", i);
//				json_str->PutStr(TStr::Fmt("Stored value %d", i));
//				json_bool->PutBool(i % 7 == 3);
//				store->AddRec(json);
//			}
//		}
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faRdOnly, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		auto res = store->GetAllRecs();
//		EXPECT_EQ(res->GetRecs(), rec_count);
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0)
//				printf("    %d\r", i);
//			TQm::TRec& rec = res->GetRec(i);
//			EXPECT_EQ(rec.GetFieldStr(field_id_str), TStr::Fmt("Stored value %d", i));
//			EXPECT_EQ(rec.GetFieldBool(field_id_bool), i % 7 == 3);
//		}
//
//		TQm::SaveBase2(Base);
//	}
//}
//
//TEST(testTStorePbBlob, Test2_String_Delete) {
//	TStr def_file = ".\\pgblob_test_str1.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TUnicodeDef::Load(unicode_file);
//	int rec_count = 300 * 1000;
//
//	{
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		{
//			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//			TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//			PJsonVal json = TJsonVal::GetValFromStr(s);
//			PJsonVal json_str = json->GetObjKey("FieldString");
//			PJsonVal json_bool = json->GetObjKey("FieldBool");
//			for (int i = 0; i < rec_count; i++) {
//				if (i % 1000 == 0)
//					printf("    %d\r", i);
//				json_str->PutStr(TStr::Fmt("Stored value %d", i));
//				json_bool->PutBool(i % 7 == 3);
//				store->AddRec(json);
//			}
//		}
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		// now delete each record with id, where id % 3 == 2
//
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		auto res = store->GetAllRecs();
//		EXPECT_EQ(res->GetRecs(), rec_count);
//
//		TUInt64V ids;
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 3 != 2)
//				continue;
//			TQm::TRec& rec = res->GetRec(i);
//			ids.Add(rec.GetRecId());
//		}
//		store->DeleteRecs(ids);
//
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faRdOnly, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//
//		// load and check
//
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		auto res = store->GetAllRecs();
//		EXPECT_EQ(res->GetRecs(), 2 * rec_count / 3);
//
//		TQm::SaveBase2(Base);
//	}
//}

TEST(testTStorePbBlob, PerfCompare_StrAddCache) {
	TStr def_file = ".\\pgblob_test_str1.def";
	TQm::TEnv::Init();
	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
	TStr dir = "data\\";

	// init unicode
	TTmStopWatch sw(false);
	TUnicodeDef::Load(unicode_file);
	int rec_count = 5 * 1000 * 1000;

	printf("Starting performance comparison - insert\n");
	{
		printf("Starting - old implementation\n");
		sw.Reset(true);
		// create new base from definition
		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);

		{
			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
			TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
			PJsonVal json = TJsonVal::GetValFromStr(s);
			PJsonVal json_str = json->GetObjKey("FieldString");
			PJsonVal json_bool = json->GetObjKey("FieldBool");
			for (int i = 0; i < rec_count; i++) {
				//if (i % 1000 == 0) printf("    %d\r", i);
				json_str->PutStr(TStr::Fmt("Stored value %d", i));
				json_bool->PutBool(i % 7 == 3);
				store->AddRec(json);
			}
		}
		printf("\n++++ insert intermediate: %d\n", sw.GetMSecInt());
		TQm::TStorage::SaveBase(Base);
	}
	sw.Stop();
	printf("++++ insert: %d\n\n", sw.GetMSecInt());
	{
		printf("Starting - old implementation - update seq bool\n");
		sw.Reset(true);
		TPt<TQm::TBase> Base = TQm::TStorage::LoadBase(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore");
		auto field_id_str = store->GetFieldId("FieldString");
		auto field_id_bool = store->GetFieldId("FieldBool");
		auto res = store->GetAllRecs();
		for (int i = 0; i < rec_count; i++) {
			TQm::TRec& rec = res->GetRec(i);
			rec.SetFieldBool(field_id_bool, i % 7 == 3);
		}
		printf("\n++++ update intermediate: %d\n", sw.GetMSecInt());
		TQm::TStorage::SaveBase(Base);
	}
	sw.Stop();
	printf("++++ update: %d\n\n", sw.GetMSecInt());
	{
		printf("Starting - old implementation - update seq str\n");
		sw.Reset(true);
		TPt<TQm::TBase> Base = TQm::TStorage::LoadBase(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore");
		auto field_id_str = store->GetFieldId("FieldString");
		auto field_id_bool = store->GetFieldId("FieldBool");
		auto res = store->GetAllRecs();
		for (int i = 0; i < rec_count; i++) {
			TQm::TRec& rec = res->GetRec(i);
			rec.SetFieldStr(field_id_str, TStr::Fmt("Stored value %s %d", (i % 4 == 3 ? "" : "extension"), i));
		}
		printf("\n++++ update intermediate: %d\n", sw.GetMSecInt());
		TQm::TStorage::SaveBase(Base);
	}
	sw.Stop();
	printf("++++ update: %d\n\n", sw.GetMSecInt());
	{
		printf("Starting - new implementation\n");
		sw.Reset(true);
		// create new base from definition
		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);

		{
			TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
			TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
			PJsonVal json = TJsonVal::GetValFromStr(s);
			PJsonVal json_str = json->GetObjKey("FieldString");
			PJsonVal json_bool = json->GetObjKey("FieldBool");
			for (int i = 0; i < rec_count; i++) {
				//if (i % 1000 == 0) printf("    %d\r", i);
				json_str->PutStr(TStr::Fmt("Stored value %d", i));
				json_bool->PutBool(i % 7 == 3);
				store->AddRec(json);
			}
		}
		printf("\n++++ insert intermediate: %d\n", sw.GetMSecInt());
		TQm::SaveBase2(Base);
	}
	sw.Stop();
	printf("++++ %d\n\n", sw.GetMSecInt());
	{
		printf("Starting - new implementation - update seq bool\n");
		sw.Reset(true);
		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore");
		auto field_id_str = store->GetFieldId("FieldString");
		auto field_id_bool = store->GetFieldId("FieldBool");
		auto res = store->GetAllRecs();
		for (int i = 0; i < rec_count; i++) {
			TQm::TRec& rec = res->GetRec(i);
			rec.SetFieldBool(field_id_bool, i % 7 == 3);
		}
		printf("\n++++ update intermediate: %d\n", sw.GetMSecInt());
		TQm::SaveBase2(Base);
	}
	sw.Stop();
	printf("++++ update: %d\n\n", sw.GetMSecInt());
	{
		printf("Starting - new implementation - update seq str\n");
		sw.Reset(true);
		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore");
		auto field_id_str = store->GetFieldId("FieldString");
		auto field_id_bool = store->GetFieldId("FieldBool");
		auto res = store->GetAllRecs();
		for (int i = 0; i < rec_count; i++) {
			TQm::TRec& rec = res->GetRec(i);
			rec.SetFieldStr(field_id_str, TStr::Fmt("Stored value %s %d", (i % 4 == 3 ? "" : "extension"), i));
		}
		printf("\n++++ update intermediate: %d\n", sw.GetMSecInt());
		TQm::SaveBase2(Base);
	}
	sw.Stop();
	printf("++++ update: %d\n\n", sw.GetMSecInt());
}
//
//
//
//TEST(testTStorePbBlob, PerfCompare_StrAddDelete) {
//	TStr def_file = ".\\pgblob_test_str1.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TTmStopWatch sw(false);
//	TUnicodeDef::Load(unicode_file);
//	int loops = 100;
//	int rec_count = 3 * 1000 * 1000;
//	int del_per_loop = rec_count / loops;
//
//	printf("Starting performance comparison - insert and delete\n");
//	{
//		printf("Starting - old implementation - insert and delete\n");
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		for (int i = 0; i < rec_count; i++) {
//			//if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			store->AddRec(json);
//		}
//
//		sw.Reset(true);
//		for (int j = 0; j < loops; j++) {
//			store->DeleteFirstNRecs(del_per_loop);
//		}
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::TStorage::SaveBase(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//
//	{
//		printf("Starting - new implementation - insert and delete\n");
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		TVec<uint64> rec_ids;
//		for (int i = 0; i < rec_count; i++) {
//			//if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			rec_ids.Add(store->AddRec(json));
//		}
//
//		sw.Reset(true);
//		for (int i = 0; i < loops; i++) {
//			TUInt64V rec_ids_to_delete;
//			for (int j = 0; j < del_per_loop; j++) {
//				rec_ids_to_delete.Add(rec_ids[i * del_per_loop + j]);
//			}
//			printf("delete in this loop: %d\n", rec_ids_to_delete.Len());
//			store->DeleteRecs(rec_ids_to_delete);
//		}
//
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::SaveBase2(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//}
//
//
//TEST(testTStorePbBlob, PerfCompare_ReadAll) {
//	TStr def_file = ".\\pgblob_test_str1.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TTmStopWatch sw(false);
//	TUnicodeDef::Load(unicode_file);
//	int loops = 100;
//	int rec_count = 3 * 1000 * 1000;
//	int del_per_loop = rec_count / loops;
//
//	printf("Starting performance comparison - insert and delete\n");
//	{
//		printf("Starting - old implementation - insert and delete\n");
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			store->AddRec(json);
//		}
//
//		sw.Reset(true);
//		auto rs = store->GetAllRecs();
//		int max_len = -1;
//		for (int j = 0; j < rs->GetRecs(); j++) {
//			auto rec = rs->GetRec(j);
//			max_len = MAX(max_len, rec.GetFieldStr(field_id_str).Len());
//		}
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		printf("%d\n", max_len);
//		TQm::TStorage::SaveBase(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//
//	{
//		printf("Starting - new implementation - insert and delete\n");
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		TVec<uint64> rec_ids;
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			rec_ids.Add(store->AddRec(json));
//		}
//
//		sw.Reset(true);
//		auto rs = store->GetAllRecs();
//		int max_len = -1;
//		for (int j = 0; j < rs->GetRecs(); j++) {
//			auto rec = rs->GetRec(j);
//			max_len = MAX(max_len, rec.GetFieldStr(field_id_str).Len());
//		}
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		printf("%d\n", max_len);
//		TQm::SaveBase2(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//}
//
//
//TEST(testTStorePbBlob, PerfCompare_DelAll) {
//	TStr def_file = ".\\pgblob_test_str1.def";
//	TQm::TEnv::Init();
//	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
//	TStr dir = "data\\";
//
//	// init unicode
//	TTmStopWatch sw(false);
//	TUnicodeDef::Load(unicode_file);
//	int loops = 100;
//	int rec_count = 6 * 1000 * 1000;
//	int del_per_loop = rec_count / loops;
//
//	printf("Starting performance comparison - insert and delete\n");
//	{
//		printf("Starting - old implementation - insert and delete\n");
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			store->AddRec(json);
//		}
//		TQm::TStorage::SaveBase(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::TStorage::LoadBase(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		sw.Reset(true);
//		store->DeleteAllRecs();
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::TStorage::SaveBase(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//
//	printf("Starting performance comparison - insert and delete\n");
//	{
//		printf("Starting - old implementation - insert and delete\n");
//		// create new base from definition
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		for (int i = 0; i < rec_count; i++) {
//			if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			store->AddRec(json);
//		}
//		TQm::TStorage::SaveBase(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::TStorage::LoadBase(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto rs = store->GetAllRecs();
//		int max_len = -1;
//		for (int j = 0; j < rs->GetRecs(); j++) {
//			auto rec = rs->GetRec(j);
//			max_len = MAX(max_len, rec.GetFieldStr(field_id_str).Len());
//		}
//
//		sw.Reset(true);
//		store->DeleteAllRecs();
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::TStorage::SaveBase(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//
//	/////////////////
//
//
//	{
//		printf("Starting - new implementation - insert and delete\n");
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		TVec<uint64> rec_ids;
//		for (int i = 0; i < rec_count; i++) {
//			//if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			rec_ids.Add(store->AddRec(json));
//		}
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		sw.Reset(true);
//		store->DeleteAllRecs();
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::SaveBase2(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//
//	{
//		printf("Starting - new implementation - insert and delete\n");
//		PJsonVal SchemaVal = TJsonVal::GetValFromStr(TStr::LoadTxt(def_file));
//		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//		auto field_id_bool = store->GetFieldId("FieldBool");
//		TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
//		PJsonVal json = TJsonVal::GetValFromStr(s);
//		PJsonVal json_str = json->GetObjKey("FieldString");
//		PJsonVal json_bool = json->GetObjKey("FieldBool");
//		TVec<uint64> rec_ids;
//		for (int i = 0; i < rec_count; i++) {
//			//if (i % 1000 == 0) printf("    %d\r", i);
//			json_str->PutStr(TStr::Fmt("Stored value %d", i));
//			json_bool->PutBool(i % 7 == 3);
//			rec_ids.Add(store->AddRec(json));
//		}
//		TQm::SaveBase2(Base);
//	}
//	{
//		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
//		auto store = Base->GetStoreByStoreNm("TestStore");
//		auto field_id_str = store->GetFieldId("FieldString");
//
//		printf("Loading all records...\n", sw.GetMSecInt());
//		auto rs = store->GetAllRecs();
//		int max_len = -1;
//		for (int j = 0; j < rs->GetRecs(); j++) {
//			auto rec = rs->GetRec(j);
//			max_len = MAX(max_len, rec.GetFieldStr(field_id_str).Len());
//		}
//		printf("Starting delete...\n", sw.GetMSecInt());
//		sw.Reset(true);
//		store->DeleteAllRecs();
//		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
//		TQm::SaveBase2(Base);
//	}
//	sw.Stop();
//	printf("++++ loop: %d\n\n", sw.GetMSecInt());
//}


TEST(testTStorePbBlob, PerfCompare_StrAddDelete2) {
	TStr def_file = ".\\pgblob_test_str2.def";
	TQm::TEnv::Init();
	TStr unicode_file = "..\\..\\..\\..\\..\\src\\glib\\bin\\UnicodeDef.Bin";
	TStr dir = "data\\";

	// init unicode
	TTmStopWatch sw(false);
	TUnicodeDef::Load(unicode_file);
	int loops = 100;
	int rec_count = 3 * 1000 * 1000;
	int del_per_loop = rec_count / loops;
	TStr definition = TStr::LoadTxt(def_file);
	PJsonVal SchemaVal = TJsonVal::GetValFromStr(definition);
	TStr s = "{ \"FieldString1\" : \"xxx\", \"FieldString2\" : \"xxx\", \"FieldString3\" : \"xxx\", \"FieldString4\" : \"xxx\", \"FieldString5\" : \"xxx\", \"FieldBool\" : true}";
	PJsonVal json = TJsonVal::GetValFromStr(s);
	PJsonVal json_str1 = json->GetObjKey("FieldString1");
	PJsonVal json_str2 = json->GetObjKey("FieldString2");
	PJsonVal json_str3 = json->GetObjKey("FieldString3");
	PJsonVal json_str4 = json->GetObjKey("FieldString4");
	PJsonVal json_str5 = json->GetObjKey("FieldString5");
	PJsonVal json_bool = json->GetObjKey("FieldBool");

	printf("Starting performance comparison - insert and delete\n");
	{
		printf("Starting - old implementation - insert and delete\n");

		TPt<TQm::TBase> Base = TQm::TStorage::NewBase(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		TWPt<TQm::TStore> store = Base->GetStoreByStoreNm("TestStore2");
		for (int i = 0; i < rec_count; i++) {
			if (i % 10000 == 0) printf("    %d\r", i);
			json_str1->PutStr(TStr::Fmt("Stored value %d", i));
			json_str2->PutStr(definition.GetSubStr(i % 8, 20 + i % 10));
			json_str3->PutStr(definition.GetSubStr(i % 8, 20 + i % 10));
			json_str4->PutStr(TStr::Fmt("c%d", i % 7));
			json_str5->PutStr(TStr::Fmt("d%d", i % 4));
			json_bool->PutBool(i % 7 == 3);
			store->AddRec(json);
		}

		sw.Reset(true);
		for (int j = 0; j < loops; j++) {
			store->DeleteFirstNRecs(del_per_loop);
		}
		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
		TQm::TStorage::SaveBase(Base);
	}
	sw.Stop();
	printf("++++ loop: %d\n\n", sw.GetMSecInt());

	{
		printf("Starting - new implementation - insert and delete\n");
		TPt<TQm::TBase> Base = TQm::NewBase2(dir, SchemaVal, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore2");
		//auto field_id_str = store->GetFieldId("FieldString");
		//auto field_id_bool = store->GetFieldId("FieldBool");
		//TStr s = "{ \"FieldString\" : \"xxx\", \"FieldBool\" : true}";
		//PJsonVal json = TJsonVal::GetValFromStr(s);
		//PJsonVal json_str = json->GetObjKey("FieldString");
		//PJsonVal json_bool = json->GetObjKey("FieldBool");
		//TVec<uint64> rec_ids;
		for (int i = 0; i < rec_count; i++) {
			if (i % 10000 == 0) printf("    %d\r", i);
			json_str1->PutStr(TStr::Fmt("Stored value %d", i));
			json_str2->PutStr(definition.GetSubStr(i % 8, 20 + i % 10));
			json_str3->PutStr(definition.GetSubStr(i % 8, 20 + i % 10));
			json_str4->PutStr(TStr::Fmt("c%d", i % 7));
			json_str5->PutStr(TStr::Fmt("d%d", i % 4));
			json_bool->PutBool(i % 7 == 3);
			store->AddRec(json);
		}
		TQm::SaveBase2(Base);
	}
	TUInt64V rec_ids;
	{
		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore2");
		auto rs = store->GetAllRecs();
		rs->GetRecIdV(rec_ids);
		TQm::SaveBase2(Base);
	}
	{
		TPt<TQm::TBase> Base = TQm::LoadBase2(dir, TFAccess::faUpdate, 2 * 1024 * 1024, 2 * 1024 * 1024, TStrUInt64H(), true, 4 * TInt::Kilo);
		auto store = Base->GetStoreByStoreNm("TestStore2");
		sw.Reset(true);
		for (int i = 0; i < loops; i++) {
			TUInt64V rec_ids_to_delete;
			for (int j = 0; j < del_per_loop; j++) {
				rec_ids_to_delete.Add(rec_ids[i * del_per_loop + j]);
			}
			printf("delete in this loop: %d\n", rec_ids_to_delete.Len());
			store->DeleteRecs(rec_ids_to_delete);
		}

		printf("\n++++ loop intermediate: %d\n", sw.GetMSecInt());
		TQm::SaveBase2(Base);
	}
	sw.Stop();
	printf("++++ loop: %d\n\n", sw.GetMSecInt());
}

/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#define XTEST

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <base.h>
#include <mine.h>

#include <iostream>
#include <windows.h>

#include "gtest/gtest.h"
#include <qminer.h>


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
// this class must be named XTest, because it is declared as friend in
// source classes (wrapped in XTEST preprocessor directive)
class XTest {
public:

	//////////////////////////////////////////////////////////////////////////////////////
	// TInMemStorage

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

	/////////////////////////////////////////////////////////////////////////////
	// TPgBlob tests

	/////////////////////////////////////

	static void TPgBlob_Complex1() {
		auto Base = TPgBlob::Create("data\\xyz");
		TPgBlobPgPt Pt;
		char* BfPt;
		Base->CreateNewPage(Pt, &BfPt);
		EXPECT_EQ(Pt.GetFIx(), 0);
		EXPECT_EQ(Pt.GetPg(), 0);
		Base->CreateNewPage(Pt, &BfPt);
		EXPECT_EQ(Pt.GetFIx(), 0);
		EXPECT_EQ(Pt.GetPg(), 1);
	}

	static void TPgBlob_Page_Init() {
		char* bf = new char[PG_PAGE_SIZE];

		TPgBlob::InitPageP(bf);
		auto header = (TPgBlob::TPgHeader*)bf;

		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true); // new page is not saved yet
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 0);
		EXPECT_EQ(header->OffsetFreeStart, 10);
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE);

		delete[] bf;
	}

	static void TPgBlob_Page_AddInt() {
		char* bf = new char[PG_PAGE_SIZE];
		int data = 8765;

		TPgBlob::InitPageP(bf);

		// add value
		auto res = TPgBlob::AddItem(bf, (char*)&data, sizeof(int));
		EXPECT_EQ(res, 0);

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 4);

		// retrieve value
		auto rec = TPgBlob::GetItemRec(bf, res);
		int* b = (int*)(bf + rec->Offset);
		EXPECT_EQ(*b, data);

		delete[] bf;
	}

	static void TPgBlob_Page_AddDouble() {
		char* bf = new char[PG_PAGE_SIZE];
		double data = 8765.4321;

		TPgBlob::InitPageP(bf);

		// add value
		auto res = TPgBlob::AddItem(bf, (char*)&data, sizeof(double));
		EXPECT_EQ(res, 0);

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 8);

		// retrieve value
		auto rec = TPgBlob::GetItemRec(bf, res);
		double* b = (double*)(bf + rec->Offset);
		EXPECT_EQ(*b, data);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveral() {
		char* bf = new char[PG_PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		TPgBlob::InitPageP(bf);

		// add value
		auto res1 = TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));
		EXPECT_EQ(res1, 0);
		EXPECT_EQ(res2, 1);
		EXPECT_EQ(res3, 2);

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 3 * 4);

		// retrieve values
		auto rec1 = TPgBlob::GetItemRec(bf, res1);
		int* b1 = (int*)(bf + rec1->Offset);
		EXPECT_EQ(*b1, data1);
		auto rec2 = TPgBlob::GetItemRec(bf, res2);
		int* b2 = (int*)(bf + rec2->Offset);
		EXPECT_EQ(*b2, data2);
		auto rec3 = TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntMany() {
		char* bf = new char[PG_PAGE_SIZE];
		int data1 = 5;

		TPgBlob::InitPageP(bf);

		// add values
		for (int i = 0; i < 1000; i++) {
			TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
			data1 += 3;
		}

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 1000);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 1000 * 4); // item record
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 1000 * 4);

		// retrieve values
		data1 = 5;
		for (int i = 0; i < 1000; i++) {
			auto rec1 = TPgBlob::GetItemRec(bf, i);
			int* b1 = (int*)(bf + rec1->Offset);
			EXPECT_EQ(*b1, data1);
			data1 += 3;
		}

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveralDelete() {
		char* bf = new char[PG_PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		TPgBlob::InitPageP(bf);

		// add value
		auto res1 = TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));

		TPgBlob::DeleteItem(bf, res2);

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // 3 items
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 2 * 4); // 2 actually contain data

															 // retrieve values
		auto rec1 = TPgBlob::GetItemRec(bf, res1);
		int* b1 = (int*)(bf + rec1->Offset);
		EXPECT_EQ(*b1, data1);
		auto rec3 = TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}

	static void TPgBlob_Page_AddIntSeveralDelete2() {
		char* bf = new char[PG_PAGE_SIZE];
		int data1 = 8765;
		int data2 = 77;
		int data3 = 91826;

		TPgBlob::InitPageP(bf);

		// add value
		auto res1 = TPgBlob::AddItem(bf, (char*)&data1, sizeof(int));
		auto res2 = TPgBlob::AddItem(bf, (char*)&data2, sizeof(int));
		auto res3 = TPgBlob::AddItem(bf, (char*)&data3, sizeof(int));

		TPgBlob::DeleteItem(bf, res1);

		// check internal state
		auto header = (TPgBlob::TPgHeader*)bf;
		EXPECT_EQ(header->PageSize, PG_PAGE_SIZE);
		EXPECT_EQ(header->IsDirty(), true);
		EXPECT_EQ(header->IsLock(), false);
		EXPECT_EQ(header->ItemCount, 3);
		EXPECT_EQ(header->OffsetFreeStart, 10 + 3 * 4); // 3 items
		EXPECT_EQ(header->OffsetFreeEnd, PG_PAGE_SIZE - 2 * 4); // 2 actually contain data

															 // retrieve values
		auto rec2 = TPgBlob::GetItemRec(bf, res2);
		int* b2 = (int*)(bf + rec2->Offset);
		EXPECT_EQ(*b2, data2);
		auto rec3 = TPgBlob::GetItemRec(bf, res3);
		int* b3 = (int*)(bf + rec3->Offset);
		EXPECT_EQ(*b3, data3);

		delete[] bf;
	}

	static void TPgBlob_AddBf1() {

		double d1 = 65.43;
		double d2 = 111234.7;
		int i1 = 89;
		TFlt tmp = 0;
		TInt tmp3 = 0;

		TPgBlob pb("data/pb", TFAccess::faCreate, 4194304);
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
		auto pg_item = (TPgBlob::TPgBlobPageItem*)
			(pb.GetPageBf(0) + sizeof(TPgBlob::TPgHeader));
		EXPECT_EQ(pg_item->Len, 4);
		EXPECT_EQ(pg_item->Offset, 8180);
		pg_item++;
		EXPECT_EQ(pg_item->Len, 8);
		EXPECT_EQ(pg_item->Offset, 8184);
	}

	//////////////

	static void TBinTreeMaxVals_Add1() {
		TBinTreeMaxVals Vals;
		TPgBlobPgPt Pt(0, 0);
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
		EXPECT_EQ(Vals.GetIndexOfMax(), 6);
		Vals.Change(6, 2);
		EXPECT_EQ(Vals.GetIndexOfMax(), 3);
		Vals.Change(3, 3);
		EXPECT_EQ(Vals.GetIndexOfMax(), 5);
	}

	/////////////////////////////////////////////////////////////////////////////
	// TGix tests

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
		int loops = 200 * 1000;
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

	// add and remove items from the index. assert that we have at the end only the items not deleted
	static void Test_AddDeleteOne() {
		PMyGix Gix = TMyGix::New("GixAddAndDelete", "", TFAccess::faCreate, 1 * TInt::Kilo, 16, true);
		TRnd Rnd;
		TMyKey Key(0,0);
		THash<TUInt64, TInt> KeyToValH;
		for (int ItemN = 0; ItemN < 500; ItemN += 1) {
			const TUInt64 Item = ItemN * 10 + Rnd.GetUniDevInt(100);
			KeyToValH.AddDat(Item) += 1;
			Gix->AddItem(Key, TMyItem(Item, 1));
		}

		for (int N = 0, KeyId = KeyToValH.FFirstKeyId(); N < 200 && KeyToValH.FNextKeyId(KeyId); N++) {
			const TUInt64 Item = KeyToValH.GetKey(KeyId);
			Gix->AddItem(Key, TMyItem(Item, -1));
			KeyToValH.AddDat(Item) += -1;
		}

    PMyItemSet Set = Gix->GetItemSet((TMyKey) Key);
		Set->Def();
		const int Items = Set->GetItems();
		for (int ItemN = 0; ItemN < Items; ItemN++) {
			TMyItem Item = Set->GetItem(ItemN);
			TInt Val = KeyToValH.GetDat(Item.Key);
			ASSERT_TRUE(Val.Val == Item.Dat.Val);
		}
	}

  static void Test_AddDeleteRandom() {
    PMyGix Gix = TMyGix::New("GixAddAndDelete", "", TFAccess::faCreate, 1 * TInt::Kilo, 16, true);
    TRnd Rnd;
    TMyKey Key(0, 0);
	THash<TUInt64, TInt> KeyToValH;
	for (int ItemN = 0; ItemN < 5000; ItemN += 1) {
		const TUInt64 Item = ItemN + Rnd.GetUniDevInt(3);
		int Val = 0;
		// if we have this key, then we randomly choose whether to add or remove the value
		if (KeyToValH.IsKey(Item)) {
			if (Rnd.GetUniDevInt(2) == 0)
				Val = Rnd.GetUniDevInt(5);
			else
				Val = -KeyToValH.GetDat(Item);
		}
		// otherwise we always add
		else {
			Val = Rnd.GetUniDevInt(5);
		}
		KeyToValH.AddDat(Item) += Val;
		Gix->AddItem(Key, TMyItem(Item, Val));
	}

	PMyItemSet Set = Gix->GetItemSet((TMyKey) Key);
	Set->Def();
	const int Items = Set->GetItems();
	for (int ItemN = 0; ItemN < Items; ItemN++) {
		TMyItem Item = Set->GetItem(ItemN);
		TInt Val = KeyToValH.GetDat(Item.Key);
		ASSERT_TRUE(Val.Val == Item.Dat.Val);
	}
  }

  static void Test_AddDeleteAll() {
	  PMyGix Gix = TMyGix::New("GixAddAndDelete", "", TFAccess::faCreate, 1 * TInt::Kilo, 16, true);
	  TRnd Rnd;
	  TMyKey Key(0, 0);
	  THash<TUInt64, TInt> KeyToValH;
	  for (int ItemN = 0; ItemN < 500; ItemN += 1) {
		  Gix->AddItem(Key, TMyItem(ItemN, 1));
	  }

	  for (int ItemN = 0; ItemN < 500; ItemN += 1) {
		  Gix->DelItem(Key, TMyItem(ItemN, 1));
	  }

	  PMyItemSet Set = Gix->GetItemSet((TMyKey) Key);
	  Set->Def();
	  ASSERT_TRUE(Set->GetItems() == 0);
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

	static void Test_ReadOnlyAfterCrash() {
		TStr Nm = "Test1";
		TStr Path = "data";
		{
			// create index and add single item
			TMyGix gix(Nm, Path, faCreate, 10000, 100);
			for (int i = 0; i < 100; i++) {
				auto key = TIntUInt64Pr(i, i);
				gix.AddItem(key, TMyItem(7234, 1));
				// now close it when it goes out of scoope
			}
		}
		{
			// now open in read-only mode as pointer
			auto gix = new TMyGix(Nm, Path, faRdOnly, 10000, 100);
			auto key = TIntUInt64Pr(12, 12);
			auto itemset = gix->GetItemSet(key);
			// don't delete, this simulates the crash
		}
		{
			// now open again in read-only mode
			// this simulates situation when it was opened read-only
			// and the system crashed
			TMyGix gix2(Nm, Path, faRdOnly, 10000, 100);
			auto key2 = TIntUInt64Pr(12, 12);
			auto itemset2 = gix2.GetItemSet(key2);
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// Unit tests to be executed
//////////////////////////////////////////////////////////////////////////

// The fixture for testing class TGix.
class testTGix : public ::testing::Test {
protected:

	// You can remove any or all of the following functions if its body
	// is empty.

	testTGix() {
		// You can do set-up work for each test here.
		TStr DataDir = "data";
		if (!TDir::Exists(DataDir)) {
			TDir::GenDir(DataDir);
		}
	}

	virtual ~testTGix() {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp() {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	virtual void TearDown() {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}

};

TEST(testTInMemStorage, Simple1) { XTest::TInMemStorage_Simple1(); }
TEST(testTInMemStorage, Lazy1) { XTest::TInMemStorage_Lazy1(); }
TEST(testTInMemStorage, Complex1) { XTest::TInMemStorage_Complex1(); }
TEST(testTInMemStorage, LoadAll1) { XTest::TInMemStorage_LoadAll1(); }
TEST(testTInMemStorage, LoadAll2) { XTest::TInMemStorage_LoadAll2(); }
TEST(testTInMemStorage, Delete1) { XTest::TInMemStorage_Delete1(); }
TEST(testTInMemStorage, Delete2) { XTest::TInMemStorage_Delete2(); }
TEST(testTInMemStorage, SetVal) { XTest::TInMemStorage_SetVal(); }
//TEST(testTInMemStorage, PerfTest) { XTest::TInMemStorage_PerfTest(); }

TEST(testTPgBlob, Simple) { XTest::TPgBlob_Complex1(); }
TEST(testTPgBlob, PageInit) { XTest::TPgBlob_Page_Init(); }
TEST(testTPgBlob, PageAddInt) { XTest::TPgBlob_Page_AddInt(); }
TEST(testTPgBlob, PageAddIntMany) { XTest::TPgBlob_Page_AddIntMany(); }
TEST(testTPgBlob, PageAddDouble) { XTest::TPgBlob_Page_AddDouble(); }
TEST(testTPgBlob, PageAddIntSeveral) { XTest::TPgBlob_Page_AddIntSeveral(); }
TEST(testTPgBlob, PageAddIntSeveralDelete) { XTest::TPgBlob_Page_AddIntSeveralDelete(); }
TEST(testTPgBlob, PageAddIntSeveralDelete2) { XTest::TPgBlob_Page_AddIntSeveralDelete2(); }
TEST(testTPgBlob, AddBf1) { XTest::TPgBlob_AddBf1(); }
TEST(TBinTreeMaxVals, Add1) { XTest::TBinTreeMaxVals_Add1(); }

TEST_F(testTGix, Simple10) { XTest::Test_Simple_1(); }
TEST_F(testTGix, Simple200) { XTest::Test_Simple_220(); }
TEST_F(testTGix, Simple220Unsorted) { XTest::Test_Simple_220_Unsorted(); }
TEST_F(testTGix, Merge220Into50) { XTest::Test_Merge_220_Into_50(); }
TEST_F(testTGix, Merge220Into120) { XTest::Test_Merge_220_Into_120(); }
TEST_F(testTGix, Merge22000Into50) { XTest::Test_Merge_22000_Into_50(); }
TEST_F(testTGix, Delete1) { XTest::Test_Delete_1(); }
TEST_F(testTGix, Delete20) { XTest::Test_Delete_20(); }
TEST_F(testTGix, Delete20And1) { XTest::Test_Delete_20And1(); }
TEST_F(testTGix, Delete120) { XTest::Test_Delete_120(); }
TEST_F(testTGix, Delete120And1) { XTest::Test_Delete_120And1(); }
TEST_F(testTGix, Delete120And110) { XTest::Test_Delete_120And110(); }
TEST_F(testTGix, Delete22000And1000) { XTest::Test_Delete_22000And1000(); }
TEST_F(testTGix, QuasiDelete120And1And2) { XTest::Test_QuasiDelete_120And1And2(); }
TEST_F(testTGix, QuasiDelete120And20) { XTest::Test_QuasiDelete_120And20(); }
TEST_F(testTGix, QuasiDelete22000And1000) { XTest::Test_QuasiDelete_22000And1000(); }
TEST_F(testTGix, Test_AddDeleteOne) { XTest::Test_AddDeleteOne(); }
TEST_F(testTGix, Test_AddDeleteRandom) { XTest::Test_AddDeleteRandom(); }
TEST_F(testTGix, Test_AddDeleteAll) { XTest::Test_AddDeleteAll(); }

//////////////////////////////////////////////////////////////////////////
// Tests of online variance calculator

TEST(testNumericFeatureAggr, TVar) {
	TSignalProc::TVarSimple Var;

	Var.Update(1);
	Var.Update(2);
	Var.Update(3);

	EXPECT_EQ(Var.GetMean(), 2);
	EXPECT_EQ(Var.GetVar(), 1);

	Var.Update(4);
	Var.Update(5);
	Var.Update(6);
	Var.Update(7);
	Var.Update(8);
	Var.Update(9);
	Var.Update(10);

	EXPECT_EQ(Var.GetMean(), 5.5);
	EXPECT_EQ(round(1000000 * Var.GetVar()), 9166667);
}

TEST(testTFtrGen, TNumeric) {
	TFtrGen::TNumeric Num(true, true);

	Num.Update(1);
	Num.Update(2);
	Num.Update(3);

	EXPECT_EQ(Num.GetFtr(2), 0);
	EXPECT_EQ(Num.GetFtr(3), 1);
	EXPECT_EQ(Num.GetFtr(1), -1);
}


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
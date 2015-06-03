
#include "pgblob.h"

namespace glib {

	///////////////////////////////////////////////////////////////////////////

	/// Assignment operator
	TPgBlobPgPt& TPgBlobPgPt::operator=(const TPgBlobPgPt& Pt) {
		if (this != &Pt) {
			Page = Pt.Page;
			FileIndex = Pt.FileIndex;
		}
		return *this;
	}

	/// Equality comparer
	bool TPgBlobPgPt::operator==(const TPgBlobPgPt& Pt) const {
		return (Page == Pt.Page) && (FileIndex == Pt.FileIndex);
	}

	/// Comparison of pointers for sorting
	bool TPgBlobPgPt::operator<(const TPgBlobPgPt& Pt) const {
		return
			(FileIndex < Pt.FileIndex) ||
			((FileIndex == Pt.FileIndex) && (Page < Pt.Page));
	}

	/// set all values
	void TPgBlobPgPt::Set(int16 fi, uint32 pg) {
		Page = pg;
		FileIndex = fi;
	}

	/// for TPgBlobPgPt into THash
	int TPgBlobPgPt::GetPrimHashCd() const {
		return abs(int(Page) + FileIndex);
	}

	/// for insertion into THash
	int TPgBlobPgPt::GetSecHashCd() const {
		return abs(int(Page)) + int(FileIndex) * 0x10;
	}

	TPgBlobPgPt::TPgBlobPgPt(const TPgBlobPt& Src) { 
		Set(Src.GetFIx(), Src.GetPg()); 
	}

	///////////////////////////////////////////////////////////////////////////

	/// Assignment operator
	TPgBlobPt& TPgBlobPt::operator=(const TPgBlobPt& Pt) {
		if (this != &Pt) {
			Page = Pt.Page;
			FileIndex = Pt.FileIndex;
			ItemIndex = Pt.ItemIndex;
		}
		return *this;
	}

	/// Equality comparer
	bool TPgBlobPt::operator==(const TPgBlobPt& Pt) const {
		return
			(Page == Pt.Page) &&
			(FileIndex == Pt.FileIndex) &&
			(ItemIndex == Pt.ItemIndex);
	}

	/// Comparison of pointers for sorting
	bool TPgBlobPt::operator<(const TPgBlobPt& Pt) const {
		return
			(FileIndex < Pt.FileIndex) ||
			((FileIndex == Pt.FileIndex) && (Page < Pt.Page)) ||
			((FileIndex == Pt.FileIndex) && (Page == Pt.Page) && (ItemIndex < Pt.ItemIndex));
	}

	/// set all values
	void TPgBlobPt::Set(int16 fi, uint32 pg, uint16 ii) {
		Page = pg;
		FileIndex = fi;
		ItemIndex = ii;
	}

	/// for insertion into THash
	int TPgBlobPt::GetPrimHashCd() const {
		return abs(int(Page) + ItemIndex + FileIndex);
	}

	/// for insertion into THash
	int TPgBlobPt::GetSecHashCd() const {
		return abs(int(Page)) + int(ItemIndex) * 0x10 + int(FileIndex) * 0x100;
	}

	///////////////////////////////////////////////////////////////////////////

	/// Private constructor
	TPgBlobFile::TPgBlobFile(
		const TStr& _FNm, const TFAccess& _Access, const uint32& _MxSegLen) {

		Access = _Access;
		FNm = _FNm;
		MxFileLen = _MxSegLen*PAGE_SIZE;

		switch (Access) {
		case faCreate:
			FileId = fopen(FNm.CStr(), "w+b");
			break;
		case faRdOnly:
			FileId = fopen(FNm.CStr(), "rb");
			break;
		case faAppend:
			FileId = fopen(FNm.CStr(), "r+b");
			if (FileId != NULL) {
				fseek(FileId, SEEK_END, 0);
			}
			break;
		case faUpdate:
			FileId = fopen(FNm.CStr(), "r+b");
			break;
		default:
			break;
		}
	}

	/// Destructor
	TPgBlobFile::~TPgBlobFile() {
		EAssertR(
			fclose(FileId) == 0,
			"Can not close file '" + TStr(FNm.CStr()) + "'.");
	}

	/// Load page with given index from the file into buffer
	int TPgBlobFile::LoadPage(const uint32& Page, void* Bf) {
		SetFPos(Page * PAGE_SIZE);
		EAssertR(
			fread(Bf, 1, PAGE_SIZE, FileId) == PAGE_SIZE,
			"Error reading file '" + TStr(FNm) + "'.");
		return 0;
	}

	/// Save buffer to page within the file 
	int TPgBlobFile::SavePage(const uint32& Page, const void* Bf) {
		SetFPos(Page * PAGE_SIZE);
		EAssertR(
			fwrite(Bf,1,  PAGE_SIZE, FileId) == PAGE_SIZE,
			"Error writing file '" + TStr(FNm) + "'.");
		return 0;
	}

	/// Refresh the position - internal check
	void TPgBlobFile::RefreshFPos() {
		EAssertR(
			fseek(FileId, 0, SEEK_CUR) == 0,
			"Error seeking into file '" + TStr(FNm) + "'.");
	}

	/// Set position in the file
	void TPgBlobFile::SetFPos(const int& FPos) {
		EAssertR(
			fseek(FileId, FPos, SEEK_SET) == 0,
			"Error seeking into file '" + TStr(FNm) + "'.");
	}

	/// Reserve new space in the file. Returns -1 if file is full.
	uint32 TPgBlobFile::CreateNewPage() {
		EAssertR(
			fseek(FileId, 0, SEEK_END) == 0,
			"Error seeking into file '" + TStr(FNm) + "'.");
		long len = ftell(FileId);
		if (MxFileLen > 0 && len >= MxFileLen) {
			return -1;
		}
		// write to the end of file - take what-ever chunk of memory
		char tc = 0;
		EAssertR(
			fwrite(&tc, 1, PAGE_SIZE, FileId) == PAGE_SIZE,
			"Error writing file '" + TStr(FNm) + "'.");
		return (uint32)(len / PAGE_SIZE);
	}

	///////////////////////////////////////////////////////////////////////////

	/// Add given buffer to page, to existing item that has length 0
	void TPgBlob::ChangeItem(
		// TODO locks?
		byte* Pg, uint16 ItemIndex, const byte* Bf, const int BfL) {

		TPgHeader* Header = (TPgHeader*)Pg;
		EAssert(BfL + sizeof(TPgBlobPageItem) <= Header->GetFreeMem());

		uint16 res = Header->ItemCount;
		TPgBlobPageItem* NewItem = GetItemRec(Pg, ItemIndex);
		EAssert(NewItem->Len == 0);
		NewItem->Len = BfL;
		NewItem->Offset = Header->OffsetFreeEnd - BfL;

		if (Bf != NULL) { // only copy when there is some data
			memcpy(Pg + NewItem->Offset, Bf, BfL);
		}

		Header->OffsetFreeEnd -= BfL;
		Header->SetDirty(true);
	}

	/// Add given buffer to page, return item-index
	uint16 TPgBlob::AddItem(byte* Pg, const byte* Bf, const int BfL) {
		// TODO locks?
		TPgHeader* Header = (TPgHeader*)Pg;
		EAssert(Header->CanStoreBf(BfL));

		uint16 res = Header->ItemCount;
		TPgBlobPageItem* NewItem = GetItemRec(Pg, Header->ItemCount);
		NewItem->Len = BfL;
		NewItem->Offset = Header->OffsetFreeEnd - BfL;

		if (Bf != NULL) { // only copy when there is some data
			memcpy(Pg + NewItem->Offset, Bf, BfL);
		}

		Header->ItemCount++;
		Header->OffsetFreeEnd -= BfL;
		Header->OffsetFreeStart += sizeof(TPgBlobPageItem);

		Header->SetDirty(true);
		return res;
	}

	/// Retrieve buffer from specified page
	void TPgBlob::GetItem(byte* Pg, uint16 ItemIndex, byte** Bf, int& BfL) {
		// TODO locks?
		TPgBlobPageItem* Item = GetItemRec(Pg, ItemIndex);
		BfL = Item->Len;
		*Bf = Pg + Item->Offset;
	}

	/// Delete buffer from specified page
	void TPgBlob::DeleteItem(byte* Pg, uint16 ItemIndex) {
		// TODO locks?

		TPgBlobPageItem* Item = GetItemRec(Pg, ItemIndex);
		int PackOffset = Item->Len;
		TPgHeader* Header = (TPgHeader*)Pg;
		byte* OldFreeEnd = Pg + Header->OffsetFreeEnd;
		int Len = 0;
		for (int i = ItemIndex + 1; i < Header->ItemCount; i++) {
			TPgBlobPageItem* ItemX = GetItemRec(Pg, i);
			if (ItemX->Len == 0) {
				continue;
			}
			Len += ItemX->Len;
			ItemX->Offset += PackOffset;
		}
		Header->OffsetFreeEnd += PackOffset;
		memmove(OldFreeEnd + PackOffset, OldFreeEnd, Len);

		Item->Len = 0;
		Header->SetDirty(true);
	}

	/// Get pointer to item record - in it are offset and length
	TPgBlob::TPgBlobPageItem* TPgBlob::GetItemRec(
		byte* Pg, uint16 ItemIndex) {
		// TODO locks?
		return (TPgBlob::TPgBlobPageItem*)(
			Pg
			+ sizeof(TPgBlob::TPgHeader)
			+ ItemIndex * sizeof(TPgBlobPageItem));
	}

	/// Private constructor
	TPgBlob::TPgBlob(const TStr& _FNm, const TFAccess& _Access,
		const uint64& CacheSize) {

		EAssertR(CacheSize >= PAGE_SIZE, "Invalid cache size for TPgBlob.");

		FNm = _FNm;
		Access = _Access;

		switch (Access) {
		case faCreate:
			TFile::DelWc(FNm + ".*");
			SaveMain();
			break;
		case faRdOnly:
		case faUpdate:
			LoadMain();
			break;
		default:
			FailR("Unsupported TFAccess flag for TPgBlob.");
		}

		// init cache
		Bf = new byte[CacheSize];
		BfL = CacheSize;
		MxLoadedPages = CacheSize / PAGE_SIZE;
		LruFirst = LruLast = -1;
	}

	/// Destructor
	TPgBlob::~TPgBlob() {
		for (int i = 0; i < LoadedPages.Len(); i++) {
			if (ShouldSavePage(i)) {
				LoadedPage& a = LoadedPages[i];
				Files[a.Pt.GetFIx()]->SavePage(a.Pt.GetPg(), GetPageBf(i));
			}
		}
		SaveMain();
		Files.Clr();
		delete[] Bf;
	}

	/// Save main file
	void TPgBlob::SaveMain() {
		TFOut SOut(FNm + ".main");
		TInt children_cnt(Files.Len());
		children_cnt.Save(SOut);
		Fsm.Save(SOut);
	}

	/// Load main file
	void TPgBlob::LoadMain() {
		TFIn SIn(FNm + ".main");
		TInt children_cnt;
		children_cnt.Load(SIn);
		Fsm.Load(SIn);
		Files.Clr();
		for (int i = 0; i < children_cnt; i++) {
			TStr FNmChild = FNm + ".bin" + TStr::GetNrNumFExt(i);
			Files.Add(TPgBlobFile::New(FNmChild, Access, TInt::Giga));
		}
	}

	/// remove given page from LRU list
	void TPgBlob::UnlistFromLru(int Pg) {
		LoadedPage& a = LoadedPages[Pg];
		if (LruFirst == Pg) {
			LruFirst = a.LruNext;
		}
		if (LruLast == Pg) {
			LruLast = a.LruPrev;
		}
		if (a.LruNext >= 0) {
			LoadedPages[a.LruNext].LruPrev = a.LruPrev;
		}
		if (a.LruPrev >= 0) {
			LoadedPages[a.LruPrev].LruNext = a.LruNext;
		}
	}

	/// insert given (new) page to the start of LRU list
	void TPgBlob::EnlistToStartLru(int Pg) {
		LoadedPage& a = LoadedPages[Pg];
		a.LruPrev = -1;
		a.LruNext = LruFirst;
		if (LruFirst >= 0) {
			LoadedPages[LruFirst].LruPrev = Pg;
		}
		LruFirst = Pg;
	}

	/// move given page to the start of LRU list
	void TPgBlob::MoveToStartLru(int Pg) {
		LoadedPage& a = LoadedPages[Pg];
		if (LruFirst < 0) { // empty LRU list
			a.LruNext = a.LruPrev = -1;
			LruFirst = LruLast = Pg;
		} else if (LruFirst == Pg) {
			// it's ok, already at start LRU
		} else {
			UnlistFromLru(Pg);
			EnlistToStartLru(Pg);
		}
	}

	/// Evicts last possible page from cache.
	int TPgBlob::Evict() {
		int Pg = LruLast;
		while (Pg != LruFirst) {
			if (CanEvictPage(Pg)) {
				break;
			}
			Pg = LoadedPages[Pg].LruPrev;
		}
		LoadedPage& a = LoadedPages[Pg];
		UnlistFromLru(Pg);
		LoadedPagesH.DelKey(a.Pt);
		if (ShouldSavePage(Pg)) {
			Files[a.Pt.GetFIx()]->SavePage(a.Pt.GetPg(), GetPageBf(Pg));
		}
		return Pg;
	}

	/// Load given page into memory
	byte* TPgBlob::LoadPage(const TPgBlobPgPt& Pt) {
		int Pg;
		if (LoadedPagesH.IsKeyGetDat(Pt, Pg)) { // is page in cache
			MoveToStartLru(Pg);
			return GetPageBf(Pg);
		}
		if (LoadedPages.Len() == MxLoadedPages) {
			// evict last page + load new page
			Pg = Evict();
			LoadedPage& a = LoadedPages[Pg];
			Files[Pt.GetFIx()]->LoadPage(Pt.GetPg(), GetPageBf(Pg));
			a.Pt = Pt;
			EnlistToStartLru(Pg);
			int hid = LoadedPagesH.AddKey(Pt);
			LoadedPagesH(Pt) = Pg;
		} else {
			// simply load the page
			Pg = LoadedPages.Add();
			LoadedPage& a = LoadedPages[Pg];
			Files[Pt.GetFIx()]->LoadPage(Pt.GetPg(), GetPageBf(Pg));
			a.Pt = Pt;
			EnlistToStartLru(Pg);
			int hid = LoadedPagesH.AddKey(Pt);
			LoadedPagesH[hid] = Pg;
		}
		return GetPageBf(Pg);
	}

	/// Create new page and return pointers to it
	void TPgBlob::CreateNewPage(TPgBlobPgPt& Pt, byte** Bf) {
		// determine if last file is empty
		if (Files.Len() > 0) {
			// try to add to last file
			uint32 Pg = Files.Last()->CreateNewPage();
			if (Pg >= 0) {
				Pt.Set(Files.Len() - 1, Pg);
				*Bf = LoadPage(Pt);
				InitPageP(*Bf);
				return;
			}
		}
		TStr NewFNm = FNm + ".bin" + TStr::GetNrNumFExt(Files.Len());
		Files.Add(TPgBlobFile::New(NewFNm, TFAccess::faCreate, TInt::Giga));
		uint32 Pg = Files.Last()->CreateNewPage();
		EAssert(Pg >= 0);
		Pt.Set(Files.Len() - 1, Pg);
		*Bf = LoadPage(Pt);
		InitPageP(*Bf);
	}

	/// Factory method for creating new BLOB storage
	PPgBlob TPgBlob::Create(const TStr& FNm, const uint64& CacheSize) {
		return PPgBlob(new TPgBlob(FNm, TFAccess::faCreate, CacheSize));
	}

	/// Factory method for opening existing BLOB storage
	PPgBlob TPgBlob::Open(const TStr& FNm, const uint64& CacheSize) {
		return PPgBlob(new TPgBlob(FNm, TFAccess::faUpdate, CacheSize));
	}

	/// Initialize new page
	void TPgBlob::InitPageP(byte* Pt) {
		TPgHeader* Pt2 = (TPgHeader*)Pt;
		Pt2->Flags = PgHeaderDirtyFlag; // not saved yet
		Pt2->PageSize = PAGE_SIZE;
		Pt2->PageVersion = 1;
		Pt2->OffsetFreeStart = sizeof(TPgHeader);
		Pt2->OffsetFreeEnd = PAGE_SIZE;
		Pt2->ItemCount = 0;
	}

	/// Store BLOB to storage
	TPgBlobPt TPgBlob::Put(const byte* Bf, const int& BfL) {
		// find page
		TPgBlobPgPt PgPt;
		byte* PgBf = NULL;
		TPgHeader* PgH = NULL;
		bool is_new_page = false;

		// first scan those that are in memory
		// use the first one with enough space
		// TODO is there a faster way? is this slow? it's linear...
		for (int i = 0; i < LoadedPages.Len(); i++) {
			byte* PgBfTmp = GetPageBf(i);
			PgH = (TPgHeader*)PgBfTmp;
			if (PgH->CanStoreBf(BfL)) {
				PgBf = PgBfTmp;
				PgPt = LoadedPages[i].Pt;
				break;
			}
		}

		// if no page found in memory, use FSM
		if (PgBf == NULL) {
			if (Fsm.FsmGetFreePage(BfL + sizeof(TPgBlobPageItem), PgPt)) {
				PgBf = LoadPage(PgPt);
			} else {
				CreateNewPage(PgPt, &PgBf);
				is_new_page = true;
			}
		}

		// add data
		uint16 ii = AddItem(PgBf, Bf, BfL);
		TPgBlobPt Pt(PgPt.GetFIx(), PgPt.GetPg(), ii);
		PgH = (TPgHeader*)PgBf;

		// update free-space-map
		if (is_new_page) {
			Fsm.FsmAddPage(PgPt, PgH->GetFreeMem());
		} else {
			Fsm.FsmUpdatePage(PgPt, PgH->GetFreeMem());
		}
		return Pt;
	}

	/// Store existing BLOB to storage
	TPgBlobPt TPgBlob::Put(
		const byte* Bf, const int& BfL, const TPgBlobPt& Pt) {

		// find page
		byte* PgBf = NULL;
		TPgHeader* PgH = NULL;
		TPgBlobPageItem* ItemRec = NULL;

		TPgBlobPgPt PgPt = Pt;
		PgBf = LoadPage(PgPt);
		PgH = (TPgHeader*)PgBf;
		ItemRec = GetItemRec(PgBf, Pt.GetIIx());

		int existing_size = ItemRec->Len;
		if (existing_size == BfL) {
			// we're so lucky, just overwrite buffer
			memcpy(PgBf + ItemRec->Offset, Bf, BfL);
			return Pt;

		} else if (existing_size + PgH->GetFreeMem() >= BfL) {
			// ok, everything can still be inside this page
			DeleteItem(PgBf, Pt.GetIIx());
			ChangeItem(PgBf, Pt.GetIIx(), Bf, BfL);
			Fsm.FsmUpdatePage(Pt, PgH->GetFreeMem());
			return Pt;

		} else {
			// bad luck, we need to move data to new page
			DeleteItem(PgBf, Pt.GetIIx());
			Fsm.FsmUpdatePage(PgPt, PgH->GetFreeMem());

			CreateNewPage(PgPt, &PgBf);
			uint16 ii = AddItem(PgBf, Bf, BfL);

			TPgBlobPt Pt2(PgPt.GetFIx(), PgPt.GetPg(), ii);
			PgH = (TPgHeader*)PgBf;
			Fsm.FsmUpdatePage(PgPt, PgH->GetFreeMem());
			return Pt2;
		}
	}

	/// Retrieve BLOB from storage
	TQm::TStorage::TThinMIn TPgBlob::Get(const TPgBlobPt& Pt) {
		byte* Pg = LoadPage(Pt);
		TPgBlobPageItem* Item = GetItemRec(Pg, Pt.GetIIx());
		byte* Data;
		int Len = Item->Len;
		GetItem(Pg, Pt.GetIIx(), &Data, Len);
		return TQm::TStorage::TThinMIn(Data, Len);
	}

	/// Retrieve BLOB from storage
	TMemBase TPgBlob::GetMemBase(const TPgBlobPt& Pt) {
		return Get(Pt).GetMemBase();
	}

	/// Save part of the data, given time-window
	void TPgBlob::PartialFlush(int WndInMsec) {
		TTmStopWatch sw(true);
		for (int i = 0; i < LoadedPages.Len(); i++) {
			if (ShouldSavePage(i)) {
				LoadedPage& a = LoadedPages[i];
				Files[a.Pt.GetFIx()]->SavePage(a.Pt.GetPg(), GetPageBf(i));
				if (sw.GetMSec() > WndInMsec)
					break;
			}
		}
	}

	/// Marks page as dirty - data inside was written directly
	void TPgBlob::SetDirty(const TPgBlobPt& Pt) {
		byte* Pg = LoadPage(Pt);
		((TPgHeader*)Pg)->SetDirty(true);
	}

	/// Retrieve statistics for this object
	PJsonVal TPgBlob::GetStats() {
		int dirty = 0;
		for (int i = 0; i < LoadedPages.Len(); i++) {
			if (ShouldSavePage(i)) {
				dirty++;
			}
		}

		PJsonVal res = TJsonVal::NewObj();
		res->AddToObj("page_size", PAGE_SIZE);
		res->AddToObj("loaded_pages", LoadedPages.Len());
		res->AddToObj("dirty_pages", dirty);
		res->AddToObj("cache_size", BfL);
		return res;
	}

	///////////////////////////////////////////////////////////////////////////

	/// Add new page to free-space-map
	void TPgBlobFsm::FsmAddPage(const TPgBlobPgPt& Pt, const uint16& FreeSpace) {
		int index = Len();
		Add(TPgBlobPt(Pt.GetFIx(), Pt.GetPg(), FreeSpace));
		FsmSiftUp(index);
	}

	/// Update existing page inside free-space-map
	void TPgBlobFsm::FsmUpdatePage(const TPgBlobPgPt& Pt, const uint16& FreeSpace) {
		// find record
		int rec = -1;
		for (int i = 0; i < Len(); i++) {
			if (GetVal(i).GetPg() == Pt.GetPg() &&
				GetVal(i).GetFIx() == Pt.GetFIx()) {
				rec = i;
				break;
			}
		}
		EAssert(rec >= 0);
		if (rec < Len() - 1) {
			GetVal(rec) = Last();
			DelLast();
			FsmPushDown(rec);
			FsmAddPage(Pt, FreeSpace);
		} else {
			Last().SetIIx(FreeSpace);
			FsmSiftUp(rec);
		}
	}

	/// Find page with most open space.
	/// Returns false if no such page, true otherwise
	/// If page exists, pointer to it is stored into sent parameter
	bool TPgBlobFsm::FsmGetFreePage(int RequiredSpace, TPgBlobPgPt& PgPt) {
		if (Len() == 0) {
			return false;
		}
		TPgBlobPt& Pt = GetVal(0);
		PgPt.Set(Pt.GetFIx(), Pt.GetPg());
		return (Pt.GetIIx() >= RequiredSpace);
	}

	/// Move item up the heap if needed
	int TPgBlobFsm::FsmSiftUp(int index) {
		EAssert(index < Len());
		EAssert(index >= 0);
		if (index == 0)
			return index;
		int parent_index = FsmParent(index);
		if (GetVal(parent_index).GetIIx() < GetVal(index).GetIIx()) {
			Swap(parent_index, index);
			return FsmSiftUp(parent_index);
		} else {
			return index;
		}
	}

	/// Move item down the heap if needed
	int TPgBlobFsm::FsmPushDown(int index) {
		EAssert(index < Len());
		EAssert(index >= 0);
		int child1 = FsmLeftChild(index);
		int child2 = FsmRightChild(index);
		if (child1 >= Len())
			child1 = -1;
		if (child2 >= Len())
			child2 = -1;

		if (child1 < 0)
			return index; // no children
		if (child2 < 0) {
			// compare to child1
			if (GetVal(index).GetIIx() < GetVal(child1).GetIIx()) {
				Swap(child1, index);
				return FsmPushDown(child1);
			}
		} else {
			uint16 v1 = GetVal(child1).GetIIx();
			uint16 v2 = GetVal(child2).GetIIx();
			bool v2_is_bigger = (v2 > v1);
			if (v2_is_bigger) {
				if (GetVal(index).GetIIx() < v2) {
					Swap(child2, index);
					return FsmPushDown(child2);
				}
			} else {
				if (GetVal(index).GetIIx() < v1) {
					Swap(child1, index);
					return FsmPushDown(child1);
				}
			}
		}
		return index;
	}
}
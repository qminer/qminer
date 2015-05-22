
#include "pgblob.h"

namespace glib {

	/// Assignment operator
	TPgBlobPt& TPgBlobPt::operator = (const TPgBlobPt& Pt) {
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

	/////////////////////////////////////////////////////////////////////////////

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
			fwrite(Bf, 1, PAGE_SIZE, FileId) == PAGE_SIZE,
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
		EAssertR(
			fwrite(this, 1, PAGE_SIZE, FileId) == PAGE_SIZE,
			"Error writing file '" + TStr(FNm) + "'.");
		return (uint32)(len / PAGE_SIZE);
	}

	///////////////////////////////////////////////////////////////////////////

	TPgBlobPage::TPgBlobPage() {}

	TPgBlobPage::~TPgBlobPage() {}

	///////////////////////////////////////////////////////////////////////////

	/// Private constructor
	TPgBlob::TPgBlob(const TStr& _FNm, const TFAccess& _Access, const uint64& CacheSize) {

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
		Init();
	}

	/// Destructor
	TPgBlob::~TPgBlob() {
		for (int i = 0; i < LoadedPages.Len(); i++) {
			LoadedPage& a = LoadedPages[i];
			Files[a.Pt.GetFileIndex()]->SavePage(a.Pt.GetPage(), GetPageBf(i));
		}
		SaveMain();
		Files.Clr();
		delete[] Bf;
	}


	/// Save main file
	void TPgBlob::SaveMain() {
		PSOut SOut = TFOut::New(FNm + ".main");
		TInt children_cnt(Files.Len());
		SOut->Save(children_cnt);
	}

	/// Load main file
	void TPgBlob::LoadMain() {
		PSIn SIn = TFIn::New(FNm + ".main");
		int children_cnt;
		SIn->Load(children_cnt);
		Files.Clr();
		for (int i = 0; i < children_cnt; i++) {
			TStr FNmChild = FNm + ".bin" + TStr::GetNrNumFExt(children_cnt);
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
			Files[a.Pt.GetFileIndex()]->SavePage(a.Pt.GetPage(), GetPageBf(Pg));
		}
		return Pg;
	}

	/// Load given page into memory
	byte* TPgBlob::LoadPage(const TPgBlobPt& Pt) {
		int Pg;
		if (LoadedPagesH.IsKeyGetDat(Pt, Pg)) { // is page in cache
			MoveToStartLru(Pg);
			return GetPageBf(Pg);
		}
		if (LoadedPages.Len() == MxLoadedPages) {
			// evict last page + load new page
			Pg = Evict();
			LoadedPage& a = LoadedPages[Pg];
			Files[Pt.GetFileIndex()]->LoadPage(Pt.GetPage(), GetPageBf(Pg));
			a.Pt = Pt;
			EnlistToStartLru(Pg);
			int hid = LoadedPagesH.AddKey(Pt);
			LoadedPagesH(Pt) = Pg;
		} else {
			// simply load the page
			Pg = LoadedPages.Add();
			LoadedPage& a = LoadedPages[Pg];
			Files[Pt.GetFileIndex()]->LoadPage(Pt.GetPage(), GetPageBf(Pg));
			a.Pt = Pt;
			EnlistToStartLru(Pg);
			int hid = LoadedPagesH.AddKey(Pt);
			LoadedPagesH[hid] = Pg;
		}
		return GetPageBf(Pg);
	}

	/// Create new page and return pointers to it
	TPair<TPgBlobPt, byte*> TPgBlob::CreateNewPage() {
		TPair<TPgBlobPt, byte*> res;
		// determine if last file is empty
		if (Files.Len() > 0) {
			// try to add to last file
			uint32 Pg = Files.Last()->CreateNewPage();
			if (Pg >= 0) {
				res.Val1 = TPgBlobPt(Pg, Files.Len() - 1, 0);
				res.Val2 = LoadPage(res.Val1);
				InitPageP(res.Val2);
				return res;
			}
		}
		TStr NewFNm = FNm + ".bin" + TStr::GetNrNumFExt(Files.Len());
		Files.Add(TPgBlobFile::New(NewFNm, TFAccess::faCreate, TInt::Giga));
		uint32 Pg = Files.Last()->CreateNewPage();
		EAssert(Pg >= 0);
		res.Val1 = TPgBlobPt(Pg, Files.Len() - 1, 0);
		res.Val2 = LoadPage(res.Val1);
		InitPageP(res.Val2);
		return res;
	}

	//////////////////////////////////////////////////////

	/// Destructor
	TTestPgBlob::~TTestPgBlob() {
		for (int i = 0; i < LoadedPages.Len(); i++) {
			if (ShouldSavePage(i)) {
				LoadedPage& a = LoadedPages[i];
				Files[a.Pt.GetFileIndex()]->SavePage(a.Pt.GetPage(), GetPageBf(i));
			}
		}
		SaveMain();
		Files.Clr();
		delete[] Bf;
	}

	/// Factory method for creating new BLOB storage
	PTestPgBlob TTestPgBlob::Create(const TStr& FNm, const uint64& CacheSize) {
		return PTestPgBlob(new TTestPgBlob(FNm, TFAccess::faCreate, CacheSize));
	}

	/// Factory method for opening existing BLOB storage
	PTestPgBlob TTestPgBlob::Open(const TStr& FNm, const uint64& CacheSize) {
		return PTestPgBlob(new TTestPgBlob(FNm, TFAccess::faUpdate, CacheSize));
	}

	/// Initialize new page
	void TTestPgBlob::InitPageP(byte* Pt) {
		TPgHeader* Pt2 = (TPgHeader*)Pt;
		Pt2->Flags = PgHeaderDirtyFlag; // not saved yet
		Pt2->PageSize = PAGE_SIZE;
		Pt2->PageVersion = 1;
		Pt2->OffsetFreeStart = sizeof(TPgHeader);
		Pt2->OffsetFreeEnd = PAGE_SIZE;
	}
}
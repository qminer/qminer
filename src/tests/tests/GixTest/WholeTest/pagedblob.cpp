
#include "pagedblob.h"

namespace glib {


	/// Private constructor
	TPgBlobFile::TPgBlobFile(
		const TStr& _FNm, const TFAccess& _Access, const int& _MxSegLen) {

		Access = _Access;
		FNm = _FNm;

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
			"Error reading file '" + TStr(FNm) + "'.");
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////



	TPgBlobPage::TPgBlobPage() {}

	TPgBlobPage::~TPgBlobPage() {}

	///////////////////////////////////////////////////////////////////////////

	TPgBlob::TPgBlob(const TStr& FNm, const uint64& CacheSize) {
		// TODO open or create
		EAssertR(
			CacheSize >= PAGE_SIZE,
			"Invalid cache size for TPgBlob '" + TStr(CacheSize) + "'.");
		// init cache
		Bf = new byte[CacheSize];
		BfL = CacheSize;
		MxLoadedPages = CacheSize / PAGE_SIZE;
		LruFirst = LruLast = -1;
	}

	TPgBlob::~TPgBlob() {
		// TODO write all to disk
		Files.Clr();
		delete[] Bf;
	}

	PPgBlob TPgBlob::Create(const TStr& FNm, const uint64& CacheSize) {
		// TODO
		// check if any file exists, delete them if needed
		return PPgBlob(new TPgBlob(FNm, CacheSize));
	}

	PPgBlob TPgBlob::Open(const TStr& FNm, const uint64& CacheSize) {
		// TODO
		return PPgBlob(new TPgBlob(FNm, CacheSize));
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

	/// Load given page into memory
	byte* TPgBlob::LoadPage(const TPgBlobPt& Pt) {
		int Pg;
		if (LoadedPagesH.IsKeyGetDat(Pt, Pg)) {
			MoveToStartLru(Pg);
			return GetPageBf(Pg);
		}
		if (LoadedPages.Len() == MxLoadedPages) {
			// evict last page
			Pg = LruLast;
			LoadedPage& a = LoadedPages[Pg];
			UnlistFromLru(Pg);
			LoadedPagesH.DelKey(a.Pt);
			if (ShouldSavePage(Pg)) {
				Files[a.Pt.GetFileIndex()]->SavePage(a.Pt.GetPage(), GetPageBf(Pg));
			}
			// load new page
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
	}

	/// Create new page and return pointers to it
	void TPgBlob::CreateNewPage(TPgBlobPt& BlobPt, byte** Pt) {}
}
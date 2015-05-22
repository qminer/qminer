#ifndef PAGEDBLOB_H
#define PAGEDBLOB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bd.h>
#include <base.h>
#include <mine.h>

namespace glib {

	//////////////////////////////////////////////////////////////////
	// Foward declarations
	class TPgBlobPt;
	class TPgBlobFile;
	class TPgBlobPage;
	class TPgBlob;
	/// Smart pointers
	typedef TPt<TPgBlobPage> TPgBlobPtPPgBlobPage;
	typedef TPt<TPgBlobFile> PPgBlobFile;
	typedef TPt<TPgBlob> PPgBlob;


	/// Page size is 8kb
#define PAGE_SIZE 8192

	////////////////////////////////////////////////////////////
	/// Pointer into Paged-Blob storage 
	class TPgBlobPt {
	private:

		/// index of page within the file
		uint32 Page;
		/// index of file - -1 means NULL pointer
		int16 FileIndex;
		/// item index within page
		uint16 ItemIndex;

	public:
		/// Default constructor
		TPgBlobPt() : Page(0), FileIndex(-1), ItemIndex(0) {}

		/// Deserialization constructor
		TPgBlobPt(TSIn& SIn) {
			SIn.Load(Page);  SIn.Load(FileIndex); SIn.Load(ItemIndex);
		}
		// Set constructor
		TPgBlobPt(uint32 _Page, int16 _FileIndex, uint16 _ItemIndex) {
			Page = _Page; FileIndex = _FileIndex; ItemIndex = _ItemIndex;
		}

		/// get index of page within the file
		uint32 GetPage() const { return Page; }
		/// get index of file - -1 means NULL pointer
		int16 GetFileIndex() const { return FileIndex; }
		/// get item index within page
		uint16 GetItemIndex() const { return ItemIndex; }

		/// Serialization of this object
		void Save(TSOut& SOut) const {
			SOut.Save(Page); SOut.Save(FileIndex); SOut.Save(ItemIndex);
		}

		/// Is this NULL pointer
		bool Empty() { return (FileIndex < 0); }

		/// Clear this pointer - make it NULL
		bool Clr() { FileIndex = -1; Page = ItemIndex = 0; }

		/// Assignment operator
		TPgBlobPt& operator=(const TPgBlobPt& Pt);
		/// Equality comparer
		bool operator==(const TPgBlobPt& Pt) const;
		/// Comparison of pointers for sorting
		bool operator<(const TPgBlobPt& Pt) const;

		/// Returns memory usage - for caching and other stuff
		uint64 GetMemUsed() const { return sizeof(TPgBlobPt); }

		int GetPrimHashCd() const { return abs(int(Page) + ItemIndex + FileIndex); }
		int GetSecHashCd() const { return (abs(int(Page)) + int(ItemIndex) * 0x10 + +int(FileIndex) * 0x100); }
	};

	///////////////////////////////////////////////////////////////////////
	/// Single Paged-Blob-storage file
	class TPgBlobFile {
	private:

		/// Maximal length of single file
		long MxFileLen;
		/// File name
		TStr FNm;
		/// File access type
		TFAccess Access;
		/// Random-access file - BLOB storage
		FILE* FileId;

		/// Private constructor
		TPgBlobFile(const TStr& _FNm, const TFAccess& _Access = faRdOnly,
			const uint32& _MxSegLen = -1);

		/// Refresh the position - internal check
		void RefreshFPos();
		/// Set position in the file
		void SetFPos(const int& FPos);

	public:
		/// Reference count for smart pointers
		TCRef CRef;

		/// Destructor
		~TPgBlobFile();

		/// Factory method
		static PPgBlobFile New(const TStr& FNm, const TFAccess& Access = faRdOnly,
			const uint32& MxSegLen = -1) {
			return PPgBlobFile(new TPgBlobFile(FNm, Access, MxSegLen));
		}

		/// Load page with given index from the file into buffer
		int LoadPage(const uint32& Page, void* Bf);
		/// Save buffer to page within the file 
		int SavePage(const uint32& Page, const void* Bf);
		/// Reserve new space in the file. Returns -1 if file is full.
		uint32 CreateNewPage();
	};

	////////////////////////////////////////////////////////////
	/// Page, used in multi-file paged-BLOB-storage
	class TPgBlobPage {
	private:
		TPgBlobPage();
	public:
		~TPgBlobPage();

		/// Reference count for smart pointers
		TCRef CRef;

		/// Parent class 
		friend class TPgBlob;
	};

	////////////////////////////////////////////////////////////
	/// Multi-file paged-BLOB-storage with cache.
	/// Has no clue about the meaning of the data in pages. 
	/// It doesn't even know if data is dirty.
	class TPgBlob {
	private:

		/// Housekeeping record for each loaded page
		struct LoadedPage {
		public:
			/// Blob pointer of this page
			TPgBlobPt Pt;
			/// Next item in LRU list
			int LruNext;
			/// Previous item in LRU list
			int LruPrev;
		};

		/// File name
		TStr FNm;
		/// File access
		TFAccess Access;
		/// Individual files that comprise this BLOB storage
		TVec<PPgBlobFile> Files;
		/// Pointers for loaded pages
		THash<TPgBlobPt, int> LoadedPagesH;
		/// Pointers for loaded pages
		TVec<LoadedPage> LoadedPages;

		/// Next item in LRU list - this one was accessed last
		int LruFirst;
		/// Previous item in LRU list - the next candidate for eviction
		int LruLast;

		/// Memory buffer - cache
		byte* Bf;
		/// Buffer length
		uint64 BfL;
		/// Maximal number of loaded pages
		uint64 MxLoadedPages;

		/// Returns starting address of page in Bf
		byte* GetPageBf(int Pg) {
			return Bf + Pg * PAGE_SIZE; 
		}

		/// Private constructor
		TPgBlob(const TStr& _FNm, const TFAccess& _Access, const uint64& CacheSize);
		/// remove given page from LRU list
		void UnlistFromLru(int Pg);
		/// move given page to the start of LRU list
		void MoveToStartLru(int Pg);
		/// insert given (new) page to the start of LRU list
		void EnlistToStartLru(int Pg);

		/// Evicts last possible page from cache.
		int Evict();

		/// Save main file
		void SaveMain();
		/// Load main file
		void LoadMain();
		/// Find which child files exist
		void DetectSegments();
	public:
		/// Reference count for smart pointers
		TCRef CRef;
		/// Factory method for creating new BLOB storage
		static PPgBlob Create(const TStr& FNm, const uint64& CacheSize = 10 * TNum<int>::Mega);
		/// Factory method for opening existing BLOB storage
		static PPgBlob Open(const TStr& FNm, const uint64& CacheSize = 10 * TNum<int>::Mega);
		/// Destructor
		~TPgBlob();

		/// Load given page into memory
		byte* LoadPage(const TPgBlobPt& Pt);

		/// Create new page and return pointers to it
		TPair<TPgBlobPt, byte*> CreateNewPage();

		/// This method should be overridden in derived class to tell 
		/// if given page should be stored to disk.
		virtual bool ShouldSavePage(byte* Pt) { return true; }

		/// This method should be overridden in derived class to tell 
		/// if given page can be evicted from cache.
		virtual bool CanEvictPage(byte* Pt) { return true; }

		/// This method should be overridden in derived class to tell 
		/// if given page should be stored to disk.
		bool ShouldSavePage(int Pg) { return ShouldSavePage(GetPageBf(Pg)); }

		/// This method should be overridden in derived class to tell 
		/// if given page can be evicted from cache.
		bool CanEvictPage(int Pg) { return CanEvictPage(GetPageBf(Pg)); }
	};

}
#endif
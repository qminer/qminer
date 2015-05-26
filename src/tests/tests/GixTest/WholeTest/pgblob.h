#ifndef PAGEDBLOB_H
#define PAGEDBLOB_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <bd.h>
#include <base.h>
#include <mine.h>

#include <qminer_core.h>
#include <qminer_gs.h>

namespace glib {

	//////////////////////////////////////////////////////////////////
	// Foward declarations
	class TPgBlobPt;
	class TPgBlobFile;
	class TPgBlobPage;
	class TPgBlob;
	// Smart pointers
	typedef TPt<TPgBlobPage> TPgBlobPtPPgBlobPage;
	typedef TPt<TPgBlobFile> PPgBlobFile;
	typedef TPt<TPgBlob> PPgBlob;

	// Macros and constants	
#define PAGE_SIZE (8 * 1024) // Page size is 8kb
#define PgHeaderDirtyFlag (0x01)
#define PgHeaderSLockFlag (0x02)
#define PgHeaderXLockFlag (0x04)

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
	/// Multi-file paged-BLOB-storage with cache.
	/// Has no clue about the meaning of the data in pages. 
	/// It doesn't even know if data is dirty.
	class TPgBlob {
	protected:

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

		/// Single record in item index section
		struct TPgBlobPageItem {
			/// Offset of data from start of page
			uint16 Offset;
			/// Length of data, 0 means it was deleted
			uint16 Len;
		};

		/// Wrapper for header-data of the page
		class TPgHeader {
		public:
			/// Size of the page that is stored
			uint16 PageSize; // page size should not be more than 64k
			/// Version of page layout
			uchar PageVersion;
			/// Page flags - dirty, lock etc.
			uchar Flags;
			/// Number of items in this page, including deleted ones
			uint16 ItemCount;
			/// Offset of free space in the block
			uint16 OffsetFreeStart;
			/// Offset of end of the free space
			uint16 OffsetFreeEnd;

			/// Is this page dirty
			bool IsDirty() { return (Flags & PgHeaderDirtyFlag) != 0; }
			/// Is this page shared-lock-ed
			bool IsSLock() { return (Flags & PgHeaderSLockFlag) != 0; }
			/// Is this page exclusive-lock-ed
			bool IsXLock() { return (Flags & PgHeaderXLockFlag) != 0; }
			/// Is this page locked (any type)
			bool IsLock() { return (Flags & (PgHeaderSLockFlag | PgHeaderXLockFlag)) != 0; }

			/// Set dirty flag for this page
			void SetDirty(bool val) {
				if (val) { Flags |= PgHeaderDirtyFlag; } else { Flags ^= PgHeaderDirtyFlag; }
			}
			/// Set S-lock flag for this page
			void SetSLock(bool val) {
				if (val) { Flags |= PgHeaderSLockFlag; } else { Flags ^= PgHeaderSLockFlag; }
			}
			/// Set X-lock flag for this page
			void SetXLock(bool val) {
				if (val) { Flags |= PgHeaderXLockFlag; } else { Flags ^= PgHeaderXLockFlag; }
			}
			/// Get amount of free space in this page
			int GetFreeMem() { return OffsetFreeEnd - OffsetFreeStart; }
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
		byte* GetPageBf(int Pg) { return Bf + Pg * PAGE_SIZE; }

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

		/// This method tells if given page should be stored to disk.
		bool ShouldSavePage(int Pg) { return ShouldSavePageP(GetPageBf(Pg)); }

		/// This method tells if given page can be evicted from cache.
		bool CanEvictPage(int Pg) { return CanEvictPageP(GetPageBf(Pg)); }

		/// This method should be overridden in derived class to tell 
		/// if given page should be stored to disk.
		bool ShouldSavePageP(byte* Pt) { return ((TPgHeader*)Pt)->IsDirty(); }

		/// This method should be overridden in derived class to tell 
		/// if given page can be evicted from cache.
		bool CanEvictPageP(byte* Pt) { return ((TPgHeader*)Pt)->IsLock(); }

		/// Load given page into memory
		byte* LoadPage(const TPgBlobPt& Pt);

		/// Create new page and return pointers to it
		TPair<TPgBlobPt, byte*> CreateNewPage();

		/// Initialize new page.
		static void InitPageP(byte* Pt);

		/// Get pointer to item record - in it are offset and length
		static TPgBlobPageItem* GetItemRec(byte* Pg, uint16 ItemIndex);
		/// Add given buffer to page, return item-index
		static uint16 AddItem(byte* Pg, byte* Bf, int BfL);
		/// Retrieve buffer from specified page
		static void GetItem(byte* Pg, uint16 ItemIndex, byte** Bf, int& BfL);
		/// Delete buffer from specified page
		static void DeleteItem(byte* Pg, uint16 ItemIndex);

	public:

		/// Reference count for smart pointers
		TCRef CRef;

		/// Constructor
		TPgBlob(const TStr& _FNm, const TFAccess& _Access, const uint64& CacheSize);
		/// Destructor
		~TPgBlob();

		/// Factory method for creating new BLOB storage
		static PPgBlob Create(const TStr& FNm, const uint64& CacheSize = 10 * TNum<int>::Mega);
		/// Factory method for opening existing BLOB storage
		static PPgBlob Open(const TStr& FNm, const uint64& CacheSize = 10 * TNum<int>::Mega);

		TPgBlobPt Put(const TQm::TStorage::TThinMIn& Data);
		TQm::TStorage::TThinMIn Get(TPgBlobPt);
#ifdef XTEST
		friend class XTest;
#endif
	};
}
#endif
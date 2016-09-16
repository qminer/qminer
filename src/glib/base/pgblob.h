/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/
#ifndef PGBLOB_H
#define PGBLOB_H

//////////////////////////////////////////////////////////////////
// Foward declarations
class TPgBlobPgPt;
class TPgBlobPt;
class TPgBlobFile;
class TPgBlobPage;
class TPgBlob;
// Smart pointers
typedef TPt<TPgBlobPage> TPgBlobPtPPgBlobPage;
typedef TPt<TPgBlobFile> PPgBlobFile;
typedef TPt<TPgBlob> PPgBlob;

// Macros and constants
#define PG_PAGE_SIZE (8 * 1024) // Page size is 8k
#define PG_EXTENT_PCOUNT 8        // Number of pages per extent
#define PG_EXTENT_SIZE (PG_PAGE_SIZE * PG_EXTENT_PCOUNT) // Extent size - 64k
#define PgHeaderDirtyFlag (0x01)
#define PgHeaderSLockFlag (0x02)
#define PgHeaderXLockFlag (0x04)


////////////////////////////////////////////////////////////
/// Pointer to Paged-Blob page
class TPgBlobPgPt {
protected:
    /// index of page within the file
    uint32 Page;
    /// index of file - -1 means NULL pointer
    int16 FileIndex;
public:
    /// Default constructor
    TPgBlobPgPt() { FileIndex = -1; Page = 0; }
    /// Deserialization constructor
    TPgBlobPgPt(TSIn& SIn) { SIn.Load(Page);  SIn.Load(FileIndex); }
    // Set constructor
    TPgBlobPgPt(int16 _FileIndex, uint32 _Page) { Set(_FileIndex, _Page); }
    // Set constructor
    TPgBlobPgPt(const TPgBlobPt& Src);

    /// get index of page within the file
    uint32 GetPg() const { return Page; }
    /// get index of file - -1 means NULL pointer
    int16 GetFIx() const { return FileIndex; }

    /// Set internal values
    void Set(int16 fi, uint32 pg);
    /// Serialization of this object
    void Save(TSOut& SOut) const { SOut.Save(Page); SOut.Save(FileIndex); }

    /// Assignment operator
    TPgBlobPgPt& operator=(const TPgBlobPgPt& Pt);
    /// Equality comparer
    bool operator==(const TPgBlobPgPt& Pt) const;
    /// Comparison of pointers for sorting
    bool operator<(const TPgBlobPgPt& Pt) const;

    /// Returns memory usage - for caching and other stuff
    uint64 GetMemUsed() const { return sizeof(TPgBlobPgPt); }

    /// for insertion into THash
    int GetPrimHashCd() const;
    /// for insertion into THash
    int GetSecHashCd() const;
};

////////////////////////////////////////////////////////////
/// Pointer into Paged-Blob storage
class TPgBlobPt {
protected:

    /// index of page within the file
    uint32 Page;
    /// index of file - -1 means NULL pointer
    int16 FileIndex;
    /// item index within page.
    /// alternatively can also be used to store free-space.
    uint16 ItemIndex;

public:
    /// Default constructor
    TPgBlobPt() { FileIndex = -1; Page = 0; ItemIndex = 0; }

    /// Deserialization constructor
    TPgBlobPt(TSIn& SIn) {
        SIn.Load(Page);  SIn.Load(FileIndex); SIn.Load(ItemIndex);
    }
    /// Set constructor
    TPgBlobPt(int16 _FileIndex, uint32 _Page, uint16 _ItemIndex) {
        Page = _Page; FileIndex = _FileIndex; ItemIndex = _ItemIndex;
    }

    /// get index of page within the file
    uint32 GetPg() const { return Page; }
    /// get index of file - -1 means NULL pointer
    int16 GetFIx() const { return FileIndex; }
    /// get item index within page
    uint16 GetIIx() const { return ItemIndex; }
    /// set all values
    void Set(int16 fi, uint32 pg, uint16 ii);
    /// set only ItemIndex
    void SetIIx(uint16 ii) { ItemIndex = ii; }

    /// Serialization of this object
    void Save(TSOut& SOut) const {
        SOut.Save(Page); SOut.Save(FileIndex); SOut.Save(ItemIndex);
    }

    /// Is this NULL pointer
    bool Empty() const { return (FileIndex < 0); }

    /// Clear this pointer - make it NULL
    void Clr() { FileIndex = -1; Page = ItemIndex = 0; }

    /// Assignment operator
    TPgBlobPt& operator=(const TPgBlobPt& Pt);
    /// Equality comparer
    bool operator==(const TPgBlobPt& Pt) const;
    /// Comparison of pointers for sorting
    bool operator<(const TPgBlobPt& Pt) const;
    // conversion to A (type-cast operator)
    operator TPgBlobPgPt() { return TPgBlobPgPt(FileIndex, Page); }

    /// Returns memory usage - for caching and other stuff
    uint64 GetMemUsed() const { return sizeof(TPgBlobPt); }

    /// for insertion into THash
    int GetPrimHashCd() const;
    /// for insertion into THash
    int GetSecHashCd() const;
};

///////////////////////////////////////////////////////////////////////
/// Binary tree of max values
//template <class TVal>
class TBinTreeMaxVals {
protected:
    TVec<TVec<TUInt>> Layers;
    TVec<TPgBlobPgPt> Pts;
public:
    /// Simple constructor
    TBinTreeMaxVals() { Layers.Add(); }
    /// Add new record to the structure
    int Add(const uint16& Val, const TPgBlobPgPt& Pt);
    /// Number of stored elements
    int Len() const { return Layers[0].Len(); }
    /// Change value at given position
    void Change(const int& RecN, const uint16& Val);
    /// Get index of item with max value
    int GetIndexOfMax() const;
    /// Get max value
    int GetVal(const int& RecN) const { return Layers[0][RecN]; }
    /// Get BLOB pointer for item with max value
    const TPgBlobPgPt& GetPtOf(const int& RecN) const { return Pts[RecN]; }
    /// Clears content
    void Clr() { Layers.Clr(); }

    /// Save to output stream
    void Save(TSOut& SOut) const { Layers.Save(SOut); Pts.Save(SOut); }
    /// Load from input stream
    void Load(TSIn& SIn) { Layers.Load(SIn); Pts.Load(SIn); }

#ifdef XTEST
    friend class XTest;
    /// For debugging purposes
    void Print();
#endif
};


///////////////////////////////////////////////////////////////////////
/// Free-space-map (heap)

class TPgBlobFsm {
protected:

    /// Mapping from pagged-blob pointer to index in MaxFSpace (layer 0)
    THash<TPgBlobPgPt, TInt> PtH;

    /// Binary three for quick search of biggest value
    /// Each parent is maximum of their children
    TBinTreeMaxVals MaxFSpace;

public:
    /// Add new page to free-space-map
    void FsmAddPage(const TPgBlobPgPt& Pt, const uint16& FreeSpace);
    /// Update existing page inside free-space-map
    void FsmUpdatePage(const TPgBlobPgPt& Pt, const uint16& FreeSpace);
    /// Find page with most open space
    /// Returns false if no such page, true otherwise
    /// If page exists, pointer to it is stored into sent parameter
    bool FsmGetFreePage(int RequiredSpace, TPgBlobPgPt& Pg);
    /// Get number of stored elements
    int Len() const { return MaxFSpace.Len(); }
    /// Clears internal data
    void Clr() { PtH.Clr(); MaxFSpace.Clr(); }
    /// Save to output stream
    void Save(TSOut& SOut) const {
        PtH.Save(SOut); MaxFSpace.Save(SOut);
    }
    /// Load from input stream
    void Load(TSIn& SIn) {
        PtH.Load(SIn); MaxFSpace.Load(SIn);
    }
    /// Get element at given position
    const TPgBlobPgPt& GetVal(int RecN) const { return MaxFSpace.GetPtOf(RecN); }
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
    static char* EmptyPage;

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
    int SavePage(const uint32& Page, const void* Bf, int Len = -1);
    /// Reserve new space in the file. Returns -1 if file is full.
    long CreateNewPage();
};

////////////////////////////////////////////////////////////
/// Multi-file paged-BLOB-storage with cache.
/// Has no clue about the meaning of the data in pages.
class TPgBlob {
private:
    static const int MxBlobFLen;

protected:
    /// Housekeeping record for each loaded page
    struct LoadedPage {
    public:
        /// Blob pointer of this page
        TPgBlobPgPt Pt;
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
        /// Test if given buffer could be stored to page
        bool CanStoreBf(int Len) { return GetFreeMem() >= Len + (int)sizeof(TPgBlobPageItem); }
    };

    /// File name
    TStr FNm;
    /// File access
    TFAccess Access;
    /// Individual files that comprise this BLOB storage
    TVec<PPgBlobFile> Files;
    /// Pointers for loaded pages
    THash<TPgBlobPgPt, int> LoadedPagesH;
    /// Pointers for loaded pages
    TVec<LoadedPage> LoadedPages;
    /// Heap structure that keeps track of free space in pages
    TPgBlobFsm Fsm;

    /// Next item in LRU list - this one was accessed last
    int LruFirst;
    /// Previous item in LRU list - the next candidate for eviction
    int LruLast;

    /// Vector of allocated extents
    TVec<TMemBase> Extents;
    /// Number of loaded pages in last extent
    int LastExtentCnt;

    ///// Memory buffer - cache
    /// Maximal number of loaded pages
    uint64 MxLoadedPages;

    /// Returns starting address of page in Bf
    char* GetPageBf(int Pg) {
        return
            Extents[Pg / PG_EXTENT_PCOUNT].GetBf() +
            PG_PAGE_SIZE *(Pg % PG_EXTENT_PCOUNT);
    }

    // Method for handling LRU list ///////////////////////////////////

    /// remove given page from LRU list
    void UnlistFromLru(int Pg);
    /// move given page to the start of LRU list
    void MoveToStartLru(int Pg);
    /// move given page to the end of LRU list - so that it is evicted first
    void MoveToEndLru(int Pg);
    /// insert given (new) page to the start of LRU list
    void EnlistToStartLru(int Pg);
    /// insert given (new) page to the end of LRU list
    void EnlistToEndLru(int Pg);

    /// Evicts last possible page from cache.
    int Evict();

    /// Save main file
    void SaveMain();
    /// Load main file
    void LoadMain();
    /// Find which child files exist
    void DetectSegments();

    // Methods for handling page cache //////////////////////////////////

    /// This method tells if given page should be stored to disk.
    bool ShouldSavePage(int Pg) { return ShouldSavePageP(GetPageBf(Pg)); }
    /// This method tells if given page can be evicted from cache.
    bool CanEvictPage(int Pg) { return CanEvictPageP(GetPageBf(Pg)); }
    /// This method should be overridden in derived class to tell
    /// if given page should be stored to disk.
    bool ShouldSavePageP(char* Pt) { return ((TPgHeader*)Pt)->IsDirty(); }
    /// This method should be overridden in derived class to tell
    /// if given page can be evicted from cache.
    bool CanEvictPageP(char* Pt) { return !((TPgHeader*)Pt)->IsLock(); }
    /// Load given page into memory
    char* LoadPage(const TPgBlobPgPt& Pt, const bool& LoadData = true);
    /// Create new page and return pointers to it
    void CreateNewPage(TPgBlobPgPt& Pt, char** Bf);

    // Methods for manupulating raw page //////////////////////////////

    /// Initialize new page.
    static void InitPageP(char* Pt);
    /// Get pointer to item record - in it are offset and length
    static TPgBlobPageItem* GetItemRec(char* Pg, uint16 ItemIndex);
    /// Add given buffer to page, return item-index
    static uint16 AddItem(char* Pg, const char* Bf, const int BfL);
    /// Retrieve buffer from specified page
    static void GetItem(char* Pg, uint16 ItemIndex, char** Bf, int& BfL);
    /// Delete buffer from specified page
    static void DeleteItem(char* Pg, uint16 ItemIndex);
    /// Add given buffer to page, to existing item that has length 0
    static void ChangeItem(
        char* Pg, uint16 ItemIndex, const char* Bf, const int BfL);

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

    /// Store new BLOB to storage
    TPgBlobPt Put(const char* Bf, const int& BfL);
    /// Store existing BLOB to storage
    TPgBlobPt Put(const char* Bf, const int& BfL, const TPgBlobPt& Pt);
    /// Marks page as dirty - data inside was written directly
    void SetDirty(const TPgBlobPt& Pt);
    /// Retrieve BLOB from storage
    TThinMIn Get(const TPgBlobPt& Pt);
    /// Delete BLOB from storage
    void Del(const TPgBlobPt& Pt);
    /// Retrieve BLOB from storage as TMemBase
    TMemBase GetMemBase(const TPgBlobPt& Pt);
    /// Loads all pages into cache- cache must be big enough
    void LoadAll();
    /// Clear all contents
    void Clr();

    /// Save part of the data, given time-window
    void PartialFlush(int WndInMsec = 500);
    /// Retrieve statistics for this object
    PJsonVal GetStats();

    static void PrintHeaderInfo(char* Pg);
    // return the number of items in header as well as the number of empty items
    static void GetHeaderOverhead(char* Pg, int& Items, int& EmptyItems);
    // go over all loaded pages and compute the overheads
    void GetOverheads(int& Items, int& EmptyItems);

    /// Returns maximal BLOB length that can be stored in single page
    int GetMxBlobLen() const { return PG_PAGE_SIZE - sizeof(TPgHeader) - sizeof(TPgBlobPageItem); }

#ifdef XTEST
    friend class XTest;
#endif
};

#endif
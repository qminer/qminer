/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

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

char* TPgBlobFile::EmptyPage = NULL;

/// Private constructor
TPgBlobFile::TPgBlobFile(
    const TStr& _FNm, const TFAccess& _Access, const uint32& _MxSegLen) {

    // initialize the array used as the empty page
    if (EmptyPage == NULL) {
        EmptyPage = new char[PG_PAGE_SIZE];
        memset(EmptyPage, 0, PG_PAGE_SIZE);
    }
    Access = _Access;
    FNm = _FNm;
    MxFileLen = _MxSegLen;

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
    SetFPos(Page * PG_PAGE_SIZE);
    EAssertR(
        fread(Bf, 1, PG_PAGE_SIZE, FileId) == PG_PAGE_SIZE,
        "Error reading file '" + TStr(FNm) + "'.");
    return 0;
}

/// Save buffer to page within the file
int TPgBlobFile::SavePage(const uint32& Page, const void* Bf, int Len) {
    SetFPos(Page * PG_PAGE_SIZE);
    Len = (Len <= 0 ? PG_PAGE_SIZE : Len);
    EAssertR(
        (Access != TFAccess::faRdOnly) && (int)fwrite(Bf, 1, Len, FileId) == Len,
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
long TPgBlobFile::CreateNewPage() {
    EAssertR(
        (Access != TFAccess::faRdOnly) && (fseek(FileId, 0, SEEK_END) == 0),
        "Error seeking into file '" + TStr(FNm) + "' - " + TStr::Fmt("%d", errno));
    long len = ftell(FileId);
    if (MxFileLen > 0 && len >= MxFileLen) {
        return -1;
    }
    // write to the end of file - take what-ever chunk of memory
    //memset(&tc, 0, PG_PAGE_SIZE);
    //char tc[PG_PAGE_SIZE];
    size_t written = fwrite(EmptyPage, PG_PAGE_SIZE, 1, FileId);
    EAssertR(written == 1,
        "Error writing file '" + TStr(FNm) + "'.");
    return len / PG_PAGE_SIZE;
}

///////////////////////////////////////////////////////////////////////////

const int TPgBlob::MxBlobFLen = 2000000000;

/// Add given buffer to page, to existing item that has length 0
void TPgBlob::ChangeItem(
    char* Pg, uint16 ItemIndex, const char* Bf, const int BfL) {
    TPgHeader* Header = (TPgHeader*)Pg;
    EAssert(BfL + (int)sizeof(TPgBlobPageItem) <= Header->GetFreeMem());

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
uint16 TPgBlob::AddItem(char* Pg, const char* Bf, const int BfL) {
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
void TPgBlob::GetItem(char* Pg, uint16 ItemIndex, char** Bf, int& BfL) {
    TPgBlobPageItem* Item = GetItemRec(Pg, ItemIndex);
    BfL = Item->Len;
    *Bf = Pg + Item->Offset;
}

/// Delete buffer from specified page
void TPgBlob::DeleteItem(char* Pg, uint16 ItemIndex) {
    TPgBlobPageItem* Item = GetItemRec(Pg, ItemIndex);
    int PackOffset = Item->Len;
    TPgHeader* Header = (TPgHeader*)Pg;
    char* OldFreeEnd = Pg + Header->OffsetFreeEnd;
    int Len = 0;
    for (int i = 0; i < Header->ItemCount; i++) {
        TPgBlobPageItem* ItemX = GetItemRec(Pg, i);
        if (ItemX->Offset >= Item->Offset) {
            continue;
        }
        if (ItemX->Len == 0) {
            continue;
        }
        Len += ItemX->Len;
        ItemX->Offset += PackOffset;
    }
    Header->OffsetFreeEnd += PackOffset;
    memmove(OldFreeEnd + PackOffset, OldFreeEnd, Len);

    Item->Offset += Item->Len;
    Item->Len = 0;
    Header->SetDirty(true);

    // optimization - if all items are deleted, mark as empty page
    bool all_deleted = true;
    for (int i = 0; i < Header->ItemCount; i++) {
        if (GetItemRec(Pg, i)->Len == 0) {
            all_deleted = false;
            break;
        }
    }
    if (all_deleted) {
        Header->OffsetFreeStart = sizeof(TPgHeader);
        Header->OffsetFreeEnd = PG_PAGE_SIZE;
        Header->ItemCount = 0;
    }

    /*printf("##Delete end\n");
    PrintHeaderInfo(Pg);*/
}

/// Get pointer to item record - in it are offset and length
TPgBlob::TPgBlobPageItem* TPgBlob::GetItemRec(
    char* Pg, uint16 ItemIndex) {
    return (TPgBlob::TPgBlobPageItem*)(
        Pg
        + sizeof(TPgBlob::TPgHeader)
        + ItemIndex * sizeof(TPgBlobPageItem));
}

/// Private constructor
TPgBlob::TPgBlob(const TStr& _FNm, const TFAccess& _Access,
    const uint64& CacheSize) {
    EAssertR(CacheSize >= PG_PAGE_SIZE, "Invalid cache size for TPgBlob.");

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
    LastExtentCnt = PG_EXTENT_PCOUNT; // this means the "last" extent is full, so use new one
    MxLoadedPages = CacheSize / PG_PAGE_SIZE;
    LruFirst = LruLast = -1;
}

/// Destructor
TPgBlob::~TPgBlob() {
    if (Access != TFAccess::faRdOnly) {
        for (int i = 0; i < LoadedPages.Len(); i++) {
            if (ShouldSavePage(i)) {
                LoadedPage& a = LoadedPages[i];
                Files[a.Pt.GetFIx()]->SavePage(a.Pt.GetPg(), GetPageBf(i));
            }
        }
        SaveMain();
        Files.Clr();
    }
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
        Files.Add(TPgBlobFile::New(FNmChild, Access, MxBlobFLen));
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
    if (LruLast < 0) {
        LruLast = Pg;
    }
}

/// insert given (new) page to the end of LRU list
void TPgBlob::EnlistToEndLru(int Pg) {
    LoadedPage& a = LoadedPages[Pg];
    a.LruPrev = LruLast;
    a.LruNext = -1;
    if (LruLast >= 0) {
        LoadedPages[LruLast].LruNext = Pg;
    }
    LruLast = Pg;
    if (LruFirst < 0) {
        LruFirst = Pg;
    }
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
        EnlistToEndLru(Pg);
    }
}

/// move given page to the end of LRU list - so that it is evicted first
void TPgBlob::MoveToEndLru(int Pg) {
    LoadedPage& a = LoadedPages[Pg];
    if (LruLast < 0) { // empty LRU list
        a.LruNext = a.LruPrev = -1;
        LruFirst = LruLast = Pg;
    } else if (LruLast == Pg) {
        // it's ok, already at end LRU
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
    char* PgPt = GetPageBf(Pg);
    if (ShouldSavePageP(PgPt)) {
        int Len = (((TPgHeader*)PgPt)->ItemCount > 0 ? -1 : sizeof(TPgHeader));
        Files[a.Pt.GetFIx()]->SavePage(a.Pt.GetPg(), PgPt, Len);
    } else {
    }
    return Pg;
}

/// Load given page into memory
char* TPgBlob::LoadPage(const TPgBlobPgPt& Pt, const bool& LoadData) {
    int Pg;
    if (LoadedPagesH.IsKeyGetDat(Pt, Pg)) { // is page in cache
        MoveToStartLru(Pg);
        return GetPageBf(Pg);
    }
    if ((uint64)LoadedPages.Len() == MxLoadedPages) {
        // evict last page + load new page
        Pg = Evict();
        LoadedPage& a = LoadedPages[Pg];
        if (LoadData) {
            Files[Pt.GetFIx()]->LoadPage(Pt.GetPg(), GetPageBf(Pg));
        }
        a.Pt = Pt;
        EnlistToStartLru(Pg);
        LoadedPagesH.AddDat(Pt, Pg);
    } else {
        // simply load the page
        LastExtentCnt++;
        if (LastExtentCnt >= PG_EXTENT_PCOUNT) {
            Extents.Add();
            Extents.Last() = TMemBase(PG_EXTENT_SIZE);
            LastExtentCnt = 0;
        }
        Pg = LoadedPages.Add();
        LoadedPage& a = LoadedPages[Pg];
        if (LoadData) {
            Files[Pt.GetFIx()]->LoadPage(Pt.GetPg(), GetPageBf(Pg));
        }
        a.Pt = Pt;
        EnlistToStartLru(Pg);
        LoadedPagesH.AddDat(Pt, Pg);
    }
    char* PgPt = GetPageBf(Pg);
    ((TPgHeader*)PgPt)->SetDirty(false);
    return PgPt;
}

/// Create new page and return pointers to it
void TPgBlob::CreateNewPage(TPgBlobPgPt& Pt, char** Bf) {
    // determine if last file is empty
    if (Files.Len() > 0) {
        // try to add to last file
        long Pg = Files.Last()->CreateNewPage();
        if (Pg >= 0) {
            Pt.Set(Files.Len() - 1, (uint32)Pg);
            *Bf = LoadPage(Pt, false);
            InitPageP(*Bf);
            return;
        }
    }
    TStr NewFNm = FNm + ".bin" + TStr::GetNrNumFExt(Files.Len());
    Files.Add(TPgBlobFile::New(NewFNm, TFAccess::faCreate, MxBlobFLen));
    long Pg = Files.Last()->CreateNewPage();
    EAssert(Pg >= 0);
    Pt.Set(Files.Len() - 1, (uint32)Pg);
    *Bf = LoadPage(Pt, false);
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
void TPgBlob::InitPageP(char* Pt) {
    TPgHeader* Pt2 = (TPgHeader*)Pt;
    Pt2->Flags = PgHeaderDirtyFlag; // not saved yet
    Pt2->PageSize = PG_PAGE_SIZE;
    Pt2->PageVersion = 1;
    Pt2->OffsetFreeStart = sizeof(TPgHeader);
    Pt2->OffsetFreeEnd = PG_PAGE_SIZE;
    Pt2->ItemCount = 0;
}

/// Store BLOB to storage
TPgBlobPt TPgBlob::Put(const char* Bf, const int& BfL) {
    IAssert(Access != TFAccess::faRdOnly);

    // find page
    TPgBlobPgPt PgPt;
    char* PgBf = NULL;
    TPgHeader* PgH = NULL;
    bool is_new_page = false;

    // scan last 5 used pages if there is some space
    // the logic is that during batch inserts we should reuse
    // recently-used pages so that data is packed together
    int LoadedPage = LruFirst;
    for (int i = 0; i < 5 && LoadedPage != -1; i++) {
        char* PgBfTmp = GetPageBf(LoadedPage);
        PgH = (TPgHeader*)PgBfTmp;
        if (PgH->CanStoreBf(BfL)) {
            PgBf = PgBfTmp;
            PgPt = LoadedPages[LoadedPage].Pt;
            break;
        }
        LoadedPage = LoadedPages[LoadedPage].LruNext;
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
    const char* Bf, const int& BfL, const TPgBlobPt& Pt) {
    IAssert(Access != TFAccess::faRdOnly);

    // find page
    char* PgBf = NULL;
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

    } else if (existing_size + PgH->GetFreeMem() >= BfL + (int)sizeof(TPgBlobPageItem)) {
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
        Fsm.FsmAddPage(PgPt, PgH->GetFreeMem());
        return Pt2;
    }
}

/// Retrieve BLOB from storage
TThinMIn TPgBlob::Get(const TPgBlobPt& Pt) {
    char* Pg = LoadPage(Pt);
    TPgBlobPageItem* Item = GetItemRec(Pg, Pt.GetIIx());
    char* Data;
    int Len = Item->Len;
    GetItem(Pg, Pt.GetIIx(), &Data, Len);
    return TThinMIn(Data, Len);
}

/// Delete BLOB from storage
void TPgBlob::Del(const TPgBlobPt& Pt) {
    IAssert(Access != TFAccess::faRdOnly);

    // find page
    TPgBlobPgPt PgPt = Pt;
    char* PgBf = LoadPage(PgPt);
    TPgHeader* PgH = (TPgHeader*)PgBf;

    DeleteItem(PgBf, Pt.GetIIx());
    if (PgH->ItemCount == 0) {
        // optimization - empty pages are to be flushed as fast a s possible
        MoveToEndLru(Pt.GetPg());
    }
    Fsm.FsmUpdatePage(PgPt, PgH->GetFreeMem());
}

/// Retrieve BLOB from storage
TMemBase TPgBlob::GetMemBase(const TPgBlobPt& Pt) {
    return Get(Pt).GetMemBase();
}

/// Loads all pages into cache - cache must be big enough
void TPgBlob::LoadAll() {
    for (int i = 0; i < Fsm.Len(); i++) {
        LoadPage(Fsm.GetVal(i));
    }
}

/// Loads all pages into cache - cache must be big enough
void TPgBlob::Clr() {
    Extents.Clr();
    LoadedPagesH.Clr();
    LoadedPages.Clr();
    Fsm.Clr();
    Files.Clr();
    TFile::DelWc(FNm + ".bin*"); // delete all child files
    LastExtentCnt = PG_EXTENT_PCOUNT;
    LruFirst = LruLast = -1;
    SaveMain();
}

/// Save part of the data, given time-window
void TPgBlob::PartialFlush(int WndInMsec) {
    if (Access == TFAccess::faRdOnly)
        return;
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
    IAssert(Access != TFAccess::faRdOnly);
    char* Pg = LoadPage(Pt);
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
    res->AddToObj("page_size", PG_PAGE_SIZE);
    res->AddToObj("loaded_pages", LoadedPages.Len());
    res->AddToObj("dirty_pages", dirty);
    res->AddToObj("loaded_extents", Extents.Len());
    res->AddToObj("cache_size", PG_EXTENT_SIZE * Extents.Len());
    return res;
}


void TPgBlob::PrintHeaderInfo(char* Pg) {
    TPgHeader* Header = (TPgHeader*) Pg;
    printf("ItemCount %d, OffsetFreeEnd %d, OffsetFreeStart %d \n", Header->ItemCount, Header->OffsetFreeEnd, Header->OffsetFreeStart);
    for (int i = 0; i < Header->ItemCount; i++) {
        TPgBlobPageItem* Item = GetItemRec(Pg, i);
        printf("Len %d, Offset %d \n", Item->Len, Item->Offset);
    }
}

void TPgBlob::GetHeaderOverhead(char* Pg, int& Items, int& EmptyItems)
{
    TPgHeader* Header = (TPgHeader*) Pg;
    Items = Header->ItemCount;
    EmptyItems = 0;
    for (int i = 0; i < Header->ItemCount; i++) {
        TPgBlobPageItem* Item = GetItemRec(Pg, i);
        if (Item->Len == 0)
            EmptyItems++;
    }
}

void TPgBlob::GetOverheads(int& Items, int& EmptyItems)
{
    Items = 0;
    EmptyItems = 0;
    for (int KeyId = LoadedPagesH.FFirstKeyId(); LoadedPagesH.FNextKeyId(KeyId); ) {
        int Pg = LoadedPagesH[KeyId];
        char * PgBuff = GetPageBf(Pg);
        int PgItems = 0;
        int PgEmptyItems = 0;
        GetHeaderOverhead(PgBuff, PgItems, PgEmptyItems);
        Items += PgItems;
        EmptyItems += PgEmptyItems;
    }
}

///////////////////////////////////////////////////////////////////////////

/// Add new page to free-space-map
void TPgBlobFsm::FsmAddPage(const TPgBlobPgPt& Pt, const uint16& FreeSpace) {
    int index = MaxFSpace.Add(FreeSpace, Pt);
    PtH.AddDat(Pt, index);
}

/// Update existing page inside free-space-map
void TPgBlobFsm::FsmUpdatePage(const TPgBlobPgPt& Pt, const uint16& FreeSpace) {
    MaxFSpace.Change(PtH.GetDat(Pt), FreeSpace);
}

/// Find page with most open space.
/// Returns false if no such page, true otherwise
/// If page exists, pointer to it is stored into sent parameter
bool TPgBlobFsm::FsmGetFreePage(int RequiredSpace, TPgBlobPgPt& PgPt) {
    int index_max = MaxFSpace.GetIndexOfMax();
    if (index_max < 0 || MaxFSpace.GetVal(index_max) < RequiredSpace)
        return false;
    PgPt = MaxFSpace.GetPtOf(index_max);
    return true;
}

//////////////////////////////////////////////////////

/// Add new record to the structure
int TBinTreeMaxVals::Add(const uint16& Val, const TPgBlobPgPt& Pt) {
    int res = Layers[0].Add(Val);
    Pts.Add(Pt);
    if (res == 0) return res;

    int index = res;
    int layer = 0;
    while (true) {
        layer++;
        if (layer >= Layers.Len()) {
            if (index == 1) {
                Layers.Add();
                Layers.Last().Add(MAX(Layers[layer - 1][0], Layers[layer - 1][1]));
            }
            break;
        }
        int index2 = index / 2;
        if (Layers[layer].Len() <= index2) {
            Layers[layer].Add(Val);
        } else if (Layers[layer][index2] < Val) {
            Layers[layer][index2] = Val;
        } else {
            break;
        }
        index = index2;
    }

    return res;
}

/// Get index of item with max value
int TBinTreeMaxVals::GetIndexOfMax()const {
    if (Layers[0].Len() == 0) { // no data yet
        return -1;
    }
    int layer = Layers.Len() - 1; // search from top layer
    int index = 0;
    uint target = Layers[layer][index];
    while (layer > 0) {
        layer--;
        index *= 2; // go to first child
        if (Layers[layer][index] != target) {
            index++; // ok, it must the second child
        }
    }
    return index;
}

/// Change value at given position
void TBinTreeMaxVals::Change(const int& RecN, const uint16& Val) {
    Layers[0][RecN] = Val;
    int layer = 1;
    int index = RecN;
    while (layer < Layers.Len()) {
        int index2 = index / 2;
        if (Layers[layer - 1].Len() > 2 * index2 + 1) {
            Layers[layer][index2] = MAX(Layers[layer - 1][2 * index2], Layers[layer - 1][2 * index2 + 1]);
        } else {
            Layers[layer][index2] = Layers[layer - 1][index];
        }
        index = index2;
        layer++;
    }
}

#ifdef XTEST
/// For debugging purposes
void TBinTreeMaxVals::Print() {
    printf("------------------------\n");
    for (int i = 0; i < Layers.Len(); i++) {
        for (int j = 0; j < Layers[i].Len(); j++) {
            printf("%d ", Layers[i][j]);
        }
        printf("\n");
    }
}
#endif

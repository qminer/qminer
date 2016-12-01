/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "qminer_srv.h"

namespace TQm {

///////////////////////////////////////////
// QMiner-Server-Function
void TSrvFun::RegDefFun(const TWPt<TBase>& Base, TSAppSrvFunV& SrvFunV) {
    // register qminer functions
    SrvFunV.Add(TSfExit::New(Base));
    SrvFunV.Add(TSfFail::New(Base));
    SrvFunV.Add(TSfStores::New(Base));
    SrvFunV.Add(TSfWordVoc::New(Base));
    SrvFunV.Add(TSfStoreRec::New(Base));
    SrvFunV.Add(TSfPartialFlush::New(Base));
}

///////////////////////////////////////////
// QMiner-Server-Function-Stream-Aggregator
TStr TSfExit::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    // read return code (default 0)
    const int ReturnCode = GetFldInt(FldNmValPrV, "return", 0);
    // set return code
    TEnv::ReturnCode = ReturnCode;
    // stop the server loop
    TLoop::Stop();
    // return we did well
    PJsonVal JsonVal = TJsonVal::NewObj("return", TEnv::ReturnCode);
    return TJsonVal::GetStrFromVal(JsonVal);
}

///////////////////////////////////////////
// QMiner-Server-Function-Stores
TStr TSfStores::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    const TWPt<TBase>& Base = GetBase();
    const int Stores = Base->GetStores();
    TJsonValV StoreValV;
    for (int StoreN = 0; StoreN < Stores; StoreN++) {
        TWPt<TStore> Store = Base->GetStoreByStoreN(StoreN);
        StoreValV.Add(Store->GetStoreJson(Base));
    }
    PJsonVal JsonVal = TJsonVal::NewArr(StoreValV);
    return TJsonVal::GetStrFromVal(JsonVal);
}

///////////////////////////////////////////
// QMiner-Server-Function-WordVoc
void TSfWordVoc::GetWordVoc(const TStrKdV& FldNmValPrV, TStrIntPrV& WordStrFqV) {
    TWPt<TIndexVoc> IndexVoc = Base->GetIndexVoc();
    // read key id
    int KeyId = -1;
    if (IsFldNm(FldNmValPrV, "keyid")) {
        KeyId = GetFldInt(FldNmValPrV, "keyid", -1);
        if (!IndexVoc->IsKeyId(KeyId) || !IndexVoc->IsWordVoc(KeyId)) {
            throw TQmExcept::New(TStr::Fmt("Wrong keyid='%d': unknown ID or no vocabular!", KeyId));
        }
    } else if (IsFldNm(FldNmValPrV, "store") && IsFldNm(FldNmValPrV, "key")) {
        // parse store
        TStr StoreNm = GetFldVal(FldNmValPrV, "store");
        if (!Base->IsStoreNm(StoreNm)) { throw TQmExcept::New("Unknown store " + StoreNm); }
        const uint StoreId = Base->GetStoreByStoreNm(StoreNm)->GetStoreId();
        // parse key
        TStr KeyNm = GetFldVal(FldNmValPrV, "key");
        if (!IndexVoc->IsKeyNm(StoreId, KeyNm)) { throw TQmExcept::New("Unknown key " + StoreNm + "." + KeyNm); }
        KeyId = IndexVoc->GetKeyId(StoreId, KeyNm);
    } else {
        throw TQmExcept::New("No specified key (either keyid or [store,key] names)");
    }
    // get all the words
    IndexVoc->GetAllWordStrFqV(KeyId, WordStrFqV);
    // filter down
    TIntStrPrV WordFqStrV;
    if (IsFldNm(FldNmValPrV, "prefix")) { 
        TStr PrefixStr = GetFldVal(FldNmValPrV, "prefix");
        if (PrefixStr.Empty()) {
            // return all
            GetSwitchedPrV<TStr, TInt>(WordStrFqV, WordFqStrV);
        } else {
            // find all the matching words
            const int Words = WordStrFqV.Len();
            for (int WordN = 0; WordN < Words; WordN++) {
                const TStr& WordStr = WordStrFqV[WordN].Val1;
                if (WordStr.StartsWith(PrefixStr)) {
                    const int WordFq = WordStrFqV[WordN].Val2;
                    WordFqStrV.Add(TIntStrPr(WordFq, WordStr));
                }
            }
        }
    } else if (IsFldNm(FldNmValPrV, "term")) {
        TStr TermStr = GetFldVal(FldNmValPrV, "term");
        TStrV TermV; TermStr.SplitOnAllAnyCh(" ,;", TermV);
        if (TermV.Empty()) { 
            // return all
            GetSwitchedPrV<TStr, TInt>(WordStrFqV, WordFqStrV);
        } else {
            // find all the matching words
            const int Words = WordStrFqV.Len();
            for (int WordN = 0; WordN < Words; WordN++) {
                const TStr& WordStr = WordStrFqV[WordN].Val1;
                int Terms = 0;
                for (int TermN = 0; TermN < TermV.Len(); TermN++) {
                    if (WordStr.IsStrIn(TermV[TermN])) { Terms++; }
                    else { break; } // one missing is enough evidence to skip word
                }
                if (Terms == TermV.Len()) { 
                    const int WordFq = WordStrFqV[WordN].Val2;
                    WordFqStrV.Add(TIntStrPr(WordFq, WordStr));
                }
            }
        }
    } else {
        GetSwitchedPrV<TStr, TInt>(WordStrFqV, WordFqStrV);
    }
    // sort limit
    WordFqStrV.Sort(false);
    // limit if necessary
    const int MxWords = GetFldInt(FldNmValPrV, "limit", -1);
    if (MxWords != -1) { WordFqStrV.Trunc(MxWords); }
    // output the results 
    GetSwitchedPrV<TInt, TStr>(WordFqStrV, WordStrFqV);
}

TStr TSfWordVoc::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    try {
        // get matching words
        TStrIntPrV WordStrFqV; GetWordVoc(FldNmValPrV, WordStrFqV);
        // output the results 
        TJsonValV WordValV;     
        for (int WordN = 0; WordN < WordStrFqV.Len(); WordN++) {
            PJsonVal WordVal = TJsonVal::NewObj();
            WordVal->AddToObj("str", WordStrFqV[WordN].Val1);
            WordVal->AddToObj("fq", WordStrFqV[WordN].Val2);
            WordValV.Add(WordVal);
        }
        PJsonVal JsonVal = TJsonVal::NewArr(WordValV);
        return TJsonVal::GetStrFromVal(JsonVal);
    } catch (PExcept Except) {
        return GetErrorJsonRes(Except->GetMsgStr());
    }
}

///////////////////////////////////////////
// QMiner-Server-Function-Record
TWPt<TStore> TSfStoreRec::GetStore(const TStrKdV& FldNmValPrV) const {
    if (IsFldNm(FldNmValPrV, "storeid")) {
        TStr StoreIdStr = GetFldVal(FldNmValPrV, "storeid");
        QmAssertR(StoreIdStr.IsInt(), "Missing or invalid store ID " + StoreIdStr);
        const uint StoreId = StoreIdStr.GetUInt();
        QmAssertR(Base->IsStoreId(StoreId), "No store with ID " + StoreIdStr);
        return Base->GetStoreByStoreId(StoreId);
    } else if (IsFldNm(FldNmValPrV, "store")) {
        TStr StoreNm = GetFldVal(FldNmValPrV, "store");
        QmAssertR(Base->IsStoreNm(StoreNm), "No store with name " + StoreNm);
        return Base->GetStoreByStoreNm(StoreNm);
    }
    throw TQmExcept::New("No 'store' or 'storeid' parameter to define the store");
}

TRec TSfStoreRec::GetRec(const TStrKdV& FldNmValPrV, const TWPt<TStore>& Store) const {
    if (IsFldNm(FldNmValPrV, "recid")) {
        TStr RecIdStr = GetFldVal(FldNmValPrV, "recid");
        QmAssertR(RecIdStr.IsInt(), "Missing or invalid record ID " + RecIdStr);
        const uint64 RecId = RecIdStr.GetUInt64();
        QmAssertR(Store->IsRecId(RecId), "No record with ID " + RecIdStr);
        return Store->GetRec(RecId);
    } else if (IsFldNm(FldNmValPrV, "rec")) {
        TStr RecNm = GetFldVal(FldNmValPrV, "rec");
        QmAssertR(Store->IsRecNm(RecNm), "No record with name " + RecNm);
        return Store->GetRec(RecNm);
    }
    throw TQmExcept::New("No 'rec' or 'recid' parameter to define the store");
}

TStr TSfStoreRec::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    TWPt<TStore> Store = GetStore(FldNmValPrV);
    TRec Rec = GetRec(FldNmValPrV, Store);
    const bool JoinRecsP = IsFldNmVal(FldNmValPrV, "join", "T");
    return TJsonVal::GetStrFromVal(Rec.GetJson(Base, true, true, JoinRecsP));
}

///////////////////////////////////////////
// QMiner-Server-Function-Debug
TStr TSfDebug::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    // dump stuff
    if (IsFldNm(FldNmValPrV, "index")) { 
        Base->PrintIndex("dumpIndex.txt", GetFldVal(FldNmValPrV, "index") == "sort"); 
    }
    if (IsFldNm(FldNmValPrV, "indexvoc")) { 
        Base->PrintIndexVoc("dumpIndexVoc.txt"); 
    }
    if (IsFldNm(FldNmValPrV, "store")) { 
        Base->PrintStores("dumpStores.txt", GetFldVal(FldNmValPrV, "store") == "full");
    }
    // report done
    return TJsonVal::GetStrFromVal(TJsonVal::NewObj("Debug", "Done"));
}

///////////////////////////////////////////
// QMiner-Server-Function-Debug
TStr TSfPartialFlush::ExecJSon(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    int wnd_in_msec = 500;
    if (IsFldNm(FldNmValPrV, "wnd_in_msec")) {
        wnd_in_msec = GetFldVal(FldNmValPrV, "wnd_in_msec").GetInt();
    }
    int res = Base->PartialFlush(wnd_in_msec);
    // report done
    return TJsonVal::GetStrFromVal(TJsonVal::NewObj("SavedToDisk", res));
}
}
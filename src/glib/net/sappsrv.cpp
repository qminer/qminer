/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

//////////////////////////////////////
// Simple-App-Server-Request-Environment
void TSAppSrvRqEnv::ExecFun(const TStr& FunNm, const TStrKdV& FldNmValPrV) {
    EAssert(FunNmToFunH.IsKey(FunNm));
    FunNmToFunH.GetDat(FunNm)->Exec(FldNmValPrV, this); 
}

//////////////////////////////////////
// Simple-App-Server-Function
bool TSAppSrvFun::IsFldNm(const TStrKdV& FldNmValPrV, const TStr& FldNm) {
    const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
    return (ValN != -1);
}

TStr TSAppSrvFun::GetFldVal(const TStrKdV& FldNmValPrV,
        const TStr& FldNm, const TStr& DefFldVal) {

    const int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
    return (ValN == -1) ? DefFldVal : FldNmValPrV[ValN].Dat;    
}

int TSAppSrvFun::GetFldInt(const TStrKdV& FldNmValPrV, const TStr& FldNm) {
    EAssertR(IsFldNm(FldNmValPrV, FldNm), "Missing parameter '" + FldNm + "'");
    TStr IntStr = GetFldVal(FldNmValPrV, FldNm, "");
    EAssertR(IntStr.IsInt(), "Parameter '" + FldNm + "' not a number");
    return IntStr.GetInt();
}

int TSAppSrvFun::GetFldInt(const TStrKdV& FldNmValPrV, const TStr& FldNm, const int& DefInt) {
    if (!IsFldNm(FldNmValPrV, FldNm)) { return DefInt; }
    TStr IntStr = GetFldVal(FldNmValPrV, FldNm, "");
    EAssertR(IntStr.IsInt(), "Parameter '" + FldNm + "' not a number");
    return IntStr.GetInt();
}

double TSAppSrvFun::GetFldFlt(const TStrKdV& FldNmValPrV, const TStr& FldNm, const double& DefFlt) {
    if (!IsFldNm(FldNmValPrV, FldNm)) { return DefFlt; }
    TStr IntStr = GetFldVal(FldNmValPrV, FldNm, "");
    EAssertR(IntStr.IsFlt(), "Parameter '" + FldNm + "' not a number");
    return IntStr.GetFlt();
}

bool TSAppSrvFun::GetFldBool(const TStrKdV& FldNmValPrV, const TStr& FldNm, const bool& DefVal)
{
    if (!IsFldNm(FldNmValPrV, FldNm)) return DefVal;
    TStr Val = GetFldVal(FldNmValPrV, FldNm, "").GetLc();
    if (Val == "1" || Val == "true" || Val == "t") return true;
    return false;
}

uint64 TSAppSrvFun::GetFldUInt64(const TStrKdV& FldNmValPrV, const TStr& FldNm, const uint64& DefVal) {
    TStr IntStr = GetFldVal(FldNmValPrV, FldNm, TUInt64(DefVal).GetStr());
    EAssertR(IntStr.IsUInt64(), "Parameter '" + FldNm + "' not a number");
    return IntStr.GetUInt64();
}

void TSAppSrvFun::GetFldValV(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrV& FldValV) {
    FldValV.Clr();
    int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
    while (ValN != -1) {
        FldValV.Add(FldNmValPrV[ValN].Dat);
        ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
    }
}

void TSAppSrvFun::GetFldValSet(const TStrKdV& FldNmValPrV, const TStr& FldNm, TStrSet& FldValSet) {
    FldValSet.Clr();
    int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
    while (ValN != -1) {
        FldValSet.AddKey(FldNmValPrV[ValN].Dat);
        ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
    }
}

bool TSAppSrvFun::IsFldNmVal(const TStrKdV& FldNmValPrV,
        const TStr& FldNm, const TStr& FldVal) {

    int ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""));
    while (ValN != -1) {
        if (FldNmValPrV[ValN].Dat == FldVal) { return true; }
        ValN = FldNmValPrV.SearchForw(TStrKd(FldNm, ""), ValN + 1);
    }
    return false;
}

// if one or more items with FldNm already exist remove them first
// add to FldNmValPrV pair (FldNm, FldVal)
void TSAppSrvFun::SetFldNmVal(TStrKdV& FldNmValPrV, const TStr& FldNm, const TStr& FldVal)
{
    for (int N = FldNmValPrV.Len() - 1; N >= 0; N--) {
        if (FldNmValPrV[N].Key == FldNm)
            FldNmValPrV.Del(N);
    }
    FldNmValPrV.Add(TStrKd(FldNm, FldVal));
}

void TSAppSrvFun::SetFldNmInt(TStrKdV& FldNmValPrV, const TStr& FldNm, const int& FldVal)
{
    SetFldNmVal(FldNmValPrV, FldNm, TInt(FldVal).GetStr());
}

void TSAppSrvFun::SetFldNmBool(TStrKdV& FldNmValPrV, const TStr& FldNm, const bool& FldVal)
{
    SetFldNmVal(FldNmValPrV, FldNm, FldVal ? "true" : "false");
}

void TSAppSrvFun::SetFldNmFlt(TStrKdV& FldNmValPrV, const TStr& FldNm, const double& FldVal)
{
    SetFldNmVal(FldNmValPrV, FldNm, TFlt(FldVal).GetStr());
}

TStr TSAppSrvFun::XmlHdStr = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";

PXmlDoc TSAppSrvFun::GetErrorXmlRes(const TStr& ErrorStr) {
    return TXmlDoc::New(TXmlTok::New("error", ErrorStr)); 
}

TStr TSAppSrvFun::GetErrorJsonRes(const TStr& ErrorStr) {
    PJsonVal ErrorVal = TJsonVal::NewObj("error", TJsonVal::NewStr(ErrorStr));
    return TJsonVal::GetStrFromVal(ErrorVal);
}

void TSAppSrvFun::Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    const PNotify& Notify = RqEnv->GetWebSrv()->GetNotify();
    PHttpResp HttpResp;
    try {
        // log the call
        if (NotifyOnRequest)
            Notify->OnStatus(TStr::Fmt("RequestStart  %s", FunNm.CStr()));
        TTmStopWatch StopWatch(true);
        // execute the actual function, according to the type
        PSIn BodySIn; TStr ContTypeVal;
        if (GetFunOutType() == saotXml) {
            PXmlDoc ResXmlDoc = ExecXml(FldNmValPrV, RqEnv);        
            TStr ResXmlStr; ResXmlDoc->SaveStr(ResXmlStr);
            BodySIn = TMIn::New(XmlHdStr + ResXmlStr);
            ContTypeVal = THttp::TextXmlFldVal;
        } else if (GetFunOutType() == saotJSon) {
            TStr ResStr = ExecJSon(FldNmValPrV, RqEnv);
            BodySIn = TMIn::New(ResStr);
            ContTypeVal = THttp::AppJSonFldVal;
        } else {
            BodySIn = ExecSIn(FldNmValPrV, RqEnv, ContTypeVal);
        }
        if (ReportResponseSize)
            Notify->OnStatusFmt("Response size: %.1f KB", BodySIn->Len() / (double) TInt::Kilo);
        // log finish of the call
        if (NotifyOnRequest)
            Notify->OnStatus(TStr::Fmt("RequestFinish %s [request took %d ms]", FunNm.CStr(), StopWatch.GetMSecInt()));
        // prepare response
        HttpResp = THttpResp::New(THttp::OkStatusCd,
			ContTypeVal, false, BodySIn, TStr(), StopWatch.GetMSecInt());
    } catch (PExcept Except) {
        // known internal error
        Notify->OnStatusFmt("Exception: %s", Except->GetMsgStr().CStr());
        Notify->OnStatusFmt("Location: %s", Except->GetLocStr().CStr());
        TStr ResStr, ContTypeVal = THttp::TextPlainFldVal;
        if (GetFunOutType() == saotXml) {
            PXmlTok TopTok = TXmlTok::New("error");
            TopTok->AddSubTok(TXmlTok::New("message", Except->GetMsgStr()));
            TopTok->AddSubTok(TXmlTok::New("location", Except->GetLocStr()));
            PXmlDoc ErrorXmlDoc = TXmlDoc::New(TopTok); 
            ResStr = XmlHdStr + ErrorXmlDoc->SaveStr();
            ContTypeVal = THttp::TextXmlFldVal;
        } else if (GetFunOutType() == saotJSon) {
            PJsonVal ResVal = TJsonVal::NewObj();
            ResVal->AddToObj("message", Except->GetMsgStr());
            ResVal->AddToObj("location", Except->GetLocStr());
            ResStr = TJsonVal::NewObj("error", ResVal)->SaveStr();
            ContTypeVal = THttp::AppJSonFldVal;
        }
        // prepare response
        HttpResp = THttpResp::New(THttp::InternalErrStatusCd,
            ContTypeVal, false, TMIn::New(ResStr));
    } catch (...) {
        // unknown internal error
        TStr ResStr, ContTypeVal = THttp::TextPlainFldVal;
        if (GetFunOutType() == saotXml) {
            PXmlDoc ErrorXmlDoc = TXmlDoc::New(TXmlTok::New("error")); 
            ResStr = XmlHdStr + ErrorXmlDoc->SaveStr();
            ContTypeVal = THttp::TextXmlFldVal;
        } else if (GetFunOutType() == saotJSon) {
            ResStr = TJsonVal::NewObj("error", "Unknown")->SaveStr();
            ContTypeVal = THttp::AppJSonFldVal;
        }
        // prepare response
        HttpResp = THttpResp::New(THttp::InternalErrStatusCd,
            ContTypeVal, false, TMIn::New(ResStr));
    }

    if (LogRqToFile)
        LogReqRes(FldNmValPrV, HttpResp);
    // send response
    RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp); 
}

void TSAppSrvFun::LogReqRes(const TStrKdV& FldNmValPrV, const PHttpResp& HttpResp)
{
    if (LogRqFolder == "")
        return;
    try     {
        TDir::GenDir(LogRqFolder);
        TStr TimeNow = TTm::GetCurLocTm().GetWebLogDateTimeStr(true);
        TimeNow.ChangeChAll(':', '.');
        PSOut Output = TFOut::New(LogRqFolder + "/" + TimeNow + ".txt", false);
        Output->PutStr(FunNm.CStr()); Output->PutCh('\n');
        for (int N=0; N < FldNmValPrV.Len(); N++)
            Output->PutStrFmt("  %s: %s\n", FldNmValPrV[N].Key.CStr(), FldNmValPrV[N].Dat.CStr());
        Output->PutCh('\n');
        Output->PutStr(HttpResp->GetBodyAsStr(), false);
    } catch (...) {
        /*const PNotify& Notify = RqEnv->GetWebSrv()->GetNotify();
        Notify->OnStatus("Unable to log request for function '" + GetFunNm() + "'!");*/
    }
}


//////////////////////////////////////
// Http Request Serialization Info
THttpReqSerInfo::THttpReqSerInfo(const TStr& _UrlRel, const TStr& _UrlBase, const THttpRqMethod& _ReqMethod, const TMem& _Body) :
UrlRel(_UrlRel), UrlBase(_UrlBase), ReqMethod((THttpRqMethod) _ReqMethod), Body(_Body)
{
}

THttpReqSerInfo::THttpReqSerInfo(const PHttpRq& HttpRq)
{
    UrlRel = HttpRq->GetUrl()->GetRelUrlStr();
    UrlBase = HttpRq->GetUrl()->GetBaseUrlStr();
    ReqMethod = (char) HttpRq->GetMethod();
    HttpRq->GetBodyAsMem(Body);
}

THttpReqSerInfo::THttpReqSerInfo(TSIn& SIn) : UrlRel(SIn), UrlBase(SIn), ReqMethod(SIn), Body(SIn)
{
}

void THttpReqSerInfo::Save(TSOut& SOut)
{
    UrlRel.Save(SOut);
    UrlBase.Save(SOut);
    ReqMethod.Save(SOut);
    Body.Save(SOut);
}

PHttpRq THttpReqSerInfo::GetHttpRq()
{
    PUrl Url = TUrl::New(UrlRel, UrlBase);
    if ((THttpRqMethod) ReqMethod.Val == hrmGet)
        return THttpRq::New(Url);
    else
        return THttpRq::New((THttpRqMethod) ReqMethod.Val, Url, "", Body);
}

//////////////////////////////////////////////////////////////////////////
// Simple-App-Server
#include "favicon.cpp"

TSAppSrv::TSAppSrv(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify, 
        const bool& _ShowParamP, const bool& _ListFunP): TWebSrv(PortN, true, Notify), 
        Favicon(Favicon_bf, Favicon_len) {

    ShowParamP = _ShowParamP;
    ListFunP = _ListFunP;
    // initiaize hash-table with mappings
    for (int SrvFunN = 0; SrvFunN < SrvFunV.Len(); SrvFunN++) {
        PSAppSrvFun SrvFun =  SrvFunV[SrvFunN];
        FunNmToFunH.AddDat(SrvFun->GetFunNm(), SrvFun);
    }
}

void TSAppSrv::OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq) {
    // last appropriate error code, start with bad request
    int ErrStatusCd = THttp::BadRqStatusCd;
    try {
        // check http-request correctness - return if error
        EAssertR(HttpRq->IsOk(), "Bad HTTP request!");
        // check url correctness - return if error
        PUrl HttpRqUrl = HttpRq->GetUrl();
        EAssertR(HttpRqUrl->IsOk(), "Bad request URL!");
        // extract function name
        EAssertR(HttpRqUrl->GetPathSegs() > 0, "Bad request URL!");
        TStr FunNm = HttpRqUrl->GetPathSeg(0);
        // check if we have the function registered
        if (FunNm == "favicon.ico") {
            PHttpResp HttpResp = THttpResp::New(THttp::OkStatusCd,
                THttp::ImageIcoFldVal, false, Favicon.GetSIn());
            SendHttpResp(SockId, HttpResp); 
            return;
        } else if (!FunNm.Empty() && !FunNmToFunH.IsKey(FunNm)) { 
            ErrStatusCd = THttp::ErrNotFoundStatusCd;
            GetNotify()->OnStatusFmt("[AppSrv] Unknown function '%s'!", FunNm.CStr());
            TExcept::Throw("Unknown function '" + FunNm + "'!");
        }
        // extract parameters
        PUrlEnv HttpRqUrlEnv = HttpRq->GetUrlEnv();
        TStrKdV FldNmValPrV; HttpRqUrlEnv->GetKeyValPrV(FldNmValPrV);

        // report call
        if (ShowParamP) {  GetNotify()->OnStatus(HttpRq->GetUrl()->GetUrlStr()); }
        // request parsed well, from now on it's internal error
        ErrStatusCd = THttp::InternalErrStatusCd;
        // processed requested function
        if (!FunNm.Empty()) {
            // prepare request environment
            PSAppSrvRqEnv RqEnv = TSAppSrvRqEnv::New(this, SockId, HttpRq, FunNmToFunH);
            // retrieve function
            PSAppSrvFun SrvFun = FunNmToFunH.GetDat(FunNm);
            // call function
            SrvFun->Exec(FldNmValPrV, RqEnv);
        } else {
            // internal SAppSrv call
            if (!ListFunP) {
                // we are not allowed to list functions
                ErrStatusCd = THttp::ErrNotFoundStatusCd;
                TExcept::Throw("Unknown page");
            }
            // prepare a list of registered functions
            PJsonVal FunArrVal = TJsonVal::NewArr();
            int KeyId = FunNmToFunH.FFirstKeyId();
            while (FunNmToFunH.FNextKeyId(KeyId)) {
                FunArrVal->AddToArr(TJsonVal::NewObj("name", FunNmToFunH.GetKey(KeyId)));
            }
            PJsonVal ResVal = TJsonVal::NewObj();
            ResVal->AddToObj("port", GetPortN());
            ResVal->AddToObj("connections", GetConns());
            ResVal->AddToObj("functions", FunArrVal);
            TStr ResStr = ResVal->SaveStr();
            // prepare response
            PHttpResp HttpResp = THttpResp::New(THttp::OkStatusCd, 
                THttp::AppJSonFldVal, false, TMIn::New(ResStr));
            // send response
            SendHttpResp(SockId, HttpResp); 
        }
    } catch (PExcept Except) {
        // known internal error
        TNotify::StdNotify->OnNotifyFmt(ntErr, "Error: %s", Except->GetMsgStr().CStr());
        TNotify::StdNotify->OnNotifyFmt(ntErr, "Error location info: %s", Except->GetLocStr().CStr());

        PJsonVal ErrorVal = TJsonVal::NewObj();
        ErrorVal->AddToObj("message", Except->GetMsgStr());
        ErrorVal->AddToObj("location", Except->GetLocStr());
        PJsonVal ResVal = TJsonVal::NewObj("error", ErrorVal);
        TStr ResStr = ResVal->SaveStr();
        // prepare response
        PHttpResp HttpResp = THttpResp::New(ErrStatusCd,
            THttp::AppJSonFldVal, false, TMIn::New(ResStr));
        // send response
        SendHttpResp(SockId, HttpResp);
    } catch (...) {
        TNotify::StdNotify->OnNotify(ntErr, "Unknown internal error");
        // unknown internal error
        PJsonVal ResVal = TJsonVal::NewObj("error", "Unknown internal error");
        TStr ResStr = ResVal->SaveStr();
        // prepare response
        PHttpResp HttpResp = THttpResp::New(ErrStatusCd,
            THttp::AppJSonFldVal, false, TMIn::New(ResStr));
        // send response
        SendHttpResp(SockId, HttpResp);
    }
}

//////////////////////////////////////
// App-Server with loging and replaying of requests
TReplaySrv::TReplaySrv(const int& PortN, const TSAppSrvFunV& SrvFunV, const PNotify& Notify,
    const bool& _ShowParamP, const bool& _ListFunP) : TSAppSrv(PortN, SrvFunV, Notify, _ShowParamP, _ListFunP)
{
}

TReplaySrv::~TReplaySrv()
{
    StopLogging();
}

// open the log file and replay all the requests that we can find in it
bool TReplaySrv::ReplayLog(const TStr& LogFNm, const PNotify& ErrorNotify)
{
    if (TFile::Exists(LogFNm)) {
        PSIn SIn = TFIn::New(LogFNm);
        return ReplayStream(SIn, ErrorNotify);
    }
    return false;
}

bool TReplaySrv::ReplayStream(const PSIn& SIn, const PNotify& ErrorNotify)
{

    while (!SIn->Eof()) {
        try {
            THttpReqSerInfo ReqInfo(*SIn);
            PHttpRq HttpRq = ReqInfo.GetHttpRq();
            ReplayHttpRq(HttpRq);
        }
        catch (PExcept E) {
            ErrorNotify->OnNotifyFmt(ntErr, "TReplaySrv::ReplayStream. Exception while loading next request: %s", E->GetMsgStr().CStr());
        }
        catch (...) {
            ErrorNotify->OnNotifyFmt(ntErr, "TReplaySrv::ReplayStream. General exception while loading next request.");
        }
    }
    return true;
}

// open the log file. depending on the needs you can flush the log file after each request
// by setting Append = false you're effectively clearing the file first before starting to write in it
void TReplaySrv::StartLogging(const TStr& LogFNm, const bool& _FlushEachRequest, const bool& Append)
{
    SOut = TFOut::New(LogFNm, Append);
    FlushEachRequest = _FlushEachRequest;
}

// close the log file
void TReplaySrv::StopLogging()
{
    SOut = NULL;
}

bool TReplaySrv::RemoveLogData(const TStr& LogFNm)
{
    try {
        if (TFile::Exists(LogFNm))
            return TFile::Del(LogFNm, false);
    }
    catch (PExcept Except) {
        TNotify::StdNotify->OnNotifyFmt(ntErr, "TReplaySrv::RemoveLogData. Error while removing file: %s\n%s.", LogFNm.CStr(), Except->GetLocStr().CStr());
    }
    catch (...) {
        TNotify::StdNotify->OnNotifyFmt(ntErr, "TReplaySrv::RemoveLogData. Error while removing file: %s.", LogFNm.CStr());
    }
    return false;
}

// a http request is received. process it using the standard TSAppSrv method
// and then optionally also save the request to the log file
void TReplaySrv::OnHttpRq(const uint64& SockId, const PHttpRq& HttpRq)
{
    // if we are logging requests then log it
    if (!SOut.Empty()) {
        try {
            PUrl HttpRqUrl = HttpRq->GetUrl();
            TStr FunNm = HttpRqUrl->GetPathSeg(0);
            if (LoggingFunNmH.IsKey(FunNm)) {
                THttpReqSerInfo ReqInfo(HttpRq);
                ReqInfo.Save(*SOut);
                if (FlushEachRequest)
                    SOut->Flush();
            }
        }
        catch (PExcept Except) {
            TFileNotify Notify("ReplaySrvError.txt");
            Notify.OnNotifyFmt(ntErr, "TReplaySrv::OnHttpRq. Exception %s.", Except->GetMsgStr().CStr());
            Notify.OnNotifyFmt(ntErr, "TReplaySrv::OnHttpRq. Location info: \n%s.", Except->GetLocStr().CStr());
        }
        catch (...) {
            TFileNotify Notify("ReplaySrvError.txt");
            Notify.OnNotify(ntErr, "TReplaySrv::OnHttpRq. Unrecognized exception.");
        }
    }

    // only after we've logged the request we also execute it - otherwise we could process the request and crash before saving the data
    TSAppSrv::OnHttpRq(SockId, HttpRq);
}

// replay a particular http request
void TReplaySrv::ReplayHttpRq(const PHttpRq& HttpRq)
{
    PUrl HttpRqUrl = HttpRq->GetUrl();
    // extract function name
    TStr FunNm = HttpRqUrl->GetPathSeg(0);
    // extract parameters
    TStrKdV FldNmValPrV;
    PUrlEnv HttpRqUrlEnv = HttpRq->GetUrlEnv();
    const int Keys = HttpRqUrlEnv->GetKeys();
    for (int KeyN = 0; KeyN < Keys; KeyN++) {
        TStr KeyNm = HttpRqUrlEnv->GetKeyNm(KeyN);
        const int Vals = HttpRqUrlEnv->GetVals(KeyN);
        for (int ValN = 0; ValN < Vals; ValN++) {
            TStr Val = HttpRqUrlEnv->GetVal(KeyN, ValN);
            FldNmValPrV.Add(TStrKd(KeyNm, Val));
        }
    }
    // processed requested function
    if (!FunNm.Empty()) {
        // prepare request environment
        PSAppSrvRqEnv RqEnv = TSAppSrvRqEnv::New(this, TUInt64::Mx, HttpRq, FunNmToFunH);
        // retrieve function
        PSAppSrvFun SrvFun = FunNmToFunH.GetDat(FunNm);
        // call function
        SrvFun->Exec(FldNmValPrV, RqEnv);
    }
}

//////////////////////////////////////
// File-Download-Function
PSIn TSASFunFPath::ExecSIn(const TStrKdV& FldNmValPrV,
        const PSAppSrvRqEnv& RqEnv, TStr& ContTypeStr) {

    // construct file name
    TStr FNm = FPath;
    PUrl Url = RqEnv->GetHttpRq()->GetUrl();
    const int PathSegs = Url->GetPathSegs();
    if ((PathSegs == 1) || (PathSegs == 2 && Url->GetPathSeg(1).Empty())) {
        // nothing specified, do the default
        TStr PathSeg = Url->GetPathSeg(0);
        if (PathSeg.LastCh() != '/') { FNm += "/"; }
        FNm += DefaultFNm;
    } else {
        // extract file name
        for (int PathSegN = 1; PathSegN < PathSegs; PathSegN++) {
            FNm += "/"; FNm += Url->GetPathSeg(PathSegN);
        }
    }

    // get mime-type
    TStr FExt = FNm.GetFExt();
    if (FExt == ".htm") { ContTypeStr = THttp::TextHtmlFldVal; }
    else if (FExt == ".html") { ContTypeStr = THttp::TextHtmlFldVal; }
    else if (FExt == ".js") { ContTypeStr = THttp::TextJavaScriptFldVal; }
    else if (FExt == ".css") { ContTypeStr = THttp::TextCssFldVal; }
    else if (FExt == ".ico") { ContTypeStr = THttp::ImageIcoFldVal; }
    else if (FExt == ".png") { ContTypeStr = THttp::ImagePngFldVal; }
    else if (FExt == ".jpg") { ContTypeStr = THttp::ImageJpgFldVal; }
    else if (FExt == ".jpeg") { ContTypeStr = THttp::ImageJpgFldVal; }
    else if (FExt == ".gif") { ContTypeStr = THttp::ImageGifFldVal; }
    else {
        printf("Unknown MIME type for extension '%s' for file '%s'", FExt.CStr(), FNm.CStr());
        ContTypeStr = THttp::AppOctetFldVal; 
    }

    // return stream to the file
    return TFIn::New(FNm);
}

//////////////////////////////////////
// URL-Redirect-Function
TSASFunRedirect::TSASFunRedirect(const TStr& FunNm,
        const TStr& SettingFNm): TSAppSrvFun(FunNm, saotUndef) { 

    printf("Loading redirects %s\n", FunNm.CStr());
    TFIn FIn(SettingFNm); TStr LnStr, OrgFunNm;
    while (FIn.GetNextLn(LnStr)) {
        TStrV PartV;  LnStr.SplitOnAllCh('\t', PartV, false);
        if (PartV.Empty()) { continue; }
        if (PartV[0].Empty()) {
            // parameters
            EAssert(PartV.Len() >= 3);
            TStr FldNm = PartV[1];
            TStr FldVal = PartV[2];
            if (FldVal.StartsWith("$")) {
                MapH.GetDat(OrgFunNm).FldNmMapH.AddDat(FldVal.Right(1), FldNm);
            } else {
                MapH.GetDat(OrgFunNm).FldNmValPrV.Add(TStrKd(FldNm, FldVal));
            }
        } else {
            // new function
            EAssert(PartV.Len() >= 2);
            OrgFunNm = PartV[0];
            MapH.AddDat(OrgFunNm).FunNm = PartV[1];
            printf("  %s - %s\n", PartV[0].CStr(), PartV[1].CStr());
        }
    }
    printf("Done\n");
}

void TSASFunRedirect::Exec(const TStrKdV& FldNmValPrV, const PSAppSrvRqEnv& RqEnv) {
    PUrl Url = RqEnv->GetHttpRq()->GetUrl();
    TStr FunNm = (Url->GetPathSegs() > 1) ? Url->GetPathSeg(1) : "";
    EAssert(MapH.IsKey(FunNm));
    const TRedirect& Redirect =  MapH.GetDat(FunNm);
    TStrKdV _FldNmValPrV = Redirect.FldNmValPrV;
    for (int FldN = 0; FldN < FldNmValPrV.Len(); FldN++) {
        TStr OrgFldNm = FldNmValPrV[FldN].Key;
        if (Redirect.FldNmMapH.IsKey(OrgFldNm)) {
            _FldNmValPrV.Add(TStrKd(Redirect.FldNmMapH.GetDat(OrgFldNm), FldNmValPrV[FldN].Dat));
        }
    }
    RqEnv->ExecFun(Redirect.FunNm, _FldNmValPrV);
}

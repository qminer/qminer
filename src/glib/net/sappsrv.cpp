/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
        Notify->OnStatus(TStr::Fmt("[AppSrv] RequestStart %s", FunNm.CStr()));
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
		// log finis of the call
		Notify->OnStatus(TStr::Fmt("[AppSrv] RequestFinish %s [%d ms]", 
				FunNm.CStr(), StopWatch.GetMSecInt()));
		// prepare response
		HttpResp = THttpResp::New(THttp::OkStatusCd, 
			ContTypeVal, false, BodySIn);
    } catch (PExcept Except) {
		// known internal error
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
	// send response
	RqEnv->GetWebSrv()->SendHttpResp(RqEnv->GetSockId(), HttpResp); 
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
        PUrl RqUrl = HttpRq->GetUrl();
        EAssertR(RqUrl->IsOk(), "Bad request URL!");
        // extract function name
        PUrl HttpRqUrl = HttpRq->GetUrl();
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
		// report call
		if (ShowParamP) {  GetNotify()->OnStatus(" " + HttpRq->GetUrl()->GetUrlStr()); }
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

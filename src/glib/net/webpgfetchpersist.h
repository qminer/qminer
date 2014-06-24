#ifndef webpgfetchpersist_h
#define webpgfetchpersist_h

#include "../base/base.h"
#include "net.h"

// the persisten version of the web page fetch class
// if SaveFName is provided then the class will save in the provided file name
// the page requests that are still in the queue
// on creating the class instance the requests will be loaded from the file 
// and again requested

// if RepeatFailedRequests == true then the failed requests will be again requeued
// useful for calling web services that often timeout

ClassTPE(TWebPgFetchPersist, PWebPgFetchPersist, TWebPgFetch)//{
protected:
	TInt SuccessCount;
	TInt ErrorCount;
	TStr SaveFName;
	bool RepeatFailedRequests;
	bool ReportState;
	TStr ReportPrefix;
	PNotify Notify;

	TWebPgFetchPersist(const TStr& _SaveFName = "", const bool& _RepeatFailedRequests = true, const bool& _ReportState = false, const TStr& _ReportPrefix = "", const PNotify& _Notify = NULL) :
		SaveFName(_SaveFName), RepeatFailedRequests(_RepeatFailedRequests), ReportState(_ReportState), ReportPrefix(_ReportPrefix), Notify(_Notify)
	{
		PutMxConns(10);
		PutTimeOutMSecs(30 * 1000);
		if (SaveFName != "" && TFile::Exists(SaveFName)) {
			TFIn FIn(SaveFName);
			Load(FIn);
		}
	}

	void Load(TSIn& SIn)
	{
		// load PUrls and call FetchUrl on each of them
		TVec<PUrl> UrlV = TVec<PUrl>(SIn);
		int Len = UrlV.Len();
		for (int N = 0; N < Len; N++) {
			FetchUrl(UrlV[N]);
		}
		if (!Notify.Empty() && Len > 0)
			Notify->OnStatusFmt("TWebPgFetchPersist.Load. Loaded %d requests from the disk", Len);
	}

	void Save(TSOut& SOut) const
	{
		int Len = WaitFIdUrlPrL.Len();
		// serialize only PUrl, skip FIds
		TVec<PUrl> UrlV(Len);
		TLstNd<TIdUrlPr>* Item = WaitFIdUrlPrL.First();
		int N = 0;
		while (Item != NULL) {
			UrlV[N] = Item->Val.Val2;
			Item = Item->Next();
			N++;
		}
		UrlV.Save(SOut);
		if (!Notify.Empty() && UrlV.Len() > 0)
			Notify->OnStatusFmt("TWebPgFetchPersist.Save. Saving %d requests to the disk", UrlV.Len());
	}


public:
	~TWebPgFetchPersist() {
		if (SaveFName != "") {
			TFOut FOut(SaveFName);
			Save(FOut);
		}
	}

	static PWebPgFetchPersist New(const TStr& SaveFName = "", const bool& RepeatFailedRequests = true, const bool& ReportState = false, const TStr& ReportPrefix = "") {
		return new TWebPgFetchPersist(SaveFName, RepeatFailedRequests, ReportState, ReportPrefix);
	}
	
	virtual void ReportError(const TStr& MsgStr) {
		TNotify::StdNotify->OnStatusFmt("Error info: %s\n", MsgStr.CStr());
		/*if (!Notify.Empty())
			Notify->OnStatusFmt("TWebPgFetchPersist.ReportError: %s", MsgStr.CStr());*/
	}
	virtual void OnFetch(const int& FId, const PWebPg& WebPg) { 
		SuccessCount++; Report(); 
	}
	virtual void OnError(const int& FId, const TStr& MsgStr) {
		ErrorCount++; 
		ReportError(MsgStr);
		Report(); 
		// in case of bad request don't make the request again
		/*if (!HttpResp.Empty() && HttpResp->GetStatusCd() == 400) {
			TStr Url = "";
			if (IsConn(FId))
				Url = GetConnUrl(FId)->GetUrlStr();
			if (!Notify.Empty()) {
				TStr Error = "TWebPgFetchPersist.OnError: Received http response 400 (Bad request). Skipping the request. Request: " + Url;
				Notify->OnStatus(Error.CStr());
			}
			return;
		}
		else if (!HttpResp.Empty() && !Notify.Empty()) {
			Notify->OnStatusFmt("TWebPgFetchPersist.OnError: Received http response %d.", HttpResp->GetStatusCd());
		}*/

		if (IsConn(FId) && RepeatFailedRequests) {
			PUrl Url = GetConnUrl(FId);
			FetchUrl(Url, false);	// enqueue request at the beginning of the queue		
		}
	}
	virtual void Report() { 
		if (ReportState)
			printf("%squeue size: %6d. Completed: %6d. Failed: %6d\r", ReportPrefix.CStr(), WaitFIdUrlPrL.Len(), SuccessCount.Val, ErrorCount.Val);
	}
};

#endif
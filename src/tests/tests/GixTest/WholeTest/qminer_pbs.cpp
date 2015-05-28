
#include "qminer_pbs.h"

namespace TQm {

	using namespace TQm::TStorage;
	using namespace glib;

	/// Add new record
	uint64 TStorePbBlob::AddRec(const PJsonVal& RecVal) {
	}

	/// Update existing record
	void TStorePbBlob::UpdateRec(const uint64& RecId, const PJsonVal& RecVal) {
	}

	/// Save part of the data, given time-window
	int TStorePbBlob::PartialFlush(int WndInMsec = 500) {
		Data->PartialFlush(WndInMsec);
	}
	
	/// Retrieve performance statistics for this store
	PJsonVal TStorePbBlob::GetStats() {
		PJsonVal res = TJsonVal::NewObj();
		res->AddToObj("name", GetStoreNm());
		res->AddToObj("storage", Data->GetStats());
		return res;
	}
}
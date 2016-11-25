/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "streamstory.h"

using namespace TMc;

///////////////////////////////////////////////
// Feature information
TFtrInfo::TFtrInfo():
		Type(ftUndefined),
		Offset(-1),
		Length(0) {}

TFtrInfo::TFtrInfo(const TFtrType& _Type, const int& _Offset, const int& _Length):
		Type(_Type),
		Offset(_Offset),
		Length(_Length) {}

TFtrInfo::TFtrInfo(TSIn& SIn):
		Type(static_cast<TFtrType>(int(TInt(SIn)))),
		Offset(TInt(SIn)),
        Length(TInt(SIn)) {}

void TFtrInfo::Save(TSOut& SOut) const {
	TInt(Type).Save(SOut);
	TInt(Offset).Save(SOut);
	TInt(Length).Save(SOut);
}

TFtrInfo& TFtrInfo::operator =(const TFtrInfo& Info) {
	if (this != &Info) {
		Type = Info.Type;
		Offset = Info.Offset;
		Length = Info.Length;
	}
	return *this;
}

TStr TFtrInfo::GetTypeStr() const {
	switch (GetType()) {
	case ftUndefined:
		return "undefined";
	case ftNumeric:
		return "numeric";
	case ftCategorical:
		return "categorical";
	case ftTime:
	    return "time";
	default:
		throw TExcept::New("Unknown feature type: " + TInt::GetStr((int) GetType()));
	}
}

int TFtrInfo::GetCategoricalFtrVal(const TFltV& FtrV) const {
	for (int FtrN = Offset; FtrN < Offset + Length; FtrN++) {
		if (FtrV[FtrN] > 0) {
			return FtrN - Offset;
		}
	}

	TFltV SingleFtrV;
	for (int FtrN = Offset; FtrN < Offset + Length; FtrN++) {
		SingleFtrV.Add(FtrV[FtrN]);
	}

	printf("Feature:\n%s\n", TStrUtil::GetStr(SingleFtrV, ", ", "%.2f").CStr());

	throw TExcept::New("Could not find the value of a categorical feature!");
}

double TFtrInfo::GetNumericFtrVal(const TFltV& FtrV) const {
	EAssert(0 <= Offset && Offset < FtrV.Len());
	return FtrV[Offset];
}

double TFtrInfo::GetFtrVal(const TFltV& FtrV) const {
	if (IsNumeric()) {
		return GetNumericFtrVal(FtrV);
	} else if (IsCategorical()) {
		return (double) GetCategoricalFtrVal(FtrV);
	} else {
		throw TExcept::New("Feature is not numeric or nominal!");
	}
}

//////////////////////////////////////////////////
// Histogram class
THistogram::THistogram():
		Bins(),
		TotalCount(),
		CountV(),
		BinValV() {}

THistogram::THistogram(const int& NBins, const double& MnVal, const double& MxVal):
		Bins(NBins),
		TotalCount(),
		CountV(NBins),
		BinValV(NBins) {

	const double Span = MxVal - MnVal;
	const double BinSize = Span / NBins;

	double CurrVal = MnVal + BinSize/2;
	for (int BinN = 0; BinN < NBins; BinN++) {
		BinValV[BinN] = CurrVal;
		CurrVal += BinSize;
	}
}

THistogram::THistogram(TSIn& SIn):
		Bins(SIn),
		TotalCount(SIn),
		CountV(SIn),
		BinValV(SIn) {}

void THistogram::Save(TSOut& SOut) const {
	Bins.Save(SOut);
	TotalCount.Save(SOut);
	CountV.Save(SOut);
	BinValV.Save(SOut);
}

void THistogram::Update(const double& FtrVal) {
	const double HalfBinSize = GetBinSize()/2;
	const double Eps = 1e-12;

	const double FirstBinEdge = BinValV[0] + HalfBinSize;
	const double LastBinEdge = BinValV.Last() - HalfBinSize - Eps;

	if (FtrVal < FirstBinEdge) {
		CountV[0]++;
	} else if (LastBinEdge <= FtrVal) {
		CountV.Last()++;
	} else {
		bool Updated = false;
		for (int BinN = 1; BinN < Bins-1; BinN++) {
			const double LowerEdge = BinValV[BinN] - HalfBinSize - Eps;
			const double UpperEdge = BinValV[BinN] + HalfBinSize;
			if (LowerEdge <= FtrVal && FtrVal < UpperEdge) {
				Updated = true;
				CountV[BinN]++;
				break;
			}
		}
		EAssertR(Updated, "Histogram not updated! Value: " + TFlt::GetStr(FtrVal) + ", bin vals:\n" + TStrUtil::GetStr(BinValV, ", ", "%.3f"));
	}

	TotalCount++;
}

void THistogram::GenSamples(const TFltV& CountV, const int& NTrials, TFltV& SimBinV, TRnd& Rnd) {
	const double Sum = TLinAlg::SumVec(CountV);

	TFltV ThresholdV(CountV.Len());
	SimBinV.Gen(CountV.Len());

	// calculate the thresholds
	double TotalProb = 0;
	for (int BinN = 0; BinN < CountV.Len(); BinN++) {
		const double& BinCount = CountV[BinN];
		const double BinProb = BinCount / Sum;
		ThresholdV[BinN] = TotalProb + BinProb;
		TotalProb += BinProb;
	}
	// generate the samples
	for (int TrialN = 0; TrialN < NTrials; TrialN++) {
		const double Val = Rnd.GetUniDev();
		int BinN = 0;
		while (Val > ThresholdV[BinN]) {
			BinN++;
		}
		SimBinV[BinN]++;
	}
}



//////////////////////////////////////////////////
// State identifier
const int TStateIdentifier::MX_ITER = 10000;
const int TStateIdentifier::TIME_HIST_BINS = 10000;

TStateIdentifier::TStateIdentifier(const PDenseKMeans& _KMeans, const int _NHistBins,
			const double& _Sample, const bool _IncludeTmFtrV, const TRnd& _Rnd,
			const bool& _Verbose):
		Rnd(_Rnd),
		KMeans(_KMeans),
		CentroidDistStatV(),
		NHistBins(_NHistBins),
		ObsHistVV(),
		ControlHistVV(),
		IgnoredHistVV(),
		StateTimeHistV(),
		StateContrFtrValVV(),
		IncludeTmFtrV(_IncludeTmFtrV),
		TmUnit(0),
		Sample(_Sample),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(NHistBins >= 2, "Should have at least 2 bins for the histogram!");
}

TStateIdentifier::TStateIdentifier(TSIn& SIn):
	Rnd(SIn),
	KMeans(TAbsKMeans<TFltVV>::Load(SIn)),
	ControlCentroidVV(SIn),
	IgnoredCentroidVV(SIn),
	CentroidDistStatV(SIn),
	NHistBins(TInt(SIn)),
	ObsHistVV(SIn),
	ControlHistVV(SIn),
	IgnoredHistVV(SIn),
	StateTimeHistV(SIn),
	StateYearHistV(SIn),
	StateMonthHistV(SIn),
	StateWeekHistV(SIn),
	StateDayHistV(SIn),
	StateContrFtrValVV(SIn),
	IncludeTmFtrV(TBool(SIn)),
	TmUnit(TUInt64(SIn)),
	Sample(TFlt(SIn)),
	Verbose(TBool(SIn)) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

TStateIdentifier::~TStateIdentifier() {}

void TStateIdentifier::Save(TSOut& SOut) const {
	Rnd.Save(SOut);
	KMeans->Save(SOut);
	ControlCentroidVV.Save(SOut);
	IgnoredCentroidVV.Save(SOut);
	CentroidDistStatV.Save(SOut);
	TInt(NHistBins).Save(SOut);
	ObsHistVV.Save(SOut);
	ControlHistVV.Save(SOut);
	IgnoredHistVV.Save(SOut);
	StateTimeHistV.Save(SOut);
	StateYearHistV.Save(SOut);
	StateMonthHistV.Save(SOut);
	StateWeekHistV.Save(SOut);
	StateDayHistV.Save(SOut);
	StateContrFtrValVV.Save(SOut);
	TBool(IncludeTmFtrV).Save(SOut);
	TUInt64(TmUnit).Save(SOut);
	TFlt(Sample).Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TStateIdentifier::Init(const TStreamStory& StreamStory, const TUInt64V& TmV,
		const TFltVV& ObsFtrVV, const TFltVV& ControlFtrVV, const TFltVV& IgnoredFtrVV) {
	EAssertR(Sample >= 0, "Cannot sample a negative number of instances!");

	const int NInst = ObsFtrVV.GetCols();

	Notify->OnNotify(TNotifyType::ntInfo, "Clustering ...");

	TmUnit = StreamStory.GetTransitionModeler().GetTimeUnit();

	if (Sample == 1) {
		TFltVV ClustFtrVV;	GenClustFtrVV(TmV, ObsFtrVV, ClustFtrVV);
		KMeans->Apply(ClustFtrVV, false, MX_ITER, Notify);
	} else {
		const int NSamples = Sample < 1 ? (int)ceil(NInst*Sample) : TMath::Mn(NInst, int(Sample));

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Sampling %d instances...", NSamples);

		TIntV SampleV;	TLinAlgTransform::RangeV(NInst, SampleV);

		SampleV.Shuffle(Rnd);
		SampleV.Trunc(NSamples);

		TFltVV ObsSampleVV(ObsFtrVV.GetRows(), NSamples);
		TFltV FtrV;
		TUInt64V SampleTmV;
		for (int SampleN = 0; SampleN < NSamples; SampleN++) {
			const int& RecN = SampleV[SampleN];

			ObsFtrVV.GetCol(RecN, FtrV);
			ObsSampleVV.SetCol(SampleN, FtrV);
			SampleTmV.Add(TmV[RecN]);
		}

		TFltVV ClustFtrVV;	GenClustFtrVV(SampleTmV, ObsSampleVV, ClustFtrVV);
		KMeans->Apply(ClustFtrVV, false, MX_ITER, Notify);
	}

	TIntV AssignV;	Assign(TmV, ObsFtrVV, AssignV);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Checking correctness ...");
	TIntSet AssignedStateSet;
	for (int RecN = 0; RecN < AssignV.Len(); RecN++) {
		AssignedStateSet.AddKey(AssignV[RecN]);
	}
	if (AssignedStateSet.Len() != GetStates()) {
		Notify->OnNotify(ntInfo, "Some of the states are empty! Removing them ...");
		TIntV RemoveStateIdV;
		for (int StateId = 0; StateId < GetStates(); StateId++) {
			if (!AssignedStateSet.IsKey(StateId)) {
				Notify->OnNotifyFmt(ntInfo, "Removing state %d ...", StateId);
				RemoveStateIdV.Add(StateId);
			}
		}
		KMeans->RemoveCentroids(RemoveStateIdV);

		AssignV.Clr();
		Assign(TmV, ObsFtrVV, AssignV);
	}
	EAssert(AssignedStateSet.Len() == GetStates());

	InitStatistics(TmV, ObsFtrVV, AssignV);

	InitCentroidVV(AssignV, ControlFtrVV, ControlCentroidVV);
	InitCentroidVV(AssignV, IgnoredFtrVV, IgnoredCentroidVV);
	InitHistograms(StreamStory, ObsFtrVV, ControlFtrVV, IgnoredFtrVV, AssignV);
	InitTimeHistogramV(TmV, AssignV, TIME_HIST_BINS);
	ClearControlFtrVV(ControlFtrVV.GetRows());

	Notify->OnNotify(TNotifyType::ntInfo, "Done.");
}

void TStateIdentifier::InitHistograms(const TStreamStory& StreamStory, const TFltVV& ObsFtrVV,
		const TFltVV& ContrFtrVV, const TFltVV& IgnoredFtrVV, const TIntV& AssignV) {
	Notify->OnNotify(TNotifyType::ntInfo, "Computing histograms ...");

	const int NClusts = GetStates();

	InitHists(StreamStory, ObsFtrVV, ContrFtrVV, IgnoredFtrVV);

	UpdateHistVV(StreamStory.GetObsFtrInfoV(), ObsFtrVV, AssignV, NClusts, ObsHistVV);
	UpdateHistVV(StreamStory.GetContrFtrInfoV(), ContrFtrVV, AssignV, NClusts, ControlHistVV);
	UpdateHistVV(StreamStory.GetIgnFtrInfoV(), IgnoredFtrVV, AssignV, NClusts, IgnoredHistVV);
}

void TStateIdentifier::InitTimeHistogramV(const TUInt64V& TmV, const TIntV& AssignV,
		const int& Bins) {
	const int NStates = GetStates();
	StateTimeHistV.Clr();

	const uint64 StartTm = TmV[0];
	const uint64 EndTm = TmV.Last();

	for (int StateId = 0; StateId < NStates; StateId++) {
		StateTimeHistV.Add(THistogram(Bins, (double) StartTm, (double) EndTm));
		StateYearHistV.Add(THistogram(12, 1, 13));
		StateMonthHistV.Add(THistogram(31, 1, 32));
		StateWeekHistV.Add(THistogram(7, 0, 7));
		StateDayHistV.Add(THistogram(24, 0, 24));
	}

	for (int RecN = 0; RecN < AssignV.Len(); RecN++) {
		const uint64 RecTmMSecs = TmV[RecN];
		const int StateId = AssignV[RecN];

		StateTimeHistV[StateId].Update((double) RecTmMSecs);

		const TTm RecTm = TTm::GetTmFromMSecs(RecTmMSecs);
		const int Month = RecTm.GetMonth();
		const int Day = RecTm.GetDay();
		const int DayOfWeek = RecTm.GetDaysSinceMonday();
		const int Hour = RecTm.GetHour();

		StateYearHistV[StateId].Update(Month);
		StateMonthHistV[StateId].Update(Day);
		StateWeekHistV[StateId].Update(DayOfWeek);
		StateDayHistV[StateId].Update(Hour);
	}
}

int TStateIdentifier::Assign(const uint64& RecTm, const TFltV& FtrV) const {
	TFltV DistV;	GetCentroidDistV(RecTm, FtrV, DistV);
	return TLinAlgSearch::GetMinIdx(DistV);
}

void TStateIdentifier::Assign(const TUInt64V& RecTmV, const TFltVV& FtrVV, TIntV& AssignV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Assigning %d instances ...", FtrVV.GetCols());
	TFltVV TransFtrVV;	GenClustFtrVV(RecTmV, FtrVV, TransFtrVV);
	KMeans->Assign(TransFtrVV, AssignV);
}

void TStateIdentifier::GetCentroidDistV(const uint64& RecTm, const TFltV& FtrV, TFltV& DistV) const {
	TFltV TransFtrV;	GenClustFtrV(RecTm, FtrV, TransFtrV);
	KMeans->GetCentroidDistV(TransFtrV, DistV);
}

void TStateIdentifier::GetCentroidDistVV(const TUInt64V& RecTmV, const TFltVV& FtrVV, TFltVV& DistVV) const {
	TFltVV TransFtrVV;	GenClustFtrVV(RecTmV, FtrVV, TransFtrVV);
	TFltVV DistMat;	KMeans->GetDistVV(TransFtrVV, DistVV);
}

double TStateIdentifier::GetDist(const uint64& RecTm, const int& StateId, const TFltV& FtrV) const {
	TFltV TransFtrV;	GenClustFtrV(RecTm, FtrV, TransFtrV);
	return KMeans->GetDist(StateId, TransFtrV);
}

void TStateIdentifier::GetJoinedCentroid(const int& FtrSpaceN, const TIntV& StateIdV,
		TFltV& FtrV) const {
	TUInt64V StateSizeV(StateIdV.Len());
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		StateSizeV[StateN] = GetStateSize(StateIdV[StateN]);
	}

	switch (FtrSpaceN) {
	case 0: {
		TFltVV AllCentroidVV;	GetCentroidVV(AllCentroidVV);
		GetJoinedCentroid(StateIdV, AllCentroidVV, StateSizeV, FtrV);
		break;
	}
	case 1: {
		GetJoinedCentroid(StateIdV, ControlCentroidVV, StateSizeV, FtrV);
		break;
	}
	case 2: {
		GetJoinedCentroid(StateIdV, IgnoredCentroidVV, StateSizeV, FtrV);
		break;
	}
	default:
		throw TExcept::New("Invalid feature space number: " + TInt::GetStr(FtrSpaceN));
	}
}

void TStateIdentifier::GetAllCentroid(const int& StateId, TFltV& FtrV) const {
	TFltV TempV;
	GetObsCentroid(StateId, FtrV);
	GetControlCentroid(StateId, TempV);
	FtrV.AddV(TempV);
	GetIgnoredCentroid(StateId, TempV);
	FtrV.AddV(TempV);
}

double TStateIdentifier::GetMeanPtCentDist(const int& CentroidIdx) const {
	EAssertR(CentroidIdx < GetStates(), TStr::Fmt("TFullKMeans::GetMeanPtCentDist: Invalid centroid index: %d", CentroidIdx));
	return CentroidDistStatV[CentroidIdx].Val2 / double(CentroidDistStatV[CentroidIdx].Val1);
}

uint64 TStateIdentifier::GetStateSize(const int& StateId) const {
	return CentroidDistStatV[StateId].Val1;
}

void TStateIdentifier::GetHistogram(const TStreamStory& StreamStory, const int& FtrId,
		const TAggState& AggState, TFltV& BinValV, TFltV& BinV, const bool& NormalizeP) const {
	EAssertR(0 <= FtrId && FtrId < StreamStory.GetAllDim(), "Invalid feature ID: " + TInt::GetStr(FtrId));

	if (StreamStory.IsObsFtrId(FtrId)) {
		const int FtrN = FtrId;
		GetHistogram(ObsHistVV, FtrId, StreamStory.GetObsFtrInfoV()[FtrN], AggState, BinValV, BinV, NormalizeP);
	} else if (StreamStory.IsContrFtrId(FtrId)) {
		const int FtrN = FtrId - StreamStory.GetObsDim();
		GetHistogram(ControlHistVV, FtrN, StreamStory.GetContrFtrInfoV()[FtrN], AggState, BinValV, BinV, NormalizeP);
	} else if (StreamStory.IsIgnFtrId(FtrId)) {
		const int FtrN = FtrId - StreamStory.GetObsDim() - StreamStory.GetContrDim();
		GetHistogram(IgnoredHistVV, FtrN, StreamStory.GetIgnFtrInfoV()[FtrN], AggState, BinValV, BinV, NormalizeP);
	} else {
		throw TExcept::New("WTF!? feature " + TInt::GetStr(FtrId) + " is not in any of the feature spaces!");
	}
}

void TStateIdentifier::GetGlobalTimeHistogram(const TAggState& AggState, TUInt64V& TmV, TFltV& BinV,
		const int NBins, const bool& NormalizeP) const {

	if (BinV.Len() != NBins) { BinV.Gen(NBins); }
	if (TmV.Len() != NBins) { TmV.Gen(NBins); }

	TFltV CountV, ValV;
	bool TmVInitP = false;
	for (int StateN = 0; StateN < AggState.Len(); StateN++) {
		const int& StateId = AggState[StateN];
		const THistogram& Hist = StateTimeHistV[StateId];
		const TIntV& OrigCountV = Hist.GetCountV();
		const TFltV& OrigValV = Hist.GetBinValV();

		ResampleHist(NBins, OrigValV, OrigCountV, ValV, CountV);

		for (int BinN = 0; BinN < NBins; BinN++) {
			BinV[BinN] += CountV[BinN];
		}

		if (!TmVInitP) {
			for (int BinN = 0; BinN < NBins; BinN++) {
				TmV[BinN] = (uint64) ValV[BinN];
			}
			TmVInitP = true;
		}
	}

	if (NormalizeP) {
		TLinAlg::NormalizeL1(BinV);
	}
}

void TStateIdentifier::GetTimeHistogram(const TAggState& AggState, const TTmHistType& HistType,
		TIntV& BinValV, TFltV& BinV) const {

	for (int StateN = 0; StateN < AggState.Len(); StateN++) {
		const int& StateId = AggState[StateN];

		const THistogram* Hist;
		switch (HistType) {
		case thtYear:
			Hist = &StateYearHistV[StateId];
			break;
		case thtMonth:
			Hist = &StateMonthHistV[StateId];
			break;
		case thtWeek:
			Hist = &StateWeekHistV[StateId];
			break;
		case thtDay:
			Hist = &StateDayHistV[StateId];
			break;
		default:
			throw TExcept::New("Unknown histogram type: " + TInt::GetStr(HistType));
		}

		if (BinV.Len() != Hist->GetBins()) { BinV.Gen(Hist->GetBins()); }

		const TIntV& CountV = Hist->GetCountV();
		const TFltV& ValV = Hist->GetBinValV();

		if (BinV.Empty()) { BinV.Gen(CountV.Len()); }
		if (BinValV.Empty()) {
			BinValV.Gen(Hist->GetBins());
			for (int BinN = 0; BinN < ValV.Len(); BinN++) {
				BinValV[BinN] = (int) ValV[BinN];
			}
		}

		for (int BinN = 0; BinN < CountV.Len(); BinN++) {
			BinV[BinN] += (double) CountV[BinN];
		}
	}
}

void TStateIdentifier::GetCentroidVV(TFltVV& CentroidVV) const {
	const TFltVV& OrigCentroidVV = GetRawCentroidVV();

	if (IncludeTmFtrV) {
		TLinAlg::SubMat(
			OrigCentroidVV,
			GetTmFtrDim(TmUnit),		// start row
			OrigCentroidVV.GetRows(),	// end row
			0,							// start col
			OrigCentroidVV.GetCols(),	// end col
			CentroidVV
		);
	}
	else {
		CentroidVV = OrigCentroidVV;
	}
}

void TStateIdentifier::GetControlCentroidVV(const TStreamStory& StreamStory, TStateFtrVV& StateFtrVV) const {
	const int Cols = ControlCentroidVV.GetCols();
	const int Rows = ControlCentroidVV.GetRows();

	const TFtrInfoV& FtrInfoV = StreamStory.GetContrFtrInfoV();

	StateFtrVV.Gen(Cols);

	for (int StateId = 0; StateId < Cols; StateId++) {
		StateFtrVV[StateId].Gen(Rows);

		for (int FtrN = 0; FtrN < FtrInfoV.Len(); FtrN++) {
			const TFtrInfo& FtrInfo = FtrInfoV[FtrN];

			if (IsControlFtrSet(StateId, FtrInfo)) {
				TFltV FtrV;	GetControlFtrV(StateId, FtrInfo, FtrV);

				const int Start = FtrInfo.GetOffset();
				const int End = Start + FtrInfo.GetLength();

				for (int i = Start; i < End; i++) {
					StateFtrVV[StateId][i] = FtrV[i - Start];
				}
			} else {
				const int Start = FtrInfo.GetOffset();
				const int End = Start + FtrInfo.GetLength();
				for (int i = Start; i < End; i++) {
					StateFtrVV[StateId][i] = ControlCentroidVV(i, StateId);
				}
			}

		}
	}
}

double TStateIdentifier::GetControlFtr(const int& StateId, const TFtrInfo& FtrInfo, const double& DefaultVal) const {
	const bool IsSet = IsControlFtrSet(StateId, FtrInfo);
	return IsSet ? GetControlFtr(StateId, FtrInfo) : DefaultVal;
}

void TStateIdentifier::SetControlFtr(const int& StateId, const TFtrInfo& FtrInfo,
		const double& Val) {
	EAssertR(StateId < StateContrFtrValVV.Len(), "Invalid state ID when setting feature!");

	const int& FtrOffset = FtrInfo.GetOffset();
	const int& FtrLen = FtrInfo.GetLength();

	EAssertR(0 <= FtrOffset && FtrOffset + FtrLen < StateContrFtrValVV[StateId].Len(), "Invalid feature ID when setting feature!");

	if (FtrInfo.IsNumeric()) {
		StateContrFtrValVV[StateId][FtrOffset] = Val;
	} else if (FtrInfo.IsCategorical()) {
		if (Val == TFlt::PInf) {	// clear the value
			for (int FtrN = FtrOffset; FtrN < FtrOffset + FtrLen; FtrN++) {
				StateContrFtrValVV[StateId][FtrN] = TFlt::PInf;
			}
		} else {
			EAssert(0 <= Val && Val <= FtrInfo.GetRange());

			for (int FtrN = FtrOffset; FtrN < FtrOffset + FtrLen; FtrN++) {
				StateContrFtrValVV[StateId][FtrN] = 0;
			}

			StateContrFtrValVV[StateId][(int) Val] = 1;
		}
	} else {
		throw TExcept::New("WTF!? feature is not numeric or nominal!");
	}
}

void TStateIdentifier::ClearControlFtr(const int& StateId, const TFtrInfo& FtrInfo) {
	SetControlFtr(StateId, FtrInfo, TFlt::PInf);
}

void TStateIdentifier::ClearControlFtrVV() {
	ClearControlFtrVV(StateContrFtrValVV[0].Len());
}

bool TStateIdentifier::IsControlFtrSet(const int& StateId, const TFtrInfo& FtrInfo) const {
	const int& Offset = FtrInfo.GetOffset();
	const int& Length = FtrInfo.GetLength();

	const TFltV& FtrV = StateContrFtrValVV[StateId];

	for (int FtrN = Offset; FtrN < Offset + Length; FtrN++) {
		if (FtrV[FtrN] == TFlt::PInf) { return false; }
	}

	return true;
}

bool TStateIdentifier::IsAnyControlFtrSet(const TFtrInfoV& FtrInfoV) const {
	const int States = GetStates();
	const int Dim = FtrInfoV.Len();

	for (int StateId = 0; StateId < States; StateId++) {
		for (int FtrId = 0; FtrId < Dim; FtrId++) {
			if (IsControlFtrSet(StateId, FtrInfoV[FtrId])) {
				return true;
			}
		}
	}

	return false;
}

void TStateIdentifier::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

void TStateIdentifier::InitStatistics(const TUInt64V& RecTmV, const TFltVV& FtrVV,
		const TIntV& AssignV) {
	const int K = GetStates();

	TFltVV DistVV;	GetCentroidDistVV(RecTmV, FtrVV, DistVV);

	CentroidDistStatV.Gen(K,K);

	TUInt64V ClustSizeV(K);
	TFltV ClustDistSumV(K);

	for (int RecN = 0; RecN < AssignV.Len(); RecN++) {
		const int& ClustN = AssignV[RecN];

		ClustSizeV[ClustN]++;
		ClustDistSumV[ClustN] += DistVV(ClustN, RecN);
	}

	for (int ClustN = 0; ClustN < K; ClustN++) {
		CentroidDistStatV[ClustN].Val1 = ClustSizeV[ClustN];
		CentroidDistStatV[ClustN].Val2 = ClustDistSumV[ClustN];
	}
}

void TStateIdentifier::InitCentroidVV(const TIntV& AssignV, const TFltVV& FtrVV,
		TFltVV& CentroidVV) {
	const int NCentroids = GetStates();
	const int Dim = FtrVV.GetRows();
	const int NInst = FtrVV.GetCols();

	CentroidVV.Gen(Dim, NCentroids);
	TUInt64V CountV(NCentroids);

	for (int i = 0; i < NInst; i++) {
		CountV[AssignV[i]]++;
	}

	for (int ColN = 0; ColN < NInst; ColN++) {
		const int CentId = AssignV[ColN];
		for (int RowN = 0; RowN < Dim; RowN++) {
			CentroidVV(RowN, CentId) += FtrVV(RowN, ColN) / double(CountV[CentId]);
		}
	}
}

void TStateIdentifier::GetObsCentroid(const int& StateId, TFltV& FtrV) const {
	EAssert(0 <= StateId && StateId < GetStates());
	TFltVV CentroidVV;	GetCentroidVV(CentroidVV);
	CentroidVV.GetCol(StateId, FtrV);
}

void TStateIdentifier::GetControlCentroid(const int& StateId, TFltV& FtrV) const {
	EAssert(0 <= StateId && StateId < ControlCentroidVV.GetCols());
	ControlCentroidVV.GetCol(StateId, FtrV);
}

void TStateIdentifier::GetIgnoredCentroid(const int& StateId, TFltV& FtrV) const {
	EAssert(0 <= StateId && StateId < IgnoredCentroidVV.GetCols());
	IgnoredCentroidVV.GetCol(StateId, FtrV);
}

double TStateIdentifier::GetControlFtr(const int& StateId, const TFtrInfo& FtrInfo) const {
	EAssertR(StateId < StateContrFtrValVV.Len(), "Invalid state ID when fetching feature value!");

	return FtrInfo.GetFtrVal(StateContrFtrValVV[StateId]);
}

void TStateIdentifier::GetControlFtrV(const int& StateId, const TFtrInfo& FtrInfo, TFltV& FtrV) const {
	EAssert(IsControlFtrSet(StateId, FtrInfo));
	EAssertR(StateId < StateContrFtrValVV.Len(), "Invalid state ID when fetching feature value!");
	const int& FtrOffset = FtrInfo.GetOffset();
	const int& FtrLength = FtrInfo.GetLength();

	if (FtrV.Len() != FtrLength) { FtrV.Gen(FtrLength); }

	for (int i = FtrOffset; i < FtrOffset + FtrLength; i++) {
		FtrV[i - FtrOffset] = StateContrFtrValVV[StateId][i];
	}
}

void TStateIdentifier::ClearControlFtrVV(const int& Dim) {
	const int NStates = GetStates();

	StateContrFtrValVV.Gen(NStates);
	for (int StateId = 0; StateId < NStates; StateId++) {
		StateContrFtrValVV[StateId].Gen(Dim);
		for (int FtrId = 0; FtrId < Dim; FtrId++) {
			StateContrFtrValVV[StateId][FtrId] = TFlt::PInf;
		}
	}
}

void TStateIdentifier::GetHistogram(const TStateFtrHistVV& StateHistVV, const int& FtrN,
		const TFtrInfo& FtrInfo, const TIntV& AggState, TFltV& BinStartV, TFltV& BinV,
		const bool& NormalizeP) const {

	if (FtrInfo.IsNumeric()) {
		BinV.Gen(NHistBins);
	}
	else if (FtrInfo.IsCategorical()) {
		BinV.Gen(FtrInfo.GetLength());
	}
	else {
		throw TExcept::New("Histograms of non-numeric and non-nominal features unsupported!");
	}

	BinStartV = StateHistVV[0][FtrN].GetBinValV();

	for (int StateN = 0; StateN < AggState.Len(); StateN++) {
		const int StateId = AggState[StateN];
		const THistogram& Hist = StateHistVV[StateId][FtrN];
		const TIntV& CountV = Hist.GetCountV();

		for (int BinN = 0; BinN < CountV.Len(); BinN++) {
			BinV[BinN] += (double) CountV[BinN];
		}
	}

	if (NormalizeP) {
		TLinAlg::NormalizeL1(BinV);
	}
}

void TStateIdentifier::InitHistVV(const TFtrInfoV& FtrInfoV, const int& NInst,
		const TFltVV& FtrVV, TStateFtrHistVV& HistVV) {
	const int States = GetStates();
	const int Dim = FtrInfoV.Len();
	const int Bins = NHistBins;

	HistVV.Gen(States);

	for (int State1Id = 0; State1Id < States; State1Id++) {
		HistVV[State1Id].Gen(Dim);
	}

	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		const TFtrInfo& FtrInfo = FtrInfoV[FtrN];

		switch (FtrInfo.GetType()) {
		case ftUndefined: {
			throw TExcept::New("Cannot initialize a histogram for undefined feature type!");
		}
		case ftNumeric: {
			// find min and max value
			double MnVal = TFlt::Mx;
			double MxVal = TFlt::Mn;

			for (int InstN = 0; InstN < NInst; InstN++) {
				if (FtrVV(FtrN, InstN) < MnVal) { MnVal = FtrVV(FtrN, InstN); }
				if (FtrVV(FtrN, InstN) > MxVal) { MxVal = FtrVV(FtrN, InstN); }
			}

			// go through all the histograms and initialize them
			for (int State1Id = 0; State1Id < States; State1Id++) {
				HistVV[State1Id].SetVal(FtrN, THistogram(Bins, MnVal, MxVal));
			}
			break;
		}
		case ftCategorical: {
			for (int StateId = 0; StateId < States; StateId++) {
				HistVV[StateId].SetVal(FtrN, THistogram(FtrInfo.GetLength(), 0, FtrInfo.GetLength()));
			}
			break;
		}
		case ftTime: {
		    throw TExcept::New("Histograms not supported for time features!");
		}
		default: {
			throw TExcept::New("Unknown feature type when initializing histograms: " + TInt::GetStr(FtrInfo.GetType()));
		}
		}
	}
}

void TStateIdentifier::InitHists(const TStreamStory& StreamStory, const TFltVV& ObsFtrVV,
		const TFltVV& ContrFtrVV, const TFltVV& IgnoredFtrVV) {

	const int NInst = ObsFtrVV.GetCols();

	InitHistVV(StreamStory.GetObsFtrInfoV(), NInst, ObsFtrVV, ObsHistVV);
	InitHistVV(StreamStory.GetContrFtrInfoV(), NInst, ContrFtrVV, ControlHistVV);
	InitHistVV(StreamStory.GetIgnFtrInfoV(), NInst, IgnoredFtrVV, IgnoredHistVV);
}

void TStateIdentifier::GenClustFtrVV(const TUInt64V& TmV, const TFltVV& ObsFtrVV, TFltVV& FtrVV) const {
	EAssert(TmV.Len() > 1);
	EAssert(TmV.Len() == ObsFtrVV.GetCols());

	if (IncludeTmFtrV) {
		const int NInst = TmV.Len();
		const int ObsFtrVDim = ObsFtrVV.GetRows();
		const int TmFtrDim = GetTmFtrDim(TmUnit);

		FtrVV.Gen(TmFtrDim + ObsFtrVDim, NInst);

		TFltV FtrV;
		for (int RecN = 0; RecN < TmV.Len(); RecN++) {
			const uint64& RecTm = TmV[RecN];

			GenTmFtrV(RecTm, FtrV);
			for (int FtrN = 0; FtrN < TmFtrDim; FtrN++) {
				FtrVV(FtrN, RecN) = FtrV[FtrN];
			}
			for (int FtrN = 0; FtrN < ObsFtrVDim; FtrN++) {
				FtrVV(TmFtrDim + FtrN, RecN) = ObsFtrVV(FtrN, RecN);
			}
		}
	}
	else {
		FtrVV = ObsFtrVV;
	}
}

void TStateIdentifier::GenClustFtrV(const uint64& RecTm, const TFltV& FtrV, TFltV& ClustFtrV) const {
	if (IncludeTmFtrV) {
		const int ObsFtrVDim = FtrV.Len();
		const int TmFtrDim = GetTmFtrDim(TmUnit);

		ClustFtrV.Gen(TmFtrDim + ObsFtrVDim);

		TFltV TmFtrV;	GenTmFtrV(RecTm, TmFtrV);
		for (int FtrN = 0; FtrN < TmFtrDim; FtrN++) {
			ClustFtrV[FtrN] = TmFtrV[FtrN];
		}
		for (int FtrN = 0; FtrN < ObsFtrVDim; FtrN++) {
			ClustFtrV[TmFtrDim + FtrN] = FtrV[FtrN];
		}
	}
	else {
		ClustFtrV = FtrV;
	}
}

void TStateIdentifier::GenTmFtrV(const uint64& RecTm, TFltV& FtrV) const {
	const int TmFtrDim = GetTmFtrDim(TmUnit);

	FtrV.Gen(TmFtrDim);

	if (TmUnit == TCtmcModeller::TU_SECOND) {
		const TTm Tm = TTm::GetTmFromMSecs(RecTm);
		const int Second = Tm.GetSec();
		const int FtrVal = Second / 10;
		FtrV[FtrVal] = 1;
	}
	else if (TmUnit == TCtmcModeller::TU_MINUTE) {
		const TTm Tm = TTm::GetTmFromMSecs(RecTm);
		const int Minute = Tm.GetMin();
		const int FtrVal = Minute / 10;
		FtrV[FtrVal] = 1;
	}
	else if (TmUnit == TCtmcModeller::TU_HOUR) {
		const TTm Tm = TTm::GetTmFromMSecs(RecTm);
		const int Hour = Tm.GetHour();
		FtrV[Hour] = 1;
	}
	else if (TmUnit == TCtmcModeller::TU_DAY) {
		const TTm Tm = TTm::GetTmFromMSecs(RecTm);
		const int DayN = Tm.GetDaysSinceMonday();
		FtrV[DayN] = 1;
	}
	else if (TmUnit == TCtmcModeller::TU_MONTH) {
		const TTm Tm = TTm::GetTmFromMSecs(RecTm);
		const int MonthN = Tm.GetMonth() - 1;
		FtrV[MonthN] = 1;
	}
	else {
		throw TExcept::New("Invalid time unit: " + TUInt64::GetStr(TmUnit));
	}
}

void TStateIdentifier::UpdateHistVV(const TFtrInfoV& FtrInfoV, const TFltVV& FtrVV,
		const TIntV& AssignV, const int& States, TStateFtrHistVV& StateFtrHistVV) {

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrInfoV.Len();

	// update the histograms
	for (int InstN = 0; InstN < NInst; InstN++) {
		TFltV FtrV;	FtrVV.GetCol(InstN, FtrV);
		const int StateId = AssignV[InstN];

		TFtrHistV& FtrHistV = StateFtrHistVV[StateId];

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			const TFtrInfo& FtrInfo = FtrInfoV[FtrN];

			switch (FtrInfo.GetType()) {
			case ftUndefined: {
				throw TExcept::New("Cannot update a histogram for undefined feature type!");
			}
			case ftNumeric: {
				const int& FtrOffset = FtrInfo.GetOffset();
				FtrHistV[FtrN].Update(FtrVV(FtrOffset, InstN));
				break;
			}
			case ftCategorical: {
				TFltV FtrV;	FtrVV.GetCol(InstN, FtrV);
				const int FtrVal = FtrInfo.GetCategoricalFtrVal(FtrV);
				FtrHistV[FtrN].Update((double) FtrVal);
				break;
			}
			case ftTime: {
			    throw TExcept::New("Cannot initialize histogram of time feature!");
			}
			default: {
				throw TExcept::New("Unknown feature type when initializing histograms: " + TInt::GetStr(FtrInfo.GetType()));
			}
			}
		}
	}
}

void TStateIdentifier::GetJoinedCentroid(const TIntV& StateIdV,
		const TFltVV& CentroidVV, const TUInt64V& StateSizeV, TFltV& FtrV) {
	const int Dim = CentroidVV.GetRows();

	if (FtrV.Empty()) { FtrV.Gen(Dim); }
	EAssert(FtrV.Len() == Dim);

	TFltV CentroidV;
	double TotalSize = 0;
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		const int StateId = StateIdV[StateN];
		const double StateSize = (double) StateSizeV[StateN];

		CentroidVV.GetCol(StateId, CentroidV);

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			FtrV[FtrN] += CentroidV[FtrN] * StateSize;
		}

		TotalSize += StateSize;
	}

	for (int FtrN = 0; FtrN < Dim; FtrN++) {
		FtrV[FtrN] /= TotalSize;
	}
}

void TStateIdentifier::ResampleHist(const int& Bins, const TFltV& OrigBinValV,
		const TIntV& OrigBinV, TFltV& BinValV, TFltV& BinV) {
	EAssert(Bins > 1);
	EAssert(Bins < OrigBinV.Len());

	if (BinValV.Len() != Bins) { BinValV.Gen(Bins); }
	if (BinV.Len() != Bins) { BinV.Gen(Bins); }

	const double OrigBinsPerBin = double(OrigBinV.Len()) / Bins;

	int OrigBinN = 0;
	double OrigCountedPerc = 0;
	for (int BinN = 0; BinN < Bins; BinN++) {
		double BinsCounted = 0;
		while (BinsCounted + 1 <= OrigBinsPerBin) {
			BinV[BinN] += OrigBinV[OrigBinN]*(1 - OrigCountedPerc);
			BinValV[BinN] += OrigBinValV[OrigBinN]*(1 - OrigCountedPerc);
			OrigBinN++;
			OrigCountedPerc = 0;
			BinsCounted += 1;
		}

		if (BinsCounted < OrigBinsPerBin) {
			OrigCountedPerc = (OrigBinsPerBin - BinsCounted);
			BinV[BinN] += OrigCountedPerc * OrigBinV[OrigBinN];
			BinValV[BinN] += OrigCountedPerc * OrigBinValV[OrigBinN];
		}

		BinValV[BinN] /= OrigBinsPerBin;
	}
}

int TStateIdentifier::GetTmFtrDim(const uint64& TmUnit) {
	if (TmUnit == TCtmcModeller::TU_SECOND) {
		return 6;
	}
	else if (TmUnit == TCtmcModeller::TU_MINUTE) {
		return 6;
	}
	else if (TmUnit == TCtmcModeller::TU_HOUR) {
		return 24;
	}
	else if (TmUnit == TCtmcModeller::TU_DAY) {
		return 7;
	}
	else if (TmUnit == TCtmcModeller::TU_MONTH) {
		return 12;
	}
	else {
		throw TExcept::New("Invalid time unit: " + TUInt64::GetStr(TmUnit));
	}
}

/////////////////////////////////////////////
// MDS
void TEuclMds::Project(const TFltVV& FtrVV, TFltVV& ProjVV, const int& d) {
	// first center the rows of matrix X
	TFullMatrix X1(FtrVV);	TLinAlgTransform::CenterRows(X1.GetMat());

	// Let B = X'X, then we can decompose B into its spectral decomposition
	// B = V*L*V' where L is a diagonal matrix of eigenvalues, and A holds the
	// corresponding eigenvectors in its columns

	// we can now aaproximate B with just the highest 'd' eigenvalues and
	// eigenvectors: B_d = V_d * L_d * V_d'
	// the coordinates of X can then be recovered by: X_d = V_d*L_d^(.5)

	// we can use SVD do find the eigenvectors V_d and eigenvalues L_d
	// X = U*S*V', where:
	// S is a diagonal matrix where {s_i^2} are the eigenvalues of X'X=B and X*X'
	// U holds the eigenvectors of X*X'
	// V holds the eigenvectors of X'X

	// so X_d = V_d * diag({|s_i|})
	TMatVecMatTr Svd = X1.Svd(d);

	const TVector& EigValsSqrt = Svd.Val2.Map([&](const TFlt& Val) { return fabs(Val); });
	const TFullMatrix& V = Svd.Val3;

	TFullMatrix X_d = V*TFullMatrix::Diag(EigValsSqrt);

	X_d.GetT(ProjVV);
}

/////////////////////////////////////////////////////////////////
// TBernoulliIntens
const double TBernoulliIntens::MIN_PROB = 1e-5;

TBernoulliIntens::TBernoulliIntens():
		NStates(0),
		LogRegVV(),
		HasJumpedVV(),
		EmptyVV(),
		DeltaTm(0) {}

TBernoulliIntens::TBernoulliIntens(const int& _NStates, const double& _DeltaTm,
		const double& RegFact, const TBoolVV& _HasJumpedVV, const bool Verbose):
		NStates(_NStates),
		LogRegVV(_NStates, _NStates),
		HasJumpedVV(_HasJumpedVV),
		EmptyVV(_NStates, _NStates),
		DeltaTm(_DeltaTm) {

	for (int RowN = 0; RowN < NStates; RowN++) {
		for (int ColN = 0; ColN < NStates; ColN++) {
			LogRegVV.PutXY(RowN, ColN, TLogReg(RegFact, true, Verbose));
			EmptyVV.PutXY(RowN, ColN, true);
		}
	}
}

TBernoulliIntens::TBernoulliIntens(TSIn& SIn):
		NStates(TInt(SIn)),
		LogRegVV(SIn),
		HasJumpedVV(SIn),
		EmptyVV(SIn),
		DeltaTm(TFlt(SIn)) {}

void TBernoulliIntens::Save(TSOut& SOut) const {
	TInt(NStates).Save(SOut);
	LogRegVV.Save(SOut);
	HasJumpedVV.Save(SOut);
	EmptyVV.Save(SOut);
	TFlt(DeltaTm).Save(SOut);
}

void TBernoulliIntens::Fit(const int& RowN, const int& ColN, const TFltVV& X, const TFltV& y,
		const double& Eps) {
	LogRegVV(RowN, ColN).Fit(X, y, Eps);
	EmptyVV.PutXY(RowN, ColN, false);
}

void TBernoulliIntens::GetQMat(const TStateFtrVV& StateFtrVV, TFltVV& QMat) const {
	if (QMat.Empty()) { QMat.Gen(NStates, NStates); }
	EAssert(QMat.GetRows() == NStates && QMat.GetCols() == NStates);

	TFltV IntensV;
	for (int State1Id = 0; State1Id < NStates; State1Id++) {
		const TFltV& State1FtrV = StateFtrVV[State1Id];
		GetQMatRow(State1Id, State1FtrV, IntensV);
		QMat.SetRow(State1Id, IntensV);
	}
}

void TBernoulliIntens::GetQMatRow(const int& RowN, const TFltV& FtrV, TFltV& IntensV) const {
	if (IntensV.Empty()) { IntensV.Gen(NStates); }

	Assert(IntensV.Len() == NStates);
	Assert(DeltaTm > 0);

	double Prob;
	for (int ColN = 0; ColN < NStates; ColN++) {
		Prob = !EmptyVV(RowN, ColN) ? LogRegVV(RowN, ColN).Predict(FtrV) : 0;
		if (HasJumpedVV(RowN, ColN) && Prob < MIN_PROB) { Prob = MIN_PROB; }
		IntensV[ColN] = Prob;
	}

	TLinAlg::NormalizeL1(IntensV);
	for (int ColN = 0; ColN < NStates; ColN++) {
		IntensV[ColN] /= DeltaTm;
		EAssertR(IntensV[ColN] >= 0, "Intensity is less than 0!!!");
		EAssert(!TFlt::IsNan(IntensV[ColN]));
		if (IntensV[ColN] > 10000) { IntensV[ColN] = 10000; }	// TODO fix
	}

	// set q_ii
	IntensV[RowN] = 0;
	const double Qii = -TLinAlg::SumVec(IntensV);
	IntensV[RowN] = Qii;
	EAssertR(Qii < 0, TStr::Fmt("Invalid row %d of the Q matrix: %s", RowN, TStrUtil::GetStr(IntensV, ", ", "%.5f").CStr()));
}

/////////////////////////////////////////////////////////////////
// Discrete time Markov chain
void TDtMChain::GetProbVV(const TFltVV& PMat, const int& Steps, TFltVV& ProbVV) {
	EAssert(Steps >= 0);
	TLinAlg::Pow(PMat, Steps, ProbVV);
}

void TDtMChain::GetRevPMat(const TFltVV& PMat, TFltVV& RevPMat) {
	const int States = PMat.GetRows();

	if (RevPMat.Empty()) { RevPMat.Gen(States, States); }
	EAssert(RevPMat.GetRows() == States && RevPMat.GetCols() == States);

	TFltV StatDist;	GetStatDistV(PMat, StatDist);

	for (int i = 0; i < States; i++) {
		for (int j = 0; j < States; j++) {
			RevPMat(j,i) = StatDist[i] * PMat(i,j) / StatDist[j];
		}
	}
}

void TDtMChain::GetStatDistV(const TFltVV& PMat, TFltV& DistV) {
	const int States = PMat.GetRows();

	if (DistV.Empty()) { DistV.Gen(States); }
	EAssert(DistV.Len() == States);

	TFltVV PMatT;	TLinAlg::Transpose(PMat, PMatT);

	TNumericalStuff::GetEigenVec(PMatT, 1, DistV);

	//===========================================================
	// TODO remove this assertion after you know this works
	// check if the result is correct
	TFltV PiTimesP;
	TLinAlg::MultiplyT(PMat, DistV, PiTimesP);
	TLinAlg::LinComb(1, DistV, -1, PiTimesP, PiTimesP);
	const double PiQNorm = TLinAlg::Norm(PiTimesP);
	EAssertR(PiQNorm < 1e-3, "This is not an eigenvector with eigenvalue 0");
	//===========================================================

	const double Sum = TLinAlg::SumVec(DistV);
	for (int i = 0; i < States; i++) {
		DistV[i] /= Sum;
	}
}

/////////////////////////////////////////////////////////////////
// Continuous time Markov chain
const double TCtMChain::EPS = 1e-6;

void TCtMChain::GetRevQMat(const TFltVV& QMat, TFltVV& RevQMat) {
	const int States = QMat.GetRows();

	if (RevQMat.Empty()) { RevQMat.Gen(States, States); }
	EAssert(RevQMat.GetRows() == States && RevQMat.GetCols() == States);

	TFltV StatDistV;	GetStatDistV(QMat, StatDistV);

	for (int RowN = 0; RowN < States; RowN++) {
		for (int ColN = 0; ColN < States; ColN++) {
			RevQMat(ColN,RowN) = QMat(RowN,ColN) * StatDistV[RowN] / StatDistV[ColN];
		}
	}
}

void TCtMChain::GetStatDistV(const TFltVV& QMat, TFltV& ProbV) {
	const int Dim = QMat.GetRows();

	if (Dim == 1) {	// edge case, will get nans
		ProbV.Gen(1);
		ProbV[0] = 1;
		return;
	}

	// returns the stationary distribution
	// pi*Q = 0
	TFltVV QMatT(QMat.GetCols(), QMat.GetRows());	TLinAlg::Transpose(QMat, QMatT);

//	TNumericalStuff::GetEigenVec(QMatT, 0.0, ProbV);
	TNumericalStuff::GetKernelVec(QMatT, ProbV);

	double EigSum = TLinAlg::SumVec(ProbV);

	EAssertR(EigSum != 0, "Eigenvector should not be 0, norm is " + TFlt::GetStr(TLinAlg::Norm(ProbV)) + "!");
	EAssertR(!TFlt::IsNan(EigSum), "NaNs in eigenvector!");

	//===========================================================
	// TODO remove this assertion after you know this works
	// check if the result is correct
	TFltV PiTimesQ;
	TLinAlg::MultiplyT(QMat, ProbV, PiTimesQ);
	const double PiQNorm = TLinAlg::Norm(PiTimesQ);
	EAssertR(PiQNorm < 1e-3, "This is not an eigenvector with eigenvalue 0");
	//===========================================================

	// normalize to get a distribution
	bool NumErr = false;
	for (int StateId = 0; StateId < Dim; StateId++) {
		ProbV[StateId] /= EigSum;

		// fix numerical issues (this happened once when I was testing for the evaluation (using MKLFunctions))
		EAssert(ProbV[StateId] > -1e-3);

		// correct the stationary distribution so we don't get states
		// with probability 0
		if (ProbV[StateId] < EPS) {
			ProbV[StateId] = EPS;
			NumErr = true;
		}
	}

	if (NumErr) {
		EigSum = TLinAlg::SumVec(ProbV);
		for (int StateId = 0; StateId < Dim; StateId++) {
			ProbV[StateId] /= EigSum;
		}
	}
}

void TCtMChain::GetHoldingTmV(const TFltVV& QMat, TFltV& HoldingTmV) {
	const int States = QMat.GetRows();

	if (HoldingTmV.Empty()) { HoldingTmV.Gen(States); }
	EAssert(HoldingTmV.Len() == States);

	double Intens;
	for (int StateId = 0; StateId < States; StateId++) {
		Intens = -QMat(StateId,StateId);
		HoldingTmV[StateId] = 1 / Intens;
	}
}

void TCtMChain::GetProbVV(const TFltVV& QMat, const double& Dt, TFltVV& ProbVV) {
	// P = I + Q*dt
	const int Dim = QMat.GetRows();
	// calculate Q*dt
	ProbVV.Gen(Dim, Dim);
	TLinAlg::MultiplyScalar(Dt, QMat, ProbVV);
	// add I
	for (int i = 0; i < Dim; i++) {
		ProbVV(i, i) += 1;
	}
}

void TCtMChain::GetFutProbVV(const TFltVV& QMat, const double& DeltaTm, const double& Tm,
		TFltVV& ProbVV) {
	const int Steps = int(ceil(Tm / DeltaTm));

	EAssertR(Tm >= 0, "Time is not greater than 0!");
	EAssert(Steps > 0);

	if (Steps > 1) {
		TFltVV PMat;
		// get a probability matrix
		GetProbVV(QMat, DeltaTm, PMat);
		TDtMChain::GetProbVV(PMat, Steps, ProbVV);
	} else {
		GetProbVV(QMat, DeltaTm, ProbVV);
	}
}

void TCtMChain::GetProbVV(const TFltVV& QMat, const double& DeltaTm, const double& Tm,
		TFltVV& ProbVV) {
	if (Tm >= 0) {
		GetFutProbVV(QMat, DeltaTm, Tm, ProbVV);
	} else {
		TFltVV RevQMat;	GetRevQMat(QMat, RevQMat);
		GetFutProbVV(RevQMat, DeltaTm, -Tm, ProbVV);
	}
}

void TCtMChain::GetJumpV(const TFltVV& QMat, const int& CurrStateId, TFltV& JumpV) {
	const int States = QMat.GetRows();

	if (JumpV.Empty()) { JumpV.Gen(States); }
	EAssert(JumpV.Len() == States);

	if (QMat(CurrStateId,CurrStateId) == 0.0) {
		for (int StateId = 0; StateId < States; StateId++) { JumpV[StateId] = 0; }
		JumpV[CurrStateId] = 1;
	} else {
		double Q_ij, Q_ii, J_ij;

		for (int j = 0; j < States; j++) {
			if (j != CurrStateId) {
				Q_ij = QMat(CurrStateId,j);
				Q_ii = -QMat(CurrStateId,CurrStateId);
				J_ij = Q_ij / Q_ii;

				EAssertR(!TFlt::IsNan(J_ij), "Jump matrix contains nan on indexes " + TInt::GetStr(CurrStateId) +", " + TInt::GetStr(j));

				JumpV[j] = J_ij;
			}
		}
		JumpV[CurrStateId] = 0;
	}
}

void TCtMChain::GetJumpVV(const TFltVV& QMat, TFltVV& JumpVV) {
	const int States = QMat.GetRows();

	if (JumpVV.Empty()) { JumpVV.Gen(States, States); }
	EAssert(JumpVV.GetRows() == States && JumpVV.GetCols() == States);

	TFltV JumpV;
	for (int StateId = 0; StateId < States; StateId++) {
		GetJumpV(QMat, StateId, JumpV);
		JumpVV.SetRow(StateId, JumpV);
	}
}

double TCtMChain::HitTmPdf(const TFltVV& QMat, const int& StateId, const int& TargetStateId,
		const double& DeltaTm, const double& TotalTm, const int& PdfBins, TFltV& TmV,
		TFltV& HitProbV) {

	const int States = QMat.GetRows();
	const double TmStep = (TotalTm / PdfBins > 2*DeltaTm) ? (TotalTm / PdfBins) : DeltaTm;
	const int OutputSize = (int) ceil(TotalTm / TmStep) + 1;

	TFltVV PMat;	GetFutProbVV(QMat, DeltaTm, TmStep, PMat);

	TFltVV CurrProbMat(States, States);
	TFltVV TempPMat(States, States);

	// initialize return structures
	double CumHitProb = 0;
	TmV.Gen(OutputSize, OutputSize);
	HitProbV.Gen(OutputSize, OutputSize);
	TFltV ReturnProbV(OutputSize, OutputSize);

	TmV[0] = 0;
	HitProbV[0] = TargetStateId == StateId ? 1 : 0;
	ReturnProbV[0] = 1;

	if (TargetStateId == StateId) { return 1; }

	// P(0) = I
	for (int i = 0; i < States; i++) { CurrProbMat(i, i) = 1; }

	double CumReturnProb, HitProb, ReturnProb, Prob;

	int StepN = 1;
	double CurrTm = TmStep;

	while (CurrTm <= TotalTm) {
		// P(nh) <- P((n-1)h)*P(h)
		TLinAlg::Multiply(CurrProbMat, PMat, TempPMat);
		std::swap(CurrProbMat, TempPMat);

		ReturnProb = CurrProbMat(TargetStateId, TargetStateId);
		EAssertR(!TFlt::IsNan(ReturnProb), "The return probability is nan!");

		Prob = CurrProbMat(StateId, TargetStateId);
		ReturnProbV[StepN] = ReturnProb;

		CumReturnProb = 0;
		for (int TmN = 0; TmN < StepN; TmN++) {
			CumReturnProb += HitProbV[TmN]*ReturnProbV[StepN - TmN];
		}

		EAssertR(!TFlt::IsNan(Prob), "The probability of reachig the target state is nan!");
		HitProb = Prob - CumReturnProb*TmStep;
		EAssertR(!TFlt::IsNan(HitProb), "The HitProb is nan!");

		CumHitProb += HitProb;
		HitProbV[StepN] = HitProb;
		TmV[StepN] = CurrTm;

		StepN++;
		CurrTm += TmStep;
	}

	return CumHitProb * TmStep;
}

void TCtMChain::GetAggrQMat(const TFltVV& QMat, const TAggStateV& AggrStateV,
		TFltVV& AggrQMat) {
	const int NAggrStates = AggrStateV.Len();

	if (AggrQMat.Empty()) { AggrQMat.Gen(NAggrStates, NAggrStates); }
	EAssert(AggrQMat.GetRows() == NAggrStates && AggrQMat.GetCols() == NAggrStates);

	TFltV StatDistV;	GetStatDistV(QMat, StatDistV);

	for (int JoinState1Idx = 0; JoinState1Idx < NAggrStates; JoinState1Idx++) {
		const TIntV& JoinState1 = AggrStateV[JoinState1Idx];
		EAssertR(!JoinState1.Empty(), "An aggregated state is empty!");

		for (int JoinState2Idx = 0; JoinState2Idx < NAggrStates; JoinState2Idx++) {
			if (JoinState1Idx == JoinState2Idx) { continue; }

			const TIntV& JoinState2 = AggrStateV[JoinState2Idx];
			EAssertR(!JoinState2.Empty(), "An aggregated state is empty!");

			// the transition probability from set Ai to Aj can be
			// calculated as: q_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} q_{k,l}} {\sum_{k \in A_i} \pi_k}

			double Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];
				const double PiK = StatDistV[JoinState1[k]];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					const double Q_kl = QMat(StateK,StateL);
					SumK += Q_kl;
				}

				Sum += PiK*SumK;
				SumP += PiK;
			}

			AggrQMat(JoinState1Idx, JoinState2Idx) = Sum / SumP;
			AssertR(!TFlt::IsNan(AggrQMat(JoinState1Idx, JoinState2Idx)), "NaN appears when aggregating the QMatrix, this means that the joined stationary distribution is 0 for some states. Please check that the dataset is recurrent!");
		}

		const double Q_ii = -TLinAlg::SumRow(AggrQMat, JoinState1Idx);

		// check if we have zero on the diagonal
		if (NAggrStates > 1 && Q_ii == 0) {
			TStr AggStateVStr;
			for (int AggStateId = 0; AggStateId < AggrStateV.Len(); AggStateId++) {
				AggStateVStr += TStrUtil::GetStr(AggrStateV[AggStateId]) + "\n";
			}
			throw TExcept::New("Aggregated QMatrix has a zero on diagonal:\nQMat:\n" + TStrUtil::GetStr(QMat) + "\nAgg states:\n" + AggStateVStr);
		}

//		EAssertR(NAggrStates == 1 || Q_ii != 0, "Aggregated QMatrix has a zero on diagonal!");
		AggrQMat(JoinState1Idx, JoinState1Idx) = Q_ii;
	}
}

void TCtMChain::GetSubChain(const TFltVV& QMat, const TIntV& StateIdV, TFltVV& SubQMat) {
	const int SubMatDim = StateIdV.Len();
//	const double Eps = 1e-6;

	if (SubQMat.Empty()) { SubQMat.Gen(SubMatDim, SubMatDim); }
	EAssert(SubQMat.GetRows() == SubMatDim && SubQMat.GetCols() == SubMatDim);

	double RowSum, Val;
	for (int RowN = 0; RowN < SubMatDim; RowN++) {
		RowSum = 0;
		for (int ColN = 0; ColN < SubMatDim; ColN++) {
			if (ColN != RowN) {
				Val = QMat(StateIdV[RowN], StateIdV[ColN]);
				SubQMat(RowN, ColN) = Val;
				RowSum += Val;
			}
		}

		if (RowSum != 0.0) {
			SubQMat(RowN,RowN) = -RowSum;
		}
//		else {	// edge case, add eps to all transitions
//			for (int ColN = 0; ColN < SubMatDim; ColN++) {
//				if (RowN != ColN) {
//					SubQMat(RowN, ColN) = Eps;
//				}
//			}
//			SubQMat(RowN,RowN) = -(SubMatDim-1)*Eps;
//		}
	}

//	// check if any of the states is inaccessible
//	for (int ColN = 0; ColN < SubMatDim; ColN++) {
//		double ColSum = 0;
//
//		for (int RowN = 0; RowN < SubMatDim; RowN++) {
//			if (RowN != ColN) {
//				ColSum += SubQMat(RowN, ColN);
//			}
//		}
//
//		if (ColSum == 0) {
//			for (int RowN = 0; RowN < SubMatDim; RowN++) {
//				if (RowN != ColN) {
//					SubQMat(RowN, ColN) = Eps;
//					SubQMat(RowN, RowN) -= Eps;
//				}
//			}
//		}
//	}
}

void TCtMChain::BiPartition(const TFltVV& QMat, const TFltV& ProbV, TIntV& PartV) {
	const int Dim = QMat.GetRows();

	if (PartV.Empty()) { PartV.Gen(Dim); }
	EAssert(PartV.Len() == Dim);

	// calculate the symmetrized laplacian Qs = (Pi*Q + Q'*Pi) / 2
	// the stationary distribution
//	TFltV ProbV;	GetStatDistV(QMat, ProbV);

	TFltVV QSim(Dim, Dim);
	for (int RowN = 0; RowN < Dim; RowN++) {
		for (int ColN = 0; ColN < Dim; ColN++) {
			QSim(RowN, ColN) = (ProbV[RowN]*QMat(RowN, ColN) + QMat(ColN, RowN)*ProbV[ColN]) / 2;
		}
	}

	// solve the following generalized eigenvalue problem: Qs*v = l2*Pi*v
	TFltVV Pi;	TLinAlgTransform::Diag(ProbV, Pi);
//
	printf("Q:\n%s\n", TStrUtil::GetStr(QMat, ",", "%.15f").CStr());
	printf("QSim:\n%s\n", TStrUtil::GetStr(QSim, ",", "%.15f").CStr());
	printf("Pi:\n%s\n", TStrUtil::GetStr(Pi, ",", "%.15f").CStr());

	TFltVV EigVecVV;	TFltV EigValV;
	TLinAlg::GeneralizedEigDecomp(QSim, Pi, EigValV, EigVecVV);

	// find the second largest eigenvalue
	double MaxEig = TFlt::NInf;
	double SecondMaxEig = TFlt::NInf;
	int MaxEigN = -1;
	int SecondMaxEigN = -1;

	double EigVal;
	for (int EigN = 0; EigN < Dim; EigN++) {
		EigVal = EigValV[EigN];
		if (EigVal > MaxEig) {
			SecondMaxEig = MaxEig;
			SecondMaxEigN = MaxEigN;
			MaxEig = EigVal;
			MaxEigN = EigN;
		} else if (EigVal > SecondMaxEig) {
			SecondMaxEig = EigVal;
			SecondMaxEigN = EigN;
		}
	}

	// extract the partition from the eigenvector corresponding to the second
	// largest eigevvalue
	TFltV EigV;	EigVecVV.GetCol(SecondMaxEigN, EigV);
	for (int i = 0; i < Dim; i++) {
		PartV[i] = EigV[i] >= 0 ? 1 : 0;
	}
}

void TCtMChain::Partition(const TFltVV& QMat, TIntV& HierarchV, TFltV& HeightV) {
	const int NStates = QMat.GetCols();
	const int NSplits = NStates-1;

	const int TotalStates = 2*NStates - 1;
	const int RootId = TotalStates - 1;

	TFltV StatDistV;	GetStatDistV(QMat, StatDistV);

	HierarchV.Gen(TotalStates);
	HeightV.Gen(TotalStates);
	TIntV StateIdV;

	// initialize
	// the initial partition has only one super state with all the states inside
	TAggStateV CurrAggStateV(TotalStates, 0);
	CurrAggStateV.Add(TAggState(NStates, 0));
	for (int StateN = 0; StateN < NStates; StateN++) {
		CurrAggStateV[0].Add(StateN);
	}

	// insert the initial partition into the result
	HierarchV[RootId] = RootId;
	HeightV[RootId] = RelativeEntropy(QMat, CurrAggStateV);//TFlt::PInf;
	StateIdV.Add(RootId);

	// run the algorithm
	double MinDist;
	int BestStateN;
	TIntV BestPartV;

	int TempCurrStateId;
	int CurrStateId = RootId;
	for (int SplitN = 0; SplitN < NSplits; SplitN++) {
		MinDist = TFlt::PInf;
		BestStateN = -1;

		printf("====================================================================\n");
		printf("====================================================================\n");
		printf("State sets:\n");
		for (int i = 0; i < CurrAggStateV.Len(); i++) {
			printf("%s\n", TStrUtil::GetStr(CurrAggStateV[i], ",").CStr());
		}

		for (int StateN = 0; StateN < CurrAggStateV.Len(); StateN++) {
			printf("%s\n", TStrUtil::GetStr(CurrAggStateV[StateN], ",").CStr());
			if (CurrAggStateV[StateN].Len() > 1) {
				// get a sub chain formed from a single state
				const int StateId = StateIdV[StateN];
				const TAggState AggState = CurrAggStateV[StateN];

				// calculate the static distribution in the aggregated state
				TFltV ProbV(AggState.Len());
				double AggProbSum = 0;
				for (int i = 0; i < AggState.Len(); i++) {
					AggProbSum += StatDistV[AggState[i]];
				}
				for (int i = 0; i < AggState.Len(); i++) {
					ProbV[i] = StatDistV[AggState[i]] / AggProbSum;
				}

				// partition the state
				TFltVV SubQMat;	GetSubChain(QMat, AggState, SubQMat);
				TIntV BiPartV;	BiPartition(SubQMat, ProbV, BiPartV);

				// add the two new partitions to the current chain
				TempCurrStateId = CurrStateId;
				SplitAggState(StateN, BiPartV, CurrAggStateV, StateIdV, TempCurrStateId);

				// calculate the distance to the original chain
				const double DistToOrig = RelativeEntropy(QMat, CurrAggStateV);

				if (DistToOrig < MinDist) {
					MinDist = DistToOrig;
					BestStateN = StateN;
					BestPartV = BiPartV;
					printf("Best state:\n%s\n", TStrUtil::GetStr(AggState, ",").CStr());
				}

				printf("index %d, dist: %.5f, best: %.5f\n", StateN, DistToOrig, MinDist);

				// restore the partition to the way it was
				StateIdV.DelLast();
				StateIdV.DelLast();
				CurrAggStateV.DelLast();
				CurrAggStateV.DelLast();

				StateIdV.Ins(StateN, StateId);
				CurrAggStateV.Ins(StateN, AggState);
			}
		}

		EAssert(BestStateN >= 0);

		printf("====================================================================\n");

		printf("Best state idx: %d\n", BestStateN);

		// bi-partition the best candidate
		const int ParentId = StateIdV[BestStateN];
		const TAggState ParentState = CurrAggStateV[BestStateN];

		printf("Parent id: %d\n", ParentId);
		printf("Parent state: %s\n", TStrUtil::GetStr(ParentState).CStr());
		printf("Partition: %s\n", TStrUtil::GetStr(BestPartV).CStr());

		// insert the two new states
		SplitAggState(BestStateN, BestPartV, CurrAggStateV, StateIdV, CurrStateId);

		// insert this into the result
		// get the index of the new states parent
		AddAggSet(QMat, CurrAggStateV.Len()-2, CurrAggStateV, StateIdV, ParentId, MinDist, HierarchV, HeightV);
		AddAggSet(QMat, CurrAggStateV.Len()-1, CurrAggStateV, StateIdV, ParentId, MinDist, HierarchV, HeightV);
	}

	// clean the hierarchy vector for the bottom-most states (numerical errors)
	for (int i = 0; i < NStates; i++ ) {
		HeightV[i] = 0.0;
	}

	printf("State sets:\n");
	for (int i = 0; i < CurrAggStateV.Len(); i++) {
		printf("%s\n", TStrUtil::GetStr(CurrAggStateV[i], ",").CStr());
	}

	TFltV TempHeightV;	HeightV.GetSubValV(0, HeightV.Len()-2, TempHeightV);
	printf("Number of leafs: %d\n", NStates);
	printf("HeightV: %s\n", TStrUtil::GetStr(TempHeightV, ",", "%.15f").CStr());
	printf("Hierarchy: %s\n", TStrUtil::GetStr(HierarchV).CStr());
}

double TCtMChain::WassersteinDist1(const TFltVV& QMat, const TAggStateV& StateSetV) {
	const int NStates = QMat.GetRows();
	const int NAggStates = StateSetV.Len();

	// the distance is defined as:
	// \sum_{i \in X}\sum_{\psi(j) \in Y} |\sum_{j \in \psi(j)} (p_{\psi(i)}inv(P'\Pi P)P'\Pi - p_i)inv(Q)p_j|

	TFltVV QInv;	TLinAlg::InverseSVD(QMat, QInv, 1e-9);
	TFltV ProbV;	GetStatDistV(QMat, ProbV);

	TFltV ProjDiffV(NStates);
	TFltV ProjDiffTimesQ(NStates);

	double Dist = 0;
	for (int PsiiN = 0; PsiiN < NAggStates; PsiiN++) {
		const TAggState& PsiI = StateSetV[PsiiN];

		// construct the left projection vector: p_{\psi(i)}inv(P'\Pi P)P'\Pi = \tilde(p)_{\psi(i)}
		// first clear the projection
		for (int StateN = 0; StateN < NStates; StateN++) {
			ProjDiffV[StateN] = 0;
		}

		// compute \tilde(p)_{\psi(i)}
		// fill the projection for the current aggregated state
		double PsiIProb = 0;
		for (int i = 0; i < PsiI.Len(); i++) {
			PsiIProb += ProbV[PsiI[i]];
		}
		for (int i = 0; i < PsiI.Len(); i++) {
			const int StateN = PsiI[i];
			ProjDiffV[StateN] = ProbV[StateN] / PsiIProb;
		}

		// do the work
		for (int i = 0; i < PsiI.Len(); i++) {
			const int StateI = PsiI[i];

			// compute dp = \tilde(p)_{\psi(i)} - p_i
			// set the projection for this particular i
			ProjDiffV[StateI] = ProbV[StateI] / PsiIProb - 1;

			// multiply dp*Q
			TLinAlg::MultiplyT(QInv, ProjDiffV, ProjDiffTimesQ);

			for (int PsijN = 0; PsijN < NAggStates; PsijN++) {
				const TAggState& PsiJ = StateSetV[PsijN];

				double Sum = 0;
				for (int j = 0; j < PsiJ.Len(); j++) {
					const int StateJ = PsiJ[j];
					Sum += ProjDiffTimesQ[StateJ];
				}

				Dist += TFlt::Abs(Sum);
			}

			// clear the projection for this particular i
			ProjDiffV[StateI] = ProbV[StateI] / PsiIProb;
		}
	}

	return Dist;
}

double TCtMChain::RelativeEntropy(const TFltVV& QMat, const TAggStateV& AggStateV) {
	TFltVV JoinedQ;	GetAggrQMat(QMat, AggStateV, JoinedQ);
	TFltV ProbV;	GetStatDistV(QMat, ProbV);

	const int NAggStates = AggStateV.Len();

	double Sum1 = 0, Sum2 = 0;

	int i, j;
	for (int PsiiN = 0; PsiiN < NAggStates; PsiiN++) {
		const TAggState& PsiI = AggStateV[PsiiN];

		for (int PsijN = 0; PsijN < NAggStates; PsijN++) {
			if (PsiiN != PsijN) {
				const TAggState& PsiJ = AggStateV[PsijN];

				for (int StateIN = 0; StateIN < PsiI.Len(); StateIN++) {
					i = PsiI[StateIN];

					double IntensIToPsiJ = 0;
					for (int StateJN = 0; StateJN < PsiJ.Len(); StateJN++) {
						j = PsiJ[StateJN];

						IntensIToPsiJ += QMat(i,j);
					}

					// log(0/0) == 0
					if (TFlt::Abs(IntensIToPsiJ) > 1e-9) {	// numerical errors
						Sum1 += ProbV[i]*IntensIToPsiJ*TMath::Log(IntensIToPsiJ / JoinedQ(PsiiN,PsijN));
					}
				}
			}
		}

		for (int StateIN = 0; StateIN < PsiI.Len(); StateIN++) {
			i = PsiI[StateIN];

			double IntensIToPsiI = 0;
			for (int StateJN = 0; StateJN < PsiI.Len(); StateJN++) {
				j = PsiI[StateJN];
				IntensIToPsiI += QMat(i,j);
			}

			Sum2 += ProbV[i]*(IntensIToPsiI - JoinedQ(PsiiN,PsiiN));
		}
	}

	return Sum1 + Sum2;
}

void TCtMChain::AddAggSet(const TFltVV& QMat, const int& StateN, const TAggStateV& AggStateV,
		const TIntV& StateIdV, const int& ParentId, const double& MinDist,
		TIntV& HierarchV, TFltV& HeightV) {

	const int NLeafs = QMat.GetCols();

	const int StateId = StateIdV[StateN];
	const TAggState& AggState = AggStateV[StateN];

	TIntSet AggStateH(AggState);
	TAggStateV TempAggStateV;
	for (int StateId = 0; StateId < NLeafs; StateId++) {
		if (!AggStateH.IsKey(StateId)) {
			TempAggStateV.Add(TAggState());
			TempAggStateV.Last().Add(StateId);
		}
	}
	TempAggStateV.Add(AggState);

	HierarchV[StateId] = ParentId;
	HeightV[StateId] = RelativeEntropy(QMat, TempAggStateV);
}

void TCtMChain::SplitAggState(const int& StateN, const TIntV& PartV, TAggStateV& AggStateV, TIntV& StateIdV, int& CurrStateId) {
	const TAggState& AggState = AggStateV[StateN];

	AggStateV.Add(TAggState());
	AggStateV.Add(TAggState());

	TAggState& NewState0 = AggStateV[AggStateV.Len()-2];
	TAggState& NewState1 = AggStateV[AggStateV.Len()-1];
	for (int i = 0; i < PartV.Len(); i++) {
		if (PartV[i] == 0) {
			NewState0.Add(AggState[i]);
		} else {
			NewState1.Add(AggState[i]);
		}
	}

	StateIdV.Add(NewState0.Len() == 1 ? int(NewState0[0]) : --CurrStateId);
	StateIdV.Add(NewState1.Len() == 1 ? int(NewState1[0]) : --CurrStateId);

	printf("New state 0: %s\n", TStrUtil::GetStr(NewState0).CStr());
	printf("New state 1: %s\n", TStrUtil::GetStr(NewState1).CStr());

	StateIdV.Del(StateN);
	AggStateV.Del(StateN);
}


/////////////////////////////////////////////////////////////////
// Abstract Markov Chain
//TTransitionModeler::TTransitionModeler(const bool& _Verbose):
//		NStates(-1),
//		CurrStateId(-1),
//		TmHorizon(1),
//		PredictionThreshold(.5),
//		PdfBins(100),
//		HasHiddenState(false),
//		Verbose(_Verbose),
//		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

//TTransitionModeler::TTransitionModeler(TSIn& SIn):
//		NStates(TInt(SIn)),
//		CurrStateId(TInt(SIn)),
//		TmHorizon(TFlt(SIn)),
//		PredictionThreshold(TFlt(SIn)),
//		PdfBins(TInt(SIn)),
//		HasHiddenState(TBool(SIn)),
//		Verbose(TBool(SIn)),
//		Notify(nullptr) {
//
//	PdfBins = 100;
//	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
//}


//void TTransitionModeler::Save(TSOut& SOut) const {
//	GetType().Save(SOut);
//	TInt(NStates).Save(SOut);
//	TInt(CurrStateId).Save(SOut);
//	TFlt(TmHorizon).Save(SOut);
//	TFlt(PredictionThreshold).Save(SOut);
//	TInt(PdfBins).Save(SOut);
//	TBool(HasHiddenState).Save(SOut);
//	TBool(Verbose).Save(SOut);
//}

//TTransitionModeler* TTransitionModeler::Load(TSIn& SIn) {
//	const TStr Type(SIn);
//
//	if (Type == "continuous") {
//		return new TCtModeler(SIn);
//	} else {
//		throw TExcept::New("Invalid type of Markov chain: " + Type, "TMChain::Load");
//	}
//}

//void TTransitionModeler::Init(const TFltVV& FtrVV, const int& _NStates, const TIntV& AssignV,
//		const TUInt64V& TmV, const bool _HasHiddenState, const TBoolV& EndBatchV) {
//	NStates = _NStates;
//	HasHiddenState = _HasHiddenState;
//
//	Notify->OnNotify(TNotifyType::ntInfo, "Initializing Markov chain ...");
//	InitIntensities(FtrVV, TmV, AssignV, EndBatchV);
//	Notify->OnNotify(TNotifyType::ntInfo, "Done!");
//
//	Notify->OnNotify(TNotifyType::ntInfo, "Running the examples through the Markov chain ...");
//
//	const int NInst = FtrVV.GetCols();
//	if (EndBatchV.Empty()) {
//		for (int i = 0; i < NInst; i++) {
//			OnAddRec(AssignV[i], TmV[i], false);
//		}
//	} else {
//		for (int i = 0; i < NInst; i++) {
//			OnAddRec(AssignV[i], TmV[i], EndBatchV[i]);
//		}
//	}
//
//
//	Notify->OnNotify(TNotifyType::ntInfo, "Markov chain initialization complete!");
//}

//void TTransitionModeler::OnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) {
//	EAssertR(HasHiddenState || !EndsBatch, "Cannot be last in sequence if a hidden state does not exist!");
//
//	// call child method
//	AbsOnAddRec(StateId, RecTm, EndsBatch);
//	if (HasHiddenState && EndsBatch) {
//		CurrStateId = GetHiddenStateId();
//	} else {
//		CurrStateId = StateId;
//	}
//}

//void TTransitionModeler::GetFutureProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
//		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
//		TIntFltPrV& StateIdProbV) const {
//
//	const int StateIdx = StateIdV.SearchForw(StateId);
//
//	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");
//
//	TFltVV ProbVV;	GetFutureProbVV(StateSetV, StateFtrVV, Tm, ProbVV);
//	TFltV ProbV;	TLAUtil::GetRow(ProbVV, StateIdx, ProbV);
//
//	for (int i = 0; i < StateIdV.Len(); i++) {
//		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
//	}
//}

//void TTransitionModeler::GetPastProbV(const TStateSetV& StateSetV, const TStateFtrVV& StateFtrVV,
//		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
//		TIntFltPrV& StateIdProbV) const {
//
//	const int StateIdx = StateIdV.SearchForw(StateId);
//
//	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");
//
//	TFltVV ProbVV;	GetPastProbVV(StateSetV, StateFtrVV, Tm, ProbVV);
//	TFltV ProbV;	TLAUtil::GetRow(ProbVV, StateIdx, ProbV);
//
//	for (int i = 0; i < StateIdV.Len(); i++) {
//		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
//	}
//}

//void TTransitionModeler::SetVerbose(const bool& _Verbose) {
//	if (_Verbose != Verbose) {
//		Verbose = _Verbose;
//		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
//	}
//}

//void TTransitionModeler::InsHiddenState(TStateSetV& StateSetV) const {
//	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");
//
//	StateSetV.Add(TIntV(1, 1));
//	StateSetV.Last()[0] = GetHiddenStateId();
//}

//void TTransitionModeler::InsHiddenState(TStateIdV& StateIdV) const {
//	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");
//	StateIdV.Add(GetHiddenStateId());
//}

//void TTransitionModeler::RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const {
//	EAssertR(HasHiddenState, "TMChain::RemoveHiddenStateProb: The model does not have a hidden state!");
//
//	const int HiddenStateId = GetHiddenStateId();
//
//	for (int i = 0; i < StateIdProbV.Len(); i++) {
//		if (StateIdProbV[i].Val1 == HiddenStateId) {
//			StateIdProbV.Del(i);
//			break;
//		}
//	}
//}

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
const uint64 TCtmcModeller::TU_SECOND = 1000;
const uint64 TCtmcModeller::TU_MINUTE = TU_SECOND*60;
const uint64 TCtmcModeller::TU_HOUR = TU_MINUTE*60;
const uint64 TCtmcModeller::TU_DAY = TU_HOUR*24;
const uint64 TCtmcModeller::TU_MONTH = uint64(365.25 * TU_DAY / 12);

const double TCtmcModeller::MIN_STAY_TM = 1e-2;
const double TCtmcModeller::HIDDEN_STATE_INTENSITY = 1 / MIN_STAY_TM;

TCtmcModeller::TCtmcModeller(const uint64& _TimeUnit, const double& _DeltaTm, const bool& _Verbose):
		NStates(-1),
		CurrStateId(-1),
		PrevJumpTm(-1),
		IntensModel(),
		HasHiddenState(false),
		HiddenStateJumpCountV(),
		TimeUnit(_TimeUnit),
		DeltaTm(_DeltaTm),
		TmHorizon(1),
		PredictionThreshold(.5),
		PdfBins(100),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TCtmcModeller::TCtmcModeller(TSIn& SIn):
		NStates(TInt(SIn)),
		CurrStateId(TInt(SIn)),
		PrevJumpTm(TUInt64(SIn)),
		IntensModel(SIn),
		HasHiddenState(TBool(SIn)),
		HiddenStateJumpCountV(SIn),
		TimeUnit(TUInt64(SIn)),
		DeltaTm(TFlt(SIn)),
		TmHorizon(TFlt(SIn)),
		PredictionThreshold(TFlt(SIn)),
		PdfBins(TInt(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TCtmcModeller::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	TInt(NStates).Save(SOut);
	TInt(CurrStateId).Save(SOut);
	TUInt64(PrevJumpTm).Save(SOut);
	IntensModel.Save(SOut);
	TBool(HasHiddenState).Save(SOut);
	HiddenStateJumpCountV.Save(SOut);
	TUInt64(TimeUnit).Save(SOut);
	TFlt(DeltaTm).Save(SOut);
	TFlt(TmHorizon).Save(SOut);
	TFlt(PredictionThreshold).Save(SOut);
	TInt(PdfBins).Save(SOut);
	TBool(Verbose).Save(SOut);
}

TCtmcModeller* TCtmcModeller::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "continuous") {
		return new TCtmcModeller(SIn);
	} else {
		throw TExcept::New("Invalid type of Markov chain: " + Type, "TMChain::Load");
	}
}

void TCtmcModeller::Init(const TFltVV& FtrVV, const int& _NStates, const TIntV& AssignV,
		const TUInt64V& TmV, const bool _HasHiddenState, const TBoolV& EndBatchV) {
	NStates = _NStates;
	HasHiddenState = _HasHiddenState;

	Notify->OnNotify(TNotifyType::ntInfo, "Initializing Markov chain ...");
	InitIntensities(FtrVV, TmV, AssignV, EndBatchV);
	Notify->OnNotify(TNotifyType::ntInfo, "Done!");

	Notify->OnNotify(TNotifyType::ntInfo, "Running the examples through the Markov chain ...");

	const int NInst = FtrVV.GetCols();
	if (EndBatchV.Empty()) {
		for (int i = 0; i < NInst; i++) {
			OnAddRec(AssignV[i], TmV[i], false);
		}
	} else {
		for (int i = 0; i < NInst; i++) {
			OnAddRec(AssignV[i], TmV[i], EndBatchV[i]);
		}
	}


	Notify->OnNotify(TNotifyType::ntInfo, "Markov chain initialization complete!");
}

void TCtmcModeller::OnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot be last in sequence if a hidden state does not exist!");

	// call child method
	AbsOnAddRec(StateId, RecTm, EndsBatch);
	if (HasHiddenState && EndsBatch) {
		CurrStateId = GetHiddenStateId();
	} else {
		CurrStateId = StateId;
	}
}

void TCtmcModeller::GetFutureProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TFltVV ProbVV;	GetFutureProbVV(StateSetV, StateFtrVV, Tm, ProbVV);
	TFltV ProbV;	ProbVV.GetRow(StateIdx, ProbV);//TLAUtil::GetRow(ProbVV, StateIdx, ProbV);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TCtmcModeller::GetPastProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TFltVV ProbVV;	GetPastProbVV(StateSetV, StateFtrVV, Tm, ProbVV);
	TFltV ProbV;	ProbVV.GetRow(StateIdx, ProbV);//TLAUtil::GetRow(ProbVV, StateIdx, ProbV);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TCtmcModeller::GetNextStateProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV, const TStateIdV& ExtStateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const {

	TStateIdV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);
	GetNextStateProbV(QMat, StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtmcModeller::GetPrevStateProbV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& ExtStateIdV, const int& StateId, TIntFltPrV& StateIdProbV,
		const int& NFutStates) const {

	TStateIdV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	TFltVV RevQMat;	GetRevQMatrix(StateSetV, StateFtrVV, RevQMat);
	GetNextStateProbV(RevQMat, StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtmcModeller::GetLikelyPathTree(const int& StartStateId, const TIntV& StateIdV, const TAggStateV& AggStateV,
		const TStateFtrVV& StateFtrVV, const int& MxDepth, TFltVV& PMat, TIntV& PMatStateIdV,
		const double& TransThreshold) const {

	int TotalStates = 0;
	PJsonVal RootJson = GetLikelyPathTree(StartStateId, StateIdV, AggStateV, StateFtrVV, MxDepth, TotalStates, TransThreshold);

	PJsonVal RootWrapper = TJsonVal::NewObj();
	RootWrapper->AddToObj("prob", 1);
	RootWrapper->AddToObj("state", RootJson);

	PMat.Gen(TotalStates, TotalStates);
	PMatStateIdV.Gen(TotalStates);

	int CurrStateN = 0;
	PMatFromTree(RootWrapper, CurrStateN, PMat, PMatStateIdV);
}

void TCtmcModeller::GetProbVAtTime(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const TStateIdV& StateIdV, const int& StartStateId, const double& Tm,
		TFltV& ProbV) const {

	TFltVV QMat, ProbVV;

	GetQMatrix(StateSetV, StateFtrVV, QMat);
	TCtMChain::GetProbVV(QMat, DeltaTm, Tm, ProbVV);

	const int StateIdx = StateIdV.SearchForw(StartStateId);
	EAssertR(StateIdx >= 0, "Could not find target state!");

	ProbVV.GetRow(StateIdx, ProbV);
}

bool TCtmcModeller::PredictOccurenceTime(const TStateFtrVV& StateFtrVV, const TAggStateV& StateSetV,
		const TStateIdV& StateIdV, const int& CurrStateId, const int& TargetStateId,
		double& Prob, TFltV& ProbV, TFltV& TmV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Predicting occurrence time from state %d to state %d ...", CurrStateId, TargetStateId);

	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);

	// find the index of the current and target states on this height
	const int CurrStateIdx = StateIdV.SearchForw(CurrStateId);
	const int TargetStateIdx = StateIdV.SearchForw(TargetStateId);

	EAssertR(CurrStateIdx >= 0, "Could not find the start state!");
	EAssertR(TargetStateIdx >= 0, "Could not find the start state!");

	Prob = TCtMChain::HitTmPdf(QMat, CurrStateIdx, TargetStateIdx, DeltaTm, TmHorizon,
			PdfBins, TmV, ProbV);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Will reach state %d from state %d with prob: %.4f in time %.4f", CurrStateId, TargetStateId, Prob, TmHorizon);

	if (Prob < PredictionThreshold) { return false; }

	if (PdfBins > 0) {
		// reduce the data sent to the outside world by intepolating the PDF
		const int SamplingRate = ProbV.Len() / PdfBins;
		TFltV NewProbV(PdfBins, 0), NewTmV(PdfBins, 0);

		for (int i = 0; i < PdfBins; i++) {
			NewProbV.Add(ProbV[i*SamplingRate]);
			NewTmV.Add(TmV[i*SamplingRate]);
		}

		ProbV = NewProbV;
		TmV = NewTmV;
	}

	return true;
}

void TCtmcModeller::GetStatDist(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		TFltV& StatDist) const {

	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);
	TCtMChain::GetStatDistV(QMat, StatDist);
}

void TCtmcModeller::GetJumpVV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		TFltVV& JumpVV) const {
	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);

	GetJumpVV(QMat, JumpVV);
	if (HasHiddenState) {
		// take the jump matrix and remove the last row and column
		// don't normalize the rows, so the person can see which are the end states
		TFltVV SubJumpVV;	TLinAlg::SubMat(JumpVV, 0, JumpVV.GetRows()-1, 0, JumpVV.GetCols()-1, SubJumpVV);
		JumpVV = SubJumpVV;
	}
}

void TCtmcModeller::GetQMatrix(const TAggStateV& InStateSetV, const TStateFtrVV& StateFtrVV,
		TFltVV& JoinedQMat) const {
	TAggStateV StateSetV(InStateSetV);

	if (HasHiddenState) {
		InsHiddenState(StateSetV);
	}

	TFltVV QMat; GetQMatrix(StateFtrVV, QMat);
	TCtMChain::GetAggrQMat(QMat, StateSetV, JoinedQMat);
}

void TCtmcModeller::GetSubQMatrix(const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV,
			const TAggState& TargetState, TFltVV& SubQMat) {

	TFltVV QMat;	GetQMatrix(StateFtrVV, QMat);
	TCtMChain::GetSubChain(QMat, TargetState, SubQMat);
}

void TCtmcModeller::GetHoldingTmV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV, TFltV& HoldingTmV) const {
	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);
	GetHoldingTmV(QMat, HoldingTmV);
	if (HasHiddenState) {
		HoldingTmV.DelLast();
	}
}

bool TCtmcModeller::IsAnomalousJump(const TFltV& FtrV, const int& NewStateId, const int& OldStateId) const {
	TFltV IntensV;	GetStateIntensV(OldStateId, FtrV, IntensV);

	return IntensV[NewStateId] / (-IntensV[OldStateId]) < 1e-3;
}

void TCtmcModeller::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

void TCtmcModeller::AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot process batches with no hidden state!");

	// warn if times don't aren't ascending
	if (CurrStateId != -1 && RecTm < PrevJumpTm && (!HasHiddenState || CurrStateId != GetHiddenStateId())) {	// got past time, do not update the statistics
		TNotify::StdNotify->OnNotifyFmt(TNotifyType::ntWarn, "Current time smaller that previous time curr: %ld, prev: %ld", RecTm, PrevJumpTm);
		PrevJumpTm = RecTm;
		return;
	}
}

void TCtmcModeller::GetFutureProbVV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const double& Tm, TFltVV& ProbVV) const {

	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);
	GetFutureProbVV(QMat, Tm, DeltaTm, ProbVV, HasHiddenState);
}

void TCtmcModeller::GetPastProbVV(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		const double& Tm, TFltVV& ProbVV) const {

	TFltVV QMat;	GetRevQMatrix(StateSetV, StateFtrVV, QMat);
	GetFutureProbVV(QMat, Tm, DeltaTm, ProbVV, HasHiddenState);
}

void TCtmcModeller::InitIntensities(const TFltVV& FtrVV, const TUInt64V& TmV,
		const TIntV& AssignV, const TBoolV& EndBatchV) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Modeling intensities ...");

	// TODO handle hidden states

	const double RegFact = 1e-3;

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();
	const int NStates = GetStates() + (HasHiddenState ? 1 : 0);

	TLabelVMat LabelVMat;							// stores the class labels
	TJumpFtrVVMat JumpFtrVVMat(NStates, NStates);	// used when constructing feature vectors
	TJumpFtrMatMat JumpFtrMatVV(NStates, NStates);	// stores the feature vectors

	TBoolVV HasJumpedVV(NStates, NStates);

	for (int i = 0; i < NStates; i++) {
		LabelVMat.Add(TVec<TLabelV>());
		for (int j = 0; j < NStates; j++) {
			LabelVMat[i].Add(TLabelV());
		}
	}

	int CurrStateId, NextStateId;
	double Label;
	double MeanSampleInterval = 0;
	uint64 DeltaTm;
	TFltV FtrV;
	for (int CurrTmN = 0; CurrTmN < NInst-1; CurrTmN++) {
		CurrStateId = AssignV[CurrTmN];
		NextStateId = AssignV[CurrTmN+1];

		DeltaTm = TmV[CurrTmN+1] - TmV[CurrTmN];
		MeanSampleInterval += double(DeltaTm) / double(TimeUnit);

		EAssertR(DeltaTm > 0, "Delta time is not positive time: " + TmV[CurrTmN+1].GetStr() + ", prev time: " + TmV[CurrTmN].GetStr() + ", index: " + TInt::GetStr(CurrTmN));

		FtrVV.GetCol(CurrTmN, FtrV);				// feature vector

		for (int JumpStateId = 0; JumpStateId < NStates; JumpStateId++) {
			Label = JumpStateId == NextStateId ? 1 : 0;

			JumpFtrVVMat(CurrStateId, JumpStateId).Add(FtrV);
			LabelVMat[CurrStateId][JumpStateId].Add(Label);
		}

		HasJumpedVV(CurrStateId, NextStateId) = true;
	}

	MeanSampleInterval /= (NInst - 1);

	// construct feature matrices
	for (int State1Id = 0; State1Id < NStates; State1Id++) {
		if (HasHiddenState && State1Id == GetHiddenStateId()) { continue; }

		for (int State2Id = 0; State2Id < NStates; State2Id++) {
			const TFtrVV& JumpFtrVV = JumpFtrVVMat(State1Id, State2Id);

			const int NRows = Dim;
			const int NCols = JumpFtrVV.Len();

			JumpFtrMatVV.PutXY(State1Id, State2Id, TJumpFtrMat(FtrVV.GetRows(), JumpFtrVV.Len()));

			if (NCols == 0) { continue; }

			TJumpFtrMat& JumpFtrMat = JumpFtrMatVV(State1Id, State2Id);
			for (int RowN = 0; RowN < NRows; RowN++) {
				for (int ColN = 0; ColN < NCols; ColN++) {
					JumpFtrMat(RowN, ColN) = JumpFtrVV[ColN][RowN];
				}
			}
		}
	}

	IntensModel = TBernoulliIntens(NStates, MeanSampleInterval, RegFact, HasJumpedVV, Verbose);

	// fit the regression models

	TBoolV HasJumpedV;
	for (int State1Id = 0; State1Id < NStates; State1Id++) {
		if (HasHiddenState && State1Id == GetHiddenStateId()) { continue; }

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Regressing intensities for state %d", State1Id);
		for (int State2Id = 0; State2Id < NStates; State2Id++) {
			const TJumpFtrMat& JumpFtrVV = JumpFtrMatVV(State1Id, State2Id);
			const TLabelV& LabelV = LabelVMat[State1Id][State2Id];

			if (LabelV.Empty() || TLinAlgCheck::IsZero(LabelV)) {
				continue;
			}

			//============================================================
			// TODO delete me
			int JumpN = 0;
			for (int i = 0; i < LabelV.Len(); i++) {
				const double Label = LabelV[i];
				if (Label > 0) {
					JumpN++;
				}
			}
			EAssertR(JumpN > 0, "WTF!? How did a zero vector get here???");
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d jumps from state %d to %d ...", JumpN, State1Id, State2Id);
			//============================================================

			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Fitting a regression model from state %d to %d", State1Id, State2Id);
			IntensModel.Fit(State1Id, State2Id, JumpFtrVV, LabelV);
		}
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Done!");
}

void TCtmcModeller::GetStateIntensV(const int StateId, const TFltV& FtrV, TFltV& IntensV) const {
	const int NStates = GetStates();

	if (IntensV.Empty()) { IntensV.Gen(NStates); }
	AssertR(IntensV.Len() == NStates, "TCtMChain::GetStateIntensV: the length of the intensity vector doesn't match the number of states!");

	IntensModel.GetQMatRow(StateId, FtrV, IntensV);
}

void TCtmcModeller::GetQMatrix(const TStateFtrVV& StateFtrVV, TFltVV& QMat) const {
	// compute the intensities
	const int NStates = GetStates();
	IntensModel.GetQMat(StateFtrVV, QMat);

	if (HasHiddenState) {
		const int HiddenStateId = GetHiddenStateId();

		for (int State2Id = 0; State2Id < NStates; State2Id++) {
			if (IsHiddenStateId(State2Id)) { continue; }

			const double Intens = HIDDEN_STATE_INTENSITY * HiddenStateJumpCountV[State2Id];
			QMat(HiddenStateId, State2Id) = Intens;
		}

		const double Q_ii = -TLinAlg::SumRow(QMat, HiddenStateId);//.RowSum(HiddenStateId);
		EAssertR(Q_ii != 0, "Q_ii has a zero row!");
		QMat(HiddenStateId,HiddenStateId) = Q_ii;
	}
}

void TCtmcModeller::GetRevQMatrix(const TAggStateV& StateSetV, const TStateFtrVV& StateFtrVV,
		TFltVV& RevQMat) const {

	TFltVV QMat;	GetQMatrix(StateSetV, StateFtrVV, QMat);
	TCtMChain::GetRevQMat(QMat, RevQMat);
}

void TCtmcModeller::InsHiddenState(TAggStateV& AggStateV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");

	AggStateV.Add(TIntV(1, 1));
	AggStateV.Last()[0] = GetHiddenStateId();
}

void TCtmcModeller::InsHiddenState(TStateIdV& StateIdV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");
	StateIdV.Add(GetHiddenStateId());
}

bool TCtmcModeller::IsHiddenStateId(const int& StateId) const {
	return HasHiddenState && StateId == GetHiddenStateId();
}

int TCtmcModeller::GetHiddenStateId() const {
	return HasHiddenState ? GetStates() : -2;
}

void TCtmcModeller::RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const {
	EAssertR(HasHiddenState, "TMChain::RemoveHiddenStateProb: The model does not have a hidden state!");

	const int HiddenStateId = GetHiddenStateId();

	for (int i = 0; i < StateIdProbV.Len(); i++) {
		if (StateIdProbV[i].Val1 == HiddenStateId) {
			StateIdProbV.Del(i);
			break;
		}
	}
}

PJsonVal TCtmcModeller::GetLikelyPathTree(const int& StartStateId, const TIntV& StateIdV,
		const TAggStateV& AggStateV, const TStateFtrVV& StateFtrVV, const int& MxDepth,
		int& GeneratedStates, const double& TransThreshold) const {

	EAssert(0 <= TransThreshold && TransThreshold <= 1);

	PJsonVal RootJson = TJsonVal::NewObj();
	PJsonVal ChildJsonV = TJsonVal::NewArr();

	if (MxDepth > 0) {
		const int Dim = StateIdV.Len();

		TIntFltPrV NextStateIdProbPrV;
		GetNextStateProbV(AggStateV, StateFtrVV, StateIdV, StartStateId, NextStateIdProbPrV, Dim);

		for (int StateN = 0; StateN < NextStateIdProbPrV.Len(); StateN++) {
			const TIntFltPr& StateIdProbPr = NextStateIdProbPrV[StateN];
			const int& ChildId = StateIdProbPr.Val1;
			const double& Prob = StateIdProbPr.Val2;

			if (Prob < TransThreshold) { break; }

			PJsonVal ChildJson = GetLikelyPathTree(ChildId, StateIdV, AggStateV, StateFtrVV, MxDepth-1, GeneratedStates, TransThreshold);

			PJsonVal ChildWrapJson = TJsonVal::NewObj();
			ChildWrapJson->AddToObj("state", ChildJson);
			ChildWrapJson->AddToObj("prob", Prob);

			ChildJsonV->AddToArr(ChildWrapJson);
		}
	}

	RootJson->AddToObj("id", StartStateId);
	RootJson->AddToObj("children", ChildJsonV);

	GeneratedStates++;

	return RootJson;
}

int TCtmcModeller::PMatFromTree(const PJsonVal& RootJsonWrap, int& CurrStateN, TFltVV& PMat,
			TIntV& PMatStateIdV) const {
	const int RootN = CurrStateN++;

	const PJsonVal RootJson = RootJsonWrap->GetObjKey("state");
	const PJsonVal ChildWrapV = RootJson->GetObjKey("children");
	const int RootId = RootJson->GetObjInt("id");

	for (int ChildN = 0; ChildN < ChildWrapV->GetArrVals(); ChildN++) {
		const PJsonVal ChildJsonWrap = ChildWrapV->GetArrVal(ChildN);
		const double Prob = ChildJsonWrap->GetObjNum("prob");

		const int ChildPMatN = PMatFromTree(ChildJsonWrap, CurrStateN, PMat, PMatStateIdV);
		PMat(RootN, ChildPMatN) = Prob;
	}

	PMatStateIdV[RootN] = RootId;

	return RootN;
}

void TCtmcModeller::GetNextStateProbV(const TFltVV& QMat, const TStateIdV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
		const PNotify& Notify) {

	const int Dim = QMat.GetRows();

	const int NFStates = NFutStates == -1 ? Dim-1 : TMath::Mn(NFutStates, Dim-1);
	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TCtMChain::GetNextStateProbV: Could not find target state!");

	TFltV ProbV;	TCtMChain::GetJumpV(QMat, StateIdx, ProbV);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbV.Len(); j++) {
			if (j != StateIdx && !TakenIdxSet.IsKey(j) && ProbV[j] > MxProb) {
				MxProb = ProbV[j];
				MxIdx = j;
			}
		}

		if (ProbV[MxIdx] <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		StateIdProbV.Add(TIntFltPr(StateIdV[MxIdx], MxProb));
	}
}

void TCtmcModeller::GetFutureProbVV(const TFltVV& QMat, const double& Tm,
		const double& DeltaTm, TFltVV& ProbVV, const bool HasHiddenState) {
	EAssertR(Tm >= 0, "TCtMChain::GetFutureProbMat: does not work for negative time!");

	const int Dim = QMat.GetRows();

	if (Tm == 0) {
		TLinAlgTransform::Identity(Dim, ProbVV);
		return;
	}

	const double QMatNorm = TLinAlg::Frob(QMat);;
	const double Dt = TMath::Mn(DeltaTm / QMatNorm, DeltaTm);

	const int Steps = (int) ceil(Tm / Dt);

	TFltVV ProbMat;	TCtMChain::GetProbVV(QMat, Dt, ProbMat);

	// the probabilities from state i to the hidden state should now go from i to i
	if (HasHiddenState) {
		TFltVV CurrProbMat(Dim, Dim);

		const int Dim = ProbMat.GetRows()-1;

		TLinAlg::SubMat(ProbMat, 0, Dim, 0, Dim, CurrProbMat);
		for (int RowIdx = 0; RowIdx < Dim; RowIdx++) {
			const double HiddenProb = ProbMat(RowIdx, Dim);
			CurrProbMat(RowIdx, RowIdx) += HiddenProb;
		}

		ProbMat = CurrProbMat;
	}

	TLinAlg::Pow(ProbMat, Steps, ProbVV);
}

/////////////////////////////////////////////////////////////////
// Scale helper
void TScaleHelper::CalcNaturalScales(const TScaleDescV& ScaleQMatPrV,
	const TRnd& Rnd, TFltV& ScaleV) const {
	Notify->OnNotify(ntInfo, "Calculating natural scales ...");

	const int NScales = ScaleQMatPrV.Len() / 2;

	TFltVV FtrVV(GetFtrVDim(), ScaleQMatPrV.Len());

	for (int HeightN = 0; HeightN < ScaleQMatPrV.Len(); HeightN++) {
		const TFltVV& QMat = ScaleQMatPrV[HeightN].Val2;
		TFltV FtrV;	GetScaleFtrV(QMat, FtrV);
		FtrVV.SetCol(HeightN, FtrV);
	}

	Assert(!TLinAlgCheck::ContainsNan(FtrVV));

	TClustering::TDenseKMeans KMeans(NScales, Rnd, TCosDist::New());
	KMeans.Apply(FtrVV, false, 1000, Notify);
	TFltVV ClustDistVV;	KMeans.GetDistVV(FtrVV, ClustDistVV);

	printf("\n%s\n", TStrUtil::GetStr(ClustDistVV, ", ", "%.4f").CStr());

	TIntV MedoidIdV;	TLinAlgSearch::GetRowMinIdxV(ClustDistVV, MedoidIdV);

	ScaleV.Gen(NScales);
	for (int MedoidN = 0; MedoidN < NScales; MedoidN++) {
		ScaleV[MedoidN] = ScaleQMatPrV[MedoidIdV[MedoidN]].Val1;
	}

	Notify->OnNotify(ntInfo, "Scales calculated!");
}

/////////////////////////////////////////////////////////////////
// Scale helper - based on eigenvalues
const int TEigValScaleHelper::FTRV_DIM = 3;

void TEigValScaleHelper::GetScaleFtrV(const TFltVV& QMat, TFltV& FtrV) const {
	const int Dim = GetFtrVDim();

	// in the first version the feature vector will contain singular values
	TFltVV U, Vt;
	TFltV Sing;
	TLinAlg::ComputeSVD(QMat, U, Sing, Vt);

	if (FtrV.Len() != Dim) { FtrV.Gen(Dim); }

	for (int SingN = 0; SingN < Dim; SingN++) {
		FtrV[SingN] = SingN < Sing.Len() ? TMath::Sqrt(Sing[SingN]) : 0;
	}
}

/////////////////////////////////////////////////////////////////
// Hierarchy modeler
THierarch::THierarch(const bool& _HistCacheSize, const bool& _IsTransitionBased,
			const TRnd& _Rnd, const bool& _Verbose):
		HierarchV(),
		StateHeightV(),
		UniqueHeightV(),
		NaturalScaleV(),
		MxHeight(TFlt::Mn),
		HistCacheSize(_HistCacheSize),
		PastStateIdV(),
		HierarchHistoryVV(),
		NLeafs(0),
		StateNmV(),
		StateLabelV(),
		IsTransitionBased(_IsTransitionBased),
		Rnd(_Rnd),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(HistCacheSize >= 1, "Have to hold at least the current state!");
}

THierarch::THierarch(TSIn& SIn):
		HierarchV(SIn),
		StateHeightV(SIn),
		UniqueHeightV(SIn),
		NaturalScaleV(SIn),
		MxHeight(SIn),
		HistCacheSize(TInt(SIn)),
		PastStateIdV(SIn),
		HierarchHistoryVV(SIn),
		NLeafs(TInt(SIn)),
		StateNmV(SIn),
		StateLabelV(SIn),
		TargetIdHeightSet(SIn),
		IsTransitionBased(TBool(SIn)),
		Rnd(SIn),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void THierarch::Save(TSOut& SOut) const {
	HierarchV.Save(SOut);
	StateHeightV.Save(SOut);
	UniqueHeightV.Save(SOut);
	NaturalScaleV.Save(SOut);
	MxHeight.Save(SOut);
	TInt(HistCacheSize).Save(SOut);
	PastStateIdV.Save(SOut);
	HierarchHistoryVV.Save(SOut);
	TInt(NLeafs).Save(SOut);
	StateNmV.Save(SOut);
	StateLabelV.Save(SOut);
	TargetIdHeightSet.Save(SOut);
	TBool(IsTransitionBased).Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

THierarch* THierarch::Load(TSIn& SIn) {
	return new THierarch(SIn);
}

void THierarch::Init(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV, const int& CurrLeafId,
        const TStreamStory& StreamStory) {
	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();

	TFltVV CentroidVV;	StateIdentifier.GetCentroidVV(CentroidVV);

	ClrFlds();

	NLeafs = CentroidVV.GetCols();

	if (IsTransitionBased) {
		InitHierarchyTrans(StreamStory);
	} else {
		InitHierarchyDist(StateIdentifier);
	}

	// initialize history
	GenUniqueHeightV(StateHeightV, UniqueHeightV);
	PastStateIdV.Gen(UniqueHeightV.Len(), UniqueHeightV.Len());
	UpdateHistory(CurrLeafId);

	// initialize state names
	StateNmV.Gen(HierarchV.Len());
	StateLabelV.Gen(HierarchV.Len());

	// initialize state labels
	// split the hierarchy into 5 levels
	for (int StateId = 0; StateId < NLeafs; StateId++) {
		StateLabelV[StateId] = "1." + TInt::GetStr(StateId+1);
	}

	TFltIntPrV HeightStateIdV;
	for (int StateId = NLeafs; StateId < HierarchV.Len(); StateId++) {
		HeightStateIdV.Add(TFltIntPr(StateHeightV[StateId], StateId));
	}

	HeightStateIdV.Sort(true);

	int PerLevel = (int) ceil(double(HeightStateIdV.Len()) / 5.0);
	for (int StateN = 0; StateN < HeightStateIdV.Len(); StateN++) {
		const int StateId = HeightStateIdV[StateN].Val2;
		int Level = 2 + StateN / PerLevel;

		StateLabelV[StateId] = TInt::GetStr(Level) + "." + TInt::GetStr((StateN % PerLevel) + 1);
	}

	CalcNaturalScales(StreamStory, Rnd, NaturalScaleV);

	// init the hierarchy history
	InitHistHierarch(RecTmV, ObsFtrVV, StateIdentifier);
}

void THierarch::UpdateHistory(const int& CurrLeafId) {
	const TFltV& HeightV = GetUniqueHeightV();
	TIntFltPrV StateIdHeightPrV;	GetAncestorV(CurrLeafId, StateIdHeightPrV);

	EAssertR(HeightV.Len() == PastStateIdV.Len(), "Number of heights doesn't match the number of heights in the past state cache!");

	int CurrHeightIdx = 0;
	for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
		const int CurrStateId = StateIdHeightPrV[i].Val1;

		while (CurrHeightIdx < HeightV.Len() && IsOnHeight(CurrStateId, HeightV[CurrHeightIdx])) {
			if (PastStateIdV[CurrHeightIdx].Empty() || PastStateIdV[CurrHeightIdx][0] != CurrStateId) {
				PastStateIdV[CurrHeightIdx].Ins(0, CurrStateId);
				// cleanup
				while (PastStateIdV[CurrHeightIdx].Len() > 2 /* HistCacheSize TODO */) { PastStateIdV[CurrHeightIdx].DelLast(); }
			}
			CurrHeightIdx++;
		}
	}
}

void THierarch::GetStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	const int NStates = GetStates();

	for (int StateId = 0; StateId < NStates; StateId++) {
		StateIdHeightPrV.Add(TIntFltPr(StateId, GetStateHeight(StateId)));
	}
}

void THierarch::GetStateSetsAtHeight(const double& Height, TStateIdV& StateIdV,
		TAggStateV& AggStateV) const {
	TIntIntVH StateSubStateH;	GetAncSuccH(Height, StateSubStateH);

	StateIdV.Gen(StateSubStateH.Len());
	AggStateV.Gen(StateSubStateH.Len());

	int i = 0;
	int KeyId = StateSubStateH.FFirstKeyId();
	while (StateSubStateH.FNextKeyId(KeyId)) {
		const int StateId = StateSubStateH.GetKey(KeyId);

		if (StateSubStateH[KeyId].Empty()) {
			AggStateV[i].Add(StateId);
		} else {
			AggStateV[i] = StateSubStateH[KeyId];
		}

		StateIdV[i] = StateId;

		i++;
	}
}

void THierarch::GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const {
	const int NStates = GetStates();

	for (int StateId = 0; StateId < NStates; StateId++) {
		if (IsOnHeight(StateId, Height)) {
			StateIdV.AddKey(StateId);
		}
	}
}

double THierarch::GetNextLevel(const TIntV& CurrLevelIdV, TIntV& NextLevelIdV) const {
	double NextHeight = TFlt::PInf;
	for (int StateN = 0; StateN < CurrLevelIdV.Len(); StateN++) {
		const int ParentId = GetParentId(CurrLevelIdV[StateN]);
		const double ParentHeight = GetStateHeight(ParentId);
		if (ParentHeight < NextHeight) {
			NextHeight = ParentHeight;
		}
	}

	TIntSet TakenParentIdSet;
	for (int StateN = 0; StateN < CurrLevelIdV.Len(); StateN++) {
		const int StateId = CurrLevelIdV[StateN];
		const int ParentId = GetParentId(StateId);

		if (GetStateHeight(ParentId) <= NextHeight) {
			if (!TakenParentIdSet.IsKey(ParentId)) {
				TakenParentIdSet.AddKey(ParentId);
				NextLevelIdV.Add(ParentId);
			}
		} else {
			NextLevelIdV.Add(StateId);
		}
	}

	return NextHeight;
}

double THierarch::GetNextUiLevel(const TIntV& CurrLevelIdV, TIntV& NextLevelIdV) const {
	double CurrHeight = 0;
	for (int StateN = 0; StateN < CurrLevelIdV.Len(); StateN++) {
		const int& StateId = CurrLevelIdV[StateN];
		const double& StateHeight = GetStateHeight(StateId);
		if (StateHeight > CurrHeight) {
			CurrHeight = StateHeight;
		}
	}
	const double NextHeight = GetNextUiHeight(CurrHeight);

	TIntSet TakenParentIdSet;
	for (int StateN = 0; StateN < CurrLevelIdV.Len(); StateN++) {
		const int StateId = CurrLevelIdV[StateN];
		const int ParentId = GetAncestorAtHeight(StateId, NextHeight);

		if (!TakenParentIdSet.IsKey(ParentId)) {
			TakenParentIdSet.AddKey(ParentId);
			NextLevelIdV.Add(ParentId);
		}
		else {
			NextLevelIdV.Add(StateId);
		}
	}

	return NextHeight;
}

void THierarch::GetAncestorV(const int& StateId, TIntFltPrV& StateIdHeightPrV) const {
	StateIdHeightPrV.Add(TIntFltPr(StateId, GetStateHeight(StateId)));

	int AncestorId = StateId;
	do {
		AncestorId = GetParentId(AncestorId);
		StateIdHeightPrV.Add(TIntFltPr(AncestorId, GetStateHeight(AncestorId)));
	} while (!IsRoot(AncestorId));
}

int THierarch::GetAncestorAtHeight(const int& StateId, const double& Height) const {
	EAssertR(Height <= MxHeight, "Cannot search for states at height larger than MxHeight!");
	EAssert(IsOnHeight(StateId, Height) || IsBelowHeight(StateId, Height));

	int AncestorId = StateId;

	while (!IsOnHeight(AncestorId, Height)) {
		AncestorId = GetParentId(AncestorId);
	}

	return AncestorId;
}

void THierarch::GetLeafDescendantV(const int& TargetStateId, TAggState& DescendantV) const {
	if (!DescendantV.Empty()) { DescendantV.Clr(); }

	if (IsLeaf(TargetStateId)) {
		DescendantV.Add(TargetStateId);
		return;
	}

	const int NStates = HierarchV.Len();

	TIntV TempHierarchV(HierarchV);

	// for each state compute the oldest ancestor until you hit the target state or root
	bool Change;
	do {
		Change = false;

		for (int LeafId = 0; LeafId < NStates; LeafId++) {
			if (GetParentId(LeafId, TempHierarchV) != TargetStateId && !IsRoot(GetParentId(LeafId, TempHierarchV), TempHierarchV)) {
				GetParentId(LeafId, TempHierarchV) = GetGrandparentId(LeafId, TempHierarchV);
				Change = true;
			}
		}
	} while (Change);

	// take only the leafs with the target ancestor
	for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
		if (GetParentId(LeafId, TempHierarchV) == TargetStateId) {
			DescendantV.Add(LeafId);
		}
	}
}

void THierarch::GetLeafIdV(TIntV& LeafIdV) const {
	const int NStates = GetStates();

	LeafIdV.Gen(GetLeafs(), 0);
	for (int StateId = 0; StateId < NStates; StateId++) {
		if (IsLeaf(StateId)) {
			LeafIdV.Add(StateId);
		}
	}
}

void THierarch::GetDescendantsAtHeight(const double& Height, const TIntV& StateIdV, TAggStateV& AggStateV) {
	const int NStates = StateIdV.Len();
	AggStateV.Gen(NStates, NStates);

	TIntH StateIdStateNH;
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		StateIdStateNH.AddDat(StateIdV[StateN], StateN);
	}

	TIntV TempHierarchV = HierarchV;

	if (Height <= 0) {
		bool Change;
		do {
			Change = false;
			for (int StateId = 0; StateId < TempHierarchV.Len(); StateId++) {
				const int AncestorId = TempHierarchV[StateId];

				if (!StateIdStateNH.IsKey(AncestorId) && !IsRoot(AncestorId)) {
					TempHierarchV[StateId] = TempHierarchV[TempHierarchV[StateId]];
					Change = true;
				}
			}
		} while (Change);

		for (int StateId = 0; StateId < NLeafs; StateId++) {
			if (StateIdStateNH.IsKey(StateId)) {
				const int StateN = StateIdStateNH.GetDat(StateId);
				AggStateV[StateN].Add(StateId);
			} else {
				const int AncestorId = TempHierarchV[StateId];
				EAssertR(StateIdStateNH.IsKey(AncestorId), "State " + TInt::GetStr(AncestorId) + " is not in the super states!");
				const int AncestorN = StateIdStateNH.GetDat(AncestorId);

				AggStateV[AncestorN].Add(StateId);
			}
		}
	} else {
		bool Change;
		do {
			Change = false;
			for (int StateId = 0; StateId < TempHierarchV.Len(); StateId++) {
				const int AncestorId = TempHierarchV[StateId];

				if (IsOnHeight(AncestorId, Height)) { continue; }

				if (!StateIdStateNH.IsKey(AncestorId) && !IsRoot(AncestorId)) {
					TempHierarchV[StateId] = TempHierarchV[TempHierarchV[StateId]];
					Change = true;
				}
			}
		} while (Change);

		TIntSet UsedStateIdSet;
		for (int StateId = 0; StateId < NLeafs; StateId++) {

			int AddStateN, AddStateId;

			if (StateIdStateNH.IsKey(StateId)) {
				AddStateId = StateId;
				AddStateN = StateIdStateNH.GetDat(StateId);
			} else {
				const int AncestorOnHeightId = TempHierarchV[StateId];
				if (StateIdStateNH.IsKey(AncestorOnHeightId)) {
					AddStateId = StateId;
					AddStateN = StateIdStateNH.GetDat(AncestorOnHeightId);
				} else {
					const int SuperAncestorId = TempHierarchV[AncestorOnHeightId];
					EAssert(StateIdStateNH.IsKey(SuperAncestorId));

					AddStateN = StateIdStateNH.GetDat(SuperAncestorId);
					AddStateId = AncestorOnHeightId;
				}
			}

			if (!UsedStateIdSet.IsKey(AddStateId)) {
				AggStateV[AddStateN].Add(AddStateId);
				UsedStateIdSet.AddKey(AddStateId);
			}
		}
	}
}

void THierarch::GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	const TFltV& HeightV = GetUniqueHeightV();
	for (int i = 0; i < PastStateIdV.Len(); i++) {
		const TIntV& PastStateIdVOnH = PastStateIdV[i];
		EAssertR(!PastStateIdVOnH.Empty(), "Past state cache empty!");
		StateIdHeightPrV.Add(TIntFltPr(PastStateIdVOnH[0], HeightV[i]));
	}
}

void THierarch::GetUiCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	const TFltV& HeightV = GetUiHeightV();
	const TIntV CurrLeafHistoryIdV = PastStateIdV[0];

	EAssertR(!CurrLeafHistoryIdV.Empty(), "Past state cache empty!");

	const int& CurrLeafId = CurrLeafHistoryIdV[0];

	for (int HeightN = 0; HeightN < HeightV.Len(); HeightN++) {
		const double& Height = HeightV[HeightN];
		const int StateId = GetAncestorAtHeight(CurrLeafId, Height);

		StateIdHeightPrV.Add(TIntFltPr(StateId, Height));
	}
}

void THierarch::GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const {
	const int NearestHeightIdx = GetHeightN(Height);
	const TIntV& HistV = PastStateIdV[NearestHeightIdx];
	for (int i = 1; i < HistV.Len(); i++) {
		StateIdV.Add(HistV[i]);
	}
}

void THierarch::GetStateHistory(const double& RelOffset, const double& RelRange, const int& MxStates,
        const double& Scale, uint64& GlobalMnDur,
        TStateHist& TmDurStateIdPercHTrV) const {
    // first get the time range
    EAssertR(!HierarchHistoryVV.Empty(), "No scales in history vector!!");
    const TUInt64IntPrV& LowestScaleHistV = HierarchHistoryVV[0];

    // set the minimum and maximum times
    EAssertR(LowestScaleHistV.Len() > 1, "Need at least 2 elements on the lowest scale!");
    const uint64& MnTm = LowestScaleHistV[0].Val1;
    const uint64& MxTm = LowestScaleHistV.Last().Val1;
    const uint64 TmRange = MxTm - MnTm;

    Notify->OnNotifyFmt(ntInfo, "Calculating history for scale: %.4f", Scale);

    const uint64 IntervalStartTm = (uint64) (LowestScaleHistV[0].Val1 + TmRange*RelOffset);
    const uint64 IntervalEndTm = (uint64) (IntervalStartTm + TmRange*RelRange);

    Notify->OnNotifyFmt(ntInfo, "Start time: %ld, end time: %ld", IntervalStartTm, IntervalEndTm);

    const TUInt64IntPrV& ScaleHistV = HierarchHistoryVV[GetUiScaleN(Scale)];
    TUInt64UInt64IntTrV StateTmDurIdTrV(ScaleHistV.Len()-1, 0);
    for (int TmN = 0; TmN < ScaleHistV.Len(); TmN++) {
        const TUInt64IntPr& StateTmStateIdPr = ScaleHistV[TmN];

        if (TmN < ScaleHistV.Len()-1) {
            const TUInt64IntPr& NextStateTmStateIdPr = ScaleHistV[TmN+1];

            uint64 StateStartTm = StateTmStateIdPr.Val1;
            uint64 StateEndTm = NextStateTmStateIdPr.Val1;

            const int& StateId = StateTmStateIdPr.Val2;

            if (StateEndTm < IntervalStartTm) { continue; }
            if (StateStartTm > IntervalEndTm) { break; }

            if (StateStartTm < IntervalStartTm) { StateStartTm = IntervalStartTm; }
            if (StateEndTm > IntervalEndTm) { StateEndTm = IntervalEndTm; }

            const uint64 StateDur = StateEndTm - StateStartTm;

            StateTmDurIdTrV.Add(TUInt64UInt64IntTr(StateStartTm, StateDur, StateId));
        } else {
            // this is the last state and we are still in the loop
            const uint64& StateStartTm = StateTmStateIdPr.Val1;

            if (StateStartTm > IntervalEndTm) { break; }

            const int& StateId = StateTmStateIdPr.Val2;
            const uint64 StateDur = IntervalEndTm - StateStartTm;

            StateTmDurIdTrV.Add(TUInt64UInt64IntTr(StateStartTm, StateDur, StateId));
        }
    }

    if (StateTmDurIdTrV.Len() > MxStates) {
        Notify->OnNotifyFmt(ntInfo, "Resampling %d values ...", StateTmDurIdTrV.Len());

        // find the MAX_HISTORY_SAMPLES-th largest duration
        THeap<TUInt64, TGtr<TUInt64>> DurHeap(MxStates);

        for (int TmN = 0; TmN < MxStates; TmN++) {
            const uint64& StateDur = StateTmDurIdTrV[TmN].Val2;
            DurHeap.PushHeap(StateDur);
        }
        for (int TmN = MxStates; TmN < StateTmDurIdTrV.Len(); TmN++) {
            const uint64& StateDur = StateTmDurIdTrV[TmN].Val2;
            const uint64& CurMnDur = DurHeap.TopHeap();
            if (StateDur > CurMnDur) {
                DurHeap.PushHeap(StateDur);
                DurHeap.PopHeap();
            }
        }

        const uint64& MnDur = DurHeap.TopHeap();

        if (MnDur < GlobalMnDur) {
            GlobalMnDur = MnDur;
        }

        Notify->OnNotifyFmt(ntInfo, "Skipping durations below %s ...", TStrUtil::GetHMSStrFromMSecs(GlobalMnDur).CStr());

        TmDurStateIdPercHTrV.Gen(MxStates, 0);
        for (int TmN = 0; TmN < StateTmDurIdTrV.Len(); TmN++) {
            const TUInt64UInt64IntTr& TmDurIdTr = StateTmDurIdTrV[TmN];
            const uint64& Dur = TmDurIdTr.Val2;

            if (TmN == 0) {
                TmDurStateIdPercHTrV.Add();
                TTriple<TUInt64,TUInt64,TIntFltH>& TmDurStateDistHTr = TmDurStateIdPercHTrV.Last();

                TmDurStateDistHTr.Val1 = TmDurIdTr.Val1;
                TmDurStateDistHTr.Val2 = TmDurIdTr.Val2;
                TmDurStateDistHTr.Val3.AddDat(TmDurIdTr.Val3, 1);
            }
            else {
                if (Dur < GlobalMnDur) {
                    // add the duration of the current state to the last state
                    // that will be output
                    TTriple<TUInt64,TUInt64,TIntFltH>& PrevTmDurStateDistHTr = TmDurStateIdPercHTrV.Last();
                    const uint64 PrevDur = PrevTmDurStateDistHTr.Val2;

                    const double PrevPerc = double(PrevDur) / (PrevDur + Dur);
                    const double CurrPerc = 1 - PrevPerc;

                    // redistribute the states in this block
                    TIntFltH& PrevStateDistH = PrevTmDurStateDistHTr.Val3;
                    int KeyId = PrevStateDistH.FFirstKeyId();
                    while (PrevStateDistH.FNextKeyId(KeyId)) {
                        PrevStateDistH[KeyId] *= PrevPerc;
                    }

                    if (PrevStateDistH.IsKey(TmDurIdTr.Val3)) {
                        PrevStateDistH.GetDat(TmDurIdTr.Val3) += CurrPerc;
                    } else {
                        PrevStateDistH.AddDat(TmDurIdTr.Val3, CurrPerc);
                    }

                    // increase the total duration of the block
                    PrevTmDurStateDistHTr.Val2 += Dur;
                }
                else {
                    TmDurStateIdPercHTrV.Add();
                    TTriple<TUInt64,TUInt64,TIntFltH>& TmDurStateDistHTr = TmDurStateIdPercHTrV.Last();

                    TmDurStateDistHTr.Val1 = TmDurIdTr.Val1;
                    TmDurStateDistHTr.Val2 = TmDurIdTr.Val2;
                    TmDurStateDistHTr.Val3.AddDat(TmDurIdTr.Val3, 1);
                }
            }

            // if two consecutive states are duplicate => merge them
            const int CurrN = TmDurStateIdPercHTrV.Len();
            if (CurrN >= 2 &&
                    HaveSameKeys(TmDurStateIdPercHTrV.Last().Val3, TmDurStateIdPercHTrV[CurrN-2].Val3) &&
                    GetStateProbDiff(TmDurStateIdPercHTrV.Last().Val3, TmDurStateIdPercHTrV[CurrN-2].Val3) < .07) {
                TIntFltH& Dist1 = TmDurStateIdPercHTrV[CurrN-2].Val3;
                const TIntFltH Dist2 = TmDurStateIdPercHTrV.Last().Val3;

                const double Perc1 = double(TmDurStateIdPercHTrV[CurrN-2].Val2) / (TmDurStateIdPercHTrV[CurrN-2].Val2 + TmDurStateIdPercHTrV.Last().Val2);
                const double Perc2 = 1 - Perc1;

                int KeyId = Dist1.FFirstKeyId();
                while (Dist1.FNextKeyId(KeyId)) {
                    const int& StateId = Dist1.GetKey(KeyId);

                    Dist1.GetDat(StateId) = Perc1*Dist1.GetDat(StateId) + Perc2*Dist2.GetDat(StateId);
                }

                TmDurStateIdPercHTrV[CurrN-2].Val2 += TmDurStateIdPercHTrV.Last().Val2;
                TmDurStateIdPercHTrV.DelLast();
            }
        }

        Notify->OnNotifyFmt(ntInfo, "%d historical states returned ...", TmDurStateIdPercHTrV.Len());
    } else {
        TmDurStateIdPercHTrV.Gen(StateTmDurIdTrV.Len());
        for (int TmN = 0; TmN < StateTmDurIdTrV.Len(); TmN++) {
            const TUInt64UInt64IntTr& InTmDurIdTr = StateTmDurIdTrV[TmN];
            TTriple<TUInt64,TUInt64,TIntFltH>& TmDurStatePercHTr = TmDurStateIdPercHTrV[TmN];

            TmDurStatePercHTr.Val1 = InTmDurIdTr.Val1;
            TmDurStatePercHTr.Val2 = InTmDurIdTr.Val2;
            TmDurStatePercHTr.Val3.AddDat(InTmDurIdTr.Val3, 1);
        }
    }
}

void THierarch::GetStateHistory(const double& RelOffset, const double& RelRange,
        const int& MxStates,
        TScaleStateHistV& ScaleTmDurIdDistPrTrV,
        uint64& MnTm, uint64& MxTm) const {
    const TFltV& ScaleV = GetUiHeightV();

    Notify->OnNotifyFmt(ntInfo, "Fetching history for %d all scales ...", ScaleV.Len());

    ScaleTmDurIdDistPrTrV.Gen(ScaleV.Len());

    // set the minimum and maximum times
    const TUInt64IntPrV& LowestScaleHistV = HierarchHistoryVV[0];
    EAssertR(LowestScaleHistV.Len() > 1, "Need at least 2 elements on the lowest scale!");
    MnTm = LowestScaleHistV[0].Val1;
    MxTm = LowestScaleHistV.Last().Val1;

    // get the the history blocks
    uint64 MnDur = TUInt64::Mx;
    for (int ScaleN = 0; ScaleN < ScaleV.Len(); ScaleN++) {
        const double& Scale = ScaleV[ScaleN];

        ScaleTmDurIdDistPrTrV[ScaleN].Val1 = Scale;
        GetStateHistory(
            RelOffset,
            RelRange,
            MxStates,
            Scale,
            MnDur,
            ScaleTmDurIdDistPrTrV[ScaleN].Val2
        );
    }

    Notify->OnNotify(ntInfo, "Done!");
}

void THierarch::GetLeafSuccesorCountV(TIntV& LeafCountV) const {
	const int NStates = GetStates();

	LeafCountV.Gen(NStates, NStates);

	for (int i = 0; i < NLeafs; i++) {
		LeafCountV[i] = 1;
	}

	TIntV TempHierarchV(HierarchV);

	bool Change;
	do {
		Change = false;

		for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
			const int AncestorId = GetParentId(LeafId, TempHierarchV);
			const int LeafWeight = LeafCountV[LeafId];
			LeafCountV[AncestorId] += LeafWeight;
		}

		for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
			// check if the parent is root
			if (!IsRoot(GetParentId(LeafId, TempHierarchV), TempHierarchV)) {
				GetParentId(LeafId, TempHierarchV) = GetGrandparentId(LeafId, TempHierarchV);
				Change = true;
			}
		}
	} while (Change);
}

bool THierarch::IsStateNm(const int& StateId) const {
	return 0 <= StateId && StateId < HierarchV.Len() && !StateNmV[StateId].Empty();
}

void THierarch::SetStateNm(const int& StateId, const TStr& StateNm) {
	EAssertR(0 <= StateId && StateId < StateNmV.Len(), "THierarch::SetStateNm: Invalid state ID!");
	StateNmV[StateId] = StateNm;
}

const TStr& THierarch::GetStateNm(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < StateNmV.Len(), "THierarch::GetStateNm: Invalid state ID!");
	return StateNmV[StateId];
}

const TStr& THierarch::GetStateLabel(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < StateLabelV.Len(), "THierarch::GetStateLabel: Invalid state ID!");
	return StateLabelV[StateId];
}

bool THierarch::IsTarget(const int& StateId) const {
	EAssert(IsStateId(StateId));
	double StateHeight = GetStateHeight(StateId);
	return TargetIdHeightSet.IsKey(TIntFltPr(StateId, StateHeight));
}

bool THierarch::HasTargetStates() const {
	return !TargetIdHeightSet.Empty();
}

void THierarch::SetTarget(const int& StateId) {
	EAssert(IsStateId(StateId));
	double StateHeight = GetStateHeight(StateId);
	TargetIdHeightSet.AddKey(TIntFltPr(StateId, StateHeight));
}

void THierarch::RemoveTarget(const int& StateId) {
	EAssert(IsTarget(StateId));

	double StateHeight = GetStateHeight(StateId);
	TIntFltPr StateIdHeightPr(StateId, StateHeight);

	TargetIdHeightSet.DelKey(StateIdHeightPr);
}

bool THierarch::IsLeaf(const int& StateId) const {
	return StateId < NLeafs;
}

void THierarch::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

void THierarch::PrintHierarch() const {
	TChA ChA = "";

	for (int i = 0; i < HierarchV.Len(); i++) {
		ChA += "(" + TInt(i).GetStr() + "," + TInt::GetStr(GetParentId(i)) + "," + TFlt::GetStr(GetStateHeight(i), "%.3f") + ")";
		if (i < HierarchV.Len()-1) { ChA += ","; }
	}

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Hierarchy: %s", ChA.CStr());
}

void THierarch::InitHierarchyDist(const TStateIdentifier& StateIdentifier) {
	const TFltVV& CentroidVV = StateIdentifier.GetRawCentroidVV();

	// create a hierarchy
	TIntIntFltTrV MergeV;	TAlAggClust::MakeDendro(CentroidVV, MergeV, Notify);

	Notify->OnNotify(TNotifyType::ntInfo, TStrUtil::GetStr(MergeV, ", "));

	const int NMiddleStates = MergeV.Len();
	const int NStates = NLeafs + NMiddleStates;

	HierarchV.Gen(NStates);
	StateHeightV.Gen(NStates);

	for (int i = 0; i < HierarchV.Len(); i++) {
		HierarchV[i] = -1;
	}

	for (int i = 0; i < MergeV.Len(); i++) {
		const int LeafState1Idx = MergeV[i].Val1;
		const int LeafState2Idx = MergeV[i].Val2;
		const double Height = MergeV[i].Val3;

		// find the states into which state 1 and state 2 were merged
		const int State1Idx = GetOldestAncestIdx(LeafState1Idx);
		const int State2Idx = GetOldestAncestIdx(LeafState2Idx);
		const int MergeStateIdx = NLeafs + i;

		HierarchV[State1Idx] = MergeStateIdx;
		HierarchV[State2Idx] = MergeStateIdx;
		StateHeightV[MergeStateIdx] = Height;

		EAssertR(StateHeightV[MergeStateIdx] > StateHeightV[State1Idx] && StateHeightV[MergeStateIdx] > StateHeightV[State2Idx], "Parent should have greater height that any of its children!");

		if (Height > MxHeight) { MxHeight = Height; }
	}

	HierarchV.Last() = HierarchV.Len() - 1;
}

void THierarch::InitHierarchyTrans(const TStreamStory& StreamStory) {

	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const TCtmcModeller& MChain = StreamStory.GetTransitionModeler();

	TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StreamStory, StateFtrVV);

	TAggStateV StateSetV;
	for (int StateId = 0; StateId < StateIdentifier.GetStates(); StateId++) {
		StateSetV.Add(TAggState());
		StateSetV[StateId].Add(StateId);
	}

	TFltVV QMat;	MChain.GetQMatrix(StateSetV, StateFtrVV, QMat);

	TCtMChain::Partition(QMat, HierarchV, StateHeightV);

	// reconstruct the hierarchy so that the height always grows
	const int NLeafs = QMat.GetCols();
	MxHeight = TFlt::NInf;

	TIntIntH OldToNewIdH;
	TIntV NewHierarchV;
	TFltV NewHeightV;
	for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
		NewHierarchV.Add(LeafId);
		NewHeightV.Add(0);
		OldToNewIdH.AddDat(LeafId, LeafId);
	}

	int CurrStateId = NewHierarchV.Len();
	for (int LeafId = 0; LeafId < NLeafs; LeafId++) {
		int StateId = LeafId;

		while (StateHeightV[StateId] < StateHeightV[HierarchV[StateId]]) {
			int ChildId = StateId;
			int ParentId = HierarchV[StateId];

			int NewChildId = OldToNewIdH.GetDat(ChildId);
			int NewParentId;

			if (!OldToNewIdH.IsKey(ParentId)) {
				NewParentId = CurrStateId++;
				NewHierarchV.Add(NewParentId);	// the parent points to itself (is a root)
				NewHeightV.Add(StateHeightV[ParentId]);
				OldToNewIdH.AddDat(ParentId, NewParentId);
			} else {
				NewParentId = OldToNewIdH.GetDat(ParentId);
			}

			NewHierarchV[NewChildId] = NewParentId;

			if (StateHeightV[ParentId] > MxHeight) {
				MxHeight = StateHeightV[ParentId];
			}

			StateId = ParentId;
		}
	}

	// add a dummy state to confirm with the distance based hierarchy
	MxHeight = MxHeight+1;
	int RootId = NewHierarchV.Len();

	for (int StateId = 0; StateId < NewHierarchV.Len(); StateId++) {
		if (NewHierarchV[StateId] == StateId) {
			NewHierarchV[StateId] = RootId;
		}
	}
	NewHierarchV.Add(RootId);
	NewHeightV.Add(MxHeight);

	printf("Old hierarch:\n%s\n", TStrUtil::GetStr(HierarchV).CStr());
	printf("New hierarch:\n%s\n", TStrUtil::GetStr(NewHierarchV).CStr());
	printf("Old HeightV:\n%s\n", TStrUtil::GetStr(StateHeightV).CStr());
	printf("New HeightV:\n%s\n", TStrUtil::GetStr(NewHeightV).CStr());

	// finish
	StateHeightV = NewHeightV;
	HierarchV = NewHierarchV;
}

int THierarch::GetParentId(const int& StateId) const {
	return GetParentId(StateId, HierarchV);
}

int THierarch::GetHeightN(const double& Height) const {
    const TFltV& HeightV = GetUniqueHeightV();
    return FindScaleNBin(Height, HeightV);
}

int THierarch::GetUiScaleN(const double& Scale) const {
    const TFltV& ScaleV = GetUiHeightV();
    return FindScaleNBin(Scale, ScaleV);
}

double THierarch::GetNearestHeight(const double& InHeight) const {
	return UniqueHeightV[GetHeightN(InHeight)];
}

bool THierarch::IsRoot(const int& StateId) const {
	return IsRoot(StateId, HierarchV);
}

bool THierarch::IsOnHeight(const int& StateId, const double& Height) const {
	if (IsRoot(StateId) && Height >= GetStateHeight(StateId)/*MxHeight*/) { return true; }

	const double StateHeight = GetStateHeight(StateId);
	const double ParentHeight = GetStateHeight(GetParentId(StateId));

	return StateHeight <= Height && Height < ParentHeight;
}

bool THierarch::IsBelowHeight(const int& StateId, const double& Height) const {
	if (IsOnHeight(StateId, Height)) { return false; }
	return GetStateHeight(GetParentId(StateId)) <= Height;
}

bool THierarch::IsAboveHeight(const int& StateId, const double& Height) const {
	return !IsOnHeight(StateId, Height) && !IsBelowHeight(StateId, Height);
}

void THierarch::GetAncSuccH(const double& Height, TIntIntVH& StateSubStateH) const {
	const int NStates = GetStates();

	// build a model on this height
	// get all the states on this height
	TIntSet StateSet;	GetStatesAtHeight(Height, StateSet);

	int KeyId = StateSet.FFirstKeyId();
	while (StateSet.FNextKeyId(KeyId)) {
		StateSubStateH.AddDat(StateSet.GetKey(KeyId), TIntV());
	}

	// get all the substates of the states on this height
	TIntV TempHierarchV(HierarchV);
	bool Change;
	do {
		Change = false;
		for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
			if (!StateSet.IsKey(TempHierarchV[StateIdx]) &&
					TempHierarchV[StateIdx] != TempHierarchV[TempHierarchV[StateIdx]]) {
				TempHierarchV[StateIdx] = TempHierarchV[TempHierarchV[StateIdx]];
				Change = true;
			}
		}
	} while (Change);

	for (int StateIdx = 0; StateIdx < NLeafs; StateIdx++) {
		if (StateSet.IsKey(TempHierarchV[StateIdx])) {
			StateSubStateH.GetDat(TempHierarchV[StateIdx]).Add(StateIdx);
		}
	}
}

int THierarch::GetOldestAncestIdx(const int& StateIdx) const {
	int AncestIdx = StateIdx;

	while (HierarchV[AncestIdx] != -1) {
		AncestIdx = HierarchV[AncestIdx];
	}

	return AncestIdx;
}

void THierarch::CalcNaturalScales(const TStreamStory& StreamStory, const TRnd& Rnd, TFltV& ScaleV) const {
	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const TCtmcModeller& MChain = StreamStory.GetTransitionModeler();

	const TFltV& HeightV = UniqueHeightV;
	const int TotalScales = HeightV.Len()-1;

	TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StreamStory, StateFtrVV);

	TVec<TPair<TFlt,TFltVV>> ScaleQMatPrV(TotalScales);

	for (int HeightN = 0; HeightN < TotalScales; HeightN++) {
		const double& Height = HeightV[HeightN];
		printf("Unique height: %.5f\n", Height);
		// get the intensity matrix on this height
		TIntV StateIdV; TAggStateV AggStateV;
		GetStateSetsAtHeight(Height, StateIdV, AggStateV);

		ScaleQMatPrV[HeightN].Val1 = Height;
		MChain.GetQMatrix(AggStateV, StateFtrVV, ScaleQMatPrV[HeightN].Val2);
	}

	TEigValScaleHelper ScaleHelper(Notify);
	ScaleHelper.CalcNaturalScales(ScaleQMatPrV, Rnd, ScaleV);

	ScaleV.Sort(true);
}

void THierarch::InitHistHierarch(const TUInt64V& RecTmV, const TFltVV& FtrVV,
            const TStateIdentifier& StateIdentifier) {
    const TFltV& ScaleV = GetUiHeightV();
    TIntV AssignV;  StateIdentifier.Assign(RecTmV, FtrVV, AssignV);

    const int NScales = ScaleV.Len();
    const int NRecs = RecTmV.Len();

    HierarchHistoryVV.Gen(NScales);

    // fill the first entry in history
    {
        EAssertR(!AssignV.Empty(), "No records found when constructing history!");
        EAssertR(!RecTmV.Empty(), "Time vector empty when constructing history!");

        const int& LeafId = AssignV[0];
        const uint64 RecTm = RecTmV[0];
        for (int ScaleN = 0; ScaleN < NScales; ScaleN++) {
            const double& Scale = ScaleV[ScaleN];
            const int& AncestorId = GetAncestorAtHeight(LeafId, Scale);

            HierarchHistoryVV[ScaleN].Add(TUInt64IntPr(RecTm, AncestorId));
        }
    }

    for (int RecN = 1; RecN < NRecs; RecN++) {
        const int& LeafId = AssignV[RecN];
        const uint64& RecTm = RecTmV[RecN];

        for (int ScaleN = 0; ScaleN < NScales; ScaleN++) {
            const double& Scale = ScaleV[ScaleN];
            const int& AncestorId = GetAncestorAtHeight(LeafId, Scale);

            const int& CurrHistStateId = HierarchHistoryVV[ScaleN].Last().Val2;

            // check if we need to update the history
            if (CurrHistStateId == AncestorId) { break; }

            HierarchHistoryVV[ScaleN].Add(TUInt64IntPr(RecTm, AncestorId));
        }
    }
}

bool THierarch::IsRoot(const int& StateId, const TIntV& HierarchV) {
	return GetParentId(StateId, HierarchV) == StateId;
}

void THierarch::GenUniqueHeightV(const TFltV& HeightV, TFltV& UniqueHeightV) {
	TFltSet UsedHeightSet;
	for (int i = 0; i < HeightV.Len(); i++) {
		if (!UsedHeightSet.IsKey(HeightV[i])) {
			UniqueHeightV.Add(HeightV[i]);
			UsedHeightSet.AddKey(HeightV[i]);
		}
	}

	UniqueHeightV.Sort(true);	// sort ascending
}

int THierarch::FindScaleNBin(const double& Scale, const TFltV& ScaleV) {
    EAssertR(!ScaleV.Empty(), "Cannot search scales in an empty vector!");

    int StartN = 0;
    int EndN = ScaleV.Len()-1;

    while (StartN < EndN) {
        const int MiddleN = (StartN + EndN) / 2;

        if (Scale < ScaleV[MiddleN]) {
            EndN = MiddleN;
        }
        else if (ScaleV[MiddleN+1] <= Scale) {    // ScaleV[Middle] <= Scale
            StartN = MiddleN+1;
        }
        else {  // ScaleV[Middle] <= Scale && ScaleV[Middle+1] > Scale
            return MiddleN;
        }
    }

    // StartN == EndN
    return StartN;
}

void THierarch::ClrFlds() {
	HierarchV.Clr();
	StateHeightV.Clr();
	UniqueHeightV.Clr();
	MxHeight = TFlt::Mn;
	HistCacheSize = 1;
	PastStateIdV.Clr();
	NLeafs = 0;
	StateNmV.Clr();
	TargetIdHeightSet.Clr();
}

/////////////////////////////////////////////////////////////////
// UI helper
TUiHelper::TAutoNmDesc::TAutoNmDesc(const int& _FtrN, const double& _PVal):
		FtrN(_FtrN),
		PVal(_PVal) {}

TUiHelper::TAutoNmDesc::TAutoNmDesc(TSIn& SIn):
		FtrN(TInt(SIn)),
		PVal(TFlt(SIn)) {}

TUiHelper::PAutoNmDesc TUiHelper::TAutoNmDesc::Load(TSIn& SIn) {
	const int TypeInt = TInt(SIn);
	const TFtrType Type = (TFtrType) TypeInt;

	switch (Type) {
	case ftNumeric: {
		return new TNumAutoNmDesc(SIn);
	}
	case ftCategorical: {
		return new TCatAutoNmDesc(SIn);
	}
	case ftTime: {
	    return new TAutoNmTmDesc(SIn);
	}
	default:
		throw TExcept::New("Unknown feature type: " + TInt::GetStr((int) Type));
	}
}

void TUiHelper::TAutoNmDesc::Save(TSOut& SOut) const {
	SOut.Save((int) GetFtrType());
	SOut.Save(FtrN);
	SOut.Save(PVal);
}

TUiHelper::TNumAutoNmDesc::TNumAutoNmDesc(const int& FtrN, const double& PVal,
			const TNumAutoNmLevel& _Level):
		TAutoNmDesc(FtrN, PVal),
		Level(_Level) {}

TUiHelper::TNumAutoNmDesc::TNumAutoNmDesc(TSIn& SIn):
		TAutoNmDesc(SIn),
		Level((TNumAutoNmLevel) TInt(SIn).Val) {}

void TUiHelper::TNumAutoNmDesc::Save(TSOut& SOut) const {
	TAutoNmDesc::Save(SOut);
	SOut.Save((int) Level);
}

PJsonVal TUiHelper::TNumAutoNmDesc::GetJson() const {
	PJsonVal Result = TJsonVal::NewObj();

	Result->AddToObj("ftrId", GetFtrId());
	Result->AddToObj("type", "numeric");

	switch (GetLevel()) {
	case TUiHelper::TNumAutoNmLevel::nanlLowest: {
		Result->AddToObj("range", "LOWEST");
		break;
	}
	case TUiHelper::TNumAutoNmLevel::nanlLow: {
		Result->AddToObj("range", "LOW");
		break;
	}
	case TUiHelper::TNumAutoNmLevel::nanlMeduim: {
		// nothing
		break;
	}
	case TUiHelper::TNumAutoNmLevel::nanlHigh: {
		Result->AddToObj("range", "HIGH");
		break;
	}
	case TUiHelper::TNumAutoNmLevel::nanlHighest: {
		Result->AddToObj("range", "HIGHEST");
		break;
	}
	default:
		throw TExcept::New("Unknown level " + TInt(GetLevel()).GetStr());
	}

	return Result;
}

PJsonVal TUiHelper::TNumAutoNmDesc::GetNarrateJson() const {
	PJsonVal DescJson = TJsonVal::NewObj();
	DescJson->AddToObj("p", GetPVal());
	DescJson->AddToObj("type", "numeric");
	DescJson->AddToObj("ftrId", GetFtrId());
	DescJson->AddToObj("ftrDesc", GetAutoNmLowHighDesc());
	return DescJson;
}

TStr TUiHelper::TNumAutoNmDesc::GetAutoNmLowHighDesc() const {
	if (Level == TNumAutoNmLevel::nanlHigh || Level == TNumAutoNmLevel::nanlHighest) {
		return "high";
	} else if (Level == TNumAutoNmLevel::nanlLow || Level == TNumAutoNmLevel::nanlLowest) {
		return "low";
	} else if (Level == TNumAutoNmLevel::nanlMeduim) {
		return "mean";
	} else {
		throw TExcept::New("Unknown level: " + TStr(Level));
	}
}

TUiHelper::TNumAutoNmLevel TUiHelper::TNumAutoNmDesc::GetAutoNmLevel(const double& PVal,
		const double& LowPercPVal, const double& HighPercPVal) {
	if (PVal < LOWEST_PVAL_THRESHOLD) {
		return LowPercPVal < HighPercPVal ? TNumAutoNmLevel::nanlLowest : TNumAutoNmLevel::nanlHighest;
	}
	else if (PVal < LOW_PVAL_THRESHOLD) {
		return LowPercPVal < HighPercPVal ? TNumAutoNmLevel::nanlLow : TNumAutoNmLevel::nanlHigh;
	}
	else {
		return TNumAutoNmLevel::nanlMeduim;
	}
}

TUiHelper::TCatAutoNmDesc::TCatAutoNmDesc(const int& FtrN, const double& PVal, const int& _BinN):
		TAutoNmDesc(FtrN, PVal),
		BinN(_BinN) {}

TUiHelper::TCatAutoNmDesc::TCatAutoNmDesc(TSIn& SIn):
		TAutoNmDesc(SIn),
		BinN(TInt(SIn)) {}

void TUiHelper::TCatAutoNmDesc::Save(TSOut& SOut) const {
	TAutoNmDesc::Save(SOut);
	SOut.Save(BinN);
}

PJsonVal TUiHelper::TCatAutoNmDesc::GetJson() const {
	PJsonVal Result = TJsonVal::NewObj();

	Result->AddToObj("ftrId", GetFtrId());
	Result->AddToObj("type", "categorical");
	Result->AddToObj("value", GetBin());

	return Result;
}

PJsonVal TUiHelper::TCatAutoNmDesc::GetNarrateJson() const {
	PJsonVal Result = TJsonVal::NewObj();

	Result->AddToObj("p", GetPVal());
	Result->AddToObj("type", "categorical");
	Result->AddToObj("ftrId", GetFtrId());
	Result->AddToObj("bin", GetBin());

	return Result;
}

TUiHelper::TAutoNmTmDesc::TAutoNmTmDesc(const TTmDesc& TmDesc):
        TAutoNmDesc(-1, 0), // TODO hacked by setting p-val to zero!!!
        FromToStrPr() {
    GetFromToStrPr(TmDesc, FromToStrPr, dtShort);
}

TUiHelper::TAutoNmTmDesc::TAutoNmTmDesc(TSIn& SIn):
        TAutoNmDesc(SIn),
        FromToStrPr(SIn) {}

void TUiHelper::TAutoNmTmDesc::Save(TSOut& SOut) const {
    TAutoNmDesc::Save(SOut);
    FromToStrPr.Save(SOut);
}

PJsonVal TUiHelper::TAutoNmTmDesc::GetJson() const {
    PJsonVal Result = TJsonVal::NewObj();

    Result->AddToObj("type", "time");

    if (FromToStrPr.Val1 != FromToStrPr.Val2) {
        Result->AddToObj("from", FromToStrPr.Val1);
        Result->AddToObj("to", FromToStrPr.Val2);
    } else {
        Result->AddToObj("on", FromToStrPr.Val1);
    }

    return Result;
}

PJsonVal TUiHelper::TAutoNmTmDesc::GetNarrateJson() const {
    PJsonVal Result = TJsonVal::NewObj();

    Result->AddToObj("type", "time");

    return Result;
}

/////////////////////////////////////////////////////////////////
// UI helper
const double TUiHelper::RADIUS_FACTOR = 1.8;
const double TUiHelper::STEP_FACTOR = 1e-2;
const double TUiHelper::INIT_RADIUS_FACTOR = 1.1;
const double TUiHelper::STATE_OCCUPANCY_PERC = .5;	// too much, but let's see

const double TUiHelper::LOW_PVAL_THRESHOLD = .25;
const double TUiHelper::LOWEST_PVAL_THRESHOLD = .125;
const double TUiHelper::STATE_LOW_PVAL_THRESHOLD = .4;

const TStr TUiHelper::MONTHS[] = {
		"January",
		"February",
		"March",
		"April",
		"May",
		"June",
		"July",
		"August",
		"September",
		"October",
		"November",
		"December"
};

const TStr TUiHelper::MONTHS_SHORT[] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec"
};

const TStr TUiHelper::DAYS_IN_MONTH[] = {
		"1st",
		"2nd",
		"3rd",
		"4th",
		"5th",
		"6th",
		"7th",
		"8th",
		"9th",
		"10th",
		"11th",
		"12th",
		"13th",
		"14th",
		"15th",
		"16th",
		"17th",
		"18th",
		"19th",
		"20th",
		"21st",
		"22nd",
		"23rd",
		"24th",
		"25th",
		"26th",
		"27th",
		"28th",
		"29th",
		"30th",
		"31th"
};

const TStr TUiHelper::HOURS_IN_DAY[] = {
		"Midnight",
		"1AM",
		"2AM",
		"3AM",
		"4AM",
		"5AM",
		"6AM",
		"7AM",
		"8AM",
		"9AM",
		"10AM",
		"11AM",
		"Noon",
		"1PM",
		"2PM",
		"3PM",
		"4PM",
		"5PM",
		"6PM",
		"7PM",
		"8PM",
		"9PM",
		"10PM",
		"11PM"
};

const TStr TUiHelper::DAYS_IN_WEEK[] = {
		"Monday",
		"Tuesday",
		"Wednesday",
		"Thursday",
		"Friday",
		"Saturday",
		"Sunday"
};

const TStr TUiHelper::DAYS_IN_WEEK_SHORT[] = {
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat",
		"Sun"
};

TUiHelper::TUiHelper(const TRnd& _Rnd, const bool& _Verbose):
		StateCoordV(),
		StateAutoNmV(),
		StateIdAutoNmDescVV(),
		StateIdOccTmDescV(),
		Rnd(_Rnd),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TUiHelper::TUiHelper(TSIn& SIn):
		StateCoordV(SIn),
		StateAutoNmV(SIn),
		StateIdAutoNmDescVV(SIn),
		StateIdOccTmDescV(SIn),
		Rnd(SIn),
		Verbose(TBool(SIn)) {
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TUiHelper::Save(TSOut& SOut) const {
	StateCoordV.Save(SOut);
	StateAutoNmV.Save(SOut);
	StateIdAutoNmDescVV.Save(SOut);
	StateIdOccTmDescV.Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TUiHelper::Init(const TStreamStory& StreamStory) {
	Notify->OnNotify(TNotifyType::ntInfo, "Initializing UI helper ...");
	InitStateCoordV(StreamStory);
	RefineStateCoordV(StreamStory);
	InitAutoNmV(StreamStory);
	InitStateExplain(StreamStory);
	RefineAutoNmV();
}

const TFltPr& TUiHelper::GetStateCoords(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < StateCoordV.Len(), TStr::Fmt("Invalid state id: %d", StateId));
	return StateCoordV[StateId];
}

void TUiHelper::SetStateCoords(const int& StateId, const double& x, const double& y) {
	EAssertR(0 <= StateId && StateId < StateCoordV.Len(), TStr::Fmt("Invalid state id: %d", StateId));
	StateCoordV[StateId].Val1 = x;
	StateCoordV[StateId].Val2 = y;
}

void TUiHelper::SetStateCoords(const TFltPrV& CoordV) {
	for (int StateId = 0; StateId < CoordV.Len(); StateId++) {
		SetStateCoords(StateId, CoordV[StateId].Val1, CoordV[StateId].Val2);
	}
}

void TUiHelper::GetStateRadiusV(const TFltV& ProbV, TFltV& RadiusV) const {
	RadiusV.Gen(ProbV.Len());
	for (int i = 0; i < ProbV.Len(); i++) {
		RadiusV[i] = GetStateRaduis(ProbV[i]);// / (Height + .1);
	}
}

const TUiHelper::PAutoNmDesc& TUiHelper::GetStateAutoNm(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < StateAutoNmV.Len(), "THierarch::GetStateAutoNm: Invalid state ID!");
	return StateAutoNmV[StateId];
}

void TUiHelper::SetStateAutoNm(const int& StateId, const TUiHelper::PAutoNmDesc& Desc) {
	EAssertR(0 <= StateId && StateId < StateAutoNmV.Len(), "THierarch::GetStateAutoNm: Invalid state ID!");
	StateAutoNmV[StateId] = Desc;
}

void TUiHelper::GetAutoNmPValDesc(const int& StateId, PAutoNmDescV& DescV) const {
	EAssert(0 <= StateId && StateId < StateIdAutoNmDescVV.Len());
	if (!DescV.Empty()) { DescV.Clr(); }

	const PAutoNmDescV& AutoNmDescV = StateIdAutoNmDescVV[StateId];

	for (int DescN = 0; DescN < AutoNmDescV.Len(); DescN++) {
		const PAutoNmDesc& Desc = AutoNmDescV[DescN];

		switch (Desc->GetFtrType()) {
		case ftNumeric: {
			const TNumAutoNmDesc* NumDesc = (TNumAutoNmDesc*) Desc();
			if (NumDesc->GetLevel() == TNumAutoNmLevel::nanlMeduim) { continue; }

			DescV.Add(Desc);
			break;
		}
		case ftCategorical: {
			DescV.Add(Desc);
			break;
		}
		case ftTime: {
		    DescV.Add(Desc);
		    break;
		}
		default: {
			throw TExcept::New("Unknown feature type: " + TInt::GetStr((int) Desc->GetFtrType()));
		}
		}
	}
}

void TUiHelper::GetTmDesc(const int& StateId, TStrPrV& DescIntervalV) const {
	TTmDescV TmDescV;	GetTmDesc(StateId, TmDescV);

	DescIntervalV.Gen(TmDescV.Len());

	for (int DescN = 0; DescN < TmDescV.Len(); DescN++) {
		const TTmDesc& Desc = TmDescV[DescN];

		GetFromToStrPr(Desc, DescIntervalV[DescN], dtNormal);
	}
}

TFltPr& TUiHelper::GetModStateCoords(const int& StateId) {
	EAssertR(0 <= StateId && StateId < StateCoordV.Len(), TStr::Fmt("Invalid state id: %d", StateId));
	return StateCoordV[StateId];
}

void TUiHelper::InitStateCoordV(const TStreamStory& StreamStory) {

	Notify->OnNotify(TNotifyType::ntInfo, "Computing initial state coordinates ...");

	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const THierarch& Hierarch = StreamStory.GetHierarch();
	const TCtmcModeller& MChain = StreamStory.GetTransitionModeler();

	const int NStates = Hierarch.GetStates();
	const int NLeafs = Hierarch.GetLeafs();
	TFltVV CentroidVV;	StateIdentifier.GetCentroidVV(CentroidVV);

	StateCoordV.Gen(NStates, NStates);

	TFltVV CoordMat;	TEuclMds::Project(CentroidVV, CoordMat, 2);
	for (int ColN = 0; ColN < CoordMat.GetCols(); ColN++) {
		StateCoordV[ColN].Val1 = CoordMat(0, ColN);
		StateCoordV[ColN].Val2 = CoordMat(1, ColN);
	}

	// scale the state positions so that the states occupy 20% of the screen
	// construct state sets
	TAggStateV StateSetV; TIntV StateIdV;
	Hierarch.GetStateSetsAtHeight(Hierarch.GetMinHeight(), StateIdV, StateSetV);

	TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StreamStory, StateFtrVV);
	TFltV ProbV;	MChain.GetStatDist(StateSetV, StateFtrVV, ProbV);
	TFltV RadiusV;	GetStateRadiusV(ProbV, RadiusV);

	for (int StateN = 0; StateN < NLeafs; StateN++) {
		RadiusV[StateN] *= INIT_RADIUS_FACTOR;
	}

	double MnX = TFlt::Mx;
	double MxX = TFlt::Mn;
	double MnY = TFlt::Mx;
	double MxY = TFlt::Mn;

	double TotalStateArea = 0;

	for (int StateN = 0; StateN < NLeafs; StateN++) {
		const TFltPr& StatePos = StateCoordV[StateN];
		const double& Radius = RadiusV[StateN];

		if (StatePos.Val1 + Radius > MxX) { MxX = StatePos.Val1 + Radius; }
		if (StatePos.Val1 - Radius < MnX) { MnX = StatePos.Val1 - Radius; }
		if (StatePos.Val2 + Radius > MxY) { MxY = StatePos.Val2 + Radius; }
		if (StatePos.Val2 - Radius < MnY) { MnY = StatePos.Val2 - Radius; }

		TotalStateArea += TMath::Pi*Radius*Radius;
	}

	const double ScreenArea = (MxX - MnX) * (MxY - MnY);
	const double DesiredArea = TotalStateArea / STATE_OCCUPANCY_PERC;
	// instead of shrinking the screen, disperse the states
	const double ScaleFactor = DesiredArea / ScreenArea;
	const double ScaleFactorX = TMath::Sqrt(ScaleFactor);
	const double ScaleFactorY = ScaleFactorX * (MxX - MnX) / (MxY - MnY);

	Notify->OnNotifyFmt(ntInfo, "Scaling state positions by %.4f", ScaleFactor);

	const double CenterX = (MxX - MnX) / 2;
	const double CenterY = (MxY - MnY) / 2;

	for (int StateN = 0; StateN < NLeafs; StateN++) {
		TFltPr& StatePos = StateCoordV[StateN];

		StatePos.Val1 = CenterX + (StatePos.Val1 - CenterX) * ScaleFactorX;
		StatePos.Val2 = CenterY + (StatePos.Val2 - CenterY) * ScaleFactorY;
	}

	// first find out how many ancestors each state has, so you can weight
	// the childs coordinates appropriately
	TIntV SuccesorCountV;	Hierarch.GetLeafSuccesorCountV(SuccesorCountV);

	TIntV TempHierarchV(Hierarch.GetHierarchV());

	bool Change;
	do {
		Change = false;

		for (int i = 0; i < NLeafs; i++) {
			const int AncestorIdx = TempHierarchV[i];
			const int AncestorSize = SuccesorCountV[AncestorIdx];
			StateCoordV[AncestorIdx].Val1 += StateCoordV[i].Val1 / AncestorSize;
			StateCoordV[AncestorIdx].Val2 += StateCoordV[i].Val2 / AncestorSize;
		}

		for (int i = 0; i < NLeafs; i++) {
			if (TempHierarchV[i] != TempHierarchV[TempHierarchV[i]]) {
				TempHierarchV[i] = TempHierarchV[TempHierarchV[i]];
				Change = true;
			}
		}
	} while (Change);
}

void TUiHelper::RefineStateCoordV(const TStreamStory& StreamStory) {
	Notify->OnNotify(TNotifyType::ntInfo, "Refining node positions ...");

	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const THierarch& Hierarch = StreamStory.GetHierarch();
	const TCtmcModeller& MChain = StreamStory.GetTransitionModeler();

	const TFltV& UniqueHeightV = Hierarch.GetUniqueHeightV();

	TFltPr MoveDir;
	TIntV ShuffleV;

	bool Change;
	int k = 0;
	do {
		Change = false;
		k++;

		for (int HeightN = 0; HeightN < UniqueHeightV.Len()-1; HeightN++) {
			const double CurrHeight = UniqueHeightV[HeightN];

			// construct state sets
			TAggStateV StateSetV; TIntV StateIdV;
			Hierarch.GetStateSetsAtHeight(CurrHeight, StateIdV, StateSetV);

			TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StreamStory, StateFtrVV);
			TFltV ProbV;	MChain.GetStatDist(StateSetV, StateFtrVV, ProbV);
			TFltV RadiusV;	GetStateRadiusV(ProbV, RadiusV);

			EAssertR(TFlt::Abs(1 - TLinAlg::SumVec(ProbV)) < 1e-3, "Got a bad static distribution that sums to " + TFlt(TLinAlg::SumVec(ProbV)).GetStr());

			// increase the radius for visualization purposes
			for (int i = 0; i < RadiusV.Len(); i++) {
				RadiusV[i] *= INIT_RADIUS_FACTOR;
			}

			ShuffleV.Gen(StateIdV.Len());
			for (int i = 0; i < StateIdV.Len(); i++) {
				ShuffleV[i] = i;
			}

			ShuffleV.Shuffle(Rnd);

			for (int i = 0; i < ShuffleV.Len()-1; i++) {
				const int& State1Idx = ShuffleV[i];
				const int& State1Id = StateIdV[State1Idx];
				const double& RadiusI = RadiusV[State1Idx];
				TFltPr& PosI = GetModStateCoords(State1Id);

				// move the node
				for (int j = i+1; j < ShuffleV.Len(); j++) {
					const int& State2Idx = ShuffleV[j];
					const int& State2Id = StateIdV[State2Idx];
					const double& RadiusJ = RadiusV[State2Idx];
					const TFltPr& PosJ = GetModStateCoords(State2Id);

					const double Overlap = GetOverlap(PosI, PosJ,  RadiusI, RadiusJ);
					if (Overlap > 0) {
						GetMoveDir(PosI, PosJ, MoveDir);
						MoveDir.Val1 *= STEP_FACTOR;
						MoveDir.Val2 *= STEP_FACTOR;
						PosI.Val1 += MoveDir.Val1;
						PosI.Val2 += MoveDir.Val2;
						Change = true;
					}
				}
			}

			if (k % 100 == 0) {
				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Iteration %d, height %.4f, final overlaps ...", k, CurrHeight);
				for (int i = 0; i < RadiusV.Len()-1; i++) {
					const int& State1Id = StateIdV[i];

					for (int j = i+1; j < RadiusV.Len(); j++) {
						const int& State2Id = StateIdV[j];

						const double Overlap = GetOverlap(StateCoordV[State1Id], StateCoordV[State2Id],  RadiusV[i], RadiusV[j]);
						Notify->OnNotifyFmt(TNotifyType::ntInfo, "%d -> %d, r1: %.4f, r2: %.4f, prob1: %.4f, prob2: %.4f, pos1: (%.4f,%.4f), pos2: (%.4f,%.4f) overlap %.4f", i, j, RadiusV[i].Val, RadiusV[j].Val, ProbV[i].Val, ProbV[j].Val, StateCoordV[i].Val1, StateCoordV[i].Val2, StateCoordV[j].Val1, StateCoordV[j].Val2, Overlap);
					}
				}
			}
		}
	} while (Change);

	Notify->OnNotify(TNotifyType::ntInfo, "Done!");
}

void TUiHelper::InitAutoNmV(const TStreamStory& StreamStory) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Generating automatic names ...");

	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const THierarch& Hierarch = StreamStory.GetHierarch();

	const int States = Hierarch.GetStates();

	StateAutoNmV.Gen(States);
	StateIdAutoNmDescVV.Gen(States);

	const int AllDim = StreamStory.GetAllDim();

	TIntV AllLeafIdV;	Hierarch.GetLeafIdV(AllLeafIdV);

	TVec<TFltV> FtrAllBinV(AllDim, AllDim);

	TVec<TFltV> FtrPValVV(AllDim, AllDim);
	for (int FtrId = 0; FtrId < AllDim; FtrId++) {
		TFltV BinStartV;
		StateIdentifier.GetHistogram(StreamStory, FtrId, AllLeafIdV, BinStartV, FtrAllBinV[FtrId], false);

		const double TotalCount = TLinAlg::SumVec(FtrAllBinV[FtrId]);
		double ProbSum = 0;
		for (int BinN = 0; BinN < BinStartV.Len(); BinN++) {
			const double BinProb = double(FtrAllBinV[FtrId][BinN]) / TotalCount;
			FtrPValVV[FtrId].Add(ProbSum + BinProb/2);
			ProbSum += BinProb;
		}
	}

	TFltV BinValV, StateBinCountV;

	for (int StateId = 0; StateId < States; StateId++) {
		TAggState AggState;	Hierarch.GetLeafDescendantV(StateId, AggState);

		PAutoNmDescV& StateAutoNmDescV = StateIdAutoNmDescVV[StateId];

		int BestFtrN = -1;
		double BestFtrPVal = TFlt::Mx;
		int BestFtrSampleSize = 0;

		for (int FtrN = 0; FtrN < AllDim; FtrN++) {
			const TFltV& AllPValV = FtrPValVV[FtrN];
			const TFtrInfo& FtrInfo = StreamStory.GetFtrInfo(FtrN);

			StateIdentifier.GetHistogram(StreamStory, FtrN, AggState, BinValV, StateBinCountV, false);

			const double TotalCount = TLinAlg::SumVec(StateBinCountV);

			double PVal;
			int SampleSize;

			switch (FtrInfo.GetType()) {
			case ftUndefined: {
				throw TExcept::New("Cannot generate auto names for undefined feature type!");
				break;
			}
			case ftNumeric: {
				// calculate the mean and check into which percentile it falls
				int LowPercN = -1, HighPercN = -1;

				SampleSize = 0;
				double ProbSum = 0;
				for (int BinN = 0; BinN < BinValV.Len(); BinN++) {
					const int BinSize = (int) StateBinCountV[BinN];
					const double Prob = double(BinSize) / TotalCount;

					if (ProbSum <= STATE_LOW_PVAL_THRESHOLD && ProbSum + Prob > STATE_LOW_PVAL_THRESHOLD) {
						LowPercN = BinN;
					}
					if (ProbSum < 1 - STATE_LOW_PVAL_THRESHOLD && ProbSum + Prob >= 1 - STATE_LOW_PVAL_THRESHOLD) {
						HighPercN = BinN;
					}

					ProbSum += Prob;
					SampleSize += BinSize;
				}

				EAssert(LowPercN >= 0 && HighPercN >= 0);

				const double LowPercPVal = AllPValV[LowPercN];
				const double HighPercPVal = 1 - AllPValV[HighPercN];

				PVal = TMath::Mn(LowPercPVal, HighPercPVal);

				const TNumAutoNmLevel Level = TNumAutoNmDesc::GetAutoNmLevel(PVal, LowPercPVal, HighPercPVal);
				StateAutoNmDescV.Add(new TNumAutoNmDesc(FtrN, PVal, Level));
				break;
			}
			case ftCategorical: {
				const TFltV& AllBinCountV = FtrAllBinV[FtrN];

				// find the bin with most of the mass
				int TargetBinN = -1;
				double BestBinProb = -1;
				SampleSize = 0;
				for (int BinN = 0; BinN < BinValV.Len(); BinN++) {
					const int BinSize = (int) StateBinCountV[BinN];
					const double Prob = double(BinSize) / TotalCount;

					if (Prob > BestBinProb) {
						BestBinProb = Prob;
						TargetBinN = BinN;
					}

					SampleSize += BinSize;
				}

				EAssert(TargetBinN >= 0);

				// calculate the p-value for the found bin
				const int NTrials = 10000;

				int SuccTrials = 0;
				for (int TrialN = 0; TrialN < NTrials; TrialN++) {
					TFltV SimCountV;	THistogram::GenSamples(AllBinCountV, SampleSize, SimCountV, Rnd);

					const double SimTargetBinProb = double(SimCountV[TargetBinN]) / SampleSize;
					if (SimTargetBinProb >= BestBinProb) {
						SuccTrials++;
					}
				}

				PVal = double(SuccTrials) / NTrials;

				// add to the structure and update the best p-value
				StateAutoNmDescV.Add(new TCatAutoNmDesc(FtrN, PVal, TargetBinN));
				break;
			}
			case ftTime: {
			    throw TExcept::New("Time features not supported while generating auto names");
			}
			default:
				throw TExcept::New("Unknown feature type: " + TInt::GetStr(FtrInfo.GetType()));
			}

			if (PVal < BestFtrPVal || (PVal == BestFtrPVal && SampleSize > BestFtrSampleSize)) {
				BestFtrPVal = PVal;
				BestFtrN = FtrN;
				BestFtrSampleSize = SampleSize;
			}
		}

		StateAutoNmV[StateId] = StateAutoNmDescV[BestFtrN];
		StateAutoNmDescV.Sort(true);
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Auto names generated!");
}

void TUiHelper::RefineAutoNmV() {
    const int States = StateAutoNmV.Len();

    for (int StateId = 0; StateId < States; StateId++) {
        const PAutoNmDesc& AutoNmDesc = GetStateAutoNm(StateId);

        const TFtrType& FtrType = AutoNmDesc->GetFtrType();
        switch (FtrType) {
        case ftNumeric: {
            const TNumAutoNmDesc* NumAutoNmDesc = (TNumAutoNmDesc*) AutoNmDesc();
            const TNumAutoNmLevel& Level = NumAutoNmDesc->GetLevel();

            if (Level == nanlMeduim) {
                // try to name the state based on the time
                TTmDescV StateTmDescV;   GetTmDesc(StateId, StateTmDescV);

                if (!StateTmDescV.Empty()) {
                    const PAutoNmDesc NewAutoNmDesc = new TAutoNmTmDesc(StateTmDescV[0]);
                    SetStateAutoNm(StateId, NewAutoNmDesc);
                }
            }
            break;
        }
        case ftCategorical: {
            // TODO do nothing for now
            break;
        }
        case ftTime: {
            throw TExcept::New("Tried to refine a time feature, these should not exist yet!");
        }
        default: {
            throw TExcept::New("Unknown feature type: " + TInt::GetStr(FtrType));
        }
        }
    }

}

void TUiHelper::InitStateExplain(const TStreamStory& StreamStory) {

	const TStateIdentifier& StateIdentifier = StreamStory.GetStateIdentifier();
	const THierarch& Hierarch = StreamStory.GetHierarch();
	const TCtmcModeller& MChain = StreamStory.GetTransitionModeler();

	TIntFltPrV StateIdHeightPrV;	Hierarch.GetStateIdHeightPrV(StateIdHeightPrV);
	StateIdOccTmDescV.Gen(Hierarch.GetStates());

	const uint64& TmUnit = MChain.GetTimeUnit();

	const int MxPeaks = 1;
	const double MnSupport = .7;

	TIntPrV PeakStartEndV;
	double PeakMass;
	int PeakBinCount;

	for (int StateN = 0; StateN < StateIdHeightPrV.Len(); StateN++) {
		const int& StateId = StateIdHeightPrV[StateN].Val1;

		TIntV LeafV;	Hierarch.GetLeafDescendantV(StateId, LeafV);

		TIntV YearBinValV;
		TIntV MonthBinValV;
		TIntV WeekBinValV;
		TIntV DayBinValV;

		TFltV YearBinV;
		TFltV MonthBinV;
		TFltV WeekBinV;
		TFltV DayBinV;

		StateIdentifier.GetTimeHistogram(LeafV, TStateIdentifier::TTmHistType::thtYear, YearBinValV, YearBinV);
		StateIdentifier.GetTimeHistogram(LeafV, TStateIdentifier::TTmHistType::thtMonth, MonthBinValV, MonthBinV);
		StateIdentifier.GetTimeHistogram(LeafV, TStateIdentifier::TTmHistType::thtWeek, WeekBinValV, WeekBinV);
		StateIdentifier.GetTimeHistogram(LeafV, TStateIdentifier::TTmHistType::thtDay, DayBinValV, DayBinV);

		TTmDescV& StateTmDescV = StateIdOccTmDescV[StateId];

		if (TmUnit <= TCtmcModeller::TU_DAY && HasMxPeaks(MxPeaks, MnSupport, DayBinV, PeakStartEndV, PeakMass, PeakBinCount)) {
			Notify->OnNotifyFmt(ntInfo, "State %d has daily peaks!", StateId);
			Notify->OnNotifyFmt(ntInfo, TStrUtil::GetStr(DayBinV, ", ", "%.5f").CStr());

			StateTmDescV.Add(TTmDesc(TStateIdentifier::TTmHistType::thtDay, PeakStartEndV[0].Val1, PeakStartEndV[0].Val2));
		}
		if (TmUnit < TCtmcModeller::TU_MONTH && HasMxPeaks(MxPeaks, MnSupport, WeekBinV, PeakStartEndV, PeakMass, PeakBinCount)) {
			Notify->OnNotifyFmt(ntInfo, "State %d has weekly peaks!", StateId);
			Notify->OnNotifyFmt(ntInfo, TStrUtil::GetStr(WeekBinV, ", ", "%.5f").CStr());

			StateTmDescV.Add(TTmDesc(TStateIdentifier::TTmHistType::thtWeek, PeakStartEndV[0].Val1, PeakStartEndV[0].Val2));
		}
		if (TmUnit < TCtmcModeller::TU_MONTH && HasMxPeaks(MxPeaks, MnSupport, MonthBinV, PeakStartEndV, PeakMass, PeakBinCount)) {
			Notify->OnNotifyFmt(ntInfo, "State %d has monthly peaks!", StateId);
			Notify->OnNotifyFmt(ntInfo, TStrUtil::GetStr(MonthBinV, ", ", "%.5f").CStr());

			StateTmDescV.Add(TTmDesc(TStateIdentifier::TTmHistType::thtMonth, PeakStartEndV[0].Val1, PeakStartEndV[0].Val2));
		}
		if (HasMxPeaks(MxPeaks, MnSupport, YearBinV, PeakStartEndV, PeakMass, PeakBinCount)) {
			Notify->OnNotifyFmt(ntInfo, "State %d has yearly peaks!", StateId);
			Notify->OnNotifyFmt(ntInfo, TStrUtil::GetStr(YearBinV, ", ", "%.5f").CStr());

			StateTmDescV.Add(TTmDesc(TStateIdentifier::TTmHistType::thtYear, PeakStartEndV[0].Val1, PeakStartEndV[0].Val2));
		}

		Notify->OnNotifyFmt(ntInfo, "Got %d explanations for state %d!", StateTmDescV.Len(), StateId);
	}
}

bool TUiHelper::HasMxPeaks(const int& MxPeakCount, const double& PeakMassThreshold,
		const TFltV& PdfHist, TIntPrV& PeakBorderV, double& PeakMass, int& PeakBinCount) const {
	const int NBins = PdfHist.Len();
	const double TotalMass = TLinAlg::SumVec(PdfHist);
	const double MeanBinMass = TotalMass / NBins;

	EAssert(NBins > 0);
	PeakBorderV.Clr();

	PeakMass = 0;
	PeakBinCount = 0;

	int PeakCount = 0;
	bool IsInPeak = false;
	for (int BinN = 0; BinN < NBins; BinN++) {
		const double& BinVal = PdfHist[BinN];

		const bool BinInPeak = BinVal > MeanBinMass;

		if (BinInPeak != IsInPeak) {
			if (IsInPeak) {
				IsInPeak = false;
				PeakCount++;
				EAssertR(!PeakBorderV.Empty(), "Tried to end a peak, but no peak started!");
				PeakBorderV.Last().Val2 = BinN-1;
			} else {
				IsInPeak = true;
				PeakBorderV.Add(TIntPr(BinN, -1));
			}
		}

		if (IsInPeak) {
			PeakMass += BinVal;
			PeakBinCount++;
		}
	}

	// check if the peak is circular
	if (IsInPeak) {
	    if (PdfHist[0] > MeanBinMass) {
	        PeakCount--;
            EAssertR(PeakBorderV.Len() >= 2, "Not enough peaks in distribution when joining circularly!");
            PeakBorderV[0].Val1 = PeakBorderV.Last().Val1;
            PeakBorderV.DelLast();
	    } else {
	        PeakBorderV.Last().Val2 = PeakBorderV.Last().Val1;
	    }
	}

	EAssertR(PeakBorderV.Last().Val2 >= 0, "Failed to close the last peak in the histogram!");

	return PeakCount <= MxPeakCount && PeakMass / TotalMass >= PeakMassThreshold;
}

void TUiHelper::GetTmDesc(const int& StateId, TTmDescV& DescV) const {
	DescV = StateIdOccTmDescV[StateId];
}

void TUiHelper::GetFromToStrPr(const TTmDesc& Desc, TStrPr& StrDesc, const TDescType& DescType) {
	const TStateIdentifier::TTmHistType HistTmScale = (TStateIdentifier::TTmHistType) ((uchar) Desc.Val1);

	switch (HistTmScale) {
	case TStateIdentifier::TTmHistType::thtYear: {
		const int& StartMonth = Desc.Val2;
		const int& EndMonth = Desc.Val3;

		EAssert(0 <= StartMonth && StartMonth < 12);
		EAssert(0 <= EndMonth && EndMonth < 12);

		switch (DescType) {
		case dtNormal: {
            StrDesc.Val1 = MONTHS[StartMonth];
            StrDesc.Val2 = MONTHS[EndMonth];
            break;
		}
		case dtShort: {
            StrDesc.Val1 = MONTHS_SHORT[StartMonth];
            StrDesc.Val2 = MONTHS_SHORT[EndMonth];
            break;
		}
		}

		break;
	}
	case TStateIdentifier::TTmHistType::thtMonth: {
		const int& StartDay = Desc.Val2;
		const int& EndDay = Desc.Val3;

		EAssert(0 <= StartDay && StartDay < 31);
		EAssert(0 <= EndDay && EndDay < 31);

		StrDesc.Val1 = DAYS_IN_MONTH[StartDay];
		StrDesc.Val2 = DAYS_IN_MONTH[EndDay];

		break;
	}
	case TStateIdentifier::TTmHistType::thtWeek: {
		const int& StartDay = Desc.Val2;
		const int& EndDay = Desc.Val3;

		EAssert(0 <= StartDay && StartDay < 7);
		EAssert(0 <= EndDay && EndDay < 7);

		switch (DescType) {
		case dtNormal: {
            StrDesc.Val1 = DAYS_IN_WEEK[StartDay];
            StrDesc.Val2 = DAYS_IN_WEEK[EndDay];
            break;
		}
		case dtShort: {
            StrDesc.Val1 = DAYS_IN_WEEK_SHORT[StartDay];
            StrDesc.Val2 = DAYS_IN_WEEK_SHORT[EndDay];
            break;
		}
		}

		break;
	}
	case TStateIdentifier::TTmHistType::thtDay: {
		const int& StartHour = Desc.Val2;
		const int& EndHour = Desc.Val3;

		EAssert(0 <= StartHour && StartHour < 24);
		EAssert(0 <= EndHour && EndHour < 24);

		StrDesc.Val1 = HOURS_IN_DAY[StartHour];
		StrDesc.Val2 = HOURS_IN_DAY[EndHour];

		break;
	}
	default: {
		throw TExcept::New("Unknown time histogram type: " + TStr(HistTmScale));
	}
	}
}

double TUiHelper::GetStateRaduis(const double& Prob) {
	// the probability is proportional to the area, so the raduis should
	// be proportional to the square root of the probability
	return /*RADIUS_FACTOR */TMath::Sqrt(Prob / TMath::Pi);
}

bool TUiHelper::NodesOverlap(const int& StartId, const int& EndId, const TFltPrV& CoordV, const TFltV& RaduisV) {
	for (int i = StartId; i < EndId; i++) {
		for (int j = i+1; j <= EndId; j++) {
			if (i == j) { continue; }
			if (GetOverlap(CoordV[i], CoordV[j], RaduisV[i], RaduisV[j]) > 0) {
				return true;
			}
		}
	}
	return false;
}

int TUiHelper::CountOverlaps(const int& StartId, const int& EndId, const TFltPrV& CoordV, const TFltV& RaduisV) {
	int Count = 0;
	for (int i = StartId; i < EndId; i++) {
		for (int j = i+1; j <= EndId; j++) {
			if (i == j) { continue; }
			if (GetOverlap(CoordV[i], CoordV[j], RaduisV[i], RaduisV[j]) > 0) {
				Count++;
			}
		}
	}
	return Count;
}

double TUiHelper::GetOverlap(const TFltPr& Pos1, const TFltPr& Pos2,
			const double& Radius1, const double& Radius2) {
	return Radius1 + Radius2 - TMath::Sqrt(TMath::Sqr(Pos1.Val1 - Pos2.Val1) + TMath::Sqr(Pos1.Val2 - Pos2.Val2));
}

void TUiHelper::GetMoveDir(const TFltPr& Pos1, const TFltPr& Pos2, TFltPr& Dir) {
	const double Norm = TLinAlg::EuclDist(Pos1, Pos2);
	Dir.Val1 = (Pos1.Val1 - Pos2.Val1) / Norm;
	Dir.Val2 = (Pos1.Val2 - Pos2.Val2) / Norm;
}

////////////////////////////////////////////////
// State assistant
TStateAssist::TStateAssist(const bool _Verbose):
		ClassifyV(),
		DecisionTreeV(),
		FtrBoundV(),
		Rnd(1),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TStateAssist::TStateAssist(TSIn& SIn):
		ClassifyV(SIn),
		DecisionTreeV(SIn),
		FtrBoundV(SIn),
		Rnd(SIn),
		Verbose(TBool(SIn)) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStateAssist::Save(TSOut& SOut) const {
	ClassifyV.Save(SOut);
	DecisionTreeV.Save(SOut);
	FtrBoundV.Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TStateAssist::Init(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
		const TFltVV& IgnFtrVV, const TStateIdentifier& Clust, const THierarch& Hierarch,
		TStreamStoryCallback* Callback, const bool& MultiThread) {

	Notify->OnNotify(TNotifyType::ntInfo, "Computing state assist ...");

	const int NInst = ObsFtrVV.GetCols();

	const int ObsDim = ObsFtrVV.GetRows();
	const int ContrDim = ContrFtrVV.GetRows();
	const int IgnDim = IgnFtrVV.GetRows();

	const int AllDim = ObsDim + ContrDim + IgnDim;

	TFltVV AllFtrVV(AllDim, NInst);
	for (int ColN = 0; ColN < NInst; ColN++) {
		for (int RowN = 0; RowN < ObsDim; RowN++) {
			AllFtrVV(RowN, ColN) = ObsFtrVV(RowN, ColN);
		}
		for (int RowN = 0; RowN < ContrDim; RowN++) {
			AllFtrVV(ObsDim + RowN, ColN) = ContrFtrVV(RowN, ColN);
		}
		for (int RowN = 0; RowN < IgnDim; RowN++) {
			AllFtrVV(ObsDim + ContrDim + RowN, ColN) = IgnFtrVV(RowN, ColN);
		}
	}

	InitFtrBounds(ObsFtrVV, ContrFtrVV, IgnFtrVV);

	// get all the heights from the hierarchy
	TIntFltPrV StateIdHeightPrV;	Hierarch.GetStateIdHeightPrV(StateIdHeightPrV);
	TIntV AssignV;	Clust.Assign(RecTmV, ObsFtrVV, AssignV);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist, total states %d ...", StateIdHeightPrV.Len());

	ClassifyV.Clr(true);
	DecisionTreeV.Clr(true);

	for (int HeightN = 0; HeightN < StateIdHeightPrV.Len(); HeightN++) {
		ClassifyV.Add(TLogReg(1, true, false));
		DecisionTreeV.Add(TDecisionTree());
	}

	// initialize
	if (MultiThread) {
		int NFinished = 0;
		const int TotalTasks = StateIdHeightPrV.Len();

		TVec<TRnd> RndV;
		for (int HeightN = 0; HeightN < StateIdHeightPrV.Len(); HeightN++) {
			RndV.Add(TRnd(Rnd.GetUniDevInt()));
		}

		#pragma omp parallel for
		for (int HeightN = 0; HeightN < TotalTasks; HeightN++) {
			const TIntFltPr StateIdHeightPr = StateIdHeightPrV[HeightN];

			InitSingle(AllFtrVV, StateIdHeightPr.Val1, StateIdHeightPr.Val2, Hierarch,
					AssignV, RndV[HeightN], ClassifyV[HeightN], DecisionTreeV[HeightN]);

			#pragma omp critical
			{
				Notify->OnNotifyFmt(TNotifyType::ntInfo, "Finished task %d out of %d ...", ++NFinished, TotalTasks);
				if (Callback != nullptr) {
					Callback->OnProgress(70, "Initilized " + TInt::GetStr(NFinished) + " of " + TInt::GetStr(TotalTasks) + " states ...");
				}
			}
		}
	} else {
		for (int HeightN = 0; HeightN < StateIdHeightPrV.Len(); HeightN++) {
			const TIntFltPr& StateIdHeightPr = StateIdHeightPrV[HeightN];
			const int& StateId = StateIdHeightPr.Val1;
			const double& Height = StateIdHeightPr.Val2;

			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist for state %d ...", StateId);

			InitSingle(AllFtrVV, StateId, Height, Hierarch, AssignV, Rnd, ClassifyV[HeightN], DecisionTreeV[HeightN]);

			if (Callback != nullptr) {
				Callback->OnProgress(70, "Initialized " + TInt::GetStr(HeightN+1) + " of " + TInt::GetStr(StateIdHeightPrV.Len()) + " states ...");
			}
		}
	}

	Notify->OnNotify(TNotifyType::ntInfo, "State assist initialized!");
}

void TStateAssist::InitFtrBounds(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
		const TFltVV& IgnoredFtrVV) {
	const int NObsFtrs = ObsFtrVV.GetRows();
	const int NContrFtrs = ContrFtrVV.GetRows();
	const int NIgnFtrs = IgnoredFtrVV.GetRows();

	const int TotalFtrs = NObsFtrs + NContrFtrs + NIgnFtrs;
	const int NInst = ObsFtrVV.GetCols();


	FtrBoundV.Gen(TotalFtrs);

	for (int FtrN = 0; FtrN < NObsFtrs; FtrN++) {
		double Min = TFlt::Mx, Max = TFlt::Mn;
		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ObsFtrVV(FtrN, InstN) < Min) { Min = ObsFtrVV(FtrN, InstN); }
			if (ObsFtrVV(FtrN, InstN) > Max) { Max = ObsFtrVV(FtrN, InstN); }
		}

		FtrBoundV[FtrN] = TFltPr(Min, Max);
	}

	for (int FtrN = 0; FtrN < NContrFtrs; FtrN++) {
		double Min = TFlt::Mx, Max = TFlt::Mn;
		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ContrFtrVV(FtrN, InstN) < Min) { Min = ContrFtrVV(FtrN, InstN); }
			if (ContrFtrVV(FtrN, InstN) > Max) { Max = ContrFtrVV(FtrN, InstN); }
		}

		FtrBoundV[NObsFtrs + FtrN] = TFltPr(Min, Max);
	}

	for (int FtrN = 0; FtrN < NIgnFtrs; FtrN++) {
		double Min = TFlt::Mx, Max = TFlt::Mn;
		for (int InstN = 0; InstN < NInst; InstN++) {
			if (IgnoredFtrVV(FtrN, InstN) < Min) { Min = IgnoredFtrVV(FtrN, InstN); }
			if (IgnoredFtrVV(FtrN, InstN) > Max) { Max = IgnoredFtrVV(FtrN, InstN); }
		}

		FtrBoundV[FtrN + NObsFtrs + NContrFtrs] = TFltPr(Min, Max);
	}
}

const TFltPr& TStateAssist::GetFtrBounds(const int& FtrId) const {
	EAssertR(FtrId < FtrBoundV.Len(), "TStateAssist::GetFtrBounds: invalid feature ID!");
	return FtrBoundV[FtrId];
}

void TStateAssist::GetFtrWgtV(const int& StateId, const int& Offset, const int& Length,
		TFltV& WgtV) const {
	EAssertR(0 <= StateId && StateId < ClassifyV.Len(), "Invalid state ID!");

	if (WgtV.Len() != Length) { WgtV.Gen(Length); }

	const TLogReg& Classify = ClassifyV[StateId];
	TFltV AllWgtV;	Classify.GetWgtV(AllWgtV);

	EAssert(0 <= Offset && Offset + Length <= AllWgtV.Len());

	for (int FtrN = 0; FtrN < Length; FtrN++) {
		WgtV[FtrN] = AllWgtV[Offset + FtrN];
	}
}

PJsonVal TStateAssist::GetStateClassifyTree(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < DecisionTreeV.Len(), "Invalid StateId!");
	return DecisionTreeV[StateId].GetJson();
}

PJsonVal TStateAssist::GetStateExplain(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < DecisionTreeV.Len(), "Invalid StateId!");
	return DecisionTreeV[StateId].ExplainPositive();
}

void TStateAssist::InitSingle(const TFltVV& FtrVV, const int& StateId, const double& Height,
		const THierarch& Hierarch, const TIntV& AssignV, TRnd& Rnd, TLogReg& LogReg,
		TDecisionTree& Tree) {

	const int NInst = FtrVV.GetCols();

	TStateIdV StateIdV; TAggStateV AggStateV;
	Hierarch.GetStateSetsAtHeight(Height, StateIdV, AggStateV);

	const int NStates = StateIdV.Len();
	const int TrgStateIdx = StateIdV.SearchForw(StateId);
	EAssertR(TrgStateIdx >= 0, "Could not find the target state!");

	TIntV TargetIdxV, NonTargetIdxV;
	TVec<TIntV> StateIdFtrIdxVV(NStates);

	TIntSet TargetStateSet;
	for (int StateN = 0; StateN < NStates; StateN++) {
		if (StateIdV[StateN] == StateId) {
			TargetStateSet.AddKeyV(AggStateV[StateN]);
		}
	}

	EAssert(!TargetStateSet.Empty());

	TFltV LabelV(NInst);
	for (int ColN = 0; ColN < NInst; ColN++) {
		LabelV[ColN] = TargetStateSet.IsKey(AssignV[ColN]) ? 1 : 0;
	}

	FitAssistModels(FtrVV, LabelV, LogReg, Tree);
}

void TStateAssist::FitAssistModels(const TFltVV& FtrVV, const TFltV& LabelV, TLogReg& LogReg,
			TDecisionTree& Tree) {
	const int NInst = FtrVV.GetCols();

	{
		int NPos = 0;
		for (int ColN = 0; ColN < NInst; ColN++) {
			NPos += (int) LabelV[ColN];
		}

		const int MinExamples = TMath::Mn(50, NInst / 32);
		const double MinPosProb = TMath::Mn((double(NPos) / NInst) / 2, .05);
		const double MinNegProb = TMath::Mn((double(NInst - NPos) / NInst) / 2, .05);

		PDtSplitCriteria SplitCriteria = TInfoGain::New();
		PDtPruneCriteria PruneCriteria = TDtMinExamplesPrune::New(MinExamples);
		PDtGrowCriteria GrowCriteria = TDtGrowCriteria::New(MinPosProb, MinNegProb, MinExamples);

		Tree = TDecisionTree(SplitCriteria, PruneCriteria, GrowCriteria, true);
	}

	LogReg.Fit(FtrVV, LabelV);
	Tree.Fit(FtrVV, LabelV, TNotify::NullNotify);
}

/////////////////////////////////////////////////////////////////
// Activity info
TActivityDetector::TActivity::TActivity():
		ActivitySeq(),
		UniqueStepV(),
		StepStartEndTmIdV() {}

TActivityDetector::TActivity::TActivity(const TActivityStepV& StepV):
		ActivitySeq(),
		UniqueStepV(),
		StepStartEndTmIdV() {

	// construct unique steps
	for (int StepN = 0; StepN < StepV.Len(); StepN++) {
		const TActivityStep& Step = StepV[StepN];

		if (!ContainsStep(Step)) {
			UniqueStepV.Add(Step);
		}
	}

	for (int StepN = 0; StepN < StepV.Len(); StepN++) {
		const TActivityStep& Step = StepV[StepN];
		const int StateId = Step[0];
		const int StepId = GetStepId(StateId);
		ActivitySeq.Add(StepId);
	}
}

TActivityDetector::TActivity::TActivity(TSIn& SIn):
		ActivitySeq(SIn),
		UniqueStepV(SIn)/*,
		StepStartEndTmIdV(SIn)*/ {}

void TActivityDetector::TActivity::Save(TSOut& SOut) const {
	ActivitySeq.Save(SOut);
	UniqueStepV.Save(SOut);
//	StepStartEndTmIdV.Save(SOut);
}

bool TActivityDetector::TActivity::Update(const uint64& Tm, const int& StateId,
		const PNotify& Notify) {
	const int StepId = GetStepId(StateId);
	if (StepStartEndTmIdV.Empty() || StepStartEndTmIdV.Last().Val3 != StepId) {
		if (!StepStartEndTmIdV.Empty()) {
			StepStartEndTmIdV.Last().Val2 = Tm;
		}
		StepStartEndTmIdV.Add(TUInt64UInt64IntTr(Tm, TUInt64::Mx, StepId));

		while (StepStartEndTmIdV.Len() > ActivitySeq.Len() + 1) {
			StepStartEndTmIdV.Del(0);
		}

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Updated history: %s", TStrUtil::GetStr(StepStartEndTmIdV).CStr());
		return true;
	}

	return false;
}

bool TActivityDetector::TActivity::Detect(uint64& StartTm, uint64& EndTm, const PNotify& Notify) {
	if (StepStartEndTmIdV.Len() != ActivitySeq.Len() + 1) {
		Notify->OnNotify(TNotifyType::ntInfo, "Won't detect activities, not enough history ...");
		return false;
	}

	for (int StepN = 0; StepN < ActivitySeq.Len(); StepN++) {
		const int& TemplateStepId = ActivitySeq[StepN];
		const int& RealStepId = StepStartEndTmIdV[StepN].Val3;

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Matching steps, template: %d, real: %d", TemplateStepId, RealStepId);

		if (TemplateStepId != RealStepId) {
			return false;
		}
	}

	// set the start/end times
	StartTm = StepStartEndTmIdV[0].Val1;
	EndTm = StepStartEndTmIdV[ActivitySeq.Len()-1].Val2;

	Notify->OnNotifyFmt(ntInfo, "Detected activity lasting %.2f minutes!", double(EndTm - StartTm) / (1000*60));

	EAssert(EndTm != TUInt64::Mx);

	// remove the history, so we don't detect the activity again
	StepStartEndTmIdV.Del(0);

	return true;
}

int TActivityDetector::TActivity::GetStepId(const int& StateId) const {
	int Result = -1;
	for (int StepN = 0; StepN < UniqueStepV.Len(); StepN++) {
		if (UniqueStepV[StepN].IsKey(StateId)) {
			Result = StepN;
		}
	}
	return Result;
}

bool TActivityDetector::TActivity::ContainsStep(const TActivityStep& Step) const {
	for (int StepN = 0; StepN < UniqueStepV.Len(); StepN++) {
		if (UniqueStepV[StepN] == Step) {
			return true;
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////////
// Activity detector
const int TActivityDetector::STATE_CACHE_SIZE = 150;

TActivityDetector::TActivityDetector(const bool& _Verbose):
		ActivityH(),
		StateIdHistoryV(STATE_CACHE_SIZE, 0),
		Callback(nullptr),
		Verbose(_Verbose),
		Notify() {
	SetVerbose(Verbose);
}

TActivityDetector::TActivityDetector(TSIn& SIn):
		ActivityH(SIn),
		StateIdHistoryV(SIn),
		Callback(nullptr),
		Verbose(TBool(SIn)),
		Notify() {
	SetVerbose(Verbose);
}

void TActivityDetector::Save(TSOut& SOut) const {
	ActivityH.Save(SOut);
	StateIdHistoryV.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TActivityDetector::OnStateChanged(const uint64& Tm, const int& NewStateId) {
	Notify->OnNotify(TNotifyType::ntInfo, "Matching activities ...");

	// check is any of the activities match
	int KeyId = ActivityH.FFirstKeyId();
	while (ActivityH.FNextKeyId(KeyId)) {
		const TStr& ActNm = ActivityH.GetKey(KeyId);
		TActivity& Activity = ActivityH[KeyId];

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Updating activity \"%s\" ...", ActNm.CStr());

		if (!Activity.Update(Tm, NewStateId, Notify)) { continue; }

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Matching activity \"%s\" ...", ActNm.CStr());

		uint64 StartTm, EndTm;
		if (Activity.Detect(StartTm, EndTm, Notify)) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Found activity match: %s", ActNm.CStr());
			if (Callback != nullptr) { Callback->OnActivityDetected(StartTm, EndTm, ActNm); }
		}
	}
}

void TActivityDetector::AddActivity(const TStr& ActName, const TActivityStepV& StepV) {
	EAssertR(!ActivityH.IsKey(ActName), "Activity " + ActName + " already present!");
	ActivityH.AddDat(ActName, TActivity(StepV));
}

void TActivityDetector::RemoveActivity(const TStr& ActNm) {
	EAssertR(ActivityH.IsKey(ActNm), "Activity " + ActNm + " is not present!");
	ActivityH.DelKey(ActNm);
}

void TActivityDetector::GetActivities(TStrV& ActNmV, TIntV& NumStepsV) const {
	int KeyId = ActivityH.FFirstKeyId();
	while (ActivityH.FNextKeyId(KeyId)) {
		const TActivity& Activity = ActivityH[KeyId];

		ActNmV.Add(ActivityH.GetKey(KeyId));
		NumStepsV.Add(Activity.GetNumSteps());
	}
}

bool TActivityDetector::IsEmpty() const {
	return ActivityH.Empty();
}

void TActivityDetector::SetCallback(TStreamStoryCallback* _Callback) {
	Callback = _Callback;
}

void TActivityDetector::SetVerbose(const bool& _Verbose) {
	Verbose = _Verbose;
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

/////////////////////////////////////////////////////////////////
// Main StreamStory component
TStreamStory::TStreamStory():
		StateIdentifier(nullptr),
		MChain(nullptr),
		Hierarch(nullptr),
		StateAssist(nullptr),
		ActivityDetector(nullptr),
		UiHelper(nullptr),
		FtrNToIdV(),
		ObsFtrInfoV(),
		ContrFtrInfoV(),
		IgnFtrInfoV(),
		FtrVecPredP(false),
		LastObsFtrV(),
		LastContrFtrV(),
		LastStateId(-1),
		LastRecTm(),
		Verbose(true),
		Callback(nullptr),
		Notify(nullptr) {}

TStreamStory::TStreamStory(TStateIdentifier* _StateIdentifier, TCtmcModeller* _MChain,
		THierarch* _Hierarch, const TRnd& Rnd, const bool& _Verbose):
		StateIdentifier(_StateIdentifier),
		MChain(_MChain),
		Hierarch(_Hierarch),
		StateAssist(new TStateAssist(_Verbose)),
		ActivityDetector(new TActivityDetector(_Verbose)),
		UiHelper(new TUiHelper(Rnd, _Verbose)),
		FtrNToIdV(),
		ObsFtrInfoV(),
		ContrFtrInfoV(),
		IgnFtrInfoV(),
		FtrVecPredP(false),
		LastObsFtrV(),
		LastContrFtrV(),
		LastStateId(-1),
		LastRecTm(),
		Verbose(_Verbose),
		Callback(nullptr),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
}

TStreamStory::TStreamStory(TSIn& SIn):
		StateIdentifier(new TStateIdentifier(SIn)),
		MChain(TCtmcModeller::Load(SIn)),
		Hierarch(THierarch::Load(SIn)),
		StateAssist(new TStateAssist(SIn)),
		ActivityDetector(new TActivityDetector(SIn)),
		UiHelper(new TUiHelper(SIn)),
		FtrNToIdV(SIn),
		ObsFtrInfoV(SIn),
		ContrFtrInfoV(SIn),
		IgnFtrInfoV(SIn),
		FtrVecPredP(TBool(SIn)),
		LastObsFtrV(SIn),
		LastContrFtrV(SIn),
		LastStateId(TInt(SIn)),
		LastRecTm(TUInt64(SIn)),
		Verbose(TBool(SIn)),
		Callback(nullptr),
		Notify() {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

TStreamStory::~TStreamStory() {
	if (StateIdentifier != nullptr) { delete StateIdentifier; }
	if (MChain != nullptr) { delete MChain; }
	if (Hierarch != nullptr) { delete Hierarch; }
	if (StateAssist != nullptr) { delete StateAssist; }
	if (ActivityDetector != nullptr) { delete ActivityDetector; }
	if (UiHelper != nullptr) { delete UiHelper; }
}

void TStreamStory::Save(TSOut& SOut) const {
	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::Save: saving to stream ...");

	StateIdentifier->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
	StateAssist->Save(SOut);
	ActivityDetector->Save(SOut);
	UiHelper->Save(SOut);
	FtrNToIdV.Save(SOut);
	ObsFtrInfoV.Save(SOut);
	ContrFtrInfoV.Save(SOut);
	IgnFtrInfoV.Save(SOut);
	TBool(FtrVecPredP).Save(SOut);
	LastObsFtrV.Save(SOut);
	LastContrFtrV.Save(SOut);
	TInt(LastStateId).Save(SOut);
	TUInt64(LastRecTm).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PJsonVal TStreamStory::GetJson() const {
	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::GetJson: saving JSON ...");

	PJsonVal Result = TJsonVal::NewArr();

	// we need to build a hierarchy and model state transitions
	// on each level of the hierarchy

	// variables
	TAggStateV StateSetV;
	TStateIdV StateIdV;
	TIntFltPrV StateIdProbPrV;

	const TFltV& UniqueHeightV = Hierarch->GetUiHeightV();
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV, false);

	// go through all the heights except the last one, which is not interesting
	// since it is only one state
	for (int HeightN = 0; HeightN < UniqueHeightV.Len(); HeightN++) {
		const double CurrHeight = UniqueHeightV[HeightN];
		const double NextHeight = HeightN < UniqueHeightV.Len()-1 ? UniqueHeightV[HeightN+1].Val : TFlt::PInf;

		StateIdV.Clr();
		StateSetV.Clr();
		StateIdProbPrV.Clr();

		// get the states on this level
		Hierarch->GetStateSetsAtHeight(CurrHeight, StateIdV, StateSetV);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// children
		TFltVV TransitionVV;	MChain->GetJumpVV(StateSetV, StateFtrVV, TransitionVV);
		TFltV HoldingTimeV;		MChain->GetHoldingTmV(StateSetV, StateFtrVV, HoldingTimeV);
		TFltV ProbV;			MChain->GetStatDist(StateSetV, StateFtrVV, ProbV);
		TFltV RadiusV;			UiHelper->GetStateRadiusV(ProbV, RadiusV);

		PJsonVal LevelJsonVal = GetLevelJson(CurrHeight, NextHeight, StateIdV, TransitionVV, HoldingTimeV, ProbV, RadiusV);
		Result->AddToArr(LevelJsonVal);
	}

	return Result;
}

PJsonVal TStreamStory::GetSubModelJson(const int& StateId) const {
	PJsonVal Result = TJsonVal::NewArr();

	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV, false);

	const double MxHeight = Hierarch->GetStateHeight(StateId);
	TIntV TopStateIdV;	TAggStateV TopStateAggV;
	Hierarch->GetStateSetsAtHeight(MxHeight, TopStateIdV, TopStateAggV);

	const int TargetStateN = TopStateIdV.SearchForw(StateId);

	TIntV DescendantIdV = TopStateAggV[TargetStateN];

	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::GetSubModelJson: saving JSON ...");

	TAggStateV AncAggStateV;
	TAggStateV AggStateV(TopStateIdV.Len()-1, 0);
	{
		Hierarch->GetDescendantsAtHeight(0, TopStateIdV, AncAggStateV);
		for (int StateN = 0; StateN < AncAggStateV.Len(); StateN++) {
			if (StateN != TargetStateN) {
				AggStateV.Add(AncAggStateV[StateN]);
			}
		}
	}


	// construct for height 0
	double CurrHeight = 0;
	TIntV NewDescIdV;
	while (CurrHeight < MxHeight) {
		PJsonVal LevelJsonVal = TJsonVal::NewObj();
		const double NextHeight = Hierarch->GetNextUiLevel(DescendantIdV, NewDescIdV);

		// prepare the structures
		AncAggStateV.Clr();
		AggStateV.Trunc(TopStateIdV.Len()-1);

		Hierarch->GetDescendantsAtHeight(CurrHeight, TopStateIdV, AncAggStateV);

		const TIntV& LevelAggTargetState = AncAggStateV[TargetStateN];
		for (int StateN = 0; StateN < LevelAggTargetState.Len(); StateN++) {
			AggStateV.Add(TIntV());
			Hierarch->GetLeafDescendantV(LevelAggTargetState[StateN], AggStateV.Last());
		}

		// construct this level
		TFltVV AllJumpVV;		MChain->GetJumpVV(AggStateV, StateFtrVV, AllJumpVV);//TCtmcModeller::GetJumpVV(SubQMat, TransitionVV);
		TFltV AllHoldingTmV;	MChain->GetHoldingTmV(AggStateV, StateFtrVV, AllHoldingTmV);
		TFltV AllProbV;			MChain->GetStatDist(AggStateV, StateFtrVV, AllProbV);
		TFltV AllRadiusV;		UiHelper->GetStateRadiusV(AllProbV, AllRadiusV);

		// construct the values for the sub chain
		const int NStates = LevelAggTargetState.Len();

		TFltVV JumpVV(NStates, NStates);
		TFltV HoldingTmV(NStates, NStates);
		TFltV ProbV(NStates, NStates);
		TFltV RadiusV(NStates, NStates);

		for (int StateN = 0; StateN < NStates; StateN++) {
			const int AllStateN = (AncAggStateV.Len() - 1) + StateN;

			HoldingTmV[StateN] = AllHoldingTmV[AllStateN];
			ProbV[StateN] = AllProbV[AllStateN];
			RadiusV[StateN] = AllRadiusV[AllStateN];

			for (int DstStateN = 0; DstStateN < NStates; DstStateN++) {
				const int DstAllStateN = (AncAggStateV.Len() - 1) + DstStateN;

				JumpVV(StateN, DstStateN) = AllJumpVV(AllStateN, DstAllStateN);
			}
		}

		PJsonVal LevelJsonV = GetLevelJson(CurrHeight, NextHeight, LevelAggTargetState, JumpVV, HoldingTmV, ProbV, RadiusV);
		Result->AddToArr(LevelJsonV);

		// move the the next level
		CurrHeight = NextHeight;
		DescendantIdV = NewDescIdV;
		NewDescIdV.Clr();
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Done.");

	return Result;
}

PJsonVal TStreamStory::GetLikelyPathTreeJson(const int& StateId, const double& Height,
		const int& MxDepth, const double& TransThreshold) const {
	TIntV StateIdV;	TAggStateV AggStateV;	TIntV NextLevelIdV;
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, AggStateV);

	const double NextHeight =Hierarch->GetNextUiLevel(StateIdV, NextLevelIdV);

	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV, false);

	TFltVV PMat; TIntV PMatStateIdV;
	MChain->GetLikelyPathTree(StateId, StateIdV, AggStateV, StateFtrVV, MxDepth, PMat, PMatStateIdV, TransThreshold);

	// construct the result
	PJsonVal Result = TJsonVal::NewArr();

	TFltV AllHoldingTmV;	MChain->GetHoldingTmV(AggStateV, StateFtrVV, AllHoldingTmV);
	TFltV AllProbV;			MChain->GetStatDist(AggStateV, StateFtrVV, AllProbV);
	TFltV AllRadiusV;		UiHelper->GetStateRadiusV(AllProbV, AllRadiusV);

	TFltV HoldingTmV(PMatStateIdV.Len());
	TFltV ProbV(PMatStateIdV.Len());
	TFltV RadiusV(PMatStateIdV.Len());

	TIntH StateIdStateNH;
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		StateIdStateNH.AddDat(StateIdV[StateN], StateN);
	}

	for (int PMatStateN = 0; PMatStateN < PMatStateIdV.Len(); PMatStateN++) {
		const int StateId = PMatStateIdV[PMatStateN];
		const int StateN = StateIdStateNH.GetDat(StateId);

		HoldingTmV[PMatStateN] = AllHoldingTmV[StateN];
		ProbV[PMatStateN] = AllProbV[StateN];
		RadiusV[PMatStateN] = AllRadiusV[StateN];
	}

	TFltPrV StatePosV;	TreeLayout(PMat, StateIdV, RadiusV, StatePosV);

	PJsonVal LevelJson = GetLevelJson(Height, NextHeight, PMatStateIdV, PMat, HoldingTmV, ProbV, RadiusV);

	// state ids repeat, so transform them for the UI
	PJsonVal StatesJson = LevelJson->GetObjKey("states");

	for (int StateN = 0; StateN < StatesJson->GetArrVals(); StateN++) {
		PJsonVal StateJson = StatesJson->GetArrVal(StateN);

		StateJson->AddToObj("x", StatePosV[StateN].Val1);
		StateJson->AddToObj("y", StatePosV[StateN].Val2);
	}

	Result->AddToArr(LevelJson);

	return Result;
}

void TStreamStory::Init(const TFtrInfoV& _ObsFtrInfoV, const TFtrInfoV& _ContrFtrInfoV,
		const TFtrInfoV& _IgnFtrInfoV, const TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
		const TFltVV& IgnoredFtrVV, const TUInt64V& RecTmV, const bool& MultiThread) {

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Creating a model on %d instances ...", ObservFtrVV.GetCols());

	ObsFtrInfoV = _ObsFtrInfoV;
	ContrFtrInfoV = _ContrFtrInfoV;
	IgnFtrInfoV = _IgnFtrInfoV;

	InitFtrNToIdV();

	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, TBoolV(), FtrVV);

	EAssertR(!TLinAlgCheck::ContainsNan(ObservFtrVV), "Nans in the data!");

	Callback->OnProgress(0, "Clustering ...");
	TIntV AssignV;	InitClust(RecTmV, ObservFtrVV, FtrVV, IgnoredFtrVV, AssignV);
	Callback->OnProgress(30, "Modeling transitions ...");
	InitMChain(FtrVV, AssignV, RecTmV, false, TBoolV());
	Callback->OnProgress(50, "Initializing hierarchy ...");
	InitHierarch(RecTmV, ObservFtrVV);
	Callback->OnProgress(60, "Initializing states ...");
	InitStateAssist(RecTmV, ObservFtrVV, ControlFtrVV, IgnoredFtrVV, MultiThread);
	Callback->OnProgress(90, "Computing positions ...");
	UiHelper->Init(*this);
}

void TStreamStory::InitBatches(const TFtrInfoV& _ObsFtrInfoV, const TFtrInfoV& _ContrFtrInfoV,
		const TFtrInfoV& _IgnFtrInfoV, const TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
		const TFltVV& IgnoredFtrVV, const TUInt64V& RecTmV, const TBoolV& BatchEndV,
		const bool& MultiThread) {

	ObsFtrInfoV = _ObsFtrInfoV;
	ContrFtrInfoV = _ContrFtrInfoV;
	IgnFtrInfoV = _IgnFtrInfoV;

	InitFtrNToIdV();

	CheckBatches(RecTmV, BatchEndV);

	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, BatchEndV, FtrVV);

	TIntV AssignV;	InitClust(RecTmV, ObservFtrVV, FtrVV, IgnoredFtrVV, AssignV);
	InitMChain(FtrVV, AssignV, RecTmV, true, BatchEndV);
	InitHierarch(RecTmV, ObservFtrVV);
	InitStateAssist(RecTmV, ObservFtrVV, ControlFtrVV, IgnoredFtrVV, MultiThread);
	UiHelper->Init(*this);
}

void TStreamStory::InitFtrNToIdV() {
	if (!FtrNToIdV.Empty()) { FtrNToIdV.Clr(); }

	for (int FtrId = 0; FtrId < GetAllDim(); FtrId++) {
		const TFtrInfo& Info = GetFtrInfo(FtrId);
		for (int FtrN = 0; FtrN < Info.GetLength(); FtrN++) {
			FtrNToIdV.Add(FtrId);
		}
	}
}

void TStreamStory::InitClust(const TUInt64V& TmV, const TFltVV& ObsFtrVV, const TFltVV& FtrVV,
		const TFltVV& IgnoredFtrVV, TIntV& AssignV) {
	StateIdentifier->Init(*this, TmV, ObsFtrVV, FtrVV, IgnoredFtrVV);
	StateIdentifier->Assign(TmV, ObsFtrVV, AssignV);
}

void TStreamStory::InitMChain(const TFltVV& FtrVV, const TIntV& AssignV,
		const TUInt64V& RecTmV, const bool IsBatchData, const TBoolV& EndBatchV) {
	MChain->Init(FtrVV, StateIdentifier->GetStates(), AssignV, RecTmV, IsBatchData, EndBatchV);
}

void TStreamStory::InitHierarch(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV) {
	Hierarch->Init(RecTmV, ObsFtrVV, MChain->GetCurrStateId(), *this);
}

void TStreamStory::InitHistograms(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
		const TFltVV& IgnoredFtrVV, const TUInt64V& RecTmV, const TBoolV& BatchEndV) {
	TFltVV FtrVV;	CreateFtrVV(ObsFtrVV, ContrFtrVV, RecTmV, BatchEndV, FtrVV);
	TIntV AssignV;	StateIdentifier->Assign(RecTmV, ObsFtrVV, AssignV);
	StateIdentifier->InitHistograms(*this, ObsFtrVV, FtrVV, IgnoredFtrVV, AssignV);
}

void TStreamStory::InitStateAssist(const TUInt64V& RecTmV, const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
		const TFltVV& IgnFtrVV, const bool& MultiThread) {
	StateAssist->Init(RecTmV, ObsFtrVV, ContrFtrVV, IgnFtrVV, *StateIdentifier, *Hierarch, Callback, MultiThread);
}

void TStreamStory::OnAddRec(const uint64& RecTm, const TFltV& ObsFtrV,
		const TFltV& ContrFtrV) {
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV, false);
	TFltV FtrV;	CreateFtrV(ObsFtrV, ContrFtrV, RecTm, FtrV);

	const int OldStateId = MChain->GetCurrStateId();
	const int NewStateId = StateIdentifier->Assign(RecTm, ObsFtrV);

	DetectAnomalies(OldStateId, NewStateId, ObsFtrV, FtrV);

	if (NewStateId != -1) {
		MChain->OnAddRec(NewStateId, RecTm, false);

		if (NewStateId != OldStateId && Callback != nullptr) {
			PredictTargets(RecTm, StateFtrVV, NewStateId);
			Hierarch->UpdateHistory(NewStateId);
			ActivityDetector->OnStateChanged(RecTm, NewStateId);

			TIntFltPrV CurrStateV;	GetCurrStateAncestry(CurrStateV);
			Callback->OnStateChanged(RecTm, CurrStateV);
		}
	}

	LastObsFtrV = ObsFtrV;
	LastContrFtrV = ContrFtrV;
	LastStateId = NewStateId;
	LastRecTm = RecTm;
}

void TStreamStory::GetFutStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	EAssertR(Tm >= 0, "Time should be greater than 0!");

	try {
		TAggStateV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetFutureProbV(StateSetV, StateFtrVV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetFutStateProbs: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetPastStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	try {
		TAggStateV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetPastProbV(StateSetV, StateFtrVV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetPastStateProbV: Failed to compute past state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetNextStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TAggStateV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetNextStateProbV(StateSetV, StateFtrVV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetNextStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetPrevStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TAggStateV StateSetV;
		TStateIdV StateIdV;
		TStateFtrVV StateFtrVV;

		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetPrevStateProbV(StateSetV, StateFtrVV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetPrevStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetProbVAtTime(const int& StartStateId, const double& Height, const double& Time,
		TIntV& StateIdV, TFltV& ProbV) const {
	try {
		TAggStateV StateSetV;
		TStateFtrVV StateFtrVV;
		GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
		MChain->GetProbVAtTime(StateSetV, StateFtrVV, StateIdV, StartStateId, Time, ProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetProbVAtTime: Failed to compute probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const {
	try {
		Hierarch->GetHistStateIdV(Height, StateIdV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetHistStateIdV: Failed to compute fetch historical states: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::PredictNextState(const bool& UseFtrVP, const int& FutStateN,
		TVec<TPair<TFlt, TIntFltPrV>>& HeightStateIdProbPrVPrV) const {
	EAssert(LastStateId >= 0);

	if (!HeightStateIdProbPrVPrV.Empty()) { HeightStateIdProbPrVPrV.Clr(); }

	TStateFtrVV StateFtrVV;
	GetStateFtrVV(StateFtrVV, UseFtrVP);

	const TFltV& HeightV = Hierarch->GetUiHeightV();
	for (int HeightN = 0; HeightN < HeightV.Len()-1; HeightN++) {
		const double& Height = HeightV[HeightN];

		HeightStateIdProbPrVPrV.Add(TPair<TFlt,TIntFltPrV>());
		HeightStateIdProbPrVPrV.Last().Val1 = Height;

		TIntV StateIdV;
		TAggStateV AggStateV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, AggStateV);
		const int CurrStateId = Hierarch->GetAncestorAtHeight(LastStateId, Height);

		MChain->GetNextStateProbV(AggStateV, StateFtrVV, StateIdV, CurrStateId, HeightStateIdProbPrVPrV.Last().Val2, FutStateN);
	}
}

void TStreamStory::GetHistogram(const int& StateId, const int& FtrId, TFltV& BinValV,
		TFltV& CountV, TFltV& AllCountV) const {
	try {
		TIntV LeafIdV;
		TIntV AllLeafIdV;

		Hierarch->GetLeafDescendantV(StateId, LeafIdV);
		Hierarch->GetLeafIdV(AllLeafIdV);

		StateIdentifier->GetHistogram(*this, FtrId, LeafIdV, BinValV, CountV, false);
		StateIdentifier->GetHistogram(*this, FtrId, AllLeafIdV, BinValV, AllCountV, false);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetTransitionHistogram(const int& SourceId, const int& TargetId,
		const int& FtrId, TFltV& BinValV, TFltV& SourceProbV, TFltV& TargetProbV,
		TFltV& AllProbV) const {
	try {
		TIntV SourceLeafIdV;
		TIntV TargetLeafIdV;
		TIntV AllLeafIdV;

		Hierarch->GetLeafDescendantV(SourceId, SourceLeafIdV);
		Hierarch->GetLeafDescendantV(TargetId, TargetLeafIdV);
		Hierarch->GetLeafIdV(AllLeafIdV);

		StateIdentifier->GetHistogram(*this, FtrId, SourceLeafIdV, BinValV, SourceProbV, false);
		StateIdentifier->GetHistogram(*this, FtrId, TargetLeafIdV, BinValV, TargetProbV, false);
		StateIdentifier->GetHistogram(*this, FtrId, AllLeafIdV, BinValV, AllProbV, false);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetTransitionHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetGlobalTimeHistogram(const int& StateId, TUInt64V& TmV, TFltV& ProbV,
		const int& NBins) const {
	TAggState AggState;
	Hierarch->GetLeafDescendantV(StateId, AggState);
	StateIdentifier->GetGlobalTimeHistogram(AggState, TmV, ProbV, NBins, false);
}

void TStreamStory::GetTimeHistogram(const int& StateId, const TStateIdentifier::TTmHistType& HistType,
		TIntV& BinV, TFltV& ProbV) const {
	TAggState AggState;
	Hierarch->GetLeafDescendantV(StateId, AggState);
	StateIdentifier->GetTimeHistogram(AggState, HistType, BinV, ProbV);
}

void TStreamStory::GetStateHistory(const double& RelOffset, const double& RelRange,
	        const int& MxStates,
	        TVec<TPair<TFlt, TVec<TTriple<TUInt64,TUInt64,TIntFltH>>>>& ScaleTmDurIdTrPrV,
	        uint64& MnTm, uint64& MxTm) const {
    Hierarch->GetStateHistory(RelOffset, RelRange, MxStates, ScaleTmDurIdTrPrV, MnTm, MxTm);
}

PJsonVal TStreamStory::GetStateWgtV(const int& StateId) const {
	PJsonVal Result = TJsonVal::NewArr();

	for (int FtrId = 0; FtrId < GetAllDim(); FtrId++) {
		const TFtrInfo& FtrInfo = GetFtrInfo(FtrId);
		const int& Offset = FtrInfo.GetOffset();
		const int& Length = FtrInfo.GetLength();

		TFltV WgtV;	StateAssist->GetFtrWgtV(StateId, Offset, Length, WgtV);

		PJsonVal FtrJson = TJsonVal::NewObj();
		FtrJson->AddToObj("type", FtrInfo.GetTypeStr());

		if (Length == 0) {
			FtrJson->AddToObj("value", WgtV[0]);
		} else {
			PJsonVal WgtJsonV = TJsonVal::NewArr();
			for (int BinN = 0; BinN < Length; BinN++) {
				WgtJsonV->AddToArr(WgtV[BinN]);
			}
			FtrJson->AddToObj("value", WgtJsonV);
		}

		Result->AddToArr(FtrJson);
	}

	return Result;
}

PJsonVal TStreamStory::GetStateClassifyTree(const int& StateId) const {
	PJsonVal TreeJson = StateAssist->GetStateClassifyTree(StateId);

	TransformExplainTree(TreeJson);

	return TreeJson;
}

PJsonVal TStreamStory::GetStateExplain(const int& StateId) const {
	PJsonVal UnionJson = StateAssist->GetStateExplain(StateId);

	Notify->OnNotify(ntInfo, "Transforming state explanation tree ...");

	for (int IntersectN = 0; IntersectN < UnionJson->GetArrVals(); IntersectN++) {
		PJsonVal IntersectJson = UnionJson->GetArrVal(IntersectN);
		PJsonVal TermsJson = IntersectJson->GetObjKey("terms");

		for (int TermN = 0; TermN < TermsJson->GetArrVals(); TermN++) {
			PJsonVal TermJson = TermsJson->GetArrVal(TermN);

			const int FtrN = TermJson->GetObjInt("ftrId");
			const int FtrId = GetFtrId(FtrN);
			const TFtrInfo& FtrInfo = GetFtrInfo(FtrId);

			TermJson->AddToObj("ftrType", FtrInfo.GetTypeStr());

			PJsonVal NewTermJson = TJsonVal::NewObj();

			NewTermJson->AddToObj("ftrId", FtrId);
			NewTermJson->AddToObj("ftrType", FtrInfo.GetTypeStr());

			const double& LessThan = TermJson->GetObjNum("le");
			const double& GreaterThan = TermJson->GetObjNum("gt");

			switch (FtrInfo.GetType()) {
			case ftNumeric: {
				NewTermJson->AddToObj("le", LessThan);
				NewTermJson->AddToObj("gt", GreaterThan);
				break;
			}
			case ftCategorical: {
				const int BinN = FtrN - FtrInfo.GetOffset();

				if (LessThan != TFlt::PInf) {
					NewTermJson->AddToObj("neq", BinN);
				}
				else if (GreaterThan != TFlt::NInf) {
					NewTermJson->AddToObj("eq", BinN);
				}
				else {
					throw TExcept::New("WTF!? the categorical description contains both le and gt!");
				}
				break;
			}
			case ftTime: {
			    // do nothing, this is not included in the narration
			    continue;
			}
			default: {
				throw TExcept::New("Invalid feature type: " + TInt::GetStr((int) FtrInfo.GetType()));
			}
			}

			TermsJson->SetArrVal(TermN, NewTermJson);
		}

	}

	return UnionJson;
}

void TStreamStory::GetTransitionModel(const double& Height, TFltVV& Mat) const {
	TStateIdV StateIdV;
	TAggStateV StateSetV;
	TStateFtrVV StateFtrVV;

	GetStatsAtHeight(Height, StateSetV, StateIdV, StateFtrVV);
	MChain->GetQMatrix(StateSetV, StateFtrVV, Mat);
}

void TStreamStory::GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const {
	Hierarch->GetAncestorV(StateId, StateIdHeightPrV);
}

void TStreamStory::GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const {
	Hierarch->GetUiCurrStateIdHeightPrV(StateIdHeightPrV);
}

int TStreamStory::GetCurrStateId(const double& Height) const {
	return Hierarch->GetAncestorAtHeight(MChain->GetCurrStateId(), Height);
}

void TStreamStory::GetCentroid(const int& StateId, const int& FtrSpaceN, TFltV& FtrV) const {
	TIntV LeafIdV;	Hierarch->GetLeafDescendantV(StateId, LeafIdV);

	StateIdentifier->GetJoinedCentroid(FtrSpaceN, LeafIdV, FtrV);
}

void TStreamStory::GetCentroidVV(const int& StateId, TVec<TFltV>& FtrVV) const {
	TAggState AggState;

	if (StateId >= 0) {
		Hierarch->GetLeafDescendantV(StateId, AggState);
	} else {
		Hierarch->GetLeafIdV(AggState);
	}

	FtrVV.Gen(AggState.Len());
	for (int StateN = 0; StateN < AggState.Len(); StateN++) {
		const int LeafStateId = AggState[StateN];
		StateIdentifier->GetAllCentroid(LeafStateId, FtrVV[StateN]);
	}
}

void TStreamStory::GetStateIdVAtHeight(const double& Height, TStateIdV& StateIdV) const {
	try {
		TAggStateV StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::TStreamStory::GetStateIdVAtHeight: Failed to fetch state IDs for height %.3f: %s", Height, Except->GetMsgStr().CStr());
		throw Except;
	}
}

uint64 TStreamStory::GetTimeUnit() const {
	return MChain->GetTimeUnit();
}

void TStreamStory::SetTargetState(const int& StateId, const bool& IsTrg) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Setting target state %d, isTarget: %s", StateId, TBool::GetStr(IsTrg).CStr());

	if (IsTrg) {
		Hierarch->SetTarget(StateId);
	} else {
		Hierarch->RemoveTarget(StateId);
	}
}

void TStreamStory::AddActivity(const TStr& ActName, const TVec<TIntV>& StateIdSeqVV) {
	// find the leafs of each state
	TVec<TIntSet> LeafStateIdSeqVV(StateIdSeqVV.Len());

	for (int StepN = 0; StepN < StateIdSeqVV.Len(); StepN++) {
		const TIntV& Step = StateIdSeqVV[StepN];
		for (int StateN = 0; StateN < Step.Len(); StateN++) {
			const int StateId = Step[StateN];

			TIntV LeafIdV;	Hierarch->GetLeafDescendantV(StateId, LeafIdV);
			for (int LeafN = 0; LeafN < LeafIdV.Len(); LeafN++) {
				LeafStateIdSeqVV[StepN].AddKey(LeafIdV[LeafN]);
			}
		}
	}


	ActivityDetector->AddActivity(ActName, LeafStateIdSeqVV);
}

void TStreamStory::RemoveActivity(const TStr& ActNm) {
	ActivityDetector->RemoveActivity(ActNm);
}

void TStreamStory::GetActivities(TStrV& ActNmV, TIntV& NumStepsV) const {
	ActivityDetector->GetActivities(ActNmV, NumStepsV);
}

bool TStreamStory::IsLeaf(const int& StateId) const {
	return Hierarch->IsLeaf(StateId);
}

void TStreamStory::SetStateNm(const int& StateId, const TStr& StateNm) {
	try {
		Hierarch->SetStateNm(StateId, StateNm);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetStateNm: Failed to set name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetStatePosV(const TFltPrV& PosV) {
	try {
		UiHelper->SetStateCoords(PosV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetStatePosV: Failed to set state positions: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetControlFtrVal(const int& StateId, const int& FtrId, const double& Val) {
	try {
		const int FtrN = FtrId - GetObsDim();
		const TFtrInfo& FtrInfo = ContrFtrInfoV[FtrN];
		StateIdentifier->SetControlFtr(StateId, FtrInfo, Val);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetControlFtrFactor: Failed to set factor: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetControlFtrsVal(const int& FtrId, const double& Val) {
	const int States = StateIdentifier->GetStates();
	for (int StateId = 0; StateId < States; StateId++) {
		SetControlFtrVal(StateId, FtrId, Val);
	}
}

void TStreamStory::ResetControlFtrVal(const int& StateId, const int& FtrId) {
	try {
		const int FtrN = FtrId - GetObsDim();
		const TFtrInfo& FtrInfo = ContrFtrInfoV[FtrN];

		StateIdentifier->ClearControlFtr(StateId, FtrInfo);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetControlFtrFactor: Failed to set factor: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::ResetControlFtrVals(const int& StateId) {
	const int ObsDim = GetObsDim();
	const int Dim = GetContrDim();
	for (int FtrId = 0; FtrId < Dim; FtrId++) {
		ResetControlFtrVal(StateId, FtrId + ObsDim);
	}
}

void TStreamStory::ResetControlFtrVals() {
	StateIdentifier->ClearControlFtrVV();
}

bool TStreamStory::IsAnyControlFtrSet() const {
	return StateIdentifier->IsAnyControlFtrSet(ContrFtrInfoV);
}

const TFltPr& TStreamStory::GetFtrBounds(const int& FtrId) const {
	try {
		return StateAssist->GetFtrBounds(FtrId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetControlFtrBounds: Failed to get feature bounds: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

const TStr& TStreamStory::GetStateLabel(const int& StateId) const {
	try {
		return Hierarch->GetStateLabel(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateLabel: Failed to get label of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

const TUiHelper::PAutoNmDesc& TStreamStory::GetStateAutoNm(const int& StateId) const {
	return UiHelper->GetStateAutoNm(StateId);
}

void TStreamStory::GetStateFtrPValDesc(const int& StateId, TUiHelper::PAutoNmDescV& DescV) const {
	UiHelper->GetAutoNmPValDesc(StateId, DescV);
}

void TStreamStory::GetStateTmDesc(const int& StateId, TStrPrV& StateIntervalV) const {
	UiHelper->GetTmDesc(StateId, StateIntervalV);
}

const TStr& TStreamStory::GetStateNm(const int& StateId) const {
	try {
		return Hierarch->GetStateNm(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateNm: Failed to get name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}

	StateIdentifier->SetVerbose(Verbose);
	MChain->SetVerbose(Verbose);
	Hierarch->SetVerbose(Verbose);
	ActivityDetector->SetVerbose(Verbose);
}

void TStreamStory::SetCallback(TStreamStoryCallback* _Callback) {
	Callback = _Callback;
	ActivityDetector->SetCallback(Callback);
}

int TStreamStory::GetFtrId(const int& FtrN) const {
	EAssert(0 <= FtrN && FtrN < FtrNToIdV.Len());
	return FtrNToIdV[FtrN];
}

const TFtrInfo& TStreamStory::GetFtrInfo(const int& FtrId) const {
	if (FtrId < GetObsDim()) {
		return ObsFtrInfoV[FtrId];
	}
	else if (FtrId < GetContrDim()) {
		return ContrFtrInfoV[FtrId - GetObsDim()];
	}
	else {
		return IgnFtrInfoV[FtrId - GetObsDim() - GetContrDim()];
	}
}

bool TStreamStory::IsDetectingActivities() const {
	return !ActivityDetector->IsEmpty();
}

bool TStreamStory::IsPredictingStates() const {
	return Hierarch->HasTargetStates();
}

PJsonVal TStreamStory::GetLevelJson(const double& Height, const double& NextHeight,
		const TStateIdV& StateIdV, const TFltVV& TransitionVV, const TFltV& HoldingTimeV,
		const TFltV& ProbV, const TFltV& RadiusV) const {

	PJsonVal Result = TJsonVal::NewObj();

	PJsonVal StateJsonV = TJsonVal::NewArr();
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		const int StateId = StateIdV[StateN];
		const int ParentId = NextHeight != TFlt::PInf ? Hierarch->GetAncestorAtHeight(StateId, NextHeight) : StateId;
		const TFltPr& StateCoords = UiHelper->GetStateCoords(StateId);

		PJsonVal StateJson = TJsonVal::NewObj();
		StateJson->AddToObj("id", StateId);
		StateJson->AddToObj("parentId", ParentId);
		StateJson->AddToObj("x", StateCoords.Val1);
		StateJson->AddToObj("y", StateCoords.Val2);
		StateJson->AddToObj("radius", RadiusV[StateN]);
		StateJson->AddToObj("timeProportion", ProbV[StateN]);
		StateJson->AddToObj("holdingTime", HoldingTimeV[StateN]);
		StateJson->AddToObj("isTarget", Hierarch->IsTarget(StateId));
		StateJson->AddToObj("label", Hierarch->GetStateLabel(StateId));

		const TUiHelper::PAutoNmDesc& AutoNmDesc = UiHelper->GetStateAutoNm(StateId);
		StateJson->AddToObj("autoName", AutoNmDesc->GetJson());

		if (Hierarch->IsStateNm(StateId)) {
			StateJson->AddToObj("name", Hierarch->GetStateNm(StateId));
		}

		StateJsonV->AddToArr(StateJson);
	}

	// construct transition JSON
	PJsonVal JumpMatJson = TJsonVal::NewArr();
	for (int RowIdx = 0; RowIdx < TransitionVV.GetRows(); RowIdx++) {
		PJsonVal RowJson = TJsonVal::NewArr();

		for (int ColIdx = 0; ColIdx < TransitionVV.GetCols(); ColIdx++) {
			RowJson->AddToArr(TransitionVV(RowIdx, ColIdx));
		}

		JumpMatJson->AddToArr(RowJson);
	}

	Result->AddToObj("height", Height);
	Result->AddToObj("states", StateJsonV);
	Result->AddToObj("transitions", JumpMatJson);

	return Result;
}

void TStreamStory::CreateFtrVV(const TFltVV& ObsFtrMat, const TFltVV& ContrFtrMat,
		const TUInt64V& RecTmV, const TBoolV& EndsBatchV, TFltVV& FtrVV) const {
	const int NInst = ObsFtrMat.GetCols();
//	const bool HasHiddenState = !EndsBatchV.Empty();

	FtrVV.Gen(ContrFtrMat.GetRows(), NInst);
	for (int ColN = 0; ColN < NInst; ColN++) {
		for (int RowN = 0; RowN < ContrFtrMat.GetRows(); RowN++) {
			FtrVV(RowN, ColN) = ContrFtrMat(RowN, ColN);
		}
	}
}

void TStreamStory::CreateFtrV(const TFltV& ObsFtrV, const TFltV& ContrFtrV,
		const uint64& RecTm, TFltV& FtrV) const {
	FtrV = ContrFtrV;
}

void TStreamStory::GetStateFtrVV(TStateFtrVV& StateFtrVV, const bool& UseFtrVP) const {
	StateIdentifier->GetControlCentroidVV(*this, StateFtrVV);

	if (UseFtrVP && LastStateId != -1) {
		TFltV& LastStateFtrV = StateFtrVV[LastStateId];
		for (int FtrN = 0; FtrN < LastContrFtrV.Len(); FtrN++) {
			LastStateFtrV[FtrN] = LastContrFtrV[FtrN];
		}
	}
}

void TStreamStory::GetStatsAtHeight(const double& Height, TAggStateV& StateSetV,
		TStateIdV& StateIdV, TStateFtrVV& StateFtrVV) const {
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	GetStateFtrVV(StateFtrVV, false);
}

void TStreamStory::DetectAnomalies(const int& NewStateId, const int& OldStateId,
		const TFltV& ObsFtrV, const TFltV& FtrV) const {
	if (NewStateId != OldStateId) {
		if (MChain->IsAnomalousJump(FtrV, NewStateId, OldStateId)) {
			Callback->OnAnomaly(TStr::Fmt("Anomalous jump, old state %d, new state %d", OldStateId, NewStateId));
		}
		if (NewStateId == -1) {
			Callback->OnOutlier(ObsFtrV);
		}
	}
}

void TStreamStory::PredictTargets(const uint64& RecTm, const TStateFtrVV& StateFtrVV,
		const int& CurrLeafId) const {
	const TIntFltPrSet& TargetIdHeightSet = Hierarch->GetTargetStateIdSet();

	double Prob;
	TFltV ProbV, TmV;

	TAggStateV StateSetV;
	TStateIdV StateIdV;

	int KeyId = TargetIdHeightSet.FFirstKeyId();
	while (TargetIdHeightSet.FNextKeyId(KeyId)) {
		const TIntFltPr& TrgIdHeightPr = TargetIdHeightSet.GetKey(KeyId);

		const int& TrgStateId = TrgIdHeightPr.Val1;
		const double& Height = TrgIdHeightPr.Val2;
		const int CurrStateId = Hierarch->GetAncestorAtHeight(CurrLeafId, Height);

		Prob = 0;
		ProbV.Clr();
		TmV.Clr();
		StateSetV.Clr();
		StateIdV.Clr();

		// get the state sets
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);

		bool WillOccur = MChain->PredictOccurenceTime(StateFtrVV, StateSetV, StateIdV,
				CurrStateId, TrgStateId, Prob, ProbV, TmV);

		if (WillOccur) {
			Callback->OnPrediction(RecTm, CurrStateId, TrgStateId, Prob, ProbV, TmV);
		}
	}
}

void TStreamStory::CheckBatches(const TUInt64V& TmV, const TBoolV& BatchEndV) const {
	Notify->OnNotify(TNotifyType::ntInfo, "Checking batches ...");

	// check if any batches of length 0 exist and the times are OK
	bool JustEnded = false;
	uint64 PrevTm = 0;

	for (int i = 0; i < BatchEndV.Len(); i++) {
		const bool EndsBatch = BatchEndV[i];
		const uint64 Tm = TmV[i];

		EAssertR(!EndsBatch || !JustEnded, "Found a batch of length 0. Cannot model such data! Index: " + TInt::GetStr(i));
		EAssertR(Tm >= PrevTm || JustEnded, "Time less then previous time even though the batch hasn't ended! Index: " + TInt::GetStr(i));

		JustEnded = EndsBatch;
		PrevTm = Tm;
	}

	Notify->OnNotify(TNotifyType::ntInfo, "The batches are OK!");
}

void TStreamStory::TreeLayout(const TFltVV& PMat, const TIntV& StateIdV, const TFltV& RadiusV,
		TFltPrV& PosV) const {
	const int RootN = 0;
	const double NodePadding = .05;

	PosV.Gen(PMat.GetCols());
	TFltV WidthV(PMat.GetCols());

	CalcTreeWidthV(PMat, RadiusV, NodePadding, RootN, WidthV);
	CalcTreePosV(PMat, RootN, RadiusV, WidthV, 0.0, 0.0, PosV);
}

void TStreamStory::CalcTreeWidthV(const TFltVV& PMat, const TFltV& RadiusV,
		const double& NodePadding, const int& RootN, TFltV& WidthV) const {
	const int NStates = PMat.GetCols();

	// construct the vector of children
	TIntV ChildNV;
	for (int StateN = 0; StateN < NStates; StateN++) {
		if (PMat(RootN, StateN) > 0) {
			ChildNV.Add(StateN);
		}
	}

	const bool IsLeaf = ChildNV.Empty();
	if (IsLeaf) {
		WidthV[RootN] = 3*RadiusV[RootN];// + NodePadding);	TODO remove node padding
	} else {
		double NodeW = 0;
		for (int StateN = 0; StateN < ChildNV.Len(); StateN++) {
			const int ChildN = ChildNV[StateN];
			CalcTreeWidthV(PMat, RadiusV, NodePadding, ChildN, WidthV);
			NodeW += WidthV[ChildN];// + 2*NodePadding;
		}
		WidthV[RootN] = NodeW;
	}
}

void TStreamStory::CalcTreePosV(const TFltVV& PMat, const int& RootN, const TFltV& RadiusV,
		const TFltV& WidthV, const double& RootX, const double& RootY, TFltPrV& PosV) const {
	const int NStates = PMat.GetCols();

	// set root position
	PosV[RootN].Val1 = RootX;
	PosV[RootN].Val2 = RootY;

	// find & layout the children
	TIntV ChildNV;
	for (int StateN = 0; StateN < NStates; StateN++) {
		if (PMat(RootN,StateN) > 0) {
			ChildNV.Add(StateN);
		}
	}

	const double StartX = RootX - WidthV[RootN] / 2;
	double WidthSum = 0;
	for (int StateN = 0; StateN < ChildNV.Len(); StateN++) {
		const int ChildN = ChildNV[StateN];

		const double ChildWidth = WidthV[ChildN];
		const double ChildY = RootY + 2*(RadiusV[RootN] + RadiusV[ChildN]);
		const double ChildX = StartX + WidthSum + .5*ChildWidth;

		CalcTreePosV(PMat, ChildN, RadiusV, WidthV, ChildX, ChildY, PosV);

		WidthSum += ChildWidth;
	}
}

void TStreamStory::TransformExplainTree(PJsonVal& RootJson) const {
	PJsonVal ChildrenJson = RootJson->GetObjKey("children");
	for (int ChildN = 0; ChildN < ChildrenJson->GetArrVals(); ChildN++) {
		PJsonVal ChildJson = ChildrenJson->GetArrVal(ChildN);
		TransformExplainTree(ChildJson);
	}

	if (RootJson->IsObjKey("cut")) {
		PJsonVal CutJson = RootJson->GetObjKey("cut");

		const int FtrN = CutJson->GetObjInt("id");

		const int FtrId = GetFtrId(FtrN);
		const TFtrInfo& FtrInfo = GetFtrInfo(FtrId);

		CutJson->AddToObj("id", FtrId);
		CutJson->AddToObj("ftrType", FtrInfo.GetTypeStr());

		switch (FtrInfo.GetType()) {
		case ftNumeric: {
			// do nothing
			break;
		}
		case ftCategorical: {
			// transform
			const int FtrVal = FtrN - FtrInfo.GetOffset();

			PJsonVal ValJson = TJsonVal::NewArr();
			for (int BinN = 0; BinN < FtrInfo.GetLength(); BinN++) {
				ValJson->AddToArr(BinN == FtrVal ? 1 : 0);
			}

			CutJson->AddToObj("value", ValJson);
			break;
		}
		case ftTime: {
		    throw TExcept::New("Time feature not supported in explanation tree");
		}
		default: {
			throw TExcept::New("Invalid feature type: " + TInt::GetStr((int) FtrInfo.GetType()));
		}
		}
	}
}

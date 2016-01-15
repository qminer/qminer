/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TMc;

//////////////////////////////////////////////////
// Histogram class
THistogram::THistogram():
		Bins(),
		TotalCount(),
		CountV(),
		BinStartV() {}

THistogram::THistogram(const int& NBins, const double& MnVal, const double& MxVal):
		Bins(NBins),
		TotalCount(),
		CountV(NBins+2),
		BinStartV(NBins+1) {

	const double Span = MxVal - MnVal;
	const double BinSize = Span / NBins;
	for (int i = 0; i < NBins+1; i++) {
		BinStartV[i] = MnVal + i*BinSize;
	}
}

THistogram::THistogram(TSIn& SIn):
		Bins(SIn),
		TotalCount(SIn),
		CountV(SIn),
		BinStartV(SIn) {}

void THistogram::Save(TSOut& SOut) const {
	Bins.Save(SOut);
	TotalCount.Save(SOut);
	CountV.Save(SOut);
	BinStartV.Save(SOut);
}

void THistogram::Update(const double& FtrVal) {
	// determine the bin
	int BinN = 0;
	if (FtrVal >= BinStartV[Bins]) { BinN = CountV.Len()-1; }
	else {
		while (BinN < BinStartV.Len() && FtrVal >= BinStartV[BinN]) {
			BinN++;
		}
	}
	CountV[BinN]++;
	TotalCount++;
}



//////////////////////////////////////////////////
// Abstract clustering
const int TStateIdentifier::MX_ITER = 10000;

TStateIdentifier::TStateIdentifier(const PClust& _KMeans, const int _NHistBins,
			const double& _Sample, const TRnd& _Rnd, const bool& _Verbose):
		Rnd(_Rnd),
		KMeans(_KMeans),
		CentroidDistStatV(),
		NHistBins(_NHistBins),
		StateContrFtrValVV(),
		Sample(_Sample),
		Verbose(_Verbose),
		Notify(Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(NHistBins >= 2, "Should have at least 2 bins for the histogram!");
}

TStateIdentifier::TStateIdentifier(TSIn& SIn):
	Rnd(SIn),
	KMeans(TClust::Load(SIn)),
	ControlCentroidMat(SIn),
	CentroidDistStatV(SIn),
	NHistBins(TInt(SIn)),
	ObsHistVV(SIn),
	ControlHistVV(SIn),
	TransHistMat(SIn),
	StateContrFtrValVV(SIn),
	Sample(TFlt(SIn)),
	Verbose(TBool(SIn)) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStateIdentifier::Save(TSOut& SOut) const {
	Rnd.Save(SOut);
	KMeans->Save(SOut);
	ControlCentroidMat.Save(SOut);
	CentroidDistStatV.Save(SOut);
	TInt(NHistBins).Save(SOut);
	ObsHistVV.Save(SOut);
	ControlHistVV.Save(SOut);
	TransHistMat.Save(SOut);
	StateContrFtrValVV.Save(SOut);
	TFlt(Sample).Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TStateIdentifier::Init(TFltVV& ObsFtrVV, const TFltVV& ControlFtrVV) {
	EAssertR(Sample >= 0, "Cannot sample a negative number of instances!");

	const int NInst = ObsFtrVV.GetCols();

	Notify->OnNotify(TNotifyType::ntInfo, "Clustering ...");

	if (Sample == 1) {
		KMeans->Apply(ObsFtrVV, MX_ITER, Notify);
	} else {
		const int NSamples = Sample < 1 ? (int)ceil(NInst*Sample) : TMath::Mn(NInst, int(Sample));

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Sampling %d instances...", NSamples);

		TIntV SampleV(NInst, 0);
		for (int i = 0; i < NInst; i++) {
			SampleV.Add(i);
		}

		SampleV.Shuffle(Rnd);
		SampleV.Trunc(NSamples);

		KMeans->Apply(TFullMatrix(ObsFtrVV, true)(TVector::Range(ObsFtrVV.GetRows()), SampleV).GetMat(), MX_ITER, Notify);	// TODO remove TFullMatrix
	}

	InitStatistics(ObsFtrVV);

	InitControlCentroids(ObsFtrVV, ControlFtrVV);
	InitHistograms(ObsFtrVV, ControlFtrVV);
	ClearControlFtrVV(ControlFtrVV.GetRows());

	Notify->OnNotify(TNotifyType::ntInfo, "Done.");
}

void TStateIdentifier::InitHistograms(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV) {
	Notify->OnNotify(TNotifyType::ntInfo, "Computing histograms ...");

	const int NClusts = GetStates();

	TIntV AssignV;	Assign(ObsFtrVV, AssignV);

	InitHists(ObsFtrVV, ContrFtrVV);

	UpdateHist(ObsFtrVV, AssignV, NClusts, ObsHistVV);
	UpdateHist(ContrFtrVV, AssignV, NClusts, ControlHistVV);
	UpdateTransitionHist(ObsFtrVV, ContrFtrVV, AssignV);
}

int TStateIdentifier::Assign(const TFltV& x) const {
	TFltV DistV;	GetCentroidDistV(x, DistV);
	return TLAMisc::GetMinIdx(DistV);
}

void TStateIdentifier::Assign(const TFltVV& FtrVV, TIntV& AssignV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Assigning %d instances ...", FtrVV.GetCols());
	KMeans->Assign(FtrVV, AssignV);
}

void TStateIdentifier::GetCentroidDistV(const TFltV& FtrVV, TFltV& DistV) const {
	KMeans->GetCentroidDistV(FtrVV, DistV);
}

double TStateIdentifier::GetDist(const int& StateId, const TFltV& FtrV) const {
	return KMeans->GetDist(StateId, FtrV);
}

void TStateIdentifier::GetJoinedCentroid(const TIntV& CentroidIdV, TFltV& Centroid) const {
	const int Dim = GetDim();

	if (Centroid.Empty()) { Centroid.Gen(Dim); }
	EAssert(Centroid.Len() == Dim);

	TFltV FtrV;

	double TotalSize = 0;
	for (int i = 0; i < CentroidIdV.Len(); i++) {
		const int CentroidIdx = CentroidIdV[i];
		const uint64 CentroidSize = GetClustSize(CentroidIdx);

		GetCentroid(CentroidIdx, FtrV);

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			Centroid[FtrN] += FtrV[FtrN] * CentroidSize;
		}

		TotalSize += CentroidSize;
	}

	for (int i = 0; i < Dim; i++) {
		Centroid[i] /= TotalSize;
	}
}

void TStateIdentifier::GetJoinedControlCentroid(const TIntV& CentroidIdV, TFltV& Centroid) const {
	const int Dim = GetControlDim();

	if (Centroid.Empty()) { Centroid.Gen(Dim); }
	EAssert(Centroid.Len() == Dim);

	TFltV FtrV;

	double TotalSize = 0;
	for (int i = 0; i < CentroidIdV.Len(); i++) {
		const int CentroidIdx = CentroidIdV[i];
		const uint64 CentroidSize = GetClustSize(CentroidIdx);

		GetControlCentroid(CentroidIdx, FtrV);

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			Centroid[FtrN] += FtrV[FtrN] * CentroidSize;
		}

		TotalSize += CentroidSize;
	}

	for (int i = 0; i < Dim; i++) {
		Centroid[i] /= TotalSize;
	}
}

double TStateIdentifier::GetMeanPtCentDist(const int& CentroidIdx) const {
	EAssertR(CentroidIdx < GetStates(), TStr::Fmt("TFullKMeans::GetMeanPtCentDist: Invalid centroid index: %d", CentroidIdx));
	return CentroidDistStatV[CentroidIdx].Val2 / CentroidDistStatV[CentroidIdx].Val1;
}

uint64 TStateIdentifier::GetClustSize(const int& ClustIdx) const {
	return CentroidDistStatV[ClustIdx].Val1;
}

void TStateIdentifier::GetHistogram(const int& FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const {
	EAssertR(FtrId < GetDim() + GetControlDim(), "Invalid feature ID: " + TInt::GetStr(FtrId));

	BinV.Gen(NHistBins+2);

	const int FtrN = FtrId < GetDim() ? FtrId : FtrId - GetDim();
	const TStateFtrHistVV& StateIdFtrIdHistVV = FtrId < GetDim() ? ObsHistVV : ControlHistVV;

	// go through all the features and extract the bin start vectors

	BinStartV = StateIdFtrIdHistVV[0][FtrN].GetBinStartV();

	for (int i = 0; i < StateSet.Len(); i++) {
		const int StateId = StateSet[i];
		const THistogram& Hist = StateIdFtrIdHistVV[StateId][FtrN];
		const TIntV& CountV = Hist.GetCountV();

		for (int j = 0; j < CountV.Len(); j++) {
			BinV[j] += (double) CountV[j];
		}
	}

	TLinAlg::NormalizeL1(BinV);
}

void TStateIdentifier::GetTransitionHistogram(const int& FtrId, const TIntV& SourceStateSet,
		const TIntV& TargetStateSet, TFltV& BinStartV, TFltV& ProbV) const {
	EAssertR(FtrId < GetDim() + GetControlDim(), "Invalid feature ID: " + TInt::GetStr(FtrId));

	ProbV.Gen(NHistBins+2);
	BinStartV.Clr();

	for (int SourceN = 0; SourceN < SourceStateSet.Len(); SourceN++) {
		const int& SourceId = SourceStateSet[SourceN];

		for (int TargetN = 0; TargetN < TargetStateSet.Len(); TargetN++) {
			const int& TargetId = TargetStateSet[TargetN];

			const TFtrHistV& FtrIdHistV = TransHistMat(SourceId, TargetId);
			EAssertR(FtrId < FtrIdHistV.Len(), "Invalid feature ID: " + TInt::GetHexStr(FtrId));
			const THistogram& Hist = FtrIdHistV[FtrId];

			if (Hist.Empty()) { continue; }

			const TIntV& CountV = Hist.GetCountV();
			for (int i = 0; i < CountV.Len(); i++) {
				ProbV[i] += (double) CountV[i];
			}

			// initialize the bin start vector if not already
			// try to think of anything better than lazy initialization
			if (BinStartV.Empty()) {
				BinStartV = Hist.GetBinStartV();
			}
		}
	}

	TLinAlg::NormalizeL1(ProbV);
}

void TStateIdentifier::GetCentroidVV(TVec<TFltV>& ResultVV) const {
	const TFltVV& CentroidMat = KMeans->GetCentroidVV();
	const int Cols = CentroidMat.GetCols();

	ResultVV.Gen(Cols);

	for (int ColN = 0; ColN < Cols; ColN++) {
		CentroidMat.GetCol(ColN, ResultVV[ColN]);
	}
}

void TStateIdentifier::GetControlCentroidVV(TStateFtrVV& StateFtrVV) const {
	const int Cols = ControlCentroidMat.GetCols();
	const int Rows = ControlCentroidMat.GetRows();

	StateFtrVV.Gen(Cols);

	for (int StateId = 0; StateId < Cols; StateId++) {
		StateFtrVV[StateId].Gen(Rows);
		for (int FtrId = 0; FtrId < Rows; FtrId++) {
			const double& FtrVal = GetControlFtr(StateId, FtrId, ControlCentroidMat(FtrId, StateId));
			StateFtrVV[StateId][FtrId] = FtrVal;
		}
	}
}

double TStateIdentifier::GetControlFtr(const int& StateId, const int& FtrId, const double& DefaultVal) const {
	const bool IsSet = IsControlFtrSet(StateId, FtrId);
	return IsSet ? GetControlFtr(StateId, FtrId) : DefaultVal;
}

void TStateIdentifier::SetControlFtr(const int& StateId, const int& FtrId, const double& Val) {
	EAssertR(StateId < StateContrFtrValVV.Len(), "Invalid state ID when setting feature!");
	EAssertR(FtrId < StateContrFtrValVV[StateId].Len(), "Invalid feature ID when setting feature!");
	StateContrFtrValVV[StateId][FtrId] = Val;
}

void TStateIdentifier::ClearControlFtr(const int& StateId, const int& FtrId) {
	SetControlFtr(StateId, FtrId, TFlt::PInf);
}

void TStateIdentifier::ClearControlFtrVV() {
	ClearControlFtrVV(StateContrFtrValVV[0].Len());
}

bool TStateIdentifier::IsControlFtrSet(const int& StateId, const int& FtrId) const {
	return GetControlFtr(StateId, FtrId) != TFlt::PInf;
}

bool TStateIdentifier::IsAnyControlFtrSet() const {
	const int States = GetStates();
	const int Dim = GetControlDim();

	for (int StateId = 0; StateId < States; StateId++) {
		for (int FtrId = 0; FtrId < Dim; FtrId++) {
			if (IsControlFtrSet(StateId, FtrId)) {
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

//void TStateIdentifier::Assign(const TFltVV& X, const TFltV& NormX2, const TFltV& NormC2,
//		TIntV& AssignV) const {
////	return GetDistMat2(X, NormX2, CentroidMat.ColNorm2V().Transpose(), OnesN, OnesK).GetColMinIdxV();
//	TFltVV DistVV;	GetDistMat2(X, NormX2, NormC2, DistVV);
//	TLinAlg::GetColMinIdxV(DistVV, AssignV);
//}

void TStateIdentifier::InitStatistics(const TFltVV& X) {
	TIntV AssignV;	KMeans->Assign(X, AssignV);

	const int K = GetStates();
	TFltVV DistMat;	KMeans->GetDistVV(X, DistMat);

	TVector OnesDim = TVector::Ones(X.GetRows());

	CentroidDistStatV.Gen(K,K);
	for (int ClustN = 0; ClustN < K; ClustN++) {
		TIntV VCellIdxV;	AssignV.FindAll(ClustN, VCellIdxV);
		TVector Di = TFullMatrix(DistMat, true)(ClustN, VCellIdxV);	// TODO remove TFullMatrix

		CentroidDistStatV[ClustN].Val1 = VCellIdxV.Len();
		CentroidDistStatV[ClustN].Val2 = Di.Sum();
	}
}

void TStateIdentifier::InitControlCentroids(const TFltVV& ObsMat,  const TFltVV& ControlFtrVV) {
	const int NCentroids = GetStates();
	const int Dim = ControlFtrVV.GetRows();
	const int NInst = ControlFtrVV.GetCols();

	ControlCentroidMat.Gen(Dim, NCentroids);
	TUInt64V CountV(NCentroids);

	TIntV AssignV;	Assign(ObsMat, AssignV);
	for (int i = 0; i < NInst; i++) {
		CountV[AssignV[i]]++;
	}

	for (int ColN = 0; ColN < NInst; ColN++) {
		const int CentId = AssignV[ColN];
		for (int RowN = 0; RowN < Dim; RowN++) {
			ControlCentroidMat(RowN, CentId) += ControlFtrVV(RowN, ColN) / CountV[CentId];
		}
	}
}

void TStateIdentifier::GetCentroid(const int& StateId, TFltV& FtrV) const {
	EAssert(0 <= StateId && StateId < GetStates());
	KMeans->GetCentroid(StateId, FtrV);
}

void TStateIdentifier::GetControlCentroid(const int& StateId, TFltV& FtrV) const {
	EAssert(0 <= StateId && StateId < ControlCentroidMat.GetCols());
	ControlCentroidMat.GetCol(StateId, FtrV);
}

double TStateIdentifier::GetControlFtr(const int& StateId, const int& FtrId) const {
	EAssertR(StateId < StateContrFtrValVV.Len(), "Invalid state ID when fetching feature value!");
	EAssertR(FtrId < StateContrFtrValVV[StateId].Len(), "Invalid feature ID when fetching feature value!");
	return StateContrFtrValVV[StateId][FtrId];
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

void TStateIdentifier::InitHists(const TFltVV& ObsFtrVV, const TFltVV& ConstFtrVV) {
	const int States = GetStates();
	const int Bins = NHistBins;
	const int NInst = ObsFtrVV.GetCols();

	ObsHistVV.Gen(States);
	ControlHistVV.Gen(States);
	TransHistMat.Gen(States, States);

	for (int State1Id = 0; State1Id < States; State1Id++) {
		ObsHistVV[State1Id].Gen(ObsFtrVV.GetRows());
		ControlHistVV[State1Id].Gen(ConstFtrVV.GetRows());
		for (int State2Id = 0; State2Id < States; State2Id++) {
			TransHistMat(State1Id, State2Id).Gen(ObsFtrVV.GetRows() + ConstFtrVV.GetRows());
		}
	}

	// find the min and max value of each observation feature
	double MnVal, MxVal;
	for (int FtrN = 0; FtrN < ObsFtrVV.GetRows(); FtrN++) {
		// find min and max value
		MnVal = TFlt::Mx;
		MxVal = TFlt::Mn;

		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ObsFtrVV(FtrN, InstN) < MnVal) { MnVal = ObsFtrVV(FtrN, InstN); }
			if (ObsFtrVV(FtrN, InstN) > MxVal) { MxVal = ObsFtrVV(FtrN, InstN); }
		}

		// go through all the histograms and initialize them
		for (int State1Id = 0; State1Id < States; State1Id++) {
			ObsHistVV[State1Id].SetVal(FtrN, THistogram(Bins, MnVal, MxVal));
			for (int State2Id = 0; State2Id < States; State2Id++) {
				TransHistMat(State1Id, State2Id).SetVal(FtrN, THistogram(Bins, MnVal, MxVal));
			}
		}
	}

	// find the min and max value of each control feature
	for (int FtrN = 0; FtrN < ConstFtrVV.GetRows(); FtrN++) {
		// find min and max value
		MnVal = TFlt::Mx;
		MxVal = TFlt::Mn;

		for (int InstN = 0; InstN < NInst; InstN++) {
			if (ConstFtrVV(FtrN, InstN) < MnVal) { MnVal = ConstFtrVV(FtrN, InstN); }
			if (ConstFtrVV(FtrN, InstN) > MxVal) { MxVal = ConstFtrVV(FtrN, InstN); }
		}

		// go through all the histograms and initialize them
		for (int State1Id = 0; State1Id < States; State1Id++) {
			ControlHistVV[State1Id].SetVal(FtrN, THistogram(Bins, MnVal, MxVal));
			for (int State2Id = 0; State2Id < States; State2Id++) {
				TransHistMat(State1Id, State2Id).SetVal(ObsFtrVV.GetRows() + FtrN, THistogram(Bins, MnVal, MxVal));
			}
		}
	}
}

void TStateIdentifier::UpdateTransitionHist(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV,
			const TIntV& AssignV) {
	const int NInst = AssignV.Len();

	EAssertR(NInst > 0, "Cannot fit a model to 0 instances!");

	const int ObsDim = ObsFtrVV.GetRows();
	const int ContrDim = ContrFtrVV.GetRows();

	int CurrState, PrevState = AssignV[0];
	for (int InstN = 0; InstN < NInst; InstN++) {
		CurrState = AssignV[InstN];

		if (CurrState != PrevState) {	// we have just jumped
			// get the features of the previous time step
			const int JumpInst = InstN-1;
			TFtrHistV& FtrIdHistV = TransHistMat(PrevState, CurrState);

			for (int FtrN = 0; FtrN < ObsDim; FtrN++) {
				FtrIdHistV[FtrN].Update(ObsFtrVV(FtrN, JumpInst));
			}
			for (int FtrN = 0; FtrN < ContrDim; FtrN++) {
				FtrIdHistV[GetDim() + FtrN].Update(ContrFtrVV(FtrN, JumpInst));
			}

			PrevState = CurrState;
		}
	}
}

void TStateIdentifier::UpdateHist(const TFltVV& FtrVV, const TIntV& AssignV,
		const int& States, TStateFtrHistVV& StateFtrHistVV) {

	const int NInst = FtrVV.GetCols();
	const int Dim = FtrVV.GetRows();

	// update the histograms
	for (int InstN = 0; InstN < NInst; InstN++) {
		TFltV FtrV;	FtrVV.GetCol(InstN, FtrV);
		const int StateId = AssignV[InstN];

		TFtrHistV& FtrHistV = StateFtrHistVV[StateId];

		for (int FtrN = 0; FtrN < Dim; FtrN++) {
			FtrHistV[FtrN].Update(FtrVV(FtrN, InstN));
		}
	}
}

/////////////////////////////////////////////
// MDS
void TEuclMds::Project(const TFltVV& FtrVV, TFltVV& ProjVV, const int& d) {
	// first center the rows of matrix X
	TFullMatrix X1(FtrVV);	TLAUtil::CenterRows(X1.GetMat());

	// Let B = X'X, then we can decompose B into its spectral decomposition
	// B = V*L*V' where L is a diagonal matrix of eigenvalues, and A holds the
	// corresponding eigenvectors in its columns

	// we can now aaproximate B with just the highest 'd' eigenvalues and
	// eigenvectors: B_d = V_d * L_d * V_d'
	// the coordinates of X can then be recovered by: X_d = V_d*L_d^(.5)

	// se can use SVD do find the eigenvectors V_d and eigenvalues L_d
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
	TNumericalStuff::GetEigenVec(QMatT, 0.0, ProbV);

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
		for (int JoinState2Idx = 0; JoinState2Idx < NAggrStates; JoinState2Idx++) {
			if (JoinState1Idx == JoinState2Idx) { continue; }

			const TIntV& JoinState2 = AggrStateV[JoinState2Idx];

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
	TFltVV Pi;	TLAUtil::Diag(ProbV, Pi);
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
	TFltV ProbV;	TLAUtil::GetRow(ProbVV, StateIdx, ProbV);

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
	TFltV ProbV;	TLAUtil::GetRow(ProbVV, StateIdx, ProbV);

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
		TFltVV SubJumpVV;	TLAUtil::SubMat(JumpVV, 0, JumpVV.GetRows()-1, 0, JumpVV.GetCols()-1, SubJumpVV);
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
		MeanSampleInterval += double(DeltaTm) / TimeUnit;

		EAssertR(DeltaTm > 0, "Delta time is not positive!");

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

			if (LabelV.Empty() || TLinAlg::IsZero(LabelV)) {
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

void TCtmcModeller::GetNextStateProbV(const TFltVV& QMat, const TStateIdV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates,
		const PNotify& Notify) {

	const int Dim = QMat.GetRows();

	const int NFStates = TMath::Mn(NFutStates, Dim-1);
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
		TLAUtil::Identity(Dim, ProbVV);
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

		TLAUtil::SubMat(ProbMat, 0, Dim, 0, Dim, CurrProbMat);
		for (int RowIdx = 0; RowIdx < Dim; RowIdx++) {
			const double HiddenProb = ProbMat(RowIdx, Dim);
			CurrProbMat(RowIdx, RowIdx) += HiddenProb;
		}

		ProbMat = CurrProbMat;
	}

	TLinAlg::Pow(ProbMat, Steps, ProbVV);
}

/////////////////////////////////////////////////////////////////
// Agglomerative clustering
THierarch::THierarch(const bool& _HistCacheSize, const bool& _IsTransitionBased,
			const bool& _Verbose):
		HierarchV(),
		StateHeightV(),
		MxHeight(TFlt::Mn),
		HistCacheSize(_HistCacheSize),
		PastStateIdV(),
		NLeafs(0),
		IsTransitionBased(_IsTransitionBased),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(HistCacheSize >= 1, "Have to hold at least the current state!");
}

THierarch::THierarch(TSIn& SIn):
		HierarchV(SIn),
		StateHeightV(SIn),
		UniqueHeightV(SIn),
		MxHeight(SIn),
		HistCacheSize(TInt(SIn)),
		PastStateIdV(SIn),
		NLeafs(TInt(SIn)),
		StateNmV(SIn),
		StateLabelV(SIn),
		TargetIdHeightSet(SIn),
		IsTransitionBased(TBool(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void THierarch::Save(TSOut& SOut) const {
	HierarchV.Save(SOut);
	StateHeightV.Save(SOut);
	UniqueHeightV.Save(SOut);
	MxHeight.Save(SOut);
	TInt(HistCacheSize).Save(SOut);
	PastStateIdV.Save(SOut);
	TInt(NLeafs).Save(SOut);
	StateNmV.Save(SOut);
	StateLabelV.Save(SOut);
	TargetIdHeightSet.Save(SOut);
	TBool(IsTransitionBased).Save(SOut);
	TBool(Verbose).Save(SOut);
}

THierarch* THierarch::Load(TSIn& SIn) {
	return new THierarch(SIn);
}

void THierarch::Init(const int& CurrLeafId, const TStateIdentifier& StateIdentifier,
		const TCtmcModeller& MChain) {
	const TFltVV& CentroidMat = StateIdentifier.GetCentroidMat();

	ClrFlds();

	NLeafs = CentroidMat.GetCols();

	if (IsTransitionBased) {
		InitHierarchyTrans(StateIdentifier, MChain);
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
		TAggStateV& StateSetV) const {
	TIntIntVH StateSubStateH;	GetAncSuccH(Height, StateSubStateH);

	StateIdV.Gen(StateSubStateH.Len());
	StateSetV.Gen(StateSubStateH.Len());

	int i = 0;
	int KeyId = StateSubStateH.FFirstKeyId();
	while (StateSubStateH.FNextKeyId(KeyId)) {
		const int StateIdx = StateSubStateH.GetKey(KeyId);

		if (StateSubStateH[KeyId].Empty()) {
			StateSetV[i].Add(StateIdx);
		} else {
			StateSetV[i] = StateSubStateH[KeyId];
		}

		StateIdV[i] = StateIdx;

		i++;
	}
}

void THierarch::GetStatesAtHeight(const double& Height, TIntSet& StateIdV) const {
	const int NStates = GetStates();

	for (int StateIdx = 0; StateIdx < NStates; StateIdx++) {
		if (IsOnHeight(StateIdx, Height)) {
			StateIdV.AddKey(StateIdx);
		}
	}
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

void THierarch::GetLeafDescendantV(const int& TargetStateId, TIntV& DescendantV) const {
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

void THierarch::GetCurrStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	const TFltV& HeightV = GetUniqueHeightV();
	for (int i = 0; i < PastStateIdV.Len(); i++) {
		const TIntV& PastStateIdVOnH = PastStateIdV[i];
		EAssertR(!PastStateIdVOnH.Empty(), "Past state cache empty!");
		StateIdHeightPrV.Add(TIntFltPr(PastStateIdVOnH[0], HeightV[i]));
	}
}

void THierarch::GetHistStateIdV(const double& Height, TStateIdV& StateIdV) const {
	const int NearestHeightIdx = GetNearestHeightIdx(Height);
	const TIntV& HistV = PastStateIdV[NearestHeightIdx];
	for (int i = 1; i < HistV.Len(); i++) {
		StateIdV.Add(HistV[i]);
	}
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
	const TFltVV& CentroidMat = StateIdentifier.GetCentroidMat();

	// create a hierarchy
	TIntIntFltTrV MergeV;	TAlAggClust::MakeDendro(CentroidMat, MergeV, Notify);

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

void THierarch::InitHierarchyTrans(const TStateIdentifier& StateIdentifier,
		const TCtmcModeller& MChain) {

	TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StateFtrVV);

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

//	TIntSet TakenIdSet;
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
//
//	MxHeight = StateHeightV.Last();
}

int THierarch::GetParentId(const int& StateId) const {
	return GetParentId(StateId, HierarchV);
}

int THierarch::GetNearestHeightIdx(const double& Height) const {
	// TODO optimize:
	// 1) use binary search to find the nearest height
	const TFltV& HeightV = GetUniqueHeightV();
	for (int i = 0; i < HeightV.Len() - 1; i++) {
		if (HeightV[i] <= Height && HeightV[i+1] > Height) {
			return i;
		}
	}
	return HeightV.Len() - 1;
}

double THierarch::GetNearestHeight(const double& InHeight) const {
	return UniqueHeightV[GetNearestHeightIdx(InHeight)];
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

void THierarch::ClrFlds() {
	HierarchV.Clr();
	StateHeightV.Clr();
	UniqueHeightV.Clr();
	MxHeight = TFlt::Mn;
	HistCacheSize = 1;
	PastStateIdV.Clr();
//	StateCoordV.Clr();
	NLeafs = 0;
	StateNmV.Clr();
	TargetIdHeightSet.Clr();
}

/////////////////////////////////////////////////////////////////
// UI helper
const double TUiHelper::STEP_FACTOR = 1e-2;
const double TUiHelper::INIT_RADIUS_FACTOR = 1.4;//1.2;

TUiHelper::TUiHelper(const TRnd& _Rnd, const bool& _Verbose):
		StateCoordV(),
		Rnd(_Rnd),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TUiHelper::TUiHelper(TSIn& SIn):
		StateCoordV(SIn),
		Rnd(SIn),
		Verbose(TBool(SIn)) {
	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TUiHelper::Save(TSOut& SOut) const {
	StateCoordV.Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TUiHelper::Init(const TStateIdentifier& StateIdentifier, const THierarch& Hierarch,
		const TCtmcModeller& MChain) {
	Notify->OnNotify(TNotifyType::ntInfo, "Initializing UI helper ...");
	InitStateCoordV(StateIdentifier, Hierarch);
	RefineStateCoordV(StateIdentifier, Hierarch, MChain);
//	TransformToUnit();
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
		RadiusV[i] = GetUIStateRaduis(ProbV[i]);// / (Height + .1);
	}
}

TFltPr& TUiHelper::GetModStateCoords(const int& StateId) {
	EAssertR(0 <= StateId && StateId < StateCoordV.Len(), TStr::Fmt("Invalid state id: %d", StateId));
	return StateCoordV[StateId];
}

void TUiHelper::InitStateCoordV(const TStateIdentifier& StateIdentifier,
		const THierarch& Hierarch) {

	Notify->OnNotify(TNotifyType::ntInfo, "Computing initial state coordinates ...");

	const int NStates = Hierarch.GetStates();
	const int NLeafs = Hierarch.GetLeafs();
	const TFltVV& CentroidVV = StateIdentifier.GetCentroidMat();

	StateCoordV.Gen(NStates, NStates);

	TFltVV CoordMat;	TEuclMds::Project(CentroidVV, CoordMat, 2);
	for (int ColIdx = 0; ColIdx < CoordMat.GetCols(); ColIdx++) {
		StateCoordV[ColIdx].Val1 = CoordMat(0, ColIdx);
		StateCoordV[ColIdx].Val2 = CoordMat(1, ColIdx);
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

void TUiHelper::RefineStateCoordV(const TStateIdentifier& StateIdentifier,
		const THierarch& Hierarch, const TCtmcModeller& MChain) {
	Notify->OnNotify(TNotifyType::ntInfo, "Refining node positions ...");

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

			TStateFtrVV StateFtrVV;	StateIdentifier.GetControlCentroidVV(StateFtrVV);
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

double TUiHelper::GetUIStateRaduis(const double& Prob) {
	// the probability is proportional to the area, so the raduis should
	// be proportional to the square root of the probability
	return TMath::Sqrt(Prob / TMath::Pi);
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

void TStateAssist::Init(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const TStateIdentifier& Clust,
		const THierarch& Hierarch, TStreamStoryCallback* Callback, const bool& MultiThread) {

	Notify->OnNotify(TNotifyType::ntInfo, "Computing state assist ...");

	InitFtrBounds(ObsFtrVV, ContrFtrVV);

	// get all the heights from the hierarchy
	TIntFltPrV StateIdHeightPrV;	Hierarch.GetStateIdHeightPrV(StateIdHeightPrV);

	//====================================================
	// TODO
	printf("HierarchV: %s\n", TStrUtil::GetStr(Hierarch.GetHierarchV()).CStr());
	printf("StateIdHeightPrV: %s\n", TStrUtil::GetStr(StateIdHeightPrV).CStr());
	//====================================================
	TIntV AssignV;	Clust.Assign(ObsFtrVV, AssignV);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist, total states %d ...", StateIdHeightPrV.Len());

	ClassifyV.Clr(true);
	DecisionTreeV.Clr(true);

	for (int HeightN = 0; HeightN < StateIdHeightPrV.Len(); HeightN++) {
		ClassifyV.Add(TLogReg(1, true, false));
		DecisionTreeV.Add(TDecisionTree());
	}

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

			InitSingle(ObsFtrVV, StateIdHeightPr.Val1, StateIdHeightPr.Val2, Hierarch,
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

			InitSingle(ObsFtrVV, StateId, Height, Hierarch, AssignV, Rnd, ClassifyV[HeightN], DecisionTreeV[HeightN]);

			if (Callback != nullptr) {
				Callback->OnProgress(70, "Initilized " + TInt::GetStr(HeightN+1) + " of " + TInt::GetStr(StateIdHeightPrV.Len()) + " states ...");
			}
		}
	}

	Notify->OnNotify(TNotifyType::ntInfo, "State assist initialized!");
}

void TStateAssist::InitFtrBounds(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV) {
	const int NObsFtrs = ObsFtrVV.GetRows();
	const int NContrFtrs = ContrFtrVV.GetRows();
	const int TotalFtrs = NObsFtrs + NContrFtrs;
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
}

const TFltPr& TStateAssist::GetFtrBounds(const int& FtrId) const {
	EAssertR(FtrId < FtrBoundV.Len(), "TStateAssist::GetFtrBounds: invalid feature ID!");
	return FtrBoundV[FtrId];
}

void TStateAssist::GetSuggestFtrs(const int& StateId, TFltV& WgtV) const {
	EAssertR(0 <= StateId && StateId < ClassifyV.Len(), "Invalid state ID!");

	const TLogReg& Classify = ClassifyV[StateId];
	Classify.GetWgtV(WgtV);
}

PJsonVal TStateAssist::GetStateClassifyTree(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < DecisionTreeV.Len(), "Invalid StateId!");
	return DecisionTreeV[StateId].GetJson();
}

PJsonVal TStateAssist::GetStateExplain(const int& StateId) const {
	EAssertR(0 <= StateId && StateId < DecisionTreeV.Len(), "Invalid StateId!");
	return DecisionTreeV[StateId].ExplainPositive();
}

void TStateAssist::InitSingle(const TFltVV& ObsFtrVV, const int& StateId, const double& Height,
		const THierarch& Hierarch, const TIntV& AssignV, TRnd& Rnd, TLogReg& LogReg,
		TDecisionTree& Tree, const bool& Sample) {

	const int NInst = ObsFtrVV.GetCols();
	const int Dim = ObsFtrVV.GetRows();

	TStateIdV StateIdV; TAggStateV StateSetV;
	Hierarch.GetStateSetsAtHeight(Height, StateIdV, StateSetV);

	const int NStates = StateIdV.Len();
	const int TrgStateIdx = StateIdV.SearchForw(StateId);
	EAssertR(TrgStateIdx >= 0, "Could not find the target state!");

	TIntV TargetIdxV, NonTargetIdxV;
	TVec<TIntV> StateIdFtrIdxVV(NStates);

	TIntSet TargetStateSet;
	for (int StateN = 0; StateN < NStates; StateN++) {
		if (StateIdV[StateN] == StateId) {
			TargetStateSet.AddKeyV(StateSetV[StateN]);
		}
	}

	EAssert(!TargetStateSet.Empty());

	if (Sample) {
		for (int StateN = 0; StateN < NStates; StateN++) {
			const TIntSet StateSet(StateSetV[StateN]);
			AssignV.FindAllSatisfy([&](const TInt& StateId) { return StateSet.IsKey(StateId); }, StateIdFtrIdxVV[StateN]);
			StateIdFtrIdxVV[StateN].Shuffle(Rnd);
		}

		// while there are instances select one instance from each state
		// so that we always get instances from all the states
		bool AllEmpty;
		do {
			AllEmpty = true;

			for (int StateN = 0; StateN < NStates; StateN++) {
				TIntV& IdxV = StateIdFtrIdxVV[StateN];

				if (IdxV.Empty()) { continue; }

				if (StateN == TrgStateIdx) {
					TargetIdxV.Add(IdxV.Last());
				} else {
					NonTargetIdxV.Add(IdxV.Last());
				}

				IdxV.DelLast();
				AllEmpty = false;
			}
		} while (!AllEmpty);

		// truncate the larger dataset
		if (TargetIdxV.Len() > NonTargetIdxV.Len()) {
			TargetIdxV.Trunc(NonTargetIdxV.Len());
		} else if (NonTargetIdxV.Len() > TargetIdxV.Len()) {
			NonTargetIdxV.Trunc(TargetIdxV.Len());
		}

		if (TargetIdxV.Empty() || NonTargetIdxV.Empty()) { return; }

		// construct a feature matrix
		TFltVV FtrVV(Dim, TargetIdxV.Len() + NonTargetIdxV.Len());
		TFltV LabelV(TargetIdxV.Len() + NonTargetIdxV.Len());

		for (int ColIdx = 0; ColIdx < TargetIdxV.Len(); ColIdx++) {
			const TInt& TargetColN = TargetIdxV[ColIdx];
			const TInt& NonTargetColN = NonTargetIdxV[ColIdx];

			const int TargetColIdx = ColIdx;
			const int NonTargetColIdx = TargetIdxV.Len() + ColIdx;

			for (int RowN = 0; RowN < Dim; RowN++) {
				FtrVV(RowN, TargetColIdx) = ObsFtrVV(RowN, TargetColN);
				FtrVV(RowN, NonTargetColIdx) = ObsFtrVV(RowN, NonTargetColN);
			}

			LabelV[TargetColIdx] = 1;
			LabelV[NonTargetColIdx] = 0;
		}

		FitAssistModels(FtrVV, LabelV, LogReg, Tree);
	} else {
		TFltV LabelV(NInst);

		for (int ColN = 0; ColN < NInst; ColN++) {
			LabelV[ColN] = TargetStateSet.IsKey(AssignV[ColN]) ? 1 : 0;
		}

		FitAssistModels(ObsFtrVV, LabelV, LogReg, Tree);
	}
}

void TStateAssist::FitAssistModels(const TFltVV& FtrVV, const TFltV& LabelV, TLogReg& LogReg,
			TDecisionTree& Tree) {
	const int NInst = FtrVV.GetCols();

	{
		int NPos = 0;
		for (int ColN = 0; ColN < NInst; ColN++) {
			NPos += LabelV[ColN];
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
// Hierarchical continous time Markov Chain
TStreamStory::TStreamStory():
		StateIdentifier(nullptr),
		MChain(nullptr),
		Hierarch(nullptr),
		StateAssist(nullptr),
		UiHelper(nullptr),
		PrevObsFtrV(),
		PrevContrFtrV(),
		PrevRecTm(),
		Verbose(true),
		Callback(nullptr),
		Notify(nullptr) {}

TStreamStory::TStreamStory(TStateIdentifier* _StateIdentifier,
		TCtmcModeller* _MChain, THierarch* _Hierarch, const TRnd& Rnd,
		const bool& _Verbose):
		StateIdentifier(_StateIdentifier),
		MChain(_MChain),
		Hierarch(_Hierarch),
		StateAssist(new TStateAssist(_Verbose)),
		UiHelper(new TUiHelper(Rnd, _Verbose)),
		PrevObsFtrV(),
		PrevContrFtrV(),
		PrevRecTm(),
		Verbose(_Verbose),
		Callback(nullptr),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
}

TStreamStory::TStreamStory(TSIn& SIn):
		StateIdentifier(new TStateIdentifier(SIn)),
		MChain(TCtmcModeller::Load(SIn)),
		Hierarch(THierarch::Load(SIn)),
		StateAssist(new TStateAssist(SIn)),
		UiHelper(new TUiHelper(SIn)),
		PrevObsFtrV(SIn),
		PrevContrFtrV(SIn),
		PrevRecTm(TUInt64(SIn)),
		Verbose(TBool(SIn)),
		Callback(nullptr),
		Notify() {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

TStreamStory::~TStreamStory() {
	delete StateIdentifier;
	delete MChain;
	delete Hierarch;
	delete StateAssist;
	delete UiHelper;
}

void TStreamStory::Save(TSOut& SOut) const {
	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::Save: saving to stream ...");

	StateIdentifier->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
	StateAssist->Save(SOut);
	UiHelper->Save(SOut);
	PrevObsFtrV.Save(SOut);
	PrevContrFtrV.Save(SOut);
	TUInt64(PrevRecTm).Save(SOut);
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

	const TFltV& UniqueHeightV = Hierarch->GetUniqueHeightV();
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV);

	// go through all the heights except the last one, which is not interesting
	// since it is only one state
	for (int HeightN = 0; HeightN < UniqueHeightV.Len()-1; HeightN++) {
		const double CurrHeight = UniqueHeightV[HeightN];

		StateIdV.Clr();
		StateSetV.Clr();
		StateIdProbPrV.Clr();

		// get the states on this level
		Hierarch->GetStateSetsAtHeight(CurrHeight, StateIdV, StateSetV);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// chindren
		TFltVV TransitionVV;	MChain->GetJumpVV(StateSetV, StateFtrVV, TransitionVV);
		TFltV HoldingTimeV;		MChain->GetHoldingTmV(StateSetV, StateFtrVV, HoldingTimeV);
		TFltV ProbV;			MChain->GetStatDist(StateSetV, StateFtrVV, ProbV);
		TFltV RadiusV;			UiHelper->GetStateRadiusV(ProbV, RadiusV);

		PJsonVal LevelJsonVal = GetLevelJson(CurrHeight, StateIdV, TransitionVV, HoldingTimeV, ProbV, RadiusV);
		Result->AddToArr(LevelJsonVal);
	}

	return Result;
}

PJsonVal TStreamStory::GetSubModelJson(const int& StateId) const {
	TIntV DescendantIdV;	Hierarch->GetLeafDescendantV(StateId, DescendantIdV);

	Notify->OnNotify(TNotifyType::ntInfo, "TStreamStory::GetJson: saving JSON ...");

	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV);

	PJsonVal Result = TJsonVal::NewArr();

	// construct for height 0
	const double CurrHeight = 0;
	const int NStates = DescendantIdV.Len();
	PJsonVal LevelJsonVal = TJsonVal::NewObj();

	TAggStateV AggStateV;
	for (int StateId = 0; StateId < MChain->GetStates(); StateId++) {
		AggStateV.Add(TIntV(1, 1));
		AggStateV[StateId][0] = StateId;
	}

//	TFltVV SubQMat;	MChain->GetSubQMatrix(AggStateV, StateFtrVV, DescendantIdV, SubQMat);

	// construct the values for the original chain
	TFltVV AllJumpVV;		MChain->GetJumpVV(AggStateV, StateFtrVV, AllJumpVV);//TCtmcModeller::GetJumpVV(SubQMat, TransitionVV);
	TFltV AllHoldingTmV;	MChain->GetHoldingTmV(AggStateV, StateFtrVV, AllHoldingTmV);
	TFltV AllProbV;			MChain->GetStatDist(AggStateV, StateFtrVV, AllProbV);
	TFltV AllRadiusV;		UiHelper->GetStateRadiusV(AllProbV, AllRadiusV);

	// construct the values for the sub chain
	TFltVV JumpVV(NStates, NStates);
	TFltV HoldingTmV(NStates, NStates), ProbV(NStates, NStates), RadiusV(NStates, NStates);
	for (int StateN = 0; StateN < DescendantIdV.Len(); StateN++) {
		const int StateId = DescendantIdV[StateN];

		HoldingTmV[StateN] = AllHoldingTmV[StateId];
		ProbV[StateN] = AllProbV[StateId];
		RadiusV[StateN] = AllRadiusV[StateId];

		for (int DstStateN = 0; DstStateN < NStates; DstStateN++) {
			const int DstStateId = DescendantIdV[DstStateN];

			JumpVV(StateN, DstStateN) = AllJumpVV(StateId, DstStateId);
		}
	}

	PJsonVal LevelJsonV = GetLevelJson(CurrHeight, DescendantIdV, JumpVV, HoldingTmV, ProbV, RadiusV);
	Result->AddToArr(LevelJsonV);

	return Result;
}

void TStreamStory::Init(TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV, const TUInt64V& RecTmV,
		const bool& MultiThread) {
	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, TBoolV(), FtrVV);

	Callback->OnProgress(0, "Clustering ...");
	TIntV AssignV;	InitClust(ObservFtrVV, FtrVV, AssignV);
	Callback->OnProgress(30, "Modeling transitions ...");
	InitMChain(FtrVV, AssignV, RecTmV, false, TBoolV());
	Callback->OnProgress(50, "Initializing hierarchy ...");
	InitHierarch();
	Callback->OnProgress(60, "Initializing states ...");
	InitStateAssist(ObservFtrVV, ControlFtrVV, MultiThread);
	Callback->OnProgress(90, "Computing positions ...");
	UiHelper->Init(*StateIdentifier, *Hierarch, *MChain);
}

void TStreamStory::InitBatches(TFltVV& ObservFtrVV, const TFltVV& ControlFtrVV,
		const TUInt64V& RecTmV, const TBoolV& BatchEndV, const bool& MultiThread) {
	CheckBatches(RecTmV, BatchEndV);

	TFltVV FtrVV;	CreateFtrVV(ObservFtrVV, ControlFtrVV, RecTmV, BatchEndV, FtrVV);

	TIntV AssignV;	InitClust(ObservFtrVV, FtrVV, AssignV);
	InitMChain(FtrVV, AssignV, RecTmV, true, BatchEndV);
	InitHierarch();
	InitStateAssist(ObservFtrVV, ControlFtrVV, MultiThread);
	UiHelper->Init(*StateIdentifier, *Hierarch, *MChain);
}

void TStreamStory::InitClust(TFltVV& ObsFtrVV, const TFltVV& FtrVV,
		TIntV& AssignV) {
	StateIdentifier->Init(ObsFtrVV, FtrVV);
	StateIdentifier->Assign(ObsFtrVV, AssignV);
}

void TStreamStory::InitMChain(const TFltVV& FtrVV, const TIntV& AssignV,
		const TUInt64V& RecTmV, const bool IsBatchData, const TBoolV& EndBatchV) {
	MChain->Init(FtrVV, StateIdentifier->GetStates(), AssignV, RecTmV, IsBatchData, EndBatchV);
}

void TStreamStory::InitHierarch() {
	Hierarch->Init(MChain->GetCurrStateId(), *StateIdentifier, *MChain);
}

void TStreamStory::InitHistograms(const TFltVV& ObsMat, const TFltVV& ControlMat,
		const TUInt64V& RecTmV, const TBoolV& BatchEndV) {
	TFltVV FtrVV;	CreateFtrVV(ObsMat, ControlMat, RecTmV, BatchEndV, FtrVV);
	StateIdentifier->InitHistograms(ObsMat, FtrVV);
}

void TStreamStory::InitStateAssist(const TFltVV& ObsFtrVV, const TFltVV& ContrFtrVV, const bool& MultiThread) {
	StateAssist->Init(ObsFtrVV, ContrFtrVV, *StateIdentifier, *Hierarch, Callback, MultiThread);
}

void TStreamStory::OnAddRec(const uint64& RecTm, const TFltV& ObsFtrV, const TFltV& ContrFtrV) {
	TStateFtrVV StateFtrVV;	GetStateFtrVV(StateFtrVV);
	TFltV FtrV;	CreateFtrV(ObsFtrV, ContrFtrV, RecTm, FtrV);

	const int OldStateId = MChain->GetCurrStateId();
	const int NewStateId = StateIdentifier->Assign(ObsFtrV);

	DetectAnomalies(OldStateId, NewStateId, ObsFtrV, FtrV);

	if (NewStateId != -1) {
		MChain->OnAddRec(NewStateId, RecTm, false);

		if (NewStateId != OldStateId && Callback != nullptr) {
			PredictTargets(RecTm, StateFtrVV, NewStateId);
			Hierarch->UpdateHistory(NewStateId);

			TIntFltPrV CurrStateV;	GetCurrStateAncestry(CurrStateV);
			Callback->OnStateChanged(CurrStateV);
		}
	}

	PrevObsFtrV = ObsFtrV;
	PrevContrFtrV = ContrFtrV;
	PrevRecTm = RecTm;
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

void TStreamStory::GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const {
	try {
		TIntV LeafV;
		Hierarch->GetLeafDescendantV(StateId, LeafV);
		StateIdentifier->GetHistogram(FtrId, LeafV, BinStartV, ProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetTransitionHistogram(const int& SourceId, const int& TargetId, const int& FtrId,
		TFltV& BinStartV, TFltV& ProbV) const {
	try {
		TIntV SourceLeafV, TargetLeafV;
		Hierarch->GetLeafDescendantV(SourceId, SourceLeafV);
		Hierarch->GetLeafDescendantV(TargetId, TargetLeafV);
		StateIdentifier->GetTransitionHistogram(FtrId, SourceLeafV, TargetLeafV, BinStartV, ProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetTransitionHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::GetStateWgtV(const int& StateId, TFltV& WgtV) const {
	try {
		StateAssist->GetSuggestFtrs(StateId, WgtV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateWgtV: Failed to fetch weight vector for state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

PJsonVal TStreamStory::GetStateClassifyTree(const int& StateId) const {
	try {
		return StateAssist->GetStateClassifyTree(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateClassifyTree: Exceptino while fetching classification tree for state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

PJsonVal TStreamStory::GetStateExplain(const int& StateId) const {
	try {
		return StateAssist->GetStateExplain(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::GetStateExplain: Exceptino while generating explanation for state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
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
	Hierarch->GetCurrStateIdHeightPrV(StateIdHeightPrV);
}

int TStreamStory::GetCurrStateId(const double& Height) const {
	return Hierarch->GetAncestorAtHeight(MChain->GetCurrStateId(), Height);
}

void TStreamStory::GetCentroid(const int& StateId, TFltV& FtrV, const bool ObsCentroid) const {
	TIntV LeafIdV;	Hierarch->GetLeafDescendantV(StateId, LeafIdV);

	if (ObsCentroid) {
		StateIdentifier->GetJoinedCentroid(LeafIdV, FtrV);
	} else {
		StateIdentifier->GetJoinedControlCentroid(LeafIdV, FtrV);
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
		const int ObsFtrs = StateIdentifier->GetDim();
		StateIdentifier->SetControlFtr(StateId, FtrId - ObsFtrs, Val);
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
		const int ObsFtrs = StateIdentifier->GetDim();
		StateIdentifier->ClearControlFtr(StateId, FtrId - ObsFtrs);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "TStreamStory::SetControlFtrFactor: Failed to set factor: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void TStreamStory::ResetControlFtrVals(const int& StateId) {
	const int ObsDim = StateIdentifier->GetDim();
	const int Dim = StateIdentifier->GetControlDim();
	for (int FtrId = 0; FtrId < Dim; FtrId++) {
		ResetControlFtrVal(StateId, FtrId + ObsDim);
	}
}

void TStreamStory::ResetControlFtrVals() {
	StateIdentifier->ClearControlFtrVV();
}

bool TStreamStory::IsAnyControlFtrSet() const {
	return StateIdentifier->IsAnyControlFtrSet();
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
}

void TStreamStory::SetCallback(TStreamStoryCallback* _Callback) {
	Callback = _Callback;
}

PJsonVal TStreamStory::GetLevelJson(const double& Height, const TStateIdV& StateIdV, const TFltVV& TransitionVV,
		const TFltV& HoldingTimeV, const TFltV& ProbV, const TFltV& RadiusV) const {

	PJsonVal Result = TJsonVal::NewObj();

	PJsonVal StateJsonV = TJsonVal::NewArr();
	for (int StateN = 0; StateN < StateIdV.Len(); StateN++) {
		const int StateId = StateIdV[StateN];
		const TFltPr& StateCoords = UiHelper->GetStateCoords(StateId);

		PJsonVal StateJson = TJsonVal::NewObj();
		StateJson->AddToObj("id", StateId);
		StateJson->AddToObj("x", StateCoords.Val1);
		StateJson->AddToObj("y", StateCoords.Val2);
		StateJson->AddToObj("radius", RadiusV[StateN]);
		StateJson->AddToObj("timeProportion", ProbV[StateN]);
		StateJson->AddToObj("holdingTime", HoldingTimeV[StateN]);
		StateJson->AddToObj("isTarget", Hierarch->IsTarget(StateId));
		StateJson->AddToObj("label", Hierarch->GetStateLabel(StateId));

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
//	MChain->CreateFtrV(ObsFtrV, PrevObsFtrV, ContrFtrV, PrevContrFtrV, RecTm, PrevRecTm, FtrV);
}

void TStreamStory::GetStateFtrVV(TStateFtrVV& StateFtrVV) const {
	StateIdentifier->GetControlCentroidVV(StateFtrVV);
}

void TStreamStory::GetStatsAtHeight(const double& Height, TAggStateV& StateSetV,
		TStateIdV& StateIdV, TStateFtrVV& StateFtrVV) const {
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	GetStateFtrVV(StateFtrVV);
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

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

#include "mc.h"

using namespace TMc;

//////////////////////////////////////////////////////
// Distance measures
TFullMatrix TEuclDist::GetDist2(const TFullMatrix& X, const TFullMatrix& Y) {
	const TVector OnesX = TVector::Ones(X.GetCols(), true);
	const TVector OnesY = TVector::Ones(Y.GetCols(), false);
	const TVector NormX2 = X.ColNorm2V().Transpose();
	const TVector NormY2 = Y.ColNorm2V();

	return (NormX2 * OnesY) - (X*2).MulT(Y) + (OnesX * NormY2);
}


/////////////////////////////////////////////
// MDS
TFullMatrix TEuclMds::Project(const TFullMatrix& X, const int& d) {
	// first center the rows of matrix X
	TFullMatrix X1 = X.GetCenteredRows();

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

	X_d.Transpose();

	return X_d;
}

void TAvgLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = (DistMat(MnI, i)*ItemCountV[MnI] + DistMat(MnJ, i)*ItemCountV[MnJ]) / (ItemCountV[MnI] + ItemCountV[MnJ]);
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

void TCompleteLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mx(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

void TSingleLink::JoinClusts(TFullMatrix& DistMat, const TVector& ItemCountV, const int& MnI, const int& MnJ) {
	TVector NewDistV(DistMat.GetRows(), false);
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mn(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV.Transpose());
}

/////////////////////////////////////////////////////////////////
// Agglomerative clustering
THierarch::THierarch(const bool& _HistCacheSize, const bool& _Verbose):
		HierarchV(),
		StateHeightV(),
		MxHeight(TFlt::Mn),
		HistCacheSize(_HistCacheSize),
		PastStateIdV(),
		StateCoordV(),
		NLeafs(0),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {

	EAssertR(HistCacheSize >= 1, "Have to hold at least the current state!");
}

THierarch::THierarch(TSIn& SIn):
		HierarchV(SIn),
		StateHeightV(SIn),
		MxHeight(SIn),
		HistCacheSize(TInt(SIn)),
		PastStateIdV(SIn),
		StateCoordV(SIn),
		NLeafs(TInt(SIn)),
		StateNmV(SIn),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void THierarch::Save(TSOut& SOut) const {
	HierarchV.Save(SOut);
	StateHeightV.Save(SOut);
	MxHeight.Save(SOut);
	TInt(HistCacheSize).Save(SOut);
	PastStateIdV.Save(SOut);
	StateCoordV.Save(SOut);
	TInt(NLeafs).Save(SOut);
	StateNmV.Save(SOut);
	TBool(Verbose).Save(SOut);
}

PHierarch THierarch::Load(TSIn& SIn) {
	return new THierarch(SIn);
}

void THierarch::Init(const TFullMatrix& CentroidMat, const int& CurrLeafId) {
	ClrFlds();

	NLeafs = CentroidMat.GetCols();

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

	// compute state coordinates
	ComputeStateCoords(CentroidMat, NStates);

	// initialize history
	TFltV HeightV;	GetUniqueHeightV(HeightV);
	PastStateIdV.Gen(HeightV.Len(), HeightV.Len());
	UpdateHistory(CurrLeafId);

	// initialize state names
	StateNmV.Gen(HierarchV.Len());
}

void THierarch::UpdateHistory(const int& CurrLeafId) {
	TFltV HeightV;	GetUniqueHeightV(HeightV);
	TIntFltPrV StateIdHeightPrV;	GetAncestorV(CurrLeafId, StateIdHeightPrV);

	EAssertR(HeightV.Len() == PastStateIdV.Len(), "Number of heights doesn't match the number of heights in the past state cache!");

	int CurrHeightIdx = 0;
	for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
		const int CurrStateId = StateIdHeightPrV[i].Val1;

		while (CurrHeightIdx < HeightV.Len() && IsOnHeight(CurrStateId, HeightV[CurrHeightIdx])) {
			if (PastStateIdV[CurrHeightIdx].Empty() || PastStateIdV[CurrHeightIdx][0] != CurrStateId) {
				PastStateIdV[CurrHeightIdx].Ins(0, CurrStateId);
				// cleanup
				while (PastStateIdV[CurrHeightIdx].Len() > HistCacheSize) { PastStateIdV[CurrHeightIdx].DelLast(); }
			}
			CurrHeightIdx++;
		}
	}
}

void THierarch::GetUniqueHeightV(TFltV& HeightV) const {
	const int NStates = GetStates();

	TFltSet UsedHeightSet;
	for (int i = 0; i < NStates; i++) {
		if (!UsedHeightSet.IsKey(StateHeightV[i])) {
			HeightV.Add(StateHeightV[i]);
			UsedHeightSet.AddKey(StateHeightV[i]);
		}
	}

	HeightV.Sort(true);	// sort ascending
}

void THierarch::GetStateIdHeightPrV(TIntFltPrV& StateIdHeightPrV) const {
	for (int i = 0; i < HierarchV.Len(); i++) {
		StateIdHeightPrV.Add(TIntFltPr(i, GetStateHeight(i)));
	}
}

void THierarch::GetStateSetsAtHeight(const double& Height, TIntV& StateIdV, TVec<TIntV>& StateSetV) const {
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
	TFltV HeightV;	GetUniqueHeightV(HeightV);
	for (int i = 0; i < PastStateIdV.Len(); i++) {
		const TIntV& PastStateIdVOnH = PastStateIdV[i];
		EAssertR(!PastStateIdVOnH.Empty(), "Past state cache empty!");
		StateIdHeightPrV.Add(TIntFltPr(PastStateIdVOnH[0], HeightV[i]));
	}
}

void THierarch::GetHistStateIdV(const double& Height, TIntV& StateIdV) const {
	const int NearestHeightIdx = GetNearestHeightIdx(Height);
	const TIntV& HistV = PastStateIdV[NearestHeightIdx];
	for (int i = 1; i < HistV.Len(); i++) {
		StateIdV.Add(HistV[i]);
	}
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

int THierarch::GetParentId(const int& StateId) const {
	return GetParentId(StateId, HierarchV);
}

int THierarch::GetNearestHeightIdx(const double& Height) const {
	// TODO optimize:
	// 1) Precompute the unique heights
	// 2) use binary search to find the nearest height
	TFltV HeightV;	GetUniqueHeightV(HeightV);
	for (int i = 0; i < HeightV.Len() - 1; i++) {
		if (HeightV[i] <= Height && HeightV[i+1] > Height) {
			return i;
		}
	}
	return HeightV.Len() - 1;
}

bool THierarch::IsRoot(const int& StateId) const {
	return IsRoot(StateId, HierarchV);
}

bool THierarch::IsLeaf(const int& StateId) const {
	return StateId < NLeafs;
}

bool THierarch::IsOnHeight(const int& StateId, const double& Height) const {
	if (IsRoot(StateId) && Height >= MxHeight) { return true; }
	return GetStateHeight(StateId) <= Height && GetStateHeight(GetParentId(StateId)) > Height;
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

void THierarch::ComputeStateCoords(const TFullMatrix& CentroidMat, const int& NStates) {
	StateCoordV.Gen(NStates, NStates);

	TFullMatrix CoordMat = TEuclMds::Project(CentroidMat, 2);
	for (int ColIdx = 0; ColIdx < CoordMat.GetCols(); ColIdx++) {
		StateCoordV[ColIdx].Val1 = CoordMat(0, ColIdx);
		StateCoordV[ColIdx].Val2 = CoordMat(1, ColIdx);
	}

	// first find out how many ancestors each state has, so you can weight
	// the childs coordinates appropriately
	TIntV SuccesorCountV;	GetLeafSuccesorCountV(SuccesorCountV);

	TIntV TempHierarchV(HierarchV);

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

bool THierarch::IsRoot(const int& StateId, const TIntV& HierarchV) {
	return GetParentId(StateId, HierarchV) == StateId;
}

void THierarch::ClrFlds() {
	HierarchV.Clr();
	StateHeightV.Clr();
	MxHeight = TFlt::Mn;
	HistCacheSize = 1;
	PastStateIdV.Clr();
	StateCoordV.Clr();
	NLeafs = 0;
}

/////////////////////////////////////////////////////////////////
// Abstract Markov Chain
TMChain::TMChain(const bool& _Verbose):
		NStates(-1),
		CurrStateId(-1),
		HasHiddenState(false),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TMChain::TMChain(TSIn& SIn):
		NStates(TInt(SIn)),
		CurrStateId(TInt(SIn)),
		HasHiddenState(TBool(SIn)),
		Verbose(TBool(SIn)),
		Notify(nullptr) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}



void TMChain::Save(TSOut& SOut) const {
	GetType().Save(SOut);
	TInt(NStates).Save(SOut);
	TInt(CurrStateId).Save(SOut);
	TBool(HasHiddenState).Save(SOut);
	TBool(Verbose).Save(SOut);
}

PMChain TMChain::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == "discrete") {
		return new TDtMChain(SIn);
	} else if (Type == "continuous") {
		return new TCtMChain(SIn);
	} else {
		throw TExcept::New("Invalid type of Markov chain: " + Type, "TMChain::Load");
	}
}

void TMChain::Init(const int& _NStates, const TIntV& StateAssignV, const TUInt64V& TmV,
		const bool _HasHiddenState, const TBoolV& EndBatchV) {
	NStates = _NStates;
	HasHiddenState = _HasHiddenState;

	Notify->OnNotify(TNotifyType::ntInfo, "Initializing Markov chain ...");

	// initialize statistics
	InitStats(NStates);

	// update states
	const uint64 NRecs = TmV.Len();
	for (uint64 i = 0; i < NRecs; i++) {
		if (i % 10000 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, TUInt64::GetStr(i).CStr());
		}

		const bool EndsBatch = HasHiddenState ? bool(EndBatchV[i]) : false;
		OnAddRec(StateAssignV[i], TmV[i], true, EndsBatch);
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Done!");

//	PrintStats();
}

void TMChain::OnAddRec(const int& StateId, const uint64& RecTm, const bool UpdateStats, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot be last in sequence if a hidden state does not exist!");

	// call child method
	AbsOnAddRec(StateId, RecTm, UpdateStats, EndsBatch);
	if (HasHiddenState && EndsBatch) {
		CurrStateId = GetHiddenStateId();
	} else {
		CurrStateId = StateId;
	}
}

void TMChain::GetFutureProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV,
		const int& StateId, const double& Tm, TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TVector ProbV = GetFutureProbMat(StateSetV, Tm).GetRow(StateIdx);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TMChain::GetPastProbV(const TVec<TIntV>& StateSetV, const TIntV& StateIdV,
		const int& StateId, const double& Tm, TIntFltPrV& StateIdProbV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TMChain::GetFutureProbV: Could not find target state!");

	TVector ProbV = GetPastProbMat(StateSetV, Tm).GetRow(StateIdx);

	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdProbV.Add(TIntFltPr(StateIdV[i], ProbV[i]));
	}
}

void TMChain::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

int TMChain::GetHiddenStateId() const {
	return HasHiddenState ? GetStates() : -2;
}

void TMChain::InsHiddenState(TVec<TIntV>& StateSetV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");

	StateSetV.Add(TIntV(1, 1));
	StateSetV.Last()[0] = GetHiddenStateId();
}

void TMChain::InsHiddenState(TIntV& StateIdV) const {
	EAssertR(HasHiddenState, "TMChain::InsHiddenState: The model does not have a hidden state!");
	StateIdV.Add(GetHiddenStateId());
}

void TMChain::RemoveHiddenStateProb(TIntFltPrV& StateIdProbV) const {
	EAssertR(HasHiddenState, "TMChain::RemoveHiddenStateProb: The model does not have a hidden state!");

	const int HiddenStateId = GetHiddenStateId();

	for (int i = 0; i < StateIdProbV.Len(); i++) {
		if (StateIdProbV[i].Val1 == HiddenStateId) {
			StateIdProbV.Del(i);
			break;
		}
	}
}

void TMChain::GetFutureProbVOverTm(const TFullMatrix& PMat, const int& StateIdx,
		const int& Steps, TVec<TFltV>& ProbVV, const PNotify& Notify, const bool IncludeT0) {

	TFullMatrix CurrentMat = TFullMatrix::Identity(PMat.GetRows());
	TVector ProbV = CurrentMat.GetRow(StateIdx);

	if (IncludeT0) {
		ProbVV.Add(ProbV.Vec);
	}

	for (int i = 0; i < Steps; i++) {
		if (i % 100 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "steps: %d", i);
		}
		// increase time
		CurrentMat = CurrentMat * PMat;
		ProbV = CurrentMat.GetRow(StateIdx);
		// normalize to minimize the error
		ProbV /= ProbV.Sum();
		// add to result
		ProbVV.Add(ProbV.Vec);
	}
}

/////////////////////////////////////////////////////////////////
// Discrete time Markov Chain
TDtMChain::TDtMChain(const bool& _Verbose):
		TMChain(_Verbose),
		JumpCountMat() {}

TDtMChain::TDtMChain(TSIn& SIn):
		TMChain(SIn),
		JumpCountMat() {
	JumpCountMat.Load(SIn);
}

void TDtMChain::Save(TSOut& SOut) const {
	TMChain::Save(SOut);
	JumpCountMat.Save(SOut);
}

void TDtMChain::GetNextStateProbV(const TVec<TIntV>& JoinedStateVV, const TIntV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const {

	const int NFStates = TMath::Mn(NFutStates, JoinedStateVV.Len()-1);
	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TDtMChain::GetNextStateProbV: Could not find target state!");

	TVector ProbVec = GetTransitionMat(JoinedStateVV).GetRow(StateIdx);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbVec.Len(); j++) {
			if (j != StateIdx && !TakenIdxSet.IsKey(j) && ProbVec[j] > MxProb) {
				MxProb = ProbVec[j];
				MxIdx = j;
			}
		}

		// if we exclude the current state than the probability that j will be the future state
		// is sum_{k=0}^{inf} p_{ii}^k*P_{ij} = p_{ij} / (1 - p_{ii})
		const double Prob = ProbVec[MxIdx] / (1 - ProbVec[StateIdx]);

		if (Prob <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		StateIdProbV.Add(TIntFltPr(StateIdV[MxIdx], Prob));
	}
}

void TDtMChain::GetPrevStateProbV(const TVec<TIntV>& JoinedStateVV, const TIntV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NPrevStates) const {
	throw TExcept::New("TDtMChain::GetNextStateProbV: Not implemented!!!", "TDtMChain::GetNextStateProbV");
}

TFullMatrix TDtMChain::GetTransitionMat(const TVec<TIntV>& JoinedStateVV) const {
	TFullMatrix Result(JoinedStateVV.Len(), JoinedStateVV.Len());

	const TFullMatrix PMat = GetTransitionMat();
	const TVector StatDist = GetStatDist();

	for (int JoinState1Idx = 0; JoinState1Idx < JoinedStateVV.Len(); JoinState1Idx++) {
		const TIntV& JoinState1 = JoinedStateVV[JoinState1Idx];
		for (int JoinState2Idx = 0; JoinState2Idx < JoinedStateVV.Len(); JoinState2Idx++) {
			const TIntV& JoinState2 = JoinedStateVV[JoinState2Idx];

			// the transition probability from set Ai to Aj can be
			// calculated as: p_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} p_{k,l}} {\sum_{k \in A_i} \pi_k}

			TFlt Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					SumK += PMat(StateK,StateL);
				}

				Sum += StatDist[JoinState1[k]]*SumK;
				SumP += StatDist[JoinState1[k]];
			}
			Result(JoinState1Idx, JoinState2Idx) = Sum / SumP;
		}
	}

	return Result;
}

bool TDtMChain::IsAnomalousJump(const int& NewStateId, const int& OldStateId) const {
	return JumpCountMat(OldStateId, NewStateId) == 0.0;
}

void TDtMChain::InitStats(const int& NStates) {
	JumpCountMat = TFullMatrix(NStates, NStates);
}

void TDtMChain::AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool UpdateStats, const bool IsLastInSeq) {
	// update jump stats
	if (UpdateStats && CurrStateId != -1) {
		JumpCountMat(CurrStateId, StateId)++;
	}

	if (StateId != CurrStateId) {
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Jumped to state %d", StateId);
	}
}

TFullMatrix TDtMChain::GetFutureProbMat(const TVec<TIntV>& JoinedStateVV, const double& TimeSteps) const {
	const int Steps = (int) TimeSteps;
	EAssertR(Steps >= 0, "Probs for past states not implemented!");

	return GetTransitionMat(JoinedStateVV)^Steps;
}

TFullMatrix TDtMChain::GetPastProbMat(const TVec<TIntV>& JoinedStateVV, const double& TimeSteps) const {
	throw TExcept::New("GetPastProbMat: Not implemented!!!");
}

TFullMatrix TDtMChain::GetTransitionMat() const {
	TFullMatrix Result(NStates, NStates);

	#pragma omp for
	for (int RowIdx = 0; RowIdx < NStates; RowIdx++) {
		double Count = JumpCountMat.RowSum(RowIdx);


		for (int ColIdx = 0; ColIdx < NStates; ColIdx++) {
			Result(RowIdx, ColIdx) = JumpCountMat(RowIdx, ColIdx) / Count;
		}
	}

	return Result;
}

TVector TDtMChain::GetStatDist(const TFullMatrix& PMat) {
	if (PMat.Empty()) { return TVector(0); }

	TVector EigenVec(PMat.GetRows());
	TNumericalStuff::GetEigenVec(PMat.GetT().GetMat(), 1.0, EigenVec.Vec);

	return EigenVec /= EigenVec.Sum();
}

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
const uint64 TCtMChain::TU_SECOND = 1000;
const uint64 TCtMChain::TU_MINUTE = TU_SECOND*60;
const uint64 TCtMChain::TU_HOUR = TU_MINUTE*60;
const uint64 TCtMChain::TU_DAY = TU_HOUR*24;
const uint64 TCtMChain::TU_MONTH = 365.25 * TU_DAY / 12;
const double TCtMChain::MIN_JUMP_TM = 1e-2;
const double TCtMChain::HIDDEN_STATE_INTENSITY = 1 / MIN_JUMP_TM;

TCtMChain::TCtMChain(const uint64& _TimeUnit, const double& _DeltaTm, const bool& _Verbose):
		TMChain(_Verbose),
		QMatStats(),
		DeltaTm(_DeltaTm),
		TimeUnit(_TimeUnit),
		PrevJumpTm(-1) {}

TCtMChain::TCtMChain(TSIn& SIn):
		TMChain(SIn),
		QMatStats(),
		DeltaTm(0),
		TimeUnit(0),
		PrevJumpTm(0) {

	QMatStats.Load(SIn);
	DeltaTm = TFlt(SIn);
	TimeUnit = TUInt64(SIn);
	PrevJumpTm = TUInt64(SIn);
}

void TCtMChain::Save(TSOut& SOut) const {
	TMChain::Save(SOut);
	QMatStats.Save(SOut);
	TFlt(DeltaTm).Save(SOut);
	TUInt64(TimeUnit).Save(SOut);
	TUInt64(PrevJumpTm).Save(SOut);
}

void TCtMChain::GetNextStateProbV(const TVec<TIntV>& StateSetV, const TIntV& ExtStateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const {

	TIntV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	GetNextStateProbV(GetQMatrix(StateSetV), StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtMChain::GetPrevStateProbV(const TVec<TIntV>& StateSetV, const TIntV& ExtStateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates) const {

	TIntV StateIdV(ExtStateIdV);
	if (HasHiddenState) {
		InsHiddenState(StateIdV);
	}

	GetNextStateProbV(GetRevQMatrix(StateSetV), StateIdV, StateId, StateIdProbV, NFutStates, Notify);

	if (HasHiddenState) {
		RemoveHiddenStateProb(StateIdProbV);
	}
}

void TCtMChain::GetProbVOverTm(const double& Height, const int& StateId, const double& StartTm, const double EndTm,
		const double& DeltaTm, const TVec<TIntV>& StateSetV, const TIntV& StateIdV, TVec<TFltV>& FutProbVV, TVec<TFltV>& PastProbVV) const {

	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "Could not find target state!");
	EAssertR(StartTm <= 0 && EndTm >= 0, "The start and end times should include the current time!");

	const int FutureSteps = ceil(EndTm / DeltaTm);
	const TFullMatrix FutProbMat = GetFutureProbMat(StateSetV, DeltaTm);

	GetFutureProbVOverTm(FutProbMat, StateIdx, FutureSteps, FutProbVV, Notify);

	if (StartTm < 0) {
		const int PastSteps = ceil(-StartTm / DeltaTm);
		const TFullMatrix PastProbMat = GetPastProbMat(StateSetV, DeltaTm);
		GetFutureProbVOverTm(PastProbMat, StateIdx, PastSteps, PastProbVV, Notify, false);
	}
}

TVector TCtMChain::GetStatDist(const TVec<TIntV>& StateSetV) const {
	TVector StaticDist = GetStatDist(GetQMatrix(StateSetV), Notify);
	if (HasHiddenState) {
		StaticDist.DelLast();
		StaticDist /= StaticDist.Sum();
	}
	return StaticDist;
}

TVector TCtMChain::GetStateSizeV(const TVec<TIntV>& JoinedStateVV) const {
//	return GetHoldingTimeV(GetQMatrix(JoinedStateVV));
	return GetStatDist(JoinedStateVV);
}

TFullMatrix TCtMChain::GetTransitionMat(const TVec<TIntV>& StateSetV) const {
	return GetJumpMatrix(StateSetV);
}

TFullMatrix TCtMChain::GetJumpMatrix(const TVec<TIntV>& StateSetV) const {
	TFullMatrix JumpMat = GetJumpMatrix(GetQMatrix(StateSetV));
	if (HasHiddenState) {
		JumpMat = JumpMat(TVector::Range(JumpMat.GetRows()-1), TVector::Range(JumpMat.GetCols()-1));
//		JumpMat.NormalizeRowsL1(); TODO
		return JumpMat;
	} else {
		return JumpMat;
	}
}

TVector TCtMChain::GetHoldingTimeV(const TVec<TIntV>& StateSetV) const {
	TVector HoldingTmV = GetHoldingTimeV(GetQMatrix(StateSetV));
	if (HasHiddenState) {
		HoldingTmV.DelLast();
	}
	return HoldingTmV;
}

bool TCtMChain::IsAnomalousJump(const int& NewStateId, const int& OldStateId) const {
	return QMatStats[OldStateId][NewStateId].Val1 == 0;
}

void TCtMChain::InitStats(const int& NStates) {
	// initialize a matrix holding the number of measurements and the sum

	// if we get separate sequences than we need to add a hidden state, which will be the last
	// state in the Q matrix. This state will immediately jump to the next state

	const int States = HasHiddenState ? NStates + 1 : NStates;

	QMatStats.Gen(States, 0);
	for (int i = 0; i < States; i++) {
		QMatStats.Add(TUInt64FltPrV(States, States));
	}
}

void TCtMChain::AbsOnAddRec(const int& StateId, const uint64& RecTm, const bool UpdateStats, const bool EndsBatch) {
	EAssertR(HasHiddenState || !EndsBatch, "Cannot process batches with no hidden state!");

	// warn if times don't aren't ascending
	if (CurrStateId != -1 && RecTm < PrevJumpTm && (!HasHiddenState || CurrStateId != GetHiddenStateId())) {	// got past time, do not update the statistics
		TNotify::StdNotify->OnNotifyFmt(TNotifyType::ntWarn, "Current time smaller that previous time curr: %ld, prev: %ld", RecTm, PrevJumpTm);
		PrevJumpTm = RecTm;
		return;
	}

	// update intensities
	if (UpdateStats && CurrStateId != -1 && StateId != CurrStateId /*&&
			(!HasHiddenState || CurrStateId != GetHiddenStateId())*/) {	// the state has changed
		UpdateIntensity(CurrStateId, StateId, RecTm - PrevJumpTm);
	}

	if (StateId != CurrStateId) { PrevJumpTm = RecTm; }

	if (HasHiddenState && EndsBatch) {
		UpdateIntensity(StateId, GetHiddenStateId(), RecTm - PrevJumpTm);
	}
}

TFullMatrix TCtMChain::GetFutureProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const {
	return GetFutureProbMat(GetQMatrix(StateSetV), Tm, DeltaTm, HasHiddenState);
}

TFullMatrix TCtMChain::GetPastProbMat(const TVec<TIntV>& StateSetV, const double& Tm) const {
	return GetFutureProbMat(GetRevQMatrix(StateSetV), Tm, DeltaTm, HasHiddenState);
}

void TCtMChain::PrintStats() const {
	TChA ChA;

	for (int i = 0; i < QMatStats.Len(); i++) {
		for (int j = 0; j < QMatStats[i].Len(); j++) {
			ChA += TStr::Fmt("(%ld, %.16f)", QMatStats[i][j].Val1.Val, QMatStats[i][j].Val2.Val);
			if (j < QMatStats[i].Len()-1) {
				ChA += ",";
			}
		}
		ChA += "\n";
	}

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "QMatrix statistics:\n%s", ChA.CStr());
}

TFullMatrix TCtMChain::GetQMatrix() const {
	// compute the intensities
	const int NStates = QMatStats.Len();

	PrintStats();

	printf("\n\nQMatrix statistics:\n");

	// Q-matrix: holds jump intensities
	TFullMatrix QMatrix(NStates, NStates);
	for (int i = 0; i < NStates; i++) {
		for (int j = 0; j < NStates; j++) {
			printf("(%ld,%.3f)", QMatStats[i][j].Val1.Val, QMatStats[i][j].Val2.Val);
			if (j != i) {
				const uint64 N = QMatStats[i][j].Val1;
				const double Sum = QMatStats[i][j].Val2;

				QMatrix(i,j) = N > 0 ? N / Sum : 0;
			}
		}

		printf("\n");

		const double Q_ii = -QMatrix.RowSum(i);

		EAssertR(Q_ii != 0, "Q_ii has a zero row!");

		QMatrix(i,i) = Q_ii;
	}

	printf("\n");

	return QMatrix;
}

TFullMatrix TCtMChain::GetQMatrix(const TVec<TIntV>& InStateSetV) const {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing joined Q matrix for %d states ...", InStateSetV.Len());

	TVec<TIntV> StateSetV(InStateSetV);

	if (HasHiddenState) {
		InsHiddenState(StateSetV);
	}

	const int NStates = StateSetV.Len();

	TFullMatrix JoinedQMat(NStates, NStates);

	const TFullMatrix QMat = GetQMatrix();
	const TVector StatDist = GetStatDist(QMat, Notify);

	printf("Static distribution:\n%s\n", TStrUtil::GetStr(StatDist.Vec, ", ", "%.15f").CStr());

	for (int JoinState1Idx = 0; JoinState1Idx < NStates; JoinState1Idx++) {
		const TIntV& JoinState1 = StateSetV[JoinState1Idx];
		for (int JoinState2Idx = 0; JoinState2Idx < NStates; JoinState2Idx++) {
			if (JoinState1Idx == JoinState2Idx) { continue; }

			const TIntV& JoinState2 = StateSetV[JoinState2Idx];

			// the transition probability from set Ai to Aj can be
			// calculated as: q_{A_i,A_j} = \frac {\sum_{k \in A_i} \pi_k * \sum_{l \in A_j} q_{k,l}} {\sum_{k \in A_i} \pi_k}

			double Sum = 0, SumP = 0;
			for (int k = 0; k < JoinState1.Len(); k++) {
				const int StateK = JoinState1[k];
				const double PiK = StatDist[JoinState1[k]];

				double SumK = 0;
				for (int l = 0; l < JoinState2.Len(); l++) {
					const int StateL = JoinState2[l];
					const double Q_kl = QMat(StateK,StateL);
					SumK += Q_kl;
				}

				Sum += PiK*SumK;
				SumP += PiK;
			}

			JoinedQMat(JoinState1Idx, JoinState2Idx) = Sum / SumP;
		}

		const double Q_ii = -JoinedQMat.RowSum(JoinState1Idx);
		EAssertR(Q_ii != 0, "Joined QMatrix has zero on diagonal!");
		JoinedQMat(JoinState1Idx, JoinState1Idx) = Q_ii;
	}

	printf("Joined Q matrix:\n%s\n", TStrUtil::GetStr(JoinedQMat.GetMat(), ", ", "%.3f").CStr());

	return JoinedQMat;
}

TFullMatrix TCtMChain::GetRevQMatrix(const TVec<TIntV>& StateSetV) const {
	const int n = StateSetV.Len();
	const TFullMatrix QMat = GetQMatrix(StateSetV);
	const TVector StatDist = GetStatDist(QMat, Notify);

	TFullMatrix QRev(n,n);

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			QRev(j,i) = QMat(i,j) * StatDist[i] / StatDist[j];
		}
	}

	return QRev;
}

TVector TCtMChain::GetHoldingTimeV(const TFullMatrix& QMat) const {
	const int Rows = QMat.GetRows();

	TVector HoldTmV(Rows);
	for (int i = 0; i < Rows; i++) {
		HoldTmV[i] = -1 / QMat(i,i);
	}

	return HoldTmV;
}

void TCtMChain::UpdateIntensity(const int& FromStateId, const int& ToStateId, const double& Tm) {
	EAssertR(HasHiddenState || FromStateId != GetHiddenStateId(), "Cannot model hidden state if deactivated!");

	if (HasHiddenState && FromStateId == GetHiddenStateId()) {
		QMatStats[FromStateId][ToStateId].Val1 = 1;
		QMatStats[FromStateId][ToStateId].Val2 = 1 / HIDDEN_STATE_INTENSITY;
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Updated intensity from the hidden state to %d", ToStateId);
	} else {
		const double CorrTm = TMath::Mx(Tm / TimeUnit, MIN_JUMP_TM);
		QMatStats[FromStateId][ToStateId].Val1++;
		QMatStats[FromStateId][ToStateId].Val2 += CorrTm;
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Updated intensity: prev state: %d, curr state: %d, time: %.16f", FromStateId, ToStateId, CorrTm);
	}
}

void TCtMChain::GetNextStateProbV(const TFullMatrix& QMat, const TIntV& StateIdV,
		const int& StateId, TIntFltPrV& StateIdProbV, const int& NFutStates, const PNotify& Notify) {

	const int Dim = QMat.GetRows();

	const int NFStates = TMath::Mn(NFutStates, Dim-1);
	const int StateIdx = StateIdV.SearchForw(StateId);

	EAssertR(StateIdx >= 0, "TCtMChain::GetNextStateProbV: Could not find target state!");

	Notify->OnNotify(TNotifyType::ntInfo, "Fetching future states ...");

	const TFullMatrix JumpMat = GetJumpMatrix(QMat);
	const TVector ProbVec = JumpMat.GetRow(StateIdx);

	// TODO can be optimized
	TIntSet TakenIdxSet;

	double MxProb;
	int MxIdx;
	for (int i = 0; i < NFStates; i++) {
		MxProb = TFlt::Mn;
		MxIdx = -1;
		for (int j = 0; j < ProbVec.Len(); j++) {
			if (j != StateIdx && !TakenIdxSet.IsKey(j) && ProbVec[j] > MxProb) {
				MxProb = ProbVec[j];
				MxIdx = j;
			}
		}

		if (ProbVec[MxIdx] <= 0) { break; }

		TakenIdxSet.AddKey(MxIdx);
		StateIdProbV.Add(TIntFltPr(StateIdV[MxIdx], MxProb));
	}
}

TVector TCtMChain::GetStatDist(const TFullMatrix& QMat, const PNotify& Notify) {
	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing static distribution of %d states ...", QMat.GetRows());

	// THE PROPER WAY

	// returns the stationary distribution
	// pi*Q = 0
	TVector EigenVec(QMat.GetRows(), false);
	TNumericalStuff::GetEigenVec(QMat.GetT().GetMat(), 0.0, EigenVec.Vec);

	const double EigSum = EigenVec.Sum();

	EAssertR(EigSum != 0, "Eigenvector should not be 0, norm is " + TFlt::GetStr(EigenVec.Norm()) + "!");
	EAssertR(!TFlt::IsNan(EigSum), "NaNs in eigenvector!");

//	// THE INPROPER WAY TODO use the proper way, once linalg is fixed
//
//	// Norris: Markov Chains states:
//	// Let Q be a Q-matrix with jump matrix Pi and let lambda be a measure,
//	// than the following are equivalent
//	// 1) lambda is invariant
//	// 2) mu*Pi = mu where mu_i = lambda_i / q_i, where q_i = -q_ii
//
//
//
//	TFullMatrix JumpMat = GetJumpMatrix(QMat);
//
//	// find the eigenvector of the jump matrix with eigen value 1
//	TVector EigenVec(QMat.GetRows(), false);
//	TNumericalStuff::GetEigenVec(JumpMat.GetT().GetMat(), 1.0, EigenVec.Vec);
//
//	// divide the elements by q_i
//	for (int i = 0; i < QMat.GetRows(); i++) {
//		EigenVec[i] /= -QMat(i,i);
//	}
//
//	const double EigSum = EigenVec.Sum();
//
//	EAssertR(EigSum != 0, "Eigenvector should not be 0, norm is " + TFlt::GetStr(EigenVec.Norm()) + "!");
//	EAssertR(!TFlt::IsNan(EigSum), "NaNs in eigenvector!");


	//===========================================================
	// TODO remove this assertion after you know this works
	const double PiQNorm = (EigenVec * QMat).Norm();
	EAssertR(PiQNorm < 1e-3, "This is not an eigenvector with eigenvalue 0");
	//===========================================================

	// normalize to get a distribution
	return EigenVec /= EigSum;
}

TFullMatrix TCtMChain::GetFutureProbMat(const TFullMatrix& QMat, const double& Tm,
		const double& DeltaTm, const bool HasHiddenState) {
	EAssertR(Tm >= 0, "TCtMChain::GetFutureProbMat: does not work for negative time!");

	const int Dim = QMat.GetRows();

	if (Tm == 0) { return TFullMatrix::Identity(Dim); }


	const double QMatNorm = QMat.FromNorm();
	const double Dt = TMath::Mn(DeltaTm / QMatNorm, DeltaTm);

	const int Steps = (int) ceil(Tm / Dt);

	TFullMatrix ProbMat = TFullMatrix::Identity(Dim) + QMat*Dt;

	// the probabilities from state i to the hidden state should now go from i to i
	if (HasHiddenState) {
		const int Dim = ProbMat.GetRows()-1;

		TFullMatrix CorrProbMat = ProbMat(TVector::Range(Dim), TVector::Range(Dim));
		for (int RowIdx = 0; RowIdx < Dim; RowIdx++) {
			const double HiddenProb = ProbMat(RowIdx, Dim);
			CorrProbMat(RowIdx, RowIdx) += HiddenProb;
		}

		ProbMat = CorrProbMat;
	}

	return ProbMat^Steps;
}

TFullMatrix TCtMChain::GetJumpMatrix(const TFullMatrix& QMat) {
	const int Rows = QMat.GetRows();
	const int Cols = QMat.GetCols();

	TFullMatrix JumpMat(Rows, Cols);

	printf("\n\n%s\n", TStrUtil::GetStr(QMat.GetMat()).CStr());

	double Q_ij, Q_ii, J_ij;

	for (int i = 0; i < Rows; i++) {
		if (QMat(i,i) == 0.0) {
			JumpMat(i,i) = 1;
		} else {
			for (int j = 0; j < Cols; j++) {
				if (j != i) {
					Q_ij = QMat(i,j);
					Q_ii = -QMat(i,i);
					J_ij = Q_ij / Q_ii;

					EAssertR(!TFlt::IsNan(J_ij), "Jump matrix contains nan on indexes " + TInt::GetHexStr(i) +", " + TInt::GetStr(j));

					JumpMat(i,j) = J_ij;
				}
			}
		}
	}

	return JumpMat;
}

////////////////////////////////////////////////
// State assistant
TStateAssist::TStateAssist(const bool _Verbose):
		ClassifyV(),
		Rnd(1),
		Verbose(_Verbose),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {}

TStateAssist::TStateAssist(TSIn& SIn):
		ClassifyV(SIn),
		Rnd(SIn),
		Verbose(TBool(SIn)) {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void TStateAssist::Save(TSOut& SOut) const {
	ClassifyV.Save(SOut);
	Rnd.Save(SOut);
	TBool(Verbose).Save(SOut);
}

void TStateAssist::Init(const TFullMatrix& X, const PFullClust& Clust, const PHierarch& Hierarch) {
	// get all the heights from the hierarchy
	TIntFltPrV StateIdHeightPrV;	Hierarch->GetStateIdHeightPrV(StateIdHeightPrV);
	TVector AssignV = Clust->Assign(X);

	Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist, total states %d ...", StateIdHeightPrV.Len());

	for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
		const TIntFltPr& StateIdHeightPr = StateIdHeightPrV[i];
		const int StateId = StateIdHeightPr.Val1;
		const double Height = StateIdHeightPr.Val2;

		ClassifyV.Add(TLogReg(10), true);

		Notify->OnNotifyFmt(TNotifyType::ntInfo, "Computing state assist for state %d ...", StateId);

		TIntV StateIdV; TVec<TIntV> StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);

		const int StateIdx = StateIdV.SearchForw(StateId);

		EAssertR(StateIdx >= 0, "Could not find the target state!");

		const TIntSet TargetStateSet(StateSetV[StateIdx]);

		TIntV TargetIdxV;	AssignV.Find([&](const TFlt& StateId) { return TargetStateSet.IsKey(TInt(StateId)); }, TargetIdxV);
		TIntV NonTargetIdxV;	AssignV.Find([&](const TFlt& StateId) { return !TargetStateSet.IsKey(TInt(StateId)); }, NonTargetIdxV);

		if (TargetIdxV.Len() == 0 || NonTargetIdxV.Len() == 0) continue;

		// make the sets equally sized
		if (NonTargetIdxV.Len() > TargetIdxV.Len()) {
			NonTargetIdxV.Shuffle(Rnd);
			NonTargetIdxV.Trunc(TargetIdxV.Len());
		} else if (TargetIdxV.Len() > NonTargetIdxV.Len()) {
			TargetIdxV.Shuffle(Rnd);
			TargetIdxV.Trunc(NonTargetIdxV.Len());
		}

		// get the instances
		TFullMatrix PosInstMat = X(TVector::Range(X.GetRows()), TargetIdxV);
		TFullMatrix NegInstMat = X(TVector::Range(X.GetRows()), NonTargetIdxV);

		TFltVV InstanceMat(X.GetRows(), PosInstMat.GetCols() + NegInstMat.GetCols());
		TFltV y(PosInstMat.GetCols() + NegInstMat.GetCols(), PosInstMat.GetCols() + NegInstMat.GetCols());
		for (int ColIdx = 0; ColIdx < PosInstMat.GetCols(); ColIdx++) {
			for (int RowIdx = 0; RowIdx < X.GetRows(); RowIdx++) {
				InstanceMat(RowIdx, ColIdx) = PosInstMat(RowIdx, ColIdx);
			}
			y[ColIdx] = 1;
		}
		for (int ColIdx = 0; ColIdx < NegInstMat.GetCols(); ColIdx++) {
			for (int RowIdx = 0; RowIdx < X.GetRows(); RowIdx++) {
				InstanceMat(RowIdx, PosInstMat.GetCols() + ColIdx) = NegInstMat(RowIdx, ColIdx);
			}
			y[PosInstMat.GetCols() + ColIdx] = 0;
		}

		// TODO include the intercept
		ClassifyV.Last().Fit(InstanceMat, y);
	}
}

void TStateAssist::GetSuggestFtrs(const int& StateId, TFltV& WgtV) const {
	EAssertR(0 <= StateId && StateId < ClassifyV.Len(), "Invalid state ID!");

	const TLogReg& Classify = ClassifyV[StateId];
	Classify.GetWgtV(WgtV);
}

/////////////////////////////////////////////////////////////////
// Hierarchical continous time Markov Chain
THierarchCtmc::THierarchCtmc():
		Clust(nullptr),
		MChain(nullptr),
		Hierarch(nullptr),
		StateAssist(nullptr),
		Verbose(true),
		Callback(nullptr),
		Notify(nullptr) {}

THierarchCtmc::THierarchCtmc(const PFullClust& _Clust, const PMChain& _MChain,
		const PHierarch& _Hierarch, const bool& _Verbose):
		Clust(_Clust),
		MChain(_MChain),
		Hierarch(_Hierarch),
		StateAssist(new TStateAssist(_Verbose)),
		Verbose(_Verbose),
		Callback(nullptr),
		Notify(_Verbose ? TNotify::StdNotify : TNotify::NullNotify) {
}

THierarchCtmc::THierarchCtmc(TSIn& SIn):
	Clust(TFullClust::Load(SIn)),
	MChain(TMChain::Load(SIn)),
	Hierarch(THierarch::Load(SIn)),
//	StateAssist(new TStateAssist(true)),//StateAssist(StateAssist::Load(SIn)), FIXME
	StateAssist(new TStateAssist(SIn)),	// TODO
	Verbose(TBool(SIn)),
	Callback(nullptr),
	Notify() {

	Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
}

void THierarchCtmc::Save(TSOut& SOut) const {
	Notify->OnNotify(TNotifyType::ntInfo, "THierarchCtmc::Save: saving to stream ...");

	Clust->Save(SOut);
	MChain->Save(SOut);
	Hierarch->Save(SOut);
	StateAssist->Save(SOut);
	TBool(Verbose).Save(SOut);
}

PJsonVal THierarchCtmc::SaveJson() const {
	Notify->OnNotify(TNotifyType::ntInfo, "THierarchCtmc::SaveJson: saving JSON ...");

	PJsonVal Result = TJsonVal::NewArr();

	// we need to build a hierarchy and model state transitions
	// on each level of the hierarchy

	// variables
	TVec<TIntV> StateSetV;
	TIntV StateIdV;
	TIntFltPrV StateIdProbPrV;

	Hierarch->PrintHierarch();

	TFltV UniqueHeightV;	Hierarch->GetUniqueHeightV(UniqueHeightV);

	// go through all the heights except the last one, which is not interesting
	// since it is only one state
	for (int HeightIdx = 0; HeightIdx < UniqueHeightV.Len()-1; HeightIdx++) {
		const double CurrHeight = UniqueHeightV[HeightIdx];

		PJsonVal LevelJsonVal = TJsonVal::NewObj();

		StateIdV.Clr();
		StateSetV.Clr();
		StateIdProbPrV.Clr();

		// get the states on this level
		Hierarch->GetStateSetsAtHeight(CurrHeight, StateIdV, StateSetV);

		// ok, now that I have all the states I need their expected staying times
		// and transition probabilities
		// iterate over all the parent states and get the joint staying times of their
		// chindren
		TFullMatrix TransitionMat = MChain->GetTransitionMat(StateSetV);
		TVector StateSizeV = MChain->GetStateSizeV(StateSetV).Map([&](const TFlt& Val) { return Val*(CurrHeight + .1); });
		TVector HoldingTimeV = MChain->GetHoldingTimeV(StateSetV);

		// construct state JSON
		PJsonVal StateJsonV = TJsonVal::NewArr();
		for (int i = 0; i < StateIdV.Len(); i++) {
			const int StateId = StateIdV[i];
			const TFltPr& StateCoords = Hierarch->GetStateCoords(StateId);

			PJsonVal StateJson = TJsonVal::NewObj();
			StateJson->AddToObj("id", StateId);
			StateJson->AddToObj("x", StateCoords.Val1);
			StateJson->AddToObj("y", StateCoords.Val2);
			StateJson->AddToObj("size", StateSizeV[i]);
			StateJson->AddToObj("holdingTime", HoldingTimeV[i]);

			if (Hierarch->IsStateNm(StateId)) {
				StateJson->AddToObj("name", Hierarch->GetStateNm(StateId));
			}

			StateJsonV->AddToArr(StateJson);
		}

		// construct tansition JSON
		PJsonVal JumpMatJson = TJsonVal::NewArr();
		for (int RowIdx = 0; RowIdx < TransitionMat.GetRows(); RowIdx++) {
			PJsonVal RowJson = TJsonVal::NewArr();

			for (int ColIdx = 0; ColIdx < TransitionMat.GetCols(); ColIdx++) {
				RowJson->AddToArr(TransitionMat(RowIdx, ColIdx));
			}

			JumpMatJson->AddToArr(RowJson);
		}

		LevelJsonVal->AddToObj("height", CurrHeight);
		LevelJsonVal->AddToObj("states", StateJsonV);
		LevelJsonVal->AddToObj("transitions", JumpMatJson);

		Result->AddToArr(LevelJsonVal);
	}

	return Result;
}

void THierarchCtmc::Init(const TFullMatrix& X, const TUInt64V& RecTmV) {
	InitClust(X);
	InitMChain(X, RecTmV, false, TBoolV());
	InitHierarch();
	InitStateAssist(X);
}

void THierarchCtmc::InitBatches(const TFullMatrix& X, const TUInt64V& RecTmV,
		const TBoolV& BatchEndV) {
	CheckBatches(BatchEndV);

	InitClust(X);
	InitMChain(X, RecTmV, true, BatchEndV);
	InitHierarch();
	InitStateAssist(X);
}

void THierarchCtmc::InitClust(const TFullMatrix& X) {
	Clust->Init(X);
}

void THierarchCtmc::InitMChain(const TFullMatrix& X, const TUInt64V& RecTmV,
		const bool IsBatchData, const TBoolV& EndBatchV) {
	TIntV AssignV;	Clust->Assign(X, AssignV);
	MChain->Init(Clust->GetClusts(), AssignV, RecTmV, IsBatchData, EndBatchV);
}

void THierarchCtmc::InitHierarch() {
	Hierarch->Init(Clust->GetCentroidMat(), MChain->GetCurrStateId());
}

void THierarchCtmc::InitHistograms(TFltVV& InstMat) {
	Clust->InitHistogram(TFullMatrix(InstMat, true));
}

void THierarchCtmc::InitStateAssist(const TFullMatrix& X) {
	StateAssist->Init(X, Clust, Hierarch);
}

void THierarchCtmc::OnAddRec(const uint64 RecTm, const TFltV& Rec) {
	TVector FtrVec(Rec);	// TODO copying

	const int OldStateId = MChain->GetCurrStateId();
	const int NewStateId = Clust->Assign(FtrVec);

	DetectAnomalies(OldStateId, NewStateId, FtrVec);

	if (NewStateId != -1) {
		MChain->OnAddRec(NewStateId, RecTm, false, false);

		if (NewStateId != OldStateId && Callback != nullptr) {
			Hierarch->UpdateHistory(NewStateId);

			TIntFltPrV CurrStateV;	GetCurrStateAncestry(CurrStateV);
			Callback->OnStateChanged(CurrStateV);
		}
	}
}

void THierarchCtmc::GetFutStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	EAssertR(Tm >= 0, "Time should be greater than 0!");

	try {
		TVec<TIntV> JoinedStateVV;
		TIntV StateIdV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, JoinedStateVV);
		MChain->GetFutureProbV(JoinedStateVV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::GetFutStateProbs: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetPastStateProbV(const double& Height, const int& StateId, const double& Tm,
		TIntFltPrV& StateIdProbPrV) const {
	try {
		TVec<TIntV> StateSetV;
		TIntV StateIdV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
		MChain->GetPastProbV(StateSetV, StateIdV, StateId, Tm, StateIdProbPrV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::GetPastStateProbV: Failed to compute past state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetNextStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TVec<TIntV> JoinedStateVV;
		TIntV StateIdV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, JoinedStateVV);
		MChain->GetNextStateProbV(JoinedStateVV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetNextStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetPrevStateProbV(const double& Height, const int& StateId, TIntFltPrV& StateIdProbV) const {
	try {
		TVec<TIntV> StateSetV;
		TIntV StateIdV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
		MChain->GetPrevStateProbV(StateSetV, StateIdV, StateId, StateIdProbV, StateIdV.Len()-1);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetPrevStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetProbVOverTm(const double& Height, const int& StateId, const double StartTm, const double EndTm, const double& DeltaTm, TIntV& StateIdV, TVec<TFltV>& FutProbV, TVec<TFltV>& PastProbV) const {
	try {
		TVec<TIntV> StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
		MChain->GetProbVOverTm(Height, StateId, StartTm, EndTm, DeltaTm, StateSetV, StateIdV, FutProbV, PastProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetPrevStateProbV: Failed to compute future state probabilities: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetHistStateIdV(const double& Height, TIntV& StateIdV) const {
	try {
		Hierarch->GetHistStateIdV(Height, StateIdV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistStateIdV: Failed to compute fetch historical states: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetHistogram(const int& StateId, const int& FtrId, TFltV& BinStartV, TFltV& ProbV) const {
	try {
		TIntV LeafV;
		Hierarch->GetLeafDescendantV(StateId, LeafV);
		Clust->GetHistogram(FtrId, LeafV, BinStartV, ProbV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarch::GetHistogram: Failed to fetch histogram: %s", Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetStateWgtV(const int& StateId, TFltV& WgtV) const {
	try {
		StateAssist->GetSuggestFtrs(StateId, WgtV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::GetStateWgtV: Failed to fetch weight vector for state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::GetTransitionModel(const double& Height, TFltVV& Mat) const {
	TIntV StateIdV;
	TVec<TIntV> StateSetV;
	Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	Mat = MChain->GetModel(StateSetV).GetMat();
}

void THierarchCtmc::GetStateAncestry(const int& StateId, TIntFltPrV& StateIdHeightPrV) const {
	Hierarch->GetAncestorV(StateId, StateIdHeightPrV);
}

void THierarchCtmc::GetCurrStateAncestry(TIntFltPrV& StateIdHeightPrV) const {
	return Hierarch->GetCurrStateIdHeightPrV(StateIdHeightPrV);
}

int THierarchCtmc::GetCurrStateId(const double& Height) const {
	return Hierarch->GetAncestorAtHeight(MChain->GetCurrStateId(), Height);
}

void THierarchCtmc::GetCentroid(const int& StateId, TFltV& FtrV) const {
	TIntV LeafIdV;	Hierarch->GetLeafDescendantV(StateId, LeafIdV);
	TVector Centroid = Clust->GetJoinedCentroid(LeafIdV);
	FtrV = Centroid.Vec;
}

void THierarchCtmc::GetStateIdVAtHeight(const double& Height, TIntV& StateIdV) const {
	try {
		TVec<TIntV> StateSetV;
		Hierarch->GetStateSetsAtHeight(Height, StateIdV, StateSetV);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::THierarchCtmc::GetStateIdVAtHeight: Failed to fetch state IDs for height %.3f: %s", Height, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::SetStateNm(const int& StateId, const TStr& StateNm) {
	try {
		Hierarch->SetStateNm(StateId, StateNm);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::SetStateNm: Failed to set name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

const TStr& THierarchCtmc::GetStateNm(const int& StateId) const {
	try {
		return Hierarch->GetStateNm(StateId);
	} catch (const PExcept& Except) {
		Notify->OnNotifyFmt(TNotifyType::ntErr, "THierarchCtmc::GetStateNm: Failed to get name of state %d: %s", StateId, Except->GetMsgStr().CStr());
		throw Except;
	}
}

void THierarchCtmc::SetVerbose(const bool& _Verbose) {
	if (_Verbose != Verbose) {
		Verbose = _Verbose;
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}

	Clust->SetVerbose(Verbose);
	MChain->SetVerbose(Verbose);
	Hierarch->SetVerbose(Verbose);
}

void THierarchCtmc::SetCallback(TMcCallback* _Callback) {
	Callback = _Callback;
}

void THierarchCtmc::DetectAnomalies(const int& NewStateId, const int& OldStateId, const TVector& FtrVec) const {
	if (NewStateId != OldStateId) {
		if (MChain->IsAnomalousJump(NewStateId, OldStateId)) {
			Callback->OnAnomaly(TStr::Fmt("Anomalous jump, old state %d, new state %d", OldStateId, NewStateId));
		}
		if (NewStateId == -1) {
			Callback->OnOutlier(FtrVec.Vec);
		}
	}
}

void THierarchCtmc::CheckBatches(const TBoolV& BatchEndV) {
	// check if any batches of length 0 exist
	bool JustEnded = false;
	for (int i = 0; i < BatchEndV.Len(); i++) {
		const bool EndsBatch = BatchEndV[i];
		EAssertR(!EndsBatch || !JustEnded, "Found a batch of length 0. Cannot model such data!");
		JustEnded = EndsBatch;
	}
}

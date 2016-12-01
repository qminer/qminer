/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

using namespace TClustering;

//////////////////////////////////////////////////////
// Distance measures
PDist TDist::Load(TSIn& SIn) {
	const TStr Type(SIn);

	if (Type == TEuclDist::TYPE) {
		return TEuclDist::New();
	} else if (Type == TCosDist::TYPE) {
		return TCosDist::New();
	} else {
		throw TExcept::New("Unknown distance type when loading: " + Type);
	}
}

const TStr TEuclDist::TYPE = "euclidean";
const TStr TCosDist::TYPE = "cos";

///////////////////////////////////////////
// TAbsKMeans

///////////////////////////////////////////
// K-Means

///////////////////////////////////////////
// DPMeans

///////////////////////////////////////////
// Agglomerative clustering - average link
void TAvgLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = (DistMat(MnI, i)*ItemCountV[MnI] + DistMat(MnJ, i)*ItemCountV[MnJ]) / (ItemCountV[MnI] + ItemCountV[MnJ]);
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

///////////////////////////////////////////
// Agglomerative clustering - complete link
void TCompleteLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mx(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

///////////////////////////////////////////
// Agglomerative clustering - single link
void TSingleLink::JoinClusts(TFltVV& DistMat, const TIntV& ItemCountV, const int& MnI, const int& MnJ) {
	TFltV NewDistV(DistMat.GetRows());
	for (int i = 0; i < DistMat.GetRows(); i++) {
		NewDistV[i] = TMath::Mn(DistMat(MnI, i), DistMat(MnJ, i));
	}

	DistMat.SetRow(MnI, NewDistV);
	DistMat.SetCol(MnI, NewDistV);
}

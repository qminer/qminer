/*
 * denseclust.h
 *
 *  Created on: Sep 17, 2014
 *      Author: lstopar
 */

#ifndef CTMC_H_
#define CTMC_H_

namespace TCtmc {

//////////////////////////////////////////////////////
// Distance measures - eucledian distance
class TEuclDist {
public:
	// returns a matrix D of distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist(const TFullMatrix& X, const TFullMatrix& Y);
	// returns a matrix D of squared distances between elements of X to elements of Y
	// X and Y are assumed to have column vectors
	// D_ij is the distance between x_i and y_j
	static TFullMatrix GetDist2(const TFullMatrix& X, const TFullMatrix& Y);
};

class TClust;
typedef TPt<TClust> PClust;
class TClust {
private:
  TCRef CRef;
public:
  friend class TPt<TClust>;
protected:
	TRnd Rnd;
	// holds centroids as column vectors
	TFullMatrix CentroidMat;
	// holds pairs <n,sum> where n is the number of points assigned to the
	// centroid at index i and sum is the sum of distances of all the points
	// assigned to the centroid to the centroid
	TUInt64FltPrV CentroidDistStatV;

public:
	TClust(const TRnd& Rnd=TRnd(0));
	virtual ~TClust() {}

	// Applies the algorithm. Instances should be in the columns of X.
	virtual TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000) = 0;

	const int GetClusts() const { return CentroidMat.GetCols(); }
	const TFullMatrix& GetCentroidMat() const { return CentroidMat; }

	// assign instances to centroids
	int Assign(const TVector& Inst) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	TVector Assign(const TFullMatrix& InstMat) const;

	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
	TFullMatrix GetDistMat(const TFullMatrix& X) const;
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	TVector GetDistVec(const TVector& x) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const int& CentroidIdx, const TVector& Pt) const;


	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidIdx) const;
	// returns the number of points in the cluster
	uint64 GetClustSize(const int& ClustIdx) const;

protected:
	TFullMatrix SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV);
	void UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV);

	// returns a matrix of squared distances
	TFullMatrix GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;
	TVector Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;

	void InitStatistics(const TFullMatrix& X, const TVector& AssignV);
};

class TFullKMeans: public TClust {
private:
	TInt K;

public:
	TFullKMeans(const int& K, const TRnd& Rnd=TRnd(0));

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000);
};

class TDpMeans: public TClust {
private:
	TFlt Lambda;
	TInt MinClusts;
	TInt MaxClusts;	// TODO max clusts is ignored for now

public:
	TDpMeans(const TFlt& Lambda, const TInt& MinClusts=1, const TInt& MaxClusts=TInt::Mx, const TRnd& Rnd=TRnd(0));

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	TFullMatrix Apply(const TFullMatrix& X, TIntV& AssignV, const int& MaxIter=10000);
};

class TAggClust;
typedef TPt<TAggClust> PAggClust;
class TAggClust {
private:
	TCRef CRef;
public:
	friend class TPt<TAggClust>;
public:
	void MakeDendro(const TFullMatrix& X, TIntIntFltTrV& MergeV);
};

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtMChain;
typedef TPt<TCtMChain> PCtMChain;
class TCtMChain {
private:
	TCRef CRef;
public:
	friend class TPt<TCtMChain>;
public:
	const static uint64 TU_SECOND;
	const static uint64 TU_MINUTE;
	const static uint64 TU_HOUR;
	const static uint64 TU_DAY;

private:
//	TFullMatrix StateCentMat;
	TVec<TUInt64FltPrV> QMatStats;

//	// holds pairs <n,sum> where n is the number of points in state i
//	// and sum is the sum of distances to the centroid
//	TVec<TUInt64FltPr> StateStatV;

//	PClust Clust;

	const uint64 TimeUnit;
	int NStates;

	int CurrStateIdx;
	uint64 PrevJumpTm;

public:
	TCtMChain(const uint64 TimeUnit);

	void Init(const int& NStates, const TIntV& StateAssignV, const TUInt64V& TmV);
//	void Init(const TFullMatrix& X, const TUInt64V& RecTmV);

	void OnAddRec(const int& StateIdx, const uint64& RecTm);

	// returns the total number of stats in the system
	int GetStates() const { return NStates; }
	// returns the dimension of the points
//	int GetDim() const { return StateCentMat.GetRows(); }

	// continuous time Markov chain stuff
	// returns the stationary distribution of the stohastic process
	TVector GetStatDist() const;
	// returns a jump matrix for the given transition rate matrix
	// when the process decides to jump the jump matrix describes to
	// which state it will jump with which probability
	static TFullMatrix GetJumpMatrix(const TFullMatrix& QMat);
	// returns a vector of holding times
	// a holding time is the expected time that the process will stay in state i
	// it is an exponential random variable of parameter -q_ii, so its expected value
	// is -1/q_ii
	static TVector GetHoldingTimeV(const TFullMatrix& QMat);

private:
	// init functions
//	void InitStateStats(const TFullMatrix& X);
//	void InitIntensities(const TFullMatrix& X, const TUInt64V& RecTmV, const TIntV& AssignV);

	// update functions
	void UpdateIntensities(const uint64 RecTm, const int& RecState);
//	void UpdateStatistics(const TVector& Rec, const int& RecState);

	// clustering stuff
//	double GetMeanPtCentroidDist(const int& StateIdx) const;
//	uint64 GetStateSize(const int& StateIdx) const;
};

class THierarchCtmc;
typedef TPt<THierarchCtmc> PHierarchCtmc;
class THierarchCtmc {
private:
	TCRef CRef;
public:
	friend class TPt<THierarchCtmc>;
private:
	PClust Clust;
    PCtMChain MChain;
    PAggClust AggClust;

    // a vector which describes the hierarchy. each state has its own index
    // and the value at index i is the index of i-ths parent
    TIntV HierarchV;
    TIntV StateHeightV;

public:
    THierarchCtmc(const PClust& _Clust, const PCtMChain& _MChain, const PAggClust& _AggClust);

    // initializes the model
	void Init(const TFullMatrix& X, const TUInt64V& RecTmV);

	// saves this models as JSON
	PJsonVal SaveJson() const;

private:
	// returns the index of the oldest ancestor of the state
	// this method is only used when initially building the hierarchy
	int GetOldestAncestIdx(const int& StateIdx) const;
};

}

#endif /* CTMC_H_ */

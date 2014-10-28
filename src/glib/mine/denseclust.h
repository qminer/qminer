/*
 * denseclust.h
 *
 *  Created on: Sep 17, 2014
 *      Author: lstopar
 */

#ifndef DENSECLUST_H_
#define DENSECLUST_H_

namespace TFullClust {

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

	const TInt GetClusts() const { return CentroidMat.GetCols(); }

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

class TKMeans: public TClust {
private:
	TInt K;

public:
	TKMeans(const int& K, const TRnd& Rnd=TRnd(0));

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

/////////////////////////////////////////////////////////////////
// Continous time Markov Chain
class TCtmc {
private:
	TFullMatrix CentroidMat;
	TVec<TUInt64FltPrV> QMatStats;

	// holds pairs <n,sum> where n is the number of points in state i
	// and sum is the sum of distances to the centroid
	TVec<TUInt64FltPr> StateStatV;

	PClust Clust;

	TUInt64V RecIdV;

	uint64 TimeUnit;

	int CurrStateIdx;
	uint64 PrevJumpTm;

public:
	TCtmc(const PClust& Clust);

	void Init(const TFullMatrix& X, const TUInt64V& RecTmV);

	void OnAddRec(const TVector& Rec, const uint64& RecTm);

	// returns the total number of stats in the system
	int GetStates() const { return CentroidMat.GetCols(); }
	// returns the dimension of the points
	int GetDim() const { return CentroidMat.GetRows(); }

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
	void InitStateStats(const TFullMatrix& X);
	void InitIntensities(const TFullMatrix& X, const TUInt64V& RecTmV, const TIntV& AssignV);

	// update functions
	void UpdateIntensities(const TVector& Rec, const uint64 RecTm, const int& RecState);
	void UpdateStatistics(const TVector& Rec, const int& RecState);

	// clustering stuff
	double GetMeanPtCentroidDist(const int& StateIdx) const;
	uint64 GetStateSize(const int& StateIdx) const;
};

}

#endif /* DENSECLUST_H_ */

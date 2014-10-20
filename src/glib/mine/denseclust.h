/*
 * denseclust.h
 *
 *  Created on: Sep 17, 2014
 *      Author: lstopar
 */

#ifndef DENSECLUST_H_
#define DENSECLUST_H_

namespace TFullClust {

class TClust {
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

}

#endif /* DENSECLUST_H_ */

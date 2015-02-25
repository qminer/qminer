/*
 * clust.h
 *
 *  Created on: Feb 9, 2015
 *      Author: lstopar
 */

#ifndef SRC_GLIB_MINE_ML_H_
#define SRC_GLIB_MINE_ML_H_

namespace TMl {

class TFullClust;
typedef TPt<TFullClust> PFullClust;
class TFullClust {
private:
  TCRef CRef;
public:
  friend class TPt<TFullClust>;
protected:
  	const static int MX_ITER;

    typedef TPair<TUInt64, TUInt64V> TFtrHistStat;
    typedef TVec<TFtrHistStat> TClustHistStat;
    typedef TVec<TClustHistStat> THistStat;

  	TRnd Rnd;
  	// holds centroids as column vectors
  	TFullMatrix CentroidMat;
  	// holds pairs <n,sum> where n is the number of points assigned to the
  	// centroid at index i and sum is the sum of distances of all the points
  	// assigned to the centroid to the centroid
  	TUInt64FltPrV CentroidDistStatV;

  	int NHistBins;			// the number of bins used in a histogram
  	TFltVV FtrBinStartVV;		// stores where each bin starts	// TODO save/load
  	THistStat HistStat;		// stores histogram for every feature in every cluster

  	double Sample;

  	bool Verbose;
  	PNotify Notify;

  	TFullClust(const int NHistBins, const double& Sample, const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
  	TFullClust(TSIn& SIn);

	virtual ~TFullClust() {}

public:
	// saves the model to the output stream
	virtual void Save(TSOut& SOut) const;
	// loads the model from the output stream
	static PFullClust Load(TSIn& SIn);

	// performs the clustering
	void Init(const TFullMatrix& X);
	// initializes histograms for every feature
	void InitHistogram(const TFullMatrix& X);

	// assign methods
	// assign instances to centroids
	int Assign(const TVector& Inst) const;
	// assign instances to centroids, instances should be in the columns of the matrix
	TVector Assign(const TFullMatrix& InstMat) const;
	void Assign(const TFullMatrix& InstMat, TIntV& AssignV) const;

	// distance methods
	// returns a matrix D with the distance to all the centroids
	// D_ij is the distance between centroid i and instance j
	// points should be represented as columns of X
	TFullMatrix GetDistMat(const TFullMatrix& X) const;
	// Returns a vector y containing the distance to all the
	// centroids. The input vector x should be a column vector
	TVector GetDistVec(const TVector& x) const;
	// returns the distance from the cluster centroid to the point
	double GetDist(const int& CentroidId, const TVector& Pt) const;

	// returns the coordinates of a "joined" centroid
	TVector GetJoinedCentroid(const TIntV& CentroidIdV) const;

	// cluster statistics
	// returns the means distance of all the points assigned to centroid CentroidIdx
	// to that centroid
	double GetMeanPtCentDist(const int& CentroidId) const;
	// returns the number of points in the cluster
	uint64 GetClustSize(const int& ClustId) const;

	void GetHistogram(const int FtrId, const TIntV& StateSet, TFltV& BinStartV, TFltV& BinV) const;

	int GetClusts() const { return CentroidMat.GetCols(); }
	int GetDim() const { return CentroidMat.GetRows(); }
	const TFullMatrix& GetCentroidMat() const { return CentroidMat; }

	// sets the log to verbose or none
	void SetVerbose(const bool& Verbose);

protected:
	// Applies the algorithm. Instances should be in the columns of X.
	virtual void Apply(const TFullMatrix& X, const int& MaxIter=10000) = 0;
	TVector Assign(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;
	// returns a matrix of squared distances
	TFullMatrix GetDistMat2(const TFullMatrix& X, const TVector& NormX2, const TVector& NormC2, const TVector& OnesN, const TVector& OnesK) const;

	// used during initialization
	TFullMatrix SelectInitCentroids(const TFullMatrix& X, const int& NCentroids, TVector& AssignIdxV);
	void UpdateCentroids(const TFullMatrix& X, const TVector& AssignIdxV);
	void InitStatistics(const TFullMatrix& X, const TVector& AssignV);

	// returns the type of this clustering
	virtual const TStr GetType() const = 0;

private:
	// returns the coordinates of the centroid with the specified ID
	TVector GetCentroid(const int& CentroidId) const;
};

///////////////////////////////////////////
// K-Means
class TFullKMeans: public TFullClust {
private:
	TInt K;

public:
	TFullKMeans(const int& NHistBins, const double Sample, const int& K, const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
	TFullKMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFullMatrix& X, const int& MaxIter);

protected:
	const TStr GetType() const { return "kmeans"; }
};


///////////////////////////////////////////
// DPMeans
class TDpMeans: public TFullClust {
private:
	TFlt Lambda;
	TInt MinClusts;
	TInt MaxClusts;

public:
	TDpMeans(const int& NHistBins, const double& Sample, const TFlt& Lambda, const TInt& MinClusts=1, const TInt& MaxClusts=TInt::Mx, const TRnd& Rnd=TRnd(0), const bool& Verbose=false);
	TDpMeans(TSIn& SIn);

	// saves the model to the output stream
	void Save(TSOut& SOut) const;

	// Applies the algorithm. Instances should be in the columns of X. AssignV contains indexes of the cluster
	// the point is assigned to
	void Apply(const TFullMatrix& X, const int& MaxIter);

protected:
	const TStr GetType() const { return "dpmeans"; }
};

///////////////////////////////////////////
// Logistic Regression using the Newton-Raphson method
class TLogReg {
private:
	double Lambda;
	TFltV WgtV;

	bool IncludeIntercept;

	bool Verbose;
	PNotify Notify;

public:
	// default constructor, sets the regularization parameter
	TLogReg(const double& Lambda=1, const bool IncludeIntercept=false, const bool Verbose=true);
	TLogReg(TSIn& SIn);

	void Save(TSOut& SOut) const;

	// Fits the regression model. The method assumes that the instances are stored in the
	// columns of the matrix X and the responses are stored in vector y.
	void Fit(const TFltVV& X, const TFltV& y, const double& Eps=1e-3);

	void GetWgtV(TFltV& WgtV) const;
};


}

#endif /* SRC_GLIB_MINE_ML_H_ */

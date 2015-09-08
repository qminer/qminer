/////////////////////////////////////////////
/// Annomaly Detection methods
namespace TAnomalyDetection {

/////////////////////////////////////////////
/// Nearest Neighbor based Annomaly Detection.
/// Anomaly detector that checks if the test point is too far from the nearest known point/.
class TNearestNeighbor {
private:
    /// Threhsold rates (anonaly percentiles)
    TFltV RateV;
    /// Window size -- number of latest vectors that comprise the neighborhood
    TInt WindowSize;
    /// Neighborhood matrix with members as columns
    TVec<TIntFltKdV> Mat;
    /// Distance to the nearest neighbor for each Mat column
    TFltV DistV;
    /// Index of the column corresponding to the nearest neighbor
    TIntV DistColV;
    /// Thresholds for identifying anomalies
    TFltV ThresholdV;
    /// Number of initial vectors
    TInt InitVecs;
    /// Number of the next available column
    TInt NextCol;

    /// Check if model is initialized
    bool IsInit() const { return (InitVecs == WindowSize); }
    /// Update all distances as if Mat[ColId] is new vector, ignoring column IgnoreColId
    void UpdateDistance(const int& ColId, const int& IgnoreColId = -1);
    /// Forget vector Mat[ColId] from the nearest neighbors
    void Forget(const int& ColId);
    /// Update thresholds
    void UpdateThreshold();

public:
    TNearestNeighbor(const double& Rate, const int& WindowSize);

    TNearestNeighbor(TSIn& SIn);
    void Save(TSOut& SOut);

    /// Add new element to the model
    void PartialFit(const TIntFltKdV& Vec);

    /// Distance to the nearest neighbor
    double DecisionFunction(const TIntFltKdV& Vec) const;
    /// ID of the rate which triggered the anomaly (0 = none, 1 = RateV[0], ...)
    int Predict(const TIntFltKdV& Vec) const;

    // parameters
    int GetRates() const { return RateV.Len(); }
    double GetRate(const int& RateN) const { return RateV[RateN]; }
    double GetThreshold(const int& RateN) const { return IsInit() ? ThresholdV[RateN].Val : 0.0; }
    int GetWindowSize() const { return WindowSize; }
};

};
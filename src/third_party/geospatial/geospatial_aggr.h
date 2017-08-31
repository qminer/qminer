/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/
#ifndef QMINER_GEOSPATIAL_AGGR_H
#define QMINER_GEOSPATIAL_AGGR_H

#include "qminer_core.h"
#include "qminer_storage.h"
#include "qminer_ftr.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

namespace TQm {
namespace TStreamAggrs {

//////////////////////
/// representation of one GPS point
struct TPoint {
    TFlt Lat;
    TFlt Lon;
    TPoint(){}
    TPoint(TFlt lat, TFlt lon) : Lat(lat), Lon(lon) {}
};

//////////////////////
/// representation of one GPS measurement
class TGPSMeasurement {
private:
    //PJsonVal Json;
public:
    TGPSMeasurement() : Speed(-1), Distance(-1){}
    TGPSMeasurement(const PJsonVal& Rec);
    TPoint LatLon;
    TUInt64 Time;//timestamp
    TFlt Accuracy;//accuracy
    TFlt Speed;//given speed by GPS
    TFlt Distance;//distance to previous
    TInt64 TimeDiff;//time difference with previous
	TStr Accelerometer;//accelerometer data
    PJsonVal ToJson() const;
    //Temporal hack untile we create a new aggregate caluclating the 
    //type and avg activity (walking, running) inside a geoActivity (path).
    //the idices are aligned with the exports.GeoActivityActivity 
    //(shared_consts) from NextPin

    TIntV SensorActivities;
    const static TInt NumOfSensorActs; //see shared_consts;
};

//////////////////////
/// provides common Geo utilities
class TGeoUtils {
private:
    const static int R = 6371000;//earth radius in m
public:
    /// Calculates distance betweet 2 geo points based on
    /// equirectangular distance approximation - this is
    /// good enough for small distances
    static double QuickDist(const TPoint& P1, const TPoint& P2);
};

//////////////////////
/// Presents a cluster of points (staypoint)
enum TGeoActivityType { Staytpoint, Path };
enum TGeoActivityStatus { Current, Possible, Detected };
class TGeoCluster {
private:
    TInt MStartIdx;//start pointer to GPS Measurements state array
    TInt MEndIdx;  //end pointer to GPS Measurements state array
    TUInt64 Arrive;
    TUInt64 Depart;
    TPoint CenterPoint;
    TFlt AvgSpeed;
    TFlt AvgAccuracy;
    TFlt Distance;
    TGeoActivityType GeoType;
    TGeoActivityStatus GeoActStatus;
    TFltV AvgSensorActs;
public:
    TGeoCluster() : TGeoCluster(TGeoActivityType::Path) {};
    TGeoCluster(TGeoActivityType _Type) : 
        MStartIdx(-1), 
        MEndIdx(-1), 
        GeoType(_Type), 
        GeoActStatus(TGeoActivityStatus::Current),
        AvgSensorActs(TGPSMeasurement::NumOfSensorActs){};
    TGeoCluster(const PJsonVal& Rec);
    TGeoCluster(const int& StartIdx, const int& EndIdx,
        const TVec<TGPSMeasurement>& StateVec);
    
    void AddPoint(const int& Idx, const TVec<TGPSMeasurement>& _GpsState);
    int64 Duration();
    int Len() const;
    int EndIdx() const { return MEndIdx; }
    int StartIdx() const { return MStartIdx; }
    void DownShiftIdx(int _Num) { MStartIdx -= _Num; MEndIdx -= _Num; }
    //const TPoint& Center() const { return CenterPoint; }
    double QuickDist(const TPoint& P1);
    TGeoActivityType Type() const { return GeoType; }
    void SetStatus(TGeoActivityStatus Status) {
        GeoActStatus = Status;
    }
    TGeoActivityStatus Status() const { return GeoActStatus; }
    PJsonVal ToJson(const TVec<TGPSMeasurement>& _GpsStateVec, 
                    const bool& FullLoc) const;
};

///////////////////////////////
/// StayPoint detector aggregate
class TStayPointDetector : public TStreamAggr {
private:
    /// pointer to store
    TWPt<TStore> Store;
    /// location field id for fast access
    TInt LocationFieldId;
    /// accuracy field id for fast access
    TInt AccuracyFieldId;
    /// time field id for fast access
    TInt TimeFieldId;
	/// speed field id for fast access
	TInt SpeedFieldId;
    /// distance field id for fast access
    TInt DistanceFieldId;
	/// distance field id for fast access
	TInt AccelerometerFieldId;

    /// algorithm parameters
    PJsonVal Params;
    /// distance threshold (meters)
    TInt TrDist;
    /// time threshold (seconds)
    TInt TrTime;
    /// array of activity probabilities (see TGPSMeasurement::sensorActivities
    TInt ActivitiesField;

    /// state
    //list of GPS measurements currently part of the satate
    TVec<TGPSMeasurement> StateGpsMeasurementsV;
    //TGPSMeasurement lastRecord;//holds previous GPS record
    TGeoCluster CL;//cluster from the algorihm (cl)
    TGeoCluster Plocs;//path locs from the algorithm (plocs)
    TVec<TGeoCluster> DetectedGeoActivitiesV;
    bool HasFinishedGeoActs = false;

    /// helper methods
    bool ParseGPSRec(const TRec& Rec, TGPSMeasurement& Gps);
    TGPSMeasurement* PrepareGPSRecord(const TRec& Rec);
protected:
    ///  Updates the stay point model, sets State JSON
    void OnAddRec(const TRec& Rec, const TWPt<TStreamAggr>& CallerAggr);
    /// JSON based constructor.
    TStayPointDetector(const TWPt<TBase>& Base, const PJsonVal& ParamVal);
public:
    /// Smart pointer constructor
    static PStreamAggr New(const TWPt<TBase>& Base, const PJsonVal& ParamVal) {
        return new TStayPointDetector(Base, ParamVal);
    }
    
    /// Loads state
    void LoadState(TSIn& SIn);
    /// Saves state
    void SaveState(TSOut& SOut) const;
    /// Load stream aggregate state from JSON
    void LoadStateJson(const PJsonVal& State);
    /// Save state of stream aggregate and return it as a JSON
    PJsonVal SaveStateJson() const;

    /// Is the aggregate initialized?
    bool IsInit() const;
    /// Resets the aggregate
    void Reset();
    /// JSON serialization
    PJsonVal SaveJson(const int& Limit) const;// { return State; }
                                                /// Stream aggregator type name
    static TStr GetType() { return "stayPointDetector"; }
    /// Stream aggregator type name
    TStr Type() const { return GetType(); }
};

// Register aggregates
void InitGeoSpatial_Aggr() {
    TQm::TStreamAggr::Register<TQm::TStreamAggrs::TStayPointDetector>();
}

INIT_EXTERN_AGGR(InitGeoSpatial_Aggr);

} // TStreamAggrs namespace
} // TQm namespace

#endif

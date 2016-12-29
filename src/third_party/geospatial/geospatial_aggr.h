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

#include <iostream>
#include <vector>// std::vector

namespace TQm {
namespace TStreamAggrs {

//////////////////////
/// representation of one GPS point
struct TPoint {
    TFlt Lat;
    TFlt Lon;
    TPoint() : Lat(0), Lon(0) {}
    TPoint(TFlt lat, TFlt lon) : Lat(lat), Lon(lon) {}
};

//////////////////////
/// representation of one GPS measurement
class TGPSMeasurement {
private:
    //PJsonVal Json;
public:
    TGPSMeasurement();
    TPoint LatLon;
    TUInt64 Time = 0;//timestamp
    TFlt Accuracy = 0;//accuracy
    TFlt Speed = -1;//given speed by GPS
    TFlt Distance = -1;//distance to previous
    TUInt64 TimeDiff = 0;//time difference with previous

    void PrintConsole() {
        std::cout << "GPS:{time:" << Time << "\n";
        std::cout << "lat:" << LatLon.Lat << "\n";
        std::cout << "lon:" << LatLon.Lon << "\n";
        std::cout << "acc:" << Accuracy << "\n";
        std::cout << "speed:" << Speed << "\n";
        std::cout << "dist:" << Distance << "\n";
        std::cout << "timeDiff:" << TimeDiff << "}\n";
    }

    PJsonVal ToJson() const;
};

//////////////////////
/// provides common Geo utilities
class TGeoUtils {
private:
    const static int R = 6371000;//earth radius in m
    const static double Pi;//Pi
public:
    /// Calculates distance betweet 2 geo points based on
    /// equirectangular distance approximation - this is
    /// good enough for small distances
    static double QuickDist(const TPoint& p1, const TPoint& p2);
};

//////////////////////
/// Presents a cluster of points (staypoint)
enum TGeoActivityType { Staytpoint, Path };
enum TGeoActivityStatus { Current, Possible, Detected };
class TGeoCluster {
private:
    TInt MStartIdx = -1;//start pointer to GPS Measurements state array
    TInt MEndIdx = -1;  //end pointer to GPS Measurements state array
    TUInt64 Arrive = 0;
    TUInt64 Depart = 0;
    TPoint CenterPoint = TPoint(0, 0);
    TGeoActivityType GeoType = TGeoActivityType::Path;
    TGeoActivityStatus GeoActStatus = TGeoActivityStatus::Current;
public:
    TGeoCluster(const int StartIdx, const int EndIdx,
        const TVec<TGPSMeasurement>& StateVec);
    TGeoCluster(TGeoActivityType _Type) : GeoType(_Type) {};
    TGeoCluster() : GeoType(TGeoActivityType::Path) {};
    void AddPoint(const TInt Idx, const TVec<TGPSMeasurement>& _GpsState);
    uint Duration();
    uint Len() const;
    uint EndIdx() const { return MEndIdx; }
    uint StartIdx() const { return MStartIdx; }
    void DownShiftIdx(int _Num) { MStartIdx -= _Num; MEndIdx -= _Num; }
    const TPoint& Center() const { return CenterPoint; }
    const TGeoActivityType Type() const { return GeoType; }
    void SetStatus(TGeoActivityStatus _status) {
        GeoActStatus = _status;
    }
    const TGeoActivityStatus Status() const { return GeoActStatus; }
    PJsonVal ToJson(const TVec<TGPSMeasurement>& _GpsStateVec, 
                    const bool fullLoc) const;
};

///////////////////////////////
/// StayPoint detector aggregate
class TStayPointDetector : public TStreamAggr {
private:
    /// state returned with SaveJson
    PJsonVal State;
    /// pointer to store
    TWPt<TStore> Store;
    /// location field id for fast access
    TInt LocationFieldId;
    /// accuracy field id for fast access
    TInt AccuracyFieldId;
    /// time field id for fast access
    TInt TimeFieldId;

    /// algorithm parameters
    PJsonVal Params;
    /// distance threshold (meters)
    TUInt TrDist = 50;
    /// time threshold (seconds)
    TUInt TrTime = 300;

    /// state
    //list of GPS measurements currently part of the satate
    TVec<TGPSMeasurement> StateGpsMeasurements;
    //TGPSMeasurement lastRecord;//holds previous GPS record
    TGeoCluster cl;//cluster from the algorihm (cl)
    TGeoCluster plocs;//path locs from the algorithm (plocs)
    TVec<TGeoCluster> DetectedGeoActivities;
    bool hasFinishedGeoActs = false;

    /// helper methods
    bool parseRecord(const TRec& Rec, TGPSMeasurement& gps);
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


} // TStreamAggrs namespace
} // TQm namespace

#endif
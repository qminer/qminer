/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/

#include "geospatial_aggr.h"

namespace TQm {
namespace TStreamAggrs {

/////////////////////////////
///TGPSMeasurement
TGPSMeasurement::TGPSMeasurement() {
    //Json = TJsonVal::NewObj();
    //Json->PutNull();
}

PJsonVal TGPSMeasurement::ToJson() const {
    //if (Json->IsNull()) {
    PJsonVal Json = TJsonVal::NewObj();
    Json->AddToObj("time", Time);
    Json->AddToObj("latitude", LatLon.Lat);
    Json->AddToObj("longitude", LatLon.Lon);
    Json->AddToObj("accuracy", Accuracy);
    Json->AddToObj("speed", Speed);
    Json->AddToObj("distanceDiff", Distance);
    Json->AddToObj("timeDiff", TimeDiff);
    //}
    return Json;
}


///////////////////////////////
/// TStaypointCluster
void TGeoCluster::AddPoint(const TInt Idx,
    const TVec<TGPSMeasurement>& _StateVec)
{
    const TGPSMeasurement& CurrentGPS = _StateVec.GetVal(Idx);
    if (MEndIdx > -1) {//if this cluster already contains some measurements
        const TGPSMeasurement& Last = _StateVec.GetVal(MEndIdx);
        AssertR(Last.Time < CurrentGPS.Time,
            "New Point must be newr than the last one");
    }
    else {//if first point
        Arrive = CurrentGPS.Time;
        MStartIdx = Idx;
    }

    Depart = CurrentGPS.Time;
    MEndIdx = Idx;
    TInt Len = this->Len();
    //incremental averaging (m_n = m_n-1 + ((a_n-m_n-1)/n)
	//m_n = avg value we want to calculate
	//m_n-1 = previous avg value
	//a_n = current value
	//n = number of records (values)
    CenterPoint.Lat = CenterPoint.Lat +
        ((CurrentGPS.LatLon.Lat - CenterPoint.Lat) / Len);
    CenterPoint.Lon = CenterPoint.Lon +
        ((CurrentGPS.LatLon.Lon - CenterPoint.Lon) / Len);
	AvgSpeed = AvgSpeed +
		((CurrentGPS.Speed - AvgSpeed) / Len);
	AvgAccuracy = AvgAccuracy +
		((CurrentGPS.Accuracy - AvgAccuracy) / Len);
	
	//distance
	Distance = Distance + CurrentGPS.Distance;
}//TGeoCluster::addPoint

/// returns duration in seconds
uint TGeoCluster::Duration() {
    return (int)(Depart - Arrive) / 1000;
}//TGeoCluster::duration


uint TGeoCluster::Len() const {
    if (MEndIdx < 0 || MStartIdx < 0) return 0;
    return MEndIdx - MStartIdx + 1;
}//TGeoCluster::size

TGeoCluster::TGeoCluster(const int StartIdx, const int EndIdx,
    const TVec<TGPSMeasurement>& StateVec)
{
    for (int Idx = StartIdx; Idx <= EndIdx; Idx++) {
        this->AddPoint(Idx, StateVec);
    }
}

PJsonVal TGeoCluster::ToJson(const TVec<TGPSMeasurement>& _GpsStateVec,
    const bool fullLoc) const
{
    PJsonVal JGeoAct = TJsonVal::NewObj();
    if (Type() == TGeoActivityType::Path) {
        JGeoAct->AddToObj("type", "P");
    }
    else {
        JGeoAct->AddToObj("type", "S");
    }
    JGeoAct->AddToObj("status", Status());
    JGeoAct->AddToObj("latitude", Center().Lat);
    JGeoAct->AddToObj("longitude", Center().Lon);
    JGeoAct->AddToObj("start_time", Arrive);
    JGeoAct->AddToObj("end_time", Depart);
    JGeoAct->AddToObj("duration", (Depart - Arrive) / 1000);
    JGeoAct->AddToObj("locationsNum", this->Len());
	JGeoAct->AddToObj("avgSpeed", AvgSpeed);
	JGeoAct->AddToObj("avgAccuracy", AvgAccuracy);
	JGeoAct->AddToObj("distance", Distance);

    if (fullLoc) {
        PJsonVal JLocs = TJsonVal::NewArr();
        for (int LocIdx = MStartIdx; LocIdx <= MEndIdx; LocIdx++) {
            const TGPSMeasurement& Gps = _GpsStateVec.GetVal(LocIdx);
            JLocs->AddToArr(Gps.ToJson());
        }
        JGeoAct->AddToObj("locations", JLocs);
    }
    return JGeoAct;
}


///////////////////////////////
/// GeoUtils
const double TGeoUtils::Pi = 3.14159265358979323846;//Pi
double TGeoUtils::QuickDist(const TPoint& p1, const TPoint& p2) {
    double lat1 = p1.Lat*Pi / 180;
    double lon1 = p1.Lon*Pi / 180;
    double lat2 = p2.Lat*Pi / 180;
    double lon2 = p2.Lon*Pi / 180;
    double x = (lon2 - lon1)* cos(0.5*lat2 + lat1);
    double y = lat2 - lat1;
    return R*sqrt(x*x + y*y);
}//QuickDist function


///////////////////////////////
/// StayPoint detector aggregate

TStayPointDetector::TStayPointDetector(
    const TWPt<TBase>& Base,
    const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal)
{
    if (ParamVal->IsObjKey("params")) {
        Params = ParamVal->GetObjKey("params");
        if (Params->IsObjKey("dT")) {
            if (!Params->GetObjKey("dT")->IsNum()) {
                throw TQmExcept::New("param dT must be a Number");
            }
            else {
                TrDist = Params->GetObjKey("dT")->GetUInt();
            }
        }
        if (Params->IsObjKey("tT")) {
            if (!Params->GetObjKey("tT")->IsNum()) {
                throw TQmExcept::New("param tT must be a Number");
            }
            else {
                TrTime = Params->GetObjKey("tT")->GetUInt();
            }
        }
    }//if params given

     ///support (fast access of the fields)
    TStr StoreNm = ParamVal->GetObjStr("store");
    Store = Base->GetStoreByStoreNm(StoreNm);
    TStr TimeFieldName = ParamVal->GetObjStr("timeField");
    TimeFieldId = Store->GetFieldId(TimeFieldName);
    TStr LocationFieldName = ParamVal->GetObjStr("locationField");
    LocationFieldId = Store->GetFieldId(LocationFieldName);
    TStr AccuracyFieldName = ParamVal->GetObjStr("accuracyField");
    AccuracyFieldId = Store->GetFieldId(AccuracyFieldName);

    //init the state in case saveJson is called before onAdd();
    State = TJsonVal::NewArr();
}//TStayPointDetector::constructor

void TStayPointDetector::OnAddRec(const TRec& Rec,
    const TWPt<TStreamAggr>& CallerAggr)
{
    TScopeStopWatch StopWatch(ExeTm);//auto stopwatch for the QM aggregate
                                        //parse new GPS record from JSON

    int NumOfRecordsToForget = 0;
    hasFinishedGeoActs = false;
    //delete previously detected stuff
    for (int i = 0; i < DetectedGeoActivities.Len(); i++) {
        TGeoCluster& clstr = DetectedGeoActivities.GetVal(i);
        NumOfRecordsToForget = clstr.EndIdx();
    }
    if (NumOfRecordsToForget) {
        DetectedGeoActivities.Clr();
        StateGpsMeasurements.Del(0, NumOfRecordsToForget);
        //+1 because it deletes 0 IDX as well
        cl.DownShiftIdx(NumOfRecordsToForget + 1);
		plocs.DownShiftIdx(NumOfRecordsToForget + 1);
    }
    TGPSMeasurement* NewRec = PrepareGPSRecord(Rec);
    if (NewRec == NULL) {//rejected record
        return;
    }

    TInt CurrStateIdx = StateGpsMeasurements.Len() - 1;

    State = TJsonVal::NewArr();

    /* Implementation of ETC (named by Chinese paper):
    *  Extracting places from traces of locations:
    *  Kang, J. H., Welbourne, W., Stewart, B., & Borriello, G. (2005).
    *  Extracting places from traces of locations. ACM SIGMOBILE Mobile
    *  Computing and Communications Review, 9(3), 58.
    *  http://doi.org/10.1145/1094549.1094558 */
    if (TGeoUtils::QuickDist(cl.Center(), NewRec->LatLon) < TrDist) {	   //01
        cl.AddPoint(CurrStateIdx, StateGpsMeasurements);			       //02
        plocs = TGeoCluster(TGeoActivityType::Path);	        	       //03
    }
    else {							    							       //04
        if (plocs.Len() > 1) {										       //05
            if (cl.Duration() > TrTime) {						           //06
                cl.SetStatus(TGeoActivityStatus::Detected);
                DetectedGeoActivities.Add(cl);						       //07
                hasFinishedGeoActs = true;
            }
            cl = TGeoCluster(TGeoActivityType::Staytpoint);    		       //08
            cl.AddPoint(plocs.EndIdx(), StateGpsMeasurements);	           //09
            plocs = TGeoCluster(TGeoActivityType::Path);				   //10
            if (TGeoUtils::QuickDist(cl.Center(), NewRec->LatLon) < TrDist) {//11
                cl.AddPoint(CurrStateIdx, StateGpsMeasurements);		   //12
                plocs = TGeoCluster(TGeoActivityType::Path);               //13??? not needed
            }
            else {													       //14
                plocs.AddPoint(CurrStateIdx, StateGpsMeasurements);		   //15
            }
        }//if plcocs len >1
        else															   //16
        {
            plocs.AddPoint(CurrStateIdx, StateGpsMeasurements);			   //17
        }
    }
}//TStayPointDetector::OnAddRec

    /// save Json - get current state
PJsonVal TStayPointDetector::SaveJson(const int& Limit) const
{
    TGeoCluster Path;
    int LastIdx = StateGpsMeasurements.Len() - 1;
    int StartIdx = 0; int EndIdx = 0;
    //if there is detected cluster, we know that this cluster and path
    //before (if exists), are of status 2
    if (DetectedGeoActivities.Len() > 0) {
        const TGeoCluster& DetectedStp = DetectedGeoActivities.GetVal(0);
        if (DetectedStp.StartIdx() > 0) {//if there is a path before this stp
            EndIdx = DetectedStp.StartIdx() - 1;//We need to add Path first
            Path = TGeoCluster(StartIdx, EndIdx, StateGpsMeasurements);
            Path.SetStatus(TGeoActivityStatus::Detected);
            State->AddToArr(Path.ToJson(StateGpsMeasurements, 
                hasFinishedGeoActs));
        }
        State->AddToArr(DetectedStp.ToJson(StateGpsMeasurements, 
                hasFinishedGeoActs));
        StartIdx = DetectedStp.EndIdx() + 1;
    }//if detected staypoint
        //if plocs locations are earlier than cl, we simply add them to the path
    if (cl.Len() == 0) {//if CL or CL and PLOCS are empty
        EndIdx = LastIdx;
    }
    //cl will be more also if plocs empty
    else if (plocs.StartIdx() < cl.StartIdx()) {
        EndIdx = plocs.EndIdx();
    }
    else {
        EndIdx = cl.StartIdx() - 1;
    }

    //there could be a path between detectedSTP and cl or plocs. This also
    //covers scenario when there is no detectedSTP and all up to here is a path
    if (StartIdx <= EndIdx) {
        Path = TGeoCluster(StartIdx, EndIdx, StateGpsMeasurements);
        State->AddToArr(Path.ToJson(StateGpsMeasurements, hasFinishedGeoActs));
    }

    if (cl.Len() > 0) {
        State->AddToArr(cl.ToJson(StateGpsMeasurements, hasFinishedGeoActs));
        EndIdx = cl.EndIdx() + 1;
    }

    if (EndIdx < LastIdx) {
        Path = TGeoCluster(EndIdx, LastIdx, StateGpsMeasurements);
        State->AddToArr(Path.ToJson(StateGpsMeasurements, hasFinishedGeoActs));
    }
    return State;
}

/// Helper function for easier GPSRecord creation. It assigns values to new
/// TGPSRecord, compares it to the previous record and then re-stores it as
/// lastRecord, for next parsing. If parse is not successfull, it returns false
/// otherwise True
bool TStayPointDetector::parseRecord(const TRec& Rec, TGPSMeasurement& gps) {
    TTm Timestamp; Rec.GetFieldTm(TimeFieldId, Timestamp);
    uint64 time =
        TTm::GetUnixMSecsFromWinMSecs(Timestamp.GetMSecsFromTm(Timestamp));
    double lat = Rec.GetFieldFltPr(LocationFieldId).Val1;
    double lon = Rec.GetFieldFltPr(LocationFieldId).Val2;

	double accuracy = 0;
	if (!Rec.IsFieldNull(AccuracyFieldId)) {
		accuracy = Rec.GetFieldByte(AccuracyFieldId);
	}
    gps.Time = time;
    gps.LatLon = TPoint(lat, lon);
    gps.Accuracy = accuracy;

    if (StateGpsMeasurements.Len() > 0) {
        TGPSMeasurement* lastRecord = &StateGpsMeasurements.Last();
        //reject this record - it is earlier or same as previous
        if (lastRecord->Time >= gps.Time) {
            return false;
        }
        gps.Distance = TGeoUtils::QuickDist(gps.LatLon, lastRecord->LatLon);
        gps.TimeDiff = gps.Time - lastRecord->Time;
        if (gps.Speed == -1.0) {
            gps.Speed = gps.Distance / gps.TimeDiff;
        }
    }
    else {
        if (gps.Speed == -1.0) { gps.Speed = 0; }
        gps.Distance = 0;
    }
    return true;
}//parseRecord

    /// Helper function for easier GPSRecord creation. It assigns values to new
    /// TGPSRecord, compares it to the previous record and then re-stores it as
    /// lastRecord, for next parsing
TGPSMeasurement* TStayPointDetector::PrepareGPSRecord(const TRec& Rec) {
    TGPSMeasurement gps;
    //fill in the record
    if (!parseRecord(Rec, gps)) {
        return NULL;
    }

    StateGpsMeasurements.Add(gps);
    return &StateGpsMeasurements.Last();//record is on the heap in the array
}//parseRecord


void TStayPointDetector::LoadState(TSIn& SIn) {
    // TODO
}
/// Saves state
void TStayPointDetector::SaveState(TSOut& SOut) const {
    // TODO
}
/// Is the aggregate initialized?
bool TStayPointDetector::IsInit() const {
    // TODO: initialization logic (do we have enough data - does SaveJson 
    //return a sensible object?)
    return true;
}
/// Resets the aggregate
void TStayPointDetector::Reset() {
    // TODO
}

} // TStreamAggrs namespace
} // TQm namespace

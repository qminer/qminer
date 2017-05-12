/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and
* contributors
*
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/
#include <stdio.h>
#include "geospatial_aggr.h"

namespace TQm {
namespace TStreamAggrs {

///////////////////////////////////////////////////////////////////////////////
///TGPSMeasurement
///////////////////////////////////////////////////////////////////////////////
const TInt TGPSMeasurement::NumOfSensorActs = 17; //see shared_consts;

///
/// TGPSMeasurement constructor initializing TGPSMeasurement from Json
///
TGPSMeasurement::TGPSMeasurement(const PJsonVal& Rec) {
    if (!Rec->IsNull()) {
        printf("TIme:\n");
        Time = (uint64)Rec->GetObjKey("time")->GetUInt64();
        printf("latlon:\n");
        double Lat = Rec->GetObjKey("latitude")->GetNum();
        double Lon = Rec->GetObjKey("longitude")->GetNum();
        LatLon = TPoint(Lat, Lon);
        printf("acc:\n");
        Accuracy = Rec->GetObjKey("accuracy")->GetNum();
        printf("distoff:\n");
        Distance = Rec->GetObjKey("distanceDiff")->GetNum();
        printf("speed:\n");
        Speed = Rec->GetObjKey("speed")->GetNum();
        printf("TIme diff:\n");
        TimeDiff = Rec->GetObjKey("timeDiff")->GetInt64();
        printf("Activities:\n");
        if (Rec->IsObjKey("activities")) {
            Rec->GetObjKey("activities")->GetArrIntV(SensorActivities);
        }//if it has activities

        printf("Before arraying:\n");
        int SensLen = SensorActivities.Len();
        if (SensLen < TGPSMeasurement::NumOfSensorActs) {
            int ToAdd = TGPSMeasurement::NumOfSensorActs - SensLen;
            int StartIdx = 0;//starting index of the loop
            if (SensLen == 0) {
                SensorActivities.Add(100);//first one is UNKNOWN
                StartIdx = 1;
            }

            for (int AddIdx = StartIdx; AddIdx < ToAdd; AddIdx++) {
                SensorActivities.Add(0);
            }
        }
    }//if JSon Rec is not null
}//TGPSMeasurement::TGPSMeasurement

///
/// Returns JSON represenation of GPS measurement 
///
PJsonVal TGPSMeasurement::ToJson() const {
    PJsonVal Json = TJsonVal::NewObj();
    Json->AddToObj("time", Time);
    Json->AddToObj("latitude", LatLon.Lat);
    Json->AddToObj("longitude", LatLon.Lon);
    Json->AddToObj("accuracy", Accuracy);
    Json->AddToObj("speed", Speed);
    Json->AddToObj("distanceDiff", Distance);
    Json->AddToObj("timeDiff", (int64)TimeDiff);

    PJsonVal JsonSensActivities = TJsonVal::NewArr();
    //TODO: This is temporal until we have special aggregate - 17 is due to
    //consts in shared_consts.js (NextPin)
    int SenLen = SensorActivities.Len();
    for (int iSens = 0; iSens < SenLen; iSens++) {
       JsonSensActivities->AddToArr((int)SensorActivities[iSens]);
    }
    Json->AddToObj("activities", JsonSensActivities);
    return Json;
}

///////////////////////////////////////////////////////////////////////////////
/// TStaypointCluster
///////////////////////////////////////////////////////////////////////////////

TGeoCluster::TGeoCluster(const int& StartIdx, const int& EndIdx,
    const TVec<TGPSMeasurement>& StateVec) : TGeoCluster(TGeoActivityType::Path)
{
    for (int Idx = StartIdx; Idx <= EndIdx; Idx++) {
        AddPoint(Idx, StateVec);
    }
}

///
/// Constructor creating a TGeoCluster fom JSON
/// PJsonVal& Rec: Json representation of GeoCluster
///
TGeoCluster::TGeoCluster(const PJsonVal& Rec): 
    TGeoCluster(TGeoActivityType::Path) 
{
    printf("Before Status:\n");
    //status
    int Status = Rec->GetObjInt("status", 0);
    if (Status == 0) {
        GeoActStatus = TGeoActivityStatus::Current;
    }
    else if (Status == 1) {
        GeoActStatus = TGeoActivityStatus::Possible;
    }
    else if (Status == 2) {
        GeoActStatus = TGeoActivityStatus::Detected;
    }
    printf("Before Type:\n");
    //type
    TStr Type = Rec->GetObjStr("type", "P");
    if (Type == "P") {
        GeoType = TGeoActivityType::Path;
    }
    else
    {
        GeoType = TGeoActivityType::Staytpoint;
    }
    
    printf("Before latlon:\n");
    double Lat = Rec->GetObjNum("latitude", 0);
    double Lon = Rec->GetObjNum("longitude", 0);
    CenterPoint = TPoint(Lat, Lon);
    printf("Before start:\n");
    Arrive = Rec->GetObjInt64("start_time", 0);
    printf("Before end:\n");
    Depart = Rec->GetObjInt64("end_time", 0);
    printf("Before avg_speed:\n");
    AvgSpeed = Rec->GetObjNum("avg_speed", 0);
    printf("Before avg_acc:\n");
    AvgAccuracy = Rec->GetObjNum("avg_accuracy", 0);
    printf("Before distance:\n");
    Distance = Rec->GetObjNum("distance", 0);
    printf("Before index:\n");
    MStartIdx = Rec->GetObjInt("startIdx", -1);
    MEndIdx = Rec->GetObjInt("endIdx", -1);  
    
    printf("Before activities:\n");
    if (Rec->IsObjKey("activities")) {
        AvgSensorActs.Clr();
        Rec->GetObjKey("activities")->GetArrNumV(AvgSensorActs);
        
        //add zeros at the end
        if (AvgSensorActs.Len() < TGPSMeasurement::NumOfSensorActs) {
            int ToAdd = TGPSMeasurement::NumOfSensorActs -
                AvgSensorActs.Len();
            for (int AddIdx = 0; AddIdx < ToAdd; AddIdx++) {
                AvgSensorActs.Add(0);
            }
        }
    }//if key activities exists
}//TGeoCluster::TGeoCluster

///
/// Indirectly adds one GPS Measurement to the cluster. The measurement needs
/// to be added to the vecotr first, then this method takes the reference to
/// the measurements vector and the appropriate index of the measurement 
///
void TGeoCluster::AddPoint(const int& Idx,
    const TVec<TGPSMeasurement>& _StateVec)
{
    const TGPSMeasurement& CurrentGPS = _StateVec[Idx];
    if (MEndIdx > -1) {//if this cluster already contains some measurements
        const TGPSMeasurement& Last = _StateVec[MEndIdx];
        EAssertR(Last.Time < CurrentGPS.Time,
            "New Point must be newr than the last one");
    }
    else {//if first point
        Arrive = CurrentGPS.Time;
        MStartIdx = Idx;
    }

    Depart = CurrentGPS.Time;
    MEndIdx = Idx;
    int Len = this->Len();
	//distance
	Distance = Distance + CurrentGPS.Distance;
    // incremental averaging (m_n = m_n-1 + ((a_n-m_n-1)/n)
    // m_n = avg value we want to calculate
    // m_n-1 = previous avg value
    // a_n = current value
    // n = number of records (values)
    CenterPoint.Lat = CenterPoint.Lat +
        ((CurrentGPS.LatLon.Lat - CenterPoint.Lat) / Len);
    CenterPoint.Lon = CenterPoint.Lon +
        ((CurrentGPS.LatLon.Lon - CenterPoint.Lon) / Len);
    AvgSpeed = AvgSpeed +
		((CurrentGPS.Distance + 1) * (CurrentGPS.Speed - AvgSpeed) / (Distance + Len));
    AvgAccuracy = AvgAccuracy +
        ((CurrentGPS.Accuracy - AvgAccuracy) / Len);
    
    //avg sensor act
	// incremental average on weighted average -- weights are distances!
	// using Laplace smoothing to avoid division by 0 --- adding 1 and Len to distances
	for (int iSensorAct = 0; iSensorAct < TGPSMeasurement::NumOfSensorActs;
		iSensorAct++)
	{
		AvgSensorActs[iSensorAct] = AvgSensorActs[iSensorAct] +	
			((CurrentGPS.Distance + 1) * (CurrentGPS.SensorActivities[iSensorAct] - 
			AvgSensorActs[iSensorAct]) / (Distance + Len));
	}
}//TGeoCluster::addPoint

/// returns duration in seconds
int64 TGeoCluster::Duration() {
    return (Depart - Arrive) / 1000;
}//TGeoCluster::duration

int TGeoCluster::Len() const {
    if (MEndIdx < 0 || MStartIdx < 0) {
        return 0;
    }
    return MEndIdx - MStartIdx + 1;
}//TGeoCluster::size

double TGeoCluster::QuickDist(const TPoint& P2) {
    return TGeoUtils::QuickDist(CenterPoint, P2);
}

PJsonVal TGeoCluster::ToJson(const TVec<TGPSMeasurement>& _GpsStateVec,
    const bool& FullLoc) const
{
    PJsonVal JGeoAct = TJsonVal::NewObj();
    if (Type() == TGeoActivityType::Path) {
        JGeoAct->AddToObj("type", "P");
    }
    else {
        JGeoAct->AddToObj("type", "S");
    }
    JGeoAct->AddToObj("status", Status());
    JGeoAct->AddToObj("latitude", CenterPoint.Lat);
    JGeoAct->AddToObj("longitude", CenterPoint.Lon);
    JGeoAct->AddToObj("start_time", Arrive);
    JGeoAct->AddToObj("end_time", Depart);
    JGeoAct->AddToObj("duration", (Depart - Arrive) / 1000);
    JGeoAct->AddToObj("locationsNum", this->Len());
    JGeoAct->AddToObj("avg_speed", AvgSpeed);
    JGeoAct->AddToObj("avg_accuracy", AvgAccuracy);
    JGeoAct->AddToObj("distance", Distance);
    JGeoAct->AddToObj("startIdx", MStartIdx);
    JGeoAct->AddToObj("endIdx", MEndIdx);

    //TODO: This is temporal until we have special aggregate - 17 is due to
    //consts in shared_consts.js (NextPin)
    PJsonVal JSenActArr = TJsonVal::NewArr();
    int SenLen = AvgSensorActs.Len();
    for (int iSens = 0; iSens < SenLen; iSens++) {
        JSenActArr->AddToArr(AvgSensorActs[iSens]);
    }
    JGeoAct->AddToObj("activities", JSenActArr);

    if (FullLoc) {
        PJsonVal JLocs = TJsonVal::NewArr();
        if (MStartIdx >= 0 && MEndIdx <= _GpsStateVec.Len()) {
            for (int LocIdx = MStartIdx; LocIdx <= MEndIdx; LocIdx++) {
                const TGPSMeasurement& Gps = _GpsStateVec[LocIdx];
                JLocs->AddToArr(Gps.ToJson());
            }
        }//in case the Cluster is still empty skip that
        JGeoAct->AddToObj("locations", JLocs);
    }
    return JGeoAct;
}


///////////////////////////////////////////////////////////////////////////////
/// GeoUtils
///////////////////////////////////////////////////////////////////////////////
double TGeoUtils::QuickDist(const TPoint& P1, const TPoint& P2) {
    double Lat1 = P1.Lat * TMath::Pi / 180;
    double Lon1 = P1.Lon * TMath::Pi / 180;
    double Lat2 = P2.Lat * TMath::Pi / 180;
    double Lon2 = P2.Lon * TMath::Pi / 180;
    double X = (Lon2 - Lon1) * cos(0.5*Lat2 + Lat1);
    double Y = Lat2 - Lat1;
    return R*sqrt(X*X + Y*Y);
}//QuickDist function


///////////////////////////////////////////////////////////////////////////////
/// StayPoint detector aggregate
///////////////////////////////////////////////////////////////////////////////
TStayPointDetector::TStayPointDetector(
    const TWPt<TBase>& Base,
    const PJsonVal& ParamVal) : TStreamAggr(Base, ParamVal)
{
    if (ParamVal->IsObjKey("params")) {
        Params = ParamVal->GetObjKey("params");
        TrDist = (int)Params->GetObjNum("dT", 50);
        TrTime = (int)Params->GetObjNum("tT", 300);
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
    TStr ActivitiesFieldName = ParamVal->GetObjStr("activitiesField");
    ActivitiesField = Store->GetFieldId(ActivitiesFieldName);
	TStr SpeedFieldName = ParamVal->GetObjStr("speedField");
	SpeedFieldId = Store->GetFieldId(SpeedFieldName);
	
}//TStayPointDetector::constructor

void TStayPointDetector::OnAddRec(const TRec& Rec,
    const TWPt<TStreamAggr>& CallerAggr)
{
    TScopeStopWatch StopWatch(ExeTm);//auto stopwatch for the QM aggregate
                                        //parse new GPS record from JSON
    int NumOfRecordsToForget = 0;
    HasFinishedGeoActs = false;
    //delete previously detected stuff
    int DetectedLen = DetectedGeoActivitiesV.Len();
    for (int GeoActN = 0; GeoActN < DetectedLen; GeoActN++) {
        TGeoCluster& clstr = DetectedGeoActivitiesV[GeoActN];
        NumOfRecordsToForget = clstr.EndIdx();
    }
    if (NumOfRecordsToForget) {
        DetectedGeoActivitiesV.Clr();
        StateGpsMeasurementsV.Del(0, NumOfRecordsToForget);
        //+1 because it deletes 0 IDX as well
        CL.DownShiftIdx(NumOfRecordsToForget + 1);
        Plocs.DownShiftIdx(NumOfRecordsToForget + 1);
    }
    TGPSMeasurement* NewRec = PrepareGPSRecord(Rec);
    if (NewRec == NULL) {//rejected record
		printf("REJECTED RECORD\n");
        return;
    }
    TInt CurrStateIdx = StateGpsMeasurementsV.Len() - 1;
   
    /* Implementation of ETC (named by Chinese paper):
    *  Extracting places from traces of locations:
    *  Kang, J. H., Welbourne, W., Stewart, B., & Borriello, G. (2005).
    *  Extracting places from traces of locations. ACM SIGMOBILE Mobile
    *  Computing and Communications Review, 9(3), 58.
    *  http://doi.org/10.1145/1094549.1094558 */
    if (CL.QuickDist(NewRec->LatLon) < TrDist) {                           //01
        CL.AddPoint(CurrStateIdx, StateGpsMeasurementsV);                  //02
        Plocs = TGeoCluster(TGeoActivityType::Path);                       //03
    }
    else {                                                                 //04
        if (Plocs.Len() > 1) {                                             //05
            if (CL.Duration() > TrTime) {                                  //06
                CL.SetStatus(TGeoActivityStatus::Detected);
                DetectedGeoActivitiesV.Add(CL);                            //07
                HasFinishedGeoActs = true;
            }
            CL = TGeoCluster(TGeoActivityType::Staytpoint);                //08
            CL.AddPoint(Plocs.EndIdx(), StateGpsMeasurementsV);            //09
            Plocs = TGeoCluster(TGeoActivityType::Path);                   //10
            if (CL.QuickDist(NewRec->LatLon) < TrDist) {                   //11
                CL.AddPoint(CurrStateIdx, StateGpsMeasurementsV);          //12
                Plocs = TGeoCluster(TGeoActivityType::Path);               //13??? not needed
            }
            else {                                                         //14
                Plocs.AddPoint(CurrStateIdx, StateGpsMeasurementsV);       //15
            }
        }//if plcocs len >1
        else                                                               //16
        {
            Plocs.AddPoint(CurrStateIdx, StateGpsMeasurementsV);           //17
        }
    }
}//TStayPointDetector::OnAddRec

/// save Json - get current state
PJsonVal TStayPointDetector::SaveJson(const int& Limit) const
{
    PJsonVal State = TJsonVal::NewArr();
    if (StateGpsMeasurementsV.Len() == 0) {
        return State;
    }

    bool GetFullLocs = false;
    if (Limit || HasFinishedGeoActs)
    {
        GetFullLocs = true;
    }
    TGeoCluster Path;
    int LastIdx = StateGpsMeasurementsV.Len() - 1;
    int StartIdx = 0; 
    
    //if there is detected cluster, we know that this cluster and path
    //before (if exists), are of status 2. In this instance there can be only
    //one - array is here for future alogrithms
    if (DetectedGeoActivitiesV.Len() > 0) {
        const TGeoCluster& DetectedStp = DetectedGeoActivitiesV[0];
        if (DetectedStp.StartIdx() > 0) {//if there is a path before this stp
            int EndIdx = DetectedStp.StartIdx() - 1;//We need to add Path first
            Path = TGeoCluster(StartIdx, EndIdx, StateGpsMeasurementsV);
            Path.SetStatus(TGeoActivityStatus::Detected);
            State->AddToArr(Path.ToJson(StateGpsMeasurementsV, 
                GetFullLocs));
        }
        State->AddToArr(DetectedStp.ToJson(StateGpsMeasurementsV, 
                        GetFullLocs));
        StartIdx = DetectedStp.EndIdx() + 1;
    }//if detected staypoint
    
    //plocs can be ignored since these are path as well as the unclustered
    //locations. if CL doesn't have any locations we can simply add all that's
    //left to the path
    if (CL.Len() == 0) {
        Path = TGeoCluster(StartIdx, LastIdx, StateGpsMeasurementsV);
        State->AddToArr(Path.ToJson(StateGpsMeasurementsV, GetFullLocs));
        return State;
    }

    //if start idx of CL is not the same as set up as previous steps, there is 
    // a path before path before
    if (CL.StartIdx() > StartIdx) {
        Path = TGeoCluster(StartIdx, CL.StartIdx() - 1, StateGpsMeasurementsV);
        State->AddToArr(Path.ToJson(StateGpsMeasurementsV, GetFullLocs));
    }
    State->AddToArr(CL.ToJson(StateGpsMeasurementsV, GetFullLocs));
    StartIdx = CL.EndIdx() + 1;

    //there could be a path after CL
    if (StartIdx <= LastIdx) {
        Path = TGeoCluster(StartIdx, LastIdx, StateGpsMeasurementsV);
        State->AddToArr(Path.ToJson(StateGpsMeasurementsV, GetFullLocs));
    }
    return State;
}//SaveJson

///
/// Loads internal state from JSON ojbect
///
void TStayPointDetector::LoadStateJson(const PJsonVal& State){
    if (!State->IsNull() && !State->IsArr()) {
        if (State->IsObjKey("locations")) {
            PJsonVal LocationsArr = State->GetObjKey("locations");
            StateGpsMeasurementsV.Clr();
            int LocLen = LocationsArr->GetArrVals();
            for (int LocIdx = 0; LocIdx < LocLen; LocIdx++) {
                TGPSMeasurement GpsRec =
                    TGPSMeasurement(LocationsArr->GetArrVal(LocIdx));
                StateGpsMeasurementsV.Add(GpsRec);
            }
        }//if locations attribute
        printf("Before CL:\n");
        if (State->IsObjKey("CL")) {
            CL = TGeoCluster(State->GetObjKey("CL"));
        }//if CL attribute
        printf("Before PLocs:\n");
        if (State->IsObjKey("Plocs")) {
            Plocs = TGeoCluster(State->GetObjKey("Plocs"));
        }//if CL attribute
        printf("Before Detected:\n");
        if (State->IsObjKey("Detected")) {
            PJsonVal DetectedArr = State->GetObjKey("Detected");
            for (int DetIdx = 0; DetIdx < DetectedArr->GetArrVals(); DetIdx++){
                TGeoCluster GeoAct = 
                    TGeoCluster(DetectedArr->GetArrVal(DetIdx));
                DetectedGeoActivitiesV.Add(GeoAct);
            }
        }//if it has DetectedGeoActivities
    }//if the root object is real JSON object 
}

/// 
/// Returns the full internal state of the aggregate as JSON object
///
PJsonVal TStayPointDetector::SaveStateJson() const { 
    PJsonVal SateObj = TJsonVal::NewObj(); 
    PJsonVal JLocs = TJsonVal::NewArr();

    //save locations
    int Len = StateGpsMeasurementsV.Len();
    for (int LocIdx = 0; LocIdx < Len; LocIdx++) {
        const TGPSMeasurement& Gps = StateGpsMeasurementsV[LocIdx];
        JLocs->AddToArr(Gps.ToJson());
    }
    SateObj->AddToObj("locations", JLocs);
    SateObj->AddToObj("CL", CL.ToJson(StateGpsMeasurementsV, false));
    SateObj->AddToObj("Plocs", Plocs.ToJson(StateGpsMeasurementsV, false));
    if (DetectedGeoActivitiesV.Len() > 0) {
        PJsonVal JDetectedGeoActs = TJsonVal::NewArr();
        for (int DetIdx = 0; DetIdx < DetectedGeoActivitiesV.Len(); DetIdx++) {
            const TGeoCluster& GAct = DetectedGeoActivitiesV[DetIdx];
            JDetectedGeoActs->
                AddToArr(GAct.ToJson(StateGpsMeasurementsV, true));
        }
        SateObj->AddToObj("Detected", JDetectedGeoActs);
    }
    return SateObj;
}

///
/// Helper function for easier GPSRecord creation. It assigns values to new
/// TGPSRecord, compares it to the previous record and then re-stores it as
/// lastRecord, for next parsing. If parse is not successfull, it returns false
/// otherwise True
///
bool TStayPointDetector::ParseGPSRec(const TRec& Rec, TGPSMeasurement& Gps) {
    TTm Timestamp; 
    Rec.GetFieldTm(TimeFieldId, Timestamp);
    uint64 Time =
        TTm::GetUnixMSecsFromWinMSecs(Timestamp.GetMSecsFromTm(Timestamp));
 
    double Lat = Rec.GetFieldFltPr(LocationFieldId).Val1;
    double Lon = Rec.GetFieldFltPr(LocationFieldId).Val2;

    double Accuracy = 0;
    if (!Rec.IsFieldNull(AccuracyFieldId)) {
        Accuracy = Rec.GetFieldByte(AccuracyFieldId);
    }
	double Speed = -1.0;
	if (!Rec.IsFieldNull(SpeedFieldId)) {
		Speed = Rec.GetFieldFlt(SpeedFieldId);
	}
    Gps.Time = Time;
    Gps.LatLon = TPoint(Lat, Lon);
    Gps.Accuracy = Accuracy;
	Gps.Speed = Speed;
    
    if (!Rec.IsFieldNull(ActivitiesField)) {
        //TODO: This sensorActivities will go into a separate Aggregate
        //once we construct the pipeline - this is a temporary functionality
        //which allows us to continue to work on NextPin on another parts of
        //the project
        Rec.GetFieldIntV(ActivitiesField, Gps.SensorActivities);
    }
    if (Gps.SensorActivities.Len() < TGPSMeasurement::NumOfSensorActs) {
        int ToAdd = TGPSMeasurement::NumOfSensorActs -
            Gps.SensorActivities.Len();
        for (int AddIdx = 0; AddIdx < ToAdd; AddIdx++) {
            Gps.SensorActivities.Add(0);
        }
    }
    
    if (StateGpsMeasurementsV.Len() > 0) {
        TGPSMeasurement* lastRecord = &StateGpsMeasurementsV.Last();
        //reject this record - it is earlier or same as previous
        if (lastRecord->Time >= Gps.Time) {
            return false;
        }
        Gps.Distance = TGeoUtils::QuickDist(Gps.LatLon, lastRecord->LatLon);
        Gps.TimeDiff = (Gps.Time - lastRecord->Time);
        if (Gps.Speed == -1.0) {
            Gps.Speed = Gps.Distance / Gps.TimeDiff;
        }
    }
    else {
		if (Gps.Speed == -1.0) { Gps.Speed = 0; }
        Gps.Distance = 0;
    }
    return true;
}//parseRecord

    /// Helper function for easier GPSRecord creation. It assigns values to new
    /// TGPSRecord, compares it to the previous record and then re-stores it as
    /// lastRecord, for next parsing
TGPSMeasurement* TStayPointDetector::PrepareGPSRecord(const TRec& Rec) {
    TGPSMeasurement gps;
    //fill in the record
    if (!ParseGPSRec(Rec, gps)) {
        return NULL;
    }

    StateGpsMeasurementsV.Add(gps);
    return &StateGpsMeasurementsV.Last();//record is on the heap in the array
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

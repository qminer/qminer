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

/// Returns JSON represenattion of GPS measurement 
PJsonVal TGPSMeasurement::ToJson() const {
    PJsonVal Json = TJsonVal::NewObj();
    Json->AddToObj("time", Time);
    Json->AddToObj("latitude", LatLon.Lat);
    Json->AddToObj("longitude", LatLon.Lon);
    Json->AddToObj("accuracy", Accuracy);
    Json->AddToObj("speed", Speed);
    Json->AddToObj("distanceDiff", Distance);
    Json->AddToObj("timeDiff", TimeDiff);
    return Json;
}


///////////////////////////////
/// TStaypointCluster
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
		((CurrentGPS.Speed - AvgSpeed) / Len);
	AvgAccuracy = AvgAccuracy +
		((CurrentGPS.Accuracy - AvgAccuracy) / Len);
	
	//distance
	Distance = Distance + CurrentGPS.Distance;
}//TGeoCluster::addPoint

/// returns duration in seconds
uint64 TGeoCluster::Duration() {
    return (Depart - Arrive) / 1000;
}//TGeoCluster::duration


int TGeoCluster::Len() const {
	if (MEndIdx < 0 || MStartIdx < 0) {
		return 0;
	}
    return MEndIdx - MStartIdx + 1;
}//TGeoCluster::size

TGeoCluster::TGeoCluster(const int& StartIdx, const int& EndIdx,
    const TVec<TGPSMeasurement>& StateVec)
{
    for (int Idx = StartIdx; Idx <= EndIdx; Idx++) {
        AddPoint(Idx, StateVec);
    }
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
	JGeoAct->AddToObj("latitude", Center().Lat);
	JGeoAct->AddToObj("longitude", Center().Lon);
	JGeoAct->AddToObj("start_time", Arrive);
	JGeoAct->AddToObj("end_time", Depart);
	JGeoAct->AddToObj("duration", (Depart - Arrive) / 1000);
	JGeoAct->AddToObj("locationsNum", this->Len());
	JGeoAct->AddToObj("avgSpeed", AvgSpeed);
	JGeoAct->AddToObj("avgAccuracy", AvgAccuracy);
	JGeoAct->AddToObj("distance", Distance);

	if (FullLoc) {
		PJsonVal JLocs = TJsonVal::NewArr();
		for (int LocIdx = MStartIdx; LocIdx <= MEndIdx; LocIdx++) {
			const TGPSMeasurement& Gps = _GpsStateVec[LocIdx];
			JLocs->AddToArr(Gps.ToJson());
		}
		JGeoAct->AddToObj("locations", JLocs);
	}
	return JGeoAct;
}


///////////////////////////////
/// GeoUtils

double TGeoUtils::QuickDist(const TPoint& P1, const TPoint& P2) {
    double Lat1 = P1.Lat * TMath::Pi / 180;
    double Lon1 = P1.Lon * TMath::Pi / 180;
    double Lat2 = P2.Lat * TMath::Pi / 180;
    double Lon2 = P2.Lon * TMath::Pi / 180;
    double X = (Lon2 - Lon1) * cos(0.5*Lat2 + Lat1);
    double Y = Lat2 - Lat1;
    return R*sqrt(X*X + Y*Y);
}//QuickDist function


///////////////////////////////
/// StayPoint detector aggregate

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

    //init the state in case saveJson is called before onAdd();
    State = TJsonVal::NewArr();
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
        return;
    }

    TInt CurrStateIdx = StateGpsMeasurementsV.Len() - 1;
    
    State = TJsonVal::NewArr();
    

    /* Implementation of ETC (named by Chinese paper):
    *  Extracting places from traces of locations:
    *  Kang, J. H., Welbourne, W., Stewart, B., & Borriello, G. (2005).
    *  Extracting places from traces of locations. ACM SIGMOBILE Mobile
    *  Computing and Communications Review, 9(3), 58.
    *  http://doi.org/10.1145/1094549.1094558 */
    if (TGeoUtils::QuickDist(CL.Center(), NewRec->LatLon) < TrDist) {	   //01
        CL.AddPoint(CurrStateIdx, StateGpsMeasurementsV);			       //02
        Plocs = TGeoCluster(TGeoActivityType::Path);	        	       //03
    }
    else {							    							       //04
        if (Plocs.Len() > 1) {										       //05
            if (CL.Duration() > TrTime) {						           //06
                CL.SetStatus(TGeoActivityStatus::Detected);
                DetectedGeoActivitiesV.Add(CL);						       //07
                HasFinishedGeoActs = true;
            }
            CL = TGeoCluster(TGeoActivityType::Staytpoint);    		       //08
            CL.AddPoint(Plocs.EndIdx(), StateGpsMeasurementsV);	           //09
            Plocs = TGeoCluster(TGeoActivityType::Path);				   //10
            if (TGeoUtils::QuickDist(CL.Center(), NewRec->LatLon) < TrDist) {//11
                CL.AddPoint(CurrStateIdx, StateGpsMeasurementsV);		   //12
                Plocs = TGeoCluster(TGeoActivityType::Path);               //13??? not needed
            }
            else {													       //14
                Plocs.AddPoint(CurrStateIdx, StateGpsMeasurementsV);		   //15
            }
        }//if plcocs len >1
        else															   //16
        {
            Plocs.AddPoint(CurrStateIdx, StateGpsMeasurementsV);			   //17
        }
    }
}//TStayPointDetector::OnAddRec

    /// save Json - get current state
PJsonVal TStayPointDetector::SaveJson(const int& Limit) const
{
    if (State->GetArrVals() > 0) {
        return State;
    }

    TGeoCluster Path;
    int LastIdx = StateGpsMeasurementsV.Len() - 1;
    int StartIdx = 0; int EndIdx = 0;
    //if there is detected cluster, we know that this cluster and path
    //before (if exists), are of status 2
    if (DetectedGeoActivitiesV.Len() > 0) {
        const TGeoCluster& DetectedStp = DetectedGeoActivitiesV[0];
        if (DetectedStp.StartIdx() > 0) {//if there is a path before this stp
            EndIdx = DetectedStp.StartIdx() - 1;//We need to add Path first
            Path = TGeoCluster(StartIdx, EndIdx, StateGpsMeasurementsV);
            Path.SetStatus(TGeoActivityStatus::Detected);
            State->AddToArr(Path.ToJson(StateGpsMeasurementsV, 
                HasFinishedGeoActs));
        }
        State->AddToArr(DetectedStp.ToJson(StateGpsMeasurementsV, 
                HasFinishedGeoActs));
        StartIdx = DetectedStp.EndIdx() + 1;
    }//if detected staypoint
        //if plocs locations are earlier than cl, we simply add them to the path
    if (CL.Len() == 0) {//if CL or CL and PLOCS are empty
        EndIdx = LastIdx;
    }
    //cl will be more also if plocs empty
    else if (Plocs.StartIdx() < CL.StartIdx()) {
        EndIdx = Plocs.EndIdx();
    }
    else {
        EndIdx = CL.StartIdx() - 1;
    }

    //there could be a path between detectedSTP and cl or plocs. This also
    //covers scenario when there is no detectedSTP and all up to here is a path
    if (StartIdx <= EndIdx) {
        Path = TGeoCluster(StartIdx, EndIdx, StateGpsMeasurementsV);
        State->AddToArr(Path.ToJson(StateGpsMeasurementsV, HasFinishedGeoActs));
    }

    if (CL.Len() > 0) {
        State->AddToArr(CL.ToJson(StateGpsMeasurementsV, HasFinishedGeoActs));
        EndIdx = CL.EndIdx() + 1;
    }

    if (EndIdx < LastIdx) {
        Path = TGeoCluster(EndIdx, LastIdx, StateGpsMeasurementsV);
        State->AddToArr(Path.ToJson(StateGpsMeasurementsV, HasFinishedGeoActs));
    }
    return State;
}

/// Helper function for easier GPSRecord creation. It assigns values to new
/// TGPSRecord, compares it to the previous record and then re-stores it as
/// lastRecord, for next parsing. If parse is not successfull, it returns false
/// otherwise True
bool TStayPointDetector::ParseRecord(const TRec& Rec, TGPSMeasurement& Gps) {
    TTm Timestamp; Rec.GetFieldTm(TimeFieldId, Timestamp);
    uint64 Time =
        TTm::GetUnixMSecsFromWinMSecs(Timestamp.GetMSecsFromTm(Timestamp));
    double Lat = Rec.GetFieldFltPr(LocationFieldId).Val1;
    double Lon = Rec.GetFieldFltPr(LocationFieldId).Val2;

	double Accuracy = 0;
	if (!Rec.IsFieldNull(AccuracyFieldId)) {
		Accuracy = Rec.GetFieldByte(AccuracyFieldId);
	}
    Gps.Time = Time;
    Gps.LatLon = TPoint(Lat, Lon);
    Gps.Accuracy = Accuracy;

    if (StateGpsMeasurementsV.Len() > 0) {
        TGPSMeasurement* lastRecord = &StateGpsMeasurementsV.Last();
        //reject this record - it is earlier or same as previous
        if (lastRecord->Time >= Gps.Time) {
            return false;
        }
        Gps.Distance = TGeoUtils::QuickDist(Gps.LatLon, lastRecord->LatLon);
        Gps.TimeDiff = Gps.Time - lastRecord->Time;
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
    if (!ParseRecord(Rec, gps)) {
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

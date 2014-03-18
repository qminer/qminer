/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

namespace TSignalProc {

/////////////////////////////////////////////////
// Exponential Moving Average
typedef enum { etPreviousPoint, etLinear, etNextPoint } TEmaType;
class TEma {
private:
	// parameters
	TFlt Decay; // decaying factor
	TEmaType Type; // interpolation type
	// current state
	TFlt LastVal; // last input value
	TFlt Ema; // current computed EMA value 
	TUInt64 TmMSecs; // timestamp of current EMA
	double TmInterval; // time interval for definition of decay
	// buffer for initialization
	TBool InitP; // true if already initialized
	TUInt64 InitMinMSecs; // time window of requiered values for initialization
	TFltV InitValV; // first N values
	TUInt64V InitMSecsV; // weights of first N values
 	
	double GetNi(const double& Alpha, const double& Mi);
public:
	TEma(const double& _Decay, const TEmaType& _Type, 
        const uint64& _InitMinMSecs, const uint64& _TmInterval);
	TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,
        const double& _TmInterval);
    TEma(const PJsonVal& ParamVal);

	void Update(const double& Val, const uint64& NewTmMSecs);
	// current status
	bool IsInit() const { return InitP; }
	double GetEma() const { return Ema; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

///////////////////////////////
// Sequence buffer
template <class TVal>
class TBuffer {
private:
    // buffer size
	TInt BufferLen;
    // next place in buffer
	TInt NextValN;
    // internal vector used to store circular buffer
	TVec<TVal> ValV;
    
public:
	TBuffer(const int& BufferLen_): BufferLen(BufferLen_), 
        NextValN(0), ValV(BufferLen_, 0) { }
    
    // add new value to the buffer
    void Update(const TVal& Val){
        if (ValV.Len() < BufferLen) {
            ValV.Add(Val);
            NextValN++;
        } else {
            ValV[NextValN] = Val;
            NextValN++;
        }
        NextValN = NextValN % BufferLen;
    }
    
    /// Is buffered initialized 
    bool Init() const { return ValV.Len() == BufferLen; }
    /// Number of elements at the moment
    int Len() const { return ValV.Len(); }
        
	/// Get values from buffer
	const TVal& GetVal(const int& ValN) const {
		return ValV[(NextValN + BufferLen - 1 - ValN) % BufferLen]; }
    /// Get the oldest value
	const TVal& GetVal() const {
		return Init() ? ValV[NextValN] : (ValV.Empty() ? TVal() : ValV[0]); }
};

/////////////////////////////////////////
// Time series interpolator interface
class TInterpolator;
typedef TPt<TInterpolator> PInterpolator;
class TInterpolator {
private:
    TCRef CRef;
    friend class TPt<TInterpolator>;
public:	
    static PInterpolator New(const TStr& InterpolatorType);
    
	virtual ~TInterpolator() { }
	virtual double Interpolate(const uint64& Time) const = 0;    
	virtual void Update(const double& Val, const uint64& Tm) = 0;
};

/////////////////////////////////////////
// Previous point interpolator.
// Interpolate by returning last seen value
class TPreviousPoint : public TInterpolator {
private:
    // previous value, that we return as interpolation
    TFlt PrevVal;
    // current value, waiting to become previous value
    TFlt CurrVal;
    
	TPreviousPoint() { }
public:	
    static PInterpolator New() { return new TPreviousPoint; }
    
	double Interpolate(const uint64& TmMSecs) const { return PrevVal; }
	void Update(const double& Val, const uint64& TmMSecs) { PrevVal = CurrVal; CurrVal = Val; }
};

/////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
	TFlt ForgetFact;
	TFullMatrix P;
	TVector Coeffs;
	int NSamples;

	PNotify Notify;

	TRecLinReg(const TRecLinReg& LinReg): ForgetFact(LinReg.ForgetFact), P(LinReg.P), Coeffs(LinReg.Coeffs), NSamples(LinReg.NSamples), Notify(LinReg.Notify) {}
	TRecLinReg(const TRecLinReg&& LinReg): ForgetFact(std::move(LinReg.ForgetFact)),
		P(std::move(LinReg.P)), Coeffs(std::move(LinReg.Coeffs)), NSamples(std::move(LinReg.NSamples)), Notify(std::move(LinReg.Notify)) {}
public:
	TRecLinReg(const int& Dim, const double _ForgetFact=1, const PNotify _Notify=TStdNotify::New()):
		ForgetFact(_ForgetFact), P(TFullMatrix::Identity(Dim)), Coeffs(Dim), NSamples(0), Notify(_Notify) {}

	TRecLinReg& operator =(TRecLinReg LinReg);

	static PRecLinReg New(const int& Dim, const bool& _AutoBias)
		{ return new TRecLinReg(Dim, _AutoBias); }

	int GetDim() const { return Coeffs.GetDim(); }
	void Learn(const TFltV& Sample, const double& SampleVal);
	double Predict(const TFltV& Sample) const;
};

}

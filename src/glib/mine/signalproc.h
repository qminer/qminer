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
// Online Moving Average (M1))
class TMa {
private:	
    TFlt Ma; // current computed MA value 
	TUInt64 TmMSecs; // timestamp of current MA	    
public:
	TMa() { Ma = 0; };	
    TMa(const PJsonVal& ParamVal) { TMa(); };

	void Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N);	
	double GetMa() const { return Ma; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};
    
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
        const uint64& _InitMinMSecs, const double& _TmInterval);
	TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,
        const double& _TmInterval);
    TEma(const PJsonVal& ParamVal);

	void Update(const double& Val, const uint64& NewTmMSecs);
	// current status
	bool IsInit() const { return InitP; }
	double GetEma() const { return Ema; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Online M2 (variance)
class TVar {
private:	
    TFlt Ma; // current computed MA value 
    TFlt M2; // current computed M2 value 
	TUInt64 TmMSecs; // timestamp of current WMA	
    TFlt pNo;
public:
	TVar() { Ma = 0; M2 = 0; };	
    TVar(const PJsonVal& ParamVal) { TVar(); };

	void Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N);	
	double GetM2() const { return M2/pNo; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Online M2(X,Y) (covariance)
class TCov {
private:	
    TFlt MaX, MaY; // current computed MA value        
    TFlt Cov;
	TUInt64 TmMSecs; // timestamp of current WMA	
    TFlt pNo;
public:
	TCov() {  };	
    TCov(const PJsonVal& ParamVal) { TCov(); };

	void Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV, const int& N);	
	double GetCov() const { return Cov/pNo; }
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
    
    /// add new value to the buffer
    void Update(const TVal& Val){
        // we add to the end while not full
        if (ValV.Len() < BufferLen) { ValV.Add(Val); }
        // after we overwrite existing
        else { ValV[NextValN] = Val; }
        // we always point to next empty(-to-be) place
        NextValN++;
        // reset count once we come to BufferLen
        NextValN = NextValN % BufferLen;
    }
    
    /// Is buffered initialized 
    bool IsInit() const { return ValV.Len() == BufferLen; }
    /// Is buffer empty
    bool Empty() const { return ValV.Empty(); }
    /// Number of elements at the moment
    int Len() const { return ValV.Len(); }
        
	/// Get values from buffer (older values = larger index): example buffer = [x_(t-3) x_(t-2) x_(t-1) x_t] => GetVal(i) returns x_(t-1)
	const TVal& GetVal(const int& ValN) const {
		return ValV[(NextValN + BufferLen - 1 - ValN) % BufferLen]; }
    /// Get the oldest value
	const TVal& GetOldest() const {
        Assert(!Empty()); return IsInit() ? GetVal(BufferLen - 1) : ValV[0]; }
	/// Get the most recent value
	const TVal& GetNewest() const {
        Assert(!Empty()); return IsInit() ?  GetVal(0) : ValV.Last(); }
};

/////////////////////////////////////////
// Time series interpolator interface
class TInterpolator;
typedef TPt<TInterpolator> PInterpolator;
class TInterpolator {
private:
    TCRef CRef;
    friend class TPt<TInterpolator>;
private:
    const TStr InterpolatorType;
protected:
    TInterpolator(const TStr& _InterpolatorType): InterpolatorType(_InterpolatorType) {}
public:
    static PInterpolator New(const TStr& InterpolatorType);
    static PInterpolator Load(TSIn& SIn);

 	virtual ~TInterpolator() { }

	virtual void Save(TSOut& SOut) const { InterpolatorType.Save(SOut); }

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
    
	TPreviousPoint(): TInterpolator(TPreviousPoint::GetType()) { }
	TPreviousPoint(TSIn& SIn): TInterpolator(TPreviousPoint::GetType()), PrevVal(SIn), CurrVal(SIn) {}
public:	
    static PInterpolator New() { return new TPreviousPoint; }
    static PInterpolator New(TSIn& SIn) { return new TPreviousPoint(SIn); }
	void Save(TSOut& SOut) const { TInterpolator::Save(SOut); PrevVal.Save(SOut); CurrVal.Save(SOut); };
    
	double Interpolate(const uint64& TmMSecs) const { return PrevVal; }
	void Update(const double& Val, const uint64& TmMSecs) { PrevVal = CurrVal; CurrVal = Val; }

	static TStr GetType() { return "previous"; }
};

/////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
	TFlt ForgetFact;	// forgetting factor
	TFlt RegFact;		// regularization

	TFullMatrix P;		// correlation matrix
	TVector Coeffs;		// model

	// copy constructor
	TRecLinReg(const TRecLinReg& LinReg);
	// move constructor
	TRecLinReg(const TRecLinReg&& LinReg);
	// load constructor
	TRecLinReg(TSIn& SIn);
public:
	// default constructor
	TRecLinReg(const int& Dim, const double& _RegFact = 1.0, const double& _ForgetFact=1.0);
	// new method
	static PRecLinReg New(const int& Dim, const double& RegFact, const double& ForgetFact)
			{ return new TRecLinReg(Dim, RegFact, ForgetFact); }

	virtual ~TRecLinReg() {}

	void Save(TSOut& SOut) const;
	static PRecLinReg Load(TSIn& SIn);

	// assign operator
	TRecLinReg& operator =(TRecLinReg LinReg);

	// returns the dimension of the instances
	int GetDim() const { return Coeffs.GetDim(); }
	// learns a new sample
	void Learn(const TFltV& Sample, const double& SampleVal);
	// predicts a value
	double Predict(const TFltV& Sample);
	// copies the current models coefficients into the specified list
	void GetCoeffs(TFltV& Coef) const;
};

}

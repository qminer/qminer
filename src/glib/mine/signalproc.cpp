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
// Online Moving Average 
void TMa::Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N){
    
    int tempN = N - 1 + OutValV.Len();
    double delta;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValV.Len(); ValN++)
    {        
        tempN--;       
        delta = OutValV[ValN] - Ma;
        Ma = Ma - delta/tempN;                 
    }
    //add the new value to the resulting mean    
    delta = InVal - Ma;
    Ma = Ma + delta/N;
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////////////
// Exponential Moving Average
double TEma::GetNi(const double& Alpha, const double& Mi) {
	switch (Type) {
	case etPreviousPoint: return 1.0;
	case etLinear: return (1 - Mi) / Alpha;
	case etNextPoint: return Mi;
	}
	throw TExcept::New("Unknown EMA interpolation type");
}

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const double& _Decay, const TEmaType& _Type, const uint64& _InitMinMSecs, 
    const uint64& _TmInterval): Decay(_Decay), Type(_Type), TmInterval((double)_TmInterval), 
    InitP(false), InitMinMSecs(_InitMinMSecs) { }

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,const double& _TmInterval):
	Type(_Type), TmInterval(_TmInterval), InitP(false), InitMinMSecs(_InitMinMSecs) { }

TEma::TEma(const PJsonVal& ParamVal): InitP(false) {
    // type
    TStr TypeStr = ParamVal->GetObjStr("emaType");
    if (TypeStr == "previous") { 
        Type = etPreviousPoint;
    } else if (TypeStr == "linear") {
        Type = etLinear;
    } else if (TypeStr == "next") {
        Type = etNextPoint;
    } else {
        throw TExcept::New("Unknown ema type " + TypeStr);
    }
    // rest
    TmInterval = ParamVal->GetObjNum("interval");
    InitMinMSecs = ParamVal->GetObjInt("initWindow");
}

void TEma::Update(const double& Val, const uint64& NewTmMSecs) {
	uint64 TmInterval1;
	if(NewTmMSecs == TmMSecs) {
		TmInterval1 = 1;
    } else{
        TmInterval1 = NewTmMSecs - TmMSecs;
    }
	if (InitP) {
		// computer parameters for EMA
		double Alpha;
		if (Decay == 0.0) {			
			Alpha = (double)(TmInterval1) / TmInterval;
		} else {			
			Alpha = (double)((TmInterval1)) /((double)(TmInterval))  * (-1) * TMath::Log(Decay);
		}
		const double Mi = exp(-Alpha);
		const double Ni = GetNi(Alpha, Mi);
		// compute new ema
		Ema = Mi*Ema + (Ni - Mi)*LastVal + (1.0 - Ni)*Val;
	} else {
		// update buffers
		InitValV.Add(Val);
		InitMSecsV.Add(NewTmMSecs);
		// initialize when enough data
		const uint64 StartInitMSecs = InitMSecsV[0] + InitMinMSecs;
		if (StartInitMSecs < NewTmMSecs) {
			// Initialize using "buildup time interval",
			//TODO: check how interpolation type influences this code
			const int Vals = InitMSecsV.Len();	
			// compute weights for each value in buffer
			TFltV WeightV(Vals, 0);
			for (int ValN = 0; ValN < Vals; ValN++) {
				const double Alpha = (double)(TmInterval1) / Decay;
				WeightV.Add(exp(-Alpha));
			}
			// normalize weights so they sum to 1.0
			TLinAlg::NormalizeL1(WeightV);
			// compute initial value of EMA as weighted sum
			Ema = TLinAlg::DotProduct(WeightV, InitValV);
			// mark that we are done and clean up after us
			InitP = true; InitValV.Clr(); InitMSecsV.Clr();
		}
	}
	// update last value
	LastVal = Val;
	// update curret time
	TmMSecs = NewTmMSecs;
}

/////////////////////////////////////////////////
// Online Moving Standard M2 
void TVar::Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N){
    
    pNo = N;
    int tempN = N - 1 + OutValV.Len();
    double delta;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValV.Len(); ValN++)
    {        
        tempN--;       
        delta = OutValV[ValN] - Ma;
        Ma = Ma - delta/tempN;  
        M2 = M2 - delta * (OutValV[ValN] - Ma);
    }
    //add the new value to the resulting mean    
    delta = InVal - Ma;
    Ma = Ma + delta/N;
    M2 = M2 + delta * (InVal - Ma);
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////////////
// Online Moving Covariance (assumes X and Y have the same time stamp)
void TCov::Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV, const int& N){
    
    pNo = N;
    int tempN = N - 1 + OutValVX.Len();
    double deltaX, deltaY;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValVX.Len(); ValN++)
    {        
        tempN--;       
        deltaX = OutValVX[ValN] - MaX;
        deltaY = OutValVY[ValN] - MaY;
        MaX = MaX - deltaX/tempN;  
        MaY = MaY - deltaY/tempN;  
        Cov = Cov - (OutValVX[ValN] - MaX) * (OutValVY[ValN] - MaY);
    }
    //add the new value to the resulting mean    
    deltaX = InValX - MaX;
    deltaY = InValY - MaY;
    MaX = MaX + deltaX/N;
    MaY = MaY + deltaY/N;
    Cov = Cov + (InValX - MaX) * (InValY - MaY);
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////
// Time series interpolator interface
PInterpolator TInterpolator::New(const TStr& InterpolatorType) {
    if(InterpolatorType == TPreviousPoint::GetType()) {
		return TPreviousPoint::New();
	}    
    throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

PInterpolator TInterpolator::Load(TSIn& SIn) {
	TStr InterpolatorType(SIn);
	if(InterpolatorType == TPreviousPoint::GetType()) {
		return TPreviousPoint::New(SIn);
	}
	throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

///////////////////////////////////////////////////////////////////
// Recursive Linear Regression
TRecLinReg::TRecLinReg(const TRecLinReg& LinReg):
		ForgetFact(LinReg.ForgetFact),
		RegFact(LinReg.RegFact),
		P(LinReg.P),
		Coeffs(LinReg.Coeffs) {}

TRecLinReg::TRecLinReg(const TRecLinReg&& LinReg):
		ForgetFact(std::move(LinReg.ForgetFact)),
		RegFact(std::move(LinReg.RegFact)),
		P(std::move(LinReg.P)),
		Coeffs(std::move(LinReg.Coeffs)) {}

TRecLinReg::TRecLinReg(TSIn& SIn):
		ForgetFact(SIn),
		RegFact(SIn) {
	P.Load(SIn);
	Coeffs.Load(SIn);
}

TRecLinReg::TRecLinReg(const int& Dim, const double& _RegFact,
			const double& _ForgetFact):
		ForgetFact(_ForgetFact),
		RegFact(_RegFact),
		P(TFullMatrix::Identity(Dim) / RegFact),
		Coeffs(Dim, true) {}

void TRecLinReg::Save(TSOut& SOut) const {
	ForgetFact.Save(SOut);
	RegFact.Save(SOut);
	P.Save(SOut);
	Coeffs.Save(SOut);
}

PRecLinReg TRecLinReg::Load(TSIn& SIn) {
	return new TRecLinReg(SIn);
}

TRecLinReg& TRecLinReg::operator =(TRecLinReg LinReg) {
	std::swap(ForgetFact, LinReg.ForgetFact);
	std::swap(P, LinReg.P);
	std::swap(Coeffs, LinReg.Coeffs);

	return *this;
}

double TRecLinReg::Predict(const TFltV& Sample) {
    return Coeffs.DotProduct(Sample);
}

void TRecLinReg::Learn(const TFltV& Sample, const double& SampleVal) {
    double PredVal = Predict(Sample);

    TVector x(Sample);

    TVector Px = P * x;
	double xPx = Px.DotProduct(Sample);

	/*
	 * linreg.P = (linreg.P - (Px * Px') / (linreg.lambda + xPx)) / linreg.lambda;
	 * linreg.w = linreg.w + Px*((y - y_hat)/(linreg.lambda + xPx));
	 */
	P = (P - (Px*Px.GetT()) / (ForgetFact + xPx)) / ForgetFact;
	Coeffs += Px*((SampleVal - PredVal) / (ForgetFact + xPx));
}

void TRecLinReg::GetCoeffs(TFltV& Coef) const {
	Coef = Coeffs.GetVec();
}

}

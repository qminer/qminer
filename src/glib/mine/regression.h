/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TRegression {

///////////////////////////////////////////
// Proportional Hazards model
class TPropHazards {
private:
    typedef TVec<TIntFltKdV> TSpVV;

	double Lambda;
	TFltV WgtV;

	bool Verbose;
	PNotify Notify;

public:
	TPropHazards(const double& Lambda=0, const bool Verbose=false);
	TPropHazards(TSIn& SIn);

	void Save(TSOut& SOut) const;

	void Fit(const TFltVV& X, const TFltV& t, const double& Eps=1e-6);
	double Predict(const TFltV& x) const;

	void GetWgtV(TFltV& WgtV) const;

	double GetLambda() const { return Lambda; }
	void SetLambda(const double& _Lambda) { Lambda = _Lambda; }

private:
	void PredictInternal(const TFltVV& X, TFltV& IntensV) const;
};
    
/////////////////////////////////////////////
// Ridge Regression
class TRidgeReg {
private:
    TFlt Gamma;
    TFltV WgtV;
    
public:
    TRidgeReg(const double& _Gamma): Gamma(_Gamma) { }
    TRidgeReg(TSIn& SIn) : Gamma(SIn), WgtV(SIn) { }
    
    void Save(TSOut& SOut) const { Gamma.Save(SOut); WgtV.Save(SOut); }
    
    void Fit(const TFltVV& X, const TFltV& y);
    double Predict(const TFltV& x) const;
    
    const TFltV& GetWgtV() const { return WgtV; }
    double GetGamma() const { return Gamma; }
    void SetGamma(const double& _Gamma) { Gamma = _Gamma; }
};


}

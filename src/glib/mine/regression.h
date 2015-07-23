/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TRegression {

///////////////////////////////////////////
// Logistic Regression using the Newton-Raphson method
class TLogReg {
private:
	double Lambda;
	TFltV WgtV;

	bool IncludeIntercept;

	bool Verbose;
	PNotify Notify;

public:
	// default constructor, sets the regularization parameter
	TLogReg(const double& Lambda=1, const bool IncludeIntercept=false, const bool Verbose=true);
	TLogReg(TSIn& SIn);

	void Save(TSOut& SOut) const;

	// Fits the regression model. The method assumes that the instances are stored in the
	// columns of the matrix X and the responses are stored in vector y.
	void Fit(const TFltVV& X, const TFltV& y, const double& Eps=1e-3);
	// returns the expected response for the given feature vector
	double Predict(const TFltV& x) const;

	void GetWgtV(TFltV& WgtV) const;
private:
	double PredictWithoutIntercept(const TFltV& x) const;
};

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

private:
	void PredictInternal(const TFltVV& X, TFltV& IntensV) const;
};


}

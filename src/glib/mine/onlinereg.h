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
 
#ifndef LINREG_H_
#define LINREG_H_

/////////////////////////////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
	TFlt ForgetFact;
	TFullMatrix P;
	TVector Coeffs;
	int NSamples;

	PNotify Notify;

public:
	TRecLinReg(const int& Dim, const double _ForgetFact=1, const PNotify _Notify=TStdNotify::New()):
		ForgetFact(_ForgetFact), P(TFullMatrix::Identity(Dim)), Coeffs(Dim), NSamples(0), Notify(_Notify) {}

	TRecLinReg(const TRecLinReg& LinReg): ForgetFact(LinReg.ForgetFact), P(LinReg.P), Coeffs(LinReg.Coeffs), NSamples(LinReg.NSamples), Notify(LinReg.Notify) {}
	TRecLinReg(const TRecLinReg&& LinReg): ForgetFact(std::move(LinReg.ForgetFact)),
		P(std::move(LinReg.P)), Coeffs(std::move(LinReg.Coeffs)), NSamples(std::move(LinReg.NSamples)), Notify(std::move(LinReg.Notify)) {}

	TRecLinReg& operator =(TRecLinReg LinReg);

	static PRecLinReg New(const int& Dim, const bool& _AutoBias)
		{ return new TRecLinReg(Dim, _AutoBias); }

public:
	void Learn(const TFltV& Sample, const double& SampleVal);
	double Predict(const TFltV& Sample) const;
};


#endif /* LINREG_H_ */

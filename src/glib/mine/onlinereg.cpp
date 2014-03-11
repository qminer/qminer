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

///////////////////////////////////////////////////////////////////
// Recursive Linear Regression
TRecLinReg& TRecLinReg::operator =(TRecLinReg LinReg) {
	std::swap(ForgetFact, LinReg.ForgetFact);
	std::swap(P, LinReg.P);
	std::swap(Coeffs, LinReg.Coeffs);

	return *this;
}

double TRecLinReg::Predict(const TFltV& Sample) const {
	return Coeffs.DotProduct(Sample);
}

void TRecLinReg::Learn(const TFltV& Sample, const double& SampleVal) {
	double PredVal = Predict(Sample);
	TVector x(Sample);

	TVector Px = P * x;
	double xPx = Px.DotProduct(Sample);

	P = (P - (Px * Px.GetT()) / (ForgetFact + xPx)) / ForgetFact;
	Coeffs += Px*(SampleVal - PredVal);

	if (++NSamples % 1000 == 0) {
		Notify->OnNotifyFmt(TNotifyType::ntInfo, "TRecLinReg: %d samples\n", NSamples);
	}
}

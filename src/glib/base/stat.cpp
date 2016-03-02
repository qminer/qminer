#include "stat.h"

void TStatFun::ChiSquare(const TFltV& OutValVX, const TFltV& OutValVY, const TInt& Df,
		TFlt& Chi2, TFlt& P) {
	Chi2 = 0.0;
	P = 1.0;
	EAssertR(OutValVX.Len() == OutValVY.Len(), "TChiSquare: histogram dimensions do not match!");
	// http://www.itl.nist.gov/div898/software/dataplot/refman1/auxillar/chi2samp.htm
	double SumR = TLinAlg::SumVec(OutValVX);
	double SumS = TLinAlg::SumVec(OutValVY);
	// Do nothing if zero histogram is detected
	if (SumR <= 0.0 || SumS <= 0.0) { return; }
	double K1 = TMath::Sqrt(SumS / SumR);
	double K2 = 1.0 / K1;
	for (int ValN = 0; ValN < OutValVX.Len(); ValN++) {
		double Ri = OutValVX[ValN];
		double Si = OutValVY[ValN];
		double RpS = Ri + Si;
		if (RpS > 0) {
			Chi2 += TMath::Sqr(K1 * Ri - K2 * Si) / RpS;
		}
	}
	if (Chi2 != 0.0) {
		P = TSpecFunc::GammaQ(0.5*(Df), 0.5*(Chi2));
	}
}

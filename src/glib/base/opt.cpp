#include "opt.h"

using namespace TOpt;

void TOptUtil::FindFeasible(const TFltVV& A, const TFltV& b, TFltV& x, const PNotify Notify) {
	Notify->OnNotify(TNotifyType::ntInfo, "Finding feasible solution ...");

	// initialize variables
	const int Dim = A.GetRows();
	const int NConstr = A.GetCols();

	if (x.Empty()) {
		x.Gen(Dim);
	}

	double NormalNorm;
	double p;
	double Dist, Factor;

	TFltV FactV(NConstr);
	TFltV Normal(Dim);

	// prepare structures
	for (int i = 0; i < NConstr; i++) {
		FactV[i] = 1;
	}

	int k = 0;
	int UnsatisfiedNum = TInt::Mx;
	while (UnsatisfiedNum > 0) {
		UnsatisfiedNum = 0;
		// go through all the constraints
		for (int ConstrN = 0; ConstrN < NConstr; ConstrN++) {
			// check if the constraint is violated
			if (TLinAlg::DotProduct(A, ConstrN, x) > b[ConstrN]) {
				// translate the solution over the unsatisfied constraint
				// we can put the hyperplane in Hessian normal form:
				// n*x = -p
				// where p = d / |n| and n is the normalized normal
				A.GetCol(ConstrN, Normal);
				NormalNorm = TLinAlg::Normalize(Normal);

				p = -b[ConstrN] / NormalNorm;
				// the distance from x_0 to the hyperplane is: n*x_0 + p
				Dist = TLinAlg::DotProduct(Normal, x) + p;

				// translate x
				Factor = (1 + FactV[ConstrN]) * Dist;
				for (int i = 0; i < Dim; i++) {
					x[i] -= Factor * Normal[i];
				}

				// reduce the translation factor for this constraint
				FactV[ConstrN] = TMath::Mx(FactV[ConstrN] * .5, 1e-14);
				UnsatisfiedNum++;
			}
		}

		if (k++ % 100 == 0) {
			Notify->OnNotifyFmt(TNotifyType::ntInfo, "Step %d modified %d constraints ...", k, UnsatisfiedNum);
		}
	}

	Notify->OnNotify(TNotifyType::ntInfo, "Feasible solution found!");
}

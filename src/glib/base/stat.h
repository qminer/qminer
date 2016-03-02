#ifndef STAT_H
#define STAT_H

class TStatFun {
public:
	static void ChiSquare(const TFltV& OutValVX, const TFltV& OutValVY, const TInt& Df,
			TFlt& Chi2, TFlt& P);
};

#endif

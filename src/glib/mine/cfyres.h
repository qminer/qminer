/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

//////////////////////////////////////////////////////////////////////////
// Classification Results
class TCountCfyRes;
ClassTV(TCfyRes, TCfyResV)//{
private:
	PMom AccMom, PrecMom, RecMom, F1Mom, BreakEvenPointMom, AvgPrecMom;
	TMomV DecPrecMomV, DecRecMomV;
	TFltPrV LastRocV;
	TStrFltKdV StatNmWgtV; // extra statistics
public:
	TCfyRes();
	TCfyRes(TCountCfyRes& CfyRes);

	// for averaging result
	void Add(TCfyRes& CfyRes);
	void Def();

	// Precision
	double Prec() const { return PrecMom->GetMean(); }
	double PrecStDiv() const { return PrecMom->GetSDev(); }
	// Recall
	double Rec() const { return RecMom->GetMean(); }
	double RecStDiv() const { return RecMom->GetSDev(); }
	// F1
	double F1() const { return F1Mom->GetMean(); }
	double F1StDiv() const { return F1Mom->GetSDev(); }

	// Accuracy
	double Acc() const { return AccMom->GetMean(); }
	double AccStDiv() const { return AccMom->GetSDev(); }
	// Average Precision
	double AvgPrec() const { return AvgPrecMom->GetMean(); }
	double AvgPrecStDiv() const { return AvgPrecMom->GetSDev(); }
	// Break Even Point
	double BreakEvenPoint() const { return BreakEvenPointMom->GetMean(); }
	double BreakEvenPointStDiv() const { return BreakEvenPointMom->GetSDev(); }

	// precision / recall at different levels
	int GetDecFolds() const { return DecPrecMomV.Len(); }
	double PrecDec(const int& FoldN) const { return DecPrecMomV[FoldN]->GetMean(); }
	double PrecDecStDiv(const int& FoldN) const { return DecPrecMomV[FoldN]->GetSDev(); }
	double RecDec(const int& FoldN) const { return DecRecMomV[FoldN]->GetMean(); }
	double RecDecStDiv(const int& FoldN) const { return DecRecMomV[FoldN]->GetSDev(); }
	// ROC curve
	const TFltPrV& RocCurve() const { return LastRocV; }

	// extra statistics
	int GetStats() const { return StatNmWgtV.Len(); }
	TStr GetStatNm(const int& StatN) const { return StatNmWgtV[StatN].Key; }
	double GetStatWgt(const int& StatN) const { return StatNmWgtV[StatN].Dat; }
	void AddStat(const TStr& StatNm, const double& StatWgt) {
		StatNmWgtV.Add(TStrFltKd(StatNm, StatWgt)); }

	// String with classification statistics
	TStr GetStatStr(const TStr& Desc);
	// Print statistics
	void PrStat(const TStr& Desc) { printf("%s\n", GetStatStr(Desc).CStr()); }
	// Write statistics to stream
	void PrStat(const TStr& Desc, PSOut SOut) { SOut->PutStrLn(GetStatStr(Desc)); }

	// print machine readable statistics to file
	void PrTabStat(const TStr& StatFNm, const TStr& Nm = "");
	// print machine readable header
	void PrTabHeader(PSOut SOut);
	// print machine readable data
	void PrTabVals(PSOut SOut, const TStr& Nm = "");
};

//////////////////////////////////////////////////////////////////////////
// Count -- Classification Results
class TCountCfyRes {
private:
	TFltKdV ResultV; // vector with predicted and true class for each sample
	bool ResultVSorted;
	int TPos, TNeg, FPos, FNeg;

	void PrepareResultV() { if (!ResultVSorted) ResultV.Sort(false); ResultVSorted = true; }
	void GetDec(const TFltV& ResV, TFltV& DecResV);
	void GetDec(const TFltPrV& ResV, TFltPrV& DecResV);
public:
	TCountCfyRes(): ResultV(), ResultVSorted(false), TPos(0), TNeg(0), FPos(0), FNeg(0) { }
	TCfyRes ToTCfyRes() { PrepareResultV(); return TCfyRes(*this); }
	static TCfyRes ToMacroTCfyRes(TVec<TCountCfyRes>& CfyResV);

	void Add(const double& PredCls, const double& TrueCls);

	double Acc() const { if (TPos+TNeg>0) return (TPos+TNeg)/double(TPos+TNeg+FPos+FNeg); else return 0.0; }
	double Prec() const { if (TPos+FPos>0) return TPos/double(TPos+FPos); else return 0.0; }
	double Rec() const { if (TPos+FNeg>0) return TPos/double(TPos+FNeg); else return 0.0; }
	double F1() const { if (Prec()+Rec()>0.0) return  2.0*Prec()*Rec()/(Prec()+Rec()); else return 0.0; }
	double BreakEvenPoint();
	double AvgPrec();

	void PrecRec(TFltV& PrecV, TFltV& RecV);
	void DecPrecRec(TFltV& DecPrecV, TFltV& DecRecV);
	void Roc(TFltPrV& RocV);
	void DecRoc(TFltPrV& RocV);
};

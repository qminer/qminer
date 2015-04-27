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
TCfyRes::TCfyRes() {
    AccMom = TMom::New(); PrecMom = TMom::New(); 
    RecMom = TMom::New(); F1Mom = TMom::New(); 
    BreakEvenPointMom = TMom::New(); AvgPrecMom = TMom::New();
	const int Folds = 11; DecPrecMomV.Gen(Folds); DecRecMomV.Gen(Folds);
	for (int FoldN = 0; FoldN < Folds; FoldN++) {
		DecPrecMomV[FoldN] = TMom::New();
		DecRecMomV[FoldN] = TMom::New();
	}
}

TCfyRes::TCfyRes(TCountCfyRes& CfyRes) {
    PrecMom = TMom::New(); PrecMom->Add(CfyRes.Prec()); PrecMom->Def();
    RecMom = TMom::New(); RecMom->Add(CfyRes.Rec()); RecMom->Def();
    F1Mom = TMom::New(); F1Mom->Add(CfyRes.F1()); F1Mom->Def();

    AccMom = TMom::New(); AccMom->Add(CfyRes.Acc()); AccMom->Def();
    AvgPrecMom = TMom::New(); AvgPrecMom->Add(CfyRes.AvgPrec());  AvgPrecMom->Def();

    BreakEvenPointMom = TMom::New();
    BreakEvenPointMom->Add(CfyRes.BreakEvenPoint());
    BreakEvenPointMom->Def();

	const int Folds = 11; 
	DecPrecMomV.Gen(Folds); DecRecMomV.Gen(Folds);
	TFltV DecPrecV, DecRecV; 
	CfyRes.DecPrecRec(DecPrecV, DecRecV);
	for (int FoldN = 0; FoldN < Folds; FoldN++) {
		DecPrecMomV[FoldN] = TMom::New();
		DecPrecMomV[FoldN]->Add(DecPrecV[FoldN]);
		DecPrecMomV[FoldN]->Def();
		DecRecMomV[FoldN] = TMom::New();
		DecRecMomV[FoldN]->Add(DecRecV[FoldN]);
		DecRecMomV[FoldN]->Def();
	}

	CfyRes.DecRoc(LastRocV);
}; 

void TCfyRes::Add(TCfyRes& CfyRes) {
    AccMom->Add(CfyRes.Acc()); PrecMom->Add(CfyRes.Prec());
    RecMom->Add(CfyRes.Rec()); F1Mom->Add(CfyRes.F1());
    BreakEvenPointMom->Add(CfyRes.BreakEvenPoint());
    AvgPrecMom->Add(CfyRes.AvgPrec());
	const int Folds = DecPrecMomV.Len(); 
	for (int FoldN = 0; FoldN < Folds; FoldN++) {
		DecPrecMomV[FoldN]->Add(CfyRes.PrecDec(FoldN));
		DecRecMomV[FoldN]->Add(CfyRes.RecDec(FoldN));
	}
	LastRocV = CfyRes.RocCurve();
}

void TCfyRes::Def() {
    AccMom->Def(); PrecMom->Def(); RecMom->Def();
    F1Mom->Def(); BreakEvenPointMom->Def(); AvgPrecMom->Def();
	const int Folds = DecPrecMomV.Len(); 
	for (int FoldN = 0; FoldN < Folds; FoldN++) {
		DecPrecMomV[FoldN]->Def();
		DecRecMomV[FoldN]->Def();
	}
}

TStr TCfyRes::GetStatStr(const TStr& Desc) {
    TStr StatStr = Desc + ":\n";
    StatStr += TStr::Fmt("prec: [%5.2f%%,%5.2f%%] rec: [%5.2f%%,%5.2f%%] F1:[%5.2f%%,%5.2f%%]\n",
        100.0*Prec(), 100.0*PrecStDiv(), 100.0*Rec(), 100.0*RecStDiv(), 100.0*F1(), 100.0*F1StDiv());
    StatStr += TStr::Fmt("acc: [%5.2f%%,%5.2f%%] avgprec: [%5.2f%%,%5.2f%%] BEP:[%5.2f%%,%5.2f%%]\n",
        100.0*Acc(), 100.0*AccStDiv(), 100.0*AvgPrec(), 100.0*AvgPrecStDiv(), 100.0*BreakEvenPoint(), 100.0*BreakEvenPointStDiv());
	//const int Folds = DecPrecMomV.Len(); 
	//StatStr += "Prec:";
	//for (int FoldN = 0; FoldN < Folds; FoldN++) {
	//	StatStr += (FoldN % 4 == 0) ? "\n" : " ";
	//	StatStr += TStr::Fmt("[%5.2f%%,%5.2f%%]", 100.0*PrecDec(FoldN), 100.0*PrecDecStDiv(FoldN));
	//} StatStr += "\n";
	//StatStr += "Rec:";
	//for (int FoldN = 0; FoldN < Folds; FoldN++) {
	//	StatStr += (FoldN % 4 == 0) ? "\n" : " ";
	//	StatStr += TStr::Fmt("[%5.2f%%,%5.2f%%]", 100.0*RecDec(FoldN), 100.0*RecDecStDiv(FoldN));
	//}
    return StatStr;
}

void TCfyRes::PrTabStat(const TStr& StatFNm, const TStr& Nm) {
    PSOut SOut = TFOut::New(StatFNm);
    PrTabHeader(SOut);
    PrTabVals(SOut, Nm);
}

void TCfyRes::PrTabHeader(PSOut SOut) {
    SOut->PutStr("Nm\tPrec\t\tRec\t\tF1\t\tAcc\t\tBEP\t");
    for (int StatN = 0; StatN < GetStats(); StatN++) {
        SOut->PutStr("\t" + GetStatNm(StatN)); }
    SOut->PutLn();
}

void TCfyRes::PrTabVals(PSOut SOut, const TStr& Nm) {
    SOut->PutStr(Nm + "\t");
    SOut->PutFlt(Prec()); SOut->PutCh('\t'); SOut->PutFlt(PrecStDiv()); SOut->PutCh('\t');
    SOut->PutFlt(Rec()); SOut->PutCh('\t'); SOut->PutFlt(RecStDiv()); SOut->PutCh('\t');
    SOut->PutFlt(F1()); SOut->PutCh('\t'); SOut->PutFlt(F1StDiv()); SOut->PutCh('\t');
    SOut->PutFlt(Acc()); SOut->PutCh('\t'); SOut->PutFlt(AccStDiv()); SOut->PutCh('\t');
    SOut->PutFlt(BreakEvenPoint()); SOut->PutCh('\t'); SOut->PutFlt(BreakEvenPointStDiv()); 
    for (int StatN = 0; StatN < GetStats(); StatN++) {
        SOut->PutCh('\t'); SOut->PutFlt(GetStatWgt(StatN)); }    
    SOut->PutCh('\n');
}

//////////////////////////////////////////////////////////////////////////
// Count -- Classification Results
void TCountCfyRes::GetDec(const TFltV& ValV, TFltV& DecValV) {
	const int Folds = 10, Vals = ValV.Len(); DecValV.Gen(Folds + 1);
	for (int Fold = 0; Fold <= Folds; Fold++) { 
		const int ValN = TInt::GetMn(Vals-1, 
			TFlt::Round((double(Fold) / double(Folds)) * double(Vals)));
		DecValV[Fold] = ValV[ValN];
	}
}

void TCountCfyRes::GetDec(const TFltPrV& ValV, TFltPrV& DecValV) {
	const int Folds = 10, Vals = ValV.Len(); DecValV.Gen(Folds + 1);
	for (int Fold = 0; Fold <= Folds; Fold++) { 
		const int ValN = TInt::GetMn(Vals-1, 
			TFlt::Round((double(Fold) / double(Folds)) * double(Vals)));
		DecValV[Fold] = ValV[ValN];
	}
}

TCfyRes TCountCfyRes::ToMacroTCfyRes(TVec<TCountCfyRes>& CfyResV) {
    TCfyRes CfyRes;
    for (int CfyResN = 0; CfyResN < CfyResV.Len(); CfyResN++) {
        CfyResV[CfyResN].PrepareResultV();
        Fail;
        //TODO temporary-used warning CfyRes.Add(CfyResV[CfyResN].ToTCfyRes());
    }
    CfyRes.Def();
    return CfyRes;
}

void TCountCfyRes::Add(const double& PredCls, const double& TrueCls) {
	ResultV.Add(TFltKd(PredCls, TrueCls));
	ResultVSorted = false;
	if (TrueCls > 0.0) { 
        if (PredCls > 0.0) { TPos++; }
        else { FNeg++; }
	} else if (TrueCls < 0.0) { 
        if (PredCls < 0.0) { TNeg++; }
        else { FPos++; }
    } else { Fail; }
}

double TCountCfyRes::AvgPrec() {
	PrepareResultV();
    int PosN = (TPos+FNeg), PosC = 0, i = 0;
    double ap = 0.0;
    while (PosC < PosN) {
        i++;
        if (ResultV[i-1].Dat > 0.0) {
            PosC++;
            ap += (double)PosC/(double)i;
        }
    }
    if (PosN > 0.0) { ap /= PosN; }
    return ap;
}

double TCountCfyRes::BreakEvenPoint() {
	PrepareResultV();
    const int Len = ResultV.Len();
    int TP = 0, FP = 0, TN = FPos + TNeg, FN = TPos + FNeg;
    double Prec = 1.0, Rec = 0.0, Diff = 1.0, BEP = 0.0; 

    for (int i = 0; i < Len; i++) {
        if (Diff > TFlt::Abs(Prec - Rec) && Prec != 0.0 && Rec != 0.0) {
            Diff = TFlt::Abs(Prec - Rec);
            BEP = (Prec + Rec)/2;
        }
        if (ResultV[i].Dat > 0.0) {
            FN--; TP++;
        } else {
            TN--; FP++; 
        }
        Prec = (double)TP/(TP+FP); 
        Rec = (double)TP/(TP+FN);
    }

    return BEP;
}

void TCountCfyRes::PrecRec(TFltV& PrecV, TFltV& RecV) {
	PrepareResultV();
    const int Len = ResultV.Len();
    int TP = 0, FP = 0, TN = FPos + TNeg, FN = TPos + FNeg;
    double Prec = 1.0, Rec = 0.0; 
    PrecV.Gen(Len+1, 0); RecV.Gen(Len+1,0);

    PrecV.Add(Prec); RecV.Add(Rec);
    for (int i = 0; i < Len; i++) {
        if (ResultV[i].Dat > 0.0) { FN--; TP++; }
        else { TN--; FP++; }
        Prec = (double)TP/(TP+FP); Rec = (double)TP/(TP+FN);
        PrecV.Add(Prec); RecV.Add(Rec);
    }
}

void TCountCfyRes::DecPrecRec(TFltV& DecPrecV, TFltV& DecRecV) {
	TFltV PrecV, RecV; PrecRec(PrecV, RecV);
	GetDec(PrecV, DecPrecV); GetDec(RecV, DecRecV);
}

void TCountCfyRes::Roc(TFltPrV& RocV) {
	PrepareResultV(); 
    const int Len = ResultV.Len(); RocV.Gen(Len, 0);
    int TP = 0, FP = 0, TN = FPos + TNeg, FN = TPos + FNeg;
    for (int i = 0; i < Len; i++) {
        if (ResultV[i].Dat > 0.0) { FN--; TP++; } else { TN--; FP++; }
        const double FPR = double(FP) / double(FP + TN);
		const double TPR = double(TP) / double(TP + FN);
        RocV.Add(TFltPr(FPR, TPR));
    }
}

void TCountCfyRes::DecRoc(TFltPrV& DecRocV) {
	TFltPrV RocV; Roc(RocV);
	GetDec(RocV, DecRocV);
}

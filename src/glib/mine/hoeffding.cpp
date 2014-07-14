namespace THoeffding {
	///////////////////////////////
	// constant-definitions
	// const int BinsN;

	///////////////////////////////
	// simple-lexical-analyzer
	TToken TLexer::GetNextTok() {
		if (BackP) {
			BackP = false;
			return LastTok;
		}
		CurrCh = SIn->GetCh();
		// skip whitespace 
		EatWs();
		// ignore comments 
		while (CurrCh == '#') { SkipLn(); EatWs(); }

		switch (CurrCh) {
		case '(':
			LastTok = TToken("(", totLPARENTHESIS, LineN);
			return LastTok;
		case ')':
			LastTok = TToken(")", totRPARENTHESIS, LineN);
			return LastTok;
		case ':':
			LastTok = TToken(":", totCOLON, LineN);
			return LastTok;
		case ';':
			LastTok = TToken(";", totSEMIC, LineN);
			return LastTok;
		case ',':
			LastTok = TToken(",", totCOMMA, LineN);
			return LastTok;
		case '=':
			LastTok = TToken("=", totEQU, LineN);
			return LastTok;
		}
		// identifier?
		if (IsValid(CurrCh)) {
			TChA ChA;
			ChA.AddCh(CurrCh);
			while (!SIn->Eof() && IsValid(SIn->PeekCh())) {
				CurrCh = SIn->GetCh();
				ChA.AddCh(CurrCh);
			}
			if (ChA == "dataFormat") {
				LastTok = TToken(ChA, totDFORMAT, LineN);
			} else if (ChA == "discrete") {
				LastTok = TToken(ChA, totDISCRETE, LineN);
			} else if (ChA == "numeric") {
				LastTok = TToken(ChA, totNUMERIC, LineN);
			} else {
				LastTok = TToken(ChA, totID, LineN);
			}
		} else {
			if (!SIn->Eof()) {
				printf("[Line %d] Illegal character: '%c'.\n", LineN, CurrCh);
				Fail;
			}
			LastTok = TToken("", totEND, LineN); // end-of-file
		}
		return LastTok;
	}

	void TLexer::EatWs() {
		while (!SIn->Eof() && TCh::IsWs(CurrCh)) {
			if (CurrCh == '\n') ++LineN;
			CurrCh = SIn->GetCh();
		}
	}

	void TLexer::SkipLn() {
		while (!SIn->Eof() && CurrCh != '\n') { CurrCh = SIn->GetCh(); }
	}
	///////////////////////////////
	// parameters
	void TParser::CfgParse(const TStr& FileNm) {
		TLexer Lexer(FileNm);
		TToken Tok;
		//while ((Tok = Lexer.GetNextTok()).Type != totEND) {
		//	printf("\tToken: %s\n", Tok.Val.CStr());
		//}
		printf("--------- Parsing format specification ---------\n");
		InitLine(Lexer);
		printf("--------- Parsing attributes ---------\n");
		AttrLine(Lexer);
		printf("Parsing succeeded!\n\n"); 
	}

	void TParser::InitLine(TLexer& Lexer) {
		TToken Tok;
		Tok = Lexer.GetNextTok();
		if (Tok.Type != totDFORMAT) {
			printf("[Line %d] Expected 'dataFormat' keyword instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
			Fail;
		}
		// =
		Tok = Lexer.GetNextTok();
		if (Tok.Type != totCOLON) {
			printf("[Line %d] Expected ':' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
			Fail;
		}
		// (
		Tok = Lexer.GetNextTok();
		if (Tok.Type != totLPARENTHESIS) {
			printf("[Line %d] Expected '(' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
			Fail;
		}
		// parameter list 
		InitParam(Lexer);
		// )
		Tok = Lexer.GetNextTok();
		if (Tok.Type != totRPARENTHESIS) {
			printf("[Line %d] Expected ')' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
			Fail;
		}
	}

	void TParser::InitParam(TLexer& Lexer) {
		TToken Tok;
		int IdxN = 0;
		while (true) {
			// id
			Tok = Lexer.GetNextTok();
			if (Tok.Type != totID) {
				printf("[Line %d] Expected identifier instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			printf("Declaring '%s'...\n", Tok.Val.CStr());
			DataFormatH.AddDat(Tok.Val, IdxN);
			InvDataFormatH.AddDat(IdxN, Tok.Val);
			// ,
			Tok = Lexer.GetNextTok();
			if (Tok.Type == totRPARENTHESIS) { break; } // end of parameter list 
			if (Tok.Type != totCOMMA) {
				printf("[Line %d] Expected ',' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			++IdxN;
		}
		++IdxN;
		AttrsHV.Reserve(IdxN, IdxN);
		InvAttrsHV.Reserve(IdxN, IdxN);
		Lexer.PutBack();
	}

	void TParser::AttrLine(TLexer& Lexer) {
		TToken Tok;
		TStr AttrNm;
		while (true) {
			// id
			Tok = Lexer.GetNextTok();
			if (Tok.Type == totEND) { break; } // end-of-file
				
			if (Tok.Type != totID) {
				printf("[Line %d] Expected identifier instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			AttrNm = Tok.Val;
			// make sure attribute was ``declared'' in dataFormat statement 
			if (!DataFormatH.IsKey(AttrNm)) {
				printf("Attribute '%s' is undeclared.\n", AttrNm.CStr());
				Fail;
			}
			// :
			Tok = Lexer.GetNextTok();
			if (Tok.Type != totCOLON) {
				printf("[Line %d] Expected ':' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			// discrete/numeric
			Tok = Lexer.GetNextTok();
			if (Tok.Type == totDISCRETE) {
				printf("Nominal attirubte %s\n", AttrNm.CStr());
				// (
				Tok = Lexer.GetNextTok();
				if (Tok.Type != totLPARENTHESIS) {
					printf("[Line %d] Expected '(' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
					Fail;
				}
				AttrParam(Lexer, AttrNm);
				// )
				Tok = Lexer.GetNextTok();
				if (Tok.Type != totRPARENTHESIS) {
					printf("[Line %d] Expected ')' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
					Fail;
				}
			} else if (Tok.Type == totNUMERIC) {
				printf("Numeric atribute\n");
				const int CountN = DataFormatH.GetDat(AttrNm);
				AttrsHV.GetVal(CountN).AddDat("", 0);
				InvAttrsHV.GetVal(CountN).AddDat(0, "");
			} else {
				printf("[Line %d] Expected attribute (discrete or numeric) type instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
		}
	}

	void TParser::AttrParam(TLexer& Lexer, const TStr& AttrNm) {
		int IdxN = 0;
		const int CountN = DataFormatH.GetDat(AttrNm);
		TToken Tok;
		TStr ValNm;
		while (true) { // loop through all values 
			// id
			Tok = Lexer.GetNextTok();
			if (Tok.Type != totID) {
				printf("[Line %d] Expected identifier instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			ValNm = Tok.Val;
			// set up the mappings 
			AttrsHV.GetVal(CountN).AddDat(ValNm, IdxN);
			InvAttrsHV.GetVal(CountN).AddDat(IdxN, ValNm);
			// ,
			Tok = Lexer.GetNextTok();
			if (Tok.Type == totRPARENTHESIS) { break; } // end of parameter list 
			if (Tok.Type != totCOMMA) {
				printf("[Line %d] Expected ',' instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
				Fail;
			}
			++IdxN;
		}
		Lexer.PutBack();
	}

	void TParser::Error(const TStr& Msg) {
		throw TExcept::New("Parsing error.");
	}

	///////////////////////////////
	// Helper-functions
	void TMisc::AddVec(const int& Scalar, TIntV& FstV, TIntV& SndV) {
		while (FstV.Len() < SndV.Len()) { FstV.Add(0); }
		while (FstV.Len() > SndV.Len()) { SndV.Add(0); }
		
		for (int ElN = 0; ElN < FstV.Len(); ++ElN) {
			SndV[ElN] += Scalar*FstV[ElN];
		}
	}
	double TMisc::Entropy(const TIntV& FreqV, const int& N) {
		double h = 0.0, p = 0.0;
		for (auto It = FreqV.BegI(); It != FreqV.EndI(); ++It) {
			p = N > 0 ? 1.0*It->Val/N : 0.0;
			if (p > 0) { h -= p*TMath::Log2(p); }
		}
		return h;
	}

	///////////////////////////////
	// Bin
	bool operator<=(const TBin& Bin1, const TBin& Bin2) {
		return Bin1.Value <= Bin2.Value;
	}
	bool operator>=(const TBin& Bin1, const TBin& Bin2) {
		return Bin1.Value >= Bin2.Value;
	}
	bool operator<(const TBin& Bin1, const TBin& Bin2) {
		return Bin1.Value < Bin2.Value;
	}
	bool operator>(const TBin& Bin1, const TBin& Bin2) {
		return Bin1.Value > Bin2.Value;
	}
	bool operator==(const TBin& Bin1, const TBin& Bin2) {
		return Bin1.Value == Bin2.Value;
	}
	bool operator!=(const TBin& Bin1, const TBin& Bin2) {
		return !(Bin1 == Bin2);
	}
	double TBin::Entropy() const {
		return TMisc::Entropy(PartitionV, Count);
	}

	///////////////////////////////
	// Histogram
	// Per-class distribution for examples with attribute 
	// NOTE: This function must ensure the Example->BinId is set to the maximum ID of the bins containing the example 
	void THist::IncCls(PExample Example, const int& AttrIdx, PIdGen IdGen) {
		int Idx = 0, BinN = 0;
		double CurrDist = 0.0, PrevDist = 0.0;
		const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
		const int Label = Example->Label;
		// Add new bin, initialized with Val, if the number of bins didn't reach the treshold 
		if ((Idx = BinsV.SearchBin(Val)) == -1 && BinsV.Len() < BinsN) {
			const int CrrBinId = IdGen->GetNextBinId();
			const int CrrIdx = BinsV.AddSorted(TBin(Val, CrrBinId), true);
			BinsV.GetVal(CrrIdx).Inc(Label);
			Example->SetBinId(TMath::Mx<int>(Example->BinId, CrrBinId));
		} else { // Find the closest bin 
			if (Idx != -1) { // Bin initialized with this very value 
				BinsV.GetVal(Idx).Inc(Label);
				Example->SetBinId(TMath::Mx<int>(Example->BinId, BinsV.GetVal(Idx).Id));
			} else { // Otherwise, increment the closest bin 
				Idx = 0;
				PrevDist = CurrDist = abs(Val - BinsV.GetVal(0).GetVal());
				// NOTE: We could use binary search because of the ordering invariant; but the number of bins rarely exeecds 100 (hardcoded constant)
				// While distance starts increasing, stop --- our bin is the one before the current one 
				for (BinN = 1; BinN < BinsV.Len(); ++BinN) {
					PrevDist = CurrDist;
					if ((CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal())) > PrevDist) {
						Idx = BinN-1; break;
					}
				}
				if (BinN == BinsV.Len()) { Idx = BinN-1; }
				BinsV.GetVal(Idx).Inc(Label);
				Example->SetBinId(TMath::Mx<int>(Example->BinId, BinsV.GetVal(Idx).Id));
			}
		}
	}
	// NOTE: This function must ensure the example is removed from the bins that existed at the time of its arrival 
	void THist::DecCls(PExample Example, const int& AttrIdx) {
		int Idx = 0, BinN = 0, PrevIdx = 0;
		double CurrDist = 0.0, PrevDist = 0.0;
		const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
		const int Label = Example->Label;
		// Idx = BinsV.SearchBin(Val); // Binary search for Val 
		if ((Idx = BinsV.SearchBin(Val)) == -1 && BinsV.Len() < BinsN) {
			// printf("Searching for value: %f\n", Val);
			Print();
			FailR("By construction, the value cannot be missing."); // NOTE: For deubgging purposes 
		} else { // Find the closest bin 
			if (Idx != -1 && BinsV.GetVal(Idx).Id <= Example->BinId) { // Bin initialized with this very value 
				BinsV.GetVal(Idx).Dec(Label);
			} else { // Otherwise, decrement the closest bin that WAS NOT created after the example was accumulated 
				Idx = 0;
				// NOTE: We can't take the first bin as it may have been created AFTER the example was accumulated; instead we find the first suitable bin 
				for (BinN = 0; BinN < BinsV.Len() && BinsV.GetVal(BinN).Id > Example->BinId; ++BinN);
				AssertR(BinN < BinsV.Len(), "No suitable bin --- impossible."); // NOTE: For debugging purposes 
				PrevIdx = Idx = BinN; // First suitable bin 
				PrevDist = CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal());
				// The order is preserved even though new bins might have been created between the old ones 
				for (; BinN < BinsV.Len(); ++BinN) {
					if (BinsV.GetVal(BinN).Id <= Example->BinId) {
						PrevDist = CurrDist;
						if ((CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal())) > PrevDist) {
							Idx = PrevIdx; break;
						} else { PrevIdx = BinN; }
					}
				}
				if (BinN == BinsV.Len() && Idx != PrevIdx) { Idx = PrevIdx; }
				BinsV.GetVal(Idx).Dec(Label);
			}
		}
	}
	void THist::IncReg(const PExample Example, const int& AttrIdx) {
		int Idx = 0, BinN = 0;
		double CurrDist = 0.0, PrevDist = 0.0;
		const double Val = Example->AttributesV.GetVal(AttrIdx).Num; // Numeric attribute value 
		const double RegValue = Example->Value; // Value of the target variable 
		if (BinsV.Len() < BinsN && (Idx = BinsV.SearchForw(Val, 0)) == -1) { // BinsV.SearchBin(Val)) == -1) {
			const int TmpIdx = BinsV.AddSorted(TBin(Val), true);
			BinsV.GetVal(TmpIdx).Inc(RegValue);
		} else { // Find the closest bin 
			if (Idx != -1) { // Bin initialized with this very value 
				BinsV.GetVal(Idx).Inc(RegValue);
			} else { // Otherwise, increment the closest bin 
				Idx = 0;
				CurrDist = PrevDist = abs(Val - BinsV.GetVal(0).GetVal());
				for (BinN = 1; BinN < BinsV.Len(); ++BinN) {
					PrevDist = CurrDist;
					// We are fine, because bins are ordered inside the vector by the initialization values 
					if ((CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal())) > PrevDist) {
						Idx = BinN - 1;
						break;
					}
				}
				if(BinN == BinsV.Len()) { Idx = BinN-1; }
				BinsV.GetVal(Idx).Inc(RegValue);
			}
		}
	}
	// Find best split 
	double THist::InfoGain(double& SplitVal) const {
		int HiCount = 0, LoCount = 0, CurrCount = 0, MxIdx = 0;
		double MxGain = 0.0, CurrGain = 0.0;
		double LoImp = 0.0, HiImp = 0.0;
		TIntV LoV, HiV;
		double* GArr = new double[sizeof(double)*BinsV.Len()]();
		int* NArr = new int[sizeof(int)*BinsV.Len()]();
		// Compute initial split 
		LoCount = 0; // BinsV.GetVal(0).Count;
		// LoV = BinsV.GetVal(0).PartitionV;
		HiCount = 0;
		for (int BinN = 0; BinN < BinsV.Len(); ++BinN) {
			TIntV TmpV = BinsV.GetVal(BinN).PartitionV;
			TMisc::AddVec(1, TmpV, HiV); // HiV = HiV+TmpV
			HiCount += BinsV.GetVal(BinN).Count;
			GArr[BinN] = TMisc::Entropy(HiV, HiCount); // h_i := H(B_1\cup B_2\cup \ldots\cup B_i)
			NArr[BinN] = HiCount; // n_i := |B_1|+\ldots+|B_i|
		}
		const int AllN = HiCount;
		const double H = TMisc::Entropy(HiV, AllN);
		// printf("H = %f\n", H);
		// Now find the best split 
		CurrGain = MxGain = 0.0;
		MxIdx = 0;
		for (int BinN = BinsV.Len()-2; BinN >= 0; --BinN) {
			CurrCount = BinsV.GetVal(BinN+1).Count;
			// No need for this: BinsV.GetVal(MxIdx).GetVal()
			// Val = BinsV.GetVal(BinN+1).Value; 
			LoCount += CurrCount;
			HiCount = NArr[BinN];
			HiImp = GArr[BinN];
			TIntV TmpV = BinsV.GetVal(BinN+1).PartitionV;
			TMisc::AddVec(1, TmpV, LoV);
			LoImp = TMisc::Entropy(LoV, LoCount);
			if ((CurrGain = H - LoCount*LoImp/AllN - HiCount*HiImp/AllN) > MxGain) {
				MxGain = CurrGain;
				MxIdx = BinN;
			}
		}
		delete GArr;
		delete NArr;
		if (MxIdx > 0) {
			SplitVal = BinsV.GetVal(MxIdx).GetVal();
			return MxGain;
		} else {
			return 0;
		}
	}

	double THist::GiniGain(double& SpltVal) const {
		EFailR("Implementation in progress.");
		return 0.0;
	}

	// See [Knuth, 1997] and [Chan et al., 1979] for details regarding updating formulas for variance 
	// (Wikipedia link: http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance, accessed on 7 Jun 2013)
	double THist::StdGain(double& SpltVal) const { // for regression 
		int HiCnt, LoCnt, CrrCnt;
		int MxIdx;
		double MxGain, CrrGain;
		double LoS, HiS, LoT, HiT;
		double* SArr = new double[BinsN](); // Define VarArr[i] := n*Var(B_1\cup B_2\cup ...\cup B_i) 
		double* TArr = new double[BinsN](); // Define AvgArr[i] := x_1+x_2+...+x_i
		// Compute initial split 
		LoCnt = HiCnt = 0; // BinsV.GetVal(0).Count;
		SArr[0] = TArr[0] = 0;
		for (int BinN = 0; BinN < BinsV.Len(); ++BinN) {
			const TBin CrrBin = BinsV.GetVal(BinN);
			const double PrevS = BinN > 0 ? SArr[BinN-1] : 0;
			const double PrevT = BinN > 0 ? TArr[BinN-1] : 0;
			TArr[BinN] = PrevT + CrrBin.T;
			CrrCnt = BinsV.GetVal(BinN).Count;
			SArr[BinN] = PrevS+CrrBin.S;
			if (CrrCnt > 0 && HiCnt > 0) {
				SArr[BinN] += TMath::Sqr(CrrCnt*PrevT/HiCnt-CrrBin.T)*HiCnt/(CrrCnt*(CrrCnt+HiCnt));
			}
			HiCnt += CrrCnt;
		}
		const int AllN = HiCnt;
		const double S = SArr[BinsV.Len()-1];
		// printf("AllN = %d; S = %f\n", AllN, S);
		HiS = CrrGain = MxGain = 0.0;
		MxIdx = 0;
		LoS = BinsV.Last().S;
		LoT = BinsV.Last().T;
		LoCnt = BinsV.Last().Count;
		HiCnt -= LoCnt;
		// Compute expected variance reduction, as defined by [Ikonomovska, 2012] and [Ikonomovska et al., 2011]
		for (int BinN = BinsV.Len()-2; BinN >= 0; --BinN) {
			HiS = SArr[BinN];
			HiT = TArr[BinN];
			const double SigmaS = TMath::Sqrt(S/(AllN));
			const double SigmaS1 = TMath::Sqrt(LoS/(LoCnt));
			const double SigmaS2 = TMath::Sqrt(HiS/(HiCnt));
			// printf("S = %f ;; S1 = %f ;; S2 = %f\n", SigmaS, LoCnt*SigmaS1/AllN, HiCnt*SigmaS2/AllN);
			if ((CrrGain = SigmaS - LoCnt*SigmaS1/AllN - HiCnt*SigmaS2/AllN) > MxGain) {
				MxGain = CrrGain;
				MxIdx = BinN;
			}
			// printf("CurrGain = %f\n", CrrGain); getchar();
			// Update variance 
			const double CrrS = BinsV.GetVal(BinN).S; // S_2
			const double CrrT = BinsV.GetVal(BinN).T;
			CrrCnt = BinsV.GetVal(BinN).Count; // n
			LoS += CrrS;
			if (LoCnt > 0 && CrrCnt > 0) {
				LoS += TMath::Sqr(CrrCnt*LoT/LoCnt-CrrT)*LoCnt/(CrrCnt*(CrrCnt+LoCnt));
			}
			LoT += BinsV.GetVal(BinN).T;
			LoCnt += CrrCnt;
			HiCnt -= CrrCnt;
		}
		delete [] TArr;
		delete [] SArr;
		if (MxIdx > 0) {
			SpltVal = BinsV.GetVal(MxIdx).GetVal();
			// printf("MxGain = %f\n", MxGain);
			return MxGain;
		} else {
			return 0;
		}
	}

	void THist::Print() const {
		for (auto It = BinsV.BegI(); It != BinsV.EndI(); ++It) {
			printf("%f (%d)\t", It->GetVal(), It->Count);
		}
		putchar('\n');
	}

	///////////////////////////////
	// Attribute
	
	///////////////////////////////
	// Attribute-Managment
	TAttrMan::TAttrMan(const THash<TStr, TInt>& AttrH_, const THash<TInt, TStr>& InvAttrH_,
		const int& Id_, const TStr& Nm_, const TAttrType& Type_)
		: AttrH(AttrH_), InvAttrH(InvAttrH_), Type(Type_), Nm(Nm_), Id(Id_) {
			AttrH.GetDatV(ValueV); // Possible values; there is a single value for numeric attributes 
	}
	
	///////////////////////////////
	// Example
	TExample& TExample::operator=(const TExample& Example) {
		if(*this != Example) {
			AttributesV = Example.AttributesV;	BinId = Example.BinId;
			Label = Example.Label; LeafId = Example.LeafId;	Value = Example.Value;
		}
		return *this;
	}

	/////////////////////////////////
	// Node
	// Copy constructor 
	TNode::TNode(const TNode& Node)
		: CndAttrIdx(Node.CndAttrIdx), ExamplesN(Node.ExamplesN), Val(Node.Val), 
		Avg(Node.Avg), VarSum(Node.VarSum), Err(Node.Err), TestModeN(Node.TestModeN),
        Type(Node.Type), ExamplesV(Node.ExamplesV), PartitionV(Node.PartitionV), 
        UsedAttrs(Node.UsedAttrs), HistH(Node.HistH), Id(Node.Id), 
        Correct(Node.Correct), All(Node.All) { }
	// Assignment operator 
	TNode& TNode::operator=(const TNode& Node) {
		if (*this != Node) {
			//Clr(); // Delete old elements 
			All = Node.All; AltTreesV = Node.AltTreesV;
			Avg = Node.Avg; ChildrenV = Node.ChildrenV;
			CndAttrIdx = Node.CndAttrIdx; Correct = Node.Correct;
			Counts = Node.Counts; Err = Node.Err; ExamplesN = Node.ExamplesN;
			ExamplesV = Node.ExamplesV; HistH = Node.HistH; Id = Node.Id;
			PartitionV = Node.PartitionV;
//#ifdef GLIB_OK
			SeenH = Node.SeenH;
//#else
//			seen_h = Node.seen_h;
//#endif
			TestModeN = Node.TestModeN; Type = Node.Type;
			Val = Node.Val; VarSum = Node.VarSum;
		}
		return *this;
	}
	bool TNode::operator==(const TNode& Node) const {
		return CndAttrIdx == Node.CndAttrIdx && Type == Node.Type && ExamplesV == Node.ExamplesV &&
			Counts == Node.Counts && PartitionV == Node.PartitionV && Id == Node.Id &&
			ChildrenV == Node.ChildrenV && UsedAttrs == Node.UsedAttrs;
	}
	// Training set entropy 
	double TNode::ComputeEntropy() const {
		return TMisc::Entropy(PartitionV, ExamplesN);
	}
	double TNode::ComputeGini() const {
		double g = 1.0, p = 0.0;
		for(auto It = PartitionV.BegI(); It != PartitionV.EndI(); It++) {
			p = 1.0*(*It)/ExamplesN;
			g -= p*p;
		}
		return g;
	}
	// Compute inforation gain from sufficient statistics 
	double TNode::InfoGain(const int& AttrIndex, const TAttrManV& AttrManV) const {
		double h = 0, hj = 0, p = 0, pj = 0;
		int SubExamplesN = 0; // Number of examples x with A(x)=a_j for j=1,2,...,ValsN
		const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
		TAttrMan AttrMan(AttrManV.GetVal(AttrIndex));
		const int ValsN = AttrMan.ValueV.Len();
		// Compute entropy H(E) 
		h = TMisc::Entropy(PartitionV, ExamplesN);
		// Compute information gain 
		for (int j = 0; j < ValsN; ++j) {
			SubExamplesN = 0;
			// Compute |E_j|
			for (int i = 0; i < LabelsN; ++i) {
				TTriple<TInt, TInt, TInt> TmpTriple(AttrIndex, j, i);
				if(Counts.IsKey(TmpTriple)) {
					SubExamplesN += Counts.GetDat(TmpTriple);
				}
			}
			hj = 0;
			// Compute H(E_j)
			for (int i = 0; i < LabelsN; ++i) {
				TTriple<TInt, TInt, TInt> TmpTriple(AttrIndex, j, i);
				if (Counts.IsKey(TmpTriple)) {
					pj = SubExamplesN > 0 ? 1.0*Counts.GetDat(TmpTriple)/SubExamplesN : 0; // Prevent divison by zero 
					if (pj > 0) { // Ensure Log2(pj) exists 
						hj -= pj*TMath::Log2(pj);
					}
				}
			}
			p = ExamplesN > 0 ? 1.0*SubExamplesN/ExamplesN : 0;
			h -= p*hj;
		}
		// Return information gain G(A) 
		return h;
	}
	// Compute Gini index from sufficient statistics 
	double TNode::GiniGain(const int& AttrIndex, const TVec<TAttrMan>& AttrManV) const {
		double g = 1.0, gj = 0.0, p = 0, pj = 0;
		int SubExamplesN = 0; // Number of examples x with A(x)=a_j for j=1,2,...,ValsN
		const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
		TAttrMan AttrMan(AttrManV.GetVal(AttrIndex));
		const int ValsN = AttrMan.ValueV.Len();
		for (auto It = PartitionV.BegI(); It != PartitionV.EndI(); ++It) {
			p = ExamplesN > 0 ? 1.0*(*It)/ExamplesN : 0; // Prevent division by zero 
			g -= p*p;
		}
		for (int j = 0; j < ValsN; ++j) {
			SubExamplesN = 0;
			// Compute |E_j|
			for (int i = 0; i < LabelsN; ++i) {
				TTriple<TInt, TInt, TInt> TmpTriple(AttrIndex, j, i);
				if (Counts.IsKey(TmpTriple)) {
					SubExamplesN += Counts.GetDat(TmpTriple);
				}
			}
			gj = 1.0;
			for (int i = 0; i < LabelsN; ++i) {
				TTriple<TInt, TInt, TInt> TmpTriple(AttrIndex, j, i);
				if (Counts.IsKey(TmpTriple)) {
					pj = SubExamplesN > 0 ? 1.0*Counts.GetDat(TmpTriple)/SubExamplesN : 0; // Prevent divison by zero 
					gj -= pj*pj;
				}
			}
			p = ExamplesN > 0 ? 1.0*SubExamplesN/ExamplesN : 0;
			g -= p*gj;
		}
		// Return information gain GiniGain(A) 
		return g;
	}
	double TNode::StdGain(const int& AttrIdx, const TAttrManV& AttrManV) const {
		// NOTE: Compute variances Var(S_i) for all possible values attribute A_i can take 
		const TAttrType AttrType = AttrManV.GetVal(AttrIdx).Type;
		EAssertR(AttrType == atDISCRETE, "This function works with nominal attributes.");
		const int ValsN = AttrManV.GetVal(AttrIdx).ValueV.Len();
		TVec<TTriple<TFlt, TFlt, TInt> > VarV; // Vector of (mean, variance, n) pairs 
		// TODO: Avoid iterating over the vector twice --- is there a faster way to initialize the thing?
		for (int ValN = 0; ValN < ValsN; ++ValN) {
			VarV.Add(TTriple<TFlt, TFlt, TInt>(0.0, 0.0, 0));
		}
		// Incrementally compute variances 
		for (int ValN = 0; ValN < ExamplesV.Len(); ++ValN) {
			const int CrrIdx = ExamplesV.GetVal(ValN)->AttributesV.GetVal(AttrIdx).Value;
			const double CrrVal = ExamplesV.GetVal(ValN)->Value;
			TTriple<TFlt, TFlt, TInt>& CrrTriple = VarV.GetVal(CrrIdx);
			// See [Knuth, 1997] for details regarding incremental algorithms for variance 
			const int N = CrrTriple.Val3++;
			const double Delta = CrrVal - CrrTriple.Val1;
			CrrTriple.Val1 += Delta/N;
			CrrTriple.Val2 += Delta*(CrrVal - CrrTriple.Val1);
		}
		double CrrStd = Std();
		for (int ValN = 0; ValN < ValsN; ++ValN) {
			const int CrrN = VarV.GetVal(ValN).Val3;
			if (CrrN > 0) {
				const double TmpStd = TMath::Sqrt(VarV.GetVal(ValN).Val2/CrrN);
				CrrStd -= CrrN*TmpStd/ExamplesN;
			}
		}
		return CrrStd;
	}
	TBstAttr TNode::BestAttr(const TAttrManV& AttrManV, const TTaskType& TaskType) {
		if (TaskType == ttCLASSIFICATION) {
			return BestClsAttr(AttrManV);
		} else {
			return BestRegAttr(AttrManV);
		}
	}
	TBstAttr TNode::BestRegAttr(const TAttrManV& AttrManV) { // Regression 
		const int AttrsN = AttrManV.Len()-1; // AttrsManV includes attribute manager for the label 
		double CrrSdr, Mx1, Mx2;
		int Idx1, Idx2;
		CrrSdr = Mx1 = Mx2 = 0;
		Idx1 = Idx2 = 0;
		for (int AttrN = 0; AttrN < AttrsN; ++AttrN) {
			const TAttrType AttrType = AttrManV.GetVal(AttrN).Type;
			if (AttrType == atDISCRETE) { // Discrete 
				if (UsedAttrs.SearchForw(AttrN, 0) < 0) {
					// Compute standard deviation reduction 
					CrrSdr = StdGain(AttrN, AttrManV);
				}
			} else { // Continuous 
				CrrSdr = HistH.GetDat(AttrN).StdGain(Val);
				// printf("SplitVal = %f\n", CrrSdr);
			}
			if (CrrSdr > Mx1) {
				Idx2 = Idx1; Idx1 = AttrN; Mx2 = Mx1; Mx1 = CrrSdr;
			} else if (CrrSdr >= Mx2) {
				Idx2 = AttrN; Mx2 = CrrSdr;
			}
		}
		const double Ratio = Mx2/Mx1;
		return TBstAttr(TPair<TInt, TFlt>(Idx1, Mx1), TPair<TInt, TFlt>(Idx2, Mx2), Ratio);
	}
	TBstAttr TNode::BestClsAttr(const TAttrManV& AttrManV, const TIntV& BannedAttrV) { // Classification
		int Idx1, Idx2;
		double Mx1, Mx2, Crr, SplitVal;
		const int AttrsN = AttrManV.Len()-1;
		Crr = Mx1 = Mx2 = 0;
		Idx1 = Idx2 = -1;
		for (int AttrN = 0; AttrN < AttrsN; ++AttrN) {
			// NOTE: BannedAttrV almost never contains more than two indices 
			if (BannedAttrV.IsIn(AttrN)) { continue; }
			if (AttrManV.GetVal(AttrN).Type == atDISCRETE) {
				if (UsedAttrs.SearchForw(AttrN, 0) < 0) {
					Crr = InfoGain(AttrN, AttrManV);
				}
			} else { // Numeric attribute 
				Crr = HistH.GetDat(AttrN).InfoGain(SplitVal);
				// HistH.GetDat(AttrN).Print();
				// getchar();
				Val = SplitVal;
			}
			if (Crr > Mx1) {
				Idx2 = Idx1; Idx1 = AttrN; Mx2 = Mx1; Mx1 = Crr;
			} else if (Crr > Mx2) {
				Idx2 = AttrN; Mx2 = Crr;
			}
		}
		const double Diff = Mx1 - Mx2;
		return TBstAttr(TPair<TInt, TFlt>(Idx1, Mx1), TPair<TInt, TFlt>(Idx2, Mx2), Diff);
	}
	// See [Domingos and Hulten, 2000] and [Hulten et al., 2001] for explanation
	double TNode::ComputeTreshold(const double& Delta, const int& LabelsN) const {
		const double R = TMath::Log2(LabelsN); // Range of the random variable for information gain 
		// EAssertR(!ExamplesN > 0, "This node has no examples.\n");
		return TMath::Sqrt(R*R*TMath::Log(1.0/Delta)/(2.0*ExamplesN)); // t = \sqrt{ \frac{R^2 * log(1/delta)}{2n} }
	}
	void TNode::Split(const int& AttrIdx, const TAttrManV& AttrManV, PIdGen IdGen) {
		// (i) Mark attribute, if discrete, as used
		// New child for each value of AttrIdx attribute 
		CndAttrIdx = AttrIdx;
		const TAttrType AttrType = AttrManV.GetVal(AttrIdx).Type;
		int ValsN = AttrManV.GetVal(AttrIdx).ValueV.Len();
		if (AttrType == atDISCRETE) { // Categorial attributes can only be used once 
			UsedAttrs.Add(AttrIdx);
		} else {
			ValsN = 2;
			// printf("[DEBUG] Splitting on continuous value %f\n", Val);
		}
		// UsedAttrs.Add(CondAttrIndex);
		const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
		for (int ValN = 0; ValN < ValsN; ++ValN) {
			ChildrenV.Add(TNode::New(LabelsN, UsedAttrs, AttrManV, IdGen->GetNextLeafId())); // Leaf node 
		}
		if(Type != ntROOT) { Type = ntINTERNAL; }
	}
	void TNode::Clr() { // Forget training examples 
		ExamplesV.Clr(); PartitionV.Clr(); Counts.Clr();
		HistH.Clr(true); AltTreesV.Clr(); UsedAttrs.Clr();
#ifdef GLIB_OK
		SeenH.Clr(true);
#endif
	}
	// See page 232 of Knuth's TAOCP, Vol. 2: Seminumeric Algorithms [Knuth, 1997] for details
	void TNode::UpdateStats(PExample Example) {
		++ExamplesN;
		const double CrrValue = Example->Value;
		const double Delta = CrrValue - Avg;
		Avg += Delta/ExamplesN;
		VarSum += Delta*(CrrValue - Avg);
		// Variance := VarSum/(ExamplesN - 1)
	}
	void TNode::UpdateErr(const double& Loss, const double& Alpha) {
		Err = Loss+Alpha*Err;
		if (++TestModeN > 500) { TestModeN = 0; }
	}
	void TNode::Init(const TAttrManV& AttrManV) {
		for (int AttrN = 0; AttrN < AttrManV.Len(); ++AttrN) {
			if (AttrManV.GetVal(AttrN).Type == atCONTINUOUS) {
				// const int LabelsN = AttrManV.GetVal(AttrN).ValueV.Last();
				HistH.AddDat(AttrN, THist());
			}
		}
	}

	/////////////////////////////////
	// Hoeffding-Tree
	double THoeffdingTree::Predict(PExample Example) const { // Regression
		PNode CrrNode = Root;
		while (CrrNode->CndAttrIdx != -1) {
			const TAttrType AttrType = AttrManV.GetVal(CrrNode->CndAttrIdx).Type;
			if (AttrType == atDISCRETE) {
				CrrNode = CrrNode->ChildrenV.GetVal(Example->AttributesV.GetVal(CrrNode->CndAttrIdx).Value);
			} else { // Numeric attribute 
				const double Val = Example->AttributesV.GetVal(CrrNode->CndAttrIdx).Num;
				const int Idx = Val <= CrrNode->Val;
				CrrNode = CrrNode->ChildrenV.GetVal(Idx);
			}
		}
		// Ikonomovska [Ikonomovska, 2012] trains perceptron in the leaves 
		return CrrNode->Avg;
	}
	TStr THoeffdingTree::Classify(PNode Node, PExample Example) const {
		PNode CrrNode = Node;
		while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
		return GetMajorityNm(CrrNode);
	}
	TStr THoeffdingTree::Classify(const TStrV& DiscreteV, const TFltV& NumericV) const {
		int DisIdx = 0, FltIdx = 0;
		TAttributeV AttributesV;
		const int AttrsN = AttrManV.Len();
		for (int AttrN = 0; AttrN < AttrsN-1; ++AttrN) {
			switch (AttrManV.GetVal(AttrN).Type) {
			case atDISCRETE:
				// printf(DiscreteV.GetVal(DisIdx).CStr());
				AttributesV.Add(TAttribute(AttrN, AttrsHashV.GetVal(AttrN).GetDat(DiscreteV.GetVal(DisIdx++))));
				break;
			case atCONTINUOUS:
				AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
				break;
			default:
				EFailR("Unsupported attribute type");
			}
		}
		TLabel Label = AttrsHashV.GetVal(AttrsN-1)[0]; // .operator[](0);
		return Classify(TExample::New(AttributesV, Label));
	}
	TStr THoeffdingTree::Classify(PExample Example) const { // Classification 
		PNode CrrNode = Root;
		while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
		return GetMajorityNm(CrrNode);
		// return NaiveBayes(CrrNode, Example);
	}
	void THoeffdingTree::IncCounts(PNode Node, PExample Example) const {
		Node->PartitionV.GetVal(Example->Label)++;
		Node->ExamplesN++;
		int AttrN = 0;
		for (auto It = Example->AttributesV.BegI(); It != Example->AttributesV.EndI(); ++It) {
			switch (AttrManV.GetVal(It->Id).Type) {
			case atDISCRETE: {
				TTriple<TInt, TInt, TInt> Idx(It->Id, It->Value, Example->Label);
				if (Node->Counts.IsKey(Idx)) {
					Node->Counts.GetDat(Idx)++;
				} else {
					Node->Counts.AddDat(Idx, 1);
				}
				break;										}
			case atCONTINUOUS:
				Node->HistH.GetDat(AttrN).IncCls(Example, AttrN, IdGen);
				break;
			default:
				EFailR("Attribute type not supported.");
			}
			++AttrN;
		}
	}
	void THoeffdingTree::DecCounts(PNode Node, PExample Example) const {
		EAssertR(Node->PartitionV.GetVal(Example->Label)-- >= 0, "Negative partition count.");
		EAssertR(--Node->ExamplesN >= 0, "Negative example count.");
		int AttrN = 0;
		for (auto It = Example->AttributesV.BegI(); It != Example->AttributesV.EndI(); ++It) {
			switch (AttrManV.GetVal(It->Id).Type) {
			case atDISCRETE: {
				TTriple<TInt, TInt, TInt> Idx(It->Id, It->Value, Example->Label);
				if (Node->Counts.IsKey(Idx)) {
					EAssertR(Node->Counts.GetDat(Idx)-- >= 0, "Negative id-value-label triple count.");
				} else {
					Print(Example);
					printf("Example ID: %d; Node ID: %d; Node examples: %d\n", Example->LeafId.Val, Node->Id, Node->ExamplesN);
					if(!IsLeaf(Node)) { printf("Node test attribute: %s\n", AttrManV.GetVal(Node->CndAttrIdx).Nm.CStr()); }
					printf("Problematic attribute: %s = %s\n", AttrManV.GetVal(It->Id).Nm.CStr(), AttrManV.GetVal(It->Id).InvAttrH.GetDat(It->Value).CStr());
					EFailR("Corresponding id-value-label triple is missing in counts hashtable."); // NOTE: For dbugging purposes; this fail probably indicates serious problems 
				}
				break;										}
			case atCONTINUOUS:
				Node->HistH.GetDat(AttrN).DecCls(Example, AttrN);
				break;
			default:
				EFailR("Attribute type not supported.");
			}
			++AttrN;
		}
	}
	bool THoeffdingTree::IsAltSplitIdx(PNode Node, const int& AttrIdx) const {
		for (auto It = Node->AltTreesV.BegI(); It != Node->AltTreesV.EndI(); ++It) {
			if ((*It)->CndAttrIdx == AttrIdx) { // || IsAltSplitIdx((*It)->Root, AttrIdx)) {
				return true;
			}
		}
		return false;
	}
	void THoeffdingTree::CheckSplitValidityCls() { // Classification 
		PNode CrrNode = Root;
		TSStack<PNode> NodeS;
		const int AttrsN = AttrManV.Len()-1; // Need -1 because AttrManV also manages class labels 
		NodeS.Push(CrrNode);
		// Depth-first tree traversal 
		while (!NodeS.Empty()) {
			CrrNode = NodeS.Top(); NodeS.Pop();
			// Check split validity in the alternate trees 
			for (auto It = CrrNode->AltTreesV.BegI(); It != CrrNode->AltTreesV.EndI(); ++It) {
				if (!IsLeaf(*It)) { NodeS.Push(*It); }
			}
			// Push non-leaf children on the stack 
			for (auto It = CrrNode->ChildrenV.BegI(); It != CrrNode->ChildrenV.EndI(); ++It) {
				if (!IsLeaf(*It)) { NodeS.Push(*It); }
			}
			// Find the best two attributes among the remaining attributes --- must not use CrrSplitAttrIdx 
			const int CrrSpltAttrIdx = CrrNode->CndAttrIdx;
			TVec<TInt> CrrBannedAttrV; CrrBannedAttrV.Add(CrrSpltAttrIdx);
			TBstAttr SpltAttr = CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV);
			CrrBannedAttrV.Clr(); CrrBannedAttrV.Add(SpltAttr.Val1.Val1);
			TBstAttr AltAttr = CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV);
			const double EstG = SpltAttr.Val1.Val2 - AltAttr.Val1.Val2;
			// Does it make sense to split on this one?
			if (EstG >= 0 && SpltAttr.Val1.Val1 != -1 && SpltAttr.Val2.Val1 != -1 && !IsAltSplitIdx(CrrNode, SpltAttr.Val1.Val1)) {
				// Hoeffding test
				const double Eps = CrrNode->ComputeTreshold(SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
				if (EstG > Eps || (Eps < TieBreaking && EstG >= TieBreaking/2)) { // EstG >= TieBreaking/2 ?
					// Grow alternate tree 
					Print('-');
					printf("Starting alternate tree for node splitting on `%s' with `%s' at root ; tie = %d\n", AttrManV.GetVal(CrrNode->CndAttrIdx).Nm.CStr(), AttrManV.GetVal(SpltAttr.Val1.Val1).Nm.CStr(), EstG <= Eps);
					// Export("exports/titanic-"+TInt(ExportN++).GetStr()+".gv", etDOT);
					const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
					PNode AltHt = TNode::New(LabelsN, CrrNode->UsedAttrs, AttrManV, IdGen->GetNextLeafId());
					AltHt->Split(SpltAttr.Val1.Val1, AttrManV, IdGen);
					CrrNode->AltTreesV.Add(AltHt);
					++AltTreesN;
				}
			}
		}
	}
	void THoeffdingTree::ForgetCls(PExample Example) const { // Classification 
		PNode CrrNode = Root;
		TSStack<PNode> NodeS;
		NodeS.Push(CrrNode);
		// EAssertR(!Sacrificed(CrrNode, Example), "Izgleda v redu.");
		while (!NodeS.Empty()) {
			CrrNode = NodeS.Top(); NodeS.Pop();
			if (CrrNode->Id <= Example->LeafId && !Sacrificed(CrrNode, Example)) {
				DecCounts(CrrNode, Example);
				if (!IsLeaf(CrrNode)) {
					NodeS.Push(GetNextNodeCls(CrrNode, Example));
					for (auto It = CrrNode->AltTreesV.BegI(); It != CrrNode->AltTreesV.EndI(); ++It) {
						if ((*It)->Id <= Example->LeafId) { NodeS.Push(*It); }
					}
				}
			} else if (Sacrificed(CrrNode, Example)) { // Unmark 
//#ifdef GLIB_OK
				CrrNode->SeenH.DelIfKey(*Example); 
//#else
//				CrrNode->seen_h.erase(*Example);
//#endif
			}
		}
	}
	void THoeffdingTree::ProcessLeafReg(PNode Leaf, PExample Example) { // Regression
		Leaf->UpdateStats(Example);
		// Leaf->ExamplesV.Add(Example);
		const int AttrsN = Example->AttributesV.Len();
		for (int AttrN = 0; AttrN < AttrsN; AttrN++) {
			if (AttrManV.GetVal(AttrN).Type == atCONTINUOUS) {
				// TODO: Find an efficient way to compute s(A) from s(A1) and s(A2) if A1 and A2 parition A
				Leaf->HistH.GetDat(AttrN).IncReg(Example, AttrN);
				// EFailR("Current regression discretization is deprecated.");
			}
		}
		if (Leaf->ExamplesN % GracePeriod == 0 && Leaf->Std() > 0) { // Regression
			// See if we can get variance reduction 
			TBstAttr SplitAttr = Leaf->BestAttr(AttrManV, TaskType);
			// Pass 2, because TMath::Log2(2) = 1; since r lies in [0,1], we have R=1; see also PhD thesis [Ikonomovska, 2012] and [Ikonomovska et al., 2011]
			const double Eps = Leaf->ComputeTreshold(SplitConfidence, 2);
			const double EstG = SplitAttr.Val3;
			printf("EstG = %f\n", EstG);
			if ((EstG < 1.0-Eps /*|| Eps < TieBreaking*/) && Leaf->UsedAttrs.SearchForw(SplitAttr.Val1.Val1, 0) < 0) {
				//printf("[DEBUG] Selected split attribute: %d\n", SplitAttr.Val1.Val1.Val);
				Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
			}
		}
	}
	void THoeffdingTree::ProcessLeafCls(PNode Leaf, PExample Example) { // Classification 
		const int AttrsN = Example->AttributesV.Len();
		IncCounts(Leaf, Example);
		// TODO: The constant 0.65 is hard-coded. 
		if (Leaf->ExamplesN % GracePeriod == 0 && Leaf->ComputeEntropy() > 0.65) {
			TBstAttr SplitAttr = Leaf->BestAttr(AttrManV, TaskType);
			const double EstG = SplitAttr.Val3;
			const double Eps = Leaf->ComputeTreshold(SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
			if (SplitAttr.Val1.Val1 != -1 && (EstG > Eps || (EstG <= Eps && Eps < TieBreaking))) {
				//printf("[DEBUG] best = %d :: tie = %d\n", EstG > Eps, EstG <= Eps && Eps < TieBreaking);
				//printf("[DEBUG] t = %f :: n = %d\n", Eps, Leaf->ExamplesN);
				//printf("[DEBUG] Splitting at %d examples on attribute `%s' with confidence %f\n", Leaf->ExamplesN, AttrManV.GetVal(SplitAttr.Val1.Val1).Nm.CStr(), 1.0-SplitConfidence);
				//if (Leaf->UsedAttrs.Len() > 0) {
				//	printf("[DEBUG] Previous attribute = %d; so far used %d attributes on this path.\n", Leaf->UsedAttrs.Last().Val, Leaf->UsedAttrs.LastValN()+1);
				//}
				Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
			}
		}
	}
	void THoeffdingTree::ProcessCls(PExample Example) {
		//////
		//TStr FNm = ConceptDriftP ? "err-cvfdt.dat" : "err-vfdt.dat";
		//Salpha = 0.995*Salpha+(Example->Label != Classify(Example));
		//Nalpha = 0.995*Nalpha+1;
		//if (Root->TestModeN == 9999) {
		//	PSOut FOut = TFOut::New(FNm, true);
		//	FOut->PutFlt(Salpha/Nalpha); FOut->PutLn();
		//}
		//////
		PNode CrrNode = Root;
		int MxId = 0;
		if (ConceptDriftP) {
			ExampleQ.Push(Example);
			if (ExampleQ.Len() > WindowSize) { // INVARIANT: ExampleQ.Len() <= WindowSize+1
				PExample LastExample = ExampleQ.Top();
				ExampleQ.Pop(); // Delete it from the window 
				ForgetCls(LastExample); // Update sufficient statistics 
			}
			TSStack<PNode> NodeS;
			NodeS.Push(CrrNode);
			while (!NodeS.Empty()) {
				CrrNode = NodeS.Top(); NodeS.Pop();
				if (IsLeaf(CrrNode)) { // Leaf node
					MxId = TMath::Mx<int>(MxId, CrrNode->Id);
					ProcessLeafCls(CrrNode, Example);
				} else {
					if (TestMode(CrrNode)) { // Don't update counts --- sacrifice the next 2000 or so examples for internal evaluation 
						SelfEval(CrrNode, Example);
					} else { // Everything goes as usual
						IncCounts(CrrNode, Example); // Update sufficient statistics 
						NodeS.Push(GetNextNodeCls(CrrNode, Example));
						for (auto It = CrrNode->AltTreesV.BegI(); It != CrrNode->AltTreesV.EndI(); ++It) {
							NodeS.Push(*It);
						}
					}
				}
			}
			Example->SetLeafId(TMath::Mx<int>(MxId, Example->LeafId));
			if (Root->HistH.Empty()) { Example->SetBinId(IdGen->GetNextBinId()); } /* Hack */
			if (++DriftExamplesN >= DriftCheck && !IsLeaf(CrrNode)) { // Make sure we're checking split validity of an internal node 
				DriftExamplesN = 0;
				CheckSplitValidityCls();
			}
		} else { // No concept drift detection 
			// if (!TestMode(CrrNode)) { // NOTE: This shoudl be false for VFDT because
			// AltTressV.Empty() is always true: there are no alternate trees in VFDT.
				while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
				ProcessLeafCls(CrrNode, Example);
			// } else {
			//	SelfEval(CrrNode, Example);
			// }
		}
	}
	void THoeffdingTree::ProcessReg(PExample Example) {
		PNode CrrNode = Root;
		while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
		ProcessLeafReg(CrrNode, Example);
	}
	void THoeffdingTree::SelfEval(PNode Node, PExample Example) const {
//#ifdef GLIB_OK
		Node->SeenH.AddDat(*Example, true);
//#else
//		Node->seen_h[*Example] = true;
//#endif
		// Update classification error for alternate trees 
		for (auto It = Node->AltTreesV.BegI(); It != Node->AltTreesV.EndI(); ++It) {
			PNode CrrNode = *It;
			while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
			(*It)->Correct += Example->Label == NaiveBayes(CrrNode, Example);
			++(*It)->All;
		}
		// Update classfication error for the main subtree 
		PNode CrrNode = Node;
		while (!IsLeaf(CrrNode)) { CrrNode = GetNextNodeCls(CrrNode, Example); }
		Node->Correct += Example->Label == NaiveBayes(CrrNode, Example);
		++Node->All;
	}
	bool THoeffdingTree::TestMode(PNode Node) {
		if (Node->AltTreesV.Empty() && Node->Type != ntROOT) { return false; }
		if (Node->All == 2000) { // Swap with the best performing subtree 
			PNode BestAlt = Node;
			//const double Acc = 1.0*BestAlt->Correct/BestAlt->All; // Classification accuracy 
			for (auto It = Node->AltTreesV.BegI(); It != Node->AltTreesV.EndI(); ++It) {
				if (1.0*(*It)->Correct/(*It)->All > 1.0*BestAlt->Correct/BestAlt->All) { BestAlt = *It; }
				else { (*It)->All = (*It)->Correct = 0; } // Reset 
			}
			if (BestAlt != Node) {
				//printf("[DEBUG] Swapping node with an alternate tree.\n");
				// Export("exports/titanic-"+TInt(ExportN++).GetStr()+".gv", etDOT);
				if(Node->Type == ntROOT) { BestAlt->Type = ntROOT; }
				*Node = *BestAlt;
			}
			Node->All = Node->Correct = 0; // Reset 
			/*
			if(Node->Type == ntROOT) {
				TStr FNm = ConceptDriftP ? "err-cvfdt.dat" : "err-vfdt.dat";
				PSOut FOut = TFOut::New(FNm, true);
				FOut->PutFlt(1.0-Acc); FOut->PutLn();
			}
			*/
			return false;
		} else if (Node->All == 0 && Node->TestModeN >= 10000) {
			// printf("Entering test mode...\n");
			Node->TestModeN = 0;
			return true;
		} else if (Node->All > 0) { return true; }
		++Node->TestModeN;
		return false;
	}
	void THoeffdingTree::Process(const TStrV& DiscreteV, const TFltV& NumericV, const TStr& Label) {
		int DisIdx = 0, FltIdx = 0;
		TAttributeV AttributesV;
		for (int AttrN = 0; AttrN < AttrManV.Len()-1; ++AttrN) {
			switch (AttrManV.GetVal(AttrN).Type) {
			case atDISCRETE:
				// printf(DiscreteV.GetVal(DisIdx).CStr());
				AttributesV.Add(TAttribute(AttrN, AttrsHashV.GetVal(AttrN).GetDat(DiscreteV.GetVal(DisIdx++))));
				break;
			case atCONTINUOUS:
				AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
				break;
			default:
				EFailR("Unknown attribute type");
			}
		}
		Process(TExample::New(AttributesV, AttrsHashV.Last().GetDat(Label)));
	}
	PExample THoeffdingTree::Preprocess(const TStr& Line, const TCh& Delimiter) const {
		TStrV LineV; TVec<TAttribute> AttributesV;
		Line.SplitOnAllCh(Delimiter, LineV);
		int ValN;
		EAssertR(AttrsHashV.Len() == LineV.Len(), "Number of attributes in the dataset doesn't match the number of attributes in the configuration file.");
		const int AttrsN = LineV.Len()-1;
		for (int CountN = 0; CountN < AttrsN; ++CountN) {
			// (1) Get appropriate hash table
			// (2) Get appropriate raw value from input attribute vector 
			// (3) Map raw attribute value to TInt with hash table 
			switch (AttrManV.GetVal(CountN).Type) {
			case atDISCRETE:
				if (LineV.GetVal(CountN)  == "?") {
					// EFailR("Missing values are not allowed.");
					// printf("[WARNING] Missing value; assuming default.\n");
					ValN = 0;
				} else {
					ValN = AttrsHashV.GetVal(CountN).GetDat(LineV.GetVal(CountN));
				}
				AttributesV.Add(TAttribute(CountN, ValN));
				break;
			case atCONTINUOUS:
				AttributesV.Add(TAttribute(CountN, LineV.GetVal(CountN).GetFlt()));
				break;
			default:
				EFailR("Unsupported attribute type.");
			}
		}
		if (TaskType == ttCLASSIFICATION) {
			return TExample::New(AttributesV, AttrsHashV.GetVal(AttrsN).GetDat(LineV.GetVal(AttrsN)));
		} else {
			return TExample::New(AttributesV, LineV.Last().GetFlt());
		}
	}
	PNode THoeffdingTree::GetNextNodeCls(PNode Node, PExample Example) const {
		if (!IsLeaf(Node)) {
			//printf("CndAttrIdx = %d\n", Node->CndAttrIdx);
			const TAttrType AttrType = AttrManV.GetVal(Node->CndAttrIdx).Type;
			if (AttrType == atDISCRETE) {
				//printf("# = %d\n", Node->ChildrenV.Len());
				// XXX: THIS IS THE BUG! 
				// TODO: Check whether all JS/C++ conversion are OK. 
				//printf("first ATTR VAL = %d\n", Example->AttributesV.GetVal(0));
				//printf("ATTR VAL = %d\n", Example->AttributesV.GetVal(Node->CndAttrIdx).Value);
				PNode RetNode = Node->ChildrenV.GetVal(Example->AttributesV.GetVal(Node->CndAttrIdx).Value);
				//printf("OK\n");
				//printf("return node ID: %d\n", RetNode->Id);
				return Node->ChildrenV.GetVal(Example->AttributesV.GetVal(Node->CndAttrIdx).Value);
			} else { // Numeric attribute 
				const double Num = Example->AttributesV.GetVal(Node->CndAttrIdx).Num;
				const int Idx = Num <= Node->Val ? 0 : 1;
				return Node->ChildrenV.GetVal(Idx);
			}
		}
		return nullptr; // No children 
	}
	void THoeffdingTree::Clr(PNode Node, PNode SubRoot) {
		TSStack<PNode> NodeS;
		for (auto It = Node->ChildrenV.BegI(); It != Node->ChildrenV.EndI(); ++It) {
			NodeS.Push(*It);	
		}
		for (auto It = Node->AltTreesV.BegI(); It != Node->AltTreesV.EndI(); ++It) {
			if (*It != SubRoot) { NodeS.Push(*It); }
		}
		PNode CrrNode = nullptr;
		while(!NodeS.Empty()) {
			CrrNode = NodeS.Top(); NodeS.Pop();
			for(auto It = CrrNode->ChildrenV.BegI(); It != CrrNode->ChildrenV.EndI(); ++It) {
				NodeS.Push(*It);
			}
			for(auto It = CrrNode->AltTreesV.BegI(); It != CrrNode->AltTreesV.EndI(); ++It) {
				NodeS.Push(*It);
			}
			CrrNode->Clr();
		}
		Node->Clr();
	}
	void THoeffdingTree::Export(const TStr& FileNm, const TExportType& ExportType) const {
		// printf("Writing the decision tree to `%s'.\n", FileNm.CStr());
		TFOut FOut(FileNm);
		switch (ExportType) {
		case etXML:
			FOut.PutStrLn("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>");
			FOut.PutStrFmtLn("<dt classes=%d>", LabelH.Len());
			PrintXML(Root, 1, FOut);
			FOut.PutStrFmtLn("</dt>");
			break;
		case etJSON:
			EFailR("Not yet supported.");
		case etDOT:
			FOut.PutStrFmtLn("digraph dt_fig {"); // %s {", FileNm.GetFBase().CStr());
			PrintDOT(Root, FOut, true);
			FOut.PutStrLn("}");
			break;
		default:
			EFailR("Uknown export format.");
		}
		FOut.Flush();
	}

	void THoeffdingTree::Init(const TStr& ConfigFNm) {
		TParser Parser(ConfigFNm);
		// NOTE: Although not critical, this can still be expensive. We should use C++11 move semantics.
		TParams Params = Parser.GetParams();
		AttrsHashV = Params.AttrsHV;
		InvAttrsHashV = Params.InvAttrsHV;
		LabelH = Params.DataFormatH;
		InvLabelH = Params.InvDataFormatH;
		
		InitAttrMan();
	}
	// TODO: Check whether this is valid 
	void THoeffdingTree::Init(PJsonVal JsonConfig) {
		EAssertR(JsonConfig->IsObjKey("dataFormat"), "Expected key 'dataFormat'.");
		PJsonVal DataFormatArr = JsonConfig->GetObjKey("dataFormat");
		EAssertR(DataFormatArr->IsArr(), "Expected array of attributes after 'dataFormat:'.");
		TStrV AttrNmV;
		DataFormatArr->GetArrStrV(AttrNmV);
		// printf("----------------------------\n");

		AttrsHashV.Reserve(AttrNmV.Len(), AttrNmV.Len());
		InvAttrsHashV.Reserve(AttrNmV.Len(), AttrNmV.Len());
		for (int AttrN = 0; AttrN != AttrNmV.Len(); ++AttrN) {
			LabelH.AddDat(AttrNmV.GetVal(AttrN), AttrN);
			InvLabelH.AddDat(AttrN, AttrNmV.GetVal(AttrN));
			// printf("-- Processing attribute '%s' --\n", AttrNmV.GetVal(AttrN).CStr());
			PJsonVal AttrVal = JsonConfig->GetObjKey(AttrNmV.GetVal(AttrN));
			EAssertR(AttrVal->IsObjKey("type"), "Expected key 'type' for each attribute.");
			EAssertR(AttrVal->IsObjKey("values"), "Expected key 'values' for each attribute.");
			// Retrieve possible values 
			if (AttrVal->GetObjStr("type") == "discrete") {
				PJsonVal ValuesArr = AttrVal->GetObjKey("values");
				EAssertR(ValuesArr->IsArr(), "Expected array of values after 'values' for discrete attribute.");
				TStrV ValuesV;
				// printf("\tValues: ");
				ValuesArr->GetArrStrV(ValuesV);
				// Now iterating through values of each of the attribute 
				for (int AttrValN = 0; AttrValN != ValuesV.Len(); ++AttrValN) {
					// printf("%s\t", ValuesV.GetVal(AttrValN).CStr());
					AttrsHashV.GetVal(AttrN).AddDat(ValuesV.GetVal(AttrValN), AttrValN);
					InvAttrsHashV.GetVal(AttrN).AddDat(AttrValN, ValuesV.GetVal(AttrValN));
				}
				// printf("\n");
			} else if (AttrVal->GetObjStr("type") == "numeric") {
				// printf("\tNumeric attribute");
				AttrsHashV.GetVal(AttrN).AddDat("", 0);
				InvAttrsHashV.GetVal(AttrN).AddDat(0, "");
			} else {
				FailR(TStr::Fmt("Attribute '%s': Each attribute 'type' is either 'discete' or 'numeric'.", AttrNmV.GetVal(AttrN).CStr()).CStr());
			}
		}

		// printf("-----------------------------\n");
		// printf(" -- Done processing config --\n");
		// printf("-----------------------------\n");

		InitAttrMan();
	}
	void THoeffdingTree::SetParams(PJsonVal JsonParams) {
		if (JsonParams->IsObjKey("gracePeriod") && JsonParams->GetObjKey("gracePeriod")->IsNum()) {
			GracePeriod = JsonParams->GetObjInt("gracePeriod");
			// printf("GracePeriod = %d\n", GracePeriod);
		}
		if (JsonParams->IsObjKey("windowSize") && JsonParams->GetObjKey("windowSize")->IsNum()) {
			WindowSize = JsonParams->GetObjInt("windowSize");
			// printf("WindowSize = %d\n", WindowSize);
		}
		if (JsonParams->IsObjKey("driftCheck") && JsonParams->GetObjKey("driftCheck")->IsNum()) {
			DriftCheck = JsonParams->GetObjInt("driftCheck");
			// printf("DriftCheck = %d\n", DriftCheck);
		}
		if (JsonParams->IsObjKey("tieBreaking") && JsonParams->GetObjKey("tieBreaking")->IsNum()) {
			TieBreaking = JsonParams->GetObjNum("tieBreaking");
			// printf("TieBreaking = %f\n", TieBreaking);
		}
		if (JsonParams->IsObjKey("splitConfidence") && JsonParams->GetObjKey("splitConfidence")->IsNum()) {
			SplitConfidence = JsonParams->GetObjNum("splitConfidence");
			// printf("SplitConfidence = %f\n", SplitConfidence);
		}
		if (JsonParams->IsObjKey("conceptDriftP") && JsonParams->GetObjKey("conceptDriftP")->IsBool()) {
			ConceptDriftP = JsonParams->GetObjBool("conceptDriftP");
			// printf("ConceptDriftP = %d\n", ConceptDriftP);
		}
	}
	// Create attribute manager object for each attribute 
	// NOTE: Label is also ``attribute-managed''
	void THoeffdingTree::InitAttrMan() {
		const int AttrsN = AttrsHashV.Len();
		for (int CountN = 0; CountN < AttrsN; ++CountN) {
			if (AttrsHashV.GetVal(CountN).Len() == 1) { // Continuous attributes have, in a sense, a `single' value 
				AttrManV.Add(TAttrMan(AttrsHashV.GetVal(CountN), InvAttrsHashV.GetVal(CountN), CountN, InvLabelH.GetDat(CountN), atCONTINUOUS));
			} else {
				// printf("%s\n", InvAttrsHashV.GetVal(CountN).GetDat(0).CStr());
				AttrManV.Add(TAttrMan(AttrsHashV.GetVal(CountN), InvAttrsHashV.GetVal(CountN), CountN, InvLabelH.GetDat(CountN), atDISCRETE));
			}
		}
		const TAttrType PredType = AttrManV.Last().Type;
		// EAssert(PredType == atDISCRETE);
		if (PredType == atDISCRETE) {
			TaskType = ttCLASSIFICATION;
		} else {
			TaskType = ttREGRESSION;
		}
		Root = TNode::New(LabelH.Len(), TVec<TInt>(), AttrManV, IdGen->GetNextLeafId(), ntROOT); // Initialize the root node 
	}

	// Pre-order depth-first tree traversal 
	void THoeffdingTree::PrintXML(PNode Node, const int& Depth, TFOut& FOut) const {
		TStr Indent("");
		for (int i = 0; i < Depth; ++i) { Indent += "\t"; }
		if (!Node->ChildrenV.Len()) { // Leaf node 
			FOut.PutStr(Indent);
			FOut.PutStrFmtLn("<leaf class=\"%s\"></leaf>", GetMajorityNm(Node).CStr());
			return;
		}
		TStr ValNm;
		const int ChildrenN = Node->ChildrenV.Len();
		for (int ChildN = 0; ChildN < ChildrenN; ++ChildN) {
			FOut.PutStr(Indent);
			if (AttrManV.GetVal(Node->CndAttrIdx).Type == atDISCRETE) {
				ValNm = GetNodeValueNm(Node, ChildN);
			} else {
				ValNm = (ChildN ? ">" : "<=");
				ValNm += TFlt::GetStr(Node->Val);
			}
			FOut.PutStrFmtLn("<node attribute=\"%s\" value=\"%s\">", GetNodeNm(Node).CStr(), ValNm.CStr());
			PrintXML(Node->ChildrenV.GetVal(ChildN), Depth+1, FOut);
			FOut.PutStr(Indent);
			FOut.PutStrLn("</node>");
		}
	}

	void THoeffdingTree::PrintJSON(PNode Node, const int& Depth, TFOut& FOut) const {
		EFailR("JSON export not yet implemented.");
	}

	// Breadh-first tree traversal
	void THoeffdingTree::PrintDOT(PNode Node, TFOut& FOut, const bool& AlternateP) const {
		TQQueue<TPair<PNode, TInt> > Queue;
		int NodeId = 0; // Used to achieve uniqueness 
		Queue.Push(TPair<PNode, TInt>(Node, NodeId));
		if (Node->ChildrenV.Empty()) {
			if (TaskType == ttCLASSIFICATION) {
				FOut.PutStrFmtLn("\t\"%s\";", GetMajorityNm(Node).CStr());
			} else {
				FOut.PutStrFmtLn("\t\"%f\";", Node->Avg);
			}
			return;
		}
		while (!Queue.Empty()) {
			TPair<PNode, TInt> CurrPair = Queue.Top();
			PNode CrrNode = CurrPair.Val1;
			Queue.Pop();
			TStr ValueNm;
			for (int NodeN = 0; NodeN < CrrNode->ChildrenV.Len(); ++NodeN) {
				++NodeId;
				PNode TmpNode = CrrNode->ChildrenV[NodeN];
				if (AttrManV.GetVal(CrrNode->CndAttrIdx).Type == atDISCRETE) {
					ValueNm = GetNodeValueNm(CrrNode, NodeN);
				} else {
					ValueNm = NodeN == 0 ? "<= " : "> ";
					ValueNm += TFlt::GetStr(CrrNode->Val);
				}
				TStr TmpValueNm = ValueNm;
				TmpValueNm.ChangeCh('-', 'D'); // Make sure DOT doesn't crash later beacuse of illegal labels 
				if (TmpNode->ChildrenV.Empty()) {
					if (TaskType == ttCLASSIFICATION) {
						FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, GetMajorityNm(TmpNode).CStr(), NodeId, TmpValueNm.CStr());
					} else {
						FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, TFlt::GetStr(TmpNode->Avg).CStr(), NodeId, TmpValueNm.CStr());
					}
				} else {
					FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, GetNodeNm(TmpNode).CStr(), NodeId, TmpValueNm.CStr());
					Queue.Push(TPair<PNode, TInt>(TmpNode, NodeId));
				}
			}
			// Draw alternate trees 
			for (int TreeN = 0; AlternateP && TreeN < CrrNode->AltTreesV.Len(); ++TreeN) {
				++NodeId;
				PNode TmpNode = CrrNode->AltTreesV[TreeN];
				ValueNm = "*"; // Marks alternate tree 
				if (TmpNode->ChildrenV.Empty()) {
					if (TaskType == ttCLASSIFICATION) {
						FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\",style=\"dotted\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, GetMajorityNm(TmpNode).CStr(), NodeId, ValueNm.CStr());
					} else {
						FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\",style=\"dotted\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, TFlt::GetStr(TmpNode->Avg).CStr(), NodeId, ValueNm.CStr());
					}
				} else {
					FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\",style=\"dotted\"];", GetNodeNm(CrrNode).CStr(), CurrPair.Val2, GetNodeNm(TmpNode).CStr(), NodeId, ValueNm.CStr());
					Queue.Push(TPair<PNode, TInt>(TmpNode, NodeId));
				}
			}
		}
	}

	void THoeffdingTree::PrintHist(const TStr& FNm, const TCh& Ch) const {
		TFOut FOut(FNm);
		TFOut FVec(FNm+".vec");
		double SplitVal;
		// Find the first numeric attribute if any 
		// for(TAttrManV::TIter It = AttrManV.BegI(); It != AttrManV.EndI(); ++It) {
		TAttrManV::TIter It = AttrManV.BegI(); ++It; ++It;
			if (It->Type == atCONTINUOUS) {
				const int AttrN = It->Id;
				//printf("Numeric distribution for '%s' attribute.\n", It->Nm.CStr());
				Root->HistH.GetDat(AttrN).InfoGain(SplitVal);
				const TBinV BinV = Root->HistH.GetDat(AttrN).BinsV;
				for (TBinV::TIter It = BinV.BegI(); It != BinV.EndI(); ++It) {
					for (int TmpN = 0; TmpN < It->Count/10; ++TmpN) {
						FOut.PutCh(Ch);
						// printf("#");
					}
					// FVec.PutUInt(It->Count);
					FVec.PutFlt(It->Entropy());
					FVec.PutCh(' ');
					FOut.PutLn();
					// printf("\n");
				}
				// printf("\n");
				// break;
			}
		// }
	}
	void THoeffdingTree::Print(PExample Example) const {
		for (auto It = Example->AttributesV.BegI(); It != Example->AttributesV.EndI(); ++It) {
			switch (AttrManV.GetVal(It->Id).Type) {
				case atCONTINUOUS: {
					printf("%f\t", It->Num.Val);
					break;
				}
				case atDISCRETE: {
					printf("%s\t", AttrManV.GetVal(It->Id).InvAttrH.GetDat(It->Value).CStr());
					break;
				}
			}
		}
		putchar('\n');
	}
	void THoeffdingTree::Print(const TCh& Ch, const TInt& Num) {
		for (int ChN = 0; ChN < Num; ++ChN) {
			printf("%c", Ch.Val);
		}
		printf("\n");
	}

	// Naive bayes classifier 
	TLabel THoeffdingTree::NaiveBayes(PNode Node, PExample Example) const {
		const THash<TTriple<TInt, TInt, TInt>, TInt> Counts = Node->Counts;
		const TIntV PartitionV = Node->PartitionV;
		const int ExamplesN = Node->ExamplesN;
		const int AttrsN = Example->AttributesV.Len();
		const int LabelsN = AttrManV.Last().ValueV.Len();
		double pk = 0.0, pc = 0.0;
		int nk = 0;// Maj = 0;
		int MxLabel = 0;
		double MxProb = 0;
		TVec<TInt> SubExamplesN;
		SubExamplesN.Reserve(AttrsN, AttrsN);
		// Precompute n(x_k) for k=0,1,...,AttrsN-1
		for (int AttrN = 0; AttrN < AttrsN; ++AttrN) {
			SubExamplesN.GetVal(AttrN) = 0;
			for (int LabelN = 0; LabelN < LabelsN; ++LabelN) {
				TTriple<TInt, TInt, TInt> TmpTriple(AttrN, Example->AttributesV.GetVal(AttrN).Value, LabelN);
				if (Counts.IsKey(TmpTriple) && Counts.GetDat(TmpTriple) > 0) {
					SubExamplesN.GetVal(AttrN) += Counts.GetDat(TmpTriple);
				}
			}
		}
		for (int LabelN = 0; LabelN < LabelsN; ++LabelN) {
			nk = PartitionV.GetVal(LabelN); // number of positive examples 
			//printf("[DEBUG] #Examples = %d\n", nk);
			pk = (nk+1.0)/(ExamplesN+LabelsN); // TProbEstimates::LaplaceEstiamte(nk, CurrNode->ExamplesN-nk, 2); 
			//printf("[DEBUG] Current: %f\n", pk);
			for (int i = 0; i < AttrsN; ++i) {
				TTriple<TInt, TInt, TInt> TmpTriple(i, Example->AttributesV.GetVal(i).Value, LabelN);
				if (Counts.IsKey(TmpTriple) && Counts.GetDat(TmpTriple) > 0) {
					// p0 = 1.0*CurrNode->Counts(TmpTriple)/nk; // apriori probability 
					// pk *= TProbEstimates::MEstimate(CurrNode->Counts(TmpTriple), nk, p0, 2); // compute conditional probability using m-estimate 
					// pk *= 1.0*CurrNode->Counts(TmpTriple)/nk; 
					// (m * P(c_i) + n(x_k,c_i))/(P(c_i) * (m + n(x_k)))
					pc = (nk+1.0)/(ExamplesN+LabelsN); // laplace estimate for P(c_i) 
					pk *= (2.0*pc+Counts.GetDat(TmpTriple))/(pc*(2+SubExamplesN.GetVal(i)));
				}/* else {
					printf("[DEBUG] Zero probability!\n");
				}*/
			}
			if (MxProb < pk) {
				MxProb = pk; MxLabel = LabelN;
			}
			//printf("[DEBUG] Class `%s' with `probability' %f\n", InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().ValueV.GetVal(LabelN)).CStr(), pk);
		}
		// Print('-', 120);
		// printf("-----------------------\n");
		// printf("[DEBUG] Bayes classified it as %s\n", InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().ValueV.GetVal(MxLabel)).CStr());
		/* ----------- */
		// printf("[DEBUG] CLASSIFYING:\n");
		// for(int i = 0; i < PartitionV.Len(); ++i) {
		//	printf("\t[DEBUG] #%s = %d\n", InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().ValueV.GetVal(i)).CStr(), PartitionV.GetVal(i));
		// }
		return MxLabel;
	}
}

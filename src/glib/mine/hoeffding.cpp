namespace THoeffding {
   ///////////////////////////////
   // constant-definitions
   // const int BinsN;

   ///////////////////////////////
   // Simple-lexical-analyzer
   TToken TLexer::GetNextTok() {
      if (BackP) {
         BackP = false;
         return LastTok;
      }
      CurrCh = SIn->GetCh();
      // Skip whitespace 
      EatWs();
      // Ignore comments 
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
      // Identifier?
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
            // printf("[Line %d] Illegal character: '%c'.\n", LineN, CurrCh);
            EFailR("Illegal character.");
         }
         LastTok = TToken("", totEND, LineN); // End-of-file
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
   // Parameters
   
   ///////////////////////////////
   // Parser
   void TParser::CfgParse(const TStr& FileNm) {
      TLexer Lexer(FileNm);
      TToken Tok;
      // Parsing format specification
      InitLine(Lexer);
      // Parsing attributes 
      AttrLine(Lexer);
   }
   void TParser::InitLine(TLexer& Lexer) {
      TToken Tok;
      Tok = Lexer.GetNextTok();
      if (Tok.Type != totDFORMAT) {
         // printf("[Line %d] Expected 'dataFormat' keyword instead 
         // of '%s'.\n", Tok.LineN, Tok.Val.CStr());
         EFailR("Expected 'dataFormat'.");
      }
      // =
      Tok = Lexer.GetNextTok();
      if (Tok.Type != totCOLON) {
         // printf("[Line %d] Expected ':' instead of '%s'.\n",
         //   Tok.LineN, Tok.Val.CStr());
         EFailR("Expected ':'.");
      }
      // (
      Tok = Lexer.GetNextTok();
      if (Tok.Type != totLPARENTHESIS) {
         // printf("[Line %d] Expected '(' instead of '%s'.\n",
         //   Tok.LineN, Tok.Val.CStr());
         EFailR("Expected '('.");
      }
      // Parameter list 
      InitParam(Lexer);
      // )
      Tok = Lexer.GetNextTok();
      if (Tok.Type != totRPARENTHESIS) {
         // printf("[Line %d] Expected ')' instead of '%s'.\n",
         //   Tok.LineN, Tok.Val.CStr());
         EFailR("Expected ')'.");
      }
   }
   void TParser::InitParam(TLexer& Lexer) {
      TToken Tok;
      int IdxN = 0;
      while (true) {
         // Identifier 
         Tok = Lexer.GetNextTok();
         if (Tok.Type != totID) {
            // printf("[Line %d] Expected identifier instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected identifier.");
         }
         printf("Declaring '%s'...\n", Tok.Val.CStr());
         DataFormatH.AddDat(Tok.Val, IdxN);
         InvDataFormatH.AddDat(IdxN, Tok.Val);
         // ,
         Tok = Lexer.GetNextTok();
         // End of parameter list 
         if (Tok.Type == totRPARENTHESIS) { break; }
         if (Tok.Type != totCOMMA) {
            // printf("[Line %d] Expected ',' instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected ','.");
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
         // Identifier 
         Tok = Lexer.GetNextTok();
         // End-of-file
         if (Tok.Type == totEND) { break; }
            
         if (Tok.Type != totID) {
            // printf("[Line %d] Expected identifier instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected identifier.");
         }
         AttrNm = Tok.Val;
         // Make sure attribute was ``declared'' in dataFormat statement 
         if (!DataFormatH.IsKey(AttrNm)) {
            // printf("Attribute '%s' is undeclared.\n", AttrNm.CStr());
            EFailR("Undeclared attribute (i.e., attribute not mentioned \
               in dataFormat).");
         }
         // :
         Tok = Lexer.GetNextTok();
         if (Tok.Type != totCOLON) {
            // printf("[Line %d] Expected ':' instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected ':'.");
         }
         // Attribute type (discrete or numeric)
         Tok = Lexer.GetNextTok();
         if (Tok.Type == totDISCRETE) {
            printf("Nominal attirubte %s\n", AttrNm.CStr());
            // (
            Tok = Lexer.GetNextTok();
            if (Tok.Type != totLPARENTHESIS) {
               // printf("[Line %d] Expected '(' instead of '%s'.\n",
               //   Tok.LineN, Tok.Val.CStr());
               EFailR("Expected '('.");
            }
            AttrParam(Lexer, AttrNm);
            // )
            Tok = Lexer.GetNextTok();
            if (Tok.Type != totRPARENTHESIS) {
               // printf("[Line %d] Expected ')' instead of '%s'.\n",
               //   Tok.LineN, Tok.Val.CStr());
               EFailR("Expected ')'.");
            }
         } else if (Tok.Type == totNUMERIC) {
            // Numeric atribute 
            const int CountN = DataFormatH.GetDat(AttrNm);
            AttrsHV.GetVal(CountN).AddDat("", 0);
            InvAttrsHV.GetVal(CountN).AddDat(0, "");
         } else {
            // printf("[Line %d] Expected attribute (discrete or numeric) type
            //   instead of '%s'.\n", Tok.LineN, Tok.Val.CStr());
            EFailR("Expected attribute type ('discrete' or 'numeric').");
         }
      }
   }
   void TParser::AttrParam(TLexer& Lexer, const TStr& AttrNm) {
      int IdxN = 0;
      const int CountN = DataFormatH.GetDat(AttrNm);
      TToken Tok;
      TStr ValNm;
      while (true) { // Loop through all values 
         // Identifier 
         Tok = Lexer.GetNextTok();
         if (Tok.Type != totID) {
            // printf("[Line %d] Expected identifier instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected identifier");
         }
         ValNm = Tok.Val;
         // Set up the mappings 
         AttrsHV.GetVal(CountN).AddDat(ValNm, IdxN);
         InvAttrsHV.GetVal(CountN).AddDat(IdxN, ValNm);
         // ,
         Tok = Lexer.GetNextTok();
         if (Tok.Type == totRPARENTHESIS) { break; } // end of parameter list 
         if (Tok.Type != totCOMMA) {
            // printf("[Line %d] Expected ',' instead of '%s'.\n",
            //   Tok.LineN, Tok.Val.CStr());
            EFailR("Expected ','.");
         }
         ++IdxN;
      }
      Lexer.PutBack();
   }
   void TParser::Error(const TStr& Msg) {
      throw TExcept::New("Parsing error.");
   }
   
   ///////////////////////////////
   // Node-and-Bin-ID-Generator
   
   ///////////////////////////////
   // Probability-Estimation-Functions
   
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
      // Make sure frequencies add up to N 
      int FreqSum = 0;
      for (auto It = FreqV.BegI(); It != FreqV.EndI(); ++It) {
         EAssertR(It->Val <= N, "Frequencey counts don't add up (Val>N).");
         FreqSum += It->Val;
         p = N > 0 ? 1.0*It->Val/N : 0.0;
         if (p > 0) { h -= p*TMath::Log2(p); }
      }
      EAssertR(FreqSum == N, "Frequency counts don't add up.");
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
   // NOTE: This function must ensure that Example->BinId is set to the 
   // maximum ID of the bins containing the example 
   void THist::IncCls(PExample Example, const int& AttrIdx, PIdGen IdGen) {
      EAssertR(AttrIdx >= 0 && AttrIdx < Example->AttributesV.Len(),
         "Index out of bounds.");
      int Idx = 0, BinN = 0;
      double CurrDist = 0.0, PrevDist = 0.0;
      const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
      const int Label = Example->Label;
      // Add new bin, initialized with Val, if the number of bins (BinsN)
      // didn't reach the treshold 
      if ((Idx = BinsV.SearchBin(Val)) == -1 && BinsV.Len() < BinsN) {
         const int CrrBinId = IdGen->GetNextBinId();
         const int CrrIdx = BinsV.AddSorted(TBin(Val, CrrBinId), true);
         BinsV.GetVal(CrrIdx).Inc(Label);
         EAssertR(Example->BinId < CrrBinId,
            "Example->BinId >= CrrBinId for a newly generated bin ID.");
         Example->SetBinId(TMath::Mx<int>(Example->BinId, CrrBinId));
      } else { // Find the closest bin 
         if (Idx != -1) { // Bin initialized with exactly this value 
            BinsV.GetVal(Idx).Inc(Label);
            Example->SetBinId(TMath::Mx<int>(Example->BinId,
               BinsV.GetVal(Idx).Id));
         } else { // Otherwise, increment the closest bin 
            Idx = 0;
            PrevDist = CurrDist = abs(Val - BinsV.GetVal(0).GetVal());
            // NOTE: We could use binary search because of the ordering
            // invariant; but the number of bins rarely exeecds 100 
            // While distance starts increasing, stop --- our bin is the one
            // before the current one 
            for (BinN = 1; BinN < BinsV.Len(); ++BinN) {
               EAssertR(BinsV.GetVal(BinN-1).GetVal() <=
                  BinsV.GetVal(BinN).GetVal(), "Bins not sorted.");
               PrevDist = CurrDist;
               CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal());
               if (CurrDist > PrevDist) {
                  Idx = BinN-1; break;
               }
            }
            if (BinN == BinsV.Len()) { Idx = BinN-1; }
            BinsV.GetVal(Idx).Inc(Label);
            Example->SetBinId(TMath::Mx<int>(
               Example->BinId, BinsV.GetVal(Idx).Id));
         }
      }
   }
   // NOTE: This function must ensure the example is removed from
   // the bins that existed at the time of its arrival 
   void THist::DecCls(PExample Example, const int& AttrIdx) {
      EAssertR(AttrIdx >= 0 && AttrIdx < Example->AttributesV.Len(),
         "Index out of bounds.");
      int Idx = 0, BinN = 0, PrevIdx = 0;
      double CurrDist = 0.0, PrevDist = 0.0;
      const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
      const int Label = Example->Label;
      // Idx = BinsV.SearchBin(Val); // Binary search for Val 
      Idx = BinsV.SearchBin(Val);
      EAssertR(Idx != -1 || BinsN <= BinsV.Len(),
         "By construction, the value cannot be missing.");
      // Find the closest bin 
      // Bin initialized with this very value 
      if (Idx != -1 && BinsV.GetVal(Idx).Id <= Example->BinId) {
         BinsV.GetVal(Idx).Dec(Label);
      } else {
         // Otherwise, decrement the closest bin that created 
         // before or at the same time as the example was accumulated 
         Idx = 0;
         // NOTE: We can't take the first bin as it may have been
         // created AFTER the example was accumulated; instead we find
         // the first suitable bin 
         EAssertR(BinsV.Len() == BinsN,
            "Expected histogram to be filled with bins.");
         // Find the first suitable bin 
         for (BinN = 0; BinN < BinsV.Len() &&
            BinsV.GetVal(BinN).Id > Example->BinId; ++BinN);
         // NOTE: For debugging purposes 
         EAssertR(BinN < BinsV.Len(), "No suitable bin --- impossible.");
         PrevIdx = Idx = BinN; // First suitable bin 
         PrevDist = CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal());
         // The order is preserved even though new bins might have been
         // created between the old ones 
         for (; BinN < BinsV.Len(); ++BinN) {
            if (BinsV.GetVal(BinN).Id <= Example->BinId) {
               PrevDist = CurrDist;
               CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal());
               if (CurrDist > PrevDist) {
                  Idx = PrevIdx; break;
               } else { PrevIdx = BinN; }
            }
         }
         if (BinN == BinsV.Len() && Idx != PrevIdx) { Idx = PrevIdx; }
         BinsV.GetVal(Idx).Dec(Label);
      }
   }
   void THist::IncReg(const PExample Example, const int& AttrIdx) {
      int Idx = 0, BinN = 0;
      double CurrDist = 0.0, PrevDist = 0.0;
      // Numeric attribute value 
      const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
      const double RegValue = Example->Value; // Value of the target variable 
      // We could use `BinsV.SearchBin(Val)) == -1` -- binary search 
      if (BinsV.Len() < BinsN && (Idx = BinsV.SearchForw(Val, 0)) == -1) {
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
               // We are fine, because bins are ordered inside the vector
               // by the initialization values 
               CurrDist = abs(Val - BinsV.GetVal(BinN).GetVal());
               if (CurrDist > PrevDist) {
                  Idx = BinN - 1;
                  break;
               }
            }
            if (BinN == BinsV.Len()) { Idx = BinN-1; }
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
         // h_i := H(B_1\cup B_2\cup \ldots\cup B_i)
         GArr[BinN] = TMisc::Entropy(HiV, HiCount);
         NArr[BinN] = HiCount; // n_i := |B_1|+\ldots+|B_i|
      }
      const int AllN = HiCount;
      const double H = TMisc::Entropy(HiV, AllN);
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
         CurrGain = H - LoCount*LoImp/AllN - HiCount*HiImp/AllN;
         if (CurrGain > MxGain) {
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
   // See [Knuth, 1997] and [Chan et al., 1979] for details regarding 
   // updating formulas for variance 
   // (http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance,
   // accessed on 7 Jun 2013)
   double THist::StdGain(double& SpltVal) const { // for regression 
      int HiCnt, LoCnt, CrrCnt;
      int MxIdx;
      double MxGain, CrrGain;
      double LoS, HiS, LoT, HiT;
      // Define VarArr[i] := n*Var(B_1\cup B_2\cup ...\cup B_i) 
      double* SArr = new double[BinsN]();
      // Define AvgArr[i] := x_1+x_2+...+x_i 
      double* TArr = new double[BinsN]();
      // Compute initial split 
      LoCnt = HiCnt = 0; // BinsV.GetVal(0).Count;
      SArr[0] = TArr[0] = 0.0;
      for (int BinN = 0; BinN < BinsV.Len(); ++BinN) {
         const TBin CrrBin = BinsV.GetVal(BinN);
         const double PrevS = BinN > 0 ? SArr[BinN-1] : 0.0;
         const double PrevT = BinN > 0 ? TArr[BinN-1] : 0.0;
         TArr[BinN] = PrevT + CrrBin.T;
         CrrCnt = BinsV.GetVal(BinN).Count;
         SArr[BinN] = PrevS+CrrBin.S;
         if (CrrCnt > 0 && HiCnt > 0) {
            SArr[BinN] += TMath::Sqr(
               CrrCnt*PrevT/HiCnt-CrrBin.T)*1.0*HiCnt/(CrrCnt*(CrrCnt+HiCnt));
         }
         HiCnt += CrrCnt;
      }
      const int AllN = HiCnt;
      const double S = SArr[BinsV.Len()-1];
      HiS = CrrGain = MxGain = 0.0;
      MxIdx = 0;
      LoS = BinsV.Last().S;
      LoT = BinsV.Last().T;
      LoCnt = BinsV.Last().Count;
      HiCnt -= LoCnt;
      // Compute expected variance reduction, as defined
      // in [Ikonomovska, 2012] and [Ikonomovska et al., 2011] 
      for (int BinN = BinsV.Len()-2; BinN >= 0; --BinN) {
         HiS = SArr[BinN];
         HiT = TArr[BinN];
         const double SigmaS = TMath::Sqrt(S/(AllN));
         const double SigmaS1 = TMath::Sqrt(LoS/(LoCnt));
         const double SigmaS2 = TMath::Sqrt(HiS/(HiCnt));
         CrrGain = SigmaS - LoCnt*SigmaS1/AllN - HiCnt*SigmaS2/AllN;
         if (CrrGain > MxGain) {
            MxGain = CrrGain;
            MxIdx = BinN;
         }
         // Update variance 
         const double CrrS = BinsV.GetVal(BinN).S; // S_2
         const double CrrT = BinsV.GetVal(BinN).T;
         CrrCnt = BinsV.GetVal(BinN).Count; // n
         LoS += CrrS;
         if (LoCnt > 0 && CrrCnt > 0) {
            LoS += TMath::Sqr(
               CrrCnt*LoT/LoCnt-CrrT)*LoCnt/(CrrCnt*(CrrCnt+LoCnt));
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
   // Attribute-Managment
   TAttrMan::TAttrMan(const THash<TStr, TInt>& AttrH_,
      const THash<TInt, TStr>& InvAttrH_, const int& Id_, const TStr& Nm_,
      const TAttrType& Type_)
      : AttrH(AttrH_), InvAttrH(InvAttrH_), Type(Type_), Nm(Nm_), Id(Id_) {
         // Possible values; there is a single value for numeric attributes 
         AttrH.GetDatV(ValueV);
   }
   
   ///////////////////////////////
   // Attribute
   
   ///////////////////////////////
   // Example
   TExample& TExample::operator=(const TExample& Example) {
      if (this != &Example) {
         LeafId = Example.LeafId;
         BinId = Example.BinId;
         Id = Example.Id;
         AttributesV = Example.AttributesV;
         Label = Example.Label;
         Value = Example.Value;
      }
      return *this;
   }

   /////////////////////////////////
   // Node
   // Copy constructor 
   TNode::TNode(const TNode& Node)
      : CndAttrIdx(Node.CndAttrIdx), ExamplesN(Node.ExamplesN), Val(Node.Val),
         Avg(Node.Avg), VarSum(Node.VarSum), Err(Node.Err),
         TestModeN(Node.TestModeN), Type(Node.Type), ExamplesV(Node.ExamplesV),
         PartitionV(Node.PartitionV), UsedAttrs(Node.UsedAttrs),
         HistH(Node.HistH), Id(Node.Id), Correct(Node.Correct), All(Node.All)
   { }
   // Assignment operator 
   TNode& TNode::operator=(const TNode& Node) {
      if (this != &Node) {
         //Clr(); // Delete old elements 
         All = Node.All; AltTreesV = Node.AltTreesV;
         Avg = Node.Avg; ChildrenV = Node.ChildrenV;
         CndAttrIdx = Node.CndAttrIdx; Correct = Node.Correct;
         Counts = Node.Counts; Err = Node.Err; ExamplesN = Node.ExamplesN;
         ExamplesV = Node.ExamplesV; HistH = Node.HistH; Id = Node.Id;
         PartitionV = Node.PartitionV;
         SeenH = Node.SeenH; 
         TestModeN = Node.TestModeN; Type = Node.Type;
         Val = Node.Val; VarSum = Node.VarSum;
      }
      return *this;
   }
   bool TNode::operator==(const TNode& Node) const {
      return CndAttrIdx == Node.CndAttrIdx && Type == Node.Type &&
         ExamplesV == Node.ExamplesV && Counts == Node.Counts &&
         PartitionV == Node.PartitionV && Id == Node.Id &&
         ChildrenV == Node.ChildrenV && UsedAttrs == Node.UsedAttrs;
   }
   // Training set entropy 
   double TNode::ComputeEntropy() const {
      return TMisc::Entropy(PartitionV, ExamplesN);
   }
   double TNode::ComputeGini() const {
      double g = 1.0, p = 0.0;
      for (auto It = PartitionV.BegI(); It != PartitionV.EndI(); It++) {
         p = 1.0*(*It)/ExamplesN;
         g -= p*p;
      }
      return g;
   }
   // Compute information gain from sufficient statistics 
   double TNode::InfoGain(const int& AttrIdx,
      const TAttrManV& AttrManV) const {
      double h = 0.0, hj = 0.0, p = 0.0, pj = 0.0;
      // Number of examples x with A(x)=a_j for j=1,2,...,ValsN
      int SubExamplesN = 0;
      const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
      TAttrMan AttrMan(AttrManV.GetVal(AttrIdx));
      const int ValsN = AttrMan.ValueV.Len();
      // Compute entropy H(E) 
      h = TMisc::Entropy(PartitionV, ExamplesN);
      // Compute information gain 
      for (int j = 0; j < ValsN; ++j) {
         SubExamplesN = 0;
         // Compute |E_j|
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if(Counts.IsKey(TmpTriple)) {
               SubExamplesN += Counts.GetDat(TmpTriple);
            }
         }
         hj = 0.0;
         // Compute H(E_j)
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if (Counts.IsKey(TmpTriple)) {
               // Prevent divison by zero 
               pj = SubExamplesN > 0 ?
                  1.0*Counts.GetDat(TmpTriple)/SubExamplesN : 0.0;
               if (pj > 0) { // Ensure Log2(pj) exists 
                  hj -= pj*TMath::Log2(pj);
               }
            }
         }
         p = ExamplesN > 0 ? 1.0*SubExamplesN/ExamplesN : 0.0;
         h -= p*hj;
      }
      // Return information gain G(A) 
      return h;
   }
   // Compute Gini index from sufficient statistics 
   double TNode::GiniGain(const int& AttrIdx,
      const TVec<TAttrMan>& AttrManV) const {
      double g = 1.0, gj = 0.0, p = 0, pj = 0;
      // Number of examples x with A(x)=a_j for j=1,2,...,ValsN 
      int SubExamplesN = 0;
      const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
      TAttrMan AttrMan(AttrManV.GetVal(AttrIdx));
      const int ValsN = AttrMan.ValueV.Len();
      for (auto It = PartitionV.BegI(); It != PartitionV.EndI(); ++It) {
         // Prevent division by zero 
         p = ExamplesN > 0 ? 1.0*(*It)/ExamplesN : 0;
         g -= p*p;
      }
      for (int j = 0; j < ValsN; ++j) {
         SubExamplesN = 0;
         // Compute |E_j|
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if (Counts.IsKey(TmpTriple)) {
               SubExamplesN += Counts.GetDat(TmpTriple);
            }
         }
         gj = 1.0;
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if (Counts.IsKey(TmpTriple)) {
               // Prevent divison by zero 
               pj = SubExamplesN > 0 ?
                  1.0*Counts.GetDat(TmpTriple)/SubExamplesN : 0.0;
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
      // NOTE: Compute variances Var(S_i) for all possible values
      // attribute A_i can take 
      const TAttrType AttrType = AttrManV.GetVal(AttrIdx).Type;
      EAssertR(AttrType == atDISCRETE, "This function works with discrete \
         attributes.");
      const int ValsN = AttrManV.GetVal(AttrIdx).ValueV.Len();
      // Vector of (mean, variance, n) pairs 
      TVec<TTriple<TFlt, TFlt, TInt> > VarV;
      // TODO: Avoid iterating over the vector twice --- is there a faster 
      // way to initialize the thing?
      for (int ValN = 0; ValN < ValsN; ++ValN) {
         VarV.Add(TTriple<TFlt, TFlt, TInt>(0.0, 0.0, 0));
      }
      // Incrementally compute variances 
      for (int ValN = 0; ValN < ExamplesV.Len(); ++ValN) {
         const int CrrIdx =
            ExamplesV.GetVal(ValN)->AttributesV.GetVal(AttrIdx).Value;
         const double CrrVal = ExamplesV.GetVal(ValN)->Value;
         TTriple<TFlt, TFlt, TInt>& CrrTriple = VarV.GetVal(CrrIdx);
         // See [Knuth, 1997] for details regarding incremental algorithms 
         // for variance 
         // This way it is always the case that N>0 
         const int N = ++CrrTriple.Val3;
         const double Delta = CrrVal - CrrTriple.Val1;
         CrrTriple.Val1 += Delta/N;
         CrrTriple.Val2 += Delta*(CrrVal - CrrTriple.Val1);
      }
      double CrrStd = Std();
      for (int ValN = 0; ValN < ValsN; ++ValN) {
         const int CrrN = VarV.GetVal(ValN).Val3;
         if (CrrN > 1) {
            const double TmpStd = TMath::Sqrt(VarV.GetVal(ValN).Val2/(CrrN-1));
            CrrStd -= CrrN*TmpStd/ExamplesN;
         }
      }
      return CrrStd;
   }
   // See [Domingos and Hulten, 2000] and [Hulten et al., 2001] for explanation
   double TNode::ComputeTreshold(const double& Delta,
      const int& LabelsN) const {
      // Range of the random variable for information gain 
      const double R = TMath::Log2(LabelsN);
      EAssertR(ExamplesN > 0, "This node has no examples.\n");
      // t = \sqrt{ \frac{R^2 * log(1/delta)}{2n} }
      return TMath::Sqrt(R*R*TMath::Log(1.0/Delta)/(2.0*ExamplesN));
   }
   void TNode::Split(const int& AttrIdx, const TAttrManV& AttrManV,
      PIdGen IdGen) {
      // (i) Mark attribute, if discrete, as used
      // New child for each value of AttrIdx attribute 
      CndAttrIdx = AttrIdx;
      const TAttrType AttrType = AttrManV.GetVal(AttrIdx).Type;
      int ValsN = AttrManV.GetVal(AttrIdx).ValueV.Len();
      // Categorial attributes can only be used once 
      switch(AttrType) {
      case atDISCRETE:
         UsedAttrs.Add(AttrIdx);
         break;
      case atCONTINUOUS:
         ValsN = 2;
         break;
         // printf("[DEBUG] Splitting on continuous value %f\n", Val);
      default:
         EFailR("Invalid attribute type AttrType");
      }
      // UsedAttrs.Add(CondAttrIndex);
      const int LabelsN = AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
      for (int ValN = 0; ValN < ValsN; ++ValN) {
          // Leaf node 
         ChildrenV.Add(TNode::New(LabelsN, UsedAttrs, AttrManV,
            IdGen->GetNextLeafId())); 
      }
      if (Type != ntROOT) { Type = ntINTERNAL; }
   }
   void TNode::Clr() { // Forget training examples 
      ExamplesV.Clr(); PartitionV.Clr(); Counts.Clr();
      HistH.Clr(true); AltTreesV.Clr(); UsedAttrs.Clr();
      SeenH.Clr(true);
   }
   TBstAttr TNode::BestAttr(const TAttrManV& AttrManV,
      const TTaskType& TaskType) {
      EAssertR(TaskType == ttCLASSIFICATION || TaskType == ttREGRESSION,
         "Invalid task type TaskType.");
      if (TaskType == ttCLASSIFICATION) {
         return BestClsAttr(AttrManV);
      } else {
         return BestRegAttr(AttrManV);
      }
   }
   // Regression 
   TBstAttr TNode::BestRegAttr(const TAttrManV& AttrManV) { 
      // AttrsManV includes attribute manager for the label 
      const int AttrsN = AttrManV.Len()-1; 
      double CrrSdr, Mx1, Mx2;
      int Idx1, Idx2;
      CrrSdr = Mx1 = Mx2 = 0;
      Idx1 = Idx2 = 0;
      for (int AttrN = 0; AttrN < AttrsN; ++AttrN) {
         const TAttrType AttrType = AttrManV.GetVal(AttrN).Type;
         EAssertR(AttrType == atDISCRETE || AttrType == atCONTINUOUS,
            "Invalid attribute type AttrType.");
         if (AttrType == atDISCRETE) { // Discrete 
            if (UsedAttrs.SearchForw(AttrN, 0) < 0) {
               // Compute standard deviation reduction 
               CrrSdr = StdGain(AttrN, AttrManV);
               // printf("CrrStd(A%d) = %f\n", AttrN, CrrSdr);
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
      // printf("Mx1 = %f ; Mx2 = %f\n", Mx1, Mx2);
      // If Mx1==0.0, then Mx2==0.0, because we have 0.0<=Mx2<=Mx1 
      const double Ratio = Mx1 > 0.0 ? Mx2/Mx1 : 1.0;
      return TBstAttr(TPair<TInt, TFlt>(Idx1, Mx1),
         TPair<TInt, TFlt>(Idx2, Mx2), Ratio);
   }
   // Classification 
   TBstAttr TNode::BestClsAttr(const TAttrManV& AttrManV,
      const TIntV& BannedAttrV) {
      int Idx1, Idx2;
      double Mx1, Mx2, Crr, SplitVal;
      const int AttrsN = AttrManV.Len()-1;
      Crr = Mx1 = Mx2 = 0;
      Idx1 = Idx2 = -1;
      for (int AttrN = 0; AttrN < AttrsN; ++AttrN) {
         // NOTE: BannedAttrV almost never contains more than two indices 
         if (BannedAttrV.IsIn(AttrN)) { continue; }
         TAttrType AttrType = AttrManV.GetVal(AttrN).Type;
         EAssertR(AttrType == atDISCRETE || AttrType == atCONTINUOUS,
            "Invalid attribute type AttrType.");
         if (AttrType == atDISCRETE) {
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
      return TBstAttr(TPair<TInt, TFlt>(Idx1, Mx1),
         TPair<TInt, TFlt>(Idx2, Mx2), Diff);
   }
   // See page 232 of Knuth's TAOCP, Vol. 2: Seminumeric Algorithms, 1997
   // for details 
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
   double THoeffdingTree::Predict(const TStrV& DiscreteV,
      const TFltV& NumericV) const {
      int DisIdx = 0, FltIdx = 0;
      TAttributeV AttributesV;
      const int AttrsN = AttrManV.Len();
      for (int AttrN = 0; AttrN < AttrsN-1; ++AttrN) {
         switch (AttrManV.GetVal(AttrN).Type) {
         case atDISCRETE:
            // printf(DiscreteV.GetVal(DisIdx).CStr());
            AttributesV.Add(TAttribute(AttrN,
               AttrsHashV.GetVal(AttrN).GetDat(DiscreteV.GetVal(DisIdx++))));
            break;
         case atCONTINUOUS:
            AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
            break;
         default:
            EFailR("Unsupported attribute type");
         }
      }
      EAssertR(AttrManV.Last().Type == atCONTINUOUS,
         "This function works only for regression.");
      return Predict(TExample::New(AttributesV, 0.0));
   }
   // Regression
   double THoeffdingTree::Predict(PExample Example) const {
      PNode CrrNode = Root;
      while (CrrNode->CndAttrIdx != -1) {
         const TAttrType AttrType = AttrManV.GetVal(CrrNode->CndAttrIdx).Type;
         EAssertR(AttrType == atDISCRETE || AttrType == atCONTINUOUS,
            "Invalid attribute type AttrType.");
         if (AttrType == atDISCRETE) {
            CrrNode = CrrNode->ChildrenV.GetVal(Example->AttributesV.GetVal(
               CrrNode->CndAttrIdx).Value);
         } else { // Numeric attribute 
            const double Val = Example->AttributesV.GetVal(
               CrrNode->CndAttrIdx).Num;
            const int Idx = Val <= CrrNode->Val;
            CrrNode = CrrNode->ChildrenV.GetVal(Idx);
         }
      }
      // Ikonomovska [Ikonomovska, 2012] trains perceptron in the leaves 
      return CrrNode->Avg;
   }
   // TODO: Let the user decide what classifier to use in the leaves 
   TStr THoeffdingTree::Classify(PNode Node, PExample Example) const {
      PNode CrrNode = Node;
      while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
      return GetMajorityNm(CrrNode); // NaiveBayes(CrrNode, Example);
   }
   TStr THoeffdingTree::Classify(const TStrV& DiscreteV,
      const TFltV& NumericV) const {
      int DisIdx = 0, FltIdx = 0;
      TAttributeV AttributesV;
      const int AttrsN = AttrManV.Len();
      for (int AttrN = 0; AttrN < AttrsN-1; ++AttrN) {
         switch (AttrManV.GetVal(AttrN).Type) {
         case atDISCRETE:
            // printf(DiscreteV.GetVal(DisIdx).CStr());
            AttributesV.Add(TAttribute(AttrN, AttrsHashV.GetVal(AttrN).GetDat(
               DiscreteV.GetVal(DisIdx++))));
            break;
         case atCONTINUOUS:
            AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
            break;
         default:
            EFailR("Unsupported attribute type");
         }
      }
      EAssertR(AttrManV.Last().Type == atDISCRETE,
         "This function works only for classification.");
      TLabel Label = AttrsHashV.GetVal(AttrsN-1)[0]; // .operator[](0);
      return Classify(TExample::New(AttributesV, Label));
   }
   TStr THoeffdingTree::Classify(PExample Example) const { // Classification 
      return Classify(Root, Example);
   }
   void THoeffdingTree::IncCounts(PNode Node, PExample Example) const {
      EAssertR(Example->Label >= 0 && Example->Label <= Node->PartitionV.Len(),
         "Label out of bounds.");
      EAssertR(Node->PartitionV.GetVal(Example->Label) >= 0, "Negative count");
      Node->PartitionV.GetVal(Example->Label)++;
      EAssertR(Node->ExamplesN >= 0, "Negative number of examples.");
      Node->ExamplesN++;
      int AttrN = 0;
      for (auto It = Example->AttributesV.BegI();
         It != Example->AttributesV.EndI(); ++It) {
         switch (AttrManV.GetVal(It->Id).Type) {
         case atDISCRETE: {
            TTriple<TInt, TInt, TInt> Idx(It->Id, It->Value, Example->Label);
            if (Node->Counts.IsKey(Idx)) {
               EAssertR(Node->Counts.GetDat(Idx) >= 0, "Negative count.");
               ++Node->Counts.GetDat(Idx);
            } else {
               Node->Counts.AddDat(Idx, 1);
            }
            break;                              }
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
      EAssertR(Node->Id <= Example->LeafId.Val,
         "The example did not affect this node.");
      EAssertR(Node->PartitionV.GetVal(Example->Label) > 0,
         "Negative partition count.");
      --Node->PartitionV.GetVal(Example->Label);
      EAssertR(Node->ExamplesN > 0, "Negative example count.");
      --Node->ExamplesN;
      EAssertR(!Node->SeenH.IsKey(*Example), "Forgetting sacrificed example.");
      int AttrN = 0;
      for (auto It = Example->AttributesV.BegI();
         It != Example->AttributesV.EndI(); ++It) {
         switch (AttrManV.GetVal(It->Id).Type) {
         case atDISCRETE: {
            TTriple<TInt, TInt, TInt> Idx(It->Id, It->Value, Example->Label);
            if (Node->Counts.IsKey(Idx)) {
               EAssertR(Node->Counts.GetDat(Idx) > 0,
                  "Negative id-value-label triple count.");
               --Node->Counts.GetDat(Idx);
            } else {
               Print(Example);
               printf("Example ID: %d; Node ID: %d; Node examples: %d\n",
                  Example->LeafId.Val, Node->Id, Node->ExamplesN);
               if (!IsLeaf(Node)) {
                  printf("Node test attribute: %s\n",
                     AttrManV.GetVal(Node->CndAttrIdx).Nm.CStr());
               }
               printf("Problematic attribute: %s = %s\n",
                  AttrManV.GetVal(It->Id).Nm.CStr(),
                  AttrManV.GetVal(It->Id).InvAttrH.GetDat(It->Value).CStr());
               // NOTE: For debugging purposes; this fail
               // indicates serious problems 
               EFailR("Fatal: Corresponding id-value-label triple \
                  is missing in counts hashtable.");
            }
            break;                              }
         case atCONTINUOUS:
            // printf("Decrementing count for attribute %s\n",
            //    AttrManV.GetVal(It->Id).Nm.CStr()); // XXX: Debug 
            Node->HistH.GetDat(AttrN).DecCls(Example, AttrN);
            break;
         default:
            EFailR("Attribute type not supported.");
         }
         ++AttrN;
      }
   }
   bool THoeffdingTree::IsAltSplitIdx(PNode Node, const int& AttrIdx) const {
      for (auto It = Node->AltTreesV.BegI();
         It != Node->AltTreesV.EndI(); ++It) {
         // NOTE: Add `|| IsAltSplitIdx((*It)->Root, AttrIdx))` to allow
         // alternate trees to have alternate trees 
         // || IsAltSplitIdx((*It)->Root, AttrIdx)) { 
         if ((*It)->CndAttrIdx == AttrIdx) {
            return true;
         }
      }
      return false;
   }
   void THoeffdingTree::CheckSplitValidityCls() { // Classification 
      PNode CrrNode = Root;
      TSStack<PNode> NodeS;
      // Need -1 because AttrManV also manages class labels 
      const int AttrsN = AttrManV.Len()-1;
      NodeS.Push(CrrNode);
      // Depth-first tree traversal 
      while (!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         // Check split validity in the alternate trees 
         for (auto It = CrrNode->AltTreesV.BegI();
            It != CrrNode->AltTreesV.EndI(); ++It) {
            if (!IsLeaf(*It)) { NodeS.Push(*It); }
         }
         // Push non-leaf children on the stack 
         for (auto It = CrrNode->ChildrenV.BegI();
            It != CrrNode->ChildrenV.EndI(); ++It) {
            if (!IsLeaf(*It)) { NodeS.Push(*It); }
         }
         // Find the best two attributes among the remaining
         // attributes --- must not use CrrSplitAttrIdx 
         const int CrrSpltAttrIdx = CrrNode->CndAttrIdx;
         TVec<TInt> CrrBannedAttrV; CrrBannedAttrV.Add(CrrSpltAttrIdx);
         TBstAttr SpltAttr = CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV);
         CrrBannedAttrV.Clr(); CrrBannedAttrV.Add(SpltAttr.Val1.Val1);
         TBstAttr AltAttr = CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV);
         const double EstG = SpltAttr.Val1.Val2 - AltAttr.Val1.Val2;
         // Does it make sense to split on this one?
         if (EstG >= 0 && SpltAttr.Val1.Val1 != -1 &&
            SpltAttr.Val2.Val1 != -1 &&
            !IsAltSplitIdx(CrrNode, SpltAttr.Val1.Val1)) {
            // Hoeffding test
            const double Eps = CrrNode->ComputeTreshold(
               SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
            // EstG >= TieBreaking/2 ?
            if (EstG > Eps || (Eps < TieBreaking && EstG >= TieBreaking/2)) {
               // Grow alternate tree 
               Print('-');
               printf("Starting alternate tree for node splitting on `%s' \
                  with `%s' at root ; tie = %d\n",
                  AttrManV.GetVal(CrrNode->CndAttrIdx).Nm.CStr(),
                  AttrManV.GetVal(SpltAttr.Val1.Val1).Nm.CStr(),
                  EstG <= Eps);
               // TODO: Give user a chance to export before the model changes 
               // Export("exports/titanic-"+TInt(ExportN++).GetStr()+".gv",
               //   etDOT);
               const int LabelsN =
                  AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
               PNode AltHt = TNode::New(LabelsN, CrrNode->UsedAttrs, AttrManV,
                  IdGen->GetNextLeafId());
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
      while (!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         if (CrrNode->Id <= Example->LeafId && !Sacrificed(CrrNode, Example)) {
            DecCounts(CrrNode, Example);
            if (!IsLeaf(CrrNode)) {
               NodeS.Push(GetNextNode(CrrNode, Example));
               for (auto It = CrrNode->AltTreesV.BegI();
                  It != CrrNode->AltTreesV.EndI(); ++It) {
                  if ((*It)->Id <= Example->LeafId) { NodeS.Push(*It); }
               }
            }
         } else if (Sacrificed(CrrNode, Example)) { // Unmark 
            EAssertR(CrrNode->SeenH.GetDat(*Example) >= 1, "Decrementing \
               count lesser than or equal to zero.");
            if (!--CrrNode->SeenH.GetDat(*Example)) { // count == 0 
               CrrNode->SeenH.DelIfKey(*Example); 
            }
         }
      }
   }
   // Regression 
   void THoeffdingTree::ProcessLeafReg(PNode Leaf, PExample Example) {
      Leaf->UpdateStats(Example);
      // TODO: Get rid of this --- save variances only 
      Leaf->ExamplesV.Add(Example);
      const int AttrsN = Example->AttributesV.Len();
      for (int AttrN = 0; AttrN < AttrsN; AttrN++) {
         if (AttrManV.GetVal(AttrN).Type == atCONTINUOUS) {
            // TODO: Find an efficient way to compute s(A) from s(A1) and
            // s(A2) if A1 and A2 parition A 
            Leaf->HistH.GetDat(AttrN).IncReg(Example, AttrN);
            // EFailR("Current regression discretization is deprecated.");
         }
      }
      // Regression
      if (Leaf->ExamplesN % GracePeriod == 0 && Leaf->Std() > 0) {
         // See if we can get variance reduction 
         TBstAttr SplitAttr = Leaf->BestAttr(AttrManV, TaskType);
         // Pass 2, because TMath::Log2(2) = 1; since r lies in [0,1], we have
         // R=1; see also [Ikonomovska, 2012] and [Ikonomovska et al., 2011]
         const double Eps = Leaf->ComputeTreshold(SplitConfidence, 2);
         const double EstG = SplitAttr.Val3;
         // TODO: Handle the case when a branch of the tree is left with a
         // single unused attribute. One option: only split on the last
         // attribute if it reduces standard dev. "a lot", and if the leaf
         // contains many examples. 
         // const int UnusedAttrsN = AttrManV.Len()-1-Leaf->UsedAttrs.Len();
         // printf("# of unsed attrs = %d\n", UnusedAttrsN);
         // printf("EstG = %f ; Eps = %f\n", EstG, Eps); 
         // if (UsedAttrsN == 1 && Leaf->Std() > 
         if ((EstG < 1.0-Eps || Eps < TieBreaking) &&
            Leaf->UsedAttrs.SearchForw(SplitAttr.Val1.Val1, 0) < 0) {
            Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
         }
      }
   }
   // Classification 
   void THoeffdingTree::ProcessLeafCls(PNode Leaf, PExample Example) {
      const int AttrsN = Example->AttributesV.Len();
      IncCounts(Leaf, Example);
      // TODO: The constant 0.65 is hard-coded. 
      if (Leaf->ExamplesN % GracePeriod == 0 &&
         Leaf->ComputeEntropy() > 0.65) {
         TBstAttr SplitAttr = Leaf->BestAttr(AttrManV, TaskType);
         const double EstG = SplitAttr.Val3;
         const double Eps = Leaf->ComputeTreshold(
            SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
         if (SplitAttr.Val1.Val1 != -1 &&
            (EstG > Eps || (EstG <= Eps && Eps < TieBreaking))) {
            Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
         }
      }
   }
   void THoeffdingTree::SelfEval(PNode Node, PExample Example) const {
      // Remember we sacrificed Example in Node 
      if (Node->SeenH.IsKey(*Example)) {
         ++Node->SeenH.GetDat(*Example);
      } else { Node->SeenH.AddDat(*Example, 1); }
      // Update classification error for alternate trees 
      for (auto It = Node->AltTreesV.BegI();
         It != Node->AltTreesV.EndI(); ++It) {
         PNode CrrNode = *It;
         while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
         // TODO: Give user a chance to choose the classifier in leaves 
         EAssertR((*It)->Correct >= 0, "Negative number of correctly \
            classified examples < 0.");
         // NaiveBayes(CrrNode, Example);
         (*It)->Correct += (Example->Label == Majority(CrrNode));
         EAssertR((*It)->All >= 0, "Negative number of sacrificed examples");
         ++(*It)->All;
      }
      // Update classfication error for the main subtree 
      PNode CrrNode = Node;
      while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
      EAssertR(Node->Correct >= 0, "Negative number of \
         correctly classified examples.");
      // NaiveBayes(CrrNode, Example);
      Node->Correct += (Example->Label == Majority(CrrNode));
      EAssertR(Node->All >= 0, "Negative number of sacrificed examples.");
      ++Node->All;
   }
   bool THoeffdingTree::TestMode(PNode Node) {
      EAssertR(Node->TestModeN >= 0 && Node->TestModeN <= 10000, \
         "TestModeN out of bounds.");
      EAssertR(Node->All >= 0 && Node->All <= 2000, "All out of bounds.");
      // If the node has no alternate trees, then there is nothing to do 
      if (Node->AltTreesV.Empty()) { return false; }
      // Otherwise, see whether one of the alternate trees performs better
      // then the "main" subtree, spanned by Node. 
      if (Node->All == 2000) { // Swap with the best performing subtree 
         PNode BestAlt = Node;
         // Classification accuracy 
         //const double Acc = 1.0*BestAlt->Correct/BestAlt->All;
         for (auto It = Node->AltTreesV.BegI();
            It != Node->AltTreesV.EndI(); ++It) {
            const double CrrAcc = 1.0*(*It)->Correct/(*It)->All;
            if (CrrAcc > 1.0*BestAlt->Correct/BestAlt->All) { BestAlt = *It; }
            else { (*It)->All = (*It)->Correct = 0; } // Reset 
         }
         if (BestAlt != Node) {
            printf("[DEBUG] Swapping node with an alternate tree.\n");
            // TODO: Notify the user we're about to modify the model? 
            // Export("exports/titanic-"+TInt(ExportN++).GetStr()+".gv",
            //   etDOT);
            if (Node->Type == ntROOT) { BestAlt->Type = ntROOT; }
            *Node = *BestAlt;
         }
         Node->All = Node->Correct = 0; // Reset 
         return false;
      // Every 10000 examples the node enters the self-evaluation mode 
      // TODO: Let user set the parameter in the config?
      } else if (Node->All == 0 && Node->TestModeN >= 10000) {
         // printf("[DEBUG] Entering test mode...\n");
         Node->TestModeN = 0;
         return true;
      } else if (Node->All > 0) { return true; }
      ++Node->TestModeN;
      return false;
   }
   void THoeffdingTree::Process(const TStrV& DiscreteV, const TFltV& NumericV,
      const double& Val) {
      int DisIdx = 0, FltIdx = 0;
      TAttributeV AttributesV;
      for (int AttrN = 0; AttrN < AttrManV.Len()-1; ++AttrN) {
         switch (AttrManV.GetVal(AttrN).Type) {
         case atDISCRETE:
            // printf(DiscreteV.GetVal(DisIdx).CStr());
            AttributesV.Add(TAttribute(AttrN, AttrsHashV.GetVal(AttrN).GetDat(
               DiscreteV.GetVal(DisIdx++))));
            break;
         case atCONTINUOUS:
            AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
            break;
         default:
            EFailR("Unknown attribute type");
         }
      }
      ProcessReg(TExample::New(AttributesV, Val));
   }
   void THoeffdingTree::Process(const TStrV& DiscreteV, const TFltV& NumericV,
      const TStr& Label) {
      int DisIdx = 0, FltIdx = 0;
      TAttributeV AttributesV;
      for (int AttrN = 0; AttrN < AttrManV.Len()-1; ++AttrN) {
         switch (AttrManV.GetVal(AttrN).Type) {
         case atDISCRETE:
            // printf(DiscreteV.GetVal(DisIdx).CStr());
            AttributesV.Add(TAttribute(AttrN, AttrsHashV.GetVal(AttrN).GetDat(
               DiscreteV.GetVal(DisIdx++))));
            break;
         case atCONTINUOUS:
            AttributesV.Add(TAttribute(AttrN, NumericV.GetVal(FltIdx++)));
            break;
         default:
            EFailR("Unknown attribute type");
         }
      }
      ProcessCls(TExample::New(AttributesV, AttrsHashV.Last().GetDat(Label)));
   }
   void THoeffdingTree::ProcessCls(PExample Example) {
      Example->SetId(IdGen->GetNextExampleId());
      PNode CrrNode = Root;
      int MxId = 0;
      if (ConceptDriftP) {
         // Examples from ExampleQ are OK as they don't change anymore 
         // INVARIANT: ExampleQ.Len() <= WindowSize+1
         if (ExampleQ.Len()+1 >= WindowSize) {
            PExample LastExample = ExampleQ.Top();
            ExampleQ.Pop(); // Delete it from the window 
            ForgetCls(LastExample); // Update sufficient statistics 
         }
         TSStack<PNode> NodeS;
         TQQueue<PNode> NodeQ; // Self eval queue 
         NodeS.Push(CrrNode);
         while (!NodeS.Empty()) {
            CrrNode = NodeS.Top(); NodeS.Pop();
            // If an internal node sacrifices Example, it is possible 
            // it has higher ID than eveyrone except its siblings 
            MxId = TMath::Mx<int>(MxId, CrrNode->Id);
            if (IsLeaf(CrrNode)) { // Leaf node
               // ProcessLeafCls also increments counts 
               ProcessLeafCls(CrrNode, Example);
            } else {
               // Don't update counts --- sacrifice the next 2000 or so 
               // examples for internal evaluation 
               if (TestMode(CrrNode)) {
                  NodeQ.Push(CrrNode);
               } else { // Everything goes as usual
                  // Update sufficient statistics 
                  IncCounts(CrrNode, Example);
                  NodeS.Push(GetNextNode(CrrNode, Example));
                  for (auto It = CrrNode->AltTreesV.BegI();
                     It != CrrNode->AltTreesV.EndI(); ++It) {
                     NodeS.Push(*It);
                  }
               }
            }
         }
         Example->SetLeafId(TMath::Mx<int>(MxId, Example->LeafId));
         EAssertR(Example->LeafId >= 0 && Example->BinId >= 0, "Negative ID.");
         EAssertR((Example->BinId > 0 && Example->BinId >= Example->LeafId) ||
            Example->BinId == 0, "Problem with bin IDs.");
         ExampleQ.Push(Example);
         EAssertR(ExampleQ.Len() < WindowSize, "Too many examples in the \
            sliding window.");
         // Now, when Example stopped changing, do self-evaluation 
         while (!NodeQ.Empty()) {
            SelfEval(NodeQ.Top(), Example);
            NodeQ.Pop();
         }
         // TODO: Explain this 
         if (Root->HistH.Empty()) { Example->SetBinId(IdGen->GetNextBinId()); }
         EAssertR(DriftExamplesN >= 0 && DriftExamplesN <= DriftCheck, "Need \
            to check for concept drift.");
         if (++DriftExamplesN >= DriftCheck) {
            DriftExamplesN = 0;
            // printf("[DEBUG] Performing split validity check.\n");
            CheckSplitValidityCls();
         }
      } else {
         // Note that TestMode(CrrNode) should be false for VFDT because
         // AltTressV.Empty() is always true: there are no alternate trees
         // in VFDT.
         EAssertR(!TestMode(CrrNode), "Self-evaluating in adaptive mode.");
         while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
         ProcessLeafCls(CrrNode, Example);
      }
   }
   void THoeffdingTree::ProcessReg(PExample Example) {
      PNode CrrNode = Root;
      while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
      ProcessLeafReg(CrrNode, Example);
   }
   PExample THoeffdingTree::Preprocess(const TStr& Line,
      const TCh& Delimiter) const {
      TStrV LineV; TVec<TAttribute> AttributesV;
      Line.SplitOnAllCh(Delimiter, LineV);
      int ValN;
      EAssertR(AttrsHashV.Len() == LineV.Len(), "Number of attributes in the \
         dataset doesn't match the number of attributes in the configuration \
         file.");
      const int AttrsN = LineV.Len()-1;
      for (int CountN = 0; CountN < AttrsN; ++CountN) {
         // (1) Get appropriate hash table
         // (2) Get appropriate raw value from input attribute vector 
         // (3) Map raw attribute value to TInt with hash table 
         switch (AttrManV.GetVal(CountN).Type) {
         case atDISCRETE:
            // TODO: User should be notified about this. The least we can do is
            // give him a chance to specify in the config whether he
            // wants notifications.
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
         return TExample::New(AttributesV, AttrsHashV.GetVal(AttrsN).GetDat(
            LineV.GetVal(AttrsN)));
      } else {
         return TExample::New(AttributesV, LineV.Last().GetFlt());
      }
   }
   // NOTE: This is not limited to classification. 
   PNode THoeffdingTree::GetNextNode(PNode Node, PExample Example) const {
      if (!IsLeaf(Node)) {
         const TAttrType AttrType = AttrManV.GetVal(Node->CndAttrIdx).Type;
         EAssertR(AttrType == atDISCRETE || AttrType == atCONTINUOUS,
            "Invalid attribute type AttrType.");
         if (AttrType == atDISCRETE) {
            PNode RetNode = Node->ChildrenV.GetVal(Example->AttributesV.GetVal(
               Node->CndAttrIdx).Value);
            return Node->ChildrenV.GetVal(Example->AttributesV.GetVal(
               Node->CndAttrIdx).Value);
         } else { // Numeric attribute 
            const double Num =
               Example->AttributesV.GetVal(Node->CndAttrIdx).Num;
            const int Idx = Num <= Node->Val ? 0 : 1;
            return Node->ChildrenV.GetVal(Idx);
         }
      }
      return nullptr; // No children 
   }
   void THoeffdingTree::Clr(PNode Node, PNode SubRoot) {
      TSStack<PNode> NodeS;
      for (auto It = Node->ChildrenV.BegI();
         It != Node->ChildrenV.EndI(); ++It) {
         NodeS.Push(*It);   
      }
      for (auto It = Node->AltTreesV.BegI(); It !=
         Node->AltTreesV.EndI(); ++It) {
         if (*It != SubRoot) { NodeS.Push(*It); }
      }
      PNode CrrNode = nullptr;
      while(!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         for(auto It = CrrNode->ChildrenV.BegI();
            It != CrrNode->ChildrenV.EndI(); ++It) {
            NodeS.Push(*It);
         }
         for(auto It = CrrNode->AltTreesV.BegI();
            It != CrrNode->AltTreesV.EndI(); ++It) {
            NodeS.Push(*It);
         }
         CrrNode->Clr();
      }
      Node->Clr();
   }
   void THoeffdingTree::Export(const TStr& FileNm,
      const TExportType& ExportType) const {
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
         // %s {", FileNm.GetFBase().CStr()); 
         FOut.PutStrFmtLn("digraph dt_fig {");
         PrintDOT(Root, FOut, true);
         FOut.PutStrLn("}");
         break;
      default:
         EFailR("Uknown export format.");
      }
      FOut.Flush();
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
            TTriple<TInt, TInt, TInt> TmpTriple(AttrN,
               Example->AttributesV.GetVal(AttrN).Value, LabelN);
            if (Counts.IsKey(TmpTriple) && Counts.GetDat(TmpTriple) > 0) {
               SubExamplesN.GetVal(AttrN) += Counts.GetDat(TmpTriple);
            }
         }
      }
      for (int LabelN = 0; LabelN < LabelsN; ++LabelN) {
         nk = PartitionV.GetVal(LabelN); // number of positive examples 
         //printf("[DEBUG] #Examples = %d\n", nk);
         // TProbEstimates::LaplaceEstiamte(nk, CurrNode->ExamplesN-nk, 2); 
         pk = (nk+1.0)/(ExamplesN+LabelsN); 
         //printf("[DEBUG] Current: %f\n", pk);
         for (int i = 0; i < AttrsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(i,
               Example->AttributesV.GetVal(i).Value, LabelN);
            if (Counts.IsKey(TmpTriple) && Counts.GetDat(TmpTriple) > 0) {
               // apriori probability 
               // p0 = 1.0*CurrNode->Counts(TmpTriple)/nk;
               // compute conditional probability using m-estimate 
               // pk *= TProbEstimates::MEstimate(
               //   CurrNode->Counts(TmpTriple), nk, p0, 2);
               // pk *= 1.0*CurrNode->Counts(TmpTriple)/nk; 
               // (m * P(c_i) + n(x_k,c_i))/(P(c_i) * (m + n(x_k)))
               // laplace estimate for P(c_i) 
               pc = (nk+1.0)/(ExamplesN+LabelsN);
               pk *= (2.0*pc+Counts.GetDat(TmpTriple)) /
                  (pc*(2+SubExamplesN.GetVal(i)));
            }/* else {
               printf("[DEBUG] Zero probability!\n");
            }*/
         }
         if (MxProb < pk) {
            MxProb = pk; MxLabel = LabelN;
         }
         // printf("[DEBUG] Class `%s' with `probability' %f\n",
         //   InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().
         // ValueV.GetVal(LabelN)).CStr(), pk);
      }
      // Print('-', 120);
      // printf("-----------------------\n");
      // printf("[DEBUG] Bayes classified it as %s\n",
      // InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().ValueV.
      // GetVal(MxLabel)).CStr());
      /* ----------- */
      // printf("[DEBUG] CLASSIFYING:\n");
      // for(int i = 0; i < PartitionV.Len(); ++i) {
      //   printf("\t[DEBUG] #%s = %d\n",
      // InvAttrsHashV.GetVal(AttrsN).GetDat(AttrManV.Last().
      // ValueV.GetVal(i)).CStr(), PartitionV.GetVal(i));
      // }
      return MxLabel;
   }
   void THoeffdingTree::PrintHist(const TStr& FNm, const TCh& Ch) const {
      TFOut FOut(FNm);
      TFOut FVec(FNm+".vec");
      double SplitVal;
      // Find the first numeric attribute if any 
      // for(TAttrManV::TIter It = AttrManV.BegI();
      //   It != AttrManV.EndI(); ++It) {
      TAttrManV::TIter It = AttrManV.BegI(); ++It; ++It;
         if (It->Type == atCONTINUOUS) {
            const int AttrN = It->Id;
            //printf("Numeric distribution for '%s' attribute.\n",
            //   It->Nm.CStr());
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
      for (auto It = Example->AttributesV.BegI();
         It != Example->AttributesV.EndI(); ++It) {
         switch (AttrManV.GetVal(It->Id).Type) {
            case atCONTINUOUS: {
               printf("%f\t", It->Num.Val);
               break;
            }
            case atDISCRETE: {
               printf("%s\t",
                  AttrManV.GetVal(It->Id).InvAttrH.GetDat(It->Value).CStr());
               break;
            }
         }
      }
      putchar('\n');
   }
   void THoeffdingTree::Init(const TStr& ConfigFNm) {
      TParser Parser(ConfigFNm);
      // NOTE: Although not critical, this can still be expensive.
      // We should use C++11 move semantics.
      TParams Params = Parser.GetParams();
      AttrsHashV = Params.AttrsHV;
      InvAttrsHashV = Params.InvAttrsHV;
      LabelH = Params.DataFormatH;
      InvLabelH = Params.InvDataFormatH;
      
      InitAttrMan();
   }
   void THoeffdingTree::Init(PJsonVal JsonConfig) {
      EAssertR(JsonConfig->IsObjKey("dataFormat"),
         "Expected key 'dataFormat'.");
      PJsonVal DataFormatArr = JsonConfig->GetObjKey("dataFormat");
      EAssertR(DataFormatArr->IsArr(),
         "Expected array of attributes after 'dataFormat:'.");
      TStrV AttrNmV;
      DataFormatArr->GetArrStrV(AttrNmV);
      // printf("----------------------------\n");

      AttrsHashV.Reserve(AttrNmV.Len(), AttrNmV.Len());
      InvAttrsHashV.Reserve(AttrNmV.Len(), AttrNmV.Len());
      for (int AttrN = 0; AttrN != AttrNmV.Len(); ++AttrN) {
         LabelH.AddDat(AttrNmV.GetVal(AttrN), AttrN);
         InvLabelH.AddDat(AttrN, AttrNmV.GetVal(AttrN));
         // printf("-- Processing attribute '%s' --\n",
         //    AttrNmV.GetVal(AttrN).CStr());
         PJsonVal AttrVal = JsonConfig->GetObjKey(AttrNmV.GetVal(AttrN));
         EAssertR(AttrVal->IsObjKey("type"),
            "Expected key 'type' for each attribute.");
         // Retrieve possible values 
         if (AttrVal->GetObjStr("type") == "discrete") {
            EAssertR(AttrVal->IsObjKey("values"),
               "Expected key 'values' for each attribute.");
            PJsonVal ValuesArr = AttrVal->GetObjKey("values");
            EAssertR(ValuesArr->IsArr(), "Expected array of values after \
               'values' for discrete attribute.");
            TStrV ValuesV;
            // printf("\tValues: ");
            ValuesArr->GetArrStrV(ValuesV);
            // Now iterating through values of each of the attribute 
            for (int AttrValN = 0; AttrValN != ValuesV.Len(); ++AttrValN) {
               // printf("%s\t", ValuesV.GetVal(AttrValN).CStr());
               AttrsHashV.GetVal(AttrN).AddDat(
                  ValuesV.GetVal(AttrValN), AttrValN);
               InvAttrsHashV.GetVal(AttrN).AddDat(
                  AttrValN, ValuesV.GetVal(AttrValN));
            }
            // printf("\n");
         } else if (AttrVal->GetObjStr("type") == "numeric") {
            // Numeric attribute 
            AttrsHashV.GetVal(AttrN).AddDat("", 0);
            InvAttrsHashV.GetVal(AttrN).AddDat(0, "");
         } else {
            EFailR(TStr::Fmt("Attribute '%s': Each attribute 'type' is either \
            'discete' or 'numeric'.", AttrNmV.GetVal(AttrN).CStr()).CStr());
         }
      }
      // Done processing config 
      InitAttrMan();
   }
   void THoeffdingTree::SetParams(PJsonVal JsonParams) {
      if (JsonParams->IsObjKey("gracePeriod") &&
         JsonParams->GetObjKey("gracePeriod")->IsNum()) {
         GracePeriod = JsonParams->GetObjInt("gracePeriod");
         // printf("GracePeriod = %d\n", GracePeriod);
      }
      if (JsonParams->IsObjKey("windowSize") &&
         JsonParams->GetObjKey("windowSize")->IsNum()) {
         WindowSize = JsonParams->GetObjInt("windowSize");
         // printf("WindowSize = %d\n", WindowSize);
      }
      if (JsonParams->IsObjKey("driftCheck") &&
         JsonParams->GetObjKey("driftCheck")->IsNum()) {
         DriftCheck = JsonParams->GetObjInt("driftCheck");
         // printf("DriftCheck = %d\n", DriftCheck);
      }
      if (JsonParams->IsObjKey("tieBreaking") &&
         JsonParams->GetObjKey("tieBreaking")->IsNum()) {
         TieBreaking = JsonParams->GetObjNum("tieBreaking");
         // printf("TieBreaking = %f\n", TieBreaking);
      }
      if (JsonParams->IsObjKey("splitConfidence") &&
         JsonParams->GetObjKey("splitConfidence")->IsNum()) {
         SplitConfidence = JsonParams->GetObjNum("splitConfidence");
         // printf("SplitConfidence = %f\n", SplitConfidence);
      }
      if (JsonParams->IsObjKey("conceptDriftP") &&
         JsonParams->GetObjKey("conceptDriftP")->IsBool()) {
         ConceptDriftP = JsonParams->GetObjBool("conceptDriftP");
         // printf("ConceptDriftP = %d\n", ConceptDriftP);
      }
   }
   // Create attribute manager object for each attribute 
   // NOTE: Label is also ``attribute-managed''
   void THoeffdingTree::InitAttrMan() {
      const int AttrsN = AttrsHashV.Len();
      for (int CountN = 0; CountN < AttrsN; ++CountN) {
         // Continuous attributes have, in a sense, a `single' value 
         if (AttrsHashV.GetVal(CountN).Len() == 1) {
            AttrManV.Add(TAttrMan(AttrsHashV.GetVal(CountN),
               InvAttrsHashV.GetVal(CountN), CountN,
               InvLabelH.GetDat(CountN), atCONTINUOUS));
         } else {
            // printf("%s\n", InvAttrsHashV.GetVal(CountN).GetDat(0).CStr());
            AttrManV.Add(TAttrMan(AttrsHashV.GetVal(CountN),
               InvAttrsHashV.GetVal(CountN), CountN,
               InvLabelH.GetDat(CountN), atDISCRETE));
         }
      }
      const TAttrType PredType = AttrManV.Last().Type;
      EAssertR(PredType == atDISCRETE || PredType == atCONTINUOUS,
         "Invalid target attribute type PredType.");
      if (PredType == atDISCRETE) {
         TaskType = ttCLASSIFICATION;
      } else {
         TaskType = ttREGRESSION;
      }
      // Initialize the root node 
      Root = TNode::New(LabelH.Len(), TVec<TInt>(), AttrManV,
         IdGen->GetNextLeafId(), ntROOT);
   }

   // Pre-order depth-first tree traversal 
   void THoeffdingTree::PrintXML(PNode Node, const int& Depth,
      TFOut& FOut) const {
      TStr Indent("");
      for (int i = 0; i < Depth; ++i) { Indent += "\t"; }
      if (!Node->ChildrenV.Len()) { // Leaf node 
         FOut.PutStr(Indent);
         FOut.PutStrFmtLn("<leaf class=\"%s\"></leaf>",
            GetMajorityNm(Node).CStr());
         return;
      }
      TStr ValNm;
      const int ChildrenN = Node->ChildrenV.Len();
      for (int ChildN = 0; ChildN < ChildrenN; ++ChildN) {
         FOut.PutStr(Indent);
         TAttrType AttrType = AttrManV.GetVal(Node->CndAttrIdx).Type;
         EAssertR(AttrType == atDISCRETE || AttrType == atCONTINUOUS,
            "Invalid attribute type AttrType.");
         if (AttrType == atDISCRETE) {
            ValNm = GetNodeValueNm(Node, ChildN);
         } else {
            ValNm = (ChildN ? ">" : "<=");
            ValNm += TFlt::GetStr(Node->Val);
         }
         FOut.PutStrFmtLn("<node attribute=\"%s\" value=\"%s\">",
            GetNodeNm(Node).CStr(), ValNm.CStr());
         PrintXML(Node->ChildrenV.GetVal(ChildN), Depth+1, FOut);
         FOut.PutStr(Indent);
         FOut.PutStrLn("</node>");
      }
   }
   void THoeffdingTree::PrintJSON(PNode Node, const int& Depth,
      TFOut& FOut) const {
      EFailR("JSON export not yet implemented.");
   }
   // Breadh-first tree traversal
   void THoeffdingTree::PrintDOT(PNode Node, TFOut& FOut,
      const bool& AlternateP) const {
      TQQueue<TPair<PNode, TInt> > Queue;
      int NodeId = 0; // Used to achieve uniqueness 
      Queue.Push(TPair<PNode, TInt>(Node, NodeId));
      if (Node->ChildrenV.Empty()) {
         EAssertR(TaskType == ttCLASSIFICATION || TaskType == ttREGRESSION,
            "Invalid task type.");
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
            // Make sure DOT doesn't crash later beacuse of illegal labels 
            TmpValueNm.ChangeCh('-', 'D');
            if (TmpNode->ChildrenV.Empty()) {
               EAssertR(TaskType == ttCLASSIFICATION || TaskType == ttREGRESSION,
                  "Invalid task type.");
               if (TaskType == ttCLASSIFICATION) {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\"];",
                     GetNodeNm(CrrNode).CStr(), CurrPair.Val2,
                     GetMajorityNm(TmpNode).CStr(), NodeId, TmpValueNm.CStr());
               } else {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\"];",
                     GetNodeNm(CrrNode).CStr(), CurrPair.Val2,
                     TFlt::GetStr(TmpNode->Avg).CStr(), NodeId,
                     TmpValueNm.CStr());
               }
            } else {
               FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\"];",
                  GetNodeNm(CrrNode).CStr(), CurrPair.Val2,
                  GetNodeNm(TmpNode).CStr(), NodeId, TmpValueNm.CStr());
               Queue.Push(TPair<PNode, TInt>(TmpNode, NodeId));
            }
         }
         // Draw alternate trees 
         for (int TreeN = 0; AlternateP &&
            TreeN < CrrNode->AltTreesV.Len(); ++TreeN) {
            ++NodeId;
            PNode TmpNode = CrrNode->AltTreesV[TreeN];
            ValueNm = "*"; // Marks alternate tree 
            if (TmpNode->ChildrenV.Empty()) {
               EAssertR(TaskType == ttCLASSIFICATION || TaskType == ttREGRESSION,
                  "Invalid task type.");
               if (TaskType == ttCLASSIFICATION) {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\", \
                     style=\"dotted\"];", GetNodeNm(CrrNode).CStr(),
                     CurrPair.Val2, GetMajorityNm(TmpNode).CStr(),
                     NodeId, ValueNm.CStr());
               } else {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\", \
                     style=\"dotted\"];", GetNodeNm(CrrNode).CStr(),
                     CurrPair.Val2, TFlt::GetStr(TmpNode->Avg).CStr(),
                     NodeId, ValueNm.CStr());
               }
            } else {
               FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\", \
                  style=\"dotted\"];", GetNodeNm(CrrNode).CStr(),
                  CurrPair.Val2, GetNodeNm(TmpNode).CStr(), NodeId,
                  ValueNm.CStr());
               Queue.Push(TPair<PNode, TInt>(TmpNode, NodeId));
            }
         }
      }
   }
   void THoeffdingTree::Print(const TCh& Ch, const TInt& Num) {
      for (int ChN = 0; ChN < Num; ++ChN) {
         printf("%c", Ch.Val);
      }
      printf("\n");
   }
}

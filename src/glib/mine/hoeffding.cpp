/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

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
      CrrCh = SIn->GetCh();
      // Skip whitespace
      EatWs();
      // Ignore comments
      while (CrrCh == '#') { SkipLn(); EatWs(); }

      switch (CrrCh) {
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
      if (IsValid(CrrCh)) {
         TChA ChA;
         ChA.AddCh(CrrCh);
         while (!SIn->Eof() && IsValid(SIn->PeekCh())) {
            CrrCh = SIn->GetCh();
            ChA.AddCh(CrrCh);
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
            // printf("[Line %d] Illegal character: '%c'.\n", LineN, CrrCh);
            EFailR("Illegal character.");
         }
         LastTok = TToken("", totEND, LineN); // End-of-file
      }
      return LastTok;
   }
   void TLexer::EatWs() {
      while (!SIn->Eof() && TCh::IsWs(CrrCh)) {
         if (CrrCh == '\n') ++LineN;
         CrrCh = SIn->GetCh();
      }
   }
   void TLexer::SkipLn() {
      while (!SIn->Eof() && CrrCh != '\n') { CrrCh = SIn->GetCh(); }
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
      EAssertR(N >= 0, "N < 0 in Entropy");
      if (N == 0) { return 0.0; }
      // Make sure frequencies add up to N
      int FreqSum = 0;
      for (auto It = FreqV.BegI(); It != FreqV.EndI(); ++It) {
         EAssertR(It->Val <= N, "Frequencey counts don't add up (Val>N).");
         FreqSum += It->Val;
         p = 1.0*It->Val/N;
         if (p > 0) { h -= p*TMath::Log2(p); }
      }
      EAssertR(FreqSum == N, "Frequency counts don't add up.");
      return h;
   }
   double TMisc::GiniIndex(const TIntV& FreqV, const int& N) {
      double g = 1.0, p = 0.0;
      EAssertR(N >= 0, "N < 0 in GiniIndex");
      if (N == 0) { return 0.0; }
      int FreqSum = 0;
      for (auto It = FreqV.BegI(); It != FreqV.EndI(); ++It) {
         EAssertR(It->Val <= N, "Frequencey counts don't add up (Val>N).");
         FreqSum += It->Val;
         p = 1.0*It->Val/N;
         g -= p*p;
      }
      EAssertR(FreqSum == N, "Frequency counts don't add up.");
      return g;
   }
   ///////////////////////////////
   // Extended-Binary-Search-Tree
   double TExBST::ComputeSDR() const {
      const double TotalSumSq = TotalSumSqLeft+TotalSumSqRight;
      EAssertR(TMath::Sqr(TotalSumSq - Root->SumSqLeft - Root->SumSqRight) < 1e-6,
         "Sums of squares dont match");
      const double TotalSum = TotalSumLeft+TotalSumRight;
      EAssertR(TMath::Sqr(TotalSum - Root->SumLeft - Root->SumRight) < 1e-6,
         "Sums dont match");
      const int TotalCount = TotalCountLeft+TotalCountRight;
      EAssertR(TMath::Sqr(TotalCount - Root->CountLeft - Root->CountRight) < 1e-6,
         "Counts dont match");

      // This is not unbiased estimator
      if (TotalCount == 0) {
         return 0.0;
      }
      const double V = (TotalSumSq-TMath::Sqr(TotalSum)/TotalCount)/TotalCount;
      EAssertR(V >= -1e-5, "V < -1e-5");
      const double SD = V > 0 ? TMath::Sqrt(V) : 0.0;

      double LeftV = 0.0;
      if (TotalCountLeft > 0) {
         LeftV = (TotalSumSqLeft-TMath::Sqr(TotalSumLeft)/TotalCountLeft)/TotalCountLeft;
      }
      EAssertR(LeftV >= -1e-5, "LeftV < -1e-5");
      const double LeftSD = LeftV > 0.0 ? TMath::Sqrt(LeftV) : 0.0;

      double RightV = 0.0;
      if (TotalCountRight > 0) {
         RightV = (TotalSumSqRight-TMath::Sqr(TotalSumRight)/TotalCountRight)/TotalCountRight;
      }
      EAssertR(RightV >= -1e-5, "RightV < -1e-5");
      const double RightSD = RightV > 0.0 ? TMath::Sqrt(RightV) : 0.0;

      double SDR = SD;
      if (TotalCount > 0) {
         SDR = SD - TotalCountLeft*LeftSD/TotalCount - TotalCountRight*RightSD/TotalCount;
      }
      EAssertR(SDR >= -1e-6, "SDR < 0");
      return SDR;
   }
   void TExBST::Insert(const double& Key, const double& Val) {
      if (Root() == nullptr) {
         Root = TExBSTNode::New(Key);
      }
      PExBSTNode CrrNode = Root;
      // CrrNode()==nullptr when inserting key that is already in the tree
      while (CrrNode() != nullptr) {
         if (Key <= CrrNode->Key) {
            CrrNode->SumLeft += Val;
            CrrNode->SumSqLeft += Val*Val;
            ++CrrNode->CountLeft;
            // Add a new node to the tree (if key not in the tree already)
            if (CrrNode->LeftChild() == nullptr && Key != CrrNode->Key) {
               PExBSTNode NewNode = TExBSTNode::New(Key);
               CrrNode->LeftChild = NewNode;
               break;
            } else {
               CrrNode = CrrNode->LeftChild;
            }
         } else { // Key > CrrNode->Key
            CrrNode->SumRight += Val;
            CrrNode->SumSqRight += Val*Val;
            ++CrrNode->CountRight;
            if (CrrNode->RightChild() == nullptr) {
               PExBSTNode NewNode = TExBSTNode::New(Key);
               CrrNode->RightChild = NewNode;
               break;
            } else {
               CrrNode = CrrNode->RightChild;
            }
         }
      }
   }
   void TExBST::Reset() {
      MxSDR = 0.0; // Reset MxSDR so we can use it again
      SplitVal = 0.0; // Reset the split value
      TotalSumLeft = 0.0;
      TotalSumRight = Root->SumLeft+Root->SumRight;
      TotalSumSqLeft = 0.0;
      TotalSumSqRight = Root->SumSqLeft+Root->SumSqRight;
      TotalCountLeft = 0;
      TotalCountRight = Root->CountRight+Root->CountLeft;
   }
   // Finds the best split by in-order traversal of the tree
   void TExBST::FindBestSplit(PExBSTNode Node) {
      if (Node->LeftChild() != nullptr) {
         FindBestSplit(Node->LeftChild);
      }
      // Update the sums and counts for computing the SDR
      TotalSumLeft += Node->SumLeft;
      TotalSumRight -= Node->SumLeft;
      TotalSumSqLeft += Node->SumSqLeft;
      TotalSumSqRight -= Node->SumSqLeft;
      TotalCountLeft += Node->CountLeft;
      TotalCountRight -= Node->CountLeft;
      // Remember the best split point
      const double CrrSDR = ComputeSDR();
      if (MxSDR < CrrSDR) {
         SplitVal = Node->Key;
         MxSDR = CrrSDR;
      }
      if (Node->RightChild() != nullptr) {
         FindBestSplit(Node->RightChild);
      }
      // Update the sums and counts for returing to parent
      TotalSumLeft -= Node->SumLeft;
      TotalSumRight += Node->SumLeft;
      TotalSumSqLeft -= Node->SumSqLeft;
      TotalSumSqRight += Node->SumSqLeft;
      TotalCountLeft -= Node->CountLeft;
      TotalCountRight += Node->CountLeft;
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

   ///////////////////////////////
   // Histogram
   // Per-class distribution for examples with attribute
   // NOTE: This function must ensure that Example->BinId is set to the
   // maximum ID of the bins containing the example
   void THist::IncCls(PExample Example, const int& AttrIdx, PIdGen IdGen) {
      EAssertR(AttrIdx >= 0 && AttrIdx < Example->AttributesV.Len(),
         "Index out of bounds.");
      int Idx = 0, BinN = 0;
      double CrrDist = 0.0, PrevDist = 0.0;
      const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
      const int Label = Example->Label;
      // Add new bin, initialized with Val, if the number of bins (BinsN)
      // didn't reach the threshold
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
            PrevDist = CrrDist = fabs(Val - BinsV.GetVal(0).GetVal());
            // NOTE: We could use binary search because of the ordering
            // invariant; but the number of bins rarely exeecds 100
            // While distance starts increasing, stop --- our bin is the one
            // before the current one
            for (BinN = 1; BinN < BinsV.Len(); ++BinN) {
               EAssertR(BinsV.GetVal(BinN-1).GetVal() <=
                  BinsV.GetVal(BinN).GetVal(), "Bins not sorted.");
               PrevDist = CrrDist;
               CrrDist = fabs(Val - BinsV.GetVal(BinN).GetVal());
               if (CrrDist > PrevDist) {
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
      double CrrDist = 0.0, PrevDist = 0.0;
      const double Val = Example->AttributesV.GetVal(AttrIdx).Num;
      const int Label = Example->Label;
      // Idx = BinsV.SearchBin(Val); // Binary search for Val
      Idx = BinsV.SearchBin(Val);
      EAssertR(Idx != -1 || BinsN == BinsV.Len(),
         "No bin initialized with this value nor enough bins.");
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
         // if (BinN == BinsV.Len()) {
         //   for (int i = 0; i < BinsV.Len(); ++i) {
         //     printf("BinId=%d\n", BinsV.GetVal(i).Id);
         //   }
         // }
         // NOTE: For debugging purposes
         EAssertR(BinN < BinsV.Len(), "No suitable Bin with Bin.ID<=Ex.ID.");
         PrevIdx = Idx = BinN; // First suitable bin
         PrevDist = CrrDist = fabs(Val - BinsV.GetVal(BinN).GetVal());
         // The order is preserved even though new bins might have been
         // created between the old ones
         for (; BinN < BinsV.Len(); ++BinN) {
            if (BinsV.GetVal(BinN).Id <= Example->BinId) {
               PrevDist = CrrDist;
               CrrDist = fabs(Val - BinsV.GetVal(BinN).GetVal());
               if (CrrDist > PrevDist) {
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
      double CrrDist = 0.0, PrevDist = 0.0;
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
            CrrDist = PrevDist = fabs(Val - BinsV.GetVal(0).GetVal());
            for (BinN = 1; BinN < BinsV.Len(); ++BinN) {
               PrevDist = CrrDist;
               // We are fine, because bins are ordered inside the vector
               // by the initialization values
               CrrDist = fabs(Val - BinsV.GetVal(BinN).GetVal());
               if (CrrDist > PrevDist) {
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
      int HiCount = 0, LoCount = 0, CrrCount = 0, MxIdx = 0;
      double MxGain = 0.0, CrrGain = 0.0;
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
      CrrGain = MxGain = 0.0;
      MxIdx = 0;
      for (int BinN = BinsV.Len()-2; BinN >= 0; --BinN) {
         CrrCount = BinsV.GetVal(BinN+1).Count;
         // No need for this: BinsV.GetVal(MxIdx).GetVal()
         // Val = BinsV.GetVal(BinN+1).Value;
         LoCount += CrrCount;
         HiCount = NArr[BinN];
         HiImp = GArr[BinN];
         TIntV TmpV = BinsV.GetVal(BinN+1).PartitionV;
         TMisc::AddVec(1, TmpV, LoV);
         LoImp = TMisc::Entropy(LoV, LoCount);
         CrrGain = H - LoCount*LoImp/AllN - HiCount*HiImp/AllN;
         if (CrrGain > MxGain) {
            MxGain = CrrGain;
            MxIdx = BinN;
         }
      }
      delete [] GArr;
      delete [] NArr;
      if (MxIdx > 0) {
         SplitVal = BinsV.GetVal(MxIdx).GetVal();
         return MxGain;
      } else {
         printf("[DEBUG] InfoGain chose 0th split bin.\n");
         return 0.0;
      }
   }
   double THist::GiniGain(double& SplitVal) const {
      int HiCount = 0, LoCount = 0, CrrCount = 0, MxIdx = 0;
      double MxGain = 0.0, CrrGain = 0.0;
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
         // h_i := Gini(B_1\cup B_2\cup \ldots\cup B_i)
         GArr[BinN] = TMisc::GiniIndex(HiV, HiCount);
         NArr[BinN] = HiCount; // n_i := |B_1|+\ldots+|B_i|
      }
      const int AllN = HiCount;
      const double H = TMisc::GiniIndex(HiV, AllN);
      // Now find the best split
      CrrGain = MxGain = 0.0;
      MxIdx = 0;
      for (int BinN = BinsV.Len()-2; BinN >= 0; --BinN) {
         CrrCount = BinsV.GetVal(BinN+1).Count;
         // No need for this: BinsV.GetVal(MxIdx).GetVal()
         // Val = BinsV.GetVal(BinN+1).Value;
         LoCount += CrrCount;
         HiCount = NArr[BinN];
         HiImp = GArr[BinN];
         TIntV TmpV = BinsV.GetVal(BinN+1).PartitionV;
         TMisc::AddVec(1, TmpV, LoV);
         LoImp = TMisc::GiniIndex(LoV, LoCount);
         CrrGain = H - LoCount*LoImp/AllN - HiCount*HiImp/AllN;
         if (CrrGain > MxGain) {
            MxGain = CrrGain;
            MxIdx = BinN;
         }
      }
      delete [] GArr;
      delete [] NArr;
      if (MxIdx > 0) {
         SplitVal = BinsV.GetVal(MxIdx).GetVal();
         return MxGain;
      } else {
         printf("[DEBUG] GiniGain chose 0th split bin.\n");
         return 0.0;
      }
   }
   // See [Knuth, 1997] and [Chan et al., 1979] for details regarding
   // updating formulas for variance
   // (http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance,
   // accessed on 7 Jun 2013)
   // Formulas from [Chan et al., 1979] allow us to compute the totial
   // sample variance by combining variances of bins. See page 2 and equations
   // (1.5a) and (1.5b).
   double THist::StdGain(double& SpltVal) const { // For regression
      int HiCnt, LoCnt, CrrCnt;
      int MxIdx;
      double MxGain, CrrGain;
      double LoS, HiS, LoT;
      // Define VarArr: SArr[i] := n*Var(B_1\cup B_2\cup ...\cup B_i)
      double* SArr = new double[BinsN]();
      // Define AvgArr: TArr[i] := x_1+x_2+...+x_i
      double* TArr = new double[BinsN]();
      // Compute initial split
      LoCnt = HiCnt = 0; // BinsV.GetVal(0).Count;
      SArr[0] = TArr[0] = 0.0;
      // Compute S and T of the first i bins together, 1 <= i <= BinsV.len()
      for (int BinN = 0; BinN < BinsV.Len(); ++BinN) {
         const TBin CrrBin = BinsV.GetVal(BinN);
         const double PrevS = BinN > 0 ? SArr[BinN-1] : 0.0;
         const double PrevT = BinN > 0 ? TArr[BinN-1] : 0.0;
         TArr[BinN] = PrevT+CrrBin.T;
         CrrCnt = BinsV.GetVal(BinN).Count;
         // Accounts for the S_{1,m}+S_{m+1,m+n} part of the formula
         SArr[BinN] = PrevS+CrrBin.S;
         if (CrrCnt > 0 && HiCnt > 0) {
            // See [Chan et al., Algorithms for Computing the Sample Variance,
            // 1979]
            // Accoutns for m/(n(m+n)) * (n/m * T_{1,m}-T_{m+1,m+n})^2
            SArr[BinN] += TMath::Sqr( // this is squaring (^2)
               CrrCnt*PrevT/HiCnt-CrrBin.T)*1.0*HiCnt/(CrrCnt*(CrrCnt+HiCnt));
         }
         HiCnt += CrrCnt;
      }
      const int AllN = HiCnt;
      // S is the "whole" sum (x1-mean)^2+...+(xn-mean)^2
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
         // HiT = TArr[BinN];
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
         LoT += CrrT; // CrrT == BinsV.GetVal(BinN).T;
         LoCnt += CrrCnt;
         HiCnt -= CrrCnt;
      }
      delete [] TArr;
      delete [] SArr;
      if (MxIdx > 0) {
         // Set the split val
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
         /* LstExamplesQ(Node.LstExamplesQ), */ SeenH(Node.SeenH),
         CountsH(Node.CountsH), PartitionV(Node.PartitionV),
         UsedAttrs(Node.UsedAttrs), HistH(Node.HistH),
         AltTreesV(Node.AltTreesV), Id(Node.Id), Correct(Node.Correct),
         All(Node.All), PhAvgErr(Node.PhAvgErr), PhMnErr(Node.PhMnErr),
         PhCumSum(Node.PhCumSum), PhAlpha(Node.PhAlpha),
         PhLambda(Node.PhLambda), PhInitN(Node.PhInitN)
   { EFailR("TNode (const TNode&)"); }
   // Assignment operator
   TNode& TNode::operator=(const TNode& Node) {
      if (this != &Node) {
         //Clr(); // Delete old elements
         CndAttrIdx = Node.CndAttrIdx;
         ExamplesN = Node.ExamplesN;
         Val = Node.Val;
         Avg = Node.Avg;
         VarSum = Node.VarSum;
         Err = Node.Err;
         TestModeN = Node.TestModeN;
         Type = Node.Type;
         ExamplesV = Node.ExamplesV;
         // LstExamplesQ = Node.LstExamplesQ;
         SeenH = Node.SeenH;
         CountsH = Node.CountsH;
         PartitionV = Node.PartitionV;
         ChildrenV = Node.ChildrenV;
         UsedAttrs = Node.UsedAttrs;
         HistH = Node.HistH;
         AltTreesV = Node.AltTreesV;
         Id = Node.Id;
         Correct = Node.Correct;
         All = Node.All;
         PhAvgErr = Node.PhAvgErr;
         PhMnErr = Node.PhMnErr;
         PhCumSum = Node.PhCumSum;
         PhAlpha = Node.PhAlpha;
         PhLambda = Node.PhLambda;
         PhInitN = Node.PhInitN;
         /* EFailR("TNode& operator=(const TNode&)"); */
      }
      return *this;
   }
   bool TNode::operator==(const TNode& Node) const {
      return CndAttrIdx == Node.CndAttrIdx && Type == Node.Type &&
         ExamplesV == Node.ExamplesV && CountsH == Node.CountsH &&
         PartitionV == Node.PartitionV && Id == Node.Id &&
         ChildrenV == Node.ChildrenV && UsedAttrs == Node.UsedAttrs &&
         SeenH == Node.SeenH; /* this line added 4th aug 2014 */
     EFailR("bool TNode::operator==(const TNode&) const;");
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
            if(CountsH.IsKey(TmpTriple)) {
               SubExamplesN += CountsH.GetDat(TmpTriple);
            }
         }
         hj = 0.0;
         // Compute H(E_j)
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if (CountsH.IsKey(TmpTriple)) {
               // Prevent divison by zero
               pj = SubExamplesN > 0 ?
                  1.0*CountsH.GetDat(TmpTriple)/SubExamplesN : 0.0;
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
            if (CountsH.IsKey(TmpTriple)) {
               SubExamplesN += CountsH.GetDat(TmpTriple);
            }
         }
         gj = 1.0;
         for (int i = 0; i < LabelsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(AttrIdx, j, i);
            if (CountsH.IsKey(TmpTriple)) {
               // Prevent divison by zero
               pj = SubExamplesN > 0 ?
                  1.0*CountsH.GetDat(TmpTriple)/SubExamplesN : 0.0;
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
   double TNode::ComputeThreshold(const double& Delta,
      const int& LabelsN) const {
      // Range of the random variable for information gain
      const double R = TMath::Log2(LabelsN);
      EAssertR(ExamplesN > 0, "This node has no examples.");
      EAssertR(Delta > 0.0, "Delta <= 0.0");
      // \epsilon = \sqrt{ \frac{R^2 * log(1/delta)}{2n} }
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
         PNode NewNode = TNode::New(LabelsN, UsedAttrs, AttrManV,
            IdGen->GetNextLeafId());
         // NewNode->LstExamplesQ = LstExamplesQ;

         // Set parameters for the Page-Hinkley test
         NewNode->SetPhAlpha(PhAlpha);
         NewNode->SetPhLambda(PhLambda);

         ChildrenV.Add(NewNode);
      }
      if (Type != ntROOT) { Type = ntINTERNAL; }
   }
   void TNode::Clr() { // Forget training examples
      ExamplesV.Clr(); PartitionV.Clr(); CountsH.Clr();
      HistH.Clr(true); AltTreesV.Clr(); UsedAttrs.Clr();
      SeenH.Clr(true);
   }
   // Regression
   TBstAttr TNode::BestRegAttr(const TAttrManV& AttrManV,
      const TAttrDiscretization& AttrDiscretization) {
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
            // printf("#\nCrrSdr=%f\n", CrrSdr);
            switch (AttrDiscretization) {
            case adBST:
               CrrSdr = BstH.GetDat(AttrN).GetBestSplit(Val);
               break;
            case adHISTOGRAM:
               // This is the "old" way, using histogram
               CrrSdr = HistH.GetDat(AttrN).StdGain(Val);
               // printf("SplitVal = %f\n", CrrSdr);
               break;
            default:
               EFailR("Undefined attribute discretization option.");
            }
         }
         if (CrrSdr > Mx1) {
            Idx2 = Idx1; Idx1 = AttrN; Mx2 = Mx1; Mx1 = CrrSdr;
         } else if (CrrSdr >= Mx2) {
            Idx2 = AttrN; Mx2 = CrrSdr;
         }
      }
      // printf("Mx1 = %f ; Mx2 = %f\n", Mx1, Mx2);
      // printf("A1 = %d ; A2 = %d\n", Idx1, Idx2);
      // If Mx1==0.0, then Mx2==0.0, because we have 0.0<=Mx2<=Mx1
      const double Ratio = Mx1 > 0.0 ? Mx2/Mx1 : 1.0;
      return TBstAttr(TPair<TInt, TFlt>(Idx1, Mx1),
         TPair<TInt, TFlt>(Idx2, Mx2), Ratio);
   }
   // Classification
   TBstAttr TNode::BestClsAttr(const TAttrManV& AttrManV,
      const TIntV& BannedAttrV, const TAttrHeuristic& AttrHeuristic) {
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
               switch (AttrHeuristic) {
               case ahINFO_GAIN:
                  Crr = InfoGain(AttrN, AttrManV);
                  break;
               case ahGINI_GAIN:
                  Crr = GiniGain(AttrN, AttrManV);
                  break;
               default:
                  EFailR("Unknown attribute heuristic for classification.");
               }
            }
         } else { // Numeric attribute
            switch (AttrHeuristic) {
            case ahINFO_GAIN:
               Crr = HistH.GetDat(AttrN).InfoGain(SplitVal);
               break;
            case ahGINI_GAIN:
               Crr = HistH.GetDat(AttrN).GiniGain(SplitVal);
               break;
            default:
               EFailR("Unknown attribute heuristic for classification.");
            }
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
   TBstAttr TNode::BestClsAttr(const TAttrManV& AttrManV,
         const TAttrHeuristic& AttrHeuristic) {
      TIntV DummyBannedV; // Empty vector
      return BestClsAttr(AttrManV, DummyBannedV, AttrHeuristic);
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
            BstH.AddDat(AttrN, TExBST());
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
   double THoeffdingTree::Predict(PNode Node, PExample Example) const {
      PNode CrrNode = Node;
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
            // If Val <= Node->SplitVal, go left; else go right
            const int Idx = Val <= CrrNode->Val ? 0 : 1;
            CrrNode = CrrNode->ChildrenV.GetVal(Idx);
         }
      }
      double Pred = 0.0; // Prediction
      // Ikonomovska [Ikonomovska, 2012] trains perceptron in the leaves
      switch (RegressLeaves) {
      case rlMEAN:
         Pred = CrrNode->Avg;
         break;
      case rlLINEAR:
         EFailR("Linear models for regression not yet implemented.");
         break;
      default:
         EFailR("Unkown model. Choose rlMEAN or rlLINEAR.");
      }
      return Pred;
   }
   double THoeffdingTree::Predict(PExample Example) const {
      return Predict(Root, Example);
   }
   TStr THoeffdingTree::Classify(PNode Node, PExample Example) const {
      PNode CrrNode = Node;
      while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
      TStr Label = "";
      switch (ClassifyLeaves) {
      case clMAJORITY:
         Label = GetMajorityNm(CrrNode);
         break;
      case clNAIVE_BAYES: {
         int Idx = NaiveBayes(CrrNode, Example);
         Label = AttrManV.GetVal(AttrManV.Len()-1).InvAttrH.GetDat(Idx); }
         break;
      default:
         EFailR("Unknown model. Choose clMAJORITY or clNAIVE_BAYES.");
      }
      return Label;
   }
   TStr THoeffdingTree::Classify(const TStrV& DiscreteV,
      const TFltV& NumericV) const {
      int DisIdx = 0, FltIdx = 0;
      TAttributeV AttributesV;
      const int AttrsN = AttrManV.Len();
      for (int AttrN = 0; AttrN < AttrsN-1; ++AttrN) {
         switch (AttrManV.GetVal(AttrN).Type) {
         case atDISCRETE:
            // printf("%s\n", DiscreteV.GetVal(DisIdx).CStr());
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
      EAssertR(Example->Label >= 0 && Example->Label < Node->PartitionV.Len(),
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
            if (Node->CountsH.IsKey(Idx)) {
               EAssertR(Node->CountsH.GetDat(Idx) >= 0, "Negative count.");
               ++Node->CountsH.GetDat(Idx);
            } else {
               Node->CountsH.AddDat(Idx, 1);
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
      EAssertR(Node->Id <= Example->LeafId,
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
            if (Node->CountsH.IsKey(Idx)) {
               if (Node->CountsH.GetDat(Idx) <= 0) { printf("Example ID=%d\n", Example->Id.Val); }
               EAssertR(Node->CountsH.GetDat(Idx) > 0,
                  "Negative id-value-label triple count.");
               --Node->CountsH.GetDat(Idx);
            } else { // TODO: Replace this execution branch with an assert
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
               EFailR("Corresponding id-value-label triple \
                  is missing in the counts hashtable.");
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
      if (!IsLeaf(CrrNode)) { NodeS.Push(CrrNode); }
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
         TBstAttr SpltAttr =
            CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV, AttrHeuristic);
         CrrBannedAttrV.Clr(); CrrBannedAttrV.Add(SpltAttr.Val1.Val1);
         TBstAttr AltAttr =
            CrrNode->BestClsAttr(AttrManV, CrrBannedAttrV, AttrHeuristic);
         const double EstG = SpltAttr.Val1.Val2 - AltAttr.Val1.Val2;
         // Does it make sense to split on this one?
         if (EstG >= 0 && SpltAttr.Val1.Val1 != -1 &&
            SpltAttr.Val2.Val1 != -1 &&
            !IsAltSplitIdx(CrrNode, SpltAttr.Val1.Val1)) {
            // Hoeffding test
            const double Eps = CrrNode->ComputeThreshold(
               SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
            // EstG >= TieBreaking/2 ?
            if (EstG > Eps || (Eps < TieBreaking && EstG >= TieBreaking/2)) {
               // Grow alternate tree
               Print('-');
               printf("CndAttrIdx=%d\n", CrrNode->CndAttrIdx);
               printf("Starting alternate tree for node splitting on `%s' \
                  with `%s' at root ; tie = %d\n",
                  AttrManV.GetVal(CrrNode->CndAttrIdx).Nm.CStr(),
                  AttrManV.GetVal(SpltAttr.Val1.Val1).Nm.CStr(),
                  EstG <= Eps);
               // TODO: Give user a chance to export before the model changes
               // Export("exports/dataset-"+TInt(ExportN++).GetStr()+".gv",
               //   etDOT);
               const int LabelsN =
                  AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
               PNode AltHt = TNode::New(LabelsN, CrrNode->UsedAttrs, AttrManV,
                  IdGen->GetNextLeafId());

               // Set parameters for the Page-Hinkley test
               AltHt->SetPhAlpha(PhAlpha);
               AltHt->SetPhLambda(PhLambda);

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
         // If CrrNode->Id <= Example->LeafId, then CrrNode existed before
         // Example arrived
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
            EAssertR(CrrNode->SeenH.GetDat(*Example) >= 1,
               "Decrementing counter lesser than or equal to zero.");
            if (!--CrrNode->SeenH.GetDat(*Example)) { // count == 0
               CrrNode->SeenH.DelIfKey(*Example);
            }
         }
      }
   }
   // Regression
   double THoeffdingTree::ProcessLeafReg(PNode Leaf, PExample Example) {
      Leaf->UpdateStats(Example);
      // TODO: Get rid of this --- save variances only
      Leaf->ExamplesV.Add(Example);
      // Leaf->LstExamplesQ.Push(Example);
      // if (Leaf->LstExamplesQ.Len() > 100) {
      //   Leaf->LstExamplesQ.Pop();
      // }
      const int AttrsN = Example->AttributesV.Len();
      for (int AttrN = 0; AttrN < AttrsN; AttrN++) {
         if (AttrManV.GetVal(AttrN).Type == atCONTINUOUS) {
            switch (AttrDiscretization) {
            case adBST: {
               // Key is the attribute value
               const double Key = Example->AttributesV.GetVal(AttrN).Num;
               // Val is the value of the target variable
               const double Val = Example->Value;
               Leaf->BstH.GetDat(AttrN).Insert(Key, Val);
               break; }
            case adHISTOGRAM:
               // TODO: Find an efficient way to compute s(A) from s(A1) and
               // s(A2) if A1 and A2 parition A
               Leaf->HistH.GetDat(AttrN).IncReg(Example, AttrN);
               break;
            default:
               EFailR(
                  "Regression: Undefined attribute discretization option.");
            }
         }
      }
      ++Leaf->All; // The number of examples in this node
      const double CrrPrediction = Leaf->Avg;
      // Regression
      // The second condition --- the one with MxNodes --- makes sure that
      // the implication `if MxNodes != 0, then NodesN() < MxNodes` holds
      if (Leaf->ExamplesN % GracePeriod == 0 && Leaf->Std() >= SdThresh &&
         (MxNodes == 0 || (MxNodes != 0 && GetNodesN() < MxNodes))) {
         // See if we can get variance reduction
         TBstAttr SplitAttr = Leaf->BestRegAttr(AttrManV, AttrDiscretization);
         // Pass 2, because TMath::Log2(2) = 1; since r lies in [0,1], we have
         // R=1; see also [Ikonomovska, 2012] and [Ikonomovska et al., 2011]
         const double Eps = Leaf->ComputeThreshold(SplitConfidence, 2);
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
            Leaf->UsedAttrs.SearchForw(SplitAttr.Val1.Val1, 0) < 0 &&
            SplitAttr.Val1.Val2 >= SdrThresh) {
            Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
         }
      }
      return CrrPrediction;
   }
   // Classification
   void THoeffdingTree::ProcessLeafCls(PNode Leaf, PExample Example) {
      const int AttrsN = Example->AttributesV.Len();
      IncCounts(Leaf, Example);
      const double H = Leaf->ComputeEntropy();
      // TODO: Use stricter condition to prevent growth
      if (Leaf->ExamplesN % GracePeriod == 0 && Leaf->GetExamplesN() > 5 &&
         H > 0 && (MxNodes == 0 || (MxNodes != 0 && GetNodesN() < MxNodes))) {
         TBstAttr SplitAttr = Leaf->BestClsAttr(AttrManV, AttrHeuristic);
         const double EstG = SplitAttr.Val3;
         const double Eps = Leaf->ComputeThreshold(
            SplitConfidence, AttrManV.GetVal(AttrsN).ValueV.Len());
         if (SplitAttr.Val1.Val1 != -1) {
            // Information gain of the best attribute
            const double IG1 = SplitAttr.Val1.Val2;
            // Preprunning [Hulten et al., 2001]
            // Note that (H-IG1)-H == -IG1
            EAssertR(fabs((H-IG1)-H - (-IG1)) < 0.000001, "(H-IG1)-H != -IG1");
            if (-IG1 > Eps || (IG1 < Eps && Eps < TieBreaking)) {
               printf("[DEBUG] Preprunned.\n");
               return;
            }
            if ((EstG > Eps || (EstG <= Eps && Eps < TieBreaking))) {
               Leaf->Split(SplitAttr.Val1.Val1, AttrManV, IdGen);
            }
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
         EAssertR((*It)->Correct >= 0, "Negative number of correctly \
            classified examples < 0.");
         int Loss = 0;
         switch (ClassifyLeaves) {
         case clMAJORITY:
            Loss = (Example->Label == Majority(CrrNode));
            break;
         case clNAIVE_BAYES:
            Loss = (Example->Label == NaiveBayes(CrrNode, Example));
            break;
         }
         (*It)->Correct += Loss;
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
      EAssertR(Node->TestModeN >= 0 && Node->TestModeN <= 10000,
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
   void THoeffdingTree::Debug_Finalize() {
      // Empty the sliding window and make sure all counts are reset to 0
      while (!ExampleQ.Empty()) {
         ForgetCls(ExampleQ.Top());
         ExampleQ.Pop();
      }
      // Now go through all nodes of the tree and make sure counts == 0
      TSStack<PNode> NodeS;
      PNode CrrNode = Root;
      NodeS.Push(CrrNode);
      while (!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         // Check counts
         EAssertR(CrrNode->ExamplesN == 0, "ExamplesN != 0");
         for (int AttrN = 0; AttrN < AttrManV.Len()-1; ++AttrN) {
            switch (AttrManV.GetVal(AttrN).Type) {
            case atDISCRETE:
               for (int ValN = 0;
                  ValN < AttrManV.GetVal(AttrN).ValueV.Len(); ++ValN) {
                  for (int LabelN = 0;
                     LabelN < AttrManV.Last().ValueV.Len(); ++LabelN) {
                     TTriple<TInt, TInt, TInt> Idx(AttrN, ValN, LabelN);
                     EAssertR(CrrNode->CountsH.IsKey(Idx) &&
                        CrrNode->CountsH.GetDat(Idx) == 0, "No way");
                  }
               }
               break;
            case atCONTINUOUS:
               for (int i = 0; i < CrrNode->HistH.Len(); ++i) {
                  CrrNode->HistH.GetDat(AttrN).Debug_Check();
               }
               break;
            default:
               EFailR("Unknown attribute type");
            }
         }
         if (!IsLeaf(CrrNode)) {
            for (auto It = CrrNode->ChildrenV.BegI();
               It != CrrNode->ChildrenV.EndI(); ++It) {
               NodeS.Push(*It);
            }
         }
      }
   }
   void THoeffdingTree::Debug_CheckInvariant(PExample Example) const {
      // XXX: Make sure that Example->BinId is as it is supposed to be
      PNode CrrNode = Root;
      TSStack<PNode> NodeS;
      NodeS.Push(CrrNode);
      while (!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         // Now check Example->BinId >= max(bin ids from CrrNode)
         const int AttrsN = Example->AttributesV.Len();
         for (int AttrN = 0; AttrN < AttrsN; AttrN++) {
            if (AttrManV.GetVal(AttrN).Type == atCONTINUOUS &&
               CrrNode->HistH.GetDat(AttrN).BinsV.Len() > 0) {
               // TODO: Find an efficient way to compute s(A) from s(A1) and
               // s(A2) if A1 and A2 parition A
               int MnId = CrrNode->HistH.GetDat(AttrN).BinsV.Last().Id;
               for (int BinN = 0;
                  BinN < CrrNode->HistH.GetDat(AttrN).BinsV.Len(); ++BinN) {
                  MnId = TMath::Mn<int>(MnId,
                     CrrNode->HistH.GetDat(AttrN).BinsV.GetVal(BinN).Id);
                  EAssertR(
                     MnId<=CrrNode->HistH.GetDat(AttrN).BinsV.GetVal(BinN).Id,
                     "BinId should be minimal, but is not.");
               }
               if (Example->BinId < MnId) {
                  printf("Ex->BinId=%d; MnId=%d\n", Example->BinId.Val, MnId);
               }
               EAssertR(Example->BinId >= MnId, "Ex->BinId < MnId");
            }
         }
         if (!IsLeaf(CrrNode) && !Sacrificed(CrrNode, Example) &&
            CrrNode->Id <= Example->LeafId) {
            PNode TmpNode = GetNextNode(CrrNode, Example);
            // EAssertR(TmpNode->Id <= Example->LeafId, "Node-Id>Ex->LeafId.");
            NodeS.Push(TmpNode);
            for (auto It = CrrNode->AltTreesV.BegI();
               It != CrrNode->AltTreesV.EndI(); ++It) {
               if ((*It)->Id <= Example->LeafId) { NodeS.Push(*It); }
            }
         }
      }
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
            // printf("Check invariant before forgetting\n");
            // Debug_CheckInvariant(LastExample);
            ForgetCls(LastExample); // Update sufficient statistics
         }
         TSStack<PNode> NodeS;
         TQQueue<PNode> NodeQ; // Self eval queue
         NodeS.Push(CrrNode);
         while (!NodeS.Empty()) {
            CrrNode = NodeS.Top(); NodeS.Pop();
            // If an internal node sacrifices Example, it is possible
            // it has higher ID than everyone except its siblings
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

         // if (!Sacrificed(Root, Example)) { Debug_CheckInvariant(Example); }

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
         EAssertR(DriftExamplesN >= 0, "DriftExamplesN<0");
         EAssertR(DriftExamplesN <= DriftCheck,
            "Need to check for concept drift.");
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
   // Notes:
   // (*) Backpropagate error over the branches of the main tree
   // (*) Update error at roots of the alternate trees -- not other nodes
   double THoeffdingTree::ProcessReg(PExample Example) {
      PNode CrrNode = Root;
      double Pred = 0.0;
      // Use Page-Hinkley test to detect concept drift
      if (ConceptDriftP) {
         TSStack<PNode> TraverseS;
         TraverseS.Push(CrrNode);
         while (!IsLeaf(CrrNode)) {
            for (auto It = CrrNode->AltTreesV.BegI();
               It != CrrNode->AltTreesV.EndI(); ++It) {
               Pred = LeafUpdatePh(*It, Example);
               if (!IsLeaf(*It)) {
                  UpdatePhStats(*It, Example->Value, Pred);
               }
            }
            CrrNode = GetNextNode(CrrNode, Example);
            TraverseS.Push(CrrNode);
         }
         EAssertR(CrrNode() == TraverseS.Top()(), "Top not leaf");
         TraverseS.Pop(); // So we don't increment leaf counts twice
         Pred = ProcessLeafReg(CrrNode, Example);
         while (!TraverseS.Empty()) {
            CrrNode = TraverseS.Top(); TraverseS.Pop();
            UpdatePhStats(CrrNode, Example->Value, Pred);
            EAssertR(!IsLeaf(CrrNode), "Leaf node not on top of the stack");
            // Check whether m(T)-M(T)>Lambda
            if (PhTriggered(CrrNode)) {
               if (CrrNode->All % 100 == 0 &&
                  !CrrNode->AltTreesV.Empty()) {
                  // Evaluate Q-statistic and see whether it makes
                  // sense to swap
                  // (1) Find the best-pefroming alternate tree
                  PNode BestNode = CrrNode->AltTreesV.Last();
                  for (auto It = CrrNode->AltTreesV.BegI();
                     It != CrrNode->AltTreesV.EndI(); ++It) {
                     if ((*It)->PhAvgErr < BestNode->PhAvgErr &&
                        (*It)->All >= PhInitN) {
                        BestNode = *It;
                     }
                  }
                  // (2) Compute log(S_orig/S_alt) and swap if necessary
                  if (BestNode->PhAvgErr > 0.0 &&
                     CrrNode->PhAvgErr > BestNode->PhAvgErr) {
                     printf("Swapped!\n");
                     CrrNode = BestNode;
                     BestNode->AltTreesV.Clr();
                  }
               } else {
                  TBstAttr BstAttr =
                     CrrNode->BestRegAttr(AttrManV, AttrDiscretization);
                  if (BstAttr.Val1.Val1 != CrrNode->CndAttrIdx &&
                     !IsAltSplitIdx(CrrNode, BstAttr.Val1.Val1)) {
                     const int LabelsN =
                        AttrManV.GetVal(AttrManV.Len()-1).ValueV.Len();
                     printf("Starting an alternate tree on %s instead of %s\n",
                        AttrManV.GetVal(BstAttr.Val1.Val1).Nm.CStr(),
                        AttrManV.GetVal(CrrNode->CndAttrIdx).Nm.CStr());
                     PNode AltHt = TNode::New(LabelsN,
                        CrrNode->UsedAttrs, AttrManV, IdGen->GetNextLeafId());
                     // Set parameters for the Page-Hinkley test
                     AltHt->SetPhAlpha(PhAlpha);
                     AltHt->SetPhLambda(PhLambda);
                     AltHt->Split(BstAttr.Val1.Val1, AttrManV, IdGen);
                     CrrNode->AltTreesV.Add(AltHt);
                     ++AltTreesN;
                  }
               }
            }
         }
      } else { // No concept drift detection -- life is much simpler here
         while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
         Pred = ProcessLeafReg(CrrNode, Example);
      }
      return Pred;
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
            // If attribute value <= split value, go left; else go right
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
   // TODO: Return P(X=c) for all class labels c; it's more informative
   // than argmax_c P(X=c)
   TLabel THoeffdingTree::NaiveBayes(PNode Node, PExample Example) const {
      const THash<TTriple<TInt, TInt, TInt>, TInt> CountsH = Node->CountsH;
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
            if (CountsH.IsKey(TmpTriple) && CountsH.GetDat(TmpTriple) > 0) {
               SubExamplesN.GetVal(AttrN) += CountsH.GetDat(TmpTriple);
            }
         }
      }
      for (int LabelN = 0; LabelN < LabelsN; ++LabelN) {
         nk = PartitionV.GetVal(LabelN); // Number of positive examples
         //printf("[DEBUG] #Examples = %d\n", nk);
         // TProbEstimates::LaplaceEstiamte(nk, CrrNode->ExamplesN-nk, 2);
         pk = (nk+1.0)/(ExamplesN+LabelsN);
         //printf("[DEBUG] Current: %f\n", pk);
         for (int i = 0; i < AttrsN; ++i) {
            TTriple<TInt, TInt, TInt> TmpTriple(i,
               Example->AttributesV.GetVal(i).Value, LabelN);
            if (CountsH.IsKey(TmpTriple) && CountsH.GetDat(TmpTriple) > 0) {
               // Apriori probability
               // p0 = 1.0*CrrNode->Counts(TmpTriple)/nk;
               // Compute conditional probability using m-estimate
               // pk *= TProbEstimates::MEstimate(
               //   CrrNode->Counts(TmpTriple), nk, p0, 2);
               // pk *= 1.0*CrrNode->Counts(TmpTriple)/nk;
               // (m * P(c_i) + n(x_k,c_i))/(P(c_i) * (m + n(x_k)))
               // Laplace estimate for P(c_i)
               pc = (nk+1.0)/(ExamplesN+LabelsN);
               pk *= (2.0*pc+CountsH.GetDat(TmpTriple)) /
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
   int THoeffdingTree::GetNodesN(const bool& AltP) const {
      int NodesN = 0;
      PNode CrrNode = Root;
      TSStack<PNode> NodeS;

      NodeS.Push(CrrNode); ++NodesN;
      while (!NodeS.Empty()) {
         CrrNode = NodeS.Top(); NodeS.Pop();
         for (auto It = CrrNode->ChildrenV.BegI();
            It != CrrNode->ChildrenV.EndI(); ++It) {
            ++NodesN;
            if (!IsLeaf(*It)) { NodeS.Push(*It); }
         }
         // Also count alternate trees
         if (AltP) {
            for (auto It = CrrNode->AltTreesV.BegI();
               It != CrrNode->AltTreesV.EndI(); ++It) {
               ++NodesN;
               if (!IsLeaf(*It)) { NodeS.Push(*It); }
            }
         }
      }
      return NodesN;
   }
   void THoeffdingTree::UpdatePhStats(PNode Node, PExample Example) const {
      const double& Val = Example->Value;
      const double& Pred = Predict(Node, Example);
      UpdatePhStats(Node, Val, Pred);
   }
   void THoeffdingTree::UpdatePhStats(PNode Node, const double& Val,
      const double& Pred) const {
      ++Node->All; // Number of examples that passed through the node
      // Update the mean error [Knuth, TAOCP, Vol. 2, p. 232]
      const double TmpErr = fabs(Val-Pred);
      const double CrrErr = Node->PhAvgErr;
      Node->PhAvgErr = CrrErr + (TmpErr-CrrErr)/Node->All;
      // Update cumulative sum
      Node->PhCumSum +=
         (TmpErr - Node->PhAvgErr - Node->PhAlpha);
      // PhMnErr = -1.0 means the error is not yet set
      // Wait for PhInitN examples, so statistics "stabilizes"
      if (Node->PhMnErr < 0.0 || Node->All < PhInitN) {
         Node->PhMnErr = Node->PhCumSum;
      }
      Node->PhMnErr = TMath::Mn(Node->PhMnErr, Node->PhCumSum);
   }
   bool THoeffdingTree::PhTriggered(PNode Node) const {
      return Node->PhCumSum-Node->PhMnErr > Node->PhLambda &&
         Node->All >= PhInitN;
   }
   double THoeffdingTree::LeafUpdatePh(PNode Node, PExample Example) {
      PNode CrrNode = Node;
      while (!IsLeaf(CrrNode)) { CrrNode = GetNextNode(CrrNode, Example); }
      return ProcessLeafReg(Node, Example);
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
      // XXX: Binary-search tree technique not implemented for classification
      if (TaskType == ttCLASSIFICATION) {
         EAssertR(AttrDiscretization == adHISTOGRAM,
            "BST discretization not implemented for classification");
      } else { // ttREGRESSION
         // XXX: Working on it as we speak
         // EAssertR(ConceptDriftP == false,
         //   "Concept adaption not implemented for regression");
      }
      EAssertR(AttrDiscretization == adHISTOGRAM ||
         AttrDiscretization == adBST,
         "AttrDiscretization has invalid value");
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
      if (JsonParams->IsObjKey("maxNodes") &&
         JsonParams->GetObjKey("maxNodes")->IsNum()) {
         MxNodes = (int)JsonParams->GetObjNum("maxNodes");
         // printf("MxNodes = %d\n", MxNodes);
      }
      if (JsonParams->IsObjKey("conceptDriftP") &&
         JsonParams->GetObjKey("conceptDriftP")->IsBool()) {
         ConceptDriftP = JsonParams->GetObjBool("conceptDriftP");
         // printf("ConceptDriftP = %d\n", ConceptDriftP);
      }
      // Functional leaves
      if (JsonParams->IsObjKey("regLeafModel") &&
         JsonParams->GetObjKey("regLeafModel")->IsStr()) {
         TStr ModelStr = JsonParams->GetObjStr("regLeafModel");
         // printf("ModelStr = %s\n", ModelStr.CStr());
         if (ModelStr == "mean") {
            RegressLeaves = rlMEAN;
         } else if (ModelStr == "linear") {
            RegressLeaves = rlLINEAR;
         } else {
            EFailR("Unknown option: '"+ModelStr+"'");
         }
      }
      if (JsonParams->IsObjKey("clsLeafModel") &&
         JsonParams->GetObjKey("clsLeafModel")->IsStr()) {
         TStr ModelStr = JsonParams->GetObjStr("clsLeafModel");
         if (ModelStr == "majority") {
            ClassifyLeaves = clMAJORITY;
         } else if (ModelStr == "naiveBayes") {
            ClassifyLeaves = clNAIVE_BAYES;
         } else {
            EFailR("Unknown option: '"+ModelStr+"'");
         }
      }
      // Attribute heuristic measure
      if (JsonParams->IsObjKey("clsAttrHeuristic") &&
         JsonParams->GetObjKey("clsAttrHeuristic")->IsStr()) {
         TStr HeuristicStr = JsonParams->GetObjStr("clsAttrHeuristic");
         if (HeuristicStr == "infoGain") {
            AttrHeuristic = ahINFO_GAIN;
         } else if ( HeuristicStr == "giniGain") {
            AttrHeuristic = ahGINI_GAIN;
         } else {
            EFailR("Unknown option: '"+HeuristicStr+"'");
         }
      }
      // Numeric attribute discretization
      if (JsonParams->IsObjKey("attrDiscretization") &&
         JsonParams->GetObjKey("attrDiscretization")->IsStr()) {
         TStr DiscretizationStr = JsonParams->GetObjStr("attrDiscretization");
         if (DiscretizationStr == "histogram") {
            AttrDiscretization = adHISTOGRAM;
         } else if (DiscretizationStr == "bst") {
            AttrDiscretization = adBST;
         } else {
            EFailR("Unknown option: '"+DiscretizationStr+"'");
         }
      }
      // SDR threshold
      if (JsonParams->IsObjKey("sdrThreshold") &&
         JsonParams->GetObjKey("sdrThreshold")->IsNum()) {
         const double Thresh = JsonParams->GetObjNum("sdrThreshold");
         EAssertR(Thresh >= 0.0,
            "JSON config error: sdrThreshold must be nonnegative");
         SdrThresh = Thresh;
      }
      // SD threshold (*not* the same as SdrThresh)
      if (JsonParams->IsObjKey("sdThreshold") &&
         JsonParams->GetObjKey("sdThreshold")->IsNum()) {
         const double Thresh = JsonParams->GetObjNum("sdThreshold");
         EAssertR(Thresh >= 0.0,
            "JSON config error: sdThreshold must be nonnegative");
         SdThresh = Thresh;
      }
      // Page-Hinkley test parameters
      // Alpha parameter (set to expected standard deviation of the signal)
      if (JsonParams->IsObjKey("phAlpha") &&
         JsonParams->GetObjKey("phAlpha")->IsNum()) {
         PhAlpha = JsonParams->GetObjNum("phAlpha");
         EAssertR(PhAlpha >= 0.0,
            "JSON config error: phAlpha must be nonnegative");
      }
      // Lambda parameter (the threshold)
      if (JsonParams->IsObjKey("phLambda") &&
         JsonParams->GetObjKey("phLambda")->IsNum()) {
         PhLambda = JsonParams->GetObjNum("phLambda");
         EAssertR(PhLambda >= 0.0,
            "JSON config error: phLambda must be nonnegative");
      }
      // InitN, the number of examples for mean value to "stabilize"
      if (JsonParams->IsObjKey("phInit") &&
         JsonParams->GetObjKey("phInit")->IsNum()) {
         PhInitN = (int)JsonParams->GetObjNum("phInit");
         EAssertR(PhInitN >= 0,
            "JSON config error: phInit must be nonnegative");
      }
      // Lambda parameter (the threshold)
      if (JsonParams->IsObjKey("fadingFactor") &&
         JsonParams->GetObjKey("fadingFactor")->IsNum()) {
         FadingFactor = JsonParams->GetObjNum("fadingFactor");
         EAssertR(FadingFactor > 0.0,
            "JSON config error: fadingFactor must be positive");
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

     // Set parameters for the Page-Hinkley test
     Root->SetPhAlpha(PhAlpha);
     Root->SetPhLambda(PhLambda);
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
            FOut.PutStrFmtLn("v=\t\"%f\";", Node->Avg);
         }
         return;
      }
      while (!Queue.Empty()) {
         TPair<PNode, TInt> CrrPair = Queue.Top();
         PNode CrrNode = CrrPair.Val1;
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
                     GetNodeNm(CrrNode).CStr(), (int)CrrPair.Val2,
                     GetMajorityNm(TmpNode).CStr(), NodeId, TmpValueNm.CStr());
               } else {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\"];",
                     GetNodeNm(CrrNode).CStr(), (int)CrrPair.Val2,
                     TFlt::GetStr(TmpNode->Avg).CStr(), NodeId,
                     TmpValueNm.CStr());
               }
            } else {
               FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\"];",
                  GetNodeNm(CrrNode).CStr(), (int)CrrPair.Val2,
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
                     (int)CrrPair.Val2, GetMajorityNm(TmpNode).CStr(),
                     NodeId, ValueNm.CStr());
               } else {
                  FOut.PutStrFmtLn("\t%s%d -> \"%s%d\" [label=\"L%s\", \
                     style=\"dotted\"];", GetNodeNm(CrrNode).CStr(),
                     (int)CrrPair.Val2, TFlt::GetStr(TmpNode->Avg).CStr(),
                     NodeId, ValueNm.CStr());
               }
            } else {
               FOut.PutStrFmtLn("\t%s%d -> %s%d [label=\"L%s\", \
                  style=\"dotted\"];", GetNodeNm(CrrNode).CStr(),
                  (int)CrrPair.Val2, GetNodeNm(TmpNode).CStr(), NodeId,
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


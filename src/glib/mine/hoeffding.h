#ifndef HOEFFDING_H
#define HOEFFDING_H

namespace THoeffding {
   ///////////////////////////////
   // Forward-Declarations
   class THist;
   class TBin;
   class TAttribute;
   class TNode;
   class TAttrMan;
   ClassHdTP(TExample, PExample);
   ClassHdTP(THoeffdingTree, PHoeffdingTree)

   typedef TInt TLabel;
   typedef TVec<THist> THistV;
   typedef TVec<TAttribute> TAttributeV;
   typedef TVec<TBin> TBinV;
   typedef TVec<TNode> TNodeV;
   // Typedef TVec<PExample> PExampleV;
   typedef TVec<TAttrMan> TAttrManV;
   // Another option is struct { int Idx1, Idx2; double Mx1, Mx2, Diff; };
   typedef TTriple<TPair<TInt, TFlt>, TPair<TInt, TFlt>, TFlt> TBstAttr;

   // Numeric attribute discretization
   const int BinsN = 100;

   // Model in the leaves for regression 
   typedef enum {
      rlMEAN,              // Predict the mean in the leaves 
      rlLINEAR             // Fit a linear model in the leaves 
   } TRegressLeaves;
   
   // Classifier in the leaves 
   typedef enum {
      clMAJORITY,      // Use majority classifier 
      clNAIVE_BAYES  // Use NaiveBayes classifier 
   } TClassifyLeaves;
   
   // Learning task type 
   typedef enum {
      ttCLASSIFICATION,  // Classification task 
      ttREGRESSION       // Regression task 
   } TTaskType;
   
   // Attribute value type 
   typedef enum {
      atDISCRETE,   // Discrete (i.e. nominal) attribute 
      atCONTINUOUS  // Continuous (i.e. numeric) attribute 
   } TAttrType;
   
   // Node type 
   typedef enum {
      ntROOT,      // Root node 
      ntINTERNAL,  // Internal node 
      ntLEAF       // Leaf node 
   } TNodeType;
   
   // Export type 
   typedef enum {
      etXML,   // Export model as XML
      etJSON,  // Export model as JSON 
      etDOT      // Export model as DOT 
   } TExportType;
   
   // Attribute heuristic measures 
   typedef enum {
      ahINFO_GAIN,  // Use information gain as heuristic measure 
      ahGINI_GAIN   // Use Gini gain as heuristic measure 
   } TAttrHeuristic;
   
   // Token type (used for parsing configuration files) 
   typedef enum {
      totDFORMAT,       // 'dataFormat' 
      totDISCRETE,      // 'discrete'
      totNUMERIC,       // 'numeric' 
      totCOLON,         // ':'
      totCOMMA,         // ','
      totEQU,           // '=' 
      totEND,           // End-of-file
      totID,            // identifier 
      totLPARENTHESIS,  // '('
      totRPARENTHESIS,  // ')'
      totSEMIC          // ';'
   } TTokType;

   ///////////////////////////////
   // Token
   struct TToken {
      TToken(const TStr& Val = "", const TTokType& Type = totEND,
         const int& LineN = 0)
         : Val(Val), Type(Type), LineN(LineN) { }
      TStr Val;
      TTokType Type;
      int LineN;
   };

   ///////////////////////////////
   // Simple-Lexical-Analyzer
   class TLexer {
   public:
      TLexer(const TStr& FileNm)
         : LineN(1), BackP(false) { SIn = TFIn::New(FileNm); }
      TLexer(TLexer& Lexer)
         : CrrCh(Lexer.CrrCh), LineN(Lexer.LineN), LastTok(Lexer.LastTok),
            BackP(Lexer.BackP), SIn(Lexer.SIn) { }
      TToken GetNextTok();
      // Need this for lookahead 
      inline void PutBack() {
         BackP = true;
      }
      static bool IsValid(const char& Ch) {
         return TCh::IsAlNum(Ch) || (Ch == '_' || Ch == '-' || Ch == '&' || 
            Ch == '<' || Ch == '>' || Ch == '=' || Ch == '.');
      }
   private:
      void EatWs(); // Eats whitespace 
      void SkipLn(); // Skip line 
      int CrrCh; // Current character 
      int LineN; // Current line number 
      TToken LastTok;
      bool BackP;
      PSIn SIn;
   };

   ///////////////////////////////
   // Learning-Algorithm-Parameters
   class TParams {
   public:
      TParams(const TVec<THash<TStr, TInt> >& AttrsHV_,
         const THash<TStr, TInt>& DataFormatH_,
         const TVec<THash<TInt, TStr> >& InvAttrsHV_,
         const THash<TInt, TStr>& InvDataFormatH_)
         : AttrsHV(AttrsHV_), DataFormatH(DataFormatH_),
            InvAttrsHV(InvAttrsHV_), InvDataFormatH(InvDataFormatH_)
      { }
      TVec<THash<TStr, TInt> > AttrsHV;
      THash<TStr, TInt> DataFormatH;
      TVec<THash<TInt, TStr> > InvAttrsHV;
      THash<TInt, TStr> InvDataFormatH;
   };

   ///////////////////////////////
   // Config-Parsing
   class TParser {
   public:
      typedef enum { ahINFO_GAIN, ahGINI } TAttrHeuristic;
      TParser(const TStr& FNm) {
         CfgParse(FNm);
      }
      TParser(TParser& Parser)
         : AttrsHV(Parser.AttrsHV), DataFormatH(Parser.DataFormatH),
            InvAttrsHV(Parser.InvAttrsHV),
            InvDataFormatH(Parser.InvDataFormatH)
      { }
      inline TParams GetParams() const {
         return TParams(AttrsHV, DataFormatH, InvAttrsHV, InvDataFormatH);
      }

      TVec<THash<TStr, TInt> > AttrsHV;
      THash<TStr, TInt> DataFormatH;
      TVec<THash<TInt, TStr> > InvAttrsHV;
      THash<TInt, TStr> InvDataFormatH;
   private:
      // Parse config file 
      void CfgParse(const TStr& FileNm);
      // dataFormat = (a1, a2, ..., an)
      void InitLine(TLexer& Lexer);
      // Parameter list, i.e., `a1, a2, ..., an'
      void InitParam(TLexer& Lexer);
      // ai: discrete(vi1, vi2, ..., vini) or aj: numeric
      void AttrLine(TLexer& Lexer);
      // Value list, i.e., `vi1, vi2, ..., vini'
      void AttrParam(TLexer& Lexer, const TStr& AttrNm);
      void Error(const TStr& Msg);
   };

   ///////////////////////////////
   // Decision-Tree-Exception
   class TDtExcept : public TExcept {
   public:
      static PExcept New(const TStr& MsgStr, const TStr& LocStr = TStr()) {
         return PExcept(new TDtExcept(MsgStr, LocStr));
      }
   private:
      TDtExcept(const TStr& MsgStr, const TStr& LocStr)
         : TExcept(MsgStr, LocStr)
      { }
   };
   
   ///////////////////////////////
   // Node-and-Bin-ID-Generator
   ClassTP(TIdGen, PIdGen) //{
   public:
      static PIdGen New() { return new TIdGen(); }

      inline int GetNextLeafId() {
         EAssertR(CrrLeafId > 0, "Nonpositive ID in generator.");
         return CrrLeafId++;
      }
      inline int GetNextBinId() {
         EAssertR(CrrBinId > 0, "Nonpositive ID in generator.");
         return CrrBinId++;
      }
      inline int GetNextExampleId() {
         EAssertR(CrrExampleId > 0, "Nonpositive ID in generator");
         return CrrExampleId++;
      }
   private:
      // TODO: We probably want unsigned long long types 
      TIdGen() : CrrLeafId(1), CrrBinId(1), CrrExampleId(1) { };
      int CrrLeafId;
      int CrrBinId;
      int CrrExampleId;
   };

   ///////////////////////////////
   // Probability-Estimation-Functions
   class TProbEstimates {
   public:
      // m-estimate; see PhD thesis [Cestnik, 1991] for details
      static double MEstimate(const int& PositivesN, const int& AllN,
         const double& Apriori, const int& WeightN = 2) {
         EAssertR(PositivesN >= 0 && PositivesN <= AllN,
            "Positives<0 or Positives>AllN");
         // Let r be the number of positive examples and n be the number
         // of all examples. Furthermore let m be the expert-defined
         // parameter (i.e. ``trust'' parameter) and let p0 be apriori
         // probability. Then we define m-estimate as p := (r+m*p0)/(n+m) 
         // p = (r+m*p0)/(n+m);
         return (PositivesN+Apriori*WeightN)/(AllN+WeightN);
      }
      // laplace estimate; see [Kononenko & Robnik-Sikonja, 2010] for details 
      static double LaplaceEstiamte(const int& PositivesN,
         const int& NegativesN, const int& OutcomesN) {
         EAssertR(PositivesN >= 0 && NegativesN >= 0 &&
            OutcomesN >= NegativesN+PositivesN, "Negative count");
         // Let r be the number of positive examples, let n be the number of
         // all examples, and let k be the number of possibles outcomes, i.e.,
         // k=2 for coin tossing example)
         // Then we define Laplace probability estimate as p := (r+1)/(n+k).
         if (OutcomesN > 0) {
            // p = (r+1)/(n+k)
            return 1.0*(PositivesN+1)/(NegativesN+PositivesN+OutcomesN); 
         } else { // in this case, OutcomesN=0
            return 0.0;
         }
      }
      // relative frequency 
      static double RelativeFreq(const int& PositivesN, const int& AllN) {
         EAssertR(PositivesN >= 0 && AllN >= PositivesN,
            "PositivesN<0 or AllN<PositivesN");
         return AllN > 0 ? 1.0*PositivesN/AllN : 0.0;
      }
   };

   ///////////////////////////////
   // Helper-functions
   class TMisc {
   public:
      // SndV = Scalar*FstV + SndV
      static void AddVec(const int& Scalar, TIntV& FstV, TIntV& SndV);
      // N = sum(FreqV)
      static double Entropy(const TIntV& FreqV, const int& N);
      // Compute variance from sufficient statistic: Sum of squared values,
      // sum of values, and number of values
      static double Variance(const double& SqSum, const double& Sum,
         const int& N) {
         EAssertR(N > 1, "Division by zero.");
         // Ubiased variance estimator 
         return SqSum/(N-1)-TMath::Sqr(Sum/(N-1));
      }
      static double StdDev(const double& SqSum, const double& Sum,
         const int& N) {
         EAssertR(N > 0, "Division by zero.");
         return TMath::Sqrt(Variance(SqSum, Sum, N));
      }
   };
   
   ///////////////////////////////
   // Extended binary search tree [Ikonomovska et al., 2011] 
   // Used for finding the best split points of continuous attributes
   // in regression. Memory intensive. (TODO: Consider disabling bad splits.)
   // Left corresponds to <= key; Right corresponds to > key; 
   ClassTP(TExBSTNode, PExBSTNode) // {
   public:
      TExBSTNode(const double& _Key) : Key(_Key), SumLeft(0.0), SumRight(0.0),
         SumSqLeft(0.0), SumSqRight(0.0), CountLeft(0), CountRight(0),
         LeftChild(nullptr), RightChild(nullptr) { } 
      static PExBSTNode New(const double& Key) { return new TExBSTNode(Key); } 
      // Key is the value of the continuous attribute 
      double Key;
      // SumLeft is the sum of target variables of examples with 
      // attribute value <= key; right is similar, but for > key 
      double SumLeft, SumRight;
      // SumSqLeft is the sum of squares of target variables of examples 
      // with attribute value <= key; right is similar, but for > key 
      double SumSqLeft, SumSqRight;
      // CountLeft is the number of example with attribute value <= key; 
      // CountRight is the number of examples with attribute value > key 
      int CountLeft, CountRight;
      PExBSTNode LeftChild, RightChild;
   };
   class TExBST {
   public:
      TExBST() : Root(nullptr), TotalSumLeft(0.0), TotalSumSqLeft(0.0),
         TotalSumRight(0.0), TotalSumSqRight(0.0), TotalCountLeft(0),
         TotalCountRight(0), SplitVal(0.0), MxSDR(0.0) { }
      double GetBestSplit(double& OutVal) {
         Reset();
         FindBestSplit(Root);
         OutVal = SplitVal;
         return MxSDR;
      }
      double ComputeSDR() const {
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
      void Insert(const double& Key, const double& Val);
   private:
      void Reset();
      void FindBestSplit(PExBSTNode Node);
      PExBSTNode Root;
      double TotalSumLeft;
      double TotalSumSqLeft;
      double TotalSumRight;
      double TotalSumSqRight;
      int TotalCountLeft;
      int TotalCountRight;
      double SplitVal;
      double MxSDR;
   };
   
   ///////////////////////////////
   // Bin (for handling numeric attributes) 
   class TBin {
   public:
      TBin(const double& _Value = 0.0, const int& _Id = 0,
         const int& _Count = 0)
         : S(0.0), Mean(0.0), T(0.0), Value(_Value), Count(_Count), Id(_Id)
      { }
      TBin(const TBin& oth)
         : S(oth.S), Mean(oth.Mean), T(oth.T), Value(oth.Value),
            Count(oth.Count), Id(oth.Id), PartitionV(oth.PartitionV)
      { }
      // Note that we MUST NOT assign id=rhs.id because it servers as 
      // a time stamp --- bin must keep its original ID. 
      TBin& operator=(const TBin& rhs) {
         if (this != &rhs) {
            S = rhs.S; Mean = rhs.Mean;
            T = rhs.T; Value = rhs.Value;
            Count = rhs.Count; Id = rhs.Id;
            PartitionV = rhs.PartitionV;
         }
         return *this;
      }
      
      friend bool operator<=(const TBin& Bin1, const TBin& Bin2);
      friend bool operator>=(const TBin& Bin1, const TBin& Bin2);
      friend bool operator<(const TBin& Bin1, const TBin& Bin2);
      friend bool operator>(const TBin& Bin1, const TBin& Bin2);
      friend bool operator==(const TBin& Bin1, const TBin& Bin2);
      friend bool operator!=(const TBin& Bin1, const TBin& Bin2);

      double GetVal() const {
         return Value;
      }
      void Inc(const int& Label) {
         while (Label >= PartitionV.Len()) { PartitionV.Add(0); }
         EAssertR(PartitionV.GetVal(Label) >= 0,
            "Negative partition count.");
         PartitionV.GetVal(Label)++;
         EAssertR(Count >= 0, "Negative count.");
         ++Count;
      }
      void Dec(const int& Label) { // NOTE: Asserts serve debugging purposes 
         EAssertR(Label < PartitionV.Len(), "Label >= PartitionV.Len()");
         EAssertR(PartitionV.GetVal(Label) > 0,
            "Negative partition count in bin.");
         PartitionV.GetVal(Label)--;
         EAssertR(Count > 0, "Negative count.");
         --Count;
      }
      // NOTE: Here, ValueV.Len() is the number of examples in the leaf 
      void Inc(const double& RegValue) { // Regression 
         // ValueV.Add(RegValue);
         EAssertR(Count >= 0, "Negative count.");
         ++Count;
         const double Delta = RegValue - Mean;
         T += RegValue;
         Mean += Delta/Count;
         S += Delta*(RegValue - Mean);
      }
      TBin& operator++() { ++Count; return *this; }
      TBin& operator++(int) { Count++; return *this; }
      double Entropy() const;
      // int operator--() { Assert(Count); return --Count; }
      // int operator--(int) { Assert(Count); return Count--; }
   public:
      TFltV ValueV; // For regression
      // Sum of the squares of element and mean differences
      // S = (x1-mean)^2+(x2-mean)^2+...+(xn-mean)^2
      double S;
      double Mean; // Sample mean 
      double T; // Sum of the sample elements x1+x2+...+xn
      double Value; // Initialize bin with this value 
      int Count;
      int Id; // ID needed for forgetting 
      TIntV PartitionV; // For classification 
   };

   ///////////////////////////////
   // Histogram
   class THist {
   public:
      // Classification
      void IncCls(PExample Example, const int& AttrIdx, PIdGen IdGen);
      // Classification 
      void DecCls(PExample Example, const int& AttrIdx);
      // Regression
      void IncReg(PExample Example, const int& AttrIdx);
      
      // void DecReg(const PExample Example, const int& AttrIdx); // Regression
      double InfoGain(double& SpltVal) const; // Classification 
      double GiniGain(double& SpltVal) const; // Classification 
      double StdGain(double& SpltVal) const; // Regression 
      
      // Make sure counters were reset 
      void Debug_Check() const {
         for (auto It = BinsV.BegI(); It != BinsV.EndI(); ++It) {
            EAssertR(It->Count == 0, "Count>0");
            for (int i = 0; i < It->PartitionV.Len(); ++i) {
               EAssertR(It->PartitionV.GetVal(i) == 0, "Label not OK.");
            }
         }
      }
      
      TBinV BinsV;
      
      void Print() const;
   };

   ///////////////////////////////
   // Attribute-Managment
   class TAttrMan {
   public:
      TAttrMan(const THash<TStr, TInt>& AttrH_ = THash<TStr, TInt>(),
         const THash<TInt, TStr>& InvAttrH_ = THash<TInt, TStr>(),
         const int& Id_ = -1, const TStr& Nm_ = "Anon",
         const TAttrType& Type_ = atDISCRETE);
   public:
      THash<TStr, TInt> AttrH; // Maps attribute value to id 
      THash<TInt, TStr> InvAttrH; // Maps id to attribute value; inverts AttrH 
      TIntV ValueV; // Possible values 
      TAttrType Type; // Attribute type 
      TStr Nm; // Attribute name 
      TInt Id; // Attribute ID used internally 
   };

   ///////////////////////////////
   // Attribute
   class TAttribute {
   public:
      TAttribute(const int& Id_ = -1, const int& Value_ = -1)
         : Id(Id_), Value(Value_), Num(-1) { }
      TAttribute(const int& Id_, const double& Num_)
         : Id(Id_), Value(-1), Num(Num_) { }
      
      inline bool operator==(const TAttribute& Attr) const {
         return (Value != -1 && Value == Attr.Value) || Num == Attr.Num;
      }
      inline bool operator!=(const TAttribute& Attr) const {
         return !(Attr == *this);
      }
      // NOTE: BAD way of combining HASH CODES!!!
      inline int GetPrimHashCd() const {
      return Id.GetPrimHashCd()+Value.GetPrimHashCd()+Num.GetPrimHashCd();
      }
      // NOTE: BAD way of combining HASH CODES!!!
      inline int GetSecHashCd() const {
         return Id.GetSecHashCd()+Value.GetSecHashCd()+Num.GetSecHashCd();
      }
   public:
      TInt Id; // Index in the AttrManV vector 
      TInt Value; // Discrete value; also mapped via hashtable 
      TFlt Num; // Numeric value 
   };

   ///////////////////////////////
   // Training-Example
   ClassTP(TExample, PExample) // {
   public:
      static PExample New() { return new TExample(); }
      static PExample New(const TAttributeV& AttributesV, const int& Label) {
         return new TExample(AttributesV, Label);
      }
      static PExample New(const TAttributeV& AttributesV, const double& Val) {
         return new TExample(AttributesV, Val);
      }

      TExample() : LeafId(0), BinId(0), Id(0), Label(-1), Value(0) { }
      TExample(const TAttributeV& AttributesV_, const int& Label_)
         : LeafId(0), BinId(0), Id(0), AttributesV(AttributesV_),
            Label(Label_), Value(0) { }
      TExample(const TAttributeV& AttributesV_, const double& Value_)
         : LeafId(0), BinId(0), Id(0), AttributesV(AttributesV_), Label(-1),
            Value(Value_) { }
      TExample(const TExample& Example_)
         : LeafId(Example_.LeafId), BinId(Example_.BinId), Id(Example_.Id),
            AttributesV(Example_.AttributesV), Label(Example_.Label),
            Value(Example_.Value) { }
      
      TExample& operator=(const TExample& Example);
      
      // TODO: Harmful? Solve this another way? May not be what user expects?
      inline bool operator<(const TExample& Example) const {
         return Label < Example.Label;
      }
      inline bool operator==(const TExample& Example) const {
         return LeafId == Example.LeafId && BinId == Example.BinId &&
            Id == Example.Id && AttributesV == Example.AttributesV &&
            Label == Example.Label && Value == Example.Value;
      }
      inline bool operator!=(const TExample& Example) const {
         return !(*this == Example);
      }
      inline void SetLeafId(const int& LeafId_) { LeafId = LeafId_; }
      inline void SetBinId(const int& BinId_) { BinId = BinId_; }
      inline void SetId(const int& Id_) { Id = Id_; }
      // NOTE: BAD way of combining HASH CODES!!!
      inline int GetPrimHashCd() const {
         return LeafId.GetPrimHashCd()+BinId.GetPrimHashCd()+
            Id.GetPrimHashCd()+AttributesV.GetPrimHashCd()+
            Label.GetPrimHashCd()+Value.GetPrimHashCd();
      }
      // NOTE: BAD way of combining HASH CODES!!!
      inline int GetSecHashCd() const {
         return LeafId.GetSecHashCd()+BinId.GetSecHashCd()+
            Id.GetSecHashCd()+AttributesV.GetSecHashCd()+
            Label.GetSecHashCd()+Value.GetSecHashCd();
      }
      // Split on Separator and map to integers using hashtables 
      // TExample(const TStr& Line, const TCh& Separator); 
   public:
      TInt LeafId;
      TInt BinId;
      TInt Id;
      TAttributeV AttributesV;
      TLabel Label; // Classification only 
      TFlt Value; // Regression only 
   };

   ///////////////////////////////
   // Node
   ClassTP(TNode, PNode) // { 
      friend class THoeffdingTree;
   public:
      static PNode New(const int& LabelsN = 2,
         const TIntV& UsedAttrs = TVec<TInt>(), const int& Id = 0,
         const TNodeType& Type = ntLEAF) {
         return new TNode(LabelsN, UsedAttrs, Id, Type);
      }
      static PNode New(const int& LabelsN, const TIntV& UsedAttrV,
         const TAttrManV& AttrManV, const int& Id,
         const TNodeType& Type = ntLEAF) {
         return new TNode(LabelsN, UsedAttrV, AttrManV, Id, Type);
      }

      // TODO: Initialize PartitionV class label distribution counts 
      TNode(const int& LabelsN = 2, const TIntV& UsedAttrs_ = TIntV(),
         const int& Id_ = 0, const TNodeType& Type_ = ntLEAF)
         : CndAttrIdx(-1), ExamplesN(0), Avg(0), VarSum(0), 
         Err(0), TestModeN(0), Type(Type_), UsedAttrs(UsedAttrs_), Id(Id_),
            Correct(0), All(0) {
         PartitionV.Reserve(LabelsN, LabelsN);
      }
      TNode(const int& LabelsN, const TIntV& UsedAttrs_,
         const TAttrManV& AttrManV, const int& Id_, const TNodeType& Type_)
         : CndAttrIdx(-1), ExamplesN(0), Avg(0), VarSum(0), 
            Err(0), TestModeN(0), Type(Type_), UsedAttrs(UsedAttrs_),
            Id(Id_), Correct(0), All(0) {
         PartitionV.Reserve(LabelsN, LabelsN); Init(AttrManV);
      }
      TNode(const TNode& Node);
      // TNode(TNode&& Node);

      ~TNode() { Clr(); }

      TNode& operator=(const TNode& Node);
      // TNode& operator=(const TNode&& Node);
      
      bool operator==(const TNode& Node) const;
      bool operator!=(const TNode& Node) const { return !(*this == Node); }
      
      int GetExamplesN() const { return ExamplesN; }
      double ComputeEntropy() const;
      double ComputeGini() const;
      // Classification 
      double InfoGain(const int& AttrIdx,
         const TVec<TAttrMan>& AttrManV) const;
      // Classification 
      double GiniGain(const int& AttrIdx,
         const TVec<TAttrMan>& AttrManV) const;
      // Regression  
      double StdGain(const int& AttrIdx, const TVec<TAttrMan>& AttrManV) const;
      double ComputeTreshold(const double& Delta, const int& LabelsN) const;
      // Split the leaf on the AttrIdx attribute; return the number of
      // newly created leaves 
      void Split(const int& AttrIdx, const TAttrManV& AttrManV, PIdGen IdGen);
      void Clr();
      TBstAttr BestAttr(const TAttrManV& AttrManV,
         const TTaskType& TaskType = ttCLASSIFICATION);
      TBstAttr BestRegAttr(const TAttrManV& AttrManV); // Regression 
      // Classification 
      TBstAttr BestClsAttr(const TAttrManV& AttrManV,
         const TIntV& BannedAttrV = TVec<TInt>());
      void UpdateStats(PExample Example); // Regression 
      inline double Std() const {
         // NOTE: Unbiased variance estimator is VarSum/(ExamplesN-1)
         EAssertR(ExamplesN > 1, "Division by zero.");
         return TMath::Sqrt(VarSum/(ExamplesN-1));
      }
      inline double Mean() const {
         return Avg;
      }
      void UpdateErr(const double& Loss, const double& Alpha);
   // private:
   public:
      void Init(const TAttrManV& AttrManV);
      // Attribute this node tests on; this is set to -1 in leaf nodes 
      int CndAttrIdx;
      // Count the number of examples we accumulated so
      // far (needed for GracePeriod parameter) 
      int ExamplesN; 
      double Val; // Test for `numerical attribute' <= Val 
      double Avg; // Current mean (regression only)
      double VarSum;
      double Err; // Current error 
      int TestModeN; // Testing mode 
      TNodeType Type; // Root, Internal, Leaf 
      TVec<PExample> ExamplesV;
      // TIntV IdxV; // Sacrificed example indices 
      THash<TExample, TInt> SeenH; // Examples sacrificed for self-evaluation 
      // sufficient statistics; <AttributeID, AttributeValue, Class> 
      THash<TTriple<TInt, TInt, TInt>, TInt> Counts;
      TIntV PartitionV; // Number of examples with the same label 
      TVec<PNode> ChildrenV; // Vector of children (root nodes of the subtrees)
      TIntV UsedAttrs; // Attributes we already used in predecessor nodes 
      // For each numeric attribute; maps attribute index to histogram
      THash<TInt, THist> HistH;
      // Maps numeric attributes to binary search trees 
      THash<TInt, TExBST> BstH;
      // The following are used for time-changeable decision trees 
      TVec<PNode> AltTreesV; // Vector of alternate trees (their root nodes) 
      // Monotonically increasing ID, assigned to each node at creation 
      int Id;
      int Correct;
      int All;
   };

   ///////////////////////////////
   // Hoeffding-Tree
   ClassTP(THoeffdingTree, PHoeffdingTree) // {
   public:
      THoeffdingTree(const TStr& ConfigNm_, const int& GracePeriod_,
         const double& SplitConfidence_, const double& TieBreaking_,
         const int& DriftCheck_ = 100, const int& WindowSize_ = 10000,
         const bool& IsAlt_ = false, PIdGen IdGen_ = nullptr)
         : ExportN(0), TieBreaking(TieBreaking_),
            SplitConfidence(SplitConfidence_), GracePeriod(GracePeriod_),
            DriftCheck(DriftCheck_), WindowSize(WindowSize_), IsAlt(IsAlt_),
            BinsN(1000), MxId(1), AltTreesN(0), DriftExamplesN(0),
            IdGen(IdGen_), ConceptDriftP(true), MxNodes(0),
            RegressLeaves(rlMEAN), ClassifyLeaves(clMAJORITY) {
         if (IdGen() == nullptr) { IdGen = TIdGen::New(); }
         Init(ConfigNm_);
      }
      THoeffdingTree(PJsonVal JsonConfig_, const int& GracePeriod_,
         const double& SplitConfidence_, const double& TieBreaking_,
         const int& DriftCheck_ = 100, const int& WindowSize_ = 10000,
         const bool& IsAlt_ = false, PIdGen IdGen_ = nullptr)
         : ExportN(0), TieBreaking(TieBreaking_),
            SplitConfidence(SplitConfidence_),
            GracePeriod(GracePeriod_), DriftCheck(DriftCheck_), 
            WindowSize(WindowSize_), IsAlt(IsAlt_), BinsN(1000),
            MxId(1), AltTreesN(0), DriftExamplesN(0), IdGen(IdGen_), 
            ConceptDriftP(true), MxNodes(0), RegressLeaves(rlMEAN),
            ClassifyLeaves(clMAJORITY) {
         if (IdGen() == nullptr) { IdGen = TIdGen::New(); }
         Init(JsonConfig_);
      }
      THoeffdingTree(PJsonVal JsonConfig_, PJsonVal JsonParams_,
         const bool& IsAlt_ = false, PIdGen IdGen_ = nullptr)
         : ExportN(0), IsAlt(IsAlt_), BinsN(1000), MxId(1),
            AltTreesN(0), DriftExamplesN(0), IdGen(IdGen_),
            ConceptDriftP(true), MxNodes(0), RegressLeaves(rlMEAN),
            ClassifyLeaves(clMAJORITY) {
         if (IdGen() == nullptr) { IdGen = TIdGen::New(); }
         // NOTE: SetParams() must execute BEFORE Init() to
         // initialize the paramters
         SetParams(JsonParams_); Init(JsonConfig_);
      }

      static PHoeffdingTree New(const TStr& ConfigNm, const int& GracePeriod,
         const double& SplitConfidence, const double& TieBreaking,
         const int& DriftCheck = 100, const int& WindowSize = 10000,
         const bool& IsAlt = false, PIdGen IdGen = nullptr) {
         return new THoeffdingTree(ConfigNm, GracePeriod, SplitConfidence,
            TieBreaking, DriftCheck, WindowSize, IsAlt, IdGen);
      }
      static PHoeffdingTree New(PJsonVal JsonConfig, const int& GracePeriod,
         const double& SplitConfidence, const double& TieBreaking,
         const int& DriftCheck = 100, const int& WindowSize = 10000,
         const bool& IsAlt = false, PIdGen IdGen = nullptr) {
         return new THoeffdingTree(JsonConfig, GracePeriod, SplitConfidence,
            TieBreaking, DriftCheck, WindowSize, IsAlt, IdGen);
      }
      static PHoeffdingTree New(PJsonVal JsonConfig, PJsonVal JsonParams,
         const bool& IsAlt = false, PIdGen IdGen = nullptr) {
         return new THoeffdingTree(JsonConfig, JsonParams, IsAlt, IdGen);
      }

      // TODO: Define copy and move constructors and
      // operators for THoeffdingTree?
      // THoeffdingTree(const THoeffdingTree& HoeffdingTree) =delete;
      // THoeffdingTree(THoeffdingTree&& HoeffdingTree) =delete;
      // THoeffdingTree& operator=(
      //   const THoeffdingTree& HoeffdingTree) =delete;
      // THoeffdingTree& operator=(THoeffdingTree&& HoeffdingTree) =delete;
      
      double Predict(const TStrV& DiscreteV, const TFltV& NumericV) const;
      double Predict(PExample Example) const;
      double Predict(const TStr& Line, const TCh& Delimiter = ',') const {
         return Predict(Preprocess(Line, Delimiter));
      }
      TStr Classify(PNode Node, PExample Example) const;
      TStr Classify(const TStrV& DiscreteV, const TFltV& NumericV) const;
      TStr Classify(PExample Example) const;
      TStr Classify(const TStr& Line, const TCh& Delimiter = ',') const {
         if (Line.CountCh(Delimiter) < AttrsHashV.Len()) { // Missing label 
            TStr Label = InvAttrsHashV.Last()[0];
            return Classify(Preprocess(Line+","+Label, Delimiter));
         } else {
            return Classify(Preprocess(Line, Delimiter));
         }
      }
      void IncCounts(PNode Node, PExample Example) const; // Classification 
      void DecCounts(PNode Node, PExample Example) const; // Classification 
      bool IsAltSplitIdx(PNode Node, const int& AttrIdx) const;
      void CheckSplitValidityCls();
      void ForgetCls(PExample Example) const; // Classification 
      void ProcessLeafReg(PNode Leaf, PExample Example); // Regression 
      void Debug_Finalize();
      void Debug_CheckInvariant(PExample Example) const; 
      void ProcessLeafCls(PNode Leaf, PExample Example); // Classification 
      void SelfEval(PNode Node, PExample Example) const;
      bool TestMode(PNode Node);
      // Regression 
      void Process(const TStrV& DiscreteV, const TFltV& NumericV,
         const double& Val);
      // Classification 
      void Process(const TStrV& DiscreteV, const TFltV& NumericV,
         const TStr& Label);
      // Both (classification and regression) 
      void Process(const TStr& Line, const TCh& Delimiter = ',') {
         Process(Preprocess(Line, Delimiter));
      }
      inline void Process(PExample Example) {
         switch(TaskType) {
         case ttCLASSIFICATION:
            ProcessCls(Example);
            break;
         case ttREGRESSION:
            ProcessReg(Example);
            break;
         default:
            EFailR("Invalid TaskType");
         }
      }
      void ProcessCls(PExample Example); // Classification 
      void ProcessReg(PExample Example); // Regression 
      PExample Preprocess(const TStr& Line, const TCh& Delimiter = ',') const;
      PNode GetNextNode(PNode Node, PExample Example) const; 
      void Clr(PNode Node, PNode SubRoot = nullptr);
      void Export(const TStr& FileNm,
         const TExportType& ExportType = etXML) const;
      TLabel NaiveBayes(PNode Node, PExample Example) const; // Classification 
      inline TLabel Majority(PNode Node) const { // Classification 
         return Node->PartitionV.GetMxValN();
      }
      inline TStr GetNodeNm(PNode Node) const {
         return AttrManV.GetVal(Node->CndAttrIdx).Nm.CStr();
      }
      inline TStr GetNodeValueNm(PNode Node, const int& ChildN) const {
         EAssertR(Node->CndAttrIdx >= 0 &&
            Node->CndAttrIdx < AttrManV.Len()-1,
            "Attribute index out of bouds.");
         EAssertR(ChildN >= 0, "Can't have negative number of children.");
         return AttrManV.GetVal(Node->CndAttrIdx).InvAttrH.GetDat(ChildN);
      }
      inline TStr GetMajorityNm(PNode Node) const { // Classification 
         return AttrManV.GetVal(AttrManV.Len()-1).InvAttrH.GetDat(
            Node->PartitionV.GetMxValN());
      }
      static bool IsLeaf(PNode Node) { return Node->CndAttrIdx == -1; }
      void PrintHist(const TStr& FNm, const TCh& Ch = '#') const;
      // Print example in human-readable form 
      void Print(PExample Example) const; 
      void SetAdaptive(const bool& DriftP) { ConceptDriftP = DriftP; }
      bool IsAdaptive() const { return ConceptDriftP; }
      static bool Sacrificed(PNode Node, PExample Example) {
         return Node->SeenH.IsKey(*Example) &&
            Node->SeenH.GetDat(*Example) > 0;
      }
      // Get the number of nodes in the tree 
      // If AltP=false, do not count alterante trees 
      int GetNodesN(const bool& AltP = false) const;
      // NOTE: Not implemented yet; avoids using hash tables
      // for self-evaluation 
      //void UpdateIndices(PNode Node) const {
      //   for(int IdxN = 0; IdxN < Node->IdxV.Len(); ++IdxN) {
      //      if(Node->IdxV[IdxN]++ >= WindowSize) { /* throw it out */
      //         Node->IdxV.Del(IdxN); break;
      //      }
      //   }
      //}
   public:
      PNode Root; // Root node 
      THash<TStr, TInt> LabelH; // Maps label to integer 
      THash<TInt, TStr> InvLabelH; // Maps integer to label 
      TVec<THash<TStr, TInt> > AttrsHashV; // Vector of attribute hash tables 
      // Vector of attribute inverse hash tables 
      TVec<THash<TInt, TStr> > InvAttrsHashV; 
      TAttrManV AttrManV; // Attribute managment 
      int ExportN;
   private:
      double TieBreaking; // tau; When to consider two attributes equally good 
      // delta; NOTE: This is actually error tolerance;
      // Condifdence would be 1-`SplitConfidence'
      double SplitConfidence; 
      // Recompute attribute heuristic estimates every GracePeriod examples 
      int GracePeriod;
      int DriftCheck; // Check for drift every `DriftCheck' examples 
      int WindowSize; // Keep `WindowSize' examples in main memory 
      // int MemoryConstraint; // Memory contraints (?) 
      // Alternate trees are not allowed to grow altenrate trees 
      const bool IsAlt;
      // Number of bins when apprximating numberic attribute's value
      // Distribution with histogram 
      const int BinsN;
      int MxId;
      TTaskType TaskType;
      int AltTreesN; // Number of alternate trees 
      TQQueue<PExample> ExampleQ; // Last W examples 
      int DriftExamplesN; // Examples since last drift check 
      PIdGen IdGen; // ID generator 
      bool ConceptDriftP;
      int MxNodes; // The max allowed size of the tree 
      // TAttrHeuristic AttrHeuristic; // Heuristic measure 
      // Leaf model in regression trees
      TRegressLeaves RegressLeaves;
      // Leaf model in classification trees
      TClassifyLeaves ClassifyLeaves;
   private:
      // Initialize attribute managment classes 
      void Init(const TStr& ConfigFNm);
      // Initialize attribute managment classes 
      void Init(PJsonVal JsonConfig); 
      void SetParams(PJsonVal JsonParams); // Accepts JSON paramters 
      void InitAttrMan();
      // Export decision tree to XML 
      void PrintXML(PNode Node, const int& Depth,
         TFOut& FOut) const; 
      void PrintJSON(PNode Node, const int& Depth, TFOut& FOut) const;
      void PrintDOT(PNode Node, TFOut& FOut,
         const bool& AlternateP = false) const;
      static void Print(const TCh& Ch = '-', const TInt& Num = 80);
   };
}

#endif // HOEFFDING_H 


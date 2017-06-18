#include <algorithm>
#include <iostream>

namespace TQuant {

    std::ostream& operator <<(std::ostream& os, const TGkTuple& Tup) {
        os << '<' << Tup.GetVal() << ", " << Tup.GetTupleSize() << ", " << Tup.GetMnMxRankDiff() << '>';
        return os;
    }

    std::ostream& operator <<(std::ostream& os, const TUInt& Val) {
        return os << Val.Val;
    }

    TGkTuple::TGkTuple() {}

    TGkTuple::TGkTuple(const double& _Val, const uint& _TupleSize, const uint& Delta):
            Val(_Val),
            TupleSize(_TupleSize),
            MnMxRankDiff(Delta) {
        AssertR(Delta <= TUInt(TInt::Mx), "Invalid value of Delta: " + TUInt::GetStr(Delta));
    }

    void TGkTuple::Swallow(const TGkTuple& Tuple) {
        TupleSize += Tuple.GetTupleSize();
    }

    TGk::TGreenwaldKhanna(const double& _Eps):
            Eps(_Eps) {}

    TGk::TGreenwaldKhanna(const double& _Eps, const TCompressStrategy& Cs):
            Eps(_Eps),
            CompressStrategy(Cs) {}

    double TGk::Query(const double& Quantile) const {
        Assert(0.0 < Quantile && Quantile < 1);

        const int TargetRank = std::ceil(Quantile * SampleN);
        const int HalfErrRange = Eps * SampleN;

        const int SummarySize = GetSummarySize();

        // these are here to find the range of quantiles which match the query
        // and then choose the median of those
        int FirstHitN = -1;
        int LastHitN = -1;

        int CurrMnRank = 0;
        for (int TupleN = 0; TupleN < SummarySize; TupleN++) {
            const TTuple& CurrTuple = Summary[TupleN];

            const int MnRank = CurrMnRank + GetPrevMnRankDiff(CurrTuple);
            const int MxRank = MnRank + GetInsRankRange(CurrTuple);

            const int UpperBound = MnRank + HalfErrRange;
            const int LowerBound = MxRank - HalfErrRange;

            const bool IsHit = LowerBound <= TargetRank && TargetRank <= UpperBound;

            // check if this is the first hit
            if (IsHit && FirstHitN == -1) {
                FirstHitN = TupleN;
            }
            // check if we are one past the last hit
            if (!IsHit && FirstHitN != -1) {
                LastHitN = TupleN - 1;
                break;
            }

            CurrMnRank = MnRank;
        }

        // if we had not found a quantile which matches, then we return the final quantile
        if (FirstHitN == -1) { FirstHitN = SummarySize-1; }
        // if we have not finished the final quantile, then we finish it with the last tuple
        if (LastHitN == -1) { LastHitN = SummarySize-1; }

        const int QuantN = (FirstHitN + LastHitN) / 2;
        const TTuple& Tuple = Summary[QuantN];

        return GetQuantile(Tuple);
    }

    void TGk::Insert(const double& Val) {
        // binary search to find the first tuple with value greater than val
        // this is where we will insert the new value
        const auto Cmp = [&](const TTuple& Tup, const double& Val) { return Tup.Val1 < Val; };
        const auto ValIt = std::lower_bound(Summary.begin(), Summary.end(), Val, Cmp);

        const int NewValN = ValIt - Summary.begin();

        // if Val is the smallest or largest element, we must insert <v,1,0>
        if (NewValN == 0 || NewValN == Summary.Len()) {
            Summary.Ins(NewValN, TTuple(Val, 1, 0));
        }
        else {
            // insert tuple <v,1,floor(2*eps*n)>
            Summary.Ins(NewValN, TTuple(Val, 1, (int) (2*Eps*SampleN)));
        }

        ++SampleN;

        if (ShouldAutoCompress()) {
            Compress();
        }
    }

    void TGk::Compress() {
        const int ErrRange = 2*Eps*SampleN;

        // go throught the tuples and merge the ones you can
        // the first and last tuple should never get merged
        for (int TupleN = Summary.Len() - 3; TupleN >= 1; TupleN--) {
            // delete all descendants of i and i itself
            // the descendants of i are the ones with Band_k < Band_i and
            // the children of i are the set {k_1,...,k_m | k_{j+1}=k_j+1,band(k_j)>=band(k_{j+1}),band(i)>band(k_1)}
            const TTuple& CurrTuple = Summary[TupleN];
            const TTuple& NextTuple = Summary[TupleN+1];

            if (UseBands && GetBand(TupleN) > GetBand(TupleN+1)) { continue; }

            if (GetPrevMnRankDiff(CurrTuple) + GetPrevMnRankDiff(NextTuple) + GetInsRankRange(NextTuple) <= ErrRange) {
                // merge the two tuples
                const int CurrPrevMnRankDiff = GetPrevMnRankDiff(CurrTuple);
                Summary.Del(TupleN);
                GetPrevMnRankDiff(Summary[TupleN]) += CurrPrevMnRankDiff;
            }
        }
    }

    int TGk::GetSummarySize() const {
        return Summary.Len();
    }

    void TGk::PrintSummary() const {
        std::cout << Summary << "\n";
    }

    bool TGk::ShouldAutoCompress() const {
        return CompressStrategy == TCompressStrategy::csAuto &&
               SampleN % int(ceil(1.0 / (2.0*Eps))) == 0;
    }

    int TGk::GetBand(const int& TupleN) const {
        // the band groups tuples by their capacity
        // it is defined as alpha, such that:
        // 2^(alpha-1) + mod(p,2^(alpha-1)) <= capacity < 2^alpha + mod(p,2^alpha)
        // where capacity is defined as floor(2*eps*n) - delta_i
        const TTuple& Tuple = Summary[TupleN];

        const uint64 p = (int) 2*Eps*SampleN;
        const uint64 DeltaI = Tuple.Val3;
        const uint64 Capacity = p - DeltaI;

        // Special case: the first 1 / (2*eps) tuples (with d_i == 0) are in a band of their own
        if (DeltaI == 0) { return TInt::Mx; }
        // Special case: we define band 0 to simply be p
        if (DeltaI == p) { return 0; }

        const auto TestBand = [&](const int& Band) {
            return TMath::Pow2(Band-1) + (p % TMath::Pow2(Band-1)) <= Capacity &&
                   Capacity < TMath::Pow2(Band) + (p % TMath::Pow2(Band));
        };

        const int CandidateBand = (int) TMath::Log2(Capacity);
        if (TestBand(CandidateBand)) {
            return CandidateBand;
        }
        else if (TestBand(CandidateBand+1)) {
            return CandidateBand+1;
        }
        else {
            // fail horribly and think about what you did
            const TStr MsgStr = "Could not find the band of tuple: <" + TFlt::GetStr(Tuple.Val1) + "," + TInt::GetStr(Tuple.Val2) + "," + TInt::GetStr(Tuple.Val3) + ">, p=" + TInt::GetStr(p) + ", candidate band = " + TInt::GetStr(CandidateBand) + "!";
            FailR(MsgStr.CStr());
            return -1;
        }
    }

    TFlt& TGk::GetQuantile(TTuple& Tuple) {
        return Tuple.Val1;
    }

    const TFlt& TGk::GetQuantile(const TTuple& Tuple) {
        return Tuple.Val1;
    }

    TInt& TGk::GetPrevMnRankDiff(TTuple& Tuple) {
        return Tuple.Val2;
    }

    const TInt& TGk::GetPrevMnRankDiff(const TTuple& Tuple) {
        return Tuple.Val2;
    }

    TInt& TGk::GetInsRankRange(TTuple& Tuple) {
        return Tuple.Val3;
    }

    const TInt& TGk::GetInsRankRange(const TTuple& Tuple) {
        return Tuple.Val3;
    }

    //////////////////////////////////////////
    /// The CKMS (generalization of GK) algorithm for online
    /// biased quantile estimation.
    TBiasedGk::TBiasedGk(const double& _Quant0, const double& _Eps, const bool& _UseBands):
            Quant0(_Quant0),
            Eps(_Eps),
            UseBands(_UseBands) {

        EAssert(Eps > 0.0);
        EAssert(0 <= Quant0 && Quant0 <= 1);

        if (Quant0 > .5) {
            Quant0 = 1 - Quant0;
            Dir = -1;
        }
    }

    double TBiasedGk::Query(const double& Quantile) const {
        if (GetSummarySize() == 0) { return 0; }

        const double TargetRank = Dir > 0 ? Quantile*SampleN : (1 - Quantile)*SampleN;
        const double EpsRank = GetMxTupleSize(TargetRank) / 2;

        if (TargetRank <= 1) { return Summary[0].GetVal(); }

        int CurrMnRank = 0;
        for (int TupleN = 0; TupleN < Summary.Len(); TupleN++) {
            const TGkTuple& Tuple = Summary[TupleN];
            const uint64 MxRank = CurrMnRank + Tuple.GetTupleSize() + Tuple.GetMnMxRankDiff();

            if (MxRank > TargetRank + EpsRank) {
                return Summary[TupleN-1].GetVal();
            }

            CurrMnRank += Tuple.GetTupleSize();
        }

        return Summary.Last().GetVal();
    }

    void TBiasedGk::Insert(const double& Val) {
        int TupleN = 0;
        uint64 PrevTupleMnRank = 0;

        while (TupleN < Summary.Len()) {
            const TGkTuple& Tuple = Summary[TupleN];
            if (Dir*(Val - Tuple.GetVal()) <= 0) { break; }
            PrevTupleMnRank += Tuple.GetTupleSize();
            TupleN++;
        }

        if (TupleN == 0) {
            // special case
            Summary.Ins(0, TGkTuple(Val, 1, 0));
        } else if (TupleN == Summary.Len()) {
            // special case (symmetrical to the previous case)
            Summary.Add(TGkTuple(Val, 1, 0));
        } else {
            const double NewTupleMxSize = GetMxTupleSize(PrevTupleMnRank);
            Summary.Ins(TupleN, TGkTuple(Val, 1, std::floor(NewTupleMxSize-1)));
            AssertR(NewTupleMxSize-1 >= 0, "Max tuple size is 0, but should be at least 1!");
        }

        ++SampleN;

        if (ShouldCompress()) {
            Compress();
        }
    }

    void TBiasedGk::Compress() {
        if (Summary.Empty()) { return; }

        uint64 CurrMnRank = Summary[0].GetTupleSize();
        for (int TupleN = 1; TupleN < Summary.Len()-1; TupleN++) {
            const TGkTuple& CurTuple = Summary[TupleN];
            const TGkTuple& AdjTuple = Summary[TupleN+1];

            const uint64 PrevMnRank = CurrMnRank;

            CurrMnRank += CurTuple.GetTupleSize();

            const uint NewTupleRange = CurTuple.GetTupleSize() +
                                       AdjTuple.GetTupleSize() +
                                       AdjTuple.GetMnMxRankDiff();
            const double MxTupleRange = GetMxTupleSize(PrevMnRank);

            if (UseBands) {
                const int CurTupleBand = GetBand(CurTuple, CurrMnRank);
                const int AdjTupleBand = GetBand(AdjTuple, CurrMnRank + AdjTuple.GetTupleSize());
                if (CurTupleBand > AdjTupleBand) {
                    continue;
                }
            }

            if (NewTupleRange <= MxTupleRange) {
                // merge the tuples
                Summary[TupleN+1].Swallow(Summary[TupleN]);
                Summary.Del(TupleN);
                TupleN--;
                continue;
            }
        }
    }

    int TBiasedGk::GetSummarySize() const {
        return Summary.Len();
    }

    void TBiasedGk::PrintSummary() const {
        std::cout << Summary << "\n";
    }

    double TBiasedGk::GetEps(const double& Quantile) const {
        if (Quantile <= Quant0) {
            return Eps * Quant0 / Quantile;
        } else {
            return Eps;
        }
    }

    double TBiasedGk::GetMxTupleSize(const double& Rank) const {
        return TMath::Mx(1.0, 2.0*GetEps(Rank / SampleN)*Rank);
    }

    bool TBiasedGk::ShouldCompress() const {
        return SampleN % uint64(std::ceil(0.5 / Eps)) == 0;    // TODO set some smart compress strategy
    }

    int TBiasedGk::GetBand(const TGkTuple& Tuple, const uint64& MnRank) const {
        if (Tuple.GetMnMxRankDiff() == 0) { return TInt::Mx; }

        const uint MxTupleSize = GetMxTupleSize(MnRank);
        const uint TupleDelta = Tuple.GetMnMxRankDiff();
        const uint Capacity = MxTupleSize - TupleDelta;
        AssertR(MxTupleSize >= TupleDelta, "Tuple uncertainty greater than capacity!");

        if (Capacity == 0) { return 0; }

        const auto TestBand = [&](const int& Band) {
            return TMath::Pow2(Band-1) + (MxTupleSize % TMath::Pow2(Band-1)) <= Capacity &&
                   Capacity < TMath::Pow2(Band) + (MxTupleSize % TMath::Pow2(Band));
        };

        const int CandidateBand = (int) TMath::Log2(Capacity);
        if (TestBand(CandidateBand)) {
            return CandidateBand;
        }
        else if (TestBand(CandidateBand+1)) {
            return CandidateBand+1;
        }
        else {
            // fail horribly and think about what you did
            PrintSummary();
            const TStr MsgStr = "Could not find the band of tuple: <" +
                TFlt::GetStr(Tuple.GetVal()) + "," + TUInt::GetStr(Tuple.GetTupleSize()) + "," +
                TUInt::GetStr(Tuple.GetMnMxRankDiff()) + ">, max size=" + TInt::GetStr(MxTupleSize) +
                ", max tuple size: " + TUInt::GetStr(MxTupleSize) +
                ", delta: " + TUInt::GetStr(TupleDelta) +
                ", capacity: " + TUInt::GetStr(Capacity) +
                ", candidate band = " + TInt::GetStr(CandidateBand) + "!";
            FailR(MsgStr.CStr());
            return -1;
        }
    }

    TInterval::TInterval(): TInterval(0ul) {}

    TInterval::TInterval(const uint64& _StartTm):
            StartTm(_StartTm) {}

    TInterval::TInterval(const uint64& _StartTm, const uint64& _Dur, const uint& _Count):
            StartTm(_StartTm),
            DurMSec(_Dur),
            ElCount(_Count) {}

    uint64 TInterval::GetEndTm() const {
        return StartTm + DurMSec;
    }

    void TInterval::Swallow(const TInterval& Other) {
        Assert(StartTm <= Other.StartTm);

        DurMSec = Other.DurMSec +  Other.StartTm - StartTm;
        ElCount <<= 1;  // multiply by 2
    }

    ////////////////////////////////////////////
    /// Approximate interval with maximum
    TIntervalWithMax::TIntervalWithMax() : TIntervalWithMax(0, 0.0) {}

    TIntervalWithMax::TIntervalWithMax(const uint64& BeginTm, const double& Val):
        TInterval(BeginTm),
        MxVal(Val) {}

    TIntervalWithMax::TIntervalWithMax(const uint64& _StartTm, const uint64& _Dur, const uint& _Cnt,
            const double& _MxVal):
        TInterval(_StartTm, _Dur, _Cnt),
        MxVal(_MxVal) {}

    void TIntervalWithMax::Swallow(const TIntervalWithMax& Other) {
        TInterval::Swallow(Other);
        if (Other.MxVal > MxVal) {
            MxVal = Other.MxVal;
        }
    }

    ////////////////////////////////////////////
    /// Approximate interval with minimum
    TIntervalWithMin::TIntervalWithMin() : TIntervalWithMin(0, 0.0) {}

    TIntervalWithMin::TIntervalWithMin(const uint64& StartTm, const double& Val):
        TInterval(StartTm),
        MnVal(Val) {}

    TIntervalWithMin::TIntervalWithMin(const uint64& _StartTm, const uint64& _Dur, const uint& _Cnt,
            const double& _MnVal):
        TInterval(_StartTm, _Dur, _Cnt),
        MnVal(_MnVal) {}

    void TIntervalWithMin::Swallow(const TIntervalWithMin& Other) {
        TInterval::Swallow(Other);
        if (Other.MnVal < MnVal) {
            MnVal = Other.MnVal;
        }
    }


    // print operators
    std::ostream& operator <<(std::ostream& os, const TInterval& Interval) {
        return os << "<"
                  << Interval.GetStartTm() << ", "
                  << Interval.GetDurMSec() << ", "
                  << Interval.GetCount()
                  << ">";
    }

    std::ostream& operator <<(std::ostream& os, const TIntervalWithMax& Interval) {
        return os << "<"
           << Interval.GetStartTm()
           << ", " << Interval.GetDurMSec()
           << ", " << Interval.GetCount()
           << ", " << Interval.GetMxVal()
           << ">";
    }

    std::ostream& operator <<(std::ostream& os, const TIntervalWithMin& Interval) {
        return os << "<"
           << Interval.GetStartTm()
           << ", " << Interval.GetDurMSec()
           << ", " << Interval.GetCount()
           << ", " << Interval.GetMnVal()
           << ">";
    }

    std::ostream& operator <<(std::ostream& os, const TUIntUInt64Pr& Pr) {
        return os << "<" << Pr.Val1.Val << ", " << Pr.Val2.Val << ">";
    }

    ///////////////////////////////////////////
    /// Exponential Histogram
    void TExpHistogram::Add(const uint64& Tm) {
        TBase::Add(TInterval(Tm));
    }

    ///////////////////////////////////////////
    /// Exponential Histogram with maximum
    TExpHistWithMax::TExpHistWithMax(const TExpHistWithMax& Other):
            TBase(Other),
            MxVal(Other.MxVal) {}

    TExpHistWithMax& TExpHistWithMax::operator =(const TExpHistWithMax& Other) {
        TBase::operator =(Other);
        MxVal = Other.MxVal;
        return *this;
    }

    TExpHistWithMax::TExpHistWithMax(TExpHistWithMax&& Other):
            TBase(Other),
            MxVal(Other.MxVal) {}

    TExpHistWithMax& TExpHistWithMax::operator =(TExpHistWithMax&& Other) {
        TBase::operator =(Other);
        MxVal = Other.MxVal;
        return *this;
    }

    void TExpHistWithMax::Add(const uint64& Tm, const double& Val) {
        if (TotalCount == 0 || Val > MxVal) { MxVal = Val; }
        TBase::Add(TIntervalWithMax(Tm, Val));
    }

    double TExpHistWithMax::GetMxVal() const {
        // TODO the paper returns the maximum of all BUT the last block
        return MxVal;
    }

    double TExpHistWithMax::GetMxVal(const uint64& Tm) {
        Forget(Tm);
        return GetMxVal();
    }

    void TExpHistWithMax::ToExpHist(TExpHistogram& ExpHist) const {
        ExpHist.IntervalV.Gen(IntervalV.Len(), IntervalV.Len());
        ExpHist.LogSizeToBlockCountV = LogSizeToBlockCountV;
        ExpHist.WindowMSec = WindowMSec;
        ExpHist.Eps = Eps;
        ExpHist.TotalCount = TotalCount;

        for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
            ExpHist.IntervalV[IntervalN] = IntervalV[IntervalN];
        }
    }

    void TExpHistWithMax::OnIntervalForgotten(const TIntervalWithMax& Interval) {
        if (TotalCount == 0) { return; }

        if (MxVal == Interval.GetMxVal()) {
            // find a new maximal value
            MxVal = IntervalV[0].GetMxVal();
            for (int IntervalN = 1; IntervalN < IntervalV.Len(); IntervalN++) {
                if (IntervalV[IntervalN].GetMxVal() > MxVal) {
                    MxVal = IntervalV[IntervalN].GetMxVal();
                }
            }
        }
    }

    void TExpHistWithMax::OnAfterSwallow() {
        double NewMxVal = TFlt::NInf;
        for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
            if (IntervalV[IntervalN].GetMxVal() > NewMxVal) {
                NewMxVal = IntervalV[IntervalN].GetMxVal();
            }
        }
        MxVal = NewMxVal;
    }


    ///////////////////////////////////////////////////
    /// Approximate minimum on a sliding window
    TWindowMin::TWindowMin(const uint64& _WindowMSec, const double& _Eps):
            WindowMSec(_WindowMSec),
            Eps(_Eps) {
        EAssert(0 <= Eps && Eps < .5);
    }

    void TWindowMin::Add(const uint64& Tm, const double& Val) {
        if (IntervalV.Empty() || Val < MnVal) { MnVal = Val; }

        // discard all values which are >= Val
        while (!IntervalV.Empty()) {
            const uint IntervalSize = IntervalV.Last().GetCount();
            IntervalV.DelLast();
            --BlockSizeToBlockCount(IntervalSize);
        }

        // add the new element
        IntervalV.Add(TIntervalWithMin(Tm, Val));

        ++LogBlockSizeToBlockCount(0);

        Compress();
        Forget(Tm);

        AssertR(CheckInvariant1(), "WindowMin: Invariant 1 fails after Add!");
        AssertR(CheckInvariant2(), "WindowMin: Invariant 2 fails after Add!");
    }

    double TWindowMin::GetMnVal(const uint64& Tm) {
        Forget(Tm);
        return GetMnVal();
    }

    double TWindowMin::GetMnVal() const {
        // TODO do the same is with TExpHistWithMax
        return MnVal;
    }

    void TWindowMin::SetMnChangedCallback(TMnCallback* Cb) {
        MnCallback = Cb;
    }

    void TWindowMin::Forget(const uint64& CurrTm) {
        const uint64 CutoffTm = CurrTm >= WindowMSec >= WindowMSec ? CurrTm - WindowMSec : 0ul;

        while (!IntervalV.Empty() && IntervalV[0].GetEndTm() < CutoffTm) {
            const TIntervalWithMin& RemovedInterval = IntervalV[0];
            const uint IntervalSize = RemovedInterval.GetCount();
            const double IntervalMnVal = RemovedInterval.GetMnVal();

            --BlockSizeToBlockCount(IntervalSize);
            IntervalV.Del(0);

            // check if we need to set a new minimum value
            if (IntervalMnVal == MnVal && !IntervalV.Empty()) {
                MnVal = IntervalV[0].GetMnVal();
                for (int IntervalN = 1; IntervalN < IntervalV.Len(); IntervalN++) {
                    if (IntervalV[IntervalN].GetMnVal() < MnVal) {
                        MnVal = IntervalV[IntervalN].GetMnVal();
                    }
                }
                if (MnCallback != nullptr) {
                    MnCallback->OnMnChanged(MnVal);
                }
            }
        }
    }

    void TWindowMin::Compress() {
        int CurrLogSize = 0;
        int CurrPos = IntervalV.Len()-1;
        while (CurrLogSize < LogSizeToBlockCountV.Len()) {
            const uint BlockCount = LogBlockSizeToBlockCount(CurrLogSize);

            if (BlockCount <= GetMxBlockCount()) { break; }

            // merge the block
            const int MergePos = CurrPos - BlockCount + 1;
            IntervalV[MergePos].Swallow(IntervalV[MergePos+1]);
            IntervalV.Del(MergePos+1);

            LogBlockSizeToBlockCount(CurrLogSize) -= 2;
            ++LogBlockSizeToBlockCount(CurrLogSize+1);

            ++CurrLogSize;
            CurrPos = MergePos;
        }
    }

    TUInt& TWindowMin::LogBlockSizeToBlockCount(const uint& LogSize) {
        Assert(LogSize < uint(LogSizeToBlockCountV.Len()));
        return LogSizeToBlockCountV[LogSize];
    }

    TUInt& TWindowMin::BlockSizeToBlockCount(const uint& BlockSize) {
        Assert(1 <= BlockSize);
        return LogBlockSizeToBlockCount(TMath::Log2(BlockSize));
    }

    uint TWindowMin::GetMxBlockCount() const {
        if (Eps == 0.0) { return TUInt::Mx; }
        return std::ceil(1.0 / Eps) + 1;
    }


    ///////////////////////////////////////////////////
    /// GK algorithm for sliding windows
    TSwGk::TTuple::TTuple(const uint64& WindowLen, const double& Eps,
                const uint64& Tm, const double& Val):
            TupleSizeExpHist(WindowLen, Eps),
            MnMxRankDiffExpHist(WindowLen, Eps) {
        // initialize G
        TupleSizeExpHist.Add(Tm, Val);
    }

    TSwGk::TTuple::TTuple(const uint64& WindowLen, const double& Eps, const uint64& Tm,
                    const double& Val, TTuple& RightTup):
            TupleSizeExpHist(WindowLen, Eps),
            MnMxRankDiffExpHist() {

        // initialize G = {(v,t)}
        TupleSizeExpHist.Add(Tm, Val);

        // initialize D = (Gi \ vi) U Di
        // 1) D = Gi
        RightTup.TupleSizeExpHist.ToExpHist(MnMxRankDiffExpHist);
        // 2) D = (Gi \ {vi}) (since this histogram only counts, we can just remove the first element)
        MnMxRankDiffExpHist.DelNewest();
        // 3) D = (Gi \ {vi}) U Di
        MnMxRankDiffExpHist.Swallow(RightTup.MnMxRankDiffExpHist);
    }

    double TSwGk::TTuple::GetVal(const uint64& Tm) {
        return TupleSizeExpHist.GetMxVal(Tm);
    }

    uint TSwGk::TTuple::GetTupleSize(const uint64& Tm) {
        return TupleSizeExpHist.GetCount(Tm);
    }

    uint TSwGk::TTuple::GetMnMxRankDiff(const uint64& Tm) {
        return MnMxRankDiffExpHist.GetCount(Tm);
    }

    TSwGk::TSwGk(const uint64& _WindowLen, const double& _EpsGk, const double& _EpsEh):
            WindowLen(_WindowLen),
            EpsGk(_EpsGk),
            EpsEh(_EpsEh) {
        EAssert(EpsGk < .2);
        EAssert(EpsEh < .2);
        WinMin.SetMnChangedCallback(this);
    }

    double TSwGk::Query(const double& Quantile) {
        if (GetSummarySize() == 0) { return 0; }

        const uint64 WinSize = TMath::Mn(WindowLen, SampleN);
        const double MnRankThreshold = WinSize*(TMath::Mx(0.0, Quantile - EpsGk));

        uint64 CurrMnRank = 0;
        double MxVal = TFlt::NInf;

        typename TSummary::iterator TupleIt = Summary.begin();
        while (TupleIt != Summary.end()) {
            const double TupleVal = TupleIt->GetVal(SampleN);
            CurrMnRank += TupleIt->GetTupleSize(SampleN);

            if (TupleVal > MxVal) { MxVal = TupleVal; }
            if (CurrMnRank >= MnRankThreshold) { break; }

            ++TupleIt;
        }

        return MxVal;
    }

    void TSwGk::Insert(const double& Val) {
        std::cout << "inserting " << Val << std::endl;
        typename TSummary::iterator TupleIt = Summary.begin();

        // iterate to the correct position
        while (TupleIt != Summary.end() && TupleIt->GetVal(SampleN) <= Val) {
            ++TupleIt;
        }

        // insert the tuple
        if (TupleIt == Summary.end()) {
            Summary.insert(TupleIt, TTuple(WindowLen, EpsEh, SampleN, Val));
        } else {
            Summary.insert(TupleIt, TTuple(WindowLen, EpsEh, SampleN, Val, *TupleIt));
        }

        ++SampleN;

        // compress if needed
        if (ShouldCompress()) {
            Compress();
        }
    }

    void TSwGk::Compress() {
        std::cout << "starting compress, eps: " << EpsGk << std::endl;
        PrintSummary();
        const uint64 WinSize = TMath::Mn(WindowLen, SampleN);

        std::cout << "window size " << WinSize << std::endl;

        typename TSummary::iterator LeftIt = --Summary.end();

        // erase empty tuples from the start of the list
        while (LeftIt->GetTupleSize(SampleN) == 0) {
            LeftIt = --Summary.erase(LeftIt);
        }

        if (LeftIt == Summary.begin()) { return; }

        do {
            typename TSummary::iterator RightIt = LeftIt--;

            const uint LeftCount = LeftIt->GetTupleSize(SampleN);
            // if the tuple is empty then delete it immediately
            if (LeftCount == 0) {
                LeftIt = Summary.erase(LeftIt);
            } else {
                // try to merge the two tuples
                typename TSummary::iterator LargerIt = RightIt->GetVal(SampleN) > LeftIt->GetVal(SampleN) ?
                                                            RightIt : LeftIt;

                const uint LeftTupleCount = LeftIt->GetTupleSize(SampleN);
                const uint RightTupleCount = RightIt->GetTupleSize(SampleN);
                const uint LargerCorr = LargerIt->GetMnMxRankDiff(SampleN);

                std::cout << "joined tuple count: " << LeftTupleCount + RightTupleCount << ", correction: " << LargerCorr << ", 2*eps*W: " << 2*EpsGk*WinSize << std::endl;
                if (LeftTupleCount + RightTupleCount + LargerCorr < 2*EpsGk*WinSize) {
                    // merge the two tuples
                    RightIt->Swallow(*LeftIt, LargerIt == LeftIt);
                    LeftIt = Summary.erase(LeftIt);
                }
            }
        } while (LeftIt != Summary.begin());

        std::cout << "ending compress" << std::endl;
        PrintSummary();
    }

    int TSwGk::GetSummarySize() const {
        return Summary.size();
    }

    void TSwGk::PrintSummary() const {
        std::cout << "[";

        auto It = Summary.begin();
        if (It != Summary.end()) { std::cout << *(It++); }
        while (It != Summary.end()) {
            std::cout << ", " << *(It++);
        }
        std::cout << "]" << std::endl;
    }

    void TSwGk::OnMnChanged(const double&) {
        // TODO take one element from G1
        if (GetSummarySize() > 0) {
            Summary.begin()->DelNewestNonMx();
        }
    }

    bool TSwGk::ShouldCompress() const {
        return SampleN % int(ceil(1.0 / (2.0*EpsGk))) == 0;
    }
}

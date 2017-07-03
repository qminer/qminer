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

    // TODO when compressing the min can change because the oldest block can suddenly be in the
    // window
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

    void TExpHistWithMax::Forget(const int64& ForgetTm) {
        TBase::Forget(ForgetTm);
        // if there are no intervals left, we can terminate the procedure
        if (IntervalV.Empty()) {
            MxVal = TFlt::NInf;
            return;
        }
        // check if the value of the last interval fell outside the window
        // if it did, then update the max value
        if (!IsMaxInWindow(ForgetTm, IntervalV[0]) && IntervalV[0].GetMxVal() == MxVal) {
            FindNewMxVal(1);
        }
    }

    void TExpHistWithMax::Add(const uint64& Tm, const double& Val) {
        if (TotalCount == 0 || Val > MxVal) { MxVal = Val; }
        TBase::Add(TIntervalWithMax(Tm, Val));
    }

    double TExpHistWithMax::GetMxVal() const {
        Assert(!IntervalV.Empty());
        return MxVal;
    }

    void TExpHistWithMax::DelNewestNonMx(const bool& FireEvent) {
        Assert(!IntervalV.Empty());
        if (IntervalV.Len() == 1 || IntervalV.Last().GetMxVal() != MxVal) {
            DelNewest(FireEvent);
        } else {
            // delete the second newest
            const int DelPos = IntervalV.Len()-2;

            // check if we need to split the interval
            if (IntervalV[DelPos].GetCount() > 1) {
                const TIntervalWithMax& SplitInterval = IntervalV[DelPos];
                const uint IntervalCount = SplitInterval.GetCount();

                Assert(IntervalCount == 2);

                TIntervalWithMax StartInterval, EndInterval;
                SplitInterval.Split(StartInterval, EndInterval);

                // delete the newest of the two (the end interval)
                IntervalV[DelPos] = StartInterval;

                TotalCount -= IntervalCount;
                --BlockSizeToBlockCount(IntervalCount);
                ++BlockSizeToBlockCount(IntervalCount >> 1);

                if (BlockSizeToBlockCount(IntervalCount >> 1) > GetMxBlocksSameSize()) {
                    // merge the block and the invariants will be satisfied
                    CompressOldestInBatch(IntervalV.Len()-1);
                } else {
                    // the smaller intervals are fine, but there could be a shortage
                    // in the size of the interval we split, so we may need to split
                    // the larger intervals
                    BreakBlocks(IntervalCount);
                }

                // notify subclasses so they can adapt their state
                OnIntervalRemoved(EndInterval, FireEvent);
            } else {
                const TIntervalWithMax Removed = IntervalV[DelPos];
                IntervalV.Del(DelPos);
                --TotalCount;
                --LogBlockSizeToBlockCount(0);
                BreakBlocks();
                // notify subclasses so they can adapt their state
                OnIntervalRemoved(Removed, FireEvent);
            }
        }

        AssertR(CheckInvariant1(), "EH with max: Invariant 1 fails after removing newest non-max!");
        AssertR(CheckInvariant2(), "EH with max: Invariant 2 fails after removing newest non-max!");
    }

    void TExpHistWithMax::ToExpHist(TExpHistogram& ExpHist) const {
        ExpHist.IntervalV.Gen(IntervalV.Len(), IntervalV.Len());
        ExpHist.LogSizeToBlockCountV = LogSizeToBlockCountV;
        ExpHist.Eps = Eps;
        ExpHist.TotalCount = TotalCount;

        for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
            ExpHist.IntervalV[IntervalN] = IntervalV[IntervalN];
        }
    }

    void TExpHistWithMax::OnIntervalRemoved(const TIntervalWithMax& Interval, const bool& FireEvent) {
        TExpHistBase<TIntervalWithMax>::OnIntervalRemoved(Interval, FireEvent);

        if (MxVal == Interval.GetMxVal()) {
            FindNewMxVal();
        }
    }

    void TExpHistWithMax::OnAfterSwallow() {
        FindNewMxVal();
    }


    ///////////////////////////////////////////////////
    /// Approximate minimum on a sliding window
    TWindowMin::TWindowMin(const double& _Eps):
            /* WindowMSec(_WindowMSec), */
            Eps(_Eps) {
        EAssert(0 <= Eps && Eps < .5);
    }

    void TWindowMin::Add(const uint64& Tm, const double& Val) {
        bool MnChanged = false;

        // discard all values which are >= Val
        while (!IntervalV.Empty() && IntervalV.Last().GetMnVal() >= Val) {
            --BlockSizeToBlockCount(IntervalV.Last().GetCount());
            IntervalV.DelLast();
        }

        // add the new element
        if (IntervalV.Empty() || Val < MnVal) {
            MnVal = Val;
            MnChanged = true;
        }
        IntervalV.Add(TIntervalWithMin(Tm, Val));
        ++LogBlockSizeToBlockCount(0);

        Compress();

        AssertR(CheckInvariant1(), "WindowMin: Invariant 1 fails after Add!");
        AssertR(CheckInvariant2(), "WindowMin: Invariant 2 fails after Add!");

        if (MnChanged && MnCallback != nullptr) {
            MnCallback->OnMnChanged(MnVal);
        }
    }

    void TWindowMin::Forget(const int64& NewForgetTm) {
        double MinForget = TFlt::PInf;
        while (!IntervalV.Empty() && !IsMinInWindow(NewForgetTm, IntervalV[0])) {
            const TIntervalWithMin& RemovedInterval = IntervalV[0];
            const uint IntervalSize = RemovedInterval.GetCount();
            const double IntervalMnVal = RemovedInterval.GetMnVal();

            if (IntervalMnVal < MinForget) {
                MinForget = IntervalMnVal;
            }

            --BlockSizeToBlockCount(IntervalSize);
            IntervalV.Del(0);
        }

        // check if we need to set a new minimum value
        if (MinForget == MnVal) {
            MnVal = TFlt::PInf;
            for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
                const TFlt& IntervalMn = IntervalV[IntervalN].GetMnVal();
                if (IntervalMn < MnVal) {
                    MnVal = IntervalMn;
                }
            }
            if (MnCallback != nullptr) {
                MnCallback->OnMnChanged(MnVal);
            }
        }
    }

    double TWindowMin::GetMnVal() const {
        Assert(GetSummarySize() > 0);
        return MnVal;
    }

    void TWindowMin::SetMnChangedCallback(TMnCallback* Cb) {
        // TODO?? should I call the callback????
        /* MnCallback = Cb; */
    }

    void TWindowMin::PrintSummary() const {
        std::cout << "[";
        for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
            std::cout << IntervalV[IntervalN];
            if (IntervalN < IntervalV.Len() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    }

    void TWindowMin::Compress() {
        // TODO when compressing the min can change because the oldest block can
        // go from being half out of the window to half in
        int CurrLogSize = 0;
        int CurrPos = IntervalV.Len()-1;
        while (CurrLogSize < LogSizeToBlockCountV.Len()) {
            const uint BlockCount = LogBlockSizeToBlockCount(CurrLogSize);

            if (BlockCount <= GetMxBlocksSameSize()) { break; }

            // merge the block
            const int MergePos = CurrPos - BlockCount + 1;

            IntervalV[MergePos].Swallow(IntervalV[MergePos+1]);
            IntervalV.Del(MergePos+1);

            ReserveStructures(CurrLogSize+1);
            LogBlockSizeToBlockCount(CurrLogSize) -= 2;
            ++LogBlockSizeToBlockCount(CurrLogSize+1);

            ++CurrLogSize;
            CurrPos = MergePos;
        }
    }

    void TWindowMin::ReserveStructures(const uint& LogSize) {
        while (LogSize >= uint(LogSizeToBlockCountV.Len())) {
            LogSizeToBlockCountV.Add(0);
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

    uint TWindowMin::GetMxBlocksSameSize() const {
        if (Eps == 0.0) { return TUInt::Mx; }
        return std::ceil(1.0 / Eps) + 1;
    }

    bool TWindowMin::CheckInvariant1() const {
        // older values have to be less than newer values
        for (int IntervalN = 0; IntervalN < IntervalV.Len()-1; IntervalN++) {
            if (IntervalV[IntervalN].GetMnVal() >= IntervalV[IntervalN+1].GetMnVal()) {
                std::cout << "Invariant 1 in TWindowMin failed!" << std::endl;
                PrintSummary();
                return false;
            }
        }
        return true;
    }

    bool TWindowMin::CheckInvariant2() const {
        const uint MxBlocksSameSize = GetMxBlocksSameSize();

        // 1) the buckets must be of increasing size
        // 2) the bucket sizes must be a power of 2
        // 3) the power of the bucket size is less than the number of intervals
        // 4) there are at most k/2+1 buckets of the same size

        uint CurrBucketSize = 0;
        for (int IntervalN = IntervalV.Len()-1; IntervalN >= 0; --IntervalN) {
            const uint BucketSize = IntervalV[IntervalN].GetCount();
            // 1)
            if (BucketSize < CurrBucketSize) {
                std::cout << "check on bucket size failed!" << std::endl;
                return false;
            }
            // 2)
            if (!TMath::IsPow2(BucketSize)) {
                std::cout << "power of 2 check failed!" << std::endl;
                return false;
            }
            // 3)
            if (int(TMath::Log2(BucketSize)) > IntervalV.Len()) {
                std::cout << "check 3 failed" << std::endl;
                return false;
            }

            CurrBucketSize = BucketSize;
        }

        // 4)
        for (int LogSize = 0; LogSize < LogSizeToBlockCountV.Len(); LogSize++) {
            const TUInt& BucketCount = LogSizeToBlockCountV[LogSize];
            if (BucketCount > MxBlocksSameSize) {
                std::cout << "check on the number of buchets of size " << (1 << LogSize) << " failed" << std::endl;
                std::cout << "bucket count of size " << (1 << LogSize) << " is " << BucketCount << " max is " << MxBlocksSameSize << std::endl;
                PrintSummary();
                return false;
            }
        }

        return true;
    }

    ///////////////////////////////////////////////////
    /// GK algorithm for sliding windows
    TSwGk::TSwGk(const double& _EpsGk, const double& _EpsEh):
            WinMin(_EpsEh),
            EpsGk(_EpsGk),
            EpsEh(_EpsEh) {
        EAssert(EpsGk < .2);
        EAssert(EpsEh < .2);
        WinMin.SetMnChangedCallback(this);
    }

    double TSwGk::Query(const double& Quantile) {
        if (GetSummarySize() == 0) { return 0; }

        /* if (Quantile <= EpsGk || GetSummarySize() == 1) { */
        if (Quantile <= EpsGk) {
            WinMin.Forget(ForgetTm);
            return WinMin.GetMnVal();
        } else {
            const double MnRankThreshold = ItemCount*(TMath::Mx(0.0, Quantile - EpsGk));

            uint64 CurrMnRank = 0;
            double MxVal = TFlt::NInf;

            typename TSummary::iterator TupleIt = Summary.begin();
            while (TupleIt != Summary.end()) {
                TupleIt->Forget(ForgetTm);

                const uint TupleSize = TupleIt->GetTupleSize();

                if (TupleSize > 0) {
                    CurrMnRank += TupleSize;
                    const double TupleVal = TupleIt->GetVal();

                    if (TupleVal > MxVal) { MxVal = TupleVal; }
                    if (CurrMnRank >= MnRankThreshold) { break; }
                }

                ++TupleIt;
            }

            return MxVal;
        }
    }

    void TSwGk::Insert(const uint64& ValTm, const double& Val) {
        typename TSummary::iterator TupleIt = Summary.begin();

        // iterate to the correct position
        while (TupleIt != Summary.end()) {
            TupleIt->Forget(ForgetTm);
            if (TupleIt->GetTupleSize() > 0 && TupleIt->GetVal() > Val) { break; }
            ++TupleIt;
        }

        // insert the tuple
        if (TupleIt == Summary.end()) {
            Summary.insert(TupleIt, TTuple(EpsEh, ValTm, Val, *this));
        } else {
            Summary.insert(TupleIt, TTuple(EpsEh, ValTm, Val, *TupleIt, *this));
        }

        // update counters
        ++ItemCount;
        ++SampleN;

        WinMin.Add(ValTm, Val);

        // compress if needed
        if (ShouldCompress()) {
            Compress();
        }
    }

    void TSwGk::Forget(const int64& CutoffTm) {
        ForgetTm = CutoffTm;
    }

    void TSwGk::Compress() {
        // first refresh the whole structure, so we don't have
        // to worry about empty tuples later on
        RefreshSummary();

        if (Summary.size() > 1) {
            typename TSummary::iterator LeftIt = --Summary.end();

            do {
                typename TSummary::iterator RightIt = LeftIt--;
                // try to merge the two tuples
                typename TSummary::iterator LargerIt = RightIt->GetVal() > LeftIt->GetVal() ?
                                                            RightIt : LeftIt;

                const uint LeftTupleCount = LeftIt->GetTupleSize();
                const uint RightTupleCount = RightIt->GetTupleSize();
                const uint LargerCorr = LargerIt->GetMnMxRankDiff();

                if (LeftTupleCount + RightTupleCount + LargerCorr < 2*EpsGk*ItemCount) {
                    // merge the two tuples
                    RightIt->Swallow(*LeftIt, LargerIt == LeftIt);
                    LeftIt = Summary.erase(LeftIt);
                }
            } while (LeftIt != Summary.begin());
        }

        // also update the minimum tracker
        WinMin.Forget(ForgetTm);
    }

    int TSwGk::GetSummarySize() const {
        return Summary.size();
    }

    uint64 TSwGk::GetValCount() {
        RefreshSummary();
        return ItemCount;
    }

    uint64 TSwGk::GetValRecount() {
        uint64 Count = 0;
        /* uint64 Count = WinMin.Empty() ? 0 : 1;   // one item is always in the min EH */
        typename TSummary::iterator TupleIt = Summary.begin();
        while (TupleIt != Summary.end()) {
            TupleIt->Forget(ForgetTm);
            if (TupleIt->Empty()) {
                TupleIt = Summary.erase(TupleIt);
            } else {
                Count += TupleIt->GetTupleSize();
                ++TupleIt;
            }
        }
        return Count;
    }

    void TSwGk::PrintSummary() const {
        std::cout << "minimum summary: ";
        WinMin.PrintSummary();
        std::cout << "\nsummary:\n[";

        auto It = Summary.begin();
        if (It != Summary.end()) { std::cout << *(It++); }
        while (It != Summary.end()) {
            std::cout << ", " << *(It++);
        }
        std::cout << "]" << std::endl;
    }

    void TSwGk::OnMnChanged(const double&) {    // TODO remove
        while (GetSummarySize() > 0) {
            typename TSummary::iterator FirstIt = Summary.begin();
            FirstIt->Forget(ForgetTm);
            if (FirstIt->GetTupleSize() > 0) { break; }
            Summary.erase(FirstIt);
        }
        if (GetSummarySize() > 0) {
            // TODO should ignore the change in OnItemsDeleted
            Summary.begin()->DelNewestNonMx(false);
        }
    }

    void TSwGk::OnItemsDeleted(const uint64& DelCount) {
        Assert(ItemCount >= DelCount);
        ItemCount -= DelCount;
    }

    void TSwGk::RefreshSummary() {
        typename TSummary::iterator TupleIt = Summary.begin();
        while (TupleIt != Summary.end()) {
            TupleIt->Forget(ForgetTm);
            if (TupleIt->GetTupleSize() == 0) {
                TupleIt = Summary.erase(TupleIt);
            } else {
                ++TupleIt;
            }
        }
    }

    bool TSwGk::ShouldCompress() const {
        return SampleN % int(ceil(1.0 / (2.0*EpsGk))) == 0;
    }

    TSwGk::TTuple::TTuple(const double& Eps, const uint64& ValTm, const double& Val,
                TDelCallback& DelCallback):
            TupleSizeExpHist(Eps),
            MnMxRankDiffExpHist(Eps) {
        // initialize G
        TupleSizeExpHist.SetCallback(DelCallback);
        TupleSizeExpHist.Add(ValTm, Val);
    }

    TSwGk::TTuple::TTuple(const double& Eps, const uint64& ValTm,
                    const double& Val, TTuple& RightTup, TDelCallback& DelCallback):
            TupleSizeExpHist(Eps),
            MnMxRankDiffExpHist(Eps) {

        // initialize G = {(v,t)}
        TupleSizeExpHist.SetCallback(DelCallback);
        TupleSizeExpHist.Add(ValTm, Val);

        // initialize D = (Gi \ vi) U Di
        // 1) D = Gi
        RightTup.TupleSizeExpHist.ToExpHist(MnMxRankDiffExpHist);
        // 2) D = (Gi \ {vi}) (since this histogram only counts, we can just remove the first element)
        MnMxRankDiffExpHist.DelNewest();
        // 3) D = (Gi \ {vi}) U Di
        MnMxRankDiffExpHist.Swallow(RightTup.MnMxRankDiffExpHist);
    }

    double TSwGk::TTuple::GetVal() const {
        return TupleSizeExpHist.GetMxVal();
    }

    uint TSwGk::TTuple::GetTupleSize() const {
        return TupleSizeExpHist.GetCount();
    }

    uint TSwGk::TTuple::GetMnMxRankDiff() const {
        return MnMxRankDiffExpHist.GetCount();
    }

    void TSwGk::TTuple::Forget(const int64& Tm) {
        TupleSizeExpHist.Forget(Tm);
        MnMxRankDiffExpHist.Forget(Tm);
    }

    void TSwGk::TTuple::DelNewestNonMx(const bool& FireEvent) {
        TupleSizeExpHist.DelNewestNonMx(FireEvent);
    }

    void TSwGk::TTuple::Swallow(TTuple& Other, const bool& TakeMnMxRank) {
        TupleSizeExpHist.Swallow(Other.TupleSizeExpHist);
        if (TakeMnMxRank) { std::swap(MnMxRankDiffExpHist, Other.MnMxRankDiffExpHist); }
    }

    ////////////////////////////////////////////
    /// GK algorithm which works on a fixed size window.
    TCountWindowGk::TCountWindowGk(const uint64& _WindowSize, const double& EpsGk, const double& EpsEh):
            TSwGk(EpsGk, EpsEh),
            WindowSize(_WindowSize) {
        EAssertR(WindowSize <= uint64(TInt64::Mx), "Maximum size of window is TInt64::Mx!");
    }

    void TCountWindowGk::Insert(const double& Val) {
        // first forget all the values which fall outside the window
        // then insert the new value
        const uint64 CurrN = GetSampleN();
        const int64 CutoffN = CurrN - WindowSize;
        // forget
        TSwGk::Forget(CutoffN);
        // insert
        TSwGk::Insert(CurrN, Val);
    }

    /////////////////////////////////////////////
    /// Time-window SW-GK
    TTimeWindowGk::TTimeWindowGk(const uint64& _WindowMSec, const double& _EpsGk, const double& _EpsEh):
        TSwGk(_EpsGk, _EpsEh),
        WindowMSec(_WindowMSec) {}

    void TTimeWindowGk::Insert(const uint64& ValTm, const double& Val) {
        UpdateTime(ValTm);
        TSwGk::Insert(ValTm, Val);
    }

    void TTimeWindowGk::UpdateTime(const uint64& Tm) {
        const int64 CutoffTm = int64(Tm) - int64(WindowMSec);
        TSwGk::Forget(CutoffTm);
    }
}

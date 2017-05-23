#include <algorithm>
#include <iostream>

namespace TQuant {

    template <typename T1, typename T2, typename T3>
    std::ostream& operator <<(std::ostream& os, const TTriple<T1, T2, T3>& Triple) {
        os << "<" << Triple.Val1.Val << ", " << Triple.Val2.Val << ", " << Triple.Val3.Val << ">";
        return os;
    }

    template <typename T>
    std::ostream& operator <<(std::ostream& os, const TVec<T>& Vec) {
        os << "[";
        for (int ValN = 0; ValN < Vec.Len(); ValN++) {
            os << Vec[ValN];
            if (ValN< Vec.Len() - 1) {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }

    std::ostream& operator <<(std::ostream& os, const TGkTuple& Tup) {
        os << '<' << Tup.GetVal() << ", " << Tup.GetTupleSize() << ", " << Tup.GetMnMxRankDiff() << '>';
        return os;
    }

    TGkTuple::TGkTuple():
            Val(),
            TupleSize(),
            MnMxRankDiff() {}

    TGkTuple::TGkTuple(const TFlt& _Val, const TUInt& _TupleSize, const TUInt& Delta):
            Val(_Val),
            TupleSize(_TupleSize),
            MnMxRankDiff(Delta) {
        std::cout << "delta: " << uint32(Delta) << "\n";
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
        /* std::cout << "compressing\n"; */
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
    /* TBiasedGk::TBiasedGk(const double& TargetQuantile, const double& TargetQuantileEps, */
    /*         const bool& _UseBands): */
    /*     MnQuantile(TargetQuantile), */
    /*     MnEps(TargetQuantileEps), */
    /*     UseBands(_UseBands) {} */

    TBiasedGk::TBiasedGk(const double& _MxTargetQuant, const double& _Eps, const bool& _UseBands):
            Quant0(_MxTargetQuant),
            Eps(_Eps),
            UseBands(_UseBands) {

        EAssert(Eps > 0.0);
        EAssert(Quant0 > 0);
    }

    double TBiasedGk::Query(const double& Quantile) const {
        EAssert(0 < Quantile && Quantile < 1);
        if (GetSummarySize() == 0) { return 0; }

        /* const uint TargetRank = std::ceil(Quantile*SampleN - NUM_EPS);  // fix for numerical errors */
        const double TargetRank = Quantile*SampleN;
        /* const double EpsRank = double(GetMxTupleSize(TargetRank)) / 2.0; */
        const double EpsRank = GetMxTupleSize(TargetRank) / 2;

        if (TargetRank <= 1) { return Summary[0].GetVal(); }

        /* std::cout << "q: " << Quantile << ", target rank: " << TargetRank << ", n: " << SampleN << ", allowed error: " << EpsRank << "\n"; */

        int CurrMnRank = 0;
        for (int TupleN = 0; TupleN < Summary.Len(); TupleN++) {
            const TGkTuple& Tuple = Summary[TupleN];
            const uint64 MxRank = CurrMnRank + Tuple.GetTupleSize() + Tuple.GetMnMxRankDiff();

            /* std::cout << "ri: " << CurrMnRank << ", r_max: " << MxRank << ", tuple: " << Tuple << "\n"; */

            if (MxRank > TargetRank + EpsRank) {
                /* std::cout << "returning: " << Summary[TupleN-1].GetVal() << "\n"; */
                return Summary[TupleN-1].GetVal();
            }

            CurrMnRank += Tuple.GetTupleSize();
        }

        return Summary.Last().GetVal();
    }

    void TBiasedGk::Insert(const double& Val) {
        std::cout << "inserting " << Val << "\n";
        int TupleN = 0;
        uint64 PrevTupleMnRank = 0;

        while (TupleN < Summary.Len()) {
            const TGkTuple& Tuple = Summary[TupleN];
            if (Tuple.GetVal() >= Val) { break; }   // this item will be on the right, don't add to the sum
            PrevTupleMnRank += Tuple.GetTupleSize();
            TupleN++;
        }

        /* std::cout << "found tuple number: " << TupleN << "\n"; */
        
        /* ++SampleN;      // XXX??? */

        if (TupleN == 0) {
            // special case
            Summary.Ins(0, TGkTuple(Val, 1, 0));
        } else if (TupleN == Summary.Len()) {
            // special case (symmetrical to the previous case)
            Summary.Add(TGkTuple(Val, 1, 0));
        } else {
            const double NewTupleMxSize = GetMxTupleSize(PrevTupleMnRank);
            /* std::cout << "new tuple max size: " << NewTupleMxSize << "\n"; */
            Summary.Ins(TupleN, TGkTuple(Val, 1, std::floor(NewTupleMxSize-1)));
            AssertR(NewTupleMxSize-1 >= 0, "Max tuple size is 0, but should be at least 1!");
        }

        ++SampleN;    // XXX???

        if (ShouldCompress()) {
            Compress();
        }
    }

    void TBiasedGk::Compress() {
        if (Summary.Empty()) { return; }

        /* std::cout << "compressing\n"; */

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
            /* std::cout << "quantile: " << double(PrevMnRank) / SampleN << ", eps: " << GetEps(double(PrevMnRank) / SampleN) << "\n"; */

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

/*         /1* const double Quant = Rank / SampleN; *1/ */
/*         return TMath::Mx(Eps.Val, Eps * Rank / (Quant0 * SampleN)); */
    }

    double TBiasedGk::GetMxTupleSize(const double& Rank) const {
        return TMath::Mx(1.0, 2.0*GetEps(Rank / SampleN)*Rank);
    }

    bool TBiasedGk::ShouldCompress() const {
        return SampleN % uint64(std::ceil(0.5 / Eps)) == 0;    // XXX
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
}

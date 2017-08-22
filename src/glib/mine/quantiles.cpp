#include <algorithm>
#include <iostream>

namespace TQuant {

    namespace TUtils {

        ///////////////////////////////////////////
        /// GK tuple
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


        ////////////////////////////////////////////
        /// Interval
        TInterval::TInterval(): TInterval(0ul) {}

        TInterval::TInterval(const uint64& _StartTm):
                StartTm(_StartTm) {}

        TInterval::TInterval(const uint64& _StartTm, const uint64& _Dur, const uint& _Count):
                StartTm(_StartTm),
                DurMSec(_Dur),
                ElCount(_Count) {}

        TInterval::TInterval(TSIn& SIn):
            StartTm(SIn),
            DurMSec(SIn),
            ElCount(SIn) {}

        void TInterval::Save(TSOut& SOut) const {
            SOut.Save(StartTm);
            SOut.Save(DurMSec);
            SOut.Save(ElCount);
        }

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

        TIntervalWithMax::TIntervalWithMax(TSIn& SIn):
            TInterval(SIn),
            MxVal(SIn) {}

        void TIntervalWithMax::Save(TSOut& SOut) const {
            TInterval::Save(SOut);
            SOut.Save(MxVal);
        }

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

        TIntervalWithMin::TIntervalWithMin(TSIn& SIn):
            TInterval(SIn),
            MnVal(SIn) {}

        void TIntervalWithMin::Save(TSOut& SOut) const {
            TInterval::Save(SOut);
            SOut.Save(MnVal);
        }

        void TIntervalWithMin::Swallow(const TIntervalWithMin& Other) {
            TInterval::Swallow(Other);
            if (Other.MnVal < MnVal) {
                MnVal = Other.MnVal;
            }
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

        TExpHistWithMax::TExpHistWithMax(TSIn& SIn):
                TBase(SIn),
                MxVal(SIn) {}

        void TExpHistWithMax::Save(TSOut& SOut) const {
            TBase::Save(SOut);
            SOut.Save(MxVal);
        }

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
            /* Assert(!IntervalV.Empty()); */
            return MxVal;
        }

        void TExpHistWithMax::DelNewestNonMx() {
            Assert(!IntervalV.Empty());
            if (IntervalV.Len() == 1 || IntervalV.Last().GetMxVal() != MxVal) {
                DelNewest();
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
                    OnIntervalRemoved(EndInterval);
                } else {
                    const TIntervalWithMax Removed = IntervalV[DelPos];
                    IntervalV.Del(DelPos);
                    --TotalCount;
                    --LogBlockSizeToBlockCount(0);
                    BreakBlocks();
                    // notify subclasses so they can adapt their state
                    OnIntervalRemoved(Removed);
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

        void TExpHistWithMax::OnIntervalRemoved(const TIntervalWithMax& Interval) {
            TExpHistBase<TIntervalWithMax>::OnIntervalRemoved(Interval);

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

        TWindowMin::TWindowMin(TSIn& SIn):
            IntervalV(SIn),
            LogSizeToBlockCountV(SIn),
            Eps(SIn),
            MnVal(SIn),
            ForgetTm(SIn) {}

        void TWindowMin::Save(TSOut& SOut) const {
            IntervalV.Save(SOut);
            LogSizeToBlockCountV.Save(SOut);
            Eps.Save(SOut);
            MnVal.Save(SOut);
            ForgetTm.Save(SOut);
        }

        void TWindowMin::Add(const uint64& Tm, const double& Val) {
            // discard all values which are >= Val
            while (!IntervalV.Empty() && IntervalV.Last().GetMnVal() >= Val) {
                --BlockSizeToBlockCount(IntervalV.Last().GetCount());
                IntervalV.DelLast();
            }

            // add the new element
            if (IntervalV.Empty() || Val < MnVal) {
                MnVal = Val;
            }
            IntervalV.Add(TIntervalWithMin(Tm, Val));
            ++LogBlockSizeToBlockCount(0);

            Compress();

            AssertR(CheckInvariant1(), "WindowMin: Invariant 1 fails after Add!");
            AssertR(CheckInvariant2(), "WindowMin: Invariant 2 fails after Add!");
        }

        void TWindowMin::Forget(const int64& NewForgetTm) {
            ForgetTm = NewForgetTm;

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
            }
        }

        double TWindowMin::GetMnVal() const {
            Assert(GetSummarySize() > 0);
            return MnVal;
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
            return LogBlockSizeToBlockCount((uint) TMath::Log2(BlockSize));
        }

        uint TWindowMin::GetMxBlocksSameSize() const {
            if (Eps == 0.0) { return TUInt::Mx; }
            return uint(std::ceil(1.0 / Eps) + 1);
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

        bool TWindowMin::IsMinInWindow(const int64& ForgetTm, const TIntervalWithMin& Interval) {
            return ForgetTm < int64(Interval.GetStartTm() + Interval.GetEndTm()) / 2;
        }

        ///////////////////////////////////////////////
        /// GK tuple with exponential histograms
        TEhTuple::TEhTuple(const double& Eps, const uint64& ValTm, const double& Val,
                    TDelCallback& DelCallback):
                TupleSizeExpHist(Eps),
                RightUncertExpHist(Eps) {
            // initialize G
            TupleSizeExpHist.SetCallback(DelCallback);
            TupleSizeExpHist.Add(ValTm, Val);
        }

        TEhTuple::TEhTuple(const double& Eps, const uint64& ValTm,
                        const double& Val, TEhTuple& RightTup, TDelCallback& DelCallback):
                TupleSizeExpHist(Eps),
                RightUncertExpHist(Eps) {

            // initialize G = {(v,t)}
            TupleSizeExpHist.SetCallback(DelCallback);
            TupleSizeExpHist.Add(ValTm, Val);

            // initialize D = (Gi \ vi) U Di
            // 1) D = Gi
            RightTup.TupleSizeExpHist.ToExpHist(RightUncertExpHist);
            // 2) D = (Gi \ {vi}) (since this histogram only counts, we can just remove the first element)
            RightUncertExpHist.DelNewest();
            // 3) D = (Gi \ {vi}) U Di
            RightUncertExpHist.Swallow(RightTup.RightUncertExpHist);
        }

        TEhTuple::TEhTuple(TSIn& SIn):
            TupleSizeExpHist(SIn),
            RightUncertExpHist(SIn) {}

        void TEhTuple::Save(TSOut& SOut) const {
            TupleSizeExpHist.Save(SOut);
            RightUncertExpHist.Save(SOut);
        }

        double TEhTuple::GetVal() const {
            return TupleSizeExpHist.GetMxVal();
        }

        uint TEhTuple::GetTupleSize() const {
            return TupleSizeExpHist.GetCount();
        }

        uint TEhTuple::GetMnMxRankDiff() const {
            return RightUncertExpHist.GetCount();
        }

        void TEhTuple::Forget(const int64& Tm) {
            TupleSizeExpHist.Forget(Tm);
            RightUncertExpHist.Forget(Tm);
        }

        void TEhTuple::DelNewestNonMx() {
            TupleSizeExpHist.DelNewestNonMx();
        }

        void TEhTuple::Swallow(TEhTuple& Other, const bool& TakeMnMxRank) {
            TupleSizeExpHist.Swallow(Other.TupleSizeExpHist);
            if (TakeMnMxRank) { std::swap(RightUncertExpHist, Other.RightUncertExpHist); }
        }

        /////////////////////////////////////////////
        /// SW-GK linked list summary
        TSwGkLLSummary::TSwGkLLSummary(const double& _EpsGk, const double& _EpsEh):
                EpsGk(_EpsGk),
                EpsEh(_EpsEh) {
            EAssert(EpsGk < .2);
            EAssert(EpsEh < .2);
        }

        TSwGkLLSummary::TSwGkLLSummary(TSIn& SIn):
                Summary(),
                ItemCount(SIn),
                EpsGk(SIn),
                EpsEh(SIn) {

            const TUInt Len(SIn);
            for (uint TupleN = 0; TupleN < Len; TupleN++) {
                Summary.push_back(TTuple(SIn));
            }
        }

        void TSwGkLLSummary::Save(TSOut& SOut) const {
            SOut.Save(ItemCount);
            SOut.Save(EpsGk);
            SOut.Save(EpsEh);

            const uint Len = (uint) Summary.size();
            SOut.Save(Len);
            for (auto TupleIt = Summary.begin(); TupleIt != Summary.end(); ++TupleIt) {
                TupleIt->Save(SOut);
            }
        }

        void TSwGkLLSummary::Insert(const uint64& ValTm, const double& Val) {
            TSummary::iterator TupleIt = Summary.begin();

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
        }

        double TSwGkLLSummary::Query(const double& Quantile) {
            if (Summary.empty()) { return 0; }

            uint64 CurrMnRank = 0;
            double MxVal = TFlt::NInf;

            TSummary::iterator TupleIt = Summary.begin();
            while (TupleIt != Summary.end()) {
                TupleIt->Forget(ForgetTm);

                const double MnRankThreshold = double(ItemCount)*(TMath::Mx(0.0, Quantile - EpsGk));    // must be calculated after forget, since the item count can change when forgetting
                const uint TupleSize = TupleIt->GetTupleSize();

                if (TupleSize > 0) {
                    CurrMnRank += TupleSize;
                    const double TupleVal = TupleIt->GetVal();

                    if (TupleVal > MxVal) { MxVal = TupleVal; }
                    if (CurrMnRank >= MnRankThreshold) { break; }
                }

                ++TupleIt;
            }

            // if all the tuples were empty, then compress the structure and
            // query again
            if (MxVal == TFlt::NInf) {
                Compress();
                return Query(Quantile);
            }

            return MxVal;
        }

        void TSwGkLLSummary::Query(const TFltV& PValV, TFltV& QuantV) {
            if (QuantV.Len() != PValV.Len()) { QuantV.Gen(PValV.Len(), PValV.Len()); }
            if (Summary.empty() || PValV.Empty()) { return; }

            uint64 CurrMnRank = 0;
            double MxVal = TFlt::NInf;

            int PValN = 0;
            TSummary::iterator TupleIt = Summary.begin();

            while (PValN < PValV.Len()) {
                const TFlt& PVal = PValV[PValN];

                while (TupleIt != Summary.end()) {
                    TupleIt->Forget(ForgetTm);

                    const double MnRankThreshold = double(ItemCount)*(TMath::Mx(0.0, PVal - EpsGk));
                    const uint TupleSize = TupleIt->GetTupleSize();

                    CurrMnRank += TupleSize;

                    if (TupleSize > 0) {
                        const double TupleVal = TupleIt->GetVal();

                        if (TupleVal > MxVal) { MxVal = TupleVal; }

                        if (CurrMnRank >= MnRankThreshold) {
                            // we have found the quantile
                            // reset the min rank to the begining of the iteration
                            CurrMnRank -= TupleSize;
                            break;
                        }
                    }

                    ++TupleIt;
                }

                if (MxVal != TFlt::NInf) { QuantV[PValN] = MxVal; }

                AssertR(PValN == 0 || PValV[PValN-1] <= PValV[PValN], "SW-GK: p-values should be sorted!");
                ++PValN;
            }

            // if all the tuples were empty, then compress the structure and query again
            // we could have done this in the loop, but that would require checks for every p-val
            // I decided this way would be better, since here we are penalized when the summary is empty
            // which is quick anyway
            if (MxVal == TFlt::NInf) {
                Compress();
                Query(PValV, QuantV);
            }
        }

        void TSwGkLLSummary::Forget(const uint64& _ForgetTm) {
            ForgetTm = _ForgetTm;
        }

        void TSwGkLLSummary::Compress() {
            // first refresh the whole structure, so we don't have
            // to worry about empty tuples later on
            Refresh(ForgetTm);

            if (Summary.size() > 1) {
                TSummary::iterator LeftIt = --Summary.end();

                do {
                    TSummary::iterator RightIt = LeftIt--;
                    // try to merge the two tuples
                    TSummary::iterator LargerIt = RightIt->GetVal() > LeftIt->GetVal() ?
                                                                RightIt : LeftIt;

                    const uint LeftTupleCount = LeftIt->GetTupleSize();
                    const uint RightTupleCount = RightIt->GetTupleSize();
                    const uint LargerCorr = LargerIt->GetMnMxRankDiff();

                    if (LeftTupleCount + RightTupleCount + LargerCorr < uint(2*EpsGk*double(ItemCount))) {
                        // merge the two tuples
                        RightIt->Swallow(*LeftIt, LargerIt == LeftIt);
                        LeftIt = Summary.erase(LeftIt);
                    }
                } while (LeftIt != Summary.begin());
            }
        }

        const TFlt& TSwGkLLSummary::GetEpsGk() const {
            return EpsGk;
        }

        const TFlt& TSwGkLLSummary::GetEpsEh() const {
            return EpsEh;
        }

        uint64 TSwGkLLSummary::GetValCount() {
            Refresh(ForgetTm);
            return ItemCount;
        }

        uint64 TSwGkLLSummary::GetValRecount() {
            uint64 Count = 0;
            TSummary::iterator TupleIt = Summary.begin();
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

        uint TSwGkLLSummary::GetTupleCount() const {
            return (uint) Summary.size();
        }

        void TSwGkLLSummary::OnItemsDeleted(const uint64& DelCount) {
            Assert(ItemCount >= DelCount);
            ItemCount -= DelCount;
        }

        void TSwGkLLSummary::Refresh(const uint64& ForgetTm) {
            TSummary::iterator TupleIt = Summary.begin();
            while (TupleIt != Summary.end()) {
                TupleIt->Forget(ForgetTm);
                if (TupleIt->GetTupleSize() == 0) {
                    TupleIt = Summary.erase(TupleIt);
                } else {
                    ++TupleIt;
                }
            }
        }

        std::ostream& operator <<(std::ostream& os, const TSwGkLLSummary& GkSummary) {
            const TSwGkLLSummary::TSummary& Summary = GkSummary.Summary;
            auto It = Summary.begin();
            if (It != Summary.end()) { std::cout << *(It++); }
            while (It != Summary.end()) {
                std::cout << ", " << *(It++);
            }
            std::cout << "]" << std::endl;
            return os;
        }

        // print operators
        std::ostream& operator <<(std::ostream& os, const TGkTuple& Tup) {
            os << '<' << Tup.GetVal() << ", " << Tup.GetTupleSize() << ", " << Tup.GetMnMxRankDiff() << '>';
            return os;
        }

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
    }

    TUtils::TGkMnUncertTuple::TGkMnUncertTuple(const double& Val):
            MxVal(Val) {}

    TUtils::TGkMnUncertTuple::TGkMnUncertTuple(const double& Val, const TGkMnUncertTuple& RightTuple):
            MxVal(Val),
            UncertRight(RightTuple.GetUncertRight() + RightTuple.GetTupleSize() - 1) {
        Assert(RightTuple.GetUncertRight() + RightTuple.GetTupleSize() >= 1);
    }

    TUtils::TGkMnUncertTuple::TGkMnUncertTuple(TSIn& SIn):
        MxVal(SIn),
        TupleSize(SIn),
        UncertRight(SIn) {}

    void TUtils::TGkMnUncertTuple::Save(TSOut& SOut) const {
        MxVal.Save(SOut);
        TupleSize.Save(SOut);
        UncertRight.Save(SOut);
    }

    void TUtils::TGkMnUncertTuple::Swallow(const TGkMnUncertTuple& LeftTuple) {
        TupleSize += LeftTuple.TupleSize;
    }

    void TUtils::TGkMnUncertTuple::SwallowOne() {
        ++TupleSize;
    }

    ////////////////////////////////////
    /// GK - Summary
    TUtils::TGkUtils::TVecSummary::TVecSummary(const double& _Eps):
            Eps(_Eps) {}

    TUtils::TGkUtils::TVecSummary::TVecSummary(TSIn& SIn):
            Summary(SIn),
            SampleN(SIn),
            Eps(SIn),
            UseBands(SIn) {}

    void TUtils::TGkUtils::TVecSummary::Save(TSOut& SOut) const {
        Summary.Save(SOut);
        SampleN.Save(SOut);
        Eps.Save(SOut);
        UseBands.Save(SOut);
    }

    double TUtils::TGkUtils::TVecSummary::Query(const double& PVal) const {
        if (Summary.Empty()) { return 0; }

        const double MnRankThreshold = double(SampleN)*(PVal - Eps);

        uint64 CurrMnRank = 0;
        int TupleN = 0;
        while (TupleN < Summary.Len()) {
            const TTuple& Tuple = Summary[TupleN];

            CurrMnRank += Tuple.GetTupleSize();

            if (CurrMnRank >= MnRankThreshold) {
                return Tuple.GetMxVal();
            }

            ++TupleN;
        }

        return Summary.Last().GetMxVal();
    }

    void TUtils::TGkUtils::TVecSummary::Query(const TFltV& PValV, TFltV& QuantV) const {
        if (QuantV.Len() != PValV.Len()) { QuantV.Gen(PValV.Len(), PValV.Len()); }
        if (Summary.Empty() || PValV.Empty()) { return; }

        uint64 CurrMnRank = 0;
        int PValN = 0;

        auto TupleIt = Summary.begin();
        const auto EndIt = Summary.end();

        while (PValN < PValV.Len()) {
            const TFlt& PVal = PValV[PValN];
            const double MnRankThreshold = double(SampleN)*(PVal - Eps);

            while (TupleIt != EndIt) {
                const TTuple& Tuple = *TupleIt;
                const TUInt& TupleSize = Tuple.GetTupleSize();

                CurrMnRank += TupleSize;

                if (CurrMnRank >= MnRankThreshold) {
                    QuantV[PValN] = Tuple.GetMxVal();
                    CurrMnRank -= TupleSize;
                    break;
                }

                ++TupleIt;
            }

            if (TupleIt == EndIt) {
                QuantV[PValN] = Summary.Last().GetMxVal();
            }

            ++PValN;
            EAssert(PValN >= PValV.Len() || PValV[PValN-1] <= PValV[PValN]);
        }
    }

    void TUtils::TGkUtils::TVecSummary::Insert(const double& Val) {
        // binary search to find the first tuple with value greater than val
        // this is where we will insert the new value
        const auto Cmp = [&](const TTuple& Tup, const double& Val) { return Tup.GetMxVal() < Val; };
        const auto ValIt = std::lower_bound(Summary.begin(), Summary.end(), Val, Cmp);

        const int NewValN = int(ValIt - Summary.begin());

        // if Val is the smallest or largest element, we must insert <v,1,0>
        if (NewValN == 0 || NewValN == Summary.Len()) {
            Summary.Ins(NewValN, TTuple(Val));
        }
        else {
            // let i be the index of the first tuple with greather vi, then
            // insert tuple <v,1,delta_i + g_i - 1>
            TTuple& RightTuple = Summary[NewValN];

            if (1 + RightTuple.GetTotalUncert() <= GetMxTupleUncert()) {
                // the right tuple can swallow the new tuple
                RightTuple.SwallowOne();
            } else {
                // we must insert the new tuple
                Summary.Ins(NewValN, TTuple(Val, RightTuple));
            }
        }

        ++SampleN;
    }

    void TUtils::TGkUtils::TVecSummary::Compress() {
        const uint ErrRange = GetMxTupleUncert();

        // go throught the tuples and merge the ones you can
        // the first and last tuple should never get merged
        for (int TupleN = Summary.Len() - 3; TupleN >= 1; TupleN--) {
            // delete all descendants of i and i itself
            // the descendants of i are the ones with Band_k < Band_i and
            // the children of i are the set {k_1,...,k_m | k_{j+1}=k_j+1,band(k_j)>=band(k_{j+1}),band(i)>band(k_1)}
            const TTuple& CurrTuple = Summary[TupleN];
            TTuple& NextTuple = Summary[TupleN+1];

            if (UseBands && GetBand(CurrTuple) > GetBand(NextTuple)) { continue; }

            if (CurrTuple.GetTupleSize() + NextTuple.GetTotalUncert() <= ErrRange) {
                // merge the two tuples
                NextTuple.Swallow(CurrTuple);
                Summary.Del(TupleN);
            }
        }
    }

    uint TUtils::TGkUtils::TVecSummary::GetMxTupleUncert() const {
        return (uint) (2*Eps*double(SampleN));
    }

    int TUtils::TGkUtils::TVecSummary::GetBand(const TTuple& Tuple) const {
        // the band groups tuples by their capacity
        // it is defined as alpha, such that:
        // 2^(alpha-1) + mod(p,2^(alpha-1)) <= capacity < 2^alpha + mod(p,2^alpha)
        // where capacity is defined as floor(2*eps*n) - delta_i

        const uint64 MxUncert = (uint64) GetMxTupleUncert();
        const uint UncertRight = Tuple.GetUncertRight();
        const uint64 Capacity = MxUncert - UncertRight;
        AssertR(MxUncert >= UncertRight, "Tuple uncertainty greater than capacity!");

        // Special case: the first 1 / (2*eps) tuples (with d_i == 0) are in a band of their own
        if (UncertRight == 0) { return TInt::Mx; }
        // Special case: we define band 0 to simply be MxUncert
        if (UncertRight == MxUncert) { return 0; }

        const auto TestBand = [&](const int& Band) {
            return TMath::Pow2(Band-1) + (MxUncert % TMath::Pow2(Band-1)) <= Capacity &&
                   Capacity < TMath::Pow2(Band) + (MxUncert % TMath::Pow2(Band));
        };

        const int CandidateBand = (int) TMath::Log2((double) Capacity);
        if (TestBand(CandidateBand)) {
            return CandidateBand;
        }
        else if (TestBand(CandidateBand+1)) {
            return CandidateBand+1;
        }
        else {
            // fail horribly and think about what you did
            const TStr MsgStr = "Could not find the band of tuple: <" + TFlt::GetStr(Tuple.GetMxVal()) + "," + TUInt::GetStr(Tuple.GetTupleSize()) + "," + TInt::GetStr(Tuple.GetUncertRight()) + ">, p=" + TInt::GetStr(MxUncert) + ", candidate band = " + TInt::GetStr(CandidateBand) + "!";
            FailR(MsgStr.CStr());
            return -1;
        }
    }

    ////////////////////////////////////
    /// Biased GK summary - TVec implementation
    TUtils::TBiasedUtils::TVecSummary::TVecSummary(TBiasedGk& _Model, const bool& _UseBands):
            Model(_Model),
            UseBands(_UseBands) {}

    TUtils::TBiasedUtils::TVecSummary::TVecSummary(TBiasedGk& _Model, TSIn& SIn):
            Summary(SIn),
            Model(_Model),
            UseBands(SIn) {}

    void TUtils::TBiasedUtils::TVecSummary::Save(TSOut& SOut) const {
        Summary.Save(SOut);
        UseBands.Save(SOut);
    }

    double TUtils::TBiasedUtils::TVecSummary::Query(const double& PVal) const {
        if (GetSize() == 0) { return 0; }

        const TUInt64& SampleN = Model.GetSampleN();

        const double TargetRank = Model.GetDir() > 0 ? PVal*double(SampleN) : (1 - PVal)*double(SampleN);
        const double EpsRank = Model.GetMxTupleUncert(TargetRank) / 2;

        if (TargetRank <= 1) {
            return Summary.begin()->GetMxVal();
        }

        int CurrMnRank = 0;
        for (int TupleN = 0; TupleN < Summary.Len(); TupleN++) {
            const TTuple& Tuple = Summary[TupleN];
            const uint64 MxRank = CurrMnRank + Tuple.GetTotalUncert();

            if (MxRank > TargetRank + EpsRank) {
                return Summary[TupleN-1].GetMxVal();
            }

            CurrMnRank += Tuple.GetTupleSize();
        }

        return Summary.Last().GetMxVal();
    }

    void TUtils::TBiasedUtils::TVecSummary::Query(const TFltV& PValV, TFltV& QuantV) const {
        if (QuantV.Len() != PValV.Len()) { QuantV.Gen(PValV.Len(), PValV.Len()); }
        if (GetSize() == 0 || PValV.Empty()) { return; }

        // DEBUGGING
        for (int PValN = 1; PValN < PValV.Len(); ++PValN) {
            EAssert(PValV[PValN-1] <= PValV[PValN]);
        }

        const TUInt64& SampleN = Model.GetSampleN();
        const TInt& Dir = Model.GetDir();

        int PValN = 0;
        int TupleN = 0;
        int CurrMnRank = 0;

        while (PValN < PValV.Len()) {
            const int DirPValN = Dir == 1 ? PValN : PValV.Len()-PValN-1;
            const TFlt& PVal = PValV[DirPValN];

            const double TargetRank = Dir > 0 ? PVal*double(SampleN) : (1 - PVal)*double(SampleN);
            const double EpsRank = Model.GetMxTupleUncert(TargetRank) / 2;

            if (TargetRank <= 1) {
                QuantV[DirPValN] = Summary[0].GetMxVal();
            } else {
                while (TupleN < Summary.Len()) {
                    const TTuple& Tuple = Summary[TupleN];
                    const uint64 MxRank = CurrMnRank + Tuple.GetTotalUncert();

                    if (MxRank > TargetRank + EpsRank) {
                        QuantV[DirPValN] = Summary[TupleN-1].GetMxVal();
                        break;
                    }

                    CurrMnRank += Tuple.GetTupleSize();
                    ++TupleN;
                }

                if (TupleN == Summary.Len()) {
                    QuantV[DirPValN] = Summary.Last().GetMxVal();
                }
            }

            ++PValN;
        }
    }

    void TUtils::TBiasedUtils::TVecSummary::Insert(const double& Val) {
        int NewTupleN = 0;
        uint64 TupleFirstItemN = 1;

        while (NewTupleN < Summary.Len()) {
            const TTuple& Tuple = Summary[NewTupleN];
            // stop if Val <= Tuple.GetMxVal()
            if (Model.GetDir()*(Val - Tuple.GetMxVal()) <= 0) { break; }
            TupleFirstItemN += Tuple.GetTupleSize();
            ++NewTupleN;
        }

        if (NewTupleN == 0) {
            // special case
            Summary.Ins(0, TTuple(Val));
        } else if (NewTupleN == Summary.Len()) {
            // special case (symmetrical to the previous case)
            Summary.Add(TTuple(Val));
        } else {
            // let i be the index of the first tuple with greather vi, then
            // insert tuple <v,1,delta_i + g_i - 1>
            TTuple& RightTuple = Summary[NewTupleN];

            if (1 + RightTuple.GetTotalUncert() <= Model.GetMxTupleUncert((double) TupleFirstItemN)) {
                RightTuple.SwallowOne();
            } else {
                Summary.Ins(NewTupleN, TTuple(Val, RightTuple));
            }
        }
    }

    void TUtils::TBiasedUtils::TVecSummary::Compress() {
        if (GetSize() == 0) { return; }

        uint64 ItemSum = Summary[0].GetTupleSize();
        int TupleN = 1;

        while (TupleN < Summary.Len()-1) {
            const TTuple& CurTuple = Summary[TupleN];
            const TTuple& AdjTuple = Summary[TupleN+1];

            const uint CurrTupleSize = CurTuple.GetTupleSize();

            const uint64 TupleStartItemN = ItemSum + 1;
            const uint64 NextTupleStartItemN = ItemSum + CurrTupleSize + 1;

            if (UseBands) {
                const int CurTupleBand = GetBand(CurTuple, TupleStartItemN);
                const int AdjTupleBand = GetBand(AdjTuple, NextTupleStartItemN);
                if (CurTupleBand > AdjTupleBand) {
                    ItemSum += CurTuple.GetTupleSize();
                    ++TupleN;
                    continue;
                }
            }

            const double MxTupleRange = Model.GetMxTupleUncert((double) TupleStartItemN);

            if (CurTuple.GetTupleSize() + AdjTuple.GetTotalUncert() <= MxTupleRange) {
                // merge the tuples
                Summary[TupleN+1].Swallow(Summary[TupleN]);
                Summary.Del(TupleN);
                continue;
            }

            ItemSum += CurTuple.GetTupleSize();
            ++TupleN;
        }
    }

    uint TUtils::TBiasedUtils::TVecSummary::GetSize() const {
        return Summary.Len();
    }

    void TUtils::TBiasedUtils::TVecSummary::Print() const {
        std::cout << Summary << "\n";
    }

    int TUtils::TBiasedUtils::TVecSummary::GetBand(const TTuple& Tuple, const uint64& MnRank) const {
        if (Tuple.GetUncertRight() == 0) { return TInt::Mx; }

        const uint MxTupleRange = (uint) Model.GetMxTupleUncert((double) MnRank);
        const uint TupleDelta = Tuple.GetUncertRight();
        const uint Capacity = MxTupleRange - TupleDelta;
        /* Print(); */
        AssertR(MxTupleRange >= TupleDelta, "Tuple uncertainty smaller than max range, max range: " + TInt::GetStr(MxTupleRange) + ", uncertainty: " + TInt::GetStr(TupleDelta) + "!");

        if (Capacity == 0) { return 0; }

        const auto TestBand = [&](const int& Band) {
            return TMath::Pow2(Band-1) + (MxTupleRange % TMath::Pow2(Band-1)) <= Capacity &&
                   Capacity < TMath::Pow2(Band) + (MxTupleRange % TMath::Pow2(Band));
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
            Print();
            const TStr MsgStr = "Could not find the band of tuple: <" +
                TFlt::GetStr(Tuple.GetMxVal()) + "," + TUInt::GetStr(Tuple.GetTupleSize()) + "," +
                TUInt::GetStr(Tuple.GetUncertRight()) + ">, max size=" + TInt::GetStr(MxTupleRange) +
                ", max tuple size: " + TUInt::GetStr(MxTupleRange) +
                ", delta: " + TUInt::GetStr(TupleDelta) +
                ", capacity: " + TUInt::GetStr(Capacity) +
                ", candidate band = " + TInt::GetStr(CandidateBand) + "!";
            FailR(MsgStr.CStr());
            return -1;
        }
    }


    ////////////////////////////////////
    /// GK - algorithm
    TGk::TGreenwaldKhanna(const double& _Eps):
            Summary(_Eps),
            Eps(_Eps) {}

    TGk::TGreenwaldKhanna(const double& _Eps, const TCompressStrategy& Cs):
            Summary(_Eps),
            Eps(_Eps),
            CompressStrategy(Cs) {}

    TGk::TGreenwaldKhanna(TSIn& SIn):
            Summary(SIn),
            Eps(SIn) {

        const TCh RawCmp(SIn);
        switch (RawCmp.Val) {
            case static_cast<char>(TCompressStrategy::csAuto): {
                CompressStrategy = TCompressStrategy::csAuto;
                break;
            }
            case static_cast<char>(TCompressStrategy::csManual): {
                CompressStrategy = TCompressStrategy::csManual;
                break;
            }
            default: {
                throw TExcept::New("Invalid compression strategy!");
            }
        }
    }

    void TGk::Save(TSOut& SOut) const {
        Summary.Save(SOut);
        Eps.Save(SOut);
        TCh(static_cast<char>(CompressStrategy)).Save(SOut);
    }

    double TGk::Query(const double& Quantile) const {
        return Summary.Query(Quantile);
    }

    void TGk::Query(const TFltV& PValV, TFltV& QuantV) const {
        Summary.Query(PValV, QuantV);
    }

    void TGk::Insert(const double& Val) {
        Summary.Insert(Val);

        if (ShouldAutoCompress()) {
            Compress();
        }
    }

    void TGk::Compress() {
        Summary.Compress();
    }

    int TGk::GetSummarySize() const {
        return Summary.GetSize();
    }

    void TGk::PrintSummary() const {
        std::cout << Summary << "\n";
    }

    uint32 TGk::GetCompressInterval() const {
        return uint(std::ceil(1.0 / (2.0*Eps)));
    }

    bool TGk::ShouldAutoCompress() const {
        return CompressStrategy == TCompressStrategy::csAuto &&
               Summary.GetSampleN() % GetCompressInterval() == 0;
    }

    //////////////////////////////////////////
    /// The CKMS (generalization of GK) algorithm for online
    /// biased quantile estimation.
    TBiasedGk::TBiasedGk(const double& _PVal0, const double& _Eps,
                const TCompressStrategy& _Cs, const bool& _UseBands):
            Summary(*this, _UseBands),
            PVal0(_PVal0 <= 0.5 ? _PVal0 : 1 - _PVal0),
            Eps(_Eps),
            Dir(_PVal0 <= 0.5 ? 1 : -1),
            CompressStrategy(_Cs) {

        EAssert(Eps > 0.0);
        EAssert(0 <= PVal0 && PVal0 <= 1);
    }

    TBiasedGk::TBiasedGk(TSIn& SIn):
            Summary(*this, SIn),
            SampleN(SIn),
            PVal0(SIn),
            Eps(SIn),
            Dir(SIn) {
        const TCh Cs(SIn);

        switch (Cs.Val) {
        case static_cast<std::underlying_type<TCompressStrategy>::type>(TCompressStrategy::csManual): {
            CompressStrategy = TCompressStrategy::csManual;
            break;
        }
        case static_cast<std::underlying_type<TCompressStrategy>::type>(TCompressStrategy::csAggressive): {
            CompressStrategy = TCompressStrategy::csAggressive;
            break;
        }
        case static_cast<std::underlying_type<TCompressStrategy>::type>(TCompressStrategy::csPeriodic): {
            CompressStrategy = TCompressStrategy::csPeriodic;
            break;
        }
        default: {
            throw TExcept::New("Invalid compress strategy when deserializing TBiasedGk!");
        }
        }
    }

    void TBiasedGk::Save(TSOut& SOut) const {
        Summary.Save(SOut);
        SampleN.Save(SOut);
        PVal0.Save(SOut);
        Eps.Save(SOut);
        Dir.Save(SOut);
        TCh(static_cast<std::underlying_type<TCompressStrategy>::type>(CompressStrategy)).Save(SOut);
    }

    double TBiasedGk::Query(const double& PVal) const {
        return Summary.Query(PVal);
    }

    void TBiasedGk::Query(const TFltV& PValV, TFltV& QuantV) const {
        Summary.Query(PValV, QuantV);
    }

    void TBiasedGk::Insert(const double& Val) {
        Summary.Insert(Val);
        ++SampleN;

        if (ShouldCompress()) {
            Compress();
        }
    }

    void TBiasedGk::Compress() {
        Summary.Compress();
    }

    const TFlt& TBiasedGk::GetEps() const {
        return Eps;
    }

    double TBiasedGk::GetPVal0() const {
        return Dir > 0 ? PVal0.Val : 1 - PVal0.Val;
    }

    const TBiasedGk::TCompressStrategy& TBiasedGk::GetCompressStrategy() const {
        return CompressStrategy;
    }

    const TBool& TBiasedGk::GetUseBands() const {
        return Summary.GetUseBands();
    }

    int TBiasedGk::GetSummarySize() const {
        return Summary.GetSize();
    }

    void TBiasedGk::PrintSummary() const {
        Summary.Print();
    }

    double TBiasedGk::GetMxTupleUncert(const double& Rank) const {
        const double PVal = Rank / double(SampleN);
        if (PVal <= PVal0) {
            return 2*Eps*PVal0*SampleN;
        } else {
            return 2*Eps*Rank;
        }
        /* return TMath::Mx(1.0, 2.0*GetEps()*Rank); */
    }

    bool TBiasedGk::ShouldCompress() const {
        switch (CompressStrategy) {
            case TCompressStrategy::csManual: {
                return false;
            }
            case TCompressStrategy::csAggressive: {
                return true;
            }
            case TCompressStrategy::csPeriodic: {
                return SampleN % uint64(std::ceil(0.5 / Eps)) == 0;    // TODO set some smart compress strategy
            }
            default: {
               throw TExcept::New("Invalid compression strategy!");
            }
        }
    }

    ///////////////////////////////////////////////////
    /// GK algorithm for sliding windows
    TSwGk::TSwGk(const double& _EpsGk, const double& _EpsEh):
            Summary(_EpsGk, _EpsEh),
            WinMin(_EpsEh),
            EpsGk(_EpsGk) {
        EAssert(EpsGk < .2);
    }

    TSwGk::TSwGk(TSIn& SIn):
        Summary(SIn),
        WinMin(SIn),
        EpsGk(SIn),
        ForgetTm(SIn),
        SampleN(SIn) {}

    void TSwGk::Save(TSOut& SOut) const {
        Summary.Save(SOut);
        WinMin.Save(SOut);
        EpsGk.Save(SOut);
        ForgetTm.Save(SOut);
        SampleN.Save(SOut);
    }

    double TSwGk::Query(const double& Quantile) {
        if (Quantile <= EpsGk) {
            WinMin.Forget(ForgetTm);
            if (WinMin.Empty()) { return 0; }
            return WinMin.GetMnVal();
        } else {
            return Summary.Query(Quantile);
        }
    }

    void TSwGk::Query(const TFltV& PValV, TFltV& QuantV) {
        // steps:
        // 1) query all the quantiles using the summary (all the p-vals <= eps will be incorrect)
        // 2) correct the p-vals which are <= eps
        Summary.Query(PValV, QuantV);
        // correct pvals less or equal to eps
        if (!PValV.Empty() && PValV[0] <= EpsGk) {
            WinMin.Forget(ForgetTm);
            const double MnVal = WinMin.Empty() ? 0 : WinMin.GetMnVal();

            int PValN = 0;
            while (PValN < PValV.Len() && PValV[PValN] <= EpsGk) {
                QuantV[PValN] = MnVal;
                ++PValN;
            }
        }
    }

    void TSwGk::Insert(const uint64& ValTm, const double& Val) {
        Summary.Insert(ValTm, Val);
        ++SampleN;

        WinMin.Add(ValTm, Val);

        // compress if needed
        if (ShouldCompress()) {
            Compress();
        }
    }

    void TSwGk::Forget(const int64& CutoffTm) {
        ForgetTm = CutoffTm;
        Summary.Forget(ForgetTm);
    }

    void TSwGk::Compress() {
        // compress the summary
        Summary.Compress();
        // also update the minimum tracker
        WinMin.Forget(ForgetTm);
    }

    void TSwGk::Reset() {
        *this = TSwGk(EpsGk, Summary.GetEpsEh());
    }

    const TFlt& TSwGk::GetEpsGk() const {
        return EpsGk;
    }

    const TFlt& TSwGk::GetEpsEh() const {
        return Summary.GetEpsEh();
    }

    int TSwGk::GetSummarySize() const {
        return Summary.GetTupleCount();
    }

    uint64 TSwGk::GetValCount() {
        return Summary.GetValCount();
    }

    uint64 TSwGk::GetValRecount() {
        return Summary.GetValRecount();
    }

    void TSwGk::PrintSummary() const {
        std::cout << Summary << std::endl;
    }

    void TSwGk::PrintMinWinSummary() const {
        WinMin.PrintSummary();
    }

    bool TSwGk::ShouldCompress() const {
        return SampleN % int(ceil(1.0 / (2.0*EpsGk))) == 0;
    }

    ////////////////////////////////////////////
    /// GK algorithm which works on a fixed size window.
    TCountWindowGk::TCountWindowGk(const uint64& _WindowSize, const double& EpsGk, const double& EpsEh):
            TSwGk(EpsGk, EpsEh),
            WindowSize(_WindowSize) {
        EAssertR(WindowSize <= uint64(TInt64::Mx), "Maximum size of window is TInt64::Mx!");
    }

    TCountWindowGk::TCountWindowGk(TSIn& SIn):
        TSwGk(SIn),
        WindowSize(SIn) {}

    void TCountWindowGk::Save(TSOut& SOut) const {
        TSwGk::Save(SOut);
        WindowSize.Save(SOut);
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

    const TUInt64& TCountWindowGk::GetWindowSize() const {
        return WindowSize;
    }

    /////////////////////////////////////////////
    /// Time-window SW-GK
    TTimeWindowGk::TTimeWindowGk(const uint64& _WindowMSec, const double& _EpsGk, const double& _EpsEh):
        TSwGk(_EpsGk, _EpsEh),
        WindowMSec(_WindowMSec) {}

    TTimeWindowGk::TTimeWindowGk(TSIn& SIn):
        TSwGk(SIn),
        WindowMSec(SIn) {}

    void TTimeWindowGk::Save(TSOut& SOut) const {
        TSwGk::Save(SOut);
        WindowMSec.Save(SOut);
    }

    void TTimeWindowGk::Insert(const uint64& ValTm, const double& Val) {
        UpdateTime(ValTm);
        TSwGk::Insert(ValTm, Val);
    }

    void TTimeWindowGk::UpdateTime(const uint64& Tm) {
        const int64 CutoffTm = int64(Tm) - int64(WindowMSec);
        TSwGk::Forget(CutoffTm);
    }

    const TUInt64& TTimeWindowGk::GetWindowMSec() const {
        return WindowMSec;
    }

    std::ostream& operator <<(std::ostream& os, const TUInt& Val) {
        return os << Val.Val;
    }
}

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

    TUtils::TBasicGkTuple::TBasicGkTuple(const double& Val):
        MxVal(Val) {}

    TUtils::TBasicGkTuple::TBasicGkTuple(const double& Val, const TBasicGkTuple& RightTuple):
        MxVal(Val),
        UncertRight(RightTuple.GetUncertRight() + RightTuple.GetTupleSize() - 1) {}

    void TUtils::TBasicGkTuple::Swallow(const TBasicGkTuple& LeftTuple) {
        TupleSize += LeftTuple.TupleSize;
    }

    ////////////////////////////////////
    /// GK - Summary
    TUtils::TGkVecSummary::TGkVecSummary(const double& _Eps):
            Eps(_Eps) {}

    double TUtils::TGkVecSummary::Query(const double& PVal) const {
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

    void TUtils::TGkVecSummary::Query(const TFltV& PValV, TFltV& QuantV) const {
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
        }
    }

    void TUtils::TGkVecSummary::Insert(const double& Val) {
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
            const TTuple& RightTuple = Summary[NewValN];
            Summary.Ins(NewValN, TTuple(Val, RightTuple));
        }

        ++SampleN;
    }

    void TUtils::TGkVecSummary::Compress() {
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

    uint TUtils::TGkVecSummary::GetMxTupleUncert() const {
        return (uint) (2*Eps*double(SampleN));
    }

    int TUtils::TGkVecSummary::GetBand(const TTuple& Tuple) const {
        // the band groups tuples by their capacity
        // it is defined as alpha, such that:
        // 2^(alpha-1) + mod(p,2^(alpha-1)) <= capacity < 2^alpha + mod(p,2^alpha)
        // where capacity is defined as floor(2*eps*n) - delta_i

        const uint64 MxUncert = (uint64) GetMxTupleUncert();
        const uint UncertRight = Tuple.GetUncertRight();
        const uint64 Capacity = MxUncert - UncertRight;

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
    /// GK - algorithm
    TGk::TGreenwaldKhanna(const double& _Eps):
            Summary(_Eps),
            CompressInterval(uint(std::ceil(1.0 / (2.0*_Eps)))) {}

    TGk::TGreenwaldKhanna(const double& _Eps, const TCompressStrategy& Cs):
            Summary(_Eps),
            CompressStrategy(Cs) {}

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

    bool TGk::ShouldAutoCompress() const {
        return CompressStrategy == TCompressStrategy::csAuto &&
               Summary.GetSampleN() % CompressInterval == 0;
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

        const double TargetRank = Dir > 0 ? Quantile*double(SampleN) : (1 - Quantile)*double(SampleN);
        const double EpsRank = GetMxTupleSize(TargetRank) / 2;

        if (TargetRank <= 1) { return Summary[0].GetVal(); }

        int CurrMnRank = 0;
        for (int TupleN = 0; TupleN < Summary.Len(); TupleN++) {
            const TTuple& Tuple = Summary[TupleN];
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
            const TTuple& Tuple = Summary[TupleN];
            if (Dir*(Val - Tuple.GetVal()) <= 0) { break; }
            PrevTupleMnRank += Tuple.GetTupleSize();
            TupleN++;
        }

        if (TupleN == 0) {
            // special case
            Summary.Ins(0, TTuple(Val, 1, 0));
        } else if (TupleN == Summary.Len()) {
            // special case (symmetrical to the previous case)
            Summary.Add(TTuple(Val, 1, 0));
        } else {
            const uint NewTupleMxSize = (uint) GetMxTupleSize((double) PrevTupleMnRank);
            Summary.Ins(TupleN, TTuple(Val, 1, NewTupleMxSize-1));
            AssertR(int(NewTupleMxSize)-1 >= 0, "Max tuple size is 0, but should be at least 1!");
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
            const TTuple& CurTuple = Summary[TupleN];
            const TTuple& AdjTuple = Summary[TupleN+1];

            const uint64 PrevMnRank = CurrMnRank;

            CurrMnRank += CurTuple.GetTupleSize();

            const uint NewTupleRange = CurTuple.GetTupleSize() +
                                       AdjTuple.GetTupleSize() +
                                       AdjTuple.GetMnMxRankDiff();
            const double MxTupleRange = GetMxTupleSize((double) PrevMnRank);

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
        return TMath::Mx(1.0, 2.0*GetEps(Rank / double(SampleN))*Rank);
    }

    bool TBiasedGk::ShouldCompress() const {
        return SampleN % uint64(std::ceil(0.5 / Eps)) == 0;    // TODO set some smart compress strategy
    }

    int TBiasedGk::GetBand(const TTuple& Tuple, const uint64& MnRank) const {
        if (Tuple.GetMnMxRankDiff() == 0) { return TInt::Mx; }

        const uint MxTupleSize = (uint) GetMxTupleSize((double) MnRank);
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

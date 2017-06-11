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

    //////////////////////////////////////////////
    /// Exponential Histogram Base
    template <typename TInterval>
    TExpHistBase<TInterval>::TExpHistBase(const uint64& WindowMSec, const double& _Eps):
            WindowMSec(WindowMSec),
            Eps(_Eps) {
        EAssert(0 < Eps && Eps <= .5);
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetCount() const {
        if (IntervalV.Empty()) { return 0; }
        return TotalCount - IntervalV[0].GetCount() / 2;
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Swallow(const TExpHistBase<TInterval>& Other) {
        Assert(WindowMSec == Other.WindowMSec);
        Assert(Eps == Other.Eps);

        TIntervalV NewIntervalV;

        const TIntervalV& IntervalV1 = IntervalV;
        const TIntervalV& IntervalV2 = Other.IntervalV;

        uint CurrBlockSize = TMath::Mx(IntervalV[0].GetCount(), Other.IntervalV[0].GetCount());
        TUIntUInt64Pr CarryInfo;

        const TIntervalV* OpenVPtr = nullptr;
        int* OpenNPtr = nullptr;

        int Interval1N = 0;
        int Interval2N = 0;

        while (Interval1N < IntervalV1.Len() && Interval2N < IntervalV2.Len()) {
            if (OpenVPtr == nullptr) {      // no interval open
                // find the interval that starts first and open it
                const TInterval& Interval1 = IntervalV1[Interval1N];
                const TInterval& Interval2 = IntervalV2[Interval2N];

                if (Interval1.GetStartTm() <= Interval2.GetStartTm()) {
                    // open Interval1
                    if (Interval1.GetCount() == 1) {
                        // only 1 item, open and close immediately
                        MergeAddItemBatch(
                                1,
                                Interval1.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        ++Interval1N;
                    } else {
                        MergeAddItemBatch(
                                Interval1.GetCount() >> 1,
                                Interval1.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        OpenVPtr = &IntervalV1;
                        OpenNPtr = &Interval1N;
                    }
                }
                else {
                    if (Interval2.GetCount() == 1) {
                        // only 1 item, open and close immediately
                        MergeAddItemBatch(
                                1,
                                Interval2.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        ++Interval2N;
                    } else {
                        MergeAddItemBatch(
                                Interval2.GetCount() >> 1,
                                Interval2.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        OpenVPtr = &IntervalV2;
                        OpenNPtr = &Interval2N;
                    }
                }
            }
            else {
                // one interval hasn't finished yet
                const TIntervalV& OpenIntervalV = *OpenVPtr;
                const TIntervalV& OthrIntervalV = OpenVPtr == &IntervalV ? IntervalV2 : IntervalV1;

                int& OpenN = *OpenNPtr;
                int& OthrN = OpenNPtr == &Interval1N ? Interval2N : Interval1N;

                const TInterval& OpenInterval = OpenIntervalV[OpenN];
                const TInterval& OthrInterval = OthrIntervalV[OthrN];

                Assert(OpenInterval.GetCount() > 1);

                // case 1: the open interval closes before the new one opens
                // add the batch of items from the open interval, close it and
                // go to the next iteration
                if (OpenInterval.GetEndTm() <= OthrInterval.GetStartTm()) {
                    MergeCloseInterval(
                            OpenIntervalV,
                            OpenN,
                            OthrIntervalV,
                            OthrN,
                            CurrBlockSize,
                            CarryInfo,
                            NewIntervalV
                    );
                    OpenVPtr = nullptr;
                    OpenNPtr = nullptr;
                }
                // case 2: the new interval starts before the open one closes
                //  1) add the batch from the start of the new interval
                //  2) add the batch from the end of the interval that closes first
                //  and go to next iteration
                else {
                    // if the new interval only has 1 element, then the open interval will remain open
                    if (OthrInterval.GetCount() == 1) {
                        MergeAddItemBatch(
                                1,
                                OthrInterval.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        ++OthrN;
                    } else {
                        // add the batch from the start of the new interval
                        MergeAddItemBatch(
                                OthrInterval.GetCount() >> 1,
                                OthrInterval.GetStartTm(),
                                CurrBlockSize,
                                CarryInfo,
                                NewIntervalV
                        );
                        // both intervals are now open, now close the one that finishes first
                        if (OthrInterval.GetEndTm() <= OpenInterval.GetEndTm()) {
                            // close the new interval, the open interval remains open
                            MergeCloseInterval(
                                    OthrIntervalV,
                                    OthrN,
                                    OpenIntervalV,
                                    OpenN,
                                    CurrBlockSize,
                                    CarryInfo,
                                    NewIntervalV
                            );
                        } else {
                            // close the open interval
                            MergeCloseInterval(
                                    OpenIntervalV,
                                    OpenN,
                                    OthrIntervalV,
                                    OthrN,
                                    CurrBlockSize,
                                    CarryInfo,
                                    NewIntervalV
                            );
                            OpenVPtr = &OthrIntervalV;
                            OpenNPtr = &OthrN;
                        }
                    }
                }
            }
        }

        // copy the rest of both the vectors into the new one
        while (Interval1N < IntervalV1.Len()) {
            NewIntervalV.Add(IntervalV1[Interval1N]);
            ++Interval1N;
        }
        while (Interval2N < IntervalV2.Len()) {
            NewIntervalV.Add(IntervalV2[Interval2N]);
            ++Interval2N;
        }

        // swap the changes into this object
        std::swap(IntervalV, NewIntervalV);
        // sync the counts
        TotalCount = 0;
        LogSizeToBlockCountV.Gen(TMath::Mx(LogSizeToBlockCountV.Len(), Other.LogSizeToBlockCountV.Len()));
        for (int IntervalN = 0; IntervalN < IntervalV.Len(); IntervalN++) {
            const TInterval& Interval = IntervalV[IntervalN];
            ++BlockSizeToBlockCount(Interval.GetCount());
            TotalCount += Interval.GetCount();
        }

        // compress the resulting EH
        const uint MxBlocksPerCount = GetMxBlocksSameSize();
        int CurrPos = IntervalV.Len()-1;
        for (int LogBlockSize = 0; LogBlockSize < LogSizeToBlockCountV.Len(); LogBlockSize++) {
            while (LogBlockSizeToBlockCount(LogBlockSize) > MxBlocksPerCount) {
                CompressOldestInBatch(CurrPos);
                --CurrPos;
            }
            CurrPos -= LogBlockSizeToBlockCount(LogBlockSize);
        }

        // forget any measurements which fell outside the window
        if (!IntervalV.Empty()) {
            Forget(IntervalV.Last().GetEndTm());
        }

        // finished, assert that all invariants hold
        AssertR(CheckInvariant1(), "EH: Invariant 1 fails after merge!");
        AssertR(CheckInvariant2(), "EH: Invariant 2 fails after merge!");
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetSummarySize() const {
        return IntervalV.Len();
    }

    template <typename TInterval>
    bool TExpHistBase<TInterval>::CheckInvariant1() const {
        if (IntervalV.Empty()) { return true; }

        const uint MxErr = IntervalV[0].GetCount() >> 1;

        uint OtherIntervalSum = 0;
        for (int IntervalN = 1; IntervalN < IntervalV.Len(); IntervalN++) {
            OtherIntervalSum += IntervalV[IntervalN].GetCount();
        }

        // checks if val <= n*(1 + eps)
        return MxErr <= 1 + Eps*(1 + OtherIntervalSum);
    }

    template <typename TInterval>
    bool TExpHistBase<TInterval>::CheckInvariant2() const {
        PrintSummary();
        const uint MnBlocksSameSize = GetMnBlocksSameSize();
        const uint MxBlocksSameSize = GetMxBlocksSameSize();
        // 1) bucket sizes are non-decreasing
        // 2) bucket sizes are a power of 2
        // 3) the power of the bucket size is less than the number of intervals
        // 4) for every bucket size except the last, there are at least k/2 and at
        //    most k/2+1 buckets of that size
        int CurrBucketSize = 0;
        for (int IntervalN = IntervalV.Len()-1; IntervalN >= 0; IntervalN--) {
            const int BucketSize = IntervalV[IntervalN].GetCount();
            // 1)
            /* std::cout << "check 1\n"; */
            if (BucketSize < CurrBucketSize) { return false; }
            // 2)
            /* std::cout << "check 2\n"; */
            if (!TMath::IsPow2(BucketSize)) { return false; }
            // 3)
            /* std::cout << "check 3\n"; */
            if (int(TMath::Log2(BucketSize)) > IntervalV.Len()) { return false; }

            CurrBucketSize = BucketSize;
        }

        /* std::cout << "check 4, k/2 = " << MnBlocksSameSize << ", k/2+1 = " << MxBlocksSameSize << "\n"; */
        /* std::cout << LogSizeToBlockCountV << "\n"; */
        // 4)
        for (int LogSize = 0; LogSize < LogSizeToBlockCountV.Len()-1; LogSize++) {
            const TUInt& BucketCount = LogSizeToBlockCountV[LogSize];
            // skip the last set of buckets (those can have lower counts)
            if (LogSizeToBlockCountV[LogSize+1] == 0) { break; }
            // check if the number of buckets is in correct range
            if (BucketCount < MnBlocksSameSize || BucketCount > MxBlocksSameSize) {
                return false;
            }
        }
        // all checks pass
        return true;
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Add(const TInterval& Interval) {
        std::cout << "adding\n";
        Assert(Interval.GetCount() == 1);
        Assert(Interval.GetDurMSec() == 0);

        IntervalV.Add(Interval);

        ++LogBlockSizeToBlockCount(0);
        ++TotalCount;

        Compress();
        Forget(Interval.GetStartTm());

        AssertR(CheckInvariant1(), "EH: Invariant 1 fails after add!");
        AssertR(CheckInvariant2(), "EH: Invariant 2 fails after add!");
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Compress() {
        std::cout << "compressing\n";
        const uint MxBlocksPerCount = GetMxBlocksSameSize();

        uint CurrBlockPos = IntervalV.Len()-1;
        for (int LogBlockSize = 0; LogBlockSize < LogSizeToBlockCountV.Len(); LogBlockSize++) {
            const uint BlockCount = LogBlockSizeToBlockCount(LogBlockSize);
            std::cout<< "block count " << BlockCount << "\n";

            if (BlockCount <= MxBlocksPerCount) { break; }
            std::cout << "will compress\n";

            CompressOldestInBatch(CurrBlockPos);
            CurrBlockPos -= BlockCount - 1;
        }

        std::cout << "finished compression, block sizes: " << LogSizeToBlockCountV << "\n";
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Forget(const uint64& CurrTm) {
        std::cout << "forgetting, block sizes: " << LogSizeToBlockCountV << "\n";
        PrintSummary();
        const uint64 CutoffTm = CurrTm >= WindowMSec ? CurrTm - WindowMSec : 0ul;

        while (!IntervalV.Empty() && IntervalV[0].GetEndTm() < CutoffTm) {
            const TInterval RemovedInterval = IntervalV[0];
            const uint IntervalSize = RemovedInterval.GetCount();

            std::cout << "forgetting interval: " << RemovedInterval << "\n";
            TotalCount -= IntervalSize;
            --BlockSizeToBlockCount(IntervalSize);
            IntervalV.Del(0);

            OnIntervalForgotten(RemovedInterval);
        }

        std::cout << "finished forgetting, block sizes: " << LogSizeToBlockCountV << "\n";
        PrintSummary();
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::CompressOldestInBatch(const int& BatchPos) {
        Assert(0 <= BatchPos && BatchPos < IntervalV.Len());
        std::cout << "compressing oldest in batch, index: "<< BatchPos << ", summary: " << IntervalV << "\n";

        const TInterval& FirstInterval = IntervalV[BatchPos];
        const uint LogBlockSize = TMath::Log2(FirstInterval.GetCount());
        const uint& BlockCount = LogBlockSizeToBlockCount(LogBlockSize);

        const int DelElN = BatchPos - BlockCount + 2;
        GetPrevInterval(DelElN).Swallow(GetInterval(DelElN));
        IntervalV.Del(DelElN);

        LogBlockSizeToBlockCount(LogBlockSize) -= 2;
        ReserveStructures(LogBlockSize+1);
        ++LogBlockSizeToBlockCount(LogBlockSize+1);
    }

    template <typename TInterval>
    TInterval& TExpHistBase<TInterval>::GetInterval(const int& IntervalN) {
        Assert(IntervalN >= 0);
        return IntervalV[IntervalN];
    }

    template <typename TInterval>
    TInterval& TExpHistBase<TInterval>::GetPrevInterval(const int& IntervalN) {
        Assert(IntervalN > 0);
        return IntervalV[IntervalN-1];
    }

    template <typename TInterval>
    TUInt& TExpHistBase<TInterval>::LogBlockSizeToBlockCount(const uint& LogBlockSize) {
        Assert(LogBlockSize < uint(LogSizeToBlockCountV.Len()));
        return LogSizeToBlockCountV[LogBlockSize];
    }

    template <typename TInterval>
    TUInt& TExpHistBase<TInterval>::BlockSizeToBlockCount(const uint& BlockSize) {
        Assert(1 <= BlockSize);
        return LogBlockSizeToBlockCount(TMath::Log2(BlockSize));
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::ReserveStructures(const uint& LogBlockSize) {
        while (LogBlockSize >= uint(LogSizeToBlockCountV.Len())) {
            LogSizeToBlockCountV.Add(0);
        }
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetMnBlocksSameSize() const {
        return std::ceil(1 / (2*Eps));
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetMxBlocksSameSize() const {
        return GetMnBlocksSameSize() + 1;
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm, const uint& BlockSize,
            TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV) {
        if (CarryInfo.Val1 == 0) { CarryInfo.Val2 = BatchTm; }
        CarryInfo.Val1 += BatchSize;
        MergeFlushCarryInfo(BlockSize, BatchTm, CarryInfo, NewIntervalV);
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::MergeFlushCarryInfo(const uint& BlockSize, const uint64 EndTm,
            TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV) {

        if (CarryInfo.Val1 >= BlockSize) {
            const uint64 StartTm = CarryInfo.Val2;
            const uint64 Dur = EndTm - StartTm;
            NewIntervalV.Add(TInterval(StartTm, Dur, BlockSize));
            CarryInfo.Val1 -= BlockSize;
            CarryInfo.Val2 = EndTm;
        }
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::MergeCloseInterval(const TIntervalV& IntervalV, int& OpenN,
            const TIntervalV& OthrIntervalV, const int& OthrN,
            uint& CurrBlockSize, TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV) {

        // close the interval
        const TInterval& OpenInterval = IntervalV[OpenN];
        const uint64 EndTm = OpenInterval.GetEndTm();
        MergeAddItemBatch(
                OpenInterval.GetCount() >> 1,
                EndTm,
                CurrBlockSize,
                CarryInfo,
                NewIntervalV
        );
        ++OpenN;
        // check if the block size changed, if so then flush the carry info as necessary
        const uint Count1 = OpenN < IntervalV.Len() ? IntervalV[OpenN].GetCount().Val : 1u;
        const uint Count2 = OthrN < OthrIntervalV.Len() ? OthrIntervalV[OthrN].GetCount().Val : 1u;
        const uint NewBlockSize = TMath::Mx(Count1, Count2);
        // create new blocks while there are as many items in the carry info
        // as the new block size
        while (CurrBlockSize > NewBlockSize) {
            CurrBlockSize >>= 1;
            MergeFlushCarryInfo(CurrBlockSize, EndTm, CarryInfo, NewIntervalV);
        }
    }
}

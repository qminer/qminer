namespace TQuant {

    namespace TUtils {

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
        TExpHistBase<TInterval>::TExpHistBase(const double& _Eps):
                Eps(_Eps) {
            EAssert(0 <= Eps && Eps <= .5);
        }

        template <typename TInterval>
        TExpHistBase<TInterval>::TExpHistBase(TSIn& SIn):
            IntervalV(SIn),
            LogSizeToBlockCountV(SIn),
            Eps(SIn),
            TotalCount(SIn) {}

        template <typename TInterval>
        void TExpHistBase<TInterval>::Save(TSOut& SOut) const {
            IntervalV.Save(SOut);
            LogSizeToBlockCountV.Save(SOut);
            Eps.Save(SOut);
            TotalCount.Save(SOut);
        }

        template <typename TInterval>
        TExpHistBase<TInterval>::TExpHistBase(const TExpHistBase<TInterval>& Other):
                IntervalV(Other.IntervalV),
                LogSizeToBlockCountV(Other.LogSizeToBlockCountV),
                Eps(Other.Eps),
                TotalCount(Other.TotalCount),
                Callback(Other.Callback) {}

        template <typename TInterval>
        TExpHistBase<TInterval>& TExpHistBase<TInterval>::operator =(const TExpHistBase<TInterval>& Other) {
            // provide the strong guarantee (if there is an exception then no data is changed)
            TExpHistBase<TInterval> Temp(Other);
            std::swap(*this, Temp);
            return *this;
        }

        template <typename TInterval>
        TExpHistBase<TInterval>::TExpHistBase(TExpHistBase<TInterval>&& Other):
                IntervalV(std::move(Other.IntervalV)),
                LogSizeToBlockCountV(std::move(Other.LogSizeToBlockCountV)),
                Eps(std::move(Other.Eps)),
                TotalCount(std::move(Other.TotalCount)),
                Callback(Other.Callback) {}

        template <typename TInterval>
        TExpHistBase<TInterval>& TExpHistBase<TInterval>::operator =(TExpHistBase&& Other) {
            if (this != &Other) {
                std::swap(IntervalV, Other.IntervalV);
                std::swap(LogSizeToBlockCountV, Other.LogSizeToBlockCountV);
                Eps = Other.Eps;
                TotalCount = Other.TotalCount;
                Callback = Other.Callback;
            }
            return *this;
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::Forget(const int64& ForgetTm) {
            while (!IntervalV.Empty() && !IsInWindow(ForgetTm, IntervalV[0])) {
                const TInterval RemovedInterval = IntervalV[0];
                const uint IntervalSize = RemovedInterval.GetCount();

                TotalCount -= IntervalSize;
                --BlockSizeToBlockCount(IntervalSize);
                IntervalV.Del(0);

                OnIntervalRemoved(RemovedInterval);
            }
        }

        template <typename TInterval>
        uint TExpHistBase<TInterval>::GetCount() const {
            // return the result
            if (IntervalV.Empty()) { return 0; }
            return TotalCount - IntervalV[0].GetCount() / 2;
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::Swallow(const TExpHistBase<TInterval>& Other) {
            AssertR(CheckInvariant1(), "EH: Invariant 1 fails before merge!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails before merge!");
            AssertR(Other.CheckInvariant1(), "EH: Other EH invariant 1 fails before merge!");
            AssertR(Other.CheckInvariant2(), "EH: Other EH invariant 2 fails before merge!");

            // check if any of the intervals are empty, if so, take the non-empty interval
            if (IntervalV.Empty()) { *this = Other; return; }
            if (Other.IntervalV.Empty()) { return; }

            // both intervals are full, make some assertions
            Assert(Eps == Other.Eps);

            // do the work
            TIntervalV NewIntervalV;

            const TIntervalV& IntervalV1 = IntervalV;
            const TIntervalV& IntervalV2 = Other.IntervalV;

            uint CurrBlockSize = TMath::Mx(IntervalV[0].GetCount(), Other.IntervalV[0].GetCount());
            TCarryInfo CarryInfo;   TMergeHelper::UndefineOtherInfo(CarryInfo.Val3);

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
                                    TMergeHelper::ExtractOtherCarryInfo(Interval1),
                                    CurrBlockSize,
                                    CarryInfo,
                                    NewIntervalV
                            );
                            ++Interval1N;
                        } else {
                            MergeAddItemBatch(
                                    Interval1.GetCount() >> 1,
                                    Interval1.GetStartTm(),
                                    TMergeHelper::ExtractOtherCarryInfo(Interval1),
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
                                    TMergeHelper::ExtractOtherCarryInfo(Interval2),
                                    CurrBlockSize,
                                    CarryInfo,
                                    NewIntervalV
                            );
                            ++Interval2N;
                        } else {
                            MergeAddItemBatch(
                                    Interval2.GetCount() >> 1,
                                    Interval2.GetStartTm(),
                                    TMergeHelper::ExtractOtherCarryInfo(Interval2),
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
                                    TMergeHelper::ExtractOtherCarryInfo(OthrInterval),
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
                                    TMergeHelper::ExtractOtherCarryInfo(OthrInterval),
                                    TMergeHelper::ExtractOtherCarryInfo(OpenInterval),
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

            // if any intervals are open, then we must close them
            if (OpenVPtr != nullptr) {
                const TIntervalV& OpenIntervalV = *OpenVPtr;
                const TIntervalV& OthrIntervalV = OpenVPtr == &IntervalV ? IntervalV2 : IntervalV1;

                int& OpenN = *OpenNPtr;
                const int& OthrN = OpenNPtr == &Interval1N ? Interval2N : Interval1N;

                Assert(OpenIntervalV[OpenN].GetCount() > 1);

                MergeCloseInterval(
                        OpenIntervalV,
                        OpenN,
                        OthrIntervalV,
                        OthrN,
                        CurrBlockSize,
                        CarryInfo,
                        NewIntervalV
                );
            }

            // finish both the interval vectors
            MergeFinishIntervalV(
                    IntervalV1,
                    Interval1N,
                    IntervalV2,
                    Interval2N,
                    CurrBlockSize,
                    CarryInfo,
                    NewIntervalV
            );
            MergeFinishIntervalV(
                    IntervalV2,
                    Interval2N,
                    IntervalV1,
                    Interval1N,
                    CurrBlockSize,
                    CarryInfo,
                    NewIntervalV
            );

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

            // let derived classes update their state
            OnAfterSwallow();

            // finished, assert that all invariants hold
            AssertR(CheckInvariant1(), "EH: Invariant 1 fails after merge!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails after merge!");
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::DelNewest() {
            Assert(!IntervalV.Empty());

            const TInterval Deleted = IntervalV.Last();
            const uint DelCount = Deleted.GetCount();
            IntervalV.DelLast();

            TotalCount -= DelCount;
            --BlockSizeToBlockCount(DelCount);

            BreakBlocks();

            // notify the subclasses so they can update
            OnIntervalRemoved(Deleted);

            // finished, assert that all invariants hold
            AssertR(CheckInvariant1(), "EH: Invariant 1 fails after DelNewest!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails after DelNewest!");
        }

        template <typename TInterval>
        uint TExpHistBase<TInterval>::GetSummarySize() const {
            return IntervalV.Len();
        }

        template <typename TInterval>
        uint64 TExpHistBase<TInterval>::GetMemUsed() const {
            return sizeof(TExpHistBase) +
                TMemUtils::GetExtraContainerSizeShallow(IntervalV) +
                TMemUtils::GetExtraContainerSizeShallow(LogSizeToBlockCountV) +
                TMemUtils::GetExtraMemberSize(Eps) +
                TMemUtils::GetExtraMemberSize(TotalCount);
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
            const bool Success = MxErr <= 1 + Eps*(1 + OtherIntervalSum);
            if (!Success) {
                std::cout << "invariant 1 failed!" << std::endl;
                std::cout << "max error: " << MxErr << ", other interval sum: " << OtherIntervalSum << std::endl;
                std::cout << "Summary:" << std::endl;
                PrintSummary();
            }
            return Success;
        }

        template <typename TInterval>
        bool TExpHistBase<TInterval>::CheckInvariant2() const {
            const uint MnBlocksSameSize = GetMnBlocksSameSize();
            const uint MxBlocksSameSize = GetMxBlocksSameSize();
            // 1) bucket sizes are non-decreasing
            // 2) bucket sizes are a power of 2
            // 3) the power of the bucket size is less than the number of intervals
            // 4) check that the interval times are rising
            // 5) for every bucket size except the last, there are at least k/2 and at
            // 6) check that the total counts match
            //    most k/2+1 buckets of that size
            int CurrBucketSize = 0;
            uint TotalCountLogStruct = 0;
            uint TotalCountBuckets = 0;
            for (int IntervalN = IntervalV.Len()-1; IntervalN >= 0; IntervalN--) {
                const int BucketSize = IntervalV[IntervalN].GetCount();
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
                // 4)
                if (IntervalN > 0 && IntervalV[IntervalN-1].GetEndTm() > IntervalV[IntervalN].GetStartTm()) {
                    std::cout << "check 4 failed " << IntervalV[IntervalN-1] << " > " << IntervalV[IntervalN] << std::endl;
                    PrintSummary();
                    return false;
                }

                TotalCountBuckets += BucketSize;
                CurrBucketSize = BucketSize;
            }
            // 5)
            for (int LogSize = 0; LogSize < LogSizeToBlockCountV.Len()-1; LogSize++) {
                const TUInt& BucketCount = LogSizeToBlockCountV[LogSize];
                // skip the last set of buckets (those can have lower counts)
                if (LogSizeToBlockCountV[LogSize+1] == 0) { break; }
                // check if the number of buckets is in correct range
                if (BucketCount < MnBlocksSameSize || BucketCount > MxBlocksSameSize) {
                    std::cout << "check on the number of buchets of size " << (1 << LogSize) << " failed" << std::endl;
                    std::cout << "bucket count of size " << (1 << LogSize) << " is " << BucketCount << " min is " << MnBlocksSameSize << " max is " << MxBlocksSameSize << std::endl;
                    PrintSummary();
                    return false;
                }
            }
            // 6)
            for (int LogSize = 0; LogSize < LogSizeToBlockCountV.Len(); LogSize++) {
                const TUInt& BucketCount = LogSizeToBlockCountV[LogSize];
                TotalCountLogStruct += BucketCount * (1 << LogSize);
            }
            if (TotalCount != TotalCountBuckets || TotalCount != TotalCountLogStruct) {
                std::cout << "check 6 failed total count: " << TotalCount << ", total count buckets: " << TotalCountBuckets << ", total count log struct: " << TotalCountLogStruct << std::endl;
                std::cout << "log struct: " << LogSizeToBlockCountV << ", summary: ";
                PrintSummary();
                return false;
            }
            // all checks pass
            return true;
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::Add(const TInterval& Interval) {
            Assert(Interval.GetCount() == 1);
            Assert(Interval.GetDurMSec() == 0);

            IntervalV.Add(Interval);

            ++LogBlockSizeToBlockCount(0);
            ++TotalCount;

            Compress();

            AssertR(CheckInvariant1(), "EH: Invariant 1 fails after add!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails after add!");
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::Compress() {
            const uint MxBlocksPerCount = GetMxBlocksSameSize();

            uint CurrBlockPos = IntervalV.Len()-1;
            for (int LogBlockSize = 0; LogBlockSize < LogSizeToBlockCountV.Len(); LogBlockSize++) {
                const uint BlockCount = LogBlockSizeToBlockCount(LogBlockSize);

                if (BlockCount <= MxBlocksPerCount) { break; }

                CompressOldestInBatch(CurrBlockPos);
                CurrBlockPos -= BlockCount - 1;
            }

            AssertR(CheckInvariant1(), "EH: Invariant 1 fails after compress!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails after compress!");
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::OnIntervalRemoved(const TInterval& Interval) {
            if (Callback != nullptr) {
                Callback->OnItemsDeleted(Interval.GetCount());
            }
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::OnAfterSwallow() {}

        template <typename TInterval>
        void TExpHistBase<TInterval>::BreakBlocks(const int& StartBlockSize) {
            const int StartLogSize = (int) TMath::Log2(StartBlockSize);

            // move to the correct position
            int CurrPos = IntervalV.Len()-1;
            int CurrLogSize = 0;
            while (CurrLogSize < LogSizeToBlockCountV.Len() && CurrLogSize < StartLogSize) {
                CurrPos -= LogBlockSizeToBlockCount(CurrLogSize);
                ++CurrLogSize;
            }

            // break apart blocks if needed to satisfy the invariants
            while (CurrLogSize+1 < LogSizeToBlockCountV.Len()) {
                const uint NumBlocks = LogBlockSizeToBlockCount(CurrLogSize);

                // check if this set complies with the min number of blocks
                if (NumBlocks >= GetMnBlocksSameSize()) { break; }
                // the last set of blocks doen't need to comply with the count
                if (LogBlockSizeToBlockCount(CurrLogSize+1) == 0) { break; }

                // the set doesn't comply with the invariant, split blocks of
                // larger size until it does
                const int SplitPos = CurrPos - NumBlocks;
                const TInterval& SplitInterval = IntervalV[SplitPos];

                // split
                TInterval StartInterval, EndInterval;
                SplitInterval.Split(StartInterval, EndInterval);
                // insert the two intervals (the end interval will override the one we split)
                IntervalV.Ins(SplitPos, StartInterval);
                IntervalV[SplitPos+1] = EndInterval;

                // update the block counts
                LogBlockSizeToBlockCount(CurrLogSize) += 2;
                --LogBlockSizeToBlockCount(CurrLogSize+1);

                // update the current position and block size
                CurrPos -= NumBlocks + 1;
                ++CurrLogSize;
            }

            AssertR(CheckInvariant1(), "EH: Invariant 1 fails after breaking blocks!");
            AssertR(CheckInvariant2(), "EH: Invariant 2 fails after breaking blocks!");
        }

        template <typename TInterval>
        TUInt& TExpHistBase<TInterval>::LogBlockSizeToBlockCount(const uint& LogBlockSize) {
            Assert(LogBlockSize < uint(LogSizeToBlockCountV.Len()));
            return LogSizeToBlockCountV[LogBlockSize];
        }

        template <typename TInterval>
        TUInt& TExpHistBase<TInterval>::BlockSizeToBlockCount(const uint& BlockSize) {
            Assert(1 <= BlockSize);
            return LogBlockSizeToBlockCount((uint) TMath::Log2(BlockSize));
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::CompressOldestInBatch(const int& BatchPos) {
            Assert(0 <= BatchPos && BatchPos < IntervalV.Len());

            const TInterval& FirstInterval = IntervalV[BatchPos];
            const uint LogBlockSize = (uint) TMath::Log2(FirstInterval.GetCount());
            const uint& BlockCount = LogBlockSizeToBlockCount(LogBlockSize);

            const int DelElN = BatchPos - BlockCount + 2;
            GetPrevInterval(DelElN).Swallow(GetInterval(DelElN));
            IntervalV.Del(DelElN);

            LogBlockSizeToBlockCount(LogBlockSize) -= 2;
            ReserveStructures(LogBlockSize+1);
            ++LogBlockSizeToBlockCount(LogBlockSize+1);
        }

        template <typename TInterval>
        uint TExpHistBase<TInterval>::GetMnBlocksSameSize() const {
            if (Eps == 0.0) { return 0; }
            return (uint) std::ceil(1 / (2*Eps));
        }

        template <typename TInterval>
        uint TExpHistBase<TInterval>::GetMxBlocksSameSize() const {
            if (Eps == 0.0) { return TUInt::Mx; }
            return GetMnBlocksSameSize() + 1;
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
        void TExpHistBase<TInterval>::ReserveStructures(const uint& LogBlockSize) {
            while (LogBlockSize >= uint(LogSizeToBlockCountV.Len())) {
                LogSizeToBlockCountV.Add(0);
            }
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm,
                const typename TMergeHelper::TOtherCarryInfo& OtherInfo, const uint& BlockSize,
                TCarryInfo& CarryInfo, TIntervalV& NewIntervalV) {
            if (CarryInfo.Val1 == 0) { CarryInfo.Val2 = BatchTm; }

            CarryInfo.Val1 += BatchSize;
            TMergeHelper::MergeOtherCarryInfo(OtherInfo, CarryInfo.Val3);
            MergeFlushCarryInfo(BlockSize, BatchTm, CarryInfo, NewIntervalV);

            // check if we need to redefine the min/max carry info
            if (CarryInfo.Val1 > 0) {
                TMergeHelper::MergeOtherCarryInfo(OtherInfo, CarryInfo.Val3);
            }
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm,
                const typename TMergeHelper::TOtherCarryInfo& OtherInfo,
                const typename TMergeHelper::TOtherCarryInfo& OpenIntervalOtherInfo,    // XXX the method only tested with max and min intervals
                const uint& BlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV) {
            MergeAddItemBatch(
                    BatchSize,
                    BatchTm,
                    OtherInfo,
                    BlockSize,
                    CarryInfo,
                    NewIntervalV
            );
            // redefine the min/max carry info of the open interval
            TMergeHelper::MergeOtherCarryInfo(OpenIntervalOtherInfo, CarryInfo.Val3);
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::MergeFlushCarryInfo(const uint& BlockSize, const uint64 EndTm,
                TCarryInfo& CarryInfo, TIntervalV& NewIntervalV) {

            if (CarryInfo.Val1 >= BlockSize) {
                const uint64 StartTm = CarryInfo.Val2;
                const uint64 Dur = EndTm - StartTm;
                NewIntervalV.Add(TMergeHelper::CreateInterval(StartTm, Dur, BlockSize, CarryInfo.Val3));
                CarryInfo.Val1 -= BlockSize;
                CarryInfo.Val2 = EndTm;

                // undefine the min/max carry info, if any interval remains open
                // it will have to redefine it
                TMergeHelper::UndefineOtherInfo(CarryInfo.Val3);
            }
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::MergeCloseInterval(const TIntervalV& IntervalV, int& OpenN,
                const TIntervalV& OthrIntervalV, const int& OthrN,
                uint& CurrBlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV) {
            // close the interval
            const TInterval& OpenInterval = IntervalV[OpenN];
            const uint64 EndTm = OpenInterval.GetEndTm();
            MergeAddItemBatch(
                    OpenInterval.GetCount() >> 1,
                    EndTm,
                    TMergeHelper::ExtractOtherCarryInfo(OpenInterval),
                    CurrBlockSize,
                    CarryInfo,
                    NewIntervalV
            );
            // redefine the min/max carry info of the open interval
            if (OthrN < OthrIntervalV.Len()) {
                TMergeHelper::MergeOtherCarryInfo(
                    TMergeHelper::ExtractOtherCarryInfo(OthrIntervalV[OthrN]),
                    CarryInfo.Val3
                );
            }
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

                // redefine the min/max carry info of the open interval
                if (OthrN < OthrIntervalV.Len()) {
                    TMergeHelper::MergeOtherCarryInfo(
                        TMergeHelper::ExtractOtherCarryInfo(OthrIntervalV[OthrN]),
                        CarryInfo.Val3
                    );
                }
            }
        }

        template <typename TInterval>
        void TExpHistBase<TInterval>::MergeFinishIntervalV(const TIntervalV& IntervalV, int& IntervalN,
                const TIntervalV& OtherIntervalV, const int& OtherIntervalN,
                uint& CurrBlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV) {
            while (IntervalN < IntervalV.Len()) {
                const TInterval& Interval = IntervalV[IntervalN];

                // try to flush the carry information
                const uint NewBlockSize = Interval.GetCount();
                while (CurrBlockSize > NewBlockSize) {
                    CurrBlockSize >>= 1;
                    MergeFlushCarryInfo(CurrBlockSize, Interval.GetStartTm(), CarryInfo, NewIntervalV);
                }

                // if the carry information is not empty, then merge the intervals in the
                // vector with the carry info
                if (CarryInfo.Val1 != 0) {
                    Assert(Interval.GetCount() > 1);
                    MergeAddItemBatch(
                            Interval.GetCount() >> 1,
                            Interval.GetStartTm(),
                            TMergeHelper::ExtractOtherCarryInfo(Interval),
                            CurrBlockSize,
                            CarryInfo,
                            NewIntervalV
                    );
                    MergeCloseInterval(
                            IntervalV,
                            IntervalN,
                            OtherIntervalV,
                            OtherIntervalN,
                            CurrBlockSize,
                            CarryInfo,
                            NewIntervalV
                    );
                }
                // if the carry info is empty, then just copy the remaining intervals
                else {
                    NewIntervalV.Add(Interval);
                    ++IntervalN;
                }
            }
        }
    }
}

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
    TExpHistBase<TInterval>::TExpHistBase(const uint64& WindowMSec, const double& Eps):
            WindowMSec(WindowMSec),
            MinBlocksCompress(1 / Eps) {
        EAssert(0 < Eps && Eps <= .5);
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetCount() const {
        if (IntervalV.Empty()) { return 0; }
        return TotalCount - IntervalV[0].GetCount() / 2;
    }

    template <typename TInterval>
    uint TExpHistBase<TInterval>::GetIntervalCount() const {
        return IntervalV.Len();
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Swallow(const TExpHistBase<TInterval>& Other) {
        Assert(WindowMSec == Other.WindowMSec);
        Assert(MinBlocksCompress == Other.MinBlocksCompress);

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
            std::cout << "\n";
            std::cout << "n1: " << Interval1N << ", n2: " << Interval2N << "\n";
            if (OpenVPtr == nullptr) {      // no interval open
                std::cout << "both intervals closed\n";
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
                        std::cout << "opened interval 1\n";
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
                        std::cout << "opened interval 2\n";
                    }
                }
            }
            else {
                std::cout << "one interval open\n";
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
                    std::cout << "open interval closes\n";
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
                        std::cout << "opening the new interval\n";
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
                            std::cout << "closing the new interval\n";
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
                            std::cout << "closing the open interval\n";
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

        // TODO compress

        // finished, assert that all invariants hold
#ifndef NDEBUG
        AssertInvariant1();
        AssertInvariant2();
#endif
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Add(const TInterval& Interval) {
        std::cout << "Adding interval: " << Interval << "\n";
        Assert(Interval.GetCount() == 1);
        Assert(Interval.GetDurMSec() == 0);

        IntervalV.Add(Interval);

        ++LogBlockSizeToBlockCount(0);
        ++TotalCount;

        Compress();
        Forget(Interval.GetStartTm());
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Compress() {
        if (LogBlockSizeToBlockCount(0) < MinBlocksCompress) { return; }
        std::cout << "compressing\n";

        uint CurrBlockPos = IntervalV.Len()-1;
        for (int LogBlockSize = 0; LogBlockSize < LogSizeToBlockCountV.Len(); LogBlockSize++) {
            const uint BlockCount = LogBlockSizeToBlockCount(LogBlockSize);

            if (BlockCount < MinBlocksCompress) { break; }

            // compress
            const int MergeStartN = BlockCount % 2 == 0 ? CurrBlockPos : CurrBlockPos - 1;
            const int NMerges = BlockCount / 2;

            for (int MergeN = 0; MergeN < NMerges; MergeN++) {
                const int DelElN = MergeStartN - 2*MergeN;
                GetPrevInterval(DelElN).Swallow(GetInterval(DelElN));
                IntervalV.Del(DelElN);
            }

            // update the interval counts
            LogBlockSizeToBlockCount(LogBlockSize) -= 2*NMerges;
            ReserveStructures(LogBlockSize+1);
            LogBlockSizeToBlockCount(LogBlockSize+1) += NMerges;

            CurrBlockPos -= BlockCount - NMerges;
        }
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::Forget(const uint64& CurrTm) {
        const uint64 CutoffTm = CurrTm >= WindowMSec ? CurrTm - WindowMSec : 0ul;

        while (!IntervalV.Empty() && IntervalV[0].GetEndTm() < CutoffTm) {
            const TInterval RemovedInterval = IntervalV[0];
            const uint IntervalSize = RemovedInterval.GetCount();

            TotalCount -= IntervalSize;
            --BlockSizeToBlockCount(IntervalSize);
            IntervalV.Del(0);

            OnIntervalForgotten(RemovedInterval);
        }
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
        Assert(LogBlockSize <= uint(LogSizeToBlockCountV.Len()));
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
    void TExpHistBase<TInterval>::AssertInvariant1() const {
        // TODO
    }

    template <typename TInterval>
    void TExpHistBase<TInterval>::AssertInvariant2() const {
        // TODO
    }
}

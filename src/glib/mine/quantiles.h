#ifndef _STREAM_QUANTILES_H
#define _STREAM_QUANTILES_H

#include <list>
#include <iostream>

namespace TQuant {

    namespace TUtils {

        template <typename T1, typename T2, typename T3>
        std::ostream& operator <<(std::ostream& os, const TTriple<T1, T2, T3>& Triple);
        template <typename T>
        std::ostream& operator <<(std::ostream& os, const TVec<T>& Vec);

        ////////////////////////////////////
        /// A tuple class used by the GK-based algorithms
        class TGkTuple {
        private:
            TFlt Val {};
            TUInt TupleSize {};
            TUInt MnMxRankDiff {};

        public:
            TGkTuple();
            TGkTuple(const double& Val, const uint& MnRankDiff, const uint& Delta);

            const TFlt& GetVal() const { return Val; }
            const TUInt& GetTupleSize() const { return TupleSize; }
            const TUInt& GetMnMxRankDiff() const { return MnMxRankDiff; }

            void Swallow(const TGkTuple&);
        };

        ///////////////////////////////////
        /// Helper for the merge operation
        /// for basic intervals
        template <typename TInterval>
        class TBasicMergeHelper {
        public:
            using TOtherCarryInfo = TBool;

            static TInterval CreateInterval(const uint64& StartTm, const uint64& Dur, const uint& Count,
                    const TBool&) {
                return TInterval(StartTm, Dur, Count);
            }
            static TBool ExtractOtherCarryInfo(const TInterval&) { return false; }
            static void MergeOtherCarryInfo(const TBool&, const TBool&) {}
            static void UndefineOtherInfo(const TBool&) {}
        };

        ///////////////////////////////////
        /// Helper for the merge operation
        /// for intervals with max
        template <typename TInterval>
        class TMaxMergeHelper {
        public:
            using TOtherCarryInfo = TFlt;

            static TInterval CreateInterval(const uint64& StartTm, const uint64& Dur,
                    const uint& Count, const double& MxVal) {
                return TInterval(StartTm, Dur, Count, MxVal);
            }
            static TFlt ExtractOtherCarryInfo(const TInterval& Interval) { return Interval.GetMxVal(); }
            static void MergeOtherCarryInfo(const TFlt& MxVal, TFlt& CurrMxVal) {
                if (MxVal > CurrMxVal) {
                    CurrMxVal = MxVal;
                }
            }
            static void UndefineOtherInfo(TFlt& MxVal) {
                MxVal = TFlt::NInf;
            }
        };

        ////////////////////////////////////
        /// Time interval which holds a count
        /// of items
        class TInterval {
        private:
            TUInt64 StartTm;
            TUInt64 DurMSec {};  // initializes to 0, the interval covers [StartTm, StartTm + DurMSec]
            TUInt ElCount {1u};

        public:
            using TMergeHelper = TBasicMergeHelper<TInterval>;

            TInterval();
            TInterval(const uint64& StartTm);
            TInterval(const uint64& StartTm, const uint64& Dur, const uint& Count);

            // SERIALIZATION
            TInterval(TSIn&);
            void Save(TSOut& SOut) const;

            const TUInt& GetCount() const { return ElCount; }
            const TUInt64& GetStartTm() const { return StartTm; }
            const TUInt64& GetDurMSec() const { return DurMSec; }

            uint64 GetEndTm() const;

            void Swallow(const TInterval&);
            void Split(TInterval& StartInterval, TInterval& EndInterval) const {
                Assert(ElCount > 1);
                // the start interval which starts at the start of this one
                // and has duration 0
                StartInterval.StartTm = GetStartTm();
                StartInterval.DurMSec = 0;
                StartInterval.ElCount = GetCount() >> 1;
                // the end interval occurs at the end of this interval
                // and also has duration 0
                EndInterval.StartTm = GetEndTm();
                EndInterval.DurMSec = 0;
                EndInterval.ElCount = GetCount() >> 1;
            }
        };

        ///////////////////////////////////////////
        /// Interval which holds the start time,
        /// duration, item count and maximum value
        class TIntervalWithMax : public TInterval {
        private:
            TFlt MxVal;             // the maximal value merged into the interval

        public:
            using TMergeHelper = TMaxMergeHelper<TIntervalWithMax>;

            TIntervalWithMax();
            TIntervalWithMax(const uint64& BeginTm, const double& Val);
            TIntervalWithMax(const uint64& StartTm, const uint64& Dur, const uint& Cnt,
                    const double& MxVal);

            // SERIALIZATION
            TIntervalWithMax(TSIn&);
            void Save(TSOut&) const;

            const TFlt& GetMxVal() const { return MxVal; }

            void Swallow(const TIntervalWithMax&);
            void Split(TIntervalWithMax& StartInterval, TIntervalWithMax& EndInterval) const {
                TInterval::Split(StartInterval, EndInterval);
                StartInterval.MxVal = MxVal;
                EndInterval.MxVal = MxVal;
            }
        };

        ///////////////////////////////////////////
        /// Interval which holds the start time,
        /// duration, item count and minimum value
        class TIntervalWithMin : public TInterval {
        private:
            TFlt MnVal;

        public:
            TIntervalWithMin();
            TIntervalWithMin(const uint64& BeginTm, const double& Val);
            TIntervalWithMin(const uint64& BeginTm, const uint64& Dur, const uint& Cnt,
                    const double& MnVal);

            // SERIALIZATION
            TIntervalWithMin(TSIn&);
            void Save(TSOut&) const;

            const TFlt& GetMnVal() const { return MnVal; }

            void Swallow(const TIntervalWithMin&);
        };

        //////////////////////////////////////////////
        /// Base class with most of the logic needed by
        /// exponential histograms
        template <typename TInterval>
        class TExpHistBase {
        protected:
            using TMergeHelper = typename TInterval::TMergeHelper;
            using TCarryInfo = TTriple<TUInt, TUInt64, typename TMergeHelper::TOtherCarryInfo>;
            using TIntervalV = TVec<TInterval>;

        public:
            class TExpHistCallback {
            public:
                virtual void OnItemsDeleted(const uint64&) = 0;
            };

            /* TExpHistBase() {} */
            /// Sets the time window and error. The (approximate) count
            /// of the elements in time window [curr_time, curr_time - window)
            /// in the bin is at most (1 + eps)*real_count.
            TExpHistBase(const double& Eps);

            virtual ~TExpHistBase() {}

            // SERIALIZATION
            TExpHistBase(TSIn&);
            void Save(TSOut&) const;

            // COPY/MOVE OPERATIONS
            /// copy
            TExpHistBase(const TExpHistBase<TInterval>&);
            TExpHistBase& operator =(const TExpHistBase<TInterval>&);
            // move
            TExpHistBase(TExpHistBase<TInterval>&&);
            TExpHistBase<TInterval>& operator =(TExpHistBase<TInterval>&&);

            /// forgets all elements which occur at the specified time or before
            virtual void Forget(const int64&);

            /// returns the number of elements in the time window without
            /// shifting the window
            uint GetCount() const;

            /// merges the other exponential histogram into itself
            void Swallow(const TExpHistBase<TInterval>&);
            /// deletes the newest interval in the EH
            void DelNewest();

            /// returns the number of intervals in the summary
            uint GetSummarySize() const;
            /// prints the intervals to stadard output
            void PrintSummary() const { std::cout << IntervalV << "\n"; }

            /// checks if the error is bounded by eps
            bool CheckInvariant1() const;
            /// checks if the internal structure of the EH is correct
            bool CheckInvariant2() const;

            /// sets the callback to notify various events
            void SetCallback(TExpHistCallback& Cb) { Callback = &Cb; }

        protected:
            void Add(const TInterval&);
            void Compress();
            void BreakBlocks(const int& StartBlockSize = 1);

            TUInt& LogBlockSizeToBlockCount(const uint& LogBlockSize);
            TUInt& BlockSizeToBlockCount(const uint& BlockSize);

            void CompressOldestInBatch(const int& BatchPos);

            uint GetMnBlocksSameSize() const;
            uint GetMxBlocksSameSize() const;

            virtual void OnIntervalRemoved(const TInterval& Interval);
            virtual void OnAfterSwallow();

        private:

            TInterval& GetInterval(const int& IntervalN);
            TInterval& GetPrevInterval(const int& IntervalN);

            void ReserveStructures(const uint& LogBlockSize);

            static bool IsInWindow(const int64& ForgetTm, const TInterval& Interval) {
                return ForgetTm < int64(Interval.GetEndTm());
            }

            // helper functions for the merge operation
            static void MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm,
                    const typename TMergeHelper::TOtherCarryInfo&, const uint& BlockSize,
                    TCarryInfo& CarryInfo, TIntervalV& NewIntervalV);
            static void MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm,
                    const typename TMergeHelper::TOtherCarryInfo& BatchOtherInfo,
                    const typename TMergeHelper::TOtherCarryInfo& OpenIntervalOtherInfo,
                    const uint& BlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV);
            static void MergeFlushCarryInfo(const uint& BlockSize, const uint64 EndTm,
                    TCarryInfo& CarryInfo, TIntervalV& NewIntervalV);
            static void MergeCloseInterval(const TIntervalV& IntervalV, int& OpenN,
                    const TIntervalV& OthrIntervalV, const int& OthrN,
                    uint& CurrBlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV);
            static void MergeFinishIntervalV(const TIntervalV& IntervalV, int& IntervalN,
                    const TIntervalV& OtherIntervalV, const int& OtherIntervalN,
                    uint& CurrBlockSize, TCarryInfo& CarryInfo, TIntervalV& NewIntervalV);

        protected:
            // MEMBERS
            TIntervalV IntervalV {};
            TUIntV LogSizeToBlockCountV {1};  // at index i stores the number of blocks of size (i+1)
            TFlt Eps;
            TUInt TotalCount {0u};
            TExpHistCallback* Callback {nullptr}; // notifies certain events
        };

        class TExpHistWithMax;  // TODO change the order

        //////////////////////////////////////////////
        /// Exponential Histogram
        /// (approximately) counts the number of items
        /// in a time window
        class TExpHistogram : public TExpHistBase<TInterval> {
            friend class TExpHistWithMax;
        private:
            using TIntervalType = TInterval;
            using TBase = TExpHistBase<TIntervalType>;

        public:
            TExpHistogram(const double& Eps): TExpHistBase(Eps) {}
            TExpHistogram(TSIn& SIn): TExpHistBase(SIn) {}

            void Add(const uint64& Tm);
        };

        //////////////////////////////////////////////
        /// Exponential Histogram which also holds the
        /// maximum of the items
        class TExpHistWithMax : public TExpHistBase<TIntervalWithMax> {
            using TBase = TExpHistBase<TIntervalWithMax>;
        public:
            TExpHistWithMax(const double& Eps): TExpHistBase(Eps) {}

            // SERIALIZATION
            TExpHistWithMax(TSIn&);
            void Save(TSOut&) const;

            // COPY/MOVE operations
            // copy
            TExpHistWithMax(const TExpHistWithMax&);
            TExpHistWithMax& operator =(const TExpHistWithMax&);
            // move
            TExpHistWithMax(TExpHistWithMax&&);
            TExpHistWithMax& operator =(TExpHistWithMax&&);

            void Forget(const int64&);

            /// adds a new item
            void Add(const uint64& Tm, const double& Val);
            /// returns the maximum value in the time window without
            /// shifting the window
            double GetMxVal() const;

            /// deletes the newest element with values different from max
            /// from the structure if there is only one element, it is deleted
            void DelNewestNonMx();
            /// returns a "normal" exponential histogram representation of itself
            void ToExpHist(TExpHistogram&) const;

        protected:
            void OnIntervalRemoved(const TIntervalWithMax&);
            void OnAfterSwallow();

        private:
            void FindNewMxVal(const int& StartN=0) {
                MxVal = TFlt::NInf;
                for (int IntervalN = StartN; IntervalN < IntervalV.Len(); IntervalN++) {
                    if (IntervalV[IntervalN].GetMxVal() > MxVal) {
                        MxVal = IntervalV[IntervalN].GetMxVal();
                    }
                }
            }
            static bool IsMaxInWindow(const int64& ForgetTm, const TIntervalWithMax& Interval) {
                return ForgetTm < int64(Interval.GetStartTm() + Interval.GetEndTm()) / 2;
            }

            // MEMBERS
            using TIntervalV = TBase::TIntervalV;   // TODO when done check if this is needed

            TFlt MxVal; // the (approximate) maximum value on the interval
        };

        //////////////////////////////////////////////
        /// EH-like structure optimized for
        /// approximating the minimum value in a
        /// sliding window without counting the elements
        class TWindowMin {
        public:
            TWindowMin(const double& Eps);

            // SERIALIZATION

            TWindowMin(TSIn& SIn);
            void Save(TSOut& SOut) const;

            /* TWindowMin& operator =(TWindowMin&&) = default; */
            // TODO copy / move operations

            /// add a new value to the window
            void Add(const uint64& Tm, const double& Val);
            /// forgets everyting before and including the specified time
            void Forget(const int64&);
            /// returns the (approximate) minimum value in the structure
            double GetMnVal() const;

            // DEBUGGING

            uint GetSummarySize() const { return IntervalV.Len(); }
            bool Empty() const { return IntervalV.Empty(); }
            void PrintSummary() const;

        private:
            void Compress();

            void ReserveStructures(const uint& LogSize);
            TUInt& LogBlockSizeToBlockCount(const uint& LogBlockSize);
            TUInt& BlockSizeToBlockCount(const uint& BlockSize);
            uint GetMxBlocksSameSize() const;

            bool CheckInvariant1() const;
            bool CheckInvariant2() const;

            static bool IsMinInWindow(const int64& ForgetTm, const TIntervalWithMin& Interval);

            using TIntervalV = TVec<TIntervalWithMin>;

            TIntervalV IntervalV {};
            TUIntV LogSizeToBlockCountV {1};
            TFlt Eps;
            TFlt MnVal {TFlt::PInf};
            TInt64 ForgetTm {TInt64::Mn};
        };

        /// tuple used in the summary
        class TEhTuple {
            using TDelCallback = TExpHistBase<TIntervalWithMax>::TExpHistCallback;
        public:
            TEhTuple(const double& Eps, const uint64& ValTm, const double& Val,
                   TDelCallback& DelCallback);
            TEhTuple(const double& Eps, const uint64& ValTm, const double& Val,
                   TEhTuple& LeftTuple, TDelCallback& DelCallback);

            // SERIALIZATION

            TEhTuple(TSIn& SIn);
            void Save(TSOut& SOut) const;

            // TODO copy / move operations

            /// returns the (approximate) max value in the tuple
            /// without shifting the sliding window
            double GetVal() const;
            /// returns the (approximate) size of the tuple
            /// without shifting the sliding window
            uint GetTupleSize() const;
            /// returns the (approximate) correction of capacity of the tuple
            /// without shifting the sliding window
            uint GetMnMxRankDiff() const;
            /// indicates whether the tuple is empty
            bool Empty() const { return GetTupleSize() == 0; }

            /// Shifts the cutoff time. All the values at and after this time
            /// are forgotten
            void Forget(const int64& Tm);
            /* void Forget(const uint64& Tm); */

            void DelNewestNonMx();

            void Swallow(TEhTuple& Other, const bool& TakeMnMxRank);

            friend std::ostream& operator <<(std::ostream& os, const TEhTuple& Tup) {
                return os << "<"
                          << Tup.TupleSizeExpHist.GetMxVal() << ", "
                          << Tup.TupleSizeExpHist.GetCount() << ", "
                          << Tup.RightUncertExpHist.GetCount()
                          << ">";
            }

        private:
            TExpHistWithMax TupleSizeExpHist;   // G
            TExpHistogram RightUncertExpHist;   // D
        };

        ///////////////////////////////////////////
        /// A summary structure for the SwGk-based
        /// algorithms which uses a linked list structure
        class TSwGkLLSummary : public TExpHistBase<TUtils::TIntervalWithMax>::TExpHistCallback {
        public:
            TSwGkLLSummary(const double& EpsGk, const double& EpsEh);

            // SERIALIZATION
            TSwGkLLSummary(TSIn& SIn);
            void Save(TSOut& SOut) const;

            // API

            /// inserts a new value into the summary
            void Insert(const uint64& ValTm, const double& Val);
            /// returns the (approximate) quantile
            double Query(const double& PVal);
            /// returns a list of (approximate) quantiles, assumes values in PValV are sorted
            void Query(const TFltV& PValV, TFltV& QuantV);
            /// forgets values after the forget time
            void Forget(const uint64& ForgetTm);

            /// updates the time window, removes empty tuples and merges
            /// tuples which satisfy the merging criteria
            void Compress();

            // PARAMS
            const TFlt& GetEpsGk() const;
            const TFlt& GetEpsEh() const;

            /// returns the (approximate) count of items in the summary
            uint64 GetValCount();
            /// returns the (approximate) count of items in the summary by recounting the
            /// items in all the tuples
            uint64 GetValRecount();
            /// returns the number of tuples in the summary
            uint GetTupleCount() const;

            // TODO check if you can make this private or protected
            /// updates the total count of the items in the sliding window
            void OnItemsDeleted(const uint64& ItemCount);   // is called when items fall out of any of the tuples

            friend std::ostream& operator <<(std::ostream&, const TSwGkLLSummary&);

        private:
            /// for each tuple updates the time window and removes
            /// empty tuples
            void Refresh(const uint64& ForgetTm);

            using TTuple = TEhTuple;
            using TSummary = std::list<TTuple>;

            TSummary Summary {};
            TUInt64 ItemCount {};           // number of items in the sliding window, initializes to 0
            TInt64 ForgetTm {TInt64::Mn};   // the first timestamp forgotten by the summary

            TFlt EpsGk;                     // quantile estimation error
            TFlt EpsEh;                     // count estimation error
        };

        // PRINT OPERATORS

        std::ostream& operator <<(std::ostream& os, const TGkTuple& Tup);
        std::ostream& operator <<(std::ostream&, const TInterval&);
        std::ostream& operator <<(std::ostream&, const TIntervalWithMax&);
        std::ostream& operator <<(std::ostream&, const TIntervalWithMin&);
        std::ostream& operator <<(std::ostream&, const TUIntUInt64Pr&);
    }

    using TUtils::operator<<;

    ///////////////////////////////////////////////
    /// The Greenwald-Khanna (GK) online quantile estimation algorithm
    /// The error produced by the algorithm is eps
    ///
    /// http://infolab.stanford.edu/~datar/courses/cs361a/papers/quantiles.pdf
    class TGreenwaldKhanna {
    public:
        // a <v_i, g_i, d_i> tuple with elements:
        // v_i - the value of the observation
        // g_i = r_min(i) - r_min(i-1) the difference between the minimal rank of the i-th and (i-1)-the observations
        // d_i = r_max(i) - r_min(i) - the range of possible ranks for v_i
        using TTuple = TFltIntIntTr;
        // holds the tuples
        using TSummary = TVec<TTuple>;  // TODO use a linked list or some other structure

        enum class TCompressStrategy {
            csAuto,
            csManual
        };

        /// Default constructor. Initializes the algorithm with the allowed error, which
        /// is 2*eps.
        TGreenwaldKhanna(const double& Eps);
        TGreenwaldKhanna(const double& Eps, const TCompressStrategy&);

        // TODO copy / move

        // TODO save / load

        // TODO interpolate the result before returning it
        /// reutrns the (eps-approximate) value of the targeted quantile
        double Query(const double& Quantile) const;
        /// updates the summary with the new value
        void Insert(const double& Val);

        /// compresses the internal summary
        void Compress();
        /// reutrns the number of tuples stored in the summary
        int GetSummarySize() const;
        void PrintSummary() const;

    private:
        bool ShouldAutoCompress() const;
        int GetBand(const int& TupleN) const;

        // helper functions
        static TFlt& GetQuantile(TTuple&);
        static const TFlt& GetQuantile(const TTuple&);
        static TInt& GetPrevMnRankDiff(TTuple&);
        static const TInt& GetPrevMnRankDiff(const TTuple&);
        static TInt& GetInsRankRange(TTuple&);
        static const TInt& GetInsRankRange(const TTuple&);

        TSummary Summary {};
        const TFlt Eps;
        TUInt64 SampleN {uint64(0)};
        TBool UseBands {true};
        TCompressStrategy CompressStrategy {TCompressStrategy::csAuto};
    };

    using TGk = TGreenwaldKhanna;

    //////////////////////////////////////////
    /// The CKMS (generalization of GK) algorithm for online
    /// biased quantile estimation.
    ///
    /// Described in: "Effective Computation of Biased Quantiles over Data Streams"
    /// https://www.cs.rutgers.edu/~muthu/bquant.pdf
    ///
    /// Only the biased version is implemented. The targeted version is flawed.
    class TBiasedGk {
    public:
        /// constructor which sets monitored quantiles q0 and (1 - q0)
        /// the accuracy at these target quantiles is eps0, while
        /// the accuracy at other quantiles q is max(eps0, eps0 * (q / q0))
        /* TBiasedGk(const double& Quant0, const double& Eps0, const bool& UseBands=true); */

        /// the returned rank will be between (1-eps)r <= ri <= (1+eps)r up to
        /// quant0, the accuracy then stops increasing
        ///
        /// Quant0: the quantile where the accuracy stops increasing
        /// Eps: the (multiplicative) allowed error
        /// UseBands: when bands are used, the algorithm prefers to keep tuples
        ///           which were inserted early on, which can save space in the long run
        TBiasedGk(const double& Quant0, const double& Eps, const bool& UseBands=true);
        // TODO need 2 constructors:
        // 1) specify the accuracy and the method takes as much space as needed
        // 2) specify the space and the accuracy can suffer

        // TODO copy / move
        // TODO save / load

        // TODO interpolate the result before returning it
        /// returns the eps0 * (q / q0) approximate quantile
        double Query(const double& Quantile) const;
        /// inserts a new element into the summary
        void Insert(const double& Val);

        /// compresses the internal summary
        void Compress();
        /// returns the number of tuples stored in the summary
        int GetSummarySize() const;
        void PrintSummary() const;

    private:
        // helper functions which define the size and error for a tuple
        double GetEps(const double& Quantile) const;
        double GetMxTupleSize(const double& Rank) const;

        bool ShouldCompress() const;
        int GetBand(const TUtils::TGkTuple&, const uint64& MnRank) const;

        using TTuple = TUtils::TGkTuple;
        using TSummary = TVec<TTuple>;  // TODO use a linked list or some other structure

        TSummary Summary {};
        TUInt64 SampleN {};     // number of samples seen so far, initialized to 0
        TFlt Quant0;
        TFlt Eps;
        TInt Dir {1};
        TBool UseBands;
    };

    using TCkms = TBiasedGk;


    ////////////////////////////////////////////
    /// Greenwald-Khanna algorithm on a sliding
    /// window.
    ///
    /// Described in
    /// "Online Algorithm for Approximate Quantile Queries on Sliding Windows"
    /// http://dl.acm.org/citation.cfm?id=2954329
    class TSwGk {
    public:
        /// Default constructor, sets the error bound for both types of error incurred
        /// by the algorithm.
        ///
        /// EpsGk: The maximum error bound for the quantile estimation algorithm. If the
        ///        number of items in the sliding window is N, then assuming the counts
        ///        are accurate, the error in rank produced by the algorithm is EpsGk*N.
        ///        Lowering this bound will increase the number of tuples in the summary.
        ///        On average, the number of tuples will be O(1 / EpsGk)
        ///
        /// EpsEh: The error bound for the approximate counting structures (exponential histograms).
        ///        Each tuple maintains two exponential histograms. If the true count of
        ///        items in the tuple is C, then the error produced by the structure is bounded
        ///        by EpsEh*C.
        ///
        /// Overall, the worst-case error produced by the algorithm when there are N items
        /// in the sliding window is bounded by N*(EpsGk + 2*EpsEh + O(EpsEh^2)). However
        /// in practice, the error should be less.
        TSwGk(const double& EpsGk, const double& EpsEh);

        // SERIALIZATION
        /// loads the model from the input stream
        TSwGk(TSIn& SIn);
        /// saves the model to the output stream
        void Save(TSOut& SOut) const;

        // ALGORITHM API
        /// returns the (approximate) quantile
        double Query(const double& PVal);
        /// returns multiple (approximate) quantiles, the method assumes PValV is sorted!
        void Query(const TFltV& PValV, TFltV& QuantV);
        /// inserts a new value with the specified time
        void Insert(const uint64& ValTm, const double& Val);
        /// forgets all values before and including the specified time
        void Forget(const int64&);
        /// compresses the summary
        void Compress();

        /// resets the object to its original state
        void Reset();

        // PARAMS
        const TFlt& GetEpsGk() const;
        const TFlt& GetEpsEh() const;

        // DEBUGGING
        /// returns the number of tuples in the summary
        int GetSummarySize() const;
        /// returns the total count of items in the sliding window
        uint64 GetValCount();
        /// returns the total count of the items by recounting
        /// then from the summary
        uint64 GetValRecount();
        /// prints the summary at the specified time
        void PrintSummary() const;
        /// prints the summary of the structure which tracks the minimum
        void PrintMinWinSummary() const;
        /// returns the number of samples seen by the algorithm
        const TUInt64& GetSampleN() const { return SampleN; }

    private:
        // HELPER FUNCTIONS

        /// specifies whether the compress condition holds
        bool ShouldCompress() const;

        // MEMBERS

        using TSummary = TUtils::TSwGkLLSummary;
        using TWindowMin = TUtils::TWindowMin;

        TSummary Summary;               // a summary which keeps track of the quantiles
        TWindowMin WinMin;              // a structure which keeps track of the minimum value in the sliding window
        TFlt EpsGk;                     // quantile estimation error

        TInt64 ForgetTm {TInt64::Mn};   // time of the last record which is not in the sliding window
        TUInt64 SampleN {};             // initializes to 0, total number of samples seen by the algorithm
    };

    ////////////////////////////////////////////
    /// GK algorithm which works on a fixed size window.
    class TCountWindowGk : protected TSwGk {
    public:
        /// Default constructor, sets the error bound for both types of error incurred
        /// by the algorithm.
        ///
        /// WindowSize: The number of items to store in the sliding window. When there are
        ///        more items, the oldest one is forgotten.
        ///
        /// EpsGk: The maximum error bound for the quantile estimation algorithm. If the
        ///        number of items in the sliding window is N, then assuming the counts
        ///        are accurate, the error in rank produced by the algorithm is EpsGk*N.
        ///        Lowering this bound will increase the number of tuples in the summary.
        ///        On average, the number of tuples will be O(1 / EpsGk)
        ///
        /// EpsEh: The error bound for the approximate counting structures (exponential histograms).
        ///        Each tuple maintains two exponential histograms. If the true count of
        ///        items in the tuple is C, then the error produced by the structure is bounded
        ///        by EpsEh*C.
        ///
        /// Overall, the worst-case error produced by the algorithm when there are N items
        /// in the sliding window is bounded by N*(EpsGk + 2*EpsEh + O(EpsEh^2)). However
        /// in practice, the error should be less.
        TCountWindowGk(const uint64& WindowSize, const double& EpsGk, const double& EpsEh);

        // SERIALIZATION
        TCountWindowGk(TSIn&);
        void Save(TSOut&) const;

        // TODO copy / move operations

        // API
        /// Inserts a new value into the time window. If there are more
        /// items in the timw window than the maximum amount, the older
        /// ones are forgotten
        void Insert(const double& Val);
        using TSwGk::Query;
        using TSwGk::Compress;

        // PARAMS
        const TUInt64& GetWindowSize() const;
        using TSwGk::GetEpsGk;
        using TSwGk::GetEpsEh;

        // debugging stuff
        using TSwGk::PrintSummary;
        using TSwGk::GetSummarySize;
        using TSwGk::GetValCount;
        using TSwGk::GetValRecount;

    private:
        TUInt64 WindowSize;
    };

    ////////////////////////////////////////////
    /// GK algorithm which works on a fixed time window.
    class TTimeWindowGk : protected TSwGk {
    public:
        /// Default constructor, sets the error bound for both types of error incurred
        /// by the algorithm.
        ///
        /// WindowMSec: The duration of the sliding window in milliseconds. Items older
        ///         than the end of the window are forgotten.
        ///
        /// EpsGk: The maximum error bound for the quantile estimation algorithm. If the
        ///        number of items in the sliding window is N, then assuming the counts
        ///        are accurate, the error in rank produced by the algorithm is EpsGk*N.
        ///        Lowering this bound will increase the number of tuples in the summary.
        ///        On average, the number of tuples will be O(1 / EpsGk)
        ///
        /// EpsEh: The error bound for the approximate counting structures (exponential histograms).
        ///        Each tuple maintains two exponential histograms. If the true count of
        ///        items in the tuple is C, then the error produced by the structure is bounded
        ///        by EpsEh*C.
        ///
        /// Overall, the worst-case error produced by the algorithm when there are N items
        /// in the sliding window is bounded by N*(EpsGk + 2*EpsEh + O(EpsEh^2)). However
        /// in practice, the error should be less.
        TTimeWindowGk(const uint64& WindowMSec, const double& EpsGk, const double& EpsEh);

        // SERIALIZATION
        /// constructor which loads the model from an input stream
        TTimeWindowGk(TSIn& SIn);
        /// saves the model to the output stream
        void Save(TSOut& SOut) const;

        // TODO copy / move operations

        void Insert(const uint64& ValTm, const double& Val);
        /// moves the sliding window forward to the specified time
        void UpdateTime(const uint64& ValTm);
        using TSwGk::Query;
        using TSwGk::Compress;

        // PARAMS
        const TUInt64& GetWindowMSec() const;
        using TSwGk::GetEpsGk;
        using TSwGk::GetEpsEh;

        // debugging stuff
        using TSwGk::PrintSummary;
        using TSwGk::GetSummarySize;
        using TSwGk::GetValCount;
        using TSwGk::GetValRecount;

    private:
        TUInt64 WindowMSec;
    };

    std::ostream& operator <<(std::ostream& os, const TUInt& Val);
}

#include "quantiles.hpp"

#endif

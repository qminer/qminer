#ifndef _STREAM_QUANTILES_H
#define _STREAM_QUANTILES_H

#include <list>
#include <iostream>
#include <functional>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>

namespace TQuant {

    namespace TUtils {

        template <typename T1, typename T2, typename T3>
        std::ostream& operator <<(std::ostream& os, const TTriple<T1, T2, T3>& Triple);
        template <typename T>
        std::ostream& operator <<(std::ostream& os, const TVec<T>& Vec);

        //////////////////////////////////////
        /// GK tuple - value comparators determine
        /// whether a tuple should be left of the
        /// value in the summary or not
        ///
        /// puts equal values to the left of the tuple
        class TEqLeftCmp {
        public:
            template <typename TTuple>
            static bool IsRightOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsLeftOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsRightOfNegDir(const TTuple& Tuple, const double& Val, TRnd&);
        };

        //////////////////////////////////////
        /// GK tuple - value comparators determine
        /// whether a tuple should be left of the
        /// value in the summary or not
        ///
        /// puts equal values to the right of the tuple
        class TEqRightCmp {
        public:
            template <typename TTuple>
            static bool IsRightOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsLeftOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsRightOfNegDir(const TTuple& Tuple, const double& Val, TRnd&);
        };

        //////////////////////////////////////
        /// GK tuple - value comparators determine
        /// whether a tuple should be left of the
        /// value in the summary or not
        ///
        /// puts equal values randomly
        class TEqRndCmp {
        public:
            template <typename TTuple>
            static bool IsRightOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsLeftOf(const TTuple& Tuple, const double& Val, TRnd&);
            template <typename TTuple>
            static bool IsRightOfNegDir(const TTuple& Tuple, const double& Val, TRnd&);
        };


        ////////////////////////////////////
        /// A tuple class used by the GK-based algorithms
        template <typename TValCmp>
        class TGkTuple {
        public:
            TGkTuple();
            TGkTuple(const double&);
            TGkTuple(const double& MxVal, const uint& UncertRight, const TGkTuple&);

            // SERIALIZATION
            TGkTuple(TSIn&);
            void Save(TSOut&) const;

            void Swallow(const TGkTuple&);
            void SwallowOne();

            const TFlt& GetVal() const { return MxVal; }
            const TUInt& GetTupleSize() const { return TupleSize; }
            const TUInt& GetUncert() const { return UncertRight; }

            uint GetTotalUncert() const { return GetTupleSize() + GetUncert(); }

            bool IsRightOf(const double& Val, TRnd& Rnd) const;
            bool IsLeftOf(const double& Val, TRnd& Rnd) const;
            bool IsRightOfNegDir(const double& Val, TRnd& Rnd) const;

            uint64 GetMemUsed() const;

            friend std::ostream& operator <<(std::ostream& os, const TGkTuple& Tuple) {
                return os << "<"
                          << Tuple.GetVal() << ", "
                          << Tuple.GetTupleSize() << ","
                          << Tuple.GetUncert()
                          << ">";
            }

        private:
            TFlt MxVal;
            TUInt TupleSize {uint(1)};
            TUInt UncertRight {uint(0)};
        };

        /////////////////////////////////
        /// A tuple used by the GK-based algorithms which
        /// minimizes the unceirtainty of its values rank
        ///
        /// Values equal to the value of the tuple are
        /// regarded to be left of it in the summary
        template <typename TValCmp>
        class TGkMnUncertTuple {
            using TTuple = TGkMnUncertTuple<TValCmp>;

        public:
            /// creates an empty tuple
            TGkMnUncertTuple(): MxVal() {}
            /// creates a new tuple with a single element with given value
            TGkMnUncertTuple(const double&);
            /// creates a new tuple with the given right neighbour
            /// the new tuple's uncertainty is defined as g_{i+1} + delta_{i+1} - 1
            TGkMnUncertTuple(const double& Val, const uint&, const TGkMnUncertTuple& RightTuple);

            // SERIALIZATION
            TGkMnUncertTuple(TSIn&);
            void Save(TSOut&) const;

            /// adds all the items the argument summarizes to its own summary
            void Swallow(const TGkMnUncertTuple&);
            /// adds one item to its own summary
            void SwallowOne();

            /// returns the maximal value in the summary
            const TFlt& GetVal() const { return MxVal; }
            /// returns the size of the tuples' summary
            const TUInt& GetTupleSize() const { return TupleSize; }
            /// returns the uncertainty of the max values rank due to the merges
            /// happening on the right of this tuple
            const TUInt& GetUncert() const { return UncertRight; }
            /// returns the total uncertainty of the values' rank
            uint GetTotalUncert() const { return GetTupleSize() + GetUncert(); }

            bool IsRightOf(const double& Val, TRnd& Rnd) const;
            bool IsLeftOf(const double& Val, TRnd& Rnd) const;
            bool IsRightOfNegDir(const double& Val, TRnd& Rnd) const;

            // DEBUGGING
            uint64 GetMemUsed() const;

            friend std::ostream& operator <<(std::ostream& os, const TGkMnUncertTuple& Tuple) {
                return os << "<"
                          << Tuple.GetVal() << ", "
                          << Tuple.GetTupleSize() << ","
                          << Tuple.GetUncert()
                          << ">";
            }

        private:
            TFlt MxVal;             // the max value in this tuple
            TUInt TupleSize {1u};   // how many tuples has this tuple swallowed
            TUInt UncertRight {};   // how many items on the right could be smaller than this tuple
        };

        using TGkMnUncertEqLeftTuple = TGkMnUncertTuple<TEqLeftCmp>;
        using TGkMnUncertEqRightTuple = TGkMnUncertTuple<TEqRightCmp>;
        using TGkMnUncertEqRndTuple = TGkMnUncertTuple<TEqRndCmp>;

        /////////////////////////////////
        /// A tuple used by the GK-based algorithms which
        /// minimizes the unceirtainty of its values rank
        ///
        /// Values equal to the value of the tuple are
        /// regarded to be right of it in the summary
        /* class TGkMnUncertEqRightTuple { */
        /* public: */
        /*     /// creates an empty tuple */
        /*     TGkMnUncertEqRightTuple(): MxVal() {} */
        /*     /// creates a new tuple with a single element with given value */
        /*     TGkMnUncertEqRightTuple(const double&); */
        /*     /// creates a new tuple with the given right neighbour */
        /*     /// the new tuple's uncertainty is defined as g_{i+1} + delta_{i+1} - 1 */
        /*     TGkMnUncertEqRightTuple(const double& Val, const uint&, const TGkMnUncertEqRightTuple& RightTuple); */

        /*     // SERIALIZATION */
        /*     TGkMnUncertEqRightTuple(TSIn&); */
        /*     void Save(TSOut&) const; */

        /*     /// adds all the items the argument summarizes to its own summary */
        /*     void Swallow(const TGkMnUncertEqRightTuple&); */
        /*     /// adds one item to its own summary */
        /*     void SwallowOne(); */

        /*     /// returns the maximal value in the summary */
        /*     const TFlt& GetVal() const { return MxVal; } */
        /*     /// returns the size of the tuples' summary */
        /*     const TUInt& GetTupleSize() const { return TupleSize; } */
        /*     /// returns the uncertainty of the max values rank due to the merges */
        /*     /// happening on the right of this tuple */
        /*     const TUInt& GetUncert() const { return UncertRight; } */
        /*     /// returns the total uncertainty of the values' rank */
        /*     uint GetTotalUncert() const { return GetTupleSize() + GetUncert(); } */

        /*     bool IsRightOf(const double& Val) const { */
        /*         return Val < MxVal; */
        /*     } */

        /*     bool IsRightOfNegDir(const double& Val) const { */
        /*         return Val > MxVal; */
        /*     } */


        /*     // DEBUGGING */
        /*     uint64 GetMemUsed() const; */

        /*     friend std::ostream& operator <<(std::ostream& os, const TGkMnUncertEqRightTuple& Tuple) { */
        /*         return os << "<" */
        /*                   << Tuple.GetVal() << ", " */
        /*                   << Tuple.GetTupleSize() << "," */
        /*                   << Tuple.GetUncert() */
        /*                   << ">"; */
        /*     } */

        /* private: */
        /*     TFlt MxVal;             // the max value in this tuple */
        /*     TUInt TupleSize {1u};   // how many tuples has this tuple swallowed */
        /*     TUInt UncertRight {};   // how many items on the right could be smaller than this tuple */
        /* }; */

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
            static TFlt ExtractOtherCarryInfo(const TInterval& Interval) { return Interval.GetVal(); }
            static void MergeOtherCarryInfo(const TFlt& MxVal, TFlt& CurrMxVal) {
                if (MxVal > CurrMxVal) {
                    CurrMxVal = MxVal;
                }
            }
            static void UndefineOtherInfo(TFlt& MxVal) {
                MxVal = TFlt::NInf;
            }
        };

        class TIntervalWithMax;

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

            TInterval& operator =(const TIntervalWithMax&);

            // SERIALIZATION
            TInterval(TSIn&);
            void Save(TSOut& SOut) const;

            const TUInt& GetCount() const { return ElCount; }
            const TUInt64& GetStartTm() const { return StartTm; }
            const TUInt64& GetDurMSec() const { return DurMSec; }

            uint64 GetEndTm() const;

            void Swallow(const TInterval&);
            void Split(TInterval& StartInterval, TInterval& EndInterval) const;

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;
        };

        ///////////////////////////////////////////
        /// Interval which holds the start time,
        /// duration, item count and maximum value
        class TIntervalWithMax : protected TInterval {
            using TBase = TInterval;
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

            using TInterval::GetCount;
            using TInterval::GetStartTm;
            using TInterval::GetEndTm;
            using TInterval::GetDurMSec;
            const TFlt& GetVal() const { return MxVal; }

            void Swallow(const TIntervalWithMax&);
            void Split(TIntervalWithMax& StartInterval, TIntervalWithMax& EndInterval) const {
                TInterval::Split(StartInterval, EndInterval);
                StartInterval.MxVal = MxVal;
                EndInterval.MxVal = MxVal;
            }

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;
        };

        ///////////////////////////////////////////
        /// Interval which holds the start time,
        /// duration, item count and minimum value
        class TIntervalWithMin : protected TInterval {
            using TBase = TInterval;
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

            using TInterval::GetCount;
            using TInterval::GetStartTm;
            using TInterval::GetEndTm;
            using TInterval::GetDurMSec;
            const TFlt& GetMnVal() const { return MnVal; }

            void Swallow(const TIntervalWithMin&);

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;
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
            /// returns the objects memory footprint
            virtual uint64 GetMemUsed() const;
            /// prints the intervals to stadard output
            void PrintSummary() const { std::cout << IntervalV << "\n"; }

            friend std::ostream& operator <<(std::ostream& os, const TExpHistBase<TInterval>& Hist) {
                return os << Hist.IntervalV;
            }

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

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;

            /// returns a "normal" exponential histogram representation of itself
            void ToExpHist(TExpHistogram&) const;

            friend std::ostream& operator <<(std::ostream& os, const TExpHistWithMax& Hist) {
                return os << Hist.IntervalV;
            }

        protected:
            void OnIntervalRemoved(const TIntervalWithMax&);
            void OnAfterSwallow();

        private:
            void FindNewMxVal(const int& StartN=0);
            static bool IsMaxInWindow(const int64& ForgetTm, const TIntervalWithMax& Interval);

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
            uint64 GetMemUsed() const;
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

            // COPY/MOVE
            // copy
            TEhTuple(const TEhTuple&);
            TEhTuple& operator =(const TEhTuple&);
            // move
            TEhTuple(TEhTuple&&);
            TEhTuple& operator =(TEhTuple&&);

            /// returns the (approximate) max value in the tuple
            /// without shifting the sliding window
            double GetVal() const;
            /// returns the (approximate) size of the tuple
            /// without shifting the sliding window
            uint GetTupleSize() const;
            /// returns the (approximate) correction of capacity of the tuple
            /// without shifting the sliding window
            uint GetUncert() const;
            /// returns the range of the tuples' rank
            uint GetTotalUncert() const { return GetTupleSize() + GetUncert(); }
            /// indicates whether the tuple is empty
            bool Empty() const { return GetTupleSize() == 0; }

            /// Shifts the cutoff time. All the values at and after this time
            /// are forgotten
            void Forget(const int64& Tm);

            void DelNewestNonMx();

            /// merges itself with the other tuple
            void Swallow(TEhTuple& Other, const bool& TakeMnMxRank);
            /// adds a single element to itself
            void SwallowOne(const uint64& ValTm, const double& Val);

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;

            void PrintLong() const {
                std::cout << "<\n"
                    << "\tsize hist: " << TupleSizeExpHist << "\n"
                    << "\trigh hist: " << RightUncertExpHist << "\n>";
            }

            TStr GetStr() const {
                return "<" +
                       TFlt::GetStr(TupleSizeExpHist.GetMxVal()) + ", " +
                       TUInt::GetStr(TupleSizeExpHist.GetCount()) + ", " +
                       TUInt::GetStr(RightUncertExpHist.GetCount()) +
                       ">";
            }

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

        class TCondPrinter {
        public:
            TCondPrinter();
            TCondPrinter(const std::function<bool(void)>&);

            bool WillPrint() const { return ShouldPrint; }
            void StartIter();
            void Print(const TStr&) const;

            std::function<bool(void)> CondFun;
            TBool ShouldPrint {false};
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

            /// returns the objects memory footprint
            uint64 GetMemUsed() const;

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

            // TODO here for debugging
            TCondPrinter Log;
        };

        // PRINT OPERATORS

        std::ostream& operator <<(std::ostream&, const TInterval&);
        std::ostream& operator <<(std::ostream&, const TIntervalWithMax&);
        std::ostream& operator <<(std::ostream&, const TIntervalWithMin&);
        std::ostream& operator <<(std::ostream&, const TUIntUInt64Pr&);
    }

    using TUtils::operator<<;

    namespace TUtils {

        namespace TTDigestUtils {

            template <typename TWgt>
            class TCentroid {
            public:
                TCentroid();
                TCentroid(const double& Val, const TWgt& Wgt);

                // SERIALIZATION
                TCentroid(TSIn&);
                void Save(TSOut&) const;

                void Swallow(const double& Val, const TWgt& ValWgt);

                double GetDist(const double& Val) const;

                const TFlt& GetMean() const { return Mean; }
                const TWgt& GetWgt() const { return WgtSum; }

                void SubtractWgt(const TWgt& Wgt) { Assert(0 <= Wgt && Wgt < WgtSum); WgtSum -= Wgt; }

                /// adds the other centroids weight and value to its own
                /// summary
                TCentroid& operator +=(const TCentroid&);
                /// returns true if this centroids mean is smaller than the others
                bool operator <(const TCentroid&) const;

                // DEBUGGING
                uint64 GetMemUsed() const;
                TStr GetStr() const;

            private:
                TFlt Mean;
                TWgt WgtSum;
            };

            template <typename TWgt>
            inline std::ostream& operator <<(std::ostream& os, const TCentroid<TWgt>& Cent) {
                return os << "<" << Cent.GetMean().Val << "," << Cent.GetWgt().Val << ">";
            }

            template <typename TWgt>
            inline std::ostream& operator <<(std::ostream& os, const TVec<TCentroid<TWgt>>& CentroidV) {
                os << "[";
                for (int CentroidN = 0; CentroidN < CentroidV.Len(); ++CentroidN) {
                    os << CentroidV[CentroidN];
                    if (CentroidN < CentroidV.Len()-1) {
                        os << ", ";
                    }
                }
                return os << "]";
            }

            template <typename TWgt>
            class TTDigestBase {
            public:
                TTDigestBase() {}

                // SERIALIZATION
                TTDigestBase(TSIn&);
                void Save(TSOut&) const;

                /// returns the (approximate) quantile for the given
                /// cumulative probability
                double Query(const double& PVal) const;
                /// calcuated an array of (approximate) quantiles for the
                /// given cumulative probabilities
                void Query(const TFltV& PValV, TFltV& QuantV) const;

                // DEBUGGING
                /// returns the objects memory footprint
                uint64 GetMemUsed() const;
                void PrintSummary() const;

            protected:
                using TCentroidType = TCentroid<TWgt>;
                using TCentroidV = TVec<TCentroidType>;

                TCentroidV CentroidV {};
                TUInt64 SampleN;
            };
        }
    }

    ///////////////////////////////////////////
    /// Exact quantile estimator
    class TExact {
        using TTree = __gnu_pbds::tree<
            double,         /* key                */
            __gnu_pbds::null_type,         /* mapped             */
            std::less<double>,   /* compare function   */
            __gnu_pbds::rb_tree_tag, /* red-black tree tag */
            __gnu_pbds::tree_order_statistics_node_update>;
    public:
        TExact() {}

        double Query(const double& PVal) const;
        void Query(const TFltV& PValV, TFltV& QuantV) const;
        void Insert(const double& Val);

        uint64 GetSampleN() const;
        uint64 GetSummarySize() const { return GetSampleN(); }
        uint64 GetMemUsed() const {
            return sizeof(TExact) +
                sizeof(TTree) +
                GetSampleN()*(sizeof(double) + sizeof(void*));
        }

    private:
        TTree OrderTree;
    };

    ///////////////////////////////////////////////
    /// The Greenwald-Khanna (GK) online quantile estimation algorithm
    /// The error produced by the algorithm is eps
    ///
    /// http://infolab.stanford.edu/~datar/courses/cs361a/papers/quantiles.pdf
    class TGreenwaldKhanna {
        using TTuple = TUtils::TGkMnUncertEqLeftTuple;
        using TSummary = TVec<TTuple>;

    public:

        enum class TCompressStrategy : char {
            csAuto = 0,
            csManual = 1
        };

        /// Default constructor. Initializes the algorithm with the allowed error, which
        /// is 2*eps.
        TGreenwaldKhanna(const double& Eps);
        TGreenwaldKhanna(const double& Eps, const TCompressStrategy&, const bool& UseBands=true);
        TGreenwaldKhanna(const double& Eps, const TRnd& Rnd, const bool& UseBands=true);

        // SERIALIZATION
        TGreenwaldKhanna(TSIn&);
        void Save(TSOut&) const;

        // TODO interpolate the result before returning it
        /// reutrns the (eps-approximate) value of the targeted quantile
        double GetQuantile(const double& PVal) const;
        /// returns an array of quantiles corresponding to the given p-values
        void GetQuantileV(const TFltV& PValV, TFltV& QuantV) const;
        /// returns the (approxmate) value of the cumulative distribution function
        /// for the given value `Val`
        /// the CDF is defined as CDF(x) = P(X <= x)
        double GetCdf(const double& Val) const;
        void GetCdfV(const TFltV& ValV, TFltV& CdfValV) const;
        /// returns the maximum (absolute) difference of the CDFs of the two distributions
        double GetMxCdfDiff(const TGreenwaldKhanna& Other) const;
        /// updates the summary with the new value
        void Insert(const double& Val); // TODO
        /// compresses the internal summary
        void Compress();

        // PARAMS
        const TFlt& GetEps() const { return Eps; }
        const TCompressStrategy& GetCompressStrategy() const { return CompressStrategy; }

        // DEBUGGING
        /// reutrns the number of tuples stored in the summary
        const TUInt64& GetSampleN() const { return SampleN; }
        const TBool& GetUseBandsP() const { return UseBandsP; }
        int GetSummarySize() const;
        uint64 GetMemUsed() const;
        void PrintSummary() const;

    private:
        uint GetMxUncert() const;
        int GetBand(const TTuple& Tuple) const;
        uint32 GetCompressInterval() const;
        bool ShouldAutoCompress() const;

        void GetSummaryValV(TFltV& ValV) const;

        TSummary Summary {};
        TRnd Rnd {0};
        TUInt64 SampleN {uint64(0)};
        TFlt Eps;
        TCompressStrategy CompressStrategy {TCompressStrategy::csAuto};
        TBool UseBandsP {true};
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
        using TTuple = TUtils::TGkMnUncertEqRndTuple;
        using TSummary = TVec<TTuple>;

    public:
        enum class TCompressStrategy : char {
            csManual = 0,
            csAggressive = 1,
            csPeriodic = 2
        };

        /// the returned rank will be between (1-eps)r <= ri <= (1+eps)r up to
        /// quant0, the accuracy then stops increasing
        ///
        /// PVal0: the quantile where the accuracy stops increasing
        /// Eps: the (multiplicative) allowed error
        /// Type: indicates whether to track the minimum or the maximum
        /// CompressStrategy: determiner when the algorithm compresses its summary
        /// UseBands: when bands are used, the algorithm prefers to keep tuples
        ///           which were inserted early on, which can save space in the long run
        TBiasedGk(const double& PVal0, const double& Eps, const TCompressStrategy& Cs=TCompressStrategy::csPeriodic,
                const bool& UseBands=true);
        TBiasedGk(const double& PVal0, const double& Eps, const TRnd& Rnd);

        // SERIALIZATION
        TBiasedGk(TSIn&);
        void Save(TSOut&) const;

        // TODO interpolate the result before returning it
        /// returns the eps0 * (q / q0) approximate quantile
        double GetQuantile(const double& CdfVal) const;
        /// returns an array of quantiles corresponding to the given p-values
        void GetQuantileV(const TFltV& CdfValV, TFltV& QuantV) const;
        /// returns the (approxmate) value of the cumulative distribution function
        /// for the given value `Val`
        /// the CDF is defined as CDF(x) = P(X <= x)
        double GetCdf(const double& Val) const;
        /// inserts a new element into the summary
        void Insert(const double& Val);
        /// compresses the internal summary
        void Compress();

        // PARAMS

        /// returns the accuracy
        const TFlt& GetEps() const;
        /// returns the most accurate p-value tracked
        double GetPVal0() const;
        /// returns the compression strategy used by the algorithm
        const TCompressStrategy& GetCompressStrategy() const;
        /// returns true if the algorithm uses the band sub-procedure
        const TBool& GetUseBands() const;

        /// returns the total number of items seen by the algorithm
        const TUInt64& GetSampleN() const { return SampleN; }
        /// returns the number of tuples stored in the summary
        int GetSummarySize() const;
        /// returns the total memory currently used by the model
        uint64 GetMemUsed() const;
        /// prints the summary to stdout
        void PrintSummary() const;

        /// returns the maximum number of items a tuple with the given rank
        /// can summarize
        double GetMxTupleUncert(const double& Rank) const;
        const TInt& GetDir() const { return Dir; }

    private:
        bool IsPositiveDir() const { return Dir > 0; }
        bool ShouldCompress() const;
        int GetBand(const TTuple&, const uint64& MnRank) const;

        TSummary Summary {};
        TUInt64 SampleN {uint64(0)};     // number of samples seen so far, initialized to 0
        TUInt64 CompressSampleN;
        TFlt PVal0;
        TFlt Eps;
        TRnd Rnd {0};
        TInt Dir;
        TCompressStrategy CompressStrategy {TCompressStrategy::csPeriodic};
        TBool UseBands {true};
    };

    using TCkms = TBiasedGk;

    ////////////////////////////////////
    /// tDigest - clustering implementation
    class TTDigest : private TUtils::TTDigestUtils::TTDigestBase<TUInt> {
        using TBase = TUtils::TTDigestUtils::TTDigestBase<TUInt>;
        using TCentroid = TBase::TCentroidType;
        using TCentroidV = TVec<TCentroid>;

    public:
        enum class TCompressStrategy : uchar {
            csNever,
            csPeriodic
        };

        TTDigest(const int& MnCentroids, const TRnd& Rnd=TRnd());
        TTDigest(const int& MnCentroids, const TCompressStrategy& Cs=TCompressStrategy::csNever,
                const TRnd& Rnd=TRnd());
        TTDigest(const int& MnCentroids, const double& MnEps,
                const TCompressStrategy& Cs=TCompressStrategy::csNever, const TRnd& Rnd=TRnd());

        // SERIALIZATION
        TTDigest(TSIn&);
        void Save(TSOut&) const;

        using TBase::Query;
        /// inserts a new value with the given weight
        void Insert(const double& Val, const uint& ValWgt=1);

        // PARAMETERS
        const TRnd& GetRnd() const { return Rnd; }
        const TInt& GetMnCentroids() const { return MnCentroids; }
        const TFlt& GetMnEps() const { return MnEps; }
        const TCompressStrategy& GetCompressStrategy() const { return CompressStrategy; }

        // DEBUGGING
        /// returns the current number of centroids in the summary
        int GetSummarySize() const;
        const TUInt64& GetSampleN() const;
        using TTDigestBase::PrintSummary;
        /// returns the instances memory footprint size
        uint64 GetMemUsed() const;

    private:
        // insert helper methods
        void Insert(const double& Val, const uint& ValWgt, const bool& UpdateSampleN);
        void Recluster();

        /// returns the maximum number of clusters the algorithm is allowed to produce
        int GetMxCentroids() const;
        /// returns the maximum number of items a cluster with the given p-value can
        /// summarize
        int GetMxCentroidSize(const double& PVal) const;
        /// returns the relative error for the given p-value
        double GetEps(const double& PVal) const;

        static const uint64 COMPRESS_INTERVAL_FACTOR = 20;

        TRnd Rnd {0};
        TFlt MnEps {1e-4};
        TInt MnCentroids;
        TFlt MxCentroidsFactor {20.0};
        /* TFlt MxCentroidsFactor {10.0}; */
        TUInt64 ReclustSampleN {TUInt64::Mx};
        TCompressStrategy CompressStrategy {TCompressStrategy::csNever};
    };

    ////////////////////////////////////////////
    /// tDigest - buffer implementaion
    class TMergingTDigest : private TUtils::TTDigestUtils::TTDigestBase<TFlt> {
        using TBase = TUtils::TTDigestUtils::TTDigestBase<TFlt>;
        using TCentroid = TBase::TCentroidType;
        using TCentroidV = TBase::TCentroidV;
    public:
        TMergingTDigest(const double& Delta, const TRnd& Rnd);
        TMergingTDigest(const double& Delta, const int& MxBuffLen,
                const TRnd& Rnd=TRnd(0));

        // SERIALIZATION
        TMergingTDigest(TSIn&);
        void Save(TSOut&) const;

        using TBase::Query;
        /// inserts a new data point into the buffer, if the buffer is full it is
        /// automatically merged
        void Insert(const double& Val);
        /// flushes the buffer
        void Flush();

        const TFlt& GetDelta() const { return Delta; }
        const TInt& GetMxBuffLen() const { return MxBuffLen; }

        /// returns the current number of centroids in the summary
        int GetSummarySize() const;
        using TTDigestBase::PrintSummary;
        /// returns the number of samples seen so far by the model
        const TUInt64& GetSampleN() const;
        /// returns this objects memory footprint
        uint64 GetMemUsed() const;

    private:
        /* double GetCentroidMxCumProb(const double& PrevCentCumProb) const; */
        /// indicates whether the buffer should be flushed
        bool ShouldFlush() const;

        double ToScaleK(const double& ValSpaceP);
        double ToScaleP(const double& ValSpaceK);

        TCentroidV BuffV;
        TFlt Delta;
        TRnd Rnd {0};
        TInt MxBuffLen;
        TInt MxCentroids;
    };


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
        /// returns the objects memory footprint
        uint64 GetMemUsed() const;
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
        /// returns the objects memory footprint
        uint64 GetMemUsed() const;
        using TSwGk::GetSummarySize;
        using TSwGk::GetValCount;
        using TSwGk::GetValRecount;
        using TSwGk::GetSampleN;

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
        using TSwGk::GetSummarySize;
        uint64 GetMemUsed() const;
        using TSwGk::PrintSummary;
        using TSwGk::GetValCount;
        using TSwGk::GetValRecount;
        using TSwGk::GetSampleN;

    private:
        TUInt64 WindowMSec;
    };

    std::ostream& operator <<(std::ostream& os, const TUInt& Val);

    namespace TStat {

        // returns the value of the Kolmogorov-Smirnov statistic
        template <typename TDistEst1, typename TDistEst2>
        double KolmogorovSmirnov(const TDistEst1& DistEst1, const TDistEst2& DistEst2);
        // performs the Kolmogorov-Smirnov test for the given value of `Alpha`
        template <typename TDistEst1, typename TDistEst2>
        bool KolmogorovSmirnovTest(const TDistEst1& DistEst1, const TDistEst2& DistEst2,
                const double& Alpha);
    }
}

#include "quantiles.hpp"

#endif

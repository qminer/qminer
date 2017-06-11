#ifndef _STREAM_QUANTILES_H
#define _STREAM_QUANTILES_H

#include <iostream>

namespace TQuant {

    class TGkTuple;

    template <typename T1, typename T2, typename T3>
    std::ostream& operator <<(std::ostream& os, const TTriple<T1, T2, T3>& Triple);

    template <typename T>
    std::ostream& operator <<(std::ostream& os, const TVec<T>& Vec);

    std::ostream& operator <<(std::ostream& os, const TGkTuple& Tup);

    std::ostream& operator <<(std::ostream& os, const TUInt& Val);


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
        int GetBand(const TGkTuple&, const uint64& MnRank) const;

        static constexpr double NUM_EPS = 1e-12;


        using TSummary = TVec<TGkTuple>;  // TODO use a linked list or some other structure

        TSummary Summary {};
        TUInt64 SampleN {0ul};
        TFlt Quant0;
        TFlt Eps;
        TInt Dir {1};
        TBool UseBands;
    };


    class TInterval {
    private:
        TUInt64 StartTm;
        TUInt64 DurMSec {0ul};  // the interval covers [StartTm, StartTm + DurMSec]
        TUInt ElCount {1u};

    public:
        TInterval();
        TInterval(const uint64& StartTm);
        TInterval(const uint64& StartTm, const uint64& Dur, const uint& Count);

        const TUInt& GetCount() const { return ElCount; }
        const TUInt64& GetStartTm() const { return StartTm; }
        const TUInt64& GetDurMSec() const { return DurMSec; }

        uint64 GetEndTm() const;

        void Swallow(const TInterval&);
    };

    class TIntervalWithMax : public TInterval {
    private:
        TFlt MxVal;             // the maximal value merged into the interval

    public:
        TIntervalWithMax();
        TIntervalWithMax(const uint64& BeginTm, const double& Val);

        const TFlt& GetMxVal() const { return MxVal; }

        void Swallow(const TIntervalWithMax&);
    };

    std::ostream& operator <<(std::ostream&, const TInterval&);
    std::ostream& operator <<(std::ostream&, const TIntervalWithMax&);
    std::ostream& operator <<(std::ostream&, const TUIntUInt64Pr&);


    //////////////////////////////////////////////
    /// Base class with most of the logic needed by
    /// exponential histograms
    template <typename TInterval>
    class TExpHistBase {
        // TODO make OnIntervalForgotten public???
    protected:
        using TIntervalV = TVec<TInterval>;

    private:

        TIntervalV IntervalV {};
        TUIntV LogSizeToBlockCountV {1};  // at index i stores the number of blocks of size (i+1)
        TUInt64 WindowMSec;
        TFlt Eps;
        TUInt TotalCount {0u};

    public:
        /// Sets the time window and error. The (approximate) count
        /// of the elements in time window [curr_time, curr_time - window)
        /// in the bin is at most (1 + eps)*real_count.
        TExpHistBase(const uint64& WindowMSec, const double& Eps);
        virtual ~TExpHistBase() {}

        /// returns the (approximate) number of items in the time window
        uint GetCount() const;

        /// merges the other exponential histogram into itself
        void Swallow(const TExpHistBase<TInterval>&);

        /// returns the number of intervals in the summary
        uint GetSummarySize() const;
        /// prints the intervals to stadard output
        void PrintSummary() const { std::cout << IntervalV << "\n"; }

        /// checks if the error is bounded by eps
        bool CheckInvariant1() const;
        /// checks if the internal structure of the EH is correct
        bool CheckInvariant2() const;

    protected:
        void Add(const TInterval&);
        void Compress();

        virtual void OnIntervalForgotten(const TInterval&) {}

    private:
        void Forget(const uint64& CurrTm);
        void CompressOldestInBatch(const int& BatchPos);

        TInterval& GetInterval(const int& IntervalN);
        TInterval& GetPrevInterval(const int& IntervalN);

        TUInt& LogBlockSizeToBlockCount(const uint& LogBlockSize);
        TUInt& BlockSizeToBlockCount(const uint& BlockSize);
        void ReserveStructures(const uint& LogBlockSize);

        uint GetMnBlocksSameSize() const;
        uint GetMxBlocksSameSize() const;

        // helper functions for the merge operation
        static void MergeAddItemBatch(const uint& BatchSize, const uint64& BatchTm,
                const uint& BlockSize, TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV);
        static void MergeFlushCarryInfo(const uint& BlockSize, const uint64 EndTm,
                TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV);
        static void MergeCloseInterval(const TIntervalV& IntervalV, int& OpenN,
                const TIntervalV& OthrIntervalV, const int& OthrN,
                uint& CurrBlockSize, TUIntUInt64Pr& CarryInfo, TIntervalV& NewIntervalV);

    public:
        // TODO should these be deleted???
        const TIntervalV& GetIntervalV() const { return IntervalV; }

    };


    //////////////////////////////////////////////
    /// Exponential Histogram
    /// (approximately) counts the number of items
    /// in a time window
    class TExpHistogram : public TExpHistBase<TInterval> {
    private:
        using TBase = TExpHistBase<TInterval>;

    public:
        using TBase::TExpHistBase;

        void Add(const uint64& Tm);
    };

    //////////////////////////////////////////////
    /// Exponential Histogram which also holds the
    /// maximum of the items
    class TExpHistWithMax : public TExpHistBase<TIntervalWithMax> {
    private:
        using TBase = TExpHistBase<TIntervalWithMax>;
        using TIntervalV = TBase::TIntervalV;   // TODO when done check if this is needed

        TFlt MxVal; // the (approximate) maximum value on the interval

    public:
        using TBase::TExpHistBase;

        /// adds a new item
        void Add(const uint64& Tm, const double& Val);
        /// returns the (approximate) number of items in the time window
        /* int GetCount() const; */
        /// returns the maximum value stored by the exponential histogram
        double GetMxVal() const;

    private:
        void OnIntervalForgotten(const TIntervalWithMax&);
    };

    ////////////////////////////////////////////
    /// Greenwald-Khanna algorithm on a sliding
    /// window.
    ///
    /// Described in
    /// "Online Algorithm for Approximate Quantile Queries on Sliding Windows"
    /// http://dl.acm.org/citation.cfm?id=2954329
    class TSwGk {
    private:
        class TTuple {
        private:
            TExpHistWithMax TupleSizeExpHist;
            TExpHistogram MnMxRankDiffExpHist;

        public:

            double GetVal() const;
            double GetTupleSize() const;
            double GetMnMxRankDiff() const;
        };

        using TSummary = TVec<TTuple>;

        TSummary Summary;
        TFlt EpsGk;
        TFlt EpsEh;

    public:
        double Query(const double& Quantile) const;
        void Insert(const double& Val);

        /// compresses the summary
        void Compress();
        /// returns the number of tuples in the summary
        int GetSummarySize() const;
        // TODO
    };

    using TGk = TGreenwaldKhanna;
    using TCkms = TBiasedGk;
}

#include "quantiles.hpp"

// TODO all GK based: let query quantiles be 0 and 1

#endif

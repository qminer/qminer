#ifndef _STREAM_QUANTILES_H
#define _STREAM_QUANTILES_H

namespace TQuant {

    class TGkTuple {
    private:
        TFlt Val;
        TUInt TupleSize;
        TUInt MnMxRankDiff;

    public:
        TGkTuple();
        TGkTuple(const TFlt& Val, const TUInt& MnRankDiff, const TUInt& Delta);

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
        TBool UseBands;
    };

    using TGk = TGreenwaldKhanna;
    using TCkms = TBiasedGk;
}

// TODO all GK based: let query quantiles be 0 and 1

#endif

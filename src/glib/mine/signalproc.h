#ifndef SIGNALPROC_H_
#define SIGNALPROC_H_

/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TSignalProc {

/////////////////////////////////////////////////
/// Online Moving Average
class TMaSimple {
private:
    TFlt Ma; // current computed MA value  
    TUInt64 N;
public:
    TMaSimple() { };
    TMaSimple(const PJsonVal& ParamVal) { };
    TMaSimple(TSIn& SIn) : Ma(SIn), N(SIn) {}
    // serialization
    void Load(TSIn& SIn) { *this = TMaSimple(SIn); }
    void Save(TSOut& SOut) const { Ma.Save(SOut); N.Save(SOut); }
    void Update(const double& InVal) { Ma = Ma + (InVal - Ma) / double(++N); }
    double GetMa() const { return Ma; }
    void Clr() { Ma = 0; N = 0; }
};

/////////////////////////////////////////////////
// Online M2 (variance)
class TVarSimple {
private:
    TFlt OldM;
    TFlt NewM;
    TFlt OldS;
    TFlt NewS;
    TUInt64 N;
public:
    TVarSimple() { }
    TVarSimple(TSIn& SIn): OldM(SIn), NewM(SIn), OldS(SIn), NewS(SIn), N(SIn) { }

    // serialization
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;
    void Update(const double& InVal);
    // current status   
    uint64 GetN() const { return N; }
    double GetMean() const { return N > 0 ? (double)NewM : 0.0; }
    double GetStDev() const { return sqrt((double)GetVar()); }
    double GetVar() const { return (N > 1 ? NewS / double(N - 1) : 0.0); }
    void Clr() { OldM = NewM = OldS = NewS = 0.0; N = 0; }
};

/////////////////////////////////////////////////
/// Online Moving Average
class TMa {
private:
    /// Number of elements in the current MA value
    TUInt64 Count;
    /// Current computed MA value
    TFlt Ma;
    /// Timestamp of the current value
    TUInt64 TmMSecs;

    /// Add value
    void AddVal(const double& InVal);
    /// Delete value
    void DeleteVal(const double& OutVal);
    
public:
    TMa() { }
    TMa(const PJsonVal& ParamVal) { }
    TMa(TSIn& SIn): Count(SIn), Ma(SIn), TmMSecs(SIn) { }
    
    /// Load state
    void Load(TSIn& SIn);
    /// Save state
    void Save(TSOut& SOut) const;

    /// Check if we saw at least one value
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Count = 0; Ma = 0.0; TmMSecs = 0; }
    
    /// Update with a value to add and values to delete
    void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    /// Update with values to add and values to delete
    void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    /// Get current moving average value
    
    double GetValue() const { return Ma; }
    /// Get timestamp of the current value
    uint64 GetTmMSecs() const { return TmMSecs; }
};
    
/////////////////////////////////////////////////
/// Online Sum
class TSum {
private:
    /// Current computed SUM value
    TFlt Sum;
    /// Timestamp of current value
    TUInt64 TmMSecs;
    
public:
    TSum() { }
    TSum(const PJsonVal& ParamVal) { }
    TSum(TSIn& SIn): Sum(SIn), TmMSecs(SIn) { }

    /// Load state
    void Load(TSIn& SIn);
    /// Save state
    void Save(TSOut& SOut) const;

    /// Check if we saw at least one value
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Sum = 0; TmMSecs = 0; }
    /// Update with a value to add and values to delete
    void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    /// Update with values to add and values to delete
    void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    
    /// Get current sum value    
    double GetValue() const { return Sum; }
    /// Get timestamp of the current value
    uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
/// Online Sum of sparse vectors
class TSumSpVec {
private:
    /// Current computed SUM value
    TVec<TIntFltKd> Sum;
    /// Timestamp of current MA
    TUInt64 TmMSecs;

    /// Add new value to the sum
    void AddVal(const TIntFltKdV& SpV);
    /// Delete old value from the sum
    void DelVal(const TIntFltKdV& SpV);
    
public:
    /// Simple constructor
    TSumSpVec() {};
    /// Initialization from JSON value
    TSumSpVec(const PJsonVal& ParamVal) {};
    /// Deserialization constructor
    TSumSpVec(TSIn& SIn) : Sum(SIn), TmMSecs(SIn) {} 

    /// Loading from binary stream
    void Load(TSIn& SIn);
    /// Saving to binary stream
    void Save(TSOut& SOut) const;

    /// Checks if this sum received any data yet
    bool IsInit() const { return (TmMSecs > 0); } 
    /// Resets the model state
    void Reset() { Sum = TIntFltKdV(); TmMSecs = 0; }
    /// Updates internal state with incoming and outgoing data
    void Update(const TVec<TIntFltKd>& InVal, const uint64& InTmMSecs,
        const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecs);
    /// Updates internal state with incoming and outgoing data - for delayed update
    void Update(const TVec<TIntFltKdV>& InValV, const TUInt64V& InTmMSecsV,
        const TVec<TIntFltKdV>& OutValV, const TUInt64V& OutTmMSecs);

    /// Access current sum
    const TIntFltKdV& GetValue() const { return Sum; }
    /// Access last received timestampe
    uint64 GetTmMSecs() const { return TmMSecs; }
    /// Get JSON description of the sum
    PJsonVal GetJson() const;
};

/////////////////////////////////////////////////
/// Sliding Window Min
class TMin {
private:
    /// Current computed min value
    TFlt Min;
    /// Timestamp of current min value
    TUInt64 TmMSecs;
    /// Sorted vector of potential min candidates.
    /// TODO: Replace TVec with TQQueue
    TFltUInt64PrV AllValV;

    /// Add new value
    void AddVal(const double& InVal, const uint64& InTmMSecs);
    /// Delete old value (we just care for its timestamp)
    void DelVal(const uint64& OutTmMSecs);
    
public:
    TMin(): Min(TFlt::Mx) { }
    TMin(TSIn& SIn): Min(SIn), TmMSecs(SIn), AllValV(SIn) { }

    /// Loading from binary stream
    void Load(TSIn& SIn);
    /// Saving to binary stream    
    void Save(TSOut& SOut) const;

    /// Check if we saw at least one value
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Min = TFlt::Mx; TmMSecs = 0; }
    /// Update with a value to add and values to delete    
    void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    /// Update with values to add and values to delete
    void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV,
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV);
    
    /// Get current min value    
    double GetValue() const { return Min; }
    /// Get timestamp of the current value
    uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Sliding Window Max
class TMax {
private:
    /// current computed SUM value 
    TFlt Max;
    /// timestamp of current MA    
    TUInt64 TmMSecs;
    /// Sorted vector of potential min candidates.
    /// TODO: Replace TVec with TQQueue
    TFltUInt64PrV AllValV;

    /// Add new value
    void AddVal(const double& InVal, const uint64& InTmMSecs);
    /// Delete old value (we just care for its timestamp)
    void DelVal(const uint64& OutTmMSecs);
    
public:
    TMax(): Max(TFlt::Mn) { };
    TMax(TSIn& SIn): Max(SIn), TmMSecs(SIn), AllValV(SIn) { }

    /// Loading from binary stream
    void Load(TSIn& SIn);
    /// Saving to binary stream    
    void Save(TSOut& SOut) const;

    /// Check if we saw at least one value
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Max = TFlt::Mn; TmMSecs = 0; }
    /// Update with a value to add and values to delete
    void Update(const double& InVal, const uint64& InTmMSecs,
        const TFltV& OutValV, const TUInt64V& OutTmMSecs);
    /// Update with values to add and values to delete
    void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV,
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV);
    
    /// Get current max value        
    double GetValue() const { return Max; }
    /// Get timestamp of the current value
    uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Exponential Moving Average
typedef enum { etPreviousPoint, etLinear, etNextPoint } TEmaType;
class TEma {
private:
    // parameters
    TFlt Decay; ///< decaying factor
    TEmaType Type; ///< interpolation type
    // current state
    TFlt LastVal; ///< last input value
    TFlt Ema; ///< current computed EMA value 
    TUInt64 TmMSecs; ///< timestamp of current EMA
    double TmInterval; ///< time interval for definition of decay
    // buffer for initialization
    TBool InitP; ///< true if already initialized
    TUInt64 InitMinMSecs; ///< time window of required values for initialization
    TFltV InitValV; ///< first N values
    TUInt64V InitMSecsV; ///< weights of first N values
    
    double GetNi(const double& Alpha, const double& Mi);
public:
    TEma(const double& _Decay, const TEmaType& _Type, 
        const uint64& _InitMinMSecs, const double& _TmInterval);
    TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,
        const double& _TmInterval);
    TEma(const PJsonVal& ParamVal);
    TEma(TSIn& SIn);
    // serialization
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;

    void Update(const double& Val, const uint64& NewTmMSecs);
    // current status
    bool IsInit() const { return InitP; }
    /// Resets the aggregate
    void Reset();
    double GetValue() const { return Ema; }
    uint64 GetTmMSecs() const { return TmMSecs; }
};

class TEmaSpVec {
private:
    // parameters
    TEmaType Type; ///< interpolation type
    // current state
    TIntFltKdV LastVal; ///< last input value
    TIntFltKdV Ema; ///< current computed EMA value 
    TUInt64 TmMSecs; ///< timestamp of current EMA
    TFlt TmInterval; ///< time interval for definition of decay
    TFlt Cutoff; ///< Minimal value for dimension - if it falls below this, it is removed from Ema
    // buffer for initialization
    TBool InitP; ///< true if already initialized
    TUInt64 InitMinMSecs; ///< time window of required values for initialization
    TVec<TIntFltKdV> InitValV; ///< first N values
    TUInt64V InitMSecsV; ///< weights of first N values

    double GetNi(const double& Alpha, const double& Mi);
public:
    TEmaSpVec(const PJsonVal& ParamVal);
    TEmaSpVec(TSIn& SIn);

    // serialization
    void Load(TSIn& SIn);
    void Save(TSOut& SOut) const;

    void Update(const TIntFltKdV& Val, const uint64& NewTmMSecs);
    // current status
    bool IsInit() const { return InitP; }
    
    /// Resets the aggregate
    void Reset();
    const TIntFltKdV GetValue() const { return Ema; }
    uint64 GetTmMSecs() const { return TmMSecs; }
    
    PJsonVal GetJson() const; ///< Get JSON description of the sum
};

/////////////////////////////////////////////////
/// Online Moving Variance M2(X).
class TVar {
private:
    /// Count of values in the window
    TUInt64 Count;
    /// current computed MA value
    TFlt Ma;
    /// current computed M2 value
    TFlt M2;
    /// Current variance
    TFlt VarVal;
    /// timestamp of current value
    TUInt64 TmMSecs;

    /// Add new value to variance computation
    void AddVal(const double& InVal);
    /// Remove value from variance computation
    void DelVal(const double& OutVal);
    /// Update current variance value
    void UpdateVar() { VarVal = (Count > 1) ? (M2 / ((double)Count - 1.0)) : 0.0; }
    
public:
    TVar() { }
    TVar(TSIn& SIn): Count(SIn), Ma(SIn), M2(SIn), VarVal(SIn), TmMSecs(SIn) { }

    /// Loading from binary stream
    void Load(TSIn& SIn);
    /// Saving to bianry stream
    void Save(TSOut& SOut) const;

    /// Check if we got any value so far
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Count = 0; Ma = 0.0; M2 = 0.0; VarVal = 0.0; TmMSecs = 0; }
    /// Update with a value to add and values to delete
    void Update(const double& InVal, const uint64& InTmMSecs, const TFltV& OutValV, const TUInt64V& OutTmMSecsV);
    /// Update with values to add and values to delete    
    void Update(const TFltV& InValV, const TUInt64V& InTmMSecsV, const TFltV& OutValV, const TUInt64V& OutTmMSecsV);

    /// Current variance
    double GetValue() const { return VarVal; }
    /// Timestamp of current variance
    uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
/// Online Moving Covariance M2(X,Y).
/// Assumes X and Y have the same time stamp
class TCov {
private:
    /// Count of values in the window
    TUInt64 Count;    
    /// current computed MA value for X
    TFlt MaX; 
    /// current computed MA value for Y
    TFlt MaY;
    /// Current computed non-normalized Cov value
    TFlt M2;
    /// Current computed covariance
    TFlt CovVal;
    /// Timestamp of current value
    TUInt64 TmMSecs;

    /// Add new value to variance computation
    void AddVal(const double& InValX, const double& InValY);
    /// Remove value from variance computation
    void DelVal(const double& OutValX, const double& OutValY);    
    /// Update current covariance value
    void UpdateCov() { CovVal = (Count > 1) ? (M2 / ((double)Count - 1.0)) : 0; }    
    
public:
    TCov() { };
    TCov(TSIn& SIn): Count(SIn), MaX(SIn), MaY(SIn), M2(SIn), CovVal(SIn), TmMSecs(SIn) { }

    /// Loading from binary stream
    void Load(TSIn& SIn);
    /// Saving to bianry stream
    void Save(TSOut& SOut) const;
    
    /// Check if we got any value so far
    bool IsInit() const { return (TmMSecs > 0); }
    /// Resets the model state
    void Reset() { Count = 0; MaX = 0.0; MaY = 0.0; M2 = 0.0; CovVal = 0.0; TmMSecs = 0; }
    /// Update with a value to add and values to delete
    void Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV);
    /// Update with values to add and values to delete
    void Update(const TFltV& InValVX, const TFltV& InValVY, const TUInt64V& InTmMSecsV, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV);

    /// Current covariance
    double GetCov() const { return CovVal; }
    /// Timestamp of current variance
    uint64 GetTmMSecs() const { return TmMSecs; }
};

///////////////////////////////
// Sequence buffer
template <class TVal>
class TBuffer {
private:
    // buffer size
    TInt BufferLen;
    // next place in buffer
    TInt NextValN;
    // internal vector used to store circular buffer
    TVec<TVal> ValV;
    
public:
    TBuffer(const int& BufferLen_): BufferLen(BufferLen_), 
        NextValN(0), ValV(BufferLen_, 0) { }

    // serialization
    void Load(TSIn& SIn) {
        BufferLen.Load(SIn);
        NextValN.Load(SIn);
        ValV.Load(SIn);
    }
    void Save(TSOut& SOut) const {
        BufferLen.Save(SOut);
        NextValN.Save(SOut);
        ValV.Save(SOut);
    }
    
    /// add new value to the buffer
    void Update(const TVal& Val){
        // we add to the end while not full
        if (ValV.Len() < BufferLen) { ValV.Add(Val); }
        // after we overwrite existing
        else { ValV[NextValN] = Val; }
        // we always point to next empty(-to-be) place
        NextValN++;
        // reset count once we come to BufferLen
        NextValN = NextValN % BufferLen;
    }
    
    /// Is buffered initialized 
    bool IsInit() const { return ValV.Len() == BufferLen; }
    /// Resets the buffer
    void Reset() { NextValN = 0; ValV.Gen(0); }
    /// Is buffer empty
    bool Empty() const { return ValV.Empty(); }
    /// Number of elements at the moment
    int Len() const { return ValV.Len(); }
        
    /// Get values from buffer (older values = larger index):
    ///  example buffer = [x_(t-3) x_(t-2) x_(t-1) x_t] => GetVal(i) returns x_(t-1)
    const TVal& GetVal(const int& ValN) const {
        return ValV[(NextValN + BufferLen - 1 - ValN) % BufferLen]; }
    /// Get the oldest value
    const TVal& GetOldest() const {
        Assert(!Empty()); return IsInit() ? GetVal(BufferLen - 1) : ValV[0]; }
    /// Get the most recent value
    const TVal& GetNewest() const {
        Assert(!Empty()); return IsInit() ?  GetVal(0) : ValV.Last(); }
};

///////////////////////////////
// Linked buffer
template <class TVal, class TSizeTy = TUInt64>
class TLinkedBuffer {
private:
    class Node {
    public:
        Node* Next;
        const TVal Val;

        Node(Node* _Next, const TVal& _Val): Next(_Next), Val(_Val) {}
    };

private:
    Node* First;
    Node* Last;
    TSizeTy Size;

public:
    TLinkedBuffer();
    TLinkedBuffer(TSIn& SIn);

    void Save(TSOut& SOut) const;

    ~TLinkedBuffer();

    void Add(const TVal& Val);
    void DelOldest();
    void Clr();

    const TVal& GetOldest(const TSizeTy& Idx) const;
    const TVal& GetOldest() const { return GetOldest(0); };
    const TVal& GetNewest() const;

    bool Empty() const { return Len() == 0; };
    TSizeTy Len() const { return Size; };
};

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::TLinkedBuffer():
        First(NULL),
        Last(NULL),
        Size() {}

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::TLinkedBuffer(TSIn& SIn):
        First(NULL),
        Last(NULL),
        Size(SIn) {

    if (Size > 0) { First = new TLinkedBuffer<TVal, TSizeTy>::Node(NULL, TVal(SIn)); }

    Node* Curr = First;
    for (TSizeTy i = 1; i < Size; i++) {
        Curr->Next = new Node(NULL, TVal(SIn));
        Curr = Curr->Next;
    }

    Last = Curr;
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Save(TSOut& SOut) const {
    Size.Save(SOut);

    Node* Curr = First;
    while (Curr != NULL) {
        Curr->Val.Save(SOut);
        Curr = Curr->Next;
    }
}

template <class TVal, class TSizeTy>
TLinkedBuffer<TVal, TSizeTy>::~TLinkedBuffer() {
    Clr();
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Add(const TVal& Val) {
    TLinkedBuffer<TVal, TSizeTy>::Node* Node = new TLinkedBuffer<TVal, TSizeTy>::Node(NULL, Val);

    if (Size++ == 0) {
        First = Node;
        Last = Node;
    } else {
        Last->Next = Node;
        Last = Node;
    }
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::DelOldest() {
    IAssertR(!Empty(), "Cannot delete elements from empty buffer!");

    Node* Temp = First;

    if (--Size == 0) {
        First = NULL;
        Last = NULL;
    } else {
        First = First->Next;
    }

    delete Temp;
}

template <class TVal, class TSizeTy>
void TLinkedBuffer<TVal, TSizeTy>::Clr() {
    while (!Empty()) { DelOldest(); }
}

template <class TVal, class TSizeTy>
const TVal& TLinkedBuffer<TVal, TSizeTy>::GetOldest(const TSizeTy& Idx) const {
    IAssertR(Idx < Size, "Index of element greater then size!");

    Node* Curr = First;
    for (TSizeTy i = 0; i < Idx; i++) {
        Curr = Curr->Next;
    }

    return Curr->Val;
}

template <class TVal, class TSizeTy>
const TVal& TLinkedBuffer<TVal, TSizeTy>::GetNewest() const {
    IAssertR(!Empty(), "Cannot return elements from empty buffer!");
    return Last->Val;
}

/////////////////////////////////////////
// Time series interpolator interface
class TInterpolator;
typedef TPt<TInterpolator> PInterpolator;
class TInterpolator {
private:
    TCRef CRef;
    friend class TPt<TInterpolator>;
private:
    const TStr InterpolatorType;
protected:
    TInterpolator(const TStr& _InterpolatorType): InterpolatorType(_InterpolatorType) {}
    TInterpolator(TSIn& SIn): InterpolatorType(SIn) {}

public:
    static PInterpolator New(const TStr& InterpolatorType);
    static PInterpolator Load(TSIn& SIn);

    virtual ~TInterpolator() { }

    virtual void Save(TSOut& SOut) const { InterpolatorType.Save(SOut); }

    virtual void SetNextInterpTm(const uint64& Time) = 0;
    virtual double Interpolate(const uint64& Time) const = 0;
    virtual bool CanInterpolate(const uint64& Time) const = 0;
    virtual void AddPoint(const double& Val, const uint64& Tm) = 0;
};

/////////////////////////////////////////
// Buffered interpolator
// contains a buffer
// the first timestamp in the buffer is less or equal to the current time
// the other timestamps in the buffer are greater then the current time
class TBufferedInterpolator: public TInterpolator {
protected:
    // buffer holding the current and future points
    TLinkedBuffer<TPair<TUInt64, TFlt>> Buff;

    TBufferedInterpolator(const TStr& InterpolatorType);
    TBufferedInterpolator(const TStr& InterpolatorType, TSIn& SIn);

public:
    virtual void Save(TSOut& SOut) const;

    void AddPoint(const double& Val, const uint64& Tm);
};

/////////////////////////////////////////
// Previous point interpolator.
// Interpolate by returning previously seen value
// this interpolator will wait until it gets one value in the future before
// performing interpolation
class TPreviousPoint : public TBufferedInterpolator {
private:
    TPreviousPoint();
    TPreviousPoint(TSIn& SIn);
public: 
    static PInterpolator New() { return new TPreviousPoint; }
    static PInterpolator New(TSIn& SIn) { return new TPreviousPoint(SIn); }
    
    void SetNextInterpTm(const uint64& Time);
    double Interpolate(const uint64& TmMSecs) const;
    bool CanInterpolate(const uint64& Tm) const;

    static TStr GetType() { return "previous"; }
};

/////////////////////////////////////////
// Current point interpolator.
// Interpolate by returning the current point
class TCurrentPoint: public TBufferedInterpolator {
private:
    TCurrentPoint();
    TCurrentPoint(TSIn& SIn);

public:
    static PInterpolator New() { return new TCurrentPoint; }
    static PInterpolator New(TSIn& SIn) { return new TCurrentPoint(SIn); }

    void SetNextInterpTm(const uint64& Time);
    double Interpolate(const uint64& Tm) const;
    bool CanInterpolate(const uint64& Tm) const;

    static TStr GetType() { return "current"; }
};

/////////////////////////////////////////
// Linear interpolator.
// Interpolate by calculating point between two given points
class TLinear : public TBufferedInterpolator {
private:
    TLinear();
    TLinear(TSIn& SIn);
public: 
    static PInterpolator New() { return new TLinear; }
    static PInterpolator New(TSIn& SIn) { return new TLinear(SIn); }

    void SetNextInterpTm(const uint64& Tm);
    double Interpolate(const uint64& Tm) const;
    bool CanInterpolate(const uint64& Tm) const;

    static TStr GetType() { return "linear"; }
};

/////////////////////////////////////////
// Neural Networks - Neural Net
typedef enum { tanHyper, sigmoid, fastTanh, fastSigmoid, linear, softPlus } TTFunc;
//class TNNet {
ClassTP(TNNet, PNNet) //{
private:
    /////////////////////////////////////////
    // Neural Networks - Neuron
    class TLayer;    
    class TNeuron {
    private:
        static TRnd Rnd; //TODO: initialize it in the constructor with the 0

        TFlt RandomWeight(void) { return TFlt(Rnd.GetUniDev()); }
        TFlt TransferFcn(TFlt Sum);
        TFlt TransferFcnDeriv(TFlt Sum); // for back propagation learning
        // sum derivatives of weights in the next layer
        TFlt SumDOW(const TLayer& NextLayer) const;
        TFlt OutputVal;
        TFlt Gradient;
        TTFunc TFuncNm; // transfer function name
        TFltV SumDeltaWeight;
        // hash containing weight[0] and delta weight[1] for each outgoing edge
        TVec<TIntFltFltTr> OutEdgeV; 
        // Id of this neuron
        TInt Id;

    public:
        TNeuron();
        TNeuron(TInt OutputsN, TInt MyId, TTFunc TransFunc);
        TNeuron(TSIn& SIn);

        void SetOutVal(const TFlt& Val) { OutputVal = Val; }
        void SetDeltaWeight(const TInt& InNodeId, const TFlt& Val) { OutEdgeV[InNodeId].Val3 = Val; }
        void SetSumDeltaWeight(const TInt& InNodeId, const TFlt& Val) { SumDeltaWeight[InNodeId] = Val; }
        void SumDeltaWeights(const TInt& InNodeId, const TFlt& Val) { SumDeltaWeight[InNodeId] += Val; }
        void UpdateWeight(const TInt& InNodeId, const TFlt& Val) { OutEdgeV[InNodeId].Val2 += Val; }
        void UpdateInputWeights(TLayer& PrevLayer, const TFlt& LearnRate, const TFlt& Momentum, const TBool& UpdateWeights);

        TFlt GetOutVal(void) const { return OutputVal; }
        TFlt GetGradient(void) const { return Gradient; }
        TFlt GetDeltaWeight(const TInt& InNodeId) { return OutEdgeV[InNodeId].Val3; };
        TFlt GetWeight(const TInt& InNodeId) const { return OutEdgeV[InNodeId].Val2; };
        TFlt GetSumDeltaWeight(const TInt& InNodeId) const { return SumDeltaWeight[InNodeId]; };

        void FeedFwd(const TLayer& PrevLayer);
        void CalcOutGradient(TFlt TargVal);
        void CalcHiddenGradient(const TLayer& NextLayer);
        // Save the model
        void Save(TSOut& SOut);

        const TTFunc& GetFunction() { return TFuncNm; }
    };

    /////////////////////////////////////////
    // Neural Networks - Layer of neurons
    class TLayer {
    private:
        TVec<TNeuron> NeuronV;

    public:
        TLayer();
        TLayer(const TInt& NeuronsN, const TInt& OutputsN, const TTFunc& TransFunc);
        TLayer(TSIn& SIn);

        int GetNeuronN() const { return NeuronV.Len(); };
        TNeuron& GetNeuron(const TInt& NeuronN) { return NeuronV[NeuronN]; };
        TFlt GetOutVal(const TInt& NeuronN) const { return NeuronV[NeuronN].GetOutVal(); };
        TFlt GetGradient(const TInt& NeuronN) const { return NeuronV[NeuronN].GetGradient(); };
        TFlt GetWeight(const TInt& NeuronN, const TInt TargetId) const { return NeuronV[NeuronN].GetWeight(TargetId); };

        void SetOutVal(const TInt& NeuronN, const TFlt& Val) { NeuronV[NeuronN].SetOutVal(Val); };
        void UpdateInputWeights(const TInt& NeuronN, TLayer& PrevLayer,
        const TFlt& LearnRate, const TFlt& Momentum,
        const TBool& UpdateWeights) { NeuronV[NeuronN].UpdateInputWeights(PrevLayer, LearnRate, Momentum, UpdateWeights); };

        void CalcOutGradient(int& NeuronN, const TFlt& TargVal) { NeuronV[NeuronN].CalcOutGradient(TargVal); };
        void CalcHiddenGradient(int& NeuronN, const TLayer& NextLayer) { NeuronV[NeuronN].CalcHiddenGradient(NextLayer); };
        void FeedFwd(const TInt& NeuronN, const TLayer& PrevLayer) { NeuronV[NeuronN].FeedFwd(PrevLayer); };
        // Save the model
        void Save(TSOut& SOut);
    };

    TVec<TLayer> LayerV; 
    TFlt LearnRate; // [0.0..1.0] learning rate 
    TFlt Momentum; // [0.0..n] multiplier of last weight change
    TFlt Error;
    TFlt RecentAvgError;
    TFlt RecentAvgSmoothingFactor;


public:
    // constructor
    TNNet(const TIntV& LayoutV, const TFlt& _LearnRate = 0.1, 
            const TFlt& _Momentum = 0.5, const TTFunc& TFuncHiddenL = tanHyper,
            const TTFunc& TFuncOutL = tanHyper);
    TNNet(TSIn& SIn); // JOST: A rabim tudi nov PNNet za ta konstruktor?===
    static PNNet New(const TIntV& LayoutV, const TFlt& _LearnRate = 0.1, 
            const TFlt& _Momentum = 0.5, const TTFunc& TFuncHiddenL = tanHyper,
            const TTFunc& TFuncOutL = tanHyper)
            { return new TNNet(LayoutV, _LearnRate, _Momentum, TFuncHiddenL, TFuncOutL); }
    static PNNet Load(TSIn& SIn);
    // Feed forward step
    void FeedFwd(const TFltV& InValV);
    // Back propagation step
    void BackProp(const TFltV& TargValV, const TBool& UpdateWeights = true);
    // TODO: try to make backprop in less for loops
    void GetResults(TFltV& ResultV) const;
    // Set learn rate
    void SetLearnRate(const TFlt& NewLearnRate) { LearnRate = NewLearnRate; };
    // set momentum
    void SetMomentum(const TFlt& NewMomentum) { Momentum = NewMomentum; }
    // Save the model
    void Save(TSOut& SOut) const;

    void GetLayout(TIntV& layout) {
        layout.Gen(LayerV.Len());
        for (int i = 0; i < LayerV.Len(); i++) {
            layout[i] = LayerV[i].GetNeuronN() - 1;
        }
    }
    TFlt GetLearnRate() { return LearnRate; }
    TFlt GetMomentum() { return Momentum; }
    TStr GetTFuncHidden() { 
        TStr FuncHidden = GetFunction(LayerV[1].GetNeuron(0).GetFunction());
        return FuncHidden;
    };
    TStr GetTFuncOut() {
        TStr FuncOut = GetFunction(LayerV[LayerV.Len() - 1].GetNeuron(0).GetFunction());
        return FuncOut;
    }
    TStr GetFunction(const TTFunc& Func);
};

/////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
    TFlt ForgetFact;    // forgetting factor
    TFlt RegFact;       // regularization

    TFullMatrix P;      // correlation matrix
    TVector Coeffs;     // model

    // copy constructor
    TRecLinReg(const TRecLinReg& LinReg);
    // move constructor
    TRecLinReg(const TRecLinReg&& LinReg);
    // load constructor
    TRecLinReg(TSIn& SIn);
    // default constructor
    TRecLinReg(const int& Dim, const double& _RegFact = 1.0, const double& _ForgetFact = 1.0);
public: 
    // new method
    static PRecLinReg New(const int& Dim, const double& RegFact, const double& ForgetFact)
            { return new TRecLinReg(Dim, RegFact, ForgetFact); }
    static PRecLinReg Load(TSIn& SIn);

    virtual ~TRecLinReg() {}

    void Save(TSOut& SOut) const;   

    // assign operator
    TRecLinReg& operator =(TRecLinReg LinReg);

    // returns the dimension of the instances
    int GetDim() const { return Coeffs.Len(); }
    // returns the forgetting factor
    double GetForgetFact() const { return ForgetFact; }
    // returns the regulatization parameter
    double GetRegFact() const { return RegFact; }
    // learns a new sample
    void Learn(const TFltV& Sample, const double& SampleVal);
    // predicts a value
    double Predict(const TFltV& Sample);
    // copies the current models coefficients into the specified list
    void GetCoeffs(TFltV& Coef) const;
    // check if the coefficient vector contains NaN
    bool HasNaN() const;

    // set the forgetting factor
    void setForgetFact(const double& _ForgetFact) { ForgetFact = _ForgetFact; }
    // set the regularization
    void setRegFact(const double& _RegFact) { RegFact = _RegFact; }
    // set the dimensions
    void setDim(const int& _Dim) {
        P = TFullMatrix::Identity(_Dim) / RegFact;
        Coeffs = TVector(_Dim, true);
    }
};


/////////////////////////////////////////////////
/// Online histogram.
///    Given a sequence of points b_1, ...,b_n
///    the class represents a frequency histogram for each interval [b_i, b_i+1)
///    The intervals are open on the right, except for the last interval [b_n-1 b_n]
///    The count data can be incremented or decremented, so we can work in an online
///    setting.
class TOnlineHistogram {
private:
    // state
    TFltV Counts; ///< Number of occurrences
    TFltV Bounds; ///< Interval bounds (Bounds.Len() == Counts.Len() + 1)
    TFlt Count; ///< Sum of counts
    // parameters
    TFlt MinCount; ///< If Count < MinCount, then IsInit returns false
public: 
    /// Constructs uninitialized object
    TOnlineHistogram() {};
    /// Constructs given bin parameters
    TOnlineHistogram(const double& LBound, const double& UBound, const int& Bins, const bool& AddNegInf, const bool& AddPosInf) { Init(LBound, UBound, Bins, AddNegInf, AddPosInf); }
    /// Constructs given JSON arguments
    TOnlineHistogram(const PJsonVal& ParamVal);
    /// Constructs from stream
    TOnlineHistogram(TSIn& SIn) : Counts(SIn), Bounds(SIn), Count(SIn) { }

    /// Initializes the object, resets current content is present
    void Init(const double& LBound, const double& UBound, const int& Bins, const bool& AddNegInf, const bool& AddPosInf);

    /// Resets the counts
    void Reset();

    /// Loads the model from stream
    void Load(TSIn& SIn) { *this = TOnlineHistogram(SIn); }
    /// Saves the model to stream
    void Save(TSOut& SOut) const { Counts.Save(SOut); Bounds.Save(SOut); SOut.Save(Count); }
    /// Finds the bin index given val, returns -1 if not found
    int FindBin(const double& Val) const;
    /// Increments the number of occurrences of values that fall within the same bin as Val
    void Increment(const double& Val);
    /// Decrements the number of occurrences of values that fall within the same bin as Val
    void Decrement(const double& Val);
    /// Returns the number of occurrences of values that fall within the same bin as Val
    double GetCount(const double& Val) const;
    /// Returns the number of bins
    int GetBins() const { return Counts.Len(); }
    /// Copies the count vector
    void GetCountV(TFltV& Vec) const { Vec = Counts; }
    /// Returns an element of count vector given index
    double GetCountN(const int& CountN) const { return Counts[CountN]; }
    /// Returns an element of bound vector given index
    double GetBoundN(const int& BoundN) const { return Bounds[BoundN]; }
    /// Has the model beeen initialized and has sufficient data?
    bool IsInit() const { return Counts.Len() > 0 && Bounds.Len() > 0 && Count >= MinCount; }
    /// Clears the model
    void Clr() { Counts.Clr(); Bounds.Clr(); }
    /// Prints the model
    void Print() const;
    /// Returns a JSON representation of the model
    PJsonVal SaveJson() const;
};

/////////////////////////////////////////////////
/// TDigest.
///   Data structure useful for percentile and quantile estimation for online data streams.
///   It can be added to any anomaly detector to set the number of alarms triggered as a percentage of the total samples.
///   This is the Data Lib Sketch Implementation: https://github.com/vega/datalib-sketch/blob/master/src/t-digest.js
///    Paper: Ted Dunning, Otmar Ertl - https://github.com/tdunning/t-digest/blob/master/docs/t-digest-paper/histo.pdf
class TTDigest {
private:
    TInt Nc;
    TInt Size;
    TInt Last;
    TFlt TotalSum;
    TFltV Weight;
    TFltV Mean;
    TFlt Min;
    TFlt Max;
    // double buffer to simplify merge operations
    // MergeWeight also used for transient storage of cumulative weights
    TFltV MergeWeight;
    TFltV MergeMean;
    // temporary buffers for recently added values
    TInt Tempsize;
    TFlt UnmergedSum;
    TInt TempLast;
    TFltV TempWeight;
    TFltV TempMean;
    TFltV Quantiles;

    TInt Updates;
    TInt MinPointsInit;

    // Given the number of centroids, determine temp buffer size
    // Perform binary search to find value k such that N = k log2 k
    // This should give us good amortized asymptotic complexity
    int NumTemp(const int& N) const;
    // Converts a quantile into a centroid index value. The centroid index is
    // nominally the number k of the centroid that a quantile point q should
    // belong to. Due to round-offs, however, we can't align things perfectly
    // without splitting points and centroids. We don't want to do that, so we
    // have to allow for offsets.
    // In the end, the criterion is that any quantile range that spans a centroid
    // index range more than one should be split across more than one centroid if
    // possible. This won't be possible if the quantile range refers to a single
    // point or an already existing centroid.
    // We use the arcsin function to map from the quantile domain to the centroid
    // index range. This produces a mapping that is steep near q=0 or q=1 so each
    // centroid there will correspond to less q range. Near q=0.5, the mapping is
    // flatter so that centroids there will represent a larger chunk of quantiles.
    double Integrate(const double& Nc, const double& Q_) const;

    double MergeCentroid(double& Sum, double& K1, double& Wt, double& Ut);

    int Bisect(const TFltV& A, const double& X, int& Low, int& Hi) const;

    double Boundary(const int& I, const int& J, const TFltV& U, const TFltV& W) const;

    void Init(const int& N);
public:
    /// Constructs uninitialized object
    TTDigest() {
        Init(100);
    }
    /// Constructs given JSON arguments
    TTDigest(const PJsonVal& ParamVal) {
        MinPointsInit = ParamVal->GetObjInt("minCount", 0);
        if (ParamVal->IsObjKey("clusters")) {
            TInt N = ParamVal->GetObjInt("clusters");
            Init(N);
        }
        else {
            Init(100);
        }
    };
    /// Constructs initialized object
    TTDigest(const TInt& N) {
        Init(N);
    };
    // Destructor
    //~TTDigest() {}
    /// Initializes the object, resets current content if present
    void Init();
    // Query for estimated quantile *q*.
    // Argument *q* is a desired quantile in the range (0,1)
    // For example, q = 0.5 queries for the median.
    double GetQuantile(const double& Q) const;
    // Number of clusters
    int GetClusters() const;
    // Add a value to the t-digest.
    // Argument *v* is the value to add.
    // Argument *count* is the integer number of occurrences to add.
    // If not provided, *count* defaults to 1.
    void Update(const double& V, const double& Count = 1);
    bool IsInit() const { return Updates >= MinPointsInit; }
    void MergeValues();
    /// Prints the model
    void Print() const;
    /// Load from stream
    void LoadState(TSIn& SIn);
    /// Store state into stream
    void SaveState(TSOut& SOut) const;
};

/////////////////////////////////////////////////
/// Chi square
class TChiSquare {
private:         
    // state
    TFlt Chi2;
    TFlt P;
    // parameters
    TInt DegreesOfFreedom;
public:
    TChiSquare() : P(TFlt::PInf) { }
    TChiSquare(const PJsonVal& ParamVal);
    /// Reset
    void Reset() { Chi2 = 0; P = TFlt::PInf; }
    /// Compute two sample chi2 test
    void Update(const TFltV& OutValVX, const TFltV& OutValVY);
    /// Return Chi2 value
    double GetChi2() const { return Chi2; }
    /// Return P value
    double GetP() const { return P; }
    int GetDof() const {return DegreesOfFreedom;}
    /// Prints the model
    void Print() const;

    /// Load from stream
    void LoadState(TSIn& SIn);
    /// Store state into stream
    void SaveState(TSOut& SOut) const;
};

///////////////////////////////
/// Slotted histogram.
/// Maintains distribution statistics in time-slots.
class TSlottedHistogram {
protected:
    /// Period length in miliseconds
    TUInt64 PeriodLen;
    /// Slot granularity in miliseconds
    TUInt64 SlotGran;
    /// Number of bins
    TInt Bins;
    /// Data storage, index is truncated timestamp, data is histogram
    TVec<TOnlineHistogram> Dat;

    /// Given timestamp calculate index
    int GetIdx(const uint64 Ts) { return (int)((Ts % PeriodLen) / SlotGran); };
public:
    /// Empty constructor
    TSlottedHistogram() { }
    /// Constructor, reserves appropriate internal storage
    TSlottedHistogram(const uint64 _Period, const uint64 _Slot, const int _Bins);

    /// Resets the counts to 0
    void Reset();

    /// Load stream aggregate state from stream
    void LoadState(TSIn& SIn);
    /// Save state of stream aggregate to stream
    void SaveState(TSOut& SOut) const;

    /// Add new data to statistics
    void Add(const uint64& Ts, const int& Val);
    /// Remove data from statistics
    void Remove(const uint64& Ts, const int& Val);

    /// Provide statistics
    void GetStats(const uint64 TsMin, const uint64 TsMax, TFltV& Dest);
    /// Gets number of bins
    int GetBins() const { return Dat.Len(); }
};

}

#endif
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

namespace TSignalProc {

/////////////////////////////////////////////////
// Online Moving Average (M1))
class TMa {
private:	
    TFlt Ma; // current computed MA value 
	TUInt64 TmMSecs; // timestamp of current MA	    
public:
	TMa() { Ma = 0; };	
    TMa(const PJsonVal& ParamVal) { TMa(); };
	TMa(TSIn& SIn);
	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecs, const int& N);	
	double GetMa() const { return Ma; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};
    
/////////////////////////////////////////////////
// Online Summa
class TSum {
private:
	TFlt Sum; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	    
public:
	TSum() { Sum = 0; };
	TSum(const PJsonVal& ParamVal) { TSum(); };
	TSum(TSIn& SIn) : Sum(SIn), TmMSecs(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	double GetSum() const { return Sum; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Sliding Window Min
class TMin {
private:
	TFlt Min; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	   
	TFltUInt64PrV AllValV; // sorted vector of values	
public:
	TMin() { Min = TFlt::Mx; TmMSecs = 0; };
	TMin(TSIn& SIn) : Min(SIn), TmMSecs(SIn), AllValV(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	double GetMin() const { return Min; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Sliding Window Max
class TMax {
private:
	TFlt Max; // current computed SUM value 
	TUInt64 TmMSecs; // timestamp of current MA	   
	TFltUInt64PrV AllValV; // sorted vector of values	
public:
	TMax() { Max = TFlt::Mn; TmMSecs = 0; };
	TMax(TSIn& SIn) : Max(SIn), TmMSecs(SIn), AllValV(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& InVal, const uint64& InTmMSecs,
		const TFltV& OutValV, const TUInt64V& OutTmMSecs);
	double GetMax() const { return Max; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Exponential Moving Average
typedef enum { etPreviousPoint, etLinear, etNextPoint } TEmaType;
class TEma {
private:
	// parameters
	TFlt Decay; // decaying factor
	TEmaType Type; // interpolation type
	// current state
	TFlt LastVal; // last input value
	TFlt Ema; // current computed EMA value 
	TUInt64 TmMSecs; // timestamp of current EMA
	double TmInterval; // time interval for definition of decay
	// buffer for initialization
	TBool InitP; // true if already initialized
	TUInt64 InitMinMSecs; // time window of requiered values for initialization
	TFltV InitValV; // first N values
	TUInt64V InitMSecsV; // weights of first N values
 	
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
	double GetEma() const { return Ema; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Online M2 (variance)
class TVar {
private:	
    TFlt Ma; // current computed MA value 
    TFlt M2; // current computed M2 value 
	TUInt64 TmMSecs; // timestamp of current WMA	
    TFlt pNo;
public:
	TVar() { Ma = 0; M2 = 0; pNo = 1; TmMSecs = 0;}	
	TVar(TSIn& SIn) : Ma(SIn), M2(SIn), TmMSecs(SIn), pNo(SIn) { }

	// serialization
	void Load(TSIn& SIn);
	void Save(TSOut& SOut) const;

	void Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N);
	// current status	
	double GetM2() const { return M2 / pNo; }
	uint64 GetTmMSecs() const { return TmMSecs; }
};

/////////////////////////////////////////////////
// Online M2(X,Y) (covariance)
class TCov {
private:	
    TFlt MaX, MaY; // current computed MA value        
    TFlt Cov;
	TUInt64 TmMSecs; // timestamp of current WMA	
    TFlt pNo;
	// used for saving and removing the "exact" values of the covariance
	// it uses more RAM, but is more exact
	TFltV Prod; TInt pLen; TInt idx;
public:
	TCov() {  };	
    TCov(const PJsonVal& ParamVal) { TCov(); };

	void Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV, const int& N);	
	double GetCov() const { 
		if (pNo > 1) { return Cov / (pNo - 1); } 
		else { return 0; }
	}
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
    /// Is buffer empty
    bool Empty() const { return ValV.Empty(); }
    /// Number of elements at the moment
    int Len() const { return ValV.Len(); }
        
	/// Get values from buffer (older values = larger index): example buffer = [x_(t-3) x_(t-2) x_(t-1) x_t] => GetVal(i) returns x_(t-1)
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
	while (!Empty()) { DelOldest(); }
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
	TBufferedInterpolator(TSIn& SIn);

public:
	virtual void Save(TSOut& SOut) const;

	virtual void SetNextInterpTm(const uint64& Time);
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
    // Save the model
    void Save(TSOut& SOut) const;

};

/////////////////////////////////////////
// Recursive Linear Regression
ClassTP(TRecLinReg, PRecLinReg)// {
private:
	TFlt ForgetFact;	// forgetting factor
	TFlt RegFact;		// regularization

	TFullMatrix P;		// correlation matrix
	TVector Coeffs;		// model

	// copy constructor
	TRecLinReg(const TRecLinReg& LinReg);
	// move constructor
	TRecLinReg(const TRecLinReg&& LinReg);
	// load constructor
	TRecLinReg(TSIn& SIn);
public:
	// default constructor
	TRecLinReg(const int& Dim, const double& _RegFact = 1.0, const double& _ForgetFact=1.0);
	// new method
	static PRecLinReg New(const int& Dim, const double& RegFact, const double& ForgetFact)
			{ return new TRecLinReg(Dim, RegFact, ForgetFact); }

	virtual ~TRecLinReg() {}

	void Save(TSOut& SOut) const;
	static PRecLinReg Load(TSIn& SIn);

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
};

}

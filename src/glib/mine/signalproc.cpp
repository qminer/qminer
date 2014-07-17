/**
 * GLib - General C++ Library
 * 
 * Copyright (C) 2014 Jozef Stefan Institute
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License, version 3,
 * as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 * 
 */

namespace TSignalProc {
    
/////////////////////////////////////////////////
// Online Moving Average 
void TMa::Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N){
    
    int tempN = N - 1 + OutValV.Len();
    double delta;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValV.Len(); ValN++)
    {        
        tempN--;       
        delta = OutValV[ValN] - Ma;
        Ma = Ma - delta/tempN;                 
    }
    //add the new value to the resulting mean    
    delta = InVal - Ma;
    Ma = Ma + delta/N;
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////////////
// Exponential Moving Average
double TEma::GetNi(const double& Alpha, const double& Mi) {
	switch (Type) {
	case etPreviousPoint: return 1.0;
	case etLinear: return (1 - Mi) / Alpha;
	case etNextPoint: return Mi;
	}
	throw TExcept::New("Unknown EMA interpolation type");
}

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const double& _Decay, const TEmaType& _Type, const uint64& _InitMinMSecs, 
    const double& _TmInterval): Decay(_Decay), Type(_Type), TmInterval(_TmInterval), 
    InitP(false), InitMinMSecs(_InitMinMSecs) { }

//TODO: compute InitMinMSecs initialization time window from decay factor
TEma::TEma(const TEmaType& _Type, const uint64& _InitMinMSecs,const double& _TmInterval):
	Type(_Type), TmInterval(_TmInterval), InitP(false), InitMinMSecs(_InitMinMSecs) { }

TEma::TEma(const PJsonVal& ParamVal): InitP(false) {
    // type
    TStr TypeStr = ParamVal->GetObjStr("emaType");
    if (TypeStr == "previous") { 
        Type = etPreviousPoint;
    } else if (TypeStr == "linear") {
        Type = etLinear;
    } else if (TypeStr == "next") {
        Type = etNextPoint;
    } else {
        throw TExcept::New("Unknown ema type " + TypeStr);
    }
    // rest
    TmInterval = ParamVal->GetObjNum("interval");
    InitMinMSecs = ParamVal->GetObjInt("initWindow");
}

void TEma::Update(const double& Val, const uint64& NewTmMSecs) {
	double TmInterval1;
	if(NewTmMSecs == TmMSecs) {
		TmInterval1 = 1.0;
    } else{
		TmInterval1 = (double)(NewTmMSecs - TmMSecs);
    }
	if (InitP) {
		// computer parameters for EMA
		double Alpha;
		if (Decay == 0.0) {			
			Alpha = TmInterval1 / TmInterval;
		} else {			
			Alpha = TmInterval1 / TmInterval  * (-1.0) * TMath::Log(Decay);
		}
		const double Mi = exp(-Alpha);
		const double Ni = GetNi(Alpha, Mi);
		// compute new ema
		Ema = Mi*Ema + (Ni - Mi)*LastVal + (1.0 - Ni)*Val;
	} else {
		// update buffers
		InitValV.Add(Val);
		InitMSecsV.Add(NewTmMSecs);
		// initialize when enough data
		const uint64 StartInitMSecs = InitMSecsV[0] + InitMinMSecs;
		if (StartInitMSecs < NewTmMSecs) {
			// Initialize using "buildup time interval",
			//TODO: check how interpolation type influences this code
			const int Vals = InitMSecsV.Len();	
			// compute weights for each value in buffer
			TFltV WeightV(Vals, 0);
			for (int ValN = 0; ValN < Vals; ValN++) {
				const double Alpha = (double)(TmInterval1) / Decay;
				WeightV.Add(exp(-Alpha));
			}
			// normalize weights so they sum to 1.0
			TLinAlg::NormalizeL1(WeightV);
			// compute initial value of EMA as weighted sum
			Ema = TLinAlg::DotProduct(WeightV, InitValV);
			// mark that we are done and clean up after us
			InitP = true; InitValV.Clr(); InitMSecsV.Clr();
		}
	}
	// update last value
	LastVal = Val;
	// update curret time
	TmMSecs = NewTmMSecs;
}

/////////////////////////////////////////////////
// Online Moving Standard M2 
void TVar::Update(const double& InVal, const uint64& InTmMSecs, 
        const TFltV& OutValV, const TUInt64V& OutTmMSecsV, const int& N){
    
    pNo = N;
    int tempN = N - 1 + OutValV.Len();
    double delta;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValV.Len(); ValN++)
    {        
        tempN--;       
        delta = OutValV[ValN] - Ma;
        Ma = Ma - delta/tempN;  
        M2 = M2 - delta * (OutValV[ValN] - Ma);
    }
    //add the new value to the resulting mean    
    delta = InVal - Ma;
    Ma = Ma + delta/N;
    M2 = M2 + delta * (InVal - Ma);
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////////////
// Online Moving Covariance (assumes X and Y have the same time stamp)
void TCov::Update(const double& InValX, const double& InValY, const uint64& InTmMSecs, 
        const TFltV& OutValVX, const TFltV& OutValVY, const TUInt64V& OutTmMSecsV, const int& N){
    
    pNo = N;
    int tempN = N - 1 + OutValVX.Len();
    double deltaX, deltaY;    
    // remove old values from the mean
    for (int ValN = 0; ValN < OutValVX.Len(); ValN++)
    {        
        tempN--;       
        deltaX = OutValVX[ValN] - MaX;
        deltaY = OutValVY[ValN] - MaY;
        MaX = MaX - deltaX/tempN;  
        MaY = MaY - deltaY/tempN;  
        Cov = Cov - (OutValVX[ValN] - MaX) * (OutValVY[ValN] - MaY);
    }
    //add the new value to the resulting mean    
    deltaX = InValX - MaX;
    deltaY = InValY - MaY;
    MaX = MaX + deltaX/N;
    MaY = MaY + deltaY/N;
    Cov = Cov + (InValX - MaX) * (InValY - MaY);
    TmMSecs = InTmMSecs;
}

/////////////////////////////////////////
// Time series interpolator interface
PInterpolator TInterpolator::New(const TStr& InterpolatorType) {
    if(InterpolatorType == TPreviousPoint::GetType()) {
		return TPreviousPoint::New();
	}    
	if(InterpolatorType == TLinear::GetType()) {
		return TLinear::New();
	}    
    throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

PInterpolator TInterpolator::Load(TSIn& SIn) {
	TStr InterpolatorType(SIn);
	if(InterpolatorType == TPreviousPoint::GetType()) {
		return TPreviousPoint::New(SIn);
	}
	if(InterpolatorType == TLinear::GetType()) {
		return TLinear::New(SIn);
	}
	throw TExcept::New("Unknown interpolator type " + InterpolatorType);
}

/////////////////////////////////////////
// Previous point interpolator.
// Interpolate by returning last seen value
bool TPreviousPoint::CanInterpolate(const uint64& Tm) const {
	return PreviousRec.Val2 <= Tm;
}

/////////////////////////////////////////
// Time series linear interpolator
double TLinear::Interpolate(const uint64& Tm) const {
	TTm TmTTm = TTm::GetTmFromMSecs(Tm);

	AssertR(CanInterpolate(Tm), "Time not in the desired interval!");

	if (PreviousRec.Val2 == NextRec.Val2) {
		return NextRec.Val1;
	}

	return PreviousRec.Val1+((double)(Tm-PreviousRec.Val2)/(NextRec.Val2-PreviousRec.Val2))*(NextRec.Val1-PreviousRec.Val1);
}

bool TLinear::CanInterpolate(const uint64& Tm) const {
	return PreviousRec.Val2 <= Tm && Tm <= NextRec.Val2;
}

void TLinear::Update(const double& Val, const uint64& Tm) {
	AssertR(Tm != NextRec.Val2 || Val == NextRec.Val1, "Points have the same time stamp but different value!");

	PreviousRec = NextRec;
	NextRec.Val1 = Val;
	NextRec.Val2 = Tm;
}

///////////////////////////////////////////////////////////////////
// Neural Networks - Neuron
TRnd TNNet::TNeuron::Rnd = 0;

TNNet::TNeuron::TNeuron(){
}

TNNet::TNeuron::TNeuron(TInt OutputsN, TInt MyId, TTFunc TransFunc){

    TFuncNm = TransFunc;
    for(int c = 0; c < OutputsN; ++c){
        // define the edges, 0 element is weight, 1 element is weight delta
        OutEdgeV.Add(TIntFltFltTr(c, RandomWeight(), 0.0));
    }

    Id = MyId;
}

void TNNet::TNeuron::FeedFwd(const TLayer& PrevLayer){
    TFlt SumIn = 0.0;
    // get the previous layer's outputs and sum them up
    for(int NeuronN = 0; NeuronN < PrevLayer.GetNeuronN(); ++NeuronN){
        SumIn += PrevLayer.GetOutVal(NeuronN) * 
                PrevLayer.GetWeight(NeuronN, Id);
    }
    // not sure if static. maybe different fcn for output nodes
    OutputVal = TNeuron::TransferFcn(SumIn);
}

TFlt TNNet::TNeuron::TransferFcn(TFlt Sum){
    switch (TFuncNm){
        case tanHyper:
            // tanh output range [-1.0..1.0]
            // training data should be scaled to what the transfer function can handle
           return tanh(Sum);
        case sigmoid:
           // sigmoid output range [0.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return 1.0 / (1.0 + exp(-Sum));
        case fastTanh:
           // sigmoid output range [-1.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return Sum / (1.0 + abs(Sum));
        case fastSigmoid:
           // sigmoid output range [0.0..1.0]
           // training data should be scaled to what the transfer function can handle
           return (Sum / 2.0) / (1.0 + abs(Sum)) + 0.5;
        case linear:
            return Sum;         
    };
    throw TExcept::New("Unknown transfer function type");
}

TFlt TNNet::TNeuron::TransferFcnDeriv(TFlt Sum){
    switch (TFuncNm){
        case tanHyper:
            // tanh derivative approximation
            return 1.0 - Sum * Sum;
        case sigmoid:{
           double Fun = 1.0 / (1.0 + exp(-Sum));
           return Fun * (1.0 - Fun);
        }
        case fastTanh:
           return 1.0 / ((1.0 + abs(Sum)) * (1.0 + abs(Sum)));
        case fastSigmoid:
           return 1.0 / (2.0 * (1.0 + abs(Sum)) * (1.0 + abs(Sum)));
        case linear:
            return 1;         
    };
    throw TExcept::New("Unknown transfer function type");
}

void TNNet::TNeuron::CalcOutGradient(TFlt TargVal){
    TFlt Delta = TargVal - OutputVal;
    // TODO: different ways of calculating gradients
    Gradient = Delta * TNeuron::TransferFcnDeriv(OutputVal);
}

void TNNet::TNeuron::CalcHiddenGradient(const TLayer& NextLayer){
    // calculate error by summing the derivatives of the weights next layer
    TFlt DerivsOfWeights = SumDOW(NextLayer);
    Gradient = DerivsOfWeights * TNeuron::TransferFcnDeriv(OutputVal);
}

TFlt TNNet::TNeuron::SumDOW(const TLayer& NextLayer) const{
    TFlt sum = 0.0;
    // sum our contributions of the errors at the nodes we feed
    for(int NeuronN = 0; NeuronN < NextLayer.GetNeuronN() - 1; ++NeuronN){
        // weight from us to next layer neuron times its gradient
        sum += GetWeight(NeuronN) * NextLayer.GetGradient(NeuronN);
    }

    return sum;
}

void TNNet::TNeuron::UpdateInputWeights(TLayer& PrevLayer, const TFlt& LearnRate, const TFlt& Momentum){
    // the weights to be updated are in the OutEdgeV
    // in the neurons in the preceding layer
    for(int NeuronN = 0; NeuronN < PrevLayer.GetNeuronN(); ++NeuronN){
        TNeuron& Neuron = PrevLayer.GetNeuron(NeuronN);
        TFlt OldDeltaWeight = Neuron.GetDeltaWeight(Id);
        //printf(" LearnRate N: %f \n", LearnRate);

        TFlt NewDeltaWeight = 
                // individual input magnified by the gradient and train rate
                LearnRate
                * Neuron.GetOutVal()
                * Gradient
                // add momentum = fraction of previous delta weight
                + Momentum
                * OldDeltaWeight;

        Neuron.SetDeltaWeight(Id, NewDeltaWeight);
        Neuron.UpdateWeight(Id, NewDeltaWeight);
    }
}
    
/////////////////////////////////////////////////////////////////////////
//// Neural Networks - Layer of Neurons
// TODO: why do we need this empty constructor?
TNNet::TLayer::TLayer(){
}

TNNet::TLayer::TLayer(const TInt& NeuronsN, const TInt& OutputsN, const TTFunc& TransFunc){
    // Add neurons to the layer, plus bias neuron
    for(int NeuronN = 0; NeuronN <= NeuronsN; ++NeuronN){
        NeuronV.Add(TNeuron(OutputsN, NeuronN, TransFunc));
        printf("Made a neuron!");
        printf(" Neuron N: %d", NeuronN);
        printf(" Neuron H: %d", NeuronV.Len());
        printf("\n");
    } 
    // Force the bias node's output value to 1.0
    NeuronV.Last().SetOutVal(1.0);
    printf("\n");
}

/////////////////////////////////////////////////////////////////////////
//// Neural Networks - Neural Net
TNNet::TNNet(const TIntV& LayoutV, const TFlt& _LearnRate, 
            const TFlt& _Momentum, const TTFunc& TFuncHiddenL,
            const TTFunc& TFuncOutL){
    // get number of layers
    int LayersN = LayoutV.Len();
    LearnRate = _LearnRate;
    Momentum = _Momentum;
    // create each layer
    for(int LayerN = 0; LayerN < LayersN; ++LayerN){
        // Get number of neurons in the next layer
        // set number of output connections, if output layer then no output connections
        TInt OutputsN = LayerN == LayersN - 1 ? TInt(0) : LayoutV[LayerN + 1];
        // set transfer functions for hidden and output layers
        TTFunc TransFunc = LayerN == LayersN - 1 ? TFuncOutL : TFuncHiddenL;
        TInt NeuronsN = LayoutV[LayerN];
        // Add a layer to the net
        LayerV.Add(TLayer(NeuronsN, OutputsN, TransFunc));
        printf("LayerV.Len(): %d \n", LayerV.Len() );
    }
}

void TNNet::FeedFwd(const TFltV& InValV){
    // check if number of input values same as number of input neurons
    Assert(InValV.Len() == LayerV[0].GetNeuronN() - 1);
    // assign input values to input neurons
    for(int InputN = 0; InputN < InValV.Len(); ++InputN){
        LayerV[0].SetOutVal(InputN, InValV[InputN]);
    }

    // forward propagation
    for(int LayerN = 1; LayerN < LayerV.Len(); ++LayerN){
        TLayer& PrevLayer = LayerV[LayerN - 1];
        for(int NeuronN = 0; NeuronN < LayerV[LayerN].GetNeuronN() - 1; ++NeuronN){
            LayerV[LayerN].FeedFwd(NeuronN, PrevLayer);
        }
    }
}

void TNNet::BackProp(const TFltV& TargValV){
    // calculate overall net error (RMS of output neuron errors)
    TLayer& OutputLayer = LayerV.Last();
    Error = 0.0;

    for(int NeuronN = 0; NeuronN < OutputLayer.GetNeuronN() - 1; ++NeuronN){
        TFlt Delta = TargValV[NeuronN] - OutputLayer.GetOutVal(NeuronN);
        Error += Delta * Delta;
    }
    Error /= OutputLayer.GetNeuronN() - 1;
    Error = sqrt(Error); // RMS

    // recent avg error measurement
    RecentAvgError = (RecentAvgError * RecentAvgSmoothingFactor + Error)
            / (RecentAvgSmoothingFactor + 1.0);
    // Calculate output layer gradients
    for(int NeuronN = 0; NeuronN < OutputLayer.GetNeuronN() - 1; ++NeuronN){
        OutputLayer.CalcOutGradient(NeuronN, TargValV[NeuronN]);
    }        
    // Calculate gradients on hidden layers
    for(int LayerN = LayerV.Len() - 2; LayerN > 0; --LayerN){
        TLayer& HiddenLayer = LayerV[LayerN];
        TLayer& NextLayer = LayerV[LayerN + 1];

        for(int NeuronN = 0; NeuronN < HiddenLayer.GetNeuronN() - 1; ++NeuronN){
            HiddenLayer.CalcHiddenGradient(NeuronN, NextLayer);
        }

    }
    // For all layers from output to first hidden layer
    // update connection weights
    for(int LayerN = LayerV.Len() - 1; LayerN > 0; --LayerN){
        TLayer& Layer = LayerV[LayerN];
        TLayer& PrevLayer = LayerV[LayerN - 1];

        for(int NeuronN = 0; NeuronN < Layer.GetNeuronN() - 1; ++NeuronN){
            Layer.UpdateInputWeights(NeuronN, PrevLayer, LearnRate, Momentum);
        }
    }
}

void TNNet::GetResults(TFltV& ResultV) const{
    ResultV.Clr(true, -1);

    for(int NeuronN = 0; NeuronN < LayerV.Last().GetNeuronN() - 1; ++NeuronN){
        ResultV.Add(LayerV.Last().GetOutVal(NeuronN));
    }
}

///////////////////////////////////////////////////////////////////
// Recursive Linear Regression
TRecLinReg::TRecLinReg(const TRecLinReg& LinReg):
		ForgetFact(LinReg.ForgetFact),
		RegFact(LinReg.RegFact),
		P(LinReg.P),
		Coeffs(LinReg.Coeffs) {}

TRecLinReg::TRecLinReg(const TRecLinReg&& LinReg):
		ForgetFact(std::move(LinReg.ForgetFact)),
		RegFact(std::move(LinReg.RegFact)),
		P(std::move(LinReg.P)),
		Coeffs(std::move(LinReg.Coeffs)) {}

TRecLinReg::TRecLinReg(TSIn& SIn):
		ForgetFact(SIn),
		RegFact(SIn) {
	P.Load(SIn);
	Coeffs.Load(SIn);
}

TRecLinReg::TRecLinReg(const int& Dim, const double& _RegFact,
			const double& _ForgetFact):
		ForgetFact(_ForgetFact),
		RegFact(_RegFact),
		P(TFullMatrix::Identity(Dim) / RegFact),
		Coeffs(Dim, true) {}

void TRecLinReg::Save(TSOut& SOut) const {
	ForgetFact.Save(SOut);
	RegFact.Save(SOut);
	P.Save(SOut);
	Coeffs.Save(SOut);
}

PRecLinReg TRecLinReg::Load(TSIn& SIn) {
	return new TRecLinReg(SIn);
}

TRecLinReg& TRecLinReg::operator =(TRecLinReg LinReg) {
	std::swap(ForgetFact, LinReg.ForgetFact);
	std::swap(P, LinReg.P);
	std::swap(Coeffs, LinReg.Coeffs);

	return *this;
}

double TRecLinReg::Predict(const TFltV& Sample) {
    return Coeffs.DotProduct(Sample);
}

void TRecLinReg::Learn(const TFltV& Sample, const double& SampleVal) {
    double PredVal = Predict(Sample);

    TVector x(Sample);

    TVector Px = P * x;
	double xPx = Px.DotProduct(Sample);

	/*
	 * linreg.P = (linreg.P - (Px * Px') / (linreg.lambda + xPx)) / linreg.lambda;
	 * linreg.w = linreg.w + Px*((y - y_hat)/(linreg.lambda + xPx));
	 */
	P = (P - (Px*Px.GetT()) / (ForgetFact + xPx)) / ForgetFact;
	Coeffs += Px*((SampleVal - PredVal) / (ForgetFact + xPx));
}

void TRecLinReg::GetCoeffs(TFltV& Coef) const {
	Coef = Coeffs.GetVec();
}

}

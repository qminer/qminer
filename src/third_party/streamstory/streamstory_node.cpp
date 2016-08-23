/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "streamstory_node.h"

////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
const double TNodeJsStreamStory::DEFAULT_DELTA_TM = 1e-6;

void TNodeJsStreamStory::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsStreamStory>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fitAsync", _fitAsync);
	NODE_SET_PROTOTYPE_METHOD(tpl, "update", _update);
	NODE_SET_PROTOTYPE_METHOD(tpl, "futureStates", _futureStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "pastStates", _pastStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predictNextState", _predictNextState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "probsAtTime", _probsAtTime);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histStates", _histStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getHistoricalStates", _getHistoricalStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "currState", _currState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fullCoords", _fullCoords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateCentroids", _getStateCentroids);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histogram", _histogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "transitionHistogram", _transitionHistogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "timeHistogram", _timeHistogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFtrBounds", _getFtrBounds);
	NODE_SET_PROTOTYPE_METHOD(tpl, "stateIds", _stateIds);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getWeights", _getWeights);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getClassifyTree", _getClassifyTree);
	NODE_SET_PROTOTYPE_METHOD(tpl, "explainState", _explainState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setActivity", _setActivity);
	NODE_SET_PROTOTYPE_METHOD(tpl, "removeActivity", _removeActivity);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getActivities", _getActivities);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getSubModelJson", _getSubModelJson);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStatePath", _getStatePath);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTransitionModel", _getTransitionModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onStateChanged", _onStateChanged);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAnomaly", _onAnomaly);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onOutlier", _onOutlier);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onProgress", _onProgress);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onPrediction", _onPrediction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onActivity", _onActivity);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHistograms", _rebuildHistograms);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateLabel", _getStateLabel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateAutoName", _getStateAutoName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "narrateState", _narrateState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateTypTimes", _getStateTypTimes);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateName", _getStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setStateName", _setStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setStateCoords", _setStateCoords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clearStateName", _clearStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isTarget", _isTarget);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setTarget", _setTarget);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isLeaf", _isLeaf);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimeUnit", _getTimeUnit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setControlVal", _setControlVal);
	NODE_SET_PROTOTYPE_METHOD(tpl, "resetControlVal", _resetControlVal);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isAnyControlFtrSet", _isAnyControlFtrSet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isActivityDetector", _isActivityDetector);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isPredictor", _isPredictor);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParam", _getParam);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}

TNodeJsStreamStory::TNodeJsStreamStory(TMc::TStreamStory* _StreamStory):
		StreamStory(_StreamStory),
		UvHandle(TNodeJsAsyncUtil::NewHandle()),
		ProgressSection(),
		ProgressQ() {
	InitCallbacks();
}

TNodeJsStreamStory::TNodeJsStreamStory(PSIn& SIn):
		StreamStory(new TMc::TStreamStory(*SIn)),
		UvHandle(TNodeJsAsyncUtil::NewHandle()),
		ProgressSection(),
		ProgressQ() {
	InitCallbacks();
}

TNodeJsStreamStory::~TNodeJsStreamStory() {
	StateChangedCallback.Reset();
	AnomalyCallback.Reset();
	OutlierCallback.Reset();
	ProgressCallback.Reset();
	PredictionCallback.Reset();
	ActivityCallback.Reset();

	TNodeJsAsyncUtil::DelHandle(UvHandle);

	delete StreamStory;
}

TNodeJsStreamStory* TNodeJsStreamStory::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	EAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
		// load from file
		PSIn SIn = TNodeJsUtil::IsArgStr(Args, 0) ?
				TFIn::New(TNodeJsUtil::GetArgStr(Args, 0)) :
				TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0)->SIn;

		return new TNodeJsStreamStory(SIn);
	} else {
		const PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		const int RndSeed = ParamVal->GetObjInt("rndseed", 0);
		const TRnd Rnd(RndSeed);

		const int NPastStates = ParamVal->IsObjKey("pastStates") ? ParamVal->GetObjInt("pastStates") : 0;
		const bool Verbose = ParamVal->IsObjKey("verbose") ? ParamVal->GetObjBool("verbose") : true;

		const PJsonVal ClustJson = ParamVal->GetObjKey("clustering");
		const PJsonVal TransitionJson = ParamVal->GetObjKey("transitions");
		const PJsonVal HierarchJson = ParamVal->GetObjKey("hierarchy");

		// clustering
		const double Sample = ClustJson->IsObjKey("sample") ? ClustJson->GetObjNum("sample") : 1;
		const int NHistBins = ClustJson->IsObjKey("histogramBins") ? ClustJson->GetObjInt("histogramBins") : 20;
		const bool IncludeTmFtrV = ClustJson->GetObjBool("includeTimeFeatures");

		// transition modelling
		const uint64 TimeUnit = GetTmUnit(TransitionJson->GetObjStr("timeUnit"));
		const double DeltaTm = TransitionJson->IsObjKey("deltaTime") ?
				TransitionJson->GetObjNum("deltaTime") : DEFAULT_DELTA_TM;

		// hierarchy
		const bool IsTransitionBased = HierarchJson->GetObjBool("isTransitionBased");

		TMc::TStateIdentifier* StateIdentifier = new TMc::TStateIdentifier(GetClust(ClustJson, Rnd), NHistBins, Sample, IncludeTmFtrV, Rnd, Verbose);
		TMc::TCtmcModeller* MChain = new TMc::TCtmcModeller(TimeUnit, DeltaTm, Verbose);
		TMc::THierarch* Hierarch = new TMc::THierarch(NPastStates + 1, IsTransitionBased, Rnd, Verbose);

		TMc::TStreamStory* StreamStory = new TMc::TStreamStory(
				StateIdentifier,
				MChain,
				Hierarch,
				Rnd,
				Verbose
		);

		// finish
		return new TNodeJsStreamStory(StreamStory);
	}
}

TNodeJsStreamStory::TFitTask::TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args):
		TNodeTask(Args),
		JsStreamStory(nullptr),
		JsObservFtrVV(nullptr),
		JsControlFtrVV(nullptr),
		JsIgnoredFtrVV(nullptr),
		JsRecTmV(nullptr),
		JsBatchEndJsV(nullptr) {

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() > 0, "ss.fitAsync expects at least one argument!");

	JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	v8::Local<v8::Object> ArgObj = Args[0]->ToObject();

	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "observations", TNodeJsFltVV::GetClassId()), "Missing field observations or invalid class!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "controls", TNodeJsFltVV::GetClassId()), "Missing field controls or invalid class!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "times", TNodeJsFltV::GetClassId()), "Missing field times or invalid class!");

	JsObservFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "observations");
	JsControlFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "controls");
	JsIgnoredFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "ignored");
	JsRecTmV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltV>(ArgObj, "times");

	// parse the attribute types
	ParseFtrInfo(TNodeJsUtil::GetFldJson(ArgObj, "ftrInfo"), ObsFtrInfo, ContrFtrInfo, IgnFtrInfo);

	if (!TNodeJsUtil::IsFldNull(ArgObj, "batchV")) {
		EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "batchV", TNodeJsBoolV::GetClassId()), "Invalid class of field batchV!");
		JsBatchEndJsV = TNodeJsUtil::GetUnwrapFld<TNodeJsBoolV>(ArgObj, "batchV");
	}
}

v8::Handle<v8::Function> TNodeJsStreamStory::TFitTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	return TNodeJsUtil::GetArgFun(Args, 1);
}

void TNodeJsStreamStory::TFitTask::Run() {
	try {
		TUInt64V RecTmV;	TNodeJsUtil::GetCppTmMSecsV(JsRecTmV->Vec, RecTmV);

		if (JsBatchEndJsV != nullptr) {
			const TBoolV& BatchEndV = JsBatchEndJsV->Vec;
			JsStreamStory->StreamStory->InitBatches(ObsFtrInfo, ContrFtrInfo, IgnFtrInfo, JsObservFtrVV->Mat, JsControlFtrVV->Mat, JsIgnoredFtrVV->Mat, RecTmV, BatchEndV);
		} else {
			JsStreamStory->StreamStory->Init(ObsFtrInfo, ContrFtrInfo, IgnFtrInfo, JsObservFtrVV->Mat, JsControlFtrVV->Mat, JsIgnoredFtrVV->Mat, RecTmV);
		}
	} catch (const PExcept& _Except) {
		SetExcept(_Except);
	}
}

TNodeJsStreamStory::TProgressTask::TProgressTask(TNodeJsStreamStory* _JsStreamStory):
		JsStreamStory(_JsStreamStory) {}

void TNodeJsStreamStory::TProgressTask::Run() {
	JsStreamStory->ProcessProgressQ();
}

void TNodeJsStreamStory::update(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	TNodeJsFltV* JsObsFtrV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
	TNodeJsFltV* JsContrFtrV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 1);
	const uint64 RecTm = TNodeJsUtil::GetArgTmMSecs(Args, 2);

	JsMChain->StreamStory->OnAddRec(RecTm, JsObsFtrV->Vec, JsContrFtrV->Vec);
	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::futureStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
	const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);

	TIntFltPrV StateIdProbPrV;

	if (Args.Length() > 2 && !Args[2]->IsNull() && !Args[2]->IsUndefined()) {
		const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);
		JsMChain->StreamStory->GetFutStateProbV(Level, StartState, Tm, StateIdProbPrV);
	}
	else {
		JsMChain->StreamStory->GetNextStateProbV(Level, StartState, StateIdProbPrV);
	}

	v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdProbPrV.Len());
	for (int i = 0; i < StateIdProbPrV.Len(); i++) {
		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

		StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdProbPrV[i].Val1));
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, StateIdProbPrV[i].Val2));

		StateArr->Set(i, StateObj);
	}

	Args.GetReturnValue().Set(StateArr);
}

void TNodeJsStreamStory::pastStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
	const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);

	TIntFltPrV StateIdProbPrV;

	if (Args.Length() > 2 && !Args[2]->IsNull() && !Args[2]->IsUndefined()) {
		const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);
		JsStreamStory->StreamStory->GetPastStateProbV(Level, StartState, Tm, StateIdProbPrV);
	}
	else {
		JsStreamStory->StreamStory->GetPrevStateProbV(Level, StartState, StateIdProbPrV);
	}

	v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdProbPrV.Len());
	for (int i = 0; i < StateIdProbPrV.Len(); i++) {
		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

		StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdProbPrV[i].Val1));
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, StateIdProbPrV[i].Val2));

		StateArr->Set(i, StateObj);
	}

	Args.GetReturnValue().Set(StateArr);
}

void TNodeJsStreamStory::predictNextState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	const bool UserFtrP = ParamVal->GetObjBool("useFtrV");
	const int NFutStates = ParamVal->GetObjInt("futureStateN");

	const uint64 LastRecTm = JsStreamStory->StreamStory->GetLastRecTm();
	TVec<TPair<TFlt, TIntFltPrV>> HeightStateIdProbPrVPrV;
	TIntFltPrV CurrStateIdHeightPrV;
	JsStreamStory->StreamStory->GetCurrStateAncestry(CurrStateIdHeightPrV);
	JsStreamStory->StreamStory->PredictNextState(UserFtrP, NFutStates, HeightStateIdProbPrVPrV);

	PJsonVal CurrStatesJsonV = TJsonVal::NewArr();
	for (int HeightN = 0; HeightN < CurrStateIdHeightPrV.Len(); HeightN++) {
		const TIntFltPr& StateIdHeightPr = CurrStateIdHeightPrV[HeightN];

		PJsonVal StateJson = TJsonVal::NewObj();
		StateJson->AddToObj("id", StateIdHeightPr.Val1);
		StateJson->AddToObj("height", StateIdHeightPr.Val2);

		CurrStatesJsonV->AddToArr(StateJson);
	}

	PJsonVal StatesJsonV = TJsonVal::NewArr();
	for (int HeightN = 0; HeightN < HeightStateIdProbPrVPrV.Len(); HeightN++) {
		const TPair<TFlt, TIntFltPrV>& HeightStateIdProbPrPr = HeightStateIdProbPrVPrV[HeightN];
		const TIntFltPrV& StateIdProbPrV = HeightStateIdProbPrPr.Val2;

		PJsonVal HeightJson = TJsonVal::NewObj();
		PJsonVal StateIdProbJsonV = TJsonVal::NewArr();

		for (int StateN = 0; StateN < StateIdProbPrV.Len(); StateN++) {
			const TIntFltPr& StateIdProbPr = StateIdProbPrV[StateN];

			PJsonVal StateIdProbJson = TJsonVal::NewObj();
			StateIdProbJson->AddToObj("id", StateIdProbPr.Val1);
			StateIdProbJson->AddToObj("prob", StateIdProbPr.Val2);

			StateIdProbJsonV->AddToArr(StateIdProbJson);
		}

		HeightJson->AddToObj("height", HeightStateIdProbPrPr.Val1);
		HeightJson->AddToObj("states", StateIdProbJsonV);

		StatesJsonV->AddToArr(HeightJson);
	}

	PJsonVal ResJson = TJsonVal::NewObj();
	ResJson->AddToObj("timestamp", TUInt64(TNodeJsUtil::GetJsTimestamp(LastRecTm)));
	ResJson->AddToObj("current", CurrStatesJsonV);
	ResJson->AddToObj("prediction", StatesJsonV);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ResJson));
}

void TNodeJsStreamStory::probsAtTime(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StartStateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const double Level = TNodeJsUtil::GetArgFlt(Args, 1);
	const double Time = TNodeJsUtil::GetArgFlt(Args, 2);

	TIntV StateIdV; TFltV ProbV;
	JsStreamStory->StreamStory->GetProbVAtTime(StartStateId, Level, Time, StateIdV, ProbV);

	v8::Local<v8::Array> Result = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		const int& StateId = StateIdV[i];
		const double Prob = ProbV[i];

		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "stateId"), v8::Integer::New(Isolate, StateId));
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, Prob));
		Result->Set(i, StateObj);
	}

	Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamStory::histStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

	TIntV StateIdV;	JsStreamStory->StreamStory->GetHistStateIdV(Level, StateIdV);

	v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		StateArr->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
	}

	Args.GetReturnValue().Set(StateArr);
}

void TNodeJsStreamStory::getHistoricalStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

    if (Args.Length() > 0 && !TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
        const double Scale = TNodeJsUtil::GetArgFlt(Args, 0);

        TUInt64IntPrV StateTmStateIdPrV;
        JsStreamStory->StreamStory->GetStateHistory(Scale, StateTmStateIdPrV);

        v8::Local<v8::Array> JsResult = v8::Array::New(Isolate, StateTmStateIdPrV.Len());
        for (int StateN = 0; StateN < StateTmStateIdPrV.Len(); StateN++) {
            v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

            StateObj->Set(v8::String::NewFromUtf8(Isolate, "start"), v8::Number::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(StateTmStateIdPrV[StateN].Val1)));
            StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateTmStateIdPrV[StateN].Val2));

            JsResult->Set(StateN, StateObj);
        }

        Args.GetReturnValue().Set(JsResult);
    }
    else {  // all scales
        TVec<TPair<TFlt, TUInt64IntPrV>> ScaleTmIdPrV;
        JsStreamStory->StreamStory->GetStateHistory(ScaleTmIdPrV);

        v8::Local<v8::Array> JsResult = v8::Array::New(Isolate, ScaleTmIdPrV.Len());
        for (int ScaleN = 0; ScaleN < ScaleTmIdPrV.Len(); ScaleN++) {
            v8::Local<v8::Object> ScaleObj = v8::Object::New(Isolate);

            const TPair<TFlt, TUInt64IntPrV>& ScaleTmIdVPr = ScaleTmIdPrV[ScaleN];
            const TUInt64IntPrV& StateTmStateIdPrV = ScaleTmIdVPr.Val2;

            v8::Local<v8::Array> JsScaleHist = v8::Array::New(Isolate, StateTmStateIdPrV.Len());
            for (int StateN = 0; StateN < StateTmStateIdPrV.Len(); StateN++) {
                v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

                StateObj->Set(v8::String::NewFromUtf8(Isolate, "start"), v8::Number::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(StateTmStateIdPrV[StateN].Val1)));
                StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateTmStateIdPrV[StateN].Val2));

                JsScaleHist->Set(StateN, StateObj);
            }

            ScaleObj->Set(v8::String::NewFromUtf8(Isolate, "scale"), v8::Number::New(Isolate, ScaleTmIdVPr.Val1));
            ScaleObj->Set(v8::String::NewFromUtf8(Isolate, "states"), JsScaleHist);

            JsResult->Set(ScaleN, ScaleObj);
        }

        Args.GetReturnValue().Set(JsResult);
    }
}

void TNodeJsStreamStory::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsStreamStory->StreamStory->GetJson()));
}

void TNodeJsStreamStory::getSubModelJson(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const PJsonVal ModelJson = JsStreamStory->StreamStory->GetSubModelJson(StateId);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ModelJson));
}

void TNodeJsStreamStory::getStatePath(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const double Height = TNodeJsUtil::GetArgFlt(Args, 1);
	const int Depth = TNodeJsUtil::GetArgInt32(Args, 2);
	const double TransThreshold = TNodeJsUtil::GetArgFlt(Args, 3, 0.2);

	const PJsonVal PathJson = JsStreamStory->StreamStory->GetLikelyPathTreeJson(StateId, Height, Depth, TransThreshold);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, PathJson));
}

void TNodeJsStreamStory::getTransitionModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

	TFltVV Mat;	JsMChain->StreamStory->GetTransitionModel(Level, Mat);

	PJsonVal MatJson = TJsonVal::NewArr();
	for (int i = 0; i < Mat.GetRows(); i++) {
		PJsonVal RowJson = TJsonVal::NewArr();

		for (int j = 0; j < Mat.GetCols(); j++) {
			RowJson->AddToArr(Mat(i,j));
		}

		MatJson->AddToArr(RowJson);
	}

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, MatJson));
}

void TNodeJsStreamStory::currState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (Args.Length() > 0 && !Args[0]->IsNull() && !Args[0]->IsUndefined()) {
		double Height = TNodeJsUtil::GetArgFlt(Args, 0);
		int CurrStateId = JsMChain->StreamStory->GetCurrStateId(Height);

		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, CurrStateId));

		Args.GetReturnValue().Set(StateObj);
	} else {
		TIntFltPrV StateIdHeightPrV;	JsMChain->StreamStory->GetCurrStateAncestry(StateIdHeightPrV);

		v8::Local<v8::Array> AncestryArr = v8::Array::New(Isolate, StateIdHeightPrV.Len());
		for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdHeightPrV[i].Val1));
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "height"), v8::Number::New(Isolate, StateIdHeightPrV[i].Val2));

			AncestryArr->Set(i, StateObj);
		}

		Args.GetReturnValue().Set(AncestryArr);
	}
}

void TNodeJsStreamStory::fullCoords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const int FtrSpaceN = TNodeJsUtil::GetArgInt32(Args, 1);

	TFltV FtrV;	JsStreamStory->StreamStory->GetCentroid(StateId, FtrSpaceN, FtrV);

	v8::Local<v8::Array> FtrVJson = v8::Array::New(Isolate, FtrV.Len());
	for (int i = 0; i < FtrV.Len(); i++) {
		FtrVJson->Set(i, v8::Number::New(Isolate, FtrV[i]));
	}

	Args.GetReturnValue().Set(FtrVJson);
}

void TNodeJsStreamStory::getStateCentroids(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0, -1);	// -1 for all the states

	TVec<TFltV> FtrVV;
	JsStreamStory->StreamStory->GetCentroidVV(StateId, FtrVV);

	v8::Local<v8::Array> FtrVVJson = v8::Array::New(Isolate, FtrVV.Len());
	for (int CentroidN = 0; CentroidN < FtrVV.Len(); CentroidN++) {
		const TFltV& FtrV = FtrVV[CentroidN];
		v8::Local<v8::Array> FtrVJson = v8::Array::New(Isolate, FtrV.Len());

		for (int FtrN = 0; FtrN < FtrV.Len(); FtrN++) {
			FtrVJson->Set(FtrN, v8::Number::New(Isolate, FtrV[FtrN]));
		}

		FtrVVJson->Set(CentroidN, FtrVJson);
	}

	Args.GetReturnValue().Set(FtrVVJson);
}

void TNodeJsStreamStory::histogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int FtrId = TNodeJsUtil::GetArgInt32(Args, 0);
	const int StateId = TNodeJsUtil::IsArgNullOrUndef(Args, 1) ? -1 : TNodeJsUtil::GetArgInt32(Args, 1);

	TFltV BinValV;
	TFltV CountV, AllCountV;
	JsStreamStory->StreamStory->GetHistogram(StateId, FtrId, BinValV, CountV, AllCountV);

	v8::Local<v8::Object> Result = WrapHistogram(BinValV, CountV, TFltV(), AllCountV);
	Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamStory::transitionHistogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 3, "ss.transitionHistogram: expects 3 arguments!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int SourceId = TNodeJsUtil::GetArgInt32(Args, 0);
	const int TargetId = TNodeJsUtil::GetArgInt32(Args, 1);
	const int FtrId = TNodeJsUtil::GetArgInt32(Args, 2);

	TFltV BinValV, SourceProbV, TargetProbV, AllProbV;

	JsStreamStory->StreamStory->GetTransitionHistogram(SourceId, TargetId, FtrId, BinValV, SourceProbV, TargetProbV, AllProbV);

	v8::Local<v8::Object> Result = WrapHistogram(BinValV, SourceProbV, TargetProbV, AllProbV);
	Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamStory::timeHistogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr HistTypeStr = TNodeJsUtil::GetArgStr(Args, 1);

	TFltV BinValV;
	TFltV ProbV;

	if (HistTypeStr == "global") {
		const int Bins = TNodeJsUtil::GetArgInt32(Args, 2, 100);
		TUInt64V TmV;
		JsStreamStory->StreamStory->GetGlobalTimeHistogram(StateId, TmV, ProbV, Bins);

		BinValV.Gen(TmV.Len());
		for (int BinN = 0; BinN < TmV.Len(); BinN++) {
			BinValV[BinN] = (double) TNodeJsUtil::GetJsTimestamp(TmV[BinN]);
		}
	} else {
		TIntV BinValIntV;

		if (HistTypeStr == "year") {
			JsStreamStory->StreamStory->GetTimeHistogram(StateId, TMc::TStateIdentifier::TTmHistType::thtYear, BinValIntV, ProbV);
		} else if (HistTypeStr == "month") {
			JsStreamStory->StreamStory->GetTimeHistogram(StateId, TMc::TStateIdentifier::TTmHistType::thtMonth, BinValIntV, ProbV);
		} else if (HistTypeStr == "week") {
			JsStreamStory->StreamStory->GetTimeHistogram(StateId, TMc::TStateIdentifier::TTmHistType::thtWeek, BinValIntV, ProbV);
		} else if (HistTypeStr == "day") {
			JsStreamStory->StreamStory->GetTimeHistogram(StateId, TMc::TStateIdentifier::TTmHistType::thtDay, BinValIntV, ProbV);
		} else {
			throw TExcept::New("Unknown time histogram type: " + HistTypeStr);
		}

		BinValV.Gen(BinValIntV.Len());
		for (int BinN = 0; BinN < BinValIntV.Len(); BinN++) {
			BinValV[BinN] = BinValIntV[BinN];
		}
	}

	v8::Local<v8::Object> Result = WrapHistogram(BinValV, ProbV, TFltV(), TFltV());
	Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamStory::getFtrBounds(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.getFtrBounds: expects 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int FtrId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TFltPr FtrBounds = JsStreamStory->StreamStory->GetFtrBounds(FtrId);

	v8::Local<v8::Object> Result = v8::Object::New(Isolate);
	Result->Set(v8::String::NewFromUtf8(Isolate, "min"), v8::Number::New(Isolate, FtrBounds.Val1.Val));
	Result->Set(v8::String::NewFromUtf8(Isolate, "max"), v8::Number::New(Isolate, FtrBounds.Val2.Val));

	Args.GetReturnValue().Set(Result);
}

void TNodeJsStreamStory::stateIds(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Height = TNodeJsUtil::GetArgFlt(Args, 0);

	TIntV StateIdV;
	JsMChain->StreamStory->GetStateIdVAtHeight(Height, StateIdV);

	v8::Local<v8::Array> StateIdJsV = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdJsV->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
	}

	Args.GetReturnValue().Set(StateIdJsV);
}

void TNodeJsStreamStory::getWeights(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const PJsonVal WgtJson = JsMChain->StreamStory->GetStateWgtV(StateId);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, WgtJson));
}

void TNodeJsStreamStory::getClassifyTree(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const PJsonVal TreeJson = JsStreamStory->StreamStory->GetStateClassifyTree(StateId);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, TreeJson));
}

void TNodeJsStreamStory::explainState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const PJsonVal ExplainJson = JsStreamStory->StreamStory->GetStateExplain(StateId);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ExplainJson));
}

void TNodeJsStreamStory::setActivity(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const TStr ActName = TNodeJsUtil::GetArgStr(Args, 0);
	TVec<TIntV> StateIdSeqVV;	TNodeJsUtil::GetArgIntVV(Args, 1, StateIdSeqVV);

	JsStreamStory->StreamStory->AddActivity(ActName, StateIdSeqVV);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::removeActivity(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const TStr ActNm = TNodeJsUtil::GetArgStr(Args, 0);

	JsStreamStory->StreamStory->RemoveActivity(ActNm);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::getActivities(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	TStrV ActNmV;
	TIntV ActStepsV;
	JsStreamStory->StreamStory->GetActivities(ActNmV, ActStepsV);

	v8::Local<v8::String> JsNmStr = v8::String::NewFromUtf8(Isolate, "name");
	v8::Local<v8::String> JsStepsStr = v8::String::NewFromUtf8(Isolate, "steps");

	v8::Local<v8::Array> JsActV = v8::Array::New(Isolate, ActNmV.Len());
	for (int ActN = 0; ActN < ActNmV.Len(); ActN++) {
		v8::Local<v8::Object> JsAct = v8::Object::New(Isolate);

		JsAct->Set(JsNmStr, v8::String::NewFromUtf8(Isolate, ActNmV[ActN].CStr()));
		JsAct->Set(JsStepsStr, v8::Integer::New(Isolate, ActStepsV[ActN]));

		JsActV->Set(ActN, JsAct);
	}

	Args.GetReturnValue().Set(JsActV);
}

void TNodeJsStreamStory::onStateChanged(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->StateChangedCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->StateChangedCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::onAnomaly(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->AnomalyCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onAnomaly: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->AnomalyCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::onOutlier(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->OutlierCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onOutlier: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->OutlierCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::onProgress(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->ProgressCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onProgress: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->ProgressCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::onPrediction(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->PredictionCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onPrediction: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->PredictionCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::onActivity(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	if (TNodeJsUtil::IsArgNullOrUndef(Args, 0)) {
		JsStreamStory->ActivityCallback.Reset();
	} else {
		EAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onPrediction: First argument expected to be a function!");
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);
		JsStreamStory->ActivityCallback.Reset(Isolate, Callback);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::rebuildHistograms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.rebuildHistograms: expects 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	v8::Local<v8::Object> ArgObj = Args[0]->ToObject();

	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "observations", TNodeJsFltVV::GetClassId()), "Missing field observations!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "controls", TNodeJsFltVV::GetClassId()), "Missing field controls!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "times", TNodeJsFltV::GetClassId()), "Missing field times!");

	TNodeJsFltVV* JsObsFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "observations");
	TNodeJsFltVV* JsControlFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "controls");
	TNodeJsFltVV* JsIgnoredFtrVV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "ignored");
	TNodeJsFltV* JsRecTmV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltV>(ArgObj, "times");

	TUInt64V RecTmV(JsRecTmV->Vec.Len(), 0);
	for (int i = 0; i < JsRecTmV->Vec.Len(); i++) {
		RecTmV.Add(TNodeJsUtil::GetCppTimestamp((uint64)JsRecTmV->Vec[i]));
	}

	if (TNodeJsUtil::IsObjFld(ArgObj, "batchV")) {
		EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "batchV", TNodeJsBoolV::GetClassId()), "Invalid class of field batchV!");
		const TNodeJsBoolV* BatchEndJsV = TNodeJsUtil::GetUnwrapFld<TNodeJsBoolV>(ArgObj, "batchV");
		const TBoolV& BatchEndV = BatchEndJsV->Vec;
		JsStreamStory->StreamStory->InitHistograms(JsObsFtrVV->Mat, JsControlFtrVV->Mat, JsIgnoredFtrVV->Mat, RecTmV, BatchEndV);
	} else {
		JsStreamStory->StreamStory->InitHistograms(JsObsFtrVV->Mat, JsControlFtrVV->Mat, JsIgnoredFtrVV->Mat, RecTmV, TBoolV());
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::getStateLabel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "ss.getStateName: expects 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr& StateNm = JsStreamStory->StreamStory->GetStateLabel(StateId);

	Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, StateNm.CStr()));
}

void TNodeJsStreamStory::getStateAutoName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TMc::TUiHelper::PAutoNmDesc& StateAutoNm = JsStreamStory->StreamStory->GetStateAutoNm(StateId);
	const PJsonVal AutoNmJson = StateAutoNm->GetJson();

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, AutoNmJson));
}

void TNodeJsStreamStory::narrateState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	TVec<TMc::TUiHelper::PAutoNmDesc> StateFtrDescV;
	JsStreamStory->StreamStory->GetStateFtrPValDesc(StateId, StateFtrDescV);

	PJsonVal Result = TJsonVal::NewArr();
	for (int DescN = 0; DescN < StateFtrDescV.Len(); DescN++) {
		const TMc::TUiHelper::PAutoNmDesc& FtrDesc = StateFtrDescV[DescN];
		Result->AddToArr(FtrDesc->GetNarrateJson());
	}

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Result));
}

void TNodeJsStreamStory::getStateTypTimes(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	TStrPrV StateTimeV;
	JsStreamStory->StreamStory->GetStateTmDesc(StateId, StateTimeV);

	PJsonVal TimeJsonV = TJsonVal::NewArr();
	for (int i = 0; i < StateTimeV.Len(); i++) {
		const TStrPr& StartEndStrPr = StateTimeV[i];

		PJsonVal IntervalJson = TJsonVal::NewObj();
		IntervalJson->AddToObj("start", StartEndStrPr.Val1);
		IntervalJson->AddToObj("end", StartEndStrPr.Val2);

		TimeJsonV->AddToArr(IntervalJson);
	}

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, TimeJsonV));
}

void TNodeJsStreamStory::getStateName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "ss.getStateName: expects 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr& StateNm = JsStreamStory->StreamStory->GetStateNm(StateId);

	Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, StateNm.CStr()));
}

void TNodeJsStreamStory::setStateName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "ss.setStateName: expects 2 arguments!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr StateNm = TNodeJsUtil::GetArgStr(Args, 1);

	JsMChain->StreamStory->SetStateNm(StateId, StateNm);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::setStateCoords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const PJsonVal PosJsonArr = TNodeJsUtil::GetArgJson(Args, 0);

	TFltPrV PosV(JsStreamStory->StreamStory->GetStates());
	for (int i = 0; i < PosJsonArr->GetArrVals(); i++) {
		const PJsonVal& StateJson = PosJsonArr->GetArrVal(i);
		const PJsonVal& PosJson = StateJson->GetObjKey("position");

		const int StateId = StateJson->GetObjInt("id");
		const double x = PosJson->GetObjNum("x");
		const double y = PosJson->GetObjNum("y");

		PosV[StateId] = TFltPr(x, y);
	}

	JsStreamStory->StreamStory->SetStatePosV(PosV);
	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::clearStateName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "ss.clearStateName: expects 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	JsStreamStory->StreamStory->SetStateNm(StateId, "");

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::isTarget(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	bool Result = JsStreamStory->StreamStory->IsTargetState(StateId);
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, Result));
}

void TNodeJsStreamStory::setTarget(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const bool IsTarget = TNodeJsUtil::GetArgBool(Args, 1);

	JsStreamStory->StreamStory->SetTargetState(StateId, IsTarget);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::isLeaf(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStreamStory->StreamStory->IsLeaf(StateId)));
}

void TNodeJsStreamStory::getTimeUnit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const uint64 TimeUnit = JsStreamStory->StreamStory->GetTimeUnit();

	if (TimeUnit == TMc::TCtmcModeller::TU_SECOND) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "second"));
	} else if (TimeUnit == TMc::TCtmcModeller::TU_MINUTE) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "minute"));
	} else if (TimeUnit == TMc::TCtmcModeller::TU_HOUR) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "hour"));
	} else if (TimeUnit == TMc::TCtmcModeller::TU_DAY) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "day"));
	} else if (TimeUnit == TMc::TCtmcModeller::TU_MONTH) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "month"));
	} else {
		throw TExcept::New("Invalid time unit!", "TNodeJsStreamStory::getTimeUnit");
	}
}



void TNodeJsStreamStory::setControlVal(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "TNodeJsStreamStory::setControlVal: expecting 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	v8::Local<v8::Object> ArgsObj = Args[0]->ToObject();

	const int FtrId = TNodeJsUtil::GetFldInt(ArgsObj, "ftrId");
	const double Val = TNodeJsUtil::GetFldFlt(ArgsObj, "val");

	if (TNodeJsUtil::IsFldInt(ArgsObj, "stateId")) {
		const int StateId = TNodeJsUtil::GetFldInt(ArgsObj, "stateId");
		JsStreamStory->StreamStory->SetControlFtrVal(StateId, FtrId, Val);
	} else {
		JsStreamStory->StreamStory->SetControlFtrsVal(FtrId, Val);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::resetControlVal(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "TNodeJsStreamStory::setControlVal: expecting 1 argument!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	v8::Local<v8::Object> ArgsObj = Args[0]->ToObject();

	if (TNodeJsUtil::IsFldInt(ArgsObj, "stateId")) {
		const int StateId = TNodeJsUtil::GetFldInt(ArgsObj, "stateId");
		if (TNodeJsUtil::IsFldInt(ArgsObj, "ftrId")) {
			const int FtrId = TNodeJsUtil::GetFldInt(ArgsObj, "ftrId");
			JsStreamStory->StreamStory->ResetControlFtrVal(StateId, FtrId);
		} else {
			JsStreamStory->StreamStory->ResetControlFtrVals(StateId);
		}
	} else {
		JsStreamStory->StreamStory->ResetControlFtrVals();
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsStreamStory::isAnyControlFtrSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStreamStory->StreamStory->IsAnyControlFtrSet()));
}

void TNodeJsStreamStory::isActivityDetector(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStreamStory->StreamStory->IsDetectingActivities()));
}

void TNodeJsStreamStory::isPredictor(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsStreamStory->StreamStory->IsPredictingStates()));
}

void TNodeJsStreamStory::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() > 0, "hmc.setParams: Expects one argument!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

	JsMChain->SetParams(ParamVal);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsStreamStory::getParam(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.getParam: Expects one argument!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	const TStr ParamNm = TNodeJsUtil::GetArgStr(Args, 0);

	if (ParamNm == "verbose") {
		Args.GetReturnValue().Set(v8::Boolean::New(Isolate, JsMChain->StreamStory->IsVerbose()));
		return;
	} else if (ParamNm == "predictionThreshold") {
		Args.GetReturnValue().Set(v8::Number::New(Isolate, JsMChain->StreamStory->GetPredictionThreshold()));
		return;
	} else if (ParamNm == "timeHorizon") {
		Args.GetReturnValue().Set(v8::Number::New(Isolate, JsMChain->StreamStory->GetTimeHorizon()));
		return;
	} else if (ParamNm == "pdfBins") {
		Args.GetReturnValue().Set(v8::Integer::New(Isolate, JsMChain->StreamStory->GetPdfBins()));
		return;
	}
}

void TNodeJsStreamStory::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Should have 1 argument!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	PSOut SOut;
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
	} else {
		TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
		SOut = JsFOut->SOut;
	}

	JsMChain->StreamStory->Save(*SOut);

	// we return nothing currently, just close the stream if filename was used
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut.Clr();
	} else {
		// return output stream for convenience
		Args.GetReturnValue().Set(Args[0]);
	}
}

void TNodeJsStreamStory::OnStateChanged(const uint64 Tm, const TIntFltPrV& StateIdHeightV) {
	if (!StateChangedCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Value> JsTm = v8::Date::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(Tm));
		v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdHeightV.Len());

		for (int i = 0; i < StateIdHeightV.Len(); i++) {
			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdHeightV[i].Val1));
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "height"), v8::Number::New(Isolate, StateIdHeightV[i].Val2));

			StateArr->Set(i, StateObj);
		}

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, StateChangedCallback);
		TNodeJsUtil::ExecuteVoid(Callback, JsTm, StateArr);
	}
}

void TNodeJsStreamStory::OnAnomaly(const TStr& AnomalyDesc) {
	if (!AnomalyCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::String> Param = v8::String::NewFromUtf8(Isolate, AnomalyDesc.CStr());

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, AnomalyCallback);
		TNodeJsUtil::ExecuteVoid(Callback, Param);
	}
}

void TNodeJsStreamStory::OnOutlier(const TFltV& FtrV) {
	if (!OutlierCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OutlierCallback);
		TNodeJsUtil::ExecuteVoid(Callback, TNodeJsFltV::New(FtrV));
	}
}

void TNodeJsStreamStory::OnProgress(const int& Perc, const TStr& Msg) {
	{
		TLock Lock(ProgressSection);
		ProgressQ.Add(TIntStrPr(Perc, Msg));
	}

	TNodeJsAsyncUtil::ExecuteOnMain(new TProgressTask(this), UvHandle, true);
}

void TNodeJsStreamStory::OnPrediction(const uint64& RecTm, const int& CurrStateId, const int& TargetStateId,
		const double& Prob, const TFltV& ProbV, const TFltV& TmV) {
	if (!PredictionCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Array> JsProbV = v8::Array::New(Isolate, ProbV.Len());
		v8::Local<v8::Array> JsTmV = v8::Array::New(Isolate, TmV.Len());

		for (int i = 0; i < ProbV.Len(); i++) {
			JsProbV->Set(i, v8::Number::New(Isolate, ProbV[i]));
		}
		for (int i = 0; i < TmV.Len(); i++) {
			JsTmV->Set(i, v8::Number::New(Isolate, TmV[i]));
		}

		const int ArgC = 6;

		v8::Handle<v8::Value> ArgV[ArgC] = {
			v8::Date::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(RecTm)),
			v8::Integer::New(Isolate, CurrStateId),
			v8::Integer::New(Isolate, TargetStateId),
			v8::Number::New(Isolate, Prob),
			JsProbV,
			JsTmV
		};

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, PredictionCallback);
		TNodeJsUtil::ExecuteVoid(Callback, ArgC, ArgV);
	}
}

void TNodeJsStreamStory::OnActivityDetected(const uint64& StartTm, const uint64& EndTm, const TStr& ActNm) {
	if (!ActivityCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		const int ArgC = 3;
		v8::Handle<v8::Value> ArgV[ArgC] = {
			v8::Date::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(StartTm)),
			v8::Date::New(Isolate, (double) TNodeJsUtil::GetJsTimestamp(EndTm)),
			v8::String::NewFromUtf8(Isolate, ActNm.CStr())
		};

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, ActivityCallback);
		TNodeJsUtil::ExecuteVoid(Callback, ArgC, ArgV);
	}
}

void TNodeJsStreamStory::ProcessProgressQ() {
	TIntStrPrV TempProgressQ;

	{
		TLock Lock(ProgressSection);

		TempProgressQ = ProgressQ;
		ProgressQ.Clr();
	}

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (ProgressCallback.IsEmpty() || TempProgressQ.Empty()) { return; }

	v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, ProgressCallback);

	for (int ProgressN = 0; ProgressN < TempProgressQ.Len(); ProgressN++) {
		const TIntStrPr& PercMsgPr = TempProgressQ[ProgressN];

		v8::Local<v8::Integer> JsPerc = v8::Integer::New(Isolate, PercMsgPr.Val1.Val);
		v8::Local<v8::String> JsMsg = v8::String::NewFromUtf8(Isolate, PercMsgPr.Val2.CStr());

		TNodeJsUtil::ExecuteVoid(Callback, JsPerc->ToObject(), JsMsg->ToObject());
	}
}

void TNodeJsStreamStory::SetParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("verbose"))
		StreamStory->SetVerbose(ParamVal->GetObjBool("verbose"));
	if (ParamVal->IsObjKey("predictionThreshold"))
		StreamStory->SetPredictionThreshold(ParamVal->GetObjNum("predictionThreshold"));
	if (ParamVal->IsObjKey("timeHorizon"))
		StreamStory->SetTimeHorizon(ParamVal->GetObjNum("timeHorizon"));
	if (ParamVal->IsObjKey("pdfBins"))
		StreamStory->SetPdfBins(ParamVal->GetObjInt("pdfBins"));
}

void TNodeJsStreamStory::InitCallbacks() {
	StreamStory->SetCallback(this);
}

v8::Local<v8::Object> TNodeJsStreamStory::WrapHistogram(const TFltV& BinValV,
		const TFltV& SourceProbV, const TFltV& TargetProbV, const TFltV& AllProbV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Result = v8::Object::New(Isolate);
	v8::Local<v8::Array> BinStartJsV = v8::Array::New(Isolate, BinValV.Len());
	v8::Local<v8::Array> ProbJsV = v8::Array::New(Isolate, SourceProbV.Len());

	double TotalCount = 0;

	for (int i = 0; i < BinValV.Len(); i++) {
		BinStartJsV->Set(i, v8::Number::New(Isolate, BinValV[i]));
	}

	for (int i = 0; i < SourceProbV.Len(); i++) {
		ProbJsV->Set(i, v8::Number::New(Isolate, SourceProbV[i]));
		TotalCount += SourceProbV[i];
	}

	Result->Set(v8::String::NewFromUtf8(Isolate, "binValV"), BinStartJsV);
	Result->Set(v8::String::NewFromUtf8(Isolate, "countV"), ProbJsV);
	Result->Set(v8::String::NewFromUtf8(Isolate, "totalCount"), v8::Number::New(Isolate, TotalCount));

	if (!TargetProbV.Empty()) {
		v8::Local<v8::Array> TargetProbJsV = v8::Array::New(Isolate, TargetProbV.Len());
		double TargetProbSum = 0;
		for (int i = 0; i < TargetProbV.Len(); i++) {
			TargetProbJsV->Set(i, v8::Number::New(Isolate, TargetProbV[i]));
			TargetProbSum += TargetProbV[i];
		}
		Result->Set(v8::String::NewFromUtf8(Isolate, "targetCountV"), TargetProbJsV);
		Result->Set(v8::String::NewFromUtf8(Isolate, "targetTotalCount"), v8::Number::New(Isolate, TargetProbSum));
	}

	if (!AllProbV.Empty()) {
		v8::Local<v8::Array> AllProbJsV = v8::Array::New(Isolate, AllProbV.Len());
		for (int i = 0; i < AllProbV.Len(); i++) {
			AllProbJsV->Set(i, v8::Number::New(Isolate, AllProbV[i]));
		}
		Result->Set(v8::String::NewFromUtf8(Isolate, "allCountV"), AllProbJsV);
	}

	return HandleScope.Escape(Result);
}

uint64 TNodeJsStreamStory::GetTmUnit(const TStr& TimeUnitStr) {
	if (TimeUnitStr == "second") {
		return TMc::TCtmcModeller::TU_SECOND;
	} else if (TimeUnitStr == "minute") {
		return TMc::TCtmcModeller::TU_MINUTE;
	} else if (TimeUnitStr == "hour") {
		return TMc::TCtmcModeller::TU_HOUR;
	} else if (TimeUnitStr == "day") {
		return TMc::TCtmcModeller::TU_DAY;
	} else if (TimeUnitStr == "month") {
		return TMc::TCtmcModeller::TU_MONTH;
	} else {
		throw TExcept::New("Invalid time unit: " + TimeUnitStr, "TNodeJsStreamStory::GetTmUnit");
	}
}

TClustering::TAbsKMeans<TFltVV>* TNodeJsStreamStory::GetClust(const PJsonVal& ParamJson,
		const TRnd& Rnd) {
	const TStr& ClustAlg = ParamJson->GetObjStr("type");
	if (ClustAlg == "dpmeans") {
		const double Lambda = ParamJson->GetObjNum("lambda");
		const int MinClusts = ParamJson->IsObjKey("minClusts") ? ParamJson->GetObjInt("minClusts") : 1;
		const int MxClusts = ParamJson->IsObjKey("maxClusts") ? ParamJson->GetObjInt("maxClusts") : TInt::Mx;

		return new TClustering::TDpMeans<TFltVV>(Lambda, MinClusts, MxClusts, Rnd);
	} else if (ClustAlg == "kmeans") {
		const int K = ParamJson->GetObjInt("k");
		return new TClustering::TDnsKMeans<TFltVV>(K, Rnd);
	} else {
		throw TExcept::New("Invalid clustering type: " + ClustAlg, "TJsHierCtmc::TJsHierCtmc");
	}
}

void TNodeJsStreamStory::ParseFtrInfo(const PJsonVal& InfoJson, TMc::TFtrInfoV& ObsFtrInfoV,
		TMc::TFtrInfoV& ContrFtrInfoV, TMc::TFtrInfoV& IgnFtrInfo) {
	PJsonVal ObsAttrInfoJson = InfoJson->GetObjKey("observation");
	PJsonVal ContrAttrInfoJson = InfoJson->GetObjKey("control");
	PJsonVal IgnAttrInfoJson = InfoJson->GetObjKey("ignored");

	ParseFtrInfo(ObsAttrInfoJson, ObsFtrInfoV);
	ParseFtrInfo(ContrAttrInfoJson, ContrFtrInfoV);
	ParseFtrInfo(IgnAttrInfoJson, IgnFtrInfo);
}

void TNodeJsStreamStory::ParseFtrInfo(const PJsonVal& InfoJsonV, TMc::TFtrInfoV& FtrInfoV) {
	EAssertR(InfoJsonV->IsArr(), "Attribute info is not an array!");

	for (int FtrN = 0; FtrN < InfoJsonV->GetArrVals(); FtrN++) {
		const PJsonVal& InfoJson = InfoJsonV->GetArrVal(FtrN);
		const TStr& Type = InfoJson->GetObjStr("type");

		if (Type == "numeric") {
			FtrInfoV.Add(TMc::TFtrInfo(TMc::ftNumeric, InfoJson->GetObjInt("offset"), InfoJson->GetObjInt("length")));
		}
		else if (Type == "nominal") {
			FtrInfoV.Add(TMc::TFtrInfo(TMc::ftCategorical, InfoJson->GetObjInt("offset"), InfoJson->GetObjInt("length")));
		}
		else {
			throw TExcept::New("Only numeric features currently supported!");
		}
	}
}

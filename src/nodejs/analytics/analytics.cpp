#include "analytics.h"

//////////////////////////////////////////////////////
// NodeJS - analytics

////////////////////////////////////////////////////////
// Support Vector Machine
v8::Persistent<v8::Function> TNodeJsSvmModel::constructor;

TNodeJsSvmModel::TNodeJsSvmModel(const PJsonVal& ParamVal):
		Algorithm("SGD"),
		SvmCost(1.0),
		SvmUnbalance(1.0),
		SampleSize(1000),
		MxIter(10000),
		MxTime(1000*600),
		MnDiff(1e-6),
		Verbose(false),
		Notify(TNotify::NullNotify),
		Model(nullptr) {

	UpdateParams(ParamVal);
}

TNodeJsSvmModel::TNodeJsSvmModel(TSIn& SIn):
	Algorithm(SIn),
	SvmCost(TFlt(SIn)),
	SvmUnbalance(TFlt(SIn)),
	SampleSize(TInt(SIn)),
	MxIter(TInt(SIn)),
	MxTime(TInt(SIn)),
	MnDiff(TFlt(SIn)),
	Verbose(TBool(SIn)),
	Notify(TNotify::NullNotify),
	Model(nullptr) {

	bool IsInit = TBool(SIn);

	if (Verbose) { Notify = TNotify::StdNotify; }
	if (IsInit) { Model = new TSvm::TLinModel(SIn); }
}

TNodeJsSvmModel::~TNodeJsSvmModel() {
	ClrModel();
}

v8::Local<v8::Object> TNodeJsSvmModel::WrapInst(v8::Local<v8::Object> Obj, const PJsonVal& ParamVal) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsSvmModel(ParamVal));
}

v8::Local<v8::Object> TNodeJsSvmModel::WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsSvmModel(SIn));
}

void TNodeJsSvmModel::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SVC"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "SVC"),
			   tpl->GetFunction());
#endif
}

void TNodeJsSvmModel::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
    
	QmAssertR(Args.IsConstructCall(), "SVC: not a constructor call!");
	QmAssertR(Args.Length() > 0, "SVC: missing arguments!");

	try {
		if (Args[0]->IsExternal()) {
			// load the model from an input stream
			TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
			Args.GetReturnValue().Set(TNodeJsSvmModel::WrapInst(Args.This(), *JsFIn->SIn));
		} else {
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
			Args.GetReturnValue().Set(TNodeJsSvmModel::WrapInst(Args.This(), ParamVal));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsSvmModel::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
	QmAssertR(Args[1]->IsObject(), "trainSvmClassify: second argument expected to be object");

	try {

		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		Model->ClrModel();

		TFltV& ClsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject())->Vec;
		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpMat::ClassId)) {
			TVec<TIntFltKdV>& VecV = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Model->Model = new TSvm::TLinModel(TSvm::SolveClassify<TVec<TIntFltKdV>>(VecV, TLAMisc::GetMaxDimIdx(VecV) + 1,
										VecV.Len(), ClsV, Model->SvmCost, Model->SvmUnbalance, Model->MxTime,
										Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify));
			} else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefSparseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmCost, Model->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));

				Model->Model = new TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)) {
			TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Model->Model = new TSvm::TLinModel(TSvm::SolveClassify<TFltVV>(VecV, VecV.GetRows(),
						VecV.GetCols(), ClsV, Model->SvmCost, Model->SvmUnbalance, Model->MxTime,
						Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify));
			} else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmCost, Model->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));


				Model->Model = new TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
		}
		Args.GetReturnValue().Set(Args.Holder());

	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsSvmModel::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "svm.predict: missing argument");

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		QmAssertR(Model->Model != nullptr, "svm.predict: SVM not initialized");

		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {
			TNodeJsVec<TFlt, TAuxFltV>* Vec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args[0]->ToObject());
			const double Res = Model->Model->Predict(Vec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpVec::ClassId)) {
			TNodeJsSpVec* SpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
			const double Res = Model->Model->Predict(SpVec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else {
			throw TQm::TQmExcept::New("svm.predict: unsupported type of the first argument");
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsSvmModel::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Model->GetParams()));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getParams");
	}
}

void TNodeJsSvmModel::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "svm.setParams: takes 1 argument!");
	QmAssertR(TNodeJsUtil::IsArgJson(Args, 0), "svm.setParams: first argument should je a Javascript object!");

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		Model->UpdateParams(ParamVal);

		Args.GetReturnValue().Set(Args.Holder());
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getParams");
	}
}

void TNodeJsSvmModel::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Info.Holder());

		// get feature vector
		TFltV WgtV; Model->Model->GetWgtV(WgtV);

		Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::weights");
	}
}

void TNodeJsSvmModel::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		QmAssertR(Args.Length() == 1, "Should have 1 argument!");
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		PSOut SOut = JsFOut->SOut;

		Model->Save(*SOut);

		Args.GetReturnValue().Set(Args[0]);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::save");
	}
}

void TNodeJsSvmModel::UpdateParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("algorithm")) Algorithm = ParamVal->GetObjStr("algorithm");
	if (ParamVal->IsObjKey("c")) SvmCost = ParamVal->GetObjNum("c");
	if (ParamVal->IsObjKey("j")) SvmUnbalance = ParamVal->GetObjNum("j");
	if (ParamVal->IsObjKey("batchSize")) SampleSize = ParamVal->GetObjInt("batchSize");
	if (ParamVal->IsObjKey("maxIterations")) MxIter = ParamVal->GetObjInt("maxIterations");
	if (ParamVal->IsObjKey("maxTime")) MxTime = 1000*ParamVal->GetObjInt("maxTime");
	if (ParamVal->IsObjKey("minDiff")) MnDiff = ParamVal->GetObjNum("minDiff");
	if (ParamVal->IsObjKey("verbose")) {
		Verbose = ParamVal->GetObjBool("verbose");
		Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;
	}
}

PJsonVal TNodeJsSvmModel::GetParams() const {
	PJsonVal ParamVal = TJsonVal::NewObj();

	ParamVal->AddToObj("algorithm", Algorithm);
	ParamVal->AddToObj("c", SvmCost);
	ParamVal->AddToObj("j", SvmUnbalance);
	ParamVal->AddToObj("batchSize", SampleSize);
	ParamVal->AddToObj("maxIterations", MxIter);
	ParamVal->AddToObj("maxTime", MxTime);
	ParamVal->AddToObj("minDiff", MnDiff);
	ParamVal->AddToObj("verbose", Verbose);

	return ParamVal;
}

void TNodeJsSvmModel::Save(TSOut& SOut) const {
	TBool IsInit = Model != nullptr;

	Algorithm.Save(SOut);
	TFlt(SvmCost).Save(SOut);
	TFlt(SvmUnbalance).Save(SOut);
	TInt(SampleSize).Save(SOut);
	TInt(MxIter).Save(SOut);
	TInt(MxTime).Save(SOut);
	TFlt(MnDiff).Save(SOut);
	TBool(Verbose).Save(SOut);
	IsInit.Save(SOut);

	if (IsInit) { Model->Save(SOut); }
}

void TNodeJsSvmModel::ClrModel() {
	if (Model != nullptr) {
		delete Model;
		Model = nullptr;
	}
}

////////////////////////////////////////////////
// QMiner-NodeJS-Recursive-Linear-Regression
v8::Persistent<v8::Function> TNodeJsRecLinReg::constructor;

TNodeJsRecLinReg::TNodeJsRecLinReg(const TSignalProc::PRecLinReg& _Model):
		Model(_Model) {}

v8::Local<v8::Object> TNodeJsRecLinReg::WrapInst(const v8::Local<v8::Object> Obj, const TSignalProc::PRecLinReg& Model) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsRecLinReg(Model));
}

//v8::Local<v8::Object> TNodeJsRecLinReg::New(const TSignalProc::PRecLinReg& Model) {
//	return TNodeJsUtil::NewJsInstance(new TNodeJsRecLinReg(Model), constructor, v8::Isolate::GetCurrent());
//}

void TNodeJsRecLinReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "RecLinReg"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "learn", _learn);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "RecLinReg"),
			   tpl->GetFunction());
#endif
}

void TNodeJsRecLinReg::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.IsConstructCall(), "TNodeJsRecLinReg: not a constructor call!");

	try {
		QmAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

		if (Args[0]->IsExternal()) {
			TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
			Args.GetReturnValue().Set(TNodeJsRecLinReg::WrapInst(Args.This(), TSignalProc::TRecLinReg::Load(*JsFIn->SIn)));
		}
		else {
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

			const int Dim = ParamVal->GetObjInt("dim");
			const double RegFact = ParamVal->GetObjNum("regFact", 1.0);
			const double ForgetFact = ParamVal->GetObjNum("forgetFact", 1.0);

			Args.GetReturnValue().Set(TNodeJsRecLinReg::WrapInst(Args.This(), TSignalProc::TRecLinReg::New(Dim, RegFact, ForgetFact)));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsRecLinReg::learn(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)");
	QmAssertR(TNodeJsUtil::IsArgFlt(Args, 1), "Argument 1 should be float!");

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
		TNodeJsFltV* JsFeatVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
		const double Target = TNodeJsUtil::GetArgFlt(Args, 1);

		// make sure dimensions match
		QmAssertR(Model->Model->GetDim() == JsFeatVec->Vec.Len(),
			"RecLinRegModel.learn: model dimension != passed argument dimension");

		// learn
		Model->Model->Learn(JsFeatVec->Vec, Target);
		QmAssertR(!Model->Model->HasNaN(), "RecLinRegModel.learn: NaN detected!");

		Args.GetReturnValue().Set(Args.Holder());
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsRecLinReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)");

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
		TNodeJsFltV* JsFeatVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

		QmAssertR(Model->Model->GetDim() == JsFeatVec->Vec.Len(),
		        "RecLinRegModel.learn: model dimension != sample dimension");

		Args.GetReturnValue().Set(v8::Number::New(Isolate, Model->Model->Predict(JsFeatVec->Vec)));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsRecLinReg::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Model->GetParams()));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getParams");
	}
}

void TNodeJsRecLinReg::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Info.Holder());

		// get feature vector
		TFltV Coef;	Model->Model->GetCoeffs(Coef);

		Info.GetReturnValue().Set(TNodeJsFltV::New(Coef));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsRecLinReg::dim(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Info.Holder());
		Info.GetReturnValue().Set(v8::Integer::New(Isolate, Model->Model->GetDim()));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsRecLinReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());

		QmAssertR(Args.Length() == 1, "Should have 1 argument!");
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		Model->Model.Save(*JsFOut->SOut);

		Args.GetReturnValue().Set(Args[0]);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

PJsonVal TNodeJsRecLinReg::GetParams() const {
	PJsonVal ParamVal = TJsonVal::NewObj();

	ParamVal->AddToObj("dim", Model->GetDim());
	ParamVal->AddToObj("regFact", Model->GetRegFact());
	ParamVal->AddToObj("forgetFact", Model->GetForgetFact());

	return ParamVal;
}


////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
const double TNodeJsHMChain::DEFAULT_DELTA_TM = 1e-3;

v8::Persistent<v8::Function> TNodeJsHMChain::constructor;

TNodeJsHMChain::TNodeJsHMChain(const TMc::PHierarchCtmc& _McModel):
		McModel(_McModel) {
	InitCallbacks();
}

TNodeJsHMChain::TNodeJsHMChain(PSIn& SIn):
		McModel(new TMc::THierarchCtmc(*SIn)) {
	InitCallbacks();
}

TNodeJsHMChain::~TNodeJsHMChain() {
	StateChangedCallback.Reset();
	AnomalyCallback.Reset();
	OutlierCallback.Reset();
}

v8::Local<v8::Object> TNodeJsHMChain::WrapInst(const v8::Local<v8::Object> Obj, const PJsonVal& ParamVal) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	const int NPastStates = ParamVal->IsObjKey("pastStates") ? ParamVal->GetObjInt("pastStates") : 0;
	const bool Verbose = ParamVal->IsObjKey("verbose") ? ParamVal->GetObjBool("verbose") : true;

	const PJsonVal TransitionJson = ParamVal->GetObjKey("transitions");
	const PJsonVal ClustJson = ParamVal->GetObjKey("clustering");

	// transition modelling
	TMc::PMChain MChain;
	if (TransitionJson->GetObjStr("type") == "continuous") {
		const TStr TimeUnitStr = TransitionJson->GetObjStr("timeUnit");
		const double DeltaTm = TransitionJson->IsObjKey("deltaTime") ?
				TransitionJson->GetObjNum("deltaTime") : DEFAULT_DELTA_TM;

		uint64 TimeUnit;
		if (TimeUnitStr == "second") {
			TimeUnit = TMc::TCtMChain::TU_SECOND;
		} else if (TimeUnitStr == "minute") {
			TimeUnit = TMc::TCtMChain::TU_MINUTE;
		} else if (TimeUnitStr == "hour") {
			TimeUnit = TMc::TCtMChain::TU_HOUR;
		} else if (TimeUnitStr == "day") {
			TimeUnit = TMc::TCtMChain::TU_DAY;
		} else {
			throw TExcept::New("Invalid time unit: " + TimeUnitStr, "TJsHierCtmc::TJsHierCtmc");
		}

		MChain = new TMc::TCtMChain(TimeUnit, DeltaTm, Verbose);
	} else if (TransitionJson->GetObjStr("type") == "discrete") {
		MChain = new TMc::TDtMChain(Verbose);
	}

	// clustering
	TMc::PClust Clust = NULL;

	const TStr ClustAlg = ClustJson->GetObjStr("type");
	const double Sample = ClustJson->IsObjKey("sample") ? ClustJson->GetObjNum("sample") : 1;
	const int NHistBins = ClustJson->IsObjKey("histogramBins") ? ClustJson->GetObjInt("histogramBins") : 20;

	if (ClustAlg == "dpmeans") {
		const double Lambda = ClustJson->GetObjNum("lambda");
		const int MinClusts = ClustJson->IsObjKey("minClusts") ? ClustJson->GetObjInt("minClusts") : 1;
		const int MxClusts = ClustJson->IsObjKey("maxClusts") ? ClustJson->GetObjInt("maxClusts") : TInt::Mx;
		const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
		Clust = new TMc::TDpMeans(NHistBins, Sample, Lambda, MinClusts, MxClusts, TRnd(RndSeed), Verbose);
	} else if (ClustAlg == "kmeans") {
		const int K = ClustJson->GetObjInt("k");
		const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
		Clust = new TMc::TFullKMeans(NHistBins, Sample, K, TRnd(RndSeed), Verbose);
	} else {
		throw TExcept::New("Invalivalid clustering type: " + ClustAlg, "TJsHierCtmc::TJsHierCtmc");
	}

	// create the model
	TMc::PHierarch AggClust = new TMc::THierarch(NPastStates + 1, Verbose);

	// finish
	TMc::PHierarchCtmc HMcModel = new TMc::THierarchCtmc(Clust, MChain, AggClust, Verbose);

	TNodeJsHMChain* Result = new TNodeJsHMChain(HMcModel);
	return TNodeJsUtil::WrapJsInstance(Obj, Result);
}

v8::Local<v8::Object> TNodeJsHMChain::WrapInst(const v8::Local<v8::Object> Obj, PSIn& SIn) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsHMChain(SIn));
}

void TNodeJsHMChain::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "HMC"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "update", _update);
	NODE_SET_PROTOTYPE_METHOD(tpl, "futureStates", _futureStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "pastStates", _pastStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histStates", _histStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "currState", _currState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fullCoords", _fullCoords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histogram", _histogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTransitionModel", _getTransitionModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onStateChanged", _onStateChanged);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAnomaly", _onAnomaly);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onOutlier", _onOutlier);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHierarchy", _rebuildHierarchy);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHistograms", _rebuildHistograms);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "HMC"),
			   tpl->GetFunction());
#endif
}

void TNodeJsHMChain::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Constructor expects 1 argument!");
	QmAssertR(Args.IsConstructCall(), "TNodeJsHMChain::New: Not a constructor call!");

	try {
		if (TNodeJsUtil::IsArgJson(Args, 0)) {
			PJsonVal ArgJson = TNodeJsUtil::GetArgJson(Args, 0);
			Args.GetReturnValue().Set(TNodeJsHMChain::WrapInst(Args.This(), ArgJson));
		} else {
			// load from file
			PSIn SIn = TNodeJsUtil::IsArgStr(Args, 0) ?
					TFIn::New(TNodeJsUtil::GetArgStr(Args, 0)) :
					ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject())->SIn;

			Args.GetReturnValue().Set(TNodeJsHMChain::WrapInst(Args.This(), SIn));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::New");
	}
}

void TNodeJsHMChain::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		TNodeJsFltVV* JsInstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TNodeJsFltV* JsRecTmV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

		TUInt64V RecTmV(JsRecTmV->Vec.Len(), 0);
		for (int64 i = 0; i < JsRecTmV->Vec.Len(); i++) {
			RecTmV.Add(TNodeJsUtil::GetCppTimestamp(JsRecTmV->Vec[i]));
		}

		JsMChain->McModel->Init(JsInstanceMat->Mat, RecTmV);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsHMChain::update(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		TNodeJsFltV* JsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

		uint64 RecTm;
		if (Args[1]->IsDate()) {
			// TODO
            RecTm = 0;
		} else {
			// Args[1] is a timestamp (UNIX timestamp)
			RecTm = TTm::GetWinMSecsFromUnixMSecs(TNodeJsUtil::GetArgFlt(Args, 1));
		}

		JsMChain->McModel->OnAddRec(RecTm, JsFtrV->Vec);
		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsHMChain::futureStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
		const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);

		TIntFltPrV StateIdProbPrV;

		if (Args.Length() > 2 && !Args[2]->IsNull() && !Args[2]->IsUndefined()) {
			const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);
			JsMChain->McModel->GetFutStateProbV(Level, StartState, Tm, StateIdProbPrV);
		}
		else {
			JsMChain->McModel->GetNextStateProbV(Level, StartState, StateIdProbPrV);
		}

		v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdProbPrV.Len());
		for (int i = 0; i < StateIdProbPrV.Len(); i++) {
			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdProbPrV[i].Val1));
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, StateIdProbPrV[i].Val2));

			StateArr->Set(i, StateObj);
		}

		Args.GetReturnValue().Set(StateArr);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::futureStates");
	}
}

void TNodeJsHMChain::pastStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
		const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);

		TIntFltPrV StateIdProbPrV;

		if (Args.Length() > 2 && !Args[2]->IsNull() && !Args[2]->IsUndefined()) {
			const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);
			JsMChain->McModel->GetPastStateProbV(Level, StartState, Tm, StateIdProbPrV);
		}
		else {
			JsMChain->McModel->GetPrevStateProbV(Level, StartState, StateIdProbPrV);
		}

		v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdProbPrV.Len());
		for (int i = 0; i < StateIdProbPrV.Len(); i++) {
			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdProbPrV[i].Val1));
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, StateIdProbPrV[i].Val2));

			StateArr->Set(i, StateObj);
		}

		Args.GetReturnValue().Set(StateArr);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::futureStates");
	}
}

void TNodeJsHMChain::histStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

		TIntV StateIdV;	JsMChain->McModel->GetHistStateIdV(Level, StateIdV);

		v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdV.Len());
		for (int i = 0; i < StateIdV.Len(); i++) {
			StateArr->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
		}

		Args.GetReturnValue().Set(StateArr);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::futureStates");
	}
}

void TNodeJsHMChain::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsMChain->McModel->SaveJson()));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsHMChain::getTransitionModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

		TFltVV Mat;	JsMChain->McModel->GetTransitionModel(Level, Mat);

		PJsonVal MatJson = TJsonVal::NewArr();
		for (int i = 0; i < Mat.GetRows(); i++) {
			PJsonVal RowJson = TJsonVal::NewArr();

			for (int j = 0; j < Mat.GetCols(); j++) {
				RowJson->AddToArr(Mat(i,j));
			}

			MatJson->AddToArr(RowJson);
		}

		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, MatJson));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::currState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		if (Args.Length() > 0 && !Args[0]->IsNull() && !Args[0]->IsUndefined()) {
			double Height = TNodeJsUtil::GetArgFlt(Args, 0);
			int CurrStateId = JsMChain->McModel->GetCurrStateId(Height);

			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, CurrStateId));

			Args.GetReturnValue().Set(StateObj);
		} else {
			TIntFltPrV StateIdHeightPrV;	JsMChain->McModel->GetCurrStateAncestry(StateIdHeightPrV);

			v8::Local<v8::Array> AncestryArr = v8::Array::New(Isolate, StateIdHeightPrV.Len());
			for (int i = 0; i < StateIdHeightPrV.Len(); i++) {
				v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

				StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdHeightPrV[i].Val1));
				StateObj->Set(v8::String::NewFromUtf8(Isolate, "height"), v8::Number::New(Isolate, StateIdHeightPrV[i].Val2));

				AncestryArr->Set(i, StateObj);
			}

			Args.GetReturnValue().Set(AncestryArr);
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::fullCoords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

		TFltV FtrV;	JsMChain->McModel->GetCentroid(StateId, FtrV);

		v8::Local<v8::Array> FtrVJson = v8::Array::New(Isolate, FtrV.Len());
		for (int i = 0; i < FtrV.Len(); i++) {
			FtrVJson->Set(i, v8::Number::New(Isolate, FtrV[i]));
		}

		Args.GetReturnValue().Set(FtrVJson);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::histogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "hmc.histogram: expects 2 arguments!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
		const int FtrId = TNodeJsUtil::GetArgInt32(Args, 1);

		TFltV BinStartV, ProbV;
		JsMChain->McModel->GetHistogram(StateId, FtrId, BinStartV, ProbV);

		v8::Local<v8::Object> Result = v8::Object::New(Isolate);
		v8::Local<v8::Array> BinStartJsV = v8::Array::New(Isolate, BinStartV.Len());
		v8::Local<v8::Array> ProbJsV = v8::Array::New(Isolate, ProbV.Len());

		for (int i = 0; i < BinStartV.Len(); i++) {
			BinStartJsV->Set(i, v8::Number::New(Isolate, BinStartV[i]));
		}

		for (int i = 0; i < ProbV.Len(); i++) {
			ProbJsV->Set(i, v8::Number::New(Isolate, ProbV[i]));
		}

		Result->Set(v8::String::NewFromUtf8(Isolate, "binStartV"), BinStartJsV);
		Result->Set(v8::String::NewFromUtf8(Isolate, "probs"), ProbJsV);

		Args.GetReturnValue().Set(Result);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::onStateChanged(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

		JsMChain->StateChangedCallback.Reset(Isolate, Callback);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::onAnomaly(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

		JsMChain->AnomalyCallback.Reset(Isolate, Callback);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::onOutlier(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

		JsMChain->OutlierCallback.Reset(Isolate, Callback);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::rebuildHierarchy(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		JsMChain->McModel->InitHierarch();

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::rebuildHistograms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.rebuildHistograms: expects 1 argument!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());

		JsMChain->McModel->InitHistograms(JsFltVV->Mat);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "hmc.setParams: Expects one argument!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		JsMChain->SetParams(ParamVal);

		Args.GetReturnValue().Set(Args.Holder());
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::getTransitionModel");
	}
}

void TNodeJsHMChain::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		PSOut SOut;
		if (TNodeJsUtil::IsArgStr(Args, 0)) {
			SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
		} else {
			TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
			SOut = JsFOut->SOut;
		}

		JsMChain->McModel->Save(*SOut);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::save");
	}
}

void TNodeJsHMChain::OnStateChanged(const TIntFltPrV& StateIdHeightV) {
	if (!StateChangedCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdHeightV.Len());
		for (int i = 0; i < StateIdHeightV.Len(); i++) {
			v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);

			StateObj->Set(v8::String::NewFromUtf8(Isolate, "id"), v8::Integer::New(Isolate, StateIdHeightV[i].Val1));
			StateObj->Set(v8::String::NewFromUtf8(Isolate, "height"), v8::Number::New(Isolate, StateIdHeightV[i].Val2));

			StateArr->Set(i, StateObj);
		}

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, StateChangedCallback);
		TNodeJsUtil::ExecuteVoid(Callback, StateArr);
	}
}

void TNodeJsHMChain::OnAnomaly(const TStr& AnomalyDesc) {
	if (!AnomalyCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::String> Param = v8::String::NewFromUtf8(Isolate, AnomalyDesc.CStr());

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, AnomalyCallback);
		TNodeJsUtil::ExecuteVoid(Callback, Param);
	}
}

void TNodeJsHMChain::OnOutlier(const TFltV& FtrV) {
	if (!OutlierCallback.IsEmpty()) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);

		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::New(Isolate, OutlierCallback);
		TNodeJsUtil::ExecuteVoid(Callback, TNodeJsFltV::New(FtrV));
	}
}

void TNodeJsHMChain::SetParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("verbose")) {
		McModel->SetVerbose(ParamVal->GetObjBool("verbose"));
	}
}

void TNodeJsHMChain::InitCallbacks() {
	McModel->SetCallback(this);
}
////////////////////////////////////////////////////////
// Neural Network model
v8::Persistent<v8::Function> TNodeJsNNet::constructor;

TNodeJsNNet::TNodeJsNNet(const PJsonVal& ParamVal) {
	TIntV LayoutV; // kako naj initiram tuki nek placeholder Vector?
	double LearnRate;
	double Momentum;
	TSignalProc::TTFunc TFuncHiddenL;
	TSignalProc::TTFunc TFuncOutL;

	if (ParamVal->IsObjKey("layout")) {
		ParamVal->GetObjIntV("layout", LayoutV);
	}
	else {
		LayoutV.Gen(3);
		LayoutV[0] = 1;
		LayoutV[1] = 2;
		LayoutV[2] = 1;
	}
	LearnRate = ParamVal->GetObjNum("learnRate", 0.1);
	Momentum = ParamVal->GetObjNum("momentum", 0.5);
	TFuncHiddenL = ExtractFuncFromString(ParamVal->GetObjStr("tFuncHidden", "tanHyper"));
	TFuncOutL = ExtractFuncFromString(ParamVal->GetObjStr("tFuncOut", "tanHyper"));

	try {
		Model = TSignalProc::TNNet::New(LayoutV, LearnRate, Momentum, TFuncHiddenL, TFuncOutL);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}

}

TNodeJsNNet::TNodeJsNNet(TSIn& SIn) {
	try {
		Model = TSignalProc::TNNet::Load(SIn);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}

}

v8::Local<v8::Object> TNodeJsNNet::WrapInst(v8::Local<v8::Object> Obj, const PJsonVal& ParamVal) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsNNet(ParamVal));
}

v8::Local<v8::Object> TNodeJsNNet::WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn) {
	return TNodeJsUtil::WrapJsInstance(Obj, new TNodeJsNNet(SIn));
}

void TNodeJsNNet::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	try {
		v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
		tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "NNet"));
		// ObjectWrap uses the first internal field to store the wrapped pointer.
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		// Add all methods, getters and setters here.
		NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
		NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
		NODE_SET_PROTOTYPE_METHOD(tpl, "setLearnRate", _setLearnRate);
		NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
		//NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

		// properties
		//tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
		//tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);

		constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "NNet"),
			   tpl->GetFunction());
#endif
	} catch(const PExcept& Except) {
		Isolate->ThrowException(
			v8::Exception::TypeError(
				v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())
			)
		);

	}
}

void TNodeJsNNet::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//QmAssertR(Args.IsConstructCall(), "SVC: not a constructor call!");
	//QmAssertR(Args.Length() > 0, "SVC: missing arguments!");

	try {
		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFIn::ClassId)) { // preverjanje, če je input stream
			// load the model from an input stream
			// currently not used, will be implemented
			TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
			Args.GetReturnValue().Set(TNodeJsNNet::WrapInst(Args.This(), *JsFIn->SIn));
		} else { // dobil JSON
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
			Args.GetReturnValue().Set(TNodeJsNNet::WrapInst(Args.This(), ParamVal));
		}
	} catch (const PExcept& Except) {
		Isolate->ThrowException(
			v8::Exception::TypeError(
				v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())
			)
		);
	}
}

void TNodeJsNNet::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 2, "NNet.fit: missing argument");

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());
		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {

			TNodeJsFltV* JsVecIn = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
			TNodeJsFltV* JsVecTarget = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

			// TODO: do some checking of dimensions etc..
	        // first get output values
			Model->Model->FeedFwd(JsVecIn->Vec);

	        // then check how we performed and learn
	        Model->Model->BackProp(JsVecTarget->Vec);
		}
	    else if(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)){
			TNodeJsFltVV* JsVVecIn = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
			TNodeJsFltVV* JsVVecTarget = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[1]->ToObject());
	        for(int Row = 0; Row < JsVVecIn->Mat.GetRows(); Row++){
	            TFltV InFltV;
	            JsVVecIn->Mat.GetRow(Row, InFltV);
	            Model->Model->FeedFwd(InFltV);
	            // then check how we performed and learn
	            TFltV TargFltV;
	            JsVVecTarget->Mat.GetRow(Row, TargFltV);
	            if(Row == JsVVecIn->Mat.GetRows() - 1){
	                Model->Model->BackProp(TargFltV);
	            }
	            else {
	                Model->Model->BackProp(TargFltV, false);
	            }
	        }
	    }
	    else {
	    	// TODO: throw an error
	        printf("NeuralNetwork.learn: The arguments must be a JsTFltV or JsTFltVV (js linalg full vector or matrix)");
	    }
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsNNet::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "NNet.predict: missing argument");

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());

		QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()),
				"NNet.predict: The first argument must be a JsTFltV (js linalg full vector)");
		TNodeJsFltV* JsVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

		Model->Model->FeedFwd(JsVec->Vec);

		TFltV FltV;
	    Model->Model->GetResults(FltV);

	    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltV));

	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsNNet::setLearnRate(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());
	TFlt NewLearnRate = TNodeJsUtil::GetArgFlt(Args, 0);

	Model->Model->SetLearnRate(NewLearnRate);

	//printf("&&Set learnrate to: %f \n", (double)NewLearnRate);
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsNNet::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());

		QmAssertR(Args.Length() == 1, "Should have 1 argument!");
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		PSOut SOut = JsFOut->SOut;

		Model->Model->Save(*SOut);

		Args.GetReturnValue().Set(Args[0]);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsNNet::save");
	}
}


TSignalProc::TTFunc TNodeJsNNet::ExtractFuncFromString(const TStr& FuncString) {
	TSignalProc::TTFunc TFunc;

	if (FuncString == "tanHyper") {
		TFunc = TSignalProc::TTFunc::tanHyper;
    } else if (FuncString == "sigmoid") {
    	TFunc = TSignalProc::TTFunc::sigmoid;
    } else if (FuncString == "fastTanh") {
    	TFunc = TSignalProc::TTFunc::fastTanh;
    } else if (FuncString == "softPlus") {
    	TFunc = TSignalProc::TTFunc::softPlus;
    } else if (FuncString == "fastSigmoid") {
    	TFunc = TSignalProc::TTFunc::fastSigmoid;
    } else if (FuncString == "linear") {
    	TFunc = TSignalProc::TTFunc::linear;
    } else {
        throw TExcept::New("Unknown transfer function type " + FuncString);
    }

	return TFunc;
}

///////////////////////////////
// QMiner-JavaScript-Tokenizer
v8::Persistent <v8::Function> TNodeJsTokenizer::constructor;

void TNodeJsTokenizer::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "Tokenizer"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getTokens", _getTokens);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getSentences", _getSentences);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParagraphs", _getParagraphs);

	constructor.Reset(Isolate, tpl->GetFunction());
	exports->Set(v8::String::NewFromUtf8(Isolate, "TTokenizer"), tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsTokenizer::New(const PTokenizer& Tokenizer) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope EscapableHandleScope(Isolate);

	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsTokenizer* JsTokenizer = new TNodeJsTokenizer(Tokenizer);
	JsTokenizer->Wrap(Instance);

	return EscapableHandleScope.Escape(Instance);
}

void TNodeJsTokenizer::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	EFailR("Use analytics.newTokenizer() instead of new Tokenizer();");
	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsTokenizer::getTokens(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected a string as the argument.");
	TNodeJsTokenizer* JsTokenizer = ObjectWrap::Unwrap<TNodeJsTokenizer>(Args.Holder());
	TStr TextStr = TNodeJsUtil::GetArgStr(Args, 0);
	TStrV TokenV; JsTokenizer->Tokenizer->GetTokens(TextStr, TokenV);

	Args.GetReturnValue().Set(TNodeJsUtil::GetStrArr(TokenV));
}

void TNodeJsTokenizer::getSentences(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected a string as the argument.");
	TStr TextStr = TNodeJsUtil::GetArgStr(Args, 0);
	TStrV SentenceV; TTokenizerUtil::Sentencize(TextStr, SentenceV);

	Args.GetReturnValue().Set(TNodeJsUtil::GetStrArr(SentenceV));
}

void TNodeJsTokenizer::getParagraphs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsString(), "Expected a string as the argument.");
	TStr TextStr = TNodeJsUtil::GetArgStr(Args, 0);
	TStrV ParagraphV; TTokenizerUtil::Sentencize(TextStr, ParagraphV);

	Args.GetReturnValue().Set(TNodeJsUtil::GetStrArr(ParagraphV));
}

///////////////////////////////
// Register functions, etc.
#ifndef MODULE_INCLUDE_ANALYTICS

void init(v8::Handle<v8::Object> exports) {
    // QMiner package
	TNodeJsSvmModel::Init(exports);
	TNodeJsRecLinReg::Init(exports);
	TNodeJsHMChain::Init(exports);
	TNodeJsNNet::Init(exports);
   // Linear algebra package
    TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
    TNodeJsVec<TInt, TAuxIntV>::Init(exports);
    TNodeJsVec<TStr, TAuxStrV>::Init(exports);
    TNodeJsFltVV::Init(exports);
    TNodeJsSpVec::Init(exports);
    TNodeJsSpMat::Init(exports);
	TNodeJsTokenizer::Init(exports);
}

NODE_MODULE(analytics, init)

#endif

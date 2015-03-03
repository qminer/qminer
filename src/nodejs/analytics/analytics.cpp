#include "analytics.h"

//////////////////////////////////////////////////////
// NodeJS - analytics

////////////////////////////////////////////////////////
// Support Vector Machine

TNodeJsSvmModel::TNodeJsSvmModel(const PJsonVal& ParamVal):
		Algorithm("SGD"),
		SvmCost(1.0),
		SvmUnbalance(1.0),
		SvmEps(1.0),
		SampleSize(1000),
		MxIter(10000),
		MxTime(1000*1),
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
	SvmEps(TFlt(SIn)),
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
	auto Object = new TNodeJsSvmModel(ParamVal);
	Object->Wrap(Obj);
	return Obj;	
}

v8::Local<v8::Object> TNodeJsSvmModel::WrapInst(v8::Local<v8::Object> Obj, TSIn& SIn) {
	auto Object = new TNodeJsSvmModel(SIn);
	Object->Wrap(Obj);
	return Obj;
}

void TNodeJsSvmModel::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	try {
		QmAssertR(Args.IsConstructCall(), "SVC: not a constructor call!");
		if (Args.Length() == 0) {
			Args.GetReturnValue().Set(TNodeJsSvmModel::WrapInst(Args.This(), TJsonVal::NewObj()));
			return;
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFIn::ClassId)) {
			// load the model from an input stream
			TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
			Args.GetReturnValue().Set(TNodeJsSvmModel::WrapInst(Args.This(), *JsFIn->SIn));
		}
		else {
			PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
			Args.GetReturnValue().Set(TNodeJsSvmModel::WrapInst(Args.This(), ParamVal));
		}
	}
	catch (const PExcept& Except) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())));
	}
}


void TNodeJsSvmModel::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		if (Model->Model != nullptr) {
			Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Model->GetParams()));
		}
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
		if (Model->Model != nullptr) {
			// get feature vector
			TFltV WgtV; Model->Model->GetWgtV(WgtV);

			Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsSvmModel::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		QmAssertR(Args.Length() == 1, "Should have 1 argument!");
		PSOut SOut;
		if (TNodeJsUtil::IsArgStr(Args, 0)) {
			SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
		}
		else {
			TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
			SOut = JsFOut->SOut;
		}

		Model->Save(*SOut);

		// we return nothing currently, just close the stream if filename was used
		if (TNodeJsUtil::IsArgStr(Args, 0)) {
			SOut.Clr();
		} else {
			// return output stream for convenience
			Args.GetReturnValue().Set(Args[0]);
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::save");
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
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::predict");
	}
}

void TNodeJsSvmModel::UpdateParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("algorithm")) Algorithm = ParamVal->GetObjStr("algorithm");
	if (ParamVal->IsObjKey("c")) SvmCost = ParamVal->GetObjNum("c");
	if (ParamVal->IsObjKey("j")) SvmUnbalance = ParamVal->GetObjNum("j");
	if (ParamVal->IsObjKey("eps")) SvmEps = ParamVal->GetObjNum("eps");
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
	ParamVal->AddToObj("eps", SvmEps);
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
	TFlt(SvmEps).Save(SOut);
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


v8::Persistent<v8::Function> TNodeJsSVC::constructor;

void TNodeJsSVC::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SVC"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.	
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);	

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS	
	exports->Set(v8::String::NewFromUtf8(Isolate, "SVC"), tpl->GetFunction());
#endif
}

void TNodeJsSVC::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
	QmAssertR(Args[1]->IsObject(), "SCV.fit: second argument expected to be object");

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
			}
			else if (Model->Algorithm == "PR_LOQO") {
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
			}
			else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmCost, Model->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));


				Model->Model = new TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
		}
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "SVC.fit");
	}
}

v8::Persistent<v8::Function> TNodeJsSVR::constructor;

void TNodeJsSVR::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SVR"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.		
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);	

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "SVR"), tpl->GetFunction());
#endif
}

void TNodeJsSVR::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args[0]->IsObject(), "first argument expected to be object");
	QmAssertR(Args[1]->IsObject(), "SVR.fit: second argument expected to be object");

	try {
		TNodeJsSvmModel* Model = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		Model->ClrModel();

		TFltV& ValV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject())->Vec;
		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpMat::ClassId)) {
			TVec<TIntFltKdV>& VecV = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Model->Model = new TSvm::TLinModel(TSvm::SolveRegression<TVec<TIntFltKdV>>(VecV, TLAMisc::GetMaxDimIdx(VecV) + 1,
					VecV.Len(), ValV, Model->SvmCost, Model->SvmEps, Model->MxTime,
					Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify));
			}
			else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefSparseTrainSet::New(VecV, ValV);
				PSVMModel SvmModel = TSVMModel::NewRegLinear(TrainSet, Model->SvmEps, Model->SvmCost,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));

				Model->Model = new TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)) {
			TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Model->Model = new TSvm::TLinModel(TSvm::SolveRegression<TFltVV>(VecV, VecV.GetRows(),
					VecV.GetCols(), ValV, Model->SvmCost, Model->SvmEps, Model->MxTime,
					Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify));
			}
			else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ValV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmEps, Model->SvmCost,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));


				Model->Model = new TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
		}
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "SVR.fit");
	}
}

////////////////////////////////////////////////
// QMiner-NodeJS-Recursive-Linear-Regression
const TStr TNodeJsRecLinReg::ClassId = "RecLinReg";

TNodeJsRecLinReg::TNodeJsRecLinReg(const TSignalProc::PRecLinReg& _Model):
		Model(_Model) {}


void TNodeJsRecLinReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsRecLinReg>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "RecLinReg"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);

#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "RecLinReg"),
			   tpl->GetFunction());
#endif
}

TNodeJsRecLinReg* TNodeJsRecLinReg::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

	if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFIn::ClassId)) {
		TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
		return new TNodeJsRecLinReg(TSignalProc::TRecLinReg::Load(*JsFIn->SIn));
	}
	else {
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		const int Dim = ParamVal->GetObjInt("dim");
		const double RegFact = ParamVal->GetObjNum("regFact", 1.0);
		const double ForgetFact = ParamVal->GetObjNum("forgetFact", 1.0);

		return new TNodeJsRecLinReg(TSignalProc::TRecLinReg::New(Dim, RegFact, ForgetFact));
	}
}

void TNodeJsRecLinReg::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)");
	QmAssertR(TNodeJsUtil::IsArgFlt(Args, 1), "Argument 1 should be float!");

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
}

void TNodeJsRecLinReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)");

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	TNodeJsFltV* JsFeatVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

	QmAssertR(Model->Model->GetDim() == JsFeatVec->Vec.Len(),
		"RecLinRegModel.learn: model dimension != sample dimension");

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Model->Model->Predict(JsFeatVec->Vec)));
}

void TNodeJsRecLinReg::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Model->GetParams()));
}

void TNodeJsRecLinReg::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Info.Holder());

	// get feature vector
	TFltV Coef;	Model->Model->GetCoeffs(Coef);

	Info.GetReturnValue().Set(TNodeJsFltV::New(Coef));
}

void TNodeJsRecLinReg::dim(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Info.Holder());
	Info.GetReturnValue().Set(v8::Integer::New(Isolate, Model->Model->GetDim()));
}

void TNodeJsRecLinReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");
	PSOut SOut;
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
	}
	else {
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
		SOut = JsFOut->SOut;
	}

	Model->Model.Save(*SOut);

	// we return nothing currently, just close the stream if filename was used
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut.Clr();
	}
	else {
		// return output stream for convenience
		Args.GetReturnValue().Set(Args[0]);
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
// Logistic regression model
const TStr TNodeJsLogReg::ClassId = "LogReg";
v8::Persistent<v8::Function> TNodeJsLogReg::constructor;

TNodeJsLogReg* TNodeJsLogReg::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		// parse the argumentts
		PJsonVal ArgJson = Args.Length() > 0 ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();

		const double Lambda = ArgJson->IsObjKey("lambda") ? ArgJson->GetObjNum("lambda") : 1;
		const bool IncludeIntercept = ArgJson->IsObjKey("intercept") ? ArgJson->GetObjBool("intercept") : false;

		return new TNodeJsLogReg(TMl::TLogReg(Lambda, IncludeIntercept));
	} catch (const PExcept& Except) {
		Isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())));
		return nullptr;
	}
}

void TNodeJsLogReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "LogReg"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "LogReg"),
			   tpl->GetFunction());
#endif
}


void TNodeJsLogReg::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "logreg.fit: expects at least 2 arguments!");

	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());

	// get the arguments
	TNodeJsFltVV* InstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* ResponseJsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

	if (Args.Length() > 2) {
		const double ConvergEps = TNodeJsUtil::GetArgFlt(Args, 2);
		JsModel->LogReg.Fit(InstanceMat->Mat, ResponseJsV->Vec, ConvergEps);
	} else {
		JsModel->LogReg.Fit(InstanceMat->Mat, ResponseJsV->Vec);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsLogReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "logreg.predict: expects 1 argument!");

	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());
	TNodeJsFltV* JsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

	const double Result = JsModel->LogReg.Predict(JsFtrV->Vec);

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsLogReg::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsLogReg* JsLogReg = ObjectWrap::Unwrap<TNodeJsLogReg>(Info.Holder());

		if (JsLogReg != nullptr) {
			TFltV WgtV;	JsLogReg->LogReg.GetWgtV(WgtV);
			Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsLogReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "logreg.save: expects 1 argument!");

	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->LogReg.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

////////////////////////////////////////////////////////
// Exponential Regression
const TStr TNodeJsExpReg::ClassId = "ExpReg";
v8::Persistent<v8::Function> TNodeJsExpReg::constructor;

TNodeJsExpReg* TNodeJsExpReg::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		// parse the argumentts
		PJsonVal ArgJson = Args.Length() > 0 ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();

		const double Lambda = ArgJson->IsObjKey("lambda") ? ArgJson->GetObjNum("lambda") : 1;
		const bool IncludeIntercept = ArgJson->IsObjKey("intercept") ? ArgJson->GetObjBool("intercept") : false;

		return new TNodeJsExpReg(TMl::TExpReg(Lambda, IncludeIntercept));
	} catch (const PExcept& Except) {
		Isolate->ThrowException(v8::Exception::TypeError(
					v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())));
		return nullptr;
	}
}

void TNodeJsExpReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, ClassId.CStr()),
			   tpl->GetFunction());
#endif
}

void TNodeJsExpReg::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "expreg.fit: expects at least 2 arguments!");

	TNodeJsExpReg* JsModel = ObjectWrap::Unwrap<TNodeJsExpReg>(Args.Holder());

	// get the arguments
	TNodeJsFltVV* InstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* ResponseJsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

	if (Args.Length() > 2) {
		const double ConvergEps = TNodeJsUtil::GetArgFlt(Args, 2);
		JsModel->ExpReg.Fit(InstanceMat->Mat, ResponseJsV->Vec, ConvergEps);
	} else {
		JsModel->ExpReg.Fit(InstanceMat->Mat, ResponseJsV->Vec);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsExpReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "expreg.predict: expects 1 argument!");

	TNodeJsExpReg* JsModel = ObjectWrap::Unwrap<TNodeJsExpReg>(Args.Holder());

	// get the arguments
	TNodeJsFltV* JsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

	const double Result = JsModel->ExpReg.Predict(JsFtrV->Vec);

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsExpReg::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsExpReg* JsExpReg = ObjectWrap::Unwrap<TNodeJsExpReg>(Info.Holder());

		if (JsExpReg != nullptr) {
			TFltV WgtV;	JsExpReg->ExpReg.GetWgtV(WgtV);
			Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsExpReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "expreg.save: expects 1 argument!");

	TNodeJsExpReg* JsModel = ObjectWrap::Unwrap<TNodeJsExpReg>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->ExpReg.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
const TStr TNodeJsHMChain::ClassId = "HMC";
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

void TNodeJsHMChain::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "HMC"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "update", _update);
	NODE_SET_PROTOTYPE_METHOD(tpl, "futureStates", _futureStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "pastStates", _pastStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "probsOverTime", _probsOverTime);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histStates", _histStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "currState", _currState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fullCoords", _fullCoords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histogram", _histogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "stateIds", _stateIds);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateWgtV", _getStateWgtV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTransitionModel", _getTransitionModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onStateChanged", _onStateChanged);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAnomaly", _onAnomaly);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onOutlier", _onOutlier);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHierarchy", _rebuildHierarchy);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHistograms", _rebuildHistograms);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateName", _getStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setStateName", _setStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "HMC"),
			   tpl->GetFunction());
#endif
}

TNodeJsHMChain* TNodeJsHMChain::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	QmAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

	if (TNodeJsUtil::IsArgJson(Args, 0)) {
		const PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

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
			} else if (TimeUnitStr == "month") {
				TimeUnit = TMc::TCtMChain::TU_MONTH;
			} else {
				throw TExcept::New("Invalid time unit: " + TimeUnitStr, "TJsHierCtmc::TJsHierCtmc");
			}

			MChain = new TMc::TCtMChain(TimeUnit, DeltaTm, Verbose);
		} else if (TransitionJson->GetObjStr("type") == "discrete") {
			MChain = new TMc::TDtMChain(Verbose);
		}

		// clustering
		TMl::PFullClust Clust = NULL;

		const TStr ClustAlg = ClustJson->GetObjStr("type");
		const double Sample = ClustJson->IsObjKey("sample") ? ClustJson->GetObjNum("sample") : 1;
		const int NHistBins = ClustJson->IsObjKey("histogramBins") ? ClustJson->GetObjInt("histogramBins") : 20;

		if (ClustAlg == "dpmeans") {
			const double Lambda = ClustJson->GetObjNum("lambda");
			const int MinClusts = ClustJson->IsObjKey("minClusts") ? ClustJson->GetObjInt("minClusts") : 1;
			const int MxClusts = ClustJson->IsObjKey("maxClusts") ? ClustJson->GetObjInt("maxClusts") : TInt::Mx;
			const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
			Clust = new TMl::TDpMeans(NHistBins, Sample, Lambda, MinClusts, MxClusts, TRnd(RndSeed), Verbose);
		} else if (ClustAlg == "kmeans") {
			const int K = ClustJson->GetObjInt("k");
			const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
			Clust = new TMl::TFullKMeans(NHistBins, Sample, K, TRnd(RndSeed), Verbose);
		} else {
			throw TExcept::New("Invalivalid clustering type: " + ClustAlg, "TJsHierCtmc::TJsHierCtmc");
		}

		// create the model
		TMc::PHierarch AggClust = new TMc::THierarch(NPastStates + 1, Verbose);

		// finish
		TMc::PHierarchCtmc HMcModel = new TMc::THierarchCtmc(Clust, MChain, AggClust, Verbose);

		return new TNodeJsHMChain(HMcModel);
	} else {
		// load from file
		PSIn SIn = TNodeJsUtil::IsArgStr(Args, 0) ?
				TFIn::New(TNodeJsUtil::GetArgStr(Args, 0)) :
				ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject())->SIn;

		return new TNodeJsHMChain(SIn);
	}
}

void TNodeJsHMChain::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "hmc.fit expect 2 or more arguments!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	TNodeJsFltVV* JsInstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* JsRecTmV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

	TUInt64V RecTmV(JsRecTmV->Vec.Len(), 0);
	for (int64 i = 0; i < JsRecTmV->Vec.Len(); i++) {
		RecTmV.Add(TNodeJsUtil::GetCppTimestamp(JsRecTmV->Vec[i]));
	}

	if (Args.Length() > 2) {
		const TNodeJsBoolV* BatchEndJsV = ObjectWrap::Unwrap<TNodeJsBoolV>(Args[2]->ToObject());
		const TBoolV& BatchEndV = BatchEndJsV->Vec;
		JsMChain->McModel->InitBatches(JsInstanceMat->Mat, RecTmV, BatchEndV);
	} else {
		JsMChain->McModel->Init(JsInstanceMat->Mat, RecTmV);
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::update(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
}

void TNodeJsHMChain::futureStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
}

void TNodeJsHMChain::pastStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
}

void TNodeJsHMChain::probsOverTime(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
	const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);
	const double StartTm = TNodeJsUtil::GetArgFlt(Args, 2);
	const double EndTm = TNodeJsUtil::GetArgFlt(Args, 3);
	const double DeltaTm = TNodeJsUtil::GetArgFlt(Args, 4);

	TVec<TFltV> FutProbV, PastProbV;
	TIntV StateIdV;
	JsMChain->McModel->GetProbVOverTm(Level, StartState, StartTm, EndTm, DeltaTm, StateIdV, FutProbV, PastProbV);

	v8::Local<v8::Array> TimeArr = v8::Array::New(Isolate, FutProbV.Len() + PastProbV.Len());

	double Tm = -DeltaTm*PastProbV.Len();
	for (int i = 0; i < PastProbV.Len(); i++) {
		const TFltV& ProbV = PastProbV[PastProbV.Len()-1-i];

		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);
		v8::Local<v8::Array> ProbArr = v8::Array::New(Isolate, FutProbV[0].Len());

		for (int j = 0; j < ProbV.Len(); j++) {
			v8::Local<v8::Object> ProbObj = v8::Object::New(Isolate);

			ProbObj->Set(v8::String::NewFromUtf8(Isolate, "stateId"), v8::Integer::New(Isolate, StateIdV[j]));
			ProbObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, ProbV[j]));

			ProbArr->Set(j, ProbObj);
		}

		StateObj->Set(v8::String::NewFromUtf8(Isolate, "time"), v8::Number::New(Isolate, Tm));
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "probs"), ProbArr);

		TimeArr->Set(i, StateObj);

		Tm += DeltaTm;
	}

	for (int i = 0; i < FutProbV.Len(); i++) {
		const TFltV& ProbV = FutProbV[i];

		v8::Local<v8::Object> StateObj = v8::Object::New(Isolate);
		v8::Local<v8::Array> ProbArr = v8::Array::New(Isolate, FutProbV[0].Len());

		for (int j = 0; j < ProbV.Len(); j++) {
			v8::Local<v8::Object> ProbObj = v8::Object::New(Isolate);

			ProbObj->Set(v8::String::NewFromUtf8(Isolate, "stateId"), v8::Integer::New(Isolate, StateIdV[j]));
			ProbObj->Set(v8::String::NewFromUtf8(Isolate, "prob"), v8::Number::New(Isolate, ProbV[j]));

			ProbArr->Set(j, ProbObj);
		}

		StateObj->Set(v8::String::NewFromUtf8(Isolate, "time"), v8::Number::New(Isolate, Tm));
		StateObj->Set(v8::String::NewFromUtf8(Isolate, "probs"), ProbArr);

		TimeArr->Set(PastProbV.Len() + i, StateObj);

		Tm += DeltaTm;
	}

	Args.GetReturnValue().Set(TimeArr);
}

void TNodeJsHMChain::histStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

	TIntV StateIdV;	JsMChain->McModel->GetHistStateIdV(Level, StateIdV);

	v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		StateArr->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
	}

	Args.GetReturnValue().Set(StateArr);
}

void TNodeJsHMChain::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsMChain->McModel->SaveJson()));
}

void TNodeJsHMChain::getTransitionModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
}

void TNodeJsHMChain::currState(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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
}

void TNodeJsHMChain::fullCoords(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	TFltV FtrV;	JsMChain->McModel->GetCentroid(StateId, FtrV);

	v8::Local<v8::Array> FtrVJson = v8::Array::New(Isolate, FtrV.Len());
	for (int i = 0; i < FtrV.Len(); i++) {
		FtrVJson->Set(i, v8::Number::New(Isolate, FtrV[i]));
	}

	Args.GetReturnValue().Set(FtrVJson);
}

void TNodeJsHMChain::histogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "hmc.histogram: expects 2 arguments!");

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
}

void TNodeJsHMChain::stateIds(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.stateIds: expects 1 argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const double Height = TNodeJsUtil::GetArgFlt(Args, 0);

	TIntV StateIdV;
	JsMChain->McModel->GetStateIdVAtHeight(Height, StateIdV);

	v8::Local<v8::Array> StateIdJsV = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		StateIdJsV->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
	}

	Args.GetReturnValue().Set(StateIdJsV);
}

void TNodeJsHMChain::getStateWgtV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.getStateWgtV: expects 1 argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const double StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	TFltV WgtV;
	JsMChain->McModel->GetStateWgtV(StateId, WgtV);


	v8::Local<v8::Array> JsWgtV = v8::Array::New(Isolate, WgtV.Len());
	for (int i = 0; i < WgtV.Len(); i++) {
		JsWgtV->Set(i, v8::Number::New(Isolate, WgtV[i]));
	}

	Args.GetReturnValue().Set(JsWgtV);
}

void TNodeJsHMChain::onStateChanged(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

	JsMChain->StateChangedCallback.Reset(Isolate, Callback);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::onAnomaly(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

	JsMChain->AnomalyCallback.Reset(Isolate, Callback);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::onOutlier(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0 && Args[0]->IsFunction(), "hmc.onStateChanged: First argument expected to be a function!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	v8::Handle<v8::Function> Callback = v8::Handle<v8::Function>::Cast(Args[0]);

	JsMChain->OutlierCallback.Reset(Isolate, Callback);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::rebuildHierarchy(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	JsMChain->McModel->InitHierarch();

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::rebuildHistograms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.rebuildHistograms: expects 1 argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());

	JsMChain->McModel->InitHistograms(JsFltVV->Mat);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::getStateName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.getStateName: expects 1 argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr& StateNm = JsMChain->McModel->GetStateNm(StateId);

	Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, StateNm.CStr()));
}

void TNodeJsHMChain::setStateName(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "hmc.setStateName: expects 2 arguments!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const TStr StateNm = TNodeJsUtil::GetArgStr(Args, 1);

	JsMChain->McModel->SetStateNm(StateId, StateNm);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsHMChain::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "hmc.setParams: Expects one argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

	JsMChain->SetParams(ParamVal);

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsHMChain::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

	PSOut SOut;
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
	} else {
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
		SOut = JsFOut->SOut;
	}

	JsMChain->McModel->Save(*SOut);

	// we return nothing currently, just close the stream if filename was used
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut.Clr();
	} else {
		// return output stream for convenience
		Args.GetReturnValue().Set(Args[0]);
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
const TStr TNodeJsNNet::ClassId = "NNet";
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

void TNodeJsNNet::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsNNet>);
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

#ifndef MODULE_INCLUDE_ANALYTICS
	exports->Set(v8::String::NewFromUtf8(Isolate, "NNet"),
		tpl->GetFunction());
#endif

}

TNodeJsNNet* TNodeJsNNet::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Expected one argument.");

	if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFIn::ClassId)) {
		// load the model from an input stream
		// currently not used, will be implemented
		TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
		return new TNodeJsNNet(*JsFIn->SIn);
	}
	else { // got JSON
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		return new TNodeJsNNet(ParamVal);
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
	exports->Set(v8::String::NewFromUtf8(Isolate, "Tokenizer"), tpl->GetFunction());
}

v8::Local<v8::Object> TNodeJsTokenizer::New(const PTokenizer& Tokenizer) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope EscapableHandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsTokenizer::New: constructor is empty. Did you call TNodeJsTokenizer::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	TNodeJsTokenizer* JsTokenizer = new TNodeJsTokenizer(Tokenizer);
	JsTokenizer->Wrap(Instance);

	return EscapableHandleScope.Escape(Instance);
}

void TNodeJsTokenizer::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	QmAssertR(ParamVal->IsObjKey("type"),
		"Missing tokenizer type " + ParamVal->SaveStr());
	const TStr& TypeNm = ParamVal->GetObjStr("type");
	// create
	PTokenizer Tokenizer = TTokenizer::New(TypeNm, ParamVal);
	// set return object
	Args.GetReturnValue().Set(TNodeJsTokenizer::New(Tokenizer));
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
	TNodeJsSVC::Init(exports);
	TNodeJsSVR::Init(exports);
	TNodeJsRecLinReg::Init(exports);
	TNodeJsLogReg::Init(exports);
	TNodeJsExpReg::Init(exports);
	TNodeJsHMChain::Init(exports);
	TNodeJsNNet::Init(exports);
	TNodeJsTokenizer::Init(exports);

	// We need this if we want to return linear algebra objects (for example TNodeJsFltV::New(TFltV ...) will crash without calling the appropriate init function)
	TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
	TNodeJsVec<TInt, TAuxIntV>::Init(exports);
	TNodeJsVec<TStr, TAuxStrV>::Init(exports);
	TNodeJsFltVV::Init(exports);
	TNodeJsSpVec::Init(exports);
	TNodeJsSpMat::Init(exports);

	// File stream
	TNodeJsFIn::Init(exports);
	TNodeJsFOut::Init(exports);
}

NODE_MODULE(analytics, init)

#endif

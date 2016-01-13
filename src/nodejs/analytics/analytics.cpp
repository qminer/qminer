/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "analytics.h"

//////////////////////////////////////////////////////
// NodeJS - analytics

////////////////////////////////////////////////////////
// Support Vector Machine

TNodeJsSvmModel::TNodeJsSvmModel(const PJsonVal& ParamVal):
		Algorithm("SGD"),
		SvmCost(1.0),
		SvmUnbalance(1.0),
		SvmEps(0.1),
		SampleSize(1000),
		MxIter(10000),
		MxTime(1000*1),
		MnDiff(1e-6),
		Verbose(false),
		Notify(TNotify::NullNotify) {

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
        Model(SIn) {

	if (Verbose) { Notify = TNotify::StdNotify; }
}

TNodeJsSvmModel* TNodeJsSvmModel::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() == 0) {
		// create new model with default parameters
		return new TNodeJsSvmModel(TJsonVal::NewObj());
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
		return new TNodeJsSvmModel(*JsFIn->SIn);
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
		// create new model from given parameters
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		return new TNodeJsSvmModel(ParamVal);
	}
	else {
		throw TExcept::New("new svm: wrong arguments in constructor!");
	}
}

void TNodeJsSvmModel::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "svm.getParams: takes 1 argument!");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsModel->GetParams()));
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "SVM::getParams");
	}
}

void TNodeJsSvmModel::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "svm.setParams: takes 1 argument!");
	EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "svm.setParams: first argument should be a Javascript object!");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		JsModel->UpdateParams(ParamVal);

		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "SVM::setParams");
	}
}

void TNodeJsSvmModel::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Info.Holder());
		Info.GetReturnValue().Set(TNodeJsFltV::New(JsModel->Model.GetWgtV()));
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsSvmModel::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());
		// get output stream from argumetns
		TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
		// save model
		JsModel->Save(*JsFOut->SOut);
		// return output stream for convenience
		Args.GetReturnValue().Set(Args[0]);
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::save");
	}
}

void TNodeJsSvmModel::decisionFunction(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "svm.decisionFunction: expecting 1 argument");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
			TNodeJsFltV* Vec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
			const double Res = JsModel->Model.Predict(Vec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpVec>(Args, 0)) {
			TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
			const double Res = JsModel->Model.Predict(SpVec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
			const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
			TFltV ResV(Mat.GetCols(), 0);
			for (int ColN = 0; ColN < Mat.GetCols(); ColN++) {
				ResV.Add(JsModel->Model.Predict(Mat, ColN));
			}
			Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
			const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
			TFltV ResV(Mat.Len(), 0);
			for (int ColN = 0; ColN < Mat.Len(); ColN++) {
				ResV.Add(JsModel->Model.Predict(Mat[ColN]));
			}
			Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
		}
		else {
			throw TExcept::New("svm.decisionFunction: unsupported type of the first argument");
		}
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::decisionFunction");
	}
}

// TODO: try to call directly decisionFunction and check response
void TNodeJsSvmModel::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "svm.predict: expecting 1 argument");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
			TNodeJsFltV* Vec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
			const double Res = (JsModel->Model.Predict(Vec->Vec) > 0.0) ? 1.0 : -1.0;
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpVec>(Args, 0)) {
			TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
			const double Res = (JsModel->Model.Predict(SpVec->Vec) > 0.0) ? 1.0 : -1.0;
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
			const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
			TFltV ResV(Mat.GetCols(), 0);
			for (int ColN = 0; ColN < Mat.GetCols(); ColN++) {
				ResV.Add(JsModel->Model.Predict(Mat, ColN) > 0.0 ? 1.0 : -1.0);
			}
			Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
			const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
			TFltV ResV(Mat.Len(), 0);
			for (int ColN = 0; ColN < Mat.Len(); ColN++) {
				ResV.Add(JsModel->Model.Predict(Mat[ColN]) > 0.0 ? 1.0 : -1.0);
			}
			Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
		}
		else {
			throw TExcept::New("svm.predict: unsupported type of the first argument");
		}
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::predict");
	}
}

void TNodeJsSvmModel::UpdateParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("algorithm")) Algorithm = ParamVal->GetObjStr("algorithm");
	if (ParamVal->IsObjKey("c")) SvmCost = ParamVal->GetObjNum("c");
	if (ParamVal->IsObjKey("j")) SvmUnbalance = ParamVal->GetObjNum("j");
	if (ParamVal->IsObjKey("eps")) SvmEps = ParamVal->GetObjNum("eps");
	if (ParamVal->IsObjKey("batchSize")) SampleSize = ParamVal->GetObjInt("batchSize");
	if (ParamVal->IsObjKey("maxIterations")) MxIter = ParamVal->GetObjInt("maxIterations");
	if (ParamVal->IsObjKey("maxTime")) MxTime = TFlt::Round(1000.0 * ParamVal->GetObjNum("maxTime"));
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
	ParamVal->AddToObj("maxTime", MxTime / 1000); // convert from miliseconds to seconds
	ParamVal->AddToObj("minDiff", MnDiff);
	ParamVal->AddToObj("verbose", Verbose);

	return ParamVal;
}

void TNodeJsSvmModel::Save(TSOut& SOut) const {
	Algorithm.Save(SOut);
	TFlt(SvmCost).Save(SOut);
	TFlt(SvmUnbalance).Save(SOut);
	TFlt(SvmEps).Save(SOut);
	TInt(SampleSize).Save(SOut);
	TInt(MxIter).Save(SOut);
	TInt(MxTime).Save(SOut);
	TFlt(MnDiff).Save(SOut);
	TBool(Verbose).Save(SOut);
	Model.Save(SOut);
}

void TNodeJsSvmModel::ClrModel() {
	Model = TSvm::TLinModel();
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Classification

void TNodeJsSVC::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsSvmModel>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SVC"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.	
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "decisionFunction", _decisionFunction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	exports->Set(v8::String::NewFromUtf8(Isolate, "SVC"), tpl->GetFunction());
}

void TNodeJsSVC::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "SVC.fit: expecting 2 arguments!");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		// check target vector is actually a vector
		EAssertR(TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 1), "SVC.fit: second argument expected to be la.Vector!");
		TFltV& ClsV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 1)->Vec;

		if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
			TVec<TIntFltKdV>& VecV = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject())->Mat;
			if (JsModel->Algorithm == "SGD") {
				JsModel->Model = TSvm::SolveClassify<TVec<TIntFltKdV>>(VecV, TLAMisc::GetMaxDimIdx(VecV) + 1,
					VecV.Len(), ClsV, JsModel->SvmCost, JsModel->SvmUnbalance, JsModel->MxTime,
					JsModel->MxIter, JsModel->MnDiff, JsModel->SampleSize, JsModel->Notify);
			}
			else if (JsModel->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefSparseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, JsModel->SvmCost, JsModel->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(JsModel->MxTime, JsModel->Verbose ? 2 : 0));
				JsModel->Model = TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
			else if (JsModel->Algorithm == "LIBSVM") {
				JsModel->Model = TSvm::LibSvmSolveClassify(VecV, ClsV, JsModel->SvmCost,
					TQm::TEnv::Debug, TQm::TEnv::Error);
			}
			else {
				throw TExcept::New("SVC.fit: unknown algorithm " + JsModel->Algorithm);
			}
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
			TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
			if (JsModel->Algorithm == "SGD") {
				JsModel->Model = TSvm::SolveClassify<TFltVV>(VecV, VecV.GetRows(),
					VecV.GetCols(), ClsV, JsModel->SvmCost, JsModel->SvmUnbalance, JsModel->MxTime,
					JsModel->MxIter, JsModel->MnDiff, JsModel->SampleSize, JsModel->Notify);
			}
			else if (JsModel->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, JsModel->SvmCost, JsModel->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(JsModel->MxTime, JsModel->Verbose ? 2 : 0));
				JsModel->Model = TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
			else if (JsModel->Algorithm == "LIBSVM") {
				JsModel->Model = TSvm::LibSvmSolveClassify(VecV, ClsV, JsModel->SvmCost,
					TQm::TEnv::Debug, TQm::TEnv::Error);
			}
			else {
				throw TExcept::New("SVC.fit: unknown algorithm " + JsModel->Algorithm);
			}
		}
		else {
			throw TExcept::New("SVC.fit: Unsupported first argument");
		}
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "SVC.fit");
	}
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Regression

void TNodeJsSVR::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsSvmModel>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SVR"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.		
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "decisionFunction", _decisionFunction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _decisionFunction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	exports->Set(v8::String::NewFromUtf8(Isolate, "SVR"), tpl->GetFunction());
}

void TNodeJsSVR::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "SVR.fit: expecting 2 arguments!");

	try {
		TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Args.Holder());

		// check target vector is actually a vector
		EAssertR(TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 1), "SVR.fit: second argument expected to be la.Vector!");
		TFltV& ClsV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 1)->Vec;


		if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
			TVec<TIntFltKdV>& VecV = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject())->Mat;
			if (JsModel->Algorithm == "SGD") {
				JsModel->Model = TSvm::SolveRegression<TVec<TIntFltKdV>>(VecV, TLAMisc::GetMaxDimIdx(VecV) + 1,
					VecV.Len(), ClsV, JsModel->SvmCost, JsModel->SvmEps, JsModel->MxTime,
					JsModel->MxIter, JsModel->MnDiff, JsModel->SampleSize, JsModel->Notify);
			}
			else if (JsModel->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefSparseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewRegLinear(TrainSet, JsModel->SvmEps, JsModel->SvmCost,
					TIntV(), TSVMLearnParam::Lin(JsModel->MxTime, JsModel->Verbose ? 2 : 0));
				JsModel->Model = TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
			else if (JsModel->Algorithm == "LIBSVM") {
				JsModel->Model = TSvm::LibSvmSolveRegression(VecV, ClsV, JsModel->SvmEps, JsModel->SvmCost,
					TQm::TEnv::Debug, TQm::TEnv::Error);
			}
			else {
				throw TExcept::New("SVR.fit: unknown algorithm " + JsModel->Algorithm);
			}
		}
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
			TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
			if (JsModel->Algorithm == "SGD") {
				JsModel->Model = TSvm::SolveRegression<TFltVV>(VecV, VecV.GetRows(),
					VecV.GetCols(), ClsV, JsModel->SvmCost, JsModel->SvmEps, JsModel->MxTime,
					JsModel->MxIter, JsModel->MnDiff, JsModel->SampleSize, JsModel->Notify);
			}
			else if (JsModel->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewRegLinear(TrainSet, JsModel->SvmEps, JsModel->SvmCost,
					TIntV(), TSVMLearnParam::Lin(JsModel->MxTime, JsModel->Verbose ? 2 : 0));
				JsModel->Model = TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh());
			}
			else if (JsModel->Algorithm == "LIBSVM") {
				JsModel->Model = TSvm::LibSvmSolveRegression(VecV, ClsV, JsModel->SvmEps, JsModel->SvmCost,
					TQm::TEnv::Debug, TQm::TEnv::Error);
			}
			else {
				throw TExcept::New("SVR.fit: unknown algorithm " + JsModel->Algorithm);
			}
		}
		else {
			throw TExcept::New("SVR.fit: Unsupported first argument");
		}
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "SVR.fit");
	}
}

/////////////////////////////////////////////
// Ridge Regression
void TNodeJsRidgeReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsRidgeReg>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "decisionFunction", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsRidgeReg* TNodeJsRidgeReg::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() == 0) {
		return new TNodeJsRidgeReg(TRegression::TRidgeReg(0));
	}
	else if (Args.Length() > 0 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
		return new TNodeJsRidgeReg(*JsFIn->SIn);
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		const double Gamma = ParamVal->GetObjNum("gamma", 0.0);
		return new TNodeJsRidgeReg(TRegression::TRidgeReg(Gamma));
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgFlt(Args, 0)) {
		printf("DEPRECATED: consider using Json object as argument!\n");
		// create new model from given gamma parameter
		const double Gamma = TNodeJsUtil::GetArgFlt(Args, 0, 0.0);
		return new TNodeJsRidgeReg(TRegression::TRidgeReg(Gamma));
	}
	else {
		throw TExcept::New("new RidgeReg: wrong arguments in constructor!");
	}
}

void TNodeJsRidgeReg::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "RidgeReg.getParams: expects none arguments!");

	TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Args.Holder());
	// get the parameters
	PJsonVal ParamVal = TJsonVal::NewObj();
	ParamVal->AddToObj("gamma", JsModel->Model.GetGamma());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsRidgeReg::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "RidgeReg.setParams: Expects one argument!");

	if (TNodeJsUtil::IsArgJson(Args, 0)) {
		TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Args.Holder());
		// set the parameters
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		const double Gamma = ParamVal->GetObjNum("gamma");
		JsModel->Model.SetGamma(Gamma);

		Args.GetReturnValue().Set(Args.Holder());
	}
	else {
		throw TExcept::New("RidgeReg.setParams: expecting Json object!");
	}
}

void TNodeJsRidgeReg::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "RidgeReg.fit: expects at least 2 arguments!");

	TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Args.Holder());

	// get the arguments
	TNodeJsFltVV* InstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* ResponseJsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());
	// fit model
	JsModel->Model.Fit(InstanceMat->Mat, ResponseJsV->Vec);
	// return self
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRidgeReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "RidgeReg.predict: expects 1 argument!");

	TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Args.Holder());

	// get the arguments
	TNodeJsFltV* JsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
	const double Result = JsModel->Model.Predict(JsFtrV->Vec);

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsRidgeReg::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Info.Holder());

	Info.GetReturnValue().Set(TNodeJsFltV::New(JsModel->Model.GetWgtV()));
}

void TNodeJsRidgeReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFOut::GetClassId()),
		"RidgeReg.save: expects 1 argument of type qminer.fs.FOut!");

	TNodeJsRidgeReg* JsModel = ObjectWrap::Unwrap<TNodeJsRidgeReg>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->Model.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(Args[0]);
}


/////////////////////////////////////////////
// Sigmoid
void TNodeJsSigmoid::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsSigmoid>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getModel", _getModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "decisionFunction", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsSigmoid* TNodeJsSigmoid::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() > 0 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
		return new TNodeJsSigmoid(*JsFIn->SIn);
	}
	else {
		return new TNodeJsSigmoid();
	}
}

void TNodeJsSigmoid::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "Sigmoid.getParams: expects 0 arguments!");

	// get the parameters
	PJsonVal ParamVal = TJsonVal::NewObj();
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsSigmoid::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Sigmoid.setParams: expects only 1 argument!");

	if (TNodeJsUtil::IsArgJson(Args, 0)) {
		Args.GetReturnValue().Set(Args.Holder());
	}
	else {
		throw TExcept::New("Sigmoid.setParams: expecting Json object!");
	}
}

void TNodeJsSigmoid::getModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "Sigmoid.getParams: expects 0 arguments!");
	TNodeJsSigmoid* JsModel = ObjectWrap::Unwrap<TNodeJsSigmoid>(Args.Holder());

	PJsonVal ParamVal = TJsonVal::NewObj();
	double A, B; JsModel->Sigmoid.GetSigmoidAB(A, B);

	ParamVal->AddToObj("A", A);
	ParamVal->AddToObj("B", B);

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsSigmoid::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "Sigmoid.fit: expects at least 2 arguments!");

	TNodeJsSigmoid* JsModel = ObjectWrap::Unwrap<TNodeJsSigmoid>(Args.Holder());

	if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
		EAssertR(TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 1),
			"Sigmoid.fit: Both parameters need to be of same type (e.g., vector).");
		// get the arguments
		const TFltV& PredV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject())->Vec;
		const TFltV& TrueV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject())->Vec;
		EAssertR(PredV.Len() == TrueV.Len(), "Sigmoid.fit: true and predicted labels have different length.");
		// load into input for sigmoid
		TFltIntKdV PredTrueV(PredV.Len(), 0);
		for (int EltN = 0; EltN < PredV.Len(); EltN++) {
			PredTrueV.Add(TFltIntKd(PredV[EltN], TFlt::Round(TrueV[EltN])));
		}
		// fit!
		JsModel->Sigmoid = TSigmoid(PredTrueV);
	}
	// return self
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSigmoid::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Sigmoid.predict: expects 1 argument!");

	TNodeJsSigmoid* JsModel = ObjectWrap::Unwrap<TNodeJsSigmoid>(Args.Holder());
	const TSigmoid& Sigmoid = JsModel->Sigmoid;

	// get the arguments
	if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
		const TFltV& PredV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject())->Vec;
		TFltV ResV(PredV.Len(), 0);
		for (int EltN = 0; EltN < PredV.Len(); EltN++) {
			ResV.Add(Sigmoid.GetVal(PredV[EltN]));
		}
		Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
	}
	else if (TNodeJsUtil::IsArgFlt(Args, 0)) {
		const double PredVal = TNodeJsUtil::GetArgFlt(Args, 0);
		const double Result = Sigmoid.GetVal(PredVal);
		Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
	}
	else {
		throw TExcept::New("Sigmoid.predict: invalid argument type");
	}
}

void TNodeJsSigmoid::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFOut>(Args, 0),
		"Sigmoid.save: expects 1 argument of type qminer.fs.FOut!");

	TNodeJsSigmoid* JsModel = ObjectWrap::Unwrap<TNodeJsSigmoid>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->Sigmoid.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(Args[0]);
}


/////////////////////////////////////////////
// Nearest Neighbor Annomaly Detection
void TNodeJsNNAnomalies::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsNNAnomalies>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getModel", _getModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "partialFit", _partialFit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "decisionFunction", _decisionFunction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);	
	NODE_SET_PROTOTYPE_METHOD(tpl, "explain", _explain);
	
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);

	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsNNAnomalies::TNodeJsNNAnomalies(const PJsonVal& ParamVal) :
Model(ParamVal->GetObjNum("rate", 0.05), ParamVal->GetObjInt("windowSize", 100)) { }


TNodeJsNNAnomalies* TNodeJsNNAnomalies::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() > 0 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
		return new TNodeJsNNAnomalies(*JsFIn->SIn);
	}
	else {
		// create new model from given gamma parameter
		PJsonVal JsonVal = (Args.Length() > 0) ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();
		return new TNodeJsNNAnomalies(JsonVal);
	}
}

void TNodeJsNNAnomalies::SetParams(const PJsonVal& ParamVal) {
	Model = TAnomalyDetection::TNearestNeighbor(
		ParamVal->GetObjNum("rate", 0.05),
		ParamVal->GetObjInt("windowSize", 100));
}

PJsonVal TNodeJsNNAnomalies::GetParams() const {
	PJsonVal ParamVal = TJsonVal::NewObj();
	ParamVal->AddToObj("rate", Model.GetRate(0));
	ParamVal->AddToObj("windowSize", Model.GetWindowSize());
	return ParamVal;
}

PJsonVal TNodeJsNNAnomalies::GetModel() const {
	PJsonVal ParamVal = TJsonVal::NewObj();
	ParamVal->AddToObj("threshold", Model.GetThreshold(0));
	return ParamVal;
}

void TNodeJsNNAnomalies::Save(TSOut& SOut) {
	Model.Save(SOut);
}

void TNodeJsNNAnomalies::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1, "NearestNeighborAD.setParams: expects 1 argument!");
	// get the arguments
	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	JsModel->SetParams(ParamVal);
	// return self
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsNNAnomalies::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// prepare parameters
	PJsonVal ParamVal = JsModel->GetParams();
	// return self
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsNNAnomalies::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1, "NearestNeighborAD.save: expects 1 argument!");
	// get the arguments
	TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
	// save model
	JsModel->Save(*JsFOut->SOut);
	// return fout
	Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsNNAnomalies::getModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// prepare parameters
	PJsonVal ParamVal = JsModel->GetModel();
	// return self
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsNNAnomalies::partialFit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() <= 2, "NearestNeighborAD.partialFit: expects at 1 or 2 arguments!");
	// get the arguments
	TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
	
	int RecId = TNodeJsUtil::GetArgInt32(Args, 1, -1);
	JsModel->Model.PartialFit(SpVec->Vec, RecId);
	
	// return self
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsNNAnomalies::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1
		|| ((Args.Length() == 2) && (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 1))),
		"NearestNeighborAD.fit: expects 1 or 2 arguments, the second optional argument must be of type IntVector from la submodule!");
	
	// get the arguments
	TNodeJsSpMat* SpMat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
	// fit model

	if (Args.Length() == 2) {
		TNodeJsIntV* IntVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsIntV>(Args, 1);
		EAssertR(SpMat->Mat.Len() == IntVec->Vec.Len(), "NearestNeighborAD.fit: number of columns of args[0] must match the number of elements of args[1]");
		for (int elN = 0; elN < SpMat->Mat.Len(); elN++) {
			JsModel->Model.PartialFit(SpMat->Mat[elN], IntVec->Vec[elN]);
		}
	} else {
		for (int elN = 0; elN < SpMat->Mat.Len(); elN++) {
			JsModel->Model.PartialFit(SpMat->Mat[elN]);
		}
	}

	// return self
	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsNNAnomalies::decisionFunction(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1, "NearestNeighborAD.predict: expects 1 argument!");
	// get the arguments
	TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
	const double Result = JsModel->Model.DecisionFunction(SpVec->Vec);
	// return result
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsNNAnomalies::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1, "NearestNeighborAD.predict: expects 1 argument!");
	// get the arguments
	TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
	const int Result = JsModel->Model.Predict(SpVec->Vec);
	// return result
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsNNAnomalies::explain(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Args.Holder());
	// check arguments
	EAssertR(Args.Length() == 1, "NearestNeighborAD.predict: expects 1 argument!");
	// get the arguments
	TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
	PJsonVal Explanation = JsModel->Model.Explain(SpVec->Vec);
	// return result
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Explanation));
}

void TNodeJsNNAnomalies::init(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// unwrap
	TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Info.Holder());	
	Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Model.IsInit()));
}


////////////////////////////////////////////////
// QMiner-NodeJS-Recursive-Linear-Regression
TNodeJsRecLinReg::TNodeJsRecLinReg(const TSignalProc::PRecLinReg& _Model):
		Model(_Model) {}


void TNodeJsRecLinReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsRecLinReg>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "partialFit", _partialFit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);

	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}

TNodeJsRecLinReg* TNodeJsRecLinReg::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
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

	// get feature matrix
	EAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId()),
		"RecLinRegModel.fit: The first argument must be a JsTFltVV (js linalg full matrix)");
	EAssertR(TNodeJsUtil::IsArgWrapObj(Args, 1, TNodeJsFltV::GetClassId()), "Argument 1 should be a full vector!");

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	TNodeJsFltVV* JsFeatMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* TargetVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

	// make sure dimensions of matrix and vector math
	EAssertR(JsFeatMat->Mat.GetCols() == TargetVec->Vec.Len(),
		"RecLinRegModel.fit: passed matrix dimension != passed argument dimension");

	// make sure dimensions match
	EAssertR(Model->Model->GetDim() == JsFeatMat->Mat.GetRows(),
		"RecLinRegModel.fit: model dimension != passed argument dimension");

	// learn
	TFltV Col;
	for (int i = 0; i < JsFeatMat->Mat.GetRows(); i++) {
		JsFeatMat->Mat.GetCol(i, Col);
		Model->Model->Learn(Col, TargetVec->Vec[i]);
	}

	EAssertR(!Model->Model->HasNaN(), "RecLinRegModel.fit: NaN detected!");

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecLinReg::partialFit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	EAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.partialFit: The first argument must be a JsTFltV (js linalg full vector)");
	EAssertR(TNodeJsUtil::IsArgFlt(Args, 1), "Argument 1 should be float!");

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	TNodeJsFltV* JsFeatVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
	const double Target = TNodeJsUtil::GetArgFlt(Args, 1);

	// make sure dimensions match
	EAssertR(Model->Model->GetDim() == JsFeatVec->Vec.Len(),
		"RecLinRegModel.partialFit: model dimension != passed argument dimension");

	// learn
	Model->Model->Learn(JsFeatVec->Vec, Target);
	EAssertR(!Model->Model->HasNaN(), "RecLinRegModel.partialFit: NaN detected!");

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecLinReg::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	EAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId()),
		"RecLinRegModel.learn: The first argument must be a JsTFltV (js linalg full vector)");

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	TNodeJsFltV* JsFeatVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

	EAssertR(Model->Model->GetDim() == JsFeatVec->Vec.Len(),
		"RecLinRegModel.learn: model dimension != sample dimension");

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Model->Model->Predict(JsFeatVec->Vec)));
}

void TNodeJsRecLinReg::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Model->GetParams()));
}

void TNodeJsRecLinReg::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Constructor expects 1 argument!");
	EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "RecLinReg.setParams: first argument should be a Javascript object!");
	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

	TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Args.Holder());

	const int Dim = ParamVal->GetObjInt("dim", Model->Model->GetDim());
	const double RegFact = ParamVal->GetObjNum("regFact", Model->Model->GetRegFact());
	const double ForgetFact = ParamVal->GetObjNum("forgetFact", Model->Model->GetForgetFact());

	// copy the values
	Model->Model->setForgetFact(ForgetFact);
	Model->Model->setRegFact(RegFact);
	Model->Model->setDim(Dim);

	Args.GetReturnValue().Set(Args.Holder());
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

	EAssertR(Args.Length() == 1, "Should have 1 argument!");
	PSOut SOut;
	if (TNodeJsUtil::IsArgStr(Args, 0)) {
		SOut = TFOut::New(TNodeJsUtil::GetArgStr(Args, 0), false);
	}
	else {
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
		SOut = JsFOut->SOut;
	}

	Model->Model->Save(*SOut);

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

///////////////////////////////
////// code below not yet ported or verified for scikit
///////////////////////////////

////////////////////////////////////////////////////////
// Logistic regression model
void TNodeJsLogReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsLogReg>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}

TNodeJsLogReg* TNodeJsLogReg::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() < 2, "new LogReg: expecting 0 or 1 parameter!");

	try {
		if (Args.Length() > 0 && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
			// load the model from the input stream
			TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
			return new TNodeJsLogReg(*JsFIn->SIn);
		}
		else if ((Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) || Args.Length() == 0)  {
			// parse the argumentts
			PJsonVal ArgJson = Args.Length() > 0 ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();

			const double Lambda = ArgJson->IsObjKey("lambda") ? ArgJson->GetObjNum("lambda") : 1;
			const bool IncludeIntercept = ArgJson->IsObjKey("intercept") ? ArgJson->GetObjBool("intercept") : false;

			return new TNodeJsLogReg(TClassification::TLogReg(Lambda, IncludeIntercept));
		}
		else {
			throw TExcept::New("new LogReg: wrong arguments in constructor!");
		}
	}
	catch (const PExcept& Except) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, TStr("[addon] Exception: " + Except->GetMsgStr()).CStr())));
		return nullptr;
	}
}

void TNodeJsLogReg::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());
	PJsonVal ParamVal = TJsonVal::NewObj();

	ParamVal->AddToObj("lambda", JsModel->LogReg.GetLambda());
	ParamVal->AddToObj("intercept", JsModel->LogReg.GetIntercept());

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsLogReg::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Constructor expects 1 argument!");
	EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "LogReg.setParams: first argument should be a Javascript object!");

	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());

	if (ParamVal->IsObjKey("lambda")) { JsModel->LogReg.SetLambda(ParamVal->GetObjNum("lambda")); }
	if (ParamVal->IsObjKey("intercept")) { JsModel->LogReg.SetIntercept(ParamVal->GetObjBool("intercept")); }

	Args.GetReturnValue().Set(Args.Holder());
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
	}
	else {
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
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsLogReg::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "logreg.save: expects 1 argument!");

	TNodeJsLogReg* JsModel = ObjectWrap::Unwrap<TNodeJsLogReg>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->LogReg.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(Args[0]);
}

////////////////////////////////////////////////////////
// Proportional Hazards Model
void TNodeJsPropHaz::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsPropHaz>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}

TNodeJsPropHaz* TNodeJsPropHaz::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() > 0 && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
		return new TNodeJsPropHaz(TRegression::TPropHazards(*JsFIn->SIn));
	}
	else if (Args.Length() == 0 || TNodeJsUtil::IsArgObj(Args, 0)) {
		// parse the arguments
		PJsonVal ArgJson = Args.Length() > 0 ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();
		const double Lambda = ArgJson->IsObjKey("lambda") ? ArgJson->GetObjNum("lambda") : 0;
		return new TNodeJsPropHaz(TRegression::TPropHazards(Lambda));
	}
	else {
		throw TExcept::New("new PropHazards: wrong arguments in constructor!");
	}
}

void TNodeJsPropHaz::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsPropHaz* JsModel = ObjectWrap::Unwrap<TNodeJsPropHaz>(Args.Holder());
	PJsonVal ParamVal = TJsonVal::NewObj();

	ParamVal->AddToObj("lambda", JsModel->Model.GetLambda());

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsPropHaz::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Constructor expects 1 argument!");
	EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "PropHaz.setParams: first argument should be a Javascript object!");

	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	TNodeJsPropHaz* JsModel = ObjectWrap::Unwrap<TNodeJsPropHaz>(Args.Holder());

	if (ParamVal->IsObjKey("lambda")) { JsModel->Model.SetLambda(ParamVal->GetObjNum("lambda")); }

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsPropHaz::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "expreg.fit: expects at least 2 arguments!");

	TNodeJsPropHaz* JsModel = ObjectWrap::Unwrap<TNodeJsPropHaz>(Args.Holder());

	// get the arguments
	TNodeJsFltVV* InstanceMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	TNodeJsFltV* ResponseJsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

	if (Args.Length() > 2) {
		const double ConvergEps = TNodeJsUtil::GetArgFlt(Args, 2);
		JsModel->Model.Fit(InstanceMat->Mat, ResponseJsV->Vec, ConvergEps);
	}
	else {
		JsModel->Model.Fit(InstanceMat->Mat, ResponseJsV->Vec);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsPropHaz::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "expreg.predict: expects 1 argument!");

	TNodeJsPropHaz* JsModel = ObjectWrap::Unwrap<TNodeJsPropHaz>(Args.Holder());

	// get the arguments
	TNodeJsFltV* JsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

	const double Result = JsModel->Model.Predict(JsFtrV->Vec);

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsPropHaz::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsPropHaz* JsExpReg = ObjectWrap::Unwrap<TNodeJsPropHaz>(Info.Holder());

		if (JsExpReg != nullptr) {
			TFltV WgtV;	JsExpReg->Model.GetWgtV(WgtV);
			Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
		}
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
	}
}

void TNodeJsPropHaz::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "expreg.save: expects 1 argument!");

	TNodeJsPropHaz* JsModel = ObjectWrap::Unwrap<TNodeJsPropHaz>(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

	JsModel->Model.Save(*JsFOut->SOut);

	Args.GetReturnValue().Set(Args[0]);
}


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
	NODE_SET_PROTOTYPE_METHOD(tpl, "probsAtTime", _probsAtTime);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histStates", _histStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "currState", _currState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fullCoords", _fullCoords);
	NODE_SET_PROTOTYPE_METHOD(tpl, "histogram", _histogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "transitionHistogram", _transitionHistogram);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getFtrBounds", _getFtrBounds);
	NODE_SET_PROTOTYPE_METHOD(tpl, "stateIds", _stateIds);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateWgtV", _getStateWgtV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getClassifyTree", _getClassifyTree);
	NODE_SET_PROTOTYPE_METHOD(tpl, "explainState", _explainState);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTransitionModel", _getTransitionModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onStateChanged", _onStateChanged);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onAnomaly", _onAnomaly);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onOutlier", _onOutlier);
	NODE_SET_PROTOTYPE_METHOD(tpl, "onPrediction", _onPrediction);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHierarchy", _rebuildHierarchy);
	NODE_SET_PROTOTYPE_METHOD(tpl, "rebuildHistograms", _rebuildHistograms);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getStateName", _getStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setStateName", _setStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "clearStateName", _clearStateName);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isTarget", _isTarget);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setTarget", _setTarget);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isLeaf", _isLeaf);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTimeUnit", _getTimeUnit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setControlVal", _setControlVal);
	NODE_SET_PROTOTYPE_METHOD(tpl, "resetControlVal", _resetControlVal);
	NODE_SET_PROTOTYPE_METHOD(tpl, "isAnyControlFtrSet", _isAnyControlFtrSet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParam", _getParam);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()),
		tpl->GetFunction());
}

TNodeJsStreamStory::TNodeJsStreamStory(TMc::TStreamStory* _StreamStory):
		StreamStory(_StreamStory) {
	InitCallbacks();
}

TNodeJsStreamStory::TNodeJsStreamStory(PSIn& SIn):
		StreamStory(new TMc::TStreamStory(*SIn)) {
	InitCallbacks();
}

TNodeJsStreamStory::~TNodeJsStreamStory() {
	StateChangedCallback.Reset();
	AnomalyCallback.Reset();
	OutlierCallback.Reset();
	PredictionCallback.Reset();
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

		const PJsonVal TransitionJson = ParamVal->GetObjKey("transitions");
		const PJsonVal ClustJson = ParamVal->GetObjKey("clustering");

		// transition modelling
		const uint64 TimeUnit = GetTmUnit(TransitionJson->GetObjStr("timeUnit"));
		const double DeltaTm = TransitionJson->IsObjKey("deltaTime") ?
				TransitionJson->GetObjNum("deltaTime") : DEFAULT_DELTA_TM;

		// clustering
		const double Sample = ClustJson->IsObjKey("sample") ? ClustJson->GetObjNum("sample") : 1;
		const int NHistBins = ClustJson->IsObjKey("histogramBins") ? ClustJson->GetObjInt("histogramBins") : 20;

		TMc::TStateIdentifier* StateIdentifier = new TMc::TStateIdentifier(GetClust(ClustJson, Rnd), NHistBins, Sample, Rnd, Verbose);
		TMc::TTransitionModeler* MChain = new TMc::TCtModeler(TimeUnit, DeltaTm, Verbose);
		TMc::THierarch* Hierarch = new TMc::THierarch(NPastStates + 1, Verbose);

		// finish
		return new TNodeJsStreamStory(new TMc::TStreamStory(StateIdentifier, MChain, Hierarch, Rnd, Verbose));
	}
}

TNodeJsStreamStory::TFitTask::TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args):
		TNodeTask(Args),
		JsStreamStory(nullptr),
		JsObservFtrs(nullptr),
		JsControlFtrs(nullptr),
		JsRecTmV(nullptr),
		JsBatchEndJsV(nullptr) {

	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "hmc.fit expects 2 arguments!");

	JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	v8::Local<v8::Object> ArgObj = Args[0]->ToObject();

	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "observations", TNodeJsFltVV::GetClassId()), "Missing field observations or invalid class!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "controls", TNodeJsFltVV::GetClassId()), "Missing field controls or invalid class!");
	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "times", TNodeJsFltV::GetClassId()), "Missing field times or invalid class!");

	JsObservFtrs = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "observations");
	JsControlFtrs = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "controls");
	JsRecTmV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltV>(ArgObj, "times");

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
			JsStreamStory->StreamStory->InitBatches(JsObservFtrs->Mat, JsControlFtrs->Mat, RecTmV, BatchEndV);
		} else {
			JsStreamStory->StreamStory->Init(JsObservFtrs->Mat, JsControlFtrs->Mat, RecTmV);
		}
	} catch (const PExcept& _Except) {
		SetExcept(_Except);
	}
}

//void TNodeJsStreamStory::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//	v8::HandleScope HandleScope(Isolate);
//
//	EAssertR(Args.Length() == 1, "hmc.fit expects 1 argument!");
//
//	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
//	v8::Local<v8::Object> ArgObj = Args[0]->ToObject();
//
//	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "observations", TNodeJsFltVV::GetClassId()), "Missing field observations or invalid class!");
//	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "controls", TNodeJsFltVV::GetClassId()), "Missing field controls or invalid class!");
//	EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "times", TNodeJsFltV::GetClassId()), "Missing field times or invalid class!");
//
//	TNodeJsFltVV* JsObservFtrs = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "observations");
//	TNodeJsFltVV* JsControlFtrs = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(ArgObj, "controls");
//	TNodeJsFltV* JsRecTmV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltV>(ArgObj, "times");
//
//	TUInt64V RecTmV;	TNodeJsUtil::GetCppTmMSecsV(JsRecTmV->Vec, RecTmV);
//
//	if (!TNodeJsUtil::IsFldNull(ArgObj, "batchV")) {
//		EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "batchV", TNodeJsBoolV::GetClassId()), "Invalid class of field batchV!");
//		const TNodeJsBoolV* BatchEndJsV = TNodeJsUtil::GetUnwrapFld<TNodeJsBoolV>(ArgObj, "batchV");
//		const TBoolV& BatchEndV = BatchEndJsV->Vec;
//		JsStreamStory->StreamStory->InitBatches(JsObservFtrs->Mat, JsControlFtrs->Mat, RecTmV, BatchEndV, false);
//	} else {
//		JsStreamStory->StreamStory->Init(JsObservFtrs->Mat, JsControlFtrs->Mat, RecTmV, false);
//	}
//
//	Args.GetReturnValue().Set(v8::Undefined(Isolate));
//}

//void TNodeJsStreamStory::fitAsync(const v8::FunctionCallbackInfo<v8::Value>& Args) {
//	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
//	v8::HandleScope HandleScope(Isolate);
//
////	TNodeJsAsyncUtil::ExecuteOnWorker(new TFitAsync(Args));
//	TFitAsync* Task = new TFitAsync(Args);
//	Task->ExtractCallback(Args);
//	TNodeJsAsyncUtil::ExecuteOnWorker(Task);
//
//	Args.GetReturnValue().Set(v8::Undefined(Isolate));
//}

void TNodeJsStreamStory::update(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 3, "hmc.update: expects 3 arguments!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	TNodeJsFltV* JsObsFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
	TNodeJsFltV* JsContrFtrV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());
	const uint64 RecTm = TNodeJsUtil::GetArgTmMSecs(Args, 2);//GetTmMSecs(Args[2]);

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

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
	const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);

	TIntFltPrV StateIdProbPrV;

	if (Args.Length() > 2 && !Args[2]->IsNull() && !Args[2]->IsUndefined()) {
		const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);
		JsMChain->StreamStory->GetPastStateProbV(Level, StartState, Tm, StateIdProbPrV);
	}
	else {
		JsMChain->StreamStory->GetPrevStateProbV(Level, StartState, StateIdProbPrV);
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

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const double Level = TNodeJsUtil::GetArgFlt(Args, 0);

	TIntV StateIdV;	JsMChain->StreamStory->GetHistStateIdV(Level, StateIdV);

	v8::Local<v8::Array> StateArr = v8::Array::New(Isolate, StateIdV.Len());
	for (int i = 0; i < StateIdV.Len(); i++) {
		StateArr->Set(i, v8::Integer::New(Isolate, StateIdV[i]));
	}

	Args.GetReturnValue().Set(StateArr);
}

void TNodeJsStreamStory::toJSON(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsMChain->StreamStory->GetJson()));
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

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const bool ObsCoords = Args.Length() > 1 && TNodeJsUtil::IsArgBool(Args, 1) ? TNodeJsUtil::GetArgBool(Args, 1) : true;

	TFltV FtrV;	JsMChain->StreamStory->GetCentroid(StateId, FtrV, ObsCoords);

	v8::Local<v8::Array> FtrVJson = v8::Array::New(Isolate, FtrV.Len());
	for (int i = 0; i < FtrV.Len(); i++) {
		FtrVJson->Set(i, v8::Number::New(Isolate, FtrV[i]));
	}

	Args.GetReturnValue().Set(FtrVJson);
}

void TNodeJsStreamStory::histogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "hmc.histogram: expects 2 arguments!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const int FtrId = TNodeJsUtil::GetArgInt32(Args, 1);

	TFltV BinStartV, ProbV;
	JsStreamStory->StreamStory->GetHistogram(StateId, FtrId, BinStartV, ProbV);
	WrapHistogram(Args, BinStartV, ProbV);
}

void TNodeJsStreamStory::transitionHistogram(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 3, "ss.transitionHistogram: expects 3 arguments!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int SourceId = TNodeJsUtil::GetArgInt32(Args, 0);
	const int TargetId = TNodeJsUtil::GetArgInt32(Args, 1);
	const int FtrId = TNodeJsUtil::GetArgInt32(Args, 2);

	TFltV BinStartV, ProbV;

	JsStreamStory->StreamStory->GetTransitionHistogram(SourceId, TargetId, FtrId, BinStartV, ProbV);
	WrapHistogram(Args, BinStartV, ProbV);
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

	EAssertR(Args.Length() == 1, "hmc.stateIds: expects 1 argument!");

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

void TNodeJsStreamStory::getStateWgtV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "hmc.getStateWgtV: expects 1 argument!");

	TNodeJsStreamStory* JsMChain = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);

	TFltV WgtV;
	JsMChain->StreamStory->GetStateWgtV(StateId, WgtV);

	v8::Local<v8::Array> JsWgtV = v8::Array::New(Isolate, WgtV.Len());
	for (int i = 0; i < WgtV.Len(); i++) {
		JsWgtV->Set(i, v8::Number::New(Isolate, WgtV[i]));
	}

	Args.GetReturnValue().Set(JsWgtV);
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

void TNodeJsStreamStory::rebuildHierarchy(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());
	JsStreamStory->StreamStory->InitHierarch();

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
	TNodeJsFltV* JsRecTmV = TNodeJsUtil::GetUnwrapFld<TNodeJsFltV>(ArgObj, "times");

	TUInt64V RecTmV(JsRecTmV->Vec.Len(), 0);
	for (int i = 0; i < JsRecTmV->Vec.Len(); i++) {
		RecTmV.Add(TNodeJsUtil::GetCppTimestamp((uint64)JsRecTmV->Vec[i]));
	}

	if (TNodeJsUtil::IsObjFld(ArgObj, "batchV")) {
		EAssertR(TNodeJsUtil::IsFldClass(ArgObj, "batchV", TNodeJsBoolV::GetClassId()), "Invalid class of field batchV!");
		const TNodeJsBoolV* BatchEndJsV = TNodeJsUtil::GetUnwrapFld<TNodeJsBoolV>(ArgObj, "batchV");
		const TBoolV& BatchEndV = BatchEndJsV->Vec;
		JsStreamStory->StreamStory->InitHistograms(JsObsFtrVV->Mat, JsControlFtrVV->Mat, RecTmV, BatchEndV);
	} else {
		JsStreamStory->StreamStory->InitHistograms(JsObsFtrVV->Mat, JsControlFtrVV->Mat, RecTmV, TBoolV());
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
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

	EAssertR(Args.Length() == 2, "hmc.setStateName: expects 2 arguments!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const double Height = TNodeJsUtil::GetArgFlt(Args, 1);

	bool Result = JsStreamStory->StreamStory->IsTargetState(StateId, Height);
	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, Result));
}

void TNodeJsStreamStory::setTarget(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 3, "hmc.setStateName: expects 2 arguments!");

	TNodeJsStreamStory* JsStreamStory = ObjectWrap::Unwrap<TNodeJsStreamStory>(Args.Holder());

	const int StateId = TNodeJsUtil::GetArgInt32(Args, 0);
	const double Height = TNodeJsUtil::GetArgFlt(Args, 1);
	const bool IsTarget = TNodeJsUtil::GetArgBool(Args, 2);

	JsStreamStory->StreamStory->SetTargetState(StateId, Height, IsTarget);

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

	if (TimeUnit == TMc::TCtModeler::TU_SECOND) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "second"));
	} else if (TimeUnit == TMc::TCtModeler::TU_MINUTE) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "minute"));
	} else if (TimeUnit == TMc::TCtModeler::TU_HOUR) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "hour"));
	} else if (TimeUnit == TMc::TCtModeler::TU_DAY) {
		Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, "day"));
	} else if (TimeUnit == TMc::TCtModeler::TU_MONTH) {
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

void TNodeJsStreamStory::OnStateChanged(const TIntFltPrV& StateIdHeightV) {
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
			v8::Date::New(Isolate, (double)TTm::GetUnixMSecsFromWinMSecs(RecTm)),
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

void TNodeJsStreamStory::WrapHistogram(const v8::FunctionCallbackInfo<v8::Value>& Args,
		const TFltV& BinStartV, const TFltV& ProbV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

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

uint64 TNodeJsStreamStory::GetTmUnit(const TStr& TimeUnitStr) {
	if (TimeUnitStr == "second") {
		return TMc::TCtModeler::TU_SECOND;
	} else if (TimeUnitStr == "minute") {
		return TMc::TCtModeler::TU_MINUTE;
	} else if (TimeUnitStr == "hour") {
		return TMc::TCtModeler::TU_HOUR;
	} else if (TimeUnitStr == "day") {
		return TMc::TCtModeler::TU_DAY;
	} else if (TimeUnitStr == "month") {
		return TMc::TCtModeler::TU_MONTH;
	} else {
		throw TExcept::New("Invalid time unit: " + TimeUnitStr, "TNodeJsStreamStory::GetTmUnit");
	}
}

TClustering::PDnsKMeans TNodeJsStreamStory::GetClust(const PJsonVal& ParamJson,
		const TRnd& Rnd) {
	const TStr& ClustAlg = ParamJson->GetObjStr("type");
	if (ClustAlg == "dpmeans") {
		const double Lambda = ParamJson->GetObjNum("lambda");
		const int MinClusts = ParamJson->IsObjKey("minClusts") ? ParamJson->GetObjInt("minClusts") : 1;
		const int MxClusts = ParamJson->IsObjKey("maxClusts") ? ParamJson->GetObjInt("maxClusts") : TInt::Mx;

		return new TClustering::TDpMeans(Lambda, MinClusts, MxClusts, Rnd);
	} else if (ClustAlg == "kmeans") {
		const int K = ParamJson->GetObjInt("k");
		return new TClustering::TDnsKMeans(K, Rnd);
	} else {
		throw TExcept::New("Invalivalid clustering type: " + ClustAlg, "TJsHierCtmc::TJsHierCtmc");
	}
}


////////////////////////////////////////////////////////
// Neural Network model
TNodeJsNNet::TNodeJsNNet(const PJsonVal& ParamVal) {
	TIntV LayoutV;
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
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}

}

TNodeJsNNet::TNodeJsNNet(TSIn& SIn) {
	try {
		Model = TSignalProc::TNNet::Load(SIn);
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), Except->GetLocStr());
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
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

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

	EAssertR(Args.Length() <= 1, "Expected at most one argument.");

	if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFIn::GetClassId())) {
		// load the model from an input stream
		// currently not used, will be implemented
		TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
		return new TNodeJsNNet(*JsFIn->SIn);
	}
	else if (Args.Length() == 0 || TNodeJsUtil::IsArgObj(Args, 0)) { // got JSON
		PJsonVal ParamVal = (Args.Length() > 0) ? TNodeJsUtil::GetArgJson(Args, 0) : TJsonVal::NewObj();
		return new TNodeJsNNet(ParamVal);
	}
	else {
		throw TExcept::New("new NNet: wrong arguments in constructor!");
	}
}

void TNodeJsNNet::fit(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2, "NNet.fit: missing argument");

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());
		if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId())) {

			TNodeJsFltV* JsVecIn = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());
			TNodeJsFltV* JsVecTarget = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject());

			// first get output values
			Model->Model->FeedFwd(JsVecIn->Vec);

			// then check how we performed and learn
			Model->Model->BackProp(JsVecTarget->Vec);
		}
		else if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId())){
			TNodeJsFltVV* JsVVecIn = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
			TNodeJsFltVV* JsVVecTarget = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[1]->ToObject());

			EAssertR(JsVVecIn->Mat.GetCols() == JsVVecTarget->Mat.GetCols(), "NNet.fit: Column dimension not equal!");

			for (int ColN = 0; ColN < JsVVecIn->Mat.GetCols(); ColN++){
				TFltV InFltV;
				JsVVecIn->Mat.GetCol(ColN, InFltV);
				Model->Model->FeedFwd(InFltV);
				// then check how we performed and learn
				TFltV TargFltV;
				JsVVecTarget->Mat.GetCol(ColN, TargFltV);
				if (ColN == JsVVecIn->Mat.GetCols() - 1){
					Model->Model->BackProp(TargFltV);
				}
				else {
					Model->Model->BackProp(TargFltV, false);
				}
			}
		}
		else {
			// TODO: throw an error
			printf("NeuralNetwork.fit: The arguments must be a JsTFltV or JsTFltVV (js linalg full vector or matrix)");
		}
		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsNNet::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() > 0, "NNet.predict: missing argument");

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());

		EAssertR(TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId()),
			"NNet.predict: The first argument must be a JsTFltV (js linalg full vector)");
		TNodeJsFltV* JsVec = ObjectWrap::Unwrap<TNodeJsFltV>(Args[0]->ToObject());

		Model->Model->FeedFwd(JsVec->Vec);

		TFltV FltV;
		Model->Model->GetResults(FltV);

		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(FltV));

	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsNNet::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "NNet.getParams: Expects no arguments!");

	TNodeJsNNet* JsModel = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());
	PJsonVal ParamVal = TJsonVal::NewObj();

	PJsonVal LayoutArr = TJsonVal::NewArr();
	TIntV LayoutV; JsModel->Model->GetLayout(LayoutV);
	for (int i = 0; i < LayoutV.Len(); i++) {
		LayoutArr->AddToArr(LayoutV[i]);
	}

	ParamVal->AddToObj("layout", LayoutArr);
	ParamVal->AddToObj("learnRate", JsModel->Model->GetLearnRate());
	ParamVal->AddToObj("momentum", JsModel->Model->GetMomentum());
	ParamVal->AddToObj("tFuncHidden", JsModel->Model->GetTFuncHidden());
	ParamVal->AddToObj("tFuncOut", JsModel->Model->GetTFuncOut());

	Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, ParamVal));
}

void TNodeJsNNet::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "NNet.setParams: Expects one argument!");

	TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());
	const PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

	TIntV LayoutV;
	TSignalProc::TTFunc TFuncHiddenL;
	TSignalProc::TTFunc TFuncOutL;

	if (ParamVal->IsObjKey("learnRate")) {
		Model->Model->SetLearnRate(ParamVal->GetObjNum("learnRate"));
	}
	if (ParamVal->IsObjKey("momentum")) {
		Model->Model->SetMomentum(ParamVal->GetObjNum("momentum"));
	}
	if (ParamVal->IsObjKey("layout")) {
		ParamVal->GetObjIntV("layout", LayoutV);
	}
	else {
		Model->Model->GetLayout(LayoutV);
	}
	TFuncHiddenL = Model->ExtractFuncFromString(ParamVal->GetObjStr("tFuncHidden", Model->Model->GetTFuncHidden()));
	TFuncOutL = Model->ExtractFuncFromString(ParamVal->GetObjStr("tFuncOut", Model->Model->GetTFuncOut()));

	if (ParamVal->IsObjKey("layout") || ParamVal->IsObjKey("tFuncHidden") || ParamVal->IsObjKey("TFuncOut")) {
		Model->Model = TSignalProc::TNNet::New(LayoutV, Model->Model->GetLearnRate(), Model->Model->GetMomentum(),
			TFuncHiddenL, TFuncOutL);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsNNet::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsNNet* Model = ObjectWrap::Unwrap<TNodeJsNNet>(Args.Holder());

		EAssertR(Args.Length() == 1, "Should have 1 argument!");
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		PSOut SOut = JsFOut->SOut;

		Model->Model->Save(*SOut);

		Args.GetReturnValue().Set(Args[0]);
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "TNodeJsNNet::save");
	}
}


TSignalProc::TTFunc TNodeJsNNet::ExtractFuncFromString(const TStr& FuncString) {
	TSignalProc::TTFunc TFunc;

	if (FuncString == "tanHyper") {
		TFunc = TSignalProc::TTFunc::tanHyper;
	}
	else if (FuncString == "sigmoid") {
		TFunc = TSignalProc::TTFunc::sigmoid;
	}
	else if (FuncString == "fastTanh") {
		TFunc = TSignalProc::TTFunc::fastTanh;
	}
	else if (FuncString == "softPlus") {
		TFunc = TSignalProc::TTFunc::softPlus;
	}
	else if (FuncString == "fastSigmoid") {
		TFunc = TSignalProc::TTFunc::fastSigmoid;
	}
	else if (FuncString == "linear") {
		TFunc = TSignalProc::TTFunc::linear;
	}
	else {
		throw TExcept::New("Unknown transfer function type " + FuncString);
	}

	return TFunc;
}

///////////////////////////////
// QMiner-JavaScript-Tokenizer

void TNodeJsTokenizer::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsTokenizer>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getTokens", _getTokens);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getSentences", _getSentences);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParagraphs", _getParagraphs);

	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsTokenizer* TNodeJsTokenizer::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	// parse arguments
	PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
	EAssertR(ParamVal->IsObjKey("type"),
		"Missing tokenizer type " + ParamVal->SaveStr());
	const TStr& TypeNm = ParamVal->GetObjStr("type");
	// create
	PTokenizer Tokenizer = TTokenizer::New(TypeNm, ParamVal);
	return new TNodeJsTokenizer(Tokenizer);
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

/////////////////////////////////////////////
// Multidimensional Scaling

TNodeJsMDS::TNodeJsMDS(const PJsonVal& ParamVal) :
MxStep(5000),
MxSecs(500),
MnDiff(1e-4),
DistType(TVizDistType::vdtEucl) {
	UpdateParams(ParamVal);
}

TNodeJsMDS::TNodeJsMDS(TSIn& SIn) :
MxStep(TInt(SIn)),
MxSecs(TInt(SIn)),
MnDiff(TFlt(SIn)),
DistType(LoadEnum<TVizDistType>(SIn)) {}

void TNodeJsMDS::UpdateParams(const PJsonVal& ParamVal) {
	if (ParamVal->IsObjKey("maxStep")) MxStep = ParamVal->GetObjNum("maxStep");
	if (ParamVal->IsObjKey("maxSecs")) MxSecs = ParamVal->GetObjNum("maxSecs");
	if (ParamVal->IsObjKey("minDiff")) MnDiff = ParamVal->GetObjNum("minDiff");
	if (ParamVal->IsObjKey("distType")) { 
		TStr Type = ParamVal->GetObjStr("distType"); 
		if (Type == "Euclid") {
			DistType = TVizDistType::vdtEucl;
		} else if (Type == "Cos") {
			DistType = TVizDistType::vdtCos;
		} else if (Type == "SqrtCos") {
			DistType = TVizDistType::vdtSqrtCos;
		} else {
			throw TExcept::New("MDS: unsupported distance type!");
		}
	}
}

PJsonVal TNodeJsMDS::GetParams() const {
	PJsonVal ParamVal = TJsonVal::NewObj();

	ParamVal->AddToObj("maxStep", MxStep);
	ParamVal->AddToObj("maxSecs", MxSecs);
	ParamVal->AddToObj("minDiff", MnDiff);
	switch (DistType) {
	case (vdtEucl) :
		ParamVal->AddToObj("distType", "Euclid"); break;
	case(vdtCos) :
		ParamVal->AddToObj("distType", "Cos"); break;
	case(vdtSqrtCos) :
		ParamVal->AddToObj("distType", "SqrtCos"); break;
	}
	return ParamVal;
}

void TNodeJsMDS::Save(TSOut& SOut) const {
	TInt(MxStep).Save(SOut);
	TInt(MxSecs).Save(SOut);
	TFlt(MnDiff).Save(SOut);
	SaveEnum<TVizDistType>(SOut, DistType);
}

void TNodeJsMDS::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsMDS>);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
	NODE_SET_PROTOTYPE_METHOD(tpl, "fitTransform", _fitTransform);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsMDS* TNodeJsMDS::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	if (Args.Length() == 0) {
		// create new model with default parameters
		return new TNodeJsMDS(TJsonVal::NewObj());
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
		// load the model from the input stream
		TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
		return new TNodeJsMDS(*JsFIn->SIn);
	}
	else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
		// create new model from given parameters
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
		return new TNodeJsMDS(ParamVal);
	}
	else {
		throw TExcept::New("new MDS: wrong arguments in constructor!");
	}
}

void TNodeJsMDS::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 0, "MDS.getParams: takes 0 argument!");

	try {
		TNodeJsMDS* JsMDS = TNodeJsMDS::Unwrap<TNodeJsMDS>(Args.Holder());
		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsMDS->GetParams()));
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "MDS::getParams");
	}
}

void TNodeJsMDS::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "MDS.setParams: takes 1 argument!");
	EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "MDS.setParams: first argument should be a Javascript object!");

	try {
		TNodeJsMDS* JsMDS = ObjectWrap::Unwrap<TNodeJsMDS>(Args.Holder());
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

		JsMDS->UpdateParams(ParamVal);

		Args.GetReturnValue().Set(Args.Holder());
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "MDS::setParams");
	}
}

void TNodeJsMDS::fitTransform(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "MDS.fitTransform: expecting 1 argument!");
	TNodeJsMDS* JsMDS = ObjectWrap::Unwrap<TNodeJsMDS>(Args.Holder());

	TVec<TFltV> Temp;
	TFltV DummyClsV;
	PSVMTrainSet TrainSet;
	// algorithm parameters
	int MxStep = JsMDS->MxStep;
	int MxSecs = JsMDS->MxSecs;
	int MnDiff = JsMDS->MnDiff;
	bool RndStartPos = true;

	PNotify Noty = TQm::TEnv::Logger;
	if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
		const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
		DummyClsV.Gen(Mat.GetCols());
		TrainSet = TRefDenseTrainSet::New(Mat, DummyClsV);
		TVizMapFactory::MakeFlat(TrainSet, TVizDistType::vdtEucl, Temp, MxStep, MxSecs, MnDiff, RndStartPos, Noty);
	}
	else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
		const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
		DummyClsV.Gen(Mat.Len());
		TrainSet = TRefSparseTrainSet::New(Mat, DummyClsV);
		TVizMapFactory::MakeFlat(TrainSet, TVizDistType::vdtEucl, Temp, MxStep, MxSecs, MnDiff, RndStartPos, Noty);
	}
	else {
		throw TExcept::New("MDS.fitTransform: argument not a sparse or dense matrix!");
	}
	
	TFltVV Result(Temp.Len(), Temp[0].Len());
	for (int RowN = 0; RowN < Temp.Len(); RowN++) {
		for (int ColN = 0; ColN < Temp[0].Len(); ColN++) {
			Result(RowN, ColN) = Temp[RowN][ColN];
		}
	}
	Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
}

void TNodeJsMDS::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "MDS.save: Should have 1 argument!");

	try {
		TNodeJsMDS* JsMDS = ObjectWrap::Unwrap<TNodeJsMDS>(Args.Holder());
		// get output stream from argumetns
		TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
		// save model
		JsMDS->Save(*JsFOut->SOut);
		// return output stream for convenience
		Args.GetReturnValue().Set(Args[0]);
	}
	catch (const PExcept& Except) {
		throw TExcept::New(Except->GetMsgStr(), "MDS::save");
	}
}

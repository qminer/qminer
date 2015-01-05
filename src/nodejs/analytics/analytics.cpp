#include "analytics.h"

//////////////////////////////////////////////////////
// NodeJS - analytics

////////////////////////////////////////////////////////
// Support Vector Machine
v8::Local<v8::Object> TNodeJsSvmModel::New(const TStr& Alg, const double& SvmCost) {
	// TODO
}

void TNodeJsSvmModel::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "svmClassify"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);

	constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsSvmModel::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() >= 2, "trainSvmClassify: missing arguments!");
	// get class information
	QmAssertR(Args[1]->IsObject(), "trainSvmClassify: second argument expected to be object");

	try {
		if (Args.IsConstructCall()) {
			// parse SVM parameters
			PJsonVal SvmParamVal = TJsonVal::NewObj();
			if (Args.Length() > 2 && TNodeJsUtil::IsArgJson(Args, 2)) {
				SvmParamVal = TNodeJsUtil::GetArgJson(Args, 2);
			}

			 // default algorithm is stochastic gradient descent
			const TStr Algorithm = SvmParamVal->GetObjStr("algorithm", "SGD");
			const double SvmCost = SvmParamVal->GetObjNum("c", 1.0);
			const double SvmUnbalance = SvmParamVal->GetObjNum("j", 1.0);
			const int SampleSize = (int)SvmParamVal->GetObjNum("batchSize", 1000);
			const int MxIter = SvmParamVal->GetObjInt("maxIterations", 10000);
			const int MxTime = (int)(1000 * SvmParamVal->GetObjNum("maxTime", 600));
			const double MnDiff = SvmParamVal->GetObjNum("minDiff", 1e-6);
			const bool Verbose = SvmParamVal->GetObjBool("verbose", false);
			PNotify Notify = Verbose ? TNotify::StdNotify : TNotify::NullNotify;

			QmAssertR(Algorithm == "SGD" || Algorithm == "PR_LOQO",
			        "trainSvmClassify: unknown algorithm " + Algorithm);

			Args.GetReturnValue().Set(TNodeJsSvmModel::New(Algorithm, SvmCost));
		} else {
			throw TQm::TQmExcept::New("Tried to create an object with a non-constructor call!", "TNodeJsHMChain::New");
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

		TFltV& ClsV = ObjectWrap::Unwrap<TNodeJsFltV>(Args[1]->ToObject())->Vec;
		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpMat::ClassId)) {
			TVec<TIntFltKdV>& VecV = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Args.GetReturnValue().Set(TNodeJsSvmLinModel::New(TSvm::SolveClassify<TVec<TIntFltKdV>>(VecV, TLAMisc::GetMaxDimIdx(VecV) + 1,
						VecV.Len(), ClsV, Model->SvmCost, Model->SvmUnbalance, Model->MxTime,
						Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify)));
			} else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefSparseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmCost, Model->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));

				Args.GetReturnValue().Set(TNodeJsSvmLinModel::New(TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh())));
			}
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)) {
			TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
			if (Model->Algorithm == "SGD") {
				Args.GetReturnValue().Set(TNodeJsSvmLinModel::New(TSvm::SolveClassify<TFltVV>(VecV, VecV.GetRows(),
						VecV.GetCols(), ClsV, Model->SvmCost, Model->SvmUnbalance, Model->MxTime,
						Model->MxIter, Model->MnDiff, Model->SampleSize, Model->Notify)));
			} else if (Model->Algorithm == "PR_LOQO") {
				PSVMTrainSet TrainSet = TRefDenseTrainSet::New(VecV, ClsV);
				PSVMModel SvmModel = TSVMModel::NewClsLinear(TrainSet, Model->SvmCost, Model->SvmUnbalance,
					TIntV(), TSVMLearnParam::Lin(Model->MxTime, Model->Verbose ? 2 : 0));
				Args.GetReturnValue().Set(TNodeJsSvmLinModel::New(TSvm::TLinModel(SvmModel->GetWgtV(), SvmModel->GetThresh())));
			}
		}
		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

////////////////////////////////////////////////////////
// Node JS Linear SVM model
void TNodeJsSvmLinModel::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "svmLinModel"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);

	constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsSvmLinModel::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() > 0, "svm.predict: missing argument");

	try {
		TNodeJsSvmLinModel* Model = ObjectWrap::Unwrap<TNodeJsSvmLinModel>(Args.Holder());

		if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::ClassId)) {
			TNodeJsVec<TFlt, TAuxFltV>* Vec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args[0]->ToObject());
			const double Res = Model->Model.Predict(Vec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpVec::ClassId)) {
			TNodeJsSpVec* SpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
			const double Res = Model->Model.Predict(SpVec->Vec);
			Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
		}
		else {
			throw TQm::TQmExcept::New("svm.predict: unsupported type of the first argument");
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsSvmLinModel::weights(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmLinModel* Model = ObjectWrap::Unwrap<TNodeJsSvmLinModel>(Info.Holder());

		// get feature vector
		TFltV WgtV; Model->Model.GetWgtV(WgtV);

		Info.GetReturnValue().Set(TNodeJsFltV::New(WgtV));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

void TNodeJsSvmLinModel::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsSvmLinModel* Model = ObjectWrap::Unwrap<TNodeJsSvmLinModel>(Args.Holder());

		QmAssertR(Args.Length() == 1, "Should have 1 argument!");
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		Model->Model.Save(*JsFOut->SOut);

		Args.GetReturnValue().Set(Args[0]);
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
	}
}

////////////////////////////////////////////////
// QMiner-NodeJS-Recursive-Linear-Regression
TNodeJsRecLinReg::TNodeJsRecLinReg(const TSignalProc::PRecLinReg& _Model):
		Model(_Model) {}

v8::Local<v8::Object> TNodeJsRecLinReg::New(const TSignalProc::PRecLinReg& Model) {
	return TNodeJsUtil::NewJsInstance(new TNodeJsRecLinReg(Model), constructor, v8::Isolate::GetCurrent());
}

void TNodeJsRecLinReg::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "recLinReg"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "learn", _learn);
	NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	// properties
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "weights"), _weights);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);

	constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsRecLinReg::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		if (Args.IsConstructCall()) {
			QmAssertR(Args.Length() == 1, "Constructor expects 1 argument!");

			if (TNodeJsUtil::IsArgJson(Args, 0)) {
				PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

				const int Dim = ParamVal->GetObjInt("dim");
				const double RegFact = ParamVal->GetObjNum("regFact", 1.0);
				const double ForgetFact = ParamVal->GetObjNum("forgetFact", 1.0);

				Args.GetReturnValue().Set(TNodeJsRecLinReg::New(TSignalProc::TRecLinReg::New(Dim, RegFact, ForgetFact)));
			} else {
				TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
				Args.GetReturnValue().Set(TNodeJsRecLinReg::New(TSignalProc::TRecLinReg::Load(*JsFIn->SIn)));
			}
		} else {
			throw TQm::TQmExcept::New("Tried to create an object with a non-constructor call!", "TNodeJsHMChain::New");
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsRecLinReg::learn(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// get feature vector
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::ClassId),
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
	QmAssertR(TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::ClassId),
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


////////////////////////////////////////////////////////
// Hierarchical Markov Chain model
TNodeJsHMChain::TNodeJsHMChain(const TMc::PHierarchCtmc& _McModel, const TQm::PFtrSpace& _FtrSpace):
		McModel(_McModel),
		FtrSpace(_FtrSpace) {}

TNodeJsHMChain::TNodeJsHMChain(const TQm::PBase Base, PSIn& SIn):
		McModel(TMc::THierarchCtmc::Load(*SIn)),
		FtrSpace(TQm::TFtrSpace::Load(Base, *SIn)) {}

v8::Local<v8::Object> TNodeJsHMChain::New(const PJsonVal& ParamVal, const TQm::PFtrSpace& FtrSpace) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	const PNotify Notify = TStdNotify::New();

	const TStr InStoreNm = ParamVal->GetObjStr("source");
	const TStr TimeFldNm = ParamVal->GetObjStr("timestamp");
	const PJsonVal TransitionJson = ParamVal->GetObjKey("transitions");
	const PJsonVal ClustJson = ParamVal->GetObjKey("clustering");
	const PJsonVal FldsJson = ParamVal->GetObjKey("fields");

	// transition modelling
	TMc::PMChain MChain;
	if (TransitionJson->GetObjStr("type") == "continuous") {
		const TStr TimeUnitStr = TransitionJson->GetObjStr("timeUnit");
		const double DeltaTm = TransitionJson->IsObjKey("deltaTime") ? TransitionJson->GetObjNum("deltaTime") : 1e-3;	// TODO hardcoded

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

		MChain = new TMc::TCtMChain(TimeUnit, DeltaTm, Notify);
	} else if (TransitionJson->GetObjStr("type") == "discrete") {
		MChain = new TMc::TDtMChain(Notify);
	}


	// clustering
	TMc::PClust Clust = NULL;

	const TStr ClustAlg = ClustJson->GetObjStr("type");
	if (ClustAlg == "dpmeans") {
		const double Lambda = ClustJson->GetObjNum("lambda");
		const int MinClusts = ClustJson->IsObjKey("minClusts") ? ClustJson->GetObjInt("minClusts") : 1;
		const int MxClusts = ClustJson->IsObjKey("maxClusts") ? ClustJson->GetObjInt("maxClusts") : TInt::Mx;
		const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
		Clust = new TMc::TDpMeans(Lambda, MinClusts, MxClusts, TRnd(RndSeed), Notify);
	} else if (ClustAlg == "kmeans") {
		const int K = ClustJson->GetObjInt("k");
		const int RndSeed = ClustJson->IsObjKey("rndseed") ? ClustJson->GetObjInt("rndseed") : 0;
		Clust = new TMc::TFullKMeans(K, TRnd(RndSeed), Notify);
	} else {
		throw TExcept::New("Invalivalid clustering type: " + ClustAlg, "TJsHierCtmc::TJsHierCtmc");
	}

	// create the model
	TMc::PHierarch AggClust = new TMc::THierarch(Notify);

	// finish
	TMc::PHierarchCtmc HMcModel = new TMc::THierarchCtmc(Clust, MChain, AggClust, Notify);
	return TNodeJsUtil::NewJsInstance(new TNodeJsHMChain(HMcModel, FtrSpace), constructor, Isolate, HandleScope);
}

v8::Local<v8::Object> TNodeJsHMChain::New(const TQm::PBase Base, PSIn& SIn) {
	return TNodeJsUtil::NewJsInstance(new TNodeJsHMChain(Base, SIn), constructor, v8::Isolate::GetCurrent());
}

void TNodeJsHMChain::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "hctmc"));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "init", _init);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toJSON", _toJSON);
	NODE_SET_PROTOTYPE_METHOD(tpl, "futureStates", _futureStates);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getTransitionModel", _getTransitionModel);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

	constructor.Reset(Isolate, tpl->GetFunction());
}

void TNodeJsHMChain::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		if (Args.IsConstructCall()) {
			QmAssertR(0 < Args.Length() && Args.Length() <= 2, "Constructor expects 2 of less arguments!");

			if (Args.Length() == 2) {
				PJsonVal ArgJson = TNodeJsUtil::GetArgJson(Args, 0);
				// TODO get feature space
			} else if (Args.Length() == 1) {
				TNodeJsBase* JsBase = ObjectWrap::Unwrap<TNodeJsBase>(Args[0]->ToObject());
				TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[1]->ToObject());
				Args.GetReturnValue().Set(TNodeJsHMChain::New(JsBase->Base, JsFIn->SIn));
			}
		} else {
			throw TQm::TQmExcept::New("Tried to create an object with a non-constructor call!", "TNodeJsHMChain::New");
		}
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), Except->GetLocStr());
	}
}

void TNodeJsHMChain::init(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		TNodeJsRecSet* JsRecSet = ObjectWrap::Unwrap<TNodeJsRecSet>(Args[0]->ToObject());

		JsMChain->InitModel(JsRecSet->RecSet);
		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::toJSON");
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

void TNodeJsHMChain::futureStates(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());

		const double Level = TNodeJsUtil::GetArgFlt(Args, 0);
		const int StartState = TNodeJsUtil::GetArgInt32(Args, 1);
		const double Tm = TNodeJsUtil::GetArgFlt(Args, 2);

		TFltV ProbV;	JsMChain->McModel->GetFutStateProbs(Level, StartState, Tm, ProbV);

		Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, TJsonVal::NewArr(ProbV)));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::futureStates");
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


void TNodeJsHMChain::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	QmAssertR(Args.Length() == 1, "Should have 1 argument!");

	try {
		TNodeJsHMChain* JsMChain = ObjectWrap::Unwrap<TNodeJsHMChain>(Args.Holder());
		TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());

		PSOut SOut = JsFOut->SOut;
		JsMChain->McModel->Save(*SOut);
		JsMChain->FtrSpace->Save(*SOut);

		Args.GetReturnValue().Set(v8::Undefined(Isolate));
	} catch (const PExcept& Except) {
		throw TQm::TQmExcept::New(Except->GetMsgStr(), "TNodeJsHMChain::save");
	}
}

void TNodeJsHMChain::InitModel(const TQm::PRecSet& RecSet) {
	// generate an instance matrix
	TFltVV InstanceVV;	FtrSpace->GetFullVV(RecSet, InstanceVV);

	// generate a time vector
	const int NRecs = RecSet->GetRecs();
	TUInt64V RecTmV(NRecs,0);

	for (int i = 0; i < NRecs; i++) {
		RecTmV.Add(GetRecTm(RecSet->GetRec(i)));
	}

	// initialize the model
	McModel->Init(InstanceVV, RecTmV);
}

uint64 TNodeJsHMChain::GetRecTm(const TQm::TRec& Rec) const {
	return Rec.GetFieldTmMSecs(Rec.GetStore()->GetFieldIdV(TQm::TFieldType::oftTm)[0]);
}

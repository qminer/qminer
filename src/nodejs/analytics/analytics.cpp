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

void TNodeJsAnalytics::Init(v8::Local<v8::Object> exports) {
    NODE_SET_METHOD(exports, "nmf", _nmf);
    NODE_SET_METHOD(exports, "nmfAsync", _nmfAsync);
}

TNodeJsAnalytics::TNMFTask::TNMFTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync) :
        TNodeTask(Args, IsAsync),
        JsFltVV(nullptr),
        JsSpVV(nullptr),
        U(nullptr),
        V(nullptr),
        Iter(10000),
        Tol(1e-6),
        Notify(TNotify::NullNotify()) {

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        JsFltVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        JsSpVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
    }
    else {
        throw TExcept::New("MDS.fitTransform: argument not a sparse or dense matrix!");
    }

    k = TNodeJsUtil::GetArgInt32(Args, 1);

    if (Args.Length() >= 3 && !TNodeJsUtil::IsArgFun(Args, 2)) {
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 2);
        Iter = ParamVal->GetObjInt("iter", 100);
        Tol = ParamVal->GetObjNum("tol", 1e-3);
        bool Verbose = ParamVal->GetObjBool("verbose", false);
        Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
    }

    U = new TNodeJsFltVV();
    V = new TNodeJsFltVV();
}

v8::Local<v8::Function> TNodeJsAnalytics::TNMFTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    if (TNodeJsUtil::IsArgFun(Args, 2)) {
        return TNodeJsUtil::GetArgFun(Args, 2);
    }
    else {
        return TNodeJsUtil::GetArgFun(Args, 3);
    }
}

void TNodeJsAnalytics::TNMFTask::Run() {
    try {
        TFltVV& URef = U->Mat;
        TFltVV& VRef = V->Mat;
        if (JsFltVV != nullptr) {
            TNmf::CFO(JsFltVV->Mat, k, URef, VRef, Iter, Tol, Notify);
        }
        else if (JsSpVV != nullptr) {
            TNmf::CFO(JsSpVV->Mat, k, URef, VRef, Iter, Tol, Notify);
        }
        else {
            throw TExcept::New("nmf: expects dense or sparse matrix!");
        }
    }
    catch (const PExcept& Except) {
        delete U;
        delete V;
        SetExcept(Except);
    }
}

v8::Local<v8::Value> TNodeJsAnalytics::TNMFTask::WrapResult() {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);

    v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result
    JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "U")), TNodeJsUtil::NewInstance(U));
    JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "V")), TNodeJsUtil::NewInstance(V));
    return HandleScope.Escape(JsObj);
}

////////////////////////////////////////////////////////
// Support Vector Machine

TNodeJsSvmModel::TNodeJsSvmModel(const PJsonVal& ParamVal):
        Algorithm("SGD"){
    if (Model) { delete Model; }
    Model = NULL;
    UpdateParams(ParamVal);
}

TNodeJsSvmModel::TNodeJsSvmModel(TSIn& SIn):
        Algorithm(SIn){
    if (Model) { delete Model; }
    if (Algorithm == "LIBSVM"){ Model = new TSvm::TLibSvmModel(); }
    else { Model = new TSvm::TLinModel(); }
    Model->Load(SIn);
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

        if (ParamVal->IsObjKey("algorithm")) { throw TExcept::New("svm.setParams: cannot safely change algorithm"); }
        else { JsModel->UpdateParams(ParamVal); }

        Args.GetReturnValue().Set(Args.Holder());
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "SVM::setParams");
    }
}

void TNodeJsSvmModel::weights(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Info.Holder());
        if (!JsModel->Model){
            throw TExcept::New("svm.weights: model is not initialized");
        }
        else{
            Info.GetReturnValue().Set(TNodeJsFltV::New(JsModel->Model->GetWgtV()));
        }
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::weights");
    }
}

void TNodeJsSvmModel::bias(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsSvmModel* JsModel = ObjectWrap::Unwrap<TNodeJsSvmModel>(Info.Holder());
        if (!JsModel->Model){
            throw TExcept::New("svm.weights: model is not initialized");
        }
        else{
            Info.GetReturnValue().Set(v8::Number::New(Isolate, JsModel->Model->GetBias()));
        }
    } catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "TNodeJsSvmModel::bias");
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
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
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

        if ((JsModel->Model->GetWgtV()).Len() == 0) {
            throw TExcept::New("svm.decisionFunction: fit was not called yet");
        }

        if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
            TNodeJsFltV* Vec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
            const double Res = JsModel->Model->Predict(Vec->Vec);
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpVec>(Args, 0)) {
            TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
            const double Res = JsModel->Model->Predict(SpVec->Vec);
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
            const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
            TFltV ResV(Mat.GetCols(), 0);
            for (int ColN = 0; ColN < Mat.GetCols(); ColN++) {
                ResV.Add(JsModel->Model->Predict(Mat, ColN));
            }
            Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
            const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
            TFltV ResV(Mat.Len(), 0);
            for (int ColN = 0; ColN < Mat.Len(); ColN++) {
                ResV.Add(JsModel->Model->Predict(Mat[ColN]));
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

        if ((JsModel->Model->GetWgtV()).Len() == 0) {
            throw TExcept::New("svm.predict: fit was not called yet");
        }

        if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
            TNodeJsFltV* Vec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
            const double Res = (JsModel->Model->Predict(Vec->Vec) > 0.0) ? 1.0 : -1.0;
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpVec>(Args, 0)) {
            TNodeJsSpVec* SpVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpVec>(Args, 0);
            const double Res = (JsModel->Model->Predict(SpVec->Vec) > 0.0) ? 1.0 : -1.0;
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
            const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
            TFltV ResV(Mat.GetCols(), 0);
            for (int ColN = 0; ColN < Mat.GetCols(); ColN++) {
                ResV.Add(JsModel->Model->Predict(Mat, ColN) > 0.0 ? 1.0 : -1.0);
            }
            Args.GetReturnValue().Set(TNodeJsFltV::New(ResV));
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
            const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
            TFltV ResV(Mat.Len(), 0);
            for (int ColN = 0; ColN < Mat.Len(); ColN++) {
                ResV.Add(JsModel->Model->Predict(Mat[ColN]) > 0.0 ? 1.0 : -1.0);
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
    if (ParamVal->IsObjKey("algorithm")) { Algorithm = ParamVal->GetObjStr("algorithm"); }
    if (!Model) {
        if (Algorithm == "LIBSVM") { Model = new TSvm::TLibSvmModel(); }
        else { Model = new TSvm::TLinModel(); }
    }
    Model->UpdateParams(ParamVal);
}

PJsonVal TNodeJsSvmModel::GetParams() const {
    PJsonVal ParamVal = Model->GetParams();
    ParamVal->AddToObj("algorithm", Algorithm);
    return ParamVal;
}

void TNodeJsSvmModel::Save(TSOut& SOut) const {
    Algorithm.Save(SOut);
    Model->Save(SOut);
}

void TNodeJsSvmModel::ClrModel() {
    delete Model;
    Model = NULL;
}

///////////////////////////////
// QMiner-JavaScript-Support-Vector-Classification

void TNodeJsSVC::Init(v8::Local<v8::Object> exports) {
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
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "bias"), _bias);

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
            if (JsModel->Algorithm == "SGD" || JsModel->Algorithm == "LIBSVM") {
                JsModel->Model->FitClassification(VecV, TLinAlgSearch::GetMaxDimIdx(VecV) + 1,
                    VecV.Len(), ClsV, TQm::TEnv::Debug, TQm::TEnv::Error);
            }
            else {
                throw TExcept::New("SVC.fit: unknown algorithm " + JsModel->Algorithm);
            }
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
            TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
            if (JsModel->Algorithm == "SGD" || JsModel->Algorithm == "LIBSVM") {
                JsModel->Model->FitClassification(VecV, VecV.GetRows(),
                    VecV.GetCols(), ClsV, TQm::TEnv::Debug, TQm::TEnv::Error);
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

void TNodeJsSVR::Init(v8::Local<v8::Object> exports) {
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
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "bias"), _bias);

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
            if (JsModel->Algorithm == "SGD" || JsModel->Algorithm == "LIBSVM") {
                JsModel->Model->FitRegression(VecV, TLinAlgSearch::GetMaxDimIdx(VecV) + 1,
                    VecV.Len(), ClsV, TQm::TEnv::Debug, TQm::TEnv::Error);
            }
            else {
                throw TExcept::New("SVR.fit: unknown algorithm " + JsModel->Algorithm);
            }
        }
        else if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
            TFltVV& VecV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject())->Mat;
            if (JsModel->Algorithm == "SGD" || JsModel->Algorithm == "LIBSVM") {
                JsModel->Model->FitRegression(VecV, VecV.GetRows(),
                    VecV.GetCols(), ClsV, TQm::TEnv::Debug, TQm::TEnv::Error);
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
void TNodeJsRidgeReg::Init(v8::Local<v8::Object> exports) {
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

void TNodeJsRidgeReg::weights(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
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
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    JsModel->Model.Save(*JsFOut->SOut);

    Args.GetReturnValue().Set(Args[0]);
}


/////////////////////////////////////////////
// Sigmoid
void TNodeJsSigmoid::Init(v8::Local<v8::Object> exports) {
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
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    JsModel->Sigmoid.Save(*JsFOut->SOut);

    Args.GetReturnValue().Set(Args[0]);
}


/////////////////////////////////////////////
// Nearest Neighbor Annomaly Detection
void TNodeJsNNAnomalies::Init(v8::Local<v8::Object> exports) {
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

TNodeJsNNAnomalies::TNodeJsNNAnomalies(const PJsonVal& ParamVal) {
    SetParams(ParamVal);
}

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
    // parse rate parameter(s)
    TFltV RateV;
    if (ParamVal->IsObjKey("rate")) {
        // check if we get single number or array of numbers
        if (ParamVal->GetObjKey("rate")->IsNum()) {
            // we have a number
            RateV.Add(ParamVal->GetObjNum("rate"));
        } else {
            // must be an array
            ParamVal->GetObjFltV("rate", RateV);
        }
    }
    // if empty, use 0.05
    if (RateV.Empty()) { RateV.Add(0.05); }
    // create model
    Model = TAnomalyDetection::TNearestNeighbor(RateV, ParamVal->GetObjInt("windowSize", 100));
}

PJsonVal TNodeJsNNAnomalies::GetParams() const {
    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("rate", TJsonVal::NewArr(Model.GetRateV()));
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
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
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

void TNodeJsNNAnomalies::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsNNAnomalies* JsModel = ObjectWrap::Unwrap<TNodeJsNNAnomalies>(Info.Holder());
    Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Model.IsInit()));
}


////////////////////////////////////////////////
// QMiner-NodeJS-Recursive-Linear-Regression
TNodeJsRecLinReg::TNodeJsRecLinReg(const TSignalProc::POnlineLinReg& _Model):
        Model(_Model) {}


void TNodeJsRecLinReg::Init(v8::Local<v8::Object> exports) {
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
        return new TNodeJsRecLinReg(TSignalProc::TOnlineLinReg::Load(*JsFIn->SIn));
    }
    else {
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

        const int Dim = ParamVal->GetObjInt("dim");
        const double RegFact = ParamVal->GetObjNum("regFact", 1.0);
        const double ForgetFact = ParamVal->GetObjNum("forgetFact", 1.0);

        return new TNodeJsRecLinReg(TSignalProc::TOnlineLinReg::New(Dim, RegFact, ForgetFact));
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
    Model->Model->SetForgetFact(ForgetFact);
    Model->Model->SetRegFact(RegFact);
    Model->Model->SetDim(Dim);

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsRecLinReg::weights(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsRecLinReg* Model = ObjectWrap::Unwrap<TNodeJsRecLinReg>(Info.Holder());

    // get feature vector
    TFltV Coef; Model->Model->GetCoeffs(Coef);

    Info.GetReturnValue().Set(TNodeJsFltV::New(Coef));
}

void TNodeJsRecLinReg::dim(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
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
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
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
void TNodeJsLogReg::Init(v8::Local<v8::Object> exports) {
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

void TNodeJsLogReg::weights(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsLogReg* JsLogReg = ObjectWrap::Unwrap<TNodeJsLogReg>(Info.Holder());

        if (JsLogReg != nullptr) {
            TFltV WgtV; JsLogReg->LogReg.GetWgtV(WgtV);
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
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    JsModel->LogReg.Save(*JsFOut->SOut);

    Args.GetReturnValue().Set(Args[0]);
}

////////////////////////////////////////////////////////
// Proportional Hazards Model
void TNodeJsPropHaz::Init(v8::Local<v8::Object> exports) {
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

void TNodeJsPropHaz::weights(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    try {
        TNodeJsPropHaz* JsExpReg = ObjectWrap::Unwrap<TNodeJsPropHaz>(Info.Holder());

        if (JsExpReg != nullptr) {
            TFltV WgtV; JsExpReg->Model.GetWgtV(WgtV);
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
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    JsModel->Model.Save(*JsFOut->SOut);

    Args.GetReturnValue().Set(Args[0]);
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

void TNodeJsNNet::Init(v8::Local<v8::Object> exports) {
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
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
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

void TNodeJsTokenizer::Init(v8::Local<v8::Object> exports) {
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
    if (Args.Length() == 0) {
        const TStr TypeNm = "unicode";
        PJsonVal ParamVal = TJsonVal::NewObj();
        ParamVal->AddToObj("type", TypeNm);
        // create tokenizer
        PTokenizer Tokenizer = TTokenizer::New(TypeNm, ParamVal);
        return new TNodeJsTokenizer(Tokenizer);
    } else if (TNodeJsUtil::IsArgObj(Args, 0)) {
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        const TStr TypeNm = ParamVal->GetObjStr("type", "unicode");
        ParamVal->AddToObj("type", TypeNm);
        // create tokenizer
        PTokenizer Tokenizer = TTokenizer::New(TypeNm, ParamVal);
        return new TNodeJsTokenizer(Tokenizer);
    } else {
        throw TExcept::New("Tokenizer construction: supported only objects!");
    }
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
    if (ParamVal->IsObjKey("maxStep")) MxStep = (int) ParamVal->GetObjNum("maxStep");
    if (ParamVal->IsObjKey("maxSecs")) MxSecs = (int) ParamVal->GetObjNum("maxSecs");
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
    case vdtEucl:
        ParamVal->AddToObj("distType", "Euclid"); break;
    case vdtCos:
        ParamVal->AddToObj("distType", "Cos"); break;
    case vdtSqrtCos:
        ParamVal->AddToObj("distType", "SqrtCos"); break;
    default:
        throw TExcept::New("MDS.GetParams: unsupported distance type " + TInt::GetStr((int)DistType));
    }
    return ParamVal;
}

void TNodeJsMDS::Save(TSOut& SOut) const {
    TInt(MxStep).Save(SOut);
    TInt(MxSecs).Save(SOut);
    TFlt(MnDiff).Save(SOut);
    SaveEnum<TVizDistType>(SOut, DistType);
}

void TNodeJsMDS::Init(v8::Local<v8::Object> exports) {
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
    NODE_SET_PROTOTYPE_METHOD(tpl, "fitTransformAsync", _fitTransformAsync);
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

TNodeJsMDS::TFitTransformTask::TFitTransformTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync):
        TNodeTask(Args, IsAsync),
        JsMDS(nullptr),
        JsFltVV(nullptr),
        JsSpVV(nullptr),
        JsResult(nullptr),
        Notify(TQm::TEnv::Logger()) {

    JsMDS = ObjectWrap::Unwrap<TNodeJsMDS>(Args.Holder());

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        JsFltVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        JsSpVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
    }
    else {
        throw TExcept::New("MDS.fitTransform: argument not a sparse or dense matrix!");
    }

    JsResult = new TNodeJsFltVV();
}

v8::Local<v8::Function> TNodeJsMDS::TFitTransformTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    return TNodeJsUtil::GetArgFun(Args, 1);
}

void TNodeJsMDS::TFitTransformTask::Run() {
    try {
        TVec<TFltV> Temp;
        TFltV DummyClsV;
        PSVMTrainSet TrainSet;
        // algorithm parameters
        int MxStep = JsMDS->MxStep;
        int MxSecs = JsMDS->MxSecs;
        double MnDiff = JsMDS->MnDiff;
        TVizDistType DistType = JsMDS->DistType;
        bool RndStartPos = true;

        if (JsFltVV != nullptr) {
            const TFltVV& Mat = JsFltVV->Mat;
            DummyClsV.Gen(Mat.GetCols());
            TrainSet = TRefDenseTrainSet::New(Mat, DummyClsV);
            TVizMapFactory::MakeFlat(TrainSet, DistType, Temp, MxStep, MxSecs, MnDiff, RndStartPos, Notify);
        } else if (JsSpVV != nullptr) {
            const TVec<TIntFltKdV>& Mat = JsSpVV->Mat;
            DummyClsV.Gen(Mat.Len());
            TrainSet = TRefSparseTrainSet::New(Mat, DummyClsV);
            TVizMapFactory::MakeFlat(TrainSet, DistType, Temp, MxStep, MxSecs, MnDiff, RndStartPos, Notify);
        } else {
            throw TExcept::New("MDS.fitTransform: expects dense or sparse matrix!");
        }

        TFltVV& Result = JsResult->Mat;
        Result.Gen(Temp.Len(), Temp[0].Len());
        for (int RowN = 0; RowN < Temp.Len(); RowN++) {
            for (int ColN = 0; ColN < Temp[0].Len(); ColN++) {
                Result(RowN, ColN) = Temp[RowN][ColN];
            }
        }
    } catch (const PExcept& Except) {
        delete JsResult;
        SetExcept(Except);
    }
}

v8::Local<v8::Value> TNodeJsMDS::TFitTransformTask::WrapResult() {
    return TNodeJsUtil::NewInstance(JsResult);
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

void TNodeJsMDS::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "MDS.save: Should have 1 argument!");

    try {
        TNodeJsMDS* JsMDS = ObjectWrap::Unwrap<TNodeJsMDS>(Args.Holder());
        // get output stream from argumetns
        TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
        // save model
        JsMDS->Save(*JsFOut->SOut);
        // return output stream for convenience
        Args.GetReturnValue().Set(Args[0]);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "MDS::save");
    }
}

/////////////////////////////////////////////
// KMeans

TNodeJsKMeans::TNodeJsKMeans(const PJsonVal& ParamVal) :
        Iter(10000),
        K(2),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(),
        SparseFitMatrix(),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        Model(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsKMeans::TNodeJsKMeans(const PJsonVal& ParamVal, const TFltVV& Mat) :
        Iter(10000),
        K(2),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(Mat),
        SparseFitMatrix(),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        Model(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsKMeans::TNodeJsKMeans(const PJsonVal& ParamVal, const TVec<TIntFltKdV>& Mat) :
        Iter(10000),
        K(2),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(),
        SparseFitMatrix(Mat),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        Model(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsKMeans::TNodeJsKMeans(TSIn& SIn) :
        Iter(TInt(SIn)),
        K(TInt(SIn)),
        AllowEmptyP(SIn),
        CalcDistQualP(SIn),
        AssignV(SIn),
        Medoids(SIn),
        FitIdx(SIn),
        DenseFitMatrix(SIn),
        SparseFitMatrix(SIn),
        DistType(LoadEnum<TDistanceType>(SIn)),
        CentType(LoadEnum<TCentroidType>(SIn)),
        Verbose(TBool(SIn)) {
    if (DistType == TDistanceType::dtEuclid) {
        Dist = new TClustering::TEuclDist;
    } else if (DistType == TDistanceType::dtCos) {
        Dist = new TClustering::TCosDist;
    } else {
        throw TExcept::New("KMeans load constructor: distance type not valid!");
    }

    if (CentType == TCentroidType::ctDense) {
        Model = (void*) TClustering::TAbsKMeans<TFltVV>::LoadPtr(SIn);
    } else if (CentType == TCentroidType::ctSparse) {
        Model = (void*) TClustering::TAbsKMeans<TVec<TIntFltKdV>>::LoadPtr(SIn);
    } else {
        throw TExcept::New("KMeans load constructor: loading invalid KMeans model!");
    }

    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

TNodeJsKMeans::~TNodeJsKMeans() {
    TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++;
    TNodeJsUtil::ObjCount.Val3++;
    CleanUp();
}

void TNodeJsKMeans::UpdateParams(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("iter")) { Iter = ParamVal->GetObjInt("iter"); }
    if (ParamVal->IsObjKey("k")) { K = ParamVal->GetObjInt("k"); }
    if (ParamVal->IsObjKey("fitIdx")) { ParamVal->GetObjIntV("fitIdx", FitIdx); }
    if (ParamVal->IsObjKey("allowEmpty")) { AllowEmptyP = ParamVal->GetObjBool("allowEmpty"); }
    if (ParamVal->IsObjKey("calcDistQual")) { CalcDistQualP = ParamVal->GetObjBool("calcDistQual"); }
    if (ParamVal->IsObjKey("distanceType")) {
        TStr dist = ParamVal->GetObjStr("distanceType");
        if (dist == "Euclid") {
            DistType = TDistanceType::dtEuclid;
        } else if (dist == "Cos") {
            DistType = TDistanceType::dtCos;
        } else {
            throw TExcept::New("Update KMeans Exception: distanceType must be Euclid or Cos!");
        }
    }
    if (ParamVal->IsObjKey("centroidType")) {
        TStr clust = ParamVal->GetObjStr("centroidType");
        if (clust == "Dense") {
            CentType = TCentroidType::ctDense;
        } else if (clust == "Sparse") {
            CentType = TCentroidType::ctSparse;
        } else {
            throw TExcept::New("Update KMeans Exception: centroidType must be Dense or Sparse!");
        }
    }
    if (ParamVal->IsObjKey("verbose")) { Verbose = ParamVal->GetObjBool("verbose"); }

    if (DistType == TDistanceType::dtEuclid) {
        Dist = new TClustering::TEuclDist;
    }
    else if (DistType == TDistanceType::dtCos) {
        Dist = new TClustering::TCosDist;
    }
    else {
        throw TExcept::New("Update KMeans Exception: distance type is not valid " + TInt::GetStr((int)DistType));
    }

    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

void TNodeJsKMeans::Save(TSOut& SOut) const {
    TInt(Iter).Save(SOut);
    TInt(K).Save(SOut);
    AllowEmptyP.Save(SOut);
    CalcDistQualP.Save(SOut);
    AssignV.Save(SOut);
    Medoids.Save(SOut);
    FitIdx.Save(SOut);
    DenseFitMatrix.Save(SOut);
    SparseFitMatrix.Save(SOut);
    SaveEnum<TDistanceType>(SOut, DistType);
    SaveEnum<TCentroidType>(SOut, CentType);
    TBool(Verbose).Save(SOut);
    if (CentType == TCentroidType::ctDense) {
        ((TClustering::TDnsKMeans<TFltVV>*)Model)->Save(SOut);
    } else if (CentType == TCentroidType::ctSparse) {
        ((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)Model)->Save(SOut);
    }
}

void TNodeJsKMeans::CleanUp() {
    if (Model != nullptr) {
        if (CentType == TCentroidType::ctDense) {
            delete (TClustering::TDnsKMeans<TFltVV>*) Model;
        }
        else if (CentType == TCentroidType::ctSparse) {
            delete (TClustering::TDnsKMeans<TVec<TIntFltKdV>>*) Model;
        }
        else {
            throw TExcept::New("KMeans.fit: CentroidType not recognized!");
        }
    }
}

void TNodeJsKMeans::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsKMeans>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fitAsync", _fitAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
    NODE_SET_PROTOTYPE_METHOD(tpl, "transform", _transform);
    NODE_SET_PROTOTYPE_METHOD(tpl, "permuteCentroids", _permuteCentroids);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "centroids"), _centroids);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "medoids"), _medoids);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "idxv"), _idxv);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "relMeanCentroidDist"), _relMeanCentroidDist);

    // properties
    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsKMeans* TNodeJsKMeans::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsKMeans(TJsonVal::NewObj());
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsKMeans(*JsFIn->SIn);
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        if (TNodeJsUtil::IsObjFld(Args[0]->ToObject(), "fitStart")) {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0, true, true);
            v8::Local<v8::Object> FitStart = TNodeJsUtil::GetFldObj(Args[0]->ToObject(), "fitStart");
            if (TNodeJsUtil::IsObjFld(FitStart, "C")) {
                if (TNodeJsUtil::IsFldClass<TNodeJsFltVV>(FitStart, "C")) {
                    TFltVV JsMat = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(FitStart, "C")->Mat;
                    return new TNodeJsKMeans(ParamVal, JsMat);
                }
                else if (TNodeJsUtil::IsFldClass<TNodeJsSpMat>(FitStart, "C")) {
                    TVec<TIntFltKdV> JsMat = TNodeJsUtil::GetUnwrapFld<TNodeJsSpMat>(FitStart, "C")->Mat;
                    return new TNodeJsKMeans(ParamVal, JsMat);
                }
                else {
                    throw TExcept::New("new KMeans: value at fitStart.C is not a dense or sparse matrix!");
                }
            }
            else {
                throw TExcept::New("new KMeans: fitStart argument C not specified!");
            }
        }
        else {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
            return new TNodeJsKMeans(ParamVal);
        }
    }
    else {
        throw TExcept::New("new KMeans: wrong arguments in constructor!");
    }
}

void TNodeJsKMeans::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 0, "KMeans.getParams: takes 0 argument!");

    try {
        v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result

        TNodeJsKMeans* JsKMeans = TNodeJsKMeans::Unwrap<TNodeJsKMeans>(Args.Holder());

        if (!JsKMeans->DenseFitMatrix.Empty() || !JsKMeans->SparseFitMatrix.Empty()) {
            v8::Local<v8::Object> FitStart = v8::Object::New(Isolate);
            if (!JsKMeans->DenseFitMatrix.Empty()) {
                FitStart->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "C")), TNodeJsFltVV::New(JsKMeans->DenseFitMatrix));
            }
            else {
                FitStart->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "C")), TNodeJsSpMat::New(JsKMeans->SparseFitMatrix));
            }
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "fitStart")), FitStart);
        }
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "iter")), v8::Integer::New(Isolate, JsKMeans->Iter));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "k")), v8::Integer::New(Isolate, JsKMeans->K));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "verbose")), v8::Boolean::New(Isolate, JsKMeans->Verbose));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "allowEmpty")), v8::Boolean::New(Isolate, JsKMeans->AllowEmptyP));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "calcDistQual")), v8::Boolean::New(Isolate, JsKMeans->CalcDistQualP));

        if (!JsKMeans->FitIdx.Empty()) {
            v8::Local<v8::Array> FitIdx = v8::Array::New(Isolate, JsKMeans->FitIdx.Len());
            for (int ElN = 0; ElN < JsKMeans->FitIdx.Len(); ElN++) {
                FitIdx->Set(ElN, v8::Integer::New(Isolate, JsKMeans->FitIdx.GetVal(ElN).Val));
            }
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "fitIdx")), FitIdx);
        }

        switch (JsKMeans->DistType) {
        case dtEuclid:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "distanceType")), v8::String::NewFromUtf8(Isolate, "Euclid"));
            break;
        case dtCos:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "distanceType")), v8::String::NewFromUtf8(Isolate, "Cos"));
            break;
        default:
            throw TExcept::New("KMeans.GetParams: unsupported distance type " + TInt::GetStr((int)JsKMeans->DistType));
        }
        switch (JsKMeans->CentType) {
        case ctDense:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "centroidType")), v8::String::NewFromUtf8(Isolate, "Dense"));
            break;
        case ctSparse:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "centroidType")), v8::String::NewFromUtf8(Isolate, "Sparse"));
            break;
        default:
            throw TExcept::New("KMeans.GetParams: unsupported centroid type " + TInt::GetStr((int)JsKMeans->CentType));
        }

        return Args.GetReturnValue().Set(JsObj);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "KMeans::getParams");
    }
}

void TNodeJsKMeans::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "KMeans.setParams: takes 1 argument!");
    EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "KMeans.setParams: first argument should be a Javascript object!");

    try {
        TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());

        // create new model from given parameters
        if (TNodeJsUtil::IsObjFld(Args[0]->ToObject(), "fitStart")) {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0, true, true);

            v8::Local<v8::Object> FitStart = TNodeJsUtil::GetFldObj(Args[0]->ToObject(), "fitStart");
            if (TNodeJsUtil::IsObjFld(FitStart, "C")) {
                if (TNodeJsUtil::IsFldClass<TNodeJsFltVV>(FitStart, "C")) {
                    JsKMeans->SparseFitMatrix.Clr();
                    JsKMeans->DenseFitMatrix = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(FitStart, "C")->Mat;
                    JsKMeans->UpdateParams(ParamVal);
                }
                else if (TNodeJsUtil::IsFldClass<TNodeJsSpMat>(FitStart, "C")) {
                    JsKMeans->SparseFitMatrix = TNodeJsUtil::GetUnwrapFld<TNodeJsSpMat>(FitStart, "C")->Mat;
                    JsKMeans->DenseFitMatrix.Clr();
                    JsKMeans->UpdateParams(ParamVal);
                }
                else {
                    throw TExcept::New("new KMeans: value at fitStart.C is not a dense or sparse matrix!");
                }
            }
            else {
                throw TExcept::New("new KMeans: fitStart argument C not specified!");
            }
        }
        else {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
            JsKMeans->UpdateParams(ParamVal);
        }
        Args.GetReturnValue().Set(Args.Holder());
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "KMeans::setParams");
    }
}

TNodeJsKMeans::TFitTask::TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync) :
        TNodeTask(Args, IsAsync),
        JsKMeans(nullptr),
        JsFltVV(nullptr),
        JsSpVV(nullptr),
        JsIntV(nullptr),
        JsArr(nullptr) {

    JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        JsFltVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        JsSpVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
    }
    else {
        throw TExcept::New("KMeans.fit: argument not a sparse or dense matrix!");
    }

    if (Args.Length() >= 2 && !TNodeJsUtil::IsArgFun(Args, 1)) {
        if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 1)) {
            JsIntV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsIntV>(Args, 1);
        }
        else if (Args[1]->IsArray()) {
            v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Args[1]);
            const int Len = Arr->Length();
            JsArr = new TNodeJsIntV(Len);
            TNodeJsUtil::GetArgIntV(Args, 1, JsArr->Vec);
        }
        else {
            throw TExcept::New("KMeans.fit: second argument expected to be an IntVector or Array!");
        }
    }
}

v8::Local<v8::Function> TNodeJsKMeans::TFitTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    if (TNodeJsUtil::IsArgFun(Args, 1)) {
        return TNodeJsUtil::GetArgFun(Args, 1);
    }
    else {
        return TNodeJsUtil::GetArgFun(Args, 2);
    }
}

void TNodeJsKMeans::TFitTask::Run() {
    try {
       // delete the previous model
       JsKMeans->CleanUp();

       const bool CalcDistQualP = JsKMeans->CalcDistQualP;

       // create a new model
       if (JsKMeans->CentType == TCentroidType::ctDense) {
           TClustering::TDenseKMeans* KMeans = new TClustering::TDenseKMeans(JsKMeans->K, TRnd(0), JsKMeans->Dist, CalcDistQualP);

           JsKMeans->Model = (void*) KMeans;

           // input dense matrix
           if (JsFltVV != nullptr) {
               if (!JsKMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->DenseFitMatrix.GetCols() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->DenseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->SparseFitMatrix.Len() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->SparseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->FitIdx.Empty()) {
                   EAssertR(JsKMeans->FitIdx.Len() == JsKMeans->K, "Length of fitIdx must be equal to number of centroids!");
                   EAssertR(JsKMeans->FitIdx.GetMxVal() < JsFltVV->Mat.GetCols(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TFltVV InitCentroidMat;
                   TLinAlg::SubMat(JsFltVV->Mat, JsKMeans->FitIdx, InitCentroidMat);
                   KMeans->Apply(JsFltVV->Mat, InitCentroidMat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else {
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               KMeans->Assign(JsFltVV->Mat, JsKMeans->AssignV);
               TFltVV D;
               JsKMeans->Dist->GetQuasiDistVV(JsFltVV->Mat, KMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsKMeans->Medoids);
               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsFltVV->Mat.GetCols(), "KMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsFltVV->Mat.GetCols(), "KMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           // input sparse matrix
           else if (JsSpVV != nullptr) {
               if (!JsKMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->DenseFitMatrix.GetCols() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->DenseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->SparseFitMatrix.Len() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->SparseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->FitIdx.Empty()) {
                   EAssertR(JsKMeans->FitIdx.Len() == JsKMeans->K, "Length of fitIdx must be equal to number of centroids!");
                   EAssertR(JsKMeans->FitIdx.GetMxVal() < JsSpVV->Mat.Len(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TFltVV InitCentroidMat;
                   int Rows = TLinAlgSearch::GetMaxDimIdx(JsSpVV->Mat) + 1;
                   int Cols = JsKMeans->K;

                   InitCentroidMat.Gen(Rows, Cols);
                   for (int ColN = 0; ColN < Cols; ColN++) {
                       const int Els = JsSpVV->Mat[ColN].Len();
                       for (int ElN = 0; ElN < Els; ElN++) {
                           InitCentroidMat.PutXY(JsSpVV->Mat[ColN][ElN].Key, ColN, JsSpVV->Mat[ColN][ElN].Dat);
                       }
                   }
                   KMeans->Apply(JsSpVV->Mat, InitCentroidMat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else {
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               KMeans->Assign(JsSpVV->Mat, JsKMeans->AssignV);

               TFltVV D;
               JsKMeans->Dist->GetQuasiDistVV(JsSpVV->Mat, KMeans->GetCentroidVV(), D);

               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsKMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsSpVV->Mat.Len(), "KMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsSpVV->Mat.Len(), "KMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           else {
               throw TExcept::New("KMeans.fit: first argument expected to be an dense or sparse matrix!");
           }
       }
       else if (JsKMeans->CentType == TCentroidType::ctSparse) {
           TClustering::TSparseKMeans* KMeans = new TClustering::TSparseKMeans(JsKMeans->K, TRnd(0), JsKMeans->Dist, CalcDistQualP);
           JsKMeans->Model = (void*) KMeans;

           // input dense matrix
           if (JsFltVV != nullptr) {
               if (!JsKMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->SparseFitMatrix.Len() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->SparseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
                else if (!JsKMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->DenseFitMatrix.GetCols() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->DenseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->FitIdx.Empty()) {
                   EAssertR(JsKMeans->FitIdx.Len() == JsKMeans->K, "Length of fitIdx must be equal to number of centroids!");
                   EAssertR(JsKMeans->FitIdx.GetMxVal() < JsFltVV->Mat.GetCols(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TVec<TIntFltKdV> InitCentroidMat;
                   int Rows = JsFltVV->Mat.GetRows();
                   int K = JsKMeans->K;
                   InitCentroidMat.Gen(K);
                   for (int i = 0; i < K; i++) {
                       const int ColN = JsKMeans->FitIdx[i];
                       for (int RowN = 0; RowN < Rows; RowN++) {
                           if (JsFltVV->Mat(RowN, ColN) != 0.0) { InitCentroidMat[i].Add(TIntFltKd(RowN, JsFltVV->Mat(RowN, ColN))); }
                       }
                   }
                   KMeans->Apply(JsFltVV->Mat, InitCentroidMat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else {
                   KMeans->Apply(JsFltVV->Mat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }

               KMeans->Assign(JsFltVV->Mat, JsKMeans->AssignV);

               TFltVV D;
               JsKMeans->Dist->GetQuasiDistVV(JsFltVV->Mat, KMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsKMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsFltVV->Mat.GetCols(), "KMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsFltVV->Mat.GetCols(), "KMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           // input sparse matrix
           else if (JsSpVV != nullptr) {
               if (!JsKMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->SparseFitMatrix.Len() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->SparseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsKMeans->DenseFitMatrix.GetCols() == JsKMeans->K, "Number of columns must be equal to number of centroids!");
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->DenseFitMatrix, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else if (!JsKMeans->FitIdx.Empty()) {
                   EAssertR(JsKMeans->FitIdx.Len() == JsKMeans->K, "Length of fitIdx must be equal to number of centroids!");
                   EAssertR(JsKMeans->FitIdx.GetMxVal() < JsSpVV->Mat.Len(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TVec<TIntFltKdV> InitCentroidMat;
                   int Dim = JsKMeans->K;

                   InitCentroidMat.Gen(Dim);
                   for (int i = 0; i < Dim; i++) {
                       const int ClustN = JsKMeans->FitIdx[i];
                       InitCentroidMat[i] = JsSpVV->Mat[ClustN];
                   }
                   KMeans->Apply(JsSpVV->Mat, InitCentroidMat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }
               else {
                   KMeans->Apply(JsSpVV->Mat, JsKMeans->AllowEmptyP, JsKMeans->Iter, JsKMeans->Notify);
               }

               KMeans->Assign(JsSpVV->Mat, JsKMeans->AssignV);

               TFltVV D;
               JsKMeans->Dist->GetQuasiDistVV(JsSpVV->Mat, KMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsKMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsSpVV->Mat.Len(), "KMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsSpVV->Mat.Len(), "KMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsKMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsKMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           else {
               throw TExcept::New("KMeans.fit: first argument expected to be an dense or sparse matrix!");
           }
       }
       // clean up
       if (JsArr != nullptr) { delete JsArr; }
    }
    catch (const PExcept& Except) {
        // clean up
        if (JsArr != nullptr) { delete JsArr; }
        SetExcept(Except);
    }
}

void TNodeJsKMeans::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "KMeans.predict: expects 1 argument!");
    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());

    if (!JsKMeans->Model) {
        throw TExcept::New("KMeans.predict: Model not initialized. First call fit!");
    }

    TIntV AssignV;
    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
        if (JsKMeans->CentType == TCentroidType::ctDense) {
            ((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->Assign(Mat, AssignV);
        }
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            ((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->Assign(Mat, AssignV);
        }
        else {
            throw TExcept::New("KMeans.predict: invalid centroid type " + TInt::GetStr((int)JsKMeans->CentType));
        }
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
        if (JsKMeans->CentType == TCentroidType::ctDense) {
            ((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->Assign(Mat, AssignV);
        }
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            ((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->Assign(Mat, AssignV);
        }
        else {
            throw TExcept::New("KMeans.predict: invalid centroid type " + TInt::GetStr((int)JsKMeans->CentType));
        }
    }
    else {
        throw TExcept::New("KMeans.predict: Argument expected to be a dense or sparse matrix!");
    }

    Args.GetReturnValue().Set(TNodeJsIntV::New(AssignV));
}

void TNodeJsKMeans::transform(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "KMeans.explain: Should have 1 argument!");

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());
    if (!JsKMeans->Model) {
        throw TExcept::New("KMeans.explain: centroids not initialized!");
    }

    TFltVV D;
    // if argument is a dense matrix
    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
        // if centroids are dense
        if (JsKMeans->CentType == TCentroidType::ctDense) {
            JsKMeans->Dist->GetDistVV(((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->GetCentroidVV(), Mat, D);
        }
        // if centroids are sparse
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            JsKMeans->Dist->GetDistVV(((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->GetCentroidVV(), Mat, D);
        }
        else {
            throw TExcept::New("KMeans.explain: centroid type invalid " + TInt::GetStr((int) JsKMeans->CentType));
        }
    }
    // if the argument is a sparse matrix
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
        // if centroids are dense
        if (JsKMeans->CentType == TCentroidType::ctDense) {
            JsKMeans->Dist->GetDistVV(((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->GetCentroidVV(), Mat, D);
        }
        // if centroids are sparse
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            JsKMeans->Dist->GetDistVV(((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->GetCentroidVV(), Mat, D);
        }
        else {
            throw TExcept::New("KMeans.explain: centroid type invalid " + TInt::GetStr((int) JsKMeans->CentType));
        }
    }
    else {
        throw TExcept::New("KMeans.explain: Argument must be a dense or sparse matrix!");
    }
    Args.GetReturnValue().Set(TNodeJsFltVV::New(D));
}

void TNodeJsKMeans::permuteCentroids(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "KMeans.permuteCentroids: Should have 1 argument!");

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());

    if (JsKMeans->Model == nullptr) {
        throw TExcept::New("KMeans.permuteCentroids: centroids not initialized!");
    }

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 0)) {

        TIntV& Mapping = TNodeJsUtil::GetArgUnwrapObj<TNodeJsIntV>(Args, 0)->Vec;
        EAssertR(Mapping.Len() == JsKMeans->K, "KMeans.permuteCentroids: Length of parameter must be equal to K!");
        EAssertR(TLinAlgSearch::GetMaxVal(Mapping) + 1 == JsKMeans->K, "KMeans.permuteCentroids: maximum index of parameter must be equal to number of centroids!");

        if (JsKMeans->CentType == TCentroidType::ctDense) {
            ((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->PermutateCentroids(Mapping);
        }
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            ((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->PermutateCentroids(Mapping);
        }
        else {
            throw TExcept::New("KMeans.permuteCentroids: centroid type invalid " + TInt::GetStr((int)JsKMeans->CentType));
        }

        TIntV AssignTemp = JsKMeans->AssignV;
        TIntV MedoidsTemp = JsKMeans->Medoids;
        for (int Idx = 0; Idx < JsKMeans->K; Idx++) {
            JsKMeans->AssignV[Idx] = AssignTemp[Mapping[Idx]];
            JsKMeans->Medoids[Idx] = MedoidsTemp[Mapping[Idx]];

        }
    }
    else {
        throw TExcept::New("KMeans.permuteCentroids: first argument should be an IntVector!");
    }
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsKMeans::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "KMeans.save: Should have 1 argument!");

    try {
        TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Args.Holder());
        // get output stream from argumetns
        TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
        // save model
        JsKMeans->Save(*JsFOut->SOut);
        // return output stream for convenience
        Args.GetReturnValue().Set(Args[0]);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "KMeans::save");
    }
}

void TNodeJsKMeans::centroids(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Info.Holder());

    if (JsKMeans->Model == nullptr) {
        Info.GetReturnValue();
    } else {
        if (JsKMeans->CentType == TCentroidType::ctDense) {
            Info.GetReturnValue().Set(TNodeJsFltVV::New(((TClustering::TDnsKMeans<TFltVV>*)JsKMeans->Model)->GetCentroidVV()));
        }
        else if (JsKMeans->CentType == TCentroidType::ctSparse) {
            Info.GetReturnValue().Set(TNodeJsSpMat::New(((TClustering::TDnsKMeans<TVec<TIntFltKdV>>*)JsKMeans->Model)->GetCentroidVV()));
        }
        else {
            throw TExcept::New("KMeans.centroids: Centroid type not valid " + TInt::GetStr((int)JsKMeans->CentType));
        }
    }
}

void TNodeJsKMeans::medoids(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Info.Holder());

    if (JsKMeans->Model == nullptr) {
        Info.GetReturnValue();
    }
    else {
        Info.GetReturnValue().Set(TNodeJsIntV::New(JsKMeans->Medoids));
    }
}

void TNodeJsKMeans::idxv(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Info.Holder());

    if (JsKMeans->Model == nullptr) {
        Info.GetReturnValue();
    }
    else {
        Info.GetReturnValue().Set(TNodeJsIntV::New(JsKMeans->AssignV));
    }
}

void TNodeJsKMeans::relMeanCentroidDist(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsKMeans* JsKMeans = ObjectWrap::Unwrap<TNodeJsKMeans>(Info.Holder());

    if (JsKMeans->Model == nullptr) {
        Info.GetReturnValue().Set(v8::Undefined(Isolate));
    }
    else {
        switch (JsKMeans->CentType) {
            case ctDense: {
                const TClustering::TDenseKMeans* KMeans = static_cast<TClustering::TDenseKMeans*>(JsKMeans->Model);
                const double RelMeanDist = KMeans->GetRelMeanCentroidDist();
                Info.GetReturnValue().Set(v8::Number::New(Isolate, RelMeanDist));
                break;
            }
            case ctSparse: {
                const TClustering::TSparseKMeans* KMeans = static_cast<TClustering::TSparseKMeans*>(JsKMeans->Model);
                const double RelMeanDist = KMeans->GetRelMeanCentroidDist();
                Info.GetReturnValue().Set(v8::Number::New(Isolate, RelMeanDist));
                break;
            }
            default: {
                throw TExcept::New("Unknown centroid type: " + TInt::GetStr(static_cast<int>(JsKMeans->CentType)));
            }
        }
    }
}

/////////////////////////////////////////////
// DpMeans

TNodeJsDpMeans::TNodeJsDpMeans(const PJsonVal& ParamVal) :
        Iter(10000),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(),
        SparseFitMatrix(),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        DpMeansModel(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsDpMeans::TNodeJsDpMeans(const PJsonVal& ParamVal, const TFltVV& Mat) :
        Iter(10000),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(Mat),
        SparseFitMatrix(),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        DpMeansModel(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsDpMeans::TNodeJsDpMeans(const PJsonVal& ParamVal, const TVec<TIntFltKdV>& Mat) :
        Iter(10000),
        AllowEmptyP(true),
        AssignV(),
        Medoids(),
        FitIdx(),
        DenseFitMatrix(),
        SparseFitMatrix(Mat),
        DistType(TDistanceType::dtEuclid),
        Dist(nullptr),
        CentType(TCentroidType::ctDense),
        DpMeansModel(nullptr),
        Verbose(false) {
    UpdateParams(ParamVal);
}

TNodeJsDpMeans::TNodeJsDpMeans(TSIn& SIn) :
        Iter(TInt(SIn)),
        Lambda(SIn),
        MnClusts(SIn),
        MxClusts(SIn),
        AllowEmptyP(SIn),
        CalcDistQualP(SIn),
        AssignV(SIn),
        Medoids(SIn),
        FitIdx(SIn),
        DenseFitMatrix(SIn),
        SparseFitMatrix(SIn),
        DistType(LoadEnum<TDistanceType>(SIn)),
        CentType(LoadEnum<TCentroidType>(SIn)),
        Verbose(TBool(SIn)) {
    if (DistType == TDistanceType::dtEuclid) {
        Dist = new TClustering::TEuclDist;
    } else if (DistType == TDistanceType::dtCos) {
        Dist = new TClustering::TCosDist;
    } else {
        throw TExcept::New("DpMeans load constructor: distance type not valid!");
    }

    if (CentType == TCentroidType::ctDense) {
        DpMeansModel = TClustering::TAbsKMeans<TFltVV>::LoadPtr(SIn);
    } else if (CentType == TCentroidType::ctSparse) {
        DpMeansModel = TClustering::TAbsKMeans<TVec<TIntFltKdV>>::LoadPtr(SIn);
    } else {
        throw TExcept::New("DpMeans load constructor: loading invalid DpMeans model!");
    }

    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

TNodeJsDpMeans::~TNodeJsDpMeans() {
    TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++;
    TNodeJsUtil::ObjCount.Val3++;
    CleanUp();
}

void TNodeJsDpMeans::UpdateParams(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("iter")) { Iter = ParamVal->GetObjInt("iter"); }
    if (ParamVal->IsObjKey("lambda")) { Lambda = ParamVal->GetObjNum("lambda"); }
    if (ParamVal->IsObjKey("minClusters")) { MnClusts = ParamVal->GetObjInt("minClusters"); }
    if (ParamVal->IsObjKey("maxClusters")) { MxClusts = ParamVal->GetObjInt("maxClusters"); }
    if (ParamVal->IsObjKey("fitIdx")) { ParamVal->GetObjIntV("fitIdx", FitIdx); }
    if (ParamVal->IsObjKey("allowEmpty")) { AllowEmptyP = ParamVal->GetObjBool("allowEmpty"); }
    if (ParamVal->IsObjKey("calcDistQual")) { CalcDistQualP = ParamVal->GetObjBool("calcDistQual"); }
    if (ParamVal->IsObjKey("distanceType")) {
        TStr dist = ParamVal->GetObjStr("distanceType");
        if (dist == "Euclid") {
            DistType = TDistanceType::dtEuclid;
        } else if (dist == "Cos") {
            DistType = TDistanceType::dtCos;
        } else {
            throw TExcept::New("Update KMeans Exception: distanceType must be Euclid or Cos!");
        }
    }
    if (ParamVal->IsObjKey("centroidType")) {
        TStr clust = ParamVal->GetObjStr("centroidType");
        if (clust == "Dense") {
            CentType = TCentroidType::ctDense;
        } else if (clust == "Sparse") {
            CentType = TCentroidType::ctSparse;
        } else {
            throw TExcept::New("Update KMeans Exception: centroidType must be Dense or Sparse!");
        }
    }
    if (ParamVal->IsObjKey("verbose")) { Verbose = ParamVal->GetObjBool("verbose"); }

    if (DistType == TDistanceType::dtEuclid) {
        Dist = new TClustering::TEuclDist;
    }
    else if (DistType == TDistanceType::dtCos) {
        Dist = new TClustering::TCosDist;
    }
    else {
        throw TExcept::New("Update KMeans Exception: distance type is not valid " + TInt::GetStr((int)DistType));
    }

    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

void TNodeJsDpMeans::Save(TSOut& SOut) const {
    TInt(Iter).Save(SOut);
    Lambda.Save(SOut);
    MnClusts.Save(SOut);
    MxClusts.Save(SOut);
    AllowEmptyP.Save(SOut);
    CalcDistQualP.Save(SOut);
    AssignV.Save(SOut);
    Medoids.Save(SOut);
    FitIdx.Save(SOut);
    DenseFitMatrix.Save(SOut);
    SparseFitMatrix.Save(SOut);
    SaveEnum<TDistanceType>(SOut, DistType);
    SaveEnum<TCentroidType>(SOut, CentType);
    TBool(Verbose).Save(SOut);
    if (CentType == TCentroidType::ctDense) {
        GetDenseModel()->Save(SOut);
    } else if (CentType == TCentroidType::ctSparse) {
        GetSparseModel()->Save(SOut);
    }
}

void TNodeJsDpMeans::CleanUp() {
    if (DpMeansModel != nullptr) {
        if (CentType == TCentroidType::ctDense) {
            delete GetDenseModel();
        }
        else if (CentType == TCentroidType::ctSparse) {
            delete GetSparseModel();
        }
        else {
            throw TExcept::New("KMeans.fit: CentroidType not recognized!");
        }
    }
}

void TNodeJsDpMeans::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsDpMeans>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fitAsync", _fitAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "predict", _predict);
    NODE_SET_PROTOTYPE_METHOD(tpl, "transform", _transform);
    NODE_SET_PROTOTYPE_METHOD(tpl, "permuteCentroids", _permuteCentroids);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "centroids"), _centroids);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "medoids"), _medoids);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "idxv"), _idxv);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "relMeanCentroidDist"), _relMeanCentroidDist);

    // properties
    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsDpMeans* TNodeJsDpMeans::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsDpMeans(TJsonVal::NewObj());
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsDpMeans(*JsFIn->SIn);
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        if (TNodeJsUtil::IsObjFld(Args[0]->ToObject(), "fitStart")) {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0, true, true);
            v8::Local<v8::Object> FitStart = TNodeJsUtil::GetFldObj(Args[0]->ToObject(), "fitStart");
            if (TNodeJsUtil::IsObjFld(FitStart, "C")) {
                if (TNodeJsUtil::IsFldClass<TNodeJsFltVV>(FitStart, "C")) {
                    TFltVV JsMat = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(FitStart, "C")->Mat;
                    return new TNodeJsDpMeans(ParamVal, JsMat);
                }
                else if (TNodeJsUtil::IsFldClass<TNodeJsSpMat>(FitStart, "C")) {
                    TVec<TIntFltKdV> JsMat = TNodeJsUtil::GetUnwrapFld<TNodeJsSpMat>(FitStart, "C")->Mat;
                    return new TNodeJsDpMeans(ParamVal, JsMat);
                }
                else {
                    throw TExcept::New("new KMeans: value at fitStart.C is not a dense or sparse matrix!");
                }
            }
            else {
                throw TExcept::New("new KMeans: fitStart argument C not specified!");
            }
        }
        else {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
            return new TNodeJsDpMeans(ParamVal);
        }
    }
    else {
        throw TExcept::New("new KMeans: wrong arguments in constructor!");
    }
}

void TNodeJsDpMeans::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 0, "KMeans.getParams: takes 0 argument!");

    try {
        v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result

        TNodeJsDpMeans* JsDpMeans = TNodeJsDpMeans::Unwrap<TNodeJsDpMeans>(Args.Holder());

        if (!JsDpMeans->DenseFitMatrix.Empty() || !JsDpMeans->SparseFitMatrix.Empty()) {
            v8::Local<v8::Object> FitStart = v8::Object::New(Isolate);
            if (!JsDpMeans->DenseFitMatrix.Empty()) {
                FitStart->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "C")), TNodeJsFltVV::New(JsDpMeans->DenseFitMatrix));
            }
            else {
                FitStart->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "C")), TNodeJsSpMat::New(JsDpMeans->SparseFitMatrix));
            }
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "fitStart")), FitStart);
        }
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "iter")), v8::Integer::New(Isolate, JsDpMeans->Iter));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "lambda")), v8::Number::New(Isolate, JsDpMeans->Lambda));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "minClusters")), v8::Integer::New(Isolate, JsDpMeans->MnClusts));
        if (JsDpMeans->MxClusts != TInt::Mx) {
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "maxClusters")), v8::Integer::New(Isolate, JsDpMeans->MxClusts));
        }
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "verbose")), v8::Boolean::New(Isolate, JsDpMeans->Verbose));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "allowEmpty")), v8::Boolean::New(Isolate, JsDpMeans->AllowEmptyP));
        JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "calcDistQual")), v8::Boolean::New(Isolate, JsDpMeans->CalcDistQualP));

        if (!JsDpMeans->FitIdx.Empty()) {
            v8::Local<v8::Array> FitIdx = v8::Array::New(Isolate, JsDpMeans->FitIdx.Len());
            for (int ElN = 0; ElN < JsDpMeans->FitIdx.Len(); ElN++) {
                FitIdx->Set(ElN, v8::Integer::New(Isolate, JsDpMeans->FitIdx.GetVal(ElN).Val));
            }
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "fitIdx")), FitIdx);
        }

        switch (JsDpMeans->DistType) {
        case TDistanceType::dtEuclid:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "distanceType")), v8::String::NewFromUtf8(Isolate, "Euclid"));
            break;
        case TDistanceType::dtCos:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "distanceType")), v8::String::NewFromUtf8(Isolate, "Cos"));
            break;
        default:
            throw TExcept::New("DpMeans.GetParams: unsupported distance type " + TInt::GetStr((int)JsDpMeans->DistType));
        }
        switch (JsDpMeans->CentType) {
        case TCentroidType::ctDense:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "centroidType")), v8::String::NewFromUtf8(Isolate, "Dense"));
            break;
        case TCentroidType::ctSparse:
            JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "centroidType")), v8::String::NewFromUtf8(Isolate, "Sparse"));
            break;
        default:
            throw TExcept::New("DpMeans.GetParams: unsupported centroid type " + TInt::GetStr((int)JsDpMeans->CentType));
        }

        return Args.GetReturnValue().Set(JsObj);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "DpMeans::getParams");
    }
}

void TNodeJsDpMeans::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "DpMeans.setParams: takes 1 argument!");
    EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "DpMeans.setParams: first argument should be a Javascript object!");

    try {
        TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());

        // create new model from given parameters
        if (TNodeJsUtil::IsObjFld(Args[0]->ToObject(), "fitStart")) {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0, true, true);

            v8::Local<v8::Object> FitStart = TNodeJsUtil::GetFldObj(Args[0]->ToObject(), "fitStart");
            if (TNodeJsUtil::IsObjFld(FitStart, "C")) {
                if (TNodeJsUtil::IsFldClass<TNodeJsFltVV>(FitStart, "C")) {
                    JsDpMeans->SparseFitMatrix.Clr();
                    JsDpMeans->DenseFitMatrix = TNodeJsUtil::GetUnwrapFld<TNodeJsFltVV>(FitStart, "C")->Mat;
                    JsDpMeans->UpdateParams(ParamVal);
                }
                else if (TNodeJsUtil::IsFldClass<TNodeJsSpMat>(FitStart, "C")) {
                    JsDpMeans->SparseFitMatrix = TNodeJsUtil::GetUnwrapFld<TNodeJsSpMat>(FitStart, "C")->Mat;
                    JsDpMeans->DenseFitMatrix.Clr();
                    JsDpMeans->UpdateParams(ParamVal);
                }
                else {
                    throw TExcept::New("new DpMeans: value at fitStart.C is not a dense or sparse matrix!");
                }
            }
            else {
                throw TExcept::New("new DpMeans: fitStart argument C not specified!");
            }
        }
        else {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
            JsDpMeans->UpdateParams(ParamVal);
        }
        Args.GetReturnValue().Set(Args.Holder());
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "DpMeans::setParams");
    }
}

TNodeJsDpMeans::TFitTask::TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync) :
        TNodeTask(Args, IsAsync) {

    JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        JsFltVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        JsSpVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
    }
    else {
        throw TExcept::New("DpMeans.fit: argument not a sparse or dense matrix!");
    }

    if (Args.Length() >= 2 && !TNodeJsUtil::IsArgFun(Args, 1)) {
        if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 1)) {
            JsIntV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsIntV>(Args, 1);
        }
        else if (Args[1]->IsArray()) {
            v8::Local<v8::Array> Arr = v8::Local<v8::Array>::Cast(Args[1]);
            const int Len = Arr->Length();
            JsArr = new TNodeJsIntV(Len);
            TNodeJsUtil::GetArgIntV(Args, 1, JsArr->Vec);
        }
        else {
            throw TExcept::New("DpMeans.fit: second argument expected to be an IntVector or Array!");
        }
    }
}

v8::Local<v8::Function> TNodeJsDpMeans::TFitTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    if (TNodeJsUtil::IsArgFun(Args, 1)) {
        return TNodeJsUtil::GetArgFun(Args, 1);
    }
    else {
        return TNodeJsUtil::GetArgFun(Args, 2);
    }
}

void TNodeJsDpMeans::TFitTask::Run() {
    try {
       // delete the previous model
       JsDpMeans->CleanUp();

       const bool CalcDistQualP = JsDpMeans->CalcDistQualP;

       // create a new model
       if (JsDpMeans->CentType == TCentroidType::ctDense) {
           TDenseModel* DpMeans = new TDenseModel(JsDpMeans->Lambda, JsDpMeans->MnClusts, JsDpMeans->MxClusts, TRnd(0), JsDpMeans->Dist, CalcDistQualP);
           JsDpMeans->DpMeansModel = (void*) DpMeans;

           // input dense matrix
           if (JsFltVV != nullptr) {
               if (!JsDpMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->DenseFitMatrix.GetCols() && JsDpMeans->DenseFitMatrix.GetCols() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->DenseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->SparseFitMatrix.Len() && JsDpMeans->SparseFitMatrix.Len() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->SparseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->FitIdx.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->FitIdx.Len() && JsDpMeans->FitIdx.Len() <= JsDpMeans->MxClusts, "Length of fitIdx must be in range [minClusters, maxClusters]!");
                   EAssertR(JsDpMeans->FitIdx.GetMxVal() < JsFltVV->Mat.GetCols(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TFltVV InitCentroidMat;
                   TLinAlg::SubMat(JsFltVV->Mat, JsDpMeans->FitIdx, InitCentroidMat);
                   DpMeans->Apply(JsFltVV->Mat, InitCentroidMat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else {
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               DpMeans->Assign(JsFltVV->Mat, JsDpMeans->AssignV);
               TFltVV D;
               JsDpMeans->Dist->GetQuasiDistVV(JsFltVV->Mat, DpMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsDpMeans->Medoids);
               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsFltVV->Mat.GetCols(), "DpMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsFltVV->Mat.GetCols(), "DpMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           // input sparse matrix
           else if (JsSpVV != nullptr) {
               if (!JsDpMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->DenseFitMatrix.GetCols() && JsDpMeans->DenseFitMatrix.GetCols() <= JsDpMeans->MxClusts, "Number of columns must be in [minClusters, maxClusters] range!");
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->DenseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->SparseFitMatrix.Len() && JsDpMeans->SparseFitMatrix.Len() <= JsDpMeans->MxClusts, "Number of columns must be in [minClusters, maxClusters] range!");
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->SparseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->FitIdx.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->FitIdx.Len() && JsDpMeans->FitIdx.Len() <= JsDpMeans->MxClusts, "Length of fitIdx must be in [minClusters, maxClusters] range!");
                   EAssertR(JsDpMeans->FitIdx.GetMxVal() < JsSpVV->Mat.Len(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TFltVV InitCentroidMat;
                   int Rows = TLinAlgSearch::GetMaxDimIdx(JsSpVV->Mat) + 1;
                   int Cols = JsDpMeans->MnClusts;

                   InitCentroidMat.Gen(Rows, Cols);
                   for (int ColN = 0; ColN < Cols; ColN++) {
                       const int Els = JsSpVV->Mat[ColN].Len();
                       for (int ElN = 0; ElN < Els; ElN++) {
                           InitCentroidMat.PutXY(JsSpVV->Mat[ColN][ElN].Key, ColN, JsSpVV->Mat[ColN][ElN].Dat);
                       }
                   }
                   DpMeans->Apply(JsSpVV->Mat, InitCentroidMat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else {
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               DpMeans->Assign(JsSpVV->Mat, JsDpMeans->AssignV);

               TFltVV D;
               JsDpMeans->Dist->GetQuasiDistVV(JsSpVV->Mat, DpMeans->GetCentroidVV(), D);

               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsDpMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsSpVV->Mat.Len(), "DpMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsSpVV->Mat.Len(), "DpMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           else {
               throw TExcept::New("DpMeans.fit: first argument expected to be an dense or sparse matrix!");
           }
       }
       else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
           TSparseModel* DpMeans = new TSparseModel(JsDpMeans->Lambda, JsDpMeans->MnClusts, JsDpMeans->MxClusts, TRnd(0), JsDpMeans->Dist, CalcDistQualP);
           JsDpMeans->DpMeansModel = (void*) DpMeans;

           // input dense matrix
           if (JsFltVV != nullptr) {
               if (!JsDpMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->SparseFitMatrix.Len() && JsDpMeans->SparseFitMatrix.Len() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->SparseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
                else if (!JsDpMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->DenseFitMatrix.GetCols() && JsDpMeans->DenseFitMatrix.GetCols() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->DenseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->FitIdx.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->FitIdx.Len() && JsDpMeans->FitIdx.Len() <= JsDpMeans->MxClusts, "Length of fitIdx must be in range [minClusters, maxClusters]!");
                   EAssertR(JsDpMeans->FitIdx.GetMxVal() < JsFltVV->Mat.GetCols(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TVec<TIntFltKdV> InitCentroidMat;
                   int Rows = JsFltVV->Mat.GetRows();
                   int K = JsDpMeans->MnClusts;
                   InitCentroidMat.Gen(K);
                   for (int i = 0; i < K; i++) {
                       const int ColN = JsDpMeans->FitIdx[i];
                       for (int RowN = 0; RowN < Rows; RowN++) {
                           if (JsFltVV->Mat(RowN, ColN) != 0.0) { InitCentroidMat[i].Add(TIntFltKd(RowN, JsFltVV->Mat(RowN, ColN))); }
                       }
                   }
                   DpMeans->Apply(JsFltVV->Mat, InitCentroidMat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else {
                   DpMeans->Apply(JsFltVV->Mat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }

               DpMeans->Assign(JsFltVV->Mat, JsDpMeans->AssignV);

               TFltVV D;
               JsDpMeans->Dist->GetQuasiDistVV(JsFltVV->Mat, DpMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsDpMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsFltVV->Mat.GetCols(), "DpMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsFltVV->Mat.GetCols(), "DpMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           // input sparse matrix
           else if (JsSpVV != nullptr) {
               if (!JsDpMeans->SparseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->SparseFitMatrix.Len() && JsDpMeans->SparseFitMatrix.Len() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->SparseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->DenseFitMatrix.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->DenseFitMatrix.GetCols() && JsDpMeans->DenseFitMatrix.GetCols() <= JsDpMeans->MxClusts, "Number of columns must be in range [minClusters, maxClusters]!");
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->DenseFitMatrix, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else if (!JsDpMeans->FitIdx.Empty()) {
                   EAssertR(JsDpMeans->MnClusts <= JsDpMeans->FitIdx.Len() && JsDpMeans->FitIdx.Len() <= JsDpMeans->MxClusts, "Length of fitIdx must be in range [minClusters, maxClusters]!");
                   EAssertR(JsDpMeans->FitIdx.GetMxVal() < JsSpVV->Mat.Len(), "Maximum index in fitIdx must not be greater to the number of columns!");

                   TVec<TIntFltKdV> InitCentroidMat;
                   int Dim = JsDpMeans->MnClusts;

                   InitCentroidMat.Gen(Dim);
                   for (int i = 0; i < Dim; i++) {
                       const int ClustN = JsDpMeans->FitIdx[i];
                       InitCentroidMat[i] = JsSpVV->Mat[ClustN];
                   }
                   DpMeans->Apply(JsSpVV->Mat, InitCentroidMat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }
               else {
                   DpMeans->Apply(JsSpVV->Mat, JsDpMeans->AllowEmptyP, JsDpMeans->Iter, JsDpMeans->Notify);
               }

               DpMeans->Assign(JsSpVV->Mat, JsDpMeans->AssignV);

               TFltVV D;
               JsDpMeans->Dist->GetQuasiDistVV(JsSpVV->Mat, DpMeans->GetCentroidVV(), D);
               TLinAlg::MultiplyScalar(-1, D, D);
               TLinAlgSearch::GetColMaxIdxV(D, JsDpMeans->Medoids);

               if (JsIntV != nullptr) {
                   EAssertR(JsIntV->Vec.Len() == JsSpVV->Mat.Len(), "DpMeans.fit: IntVector RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsIntV->Vec[ColIdV[MedN]];
                   }
               }
               else if (JsArr != nullptr) {
                   EAssertR(JsArr->Vec.Len() == JsSpVV->Mat.Len(), "DpMeans.fit: Array RecordIds.length must be equal to number of columns of X!");

                   const TIntV& ColIdV = JsDpMeans->Medoids;
                   for (int MedN = 0; MedN < ColIdV.Len(); MedN++) {
                       JsDpMeans->Medoids[MedN] = JsArr->Vec[ColIdV[MedN]];
                   }
               }
           }
           else {
               throw TExcept::New("DpMeans.fit: first argument expected to be an dense or sparse matrix!");
           }
       }
       // clean up
       if (JsArr != nullptr) { delete JsArr; }
    }
    catch (const PExcept& Except) {
        // clean up
        if (JsArr != nullptr) { delete JsArr; }
        SetExcept(Except);
    }
}

void TNodeJsDpMeans::predict(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "DpMeans.predict: expects 1 argument!");
    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        throw TExcept::New("KMeans.predict: Model not initialized. First call fit!");
    }

    TIntV AssignV;
    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        const TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            JsDpMeans->GetDenseModel()->Assign(Mat, AssignV);
        }
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            JsDpMeans->GetSparseModel()->Assign(Mat, AssignV);
        }
        else {
            throw TExcept::New("KMeans.predict: invalid centroid type " + TInt::GetStr((int) JsDpMeans->CentType));
        }
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        const TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            JsDpMeans->GetDenseModel()->Assign(Mat, AssignV);
        }
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            JsDpMeans->GetSparseModel()->Assign(Mat, AssignV);
        }
        else {
            throw TExcept::New("KMeans.predict: invalid centroid type " + TInt::GetStr((int) JsDpMeans->CentType));
        }
    }
    else {
        throw TExcept::New("KMeans.predict: Argument expected to be a dense or sparse matrix!");
    }

    Args.GetReturnValue().Set(TNodeJsIntV::New(AssignV));
}

void TNodeJsDpMeans::transform(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "DpMeans.explain: Should have 1 argument!");

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());
    if (JsDpMeans->DpMeansModel == nullptr) {
        throw TExcept::New("DpMeans.explain: centroids not initialized!");
    }

    TFltVV D;
    // if argument is a dense matrix
    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        TFltVV& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0)->Mat;
        // if centroids are dense
        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            JsDpMeans->Dist->GetDistVV(JsDpMeans->GetDenseModel()->GetCentroidVV(), Mat, D);
        }
        // if centroids are sparse
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            JsDpMeans->Dist->GetDistVV(JsDpMeans->GetSparseModel()->GetCentroidVV(), Mat, D);
        }
        else {
            throw TExcept::New("DpMeans.explain: centroid type invalid " + TInt::GetStr((int) JsDpMeans->CentType));
        }
    }
    // if the argument is a sparse matrix
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        TVec<TIntFltKdV>& Mat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0)->Mat;
        // if centroids are dense
        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            JsDpMeans->Dist->GetDistVV(JsDpMeans->GetDenseModel()->GetCentroidVV(), Mat, D);
        }
        // if centroids are sparse
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            JsDpMeans->Dist->GetDistVV(JsDpMeans->GetSparseModel()->GetCentroidVV(), Mat, D);
        }
        else {
            throw TExcept::New("DpMeans.explain: centroid type invalid " + TInt::GetStr((int) JsDpMeans->CentType));
        }
    }
    else {
        throw TExcept::New("DpMeans.explain: Argument must be a dense or sparse matrix!");
    }
    Args.GetReturnValue().Set(TNodeJsFltVV::New(D));
}

void TNodeJsDpMeans::permuteCentroids(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "DpMeans.permuteCentroids: Should have 1 argument!");

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        throw TExcept::New("DpMeans.permuteCentroids: centroids not initialized!");
    }

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 0)) {

        TIntV& Mapping = TNodeJsUtil::GetArgUnwrapObj<TNodeJsIntV>(Args, 0)->Vec;
        EAssertR(Mapping.Len() == JsDpMeans->GetClusts(), "DpMeans.permuteCentroids: Length of parameter must be equal to the number of clusters!");
        EAssertR(TLinAlgSearch::GetMaxVal(Mapping) + 1 == JsDpMeans->GetClusts(), "DpMeans.permuteCentroids: maximum index of parameter must be equal to number of centroids!");

        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            JsDpMeans->GetDenseModel()->PermutateCentroids(Mapping);
        }
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            JsDpMeans->GetSparseModel()->PermutateCentroids(Mapping);
        }
        else {
            throw TExcept::New("DpMeans.permuteCentroids: centroid type invalid " + TInt::GetStr((int)JsDpMeans->CentType));
        }

        TIntV AssignTemp = JsDpMeans->AssignV;
        TIntV MedoidsTemp = JsDpMeans->Medoids;
        for (int Idx = 0; Idx < JsDpMeans->GetClusts(); Idx++) {
            JsDpMeans->AssignV[Idx] = AssignTemp[Mapping[Idx]];
            JsDpMeans->Medoids[Idx] = MedoidsTemp[Mapping[Idx]];
        }
    }
    else {
        throw TExcept::New("DpMeans.permuteCentroids: first argument should be an IntVector!");
    }
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsDpMeans::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "DpMeans.save: Should have 1 argument!");

    try {
        TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Args.Holder());
        // get output stream from argumetns
        TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
        // save model
        JsDpMeans->Save(*JsFOut->SOut);
        // return output stream for convenience
        Args.GetReturnValue().Set(Args[0]);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "DpMeans::save");
    }
}

void TNodeJsDpMeans::centroids(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Info.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        Info.GetReturnValue();
    } else {
        if (JsDpMeans->CentType == TCentroidType::ctDense) {
            Info.GetReturnValue().Set(TNodeJsFltVV::New(JsDpMeans->GetDenseModel()->GetCentroidVV()));
        }
        else if (JsDpMeans->CentType == TCentroidType::ctSparse) {
            Info.GetReturnValue().Set(TNodeJsSpMat::New(JsDpMeans->GetSparseModel()->GetCentroidVV()));
        }
        else {
            throw TExcept::New("DpMeans.centroids: Centroid type not valid " + TInt::GetStr((int)JsDpMeans->CentType));
        }
    }
}

void TNodeJsDpMeans::medoids(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Info.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        Info.GetReturnValue();
    }
    else {
        Info.GetReturnValue().Set(TNodeJsIntV::New(JsDpMeans->Medoids));
    }
}

void TNodeJsDpMeans::idxv(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Info.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        Info.GetReturnValue();
    }
    else {
        Info.GetReturnValue().Set(TNodeJsIntV::New(JsDpMeans->AssignV));
    }
}

void TNodeJsDpMeans::relMeanCentroidDist(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsDpMeans* JsDpMeans = ObjectWrap::Unwrap<TNodeJsDpMeans>(Info.Holder());

    if (JsDpMeans->DpMeansModel == nullptr) {
        Info.GetReturnValue().Set(v8::Undefined(Isolate));
    }
    else {
        switch (JsDpMeans->CentType) {
            case TCentroidType::ctDense: {
                const double RelMeanDist = JsDpMeans->GetDenseModel()->GetRelMeanCentroidDist();
                Info.GetReturnValue().Set(v8::Number::New(Isolate, RelMeanDist));
                break;
            }
            case TCentroidType::ctSparse: {
                const double RelMeanDist = JsDpMeans->GetSparseModel()->GetRelMeanCentroidDist();
                Info.GetReturnValue().Set(v8::Number::New(Isolate, RelMeanDist));
                break;
            }
            default: {
                throw TExcept::New("Unknown centroid type: " + TInt::GetStr(static_cast<int>(JsDpMeans->CentType)));
            }
        }
    }
}

const typename TNodeJsDpMeans::TDenseModel* TNodeJsDpMeans::GetDenseModel() const {
    EAssert(CentType == TCentroidType::ctDense);
    EAssert(DpMeansModel != nullptr);
    return static_cast<TDenseModel*>(DpMeansModel);
}

const typename TNodeJsDpMeans::TSparseModel* TNodeJsDpMeans::GetSparseModel() const {
    EAssert(CentType == TCentroidType::ctSparse);
    EAssert(DpMeansModel != nullptr);
    return static_cast<TSparseModel*>(DpMeansModel);
}

typename TNodeJsDpMeans::TDenseModel* TNodeJsDpMeans::GetDenseModel() {
    EAssert(CentType == TCentroidType::ctDense);
    EAssert(DpMeansModel != nullptr);
    return static_cast<TDenseModel*>(DpMeansModel);
}

typename TNodeJsDpMeans::TSparseModel* TNodeJsDpMeans::GetSparseModel() {
    EAssert(CentType == TCentroidType::ctSparse);
    EAssert(DpMeansModel != nullptr);
    return static_cast<TSparseModel*>(DpMeansModel);
}

int TNodeJsDpMeans::GetClusts() const {
    switch (CentType) {
        case TCentroidType::ctDense:
            return GetDenseModel()->GetClusts();
        case TCentroidType::ctSparse:
            return GetSparseModel()->GetClusts();
        default:
            throw TExcept::New("DpMeans: Invalid centroid type!");
    }
}

namespace TNodeJsQuant {

/////////////////////////////////////////////
// TDigest

void TNodeJsTDigest::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsTDigest>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "insert", _insert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quantile", _quantile);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "size"), _size);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "memory"), _memory);

    // properties
    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsTDigest::TNodeJsTDigest(const PJsonVal& ParamVal):
    Model(
            ParamVal->GetObjInt("clusters", 100),
            ParamVal->GetObjNum("minEps", 1e-4),
            ExtractCompressStrategy(ParamVal->GetObjStr("compression", "never")),
            ParamVal->GetObjInt("seed", 0)
    ),
    RndSeed(ParamVal->GetObjInt("seed", 0)) {}

TNodeJsTDigest::TNodeJsTDigest(TSIn& SIn): Model(SIn), RndSeed(SIn) {}

TNodeJsTDigest* TNodeJsTDigest::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsTDigest(TJsonVal::NewObj());
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsTDigest(*JsFIn->SIn);
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsTDigest(ParamVal);
    } else {
        throw TExcept::New("new TDigest: wrong arguments in constructor!");
    }
}

void TNodeJsTDigest::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsTDigest>(Args.Holder());
    const TQuant::TTDigest& Model = JsTDigest->Model;

    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("seed", JsTDigest->RndSeed);
    ParamVal->AddToObj("clusters", Model.GetMnCentroids());
    ParamVal->AddToObj("compression", ExtractStr(Model.GetCompressStrategy()));
    ParamVal->AddToObj("minEps", Model.GetMnEps());

    v8::Local<v8::Value> JsParamVal = TNodeJsUtil::ParseJson(Isolate, ParamVal);
    Args.GetReturnValue().Set(JsParamVal);
}

void TNodeJsTDigest::insert(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsTDigest>(Args.Holder());
    TQuant::TTDigest& Model = JsTDigest->Model;

    const double Val = TNodeJsUtil::GetArgFlt(Args, 0);

    // update model
    Model.Insert(Val);
    //// return output stream for convenience
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsTDigest::quantile(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsTDigest>(Args.Holder());
    const TQuant::TTDigest& TDigest = JsTDigest->Model;

    if (TNodeJsUtil::IsArgFlt(Args, 0)) {
        const double PVal = TNodeJsUtil::GetArgFlt(Args, 0);
        const double Quant = TDigest.Query(PVal);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, Quant));
    } else {
        TFltV PValV; TNodeJsUtil::GetArgFltV(Args, 0, PValV);
        TFltV QuantV; TDigest.Query(PValV, QuantV);

        v8::Handle<v8::Array> QuantArr = v8::Array::New(Isolate, QuantV.Len());
        for (int QuantN = 0; QuantN < QuantV.Len(); ++QuantN) {
            QuantArr->Set(QuantN, v8::Number::New(Isolate, QuantV[QuantN]));
        }

        Args.GetReturnValue().Set(QuantArr);
    }
}

void TNodeJsTDigest::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "TDigest.save: Should have 1 argument!");

    TNodeJsTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsTDigest>(Args.Holder());
    // get output stream from arguments
    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    // save model
    JsTDigest->Model.Save(*JsFOut->SOut);
    JsTDigest->RndSeed.Save(*JsFOut->SOut);
    // return output stream for convenience
    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsTDigest::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsTDigest>(Info.Holder());
    Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Model.GetSampleN() > 0));
}

void TNodeJsTDigest::size(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsTDigest>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Model.GetSummarySize()));
}

void TNodeJsTDigest::memory(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsTDigest>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsModel->Model.GetMemUsed()));
}

typename TNodeJsTDigest::TCompressStrategy TNodeJsTDigest::ExtractCompressStrategy(const TStr& CsStr) {
    if (CsStr == "never") {
        return TCompressStrategy::csNever;
    } else if (CsStr == "periodic") {
        return TCompressStrategy::csPeriodic;
    }  else {
        throw TExcept::New("Invalid compress strategy: " + CsStr);
    }
}

TStr TNodeJsTDigest::ExtractStr(const TCompressStrategy& Cs) {
    switch (Cs) {
    case TCompressStrategy::csNever:
        return "never";
    case TCompressStrategy::csPeriodic:
        return "periodic";
    default: {
        typename std::underlying_type<TCompressStrategy>::type CsCast = static_cast<std::underlying_type<TCompressStrategy>::type>(Cs);
        throw TExcept::New(TStr("Invalid compress strategy: ") + CsCast);
    }
    }
}

/////////////////////////////////////////////
// BufferedTDigest

void TNodeJsBuffTDigest::Init(v8::Handle<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsBuffTDigest>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "insert", _insert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quantile", _quantile);
    NODE_SET_PROTOTYPE_METHOD(tpl, "flush", _flush);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "size"), _size);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "memory"), _memory);

    // properties
    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsBuffTDigest::TNodeJsBuffTDigest(const PJsonVal& ParamVal):
    Model(
            ParamVal->GetObjInt("delta", 100),
            ParamVal->GetObjNum("bufferLen", 1000),
            ParamVal->GetObjInt("seed", 0)
    ),
    RndSeed(ParamVal->GetObjInt("seed", 0)) {}

TNodeJsBuffTDigest::TNodeJsBuffTDigest(TSIn& SIn): Model(SIn), RndSeed(SIn) {}

TNodeJsBuffTDigest* TNodeJsBuffTDigest::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsBuffTDigest(TJsonVal::NewObj());
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsBuffTDigest(*JsFIn->SIn);
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsBuffTDigest(ParamVal);
    } else {
        throw TExcept::New("new TDigest: wrong arguments in constructor!");
    }
}

void TNodeJsBuffTDigest::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBuffTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Args.Holder());
    const TTDigest& Model = JsTDigest->Model;

    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("seed", JsTDigest->RndSeed);
    ParamVal->AddToObj("delta", Model.GetDelta());
    ParamVal->AddToObj("bufferLen", Model.GetMxBuffLen());

    v8::Local<v8::Value> JsParamVal = TNodeJsUtil::ParseJson(Isolate, ParamVal);
    Args.GetReturnValue().Set(JsParamVal);
}

void TNodeJsBuffTDigest::insert(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBuffTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Args.Holder());
    TTDigest& Model = JsTDigest->Model;

    const double Val = TNodeJsUtil::GetArgFlt(Args, 0);

    // update model
    Model.Insert(Val);
    //// return output stream for convenience
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsBuffTDigest::quantile(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBuffTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Args.Holder());
    const TTDigest& TDigest = JsTDigest->Model;

    if (TNodeJsUtil::IsArgFlt(Args, 0)) {
        const double PVal = TNodeJsUtil::GetArgFlt(Args, 0);
        const double Quant = TDigest.Query(PVal);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, Quant));
    } else {
        TFltV PValV; TNodeJsUtil::GetArgFltV(Args, 0, PValV);
        TFltV QuantV; TDigest.Query(PValV, QuantV);

        v8::Handle<v8::Array> QuantArr = v8::Array::New(Isolate, QuantV.Len());
        for (int QuantN = 0; QuantN < QuantV.Len(); ++QuantN) {
            QuantArr->Set(QuantN, v8::Number::New(Isolate, QuantV[QuantN]));
        }

        Args.GetReturnValue().Set(QuantArr);
    }
}

void TNodeJsBuffTDigest::flush(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBuffTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Args.Holder());
    TTDigest& TDigest = JsTDigest->Model;
    TDigest.Flush();

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsBuffTDigest::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "TDigest.save: Should have 1 argument!");

    TNodeJsBuffTDigest* JsTDigest = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Args.Holder());
    // get output stream from arguments
    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    // save model
    JsTDigest->Model.Save(*JsFOut->SOut);
    JsTDigest->RndSeed.Save(*JsFOut->SOut);
    // return output stream for convenience
    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsBuffTDigest::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    // unwrap
    TNodeJsBuffTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Info.Holder());
    Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Model.GetSampleN() > 0));
}

void TNodeJsBuffTDigest::size(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBuffTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Model.GetSummarySize()));
}

void TNodeJsBuffTDigest::memory(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBuffTDigest* JsModel = ObjectWrap::Unwrap<TNodeJsBuffTDigest>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Model.GetMemUsed()));
}

///////////////////////////////////////////////////////
// Greenwald-Khanna quantile estimation algorithm
void TNodeJsGk::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(
            Isolate,
            TNodeJsUtil::_NewJs<TNodeJsGk>
    );
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "insert", _insert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quantile", _quantile);
    NODE_SET_PROTOTYPE_METHOD(tpl, "cdf", _cdf);
    NODE_SET_PROTOTYPE_METHOD(tpl, "kolmogorovStat", _kolmogorovStat);
    NODE_SET_PROTOTYPE_METHOD(tpl, "kolmogorovTest", _kolmogorovTest);
    NODE_SET_PROTOTYPE_METHOD(tpl, "compress", _compress);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    // properties
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "size"), _size);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "samples"), _samples);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "memory"), _memory);

    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsGk::TNodeJsGk(const PJsonVal& ParamVal):
    Gk(
            ParamVal->GetObjNum("eps", .01),
            ParamVal->GetObjBool("autoCompress", true) ?
                TQuant::TGk::TCompressStrategy::csAuto :
                TQuant::TGk::TCompressStrategy::csManual,
            ParamVal->GetObjBool("useBands", true)
    ) {}

TNodeJsGk::TNodeJsGk(TSIn& SIn): Gk(SIn) {}

TNodeJsGk* TNodeJsGk::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsGk(TJsonVal::NewObj());
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsGk(*JsFIn->SIn);
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsGk(ParamVal);
    } else {
        throw TExcept::New("new Gk: wrong arguments in constructor!");
    }
}

void TNodeJsGk::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TQuant::TGk& Gk = JsGk->Gk;

    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("eps", Gk.GetEps());
    ParamVal->AddToObj("autoCompress", Gk.GetCompressStrategy() == TQuant::TGk::TCompressStrategy::csAuto);
    ParamVal->AddToObj("useBands", Gk.GetUseBandsP());

    v8::Local<v8::Value> JsParamVal = TNodeJsUtil::ParseJson(Isolate, ParamVal);
    Args.GetReturnValue().Set(JsParamVal);
}

void TNodeJsGk::insert(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    TQuant::TGk& Gk = JsGk->Gk;

    const double Val = TNodeJsUtil::GetArgFlt(Args, 0);
    Gk.Insert(Val);

    // return self
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsGk::quantile(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TQuant::TGk& Gk = JsGk->Gk;

    if (TNodeJsUtil::IsArgFlt(Args, 0)) {
        const double CdfVal = TNodeJsUtil::GetArgFlt(Args, 0);
        const double Quant = Gk.GetQuantile(CdfVal);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, Quant));
    } else {
        TFltV CdfValV; TNodeJsUtil::GetArgFltV(Args, 0, CdfValV);
        TFltV QuantV; Gk.GetQuantileV(CdfValV, QuantV);

        v8::Local<v8::Array> QuantArr = v8::Array::New(Isolate, QuantV.Len());
        for (int QuantN = 0; QuantN < QuantV.Len(); ++QuantN) {
            QuantArr->Set(QuantN, v8::Number::New(Isolate, QuantV[QuantN]));
        }

        Args.GetReturnValue().Set(QuantArr);
    }
}

void TNodeJsGk::cdf(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TQuant::TGk& Gk = JsGk->Gk;

    if (TNodeJsUtil::IsArgFlt(Args, 0)) {
        const double Val = TNodeJsUtil::GetArgFlt(Args, 0);
        const double CdfVal = Gk.GetQuantile(Val);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, CdfVal));
    } else {
        TFltV ValV; TNodeJsUtil::GetArgFltV(Args, 0, ValV);
        TFltV CdfValV; Gk.GetQuantileV(ValV, CdfValV);

        v8::Handle<v8::Array> QuantArr = v8::Array::New(Isolate, CdfValV.Len());
        for (int QuantN = 0; QuantN < CdfValV.Len(); ++QuantN) {
            QuantArr->Set(QuantN, v8::Number::New(Isolate, CdfValV[QuantN]));
        }

        Args.GetReturnValue().Set(QuantArr);
    }
}

void TNodeJsGk::kolmogorovStat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TNodeJsGk* OtherJsGk = TNodeJsUtil::GetArgUnwrapObj<TNodeJsGk>(Args, 0);

    const TQuant::TGk& Gk1 = JsGk->Gk;
    const TQuant::TGk& Gk2 = OtherJsGk->Gk;

    const double Stat = TQuant::TStat::KolmogorovSmirnov(Gk1, Gk2);

    Args.GetReturnValue().Set(v8::Number::New(Isolate, Stat));
}

void TNodeJsGk::kolmogorovTest(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TNodeJsGk* OtherJsGk = TNodeJsUtil::GetArgUnwrapObj<TNodeJsGk>(Args, 0);
    const double Alpha = TNodeJsUtil::GetArgFlt(Args, 1);

    const TQuant::TGk& Gk1 = JsGk->Gk;
    const TQuant::TGk& Gk2 = OtherJsGk->Gk;

    const bool Result = TQuant::TStat::KolmogorovSmirnovTest(Gk1, Gk2, Alpha);

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, Result));
}


void TNodeJsGk::compress(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    TQuant::TGk& Gk = JsGk->Gk;

    Gk.Compress();

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsGk::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGk* JsGk = ObjectWrap::Unwrap<TNodeJsGk>(Args.Holder());
    const TQuant::TGk& Gk = JsGk->Gk;

    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    Gk.Save(*JsFOut->SOut);

    // return the output stream
    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsGk::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsModel = ObjectWrap::Unwrap<TNodeJsGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Gk.GetSampleN() > 0));
}

void TNodeJsGk::size(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsModel = ObjectWrap::Unwrap<TNodeJsGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Gk.GetSummarySize()));
}

void TNodeJsGk::samples(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsModel = ObjectWrap::Unwrap<TNodeJsGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Gk.GetSampleN()));
}

void TNodeJsGk::memory(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsGk* JsModel = ObjectWrap::Unwrap<TNodeJsGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int) JsModel->Gk.GetMemUsed()));
}


////////////////////////////////////////////
// CKMS algorithm for biased quantiles
void TNodeJsBiasedGk::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(
            Isolate,
            TNodeJsUtil::_NewJs<TNodeJsBiasedGk>
    );
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "insert", _insert);
    NODE_SET_PROTOTYPE_METHOD(tpl, "quantile", _quantile);
    NODE_SET_PROTOTYPE_METHOD(tpl, "compress", _compress);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    // properties
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "init"), _init);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "size"), _size);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "samples"), _samples);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "memory"), _memory);

    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsBiasedGk::TNodeJsBiasedGk(const PJsonVal& ParamVal):
        Gk(
                ParamVal->GetObjNum("targetProb", 0.01),
                ParamVal->GetObjNum("eps", 0.1),
                ExtractCompressStrategy(ParamVal),
                ParamVal->GetObjBool("useBands", true)
        ) {}

TNodeJsBiasedGk::TNodeJsBiasedGk(TSIn& SIn): Gk(SIn) {}

TNodeJsBiasedGk* TNodeJsBiasedGk::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsBiasedGk(TJsonVal::NewObj());
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsBiasedGk(*JsFIn->SIn);
    } else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsBiasedGk(ParamVal);
    } else {
        throw TExcept::New("new BiasedGk: wrong arguments in constructor!");
    }
}

void TNodeJsBiasedGk::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBiasedGk* JsGk = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Args.Holder());
    const TQuant::TBiasedGk& Gk = JsGk->Gk;

    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("eps", Gk.GetEps());
    ParamVal->AddToObj("targetProb", Gk.GetPVal0());
    ParamVal->AddToObj("useBands", Gk.GetUseBands());

    switch (Gk.GetCompressStrategy()) {
    case TQuant::TBiasedGk::TCompressStrategy::csManual: {
        ParamVal->AddToObj("compression", "manual");
        break;
    }
    case TQuant::TBiasedGk::TCompressStrategy::csAggressive: {
        ParamVal->AddToObj("compression", "aggressive");
        break;
    }
    case TQuant::TBiasedGk::TCompressStrategy::csPeriodic: {
        ParamVal->AddToObj("compression", "periodic");
        break;
    }
    default: {
        throw TExcept::New("BiasedGk: Unknown compression strategy!");
    }
    }

    v8::Local<v8::Value> JsParamVal = TNodeJsUtil::ParseJson(Isolate, ParamVal);
    Args.GetReturnValue().Set(JsParamVal);
}

void TNodeJsBiasedGk::insert(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBiasedGk* JsGk = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Args.Holder());
    TQuant::TBiasedGk& Gk = JsGk->Gk;

    const double Val = TNodeJsUtil::GetArgFlt(Args, 0);
    Gk.Insert(Val);

    // return self
    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsBiasedGk::quantile(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBiasedGk* JsGk = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Args.Holder());
    const TQuant::TBiasedGk& Gk = JsGk->Gk;

    if (TNodeJsUtil::IsArgFlt(Args, 0)) {
        const double PVal = TNodeJsUtil::GetArgFlt(Args, 0);
        const double Quant = Gk.GetQuantile(PVal);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, Quant));
    } else {
        TFltV PValV; TNodeJsUtil::GetArgFltV(Args, 0, PValV);
        TFltV QuantV; Gk.GetQuantileV(PValV, QuantV);

        v8::Local<v8::Array> QuantArr = v8::Array::New(Isolate, QuantV.Len());
        for (int QuantN = 0; QuantN < QuantV.Len(); ++QuantN) {
            QuantArr->Set(QuantN, v8::Number::New(Isolate, QuantV[QuantN]));
        }

        Args.GetReturnValue().Set(QuantArr);
    }
}

void TNodeJsBiasedGk::compress(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBiasedGk* JsGk = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Args.Holder());
    TQuant::TBiasedGk& Gk = JsGk->Gk;

    Gk.Compress();

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsBiasedGk::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsBiasedGk* JsGk = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Args.Holder());
    const TQuant::TBiasedGk& Gk = JsGk->Gk;

    TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
    EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
    Gk.Save(*JsFOut->SOut);

    // return the output stream
    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsBiasedGk::init(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBiasedGk* JsModel = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Boolean::New(Isolate, JsModel->Gk.GetSampleN() > 0));
}

void TNodeJsBiasedGk::size(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBiasedGk* JsModel = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Gk.GetSummarySize()));
}

void TNodeJsBiasedGk::samples(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBiasedGk* JsModel = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsModel->Gk.GetSampleN()));
}

void TNodeJsBiasedGk::memory(v8::Local<v8::Name> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    const TNodeJsBiasedGk* JsModel = ObjectWrap::Unwrap<TNodeJsBiasedGk>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, (int)JsModel->Gk.GetMemUsed()));
}

TQuant::TBiasedGk::TCompressStrategy TNodeJsBiasedGk::ExtractCompressStrategy(const PJsonVal& ParamVal) {
    const TStr CompressStr = ParamVal->GetObjStr("compression", "periodic");
    if (CompressStr == "manual") {
        return TQuant::TBiasedGk::TCompressStrategy::csManual;
    } else if (CompressStr == "aggressive") {
        return TQuant::TBiasedGk::TCompressStrategy::csAggressive;
    } else if (CompressStr == "periodic") {
        return TQuant::TBiasedGk::TCompressStrategy::csPeriodic;
    } else {
        throw TExcept::New("BiasedGk: Invalid compression strategy string: " + CompressStr);
    }
}

}

/////////////////////////////////////////////
// Recommender System

TNodeJsRecommenderSys::TNodeJsRecommenderSys(const PJsonVal& ParamVal) :
    Iter(10000),
    K(2),
    Tol(1e-3),
    Verbose(false),
    Notify(TNotify::NullNotify()) {
    UpdateParams(ParamVal);
}

TNodeJsRecommenderSys::TNodeJsRecommenderSys(TSIn& SIn) :
    Iter(TInt(SIn)),
    K(TInt(SIn)),
    Tol(TFlt(SIn)),
    Verbose(TBool(SIn)),
    U(SIn),
    V(SIn) {
    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

void TNodeJsRecommenderSys::UpdateParams(const PJsonVal& ParamVal) {
    if (ParamVal->IsObjKey("iter")) { Iter = ParamVal->GetObjInt("iter"); }
    if (ParamVal->IsObjKey("k")) { K = ParamVal->GetObjInt("k"); }
    if (ParamVal->IsObjKey("tol")) { Tol = ParamVal->GetObjNum("tol"); }
    if (ParamVal->IsObjKey("verbose")) { Verbose = ParamVal->GetObjBool("verbose"); }

    Notify = Verbose ? TQm::TEnv::Debug() : TNotify::NullNotify();
}

PJsonVal TNodeJsRecommenderSys::GetParams() const {
    PJsonVal ParamVal = TJsonVal::NewObj();
    ParamVal->AddToObj("iter", Iter);
    ParamVal->AddToObj("k", K);
    ParamVal->AddToObj("tol", Tol);
    ParamVal->AddToObj("verbose", Verbose);

    return ParamVal;
}

void TNodeJsRecommenderSys::Save(TSOut& SOut) const {
    TInt(Iter).Save(SOut);
    TInt(K).Save(SOut);
    TFlt(Tol).Save(SOut);
    TBool(Verbose).Save(SOut);
    U.Save(SOut);
    V.Save(SOut);
}

void TNodeJsRecommenderSys::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsRecommenderSys>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "getParams", _getParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "setParams", _setParams);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getModel", _getModel);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fit", _fit);
    NODE_SET_PROTOTYPE_METHOD(tpl, "fitAsync", _fitAsync);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);

    // properties
    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsRecommenderSys* TNodeJsRecommenderSys::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 0) {
        // create new model with default parameters
        return new TNodeJsRecommenderSys(TJsonVal::NewObj());
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgWrapObj<TNodeJsFIn>(Args, 0)) {
        // load the model from the input stream
        TNodeJsFIn* JsFIn = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFIn>(Args, 0);
        return new TNodeJsRecommenderSys(*JsFIn->SIn);
    }
    else if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsRecommenderSys(ParamVal);
    }
    else {
        throw TExcept::New("new RecommenderSys: wrong arguments in constructor!");
    }
}

void TNodeJsRecommenderSys::getParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 0, "RecommenderSys.getParams: takes 0 argument!");

    try {
        TNodeJsRecommenderSys* JsRecSys = TNodeJsRecommenderSys::Unwrap<TNodeJsRecommenderSys>(Args.Holder());
        Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, JsRecSys->GetParams()));
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "RecommenderSys::getParams");
    }
}

void TNodeJsRecommenderSys::setParams(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "RecommenderSys.setParams: takes 1 argument!");
    EAssertR(TNodeJsUtil::IsArgJson(Args, 0), "RecommenderSys.setParams: first argument should be a Javascript object!");

    try {
        TNodeJsRecommenderSys* JsRecSys = ObjectWrap::Unwrap<TNodeJsRecommenderSys>(Args.Holder());
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);

        JsRecSys->UpdateParams(ParamVal);

        Args.GetReturnValue().Set(Args.Holder());
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "RecommenderSys::setParams");
    }
}

void TNodeJsRecommenderSys::getModel(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 0, "RecommenderSys.setParams: takes 0 arguments!");

    TNodeJsRecommenderSys* JsRecSys = ObjectWrap::Unwrap<TNodeJsRecommenderSys>(Args.Holder());

    v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result
    JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "U")), TNodeJsFltVV::New(JsRecSys->U));
    JsObj->Set(v8::Local<v8::String>(v8::String::NewFromUtf8(Isolate, "V")), TNodeJsFltVV::New(JsRecSys->V));
    Args.GetReturnValue().Set(JsObj);
}

TNodeJsRecommenderSys::TFitTask::TFitTask(const v8::FunctionCallbackInfo<v8::Value>& Args, const bool& IsAsync) :
    TNodeTask(Args, IsAsync),
    JsRecSys(nullptr),
    JsFltVV(nullptr),
    JsSpVV(nullptr) {

    JsRecSys = ObjectWrap::Unwrap<TNodeJsRecommenderSys>(Args.Holder());

    if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
        JsFltVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
    }
    else if (TNodeJsUtil::IsArgWrapObj<TNodeJsSpMat>(Args, 0)) {
        JsSpVV = TNodeJsUtil::GetArgUnwrapObj<TNodeJsSpMat>(Args, 0);
    }
    else {
        throw TExcept::New("RecommenderSys.fit: argument not a sparse or dense matrix!");
    }
}

v8::Local<v8::Function> TNodeJsRecommenderSys::TFitTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    if (TNodeJsUtil::IsArgFun(Args, 1)) {
        return TNodeJsUtil::GetArgFun(Args, 1);
    }
    else {
        return TNodeJsUtil::GetArgFun(Args, 2);
    }
}

void TNodeJsRecommenderSys::TFitTask::Run() {
    try {
        // if argument is a dense matrix
        if (JsFltVV != nullptr) {
            TNmf::WeightedCFO(JsFltVV->Mat, JsRecSys->K, JsRecSys->U, JsRecSys->V, JsRecSys->Iter, JsRecSys->Tol,
                JsRecSys->Notify);
        }
        // if argument is a sparse matrix
        else if (JsSpVV != nullptr) {
            TNmf::WeightedCFO(JsSpVV->Mat, JsRecSys->K, JsRecSys->U, JsRecSys->V, JsRecSys->Iter, JsRecSys->Tol,
                JsRecSys->Notify);
        }
        else {
            throw TExcept::New("RecommenderSys.fit: argument not a sparse or dense matrix");
        }
    }
    catch (const PExcept& Except) {
        SetExcept(Except);
    }
}

void TNodeJsRecommenderSys::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1, "RecommenderSys.save: Should have 1 argument!");

    try {
        TNodeJsRecommenderSys* JsRecSys = ObjectWrap::Unwrap<TNodeJsRecommenderSys>(Args.Holder());
        // get output stream from arguments
        TNodeJsFOut* JsFOut = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFOut>(Args, 0);
        EAssertR(!JsFOut->SOut.Empty(), "Output stream closed!");
        // save model
        JsRecSys->Save(*JsFOut->SOut);
        // return output stream for convenience
        Args.GetReturnValue().Set(Args[0]);
    }
    catch (const PExcept& Except) {
        throw TExcept::New(Except->GetMsgStr(), "RecommenderSys::save");
    }
}

/////////////////////////////////////////////
// QMiner-JavaScript-Graph-Cascade


void TNodeJsGraphCascade::Init(v8::Local<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsGraphCascade>);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "observeNode", _observeNode);
    NODE_SET_PROTOTYPE_METHOD(tpl, "computePosterior", _computePosterior);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getPosterior", _getPosterior);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getGraph", _getGraph);
    NODE_SET_PROTOTYPE_METHOD(tpl, "getOrder", _getOrder);

    exports->Set(v8::String::NewFromUtf8(Isolate, GetClassId().CStr()), tpl->GetFunction());
}

TNodeJsGraphCascade* TNodeJsGraphCascade::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    if (Args.Length() == 1 && TNodeJsUtil::IsArgObj(Args, 0)) {
        // create new model from given parameters
        PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
        return new TNodeJsGraphCascade(ParamVal);
    } else {
        throw TExcept::New("new TNodeJsGraphCascade: wrong arguments in constructor!");
    }
}

void TNodeJsGraphCascade::observeNode(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    TStr NodeNm = TNodeJsUtil::GetArgStr(Args, 0);
    uint64 TmMSecs = TNodeJsUtil::GetArgTmMSecs(Args, 1);

    TNodeJsGraphCascade* JsGraphCascade = ObjectWrap::Unwrap<TNodeJsGraphCascade>(Args.Holder());
    JsGraphCascade->Model.ObserveNode(NodeNm, TmMSecs);
}

void TNodeJsGraphCascade::computePosterior(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    uint64 TmMSecs = TNodeJsUtil::GetArgTmMSecs(Args, 0);
    int SampleSize = TNodeJsUtil::GetArgInt32(Args, 1, 10000);

    TNodeJsGraphCascade* JsGraphCascade = ObjectWrap::Unwrap<TNodeJsGraphCascade>(Args.Holder());
    JsGraphCascade->Model.ComputePosterior(TmMSecs, SampleSize);
}

void TNodeJsGraphCascade::getPosterior(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGraphCascade* JsGraphCascade = ObjectWrap::Unwrap<TNodeJsGraphCascade>(Args.Holder());
    PJsonVal ParamVal = TJsonVal::NewObj();
    if (Args.Length() > 0 && TNodeJsUtil::IsArgObj(Args, 0)) {
        ParamVal = TNodeJsUtil::GetArgJson(Args, 0);
    }
    TStrV NodeNmV;
    if (ParamVal->IsObjKey("nodes")) {
        ParamVal->GetObjKey("nodes")->GetArrStrV(NodeNmV);
    }
    TFltV QuantileV;
    if (ParamVal->IsObjKey("quantiles")) {
        ParamVal->GetObjKey("quantiles")->GetArrNumV(QuantileV);
    } else {
        QuantileV.Add(0.1);
        QuantileV.Add(0.5);
        QuantileV.Add(0.9);
    }
    PJsonVal Posterior = JsGraphCascade->Model.GetPosterior(NodeNmV, QuantileV);
    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Posterior));
}

void TNodeJsGraphCascade::getGraph(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGraphCascade* JsGraphCascade = ObjectWrap::Unwrap<TNodeJsGraphCascade>(Args.Holder());
    PJsonVal Graph = JsGraphCascade->Model.GetGraph();
    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, Graph));
}

void TNodeJsGraphCascade::getOrder(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsGraphCascade* JsGraphCascade = ObjectWrap::Unwrap<TNodeJsGraphCascade>(Args.Holder());
    PJsonVal GraphArr = JsGraphCascade->Model.GetOrder();
    Args.GetReturnValue().Set(TNodeJsUtil::ParseJson(Isolate, GraphArr));
}

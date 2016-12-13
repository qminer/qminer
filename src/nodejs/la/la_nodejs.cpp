/**
* Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
* All rights reserved.
*
* This source code is licensed under the FreeBSD license found in the
* LICENSE file in the root directory of this source tree.
*/
#include "la_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-LinAlg 
//
// These functions play the role of TJsLinAlg in QMiner JS API 
// Implement them in Javascript!
// 

///////////////////////////////
// NodeJs-Qminer-LinAlg
void TNodeJsLinAlg::Init(v8::Handle<v8::Object> exports) {
	NODE_SET_METHOD(exports, "svd", _svd);
	NODE_SET_METHOD(exports, "svdAsync", _svdAsync);
	NODE_SET_METHOD(exports, "qr", _qr);
}

TNodeJsLinAlg::TSVDTask::TSVDTask(const v8::FunctionCallbackInfo<v8::Value>& Args) :
        TNodeTask(Args),
        JsFltVV(nullptr),
        JsSpVV(nullptr),
        U(nullptr),
        V(nullptr),
        s(nullptr),
        Iters(-1),
        Tol(1e-6) {

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

	if (Args.Length() >= 4) {
		PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 2);
		Iters = ParamVal->GetObjInt("iter", -1);
		Tol = ParamVal->GetObjNum("tol", 1e-6);
	}

	U = new TNodeJsFltVV();
	V = new TNodeJsFltVV();
	s = new TNodeJsFltV();
}

v8::Handle<v8::Function> TNodeJsLinAlg::TSVDTask::GetCallback(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	if (TNodeJsUtil::IsArgFun(Args, 2)) {
		return TNodeJsUtil::GetArgFun(Args, 2);
	}
	else {
		return TNodeJsUtil::GetArgFun(Args, 3);
	}
}

void TNodeJsLinAlg::TSVDTask::Run() {
	try {
		TFltVV& URef = U->Mat;
		TFltVV& VRef = V->Mat;
		TFltV& sRef = s->Vec;
		if (JsFltVV != nullptr) {
			TFullMatrix Mat(JsFltVV->Mat, true);	// only wrap the matrix
			TLinAlg::ComputeThinSVD(Mat, k, URef, sRef, VRef, Iters, Tol);
		}
		else if (JsSpVV != nullptr) {
			if (JsSpVV->Rows != -1) {
				TSparseColMatrix Mat(JsSpVV->Mat, JsSpVV->Rows, JsSpVV->Mat.Len());
				TLinAlg::ComputeThinSVD(Mat, k, URef, sRef, VRef, Iters, Tol);
			}
			else {
				TSparseColMatrix Mat(JsSpVV->Mat);
				TLinAlg::ComputeThinSVD(Mat, k, URef, sRef, VRef, Iters, Tol);
			}
		}
		else {
			throw TExcept::New("svd: expects dense or sparse matrix!");
		}
	}
	catch (const PExcept& Except) {
		delete U;
		delete V;
		delete s;
		SetExcept(Except);
	}
}

v8::Local<v8::Value> TNodeJsLinAlg::TSVDTask::WrapResult() {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);

	v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result 
	JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "U")), TNodeJsUtil::NewInstance(U));
	JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "V")), TNodeJsUtil::NewInstance(V));
	JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "s")), TNodeJsUtil::NewInstance(s));
	return HandleScope.Escape(JsObj);
}

void TNodeJsLinAlg::qr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	v8::Handle<v8::Object> JsObj = v8::Object::New(Isolate); // Result 
	TFltVV Q;
	TFltVV R;
	double Tol = TNodeJsUtil::GetArgFlt(Args, 1, 1e-6);
	if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltVV>(Args, 0)) {
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TLinAlg::QR(JsMat->Mat, Q, R, Tol);
	}
	JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "Q")), TNodeJsFltVV::New(Q));
	JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "R")), TNodeJsFltVV::New(R));
	Args.GetReturnValue().Set(JsObj);
}

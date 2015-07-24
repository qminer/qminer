/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "stat_nodejs.h"

///////////////////////////////
// NodeJs-QMiner-Stat

void TNodeJsStat::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// Add all methods, getters and setters here.
	NODE_SET_METHOD(exports, "mean", _mean);
	NODE_SET_METHOD(exports, "std", _std);
	NODE_SET_METHOD(exports, "zscore", _zscore);
}

void TNodeJsStat::mean(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	EAssertR(Args.Length() != 0, "Error using stat.mean function. Not enough input arguments.");
	EAssertR(Args[0]->IsObject() && (TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltV::GetClassId().CStr()) || TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltVV::GetClassId())), 
		"Error using stat.std function. First argument should be la.vector or  la.matrix.");
	
	// Dim parameter
	int Dim = TNodeJsUtil::GetArgInt32(Args, 1, 1); // Default dim is 1

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId().CStr())) {
		// If input argument is vec
		//TNodeJsVec* Test = ObjectWrap::Unwrap<TNodeJsVec>(Args[0]->ToObject()); 
		TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap< TNodeJsVec< TFlt, TAuxFltV > >(Args[0]->ToObject());
		Args.GetReturnValue().Set(v8::Number::New(Isolate, TLAMisc::Mean(JsVec->Vec)));
		return;
	}
	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId())) {
		//If input argument is matrix
		TFltV Vec;
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TLAMisc::Mean(JsMat->Mat, Vec, Dim);
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Vec));
		return;
	}
}

void TNodeJsStat::std(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	EAssertR(Args.Length() != 0, "Error using stat.std function. Not enough input arguments.");
	EAssertR(Args[0]->IsObject() && (TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltV::GetClassId().CStr()) || TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltVV::GetClassId())),
		"Error using stat.std function. First argument should be la.vector or la.matrix.");

	int Flag = TNodeJsUtil::GetArgInt32(Args, 1, 0); // Default flag is 0
	int Dim = TNodeJsUtil::GetArgInt32(Args, 2, 1); // Default dim is 1

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId().CStr())) {
		// If input argument is vec
		// TODO
		throw TExcept::New("stat.std(vec) not implemented yet. Use stat.std(mat).");
	}
	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId())) {
		//If input argument is matrix
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TFltV Res;
		TLAMisc::Std(JsMat->Mat, Res, Flag, Dim);

		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
		return;
	}
}

void TNodeJsStat::zscore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	EAssertR(Args.Length() != 0, "Error using stat.zscore function. Not enough input arguments.");
	EAssertR(Args[0]->IsObject() && TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltVV::GetClassId()), "Error using stat.zscore function. First argument should be a matrix.");

	int Flag = TNodeJsUtil::GetArgInt32(Args, 1, 0); // Default flag is 0
	int Dim = TNodeJsUtil::GetArgInt32(Args, 2, 1); // Default dim is 1

	TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
	v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result
	// algorithm outputs
	TFltVV Z;
	TFltV mu;
	TFltV sigma;

	TLAMisc::ZScore(JsMat->Mat, Z, Flag, Dim);
	TLAMisc::Mean(JsMat->Mat, mu, Dim);
	TLAMisc::Std(JsMat->Mat, sigma, Flag, Dim);

	JsObj->Set(v8::String::NewFromUtf8(Isolate, "Z"), TNodeJsFltVV::New(Z));
	JsObj->Set(v8::String::NewFromUtf8(Isolate, "mu"), TNodeJsVec< TFlt, TAuxFltV >::New(mu));
	JsObj->Set(v8::String::NewFromUtf8(Isolate, "sigma"), TNodeJsVec< TFlt, TAuxFltV >::New(sigma));
	Args.GetReturnValue().Set(JsObj);
}

/////////////////////////////////
//// Register functions, etc.
//void init(v8::Handle<v8::Object> exports) {
//   TNodeJsStat::Init(exports);
//   // LA
//   TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
//   TNodeJsVec<TInt, TAuxIntV>::Init(exports);
//   TNodeJsFltVV::Init(exports);
//   TNodeJsSpVec::Init(exports);
//   TNodeJsSpMat::Init(exports);
//}
//
//NODE_MODULE(statistics, init)

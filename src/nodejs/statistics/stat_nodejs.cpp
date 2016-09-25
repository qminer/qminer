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
	NODE_SET_METHOD(exports, "studentCdf", _studentCdf);
}

void TNodeJsStat::mean(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	EAssertR(Args.Length() != 0, "Error using stat.mean function. Not enough input arguments.");
	EAssertR(Args[0]->IsObject() && (TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltV::GetClassId().CStr()) || TNodeJsUtil::IsClass(Args[0]->ToObject(), TNodeJsFltVV::GetClassId())), 
		"Error using stat.std function. First argument should be la.vector or la.matrix.");
	
	// Dim parameter
	int Dim = TNodeJsUtil::GetArgInt32(Args, 1, 1); // Default dim is 1
	EAssertR((Dim == 1 || Dim == 2), "Error using stat.mean function. Dim must be 1 (col mean) or 2 (row mean)");

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId())) {
		// If input argument is vec
		//TNodeJsVec* Test = ObjectWrap::Unwrap<TNodeJsVec>(Args[0]->ToObject()); 
		TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap< TNodeJsVec< TFlt, TAuxFltV > >(Args[0]->ToObject());
		Args.GetReturnValue().Set(v8::Number::New(Isolate, TLinAlgStat::Mean(JsVec->Vec)));
		return;
	}
	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId())) {
		//If input argument is matrix
		TFltV Vec;
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TLinAlgStat::Mean(JsMat->Mat, Vec, Dim == 1 ? TMatDim::mdCols : TMatDim::mdRows);
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
	EAssertR((Dim == 1 || Dim == 2), "Error using stat.std function. Dim must be 1 (col mean) or 2 (row mean)");

	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltV::GetClassId().CStr())) {
		// If input argument is vec
        TNodeJsFltV* JsVec = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltV>(Args, 0);
        const double Result = TLinAlgStat::Std(JsVec->Vec, Flag);

        Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
        return;
	}
	if (TNodeJsUtil::IsArgWrapObj(Args, 0, TNodeJsFltVV::GetClassId())) {
		//If input argument is matrix
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TFltV Res;
		const TMatDim CalcDim = Dim == 1 ? TMatDim::mdCols : TMatDim::mdRows;
		TLinAlgStat::Std(JsMat->Mat, Res, Flag, CalcDim);

		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
		return;
	}
}

void TNodeJsStat::zscore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	
	TNodeJsFltVV* JsMat = TNodeJsUtil::GetArgUnwrapObj<TNodeJsFltVV>(Args, 0);
	int Flag = TNodeJsUtil::GetArgInt32(Args, 1, 0); // Default flag is 0
	int Dim = TNodeJsUtil::GetArgInt32(Args, 2, 1); // Default dim is 1
	EAssertR((Dim == 1 || Dim == 2), "Error using stat.zscore function. Dim must be 1 (col mean) or 2 (row mean)");

	v8::Local<v8::Object> JsObj = v8::Object::New(Isolate); // Result
	// algorithm outputs
	TFltVV Z;
	TFltV mu;
	TFltV sigma;

	const TMatDim CalcDim = Dim == 1 ? TMatDim::mdCols : TMatDim::mdRows;
	TLinAlgStat::ZScore(JsMat->Mat, Z, Flag, CalcDim);
	TLinAlgStat::Mean(JsMat->Mat, mu, CalcDim);
	TLinAlgStat::Std(JsMat->Mat, sigma, Flag, CalcDim);

	JsObj->Set(v8::String::NewFromUtf8(Isolate, "Z"), TNodeJsFltVV::New(Z));
	JsObj->Set(v8::String::NewFromUtf8(Isolate, "mu"), TNodeJsVec< TFlt, TAuxFltV >::New(mu));
	JsObj->Set(v8::String::NewFromUtf8(Isolate, "sigma"), TNodeJsVec< TFlt, TAuxFltV >::New(sigma));
	Args.GetReturnValue().Set(JsObj);
}

void TNodeJsStat::studentCdf(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2 || Args.Length() == 4, "TNodeJsStat::studentCdf: expects 2 or 4 arguments!");

	if (Args.Length() == 2) {
		const double TVal = TNodeJsUtil::GetArgFlt(Args, 0);
		const int Df = TNodeJsUtil::GetArgInt32(Args, 1);

		Args.GetReturnValue().Set(v8::Number::New(Isolate, TSpecFunc::StudentCdf(TVal, Df)));
	}
	else {
		const double Val = TNodeJsUtil::GetArgFlt(Args, 0);
		const double Mean = TNodeJsUtil::GetArgFlt(Args, 1);
		const double Std = TNodeJsUtil::GetArgFlt(Args, 2);
		const int Df = TNodeJsUtil::GetArgInt32(Args, 3);

		Args.GetReturnValue().Set(v8::Number::New(Isolate, TSpecFunc::StudentCdf(Val, Mean, Std, Df)));
	}
}

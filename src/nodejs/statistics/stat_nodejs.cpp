/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
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
	EAssertR(Args[0]->IsObject() && (TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltV") || TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltVV")), 
		"Error using stat.std function. First argument should be la.vector or  la.matrix.");
	
	// Dim parameter
	double Dim = TNodeJsUtil::GetArgFlt(Args, 1, 1); // Default dim is 1

	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
		// If input argument is vec
		//TNodeJsVec* Test = ObjectWrap::Unwrap<TNodeJsVec>(Args[0]->ToObject()); 
		TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap< TNodeJsVec< TFlt, TAuxFltV > >(Args[0]->ToObject());
		Args.GetReturnValue().Set(v8::Number::New(Isolate, TLAMisc::Mean(JsVec->Vec)));
		return;
	}
	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
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
	EAssertR(Args[0]->IsObject() && (TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltV") || TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltVV")),
		"Error using stat.std function. First argument should be la.vector or la.matrix.");

	double Flag = TNodeJsUtil::GetArgFlt(Args, 1, 0); // Default flag is 0
	double Dim = TNodeJsUtil::GetArgFlt(Args, 2, 1); // Default dim is 1

	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
		// If input argument is vec
		// TODO
		throw TExcept::New("stat.std(vec) not implemented yet. Use stat.std(mat).");
	}
	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
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
	EAssertR(Args[0]->IsObject() && TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltVV"), "Error using stat.zscore function. First argument should be a matrix.");

	double Flag = TNodeJsUtil::GetArgFlt(Args, 1, 0); // Default flag is 0
	double Dim = TNodeJsUtil::GetArgFlt(Args, 2, 1); // Default dim is 1

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

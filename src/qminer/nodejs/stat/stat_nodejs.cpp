#include "stat_nodejs.h"

///////////////////////////////
// QMiner-Stat

void TNodeJsStat::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	// Add all methods, getters and setters here.
	//NODE_SET_METHOD(exports, "svd", svd);
	NODE_SET_METHOD(exports, "mean", _mean);
	NODE_SET_METHOD(exports, "std", _std);
	NODE_SET_METHOD(exports, "zscore", _zscore);
}

void TNodeJsStat::mean(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	// Dim parameter
	double Dim = TNodeJsUtil::GetArgFlt(Args, 1, 1); // Default dim is 1

	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
		printf("Im in TFltV!!");
		EAssertR(Args[0]->IsObject() && TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltV"), "TNodeJsStore constructor expecting store name and base object as arguments");
		// If input argument is vec
		//TNodeJsVec* Test = ObjectWrap::Unwrap<TNodeJsVec>(Args[0]->ToObject()); // Doesent work
		TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap< TNodeJsVec< TFlt, TAuxFltV > >(Args[0]->ToObject());
		Args.GetReturnValue().Set(v8::Number::New(Isolate, TLAMisc::Mean(JsVec->Vec)));
		return;
	}
	if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
		printf("Im in TFltVV!!");
		EAssertR(Args[0]->IsObject() && TNodeJsUtil::IsClass(Args[0]->ToObject(), "TFltVV"), "TNodeJsStore constructor expecting store name and base object as arguments");
		 //If input argument is matrix
		TFltV Vec;
		TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
		TLAMisc::Mean(JsMat->Mat, Vec, Dim);
		Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Vec));
		return;
	}
	printf("Im at the end!!");
	//throw TExcept::New("la.mean() can take only matrix, or vector as first input argument.");
	EAssertR(false, "la.mean() can take only matrix, or vector as first input argument.");
}

void TNodeJsStat::std(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

void TNodeJsStat::zscore(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	//Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
   TNodeJsStat::Init(exports);
   // LA
   TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
   TNodeJsVec<TInt, TAuxIntV>::Init(exports);
}

NODE_MODULE(stat, init) // Could this be a problem? Is stat allready reserved name??

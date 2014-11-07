#include "la_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-LinAlg 
//
// These functions play the role of TJsLinAlg in QMiner JS API 
// Implement them in Javascript!
// 

const TStr TAuxFltV::ClassId = "FltV";
const TStr TAuxIntV::ClassId = "IntV";

///////////////////////////////
// NodeJs-GLib-TVec
template <typename TVal, typename TAux>
v8::Persistent<v8::Function> TNodeJsVec<TVal, TAux>::constructor;

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::Init(v8::Handle<v8::Object> exports) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();

   v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
   tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "vector"));
   // ObjectWrap uses the first internal field to store the wrapped pointer.
   tpl->InstanceTemplate()->SetInternalFieldCount(1);
   
   // Add all prototype methods, getters and setters here.
   NODE_SET_PROTOTYPE_METHOD(tpl, "newIntVec", newIntVec);
   NODE_SET_PROTOTYPE_METHOD(tpl, "at", at);
   NODE_SET_PROTOTYPE_METHOD(tpl, "subVec", subVec);
   NODE_SET_PROTOTYPE_METHOD(tpl, "put", put);
   NODE_SET_PROTOTYPE_METHOD(tpl, "push", push);
   NODE_SET_PROTOTYPE_METHOD(tpl, "unshift", unshift);
   // (TODO) NODE_SET_PROTOTYPE_METHOD(tpl, "pushV", pushV);
   NODE_SET_PROTOTYPE_METHOD(tpl, "sum", sum);
   NODE_SET_PROTOTYPE_METHOD(tpl, "getMaxIdx", getMaxIdx);
   NODE_SET_PROTOTYPE_METHOD(tpl, "sort", sort);
   NODE_SET_PROTOTYPE_METHOD(tpl, "shuffle", shuffle);
   NODE_SET_PROTOTYPE_METHOD(tpl, "trunc", trunc);
   // (TODO) NODE_SET_PROTOTYPE_METHOD(tpl, "outer", outer);
   NODE_SET_PROTOTYPE_METHOD(tpl, "inner", inner);
   NODE_SET_PROTOTYPE_METHOD(tpl, "plus", plus);
   NODE_SET_PROTOTYPE_METHOD(tpl, "minus", minus);
   NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", multiply);
   NODE_SET_PROTOTYPE_METHOD(tpl, "normalize", normalize);
   NODE_SET_PROTOTYPE_METHOD(tpl, "norm", norm);
   
   // Properties 
   tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), length);
   
   // NODE_SET_PROTOTYPE_METHOD(constructor, "length", length);
   // This has to be last, otherwise the properties won't show up on the
   // object in JavaScript.
   constructor.Reset(Isolate, tpl->GetFunction());
   exports->Set(v8::String::NewFromUtf8(Isolate, "vector"),
      tpl->GetFunction());
}

// Creates a new instance of the object -- esentially does the job of Javascript new operator 
template <typename TVal, typename TAux>
v8::Handle<v8::Value> TNodeJsVec<TVal, TAux>::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   const unsigned Argc = 1;
   v8::Handle<v8::Value> Argv[Argc] = { Args[0] };
   v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
   v8::Local<v8::Object> Instance = Cons->NewInstance(Argc, Argv);
   
   return Args.GetReturnValue().Set(Instance);
}

template <typename TVal, typename TAux>
v8::Local<v8::Object> TNodeJsVec<TVal, TAux>::New(const TFltV& FltV) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::EscapableHandleScope HandleScope(Isolate);
   
   v8::Local<v8::Array> Arr = v8::Array::New(Isolate, FltV.Len());
   for (int ElN = 0; ElN < FltV.Len(); ++ElN) {
      Arr->Set(v8::Number::New(Isolate, ElN), v8::Number::New(Isolate, FltV[ElN]));
   }
   
   return HandleScope.Escape(New(Arr));
}

template <typename TVal, typename TAux>
v8::Local<v8::Object> TNodeJsVec<TVal, TAux>::New(v8::Local<v8::Array> Arr) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::EscapableHandleScope HandleScope(Isolate);
   
   const int Argc = 1;
   v8::Handle<v8::Value> Argv[Argc] = { Arr };
   v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
   return HandleScope.Escape(Cons->NewInstance(Argc, Argv));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   if (Args.IsConstructCall()) {
      TNodeJsVec<TVal, TAux>* JsVec = new TNodeJsVec<TVal, TAux>();
      
      v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	   v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
      v8::Local<v8::Object> Instance = Args.This();
      Instance->Set(Key, Value);
      
      JsVec->Wrap(Instance); // Args.This());
      // If we got Javascript array on the input: vector.new([1,2,3]) 
      if (Args[0]->IsArray()) {
         v8::Handle<v8::Array> Arr = v8::Handle<v8::Array>::Cast(Args[0]);
         const int Len = Arr->Length();
         for (int ElN = 0; ElN < Len; ++ElN) { JsVec->Vec.Add(Arr->Get(ElN)->NumberValue()); }
      } else if (Args[0]->IsObject()) {
         // Make sure we got TFltV on the input 
         if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
            TNodeJsVec<TVal, TAux>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());
            JsVec->Vec = JsVecArg->Vec;
         } else {
            // We have object with parameters, parse them out
				const int MxVals = TNodeJsUtil::GetArgInt32(Args, 0, "mxVals", -1);
				const int Vals = TNodeJsUtil::GetArgInt32(Args, 0, "vals", 0);
				if (MxVals >= 0) {
				   JsVec->Vec.Gen(MxVals, Vals);
				} else { JsVec->Vec.Gen(Vals); }
         }
      } // else return an empty vector 
      Args.GetReturnValue().Set(Instance); // Args.This());
   } else {
      const int Argc = 1;
      v8::Local<v8::Value> Argv[Argc] = { Args[0] };
      v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
      v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
      
      v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	   v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
      Instance->Set(Key, Value);
      
      Args.GetReturnValue().Set(Instance);
   }
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::newIntVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   const int Argc = 1;
   v8::Local<v8::Value> Argv[Argc] = { Args[0] };
   v8::Local<v8::Function> Cons = v8::Local<v8::Function>::New(Isolate, constructor);
   v8::Local<v8::Object> Instance = Cons->NewInstance(Argc, Argv);
   
   v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
	Instance->SetHiddenValue(Key, Value);
   
   Args.GetReturnValue().Set(Instance);
}

// Returns an element at index idx=Args[0]; assert 0 <= idx < v.length() 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   QmAssertR(Args.Length() >= 1 && Args[0]->IsInt32(), "Expected integer.");
   const int Idx = Args[0]->IntegerValue();
   
   QmAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds.");
   Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.GetVal(Idx).Val));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::subVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec>(Args.This());
   if (Args.Length() > 0) {
		if (Args[0]->IsArray()) {
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
			const int Len = Array->Length();
			v8::Handle<v8::Array> OutArr = v8::Array::New(Isolate, Len);
			for (int ElN = 0; ElN < Len; ++ElN) {
			   EAssertR(Array->Get(ElN)->IsInt32(),
			      "Expected array to contain integers only.");
			   const int Idx = Array->Get(ElN)->Int32Value();
			   EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(),
			      "One of the indices from the index vector is out of bounds");
				OutArr->Set(v8::Number::New(Isolate, ElN), v8::Number::New(Isolate, JsVec->Vec[Idx]));
			}
			
			Args.GetReturnValue().Set(New(OutArr));
		} else if (Args[0]->IsObject()) { // TJsVecUtil::IsArgClass(Args, 0, "TIntV")) {
			TNodeJsVec<TVal, TAux>* IdxV = ObjectWrap::Unwrap<TNodeJsVec>(Args[0]->ToObject());
			const int Len = IdxV->Vec.Len();
			v8::Handle<v8::Array> OutArr = v8::Array::New(Isolate, Len);
			for (int ElN = 0; ElN < Len; ElN++) {
			   EAssertR(IdxV->Vec[ElN] >= 0 && IdxV->Vec[ElN] < JsVec->Vec.Len(),
			      "One of the indices from the index vector is out of bounds");
			   OutArr->Set(v8::Number::New(Isolate, ElN), v8::Number::New(Isolate, IdxV->Vec[ElN]));
			}
			Args.GetReturnValue().Set(New(OutArr));
		} else {
		   Args.GetReturnValue().Set(v8::Undefined(Isolate));
		   QmFailR("Expected array or vector of indices.");
		}
	} else {
	   Args.GetReturnValue().Set(v8::Undefined(Isolate));
	   QmFailR("No arguments.");
	}
}

// Returns the sum of the vectors elements (only make sense for numeric values) 
template <class TVal, class TAux>
void TNodeJsVec<TVal, TAux>::sum(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   TFlt Sum = 0.0;
   for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
      Sum += JsVec->Vec.GetVal(ElN);
   }
   Args.GetReturnValue().Set(v8::Number::New(Isolate, Sum));
}

// put(idx, num) sets v[idx] := num 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   EAssertR(Args.Length() >= 2, "Expected two arguments.");
   EAssertR(Args[0]->IsInt32(),
      "First argument should be an integer.");
   EAssertR(Args[1]->IsNumber(),
      "Second argument should be a number.");
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   const int Idx = Args[0]->IntegerValue();
   
   EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds");
   
   JsVec->Vec[Idx] = Args[1]->NumberValue();
   
   Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

// Appends an element to the vector 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   if (Args.Length() < 1) {
      Isolate->ThrowException(v8::Exception::TypeError(
         v8::String::NewFromUtf8(Isolate, "Expected 1 argument, 0 given.")));
   } else if (!Args[0]->IsNumber()) {
      Isolate->ThrowException(v8::Exception::TypeError(
         v8::String::NewFromUtf8(Isolate, "Expected number")));
   } else {
      const double Val = Args[0]->ToNumber()->Value();
      JsVec->Vec.Add(Val);
      Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
   }
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::unshift(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   // assume number
   TFlt Val = Args[0]->ToNumber()->Value();
   JsVec->Vec.Ins(0, Val);
   Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.Len()));
}

// Returns i = arg max_i v[i] for a vector v 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::getMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   double MxVal = JsVec->Vec.GetVal(0);
   int MxIdx = 0;
   for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
      const double CrrVal = JsVec->Vec.GetVal(ElN);
      if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
   }
   
   Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   
   const bool Asc = Args.Length() > 0 && Args[0]->BooleanValue();
   
   TFltV ResV(JsVec->Vec);
   ResV.Sort(Asc);
   
   Args.GetReturnValue().Set(New(ResV));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   static TRnd Rnd;
   JsVec->Vec.Shuffle(Rnd);
   
   Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   if (!(Args.Length() >= 1 && Args[0]->IsInt32() && Args[0]->IntegerValue() >= 0)) {
      Isolate->ThrowException(v8::Exception::TypeError(
         v8::String::NewFromUtf8(Isolate, "Expected a nonnegative integer")));
   }
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   const int NewLen = Args[0]->IntegerValue();
   JsVec->Vec.Trunc(NewLen);
   
   Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

// XXX: Other function implementations will appear here -- we want to preserve declaration order 

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::inner(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux>>(Args.Holder());
   double Result = 0.0;
   if (Args[0]->IsObject()) {
      TNodeJsVec<TVal, TAux>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());
      Result = TLinAlg::DotProduct(OthVec->Vec, JsVec->Vec);
   }
   
   Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
      "Expected an TNodeJsVec object (a vector)");
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   TNodeJsVec<TVal, TAux>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());
   TFltV Result(JsVec->Vec.Len());
   TLinAlg::LinComb(1.0, JsVec->Vec, 1.0, OthVec->Vec, Result);
   
   Args.GetReturnValue().Set(New(Result));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1 && Args[0]->IsObject(),
      "Expected an TNodeJsVec object (a vector)");
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   TNodeJsVec<TVal, TAux>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());
   TFltV Result; Result.Gen(JsVec->Vec.Len());
   TLinAlg::LinComb(1.0, JsVec->Vec, -1.0, OthVec->Vec, Result);
   
   Args.GetReturnValue().Set(New(Result));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1 && Args[0]->IsNumber(),
      "Expected number");
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   const double Scalar = Args[0]->NumberValue();
   
   TFltV Result;
   Result.Gen(JsVec->Vec.Len());
   TLinAlg::MultiplyScalar(Scalar, JsVec->Vec, Result);
   
   Args.GetReturnValue().Set(New(Result));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::normalize(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
   if (JsVec->Vec.Len() > 0) {
      TLinAlg::Normalize(JsVec->Vec);
   } // TODO: Else, warn the user 
   
   Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::norm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.This());
   const double Result = TLinAlg::Norm(JsVec->Vec);
   Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

// Returns the size of the vector 
template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   v8::Local<v8::Object> Self = Info.Holder();
   TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Self);
   
   Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsVec->Vec.Len()));
}

///////////////////////////////
// NodeJs-Qminer-FltVV
v8::Persistent<v8::Function> TNodeJsFltVV::constructor;

void TNodeJsFltVV::Init(v8::Handle<v8::Object> exports) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();

   v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, New);
   tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "matrix"));
   // ObjectWrap uses the first internal field to store the wrapped pointer.
   tpl->InstanceTemplate()->SetInternalFieldCount(1);
   
   // Add all prototype methods, getters and setters here.
   NODE_SET_PROTOTYPE_METHOD(tpl, "at", at);
   NODE_SET_PROTOTYPE_METHOD(tpl, "put", put);
   NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", multiply);
   NODE_SET_PROTOTYPE_METHOD(tpl, "multiplyT", multiplyT);
   NODE_SET_PROTOTYPE_METHOD(tpl, "plus", plus);
   NODE_SET_PROTOTYPE_METHOD(tpl, "minus", minus);
   NODE_SET_PROTOTYPE_METHOD(tpl, "transpose", transpose);
   NODE_SET_PROTOTYPE_METHOD(tpl, "solve", solve);
   NODE_SET_PROTOTYPE_METHOD(tpl, "rowNorms", rowNorms);
   NODE_SET_PROTOTYPE_METHOD(tpl, "colNorms", colNorms);
   NODE_SET_PROTOTYPE_METHOD(tpl, "normalizeCols", normalizeCols);
   NODE_SET_PROTOTYPE_METHOD(tpl, "frob", frob);
   /*
   NODE_SET_PROTOTYPE_METHOD(tpl, "sparse", sparse);
   NODE_SET_PROTOTYPE_METHOD(tpl, "toString", toString);
   */
   NODE_SET_PROTOTYPE_METHOD(tpl, "rowMaxIdx", rowMaxIdx);
   NODE_SET_PROTOTYPE_METHOD(tpl, "colMaxIdx", colMaxIdx);
   /*
   NODE_SET_PROTOTYPE_METHOD(tpl, "getCol", getCol);
   NODE_SET_PROTOTYPE_METHOD(tpl, "setCol", setCol);
   NODE_SET_PROTOTYPE_METHOD(tpl, "getRow", getRow);
   NODE_SET_PROTOTYPE_METHOD(tpl, "setRow", setRow);
   NODE_SET_PROTOTYPE_METHOD(tpl, "diag", diag);
   NODE_SET_PROTOTYPE_METHOD(tpl, "save", save);
   */
   
   // Properties 
   tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "rows"), rows);
   tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "cols"), cols);
   
   // NODE_SET_PROTOTYPE_METHOD(constructor, "length", length);
   // This has to be last, otherwise the properties won't show up on the
   // object in JavaScript.
   constructor.Reset(Isolate, tpl->GetFunction());
   exports->Set(v8::String::NewFromUtf8(Isolate, "matrix"),
      tpl->GetFunction());
}

void TNodeJsFltVV::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TFltVV Mat;
   if (Args.IsConstructCall()) {
      if (Args.Length() > 0) {
         if (Args[0]->IsArray()) {
            printf("ok..\n");
            v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
            int Rows = Array->Length();
            if (Rows > 0) {
               // are the objects arrays
               int Cols = 0;
               for (int RowN = 0; RowN < Rows; RowN++) {
                  QmAssertR(Array->Get(RowN)->IsArray(),
                     "Object is not an array of arrays in TJsLinAlg::newMat()");
                  v8::Handle<v8::Array> Row = v8::Handle<v8::Array>::Cast(Array->Get(RowN));
                  if (RowN == 0) {
                     Cols = Row->Length();
                     Mat.Gen(Rows, Cols);
                  } else {
                     QmAssertR((int)Row->Length() == Cols,
                        "Inconsistent number of columns in TJsLinAlg::newMat()");
                  }
                  for (int ColN = 0; ColN < Cols; ColN++) {
                     Mat.PutXY(RowN, ColN, Row->Get(ColN)->NumberValue());
                  }
               }
            }
            TNodeJsFltVV* JsMat = new TNodeJsFltVV(Mat);
            JsMat->Wrap(Args.This());
            Args.GetReturnValue().Set(Args.This());
         } else {
            if (Args[0]->IsObject()) {
               const bool GenRandom = TNodeJsUtil::GetArgBool(Args, 0, "random", true);
               const int Cols = TNodeJsUtil::GetArgInt32(Args, 0, "cols", 3);
               const int Rows = TNodeJsUtil::GetArgInt32(Args, 0, "rows", 3);
               if (Cols > 0 && Rows > 0) {
                  Mat.Gen(Rows, Cols);
                  if (GenRandom) {
                     TLAMisc::FillRnd(Mat);
                  }
               }
               TNodeJsFltVV* JsMat = new TNodeJsFltVV(Mat);
               JsMat->Wrap(Args.This());
               Args.GetReturnValue().Set(Args.This());
            } else {
               QmFailR("Expected either array or object");
            }
         }
     } else { // Returns an empty matrix 
         TNodeJsFltVV* JsMat = new TNodeJsFltVV();
         JsMat->Wrap(Args.This());
         Args.GetReturnValue().Set(Args.This());
      }
   } else {
      const int Argc = 1;
      v8::Local<v8::Value> Argv[Argc] = { Args[0] };
      v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
      cons->NewInstance(Argc, Argv);
      v8::Local<v8::Object> Instance = cons->NewInstance(Argc, Argv);
      Args.GetReturnValue().Set(Instance);
   }
}

void TNodeJsFltVV::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 2 && Args[0]->IsInt32() && Args[1]->IsInt32(),
      "Expected two nonnegative integers as indices");
   
   const int RowIdx = Args[0]->IntegerValue();
   const int ColIdx = Args[1]->IntegerValue();
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   
   QmAssertR(0 <= RowIdx && RowIdx < JsMat->Mat.GetRows(), "Row index out of bounds");
   QmAssertR(0 <= ColIdx && ColIdx < JsMat->Mat.GetCols(), "Column index out of bounds");
   
   const double Res = JsMat->Mat.At(RowIdx, ColIdx);
   
   Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsFltVV::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 3 && Args[0]->IsInt32() && Args[1]->IsInt32() &&
      Args[2]->IsNumber(), "Expected two nonnegative integers as indices");
   
   const int RowIdx = Args[0]->IntegerValue();
   const int ColIdx = Args[1]->IntegerValue();
   const double Val = Args[2]->NumberValue();
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   
   QmAssertR(0 <= RowIdx && RowIdx < JsMat->Mat.GetRows(), "Row index out of bounds");
   QmAssertR(0 <= ColIdx && ColIdx < JsMat->Mat.GetCols(), "Column index out of bounds");
   
   JsMat->Mat.At(RowIdx, ColIdx) = Val;
   
   Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFltVV::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1, "Expected one argument");
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   if (Args[0]->IsNumber()) {
      const double Scalar = Args[0]->NumberValue();
      TFltVV ResMat;
      ResMat.Gen(JsMat->Mat.GetRows(), JsMat->Mat.GetCols());
      TLinAlg::MultiplyScalar(Scalar, JsMat->Mat, ResMat);
      TNodeJsFltVV* JsResMat = new TNodeJsFltVV(ResMat);
      JsResMat->Wrap(Args.This());
      Args.GetReturnValue().Set(JsResMat);
   } else if (Args[0]->IsObject()) { // IF vector, then u = A *v 
      if (true) { // TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) { // TODO: Check this out 
         TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
         QmAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "Matrix-vector multiplication: Dimension mismatch");
         TFltV Result(JsMat->Mat.GetRows());
         
			TLinAlg::Multiply(JsMat->Mat, JsVec->Vec, Result);
			printf("xxxx %d\n", Result.Len());
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
		} else if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) { // IF matrix, then C = A * B 
         TNodeJsFltVV* FltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
         TFltVV Result;
			// computation
			Result.Gen(JsMat->Mat.GetRows(), FltVV->Mat.GetCols());
			TLinAlg::Multiply(JsMat->Mat, FltVV->Mat, Result);
         Args.GetReturnValue().Set(v8::Undefined(Isolate));
      }
   } else { // TODO: What if object, etc.? 
      Args.GetReturnValue().Set(v8::Undefined(Isolate));
   }
}

void TNodeJsFltVV::multiplyT(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1, "Expected one argument");
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   if (Args[0]->IsNumber()) {
      const double Scalar = Args[0]->NumberValue();
      TFltVV ResMat (JsMat->Mat);
      ResMat.Transpose();
      TLinAlg::MultiplyScalar(Scalar, ResMat, ResMat);
      TNodeJsFltVV* JsResMat = new TNodeJsFltVV(ResMat);
      JsResMat->Wrap(Args.Holder());
      Args.GetReturnValue().Set(JsResMat);
   } else { // TODO: What if object, etc.? 
      Args.GetReturnValue().Set(v8::Undefined(Isolate));
   }
}

void TNodeJsFltVV::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   
   QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected a matrix");
   
   TNodeJsFltVV* JsOthMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
   TFltVV Result;
   Result.Gen(JsMat->Mat.GetRows(), JsOthMat->Mat.GetCols());
   TLinAlg::LinComb(1.0, JsMat->Mat, 1.0, JsOthMat->Mat, Result);
   TNodeJsFltVV* JsResMat = new TNodeJsFltVV(Result);
   JsResMat->Wrap(Args.Holder());
   Args.GetReturnValue().Set(JsResMat);
}

void TNodeJsFltVV::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   TNodeJsFltVV* JsOthMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
   TFltVV Result;
   Result.Gen(JsMat->Mat.GetRows(), JsOthMat->Mat.GetCols());
   TLinAlg::LinComb(1.0, JsMat->Mat, -1.0, JsOthMat->Mat, Result);
   TNodeJsFltVV* JsResMat = new TNodeJsFltVV(Result);
   JsResMat->Wrap(Args.Holder());
   
   Args.GetReturnValue().Set(JsResMat);   
}

void TNodeJsFltVV::transpose(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TFltVV ResMat;
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   ResMat.Gen(JsMat->Mat.GetCols(), JsMat->Mat.GetRows());
   TLinAlg::Transpose(JsMat->Mat, ResMat);
   TNodeJsFltVV* JsResMat = new TNodeJsFltVV(ResMat);
   JsResMat->Wrap(Args.Holder());
   
   Args.GetReturnValue().Set(JsResMat);
}

void TNodeJsFltVV::solve(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   
   QmAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected vector on the input");
   
   TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
   
   QmAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "Matrix \\ vector: dimensions mismatch");
   
   TFltV Result;
   Result.Gen(JsMat->Mat.GetCols());
   
   TNumericalStuff::SolveLinearSystem(JsMat->Mat, JsVec->Vec, Result);
   
   TNodeJsVec<TFlt, TAuxFltV>* JsResVec = new TNodeJsVec<TFlt, TAuxFltV>(Result);
   JsResVec->Wrap(Args.Holder());
   Args.GetReturnValue().Set(JsResVec);
}

void TNodeJsFltVV::rowNorms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   TFltV Result;   
   const int Rows = JsMat->Mat.GetRows();
   const int Cols = JsMat->Mat.GetCols();
   Result.Gen(Rows);
   Result.PutAll(0.0);
   for (int RowN = 0; RowN < Rows; RowN++) {
      for (int ColN = 0; ColN < Cols; ColN++) {
         Result[RowN] += TMath::Sqr(JsMat->Mat.At(RowN, ColN));
      }
      Result[RowN] = TMath::Sqrt(Result[RowN]);
   }
   
   TNodeJsVec<TFlt, TAuxFltV>* JsResVec = new TNodeJsVec<TFlt, TAuxFltV>(Result);
   JsResVec->Wrap(Args.Holder());
   Args.GetReturnValue().Set(JsResVec);
}

void TNodeJsFltVV::colNorms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   TFltV Result;
   const int Cols = JsMat->Mat.GetCols();
   Result.Gen(Cols);
   Result.PutAll(0.0);
   for (int ColN = 0; ColN < Cols; ColN++) {
      Result[ColN] = TLinAlg::Norm(JsMat->Mat, ColN);
   }
   // TODO: Find a way for a matrix class to return a new vector 
   // TNodeJsVec* JsResVec = new TNodeJsVec(Result);
   // JsResVec->Wrap(Args.Holder());
   Args.GetReturnValue().Set(v8::Undefined(Isolate)); // JsResVec);
}

void TNodeJsFltVV::normalizeCols(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   TLinAlg::NormalizeColumns(JsMat->Mat);
   Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsFltVV::frob(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   
   double FrobNorm = 0.0;
   const int Cols = JsMat->Mat.GetCols();
   for (int ColN = 0; ColN < Cols; ColN++) {
      FrobNorm += TLinAlg::Norm2(JsMat->Mat, ColN);
   }
   
   Args.GetReturnValue().Set(v8::Number::New(Isolate, TMath::Sqrt(FrobNorm)));
}

void TNodeJsFltVV::rowMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1 && Args[0]->Int32Value(),
      "Expected nonnegative integer");
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   const int RowN = Args[0]->Int32Value();
   
   QmAssertR(0 <= RowN && RowN < JsMat->Mat.GetRows(),
      "Index out of bounds.");
   
   const int MxIdx = TLinAlg::GetRowMaxIdx(JsMat->Mat, RowN);
   
   Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

void TNodeJsFltVV::colMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   QmAssertR(Args.Length() == 1 && Args[0]->Int32Value(),
      "Expected nonnegative integer");
   
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
   const int ColN = Args[0]->Int32Value();
   
   QmAssertR(0 <= ColN && ColN < JsMat->Mat.GetRows(),
      "Index out of bounds.");
   
   const int MxIdx = TLinAlg::GetColMaxIdx(JsMat->Mat, ColN);
   
   Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

void TNodeJsFltVV::cols(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   v8::Local<v8::Object> Self = Info.Holder();
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Self);
   
   Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetCols()));
}

void TNodeJsFltVV::rows(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
   v8::Isolate* Isolate = v8::Isolate::GetCurrent();
   v8::HandleScope HandleScope(Isolate);
   
   v8::Local<v8::Object> Self = Info.Holder();
   TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Self);
   
   Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsMat->Mat.GetRows()));
}


///////////////////////////////
// NodeJs-QMiner-Sparse-Vector
// TODO 

///////////////////////////////
// NodeJs-QMiner-Sparse-Col-Matrix
// TODO 

///////////////////////////////
// Register functions, etc.  
void init(v8::Handle<v8::Object> exports) {
   TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
   TNodeJsFltVV::Init(exports);
}

NODE_MODULE(la, init)


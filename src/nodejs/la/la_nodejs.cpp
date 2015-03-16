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
    NODE_SET_METHOD(exports, "qr", _qr);
}

void TNodeJsLinAlg::svd(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Handle<v8::Object> JsObj = v8::Object::New(Isolate); // Result 
    TFltVV U;
    TFltVV V;
    TFltV s;
    if (Args.Length() > 1) {
        int Iters = -1;
        double Tol = 1e-6;
        if (Args.Length() > 2) {
            PJsonVal ParamVal = TNodeJsUtil::GetArgJson(Args, 2);
            Iters = ParamVal->GetObjInt("iter", -1);
            Tol = ParamVal->GetObjNum("tol", 1e-6);
        }
        if (Args[0]->IsObject() && Args[1]->IsInt32()) {
            int k = Args[1]->Int32Value();
            if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
                TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
                TFullMatrix Mat(JsMat->Mat);
                TLinAlg::ComputeThinSVD(Mat, k, U, s, V, Iters, Tol);
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
                TNodeJsSpMat* JsMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
                if (JsMat->Rows != -1) {
                    TSparseColMatrix Mat(JsMat->Mat, JsMat->Rows, JsMat->Mat.Len());
                    TLinAlg::ComputeThinSVD(Mat, k, U, s, V, Iters, Tol);
                } else {
                    TSparseColMatrix Mat(JsMat->Mat);
                    TLinAlg::ComputeThinSVD(Mat, k, U, s, V, Iters, Tol);
                }
            } else {
                EFailR("SVD expects TFltVV or TVec<TIntFltKdV>");
            }
            JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "U")), TNodeJsFltVV::New(U));
            JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "V")), TNodeJsFltVV::New(V));
            JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "s")), TNodeJsFltV::New(s));
            Args.GetReturnValue().Set(JsObj);
        } else {
            Args.GetReturnValue().Set(v8::Undefined(Isolate));
        }
    } else {
        Args.GetReturnValue().Set(v8::Undefined(Isolate));
    }
}

void TNodeJsLinAlg::qr(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
    v8::Handle<v8::Object> JsObj = v8::Object::New(Isolate); // Result 
    TFltVV Q;
    TFltVV R;
    double Tol = TNodeJsUtil::GetArgFlt(Args, 1, 1e-6);
    if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
        TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
        TLinAlg::QR(JsMat->Mat, Q, R, Tol);
    }
    JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "Q")), TNodeJsFltVV::New(Q));
    JsObj->Set(v8::Handle<v8::String>(v8::String::NewFromUtf8(Isolate, "R")), TNodeJsFltVV::New(R));
    Args.GetReturnValue().Set(JsObj);
}

///////////////////////////////
// NodeJs-Qminer-Vector
const TStr TAuxFltV::ClassId = "TFltV";
const TStr TAuxIntV::ClassId = "TIntV";
const TStr TAuxStrV::ClassId = "TStrV";
const TStr TAuxBoolV::ClassId = "TBoolV";


///////////////////////////////
// NodeJs-Qminer-FltVV
v8::Persistent<v8::Function> TNodeJsFltVV::Constructor;

const TStr TNodeJsFltVV::ClassId = "TFltVV";

void TNodeJsFltVV::Init(v8::Handle<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::FunctionTemplate> Tpl = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewJs<TNodeJsFltVV>);
    // child will have the same properties and methods, but a different callback: _NewCpp
	v8::Local<v8::FunctionTemplate> Child = v8::FunctionTemplate::New(Isolate, TNodeJsUtil::_NewCpp<TNodeJsFltVV>);
	Child->Inherit(Tpl);

	Child->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	Child->InstanceTemplate()->SetInternalFieldCount(1);

	Tpl->SetClassName(v8::String::NewFromUtf8(Isolate, ClassId.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer
	Tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(Tpl, "at", _at);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "put", _put);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "multiply", _multiply);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "multiplyT", _multiplyT);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "plus", _plus);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "minus", _minus);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "transpose", _transpose);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "solve", _solve);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "rowNorms", _rowNorms);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "colNorms", _colNorms);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "normalizeCols", _normalizeCols);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "frob", _frob);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "sparse", _sparse);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "toString", _toString);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "rowMaxIdx", _rowMaxIdx);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "colMaxIdx", _colMaxIdx);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "getCol", _getCol);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "setCol", _setCol);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "getRow", _getRow);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "setRow", _setRow);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "diag", _diag);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "save", _save);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "load", _load);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "saveascii", _saveascii);
    NODE_SET_PROTOTYPE_METHOD(Tpl, "loadascii", _loadascii);

    // Properties 
    Tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "rows"), _rows);
    Tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "cols"), _cols);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    Constructor.Reset(Isolate, Child->GetFunction());
    exports->Set(v8::String::NewFromUtf8(Isolate, "Matrix"), Tpl->GetFunction());
}

TNodeJsFltVV* TNodeJsFltVV::NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TFltVV Mat;
	if (Args.Length() > 0) {
		if (Args[0]->IsArray()) {
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
			int Rows = Array->Length();
			if (Rows > 0) {
				// are the objects arrays
				int Cols = 0;
				for (int RowN = 0; RowN < Rows; RowN++) {
					EAssertR(Array->Get(RowN)->IsArray(),
						"Object is not an array of arrays in TJsLinAlg::newMat()");
					v8::Handle<v8::Array> Row = v8::Handle<v8::Array>::Cast(Array->Get(RowN));
					if (RowN == 0) {
						Cols = Row->Length();
						Mat.Gen(Rows, Cols);
					} else {
						EAssertR((int)Row->Length() == Cols,
							"Inconsistent number of columns in TJsLinAlg::newMat()");
					}
					for (int ColN = 0; ColN < Cols; ColN++) {
						Mat.PutXY(RowN, ColN, Row->Get(ColN)->NumberValue());
					}
				}
			}
			return new TNodeJsFltVV(Mat);
		} else {
			if (Args[0]->IsObject()) {
				if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)) {
					TNodeJsFltVV* FltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
					Mat = FltVV->Mat;
				} else {

					const bool GenRandom = TNodeJsUtil::GetArgBool(Args, 0, "random", false);
					const int Cols = TNodeJsUtil::GetArgInt32(Args, 0, "cols");
					const int Rows = TNodeJsUtil::GetArgInt32(Args, 0, "rows");
					EAssert(Cols > 0 && Rows > 0);
					Mat.Gen(Rows, Cols);
					if (GenRandom) {
						TLAMisc::FillRnd(Mat);
					}
				}
				return new TNodeJsFltVV(Mat);
			} else {
				throw TExcept::New("Expected either array or object");
			}
		}
	} else { // Returns an empty matrix
		return new TNodeJsFltVV();
	}
}

v8::Local<v8::Object> TNodeJsFltVV::New(const TFltVV& FltVV) {
    return TNodeJsUtil::NewInstance(new TNodeJsFltVV(FltVV));
}

v8::Local<v8::Object> TNodeJsFltVV::New(const TFltV& FltV) {
    TFltVV FltVV;	TLAMisc::Diag(FltV, FltVV);
    return TNodeJsUtil::NewInstance(new TNodeJsFltVV(FltVV));
}

void TNodeJsFltVV::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsInt32() && Args[1]->IsInt32(),
        "Expected two nonnegative integers as indices");

    const int RowIdx = Args[0]->IntegerValue();
    const int ColIdx = Args[1]->IntegerValue();

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    EAssertR(0 <= RowIdx && RowIdx < JsMat->Mat.GetRows(), "Row index out of bounds");
    EAssertR(0 <= ColIdx && ColIdx < JsMat->Mat.GetCols(), "Column index out of bounds");

    const double Res = JsMat->Mat.At(RowIdx, ColIdx);

    Args.GetReturnValue().Set(v8::Number::New(Isolate, Res));
}

void TNodeJsFltVV::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 3 && Args[0]->IsInt32() && Args[1]->IsInt32() &&
        Args[2]->IsNumber(), "Expected two nonnegative integers as indices");

    const int RowIdx = Args[0]->IntegerValue();
    const int ColIdx = Args[1]->IntegerValue();
    const double Val = Args[2]->NumberValue();

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    EAssertR(0 <= RowIdx && RowIdx < JsMat->Mat.GetRows(), "Row index out of bounds");
    EAssertR(0 <= ColIdx && ColIdx < JsMat->Mat.GetCols(), "Column index out of bounds");

    JsMat->Mat.At(RowIdx, ColIdx) = Val;

    Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsFltVV::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Expected one argument");
	TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
	if (Args[0]->IsNumber()) {
		const double Scalar = Args[0]->NumberValue();
		TFltVV ResMat;
		ResMat.Gen(JsMat->Mat.GetRows(), JsMat->Mat.GetCols());
		TLinAlg::MultiplyScalar(Scalar, JsMat->Mat, ResMat);
		Args.GetReturnValue().Set(New(ResMat));
	}
	else if (Args[0]->IsObject()) { // IF vector, then u = A *v 
		if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
			TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "Matrix-vector multiplication: Dimension mismatch");
			TFltV Result(JsMat->Mat.GetRows());

			TLinAlg::Multiply(JsMat->Mat, JsVec->Vec, Result);
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltVV::ClassId)) { // IF matrix, then C = A * B 
			TNodeJsFltVV* FltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
			TFltVV Result;
			// computation
			Result.Gen(JsMat->Mat.GetRows(), FltVV->Mat.GetCols());
			TLinAlg::Multiply(JsMat->Mat, FltVV->Mat, Result);
			Args.GetReturnValue().Set(New(Result));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
			TNodeJsSpVec* JsVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetCols() > TLAMisc::GetMaxDimIdx(JsVec->Vec), "matrix * sparse_vector: dimensions mismatch");
			int Rows = JsMat->Mat.GetRows();
			TFltVV Result(Rows, 1);
			// Copy could be omitted if we implemented SparseColMat * SparseVec
			TVec<TIntFltKdV> TempSpMat(1);
			TempSpMat[0] = JsVec->Vec;
			TLinAlg::Multiply(JsMat->Mat, TempSpMat, Result);
			// create JS result with the Result vector
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
			TNodeJsSpMat* JsMat2 = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetCols() >= JsMat2->Rows, "matrix * sparse_col_matrix: dimensions mismatch");
			// computation				
			int Rows = JsMat->Mat.GetRows();
			int Cols = JsMat->Mat.GetCols();
			if (JsMat2->Rows == -1) {
				EAssertR(Cols > TLAMisc::GetMaxDimIdx(JsMat2->Mat), "matrix * sparse_col_matrix: dimensions mismatch");
			}
			TFltVV Result(Rows, JsMat2->Mat.Len());
			TLinAlg::Multiply(JsMat->Mat, JsMat2->Mat, Result);
			// create JS result with the Result vector	
			Args.GetReturnValue().Set(New(Result));
		}
	}
	else {
		throw TExcept::New("Unsupported type");
	}
}

void TNodeJsFltVV::multiplyT(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1, "Expected one argument");
	TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
	if (Args[0]->IsNumber()) {
		const double Scalar = Args[0]->NumberValue();
		TFltVV ResMat(JsMat->Mat);
		ResMat.Transpose();
		TLinAlg::MultiplyScalar(Scalar, ResMat, ResMat);
		Args.GetReturnValue().Set(New(ResMat));
		return;
	}
	else if (Args[0]->IsObject()) {
		if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
			TNodeJsFltV* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetRows() == JsVec->Vec.Len(), "matrix' * vector: dimensions mismatch");
			// computation				
			TFltV Result(JsMat->Mat.GetCols());
			TLinAlg::MultiplyT(JsMat->Mat, JsVec->Vec, Result);
			// create JS result with the Result vector	
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {
			TNodeJsFltVV* JsMat2 = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetRows() == JsMat2->Mat.GetRows(), "matrix' * matrix: dimensions mismatch");
			TFltVV Result;
			// computation
			Result.Gen(JsMat->Mat.GetCols(), JsMat2->Mat.GetCols());
			TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
			Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
			TNodeJsSpVec* JsVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetRows() > TLAMisc::GetMaxDimIdx(JsVec->Vec), "matrix' * sparse_vector: dimensions mismatch");
			TFltVV Result(JsMat->Mat.GetCols(), 1);
			// Copy could be omitted if we implemented SparseColMat * SparseVec
			TVec<TIntFltKdV> TempSpMat(1);
			TempSpMat[0] = JsVec->Vec;
			TLinAlg::MultiplyT(JsMat->Mat, TempSpMat, Result);
			// create JS result with the Result vector	
			Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
		}
		else if (TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
			TNodeJsSpMat* JsMat2 = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
			EAssertR(JsMat->Mat.GetRows() >= JsMat2->Rows, "matrix' * sparse_col_matrix: dimensions mismatch");
			// computation				
			int Rows = JsMat->Mat.GetRows();
			int Cols = JsMat->Mat.GetCols();
			if (JsMat2->Rows == -1) {
				EAssertR(Rows > TLAMisc::GetMaxDimIdx(JsMat2->Mat), "matrix' * sparse_col_matrix: dimensions mismatch");
			}
			TFltVV Result(Cols, JsMat2->Mat.Len());
			TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
			// create JS result with the Result vector	
			Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
		}
		else {
			throw TExcept::New("Unsupported type");
		}
	}
	else {
		throw TExcept::New("Unsupported type");
	}
}


void TNodeJsFltVV::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected a matrix");

    TNodeJsFltVV* JsOthMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
    TFltVV Result;
    Result.Gen(JsMat->Mat.GetRows(), JsOthMat->Mat.GetCols());
    TLinAlg::LinComb(1.0, JsMat->Mat, 1.0, JsOthMat->Mat, Result);

    Args.GetReturnValue().Set(New(Result));
}

void TNodeJsFltVV::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    TNodeJsFltVV* JsOthMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
    TFltVV Result;
    Result.Gen(JsMat->Mat.GetRows(), JsOthMat->Mat.GetCols());
    TLinAlg::LinComb(1.0, JsMat->Mat, -1.0, JsOthMat->Mat, Result);

    Args.GetReturnValue().Set(New(Result));
}

void TNodeJsFltVV::transpose(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TFltVV ResMat;
    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    ResMat.Gen(JsMat->Mat.GetCols(), JsMat->Mat.GetRows());
    TLinAlg::Transpose(JsMat->Mat, ResMat);

    Args.GetReturnValue().Set(New(ResMat));
}

void TNodeJsFltVV::solve(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected vector on the input");

    TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());

    EAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(), "Matrix \\ vector: dimensions mismatch");

    TFltV Result;
    Result.Gen(JsMat->Mat.GetCols());
	TFltVV MatCopy(JsMat->Mat);
	TNumericalStuff::SolveLinearSystem(MatCopy, JsVec->Vec, Result);
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
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
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
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
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
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

void TNodeJsFltVV::sparse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    TVec<TIntFltKdV> SpMat = TVec<TIntFltKdV>();
    TLinAlg::Sparse(JsMat->Mat, SpMat);

    Args.GetReturnValue().Set(TNodeJsSpMat::New(SpMat, JsMat->Mat.GetRows()));
}

void TNodeJsFltVV::toString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    TStr Out = "";
    TLAMisc::PrintTFltVVToStr(JsMat->Mat, Out);

    Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, Out.CStr()));
}

void TNodeJsFltVV::rowMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsInt32(),
        "Expected integer");

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    const int RowN = Args[0]->Int32Value();

    EAssertR(0 <= RowN && RowN < JsMat->Mat.GetRows(),
        "Index out of bounds.");

    const int MxIdx = TLinAlg::GetRowMaxIdx(JsMat->Mat, RowN);

    Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

void TNodeJsFltVV::colMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsInt32() &&
        Args[0]->Int32Value() >= 0, "Expected nonnegative integer");

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    const int ColN = Args[0]->Int32Value();

    EAssertR(0 <= ColN && ColN < JsMat->Mat.GetRows(),
        "Index out of bounds.");

    const int MxIdx = TLinAlg::GetColMaxIdx(JsMat->Mat, ColN);

    Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

void TNodeJsFltVV::getCol(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsInt32() &&
        Args[0]->Int32Value() >= 0, "Expected nonnegative integer");

    const int ColIdx = Args[0]->Int32Value();
    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    TFltV Result;
    JsMat->Mat.GetCol(ColIdx, Result);
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
}

void TNodeJsFltVV::setCol(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[1]->IsObject() && Args[0]->IsInt32(),
        "Expected vector on the input");

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    TNodeJsVec<TFlt, TAuxFltV>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[1]->ToObject());

    const int ColN = Args[0]->Int32Value();

    EAssertR(JsMat->Mat.GetRows() == JsVec->Vec.Len(),
        "Number of rows of the matrix should equals the size of the vector");

    for (int RowN = 0; RowN < JsMat->Mat.GetRows(); ++RowN) {
        JsMat->Mat.At(RowN, ColN) = JsVec->Vec[RowN];
    }

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsFltVV::getRow(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsInt32() &&
        Args[0]->Int32Value() >= 0, "Expected nonnegative integer");

    const int RowIdx = Args[0]->Int32Value();
    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    TFltV Result;
    JsMat->Mat.GetRow(RowIdx, Result);
    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
}

void TNodeJsFltVV::setRow(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[1]->IsObject() && Args[0]->IsInt32(),
        "Expected vector on the input");

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    TNodeJsVec<TFlt, TAuxFltV>* JsVec =
        ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[1]->ToObject());

    const int RowN = Args[0]->Int32Value();

    EAssertR(JsMat->Mat.GetCols() == JsVec->Vec.Len(),
        "Number of rows of the matrix should equals the size of the vector");

    for (int ColN = 0; ColN < JsMat->Mat.GetCols(); ++ColN) {
        JsMat->Mat.At(RowN, ColN) = JsVec->Vec[ColN];
    }

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsFltVV::diag(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsMat = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());

    EAssertR(JsMat->Mat.GetCols() == JsMat->Mat.GetRows(),
        "Expected a square matrix.");

    TFltV DiagV; DiagV.Gen(JsMat->Mat.GetRows());
    for (int ElN = 0; ElN < JsMat->Mat.GetRows(); ++ElN) {
        DiagV[ElN] = JsMat->Mat.At(ElN, ElN);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(DiagV));
}

void TNodeJsFltVV::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFOut object");
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    // Save to stream
    JsFltVV->Mat.Save(*SOut);

    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsFltVV::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFIn object");
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    // Load from stream 
    JsFltVV->Mat.Load(*SIn);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsFltVV::saveascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFOut object");
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    TLAMisc::SaveMatlabTFltVV(JsFltVV->Mat, *SOut);

    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsFltVV::loadascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsFltVV* JsFltVV = ObjectWrap::Unwrap<TNodeJsFltVV>(Args.Holder());
    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected a TNodeJsFIn object");
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    TLAMisc::LoadMatlabTFltVV(JsFltVV->Mat, *SIn);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
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
// Sparse-Vector
v8::Persistent<v8::Function> TNodeJsSpVec::constructor;

const TStr TNodeJsSpVec::ClassId = "TIntFltKdV";

void TNodeJsSpVec::Init(v8::Handle<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SparseVector"));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "at", _at);
    NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);
    NODE_SET_PROTOTYPE_METHOD(tpl, "sum", _sum);
    NODE_SET_PROTOTYPE_METHOD(tpl, "inner", _inner);
    NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", _multiply);
    NODE_SET_PROTOTYPE_METHOD(tpl, "normalize", _normalize);
    NODE_SET_PROTOTYPE_METHOD(tpl, "norm", _norm);
    NODE_SET_PROTOTYPE_METHOD(tpl, "full", _full);
    NODE_SET_PROTOTYPE_METHOD(tpl, "valVec", _valVec);
    NODE_SET_PROTOTYPE_METHOD(tpl, "idxVec", _idxVec);
    NODE_SET_PROTOTYPE_METHOD(tpl, "toString", _toString);

    // Properties 
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "dim"), _dim);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "nnz"), _nnz);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    constructor.Reset(Isolate, tpl->GetFunction());
    #ifndef MODULE_INCLUDE_LA
    exports->Set(v8::String::NewFromUtf8(Isolate, "SparseVector"),
        tpl->GetFunction());
    #endif
}

v8::Local<v8::Object> TNodeJsSpVec::New(const TIntFltKdV& IntFltKdV, const int& Dim) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsSpVec::New: constructor is empty. Did you call TNodeJsSpVec::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntFltKdV");
    Instance->SetHiddenValue(Key, Value);

    TNodeJsSpVec* JsSpVec = new TNodeJsSpVec(IntFltKdV, Dim);
    JsSpVec->Wrap(Instance);
	if (JsSpVec->Dim != -1) {
		EAssertR(TLAMisc::GetMaxDimIdx(JsSpVec->Vec) < JsSpVec->Dim, "TNodeJsSpVec::New inconsistent dim parameter (maximal index >= dim!)");
	}
    return HandleScope.Escape(Instance);
}

void TNodeJsSpVec::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsSpVec::New: constructor is empty. Did you call TNodeJsSpVec::Init(exports); in this module's init function?");
	EAssertR(Args.IsConstructCall(), "TNodeJsSpVec::New: called without new operator!");
	TNodeJsSpVec* JsSpVec = new TNodeJsSpVec();
	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntFltKdV");
	v8::Local<v8::Object> Instance = Args.This();
	Instance->SetHiddenValue(Key, Value);

	JsSpVec->Wrap(Instance);
	// If we got Javascript array on the input: vector.new([1,2,3]) 
	if (Args[0]->IsArray()) {
		v8::Handle<v8::Array> Arr = v8::Handle<v8::Array>::Cast(Args[0]);
		const int Len = Arr->Length();		
		for (int ElN = 0; ElN < Len; ++ElN) {
			EAssertR(Arr->Get(ElN)->IsArray(), "TNodeJsSpVec::New: array constructor, found an element which is not an array (should be an array with two elements)");
			v8::Handle<v8::Array> CrrArr = v8::Handle<v8::Array>::Cast(Arr->Get(ElN));
			EAssertR(CrrArr->Length() == 2 && CrrArr->Get(0)->IsInt32() &&
				CrrArr->Get(1)->IsNumber(), "Expected a key-value pair.");
			JsSpVec->Vec.Add(TIntFltKd(
				CrrArr->Get(0)->Int32Value(), CrrArr->Get(1)->NumberValue()));
		}
		JsSpVec->Vec.Sort();
		int Dim = TNodeJsUtil::GetArgInt32(Args, 1, -1);
		JsSpVec->Dim = Dim;
	}
	Args.GetReturnValue().Set(Instance);
	if (JsSpVec->Dim != -1) {
		EAssertR(TLAMisc::GetMaxDimIdx(JsSpVec->Vec) < JsSpVec->Dim, "TNodeJsSpVec::New inconsistent dim parameter (maximal index >= dim!)");
	}

}

void TNodeJsSpVec::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsInt32(),
        "Expected integer index");

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    const int Idx = Args[0]->Int32Value();
	if (JsSpVec->Dim != -1) {
		EAssertR(Idx >= 0 && Idx < JsSpVec->Dim, "Index out of bounds.");
	} else {
		EAssertR(Idx >= 0, "Index out of bounds.");
	}
    bool FoundP = false;
    for (int ElN = 0; ElN < JsSpVec->Vec.Len(); ++ElN) {
        if ((FoundP = JsSpVec->Vec[ElN].Key == Idx)) {
            Args.GetReturnValue().Set(
                v8::Number::New(Isolate, JsSpVec->Vec[ElN].Dat));
            break;
        }
    }
    if (!FoundP) {
        Args.GetReturnValue().Set(v8::Number::New(Isolate, 0.0));
    }
}

void TNodeJsSpVec::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsInt32() &&
        Args[0]->IsNumber(),
        "Expected integer index");

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    const int Idx = Args[0]->Int32Value();
    const double Val = Args[1]->NumberValue();

    bool FoundP = false;
    for (int ElN = 0; ElN < JsSpVec->Vec.Len(); ++ElN) {
        if ((FoundP = (JsSpVec->Vec[ElN].Key == Idx))) {
            JsSpVec->Vec[ElN].Dat = Val;
            break;
        }
    }
    if (!FoundP) {
        JsSpVec->Vec.Add(TIntFltKd(Idx, Val));
        JsSpVec->Vec.Sort();
        if (JsSpVec->Dim != -1 && Idx >= JsSpVec->Dim) {
            JsSpVec->Dim = Idx+1;
        }
    }
    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, FoundP));
}

void TNodeJsSpVec::sum(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    EAssertR(JsSpVec->Vec.Len() > 0, "Summing vector of length zero");

    const double Result =
        JsSpVec->Vec.Len() > 0 ? TLinAlg::SumVec(JsSpVec->Vec) : 0.0;

    Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsSpVec::inner(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
        "Expected dnese or sparse vector on the input.");

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    if (Args[0]->IsObject()) {
        double Result = 0.0;
        if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
			TNodeJsVec<TFlt, TAuxFltV>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
			int Dim = JsSpVec->Dim();
			if (Dim == -1) {
				EAssertR(TLAMisc::GetMaxDimIdx(JsSpVec->Vec) < OthVec->Vec.Len(), "TNodeJsSpVec::inner: dimension mismatch!");
			}
			else {
				EAssertR(Dim < OthVec->Vec.Len(), "TNodeJsSpVec::inner: dimension mismatch!");
			}
            Result = TLinAlg::DotProduct(OthVec->Vec, JsSpVec->Vec);
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
        } else if (TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
            // TODO check dimensions if at least one is known!
			TNodeJsSpVec* OthSpVec =
                ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
            Result = TLinAlg::DotProduct(JsSpVec->Vec, OthSpVec->Vec);
            Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
        } else {
            EFailR("Uknown type.");
        }
    } // else {
        // Args.GetReturnValue().Set(v8::Undefined::New(Isolate));
    // }
}

void TNodeJsSpVec::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsNumber(), "Expected a scalar.");

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    double Scalar = Args[0]->NumberValue();
    // get the internal glib vector
    TIntFltKdV Result;
    // computation
    Result.Gen(JsSpVec->Vec.Len());
    TLinAlg::MultiplyScalar(Scalar, JsSpVec->Vec, Result);

    Args.GetReturnValue().Set(New(Result));
}

void TNodeJsSpVec::normalize(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    if (JsSpVec->Vec.Len() > 0) {
        TLinAlg::Normalize(JsSpVec->Vec);
    } // XXX: Otherwise, throw an exception? 

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

void TNodeJsSpVec::norm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    const double Result = TLinAlg::Norm(JsSpVec->Vec);
    Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsSpVec::full(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

	int Dim = JsSpVec->Dim;
    
    if (Args.Length() > 0 && Args[0]->IsInt32()) { Dim = Args[0]->Int32Value(); }
    if (Dim == -1) { Dim = TLAMisc::GetMaxDimIdx(JsSpVec->Vec) + 1; }
    TFltV Res;
	TLAMisc::ToVec(JsSpVec->Vec, Res, Dim);

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsSpVec::valVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    const int NNZ = JsSpVec->Vec.Len();
    TFltV Res(NNZ, 0);
    // copy values to Res
    for (int ElN = 0; ElN < NNZ; ElN++) { Res.Add(JsSpVec->Vec[ElN].Dat); }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsSpVec::idxVec(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    int NNZ = JsSpVec->Vec.Len();
    TIntV Res(NNZ, 0);
    // copy values to Res
    for (int ElN = 0; ElN < NNZ; ElN++) {
        Res.Add(JsSpVec->Vec[ElN].Key);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Res));
}

void TNodeJsSpVec::toString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args.Holder());

    TStr Str = "[";
    for (auto It = JsSpVec->Vec.BegI(); It != JsSpVec->Vec.EndI(); ++It) {
        if (It != JsSpVec->Vec.BegI()) { Str += ", "; }
        Str += "(" + TInt::GetStr(It->Key) + "," + TFlt::GetStr(It->Dat) + ")";
    }
	Str += "]";

    Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, Str.CStr()));
}

void TNodeJsSpVec::nnz(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsSpVec* JsSpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsSpVec->Vec.Len()));
}

void TNodeJsSpVec::dim(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    v8::Local<v8::Object> Self = Info.Holder();
    TNodeJsSpVec* JsSpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Self);

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsSpVec->Dim));
}

///////////////////////////////
// NodeJs-QMiner-Sparse-Col-Matrix
v8::Persistent<v8::Function> TNodeJsSpMat::constructor;

const TStr TNodeJsSpMat::ClassId = "TVec<TIntFltKdV>";

void TNodeJsSpMat::Init(v8::Handle<v8::Object> exports) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();

    v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
    tpl->SetClassName(v8::String::NewFromUtf8(Isolate, "SparseMatrix"));
    // ObjectWrap uses the first internal field to store the wrapped pointer.
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Add all prototype methods, getters and setters here.
    NODE_SET_PROTOTYPE_METHOD(tpl, "at", _at);
    NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);

	NODE_SET_PROTOTYPE_METHOD(tpl, "getCol", _indexGet);
	NODE_SET_PROTOTYPE_METHOD(tpl, "setCol", _indexSet);
    
	NODE_SET_PROTOTYPE_METHOD(tpl, "push", _push);
    NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", _multiply);
    NODE_SET_PROTOTYPE_METHOD(tpl, "multiplyT", _multiplyT);
    NODE_SET_PROTOTYPE_METHOD(tpl, "plus", _plus);
    NODE_SET_PROTOTYPE_METHOD(tpl, "minus", _minus);
    NODE_SET_PROTOTYPE_METHOD(tpl, "transpose", _transpose);
    NODE_SET_PROTOTYPE_METHOD(tpl, "colNorms", _colNorms);
    NODE_SET_PROTOTYPE_METHOD(tpl, "normalizeCols", _normalizeCols);
    NODE_SET_PROTOTYPE_METHOD(tpl, "full", _full);
    NODE_SET_PROTOTYPE_METHOD(tpl, "frob", _frob);
    NODE_SET_PROTOTYPE_METHOD(tpl, "print", _print);
    NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
    NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);

    // Properties 
    tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexGet, _indexSet);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "rows"), _rows);
    tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "cols"), _cols);

    // This has to be last, otherwise the properties won't show up on the
    // object in JavaScript.
    constructor.Reset(Isolate, tpl->GetFunction());
    #ifndef MODULE_INCLUDE_LA
    exports->Set(v8::String::NewFromUtf8(Isolate, "SparseMatrix"),
        tpl->GetFunction());
    #endif
}

v8::Local<v8::Object> TNodeJsSpMat::New(const TVec<TIntFltKdV>& Mat, const int& Rows) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsSpMat::New: constructor is empty. Did you call TNodeJsSpMat::Init(exports); in this module's init function?");
    v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
    v8::Local<v8::Object> Instance = cons->NewInstance();

    v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
    v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TVec<TIntFltKdV>");
    Instance->SetHiddenValue(Key, Value);

    TNodeJsSpMat* JsSpMat = new TNodeJsSpMat(Mat, Rows);
    JsSpMat->Wrap(Instance);

	if (JsSpMat->Rows != -1) {
		EAssertR(TLAMisc::GetMaxDimIdx(JsSpMat->Mat) < JsSpMat->Rows, "TNodeJsSpMat::New inconsistent rows parameter (maximal row index >= rows!)");
	}
    return HandleScope.Escape(Instance);
}


void TNodeJsSpMat::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsSpMat::New: constructor is empty. Did you call TNodeJsSpMat::Init(exports); in this module's init function?");
	EAssertR(Args.IsConstructCall(), "TNodeJsSpMat::New: construct using new operator!");
	TNodeJsSpMat* JsSpMat = new TNodeJsSpMat();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TVec<TIntFltKdV>");
	v8::Local<v8::Object> Instance = Args.This();
	Instance->SetHiddenValue(Key, Value);

	JsSpMat->Wrap(Instance);

	if (Args.Length() > 0) {
		if (Args.Length() >= 3 && Args.Length() <= 5 &&
			TNodeJsUtil::IsArgClass(Args, 0, "TIntV") &&
			TNodeJsUtil::IsArgClass(Args, 1, "TIntV") &&
			TNodeJsUtil::IsArgClass(Args, 2, "TFltV")) {

			TNodeJsVec<TInt, TAuxIntV>* RowIdxV =
				ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV>>(Args[0]->ToObject());
			TNodeJsVec<TInt, TAuxIntV>* ColIdxV =
				ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV>>(Args[1]->ToObject());
			TNodeJsVec<TFlt, TAuxFltV>* ValV =
				ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args[2]->ToObject());

			int Rows = Args.Length() >= 4 && Args[3]->IsInt32() ?
				Args[3]->Int32Value() : -1;
			JsSpMat->Rows = Rows;

			int Cols = Args.Length() == 5 && Args[4]->IsInt32() ?
				Args[4]->Int32Value() : -1;
			if (Cols < 0) { Cols = ColIdxV->Vec.GetMxVal() + 1; }

			TSparseOps<TInt, TFlt>::CoordinateCreateSparseColMatrix(
				RowIdxV->Vec, ColIdxV->Vec, ValV->Vec, JsSpMat->Mat, Cols);
		}
		else if (Args[0]->IsArray()) {
			v8::Handle<v8::Array> Array = v8::Handle<v8::Array>::Cast(Args[0]);
			int Cols = Array->Length();
			JsSpMat->Mat.Gen(Cols);
			for (int ColN = 0; ColN < Cols; ColN++) {
				if (Array->Get(ColN)->IsArray()) {
					v8::Handle<v8::Array> SpVecArray = v8::Handle<v8::Array>::Cast(Array->Get(ColN));
					int Els = SpVecArray->Length();
					for (int ElN = 0; ElN < Els; ElN++) {
						if (SpVecArray->Get(ElN)->IsArray()) {
							v8::Handle<v8::Array> KdPair = v8::Handle<v8::Array>::Cast(SpVecArray->Get(ElN));
							if (KdPair->Length() >= 2) {
								if (KdPair->Get(0)->IsInt32() && KdPair->Get(1)->IsNumber()) {
									JsSpMat->Mat[ColN].Add(TIntFltKd(KdPair->Get(0)->Int32Value(), KdPair->Get(1)->NumberValue()));
								}
							}
						}
					}
				}
				JsSpMat->Mat[ColN].Sort(); // XXX: How intense is this, computationaly? Jan: not a problem (this is meant to be used with smaller matrices - large arrays may break V8 anyway)
			}
			int Rows = -1;
			if (Args.Length() > 1 && Args[1]->IsInt32()) { Rows = Args[1]->Int32Value(); }
			JsSpMat->Rows = Rows;
		}
		else {
			if (Args[0]->IsObject()) {
				if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsSpMat::ClassId)) {
					TNodeJsSpMat* JsSpMatArg = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
					JsSpMat->Mat = JsSpMatArg->Mat;
					JsSpMat->Rows = JsSpMatArg->Rows;
				}
				else {
					const int Cols = TNodeJsUtil::GetArgInt32(Args, 0, "cols");
					const int Rows = TNodeJsUtil::GetArgInt32(Args, 0, "rows", -1);
					EAssert(Cols > 0 && Rows > 0);
					JsSpMat->Mat.Gen(Cols);
					JsSpMat->Rows = Rows;
				}
			}
		}
		Args.GetReturnValue().Set(Instance);
	}
	if (JsSpMat->Rows != -1) {
		EAssertR(TLAMisc::GetMaxDimIdx(JsSpMat->Mat) < JsSpMat->Rows, "TNodeJsSpMat::New inconsistent rows parameter (maximal row index >= rows!)");
	}
  
}

void TNodeJsSpMat::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 2 && Args[0]->IsInt32() && Args[1]->IsInt32(),
        "Expected row and column indices as arguments.");

    int Row = Args[0]->Int32Value();
    int Col = Args[1]->Int32Value();

    TNodeJsSpMat* JsSpMat =
        ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());

    TInt Rows = JsSpMat->Rows;
    TInt Cols = JsSpMat->Mat.Len();
    EAssertR(Row >= 0 &&  (Row < Rows || Rows == -1) && Col >= 0 && Col < Cols,
        "sparse col matrix at: index out of bounds");        
    double Result = 0.0;
    const int Els = JsSpMat->Mat[Col].Len();
    for (int ElN = 0; ElN < Els; ElN++) {
        if (JsSpMat->Mat[Col][ElN].Key == Row) {
            Result = JsSpMat->Mat[Col][ElN].Dat;
            break;
        }
    }
    Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

void TNodeJsSpMat::put(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 3 && Args[0]->IsInt32() && Args[1]->IsInt32() &&
        Args[2]->IsNumber(),
        "Expected three arguments: row and col index, and a value");

    TNodeJsSpMat* JsSpMat =
        ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());

    const int Row = Args[0]->Int32Value();
    const int Col = Args[1]->Int32Value();
    const double Val = Args[2]->NumberValue();

    TInt Rows = JsSpMat->Rows;
    TInt Cols = JsSpMat->Mat.Len();
    EAssertR(Row >= 0 &&  (Row < Rows || Rows == -1) && Col >= 0 && Col < Cols, "sparse col matrix put: index out of bounds");

    bool FoundP = false;
    const int Els = JsSpMat->Mat[Col].Len();
    for (int ElN = 0; ElN < Els; ElN++) {
        if (JsSpMat->Mat[Col][ElN].Key == Row) {
            JsSpMat->Mat[Col][ElN].Dat = Val;
            FoundP = true;
            break;
        }
    }
    if (!FoundP) {
        JsSpMat->Mat[Col].Add(TIntFltKd(Row, Val));
        JsSpMat->Mat[Col].Sort();
    }
    // update dimension
    if (JsSpMat->Rows != -1) {
        if (Row >= JsSpMat->Rows) {
            JsSpMat->Rows = Row + 1;
        }
    }

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

void TNodeJsSpMat::indexGet(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Info.Holder());
    EAssertR(Index < (uint32_t)JsSpMat->Mat.Len(), "Sparse matrix index at: index out of bounds");
    Info.GetReturnValue().Set(TNodeJsSpVec::New(JsSpMat->Mat[Index], JsSpMat->Rows));
}

void TNodeJsSpMat::indexGet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsInt32(), "Expected one integer argument");
	int Index = TNodeJsUtil::GetArgInt32(Args, 0);
	
	TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
	EAssertR(Index < JsSpMat->Mat.Len(), "Sparse matrix getCol: index out of bounds");
	
	Args.GetReturnValue().Set(TNodeJsSpVec::New(JsSpMat->Mat[Index], JsSpMat->Rows));
}

void TNodeJsSpMat::indexSet(uint32_t Index, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Info.Holder());
    // EAssertR(Index < (uint32_t)JsSpMat->Mat.Len(), "Sparse matrix index set: index out of bounds");
    v8::Handle<v8::Object> ValObj = v8::Handle<v8::Object>::Cast(Value);
    JsSpMat->Mat[Index] = ObjectWrap::Unwrap<TNodeJsSpVec>(ValObj)->Vec;
    Info.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsSpMat::indexSet(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 2 && Args[0]->IsInt32() && Args[1]->IsObject(), "Expected two arguments: integer column index and a sparse vector");
	int Index = TNodeJsUtil::GetArgInt32(Args, 0);
	
	TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
	EAssertR(Index < JsSpMat->Mat.Len(), "Sparse matrix getCol: index out of bounds");

	JsSpMat->Mat[Index] = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[1]->ToObject())->Vec;

	Args.GetReturnValue().Set(Args.Holder());
}

void TNodeJsSpMat::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject() &&
        TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV"),
        "Expected a TIntFltKdV as the only argument");

    TNodeJsSpMat* JsSpMat =
        ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());

    TNodeJsSpVec* JsSpVec =
        ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());

    JsSpMat->Mat.Add(JsSpVec->Vec);
    if (JsSpMat->Rows.Val != -1) {
        JsSpMat->Rows = MAX(JsSpMat->Rows.Val, TLAMisc::GetMaxDimIdx(JsSpVec->Vec) + 1);
    }

    Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

void TNodeJsSpMat::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    if (Args.Length() > 0) {
        if (Args[0]->IsNumber()) {
            const double Scalar = Args[0]->NumberValue();
            TVec<TIntFltKdV> Result;
            // computation    
            TLinAlg::MultiplyScalar(Scalar, JsSpMat->Mat, Result);
            Args.GetReturnValue().Set(New(Result));
        } else if (Args[0]->IsObject()) { // Vector 
            int Rows = JsSpMat->Rows;
            if (JsSpMat->Rows == -1) { Rows = TLAMisc::GetMaxDimIdx(JsSpMat->Mat) + 1; }
            if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
                TNodeJsVec<TFlt, TAuxFltV>* JsVec =
                    ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
                TFltVV Result(Rows, 1);
                // Copy could be omitted if we implemented SparseColMat * TFltV
                TLinAlg::Multiply(JsSpMat->Mat, TFltVV(JsVec->Vec, JsVec->Vec.Len(), 1), Result, Rows);
                // create JS result with the Result vector    
                Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) { // Matrix 
                TNodeJsFltVV* JsMat =
                    ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());
                TFltVV Result(Rows, 1);
                Result.Gen(Rows, JsMat->Mat.GetCols());
                TLinAlg::Multiply(JsSpMat->Mat, JsMat->Mat, Result, Rows);
                Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV")) { // Sparse vector 
                TNodeJsSpVec* JsSpVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
                TFltVV Result(Rows, 1);
                // Copy could be omitted if we implemented SparseColMat * SparseVec
                TVec<TIntFltKdV> TempSpMat(1);
                TempSpMat[0] = JsSpVec->Vec;
				if (JsSpVec->Dim != -1) {
					EAssert(JsSpVec->Dim == JsSpMat->Mat.Len());
				}
                TLinAlg::Multiply(JsSpMat->Mat, TempSpMat, Result, Rows);
                Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) { // Sparse matrix 
                TNodeJsSpMat* JsSpMat2 =
                    ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
                if (JsSpMat2->Rows == -1) {
                    EAssertR(JsSpMat->Mat.Len() >= TLAMisc::GetMaxDimIdx(JsSpMat2->Mat) + 1,
                        "sparse_col_matrix * sparse_col_matrix: dimensions mismatch");
				} else {
					EAssertR(JsSpMat->Mat.Len() == JsSpMat2->Rows,
						"sparse_col_matrix * sparse_col_matrix: dimensions mismatch");
				}
                TFltVV Result(Rows, JsSpMat2->Mat.Len());    
                TLinAlg::Multiply(JsSpMat->Mat, JsSpMat2->Mat, Result);
                Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
            }
        }
    }
    // Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsSpMat::multiplyT(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    if (Args.Length() > 0) {
        if (Args[0]->IsNumber()) {
            double Scalar = Args[0]->NumberValue();
            TVec<TIntFltKdV> Result;
            // computation
            int Rows = JsMat->Rows;
            if (Rows == -1) {
                Rows = TLAMisc::GetMaxDimIdx(JsMat->Mat) + 1;
            }
            TLinAlg::Transpose(JsMat->Mat, Result, Rows);
            TLinAlg::MultiplyScalar(Scalar, Result, Result);
            Args.GetReturnValue().Set(New(Result));
        }
        if (Args[0]->IsObject()) {
            if (TNodeJsUtil::IsArgClass(Args, 0, "TFltV")) {
                TNodeJsVec<TFlt, TAuxFltV>* JsVec =
                    ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
                EAssertR(JsMat->Rows == -1 || JsMat->Rows == JsVec->Vec.Len(),
                    "sparse_col_matrix' * vector: dimensions mismatch");
                if (JsMat->Rows == -1) {
                    EAssertR(TLAMisc::GetMaxDimIdx(JsMat->Mat) < JsVec->Vec.Len(),
                        "sparse_col_matrix' * vector: dimensions mismatch");
                }
                // computation                
                int Cols = JsMat->Mat.Len();                
                TFltVV Result(Cols, 1);
                // Copy could be omitted if we implemented SparseColMat * TFltV
                TLinAlg::MultiplyT(JsMat->Mat, TFltVV(JsVec->Vec, JsVec->Vec.Len(), 1), Result);
                // create JS result with the Result vector    
                Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TFltVV")) {            
                TNodeJsFltVV* JsMat2 = ObjectWrap::Unwrap<TNodeJsFltVV>(Args[0]->ToObject());                
                EAssertR(JsMat->Rows == -1 || JsMat->Rows == JsMat2->Mat.GetRows(), "sparse_col_matrix' * matrix: dimensions mismatch");
                if (JsMat->Rows == -1) {
                    EAssertR(TLAMisc::GetMaxDimIdx(JsMat->Mat) < JsMat2->Mat.GetRows(), "sparse_col_matrix' * matrix: dimensions mismatch");
                }
                TFltVV Result;
                // computation
                int Cols = JsMat->Mat.Len();
                Result.Gen(Cols, JsMat2->Mat.GetCols());
                TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
                Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TIntFltKdV")) {
                TNodeJsSpVec* JsVec = ObjectWrap::Unwrap<TNodeJsSpVec>(Args[0]->ToObject());
                EAssertR(JsMat->Rows == -1 || JsVec->Dim == -1 || JsMat->Rows == JsVec->Dim, "sparse_col_matrix' * sparse_vector: dimensions mismatch");
                // computation                
                int Cols = JsMat->Mat.Len();
                
				if (JsMat->Rows == -1 && JsVec->Dim != -1) {
					// check that maxdimidx < dim
					EAssert(TLAMisc::GetMaxDimIdx(JsMat->Mat) < JsVec->Dim);
				}
				else if (JsMat->Rows != -1 && JsVec->Dim == -1) {
					// check that vectorMaxIdx < JsMat->Rows
					EAssert(TLAMisc::GetMaxDimIdx(JsVec->Vec) < JsMat->Rows);
				}
				
				TFltVV Result(Cols, 1);
                // Copy could be omitted if we implemented SparseColMat * SparseVec
                TVec<TIntFltKdV> TempSpMat(1);
                TempSpMat[0] = JsVec->Vec;                

                TLinAlg::MultiplyT(JsMat->Mat, TempSpMat, Result);
                // create JS result with the Result vector
                Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result.Get1DVec()));
            } else if (TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
                TNodeJsSpMat* JsMat2 = ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
                EAssertR(JsMat->Rows == -1 || JsMat2->Rows == -1 || JsMat->Rows == JsMat2->Rows, "sparse_col_matrix' * sparse_matrix: dimensions mismatch");
                // computation                
                int Cols = JsMat->Mat.Len();
                TFltVV Result(Cols, JsMat2->Mat.Len());                    
                TLinAlg::MultiplyT(JsMat->Mat, JsMat2->Mat, Result);
                // create JS result with the Result vector    
                Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
            }
        }
   }
}

void TNodeJsSpMat::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    if (Args.Length() > 0 && Args[0]->IsObject() &&
        TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
        TNodeJsSpMat* JsSpMat2 =
            ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
        EAssertR(JsSpMat->Rows == -1 || JsSpMat2->Rows == -1 ||
            JsSpMat->Rows == JsSpMat2->Rows,
            "matrix - matrix: dimensions mismatch");
        // create JS result and get the internal data
        TVec<TIntFltKdV> Result;
        // computation                
        Result.Gen(MAX(JsSpMat->Mat.Len(), JsSpMat2->Mat.Len()));
        int Len1 = JsSpMat->Mat.Len();
        int Len2 = JsSpMat2->Mat.Len();
        int Len = Result.Len();
        for (int ColN = 0; ColN < Len; ColN++) {
            if (ColN < Len1 && ColN < Len2) {
                TLinAlg::LinComb(1.0, JsSpMat->Mat[ColN], 1.0, JsSpMat2->Mat[ColN], Result[ColN]);
            }
            if (ColN >= Len1 && ColN < Len2) {
                Result[ColN] = JsSpMat2->Mat[ColN];
            }
            if (ColN < Len1 && ColN >= Len2) {
                Result[ColN] = JsSpMat->Mat[ColN];
            }
        }
        int Rows = -1;
        if (JsSpMat->Rows == -1 && JsSpMat2->Rows == -1) {
            Rows = TLAMisc::GetMaxDimIdx(Result) + 1;
        } else {
            Rows = MAX(JsSpMat->Rows, JsSpMat2->Rows);
        }
        Args.GetReturnValue().Set(New(Result, Rows));
    } else {
        Args.GetReturnValue().Set(v8::Undefined(Isolate));
    }
}

void TNodeJsSpMat::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    if (Args.Length() > 0 && Args[0]->IsObject() &&
        TNodeJsUtil::IsArgClass(Args, 0, "TVec<TIntFltKdV>")) {
        TNodeJsSpMat* JsSpMat2 =
            ObjectWrap::Unwrap<TNodeJsSpMat>(Args[0]->ToObject());
        EAssertR(JsSpMat->Rows == -1 || JsSpMat2->Rows == -1 ||
            JsSpMat->Rows == JsSpMat2->Rows,
            "matrix - matrix: dimensions mismatch");
        // create JS result and get the internal data
        TVec<TIntFltKdV> Result;
        // computation                
        Result.Gen(MAX(JsSpMat->Mat.Len(), JsSpMat2->Mat.Len()));
        int Len1 = JsSpMat->Mat.Len();
        int Len2 = JsSpMat2->Mat.Len();
        int Len = Result.Len();
        for (int ColN = 0; ColN < Len; ColN++) {
            if (ColN < Len1 && ColN < Len2) {
                TLinAlg::LinComb(1.0, JsSpMat->Mat[ColN], -1.0, JsSpMat2->Mat[ColN], Result[ColN]);
            }
            if (ColN >= Len1 && ColN < Len2) {
                Result[ColN].Gen(JsSpMat2->Mat[ColN].Len());
                TLinAlg::MultiplyScalar(-1, JsSpMat2->Mat[ColN], Result[ColN]);
            }
            if (ColN < Len1 && ColN >= Len2) {
                Result[ColN] = JsSpMat->Mat[ColN];
            }
        }
        int Rows = -1;
        if (JsSpMat->Rows == -1 && JsSpMat2->Rows == -1) {
            Rows = TLAMisc::GetMaxDimIdx(Result) + 1;
        } else {
            Rows = MAX(JsSpMat->Rows, JsSpMat2->Rows);
        }
        Args.GetReturnValue().Set(New(Result, Rows));
    } else {
        Args.GetReturnValue().Set(v8::Undefined(Isolate));
    }
}

void TNodeJsSpMat::transpose(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TVec<TIntFltKdV> Result;
    TLinAlg::Transpose(JsSpMat->Mat, Result);

    Args.GetReturnValue().Set(TNodeJsSpMat::New(Result));
}

void TNodeJsSpMat::colNorms(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TFltV Result;
    int Cols = JsSpMat->Mat.Len();
    Result.Gen(Cols);
    for (int ColN = 0; ColN < Cols; ColN++) {
        Result[ColN] = TLinAlg::Norm(JsSpMat->Mat[ColN]);
    }

    Args.GetReturnValue().Set(TNodeJsVec<TFlt, TAuxFltV>::New(Result));
}

void TNodeJsSpMat::normalizeCols(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TLinAlg::NormalizeColumns(JsSpMat->Mat);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsSpMat::full(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TFltVV Result;
    int Rows = JsSpMat->Rows;
    if (Rows == -1) {
        Rows = TLAMisc::GetMaxDimIdx(JsSpMat->Mat) + 1;
    }
    TLinAlg::Full(JsSpMat->Mat, Result, Rows);    

    Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
}

void TNodeJsSpMat::frob(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    double FrobNorm = 0.0;
    int Cols = JsSpMat->Mat.Len();
    for (int ColN = 0; ColN < Cols; ColN++) {
        FrobNorm += TLinAlg::Norm2(JsSpMat->Mat[ColN]);
    }
	FrobNorm = sqrt(FrobNorm);

    Args.GetReturnValue().Set(v8::Number::New(Isolate, FrobNorm));
}

void TNodeJsSpMat::rows(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat =
        ObjectWrap::Unwrap<TNodeJsSpMat>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsSpMat->Rows));
}

void TNodeJsSpMat::cols(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat =
        ObjectWrap::Unwrap<TNodeJsSpMat>(Info.Holder());

    Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsSpMat->Mat.Len()));
}

void TNodeJsSpMat::print(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());

    TStr SpMatStr;
    TLAMisc::PrintSpMat(JsSpMat->Mat, SpMatStr);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

void TNodeJsSpMat::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected TJsNodeFOut object");
    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
    PSOut SOut = JsFOut->SOut;
    JsSpMat->Mat.Save(*SOut);

    Args.GetReturnValue().Set(Args[0]);
}

void TNodeJsSpMat::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
    v8::Isolate* Isolate = v8::Isolate::GetCurrent();
    v8::HandleScope HandleScope(Isolate);

    EAssertR(Args.Length() == 1 && Args[0]->IsObject(), "Expected TJsNodeFIn object");
    TNodeJsSpMat* JsSpMat = ObjectWrap::Unwrap<TNodeJsSpMat>(Args.Holder());
    TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
    PSIn SIn = JsFIn->SIn;
    JsSpMat->Mat.Load(*SIn);

    Args.GetReturnValue().Set(v8::Undefined(Isolate));
}


//#ifndef MODULE_INCLUDE_LA
/////////////////////////////////
//// Register functions, etc.
//void init(v8::Handle<v8::Object> exports) {
//    TNodeJsLinAlg::Init(exports);
//    TNodeJsVec<TFlt, TAuxFltV>::Init(exports);
//    TNodeJsVec<TInt, TAuxIntV>::Init(exports);
//    TNodeJsBoolV::Init(exports);
//    TNodeJsFltVV::Init(exports);
//    TNodeJsSpVec::Init(exports);
//    TNodeJsSpMat::Init(exports);
//
//	// File stream
//	//TNodeJsFIn::Init(exports);
//	//TNodeJsFOut::Init(exports);
//}
//
//NODE_MODULE(la, init)
//#endif


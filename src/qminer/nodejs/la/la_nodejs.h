#ifndef QMINER_NODEJS
#define QMINER_NODEJS

#define BUILDING_NODE_EXTENSION

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"

// NOTE: This is *not* the same as in QMiner JS. 
#define JsDeclareProperty(Function) \
<<<<<<< .merge_file_a08792
	static void Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info);

#define JsDeclareFunction(Function) \
   static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args);

#define QmAssert(Cond) \
  if (!(Cond)) { Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, "[la addon] Exception"))); }

// XXX: The macro expects that the variable Isolate exists. 
#define QmAssertR(Cond, MsgStr) \
  if (!(Cond)) { Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr))); }

#define QmFailR(MsgStr) \
   Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr)));
=======
	static void Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info); \
	static void _ ## Function(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) { \
	   v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
	   v8::HandleScope HandleScope(Isolate); \
	   try { \
	      Function(Name, Info); \
	   } catch (const PExcept& Except) { \
	      /* if(typeid(Except) == typeid(TQmExcept::New(""))) { */ \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[la addon] Exception"))); \
         /* } else { \
            throw Except; \
         } */ \
	   } \
	}

#define JsDeclareFunction(Function) \
   static void Function(const v8::FunctionCallbackInfo<v8::Value>& Args); \
   static void _ ## Function(const v8::FunctionCallbackInfo<v8::Value>& Args) { \
      v8::Isolate* Isolate = v8::Isolate::GetCurrent(); \
      v8::HandleScope HandleScope(Isolate); \
      try { \
         Function(Args); \
      } catch (const PExcept& Except) { \
         /* if(typeid(Except) == typeid(TQmExcept::New(""))) { */ \
            Isolate->ThrowException(v8::Exception::TypeError( \
               v8::String::NewFromUtf8(Isolate, "[la addon] Exception"))); \
         /* } else { \
            throw Except; \
         } */ \
      } \
   }

// XXX: The macro expects that variables Args and Isolate exist. 
#define QmAssert(Cond) \
   if (!(Cond)) { \
      Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, "[la addon] Exception")))); return; }

// XXX: The macro expects that variable Args and Isolate exist. 
#define QmAssertR(Cond, MsgStr) \
  if (!(Cond)) { \
   Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr)))); return; }

// XXX: The macro expects that variables Args and Isolate exist. 
#define QmFailR(MsgStr) \
   Args.GetReturnValue().Set(Isolate->ThrowException(v8::Exception::TypeError( \
         v8::String::NewFromUtf8(Isolate, MsgStr)))); return;
>>>>>>> .merge_file_a04824

class TNodeJsUtil {
public:
   /// Extract argument ArgN property as bool
	static bool GetArgBool(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const bool& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
<<<<<<< .merge_file_a08792
				 QmAssertR(Val->IsBoolean(),
=======
				 EAssertR(Val->IsBoolean(),
>>>>>>> .merge_file_a04824
				   TStr::Fmt("Argument %d, property %s expected to be boolean", ArgN, Property.CStr()).CStr());
				 return static_cast<bool>(Val->BooleanValue());
			}
		}
		return DefVal;
	}
   static int GetArgInt32(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& Property, const int& DefVal) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		if (Args.Length() > ArgN) {			
			if (Args[ArgN]->IsObject() && Args[ArgN]->ToObject()->Has(v8::String::NewFromUtf8(Isolate, Property.CStr()))) {
				v8::Handle<v8::Value> Val = Args[ArgN]->ToObject()->Get(v8::String::NewFromUtf8(Isolate, Property.CStr()));
<<<<<<< .merge_file_a08792
				 QmAssertR(Val->IsInt32(),
=======
				 EAssertR(Val->IsInt32(),
>>>>>>> .merge_file_a04824
				   TStr::Fmt("Argument %d, property %s expected to be int32", ArgN, Property.CStr()).CStr());
				 return Val->ToNumber()->Int32Value();
			}
		}
		return DefVal;
	}
	// gets the class name of the underlying glib object. the name is stored in an hidden variable "class"
	static TStr GetClass(const v8::Handle<v8::Object> Obj) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		v8::Local<v8::Value> ClassNm = Obj->GetHiddenValue(v8::String::NewFromUtf8(Isolate, "class"));
		const bool EmptyP = ClassNm.IsEmpty();
		if (EmptyP) { return ""; }
		v8::String::Utf8Value Utf8(ClassNm);
		return TStr(*Utf8);		
	}

	// checks if the class name of the underlying glib object matches the given string. the name is stored in an hidden variable "class"
	static bool IsClass(const v8::Handle<v8::Object> Obj, const TStr& ClassNm) {
		TStr ObjClassStr = GetClass(Obj);		
		return ObjClassStr == ClassNm;
	}
	/// Check if argument ArgN belongs to a given class
	static bool IsArgClass(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN, const TStr& ClassNm) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
<<<<<<< .merge_file_a08792
		QmAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()).CStr());
      QmAssertR(Args[ArgN]->IsObject(), TStr("Argument expected to be '" + ClassNm + "' but is not even an object!").CStr());
=======
		EAssertR(Args.Length() > ArgN, TStr::Fmt("Missing argument %d of class %s", ArgN, ClassNm.CStr()).CStr());
      EAssertR(Args[ArgN]->IsObject(), TStr("Argument expected to be '" + ClassNm + "' but is not even an object!").CStr());
>>>>>>> .merge_file_a04824
		v8::Handle<v8::Value> Val = Args[ArgN];
	 	v8::Handle<v8::Object> Data = v8::Handle<v8::Object>::Cast(Val);			
		TStr ClassStr = GetClass(Data);
		return ClassStr.EqI(ClassNm);
	}
};

///////////////////////////////
// NodeJs-Qminer-LinAlg
// XXX: Current implementation doesn't support all documented properties! 
class TNodeJsLinAlg : public node::ObjectWrap {
public:
   //# 
	//# **Functions and properties:**
	//# 
	//#- `intVec = la.newIntVec()` -- generate an empty integer vector
	//#- `intVec = la.newIntVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
	//#- `intVec = la.newIntVec(arr)` -- copy a javascript int array `arr` 
	//#- `intVec = la.newIntVec(vec2)` -- clone an int vector `vec2`
	// JsDeclareFunction(newIntVec);
	//#- `mat = la.newMat()` -- generates a 0x0 matrix
	//#- `mat = la.newMat({"rows":num, "cols":num2, "random":bool})` -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
	//#- `mat = la.newMat(nestedArr)` -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
	//#- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`
	JsDeclareFunction(newMat);
	//#- `spVec = la.newSpVec(len)` -- creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
	//#- `spVec = la.newSpVec(nestedArr, len)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
	JsDeclareFunction(newSpVec);
	//#- `spMat = la.newSpMat()` -- creates an empty sparse matrix `spMat`
	//#- `spMat = la.newSpMat(rowIdxVec, colIdxVec, valVec)` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
	//#- `spMat = la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
	//#- `spMat = la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
	JsDeclareFunction(newSpMat);
	//#- `svdRes = la.svd(mat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
	//#- `svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
	JsDeclareFunction(svd);	
    //TODO: #- `intVec = la.loadIntVeC(fin)` -- load integer vector from input stream `fin`.
    //JsDeclareFunction(loadIntVec);
	//#JSIMPLEMENT:src/qminer/linalg.js
private:
   // 
};

class TAuxFltV {
public:	
	static const TStr ClassId; //ClassId is set to "TFltV"
	//static v8::Handle<v8::Value> GetObjVal(const double& Val, v8::HandleScope& Handlescope) {
	//	return Handlescope.Close(v8::Number::New(Val));
	//}
	//static double GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	//	return TJsObjUtil<TJsVec<TFlt, TAuxFltV> >::GetArgFlt(Args, ArgN);
	//}
	static double CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToNumber()->Value();
	}
};

class TAuxIntV {
public:	
	static const TStr ClassId; //ClassId is set to "TIntV"
	// static v8::Handle<v8::Value> GetObjVal(const int& Val, v8::HandleScope& Handlescope) {
	//	return Handlescope.Close(v8::Integer::New(Val));
	//}
	//static int GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	//	return TJsObjUtil<TJsVec<TInt, TAuxIntV> >::GetArgInt32(Args, ArgN);
	// }
	static int CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToInt32()->Value();
	}
};

///////////////////////////////
// NodeJs-GLib-TVec
template <class TVal = TFlt, class TAux = TAuxFltV>
class TNodeJsVec : public node::ObjectWrap {
   friend class TNodeJsFltVV;
public: // So we can register the class 
   static void Init(v8::Handle<v8::Object> exports);
   // Does the job of the new operator in Javascript 
<<<<<<< .merge_file_a08792
   static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& Args);
=======
   static v8::Handle<v8::Value> NewInstance(const v8::FunctionCallbackInfo<v8::Value>& Args);
   static v8::Local<v8::Object> New(const TFltV& FltV);
   static v8::Local<v8::Object> New(v8::Local<v8::Array> Arr);
>>>>>>> .merge_file_a04824
public:
   TNodeJsVec() { }
   TNodeJsVec(const TFltV& FltV) : Vec(FltV) { }
private:
   // Returns a new integer vector 
   JsDeclareFunction(newIntVec);
   JsDeclareFunction(New);
   /// JsDeclareFunction(newIntVec);
   //#- `num = vec.at(idx)` -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
	//#- `num = intVec.at(idx)` -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
	JsDeclareFunction(at);
	//#- `vec2 = vec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	//#- `intVec2 = intVec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	JsDeclareFunction(subVec);
	//#- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
	//// JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `vec = vec.put(idx, num)` -- set value of vector `vec` at index `idx` to `num` (0-based indexing). Returns self.
	//#- `intVec = intVec.put(idx, num)` -- set value of integer vector `intVec` at index `idx` to `num` (0-based indexing). Returns self.
	JsDeclareFunction(put);	
	//#- `len = vec.push(num)` -- append value `num` to vector `vec`. Returns `len` - the length  of the modified array
	//#- `len = intVec.push(num)` -- append value `num` to integer vector `intVec`. Returns `len` - the length  of the modified array
	JsDeclareFunction(push);
	//#- `len = vec.unshift(num)` -- insert value `num` to the begining of vector `vec`. Returns the length of the modified array.
	//#- `len = intVec.unshift(num)` -- insert value `num` to the begining of integer vector `intVec`. Returns the length of the modified array.
	JsDeclareFunction(unshift);
	//#- `len = vec.pushV(vec2)` -- append vector `vec2` to vector `vec`.
	//#- `len = intVec.pushV(intVec2)` -- append integer vector `intVec2` to integer vector `intVec`.
	////JsDeclareTemplatedFunction(pushV);
	//#- `num = vec.sum()` -- return `num`: the sum of elements of vector `vec`
	//#- `num = intVec.sum()` -- return `num`: the sum of elements of integer vector `intVec`
	JsDeclareFunction(sum);
	//#- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in vector `vec`
	//#- `idx = intVec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in integer vector `vec`
	JsDeclareFunction(getMaxIdx);
	//#- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	//#- `intVec2 = intVec.sort(asc)` -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	JsDeclareFunction(sort);
	//#- `sortRes = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
	////JsDeclareTemplatedFunction(sortPerm);	
	//#- `vec = vec.shuffle()` -- shuffels the vector `vec` (inplace operation). Returns self.
	JsDeclareFunction(shuffle);
	//#- `vec = vec.trunc(num)` -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
	JsDeclareFunction(trunc);
	//#- `mat = vec.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only. 
	////JsDeclareTemplatedFunction(outer);
	//#- `num = vec.inner(vec2)` -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////(TODO)JsDeclareTemplatedFunction(inner);
	JsDeclareFunction(inner);
	//#- `vec3 = vec.plus(vec2)` --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////(TODO)JsDeclareTemplatedFunction(plus);
	JsDeclareFunction(plus);
	//#- `vec3 = vec.minus(vec2)` --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	////(TODO)JsDeclareTemplatedFunction(minus);
	JsDeclareFunction(minus);
	//#- `vec2 = vec.multiply(num)` --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
	////(TODO)JsDeclareTemplatedFunction(multiply);
	JsDeclareFunction(multiply);
	//#- `vec = vec.normalize()` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
	JsDeclareFunction(normalize);
	////(TODO)JsDeclareTemplatedFunction(normalize);
	//#- `len = vec.length` -- integer `len` is the length of vector `vec`
	//#- `len = intVec.length` -- integer `len` is the length of integer vector `vec`
	JsDeclareProperty(length);
	//#- `vec = vec.print()` -- print vector in console. Returns self.
	//#- `intVec = intVec.print()` -- print integer vector in console. Returns self.
	////JsDeclareFunction(print);
	//#- `mat = vec.diag()` -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(diag);
	//#- `spMat = vec.spDiag()` -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(spDiag);
	//#- `num = vec.norm()` -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
	////(TODO)JsDeclareTemplatedFunction(norm);
	JsDeclareFunction(norm);
	//#- `spVec = vec.sparse()` -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
	////JsDeclareTemplatedFunction(sparse);
	//#- `mat = vec.toMat()` -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
	//#- `mat = intVec.toMat()` -- `mat` is a matrix with a single column that is equal to dense integer vector `intVec`.
	////JsDeclareTemplatedFunction(toMat);
public:
   TFltV Vec;
private:
   static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Qminer-FltVV
class TNodeJsFltVV : public node::ObjectWrap {
public:
   static void Init(v8::Handle<v8::Object> exports);
<<<<<<< .merge_file_a08792
=======
   static v8::Handle<v8::Value> New(const TFltVV& FltVV);
   static v8::Handle<v8::Value> New(v8::Local<v8::Array> Arr);
>>>>>>> .merge_file_a04824
public:
   TNodeJsFltVV() { } 
   TNodeJsFltVV(const TFltVV& _Mat) : Mat(_Mat) { } 
private:
   //# 
	//# **Functions and properties:**
	//# 
	JsDeclareFunction(New);
	//#- `num = mat.at(rowIdx,colIdx)` -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);	
	//#- `mat = mat.put(rowIdx, colIdx, num)` -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `mat2 = mat.multiply(num)` -- Matrix multiplication: `num` is a number, `mat2` is a matrix
	//#- `vec2 = mat.multiply(vec)` -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
	//#- `vec = mat.multiply(spVec)` -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
	//#- `mat3 = mat.multiply(mat2)` -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
	//#- `mat2 = mat.multiply(spMat)` -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
	JsDeclareFunction(multiply);
	//#- `mat2 = mat.multiplyT(num)` -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec2 = mat.multiplyT(vec)` -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `vec = mat.multiplyT(spVec)` -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat3 = mat.multiplyT(mat2)` -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//#- `mat2 = mat.multiplyT(spMat)` -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	JsDeclareFunction(multiplyT);
	//#- `mat3 = mat.plus(mat2)` -- `mat3` is the sum of matrices `mat` and `mat2`
	JsDeclareFunction(plus);
	//#- `mat3 = mat.minus(mat2)` -- `mat3` is the difference of matrices `mat` and `mat2`
	JsDeclareFunction(minus);
	//#- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat` transposed
	JsDeclareFunction(transpose);
	//#- `vec2 = mat.solve(vec)` -- vector `vec2` is the solution to the linear system `mat * vec2 = vec`
	JsDeclareFunction(solve);
	//#- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	JsDeclareFunction(rowNorms);
	//#- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	JsDeclareFunction(colNorms);
	//#- `mat = mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	JsDeclareFunction(sparse);
	//#- `num = mat.frob()` -- number `num` is the Frobenious norm of matrix `mat`
	JsDeclareFunction(frob);
	//#- `num = mat.rows` -- integer `num` corresponds to the number of rows of `mat`
	JsDeclareProperty(rows);
	//#- `num = mat.cols` -- integer `num` corresponds to the number of columns of `mat`
	JsDeclareProperty(cols);
	//#- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	JsDeclareFunction(printStr);
	//#- `mat = mat.print()` -- print matrix `mat` to console. Returns self.
	JsDeclareFunction(print);
	//#- `colIdx = mat.rowMaxIdx(rowIdx)`: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
	JsDeclareFunction(rowMaxIdx);
	//#- `rowIdx = mat.colMaxIdx(colIdx)`: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
	JsDeclareFunction(colMaxIdx);
	//#- `vec = mat.getCol(colIdx)` -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
	JsDeclareFunction(getCol);
	//#- `mat = mat.setCol(colIdx, vec)` -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
	JsDeclareFunction(setCol);
	//#- `vec = mat.getRow(rowIdx)` -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
	JsDeclareFunction(getRow);
	//#- `mat.setRow(rowIdx, vec)` -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setRow);
	//#- `vec = mat.diag()` -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
	JsDeclareFunction(diag);
	//#- `mat = mat.save(fout)` -- print `mat` (full matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//#- `mat = mat.load(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(load);
public:
   TFltVV Mat;
private:
   static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-QMiner-Sparse-Vector
//# 
//# ### SpVector (sparse vector)
//# 
//# Sparse vector is an array of (int,double) pairs that represent column indices and values (TIntFltKdV is implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse vectors can be generated in the following ways:
//# 
//# ```JavaScript
//# var spVec = la.newSpVec(); //empty vector
//# // refer to la.newSpVec for alternative ways to generate sparse vectors
//# ```
//# 
class TNodeJsSpV : public node::ObjectWrap {
public:
   //# 
	//# **Functions and properties:**
	//# 
	//#- `num = spVec.at(idx)` -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
	JsDeclareFunction(at);	
	//#- `spVec = spVec.put(idx, num)` -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
	JsDeclareFunction(put);		
	//#- `num = spVec.sum()` -- `num` is the sum of elements of `spVec`
	JsDeclareFunction(sum);	
	//#- `num = spVec.inner(vec)` -- `num` is the inner product between `spVec` and dense vector `vec`.
	//#- `num = spVec.inner(spVec)` -- `num` is the inner product between `spVec` and sparse vector `spVec`.
	JsDeclareFunction(inner);	
	//#- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
	JsDeclareFunction(multiply);
	//#- `spVec = spVec.normalize()` -- normalizes the vector spVec (inplace operation). Returns self.
	JsDeclareFunction(normalize);
	//#- `num = spVec.nnz` -- gets the number of nonzero elements `num` of vector `spVec`
	////(TODO)JsDeclareProperty(nnz);	
	//#- `num = spVec.dim` -- gets the dimension `num` (-1 means that it is unknown)
	////(TODO)JsDeclareProperty(dim);	
	//#- `spVec = spVec.print()` -- prints the vector to console. Return self.
	JsDeclareFunction(print);
	//#- `num = spVec.norm()` -- returns `num` - the norm of `spVec`
	JsDeclareFunction(norm);
	//#- `vec = spVec.full()` --  returns `vec` - a dense vector representation of sparse vector `spVec`.
	JsDeclareFunction(full);
	//#- `valVec = spVec.valVec()` --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
	JsDeclareFunction(valVec);
	//#- `idxVec = spVec.idxVec()` --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
	JsDeclareFunction(idxVec);
private:
   //
};

///////////////////////////////
// NodeJs-QMiner-Sparse-Col-Matrix
//# 
//# ### SpMatrix (sparse column matrix)
//# 
//# SpMatrix is a sparse matrix represented as a dense vector of sparse vectors which correspond to matrix columns (TVec<TIntFltKdV>, implemented in glib/base/ds.h.)
//# Using the global `la` object, sparse matrices are generated in several ways:
//# 
//# ```JavaScript
//# var spMat = la.newSpMat(); //empty matrix
//# // refer to la.newSpMat function for alternative ways to generate sparse matrices
//# ```
//# 
class TNodeJsSpMat : public node::ObjectWrap {
public:
   //# 
	//# **Functions and properties:**
	//# 
	//#- `num = spMat.at(rowIdx,colIdx)` -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//#- `spMat = spMat.put(rowIdx, colIdx, num)` -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//#- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	////(TODO)JsDeclGetSetIndexedProperty(indexGet, indexSet);
	//#- `spMat = spMat.push(spVec)` -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
	JsDeclareFunction(push);
	//#- `spMat2 = spMat.multiply(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
	//#- `vec2 = spMat.multiply(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
	//#- `vec = spMat.multiply(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
	//#- `mat2 = spMat.multiply(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
	//#- `mat = spMat.multiply(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
	JsDeclareFunction(multiply);
	//#- `spMat2 = spMat.multiplyT(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec2 = spMat.multiplyT(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `vec = spMat.multiplyT(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat2 = spMat.multiplyT(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//#- `mat = spMat.multiplyT(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
	JsDeclareFunction(multiplyT);
	//#- `spMat3 = spMat.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(plus);
	//#- `spMat3 = spMat.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(minus);
	//#- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed 
	JsDeclareFunction(transpose);	
	//#- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	JsDeclareFunction(colNorms);
	//#- `spMat = spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//#- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	JsDeclareFunction(full);
	//#- `num = spMat.frob()` -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
	JsDeclareFunction(frob);
	//#- `num = spMat.rows` -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
	JsDeclareProperty(rows);
	//#- `num = spMat.cols` -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
	JsDeclareProperty(cols);
	//#- `spMat = spMat.print()` -- print `spMat` (sparse matrix) to console. Returns self.
	JsDeclareFunction(print);
	//#- `spMat = spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//#- `spMat = spMat.load(fin)` -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
	JsDeclareFunction(load);
	//#JSIMPLEMENT:src/qminer/spMat.js
private:
   //
};

#endif


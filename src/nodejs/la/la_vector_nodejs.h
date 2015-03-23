#ifndef QMINER_LA_VEC_NODEJS_H
#define QMINER_LA_VEC_NODEJS_H


#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"
#include "la_nodejs.h"

// Vector helper classes
class TNodeJsFltVV;

class TAuxFltV {
public:
	static const TStr ClassId; //ClassId is set to "TFltV"
	static v8::Handle<v8::Value> GetObjVal(const double& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return HandleScope.Escape(v8::Number::New(Isolate, Val));
	}
	//static double GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	//    return TJsObjUtil<TJsVec<TFlt, TAuxFltV> >::GetArgFlt(Args, ArgN);
	//}
	static double CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToNumber()->Value();
	}
	static TFlt Parse(const TStr& Str) {
		return Str.GetFlt();
	}
};

class TAuxIntV {
public:
	static const TStr ClassId; //ClassId is set to "TIntV"
	static v8::Handle<v8::Value> GetObjVal(const int& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return HandleScope.Escape(v8::Integer::New(Isolate, Val));
	}
	//static int GetArgVal(const v8::FunctionCallbackInfo<v8::Value>& Args, const int& ArgN) {
	//    return TJsObjUtil<TJsVec<TInt, TAuxIntV> >::GetArgInt32(Args, ArgN);
	// }
	static int CastVal(const v8::Local<v8::Value>& Value) {
		return Value->ToInt32()->Value();
	}
	static TInt Parse(const TStr& Str) {
		return Str.GetInt();
	}
};

class TAuxStrV {
public:
	static const TStr ClassId; //ClassId is set to "TStrV"
	static v8::Handle<v8::Value> GetObjVal(const TStr& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return HandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
	}
	static TStr CastVal(const v8::Local<v8::Value>& Value) {
		v8::String::Utf8Value Utf8(Value);
		return TStr(*Utf8);
	}
	static TStr Parse(const TStr& Str) {
		return Str;
	}
};

class TAuxBoolV {
public:
	static const TStr ClassId; //ClassId is set to "TBoolV"
	static v8::Handle<v8::Value> GetObjVal(const TBool& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return v8::Boolean::New(Isolate, Val);
	}
	static bool CastVal(const v8::Local<v8::Value>& Value) {
		return Value->BooleanValue();
	}
	static TBool Parse(const TStr& Str) {
		return TBool::GetValFromStr(Str);
	}
};

///////////////////////////////
// NodeJs-Linalg-Vector
//! 
//! ### Vector
//! 
//! Vector is an array of objects implemented in glib/base/ds.h. 
//! Some functions are implemented for float vectors only. Using the global `la` object, flaot and int vectors can be generated in the following ways:
//! 
//! ```JavaScript
//! var vec = la.newVec(); //empty vector
//! var intVec = la.newIntVec(); //empty vector
//! // refer to la.newVec, la.newIntVec functions for alternative ways to generate vectors
//! ```
//! 

/**
* <% title %>
* @classdesc Wraps a C++ array.
* @class
* @param {(Array<<% elementType %>> | module:la.<% className %>)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. An array of vector elements. Example: <% example1 %> is a vector of length 3.
* <br>2. A vector (copy constructor).
* @example
* // create a new empty vector
* var vec = new la.<% className %>();
* // create a new vector
* var vec2 = new la.<% className %>(<% example1 %>);
*/
//# exports.<% className %> = function() {}
template <class TVal = TFlt, class TAux = TAuxFltV>
class TNodeJsVec : public node::ObjectWrap {
	friend class TNodeJsFltVV;
public: // So we can register the class 
	const static TStr GetClassId() { return TAux::ClassId; }

	static void Init(v8::Handle<v8::Object> exports);
	// Does the job of the new operator in Javascript 
	//static v8::Handle<v8::Value> NewInstance(const v8::FunctionCallbackInfo<v8::Value>& Args);
	static v8::Local<v8::Object> New(const TFltV& FltV);
	static v8::Local<v8::Object> New(const TIntV& IntV);
	static v8::Local<v8::Object> New(const TStrV& StrV);
	static v8::Local<v8::Object> New(const TBoolV& BoolV);
	//static v8::Local<v8::Object> New(v8::Local<v8::Array> Arr);
public:
	TNodeJsVec() : Vec() { }
	TNodeJsVec(const TVec<TVal>& ValV) : Vec(ValV) { }
public:
	JsDeclareFunction(New);
private:
	//! 
	//! **Functions and properties:**
	//! 
	//!- `num = vec.at(idx)` -- gets the value `num` of vector `vec` at index `idx`  (0-based indexing)
	//!- `num = intVec.at(idx)` -- gets the value `num` of integer vector `intVec` at index `idx`  (0-based indexing)
	
	
	/**
	* Returns element at index.
	* @param {number} index - Element index (zero-based).
	* @returns {<% elementType %>} Vector element.
	*/
	//# exports.<% className %>.prototype.at = function(number) {}
	JsDeclareFunction(at);
	
	//!- `vec2 = vec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	//!- `intVec2 = intVec.subVec(intVec)` -- gets the subvector based on an index vector `intVec` (indices can repeat, 0-based indexing)
	/**
	* Returns a subvector.
	* @param {(Array<number> | module:la.IntVector)} arg - Index array or vector. Indices can repeat (zero based).
	* @returns {module:la.<% className %>} Subvector, where the i-th element is the arg[i]-th element of the instance.
	*/
	//# exports.<% className %>.prototype.subVec = function (arg) {}
	JsDeclareFunction(subVec);
	
	//!- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
	JsDeclareSetIndexedProperty(indexGet, indexSet);

	//!- vec = vec.put(i, val)
	/**
	* Sets an element in vector.
	* @param {number} idx - Index (zero based).
	* @param {<% elementType %>} val - Element value.
	* @returns {module:la.<% className %>} Self.
	*/
	//# exports.<% className %>.prototype.put = function (idx, val) {}
	JsDeclareFunction(put);

	//!- bool = vector.push(val)
	/**
	* Adds an element to the end of the vector.
	* @param {<% elementType %>} val - The element added to the vector.
	* @returns {number} The new length property of the object upon which the method was called.
	*/
	//# exports.<% className %>.prototype.push = function (val) {}
	JsDeclareFunction(push);

	//! /**
	//! * The splice() method changes the content of an array by removing existing elements and/or adding new elements.
	//! *
	//! *  array.splice(start, deleteCount[, item1[, item2[, ...]]])
	//! *
	//! * @param {Number} start - Index at which to start changing the array. If greater than the length of the array, actual starting index will be set to the length of the array. If negative, will begin that many elements from the end.
	//! * @param {Number} deleteCount - An integer indicating the number of old array elements to remove. If deleteCount is 0, no elements are removed. In this case, you should specify at least one new element. If deleteCount is greater than the number of elements left in the array starting at start, then all of the elements through the end of the array will be deleted.
	//! * @param {Object} [itemN] - The element to add to the array. If you don't specify any elements, splice() will only remove elements from the array.
	//! * @returns {Vector} - a reference to itself
	//! */

	/**
	* Changes the vector by removing and adding elements.
	* @param {number} start - Index at which to start changing the array.
	* @param {number} deleteCount - 
	*/
	JsDeclareFunction(splice);

	//!- len = vector.unshift(val)
	/**
	* Adds an element to the beginning of the vector.
	* @param {<% elementType %>} val - The element added to the vector.
	* @returns {number} The new length of vector.
	*/
	//# exports.<% className %>.prototype.unshift = function (val) {}
	JsDeclareFunction(unshift);

	//!- `bool = vec.pushV(vec2)` -- append vector `vec2` to vector `vec`.
	//!- `bool = intVec.pushV(intVec2)` -- append integer vector `intVec2` to integer vector `intVec`.
	/**
	* Appends a second vector to the first one.
	* @param {module:la.<% className %>} vec - The appended vector.
	* @returns {boolean} True, if appending the vector is successful. Otherwise, false.
	*/
	//# exports.<% className %>.prototype.pushV = function (vec) {}
	JsDeclareFunction(pushV);

	//! /**
	//! * Sums the elements in the vector.
	//! *
	//! * @returns {Object} - the sum
	//! */
	/** 
	* Sums the elements in the vector.
	* @returns {number} The sum of all elements in the instance.
	*/
	//# <% skipSum %>exports.<% className %>.prototype.sum = function () {}
	JsDeclareFunction(sum);

	//!- `idx = vec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in vector `vec`
	//!- `idx = intVec.getMaxIdx()` -- returns the integer index `idx` of the maximal element in integer vector `vec`
	/**
	* Gets the index of the maximal element.
	* @returns {number} Index of the maximal element in the vector.
	*/
	//# <% skipGetMaxIdx %>exports.<% className %>.prototype.getMaxIdx = function () {}
	JsDeclareSpecializedFunction(getMaxIdx);

	//!- `vec2 = vec.sort(asc)` -- `vec2` is a sorted copy of `vec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	//!- `intVec2 = intVec.sort(asc)` -- integer vector `intVec2` is a sorted copy of integer vector `intVec`. `asc=true` sorts in ascending order (equivalent `sort()`), `asc`=false sorts in descending order
	/**
	* Sorts the vector in ascending or descending order (in place operation).
	* @param {boolean} [bool] - Default is true. TODO: support comparator callback
	* @returns {module:la.<% className %>} Self
	* <br>1. Vector sorted in ascending order, if bool is true.  
	* <br>2. Vector sorted in descending order, if bool is false.
	*/
	//# <% skipSort %>exports.<% className %>.prototype.sort = function (bool) {} 
	JsDeclareFunction(sort);

	//!- `sortRes = vec.sortPerm(asc)` -- returns a sorted copy of the vector in `sortRes.vec` and the permutation `sortRes.perm`. `asc=true` sorts in ascending order (equivalent `sortPerm()`), `asc`=false sorts in descending order.
	JsDeclareSpecializedFunction(sortPerm);

	//! /**
	//! * Randomly reorders the elements of the vector.
	//! *
	//! * @returns {Vector} - returns a reference to itself
	//! */
	/**
	* Randomly reorders the elements of the vector (inplace).
	* @returns {boolean} True, if reordering was successful. Otherwise, false.
	*/
	//# exports.<% className %>.prototype.shuffle = function () {}
	JsDeclareFunction(shuffle);

	//!- `vec = vec.trunc(num)` -- truncates the vector `vec` to lenght 'num' (inplace operation). Returns self.
	//!- `intVec = intVec.trunc(num)` -- truncates the vector `intVec` to lenght 'num' (inplace operation). Returns self.
	/**
	* Adds an element to the front of the vector.
	* @param {<% elementType %>} val - Added value.
	* @returns {module:la.<% className %>} Self.
	*/
	//# exports.<% className %>.prototype.trunc = function (val) {} 
	JsDeclareFunction(trunc);
	
	//!- `mat = vec.outer(vec2)` -- the dense matrix `mat` is a rank-1 matrix obtained by multiplying `vec * vec2^T`. Implemented for dense float vectors only. 
	/**
	* Creates a dense matrix A by multiplying two vectors x and y: A = x * y^T.
	* @param {module:la.<% className %>} vec - Second vector.
	* @returns {module:la.Matrix} Matrix obtained by the outer product of the instance and second vector.
	* @example
	* // create two vectors
	* var x = new la.<% className %>([1, 2, 3]);
	* var y = new la.<% className %>([4, 5]);
	* // create the outer product of these vectors
	* var A = vec.outer(vec2); // creates the dense matrix [[4, 5], [8, 10], [12, 15]]
	*/
	//# <% skipOuter %>exports.<% className %>.prototype.outer = function (vec) {}
	JsDeclareSpecializedFunction(outer);
	
	//!- `num = vec.inner(vec2)` -- `num` is the standard dot product between vectors `vec` and `vec2`. Implemented for dense float vectors only.
	/**
	* Computes the inner product.
	* @param {module:la.Vector} vec - Other vector
	* @returns {number} Inner product between the instance and the other vector.
	*/
	//# <% skipInner %>exports.<% className %>.prototype.inner = function(vec) {}
	JsDeclareSpecializedFunction(inner);

	//!- `num = vec.cosine(vec2)` -- `num` is the cosine between vectors `vec` and `vec2`. Implemented for dense float vectors only.
	/**
	* Returns the cosine between the two vectors.
	* @param {module:la.<% className %>} vec - Second vector.
	* @returns {number} The cosine between the two vectors.
	* @example
	* // create two vectors
	* var x = new la.<% className %>([1, 0]);
	* var y = new la.<% className %>([0, 1]);
	* // calculate the cosine between those two vectors
	* var num = x.cosine(y); // returns 0
	*/
	//# <% skipCosine %>exports.<% className %>.prototype.cosine = function (vec) {}
	JsDeclareSpecializedFunction(cosine);

	//!- `vec3 = vec.plus(vec2)` --`vec3` is the sum of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	/**
	* Sums the two vectors together.
	* @param {module:la.<% className %>} vec - Second vector.
	* @returns {module:la.<% className %>} Sum of the instance and the second vector.
	*/
	//# <% skipPlus %>exports.<% className %>.prototype.plus = function (vec) {}
	JsDeclareSpecializedFunction(plus);

	//!- `vec3 = vec.minus(vec2)` --`vec3` is the difference of vectors `vec` and `vec2`. Implemented for dense float vectors only.
	/**
	* Subtracts one vector from the other.
	* @param {module:la.<% className %>} vec - Second vector.
	* @returns {module:la.<% className %>} The difference of the instance and the other vector.
	*/
	//# <% skipMinus %>exports.<% className %>.prototype.minus = function (vec) {}
	JsDeclareSpecializedFunction(minus);

	//!- `vec2 = vec.multiply(num)` --`vec2` is a vector obtained by multiplying vector `vec` with a scalar (number) `num`. Implemented for dense float vectors only.
	/**
	* Multiplies the vector with a scalar.
	* @param {number} val - Scalar.
	* @returns {module:la.<% className %>} Product of the vector and scalar.
	*/
	//# <% skipMultiply %>exports.<% className %>.prototype.multiply = function (val) {} 
	JsDeclareSpecializedFunction(multiply);

	//!- `vec = vec.normalize()` -- normalizes the vector `vec` (inplace operation). Implemented for dense float vectors only. Returns self.
	/**
	* Normalizes vector.
	* @returns {module:la.<% className %>} Self - Normalized.
	*/
	//# <% skipNormalize %>exports.<% className %>.prototype.normalize = function () {} 
	JsDeclareSpecializedFunction(normalize);

	//!- `len = vec.length` -- integer `len` is the length of vector `vec`
	//!- `len = intVec.length` -- integer `len` is the length of integer vector `vec`
	/**
	* Gives the length of vector.
	* @returns {number} Length of vector.
	*/
	//# exports.<% className %>.prototype.length = undefined;
	JsDeclareProperty(length);

	//!- `vec = vec.toString()` -- returns string representation of the vector. Returns self.
	//!- `intVec = intVec.toString()` -- returns string representation of the integer vector. 
	/**
	* Returns the vector as string.
	* @returns {string} Instance as string.
	* @example
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // create vector as string
	* vec.toString(); // returns <% output1 %>
	*/
	//# exports.<% className %>.prototype.toString = function () {}
	JsDeclareFunction(toString);

	//!- `mat = vec.diag()` -- `mat` is a diagonal dense matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	/**
	* Creates a dense diagonal matrix out of the vector.
	* @returns{module:la.Matrix} Diagonal matrix, where the (i, i)-th element is the i-th element of vector.
	*/
	//# <% skipDiag %>exports.<% className %>.prototype.diag = function () {}
	JsDeclareSpecializedFunction(diag);

	//!- `spMat = vec.spDiag()` -- `spMat` is a diagonal sparse matrix whose diagonal equals `vec`. Implemented for dense float vectors only.
	/**
	* Creates a sparse diagonal matrix out of the vector.
	* @returns {module:la.SparseMatrix} Diagonal matrix, where the (i, i)-th element is the i-th element of vector.
	*/
	//# <% skipSpDiag %>exports.<% className %>.prototype.spDiag = function () {}
	JsDeclareSpecializedFunction(spDiag);

	//!- `num = vec.norm()` -- `num` is the Euclidean norm of `vec`. Implemented for dense float vectors only.
	/**
	* Calculates the norm of the vector.
	* @returns {number} The norm of the vector.
	*/
	//# <% skipNorm %>exports.<% className %>.prototype.norm = function () {}
	JsDeclareSpecializedFunction(norm);

	//!- `spVec = vec.sparse()` -- `spVec` is a sparse vector representation of dense vector `vec`. Implemented for dense float vectors only.
	/**
	* Creates the sparse vector representation of the vector.
	* @returns {module:la.SparseVector} The sparse vector representation.
	*/
	//# <% skipSparse %>exports.<% className %>.prototype.sparse = function () {}
	JsDeclareSpecializedFunction(sparse);

	//!- `mat = vec.toMat()` -- `mat` is a matrix with a single column that is equal to dense vector `vec`.
	/**
	* Creates a matrix with a single column that is equal to the vector.
	* @returns {module:la.Matrix} The matrix with a single column that is equal to the instance.
	*/
	//# <% skipToMat %>exports.<% className %>.prototype.toMat = function () {}
	JsDeclareSpecializedFunction(toMat);

	//!- `fout = vec.save(fout)` -- saves to output stream `fout`
	//!- `fout = intVec.save(fout)` -- saves to output stream `fout`
	/**
	* Saves the vector as output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} fout.
	* @example
	* // import the fs package
	* var fs = require('qminer.fs');
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // open write stream
	* var fout = fs.openWrite('vec.dat');
	* // save matrix and close write stream
	* vec.save(fout).close();
	*/
	//# <% skipSave %>exports.<% className %>.prototype.save = function (fout) {}
	JsDeclareFunction(save);

	//!- `vec = vec.load(fin)` -- loads from input stream `fin`
	//!- `intVec = intVec.load(fin)` -- loads from input stream `fin`
	/**
	* Loads the vector from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.<% className %>} Self.
	* @example
	* // import the fs package
	* var fs = require('qminer.fs');
	* // create an empty vector
	* var vec = new la.<% className %>();
	* // open a read stream
	* var fin = fs.openRead('vec.dat');
	* // load the matrix
	* vec.load(fin);
	*/
	//# <% skipLoad %>exports.<% className %>.prototype.load = function (fin) {}
	JsDeclareFunction(load);

	//!- `fout = vec.saveascii(fout)` -- saves to output stream `fout`
	//!- `fout = intVec.saveascii(fout)` -- saves to output stream `fout`
	JsDeclareFunction(saveascii);
	//!- `vec = vec.loadascii(fin)` -- loads from input stream `fin`
	//!- `intVec = intVec.loadascii(fin)` -- loads from input stream `fin`
	JsDeclareFunction(loadascii);
public:
	TVec<TVal> Vec;
private:
	static v8::Persistent<v8::Function> constructor;
};


///////////////////////////////
// NodeJs-GLib-TVec Implementations
template <typename TVal, typename TAux>
v8::Persistent<v8::Function> TNodeJsVec<TVal, TAux>::constructor;

typedef TNodeJsVec<TFlt, TAuxFltV> TNodeJsFltV;
typedef TNodeJsVec<TInt, TAuxIntV> TNodeJsIntV;
typedef TNodeJsVec<TStr, TAuxStrV> TNodeJsStrV;
typedef TNodeJsVec<TBool, TAuxBoolV> TNodeJsBoolV;


// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TFltV& FltV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TFltV");
	Instance->SetHiddenValue(Key, Value);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec = new TNodeJsVec<TFlt, TAuxFltV>(FltV);
	JsVec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TIntV& IntV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TFltV");
	Instance->SetHiddenValue(Key, Value);

	int Len = IntV.Len();
	TFltV Vec(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		Vec[ElN] = IntV[ElN];
	}

	TNodeJsVec<TFlt, TAuxFltV>* JsVec = new TNodeJsVec<TFlt, TAuxFltV>(Vec);
	JsVec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TFltV& FltV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntV");
	Instance->SetHiddenValue(Key, Value);

	int Len = FltV.Len();
	TIntV Vec(Len);
	for (int ElN = 0; ElN < Len; ElN++) {
		Vec[ElN] = (int)FltV[ElN];
	}

	TNodeJsVec<TInt, TAuxIntV>* JsVec = new TNodeJsVec<TInt, TAuxIntV>(Vec);
	JsVec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TIntV& IntV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);

	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TIntV");
	Instance->SetHiddenValue(Key, Value);

	TNodeJsVec<TInt, TAuxIntV>* JsVec = new TNodeJsVec<TInt, TAuxIntV>(IntV);
	JsVec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}


// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TStrV& StrV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TStr, TAuxStrV>::New: constructor is empty. Did you call TNodeJsVec<TStr, TAuxStrV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, constructor);

	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, "TStrV");
	Instance->SetHiddenValue(Key, Value);

	TNodeJsVec<TStr, TAuxStrV>* JsVec = new TNodeJsVec<TStr, TAuxStrV>(StrV);
	JsVec->Wrap(Instance);
	return HandleScope.Escape(Instance);
}

// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TStrV& StrV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TStrV& StrV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TFltV& FltV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TIntV& IntV) {
	throw TExcept::New("Not implemented");
}

// TODO implement the following three
template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TInt, TAuxIntV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented. TODO");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TStr, TAuxStrV>::New(const TBoolV& BoolV) {
	throw TExcept::New("Not implemented.");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TStrV& StrV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TFltV& FltV) {
	throw TExcept::New("Not implemented");
}

template <>
inline v8::Local<v8::Object> TNodeJsVec<TBool, TAuxBoolV>::New(const TIntV& IntV) {
	throw TExcept::New("Not implemented");
}



template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::Init(v8::Handle<v8::Object> exports) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();

	TStr Name = "Vector";
	if (TAux::ClassId == TNodeJsIntV::GetClassId()) Name = "IntVector";
	if (TAux::ClassId == TNodeJsStrV::GetClassId()) Name = "StrVector";
	if (TAux::ClassId == TNodeJsBoolV::GetClassId()) Name = "BoolVector";

	v8::Local<v8::FunctionTemplate> tpl = v8::FunctionTemplate::New(Isolate, _New);
	tpl->SetClassName(v8::String::NewFromUtf8(Isolate, Name.CStr()));
	// ObjectWrap uses the first internal field to store the wrapped pointer.
	tpl->InstanceTemplate()->SetInternalFieldCount(1);

	// Add all prototype methods, getters and setters here.
	NODE_SET_PROTOTYPE_METHOD(tpl, "at", _at);
	NODE_SET_PROTOTYPE_METHOD(tpl, "subVec", _subVec);
	NODE_SET_PROTOTYPE_METHOD(tpl, "put", _put);
	NODE_SET_PROTOTYPE_METHOD(tpl, "push", _push);
	NODE_SET_PROTOTYPE_METHOD(tpl, "splice", _splice);
	NODE_SET_PROTOTYPE_METHOD(tpl, "unshift", _unshift);
	NODE_SET_PROTOTYPE_METHOD(tpl, "pushV", _pushV);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sortPerm", _sortPerm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sum", _sum);
	NODE_SET_PROTOTYPE_METHOD(tpl, "getMaxIdx", _getMaxIdx);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sort", _sort);
	NODE_SET_PROTOTYPE_METHOD(tpl, "shuffle", _shuffle);
	NODE_SET_PROTOTYPE_METHOD(tpl, "trunc", _trunc);
	NODE_SET_PROTOTYPE_METHOD(tpl, "outer", _outer);
	NODE_SET_PROTOTYPE_METHOD(tpl, "inner", _inner);
	NODE_SET_PROTOTYPE_METHOD(tpl, "cosine", _cosine);
	NODE_SET_PROTOTYPE_METHOD(tpl, "plus", _plus);
	NODE_SET_PROTOTYPE_METHOD(tpl, "minus", _minus);
	NODE_SET_PROTOTYPE_METHOD(tpl, "multiply", _multiply);
	NODE_SET_PROTOTYPE_METHOD(tpl, "normalize", _normalize);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toString", _toString);
	NODE_SET_PROTOTYPE_METHOD(tpl, "diag", _diag);
	NODE_SET_PROTOTYPE_METHOD(tpl, "spDiag", _spDiag);
	NODE_SET_PROTOTYPE_METHOD(tpl, "norm", _norm);
	NODE_SET_PROTOTYPE_METHOD(tpl, "sparse", _sparse);
	NODE_SET_PROTOTYPE_METHOD(tpl, "toMat", _toMat);
	NODE_SET_PROTOTYPE_METHOD(tpl, "save", _save);
	NODE_SET_PROTOTYPE_METHOD(tpl, "load", _load);
	NODE_SET_PROTOTYPE_METHOD(tpl, "saveascii", _saveascii);
	NODE_SET_PROTOTYPE_METHOD(tpl, "loadascii", _loadascii);

	// Properties 
	tpl->InstanceTemplate()->SetIndexedPropertyHandler(_indexGet, _indexSet);
	tpl->InstanceTemplate()->SetAccessor(v8::String::NewFromUtf8(Isolate, "length"), _length);

	// This has to be last, otherwise the properties won't show up on the
	// object in JavaScript.
	constructor.Reset(Isolate, tpl->GetFunction());
#ifndef MODULE_INCLUDE_LA
	exports->Set(v8::String::NewFromUtf8(Isolate, Name.CStr()),
		tpl->GetFunction());
#endif
}

// Returns i = arg max_i v[i] for a vector v 
template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::getMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	double MxVal = JsVec->Vec[0];
	int MxIdx = 0;
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
		const double CrrVal = JsVec->Vec[ElN];
		if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
	}
	Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

template <>
inline void TNodeJsVec<TInt, TAuxIntV>::getMaxIdx(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TInt, TAuxIntV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args.Holder());

	double MxVal = JsVec->Vec[0];
	int MxIdx = 0;
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
		const double CrrVal = JsVec->Vec[ElN];
		if (CrrVal > MxVal) { MxIdx = ElN; MxVal = CrrVal; }
	}
	Args.GetReturnValue().Set(v8::Integer::New(Isolate, MxIdx));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::sortPerm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	const bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TFltV SortedV;
	TIntV PermV;
	TVec<TFlt>::SortGetPerm(JsVec->Vec, SortedV, PermV, Asc);
	v8::Local<v8::Object> Obj = v8::Object::New(Isolate);
	Obj->Set(v8::String::NewFromUtf8(Isolate, "vec"), New(SortedV));
	Obj->Set(v8::String::NewFromUtf8(Isolate, "perm"), TNodeJsVec<TInt, TAuxIntV>::New(PermV));
	Args.GetReturnValue().Set(Obj);
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::outer(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args.Holder());

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a vector on the input");

	TFltVV ResMat;
	TNodeJsVec<TFlt, TAuxFltV>* JsArgVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args[0]->ToObject());
	ResMat.Gen(JsVec->Vec.Len(), JsArgVec->Vec.Len());

	TLinAlg::OuterProduct(JsVec->Vec, JsArgVec->Vec, ResMat);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(ResMat));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::inner(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args.Holder());
	double Result = 0.0;
	if (Args[0]->IsObject()) {
		TNodeJsVec<TFlt, TAuxFltV>* OthVec =
			ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
		Result = TLinAlg::DotProduct(OthVec->Vec, JsVec->Vec);
	}

	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::cosine(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV>>(Args.Holder());
	double Result = 0.0;
	if (Args[0]->IsObject()) {
		TNodeJsVec<TFlt, TAuxFltV>* OthVec =
			ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
		Result = TLinAlg::DotProduct(OthVec->Vec, JsVec->Vec);
		Result /= (TLinAlg::Norm(JsVec->Vec)* TLinAlg::Norm(OthVec->Vec));
	}
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template <>
inline void TNodeJsVec<TFlt, TAuxFltV>::plus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected an TNodeJsVec object (a vector)");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	TNodeJsVec<TFlt, TAuxFltV>* OthVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
	TFltV Result(JsVec->Vec.Len());
	TLinAlg::LinComb(1.0, JsVec->Vec, 1.0, OthVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::minus(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected an TNodeJsVec object (a vector)");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	TNodeJsVec<TFlt, TAuxFltV>* OthVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
	TFltV Result; Result.Gen(JsVec->Vec.Len());
	TLinAlg::LinComb(1.0, JsVec->Vec, -1.0, OthVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::multiply(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsNumber(),
		"Expected number");

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());
	const double Scalar = Args[0]->NumberValue();

	TFltV Result;
	Result.Gen(JsVec->Vec.Len());
	TLinAlg::MultiplyScalar(Scalar, JsVec->Vec, Result);

	Args.GetReturnValue().Set(New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::normalize(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	EAssertR(JsVec->Vec.Len() > 0, "Can't normalize vector of length 0.");
	if (JsVec->Vec.Len() > 0) {
		TLinAlg::Normalize(JsVec->Vec);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::diag(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TFltVV Result;
	// computation
	TLAMisc::Diag(JsVec->Vec, Result);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::spDiag(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.Holder());

	TVec<TIntFltKdV> Result;
	// computation
	TLAMisc::Diag(JsVec->Vec, Result);

	Args.GetReturnValue().Set(TNodeJsSpMat::New(Result, JsVec->Vec.Len()));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::norm(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());
	const double Result = TLinAlg::Norm(JsVec->Vec);
	Args.GetReturnValue().Set(v8::Number::New(Isolate, Result));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::sparse(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());

	TIntFltKdV Res;
	TLAMisc::ToSpVec(JsVec->Vec, Res);

	Args.GetReturnValue().Set(TNodeJsSpVec::New(Res, JsVec->Vec.Len()));
}

template<>
inline void TNodeJsVec<TFlt, TAuxFltV>::toMat(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TFlt, TAuxFltV>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args.This());

	TFltVV Res(JsVec->Vec, JsVec->Vec.Len(), 1);

	Args.GetReturnValue().Set(TNodeJsFltVV::New(Res));
}

//////

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::New(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	EAssertR(!constructor.IsEmpty(), "TNodeJsVec<TVal, TAux>::New: constructor is empty. Did you call TNodeJsVec<TVal, TAux>::Init(exports); in this module's init function? Vector ClassId: " + TAux::ClassId);
	if (Args.IsConstructCall()) {
		//printf("vector construct call, class = %s, nargs: %d\n", TAux::ClassId.CStr(), Args.Length());
		TNodeJsVec<TVal, TAux>* JsVec = new TNodeJsVec<TVal, TAux>();

		v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
		v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAux::ClassId.CStr());
		v8::Local<v8::Object> Instance = Args.This();

		// If we got Javascript array on the input: vector.new([1,2,3]) 
		if (Args[0]->IsArray()) {
			//printf("vector construct call, class = %s, input array\n", TAux::ClassId.CStr());
			v8::Handle<v8::Array> Arr = v8::Handle<v8::Array>::Cast(Args[0]);
			const int Len = Arr->Length();
			for (int ElN = 0; ElN < Len; ++ElN) { JsVec->Vec.Add(TAux::CastVal(Arr->Get(ElN))); }
		}
		else if (Args[0]->IsObject()) {
			if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsFltV::GetClassId())) {
				//printf("vector construct call, class = %s, input TFltV\n", TAux::ClassId.CStr());
				TNodeJsVec<TFlt, TAuxFltV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsIntV::GetClassId())) {
				//printf("vector construct call, class = %s, input TIntV\n", TAux::ClassId.CStr());
				TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsStrV::GetClassId())) {
				//printf("vector construct call, class = %s, input TStrV\n", TAux::ClassId.CStr());
				TNodeJsVec<TStr, TAuxStrV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TStr, TAuxStrV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgClass(Args, 0, TNodeJsBoolV::GetClassId())) {
				TNodeJsBoolV* JsBoolV = ObjectWrap::Unwrap<TNodeJsBoolV>(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsBoolV->Vec));
				return;
			}
			else {
				//printf("construct call, else branch, class = %s\n", TAux::ClassId.CStr());
				// We have object with parameters, parse them out
				const int MxVals = TNodeJsUtil::GetArgInt32(Args, 0, "mxVals", -1);
				const int Vals = TNodeJsUtil::GetArgInt32(Args, 0, "vals", 0);
				EAssertR(Vals >= 0, "vals should be nonnegative");
				if (MxVals >= 0) {
					JsVec->Vec.Gen(MxVals, Vals);
				}
				else { JsVec->Vec.Gen(Vals); }
			}
		} // else return an empty vector 

		Instance->SetHiddenValue(Key, Value);
		JsVec->Wrap(Instance);
		Args.GetReturnValue().Set(Instance);
	}
	else {
		//printf("vector NOT construct call, class = %s\n", TAux::ClassId.CStr());
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

// Returns an element at index idx=Args[0]; assert 0 <= idx < v.length() 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::at(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	EAssertR(Args.Length() >= 1 && Args[0]->IsInt32(), "Expected integer.");
	const int Idx = Args[0]->IntegerValue();

	EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds.");
	Args.GetReturnValue().Set(TAux::GetObjVal(JsVec->Vec[Idx]));
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
			TVec<TVal> ResultVec(Len);
			for (int ElN = 0; ElN < Len; ++ElN) {
				EAssertR(Array->Get(ElN)->IsInt32(),
					"Expected array to contain integers only.");
				const int Idx = Array->Get(ElN)->Int32Value();
				EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(),
					"One of the indices from the index vector is out of bounds");
				ResultVec[ElN] = JsVec->Vec[Idx];
			}
			Args.GetReturnValue().Set(TNodeJsVec<TVal, TAux>::New(ResultVec));
			return;
		}
		else if (Args[0]->IsObject() && TNodeJsUtil::IsArgClass(Args, 0, "TIntV")) {
			TNodeJsVec<TInt, TAuxIntV>* IdxV = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
			const int Len = IdxV->Vec.Len();
			TVec<TVal> ResultVec(Len);
			for (int ElN = 0; ElN < Len; ElN++) {
				EAssertR(IdxV->Vec[ElN] >= 0 && IdxV->Vec[ElN] < JsVec->Vec.Len(),
					"One of the indices from the index vector is out of bounds");
				ResultVec[ElN] = JsVec->Vec[IdxV->Vec[ElN]];
			}
			Args.GetReturnValue().Set(TNodeJsVec<TVal, TAux>::New(ResultVec));
			return;
		}
		else {
			Args.GetReturnValue().Set(v8::Undefined(Isolate));
			throw TExcept::New("Expected array or vector of indices.");
		}
	}
	else {
		Args.GetReturnValue().Set(v8::Undefined(Isolate));
		throw TExcept::New("No arguments.");
	}
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::indexGet(uint32_t Index, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Info.Holder());
	EAssertR(Index < static_cast<uint32_t>(JsVec->Vec.Len()), "Index out of bounds.");
	Info.GetReturnValue().Set(TAux::GetObjVal(JsVec->Vec[Index]));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::indexSet(uint32_t Index, v8::Local<v8::Value> Value, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Info.Holder());
	EAssertR(Index < static_cast<uint32_t>(JsVec->Vec.Len()), "Index out of bounds.");
	JsVec->Vec[Index] = TAux::CastVal(Value);
	Info.GetReturnValue().Set(v8::Undefined(Isolate));
}

// Returns the sum of the vectors elements (only make sense for numeric values) 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sum(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TVal Sum;
	for (int ElN = 0; ElN < JsVec->Vec.Len(); ++ElN) {
		Sum += JsVec->Vec[ElN];
	}
	Args.GetReturnValue().Set(TAux::GetObjVal(Sum));
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

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	const int Idx = Args[0]->IntegerValue();

	EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds");

	JsVec->Vec[Idx] = TAux::CastVal(Args[1]);

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

// Appends an element to the vector 
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::push(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	if (Args.Length() < 1) {
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected 1 argument, 0 given.")));
	}
	else if (!Args[0]->IsNumber() && !Args[0]->IsString() && !Args[0]->IsBoolean()) {
		// TODO: int vector should not silently pass and truncate non-integer values!
		Isolate->ThrowException(v8::Exception::TypeError(
			v8::String::NewFromUtf8(Isolate, "Expected number, string or boolean")));
	}
	else {
		JsVec->Vec.Add(TAux::CastVal(Args[0]));
		Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.Len()));
	}
}

// Appends an element to the vector
template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::splice(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 2, "vec.splice expects at least 2 arguments!");

	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TVec<TVal>& Vec = JsVec->Vec;

	// from the Javascript documentation:
	// If greater than the length of the array, actual starting index will be set to the length of the array.
	// If negative, will begin that many elements from the end
	int StartIdx = TMath::Mn(TNodeJsUtil::GetArgInt32(Args, 0), Vec.Len());
	if (StartIdx < 0) StartIdx += Vec.Len();

	const int DelCount = TMath::Mn(TNodeJsUtil::GetArgInt32(Args, 1), Vec.Len() - StartIdx);	// mimic javascript default behavior
	const int InsCount = Args.Length() - 2;

	EAssert(StartIdx + DelCount <= Vec.Len());

	const int NOverride = TMath::Mn(DelCount, InsCount);
	const int NDel = TMath::Mx(DelCount - InsCount, 0);
	const int NIns = TMath::Mx(InsCount - DelCount, 0);

	// override
	for (int i = 0; i < NOverride; i++) {
		Vec[StartIdx + i] = TAux::CastVal(Args[2 + i]);
	}

	// insert
	for (int i = 0; i < NIns; i++) {
		const int Idx = StartIdx + NOverride + i;
		if (Idx == Vec.Len()) {
			Vec.Add(TAux::CastVal(Args[2 + NOverride + i]));
		}
		else {
			Vec.Ins(StartIdx + NOverride + i, TAux::CastVal(Args[2 + NOverride + i]));
		}
	}

	// delete
	if (NDel > 0) {
		Vec.Del(StartIdx + NOverride, StartIdx + NOverride + NDel - 1);
	}

	Args.GetReturnValue().Set(Args.Holder());
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::unshift(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);
	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	// assume number
	TVal Val = TAux::CastVal(Args[0]);
	JsVec->Vec.Ins(0, Val);
	Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.Len()));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::pushV(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a vector on the input");

	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TNodeJsVec<TVal, TAux>* OthVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args[0]->ToObject());

	JsVec->Vec.AddV(OthVec->Vec);

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	const bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);

	JsVec->Vec.Sort(Asc);
	Args.GetReturnValue().Set(Args.Holder());
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::shuffle(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	static TRnd Rnd;
	JsVec->Vec.Shuffle(Rnd);

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 1 && Args[0]->IsInt32() &&
		Args[0]->IntegerValue() >= 0, "Expected a nonnegative integer");

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	const int NewLen = Args[0]->IntegerValue();
	JsVec->Vec.Trunc(NewLen);

	Args.GetReturnValue().Set(v8::Boolean::New(Isolate, true));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::toString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	TStr Str = "[";
	for (int ElN = 0; ElN < JsVec->Vec.Len() - 1; ++ElN) {
		Str += JsVec->Vec[ElN].GetStr() + ", ";
	}
	if (JsVec->Vec.Len() > 0) {
		Str += JsVec->Vec.Last().GetStr();
	}
	Str += "]";

	Args.GetReturnValue().Set(v8::String::NewFromUtf8(Isolate, Str.CStr()));
}

// Returns the size of the vector 
template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::length(v8::Local<v8::String> Name, const v8::PropertyCallbackInfo<v8::Value>& Info) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	v8::Local<v8::Object> Self = Info.Holder();
	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Self);

	Info.GetReturnValue().Set(v8::Integer::New(Isolate, JsVec->Vec.Len()));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::save(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a TNodeJsFOut object");
	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
	PSOut SOut = JsFOut->SOut;
	JsVec->Vec.Save(*SOut);

	Args.GetReturnValue().Set(Args[0]);
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::load(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a TNodeJsFIn object");
	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
	PSIn SIn = JsFIn->SIn;
	JsVec->Vec.Load(*SIn);

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::saveascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a TNodeJsFOut object");
	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TNodeJsFOut* JsFOut = ObjectWrap::Unwrap<TNodeJsFOut>(Args[0]->ToObject());
	PSOut SOut = JsFOut->SOut;
	const int Rows = JsVec->Vec.Len();
	for (int RowId = 0; RowId < Rows; RowId++) {
		SOut->PutStr(JsVec->Vec[RowId].GetStr());
		SOut->PutCh('\n');
	}

	Args.GetReturnValue().Set(Args[0]);
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::loadascii(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() == 1 && Args[0]->IsObject(),
		"Expected a TNodeJsFIn object");
	TNodeJsVec<TVal, TAux>* JsVec = ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	TNodeJsFIn* JsFIn = ObjectWrap::Unwrap<TNodeJsFIn>(Args[0]->ToObject());
	PSIn SIn = JsFIn->SIn;
	TStr Line;
	while (SIn->GetNextLn(Line)) {
		JsVec->Vec.Add(TAux::Parse(Line));
	}

	Args.GetReturnValue().Set(v8::Undefined(Isolate));
}

#endif


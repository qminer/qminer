/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
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
	static const TStr ClassId; //ClassId is set to TNodeJsFltV::GetClassId().CStr()
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
	static void AssertType(const v8::Local<v8::Value>& Val) {
		EAssertR(Val->IsNumber(), ClassId + "::AssertType: Value expected to be a number");
	}
	static TFlt Parse(const TStr& Str) {
		return Str.GetFlt();
	}
};

class TAuxIntV {
public:
	static const TStr ClassId; //ClassId is set to TNodeJsIntV::GetClassId().CStr()
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
	static void AssertType(const v8::Local<v8::Value>& Val) {
		EAssertR(Val->IsInt32(), ClassId + "::AssertType: Value expected to be an integer");
	}
	static TInt Parse(const TStr& Str) {
		return Str.GetInt();
	}
};

class TAuxStrV {
public:
	static const TStr ClassId; //ClassId is set to TNodeJsStrV::GetClassId().CStr()
	static v8::Handle<v8::Value> GetObjVal(const TStr& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return HandleScope.Escape(v8::String::NewFromUtf8(Isolate, Val.CStr()));
	}
	static TStr CastVal(const v8::Local<v8::Value>& Value) {
		v8::String::Utf8Value Utf8(Value);
		return TStr(*Utf8);
	}
	static void AssertType(const v8::Local<v8::Value>& Val) {
		EAssertR(Val->IsString(), ClassId + "::AssertType: Value expected to be a String");		
	}
	static TStr Parse(const TStr& Str) {
		return Str;
	}
};

class TAuxBoolV {
public:
	static const TStr ClassId; //ClassId is set to TNodeJsBoolV::GetClassId().CStr()
	static v8::Handle<v8::Value> GetObjVal(const TBool& Val) {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::EscapableHandleScope HandleScope(Isolate);
		return v8::Boolean::New(Isolate, Val);
	}
	static bool CastVal(const v8::Local<v8::Value>& Value) {
		return Value->BooleanValue();
	}
	static void AssertType(const v8::Local<v8::Value>& Val) {
		EAssertR(Val->IsBoolean(), ClassId + "::AssertType: Value expected to be a boolean");
	}
	static TBool Parse(const TStr& Str) {
		return TBool::GetValFromStr(Str);
	}
};

template <class TVal = TFlt, class TAux = TAuxFltV>
class TJsVecComparator {
private:	
	// Callbacks
	v8::Persistent<v8::Function> Callback;
public:
	~TJsVecComparator(){
		Callback.Reset();
	}
	TJsVecComparator(v8::Handle<v8::Function> _Callback) { 	Callback.Reset(v8::Isolate::GetCurrent(), _Callback);}
	bool operator()(const TVal& Val1, const TVal& Val2) const {
		v8::Isolate* Isolate = v8::Isolate::GetCurrent();
		v8::HandleScope HandleScope(Isolate);
		
		// prepare arguments
		v8::Local<v8::Value> Arg1 = TAux::GetObjVal(Val1);
		v8::Local<v8::Value> Arg2 = TAux::GetObjVal(Val2);

		v8::Local<v8::Function> Callbck = v8::Local<v8::Function>::New(Isolate, Callback);
		v8::Local<v8::Object> GlobalContext = Isolate->GetCurrentContext()->Global();
		const unsigned Argc = 2;
		v8::Local<v8::Value> ArgV[Argc] = { Arg1, Arg2 };
		v8::TryCatch TryCatch;
		v8::Local<v8::Value> ReturnVal = Callbck->Call(GlobalContext, Argc, ArgV);
		TNodeJsUtil::CheckJSExcept(TryCatch);

		EAssertR(ReturnVal->IsBoolean() || ReturnVal->IsNumber(), "Comparator callback must return a boolean or a number!");
		return ReturnVal->IsBoolean() ? ReturnVal->BooleanValue() : ReturnVal->NumberValue() < 0;
	}
};


///////////////////////////////
// NodeJs-Linalg-Vector

/**
* <% title %>
* @classdesc The <% elementType %> vector representation. Wraps a C++ array.
* @class
* @param {(Array.<<% elementType %>> | module:la.<% className %>)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. using an array of vector elements. Example: using `<% example1 %>` creates a vector of length 3,
* <br>2. using a vector (copy constructor).
* @example
* var la = require('qminer').la;
* // create a new empty vector
* var vec = new la.<% className %>();
* // create a new vector
* var vec2 = new la.<% className %>(<% example1 %>);
*/
//# exports.<% className %> = function() { return Object.create(require('qminer').la.<% className %>.prototype); }
template <class TVal = TFlt, class TAux = TAuxFltV>
class TNodeJsVec : public node::ObjectWrap {
	friend class TNodeJsUtil;
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
	TNodeJsVec(const int& Size) : Vec(Size) {}
	TNodeJsVec(const TVec<TVal>& ValV) : Vec(ValV) { }
public:
	JsDeclareFunction(New);
private:
	
	
	//# var <% className %>DefaultVal = <% defaultVal %>; // for intellisense

	/**
	* Returns element at index.
	* @param {number} index - Element index (zero-based).
	* @returns {<% elementType %>} Vector element.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // get the element at index 1
    * var el = vec[1];
	*/
	//# exports.<% className %>.prototype.at = function(number) { return <% className %>DefaultVal; }
	JsDeclareFunction(at);
	
	/**
	* Returns a subvector.
	* @param {(Array.<number> | module:la.IntVector)} arg - Index array or vector. Indices can repeat (zero based).
	* @returns {module:la.<% className %>} Subvector, where the i-th element is the `arg[i]`-th element of the instance.
    * @<% skipSubVec %>example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // get the subvector of the first two elements
    * var subvec = vec.subVec([0, 1]);
	*/
	//# <% skipSubVec %>exports.<% className %>.prototype.subVec = function (arg) { return Object.create(this); }
	JsDeclareFunction(subVec);
	
	//!- `num = vec[idx]; vec[idx] = num` -- get value `num` at index `idx`, set value at index `idx` to `num` of vector `vec`(0-based indexing)
	JsDeclareSetIndexedProperty(indexGet, indexSet);
		
	/**
	* Sets an element in vector.
	* @param {number} idx - Index (zero based).
	* @param {<% elementType %>} val - Element value.
	* @returns {module:la.<% className %>} Self. The values at index `idx` has been changed to `val`.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // set the first element to <% val1 %>
    * vec.put(0, <% val1 %>);
	*/
	//# exports.<% className %>.prototype.put = function (idx, val) { return this;}
	JsDeclareFunction(put);
		
	/**
	* Adds an element to the end of the vector.
	* @param {<% elementType %>} val - The element added to the vector.
	* @returns {number} The new length property of the object upon which the method was called.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // push an element to the vector
    * vec.push(<% val1 %>);
	*/
	//# exports.<% className %>.prototype.push = function (val) { return 0; }
	JsDeclareFunction(push);
		
	/**
	* Changes the vector by removing and adding elements.
	* @param {number} start - Index at which to start changing the array.
	* @param {number} deleteCount - Number of elements to be removed.
	* @param {...number} [itemN] - The element(s) to be add to the array. If no elements are given, splice() will only remove elements from the array.
	* @returns {module:la.<% className %>} Self. The selected elements are removed/replaced.
	* @example
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // splice the vector by removing the last two elements and adding <% input1 %>
	* vec.splice(1, 2, <% input1 %>)// returns vector <% output2 %>
	*/
	//# exports.<% className %>.prototype.splice = function (start, deleteCount, itemN) { return this; }
	JsDeclareFunction(splice);

	/**
	* Adds elements to the beginning of the vector.
	* @param {...<% elementType %>} args - One or more elements to be added to the vector.
	* @returns {number} The new length of vector.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // add two elements to the beggining of the vector
    * var len = vec.unshift(<% input1 %>); // returns 5
    */
	//# exports.<% className %>.prototype.unshift = function (args) { return 0; }
	JsDeclareFunction(unshift);

	/**
	* Appends a second vector to the first one.
	* @param {module:la.<% className %>} vec - The appended vector.
	* @returns {number} The new length property of the vectors.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create two new vectors
    * var vec = new la.<% className %>(<% example1 %>);
    * var vec2 = new la.<% className %>([<% input1 %>]);
    * // append the two vectors
    * vec.pushV(vec2);
	*/
	//# exports.<% className %>.prototype.pushV = function (vec) { return 0; }
	JsDeclareFunction(pushV);

	/** 
	* Sums the elements in the vector.
	* @returns {number} The sum of all elements in the instance.
    * @example
    * // import la modules
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // sum all the elements of the vector
    * var sum = vec.sum();
	*/
	//# <% skipSum %>exports.<% className %>.prototype.sum = function () { return <% className %>DefaultVal; }
	JsDeclareFunction(sum);

	/**
	* Gets the index of the maximal element.
	* @returns {number} Index of the maximal element in the vector.
    * // import la modules
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% example1 %>);
    * // get the index of the maximum value
    * var idx = vec.getMaxIdx();
    * 
	*/
	//# <% skipGetMaxIdx %>exports.<% className %>.prototype.getMaxIdx = function () { return 0; }
	JsDeclareSpecializedFunction(getMaxIdx);

	/**
	* Vector sort comparator callback.
	* @callback <% sortCallback %>
	* @param {<% elementType %>} arg1 - First argument.
	* @param {<% elementType %>} arg2 - Second argument.
	* @returns {(number | boolean)} If `<% sortCallback %>(arg1, arg2)` is less than 0 or false, sort `arg1` to a lower index than `arg2`, i.e. `arg1` comes first.
	*/
	
	/**
	* Sorts the vector (in place operation).
	* @param {(module:la~<% sortCallback %> | boolean)} [arg] - Sort callback or a boolean ascend flag. Default is boolean and true.
	* @returns {module:la.<% className %>} Self.
	* <br>1. Vector sorted in ascending order, if `arg` is boolean and true.  
	* <br>2. Vector sorted in descending order, if `arg` is boolean and false.
	* <br>3. Vector sorted by using the comparator callback, if `arg` is a {@link module:la~<% sortCallback %>}.
	* @example
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% exampleSort %>);
	* // sort ascending
	* vec.sort(); // sorts to: <% outputSortAsc %>
	* // sort using callback
	* vec.sort(<% inputSort %>); // sorts to: <% outputSort %>
	*/
	//# <% skipSort %>exports.<% className %>.prototype.sort = function (bool) { return this; } 
	JsDeclareFunction(sort);

	/**
	* Sorts the vector and returns the sorted vector as well as the permutation.
	* @param {boolean} [asc = true] - Sort in ascending order flag.
	* @returns {Object} The object `<% className %>SortResult` containing the properties:
    * <br> `<% className %>SortResult.vec` - The sorted vector,
    * <br> `<% className %>SortResult.perm` - Permutation vector, where `<% className %>SortResult.vec[i] = instanceVector[<% className %>SortResult.perm[i]]`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% exampleSort %>);
	* // sort ascending
	* var result = vec.sortPerm();
    * result.vec;  // <% outputSortAsc %>
	* result.perm; // permutation index vector
	*/
	//# <% skipSort %>exports.<% className %>.prototype.sortPerm = function (asc) { return {vec: Object.create(this), perm: Object.create(require('qminer').la.IntVector.prototype) }; } 
	JsDeclareSpecializedFunction(sortPerm);

	/**
	* Randomly reorders the elements of the vector (inplace).
	* @returns {module:la.<% className %>} Self. The elements are randomly reordered.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.<% className %>(<% exampleSort %>); 
    * // shuffle the elements
    * vec.shuffle();
	*/
	//# exports.<% className %>.prototype.shuffle = function () { return this; }
	JsDeclareFunction(shuffle);

	/**
	* Deletes elements with sprecific index or more.
	* @param {<% elementType %>} idx - Index (zero based).
	* @returns {module:la.<% className %>} Self after truncating.
	* @example
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // trunc all elements with index 1 or more
	* vec.trunc(1); // returns vector <% output3 %>
	*/
	//# exports.<% className %>.prototype.trunc = function (idx) { return this; } 
	JsDeclareFunction(trunc);
	
	/**
	* Creates a dense matrix A by multiplying two vectors x and y: `A = x * y^T`.
	* @param {module:la.<% className %>} vec - Second vector.
	* @returns {module:la.Matrix} Matrix obtained by the outer product of the instance and second vector.
	* @example
	* var la = require('qminer').la;
	* // create two new vectors
	* var x = new la.<% className %>([1, 2, 3]);
	* var y = new la.<% className %>([4, 5]);
	* // create the outer product of these vectors
	* var A = x.outer(y); // creates the dense matrix [[4, 5], [8, 10], [12, 15]]
	*/
	//# <% skipOuter %>exports.<% className %>.prototype.outer = function (vec) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareSpecializedFunction(outer);
	
	/**
	* Computes the inner product.
	* @param {module:la.Vector} vec - Other vector.
	* @returns {number} Inner product between the instance and the other vector.
    * @example
	* var la = require('qminer').la;
	* // create two new vectors
	* var x = new la.Vector([1, 2, 3]);
	* var y = new la.Vector([4, 5, -1]);
    * // get the inner product of the two vectors
    * var prod = x.inner(y); // returns 11
	*/
	//# <% skipInner %>exports.Vector.prototype.inner = function(vec) { return 0; }
	JsDeclareSpecializedFunction(inner);

	/**
	* Returns the cosine between the two vectors.
	* @param {module:la.Vector} vec - Second vector.
	* @returns {number} The cosine between the two vectors.
	* @example
	* var la = require('qminer').la;
	* // create two new vectors
	* var x = new la.Vector([1, 0]);
	* var y = new la.Vector([0, 1]);
	* // calculate the cosine between those two vectors
	* var num = x.cosine(y); // returns 0
	*/
	//# <% skipCosine %>exports.Vector.prototype.cosine = function (vec) { return 0.0; }
	JsDeclareSpecializedFunction(cosine);

	/**
	* Vector addition.
	* @param {module:la.Vector} vec - Second vector.
	* @returns {module:la.Vector} Sum of the instance and the second vector.
    * @example
	* var la = require('qminer').la;
	* // create two new vectors
	* var x = new la.Vector([1, 2, 3]);
	* var y = new la.Vector([4, 5, -1]);
    * // sum the vectors
    * var z = x.plus(y);
	*/
	//# <% skipPlus %>exports.Vector.prototype.plus = function (vec) { return Object.create(this); }
	JsDeclareSpecializedFunction(plus);

	/**
	* Vector substraction.
	* @param {module:la.Vector} vec - Second vector.
	* @returns {module:la.Vector} The difference of the instance and the other vector.
    * @example
	* var la = require('qminer').la;
	* // create two new vectors
	* var x = new la.Vector([1, 2, 3]);
	* var y = new la.Vector([4, 5, -1]);
    * // substract the vectors
    * var z = x.minus(y);
	*/
	//# <% skipMinus %>exports.Vector.prototype.minus = function (vec) { return Object.create(this); }
	JsDeclareSpecializedFunction(minus);

	/**
	* Multiplies the vector with a scalar.
	* @param {number} val - Scalar.
	* @returns {module:la.Vector} Product of the vector and scalar.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var x = new la.Vector([4, 5, -1]);
    * // multiply the vector with the scalar 3
    * var y = x.multiply(3);
	*/
	//# <% skipMultiply %>exports.Vector.prototype.multiply = function (val) { return Object.create(this); }
	JsDeclareSpecializedFunction(multiply);

	/**
	* Normalizes vector.
	* @returns {module:la.Vector} Self. The vector is normalized.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var x = new la.Vector([4, 5, -1]); 
    * // normalize the vector
    * x.normalize();
	*/
	//# <% skipNormalize %>exports.Vector.prototype.normalize = function () { return this; } 
	JsDeclareSpecializedFunction(normalize);

	/**
	* Gives the length of vector. Type `number`.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var x = new la.<% className %>(<% example1 %>);
    * // get the length of the vector
    * var len = x.length; // returns 3
	*/
	//# exports.<% className %>.prototype.length = 0;
	JsDeclareProperty(length);

	/**
	* Returns the vector as string.
	* @returns {string} String representation.
	* @example
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // create vector as string
	* vec.toString(); // returns <% output1 %>
	*/
	//# exports.<% className %>.prototype.toString = function () { return ''; }
	JsDeclareFunction(toString);

	/**
	* Creates a dense diagonal matrix out of the vector.
	* @returns{module:la.Matrix} Diagonal matrix, where the (i, i)-th element is the i-th element of vector.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([4, 5, -1]);
    * // create a dense matrix with the diagonal equal to vec
    * var mat = vec.diag();
	*/
	//# <% skipDiag %>exports.Vector.prototype.diag = function () { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareSpecializedFunction(diag);

	/**
	* Creates a sparse diagonal matrix out of the vector.
	* @returns {module:la.SparseMatrix} Diagonal matrix, where the (i, i)-th element is the i-th element of vector.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([4, 5, -1]);
    * // create a sparse matrix with the diagonal equal to vec
    * var mat = vec.spDiag();
	*/
	//# <% skipSpDiag %>exports.<% className %>.prototype.spDiag = function () { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareSpecializedFunction(spDiag);

	/**
	* Calculates the norm of the vector.
	* @returns {number} The norm of the vector.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([4, 5, -1]);
    * // get the norm of the vector
    * var norm = vec.norm();
	*/
	//# <% skipNorm %>exports.Vector.prototype.norm = function () { return 0.0; }
	JsDeclareSpecializedFunction(norm);

	/**
	* Creates the sparse vector representation of the vector.
	* @returns {module:la.SparseVector} The sparse vector representation.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([4, 5, -1]);
    * // create the sparse representation of the vector
    * var spVec = vec.sparse();
	*/
	//# <% skipSparse %>exports.Vector.prototype.sparse = function () { return Object.create(require('qminer').la.SparseVector.prototype); }
	JsDeclareSpecializedFunction(sparse);

	/**
	* Creates a matrix with a single column that is equal to the vector.
	* @returns {module:la.Matrix} The matrix with a single column that is equal to the instance.
    * @example
    * var la = require('qminer').la;
    * // create a new vector
    * var vec = new la.Vector([4, 5, -1]);
    * // create a matrix representation of the vector
    * var mat = vec.toMat();
	*/
	//# <% skipToMat %>exports.Vector.prototype.toMat = function () { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareSpecializedFunction(toMat);

	/**
	* Saves the vector as output stream (binary serialization).
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} The output stream `fout`.
	* @example
	* // import fs module
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // open write stream
	* var fout = fs.openWrite('vec.dat');
	* // save vector and close write stream
	* vec.save(fout).close();
	*/
	//# <% skipSave %>exports.<% className %>.prototype.save = function (fout) {  return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);

	/**
	* Loads the vector from input stream (binary deserialization).
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.<% className %>} Self. The vector is filled using the input stream `fin`.
	* @example
	* // import fs module
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create an empty vector
	* var vec = new la.<% className %>();
	* // open a read stream
	* var fin = fs.openRead('vec.dat');
	* // load the vector
	* vec.load(fin);
	*/
	//# <% skipLoad %>exports.<% className %>.prototype.load = function (fin) { return this; }
	JsDeclareFunction(load);

	/**
	* Saves the vector as output stream (ascii serialization).
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} The output stream `fout`.
	* @example
	* // import fs module
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create a new vector
	* var vec = new la.<% className %>(<% example1 %>);
	* // open write stream
	* var fout = fs.openWrite('vec.dat');
	* // save matrix and close write stream
	* vec.saveascii(fout).close();
	*/
	//# <% skipSave %>exports.<% className %>.prototype.saveascii = function (fout) {  return Object.create(require('qminer').fs.FOut.prototype); }

	JsDeclareFunction(saveascii);
	
	/**
	* Loads the vector from input stream (ascii deserialization).
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.<% className %>} Self. The vector is filled using the input stream `fin`.
	* @example
	* // import fs module
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create an empty vector
	* var vec = new la.<% className %>();
	* // open a read stream
	* var fin = fs.openRead('vec.dat');
	* // load the matrix
	* vec.loadascii(fin);
	*/
	//# <% skipLoad %>exports.<% className %>.prototype.loadascii = function (fin) { return this; }
	JsDeclareFunction(loadascii);
public:
	TVec<TVal> Vec;
private:
	static v8::Persistent<v8::Function> Constructor;
};


///////////////////////////////
// NodeJs-GLib-TVec Implementations
template <typename TVal, typename TAux>
v8::Persistent<v8::Function> TNodeJsVec<TVal, TAux>::Constructor;

typedef TNodeJsVec<TFlt, TAuxFltV> TNodeJsFltV;
typedef TNodeJsVec<TInt, TAuxIntV> TNodeJsIntV;
typedef TNodeJsVec<TStr, TAuxStrV> TNodeJsStrV;
typedef TNodeJsVec<TBool, TAuxBoolV> TNodeJsBoolV;


// template <typename TVal, typename TAux>
template <>
inline v8::Local<v8::Object> TNodeJsVec<TFlt, TAuxFltV>::New(const TFltV& FltV) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::EscapableHandleScope HandleScope(Isolate);
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAuxFltV::ClassId.CStr());
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
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TFlt, TAuxFltV>::New: constructor is empty. Did you call TNodeJsVec<TFlt, TAuxFltV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAuxFltV::ClassId.CStr());
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
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);
	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAuxIntV::ClassId.CStr());
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
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TInt, TAuxIntV>::New: constructor is empty. Did you call TNodeJsVec<TInt, TAuxIntV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);

	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAuxIntV::ClassId.CStr());
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
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TStr, TAuxStrV>::New: constructor is empty. Did you call TNodeJsVec<TStr, TAuxStrV>::Init(exports); in this module's init function?");
	v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);

	v8::Local<v8::Object> Instance = cons->NewInstance();

	v8::Handle<v8::String> Key = v8::String::NewFromUtf8(Isolate, "class");
	v8::Handle<v8::String> Value = v8::String::NewFromUtf8(Isolate, TAuxStrV::ClassId.CStr());
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

	TStr Name = TAux::ClassId;

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
	Constructor.Reset(Isolate, tpl->GetFunction());
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
	TLinAlgTransform::Diag(JsVec->Vec, Result);

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
	TLinAlgTransform::Diag(JsVec->Vec, Result);

	Args.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsSpMat>(new TNodeJsSpMat(Result, JsVec->Vec.Len())));
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
	
	int Dim = TNodeJsUtil::GetArgInt32(Args, 0, JsVec->Vec.Len());
    TIntFltKdV Res;
    TLinAlgTransform::ToSpVec(JsVec->Vec, Res);

	Args.GetReturnValue().Set(
		TNodeJsUtil::NewInstance<TNodeJsSpVec>(new TNodeJsSpVec(Res, Dim)));
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
	EAssertR(!Constructor.IsEmpty(), "TNodeJsVec<TVal, TAux>::New: constructor is empty. Did you call TNodeJsVec<TVal, TAux>::Init(exports); in this module's init function? Vector ClassId: " + TAux::ClassId);
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
			if (TNodeJsUtil::IsArgWrapObj<TNodeJsFltV>(Args, 0)) {
				//printf("vector construct call, class = %s, input TFltV\n", TAux::ClassId.CStr());
				TNodeJsVec<TFlt, TAuxFltV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TFlt, TAuxFltV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 0)) {
				//printf("vector construct call, class = %s, input TIntV\n", TAux::ClassId.CStr());
				TNodeJsVec<TInt, TAuxIntV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TInt, TAuxIntV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgWrapObj<TNodeJsStrV>(Args, 0)) {
				//printf("vector construct call, class = %s, input TStrV\n", TAux::ClassId.CStr());
				TNodeJsVec<TStr, TAuxStrV>* JsVecArg = ObjectWrap::Unwrap<TNodeJsVec<TStr, TAuxStrV> >(Args[0]->ToObject());
				Args.GetReturnValue().Set(New(JsVecArg->Vec));
				return;
			}
			else if (TNodeJsUtil::IsArgWrapObj<TNodeJsBoolV>(Args, 0)) {
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
		v8::Local<v8::Function> cons = v8::Local<v8::Function>::New(Isolate, Constructor);
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
	const int Idx = Args[0]->Int32Value();

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
		else if (TNodeJsUtil::IsArgWrapObj<TNodeJsIntV>(Args, 0)) {
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
	TAux::AssertType(Args[1]);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	const int Idx = Args[0]->Int32Value();

	EAssertR(Idx >= 0 && Idx < JsVec->Vec.Len(), "Index out of bounds");

	JsVec->Vec[Idx] = TAux::CastVal(Args[1]);

	Args.GetReturnValue().Set(Args.Holder());
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

	TVec<TVal> Temp = TVec<TVal>(Args.Length());
	for (int ArgN = 0; ArgN < Args.Length(); ArgN++) {
		// assume number
		TAux::AssertType(Args[ArgN]);
		Temp[ArgN] = TAux::CastVal(Args[ArgN]);
	}
	Temp.AddV(JsVec->Vec);
	JsVec->Vec = Temp;
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

	Args.GetReturnValue().Set(v8::Number::New(Isolate, JsVec->Vec.Len()));
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::sort(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	if (Args.Length() == 1 && Args[0]->IsFunction()) {
		v8::Local<v8::Function> Callback = v8::Local<v8::Function>::Cast(Args[0]);				
		JsVec->Vec.SortCmp(TJsVecComparator<TVal, TAux>(Callback));
	} else {
		const bool Asc = TNodeJsUtil::GetArgBool(Args, 0, true);
		JsVec->Vec.Sort(Asc);
	}
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

	Args.GetReturnValue().Set(Args.Holder());
}

template <typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::trunc(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	EAssertR(Args.Length() >= 1 && Args[0]->IsInt32() &&
		Args[0]->Int32Value() >= 0, "Expected a nonnegative integer");

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());
	const int NewLen = Args[0]->Int32Value();
	JsVec->Vec.Trunc(NewLen);

	Args.GetReturnValue().Set(Args.Holder());
}

template<typename TVal, typename TAux>
void TNodeJsVec<TVal, TAux>::toString(const v8::FunctionCallbackInfo<v8::Value>& Args) {
	v8::Isolate* Isolate = v8::Isolate::GetCurrent();
	v8::HandleScope HandleScope(Isolate);

	TNodeJsVec<TVal, TAux>* JsVec =
		ObjectWrap::Unwrap<TNodeJsVec<TVal, TAux> >(Args.Holder());

	TStr Str = "";
	for (int ElN = 0; ElN < JsVec->Vec.Len() - 1; ++ElN) {
		Str += JsVec->Vec[ElN].GetStr() + ", ";
	}
	if (JsVec->Vec.Len() > 0) {
		Str += JsVec->Vec.Last().GetStr();
	}	

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


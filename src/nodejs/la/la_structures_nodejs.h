#ifndef QMINER_LA_STRUCTURES_NODEJS_H
#define QMINER_LA_STRUCTURES_NODEJS_H


#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"

///////////////////////////////
// NodeJs-Qminer-LinAlg
//! 
//! ## Linear Algebra
//! 
//! A global object `la` is used to construct vectors (sparse, dense) and matrices and 
//! it is available in any script. The object includes
//! several functions from linear algebra.
//! 

/**
* Linear algebra module.
* @module la
* @example
* // import module, create a random matrix and a vector, multiply. find svd of the matrix
*/


class TNodeJsLinAlg : public node::ObjectWrap {
public:
    static void Init(v8::Handle<v8::Object> exports);
    //! 
    //! **Functions and properties:**
    //! 
    ////!- `vec = la.newVec()` -- generate an empty float vector
    ////!- `vec = la.newVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////!- `vec = la.newVec(arr)` -- copy a javascript number array `arr` 
    ////!- `vec = la.newVec(vec2)` -- clone a float vector `vec2`
    //JsDeclareFunction(newVec);
    ////!- `intVec = la.newIntVec()` -- generate an empty integer vector
    ////!- `intVec = la.newIntVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////!- `intVec = la.newIntVec(arr)` -- copy a javascript int array `arr` 
    ////!- `intVec = la.newIntVec(vec2)` -- clone an int vector `vec2`
    //JsDeclareFunction(newIntVec);
    ////!- `strVec = la.newStrVec()` -- generate an empty integer vector
    ////!- `strVec = la.newStrVec({"vals":num, "mxvals":num2})` -- generate a vector with `num` zeros and reserve additional `num - num2` elements 
    ////!- `strVec = la.newStrVec(arr)` -- copy a javascript int array `arr` 
    ////!- `strVec = la.newStrVec(strVec2)` -- clone an str vector `strVec2`
    //JsDeclareFunction(newStrVec);
    ////!- `mat = la.newMat()` -- generates a 0x0 matrix
    ////!- `mat = la.newMat({"rows":num, "cols":num2, "random":bool})` -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
    ////!- `mat = la.newMat(nestedArr)` -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
    ////!- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`
    //JsDeclareFunction(newMat);
    ////!- `spVec = la.newSpVec(len)` -- creates an empty sparse vector `spVec`, where `len` is an optional (-1 by default) integer parameter that sets the dimension
    ////!- `spVec = la.newSpVec(nestedArr, len)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `len` is optional and sets the dimension
    //JsDeclareFunction(newSpVec);
    ////!- `spMat = la.newSpMat()` -- creates an empty sparse matrix `spMat`
    ////!- `spMat = la.newSpMat(rowIdxVec, colIdxVec, valVec)` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec`
    ////!- `spMat = la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
    ////!- `spMat = la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
    //JsDeclareFunction(newSpMat);
    ////!- `svdRes = la.svd(mat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition mat ~ U S V^T.  `mat` is a dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
    ////!- `svdRes = la.svd(spMat, k, {"iter":num, "tol":num2})` -- Computes a truncated svd decomposition spMat ~ U S V^T.  `spMat` is a sparse or dense matrix, integer `k` is the number of singular vectors, optional parameter JSON object contains properies `iter` (integer number of iterations `num`, default 100) and `tol` (the tolerance number `num2`, default 1e-6). The outpus are stored as two dense matrices: `svdRes.U`, `svdRes.V` and a dense float vector `svdRes.s`.
    JsDeclareFunction(svd);
    //!- `qrRes = la.qr(mat, tol)` -- Computes a qr decomposition: mat = Q R.  `mat` is a dense matrix, optional parameter `tol` (the tolerance number, default 1e-6). The outpus are stored as two dense matrices: `qrRes.Q`, `qrRes.R`.
    JsDeclareFunction(qr);
    //!- `intVec = la.loadIntVeC(fin)` -- load integer vector from input stream `fin`.
    // (TODO) JsDeclareFunction(loadIntVec);
    //!JSIMPLEMENT:src/qminer/linalg.js
};

///////////////////////////////
// NodeJs-Linalg-FltVV
//! 
//! ### Matrix (dense matrix)
//! 
//! Matrix is a double 2D array implemented in glib/base/ds.h. 
//! Using the global `la` object, dense matrices are generated in several ways:
//! 
//! ```JavaScript
//! var fltv = la.newVec(); //empty matrix
//! // refer to la.newMat function for alternative ways to generate dense matrices
//! ```
//! 


////!- `mat = la.newMat()` -- generates a 0x0 matrix
////!- `mat = la.newMat({"rows":num, "cols":num2, "random":bool})` -- creates a matrix with `num` rows and `num2` columns and sets it to zero if the optional "random" property is set to `false` (default) and uniform random if "random" is `true`
////!- `mat = la.newMat(nestedArr)` -- generates a matrix from a javascript array `nestedArr`, whose elements are arrays of numbers which correspond to matrix rows (row-major dense matrix)
////!- `mat = la.newMat(mat2)` -- clones a dense matrix `mat2`


/**
* Matrix constructor parameter object
* @typedef {Object} matrixArg
* @property  {number} matrixArg.rows - Number of rows.
* @property  {number} matrixArg.cols - Number of columns.
* @property  {boolean} [matrixArg.random=false] - Generate a random matrix with entries sampled from a uniform [0,1] distribution. If set to false, a zero matrix is created.
*/

/**
* Matrix
* @classdesc Represents a dense matrix (2d array)
* @class
* @param {(module:la~matrixArg | Array<Array<number>> | module:la.Matrix)} [arg] - Constructor arguments. There are three ways of constructing.
* <br>1. Parameter object module:la~matrixArg. 
* <br>2. Nested array of matrix elements (row major). Example: [[1,2],[3,4]] has two rows, the first row is [1,2].
* <br>3. A matrix (copy constructor)
* @example
* // TODO
*/
//# exports.Matrix = function(arg) {}	

class TNodeJsFltVV : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;

public:
	static void Init(v8::Handle<v8::Object> exports);
	const static TStr ClassId;

	static TNodeJsFltVV* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	static v8::Local<v8::Object> New(const TFltVV& FltVV);
	static v8::Local<v8::Object> New(const TFltV& FltV);
public:
	TNodeJsFltVV() { }
	TNodeJsFltVV(const TFltVV& _Mat) : Mat(_Mat) { }
public:
	JsDeclareFunction(New);

private:
	//! 
	//! **Functions and properties:**
	//! 
	//!- `num = mat.at(rowIdx,colIdx)` -- Gets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.


	/**
	* Returns an element of matrix
	* @param {number} rowIdx - row index (zero based)
	* @param {number} colIdx - column index (zero based)
	* @returns {number} Matrix element
	*/
	//# exports.Matrix.prototype.at = function(rowIdx, colIdx) {}
	JsDeclareFunction(at);
	//!- `mat = mat.put(rowIdx, colIdx, num)` -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//!- `mat2 = mat.multiply(num)` -- Matrix multiplication: `num` is a number, `mat2` is a matrix
	//!- `vec2 = mat.multiply(vec)` -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
	//!- `vec = mat.multiply(spVec)` -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
	//!- `mat3 = mat.multiply(mat2)` -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
	//!- `mat2 = mat.multiply(spMat)` -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
	JsDeclareFunction(multiply);
	//!- `mat2 = mat.multiplyT(num)` -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `vec2 = mat.multiplyT(vec)` -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `vec = mat.multiplyT(spVec)` -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `mat3 = mat.multiplyT(mat2)` -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `mat2 = mat.multiplyT(spMat)` -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	JsDeclareFunction(multiplyT);
	//!- `mat3 = mat.plus(mat2)` -- `mat3` is the sum of matrices `mat` and `mat2`
	JsDeclareFunction(plus);
	//!- `mat3 = mat.minus(mat2)` -- `mat3` is the difference of matrices `mat` and `mat2`
	JsDeclareFunction(minus);
	//!- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat` transposed
	JsDeclareFunction(transpose);
	//!- `vec2 = mat.solve(vec)` -- vector `vec2` is the solution to the linear system `mat * vec2 = vec`
	JsDeclareFunction(solve);
	//!- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	JsDeclareFunction(rowNorms);
	//!- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	JsDeclareFunction(colNorms);
	//!- `mat = mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//!- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	JsDeclareFunction(toString);
	//!- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	JsDeclareFunction(sparse);
	//!- `num = mat.frob()` -- number `num` is the Frobenious norm of matrix `mat`
	JsDeclareFunction(frob);
	//!- `num = mat.rows` -- integer `num` corresponds to the number of rows of `mat`
	JsDeclareProperty(rows);
	//!- `num = mat.cols` -- integer `num` corresponds to the number of columns of `mat`
	JsDeclareProperty(cols);
	//!- `colIdx = mat.rowMaxIdx(rowIdx)`: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
	JsDeclareFunction(rowMaxIdx);
	//!- `rowIdx = mat.colMaxIdx(colIdx)`: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
	JsDeclareFunction(colMaxIdx);
	//!- `vec = mat.getCol(colIdx)` -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
	JsDeclareFunction(getCol);
	//!- `mat = mat.setCol(colIdx, vec)` -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
	JsDeclareFunction(setCol);
	//!- `vec = mat.getRow(rowIdx)` -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
	JsDeclareFunction(getRow);
	//!- `mat.setRow(rowIdx, vec)` -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
	JsDeclareFunction(setRow);
	//!- `vec = mat.diag()` -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
	JsDeclareFunction(diag);
	//!- `fout = mat.save(fout)` -- print `mat` (full matrix) to output stream `fout`. Returns `fout`.
	JsDeclareFunction(save);
	//!- `mat = mat.load(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(load);
	//!- `fout = mat.saveascii(fout)` -- save `mat` (full matrix) to output stream `fout`. Returns `fout`.
	JsDeclareFunction(saveascii);
	//!- `mat = mat.loadascii(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	JsDeclareFunction(loadascii);
public:
	TFltVV Mat;
};


///////////////////////////////
// NodeJs-Linalg-Sparse-Vector
//! 
//! ### SpVector (sparse vector)
//! 
//! Sparse vector is an array of (int,double) pairs that represent column indices and values (TIntFltKdV is implemented in glib/base/ds.h.)
//! Using the global `la` object, sparse vectors can be generated in the following ways:
//! 
//! ```JavaScript
//! var spVec = la.newSpVec(); //empty vector
//! // refer to la.newSpVec for alternative ways to generate sparse vectors
//! ```
//! 
class TNodeJsSpVec : public node::ObjectWrap {
public:
	const static TStr ClassId;

	TNodeJsSpVec() : Dim(-1) { }
	TNodeJsSpVec(const TIntFltKdV& IntFltKdV, const int& Dim = -1)
		: Vec(IntFltKdV), Dim(Dim)
	{ }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TIntFltKdV& IntFltKdV, const int& Dim = -1);
public:
	//! 
	//! **Functions and properties:**
	//! 
	//!- `spVec = la.newSpVec(dim)` -- creates an empty sparse vector `spVec`, where `dim` is an optional (-1 by default) integer parameter that sets the dimension
	//!- `spVec = la.newSpVec(nestedArr, dim)` -- creats a sparse vector `spVec` from a javascript array `nestedArr`, whose elements are javascript arrays with two elements (integer row index and double value). `dim` is optional and sets the dimension
	JsDeclareFunction(New);
	//!- `num = spVec.at(idx)` -- Gets the element of a sparse vector `spVec`. Input: index `idx` (integer). Output: value `num` (number). Uses 0-based indexing
	JsDeclareFunction(at);
	//!- `spVec = spVec.put(idx, num)` -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
	JsDeclareFunction(put);
	//!- `num = spVec.sum()` -- `num` is the sum of elements of `spVec`
	JsDeclareFunction(sum);
	//!- `num = spVec.inner(vec)` -- `num` is the inner product between `spVec` and dense vector `vec`.
	//!- `num = spVec.inner(spVec)` -- `num` is the inner product between `spVec` and sparse vector `spVec`.
	JsDeclareFunction(inner);
	//!- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
	JsDeclareFunction(multiply);
	//!- `spVec = spVec.normalize()` -- normalizes the vector spVec (inplace operation). Returns self.
	JsDeclareFunction(normalize);
	//!- `num = spVec.nnz` -- gets the number of nonzero elements `num` of vector `spVec`
	JsDeclareProperty(nnz);
	//!- `num = spVec.dim` -- gets the dimension `num` (-1 means that it is unknown)
	JsDeclareProperty(dim);
	// #- `num = spVec.norm()` -- returns `num` - the norm of `spVec`
	JsDeclareFunction(norm);
	//!- `vec = spVec.full()` --  returns `vec` - a dense vector representation of sparse vector `spVec`.
	JsDeclareFunction(full);
	//!- `valVec = spVec.valVec()` --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
	JsDeclareFunction(valVec);
	//!- `idxVec = spVec.idxVec()` --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
	JsDeclareFunction(idxVec);
	//!- `spVec = spVec.print()` -- returns the vector as string. 
	JsDeclareFunction(toString);
public:
	TIntFltKdV Vec;
	TInt Dim;
private:
	static v8::Persistent<v8::Function> constructor;
};

///////////////////////////////
// NodeJs-Linalg-Sparse-Col-Matrix
//! 
//! ### SpMatrix (sparse column matrix)
//! 
//! SpMatrix is a sparse matrix represented as a dense vector of sparse vectors which correspond to matrix columns (TVec<TIntFltKdV>, implemented in glib/base/ds.h.)
//! Using the global `la` object, sparse matrices are generated in several ways:
//! 
//! ```JavaScript
//! var spMat = la.newSpMat(); //empty matrix
//! // refer to la.newSpMat function for alternative ways to generate sparse matrices
//! ```
//! 
class TNodeJsSpMat : public node::ObjectWrap {
public:
	const static TStr ClassId;

	TNodeJsSpMat() : Rows(-1) { }
	TNodeJsSpMat(const TVec<TIntFltKdV>& _Mat, const int& _Rows = -1)
		: Mat(_Mat), Rows(_Rows) { }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static v8::Local<v8::Object> New(const TVec<TIntFltKdV>& Mat, const int& Rows = -1);
public:
	//! 
	//! **Functions and properties:**
	//! 

	//!- `spMat = new la.newSpMat()` -- creates an empty sparse matrix `spMat`
	//!- `spMat = new la.newSpMat(rowIdxVec, colIdxVec, valVec [, rows, cols])` -- creates an sparse matrix based on two int vectors `rowIdxVec` (row indices) and `colIdxVec` (column indices) and float vector of values `valVec` and optionally sets the row and column dimension
	//!- `spMat = new la.newSpMat(doubleNestedArr, rows)` -- creates an sparse matrix with `rows` rows (optional parameter), where `doubleNestedArr` is a javascript array of arrays that correspond to sparse matrix columns and each column is a javascript array of arrays corresponding to nonzero elements. Each element is an array of size 2, where the first number is an int (row index) and the second value is a number (value). Example: `spMat = linalg.newSpMat([[[0, 1.1], [1, 2.2], [3, 3.3]], [[2, 1.2]]], { "rows": 4 });`
	//!- `spMat = new la.newSpMat({"rows":num, "cols":num2})` -- creates a sparse matrix with `num` rows and `num2` columns, which should be integers
	JsDeclareFunction(New);
	//!- `num = spMat.at(rowIdx,colIdx)` -- Gets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer). Output: `num` (number). Uses zero-based indexing.
	JsDeclareFunction(at);
	//!- `spMat = spMat.put(rowIdx, colIdx, num)` -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	JsDeclareFunction(put);
	//!- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	JsDeclareSetIndexedProperty(indexGet, indexSet);

	JsDeclareFunction(indexGet);
	//!- `spVec = spMat.getCol(colIdx)` -- `spVec` corresponds to the `colIdx`-th column of a sparse matrix `spMat`. `colIdx` must be an integer.
	JsDeclareFunction(indexSet);
	//!- `spMat = spMat.setCol(colIdx, spVec)` -- Sets the column of a sparse matrix `spMat`. `colIdx` must be an integer, `spVec` must be a sparse vector. Returns self.

	//!- `spMat = spMat.push(spVec)` -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
	JsDeclareFunction(push);
	//!- `spMat2 = spMat.multiply(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
	//!- `vec2 = spMat.multiply(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
	//!- `vec = spMat.multiply(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
	//!- `mat2 = spMat.multiply(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
	//!- `mat = spMat.multiply(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
	JsDeclareFunction(multiply);
	//!- `spMat2 = spMat.multiplyT(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `vec2 = spMat.multiplyT(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `vec = spMat.multiplyT(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `mat2 = spMat.multiplyT(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `mat = spMat.multiplyT(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
	JsDeclareFunction(multiplyT);
	//!- `spMat3 = spMat.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(plus);
	//!- `spMat3 = spMat.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	JsDeclareFunction(minus);
	//!- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed 
	JsDeclareFunction(transpose);
	//!- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	JsDeclareFunction(colNorms);
	//!- `spMat = spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
	JsDeclareFunction(normalizeCols);
	//!- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	JsDeclareFunction(full);
	//!- `num = spMat.frob()` -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
	JsDeclareFunction(frob);
	//!- `num = spMat.rows` -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
	JsDeclareProperty(rows);
	//!- `num = spMat.cols` -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
	JsDeclareProperty(cols);
	//!- `spMat = spMat.print()` -- print `spMat` (sparse matrix) to console. Returns self.
	JsDeclareFunction(print);
	//!- `spMat = spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`. Returns self.
	JsDeclareFunction(save);
	//!- `spMat = spMat.load(fin)` -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
	JsDeclareFunction(load);
	//!- `spMat2 = spMat.sign()` -- create a new sparse matrix `spMat2` whose elements are sign function applied to elements of `spMat`.
	// (TODO) JsDeclareFunction(sign);
	//!JSIMPLEMENT:src/qminer/spMat.js
public:
	TVec<TIntFltKdV> Mat;
	TInt Rows;
private:
	static v8::Persistent<v8::Function> constructor;
};

#endif


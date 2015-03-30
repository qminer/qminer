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
* @classdesc Represents a dense matrix (2d array).
* @class
* @param {(module:la~matrixArg | Array<Array<number>> | module:la.Matrix)} [arg] - Constructor arguments. There are three ways of constructing:
* <br>1. Parameter object {@link module:la~matrixArg}.
* <br>2. Nested array of matrix elements (row major). Example: [[1,2],[3,4]] has two rows, the first row is [1,2].
* <br>3. A matrix (copy constructor).
* @example
* // create new matrix with matrixArg
* var mat = new la.Matrix({"rows": 3, "cols": 2, "random": true}); // creates a 3 x 2 matrix with random values
* // create a new matrix with nested arrays
* var mat2 = new la.Matrix([[1, 7, 4], [-10, 0, 3]]); // creates a 2 x 3 matrix
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
	* Returns an element of matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} Matrix element.
	*/
	//# exports.Matrix.prototype.at = function(rowIdx, colIdx) {}
	JsDeclareFunction(at);

	//!- `mat = mat.put(rowIdx, colIdx, num)` -- Sets the element of `mat` (matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	/**
	* Sets an element or a block of matrix.
	* @param {number} rowIdx - Row index (zero based). 
	* @param {number} colIdx - Column index (zero based).
	* @param {(number | module:la.Matrix)} arg - A number or a matrix. If the arg is a matrix, then it gets copied, where the argument's upper left corner, arg.at(0,0), gets copied to (rowIdx, colIdx)
	* @returns {module:la.Matrix} Self.
	* @example
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
	* var arg = new la.Matrix([[10, 11], [12, 13]]);
	* mat.put(0, 1, arg);
	* // updates the matrix to
    * // 1  10 11
    * // 4  12 13
    * // 7  8  9   
	*/
	//# exports.Matrix.prototype.put = function(rowIdx, colIdx, arg) {}
	JsDeclareFunction(put);

	//!- `mat2 = mat.multiply(num)` -- Matrix multiplication: `num` is a number, `mat2` is a matrix
	//!- `vec2 = mat.multiply(vec)` -- Matrix multiplication: `vec` is a vector, `vec2` is a vector
	//!- `vec = mat.multiply(spVec)` -- Matrix multiplication: `spVec` is a sparse vector, `vec` is a vector
	//!- `mat3 = mat.multiply(mat2)` -- Matrix multiplication: `mat2` is a matrix, `mat3` is a matrix
	//!- `mat2 = mat.multiply(spMat)` -- Matrix multiplication: `spMat` is a sparse matrix, `mat2` is a matrix
	/**
	* Right-hand side multiplication of matrix with parameter.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Matrix | module:la.Vector)}
	* <br>1. {@link module:la.Matrix}, if arg is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if arg is {@link module:la.Vector} or {@link module:la.SparseVector}.
	* @example
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* // create a new vector
	* var vec = new la.Vector([1, -1]);
	* //multiply mat and vec
	* var vec2 = mat.multiply(vec); // returns vector [-1, -6]
	*/
	//# exports.Matrix.prototype.multiply = function(arg) {}
	JsDeclareFunction(multiply);

	//!- `mat2 = mat.multiplyT(num)` -- Matrix transposed multiplication: `num` is a number, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `vec2 = mat.multiplyT(vec)` -- Matrix transposed multiplication: `vec` is a vector, `vec2` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `vec = mat.multiplyT(spVec)` -- Matrix transposed multiplication: `spVec` is a sparse vector, `vec` is a vector. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `mat3 = mat.multiplyT(mat2)` -- Matrix transposed multiplication: `mat2` is a matrix, `mat3` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient
	//!- `mat2 = mat.multiplyT(spMat)` -- Matrix transposed multiplication: `spMat` is a sparse matrix, `mat2` is a matrix. The result is numerically equivalent to mat.transpose().multiply(), but more efficient

	/**
	* Matrix transpose and right-hand side multiplication of matrix with parameter.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Matrix | module:la.Vector)}
	* <br>1. {@link module:la.Matrix}, if arg is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if arg is {@link module:la.Vector} or {@link module:la.SparseVector}.
	* @example
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* // create a new vector
	* var vec = new la.Vector([1, -1]);
	* //multiply mat and vec
	* var vec2 = mat.multiplyT(vec); // returns vector [2, 7]
	*/
	//# exports.Matrix.prototype.multiplyT = function(arg) {}
	JsDeclareFunction(multiplyT);

	//!- `mat3 = mat.plus(mat2)` -- `mat3` is the sum of matrices `mat` and `mat2`
	/**
	* Addition of two matrices.
	* @param {module:la.Matrix} mat - Second matrix.
	* @returns {module:la.Matrix} The sum of matrices.
	*/
	//# exports.Matrix.prototype.plus = function(mat2) {}
	JsDeclareFunction(plus);

	//!- `mat3 = mat.minus(mat2)` -- `mat3` is the difference of matrices `mat` and `mat2`
	/**
	* Substraction of two matrices.
	* @param {module:la.Matrix} mat - Second matrix.
	* @returns {module:la.Matrix} The difference of matrices.
	*/
	//# exports.Matrix.prototype.minus = function(mat2) {}
	JsDeclareFunction(minus);

	//!- `mat2 = mat.transpose()` -- matrix `mat2` is matrix `mat` transposed
	/**
	* Transposes matrix.
	* @returns {module:la.Matrix} Transposed matrix.
	*/
	//# exports.Matrix.prototype.transpose = function() {}
	JsDeclareFunction(transpose);

	//!- `vec2 = mat.solve(vec)` -- vector `vec2` is the solution to the linear system mat * vec2 = vec
	/**
	* Solves the linear system.
	* @param {module:la.Vector} vec - The right-hand side of the equation.
	* @returns {module:la.Vector} Solution of the linear system.
	* @example
	* // create a new matrix
	* var M = new la.Matrix([[1, 2], [-1, -5]]);
	* // create a new vector
	* var b = new la.Vector([-1, -6]);
	* // solve the linear system M*x = b
	* var x = M.solve(b); // returns vector [1, -1]
	*/
	//# exports.Matrix.prototype.solve = function (vec) {}
	JsDeclareFunction(solve);

	//!- `vec = mat.rowNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th row of `mat`
	/**
	* Returns a vector of row norms.
	* @returns {module:la.Vector} Vector, where the value at i-th index is the norm of the i-th row of matrix.
	*/
	//# exports.Matrix.prototype.rowNorms = function () {}
	JsDeclareFunction(rowNorms);


	//!- `vec = mat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `mat`
	/**
	* Returns a vector of column norms.
	* @returns {module:la.Vector} Vector, where the value at i-th index is the norm of the i-th column of matrix.
	*/
	//# exports.Matrix.prototype.colNorms = function () {}
	JsDeclareFunction(colNorms);

	//!- `mat = mat.normalizeCols()` -- normalizes each column of matrix `mat` (inplace operation). Returns self.
	/**
	* Normalizes each column of matrix
	* @returns {module:la.Matrix} Self.
	*/
	//# exports.Matrix.prototype.normalizeCols = function () {}
	JsDeclareFunction(normalizeCols);

	//!- `str = mat.printStr()` -- print matrix `mat` to a string `str`
	/**
	* Returns the matrix as string.
	* @returns {string} Dense matrix as string.
	* @example
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [3, 5]]);
	* // get matrix as string
	* var text = mat.toString(); // returns `1 2 \n3 5 \n\n`
	*/
	//# exports.Matrix.prototype.toString = function () {}
	JsDeclareFunction(toString);

	//!- `spMat = mat.sparse()` -- get sparse column matrix representation `spMat` of dense matrix `mat`
	/**
	* Transforms the matrix from dense to sparse format.
	* @returns {module:la.SparseMatrix} Sparse column matrix representation of dense matrix.
	*/
	//# exports.Matrix.prototype.sparse = function () {}
	JsDeclareFunction(sparse);

	//!- `num = mat.frob()` -- number `num` is the Frobenious norm of matrix `mat`
	/**
	* Returns the frobenious norm of matrix.
	* @returns {number} Frobenious norm of matrix.
	*/
	//# exports.Matrix.prototype.frob = function () {}
	JsDeclareFunction(frob);

	//!- `num = mat.rows` -- integer `num` corresponds to the number of rows of `mat`
	/**
	* Returns the number of rows of matrix.
	* @returns {number} Number of rows in matrix.
	*/
	//# exports.Matrix.prototype.rows = undefined
	JsDeclareProperty(rows);

	//!- `num = mat.cols` -- integer `num` corresponds to the number of columns of `mat`
	/**
	* Returns the number of columns of matrix.
	* @returns {number} Number of columns in matrix.
	*/
	//# exports.Matrix.prototype.cols = undefined
	JsDeclareProperty(cols);

	//!- `colIdx = mat.rowMaxIdx(rowIdx)`: get the index `colIdx` of the maximum element in row `rowIdx` of dense matrix `mat`
	/**
	* Index of maximum element in given row.
	* @param {number} rowIdx - Row index (zero based).
	* @returns {number} Column index (zero based) of the maximum value in rowIdx-th row of matrix.
	*/
	//# exports.Matrix.prototype.rowMaxIdx = function (rowIdx) {}
	JsDeclareFunction(rowMaxIdx);

	//!- `rowIdx = mat.colMaxIdx(colIdx)`: get the index `rowIdx` of the maximum element in column `colIdx` of dense matrix `mat`
	/**
	* Returns index of maximum element in given column.
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} Row index (zero based) of the maximum value in colIdx-th column of matrix.
	*/
	//# exports.Matrix.prototype.colMaxIdx = function (colIdx) {}
	JsDeclareFunction(colMaxIdx);

	//!- `vec = mat.getCol(colIdx)` -- `vec` corresponds to the `colIdx`-th column of dense matrix `mat`. `colIdx` must be an integer.
	/**
	* Returns the corresponding column of matrix as vector.
	* @param {number} colIdx - Column index (zero based).
	* @returns {module:la.Vector} The colIdx-th column of matrix.
	*/
	//# exports.Matrix.prototype.getCol = function (colIdx) {}
	JsDeclareFunction(getCol);

	//!- `mat = mat.setCol(colIdx, vec)` -- Sets the column of a dense matrix `mat`. `colIdx` must be an integer, `vec` must be a dense vector. Returns self.
	/**
	* Sets the column of matrix.
	* @param {number} colIdx - Column index (zero based).
	* @param {module:la.Vector} vec - The new column of matrix.
	* @returns {module:la.Matrix} Self.
	*/
	//# exports.Matrix.prototype.setCol = function (colIdx, vec) {}
	JsDeclareFunction(setCol);

	//!- `vec = mat.getRow(rowIdx)` -- `vec` corresponds to the `rowIdx`-th row of dense matrix `mat`. `rowIdx` must be an integer.
	/**
	* Returns the corresponding row of matrix as vector.
	* @param {number} rowIdx - Row index (zero based).
	* @returns {module:la.Vector} The rowIdx-th row of matrix.
	*/
	//# exports.Matrix.prototype.getRow = function (rowIdx) {}
	JsDeclareFunction(getRow);

	//!- `mat.setRow(rowIdx, vec)` -- Sets the row of a dense matrix `mat`. `rowIdx` must be an integer, `vec` must be a dense vector.
	/**
	* Sets the row of matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {module:la.Vector} vec - The new row of matrix.
	* @returns {module:la.Matrix} Self.
	*/
	//# exports.Matrix.prototype.setRow = function (rowIdx, vec) {}
	JsDeclareFunction(setRow);

	//!- `vec = mat.diag()` -- Returns the diagonal of matrix `mat` as `vec` (dense vector).
	/**
	* Returns the diagonal elements of matrix.
	* @returns {module:la.Vector} Vector containing the diagonal elements.
	* @example
	* // create a new matrix
	* var mat = new la.Matrix([[1, -1, 0], [15, 8, 3], [0, 1, 0]]);
	* // call diag function
	* var vec = mat.diag(); // returns a vector [1, 8, 0]
	*/
	//# exports.Matrix.prototype.diag = function () {}
	JsDeclareFunction(diag);

	//!- `fout = mat.save(fout)` -- print `mat` (full matrix) to output stream `fout`. Returns `fout`.
	/**
	* Saves the matrix as output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} fout
	* @example
	* // import fs module
	* var fs = require('fs.js');
	* // create new matrix
	* var mat = new la.Matrix([[1, 2], [3, 4]]);
	* // open write stream
	* var fout = fs.openWrite('mat.dat');
	* // save matrix and close write stream
	* mat.save(fout).close();
	*/
	//# exports.Matrix.prototype.save = function (fout) {}
	JsDeclareFunction(save);

	//!- `mat = mat.load(fin)` -- replace `mat` (full matrix) by loading from input steam `fin`. `mat` has to be initialized first, for example using `mat = la.newMat()`. Returns self.
	/**
	* Loads the matrix from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.Matrix} Self.
	* @example
	* // import fs module
	* var fs = require('fs.js');
	* // create an empty matrix
	* var mat = new la.Matrix();
	* // open a read stream
	* var fin = fs.openRead('mat.dat');
	* // load the matrix
	* mat.load(fin);
	*/
	//# exports.Matrix.prototype.load = function (FIn) {}
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

/**
* Sparse Vector
* @classdesc Represents a sparse vector.
* @class
* @param {(Array<Array<number>> | module:la.SparseVector)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. Nested array of vector elements. Example: [[0,2],[2,3]] has two nonzero values, first value is 2 at position 0, second value is 3 at position 2.
* <br>2. A sparse vector (copy constructor).
* @param {number} [dim] - Maximal length of sparse vector. It is only in combinantion with nested array of vector elements.
* @example
* // create new sparse vector with arrays
* var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]); // sparse vector [1, 0, 3, 6]
* // create new sparse vector with dim
* var spVec2 = new la.SparseVector([[0, 1], [2, 3], [3, 6]], 5); // largest index (zero based) is 4
*/
//# exports.SparseVector = function(arg, dim) {}	

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
	/**
	* Returns an element of sparse vector.
	* @param {number} idx - Index (zero based).
	* @returns {number} Sparse vector element.
	*/
	//# exports.SparseVector.prototype.at = function (idx) {}
	JsDeclareFunction(at);

	//!- `spVec = spVec.put(idx, num)` -- Set the element of a sparse vector `spVec`. Inputs: index `idx` (integer), value `num` (number). Uses 0-based indexing. Returns self.
	/**
	* Puts a new element in sparse vector.
	* @param {number} idx - Index (zero based).
	* @param {number} num - Input value.
	* @returns {module:la.SparseVector} Self.
	*/
	//# exports.SparseVector.prototype.put = function (idx, num) {}
	JsDeclareFunction(put);

	//!- `num = spVec.sum()` -- `num` is the sum of elements of `spVec`
	/**
	* Returns the sum of all values in sparse vector.
	* @returns {number} The sum of all values in sparse vector.
	*/
	//# exports.SparseVector.prototype.sum = function () {}
	JsDeclareFunction(sum);

	//!- `num = spVec.inner(vec)` -- `num` is the inner product between `spVec` and dense vector `vec`.
	//!- `num = spVec.inner(spVec)` -- `num` is the inner product between `spVec` and sparse vector `spVec`.
	/**
	* Returns the inner product of argument and sparse vector.
	* @param {(module:la.Vector | module:la.SparseVector)} arg - Inner product input. Supports dense vector and sparse vector.
	* @returns {number} The inner product.
	*/
	//# exports.SparseVector.prototype.inner = function (arg) {}
	JsDeclareFunction(inner);

	//!- `spVec2 = spVec.multiply(a)` -- `spVec2` is sparse vector, a product between `num` (number) and vector `spVec`
	/**
	* Multiplies sparse vector with scalar.
	* @param {number} num - Scalar.
	* @returns {module:la.SparseVector} Product of num and sparse vector.
	* @example
	* // create a new sparse vector
	* var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]);
	* // multiply sparse vector with scalar 3.14
	* var spVec2 = spVec.multiply(3.14); // returns sparse vector [3.14, 0, 9.42, 18.84]
	*/
	//# exports.SparseVector.prototype.multiply = function (num) {}
	JsDeclareFunction(multiply);

	//!- `spVec = spVec.normalize()` -- normalizes the vector spVec (inplace operation). Returns self.
	/**
	* Normalizes the sparse vector.
	* @returns {module:la.SparseVector} Self - Normalized.
	*/
	//# exports.SparseVector.prototype.normalize = function () {}
	JsDeclareFunction(normalize);

	//!- `num = spVec.nnz` -- gets the number of nonzero elements `num` of vector `spVec`
	/**
	* Returns the number of nonzero values.
	* @returns {number} Number of nonzero values.
	*/
	//# exports.SparseVector.prototype.nnz = undefined
	JsDeclareProperty(nnz);

	//!- `num = spVec.dim` -- gets the dimension `num` (-1 means that it is unknown)
	/**
	* Returns the dimension of sparse vector.
	* @returns {number} Dimension of sparse vector.
	*/
	//# exports.SparseVector.prototype.dim = undefined
	JsDeclareProperty(dim);

	// #- `num = spVec.norm()` -- returns `num` - the norm of `spVec`
	/**
	* Returns the norm of sparse vector.
	* @returns {number} Norm of sparse vector.
	*/
	//# exports.SparseVector.prototype.norm = function () {}
	JsDeclareFunction(norm);

	//!- `vec = spVec.full()` --  returns `vec` - a dense vector representation of sparse vector `spVec`.
	/**
	* Returns the dense vector representation of sparse vector.
	* @returns {module:la.Vector} Dense vector representation.
	*/
	//# exports.SparseVector.prototype.full = function () {}
	JsDeclareFunction(full);

	//!- `valVec = spVec.valVec()` --  returns `valVec` - a dense (double) vector of values of nonzero elements of `spVec`.
	/**
	* Returns a dense vector of values of nonzero elements of sparse vector.
	* @returns {module:la.Vector} Dense vector of values.
	*/
	//# exports.SparseVector.prototype.valVec = function () {}
	JsDeclareFunction(valVec);

	//!- `idxVec = spVec.idxVec()` --  returns `idxVec` - a dense (int) vector of indices (0-based) of nonzero elements of `spVec`.
	/**
	* Returns a dense vector of indices (zero based) of nonzero elements of sparse vector.
	* @returns {module:la.Vector} Dense vector of indeces.
	*/
	//# exports.SparseVector.prototype.idxVec = function () {}
	JsDeclareFunction(idxVec);
		
	/**
	* Returns the string representation.
	* @example
	* // create new sparse vector
	* var spVec = new la.SparseVector([[0, 1], [2, 3]]);	
	* spVec.toString(); // returns the string '[(0, 1), (2, 3)]'
	*/
	//# exports.SparseVector.prototype.toString = function () {}
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

/**
* Sparse Matrix
* @classdesc Represents a sparse matrix.
* @class
* @param {(Array<Array<Array<number>>> | module:la.SparseMatrix)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. A nested array of sparse vectors (columns). A sparse vector is a nested array of pairs, first value is index, second is value.. Example: [[[0,2]], [[0, 1], [2,3]]] has 2 columns.
* The second nonzero element in second column has a value 3 at index 2.
* <br>2. A sparse matrix (copy constructor).
* @param {number} [rows] - Maximal number of rows in sparse vector. It is only in combinantion with nested array of vector elements.
* @example
* // create a new sparse matrix with array
* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
* // create a new sparse matrix with specified max rows
* var mat2 = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]], 3);
*/
//# exports.SparseMatrix = function(arg) {}	

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
	/**
	* Returns an element of sparse matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} Matrix value.
	*/
	//# exports.SparseMatrix.prototype.at = function (rowIdx, colIdx) {}
	JsDeclareFunction(at);

	//!- `spMat = spMat.put(rowIdx, colIdx, num)` -- Sets the element of `spMat` (sparse matrix). Input: row index `rowIdx` (integer), column index `colIdx` (integer), value `num` (number). Uses zero-based indexing. Returns self.
	/**
	* Puts an element in sparse matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @param {number} num - Element value.
	* @returns {module:la.SparseMatrix} Self.
	*/
	//# exports.SparseMatrix.prototype.put = function (rowIdx, colIdx, num) {}
	JsDeclareFunction(put);

	//!- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	JsDeclareSetIndexedProperty(indexGet, indexSet);

	JsDeclareFunction(indexGet);

	//!- `spVec = spMat.getCol(colIdx)` -- `spVec` corresponds to the `colIdx`-th column of a sparse matrix `spMat`. `colIdx` must be an integer.
	/**
	* Returns the column of sparse matrix.
	* @param {number} colIdx - Column index (zero based).
	* @returns {module:la.SparseVector} Sparse vector corresponding to the colIdx-th column of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.getCol = function (colIdx) {}

	JsDeclareFunction(indexSet);
	//!- `spMat = spMat.setCol(colIdx, spVec)` -- Sets the column of a sparse matrix `spMat`. `colIdx` must be an integer, `spVec` must be a sparse vector. Returns self.
	/**
	* Sets a column in sparse matrix.
	* @param {number} colIdx - Volumn index (zero based).
	* @param {module:la.SparseVector} spVec - New column sparse vector.
	* @returns {module:la.SparseMatrix} Self.
	*/
	//# exports.SparseMatrix.prototype.setCol = function (colIdx, spVec) {}

	//!- `spMat = spMat.push(spVec)` -- attaches a column `spVec` (sparse vector) to `spMat` (sparse matrix). Returns self.
	/**
	* Attaches a column to sparse matrix.
	* @param {module:la.SparseVector} spVec - Attached column as sparse vector.
	* @returns {module:la.SparseMatrix} Self.
	*/
	//# exports.SparseMatrix.prototype.push = function (spVec) {}
	JsDeclareFunction(push);

	//!- `spMat2 = spMat.multiply(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix
	//!- `vec2 = spMat.multiply(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector
	//!- `vec = spMat.multiply(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector
	//!- `mat2 = spMat.multiply(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix
	//!- `mat = spMat.multiply(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix
	/**
	* Multiplies argument with sparse vector.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Vector | module:la.Matrix)}
	* <br>1. {@link module:la.Matrix}, if arg is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if arg is {@link module:la.Vector} or {@link module:la.SparseVector}.
	*/
	//# exports.SparseMatrix.prototype.multiply = function (arg) {}
	JsDeclareFunction(multiply);

	//!- `spMat2 = spMat.multiplyT(num)` -- Sparse matrix multiplication: `num` is a number, `spMat` is a sparse matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `vec2 = spMat.multiplyT(vec)` -- Sparse matrix multiplication: `vec` is a vector, `vec2` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `vec = spMat.multiplyT(spVec)` -- Sparse matrix multiplication: `spVec` is a sparse vector, `vec` is a dense vector. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `mat2 = spMat.multiplyT(mat)` -- Sprase matrix multiplication: `mat` is a matrix, `mat2` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient
	//!- `mat = spMat.multiplyT(spMat2)` -- Sparse matrix multiplication: `spMat2` is a sparse matrix, `mat` is a matrix. The result is numerically equivalent to spMat.transpose().multiply() but computationaly more efficient.
	/**
	* Sparse matrix transpose and multiplies with argument
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Vector | module:la.Matrix)}
	* <br>1. {@link module:la.Matrix}, if arg is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if arg is {@link module:la.Vector} or {@link module:la.SparseVector}.
	*/
	//# exports.SparseMatrix.prototype.multiplyT = function (arg) {}
	JsDeclareFunction(multiplyT);

	//!- `spMat3 = spMat.plus(spMat2)` -- `spMat3` is the sum of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	/**
	* Returns the sum of two matrices.
	* @param {module:la.SparseMatrix} mat - Second sparse matrix.
	* @returns {module:la.SparseMatrix} Sum of two sparse matrices.
	*/
	//# exports.SparseMatrix.prototype.plus = function (spMat) {}
	JsDeclareFunction(plus);

	//!- `spMat3 = spMat.minus(spMat2)` -- `spMat3` is the difference of matrices `spMat` and `spMat2` (all matrices are sparse column matrices)
	/**
	* Returns the difference of two matrices.
	* @param {module:la.SparseMatrix} mat - Second sparse matrix.
	* @returns {module:la.SparseMatrix} The difference of two sparse matrices.
	*/
	//# exports.SparseMatrix.prototype.minus = function (spMat) {}
	JsDeclareFunction(minus);

	//!- `spMat2 = spMat.transpose()` -- `spMat2` (sparse matrix) is `spMat` (sparse matrix) transposed 
	/**
	* Returns the transposed sparse matrix.
	* @returns {module:la.SparseMatrix} Transposed sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.transpose = function () {}
	JsDeclareFunction(transpose);

	//!- `vec = spMat.colNorms()` -- `vec` is a dense vector, where `vec[i]` is the norm of the `i`-th column of `spMat`
	/**
	* Returns the vector of column norms of sparse matrix.
	* @returns {module:la.Vector} Vector of column norms. Ihe i-th value of said vector is the norm of i-th column of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.colNorms = function () {}
	JsDeclareFunction(colNorms);

	//!- `spMat = spMat.normalizeCols()` -- normalizes each column of a sparse matrix `spMat` (inplace operation). Returns self.
	/**
	* Normalizes columns of sparse matrix.
	* @returns {module:la.SparseMatrix} Self - Normalized columns.
	*/
	//# exports.SparseMatrix.prototype.normalizeCols = function () {}
	JsDeclareFunction(normalizeCols);

	//!- `mat = spMat.full()` -- get dense matrix representation `mat` of `spMat (sparse column matrix)`
	/**
	* Returns the dense representation of sparse matrix.
	* @returns {module:la.Matrix} Dense representation of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.full = function () {}
	JsDeclareFunction(full);

	//!- `num = spMat.frob()` -- number `num` is the Frobenious norm of `spMat` (sparse matrix)
	/**
	* Returns the frobenious norm of sparse matrix.
	* @returns {number} Frobenious norm of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.frob = function () {}
	JsDeclareFunction(frob);

	//!- `num = spMat.rows` -- integer `num` corresponds to the number of rows of `spMat` (sparse matrix)
	/**
	* Number of rows of sparse matrix.
	* @returns {number} Number of rows of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.rows = undefined
	JsDeclareProperty(rows);

	//!- `num = spMat.cols` -- integer `num` corresponds to the number of columns of `spMat` (sparse matrix)
	/**
	* Number of columns of sparse matrix.
	* @returns {number} Number of columns of sparse matrix.
	*/
	//# exports.SparseMatrix.prototype.cols = undefined
	JsDeclareProperty(cols);

	//!- `spMat = spMat.print()` -- print `spMat` (sparse matrix) to console. Returns self.
	/**
	* Prints sparse matrix on screen.
	* @example
	* // create a new sparse matrix
	* var spMat = new la.SparseMatrix([[[0, 1]], [[0, 3], [1, 8]]]);
	* // print sparse matrix on screen
	* // each row represents a nonzero element, where first value is row index, second 
	* // value is column index and third value is element value. For this matrix:
	* // 0  0  1.000000
	* // 0  1  3.000000
	* // 1  1  8.000000
	* spMat.print(); 
	*/
	//# exports.SparseMatrix.prototype.print = function () {}
	JsDeclareFunction(print);

	//!- `spMat = spMat.save(fout)` -- print `spMat` (sparse matrix) to output stream `fout`. Returns self.
	/**
	* Saves the sparse matrix as output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} fout.
	* @example
	* // import fs module
	* var fs = require('fs.js');
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 1]], [[0, 3], [1, 12]]]);
	* // open write stream
	* var fout = fs.openWrite('mat.dat');
	* // save matrix and close write stream
	* mat.save(fout).close();
	*/
	//# exports.SparseMatrix.prototype.save = function (fout) {}
	JsDeclareFunction(save);

	//!- `spMat = spMat.load(fin)` -- replace `spMat` (sparse matrix) by loading from input steam `fin`. `spMat` has to be initialized first, for example using `spMat = la.newSpMat()`. Returns self.
	/**
	* Loads the sparse matrix from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.Matrix} Self.
	* @example
	* // import fs module
	* var fs = require('fs.js');
	* // create an empty matrix
	* var mat = new la.SparseMatrix();
	* // open a read stream
	* var fin = fs.openRead('mat.dat');
	* // load the matrix
	* mat.load(fin);
	*/
	//# exports.SparseMatrix.prototype.load = function (FIn) {}
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


/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef QMINER_LA_STRUCTURES_NODEJS_H
#define QMINER_LA_STRUCTURES_NODEJS_H

#include <node.h>
#include <node_object_wrap.h>
#include "base.h"
#include "../nodeutil.h"
#include "../fs/fs_nodejs.h"

///////////////////////////////
// NodeJs-Linalg-FltVV

/**
* Matrix constructor parameter object.
* @typedef {Object} matrixArg
* @property {number} rows - Number of rows.
* @property {number} cols - Number of columns.
* @property {boolean} [random=false] - Generate a random matrix with entries sampled from a uniform [0,1] distribution. If set to false, a zero matrix is created.
*/

/**
* Matrix class.
* @classdesc Represents a dense matrix (2d array), wraps a C++ object implemented in glib/base/ds.h.
* @class
* @param {(module:la~matrixArg | Array<Array<number>> | module:la.Matrix)} [arg] - Constructor arguments. There are three ways of constructing:
* <br>1. Using the parameter object {@link module:la~matrixArg},
* <br>2. using a nested array of matrix elements (row major). Example: [[1,2],[3,4]] has two rows, the first row is [1,2],
* <br>3. using a dense matrix (copy constructor).
* @example
* // import la module
* var la = require('qminer').la;
* // create new matrix with matrixArg
* var mat = new la.Matrix({"rows": 3, "cols": 2, "random": true}); // creates a 3 x 2 matrix with random values
* // create a new matrix with nested arrays
* var mat2 = new la.Matrix([[1, 7, 4], [-10, 0, 3]]); // creates a 2 x 3 matrix with the designated values
*/
//# exports.Matrix = function(arg) { return Object.create(require('qminer').la.Matrix.prototype); }	

class TNodeJsFltVV : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
public:
    ~TNodeJsFltVV() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
	static void Init(v8::Handle<v8::Object> exports);
	const static TStr GetClassId() { return "Matrix"; }

	static TNodeJsFltVV* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	static v8::Local<v8::Object> New(const TFltVV& FltVV);
	static v8::Local<v8::Object> New(const TFltV& FltV);
public:
	TNodeJsFltVV() { }
	TNodeJsFltVV(const TFltVV& _Mat) : Mat(_Mat) { }
private:
	/**
	* Returns an element of matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} The matrix element.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[2, 3], [-2, -2], [-3, 1]]);
	* // get the value at the index (2, 1)
	* var value = mat.at(2, 1); // returns the value 1
	*/
	//# exports.Matrix.prototype.at = function(rowIdx, colIdx) { return 0.0; }
	JsDeclareFunction(at);

	/**
	* Sets an element or a block of matrix.
	* @param {number} rowIdx - Row index (zero based). 
	* @param {number} colIdx - Column index (zero based).
	* @param {(number | module:la.Matrix)} arg - A number or a matrix. If arg is of type {@link module:la.Matrix}, it gets copied, where the argument's upper left corner, <code>arg.at(0,0)</code>, gets copied to position (<code>rowIdx</code>, <code>colIdx</code>).
	* @returns {module:la.Matrix} Self. The (<code>rowIdx</code>, <code>colIdx</code>) value/block is changed.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
	* var arg = new la.Matrix([[10, 11], [12, 13]]);
	* mat.put(0, 1, arg);
	* // updates the matrix to
    * // 1  10  11
    * // 4  12  13
    * // 7   8   9   
	*/
	//# exports.Matrix.prototype.put = function(rowIdx, colIdx, arg) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(put);

	/**
	* Right-hand side multiplication of matrix with parameter.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Matrix | module:la.Vector)}
	* <br>1. {@link module:la.Matrix}, if <code>arg</code> is a number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if <code>arg</code> is a {@link module:la.Vector} or {@link module:la.SparseVector}.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* // create a new vector
	* var vec = new la.Vector([1, -1]);
	* //multiply mat and vec
	* var vec2 = mat.multiply(vec); // returns vector [-1, -6]
	*/
	//# exports.Matrix.prototype.multiply = function(arg) { return (arg instanceof require('qminer').la.Vector | arg instanceof require('qminer').la.SparseVector) ? Object.create(require('qminer').la.Vector.prototype) : Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(multiply);

	/**
	* Matrix transpose and right-hand side multiplication of matrix with parameter.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input.
	* @returns {(module:la.Matrix | module:la.Vector)}
	* <br>1. {@link module:la.Matrix}, if <code>arg</code> is a number, {@link module:la.Matrix} or a {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if <code>arg</code> is a {@link module:la.Vector} or a {@link module:la.SparseVector}.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* // create a new vector
	* var vec = new la.Vector([1, -1]);
	* //multiply mat and vec
	* var vec2 = mat.multiplyT(vec); // returns vector [2, -3]
	*/
	//# exports.Matrix.prototype.multiplyT = function(arg) { return (arg instanceof require('qminer').la.Vector | arg instanceof require('qminer').la.SparseVector) ? Object.create(require('qminer').la.Vector.prototype) : Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(multiplyT);

	/**
	* Adds two matrices.
	* @param {module:la.Matrix} mat - The second matrix.
	* @returns {module:la.Matrix} The sum of the matrices.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create two matrices
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* var mat2 = new la.Matrix([[1, -1], [3, 2]]);
	* // add the matrices 
	* // the return matrix is
	* // 2   1
	* // 2   7
	* var sum = mat.plus(mat2);
	*/
	//# exports.Matrix.prototype.plus = function(mat2) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(plus);

	/**
	* Substracts two matrices.
	* @param {module:la.Matrix} mat - The second matrix.
	* @returns {module:la.Matrix} The difference of the matrices.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create two matrices
	* var mat = new la.Matrix([[1, 2], [-1, 5]]);
	* var mat2 = new la.Matrix([[1, -1], [3, 2]]);
	* // substract the matrices
	* // the return matrix is
	* //  0   3
	* // -4   3
	* var diff = mat.minus(mat2);
	*/
	//# exports.Matrix.prototype.minus = function(mat2) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(minus);

	/**
	* Transposes the matrix.
	* @returns {module:la.Matrix} Transposed matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a matrix
	* var mat = new la.Matrix([[2, -5], [3, 1]]);
	* // transpose the matrix
	* // the return matrix is
	* //  2   3
	* // -5   1
	* var trans = mat.transpose();
	*/
	//# exports.Matrix.prototype.transpose = function() { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(transpose);

	/**
	* Solves the linear system.
	* @param {module:la.Vector} vec - The right-hand side of the equation.
	* @returns {module:la.Vector} The solution of the linear system.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var M = new la.Matrix([[1, 2], [-1, -5]]);
	* // create a new vector
	* var b = new la.Vector([-1, -6]);
	* // solve the linear system M*x = b
	* var x = M.solve(b); // returns vector [1, -1]
	*/
	//# exports.Matrix.prototype.solve = function (vec) { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(solve);

	/**
	* Calculates the matrix row norms.
	* @returns {module:la.Vector} Vector, where the value at i-th index is the norm of the i-th row of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	* // get the row norms of the matrix
	* var rowNorms = mat.rowNorms(); // returns the vector [5, 17/2]
	*/
	//# exports.Matrix.prototype.rowNorms = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(rowNorms);

	/**
	* Calculates the matrix column norms.
	* @returns {module:la.Vector} Vector, where the value at i-th index is the norm of the i-th column of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	* // get the row norms of the matrix
	* var rowNorms = mat.colNorms(); // returns the vector [5, 17/2]
	*/
	//# exports.Matrix.prototype.colNorms = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(colNorms);

	/**
	* Normalizes each column of matrix.
	* @returns {module:la.Matrix} Self. The columns of matrix are normalized. 
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	* // normalize the columns of the matrix
	* // the matrix is going to be of the form:
	* // 3/5     8/17
	* // 4/5    15/17
	* mat.normalizeCols();
	*/
	//# exports.Matrix.prototype.normalizeCols = function () { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(normalizeCols);

	/**
	* Returns the matrix as string.
	* @returns {string} Dense matrix as string.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, 2], [3, 5]]);
	* // get matrix as string
	* var text = mat.toString(); // returns `1 2 \n3 5 \n\n`
	*/
	//# exports.Matrix.prototype.toString = function () { return ""; }
	JsDeclareFunction(toString);

	/**
	* Transforms the matrix from dense to sparse format.
	* @returns {module:la.SparseMatrix} Sparse column matrix representation of dense matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [0, 3], [-4, 0]]);
	* // transform the matrix into the sparse form
	* var spMat = mat.sparse();
	*/
	//# exports.Matrix.prototype.sparse = function () { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(sparse);

	/**
	* Returns the frobenious norm of matrix.
	* @returns {number} Frobenious norm of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 4]]);
	* // get the frobenious norm of the matrix
	* var frob = mat.frob(); // returns the value Math.sqrt(30)
	*/
	//# exports.Matrix.prototype.frob = function () { return 0.0; }
	JsDeclareFunction(frob);

	/**
	* Gives the number of rows of matrix. Type `number`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the number of rows
	* var rowN = mat.rows; // returns 3
	*/
	//# exports.Matrix.prototype.rows = 0;
	JsDeclareProperty(rows);

	/**
	* Gives the number of columns of matrix. Type `number`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the number of cols
	* var colsN = mat.cols; // returns 2
	*/
	//# exports.Matrix.prototype.cols = 0;
	JsDeclareProperty(cols);

	/**
	* Gives the index of the maximum element in the given row.
	* @param {number} rowIdx - Row index (zero based).
	* @returns {number} Column index (zero based) of the maximum value in the <code>rowIdx</code>-th row of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the column id of the maximum value of the second row
	* var maxRow = mat.rowMaxIdx(1); // returns the value 0
	*/
	//# exports.Matrix.prototype.rowMaxIdx = function (rowIdx) { return 0; }
	JsDeclareFunction(rowMaxIdx);

	/**
	* Gives the index of the maximum element in the given column.
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} Row index (zero based) of the maximum value in <code>colIdx</code>-th column of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the row id of the maximum value of the second column
	* var maxRow = mat.colMaxIdx(1); // returns the value 2
	*/
	//# exports.Matrix.prototype.colMaxIdx = function (colIdx) { return 0; }
	JsDeclareFunction(colMaxIdx);

	/**
	* Returns the corresponding column of matrix as vector.
	* @param {number} colIdx - Column index (zero based).
	* @returns {module:la.Vector} The <code>colIdx</code>-th column of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the second column of the matrix
	* var col = mat.getCol(1);
	*/
	//# exports.Matrix.prototype.getCol = function (colIdx) { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(getCol);

	/**
	* Sets the column of the matrix.
	* @param {number} colIdx - Column index (zero based).
	* @param {module:la.Vector} vec - The new column of matrix.
	* @returns {module:la.Matrix} Self. The <code>colIdx</code>-th column is changed.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a matrix
	* var mat = new la.Matrix([[1, -3, 2], [9, 2, -4],  [-2, 3, 3]]);
	* // create a vector
	* var vec = new la.Vector([-3, 2, 2]);
	* // set the first column of the matrix with the vector
	* // the changed matrix is now
	* // -3   -3    2
	* //  2    2   -4
	* //  2    3    3
	* mat.setCol(0, vec);
	*/
	//# exports.Matrix.prototype.setCol = function (colIdx, vec) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(setCol);

	/**
	* Gets the submatrix from the column ids.
	* @param {module:la.IntVector} intVec - The vector containing the column ids.
	* @returns {module:la.Matrix} The submatrix containing the the columns of the original matrix.
    * @example 
    * //import la module
    * var la = require('qminer').la;
    * // create a random matrix
    * var mat = new la.Matrix({ rows: 10, cols: 10, random: true });
    * // get the submatrix containing the 1, 2 and 4 column
    * var submat = mat.getColSubmatrix(new la.IntVector([0, 1, 3]));
	*/
	//# exports.Matrix.prototype.getColSubmatrix = function (intVec) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(getColSubmatrix);

	/**
	* Gets the submatrix from the column ids.
	* @param {number} minRow - The minimum row index.
	* @param {number} maxRow - The maximum row index.
	* @param {number} minCol - The minimum column index.
	* @param {number} maxCol - The maximum column index.
	* @returns {module:la.Matrix} The submatrix of the original matrix.
    * @example
    * //import la module
    * var la = require('qminer').la;
    * // create a random matrix
    * var mat = new la.Matrix({ rows: 10, cols: 10, random: true });
    * // get the submatrix containing from the position (1, 2) to (7, 4)
    * var submat = mat.getSubmatrix(1, 7, 2, 4);
	*/
	//# exports.Matrix.prototype.getSubmatrix = function (minRow, maxRow, minCol, maxCol) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(getSubmatrix);

	/**
	* Returns the corresponding row of matrix as vector.
	* @param {number} rowIdx - Row index (zero based).
	* @returns {module:la.Vector} The <code>rowIdx</code>-th row of matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create the matrix
	* var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	* // get the first row of the matrix
	* var row = mat.getRow(1);
	*/
	//# exports.Matrix.prototype.getRow = function (rowIdx) { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(getRow);

	/**
	* Sets the row of matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {module:la.Vector} vec - The new row of matrix.
	* @returns {module:la.Matrix} Self. The <code>rowIdx</code>-th row is changed.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a matrix
	* var mat = new la.Matrix([[1, -3, 2], [9, 2, -4],  [-2, 3, 3]]);
	* // create a vector
	* var vec = new la.Vector([-3, 2, 2]);
	* // set the first row of the matrix with the vector
	* // the changed matrix is now
	* // -3    2    2
	* //  9    2   -4
	* // -2    3    3
	* mat.setRow(0, vec);
	*/
	//# exports.Matrix.prototype.setRow = function (rowIdx, vec) { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(setRow);

	/**
	* Returns the diagonal elements of matrix.
	* @returns {module:la.Vector} Vector containing the diagonal elements.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new matrix
	* var mat = new la.Matrix([[1, -1, 0], [15, 8, 3], [0, 1, 0]]);
	* // call diag function
	* var vec = mat.diag(); // returns a vector [1, 8, 0]
	*/
	//# exports.Matrix.prototype.diag = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(diag);

	/**
	* Saves the matrix as output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} The output stream <code>fout</code>.
	* @example
	* // import the modules
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create new matrix
	* var mat = new la.Matrix([[1, 2], [3, 4]]);
	* // open write stream
	* var fout = fs.openWrite('mat.dat');
	* // save matrix and close write stream
	* mat.save(fout).close();
	*/
	//# exports.Matrix.prototype.save = function (fout) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);

	/**
	* Loads the matrix from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.Matrix} Self. It is made out of the input stream <code>fin</code>.
	* @example
	* // import the modules
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create an empty matrix
	* var mat = new la.Matrix();
	* // open a read stream ('mat.dat' is pre-saved)
	* var fin = fs.openRead('mat.dat');
	* // load the matrix
	* mat.load(fin);
	*/
	//# exports.Matrix.prototype.load = function (FIn) { return Object.create(require('qminer').la.Matrix.prototype); }
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
* Sparse Vector.
* @classdesc Sparse vector is an array of (int,double) pairs that represent column indices and values.
* @class
* @param {(Array<Array<number>> | module:la.SparseVector)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. Using a nested array of vector elements. Example: `[[0, 2],[2, 3]]` has two nonzero values, first value is 2 at position 0, second value is 3 at position 2,
* <br>2. using a sparse vector (copy constructor).
* @param {number} [dim] - Maximum length of sparse vector. <i>It is only in combinantion with nested array of vector elements.</i>
* @example
* // import la module
* var la = require('qminer').la;
* // create new sparse vector with arrays
* var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]); // sparse vector [1, 0, 3, 6]
* // create new sparse vector with dim
* var spVec2 = new la.SparseVector([[0, 1], [2, 3], [3, 6]], 5); // largest index (zero based) is 4
*/
//# exports.SparseVector = function(arg, dim) { return Object.create(require('qminer').la.SparseVector.prototype); }	

class TNodeJsSpVec : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsSpVec() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "SparseVector"; }

public:
	// wrapped C++ objects
	TIntFltKdV Vec;
	TInt Dim;

	// C++ constructor
	TNodeJsSpVec() : Dim(-1) { }
	TNodeJsSpVec(const TIntFltKdV& IntFltKdV, const int& Dim = -1);

public:
	static TNodeJsSpVec* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	/**
	* Returns an element of the sparse vector.
	* @param {number} idx - Index (zero based).
	* @returns {number} Sparse vector element.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a sparse vector
	* var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	* // get the value at the position 3
	* vec.at(3); // returns the value 2
	*/
	//# exports.SparseVector.prototype.at = function (idx) { return 0.0; }
	JsDeclareFunction(at);

	/**
	* Puts a new element in sparse vector.
	* @param {number} idx - Index (zero based).
	* @param {number} num - Input value.
	* @returns {module:la.SparseVector} Self. It puts/changes the values with the index `idx` to the value `num`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	* // set the new values at position 2
	* vec.put(2, -4);
	*/
	//# exports.SparseVector.prototype.put = function (idx, num) { return Object.create(require('qminer').la.SparseVector.prototype); }
	JsDeclareFunction(put);

	/**
	* Returns the sum of all values in sparse vector.
	* @returns {number} The sum of all values in sparse vector.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	* // get the sum of the values in the vector
	* vec.sum(); // returns -2
	*/
	//# exports.SparseVector.prototype.sum = function () { return 0.0; }
	JsDeclareFunction(sum);

	/**
	* Returns the inner product of the parameter and the sparse vector.
	* @param {(module:la.Vector | module:la.SparseVector)} arg - The inner product input.
	* @returns {number} The inner product of the two vectors.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create two vectors, one sparse and one dense
	* var sparse = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	* var dense = new la.Vector([3, -4, 2, 0.5, -1]);
	* // get the inner product of the vectors
	* sparse.inner(dense); // returns the value 9
	*/
	//# exports.SparseVector.prototype.inner = function (arg) { return 0.0; }
	JsDeclareFunction(inner);

	/**
	* Multiplies the sparse vector with a scalar.
	* @param {number} num - The scalar.
	* @returns {module:la.SparseVector} The product of `num` and sparse vector.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]);
	* // multiply sparse vector with scalar 3.14
	* var spVec2 = spVec.multiply(3.14); // returns sparse vector [3.14, 0, 9.42, 18.84]
	*/
	//# exports.SparseVector.prototype.multiply = function (num) { return Object.create(require('qminer').la.SparseVector.prototype); }
	JsDeclareFunction(multiply);

	/**
	* Normalizes the sparse vector.
	* @returns {module:la.SparseVector} Self. The vector is normalized.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse vector
    * var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]);
    * // normalize the sparse vector
    * spVec.normalize();
	*/
	//# exports.SparseVector.prototype.normalize = function () { return Object.create(require('qminer').la.SparseVector.prototype); }
	JsDeclareFunction(normalize);

	/**
	* Returns the number of non-zero values. Type `number`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]]);
	* // check the number of nonzero values in sparse vector
    * // returns 4
	* var nonz = vec.nnz;
	*/
	//# exports.SparseVector.prototype.nnz = 0;
	JsDeclareProperty(nnz);

	/**
	* Returns the dimension of sparse vector. Type `number`.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse vector and designate the dimension of the vector
    * var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]], 15);
    * // get the dimension of the sparse vector
    * // returns 15
    * var dim = vec.dim; 
	*/
	//# exports.SparseVector.prototype.dim = 0;
	JsDeclareProperty(dim);

	/**
	* Returns the norm of sparse vector.
	* @returns {number} Norm of sparse vector.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse vector
    * var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]]); 
    * // get the norm of the vector
    * var norm = vec.norm();
	*/
	//# exports.SparseVector.prototype.norm = function () { return 0.0; }
	JsDeclareFunction(norm);

	/**
	* Returns the dense vector representation of the sparse vector.
	* @returns {module:la.Vector} The dense vector representation.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse vector
    * var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]]); 
    * // create a dense representation of the vector
    * var dense = vec.full();
	*/
	//# exports.SparseVector.prototype.full = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(full);

	/**
	* Returns a dense vector of values of non-zero elements of sparse vector.
	* @returns {module:la.Vector} A dense vector of values.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sparse vector
    * var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]]);
    * // get the non-zero values of the sparse vector
    * var valVec = vec.valVec();
	*/
	//# exports.SparseVector.prototype.valVec = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(valVec);

	/**
	* Returns a dense vector of indices (zero based) of non-zero elements of sparse vector.
	* @returns {module:la.Vector} A dense vector of indeces.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create a new sprase vector
    * var vec = new la.SparseVector([[0, 2], [3, 1], [7, 5], [11, 4]]);
    * // get the non-zero indeces of the sparse vector
    * var idxVec = vec.idxVec();
	*/
	//# exports.SparseVector.prototype.idxVec = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(idxVec);
		
	/**
	* Returns the string representation.
	* @returns {string} The string representation of the sparse vector.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var spVec = new la.SparseVector([[0, 1], [2, 3]]);	
	* // get the string representation of the vector
	* spVec.toString(); // returns the string '[(0, 1), (2, 3)]'
	*/
	//# exports.SparseVector.prototype.toString = function () { return ""; }
	JsDeclareFunction(toString);
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
* @classdesc Sparse Matrix is represented as a dense vector of sparse vectors which correspond to matrix columns.
* @class
* @param {(Array<Array<Array<number>>> | module:la.SparseMatrix)} [arg] - Constructor arguments. There are two ways of constructing:
* <br>1. using a nested array of sparse vectors (columns). A sparse vector is a nested array of pairs, first value is index, second is value. Example: [[[0, 2]], [[0, 1], [2, 3]]] has 2 columns.
* The second non-zero element in second column has a value 3 at index 2,
* <br>2. using a sparse matrix (copy constructor).
* @param {number} [rows] - Maximal number of rows in sparse vector. <i>It is only in combinantion with nested array of vector elements.</i>
* @example
* // import la module
* var la = require('qminer').la;
* // create a new sparse matrix with array
* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
* // create a new sparse matrix with specified max rows
* var mat2 = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]], 3);
*/
//# exports.SparseMatrix = function(arg) { return Object.create(require('qminer').la.SparseMatrix.prototype); }	

class TNodeJsSpMat : public node::ObjectWrap {
	friend class TNodeJsUtil;
private:
	static v8::Persistent<v8::Function> Constructor;
    ~TNodeJsSpMat() { TNodeJsUtil::ObjNameH.GetDat(GetClassId()).Val3++; TNodeJsUtil::ObjCount.Val3++; }
public:
	static void Init(v8::Handle<v8::Object> exports);
	static const TStr GetClassId() { return "SparseMatrix"; }

	// wrapped C++ object
	TVec<TIntFltKdV> Mat;
	TInt Rows;

	// C++ constructor
	TNodeJsSpMat() : Rows(-1) { }
	TNodeJsSpMat(const TVec<TIntFltKdV>& _Mat, const int& _Rows = -1)
		: Mat(_Mat), Rows(_Rows) { }
    static v8::Local<v8::Object> New(const TVec<TIntFltKdV>& _Mat, const int& _Rows = -1);

public:
	static TNodeJsSpMat* NewFromArgs(const v8::FunctionCallbackInfo<v8::Value>& Args);

	/**
	* Returns an element of the sparse matrix at the given location.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @returns {number} Matrix value.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	* // get the value at the position (1, 1)
	* mat.at(1, 1); // returns 3
	*/
	//# exports.SparseMatrix.prototype.at = function (rowIdx, colIdx) { return 0.0; }
	JsDeclareFunction(at);

	/**
	* Puts an element in sparse matrix.
	* @param {number} rowIdx - Row index (zero based).
	* @param {number} colIdx - Column index (zero based).
	* @param {number} num - Element value.
	* @returns {module:la.SparseMatrix} Self. The value at position (`rowIdx`, `colIdx`) is changed.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	* // set the value at position (2, 2) to -4
	* mat.put(2, 2, -4);
	*/
	//# exports.SparseMatrix.prototype.put = function (rowIdx, colIdx, num) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(put);

	//!- `spVec = spMat[colIdx]; spMat[colIdx] = spVec` -- setting and getting sparse vectors `spVec` from sparse column matrix, given column index `colIdx` (integer)
	JsDeclareSetIndexedProperty(indexGet, indexSet);

	JsDeclareFunction(indexGet);

	/**
	* Returns the column of the sparse matrix.
	* @param {number} colIdx - The column index (zero based).
	* @returns {module:la.SparseVector} Sparse vector corresponding to the `colIdx`-th column of sparse matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a sparse matrix
	* var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	* // get the first column as a vector
	* var first = mat.getCol(0); // returns the first column of the sparse matrix
	*/
	//# exports.SparseMatrix.prototype.getCol = function (colIdx) { return Object.create(require('qminer').la.SparseVector.prototype); }
	JsDeclareFunction(indexSet);

	/**
	* Sets a column in sparse matrix.
	* @param {number} colIdx - Column index (zero based).
	* @param {module:la.SparseVector} spVec - The new column sparse vector.
	* @returns {module:la.SparseMatrix} Self. The `colIdx`-th column has been replaced with `spVec`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	* // create a new sparse vector to replace the third column
	* var vec = new la.SparseVector([[0, 3], [2, -5]]);
	* // set the third column of mat to vec
	* mat.setCol(2, vec); // returns mat with the third column changed
	*/
	//# exports.SparseMatrix.prototype.setCol = function (colIdx, spVec) { return Object.create(require('qminer').la.SparseVector.prototype); }

	/**
	* Attaches a column to the sparse matrix.
	* @param {module:la.SparseVector} spVec - Attached column as sparse vector.
	* @returns {module:la.SparseMatrix} Self. The last column is now the added `spVec` and the number of columns is now bigger by one.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse vector
	* var mat = new la.SparseMatrix([[[0, 2], [3, 5]], [[1, -3]]]);
	* // create a new vector
	* var vec = new la.SparseVector([[0, 2], [2, -3]]);
	* // push the newly created vector to the matrix
	* // the new matrix is going to be (in sparse form)
	* // 2    0    2
	* // 0   -3    0
	* // 0    0   -3
	* // 5    0    0
	* mat.push(vec);
	*/
	//# exports.SparseMatrix.prototype.push = function (spVec) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(push);

	/**
	* Multiplies argument with sparse maatrix.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input.
	* @returns {(module:la.Vector | module:la.Matrix)}
	* <br>1. {@link module:la.Matrix}, if `arg` is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if `arg` is {@link module:la.Vector} or {@link module:la.SparseVector}.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 2], [3, 5]], [[1, -3]]]);
    * // create a vector
    * var vec = new la.Vector([3, -2]);
    * // multiply the matrix and vector
    * var vec2 = mat.multiply(vec);
	*/
	//# exports.SparseMatrix.prototype.multiply = function (arg) { return (arg instanceof require('qminer').la.Vector | arg instanceof require('qminer').la.SparseVector) ? Object.create(require('qminer').la.Vector.prototype) : Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(multiply);

	/**
	* Sparse matrix transpose and multiplies with argument.
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input.
	* @returns {(module:la.Vector | module:la.Matrix)}
	* <br>1. {@link module:la.Matrix}, if `arg` is number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
	* <br>2. {@link module:la.Vector}, if `arg` is {@link module:la.Vector} or {@link module:la.SparseVector}.
    * @example
    * // import la module
    * var la = require('qminer').la;
    * // create new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 2], [3, 5]], [[1, -3]]]);
    * // create a dense matrix
    * var mat2 = new la.Matrix([[0, 1], [2, 3], [4, 5], [-1, 3]]);
    * // transpose mat and multiply it with mat2
    * var mat3 = mat.multiplyT(mat2);
	*/
	//# exports.SparseMatrix.prototype.multiplyT = function (arg) { return (arg instanceof require('qminer').la.Vector | arg instanceof require('qminer').la.SparseVector) ? Object.create(require('qminer').la.Vector.prototype) : Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(multiplyT);

	/**
	* Addition of two matrices.
	* @param {module:la.SparseMatrix} mat - The second sparse matrix.
	* @returns {module:la.SparseMatrix} Sum of the two sparse matrices.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create two sparse matrices
	* var mat = new la.SparseMatrix([[[0, 1], [3, 2]], [[1, -3]]]);
	* var mat2 = new la.SparseMatrix([[[0, 3]],[[2, 1]]]);
	* // get the sum of the two matrices
	* // returns the sum ( insparse form)
	* // 4    0
	* // 0   -3
	* // 0    1
	* // 2    0
	* var sum = mat.plus(mat2);
	*/
	//# exports.SparseMatrix.prototype.plus = function (spMat) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(plus);

	/**
	* Substraction of two matrices.
	* @param {module:la.SparseMatrix} mat - The second sparse matrix.
	* @returns {module:la.SparseMatrix} The difference of the two sparse matrices.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create two sparse matrices
	* var mat = new la.SparseMatrix([[[0, 1], [3, 2]], [[1, -3]]]);
	* var mat2 = new la.SparseMatrix([[[0, 3]],[[2, 1]]]);
	* // get the sum of the two matrices
	* // returns the sum ( insparse form)
	* // -2    0
	* //  0   -3
	* //  0   -1
	* //  2    0
	* var diff = mat.minus(mat2);
	*/
	//# exports.SparseMatrix.prototype.minus = function (spMat) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(minus);

	/**
	* Returns the transposed sparse matrix.
	* @returns {module:la.SparseMatrix} Transposed sparse matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2], [2, -3]], [[1, 1], [3, -2]]]);
	* // transpose the sparse matrix
	* // returns the transposed matrix (in sparse form)
	* // 2    0   -3    0
	* // 0    1    0   -2
	* mat.transpose();
	*/
	//# exports.SparseMatrix.prototype.transpose = function () { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(transpose);

    /**
    * Returns a submatrix containing only selected columns.
    * Columns are identified by a vector of ids.
	* @param {module:la.IntVector} columnIdVec - Integer vector containing selected column ids.
    * @returns {module:la.SparseMatrix} The submatrix containing the the columns of the original matrix.
    * @example
    * //import la module
    * var la = require('qminer').la;
    * // create a new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 2], [2, -3]], [[1, 1], [3, -2]]]);
    * // get the submatrix containing the 1, 2 and 4 column
    * var submat = mat.getColSubmatrix(new la.IntVector([1]));
    */
	//# exports.SparseMatrix.prototype.getColSubmatrix = function (columnIdVec) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
    JsDeclareFunction(getColSubmatrix);

    /**
    * Clear content of the matrix and sets its row dimension to -1.
    * @returns {module:la.SparseMatrix} Self. All the content has been cleared.
    * @example
    * //import la module
    * var la = require('qminer').la;
    * // create a new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 2], [2, -3]], [[1, 1], [3, -2]]]);
    * // clear the matrix
    * mat.clear();
    */
    //# exports.SparseMatrix.prototype.clear = function () { return Object.create(require('qminer').la.SparseMatrix.prototype); }
    JsDeclareFunction(clear);

	/**
	* Returns the vector of column norms of sparse matrix.
	* @returns {module:la.Vector} Vector of column norms. Ihe i-th value of the return vector is the norm of i-th column of sparse matrix.
    * @example
    * //import la module
    * var la = require('qminer').la;
    * // create a new sparse matrix
    * var mat = new la.SparseMatrix([[[0, 2], [2, -3]], [[1, 1], [3, -2]]]);
    * // get the column norms
    * var norms = mat.colNorms();
	*/
	//# exports.SparseMatrix.prototype.colNorms = function () { return Object.create(require('qminer').la.Vector.prototype); }
	JsDeclareFunction(colNorms);

	/**
	* Normalizes columns of sparse matrix.
	* @returns {module:la.SparseMatrix} Self. The columns of the sparse matrix are normalized.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	* // normalize matrix columns
    * // The new matrix elements are:
    * // 1  0.316227
    * // 0  0
    * // 0  0.948683
	* mat.normalizeCols();
	*/
	//# exports.SparseMatrix.prototype.normalizeCols = function () { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(normalizeCols);

	/**
	* Returns the dense representation of sparse matrix.
	* @returns {module:la.Matrix} Dense representation of sparse matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	* // create a dense representation of sparse matrix
    * // returns the dense matrix:
    * // 2  1
    * // 0  0
    * // 0  3
	* mat.full();
	*/
	//# exports.SparseMatrix.prototype.full = function () { return Object.create(require('qminer').la.Matrix.prototype); }
	JsDeclareFunction(full);

	/**
	* Returns the frobenious norm of sparse matrix.
	* @returns {number} Frobenious norm of sparse matrix.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 1], [1, 3]], [[0, 2], [1, 4]]]);
	* // get the frobenious norm of sparse matrix
	* var norm = mat.frob(); // returns sqrt(30)
	*/
	//# exports.SparseMatrix.prototype.frob = function () { return 0.0; }
	JsDeclareFunction(frob);

	/**
	* Gives the number of rows of sparse matrix. Type `number`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	* // check the number of rows in sparse matrix
	* mat.rows;
	*/
	//# exports.SparseMatrix.prototype.rows = 0; 
	JsDeclareProperty(rows);

	/**
	* Gives the number of columns of sparse matrix. Type `number`.
	* @example
	* // import la module
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	* // check the number of columns in sparse matrix
	* mat.cols;
	*/
	//# exports.SparseMatrix.prototype.cols = 0; 
	JsDeclareProperty(cols);

	/**
	* Prints sparse matrix on screen.
	* @example
	* // import la module
	* var la = require('qminer').la;
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

	/**
	* Saves the sparse matrix as output stream.
	* @param {module:fs.FOut} fout - Output stream.
	* @param {boolean} [saveMatlab=false] - If true, saves using matlab three column text format. Otherwise, saves using binary format.
	* @returns {module:fs.FOut} The output stream `fout`.
	* @example
	* // import the modules
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create a new sparse matrix
	* var mat = new la.SparseMatrix([[[0, 1]], [[0, 3], [1, 12]]]);
	* // open write stream
	* var fout = fs.openWrite('mat.dat');
	* // save matrix and close write stream
	* mat.save(fout).close();
	*/
	//# exports.SparseMatrix.prototype.save = function (fout, saveMatlab) { return Object.create(require('qminer').fs.FOut.prototype); }
	JsDeclareFunction(save);

	/**
	* Loads the sparse matrix from input stream.
	* @param {module:fs.FIn} fin - Input stream.
	* @returns {module:la.Matrix} Self. The content has been loaded using `fin`.
	* @example
	* // import the modules
	* var fs = require('qminer').fs;
	* var la = require('qminer').la;
	* // create an empty matrix
	* var mat = new la.SparseMatrix();
	* // open a read stream ('mat.dat' was previously created)
	* var fin = fs.openRead('mat.dat');
	* // load the matrix
	* mat.load(fin);
	*/
	//# exports.SparseMatrix.prototype.load = function (FIn) { return Object.create(require('qminer').la.SparseMatrix.prototype); }
	JsDeclareFunction(load);

	/**
	* Sets the row dimension.
	* @param {number} rowDim - Row dimension.
	* @example
	* // import the modules
	* var la = require('qminer').la;
	* // create an empty matrix
	* var mat = new la.SparseMatrix();
	* mat.setRowDim(2);
	* mat.rows // prints 2
	*/
	//# exports.SparseMatrix.prototype.setRowDim = function (rowDim) { }
	JsDeclareFunction(setRowDim);

	//!- `spMat2 = spMat.sign()` -- create a new sparse matrix `spMat2` whose elements are sign function applied to elements of `spMat`.
	// (TODO) JsDeclareFunction(sign);
	//!JSIMPLEMENT:src/qminer/spMat.js

};

#endif


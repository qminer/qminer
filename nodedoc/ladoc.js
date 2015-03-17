/**
* Linear algebra module.
* @module la
* @example
* // import module, create a random matrix and a vector, multiply. find svd of the matrix
*/
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
 exports.Matrix = function(arg) {}	
/**
	* Returns an element of matrix
	* @param {number} rowIdx - row index (zero based)
	* @param {number} colIdx - column index (zero based)
	* @returns {number} Matrix element
	*/
 exports.Matrix.prototype.at = function(rowIdx, colIdx) {}

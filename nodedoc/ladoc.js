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
/**
	* Sets an element of matrix
	* @param {number} rowIdx - row index (zero based)
	* @param {number} colIdx - column index (zero based)
	* @param {number} num - input value
	* @returns {module:la.Matrix} Self 
	*/
 exports.Matrix.prototype.put = function(rowIdx, colIdx, num) {}
/**
	* Right-hand side multiplication of matrix with parameter
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input. 
	* @returns {(module:la.Matrix | module:la.Vector)} Output - If arg is
	* <br>1. number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}: Output is {@link module:la.Matrix}.
	* <br>2. {@link module:la.Vector} or {@link module:la.SparseVector}: Output is {@link module:la.Vector}.
	*/
 exports.Matrix.prototype.multiply = function(arg) {}
/**
	* Matrix transpose and right-hand side multiplication of matrix with parameter
	* @param {(number | module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix)} arg - Multiplication input. Supports scalar, vector and matrix input. 
	* @returns {(module:la.Matrix | module:la.Vector)} Output - If arg is
	* <br>1. number, {@link module:la.Matrix} or {@link module:la.SparseMatrix}: Output is {@link module:la.Matrix}.
	* <br>2. {@link module:la.Vector} or {@link module:la.SparseVector}: Output is {@link module:la.Vector}.
	*/
 exports.Matrix.prototype.multiplyT = function(arg) {}
/**
     * Sets the element at position i.
     *
     * vec = vec.put(i, val)
     *
     * @param {Number} i - the position of the element
     * @returns {Vector} vec - a reference to itself
     */
/**
     * Adds an element to the end of the vector.
     *
     * len = vector.push(val)
     *
     * @param {Object} val - the element added to the vector
     * @returns {Number} len - the length of the vector
     */
/**
     * The splice() method changes the content of an array by removing existing elements and/or adding new elements.
     *
     * array.splice(start, deleteCount[, item1[, item2[, ...]]])
     *
     * @param {Number} start - Index at which to start changing the array. If greater than the length of the array, actual starting index will be set to the length of the array. If negative, will begin that many elements from the end.
     * @param {Number} deleteCount - An integer indicating the number of old array elements to remove. If deleteCount is 0, no elements are removed. In this case, you should specify at least one new element. If deleteCount is greater than the number of elements left in the array starting at start, then all of the elements through the end of the array will be deleted.
     * @param {Object} [itemN] - The element to add to the array. If you don't specify any elements, splice() will only remove elements from the array.
     * @returns {Vector} - a reference to itself
     */
/**
	 * Adds an element to the beginning of the vector.
	 *
	 * len = vector.unshift(val)
	 *
	 * @param {Object} val - the element added to the vector
	 * @returns {Number} len - the length of the vector
	 */
/**
     * Sums the elements in the vector.
     *
     * @returns {Object} - the sum
     */
/**
     * Randomly reorders the elements of the vector.
     *
     * @returns {Vector} - returns a reference to itself
     */

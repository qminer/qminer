require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the ladoc_structures.js file', function () {
describe("Linear algebra module, number 1", function () {
it("should make test number 1", function () {

 // import module, create a random matrix and a vector, multiply. find svd of the matrix

});
});
describe("Computes the truncated SVD decomposition, number 2", function () {
it("should make test number 2", function () {

	 // import the modules
	 var la = require('qminer').la;
	 // create a random matrix
	 var A = new la.Matrix({ rows: 10, cols: 5, random: true });
	 // set the parameters for the calculation
	 var k = 2; // number of singular vectors 
	 var param = { iter: 1000, tol: 1e-4 };
	 // calculate the svd
	 la.svd(A, k, param, function (err, result) {
	    if (err) { console.log(err); }
	    // successful calculation
	    var U = result.U;
	    var V = result.V;
	    var s = result.s;
	 });
	
});
});
describe("Computes the truncated SVD decomposition, number 3", function () {
it("should make test number 3", function () {

	 // import the modules
	 var la = require('qminer').la;
	 // create a random matrix
	 var A = new la.Matrix({ rows: 10, cols: 5, random: true });
	 // set the parameters for the calculation
	 var k = 2; // number of singular vectors
	 var param = { iter: 1000, tol: 1e-4 };
	 // calculate the svd
	 var result = la.svd(A, k, param);
	 // successful calculation
	 var U = result.U;
	 var V = result.V;
	 var s = result.s;
	
});
});
describe("Matrix clas, number 4", function () {
it("should make test number 4", function () {

 // import la module
 var la = require('qminer').la;
 // create new matrix with matrixArg
 var mat = new la.Matrix({"rows": 3, "cols": 2, "random": true}); // creates a 3 x 2 matrix with random values
 // create a new matrix with nested arrays
 var mat2 = new la.Matrix([[1, 7, 4], [-10, 0, 3]]); // creates a 2 x 3 matrix with the designated values

});
});
describe("Returns an element of matrix, number 5", function () {
it("should make test number 5", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[2, 3], [-2, -2], [-3, 1]]);
	 // get the value at the index (2, 1)
	 var value = mat.at(2, 1); // returns the value 1
	
});
});
describe("Sets an element or a block of matrix, number 6", function () {
it("should make test number 6", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2, 3], [4, 5, 6], [7, 8, 9]]);
	 var arg = new la.Matrix([[10, 11], [12, 13]]);
	 mat.put(0, 1, arg);
	 // updates the matrix to
     // 1  10  11
     // 4  12  13
     // 7   8   9   
	
});
});
describe("Right-hand side multiplication of matrix with parameter, number 7", function () {
it("should make test number 7", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [-1, 5]]);
	 // create a new vector
	 var vec = new la.Vector([1, -1]);
	 //multiply mat and vec
	 var vec2 = mat.multiply(vec); // returns vector [-1, -6]
	
});
});
describe("Matrix transpose and right-hand side multiplication of matrix with parameter, number 8", function () {
it("should make test number 8", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [-1, 5]]);
	 // create a new vector
	 var vec = new la.Vector([1, -1]);
	 //multiply mat and vec
	 var vec2 = mat.multiplyT(vec); // returns vector [2, -3]
	
});
});
describe("Addition of two matrices, number 9", function () {
it("should make test number 9", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two matrices
	 var mat = new la.Matrix([[1, 2], [-1, 5]]);
	 var mat2 = new la.Matrix([[1, -1], [3, 2]]);
	 // add the matrices 
	 // the return matrix is
	 // 2   1
	 // 2   7
	 var sum = mat.plus(mat2);
	
});
});
describe("Substraction of two matrices, number 10", function () {
it("should make test number 10", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two matrices
	 var mat = new la.Matrix([[1, 2], [-1, 5]]);
	 var mat2 = new la.Matrix([[1, -1], [3, 2]]);
	 // substract the matrices
	 // the return matrix is
	 //  0   3
	 // -4   3
	 var diff = mat.minus(mat2);
	
});
});
describe("Transposes matrix, number 11", function () {
it("should make test number 11", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a matrix
	 var mat = new la.Matrix([[2, -5], [3, 1]]);
	 // transpose the matrix
	 // the return matrix is
	 //  2   3
	 // -5   1
	 var trans = mat.transpose();
	
});
});
describe("Solves the linear system, number 12", function () {
it("should make test number 12", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var M = new la.Matrix([[1, 2], [-1, -5]]);
	 // create a new vector
	 var b = new la.Vector([-1, -6]);
	 // solve the linear system M*x = b
	 var x = M.solve(b); // returns vector [1, -1]
	
});
});
describe("Returns a vector of row norms, number 13", function () {
it("should make test number 13", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	 // get the row norms of the matrix
	 var rowNorms = mat.rowNorms(); // returns the vector [5, 17/2]
	
});
});
describe("Returns a vector of column norms, number 14", function () {
it("should make test number 14", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	 // get the row norms of the matrix
	 var rowNorms = mat.colNorms(); // returns the vector [5, 17/2]
	
});
});
describe("Normalizes each column of matrix, number 15", function () {
it("should make test number 15", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[3, 4], [4, 15/2]]);
	 // normalize the columns of the matrix
	 // the matrix is going to be of the form:
	 // 3/5     8/17
	 // 4/5    15/17
	 mat.normalizeCols();
	
});
});
describe("Returns the matrix as string, number 16", function () {
it("should make test number 16", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [3, 5]]);
	 // get matrix as string
	 var text = mat.toString(); // returns `1 2 \n3 5 \n\n`
	
});
});
describe("Transforms the matrix from dense to sparse format, number 17", function () {
it("should make test number 17", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [0, 3], [-4, 0]]);
	 // transform the matrix into the sparse form
	 var spMat = mat.sparse();
	
});
});
describe("Returns the frobenious norm of matrix, number 18", function () {
it("should make test number 18", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 4]]);
	 // get the frobenious norm of the matrix
	 var frob = mat.frob(); // returns the value Math.sqrt(30)
	
});
});
describe("Gives the number of rows of matrix, number 19", function () {
it("should make test number 19", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the number of rows
	 var rowN = mat.rows; // returns 3
	
});
});
describe("Gives the number of columns of matrix, number 20", function () {
it("should make test number 20", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the number of cols
	 var colsN = mat.cols; // returns 2
	
});
});
describe("Gives the index of the maximum element in the given row, number 21", function () {
it("should make test number 21", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the column id of the maximum value of the second row
	 var maxRow = mat.rowMaxIdx(1); // returns the value 0
	
});
});
describe("Gives the index of the maximum element in the given column, number 22", function () {
it("should make test number 22", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the row id of the maximum value of the second column
	 var maxRow = mat.colMaxIdx(1); // returns the value 2
	
});
});
describe("Returns the corresponding column of matrix as vector, number 23", function () {
it("should make test number 23", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the second column of the matrix
	 var col = mat.getCol(1);
	
});
});
describe("Sets the column of the matrix, number 24", function () {
it("should make test number 24", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a matrix
	 var mat = new la.Matrix([[1, -3, 2], [9, 2, -4],  [-2, 3, 3]]);
	 // create a vector
	 var vec = new la.Vector([-3, 2, 2]);
	 // set the first column of the matrix with the vector
	 // the changed matrix is now
	 // -3   -3    2
	 //  2    2   -4
	 //  2    3    3
	 mat.setCol(0, vec);
	
});
});
describe("Returns the corresponding row of matrix as vector, number 25", function () {
it("should make test number 25", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create the matrix
	 var mat = new la.Matrix([[1, 2], [3, 1], [-4, 5]]);
	 // get the first row of the matrix
	 var row = mat.getRow(1);
	
});
});
describe("Sets the row of matrix, number 26", function () {
it("should make test number 26", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a matrix
	 var mat = new la.Matrix([[1, -3, 2], [9, 2, -4],  [-2, 3, 3]]);
	 // create a vector
	 var vec = new la.Vector([-3, 2, 2]);
	 // set the first row of the matrix with the vector
	 // the changed matrix is now
	 // -3    2    2
	 //  9    2   -4
	 // -2    3    3
	 mat.setRow(0, vec);
	
});
});
describe("Returns the diagonal elements of matrix, number 27", function () {
it("should make test number 27", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, -1, 0], [15, 8, 3], [0, 1, 0]]);
	 // call diag function
	 var vec = mat.diag(); // returns a vector [1, 8, 0]
	
});
});
describe("Saves the matrix as output stream, number 28", function () {
it("should make test number 28", function () {

	 // import the modules
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create new matrix
	 var mat = new la.Matrix([[1, 2], [3, 4]]);
	 // open write stream
	 var fout = fs.openWrite('mat.dat');
	 // save matrix and close write stream
	 mat.save(fout).close();
	
});
});
describe("Loads the matrix from input stream, number 29", function () {
it("should make test number 29", function () {

	 // import the modules
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty matrix
	 var mat = new la.Matrix();
	 // open a read stream ('mat.dat' is pre-saved)
	 var fin = fs.openRead('mat.dat');
	 // load the matrix
	 mat.load(fin);
	
});
});
describe("Sparse Vecto, number 30", function () {
it("should make test number 30", function () {

 // import la module
 var la = require('qminer').la;
 // create new sparse vector with arrays
 var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]); // sparse vector [1, 0, 3, 6]
 // create new sparse vector with dim
 var spVec2 = new la.SparseVector([[0, 1], [2, 3], [3, 6]], 5); // largest index (zero based) is 4

});
});
describe("Returns an element of the sparse vector, number 31", function () {
it("should make test number 31", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // get the value at the position 3
	 vec.at(3); // returns the value 2
	
});
});
describe("Puts a new element in sparse vector, number 32", function () {
it("should make test number 32", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // set the new values at position 2
	 vec.put(2, -4);
	
});
});
describe("Returns the sum of all values in sparse vector, number 33", function () {
it("should make test number 33", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // get the sum of the values in the vector
	 vec.sum(); // returns -2
	
});
});
describe("Returns the inner product of the parameter and the sparse vector, number 34", function () {
it("should make test number 34", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two vectors, one sparse and one dense
	 var sparse = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 var dense = new la.Vector([3, -4, 2, 0.5, -1]);
	 // get the inner product of the vectors
	 sparse.inner(dense); // returns the value 9
	
});
});
describe("Multiplies the sparse vector with a scalar, number 35", function () {
it("should make test number 35", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]);
	 // multiply sparse vector with scalar 3.14
	 var spVec2 = spVec.multiply(3.14); // returns sparse vector [3.14, 0, 9.42, 18.84]
	
});
});
describe("Returns the number of nonzero values, number 36", function () {
it("should make test number 36", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var vec = new la.SparseVector([[0,2], [3,1], [7, 5], [11,4]]);
	 // check the number of nonzero values in sparse vector
	 var nonz = vec.nnz;
	 // returns 4
	
});
});
describe("Returns the string representation, number 37", function () {
it("should make test number 37", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3]]);	
	 // get the string representation of the vector
	 spVec.toString(); // returns the string '[(0, 1), (2, 3)]'
	
});
});
describe("Sparse Matri, number 38", function () {
it("should make test number 38", function () {

 // import la module
 var la = require('qminer').la;
 // create a new sparse matrix with array
 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
 // create a new sparse matrix with specified max rows
 var mat2 = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]], 3);

});
});
describe("Returns an element of the sparse matrix at the given location, number 39", function () {
it("should make test number 39", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // get the value at the position (1, 1)
	 mat.at(1, 1); // returns 3
	
});
});
describe("Puts an element in sparse matrix, number 40", function () {
it("should make test number 40", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	 // set the value at position (2, 2) to -4
	 mat.put(2, 2, -4);
	
});
});
describe("Returns the column of the sparse matrix, number 41", function () {
it("should make test number 41", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse matrix
	 var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	 // get the first column as a vector
	 var first = mat.getCol(0); // returns the first column of the sparse matrix
	
});
});
describe("Sets a column in sparse matrix, number 42", function () {
it("should make test number 42", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	 // create a new sparse vector to replace the third column
	 var vec = new la.SparseVector([[0, 3], [2, -5]]);
	 // set the third column of mat to vec
	 mat.setCol(2, vec); // returns mat with the third column changed
	
});
});
describe("Attaches a column to the sparse matrix, number 43", function () {
it("should make test number 43", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var mat = new la.SparseMatrix([[[0, 2], [3, 5]], [[1, -3]]]);
	 // create a new vector
	 var vec = new la.SparseVector([[0, 2], [2, -3]]);
	 // push the newly created vector to the matrix
	 // the new matrix is going to be (in sparse form)
	 // 2    0    2
	 // 0   -3    0
	 // 0    0   -3
	 // 5    0    0
	 mat.push(vec);
	
});
});
describe("Returns the sum of two matrices, number 44", function () {
it("should make test number 44", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two sparse matrices
	 var mat = new la.SparseMatrix([[[0, 1], [3, 2]], [[1, -3]]]);
	 var mat2 = new la.SparseMatrix([[[0, 3]],[[2, 1]]]);
	 // get the sum of the two matrices
	 // returns the sum ( insparse form)
	 // 4    0
	 // 0   -3
	 // 0    1
	 // 2    0
	 var sum = mat.plus(mat2);
	
});
});
describe("Returns the difference of two matrices, number 45", function () {
it("should make test number 45", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two sparse matrices
	 var mat = new la.SparseMatrix([[[0, 1], [3, 2]], [[1, -3]]]);
	 var mat2 = new la.SparseMatrix([[[0, 3]],[[2, 1]]]);
	 // get the sum of the two matrices
	 // returns the sum ( insparse form)
	 // -2    0
	 //  0   -3
	 //  0   -1
	 //  2    0
	 var diff = mat.minus(mat2);
	
});
});
describe("Returns the transposed sparse matrix, number 46", function () {
it("should make test number 46", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2], [2, -3]], [[1, 1], [3, -2]]]);
	 // transpose the sparse matrix
	 // returns the transposed matrix (in sparse form)
	 // 2    0   -3    0
	 // 0    1    0   -2
	 mat.transpose();
	
});
});
describe("Normalizes columns of sparse matrix, number 47", function () {
it("should make test number 47", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // normalize matrix columns
	 mat.normalizeCols();
	 // The new matrix elements are:
	 // 1  0.316227
	 // 0  0
	 // 0  0.948683
	
});
});
describe("Returns the dense representation of sparse matrix, number 48", function () {
it("should make test number 48", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // create a dense representation of sparse matrix
	 mat.full();
	 // returns
	 // 2  1
	 // 0  0
	 // 0  3
	
});
});
describe("Returns the frobenious norm of sparse matrix, number 49", function () {
it("should make test number 49", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 1], [1, 3]], [[0, 2], [1, 4]]]);
	 // get the frobenious norm of sparse matrix
	 var norm = mat.frob(); // returns sqrt(30)
	
});
});
describe("Gives the number of rows of sparse matrix, number 50", function () {
it("should make test number 50", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // check the number of rows in sparse matrix
	 mat.rows;
	
});
});
describe("Gives the number of columns of sparse matrix, number 51", function () {
it("should make test number 51", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // check the number of columns in sparse matrix
	 mat.cols;
	
});
});
describe("Prints sparse matrix on screen, number 52", function () {
it("should make test number 52", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var spMat = new la.SparseMatrix([[[0, 1]], [[0, 3], [1, 8]]]);
	 // print sparse matrix on screen
	 // each row represents a nonzero element, where first value is row index, second
	 // value is column index and third value is element value. For this matrix:
	 // 0  0  1.000000
	 // 0  1  3.000000
	 // 1  1  8.000000
	 spMat.print();
	
});
});
describe("Saves the sparse matrix as output stream, number 53", function () {
it("should make test number 53", function () {

	 // import the modules
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 1]], [[0, 3], [1, 12]]]);
	 // open write stream
	 var fout = fs.openWrite('mat.dat');
	 // save matrix and close write stream
	 mat.save(fout).close();
	
});
});
describe("Loads the sparse matrix from input stream, number 54", function () {
it("should make test number 54", function () {

	 // import the modules
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty matrix
	 var mat = new la.SparseMatrix();
	 // open a read stream ('mat.dat' was previously created)
	 var fin = fs.openRead('mat.dat');
	 // load the matrix
	 mat.load(fin);
	
});
});
describe("Sets the row dimensio, number 55", function () {
it("should make test number 55", function () {

	 // import the modules
	 var la = require('qminer').la;
	 // create an empty matrix
	 var mat = new la.SparseMatrix();
	 mat.setRowDim(2);
	 mat.rows // prints 2
	
});
});
describe("Returns a string displaying rows, columns and number of non-zero elements of sparse matrix, number 56", function () {
it("should make test number 56", function () {

     // import la module
     var la = require('qminer').la;
     // create a new sparse matrix
     var mat = new la.SparseMatrix([[[0, 1]], [[0, 2], [1, 8]]]);
     // create the string
     var text = mat.toString(); // returns 'rows: -1, cols: 2, nnz: 3'
    
});
});
describe("Prints the sparse vector on-screen, number 57", function () {
it("should make test number 57", function () {

     // import la module
     var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3]]);
	 // print sparse vector
	 spVec.print(); // shows on-screen [(0, 1), (2, 3)]
	
});
});
describe("Prints the matrix on-screen, number 58", function () {
it("should make test number 58", function () {

     // import la module
     var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [3, 4]]);
	 // print the matrix
     // each row represents a row in the matrix. For this example:
     // 1  2
     // 3  4
	 mat.print();
	
});
});
describe("Prints the vector on-screen, number 59", function () {
it("should make test number 59", function () {

     // import la module
     var la = require('qminer').la;
     // create a new vector
     var vec = new la.Vector([1, 2, 3]);
     // print the vector
     // For this example it prints:
     // [1, 2, 3]
     vec.print();
    
});
});
describe("Copies the vector into a JavaScript array of numbers, number 60", function () {
it("should make test number 60", function () {

     // import la module
     var la = require('qminer').la;
     // create a new vector
     var vec = new la.Vector([1, 2, 3]);
     // create a JavaScript array out of vec
     var arr = vec.toArray(); // returns an array [1, 2, 3]
    
});
});
describe("Copies the matrix into a JavaScript array of arrays of numbers, number 61", function () {
it("should make test number 61", function () {

     // import la module
     var la = require('qminer').la;
     // create a new matrix
     var mat = new la.Matrix([[1, 2], [3, 4]]);
     // create a JavaScript array out of matrix
     var arr = mat.toArray(); // returns an array [[1, 2], [3, 4]]
    
});
});
describe("Copies the vector into a JavaScript array of numbers, number 62", function () {
it("should make test number 62", function () {

     // import la module
     var la = require('qminer').la;
     // create a new vector
     var vec = new la.Vector([1, 2, 3]);
     // create a JavaScript array out of vec
     var arr = la.copyVecToArray(vec); // returns an array [1, 2, 3]
    
});
});
describe("Returns a vector with all entries set to 1.0, number 63", function () {
it("should make test number 63", function () {

     // import la module
     var la = require('qminer').la;
     // create a 3-dimensional vector with all entries set to 1.0
     var vec = la.ones(3);
    
});
});
describe("Constructs a matrix by concatenating a doubly-nested array of matrices, number 64", function () {
it("should make test number 64", function () {

     // import la module
     var la = require('qminer').la;
     // create four matrices and concatenate (2 block columns, 2 block rows)
     var la = require('qminer').la;
     var A = new la.Matrix([[1,2], [3,4]]);
     var B = new la.Matrix([[5,6], [7,8]]);
     var C = new la.Matrix([[9,10], [11,12]]);
     var D = new la.Matrix([[13,14], [15,16]]);
     var mat = la.cat([[A,B], [C,D]]);
     // returns the matrix:
     // 1  2  5  6
     // 3  4  7  8
     // 9  10 13 14
     // 11 12 15 16
    
});
});
describe("Generates an integer vector given rang, number 65", function () {
it("should make test number 65", function () {

     // import la module
     var la = require('qminer').la;
     var vec = la.rangeVec(1, 3);
     // returns the vector:
     // 1  2  3
    
});
});
describe("returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix. The resulting array has one element for vector input, number 66", function () {
it("should make test number 66", function () {

     // import la module
     var la = require('qminer').la;
     // create a dense matrix
     var mat = new la.Matrix([[1, 2], [2, 0]]);
     la.findMaxIdx(mat)
     // returns the array:
     // [1, 0]
    
});
});
describe("computes and returns the pairwise squared euclidean distances between columns of `X1` and `X2` (`mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2`), number 67", function () {
it("should make test number 67", function () {

     // import la module
     var la = require('qminer').la;
     // construct two input matrices
     var X1 = new la.Matrix([[1,2], [2,0]]);
     var X2 = new la.Matrix([[1,0.5,0],[0,-0.5,-1]]);
     la.pdist2(X1, X2)
     // returns the matrix:
     // 4 6.5 10
     // 1 2.5 5
    
});
});

});

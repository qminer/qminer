require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the ladoc.js file', function () {
describe("Linear algebra module, number 1", function () {
it("should make test number 1", function () {

 // import module, create a random matrix and a vector, multiply. find svd of the matrix

});
});
describe("Matrix clas, number 2", function () {
it("should make test number 2", function () {

 // import la module
 var la = require('qminer').la;
 // create new matrix with matrixArg
 var mat = new la.Matrix({"rows": 3, "cols": 2, "random": true}); // creates a 3 x 2 matrix with random values
 // create a new matrix with nested arrays
 var mat2 = new la.Matrix([[1, 7, 4], [-10, 0, 3]]); // creates a 2 x 3 matrix with the designated values

});
});
describe("Sets an element or a block of matrix, number 3", function () {
it("should make test number 3", function () {

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
describe("Right-hand side multiplication of matrix with parameter, number 4", function () {
it("should make test number 4", function () {

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
describe("Matrix transpose and right-hand side multiplication of matrix with parameter, number 5", function () {
it("should make test number 5", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [-1, 5]]);
	 // create a new vector
	 var vec = new la.Vector([1, -1]);
	 //multiply mat and vec
	 var vec2 = mat.multiplyT(vec); // returns vector [2, 7]
	
});
});
describe("Addition of two matrices, number 6", function () {
it("should make test number 6", function () {

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
describe("Substraction of two matrices, number 7", function () {
it("should make test number 7", function () {

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
describe("Transposes matrix, number 8", function () {
it("should make test number 8", function () {

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
describe("Solves the linear system, number 9", function () {
it("should make test number 9", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var M = new la.Matrix([[1, 2], [-1, -5]]);
	 // create a new vector
	 var b = new la.Vector([-1, -6]);
	 // solve the linear system Mx = b
	 var x = M.solve(b); // returns vector [1, -1]
	
});
});
describe("Returns the matrix as string, number 10", function () {
it("should make test number 10", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, 2], [3, 5]]);
	 // get matrix as string
	 var text = mat.toString(); // returns `1 2 \n3 5 \n\n`
	
});
});
describe("Sets the column of the matrix, number 11", function () {
it("should make test number 11", function () {

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
describe("Sets the row of matrix, number 12", function () {
it("should make test number 12", function () {

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
describe("Returns the diagonal elements of matrix, number 13", function () {
it("should make test number 13", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new matrix
	 var mat = new la.Matrix([[1, -1, 0], [15, 8, 3], [0, 1, 0]]);
	 // call diag function
	 var vec = mat.diag(); // returns a vector [1, 8, 0]
	
});
});
describe("Saves the matrix as output stream, number 14", function () {
it("should make test number 14", function () {

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
describe("Loads the matrix from input stream, number 15", function () {
it("should make test number 15", function () {

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
describe("Sparse Vecto, number 16", function () {
it("should make test number 16", function () {

 // import la module
 var la = require('qminer').la;
 // create new sparse vector with arrays
 var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]); // sparse vector [1, 0, 3, 6]
 // create new sparse vector with dim
 var spVec2 = new la.SparseVector([[0, 1], [2, 3], [3, 6]], 5); // largest index (zero based) is 4

});
});
describe("Returns an element of the sparse vector, number 17", function () {
it("should make test number 17", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // get the value at the position 3
	 vec.at(3); // returns the value 2
	
});
});
describe("Puts a new element in sparse vector, number 18", function () {
it("should make test number 18", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // set the new values at position 2
	 vec.put(2, -4);
	
});
});
describe("Returns the sum of all values in sparse vector, number 19", function () {
it("should make test number 19", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var vec = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 // get the sum of the values in the vector
	 vec.sum(); // returns -2
	
});
});
describe("Returns the inner product of the parameter and the sparse vector, number 20", function () {
it("should make test number 20", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create two vectors, one sparse and one dense
	 var sparse = new la.SparseVector([[0, 1], [3, 2], [4, -5]]);
	 var dense = new la.Vector([3, -4, 2, 0.5, -1]);
	 // get the inner product of the vectors
	 sparse.inner(dense); // returns the value 9
	
});
});
describe("Multiplies the sparse vector with a scalar, number 21", function () {
it("should make test number 21", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3], [3, 6]]);
	 // multiply sparse vector with scalar 3.14
	 var spVec2 = spVec.multiply(3.14); // returns sparse vector [3.14, 0, 9.42, 18.84]
	
});
});
describe("Returns the string representation, number 22", function () {
it("should make test number 22", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3]]);	
	 // get the string representation of the vector
	 spVec.toString(); // returns the string '[(0, 1), (2, 3)]'
	
});
});
describe("Sparse Matri, number 23", function () {
it("should make test number 23", function () {

 // import la module
 var la = require('qminer').la;
 // create a new sparse matrix with array
 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
 // create a new sparse matrix with specified max rows
 var mat2 = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]], 3);

});
});
describe("Returns an element of the sparse matrix at the given location, number 24", function () {
it("should make test number 24", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse matrix
	 var mat = new la.SparseMatrix([[[0, 2]], [[0, 1], [2, 3]]]);
	 // get the value at the position (1, 1)
	 mat.at(1, 1); // returns 3
	
});
});
describe("Puts an element in sparse matrix, number 25", function () {
it("should make test number 25", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new sparse matrix
	 var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	 // set the value at position (2, 2) to -4
	 mat.put(2, 2, -4);
	
});
});
describe("Returns the column of the sparse matrix, number 26", function () {
it("should make test number 26", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a sparse matrix
	 var mat = new la.SparseMatrix([[[0, 3], [1, 2]], [[1, -2], [3, 4]], [[10, 8]]]);
	 // get the first column as a vector
	 var first = mat.getCol(0); // returns the first column of the sparse matrix
	
});
});
describe("Sets a column in sparse matrix, number 27", function () {
it("should make test number 27", function () {

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
describe("Attaches a column to the sparse matrix, number 28", function () {
it("should make test number 28", function () {

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
describe("Returns the sum of two matrices, number 29", function () {
it("should make test number 29", function () {

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
describe("Returns the difference of two matrices, number 30", function () {
it("should make test number 30", function () {

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
describe("Returns the transposed sparse matrix, number 31", function () {
it("should make test number 31", function () {

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
describe("Prints sparse matrix on screen, number 32", function () {
it("should make test number 32", function () {

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
describe("Saves the sparse matrix as output stream, number 33", function () {
it("should make test number 33", function () {

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
describe("Loads the sparse matrix from input stream, number 34", function () {
it("should make test number 34", function () {

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
describe("Sets the row dimensio, number 35", function () {
it("should make test number 35", function () {

	 // import the modules
	 var la = require('qminer').la;
	 // create an empty matrix
	 var mat = new la.SparseMatrix();
	 mat.setRowDim(2);
	 mat.rows // prints 2
	
});
});
describe("Returns a string displaying rows, columns and number of non-zero elements of sparse matrix, number 36", function () {
it("should make test number 36", function () {

     // import la module
     var la = require('qminer').la;
     // create a new sparse matrix
     var mat = new la.SparseMatrix([[[0, 1]], [[0, 2], [1, 8]]]);
     // create the string
     var text = mat.toString(); // returns 'rows: -1, cols: 2, nnz: 3'
    
});
});
describe("Prints the sparse vector on-screen, number 37", function () {
it("should make test number 37", function () {

     // import la module
     var la = require('qminer').la;
	 // create a new sparse vector
	 var spVec = new la.SparseVector([[0, 1], [2, 3]]);
	 // print sparse vector
	 spVec.print(); // shows on-screen [(0, 1), (2, 3)]
	
});
});
describe("Prints the matrix on-screen, number 38", function () {
it("should make test number 38", function () {

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
describe("Prints the vector on-screen, number 39", function () {
it("should make test number 39", function () {

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
describe("Copies the vector into a JavaScript array of numbers, number 40", function () {
it("should make test number 40", function () {

     // import la module
     var la = require('qminer').la;
     // create a new vector
     var vec = new la.Vector([1, 2, 3]);
     // create a JavaScript array out of vec
     var arr = vec.toArray(); // returns an array [1, 2, 3]
    
});
});
describe("Copies the matrix into a JavaScript array of arrays of numbers, number 41", function () {
it("should make test number 41", function () {

     // import la module
     var la = require('qminer').la;
     // create a new matrix
     var mat = new la.Matrix([[1, 2], [3, 4]]);
     // create a JavaScript array out of matrix
     var arr = mat.toArray(); // returns an array [[1, 2], [3, 4]]
    
});
});
describe("Copies the vector into a JavaScript array of numbers, number 42", function () {
it("should make test number 42", function () {

     // import la module
     var la = require('qminer').la;
     // create a new vector
     var vec = new la.Vector([1, 2, 3]);
     // create a JavaScript array out of vec
     var arr = la.copyVecToArray(vec); // returns an array [1, 2, 3]
    
});
});
describe("Returns a vector with all entries set to 1.0, number 43", function () {
it("should make test number 43", function () {

     // import la module
     var la = require('qminer').la;
     // create a 3-dimensional vector with all entries set to 1.0
     var vec = la.ones(3);
    
});
});
describe("Constructs a matrix by concatenating a doubly-nested array of matrices, number 44", function () {
it("should make test number 44", function () {

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
describe("Generates an integer vector given rang, number 45", function () {
it("should make test number 45", function () {

     // import la module
     var la = require('qminer').la;
     var vec = la.rangeVec(1, 3);
     // returns the vector:
     // 1  2  3
    
});
});
describe("returns a JS array of indices `idxArray` that correspond to the max elements in each column of dense matrix. The resulting array has one element for vector input, number 46", function () {
it("should make test number 46", function () {

     // import la module
     var la = require('qminer').la;
     // create a dense matrix
     var mat = new la.Matrix([[1, 2], [2, 0]]);
     la.findMaxIdx(mat)
     // returns the array:
     // [1, 0]
    
});
});
describe("computes and returns the pairwise squared euclidean distances between columns of `X1` and `X2` (`mat3[i,j] = ||mat(:,i) - mat2(:,j)||^2`), number 47", function () {
it("should make test number 47", function () {

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
describe("Vector - array of double, number 48", function () {
it("should make test number 48", function () {

 var la = require('qminer').la;
 // create a new empty vector
 var vec = new la.Vector();
 // create a new vector
 var vec2 = new la.Vector([1, 2, 3]);

});
});
describe("Changes the vector by removing and adding elements, number 49", function () {
it("should make test number 49", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([1, 2, 3]);
	 // splice the vector by removing the last two elements and adding 4, 5
	 vec.splice(1, 2, 4, 5)// returns vector [1, 4, 5]
	
});
});
describe("Sorts the vector (in place operation), number 50", function () {
it("should make test number 50", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([-2.0, 1.0, 3.0]);
	 // sort ascending
	 vec.sort(); // sorts to: [-2.0, 1.0, 3.0]
	 // sort using callback
	 vec.sort(function(arg1, arg2) { return Math.abs(arg1) - Math.abs(arg2); }); // sorts to: [1.0, -2.0, 3.0]
	
});
});
describe("Sorts the vector and returns the sorted vector as well as the permutatio, number 51", function () {
it("should make test number 51", function () {

	 // import la module
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([-2.0, 1.0, 3.0]);
	 // sort ascending
	 var result = vec.sortPerm(); // result.vec: [-2.0, 1.0, 3.0]
	 result.perm; // permutation index vector
	
});
});
describe("Deletes elements with sprecific index or more, number 52", function () {
it("should make test number 52", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([1, 2, 3]);
	 // trunc all elements with index 1 or more
	 vec.trunc(1); // returns vector [1]
	
});
});
describe("Creates a dense matrix A by multiplying two vectors x and y: A = x y^T, number 53", function () {
it("should make test number 53", function () {

	 var la = require('qminer').la;
	 // create two vectors
	 var x = new la.Vector([1, 2, 3]);
	 var y = new la.Vector([4, 5]);
	 // create the outer product of these vectors
	 var A = x.outer(y); // creates the dense matrix [[4, 5], [8, 10], [12, 15]]
	
});
});
describe("Returns the cosine between the two vectors, number 54", function () {
it("should make test number 54", function () {

	 var la = require('qminer').la;
	 // create two vectors
	 var x = new la.Vector([1, 0]);
	 var y = new la.Vector([0, 1]);
	 // calculate the cosine between those two vectors
	 var num = x.cosine(y); // returns 0
	
});
});
describe("Returns the vector as string, number 55", function () {
it("should make test number 55", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([1, 2, 3]);
	 // create vector as string
	 vec.toString(); // returns '1, 2, 3'
	
});
});
describe("Saves the vector as output stream (binary serialization), number 56", function () {
it("should make test number 56", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([1, 2, 3]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save vector and close write stream
	 vec.save(fout).close();
	
});
});
describe("Loads the vector from input stream (binary deserialization), number 57", function () {
it("should make test number 57", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.Vector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the vector
	 vec.load(fin);
	
});
});
describe("Saves the vector as output stream (ascii serialization), number 58", function () {
it("should make test number 58", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.Vector([1, 2, 3]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save matrix and close write stream
	 vec.saveascii(fout).close();
	
});
});
describe("Loads the vector from input stream (ascii deserialization), number 59", function () {
it("should make test number 59", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.Vector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the matrix
	 vec.loadascii(fin);
	
});
});
describe("Vector - array of string, number 60", function () {
it("should make test number 60", function () {

 var la = require('qminer').la;
 // create a new empty vector
 var vec = new la.StrVector();
 // create a new vector
 var vec2 = new la.StrVector(['a', 'b', 'c']);

});
});
describe("Changes the vector by removing and adding elements, number 61", function () {
it("should make test number 61", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['a', 'b', 'c']);
	 // splice the vector by removing the last two elements and adding 'd', 'e'
	 vec.splice(1, 2, 'd', 'e')// returns vector ['a', 'd', 'e']
	
});
});
describe("Sorts the vector (in place operation), number 62", function () {
it("should make test number 62", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['asd', 'z', 'kkkk']);
	 // sort ascending
	 vec.sort(); // sorts to: ['asd', 'kkkk', 'z']
	 // sort using callback
	 vec.sort(function(arg1, arg2) { return arg1.length - arg2.length; }); // sorts to: ['z', 'asd', 'kkkk']
	
});
});
describe("Deletes elements with sprecific index or more, number 63", function () {
it("should make test number 63", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['a', 'b', 'c']);
	 // trunc all elements with index 1 or more
	 vec.trunc(1); // returns vector ['a']
	
});
});
describe("Returns the vector as string, number 64", function () {
it("should make test number 64", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['a', 'b', 'c']);
	 // create vector as string
	 vec.toString(); // returns 'a, b, c'
	
});
});
describe("Saves the vector as output stream (binary serialization), number 65", function () {
it("should make test number 65", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['a', 'b', 'c']);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save vector and close write stream
	 vec.save(fout).close();
	
});
});
describe("Loads the vector from input stream (binary deserialization), number 66", function () {
it("should make test number 66", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.StrVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the vector
	 vec.load(fin);
	
});
});
describe("Saves the vector as output stream (ascii serialization), number 67", function () {
it("should make test number 67", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.StrVector(['a', 'b', 'c']);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save matrix and close write stream
	 vec.saveascii(fout).close();
	
});
});
describe("Loads the vector from input stream (ascii deserialization), number 68", function () {
it("should make test number 68", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.StrVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the matrix
	 vec.loadascii(fin);
	
});
});
describe("Vector - array of integer, number 69", function () {
it("should make test number 69", function () {

 var la = require('qminer').la;
 // create a new empty vector
 var vec = new la.IntVector();
 // create a new vector
 var vec2 = new la.IntVector([1, 2, 3]);

});
});
describe("Changes the vector by removing and adding elements, number 70", function () {
it("should make test number 70", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([1, 2, 3]);
	 // splice the vector by removing the last two elements and adding 4, 5
	 vec.splice(1, 2, 4, 5)// returns vector [1, 4, 5]
	
});
});
describe("Sorts the vector (in place operation), number 71", function () {
it("should make test number 71", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([-2, 1, 3]);
	 // sort ascending
	 vec.sort(); // sorts to: [-2, 1, 3]
	 // sort using callback
	 vec.sort(function(arg1, arg2) { return Math.abs(arg1) - Math.abs(arg2); }); // sorts to: [1, -2, 3]
	
});
});
describe("Deletes elements with sprecific index or more, number 72", function () {
it("should make test number 72", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([1, 2, 3]);
	 // trunc all elements with index 1 or more
	 vec.trunc(1); // returns vector [1]
	
});
});
describe("Returns the vector as string, number 73", function () {
it("should make test number 73", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([1, 2, 3]);
	 // create vector as string
	 vec.toString(); // returns '1, 2, 3'
	
});
});
describe("Saves the vector as output stream (binary serialization), number 74", function () {
it("should make test number 74", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([1, 2, 3]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save vector and close write stream
	 vec.save(fout).close();
	
});
});
describe("Loads the vector from input stream (binary deserialization), number 75", function () {
it("should make test number 75", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.IntVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the vector
	 vec.load(fin);
	
});
});
describe("Saves the vector as output stream (ascii serialization), number 76", function () {
it("should make test number 76", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.IntVector([1, 2, 3]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save matrix and close write stream
	 vec.saveascii(fout).close();
	
});
});
describe("Loads the vector from input stream (ascii deserialization), number 77", function () {
it("should make test number 77", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.IntVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the matrix
	 vec.loadascii(fin);
	
});
});
describe("Vector - array of boolea, number 78", function () {
it("should make test number 78", function () {

 var la = require('qminer').la;
 // create a new empty vector
 var vec = new la.BoolVector();
 // create a new vector
 var vec2 = new la.BoolVector([true, true, false]);

});
});
describe("Changes the vector by removing and adding elements, number 79", function () {
it("should make test number 79", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, true, false]);
	 // splice the vector by removing the last two elements and adding false, true
	 vec.splice(1, 2, false, true)// returns vector [true, false, true]
	
});
});
describe("Sorts the vector (in place operation), number 80", function () {
it("should make test number 80", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, false, false]);
	 // sort ascending
	 vec.sort(); // sorts to: [false, true, true]
	 // sort using callback
	 vec.sort(function(arg1, arg2) { return arg2; }); // sorts to: [false, true, true]
	
});
});
describe("Deletes elements with sprecific index or more, number 81", function () {
it("should make test number 81", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, true, false]);
	 // trunc all elements with index 1 or more
	 vec.trunc(1); // returns vector [true]
	
});
});
describe("Returns the vector as string, number 82", function () {
it("should make test number 82", function () {

	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, true, false]);
	 // create vector as string
	 vec.toString(); // returns 'true, true, false'
	
});
});
describe("Saves the vector as output stream (binary serialization), number 83", function () {
it("should make test number 83", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, true, false]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save vector and close write stream
	 vec.save(fout).close();
	
});
});
describe("Loads the vector from input stream (binary deserialization), number 84", function () {
it("should make test number 84", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.BoolVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the vector
	 vec.load(fin);
	
});
});
describe("Saves the vector as output stream (ascii serialization), number 85", function () {
it("should make test number 85", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create a new vector
	 var vec = new la.BoolVector([true, true, false]);
	 // open write stream
	 var fout = fs.openWrite('vec.dat');
	 // save matrix and close write stream
	 vec.saveascii(fout).close();
	
});
});
describe("Loads the vector from input stream (ascii deserialization), number 86", function () {
it("should make test number 86", function () {

	 // import fs module
	 var fs = require('qminer').fs;
	 var la = require('qminer').la;
	 // create an empty vector
	 var vec = new la.BoolVector();
	 // open a read stream
	 var fin = fs.openRead('vec.dat');
	 // load the matrix
	 vec.loadascii(fin);
	
});
});

});

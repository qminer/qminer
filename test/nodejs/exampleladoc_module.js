require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the ladoc_module.js file', function () {
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

});

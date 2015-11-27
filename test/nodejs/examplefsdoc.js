require('qminer').la.Vector.prototype.print = function () { };require('qminer').la.SparseVector.prototype.print = function () { };require('qminer').la.SparseMatrix.prototype.print = function () { };require('qminer').la.Matrix.prototype.print = function () { };describe('example tests for the fsdoc.js file', function () {
describe("File-system module., number 1", function () {
it("should make test number 1", function () {

 // import module
 var fs = require('qminer').fs;
 // open file in write mode
 var fout = fs.openWrite('file.txt');
 // write sync and close
 fout.writeLine('example text');
 fout.close();
 // open file in read mode
 var fin = fs.openRead('file.txt');
 // read a line
 var str = fin.readLine();

});
});
describe("Input file stream., number 2", function () {
it("should make test number 2", function () {

	 // import module
	 var fs = require('qminer').fs;
	 // open file in read mode
	 var fin = new fs.FIn('file.txt');
	 // read a line
	 var str = fin.readLine();
	
});
});
describe("Output file stream., number 3", function () {
it("should make test number 3", function () {

	 // import module
	 var fs = require('qminer').fs;
	 // open file in write mode
	 var fout = new fs.FOut('file.txt');
	 // write a line
	 fout.writeLine('example text');
	 // close
	 fout.close();
	
});
});

});

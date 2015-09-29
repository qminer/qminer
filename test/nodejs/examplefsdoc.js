describe('example tests for the fsdoc.js file', function () {
describe("File-system module.", function () {
it('should make test number 1', function () {
 this.timeout(10000); 

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
describe("Input file stream.", function () {
it('should make test number 2', function () {
 this.timeout(10000); 

	 // import module
	 var fs = require('qminer').fs;
	 // open file in read mode
	 var fin = new fs.FIn('file.txt');
	 // read a line
	 var str = fin.readLine();
	
});
});
describe("Output file stream.", function () {
it('should make test number 3', function () {
 this.timeout(10000); 

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

var fs = require('../../../build/Release/fs.node');
var assert = require('assert');

// Read the whole file if it exists 
if (fs.exists("./test.out")) {
   var fin_1 = fs.openRead("./test.out");
   var s1 = fin_1.readAll();
   console.log(s1);
}

var fout_1 = fs.openWrite("./test.out");
fout_1.writeLine("Blue note");
fout_1.close();

assert(fs.exists("./test.out"));

var fout = fs.openAppend("./test.out"); 
fout.writeLine("Soncno zimsko jutro");
fout.writeLine("QMiner");
fout.flush();
fout.close();

// Equivalent way to read a file 
var fin = new fs.FIn("./test.out");
var s = fin.readAll();
console.log(s);

// File info 
var info = fs.fileInfo("./test.out");
console.log(info);

// Delte the file 
fs.del("./test.out");

// Make sure it was deleted 
assert(!fs.exists("./test.out"));



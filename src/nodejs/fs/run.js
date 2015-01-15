var fs = require('../../../build/Release/fs.node');
var assert = require('assert');

// Read the whole file if it exists 
if (fs.exists("./test.out")) {
    var fin_1 = fs.openRead("./test.out");
    var s1 = fin_1.readAll();
    console.log(s1);
}

// Write to a file 
var fout_1 = fs.openWrite("./test.out");
fout_1.writeLine("Test line 1.");
fout_1.close();

assert(fs.exists("./test.out"));

// Append to a file 
var fout = fs.openAppend("./test.out"); 
fout.writeLine("Test line 2.");
fout.writeLine("Test line 3.");
fout.flush();
fout.close();

try {
    var fin = fs.openRead("nonExistent.file");
} catch (e) {
    console.log(e);
}

// Equivalent way to read a file 
var fin = fs.openRead("./test.out");
var s = fin.readAll();
console.log(s);

// File info 
var info = fs.fileInfo("./test.out");
console.log(info);

// Copy file
fs.copy("./test.out", "./test-copy.out");
assert(fs.exists("./test-copy.out"));

// Move file 
fs.move("./test-copy.out", "./test-moved.out");
assert(!fs.exists("./test-copy.out"));
assert(fs.exists("./test-moved.out"));

// Rename file 
fs.rename("test.out", "tset.out");
assert(fs.exists("tset.out"));
assert(!fs.exists("test.out"));

// Delete file 
fs.del("tset.out");
assert(!fs.exists("tset.out"));

// Create directory 
assert(!fs.exists("test/"));
fs.mkdir("test");
assert(fs.exists("test/"));

// Remove directory 
var delP = fs.rmdir("test");
console.log("delP = " + delP);
assert(!fs.exists("./test"));

// List all C and C++ header files in the QMiner source directory 
// var fileArr = fs.listFile("../../", "h", true);
// for (var i = 0; i < fileArr.length; ++i) {
//    console.log(fileArr[i]);
// }


/**
 * Copyright (c) 2015, Quintelligence d.o.o.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */
var fs = require('../../../').fs;
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


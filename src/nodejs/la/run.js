/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
//var la = require('../../../build/Release/la.node');
//var fs = require('../../../build/Release/fs.node');
var la = require('../../../index.js').la;
var fs = require('../../../index.js').fs;

///////////////////////////
// Return a new vector or a new matrix 
la.newVector = function(args) { return new la.Vector(args); }
la.newIntVector = function(args) { return new la.IntVector(args); }
la.newStrVector = function(args) { return new la.StrVector(args); }
la.newMatrix = function(args) { return new la.Matrix(args); }
la.newSparseVector = function(args) { return new la.SparseVector(args); }
la.newSparseMatrix = function(args) { return new la.SparseMatrix(agrs); }

var vec = la.newVector([1, 2, 3, 4, 5]);
var v2 = la.newVector(vec);

console.log(v2.norm() +  " :: " + vec.norm());

///////////////////////////
// Matrix: A very simple example 
// console.log("Creating a new matrix");

var M = la.newMatrix({"cols": 3, "rows": 3, "random": true});
console.log(M.rows + "-times-" + M.cols);

///////////////////////////
// Testing +, -, *, ... 
var u = la.newVector();
var v = la.newVector();
u.push(-1); u.push(2); u.push(3);
v.push(3); v.push(2); v.push(1);

var w = u.minus(v);

// var w = v.multiply(3.0);
// console.log(w.length);
// for (var i = 0; i < w.length; ++i) { console.log(w.at(i)); } 

console.log(" ---------------- ");
console.log(" ---------------- ");

console.log("u * v = " + u.inner(v));
console.log("||v||_2 = " + v.norm());

///////////////////////////
// Vector: A very simple example 
console.log("Creating a new vector");

var w1 = la.newVector([1, 1, 1]);
var w2 = la.newVector([2, 2, 2]);

var w = w1.plus(w2);

console.log(w.at(1));

///////////////////////////
// Vector: Catching exceptions 
try {
    console.log(w1.at(-1));
} catch (e) {
    console.log("[*** Exception ***] \"" + e + "\"");
}

///////////////////////////
// Vector: subVec
console.log("== Trying out subVec function ==");

var x = la.newVector([6, 54, 32, 13, 100]);
try {
    var z = x.subVec(w1);
} catch (e) {
    console.log("Oops: " + e);
}

// console.log("z.len = " + z.length);
var y = x.subVec([1, 2, 2, 3, 3, 3, 4, 4, 4, 4]);
console.log("y.len = " + y.length);
console.log("x.len = " + x.length);

for (var i = 0; i < y.length; ++i) { console.log('y[' + i  + '] = ' + y.at(i)); }

///////////////////////////
// Matrix: Vector multipltication 
console.log(" ---------------- ");
console.log(" ---------------- ");
console.log(" == Matrix multiply ==");
// var X = la.newMatrix([[1, 0, 0], [0, 1, 0], [0, 0, 1]]);
// var x1 = X.multiply(la.newVector([1, 2, 3]));

// console.log("x1.length = " + x1.length);
// for (var i = 0; i < x1.length; ++i) { console.log(x1.at(i)); } 

///////////////////////////
// Vector: Sorting 
var t = x.sort();
for (var i = 0; i < x.length; ++i) { console.log('x['+i+'] = '+x.at(i)); }
for (var i = 0; i < t.length; ++i) { console.log('t['+i+'] = '+t.at(i)); }

///////////////////////////
// Matrix: Sum
console.log(" == Other matrix operations ==");
var M = la.newMatrix([[1,2,3],[4,5,6],[7,8,9]]);
var N = la.newMatrix([[9,8,7],[6,5,4],[3,2,1]]);

var W = N.plus(M);

// console.log(" || W * [3,3,3] || = " + W.multiply(la.newVector([3, 3, 3])).norm());

W.setRow(2, la.newVector([1, 1, 1]));

for (var rowN = 0; rowN < W.rows; ++rowN) {
    for(var colN = 0; colN < W.cols; ++colN) {
        console.log("W["+rowN+","+colN+"] = "+W.at(rowN, colN))
    }
    console.log("\n");
}

console.log("Norm of second column of W: " + W.getCol(2).norm());
console.log(Math.sqrt(201));

var d = W.diag()
for (var i = 0; i < d.length; ++i) { console.log(d.at(i)); }

console.log("W = [\n"+W.toString()+"]");

///////////////////////////
// Sparse vector
console.log(" == Sparse vector ==");

var spVec = la.newSparseVector([[0, 1], [2, 3]]);
console.log(spVec.at(0));

spVec.put(4, 10);
console.log(spVec.at(0));

console.log("dim = " + spVec.dim);

var spWec = la.newSparseVector([[1, 3], [2, 5]]);
console.log(spWec.inner(spVec));

///////////////////////////
// Outer product 
var x1 = la.newVector([1, 2, 3]);
var x2 = la.newVector([4, 5, 6]);

var M12 = x1.outer(x2);
console.log(M12.toString());
var fout = fs.openWrite("test-mat.bin");
M12.save(fout);
fout.close();
var M12_x = new la.Matrix();
M12_x.load(fs.openRead("test-mat.bin"));
console.log(M12_x.toString());
console.log(" ==== ");
fs.del("test-mat.bin");

console.log(M12.transpose().toString());
console.log(" *** ");
console.log(M12.toString());

x1.pushV(x2); // appends x2 to x1 
console.log("xx:" + x1.toString());

try {
    x1.at(-1);
} catch (e) {
    console.log(e);
}

///////////////////////////
// Sparse matrix 
var spMatrix = x1.spDiag();
spMatrix.print();

console.log("x2.toMat().toString() = [" + x2.toMat().toString() + "]");

var v = la.newVector([1,2,3,4,5]);
console.log("v: " + v.toString());

var fout = fs.openWrite("test-vec.bin");
v.save(fout);
fout.close();
var u = la.newVector();
u.load(fs.openRead("test-vec.bin"));
console.log("u: " + u.toString());
fs.del("test-vec.bin");

// The code below crashes because V8 imposes memory limit on
// standard JS arrays. 
/*
var bigArr = new Array();
for (var i = 0; i < 5*1e7; ++i) {
    // bigVec.push(i);
    bigArr.push(i);
}
console.log(bigArr.length);

var bigArr2 = new Array();
for (var i = 0; i < 5*1e7; ++i) {
    // bigVec.push(i);
    bigArr2.push(i);
}
console.log(bigArr2.length);

var bigArr3 = new Array();
for (var i = 0; i < 5*1e7; ++i) {
    // bigVec.push(i);
    bigArr3.push(i);
}
console.log(bigArr3.length);
*/


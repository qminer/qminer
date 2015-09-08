/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
var la = require('qminer').la;
var stat = require('qminer').statistics;

/////////////////////////
// Return a new vector or a new matrix 
la.newVector = function(args) { return new la.Vector(args); }
la.newIntVector = function(args) { return new la.Vector(args); }
la.newMatrix = function(args) { return new la.Matrix(args); }
la.newSparseVector = function(args) { return new la.SparseVector(args); }
la.newSparseMatrix = function(args) { return new la.SparseMatrix(agrs); }


///////////////////////////
// TESTING stat.node

console.log("\n== Testing stat module functionalities ==\n")

var vec = la.newVector([1, 2, 3]);
var mat = la.newMatrix([[1, 2, 3], [2, 3, 4]])

console.log("Testing vector is: ")
console.log(vec.toString());

console.log("\nTesting matrix is: ")
console.log(mat.toString());

//////// TESTING MEAN ////////
console.log("== Testing stat.mean functionalities ==")

console.log("\ntesting stat.mean(vec)");
console.log(stat.mean(vec).toString()); // result should be 2
console.log(stat.mean(vec, 2).toString()); // result should be 2

console.log("\ntesting stat.mean(mat)");
console.log(stat.mean(mat).toString()); // result should be [1.5, 2.5, 3.5]
console.log(stat.mean(mat, 2).toString()); // result should be [2, 3]

console.log("\ntesting error: wrong input argument")
try {
    console.log(stat.mean(2).toString());
} catch (e) {
    console.log(e);
}

console.log("\ntesting error: no input argument")
try {
    console.log(stat.mean().toString());
} catch (e) {
    console.log(e);
}

//////// TESTING STANDARD DEVIATION //////// 
console.log("\n\n== Testing stat.std functionalities ==")

console.log("\ntesting stat.std(vec)");
console.log(stat.std(mat).toString()) // result should be [0.707107, 0.707107, 0.707107]
console.log(stat.std(mat, 0, 1).toString()) // result should be [0.707107, 0.707107, 0.707107]
console.log(stat.std(mat, 1, 1).toString()) // result should be [0.5, 0.5, 0.5]
console.log(stat.std(mat, 0, 2).toString()) // result should be [1, 1]
console.log(stat.std(mat, 1, 2).toString()) // result should be [0.816497, 0.816497]


//////// TESTING Z-SCORE //////// 
console.log("\n\n== Testing stat.zscore functionalities ==")

console.log("\ntesting stat.zscore(mat).Z");
console.log(stat.zscore(mat).Z.toString()); // result should be [[-0.707107, - 0.707107, - 0.707107], [0.707107, 0.707107, 0.707107]]
console.log(stat.zscore(mat, 0, 1).Z.toString()); // result should be [[-0.707107, - 0.707107, - 0.707107], [0.707107, 0.707107, 0.707107]]
console.log(stat.zscore(mat, 0, 2).Z.toString()); // result should be [[-1, 0, 1], [-1, 0, 1]]
console.log(stat.zscore(mat, 1, 2).Z.toString()); // result should be [[-1.22474 0 1.22474],[-1.22474 0 1.22474]]
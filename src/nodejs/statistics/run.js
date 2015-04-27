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
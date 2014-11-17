var la = require('./build/Release/la.node');

///////////////////////////
// Return a new vector or a new matrix 
la.newVector = function(args) { return new la.vector(args); }
la.newIntVector = function(args) { return new la.vector(args); }
la.newMatrix = function(args) { return new la.matrix(args); }

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
var z = x.subVec(w1);
console.log("z.len = " + z.length);
var y = x.subVec([1, 2, 2, 3, 3, 3, 4, 4, 4, 4]);
console.log("y.len = " + y.length);
console.log("x.len = " + x.length);

for (var i = 0; i < y.length; ++i) { console.log('y[' + i  + '] = ' + y.at(i)); }

///////////////////////////
// Matrix: Vector multipltication 
console.log(" ---------------- ");
console.log(" ---------------- ");
console.log(" == Matrix multiply ==");
var X = la.newMatrix([[1, 0, 0], [0, 1, 0], [0, 0, 1]]);
var x1 = X.multiply(la.newVector([1,2,3]));

console.log("x1.length = " + x1.length);
for (var i = 0; i < x1.length; ++i) { console.log(x1.at(i)); } 

///////////////////////////
// Vector: Sorting 
var t = x.sort();
for (var i = 0; i < x.length; ++i) { console.log('x['+i+'] = '+x.at(i)); }
for (var i = 0; i < t.length; ++i) { console.log('t['+i+'] = '+t.at(i)); }


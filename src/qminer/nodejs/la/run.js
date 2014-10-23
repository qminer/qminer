var la = require('./build/Release/la.node');

///////////////////////////
// Return a new vector or a new matrix 
la.newVec = function(args) { return new la.vector(args); }
la.newIntVec = function(args) { return new la.vector(args); }
la.newMat = function(args) { return new la.matrix(args); }

var vec = new la.vector([1,2,3,4,5]);
var v2 = new la.vector(vec);

console.log(v2.norm() +  " :: " + vec.norm());

///////////////////////////
// Matrix: A very simple example 
// console.log("Creating a new matrix");

var M = la.newMat({"cols": 3, "rows": 3, "random": true});
console.log(M.rows + "-times-" + M.cols);

///////////////////////////
// Testing +, -, *, ... 
var u = la.newVec();
var v = la.newVec();
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

var w1 = la.newVec([1, 1, 1]);
var w2 = la.newVec([2, 2, 2]);

// console.log("multiply");
var x1 = M.multiply(w1);
// var x2 = M.multiply(w2);

console.log("x1.length = " + x1.length);
for (var i = 0; i < x1.length; ++i) { console.log(x1.at(i)); } 

/*
 TODO (jan):
   (*) TJsVec: TInt in TFlt 
   (*) TJsRecCMp::operator(): a se da poenostavit s tanovim APIjem?
   (*) spMat.js, TJsSPMat: New() dobi pot do prototipa iz spMat.js; a se da poenostavit? 
*/


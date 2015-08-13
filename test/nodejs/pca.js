var analytics = require('qminer').analytics;
var stat = require('qminer').statistics;
var la = require('qminer').la;
var assert = require('assert');


var A = new la.Matrix([[10, 20, 110, 20, 10], [3, 4, 13, 63, 1]]);
var pca = new analytics.PCA({k : 2, iter : 1000});
pca.fit(A);
var pA = pca.transform(A);
var rA = pca.inverseTransform(pA);

// test perfect reconstruction
assert(A.minus(rA).frob() < 1e-6);

// test output
//console.log(pA.toString());
//console.log(pA.transpose().toString());
//console.log(pca.P.toString());
//console.log(pca.mu.toString());
//console.log(JSON.stringify(pca));
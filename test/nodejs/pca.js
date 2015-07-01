var analytics = require('qminer').analytics;
var stat = require('qminer').statistics;
var la = require('qminer').la;
var assert = require('assert');


var A = new la.Matrix([[10, 20, 110, 20, 10], [3, 4, 13, 63, 1]]);
var pca = new analytics.PCA();
pca.fit(A, 2, 1000);
var pA = pca.predict(A);
var rA = pca.reconstruct(pA);

// test perfect reconstruction
assert(A.minus(rA).frob() < 1e-6);
var assert = require('assert.js');
var analytics = require('analytics.js');

console.say("Regression", "Starting test based on a random sample");

// only report failours
assert.silent = process.isArg("-nopass");
// name of the debug process
assert.consoleTitle = "Regression";

var dims = 10; // dimensions
var examples = 500; // examples

var X = la.genRandomMatrix(dims, examples);
var w = la.genRandomVector(dims);
var y = X.multiplyT(w);

// gaussian noise
var noiseSigma = 0.1;
var Ng = la.genRandomMatrix(dims, examples).multiply(noiseSigma);
var Xg = X.plus(Ng);
var yg = Xg.multiplyT(w);

// ridge regression
var ridge = analytics.newRidgeRegression(0, 10);
var wg = ridge.compute(Xg.transpose(), yg);
// svm regression
var svmr = analytics.trainSvmRegression(Xg, yg, { c: 0.1, eps: 1e-8, batchSize: 100, maxIterations: 1000 });
var wgs = svmr.weights;

assert.ok(w.inner(wg) / (w.norm() * wg.norm()) > 0.999 , 'ridge regression test: cos(w, wg) > 0.999');
assert.ok(w.inner(wgs) / (w.norm() * wgs.norm()) > 0.999 , 'svm regression test: cos(w, wgs) > 0.999');

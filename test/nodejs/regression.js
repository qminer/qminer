/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

console.log(__filename)
var qm = require('qminer');
var fs = qm.fs;
var assert = require('assert');
var analytics = qm.analytics;
var la = qm.la;

console.log("Regression", "Starting test based on a random sample");

var dims = 10; // dimensions
var examples = 500; // examples

var X = la.randn(dims, examples);
var w = la.randn(dims);
var y = X.multiplyT(w);

// gaussian noise
var noiseSigma = 0.1;
var Ng = la.randn(dims, examples).multiply(noiseSigma);
var Xg = X.plus(Ng);
var yg = Xg.multiplyT(w);

// ridge regression
var ridge = new analytics.RidgeRegression(0, 10);
var wg = ridge.compute(Xg.transpose(), yg);
// svm regression
var svmr = new analytics.SVR({ c: 0.1, eps: 1e-8, batchSize: 100, maxIterations: 1000 });
svmr.fit(Xg, yg);
var wgs = svmr.weights;

assert(w.inner(wg) / (w.norm() * wg.norm()) > 0.99 , 'ridge regression test: cos(w, wg) > 0.99: ' + w.inner(wg) / (w.norm() * wg.norm()));
assert(w.inner(wgs) / (w.norm() * wgs.norm()) > 0.99 , 'svm regression test: cos(w, wgs) > 0.99: ' + w.inner(wgs) / (w.norm() * wgs.norm()));
console.log('done regression');

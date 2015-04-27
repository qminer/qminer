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

assert(w.inner(wg) / (w.norm() * wg.norm()) > 0.999 , 'ridge regression test: cos(w, wg) > 0.999');
assert(w.inner(wgs) / (w.norm() * wgs.norm()) > 0.999 , 'svm regression test: cos(w, wgs) > 0.999');
console.log('done regression');

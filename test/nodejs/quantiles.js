/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var qm = require('qminer');
var analytics = qm.analytics;
var fs = qm.fs;
var assert = require("../../src/nodejs/scripts/assert.js");

describe("TDigest test", function () {

    describe("Constructor test", function () {
        it("should return a default constructor", function () {
            var tdigest = new analytics.TDigest();
            var params = tdigest.getParams();

            assert.equal(params.seed, 0, "invalid seed");
            assert.equal(params.clusters, 100, "invalid number of clusters");
            assert.equal(params.minEps, 1e-4, "invalid max accuracy");
            assert.equal(params.compression, "never", "invalid compression");
            assert(tdigest.init === false);
        });
        it("should construct using provided parameters", function () {
            var tdigest = new analytics.TDigest({ seed: 1, clusters: 50, minEps: 1e-5, compression: "periodic" });
            var params = tdigest.getParams();

            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.clusters, 50, "invalid number of clusters");
            assert.equal(params.minEps, 1e-5, "invalid max accuracy");
            assert.equal(params.compression, "periodic", "invalid compression");
            assert(tdigest.init === false);
        });
    });

    describe("Fit/predict test", function () {
        it("It should return a the prediction for the given fit", function () {
            var tdigest = new analytics.TDigest();
            assert(tdigest.init === false);
            var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
            for (var i = 0; i < inputs.length; i++) {
                tdigest.partialFit(inputs[i]);
                assert(tdigest.init === true);
            }
            var pred = tdigest.predict(0.1);
            // TODO test
        });
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var tdigest = new analytics.TDigest({ seed: 1, clusters: 50, minEps: 1e-5 });
            tdigest.save(fs.openWrite('tdigest.bin')).close();
            var tdigest2 = new analytics.TDigest(fs.openRead('tdigest.bin'));
            var params = tdigest2.getParams();
            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.clusters, 50, "invalid number of clusters");
            assert.equal(params.minEps, 1e-5, "invalid max accuracy");
            assert(tdigest.init === false);
        })
    });
});

describe('CountWindowGK test', function () {
    describe('Constructor test', function () {
        it('should set correct parameters', function () {
            var gk = new analytics.CountWindowGk({
                windowSize: 10,
                quantileEps: 0.1,
                countEps: 1e-6
            })

            var params = gk.getParams();
            assert.equal(params.windowSize, 10);
            assert.equal(params.quantileEps, 0.1);
            assert.equal(params.countEps, 1e-6);
        })
    })

    describe('Serialization test', function () {
        var batchSize = 1000;
        var nbatches = 10;

        var windowSize = batchSize;
        var quantileEps = .01;
        var countEps = .001;

        var gk = new analytics.CountWindowGk({
            windowSize: windowSize,
            quantileEps: quantileEps,
            countEps: countEps
        })

        var maxRelErr = quantileEps + 2*countEps;

        var vals = [];
        for (var i = 0; i < batchSize; i++) {
            vals.push(i);
        }
        for (var batchN = 0; batchN < nbatches; batchN++) {
            // shuffle the array
            for (var i = 0; i < batchSize; i++) {
                var swapN = Math.floor(Math.random()*batchSize);
                var temp = vals[i];
                vals[i] = vals[swapN];
                vals[swapN] = temp;
            }

            for (var i = 0; i < batchSize; i++) {
                gk.partialFit(vals[i]);
            }

            for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
                var quant_hat = gk.predict(cumProb);
                assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
                assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
            }
        }

        gk.save(qm.fs.openWrite('gk.dat')).close();
        var gk1 = new analytics.CountWindowGk(qm.fs.openRead('gk.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.predict(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

describe('Gk test', function () {
    describe('Constructor test', function () {
        it('should set correct default parameters', function () {
            var gk = new analytics.Gk();

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.autoCompress != null);

            assert.equal(params.eps, 0.01);
            assert.equal(params.autoCompress, true);
        })
        it('should set correct parameters', function () {
            var gk = new analytics.Gk({
                eps: 0.1,
                autoCompress: false
            })

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.autoCompress != null);

            assert.equal(params.eps, 0.1);
            assert.equal(params.autoCompress, false);
        })
    })

    describe('Serialization test', function () {
        var batchSize = 1000;
        var nbatches = 10;

        var eps = .01;

        var gk = new analytics.Gk({
            eps: eps
        })

        var maxRelErr = eps;

        var targets = [];
        for (var prob = 0; prob <= 1; prob += 0.001) {
            targets.push(prob);
        }

        var vals = [];
        for (var i = 0; i < batchSize; i++) {
            vals.push(i);
        }
        for (var batchN = 0; batchN < nbatches; batchN++) {
            // shuffle the array
            for (var i = 0; i < batchSize; i++) {
                var swapN = Math.floor(Math.random()*batchSize);
                var temp = vals[i];
                vals[i] = vals[swapN];
                vals[swapN] = temp;
            }

            for (var i = 0; i < batchSize; i++) {
                gk.partialFit(vals[i]);
            }

            var quants = gk.predict(targets);
            for (var targetN = 0; targetN < targets.length; targetN++) {
                var prob = targets[targetN];
                var quant = gk.predict(prob);
                assert.equal(quant, quants[targetN]);
                assert(Math.floor((prob - maxRelErr)*batchSize) <= quant);
                assert(Math.ceil((prob + maxRelErr)*batchSize) >= quant);
            }
        }

        gk.save(qm.fs.openWrite('gk-orig.dat')).close();
        var gk1 = new analytics.Gk(qm.fs.openRead('gk-orig.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.predict(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

describe('BiasedGk test', function () {
    describe('Constructor test', function () {
        it('should set correct default parameters', function () {
            var gk = new analytics.BiasedGk();

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.targetProb != null);
            assert(params.compression != null);
            assert(params.useBands != null);

            assert.equal(params.eps, 0.1);
            assert.equal(params.targetProb, 0.01);
            assert.equal(params.compression, "periodic");
            assert.equal(params.useBands, true);
        })
        it('should set correct parameters', function () {
            var gk = new analytics.BiasedGk({
                eps: 0.05,
                targetProb: 0.99,
                compression: "aggressive",
                useBands: false
            })

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.targetProb != null);
            assert(params.compression != null);
            assert(params.useBands != null);

            assert.equal(params.eps, 0.05);
            assert.equal(params.targetProb, 0.99);
            assert.equal(params.compression, "aggressive");
            assert.equal(params.useBands, false);
        })
    })

    describe('Serialization test', function () {
        var batchSize = 1000;
        var nbatches = 10;

        var eps = .1;
        var targetProb = 0.01;

        var gkLow = new analytics.BiasedGk({
            eps: eps,
            targetProb: targetProb
        })
        var gkHigh = new analytics.BiasedGk({
            eps: eps,
            targetProb: 1 - targetProb
        })

        var isErrorInRangeLow = function (prob, quant) {
            var p = Math.max(prob, targetProb);
            var mxRelError = p*eps;
            var mxError = Math.ceil(mxRelError*batchSize);
            var error = Math.abs(quant - prob*batchSize);
            return error <= mxError + 1e-9;
        }
        var isErrorInRangeHigh = function (prob, quant) {
            var p = Math.max(1 - prob, 1 - targetProb);
            var mxRelError = p*eps;
            var mxError = Math.ceil(mxRelError*batchSize);
            var error = Math.abs(quant - prob*batchSize);
            return error <= mxError + 1e-9;
        }

        var targets = [];
        for (var prob = 0; prob <= 1; prob += 0.001) {
            targets.push(prob);
        }

        var vals = [];
        for (var i = 0; i < batchSize; i++) {
            vals.push(i);
        }
        for (var batchN = 0; batchN < nbatches; batchN++) {
            // shuffle the array
            for (var i = 0; i < batchSize; i++) {
                var swapN = Math.floor(Math.random()*batchSize);
                var temp = vals[i];
                vals[i] = vals[swapN];
                vals[swapN] = temp;
            }

            for (var i = 0; i < batchSize; i++) {
                gkLow.partialFit(vals[i]);
                gkHigh.partialFit(vals[i]);
            }

            var quantsLow = gkLow.predict(targets);
            var quantsHigh = gkHigh.predict(targets);

            for (var targetN = 0; targetN < targets.length; targetN++) {
                var prob = targets[targetN];

                var quantLow = gkLow.predict(prob);
                var quantHigh = gkHigh.predict(prob);

                assert.equal(quantLow, quantsLow[targetN]);
                assert.equal(quantHigh, quantsHigh[targetN]);

                assert(isErrorInRangeLow(prob, quantLow));
                assert(isErrorInRangeHigh(prob, quantHigh));
            }
        }

        gkLow.save(qm.fs.openWrite('gkLow-orig.dat')).close();
        gkHigh.save(qm.fs.openWrite('gkHigh-orig.dat')).close();

        var gkLow1 = new analytics.BiasedGk(qm.fs.openRead('gkLow-orig.dat'));
        var gkHigh1 = new analytics.BiasedGk(qm.fs.openRead('gkHigh-orig.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quantLow = gkLow1.predict(cumProb);
            var quantHigh = gkHigh1.predict(cumProb);

            assert(isErrorInRangeLow(cumProb, quantLow));
            assert(isErrorInRangeHigh(cumProb, quantHigh));
        }

        var params = gkLow1.getParams();
        assert(params.eps != null);
        assert(params.targetProb != null);
        assert(params.compression != null);
        assert(params.useBands != null);

        assert.equal(params.eps, 0.1);
        assert.equal(params.targetProb, 0.01);
        assert.equal(params.compression, "periodic");
        assert.equal(params.useBands, true);
    })
})

describe('TimeWindowGk test', function () {
    describe('Constructor test', function () {
        it('should set correct parameters', function () {
            var gk = new analytics.TimeWindowGk({
                window: 9999,
                quantileEps: 0.1,
                countEps: 1e-6
            })

            var params = gk.getParams();
            assert.equal(params.window, 9999);
            assert.equal(params.quantileEps, 0.1);
            assert.equal(params.countEps, 1e-6);
        })
    })

    describe('Serialization test', function () {
        var batchSize = 1000;
        var nbatches = 10;

        var dt = 10;
        var windowSize = batchSize*dt;
        var quantileEps = .01;
        var countEps = .001;

        var gk = new analytics.TimeWindowGk({
            window: windowSize,
            quantileEps: quantileEps,
            countEps: countEps
        })

        var maxRelErr = quantileEps + 2*countEps;

        var vals = [];
        for (var i = 0; i < batchSize; i++) {
            vals.push(i);
        }
        for (var batchN = 0; batchN < nbatches; batchN++) {
            // shuffle the array
            for (var i = 0; i < batchSize; i++) {
                var swapN = Math.floor(Math.random()*batchSize);
                var temp = vals[i];
                vals[i] = vals[swapN];
                vals[swapN] = temp;
            }

            for (var i = 0; i < batchSize; i++) {
                var time = (batchN*batchSize + i)*dt;
                gk.partialFit(time, vals[i]);
            }

            for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
                var quant_hat = gk.predict(cumProb);
                assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
                assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
            }
        }

        gk.save(qm.fs.openWrite('gk-time.dat')).close();
        var gk1 = new analytics.CountWindowGk(qm.fs.openRead('gk-time.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.predict(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

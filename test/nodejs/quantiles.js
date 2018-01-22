/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var qm = require('qminer');
var quants = qm.analytics.quantiles;
var fs = qm.fs;
var assert = require("../../src/nodejs/scripts/assert.js");

describe("TDigest test", function () {

    describe("Constructor test", function () {
        it("should return a default constructor", function () {
            var tdigest = new quants.TDigest();
            var params = tdigest.getParams();

            assert.equal(params.seed, 0, "invalid seed");
            assert.equal(params.clusters, 100, "invalid number of clusters");
            assert.equal(params.minEps, 1e-4, "invalid max accuracy");
            assert.equal(params.compression, "never", "invalid compression");
            assert(tdigest.init === false); });
        it("should construct using provided parameters", function () {
            var tdigest = new quants.TDigest({ seed: 1, clusters: 50, minEps: 1e-5, compression: "periodic" });
            var params = tdigest.getParams();

            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.clusters, 50, "invalid number of clusters");
            assert.equal(params.minEps, 1e-5, "invalid max accuracy");
            assert.equal(params.compression, "periodic", "invalid compression");
            assert(tdigest.init === false);
        });
    });

    describe("Fit/quantile test", function () {
        it("It should return a the prediction for the given fit", function () {
            var tdigest = new quants.TDigest();
            assert(tdigest.init === false);
            var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
            for (var i = 0; i < inputs.length; i++) {
                tdigest.insert(inputs[i]);
                assert(tdigest.init === true);
            }
            var pred = tdigest.quantile(0.1);
            // TODO test
        });
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var tdigest = new quants.TDigest({ seed: 1, clusters: 50, minEps: 1e-5 });
            tdigest.save(fs.openWrite('tdigest.bin')).close();
            var tdigest2 = new quants.TDigest(fs.openRead('tdigest.bin'));
            var params = tdigest2.getParams();
            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.clusters, 50, "invalid number of clusters");
            assert.equal(params.minEps, 1e-5, "invalid max accuracy");
            assert(tdigest.init === false);
        })
    });
});

describe("BufferedTDigest test", function () {
    describe("Constructor test", function () {
        it("should return a default constructor", function () {
            var tdigest = new quants.BufferedTDigest();
            var params = tdigest.getParams();

            assert.equal(params.seed, 0, "invalid seed");
            assert.equal(params.delta, 100, "invalid delta");
            assert.equal(params.bufferLen, 1000, "invalid buffer length: " + params.bufferLen);
            assert(tdigest.init === false);
        });
        it("should construct using provided parameters", function () {
            var tdigest = new quants.BufferedTDigest({
                seed: 1,
                delta: 50,
                bufferLen: 500
            });
            var params = tdigest.getParams();

            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.delta, 50, "invalid delta");
            assert.equal(params.bufferLen, 500, "invalid buffer length: " + params.bufferLen);
            assert(tdigest.init === false);
        });
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var tdigest = new quants.BufferedTDigest({
                seed: 1,
                delta: 50,
                bufferLen: 500
            });
            tdigest.save(fs.openWrite('bufftdigest.bin')).close();
            var tdigest2 = new quants.BufferedTDigest(fs.openRead('bufftdigest.bin'));
            var params = tdigest2.getParams();
            assert.equal(params.seed, 1, "invalid seed");
            assert.equal(params.delta, 50, "invalid delta");
            assert.equal(params.bufferLen, 500, "invalid buffer length: " + params.bufferLen);
            assert(tdigest.init === false);
        })
    });

    describe('Testing buffer', function () {
        it('should auto flush at correct time', function () {
            var buffLen = 500;
            var tdigest = new quants.BufferedTDigest({
                seed: 1,
                clusters: 50,
                bufferLen: buffLen
            });

            for (var i = 0; i < buffLen-1; ++i) {
                tdigest.insert(3 + Math.random());
                var val = tdigest.quantile(0);
                assert.equal(val, 0);
            }

            tdigest.insert(3 + Math.random());
            assert(tdigest.quantile(0) > 0)
        })

        it('should flush manually at correct time', function () {
            var buffLen = 500;
            var tdigest = new quants.BufferedTDigest({
                seed: 1,
                clusters: 50,
                bufferLen: buffLen
            });

            for (var i = 0; i < 100; ++i) {
                tdigest.insert(3 + Math.random());
                var val = tdigest.quantile(0);
                assert.equal(val, 0);
            }

            tdigest.flush();
            assert(tdigest.quantile(0) > 0)
        })
    })
});

describe('CountWindowGK test', function () {
    describe('Constructor test', function () {
        it('should set correct parameters', function () {
            var gk = new quants.CountWindowGk({
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

        var gk = new quants.CountWindowGk({
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
                gk.insert(vals[i]);
            }

            for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
                var quant_hat = gk.quantile(cumProb);
                var minVal = Math.floor((cumProb - maxRelErr)*batchSize);
                var maxVal = Math.ceil((cumProb + maxRelErr)*batchSize);

                assert(minVal <= quant_hat);
                assert(maxVal >= quant_hat);
            }
        }

        gk.save(qm.fs.openWrite('gk.dat')).close();
        var gk1 = new quants.CountWindowGk(qm.fs.openRead('gk.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.quantile(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

describe('Gk test', function () {
    describe('Constructor test', function () {
        it('should set correct default parameters', function () {
            var gk = new quants.Gk();

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.autoCompress != null);
            assert(params.useBands != null);

            assert.equal(params.eps, 0.01);
            assert.equal(params.autoCompress, true);
            assert.equal(params.useBands, true);
        })
        it('should set correct parameters', function () {
            var gk = new quants.Gk({
                eps: 0.1,
                autoCompress: false,
                useBands: false
            })

            var params = gk.getParams();
            assert(params.eps != null);
            assert(params.autoCompress != null);
            assert(params.useBands != null);

            assert.equal(params.eps, 0.1);
            assert.equal(params.autoCompress, false);
            assert.equal(params.useBands, false);
        })
    })

    describe('Serialization test', function () {
        var batchSize = 1000;
        var nbatches = 10;

        var eps = .01;

        var gk = new quants.Gk({
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
                gk.insert(vals[i]);
            }

            var quantiles = gk.quantile(targets);
            for (var targetN = 0; targetN < targets.length; targetN++) {
                var prob = targets[targetN];
                var quant = gk.quantile(prob);
                assert.equal(quant, quantiles[targetN]);
                assert(Math.floor((prob - maxRelErr)*batchSize) <= quant);
                assert(Math.ceil((prob + maxRelErr)*batchSize) >= quant);
            }
        }

        gk.save(qm.fs.openWrite('gk-orig.dat')).close();
        var gk1 = new quants.Gk(qm.fs.openRead('gk-orig.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.quantile(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

describe('BiasedGk test', function () {
    describe('Constructor test', function () {
        it('should set correct default parameters', function () {
            var gk = new quants.BiasedGk();

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
            var gk = new quants.BiasedGk({
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

        var gkLow = new quants.BiasedGk({
            eps: eps,
            targetProb: targetProb
        })
        var gkHigh = new quants.BiasedGk({
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
                gkLow.insert(vals[i]);
                gkHigh.insert(vals[i]);
            }

            var quantsLow = gkLow.quantile(targets);
            var quantsHigh = gkHigh.quantile(targets);

            for (var targetN = 0; targetN < targets.length; targetN++) {
                var prob = targets[targetN];

                var quantLow = gkLow.quantile(prob);
                var quantHigh = gkHigh.quantile(prob);

                assert.equal(quantLow, quantsLow[targetN]);
                assert.equal(quantHigh, quantsHigh[targetN]);

                assert(isErrorInRangeLow(prob, quantLow));
                assert(isErrorInRangeHigh(prob, quantHigh));
            }
        }

        gkLow.save(qm.fs.openWrite('gkLow-orig.dat')).close();
        gkHigh.save(qm.fs.openWrite('gkHigh-orig.dat')).close();

        var gkLow1 = new quants.BiasedGk(qm.fs.openRead('gkLow-orig.dat'));
        var gkHigh1 = new quants.BiasedGk(qm.fs.openRead('gkHigh-orig.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quantLow = gkLow1.quantile(cumProb);
            var quantHigh = gkHigh1.quantile(cumProb);

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
            var gk = new quants.TimeWindowGk({
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

        var gk = new quants.TimeWindowGk({
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
                gk.insert(time, vals[i]);
            }

            for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
                var quant_hat = gk.quantile(cumProb);
                assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
                assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
            }
        }

        gk.save(qm.fs.openWrite('gk-time.dat')).close();
        var gk1 = new quants.CountWindowGk(qm.fs.openRead('gk-time.dat'));

        for (var cumProb = 0; cumProb <= 1; cumProb += 0.001) {
            var quant_hat = gk1.quantile(cumProb);
            assert(Math.floor((cumProb - maxRelErr)*batchSize) <= quant_hat);
            assert(Math.ceil((cumProb + maxRelErr)*batchSize) >= quant_hat);
        }
    })
})

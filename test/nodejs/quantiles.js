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

            assert.equal(params.minCount, 0);
            assert.equal(params.clusters, 100);
            assert(tdigest.init);
        });
        it("should construct using provided parameters", function () {
            var tdigest = new analytics.TDigest({ minCount: 1, clusters: 50 });
            var params = tdigest.getParams();

            assert.equal(params.minCount, 1);
            assert.equal(params.clusters, 50);
            assert(!tdigest.init);
        });
    });

    describe("Fit/predict test", function () {
        it("It should return a the prediction for the given fit", function () {
            var tdigest = new analytics.TDigest();
            var inputs = [10, 1, 2, 8, 9, 5, 6, 4, 7, 3];
            for (var i = 0; i < inputs.length; i++) {
                tdigest.partialFit(inputs[i]);
            }
            var pred = tdigest.predict(0.1);
            // TODO test
        });
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var tdigest = new analytics.TDigest({ minCount: 1, clusters: 50 });
            tdigest.save(fs.openWrite('tdigest.bin')).close();
            var tdigest2 = new analytics.TDigest(fs.openRead('tdigest.bin'));
            var params = tdigest2.getParams();
            assert.equal(params.minCount, 1);
            assert.equal(params.clusters, 50);
            assert(!tdigest2.init);
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

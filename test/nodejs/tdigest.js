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
        it("It should return a default constructor", function () {
            var tdigest = new analytics.TDigest();
            var params = tdigest.getParams();
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

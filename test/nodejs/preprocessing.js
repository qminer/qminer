/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

///////////////////////////////////////////////////////////
// Preprocessing Unit Tests

var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('../../index.js').analytics;
var la = require('../../index.js').la;

describe('Preprocessing Tests', function () {
    describe('Binarize Tests', function () {
        it('should not throw an exception', function () {
            var vec = new la.Vector([0, 1, 0, 2, 1, 1, 1, 3]);
            assert.doesNotThrow(function () {
                var binarizer = new analytics.preprocessing.Binarizer(1);
                var bin = binarizer.transform(vec);
            });
        })
        it('should throw an exception if there are no parameters given', function () {
            assert.throws(function () {
                var binarizer = new analytics.preprocessing.Binarizer();
            });
        })
        it('should return the binarized vector, la.Vector', function () {
            var vec = new la.Vector([0, 1, 1, 0, 2]);
            var binarizer = new analytics.preprocessing.Binarizer(1);
            var bin = binarizer.transform(vec);
            assert.equal(bin[0], -1);
            assert.equal(bin[1], 1);
            assert.equal(bin[2], 1);
            assert.equal(bin[3], -1);
            assert.equal(bin[4], -1);
        })
        it('should return the binarized vector, la.IntVector', function () {
            var vec = new la.IntVector([0, 1, 1, 0, 2]);
            var binarizer = new analytics.preprocessing.Binarizer(1);
            var bin = binarizer.transform(vec);
            assert.equal(bin[0], -1);
            assert.equal(bin[1], 1);
            assert.equal(bin[2], 1);
            assert.equal(bin[3], -1);
            assert.equal(bin[4], -1);
        })
        it('should return the binarized vector, la.StrVector', function () {
            var vec = new la.StrVector(['one', 'two', 'three', 'four']);
            var binarizer = new analytics.preprocessing.Binarizer('two');
            var bin = binarizer.transform(vec);
            assert.equal(bin[0], -1);
            assert.equal(bin[1], 1);
            assert.equal(bin[2], -1);
            assert.equal(bin[3], -1);
        })
        it('should return the binarized vector, la.BoolVector', function () {
            var vec = new la.BoolVector([true, false, false, true]);
            var binarizer = new analytics.preprocessing.Binarizer(true);
            var bin = binarizer.transform(vec);
            assert.equal(bin[0], 1);
            assert.equal(bin[1], -1);
            assert.equal(bin[2], -1);
            assert.equal(bin[3], 1);
        })
        it('should return an empty vector, la.SparseVector', function () {
            var vec = new la.SparseVector([[0, 1], [3, 5], [4, 2]], 6);
            var binarizer = new analytics.preprocessing.Binarizer(1);
            var bin = binarizer.transform(1);
            assert.equal(bin.length, 0);
        })
        it('should return an empty vector, if the matching index is not of type of vector', function () {
            var vec = new la.IntVector([0, 1, 2, 3]);
            var binarizer = new analytics.preprocessing.Binarizer('str');
            var bin = binarizer.transform(vec);
            assert.equal(bin.length, 4);
            assert.equal(bin[0], -1);
            assert.equal(bin[1], -1);
            assert.equal(bin[2], -1);
            assert.equal(bin[3], -1);
        })
        it('should return the binarized vector, la.IntVector, index is true', function () {
            var vec = new la.IntVector([0, 1, 2, 3]);
            var binarizer = new analytics.preprocessing.Binarizer(true);
            var bin = binarizer.transform(vec);
            assert.equal(bin[0], -1);
            assert.equal(bin[1], -1);
            assert.equal(bin[2], -1);
            assert.equal(bin[3], -1);
        })
    });
});

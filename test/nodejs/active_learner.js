/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var qm = require('../../index.js');
var la = qm.la;
var AL = qm.analytics.ActiveLearner;
var assert = require("../../src/nodejs/scripts/assert.js");

describe("Active learning tests", function () {
    describe("Constructor test", function () {
        it("should return a default constructor", function () {
            var al = new AL();
            assert(al._X == null);
            assert.strictEqual(al._y.size, 0);
            assert.strictEqual(al._settings.SVC.c, 1);
            assert.strictEqual(al._settings.SVC.j, 1);
            assert.strictEqual(al._settings.SVC.algorithm, "LIBSVM");
        });

    });

    describe("Labelled and unlabelled set test", function () {
        it("should correctly return labelled and unlabelled set", function () {
            var al = new AL();

            var X = new la.Matrix([
                [-1, 1],
                [-1, 0],
                [-1, -1],
                [1, 1],
                [1, 0],
                [1, -1]
            ]).transpose(); // column examples
            al.setX(X);

            var y = new Map();
            y.set(0, -1);
            y.set(2, -1);
            y.set(3, 1);
            y.set(5, 1);
            al.sety(y);

            assert.deepEqual(al._getLabIdxArr(al._y), [0, 2, 3, 5]);
            assert.deepEqual(al._getLabArr(al._y), [-1, -1, 1, 1]);
            assert.deepEqual(al._getUnlabIdxArr(al._X.cols, al._y), [1, 4]);
        });
    });
    describe("AL loop test", function () {
        it("should query the example nearer to the hyperplane", function () {
            var al = new AL();

            var X = new la.Matrix([
                [-2, 1],
                [-2, 0],
                [-2, -1],
                [0, 1],
                [-0.9, 0],
                [0, -1]
            ]).transpose(); // column examples
            al.setX(X);

            var y = new la.IntVector([-1, 0, -1, 1, 0, 1]);
            al.sety(y);

            var qidx = al.getQueryIdx(2);
            var qidx2 = al.getQueryIdx(3);
            assert.deepEqual(qidx, [4, 1]);
            assert.deepEqual(qidx2, [4, 1]);
        });
        it("should query the only example left", function () {
            var al = new AL();

            var X = new la.Matrix([
                [-2, 1],
                [-2, 0],
                [-2, -1],
                [0, 1],
                [-0.9, 0],
                [0, -1]
            ]).transpose(); // column examples
            al.setX(X);

            var y = [-1, 0, -1, 1, 0, 1];
            al.sety(y);
            al.setLabel(4, 1);

            var qidx = al.getQueryIdx(2);
            assert.deepEqual(qidx, [1]);
        });

    });
    describe("Save/load test", function () {
        it("should save and load an empty model", function (done) {
            var al = new AL();
            al.save("active_learner.bin");
            var al2 = new AL("active_learner.bin");
            assert(al2._X == null);
            assert.strictEqual(al2._y.size, 0);
            assert.strictEqual(al2._settings.SVC.c, 1);
            assert.strictEqual(al2._settings.SVC.j, 1);
            assert.strictEqual(al2._settings.SVC.algorithm, "LIBSVM");
            done();
        });
        it("should save and load a model after some data was processed", function (done) {
            var al = new AL();
            var X = new la.Matrix([
                [-2, 1],
                [-2, 0],
                [-2, -1],
                [0, 1],
                [-0.9, 0],
                [0, -1]
            ]).transpose(); // column examples
            al.setX(X);

            var y = [-1, 0, -1, 1, 0, 1];
            al.sety(y);
            al.setLabel(4, 1);

            var qidx = al.getQueryIdx(2);
            assert.deepEqual(qidx, [1]);

            al.save("active_learner.bin");
            var al2 = new AL("active_learner.bin");
            var qidx2 = al2.getQueryIdx(2);
            assert.deepEqual(qidx2, [1]);
            done();
        });

    });
});
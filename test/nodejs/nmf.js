/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var la = require('../../index.js').la;
var analytics = require('../../index.js').analytics;
var assert = require("../../src/nodejs/scripts/assert.js");

// Nmf Unit Tests
describe("NMF tests", function () {

    it("should calculate the NMF with the default parameters, dense matrix", function () {
        var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
        var nmf = analytics.nmf(mat, 2);
        assert.equal(nmf.U.rows, 4);
        assert.equal(nmf.U.cols, 2);
        assert.equal(nmf.V.rows, 2);
        assert.equal(nmf.V.cols, 4);
    });
    it("should calculate the NMF with the given parameters, dense matrix", function () {
        var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
        var nmf = analytics.nmf(mat, 4, { tol: 1e-6, iter: 100000 });
        assert.equal(nmf.U.rows, 4);
        assert.equal(nmf.U.cols, 4);
        assert.equal(nmf.V.rows, 4);
        assert.equal(nmf.V.cols, 4);
    });
    it("should calculate the NMF with the default parameters, sparse matrix", function () {
        var mat = new la.SparseMatrix([[[0, 1]], [[0, 5], [1, 3], [2, 5]], [[3, 4]], [[0, 1], [2, 3], [3, 1]]]);
        var nmf = analytics.nmf(mat, 2);
        assert.equal(nmf.U.rows, 4);
        assert.equal(nmf.U.cols, 2);
        assert.equal(nmf.V.rows, 2);
        assert.equal(nmf.V.cols, 4);
    })
    it("should calculate the NMF with the given parameters, sparse matrix", function () {
        var mat = new la.SparseMatrix([[[0, 1]], [[0, 5], [1, 3], [2, 5]], [[3, 4]], [[0, 1], [2, 3], [3, 1]]]);
        var nmf = analytics.nmf(mat, 4, { tol: 1e-6, iter: 100000 });
        assert.equal(nmf.U.rows, 4);
        assert.equal(nmf.U.cols, 4);
        assert.equal(nmf.V.rows, 4);
        assert.equal(nmf.V.cols, 4);
    });
    it("should throw an exception if there are no parameters given", function () {
        assert.throws(function () {
            var nmf = analytics.nmf();
        });
    })
    it("should throw an exception if there is no reduced rank given", function () {
        var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
        assert.throws(function () {
            var nmf = analytics.nmf(mat);
        });
    })
});
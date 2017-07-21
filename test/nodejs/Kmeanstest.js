/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var qm = require("qminer");
var la = qm.la;
var analytics = qm.analytics;
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for Kmeans

describe("Kmeans test", function () {

    describe("Constructor test", function () {
        it("should return empty parameter values", function () {
            var KMeans = new analytics.KMeans();
            var params = KMeans.getParams();
            assert.equal(Object.keys(params).length, 7);
        });
        it("should return parameter values", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, verbose: false });
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 2);
            assert.equal(params.distanceType, "Euclid");
            assert.equal(params.centroidType, "Dense");
            assert.equal(params.calcDistQual, false);
            assert.equal(params.verbose, false);
        });
        it("should return the custom KMeans", function () {
            var KMeans = new analytics.KMeans({ iter: 20000, k: 100, verbose: true, distanceType: "Cos", centroidType: "Sparse" });
            var params = KMeans.getParams();
            assert.equal(params.iter, 20000);
            assert.equal(params.k, 100);
            assert.equal(params.distanceType, "Cos");
            assert.equal(params.centroidType, "Sparse");
            assert.equal(params.verbose, true);
        });
        it("should return the custom KMeans with fitIdx", function () {
            var KMeans = new analytics.KMeans({ iter: 20000, k: 2, verbose: true, distanceType: "Cos", centroidType: "Sparse", fitIdx: [5, 2], calcDistQual: true });
            var params = KMeans.getParams();
            assert.equal(params.iter, 20000);
            assert.equal(params.k, 2);
            assert.equal(params.distanceType, "Cos");
            assert.equal(params.centroidType, "Sparse");
            assert.equal(params.calcDistQual, true);
            assert.equal(params.verbose, true);
            assert.equal(params.fitIdx[0], 5);
            assert.equal(params.fitIdx[1], 2);
        });
    });
    describe("Testing getParams and setParams", function () {
        it("should return the changed values of parameters", function () {
            var KMeans = new analytics.KMeans();
            KMeans.setParams({ iter: 15, k: 30, verbose: true });
            var params = KMeans.getParams();
            assert.equal(params.iter, 15);
            assert.equal(params.k, 30);
            assert.equal(params.verbose, true);
        });
        it("should return the same parameters used for the construction", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 20, fitIdx: [0, 1, 2] });
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 20);
            assert.equal(params.verbose, false);
            assert.equal(params.fitIdx[0], 0);
            assert.equal(params.fitIdx[1], 1);
            assert.equal(params.fitIdx[2], 2);
        });
        it("should return the same parameters used for the construction", function () {
            var mat = new la.Matrix([[0, 1], [2, 3]]);
            var KMeans = new analytics.KMeans({ iter: 100, k: 20, fitStart: { C: mat } });
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 20);
            assert.equal(params.verbose, false);
            assert.equal(params.fitStart.C.rows, 2);
            assert.equal(params.fitStart.C.cols, 2);
            assert.equal(params.fitStart.C.at(0, 0), 0);
            assert.equal(params.fitStart.C.at(0, 1), 1);
            assert.equal(params.fitStart.C.at(1, 0), 2);
            assert.equal(params.fitStart.C.at(1, 1), 3);
        });
        it("should return the changed values of parameters even if added keys", function () {
            var KMeans = new analytics.KMeans();
            KMeans.setParams({ iter: 10, k: 5, alpha: false });
            var params = KMeans.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.k, 5);
            assert.equal(params.verbose, false);
        });
    });
    describe("Fit test", function () {
        it("should create the model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans();
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with fitIdx, dense matrix", function () {
            var KMeans = new analytics.KMeans({ fitIdx: [0, 1] });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with dense fitStart, dense matrix", function () {
            var centroids = new la.Matrix({ rows: 2, cols: 2, random: true });
            var KMeans = new analytics.KMeans({ fitStart: { C: centroids } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with sparse fitStart, dense matrix", function () {
            var centroids = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            var KMeans = new analytics.KMeans({ fitStart: { C: centroids } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans();
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with fitIdx, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with dense fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ fitStart: { C: new la.Matrix({ rows: 11, cols: 2, random: true }) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with sparse fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ fitStart: { C: new la.SparseMatrix([[[0, 1]], [[11, 3]]]) }});
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        })
        it("should return the correct model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, calcDistQual: true });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
            assert(KMeans.relMeanCentroidDist != null);
        });
        it("should return the correct model for dense centroids with fitIdx, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitIdx: [0, 1] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with dense fitStart, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitStart: { C: new la.Matrix({ rows: 2, cols: 2, random: true }) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with sparse fitStart, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 11);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with fitIdx, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 11);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with dense fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitStart: { C: new la.Matrix({ rows: 2, cols: 2, random: true }) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with sparse fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitStart: { C: new la.SparseMatrix([[[0, 1]], [[11, 3]]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 12);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 8);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with fitIdx, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitIdx: [0, 1] });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 8);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, dense centroids", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C, X);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C, X);
        });

        it("should create the model for sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse" });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with fitIdx, dense matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with dense fitStart, dense matrix", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.Matrix([[0, 1], [2, 3]]) } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with sparse fitStart, dense matrix", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with fitIdx, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with dense fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.Matrix([[0, 1], [3, 3]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with sparse fitStart, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the correct model for sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 3);
        });
        it("should return the correct model for sparse centroids with fitIdx, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", fitIdx: [0, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 3);
        });
        it("should return the correct model for sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for sparse centroids with fitIdx, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, sparse centroids", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C.rows, -1);
            assert.deepEqual(model.C.cols, 3);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C.rows, -1);
            assert.deepEqual(model2.C.cols, 3);
        });
        it("should throw an exception if k and length of fitIdx are not equal, dense centroids", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });
        it("should throw an exception if maximum value of fitIdx is greater that number of columns of matrix, dense centroids", function () {
            var KMeans = new analytics.KMeans({ k: 2, fitIdx: [0, 5] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });
        it("should throw an exception if maximum value of fitIdx is greater that number of columns of matrix, sparse centroids", function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse", fitIdx: [0, 5] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });

        // distanceType: "Cos"

        it("should create the model for dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos" });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with fitIdx, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitIdx: [0, 1] });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with dense fitStart, dense matrix, distanceType Cos", function () {
            var centroids = new la.Matrix({ rows: 2, cols: 2, random: true });
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitStart: { C: centroids } });
            var matrix = new la.Matrix([[-1, 1], [0, 1]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with sparse fitStart, dense matrix, distanceType Cos", function () {
            var centroids = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitStart: { C: centroids } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with fitIdx, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with dense fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitStart: { C: new la.Matrix({ rows: 11, cols: 2, random: true }) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids with sparse fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitStart: { C: new la.SparseMatrix([[[0, 1]], [[11, 3]]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        })
        it("should return the correct model for dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", distanceType: "Cos", k: 2 });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with fitIdx, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitIdx: [0, 1] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with dense fitStart, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitStart: { C: new la.Matrix({ rows: 2, cols: 2, random: true }) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with sparse fitStart, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2 });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 11);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with fitIdx, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 11);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with dense fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitStart: { C: new la.Matrix({ rows: 2, cols: 2, random: true }) } });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with sparse fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitStart: { C: new la.SparseMatrix([[[0, 1]], [[11, 3]]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 12);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2 });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 8);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids with fitIdx, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitIdx: [0, 1] });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 8);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, dense centroids, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C, X);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C, X);
        });

        it("should create the model for sparse centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", centroidType: "Sparse" });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with fitIdx, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with dense fitStart, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.Matrix([[0, 1], [2, 3]]) } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with sparse fitStart, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with fitIdx, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with dense fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.Matrix([[0, 1], [3, 3]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids with sparse fitStart, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", iter: 100, k: 2, centroidType: "Sparse", fitStart: { C: new la.SparseMatrix([[[0, 1]], [[1, 3]]]) } });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the correct model for sparse centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 3, centroidType: "Sparse" });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 3);
        });
        it("should return the correct model for sparse centroids with fitIdx, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 3, centroidType: "Sparse", fitIdx: [0, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 3);
        });
        it("should return the correct model for sparse centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for sparse centroids with fitIdx, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, centroidType: "Sparse", fitIdx: [0, 1] });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, sparse centroids, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C.rows, -1);
            assert.deepEqual(model.C.cols, 3);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C.rows, -1);
            assert.deepEqual(model2.C.cols, 3);
        });
        it("should throw an exception if k and length of fitIdx are not equal, dense centroids, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 3, fitIdx: [0, 1] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });
        it("should throw an exception if maximum value of fitIdx is greater that number of columns of matrix, dense centroids, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, fitIdx: [0, 5] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });
        it("should throw an exception if maximum value of fitIdx is greater that number of columns of matrix, sparse centroids, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ distanceType: "Cos", k: 2, centroidType: "Sparse", fitIdx: [0, 5] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X);
            });
        });

    });
    describe("Predict Tests", function () {
        it("should not throw an exception dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1], [0, 0]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[4, 1]]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });

        it("should not throw an exception sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });

        // distanceType: 'Cos'

        it("should not throw an exception dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception dense centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix dense centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1], [0, 0]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[4, 1]]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });

        it("should not throw an exception sparse centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception sparse centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix sparse centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix sparse centroids, sparse matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows sparse centroids, dense matrix, distanceType Cos", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse", distanceType: 'Cos' });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
    });


    describe("Explain Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [1,2,3]);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.explain(matrix);
            });
        });
        it("should return the predictions of the matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [341,422,122]);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var explanation = KMeans.explain(matrix);
            assert.equal(explanation[0].medoidID, 422);
            assert.equal(explanation[1].medoidID, 341);
            assert.equal(explanation[2].medoidID, 122);
        });
        it("should throw an exception if the matrix columns do not match record ids", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(X , [0, 1]);
            });
        });
    });

    describe("Transform Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.transform(matrix);
            });
        });
        it("should return the distance", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3]]);
            var transform = KMeans.transform(matrix);
            assert.equal(transform.rows, 3);
            assert.equal(transform.cols, 2);
        });
        it("should throw an exception because the matrix dimension is too big", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3], [0, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
        it("should throw an exception because the matrix dimension is too short", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
    });

    describe('PermuteCentroids Tests', function () {
        it('should not throw an exception, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([2, 0, 1]);
            assert.doesNotThrow(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should not throw an exception, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([2, 0, 1]);
            assert.doesNotThrow(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should permute the centroids, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Cent1 = KMeans.getModel().C;
            var Vec = new la.IntVector([1, 0]);
            KMeans.permuteCentroids(Vec);
            var Cent2 = KMeans.getModel().C;
            assert.equal(Cent1.at(0, 0), Cent2.at(0, 1));
            assert.equal(Cent1.at(0, 1), Cent2.at(0, 0));
            assert.equal(Cent1.at(1, 0), Cent2.at(1, 1));
            assert.equal(Cent1.at(1, 1), Cent2.at(1, 0));
        })
        it('should permute the centroids, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Cent1 = KMeans.getModel().C;
            var Vec = new la.IntVector([1, 0]);
            KMeans.permuteCentroids(Vec);
            var Cent2 = KMeans.getModel().C;
            assert.equal(Cent1.at(0, 0), Cent2.at(0, 1));
            assert.equal(Cent1.at(0, 1), Cent2.at(0, 0));
            assert.equal(Cent1.at(1, 0), Cent2.at(1, 1));
            assert.equal(Cent1.at(1, 1), Cent2.at(1, 0));
        })
        it('should throw an exception if the mapping vector is too short, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should throw an exception if the mapping vector is too short, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })

        it('should throw an exception if the mapping vector is too long, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0, 1, 2]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should throw an exception if the mapping vector is too long, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0, 1, 2]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
    });

    describe('testing correctness', function () {
        var C = new la.Matrix([
            [2, 7],
            [3, 2]
        ]);

        var X = new la.Matrix([
            [ 1, 2, 3, 3, 6, 7, 8 ],
            [ 3, 3, 2, 4, 2, 2, 3 ]
        ]);

        it('should identify correct dense centroids', function () {
            var denseKMeans = new analytics.KMeans({ k: 2, calcDistQual: true, fitStart: { C: C } });
            var expectedC = new la.Matrix([
                [ 2.2, 7 ],
                [ 3, 2.25 ]
            ])

            denseKMeans.fit(X);

            assert.equal(denseKMeans.centroids.minus(expectedC).frob(), 0);
        })

        it('should identify correct sparse centroids', function () {
            var sparseKMeans = new analytics.KMeans({ k: 2, centroidType: 'Sparse', calcDistQual: true, fitStart: { C: C } });
            var expectedC = new la.SparseMatrix([
                [ [0, 2.2], [1, 3] ],
                [ [0, 7], [1, 2.25] ]
            ])

            sparseKMeans.fit(X);

            var frob = expectedC.minus(sparseKMeans.centroids).frob();
            assert(frob < 1e-9, 'Centroids differ by norm: ' + frob + ' which should be 0!');
        })
    })

    describe('Quality measure tests', function () {
        it('should calculate correct quality measure for sparse centroids', function () {
            var C = new la.Matrix([
                [2, 7],
                [3, 2]
            ]);

            var denseKMeans = new analytics.KMeans({ k: 2, calcDistQual: true, fitStart: { C: C } });
            var sparseKMeans = new analytics.KMeans({ k: 2, centroidType: 'Sparse', calcDistQual: true, fitStart: { C: C } });

            var X = new la.Matrix([
                [ 1, 2, 3, 3, 6, 7, 8 ],
                [ 3, 3, 2, 4, 2, 2, 3 ]
            ]);

            denseKMeans.fit(X);
            sparseKMeans.fit(X);

            var clust1 = [
                [ 1, 3 ],
                [ 2, 3 ],
                [ 3, 2 ],
                [ 3, 4 ]
            ]

            var clust2 = [
                [ 6, 2 ],
                [ 7, 2 ],
                [ 8, 3 ]
            ]

            var dist = function (a, b) { return Math.sqrt((a[0] - b[0])*(a[0] - b[0]) + (a[1] - b[1])*(a[1] - b[1])); }
            var sumDistCentroid = function (clust, centroid) {
                var sumDist = 0;
                for (var i = 0; i < clust.length; i++) {
                    sumDist += dist(clust[i], centroid);
                }
                return sumDist;
            }

            var sumGlobal = (function () {
                var mean = [ 0, 0 ];
                for (var i = 0; i < clust1.length; i++) {
                    mean[0] += clust1[i][0];
                    mean[1] += clust1[i][1];
                }
                for (var i = 0; i < clust2.length; i++) {
                    mean[0] += clust2[i][0];
                    mean[1] += clust2[i][1];
                }

                mean[0] /= (clust1.length + clust2.length);
                mean[1] /= (clust1.length + clust2.length);

                var result = 0;
                for (var i = 0; i < clust1.length; i++) {
                    result += dist(clust1[i], mean);
                }
                for (var i = 0; i < clust2.length; i++) {
                    result += dist(clust2[i], mean);
                }

                return result;
            })();

            var sumCent1 = sumDistCentroid(clust1, denseKMeans.centroids.getCol(0));
            var sumCent2 = sumDistCentroid(clust2, denseKMeans.centroids.getCol(1));

            var innerClustMean = 4*(sumCent1/4) / 7 + 3*(sumCent2/3) / 7;
            var globalMean = sumGlobal / 7;

            var expectedResult = innerClustMean / globalMean;

            assert(Math.abs(denseKMeans.relMeanCentroidDist - expectedResult) < 1e-7, 'Dense k-means relativeMeanDist does not match expected value!');
            assert(Math.abs(sparseKMeans.relMeanCentroidDist - expectedResult) < 1e-7, 'Sparse k-means relativeMeanDist does not match expected value!');
        })

    })

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var fin = require('qminer').fs.openWrite('kmeans_test.bin');
            KMeans.save(fin); fin.close();
            var KMeans2 = new analytics.KMeans(require('qminer').fs.openRead('kmeans_test.bin'));
            var params = KMeans.getParams();
            var params2 = KMeans2.getParams();
            assert.deepEqual(KMeans.getParams(), KMeans2.getParams());
            assert.deepEqual(KMeans.getModel().C, KMeans2.getModel().C, 1e-8);
        })
    });

    describe('Bad input tests ...', function () {
        it('should find NaNs in the distance matrix because of bad initial centroids', function () {
            if (qm.flags.debug) return; // TODO in debug, the program terminates! => for now only test in release

            var centroids = new la.SparseMatrix([[[1, 0]], [[0, -1]]]);
            var KMeans = new analytics.KMeans({ distanceType: "Cos", fitStart: { C: centroids } });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.throws(function () {
                KMeans.fit(matrix);
            });
        })
    });
});

describe("DpMeans test", function () {
    describe('Constructor test', function () {
        it("should return parameter values", function () {
            var dpmeans = new analytics.DpMeans({ iter: 100, lambda: 6, verbose: false });
            var params = dpmeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.lambda, 6);
            assert.equal(params.minClusters, 2);
            assert(params.maxClusters == null);
            assert.equal(params.distanceType, "Euclid");
            assert.equal(params.centroidType, "Dense");
            assert.equal(params.calcDistQual, false);
            assert.equal(params.verbose, false);
        });

        it('should correctly handle maxClusters < minClusters', function () {
            assert.throws(function () {
                var dpmeans = new analytics.DpMeans({ lambda: 6, minClusters: 10, maxClusters: 9 });
                var X = new la.Matrix([
                    [ 1, 2, 3, 3, 6, 7, 8 ],
                    [ 3, 3, 2, 4, 2, 2, 3 ]
                ]);
                dpmeans.fit(X);
            });
        })
    })

    describe('testing correctness', function () {
        var X = new la.Matrix([
            [ 1.5, 1.5, 2.5, 2.5, 4, 5, 8, 10 ],
            [ 2.5, 1.5, 2.5, 1.5, 7, 8, 3, 5 ]
        ])

        it('DpMeans should compute correct assignment', function () {
            var initC = new la.Matrix([
                [ 1.2, 5, 9.3 ],
                [ 1, 7.5, 4.2 ]
            ])
            var dpmeans = new analytics.DpMeans({
                lambda: 2,
                allowEmpty: false,
                fitStart: { C: initC }
            })

            var expectedAssignV = new la.IntVector([0, 0, 0, 0, 1, 1, 2, 2]);

            dpmeans.fit(X);

            var assignV = dpmeans.idxv;

            for (var i = 0; i < expectedAssignV.length; i++) {
                assert.equal(assignV[i], expectedAssignV[i]);
            }
        })
    })

    describe('testing quality measure', function () {
        it('DpMeans should calculate correct quality measure for sparse centroids', function () {
            var C = new la.Matrix([
                [2, 7],
                [3, 2]
            ]);

            var denseDpMeans = new analytics.DpMeans({ lambda: 2, minClusters: 2, maxClusters: 2, calcDistQual: true, fitStart: { C: C } });
            var sparseDpMeans = new analytics.DpMeans({ lambda: 2, minClusters: 2, maxClusters: 2, centroidType: 'Sparse', calcDistQual: true, fitStart: { C: C } });

            var X = new la.Matrix([
                [ 1, 2, 3, 3, 6, 7, 8 ],
                [ 3, 3, 2, 4, 2, 2, 3 ]
            ]);

            denseDpMeans.fit(X);
            sparseDpMeans.fit(X);

            var clust1 = [
                [ 1, 3 ],
                [ 2, 3 ],
                [ 3, 2 ],
                [ 3, 4 ]
            ]

            var clust2 = [
                [ 6, 2 ],
                [ 7, 2 ],
                [ 8, 3 ]
            ]

            var dist = function (a, b) { return Math.sqrt((a[0] - b[0])*(a[0] - b[0]) + (a[1] - b[1])*(a[1] - b[1])); }
            var sumDistCentroid = function (clust, centroid) {
                var sumDist = 0;
                for (var i = 0; i < clust.length; i++) {
                    sumDist += dist(clust[i], centroid);
                }
                return sumDist;
            }

            var sumGlobal = (function () {
                var mean = [ 0, 0 ];
                for (var i = 0; i < clust1.length; i++) {
                    mean[0] += clust1[i][0];
                    mean[1] += clust1[i][1];
                }
                for (var i = 0; i < clust2.length; i++) {
                    mean[0] += clust2[i][0];
                    mean[1] += clust2[i][1];
                }

                mean[0] /= (clust1.length + clust2.length);
                mean[1] /= (clust1.length + clust2.length);

                var result = 0;
                for (var i = 0; i < clust1.length; i++) {
                    result += dist(clust1[i], mean);
                }
                for (var i = 0; i < clust2.length; i++) {
                    result += dist(clust2[i], mean);
                }

                return result;
            })();

            var sumCent1 = sumDistCentroid(clust1, denseDpMeans.centroids.getCol(0));
            var sumCent2 = sumDistCentroid(clust2, denseDpMeans.centroids.getCol(1));

            var innerClustMean = 4*(sumCent1/4) / 7 + 3*(sumCent2/3) / 7;
            var globalMean = sumGlobal / 7;

            var expectedResult = innerClustMean / globalMean;

            assert(Math.abs(denseDpMeans.relMeanCentroidDist - expectedResult) < 1e-7, 'Dense k-means relativeMeanDist does not match expected value!');
            assert(Math.abs(sparseDpMeans.relMeanCentroidDist - expectedResult) < 1e-7, 'Sparse k-means relativeMeanDist does not match expected value!');
        })
    })

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var dpmeans = new analytics.DpMeans({ lambda: 5, minClusters: 3, maxClusters: 5 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            dpmeans.fit(X);
            var fin = require('qminer').fs.openWrite('dpmeans_test.bin');
            dpmeans.save(fin); fin.close();
            var dpmeans2 = new analytics.DpMeans(require('qminer').fs.openRead('dpmeans_test.bin'));
            var params = dpmeans.getParams();
            var params2 = dpmeans2.getParams();
            assert.deepEqual(dpmeans.getParams(), dpmeans2.getParams());
            assert.deepEqual(dpmeans.getModel().C, dpmeans2.getModel().C, 1e-8);
        })
    });
});

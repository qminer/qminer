/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var la = require("qminer").la;
var analytics = require("qminer").analytics;
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for Kmeans

describe("Kmeans test", function () {

    describe("Constructor test", function () {
        it("should return empty parameter values", function () {
            var KMeans = new analytics.KMeans();
            var params = KMeans.getParams();
            assert.equal(Object.keys(params).length, 3);
        });
        it("should return parameter values", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, verbose: false });
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 2);
            assert.equal(params.verbose, false);
            assert.equal(params.fitIdx, undefined);
        });
        it("should return empty model parameters", function () {
            var KMeans = new analytics.KMeans();
            var model = KMeans.getModel();
            assert.equal(model.C, undefined);
            assert.equal(model.idxv, undefined);
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
        it("should create the model", function () {
            var KMeans = new analytics.KMeans();
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model, using fitIdx", function () {
            var KMeans = new analytics.KMeans({ fitIdx: [0, 1] });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the correct idxv of the model", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.C.cols, 3);
            assert.equal(model.idxv.length, 3);
        });
        it("should return the correct centroids of the model", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.deepEqual(model.C, matrix);
        });
        it("should return the correct C of the model", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [1, 7, 6] });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var C = KMeans.getModel().C;
            assert.eqtol(C.at(0, 0), 2, 5e-1);
            assert.eqtol(C.at(1, 0), 6, 5e-1);
            assert.eqtol(C.at(0, 1), -10, 5e-1);
            assert.eqtol(C.at(1, 1), 0, 5e-1);
            assert.eqtol(C.at(0, 2), 3, 5e-1);
            assert.eqtol(C.at(1, 2), -2, 5e-1);
        });
        it("should throw an exception if fitIdx is not of length k", function () {
            var KMeans = new analytics.KMeans({ fitIdx: [0, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(matrix);
            });
        });
        it("should throw an exception if fitIdx has index greater than number of matrix columns", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [4, 1, 2] });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the same model even after changing parameter values", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C, X);

            KMeans.setParams({ iter: 102, fitIdx: [] });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C, X);
        });
    });
    describe("Predict Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction[0], 1);
            assert.equal(prediction[1], 0);
            assert.equal(prediction[2], 2);
        });
        it("should throw an exception if the matrix has less rows", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1], [0, 0]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
    });


    describe("Explain Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [1,2,3]);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.explain(matrix);
            });
        });
        it("should return the predictions of the matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [341,422,122]);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var explanation = KMeans.explain(matrix);
            debugger
            assert.equal(explanation[0].medoidID, 422);
            assert.equal(explanation[1].medoidID, 341);
            assert.equal(explanation[2].medoidID, 122);
        });
        it("should throw an exception if the matrix columns do not match record ids", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
            	KMeans.fit(X , [0, 1]);
            });
        });
    });

    describe("Transform Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.transform(matrix);
            });
        });
        it("should return the distance", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3]]);
            var transform = KMeans.transform(matrix);
            assert.eqtol(transform.at(0, 0), 10);
            assert.eqtol(transform.at(0, 1), 17);
            assert.eqtol(transform.at(1, 0), 1);
            assert.eqtol(transform.at(1, 1), 20);
            assert.eqtol(transform.at(2, 0), 10);
            assert.eqtol(transform.at(2, 1), 1);
        });
        it("should throw an exception because the matrix dimension is too big", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3], [0, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
        it("should throw an exception because the matrix dimension is too short", function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var KMeans = new analytics.KMeans({ k: 3, fitIdx: [0, 1, 2] });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            KMeans.save(require('qminer').fs.openWrite('kmeans_test.bin')).close();
            var KMeans2 = new analytics.KMeans(require('qminer').fs.openRead('kmeans_test.bin'));
            var params = KMeans.getParams();
            var params2 = KMeans2.getParams();
            assert.deepEqual(KMeans.getParams(), KMeans2.getParams());
            assert.deepEqual(KMeans.getModel().C, KMeans2.getModel().C, 1e-8);
        })
    });
});

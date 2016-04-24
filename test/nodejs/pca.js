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
//Unit test for PCA

describe("PCA test", function () {
    describe("Constructor tests", function () {
        it("should not throw an exception", function () {
            assert.doesNotThrow(function () {
                var pca = new analytics.PCA();
            });
        });
        it("should return default values of parameters", function () {
            var pca = new analytics.PCA();
            var params = pca.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, undefined);
        });
        it("should return values of parameters", function () {
            var pca = new analytics.PCA({ iter: 100, k: 50 });
            var params = pca.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 50);
        });
        it("should return values of parameters and added keys", function () {
            var pca = new analytics.PCA({ iter: 30, alpha: 3 });
            var params = pca.getParams();
            assert.equal(params.iter, 30);
            assert.equal(params.alpha, undefined);
        });
        it("should return empty model parameters", function () {
            var pca = new analytics.PCA();
            var model = pca.getModel();
            assert.equal(model.P, undefined);
            assert.equal(model.lambda, undefined);
            assert.equal(model.mu, undefined);
        });
    });
    describe("testing setParams", function () {
        it("should return changed values of parameters", function () {
            var pca = new analytics.PCA();
            pca.setParams({ iter: 10, k: 5 });
            var params = pca.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.k, 5);
        });
        it("should return changed values of parameters, ignoring added key values", function () {
            var pca = new analytics.PCA();
            pca.setParams({ iter: 10, beta: 30 });
            var params = pca.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.beta, undefined);
        });
    });
    describe("Fit testing", function () {
        it("should not throw an exception creating a model", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            assert.doesNotThrow(function () {
                pca.fit(matrix);
            });
        });
        it("should throw an exception because k is bigger than matrix dimensions", function () {
            var pca = new analytics.PCA({ k: 5 });
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            assert.throws(function () {
                pca.fit(matrix);
            });
        });
        it("should return the model parameters after using fit", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var model = pca.getModel();
            assert.eqtol(model.lambda[0], 1);
            assert.eqtol(model.lambda[1], 0);
            assert.eqtol(model.mu[0], 0.5);
            assert.eqtol(model.mu[1], -0.5);
            assert.eqtol(Math.abs(model.P.at(0, 0)), 0.707106781186547, 1e-6);
            assert.eqtol(Math.abs(model.P.at(1, 0)), 0.707106781186547, 1e-6);
            assert(model.P.at(0, 0) * model.P.at(1, 0) > 0);
            assert.eqtol(model.P.getCol(0).inner(model.P.getCol(1)), 0);
        });
        it("should return the model parameters of fit, second example", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[3, 5], [2, 1]]);
            pca.fit(matrix);
            var model = pca.getModel();
            assert.eqtol(model.lambda[0], 2.5);
            assert.eqtol(model.lambda[1], 0);
            assert.eqtol(model.mu[0], 4);
            assert.eqtol(model.mu[1], 1.5);
            assert.eqtol(Math.abs(model.P.at(0, 0)), 0.894427190999916, 1e-6);
            assert.eqtol(Math.abs(model.P.at(1, 0)), 0.447213595499958, 1e-6);
            assert(model.P.at(0, 0) * model.P.at(1, 0) < 0);
            assert.eqtol(model.P.getCol(0).inner(model.P.getCol(1)), 0);
        });
    });
    describe("Transform testing", function () {
        it("should not throw an exception using transform on model", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            assert.throws(function () {
                pca.transform(matrix);
            });
        });
        it("should return a result using transform with matrix", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var model = pca.getModel();
            var tran = pca.transform(matrix);
            assert.eqtol(tran.at(0, 0), -0.707106781186547, 1e-6);
            assert.eqtol(tran.at(1, 0), 0, 1e-6);
            assert.eqtol(tran.at(0, 1), 0.707106781186547, 1e-6);
            assert.eqtol(tran.at(1, 1), 0, 1e-6);
        });
        it("should return a result using transform with vector", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var vec = new la.Vector([0, -1]);
            var tran = pca.transform(vec);
            assert.eqtol(tran.at(0, 0), -0.707106781186547, 1e-6);
            assert.eqtol(tran.at(1, 0), 0, 1e-6);
        });
        it("should throw and exception because matrix dimensions don't match", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0], [2, -1]]);
            pca.fit(matrix);
            var newMatrix = new la.Matrix([[0, 1, 4], [-1, 0, 0]]);
            var model = pca.getModel();
            assert.throws(function () {
                pca.transform(newMatrix);
            });
        });
    });
    describe("inverseTransform testing", function () {
        it("should not throw an exception using inverseTransform on model", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            assert.throws(function () {
                pca.inverseTransform(matrix);
            });
        });
        it("should return a result using inverseTransform with matrix", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var model = pca.getModel();
            var tran = pca.inverseTransform(matrix);
            assert.eqtol(tran.at(0, 0), -0.207106781186547, 1e-6);
            assert.eqtol(tran.at(1, 0), 0.207106781186547, 1e-6);
            assert.eqtol(tran.at(0, 1), 1.207106781186547, 1e-6);
            assert.eqtol(tran.at(1, 1), 0.207106781186547, 1e-6);
        });
        it("should return a result using inverseTransform with vector", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var vec = new la.Vector([0, -1]);
            var tran = pca.inverseTransform(vec);
            assert.eqtol(tran.at(0, 0), -0.207106781186547, 1e-6);
            assert.eqtol(tran.at(1, 0), 0.207106781186547, 1e-6);
        });
        it("should throw and exception because matrix dimensions don't match", function () {
            var pca = new analytics.PCA();
            var matrix = new la.Matrix([[0, 1], [-1, 0], [2, -1]]);
            pca.fit(matrix);
            var newMatrix = new la.Matrix([[0, 1, 4], [-1, 0, 0]]);
            var model = pca.getModel();
            assert.throws(function () {
                pca.inverseTransform(newMatrix);
            });
        });
    });
    describe("Save and load test", function () {
        it("should serialize and deserialize the model", function () {
            var pca = new analytics.PCA({ k: 1 });
            var matrix = new la.Matrix([[0, 1], [-1, 0]]);
            pca.fit(matrix);
            var model = pca.getModel();
            var tran = pca.transform(matrix);
            pca.save(require('qminer').fs.openWrite('pca_test.bin')).close();
            var pca2 = new analytics.PCA(require('qminer').fs.openRead('pca_test.bin'));
            var model2 = pca2.getModel();
            assert.deepEqual(pca.getParams(), pca2.getParams());
            assert.eqtol(model.mu.minus(model2.mu).norm(), 0);
            assert.eqtol(model.lambda.minus(model2.lambda).norm(), 0);
            assert.eqtol(model.P.minus(model2.P).frob(), 0);
        });
    });
    describe("Ultimate test", function () {
        it("should achive perfect reconstruction", function () {
            var A = new la.Matrix([[10, 20, 110, 20, 10], [3, 4, 13, 63, 1]]);
            var pca = new analytics.PCA({ k: 2, iter: 1000 });
            pca.fit(A);
            var pA = pca.transform(A);
            var rA = pca.inverseTransform(pA);
            assert(A.minus(rA).frob() < 1e-6);
        });
    })
});

// JavaScript source code
var la = require("qminer").la;
var analytics = require("qminer").analytics;
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for PCA

describe("PCA test", function () {
    beforeEach(function () {

    });
    afterEach(function () {

    });
    describe("Constructor tests", function () {
        it("should not throw an exception", function () {
            assert.doesNotThrow(function () {
                var pca = new analytics.PCA();
            });
        });
        it("should return default values of parameters", function () {
            var pca = new analytics.PCA();
            var params = pca.getParams();
            assert.equal(params.iter, undefined);
            assert.equal(params.k, undefined);
        });
        it("should return values of parameters", function () {
            var pca = new analytics.PCA({iter: 100, k: 50});
            var params = pca.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 50);
        });
        it("should return values of parameters and added keys", function () {
            var pca = new analytics.PCA({iter: 30, alpha: 3});
            var params = pca.getParams();
            assert.equal(params.iter, 30);
            assert.equal(params.alpha, 3);
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
            pca.setParams({iter: 10, k: 5});
            var params = pca.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.k, 5);
        });
        it("should return changed values of parameters and added key values", function () {
            var pca = new analytics.PCA();
            pca.setParams({ iter: 10, beta: 30 });
            var params = pca.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.beta, 30);
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
            var pca = new analytics.PCA({ k: 5});
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
            assert.equal(model.mu[0], -0.5);
            assert.equal(model.mu[1], 0.5);
        });
    });
});
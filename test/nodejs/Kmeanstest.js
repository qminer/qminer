// JavaScript source code
var la = require("qminer").la;
var analytics = require("qminer").analytics;
var assert = require("../../src/nodejs/scripts/assert.js");
//Unit test for Kmeans

describe("Kmeans test", function () {
    beforeEach(function () {

    });
    afterEach(function () {

    });

    describe("Constructor test", function () {
        it("should return empty parameter values", function () {
            var KMeans = new analytics.KMeans();
            var params = KMeans.getParams();
            assert.equal(Object.keys(params).length, 0)
            
        });
        it("should return parameter values", function () {
            var KMeans = new analytics.KMeans();
            KMeans.setParams({iter: 100, k: 2, verbose: false});
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 2);
            assert.equal(params.verbose, false);
        });
        it("should return empty model parameters", function () {
            var KMeans = new analytics.KMeans();
            var model = KMeans.getModel();
            assert.equal(model.C, undefined);
            assert.equal(model.idxv, undefined);
        });
    })

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
            KMeans.setParams({ iter: 10, k: 5, alpha: false});
            var params = KMeans.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.k, 5);
            assert.equal(params.verbose, undefined);
        });
    })
});

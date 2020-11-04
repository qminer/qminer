/**
* Tests for Multidimensional scaling class in qm.analytics.
*/

var qm = require('../../index.js');
var analytics = qm.analytics;
var assert = require('../../src/nodejs/scripts/assert.js');

describe('MDS Tests', function () {
    describe('Constructor tests', function () {
        it('should not throw an exception for default constructor', function () {
            assert.doesNotThrow(function () {
                var mds = new analytics.MDS();
            });
        })
        it('should not throw an exception for custom constructor', function () {
            assert.doesNotThrow(function () {
                var mds = new analytics.MDS({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" });
            });
        })
        it('should create the default constructor', function () {
            var mds = new analytics.MDS();
            var params = mds.getParams();
            assert.strictEqual(params.maxSecs, 500);
            assert.strictEqual(params.maxStep, 5000);
            assert.eqtol(params.minDiff, 1e-4);
            assert.strictEqual(params.distType, "Euclid");
        })
        it('should create the custom constructor, type: Cos', function () {
            var mds = new analytics.MDS({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" });
            var params = mds.getParams();
            assert.strictEqual(params.maxSecs, 50);
            assert.strictEqual(params.maxStep, 100);
            assert.eqtol(params.minDiff, 1e-7);
            assert.strictEqual(params.distType, "Cos");
        })
        it('should create the custom constructor, type: SqrtCos', function () {
            var mds = new analytics.MDS({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "SqrtCos" });
            var params = mds.getParams();
            assert.strictEqual(params.maxSecs, 50);
            assert.strictEqual(params.maxStep, 100);
            assert.eqtol(params.minDiff, 1e-7);
            assert.strictEqual(params.distType, "SqrtCos");
        })
        it('should throw an exception for any other distance type', function () {
            assert.throws(function () {
                var mds = new analytics.MDS({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Matrix" });
            })
        })
    });
    describe('GetParams Tests', function () {
        it('should not throw an exception, default constructor', function () {
            var mds = new analytics.MDS();
            assert.doesNotThrow(function () {
                var params = mds.getParams();
            });
        })
        it('should not throw an exception, custom constructor', function () {
            var mds = new analytics.MDS({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" });
            assert.doesNotThrow(function () {
                var params = mds.getParams();
            });
        })
        it('should get the mds parameters', function () {
            var mds = new analytics.MDS();
            var params = mds.getParams();
            assert.strictEqual(params.maxSecs, 500);
            assert.strictEqual(params.maxStep, 5000);
            assert.eqtol(params.minDiff, 1e-4);
            assert.strictEqual(params.distType, "Euclid");
        })
        it('should return the json object used for the construction', function () {
            var json = { maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" };
            var mds = new analytics.MDS(json);
            var params = mds.getParams();
            assert.strictEqual(json.maxSecs, params.maxSecs);
            assert.strictEqual(json.maxStep, params.maxStep);
            assert.eqtol(json.minDiff, params.minDiff);
            assert.strictEqual(json.distType, params.distType);
        })
        it('should throw an exception if a parameter is given', function () {
            var mds = new analytics.MDS();
            assert.throws(function () {
                var params = mds.getParams(10);
            });
        })
    })
    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var mds = new analytics.MDS();
            assert.doesNotThrow(function () {
                mds.setParams({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" });
            });
        })
        it('should set the parameters of the mds instance', function () {
            var mds = new analytics.MDS();
            mds.setParams({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Cos" });
            var params = mds.getParams();
            assert.strictEqual(params.maxSecs, 50);
            assert.strictEqual(params.maxStep, 100);
            assert.eqtol(params.minDiff, 1e-7);
            assert.strictEqual(params.distType, "Cos");
        })
        it('should throw an exception if the parameters are not valid', function () {
            var mds = new analytics.MDS();
            assert.throws(function () {
                mds.setParams({ maxStep: 100, maxSecs: 50, minDiff: 1e-7, distType: "Matrix" });
            });
        })
        it('should throw an exception if no parameters are given', function () {
            var mds = new analytics.MDS();
            assert.throws(function () {
                mds.setParams();
            });
        })
    });
    describe('FitTransform Tests', function () {
        it('should not throw an exception', function () {
            var mds = new analytics.MDS();
            var mat = new qm.la.Matrix([[1, 1], [2, 3]]);
            assert.doesNotThrow(function () {
                mds.fitTransform(mat);
            });
        })
        it('should create a multidimensional scaling on the matrix', function () {
            var mds = new analytics.MDS();
            var mat = new qm.la.Matrix([[1, 1], [2, 3]]);
            var coord = mds.fitTransform(mat);
            assert.strictEqual(coord.rows, mat.cols);
            assert.eqtol(coord.cols, 2);
        })
    });
})


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

// Recommender System Unit Tests
describe("RecommenderSys tests", function () {

    describe("Constructor tests", function () {
        it("should not throw an exception", function () {
            assert.doesNotThrow(function () {
                var recSys = new analytics.RecommenderSys();
            });
        })
        it("should create the object with the default parameters", function () {
            var recSys = new analytics.RecommenderSys();
            var params = recSys.getParams();
            assert.equal(params.iter, 10000);
            assert.equal(params.tol, 1e-3);
            assert.equal(params.k, 2);
            assert.equal(params.verbose, false);
        })
        it("should create the object with the given parameters", function () {
            var recSys = new analytics.RecommenderSys({ iter: 100, tol: 1e-3, k: 20, verbose: true });
            var params = recSys.getParams();
            assert.equal(params.iter, 100);
            assert.eqtol(params.tol, 1e-3);
            assert.equal(params.k, 20);
            assert.equal(params.verbose, true);
        })
        it("should create the object with the given parameters, even when the given parameters doesn't have the correct key-values", function () {
            var recSys = new analytics.RecommenderSys({ fruit: 100, blouse: 20, house: true });
            var params = recSys.getParams();
            assert.equal(params.iter, 10000);
            assert.equal(params.k, 2);
            assert.equal(params.verbose, false);
        })
    });

    describe("GetParams tests", function () {
        it("should not throw an exception", function () {
            var recSys = new analytics.RecommenderSys();
            assert.doesNotThrow(function () {
                var params = recSys.getParams();
            });
        })
        it("should return the parameters of the object", function () {
            var recSys = new analytics.RecommenderSys();
            var params = recSys.getParams();
            assert.equal(params.iter, 10000);
            assert.eqtol(params.tol, 1e-3);
            assert.equal(params.k, 2);
            assert.equal(params.verbose, false);
        })
        it("should throw an exception if parameters are given to the function", function () {
            var recSys = new analytics.RecommenderSys({ iter: 100, k: 20, verbose: true });
            assert.throws(function () {
                var params = recSys.getParams(1);
            });
        })
        it("should return the parameters in the same format for constructing a new object", function () {
            var recSys = new analytics.RecommenderSys({ iter: 100, k: 20, verbose: true });
            var params = recSys.getParams();
            var recSys2 = new analytics.RecommenderSys(params);
            var params2 = recSys2.getParams();
            assert.equal(params.iter, params2.iter);
            assert.equal(params.tol, params2.tol);
            assert.equal(params.k, params2.k);
            assert.equal(params.verbose, params.verbose);
        })
    });

    describe("SetParams tests", function () {
        it("should not throw an exception", function () {
            var recSys = new analytics.RecommenderSys();
            var params = { iter: 100, k: 20, verbose: true };
            assert.doesNotThrow(function () {
                recSys.setParams(params);
            });
        })
        it("should set the parameters of the object", function () {
            var recSys = new analytics.RecommenderSys();
            var params = { iter: 100, k: 20, tol: 1e-6, verbose: true };
            recSys.setParams(params);
            var params2 = recSys.getParams();
            assert.equal(params.iter, params2.iter);
            assert.equal(params.k, params2.k);
            assert.equal(params.tol, params2.tol);
            assert.equal(params.verbose, params2.verbose);
        })
        it("should throw an exception if no parameter is given", function () {
            var recSys = new analytics.RecommenderSys();
            assert.throws(function () {
                recSys.setParams(params);
            });
        })
    });

    describe("Fit tests", function () {
        it("should not throw an exception", function () {
            var recSys = new analytics.RecommenderSys();
            var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
            assert.doesNotThrow(function () {
                recSys.fit(mat);
            });
        })
        it("should fit the model, dense matrix", function () {
            var recSys = new analytics.RecommenderSys();
            var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
            recSys.fit(mat);
            var model = recSys.getModel();
            assert.equal(model.U.rows, 4);
            assert.equal(model.U.cols, 2);
            assert.equal(model.V.rows, 2);
            assert.equal(model.V.cols, 4);
        })
        it('should fit the model, sparse matrix', function () {
            var recSys = new analytics.RecommenderSys();
            var mat = new la.SparseMatrix([[[0, 1]], [[0, 5], [1, 3], [2, 5]], [[3, 4]], [[0, 1], [2, 3], [3, 1]]]);
            recSys.fit(mat);
            var model = recSys.getModel();
            assert.equal(model.U.rows, 4);
            assert.equal(model.U.cols, 2);
            assert.equal(model.V.rows, 2);
            assert.equal(model.V.cols, 4);
        })
        it("should throw an exception, if the parameter k is greater of the min dimension of matrix, dense matrix", function () {
            var recSys = new analytics.RecommenderSys({ k: 10 });
            var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
            assert.throws(function () {
                recSys.fit(mat);
            });
        })
        it("should throw an exception, if the parameter k is greater of the min dimension of matrix, sparse matrix", function () {
            var recSys = new analytics.RecommenderSys({ k: 10 });
            var mat = new la.SparseMatrix([[[0, 1]], [[0, 5], [1, 3], [2, 5]], [[3, 4]], [[0, 1], [2, 3], [3, 1]]]);
            assert.throws(function () {
                recSys.fit(mat);
            });
        })
    });

    describe("GetModel tests", function () {
        it("should not throw an exception", function () {
            var recSys = new analytics.RecommenderSys();
            assert.doesNotThrow(function () {
                var model = recSys.getModel();
            });
        })
        it("should return an empty matrices of the model", function () {
            var recSys = new analytics.RecommenderSys();
            var model = recSys.getModel();
            assert.equal(model.U.rows, 0);
            assert.equal(model.U.cols, 0);
            assert.equal(model.V.rows, 0);
            assert.equal(model.V.cols, 0);
        })
        it("should return the model of the fitted data, dense matrix", function () {
            var recSys = new analytics.RecommenderSys();
            var mat = new la.Matrix([[1, 0.5, 0, 1], [0, 3, 0, 0], [0, 0.5, 0, 3], [0, 0, 0.4, 1]]);
            recSys.fit(mat);
            var model = recSys.getModel();
            assert.equal(model.U.rows, 4);
            assert.equal(model.U.cols, 2);
            assert.equal(model.V.rows, 2);
            assert.equal(model.V.cols, 4);
        })
        it("should return the model of the fitted data, sparse matrix", function () {
            var recSys = new analytics.RecommenderSys();
            var mat = new la.SparseMatrix([[[0, 1]], [[0, 5], [1, 3], [2, 5]], [[3, 4]], [[0, 1], [2, 3], [3, 1]]]);
            recSys.fit(mat);
            var model = recSys.getModel();
            assert.equal(model.U.rows, 4);
            assert.equal(model.U.cols, 2);
            assert.equal(model.V.rows, 2);
            assert.equal(model.V.cols, 4);
        })
        it("should throw an exception, if some parameters are given to the function", function () {
            var recSys = new analytics.RecommenderSys();
            assert.throws(function () {
                var model = recSys.getModel(1);
            });
        })
    });

    describe("Save and load test", function () {
        it("should serialize and deserialize the model", function () {
            var recSys = new analytics.RecommenderSys({ tol: 1e-6, iter: 101, verbose: false, k: 2 });
            var matrix = new la.Matrix([[0, 1], [1, 0]]);
            recSys.fit(matrix);
            var params = recSys.getParams();
            var model = recSys.getModel();
            recSys.save(require('qminer').fs.openWrite('recommenderSys_test.bin')).close();
            var recSys2 = new analytics.RecommenderSys(require('qminer').fs.openRead('recommenderSys_test.bin'));
            var params2 = recSys2.getParams();
            var model2 = recSys2.getModel();
            assert.equal(params.iter, params2.iter);
            assert.equal(params.k, params2.k);
            assert.equal(params.tol, params2.tol);
            assert.equal(params.verbose, params2.verbose);
            assert.eqtol(model.U.minus(model2.U).frob(), 0);
            assert.eqtol(model.V.minus(model2.V).frob(), 0);
        });
    });
})

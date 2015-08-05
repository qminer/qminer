// JavaScript source code
var la = require("qminer").la;
var analytics = require("qminer").analytics;
var assert = require("../../src/nodejs/scripts/assert.js");

//Unit test for SVR

describe("SVR test", function () {
    beforeEach(function () {

    });
    afterEach(function () {

    });

    describe("Constructor test", function () {
        it("It should return a default constructor", function () {
            var SVR = new analytics.SVR();
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        });
        it("It should return a SVR created by Json", function () {
            var SVR = new analytics.SVR({ c: 5, eps: 5, batchSize: 5, maxIterations: 5, maxTime: 1, minDiff: 1e-10, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 5);
            assert.equal(SVRjSon.eps, 5);
            assert.equal(SVRjSon.batchSize, 5);
            assert.equal(SVRjSon.maxIterations, 5);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-10);
            assert.equal(SVRjSon.verbose, true);
        });

        it("It should return a SVR created by Json, not all key values are given", function () {
            var SVR = new analytics.SVR({ c: 5, batchSize: 5, maxTime: 1, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 5);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 5);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, true);
        });

        it("It should return a SVR created by an empty Json", function () {
            var SVR = new analytics.SVR({});
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        });

        it("It should return a SVR created by Json, with added key values", function () {
            var SVR = new analytics.SVR({ alpha: 5, beta: 10, s: 3, batchSize: 10000, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 10000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, true);
        });
    });

    describe("GetParams tests", function () {
        it("should return the parameters of the default SVR model as Json", function () {
            var SVR = new analytics.SVR();
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })

        it("should return the parameters of the default SVR model as Json, without some key values", function () {
            var SVR = new analytics.SVR({ c: 3, eps: 2, maxTime: 1 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 3);
            assert.equal(SVRjSon.eps, 2);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })

        it("should return the parameters of the default SVR model as Json, with added key values", function () {
            var SVR = new analytics.SVR({ alpha: 3, beta: 3, z: 3 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 1e-1);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 1);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
    });

    describe("SetParams tests", function () {
        it("should return the existing SVR with the changed values", function () {
            var SVR = new analytics.SVR();
            SVR.setParams({ eps: 3, maxTime: 2 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 3);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 2);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
        it("should return the existing SVR with the changed, added values", function () {
            var SVR = new analytics.SVR();
            SVR.setParams({ eps: 3, maxTime: 2, alpha: 5, z: 10 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.eps, 3);
            assert.equal(SVRjSon.batchSize, 1000);
            assert.equal(SVRjSon.maxIterations, 10000);
            assert.equal(SVRjSon.maxTime, 2);
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
        it("should throw an exception if the argument is not Json", function () {
            this.timeout(4000);
            var SVR = new analytics.SVR();
            assert.throws(function () {
                SVR.setParams(1);
            });
        })
        it("should throw an exception if there is no given argument", function () {
            var SVR = new analytics.SVR();
            assert.throws(function () {
                SVR.setParams();
            });
        })
    });

    describe("Weights tests", function () {
        it("should return an empty vector", function () {
            var SVR = new analytics.SVR();
            var Vec = SVR.weights;
            assert.equal(Vec.length, 0);
        })
        it("should return an empty vector even if the parameters have been changed", function () {
            var SVR = new analytics.SVR();
            SVR.setParams({ j: 3, maxTime: 2 });
            var Vec = SVR.weights;
            assert.equal(Vec.length, 0);
        })

    });

    describe("getModel tests", function () {
        it("should return parameters of the model", function () {
            var SVR = new analytics.SVR();
            var Model = SVR.getModel();
            assert.equal(Model.weights.length, 0);
        })
        it("should ignore extra parameters given to the function", function () {
            var SVR = new analytics.SVR();
            var Model = SVR.getModel(1);
            assert.equal(Model.weights.length, 0);
        })
    });

    describe("Fit", function () {
        it("should not throw an exception for correct values", function () {
            var matrix = new la.Matrix([[1, -1], [1, -1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR();

            assert.doesNotThrow(function () {
                SVR.fit(matrix, vector);
            });         
        })
        //degenerated example
        it("should return a fitted model for the degenerated example", function () {
            var matrix = new la.Matrix([[1, -1], [1, -1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR();
            SVR.fit(matrix, vector);
            var model = SVR.getModel();
            assert.eqtol(model.weights[0], 0, 1e-2);
            assert.eqtol(model.weights[1], 0, 1e-2);
        })
        //the tolerance in the test is equal to epsilon in SVR
        it("should return a fitted model", function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({c: 10});
            SVR.fit(matrix, vector);

            var model = SVR.getModel();
            assert.eqtol(model.weights[0], 0, 1e-1);
            assert.eqtol(model.weights[1], 1, 1e-1);
        })
        it('should make a model from one-dimensional vectors', function () {
            var matrix = new la.Matrix([[1, -1]]);
            var vector = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ c: 10 });
            SVR.fit(matrix, vector);

            var weights = SVR.weights;
            assert.eqtol(weights[0], 1, 1e-1);
        })
    });
})
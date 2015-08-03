// JavaScript source code
var analytics = require("qminer").analytics;
var assert = require("../../src/nodejs/scripts/assert.js");

//Unit test for SVC

describe("SVC test", function () {
    beforeEach(function () {
        
    });
    afterEach(function () {

    });

    describe("Constructor test", function () {
        it("It should return a default constructor", function () {
            var SVC = new analytics.SVC();
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        });
        it("It should return a SVC created by Json", function () {
            var SVC = new analytics.SVC({c:5, j:5, batchSize:5, maxIterations:5, maxTime: 1, minDiff: 1e-10, verbose: true});
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.c, 5);

            assert.equal(SVCjSon.j, 5);

            assert.equal(SVCjSon.batchSize, 5);

            assert.equal(SVCjSon.maxIterations, 5);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-10);

            assert.equal(SVCjSon.verbose, true);
        });

        it("It should return a SVC created by Json, not all key values are given", function () {
            var SVC = new analytics.SVC({ c: 5, batchSize: 5, maxTime: 1, verbose: true });
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.c, 5);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 5);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, true);
        });

        it("It should return a SVC created by an empty Json", function () {
            var SVC = new analytics.SVC({});
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations,10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        });

        it("It should return a SVC created by Json, with added key values", function () {
            var SVC = new analytics.SVC({alpha:5, beta:10, s:3, batchSize:10000, verbose: true});
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 10000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, true);
        });
    });

    describe("GetParams tests", function () {
        it("should return the parameters of the default SVC model as Json", function () {
            var SVC = new analytics.SVC();
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        })

        it("should return the parameters of the default SVC model as Json, without some key values", function () {
            var SVC = new analytics.SVC({ c: 3, j: 2, maxTime: 1 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.c, 3);

            assert.equal(SVCjSon.j, 2);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        })

        it("should return the parameters of the default SVC model as Json, with added key values", function () {
            var SVC = new analytics.SVC({ alpha: 3, beta: 3, z: 3 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 1);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 1);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        })
    });

    describe("SetParams tests", function () {
        it("should return the existing SVC with the changed values", function () {
            var SVC = new analytics.SVC();
            SVC.setParams({ j: 3, maxTime: 2 });

            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 3);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 2);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        })
        it("should return the existing SVC with the changed, added values", function () {
            var SVC = new analytics.SVC();
            SVC.setParams({ j: 3, maxTime: 2, alpha: 5, z: 10 });

            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.c, 1);

            assert.equal(SVCjSon.j, 3);

            assert.equal(SVCjSon.batchSize, 1000);

            assert.equal(SVCjSon.maxIterations, 10000);

            assert.equal(SVCjSon.maxTime, 2);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        })
        it("should throw an exception if the argument is not Json", function () {
            var SVC = new analytics.SVC();
            assert.throws(function () {
                SVC.setParams(1);
            });
        })
        it("should throw an exception if there is no given argument", function () {
            var SVC = new analytics.SVC();
            assert.throws(function () {
                SVC.setParams();
            });
        })
    });

    describe("Weights tests", function () {
        it("should return an empty vector", function () {
            var SVC = new analytics.SVC();
            var Vec = SVC.weights;
            assert.equal(Vec.length, 0);
        })
        it("should return an empty vector even if the parameters have been changed", function () {
            var SVC = new analytics.SVC();
            SVC.setParams({ j: 3, maxTime: 2 });
            var Vec = SVC.weights;
            assert.equal(Vec.length, 0);
        })

    });

    describe("getModel tests", function () {
        it("should return parameters of the model", function () {
            var SVC = new analytics.SVC();
            var Model = SVC.getModel();
            assert.equal(Model.weights.length, 0);
        })
        it("should ignore extra parameters given to the function", function () {
            var SVC = new analytics.SVC();
            var Model = SVC.getModel(1);
            assert.equal(Model.weights.length, 0);
        })
    });

    describe("Fit", function () {

    });
})
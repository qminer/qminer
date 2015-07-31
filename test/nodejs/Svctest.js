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

            assert.equal(SVCjSon.maxTime, 1000);

            assert.eqtol(SVCjSon.minDiff, 1e-6);

            assert.equal(SVCjSon.verbose, false);
        });
    });
})
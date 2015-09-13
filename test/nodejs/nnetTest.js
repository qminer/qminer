var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('qminer');
var analytics = qm.analytics;
var la = qm.la;

describe('NNet Tests', function () {
    
    describe('Constructor Tests', function () {
        it('should not throw an exception, default parameters', function () {
            assert.doesNotThrow(function () {
                var nnet = new analytics.NNet();
            });
        })
        it('should create a default constructor', function () {
            var nnet = new analytics.NNet();
            var params = nnet.getParams();
            assert.equal(params.layout[0], 2);
            assert.equal(params.layout[1], 3);
            assert.equal(params.layout[2], 2);
            assert.equal(params.learnRate, 0.1);
            assert.equal(params.momentum, 0.5);
            assert.equal(params.tFuncHidden, "tanHyper");
            assert.equal(params.tFuncOut, "tanHyper");
        })
        it('should not throw an exception, custom parameters', function () {
            assert.doesNotThrow(function () {
                var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10 });
            });
        })
        it('should create a custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.equal(params.layout[0], 4);
            assert.equal(params.layout[1], 3);
            assert.equal(params.layout[2], 2);
            assert.equal(params.learnRate, 1);
            assert.equal(params.momentum, 10);
            assert.equal(params.tFuncHidden, "sigmoid");
            assert.equal(params.tFuncOut, "tanHyper");
        })
        it('should throw an exception if no json is given as the parameter', function () {
            assert.throws(function () {
                var nnet = new analytics.NNet(0);
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception, default constructor', function () {
            var nnet = new analytics.NNet();
            assert.doesNotThrow(function () {
                var params = nnet.getParams();
            });
        })
        it('should give the parameters, default constructor', function () {
            var nnet = new analytics.NNet();
            var params = nnet.getParams();
            assert.equal(params.layout[0], 2);
            assert.equal(params.layout[1], 3);
            assert.equal(params.layout[2], 2);
            assert.equal(params.learnRate, 0.1);
            assert.equal(params.momentum, 0.5);
            assert.equal(params.tFuncHidden, "tanHyper");
            assert.equal(params.tFuncOut, "tanHyper");
        })
        it('should not throw an exception, custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            assert.doesNotThrow(function () {
                var params = nnet.getParams();
            });
        })
        it('should give the parameters, custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.equal(params.layout[0], 4);
            assert.equal(params.layout[1], 3);
            assert.equal(params.layout[2], 2);
            assert.equal(params.learnRate, 1);
            assert.equal(params.momentum, 10);
            assert.equal(params.tFuncHidden, "sigmoid");
            assert.equal(params.tFuncOut, "tanHyper");
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var nnet = new analytics.NNet();
            assert.doesNotThrow(function () {
                nnet.setParams({ layout: [3, 2, 1], momentum: 10 });
            });
        })
        it('should change the parameters of the model', function () {
            var nnet = new analytics.NNet();
            nnet.setParams({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.equal(params.layout[0], 4);
            assert.equal(params.layout[1], 3);
            assert.equal(params.layout[2], 2);
            assert.equal(params.learnRate, 1);
            assert.equal(params.momentum, 10);
            assert.equal(params.tFuncHidden, "sigmoid");
            assert.equal(params.tFuncOut, "tanHyper");
        })
        it('should throw an exception if no parameter is given', function () {
            var nnet = new analytics.NNet();
            assert.throws(function () {
                nnet.setParams();
            })
        });
    })
})

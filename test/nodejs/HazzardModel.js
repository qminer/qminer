var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

describe('HazardModel Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception, default parameters', function () {
            assert.doesNotThrow(function () {
                var hazard = new analytics.PropHazards();
            });
        })

        it('should create a model with default parameters', function () {
            var hazard = new analytics.PropHazards();
            var params = hazard.getParams();
            assert.equal(params.lambda, 0);
        })

        it('should not throw an exception, custom parameters', function () {
            assert.doesNotThrow(function () {
                var hazard = new analytics.PropHazards({ 'lambda': 2 });
            });
        })

        it('should create a model with custom parameters', function () {
            var hazard = new analytics.PropHazards({ 'lambda': 2 });
            var params = hazard.getParams();
            assert.equal(params.lambda, 2);
        })
        it.skip('should throw an exception if any other object is given', function () {
            assert.throws(function () {
                var hazard = new analytics.PropHazards(10);
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception, default parameters', function () {
            var hazard = new analytics.PropHazards();
            assert.doesNotThrow(function () {
                var params = hazard.getParams();
            });
        })
        it('should return the parameters of the model, default parameters', function () {
            var hazard = new analytics.PropHazards();
            var params = hazard.getParams();
            assert.equal(params.lambda, 0);
        })
        it('should not throw an exception, custom parameters', function () {
            var hazard = new analytics.PropHazards({ 'lambda': 10 });
            assert.doesNotThrow(function () {
                var params = hazard.getParams();
            });
        })
        it('should return the parameters of the model, custom parameters', function () {
            var hazard = new analytics.PropHazards({ 'lambda': 10 });
            var params = hazard.getParams();
            assert.equal(params.lambda, 10);
        })
        it('should get the same object for construction of a copy', function () {
            var hazard1 = new analytics.PropHazards({ 'lambda': 5 });
            var params1 = hazard1.getParams();
            var hazard2 = new analytics.PropHazards(params1);
            var params2 = hazard2.getParams();
            assert.equal(params1.lambda, params2.lambda);
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var hazard = new analytics.PropHazards();
            assert.doesNotThrow(function () {
                hazard.setParams({ 'lambda': 5 });
            });
        })
        it('should set the parameters of the model', function () {
            var hazard = new analytics.PropHazards();
            hazard.setParams({ 'lambda': 5 });
            var params = hazard.getParams();
            assert.equal(params.lambda, 5);
        })
        it('should throw an exception if no parameter is given', function () {
            var hazard = new analytics.PropHazards();
            assert.throws(function () {
                hazard.setParams();
            });
        })
    });

    describe('Weights Tests', function () {
        it('should not throw an exception', function () {
            var hazard = new analytics.PropHazards();
            assert.doesNotThrow(function () {
                var weights = hazard.weights;
            });
        })
        it('should return an empty vector', function () {
            var hazard = new analytics.PropHazards();
            var weights = hazard.weights;
            assert.equal(weights.length, 0);
        })
    })

})

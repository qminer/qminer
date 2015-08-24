var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

describe('Sigmoid Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var s = new analytics.Sigmoid();
            });
        })
        it('should create the default Sigmoid model', function () {
            var s = new analytics.Sigmoid();
            var model = s.getModel();
            assert.equal(model.A, 0);
            assert.equal(model.B, 0);
        })
        it('should create the default Sigmoid model if any parameter is given', function () {
            var s = new analytics.Sigmoid({ A: 5, B: 30 });
            var model = s.getModel();
            assert.equal(model.A, 0);
            assert.equal(model.B, 0);
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            assert.doesNotThrow(function () {
                var params = s.getParams();
            });
        })
        it('should return an empty json object', function () {
            var s = new analytics.Sigmoid();
            var params = s.getParams();
            assert.deepEqual(params, {});
        })
    })

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            assert.doesNotThrow(function () {
                s.setParams({});
            });
        })
        it('should not change the Sigmoid model', function () {
            var s = new analytics.Sigmoid();
            s.setParams({});
            var model = s.getModel();
            assert.equal(model.A, 0);
            assert.equal(model.B, 0);
        })
    });

    describe('GetModel Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            assert.doesNotThrow(function () {
                var model = s.getModel();
            });
        })
        it('should return the default model parameters', function () {
            var s = new analytics.Sigmoid();
            var model = s.getModel();
            assert.equal(model.A, 0);
            assert.equal(model.B, 0);
        })
        it('should return the model parameters', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var model = s.getModel();
            assert(model.A > 0);
            assert.equal(model.B, 0);
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            assert.doesNotThrow(function () {
                s.fit(X, y);
            });
        })
        it('should fit the model with the values, symmetric', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var model = s.getModel();
            assert(model.A > 0);
            assert.equal(model.B, 0);
        })
        it('should fit the model with the values, not symmetric', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([0, 1, 2, 3, 4, 5]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var model = s.getModel();
            assert(model.B > 0);
        })
        it('should fit the model with the values, symmetric', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([3, 2, 1, -1, -2, -3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var model = s.getModel();
            assert(model.A < 0);
            assert.equal(model.B, 0);
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            assert.doesNotThrow(function () {
                var prediction = s.predict(0);
            });
        })
        it('should return the prediction probability of 0', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var prediction = s.predict(0);
            assert.eqtol(prediction, 0.5);
        })
        it('should return the prediction of the vector of values', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var test = new la.Vector([-3, 0, 3]);
            var prediction = s.predict(test);
            assert(prediction[0] < 0.10);
            assert.eqtol(prediction[1], 0.5);
            assert(prediction[2] > 0.90);
        })
    });

    describe('DecisionFunction Tests', function () {
        it('should not throw an exception', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            assert.doesNotThrow(function () {
                var prediction = s.decisionFunction(0);
            });
        })
        it('should return the prediction probability of 0', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var prediction = s.decisionFunction(0);
            assert.eqtol(prediction, 0.5);
        })
        it('should return the prediction of the vector of values', function () {
            var s = new analytics.Sigmoid();
            var X = new la.Vector([-3, -2, -1, 1, 2, 3]);
            var y = new la.Vector([-1, -1, -1, 1, 1, 1]);
            s.fit(X, y);
            var test = new la.Vector([-3, 0, 3]);
            var prediction = s.decisionFunction(test);
            assert(prediction[0] < 0.10);
            assert.eqtol(prediction[1], 0.5);
            assert(prediction[2] > 0.90);
        })
    });
});
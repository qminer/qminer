
var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

describe('RecursiveLinearRegression Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
            });
        })
        it('should create an object with mostly default params', function () {
            var linreg = new analytics.RecLinReg({ dim: 10 });
            var param = linreg.getParams();
            assert.equal(param.dim, 10);
            assert.equal(param.regFact, 1.0);
            assert.equal(param.forgetFact, 1.0);
        })
        it('should create an object out of the parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 10.0 });
            var param = linreg.getParams();
            assert.equal(param.dim, 10);
            assert.equal(param.regFact, 2.0);
            assert.equal(param.forgetFact, 10.0);
        })
        it('should cast the float value of dim to int', function () {
            var linreg = new analytics.RecLinReg({ dim: 12.5 });
            var param = linreg.getParams();
            assert.equal(param.dim, 13);
            assert.equal(param.regFact, 1.0);
            assert.equal(param.forgetFact, 1.0);
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
            assert.doesNotThrow(function () {
                var param = linreg.getParams();
            });
        })
        it('should return the parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 1.0, forgetFact: 1.0 });
            var param = linreg.getParams();
            assert.equal(param.dim, 10);
            assert.equal(param.regFact, 1.0);
            assert.equal(param.forgetFact, 1.0);
        })
        it('should return the parameters for construction of the same object', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            var param = linreg.getParams();
            var second = new analytics.RecLinReg(param);
            var param2 = second.getParams();

            assert.equal(param.dim, param2.dim);
            assert.equal(param.regFact, param2.regFact);
            assert.equal(param.forgetFact, param.forgetFact);
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            assert.doesNotThrow(function () {
                linreg.setParams({ dim: 1.0 });
            });
        })
        it('should change some of the parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            linreg.setParams({ dim: 2, regFact: 1e-10 });
            var param = linreg.getParams();
            assert.equal(param.dim, 2);
            assert.eqtol(param.regFact, 1e-10);
            assert.eqtol(param.forgetFact, 15.0);
        })
        it('should throw an exception if there are no parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            assert.throws(function () {
                linreg.setParams();
            });
        })
    })

    describe('Dim Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            assert.doesNotThrow(function () {
                var dim = linreg.dim;
            });
        })
        it('should return the dimensionality', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            var dim = linreg.dim;
            assert.equal(dim, 10);
        })
    });

    describe('PartialFit', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 2.0, forgetFact: 15.0 });
            var vec = new la.Vector([1, 2]);
            assert.doesNotThrow(function () {
                linreg.partialFit(vec, 3);
            });
        })
        it('should make the model by partial fitting one test vector', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var vec = new la.Vector([1, 1]);
            linreg.partialFit(vec, 3);

            var weights = linreg.weights;
            assert.eqtol(weights[0], 1.5);
            assert.eqtol(weights[1], 1.5);
        })
        it('should make a new model by partial fitting two test vectors', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var vec = new la.Vector([1, 1]);
            linreg.partialFit(vec, 3);

            var vec2 = new la.Vector([2, -1]);
            linreg.partialFit(vec2, 3);

            var weights = linreg.weights;
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);
        })
        it('should throw an exception if the vector is longer than the dimension of the model', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var vec = new la.Vector([1, 1, 1]);
            assert.throws(function () {
                linreg.partialFit(vec, 3);
            });
        })
        it('should throw an excpetion if the vector is shorter than the dimension of the model', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var vec = new la.Vector([1]);
            assert.throws(function () {
                linreg.partialFit(vec, 3);
            });
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 2.0, forgetFact: 15.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);

            assert.doesNotThrow(function () {
                linreg.fit(X, y);
            });
        })
        it('should make a new model by fitting the matrix', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);

            linreg.fit(X, y);
            var weights = linreg.weights;
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);
        })
        it('should throw an exception if the number of rows exceed the dimension', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1], [1, 1]]);
            var y = new la.Vector([3, 3]);

            assert.throws(function () {
                linreg.fit(X, y);
            });
        })
        it('should throw an exception if the number of rows is less than the dimension', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2]]);
            var y = new la.Vector([3, 3]);

            assert.throws(function () {
                linreg.fit(X, y);
            });
        })
    });

    describe('GetModel Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            assert.doesNotThrow(function () {
                var model = linreg.getModel();
            });
        })
        it('should return the default values of the model', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var model = linreg.getModel();
            assert.equal(model.weights.length, 2);
            assert.equal(model.weights[0], 0);
            assert.equal(model.weights[1], 0);
        })
        it('should return the model', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);

            linreg.fit(X, y);
            var model = linreg.getModel();
            assert.eqtol(model.weights.length, 2);
            assert.eqtol(model.weights[0], 2);
            assert.eqtol(model.weights[1], 1);
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);
            linreg.fit(X, y);

            var test = new la.Vector([1, 1]);
            assert.doesNotThrow(function () {
                var prediction = linreg.predict(test);
            });
        })
        it('should predict the values of the model', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);
            linreg.fit(X, y);

            var test = new la.Vector([1, 1]);
            var prediction = linreg.predict(test);
            assert.eqtol(prediction, 3, 1e-6);
        })
        it('should throw an exception if there are no parameters given', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);
            linreg.fit(X, y);
            assert.throws(function () {
                linreg.predict();
            });
        })
    })

})
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('../../index.js').analytics;
var la = require('../../index.js').la;

describe('Ridge Regression Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception, empty constructor', function () {
            assert.doesNotThrow(function () {
                var RR = new analytics.RidgeReg();
            });
        })
        it('should create a new default ridge regression model', function () {
            var RR = new analytics.RidgeReg();
            var param = RR.getParams();
            assert.equal(param.gamma, 0);
        })
        it('should not throw an exception, json parameter', function () {
            assert.doesNotThrow(function () {
                var RR = new analytics.RidgeReg({ gamma: 5 });
            });
        })
        it('should create a new ridge regression, json parameter', function () {
            var RR = new analytics.RidgeReg({ gamma: 5 });
            var param = RR.getParams();
            assert.equal(param.gamma, 5);
        })
        it('should create a default ridge regression if json doesn\'t have the gamma key', function () {
            var RR = new analytics.RidgeReg({ alpha: 5 });
            var param = RR.getParams();
            assert.equal(param.gamma, 0);
        })
        it('should throw an exception if the parameter is not valid', function () {
            assert.throws(function () {
                var RR = new analytics.RidgeReg('gamma');
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception', function () {
            var RR = new analytics.RidgeReg();
            assert.doesNotThrow(function () {
                var param = RR.getParams();
            })
        })
        it('should return the parameters of the model', function () {
            var RR = new analytics.RidgeReg();
            var param = RR.getParams();
            assert.equal(param.gamma, 0);
        })
        it('should throw an exception if parameters are given', function () {
            var RR = new analytics.RidgeReg();
            assert.throws(function () {
                var param = RR.getParams(5);
            });
        })
        it('should be used for construction of a new Ridge Regression model', function () {
            var RR = new analytics.RidgeReg({ gamma: 4 });
            var param = RR.getParams();
            var otherRR = new analytics.RidgeReg(param);
            var otherParam = otherRR.getParams();
            assert.eqtol(otherParam.gamma, 4);
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception, Json parameter', function () {
            var RR = new analytics.RidgeReg();
            assert.doesNotThrow(function () {
                RR.setParams({ gamma: 5 });
            });
        })
        it('should set the parameter gamma to 5, Json parameter', function () {
            var RR = new analytics.RidgeReg();
            RR.setParams({ gamma: 5 });
            var param = RR.getParams();
            assert.equal(param.gamma, 5);
        })
        it('should throw an exception if the parameter is not valid', function () {
            var RR = new analytics.RidgeReg();
            assert.throws(function () {
                RR.setParams("something");
            });
        })
        it('does not changes the model', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            var weights = RR.weights;
            assert.eqtol(weights.length, 2);
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);

            RR.setParams({ gamma: 5 });
            weights = RR.weights;
            assert.eqtol(weights.length, 2);
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, -2], [2, 0], [3, 0], [0, 2], [1, 1]]);
            var b = new la.Vector([3, 2]);

            assert.doesNotThrow(function () {
                RR.fit(A, b);
            });
        })
        it('should fit the model to the test values, default parameters', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);

            RR.fit(A, b);
            var weights = RR.weights;
            assert.eqtol(weights.length, 2);
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);
        })
        it('should fit the model to the test values, Json parameters', function () {
            var RR = new analytics.RidgeReg({ gamma: 1 });
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);

            RR.fit(A, b);
            var weights = RR.weights;
            assert.equal(weights.length, 2);
            assert.eqtol(weights[0], 1.588235294117647);
            assert.eqtol(weights[1], 0.529411764705883);
        })
        it('should throw an exception if the number of columns and the length of the vector are not equal', function () {
            var RR = new analytics.RidgeReg({ gamma: 1 });
            var A = new la.Matrix([[1, 2, 3], [1, -1, 2]]);
            var b = new la.Vector([3, 3]);

            assert.throws(function () {
                RR.fit(A, b);
            });
        })
        it('should forget the previous models', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);

            RR.fit(A, b);
            var weights = RR.weights;
            assert.eqtol(weights.length, 2);
            assert.eqtol(weights[0], 2);
            assert.eqtol(weights[1], 1);

            var A2 = new la.Matrix([[1, 1], [2, -1]]);
            var b2 = new la.Vector([3, 3]);
            RR.fit(A2, b2);
            weights = RR.weights;
            assert.eqtol(weights.length, 2);
            assert.eqtol(weights[0], 3);
            assert.eqtol(weights[1], 0);
        })
    });

    describe('GetModel Tests', function () {
        it('should not throw an exception', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);

            assert.doesNotThrow(function () {
                var model = RR.getModel();
            });
        })
        it('should return the model, no fit previously made', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);

            var model = RR.getModel();
            assert.eqtol(model.weights.length, 0);
        })
        it('should return the model, default parameters', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);

            var model = RR.getModel();
            assert.eqtol(model.weights.length, 2);
            assert.eqtol(model.weights[0], 2.0);
            assert.eqtol(model.weights[1], 1.0);
        })
        it('should return the model, Json parameters', function () {
            var RR = new analytics.RidgeReg({ gamma: 1 });
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);

            RR.fit(A, b);
            var model = RR.getModel();
            assert.equal(model.weights.length, 2);
            assert.eqtol(model.weights[0], 1.588235294117647);
            assert.eqtol(model.weights[1], 0.529411764705883);
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            var vec = new la.Vector([3, 4]);
            assert.doesNotThrow(function () {
                var prediction = RR.predict(vec);
            });
        })
        it('should return the prediction of the vector [3, 4]', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            var vec = new la.Vector([3, 4]);
            var prediction = RR.predict(vec);
            assert.eqtol(prediction, 10);
        })
        it('should throw an exception if the vector is longer than the model', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            var vec = new la.Vector([3, 4, 5]);
            assert.throws(function () {
                var prediction = RR.predict(vec);
            });
        })
        it('should throw an exception if the vector is shorter than the model', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            var vec = new la.Vector([3]);
            assert.throws(function () {
                var prediction = RR.predict(vec);
            });
        })
    });
    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var RR = new analytics.RidgeReg();
            var A = new la.Matrix([[1, 2], [1, -1]]);
            var b = new la.Vector([3, 3]);
            RR.fit(A, b);
            RR.save(require('../../index.js').fs.openWrite('ridgereg_test.bin')).close();
            var RR2 = new analytics.RidgeReg(require('../../index.js').fs.openRead('ridgereg_test.bin'));
            assert.deepEqual(RR.getParams(), RR2.getParams());
            assert.eqtol(RR.weights.minus(RR2.weights).norm(), 0, 1e-8);
        })
    });
});

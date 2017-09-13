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

//Unit test for LIBSVM SVR

describe("LIBSVM SVR test", function () {

    describe("Constructor test", function () {
        it("It should return a default constructor", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        });
        it("It should return a SVR created by Json", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 5, eps: 5, batchSize: 5, maxIterations: 5, maxTime: 1, minDiff: 1e-10, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 5);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 5);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 5); // XXX
            assert.equal(SVRjSon.maxIterations, 5); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-10);
            assert.equal(SVRjSon.verbose, true);
        });

        it("It should return a SVR created by Json, not all key values are given", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 5, batchSize: 5, maxTime: 1, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 5);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 5); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, true);
        });

        it("It should return a SVR created by an empty Json", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        });

        it("It should return a SVR created by Json, with added key values", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", alpha: 5, beta: 10, s: 3, batchSize: 10000, verbose: true });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 10000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, true);
        });
    });

    describe("GetParams tests", function () {
        it("should return the parameters of the default SVR model as Json", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })

        it("should return the parameters of the default SVR model as Json, without some key values", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 3, eps: 2, maxTime: 1 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 3);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 2);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })

        it("should return the parameters of the default SVR model as Json, with added key values", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", alpha: 3, beta: 3, z: 3 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 0.001);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 1); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
    });

    describe("SetParams tests", function () {
        it("should return the existing SVR with the changed values", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.setParams({ eps: 3, maxTime: 2 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 3);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 2); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
        it("should return the existing SVR with the changed, added values", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.setParams({ eps: 3, maxTime: 2, alpha: 5, z: 10 });
            var SVRjSon = SVR.getParams();
            assert.equal(SVRjSon.kernel, "LINEAR");
            assert.equal(SVRjSon.svmType, "default");
            assert.equal(SVRjSon.c, 1);
            assert.equal(SVRjSon.j, 1);
            assert.equal(SVRjSon.eps, 3);
            assert.equal(SVRjSon.gamma, 1);
            assert.equal(SVRjSon.p, 0.1);
            assert.equal(SVRjSon.degree, 1);
            assert.equal(SVRjSon.nu, 0.01);
            assert.equal(SVRjSon.coef0, 1);
            assert.equal(SVRjSon.cacheSize, 100);
            assert.equal(SVRjSon.batchSize, 1000); // XXX
            assert.equal(SVRjSon.maxIterations, 10000); // XXX
            assert.equal(SVRjSon.maxTime, 2); // XXX
            assert.eqtol(SVRjSon.minDiff, 1e-6);
            assert.equal(SVRjSon.verbose, false);
        })
        it("should throw an exception if the argument is not Json", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            assert.throws(function () {
                SVR.setParams(1);
            });
        })
        it("should throw an exception if there is no given argument", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            assert.throws(function () {
                SVR.setParams();
            });
        })
    });

    describe("Weights tests", function () {
        it("should return an empty vector", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var Vec = SVR.weights;
            assert.equal(Vec.length, 0);
        })
        it("should return an empty vector even if the parameters have been changed", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.setParams({ j: 3, maxTime: 2 });
            var Vec = SVR.weights;
            assert.equal(Vec.length, 0);
        })

    });
    
    describe("Bias tests", function () {
        it("should return zero", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var num = SVR.bias;
            assert.equal(num, 0);
        })
        it("should return zero even if the parameters have been changed", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.setParams({ j: 3, maxTime: 2 });
            var num = SVR.bias;
            assert.equal(num, 0);
        })

    });
    
    describe("getModel tests", function () {
        it("should return parameters of the model", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var Model = SVR.getModel();
            assert.equal(Model.weights.length, 0);
            assert.equal(Model.bias, 0);
        })
        it("should ignore extra parameters given to the function", function () {
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            var Model = SVR.getModel(1);
            assert.equal(Model.weights.length, 0);
            assert.equal(Model.bias, 0);
        })
    });

    describe("Fit Tests", function () {
        it("should not throw an exception for correct values", function () {
            var matrix = new la.Matrix([[1, -1], [1, -1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });

            assert.doesNotThrow(function () {
                SVR.fit(matrix, vector);
            });         
        })
        //degenerated example
        it("should return a fitted model for the degenerated example", function () {
            var matrix = new la.Matrix([[1, -1], [1, -1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.fit(matrix, vector);

            var model = SVR.getModel();
            assert.eqtol(model.weights[0], 0, 1e-2);
            assert.eqtol(model.weights[1], 0, 1e-2);
        })
        //the tolerance in the test is equal to epsilon in SVR
        it("should return a fitted model", function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var model = SVR.getModel();
            assert.eqtol(model.weights[0], 0, 1e-1);
            assert.eqtol(model.weights[1], 0, 1e-1);
        })
        //not passing
        it('should make a model from one-dimensional vectors', function () {
            var matrix = new la.Matrix([[1, -1]]);
            var vector = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);
            
            var weights = SVR.weights;
            assert.eqtol(weights[0], 1, 3e-1);
        })
        it('should throw an exception if the number of matrix cols and the vector length aren\'t equal', function () {
            var matrix = new la.Matrix([[1, -1, 2], [1, -2, 3]]);
            var vector = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });

            assert.throws(function () {
                SVR.fit(matrix, vector);
            });
        })
        //not passing
        it('should forget the previous model', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 100 });
            SVR.fit(matrix, vector);

            var weights = SVR.weights;
            assert.eqtol(weights[0], 0, 1e-1);
            assert.eqtol(weights[1], 0, 1e-1);

            var matrix2 = new la.Matrix([[1, -1]]);
            var vec2 = new la.Vector([1, -1]);
            SVR.fit(matrix2, vec2);

            weights = SVR.weights;
            assert.eqtol(weights[0], 1, 1e-1);

        })
        // test for setParams
        it('should not change the model', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var model = SVR.getModel();
            assert.eqtol(model.weights[0], 0, 1e-1);
            assert.eqtol(model.weights[1], 0, 1e-1);

            // changing the parameters
            SVR.setParams({ c: 100, maxTime: 12000 });
            var weights = SVR.weights;
            assert.eqtol(weights[0], 0, 1e-1);
            assert.eqtol(weights[1], 0, 1e-1);
        })
    });
    describe('Predict Tests', function () {
        it('should not throw an exception for giving the correct values', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([2, 3]);
            assert.doesNotThrow(function () {
                SVR.predict(vec2);
            });
        })
        it('should return the prediction of the vector [2, 3]', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);
            // Note that since all targets are 1, it suffices to set w=0 and b=-1 to solve w*x-b = y.
            var vec2 = new la.Vector([2, 3]);
            var prediction = SVR.predict(vec2);

            assert.eqtol(prediction, 1, 5e-1);
        })
        it('should throw an exception if the given vector is to long', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            assert.throws(function () {
                SVR.predict(new la.Vector([1, 2, 3]));
            });
        })
        it('should throw an exception if the given vector is to short', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            assert.throws(function () {
                SVR.predict(new la.Vector([1]));
            });
        })
        it('should throw an exception if the given vector is empty', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            assert.throws(function () {
                SVR.predict(new la.Vector());
            });
        })
        //not passing
        it('should return the prediction of the one-dimensional vector', function () {
            var matrix = new la.Matrix([[1, -1]]);
            var vector = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([3]);
            var prediction = SVR.predict(vec2);
            assert.eqtol(prediction, 3, 5e-1);
        })
        // if fit is not previously used
        it('should throw an exception if the fit is not used', function () {
            var SVR = new analytics.SVR({ c: 10 });
            var vector = new la.Vector([1, 1]);
            assert.throws(function () {
                SVR.predict(vector);
            });
        })
        it('should return the predictions for the matrix', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var mat2 = new la.Matrix([[2, -3, 5], [1, -4, 0.3]]);
            var prediction = SVR.predict(mat2);

            assert.eqtol(prediction[0], 1, 5e-1);
            assert.eqtol(prediction[1], 1, 5e-1);
            assert.eqtol(prediction[2], 1, 5e-1);
        })
        it('should throw an exception if there are too many rows in the matrix', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var mat2 = new la.Matrix([[2, -3, 5], [1, -4, 0.3], [0, 0, 1]]);
            assert.throws(function () {
                SVR.predict(mat2);
            });
        })
        it('should throw an exception if there are too lesser rows in the matrix', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var mat2 = new la.Matrix([[2, -3, 5]]);
            assert.throws(function () {
                SVR.predict(mat2);
            });
        })
    });
    describe('DecisionFunction Tests', function () {
        it('should not return an exception for the given correct values', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([2, 3]);
            assert.doesNotThrow(function () {
                SVR.decisionFunction(vec2);
            })

        })
        it('should return the distance from the model', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([2, 3]);
            var distance = SVR.decisionFunction(vec2);

            assert.eqtol(distance, 1, 5e-1);
        })
        //not passing
        it('should return the distance of the one-dimensional vector from the model', function () {
            var matrix = new la.Matrix([[1, -1]]);
            var vector = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([0.5]);
            var distance = SVR.decisionFunction(vec2);

            assert.eqtol(distance, 0.5, 1e-1);
        })
        it('should return the distance of the matrix from the model', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var matrix2 = new la.Matrix([[1, 2, 3], [-2, 3, -0.5]]);
            var distance = SVR.decisionFunction(matrix2);

            assert.eqtol(distance[0], 1, 5e-1);
            assert.eqtol(distance[1], 1, 5e-1);
            assert.eqtol(distance[2], 1, 5e-1);
        })
        it('should throw an exception if the vector is too long', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([2, 3, 4]);
            assert.throws(function () {
                SVR.decisionFunction(vec2);
            });
        })
        it('should throw an exception if the vector is too short', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var vec2 = new la.Vector([2]);
            assert.throws(function () {
                SVR.decisionFunction(vec2);
            });
        })
        it('should throw an exception if the matrix has to many rows', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var matrix2 = new la.Matrix([[1, 2, 3], [2, 3, -4], [-1, 0, 0]]);
            assert.throws(function () {
                SVR.decisionFunction(matrix2);
            })
        })
        it('should throw an exception if the matrix has too lesser of rows', function () {
            var matrix = new la.Matrix([[1, -1], [1, 1]]);
            var vector = new la.Vector([1, 1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM", c: 10 });
            SVR.fit(matrix, vector);

            var matrix2 = new la.Matrix([[1, 2, 3]]);
            assert.throws(function () {
                SVR.decisionFunction(matrix2);
            });
        })
    });
    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVR = new analytics.SVR({ algorithm: "LIBSVM" });
            SVR.fit(matrix, vec);
            SVR.save(require('qminer').fs.openWrite('svc_test.bin')).close();
            var SVR2 = new analytics.SVR(require('qminer').fs.openRead('svc_test.bin'));
            assert.deepEqual(SVR.getParams(), SVR2.getParams());
            assert.eqtol(SVR.weights.minus(SVR2.weights).norm(), 0, 1e-8);
        })
    });
    describe('Kernel tests', function () {
        it('should find a fit with polynomial kernel', function () {
             X = [[-3],
                  [-2],
                  [-1],
                  [0],
                  [1],
                  [2],
                  [3]];
             var y = [9, 4, 1, 0, 4, 1, 9];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             // unbalance: positive examples are 1000 times more important
             var SVR = new analytics.SVR({ algorithm: "LIBSVM", kernel: "POLY", degree: 2, p: 10e-3 });
             SVR.fit(matrix, vec);
             assert.eqtol(SVR.predict(matrix).minus(new la.Vector([9, 4, 1, 0, 1, 4, 9])).norm(), 0, 1e-1);
         });
        it('should find a fit with RBF kernel', function () {
             X = [[1, 0],
                  [0, 1],
                  [-1, 0],
                  [0, -1],
                  [2, 0],
                  [0, 2],
                  [-2, 0],
                  [0, -2]];
             var y = [1, 1, 1, 1, 2, 2, 2, 2];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             // unbalance: positive examples are 1000 times more important
             var SVR = new analytics.SVR({ algorithm: "LIBSVM", kernel: "RBF", p: 10e-3 });
             SVR.fit(matrix, vec);
             assert.eqtol(SVR.predict(matrix).minus(new la.Vector([1, 1, 1, 1, 2, 2, 2, 2])).norm(), 0, 1e-1);
         });
    });
})

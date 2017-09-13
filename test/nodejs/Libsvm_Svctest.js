/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

// JavaScript source code
var analytics = require("qminer").analytics;
var la = require('qminer').la;
var assert = require("../../src/nodejs/scripts/assert.js");

//Unit test for LIBSVM SVC

describe("LIBSVM SVC test", function () {

    describe("Constructor test", function () {
        it("It should return a default constructor", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        });
        it("It should return a SVC created by Json", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM", kernel: "RBF", svmType: "NU_SVC", c: 20, j: 0.4, eps: 0.15, gamma:1.5, p: 0.05, degree: 3, nu: 0.1, coef0: 0.5, cacheSize: 20, batchSize: 5, maxIterations: 5, maxTime: 1, minDiff: 1e-10, verbose: true });
            var SVCjSon = SVC.getParams();
            assert.equal(SVCjSon.kernel, "RBF");
            assert.equal(SVCjSon.svmType, "NU_SVC");
            assert.equal(SVCjSon.c, 20);
            assert.equal(SVCjSon.j, 0.4);
            assert.equal(SVCjSon.eps, 0.15);
            assert.equal(SVCjSon.gamma, 1.5);
            assert.equal(SVCjSon.p, 0.05);
            assert.equal(SVCjSon.degree, 3);
            assert.equal(SVCjSon.nu, 0.1);
            assert.equal(SVCjSon.coef0, 0.5);
            assert.equal(SVCjSon.cacheSize, 20);
            assert.equal(SVCjSon.batchSize, 5); // XXX
            assert.equal(SVCjSon.maxIterations, 5); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-10);
            assert.equal(SVCjSon.verbose, true);
        });

        it("It should return a SVC created by Json, not all key values are given", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 5, batchSize: 5, maxTime: 1, verbose: true });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 5);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 5); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, true);
        });

        it("It should return a SVC created by an empty Json", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var SVCjSon = SVC.getParams();
            
            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        });

        it("It should return a SVC created by Json, with added key values", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM", alpha: 5, beta: 10, s: 3, batchSize: 10000, verbose: true });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 10000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, true);
        });
    });

    describe("GetParams tests", function () {
        it("should return the parameters of the default SVC model as Json", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        })

        it("should return the parameters of the default SVC model as Json, without some key values", function () {
            var SVC = new analytics.SVC({ c: 3, j: 2, maxTime: 1 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 3);
            assert.equal(SVCjSon.j, 2);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        })

        it("should return the parameters of the default SVC model as Json, with added key values", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM", alpha: 3, beta: 3, z: 3 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 1);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 1); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        })
    });

    describe("SetParams tests", function () {
        it("should return the existing SVC with the changed values", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.setParams({ j: 3, maxTime: 2 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 3);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 2); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        })
        it("should return the existing SVC with the changed, added values", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.setParams({ j: 3, maxTime: 2, alpha: 5, z: 10 });
            var SVCjSon = SVC.getParams();

            assert.equal(SVCjSon.kernel, "LINEAR");
            assert.equal(SVCjSon.svmType, "default");
            assert.equal(SVCjSon.c, 1);
            assert.equal(SVCjSon.j, 3);
            assert.equal(SVCjSon.eps, 0.001);
            assert.equal(SVCjSon.gamma, 1);
            assert.equal(SVCjSon.p, 0.1);
            assert.equal(SVCjSon.degree, 1);
            assert.equal(SVCjSon.nu, 0.01);
            assert.equal(SVCjSon.coef0, 1);
            assert.equal(SVCjSon.cacheSize, 100);
            assert.equal(SVCjSon.batchSize, 1000); // XXX
            assert.equal(SVCjSon.maxIterations, 10000); // XXX
            assert.equal(SVCjSon.maxTime, 2); // XXX
            assert.eqtol(SVCjSon.minDiff, 1e-6);
            assert.equal(SVCjSon.verbose, false);
        })
        it("should throw an exception if the argument is not Json", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            assert.throws(function () {
                SVC.setParams(1);
            });
        })
        it("should throw an exception if there is no given argument", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            assert.throws(function () {
                SVC.setParams();
            });
        })
    });

    describe("Weights tests", function () {
        it("should return an empty vector", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var Vec = SVC.weights;
            assert.equal(Vec.length, 0);
        })
        it("should return an empty vector even if the parameters have been changed", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.setParams({ j: 3, maxTime: 2 });
            var Vec = SVC.weights;
            assert.equal(Vec.length, 0);
        })

    });
    
    describe("Bias tests", function () {
        it("should return zero", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var num = SVC.bias;
            assert.equal(num, 0);
        })
        it("should return zero even if the parameters have been changed", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.setParams({ j: 3, maxTime: 2 });
            var num = SVC.bias;
            assert.equal(num, 0);
        })

    });
    
    describe("GetModel tests", function () {
        it("should return parameters of the model", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var Model = SVC.getModel();
            assert.equal(Model.weights.length, 0);
            assert.equal(Model.bias, 0);
        })
        it("should ignore extra parameters given to the function", function () {
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            var Model = SVC.getModel(1);
            assert.equal(Model.weights.length, 0);
            assert.equal(Model.bias, 0);
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception when given the matrix and vector', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });

            assert.doesNotThrow(function () {
                SVC.fit(matrix, vec);
            });
        })
        it('should create a model out of the matrix and vector', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });

            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 1, 1e-3);
        })
        it('should throw an exception if the number of matrix columns and vector length are not equal', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            assert.throws(function () {
                SVC.fit(matrix, vec);
            });
        })
        it('should return a close-zero model and print a WARNING (class label 1 not found)', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([-1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });

            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 0, 1e-2);
            assert.eqtol(model.weights[1], 0, 1e-2);
        })
        it('should forget the previous model', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            // first model
            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 1, 1e-3);

            var matrix2 = new la.Matrix([[1, -1], [0, 0]]);
            var vec2 = new la.Vector([1, -1]);
            //second model
            SVC.fit(matrix2, vec2);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 0, 1e-3);
        })
        // testing getModel with fit
        it('should return the fitted model', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });

            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 0, 1e-3);
        })
        // testing setParams
        it('shouldn\'t change the model, when setting new parameters', function () {
            // creating the model
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            // setting the parameters
            SVC.setParams({ c: 10, j: 5, maxTime: 2 });
            // seeing if the model is unchanged
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 0, 1e-3);
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.doesNotThrow(function () {
                SVC.predict(vec2);
            });
        })
        it('should return 1 for the given vector', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.equal(SVC.predict(vec2), 1);
        })
        it('should throw an exception if the vector is longer', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0, 1]);
            assert.throws(function () {
                SVC.predict(vec2);
            });
        })
        it('should throw an exception if the vector is shorter', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3]);
            assert.throws(function () {
                SVC.predict(vec2);
            });
        })
        it('should return the vector [1, 1, -1] for the given matrix', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, 3, -1], [0, 3, -2]]);
            var predicted = SVC.predict(matrix2);

            assert.equal(predicted[0], 1);
            assert.equal(predicted[1], 1);
            assert.equal(predicted[2], -1);
        })
        it('should throw an exception if the matrix has too many rows', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, 3, -1], [0, 3, -2], [1, 1, 2]]);
            assert.throws(function () {
                SVC.predict(matrix2);
            });
        })
        it('should throw an exception if the matrix has too lesser rows', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, 3, -1]]);
            assert.throws(function () {
                SVC.predict(matrix2);
            });
        })
        it('should throw an exception if the parameters are wrong', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, 3, -1]]);
            assert.throws(function () {
                SVC.predict(matrix2);
            });
        })
    });
    describe('DecisionFunction Tests', function () {
        it('should not throw an exception', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.doesNotThrow(function () {
                SVC.decisionFunction(vec2);
            });
        })
        it('should return the distance of the vector from the hyperplane', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([1, 1]);
            var distance = SVC.decisionFunction(vec2);

            assert.eqtol(distance, 1, 1e-3);
        })
        it('should throw an exception if the vector is too long', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);
            var vec2 = new la.Vector([1, 1, -1]);

            assert.throws(function () {
                SVC.decisionFunction(vec2);
            });
        })
        it('should throw an exception if the vector is too short', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);
            var vec2 = new la.Vector([1]);

            assert.throws(function () {
                SVC.decisionFunction(vec2);
            });
        })
        it('should return a vector of distances if the given parameter is a matrix', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, -1, 0], [2, 1, -3]]);
            var distance = SVC.decisionFunction(matrix2);

            assert.eqtol(distance[0], 1, 1e-3);
            assert.eqtol(distance[1], -1, 1e-3);
            assert.eqtol(distance[2], 0, 1e-2);
        })
        it('should throw an exception if the matrix has too many rows', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, -1, 0], [2, 1, -3], [1, -2, 0]]);
            assert.throws(function () {
                SVC.decisionFunction(matrix2);
            });
        })
        it('should throw an exception if the matrix has too lesser or rows', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, -1, 0]]);
            assert.throws(function () {
                SVC.decisionFunction(matrix2);
            });
        })
    });
    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC({ algorithm: "LIBSVM" });
            SVC.fit(matrix, vec);
            SVC.save(require('qminer').fs.openWrite('svr_test.bin')).close();
            var SVC2 = new analytics.SVC(require('qminer').fs.openRead('svr_test.bin'));
            assert.deepEqual(SVC.getParams(), SVC2.getParams());
            assert.eqtol(SVC.weights.minus(SVC2.weights).norm(), 0, 1e-8);
            assert.eqtol(Math.abs(SVC.bias - SVC2.bias), 0, 1e-8);
        })
    });
    describe('Fitting tests', function () {
        it('should fit a model on iris dataset (dense), class=setosa, features:sepal length, sepal width', function () {
             var X = require('./irisX.json');
             var y = require('./irisY.json');
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 10000 });
             SVC.fit(matrix, vec);
             // based on matlab
             //load fisheriris
             //X = [meas(:,1), meas(:,2)];
             //Y = nominal(ismember(species,'setosa'));
             //y = 2*(double(Y)-1.5);
             //s=fitcsvm(X,y,'Standardize',false, 'BoxConstraint',10000);
             // weights = s.SupportVectors'* (s.Alpha.*(y(s.IsSupportVector)))
             // bias = s.Bias
             assert.eqtol(SVC.weights.minus(new la.Vector([-8.5680, 7.1408])).norm(), 0, 1e-3);
             assert.eqtol(Math.abs(SVC.bias - 23.1314), 0, 1e-3);
         });
         it('should fit a model on iris dataset (sparse), class=setosa, features:sepal length, sepal width', function () {
             var X = require('./irisX.json');
             var y = require('./irisY.json');
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var spMatrix = matrix.sparse();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 10000 });
             SVC.fit(spMatrix, vec);
             // based on matlab
             //load fisheriris
             //X = [meas(:,1), meas(:,2)];
             //Y = nominal(ismember(species,'setosa'));
             //y = 2*(double(Y)-1.5);
             //s=fitcsvm(X,y,'Standardize',false, 'BoxConstraint',10000);
             // weights = s.SupportVectors'* (s.Alpha.*(y(s.IsSupportVector)))
             // bias = s.Bias
             assert.eqtol(SVC.weights.minus(new la.Vector([-8.5680, 7.1408])).norm(), 0, 1e-3);
             assert.eqtol(Math.abs(SVC.bias - 23.1314), 0, 1e-3);
         });
         it('should fit a model on high-dimensional (embedded) iris dataset (dense), class=setosa, features:sepal length, sepal width', function () {
             var X = require('./irisX.json');
             var y = require('./irisY.json');
             
             var matrix0 = new la.Matrix(X);
             var zeros = la.zeros(matrix0.rows, 1000);
             matrix = la.cat([[matrix0, zeros]]);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 10000 });
             SVC.fit(matrix, vec);
             // based on matlab
             //load fisheriris
             //X = [meas(:,1), meas(:,2)];
             //Y = nominal(ismember(species,'setosa'));
             //y = 2*(double(Y)-1.5);
             //s=fitcsvm(X,y,'Standardize',false, 'BoxConstraint',10000);
             // weights = s.SupportVectors'* (s.Alpha.*(y(s.IsSupportVector)))
             // bias = s.Bias
             assert.eqtol(SVC.weights.subVec([0, 1]).minus(new la.Vector([-8.5680, 7.1408])).norm(), 0, 1e-3);
             assert.eqtol(Math.abs(SVC.bias - 23.1314), 0, 1e-3);
         });
         it('should fit a model on high-dimensional (embedded) iris dataset (sparse), class=setosa, features:sepal length, sepal width', function () {
             var X = require('./irisX.json');
             var y = require('./irisY.json');
             
             var matrix0 = new la.Matrix(X);
             var zeros = la.zeros(matrix0.rows, 1000);
             matrix = la.cat([[matrix0, zeros]]);
             matrix = matrix.transpose();
             var spMatrix = matrix.sparse();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 10000 });
             SVC.fit(spMatrix, vec);
             // based on matlab
             //load fisheriris
             //X = [meas(:,1), meas(:,2)];
             //Y = nominal(ismember(species,'setosa'));
             //y = 2*(double(Y)-1.5);
             //s=fitcsvm(X,y,'Standardize',false, 'BoxConstraint',10000);
             // weights = s.SupportVectors'* (s.Alpha.*(y(s.IsSupportVector)))
             // bias = s.Bias
             assert.eqtol(SVC.weights.subVec([0, 1]).minus(new la.Vector([-8.5680, 7.1408])).norm(), 0, 1e-3);
             assert.eqtol(Math.abs(SVC.bias - 23.1314), 0, 1e-3);
         });
         
         it('should fit a model on high-dimensional (embedded) iris dataset (dense), class=setosa, features:sepal length, sepal width', function () {
             var X = require('./irisX.json');
             var y = require('./irisY.json');
             
             var matrix0 = new la.Matrix(X);
             var seed = 1;
             var nextSeed = (x) => (x * 16807) % 2147483647;
             var D = la.zeros(matrix0.rows, 1000);
             for (var i = 0; i < 150; i++) {
                 for (var j = 0; j < 1000; j++) {
                     D.put(i, j, 1 / 1000 * (seed - 1) / 2147483646);
                     seed = nextSeed(seed);
                 }
             }
             matrix = la.cat([[matrix0, D]]);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 10000 });
             SVC.fit(matrix, vec);
             // based on matlab
             /*
             seed = 1;
             nextSeed = @(seed) mod(seed * 16807, 2147483647)
             D = zeros(150,1000);
             tic
             for i = 1:150
                 for j = 1:1000
                     D(i,j) = 1/1000 * (seed-1) / 2147483646;
                     seed = nextSeed(seed);
                 end
             end
             toc
-
             load fisheriris
             %X = [meas(:,1), meas(:,2) zeros(size(meas,1), 1000)];
             X = [meas(:,1), meas(:,2) D];
             Y = nominal(ismember(species,'setosa'));
             y = 2*(double(Y)-1.5);
             s=fitcsvm(X,y,'Standardize',false, 'BoxConstraint',10000);
             // weights = s.SupportVectors'* (s.Alpha.*(y(s.IsSupportVector)))
             // bias = s.Bias
             */
             assert.eqtol(SVC.weights.subVec([0, 1]).minus(new la.Vector([-8.5341, 7.1144])).norm(), 0, 1e-3);
             assert.eqtol(Math.abs(SVC.bias - 23.0358), 0, 1e-3);
         });
         it('should find a soft margin', function () {
             X = [[-10, 1],
                  [-3, 0],
                  [-20, -1],
                  [20, 1],
                  [3, 0],
                  [10, -1]];
             var y = [1, -1, 1, -1, 1, -1];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 1e-3 });
             SVC.fit(matrix, vec);
             assert.eqtol(SVC.predict(matrix).minus(new la.Vector([1, 1, 1, -1, -1, -1])).norm(), 0, 1e-6);
         });
         it('should find a soft margin and be biased towards negative examples', function () {
             X = [[-10, 1],
                  [-3, 0],
                  [-20, -1],
                  [20, 1],
                  [3, 0],
                  [10, -1]];
             var y = [1, -1, 1, -1, 1, -1];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             // unbalance: positive examples are 1000 times less important
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 1e-3, j: 1e-3});
             SVC.fit(matrix, vec);
             assert.eqtol(SVC.predict(matrix).minus(new la.Vector([-1, -1, -1, -1, -1, -1])).norm(), 0, 1e-6);
         });
         it('should find a soft margin and be biased towards positive examples', function () {
             X = [[-10, 1],
                  [-3, 0],
                  [-20, -1],
                  [20, 1],
                  [3, 0],
                  [10, -1]];
             var y = [1, -1, 1, -1, 1, -1];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             // unbalance: positive examples are 1000 times more important
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", c: 1e-3, j: 1e+3 });
             SVC.fit(matrix, vec);
             assert.eqtol(SVC.predict(matrix).minus(new la.Vector([1, 1, 1, 1, 1, 1])).norm(), 0, 1e-6);
         });
     });
    describe('Kernel tests', function () {
        it('should find a fit with polynomial kernel', function () {
             X = [[-3, 5],
                  [-1, 0.5],
                  [0, -0.5],
                  [1, 0.5],
                  [3, 5],
                  [-2, 5],
                  [-0.5, 1],
                  [0, 0.5],
                  [0.5, 1],
                  [2, 5]];
             var y = [1, 1, 1, 1, 1, -1, -1, -1, -1, -1];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", kernel: "POLY", degree: 2 });
             SVC.fit(matrix, vec);          
             assert.eqtol(SVC.predict(matrix).minus(new la.Vector([1, 1, 1, 1, 1, -1, -1, -1, -1, -1])).norm(), 0, 1e-6);
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
             var y = [1, 1, 1, 1, -1, -1, -1, -1];
             
             var matrix = new la.Matrix(X);
             matrix = matrix.transpose();
             var vec = new la.Vector(y);
             var SVC = new analytics.SVC({ algorithm: "LIBSVM", kernel: "RBF" });
             SVC.fit(matrix, vec);
             assert.eqtol(SVC.predict(matrix).minus(new la.Vector([1, 1, 1, 1, -1, -1, -1, -1])).norm(), 0, 1e-6);
         });
    });
});


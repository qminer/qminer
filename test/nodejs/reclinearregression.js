/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */


var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

var tol = 1e-8;

function linregWgtsFromVector(vec, y, regFact) {
    var dim = vec.length;
    var lambdaI = (function () {
        var lambdaIVec = [];
        for (var i = 0; i < dim; i++) {
            lambdaIVec.push(regFact);
        }
        return new la.Vector(lambdaIVec).diag();
    })();
    var expectedWgts = vec.outer(vec).plus(lambdaI).solve(vec.multiply(y));
    return expectedWgts;
}

function linregWgtsFromVectorArray(vecArr, y, regFact) {
    var linreg = new analytics.RidgeReg({ gamma: regFact });
    linreg.fit(new la.Matrix(vecArr).transpose(), new la.Vector(y));
    return linreg.weights;
}

/**
 * Returns weights of a linear regression model.
 *
 * @param {la.Matrix} X - matrix with the i-th observation in row i
 * @param {la.Vector} y - dependent variable
 * @param {number} lambda - the regularization
 * @param {number} beta - the forgetting factor
 */
function linregWgtsFromMatrixForgetFact(X, y, lambda, beta) {
    var nsamples = X.rows;
    var dim = X.cols;

    var B = (function () {
        var betas = [];
        var i;
        for (i = 0; i < nsamples; i++) {
            betas.push(1);
        }
        for (i = betas.length-2; i >= 0; i--) {
            betas[i] = beta*betas[i+1];
        }
        return new la.Vector(betas).diag();
    })();
    var lambdaI = (function () {
        var l = [];
        for (var i = 0; i < dim; i++) {
            l.push(lambda);
        }
        return new la.Vector(l).diag();
    })();
    var covmat = (function () {
        var cov = X.transpose().multiply(B).multiply(X);
        cov = cov.multiply(1 / nsamples);
        cov = cov.plus(lambdaI);
        return cov;
    })();
    var wgts = covmat.solve(X.transpose().multiply(B).multiply(y).multiply(1 / nsamples));
    return wgts;
}

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
        it('should throw exception because of invalid forget factor', function () {
            assert.throws(function () {
                new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 10.0 });
            })
        })
        it('should create an object out of the parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.8 });
            var param = linreg.getParams();
            assert.equal(param.dim, 10);
            assert.equal(param.regFact, 2.0);
            assert.equal(param.forgetFact, 0.8);
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
        it('should throw exception because of invalid forget factor', function () {
            assert.throws(function () {
                new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 15.0 });
            })
        })
        it('should return the parameters for construction of the same object', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.75 });
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
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.75 });
            assert.doesNotThrow(function () {
                linreg.setParams({ dim: 1.0 });
            });
        })
        it('should change some of the parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.76 });
            linreg.setParams({ dim: 2, regFact: 1e-10 });
            var param = linreg.getParams();
            assert.equal(param.dim, 2);
            assert.eqtol(param.regFact, 1e-10);
            assert.eqtol(param.forgetFact, 0.76);
        })
        it('should throw an exception if there are no parameters', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.76 });
            assert.throws(function () {
                linreg.setParams();
            });
        })
    })

    describe('Dim Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.76 });
            assert.doesNotThrow(function () {
                var dim = linreg.dim;
            });
        })
        it('should return the dimensionality', function () {
            var linreg = new analytics.RecLinReg({ dim: 10, regFact: 2.0, forgetFact: 0.76 });
            var dim = linreg.dim;
            assert.equal(dim, 10);
        })
    });

    describe('PartialFit', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 2.0, forgetFact: 0.76 });
            var vec = new la.Vector([1, 2]);
            assert.doesNotThrow(function () {
                linreg.partialFit(vec, 3);
            });
        })
        it('should make the model by partial fitting one test vector', function () {
            var regFact = 1e-10;
            var y = 3;
            var vec = new la.Vector([1, 1]);

            var linreg = new analytics.RecLinReg({ dim: 2, regFact: regFact, forgetFact: 1.0 });
            linreg.partialFit(vec, y);

            var weights = linreg.weights;
            var expectedWgts = linregWgtsFromVector(vec, y, regFact);
            assert.eqtol(weights[0], expectedWgts[0], tol);
            assert.eqtol(weights[1], expectedWgts[1], tol);
        })
        it('should make a new model by partial fitting two test vectors', function () {
            var vec1 = [1, 1];
            var vec2 = [2, -1];
            var y1 = 3;
            var y2 = 3;
            var regFact = 1e-10;

            var linreg = new analytics.RecLinReg({ dim: 2, regFact: regFact, forgetFact: 1.0 });

            linreg.partialFit(new la.Vector(vec1), y1);
            linreg.partialFit(new la.Vector(vec2), y2);

            var weights = linreg.weights;
            var expectedWgts = linregWgtsFromVectorArray([vec1, vec2], [y1, y2], regFact);
            assert.eqtol(weights[0], expectedWgts[0], tol);
            assert.eqtol(weights[1], expectedWgts[1], tol);
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
        it('should not become ill conditioned', function () {
            var linreg = new analytics.RecLinReg({ dim: 3, regFact: 1e-5, forgetFact: 0.9 });
            var ftrVec = new la.Vector([0, 0, 0]);

            var nsamples = 100000;
            for (var i = 0; i < nsamples; i++) {
                ftrVec[2] = Math.random();
                linreg.partialFit(ftrVec, Math.random());
            }
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 2.0, forgetFact: 0.76 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);

            assert.doesNotThrow(function () {
                linreg.fit(X, y);
            });
        })
        it('should make a new model by fitting the matrix', function () {
            var regFact = 1e-10;
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);

            var linreg = new analytics.RecLinReg({ dim: 2, regFact: regFact, forgetFact: 1.0 });
            var batchreg = new analytics.RidgeReg({ gamma: regFact });

            linreg.fit(X, y);
            batchreg.fit(X, y);

            var weights = linreg.weights;
            var batchWgts = batchreg.weights;

            assert.eqtol(weights[0], batchWgts[0], tol);
            assert.eqtol(weights[1], batchWgts[1], tol);
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
            assert.eqtol(model.weights[0], 2, tol);
            assert.eqtol(model.weights[1], 1, tol);
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
    });
    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var linreg = new analytics.RecLinReg({ dim: 2, regFact: 1e-10, forgetFact: 1.0 });
            var X = new la.Matrix([[1, 2], [1, -1]]);
            var y = new la.Vector([3, 3]);
            linreg.fit(X, y);
            linreg.save(require('qminer').fs.openWrite('reclinreg_test.bin')).close();
            var linreg2 = new analytics.RecLinReg(require('qminer').fs.openRead('reclinreg_test.bin'));
            assert.deepEqual(linreg.getParams(), linreg2.getParams());
            assert.eqtol(linreg.weights.minus(linreg2.weights).norm(), 0, 1e-8);
        })
    });
    describe('testing forget factor', function () {
        it('should return correct weights', function () {
            var regFact = 0.1;
            var forgetFact = 0.90;
            var n = 1000;
            var dim = 3;

            var linreg = new analytics.RecLinReg({ dim: dim, regFact: regFact, forgetFact: forgetFact });

            var X = new la.Matrix({ rows: n, cols: dim });
            var yArr = [];

            for (var rowN = 0; rowN < n; rowN++) {
                var rowArr = [1];
                for (var colN = 1; colN < dim; colN++) {
                    rowArr.push(Math.random());
                }

                var xi = new la.Vector(rowArr);
                var yi = Math.random();

                linreg.partialFit(xi, yi);

                X.setRow(rowN, xi);
                yArr.push(yi);
            }

            var y = new la.Vector(yArr);

            var wgts = linreg.weights;
            var expectedWgts = linregWgtsFromMatrixForgetFact(X, y, regFact, forgetFact);

            assert.eqtol(wgts[0], expectedWgts[0], tol);
            assert.eqtol(wgts[1], expectedWgts[1], tol);
            assert.eqtol(wgts[2], expectedWgts[2], tol);
        })
        it('should not become ill-formed', function () {
            var regFact = 0.1;
            var forgetFact = 0.5;
            var n = 10000;
            var dim = 3;

            var ones = [];
            for (var j = 0; j < dim; j++) {
                ones.push(1);
            }

            var linreg = new analytics.RecLinReg({ dim: dim, regFact: regFact, forgetFact: forgetFact });

            assert.doesNotThrow(function () {
                for (var i = 0; i < n; i++) {
                    linreg.partialFit(new la.Vector(ones), Math.random());
                }
            });
        })
    })
})

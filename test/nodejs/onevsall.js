/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

///////////////////////////////////////////////////////////
// One vs All Unit Tests

var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('../../index.js').analytics;
var la = require('../../index.js').la;

describe('OneVsAll Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            var json = { c: 10, maxTime: 12000 };
            assert.doesNotThrow(function () {
                var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            });
        })
        it('should throw an exception if there are missing crucial parameters', function () {
            assert.throws(function () {
                var onevsall = new analytics.OneVsAll();
            });
        })
        it('should construct an OneVsAll object', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            var params = onevsall.getParams();
            assert.strictEqual(params.cats, 10);
            assert.strictEqual(params.modelParam.c, 10);
            assert.strictEqual(params.modelParam.maxTime, 12000);
            assert.strictEqual(params.models.length, 0);
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            assert.doesNotThrow(function () {
                var params = onevsall.getParams();
            });
        })
        it('should return the parameters of the object', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            var params = onevsall.getParams();
            assert.strictEqual(params.cats, 10);
            assert.strictEqual(params.modelParam.c, 10);
            assert.strictEqual(params.modelParam.maxTime, 12000);
            assert.strictEqual(params.models.length, 0);
            assert.strictEqual((new params.model(params.modelParam)).getParams().c, 10);
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            assert.doesNotThrow(function () {
                onevsall.setParams({ model: analytics.SVR, modelParam: json });
            });
        })
        it('should set the parameters and override the old ones', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            var params = onevsall.getParams();
            assert.strictEqual(params.cats, 10);
            assert.strictEqual(params.modelParam.c, 10);
            assert.strictEqual(params.modelParam.maxTime, 12000);
            assert.strictEqual(params.models.length, 0);
            assert.strictEqual((new params.model(params.modelParam)).getParams().c, 10);

            onevsall.setParams({ model: analytics.SVR, cats: 5 });
            var params = onevsall.getParams();
            assert.strictEqual(params.cats, 5);
            assert.strictEqual(params.modelParam.c, 10);
            assert.strictEqual(params.modelParam.maxTime, 12000);
            assert.strictEqual(params.models.length, 0);
            assert.strictEqual((new params.model(params.modelParam)).getParams().c, 10);
        })
        it('should throw an exception if no parameters are given', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 10 });
            assert.throws(function () {
                onevsall.setParams();
            });
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            assert.doesNotThrow(function () {
                onevsall.fit(matrix, vector);
            });
        })
        it('should set the models, dense matrix', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var param = onevsall.getParams();
            assert.strictEqual(param.models.length, 2);
        })
        it('should set the models, sparse matrix', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, 2], [1, 1]], [[0, 1], [1, -3]], [[0, 1], [1, -4]]]);
            var vector = new la.Vector([0, 0, 1, 1]);

            onevsall.fit(matrix, vector);

            var param = onevsall.getParams();
            assert.strictEqual(param.models.length, 2);
        })
        it('should throw an exception if there are no parameters given', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            assert.throws(function () {
                onevsall.fit();
            });
        })
        it('should throw an exception if only the matrix is given', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);

            assert.throws(function () {
                onevsall.fit(matrix);
            });
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception, dense vector', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1, 2]);
            assert.doesNotThrow(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should return the index of the cluster, where it the most fit, dense vector', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1, 2]);
            var prediction = onevsall.predict(test);

            assert.strictEqual(prediction, 0);
        })
        it('should throw an exception if the dense vector is too long', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1, 2, 3]);
            assert.throws(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should throw an exception if the dense vector is too short', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1]);
            assert.throws(function () {
                var prediction = onevsall.predict(test);
            });
        })

        it('should not throw an exception, sparse vector', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.SparseVector([[0, 1], [1, 2]]);
            assert.doesNotThrow(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should return the index of the cluster, where it the most fit, sparse vector', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.SparseVector([[0, 1], [1, 2]]);
            var prediction = onevsall.predict(test);

            assert.strictEqual(prediction, 0);
        })

        it('should not throw an exception, dense matrix', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1], [2, -3]]);
            assert.doesNotThrow(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should return an integer vector containing the cluster indeces, dense matrix', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1], [2, -3]]);
            var prediction = onevsall.predict(test);

            assert.strictEqual(prediction.at(0), 0);
            assert.strictEqual(prediction.at(1), 1);
        })
        it('should throw an exception if there are more rows than in the model, dense matrix', function () {
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1], [2, -3], [4, 4]]);
            assert.throws(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should throw an exception if there are less rows than in the model, dense matrix', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                var prediction = onevsall.predict(test);
            });
        })

        it('should not throw an exception, sparse matrix', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, 1], [1, -3]]]);
            assert.doesNotThrow(function () {
                var prediction = onevsall.predict(test);
            });
        })
        it('should return an integer vector containing the cluster indeces, sparse matrix', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, 1], [1, -3]]]);
            var prediction = onevsall.predict(test);

            assert.strictEqual(prediction.at(0), 0);
            assert.strictEqual(prediction.at(1), 1);
        })
    });

    describe('DecisionFunction Tests', function () {
        it('should not throw an exception, dense vector', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1, 2]);
            assert.doesNotThrow(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })
        it('should throw an exception if the vector is too long, dense vector', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1, 2, 3]);
            assert.throws(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })
        it('should throw an exception if the vector is too short, dense vector', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Vector([1]);
            assert.throws(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })

        it('should not throw an exception, sparse vector', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.SparseVector([[0, 1], [1, 2]]);
            assert.doesNotThrow(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })

        it('should not throw an exception, dense matrix', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1], [2, -3]]);
            assert.doesNotThrow(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })
        it('should throw an exception if the number of rows are greater than the models', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1], [2, -3], [2, 3]]);
            assert.throws(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })
        it('should throw an exception if the number of rows are lesser than the models', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })

        it('should not throw exception, sparse matrix', function () {

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
            var vector = new la.Vector([0, 0, 1, 1]);
            onevsall.fit(matrix, vector);

            var test = new la.Matrix([[[0, 1], [1, 2]], [[0, 1], [1, -3]]]);
            assert.doesNotThrow(function () {
                var decision = onevsall.decisionFunction(test);
            });
        })
    })
})


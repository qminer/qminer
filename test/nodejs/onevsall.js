///////////////////////////////////////////////////////////
// One vs All Unit Tests

var assert = require('../../src/nodejs/scripts/assert.js');
var analytics = require('qminer').analytics;
var la = require('qminer').la;

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
            assert.equal(params.cats, 10);
            assert.equal(params.modelParam.c, 10);
            assert.equal(params.modelParam.maxTime, 12000);
            assert.equal(params.models.length, 0);
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
            assert.equal(params.cats, 10);
            assert.equal(params.modelParam.c, 10);
            assert.equal(params.modelParam.maxTime, 12000);
            assert.equal(params.models.length, 0);
            assert.equal((new params.model(params.modelParam)).getParams().c, 10);
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
            assert.equal(params.cats, 10);
            assert.equal(params.modelParam.c, 10);
            assert.equal(params.modelParam.maxTime, 12000);
            assert.equal(params.models.length, 0);
            assert.equal((new params.model(params.modelParam)).getParams().c, 10);

            onevsall.setParams({ model: analytics.SVR, cats: 5 });
            var params = onevsall.getParams();
            assert.equal(params.cats, 5);
            assert.equal(params.modelParam.c, 10);
            assert.equal(params.modelParam.maxTime, 12000);
            assert.equal(params.models.length, 0);
            assert.equal((new params.model(params.modelParam)).getParams().c, 10);
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
            this.timeout(10000);

            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            assert.doesNotThrow(function () {
                onevsall.fit(matrix, vector);
            });
        })
        it('should set the models', function () {
            this.timeout(10000);
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);
            var vector = new la.Vector([0, 0, 1, 1, 1, 1]);
            onevsall.fit(matrix, vector);

            var param = onevsall.getParams();
            assert.equal(param.models.length, 2);
        })
        it('should throw an exception if there are no parameters given', function () {
            this.timeout(10000);
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });

            assert.throws(function () {
                onevsall.fit();
            });
        })
        it('should throw an exception if only the matrix is given', function () {
            this.timeout(10000);
            var json = { c: 10, maxTime: 12000 };
            var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: json, cats: 2 });
            var matrix = new la.Matrix([[1, 2, 1, 1, -3, -4], [2, 1, -3, -4, 2, 2]]);

            assert.throws(function () {
                onevsall.fit(matrix);
            });
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {

        })
    })
})


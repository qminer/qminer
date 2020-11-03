var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');
var analytics = qm.analytics;
var la = qm.la;


describe('Logistic Regression Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception, default constructor', function () {
            assert.doesNotThrow(function () {
                var logreg = new analytics.LogReg();
            });
        })
        it('should create a default constructor', function () {
            var logreg = new analytics.LogReg();
            var param = logreg.getParams();
            assert.equal(param.lambda, 1.0);
            assert.equal(param.intercept, false);
        })
        it('should not throw an exception, given parameters', function () {
            assert.doesNotThrow(function () {
                var logreg = new analytics.LogReg({ lambda: 10, intercept: true });
            });
        })
        it('should create a constructor out of the given parameters', function () {
            var logreg = new analytics.LogReg({ lambda: 10, intercept: true });
            var param = logreg.getParams();
            assert.equal(param.lambda, 10.0);
            assert.equal(param.intercept, true);
        })
        it('should throw an exception if the parameter is not a json object', function () {
            assert.throws(function () {
                var logreg = new analytics.LogReg(10, false);
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception, default constructor', function () {
            var logreg = new analytics.LogReg();
            assert.doesNotThrow(function () {
                var param = logreg.getParams();
            });
        })
        it('should give the parameters of the default constructor', function () {
            var logreg = new analytics.LogReg();
            var param = logreg.getParams();
            assert.equal(param.lambda, 1.0);
            assert.equal(param.intercept, false);
        })
        it('should not throw an exception, given parameters', function () {
            var logreg = new analytics.LogReg({ lambda: 10, intercept: true });
            assert.doesNotThrow(function () {
                var param = logreg.getParams();
            });
        })
        it('should returns the parameters of the constructor created with the given parameters', function () {
            var logreg = new analytics.LogReg({ lambda: 10, intercept: true });
            var param = logreg.getParams();
            assert.equal(param.lambda, 10);
            assert.equal(param.intercept, true);
        })
        it('should return parameters that can be used for the constructor', function () {
            var logreg = new analytics.LogReg({ lambda: 10, intercept: true });
            var param = logreg.getParams();
            var logreg2 = new analytics.LogReg(param);
            var param2 = logreg2.getParams();

            assert.equal(param2.lambda, param.lambda);
            assert.equal(param2.intercept, param.intercept);
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var logreg = new analytics.LogReg();
            assert.doesNotThrow(function () {
                logreg.setParams({ lambda: 10, intercept: true });
            });
        })
        it('should set the parameters of the model', function () {
            var logreg = new analytics.LogReg();
            logreg.setParams({ lambda: 10, intercept: true });
            var param = logreg.getParams();
            assert.equal(param.lambda, 10);
            assert.equal(param.intercept, true);
        })
    });

    // need openblas configurations for these tests
    if (qm.flags.blas) {

        describe('Fit Tests', function () {
            it('should not throw an exception', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 2], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                assert.doesNotThrow(function () {
                    logreg.fit(mat, vec);
                });
            })
            it('should fit the model with the values', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                var weights = logreg.weights;
                assert(weights[0] > 0);
                assert(weights[1] == 0);
            })
            it('should throw an exception if matrix has more columns than length of vector', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1, 1], [1, -1, -2]]);
                var vec = new la.Vector([3, 3]);
                assert.throws(function () {
                    logreg.fit(mat, vec);
                });
            })
            it('should throw an exception if matrix has fewer columns than length of vector', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1], [1]]);
                var vec = new la.Vector([3, 3]);
                assert.throws(function () {
                    logreg.fit(mat, vec);
                });
            })
        });

        describe('Predict Tests', function () {
            it('should not throw an exception', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                var test = new la.Vector([1, 2]);
                assert.doesNotThrow(function () {
                    var prediction = logreg.predict(test);
                });
            })
            it('should return the prediction', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                var test = new la.Vector([1, 3]);
                var prediction = logreg.predict(test);
                assert.eqtol(prediction, 1);
            })
            it('should throw an exception if the given vector is longer than model', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                var test = new la.Vector([1, 3, 2]);
                assert.throws(function () {
                    var prediction = logreg.predict(test);
                });
            })
            it('should throw an excpetion if the given vector is shorter than model', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                var test = new la.Vector([1]);
                assert.throws(function () {
                    var prediction = logreg.predict(test);
                });
            })
        })

        describe('Serialization Tests', function () {
            it('should serialize and deserialize', function () {
                var logreg = new analytics.LogReg();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                logreg.fit(mat, vec);
                logreg.save(require('../../index.js').fs.openWrite('logreg_test.bin')).close();
                var logreg2 = new analytics.LogReg(require('../../index.js').fs.openRead('logreg_test.bin'));
                assert.deepEqual(logreg.getParams(), logreg2.getParams());
                assert.eqtol(logreg.weights.minus(logreg2.weights).norm(), 0, 1e-8);
            })
        });
    }
});
var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');
var analytics = qm.analytics;
var la = qm.la;

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
                var hazard = new analytics.PropHazards({ lambda: 2 });
            });
        })

        it('should create a model with custom parameters', function () {
            var hazard = new analytics.PropHazards({ lambda: 2 });
            var params = hazard.getParams();
            assert.equal(params.lambda, 2);
        })
        it('should throw an exception if any other object is given', function () {
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
            var hazard = new analytics.PropHazards({ lambda: 10 });
            assert.doesNotThrow(function () {
                var params = hazard.getParams();
            });
        })
        it('should return the parameters of the model, custom parameters', function () {
            var hazard = new analytics.PropHazards({ lambda: 10 });
            var params = hazard.getParams();
            assert.equal(params.lambda, 10);
        })
        it('should get the same object for construction of a copy', function () {
            var hazard1 = new analytics.PropHazards({ lambda: 5 });
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
                hazard.setParams({ lambda: 5 });
            });
        })
        it('should set the parameters of the model', function () {
            var hazard = new analytics.PropHazards();
            hazard.setParams({ lambda: 5 });
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
    });

    // checks if openblas is used
    if (qm.flags.blas) {

        describe('Fit Tests', function () {
            it('should not throw an exception', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                assert.doesNotThrow(function () {
                    hazard.fit(mat, vec);
                });
            })
            it('should fit the model', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                var weights = hazard.weights;
                assert(weights.length > 0);
            })
            it('should throw an exception if the vector is longer than the number of columns', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1, 1], [1, -1, -2]]);
                var vec = new la.Vector([3, 3]);
                assert.throws(function () {
                    hazard.fit(mat, vec);
                });
            })
            it('should throw an exception if the vector is shorter than the number of columns', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1], [1]]);
                var vec = new la.Vector([3, 3]);
                assert.throws(function () {
                    hazard.fit(mat, vec);
                });
            })
        });

        describe('Predict Tests', function () {
            it('should not throw an exception', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                var test = new la.Vector([1, 2]);
                assert.doesNotThrow(function () {
                    var prediction = hazard.predict(test);
                });
            })
            it('should return the prediction of the model', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                var test = new la.Vector([1, 2]);
                var prediction = hazard.predict(test);
                assert(prediction > 0);
            })
            it('should throw an exception if the length of vector is greater than of models', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                var test = new la.Vector([1, 2, 3]);
                assert.throws(function () {
                    var prediction = hazard.predict(test);
                });
            })
            it('should throw an exception if the length of vector is lesser than of models', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                var test = new la.Vector([1]);
                assert.throws(function () {
                    var prediction = hazard.predict(test);
                });
            })
        });

        describe('Serialization Tests', function () {
            it('should serialize and deserialize', function () {
                var hazard = new analytics.PropHazards();
                var mat = new la.Matrix([[1, 1], [1, -1]]);
                var vec = new la.Vector([3, 3]);
                hazard.fit(mat, vec);
                hazard.save(require('../../index.js').fs.openWrite('hazard_test.bin')).close();
                var hazard2 = new analytics.PropHazards(require('../../index.js').fs.openRead('hazard_test.bin'));
                assert.deepEqual(hazard.getParams(), hazard.getParams());
                assert.eqtol(hazard.weights.minus(hazard2.weights).norm(), 0, 1e-8);
            })
        });
    }

})

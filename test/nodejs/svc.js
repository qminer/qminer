var qm = require('qminer');
var la = qm.la;
var analytics = qm.analytics;
var assert = require('../../src/nodejs/scripts/assert.js');

describe('SVC Tests', function () {
    beforeEach(function () {

    });
    afterEach(function () {

    });

    describe('Fit Tests', function () {
        it('should not throw an exception when given the matrix and vector', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC();

            assert.doesNotThrow(function () {
                SVC.fit(matrix, vec);
            });
        })
        it('should create a model out of the matrix and vector', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC();

            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 1, 1e-3);
            assert.eqtol(model.weights[1], 1, 1e-3);
        })
        it('should throw an exception if the number of matrix columns and vector length are not equal', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1]);
            var SVC = new analytics.SVC();
            assert.throws(function () {
                SVC.fit(matrix, vec);
            });
        })
        it('should return a close-zero model', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([-1, -1]);
            var SVC = new analytics.SVC();

            SVC.fit(matrix, vec);
            var model = SVC.getModel();
            assert.eqtol(model.weights[0], 0, 1e-2);
            assert.eqtol(model.weights[1], 0, 1e-2);
        })
        it('should forget the previous model', function () {
            this.timeout(4000);
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC();
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
            var SVC = new analytics.SVC();

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
            var SVC = new analytics.SVC();
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
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.doesNotThrow(function () {
                SVC.predict(vec2);
            });
        })
        it('should return 1 for the given vector', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.equal(SVC.predict(vec2), 1);
        })
        it('should throw an exception if the vector is longer', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0, 1]);
            assert.throws(function () {
                SVC.predict(vec2);
            })
        })
        it('should return the vector [1, 1, -1] for the given matrix', function () {
            var matrix = new la.Matrix([[0, 1, -1, 0], [1, 0, 0, -1]]);
            var vec = new la.Vector([1, 1, -1, -1]);
            var SVC = new analytics.SVC();
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
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, 3, -1], [0, 3, -2], [1, 1, 2]]);
            assert.throws(function () {
                SVC.predict(matrix2);
            })
        })
    });
    describe('DecisionFunction Tests', function () {
        it('should not throw an exception', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([3, 0]);
            assert.doesNotThrow(function () {
                SVC.decisionFunction(vec2);
            });
        })
        it('should return the distance of the vector from the hyperplane', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var vec2 = new la.Vector([1, 1]);
            var distance = SVC.decisionFunction(vec2);

            assert.eqtol(distance, 1, 1e-3);
        })
        it('should throw an exception if the vector is to long or short', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);
            var vec2 = new la.Vector([1, 1, -1]);

            assert.throws(function () {
                SVC.decisionFunction(vec2);
            });
        })
        it('should return a vector of distances if the given parameter is a matrix', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, -1, 0], [2, 1, -3]]);
            var distance = SVC.decisionFunction(matrix2);

            assert.eqtol(distance[0], 1, 1e-3);
            assert.eqtol(distance[1], -1, 1e-3);
            assert.eqtol(distance[2], 0, 1e-2);
        })
        it('should throw an exception of the matrix has to many rows', function () {
            var matrix = new la.Matrix([[1, -1], [0, 0]]);
            var vec = new la.Vector([1, -1]);
            var SVC = new analytics.SVC();
            SVC.fit(matrix, vec);

            var matrix2 = new la.Matrix([[1, -1, 0], [2, 1, -3], [1, -2, 0]]);
            assert.throws(function () {
                SVC.decisionFunction(matrix2);
            });
        })
    })
})

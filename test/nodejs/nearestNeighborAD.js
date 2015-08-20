
var assert = require("../../src/nodejs/scripts/assert.js");

var analytics = require('qminer').analytics;
var la = require('qminer').la;


describe('NearestNeighborAD Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception', function () {
            assert.doesNotThrow(function () {
                var neighbour = new analytics.NearestNeighborAD();
            });
        })
        it('should construct a default NearestNeighborAD object', function () {
            var neighbour = new analytics.NearestNeighborAD();
            var params = neighbour.getParams();
            assert.equal(params.rate, 0.05);
        })
        it('should construct a NearestNeighborAD object out of the params', function () {
            var neighbour = new analytics.NearestNeighborAD({ rate: 0.5 });
            var params = neighbour.getParams();
            assert.equal(params.rate, 0.5);
        })
        it('should throw an exception if the parameter.rate is greater than 1', function () {
            assert.throws(function () {
                var neighbour = new analytics.NearestNeighborAD({ rate: 1.01 });
            });
        })
        it('should throw an exception if the parameter.rate is lesser than 0', function () {
            assert.throws(function () {
                var neighbour = new analytics.NearestNeighborAD({ rate: -0.01 });
            });
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD();
            assert.doesNotThrow(function () {
                neighbor.setParams({ rate: 0.1 });
            });
        })
        it('should set the parameters of the default object', function () {
            var neighbor = new analytics.NearestNeighborAD();
            neighbor.setParams({ rate: 0.1 });
            var params = neighbor.getParams();
            assert.equal(params.rate, 0.1);
        })
        it('should set the parameters of the object', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.25 });
            neighbor.setParams({ rate: 0.1 });
            var params = neighbor.getParams();
            assert.equal(params.rate, 0.1);
        })
        it('should throw an exception if the given rate is bigger than 1', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.25 });
            assert.throws(function () {
                neighbor.setParams({ rate: 1.01 });
            });
        })
        it('should throw an excpetion if the given rate is lesser than 0', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.25 });
            assert.throws(function () {
                neighbor.setParams({ rate: -0.001 });
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD();
            assert.doesNotThrow(function () {
                neighbor.getParams();
            });
        })
        it('should get the parameters of the default object', function () {
            var neighbor = new analytics.NearestNeighborAD();
            var params = neighbor.getParams();
            assert.equal(params.rate, 0.05);
        })
        it('should get the parameters of the object', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
            var params = neighbor.getParams();
            assert.equal(params.rate, 0.1);
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD();
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            assert.doesNotThrow(function () {
                neighbor.fit(matrix);
            });
        })
        it('should create a model with tresh equal to 8', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);
            var model = neighbor.getModel();

            assert.eqtol(model.threshold, 8);
        })
        it('should create a model with tresh equal to 2', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.34, windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);
            var model = neighbor.getModel();

            assert.eqtol(model.threshold, 2);
        })
    });

    describe('GetModel Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD();
            assert.doesNotThrow(function () {
                var model = neighbor.getModel();
            });
        })
        it('should return the default parameters of the model', function () {
            var neighbor = new analytics.NearestNeighborAD();
            var model = neighbor.getModel();
            assert.equal(model.threshold, 0);
        })
        it('should return the parameters of the model', function () {
            var neighbor = new analytics.NearestNeighborAD({ rate: 0.1 });
            var model = neighbor.getModel();
            assert.equal(model.threshold, 0);
        })
        it('should return the parameters of the model, after fit', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);
            var model = neighbor.getModel();

            assert.eqtol(model.threshold, 8);
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);

            var vector = new la.SparseVector([[0, 1], [1, 2]]);
            assert.doesNotThrow(function () {
                neighbor.predict(vector);
            });
        })
        it('should return 0 for the given vector', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);

            var vector = new la.SparseVector([[0, 1], [1, 2]]);
            var prediction = neighbor.predict(vector);
            assert.equal(prediction, 0);
        })
        it('should return 1 for the given vector', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 3 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]], [[0, 0], [1, 1]]]);
            neighbor.fit(matrix);

            var vector = new la.SparseVector([[0, 4], [1, 0]]);
            var prediction = neighbor.predict(vector);
            assert.equal(prediction, 1);
        })
    });

    describe('PartialFit Tests', function () {
        it('should not throw an exception', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 2 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]]]);
            neighbor.fit(matrix);

            var vector = new la.SparseVector([[0, 0], [1, 1]]);
            assert.doesNotThrow(function () {
                neighbor.partialFit(vector);
            });
        })
        it('should update the model and return the threshold equal to 8, vector input', function () {
            var neighbor = new analytics.NearestNeighborAD({ windowSize: 2 });
            var matrix = new la.SparseMatrix([[[0, 1], [1, 2]], [[0, -2], [1, 3]]]);
            neighbor.fit(matrix);

            var vector = new la.SparseVector([[0, 0], [1, 1]]);
            neighbor.partialFit(vector);
            var model = neighbor.getModel();

            assert.eqtol(model.threshold, 8);
        })
    });
});

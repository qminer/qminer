var assert = require('../../src/nodejs/scripts/assert.js');
var qm = require('../../index.js');
var analytics = qm.analytics;
var la = qm.la;

describe('NNet Tests', function () {

    describe('Constructor Tests', function () {
        it('should not throw an exception, default parameters', function () {
            assert.doesNotThrow(function () {
                var nnet = new analytics.NNet();
            });
        })
        it('should create a default constructor', function () {
            var nnet = new analytics.NNet();
            var params = nnet.getParams();
            assert.strictEqual(params.layout[0], 1);
            assert.strictEqual(params.layout[1], 2);
            assert.strictEqual(params.layout[2], 1);
            assert.strictEqual(params.learnRate, 0.1);
            assert.strictEqual(params.momentum, 0.5);
            assert.strictEqual(params.tFuncHidden, "tanHyper");
            assert.strictEqual(params.tFuncOut, "tanHyper");
        })
        it('should not throw an exception, custom parameters', function () {
            assert.doesNotThrow(function () {
                var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10 });
            });
        })
        it('should create a custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.strictEqual(params.layout[0], 3);
            assert.strictEqual(params.layout[1], 2);
            assert.strictEqual(params.layout[2], 1);
            assert.strictEqual(params.learnRate, 1);
            assert.strictEqual(params.momentum, 10);
            assert.strictEqual(params.tFuncHidden, "sigmoid");
            assert.strictEqual(params.tFuncOut, "tanHyper");
        })
        it('should throw an exception if no json is given as the parameter', function () {
            assert.throws(function () {
                var nnet = new analytics.NNet(0);
            });
        })
    });

    describe('GetParams Tests', function () {
        it('should not throw an exception, default constructor', function () {
            var nnet = new analytics.NNet();
            assert.doesNotThrow(function () {
                var params = nnet.getParams();
            });
        })
        it('should give the parameters, default constructor', function () {
            var nnet = new analytics.NNet();
            var params = nnet.getParams();
            assert.strictEqual(params.layout[0], 1);
            assert.strictEqual(params.layout[1], 2);
            assert.strictEqual(params.layout[2], 1);
            assert.strictEqual(params.learnRate, 0.1);
            assert.strictEqual(params.momentum, 0.5);
            assert.strictEqual(params.tFuncHidden, "tanHyper");
            assert.strictEqual(params.tFuncOut, "tanHyper");
        })
        it('should not throw an exception, custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            assert.doesNotThrow(function () {
                var params = nnet.getParams();
            });
        })
        it('should give the parameters, custom constructor', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.strictEqual(params.layout[0], 3);
            assert.strictEqual(params.layout[1], 2);
            assert.strictEqual(params.layout[2], 1);
            assert.strictEqual(params.learnRate, 1);
            assert.strictEqual(params.momentum, 10);
            assert.strictEqual(params.tFuncHidden, "sigmoid");
            assert.strictEqual(params.tFuncOut, "tanHyper");
        })
    });

    describe('SetParams Tests', function () {
        it('should not throw an exception', function () {
            var nnet = new analytics.NNet();
            assert.doesNotThrow(function () {
                nnet.setParams({ layout: [3, 2, 1], momentum: 10 });
            });
        })
        it('should change the parameters of the model', function () {
            var nnet = new analytics.NNet();
            nnet.setParams({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var params = nnet.getParams();
            assert.strictEqual(params.layout[0], 3);
            assert.strictEqual(params.layout[1], 2);
            assert.strictEqual(params.layout[2], 1);
            assert.strictEqual(params.learnRate, 1);
            assert.strictEqual(params.momentum, 10);
            assert.strictEqual(params.tFuncHidden, "sigmoid");
            assert.strictEqual(params.tFuncOut, "tanHyper");
        })
        it('should throw an exception if no parameter is given', function () {
            var nnet = new analytics.NNet();
            assert.throws(function () {
                nnet.setParams();
            })
        })
    });

    describe('Fit Tests', function () {
        it('should not throw an exception, default constructor, vectors', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([10]);
            assert.doesNotThrow(function () {
                nnet.fit(vecIn, vecOut);
            });
        })
        it('should throw an exception if the vector lengths are lesser than the layout, first layer', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector();
            var vecOut = new la.Vector([10]);
            assert.throws(function () {
                nnet.fit(vecIn, vecOut);
            });
        })
        it('should throw an exception if the vector lengths are lesser than the layout, last layer', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector();
            assert.throws(function () {
                nnet.fit(vecIn, vecOut);
            });
        })
        it('should throw an exception if the vector lengths are greater than the layout, first layer', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10, 1]);
            var vecOut = new la.Vector([10]);
            assert.throws(function () {
                nnet.fit(vecIn, vecOut);
            });
        })
        it('should throw an exception if the vector lengths are greater than the layout, last layer', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([0, 1]);
            assert.throws(function () {
                nnet.fit(vecIn, vecOut);
            });
        })
        it('should not throw an exception, custom constructor, vectors', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var vecIn = new la.Vector([10, 4, 10]);
            var vecOut = new la.Vector([10]);
            assert.doesNotThrow(function () {
                nnet.fit(vecIn, vecOut);
            });
        })

        it('should not throw an exception, default constructor, matrix', function () {
            var nnet = new analytics.NNet();
            var matIn = new la.Matrix([[1, 2, 3]]);
            var matOut = new la.Matrix([[1, 2, 3]]);
            assert.doesNotThrow(function () {
                nnet.fit(matIn, matOut);
            });
        })
        it('should not throw an exception, custom constructor, matrix', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var matIn = new la.Matrix([[1, 2, 3, 4], [-1, -2, 3, 2], [4, 4, 2, 3]]);
            var matOut = new la.Matrix([[1, 1, 2, -2]]);
            assert.doesNotThrow(function () {
                nnet.fit(matIn, matOut);
            });
        })
        it('should throw an exception if the length of columns are greater than of layers, first layer', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var matIn = new la.Matrix([[1, 2, 3, 4], [-1, -2, 3, 2], [4, 4, 2, 3], [1, 1, 1, 1]]);
            var matOut = new la.Matrix([[1, 1, 2, -2]]);
            assert.throws(function () {
                nnet.fit(matIn, matOut);
            });
        })
        it('should throw an exception if the length of columns are greater than of layers, last layer', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var matIn = new la.Matrix([[1, 2, 3, 4], [-1, -2, 3, 2], [4, 4, 2, 3]]);
            var matOut = new la.Matrix([[1, 1, 2, -2], [1, 1, 1, 1]]);
            assert.throws(function () {
                nnet.fit(matIn, matOut);
            });
        })
        it('should throw an exception if the length of columns are lesser than of layers, first layer', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var matIn = new la.Matrix([[1, 2, 3, 4], [-1, -2, 3, 2]]);
            var matOut = new la.Matrix([[1, 1, 2, -2]]);
            assert.throws(function () {
                nnet.fit(matIn, matOut);
            });
        })
        it('should throw an exception if the length of columns are lesser than of layers, last layer', function () {
            var nnet = new analytics.NNet({ layout: [3, 2, 1], learnRate: 1, momentum: 10, tFuncHidden: "sigmoid" });
            var matIn = new la.Matrix([[1, 2, 3, 4], [-1, -2, 3, 2], [4, 4, 2, 3]]);
            var matOut = new la.Matrix([]);
            assert.throws(function () {
                nnet.fit(matIn, matOut);
            });
        })
    });

    describe('Predict Tests', function () {
        it('should not throw an exception, vector', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([10]);
            nnet.fit(vecIn, vecOut);
            var test = new la.Vector([2]);
            assert.doesNotThrow(function () {
                var prediction = nnet.predict(test);
            });
        })
        it('should return the prediction, vector', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([10]);
            nnet.fit(vecIn, vecOut);
            var test = new la.Vector([2]);
            var prediction = nnet.predict(test);
            assert(prediction > 0);
        })
        it('should throw an exception if the given vector length is greater than of the models', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([10]);
            nnet.fit(vecIn, vecOut);
            var test = new la.Vector([2, 2]);
            assert.throws(function () {
                var prediction = nnet.predict(test);
            });
        })
        it('should throw an exception if the given vector length is lesser than of the models', function () {
            var nnet = new analytics.NNet();
            var vecIn = new la.Vector([10]);
            var vecOut = new la.Vector([10]);
            nnet.fit(vecIn, vecOut);
            var test = new la.Vector();
            assert.throws(function () {
                var prediction = nnet.predict(test);
            });
        })
    })
})

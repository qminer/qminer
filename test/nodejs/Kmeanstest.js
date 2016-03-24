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
//Unit test for Kmeans

describe("Kmeans test", function () {

    describe("Constructor test", function () {
        it("should return empty parameter values", function () {
            var KMeans = new analytics.KMeans();
            var params = KMeans.getParams();
            assert.equal(Object.keys(params).length, 5);
        });
        it("should return parameter values", function () {
            var KMeans = new analytics.KMeans({ iter: 100, k: 2, verbose: false });
            var params = KMeans.getParams();
            assert.equal(params.iter, 100);
            assert.equal(params.k, 2);
            assert.equal(params.distanceType, "Euclid");
            assert.equal(params.centroidType, "Dense");
            assert.equal(params.verbose, false);
        });
        it("should return the custom KMeans", function () {
            var KMeans = new analytics.KMeans({ iter: 20000, k: 100, verbose: true, distanceType: "Cos", centroidType: "Sparse" });
            var params = KMeans.getParams();
            assert.equal(params.iter, 20000);
            assert.equal(params.k, 100);
            assert.equal(params.distanceType, "Cos");
            assert.equal(params.centroidType, "Sparse");
            assert.equal(params.verbose, true);
        })
    });
    describe("Testing getParams and setParams", function () {
        it("should return the changed values of parameters", function () {
            var KMeans = new analytics.KMeans();
            KMeans.setParams({ iter: 15, k: 30, verbose: true });
            var params = KMeans.getParams();
            assert.equal(params.iter, 15);
            assert.equal(params.k, 30);
            assert.equal(params.verbose, true);
        });
        it("should return the changed values of parameters even if added keys", function () {
            var KMeans = new analytics.KMeans();
            KMeans.setParams({ iter: 10, k: 5, alpha: false });
            var params = KMeans.getParams();
            assert.equal(params.iter, 10);
            assert.equal(params.k, 5);
            assert.equal(params.verbose, false);
        });
    });
    describe("Fit test", function () {
        it("should create the model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans();
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans();
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the correct model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, 11);
            assert.equal(model.C.cols, 2);
        });
        it("should return the correct model for dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var matrix = new la.Matrix([[1, 2, 3, -10, -10, 4, 2, -10], [7, 6, 5, 5, -5, -1, -3, 0]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 8);
            assert.equal(model.C.rows, 2);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, dense centroids", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C, X);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C, X);
        });

        it("should create the model for sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse" });
            var matrix = new la.Matrix([[-1, 1], [0, 0]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should create the model for sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]]]);
            assert.doesNotThrow(function () {
                KMeans.fit(matrix);
            });
        });
        it("should return the correct model for sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var matrix = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 3);
        });
        it("should return the correct model for sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var matrix = new la.SparseMatrix([[[0, 1], [10, 2]], [[2, 3]], [[4, -1]]]);
            KMeans.fit(matrix);
            var model = KMeans.getModel();
            assert.equal(model.idxv.length, 3);
            assert.equal(model.C.rows, -1);
            assert.equal(model.C.cols, 2);
        });
        it("should return the same model even after changing parameter values, sparse centroids", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var model = KMeans.getModel();
            assert.deepEqual(model.C.rows, -1);
            assert.deepEqual(model.C.cols, 3);

            KMeans.setParams({ iter: 102 });
            var model2 = KMeans.getModel();
            assert.deepEqual(model2.C.rows, -1);
            assert.deepEqual(model2.C.cols, 3);
        });

    });
    describe("Predict Tests", function () {
        it("should not throw an exception dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1], [0, 0]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
        it("should throw an exception if the matrix has too many rows dense centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[4, 1]]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });

        it("should not throw an exception sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should not throw an exception sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[1, 1]], [[0, -1]]]);
            assert.doesNotThrow(function () {
                KMeans.predict(matrix);
            });
        });
        it("should return the predictions of the matrix sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should return the predictions of the matrix sparse centroids, sparse matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.SparseMatrix([[[0, 1]], [[1, 1]], [[0, 2], [1, 4]]]);
            var prediction = KMeans.predict(matrix);
            assert.equal(prediction.length, 3);
        });
        it("should throw an exception if the matrix has less rows sparse centroids, dense matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1]]);
            assert.throws(function () {
                KMeans.predict(matrix);
            });
        });
    });


    describe.skip("Explain Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [1,2,3]);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.explain(matrix);
            });
        });
        it("should return the predictions of the matrix", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X, [341,422,122]);
            var matrix = new la.Matrix([[-1, 2, 1], [0, 1, -3]]);
            var explanation = KMeans.explain(matrix);
            debugger
            assert.equal(explanation[0].medoidID, 422);
            assert.equal(explanation[1].medoidID, 341);
            assert.equal(explanation[2].medoidID, 122);
        });
        it("should throw an exception if the matrix columns do not match record ids", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            assert.throws(function () {
            	KMeans.fit(X , [0, 1]);
            });
        });
    });

    describe("Transform Tests", function () {
        it("should not throw an exception", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[1, 1], [0, -1]]);
            assert.doesNotThrow(function () {
                KMeans.transform(matrix);
            });
        });
        it("should return the distance", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3]]);
            var transform = KMeans.transform(matrix);
            assert.equal(transform.rows, 3);
            assert.equal(transform.cols, 2);
        });
        it("should throw an exception because the matrix dimension is too big", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0], [0, -3], [0, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
        it("should throw an exception because the matrix dimension is too short", function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var matrix = new la.Matrix([[-2, 0]]);
            assert.throws(function () {
                var transform = KMeans.transform(matrix);
            });
        });
    });

    describe('PermuteCentroids Tests', function () {
        it('should not throw an exception, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([2, 0, 1]);
            assert.doesNotThrow(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should not throw an exception, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 3, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([2, 0, 1]);
            assert.doesNotThrow(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should permute the centroids, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Cent1 = KMeans.getModel().C;
            var Vec = new la.IntVector([1, 0]);
            KMeans.permuteCentroids(Vec);
            var Cent2 = KMeans.getModel().C;
            assert.equal(Cent1.at(0, 0), Cent2.at(0, 1));
            assert.equal(Cent1.at(0, 1), Cent2.at(0, 0));
            assert.equal(Cent1.at(1, 0), Cent2.at(1, 1));
            assert.equal(Cent1.at(1, 1), Cent2.at(1, 0));
        })
        it('should permute the centroids, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Cent1 = KMeans.getModel().C;
            var Vec = new la.IntVector([1, 0]);
            KMeans.permuteCentroids(Vec);
            var Cent2 = KMeans.getModel().C;
            assert.equal(Cent1.at(0, 0), Cent2.at(0, 1));
            assert.equal(Cent1.at(0, 1), Cent2.at(0, 0));
            assert.equal(Cent1.at(1, 0), Cent2.at(1, 1));
            assert.equal(Cent1.at(1, 1), Cent2.at(1, 0));
        })
        it('should throw an exception if the mapping vector is too short, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should throw an exception if the mapping vector is too short, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })

        it('should throw an exception if the mapping vector is too long, dense centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0, 1, 2]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
        it('should throw an exception if the mapping vector is too long, sparse centroids', function () {
            var KMeans = new analytics.KMeans({ k: 2, centroidType: "Sparse" });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var Vec = new la.IntVector([0, 1, 2]);
            assert.throws(function () {
                KMeans.permuteCentroids(Vec);
            });
        })
    });

    describe('Serialization Tests', function () {
        it('should serialize and deserialize', function () {
            var KMeans = new analytics.KMeans({ k: 3 });
            var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
            KMeans.fit(X);
            var fin = require('qminer').fs.openWrite('kmeans_test.bin');
            KMeans.save(fin); fin.close();
            var KMeans2 = new analytics.KMeans(require('qminer').fs.openRead('kmeans_test.bin'));
            var params = KMeans.getParams();
            var params2 = KMeans2.getParams();
            assert.deepEqual(KMeans.getParams(), KMeans2.getParams());
            assert.deepEqual(KMeans.getModel().C, KMeans2.getModel().C, 1e-8);
        })
    });
});

// JavaScript source code
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

module.exports = exports = function (pathQmBinary) {
    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    var fs = qm.fs;
    var la = qm.la;
    var stat = qm.statistics;
    exports = qm.deprecated;

    var sget = require('sget');
    var assert = require('assert');

    var qm_util = require(__dirname + '/qm_util.js');

    /**
   * @classdesc KMeans clustering
   * @class
   * @property {number} iter - The maximum number of iterations.
   * @property {number} k - The number of centroids.
   * @property {boolean} verbose - If false, the console output is supressed.
   * @property {Array} fitIdx - Array of indexes that should be used as starting centroids. Optional.
   * @property {model} fitStart - Model from another KMeans algorithm (obtained via getModel() method). Its centroids are used as starting centroids for this model. Optional.
   * @example
   * // import analytics and la modules
   * var analytics = require('qminer').analytics;
   * var la = require('qminer').la;
   * // create a KMeans object
   * var KMeans = new analytics.KMeans();
   * // create the matrix to be fitted
   * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
   * // create the model
   * KMeans.fit(X);
   */
    exports.KMeans = function (param) {

        // Fit params
        // var iter = param.iter == undefined ? 100 : param.iter;
        // var k = param.k == undefined ? 2 : param.k;
        // var verbose = param.verbose == undefined ? false : param.verbose;
        // var fitIdx = param.fitIdx == undefined ? undefined : param.fitIdx;

        // Model
        var C = undefined;
        var idxv = undefined;
        var norC2 = undefined;
        var iter = undefined;
        var k = undefined;
        var verbose = undefined;
        var fitIdx = undefined;
        var fitStart = undefined;
        var medoids = new la.Vector();

        if (param != undefined && param instanceof fs.FIn) {
            C = new la.Matrix(); C.load(param);
            norC2 = new la.Vector(); norC2.load(param);
            idxv = new la.IntVector(); idxv.load(param);
            var fin_params = param.readJson();
            iter = fin_params.iter;
            k = fin_params.k;
            verbose = fin_params.verbose;
            medoids.load(param);
        } else if (param == undefined || typeof param == 'object') {
            param = param == undefined ? {} : param;
            // Fit params
            iter = (param.iter == undefined) ? 100 : param.iter;
            k = (param.k == undefined) ? 2 : param.k;
            verbose = (param.verbose == undefined) ? false : param.verbose;
            fitIdx = param.fitIdx == undefined ? undefined : param.fitIdx;
            fitStart = param.fitStart == undefined ? undefined : param.fitStart;
        } else {
            throw "KMeans.constructor: parameter must be a JSON object or a fs.FIn!";
        }
        param = { iter: iter, k: k, verbose: verbose };

        /**
        * Permutes centroid with given mapping.
        * @param {object} mapping - object that contains the mapping. E.g. mapping[4]=2 means "map cluster 4 into cluster 2"
        */
        this.permuteCentroids = function (mapping) {
            var cl_count = C.cols;
            var perm_matrix = la.zeros(cl_count, cl_count);
            for (var i = 0; i < cl_count; i++) {
                perm_matrix.put(i, mapping[i], 1);
            }
            var C_new = C.multiply(perm_matrix);
            var idxv_new = new la.Vector(idxv);
            for (var i = 0; i < idxv_new.length; i++) {
                idxv_new[i] = mapping[idxv[i]]
            }
            C = C_new;
            norC2 = la.square(C.colNorms());
            idxv = idxv_new;
            if (medoids.length != 0) {
                var medoids_new = new la.Vector(medoids);
                for (var i = 0; i < medoids_new.length; i++) {
                    medoids_new[i] = mapping[medoids[i]]
                }
                medoids = medoids_new;
            }
        }
        /**
        * Returns the model
        * @returns {Object} The model object whose keys are: C (centroids) and idxv (cluster ids of the training data).
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create the KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model
        * KMeans.fit(X);
        * // get the model
        * var model = KMeans.getModel();
        */
        this.getModel = function () {
            return { C: C, idxv: idxv };
        }

        /**
        * Sets the parameters.
        * @param {Object} p - Object whose keys are: k (number of centroids), iter (maximum iterations) and verbose (if false, console output is supressed).
        * @returns {module:analytics.KMeans} Self.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans();
        * // change the parameters of the KMeans object
        * KMeans.setParams({ iter: 1000, k: 5 });
        */
        this.setParams = function (p) {
            param = p;

            iter = param.iter == undefined ? iter : param.iter;
            k = param.k == undefined ? k : param.k;
            verbose = param.verbose == undefined ? verbose : param.verbose;
            fitIdx = param.fitIdx == undefined ? fitIdx : param.fitIdx;
            fitStart = param.fitStart == undefined ? undefined : param.fitStart;
        }

        /**
        * Returns the parameters.
        * @returns Object whose keys are: k (number of centroids), iter (maximum iterations) and verbose (if false, console output is supressed).
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 5 });
        * // get the parameters
        * var json = KMeans.getParams();
        */
        this.getParams = function () {
            return { iter: iter, k: k, verbose: verbose }
        }

        /**
        * Computes the centroids.
        * @param {(module:la.Matrix | module:la.SparseMatrix)} X - Matrix whose columns correspond to examples.
        * @param {module:la.IntVector} [recIds] - IDs of columns of X. The fit function stores the IDs of the medoids, which are used by the KMeans.explain function.
        * @returns {module:analytics.KMeans} Self. It stores the info about the new model.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        */
        this.fit = function (X, recIds) {
            // select random k columns of X, returns a dense C++ matrix
            var selectCols = function (X, k) {
                if (fitStart) {
                    assert(fitStart.C.cols == k, "Error: fitStart.C.cols is not of length k!");
                    var result = {};
                    result.C = fitStart.C;
                    result.idx = la.randi(X.cols, k); // this assignment is irrelevant, really
                    return result;
                }
                var idx;
                if (fitIdx == undefined) {
                    idx = la.randi(X.cols, k);
                } else {
                    assert(fitIdx.length == k, "Error: fitIdx is not of length k!");
                    assert(Math.max.apply(Math, fitIdx) < X.cols, "Error: fitIdx contains index greater than number of columns in matrix. Index out of range!");
                    idx = fitIdx;
                }
                var idxMat = new la.SparseMatrix({ cols: 0, rows: X.cols });
                for (var i = 0; i < idx.length; i++) {
                    var spVec = new la.SparseVector([[idx[i], 1.0]], X.cols);
                    idxMat.push(spVec);
                }
                var C = X.multiply(idxMat);
                var result = {};
                result.C = C;
                result.idx = idx;
                return result;
            };

            // modified k-means algorithm that avoids empty centroids
            // A Modified k-means Algorithm to Avoid Empty Clusters, Malay K. Pakhira
            // http://www.academypublisher.com/ijrte/vol01/no01/ijrte0101220226.pdf
            var getCentroids = function (X, idx, oldC) {
                // select random k columns of X, returns a dense matrix
                // 1. construct a sparse matrix (coordinate representation) that encodes the closest centroids
                var idxvec = new la.IntVector(idx);
                var rangeV = la.rangeVec(0, X.cols - 1);
                var ones_cols = la.ones(X.cols);
                var idxMat = new la.SparseMatrix(idxvec, rangeV, ones_cols, X.cols);
                idxMat = idxMat.transpose();
                var ones_n = la.ones(X.cols);
                // 2. compute the number of points that belong to each centroid, invert
                var colSum = idxMat.multiplyT(ones_n);
                for (var i = 0; i < colSum.length; i++) {
                    var val = 1.0 / (1.0 + colSum.at(i)); // modification
                    colSum.put(i, val);
                }
                // 3. compute the centroids
                //var w = new qm_util.clsStopwatch();
                //w.tic();
                var sD = colSum.spDiag();
                var C = oldC;
                if (idxMat.cols == oldC.cols)
                    C = ((X.multiply(idxMat)).plus(oldC)).multiply(sD); // modification
                return C;
            };


            // X: column examples
            // k: number of centroids
            // iter: number of iterations
            assert(k <= X.cols, "k <= X.cols");
            var w = new qm_util.clsStopwatch();
            var norX2 = la.square(X.colNorms());
            var initialCentroids = selectCols(X, k);
            C = initialCentroids.C;
            var idxvOld = initialCentroids.idx;
            //printArray(idxvOld); // DEBUG
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            w.tic();
            for (var i = 0; i < iter; i++) {
                //console.say("iter: " + i);
                norC2 = la.square(C.colNorms());
                //D =  full(C'* X) - norC2' * (0.5* ones(1, n)) - (0.5 * ones(k,1) )* norX2';
                var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
                idxv = new la.IntVector(la.findMaxIdx(D));

                if (verbose) {
                    var energy = 0.0;
                    for (var j = 0; j < X.cols; j++) {
                        if (D.at(idxv[j], j) < 0) {
                            energy += Math.sqrt(-2 * D.at(idxv[j], j));
                        }
                    }
                    console.log("energy: " + 1.0 / X.cols * energy);
                }
                if (qm_util.arraysIdentical(idxv, idxvOld)) {
                    if (verbose) {
                        console.log("converged at iter: " + i); //DEBUG
                    }
                    break;
                }
                idxvOld = new la.IntVector(idxv);
                C = getCentroids(X, idxv, C); //drag
            }
            if (verbose) {
                w.toc("end");
            }
            norC2 = la.square(C.colNorms());
            if (recIds != undefined) {
                assert(recIds.length == X.cols);
                var D = X.multiplyT(C).minus(ones_n.outer(norC2)).minus(norX2.outer(ones_k));
                medoidIdx = la.findMaxIdx(D);
                medoids = new la.Vector(medoidIdx);
                for (var i = 0; i < medoids.length; i++) {
                    medoids[i] = recIds[medoidIdx[i]];
                }
            }
        };

        /**
        * Returns an vector of cluster id assignments.
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
        * @returns {module:la.IntVector} Vector of cluster assignments.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        * // create the matrix of the prediction vectors
        * var pred = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
        * // predict the values
        * var prediction = KMeans.predict(pred);
        */
        this.predict = function (X) {
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            var norX2 = la.square(X.colNorms());
            var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
            return la.findMaxIdx(D);
        }

        /**
        * @typedef KMeansExplanation
        * @type {Object}
        * @property {number} medoidID - The ID of the nearest medoids
        * @property {module:la.IntVector} featureIDs - The IDs of features, sorted by contribution
        * @property {module:la.Vector} featureContributions - Weights of each feature contribution (sum to 1.0)
        */

        /**
        * Returns the IDs of the nearest medoid for each example.
        * @param {(module:la.Matrix | module:la.SparseMatrix)} X - Matrix whose columns correspond to examples.
        * @returns {Array.<KMeansExplanation>} Object containing the vector of medoid IDs.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // import linear algebra module
        * var la = require('qminer').la;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X using the column IDs [0,1,2]
        * KMeans.fit(X, [1234,1142,2355]);
        * // create the matrix of the prediction vectors
        * var test = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
        * // predict/explain - return the closest medoids
        * var explanation = KMeans.explain(test);
        */
        this.explain = function (X) {
            if (medoids == undefined) {
                return { medoidIDs: null };
            }
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            var norX2 = la.square(X.colNorms());
            var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
            var centroids = la.findMaxIdx(D);
            var medoidIDs = new la.IntVector(centroids);
            assert(medoids.length == k);
            var result = [];
            for (var i = 0; i < centroids.length; i++) {
                var explanation = featureContrib(X.getCol(i), C.getCol(centroids[i]));
                result[i] = {
                    medoidID: medoids[centroids[i]],
                    featureIDs: explanation.featureIDs,
                    featureContributions: explanation.featureContributions
                }
            }
            return result;
        }

        /**
        * Returns the weights and feature IDs that contributed to the distance between two vectors
        * @param {(module:la.Vector | module:la.SparseVector)} x - Vector
        * @param {(module:la.Vector | module:la.SparseVector)} y - Vector
        * @returns {Object} Feature IDs and feature contributions
        **/
        function featureContrib(x, y) {
            var fx = x.constructor.name == 'SparseVector' ? x.full() : x;
            var fy = y.constructor.name == 'SparseVector' ? y.full() : y;
            var diff = fx.minus(fy);
            var nor2 = Math.pow(diff.norm(), 2);
            for (var i = 0; i < diff.length; i++) {
                diff[i] = Math.pow(diff[i], 2) / nor2;
            }
            var sorted = diff.sortPerm(false); // sort descending
            return { featureIDs: sorted.perm, featureContributions: sorted.vec };
        }

        /**
        * Transforms the points to vectors of squared distances to centroids.
        * @param {(module:la.Matrix | module:la.SparseMatrix)} A - Matrix whose columns correspond to examples.
        * @returns {module:la.Matrix} Matrix where each column represents the squared distances to the centroid vectors.
        * @example
        * // import modules
        * var analytics = require('qminer').analytics;
        * var la = require('qminer').la;
        * // create a new KMeans object
        * var KMeans = new analytics.KMeans({ iter: 1000, k: 3 });
        * // create a matrix to be fitted
        * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
        * // create the model with the matrix X
        * KMeans.fit(X);
        * // create the matrix of the transform vectors
        * var matrix = new la.Matrix([[-2, 0], [0, -3]]);
        * // get the transform values of matrix
        * // returns the matrix
        * //  10    17
        * //   1    20
        * //  10     1
        * KMeans.transform(matrix);
        */
        this.transform = function (X) {
            var ones_n = la.ones(X.cols).multiply(0.5);
            var ones_k = la.ones(k).multiply(0.5);
            var norX2 = la.square(X.colNorms());
            var D = C.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
            D = D.multiply(-2);
            return D;
        }
        /**
        * Saves KMeans internal state into (binary) file.
        * @param {module:fs.FOut} arg - The output stream.
        * @returns {module:fs.FOut} The output stream fout.
        */
        this.save = function (fout) {
            if (!C) { throw new Error("KMeans.save() - model not created yet"); }
            C.save(fout);
            norC2.save(fout);
            idxv.save(fout);
            fout.writeJson({
                iter: iter,
                k: k,
                verbose: verbose
            });
            medoids.save(fout);
            return fout;
        }
    }

    //!ENDJSDOC

    return exports;
}
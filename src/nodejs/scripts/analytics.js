"use strict";
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 *
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */

var sget = require(__dirname + '/third_party/sget/sget.js');
var override = require(__dirname + '/third_party/json-override/json-override.js');
var assert = require('assert');
var qm_util = require(__dirname + '/qm_util.js');

module.exports = exports = function (pathQmBinary) {

    var qm = require(pathQmBinary); // This loads only c++ functions of qm
    var fs = qm.fs;
    var la = qm.la;
    var stat = qm.statistics;
    exports = qm.analytics;

    //!STARTJSDOC

    ///////////////////////////////////////////////////
    /////////////   DATA PREPROCESSING   //////////////
    ///////////////////////////////////////////////////

    /**
    * PreprocessingF
    * @namespace
    * @desc Preprocessing functions for preparing labels in formats accepted
    * by learning modules in {@link module:analytics}.
    */
    var preprocessing = preprocessing || {};
    // namespacing: http://addyosmani.com/blog/essential-js-namespacing/

    /**
    * Transforming arrays with labels to vector appropriate for binary classifiers.
    * @class
    * @classdesc
    * Transform given array of labels into binary vector with different
    * numeric value for elements when label matches specified label and
    * for other elements. By default, these values are +1 for matching
    * labels, and -1 for the rest.
    * @param {Array} y - Labels.
    * @param {string | number} positiveLabel - Positive label.
    * @param {number} [positiveId = 1] - Value when matching positive label.
    * @param {number} [negativeId = -1] - Value when not matching positive label.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create binarizer with 'b' as positive label
    * var binarizer = new analytics.preprocessing.Binarizer('b');
    */
    preprocessing.Binarizer = function (positiveLabel, positiveId, negativeId) {
        if (positiveLabel == undefined) { throw "Binarizer needs positive label"; }

        this.positiveLabel = positiveLabel;
        this.positiveId = (positiveId == undefined) ? 1 : positiveId;
        this.negativeId = (negativeId == undefined) ? -1 : negativeId;

        this.fit = function () {
            // do nothing
        }

        /**
        * Transform given array of labels to binary numeric vector.
        * @param {(Array<number> | Array<string> | module:la.Vector | module:la.StrVector)} y - Labels.
        * @return {modul:la.Vector} Binarized vector.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create binarizer with 'b' as positive label
        * var binarizer = new analytics.preprocessing.Binarizer('b');
        * // get vector with binarized labels
        * var bins = binarizer.transform(['a','b','a','c']);
        */
        this.transform = function (y) {
            var target = new la.Vector();
            for (var i = 0; i < y.length; i++) {
                target.push(y[i] === this.positiveLabel ? this.positiveId : this.negativeId);
            }
            return target;
        }
    };

    /**
    * Applies the model's `decisionFunction` method (if exists) on each column of matrix `X`.
    * @param {Object} model - The model, that has the `decisionFunction` method.
    * @param {module:la.SparseMatrix} X - The matrix.
    * @returns {module:la.Vector} The dense vector where the i-th value is the value the `model.decisionFunction`
    * returned for the sparse vector `X[i]`.
    * @example
    * // TODO
    */
    preprocessing.applyModel = function (model, X) {
        if (model.decisionFunction == undefined) {
            throw "preprocessing.applyModel: model doesn't have a method called decisionFunction!";
        }
        var target = new la.Vector();
        for (var i = 0; i < X.cols; i++) {
            target.push(model.decisionFunction(X[i]));
        }
        return target;
    }

    // Exports preprocessing namespace
    exports.preprocessing = preprocessing;

    // SVM
    /**
    * Get the model.
    * @returns {Object} The `svmModel` object containing the property:
    * <br> 1. `svmModel.weights` - The weights of the model. Type {@link module:la.Vector}.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a SVC model
    * var SVC = new analytics.SVC();
    * // get the properties of the model
    * var model = SVC.getModel();
    */
    exports.SVC.prototype.getModel = function() { return { weights: this.weights, bias: this.bias }; }
    /**
    * Get the model.
    * @returns {Object} The `svmModel` object containing the property:
    * <br> 1. `svmModel.weights` - The weights of the model. Type {@link module:la.Vector}.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a SVR model
    * var SVR = new analytics.SVR();
    * // get the properties of the model
    * var model = SVR.getModel();
    */
    exports.SVR.prototype.getModel = function() { return { weights: this.weights, bias: this.bias }; }

    // Ridge Regression
    /**
    * Gets the model.
    * @returns {Object} The `ridgeRegModel` object containing the property:
    * <br> 1. `ridgeRegModel.weights` - The weights of the model. Type {@link module:la.Vector}.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create the Ridge Regression model
    * var regmod = new analytics.RidgeReg();
    * // get the model
    * var model = regmod.getModel();
    */
    exports.RidgeReg.prototype.getModel = function () { return { weights: this.weights }; }

    // Recursive Linear Regression
    /**
    * Gets the model.
    * @returns {Object} The `recLinRegModel` object containing the property:
    * <br> 1. `recLinRegModel.weights` - The weights of the model. Type {@link module:la.Vector}.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create the Recursive Linear Regression model
    * var linreg = new analytics.RecLinReg({ dim: 10 });
    * // get the model
    * var model = linreg.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    */
    exports.RecLinReg.prototype.getModel = function () { return { weights: this.weights } }

    /**
    * @typedef {Object} oneVsAllParam
    * An object used for the construction of {@link module:analytics.OneVsAll}.
    * @property {function} [model] - Constructor for binary model to be
    * used internaly. Constructor should expect only one parameter.
    * @property {Object} [modelParam] - Parameter for `oneVsAllParam.model` constructor.
    * @property {number} [categories] - Number of categories.
    * @property {boolean} [verbose = false] - If false, the console output is supressed.
    */

    /**
    * @classdesc One vs All model for multiclass prediction. Builds binary model
    * for each category and predicts the one with the highest score. Binary model is
    * provided as part of the constructor.
    * @class
    * @param {module:analytics~oneVsAllParam} [arg] - Construction arguments.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new OneVsAll object with the model analytics.SVC
    * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
    */
    exports.OneVsAll = function (arg) {
        // remember parameters
        var model = arg.model;
        var modelParam = arg.modelParam;
        var cats = arg.cats;
        var verbose = arg.verbose == undefined ? false : arg.verbose;
        // trained models
        var models = [ ];

        /**
        * Gets the parameters.
        * @returns {module:analytics~oneVsAllParam} The constructor parameters.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
        * // get the parameters
        * // returns the JSon object
        * // { model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2, models: [] }
        * var params = onevsall.getParams();
        */
        this.getParams = function () {
            return { model: model, modelParam: modelParam, cats: cats, models: models }
        };

        /**
        * Sets the parameters.
        * @param {module:analytics~OneVsAllParam} params - The constructor parameters.
        * @returns {module:analytics.OneVsAll} Self. The parameters are changed.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
        * // set the parameters
        * var params = onevsall.setParams({ model: analytics.SVR, modelParam: { c: 12, maxTime: 10000}, cats: 3, verbose: true });
        */
        this.setParams = function (params) {
            model = params.model == undefined ? model : params.model;
            modelParam = params.modelParam == undefined ? modelParam : params.modelParam;
            cats = params.cats == undefined ? cats : params.cats;
            verbose = params.verbose == undefined ? verbose : params.verbose;
        }

        /**
         * Apply all models to the given vector and returns a vector of scores, one for each category.
         * Semantic of scores depend on the provided binary model.
         * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X -
         * Feature vector or matrix with feature vectors as columns.
         * @returns {module:la.Vector | module:la.Matrix} The score and label of the input `X`:
         * <br>1. {@link module:la.Vector} of scores, if `X` is of type {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.Matrix} with columns corresponding to instances, and rows corresponding to labels, if `X` is of type {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         * // create the vector for the decisionFunction
         * var test = new la.Vector([1, 2]);
         * // give the vector to the decision function
         * var prediction = onevsall.decisionFunction(test); // returns the vector of scores
         */
        this.decisionFunction = function(X) {
            // check what is our input
            if (X instanceof la.Vector || X instanceof la.SparseVector) {
                // evaluate all models
                var scores = new la.Vector();
                for (var cat = 0; cat < cats; cat++) {
                    scores.push(models[cat].decisionFunction(X));
                }
                return scores;
            } else if (X instanceof la.Matrix || X instanceof la.SparseMatrix) {
                // create matrix where cols are instances and rows are scores for categories
                var scores = new la.Matrix({rows: cats, cols: X.cols});
                for (var i = 0; i < X.cols; i++) {
                    var x_i = X.getCol(i);
                    for (var cat = 0; cat < cats; cat++) {
                        scores.put(cat, i, models[cat].decisionFunction(x_i));
                    }
                }
                return scores;
            } else {
                throw "analytics.OneVsAll.decisionFunction: Input data of unsupported type!";
            }
        }

        /**
         * Apply all models to the given vector and returns category with the highest score.
         * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X -
         * Feature vector or matrix with feature vectors as columns.
         * @returns {number | module:la.IntVector}
         * <br>1. number of the category with the higher score, if `X` is {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.IntVector} of categories with the higher score for each column of `X`, if `X` is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         * // create the vector for the prediction
         * var test = new la.Vector([1, 2]);
         * // get the prediction of the vector
         * var prediction = onevsall.predict(test); // returns 0
         */
        this.predict = function(X) {
            // evaluate all models
            var scores = this.decisionFunction(X);
            // select maximal one
            if (scores instanceof la.Vector) {
                return scores.getMaxIdx();
            } else if (scores instanceof la.Matrix) {
                var predictions = new la.IntVector();
                for (var i = 0; i < scores.cols; i++) {
                    predictions.push(scores.getCol(i).getMaxIdx());
                }
                return predictions;
            } else {
                throw "analytics.OneVsAll.predict: decisionFunction returns unsupported type!";
            }
        }

        // X = feature matrix
        // y = target label from 0..cats
        /**
         * Apply all models to the given vector and returns category with the highest score.
         * @param {module:la.Matrix | module:la.SparseMatrix} X - training instance feature vectors.
         * @param {module:la.Vector} y - target category for each training instance. Categories must
         * be integer numbers between `0` and `oneVsAllParam.categories-1`.
         * @returns {module:analytics.OneVsAll} Self. The models have been fitted.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 1000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         */
        this.fit = function(X, y) {
            models = [ ];
            // make model for each category
            for (var cat = 0; cat < cats; cat++) {
                if (verbose) {
                    console.log("Fitting label", (cat + 1), "/", cats);
                };
                // prepare targert vector for current category
                var target = (y instanceof la.Matrix) ?
                    // we have a special bianary vector for each category, make it into -1/+1
                    (new exports.preprocessing.Binarizer(1)).transform(y.getRow(cat)) :
                    // we have a vector with label for each element, get out -1/+1 vector
                    (new exports.preprocessing.Binarizer(cat)).transform(y);
                // get the model
                var catModel = new model(modelParam);
                models.push(catModel.fit(X, target));
            }
            if (verbose) {
                console.log("Done!");
            };
            return this;
        }
    };

    /**
     * Threshold Model
     * @class
     * @classdesc The Threshold model. Uses the methods from the {@link module:analytics.metrics}.
     * @param {Object} [arg] - The constructor parameters.
     * @param {string} [arg.target] - Target type. Possible options are `"recall"` and `"precision"`.
     * @param {TODO} [arg.level] - TODO
     * @example
     * // TODO
     */
    exports.ThresholdModel = function(params) {
        // what do we optimize
        this.target = params.target;
        if (this.target === "recall" || this.target === "precision") {
            this.level = params.level;
        }
        // threshold model
        this.model = null;

        // apply all models to the given vector and return distance to the class boundary
        // x = dense vector with prediction score for each class
        // result = traslated predictions based on thresholds
        /**
         * Apply all models to the given vector and returns the distance to the class boundary.
         * @param {number | module:la.Vector} x - The prediction score for each class.
         * @returns {number | module:la.Vector}
         * <br>1. value of the translated prediction based on the threshold, if `x` is `number`,
         * <br>2. {@link module:la.Vector} of translated prediction based on the threshold, if `x` is {@link module:la.Vector}.
         * @example
         * // TODO
         */
        this.decisionFunction = function(x) {
            if (x instanceof Number) {
                // just transate based on the model's threshold
                return x - this.model;
            } else if (x instanceof la.Vector) {
                // each element is a new instance
                var scores = new la.Vector();
                for (var i = 0; i < x.length; i++) {
                    scores.push(x[i] - this.model);
                }
                return scores;
            } else {
                throw "analytics.ThresholdModel.decisionFunction: Input data of unsupported type!";
            }
        }

        // return the most likely category
        // x = dense vector with prediction score for each class
        // result = array of positive label ids
        /**
         * Returns the most likely category.
         * @param {number | module:la.Vector} x - The prediction score for each class.
         * @returns {number | module:la.Vector}
         * <br>1. value of the positive label IDs, if `x` is `number`,
         * <br>2. {@link module:la.Vector} of the positive label IDs, if `x` is {@link module:la.Vector}.
         * @example
         * // TODO
         */
        this.predict = function(x) {
            // evaluate all models
            var scores = this.decisionFunction(x)
            // check what we get
            if (scores instanceof la.Vector) {
                return res = new la.Vector();
                for (var i = 0; i < scores.length; i++) {
                    res.push(scores[i] > 0 ? 1 : -1);
                }
                return res;
            } else {
                return scores > 0 ? 1 : -1;
            }
        }

        // X = vector of predictions for each instance (output of decision_funcition)
        // y = target labels (1 or -1)
        /**
         * Fits the model.
         * @param {module:la.Vector} X - Prediction for each instance (output of descisionFunction).
         * @param {number} y - The target labels (1 or -1).
         * @example
         * // TODO
         */
        this.fit = function(X, y) {
            if (this.target === "f1") {
                // find threshold that maximizes F1 measure
                this.model = exports.metrics.bestF1Threshold(y, X);
            } else if (this.target === "recall") {
                // find threshold that results in desired recall
                this.model = exports.metrics.desiredRecallThreshold(y, X, this.level);
            } else if (this.target === "precision") {
                // find threshold that results in desired precision
                this.model = exports.metrics.desiredPrecisionThreshold(y, X, this.level);
            } else {
                throw "Unknown threshold model target: " + this.target;
            }
        }
    }


    /**
    * Metrics
    * @namespace
    * @desc Classification and regression metrics.
    * @example <caption>Batch classification example</caption>
    * // import metrics module
    * var analytics = require('qminer').analytics;
    *
    * // true and predicted lables
    * var true_lables = [0, 1, 0, 0, 1];
    * var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    *
    * // compute ROC curve
    * var roc = analytics.metrics.rocCurve(true_lables, pred_prob);
    * @example <caption>Online classification example</caption>
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // true and predicted lables
    * var true_lables = [0, 1, 0, 0, 1];
    * var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    *
    * // create predictionCurve instance
    * var predictionCurve = new analytics.metrics.PredictionCurve();
    *
    * // simulate data flow
    * for (var i in true_lables) {
    *    // push new value
    *    predictionCurve.push(true_lables[i], pred_prob[i]);
    *}
    *
    * var roc = predictionCurve.roc(); // get ROC
    * @example <caption>Batch regression example</caption>
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // true and predicted data
    * var true_vals = [1, 2, 3, 4, 5];
    * var pred_vals = [3, 4, 5, 6, 7];
    *
    * // use batch MAE method
    * analytics.metrics.meanAbsoluteError(true_vals, pred_vals);
    * @example <caption>Online regression example</caption>
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // true and predicted data
    * var true_vals = [1, 2, 3, 4, 5];
    * var pred_vals = [3, 4, 5, 6, 7];
    *
    * // create online MAE metric instance
    * var mae = new analytics.metrics.MeanAbsoluteError();
    *
    * // simulate data flow
    * for (var i in true_vals) {
    *   // push new value
    *   mae.push(true_vals[i], pred_vals[i]);
    * }
    * // get updated error
    * mae.getError();
    */
    var metrics = metrics || {};
    // namespacing: http://addyosmani.com/blog/essential-js-namespacing/

    ///////////////////////////////////////////////////
    ///////////// CLASSIFICATION METRICS //////////////
    ///////////////////////////////////////////////////

    /**
    * For evaluating provided categories from binary? classifiers.
    * @class
    * @classdesc Class implements several classification measures (precision, recall, F1, accuracy).
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lable(s).
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lable(s).
    */
    metrics.ClassificationScore = function (yTrue, yPred) {
        /**
        * Returns `Object` containing different classification measures.
        * @returns {Object} scores - Object with different classification socres.
        * @returns {number} scores.count - Count.
        * @returns {number} scores.TP - Number of true positives.
        * @returns {number} scores.TN - Number of true negative.
        * @returns {number} scores.FP - Number of false positives.
        * @returns {number} scores.FN - Number of false positives.
        * @returns {number} scores.all - Number of all results.
        * @returns {number} scores.accuracy - Accuracy score. Formula: `(tp + tn) / (tp + fp + fn + tn)`.
        * @returns {number} scores.precision - Precision score. Formula: `tp / (tp + fp)`.
        * @returns {number} scores.recall - Recall score. Formula: `tp / (tp + fn)`.
        * @returns {number} scores.f1 - F1 score. Formula:  `2 * (precision * recall) / (precision + recall)`.
        */
        this.scores = {
            count: 0, predictionCount: 0,
            TP: 0, TN: 0, FP: 0, FN: 0,
            all: function () { return this.TP + this.FP + this.TN + this.FN; },
            precision: function () { return (this.FP == 0) ? 1 : this.TP / (this.TP + this.FP); },
            recall: function () { return (this.FN == 0) ? 1 : this.TP / (this.TP + this.FN); },
            f1: function () { return ((this.precision() + this.recall()) == 0) ? 0 :
                2 * this.precision() * this.recall() / (this.precision() + this.recall()); },
            accuracy: function () { return (this.TP + this.TN) / this.all(); }
        };

        /**
        * Adds prediction to the current statistics. Labels can be either integers.
        * or integer array (when there are zero or more then one lables).
        * @param {number} correct - Correct lable.
        * @param {number} predicted - Predicted lable.
        */
        this.push = function (correct, predicted) {
            var catCorrect = (correct > 0);
            var catPredicted = (predicted > 0);
            // update counts for correct categories
            if (catCorrect) { this.scores.count++; }
            // update counts for how many times category was predicted
            if (catPredicted) { this.scores.predictionCount++; }
            // update true/false positive/negative count
            if (catCorrect && catPredicted) {
                // both predicted and correct say true
                this.scores.TP++;
            } else if (catCorrect) {
                // this was only correct but not predicted
                this.scores.FN++;
            } else if (catPredicted) {
                // this was only predicted but not correct
                this.scores.FP++;
            } else {
                // both predicted and correct say false
                this.scores.TN++;
            }
        };

        // initialize if we are passed the data
        if (arguments.length >= 2) {
            for (var i = 0; i < yTrue.length; i++) {
                this.push(yTrue[i], yPred[i]);
            }
        }

        // check if input parameters are of correct type and binary
        for (var i = 0; i < arguments.length; i++) {
            // check type
            var argumentType = arguments[i].constructor.name;
            if (argumentType !== "Array" && argumentType !== "Vector") {
                throw new TypeError('input param must be of type "Array" or "Vector", but is ' + argumentType + ' instead');
            }
        }
    };

    /**
    * Accuracy score is the proportion of true results (both true positives and true negatives)
    * among the total number of cases examined.
    * Formula: `(tp + tn) / (tp + fp + fn + tn)`.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables.
    * @returns {number} Accuracy value.
    */
    metrics.accuracyScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.accuracy();
    };

    /**
    * Precision score is defined as the proportion of the true positives against all the
    * positive results (both true positives and false positives).
    * Formula: `tp / (tp + fp)`.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables.
    * @returns {number} Precission score.
    */
    metrics.precisionScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.precision();
    };

    /**
    * Recall score is intuitively the ability of the classifier to find all the positive samples.
    * Formula: `tp / (tp + fn)`.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables.
    * @returns {number} Recall score.
    */
    metrics.recallScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.recall();
    };

    /**
    * The F1 score can be interpreted as a weighted average of the precision and recall, where
    * an F1 score reaches its best value at 1 and worst score at 0. The relative contribution of
    * precision and recall to the F1 score are equal.
    * Formula: `2 * (precision * recall) / (precision + recall)`.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables.
    * @returns {number} F1 score.
    */
    metrics.f1Score = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.f1();
    };

    /**
    * Class implements several prediction curve measures (ROC, AOC, Precision-Recall, ...).
    * @class
    * @classdesc Used for computing ROC curve and other related measures such as AUC.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lable(s) of binary classification in range {-1, 1} or {0, 1}.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @example
    * // import metrics module
    * var metrics = require('qminer').analytics.metrics;
    *
    * // true and predicted lables
    * var true_lables = [0, 1, 0, 0, 1];
    * var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    *
    * // create predictionCurve instance
    * var predictionCurve = new metrics.PredictionCurve();
    *
    * // simulate data flow
    * for (var i in true_lables) {
    *    // push new value
    *    predictionCurve.push(true_lables[i], pred_prob[i]);
    *}
    *
    * var roc = predictionCurve.roc(); // get ROC
    * var auc = predictionCurve.auc(); // get AUC
    * var pr = predictionCurve.precisionRecallCurve() // get precision-recall curve
    */
    metrics.PredictionCurve = function (yTrue, yPred) {
        /**
        * Count of all examples.
        * @name module:analytics~metrics.PredictionCurve#length
        * @type number
        */
        this.length = 0;
        /**
        * Count of all positive examples.
        * @name module:analytics~metrics.PredictionCurve#allPositives
        * @type number
        */
        this.allPositives = 0;
        /**
        * Count of all negative examples.
        * @name module:analytics~metrics.PredictionCurve#allNegatives
        * @type number
        */
        this.allNegatives = 0;
        // store of predictions and ground truths
        /**
        * Store of ground truths.
        * @name module:analytics~metrics.PredictionCurve#grounds
        * @type module:la.Vector
        */
        this.grounds = new la.Vector();
        /**
        * Store of predictions.
        * @name module:analytics~metrics.PredictionCurve#predictions
        * @type module:la.Vector
        */
        this.predictions = new la.Vector();

        /**
        * Add new measurement with ground score (1 or -1) and predicted value
        * or integer array (when there are zero or more then one lables).
        * @param {number} ground - Correct lable.
        * @param {number} predicted - Estimated probabilities.
        */
        this.push = function (ground, predict) {
            // remember the scores
            this.grounds.push(ground)
            this.predictions.push(predict);
            // update counts
            this.length++;
            if (ground > 0) {
                this.allPositives++;
            } else {
                this.allNegatives++;
            }
        };

        // initialize if we are given data
        if (arguments.length >= 2) {
            for (var i = 0; i < yTrue.length; i++) {
                this.push(yTrue[i], yPred[i]);
            }
        }

        // check if input parameters are of correct type and binary
        for (var i = 0; i < arguments.length; i++) {
            // check type
            var argumentType = arguments[i].constructor.name;
            if (argumentType !== "Array" && argumentType !== "Vector") {
                throw new TypeError('input param must be of type "Array" or "Vector", but is ' + argumentType + ' instead');
            }
        }

        /**
        * Get Receiver Operating Characteristic (ROC) parametrization sampled on `sample` points.
        * @param {number} [sample=10] - Desired number of samples in output.
        * @returns {module:la.Matrix} A matrix with increasing false and true positive rates.
        */
        this.roc = function (sample) {
            // default sample size is 10
            sample = sample || 10;
            // sort according to predictions
            var perm = this.predictions.sortPerm(false);
            // maintaining the results as we go along
            var TP = 0, FP = 0, ROC = [[0, 0]];

            // check input samples
            if (this.allNegatives == 0) throw new Error('No positive samples in yTrue, true positive value should be meaningless.');
            if (this.allNegatives == this.length) throw new Error('No negative samples in yTrue, false positive value should be meaningless.');

            // for figuring out when to dump a new ROC sample
            var unique = 1;
            for (var i = 1; i < perm.perm.length; i++) {
                if (Math.abs(perm.vec[i] - perm.vec[i - 1]) > 1e-8) {
                    unique++;
                }
            }

            var next = Math.floor(unique / sample);

            // go over the sorted results
            for (var i = 0; i < perm.perm.length; i++) {
                // get the ground
                var ground = this.grounds[perm.perm[i]];
                // update TP/FP counts according to the ground
                if (ground > 0) { TP++ } else { FP++; }

                // see if time to do next save
                if ((i < perm.perm.length - 1) && (Math.abs(perm.vec[i] - perm.vec[i + 1]) > 1e-8)) {
                    next = next - 1;
                }

                if (next < 0) {
                    // add new datapoint to the curve
                    ROC.push([FP / this.allNegatives, TP / this.allPositives]);
                    // setup next timer
                    next = Math.floor(unique / sample);
                }
            }
            // add the last point
            ROC.push([1, 1]);
            // return ROC
            return ROC;
        }

        /**
        * Get Area Under the Curve (AUC) of the current curve.
        * @param {number} [sample=10] - Desired number of samples in output.
        * @returns {number} Area under ROC curve.
        */
        this.auc = function (sample) {
            // default sample size is 10
            sample = sample || 10;
            // get the curve
            var curve = this.roc(sample);
            // compute the area
            var result = 0;
            for (var i = 1; i < curve.length; i++) {
                // get edge points
                var left = curve[i - 1];
                var right = curve[i];
                // first the rectangle bellow
                result = result + (right[0] - left[0]) * left[1];
                // an then the triangle above
                result = result + (right[0] - left[0]) * (right[1] - left[1]) / 2;
            }
            return result;
        }

        /**
        * evalPrecisionRecall.
        * @private
        * @param {callback} callback.
        */
        this.evalPrecisionRecall = function (callback) {
            // sort according to predictions
            var perm = this.predictions.sortPerm(false);
            // maintaining the results as we go along
            var TP = 0, FP = 0, TN = this.allNegatives, FN = this.allPositives;
            // go over the sorted results
            for (var i = 0; i < perm.perm.length; i++) {
                // get the ground
                var ground = this.grounds[perm.perm[i]];
                // update TP/FP counts according to the ground
                if (ground > 0) { TP++; FN--; } else { FP++; TN--; }
                // do the update
                if ((TP + FP) > 0 && (TP + FN) > 0 && TP > 0) {
                    // compute current precision and recall
                    var precision = TP / (TP + FP);
                    var recall = TP / (TP + FN);
                    // see if we need to update current bep
                    callback.update(ground, perm.vec[i], precision, recall);
                }
            }
            return callback.finish();
        }

        /**
        * Get precision recall curve sampled on `sample` points.
        * @param {number} [sample=10] - Desired number of samples in output.
        * @returns {module:la.Matrix} Precision-recall pairs.
        */
        this.precisionRecallCurve = function (sample) {
            return this.evalPrecisionRecall(new function (sample, length) {
                // default sample size is 10
                this.sample = sample || 10;
                // curve
                this.curve = [[0, 1]];
                // for figuring out when to dump a new ROC sample
                this.next = Math.floor(length / (this.sample));
                this.counter = this.next;
                // keep last value
                this.precision = 0; this.recall = 0;
                // handlers
                this.update = function (yTrue, yPred, precision, recall) {
                    this.counter = this.counter - 1;
                    if (this.counter <= 0) {
                        // add to the curve
                        this.curve.push([recall, precision]);
                        // setup next timer
                        this.counter = this.next;
                    }
                    // always remember last value
                    this.precision = precision; this.recall = recall;
                }
                this.finish = function () {
                    // add the last point
                    this.curve.push([this.recall, this.precision]);
                    return this.curve;
                }
            }(sample, this.length));
        };

        /**
        * Get break-even point, the value where precision and recall intersect.
        * @returns {number} Break-even point.
        */
        this.breakEvenPoint = function () {
            return this.evalPrecisionRecall(new function () {
                this.minDiff = 1.0; this.bep = -1.0;
                this.update = function (yTrue, yPred, precision, recall) {
                    var diff = Math.abs(precision - recall);
                    if (diff < this.minDiff) { this.minDiff = diff; bep = (precision + recall) / 2; }
                }
                this.finish = function () { return this.bep; }
            }());
        }

        /**
        * Gets threshold for prediction score, which results in the highest F1.
        * @returns {number} Threshold with highest F1 score.
        */
        this.bestF1 = function () {
            return this.evalPrecisionRecall(new function () {
                this.maxF1 = 0.0; this.threshold = 0.0;
                this.update = function (yTrue, yPred, precision, recall) {
                    var f1 = 2 * precision * recall / (precision + recall);
                    if (f1 > this.maxF1) {
                        this.maxF1 = f1;
                        this.threshold = yPred;
                    }
                }
                this.finish = function () { return this.threshold; }
            }());
        }

        /**
        * Gets threshold for prediction score, nearest to specified recall.
        * @param {number} desiredRecall - Desired recall score.
        * @returns {number} Recal Score Threshold. Threshold for recall score, nearest to specified `recall`.
        */
        this.desiredRecall = function (desiredRecall) {
            return this.evalPrecisionRecall(new function () {
                this.recallDiff = 1.0; this.threshold = 0.0;
                this.update = function (yTrue, yPred, precision, recall) {
                    var diff = Math.abs(desiredRecall - recall);
                    if (diff < this.recallDiff) {
                        this.recallDiff = diff;
                        this.threshold = yPred;
                    }
                }
                this.finish = function () { return this.threshold; }
            }());
        }

        /**
        * Gets threshold for prediction score, nearest to specified precision.
        * @param {number} desiredPrecision - Desired precision score.
        * @returns {number} Threshold for prediction score, nearest to specified `precision`.
        */
        this.desiredPrecision = function (desiredPrecision) {
            return this.evalPrecisionRecall(new function () {
                this.precisionDiff = 1.0; this.threshold = 0.0;
                this.update = function (yTrue, yPred, precision, recall) {
                    var diff = Math.abs(desiredPrecision - precision);
                    if (diff < this.precisionDiff) {
                        this.precisionDiff = diff;
                        this.threshold = yPred;
                    }
                }
                this.finish = function () { return this.threshold; }
            }());
        }
    };

    /**
    * Get ROC parametrization sampled on `sample` points.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @param {number} [sample=10] - Desired number of samples in output.
    * @returns {module:la.Matrix} A matrix with increasing false and true positive rates.
    * @example
    * // import metrics module
    * var metrics = require('qminer').analytics.metrics;
    *
    * // true and predicted lables
    * var true_lables = [0, 1, 0, 0, 1];
    * var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    *
    * // compute ROC curve
    * var roc = metrics.rocCurve(true_lables, pred_prob); // output: [ [ 0, 0 ], [0, 0.5], [[ 0.34, 1 ],], [ 0.67, 0 ], [ 1, 1 ] ]
    */
    metrics.rocCurve = function (yTrue, yPred, sample) {
        return new metrics.PredictionCurve(yTrue, yPred).roc(sample);
    };

    /**
    * Get AUC of the current curve.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @param {number} [sample=10] - Desired number of samples in output.
    * @returns {number} Area under ROC curve.
    * @example
    * // import metrics module
    * var metrics = require('qminer').analytics.metrics;
    *
    * // true and predicted lables
    * var true_lables = [0, 1, 0, 0, 1];
    * var pred_prob = [0.3, 0.5, 0.2, 0.5, 0.8];
    *
    * // compute ROC curve
    * var auc = metrics.rocAucScore(true_lables, pred_prob); // output: 0.92
    */
    metrics.rocAucScore = function (yTrue, yPred, sample) {
        return new metrics.PredictionCurve(yTrue, yPred).auc(sample);
    };

    /**
    * Get precision recall curve sampled on `sample` points.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @param {number} [sample=10] - Desired number of samples in output.
    * @returns {module:la.Matrix} Precision-recall pairs.
    */
    metrics.precisionRecallCurve = function (yTrue, yPred, sample) {
        return new metrics.PredictionCurve(yTrue, yPred).precisionRecallCurve(sample);
    };

    /**
    * Get break-even point, the value where precision and recall intersect.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @returns {number} Break-even point score.
    */
    metrics.breakEventPointScore = function (yTrue, yPred) {
        return new metrics.PredictionCurve(yTrue, yPred).breakEvenPoint();
    };

    /**
    * Gets threshold for prediction score, which results in the highest F1.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @returns {number} Threshold with highest F1 score.
    */
    metrics.bestF1Threshold = function (yTrue, yPred) {
        return new metrics.PredictionCurve(yTrue, yPred).bestF1();
    };

    /**
    * Gets threshold for recall score, nearest to specified recall.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @param {number} desiredRecall - Desired recall score.
    * @returns {number} Threshold for recall score, nearest to specified `recall`.
    */
    metrics.desiredRecallThreshold = function (yTrue, yPred, desiredRecall) {
        return new metrics.PredictionCurve(yTrue, yPred).desiredRecall(desiredRecall);
    };

    /**
    * Gets threshold for prediction score, nearest to specified precision.
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities.
    * @param {number} desiredPrecision - Desired precision score.
    * @returns {number} Threshold for prediction score, nearest to specified `precision`.
    */
    metrics.desiredPrecisionThreshold = function (yTrue, yPred, desiredPrecision) {
        return new metrics.PredictionCurve(yTrue, yPred).desiredPrecision(desiredPrecision);
    };

    ///////////////////////////////////////////////////
    //////////// ONLINE REGRESSION METRICS ////////////
    ///////////////////////////////////////////////////

    // Online regression metrics used for evaluating online models

    // Main object for online metrics model
    /**
    * createOnlineMetric
    * @ignore
    * @class
    *
    * This provides methods used for event handling. It's not meant to
    * be used directly.
    *
    */
    function createOnlineMetric(callback) {
        var error = -1;
        this.metric = new callback(); // We can hide this later (just delete this)

        // check if input types are of correct type
        function checkPushParams() {
            for (var i = 0, j = arguments.length; i < j; i++) {
                var argumentType = arguments[i].constructor.name;
                if (argumentType !== "Number") {
                    throw new TypeError('input param ' + i + ' must be of type "Number", but is ' + argumentType + ' instead');
                }
            }
        }

        /**
        * Updates metric with ground truth target value `yTrue` and estimated target value `yPred`.
        * @ignore
        * @param {number} yTrue - Ground truth (correct) target value.
        * @param {number} yPred - Estimated target value.
        */
        this.push = function (yTrue, yPred, ref_num) {
            // set default values of optional input parameters
            var yPred = yPred == null ? 0 : yPred;
            var ref_num = ref_num == null ? 0 : ref_num;
            // check if input types are of correct type
            checkPushParams(yTrue, yPred, ref_num);
            // calculate the error with provided function from the callback function
            error = this.metric.update(yTrue, yPred);
        }

        /**
        * Returns error value.
        * @ignore
        * @returns {number} Error value.
        */
        this.getError = function () {
            return error;
        }

        /**
        * Save metric state to provided output stream `fout`.
        * @ignore
        * @param {module:fs.FOut} fout - The output stream.
        * @returns {module:fs.FOut} The output stream `fout`.
        */
        this.save = function (fout) {
            fout.writeJson(this.metric.state);
            return fout;
        }

        /**
        * Load metric state from provided input stream `fin`.
        * @ignore
        * @param {module:fs.FIn} fin - The output stream.
        * @returns {module:fs.FIn} The output stream `fin`.
        */
        this.load = function (fin) {
            this.metric.state = fin.readJson();
            error = this.metric.state.error;
            return fin;
        }

    }

    // MEAN ERROR (ME)
    /**
    * Create new (online) mean error instance.
    * @class
    * @classdesc Online Mean Error (ME) instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanError = function (fin) {
        function metric() {
            this.name = "Mean Error"
            this.shortName = "ME"
            this.state = {
                sumErr: 0,
                count: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.state.sumErr += err;
                this.state.count++;
                this.state.error = this.state.sumErr / this.state.count;
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    };

    // MEAN ABSOLUTE ERROR (MAE)
    /**
    * Create new (online) mean absolute error instance.
    * @class
    * @classdesc Online Mean Absolute Error (MAE) instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanAbsoluteError = function (fin) {
        function metric() {
            this.name = "Mean Absolute Error"
            this.shortName = "MAE"
            this.state = {
                sumErr: 0,
                count: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.state.sumErr += Math.abs(err);
                this.state.count++;
                this.state.error = this.state.sumErr / this.state.count;
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    }

    // MEAN SQUARE ERROR (MSE)
    /**
    * Create new (online) mean square error instance.
    * @class
    * @classdesc Online Mean Square Error (MSE) instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanSquareError = function (fin) {
        function metric() {
            this.name = "Mean Square Error"
            this.shortName = "MSE"
            this.state = {
                sumErr: 0,
                count: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.state.sumErr += (err * err);
                this.state.count++;
                this.state.error = this.state.sumErr / this.state.count;
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    }

    // ROOT MEAN SQUARE ERROR (RMSE)
    /**
    * Create new (online) root mean square error instance.
    * @class
    * @classdesc Online Root Mean Square Error (RMSE) instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.RootMeanSquareError = function (fin) {
        function metric() {
            this.name = "Root Mean Square Error"
            this.shortName = "RMSE"
            this.state = {
                sumErr: 0,
                count: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.state.sumErr += (err * err);
                this.state.count++;
                this.state.error = Math.sqrt(this.state.sumErr / this.state.count);
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    }

    // MEAN ABSOLUTE PERCENTAGE ERROR (MAPE)
    /**
    * Create new (online) mean absolute percentage error instance.
    * @class
    * @classdesc Online Mean Absolute Percentage Error (MAPE) instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanAbsolutePercentageError = function (fin) {
        function metric() {
            this.name = "Mean Absolute Percentage Error"
            this.shortName = "MAPE"
            this.state = {
                sumErr: 0,
                count: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                if (yTrue != 0) { // skip if yTrue is 0, otherwise we have devision by zero in the next step.
                    var err = yTrue - yPred;
                    this.state.sumErr += Math.abs(err / yTrue) * 100;
                }
                this.state.count++;
                this.state.error = this.state.sumErr / this.state.count;
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    }

    // R SQUARED SCORE (R2)
    /**
    * Create new (online) R Square instance. This statistic measures how successful the fit is in explaining the variation of the data. Best possible score is 1.0, lower values are worse.
    * @class
    * @classdesc Online R Squared (R2) score instance.
    * @param {module:fs.FIn} [fin] - Saved state can be loaded via constructor.
    * @extends module:analytics~createOnlineMetric
    */
    metrics.R2Score = function (fin) {
        function metric() {
            this.name = "R2 Score"
            this.shortName = "R2"
            this.state = {
                sst: 0,
                sse: 0,
                mean: 0,
                count: 0,
                sumTrue: 0,
                sumTrue2: 0,
                error: 0
            }
            // update function
            this.update = function (yTrue, yPred) {
                this.state.count++;
                this.state.sumTrue += yTrue;
                this.state.sumTrue2 += yTrue * yTrue;
                this.state.mean = this.state.sumTrue / this.state.count;
                //calculate R squared score
                this.state.sse += (yTrue - yPred) * (yTrue - yPred);
                this.state.sst = this.state.sumTrue2 - this.state.count * this.state.mean * this.state.mean;
                if (this.state.sst == 0.0) {
                    return (this.state.sse == 0.0) ? 1.0 : 0.0;
                }
                this.state.error = 1 - this.state.sse / this.state.sst;
                return this.state.error;
            }
        }
        // create new metric instance, and load state from fin in defined
        var errorMetric = new createOnlineMetric(metric);
        if (typeof fin !== 'undefined') errorMetric.load(fin);

        return errorMetric;
    }


    //////////////////////////////////////////////////
    //////////// BATCH REGRESSION METRICS ////////////
    //////////////////////////////////////////////////

    // function checks if input parameters are of appropriate type
    function checkBatchParams() {
        for (var i = 0, j = arguments.length; i < j; i++) {
            var argumentType = arguments[i].constructor.name;
            if (argumentType !== "Array" && argumentType !== "Vector") {
                throw new TypeError('input param ' + i + ' must be of type "Array" or "Vector", but is ' + argumentType + ' instead');
            }
        }
    }

    // calculate batch regression metrics
    function calcBatchError(yTrueVec, yPredVec) {
        // check input parameters
        checkBatchParams(yTrueVec, yPredVec);
        // calculate error with metric defined as callback functio
        function calcErr(metric) {
            // iterage over array of input data
            for (var i = 0; i < yTrueVec.length; i++) {
                metric.push(yTrueVec[i], yPredVec[i]);
            }
            // return final error
            return metric.getError()
        }

        // expose metrics which will be used in calcErr() to return error
        this.ME = function () { return calcErr(new metrics.MeanError()) };
        this.MAE = function () { return calcErr(new metrics.MeanAbsoluteError()) };
        this.MSE = function () { return calcErr(new metrics.MeanSquareError()) };
        this.RMSE = function () { return calcErr(new metrics.RootMeanSquareError()) };
        this.MAPE = function () { return calcErr(new metrics.MeanAbsolutePercentageError()) };
        this.R2 = function () { return calcErr(new metrics.R2Score()) };
    };

    /**
    * Mean error (ME) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`.
    * @returns {number} Error value.
    */
    metrics.meanError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).ME()
    }

    /**
    * Mean absolute error (MAE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`.
    * @returns {number} Error value.
    */
    metrics.meanAbsoluteError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MAE()
    }

    /**
    * Mean square error (MSE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`.
    * @returns {number} Error value.
    */
    metrics.meanSquareError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MSE()
    }

    /**
    * Root mean square (RMSE) error regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`.
    * @returns {number} Error value.
    */
    metrics.rootMeanSquareError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).RMSE()
    }

    /**
    * Mean absolute percentage error (MAPE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec.`
    * @returns {number} Error value.
    */
    metrics.meanAbsolutePercentageError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MAPE()
    }

    /**
    * R^2 (coefficient of determination) regression score.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`.
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`.
    * @returns {number} Error value.
    */
    metrics.r2Score = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).R2()
    }

    // Exports metrics namespace
    exports.metrics = metrics;

    /**
    * @typedef {Object} PCAParam
    * An object used for the construction of {@link module:analytics.PCA}.
    * @property {number} [k = null] - Number of eigenvectors to be computed.
    * @property {number} [iter = 100] - Number of iterations.
    */

    /**
    * Principal Components Analysis
    * @class
    * @classdesc Principal Components Analysis
    * @param {module:analytics~PCAParam | module:fs.FIn} [arg] - Construction arguments. There are two ways of constructing:
    * <br>1. Using the {@link module:analytics~PCAParam} object,
    * <br>2. using the file input stream {@link module:fs.FIn}.
    * @example <caption>Using default constructor</caption>
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // construct model
    * var pca = new analytics.PCA();
    *
    * @example <caption>Using custom constructor</caption>
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // construct model
    * var pca = new analytics.PCA({ k: 5, iter: 50 });
    */
    exports.PCA = function (arg) {
        var iter, k;
        var initParam;
        this.P = undefined;
        this.mu = undefined;
        this.lambda = undefined;
        var count = 1;
        if (arg != undefined && arg.constructor.name == 'FIn') {
            this.P = new la.Matrix();
            this.P.load(arg);
            this.mu = new la.Vector();
            this.mu.load(arg);
            this.lambda = new la.Vector();
            this.lambda.load(arg);
            var params_vec = new la.Vector();
            params_vec.load(arg);
            iter = params_vec[0];
            k = params_vec[1];
        } else if (arg == undefined || typeof arg == 'object') {
            arg = arg == undefined ? {} : arg;
            // Fit params
            var iter = arg.iter == undefined ? 100 : arg.iter;
            var k = arg.k; // can be undefined
        } else {
            throw "PCA.constructor: parameter must be a JSON object or a fs.FIn!";
        }
        initParam = { iter: iter, k: k };
        /**
        * Returns the model.
        * @returns {Object} The object `pcaModel` containing the properties:
        * <br>1. `pcaModel.P` - The eigenvectors. Type {@link module:la.Matrix}.
        * <br>2. `pcaModel.lambda` - The eigenvalues. Type {@link module:la.Vector}.
        * <br>3. `pcaModel.mu` - The mean values. Type {@link module:la.Vector}.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit matrix before getting the model
        * pca.fit(matrix)
        * // get your model using function getModel
        * var model = pca.getModel();
        */
        this.getModel = function () {
            return { P: this.P, mu: this.mu, lambda: this.lambda };
        }

        /**
        * Saves the model.
        * @param {module:fs.FOut} fout - The output stream.
        * @returns {module:fs.FOut} The output stream `fout`.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit matrix
        * pca.fit(matrix);
        * var model = pca.getModel();
        * // save model
        * pca.save(require('qminer').fs.openWrite('pca_test.bin')).close();
        */
        this.save = function (fout) {
            if (!this.P) {
                throw new Error("PCA.save() - model not created yet");
            }

            var params_vec = new la.Vector();
            params_vec.push(iter);
            params_vec.push(k);

            if (fout.constructor.name == 'FOut') {
                this.P.save(fout);
                this.mu.save(fout);
                this.lambda.save(fout);
                params_vec.save(fout);
                return fout;
            } else {
                throw "PCA.save: input must be fs.FOut";
            }
        }


        /**
        * Sets parameters.
        * @param {module:analytics~PCAParam} param - The constructor parameters.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // set 5 eigenvectors and 10 iterations using setParams
        * pca.setParams({iter: 10, k: 5});
        */
        this.setParams = function (param) {
            iter = param.iter == undefined ? iter : param.iter;
            k = param.k == undefined ? k : param.k;
            initParam = { iter: iter, k: k };
            return this;
        }

        /**
        * Gets parameters.
        * @returns {moduel:analytics~PCAParam} The constructor parameters.
        * @example <caption>Using default constructor</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // check the constructor parameters
        * var paramvalue = pca.getParams();
        *
        * @example <caption>Using custom constructor</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // set parameters
        * pca.setParams({iter: 10, k: 5});
        * // check the changed parameters
        * var paramvalue = pca.getParams();
        */
        this.getParams = function () {
            return initParam;
        }

        /**
        * Finds the eigenvectors of the variance matrix.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to examples.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit the matrix
        * pca.fit(matrix);
        */
        this.fit = function (A) {
            var rows = A.rows;
            var cols = A.cols;

            k = k == undefined ? rows : k;
            //iter = iter == undefined ? -1 : iter;

            var mu = stat.mean(A, 2);
            // cov(A) = 1/(n-1) A A' - mu mu'

            // center data (same as matlab)
            var cA = A.minus(mu.outer(la.ones(cols)));
            var C = cA.multiply(cA.transpose()).multiply(1 / (cols - 1));
            // alternative computation:
            //var C = (A.multiply(A.transpose()).multiply(1 / (cols - 1))).minus(mu.outer(mu));
            var res = la.svd(C, k, { iter: iter });

            this.P = res.U;
            this.lambda = res.s;
            this.mu = mu;

            return this;
        }

        /**
        * Projects the example(s) and expresses them as coefficients in the eigenvector basis `this.P`.
        * Recovering the data in the original space: `(this.P).multiply(p)`, where `p`'s rows are the coefficients
        * in the eigenvector basis.
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples.
        * @returns {(module:la.Vector | module:la.Matrix)} Returns projected vector or matrix.
        * @example <caption>Transforming the matrix</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit the matrix
        * pca.fit(matrix);
        * var model = pca.getModel();
        * // transform matrix
        * var transform = pca.transform(matrix);
        *
        * @example <caption>Transforming the vector</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create vector you wish to transform
        * var vector = new la.Vector([0, -1]);
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit the matrix
        * pca.fit(matrix);
        * var model = pca.getModel();
        * // transform vector
        * var transform = pca.transform(vector);
        */
        this.transform = function (x) {
            if (x.constructor.name == 'Matrix') {
                // P * (x - mu*ones(1, size(x,2)))
                return this.P.multiplyT(x.minus(this.mu.outer(la.ones(x.cols))));

            } else if (x.constructor.name == 'Vector') {
                // P * (x - mu)
                return this.P.multiplyT(x.minus(this.mu));
            }
        }

        /**
        * Reconstructs the vector in the original space, reverses centering.
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples, in the PCA space.
        * @returns {(module:la.Vector | module:la.Matrix)} Returns the reconstruction.
        * @example <caption>Inverse transform of matrix</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit the matrix
        * pca.fit(matrix);
        * var model = pca.getModel();
        * // use inverseTransform on matrix
        * var invTransform = pca.inverseTransform(matrix);
        *
        * @example <caption>Inverse transform of vector</caption>
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // construct model
        * var pca = new analytics.PCA();
        * // create vector
        * var vector = new la.Vector([0, -1]);
        * // create matrix
        * var matrix = new la.Matrix([[0, 1], [-1, 0]]);
        * // fit the matrix
        * pca.fit(matrix);
        * var model = pca.getModel();
        * // use inverseTransform on vector
        * var invTransform = pca.inverseTransform(vector);
        */
        this.inverseTransform = function (x) {
            if (x.constructor.name == 'Matrix') {
                // P x + mu*ones(1, size(x,2)
                return (this.P.multiply(x)).plus(this.mu.outer(la.ones(x.cols)));
            } else if (x.constructor.name == 'Vector') {
                // P x + mu
                return (this.P.multiply(x)).plus(this.mu);
            }
        }
    }



    /**
     * @typedef {Object} KMeansExplain
     * The examplanation returned by {@link module:analytics.KMeans#explain}.
     * @property {number} medoidID - The ID of the nearest medoids.
     * @property {module:la.IntVector} featureIDs - The IDs of features, sorted by contribution.
     * @property {module:la.Vector} featureContributions - Weights of each feature contribution (sum to 1.0).
     */

    /**
     * Returns the IDs of the nearest medoid for each example.
     * @param {(module:la.Matrix | module:la.SparseMatrix)} X - Matrix whose columns correspond to examples.
     * @returns {Array.<module:analytics~KMeansExplain>} Array containing the KMeans explanantions.
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
    exports.KMeans.prototype.explain = function (X) {

        /**
         * Returns the weights and feature IDs that contributed to the distance between two vectors.
         * @param {(module:la.Vector | module:la.SparseVector)} x - Vector.
         * @param {(module:la.Vector | module:la.SparseVector)} y - Vector.
         * @returns {Object} Feature IDs and feature contributions.
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

        if (this.medoids == undefined) {
            return { medoidIDs: null };
        }
        var params = this.getParams();
        var norC2 = la.square(this.centroids.colNorms());
        var ones_n = la.ones(X.cols).multiply(0.5);
        var ones_k = la.ones(params.k).multiply(0.5);
        var norX2 = la.square(X.colNorms());
        var D = this.centroids.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
        var centroids = la.findMaxIdx(D);
        var medoidIDs = new la.IntVector(centroids);
        assert(this.medoids.length == params.k);
        var result = [];
        for (var i = 0; i < centroids.length; i++) {
            var explanation = featureContrib(X.getCol(i), this.centroids.getCol(centroids[i]));
            result[i] = {
                medoidID: this.medoids[centroids[i]],
                featureIDs: explanation.featureIDs,
                featureContributions: explanation.featureContributions
            }
        }
        return result;
    }

    /**
    * Returns the model.
    * @returns {Object} The `KMeansModel` object containing the properites:
    * <br> 1. `KMeansModel.C` - The {@link module:la.Matrix} or {@link module:la.SparseMatrix} containing the centroids,
    * <br> 2. `KMeansModel.medoids` - The {@link module:la.IntVector} of cluster medoids of the training data,
    * <br> 3. `KMeansModel.idxv` - The {@link module:la.IntVector} of cluster IDs of the training data.
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
    exports.KMeans.prototype.getModel = function () { return { C: this.centroids, medoids: this.medoids, idxv: this.idxv }; }

    /**
     * @typedef {Object} DpMeansExplain
     * The examplanation returned by {@link module:analytics.KMeans#explain}.
     * @property {number} medoidID - The ID of the nearest medoids.
     * @property {module:la.IntVector} featureIDs - The IDs of features, sorted by contribution.
     * @property {module:la.Vector} featureContributions - Weights of each feature contribution (sum to 1.0).
     */

    /**
     * Returns the IDs of the nearest medoid for each example.
     * @param {(module:la.Matrix | module:la.SparseMatrix)} X - Matrix whose columns correspond to examples.
     * @returns {Array.<module:analytics~DpMeansExplain>} Array containing the DpMeans explanantions.
     * @example
     * // import analytics module
     * var analytics = require('qminer').analytics;
     * // import linear algebra module
     * var la = require('qminer').la;
     * // create a new DpMeans object
     * var DpMeans = new analytics.DpMeans({ iter: 1000, k: 3 });
     * // create a matrix to be fitted
     * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
     * // create the model with the matrix X using the column IDs [0,1,2]
     * DpMeans.fit(X, [1234,1142,2355]);
     * // create the matrix of the prediction vectors
     * var test = new la.Matrix([[2, -1, 1], [1, 0, -3]]);
     * // predict/explain - return the closest medoids
     * var explanation = DpMeans.explain(test);
     */
    exports.DpMeans.prototype.explain = function (X) {

        /**
         * Returns the weights and feature IDs that contributed to the distance between two vectors.
         * @param {(module:la.Vector | module:la.SparseVector)} x - Vector.
         * @param {(module:la.Vector | module:la.SparseVector)} y - Vector.
         * @returns {Object} Feature IDs and feature contributions.
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

        if (this.medoids == undefined) {
            return { medoidIDs: null };
        }
        var params = this.getParams();
        var norC2 = la.square(this.centroids.colNorms());
        var ones_n = la.ones(X.cols).multiply(0.5);
        var ones_k = la.ones(this.centroids.cols).multiply(0.5);
        var norX2 = la.square(X.colNorms());
        var D = this.centroids.multiplyT(X).minus(norC2.outer(ones_n)).minus(ones_k.outer(norX2));
        var centroids = la.findMaxIdx(D);
        var medoidIDs = new la.IntVector(centroids);
        var result = [];
        for (var i = 0; i < centroids.length; i++) {
            var explanation = featureContrib(X.getCol(i), this.centroids.getCol(centroids[i]));
            result[i] = {
                medoidID: this.medoids[centroids[i]],
                featureIDs: explanation.featureIDs,
                featureContributions: explanation.featureContributions
            }
        }
        return result;
    }

    /**
    * Returns the model.
    * @returns {Object} The `DpMeansModel` object containing the properites:
    * <br> 1. `DpMeansModel.C` - The {@link module:la.Matrix} or {@link module:la.SparseMatrix} containing the centroids,
    * <br> 2. `DpMeansModel.medoids` - The {@link module:la.IntVector} of cluster medoids of the training data,
    * <br> 3. `DpMeansModel.idxv` - The {@link module:la.IntVector} of cluster IDs of the training data.
    * @example
    * // import modules
    * var analytics = require('qminer').analytics;
    * var la = require('qminer').la;
    * // create the KMeans object
    * var dpmeans = new analytics.DpMeans({ iter: 1000 });
    * // create a matrix to be fitted
    * var X = new la.Matrix([[1, -2, -1], [1, 1, -3]]);
    * // create the model
    * dpmeans.fit(X);
    * // get the model
    * var model = dpmeans.getModel();
    */
    exports.DpMeans.prototype.getModel = function () { return { C: this.centroids, medoids: this.medoids, idxv: this.idxv }; }

    function defarg(arg, defaultval) {
        return arg == undefined ? defaultval : arg;
    }

    /**
    * @typedef {Object} ActiveLearnerParam
    * An object used for the construction of {@link module:analytics.ActiveLearner}.
    * @property {Object} [learner] - Learner parameters
    * @property {boolean} [learner.disableAsserts=false] - Disable input asserting
    * @property {module:analytics~SVMParam} [SVC] - Support vector classifier parameters.
    */

    /**
    * @classdesc Active learner. Uses a SVM model and implements an uncertainty measure (distance to the margin)
    * to select which unlabelled example should be labelled next.
    * @class
    * @param {module:analytics~ActiveLearnerParam} [arg] - Construction arguments.
    * @example
    * // load libs
    * var qm = require('qminer');
    * var la = qm.la;
    * // create model
    * var al = new qm.analytics.ActiveLearner();
    *
    * // set data (4 labelled and 2 unlabelled examples)
    * var X = new la.Matrix([
    *     [-2, 1],
    *     [-2, 0],
    *     [-2, -1],
    *     [0, 1],
    *     [-0.9, 0],
    *     [0, -1]
    * ]).transpose(); // column examples
    * al.setX(X);
    *
    * var y = [-1, 0, -1, 1, 0, 1];
    * al.sety(y);
    * // get the array containing 1 index of the unlabelled example
    * // that is the closest to the hyperplane
    * var qidx = al.getQueryIdx(1);
    * console.log(qidx); // 4
    */
    class ActiveLearner {
        constructor(opts) {
            opts = opts || {};
            // SETTINGS
            let settings = this._getDefaultSettings();
            override(settings, opts.settings || {});
            this._settings = settings;

            // STATE
            this._X = opts.X || null;
            this._y = opts.y || new Map();
            this._SVC = new exports.SVC(this._settings.SVC);
        }

        /**
        * Returns default settings
        */
        _getDefaultSettings() {
            return {
                learner: {
                    disableAsserts: false
                },
                SVC: {
                    algorithm: "LIBSVM",
                    c: 1.0,
                    j: 1.0
                }
            };
        }

        /**
        * Asserts if the object is a la matrix
        */
        _assertMatrix(X) {
            if (this._settings.learner.disableAsserts) { return; }
            assert(X instanceof la.Matrix || X instanceof la.SparseMatrix, "X should be a dense or a sparse matrix (qm.la object)");
        }

        /**
        * Asserts if a label and the index are valid, given number of examples `cols`
        */
        _assertLabel(cols, idx, label) {
            if (this._settings.learner.disableAsserts) { return; }
            if (cols == undefined) { throw new Error("Columns not defined"); }
            if (!isFinite(idx) || (idx >= cols) || (idx < 0)) { throw new Error("Label index out of range"); }
            if ((label != -1) && (label != 1)) { throw new Error("Label should be either -1 or 1"); }
        }

        /**
        * Asserts if a Map with labels is valid
        */
        _assertLabelSet(cols, y) {
            if (this._settings.learner.disableAsserts) { return; }
            assert(y instanceof Map, "y should be a map from data instance indices to +1 or -1");
            // assert y indices and values
            for (let pair of y.entries()) {
                this._assertLabel(cols, pair[0], pair[1]);
            }
        }

        /**
        * Transforms an Array of labels to a Map from indices to labels
        */
        _getLabelMapFromArr(_y) {
            assert(_y instanceof Array);
            let y = new Map();
            for (let i = 0; i < _y.length; i++) {
                let lab = _y[i];
                if (this._settings.learner.disableAsserts) {
                    assert(lab === -1 || lab === 1 || lab === 0, "Label must be ither -1, 0 or 1");
                }
                if (lab !== 0) {
                    y.set(i, lab);
                }
            }
            return y;
        }

        /**
        * Returns an array of indices of labelled examples
        */
        _getLabIdxArr(y) {
            return Array.from(y.keys());
        }

        /**
        * Returns an array of label values corresponding to the labelled examples
        */
        _getLabArr(y) {
            return Array.from(y.values());
        }

        /**
        * Returns an array of indices of unlabelled examples
        */
        _getUnlabIdxArr(cols, y) {
            let unlabIdxArr = [];
            for (let idx = 0; idx < cols; idx++) {
                if (!y.has(idx)) {
                    unlabIdxArr.push(idx);
                }
            }
            return unlabIdxArr;
        }

        /**
        * Retrains the SVC model
        */
        _retrain(X, y, SVC) {
            // asert y indices and values
            this._assertMatrix(X);
            let cols = X.cols;
            this._assertLabelSet(cols, y);
            if (y.size == 0) { throw new Error("No labelled information in y"); }

            // get all labelled examples and fit SVM
            let labIdxArr = this._getLabIdxArr(y);
            let trainIdx = new la.IntVector(labIdxArr);
            let Xsub = X.getColSubmatrix(trainIdx);
            let yArr = this._getLabArr(y);
            let yVec = new la.Vector(yArr);
            SVC.fit(Xsub, yVec);
        }

        /**
        * Retrains the SVC model
        */
        retrain() {
            this._retrain(this._X, this._y, this._SVC);
        }

        // return an array of indices (1 element by default)
        _getQueryIdx(X, y, SVC, num) {
            num = (isFinite(num) && num > 0 && Number.isInteger(num)) ? num : 1;
            // use the classifier on unlabelled examples and return
            // get unlabelled indices
            let unlabIdxArr = this._getUnlabIdxArr(X.cols, y);
            if (unlabIdxArr.length == 0) { return []; } // exhausted
            let unlabIdxVec = new la.IntVector(unlabIdxArr);
            let Xsub = X.getColSubmatrix(unlabIdxVec);
            if (SVC.weights.length == 0) {
                this._retrain(X, y, SVC);
            }
            // get examples with largest uncertainty
            let uncertaintyArr = SVC.decisionFunction(Xsub).toArray().map((x) =>Math.abs(x));
            let u = new la.Vector(uncertaintyArr);
            let su = u.sortPerm(); // sorted in ascending order
            num = Math.min(num, u.length);
            // take `num` unlabelled indices where we are most uncertain
            let subVec = unlabIdxVec.subVec(su.perm.trunc(num));
            return subVec.toArray();
        }

        /**
        * Returns an array of 0 or more example indices sorted by uncertainty (first element is the closest to the hyperplane)
        * @param {number} [num=1] - maximal length of the array
        * @returns {Array<number>} array of unlabelled example indices
        */
        getQueryIdx(num) {
            return this._getQueryIdx(this._X, this._y, this._SVC, num)
        }

        /**
        * Sets the label
        * @param {number} idx - instance index
        * @param {number} label - should be either 1 or -1
        */
        setLabel(idx, label) {
            let cols = this._X.cols;
            this._assertLabel(cols, idx, label);
            this._y.set(idx, label);
        }

        /**
        * Sets the data matrix (column examples)
        * @param {(module:la.Matrix | module:la.SparseMatrix)} X - data matrix (column examples)
        */
        setX(X) {
            this._assertMatrix(X);
            this._X = X
        }

        /**
        * Sets the labels
        * @param {(Array<number> | module.la.Vector | module.la.IntVector | Map)} _y - array (like) object that encodes labels
        *                                                                        (-1, 0 or 1) or a Map from indices to 1 or -1
        */
        sety(_y) {
            let y = _y;
            this._assertMatrix(this._X);
            let cols = this._X.cols;
            if (_y instanceof Array) { y = this._getLabelMapFromArr(_y); }
            if (_y.toArray != undefined) { y = this._getLabelMapFromArr(_y.toArray()); }
            this._assertLabelSet(cols, y);
            this._y = y;
        }

        /**
        * Returns the SVC model
        * @returns {module:analytics.SVC} SVC model
        */
        getSVC() { return this._SVC; }

        /**
        * Returns the data matrix (column examples)
        * @returns {(module:la.Matrix | module:la.SparseMatrix)} data matrix (column examples)
        */
        getX() { return this._X; }

        /**
        * Returns the Map from example indices to labels (-1 or 1)
        * @returns {Map} label map
        */
        gety() { return this._y; }


    }

    exports.ActiveLearner = ActiveLearner;

    //!ENDJSDOC

    return exports;
}

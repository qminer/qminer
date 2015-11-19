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
    exports = qm.analytics;

    var sget = require('sget');
    var assert = require('assert');

    var qm_util = require(__dirname + '/qm_util.js');

    //!STARTJSDOC

    ///////////////////////////////////////////////////
    /////////////   DATA PREPROCESSING   //////////////
    ///////////////////////////////////////////////////

    /**
    * Preprocessing
    * @namespace
    * @desc Preprocessing functions for preparing labels in formats accepted
    * by learning moduls in qm.analytics.
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
    * @param {Array} y - labels
    * @param {(string | number)} positiveLabel - positive label
    * @param {number} [positiveId = 1] - value when matching positive label
    * @param {number} [negativeId = -1] - value when not matching positive label
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create binarizer with 'b' as positive label
    * var binarizer = new analytics.preprocessing.Binarizer('b');
    * // get vector with binarized labels
    * var bins = binarizer.transform(['a','b','a','c']);
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
        * @param {(Array<number> | Array<string> | module:la.Vector | module:la.StrVector)} y - labels
        * @return {modul:la.Vector} binarized vector
        */
        this.transform = function (y) {
            var target = new la.Vector();
            for (var i = 0; i < y.length; i++) {
                target.push(y[i] === this.positiveLabel ? this.positiveId : this.negativeId);
            }
            return target;
        }
    };

    preprocessing.applyModel = function (model, X) {
        var target = new la.Vector();
        for (var i = 0; i < X.cols; i++) {
            target.push(model.decisionFunction(X[i]));
        }
        return target;
    }

    // Exports preprocessing namespace
    exports.preprocessing = preprocessing;

    /**
    * SVM model.
    * @typedef {Object} svmModel
    * @property  {module:la.Vector} [svmModel.weigths] - SVM normal vector.
    */
    /**
	* Get the model.
	* @returns {module:analytics~svmModel} The current SVM model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a SVC model
    * var SVC = new analytics.SVC();
    * // get the properties of the model
    * var model = SVC.getModel(); // returns { weight: new require('qminer').la.Vector(); }
	*/
    exports.SVC.prototype.getModel = function() { return { weights: this.weights }; }
    /**
	* Get the model.
	* @returns {module:analytics~svmModel} Get current SVM model
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a SVR model
    * var SVR = new analytics.SVR();
    * // get the properties of the model
    * var model = SVR.getModel(); // returns { weights: new require('qminer').la.Vector(); }
	*/
    exports.SVR.prototype.getModel = function() { return { weights: this.weights }; }

    // Ridge Regression
    /**
    * @typedef {Object} ridgeRegModel
    * @property {module:la.Vector} [ridgeRegModel.weights] - The Ridge Regression model vector.
    */

    /**
    * Gets the model.
    * @returns {module:analytics~ridgeRegModel} Get current model.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create the Ridge Regression model
    * var regmod = new analytics.RidgeReg();
    * // get the model
    * var model = regmod.getModel(); // returns { weights: new require('qminer').la.Vector(); }
    */
    exports.RidgeReg.prototype.getModel = function () { return { weights: this.weights }; }

    // Recursive Linear Regression
    /**
    * @typedef {Object} recLinRegModel
    * @property {module:la.Vector} [recLinRegModel.weights] - Recursive Linear Regression model vector.
    */
    /**
    * Gets Recursive Linear Regression model
    * @returns {module:analytics~recLnRegModel} The current model.
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
    * The parameter given to the OneVsAll object. A Json object containing the parameter keys with values.
    * @param {function} [model] - Constructor for binary model to be
    * used internaly. Constructor should expect only one parameter.
    * @param {Object} [modelParam] - Parameter for oneVsAllParam.model constructor.
    * @param {number} [categories] - Number of categories.
    * @param {boolean} [verbose = false] - If false, the console output is supressed.
    */

    /**
    * @classdesc One vs. all model for multiclass prediction. Builds binary model
    * for each category and predicts the one with the highest score. Binary model is
    * provided as part of the constructor.
    * @class
    * @param {module:analytics~oneVsAllParam} [oneVsAllParam] - Constructor parameters.
    * @example
    * // import analytics module
    * var analytics = require('qminer').analytics;
    * // create a new OneVsAll object with the model analytics.SVC
    * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
    */
    exports.OneVsAll = function (oneVsAllParam) {
        // remember parameters
        var model = oneVsAllParam.model;
        var modelParam = oneVsAllParam.modelParam;
        var cats = oneVsAllParam.cats;
        var verbose = oneVsAllParam.verbose == undefined ? false : oneVsAllParam.verbose;
        // trained models
        var models = [ ];

        /**
        * Gets the parameters.
        * @returns {Object} Json object containing the parameters.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
        * // get the parameters
        * // returns the JSon object
        * // { model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2, models: [] }
        * var params = onevsall.getParams();
        */
        this.getParams = function () {
            return { model: model, modelParam: modelParam, cats: cats, models: models }
        };

        /**
        * Sets the parameters.
        * @returns {module:analytics.OneVsAll} Self. The parameters are changed.
        * @example
        * // import analytics module
        * var analytics = require('qminer').analytics;
        * // create a new OneVsAll object with the model analytics.SVC
        * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
        * // set the parameters
        * var params = onevsall.setParams({ model: analytics.SVR, modelParam: { c: 12, maxTime: 10000}, cats: 3, verbose: true });
        */
        this.setParams = function (oneVsAllParam) {
            model = oneVsAllParam.model == undefined ? model : oneVsAllParam.model;
            modelParam = oneVsAllParam.modelParam == undefined ? modelParam : oneVsAllParam.modelParam;
            cats = oneVsAllParam.cats == undefined ? cats : oneVsAllParam.cats;
            verbose = oneVsAllParam.verbose == undefined ? verbose : oneVsAllParam.verbose;
        }

        /**
         * Apply all models to the given vector and returns a vector of scores, one for each category.
         * Semantic of scores depand on the provided binary model.
         * @param {module:la.Vector | module:la.SparseVector | module:la.Matrix | module:la.SparseMatrix} X -
         * Input feature vector or matrix with feature vectors as columns.
         * @returns {module:la.Vector | module:la.Matrix} The score and label of the input:
         * <br>1. {@link module:la.Vector} of scores, if X is of type {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.Matrix} with columns corresponding to instances, and rows corresponding to labels, if X is of type {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
         * // create the data (matrix and vector) used to fit the model
         * var matrix = new la.Matrix([[1, 2, 1, 1], [2, 1, -3, -4]]);
         * var vector = new la.Vector([0, 0, 1, 1]);
         * // fit the model
         * onevsall.fit(matrix, vector);
         * // create the vector for the decisionFunction
         * var test = new la.Vector([1, 2]);
         * // give the vector to the decision function
         * var prediction = onevsall.predict(test); // returns the vector of scores
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
         * Input feature vector or matrix with feature vectors as columns.
         * @returns {number | module:la.IntVector} Returns:
         * <br>1. number of the category with the higher score, if X is {@link module:la.Vector} or {@link module:la.SparseVector}.
         * <br>2. {@link module:la.IntVector} of categories with the higher score for each column of X, if X is {@link module:la.Matrix} or {@link module:la.SparseMatrix}.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
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
         * be integer numbers between 0 and oneVsAllParam.categories - 1.
         * @returns {module:analytics.OneVsAll} Self. The models are now fitted.
         * @example
         * // import modules
         * var analytics = require('qminer').analytics;
         * var la = require('qminer').la;
         * // create a new OneVsAll object with the model analytics.SVC
         * var onevsall = new analytics.OneVsAll({ model: analytics.SVC, modelParam: { c: 10, maxTime: 12000 }, cats: 2 });
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
    * @desc Classification and regression metrics
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
    * @classdesc Class implements several classification measures (precision, recall, F1, accuracy)
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lable(s)
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lable(s)
    */
    metrics.ClassificationScore = function (yTrue, yPred) {
        /**
        * Returns `Object` containing different classification measures
        * @returns {Object} scores - Object with different classification socres
        * @returns {number} scores.count - Count
        * @returns {number} scores.TP - Number of true positives
        * @returns {number} scores.TN - Number of true negative
        * @returns {number} scores.FP - Number of false positives
        * @returns {number} scores.FN - Number of false positives
        * @returns {number} scores.all - Number of all results
        * @returns {number} scores.accuracy - Accuracy score. Formula: (tp + tn) / (tp + fp + fn + tn)
        * @returns {number} scores.precision - Precision score. Formula: tp / (tp + fp)
        * @returns {number} scores.recall - Recall score. Formula: tp / (tp + fn)
        * @returns {number} scores.f1 - F1 score. Formula:  2 * (precision * recall) / (precision + recall)
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
        * Adds prediction to the current statistics. Labels can be either integers
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
    * Formula: (tp + tn) / (tp + fp + fn + tn).
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables
    * @returns {number} Accuracy value
    */
    metrics.accuracyScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.accuracy();
    };

    /**
    * Precision score is defined as the proportion of the true positives against all the
    * positive results (both true positives and false positives).
    * Formula: tp / (tp + fp).
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables
    * @returns {number} Precission score
    */
    metrics.precisionScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.precision();
    };

    /**
    * Recall score is intuitively the ability of the classifier to find all the positive samples.
    * Formula: tp / (tp + fn).
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables
    * @returns {number} Recall score
    */
    metrics.recallScore = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.recall();
    };

    /**
    * The F1 score can be interpreted as a weighted average of the precision and recall, where
    * an F1 score reaches its best value at 1 and worst score at 0. The relative contribution of
    * precision and recall to the F1 score are equal.
    * Formula: 2 * (precision * recall) / (precision + recall)
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Predicted (estimated) lables
    * @returns {number} F1 score
    */
    metrics.f1Score = function (yTrue, yPred) {
        return new metrics.ClassificationScore(yTrue, yPred).scores.f1();
    };

    /**
    * Class implements several prediction curve measures (ROC, AOC, Precision-Recall, ...)
    * @class
    * @classdesc used for computing ROC curve and other related measures such as AUC
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lable(s) of binary classification in range {-1, 1} or {0, 1}.
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
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
        * Count of all examples
        * @name module:analytics~metrics.PredictionCurve#length
        * @type number
        */
        this.length = 0;
        /**
        * Count of all positive examples
        * @name module:analytics~metrics.PredictionCurve#allPositives
        * @type number
        */
        this.allPositives = 0;
        /**
        * Count of all negative examples
        * @name module:analytics~metrics.PredictionCurve#allNegatives
        * @type number
        */
        this.allNegatives = 0;
        // store of predictions and ground truths
        /**
        * Store of ground truths
        * @name module:analytics~metrics.PredictionCurve#grounds
        * @type module:la.Vector
        */
        this.grounds = new la.Vector();
        /**
        * Store of predictions
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
        * Get  Receiver Operating Characteristic (ROC) parametrization sampled on `sample` points
        * @param {number} [sample=10] - Desired number of samples in output
        * @returns {module:la.Matrix} A matrix with increasing false and true positive rates
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
        * Get Area Under the Curve (AUC) of the current curve
        * @param {number} [sample=10] - Desired number of samples in output
        * @returns {number} Area under ROC curve
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
        * evalPrecisionRecall
        * @private
        * @param {callback} callback
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
        * Get precision recall curve sampled on `sample` points
        * @param {number} [sample=10] - Desired number of samples in output
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
        * Get break-even point, the value where precision and recall intersect
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
        * Gets threshold for prediction score, which results in the highest F1
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
        * Gets threshold for prediction score, nearest to specified recall
        * @param {number} desiredRecall - Desired recall score.
        * @returns {number} recal score threshold - Threshold for recall score, nearest to specified `recall`
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
        * Gets threshold for prediction score, nearest to specified precision
        * @param {number} desiredPrecision - Desired precision score.
        * @returns {number} Threshold for prediction score, nearest to specified `precision`
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
    * Get ROC parametrization sampled on `sample` points
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @param {number} [sample=10] - Desired number of samples in output
    * @returns {module:la.Matrix} A matrix with increasing false and true positive rates
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
    * Get AUC of the current curve
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @param {number} [sample=10] - Desired number of samples in output
    * @returns {number} Area under ROC curve
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
    * Get precision recall curve sampled on `sample` points
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @param {number} [sample=10] - Desired number of samples in output
    * @returns {module:la.Matrix} Precision-recall pairs
    */
    metrics.precisionRecallCurve = function (yTrue, yPred, sample) {
        return new metrics.PredictionCurve(yTrue, yPred).precisionRecallCurve(sample);
    };

    /**
    * Get break-even point, the value where precision and recall intersect
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @returns {number} Break-even point score
    */
    metrics.breakEventPointScore = function (yTrue, yPred) {
        return new metrics.PredictionCurve(yTrue, yPred).breakEvenPoint();
    };

    /**
    * Gets threshold for prediction score, which results in the highest F1
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @returns {number} Threshold with highest F1 score
    */
    metrics.bestF1Threshold = function (yTrue, yPred) {
        return new metrics.PredictionCurve(yTrue, yPred).bestF1();
    };

    /**
    * Gets threshold for recall score, nearest to specified recall
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @param {number} desiredRecall - Desired recall score.
    * @returns {number} Threshold for recall score, nearest to specified `recall`
    */
    metrics.desiredRecallThreshold = function (yTrue, yPred, desiredRecall) {
        return new metrics.PredictionCurve(yTrue, yPred).desiredRecall(desiredRecall);
    };

    /**
    * Gets threshold for prediction score, nearest to specified precision
    * @param {(Array<number> | module:la.Vector)} yTrue - Ground truth (correct) lables
    * @param {(Array<number> | module:la.Vector)} yPred - Estimated probabilities
    * @param {number} desiredPrecision - Desired precision score.
    * @returns {number} Threshold for prediction score, nearest to specified `precision`
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
    * @private
    * @class
    *
    * This provides methods used for event handling. It's not meant to
    * be used directly.
    *
    */
    function createOnlineMetric (updateCallback) {
        var error = -1;
        var calcError = new updateCallback();

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
        * @param {number} yTrue - Ground truth (correct) target value
        * @param {number} yPred - Estimated target value
        */
        this.push = function (yTrue, yPred, ref_num) {
            // set default values of optional input parameters
            var yPred = yPred == null ? 0 : yPred;
            var ref_num = ref_num == null ? 0 : ref_num;
            // check if input types are of correct type
            checkPushParams(yTrue, yPred, ref_num);
            // calculate the error with provided function from the callback function
            error = calcError.update(yTrue, yPred);
        }

        /**
        * Returns error value.
        * @returns {number} Error value
        */
        this.getError = function () {
            return error;
        }
    }

    // MEAN ERROR (ME)
    /**
    * Create new (online) mean error instance.
    * @class
    * @classdesc Online Mean Error (ME) instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanError = function () {
        function calcError() {
            this.sumErr = 0;
            this.count = 0;
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.sumErr += err;
                this.count++;
                var error = this.sumErr / this.count;
                return error;
            }
        }
        return new createOnlineMetric(calcError);
    };

    // MEAN ABSOLUTE ERROR (MAE)
    /**
    * Create new (online) mean absolute error instance.
    * @class
    * @classdesc Online Mean Absolute Error (MAE) instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanAbsoluteError = function () {
        function calcError() {
            this.sumErr = 0;
            this.count = 0;
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.sumErr += Math.abs(err);
                this.count++;
                var error = this.sumErr / this.count;
                return error;
            }
        }
        return new createOnlineMetric(calcError);
    }

    // MEAN SQUARE ERROR (MSE)
    /**
    * Create new (online) mean square error instance.
    * @class
    * @classdesc Online Mean Square Error (MSE) instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanSquareError = function () {
        function calcError() {
            this.sumErr = 0;
            this.count = 0;
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.sumErr += (err * err);
                this.count++;
                var error = this.sumErr / this.count;
                return error;
            }
        }
        return new createOnlineMetric(calcError);
    }

    // ROOT MEAN SQUARE ERROR (RMSE)
    /**
    * Create new (online) root mean square error instance.
    * @class
    * @classdesc Online Root Mean Square Error (RMSE) instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.RootMeanSquareError = function () {
        function calcError() {
            this.sumErr = 0;
            this.count = 0;
            // update function
            this.update = function (yTrue, yPred) {
                var err = yTrue - yPred;
                this.sumErr += (err * err);
                this.count++;
                var error = this.sumErr / this.count;
                return Math.sqrt(error);
            }
        }
        return new createOnlineMetric(calcError);
    }

    // MEAN ABSOLUTE PERCENTAGE ERROR (MAPE)
    /**
    * Create new (online) mean absolute percentage error instance.
    * @class
    * @classdesc Online Mean Absolute Percentage Error (MAPE) instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.MeanAbsolutePercentageError = function () {
        function calcError() {
            this.sumErr = 0;
            this.count = 0;
            // update function
            this.update = function (yTrue, yPred) {
                if (yTrue != 0) { // skip if yTrue is 0, otherwise we have devision by zero in the next step.
                    var err = yTrue - yPred;
                    this.sumErr += Math.abs(err / yTrue) * 100;
                }
                this.count++;
                var error = this.sumErr / this.count;
                return error;
            }
        }
        return new createOnlineMetric(calcError);
    }

    // R SQUARED SCORE (R2)
    /**
    * Create new (online) R Square instance. This statistic measures how successful the fit is in explaining the variation of the data. Best possible score is 1.0, lower values are worse.
    * @class
    * @classdesc Online R Squared (R2) score instance
    * @extends module:analytics~createOnlineMetric
    */
    metrics.R2Score = function () {
        function calcError() {
            this.sst = 0;
            this.sse = 0;
            this.mean = 0;
            this.count = 0;
            this.sumTrue = 0;
            this.sumTrue2 = 0;
            // update function
            this.update = function (yTrue, yPred) {
                this.count++;
                this.sumTrue += yTrue;
                this.sumTrue2 += yTrue * yTrue;
                this.mean = this.sumTrue / this.count;
                //calculate R squared score
                this.sse += (yTrue - yPred) * (yTrue - yPred);
                this.sst = this.sumTrue2 - this.count * this.mean * this.mean;
                if (this.sst == 0.0) {
                    return (this.sse == 0.0) ? 1.0 : 0.0;
                }
                return 1 - this.sse / this.sst;
            }
        }
        return new createOnlineMetric(calcError);
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
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.meanError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).ME()
    }

    /**
    * Mean absolute error (MAE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.meanAbsoluteError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MAE()
    }

    /**
    * Mean square error (MSE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.meanSquareError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MSE()
    }

    /**
    * Root mean square (RMSE) error regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.rootMeanSquareError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).RMSE()
    }

    /**
    * Mean absolute percentage error (MAPE) regression loss.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.meanAbsolutePercentageError = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).MAPE()
    }

    /**
    * R^2 (coefficient of determination) regression score.
    * @param {(Array<number> | module:la.Vector)} yTrueVec - ground truth values in `yTrueVec`
    * @param {(Array<number> | module:la.Vector)} yPredVec - estimated values in `yPredVec`
    * @returns {number} Error value
    */
    metrics.r2Score = function (yTrueVec, yPredVec) {
        return new calcBatchError(yTrueVec, yPredVec).R2()
    }

    // Exports metrics namespace
    exports.metrics = metrics;

    /**
    * @classdesc Principal components analysis
    * @class
    */
    exports.PCA = function (param) {
        param = param == undefined ? {} : param;

        // Fit params
        var iter = param.iter == undefined ? 100 : param.iter;
        var k = param.k; // can be undefined

        /**
        * Returns the model
        * @returns {Object} The model object whose keys are: P (eigenvectors), lambda (eigenvalues) and mu (mean)
        */
        this.getModel = function () {
            return { P: this.P, mu: this.mu, lambda: this.lambda };
        }

        /**
        * Sets parameters
        * @param {p} Object whose keys are: k (number of eigenvectors) and iter (maximum iterations)
        */
        this.setParams = function (p) {
            param = p;

            iter = param.iter == undefined ? iter : param.iter;
            k = param.k == undefined ? k : param.iter;
        }

        /**
        * Gets parameters
        * @returns Object whose keys are: k (number of eigenvectors) and iter (maximum iterations)
        */
        this.getParams = function () {
            return param;
        }

        /**
        * Finds the eigenvectors of the variance matrix.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to examples.
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
        }

        /**
        * Projects the example(s) and expresses them as coefficients in the eigenvector basis this.P.
        * Recovering the data in the original space: (this.P).multiply(p), where p's rows are the coefficients
        * in the eigenvector basis.
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples
        * @returns {(module:la.Vector | module:la.Matrix)} Returns projected vector or matrix
        */
        this.transform = function (x) {
            if (x.constructor.name == 'Matrix') {
                // P * (x - mu*ones(1, size(x,2))
                return this.P.multiplyT(x.minus(this.mu.outer(la.ones(x.cols))));

            } else if (x.constructor.name == 'Vector') {
                // P * (x - mu)
                return this.P.multiplyT(x.minus(this.mu));
            }
        }

        /**
        * Reconstructs the vector in the original space, reverses centering
        * @param {(module:la.Vector | module:la.Matrix)} x - Test vector or matrix with column examples, in the PCA space
        * @returns {(module:la.Vector | module:la.Matrix)} Returns the reconstruction
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
            return  { iter: iter, k: k, verbose: verbose }
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

    function defarg(arg, defaultval) {
        return arg == undefined ? defaultval : arg;
    }

    ///////////////////////////////
    ////// code below not yet ported or verified for scikit
    ///////////////////////////////

    //!- `alModel = analytics.newActiveLearner(query, qRecSet, fRecSet, ftrSpace, settings)` -- initializes the
    //!    active learning. The algorihm is run by calling `model.startLoop()`. The algorithm has two stages: query mode, where the algorithm suggests potential
    //!    positive and negative examples based on the query text, and SVM mode, where the algorithm keeps
    //!   selecting examples that are closest to the SVM margin (every time an example is labeled, the SVM
    //!   is retrained.
    //!   The inputs are: query (text), record set `qRecSet`, record set `fRecSet`,  the feature space `ftrSpace` and a
    //!   `settings`JSON object. The settings object specifies:`textField` (string) which is the name
    //!    of the field in records that is used to create feature vectors, `nPos` (integer) and `nNeg` (integer) set the number of positive and negative
    //!    examples that have to be identified in the query mode before the program enters SVM mode.
    //!   We can set two additional parameters `querySampleSize` and `randomSampleSize` which specify the sizes of subsamples of qRecSet and fRecSet, where the rest of the data is ignored in the active learning.
    //!   Final parameters are all SVM parameters (c, j, batchSize, maxIterations, maxTime, minDiff, verbose).
    exports.newActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        return new exports.ActiveLearner(query, qRecSet, fRecSet, ftrSpace, stts);
    }

    exports.ActiveLearner = function (query, qRecSet, fRecSet, ftrSpace, stts) {
        var settings = defarg(stts, {});
        settings.nPos = defarg(stts.nPos, 2);
        settings.nNeg = defarg(stts.nNeg, 2);
        settings.textField = defarg(stts.textField, "Text");
        settings.querySampleSize = defarg(stts.querySampleSize, -1);
        settings.randomSampleSize = defarg(stts.randomSampleSize, -1);
        settings.c = defarg(stts.c, 1.0);
        settings.j = defarg(stts.j, 1.0);
        settings.batchSize = defarg(stts.batchSize, 100);
        settings.maxIterations = defarg(stts.maxIterations, 100000);
        settings.maxTime = defarg(stts.maxTime, 1); // 1 second for computation by default
        settings.minDiff = defarg(stts.minDiff, 1e-6);
        settings.verbose = defarg(stts.verbose, false);

        // compute features or provide them
        settings.extractFeatures = defarg(stts.extractFeatures, true);

        if (!settings.extractFeatures) {
            if (stts.uMat == null) { throw 'settings uMat not provided, extractFeatures = false'; }
            if (stts.uRecSet == null) { throw 'settings uRecSet not provided, extractFeatures = false'; }
            if (stts.querySpVec == null) { throw 'settings querySpVec not provided, extractFeatures = false'; }
        }

        // QUERY MODE
        var queryMode = true;
        // bow similarity between query and training set

        var querySpVec;
        var uRecSet;
        var uMat;

        if (settings.extractFeatures) {
            var temp = {}; temp[settings.textField] = query;
            var queryRec = qRecSet.store.newRecord(temp); // record
            querySpVec = ftrSpace.extractSparseVector(queryRec);
            // use sampling?
            var sq = qRecSet;
            if (settings.querySampleSize >= 0 && qRecSet != undefined) {
                sq = qRecSet.sample(settings.querySampleSize);
            }
            var sf = fRecSet;
            if (settings.randomSampleSize >= 0 && fRecSet != undefined) {
                sf = fRecSet.sample(settings.randomSampleSize);
            }
            // take a union or just qset or just fset if some are undefined
            uRecSet = (sq != undefined) ? ((sf != undefined) ? sq.setunion(sf) : sq) : sf;
            if (uRecSet == undefined) { throw 'undefined record set for active learning!';}
            uMat = ftrSpace.extractSparseMatrix(uRecSet);

        } else {
            querySpVec = stts.querySpVec;
            uRecSet = stts.uRecSet;
            uMat = stts.uMat;
        }


        querySpVec.normalize();
        uMat.normalizeCols();

        var X = new la.SparseMatrix();
        var y = new la.Vector();
        var simV = uMat.multiplyT(querySpVec); //similarities (q, recSet)
        var sortedSimV = simV.sortPerm(); //ascending sort
        var simVs = sortedSimV.vec; //sorted similarities (q, recSet)
        var simVp = sortedSimV.perm; //permutation of sorted similarities (q, recSet)
        //// counters for questions in query mode
        var nPosQ = 0; //for traversing simVp from the end
        var nNegQ = 0; //for traversing simVp from the start


        // SVM MODE
        var svm;
        var posIdxV = new la.IntVector(); //indices in recordSet
        var negIdxV = new la.IntVector(); //indices in recordSet

        var posRecIdV = new la.IntVector(); //record IDs
        var negRecIdV = new la.IntVector(); //record IDs

        var classVec = new la.Vector({ "vals": uRecSet.length }); //svm scores for record set
        var resultVec = new la.Vector({ "vals": uRecSet.length }); // non-absolute svm scores for record set


        //!   - `rs = alModel.getRecSet()` -- returns the record set that is being used (result of sampling)
        this.getRecSet = function () { return uRecSet };

        //!   - `idx = alModel.selectedQuestionIdx()` -- returns the index of the last selected question in alModel.getRecSet()
        this.selectedQuestionIdx = -1;

        //!   - `bool = alModel.getQueryMode()` -- returns true if in query mode, false otherwise (SVM mode)
        this.getQueryMode = function () { return queryMode; };

        //!   - `numArr = alModel.getPos(thresh)` -- given a `threshold` (number) return the indexes of records classified above it as a javascript array of numbers. Must be in SVM mode.
        this.getPos = function (threshold) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!threshold) { threshold = 0; }
            var posIdxArray = [];
            for (var recN = 0; recN < uRecSet.length; recN++) {
                if (resultVec[recN] >= threshold) {
                    posIdxArray.push(recN);
                }
            }
            return posIdxArray;
        };

        this.debug = function () { debugger; }

        this.getTop = function (limit) {
            if (this.queryMode) { return null; } // must be in SVM mode to return results
            if (!limit) { limit = 20; }
            var idxArray = [];
            var marginArray = [];
            var sorted = resultVec.sortPerm(false);
            for (var recN = 0; recN < uRecSet.length && recN < limit; recN++) {
                idxArray.push(sorted.perm[recN]);
                var val = sorted.vec[recN];
                val = val == Number.POSITIVE_INFINITY ? Number.MAX_VALUE : val;
                val = val == Number.NEGATIVE_INFINITY ? -Number.MAX_VALUE : val;
                marginArray.push(val);
            }
            return { posIdx: idxArray, margins: marginArray };
        };

        //!   - `objJSON = alModel.getSettings()` -- returns the settings object
        this.getSettings = function () { return settings; }

        // returns record set index of the unlabeled record that is closest to the margin
        //!   - `recSetIdx = alModel.selectQuestion()` -- returns `recSetIdx` - the index of the record in `recSet`, whose class is unknonw and requires user input
        this.selectQuestion = function () {
            if (posRecIdV.length >= settings.nPos && negRecIdV.length >= settings.nNeg) { queryMode = false; }
            if (queryMode) {
                if (posRecIdV.length < settings.nPos && nPosQ + 1 < uRecSet.length) {
                    nPosQ = nPosQ + 1;
                    console.log("query mode, try to get pos");
                    this.selectedQuestionIdx = simVp[simVp.length - 1 - (nPosQ - 1)];
                    return this.selectedQuestionIdx;
                }
                if (negRecIdV.length < settings.nNeg && nNegQ + 1 < uRecSet.length) {
                    nNegQ = nNegQ + 1;
                    // TODO if nNegQ == rRecSet.length, find a new sample
                    console.log("query mode, try to get neg");
                    this.selectedQuestionIdx = simVp[nNegQ - 1];
                    return this.selectedQuestionIdx;
                }
            }
            else {
                ////call svm, get record closest to the margin
                svm = new exports.SVC(settings);
                svm.fit(X, y);//column examples, y float vector of +1/-1, default svm paramvals

                // mark positives
                for (var i = 0; i < posIdxV.length; i++) {
                    classVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[posIdxV[i]] = Number.POSITIVE_INFINITY;
                }
                // mark negatives
                for (var i = 0; i < negIdxV.length; i++) {
                    classVec[negIdxV[i]] = Number.POSITIVE_INFINITY;
                    resultVec[negIdxV[i]] = Number.NEGATIVE_INFINITY;
                }
                var posCount = posIdxV.length;
                var negCount = negIdxV.length;
                // classify unlabeled
                for (var recN = 0; recN < uRecSet.length; recN++) {
                    if (classVec[recN] !== Number.POSITIVE_INFINITY) {
                        var svmMargin = svm.predict(uMat.getCol(recN));
                        if (svmMargin > 0) {
                            posCount++;
                        } else {
                            negCount++;
                        }
                        classVec[recN] = Math.abs(svmMargin);
                        resultVec[recN] = svmMargin;
                    }
                }
                var sorted = classVec.sortPerm();
                console.log("svm mode, margin: " + sorted.vec[0] + ", npos: " + posCount + ", nneg: " + negCount);
                this.selectedQuestionIdx = sorted.perm[0];
                return this.selectedQuestionIdx;
            }

        };
        // asks the user for class label given a record set index
        //!   - `alModel.getAnswer(ALAnswer, recSetIdx)` -- given user input `ALAnswer` (string) and `recSetIdx` (integer, result of model.selectQuestion) the training set is updated.
        //!      The user input should be either "y" (indicating that recSet[recSetIdx] is a positive example), "n" (negative example).
        this.getAnswer = function (ALanswer, recSetIdx) {
            //todo options: ?newQuery
            if (ALanswer === "y") {
                posIdxV.push(recSetIdx);
                posRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(1.0);
            } else {
                negIdxV.push(recSetIdx);
                negRecIdV.push(uRecSet[recSetIdx].$id);
                //X.push(ftrSpace.extractSparseVector(uRecSet[recSetIdx]));
                X.push(uMat.getCol(recSetIdx));
                y.push(-1.0);
            }
            // +k query // rank unlabeled according to query, ask for k most similar
            // -k query // rank unlabeled according to query, ask for k least similar
        };
        //!   - `alModel.startLoop()` -- starts the active learning loop in console
        this.startLoop = function () {
            while (true) {
                var recSetIdx = this.selectQuestion();
                var ALanswer = sget(uRecSet[recSetIdx].Text + ": y/(n)/s? Command s stops the process").trim();
                if (ALanswer == "s") { break; }
                if (posIdxV.length + negIdxV.length == uRecSet.length) { break; }
                this.getAnswer(ALanswer, recSetIdx);
            }
        };
        //!   - `alModel.saveSvmModel(fout)` -- saves the binary SVM model to an output stream `fout`. The algorithm must be in SVM mode.
        this.saveSvmModel = function (outputStream) {
            // must be in SVM mode
            if (queryMode) {
                console.log("AL.save: Must be in svm mode");
                return;
            }
            svm.save(outputStream);
        };

        this.getWeights = function () {
            return svm.weights;
        }
        //this.saveLabeled
        //this.loadLabeled
    };


	{
	    /**
	     * StreamStory.
	     * @class
	     * @param {opts} HierarchMarkovParam - parameters. TODO typedef and describe
	     */
	    exports.StreamStory = function (opts) {
	    	//===================================================
	    	// CONSTRUCTOR
	    	//===================================================

	    	if (opts == null) throw new Error('Missing parameters!');
	    	if (opts.base == null) throw new Error('Missing parameter base!');

	    	// create model and feature space
	    	var mc;
	    	var base = opts.base;
	    	var obsFtrSpace;
	    	var controlFtrSpace;
	    	var id;
	    	var active = false;
	    	var online = false;

	    	if (opts.base != null && opts.config != null) {
	    		mc = new exports._StreamStory(opts.config);
	    		if (opts.obsFields != null && opts.contrFields != null) {
		    		obsFtrSpace = new qm.FeatureSpace(opts.base, opts.obsFields);
		    		controlFtrSpace = new qm.FeatureSpace(opts.base, opts.contrFields);
	    		}
	    		else if (opts.obsFtrSpace != null && opts.controlFtrSpace != null) {
	    			obsFtrSpace = opts.obsFtrSpace;
	    			controlFtrSpace = opts.controlFtrSpace;
	    		}
	    		else {
	    			throw new Error('Missing feature space configuration!');
	    		}
	    	}
	    	else if (opts.fname != null) {
	    		console.log('Loading StreamStory from: ' + opts.fname);
	    		var fin = new fs.FIn(opts.fname);
	    		mc = new exports._StreamStory(fin);
	    		console.log('Loading feature spaces ...');
	    		obsFtrSpace = new qm.FeatureSpace(base, fin);
	    		controlFtrSpace = new qm.FeatureSpace(base, fin);
	    		console.log('Loaded!');
	    	}
	    	else {
	    		throw new Error('Missing parameters (base and config) or fname!');
	    	}

	    	//===================================================
	    	// FEATURE HELPER FUNCTIONS
	    	//===================================================


	    	function getFtrNames(ftrSpace) {
	    		var names = [];

	    		var dims = ftrSpace.dims;
	    		for (var i = 0; i < dims.length; i++) {
	    			var ftrDesc = ftrSpace.getFeature(i);
	    			var match = ftrDesc.match(/\[\w*\]$/)[0];	// remove Numeric[ ]

	    			if (match != null)
	    				names.push(match.substring(1, match.length-1));
	    			else
	    				names.push(ftrDesc);
				}

	    		return names;
	    	}

	    	function getFtrCount(ftrSpace) {
	    		return ftrSpace.dims.length
	    	}

	    	function getObsFtrCount() {
	    		return getFtrCount(obsFtrSpace);
			}

	    	function getContrFtrCount() {
	    		return getFtrCount(controlFtrSpace);
			}

	    	function getObsFtrNames() {
	    		return getFtrNames(obsFtrSpace);
	    	}

	    	function getControlFtrNames() {
	    		return getFtrNames(controlFtrSpace);
	    	}

	    	function getFtrDescriptions(stateId) {
	    		var observations = [];
	    		var controls = [];

	    		var obsFtrCount = getObsFtrCount();

				var coords = mc.fullCoords(stateId);
				var obsFtrNames = getObsFtrNames();
				var invObsCoords = obsFtrSpace.invertFeatureVector(coords);
				for (var i = 0; i < invObsCoords.length; i++) {
					observations.push({
						name: obsFtrNames[i],
						value: invObsCoords.at(i),
						isControl: false,
						bounds: getFtrBounds(i)
					});
				}

				var controlCoords = mc.fullCoords(stateId, false);
				var contrFtrNames = getControlFtrNames();
				var invControlCoords = controlFtrSpace.invertFeatureVector(controlCoords);
				for (var i = 0; i < invControlCoords.length; i++) {
					controls.push({
						name: contrFtrNames[i],
						value: invControlCoords.at(i),
						isControl: true,
						bounds: getFtrBounds(i + obsFtrCount)
					});
				}

				return {
					observations: observations,
					controls: controls,
					isBottom: mc.isLeaf(stateId)
				};
	    	}

	    	function getFtrCoord(stateId, ftrIdx) {
	    		if (ftrIdx < obsFtrSpace.dims.length) {
	    			return obsFtrSpace.invertFeatureVector(mc.fullCoords(stateId))[ftrIdx];
	    		} else {
	    			return controlFtrSpace.invertFeatureVector(mc.fullCoords(stateId, false))[ftrIdx - obsFtrSpace.dims.length];
	    		}
	    	}

	    	function getFtrBounds(ftrId) {
	    		var obsFtrCount = getObsFtrCount();
	    		var bounds = mc.getFtrBounds(ftrId);

	    		if (ftrId < obsFtrCount) {
	    			return {
	    				min: obsFtrSpace.invertFeature(ftrId, bounds.min),
	    				max: obsFtrSpace.invertFeature(ftrId, bounds.max)
	    			}
	    		} else {
	    			return {
	    				min: controlFtrSpace.invertFeature(ftrId - obsFtrCount, bounds.min),
	    				max: controlFtrSpace.invertFeature(ftrId - obsFtrCount, bounds.max)
	    			}
	    		}
	    	}

	    	//===================================================
	    	// HISTOGRAM
	    	//===================================================


	    	function toServerHistogram(hist, ftrId) {
	    		var nObsFtrs = getObsFtrCount();

    			if (ftrId < nObsFtrs) {
	    			for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = obsFtrSpace.invertFeature(ftrId, hist.binStartV[i]);
	    			}
    			} else {
    				for (var i = 0; i < hist.binStartV.length; i++) {
	    				hist.binStartV[i] = controlFtrSpace.invertFeature(ftrId - nObsFtrs, hist.binStartV[i]);
	    			}
    			}

    			return hist;
	    	}

	    	//===================================================
	    	// PUBLIC METHODS
	    	//===================================================

	    	// public methods
	    	var that = {
	    		getId: function () {
	    			return id;
	    		},

	    		setId: function (modelId) {
	    			id = modelId;
	    		},

	    		isActive: function () {
	    			return active;
	    		},

	    		setActive: function (act) {
	    			active = act;
	    		},

	    		isOnline: function () {
	    			return online;
	    		},

	    		setOnline: function (isOnline) {
	    			online = isOnline;
	    		},

	    		/**
	    		 * Creates a new model out of the record set.
	    		 */
	    		fit: function (opts) {
	    			if (opts.recSet == null && opts.recV == null)
	    				throw new Error('StreamStory.fit: missing parameters recSet or recV');

	    			var batchEndV = opts.batchEndV;
	    			var timeField = opts.timeField;

	    			var obsColMat;
	    			var contrColMat;
	    			var timeV;

	    			if (opts.recV != null) {
	    				var recV = opts.recV;
	    				var nInst = recV.length;

	    				log.info('Updating feature spaces ...');
	    				for (var i = 0; i < nInst; i++) {
	    					var rec = recV[i];
	    					obsFtrSpace.updateRecord(rec);
							controlFtrSpace.updateRecord(rec);
	    				}

	    				obsColMat = new la.Matrix({rows: obsFtrSpace.dim, cols: nInst});
	    				contrColMat = new la.Matrix({rows: controlFtrSpace.dim, cols: nInst});
	    				timeV = new la.Vector({ vals: nInst });

	    				for (var i = 0; i < nInst; i++) {
	    					var rec = recV[i];
	    					var obsFtrV = obsFtrSpace.extractVector(rec);
	    					var contrFtrV = controlFtrSpace.extractVector(rec);
	    					var time = rec[timeField].getTime();

	    					obsColMat.setCol(i, obsFtrV);
	    					contrColMat.setCol(i, contrFtrV);
	    					timeV[i] = time;
	    				}
	    			} else {
	    				var recSet = opts.recSet;

	    				log.info('Updating feature spaces ...');
	    				obsFtrSpace.updateRecords(recSet);
		    			controlFtrSpace.updateRecords(recSet);

		    			obsColMat = obsFtrSpace.extractMatrix(recSet);
		    			contrColMat = controlFtrSpace.extractMatrix(recSet);
		    			timeV = recSet.getVector(timeField);
	    			}

	    			log.info('Creating model ...');
	    			mc.fit({
	    				observations: obsColMat,
	    				controls: contrColMat,
	    				times: timeV,
	    				batchV: batchEndV
	    			});
	    			log.info('Done!');

	    			return that;
	    		},

	    		/**
	    		 * Adds a new record. Doesn't update the models statistics.
	    		 */
	    		update: function (rec) {
	    			if (rec == null) return;

	    			var obsFtrVec = obsFtrSpace.extractVector(rec);
	    			var contFtrVec = controlFtrSpace.extractVector(rec);
	    			var timestamp = rec.time.getTime();

	    			mc.update(obsFtrVec, contFtrVec, timestamp);
	    		},

	    		/**
	    		 * Saves the feature space and model into the specified files.
	    		 */
	    		save: function (mcFName) {
	    			try {
	    				console.log('Saving Markov chain ...');

	    				var fout = new fs.FOut(mcFName);

		    			mc.save(fout);
		    			obsFtrSpace.save(fout);
		    			controlFtrSpace.save(fout);

		    			fout.flush();
		    			fout.close();

		    			console.log('Done!');
	    			} catch (e) {
	    				console.log('Failed to save the model!!' + e.message);
	    			}
	    		},

	    		/**
	    		 * Returns the state used in the visualization.
	    		 */
	    		getVizState: function () {
	    			log.debug('Fetching visualization ...');
	    			return mc.toJSON();
	    		},

	    		/**
	    		 * Returns the hierarchical Markov chain model.
	    		 */
	    		getModel: function () {
	    			return mc;
	    		},

	    		/**
	    		 * Returns the feature space.
	    		 */
	    		getFtrSpace: function () {
	    			return { observations: obsFtrSpace, controls: controlFtrSpace };
	    		},

	    		/**
	    		 * Returns the current state at the specified height. If the height is not specified it
	    		 * returns the current states through the hierarchy.
	    		 */
	    		currState: function (height) {
	    			return mc.currState(height);
	    		},

	    		/**
	    		 * Returns the most likely future states.
	    		 */
	    		futureStates: function (level, state, time) {
	    			return mc.futureStates(level, state, time);
	    		},

	    		/**
	    		 * Returns the most likely future states.
	    		 */
	    		pastStates: function (level, state, time) {
	    			return mc.pastStates(level, state, time);
	    		},

	    		getFtrDesc: function (ftrId) {
	    			var nObsFtrs = getObsFtrCount();

	    			if (ftrId == null) {
	    				var n = nObsFtrs + getContrFtrCount();

	    				var obsFtrs = [];
	        			var contrFtrs = [];

	    				for (var i = 0; i < n; i++) {
	    					var ftrDesc = that.getFtrDesc(i);

	    					if (i < nObsFtrs) {
	    						obsFtrs.push(ftrDesc);
	    					} else {
	    						contrFtrs.push(ftrDesc);
	    					}
	    				}

	    				return {
	        				observation: obsFtrs,
	        				control: contrFtrs
	        			}
	    			}
	    			else {
	    				if (ftrId < nObsFtrs) {
	    					var ftrNames = getObsFtrNames();
	    					return {
	    						name: ftrNames[ftrId],
	    						bounds: getFtrBounds(ftrId)
	    					}
	    				} else {
	    					var ftrNames = getControlFtrNames();
	    					return {
	    						name: ftrNames[ftrId - nObsFtrs],
	    						bounds: getFtrBounds(ftrId)
	    					}
	    				}
	    			}
	    		},

	    		/**
	    		 * Returns state details as a Javascript object.
	    		 */
	    		stateDetails: function (stateId, height) {
	    			var futureStates = mc.futureStates(height, stateId);
	    			var pastStates = mc.pastStates(height, stateId);
	    			var isTarget = mc.isTarget(stateId, height);
	    			var isLeaf = mc.isLeaf(stateId);
	    			var stateNm = mc.getStateName(stateId);
	    			var wgts = mc.getStateWgtV(stateId);

	    			var features = getFtrDescriptions(stateId);

	    			return {
	    				id: stateId,
	    				name: stateNm.length > 0 ? stateNm : null,
	    				isTarget: isTarget,
	    				isLeaf: isLeaf,
	    				features: features,
	    				futureStates: futureStates,
	    				pastStates: pastStates,
	    				featureWeights: wgts
	    			};
	    		},

	    		/**
	    		 * Returns a histogram for the desired feature in the desired state.
	    		 */
	    		histogram: function (stateId, ftrId) {
	    			var hist = mc.histogram(stateId, ftrId);
	    			return toServerHistogram(hist, ftrId);
	    		},

	    		transitionHistogram: function (sourceId, targetId, ftrId) {
	    			var hist = mc.transitionHistogram(sourceId, targetId, ftrId);
	    			return toServerHistogram(hist, ftrId);
	    		},

	    		/**
	    		 * Callback when the current state changes.
	    		 */
	    		onStateChanged: function (callback) {
	    			mc.onStateChanged(callback);
	    		},

	    		/**
	    		 * Callback when an anomaly is detected.
	    		 */
	    		onAnomaly: function (callback) {
	    			mc.onAnomaly(callback);
	    		},

	    		onOutlier: function (callback) {
	    			mc.onOutlier(function (ftrV) {
	    				var invFtrV = obsFtrSpace.invertFeatureVector(ftrV);

	    				var features = [];
	    				for (var i = 0; i < invFtrV.length; i++) {
	    					features.push({name: obsFtrSpace.getFeature(i), value: invFtrV.at(i)});
	    				}

	    				callback(features);
	    			});
	    		},

	    		onPrediction: function (callback) {
	    			mc.onPrediction(callback);
	    		},

	    		/**
	    		 * Returns the distribution of features accross the states on the
	    		 * specified height.
	    		 */
	    		getFtrDist: function (height, ftrIdx) {
	    			var stateIds = mc.stateIds(height);

	    			var result = [];
	    			for (var i = 0; i < stateIds.length; i++) {
	    				var stateId = stateIds[i];
	    				var coord = getFtrCoord(stateId, ftrIdx);
	    				result.push({ state: stateId, value: coord });
	    			}

	    			return result;
	    		},

	    		setControlVal: function (opts) {
	    			if (opts.ftrId == null) throw new Error('Missing parameter ftrId!');
	    			var controlFtrId = opts.ftrId - getObsFtrCount();

	    			var params = {
	    				ftrId: opts.ftrId,
	    				val: controlFtrSpace.extractFeature(controlFtrId, opts.val)
	    			};

	    			if (opts.stateId != null) params.stateId = opts.stateId;

	    			mc.setControlVal(params);
	    		},

	    		resetControlVal: function (opts) {
	    			var params = {};
	    			if (opts.stateId != null) params.stateId = opts.stateId;
	    			if (opts.ftrId != null) params.ftrId = opts.ftrId;

	    			mc.resetControlVal(params);
	    		}
	    	};

	    	return that;
	    };
	}
    //!ENDJSDOC

    return exports;
}

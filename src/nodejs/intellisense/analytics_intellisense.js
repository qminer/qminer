//intellisense start
exports = {}; require.modules.qminer_analytics = exports;
//intellisense end
/**
 * Copyright (c) 2015, Jozef Stefan Institute, Quintelligence d.o.o. and contributors
 * All rights reserved.
 * 
 * This source code is licensed under the FreeBSD license found in the
 * LICENSE file in the root directory of this source tree.
 */
/**
* Analytics module.
* @module analytics
* @example
* // import module
* var analytics = require('qminer').analytics;
* // REGRESSION WITH SVR
* // Set up fake train and test data.
* // Four training examples with, number of features = 2
* var featureMatrix = new la.Matrix({rows:2, cols:4});
* // Regression targets for four examples
* var targets = new la.Vector({vals:4});
* // Set up the regression model
* var SVR = new analytics.SVR({verbose:true});
* // Train regression
* SVR.fit(featureMatrix, targets);
* // Save the model to disk
* SVR.save('svr.bin');*
* // Set up a fake test vector
* var test = new la.Vector({vals:2});
* // Predict the target value
* var prediction = SVR.predict(test);
*/
/**
* SVC constructor parameters
* @typedef {Object} svcParam
* @property  {number} [svcParam.c=1.0] - Cost parameter. Increasing the parameter forces the model to fit the training data more accurately (setting it too large may lead to overfitting) 
* @property  {number} [svcParam.j=1.0] - Unbalance parameter. Increasing it gives more weight to the positive examples (getting a better fit on the positive training examples gets a higher priority). Setting c=n is like adding n-1 copies of the positive training examples to the data set.
* @property  {number} [svcParam.batchSize=10000] - Number of examples used in the subgradient estimation. Higher number of samples slows down the algorithm, but makes the local steps more accurate
* @property  {number} [svcParam.maxIterations=10000] - Maximum number of iterations
* @property  {number} [svcParam.maxTime=1.0] - Maximum runtime in seconds
* @property  {number} [svcParam.minDiff=1e-6] - Stopping criterion tolerance
* @property  {boolean} [svcParam.verbose=false] - Toggle verbose output in the console
*/
/**
* SVC
* @classdesc Support Vector Machine Classifier. Implements a soft margin linear support vector classifier using the PEGASOS algorithm, see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:fs.FIn | module:analytics~svcParam} arg - File input stream (loads the model from disk) or constructor parameters svcParam.
* @example
* // import module
* var analytics = require('qminer').analytics;
* // CLASSIFICATION WITH SVC
* // Set up fake train and test data.
* // Four training examples with, number of features = 2
* var featureMatrix = new la.Matrix({rows:2, cols:4, random:true});
* // classification targets for four examples
* var targets = new la.Vector([-1, -1, 1, 1]);
* // Set up the classification model
* var SVC = new analytics.SVC({verbose:true});
* // Train classifier
* SVC.fit(featureMatrix, targets);
* // Save the model to disk
* SVC.save('svc.bin');*
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.5]);
* // Predict the target value
* var prediction = SVC.predict(test);
*/
 exports.SVC = function(arg) {};
/**
	* returns the svc parameters	
	* @returns {module:analytics~svcParam} Parameters of the classifier model.
	*/
 exports.SVC.prototype.getParams = function() {};
/**
	* sets the svc parameters
	* @param {module:analytics~svcParam} param - Classifier training parameters.
	*/
 exports.SVC.prototype.setParams = function(param) {};
/**	
	* @property {module:la.Vector} weights - Vector of coefficients of the linear model
	*/
 exports.SVC.prototype.weights = undefined;
/**
	* saves model to output file stream 
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream
	*/
 exports.SVC.prototype.save = function(fout) {}
/**
	* sends vector through the model and returns the prediction as a real number
	* @param {module:la.Vector | module:la.SparseVector} vec - Input vector
	* @returns {number} Prediction real number. Sign of the number corresponds to the class and the magnitude corresponds to the distance from the margin (certainty).
	*/
 exports.SVC.prototype.predict = function(vec) {}
/**
	* fits an SVM classification model, given column examples in a matrix and vector of targets
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X
	*/
 exports.SVC.prototype.fit = function(X, y) {}
/**
* SVR constructor parameters
* @typedef {Object} svrParam
* @property  {number} [svrParam.c=1.0] - Cost parameter. Increasing the parameter forces the model to fit the training data more accurately (setting it too large may lead to overfitting)
* @property  {number} [svrParam.eps=1e-1] - Epsilon insensitive loss parameter. Larger values result in fewer support vectors (smaller model complexity).
* @property  {number} [svrParam.batchSize=10000] - Number of examples used in the subgradient estimation. Higher number of samples slows down the algorithm, but makes the local steps more accurate
* @property  {number} [svrParam.maxIterations=10000] - Maximum number of iterations
* @property  {number} [svrParam.maxTime=1.0] - Maximum runtime in seconds
* @property  {number} [svrParam.minDiff=1e-6] - Stopping criterion tolerance
* @property  {boolean} [svrParam.verbose=false] - Toggle verbose output in the console
*/
/**
* SVR
* @classdesc Support Vector Machine Regression. Implements a soft margin linear support vector regression using the PEGASOS algorithm with epsilon insensitive loss, see: {@link http://ttic.uchicago.edu/~nati/Publications/PegasosMPB.pdf Pegasos: Primal Estimated sub-GrAdient SOlver for SVM}.
* @class
* @param {module:fs.FIn | module:analytics~svrParam} arg - File input stream (loads the model from disk) or constructor parameters svcParam.
* @example
* // import module
* var analytics = require('qminer').analytics;
* // REGRESSION WITH SVR
* // Set up fake train and test data.
* // Four training examples with, number of features = 2
* var featureMatrix = new la.Matrix({rows:2, cols:4, random:true});
* // Regression targets for four examples
* var targets = new la.Vector([1.1, -2, 3, 4.2]);
* // Set up the regression model
* var SVR = new analytics.SVR({verbose:true});
* // Train regression
* SVR.fit(featureMatrix, targets);
* // Save the model to disk
* SVR.save('svr.bin');*
* // Set up a fake test vector
* var test = new la.Vector([1.1, -0.8]);
* // Predict the target value
* var prediction = SVR.predict(test);
*/
 exports.SVR = function(arg) {};
/**
	* returns the svr parameters
	* @returns {module:analytics~svrParam} Parameters of the regression model.
	*/
 exports.SVR.prototype.getParams = function() {};
/**
	* sets the svr parameters
	* @param {module:analytics~svrParam} param - Regression training parameters.
	*/
 exports.SVR.prototype.setParams = function(param) {};
/**
	* @property {module:la.Vector} weights - Vector of coefficients of the linear model
	*/
 exports.SVR.prototype.weights = undefined;
/**
	* saves model to output file stream
	* @param {module:fs.FOut} fout - Output stream.
	* @returns {module:fs.FOut} Output stream
	*/
 exports.SVR.prototype.save = function(fout) {}
/**
	* sends vector through the model and returns the prediction as a real number
	* @param {module:la.Vector | module:la.SparseVector} vec - Input vector
	* @returns {number} Prediction real number.
	*/
 exports.SVR.prototype.predict = function(vec) {}
/**
	* fits an SVM regression model, given column examples in a matrix and vector of targets
	* @param {module:la.Matrix | module:la.SparseMatrix} X - Input feature matrix where columns correspond to feature vectors
	* @param {module:la.Vector} y - Input vector of targets, one for each column of X
	*/
 exports.SVR.prototype.fit = function(X, y) {}
/**
 * Logistic regression model. Uses Newtons method to compute the weights.
 *
 * @constructor
 * @property {Object|FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @property {Number} [opts.lambda = 1] - the regularization parameter
 * @property {Boolean} [opts.intercept = false] - indicates wether to automatically include the intercept
 */
/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {Matrix} X - the column matrix which stores the feature vectors.
	 * @param {Vector} y - the response variable.
	 * @param {Number} [eps] - the epsilon used for convergence
	 * @returns {LogReg} - returns itself
	 */
/**
	 * Returns the expected response for the provided feature vector.
	 *
	 * @param {Vector} x - the feature vector
	 * @returns {Number} - the expected response
	 */
/**
	 * The models weights.
	 *
	 * @type {Vector}
	 */
/**
	 * Saves the model into the output stream.
	 *
	 * @param {FOut} sout - the output stream
	 */
/**
 * Exponential regression model, where the response is assumed to be exponentially
 * distributed. Finds the rate parameter with respect to the feature vector.
 *
 * Uses Newtons method to compute the weights.
 *
 * @constructor
 * @property {Object|FIn} [opts] - The options used for initialization or the input stream from which the model is loaded. If this parameter is an input stream than no other parameters are required.
 * @property {Number} [opts.lambda = 1] - the regularization parameter
 * @property {Boolean} [opts.intercept = false] - if true, the intercept will automatically be included
 */
/**
	 * Fits a column matrix of feature vectors X onto the response variable y.
	 *
	 * @param {Matrix} X - the column matrix which stores the feature vectors.
	 * @param {Vector} y - the response variable.
	 * @param {Number} [eps] - the epsilon used for convergence
	 * @returns {ExpReg} - returns itself
	 */
/**
	 * Returns the expected response for the provided feature vector.
	 *
	 * @param {Vector} x - the feature vector
	 * @returns {Number} - the expected response
	 */
/**
	 * The models weights.
	 *
	 * @type {Vector}
	 */
/**
	 * Saves the model into the output stream.
	 *
	 * @param {FOut} sout - the output stream
	 */
/**
* Ridge regression. Minimizes: ||A' x - b||^2 + ||gamma x||^2
*
* Uses Tikhonov regularization: http://en.wikipedia.org/wiki/Tikhonov_regularization
*
* @class
* @param {(number|module:fs.FIn)} [arg] - Loads a model from input stream, or creates a new model by setting gamma=arg. Empty constructor sets gamma to zero.
* @example
* la = require('qminer').la;
* analytics = require('qminer').analytics;
* // create a new model with gamma = 1.0
* regmod = new analytics.RidgeReg(1.0);
* // generate a random feature matrix
* A = la.randn(10,100);
* // generate a random model
* w = la.randn(10);
* // generate noise
* n = la.randn(100).multiply(0.01);
* // generate responses (model'*data + noise)
* b = A.transpose().multiply(w).plus(n);
* // fit model
* regmod.fit(A, b);
* // compare
* console.log('true model:');
* w.print();
* console.log('trained model:'); 
* regmod.weights.print();
* // cosine between the true and the estimated model should be close to 1 if the fit succeeded
* console.log('cosine(w, regmod.weights): ' + regmod.weights.cosine(w));
*/
 exports.RidgeReg = function(arg) {};
/**
	* Fits a column matrix of feature vectors X onto the response variable y.
	*
	* @param {module:la.Matrix} X - Column matrix which stores the feature vectors.
	* @param {module:la.Vector} y - Response variable.	
	* @returns {module:analytics.RidgeReg} Self
	*/
 exports.RidgeReg.prototype.fit = function(X,y) {}
/**
	* Returns the expected response for the provided feature vector.
	*
	* @param {module:la.Vector} x - Feature vector
	* @returns {number} Predicted response
	*/
 exports.RidgeReg.prototype.predict = function(x) {}
/**
	* @property {module:la.Vector} weights - Vector of coefficients for linear regression
	*/
 exports.RidgeReg.prototype.weights = undefined;
/**
	* Saves the model into the output stream.
	*
	* @param {module:fs.FOut} fout - Output stream
	*/
 exports.RidgeReg.prototype.save = function(fout) {};
/**
	 * Fits the model onto the data. The data instances must be stored as column vectors in X, while their times
	 * have to be stored in timeV. An optional parameter indicates wether the data provided is in
	 * batches and indicates wether the instance at index i ends a batch.
	 *
	 * @param {Matrix} X - the column matrix containing the data instances
	 * @param {Vector} timeV - a vector containing the sampling times of the instances
	 * @param {BoolVector} [endsBatchV] - a vector of boolean indicating wether the current instance ends a batch
	 * @returns {HMC} - returns itself
	 */
/**
	 * Returns the probability distribution over the future states given that the current state is the one in
	 * the parameter.
	 *
	 * @param {Number} level - the level on which we want the future states
	 * @param {Number} startState - the ID of the current state (the state we are starting from)
	 * @param {Number} [time] - optional parameter, if not specified the distribution of the next state will be returned
	 * @returns {Array} - the probability distribution
	 */
/**
	 * Returns the probability distribution over the past states given that the current state is the one in
	 * the parameter.
	 *
	 * @param {Number} level - the level on which we want the past states
	 * @param {Number} startState - the ID of the current state (the state we are starting from)
	 * @param {Number} [time] - optional parameter, if not specified the distribution of the previous state will be returned
	 * @returns {Array} - the probability distribution
	 */
/**
	 * Returns the probability distribution of past and future states over time.
	 *
	 * @param {Number} level - the level on which we want the distributions
	 * @param {Number} state - the state we are starting from
	 * @param {Number} dt - the time step (lower dt => more distributions will be returned)
	 * @returns {Array} - array of probability distributions over time
	 */
/**
	 * Returns information about previous states.
	 *
	 * @param {Number} level - the level on which we want the past states
	 * @retuns {Array} - information about the past states
	 */
/**
	 * Returns an object representation of this model.
	 *
	 * @returns {Object}
	 */
/**
	 * Returns the underlying transition model at the lowest level. (for CTMC the matrix of intensities)
	 *
	 * @returns {Array} - the transition model
	 */
/**
	 * Returns the current state throughout the hierarchy. If the level is specified it
	 * will return the current state only on that level.
	 *
	 * @param {Number} [level] - optional level parameter
	 * @returns {Array|Number} - if the level is specified it returns info about the current state on that level, otherwise it return info about the current state on each level on the hierarchy
	 */
/**
	 * Returns the centroid of the specified state.
	 *
	 * @param {Number} stateId - the ID of the state
	 * @returns {Array} - the coordinates of the state
	 */
/**
	 * Returns a histogram of the specified feature in the specified state.
	 *
	 * @param {Number} stateId - the ID of the state
	 * @param {Number} ftrId - the ID of the feature
	 * @returns {Array} - the histogram
	 */
/**
	 * Returns an array of IDs of all the states on the specified height.
	 *
	 * @param {Number} height - the height
	 * @returns {Array} - the array of IDs
	 */
/**
	 * Returns the weights of features in this state.
	 *
	 * @param {Number} stateId - The Id of the state.
	 * @returns {Array} - An array of weights.
	 */
/**
	 * Sets a callback function which is fired when the model changes states. An array of current states
	 * throughout the hierarchy is passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
/**
	 * Sets a callback function which is fired when the model detects an anomaly. A string description is
	 * passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
/**
	 * Sets a callback function which is fired when the model detects an outlier. A string description is
	 * passed to the callback.
	 *
	 * @param {function} callback - the funciton which is called
	 */
/**
	 * Rebuilds its hierarchy.
	 */
/**
	 * Rebuilds the histograms using the instances stored in the columns of X.
	 *
	 * @param {Matrix} X - the column matrix containing data instances
	 */
/**
	 * Returns the name of a state.
	 *
	 * @param {Number} stateId - ID of the state
	 * @returns {String} - the name of the state
	 */
/**
	 * Sets the name of the state.
	 *
	 * @param {Number} stateId - ID of the state
	 * @param {String} name - name of the state
	 */
/**
	 * Saves the model to the output stream.
	 *
	 * @param {FOut} fout - the output stream
	 */
 exports.HMC.prototype.save = function(arg) { return arg; }	


    function defarg(arg, defaultval) {
        return arg == undefined ? defaultval : arg;
    }

    function createBatchModel(featureSpace, models) {
        this.featureSpace = featureSpace;
        this.models = models;
        // get targets
        this.target = [];
        for (var cat in this.models) { this.target.push(cat); }
        // serialize to stream
        this.save = function (sout) {
            // save list
            sout.writeLine(this.models);
            // save feature space
            this.featureSpace.save(sout);
            // save models
            for (var cat in this.models) {
                this.models[cat].model.save(sout);
            }
            return sout;
        }

        this.predict = function (record) {
            var vec = this.featureSpace.extractSparseVector(record);
            var result = {};
            for (var cat in this.models) {
                result[cat] = this.models[cat].model.predict(vec);
            }
            return result;
        }

        this.predictLabels = function (record) {
            var result = this.predict(record);
            var labels = [];
            for (var cat in result) {
                if (result[cat] > 0.0) {
                    labels.push(cat);
                }
            }
            return labels;
        }

        this.predictTop = function (record) {
            var result = this.predict(record);
            var top = null;
            for (var cat in result) {
                if (top) {
                    if (top.weight > result[cat]) {
                        top.category = cat;
                        top.weight = result[cat];
                    }
                } else {
                    top = { category: cat, weight: result[cat] }
                }
            }
            return top.category;
        }
        return this;
    }

    //!- `batchModel = analytics.newBatchModel(rs, features, target)` -- learns a new batch model
    //!     using record set `rs` as training data and `features`; `target` is
    //!     a field descriptor JSON object for the records which we are trying to predict 
	//!     (obtained by calling store.field("Rating");
    //!     if target field string or string vector, the result is a SVM classification model,
    //!     and if target field is a float, the result is a SVM regression model; resulting 
    //!     model has the following functions:
    //!   - `strArr = batchModel.target` -- array of categories for which we have models
    //!   - `scoreArr = batchModel.predict(rec)` -- creates feature vector from record `rec`, sends it
    //!     through the model and returns the result as a dictionary where labels are keys and scores (numbers) are values.
    //!   - `labelArr = batchModel.predictLabels(rec)` -- creates feature vector from record `rec`, 
    //!     sends it through the model and returns the labels with positive weights as `labelArr`.
    //!   - `labelStr = batchModel.predictTop(rec)` -- creates feature vector from record `rec`, 
    //!     sends it through the model and returns the top ranked label `labelStr`.
    //!   - `batchModel.save(fout)` -- saves the model to `fout` output stream
    exports.newBatchModel = function (records, features, target, limitCategories) {
        console.log("newBatchModel", "Start");
        // prepare feature space
        console.log("newBatchModel", "  creating feature space");
        var featureSpace = new qm.FeatureSpace(records.store.base, features);
        // initialize features
        featureSpace.updateRecords(records);
        console.log("newBatchModel", "  number of dimensions = " + featureSpace.dim);
        // prepare spare vectors
        console.log("newBatchModel", "  preparing feature vectors");
        var sparseVecs = featureSpace.extractSparseMatrix(records);
        // prepare target vectors
        var targets = {};
        // figure out if new category name, or update count
        function initCats(categories, catName) {
            if (categories[catName]) {
                categories[catName].count++;
            } else {
                // check if we should ignore this category
                if (limitCategories && !qm_util.isInArray(limitCategories, catName)) { return; }
                // check if we should ignore this category
                categories[catName] = {
                    name: catName,
                    type: "classification",
                    count: 1,
                    target: new la.Vector({ mxVals: records.length })
                };
            }
        }
        // initialize targets
        console.log("newBatchModel", "  preparing target vectors");
        if (target.type === "string_v") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var cats = records[i][target.name];
                for (var j = 0; j < cats.length; j++) {
                    initCats(targets, cats[j]);
                }
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var cats = la.copyVecToArray(records[i][target.name]);
                for (var cat in targets) {
                    targets[cat].target.push(qm_util.isInArray(cats, cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "string") {
            // get all possible values for the field
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                initCats(targets, recCat);
            }
            // initialized with +1 or -1 for each category
            for (var i = 0; i < records.length; i++) {
                var recCat = records[i][target.name];
                for (var cat in targets) {
                    targets[cat].target.push((recCat === cat) ? 1.0 : -1.0);
                }
            }
        } else if (target.type === "float") {
            // initialized with +1 or -1 for each category
            targets[target.name] = {
                name: target.name,
                type: "regression",
                count: records.length,
                target: new la.Vector({ mxVals: records.length })

            };
            for (var i = 0; i < records.length; i++) {
                targets[target.name].target.push(records[i][target.name]);
            }
        }
        // training model for each category
        console.log("newBatchModel", "  training SVM");
        var models = {};
        for (var cat in targets) {
            if (targets[cat].count >= 50) {
                models[cat] = {
                    name: targets[cat].name,
                    type: targets[cat].type,
                };
                if (targets[cat].type === "classification") {
                    console.log("newBatchModel", "    ... " + cat + " (classification)");
                    models[cat].model = new exports.SVC({ c: 1, j: 10, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                } else if (targets[cat].type === "regression") {
                    console.log("newBatchModel", "    ... " + cat + " (regression)");
                    models[cat].model = new exports.SVR({ c: 1, eps: 1e-2, batchSize: 10000, maxIterations: 100000, maxTime: 1800, minDiff: 0.001 });
                    models[cat].model.fit(sparseVecs, targets[cat].target);
                }
            }
        }
        // done
        console.log("newBatchModel", "Done");
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };

    //!- `batchModel = analytics.loadBatchModel(base, fin)` -- loads batch model frm input stream `fin`
    exports.loadBatchModel = function (base, sin) {
        var models = JSON.parse(sin.readLine());
        var featureSpace = new qm.FeatureSpace(base, sin);
        for (var cat in models) {
            models[cat].model = new exports.SVC(sin);
        }
        // we finished the constructor
        return new createBatchModel(featureSpace, models);
    };


    //#- `cs = new analytics.classificaitonScore(cats)` -- for evaluating 
    //#     provided categories. Returns an object, which can track classification
    //#     statistics (precision, recall, F1).
    exports.classifcationScore = function (cats) {
        this.target = {};

        this.targetList = [];
        for (var i = 0; i < cats.length; i++) {
            this.target[cats[i]] = {
                id: i, count: 0, predictionCount: 0,
                TP: 0, TN: 0, FP: 0, FN: 0,
                all: function () { return this.TP + this.FP + this.TN + this.FN; },
                precision: function () { return (this.FP == 0) ? 1 : this.TP / (this.TP + this.FP); },
                recall: function () { return this.TP / (this.TP + this.FN); },
                f1: function () { return 2 * this.precision() * this.recall() / (this.precision() + this.recall()); },
                accuracy: function () { return (this.TP + this.TN) / this.all(); }
            };
            this.targetList.push(cats[i]);
        }

        //#    - `cs.count(correct, predicted)` -- adds prediction to the current
        //#         statistics. `correct` corresponds to the correct label(s), `predicted`
        //#         correspond to predicted lable(s). Labels can be either string
        //#         or string array (when there are zero or more then one lables).
        this.count = function (correct, predicted) {
            // wrapt classes in arrays if not already
            if (qm_util.isString(correct)) { this.count([correct], predicted); return; }
            if (qm_util.isString(predicted)) { this.count(correct, [predicted]); return; }
            // go over all possible categories and counts
            for (var cat in this.target) {
                var catCorrect = qm_util.isInArray(correct, cat);
                var catPredicted = qm_util.isInArray(predicted, cat);
                // update counts for correct categories
                if (catCorrect) { this.target[cat].count++; }
                // update counts for how many times category was predicted
                if (catPredicted) { this.target[cat].predictionCount++; }
                // update true/false positive/negative count
                if (catCorrect && catPredicted) {
                    // both predicted and correct say true
                    this.target[cat].TP++;
                } else if (catCorrect) {
                    // this was only correct but not predicted
                    this.target[cat].FN++;
                } else if (catPredicted) {
                    // this was only predicted but not correct
                    this.target[cat].FP++;
                } else {
                    // both predicted and correct say false
                    this.target[cat].TN++;
                }
                // update confusion matrix
            }
        };

        //#    - `cs.report()` -- prints current statisitcs for each category
        this.report = function () {
            for (var cat in this.target) {
                console.log(cat +
                    ": Count " + this.target[cat].count +
                    ", All " + this.target[cat].all() +
                    ", Precission " + this.target[cat].precision().toFixed(2) +
                    ", Recall " + this.target[cat].recall().toFixed(2) +
                    ", F1 " + this.target[cat].f1().toFixed(2) +
                    ", Accuracy " + this.target[cat].accuracy().toFixed(2));
            }
        };

        //#    - `cs.reportAvg()` -- prints current statisitcs averaged over all cagtegories
        this.reportAvg = function () {
            var count = 0, precision = 0, recall = 0, f1 = 0, accuracy = 0;
            for (var cat in this.target) {
                count++;
                precision = precision + this.target[cat].precision();
                recall = recall + this.target[cat].recall();
                f1 = f1 + this.target[cat].f1();
                accuracy = accuracy + this.target[cat].accuracy();
            }
            console.log("Categories " + count +
                ", Precission " + (precision / count).toFixed(2) +
                ", Recall " + (recall / count).toFixed(2) +
                ", F1 " + (f1 / count).toFixed(2) +
                ", Accuracy " + (accuracy / count).toFixed(2));
        }

        //#    - `cs.reportCSV(fout)` -- current statisitcs for each category to fout as CSV 
        this.reportCSV = function (fout) {
            // precison recall
            fout.writeLine("category,count,precision,recall,f1,accuracy");
            for (var cat in this.target) {
                fout.writeLine(cat +
                    "," + this.target[cat].count +
                    "," + this.target[cat].precision().toFixed(2) +
                    "," + this.target[cat].recall().toFixed(2) +
                    "," + this.target[cat].f1().toFixed(2) +
                    "," + this.target[cat].accuracy().toFixed(2));
            }
            return fout;
        };

        //#    - `res = cs.results()` -- get current statistics; `res` is an array
        //#         of object with members `precision`, `recall`, `f1` and `accuracy`
        this.results = function () {
            var res = {};
            for (var cat in this.target) {
                res[cat] = {
                    precision: this.target[cat].precision(),
                    recall: this.target[cat].recall(),
                    f1: this.target[cat].f1(),
                    accuracy: this.target[cat].accuracy(),
                };
            }
        };
    }

    //#- `result = analytics.crossValidation(rs, features, target, folds)` -- creates a batch
    //#     model for records from record set `rs` using `features; `target` is the
    //#     target field and is assumed discrete; the result is a results object
    //#     with the following API:
    //#     - `result.target` -- an object with categories as keys and the following
    //#       counts as members of these keys: `count`, `TP`, `TN`, `FP`, `FN`,
    //#       `all()`, `precision()`, `recall()`, `accuracy()`.
    //#     - `result.confusion` -- confusion matrix between categories
    //#     - `result.report()` -- prints basic report on to the console
    //#     - `result.reportCSV(fout)` -- prints CSV output to the `fout` output stream
    exports.crossValidation = function (records, features, target, folds, limitCategories) {
        // create empty folds
        var fold = [];
        for (var i = 0; i < folds; i++) {
            fold.push(new la.IntVector());
        }
        // split records into folds
        records.shuffle(1);
        var fold_i = 0;
        for (var i = 0; i < records.length; i++) {
            fold[fold_i].push(records[i].$id);
            fold_i++; if (fold_i >= folds) { fold_i = 0; }
        }
        // do cross validation
        var cfyRes = null;
        for (var fold_i = 0; fold_i < folds; fold_i++) {
            // prepare train and test record sets
            var train = new la.IntVector();
            var test = new la.IntVector();
            for (var i = 0; i < folds; i++) {
                if (i == fold_i) {
                    test.pushV(fold[i]);
                } else {
                    train.pushV(fold[i]);
                }
            }
            var trainRecs = records.store.newRecSet(train);
            var testRecs = records.store.newRecSet(test);
            console.log("Fold " + fold_i + ": " + trainRecs.length + " training and " + testRecs.length + " testing");
            // create model for the fold
            var model = exports.newBatchModel(trainRecs, features, target, limitCategories);
            // prepare test counts for each target
            if (!cfyRes) { cfyRes = new exports.classifcationScore(model.target); }
            // evaluate predictions
            for (var i = 0; i < testRecs.length; i++) {
                var correct = testRecs[i][target.name];
                var predicted = model.predictLabels(testRecs[i]);
                cfyRes.count(correct, predicted);
            }
            // report
            cfyRes.report();
        }
        return cfyRes;
    };



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


	//////////// RIDGE REGRESSION 
	// solve a regularized least squares problem
	//!- `ridgeRegressionModel = new analytics.RidgeRegression(kappa, dim, buffer)` -- solves a regularized ridge
	//!  regression problem: min|X w - y|^2 + kappa |w|^2. The inputs to the algorithm are: `kappa`, the regularization parameter,
	//!  `dim` the dimension of the model and (optional) parameter `buffer` (integer) which specifies
	//!  the length of the window of tracked examples (useful in online mode). The model exposes the following functions:
	exports.RidgeRegression = function (kappa, dim, buffer) {
	    var X = [];
	    var y = [];
	    buffer = typeof buffer !== 'undefined' ? buffer : -1;
	    var w = new la.Vector({ "vals": dim });
	    //!   - `ridgeRegressionModel.add(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set
	    this.add = function (x, target) {
	        X.push(x);
	        y.push(target);
	        if (buffer > 0) {
	            if (X.length > buffer) {
	                this.forget(X.length - buffer);
	            }
	        }
	    };
	    //!   - `ridgeRegressionModel.addupdate(vec, num)` -- adds a vector `vec` and target `num` (number) to the training set and retrains the model
	    this.addupdate = function (x, target) {
	        this.add(x, target);
	        this.update();
	    }
	    //!   - `ridgeRegressionModel.forget(n)` -- deletes first `n` (integer) examples from the training set
	    this.forget = function (ndeleted) {
	        ndeleted = typeof ndeleted !== 'undefined' ? ndeleted : 1;
	        ndeleted = Math.min(X.length, ndeleted);
	        X.splice(0, ndeleted);
	        y.splice(0, ndeleted);
	    };
	    //!   - `ridgeRegressionModel.update()` -- recomputes the model
	    this.update = function () {
	        var A = this.getMatrix();
	        var b = new la.Vector(y);
	        w = this.compute(A, b);
	    };
	    //!   - `vec = ridgeRegressionModel.getModel()` -- returns the parameter vector `vec` (dense vector)
	    this.getModel = function () {
	        return w;
	    };
	    this.getMatrix = function () {
	        if (X.length > 0) {
	            var A = new la.Matrix({ "cols": X[0].length, "rows": X.length });
	            for (var i = 0; i < X.length; i++) {
	                A.setRow(i, X[i]);
	            }
	            return A;
	        }
	    };
	    //!   - `vec2 = ridgeRegressionModel.compute(mat, vec)` -- computes the model parameters `vec2`, given 
	    //!    a row training example matrix `mat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |mat' vec2 - vec|^2 + kappa |vec2|^2.
	    //!   - `vec2 = ridgeRegressionModel.compute(spMat, vec)` -- computes the model parameters `vec2`, given 
	    //!    a row training example sparse matrix `spMat` and target vector `vec` (dense vector). The vector `vec2` solves min_vec2 |spMat' vec2 - vec|^2 + kappa |vec2|^2.
	    this.compute = function (A, b) {
	        var I = la.eye(A.cols);
	        var coefs = (A.transpose().multiply(A).plus(I.multiply(kappa))).solve(A.transpose().multiply(b));
	        return coefs;
	    };
	    //!   - `num = model.predict(vec)` -- predicts the target `num` (number), given feature vector `vec` based on the internal model parameters.
	    this.predict = function (x) {
	        return w.inner(x);
	    };
	};
	    
    
    /**
     * Hierarchical Markov model. TODO description      
     * @class
     * @param {opts} HierarchMarkovParam - parameters. TODO typedef and describe
     */
    exports.HierarchMarkov = function (opts) {
    	// constructor
    	if (opts == null) throw 'Missing parameters!';
    	if (opts.base == null) throw 'Missing parameter base!';
    	
    	// create model and feature space
    	var mc;
    	var ftrSpace;
    	
    	if (opts.hmcConfig != null && opts.ftrSpaceConfig != null && opts.base != null) {
    		mc = opts.sequenceEndV != null ? new exports.HMC(opts.hmcConfig, opts.sequenceEndV) : new exports.HMC(opts.hmcConfig);
    		ftrSpace = new qm.FeatureSpace(opts.base, opts.ftrSpaceConfig);
    	} 
    	else if (opts.hmcFile != null && opts.ftrSpaceFile != null) {
    		mc = new exports.HMC(opts.hmcFile);
    		ftrSpace = new qm.FeatureSpace(opts.base, opts.ftrSpaceFile);
    	}
    	
        // public methods
        /**
        * @lends module:analytics.HierarchMarkov.prototype
        */
    	var that = {
    		/**
    		 * Creates a new model out of the record set.
    		 */
    		fit: function (opts) {
    			var recSet = opts.recSet;
    			var batchEndV = opts.batchEndV;
    			var timeField = opts.timeField;
    			
    			log.info('Updating feature space ...');
    			ftrSpace.updateRecords(recSet);
    			
    			var colMat = ftrSpace.extractMatrix(recSet);
    			var timeV = recSet.getVector(timeField);
    			
    			log.info('Creating model ...');
    			mc.fit(colMat, timeV, batchEndV);
    			log.info('Done!');
    			
    			return that;
    		},
    		
    		/**
    		 * Adds a new record. Doesn't update the models statistics.
    		 */
    		update: function (rec) {
    			var ftrVec = ftrSpace.extractVector(rec);
    			var recTm = rec.time;
    			var timestamp = recTm.getTime();
    			
    			mc.update(ftrVec, timestamp);
    		},
    		
    		/**
    		 * Saves the feature space and model into the specified files.
    		 */
    		save: function (mcFName, ftrFname) {
    			log.info('Saving Markov chain ...');
    			mc.save(mcFName);
    			log.info('Saving feature space ...');
    			ftrSpace.save(ftrFname);
    			log.info('Done!');
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
    			return ftrSpace;
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
    		
    		getFtrNames: function () {
    			var names = [];
    			
    			var dims = ftrSpace.dims;
    			for (var i = 0; i < dims.length; i++) {
    				names.push(ftrSpace.getFeature(i));
    			}
    			
    			return names;
    		},
    		
    		/**
    		 * Returns state details as a Javascript object.
    		 */
    		stateDetails: function (stateId, level) {
    			var coords = mc.fullCoords(stateId);
    			var invCoords = ftrSpace.invFtrVec(coords);
    			var futureStates = mc.futureStates(level, stateId);
    			var pastStates = mc.pastStates(level, stateId);
    			var stateNm = mc.getStateName(stateId);
    			var wgts = mc.getStateWgtV(stateId);
    			
    			var ftrNames = that.getFtrNames();
    			var features = [];
    			for (var i = 0; i < invCoords.length; i++) {
    				features.push({name: ftrNames[i], value: invCoords.at(i)});
    			}
    			
    			return {
    				id: stateId,
    				name: stateNm.length > 0 ? stateNm : null,
    				features: features,
    				futureStates: futureStates,
    				pastStates: pastStates,
    				featureWeights: wgts
    			};
    		},
    		
    		/**
    		 * Returns a histogram for the desired feature in the desired state.
    		 */
    		histogram: function (stateId, ftrIdx) {
    			var hist = mc.histogram(stateId, ftrIdx);
    			
    			for (var i = 0; i < hist.binStartV.length; i++) {
    				hist.binStartV[i] = ftrSpace.invFtr(ftrIdx, hist.binStartV[i]);
    			}
    			
    			return hist;
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
    				var invFtrV = ftrSpace.invFtrVec(ftrV);
    				
    				var features = [];
    				for (var i = 0; i < invFtrV.length; i++) {
    					features.push({name: ftrSpace.getFeature(i), value: invFtrV.at(i)});
    				}
    				
    				callback(features);
    			});
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
    				var coords = ftrSpace.invFtrVec(mc.fullCoords(stateId));
    				
    				result.push({ state: stateId, value: coords[ftrIdx] });
    			}
    			
    			return result;
    		}
    	};
    	
    	return that;
    };

    /** 
    * @classdesc Anomaly detector that checks if the test point is too far from 
    * the nearest known point.
    * @class
    * @param {Object} [detectorParam={rate:0.05}] - Constructor parameters
    * @param {number} [detectorParam.rate=0.05] - The rate is the expected fraction of emmited anomalies (0.05 -> 5% of cases will be classified as anomalies)
    */
    exports.NearestNeighborAD = function(detectorParam) {
        // Parameters
        detectorParam = detectorParam == undefined ? {} : detectorParam;
        detectorParam.rate = detectorParam.rate == undefined ? 0.05 : detectorParam.rate;
        assert(detectorParam.rate > 0 && detectorParam.rate <= 1.0, 'rate parameter not in range (0,1]');
        // model param
        this.rate = detectorParam.rate;
        // default model
        this.thresh = 0;

        /** 
        * Gets the 100*(1-rate) percentile
        * @param {module:la.Vector} vector - Vector of values
        * @returns {number} Percentile
        */
        function getThreshold(vector, rate) {
            var sorted = vector.sortPerm().vec;
            var idx = Math.floor((1 - rate) * sorted.length);
            return sorted[idx];
        }
        var neighborDistances = undefined;

        /** 
        * Analyzes the nearest neighbor distances and computes the detector threshold based on the rate parameter.
        * @param {module:la.Matrix} A - Matrix whose columns correspond to known examples. Gets saved as it is part of
        * the model.
        */
        this.fit = function (A) {
            this.X = A;
            // distances
            var D = la.pdist2(A, A);
            // add big numbers on the diagonal (exclude the point itself from the nearest point calcualtion)
            var E = D.plus(D.multiply(la.ones(D.rows)).diag()).multiply(-1);
            var neighborDistances = new la.Vector({ vals: D.rows });
            for (var i = 0; i < neighborDistances.length; i++) {
                // nearest neighbour squared distance
                neighborDistances[i] = D.at(i, E.rowMaxIdx(i));
            }
            this.thresh = getThreshold(neighborDistances, this.rate);
        }

        /** 
        * Compares the point to the known points and returns 1 if it's too far away (based on the precomputed threshold)
        * @param {module:la.Vector} x - Test vector
        * @returns {number} Returns 1.0 if x is an anomaly and 0.0 otherwise
        */
        this.predict = function (x) {
            // compute squared dist and compare to thresh
            var d = la.pdist2(this.X, x.toMat()).getCol(0);
            var idx = d.multiply(-1).getMaxIdx();
            var p = d[idx];
            //console.log(p)
            return p > this.thresh ? 1 : 0;
        }

        /** 
        * Adds a new point (or points) to the known points and recomputes the threhshold
        * @param {(module:la.Vector | module:la.Matrix)} x - Test example (vector input) or column examples (matrix input)
        */
        this.update = function (x) {
            // append x to known examples and retrain (slow version)
            // speedup 1: don't reallocate X every time (fixed window, circular buffer)
            // speedup 2: don't recompute distances d(X,X), just d(X, y), get the minimum
            // and append to neighborDistances
            this.fit(la.cat([[this.X, x.toMat()]]));
            //console.log('new threshold ' + this.thresh);
        }
    }


    
